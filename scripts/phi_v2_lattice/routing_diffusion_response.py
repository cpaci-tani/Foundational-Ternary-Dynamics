"""Exact counting formulas and certified continuum comparison for routing transport.

The large Fourier probes evaluate a proved finite-ensemble formula, not a
materialized large lattice or a replacement microscopic evolution.
"""
from fractions import Fraction
from numbers import Integral

import flint

from . import routing_diffusion as R

SPACINGS = (4, 8, 16, 32)
PROBES = ((1, 0, 0), (1, 1, 0), (1, 1, 1))


def _natural(value, name):
    if isinstance(value, bool) or not isinstance(value, Integral) or value < 0:
        raise ValueError(f"invalid {name}")
    return int(value)


def path_probability(L, cycles):
    """Probability of any specified direction word in the declared full ensemble."""
    R.validate_freshness_scope(L, cycles)
    return Fraction(1, 6 ** int(cycles))


def endpoint_counts(cycles):
    """Exact six-step word census, diagnostic cap 12 cycles (not a law limit)."""
    cycles = _natural(cycles, "cycles")
    if cycles > 12:
        raise ValueError("explicit endpoint census is capped at 12 cycles")
    counts = {(0, 0, 0): 1}
    for _ in range(cycles):
        updated = {}
        for point, count in counts.items():
            for direction in R.VELOCITIES:
                target = tuple(x + d for x, d in zip(point, direction))
                updated[target] = updated.get(target, 0) + count
        counts = updated
    return counts


def heat_error_bound(spacing, time, diffusion, pure_fourth_sum, laplacian_squared_norm):
    """C4 weak-observable bound including both discrete and heat remainders."""
    values = (spacing, time, diffusion, pure_fourth_sum, laplacian_squared_norm)
    if any(isinstance(v, bool) or not isinstance(v, (Integral, Fraction)) for v in values):
        raise ValueError("bound inputs must be exact rationals")
    a, t, d, fourth, mixed = map(Fraction, values)
    if a <= 0 or d <= 0 or min(t, fourth, mixed) < 0:
        raise ValueError("invalid comparison domain")
    return d * t * a * a * (fourth + mixed) / 12


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
                bound_ball = flint.arb(bound.numerator) / bound.denominator
                passed = bool(error < bound_ball)
                if not passed:
                    raise ValueError("registered certified error gate failed")
                rows.append({"m": m, "wave": list(wave), "cycles": m * m,
                             "microticks": 2 * m * m, "minimum_fresh_periodic_L": 3 * m * m + 1,
                             "microscopic_expectation_enclosure": str(microscopic),
                             "heat_expectation_enclosure": str(continuum),
                             "absolute_error_enclosure": str(error), "complete_error_bound": str(bound),
                             "certified_below_bound": passed})
    return {"schema": "strict-routing-diffusion-continuum-1", "law_id": R.LAW_ID,
            "scope": "exact one-carrier path law and full-prepared-ensemble mean populations for L>3T; "
                     "weak C4 observable heat comparison under diffusive rescaling",
            "router_alphabet_size": len(R.PERMUTATIONS), "runtime_randomness": False,
            "initial_router_measure": "independent uniform S6; independent of initial banks",
            "exact_kernel": "m_next(x)=(1/6)sum_i m(x-v_i)",
            "path_word_probability": "6^(-T)", "bound_order": "O(a^2) at fixed comparison time",
            "physical_microticks_per_cycle": 2, "precision_bits": 192,
            "comparison_time": "1/6", "diffusion_parameter": "1",
            "probes": rows, "all_registered_comparisons_passed": True,
            "large_lattices_materialized": False, "multiple_carrier_independent_paths_claimed": False,
            "arbitrary_initial_router_correlations_covered": False,
            "fixed_torus_arbitrary_horizons_covered": False,
            "fixed_light_speed_calibration_claimed": False,
            "mechanical_momentum_or_energy_recovered": False,
            "interacting_fluid_continuum_recovered": False, "canonical_adoption": False}
