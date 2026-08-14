#!/usr/bin/env python3
"""FTD-0923 verifier-only repair wrapper for the FTD-0922 certificate."""

from __future__ import annotations

from hashlib import sha256
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT = ROOT / "scripts/proofs/proof_ternary_dipole_core_evanescent_c4_reference_orbit.py"
REPAIR_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_TERNARY_DIPOLE_CORE_EVANESCENT_C4_CERTIFICATE_REPAIR_v2.md"
)
PARENT_HASH = "2FEC105772F6396E49C3E2C47ADA2F2792438C7ADACF64D68AC4BE38C73CECEE"
REPAIR_PROTOCOL_HASH = "E4C8AD09EAEC580D6BD5C34588F293AAE8E8762D17331756252E5138CA371637"


def digest(path: Path) -> str:
    return sha256(path.read_bytes()).hexdigest().upper()


if digest(PARENT) != PARENT_HASH:
    raise SystemExit("FTD-0923 invalid: frozen FTD-0922 parent certificate drift")
if digest(REPAIR_PROTOCOL) != REPAIR_PROTOCOL_HASH:
    raise SystemExit("FTD-0923 invalid: repair protocol drift")

source = PARENT.read_text(encoding="utf-8")
old = '''    source_support = [sum(vector != ZERO3 for vector in grad.values()) for grad in gradients]
    source_norms = [dot(grad, grad) for grad in gradients]
    check("central dipole gradient has exactly eleven vector-support sites", source_support == [11, 11, 11, 11])
    check("central dipole gradient norm squared is seven halves", source_norms == [sp.Rational(7, 2)] * 4)
'''
new = '''    # FTD-0923 verifier-only repair: the two locked compact-source
    # values are uncontained-lattice statements. Keep the periodic gradients
    # above for every L=4 covariance/resolvent/orbit arm, but compute these two
    # values on the exact finite uncontained candidate set.
    infinite_source = {(1, 0, 0): sp.Integer(1), (-1, 0, 0): sp.Integer(-1)}
    axes = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
    candidates = set()
    for source_point in infinite_source:
        for axis in axes:
            candidates.add(tuple(source_point[i] + axis[i] for i in range(3)))
            candidates.add(tuple(source_point[i] - axis[i] for i in range(3)))
    infinite_gradient = {}
    for point in candidates:
        components = []
        for axis in axes:
            plus = tuple(point[i] + axis[i] for i in range(3))
            minus = tuple(point[i] - axis[i] for i in range(3))
            components.append(
                sp.Rational(1, 2)
                * (infinite_source.get(plus, 0) - infinite_source.get(minus, 0))
            )
        infinite_gradient[point] = sp.Matrix(components)
    source_support = sum(vector != ZERO3 for vector in infinite_gradient.values())
    source_norm = sp.simplify(
        sum((vector.T * vector)[0] for vector in infinite_gradient.values())
    )
    check("central dipole gradient has exactly eleven vector-support sites", source_support == 11)
    check("central dipole gradient norm squared is seven halves", source_norm == sp.Rational(7, 2))
'''

if source.count(old) != 1:
    raise SystemExit("FTD-0923 invalid: permitted parent block not found exactly once")

repaired = source.replace(old, new, 1)
namespace = {"__name__": "__main__", "__file__": str(PARENT)}
exec(compile(repaired, str(PARENT), "exec"), namespace)
