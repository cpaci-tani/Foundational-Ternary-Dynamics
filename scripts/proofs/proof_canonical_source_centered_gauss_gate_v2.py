#!/usr/bin/env python3
"""FTD-0886 three-marker repair wrapper for the FTD-0885 certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_OBSTRUCTION_v1.md"
)
PARENT_CERTIFICATE = ROOT / "scripts/proofs/proof_canonical_source_centered_gauss_gate.py"
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_CERTIFICATE_REPAIR_v2.md"
)

PARENT_PROTOCOL_HASH = "70000AF7DA0ACA89F92A593AA4B6A759B9C9D08C65E29E21A2D1EF5B2B2910D7"
PARENT_CERTIFICATE_HASH = "7DC08CF572BF58BC37152F985608EB45A7F11C6308165D8D94F1B0A5B55D248E"
REPAIR_PROTOCOL_HASH = "428D1C37EF2510235387C1E0D71BD0DDF489CE58AEE1C4E34A10B3E978A26B3C"

REPAIRS = (
    (
        '    "Production, `G*`, Born, Bell, Lorentz, biology, and completeness" in protocol_flat,',
        '    "production reservoir, a `G*` gearbox, Born recovery, Bell recovery, Lorentz hiding, or framework completeness" in protocol_flat,',
    ),
    (
        'check("fresh canonical port is the complete zero pair", "A fresh port is `(0,0)`" in protocol_text)',
        'check("fresh canonical port is the complete zero pair", "A fresh port is `(0,0)`" in protocol_flat)',
    ),
    (
        '    "Production and quartic-`G*` synchronization remain separate" in protocol_flat,',
        '    "production and quartic-`G*` synchronization remain separate" in protocol_flat,',
    ),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def fail(message: str) -> None:
    raise SystemExit(f"FTD-0886 REPAIR INVALID: {message}")


if sha256(PARENT_PROTOCOL) != PARENT_PROTOCOL_HASH:
    fail("parent protocol hash mismatch")
if sha256(PARENT_CERTIFICATE) != PARENT_CERTIFICATE_HASH:
    fail("parent certificate hash mismatch")
if sha256(REPAIR_PROTOCOL) != REPAIR_PROTOCOL_HASH:
    fail("repair protocol hash mismatch")

source = PARENT_CERTIFICATE.read_text(encoding="utf-8")
for index, (old, new) in enumerate(REPAIRS, start=1):
    if source.count(old) != 1:
        fail(f"R{index} expected one old anchor, found {source.count(old)}")
    if source.count(new) != 0:
        fail(f"R{index} replacement already present in frozen parent")
    source = source.replace(old, new, 1)
    if source.count(old) != 0 or source.count(new) != 1:
        fail(f"R{index} post-substitution uniqueness check failed")

print("FTD-0886 repair preflight: parent/protocol hashes and three unique anchors PASS")
namespace = {
    "__name__": "__main__",
    "__file__": str(PARENT_CERTIFICATE),
}
exec(compile(source, str(PARENT_CERTIFICATE), "exec"), namespace)
