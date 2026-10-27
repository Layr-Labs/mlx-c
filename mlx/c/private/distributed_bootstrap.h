#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>

namespace mlx::c::detail {

using BootstrapGather = int (*)(
    void*, int, int, uint64_t, const char*, size_t, char*, size_t);
using BootstrapRelease = void (*)(void*);

class BootstrapContext {
 public:
  BootstrapContext(void* value, BootstrapRelease release)
      : value_(value), release_(release) {}
  BootstrapContext(const BootstrapContext&) = delete;
  BootstrapContext& operator=(const BootstrapContext&) = delete;
  BootstrapContext(BootstrapContext&& other) noexcept
      : value_(other.value_), release_(other.release_) {
    other.release_ = nullptr;
  }
  ~BootstrapContext() {
    if (release_) {
      release_(value_);
    }
  }
  void* value() const { return value_; }
  bool owned() const { return release_ != nullptr; }

 private:
  void* value_;
  BootstrapRelease release_;
};

class BootstrapState {
 public:
  BootstrapState(
      BootstrapContext context,
      int rank,
      int size,
      size_t maximum_rank_bytes,
      size_t maximum_total_bytes,
      BootstrapGather gather)
      : context_(std::move(context)),
        rank_(rank),
        size_(size),
        maximum_rank_bytes_(maximum_rank_bytes),
        maximum_total_bytes_(maximum_total_bytes),
        gather_(gather) {
    if (!context_.owned() || !gather_ || rank < 0 || rank >= size ||
        size < 2 || size > 64 || maximum_rank_bytes == 0 ||
        maximum_rank_bytes > 1024 * 1024 || maximum_total_bytes == 0 ||
        maximum_total_bytes > 8 * 1024 * 1024 ||
        maximum_rank_bytes > maximum_total_bytes / static_cast<size_t>(size)) {
      throw std::invalid_argument("[distributed] Invalid owner bootstrap bounds");
    }
  }

  void bind(int rank, int size) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (failed_ || bound_ || rank != rank_ || size != size_) {
      failed_ = true;
      throw std::runtime_error("[distributed] Owner bootstrap rank or size differs");
    }
    bound_ = true;
  }

  void require_fresh() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!bound_ || failed_) {
      throw std::runtime_error("[distributed] Owner bootstrap requires fresh JACCL initialization");
    }
  }

  void all_gather(const char* src, char* dst, size_t n_bytes) {
    std::lock_guard<std::mutex> lock(mutex_);
    try {
      if (failed_ || !bound_ || !src || !dst || n_bytes == 0 ||
          n_bytes > maximum_rank_bytes_ ||
          n_bytes > maximum_total_bytes_ / static_cast<size_t>(size_) ||
          sequence_ == std::numeric_limits<uint64_t>::max()) {
        throw std::runtime_error("[distributed] Owner bootstrap contribution is outside bounds");
      }
      const size_t total = n_bytes * static_cast<size_t>(size_);
      std::vector<char> local(src, src + n_bytes);
      std::vector<char> gathered(total);
      if (gather_(context_.value(), rank_, size_, sequence_, local.data(),
                  n_bytes, gathered.data(), total) != 0) {
        throw std::runtime_error("[distributed] Owner bootstrap exchange failed");
      }
      if (std::memcmp(local.data(), gathered.data() + rank_ * n_bytes, n_bytes) != 0) {
        throw std::runtime_error("[distributed] Owner bootstrap local contribution differs");
      }
      std::memcpy(dst, gathered.data(), total);
      ++sequence_;
    } catch (...) {
      failed_ = true;
      throw;
    }
  }

 private:
  BootstrapContext context_;
  int rank_;
  int size_;
  size_t maximum_rank_bytes_;
  size_t maximum_total_bytes_;
  BootstrapGather gather_;
  std::mutex mutex_;
  uint64_t sequence_{0};
  bool bound_{false};
  bool failed_{false};
};

} // namespace mlx::c::detail
