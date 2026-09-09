"""Fixed eight-tick counting-ensemble diagnostic; no fitted numeric campaign."""
from dataclasses import FrozenInstanceError
from fractions import Fraction as F
from itertools import combinations, product
import json

import numpy as np
import pytest

from phi_v2_lattice import hydro_parity as H
from phi_v2_lattice import recovery_momentum_memory as M


def test_frozen_inputs_and_registration():
    rows = M.frozen_inputs()
    assert len(rows) == 3
    assert rows[-1]["sha256"] == "0e73ffcece95a3cefe1c9c89b0821d80d7cc0ad0ebfb1cf3680d8811b8e055c3"
    assert M.EPSILON == F(1, 8)
    assert M.EXPECTED_SIGNED_ERROR == -F(
        139921773018153334181957509616241,
        44601490397061246283071436545296723011960832 * 576)


def test_wrong_source_pin_rejected_without_editing_frozen_files(monkeypatch):
    monkeypatch.setattr(M, "FROZEN_INPUTS", ((M.FROZEN_INPUTS[0][0], "0" * 64),))
    with pytest.raises(ValueError, match="frozen diagnostic input changed"):
        M.certificate()


def test_all_four_endpoint_geometries_and_two_independent_bases():
    terms = M.endpoint_terms()
    assert {(t.source, t.target) for t in terms} == set(product((18, 23), repeat=2))
    for t in terms:
        assert t.actual_walsh == t.actual_configuration
        assert t.actual_walsh != 0
        assert t.momentum_weight == H.VELOCITIES[t.source][1] * H.VELOCITIES[t.target][1]
        if t.source == t.target:
            assert t.displacement == (2, -2 * H.VELOCITIES[t.source][1], 0)
            assert (t.endpoint_bits, t.shared_sites, t.configuration_pairs) == ((1, 1), 1, 4)
            assert t.returned == 0
        else:
            assert t.displacement == (2, 0, 0)
            assert (t.endpoint_bits, t.shared_sites, t.configuration_pairs) == ((6, 6), 5, 4096)
            assert t.returned == M.EXPECTED_DELTA
    assert terms[0].actual_walsh == terms[-1].actual_walsh
    assert terms[1].actual_walsh == terms[2].actual_walsh
    with pytest.raises(FrozenInstanceError):
        terms[0].source = 0


def test_registered_expectation_not_used_to_construct_result(monkeypatch):
    # A changed target cannot change either exact contraction or make it pass.
    before = M.endpoint_terms()
    monkeypatch.setattr(M, "EXPECTED_DELTA", F(0))
    M.endpoint_terms.cache_clear()
    try:
        assert M.endpoint_terms() == before
        with pytest.raises(AssertionError, match="registered evidence"):
            M.certificate()
    finally:
        M.endpoint_terms.cache_clear()


@pytest.mark.parametrize("displacement", ((2, -2, 0), (2, 0, 0), (2, 2, 0)))
def test_complete_configuration_tables_normalized_and_match_every_walsh_moment(displacement):
    left, right, blocks = M._separator(displacement)
    for _, incoming, outgoing in blocks:
        rows = M._configuration_joint(incoming, outgoing)
        assert sum(p for _, p in rows) == 1
        assert all(p >= 0 for _, p in rows)
        # The actual joint probability determines every retained character.
        for a, b in product(M._subsets(incoming), M._subsets(outgoing)):
            value = sum(p * M._character(x, a) * M._character(y, b)
                        for (x, y), p in rows)
            assert value == M._operator().moment_masks(b, a)
    for channel, retained in product((18, 23), (left, right)):
        rows = M._configuration_endpoint(channel, retained)
        assert sum(value for _, value in rows) == 0
        for mask in M._subsets(retained):
            value = sum(value * M._character(x, mask) for x, value in rows) / len(rows)
            assert value == M._operator().moment_masks(1 << channel, mask)


