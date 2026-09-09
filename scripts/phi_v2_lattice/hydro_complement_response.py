"""Exact product-linearized shear obstruction for the complement field sector.

This computes finite matrix perturbation coefficients. It never certifies
closure of perturbed microscopic trajectories or finite ensembles.
"""
from fractions import Fraction
from functools import lru_cache
from numbers import Integral

import flint

from . import hydro_complement as H

PROBES = (
    ((1, 0, 0), ((0, 1, 0), (0, 0, 1))),
    ((1, 1, 0), ((1, -1, 0), (0, 0, 1))),
    ((1, 1, 1), ((1, -1, 0), (1, 1, -2))),
)


def _fraction(value):
    return Fraction(int(value.p), int(value.q))


@lru_cache(maxsize=1)
def kinetic_inverse():
    """Exact Q pseudoinverse; immutable tuple, not a numeric-tolerance SVD."""
    projector = [[Fraction(1, 24) + Fraction(sum(a * b for a, b in zip(v, w)), 12)
                  for w in H.VELOCITIES] for v in H.VELOCITIES]
    E = flint.fmpq_mat([[flint.fmpq(x.numerator, x.denominator) for x in row]
                       for row in projector])
    Q = flint.fmpq_mat(H.collision_gram())
    inverse = (Q + E).inv() - E
    identity = flint.fmpq_mat([[int(i == j) for j in range(24)] for i in range(24)])
    if Q * inverse != identity - E or inverse * E != flint.fmpq_mat(24, 24):
        raise ValueError("conserved-complement inverse identity failed")
    return tuple(tuple(_fraction(inverse[i, j]) for j in range(24)) for i in range(24))


def _vector(value):
    try:
        value = tuple(value)
    except TypeError as error:
        raise ValueError("expected nonzero integer 3-vector") from error
    if (len(value) != 3 or any(isinstance(x, bool) or not isinstance(x, Integral) for x in value)
            or not any(value)):
        raise ValueError("expected nonzero integer 3-vector")
    return tuple(map(int, value))


def _dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def shear_form(direction, left, right, p: Fraction) -> Fraction:
    """Unnormalised shear bilinear form per squared Euclidean wavenumber."""
    n, t, u = map(_vector, (direction, left, right))
    if _dot(n, t) or _dot(n, u):
        raise ValueError("shear vectors must be transverse")
    if not isinstance(p, Fraction) or not 0 < p < 1:
        raise ValueError("p must be a Fraction strictly between zero and one")
    q_inverse = kinetic_inverse()
    a = [_dot(n, v) * _dot(t, v) for v in H.VELOCITIES]
    b = [_dot(n, v) * _dot(u, v) for v in H.VELOCITIES]
    c = p ** 11 * (1 - p) ** 11 / 2
    response = sum(a[i] * q_inverse[i][j] * b[j] for i in range(24) for j in range(24))
    return (response / c - Fraction(_dot(a, b), 2)) / _dot(n, n)


def shear_rate(direction, transverse, p: Fraction) -> Fraction:
    """Shear Rayleigh quotient; an eigenvalue only in a diagonal shear basis."""
    t = _vector(transverse)
    norm = sum(_dot(t, v) ** 2 for v in H.VELOCITIES)
    return shear_form(direction, t, t, p) / norm


def certificate() -> dict:
    p = Fraction(1, 2)
    probes = []
    for n, (t, u) in PROBES:
        rates = [shear_rate(n, x, p) for x in (t, u)]
        cross = shear_form(n, t, u, p)
        if cross:
            raise ValueError("declared shear basis has off-diagonal mixing")
        probes.append({"direction": list(n), "transverse_basis": [list(t), list(u)],
                       "rates_per_cycle": [str(r) for r in rates], "cross_form": str(cross)})
    c = p ** 11 * (1 - p) ** 11 / 2
    axis = shear_rate(PROBES[0][0], PROBES[0][1][0], p)
    diagonal = shear_rate(PROBES[1][0], PROBES[1][1][0], p)
    coefficient_axis = c * (axis + Fraction(1, 6))
    coefficient_diagonal = c * (diagonal + Fraction(1, 6))
    difference = coefficient_diagonal - coefficient_axis
    if difference <= 0:
        raise ValueError("all-density anisotropy proof failed")
    trace = sum(H.collision_gram()[i][i] for i in range(24))
    return {
        "schema": "strict-hydro-complement-conditional-response-v1", "law_id": H.LAW_ID,
        "scope": "product-linearized P(k)=diag(exp(-i k.v)) J; no microscopic closure",
        "density": str(p), "physical_microticks_per_cycle": 2,
        "probes": probes, "axis_inverse_gram_coefficient": str(coefficient_axis),
        "diagonal_inverse_gram_coefficient": str(coefficient_diagonal),
        "positive_coefficient_difference": str(difference),
        "all_density_anisotropy": "nu_diagonal(p)-nu_axis(p)=positive_difference/c(p)>0 for 0<p<1",
        "isotropic_shear_gate": False,
        "nonconserved_jacobian_eigenvalue_lower_bound": str(1 - Fraction(trace, 1 << 23)),
        "additional_unit_modulus_marginal_modes": False,
        "continuum_recovered": False, "canonical_adoption": False,
    }
