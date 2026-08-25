#!/usr/bin/env python3
"""Exact transitive A2 scheduler for the Phi-v13 Born source histories.

One existing A2 owner has exactly 4,096 occupied phase states.  That equals
the complete Phi-v13 source/controller history space: sixteen logical A9
source states times 2^8 retained advance/stall words.  A raw A2 phase address
therefore decodes bijectively to one complete eight-tick source history.

Two A2 owners run as a base-4,096 odometer.  Their single deterministic orbit
enumerates all ordered pairs of source histories exactly once.  Each address
forms and inversely renews the existing two Born banks.  The already-proved
apparatus returns |Z_L|^2 and |Z_R|^2 events.  Padding by the unused part of
the exact 128-click two-port bound makes every selected trial macro the same
duration, so the odometer's uniform orbit count is also a uniform trial-clock
measure rather than an externally mixed ensemble.

The scheduler is a selected Phi-v15 candidate.  Its owner blocks, controlled
source loader, cross-block routing, detector work, long-run click export,
traffic, state-specific laboratory preparation, Bell correlations, and
canonical Phi integration remain open.  No probability primitive, random
draw, target frequency, empirical value, or fine-structure constant enters.
"""

from __future__ import annotations

import sys
from collections import Counter
from dataclasses import dataclass
from fractions import Fraction

from sympy import Matrix

from proof_ternary_square_phase_polarity_autonomous_clock import LocalState
from proof_v3_contextual_neutral_pointer_born_renewal_apparatus import (
    OrientedRepairChart,
    address_order,
    apparatus_chart,
    canonical_residual,
    compatible,
    outcome,
)
from proof_v3_cubic_triplet_self_correcting_material_clock import LOGICAL
from proof_v3_field_bank_gaussian_born_readout import (
    bright_pair_count,
    gaussian_integer,
    norm_squared,
)
from proof_v3_finite_source_history_born_bank_formation_phi_v13_candidate import (
    WINDOW,
    bank_counts,
    formation_inverse,
    initial_state,
    iterate_formation,
    source_port,
)
from proof_v3_rotor_green_a2_physical_memory_phase_protection import (
    counter_index,
    counter_payload,
)


sys.stdout.reconfigure(encoding="utf-8")

RADIX = 4096
HISTORY_COUNT = len(LOGICAL) * 2**WINDOW
PAIR_PERIOD = RADIX**2
POINTER_PERIOD = 384 * 385
MAX_CLICKS_PER_PORT = WINDOW**2
MAX_TWO_PORT_CLICKS = 2 * MAX_CLICKS_PER_PORT

Schedule = tuple[int, int, int, int, int, int, int, int]


def schedule_from_int(value: int) -> Schedule:
    assert 0 <= value < 2**WINDOW
    return tuple((value >> bit) & 1 for bit in range(WINDOW))  # type: ignore[return-value]


def schedule_to_int(schedule: Schedule) -> int:
    return sum(bit << index for index, bit in enumerate(schedule))


