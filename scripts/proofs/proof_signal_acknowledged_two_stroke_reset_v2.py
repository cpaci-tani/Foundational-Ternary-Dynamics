#!/usr/bin/env python3
"""FTD-0869 verifier-only repair for the FTD-0868 exact certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT = ROOT / "scripts/proofs/proof_signal_acknowledged_two_stroke_reset.py"
PARENT_SHA256 = "FAC4A5BA4095742CFCFC8DB4DDE145E942E29254417EB87645648458B643E9FB"

OLD_C21 = (
    "sp.solve_univariate_inequality(action0 - B > 0, action0)\n"
    "          == sp.Interval.open(B, sp.oo)"
)
NEW_C21 = (
    "sp.solve_univariate_inequality(action0 - B > 0, action0)\n"
    "          == (B < action0)"
)
OLD_C27 = "int(partial_matter_energy == 0) * int(partial_signal_energy > 0)"
NEW_C27 = (
    "int(bool(partial_matter_energy == 0)) "
    "* int(bool(partial_signal_energy > 0))"
)


def main() -> int:
    source_bytes = PARENT.read_bytes()
    digest = hashlib.sha256(source_bytes).hexdigest().upper()
    if digest != PARENT_SHA256:
        print("FAIL  FTD-0868 parent script hash mismatch")
        return 1

    source = source_bytes.decode("utf-8")
    if source.count(OLD_C21) != 1 or NEW_C21 in source:
        print("FAIL  C21 permitted replacement count is not exactly one")
        return 1
    if source.count(OLD_C27) != 1 or NEW_C27 in source:
        print("FAIL  C27 permitted replacement count is not exactly one")
        return 1

    repaired = source.replace(OLD_C21, NEW_C21, 1)
    repaired = repaired.replace(OLD_C27, NEW_C27, 1)
    namespace = {
        "__name__": "ftd0868_repaired_in_memory",
        "__file__": str(PARENT),
    }
    exec(compile(repaired, str(PARENT), "exec"), namespace)
    result = int(namespace["main"]())
    if result != 0:
        print("FTD-0869 certificate repair: inherited certificate failed")
        return result

    print("FTD-0869 certificate repair: PASS")
    print("REPAIR_SCOPE=C21_RELATIONAL_REPRESENTATION_PLUS_C27_BOOLEAN_CAST_ONLY")
    print("PARENT_FTD0868_PRESERVED_INVALID")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
