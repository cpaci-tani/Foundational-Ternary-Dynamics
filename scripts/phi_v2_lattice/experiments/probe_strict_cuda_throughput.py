# scripts/phi_v2_lattice/experiments/probe_strict_cuda_throughput.py
"""Time the accepted CUDA advance on p=1/96 Bernoulli preparations; sizes L in {16,32,48,64}."""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
import time

import numpy as np

from .. import native_codec as N_
from .. import recovery_kinetic_reference as Ref

ROOT = Path(__file__).resolve().parents[3]


def _linux(path):
    value = str(Path(path).resolve()).replace("\\", "/")
    return "/mnt/" + value[0].lower() + value[2:] if len(value) > 1 and value[1] == ":" else value


def preparation(L, seed, p=1 / 96):
    rng = np.random.default_rng(seed)
    bank = np.zeros((L ** 3, 384), dtype=bool)
    bank[:, :192] = rng.random((L ** 3, 192)) < p
    return N_.encode(Ref.prepare_bank(bank, L, layer=0))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--sizes", type=int, nargs="+", default=[16, 32, 48, 64])
    parser.add_argument("--ticks", type=int, default=48)
    args = parser.parse_args()
    cli = Path(os.environ.get("FTD_STRICT_CUDA_CLI", ROOT / "engine/build_strict_cuda/ftd_strict_cuda_cli"))
    prefix = ["wsl", "-d", "Ubuntu-22.04", "--", _linux(cli)] if os.name == "nt" else [str(cli)]
    device = json.loads(subprocess.run(prefix + ["--device"], capture_output=True, text=True, timeout=60).stdout)
    runs = []
    with tempfile.TemporaryDirectory(dir=ROOT / "engine") as folder:
        folder = Path(folder)
        for L in args.sizes:
            blob = preparation(L, seed=20260907)
            (folder / "in.bin").write_bytes(blob)
            paths = [_linux(folder / n) if os.name == "nt" else str(folder / n) for n in ("in.bin", "out.bin", "ev.json")]
            started = time.perf_counter()
            result = subprocess.run(prefix + [paths[0], paths[1], str(args.ticks), paths[2]],
                                    capture_output=True, text=True, timeout=3600)
            elapsed = time.perf_counter() - started
            if result.returncode:
                raise RuntimeError(result.stderr)
            runs.append({"L": L, "sites": L ** 3, "microticks": args.ticks, "elapsed_seconds": elapsed,
                         "microticks_per_second": args.ticks / elapsed, "state_bytes": len(blob)})
            print(json.dumps(runs[-1]), flush=True)
    args.output.write_text(json.dumps({"device": device, "runs": runs, "note":
        "wall time includes per-tick host download, decode, validation and event extraction inside advance"},
        indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
