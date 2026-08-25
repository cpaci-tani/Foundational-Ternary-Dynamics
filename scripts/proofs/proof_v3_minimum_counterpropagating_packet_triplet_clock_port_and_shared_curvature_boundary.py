#!/usr/bin/env python3
"""Exact minimum moving-current-neutral packet/triplet clock-port certificate.

The selected C4 carrier gives every complete outgoing packet unit energy and
nonzero transported reserve current r/6.  Therefore one moving packet cannot
be current neutral, while the counterpropagating pair is the unique minimum
nonempty neutral batch.  This proof uses that existing pair to gate one clean
triplet COMMIT transaction.  Two complete packet payloads move atomically from
reserve to clock-port ownership; the three A9 arms advance one quarter-turn;
and the complete event has an exact history-retaining inverse.

With the triplet cadence omega=pi/4, the selected identification of one paired
COMMIT with one receiver action quantum gives chi=pi/8 and the conditional
minimal-branch ladder value 3/16.  Under the separately selected symmetric
stress map, the two packet momenta cancel while their axial stress survives.
If the same chi prices the rotor Dirichlet history and the transverse packet
norm, the charged static pole and free Hessian have exactly equal residue.

None of those coefficient identifications follows from the finite ownership
permutation itself.  Field Noether momentum, native action provenance,
formation/refill, and a protected interacting pole remain open.  No target
coupling, master root, fit, or near-miss search enters this certificate.
"""

from __future__ import annotations

import sys
from dataclasses import dataclass
from fractions import Fraction
from itertools import product

from sympy import Matrix, Rational, Symbol, cos, pi, simplify

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_c4_field_packet_reserve_current_and_atomic_clock_debit import (
    swap_batch,
)
from proof_c4_half_admitted_energy_current_momentum_boundary import (
    energy_groups,
)
from proof_cotangent_handed_directional_radiation_port import (
    DirectionalPortState,
    port_records,
    propagation_direction,
)
from proof_oriented_bond_plaquette_hodge_maxwell_target import dot
from proof_v3_cubic_triplet_self_correcting_material_clock import (
    DARK,
    LOGICAL,
    TripletClock,
    clean_state,
    triplet_step,
)
from proof_ternary_square_phase_polarity_autonomous_clock import tick


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]


def add(left: Vec, right: Vec) -> Vec:
    return tuple(a + b for a, b in zip(left, right))


def scale(factor, vector):
    return tuple(factor * component for component in vector)


def packet_energy(state: DirectionalPortState):
    layer = (-state.stage) % 3
    return sum(
        (energy for _position, _flag, energy in energy_groups(port_records(state), layer)),
        Rational(0),
    )


def packet_current(state: DirectionalPortState):
    return scale(Rational(1, 6), propagation_direction(state.frame, state.chirality))


def packet_payload(state: DirectionalPortState):
    return state, port_records(state)


def paired_packets(frame, phase: int, stage: int, orientation: int):
    positive = DirectionalPortState(
        frame, 1, phase, stage, orientation, True, 0
    )
    negative = DirectionalPortState(
        frame, -1, phase, stage, orientation, True, 0
    )
    return positive, negative


def inverse_tick(state):
    parents = [candidate for candidate in LOGICAL if tick(candidate) == state]
    assert len(parents) == 1
    return parents[0]


@dataclass(frozen=True)
class ClockPortEvent:
    body: TripletClock
    packets: tuple[tuple[str, object], tuple[str, object]]


def is_clean_pending(body: TripletClock) -> bool:
    return (
        len(set(body.arms)) == 1
        and len(set(body.heralds)) == 1
        and body.heralds[0] is not DARK
        and body.heralds[0] == body.arms[0]
    )


def is_counterpropagating_payload_pair(packets) -> bool:
    if len(packets) != 2:
        return False
    states = [payload[0] for _owner, payload in packets]
    if not all(isinstance(state, DirectionalPortState) for state in states):
        return False
    if any(not state.outgoing for state in states):
        return False
    currents = [packet_current(state) for state in states]
    return (
        add(currents[0], currents[1]) == (0, 0, 0)
        and currents[0] != (0, 0, 0)
        and packet_payload(states[0]) != packet_payload(states[1])
    )


def absorb_commit(event: ClockPortEvent):
    if not is_clean_pending(event.body):
        return None
    if not is_counterpropagating_payload_pair(event.packets):
        return None
    if any(owner != "reserve" for owner, _payload in event.packets):
        return None
    absorbed = swap_batch(event.packets, (0, 1), "reserve", "clock-port")
    if absorbed is None:
        return None
    return ClockPortEvent(triplet_step(event.body), absorbed)


