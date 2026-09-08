"""Locked exact transport checks; every simulation is the actual staged law."""
from dataclasses import replace
from fractions import Fraction

import numpy as np
import pytest

from phi_v2_lattice import channels as C, staged as P, state as S
from phi_v2_lattice import recovery_transport as R


@pytest.fixture(scope="module")
def tables():
    return C.load_collision_tables()


def test_all_channels_layers_and_backgrounds_close_with_charged_drift():
    orbits = R.channel_orbits()
    assert len(orbits) == 32
    assert sorted(c for orbit in orbits for c in orbit) == list(range(384))
    assert {len(orbit) for orbit in orbits} == {12}
    velocities = set()
    for channel in range(384):
        certificate = R.orbit_certificate(channel)
        assert certificate.period_microticks == 48
        assert {abs(x) for x in certificate.period_displacement} == {4}
        assert certificate.velocity == tuple(Fraction(x,48) for x in certificate.period_displacement)
        assert certificate.integer_tick_ripple_linf == Fraction(5,6)
        assert certificate.held_time_ripple_linf == Fraction(11,12)
        velocities.add(certificate.velocity)
    assert len(velocities) == 8
    for layer in range(3):
        for background in range(9):
            if background == S.BLANK_IDX:
                continue
            certificate = R.orbit_certificate(0,layer,background)
            assert len(certificate.layer_orbit) == 3
            assert len(certificate.background_orbit) == 4
            assert certificate.period_microticks == 48


@pytest.mark.parametrize("representative", [orbit[0] for orbit in R.channel_orbits()])
def test_actual_runtime_full_period_all_orbits_and_all_pending_fields(tables,representative):
    index = [orbit[0] for orbit in R.channel_orbits()].index(representative)
    backgrounds = [b for b in range(9) if b != S.BLANK_IDX]
    state = R.prepare_single_token(3,index%27,representative,index%3,backgrounds[index%8])
    original = P.checkpoint(state)
    audited = R.audit_singleton_trajectory(state,48,tables)
    assert P.checkpoint(state) == original
    assert audited.token_count == 18*27+1
    actual_displacement = tuple(audited.lifted_positions[-1][a]-audited.initial_position[a] for a in range(3))
    assert actual_displacement == audited.certificate.period_displacement
    assert audited.observed_max_tick_error_linf == Fraction(5,6)


@pytest.mark.parametrize("size,channel", [(4,0),(4,226),(7,34),(7,305)])
def test_multiple_cycles_seams_and_nonzero_global_start(tables,size,channel):
    state = R.prepare_single_token(size,size**3-1,channel)
    state.microtick = 2**65  # exact observer ordinal, phase zero; Python limit differs from native uint64
    audited = R.audit_singleton_trajectory(state,97,tables)
    assert audited.tick_end == 2**65+97
    for elapsed,position in enumerate(audited.lifted_positions):
        expected = R.lifted_displacement(channel,elapsed)
        assert position == tuple(audited.initial_position[a]+expected[a] for a in range(3))


def test_deterministic_finite_counting_transport_bound_scales_uniformly(tables):
    a = R.audit_singleton_trajectory(R.prepare_single_token(3,0,0),49,tables)
    opposite = next(orbit[0] for orbit in R.channel_orbits()
                    if R.orbit_certificate(orbit[0]).velocity == tuple(-v for v in a.certificate.velocity))
    b = R.audit_singleton_trajectory(R.prepare_single_token(3,26,opposite),49,tables)
    assert b.certificate.velocity != a.certificate.velocity
    for spacing in (Fraction(1),Fraction(1,10),Fraction(1,100)):
        bound = R.empirical_transport_bound([a,b],[2,3],spacing=spacing,tick_duration=spacing)
        assert bound.total_multiplicity == 5
        assert bound.horizon_microticks == 49
        assert bound.measured_grid_time_pairing_bound == spacing*Fraction(5,6)
        assert bound.uniform_held_time_pairing_bound == spacing*Fraction(11,12)
        assert bound.physical_velocities == (a.certificate.velocity,b.certificate.velocity)
        # Direct same-label empirical pairing and one Lipschitz test observable.
        for tick in range(50):
            differences = []
            observable_error = Fraction(0)
            for weight,trace in ((2,a),(3,b)):
                delta = tuple(spacing*(trace.lifted_positions[tick][axis]-trace.initial_position[axis]
                                      -tick*trace.certificate.velocity[axis]) for axis in range(3))
                differences.append(weight*max(map(abs,delta)))
                observable_error += weight*delta[0]
            assert sum(differences)/5 <= bound.uniform_held_time_pairing_bound
            assert abs(observable_error)/5 <= bound.uniform_held_time_pairing_bound


