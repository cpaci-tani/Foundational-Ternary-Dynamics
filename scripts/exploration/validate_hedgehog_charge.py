"""Validate the Berg-Luscher discrete topological charge formula on FTD's
octahedral (k=1 Moore-shell) geometry before touching the engine.

Berg-Luscher (Nucl.Phys. B190 (1981) 412): for a triangle of three unit
vectors (n_i, n_j, n_k) on S^2, the signed solid angle they subtend is

    tan(Omega/2) = (n_i . (n_j x n_k)) / (1 + n_i.n_j + n_j.n_k + n_k.n_i)

Total topological (hedgehog/skyrmion) charge for a closed triangulated
surface = (1/4pi) * sum of signed solid angles over all triangles.

FTD's octahedral shell (Moore k=1 layer, 6 face-neighbors at +-x,+-y,+-z)
triangulates naturally into 8 faces (one per octant sign combination),
giving a closed genus-0 surface enclosing the center voxel -- this is
the innermost closed shell in FTD's own Moore Layer Theorem decomposition,
not an arbitrary choice.

Referenced by docs/theory/03_derivations/foundational_mechanics/
DERIV_REST_MASS_FROM_TOPOLOGICAL_CHARGE.md section 2. This script does
NOT touch the engine -- it only checks the formula gives textbook-correct
answers on hand-constructed fields with known degree, and specifically
checks rotation- and magnitude-invariance (the properties that motivate
using this as a circumstance-independent candidate where energy failed).

Usage:
    python scripts/exploration/validate_hedgehog_charge.py
"""

import math
import numpy as np

OCTAHEDRON_VERTICES = {
    "+x": np.array([1, 0, 0]),
    "-x": np.array([-1, 0, 0]),
    "+y": np.array([0, 1, 0]),
    "-y": np.array([0, -1, 0]),
    "+z": np.array([0, 0, 1]),
    "-z": np.array([0, 0, -1]),
}

# 8 faces of the octahedron: one per choice of (sign_x, sign_y, sign_z).
# Orientation must be consistent (all outward-pointing normals) or signed
# solid angles cancel pairwise instead of summing -- verified by direct
# cross-product check: (x,y,z) order is outward for an EVEN number of
# minus signs, and needs one swap (y,x order) for an ODD number.
def _face(sx, sy, sz):
    xv, yv, zv = f"{sx}x", f"{sy}y", f"{sz}z"
    n_minus = [sx, sy, sz].count("-")
    return (xv, zv, yv) if n_minus % 2 == 1 else (xv, yv, zv)

OCTAHEDRON_FACES = [
    _face(sx, sy, sz)
    for sx in ("+", "-") for sy in ("+", "-") for sz in ("+", "-")
]


def solid_angle(n_i, n_j, n_k):
    """Berg-Luscher signed solid angle for one triangle (unit vectors)."""
    numerator = np.dot(n_i, np.cross(n_j, n_k))
    denominator = 1.0 + np.dot(n_i, n_j) + np.dot(n_j, n_k) + np.dot(n_k, n_i)
    return 2.0 * math.atan2(numerator, denominator)


def hedgehog_charge(direction_field):
    """direction_field: dict from octahedron vertex label -> normalized J."""
    total = 0.0
    for (a, b, c) in OCTAHEDRON_FACES:
        total += solid_angle(direction_field[a], direction_field[b], direction_field[c])
    return total / (4.0 * math.pi)


def normalize(v):
    n = np.linalg.norm(v)
    if n < 1e-12:
        raise ValueError("field vanishes at a shell vertex -- charge undefined here")
    return v / n


def test_case(name, flux_at_vertex_fn, expected):
    field = {label: normalize(flux_at_vertex_fn(vec)) for label, vec in OCTAHEDRON_VERTICES.items()}
    q = hedgehog_charge(field)
    ok = abs(q - expected) < 1e-6
    print(f"  {name:32s} Q = {q:+.6f}   expected {expected:+d}   [{'PASS' if ok else 'FAIL'}]")
    return ok


def main():
    print("=" * 68)
    print("Berg-Luscher discrete topological charge -- validation on")
    print("FTD's octahedral (Moore k=1) shell, hand-constructed fields")
    print("=" * 68)
    results = []

    # J(x) = x  (pure radial, the natural Coulomb/point-charge shape) -> Q=+1
    results.append(test_case("radial hedgehog  J = r_hat", lambda v: v, 1))

    # J(x) = -x (anti-radial / sink) -> Q = -1
    results.append(test_case("anti-hedgehog    J = -r_hat", lambda v: -v, -1))

    # J(x) = (1,0,0) constant direction everywhere -> Q = 0 (degenerate map,
    # not covering the sphere; formula should return ~0, and at least one
    # triangle is degenerate (repeated vertex direction) so this also
    # stresses the atan2 branch handling)
    try:
        results.append(test_case("uniform          J = +x_hat", lambda v: np.array([1.0, 0.0, 0.0]), 0))
    except ValueError as e:
        print(f"  uniform field: {e} (expected -- degenerate map, skip)")

    # A genuinely non-degenerate but still direction-independent-ish probe:
    # tilt every vertex's direction by mixing in a fixed offset, still
    # topologically trivial (small perturbation of a constant map) -> Q=0
    def tilted_constant(v):
        base = np.array([0.2, 0.3, 0.9])
        return base + 0.05 * v
    results.append(test_case("tilted-constant (trivial)", tilted_constant, 0))

    # Rotate the pure radial field by a fixed rotation matrix -- degree is
    # rotation-invariant, must still give exactly +1.
    theta = 0.7
    R = np.array([
        [math.cos(theta), -math.sin(theta), 0],
        [math.sin(theta), math.cos(theta), 0],
        [0, 0, 1],
    ])
    results.append(test_case("rotated hedgehog (R . r_hat)", lambda v: R @ v, 1))

    # Scale-invariance check: degree must not care about magnitude, only
    # direction -- scale each vertex's field by a different positive factor.
    scales = {"+x": 5.0, "-x": 0.1, "+y": 3.0, "-y": 2.0, "+z": 0.5, "-z": 4.0}
    def scaled_radial(v):
        for label, vec in OCTAHEDRON_VERTICES.items():
            if np.allclose(v, vec):
                return scales[label] * v
        raise AssertionError("unreachable")
    results.append(test_case("magnitude-distorted hedgehog", scaled_radial, 1))

    print()
    print(f"{sum(results)}/{len(results)} checks passed")
    return 0 if all(results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
