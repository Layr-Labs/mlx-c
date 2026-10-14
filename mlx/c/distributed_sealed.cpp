/* Copyright © 2026 Darkbloom.                         */
/* Hand-written (fork): the sealed all-sum C API.      */

#include "mlx/c/distributed_group.h"
#include "mlx/c/error.h"
#include "mlx/c/private/mlx.h"
#include "mlx/distributed/sealed.h"

extern "C" int mlx_distributed_set_seal_codec(
    mlx_distributed_seal_fn seal, mlx_distributed_open_fn open, void* context, size_t overhead) {
  try {
    mlx::core::distributed::SealCodec codec;
    codec.seal = seal;
    codec.open = open;
    codec.ctx = context;
    codec.overhead = overhead;
    mlx::core::distributed::set_seal_codec(codec);
  } catch (std::exception& e) {
    mlx_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlx_distributed_all_sum_sealed(
    mlx_array* res,
    const mlx_array x,
    const mlx_distributed_group group /* may be null */,
    const mlx_stream s) {
  try {
    mlx_array_set_(
        *res,
        mlx::core::distributed::all_sum_sealed(
            mlx_array_get_(x),
            (group.ctx ? std::make_optional(mlx_distributed_group_get_(group)) : std::nullopt),
            mlx_stream_get_(s)));
  } catch (std::exception& e) {
    mlx_error(e.what());
    return 1;
  }
  return 0;
}
