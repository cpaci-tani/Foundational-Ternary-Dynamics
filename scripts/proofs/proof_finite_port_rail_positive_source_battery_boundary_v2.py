#!/usr/bin/env python3
"""FTD-0884 one-substitution repair wrapper for the FTD-0883 certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_BOUNDARY_v1.md"
)
PARENT_CERTIFICATE = ROOT / (
    "scripts/proofs/proof_finite_port_rail_positive_source_battery_boundary.py"
)
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_CERTIFICATE_REPAIR_v2.md"
)

PARENT_PROTOCOL_HASH = (
    "0B6ACD3C1E41B4D1EE60CCA9A5E04E91E84FC96F06A3725B1F41DDDFD79E8C0B"
)
PARENT_CERTIFICATE_HASH = (
    "9596738C5FA23964CDEE234BD73E1A48B658516D931B5E92CC085118D90DD02B"
)
REPAIR_PROTOCOL_HASH = (
    "13B9456E1DCF188DB26BDA7D6816FB88CEDADF1F59653841CE5FAA289BD4BDE8"
)

OLD = '    "explicit one-vector-per-port cyclic representation" in protocol_text,'
NEW = (
    '    "explicit one-vector-per-port cyclic representation"\n'
    '    in " ".join(protocol_text.split()),'
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def fail(message: str) -> None:
    raise SystemExit(f"FTD-0884 REPAIR INVALID: {message}")


if sha256(PARENT_PROTOCOL) != PARENT_PROTOCOL_HASH:
    fail("parent protocol hash mismatch")
if sha256(PARENT_CERTIFICATE) != PARENT_CERTIFICATE_HASH:
    fail("parent certificate hash mismatch")
if sha256(REPAIR_PROTOCOL) != REPAIR_PROTOCOL_HASH:
    fail("repair protocol hash mismatch")

source = PARENT_CERTIFICATE.read_text(encoding="utf-8")
if source.count(OLD) != 1:
    fail(f"expected one old C3 anchor, found {source.count(OLD)}")
if source.count(NEW) != 0:
    fail("replacement anchor already present in frozen parent")
repaired = source.replace(OLD, NEW, 1)
if repaired.count(OLD) != 0 or repaired.count(NEW) != 1:
    fail("post-substitution uniqueness check failed")

print("FTD-0884 repair preflight: parent/protocol hashes and unique anchor PASS")
namespace = {
    "__name__": "__main__",
    "__file__": str(PARENT_CERTIFICATE),
}
exec(compile(repaired, str(PARENT_CERTIFICATE), "exec"), namespace)

