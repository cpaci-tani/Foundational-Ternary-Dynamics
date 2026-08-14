#!/usr/bin/env python3
"""FTD-0848 verifier-only repair of FTD-0847 C25/C28 comparisons."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_SCRIPT = ROOT / "scripts/proofs/proof_loss_booked_ternary_phase_latch.py"
PARENT_PROTOCOL = (
    ROOT / "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/PREREG_LOSS_BOOKED_TERNARY_PHASE_LATCH_v1.md"
)

EXPECTED_SCRIPT = "8C0D60C2B0624FC58BA00B9B4A76DA1B641C37D9E4873D991D9ED89CD30103CE"
EXPECTED_PROTOCOL = "B559BD68C7FF3E8D20431A753433A9C431A68F3363623D74660A25E5AEE0D6CD"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


if digest(PARENT_SCRIPT) != EXPECTED_SCRIPT:
    raise SystemExit("FTD-0848 fail closed: invalid parent script hash")
if digest(PARENT_PROTOCOL) != EXPECTED_PROTOCOL:
    raise SystemExit("FTD-0848 fail closed: invalid parent protocol hash")

source = PARENT_SCRIPT.read_text(encoding="utf-8")

old_rho = """def rho(value: sp.Expr) -> int:
    if bool(value < -theta):
        return -1
    if bool(value > theta):
        return 1
    return 0
"""
new_rho = """def rho(value: sp.Expr) -> int:
    normalized = sp.simplify(value / theta)
    if sp.ask(sp.Q.negative(normalized + 1)) is True:
        return -1
    if sp.ask(sp.Q.positive(normalized - 1)) is True:
        return 1
    return 0
"""

if source.count(old_rho) != 1:
    raise SystemExit("FTD-0848 fail closed: rho repair anchor is not unique")

source = source.replace(old_rho, new_rho)

namespace = {
    "__name__": "__main__",
    "__file__": str(PARENT_SCRIPT),
}
exec(compile(source, str(PARENT_SCRIPT), "exec"), namespace)

print("FTD-0848 CERTIFICATE_REPAIR_ONLY_C25_C28_NORMALIZED_EXACT_ORDERING")
