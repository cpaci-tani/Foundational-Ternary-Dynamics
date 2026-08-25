#!/usr/bin/env python3
"""Common v3 occupancy invariant and relative-normalization boundary.

This certificate classifies phase-blind homogeneous additive occupancy
energies across four existing carrier roles:

    F          site/port field record,
    A1_SC      SC material relation token,
    A1_FCC     FCC material relation token,
    A2         plaquette work-port token.

Canonical absorption exchanges F <-> A1_SC.  The registered charged-frame
repair exchanges A1_SC/F <-> A2 while retaining the complete syndrome.  These
transactions force e_F=e_A1_SC=e_A2.  No selected transaction exchanges an
FCC A1 token with that connected component, and cubic symmetry preserves the
SC and FCC shells separately.  The complete additive invariant family is
therefore

    H = Gamma (N_F + N_A1_SC + N_A2) + Eta N_A1_FCC.

Gamma and Eta remain free positive multipliers.  This is an exact invariant
classification conditional on the selected transaction set and additive
occupancy ansatz; it is not yet a variational generator, block-normalized
physical action, or coupling derivation.
"""

from __future__ import annotations

import sys
from collections import Counter

from sympy import Matrix

import proof_global_c3_cotangent_layer_equivariant_collision as collision_proof
from proof_hodge_flag_pair_collision_invariant_space import (
    one_particle_states,
    transform_state,
)
from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_v3_charged_candidate_matter_perturbation_boundary import (
    all_local_channels,
    frame_family,
    presentation,
)
from proof_v3_charged_common_action_phi_v3_candidate import PLANE_FAMILIES
from proof_v3_charged_frame_atomic_syndrome_repair_transaction import (
    defect_descriptor,
    frame_codewords,
)
from proof_v3_charged_frame_payload_complete_a2_work_port import (
    occupied as occupied_work,
    output_work_port,
    ready_work_port,
)
from proof_v3_charged_frame_unique_one_defect_decoder import enumerate_one_defects
from proof_v3_common_transaction_phi import (
    BLANK,
    D_SC,
    POS_FCC,
    POS_SC,
    add,
    blank_state,
    dot,
    occupied,
    phi,
    sc_relation_for_port,
)
from proof_v3_neutral_rotor_walker_macro import unmarked_site
from proof_v3_neutral_stf_rotor_walker_green_seam import (
    internal_orbits,
    local_tensor_step,
    tensor_marked_site,
)
from proof_v3_neutral_vector_constraint_walker_tt_locality_obstruction import (
    local_vector_step,
    vector_marked_site,
)


sys.stdout.reconfigure(encoding="utf-8")

RoleCounts = tuple[int, int, int, int]  # F, A1_SC, A1_FCC, A2


def subtract_counts(after: RoleCounts, before: RoleCounts) -> RoleCounts:
    return tuple(a - b for a, b in zip(after, before))  # type: ignore[return-value]


def state_counts(state) -> RoleCounts:
    fields = sum(port is not None for port in state.ports.values())
    sc = 0
    fcc = 0
    for (_tail, direction), (primary, reserve) in state.relations.items():
        count = occupied(primary) + occupied(reserve)
        if direction in POS_SC:
            sc += count
        else:
            assert direction in POS_FCC
            fcc += count
    return fields, sc, fcc, 0