def test_held_time_bound_includes_ripple_not_just_cycle_boundaries():
    c = R.orbit_certificate(0)
    for tick in range(c.period_microticks):
        position = R.lifted_displacement(0,tick)
        for fraction in (Fraction(0),Fraction(1,2),Fraction(999,1000)):
            error = max(abs(position[a]-(tick+fraction)*c.velocity[a]) for a in range(3))
            assert error <= c.held_time_ripple_linf
    assert c.held_time_ripple_linf > c.integer_tick_ripple_linf


def test_closed_form_handles_large_finite_horizons_without_skipping_runtime_claim():
    c = R.orbit_certificate(34)
    period_count = 10**20
    for remainder in range(48):
        large = R.lifted_displacement(34,48*period_count+remainder)
        small = R.lifted_displacement(34,remainder)
        assert large == tuple(period_count*c.period_displacement[a]+small[a] for a in range(3))
    # This is an exact algebraic identity, not a claim that 10^20 cycles were run.


@pytest.mark.parametrize("corruption", ["two_tokens","blank_background","nonuniform_background","nonuniform_layer","manifestation","phase"])
def test_outside_sector_is_rejected_not_promoted(corruption):
    state = R.prepare_single_token(3)
    if corruption == "two_tokens": state.lattice.bank[1,1]=True
    elif corruption == "blank_background": state.lattice.sc[:]=S.BLANK_IDX
    elif corruption == "nonuniform_background": state.lattice.fcc.flat[0]=(int(state.lattice.fcc.flat[0])+1)%9
    elif corruption == "nonuniform_layer": state.lattice.ell[0]=1
    elif corruption == "manifestation": state.lattice.s[0]=1
    else: state.microtick=1
    with pytest.raises(ValueError): R.audit_singleton_trajectory(state,0)


def test_actual_runtime_intervention_is_detected(tables,monkeypatch):
    original = P.step
    def broken(state,tables):
        out,events=original(state,tables)
        out.lattice.bank[1,1]=True
        return out,events
    monkeypatch.setattr(P,"step",broken)
    with pytest.raises(ValueError,match="singleton"):
        R.audit_singleton_trajectory(R.prepare_single_token(3),1,tables)


def test_invalid_external_scaling_and_counting_inputs(tables):
    audit = R.audit_singleton_trajectory(R.prepare_single_token(3),0,tables)
    for value in (0,-1,True,.5):
        with pytest.raises(ValueError): R.empirical_transport_bound([audit],[1],spacing=value)
    for weights in ([0],[-1],[True],[]):
        with pytest.raises(ValueError): R.empirical_transport_bound([audit],weights)
    with pytest.raises(ValueError):
        R.empirical_transport_bound([audit,replace(audit,tick_end=1)],[1,1])


@pytest.mark.parametrize("corruption", ["negative_error", "wrong_error", "clock", "position", "certificate", "count"])
def test_external_report_cannot_supply_an_unchecked_bound(tables, corruption):
    audit = R.audit_singleton_trajectory(R.prepare_single_token(3), 3, tables)
    if corruption == "negative_error": audit = replace(audit, observed_max_tick_error_linf=Fraction(-1))
    elif corruption == "wrong_error": audit = replace(audit, observed_max_tick_error_linf=Fraction(0))
    elif corruption == "clock": audit = replace(audit, tick_end=-1)
    elif corruption == "position": audit = replace(audit, lifted_positions=((0,0,0),)*4)
    elif corruption == "certificate": audit = replace(audit, certificate=replace(audit.certificate, held_time_ripple_linf=Fraction(0)))
    else: audit = replace(audit, token_count=1)
    with pytest.raises(ValueError):
        R.empirical_transport_bound([audit],[1])
