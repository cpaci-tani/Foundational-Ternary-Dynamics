"""Exact wave-2 finite calculations, separate from registered GPU measurements."""
from __future__ import annotations

import argparse
from collections import Counter
from dataclasses import asdict, is_dataclass
from fractions import Fraction
import hashlib
import json
from pathlib import Path
import sys

from .. import channels as C, staged as P
from .. import recovery_pair_sector as Pair, recovery_kinetic_response as Response
from .. import recovery_continuity as Continuity


def exact(value):
    if isinstance(value, Fraction):
        return {"numerator": str(value.numerator), "denominator": str(value.denominator)}
    if is_dataclass(value):
        return exact(asdict(value))
    if isinstance(value, dict):
        return {str(k): exact(v) for k, v in value.items()}
    if isinstance(value, (tuple, list)):
        return [exact(v) for v in value]
    return value


def report():
    h = (1,) + (0,)*191
    r = Response.REFERENCE_P*(1-Response.REFERENCE_P)**189
    responses = []
    for layer in range(3):
        response = Response.collision_response(layer, h)
        target = Pair.collide((0, 1), layer)
        selected = next(value for j, k, value in response.pair_covariances if (j, k) == target)
        encoded = json.dumps(exact(response.pair_covariances), separators=(",", ":"))
        responses.append({"layer": layer, "nonzero_pair_derivatives": len(response.pair_covariances),
                          "maximum_abs_derivative_over_r": response.maximum_pair_covariance/r,
                          "selected_pair": target, "selected_derivative_over_r": selected/r,
                          "complete_nonzero_covariance_sha256": hashlib.sha256(encoded.encode()).hexdigest()})
    orbits = Pair.colocated_orbits()
    budgets = []
    for horizon in (0, 2, 4, 8):
        budget = Response.homogeneous_tangent_budget(3, horizon, h)
        budgets.append({"L": 3, "horizon": horizon,
                        "initial_score_norm_squared": budget.initial_score_norm_squared,
                        "final_projected_norm_squared": budget.final_projected_norm_squared,
                        "all_order_leakages_squared": budget.collision_leakages_squared,
                        "norm_error_upper": budget.norm_error_upper})
    amplitude = Response.homogeneous_finite_amplitude_budget(3, 8, h, Fraction(1, 192))
    current = {"owner_measure": "bank plus SC/FCC primary/reserve polarity inventories",
               "admission_current": "negative-direction field to neighboring SC storage owner",
               "exact_weak_identity": "sum f delta(rho) = sum Q (f(destination)-f(source))",
               "conditional_error_formula": "B*a^2*N_hops/(2*normalizer)",
               "fixed_T_B_speed_unit_bound": {str(n): Continuity.weak_current_error_bound(n, Fraction(1, n), 1)
                                             for n in (4, 8, 16, 32)}}
    result = {"schema": "strict-recovery-wave2-exact-1", "law_id": P.LAW_ID,
              "collision_sha256": C.COLLISION_HASH,
              "calculation_kind": "exact_finite_maps_and_conditional_bounds_not_a_measurement_campaign",
              "response": {"p": Response.REFERENCE_P, "r": r, "probe": "h=e_0",
                           "layers": responses, "common_fixed_additive_moments": Response.common_additive_classification(),
                           "homogeneous_tangent_budgets": budgets,
                           "homogeneous_finite_amplitude": {
                               "L": 3, "horizon": 8, "epsilon": amplitude.epsilon,
                               "initial_remainder_squared": amplitude.initial_product_remainder_squared,
                               "initial_remainder_upper": amplitude.initial_product_remainder_upper,
                               "transported_tangent_error_upper": amplitude.transported_tangent_error_upper,
                               "full_density_error_upper": amplitude.full_density_error_upper,
                               "comparison": amplitude.comparison}},
              "pair_sector": {"local_census": Pair.census(), "orbits": orbits,
                              "orbit_length_counts": dict(Counter(len(orbit.states) for orbit in orbits))},
              "inventory_continuity": current,
              "limits": {"binding_identified": False, "physical_momentum_identified": False,
                         "atomic_molecular_recovery": False, "constitutive_law_recovery": False,
                         "gravitational_recovery": False, "canonical_adoption": False}}
    root = Path(__file__).resolve().parents[3]
    sources = {}
    for module in tuple(sys.modules.values()):
        path = getattr(module, "__file__", None)
        if path:
            path = Path(path).resolve()
            if path.suffix == ".py" and path.is_relative_to(root):
                sources[path.relative_to(root).as_posix()] = hashlib.sha256(path.read_bytes()).hexdigest()
    result["source_sha256"] = dict(sorted(sources.items()))
    return exact(result)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report(), indent=2, sort_keys=True)+"\n", encoding="utf-8")


if __name__ == "__main__":
    main()
