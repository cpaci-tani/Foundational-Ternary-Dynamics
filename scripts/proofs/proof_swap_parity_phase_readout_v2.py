#!/usr/bin/env python3
"""FTD-0846 verifier-only repair of FTD-0845 C9."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_SCRIPT = ROOT / "scripts/proofs/proof_swap_parity_phase_readout.py"
PARENT_PROTOCOL = (
    ROOT / "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/PREREG_SWAP_PARITY_PHASE_READOUT_v1.md"
)

EXPECTED_SCRIPT = "41E1D1E9043620D20E71A2B18EC72041D5BBC7298133F6C082F9FB877F58FB66"
EXPECTED_PROTOCOL = "0AACC3A6E33CB65DD045CBA82E6BF3ED8F6C522EBCA1B5DD5A217ADCDBDC6054"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


if digest(PARENT_SCRIPT) != EXPECTED_SCRIPT:
    raise SystemExit("FTD-0846 fail closed: invalid parent script hash")
if digest(PARENT_PROTOCOL) != EXPECTED_PROTOCOL:
    raise SystemExit("FTD-0846 fail closed: invalid parent protocol hash")

source = PARENT_SCRIPT.read_text(encoding="utf-8")

old_c9 = "      and sp.factor(W_plus) == kap * (a_ptr - q**2) ** 2 / 2)"
new_c9 = (
    "      and exact_zero(sp.factor(W_plus) "
    "- kap * (a_ptr - q**2) ** 2 / 2))"
)

if source.count(old_c9) != 1:
    raise SystemExit("FTD-0846 fail closed: C9 repair anchor is not unique")

source = source.replace(old_c9, new_c9)

namespace = {
    "__name__": "__main__",
    "__file__": str(PARENT_SCRIPT),
}
exec(compile(source, str(PARENT_SCRIPT), "exec"), namespace)

print("FTD-0846 CERTIFICATE_REPAIR_ONLY_C9_EXACT_SIMPLIFIED_DIFFERENCE")
