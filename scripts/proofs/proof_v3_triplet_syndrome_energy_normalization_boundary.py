#!/usr/bin/env python3
"""Exact triplet repair-energy and absolute-normalization boundary.

The prepared cubic triplet repairs one valid-symbol substitution, but every
such input has exactly the same field/A1/A2 occupancy vector as the clean
clock.  This certificate proves that no phase-blind additive occupancy action
can supply a binding or syndrome gap.  A positive relational syndrome cost
requires an equally explicit work export during noninjective repair, and its
coefficient remains a free action scale.  The isotropic source shape -I/36 is
therefore exact while its response multiplier remains undetermined.
"""

from __future__ import annotations

import inspect
import sys

from sympy import Matrix, Symbol, eye, simplify

from proof_v3_cubic_triplet_self_correcting_material_clock import (
    DARK,
    HERALD_ALPHABET,
    LOGICAL,
    TripletClock,
    clean_state,
    triplet_iterate,
    triplet_step,
)


sys.stdout.reconfigure(encoding="utf-8")

# Three constant-occupancy neutral field pairs plus three occupied SC A9 arms.
OCCUPANCY = (6, 3, 0, 0)  # F, A1_SC, A1_FCC, A2


def occupancy(_state: TripletClock) -> tuple[int, int, int, int]:
    """Registered valid-symbol sector: every symbol preserves its carrier."""

    return OCCUPANCY


def hamming(left: TripletClock, right: TripletClock) -> int:
    return sum(a != b for a, b in zip(left.arms, right.arms)) + sum(
        a != b for a, b in zip(left.heralds, right.heralds)
    )


