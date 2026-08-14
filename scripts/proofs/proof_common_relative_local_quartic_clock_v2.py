#!/usr/bin/env python3
"""FTD-0844 verifier-only repair of FTD-0843 C14/C28."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_SCRIPT = ROOT / "scripts/proofs/proof_common_relative_local_quartic_clock.py"
PARENT_PROTOCOL = (
    ROOT / "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_v1.md"
)

EXPECTED_SCRIPT = "D5CCC53504E162D9999AAAE7F0142F7FD8EA98DBE153328059A6672C79B68076"
EXPECTED_PROTOCOL = "050EAC8DB2BDC0A7AA2116874F7F43A4F08D6246703004BB8C4573A0795A6F79"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


if digest(PARENT_SCRIPT) != EXPECTED_SCRIPT:
    raise SystemExit("FTD-0844 fail closed: invalid parent script hash")
if digest(PARENT_PROTOCOL) != EXPECTED_PROTOCOL:
    raise SystemExit("FTD-0844 fail closed: invalid parent protocol hash")

source = PARENT_SCRIPT.read_text(encoding="utf-8")

old_c14 = (
    'check("C14 common quadratic tick invariant is exact", '
    'U.T * G * U == G)'
)
new_c14 = (
    'check("C14 common quadratic tick invariant is exact", '
    'all(zero(entry) for entry in (U.T * G * U - G)))'
)

old_c28 = "    and U.T * G * U == G,"
new_c28 = "    and all(zero(entry) for entry in (U.T * G * U - G)),"

if source.count(old_c14) != 1 or source.count(old_c28) != 1:
    raise SystemExit("FTD-0844 fail closed: repair anchors are not unique")

source = source.replace(old_c14, new_c14)
source = source.replace(old_c28, new_c28)

namespace = {
    "__name__": "__main__",
    "__file__": str(PARENT_SCRIPT),
}
exec(compile(source, str(PARENT_SCRIPT), "exec"), namespace)

print("FTD-0844 CERTIFICATE_REPAIR_ONLY_C14_C28_EXACT_SIMPLIFIED_DIFFERENCE")

