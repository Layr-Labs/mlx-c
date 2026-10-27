#include <memory>

#include "mlx/c/distributed_group.h"
#include "mlx/c/error.h"
#include "mlx/c/private/distributed_bootstrap.h"
#include "mlx/c/private/mlx.h"

extern "C" int mlx_distributed_init_jaccl_with_bootstrap(
    mlx_distributed_group* res,
    int expected_rank,
    int expected_size,
    size_t maximum_rank_bytes,
    size_t maximum_total_bytes,
    mlx_distributed_bootstrap_gather gather,
    void* context,
    mlx_distributed_bootstrap_release release_context) {
  try {
    mlx::c::detail::BootstrapContext owner(context, release_context);
    if (!res) {
      throw std::invalid_argument("[distributed] Missing owner bootstrap result");
    }
    auto state = std::make_shared<mlx::c::detail::BootstrapState>(
        std::move(owner), expected_rank, expected_size,
        maximum_rank_bytes, maximum_total_bytes, gather);
    auto group = mlx::core::distributed::init(
        true, "jaccl", [state](int rank, int size) {
          state->bind(rank, size);
          return [state](const char* src, char* dst, size_t n_bytes) {
            state->all_gather(src, dst, n_bytes);
          };
        });
    state->require_fresh();
    mlx_distributed_group_set_(*res, std::move(group));
    return 0;
  } catch (std::exception& error) {
    mlx_error(error.what());
    return 1;
  } catch (...) {
    mlx_error("[distributed] Unknown owner bootstrap failure");
    return 1;
  }
}
