"""Bounded independent complete-checkpoint parity against the native candidate.

Run: python -m scripts.phi_v2_lattice.thermal.verify_native --binary PATH
No main dashboard owner is created. Fixtures are regression data, not gas trials.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from random import Random
import subprocess
from tempfile import TemporaryDirectory

from . import runtime as R


def verify(binary: Path, equivariant=False) -> dict:
    if equivariant:
        from . import runtime_v2 as law
    else:
        law = R
    native_hash = subprocess.check_output([str(binary), '--table-hash'], text=True, timeout=15).strip()
    if native_hash != law.collision_identity():
        raise AssertionError("complete native collision table differs from the independent law")
    count = 0
    with TemporaryDirectory(prefix="ftd-thermal-parity-") as folder:
        root = Path(folder)
        for L in (3, 5):
            for phase in range(4):
                state = R.Records.empty(L)
                state.microtick = (1 << 54)+phase  # catches float or int32 clock narrowing
                rng = Random(20260910+L*4+phase)
                for i in range(L**3):
                    for c in rng.sample(range(R.CHANNELS), rng.randrange(5)):
                        state.bank[i] |= 1 << c
                source = law.encode(state)
                for steps in (0, 1, 3, 4, 9):
                    before, after = root / "input.bin", root / "output.bin"
                    before.write_bytes(source)
                    subprocess.run([str(binary), "--checkpoint", str(before), str(after), str(steps)],
                                   check=True, timeout=15, capture_output=True)
                    expected = law.decode(source)
                    law.advance(expected, steps)
                    actual = after.read_bytes()
                    if actual != law.encode(expected):
                        raise AssertionError(f"complete-state parity failure L={L}, phase={phase}, steps={steps}")
                    if law.totals(law.decode(actual)) != law.totals(state):
                        raise AssertionError("native accounting changed")
                    count += 1
    return {"law": law.LAW_ID, "table_hash": law.collision_identity(), "complete_state_cases": count,
            "phases": [0, 1, 2, 3], "sizes": [3, 5], "counter_exceeds_js_safe_integer": True,
            "scope": "bounded Python/native parity; no WASM/CUDA or physical recovery claim"}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--equivariant", action='store_true')
    args = parser.parse_args()
    print(json.dumps(verify(args.binary.resolve(), args.equivariant), indent=2))


if __name__ == "__main__":
    main()
