"""V2 finite-law checks; no fluid-recovery claim follows from these tests."""
from fractions import Fraction as Q
from hashlib import sha256
from itertools import permutations, product
from random import Random

import pytest

from scripts.phi_v2_lattice.thermal import runtime as V1, runtime_v2 as R
from scripts.phi_v2_lattice.thermal.equivariant import audit, construction
from scripts.phi_v2_lattice.thermal.analysis import (
    stationary_occupations, exactly_two_probability, equivariant_collision_activity,
    equivariant_collision_census,
    shear_tangent_obstruction,
    shear_tangent_leakage,
)


def fixture(L=5):
    rng = Random(20260910)
    state = R.Records.empty(L)
    for site in range(L**3):
        for c in rng.sample(range(R.CHANNELS), rng.randrange(5)):
            state.bank[site] |= 1 << c
    return state


def transform(state, axes, signs):
    L = state.L
    result = R.Records.empty(L)
    result.microtick = state.microtick
    for site, bits in enumerate(state.bank):
        pos = (site % L, site//L % L, site//(L*L))
        x, y, z = ((signs[i]*pos[axes[i]]) % L for i in range(3))
        destination = x+L*(y+L*z)
        for c in R.occupied(bits):
            v = R.VELOCITIES[c]
            rotated = R.channel(tuple(signs[i]*v[axes[i]] for i in range(3)))
            result.bank[destination] |= 1 << rotated
    return result


def test_exhaustive_collision_involution_accounting_and_cubic_covariance():
    evidence = audit()
    assert evidence["pairs"] == 58653
    assert evidence["moved"] == 44856
    assert evidence["fixed"] == 13797
    assert evidence["symmetry_comparisons"] == 2815344
    assert evidence["table_hash"] == "726908e917ef3723d02b1f9d50285fe64b1ac4e0b614b45ef3e5b1a6971f7d6d"


def test_selected_map_has_only_five_rational_additive_invariants():
    evidence = equivariant_collision_census()
    assert evidence["selected_edges"] == 22428
    assert evidence["rank_mod_prime"] == 338
    assert evidence["complete_additive_invariants"]


@pytest.mark.parametrize("phase", range(4))
def test_sampled_lattice_covariance_for_all_48_symmetries(phase):
    before = fixture()
    before.microtick = phase
    after = R.decode(R.encode(before))
    R.step(after)
    for axes in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            transformed = transform(before, axes, signs)
            R.step(transformed)
            assert R.encode(transformed) == R.encode(transform(after, axes, signs))


@pytest.mark.parametrize("phase", range(4))
def test_sampled_complete_state_locality_and_checkpoint_continuation(phase):
    state = fixture(7)
    state.microtick = (1 << 54)+phase
    other = R.decode(R.encode(state))
    other.bank[3+7*(3+7*3)] ^= 1 << R.channel((3, -2, 1))
    R.step(other)
    restored = R.decode(R.encode(state))
    R.step(state)
    for i, (a, b) in enumerate(zip(state.bank, other.bank)):
        if a != b:
            assert max(abs(p-3) for p in (i % 7, i//7 % 7, i//49)) <= 1
    expected_totals = R.totals(state)
    R.advance(state, 8)
    R.advance(restored, 9)
    assert R.encode(state) == R.encode(restored)
    assert R.totals(state) == expected_totals


def test_checkpoint_identity_rejects_foreign_law_and_overflow_is_atomic():
    state = fixture()
    with pytest.raises(ValueError): R.decode(V1.encode(state))
    with pytest.raises(ValueError): V1.decode(R.encode(state))
    with pytest.raises(ValueError): R.step(None)
    state.microtick = R.MAX_TICK
    frozen = R.encode(state)
    with pytest.raises(OverflowError): R.step(state)
    with pytest.raises(ValueError): R.advance(state, 1)
    assert R.encode(state) == frozen


def test_rejects_subclasses_with_unserialized_schedule_or_bank_behavior():
    class DifferentClock(R.Records):
        @property
        def phase(self):
            return 1

    class DifferentBank(list):
        def copy(self):
            return [0]*len(self)

    class DifferentCheckpoint(bytes):
        def __getitem__(self, key):
            return b""

    state = DifferentClock(3, 0, [0]*27)
    for engine in (V1, R):
        with pytest.raises(ValueError): engine.encode(state)
        with pytest.raises(ValueError): engine.step(state)
        damaged = R.Records(3, 0, DifferentBank([1]*27))
        with pytest.raises(ValueError): engine.advance(damaged, 1)
        with pytest.raises(ValueError): engine.decode(DifferentCheckpoint(engine.encode(R.Records.empty(3))))


def test_law_storage_and_cached_metadata_cannot_be_mutated():
    for array in construction():
        with pytest.raises(ValueError): array.setflags(write=True)
    pairs, *_ = construction()
    pairs.shape = (pairs.size,)  # A caller may change only its disposable view.
    assert construction()[0].shape == (58653, 2)
    assert R.collision_identity() == "726908e917ef3723d02b1f9d50285fe64b1ac4e0b614b45ef3e5b1a6971f7d6d"


def test_actual_collision_preserves_stationary_weights_and_has_feasible_events():
    f = stationary_occupations()
    odds = [p/(1-p) for p in f]
    for (a, b), (c, d) in R.collision_map().items():
        assert odds[a]*odds[b] == odds[c]*odds[d]
    activity = equivariant_collision_activity()
    assert activity["exactly_two"] == exactly_two_probability(f)
    assert Q(0) < activity["shell_energy_exchange"] < activity["changed_pair"] < activity["exactly_two"] < Q(1)


def test_shear_product_tangent_generates_correlations_at_first_order():
    witness = shear_tangent_obstruction()
    assert witness["incoming_score"] == 13
    assert witness["outgoing_score"] == 12
    assert witness["pushed_score_mixed_difference"] == -1
    # Verify the mixed difference against actual finite state transitions;
    # a genuinely additive score would give zero on these four configurations.
    a, b = (R.channel(v) for v in witness["input"])
    score = []
    for bits in (0, 1 << a, 1 << b, (1 << a) | (1 << b)):
        state = R.Records.empty(3)
        state.bank[0] = bits
        R.step(state)
        score.append(sum(R.VELOCITIES[c][0]*R.VELOCITIES[c][1] for c in R.occupied(state.bank[0])))
    assert score[3]-score[1]-score[2]+score[0] == -1


def test_exact_shear_projection_leakage_is_positive_and_fully_accounted():
    evidence = shear_tangent_leakage()
    assert evidence["fugacity"] == Q(1, 8)
    assert evidence["energy_activity"] == Q(1, 2)
    assert 0 < evidence["lost_squared_norm"] < evidence["incoming_squared_norm"]
    assert evidence["incoming_squared_norm"] == evidence["projected_squared_norm"]+evidence["lost_squared_norm"]
    assert evidence["lost_squared_fraction"] == evidence["lost_squared_norm"]/evidence["incoming_squared_norm"]
    # Independently reproduced exact rational result; no rounded tolerance.
    assert sha256(str(evidence["lost_squared_fraction"]).encode("ascii")).hexdigest() == (
        "747dfc05e8f19ee0f4254370ddb6c5e4133be497acc72540c2734f6ee35b9596")
