"""Replay one registered H2' campaign case, for audit ties.

Two independent ties against a completed campaign directory
(`engine/build_strict_hydro/campaign_v1`), neither of which touches any campaign file:

  1. **Instrument tie.** Re-run the *same* frozen CUDA instrument
     (`campaign.RUNNER`, hash-locked in `lock.json`) on a one-case manifest built from the
     campaign's own `manifest.tsv` row for that case, writing into a scratch directory,
     and compare the resulting `stages + 1` trace records **byte-for-byte** with the
     records the campaign's `trace.jsonl` carries for the same case. This is the direct
     test of the property `resume_campaign`'s receipt asserts when it concatenates
     kept-complete blocks with freshly re-run ones: that each case is an independent
     deterministic computation of its own frozen preparation, so a case's block does not
     depend on which other cases ran before it, or on when.

  2. **Python-reference tie.** Decode the same frozen preparation with
     `hydro.codec`, step it with the pure-Python staged reference
     (`hydro.staged.step`, four microticks per stage), and compare the projected moments
     (`campaign.observe`) against the instrument's recorded `moments`, and the
     exact-integer `mass`/`total_momentum` conservation receipt. The moment comparison is
     gated on the RELATIVE difference (`--moment-tolerance`, default 1e-9): both sides sum
     e^{-ik.x} over L^3 = 110592 sites in different orders and different precisions, so the
     absolute residual on a moment of magnitude ~1e4 floats at the 1e-8 level while the
     relative residual sits near 1e-12 -- the meaningful figure. The reference is O(L^3)
     Python per microtick, so this tie is run over the first few stages only
     (`--python-stages`, default 2 = 8 microticks).

Nothing here is part of the registered apparatus: this module is an audit instrument,
never imported by `campaign.py`, and it writes only under the directory passed as
`--scratch`.

Example (from the repository root):

    python scripts/phi_v2_lattice/hydro/experiments/replay_hydro_case.py \\
        --campaign engine/build_strict_hydro/campaign_v1 \\
        --scratch engine/build_strict_hydro/audit_scratch/replay \\
        --case shear-n100-t010-m1-e1_10-s0 --python-stages 2
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import time
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[4]
if str(ROOT / "scripts") not in sys.path:
    sys.path.insert(0, str(ROOT / "scripts"))

from phi_v2_lattice.hydro import campaign as C  # noqa: E402
from phi_v2_lattice.hydro import channels as H  # noqa: E402
from phi_v2_lattice.hydro import codec as CODEC  # noqa: E402
from phi_v2_lattice.hydro import staged as Staged  # noqa: E402


def read_manifest_rows(campaign: Path) -> dict[str, str]:
    """case_id -> its verbatim `manifest.tsv` line (the executable manifest the lock pins)."""
    text = (campaign / "manifest.tsv").read_text(encoding="utf-8")
    return {line.split("\t", 1)[0]: line for line in text.split("\n") if line.strip()}


def read_trace_block(campaign: Path, case_id: str, stages: int) -> list[bytes]:
    """The `stages + 1` raw record lines the campaign trace carries for `case_id`, in file
    order, with no text-mode newline translation."""
    data = (campaign / "trace.jsonl").read_bytes()
    lines = data.split(b"\n")
    if lines and lines[-1] == b"":
        lines.pop()
    block = [raw for raw in lines[1:] if raw and json.loads(raw.decode("utf-8"))["case_id"] == case_id]
    if len(block) != stages + 1:
        raise ValueError(f"{case_id}: expected {stages + 1} records in trace.jsonl, found {len(block)}")
    return block


def run_instrument(case_id: str, manifest_row: str, scratch: Path, stages: int) -> tuple[list[bytes], float, dict]:
    """Run the locked CUDA instrument on a one-case manifest in `scratch`; return its
    `stages + 1` raw record lines, the elapsed wall time, and the device provenance."""
    workdir = scratch / case_id
    workdir.mkdir(parents=True, exist_ok=True)
    manifest_path = workdir / "manifest.tsv"
    trace_path = workdir / "trace.jsonl"
    manifest_path.write_bytes((manifest_row + "\n").encode("utf-8"))
    for stale in (trace_path, Path(str(trace_path) + ".part")):
        if stale.exists():
            stale.unlink()

    runner = ROOT / C.RUNNER
    args = [H.table_path(), manifest_path, trace_path]
    linux_args = [C._linux(p) for p in args] if os.name == "nt" else [str(p) for p in args]
    runner_arg = C._linux(runner) if os.name == "nt" else str(runner)
    command = (["wsl", "-d", "Ubuntu-22.04", "--", runner_arg] + linux_args if os.name == "nt"
               else [str(runner)] + linux_args) + [str(stages)]
    started = time.perf_counter()
    result = subprocess.run(command, capture_output=True, text=True)
    elapsed = time.perf_counter() - started
    (workdir / "stderr.txt").write_text(result.stderr, encoding="utf-8")
    if result.returncode:
        raise RuntimeError(f"instrument failed for {case_id}: {result.stderr}")
    devices = [json.loads(line) for line in result.stderr.splitlines() if line.startswith("{")]
    if not devices or devices[0].get("backend") != "cuda_device_kernels":
        raise RuntimeError(f"no real GPU provenance for {case_id}: {result.stderr}")
    lines = trace_path.read_bytes().split(b"\n")
    if lines and lines[-1] == b"":
        lines.pop()
    return lines[1:], elapsed, devices[0]


def python_reference_tie(campaign: Path, case_id: str, lock: dict, block: list[bytes],
                         stages: int) -> list[dict]:
    """Step the frozen preparation with the pure-Python staged reference and compare the
    projected moments (float) and the mass/momentum receipt (exact integers) against the
    instrument's own records for stages 0..`stages`."""
    row = next(r for r in lock["manifest"] if r["case"]["case_id"] == case_id)
    case = C._case_from_dict(row["case"])
    state = CODEC.decode((campaign / (case_id + ".bin")).read_bytes())
    table = H.load_table()
    velocities = np.array(H.VELOCITIES, dtype=np.int64)
    out = []
    for stage in range(stages + 1):
        if stage:
            for _ in range(C.STAGE_MICROTICKS):
                state, _events = Staged.step(state, table)
        record = json.loads(block[stage].decode("utf-8"))
        moments_instrument = np.array([complex(a, b) for a, b in record["moments"]])
        moments_reference = C.observe(state, case)
        bank = state.lattice.bank.reshape(-1, H.N_POL, H.N_PHASE, H.N_VEL)
        per_velocity = bank.sum(axis=(0, 1, 2))
        out.append({
            "stage": stage,
            "microtick": int(state.microtick),
            "max_abs_moment_difference": float(np.max(np.abs(moments_reference - moments_instrument))),
            "max_relative_moment_difference": float(
                np.max(np.abs(moments_reference - moments_instrument)) / max(np.max(np.abs(moments_instrument)), 1.0)),
            "mass_reference": int(bank.sum()),
            "mass_instrument": int(record["mass"]),
            "momentum_reference": [int(v) for v in (per_velocity @ velocities)],
            "momentum_instrument": [int(v) for v in record["total_momentum"]],
        })
    return out


