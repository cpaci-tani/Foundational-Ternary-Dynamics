"""Exact finite-ensemble identities and independently contracted return probes."""
from fractions import Fraction as F
from itertools import combinations, product
import json
import random

import numpy as np
import pytest

from phi_v2_lattice import hydro_complement as OLD
from phi_v2_lattice import hydro_parity as NEW
from phi_v2_lattice import recovery_micro_correlation as M


@pytest.fixture(scope="module", params=(OLD, NEW), ids=("complement", "parity"))
def law(request):
    return request.param


@pytest.fixture(scope="module")
def operator(law):
    return M.CharacterOperator(law)


def _reflect(mask, labels):
    index = {w: c for c, w in enumerate(labels)}
    return sum(1 << index[w[:3]+(-w[3],)] for c, w in enumerate(labels) if mask >> c & 1)


def test_character_identity_on_every_eligible_and_registered_other_records(law, operator):
    # Exhausts the complement-eligible scope; other sectors are sampled fixtures.
    rng = random.Random(1729)
    masks = (*operator.eligible, 0, M.FULL,
             *(1 << c for c in range(24)), *(M.FULL ^ (1 << c) for c in range(24)),
             *(rng.getrandbits(24) for _ in range(128)))
    eligible = set(operator.eligible)
    outputs = (0, 1, 1 << 2, (1 << 2) | (1 << 3), 1 | (1 << 2) | (1 << 3))
    for occupied in masks:
        actual = law.collide_mask(occupied)
        for output in outputs:
            original = M.character(output, occupied)
            if operator.parity:
                reflected = M.character(_reflect(output, operator.labels), occupied)
                parity = M.character(M.FULL, occupied)
                predicted = (original+reflected+parity*(original-reflected)) // 2
            else:
                predicted = original
            if occupied in eligible and output.bit_count() % 2:
                predicted -= 2*original
            assert predicted == M.character(output, actual)


def test_independent_character_jacobian_and_even_blocks(law, operator):
    assert operator.jacobian == law.marginal_jacobian(F(1, 2))
    for a, i in product(range(24), repeat=2):
        assert operator.moment((a,), (i,)) == operator.moment((i,), (a,))
    for output in ((0, 1), (2, 3), (2, 4)):
        for input_ in ((), (0,), (2,), (0, 1), (2, 3), (2, 4)):
            if len(input_) != 2:
                assert operator.moment(output, input_) == 0
            else:
                out_mask = sum(1 << c for c in output)
                in_mask = sum(1 << c for c in input_)
                expected = F(in_mask == out_mask)
                if operator.parity:
                    expected = F(int(in_mask == out_mask)
                                 + int(in_mask == _reflect(out_mask, operator.labels)), 2)
                assert operator.moment(output, input_) == expected


def test_first_leakage_pair_blindness_and_exact_finite_amplitude(operator):
    i = operator.labels.index(M.SOURCE_LABEL)
    j = operator.labels.index((1, 0, 0, -1))
    k = operator.labels.index((1, 0, 0, 1))
    third = operator.moment((i,), (i, j, k))
    assert third != 0
    assert operator.moment((i,), (j, k)) == 0
    assert third == -F(operator.eligible_character_sum((1 << j) | (1 << k)), 1 << 23)
    # Regenerating a product with exact means predicts an O(epsilon^3) triple.
    eps = F(1, 8)
    product_triple = eps**3*operator.jacobian[i][i]*operator.jacobian[j][i]*operator.jacobian[k][i]
    assert eps*third != product_triple


@pytest.mark.parametrize("L", (3, 4, 5, 6))
def test_every_three_label_character_streams_to_multiple_sites(operator, L):
    destinations = tuple(tuple(x % L for x in v) for v in operator.velocities)
    assert max(destinations.count(site) for site in set(destinations)) == 2
    # Every larger subset contains a triple, so this exhausts the minimal
    # geometric premise of the two-collision orthogonality argument.
    for triple in combinations(range(24), 3):
        assert len({destinations[c] for c in triple}) >= 2


