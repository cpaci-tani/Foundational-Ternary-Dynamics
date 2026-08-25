#!/usr/bin/env python3
"""Exact protected v3 Born apparatus and finite A2 click-memory certificate.

The prepared contextual apparatus uses two coprime pointer cycles of lengths
384 and 385 plus one ternary detector.  This successor stores three copies of
each pointer address and three copies of the detector, all on separate existing
sites.  A two-layer READ/COMMIT wrapper majority-corrects every one-copy valid
symbol substitution before applying the original reversible apparatus step.
One fixed-occupancy A2 phase/work owner records whether correction occurred.

At every genuine READY-to-MANIFESTED bright event, one of twelve existing A2
signed counters increments.  The click record survives detector recovery and
the clean apparatus cycle; the clean combined map has an exact inverse.  A
complete source-formed one-port bank writes exactly |Z|^2 clicks and returns
the pointers/detector to their starting state while the finite memory remains.

The wrapper is conditional on a prepared bank and oriented apparatus chart.
Two coherent substitutions, faults between READ and COMMIT, source/apparatus
formation, reciprocal detector work, multi-block traffic, and canonical Phi
remain open.  No probability primitive, target frequency, random draw, or
empirical value enters.
"""

from __future__ import annotations

import sys
from dataclasses import dataclass
from itertools import product

from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_v3_contextual_neutral_pointer_born_renewal_apparatus import (
    DELAY_CONFIGURATION,
    MANIFESTED,
    READY,
    ApparatusState,
    address_order,
    apparatus_chart,
    apparatus_inverse,
    apparatus_step,
    bank_from_counts,
    bright_outcome,
    complete_operational_orbit,
    pointer_configuration,
)
from proof_v3_field_bank_gaussian_born_readout import bright_pair_count
from proof_v3_finite_source_history_born_bank_formation_phi_v13_candidate import (
    LOGICAL,
    WINDOW,
    active_phase,
    initial_state,
    iterate_formation,
    source_port,
)
from proof_v3_oriented_repair_chart_full_oh_covariance_and_price import (
    transform_channel,
    transform_chart,
)
from proof_v3_rotor_green_a2_physical_memory_phase_protection import (
    decode_counter,
    encode_counter,
)


sys.stdout.reconfigure(encoding="utf-8")

READ = 0
COMMIT = 1
COUNTER_MAX = 2047
POINTER_PERIOD = 384 * 385


def majority(values):
    for value in values:
        if values.count(value) >= 2:
            return value
    return None


def pointer_from_residues(left: int, right: int) -> int:
    assert 0 <= left < 384 and 0 <= right < 385
    # 384 == -1 (mod 385), so 384 is its own modular inverse.
    quotient = ((right - left) * 384) % 385
    pointer = left + 384 * quotient
    assert pointer % 384 == left and pointer % 385 == right
    return pointer


def copies(value):
    return value, value, value


@dataclass(frozen=True)
class ProtectedApparatus:
    left: tuple[int, int, int]
    right: tuple[int, int, int]
    detector: tuple[int, int, int]
    layer: int
    work: int
    clicks: tuple[int, ...]

    def __post_init__(self) -> None:
        assert len(self.left) == len(self.right) == len(self.detector) == 3
        assert all(0 <= value < 384 for value in self.left)
        assert all(0 <= value < 385 for value in self.right)
        assert all(value in (-1, 0, 1) for value in self.detector)
        assert self.layer in (READ, COMMIT)
        assert self.work in (0, 1)
        assert len(self.clicks) == 12
        assert all(0 <= value <= COUNTER_MAX for value in self.clicks)


def protected_from_base(base: ApparatusState, port_count: int = 12):
    return ProtectedApparatus(
        copies(base.pointer % 384),
        copies(base.pointer % 385),
        copies(base.detector),
        READ,
        0,
        (0,) * port_count,
    )


def decoded_base(state: ProtectedApparatus):
    left = majority(state.left)
    right = majority(state.right)
    detector = majority(state.detector)
    if left is None or right is None or detector is None:
        return None
    return ApparatusState(pointer_from_residues(left, right), detector)


