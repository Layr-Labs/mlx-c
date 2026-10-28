#!/usr/bin/env python3
"""Build the real C bridge and core forwarding without a native backend."""
import argparse
import pathlib
import subprocess
import sys

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--core", type=pathlib.Path, required=True)
parser.add_argument("--output", type=pathlib.Path, required=True)
parser.add_argument("--compiler", default="clang++")
args = parser.parse_args()
root = pathlib.Path(__file__).resolve().parents[2]
core = args.core.resolve()
output = args.output.resolve()
output.mkdir(parents=True, exist_ok=True)
# Dead stripping keeps only the tested entry points. No fake core symbols,
# RDMA library, Metal framework or GPU evaluator are linked.
command = [
    args.compiler, "-std=c++20", "-O1", "-g",
    "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
    "-ffunction-sections", "-fdata-sections", "-Wl,-dead_strip",
    "-I" + str(root), "-I" + str(core),
    "-I" + str(core / "mlx/distributed/jaccl/lib"),
    str(root / "tests/standalone/startup_wait_budget.cpp"),
    str(root / "mlx/c/distributed_group.cpp"),
    str(root / "mlx/c/error.cpp"),
    str(core / "mlx/distributed/distributed.cpp"),
    "-o", str(output / "startup-wait-c-bridge"),
]
subprocess.run(command, check=True, timeout=120)
subprocess.run([str(output / "startup-wait-c-bridge")], check=True, timeout=30)
