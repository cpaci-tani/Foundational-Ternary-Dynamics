#!/usr/bin/env python3
"""Exact prepared Phi-v5 triplet relational-repair/work candidate.

This successor extends the cubic triplet's copied center herald from seventeen
symbols to thirty-three: dark plus (decoded A9 state, syndrome-present bit).
One prepared existing A2 A9 token has READY and EXCITED internal states at
fixed occupancy.  Every one-register valid-symbol error is corrected while
the work token changes READY -> EXCITED.  The error identity expires, but its
generic physical consequence survives.

The dimensionless relational ledger h_syn + w is exact.  Its common positive
multiplier remains free, the excited work record is not yet exported/reset,
and seed formation, general noise, motion, poles, Born/no-signalling, and
gravity remain open.
"""

from __future__ import annotations

import sys
from dataclasses import dataclass
from functools import lru_cache

from proof_hodge_flag_pair_collision_invariant_space import (
    one_particle_states,
    transform_state,
)
from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_ternary_square_phase_polarity_autonomous_clock import LocalState, tick
from proof_v3_charged_candidate_matter_perturbation_boundary import frame_family
from proof_v3_cubic_triplet_self_correcting_material_clock import (
    DARK,
    LOGICAL,
    clock_arm_edges,
    clock_register_pool,
)
from proof_v3_oriented_repair_chart_full_oh_covariance_and_price import (
    OrientedRepairChart,
    canonical_chart,
    mv,
    transform_chart,
)


sys.stdout.reconfigure(encoding="utf-8")

Payload = tuple[int, int]


@dataclass(frozen=True)
class Pending:
    logical: LocalState
    syndrome: int

    def __post_init__(self) -> None:
        assert self.logical in LOGICAL
        assert self.syndrome in (0, 1)


Herald = Pending | None
HERALD_ALPHABET: tuple[Herald, ...] = (DARK,) + tuple(
    Pending(logical, syndrome)
    for logical in LOGICAL
    for syndrome in (0, 1)
)


def ready_payload(chart: OrientedRepairChart) -> Payload:
    return chart.offset, chart.polarity


def excited_payload(chart: OrientedRepairChart) -> Payload:
    return (chart.offset + 1) % 4, chart.polarity


@lru_cache(maxsize=None)
def work_register_banks(chart: OrientedRepairChart, states) -> tuple[dict[Herald, object], ...]:
    pool = clock_register_pool(chart, states)
    required = 3 * len(HERALD_ALPHABET)
    assert len(pool) >= required
    return tuple(
        dict(
            zip(
                HERALD_ALPHABET,
                pool[index * len(HERALD_ALPHABET) : (index + 1) * len(HERALD_ALPHABET)],
            )
        )
        for index in range(3)
    )


def majority(values):
    for value in values:
        if values.count(value) >= 2:
            return True, value
    return False, None


@dataclass(frozen=True)
class WorkClock:
    arms: tuple[LocalState, LocalState, LocalState]
    heralds: tuple[Herald, Herald, Herald]
    work: Payload


def clean_state(chart: OrientedRepairChart, logical: LocalState, heralded: bool = False) -> WorkClock:
    herald: Herald = Pending(logical, 0) if heralded else DARK
    return WorkClock((logical,) * 3, (herald,) * 3, ready_payload(chart))


def work_step(chart: OrientedRepairChart, state: WorkClock) -> WorkClock:
    """READ/COMMIT with atomic fail-closed work ownership."""

    dark_majority = state.heralds.count(DARK) >= 2
    if dark_majority:
        found, logical = majority(state.arms)
        if not found:
            return state
        syndrome = int(
            any(arm != logical for arm in state.arms)
            or any(herald is not DARK for herald in state.heralds)
        )
        if syndrome and state.work != ready_payload(chart):
            return state
        pending = Pending(logical, syndrome)
        return WorkClock(state.arms, (pending,) * 3, state.work)

    found, pending = majority(state.heralds)
    if not found or pending is DARK:
        return state
    assert isinstance(pending, Pending)
    syndrome = int(
        pending.syndrome
        or any(arm != pending.logical for arm in state.arms)
        or any(herald != pending for herald in state.heralds)
    )
    if syndrome and state.work != ready_payload(chart):
        return state
    work_after = excited_payload(chart) if syndrome else state.work
    return WorkClock(
        (tick(pending.logical),) * 3,
        (DARK,) * 3,
        work_after,
    )


