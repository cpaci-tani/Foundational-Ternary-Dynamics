"""Small exact fixtures for conservation, sources and the continuum error budget."""
from fractions import Fraction as F
from itertools import product

import numpy as np
import pytest

from scripts.phi_v2_lattice import geometry as G
from scripts.phi_v2_lattice._proofs import A9, BLANK, relation_tick
from scripts.phi_v2_lattice.hydro import channels as H, codec, prepare, staged, state
from scripts.phi_v2_lattice.hydro.weak_balance import (
    acoustic_taylor_bound, admit_conservative_sector, certify_cycle, observe_checkpoint,
)


def _cycle(channels, *, frozen=True, phase=0, origin=0):
    """One tiny actual-law cycle; no random inputs or parameter choices."""
    lattice = prepare.frozen_background(3) if frozen else state.blank(3)
    for pol, velocity in channels:
        lattice.bank[origin, H.channel(pol, phase, velocity)] = True
    initial = staged.initialize(lattice)
    first, _ = staged.step(initial)
    prestream, _ = staged.step(first)
    third, _ = staged.step(prestream)
    final, _ = staged.step(third)
    return tuple(codec.checkpoint(st) for st in (initial, prestream, final))


def _quartic(t):
    t %= 1
    return t*t*(1-t)*(1-t)


def _probe(L):
    return tuple(sum((a+1)*_quartic(F(coord, L)) for a, coord in enumerate(G.coords(L, x)))
                 for x in range(L**3))


def test_complete_checkpoint_observation_is_exact_owned_and_immutable():
    a, _, _ = _cycle(((0, 12), (1, 2)), origin=26)
    snapshot = observe_checkpoint(a)
    assert snapshot.moments[0][26] == (1, 1, 1, 0, 1, 1, 0, 1, 0, 0)
    assert snapshot.moments[1][26] == (1, -1, 0, 0, 1, 0, 0, 0, 0, 0)
    assert snapshot.field_tokens == (1, 1)
    assert snapshot.relation_tokens == 18*27
    assert snapshot.total_tokens == 18*27+2
    with pytest.raises((AttributeError, TypeError)):
        snapshot.moments[0][26][0] = 7
    restored = codec.restore(a)
    restored.lattice.bank.fill(False)
    assert snapshot.field_tokens == (1, 1)


def test_conservative_balance_all_components_and_periodic_seam():
    a, b, c = _cycle(((0, 12), (1, 2)), origin=26)
    result = certify_cycle(a, b, c, _probe(3), cell_volume=F(1, 27))
    assert result.conservative_sector
    assert result.tokens == (488, 488, 488)
    assert result.weak_source == ((0, 0, 0, 0), (0, 0, 0, 0))
    assert result.weak_change == result.weak_stream
    assert result.weak_change[0][0] != 0
    assert admit_conservative_sector(a).fully_occupied_relations


def test_absorption_source_is_local_signed_and_owner_accounted():
    a, b, c = _cycle(((1, 2),), frozen=False, phase=2, origin=0)
    result = certify_cycle(a, b, c, tuple(F(x+1, 7) for x in range(27)))
    assert not result.conservative_sector
    assert result.tokens == (1, 1, 1)
    assert result.absorbed_moments[1][0] == (1, -1, 0, 0)
    assert sum(result.relation_arrivals) == 1
    assert result.relation_arrivals[G.shift(3, 0, (-1, 0, 0))] == 1
    assert result.weak_change[1] == (-F(1, 7), F(1, 7), 0, 0)
    assert result.weak_stream[1] == (0, 0, 0, 0)
    with pytest.raises(ValueError, match="conservative sector"):
        admit_conservative_sector(a)


def test_collision_error_cannot_be_relabelled_as_an_absorption_source():
    a, b, c = _cycle(((0, 0),))
    corrupt = codec.restore(b)
    corrupt.lattice.bank.fill(False)
    with pytest.raises(ValueError, match="local collision/source"):
        certify_cycle(a, codec.checkpoint(corrupt), c, _probe(3))