@pytest.mark.parametrize("L", (5, 6))
def test_two_cycle_full_norm_and_independent_dense_spatial_order(operator, L):
    i = operator.labels.index(M.SOURCE_LABEL)
    origin = (L-1, 0, L-1)
    rows = M.two_cycle_density(operator, L, i, origin=origin)
    jacobian = np.array(operator.jacobian, dtype=object)
    dense = np.full((L, L, L, 24), F(0), dtype=object)
    dense[(*origin, i)] = F(1)
    for tick in range(5):
        sparse = dict(M.projected_coefficients(operator, L, tick, i, origin))
        assert sparse == {(site, c): dense[(*site, c)] for site in product(range(L), repeat=3)
                          for c in range(24) if dense[(*site, c)]}
        norm = sum(value*value for value in dense.flat)
        assert rows[tick]["projected_norm_squared"] == norm
        assert rows[tick]["full_density_error_squared"] == (1-norm)/64
        assert rows[tick]["linear_comparator_nonnegative"]
        if tick % 2 == 0:
            dense = dense @ jacobian.T
        else:
            streamed = np.empty_like(dense)
            for c, v in enumerate(operator.velocities):
                streamed[..., c] = np.roll(dense[..., c], v, axis=(0, 1, 2))
            dense = streamed
    assert rows[1]["full_density_error_squared"] > 0
    assert rows[1]["full_density_error_squared"] == rows[2]["full_density_error_squared"]
    assert rows[3]["full_density_error_squared"] == rows[4]["full_density_error_squared"]


def _assignment_probability(first_mask, first_value, second_mask, second_value):
    if (first_value ^ second_value) & first_mask & second_mask:
        return F(0)
    return F(1, 1 << (first_mask | second_mask).bit_count())


def _joint_table(law, operator, incoming, outgoing, eligible_outputs):
    """Configuration counts from actual eligible outputs and exact parity count.

    This construction does not import or invert the character-moment oracle.
    For <=4 fixed input bits the remaining parity is exactly half/half.
    """
    in_bits = tuple(c for c in range(24) if incoming >> c & 1)
    out_bits = tuple(c for c in range(24) if outgoing >> c & 1)
    result = {}
    for a in M._subsets(in_bits):
        for b in M._subsets(out_bits):
            value = _assignment_probability(incoming, a, outgoing, b)
            if operator.parity:
                reflected_mask = _reflect(outgoing, operator.labels)
                reflected_value = _reflect(b, operator.labels)
                value += (_assignment_probability(incoming, a, reflected_mask, reflected_value)
                          - _assignment_probability(incoming, a, outgoing, b))/2
            correction = sum(int((actual & outgoing) == b)-int((occupied & outgoing) == b)
                             for occupied, actual in eligible_outputs if occupied & incoming == a)
            result[a, b] = value+F(correction, 1 << 24)
    assert sum(result.values()) == 1
    assert all(value >= 0 for value in result.values())
    return result


@pytest.mark.parametrize("L", (5, 6))
def test_third_collision_full_ensemble_contraction_in_configuration_basis(law, operator, L):
    i = operator.labels.index(M.SOURCE_LABEL)
    a = operator.labels.index(M.TARGET_LABEL)
    contraction = M.endpoint_contraction(operator, L, M.RETURN_DISPLACEMENT, i, a)
    assert len(contraction.blocks) == 5
    assert len(contraction.left_channels) == len(contraction.right_channels) == 6
    eligible_outputs = tuple((mask, law.collide_mask(mask)) for mask in operator.eligible)
    tables = tuple(_joint_table(law, operator, x, y, eligible_outputs)
                   for _, x, y in contraction.blocks)
    left_inputs = M._subsets(contraction.left_channels)
    right_inputs = M._subsets(contraction.right_channels)
    # Conditional endpoint functions are independently counted over E and the
    # exactly half odd-parity branch, rather than evaluated from Walsh scores.
    def conditional_score(channel, channels, occupied):
        mask = sum(1 << c for c in channels)
        value = F(M.character(1 << channel, occupied) if channel in channels else 0)
        if operator.parity:
            partner = operator.reflection[channel]
            other = M.character(1 << partner, occupied) if partner in channels else 0
            value = (value+other)/2
        count = sum(M.character(1 << channel, before) for before, _ in eligible_outputs
                    if before & mask == occupied)
        return value-F(2*count, 1 << (24-len(channels)))
    left_scores = {x: conditional_score(i, contraction.left_channels, x) for x in left_inputs}
    right_scores = {x: conditional_score(a, contraction.right_channels, x) for x in right_inputs}
    value = F(0)
    for x, y in product(left_inputs, right_inputs):
        probability = F(1)
        for (_, incoming, outgoing), table in zip(contraction.blocks, tables):
            probability *= table[x & incoming, y & outgoing]
        value += probability*left_scores[x]*right_scores[y]
    assert value == M.contract_walsh(operator, contraction)
    report = M.third_collision_response(operator, L, M.RETURN_DISPLACEMENT, i, a)
    assert report["microscopic_response"] == value
    assert report["returned_correlation"] == value-report["projected_response"]
    # Exact zero or nonzero is retained in the certificate; no adjusted case.


