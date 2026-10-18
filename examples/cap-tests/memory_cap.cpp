// Copyright © 2026 Eigen Labs.
#include <cassert>
#include <iostream>
#include "mlx/c/memory.h"
#include "mlx/memory.h"

namespace {
mlx::core::allocator::ProcessAllocationCap ledger;
bool unsupported = false;
int errors = 0;
} // namespace
// Link the actual C adapter to a CPU ledger; no Metal/device initialization.
namespace mlx::core {
void install_process_allocation_cap(size_t bytes) {
  if (unsupported) {
    throw std::runtime_error("unsupported");
  }
  ledger.install(bytes, 4);
}
ProcessAllocationCapSnapshot get_process_allocation_cap_snapshot() {
  if (unsupported) {
    throw std::runtime_error("unsupported");
  }
  return ledger.snapshot();
}
} // namespace mlx::core
extern "C" void _mlx_error(const char*, int, const char*, ...) {
  ++errors;
}
int main() {
  mlx_process_allocation_cap_snapshot s{};
  assert(mlx_get_process_allocation_cap_snapshot(nullptr) == 1);
  assert(mlx_install_process_allocation_cap(3) == 1);
  assert(mlx_get_process_allocation_cap_snapshot(&s) == 0 && !s.installed);
  assert(mlx_install_process_allocation_cap(16) == 0);
  assert(mlx_install_process_allocation_cap(32) == 1);
  ledger.reserve(12);
  ledger.acquired(12);
  ledger.cache(12);
  assert(mlx_get_process_allocation_cap_snapshot(&s) == 0);
  assert(s.installed && s.limit_bytes == 16 && s.heap_bytes == 4);
  assert(s.cached_bytes == 12 && s.charged_bytes == 16 && s.peak_bytes == 16);
  ledger.reuse(12);
  ledger.relinquish(12);
  assert(mlx_get_process_allocation_cap_snapshot(&s) == 0);
  assert(s.unreclaimed_bytes == 12 && s.live_bytes == 0 && s.cached_bytes == 0);
  ledger.destroyed(12);
  assert(
      mlx_get_process_allocation_cap_snapshot(&s) == 0 && s.charged_bytes == 4);
  unsupported = true;
  s.limit_bytes = 99;
  assert(
      mlx_get_process_allocation_cap_snapshot(&s) == 1 && s.limit_bytes == 99);
  assert(mlx_install_process_allocation_cap(16) == 1);
  assert(errors == 5);
  std::cout
      << "PASS: actual C adapter errors, null output, install-once, snapshot fields, unsupported backend, unchanged failed output\n";
}
