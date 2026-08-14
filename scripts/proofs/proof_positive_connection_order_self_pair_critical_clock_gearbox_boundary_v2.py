#!/usr/bin/env python3
"""FTD-0903 one-substitution representation repair for FTD-0902."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_POSITIVE_CONNECTION_ORDER_SELF_PAIR_CERTIFICATE_REPAIR_v2.md"
PARENT = ROOT / "scripts/proofs/proof_positive_connection_order_self_pair_critical_clock_gearbox_boundary.py"
EXPECTED_PROTOCOL_HASH = "3C7E31BF8160EDCC8D8721EA0021A051AD5F5C502010FB1014D8DB1FBC03AFE7"
EXPECTED_PARENT_HASH = "C56907311B93942ABD7CD3DA96882CDC811EA333526C11C30F9C7BE004EB107C"

OLD = "H_expected.subs({P: 0, u**2: D**4})"
NEW = "H_expected.subs(P, 0).subs(u**2, D**4)"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def gate(name: str, condition: bool) -> None:
    print(f"{'PASS' if condition else 'FAIL'}  REPAIR {name}")
    if not condition:
        raise SystemExit(1)


gate("FTD-0903 protocol hash", sha256(PROTOCOL) == EXPECTED_PROTOCOL_HASH)
gate("FTD-0902 parent certificate hash", sha256(PARENT) == EXPECTED_PARENT_HASH)
source = PARENT.read_text(encoding="utf-8")
gate("old C32 form occurs exactly once", source.count(OLD) == 1)
gate("new C32 form is absent before repair", source.count(NEW) == 0)

repaired = source.replace(OLD, NEW, 1)
gate("old C32 form is absent after repair", repaired.count(OLD) == 0)
gate("new C32 form occurs exactly once after repair", repaired.count(NEW) == 1)

namespace = {"__name__": "__main__", "__file__": str(PARENT)}
try:
    exec(compile(repaired, str(PARENT), "exec"), namespace)
except SystemExit as exc:
    code = int(exc.code or 0)
    gate("inherited parent certificate exits zero", code == 0)
else:
    gate("inherited parent certificate exits zero", True)

print("FTD-0903_REPAIR_COUNT=EXACTLY_ONE")
print("FTD-0903_MATHEMATICS_THRESHOLDS_SOURCES_OUTCOMES_UNCHANGED=TRUE")
