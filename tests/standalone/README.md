# Startup-wait C bridge CPU regression

Run from the repository root against the matching core startup-wait source:

```sh
python3 tests/standalone/run_startup_wait_tests.py --core /path/to/mlx --output /path/to/isolated-output
```

The test compiles the real C entry points, error handler and core Group
forwarding with address/undefined-behavior sanitizers. A test GroupImpl uses the
real one-shot policy. It checks C ABI types, null/invalid handles, finite duration
bounds, exactly-once forwarding, running/ended/failed/unsupported refusal and
error-handler reporting. Dead stripping excludes unrelated backend operations;
no fake core symbols, RDMA service, Metal framework or GPU evaluator is linked.
This lightweight runner uses the macOS linker and does not qualify native RDMA,
actual hardware, or the full C library.
