"""Structural inventory obstructions and explicit limits to the no-go scope."""
from itertools import product

import numpy as np
import pytest

from phi_v2_lattice import channels as C, geometry as G, prepare as P
from phi_v2_lattice import recovery_obstructions as R, staged as V, state as S, tick as T
from phi_v2_lattice._proofs import encode, readout, relation_tick


@pytest.fixture(scope="module")
def tables():
    return C.load_collision_tables()


def _polarity_count(pair):
    result = [0, 0]
    for z in pair:
        if readout(z)[0]:
            result[int(readout(z)[1] < 0)] += 1
    return tuple(result)


def test_exhaustive_relation_map_never_changes_anchor_polarity_inventory():
    checked = 0
    for primary, reserve, gate in product(S.A9_LIST, S.A9_LIST, (False, True)):
        before = primary, reserve
        after = relation_tick(*before, even_gate=gate)
        assert _polarity_count(before) == _polarity_count(after)
        checked += 1
    assert checked == 162


def test_exhaustive_collision_maps_preserve_carrier_cardinality(tables):
    for table in tables:
        assert len(table) == C.N_STATES * (C.N_STATES - 1) // 2
        for before, after in table.items():
            assert len(set(before)) == len(set(after)) == 2
            assert all(0 <= channel < C.N_STATES for channel in before + after)


@pytest.mark.parametrize("preparation", ["sparse_positive", "sparse_negative", "r5", "blank", "relations"])
def test_every_stage_obeys_anchor_and_polarity_balances(tables, preparation):
    if preparation.startswith("sparse"):
        lattice = P.sparse_material(4, seed=11, n_tokens=20, field_occupation=.025,
                                    eps=-1 if preparation.endswith("negative") else 1)
    elif preparation == "r5":
        lattice = P.r5_vacuum(3, seed=4)
    elif preparation == "relations":
        lattice = S.blank(3)
        lattice.sc[:] = S.idx_of(encode(0, 1))
        lattice.fcc[:] = S.idx_of(encode(2, -1))
    else:
        lattice = S.blank(3)
    state = V.initialize(lattice)
    initial = R.inventory(state)
    absorbed = 0
    for _ in range(16):
        after, events = V.step(state, tables)
        balance = R.assert_anchor_obstructions(state, after)
        assert balance.absorbed_tokens == len(events.absorptions)
        assert balance.transfer_residual == (0, 0)
        absorbed += balance.absorbed_tokens
        state = after
    final = R.inventory(state)
    assert absorbed == initial.field_count - final.field_count
    assert 0 <= absorbed <= initial.field_count
    assert final.relation_count - initial.relation_count == absorbed


def test_an_sc_anchor_can_absorb_only_once_and_never_emit(tables):
    lattice = S.blank(4)
    channel = next(c for c in range(384) if C.phase(c) == 2 and C.tangent(c) == (1, 0, 0))
    lattice.bank[0, channel] = True
    state = V.initialize(lattice)
    after, events = V.step(state, tables)
    assert len(events.absorptions) == 1
    assert R.assert_anchor_obstructions(state, after).created_sc_anchors == (0,)
    state = after
    assert R.field_free(state)
    anchor = R.inventory(state).sc_by_polarity[0]
    for _ in range(35):
        after, _ = V.step(state, tables)
        R.assert_anchor_obstructions(state, after)
        assert R.field_free(after)
        assert R.inventory(after).sc_by_polarity[0] == anchor
        state = after
    # A fresh preparation with a filled edge denies another absorption there.
    occupied = S.copy(state.lattice)
    occupied.bank[0, channel] = True
    start = V.initialize(occupied)
    end, events = V.step(start, tables)
    assert not any(owner == 0 and axis == 0 for _, _, owner, axis in events.absorptions)
    assert R.inventory(end).sc_by_polarity[0] == anchor


def test_field_empty_relation_sector_has_no_interanchor_signal(tables):
    a = S.blank(5)
    anchor = G.site_index(5, 2, 2, 2)
    a.sc[anchor, 0, 1] = S.idx_of(encode(0, 1))
    b = S.copy(a)
    b.sc[anchor, 0, 1] = S.idx_of(encode(1, 1))
    first, second = V.initialize(a), V.initialize(b)
    endpoints = set(G.sc_endpoints(5, anchor, 0))
    for _ in range(32):
        first, _ = V.step(first, tables)
        second, _ = V.step(second, tables)
        assert R.field_free(first) and R.field_free(second)
        changed_sc = np.argwhere(first.lattice.sc != second.lattice.sc)
        assert all(tuple(index[:2]) == (anchor, 0) for index in changed_sc)
        assert np.array_equal(first.lattice.fcc, second.lattice.fcc)
        assert set(np.flatnonzero(first.lattice.s != second.lattice.s)).issubset(endpoints)
        assert np.array_equal(first.gate_sc, second.gate_sc)
        assert np.array_equal(first.gate_fcc, second.gate_fcc)


def test_all_streaming_channels_keep_current_displacement_under_manifestation():
    for channel in range(C.N_CHANNELS):
        clear_hop, clear_output = R.streaming_direction_witness(channel, False)
        occupied_hop, occupied_output = R.streaming_direction_witness(channel, True)
        assert clear_hop == occupied_hop == C.tangent(channel)
        assert occupied_output == C.half_turn(clear_output)
        assert C.phase(occupied_output) == (C.phase(clear_output) + 2) % 4
        assert C.tangent(occupied_output) == C.tangent(clear_output)


def test_collisions_can_change_displacement_multisets_so_no_global_force_no_go(tables):
    row = np.zeros(C.N_CHANNELS, dtype=bool)
    row[[0, 32]] = True
    after, events = T.collide_row(row, 0, tables)
    assert tuple(np.flatnonzero(after)) == (144, 176)
    before_hops = sorted(C.tangent(c) for c in (0, 32))
    after_hops = sorted(C.tangent(c) for c in (144, 176))
    assert before_hops != after_hops
    assert len(events) == 1


def test_quantitative_predicates_detect_forbidden_emission_and_anchor_migration(tables):
    before = V.initialize(P.isolated_relation(4, owner=0))
    after, _ = V.step(before, tables)
    after.lattice.sc[0] = S.BLANK_IDX
    after.lattice.bank[0, 0] = True
    with pytest.raises(ValueError, match="anchor obstruction"):
        R.assert_anchor_obstructions(before, after)
    after, _ = V.step(before, tables)
    after.lattice.sc[1] = after.lattice.sc[0]
    after.lattice.sc[0] = S.BLANK_IDX
    with pytest.raises(ValueError, match="anchor obstruction"):
        R.assert_anchor_obstructions(before, after)


def test_adjacent_tick_and_same_lattice_requirements(tables):
    before = V.initialize(S.blank(3))
    after, _ = V.step(before, tables)
    after.microtick += 1
    with pytest.raises(ValueError, match="adjacent"):
        R.transition_balance(before, after)