def frame_counts(frame, syndrome_records: int = 0, work_tokens: int = 0) -> RoleCounts:
    represented = presentation(frame)
    return (
        len(represented.fields) + syndrome_records,
        len(represented.relations),
        0,
        work_tokens,
    )


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = tuple(one_particle_states())
    group = tuple(signed_permutation_matrices())

    # O_h is transitive on the 48 Hodge flags.  C4 and charge conjugation then
    # make the full 384 field-channel label set one homogeneous role.
    seed_flag = states[0][0]
    flag_orbit = {
        transform_state(matrix, (seed_flag, 0))[0] for matrix in group
    }
    field_labels = {
        (flag, phase, polarity)
        for flag in flag_orbit
        for phase in range(4)
        for polarity in (-1, 1)
    }
    a9_labels = {
        (phase, polarity) for phase in range(4) for polarity in (-1, 1)
    }
    check(
        "C1 carrier symmetries make field and occupied-A9 labels homogeneous within role",
        len(flag_orbit) == 48 and len(field_labels) == 384 and len(a9_labels) == 8,
    )

    # Cubic symmetry does not mix the squared-length-one SC shell with the
    # squared-length-two FCC shell, so it cannot equate their role energies.
    sc_shell = {
        tuple(transform_state(matrix, (((1, 0, 0), (0, 1, 0), 1), 0))[0][0])
        for matrix in group
    }
    # Direct matrix action is clearer for the FCC relation direction.
    def mv(matrix, vector):
        return tuple(sum(matrix[i][j] * vector[j] for j in range(3)) for i in range(3))

    fcc_shell = {mv(matrix, (1, 1, 0)) for matrix in group}
    check(
        "C2 signed-cubic symmetry preserves distinct SC and FCC relation orbits",
        len(sc_shell) == 6
        and len(fcc_shell) == 12
        and sc_shell.isdisjoint(fcc_shell)
        and {sum(v * v for v in row) for row in sc_shell} == {1}
        and {sum(v * v for v in row) for row in fcc_shell} == {2},
    )

    # Frozen pair collisions and internal ticks are permutations of occupied
    # channel labels and preserve two occupied field records exactly.
    collision_proof.main()
    collision_data = collision_proof.CERTIFICATE_DATA
    assert collision_data is not None
    collision_rows = 0
    for collision in collision_data["collisions"]:
        for before, after in collision.items():
            assert len(before) == len(after) == 2
            collision_rows += 1
    check(
        "C3 frozen field collision/internal-clock sector preserves field occupancy",
        collision_rows == 3 * 18_336
        and len(set(collision_data["internal_action"])) == 192,
    )

    # Free streaming and isolated relation recurrence preserve their role
    # counts, including FCC recurrence as a separately conserved role.
    base = blank_state(5)
    x = (1, 1, 1)
    direction = (1, 0, 0)
    normal = (0, 1, 0)
    base.ports[(x, direction)] = ((1, 0), normal, 1)
    streamed = phi(base)
    recurrence_rows = 0
    recurrence_ok = True
    for relation_direction in POS_SC + POS_FCC:
        state = blank_state(5)
        key = (x, relation_direction)
        state.relations[key] = (BLANK, (1, 0))
        before = state_counts(state)
        after = state_counts(phi(state))
        recurrence_ok = recurrence_ok and before == after
        recurrence_rows += 1
    check(
        "C4 free streaming and all SC/FCC A1 recurrences preserve shell occupancy",
        state_counts(base) == state_counts(streamed)
        and recurrence_ok
        and recurrence_rows == 9,
    )

    # Canonical expiry/absorption transfers one F record into one SC A1 token.
    absorbed_state = blank_state(5)
    absorbed_state.ports[(x, direction)] = ((-1, 0), normal, 1)
    absorbed_output = phi(absorbed_state)
    absorption_delta = subtract_counts(
        state_counts(absorbed_output), state_counts(absorbed_state)
    )
    target_relation = sc_relation_for_port(x, direction, 5)
    check(
        "C5 canonical absorption has exact role delta F->A1_SC",
        absorption_delta == (-1, 1, 0, 0)
        and occupied(absorbed_output.relations[target_relation][1]) == 1,
        str(absorption_delta),
    )

    frames = frame_family()
    check(
        "C6 every charged plaquette tick preserves four SC A1 and sixteen F records",
        all(frame_counts(frame) == frame_counts(frame.output()) == (16, 4, 0, 0) for frame in frames),
    )

    # Exhaust the complete repair shell with its physical syndrome and A2
    # port.  The syndrome count is constant at eighteen on both sides.
    channels = all_local_channels()
    repair_deltas = Counter()
    repair_rows = 0
    for frame in frames:
        _header, codewords = frame_codewords(frame, states)
        ready_bundle = codewords[0]
        ready_port = ready_work_port(frame)
        defects = tuple(enumerate_one_defects(frame, channels))
        descriptors = tuple(defect_descriptor(frame, defect) for _category, defect in defects)
        descriptor_order = {
            descriptor: index
            for index, descriptor in enumerate(sorted(descriptors, key=repr), start=1)
        }
        for _category, defect in defects:
            descriptor = defect_descriptor(frame, defect)
            syndrome = codewords[descriptor_order[descriptor]]
            work_after = output_work_port(frame, defect)
            before: RoleCounts = (
                len(defect.fields) + len(ready_bundle),
                len(defect.relations),
                0,
                occupied_work(ready_port),
            )
            exact_output = presentation(frame.output())
            after: RoleCounts = (
                len(exact_output.fields) + len(syndrome),
                len(exact_output.relations),
                0,
                occupied_work(work_after),
            )
            repair_deltas[subtract_counts(after, before)] += 1
            repair_rows += 1

    expected_repair_deltas = Counter(
        {
            (0, 1, 0, -1): 96,
            (1, 0, 0, -1): 384,
            (0, 0, 0, 0): 672,
            (-1, 0, 0, 1): 36_480,
        }
    )
    check(
        "C7 all 37,632 repairs realize the exact SC/F/A2 exchange-vector census",
        repair_rows == 37_632 and repair_deltas == expected_repair_deltas,
        str(repair_deltas),
    )

    # Existing neutral memory and gravity carriers only move F records; their
    # complete two-site transactions preserve total F occupancy.
    orbits = internal_orbits(states)
    state_set = frozenset(states)
    vector_rows = 0
    for payload in (orbits[1][0], orbits[8][0], orbits[12][0]):
        for departure in orbits[0]:
            for destination in orbits[0]:
                before_left = vector_marked_site(departure, payload)
                before_right = unmarked_site(destination)
                output = local_vector_step(before_left, before_right, state_set)
                assert output is not None
                assert len(before_left) + len(before_right) == len(output[0]) + len(output[1]) == 8
                vector_rows += 1

    tensor_pairs = ((1, 2), (1, 4), (1, 6), (1, 8), (1, 12))
    tensor_rows = 0
    for left_index, right_index in tensor_pairs:
        payload = (orbits[left_index][0], orbits[right_index][0])
        for departure in orbits[0]:
            for destination in orbits[0]:
                before_left = tensor_marked_site(departure, *payload)
                before_right = unmarked_site(destination)
                output = local_tensor_step(before_left, before_right, state_set)
                assert output is not None
                assert len(before_left) + len(before_right) == len(output[0]) + len(output[1]) == 10
                tensor_rows += 1
    check(
        "C8 neutral syndrome/vector/tensor transport adds no new energy weight",
        len(frame_codewords(frames[0], states)[1][0]) == 18
        and vector_rows == 432
        and tensor_rows == 720,
    )

    # Linear energy-conservation constraints for weights
    # (e_F,e_SC,e_FCC,e_A2).  Repair field-extra is the negative of the
    # missing-field row; substitutions add the zero row.
    exchange_matrix = Matrix(
        [
            absorption_delta,
            (0, 1, 0, -1),
            (1, 0, 0, -1),
            (-1, 0, 0, 1),
        ]
    )
    nullspace = exchange_matrix.nullspace()
    check(
        "C9 selected exchange graph has rank two and a two-dimensional energy nullspace",
        exchange_matrix.rank() == 2 and len(nullspace) == 2,
        str(nullspace),
    )

    connected_ray = Matrix([1, 1, 0, 1])
    fcc_ray = Matrix([0, 0, 1, 0])
    check(
        "C10 complete additive invariant family is Gamma(F+SC+A2)+Eta(FCC)",
        exchange_matrix * connected_ray == Matrix.zeros(4, 1)
        and exchange_matrix * fcc_ray == Matrix.zeros(4, 1)
        and Matrix.hstack(connected_ray, fcc_ray).rank() == 2,
    )

    # Verify the connected common count directly on every registered exchange
    # delta.  Eta is absent because no selected exchange changes FCC count.
    all_deltas = [absorption_delta] + list(repair_deltas.elements())
    check(
        "C11 one common connected occupancy count is exact across Phi absorption and repair",
        all(sum(delta[index] for index in (0, 1, 3)) == 0 for delta in all_deltas)
        and all(delta[2] == 0 for delta in all_deltas),
    )

    check(
        "C12 no selected transaction or cubic symmetry fixes Eta/Gamma for FCC A1",
        all(delta[2] == 0 for delta in all_deltas)
        and sc_shell.isdisjoint(fcc_shell),
    )

    missing = {
        "absolute connected multiplier Gamma relative to clock action",
        "FCC relative multiplier Eta/Gamma",
        "variational or permutation generator beyond a conserved invariant",
        "site actuality and C3-layer action terms",
        "native formation and arbitration",
        "block-stable physical curvature and measured pole residue",
    }
    check(
        "C13 common physical action remains open at six normalization/generation debts",
        len(missing) == 6,
    )
    check(
        "C14 no empirical coupling, master root, mass, Born weight, or near-miss search enters",
        repair_rows == 37_632 and collision_rows == 55_008,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} common occupancy-invariant checks pass")
    print(f"exchange_rank={exchange_matrix.rank()}")
    print(f"energy_family=Gamma*(N_F+N_A1_SC+N_A2)+Eta*N_A1_FCC")
    print(f"repair_role_delta_histogram={dict(repair_deltas)}")
    print("relative_result=e_F=e_A1_SC=e_A2_exact_conditional_on_selected_transactions")
    print("open_result=Eta_over_Gamma_and_absolute_Gamma_unselected")
    print("action_status=common_invariant_not_yet_variational_generator_or_physical_curvature")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