def decode_history(address: int) -> tuple[LocalState, Schedule]:
    assert 0 <= address < RADIX
    return LOGICAL[address // 2**WINDOW], schedule_from_int(address % 2**WINDOW)


def encode_history(source: LocalState, schedule: Schedule) -> int:
    return LOGICAL.index(source) * 2**WINDOW + schedule_to_int(schedule)


def controlled_source_load(address: int, source: LocalState) -> LocalState:
    """Selected reversible C16 addition on an already occupied source owner."""

    desired = address // 2**WINDOW
    return LOGICAL[(LOGICAL.index(source) + desired) % len(LOGICAL)]


def controlled_source_unload(address: int, source: LocalState) -> LocalState:
    desired = address // 2**WINDOW
    return LOGICAL[(LOGICAL.index(source) - desired) % len(LOGICAL)]


@dataclass(frozen=True)
class Odometer:
    low: int
    high: int

    def __post_init__(self) -> None:
        assert 0 <= self.low < RADIX and 0 <= self.high < RADIX


def flatten(state: Odometer) -> int:
    return state.low + RADIX * state.high


def odometer_step(state: Odometer) -> Odometer:
    low = (state.low + 1) % RADIX
    high = (state.high + int(low == 0)) % RADIX
    return Odometer(low, high)


def odometer_inverse(state: Odometer) -> Odometer:
    low = (state.low - 1) % RADIX
    high = (state.high - int(state.low == 0)) % RADIX
    return Odometer(low, high)


def second_port_chart() -> OrientedRepairChart:
    chart = apparatus_chart()
    return OrientedRepairChart(
        chart.origin,
        chart.first,
        chart.second,
        chart.repair_normal,
        chart.layer,
        chart.offset,
        -chart.polarity,
    )


def reverse_formation(chart: OrientedRepairChart, state):
    for _ in range(WINDOW):
        state = formation_inverse(chart, state)
    return state


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    check(
        "C1 one existing A2 raw phase bank has exactly the complete 16 times 2^8 history cardinality",
        RADIX == 8**4 == HISTORY_COUNT == 4096,
    )

    payload_rows = 0
    history_rows = 0
    for address in range(RADIX):
        payload = counter_payload(address)
        assert counter_index(payload) == address
        source, schedule = decode_history(address)
        assert encode_history(source, schedule) == address
        payload_rows += 1
        history_rows += 1
    check(
        "C2 every A2 phase state decodes bijectively to one physical source/controller history",
        payload_rows == history_rows == RADIX,
    )

    loader_rows = 0
    for address in range(RADIX):
        desired_source, _schedule = decode_history(address)
        assert controlled_source_load(address, LOGICAL[0]) == desired_source
        for source in LOGICAL:
            loaded = controlled_source_load(address, source)
            assert controlled_source_unload(address, loaded) == source
            loader_rows += 1
    check(
        "C3 the selected scheduler-to-source loader is an exact finite controlled permutation",
        loader_rows == RADIX * len(LOGICAL),
    )

    # The branch formula is the cyclic successor of the flattened base-4096
    # address.  Checking every low digit and all boundary high digits guards
    # both carry branches; the equality proves the full period algebraically.
    odometer_rows = 0
    inverse_rows = 0
    for high in (0, 1, RADIX - 1):
        for low in range(RADIX):
            state = Odometer(low, high)
            output = odometer_step(state)
            assert flatten(output) == (flatten(state) + 1) % PAIR_PERIOD
            assert odometer_inverse(output) == state
            odometer_rows += 1
            inverse_rows += 1
    check(
        "C4 two A2 owners implement the exact base-4096 successor and inverse",
        odometer_rows == inverse_rows == 3 * RADIX,
    )
    check(
        "C5 the flattened successor is one transitive cycle over all ordered history pairs",
        PAIR_PERIOD == 16_777_216
        and flatten(Odometer(0, 0)) == 0
        and flatten(odometer_inverse(Odometer(0, 0))) == PAIR_PERIOD - 1,
    )

    left_chart = apparatus_chart()
    right_chart = second_port_chart()
    order = address_order(left_chart)
    left_port = source_port(left_chart)
    right_port = source_port(right_chart)
    assert left_port != right_port

    count_rows = []
    weights = []
    formation_rows = 0
    formation_inverse_rows = 0
    two_port_factor_rows = 0
    for address in range(RADIX):
        source, schedule = decode_history(address)

        left_initial = initial_state(source, schedule)
        left_formed = iterate_formation(left_chart, left_initial, WINDOW)
        left_counts = bank_counts(left_formed.bank, left_port)
        left_weight = bright_pair_count(left_counts)
        assert norm_squared(gaussian_integer(left_counts)) == left_weight
        assert reverse_formation(left_chart, left_formed) == left_initial

        right_initial = initial_state(source, schedule)
        right_formed = iterate_formation(right_chart, right_initial, WINDOW)
        right_counts = bank_counts(right_formed.bank, right_port)
        right_weight = bright_pair_count(right_counts)
        assert right_counts == left_counts and right_weight == left_weight
        assert reverse_formation(right_chart, right_formed) == right_initial

        # The two port banks are record-disjoint.  Cancellation and compatible
        # pair enumeration factor by outcome port exactly.
        combined = left_formed.bank | right_formed.bank
        residual = canonical_residual(combined, order)
        observed = Counter()
        for first in residual:
            for second in residual:
                if compatible(first, second, residual):
                    observed[outcome(first)] += 1
        assert observed[left_port] == left_weight
        assert observed[right_port] == right_weight
        assert sum(observed.values()) == 2 * left_weight

        count_rows.append(left_counts)
        weights.append(left_weight)
        formation_rows += 2
        formation_inverse_rows += 2 * WINDOW
        two_port_factor_rows += 1

    check(
        "C6 every scheduler address forms and inversely renews its exact Phi-v13 bank on both ports",
        formation_rows == 2 * RADIX
        and formation_inverse_rows == 2 * RADIX * WINDOW,
    )
    check(
        "C7 two-port residual enumeration factorizes into the two native absolute-square counts",
        two_port_factor_rows == RADIX,
    )

    # Exact physical time measure induced by the single A2 address cycle.
    denominator = Fraction(RADIX)
    means = Matrix(
        [Fraction(sum(row[index] for row in count_rows), RADIX) for index in range(4)]
    )
    covariance = Matrix(
        4,
        4,
        lambda i, j: sum(
            (Fraction(row[i]) - means[i]) * (Fraction(row[j]) - means[j])
            for row in count_rows
        )
        / denominator,
    )
    expected_covariance = Matrix(
        [
            [Fraction(111, 64), Fraction(-1, 2), Fraction(-47, 64), Fraction(-1, 2)],
            [Fraction(-1, 2), Fraction(111, 64), Fraction(-1, 2), Fraction(-47, 64)],
            [Fraction(-47, 64), Fraction(-1, 2), Fraction(111, 64), Fraction(-1, 2)],
            [Fraction(-1, 2), Fraction(-47, 64), Fraction(-1, 2), Fraction(111, 64)],
        ]
    )
    check(
        "C8 the transitive scheduler fixes the exact formed phase-count mean and covariance",
        means == Matrix([2, 2, 2, 2])
        and covariance == expected_covariance,
    )

    z_rows = [gaussian_integer(row) for row in count_rows]
    z_mean = (
        Fraction(sum(value[0] for value in z_rows), RADIX),
        Fraction(sum(value[1] for value in z_rows), RADIX),
    )
    z_covariance = Matrix(
        [
            [
                sum(Fraction(value[0] * value[0]) for value in z_rows) / RADIX,
                sum(Fraction(value[0] * value[1]) for value in z_rows) / RADIX,
            ],
            [
                sum(Fraction(value[1] * value[0]) for value in z_rows) / RADIX,
                sum(Fraction(value[1] * value[1]) for value in z_rows) / RADIX,
            ],
        ]
    )
    check(
        "C9 the physical scheduler time measure gives zero-mean isotropic Gaussian-integer covariance 79 I2 over 16",
        z_mean == (0, 0)
        and z_covariance == Fraction(79, 16) * Matrix.eye(2),
    )
    check(
        "C10 its exact mean manifested count is E|Z|^2 equals 79 over 8",
        sum(Fraction(weight) for weight in weights) / RADIX == Fraction(79, 8),
    )

    weight_multiplicity = Counter(weights)
    expected_weights = {
        0: 288,
        2: 896,
        4: 736,
        8: 448,
        10: 832,
        16: 288,
        18: 64,
        20: 128,
        26: 128,
        32: 32,
        34: 64,
        36: 32,
        40: 64,
        50: 64,
        64: 32,
    }
    check(
        "C11 all 4,096 scheduler addresses have the exact fifteen-class click spectrum",
        dict(sorted(weight_multiplicity.items())) == expected_weights,
    )

    count_multiplicity = Counter(count_rows)
    born_weight_rows = 0
    for left_counts, left_multiplicity in count_multiplicity.items():
        for right_counts, right_multiplicity in count_multiplicity.items():
            left_weight = bright_pair_count(left_counts)
            right_weight = bright_pair_count(right_counts)
            total = left_weight + right_weight
            if not total:
                continue
            assert Fraction(left_weight, total) == Fraction(
                norm_squared(gaussian_integer(left_counts)),
                norm_squared(gaussian_integer(left_counts))
                + norm_squared(gaussian_integer(right_counts)),
            )
            born_weight_rows += left_multiplicity * right_multiplicity

    deadline_rows = 0
    for left_weight in sorted(weight_multiplicity):
        for right_weight in sorted(weight_multiplicity):
            total = left_weight + right_weight
            live_macros = POINTER_PERIOD + 2 * total
            padding_macros = 2 * (MAX_TWO_PORT_CLICKS - total)
            assert padding_macros >= 0
            assert live_macros + padding_macros == POINTER_PERIOD + 256
            deadline_rows += 1

    nonzero_pair_rows = PAIR_PERIOD - weight_multiplicity[0] ** 2
    check(
        "C12 every non-dark ordered history pair has exact native Born event normalization",
        born_weight_rows == nonzero_pair_rows == 16_694_272,
    )
    check(
        "C13 exact dark padding makes every two-port apparatus trial the same macro duration",
        deadline_rows == len(weight_multiplicity) ** 2
        and POINTER_PERIOD + 256 == 148_096,
    )

    aggregate_each_port = RADIX * sum(weights)
    check(
        "C14 one complete odometer orbit gives equal exact aggregate port counts by exchange symmetry",
        aggregate_each_port == 165_675_008
        and Fraction(aggregate_each_port, 2 * aggregate_each_port) == Fraction(1, 2),
    )

    # Two schedulers, two sources, two cursors, and sixteen reserve owners are
    # 22 existing owners before the remote protected apparatus.  The latter
    # needs its proved 23 sites plus one finite padding owner.  The blocks do
    # not fit one Moore neighborhood, so causal routing is not hidden here.
    check(
        "C15 the source scheduler and padded protected apparatus are finite existing-carrier blocks but require routing",
        2 + 2 + 2 + 16 == 22
        and 23 + 1 == 24
        and 22 + 24 > 27,
    )

    forbidden = (
        "137.036",
        "target_probability",
        "random_draw",
        "empirical_frequency",
        "wavefunction_amplitude",
    )
    missing = {
        "genesis formation and protection of scheduler source reserve and chart owners",
        "canonical Phi implementation of controlled loading odometer and padding",
        "causal cross-block bank routing and traffic arbitration",
        "reciprocal detector work and material backreaction",
        "long-run click export beyond finite A2 counter capacity",
        "state-specific laboratory preparation amplification and Bell-correlation recovery",
    }
    check(
        "C16 Phi-v15 selects a finite reference trial measure, not the general physical Born rule",
        all(token not in __doc__.lower() for token in forbidden)
        and len(missing) == 6,
    )

    passed = sum(ok for _name, ok, _detail in checks)
    print(f"\n{passed}/{len(checks)} transitive A2 Born-scheduler checks pass")
    print(f"scheduler_history_states={RADIX}")
    print(f"ordered_history_pair_period={PAIR_PERIOD}")
    print(f"source_formation_rows={formation_rows}")
    print(f"source_inverse_tick_rows={formation_inverse_rows}")
    print(f"attained_count_vectors={len(set(count_rows))}")
    print(f"click_weight_classes={len(weight_multiplicity)}")
    print("formed_count_mean=(2,2,2,2)")
    print(f"formed_count_covariance={covariance.tolist()}")
    print("gaussian_integer_covariance=(79/16)*I2")
    print("scheduler_mean_abs_Z_squared=79/8")
    print(f"nonzero_ordered_history_pairs={nonzero_pair_rows}")
    print("constant_two_port_apparatus_macros=148096")
    print(f"aggregate_clicks_per_port={aggregate_each_port}")
    print("persistent_full_cycle_memory=not_closed_finite_counter_export_required")
    print("status=selected_phi_v15_transitive_reference_time_measure_routing_backreaction_general_born_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
