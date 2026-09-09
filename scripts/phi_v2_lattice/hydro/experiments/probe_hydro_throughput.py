# scripts/phi_v2_lattice/hydro/experiments/probe_hydro_throughput.py
"""Amortized CUDA throughput probe for phi-hydro-staged-candidate-1 (H2' instrument
sizing; task-11-brief.md). For each L in --sizes, one `prepare.fluid(L, 0, 1/4)`
preparation is timed through the CUDA CLI (ftd_hydro_cuda_cli) at 4 and 160 microticks;
the one-time per-invocation overhead -- process start, the collision table's read +
SHA-256 verification from disk, and its one-shot upload to the device inside
`gpu::advance` -- is amortized away by taking the difference:
    microticks_per_second = (160 - 4) / (t160 - t4)
A third timing at 0 microticks (t0) is also taken. `gpu::advance` returns immediately,
before touching the device at all, when microticks == 0 (see hydro_cuda.cu: "if
(!microticks) return {};"), so t0 captures process start + the table's disk read/verify
WITHOUT the device upload. `table_upload_seconds` is then DERIVED, not directly measured
(no timing hook exists inside advance(), and none is added here since the CUDA sources
this task depends on are frozen except for CMakeLists.txt):
    table_upload_seconds = max(0, (t4 - t0) - 4 * marginal_seconds_per_microtick)
i.e. t4-t0 is process-start-cancelled overhead-plus-4-microticks; subtracting the
kernel-only cost of those 4 microticks (from the amortized rate above) isolates the
upload. This is an estimate for instrument sizing, not a registered timing result.
"""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
import time

from .. import codec as N_
from .. import prepare as R
from .. import staged as S

ROOT = Path(__file__).resolve().parents[4]


def _linux(path) -> str:
    value = str(Path(path).resolve()).replace("\\", "/")
    return "/mnt/" + value[0].lower() + value[2:] if len(value) > 1 and value[1] == ":" else value


def preparation(L: int) -> bytes:
    lattice = R.fluid(L, 0, 0.25)
    return N_.encode(S.initialize(lattice))


def _to_wsl(path) -> str:
    return _linux(path) if os.name == "nt" else str(path)


def _time_cli(prefix, table_path, source_path, folder, microticks: int) -> float:
    target, events = folder / f"out_{microticks}.bin", folder / f"ev_{microticks}.json"
    args = [table_path, source_path, _to_wsl(target), str(microticks), _to_wsl(events)]
    started = time.perf_counter()
    result = subprocess.run(prefix + args, capture_output=True, text=True, timeout=3600)
    elapsed = time.perf_counter() - started
    if result.returncode:
        raise RuntimeError(f"CUDA CLI failed at microticks={microticks}: {result.stderr}")
    return elapsed


def probe_one(prefix, table_path, L: int, folder: Path) -> dict:
    source = folder / "in.bin"
    blob = preparation(L)
    source.write_bytes(blob)
    source_path = _to_wsl(source)
    # One untimed warm-up call so the 64 MiB table's disk read hits the OS page cache before
    # any timed run -- otherwise t0 (run first) pays a cold-cache penalty t4/t160 do not,
    # confounding table_upload_seconds_derived with disk-cache warmup rather than isolating
    # the device upload.
    _time_cli(prefix, table_path, source_path, folder, 1)
    t0 = _time_cli(prefix, table_path, source_path, folder, 0)
    t4 = _time_cli(prefix, table_path, source_path, folder, 4)
    t160 = _time_cli(prefix, table_path, source_path, folder, 160)
    marginal = (t160 - t4) / (160 - 4)
    microticks_per_second = 1.0 / marginal
    table_upload_seconds = max(0.0, (t4 - t0) - 4 * marginal)
    return {"L": L, "sites": L ** 3, "state_bytes": len(blob), "t0_seconds": t0, "t4_seconds": t4,
            "t160_seconds": t160, "marginal_seconds_per_microtick": marginal,
            "microticks_per_second": microticks_per_second, "table_upload_seconds_derived": table_upload_seconds}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path,
                        default=ROOT / "engine" / "docs" / "evidence" / "strict-hydro4-throughput.json")
    parser.add_argument("--sizes", type=int, nargs="+", default=[32, 48, 64])
    args = parser.parse_args()

    from .. import channels as H

    cli = Path(os.environ.get("FTD_HYDRO_CUDA_CLI", ROOT / "engine/build_strict_hydro_cuda/ftd_hydro_cuda_cli"))
    table = H.table_path()
    prefix = ["wsl", "-d", "Ubuntu-22.04", "--", _linux(cli)] if os.name == "nt" else [str(cli)]
    table_arg = _linux(table) if os.name == "nt" else str(table)
    device = json.loads(subprocess.run(prefix + ["--device"], capture_output=True, text=True, timeout=60).stdout)

    runs = []
    with tempfile.TemporaryDirectory(dir=ROOT / "engine") as folder:
        folder = Path(folder)
        for L in args.sizes:
            run = probe_one(prefix, table_arg, L, folder)
            runs.append(run)
            print(json.dumps(run), flush=True)

    payload = {"device": device, "runs": runs,
               "note": ("microticks_per_second = (160-4)/(t160-t4), amortizing away process start, "
                        "table disk-load/verify, and the one-shot device table upload; "
                        "table_upload_seconds_derived is an ESTIMATE (see module docstring), not a direct "
                        "measurement -- no timing hook exists inside gpu::advance and none was added here.")}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"wrote": str(args.output)}))


if __name__ == "__main__":
    main()
