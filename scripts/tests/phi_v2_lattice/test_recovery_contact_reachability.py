"""Exact finite contact geometry and selected actual-law witnesses; no campaign."""
from itertools import product
import json
from math import gcd

import numpy as np
import pytest

from phi_v2_lattice import channels as C, geometry as G, staged as P, state as S
from phi_v2_lattice import recovery_contact_reachability as R
from phi_v2_lattice import recovery_kinetic_reference as K
from phi_v2_lattice._proofs import rotate


def free_oracle(c1, c2, r0, hops, L):
    """Independent one-hop iteration: no three-hop formula or quotient."""
    offset = tuple(x % L for x in r0)
    for _ in range(hops):
        offset = tuple((x + b - a) % L for x, a, b in zip(offset, C.tangent(c1), C.tangent(c2)))
        c1, c2 = C.U(c1), C.U(c2)
    return offset


def contact_oracle(c1, c2, L):
    """Walk the complete relative-position/internal-channel free orbit."""
    initial = (c1, c2, (0, 0, 0))
    current, reached = initial, set()
    for _ in range(12 * L):
        a, b, delta = current
        reached.add(tuple(-x % L for x in delta))
        after = tuple((x + db - da) % L for x, da, db in zip(delta, C.tangent(a), C.tangent(b)))
        current = (C.U(a), C.U(b), after)
        if current == initial:
            return frozenset(reached)
    raise AssertionError("independent free orbit did not close within 12L hops")


def test_certificate_checks_all_channels_without_loading_a_collision_table(monkeypatch):
    def forbidden():
        raise AssertionError("table loading is outside this certificate")
    monkeypatch.setattr(C, "load_collision_tables", forbidden)
    report = R.certificate()
    assert json.loads(json.dumps(report)) == report
    assert report["channels_checked"] == 384 and report["flags_checked"] == 48
    assert report["internal_period_hops"] == 12 and report["flag_and_tangent_period_hops"] == 3
    assert report["background_gate_checks"] == 16
    assert report["example_bounds"] == [
        dict(L=3, possible_initial_offsets=27, geometric_contact_offsets_upper=9, never_contact_offsets_lower=18),
        dict(L=4, possible_initial_offsets=64, geometric_contact_offsets_upper=6, never_contact_offsets_lower=58),
        dict(L=7, possible_initial_offsets=343, geometric_contact_offsets_upper=21, never_contact_offsets_lower=322),
        dict(L=8, possible_initial_offsets=512, geometric_contact_offsets_upper=12, never_contact_offsets_lower=500)]
    assert not report["contact_is_capture"] and not report["physical_binding_certified"]
    assert not report["continuum_recovery_certified"] and not report["complete_clock_period_claimed"]


@pytest.mark.parametrize("L", (3, 4, 5, 6, 7, 8))
def test_complete_free_orbit_oracle_even_odd_sizes_and_phase_polarity_cases(L):
    for c1, c2 in ((0, 0), (0, 1), (0, 5), (0, 32), (113, 94), (0, 192), (192, 193), (383, 254)):
        offsets = R.contact_offsets(c1, c2, L)
        assert offsets == contact_oracle(c1, c2, L)
        assert isinstance(offsets, frozenset)
        assert len(offsets) <= 3 * L // gcd(L, 2) < L ** 3
        for r0 in ((-1, L, 2 * L + 1), (L - 1, L - 1, L - 1), (0, 0, 0)):
            for hops in (0, 1, 2, 3, 11, 12, 3 * L + 2, 12 * L):
                assert R.free_relative_offset(c1, c2, r0, hops, L) == free_oracle(c1, c2, r0, hops, L)


def test_all_channel_periods_support_exact_huge_integer_hop_counts():
    for c1 in range(384):
        c2 = (c1 + 137) % 384
        for j in range(3):
            # Complete channel-and-position orbit divides 12L; no float conversion.
            hops = 12 * 7 * 10 ** 50 + j
            assert R.free_relative_offset(c1, c2, (-8, 0, 15), hops, 7) == free_oracle(c1, c2, (-8, 0, 15), j, 7)


def test_equal_flags_have_only_zero_contact_offset_for_every_phase_and_polarity():
    for c1 in range(384):
        flag, _ = C.STATES[c1 % 192]
        for phase in range(4):
            for offset in (0, 192):
                c2 = C.STATE_INDEX[(flag, phase)] + offset
                assert R.contact_offsets(c1, c2, 4) == frozenset({(0, 0, 0)})
                assert R.free_relative_offset(c1, c2, (1, -1, 2), 10 ** 30, 4) == (1, 3, 2)


def test_offsets_are_owned_immutable_and_do_not_depend_on_argument_order():
    first = R.contact_offsets(113, 94, 7)
    retained = frozenset(first)
    assert R.contact_offsets(94, 113, 7) == frozenset(tuple(-x % 7 for x in r) for r in first)
    assert R.contact_offsets(305, 286, 7) == first
    R.contact_offsets(0, 32, 4)
    assert first == retained
    with pytest.raises(AttributeError):
        first.add((1, 2, 3))
    initial = [-1, 8, 15]
    answer = R.free_relative_offset(0, 1, initial, 3, 7)
    initial[0] = 0
    assert answer == (6, 1, 1) and isinstance(answer, tuple)


