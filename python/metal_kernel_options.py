"""Hand-maintained fork extensions emitted with the generated Metal C wrapper."""


def extensions(implementation):
    if implementation:
        return r'''
extern "C" mlx_fast_metal_kernel mlx_fast_metal_kernel_new_mutable(
    const char* name,
    const mlx_vector_string input_names,
    const mlx_vector_string output_names,
    const char* source,
    const char* header,
    bool ensure_row_contiguous,
    bool atomic_outputs,
    const mlx_vector_string mutable_input_names) {
  try {
    return mlx_fast_metal_kernel({new mlx_fast_metal_kernel_cpp_(
        mlx::core::fast::metal_kernel_with_mutable_inputs(
            name, mlx_vector_string_get_(input_names),
            mlx_vector_string_get_(output_names), source,
            mlx_vector_string_get_(mutable_input_names), header,
            ensure_row_contiguous, atomic_outputs))});
  } catch (std::exception& e) {
    mlx_error(e.what());
  }
  return {nullptr};
}

extern "C" mlx_fast_metal_kernel mlx_fast_metal_kernel_new_with_options(
    const char* name,
    const mlx_vector_string input_names,
    const mlx_vector_string output_names,
    const char* source,
    const char* header,
    bool ensure_row_contiguous,
    bool atomic_outputs,
    const mlx_vector_string mutable_input_names,
    mlx_fast_metal_kernel_math_mode math_mode) {
  try {
    mlx::core::CompileOptions options;
    switch (math_mode) {
      case MLX_FAST_METAL_MATH_SAFE:
        options.math_mode = mlx::core::MathMode::Safe;
        break;
      case MLX_FAST_METAL_MATH_RELAXED:
        options.math_mode = mlx::core::MathMode::Relaxed;
        break;
      case MLX_FAST_METAL_MATH_FAST:
        options.math_mode = mlx::core::MathMode::Fast;
        break;
      default:
        throw std::invalid_argument("Unknown custom Metal kernel math mode.");
    }
    return mlx_fast_metal_kernel({new mlx_fast_metal_kernel_cpp_(
        mlx::core::fast::metal_kernel_with_mutable_inputs(
            name, mlx_vector_string_get_(input_names),
            mlx_vector_string_get_(output_names), source,
            mlx_vector_string_get_(mutable_input_names), header,
            ensure_row_contiguous, atomic_outputs, options))});
  } catch (std::exception& e) {
    mlx_error(e.what());
  }
  return {nullptr};
}
'''
    return r'''
mlx_fast_metal_kernel mlx_fast_metal_kernel_new_mutable(
    const char* name,
    const mlx_vector_string input_names,
    const mlx_vector_string output_names,
    const char* source,
    const char* header,
    bool ensure_row_contiguous,
    bool atomic_outputs,
    const mlx_vector_string mutable_input_names);

/** Compiler math mode for one custom kernel; legacy constructors stay safe. */
typedef enum mlx_fast_metal_kernel_math_mode_ {
  MLX_FAST_METAL_MATH_SAFE = 0,
  MLX_FAST_METAL_MATH_RELAXED = 1,
  MLX_FAST_METAL_MATH_FAST = 2
} mlx_fast_metal_kernel_math_mode;

/**
 * Create one kernel with explicit compiler options and mutable-input metadata.
 * Pass a valid empty vector when no input is mutable. This does not change
 * global defaults. Different modes may produce different numerical results;
 * the caller must qualify the selected mode. Invalid modes return an empty
 * handle after invoking the error handler. Backend OS restrictions still apply.
 */
mlx_fast_metal_kernel mlx_fast_metal_kernel_new_with_options(
    const char* name,
    const mlx_vector_string input_names,
    const mlx_vector_string output_names,
    const char* source,
    const char* header,
    bool ensure_row_contiguous,
    bool atomic_outputs,
    const mlx_vector_string mutable_input_names,
    mlx_fast_metal_kernel_math_mode math_mode);
'''
