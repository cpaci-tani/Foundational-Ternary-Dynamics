"""Reproduce exact wave-1 algebra and the complete-ensemble collision witness.

This is a finite calculation, not a random measurement campaign. GPU carrier
evidence is registered and executed separately by recovery_carriers.
"""
from __future__ import annotations

import argparse
from dataclasses import asdict, is_dataclass
from fractions import Fraction
import hashlib
import json
from pathlib import Path
import sys

from .. import channels as C, prepare, recovery_transport as R, staged as P
from ..recovery_ensemble import FiniteEnsemble
from .. import recovery_kinetic_reference as K


def _exact(value):
    if isinstance(value, Fraction):
        return {"numerator": str(value.numerator), "denominator": str(value.denominator)}
    if is_dataclass(value):
        return _exact(asdict(value))
    if isinstance(value, dict):
        return {str(key): _exact(item) for key, item in value.items()}
    if isinstance(value, (tuple, list)):
        return [_exact(item) for item in value]
    return value


def report():
    kinetic = {
        "finite_map_certificate": K.finite_map_certificate(),
        "selected_preparation_pricing": K.feasibility(9, 64),
        "half_occupation_pricing": K.feasibility(9, 64, Fraction(1, 2)),
        "mean_population_at_selected_density": Fraction(384 * 9**3, 96),
        "measured_campaign": False,
        "full_record_reference": "correlated phase-dependent lift; absolute clock advances",
    }
    certificates = [R.orbit_certificate(channel) for channel in range(C.N_CHANNELS)]
    members = []
    for occupied in ((), (0,), (1,), (0, 1)):
        state = P.initialize(prepare.r5_vacuum(3, seed=0, occupation=0))
        state.lattice.bank[13, list(occupied)] = True
        members.append((state, 1))
    initial = FiniteEnsemble.from_states(members)
    after = initial.advance(2)
    left = lambda state: int(state.lattice.bank[13, 4])
    right = lambda state: int(state.lattice.bank[13, 5])
    root = Path(__file__).resolve().parents[3]
    sources = {}
    for module in tuple(sys.modules.values()):
        raw = getattr(module, "__file__", None)
        if raw:
            path = Path(raw).resolve()
            if path.suffix == ".py" and path.is_relative_to(root):
                sources[path.relative_to(root).as_posix()] = hashlib.sha256(path.read_bytes()).hexdigest()
    return _exact({
        "schema": "strict_recovery_wave1_calculation_v1",
        "law_id": P.LAW_ID,
        "collision_sha256": C.COLLISION_HASH,
        "calculation_kind": "exact_finite_maps_and_complete_counting_pushforward",
        "kinetic_reference": kinetic,
        "transport": {
            "domain": "one global field token; homogeneous doubly occupied periodic background",
            "channels": len(certificates),
            "orbits": R.channel_orbits(),
            "period_microticks": sorted({c.period_microticks for c in certificates}),
            "velocities_per_microtick": sorted({c.velocity for c in certificates}),
            "maximum_integer_tick_ripple": max(c.integer_tick_ripple_linf for c in certificates),
            "maximum_held_time_ripple": max(c.held_time_ripple_linf for c in certificates),
            "comparison": "multiply the displacement bounds by declared comparison spacing a",
            "norm": "L-infinity path distance; finite label pairing bounds W1 with that cost",
        },
        "complete_counting_witness": {
            "preparation": "four equally counted singleton-site sets empty,{0},{1},{0,1}",
            "physical_microticks": 2,
            "support_before": len(initial.atoms),
            "support_after": len(after.atoms),
            "channel_4_marginal": after.expectation(left),
            "channel_5_marginal": after.expectation(right),
            "joint_4_5": after.expectation(lambda state: left(state)*right(state)),
            "product_of_marginals": after.expectation(left)*after.expectation(right),
            "covariance": after.covariance(left, right),
            "selected_external_counting_measure": True,
        },
        "limits": {
            "physical_calibration": False,
            "maxwell_or_collisional_fluid_recovery": False,
            "material_particle_identification": False,
            "atomic_molecular_recovery": False,
            "gravity_recovery": False,
            "canonical_law_adoption": False,
        },
        "source_sha256": dict(sorted(sources.items())),
    })


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    encoded = json.dumps(report(), indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded, encoding="utf-8")
    else:
        print(encoded, end="")


if __name__ == "__main__":
    main()