def _actual_free_witness(L, c1, c2, r0, layer, background, ticks=56):
    origin = (L - 1, L - 1, L - 1)  # signed paths cross periodic seams
    sites = (G.site_index(L, *origin), G.site_index(L, *(origin[a] + r0[a] for a in range(3))))
    channels = (c1, c2)
    bank = np.zeros((L ** 3, 384), dtype=bool)
    for site, channel in zip(sites, channels):
        bank[site, channel] = True
    assert int(bank.sum()) == 2  # excludes duplicate same-site same-channel preparation
    initial = K.prepare_bank(bank, L, layer, background)
    original = P.checkpoint(initial)
    state = initial
    gate_sc = np.zeros((L ** 3, 3), dtype=bool)
    gate_fcc = np.zeros((L ** 3, 3, 2), dtype=bool)
    for elapsed in range(1, ticks + 1):
        if state.phase == 0:
            for owner in range(L ** 3):
                for axis in range(3):
                    ends = G.sc_endpoints(L, owner, axis)
                    gate_sc[owner, axis] = sum(site in ends for site in sites) % 2 == 0
                for plane, diagonal in product(range(3), range(2)):
                    ends = G.fcc_endpoints(L, owner, plane, diagonal)
                    gate_fcc[owner, plane, diagonal] = sum(site in ends for site in sites) % 2 == 0
        elif state.phase == 1:
            layer = (layer - 1) % 3
            background = S.idx_of(rotate(S.z_of(background)))
        elif state.phase == 2:
            sites = tuple(G.shift(L, x, C.tangent(c)) for x, c in zip(sites, channels))
            channels = tuple(C.U(c) for c in channels)
        else:
            gate_sc.fill(False)
            gate_fcc.fill(False)
        state, events = P.step(state)
        assert all(not values for values in vars(events).values())
        actual = {tuple(map(int, row)) for row in np.argwhere(state.lattice.bank)}
        assert actual == set(zip(sites, channels))
        assert not state.lattice.s.any() and np.all(state.lattice.ell == layer)
        assert np.all(state.lattice.sc == background) and np.all(state.lattice.fcc == background)
        assert not state.admitted_sc.any()
        assert np.array_equal(state.gate_sc, gate_sc) and np.array_equal(state.gate_fcc, gate_fcc)
        assert state.microtick == elapsed and P.work_units(state) == 18 * L ** 3 + 2
        first, second = (G.coords(L, site) for site in sites)
        measured = tuple((b - a) % L for a, b in zip(first, second))
        assert measured == R.free_relative_offset(c1, c2, r0, (elapsed + 1) // 4, L)
    assert P.checkpoint(initial) == original


@pytest.mark.parametrize("index", range(8))
def test_selected_actual_56_microtick_noncontact_witnesses_all_background_codes(index):
    # Eight declared finite witnesses, not exhaustive global-state evolution.
    L = (3, 4)[index % 2]
    c1, c2 = ((0, 1), (0, 0), (0, 5), (113, 94), (192, 193), (192, 197), (0, 224), (305, 286))[index]
    contact = contact_oracle(c1, c2, L)
    r0 = next(r for r in product(range(L), repeat=3) if r not in contact)
    assert r0 not in R.contact_offsets(c1, c2, L)
    codes = [code for code in range(9) if code != S.BLANK_IDX]
    _actual_free_witness(L, c1, c2, r0, index % 3, codes[index])


def test_actual_opposite_polarity_contact_does_not_create_an_interaction():
    assert (0, 0, 0) in R.contact_offsets(0, 192, 3)
    _actual_free_witness(3, 0, 192, (0, 0, 0), 0, next(code for code in range(9) if code != S.BLANK_IDX))


def test_actual_geometric_first_contact_stops_before_the_collision_prediction_boundary():
    # Registered separated witness: first contact is tick3, collision follows tick6.
    L = 7
    bank = np.zeros((L ** 3, 384), dtype=bool)
    bank[G.site_index(L, 3, 2, 3), 113] = True
    bank[G.site_index(L, 3, 4, 3), 94] = True
    state = K.prepare_bank(bank, L, layer=2)
    assert (0, 2, 0) in R.contact_offsets(113, 94, L)
    for elapsed in range(1, 4):
        state, events = P.step(state)
        assert not events.collisions and not events.absorptions
        relative = R.free_relative_offset(113, 94, (0, 2, 0), (elapsed + 1) // 4, L)
        assert (relative == (0, 0, 0)) == (elapsed == 3)
    assert set(map(int, np.nonzero(state.lattice.bank)[0])) == {G.site_index(L, 3, 3, 3)}
    # No use of the free formula to predict post-contact same-polarity dynamics.


@pytest.mark.parametrize("args", ((True, 0, 3), (-1, 0, 3), (384, 0, 3), (0, 1.5, 3),
                                  (0, 1, True), (0, 1, 2), (0, 1, 3.5)))
def test_invalid_channels_or_sizes_rejected(args):
    with pytest.raises(ValueError):
        R.contact_offsets(*args)
    with pytest.raises(ValueError):
        R.free_relative_offset(args[0], args[1], (0, 0, 0), 0, args[2])


@pytest.mark.parametrize("offset,hops", ((None, 0), ((1, 2), 0), ((1, 2, 3, 4), 0),
                                          ((True, 0, 0), 0), ((0.5, 0, 0), 0),
                                          ((0, 0, 0), -1), ((0, 0, 0), True), ((0, 0, 0), 1.5)))
def test_invalid_offset_or_hops_rejected(offset, hops):
    with pytest.raises(ValueError):
        R.free_relative_offset(0, 1, offset, hops, 3)


def test_corrupted_internal_period_is_rejected(monkeypatch):
    monkeypatch.setattr(C, "U", lambda c: c)
    with pytest.raises(ValueError, match="period"):
        R.certificate()


def test_corrupted_routing_is_rejected(monkeypatch):
    monkeypatch.setattr(C, "tangent", lambda c: (1, 0, 0))
    with pytest.raises(ValueError, match="distinct axes"):
        R.certificate()
