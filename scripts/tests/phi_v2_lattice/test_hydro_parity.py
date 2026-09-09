"""Exact finite law, independent counts, stage accounting, and checkpoint gates."""
from dataclasses import replace
from fractions import Fraction
import hashlib
from itertools import combinations, product
import json
from math import comb

import numpy as np
import pytest

from phi_v2_lattice import hydro_parity as H


def bank(L=3):
    return np.zeros((L, L, L, 2, 24), dtype=bool)


def put(array, site, polarity, mask):
    array[(*site, polarity)] = [(mask >> i) & 1 for i in range(24)]


def test_independent_sixteen_plus_eight_count_and_bit_column_gram():
    # Different split, incremental sums, and arbitrary-precision bit columns;
    # no use of the candidate's half-class builder or NumPy Gram product.
    V = tuple(v for v in product((-1, 0, 1), repeat=4) if sum(x*x for x in v) == 2)
    suffix = {}
    for mask in range(256):
        p = tuple(sum(V[16+i][j] for i in range(8) if mask >> i & 1) for j in range(4))
        suffix.setdefault((mask.bit_count(), *p), []).append(mask)
    sums = [(0, 0, 0, 0)] * 65536
    independently_eligible = []
    for a in range(65536):
        if a:
            bit = a & -a
            v = V[bit.bit_length()-1]
            sums[a] = tuple(x+y for x, y in zip(sums[a ^ bit], v))
        key = (12-a.bit_count(), *(-x for x in sums[a]))
        independently_eligible.extend(a | (b << 16) for b in suffix.get(key, ()))
    independently_eligible.sort()
    assert len(independently_eligible) == 11740
    assert tuple(independently_eligible) == H.eligible_masks()
    columns = [0] * 24
    for row, a in enumerate(independently_eligible):
        for i in range(24):
            if a >> i & 1:
                columns[i] |= 1 << row
    gram = tuple(tuple(len(independently_eligible)-2*(a ^ b).bit_count() for b in columns)
                 for a in columns)
    assert gram == H.collision_gram()


def test_finite_certificate_retains_prices_and_passes_exact_gates():
    row = H.certificate()
    assert row["eligible_subsets"] == 11740
    assert row["eligible_sha256"] == "9740136eaf397218713fa96a126fa6b1ee63ef8e42a297409dbc5b6db0e2176b"
    assert row["complement_closure"] and row["all_eligible_mass_momentum4"]
    assert row["complement_five_weight_gate"] and row["four_fixed_channel_weight_gate"]
    assert (row["complement_gram_rank"], row["combined_constraint_rank"]) == (19, 20)
    assert row["signed_four_axis_actions"] == 384
    assert row["complement_symmetry_failures"] == 0
    assert row["physical_reflection_commutator_failures"] == 0
    assert row["traceless_stress_weights"] == 9 and row["stress_eigenvalue"] == "14880"
    assert row["stress_scalar_failures"] == row["spatial_stress_reflection_failures"] == 0
    assert row["full_law_full_four_dimensional_covariance"] is False
    assert row["eligibility_at_half"] == "2935/4194304"
    assert row["fourth_momentum_multiplier_at_half"] == "0"
    assert row["stress_relaxation_per_cycle_at_half"] == "465/262144"
    assert row["inverse_stress_relaxation_cycles_at_half"] == "262144/465"
    assert not any(row["limits"].values())


def test_all_complement_events_and_complete_small_odd_sectors():
    masks = list(H.eligible_masks())
    for count in (1, 3):
        small = [sum(1 << i for i in indices) for indices in combinations(range(24), count)]
        masks.extend(small)
        masks.extend(H.FULL_MASK ^ a for a in small)
    for a in masks:
        out = H.collide_mask(a)
        assert H.collide_mask(out) == a
        assert out.bit_count() == a.bit_count()
        assert H.momentum(out) == H.momentum(a)
        if a.bit_count() % 2:
            assert out == H.transform_mask(a, H.R4_ACTION)
        else:
            assert out == H.FULL_MASK ^ a


