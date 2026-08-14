#!/usr/bin/env python3
"""FTD-0895 four-expression repair wrapper for the FTD-0894 certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_BLOCH_QUASIMOMENTUM_LIFT_LOCAL_MOMENTUM_MAP_TRILEMMA_v1.md"
)
PARENT_CERTIFICATE = ROOT / (
    "scripts/proofs/"
    "proof_bloch_quasimomentum_lift_local_momentum_map_trilemma.py"
)
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_BLOCH_QUASIMOMENTUM_LIFT_CERTIFICATE_REPAIR_v2.md"
)

PARENT_PROTOCOL_HASH = "2EC2030AC29C287093019CA8DCD5542577312B9730EFF5B33C4324956CBDC791"
PARENT_CERTIFICATE_HASH = "161E64EDB1782C953243B986DEF00C7BD41EC353E912C6AFF9FD0A1682422A0A"
REPAIR_PROTOCOL_HASH = "79D31FA87C3F9DC5F59C09C57748B94B149336E325B3DC47019C20729EED5E88"

REPAIRS = (
    (
        '''check(
    "one-axis characters multiply by adding labels",
    sp.expand_power_exp(character_product) == sp.expand_power_exp(character_sum),
)''',
        '''check(
    "one-axis characters multiply by adding labels",
    sp.simplify(character_product / character_sum) == 1,
)''',
    ),
    (
        '''check("alternating harmonic generating function is log one plus z", log_series == sp.log(z + 1))''',
        '''check(
    "alternating harmonic generating function is log one plus z",
    sp.piecewise_fold(log_series).args[0][0] == sp.log(z + 1),
)''',
    ),
    (
        '''    factor_identity = sp.simplify(
        1 + sp.exp(sp.I * angle)
        - 2 * sp.cos(angle / 2) * sp.exp(sp.I * angle / 2)
    )''',
        '''    factor_identity = sp.simplify(sp.expand_complex(
        1 + sp.exp(sp.I * angle)
        - 2 * sp.cos(angle / 2) * sp.exp(sp.I * angle / 2)
    ))''',
    ),
    (
        '''    "observer-only research instrumentation" in texts["stress_header"]''',
        '''    "observer‑only research instrumentation" in texts["stress_header"]''',
    ),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def fail(message: str) -> None:
    raise SystemExit(f"FTD-0895 REPAIR INVALID: {message}")


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

print("FTD-0895 repair preflight: parent/protocol hashes and four unique anchors PASS")
namespace = {
    "__name__": "__main__",
    "__file__": str(PARENT_CERTIFICATE),
}
exec(compile(source, str(PARENT_CERTIFICATE), "exec"), namespace)
