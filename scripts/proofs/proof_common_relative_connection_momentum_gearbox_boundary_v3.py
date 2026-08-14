#!/usr/bin/env python3
"""FTD-0901 representation-only repair for the FTD-0899 certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT = ROOT / "scripts/proofs/proof_common_relative_connection_momentum_gearbox_boundary.py"
EXPECTED_PARENT_HASH = "75426CCCE016C6471583BB65FD2D9C608D27AE871C44643F1A224F2C867176AB"

REPAIRS = (
    (
        'return sp.Matrix(sp.symbols(f"{prefix}0:3", real=True))',
        'return sp.Matrix(sp.symbols(f"{prefix}_0:3", real=True))',
    ),
    (
        '"add the lowest-degree p4-local common--relative exchange" in texts["common_relative"],',
        '"add the lowest-degree p4-local common--relative exchange" in " ".join(texts["common_relative"].split()),',
    ),
    (
        '"would require a new combined common-action/energy proof" in texts["odd_pointer"],',
        '"would require a new combined common-action/energy proof" in " ".join(texts["odd_pointer"].split()),',
    ),
    (
        '"i supplies orientation" in texts["action_transducer"]',
        '"`i` supplies orientation" in texts["action_transducer"]',
    ),
    (
        '"native vector common action=open" in texts["reaction_transport"].replace(" ", ""),',
        '"native_vector_common_action=open" in texts["reaction_transport"].replace(" ", ""),',
    ),
    (
        '"not a globally hamiltonian time map on the phase cylinder" in texts["phase_boundary"],',
        '"not a globally hamiltonian time map on the phase cylinder" in " ".join(texts["phase_boundary"].split()),',
    ),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def gate(name: str, condition: bool) -> None:
    print(f"{'PASS' if condition else 'FAIL'}  REPAIR {name}")
    if not condition:
        raise SystemExit(1)


gate("FTD-0899 parent certificate hash", sha256(PARENT) == EXPECTED_PARENT_HASH)
source = PARENT.read_text(encoding="utf-8")
for index, (old, new) in enumerate(REPAIRS, start=1):
    gate(f"old form {index} occurs exactly once", source.count(old) == 1)
    gate(f"new form {index} is absent before repair", source.count(new) == 0)

repaired = source
for old, new in REPAIRS:
    repaired = repaired.replace(old, new, 1)

for index, (old, new) in enumerate(REPAIRS, start=1):
    gate(
        f"repair {index} applied exactly once",
        repaired.count(old) == 0 and repaired.count(new) == 1,
    )

namespace = {"__name__": "__main__", "__file__": str(PARENT)}
try:
    exec(compile(repaired, str(PARENT), "exec"), namespace)
except SystemExit as exc:
    code = int(exc.code or 0)
    gate("inherited parent certificate exits zero", code == 0)
else:
    gate("inherited parent certificate exits zero", True)

print("FTD-0901_REPAIR_COUNT=EXACTLY_SIX")
print("FTD-0901_MATHEMATICS_THRESHOLDS_SOURCES_OUTCOMES_UNCHANGED=TRUE")