def test_positive_twelve_product_mixture_exhausts_local_preparation_patterns():
    weights = tuple(v[1] for v in H.VELOCITIES if v[1])
    assert len(weights) == 12
    densities, scores = [], []
    for spins in product((-1, 1), repeat=12):
        score = F(sum(w * s for w, s in zip(weights, spins)), 12)
        products = tuple(1 + M.EPSILON * w * s for w, s in zip(weights, spins))
        density = sum(products) / 12
        assert min(products) >= F(7, 8)
        assert density == 1 + M.EPSILON * score
        densities.append(density)
        scores.append(score)
    assert sum(densities) == 4096
    assert (min(densities), max(densities)) == (F(7, 8), F(9, 8))
    assert sum(scores) == 0
    assert sum(x * x for x in scores) / 4096 == F(1, 12)
    assert sum(x * x for x in densities) / 4096 == 1 + M.EPSILON**2 / 12


def test_initial_local_conservation_and_exact_comparison_bounds_at_every_stage():
    assert M.projected_score(0) == M.projected_score(1)
    previous_norm = F(1, 12)
    for tick in range(9):
        score = M.projected_score(tick)
        assert len(dict(score)) == len(score)
        assert sum(value for _, value in score) == 0
        assert sum(H.VELOCITIES[c][1] * value for (_, c), value in score) == 1
        norm = sum(value * value for _, value in score)
        assert norm <= previous_norm
        if tick and tick % 2 == 0:
            assert norm == previous_norm
        previous_norm = norm
        minimum = 1 - M.EPSILON * sum(abs(value) for _, value in score)
        # Every slot is independent in the reference, so this assignment
        # attains the claimed minimum; it is not only a triangle upper bound.
        attained = 1 + M.EPSILON * sum(value * (-1 if value > 0 else 1)
                                      for _, value in score)
        assert attained == minimum > 0


@pytest.mark.parametrize("L", (9, 10))
@pytest.mark.parametrize("bank", (0, 1))
@pytest.mark.parametrize("seam", (False, True))
def test_registered_case_lifts_density_and_outer_streams(L, bank, seam):
    origin = (L - 1,) * 3 if seam else (0, 0, 0)
    row = M.momentum_case(L, bank, origin)
    lifted = dict(M.lifted_comparison(L, bank, origin))
    original = dict(M.projected_score())
    assert len(lifted) == len(original)
    for (site, channel), value in original.items():
        key = (tuple((origin[j] + site[j]) % L for j in range(3)), bank, channel)
        assert lifted[key] == value
    target = ((origin[0] + 4) % L, origin[1], origin[2])
    comparator = M.EPSILON * sum(F(H.VELOCITIES[c][1], 12) * value
                               for (site, p, c), value in lifted.items() if site == target)
    terms = M.endpoint_terms()
    actual = M.EPSILON * sum(t.momentum_weight * t.actual_configuration for t in terms) / 144
    assert row["target"] == target
    assert row["complete_boolean_slots"] == 48 * L**3
    assert row["represented_microticks"] == (0, 8)
    assert row["comparison_observable_expectation"] == comparator
    assert row["actual_observable_expectation"] == actual
    assert actual - comparator == M.EXPECTED_SIGNED_ERROR < 0
    assert row["signed_weak_error"] == actual - comparator
    assert row["true_density_integral"] == row["comparison_density_integral"] == 1
    assert row["comparison_density_minimum"] > 0
    assert row["full_density_L2_error_squared_lower_bound"] == 12 * (actual - comparator)**2
    assert row["total_variation_lower_bound"] == abs(actual - comparator) / 2
    with pytest.raises(TypeError):
        M.lifted_comparison(L, bank, origin)[0] = None