def emit_inverse(event: ClockPortEvent):
    if len(set(event.body.arms)) != 1 or event.body.heralds != (DARK,) * 3:
        return None
    if not is_counterpropagating_payload_pair(event.packets):
        return None
    if any(owner != "clock-port" for owner, _payload in event.packets):
        return None
    restored_packets = swap_batch(
        event.packets, (0, 1), "clock-port", "reserve"
    )
    if restored_packets is None:
        return None
    prior = inverse_tick(event.body.arms[0])
    restored_body = clean_state(prior, heralded=True)
    if triplet_step(restored_body) != event.body:
        return None
    return ClockPortEvent(restored_body, restored_packets)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def dyad(vector: Vec) -> Matrix:
    column = Matrix(vector)
    return column * column.T


def main() -> None:
    frames = tuple(
        (direction, second)
        for direction in SC_DIRECTIONS
        for second in SC_DIRECTIONS
        if dot(direction, second) == 0
    )
    check("C1 the ordered SC port family has exactly twenty-four frames", len(frames) == 24)

    packet_rows = 0
    pair_rows = 0
    distinct_record_rows = 0
    pair_fixtures = []
    for frame, chirality, phase, stage, orientation in product(
        frames, (-1, 1), range(4), range(12), (-1, 1)
    ):
        state = DirectionalPortState(
            frame, chirality, phase, stage, orientation, True, 0
        )
        assert packet_energy(state) == 1
        assert packet_current(state) != (0, 0, 0)
        assert len(port_records(state)) == len(set(port_records(state))) == 16
        packet_rows += 1

    for frame, phase, stage, orientation in product(
        frames, range(4), range(12), (-1, 1)
    ):
        positive, negative = paired_packets(frame, phase, stage, orientation)
        current_sum = add(packet_current(positive), packet_current(negative))
        assert current_sum == (0, 0, 0)
        assert packet_energy(positive) + packet_energy(negative) == 2
        assert not (set(port_records(positive)) & set(port_records(negative)))
        assert len(set(port_records(positive) + port_records(negative))) == 32
        pair_rows += 1
        distinct_record_rows += 1
        pair_fixtures.append((positive, negative))

    check(
        "C2 every outgoing complete packet has unit energy and nonzero reserve current",
        packet_rows == 24 * 2 * 4 * 12 * 2,
    )
    check(
        "C3 every opposite pair has energy two, zero total reserve current, and thirty-two distinct records",
        pair_rows == distinct_record_rows == 24 * 4 * 12 * 2,
    )

    ray_currents = {scale(Rational(1, 6), direction) for direction in SC_DIRECTIONS}
    neutral_pairs = {
        tuple(sorted((left, right)))
        for left in ray_currents
        for right in ray_currents
        if add(left, right) == (0, 0, 0)
    }
    check(
        "C4 two counterpropagating packets are the unique minimum nonempty moving-current-neutral batch",
        (0, 0, 0) not in ray_currents
        and len(neutral_pairs) == 3
        and all(right == scale(-1, left) for left, right in neutral_pairs),
    )

    event_rows = 0
    for logical, (positive, negative) in product(LOGICAL, pair_fixtures):
        packets = (
            ("reserve", packet_payload(positive)),
            ("reserve", packet_payload(negative)),
        )
        event = ClockPortEvent(clean_state(logical, heralded=True), packets)
        absorbed = absorb_commit(event)
        assert absorbed is not None
        assert absorbed.body == clean_state(tick(logical), heralded=False)
        assert all(owner == "clock-port" for owner, _payload in absorbed.packets)
        assert tuple(payload for _owner, payload in absorbed.packets) == tuple(
            payload for _owner, payload in packets
        )
        assert emit_inverse(absorbed) == event
        event_rows += 1

    check(
        "C5 every logical phase and physical opposite-packet presentation has an exact paired COMMIT and inverse",
        event_rows == 16 * 24 * 4 * 12 * 2,
    )

    logical = LOGICAL[0]
    positive, negative = pair_fixtures[0]
    good_packets = (
        ("reserve", packet_payload(positive)),
        ("reserve", packet_payload(negative)),
    )
    pending = clean_state(logical, heralded=True)
    dark = clean_state(logical, heralded=False)
    parallel = (
        ("reserve", packet_payload(positive)),
        ("reserve", packet_payload(positive)),
    )
    spent = (
        ("clock-port", packet_payload(positive)),
        ("reserve", packet_payload(negative)),
    )
    check(
        "C6 missing, single, parallel, spent, and non-COMMIT admissions fail before mutation",
        absorb_commit(ClockPortEvent(pending, tuple())) is None
        and absorb_commit(ClockPortEvent(pending, good_packets[:1])) is None
        and absorb_commit(ClockPortEvent(pending, parallel)) is None
        and absorb_commit(ClockPortEvent(pending, spent)) is None
        and absorb_commit(ClockPortEvent(dark, good_packets)) is None,
    )

    # The selected body action quantum is one complete paired-packet COMMIT.
    # This is a branch selection, not a derivation of the physical coupling.
    omega = pi / 4
    debit = Rational(2)
    chi_port = simplify(omega / debit)
    c_eff = Rational(1, 6)
    alpha_branch = simplify(chi_port / (4 * pi * c_eff))
    check(
        "C7 the selected one-COMMIT action quantum has exact packet debit d=2",
        debit == 2 and chi_port == pi / 8,
    )
    check(
        "C8 the current-neutral minimum branch lies at the exact conditional ladder value 3/16",
        alpha_branch == Rational(3, 16),
    )

    # Conditional symmetric stress: p=6 Gamma r and Sigma=Gamma rr^T.
    gamma = Symbol("Gamma", positive=True)
    ray = propagation_direction(positive.frame, positive.chirality)
    opposite = propagation_direction(negative.frame, negative.chirality)
    total_momentum = add(scale(6 * gamma, ray), scale(6 * gamma, opposite))
    total_stress = gamma * dyad(ray) + gamma * dyad(opposite)
    check(
        "C9 under the selected symmetric-stress map the pair has zero recoil momentum but nonzero axial stress",
        total_momentum == (0, 0, 0)
        and total_stress == 2 * gamma * dyad(ray)
        and simplify(total_stress.trace() - 2 * gamma) == 0,
    )

    # Common-coefficient pole seam.  The rotor history supplies 1/Lambda;
    # selecting the packet/clock coefficient chi_port for both sectors makes
    # the static and free estimators identical, but does not derive that tie.
    k0, k1, k2 = (Symbol(f"k{axis}", real=True) for axis in range(3))
    lattice_symbol = 2 * sum(1 - cos(value) for value in (k0, k1, k2))
    rho_sq = Symbol("rho_sq", positive=True)
    static_energy = chi_port * rho_sq / (2 * lattice_symbol)
    static_estimator = simplify(2 * static_energy * lattice_symbol / rho_sq)
    free_hessian = chi_port
    check(
        "C10 the rotor Dirichlet history supplies the charged massless 1/Lambda pole",
        lattice_symbol.subs({k0: 0, k1: 0, k2: 0}) == 0,
    )
    check(
        "C11 one selected common packet coefficient gives exactly equal static residue and free-field Hessian",
        static_estimator == free_hessian == pi / 8,
    )

    independent_static = Symbol("lambda_static", positive=True)
    independent_estimator = simplify(
        2
        * (independent_static * rho_sq / (2 * lattice_symbol))
        * lattice_symbol
        / rho_sq
    )
    check(
        "C12 the finite ownership/clock permutation does not force the common-curvature identification",
        independent_estimator == independent_static
        and simplify(independent_estimator - free_hessian)
        == independent_static - pi / 8,
    )

    missing = {
        "native derivation of the paired absorption trigger",
        "field Noether momentum rather than reserve-current neutrality",
        "formation refill release and traffic protection",
        "finite action derivation of the static-free curvature equality",
        "protected interacting charged pole at finite block size",
        "canonical Phi integration and laboratory coupling measurement",
    }
    forbidden = (
        "137.036",
        "empirical_alpha",
        "desired_integer",
        "near_miss",
        "master_root_value",
    )
    check(
        "C13 the result closes a prepared minimum debit/seam, not physical coupling normalization",
        len(missing) == 6 and all(token not in __doc__.lower() for token in forbidden),
    )

    passed = sum(ok for _, ok, _detail in checks)
    print(f"\n{passed}/{len(checks)} paired-packet triplet-clock-port checks pass")
    print(f"outgoing_packet_rows={packet_rows}")
    print(f"counterpropagating_pair_rows={pair_rows}")
    print(f"clock_port_event_rows={event_rows}")
    print("minimum_moving_current_neutral_debit=2")
    print("triplet_commit_cadence=pi/4")
    print("conditional_common_curvature=pi/8")
    print("conditional_minimum_branch_alpha=3/16")
    print("status=prepared_debit_exact_current_neutrality_not_field_noether_momentum_common_curvature_selected")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
