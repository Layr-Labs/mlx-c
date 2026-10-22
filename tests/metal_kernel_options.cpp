#include "mlx/c/error.h"
#include "mlx/c/fast.h"
#include <cstdio>
#include <initializer_list>
#include <stdexcept>

// Constructor/validation test only: never applies a kernel or opens a device.
static int errors = 0;
static void capture_error(const char*, void*) { ++errors; }
static void require(bool condition) {
  if (!condition) throw std::runtime_error("Metal options constructor contract failed");
}

int main() {
  mlx_set_error_handler(capture_error, nullptr, nullptr);
  auto inputs = mlx_vector_string_new();
  auto outputs = mlx_vector_string_new();
  auto empty = mlx_vector_string_new();
  auto mutable_inputs = mlx_vector_string_new();
  auto invalid_inputs = mlx_vector_string_new();
  mlx_vector_string_append_value(inputs, "x");
  mlx_vector_string_append_value(outputs, "y");
  mlx_vector_string_append_value(mutable_inputs, "x");
  mlx_vector_string_append_value(invalid_inputs, "missing");
  int result = 0;
  try {
    for (auto mode : {MLX_FAST_METAL_MATH_SAFE, MLX_FAST_METAL_MATH_RELAXED, MLX_FAST_METAL_MATH_FAST}) {
      for (auto metadata : {empty, mutable_inputs}) {
        auto kernel = mlx_fast_metal_kernel_new_with_options(
            "options", inputs, outputs, "y[0] = x[0];", "", true, false, metadata, mode);
        require(kernel.ctx != nullptr);
        mlx_fast_metal_kernel_free(kernel);
      }
    }
    auto legacy = mlx_fast_metal_kernel_new("legacy", inputs, outputs, "y[0] = x[0];", "", true, false);
    require(legacy.ctx != nullptr);
    mlx_fast_metal_kernel_free(legacy);
    auto legacy_mutable = mlx_fast_metal_kernel_new_mutable(
        "legacy_mutable", inputs, outputs, "y[0] = x[0];", "", true, false, mutable_inputs);
    require(legacy_mutable.ctx != nullptr);
    mlx_fast_metal_kernel_free(legacy_mutable);
    require(errors == 0);
    auto invalid = mlx_fast_metal_kernel_new_with_options(
        "invalid", inputs, outputs, "y[0] = x[0];", "", true, false, empty,
        static_cast<mlx_fast_metal_kernel_math_mode>(99));
    require(invalid.ctx == nullptr && errors == 1);
    auto bad_mutable = mlx_fast_metal_kernel_new_with_options(
        "bad_mutable", inputs, outputs, "y[0] = x[0];", "", true, false,
        invalid_inputs, MLX_FAST_METAL_MATH_SAFE);
    require(bad_mutable.ctx == nullptr && errors == 2);
    std::puts("8 valid constructors and 2 rejected inputs passed; no GPU execution");
  } catch (const std::exception& error) {
    std::fprintf(stderr, "%s\n", error.what());
    result = 1;
  }
  for (auto vector : {inputs, outputs, empty, mutable_inputs, invalid_inputs})
    mlx_vector_string_free(vector);
  mlx_set_error_handler(nullptr, nullptr, nullptr);
  return result;
}
