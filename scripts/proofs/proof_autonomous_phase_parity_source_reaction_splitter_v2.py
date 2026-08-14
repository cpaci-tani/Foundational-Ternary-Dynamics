#!/usr/bin/env python3
"""FTD-0888 three-marker repair wrapper for the FTD-0887 certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_AUTONOMOUS_PHASE_PARITY_AND_SOURCE_REACTION_SPLITTER_v1.md"
)
PARENT_CERTIFICATE = ROOT / (
    "scripts/proofs/proof_autonomous_phase_parity_source_reaction_splitter.py"
)
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_AUTONOMOUS_PHASE_PARITY_SOURCE_REACTION_CERTIFICATE_REPAIR_v2.md"
)

PARENT_PROTOCOL_HASH = "484EC4ED25C322D93B44F88267259B81AE510AE659AE22C4366A5DE69635146A"
PARENT_CERTIFICATE_HASH = "814B0AA2E8A555C9F48D9BCAD27C970B07862D1868888A6E0B8C321FEBA97399"
REPAIR_PROTOCOL_HASH = "F2AA1B0239B4BAC4EBBB48DB4976097185EC006CC4AF13B8A8A9602533E61CC1"

REPAIRS = (
    (
        '    "The equilibrium charge `s_0` remains fixed" in protocol_text,',
        '    "The equilibrium charge `s_0` remains fixed" in protocol_flat,',
    ),
    (
        'check("no cross-color commutation is assumed", "No commutation of different-color generators is assumed" in protocol_text)',
        'check("no cross-color commutation is assumed", "No commutation of different-color generators is assumed" in protocol_flat)',
    ),
    (
        'check("complete reaction pair is retained", "complete reaction pair is retained" in protocol_flat and M.det() == 1)',
        'check("complete reaction pair is retained", "complete reaction pair is retained" in protocol_flat and sp.simplify(M.det()) == 1)',
    ),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def fail(message: str) -> None:
    raise SystemExit(f"FTD-0888 REPAIR INVALID: {message}")


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

print("FTD-0888 repair preflight: parent/protocol hashes and three unique anchors PASS")
namespace = {
    "__name__": "__main__",
    "__file__": str(PARENT_CERTIFICATE),
}
exec(compile(source, str(PARENT_CERTIFICATE), "exec"), namespace)
