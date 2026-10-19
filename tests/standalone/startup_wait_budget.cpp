/* Copyright © 2026 Apple Inc. */

#include <atomic>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "jaccl/startup_wait_budget.h"
#include "mlx/c/distributed_group.h"
#include "mlx/c/error.h"
#include "mlx/distributed/distributed_impl.h"

using namespace mlx::core;
using namespace mlx::core::distributed;

static_assert(std::is_same_v<
              decltype(&mlx_distributed_group_begin_startup_wait_budget),
              int (*)(mlx_distributed_group, int64_t)>);
static_assert(std::is_same_v<
              decltype(&mlx_distributed_group_end_startup_wait_budget),
              int (*)(mlx_distributed_group)>);

class Unsupported : public detail::GroupImpl {
 public:
  Stream communication_stream(StreamOrDevice) override {
    throw std::runtime_error("unused");
  }
  int rank() override {
    return 0;
  }
  int size() override {
    return 1;
  }
  std::shared_ptr<detail::GroupImpl> split(int, int) override {
    return nullptr;
  }
  void all_sum(const array&, array&, Stream) override {}
  void all_max(const array&, array&, Stream) override {}
  void all_min(const array&, array&, Stream) override {}
  void all_gather(const array&, array&, Stream) override {}
  void sum_scatter(const array&, array&, Stream) override {}
  void send(const array&, int, Stream) override {}
  void recv(array&, int, Stream) override {}
};

class BudgetGroup : public Unsupported {
 public:
  void begin_startup_wait_budget(int64_t timeout_ms) override {
    ++begins;
    policy.begin(timeout_ms, failed);
  }
  void end_startup_wait_budget() override {
    ++ends;
    policy.end(failed);
  }
  jaccl::StartupWaitBudget policy;
  std::atomic<bool> failed{false};
  int begins{0};
  int ends{0};
};

std::string error;
int errors = 0;
void handler(const char* text, void*) {
  error = text;
  ++errors;
}
void require(bool condition, const char* text) {
  if (!condition) {
    throw std::runtime_error(text);
  }
}
template <typename Fn>
void refuses(Fn fn, const char* text) {
  const int before = errors;
  error.clear();
  require(fn() != 0, "C entry point accepted invalid operation");
  require(errors == before + 1, "error handler was not called exactly once");
  require(error.find(text) != std::string::npos, "wrong refusal diagnostic");
}

int main() {
  try {
    mlx_set_error_handler(handler, nullptr, nullptr);
    mlx_distributed_group empty{nullptr};
    refuses(
        [&] {
          return mlx_distributed_group_begin_startup_wait_budget(empty, 1);
        },
        "initialized");
    refuses(
        [&] { return mlx_distributed_group_end_startup_wait_budget(empty); },
        "initialized");
    auto backend = std::make_shared<BudgetGroup>();
    Group group(backend);
    mlx_distributed_group handle{&group};
    for (int64_t timeout :
         {int64_t(-1),
          int64_t(0),
          int64_t(900001),
          std::numeric_limits<int64_t>::max(),
          std::numeric_limits<int64_t>::min()}) {
      refuses(
          [&] {
            return mlx_distributed_group_begin_startup_wait_budget(
                handle, timeout);
          },
          "1..900000");
    }
    require(backend->begins == 0, "invalid duration reached backend");
    refuses(
        [&] { return mlx_distributed_group_end_startup_wait_budget(handle); },
        "No active");
    require(
        mlx_distributed_group_begin_startup_wait_budget(handle, 900000) == 0,
        "valid begin failed");
    require(backend->begins == 1, "valid begin was not forwarded once");
    refuses(
        [&] {
          return mlx_distributed_group_begin_startup_wait_budget(handle, 1);
        },
        "only begin once");
    {
      jaccl::StartupWaitBudget::Call running(backend->policy);
      refuses(
          [&] { return mlx_distributed_group_end_startup_wait_budget(handle); },
          "idle");
    }
    require(
        mlx_distributed_group_end_startup_wait_budget(handle) == 0,
        "valid end failed");
    refuses(
        [&] { return mlx_distributed_group_end_startup_wait_budget(handle); },
        "No active");
    refuses(
        [&] {
          return mlx_distributed_group_begin_startup_wait_budget(handle, 1);
        },
        "only begin once");
    backend->failed = true;
    refuses(
        [&] { return mlx_distributed_group_end_startup_wait_budget(handle); },
        "closed");
    Group unsupported(std::make_shared<Unsupported>());
    mlx_distributed_group unsupported_handle{&unsupported};
    refuses(
        [&] {
          return mlx_distributed_group_begin_startup_wait_budget(
              unsupported_handle, 1);
        },
        "require a JACCL");
    refuses(
        [&] {
          return mlx_distributed_group_end_startup_wait_budget(
              unsupported_handle);
        },
        "require a JACCL");
    Group null_group(nullptr);
    mlx_distributed_group null_handle{&null_group};
    refuses(
        [&] {
          return mlx_distributed_group_begin_startup_wait_budget(
              null_handle, 1);
        },
        "null group");
    refuses(
        [&] {
          return mlx_distributed_group_end_startup_wait_budget(null_handle);
        },
        "null group");
    std::cout
        << "PASS real C bridge/core forwarding and one-shot policy; refusals="
        << errors << " native_rdma=0 gpu=0\n";
    mlx_set_error_handler(nullptr, nullptr, nullptr);
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
