#!/usr/bin/env python3
"""FTD-0906 one-marker repair for the frozen FTD-0905 certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_NATIVE_TERNARY_DIPOLE_PHASE_WEDGE_CERTIFICATE_REPAIR_v2.md"
PARENT = ROOT / "scripts/proofs/proof_native_ternary_dipole_axis_bilateral_phase_wedge_memory_boundary.py"
EXPECTED_PROTOCOL_HASH = "F3758EECECACFD92CB35DFD501868F0C72CE3AAA7ADB77AA8826029B2C1F1340"
EXPECTED_PARENT_HASH = "FAA3CD3635C048AAD95E312AE59D6B725444C7C55571A0913A864F8AC8E038F0"

OLD = '"every nonzero discrete step has one strict orientation" in source_text["pair_theorem"]'
NEW = '"every nonzero discrete step has one strict orientation" in " ".join(source_text["pair_theorem"].split())'


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def gate(name: str, condition: bool) -> None:
    print(f"{'PASS' if condition else 'FAIL'}  REPAIR {name}")
    if not condition:
        raise SystemExit(1)


gate("FTD-0906 protocol hash", sha256(PROTOCOL) == EXPECTED_PROTOCOL_HASH)
gate("FTD-0905 parent certificate hash", sha256(PARENT) == EXPECTED_PARENT_HASH)
source = PARENT.read_text(encoding="utf-8")
gate("old C38 marker form occurs exactly once", source.count(OLD) == 1)
gate("new C38 marker form is absent before repair", source.count(NEW) == 0)

repaired = source.replace(OLD, NEW, 1)
gate("old C38 marker form is absent after repair", repaired.count(OLD) == 0)
gate("new C38 marker form occurs exactly once after repair", repaired.count(NEW) == 1)

namespace = {"__name__": "__main__", "__file__": str(PARENT)}
try:
    exec(compile(repaired, str(PARENT), "exec"), namespace)
except SystemExit as exc:
    code = int(exc.code or 0)
    gate("inherited parent certificate exits zero", code == 0)
else:
    gate("inherited parent certificate exits zero", True)

print("FTD-0906_REPAIR_COUNT=EXACTLY_ONE")
print("FTD-0906_MATHEMATICS_THRESHOLDS_SOURCES_OUTCOMES_UNCHANGED=TRUE")
