"""Finite proof ingredients plus bounded actual-law formation regressions."""
from dataclasses import FrozenInstanceError

import numpy as np
import pytest

from phi_v2_lattice import channels as C, geometry as G, staged as P, state as S
from phi_v2_lattice import recovery_kinetic_reference as K
from phi_v2_lattice import recovery_pair_formation as F


def prepare(first, second, layer=0, polarity=1):
    bank = np.zeros((27, C.N_CHANNELS), dtype=bool)
    offset = C.N_STATES if polarity < 0 else 0
    bank[first[0], first[1] + offset] = True
    bank[second[0], second[1] + offset] = True
    return K.prepare_bank(bank, 3, layer)


def test_exhaustive_local_preimage_certificate():
    certificate = F.certify_local_preimages()
    assert certificate.law_id == P.LAW_ID
    assert certificate.collision_hash == C.COLLISION_HASH
    assert certificate.collision_layers == 3
    assert certificate.collision_rows_checked == 55008
    assert certificate.field_channels_checked == 384
    assert certificate.flags_checked == 48
    assert not certificate.physical_recovery_certified


@pytest.mark.parametrize("layer", range(3))
@pytest.mark.parametrize("polarity", (1, -1))
@pytest.mark.parametrize("first,second,member", [
    ((26, 0), (26, 1), True),       # recurrent pair at a periodic seam
    ((26, 0), (0, 1), False),       # separated, same flag
    ((26, 0), (26, 32), False),     # unequal-flag actual scattering
    ((26, 0), (0, 0), False),       # equal channels in distinct slots are legal
])
def test_actual_microticks_preserve_membership_and_complete_input(first, second, member, layer, polarity):
    initial = prepare(first, second, layer, polarity)
    checkpoint = P.checkpoint(initial)
    result = F.audit_pair_membership(initial, 16)
    assert P.checkpoint(initial) == checkpoint
    assert len(result.observations) == 17
    assert tuple(o.microtick for o in result.observations) == tuple(range(17))
    assert {o.colocated_same_flag for o in result.observations} == {member}
    assert result.work_units == 18 * 27 + 2
    assert not result.physical_recovery_certified
    if member:
        assert result.collision_events == 4


@pytest.mark.parametrize("offset", ((1, 0, 0), (0, -1, 0), (1, 1, 1)))
def test_separated_same_flag_fixture_retains_exact_torus_displacement(offset):
    site = G.site_index(3, 2, 2, 2)
    second = G.shift(3, site, offset)
    state = prepare((site, 0), (second, 1))
    result = F.audit_pair_membership(state, 48)
    assert result.collision_events == 0
    # Individual labels are unnecessary: these free channels remain distinct.
    channels = (0, 1)
    for observation in result.observations:
        if observation.microtick % 4 == 3:
            channels = tuple(C.U(c) for c in channels)
        locations = {c: x for x, c in observation.slots}
        assert locations[channels[1]] == G.shift(3, locations[channels[0]], offset)


@pytest.mark.parametrize("bad", (True, -1, 1.5))
def test_invalid_horizon_rejected(bad):
    with pytest.raises(ValueError, match="nonnegative integer"):
        F.audit_pair_membership(prepare((0, 0), (0, 1)), bad)


@pytest.mark.parametrize("change", ("empty", "third_token", "opposite_polarity", "nonuniform_layer",
                                    "blank_relation", "manifestation", "intermediate_phase"))
def test_out_of_domain_preparations_rejected(change):
    state = prepare((0, 0), (0, 1))
    if change == "empty":
        state.lattice.bank.fill(False)
    elif change == "third_token":
        state.lattice.bank[1, 0] = True
    elif change == "opposite_polarity":
        state.lattice.bank[0, 1] = False
        state.lattice.bank[0, 193] = True
    elif change == "nonuniform_layer":
        state.lattice.ell[1] = 1
    elif change == "blank_relation":
        state.lattice.sc[1, 0, 0] = S.BLANK_IDX
    elif change == "manifestation":
        state.lattice.s[0] = 1
    else:
        state, _ = P.step(state)
    with pytest.raises(ValueError):
        F.audit_pair_membership(state, 1)


def test_injected_entry_into_sector_is_detected(monkeypatch):
    initial = prepare((0, 0), (1, 1))
    real_step = P.step

    def broken_step(state):
        after, events = real_step(state)
        after.lattice.bank[1, 1] = False
        after.lattice.bank[0, 1] = True
        return after, events

    monkeypatch.setattr(P, "step", broken_step)
    with pytest.raises(ValueError, match="preimage obstruction violated"):
        F.audit_pair_membership(initial, 1)


def test_observations_are_immutable_and_do_not_certify_physics():
    result = F.audit_pair_membership(prepare((0, 0), (0, 1)), 0)
    with pytest.raises(FrozenInstanceError):
        result.observations[0].colocated_same_flag = False
    assert not result.physical_recovery_certified


def test_skipped_clock_interval_is_rejected(monkeypatch):
    real_step = P.step

    def broken_step(state):
        after, events = real_step(state)
        after.microtick += 4
        return after, events

    monkeypatch.setattr(P, "step", broken_step)
    with pytest.raises(ValueError, match="physical microtick"):
        F.audit_pair_membership(prepare((0, 0), (0, 1)), 1)


def test_corrupt_table_identity_rejected(monkeypatch):
    original = C.load_collision_tables()
    corrupted = [dict(table) for table in original]
    corrupted[0][(0, 1)] = (0, 2)
    monkeypatch.setattr(C, "load_collision_tables", lambda: tuple(corrupted))
    with pytest.raises(ValueError, match="frozen candidate"):
        F.certify_local_preimages()