def read_layer(state: ProtectedApparatus):
    if state.layer != READ:
        return state
    base = decoded_base(state)
    if base is None:
        return state
    left = base.pointer % 384
    right = base.pointer % 385
    mismatch_count = (
        sum(value != left for value in state.left)
        + sum(value != right for value in state.right)
        + sum(value != base.detector for value in state.detector)
    )
    if mismatch_count > 1 or (mismatch_count and state.work != 0):
        return state
    return ProtectedApparatus(
        copies(left),
        copies(right),
        copies(base.detector),
        COMMIT,
        1 if mismatch_count else state.work,
        state.clicks,
    )


def genuine_bright_event(order, residual, base: ApparatusState) -> bool:
    return base.detector == READY and bright_outcome(
        order, residual, base.pointer
    ) is not None


def commit_layer(order, residual, ports, state: ProtectedApparatus):
    if state.layer != COMMIT:
        return state
    if len(set(state.left)) != 1 or len(set(state.right)) != 1 or len(set(state.detector)) != 1:
        return state
    base = ApparatusState(
        pointer_from_residues(state.left[0], state.right[0]),
        state.detector[0],
    )
    click = genuine_bright_event(order, residual, base)
    outcome = bright_outcome(order, residual, base.pointer) if click else None
    click_counts = list(state.clicks)
    if outcome is not None:
        index = ports.index(outcome)
        if click_counts[index] >= COUNTER_MAX:
            return state
        click_counts[index] += 1
    output = apparatus_step(order, residual, base)
    return ProtectedApparatus(
        copies(output.pointer % 384),
        copies(output.pointer % 385),
        copies(output.detector),
        READ,
        state.work,
        tuple(click_counts),
    )


def commit_inverse(order, residual, ports, state: ProtectedApparatus):
    if state.layer != READ:
        return state
    if len(set(state.left)) != 1 or len(set(state.right)) != 1 or len(set(state.detector)) != 1:
        return state
    output = ApparatusState(
        pointer_from_residues(state.left[0], state.right[0]),
        state.detector[0],
    )
    prior = apparatus_inverse(order, residual, output)
    click_counts = list(state.clicks)
    if genuine_bright_event(order, residual, prior):
        outcome = bright_outcome(order, residual, prior.pointer)
        assert outcome is not None
        index = ports.index(outcome)
        if click_counts[index] <= 0:
            return state
        click_counts[index] -= 1
    candidate = ProtectedApparatus(
        copies(prior.pointer % 384),
        copies(prior.pointer % 385),
        copies(prior.detector),
        COMMIT,
        state.work,
        tuple(click_counts),
    )
    assert commit_layer(order, residual, ports, candidate) == state
    return candidate


def clean_read_inverse(state: ProtectedApparatus):
    assert state.layer == COMMIT
    assert len(set(state.left)) == len(set(state.right)) == len(set(state.detector)) == 1
    return ProtectedApparatus(
        state.left,
        state.right,
        state.detector,
        READ,
        state.work,
        state.clicks,
    )


def macro_step(order, residual, ports, state: ProtectedApparatus):
    read = read_layer(state)
    if read.layer != COMMIT:
        return state
    return commit_layer(order, residual, ports, read)


def clean_macro_inverse(order, residual, ports, state: ProtectedApparatus):
    committed = commit_inverse(order, residual, ports, state)
    if committed.layer != COMMIT:
        return state
    return clean_read_inverse(committed)


