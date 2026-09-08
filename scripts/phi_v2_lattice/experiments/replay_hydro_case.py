"""Independent replay of two registered hydrodynamic-response campaign cases.

Task 11 audit script (reviewer, not the implementer of the campaign). Compares
the accepted CUDA backend `ftd_strict_cuda_cli` against the pure-Python
`staged.step` reference for the first two manifest cases of
`engine/build_strict_hydro/campaign_v1`, and ties the locked `trace.jsonl` to
an independent CUDA invocation of the same accepted backend. Two comparisons
are made per case:

  (a) 8-microtick Python-vs-CUDA state equality. The pure-Python `staged.step`
      reference is too slow to replay a full 48-microtick stroboscope at
      L = 32, so only the first 8 microticks are replayed with `P.step`
      (called 8 times) and compared against an 8-tick run of the accepted
      CUDA CLI on the same locked preparation.
  (b) 48-microtick CUDA-vs-trace state equality. The accepted CUDA CLI is run
      for 48 microticks (one full stroboscope) from the same locked
      preparation, independently of the campaign runner
      (`ftd_strict_hydro_campaign`), and the resulting complete-state SHA256
      is compared against the `stroboscopes[1].sha256` value already
      recorded in the campaign's `trace.jsonl`. This ties the campaign trace
      to an independent CUDA run of the accepted backend, not merely to
      itself.

Never writes into `engine/build_strict_hydro/campaign_v1/` (retained
evidence); it is read-only here. All scratch files (CUDA CLI outputs and
event logs) are written under `engine/build_strict_hydro/replay/`.
"""
from __future__ import annotations

import hashlib
import json
import subprocess
from pathlib import Path

from phi_v2_lattice import native_codec as N_
from phi_v2_lattice import recovery_hydro_campaign as C
from phi_v2_lattice import staged as P

ROOT = Path(__file__).resolve().parents[3]
CAMPAIGN_DIR = ROOT / "engine" / "build_strict_hydro" / "campaign_v1"
REPLAY_DIR = ROOT / "engine" / "build_strict_hydro" / "replay"
CLI = ROOT / "engine" / "build_strict_cuda" / "ftd_strict_cuda_cli"


def _run_cli(input_path: Path, output_path: Path, ticks: int, events_path: Path) -> None:
    """Invoke the accepted CUDA CLI via WSL2 Ubuntu-22.04, converting every path with
    the same `_linux()` helper the campaign module (`recovery_hydro_campaign`) uses."""
    if output_path.exists():
        output_path.unlink()
    command = ["wsl", "-d", "Ubuntu-22.04", "--", C._linux(CLI), C._linux(input_path),
               C._linux(output_path), str(ticks), C._linux(events_path)]
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"ftd_strict_cuda_cli failed (ticks={ticks}): {result.stderr}")


def _sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _trace_stroboscope1_sha256(case_id: str) -> str:
    with (CAMPAIGN_DIR / "trace.jsonl").open(encoding="utf-8") as stream:
        for line in stream:
            row = json.loads(line)
            if row["case_id"] == case_id:
                strobe = row["stroboscopes"][1]
                assert strobe["n"] == 1, "expected stroboscope index 1 (48 microticks)"
                return strobe["sha256"]
    raise ValueError(f"case_id {case_id!r} not found in trace.jsonl")


def replay_case(case_id: str) -> dict:
    REPLAY_DIR.mkdir(parents=True, exist_ok=True)
    prep_path = CAMPAIGN_DIR / (case_id + ".bin")  # read-only: campaign_v1 is never written to

    # (a) 8-microtick Python reference vs. 8-tick CUDA CLI, same locked preparation.
    state = N_.decode(prep_path.read_bytes())
    for _ in range(8):
        state, _events = P.step(state)
    python_8_sha256 = _sha256_bytes(N_.encode(state))

    cuda_8_out = REPLAY_DIR / f"{case_id}.cuda8.bin"
    cuda_8_events = REPLAY_DIR / f"{case_id}.cuda8.events.json"
    _run_cli(prep_path, cuda_8_out, 8, cuda_8_events)
    cuda_8_sha256 = _sha256_bytes(cuda_8_out.read_bytes())
    eight_tick_match = python_8_sha256 == cuda_8_sha256

    # (b) 48-microtick CUDA CLI (independent of the campaign runner) vs. the
    # campaign trace's recorded stroboscope-1 state hash.
    cuda_48_out = REPLAY_DIR / f"{case_id}.cuda48.bin"
    cuda_48_events = REPLAY_DIR / f"{case_id}.cuda48.events.json"
    _run_cli(prep_path, cuda_48_out, 48, cuda_48_events)
    cuda_48_sha256 = _sha256_bytes(cuda_48_out.read_bytes())
    trace_sha256 = _trace_stroboscope1_sha256(case_id)
    forty_eight_tick_match = cuda_48_sha256 == trace_sha256

    return {"case_id": case_id,
            "python_8tick_sha256": python_8_sha256, "cuda_8tick_sha256": cuda_8_sha256,
            "eight_tick_python_vs_cuda_match": eight_tick_match,
            "cuda_48tick_sha256": cuda_48_sha256, "trace_stroboscope1_sha256": trace_sha256,
            "forty_eight_tick_cuda_vs_trace_match": forty_eight_tick_match}


def main() -> None:
    lock = json.loads((CAMPAIGN_DIR / "lock.json").read_text(encoding="utf-8"))
    case_ids = [row["case"]["case_id"] for row in lock["manifest"][:2]]
    results = [replay_case(cid) for cid in case_ids]
    for r in results:
        print(r["case_id"],
              "8-tick Python==CUDA:", r["eight_tick_python_vs_cuda_match"],
              "| 48-tick CUDA==trace:", r["forty_eight_tick_cuda_vs_trace_match"])
    print(json.dumps(results, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
