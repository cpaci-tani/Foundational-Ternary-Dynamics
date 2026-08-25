#!/usr/bin/env python3
"""Exact half-admitted energy current and mechanical-momentum boundary.

For the selected C4-trivial field metric, each of the eight phase-paired ray
groups carries energy 1/8.  Under phase-parity half-admission the complete
energy centroid advances by r exactly once in every six ticks.  Therefore the
native transported energy current is J_E=r/6, while the clock-matched E x B
readout remains r/2 on every tick.  The latter is exactly three times the
actual hydrodynamic energy current and cannot yet be called momentum.

If one additionally imposes a symmetric relativistic stress tensor at
c_eff=1/6, then p_field=J_E/c_eff^2=6 Gamma r for a free packet of energy
Gamma.  Matching that to the provisional one-hop material momentum mu r would
force mu=6 Gamma, I*=7 Gamma/2, and work fraction 1/7.  But interpreting the
one-tick material hop itself as velocity gives v_M/c_eff=6.  This conditional
diagnostic confirms that the existing recoil displacement is not yet a stable
matter worldline or action-derived mechanical momentum.

No relativistic stress symmetry, inertia ratio, coupling, or alpha value is
derived here.  The exact native result is the energy-current/Poynting
separation and the minimum six-tick material-cadence boundary.
"""

from __future__ import annotations

from collections import Counter, defaultdict

from sympy import Rational, simplify, symbols

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_c4_phase_parity_half_admitted_two_polarization_carrier import (
    gated_stream,
    selected_energy_momentum,
)
from proof_cotangent_handed_directional_radiation_port import (
    DirectionalPortState,
    port_records,
    propagation_direction,
)
from proof_global_c3_cotangent_layer_hodge_maxwell_target import layer_value
from proof_oriented_bond_plaquette_hodge_maxwell_target import cross, dot
from proof_shared_edge_hodge_flag_bcc_propagation import add, scale


def subtract(left, right):
    return tuple(a - b for a, b in zip(left, right))


def energy_groups(records, layer: int):
    accumulators = defaultdict(lambda: [0] * 6)
    for position, record in records:
        key = (position, record[0])
        value = layer_value(record, layer)
        for component, entry in enumerate(value):
            accumulators[key][component] += entry

    output = []
    for (position, flag), value in accumulators.items():
        electric = value[:3]
        magnetic = value[3:]
        energy = Rational(
            dot(electric, electric) + dot(magnetic, magnetic), 64
        )
        output.append((position, flag, energy))
    return tuple(output)


def energy_centroid(records, layer: int):
    groups = energy_groups(records, layer)
    total = sum((energy for _position, _flag, energy in groups), Rational(0))
    numerator = [Rational(0), Rational(0), Rational(0)]
    for position, _flag, energy in groups:
        for axis in range(3):
            numerator[axis] += energy * position[axis]
    return tuple(value / total for value in numerator), total, groups


def verify_trace(state: DirectionalPortState, parity: int) -> int:
    checks = 0
    propagation = propagation_direction(state.frame, state.chirality)
    records = port_records(state)
    centroids = []
    for tick in range(7):
        layer = (-state.stage - tick) % 3
        centroid, energy, groups = energy_centroid(records, layer)
        readout_energy, poynting = selected_energy_momentum(records, layer)
        assert energy == readout_energy == 1
        assert len(groups) == 8
        assert Counter(group_energy for _position, _flag, group_energy in groups) == Counter(
            {Rational(1, 8): 8}
        )
        assert poynting == scale(Rational(1, 2), propagation)
        centroids.append(centroid)
        checks += 5
        if tick < 6:
            records = gated_stream(records, parity)

    increments = tuple(
        subtract(centroids[index + 1], centroids[index]) for index in range(6)
    )
    assert Counter(increments) == Counter({(0, 0, 0): 5, propagation: 1})
    assert subtract(centroids[-1], centroids[0]) == propagation
    average_current = scale(Rational(1, 6), propagation)
    raw_poynting = scale(Rational(1, 2), propagation)
    assert raw_poynting == scale(3, average_current)
    checks += 3
    return checks


def main() -> None:
    checks = 0
    frames = tuple(
        (direction, second)
        for direction in SC_DIRECTIONS
        for second in SC_DIRECTIONS
        if dot(direction, second) == 0
    )
    assert len(frames) == 24
    checks += 1

    # Exhaust spatial covariance on one internal presentation.
    for frame in frames:
        for chirality in (-1, 1):
            for orientation in (-1, 1):
                state = DirectionalPortState(
                    frame, chirality, 0, 0, orientation, True, 0
                )
                checks += verify_trace(state, 0)

    # Exhaust all internal clock offsets and both time-translated schedules on
    # one representative spatial orbit.
    representative = ((1, 0, 0), (0, 1, 0))
    for phase in range(4):
        for stage in range(12):
            for orientation in (-1, 1):
                for parity in (0, 1):
                    state = DirectionalPortState(
                        representative, 1, phase, stage, orientation, True, 0
                    )
                    checks += verify_trace(state, parity)

    # Conditional relativistic stress/momentum implication.  This is not used
    # as a native derivation; it exposes the exact remaining mechanical price.
    gamma = symbols("Gamma", positive=True)
    c_eff = Rational(1, 6)
    free_energy = gamma
    energy_current = c_eff * free_energy
    relativistic_momentum = simplify(energy_current / c_eff**2)
    assert energy_current == gamma / 6
    assert relativistic_momentum == 6 * gamma
    mu = relativistic_momentum
    source_work = simplify((gamma + mu) / 2)
    work_fraction = simplify(gamma / (gamma + mu))
    static_curvature = simplify(gamma / source_work)
    assert source_work == Rational(7, 2) * gamma
    assert work_fraction == Rational(1, 7)
    assert static_curvature == Rational(2, 7)
    assert Rational(1) / c_eff == 6
    checks += 7

    print("selected packet energy groups: eight copies of 1/8")
    print("energy centroid: one r displacement pulse per six ticks")
    print("native hydrodynamic energy current J_E=r/6")
    print("raw clock-matched E x B readout=r/2=3 J_E")
    print("therefore the raw Poynting readout is not canonical momentum")
    print("conditional symmetric-stress closure would give p_field=6 Gamma r")
    print("conditional one-hop match: mu/Gamma=6, work fraction=1/7 (not adopted)")
    print("a one-tick material hop has speed 6 c_eff and is not a stable recoil worldline")
    print(
        f"PASS: C4 half-admitted energy-current momentum boundary ({checks} exact checks)"
    )
    print(
        "Boundary: derive a slow material recoil cadence and discrete Noether/Legendre "
        "stress tensor before assigning physical momentum or any coupling"
    )


if __name__ == "__main__":
    main()
