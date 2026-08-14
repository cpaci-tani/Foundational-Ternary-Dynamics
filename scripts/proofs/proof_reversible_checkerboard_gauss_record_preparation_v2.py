#!/usr/bin/env python3
"""FTD-0882 one-substitution repair wrapper for the FTD-0881 certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_REVERSIBLE_CHECKERBOARD_GAUSS_RECORD_PREPARATION_v1.md"
)
PARENT_CERTIFICATE = ROOT / (
    "scripts/proofs/proof_reversible_checkerboard_gauss_record_preparation.py"
)
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_REVERSIBLE_CHECKERBOARD_GAUSS_RECORD_PREPARATION_CERTIFICATE_REPAIR_v2.md"
)

PARENT_PROTOCOL_HASH = (
    "50816F74F87D6120C871031D25EF704479B3E4873EB4F108080516C74E298942"
)
PARENT_CERTIFICATE_HASH = (
    "99B570E8E8CFD8FB7474060F3B0114281F2C2F02E92F47BA77E33139414EB634"
)
REPAIR_PROTOCOL_HASH = (
    "BD9E7DB871EEDD590A6CBFADD2B8F07AC38118433DC34925477292D91257B989"
)

OLD = '      and "common affine intersection" in protocol_text)'
NEW = (
    '      and "common affine intersection" in '
    '" ".join(protocol_text.split()))'
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def fail(message: str) -> None:
    raise SystemExit(f"FTD-0882 REPAIR INVALID: {message}")


if sha256(PARENT_PROTOCOL) != PARENT_PROTOCOL_HASH:
    fail("parent protocol hash mismatch")
if sha256(PARENT_CERTIFICATE) != PARENT_CERTIFICATE_HASH:
    fail("parent certificate hash mismatch")
if sha256(REPAIR_PROTOCOL) != REPAIR_PROTOCOL_HASH:
    fail("repair protocol hash mismatch")

source = PARENT_CERTIFICATE.read_text(encoding="utf-8")
if source.count(OLD) != 1:
    fail(f"expected one old C34 anchor, found {source.count(OLD)}")
if source.count(NEW) != 0:
    fail("replacement anchor already present in frozen parent")
repaired = source.replace(OLD, NEW, 1)
if repaired.count(OLD) != 0 or repaired.count(NEW) != 1:
    fail("post-substitution uniqueness check failed")

print("FTD-0882 repair preflight: parent/protocol hashes and unique anchor PASS")
namespace = {
    "__name__": "__main__",
    "__file__": str(PARENT_CERTIFICATE),
}
exec(compile(repaired, str(PARENT_CERTIFICATE), "exec"), namespace)
