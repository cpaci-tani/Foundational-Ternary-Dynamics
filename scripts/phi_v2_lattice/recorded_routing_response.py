"""Exact prepared-ensemble formulas for all eight recorded routing directions."""
from fractions import Fraction
from numbers import Integral

import flint
import numpy as np

from . import recorded_routing as R

SPACINGS = (4, 8, 16, 32)
PROBES = ((1, 0, 0), (1, 1, 0), (1, 1, 1))


def path_probability(L, cycles, corner_code):
    """Specified word probability conditional on the explicit uniform background."""
    _, cycles, _ = R.validate_freshness_scope(L, cycles, corner_code)
    return Fraction(1, 6 ** cycles)


def endpoint_counts(cycles):
    if isinstance(cycles, bool) or not isinstance(cycles, Integral) or not 0 <= cycles <= 12:
        raise ValueError("explicit endpoint census requires an integer in 0..12")
    counts = {(0, 0, 0): 1}
    for _ in range(int(cycles)):
        updated = {}
        for point, count in counts.items():
            for direction in R.VELOCITIES:
                target = tuple(x + d for x, d in zip(point, direction))
                updated[target] = updated.get(target, 0) + count
        counts = updated
    return counts


def heat_error_bound(spacing, time, diffusion, pure_fourth_sum, laplacian_squared_norm):
    values = (spacing, time, diffusion, pure_fourth_sum, laplacian_squared_norm)
    if any(isinstance(v, bool) or not isinstance(v, (Integral, Fraction)) for v in values):
        raise ValueError("bound inputs must be exact rationals")
    a, t, d, fourth, mixed = map(Fraction, values)
    if a <= 0 or d <= 0 or min(t, fourth, mixed) < 0:
        raise ValueError("invalid comparison domain")
    return d * t * a * a * (fourth + mixed) / 12


def predict_densities(initial, cycles, corner_direction):
    """External full-prepared-ensemble mean, never a microscopic restore or reset."""
    if not isinstance(initial, np.ndarray) or initial.ndim != 4:
        raise ValueError("expected LxLxLx2 mean populations")
    L = initial.shape[0]
    R._integer(L, 3, "L")
    R._validate_background(corner_direction, L)
    # This external domain check has no role in the microscopic transition.
    R.validate_freshness_scope(L, cycles, int(corner_direction[0, 0, 0]))
    if (initial.shape != (L, L, L, 2) or initial.dtype.kind not in "iuf"
            or not np.isfinite(initial).all() or np.any(initial < 0) or np.any(initial > 6)):
        raise ValueError("invalid mean populations")
    result = initial.astype(np.float64, copy=True)
    for _ in range(int(cycles)):
        result = sum(np.roll(result, v, axis=(0, 1, 2)) for v in R.VELOCITIES) / 6
    return result


def certificate():
    rows = []
    with flint.ctx.workprec(192):
        for m in SPACINGS:
            for wave in PROBES:
                norm2 = sum(x * x for x in wave)
                fourth = sum(x ** 4 for x in wave)
                bound = heat_error_bound(Fraction(1, m), Fraction(1, 6), 1, fourth, norm2 ** 2)
                eigenvalue = sum((flint.arb(k) / m).cos() for k in wave) / 3
                microscopic = eigenvalue ** (m * m)
                continuum = (-flint.arb(norm2) / 6).exp()
                error = abs(microscopic - continuum)
                if not error < flint.arb(bound.numerator) / bound.denominator:
                    raise ValueError("registered certified comparison failed")
                rows.append({"m": m, "wave": list(wave), "cycles": m * m,
                             "microticks": 2 * m * m, "minimum_fresh_periodic_L": 3 * m * m + 1,
                             "microscopic_expectation_enclosure": str(microscopic),
                             "heat_expectation_enclosure": str(continuum),
                             "absolute_error_enclosure": str(error), "complete_error_bound": str(bound),
                             "certified_below_bound": True})
    return {"schema": "strict-recorded-routing-continuum-1", "law_id": R.LAW_ID,
            "scope": "conditional constant recorded D; full prepared ensemble at L>3T; weak C4 heat comparison",
            "corner_vectors": [list(d) for d in R.CORNERS], "directions_covered_per_probe": 8,
            "freshness_identity": "D_r*delta_r+2h in [h,3h] subset (0,L)",
            "initial_router_measure": "conditionally independent uniform S6, independent of initial banks",
            "exact_kernel": "m_next(x)=(1/6)sum_i m(x-v_i)", "path_word_probability": "6^(-T)",
            "comparison_time": "1/6", "diffusion_parameter": "1", "precision_bits": 192,
            "probes": rows, "all_registered_comparisons_passed": True,
            "large_lattices_materialized": False, "runtime_randomness": False,
            "arbitrary_background_defects_covered": False, "background_formation_recovered": False,
            "arbitrary_initial_router_correlations_covered": False,
            "multiple_carrier_independent_paths_claimed": False,
            "fixed_torus_arbitrary_horizons_covered": False, "fixed_light_speed_calibration_claimed": False,
            "mechanical_momentum_or_energy_recovered": False, "interacting_fluid_continuum_recovered": False,
            "complete_v3_law_approved": False, "canonical_adoption": False}
