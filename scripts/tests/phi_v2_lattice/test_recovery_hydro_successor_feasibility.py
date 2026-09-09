"""Reproducible finite obstruction tests; no trajectories or numerical fits."""
from copy import deepcopy
from itertools import product
import json

import pytest

from phi_v2_lattice import recovery_hydro_successor_feasibility as F


@pytest.fixture(scope="module")
def report():
    return F.certificate()


def test_labels_are_24_distinct_four_vectors_with_18_projections():
    assert len(F.LABELS) == len(set(F.LABELS)) == 24
    assert len(set(F.VELOCITIES)) == 18
    for v in set(F.VELOCITIES):
        expected = 2 if sum(abs(x) for x in v) == 1 else 1
        assert F.VELOCITIES.count(v) == expected


def test_action_is_a_faithful_48_element_group_with_fixed_fourth_sign():
    assert len(set(F.CHANNEL_ACTION)) == 48
    for g, h in product(range(48), repeat=2):
        gh = F.COMPOSE[g][h]
        assert tuple(F.CHANNEL_ACTION[g][F.CHANNEL_ACTION[h][i]] for i in range(24)) == F.CHANNEL_ACTION[gh]
    for g, v in product(range(48), F.LABELS):
        result = F.LABELS[F.CHANNEL_ACTION[g][F.LABEL_INDEX[v]]]
        assert result[:3] == F.transform_vector(g, v[:3])
        assert result[3] == v[3]


def test_existing_isotropy_row_agrees_and_full_tensor_is_checked(report):
    from phi_v2_lattice import recovery_hydro_invariants as H

    old = H.fourth_rank_isotropy(H.VELOCITY_SETS["fchc_projected_18"])
    new = report["isotropy"]
    assert (old.T_xxxx, old.T_xxyy, old.T_xx, old.T_xy) == (12, 4, 12, 0)
    assert new["second_moment"] == [[12, 0, 0], [0, 12, 0], [0, 0, 12]]
    assert new["fourth_tensor_violations"] == 0


def test_57_count_by_independent_generating_polynomial_coefficient():
    # Multiply product_v (1 + z x^vx y^vy w^vz), keeping degree <=5.
    # This independent recurrence does not enumerate occupancy combinations.
    coefficients = {(0, 0, 0, 0): 1}
    for v in F.VELOCITIES:
        additions = {}
        for (m, x, y, z), multiplicity in coefficients.items():
            if m < 5:
                key = (m + 1, x + v[0], y + v[1], z + v[2])
                additions[key] = additions.get(key, 0) + multiplicity
        for key, multiplicity in additions.items():
            coefficients[key] = coefficients.get(key, 0) + multiplicity
    assert coefficients[5, -2, -2, -2] == 57


def test_three_cycle_precludes_a_unique_fixed_mass_five_state(report):
    row = report["counterexample"]
    g = row["three_cycle_group_index"]
    assert F.COMPOSE[F.COMPOSE[g][g]][g] == F.IDENTITY
    assert row["three_cycle_fixed_projected_velocities"] == 0
    assert len(row["three_cycle_channel_orbits"]) == 8
    assert all(len(orbit) == 3 for orbit in row["three_cycle_channel_orbits"])
    assert row["class_size"] % 2 == 1
    assert row["mass"] % 3 != 0
    assert not any(F.transform_state(g, tuple(a)) == tuple(a) for a in row["states"])


def test_counterexample_lists_complete_class_and_attains_minimum_three(report):
    row = report["counterexample"]
    exact = F.occupancy_classes(5)[(-2, -2, -2)]
    assert [list(a) for a in exact] == row["states"]
    assert row["orbit_size_histogram"] == {"3": 7, "6": 6}
    assert row["required_fixed_points"] == 1
    assert row["minimum_equivariant_fixed_points"] == 3
    assert row["globally_little_group_fixed_states"] == 0
    mapping = {}
    for a, b in row["relaxed_collision_pairs"]:
        mapping[tuple(a)] = tuple(b)
        mapping[tuple(b)] = tuple(a)
    for a in row["minimum_fixed_witness"]:
        mapping[tuple(a)] = tuple(a)
    assert set(mapping) == set(exact)
    for a in exact:
        assert mapping[mapping[a]] == a
        assert len(mapping[a]) == len(a)
        assert F.momentum(mapping[a]) == F.momentum(a)
        for g in F.little_group((-2, -2, -2)):
            assert mapping[F.transform_state(g, a)] == F.transform_state(g, mapping[a])


