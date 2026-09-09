"""Exact conditional response of the selected parity/complement field law.

Rates describe the product-linearized symbol D(k)J only. They do not certify
multi-step microscopic closure, finite-ensemble convergence, or fluid recovery.
"""
from fractions import Fraction
from functools import lru_cache
from numbers import Integral

import flint

from . import hydro_parity as H


PROBES = (
    ((1, 0, 0), ((0, 1, 0), (0, 0, 1))),
    ((1, 1, 0), ((1, -1, 0), (0, 0, 1))),
    ((1, 1, 1), ((1, -1, 0), (1, 1, -2))),
)


def _dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def _vector(value):
    try:
        value = tuple(value)
    except TypeError as error:
        raise ValueError("expected nonzero integer 3-vector") from error
    if (len(value) != 3 or any(isinstance(x, bool) or not isinstance(x, Integral) for x in value)
            or not any(value)):
        raise ValueError("expected nonzero integer 3-vector")
    return tuple(map(int, value))


def _matrix(rows):
    return flint.fmpq_mat([[flint.fmpq(x.numerator, x.denominator) for x in row] for row in rows])


def _fraction(value):
    return Fraction(int(value.p), int(value.q))


@lru_cache(maxsize=8)
def kinetic_inverse(p: Fraction):
    """Inverse of I-J on the 20-dimensional physical conserved complement."""
    jacobian = H.marginal_jacobian(p)
    projector = [[Fraction(1, 24) + Fraction(_dot(v, w), 12)
                  for w in H.VELOCITIES] for v in H.VELOCITIES]
    E = _matrix(projector)
    K = _matrix([[Fraction(i == j) - x for j, x in enumerate(row)]
                 for i, row in enumerate(jacobian)])
    inverse = (K + E).inv() - E
    identity = flint.fmpq_mat([[int(i == j) for j in range(24)] for i in range(24)])
    if K * inverse != identity - E or inverse * E != flint.fmpq_mat(24, 24):
        raise ValueError("physical conserved-complement inverse identity failed")
    return tuple(tuple(_fraction(inverse[i, j]) for j in range(24)) for i in range(24))


def _quadratic_form(left, right, p):
    inverse = kinetic_inverse(p)
    return sum(left[i] * inverse[i][j] * right[j] for i in range(24) for j in range(24))


def shear_form(direction, left, right, p: Fraction):
    """Unnormalised transverse dissipation form per squared Euclidean k."""
    n, t, u = map(_vector, (direction, left, right))
    if _dot(n, t) or _dot(n, u):
        raise ValueError("shear vectors must be transverse")
    H.reference_coefficients(p)
    a = [_dot(n, v) * _dot(t, v) for v in H.VELOCITIES]
    b = [_dot(n, v) * _dot(u, v) for v in H.VELOCITIES]
    return (_quadratic_form(a, b, p) - Fraction(_dot(a, b), 2)) / _dot(n, n)


def shear_rate(direction, transverse, p: Fraction):
    """Rayleigh quotient; a mode rate when the transverse form is diagonal."""
    t = _vector(transverse)
    norm = sum(_dot(t, v) ** 2 for v in H.VELOCITIES)
    return shear_form(direction, t, t, p) / norm


def sound_coefficients(direction, p: Fraction):
    """Squared first-order speed and logarithmic damping per |k|^2/cycle."""
    n = _vector(direction)
    H.reference_coefficients(p)
    n2 = _dot(n, n)
    w = [_dot(n, v) for v in H.VELOCITIES]
    norm = _dot(w, w)
    stress = [x * x - Fraction(n2, 2) for x in w]
    # The sound eigenvectors have equal normalized density and longitudinal
    # momentum components. Their kinetic dissipation is half the longitudinal
    # momentum value. The squared first-order eigenvalue is divided by |n|^2.
    speed_squared = Fraction(norm, 24 * n2)
    damping = (_quadratic_form(stress, stress, p) - Fraction(_dot(stress, stress), 2)) / (2 * norm * n2)
    return speed_squared, damping