@pytest.mark.parametrize("L", (9, 10))
def test_scalar_runtime_fixtures_verify_all_eight_physical_stages_and_bank_independence(L):
    # Finite fixtures, not an enumeration of the complete ensemble. Include
    # eligible, odd-reflection, empty, and full states in both polarity banks.
    rng = np.random.default_rng(1729)
    source = rng.integers(0, 2, (L, L, L, 2, 24), dtype=np.uint8).astype(bool)
    for bank in (0, 1):
        for x, mask in enumerate((H.eligible_masks()[0], 1, 0, H.FULL_MASK)):
            source[x, L - 1, L - 1, bank] = [(mask >> c) & 1 for c in range(24)]
    original = source.copy()
    state = H.initialize(source)
    expected = source.copy()
    weights = np.array([v[1] for v in H.VELOCITIES], dtype=np.int64)
    for tick in range(8):
        before = expected.copy()
        if tick % 2 == 0:
            for site in product(range(L), repeat=3):
                for bank in (0, 1):
                    mask = sum(int(expected[(*site, bank, c)]) << c for c in range(24))
                    output = H.collide_mask(mask)
                    expected[(*site, bank)] = [(output >> c) & 1 for c in range(24)]
            assert np.array_equal(before.astype(np.int64) @ weights,
                                  expected.astype(np.int64) @ weights)
        else:
            # Explicit source-to-destination indexing, independent of np.roll.
            for site in product(range(L), repeat=3):
                for c, velocity in enumerate(H.VELOCITIES):
                    target = tuple((site[j] + velocity[j]) % L for j in range(3))
                    expected[(*target, slice(None), c)] = before[(*site, slice(None), c)]
        prior = state
        saved = prior.bank.copy()
        state = H.step(prior)
        assert state.microtick == tick + 1
        assert np.array_equal(state.bank, expected)
        assert np.array_equal(prior.bank, saved)
        assert not np.shares_memory(state.bank, prior.bank)
    assert np.array_equal(source, original)


@pytest.mark.parametrize("L", (9, 10))
def test_two_collision_support_premise_exhausts_minimal_odd_leakage(L):
    destinations = tuple(tuple(a % L for a in v) for v in H.VELOCITIES)
    assert max(destinations.count(v) for v in set(destinations)) == 2
    for triple in combinations(range(24), 3):
        assert len({destinations[c] for c in triple}) >= 2


@pytest.mark.parametrize("L", (8, 11, True, 9.0, "9"))
def test_invalid_registered_size_rejected(L):
    with pytest.raises(ValueError):
        M.momentum_case(L)


@pytest.mark.parametrize("bank", (-1, 2, True, 0.0, "0"))
def test_invalid_bank_rejected(bank):
    with pytest.raises(ValueError):
        M.lifted_comparison(9, bank)


@pytest.mark.parametrize("origin", (None, (0, 0), (0, 0, 0, 0), (0, True, 0), (0, 1.0, 0)))
def test_invalid_origin_rejected(origin):
    with pytest.raises(ValueError):
        M.momentum_case(9, origin=origin)


@pytest.mark.parametrize("tick", (-1, 9, True, 1.0, "1"))
def test_invalid_tick_rejected_even_after_valid_cache_population(tick):
    M.projected_score(microticks=1)
    with pytest.raises(ValueError):
        M.projected_score(microticks=tick)


def test_certificate_exact_scope_and_no_unearned_recovery_claim():
    result = json.loads(json.dumps(M.certificate()))
    assert result["registered_identity_verified"]
    assert len(result["endpoint_terms"]) == 4
    assert len(result["cases"]) == 8
    assert {(r["L"], r["polarity_bank"], tuple(r["origin"])) for r in result["cases"]} == {
        (L, bank, origin) for L in (9, 10) for bank in (0, 1)
        for origin in ((0, 0, 0), (L - 1, L - 1, L - 1))}
    assert result["memory_facts"]["first_degree_exact_through_cycles"] == 3
    assert result["memory_facts"]["conserved_local_means_exact_through_microtick"] == 7
    assert not result["memory_facts"]["full_memory_kernel_computed"]
    for claim in ("continuum_recovered", "long_time_error_upper_bound", "canonical_adoption",
                  "unified_binding_law", "individual_trajectory_fluid_limit"):
        assert result["limits"][claim] is False
