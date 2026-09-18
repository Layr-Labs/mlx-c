Fast Custom Ops
===============

Per-kernel Metal compiler options
---------------------------------

``mlx_fast_metal_kernel_new_with_options`` forwards an explicit ``safe``,
``relaxed`` or ``fast`` compiler mode to MLX for that kernel only. Existing
constructors retain the core's safe default. Pass an empty mutable-input vector
for a read-only kernel; nonempty metadata retains the existing write-hazard
contract. Mode selection is fixed at construction and is included in the core
compiled-library cache identity. It does not alter other kernels or global state.

Changing compiler modes can change numerical results. Qualify the exact
operation, device and input contract before opting in. MLX's platform restrictions
remain in force (in particular, relaxed mode needs macOS 15/iOS 18 or newer).
Unknown enum values invoke the error handler and return an empty handle.

.. doxygengroup:: fast
   :content-only:
