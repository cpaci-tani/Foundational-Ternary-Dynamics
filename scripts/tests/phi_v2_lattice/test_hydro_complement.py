"""Finite-law, accounting and replay gates for the selected field sector."""
from dataclasses import replace
from fractions import Fraction
import hashlib
import json

import numpy as np
import pytest

from phi_v2_lattice import hydro_complement as H


def bank(L=3):
    return np.zeros((L, L, L, 2, 24), dtype=bool)


def put(array, site, polarity, mask):
    array[(*site, polarity)] = [(mask >> i) & 1 for i in range(24)]


def test_exact_finite_certificate_and_independent_class_census():
    report = H.certificate()
    assert report["eligible_subsets"] == 27776
    assert report["complement_closure"] and report["all_eligible_mass_momentum"]
    assert report["signed_cubic_actions"] == 48 and report["symmetry_failures"] == 0
    assert report["four_invariant_gate"] and report["fixed_invariant_dimension"] == 4
    assert report["fourth_rank_isotropy_failures"] == 0
    assert report["eligibility_at_half"] == "217/131072"
    # Independent generating-function census of all2^24 occupancies, aggregated
    # by their exact mass/momentum; no call to the meet-in-the-middle generator.
    counts = {(0, 0, 0, 0): 1}
    for velocity in H.VELOCITIES:
        advanced = dict(counts)
        for (n, x, y, z), count in counts.items():
            if n < 12:
                key = (n + 1, x + velocity[0], y + velocity[1], z + velocity[2])
                advanced[key] = advanced.get(key, 0) + count
        counts = advanced
    assert counts[(12, 0, 0, 0)] == len(H.eligible_masks())


def test_all_eligible_involutions_and_exact_jacobian_score_identity():
    masks = H.eligible_masks()
    for mask in masks:
        out = H.collide_mask(mask)
        assert out != mask and H.collide_mask(out) == mask
        assert out.bit_count() == mask.bit_count() and H.momentum(out) == H.momentum(mask)
    # Derive two Jacobian columns directly from the finite counting score,
    # using a non-half occupation to expose missing p/(1-p) factors.
    p = Fraction(1, 3)
    jacobian = H.marginal_jacobian(p)
    for j in (0, 11):
        signed_score = [0] * 24
        signed_delta = [0] * 24
        for mask in masks:
            for i in range(24):
                d = 1 - 2 * ((mask >> i) & 1)
                signed_score[i] += d * ((mask >> j) & 1)
                signed_delta[i] += d
        for i in range(24):
            direct = Fraction(i == j) + p ** 11 * (1 - p) ** 11 * (
                signed_score[i] - p * signed_delta[i])
            assert direct == jacobian[i][j]


def test_all_eligible_runtime_collisions_match_scalar_rule_and_preserve_both_banks():
    # Pack every eligible local mask into batches; no trajectory sampling here.
    masks = H.eligible_masks()
    capacity = 3 ** 3 * 2
    for start in range(0, len(masks), capacity):
        initial = bank()
        rows = initial.reshape(-1, 24)
        batch = masks[start:start + capacity]
        for row, mask in zip(rows, batch):
            row[:] = [(mask >> i) & 1 for i in range(24)]
        state = H.initialize(initial)
        after = H.step(state)
        assert H.inventories(after) == H.inventories(state)
        for row, mask in zip(after.bank.reshape(-1, 24), batch):
            actual = sum(int(bit) << i for i, bit in enumerate(row))
            assert actual == H.collide_mask(mask)


@pytest.mark.parametrize("phase", [0, 1])
def test_owned_checkpoint_continuation_and_accounting(phase):
    initial = bank(5)
    put(initial, (0, 4, 0), 0, H.eligible_masks()[0])
    put(initial, (3, 0, 4), 1, H.eligible_masks()[-1])
    initial[2, 3, 1, 0, 4] = True
    state = H.initialize(initial)
    initial[:] = False
    if phase:
        state = H.step(state)
    state = replace(state, microtick=2 ** 65 + phase)
    original = H.checkpoint(state)
    restored = H.restore(original)
    assert not np.shares_memory(state.bank, restored.bank)
    inventory = H.inventories(state)
    for _ in range(32):
        a, b = H.step(state), H.step(restored)
        assert H.checkpoint(a) == H.checkpoint(b)
        assert H.inventories(a) == inventory
        assert not np.shares_memory(a.bank, state.bank)
        state, restored = a, b
    assert original == H.checkpoint(H.restore(original))


def test_single_token_every_channel_streams_exactly_one_declared_hop_across_seams():
    for L in (3, 4):
        for polarity in (0, 1):
            for c, velocity in enumerate(H.VELOCITIES):
                initial = bank(L)
                site = tuple(L - 1 if v > 0 else 0 for v in velocity)
                initial[(*site, polarity, c)] = True
                state = H.initialize(initial)
                collision = H.step(state)
                np.testing.assert_array_equal(collision.bank, initial)
                moved = H.step(collision)
                target = tuple((x + dx) % L for x, dx in zip(site, velocity))
                assert moved.bank[(*target, polarity, c)] and moved.bank.sum() == 1


@pytest.mark.parametrize("mask", [0, 1, 3, 15, (1 << 24) - 1])
def test_ineligible_occupancies_are_identity(mask):
    assert H.collide_mask(mask) == mask


@pytest.mark.parametrize("value", [True, -1, 1.0, 1 << 24, "1"])
def test_invalid_masks_rejected(value):
    with pytest.raises(ValueError):
        H.collide_mask(value)


def test_invalid_states_and_corrupt_checkpoints_reject_without_mutation():
    state = H.initialize(bank())
    valid = H.checkpoint(state)
    broken_bool = bank()
    broken_bool.view(np.uint8).flat[0] = 2
    for candidate in (replace(state, microtick=True), replace(state, microtick=-1),
                      replace(state, L=True), replace(state, law_id="foreign"),
                      replace(state, bank=broken_bool), replace(state, bank=bank()[::-1]),
                      replace(state, bank=bank().astype(np.int8))):
        with pytest.raises(ValueError):
            H.step(candidate)
        assert H.checkpoint(state) == valid
    for payload in (b"", valid[:-1], valid + b"x", valid[:20] + b"x" + valid[21:],
                    bytearray(valid)):
        with pytest.raises(ValueError):
            H.restore(payload)
    for header in ({"L": True, "tick_hex": "0", "law": H.LAW_ID},
                   {"L": 3, "tick_hex": "00", "law": H.LAW_ID},
                   {"L": 3, "tick_hex": "A", "law": H.LAW_ID},
                   {"L": 3, "tick_hex": "0x1", "law": H.LAW_ID},
                   {"L": 3, "tick_hex": "0", "law": "foreign"},
                   {"L": 3, "tick_hex": "0", "law": H.LAW_ID, "extra": 0}):
        encoded = json.dumps(header, sort_keys=True, separators=(",", ":")).encode()
        body = len(encoded).to_bytes(4, "little") + encoded + bytes(6 * 3 ** 3)
        with pytest.raises(ValueError):
            H.restore(H.MAGIC + hashlib.sha256(body).digest() + body)


def test_checkpoint_supports_every_finite_ordinal_beyond_decimal_conversion_limit():
    state = replace(H.initialize(bank()), microtick=10 ** 5000 + 1)
    restored = H.restore(H.checkpoint(state))
    assert restored.microtick == state.microtick
    assert H.checkpoint(H.step(restored)) == H.checkpoint(H.step(state))
