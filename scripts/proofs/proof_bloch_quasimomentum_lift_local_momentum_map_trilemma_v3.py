#!/usr/bin/env python3
"""FTD-0896 one-marker repair wrapper for the FTD-0895 wrapper."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_BLOCH_QUASIMOMENTUM_LIFT_CERTIFICATE_REPAIR_v2.md"
)
PARENT_WRAPPER = ROOT / (
    "scripts/proofs/"
    "proof_bloch_quasimomentum_lift_local_momentum_map_trilemma_v2.py"
)
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_BLOCH_QUASIMOMENTUM_LIFT_SOURCE_MARKER_REPAIR_v3.md"
)

PARENT_REPAIR_PROTOCOL_HASH = "79D31FA87C3F9DC5F59C09C57748B94B149336E325B3DC47019C20729EED5E88"
PARENT_WRAPPER_HASH = "C4FCFD2BABF29FA09811BA68D5B9B96D6AA1B4B5CAF6E87C94A0CC512832E4FD"
REPAIR_PROTOCOL_HASH = "F32B412704493453ADFE9B498DF740F2743B9624A3F5385F81ADD6AD8220B6FE"

OLD = '''        ''' + "'''" + '''    "observer‑only research instrumentation" in texts["stress_header"]''' + "'''" + ''','''
NEW = '''        ''' + "'''" + '''    "observer-only research * instrumentation" in texts["stress_header"]''' + "'''" + ''','''


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def fail(message: str) -> None:
    raise SystemExit(f"FTD-0896 REPAIR INVALID: {message}")


if sha256(PARENT_REPAIR_PROTOCOL) != PARENT_REPAIR_PROTOCOL_HASH:
    fail("parent repair protocol hash mismatch")
if sha256(PARENT_WRAPPER) != PARENT_WRAPPER_HASH:
    fail("parent wrapper hash mismatch")
if sha256(REPAIR_PROTOCOL) != REPAIR_PROTOCOL_HASH:
    fail("repair protocol hash mismatch")

source = PARENT_WRAPPER.read_text(encoding="utf-8")
if source.count(OLD) != 1:
    fail(f"expected one old C73 anchor, found {source.count(OLD)}")
if source.count(NEW) != 0:
    fail("new C73 anchor already exists in frozen parent")
source = source.replace(OLD, NEW, 1)
if source.count(OLD) != 0 or source.count(NEW) != 1:
    fail("post-substitution uniqueness check failed")

print("FTD-0896 repair preflight: parent hashes and unique C73 anchor PASS")
namespace = {
    "__name__": "__main__",
    "__file__": str(PARENT_WRAPPER),
}
exec(compile(source, str(PARENT_WRAPPER), "exec"), namespace)