def test_wrong_stream_or_phase_is_rejected_even_with_same_global_counts():
    a, b, c = _cycle(((0, 0),))
    corrupt = codec.restore(c)
    site, channel = map(int, np.argwhere(corrupt.lattice.bank)[0])
    corrupt.lattice.bank[site, channel] = False
    corrupt.lattice.bank[site, H.channel(0, 3, 0)] = True
    with pytest.raises(ValueError, match="phase-resolved"):
        certify_cycle(a, b, codec.checkpoint(corrupt), _probe(3))


def test_n_and_p_do_not_close_even_at_full_site_resolution():
    # The frozen table fixes all two-particle masks. Both preparations have N=2,P=0.
    axial_x = _cycle(((0, 0), (0, 2)))
    axial_y = _cycle(((0, 4), (0, 6)))
    x0, y0 = observe_checkpoint(axial_x[0]), observe_checkpoint(axial_y[0])
    assert tuple(row[:4] for row in x0.moments[0]) == tuple(row[:4] for row in y0.moments[0])
    assert x0.moments[0][0][4:] == (2, 0, 0, 0, 0, 0)
    assert y0.moments[0][0][4:] == (0, 0, 0, 2, 0, 0)
    xf, yf = observe_checkpoint(axial_x[2]), observe_checkpoint(axial_y[2])
    assert tuple(row[0] for row in xf.moments[0]) != tuple(row[0] for row in yf.moments[0])


def test_fully_occupied_relation_support_invariant_exhausts_local_domain():
    nonblank = tuple(value for value in A9 if value != BLANK)
    assert len(nonblank) == 8
    for left, right, gate in product(nonblank, nonblank, (False, True)):
        assert all(value != BLANK for value in relation_tick(left, right, gate))


def test_blank_background_is_not_an_all_time_conservative_sector():
    lattice = state.blank(3)
    lattice.bank[0, H.channel(0, 0, 0)] = True
    st = staged.initialize(lattice)
    for _ in range(8):
        st, _ = staged.step(st)
    assert st.microtick == 8 and int(st.lattice.bank.sum()) == 1
    st, events = staged.step(st)
    assert len(events.absorptions) == 1
    assert int(st.lattice.bank.sum()) == 0
    assert staged.work_units(st) == 1


@pytest.mark.parametrize("order", (2, 3))
def test_separable_exact_taylor_budget_covers_seam_remainder(order):
    _, b, _ = _cycle(((0, 12), (1, 2)), origin=26)
    h = F(1, 3)
    bounds = (2, 4, 6) if order == 2 else (12, 24, 36)
    result = acoustic_taylor_bound(b, h=h, c_ref=2, derivative_bound=bounds, order=order)
    assert result.cycle_duration == F(1, 6)
    assert result.microtick_duration == F(1, 24)
    assert result.cell_volume == F(1, 27)
    for pol, velocity in ((0, 12), (1, 2)):
        point = tuple(F(q, 3) for q in G.coords(3, 26))
        v = H.VELOCITIES[velocity]
        actual = sum((i+1)*(_quartic(point[i]+h*v[i])-_quartic(point[i])) for i in range(3))
        taylor = sum((i+1)*h*v[i]*(2*point[i]-6*point[i]**2+4*point[i]**3) for i in range(3))
        if order == 3:
            taylor += sum((i+1)*(h*v[i])**2*(2-12*point[i]+12*point[i]**2)/2 for i in range(3))
        error = abs(actual-taylor)/27
        assert error <= result.weak_remainder[pol][0]
        assert result.rate_remainder[pol][0] == result.weak_remainder[pol][0]*6


@pytest.mark.parametrize("kwargs", ({"h":0}, {"c_ref":False}, {"derivative_bound":-1},
                                    {"derivative_bound":1.0}, {"order":True}))
def test_invalid_external_unit_or_error_contract_rejected(kwargs):
    _, b, _ = _cycle(((0, 0),))
    arguments = dict(h=F(1, 3), c_ref=1, derivative_bound=2)
    arguments.update(kwargs)
    with pytest.raises(ValueError):
        acoustic_taylor_bound(b, **arguments)


def test_float_test_function_and_wrong_clock_rejected():
    a, b, c = _cycle(((0, 0),))
    with pytest.raises(ValueError, match="exact int or Fraction"):
        certify_cycle(a, b, c, [0.0]*27)
    with pytest.raises(ValueError, match="stage ordinals"):
        certify_cycle(a, c, b, [0]*27)