def conserved_generator_blocks(direction, p: Fraction):
    """Exact A,B in log P_eff(qn)=-i q A-q^2 B+O(q^3).

    Basis: constant score followed by three spatial velocity scores. These
    are conditional effective small-wave-number coefficients, not a closed
    finite-k projected evolution. B is obtained from the full kinetic inverse.
    """
    n = _vector(direction)
    H.reference_coefficients(p)
    A = [[Fraction(0) for _ in range(4)] for _ in range(4)]
    for axis in range(3):
        A[0][axis + 1] = Fraction(n[axis], 2)
        A[axis + 1][0] = Fraction(n[axis])
    stress = [[_dot(n, v) * v[axis] - Fraction(n[axis], 2) for v in H.VELOCITIES]
              for axis in range(3)]
    B = [[Fraction(0) for _ in range(4)] for _ in range(4)]
    for i in range(3):
        for j in range(3):
            B[i + 1][j + 1] = (_quadratic_form(stress[i], stress[j], p)
                               - Fraction(_dot(stress[i], stress[j]), 2)) / 12
    return tuple(map(tuple, A)), tuple(map(tuple, B))


def certificate() -> dict:
    finite = H.certificate()
    if (not finite["four_fixed_channel_weight_gate"] or finite["stress_scalar_failures"]
            or finite["spatial_stress_reflection_failures"]):
        raise ValueError("finite invariant/stress prerequisite failed")
    p = Fraction(1, 2)
    c, b = H.reference_coefficients(p)
    eigenvalue = Fraction(finite["stress_eigenvalue"])
    relaxation = c * eigenvalue
    tau = 1 / relaxation - Fraction(1, 2)
    probes = []
    for n, (t, u) in PROBES:
        rates = [shear_rate(n, x, p) for x in (t, u)]
        cross = shear_form(n, t, u, p)
        speed_squared, damping = sound_coefficients(n, p)
        if cross or rates != [tau / 3, tau / 3] or speed_squared != Fraction(1, 2) or damping != tau / 4:
            raise ValueError("registered conditional isotropic response gate failed")
        A, B = conserved_generator_blocks(n, p)
        expected_B = [[Fraction(0) for _ in range(4)] for _ in range(4)]
        for i in range(3):
            for j in range(3):
                expected_B[i + 1][j + 1] = tau * (Fraction(_dot(n, n) * int(i == j), 3)
                                                       + Fraction(n[i] * n[j], 6))
        if B != tuple(map(tuple, expected_B)):
            raise ValueError("full four-dimensional conserved response block failed")
        probes.append({"direction": list(n), "transverse_basis": [list(t), list(u)],
                       "shear_rates_per_cycle": [str(r) for r in rates], "cross_form": str(cross),
                       "sound_speed_squared_per_cycle": str(speed_squared),
                       "sound_log_damping_per_cycle": str(damping),
                       "first_order_block": [[str(x) for x in row] for row in A],
                       "second_order_log_block": [[str(x) for x in row] for row in B]})
    return {
        "schema": "strict-hydro-parity-conditional-response-v1", "law_id": H.LAW_ID,
        "scope": "product-linearized D(k)J; exact matrix coefficients; no microscopic multi-step closure",
        "reference_density": str(p), "physical_microticks_per_cycle": 2,
        "probes": probes, "conditional_isotropic_response_gate": True,
        "all_interior_density_formula": {
            "c": "p^11*(1-p)^11/2", "b": "(1+(1-2*p)^22)/2",
            "tau": f"1/({eigenvalue}*c)-1/2", "shear": "tau/3",
            "sound_speed_squared_per_cycle": "1/2", "sound_log_damping": "tau/4",
            "fourth_momentum_multiplier": "-(1-2*p)^22",
        },
        "fourth_momentum_multiplier_at_half": str(1 - 2 * b),
        "stress_relaxation_per_cycle_at_half": str(relaxation),
        "inverse_stress_relaxation_cycles_at_half": str(1 / relaxation),
        "feasibility": {
            "hydrodynamic_small_parameter": "|k|/(14880*c)",
            "wavelength_scale": "wavelength >> 2*pi/(14880*c); scale requirement, not a proved error bound",
            "finite_small_box_hydrodynamic_campaign_feasible": "not established",
            "microscopic_ensemble_gate_required": True,
        },
        "continuum_recovered": False, "canonical_adoption": False,
    }
