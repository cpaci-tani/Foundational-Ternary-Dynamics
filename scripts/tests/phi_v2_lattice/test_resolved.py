"""Exact observation and current identities, not autonomous closure tests."""
from dataclasses import FrozenInstanceError, replace

import numpy as np
import pytest

from phi_v2_lattice import channels as C, coarse as B, conservation as K
from phi_v2_lattice import geometry as G, prepare as P, resolved as R, state as S, tick as T
from phi_v2_lattice._proofs import encode


@pytest.fixture(scope="module")
def tables():
    return C.load_collision_tables()


def test_field_basis_all_layers_polarities_and_channels():
    st = S.blank(2)
    for layer in range(3):
        st.ell[layer] = layer
        st.bank[layer, :] = True
    observed = R.restrict(st, 2)
    assert observed.field_channels.shape == (1, 3, 2, 192)
    assert set(observed.field_channels.values) == {1}
    assert sum(observed.counts.field_tokens) == 3 * 384
    assert observed.field_moments.values == K.layer_sum(st)
    assert observed.site_layers.values == (6, 1, 1)


@pytest.mark.parametrize("layer,polarity", [(l, p) for l in range(3) for p in (1, -1)])
def test_individual_channel_moments_use_canonical_map(layer, polarity):
    st = S.blank(2)
    st.ell[0] = layer
    for i in range(C.N_STATES):
        st.bank[0] = False
        channel = C.channel(i, polarity)
        st.bank[0, channel] = True
        observed = R.restrict(st, 2)
        assert observed.field_moments.values == C.layer_value_of(channel, layer)
        histogram = observed.field_channels.array()
        assert histogram[0, layer, int(polarity < 0), i] == 1
        assert sum(observed.field_channels.values) == 1


def test_relation_basis_all_orientations_slots_polarities_phases():
    st = S.blank(4)
    for slot in range(2):
        for pol_index, polarity in enumerate((1, -1)):
            for phase in range(4):
                owner = slot * 8 + pol_index * 4 + phase
                for key in R.RELATION_ORIENTATIONS:
                    getattr(st, key[0])[(owner,) + key[1:] + (slot,)] = S.idx_of(encode(phase, polarity))
    observed = R.restrict(st, 4)
    assert observed.relation_channels.shape == (1, 9, 2, 2, 4)
    assert set(observed.relation_channels.values) == {1}
    assert sum(observed.counts.relation_tokens) == 9 * 16
    assert set(observed.blank_relation_slots.values) == {64 - 8}


def test_incidence_and_stored_manifestation_are_distinct():
    st = S.blank(4)
    for axis in range(3):
        st.sc[0, axis, 0] = S.idx_of(encode(0, 1))
    observed = R.restrict(st, 1)
    assert observed.counts.incidence[0] == 3
    assert observed.counts.manifestation_counts[0] == (0, 1, 0)


def test_resolved_phase_separates_existing_count_closure_counterexample(tables):
    owner = G.site_index(4, 1, 0, 0)
    a, b = (P.isolated_relation(4, owner=owner, phase=phase) for phase in (0, 1))
    assert B.restrict(a, 2) == B.restrict(b, 2)
    assert R.restrict(a, 2).relation_channels != R.restrict(b, 2).relation_channels
    next_a, _ = T.tick(a, tables)
    next_b, _ = T.tick(b, tables)
    assert R.restrict(next_a, 2).counts.incidence != R.restrict(next_b, 2).counts.incidence
    assert "unestablished" in R.restrict(a, 2).status


def test_resolved_histograms_still_do_not_close_dynamics(tables):
    """Identical phase-resolved block data hide the relation's spatial location."""
    a = P.isolated_relation(4, owner=G.site_index(4, 0, 0, 0), phase=0)
    b = P.isolated_relation(4, owner=G.site_index(4, 1, 0, 0), phase=0)
    assert R.restrict(a, 2) == R.restrict(b, 2)
    next_a, _ = T.tick(a, tables)
    next_b, _ = T.tick(b, tables)
    assert R.restrict(next_a, 2).counts.incidence != R.restrict(next_b, 2).counts.incidence


def test_staged_histograms_retain_spatial_closure_obstruction(tables):
    from phi_v2_lattice import staged as V

    a, b = (V.initialize(P.isolated_relation(4, owner=G.site_index(4, x, 0, 0), phase=0))
            for x in (0, 1))
    assert R.restrict(a, 2) == R.restrict(b, 2)
    for _ in range(4):
        a, _ = V.step(a, tables)
        b, _ = V.step(b, tables)
    assert R.restrict(a, 2).counts.incidence != R.restrict(b, 2).counts.incidence


