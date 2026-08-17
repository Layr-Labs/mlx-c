#include "mlx/c/array.h"
#include "mlx/c/fast.h"
#include "mlx/c/stream.h"

int main(void) {
  float data[8] = {0};
  int shape[4] = {1, 1, 1, 8};
  mlx_array queries = mlx_array_new_data(data, shape, 4, MLX_FLOAT32);
  mlx_array keys = mlx_array_new_data(data, shape, 4, MLX_FLOAT32);
  mlx_array values = mlx_array_new_data(data, shape, 4, MLX_FLOAT32);
  mlx_array none = mlx_array_new();
  mlx_array output = mlx_array_new();
  mlx_stream stream = mlx_default_cpu_stream_new();

  int status = mlx_fast_scaled_dot_product_attention(
      &output, queries, keys, values, 1.0f, "", none, none, stream);

  mlx_stream_free(stream);
  mlx_array_free(output);
  mlx_array_free(none);
  mlx_array_free(values);
  mlx_array_free(keys);
  mlx_array_free(queries);
  return status;
}
