#include "mlx/c/array.h"
#include "mlx/c/error.h"
#include "mlx/c/fast.h"
#include "mlx/c/stream.h"

#include <stdio.h>
#include <string.h>

static char error_message[1024];

static void capture_error(const char* message, void* data) {
  (void)data;
  snprintf(error_message, sizeof(error_message), "%s", message);
}

int main(void) {
  float data[8] = {0};
  int shape[4] = {1, 1, 1, 8};
  mlx_array queries = mlx_array_new_data(data, shape, 4, MLX_FLOAT32);
  mlx_array keys = mlx_array_new_data(data, shape, 4, MLX_FLOAT32);
  mlx_array values = mlx_array_new_data(data, shape, 4, MLX_FLOAT32);
  mlx_array none = mlx_array_new();
  mlx_stream stream = mlx_default_cpu_stream_new();

  mlx_array fallback_output = mlx_array_new();
  int fallback_status = mlx_fast_scaled_dot_product_attention_v2(
      &fallback_output,
      queries,
      keys,
      values,
      1.0f,
      "",
      none,
      none,
      false,
      stream);

  mlx_set_error_handler(capture_error, NULL, NULL);
  mlx_array forced_output = mlx_array_new();
  int forced_status = mlx_fast_scaled_dot_product_attention_v2(
      &forced_output,
      queries,
      keys,
      values,
      1.0f,
      "",
      none,
      none,
      true,
      stream);

  mlx_stream_free(stream);
  mlx_array_free(forced_output);
  mlx_array_free(fallback_output);
  mlx_array_free(none);
  mlx_array_free(values);
  mlx_array_free(keys);
  mlx_array_free(queries);

  if (fallback_status != 0) {
    return 1;
  }
  if (forced_status != 1) {
    return 2;
  }
  if (strstr(error_message, "force_fused=True") == NULL ||
      strstr(error_message, "require a GPU") == NULL) {
    return 3;
  }
  return 0;
}