def test_nested_restriction_and_immutable_independent_storage():
    st = P.sparse_material(4, seed=5, n_tokens=12, field_occupation=.02)
    original = S.copy(st)
    fine = R.restrict(st, 1)
    assert R.merge(fine, 2) == R.restrict(st, 2)
    assert R.merge(R.merge(fine, 2), 2) == R.restrict(st, 4)
    with pytest.raises(FrozenInstanceError):
        fine.phase = 3
    with pytest.raises(TypeError):
        fine.field_channels.values[0] = 1
    detached = fine.field_channels.array()
    detached[:] = 99
    assert 99 not in fine.field_channels.values
    for name in ("s", "ell", "bank", "sc", "fcc"):
        np.testing.assert_array_equal(getattr(st, name), getattr(original, name))
    st.bank[:] = True
    st.sc[:] = S.BLANK_IDX
    assert fine == R.restrict(original, 1)


@pytest.mark.parametrize("kind,owner,indices", [
    ("sc", (1, 0, 0), (0,)),
    ("sc", (3, 0, 0), (0,)),
    ("fcc", (1, 1, 0), (2, 0)),
    ("fcc", (1, 1, 0), (2, 1)),
    ("fcc", (3, 3, 0), (2, 0)),
    ("fcc", (3, 3, 0), (2, 1)),
])
def test_live_sc_fcc_seam_transfers_and_time_composition(tables, kind, owner, indices):
    st = S.blank(4)
    site = G.site_index(4, *owner)
    getattr(st, kind)[(site,) + indices + (1,)] = S.idx_of(encode(0, 1))
    windows = []
    for tick in range(8):
        after, events = T.tick(st, tables)
        window = R.boundary_transfers(st, after, events, 2, start_tick=tick)
        assert not any(window.residual())
        if tick == 0:
            assert window.edges
            assert any(R.boundary_transfers(st, after, T.TickEvents(), 2, start_tick=tick).residual())
        fine = R.boundary_transfers(st, after, events, 1, start_tick=tick)
        assert R.merge_transfers(fine, 2) == window
        assert R.merge_transfers(window, 2).edges == ()
        events.crossings.clear()
        assert not any(window.residual())
        windows.append(window)
        st = after
    total = R.compose_transfers(*windows)
    assert total.start_tick == 0 and total.stop_tick == 8
    assert not any(total.residual())
    assert total == R.compose_transfers(R.compose_transfers(*windows[:3]),
                                        R.compose_transfers(*windows[3:]))
    assert R.merge_transfers(total, 2) == R.compose_transfers(
        *(R.merge_transfers(w, 2) for w in windows))
    with pytest.raises(ValueError, match="contiguous"):
        R.compose_transfers(windows[0], windows[2])
    with pytest.raises(ValueError, match="contiguous"):
        R.compose_transfers(windows[0], windows[0])
    broken = replace(windows[1], incidence_before=(99,) + windows[1].incidence_before[1:])
    with pytest.raises(ValueError, match="endpoint"):
        R.compose_transfers(windows[0], broken)


def test_transfers_reject_invalid_clock_and_partition(tables):
    st = S.blank(2)
    after, events = T.tick(st, tables)
    for start in (None, -1, True, 0.5):
        with pytest.raises(ValueError):
            R.boundary_transfers(st, after, events, 1, start_tick=start)
    with pytest.raises(ValueError):
        R.compose_transfers()
    first = R.boundary_transfers(st, after, events, 1, start_tick=0)
    second = R.boundary_transfers(st, after, events, 2, start_tick=1)
    with pytest.raises(ValueError, match="partitions"):
        R.compose_transfers(first, second)
    with pytest.raises(ValueError, match="clocks"):
        R.compose_transfers(first, replace(first, clock="staged_microtick"))


def test_merges_use_arbitrary_precision_integers():
    observed = R.restrict(S.blank(2), 1)
    large = 2 ** 70
    tensor = R.IntTensor((8, 6), (large,) * 48)
    altered = replace(observed, field_moments=tensor)
    assert R.merge(altered, 2).field_moments.values == (8 * large,) * 6