def test_nonzero_fourth_momentum_excludes_complement_and_full_symmetry_is_priced():
    # Start from an eligible occupancy and exchange equal spatial-velocity
    # labels when exactly one is occupied; P3 remains zero but P4 changes.
    a = next(a for a in H.eligible_masks()
             if any(((a >> i) & 1) != ((a >> j) & 1) for i, j in enumerate(H.R4_ACTION)))
    i, j = next((i, j) for i, j in enumerate(H.R4_ACTION) if ((a >> i) & 1) != ((a >> j) & 1))
    rejected = a ^ (1 << i) ^ (1 << j)
    assert rejected.bit_count() == 12 and H.momentum(rejected) == (0, 0, 0)
    assert H.label_momentum(rejected)[3] != 0 and H.collide_mask(rejected) == rejected
    # Swapping spatial coordinate 0 with label coordinate 3 need not commute
    # with the full parity law; this is the specification's explicit price.
    index = {v: i for i, v in enumerate(H.LIFTED_VELOCITIES)}
    swap = tuple(index[(v[3], v[1], v[2], v[0])] for v in H.LIFTED_VELOCITIES)
    assert any(H.collide_mask(H.transform_mask(1 << i, swap))
               != H.transform_mask(H.collide_mask(1 << i), swap) for i in range(24))


@pytest.mark.parametrize("p", [Fraction(1, 2), Fraction(1, 3), Fraction(1, 4)])
def test_parity_background_identity_and_direct_score_jacobian(p):
    c, b = H.reference_coefficients(p)
    direct_even = sum(Fraction(comb(22, k))*p**k*(1-p)**(22-k) for k in range(0, 23, 2))
    assert b == direct_even == (1 + (1-2*p)**22) / 2
    masks = H.eligible_masks()
    jacobian = H.marginal_jacobian(p)
    # A face slot and an edge slot test the parity-changing and inert cases.
    for j in (H.R4_ACTION.index(next(i for i, v in enumerate(H.LIFTED_VELOCITIES) if v[3])), 0):
        scores = [0] * 24
        delta_sums = [0] * 24
        for a in masks:
            for i in range(24):
                delta = 1 - 2*((a >> i) & 1)
                scores[i] += delta*((a >> j) & 1)
                delta_sums[i] += delta
        for i in range(24):
            complement = p**11*(1-p)**11*(scores[i]-p*delta_sums[i])
            odd = direct_even*(int(i == H.R4_ACTION[j])-int(i == j))
            assert jacobian[i][j] == int(i == j) + complement + odd
    fourth = [v[3] for v in H.LIFTED_VELOCITIES]
    assert [sum(x*y for x, y in zip(row, fourth)) for row in jacobian] == [-(1-2*p)**22*x for x in fourth]


def test_vector_runtime_all_complements_and_small_odd_masks():
    masks = list(H.eligible_masks())
    masks.extend(sum(1 << i for i in indices) for indices in combinations(range(24), 3))
    masks.extend((0, H.FULL_MASK))
    capacity = 3**3*2
    for start in range(0, len(masks), capacity):
        initial = bank()
        rows = initial.reshape(-1, 24)
        batch = masks[start:start+capacity]
        for row, a in zip(rows, batch):
            row[:] = [(a >> i) & 1 for i in range(24)]
        state = H.initialize(initial)
        actual = H.step(state)
        assert H.inventories(actual) == H.inventories(state)
        np.testing.assert_array_equal(state.bank, initial)
        for row, a in zip(actual.bank.reshape(-1, 24), batch):
            assert sum(int(bit) << i for i, bit in enumerate(row)) == H.collide_mask(a)


