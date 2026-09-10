"""Exact finite checks plus explicitly sampled trajectory/locality fixtures."""
from fractions import Fraction as Q
from hashlib import sha256
from random import Random
import struct

import pytest
from scripts.phi_v2_lattice.thermal import runtime as R
from scripts.phi_v2_lattice.thermal.analysis import (
    d1q7_weights, stationary_occupations, equilibrium_moments, exactly_two_probability,
)


def fixture(L=5):
    rng = Random(20260910)
    state = R.Records.empty(L)
    for site in range(L**3):
        for c in rng.sample(range(R.CHANNELS), rng.randrange(5)):
            state.bank[site] |= 1 << c
    return state


def test_exhaustive_pair_permutation_conserves_and_exchanges_shell_energy():
    mapping = R.collision_map()
    assert len(mapping) == R.CHANNELS*(R.CHANNELS-1)//2
    assert set(mapping) == set(mapping.values())
    exchanges = 0
    for (a, b), (c, d) in mapping.items():
        assert a < b and c < d
        assert tuple(R.VELOCITIES[a][i]+R.VELOCITIES[b][i] for i in range(3)) == tuple(
            R.VELOCITIES[c][i]+R.VELOCITIES[d][i] for i in range(3))
        assert R.ENERGY2[a]+R.ENERGY2[b] == R.ENERGY2[c]+R.ENERGY2[d]
        exchanges += sorted((R.ENERGY2[a], R.ENERGY2[b])) != sorted((R.ENERGY2[c], R.ENERGY2[d]))
    assert exchanges > 0


def test_phase_complete_codec_and_exact_sampled_continuation():
    state = fixture()
    expected = R.totals(state)
    for _ in range(12):
        snapshot = R.encode(state)
        restored = R.decode(snapshot)
        assert R.encode(restored) == snapshot
        R.advance(restored, 3)
        repeated = R.decode(snapshot)
        for _ in range(3): R.step(repeated)
        assert R.encode(restored) == R.encode(repeated)
        R.step(state)
        assert R.totals(state) == expected


@pytest.mark.parametrize("phase", range(4))
def test_sampled_microtick_dependencies_stay_within_one_moore_hop(phase):
    state = fixture(7)
    state.microtick = phase
    changed = R.decode(R.encode(state))
    site = 3+7*(3+7*3)
    changed.bank[site] ^= (1 << R.channel((3, -2, 1)))
    R.step(state); R.step(changed)
    for i, (a, b) in enumerate(zip(state.bank, changed.bank)):
        if a != b:
            pos = (i % 7, i//7 % 7, i//49)
            assert max(abs(x-3) for x in pos) <= 1


def test_streaming_cycle_displacement_and_physical_clock():
    state = R.Records.empty(7)
    origin = 2+7*(3+7*4)
    c = R.channel((3, -2, 1))
    state.bank[origin] = 1 << c
    R.advance(state, 4)
    assert state.microtick == 4
    assert state.bank[5+7*(1+7*5)] == 1 << c
    assert R.totals(state) == (1, (3, -2, 1), 14)


def test_overflow_invalid_shape_reserved_bits_and_checksum_are_atomic():
    state = fixture()
    state.microtick = R.MAX_TICK
    before = R.encode(state)
    with pytest.raises(OverflowError): R.step(state)
    with pytest.raises(ValueError): R.advance(state, 1)
    assert R.encode(state) == before
    data = bytearray(before); data[-1] ^= 1
    with pytest.raises(ValueError): R.decode(bytes(data))
    data = bytearray(before); data[96+47] |= 128
    data[-32:] = sha256(data[:-32]).digest()
    with pytest.raises(ValueError): R.decode(bytes(data))
    for offset, fmt, value in [(8, '<I', 9), (12, '<I', 0), (88, '<Q', 0)]:
        data = bytearray(before); struct.pack_into(fmt, data, offset, value)
        data[-32:] = sha256(data[:-32]).digest()
        with pytest.raises(ValueError): R.decode(bytes(data))
    state.bank.pop()
    damaged = state.bank.copy()
    with pytest.raises(ValueError): R.step(state)
    assert state.bank == damaged and state.microtick == R.MAX_TICK


def test_manifestation_is_declared_quotient_not_extra_state():
    assert [R.manifestation(x) for x in (0, 1, 3, 7)] == [0, 1, -1, 1]


def test_degree_six_moment_feasibility_exact_rational_points():
    # Full interval positivity is proven symbolically in the law specification;
    # these points verify the implemented rational formulas, not that proof.
    for t in (Q(3, 4), Q(1), Q(5, 4)):
        weights = d1q7_weights(t)
        assert all(w > 0 for w in weights.values())
        assert sum(weights.values()) == 1
        for k, target in [(2, t), (4, 3*t*t), (6, 15*t**3)]:
            assert sum(w*v**k for v, w in weights.items()) == target


def test_gibbs_pair_probability_is_exactly_invariant():
    f = stationary_occupations()
    odds = [p/(1-p) for p in f]
    for members in R.collision_classes():
        expected = odds[members[0][0]] * odds[members[0][1]]
        assert all(odds[a]*odds[b] == expected for a, b in members)


def test_stationary_measure_is_not_moment_feasibility_or_fluid_certificate():
    moments = equilibrium_moments()
    assert moments['fourth_isotropy_residual'] != 0
    assert moments['sixth_isotropy_residual'] != 0
    assert Q(0) < exactly_two_probability(stationary_occupations()) < Q(1)
    probability_half = Q(R.CHANNELS*(R.CHANNELS-1)//2, 2**R.CHANNELS)
    assert probability_half < Q(1, 10**98)


def test_pinned_symmetry_failure_is_not_promoted_to_isotropy():
    rotate = lambda c: R.channel((R.VELOCITIES[c][0], -R.VELOCITIES[c][2], R.VELOCITIES[c][1]))
    pair = (R.channel((-1, 0, 0)), R.channel((1, 0, 0)))
    rotated_input = tuple(sorted(map(rotate, pair)))
    assert rotated_input == pair
    assert R.collision_map()[pair] == (R.channel((0, -1, 0)), R.channel((0, 1, 0)))
    assert R.collision_map()[rotated_input] != tuple(sorted(map(rotate, R.collision_map()[pair])))


def test_degree_six_tensor_preparation_has_nonstationary_collision_weight():
    q = d1q7_weights(Q(1))
    probabilities = [q[x]*q[y]*q[z]/8 for x, y, z in R.VELOCITIES]
    odds = [f/(1-f) for f in probabilities]
    pair = (R.channel((0, 0, 0)), R.channel((2, 0, 0)))
    a, b = pair
    c, d = R.collision_map()[pair]
    assert odds[a]*odds[b]/(odds[c]*odds[d]) == Q(1809739687, 2398596583)