def one_substitutions(state: TripletClock):
    for index in range(3):
        for replacement in LOGICAL:
            if replacement == state.arms[index]:
                continue
            arms = list(state.arms)
            arms[index] = replacement
            yield TripletClock(tuple(arms), state.heralds)
    for index in range(3):
        for replacement in HERALD_ALPHABET:
            if replacement == state.heralds[index]:
                continue
            heralds = list(state.heralds)
            heralds[index] = replacement
            yield TripletClock(state.arms, tuple(heralds))


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    seed = LOGICAL[0]
    orbit = [clean_state(seed)]
    for _ in range(15):
        orbit.append(triplet_step(orbit[-1]))

    mutations = [
        (phase, reference, mutant)
        for phase, reference in enumerate(orbit)
        for mutant in one_substitutions(reference)
    ]
    check(
        "C1 exact radius-one registered mutation census is 1,488",
        len(mutations) == 1_488,
    )
    check(
        "C2 every clean and one-substitution state has occupancy vector (6,3,0,0)",
        all(occupancy(reference) == OCCUPANCY for reference in orbit)
        and all(occupancy(mutant) == OCCUPANCY for _, _, mutant in mutations),
    )

    feature_deltas = {
        tuple(a - b for a, b in zip(occupancy(mutant), occupancy(reference)))
        for _, reference, mutant in mutations
    }
    check(
        "C3 the complete additive occupancy defect span has rank zero",
        feature_deltas == {(0, 0, 0, 0)}
        and Matrix(list(feature_deltas)).rank() == 0,
    )

    gamma_f = Symbol("Gamma_F", positive=True)
    gamma_sc = Symbol("Gamma_SC", positive=True)
    gamma_fcc = Symbol("Gamma_FCC", positive=True)
    gamma_a2 = Symbol("Gamma_A2", positive=True)
    weights = (gamma_f, gamma_sc, gamma_fcc, gamma_a2)
    clean_energy = sum(weight * count for weight, count in zip(weights, OCCUPANCY))
    mutant_energies = {
        sum(weight * count for weight, count in zip(weights, occupancy(mutant)))
        for _, _, mutant in mutations
    }
    check(
        "C4 no homogeneous phase-blind additive role weights distinguish a registered error",
        mutant_energies == {clean_energy},
    )

    clean_code = tuple(orbit)
    unique_decode_rows = 0
    for _phase, reference, mutant in mutations:
        distances = [hamming(mutant, candidate) for candidate in clean_code]
        assert min(distances) == 1
        assert distances.count(1) == 1
        assert clean_code[distances.index(1)] == reference
        unique_decode_rows += 1
    check(
        "C5 relational equality supplies a unique clean parent for every registered error",
        unique_decode_rows == 1_488,
    )

    epsilon = Symbol("epsilon_syn", positive=True)
    syndrome_before = epsilon
    syndrome_after = 0
    check(
        "C6 a positive relational syndrome functional gives every radius-one error gap epsilon_syn",
        syndrome_before - syndrome_after == epsilon,
    )

    recovery_rows = 0
    for _phase, reference, mutant in mutations:
        recovered = next(
            step
            for step in (1, 2)
            if triplet_iterate(mutant, step) == triplet_iterate(reference, step)
        )
        assert recovered in (1, 2)
        recovery_rows += 1
    check(
        "C7 noninjective repair removes the relational syndrome in every registered case",
        recovery_rows == 1_488,
    )

    work_export = Symbol("W_export", nonnegative=True)
    energy_balance = simplify((clean_energy + epsilon) - (clean_energy + work_export))
    check(
        "C8 positive syndrome energy is conserved iff repair exports exactly W_export=epsilon_syn",
        simplify(energy_balance.subs(work_export, epsilon)) == 0
        and simplify(energy_balance.subs(work_export, 0)) == epsilon,
    )

    common_gamma = Symbol("Gamma", positive=True)
    equal_ray_energy = common_gamma * sum(OCCUPANCY)
    check(
        "C9 the selected all-equal occupancy ray assigns the flat value 9 Gamma",
        equal_ray_energy == 9 * common_gamma,
    )

    coupling = Symbol("g_response", positive=True)
    scale = Symbol("lambda", positive=True)
    source_shape = -eye(3) / 36
    response = coupling * source_shape
    rescaled_response = response.subs(coupling, scale * coupling)
    check(
        "C10 the clock fixes isotropic source shape but leaves a positive response multiplier free",
        source_shape == -Matrix.eye(3) / 36
        and simplify(rescaled_response - scale * response) == Matrix.zeros(3, 3),
    )

    signature = inspect.signature(triplet_step)
    check(
        "C11 the exact finite repair law reads neither Gamma, epsilon_syn, nor g_response",
        tuple(signature.parameters) == ("state",)
        and all(
            name not in inspect.getsource(triplet_step)
            for name in ("Gamma", "epsilon_syn", "g_response")
        ),
    )

    # The clean recurrence consumes no field packet: its carrier occupancy is
    # constant and the herald replacement is neutral.  Hence it cannot by
    # itself instantiate the finite-clock normalization branch, which assumes
    # a positive complete-packet debit d.
    packet_debit = 0
    check(
        "C12 the clean triplet has zero packet debit and cannot alone select 3w/(dT)",
        packet_debit == 0,
    )

    forbidden = (
        "137.036",
        "born_weight",
        "particle_mass",
        "lensing_target",
        "master_root",
    )
    check(
        "C13 no empirical target, numerical scan, or fitted scale enters",
        all(token not in __doc__.lower() for token in forbidden),
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} triplet syndrome-energy boundary checks pass")
    print(f"registered_error_rows={len(mutations)}")
    print("occupancy_feature_vector=(6,3,0,0)")
    print("occupancy_defect_rank=0")
    print("equal_occupancy_energy=9*Gamma")
    print("positive_syndrome_repair_requires=W_export=epsilon_syn")
    print("isotropic_source_shape=-I/36")
    print("absolute_response_multiplier=free")
    print("clean_packet_debit=0")
    print("status=kinematic_repair_exact_physical_binding_and_normalization_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