def test_small_sector_coverage_and_scoped_negative(report):
    rows = report["small_sector_census"]
    assert [row["occupancy_states"] for row in rows] == [1, 24, 276, 2024, 10626, 42504]
    assert [row["momentum_orbits"] for row in rows] == [1, 2, 7, 13, 19, 27]
    assert [row["obstructed_momentum_orbits"] for row in rows] == [0, 0, 0, 0, 0, 1]
    for row in rows:
        assert all(not any(c["constructive_relaxation_violations"].values()) for c in row["classes"])
    assert not report["full_2_to_24_table_generated"]


def test_class_construction_transports_equivariantly_across_momenta():
    _, reference = F.minimum_fixed_involution(5, (-2, -2, -2))
    transported = {}
    for g in range(48):
        for a, b in reference.items():
            ga, gb = F.transform_state(g, a), F.transform_state(g, b)
            assert transported.setdefault(ga, gb) == gb
    assert len(transported) == 8 * 57
    for a, b in transported.items():
        assert F.momentum(a) == F.momentum(b)
        assert transported[b] == a
        for g in range(48):
            assert transported[F.transform_state(g, a)] == F.transform_state(g, b)


@pytest.mark.parametrize("mass", [-1, 6, 24, True, 1.0])
def test_large_or_invalid_occupancy_scan_is_rejected(mass):
    with pytest.raises(ValueError, match="0..5"):
        F.occupancy_classes(mass)


def test_erasing_independent_phase_bits_loses_mass_and_momentum(report):
    row = report["phase_contract"]["erasure_counterexample"]
    state = row["full_state"]
    assert len({tuple(c) for c in state}) == row["true_mass"] == 2
    assert len({c[0] for c in state}) == row["erased_mass"] == 1
    assert [sum(F.VELOCITIES[i][a] for i, _ in state) for a in range(3)] == row["true_momentum"]
    assert row["true_momentum"] != row["erased_momentum"]


def test_passive_sorted_assignment_breaks_symmetry_even_for_valid_occupancy_swap(report):
    row = report["phase_contract"]["sorted_assignment_counterexample"]
    assert row["occupancy_mass_momentum_preserved"]
    assert row["occupancy_pair_symmetry_compatible"]
    assert row["input"] == row["transformed_input"]
    assert row["output"] != row["transformed_output"]
    # No alternative phase-preserving bijection repairs this same collision:
    # its input is fixed individually, but its two outputs are exchanged.
    g = row["group_index"]
    for phases in ((0, 1), (1, 0)):
        output = tuple((row["output"][i][0], phases[i]) for i in range(2))
        rotated = tuple(sorted((F.CHANNEL_ACTION[g][i], k) for i, k in output))
        assert rotated != output


def test_passive_phase_cycle_forces_stroboscopic_counts(report):
    row = report["phase_contract"]
    cycle = row["phase_count_cycle"]
    assert cycle == [[0, 0, 0, 1], [1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0]]
    assert row["phase_count_cycle_fourth_power"] == [[int(a == b) for b in range(4)] for a in range(4)]
    # Independence of four color counts and three momenta: coefficients of
    # opposite-velocity singleton records force each momentum coefficient to
    # zero; singleton records of each phase then force each count coefficient.
    for k in range(4):
        for axis in range(3):
            positive = tuple(int(i == axis) for i in range(3))
            assert positive in F.VELOCITIES
            assert tuple(-x for x in positive) in F.VELOCITIES
    assert row["four_hop_additive_invariant_lower_bound"] == 7


def test_certificate_is_integral_stable_and_tamper_evident(report):
    def check(value):
        assert not isinstance(value, float)
        if isinstance(value, dict):
            for item in value.values():
                check(item)
        elif isinstance(value, (list, tuple)):
            for item in value:
                check(item)

    check(report)
    assert json.loads(F.canonical_bytes(report)) == report
    assert F.validate_certificate(report)
    altered = deepcopy(report)
    altered["counterexample"]["class_size"] = 56
    with pytest.raises(ValueError, match="exact recomputation"):
        F.validate_certificate(altered)
