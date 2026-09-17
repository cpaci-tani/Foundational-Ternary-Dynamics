"""Initial-record recipes: exact defaults, geometry, labels and rejection gates."""
from copy import deepcopy
from hashlib import sha256
import numpy as np
import pytest

from phi_v2_lattice import web_seeding as B, web_scenarios as W, native_codec as N, state as S
from phi_v2_lattice.geometry import site_index
from phi_v2_lattice._proofs import encode


def recipe(size=3, components=()):
    return dict(version=1, scenarioId='record-empty', size=size, blank=True,
                randomSeed=0, components=list(components))


def component(kind='field', **parameters):
    defaults = dict(field=dict(channel=0, occupied=True, density=1),
                    relation=dict(orientation=0, slot=0, phase=0, polarity=1, occupied=True),
                    manifestation=dict(value=0), collision=dict(value=0))
    return dict(id=kind, kind=kind, enabled=True,
                region=dict(shape='point', x=0, y=1, z=2, dx=1, dy=1, dz=1, radius=1),
                parameters={**defaults[kind], **parameters})


@pytest.mark.parametrize('row', W.scenarios(), ids=lambda r: r.id)
def test_every_registered_variant_is_byte_identical_without_overrides(row):
    for size in row.sizes:
        r = recipe(size)
        r.update(scenarioId='record-' + row.id, blank=False)
        data, digest, _ = B.checkpoint(r)
        assert (data, digest) == W.checkpoint(row.id, size)
        assert N.decode(data).microtick == 0


def test_all_initial_record_types_and_coordinate_order_are_exact():
    components = [component('field', channel=383), component('relation', orientation=8, slot=1, phase=3, polarity=-1),
                  component('manifestation', value=-1), component('collision', value=2)]
    r = recipe(3, components)
    before = deepcopy(r)
    result = B.prepare(r)
    i = site_index(3, 0, 1, 2)
    assert i == 5 and np.flatnonzero(result.lattice.bank.any(axis=1)).tolist() == [5]
    assert result.lattice.bank[i, 383] and result.lattice.bank.sum() == 1
    assert result.lattice.fcc[i, 2, 1, 1] == S.idx_of(encode(3, -1))
    assert np.count_nonzero(result.lattice.fcc != S.BLANK_IDX) == 1
    assert result.lattice.s[i] == -1 and result.lattice.ell[i] == 2
    assert result.microtick == 0 and r == before


@pytest.mark.parametrize('orientation', range(9))
@pytest.mark.parametrize('slot', range(2))
def test_every_relation_orientation_and_slot_preserves_phase_and_polarity(orientation, slot):
    c = component('relation', orientation=orientation, slot=slot, phase=2, polarity=-1)
    state = B.prepare(recipe(components=[c])).lattice
    if orientation < 3:
        assert state.sc[5, orientation, slot] == S.idx_of(encode(2, -1))
    else:
        p, q = divmod(orientation - 3, 2)
        assert state.fcc[5, p, q, slot] == S.idx_of(encode(2, -1))
    assert np.count_nonzero(state.sc != S.BLANK_IDX) + np.count_nonzero(state.fcc != S.BLANK_IDX) == 1


@pytest.mark.parametrize('shape,origin,extent,radius,count', [
    ('all', (0, 0, 0), (1, 1, 1), 1, 27),
    ('box', (1, 0, 0), (2, 3, 1), 1, 6),
    ('sphere', (1, 1, 1), (1, 1, 1), 1, 7),
    ('sphere', (0, 0, 0), (1, 1, 1), 1, 4),
    ('sphere', (0, 0, 0), (1, 1, 1), 0, 1),
])
def test_masks_have_independent_geometric_counts(shape, origin, extent, radius, count):
    c = component()
    c['region'].update(shape=shape, **dict(zip(('x', 'y', 'z'), origin)),
                       **dict(zip(('dx', 'dy', 'dz'), extent)), radius=radius)
    assert B.prepare(recipe(components=[c])).lattice.bank.sum() == count


def test_overlap_order_set_clear_and_disabled_are_explicit():
    on = component()
    off = deepcopy(on)
    off.update(id='clear', parameters={**off['parameters'], 'occupied': False})
    assert not B.prepare(recipe(components=[on, off])).lattice.bank.any()
    assert B.prepare(recipe(components=[off, on])).lattice.bank.sum() == 1
    off['enabled'] = False
    assert B.prepare(recipe(components=[on, off])).lattice.bank.sum() == 1


def test_stochastic_component_is_repeatable_and_unrelated_streams_do_not_reroll_it():
    field = component('field', density=0.4)
    field['region']['shape'] = 'all'
    r = recipe(9, [field])
    data, digest, recipe_digest = B.checkpoint(r)
    assert B.checkpoint(deepcopy(r)) == (data, digest, recipe_digest)
    assert sha256(data).hexdigest() == digest
    baseline = B.prepare(r).lattice.bank.copy()
    r['components'].append(component('manifestation', value=1))
    assert np.array_equal(B.prepare(r).lattice.bank, baseline)
    r['randomSeed'] = 1
    assert not np.array_equal(B.prepare(r).lattice.bank, baseline)
    r['components'][0]['parameters']['density'] = 0
    assert not B.prepare(r).lattice.bank.any()


@pytest.mark.parametrize('mutate', [
    lambda r: r.update(version=3), lambda r: r.update(size=True), lambda r: r.update(size=18),
    lambda r: r.update(randomSeed=-1), lambda r: r.update(scenarioId='record-missing'),
    lambda r: r.update(unknown=1), lambda r: r.update(blank='true'),
    lambda r: r.update(components=[component()] * 65),
    lambda r: r.update(components=[component(), component()]),
    lambda r: r['components'][0]['region'].update(x=-1),
    lambda r: r['components'][0]['region'].update(shape='box', dx=3, x=1),
    lambda r: r['components'][0]['region'].update(radius=10**500),
    lambda r: r['components'][0]['parameters'].update(channel=384),
    lambda r: r['components'][0]['parameters'].update(density=float('nan')),
    lambda r: r['components'][0]['parameters'].update(occupied=1),
    lambda r: r['components'][0].update(enabled=False, parameters={'channel': -1, 'occupied': True, 'density': 1}),
])
def test_invalid_requests_fail_before_a_checkpoint_exists(mutate):
    r = recipe(components=[component()])
    mutate(r)
    with pytest.raises(ValueError):
        B.checkpoint(r)


def test_blank_parent_can_use_new_size_but_registered_defaults_cannot():
    r = recipe(17)
    B.checkpoint(r)
    r['blank'] = False
    with pytest.raises(ValueError):
        B.checkpoint(r)
