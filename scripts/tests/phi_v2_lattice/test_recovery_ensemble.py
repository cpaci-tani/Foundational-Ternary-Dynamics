"""Complete finite ensembles retain correlations and exact expiry weights."""
from fractions import Fraction

import pytest

from phi_v2_lattice import prepare, staged as P, state as S
from phi_v2_lattice.recovery_ensemble import CountedCheckpoint, FiniteEnsemble


def blank():
    return P.initialize(S.blank(3))


def test_exact_multiplicities_survive_complete_state_expiry():
    a, b = blank(), blank()
    a.lattice.s[0] = 1
    b.lattice.s[0] = -1
    ensemble = FiniteEnsemble.from_states([(a, 2**60 + 1), (b, 2**60)])
    assert len(ensemble.atoms) == 2
    assert ensemble.expectation(lambda st: int(st.lattice.s[0])) == Fraction(1, 2**61 + 1)
    before = ensemble.advance(3)
    assert len(before.atoms) == 2
    after = before.advance(1)
    assert len(after.atoms) == 1
    assert after.total_count == 2**61 + 1
    assert after.microtick == 4
    assert after.atoms[0].checkpoint == P.checkpoint(P.step(P.step(P.step(P.step(a)[0])[0])[0])[0])


def test_inputs_and_mutating_diagnostic_do_not_change_ensemble():
    st = blank()
    ensemble = FiniteEnsemble.from_states([(st, 1), (st, 2)])
    st.lattice.bank[0, 0] = True
    assert ensemble.total_count == 3
    assert ensemble.expectation(P.work_units) == 0
    def corrupt_copy(state):
        state.lattice.bank[0, 0] = True
        return P.work_units(state)
    assert ensemble.expectation(corrupt_copy) == 1
    assert ensemble.expectation(P.work_units) == 0


def test_actual_four_member_ensemble_retains_collision_correlations():
    members = []
    for occupied in ((), (0,), (1,), (0, 1)):
        st = P.initialize(prepare.r5_vacuum(3, seed=0, occupation=0))
        st.lattice.bank[13, list(occupied)] = True
        members.append((st, 1))
    ensemble = FiniteEnsemble.from_states(members)
    x = lambda st: int(st.lattice.bank[13, 4])
    y = lambda st: int(st.lattice.bank[13, 5])
    assert ensemble.covariance(x, y) == 0
    after = ensemble.advance(2)
    assert after.expectation(x) == after.expectation(y) == Fraction(1, 4)
    assert after.expectation(lambda st: x(st)*y(st)) == Fraction(1, 4)
    assert after.covariance(x, y) == Fraction(3, 16)
    assert after.expectation(P.work_units) == ensemble.expectation(P.work_units)
    # Exact full pushforward has the semigroup property; no marginal reinsertion.
    assert ensemble.advance(8) == after.advance(6)


@pytest.mark.parametrize("weight", [0, -1, True, 0.5])
def test_invalid_counting_weights_rejected(weight):
    with pytest.raises(ValueError):
        FiniteEnsemble.from_states([(blank(), weight)])


def test_frame_resource_and_float_rejection():
    a, b = blank(), blank()
    b.lattice.s[0] = 1
    with pytest.raises(ValueError, match="support"):
        FiniteEnsemble.from_states([(a, 1), (b, 1)], max_support=1)
    with pytest.raises(ValueError, match="clock"):
        FiniteEnsemble.from_states([(a, 1), (P.step(b)[0], 1)])
    with pytest.raises(ValueError, match="lattice"):
        FiniteEnsemble.from_states([(a, 1), (P.initialize(S.blank(4)), 1)])
    ensemble = FiniteEnsemble.from_states([(a, 1), (b, 1)])
    with pytest.raises(ValueError, match="budget"):
        ensemble.advance(4, max_updates=7)
    assert ensemble.microtick == 0 and len(ensemble.atoms) == 2
    with pytest.raises(TypeError, match="exact"):
        ensemble.expectation(lambda st: 0.1)
    assert ensemble.provenance()["marginal_reprojection"] is False
    with pytest.raises(ValueError):
        FiniteEnsemble(())
    with pytest.raises((ValueError, TypeError)):
        FiniteEnsemble((CountedCheckpoint(b"corrupt", 1),))


def test_support_limit_stops_preparation_iterator_before_later_branches():
    def members():
        a, b = blank(), blank()
        b.lattice.s[0] = 1
        yield a, 1
        yield b, 1
        raise AssertionError("support budget must reject before preparing another branch")
    with pytest.raises(ValueError, match="support"):
        FiniteEnsemble.from_states(members(), max_support=1)