def test_live_staged_pending_records_and_every_microtick_balance(tables):
    from phi_v2_lattice import staged as V

    lattice = S.blank(4)
    channel = next(c for c in range(384) if C.phase(c) == 2 and C.tangent(c) == (1, 0, 0))
    lattice.bank[0, channel] = True
    for kind, owner, indices in (
            ("sc", (1, 0, 0), (0,)),
            ("sc", (3, 0, 0), (0,)),
            ("fcc", (1, 1, 0), (2, 0)),
            ("fcc", (1, 1, 0), (2, 1))):
        site = G.site_index(4, *owner)
        getattr(lattice, kind)[(site,) + indices + (1,)] = S.idx_of(encode(0, 1))
    st = V.initialize(lattice)
    work = V.work_units(st)
    windows, observations = [], []
    saw_absorption = saw_crossing = False
    for tick in range(8):
        observed = R.restrict(st, 2)
        observations.append(observed)
        assert observed.microtick == tick and observed.phase == tick % 4
        assert R.merge(R.restrict(st, 1), 2) == observed
        assert sum(observed.counts.field_tokens) + sum(observed.counts.relation_tokens) == work
        assert len(observed.pending) == 3
        for pending in observed.pending:
            assert pending.data == R.IntTensor.freeze(getattr(st, pending.name))
            assert pending.data.shape[0] == 4 ** 3
            assert "owner" in pending.ownership
        after, events = V.step(st, tables)
        saw_absorption |= bool(events.absorptions)
        saw_crossing |= bool(events.crossings)
        window = R.boundary_transfers(st, after, events, 2)
        assert window.clock == "staged_microtick" and window.start_tick == tick
        assert not any(window.residual())
        assert R.merge_transfers(R.boundary_transfers(st, after, events, 1), 2) == window
        windows.append(window)
        st = after
    assert saw_absorption and saw_crossing
    assert any(observations[1].pending[0].data.values)
    assert not any(observations[0].pending[0].data.values)
    assert not any(observations[4].pending[0].data.values)
    assert R.compose_transfers(*windows).stop_tick == 8
    assert not any(R.compose_transfers(*windows).residual())
    st.gate_sc[:] = True
    assert observations[4].pending[1].data.values == (0,) * (4 ** 3 * 3)


def test_staged_transfer_clock_cannot_skip_microticks(tables):
    from phi_v2_lattice import staged as V

    before = V.initialize(S.blank(3))
    after, events = V.step(before, tables)
    later, _ = V.step(after, tables)
    with pytest.raises(ValueError, match="exactly one"):
        R.boundary_transfers(before, later, events, 1)
    with pytest.raises(ValueError, match="match staged"):
        R.boundary_transfers(before, after, events, 1, start_tick=5)
    with pytest.raises(ValueError, match="mix reference"):
        R.boundary_transfers(before.lattice, after, events, 1)


def test_duplicate_crossing_log_is_rejected_before_legacy_overwrite(tables):
    before = P.isolated_relation(4, owner=16, phase=0)
    after, events = T.tick(before, tables)
    assert events.crossings == [("sc", 16, (0,), -1)]
    events.crossings *= 2
    with pytest.raises(ValueError, match="duplicate"):
        R.boundary_transfers(before, after, events, 2, start_tick=0)


@pytest.mark.parametrize("row", [
    ("unknown", 16, (0,), -1), ("sc", -1, (0,), -1),
    ("sc", 64, (0,), -1), ("sc", True, (0,), -1),
    ("sc", 16, (), -1), ("sc", 16, (3,), -1),
    ("sc", 16, (False,), -1), ("fcc", 16, (0, 2), -1),
    ("fcc", 16, (0,), -1), ("sc", 16, (0,), 0),
    ("sc", 16, (0,), True), ("sc", 16, (0,), 1),
    ("sc", 15, (0,), -1), ("sc", 16, (0,)),
])
def test_crossing_log_rejects_invalid_alphabet_or_source(tables, row):
    before = P.isolated_relation(4, owner=16, phase=0)
    after, _ = T.tick(before, tables)
    with pytest.raises(ValueError):
        R.boundary_transfers(before, after, T.TickEvents(crossings=[row]), 2, start_tick=0)


def test_crossing_log_checks_final_polarity_and_initial_phase(tables):
    before = P.isolated_relation(4, owner=16, phase=0)
    after, events = T.tick(before, tables)
    after.sc[16, 0, 0] = S.idx_of(encode(1, -1))
    with pytest.raises(ValueError, match="final slots"):
        R.boundary_transfers(before, after, events, 2, start_tick=0)
    before = P.isolated_relation(4, owner=16, phase=1)
    after, _ = T.tick(before, tables)
    with pytest.raises(ValueError, match="source phase"):
        R.boundary_transfers(before, after, events, 2, start_tick=0)
