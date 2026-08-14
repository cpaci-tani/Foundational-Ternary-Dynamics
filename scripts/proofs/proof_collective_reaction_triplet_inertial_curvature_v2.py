#!/usr/bin/env python3
"""FTD-0892 five-expression repair wrapper for the FTD-0891 certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_COLLECTIVE_REACTION_TRIPLET_AND_INERTIAL_CURVATURE_BOUNDARY_v1.md"
)
PARENT_CERTIFICATE = ROOT / (
    "scripts/proofs/proof_collective_reaction_triplet_inertial_curvature.py"
)
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_COLLECTIVE_REACTION_TRIPLET_INERTIAL_CURVATURE_CERTIFICATE_REPAIR_v2.md"
)

PARENT_PROTOCOL_HASH = "D273F1A61E1A55B26781116E3B9D3984DAFF843DB04F18E160C706EBEAC6C595"
PARENT_CERTIFICATE_HASH = "ED729418595D0B6B0F69F9381CB5DF007DF764E79CDF9145DF15DA9C4B6104FE"
REPAIR_PROTOCOL_HASH = "3036B665B6C8120D13D33A18A25CF8FDA71BA63ADB2A999C9ABC385DD928366B"

REPAIRS = (
    (
        '''check("their momentum curvatures differ for unequal masses",
      sp.simplify(sp.diff(H1, mom, 2) - sp.diff(H2, mom, 2))
      == 1 / m1 - 1 / m2)''',
        '''check("their momentum curvatures differ for unequal masses",
      sp.simplify(sp.diff(H1, mom, 2) - sp.diff(H2, mom, 2)
                  - (1 / m1 - 1 / m2)) == 0)''',
    ),
    (
        '''check("collective triplet is kinematic content not substrate formation",
      "does not derive that action, its bond graph" in block_analysis)''',
        '''check("collective triplet is kinematic content not substrate formation",
      "do not derive that action, its bond graph" in block_analysis)''',
    ),
    (
        '''check("protocol freezes static-data mass non-identifiability",
      "static stability and k do not determine inertia" in protocol_text)''',
        '''protocol_plain = protocol_text.replace("`", "")
check("protocol freezes static-data mass non-identifiability",
      "static stability and k do not determine inertia" in protocol_plain)''',
    ),
    (
        '''check("protocol freezes the discrete-translation Noether boundary",
      "z^3, not r^3" in protocol_text
      and "not make p_matter+p_field" in protocol_text)''',
        '''check("protocol freezes the discrete-translation Noether boundary",
      "z^3, not r^3" in protocol_plain
      and "not make p_matter+p_field" in protocol_plain)''',
    ),
    (
        '''check("Born Bell Gstar Lorentz biology and completeness firewall is frozen",
      "no change to born/bell, g*, lorentz, biology, or completeness status"
      in protocol_text)''',
        '''check("Born Bell Gstar Lorentz biology and completeness firewall is frozen",
      "no change to born/bell, g*, lorentz, biology, or completeness status"
      in protocol_plain)''',
    ),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def fail(message: str) -> None:
    raise SystemExit(f"FTD-0892 REPAIR INVALID: {message}")


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

print("FTD-0892 repair preflight: parent/protocol hashes and five unique anchors PASS")
namespace = {
    "__name__": "__main__",
    "__file__": str(PARENT_CERTIFICATE),
}
exec(compile(source, str(PARENT_CERTIFICATE), "exec"), namespace)
