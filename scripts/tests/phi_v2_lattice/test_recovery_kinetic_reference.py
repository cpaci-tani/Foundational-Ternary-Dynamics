"""Finite operator checks and bounded fixtures, not a measurement campaign."""
from fractions import Fraction
from itertools import combinations
from math import comb

import numpy as np
import pytest

from phi_v2_lattice import channels as C, geometry as G, staged as P, state as S
from phi_v2_lattice import recovery_kinetic_reference as K
from phi_v2_lattice._proofs import encode, rotate


def test_complete_pair_operators_and_streaming_coordinates_are_permutations():
    certificate = K.finite_map_certificate()
    assert certificate["pair_states_per_layer"] == comb(192, 2)
    pairs = set(combinations(range(192), 2))
    for table in C.load_collision_tables():
        assert set(table) == set(table.values()) == pairs
    destinations = {(G.shift(3, x, C.tangent(c)), C.U(c))
                    for x in range(27) for c in range(384)}
    assert len(destinations) == 27*384
    assert certificate["record_family_period_microticks"] == 48
    assert not certificate["stationary_absolute_clock"]


@pytest.mark.parametrize("layer", range(3))
@pytest.mark.parametrize("kind", ("empty", "full", "dense", "pairs"))
def test_actual_four_stage_maps_preserve_arbitrary_bank_reference_support(layer, kind):
    bank = np.zeros((27, 384), dtype=bool)
    if kind == "full": bank[:] = True
    if kind == "dense": bank[:] = (np.arange(bank.size).reshape(bank.shape) % 5 < 2)
    if kind == "pairs":
        bank[0, [0, 1]] = True
        bank[1, [204, 205]] = True
        bank[26, [0, 1, 2]] = True  # Non-two cardinality sector stays local identity.
    state = K.prepare_bank(bank, 3, layer)
    initial_background = int(state.lattice.sc.flat[0])
    initial_count = int(bank.sum())
    weights = [K.CountingReference(3, p).multiplicity(initial_count)
               for p in (Fraction(1, 2), Fraction(1, 96))]
    gates = None
    for tick in range(4):
        before = state
        state, events = P.step(state)
        assert int(state.lattice.bank.sum()) == initial_count
        assert not events.absorptions and not events.crossings and not events.gate_holds
        assert not state.admitted_sc.any()
        assert not state.lattice.s.any()
        expected_background = initial_background if tick == 0 else S.idx_of(rotate(S.z_of(initial_background)))
        assert np.all(state.lattice.sc == expected_background)
        assert np.all(state.lattice.fcc == expected_background)
        assert np.all(state.lattice.ell == (layer-(tick >= 1)) % 3)
        if tick == 0:
            gates = state.gate_sc.copy(), state.gate_fcc.copy()
        elif tick < 3:
            np.testing.assert_array_equal(state.gate_sc, gates[0])
            np.testing.assert_array_equal(state.gate_fcc, gates[1])
        else:
            assert not state.gate_sc.any() and not state.gate_fcc.any()
        if kind == "pairs" and tick == 1:
            assert len(events.collisions) == 2
            np.testing.assert_array_equal(state.lattice.bank[26], before.lattice.bank[26])
            table = C.load_collision_tables()[layer]
            assert tuple(np.flatnonzero(state.lattice.bank[0])) == table[(0, 1)]
            assert tuple(np.flatnonzero(state.lattice.bank[1])-192) == table[(12, 13)]
    assert [K.CountingReference(3, p).multiplicity(int(state.lattice.bank.sum()))
            for p in (Fraction(1, 2), Fraction(1, 96))] == weights