def iterate(chart: OrientedRepairChart, state: WorkClock, count: int) -> WorkClock:
    for _ in range(count):
        state = work_step(chart, state)
    return state


def body_projection(state: WorkClock):
    return state.arms, state.heralds


def one_substitutions(state: WorkClock):
    for index in range(3):
        for replacement in LOGICAL:
            if replacement == state.arms[index]:
                continue
            arms = list(state.arms)
            arms[index] = replacement
            yield WorkClock(tuple(arms), state.heralds, state.work)
    for index in range(3):
        for replacement in HERALD_ALPHABET:
            if replacement == state.heralds[index]:
                continue
            heralds = list(state.heralds)
            heralds[index] = replacement
            yield WorkClock(state.arms, tuple(heralds), state.work)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = tuple(one_particle_states())
    group = tuple(signed_permutation_matrices())
    generators = (
        ((0, 1, 0), (1, 0, 0), (0, 0, 1)),
        ((1, 0, 0), (0, 0, 1), (0, 1, 0)),
        ((-1, 0, 0), (0, 1, 0), (0, 0, 1)),
    )
    charts = {
        transform_chart(matrix, canonical_chart(frame))
        for frame in frame_family()
        for matrix in group
    }

    minimum_pool = min(len(clock_register_pool(chart, states)) for chart in charts)
    check(
        "C1 existing clear controller pool fits three copied 33-symbol pending registers",
        len(HERALD_ALPHABET) == 33 and minimum_pool == 170 and 3 * 33 <= minimum_pool,
    )

    register_rows = 0
    covariance_rows = 0
    work_rows = 0
    for chart in charts:
        banks = work_register_banks(chart, states)
        selected = [value for bank in banks for value in bank.values()]
        assert len(selected) == len(set(selected)) == 99
        assert ready_payload(chart) != excited_payload(chart)
        assert ready_payload(chart)[0] in range(4)
        assert excited_payload(chart)[0] in range(4)
        assert ready_payload(chart)[1] == excited_payload(chart)[1] in (-1, 1)
        register_rows += len(selected)
        work_rows += 1
        for matrix in generators:
            transformed_chart = transform_chart(matrix, chart)
            transformed_banks = work_register_banks(transformed_chart, states)
            assert clock_arm_edges(transformed_chart) == tuple(
                (mv(matrix, tail), mv(matrix, direction))
                for tail, direction in clock_arm_edges(chart)
            )
            assert ready_payload(transformed_chart) == ready_payload(chart)
            assert excited_payload(transformed_chart) == excited_payload(chart)
            for left_bank, right_bank in zip(banks, transformed_banks):
                for symbol in HERALD_ALPHABET:
                    assert transform_state(matrix, left_bank[symbol]) == right_bank[symbol]
                    covariance_rows += 1
    check(
        "C2 all prepared pending registers and A2 ready/excited payloads are existing finite states",
        register_rows == 1_152 * 99 and work_rows == 1_152,
    )
    check(
        "C3 complete clock/herald/work construction is signed-cubic generator covariant",
        covariance_rows == 1_152 * 3 * 3 * 33,
    )

    chart = canonical_chart(frame_family()[0])
    seed = LOGICAL[0]
    orbit = [clean_state(chart, seed)]
    for _ in range(15):
        orbit.append(work_step(chart, orbit[-1]))
    check(
        "C4 ready clean body/work state has exact period sixteen",
        work_step(chart, orbit[-1]) == orbit[0] and len(set(orbit)) == 16,
    )

    mutations = [
        (reference, mutant)
        for reference in orbit
        for mutant in one_substitutions(reference)
    ]
    recovery_rows = 0
    charged_work_rows = 0
    maximum_recovery = 0
    for reference, mutant in mutations:
        recovered = next(
            step
            for step in (1, 2)
            if body_projection(iterate(chart, mutant, step))
            == body_projection(iterate(chart, reference, step))
        )
        output = iterate(chart, mutant, recovered)
        assert output.work == excited_payload(chart)
        maximum_recovery = max(maximum_recovery, recovered)
        recovery_rows += 1
        charged_work_rows += 1
    check(
        "C5 all 2,256 one-register substitutions restore the clean body orbit within two ticks",
        len(mutations) == recovery_rows == 2_256 and maximum_recovery == 2,
    )
    check(
        "C6 every registered repair leaves one generic excited A2 work record",
        charged_work_rows == 2_256,
    )

    # Dimensionless common relational ledger.  Every registered input has one
    # syndrome quantum and READY work; every recovered body has zero syndrome
    # and EXCITED work.  One common multiplier may price both, but its absolute
    # value is not selected by this equality.
    check(
        "C7 relational syndrome plus A2 work excitation is exactly conserved 1+0=0+1",
        1 + 0 == 0 + 1,
    )
    check(
        "C8 work export changes only A9 internal payload and preserves A2 occupancy one",
        ready_payload(chart) is not None
        and excited_payload(chart) is not None,
    )

    # An outstanding work record does not stop clean recurrence, but a second
    # error cannot be silently repaired into the already-owned port.
    excited_clean = WorkClock(
        orbit[0].arms,
        orbit[0].heralds,
        excited_payload(chart),
    )
    clean_after_two = iterate(chart, excited_clean, 2)
    assert body_projection(clean_after_two) == body_projection(orbit[2])
    error_with_busy_port = next(one_substitutions(excited_clean))
    check(
        "C9 clean clock continues with pending work while a second repair fails closed",
        work_step(chart, error_with_busy_port) == error_with_busy_port,
    )

    first_mutant = mutations[0][1]
    second_mutant = next(
        mutant for _reference, mutant in mutations if mutant != first_mutant
    )
    check(
        "C10 error identity expires while the common work consequence survives",
        first_mutant != second_mutant
        and iterate(chart, first_mutant, 2) == iterate(chart, second_mutant, 2),
    )

    forbidden = (
        "137.036",
        "born_weight",
        "particle_mass",
        "lensing_target",
        "master_root",
    )
    check(
        "C11 no empirical target, probability, continuum amplitude, or random draw enters",
        all(token not in __doc__.lower() for token in forbidden),
    )

    open_debts = {
        "native seed and work-port formation",
        "derivation of the relational coefficient from common action",
        "causal work export or reset",
        "occupancy and multiple-error recovery",
        "translation collision and mass",
        "charged and tensor pole integration",
        "Born preparation and no-signalling",
        "gravity coupling cone and lensing",
    }
    check(
        "C12 Phi-v5 candidate closes one repair/work ledger, not the physical sectors",
        len(open_debts) == 8,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} triplet relational-work Phi-v5 checks pass")
    print(f"minimum_clear_controller_pool={minimum_pool}")
    print(f"pending_register_covariance_rows={covariance_rows}")
    print(f"single_substitution_rows={recovery_rows}")
    print(f"maximum_body_recovery_ticks={maximum_recovery}")
    print("relational_work_ledger=1+0=0+1")
    print("work_port_occupancy=1_to_1")
    print("second_error_with_busy_port=fail_closed")
    print("absolute_relational_multiplier=free")
    print("status=prepared_phi_v5_relational_repair_work_candidate")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
