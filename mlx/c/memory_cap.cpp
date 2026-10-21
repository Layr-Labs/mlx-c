// Copyright © 2026 Eigen Labs.
#include "mlx/c/error.h"
#include "mlx/c/memory.h"
#include "mlx/memory.h"

extern "C" int mlx_install_process_allocation_cap(size_t bytes) {
  try {
    mlx::core::install_process_allocation_cap(bytes);
    return 0;
  } catch (const std::exception& e) {
    mlx_error("%s", e.what());
    return 1;
  }
}

extern "C" int mlx_get_process_allocation_cap_snapshot(
    mlx_process_allocation_cap_snapshot* res) {
  try {
    if (!res) {
      throw std::invalid_argument("null allocation cap snapshot");
    }
    const auto s = mlx::core::get_process_allocation_cap_snapshot();
    *res = {
        s.installed,
        s.limit_bytes,
        s.heap_bytes,
        s.live_bytes,
        s.cached_bytes,
        s.pending_bytes,
        s.unreclaimed_bytes,
        s.charged_bytes,
        s.peak_bytes};
    return 0;
  } catch (const std::exception& e) {
    mlx_error("%s", e.what());
    return 1;
  }
}