def replay(campaign: Path, scratch: Path, case_ids: list[str], python_stages: int,
           moment_tolerance: float) -> dict:
    lock = C.validate_lock(campaign)
    stages = lock["registration"]["stages"]
    rows = read_manifest_rows(campaign)
    report = {"campaign": str(campaign), "stages": stages, "cases": []}
    for case_id in case_ids:
        if case_id not in rows:
            raise ValueError(f"{case_id} is not a registered manifest case")
        recorded = read_trace_block(campaign, case_id, stages)
        replayed, elapsed, device = run_instrument(case_id, rows[case_id], scratch, stages)
        identical = replayed == recorded
        first_difference = None
        if not identical:
            for index, (a, b) in enumerate(zip(recorded, replayed)):
                if a != b:
                    first_difference = {"index": index, "recorded": a.decode("utf-8", "replace"),
                                        "replayed": b.decode("utf-8", "replace")}
                    break
        entry = {"case_id": case_id, "records_recorded": len(recorded), "records_replayed": len(replayed),
                 "byte_identical": identical, "first_difference": first_difference,
                 "instrument_seconds": elapsed, "device": device.get("name")}
        if python_stages:
            ties = python_reference_tie(campaign, case_id, lock, recorded, python_stages)
            entry["python_reference"] = ties
            entry["python_reference_ok"] = all(
                t["max_relative_moment_difference"] <= moment_tolerance
                and t["mass_reference"] == t["mass_instrument"]
                and t["momentum_reference"] == t["momentum_instrument"] for t in ties)
        report["cases"].append(entry)
    return report


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--campaign", default="engine/build_strict_hydro/campaign_v1",
                        help="completed campaign directory (read-only)")
    parser.add_argument("--scratch", default="engine/build_strict_hydro/audit_scratch/replay",
                        help="scratch directory for the one-case manifests and traces")
    parser.add_argument("--case", action="append", dest="cases", required=True,
                        help="registered case_id to replay (repeatable)")
    parser.add_argument("--python-stages", type=int, default=2,
                        help="stages to tie against the pure-Python staged reference (0 disables)")
    parser.add_argument("--moment-tolerance", type=float, default=1e-9,
                        help="relative tolerance on the projected moments in the Python tie")
    parser.add_argument("--json", dest="json_path", default=None, help="write the report here")
    args = parser.parse_args(argv)

    campaign = Path(args.campaign)
    campaign = campaign if campaign.is_absolute() else ROOT / campaign
    scratch = Path(args.scratch)
    scratch = scratch if scratch.is_absolute() else ROOT / scratch
    report = replay(campaign.resolve(), scratch.resolve(), args.cases, args.python_stages,
                    args.moment_tolerance)
    text = json.dumps(report, indent=2, sort_keys=True)
    print(text)
    if args.json_path:
        Path(args.json_path).write_text(text + "\n", encoding="utf-8")
    return 0 if all(c["byte_identical"] and c.get("python_reference_ok", True)
                    for c in report["cases"]) else 1


if __name__ == "__main__":
    raise SystemExit(main())
