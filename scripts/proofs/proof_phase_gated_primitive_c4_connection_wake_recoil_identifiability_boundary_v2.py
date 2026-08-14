#!/usr/bin/env python3
"""FTD-0938 one-comparison repair for the FTD-0937 certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT = ROOT / "scripts/proofs/proof_phase_gated_primitive_c4_connection_wake_recoil_identifiability_boundary.py"
EXPECTED_PARENT_HASH = "FB14DD1CB379C38AAC9A241E4E2373E2CE511F8C988139ED47EBD50F2315057D"

OLD = "delta_mechanical == -gate * gamma * (q1**2 - q0**2) * u"
NEW = "sp.simplify(delta_mechanical + gate * gamma * (q1**2 - q0**2) * u) == sp.zeros(3, 1)"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def gate(name: str, condition: bool) -> None:
    print(f"{'PASS' if condition else 'FAIL'}  REPAIR {name}")
    if not condition:
        raise SystemExit(1)


gate("FTD-0937 parent certificate hash", sha256(PARENT) == EXPECTED_PARENT_HASH)
source = PARENT.read_text(encoding="utf-8")
gate("old comparison occurs exactly once", source.count(OLD) == 1)
gate("zero-residual comparison is absent before repair", source.count(NEW) == 0)

repaired = source.replace(OLD, NEW, 1)
gate("old comparison is absent after repair", repaired.count(OLD) == 0)
gate("zero-residual comparison occurs exactly once after repair", repaired.count(NEW) == 1)

namespace = {"__name__": "__main__", "__file__": str(PARENT)}
try:
    exec(compile(repaired, str(PARENT), "exec"), namespace)
except SystemExit as exc:
    code = int(exc.code or 0)
    gate("inherited parent certificate exits zero", code == 0)
else:
    gate("inherited parent certificate exits zero", True)

print("FTD-0938_REPAIR_COUNT=EXACTLY_ONE")
print("FTD-0938_REPAIR_SCOPE=SYMPY_MATRIX_ZERO_RESIDUAL_NORMALIZATION")
print("FTD-0938_MATHEMATICS_THRESHOLDS_SOURCES_OUTCOMES_UNCHANGED=TRUE")