def test_full_phase_lift_retains_correlated_gates_and_does_not_mutate_preparation():
    bank = np.zeros((27,384), dtype=bool)
    bank[0, [0,1]] = True
    bank[1,0] = True
    boundary = K.prepare_bank(bank,3)
    phases = [K.phase_lift(boundary,j) for j in range(4)]
    assert boundary.microtick == 0 and not boundary.gate_sc.any()
    for lifted in phases[1:]:
        np.testing.assert_array_equal(lifted.gate_sc, phases[1].gate_sc)
        np.testing.assert_array_equal(lifted.gate_fcc, phases[1].gate_fcc)
    empty = K.phase_lift(K.prepare_bank(np.zeros_like(bank),3),1)
    one = np.zeros_like(bank); one[0,0] = True
    occupied = K.phase_lift(K.prepare_bank(one,3),1)
    assert empty.gate_sc[0,0] and not occupied.gate_sc[0,0]
    assert K.even_gate_weight(Fraction(1,2)) == Fraction(1,2)
    # The gate is deterministic given the complete bank, although its marginal
    # is one half at this reference. Do not replace it by an independent bit.
    assert K.even_gate_weight(0) == K.even_gate_weight(1) == 1


def test_background_layer_family_returns_after_48_actual_microticks():
    bank = np.zeros((27,384),dtype=bool); bank[0,[0,1]] = True
    state = K.prepare_bank(bank,3,2,S.idx_of(encode(3,-1)))
    initial = state.lattice.sc.copy(),state.lattice.fcc.copy(),state.lattice.ell.copy()
    for _ in range(48): state,_ = P.step(state)
    for observed,expected in zip((state.lattice.sc,state.lattice.fcc,state.lattice.ell),initial):
        np.testing.assert_array_equal(observed,expected)
    assert state.microtick == 48 and not state.gate_sc.any() and not state.gate_fcc.any()
    assert not state.lattice.s.any() and int(state.lattice.bank.sum()) == 2


def test_exact_counting_weights_include_degenerate_endpoints():
    zero,one = K.CountingReference(3,0),K.CountingReference(3,1)
    assert zero.multiplicity(0) == one.multiplicity(one.slots) == 1
    assert zero.multiplicity(1) == one.multiplicity(0) == 0
    half = K.CountingReference(3,Fraction(1,2))
    assert half.multiplicity(0) == half.multiplicity(half.slots) == 1
    assert half.total_multiplicity == 2**half.slots
    reference = K.CountingReference(3,Fraction(2,3))
    assert reference.multiplicity(7) == 2**7
    assert reference.configuration_weight(7) == Fraction(2**7,3**reference.slots)


def test_eligibility_maximum_and_actual_stage_timing_are_exact():
    optimum = Fraction(1,96)
    assert K.collision_eligibility(optimum) == comb(192,2)*optimum**2*(1-optimum)**190
    assert K.collision_eligibility(Fraction(1,2)) == Fraction(comb(192,2),2**192)
    assert K.eligibility_derivative_sign(Fraction(1,192)) == 1
    assert K.eligibility_derivative_sign(optimum) == 0
    assert K.eligibility_derivative_sign(Fraction(1,2)) == -1
    assert K.collision_eligibility(0) == K.collision_eligibility(1) == 0
    for start in range(4):
        for horizon in range(9):
            f = K.feasibility(3,horizon,optimum,start)
            actual = sum(t % 4 == 1 for t in range(start,start+horizon))
            assert f["collision_stages"] == actual
            assert f["expected_logged_pair_events"] == 54*actual*K.collision_eligibility(optimum)
            assert not f["temporal_independence_assumed"]
    assert K.feasibility(9,64)["collision_stages"] == 16


@pytest.mark.parametrize("invalid", (True,0.5,Fraction(-1,2),Fraction(3,2)))
def test_inexact_or_invalid_weights_rejected(invalid):
    with pytest.raises(ValueError): K.CountingReference(3,invalid)
    with pytest.raises(ValueError): K.collision_eligibility(invalid)


def test_lifts_and_preparations_reject_invalid_sector_and_noncanonical_bool():
    bank = np.zeros((27,384),dtype=bool)
    bank.view(np.uint8)[0,0] = 2
    with pytest.raises(ValueError): K.prepare_bank(bank,3)
    bank[0,0] = False
    state = K.prepare_bank(bank,3)
    state.lattice.sc[0,0,0] = S.BLANK_IDX
    with pytest.raises(ValueError): K.phase_lift(state,1)
