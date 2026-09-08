"""Exact finite-map census and bounded deterministic fixtures, not a campaign."""
from collections import Counter

import numpy as np
import pytest

from phi_v2_lattice import channels as C, geometry as G, staged as P
from phi_v2_lattice import recovery_pair_sector as R


def test_all_frozen_collision_rows_and_invariant_subsector():
    result=R.census()
    for layer,row in enumerate(result):
        assert row == {"rows":18336,"hop_multiset_changed":(96,288,384)[layer],
                       "drift_multiset_changed":480,"drift_sum_changed":288,
                       "same_flag_inputs":288}


def test_all_same_flag_layer_orbits_return_and_charge_background_period():
    orbits=R.colocated_orbits()
    assert Counter(len(o.states) for o in orbits)=={3:96,12:48}
    assert sum(len(o.states) for o in orbits)==864
    assert len({s for o in orbits for s in o.states})==864
    for orbit in orbits:
        assert orbit.full_period_microticks==48
        absolute=sorted(map(abs,orbit.full_period_displacement))
        assert absolute==([4,4,4] if len(orbit.states)==3 else [0,4,4])
        q,pair=orbit.states[0]
        for expected in (*orbit.states[1:],orbit.states[0]):
            out=R.collide(pair,q)
            q,pair=(q-1)%3,tuple(sorted(C.U(c) for c in out))
            assert (q,pair)==expected


def test_free_relative_motion_is_three_hop_periodic_modulo_drift_for_all_channels():
    for first in range(384):
        second=(first+137)%384
        delta=tuple(b-a for a,b in zip(R.drift_numerator(first),R.drift_numerator(second)))
        for r in range(3):
            start=R.free_relative(first,second,r)
            assert R.free_relative(first,second,3*10**20+r)==tuple(10**20*d+x for d,x in zip(delta,start))
        for r in range(3):
            assert max(map(abs,R.free_relative(first,second,r)))<=2


def test_opposite_polarity_never_collides_even_if_same_internal_state():
    for first in range(192):
        for second in range(192,384):
            for layer in range(3):
                assert R.collide((second,first),layer)==(first,second)


@pytest.mark.parametrize("pair,expected_vertices", [((0,1),12),((192,193),12),((0,193),0),((0,192),0)])
def test_actual_repeated_pair_interactions_include_every_live_record(pair,expected_vertices):
    state=R.prepare_pair(3,*pair,site=26)
    original=P.checkpoint(state)
    trace=R.audit_colocated(state,12)
    assert P.checkpoint(state)==original
    assert trace.tick_end-trace.tick_start==48
    assert trace.work==18*27+2
    assert trace.collision_vertices==expected_vertices
    assert not trace.binding_identified
    displacement=tuple(b-a for a,b in zip(trace.positions[0],trace.positions[-1]))
    if expected_vertices:
        assert displacement==(-4,0,4)
        assert displacement!=R.free_displacement(pair[0],12)
    else:
        assert displacement==R.free_displacement(pair[0],12)
    for tick in range(1,49):
        delta=tuple(b-a for a,b in zip(trace.positions[tick-1],trace.positions[tick]))
        assert sum(map(abs,delta))==int(tick%4==3)


def test_actual_spatial_scattering_changes_outgoing_direction_multiset():
    original=(0,32)
    obs=R.scattering(original,0)
    assert obs.outgoing==(144,176)
    assert obs.free_hops==((-1,0,0),(1,0,0))
    assert obs.actual_hops==((0,0,-1),(0,0,1))
    L=5;site=G.site_index(L,2,2,2)
    state=R.prepare_pair(L,*original,site=site)
    state,_=P.step(state)
    state,events=P.step(state)
    assert events.collisions==[(site,1,original,obs.outgoing)]
    state,_=P.step(state)
    actual={tuple(map(int,p)) for p in np.argwhere(state.lattice.bank)}
    expected={(G.shift(L,site,C.tangent(c)),C.U(c)) for c in obs.outgoing}
    disabled={(G.shift(L,site,C.tangent(c)),C.U(c)) for c in original}
    assert actual==expected and actual!=disabled
    # Opposite polarity is an ACTUAL law no-collision control, not a runtime toggle.
    control=R.prepare_pair(L,0,224,site=site)
    for _ in range(3):control,_=P.step(control)
    assert {tuple(map(int,p)) for p in np.argwhere(control.lattice.bank)}=={
        (G.shift(L,site,C.tangent(c)),C.U(c)) for c in (0,224)}


@pytest.mark.parametrize("pair", [(0,0),(True,1),(-1,0),(0,384),(0,), (0,1,2)])
def test_invalid_colocated_channel_pairs_rejected(pair):
    with pytest.raises(ValueError):R.collide(pair,0)


def test_scoped_audit_rejects_noninvariant_preparation():
    with pytest.raises(ValueError):R.audit_colocated(R.prepare_pair(3,0,32),1)
    state=R.prepare_pair(3,0,1)
    state.lattice.bank[0,1]=False;state.lattice.bank[1,1]=True
    with pytest.raises(ValueError):R.audit_colocated(state,1)
    for value in (-1,True,.5):
        with pytest.raises(ValueError):R.free_relative(0,192,value)


def test_one_site_relative_perturbation_has_no_restoring_interaction():
    state=R.prepare_pair(3,0,1)
    displaced=G.shift(3,0,(1,0,0))
    state.lattice.bank[0,1]=False;state.lattice.bank[displaced,1]=True
    channels=(0,1)
    sites=(0,displaced)
    for tick in range(48):
        state,events=P.step(state)
        assert not events.collisions and not events.absorptions
        if tick%4==2:
            sites=tuple(G.shift(3,x,C.tangent(c)) for x,c in zip(sites,channels))
            channels=tuple(C.U(c) for c in channels)
        assert sites[1]==G.shift(3,sites[0],(1,0,0))
        assert {tuple(map(int,p)) for p in np.argwhere(state.lattice.bank)}==set(zip(sites,channels))


def test_injected_runtime_disagreement_is_rejected(monkeypatch):
    original=P.step
    def broken(state):
        result,events=original(state)
        result.gate_sc[0,0]=False
        return result,events
    monkeypatch.setattr(P,"step",broken)
    with pytest.raises(ValueError,match="background"):
        R.audit_colocated(R.prepare_pair(3,0,1),1)
