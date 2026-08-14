#!/usr/bin/env python3
"""FTD-0900 one-substitution repair for the FTD-0899 certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_COMMON_RELATIVE_CONNECTION_MOMENTUM_GEARBOX_BOUNDARY_v1.md"
)
PARENT_CERTIFICATE = ROOT / (
    "scripts/proofs/"
    "proof_common_relative_connection_momentum_gearbox_boundary.py"
)
EXPECTED_PARENT_PROTOCOL_HASH = (
    "38B7B6C929CC10F3F296FBA56A36478790D5AD648F8F9D2603058EE58F245AA0"
)
EXPECTED_PARENT_CERTIFICATE_HASH = (
    "75426CCCE016C6471583BB65FD2D9C608D27AE871C44643F1A224F2C867176AB"
)

OLD = 'return sp.Matrix(sp.symbols(f"{prefix}0:3", real=True))'
NEW = 'return sp.Matrix(sp.symbols(f"{prefix}_0:3", real=True))'


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def gate(name: str, condition: bool) -> None:
    print(f"{'PASS' if condition else 'FAIL'}  REPAIR {name}")
    if not condition:
        raise SystemExit(1)


gate("parent protocol hash", sha256(PARENT_PROTOCOL) == EXPECTED_PARENT_PROTOCOL_HASH)
gate(
    "parent certificate hash",
    sha256(PARENT_CERTIFICATE) == EXPECTED_PARENT_CERTIFICATE_HASH,
)
source = PARENT_CERTIFICATE.read_text(encoding="utf-8")
gate("old symbol-generator form occurs exactly once", source.count(OLD) == 1)
gate("new symbol-generator form is absent before repair", source.count(NEW) == 0)
repaired = source.replace(OLD, NEW, 1)
gate("repair changes exactly one symbol-generator form", repaired.count(OLD) == 0 and repaired.count(NEW) == 1)

namespace = {
    "__name__": "__main__",
    "__file__": str(PARENT_CERTIFICATE),
}
try:
    exec(compile(repaired, str(PARENT_CERTIFICATE), "exec"), namespace)
except SystemExit as exc:
    code = int(exc.code or 0)
    gate("inherited parent certificate exits zero", code == 0)
else:
    gate("inherited parent certificate exits zero", True)

print("FTD-0900_REPAIR_SCOPE=SYMPY_COMPONENT_SYMBOL_SEPARATOR_ONLY")
print("FTD-0900_PARENT_EQUATIONS_THRESHOLDS_SOURCES_OUTCOMES_UNCHANGED=TRUE")
