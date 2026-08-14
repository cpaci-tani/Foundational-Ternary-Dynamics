#!/usr/bin/env python3
"""FTD-0907 one-marker repair of the frozen FTD-0906 wrapper."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_NATIVE_TERNARY_DIPOLE_PHASE_WEDGE_SOURCE_MARKER_REPAIR_v3.md"
PARENT_WRAPPER = ROOT / "scripts/proofs/proof_native_ternary_dipole_axis_bilateral_phase_wedge_memory_boundary_v2.py"
EXPECTED_PROTOCOL_HASH = "E6B1B158B525D83036D1C78AB68AC5435542C10E60999EF399AF580A3376EE96"
EXPECTED_PARENT_WRAPPER_HASH = "4608E92745BCB047AA18BBB8B5EE8DDB7C825E9D2B4DCD0A2148F7B0EBD53E8B"

OLD = '"every nonzero discrete step has one strict orientation" in " ".join(source_text["pair_theorem"].split())'
NEW = '"on every nonzero step" in " ".join(source_text["pair_theorem"].split())'


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def gate(name: str, condition: bool) -> None:
    print(f"{'PASS' if condition else 'FAIL'}  REPAIR {name}")
    if not condition:
        raise SystemExit(1)


gate("FTD-0907 protocol hash", sha256(PROTOCOL) == EXPECTED_PROTOCOL_HASH)
gate("FTD-0906 parent wrapper hash", sha256(PARENT_WRAPPER) == EXPECTED_PARENT_WRAPPER_HASH)
source = PARENT_WRAPPER.read_text(encoding="utf-8")
gate("incorrect C38 proposal occurs exactly once", source.count(OLD) == 1)
gate("actual-source C38 proposal is absent before repair", source.count(NEW) == 0)

repaired = source.replace(OLD, NEW, 1)
gate("incorrect C38 proposal is absent after repair", repaired.count(OLD) == 0)
gate("actual-source C38 proposal occurs exactly once after repair", repaired.count(NEW) == 1)

namespace = {"__name__": "__main__", "__file__": str(PARENT_WRAPPER)}
try:
    exec(compile(repaired, str(PARENT_WRAPPER), "exec"), namespace)
except SystemExit as exc:
    code = int(exc.code or 0)
    gate("inherited repair wrapper and parent certificate exit zero", code == 0)
else:
    gate("inherited repair wrapper and parent certificate exit zero", True)

print("FTD-0907_REPAIR_COUNT=EXACTLY_ONE")
print("FTD-0907_MATHEMATICS_THRESHOLDS_SOURCES_OUTCOMES_UNCHANGED=TRUE")
