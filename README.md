# MLX C

> **This is a fork.** `Layr-Labs/mlx-c` tracks [`ml-explore/mlx-c`](https://github.com/ml-explore/mlx-c) and adds the C entry points Layr-Labs' Swift inference stack needs from its forked MLX core. Everything changed relative to upstream is published as a fork diff at **https://layr-labs.github.io/mlx-c/**, described in [`fork.yaml`](fork.yaml) and kept honest by CI — see [FORKDIFF.md](FORKDIFF.md).

MLX C is a C API for [MLX](https://github.com/ml-explore/mlx).

MLX is an array framework for machine learning on Apple silicon. MLX C expands
MLX to the C language, making research and experimentation easier on Apple
silicon.

MLX C can be used standalone or as a bridge to bind other languages to
MLX. For example, the [MLX Swift](https://github.com/ml-explore/mlx-swift/)
package uses MLX C to provide a Swift API to MLX.

For more information see the [docs](https://ml-explore.github.io/mlx-c).

## Install

CMake is required to build MLX C. You can install it with [Homebrew](https://brew.sh/):

```shell
brew install cmake
```

To build, run the following commands:

```shell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

From the `build/` directory, you can run an [example](examples/example.c)
that uses MLX C with `./example`.

## Contributing

Check out the [contribution guidelines](CONTRIBUTING.md) for more information
on contributing to MLX C. See the
[docs](https://ml-explore.github.io/mlx/build/html/install.html) for more
information on building from source, and running tests.

We are grateful for all of [our
contributors](ACKNOWLEDGMENTS.md#Individual-Contributors). If you contribute
to MLX C and wish to be acknowledged, please add your name to the list in your
pull request.

## Bounded JACCL startup waits

The initialized-group C API offers `mlx_distributed_group_begin_startup_wait_budget`
and `mlx_distributed_group_end_startup_wait_budget`. Begin accepts 1..900000 ms;
the monotonic deadline includes loading and cannot be renewed by progress or a
second begin. Drain queued work and serialize scheduling around both operations.
End requires an idle, open, unexpired active budget and restores the original
idle policy. Null handles, invalid bounds, unsupported backends and closed groups
are refused through the installed error handler with a nonzero return value.
No call clears failure, changes memory caps or authenticates transport. The
embedding runtime still owns hard startup cancellation and readiness publication.

These entry points require the matching core startup-wait implementation; the
CMake core pin carries it. Standalone CPU regression instructions are in
`tests/standalone/README.md`; they do not qualify native RDMA or GPU execution.