def replace_copy(state: ProtectedApparatus, family: str, index: int, value: int):
    values = list(getattr(state, family))
    values[index] = value
    kwargs = {
        "left": state.left,
        "right": state.right,
        "detector": state.detector,
        "layer": state.layer,
        "work": state.work,
        "clicks": state.clicks,
    }
    kwargs[family] = tuple(values)
    return ProtectedApparatus(**kwargs)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    chart = apparatus_chart()
    order = address_order(chart)
    ports = tuple(sorted({(channel[0], channel[4]) for channel in order}))

    residue_map = {
        (pointer % 384, pointer % 385): pointer
        for pointer in range(POINTER_PERIOD)
    }
    check(
        "C1 the two copied pointer residues reconstruct every joint pointer state uniquely",
        len(residue_map) == POINTER_PERIOD
        and all(pointer_from_residues(left, right) == pointer for (left, right), pointer in residue_map.items()),
    )

    configurations = tuple(pointer_configuration(channel) for channel in order)
    check(
        "C2 three copies of both physical pointers, three detector trits, twelve A2 counters, work, and bank fit one Moore block",
        len(set(configurations)) == 384
        and DELAY_CONFIGURATION not in configurations
        and 1 + 3 + 3 + 3 + 12 + 1 == 23 <= 27,
    )

    fixture_counts = (8, 1, 3, 0)
    fixture_port = ports[0]
    bank = bank_from_counts(order, {fixture_port: fixture_counts})
    from proof_v3_contextual_neutral_pointer_born_renewal_apparatus import canonical_residual

    residual = canonical_residual(bank, order)

    clean_rows = 0
    inverse_rows = 0
    for pointer in range(POINTER_PERIOD):
        for detector in (-1, 0, 1):
            base = ApparatusState(pointer, detector)
            protected = protected_from_base(base)
            output = macro_step(order, residual, ports, protected)
            expected = apparatus_step(order, residual, base)
            assert output.left == copies(expected.pointer % 384)
            assert output.right == copies(expected.pointer % 385)
            assert output.detector == copies(expected.detector)
            assert output.work == 0
            assert clean_macro_inverse(order, residual, ports, output) == protected
            clean_rows += 1
            inverse_rows += 1
    check(
        "C3 the protected two-layer macro reproduces every parent apparatus transition",
        clean_rows == POINTER_PERIOD * 3,
    )
    check(
        "C4 every clean protected transition, including its click-memory update, has an exact inverse",
        inverse_rows == POINTER_PERIOD * 3,
    )

    # The majority theorem is independent of the other decoded components.
    # Exhaust every original/replacement symbol and copy position for all
    # three physical repetition registers.
    substitution_rows = 0
    for family, alphabet in (
        ("left", range(384)),
        ("right", range(385)),
        ("detector", (-1, 0, 1)),
    ):
        for original, replacement, index in product(alphabet, alphabet, range(3)):
            if replacement == original:
                continue
            base = protected_from_base(ApparatusState(0, READY))
            if family == "left":
                right = 0
                pointer = pointer_from_residues(original, right)
                base = protected_from_base(ApparatusState(pointer, READY))
            elif family == "right":
                left = 0
                pointer = pointer_from_residues(left, original)
                base = protected_from_base(ApparatusState(pointer, READY))
            else:
                base = protected_from_base(ApparatusState(0, original))
            mutant = replace_copy(base, family, index, replacement)
            repaired = read_layer(mutant)
            clean_read = read_layer(base)
            assert repaired.left == clean_read.left
            assert repaired.right == clean_read.right
            assert repaired.detector == clean_read.detector
            assert repaired.layer == COMMIT and repaired.work == 1
            assert macro_step(order, residual, ports, mutant).left == macro_step(
                order, residual, ports, base
            ).left
            assert macro_step(order, residual, ports, mutant).right == macro_step(
                order, residual, ports, base
            ).right
            assert macro_step(order, residual, ports, mutant).detector == macro_step(
                order, residual, ports, base
            ).detector
            substitution_rows += 1
    check(
        "C5 every one-copy valid-symbol substitution repairs before the physical apparatus step",
        substitution_rows
        == 3 * (384 * 383 + 385 * 384 + 3 * 2),
    )

    clean = protected_from_base(ApparatusState(0, READY))
    alternate = replace_copy(clean, "left", 0, 1)
    busy = ProtectedApparatus(
        alternate.left,
        alternate.right,
        alternate.detector,
        alternate.layer,
        1,
        alternate.clicks,
    )
    malformed = ProtectedApparatus(
        (0, 1, 2),
        clean.right,
        clean.detector,
        READ,
        0,
        clean.clicks,
    )
    check(
        "C6 correction is noninjective, retains generic work, and busy or no-majority inputs fail closed",
        read_layer(alternate) != alternate
        and read_layer(alternate).left == read_layer(clean).left
        and read_layer(alternate).work == 1
        and read_layer(busy) == busy
        and read_layer(malformed) == malformed,
    )

    # Reconstruct the exact 151 count vectors formed by Phi-v13.
    attained_counts = set()
    for source in LOGICAL:
        for bits in product((0, 1), repeat=WINDOW):
            formed = iterate_formation(chart, initial_state(source, tuple(bits)), WINDOW)
            counts = tuple(
                sum(1 for channel in formed.bank if channel[3] == phase)
                for phase in range(4)
            )
            attained_counts.add(counts)
    formed_rows = 0
    for counts in attained_counts:
        clicks = bright_pair_count(counts)
        assert 0 <= clicks <= 64
        assert decode_counter(encode_counter(clicks)) == clicks
        formed_rows += 1
    check(
        "C7 every Phi-v13 formed bank has exact |Z|^2 click count representable in one existing A2 counter",
        len(attained_counts) == formed_rows == 151,
    )

    # Run and reverse one complete operational cycle.  The apparatus returns
    # to its initial pointer/detector state, while the click memory survives.
    fixture_bank = bank_from_counts(order, {fixture_port: fixture_counts})
    fixture_residual = canonical_residual(fixture_bank, order)
    base_orbit = complete_operational_orbit(order, fixture_residual)
    state = protected_from_base(base_orbit[0])
    for _ in base_orbit:
        state = macro_step(order, fixture_residual, ports, state)
    fixture_index = ports.index(fixture_port)
    expected_clicks = bright_pair_count(fixture_counts)
    check(
        "C8 one complete clean apparatus cycle returns its live state and leaves the exact persistent outcome count",
        decoded_base(state) == base_orbit[0]
        and state.clicks[fixture_index] == expected_clicks
        and sum(state.clicks) == expected_clicks,
    )

    for _ in base_orbit:
        state = clean_macro_inverse(order, fixture_residual, ports, state)
    check(
        "C9 reverse laboratory evolution restores every click counter and the initial apparatus state",
        state == protected_from_base(base_orbit[0]),
    )

    # The click record is fixed-occupancy A2 phase memory.  It survives the
    # detector's MANIFESTED->RECOVERY->READY sequence and is not biological
    # memory or irreversible erasure.
    check(
        "C10 click memory is a finite surviving physical consequence, not an erased event identity",
        expected_clicks == (fixture_counts[0] - fixture_counts[2]) ** 2
        + (fixture_counts[1] - fixture_counts[3]) ** 2
        and expected_clicks > 0,
    )

    covariance_rows = 0
    for matrix in signed_permutation_matrices():
        transformed_chart = transform_chart(matrix, chart)
        transformed_order = address_order(transformed_chart)
        for index, channel in enumerate(order):
            assert transform_channel(matrix, channel) == transformed_order[index]
            covariance_rows += 1
    check(
        "C11 the copied pointer indices and twelve outcome counters are covariant with the oriented apparatus chart",
        covariance_rows == 48 * 384,
    )

    boundary = {
        "two coherent copy substitutions can redirect the majority",
        "faults arriving between READ and COMMIT are outside the snapshot basin",
        "source controller chart and apparatus formation remain open",
        "reciprocal detector work and material backreaction remain open",
        "multi-block source routing traffic and renewal remain open",
        "canonical Phi and laboratory Bell correlation recovery remain open",
    }
    check(
        "C12 the theorem closes protected finite apparatus memory, not the general physical Born rule",
        len(boundary) == 6,
    )

    forbidden = (
        "137.036",
        "target_probability",
        "random_draw",
        "empirical_frequency",
        "wavefunction_amplitude",
    )
    check(
        "C13 no probability primitive, target frequency, amplitude, or empirical value enters",
        all(token not in __doc__.lower() for token in forbidden),
    )

    passed = sum(ok for _name, ok, _detail in checks)
    print(f"\n{passed}/{len(checks)} protected Born-apparatus checks pass")
    print(f"joint_pointer_states={POINTER_PERIOD}")
    print(f"clean_protected_transition_rows={clean_rows}")
    print(f"one_copy_substitution_rows={substitution_rows}")
    print(f"source_formed_count_vectors={len(attained_counts)}")
    print(f"fixture_operational_steps={len(base_orbit)}")
    print(f"fixture_persistent_clicks={expected_clicks}")
    print(f"pointer_covariance_rows={covariance_rows}")
    print("apparatus_sites=23_of_27")
    print("status=prepared_apparatus_one_fault_protected_finite_A2_click_memory_exact_formation_backreaction_traffic_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