def _mask_at(bank, site, polarity):
    return sum(int(bit) << c for c, bit in enumerate(bank[(*site, polarity)]))


def _backward_five_tick_spin(law, bank, target, channel, polarity):
    """Actual full Boolean causal cone, no moment or factorization formula."""
    L = bank.shape[0]
    first = {}
    def first_collision(site):
        if site not in first:
            first[site] = law.collide_mask(_mask_at(bank, site, polarity))
        return first[site]
    final_input = 0
    for b, v in enumerate(law.VELOCITIES):
        y = tuple((target[j]-v[j]) % L for j in range(3))
        middle_input = 0
        for c, w in enumerate(law.VELOCITIES):
            z = tuple((y[j]-w[j]) % L for j in range(3))
            middle_input |= ((first_collision(z) >> c) & 1) << c
        final_input |= ((law.collide_mask(middle_input) >> b) & 1) << b
    return 2*((law.collide_mask(final_input) >> channel) & 1)-1


@pytest.mark.parametrize("L,origin", ((5, (0, 0, 0)), (5, (4, 4, 4)), (6, (5, 0, 5))))
def test_actual_runtime_causal_cone_order_polarities_and_seams(law, operator, L, origin):
    bank = np.random.default_rng(1729).integers(0, 2, size=(L, L, L, 2, 24), dtype=np.uint8).astype(bool)
    snapshot = bank.copy()
    state = law.initialize(bank)
    target = tuple((origin[j]+M.RETURN_DISPLACEMENT[j]) % L for j in range(3))
    channel = operator.labels.index(M.TARGET_LABEL)
    expected = tuple(_backward_five_tick_spin(law, bank, target, channel, p) for p in range(2))
    for _ in range(5):
        state = law.step(state)
    assert state.microtick == 5
    for p in range(2):
        assert 2*int(state.bank[(*target, p, channel)])-1 == expected[p]
    streamed = law.step(state)
    moved = tuple((target[j]+law.VELOCITIES[channel][j]) % L for j in range(3))
    for p in range(2):
        assert 2*int(streamed.bank[(*moved, p, channel)])-1 == expected[p]
    assert np.array_equal(bank, snapshot)


def test_certificate_json_scope_and_zero_amplitude(law, operator):
    report = M.certificate(law)
    json.dumps(report, allow_nan=False)
    assert report["law_id"] == law.LAW_ID
    assert not report["limits"]["hydrodynamic_recovery"]
    assert not report["limits"]["all_response_entries_at_third_collision"]
    assert not report["limits"]["long_horizon_memory_tail_certified"]
    assert report["third_collision_return"][0]["returned_correlation"] == report["third_collision_return"][1]["returned_correlation"]
    assert [row["L"] for row in report["third_collision_return"]] == [5, 6]
    assert all(row["linear_comparator_nonnegative"] for row in report["third_collision_return"])
    # Regression added after the first exact execution; the preregistered
    # case allowed either outcome, and its measured rational is retained.
    expected = (F(139921773018153334181957509616241, 44601490397061246283071436545296723011960832)
                if operator.parity else
                F(3953223406038963574572627425, 42535295865117307932921825928971026432))
    assert F(report["third_collision_return"][0]["returned_correlation"]) == expected
    assert all(row["full_density_error_squared"] == 0 for row in M.two_cycle_density(operator, 5, 0, 0))


@pytest.mark.parametrize("bad", (True, -1, 24, 1.0, "0"))
def test_invalid_channel_rejected_without_mutation(operator, bad):
    before = operator.eligible
    with pytest.raises(ValueError):
        M.projected_coefficients(operator, 5, 1, bad)
    assert operator.eligible is before


@pytest.mark.parametrize("L,ticks", ((True, 1), (2, 1), (5, True), (5, -1), (5, 7)))
def test_invalid_lattice_or_horizon(operator, L, ticks):
    with pytest.raises(ValueError):
        M.projected_coefficients(operator, L, ticks, 0)


def test_budget_invalid_characters_and_foreign_law(operator):
    with pytest.raises(ValueError, match="bit budget"):
        M.endpoint_contraction(operator, 5, (0, 0, 0), 0, 0)
    with pytest.raises(ValueError):
        operator.moment((0, 0), (1,))
    for epsilon in (True, 0.1, F(9, 8)):
        with pytest.raises(ValueError):
            M.two_cycle_density(operator, 5, 0, epsilon)
    with pytest.raises(ValueError):
        M.projected_coefficients(operator, 5, 1, 0, (True, 0, 0))
    with pytest.raises(ValueError):
        M.CharacterOperator(type("Foreign", (), {"LAW_ID": "not-a-candidate"}))