@pytest.mark.parametrize("phase", [0, 1])
def test_scalar_stage_reference_owned_replay_and_causality(phase):
    L = 5
    initial = bank(L)
    fixtures = [H.eligible_masks()[0], 1, 7, 3, H.FULL_MASK]
    for x, y, z, polarity in product(range(L), range(L), range(L), range(2)):
        a = fixtures[(x+2*y+3*z+polarity) % len(fixtures)]
        put(initial, (x, y, z), polarity, a)
    state = replace(H.initialize(initial), microtick=2**65 + phase)
    expected = bank(L)
    for x, y, z, polarity in product(range(L), range(L), range(L), range(2)):
        occupied = {i for i in range(24) if initial[x, y, z, polarity, i]}
        if phase == 0:
            if len(occupied) % 2:
                occupied = {H.R4_ACTION[i] for i in occupied}
            elif len(occupied) == 12 and all(sum(H.LIFTED_VELOCITIES[i][j] for i in occupied) == 0 for j in range(4)):
                occupied = set(range(24)) - occupied
        for i in occupied:
            target = (x, y, z) if phase == 0 else tuple((r+d) % L for r, d in zip((x, y, z), H.VELOCITIES[i]))
            expected[(*target, polarity, i)] = True
    actual = H.step(state)
    np.testing.assert_array_equal(actual.bank, expected)
    assert not np.shares_memory(actual.bank, state.bank)
    assert H.inventories(actual) == H.inventories(state)
    restored = H.restore(H.checkpoint(state))
    assert H.checkpoint(H.step(restored)) == H.checkpoint(actual)
    assert not np.shares_memory(restored.bank, state.bank)
    changed = state.bank.copy()
    changed[2, 2, 2, 0, 0] = ~changed[2, 2, 2, 0, 0]
    perturbed = H.step(replace(state, bank=changed))
    sites = np.argwhere(np.any(actual.bank != perturbed.bank, axis=(-1, -2)))
    assert all(max(abs(x-2) for x in site) <= phase for site in sites)


def test_all_channels_polarities_and_periodic_streaming_seams():
    for L, polarity, i in product((3, 4), range(2), range(24)):
        initial = bank(L)
        v = H.VELOCITIES[i]
        origin = tuple(L-1 if x > 0 else 0 for x in v)
        initial[(*origin, polarity, i)] = True
        state = replace(H.initialize(initial), microtick=1)
        actual = H.step(state)
        target = tuple((x+d) % L for x, d in zip(origin, v))
        assert actual.bank[(*target, polarity, i)] and actual.bank.sum() == 1


@pytest.mark.parametrize("value", [True, -1, 1.0, 1 << 24, "1"])
def test_bad_masks_reject(value):
    with pytest.raises(ValueError):
        H.collide_mask(value)


def test_invalid_state_and_checkpoint_rejection_and_foreign_law_separation():
    from phi_v2_lattice import hydro_complement as old

    state = H.initialize(bank())
    original = H.checkpoint(state)
    invalid_bool = bank()
    invalid_bool.view(np.uint8).flat[0] = 2
    for candidate in (replace(state, microtick=True), replace(state, microtick=-1),
                      replace(state, L=True), replace(state, law_id=old.LAW_ID),
                      replace(state, bank=bank()[::-1]), replace(state, bank=invalid_bool),
                      replace(state, bank=bank().astype(np.int8)), old.initialize(bank())):
        with pytest.raises(ValueError):
            H.step(candidate)
        assert H.checkpoint(state) == original
    for blob in (b"", original[:-1], original + b"x", bytearray(original), old.checkpoint(old.initialize(bank()))):
        with pytest.raises(ValueError):
            H.restore(blob)
    for header in ({"L": True, "law": H.LAW_ID, "tick_hex": "0"},
                   {"L": 3, "law": H.LAW_ID, "tick_hex": "00"},
                   {"L": 3, "law": H.LAW_ID, "tick_hex": "A"},
                   {"L": 3, "law": H.LAW_ID, "tick_hex": "0x1"},
                   {"L": 3, "law": old.LAW_ID, "tick_hex": "0"}):
        encoded = json.dumps(header, sort_keys=True, separators=(",", ":")).encode()
        body = len(encoded).to_bytes(4, "little") + encoded + bytes(6*3**3)
        with pytest.raises(ValueError):
            H.restore(H.MAGIC + hashlib.sha256(body).digest() + body)
    huge = replace(state, microtick=10**5000+1)
    assert H.restore(H.checkpoint(huge)).microtick == huge.microtick
    assert H.checkpoint(H.step(H.restore(H.checkpoint(huge)))) == H.checkpoint(H.step(huge))
