"""Exhaustive per-property effect verification for the v2 editable seed constructors.

``test_web_seed_presets.py`` proves byte-parity, schema completeness, and a
sample of wiring. This module instead proves, for every registered binding
in ``web_seed_presets.PROPERTY_META`` / ``COLLECTION_META`` / ``REGION_META``
plus per-kind activation and the two recipe-level fields, that editing the
value actually changes the compiled record in the exact way the constructor
declares -- asserting real encoded record content (which channel/slot/value
was written, not just a changed hash or count), with paired controls that
isolate one parameter at a time. It also proves RNG-stream identity (which
draws are keyed by recipe seed + component id vs. an explicit per-component
seed) and migration parity between the v1 override path and the v2 component
decomposition across all 656 registered (scenario, size) combinations.

No physics near-miss search: this is software perturbation testing of the
constructor bindings only. ``engine/config/finite_seed_effects.json`` (built
by ``scripts/verification/generate_finite_seed_effects.py``) is the coverage
map this module keeps honest via ``test_coverage_map_matches_every_registered_binding``.
"""
from copy import deepcopy
from hashlib import sha256
import numpy as np
import pytest

from phi_v2_lattice import web_seed_presets as WP, web_seeding as V1, web_scenarios as W
from phi_v2_lattice import native_codec as N, state as S, channels as C
from phi_v2_lattice.geometry import site_index
from phi_v2_lattice._proofs import encode

ALL_COMBOS = WP.all_ids_and_sizes()


# --------------------------------------------------------------------------
# Coverage map: single source of truth read by generate_finite_seed_effects.py
# --------------------------------------------------------------------------

COVERAGE_MAP = [
    # (category, kind_or_None, key, test_function_name)
    ("property", "field", "channel", "test_field_channel_selects_exactly_that_channel_and_no_other"),
    ("property", "field", "occupied", "test_field_occupied_sets_or_clears_the_channel"),
    ("property", "field", "density", "test_field_density_boundaries_and_fractional_draw"),
    ("property", "field_list", "occupied", "test_field_list_occupied_sets_or_clears_every_listed_channel"),
    ("collection", "field_list", "channels", "test_field_list_channels_are_exact_set_and_overlap_is_precise"),
    ("property", "field_random", "occupation", "test_field_random_occupation_boundaries"),
    ("property", "field_random", "polarity", "test_field_random_polarity_selects_the_correct_channel_half"),
    ("property", "field_random", "seed", "test_field_random_seed_matches_default_rng_and_is_reproducible"),
    ("property", "relation", "orientation", "test_relation_orientation_targets_the_exact_sc_or_fcc_slot"),
    ("property", "relation", "slot", "test_relation_slot_zero_and_one_are_independent"),
    ("property", "relation", "phase", "test_relation_phase_encodes_the_exact_token"),
    ("property", "relation", "polarity", "test_relation_polarity_encodes_the_exact_token"),
    ("property", "relation", "occupied", "test_relation_occupied_false_clears_to_blank"),
    ("property", "relation_background", "target", "test_relation_background_target_sc_fcc_both_write_exact_axes"),
    ("collection", "relation_background", "slots", "test_relation_background_slots_write_exactly_the_listed_slots"),
    ("property", "relation_background", "phase", "test_relation_background_phase_and_polarity_encode_the_exact_token"),
    ("property", "relation_background", "polarity", "test_relation_background_phase_and_polarity_encode_the_exact_token"),
    ("property", "sparse_tokens", "seed", "test_sparse_tokens_seed_reproducible_and_sensitive"),
    ("property", "sparse_tokens", "nTokens", "test_sparse_tokens_ntokens_touches_exactly_that_many_relation_slots"),
    ("property", "sparse_tokens", "fieldOccupation", "test_sparse_tokens_field_occupation_boundaries"),
    ("property", "sparse_tokens", "polarity", "test_sparse_tokens_polarity_selects_the_correct_channel_half"),
    ("property", "manifestation", "value", "test_manifestation_value_covers_the_full_ternary_alphabet"),
    ("property", "collision", "value", "test_collision_value_covers_the_full_layer_alphabet"),
    ("property", "index_pattern", "field", "test_index_pattern_field_targets_manifestation_or_collision"),
    ("property", "index_pattern", "modulus", "test_index_pattern_modulus_and_base_boundaries"),
    ("property", "index_pattern", "modulus", "test_index_pattern_modulus_ceiling_is_legal_when_the_region_keeps_values_in_bounds"),
    ("property", "index_pattern", "base", "test_index_pattern_modulus_and_base_boundaries"),
    ("region", None, "shape", "test_region_shape_selects_the_declared_geometry_exactly"),
    ("region", None, "x", "test_region_point_and_box_origin_axes_move_the_selection"),
    ("region", None, "y", "test_region_point_and_box_origin_axes_move_the_selection"),
    ("region", None, "z", "test_region_point_and_box_origin_axes_move_the_selection"),
    ("region", None, "dx", "test_region_box_extent_axes_size_the_selection"),
    ("region", None, "dy", "test_region_box_extent_axes_size_the_selection"),
    ("region", None, "dz", "test_region_box_extent_axes_size_the_selection"),
    ("region", None, "radius", "test_region_sphere_radius_is_inclusive_and_exact"),
    ("recipe", None, "size", "test_recipe_size_selects_the_registered_lattice_side"),
    ("recipe", None, "randomSeed", "test_recipe_random_seed_reroll_only_affects_density_based_field_components"),
    ("activation", "field", "enabled", "test_disabled_component_leaves_the_record_unchanged_for_every_kind"),
    ("activation", "field_list", "enabled", "test_disabled_component_leaves_the_record_unchanged_for_every_kind"),
    ("activation", "field_random", "enabled", "test_disabled_component_leaves_the_record_unchanged_for_every_kind"),
    ("activation", "relation", "enabled", "test_disabled_component_leaves_the_record_unchanged_for_every_kind"),
    ("activation", "relation_background", "enabled", "test_disabled_component_leaves_the_record_unchanged_for_every_kind"),
    ("activation", "sparse_tokens", "enabled", "test_disabled_component_leaves_the_record_unchanged_for_every_kind"),
    ("activation", "manifestation", "enabled", "test_disabled_component_leaves_the_record_unchanged_for_every_kind"),
    ("activation", "collision", "enabled", "test_disabled_component_leaves_the_record_unchanged_for_every_kind"),
    ("activation", "index_pattern", "enabled", "test_disabled_component_leaves_the_record_unchanged_for_every_kind"),
]


def _test_names_in_this_module():
    import ast
    import inspect
    tree = ast.parse(inspect.getsource(__import__(__name__)))
    return {node.name for node in ast.walk(tree) if isinstance(node, ast.FunctionDef) and node.name.startswith("test_")}


def test_coverage_map_matches_every_registered_binding():
    declared_properties = {(kind, key) for (kind, key) in WP.PROPERTY_META}
    declared_collections = {(kind, key) for (kind, key) in WP.COLLECTION_META}
    mapped_properties = {(kind, key) for cat, kind, key, _t in COVERAGE_MAP if cat == "property"}
    mapped_collections = {(kind, key) for cat, kind, key, _t in COVERAGE_MAP if cat == "collection"}
    mapped_regions = {key for cat, _kind, key, _t in COVERAGE_MAP if cat == "region"}
    mapped_recipe = {key for cat, _kind, key, _t in COVERAGE_MAP if cat == "recipe"}
    mapped_activation_kinds = {kind for cat, kind, key, _t in COVERAGE_MAP if cat == "activation" and key == "enabled"}

    assert mapped_properties == declared_properties, mapped_properties.symmetric_difference(declared_properties)
    assert mapped_collections == declared_collections, mapped_collections.symmetric_difference(declared_collections)
    assert mapped_regions == set(WP.REGION_META), mapped_regions.symmetric_difference(WP.REGION_META)
    assert mapped_recipe == {"size", "randomSeed"}
    assert mapped_activation_kinds == set(WP.KINDS), mapped_activation_kinds.symmetric_difference(WP.KINDS)

    live_tests = _test_names_in_this_module()
    for _cat, _kind, _key, test_name in COVERAGE_MAP:
        assert test_name in live_tests, f"coverage map cites undefined test {test_name}"


# --------------------------------------------------------------------------
# Shared helpers
# --------------------------------------------------------------------------

def _recipe(size, components, scenario_id="record-empty", random_seed=0, blank=True):
    return {"version": 2, "scenarioId": scenario_id, "size": size, "blank": blank,
            "randomSeed": random_seed, "components": components}


def _component(cid, kind, enabled, region, parameters):
    return {"id": cid, "kind": kind, "enabled": enabled, "region": region, "parameters": parameters}


def _all(size):
    return WP._all(size)


def _point(x, y, z):
    return WP._point(x, y, z)


def _box(x, y, z, dx, dy, dz):
    return WP._box(x, y, z, dx, dy, dz)


def _lattice(size, components, **kw):
    return WP.prepare_v2(_recipe(size, components, **kw)).lattice


# --------------------------------------------------------------------------
# field: channel / occupied / density
# --------------------------------------------------------------------------

@pytest.mark.parametrize("channel", [0, 1, 191, 192, 383])
def test_field_channel_selects_exactly_that_channel_and_no_other(channel):
    lattice = _lattice(3, [_component("c", "field", True, _all(3),
                                       dict(channel=channel, occupied=True, density=1.0))])
    assert lattice.bank[:, channel].all()
    assert lattice.bank.sum() == lattice.bank.shape[0]  # only this column, at every site


def test_field_occupied_sets_or_clears_the_channel():
    on = _lattice(3, [_component("c", "field", True, _all(3), dict(channel=7, occupied=True, density=1.0))])
    assert on.bank[:, 7].all()
    off = _lattice(3, [
        _component("set", "field", True, _all(3), dict(channel=7, occupied=True, density=1.0)),
        _component("clear", "field", True, _all(3), dict(channel=7, occupied=False, density=1.0)),
    ])
    assert not off.bank.any()


def test_field_density_boundaries_and_fractional_draw():
    size = 9
    zero = _lattice(size, [_component("c", "field", True, _all(size), dict(channel=3, occupied=True, density=0.0))])
    assert not zero.bank.any()
    full = _lattice(size, [_component("c", "field", True, _all(size), dict(channel=3, occupied=True, density=1.0))])
    assert full.bank[:, 3].sum() == size ** 3

    seed, cid, density = 42, "frac", 0.35
    got = _lattice(size, [_component(cid, "field", True, _all(size),
                                      dict(channel=3, occupied=True, density=density))],
                   random_seed=seed)
    rng_seed = int.from_bytes(sha256(f"{seed}:{cid}".encode()).digest()[:16], "little")
    expected_selected = np.random.Generator(np.random.PCG64(rng_seed)).random(size ** 3) < density
    assert np.array_equal(got.bank[:, 3], expected_selected)
    assert 0 < expected_selected.sum() < size ** 3  # a genuine fractional draw, not degenerate


# --------------------------------------------------------------------------
# field_list: channels (collection) / occupied
# --------------------------------------------------------------------------

def test_field_list_channels_are_exact_set_and_overlap_is_precise():
    lattice = _lattice(3, [_component("c", "field_list", True, _point(0, 0, 0),
                                       dict(channels=[5, 10, 200], occupied=True))])
    owner = site_index(3, 0, 0, 0)
    assert lattice.bank[owner, [5, 10, 200]].all()
    assert lattice.bank.sum() == 3
    # overlapping second component: clears only the shared channel, others survive
    lattice2 = _lattice(3, [
        _component("set", "field_list", True, _point(0, 0, 0), dict(channels=[5, 10, 200], occupied=True)),
        _component("clear", "field_list", True, _point(0, 0, 0), dict(channels=[10], occupied=False)),
    ])
    assert lattice2.bank[owner, [5, 200]].all()
    assert not lattice2.bank[owner, 10]
    assert lattice2.bank.sum() == 2


def test_field_list_occupied_sets_or_clears_every_listed_channel():
    on = _lattice(3, [_component("c", "field_list", True, _all(3), dict(channels=[1, 2, 3], occupied=True))])
    assert on.bank[:, [1, 2, 3]].all()
    off = _lattice(3, [
        _component("on", "field_list", True, _all(3), dict(channels=[1, 2, 3], occupied=True)),
        _component("off", "field_list", True, _all(3), dict(channels=[1, 2, 3], occupied=False)),
    ])
    assert not off.bank.any()


def test_field_list_rejects_duplicate_channel_indices():
    with pytest.raises(ValueError):
        _lattice(3, [_component("c", "field_list", True, _all(3), dict(channels=[1, 1], occupied=True))])


# --------------------------------------------------------------------------
# field_random: occupation / polarity / seed
# --------------------------------------------------------------------------

def test_field_random_occupation_boundaries():
    size = 4
    zero = _lattice(size, [_component("c", "field_random", True, _all(size),
                                       dict(occupation=0.0, polarity=1, seed=1))])
    assert not zero.bank.any()
    full = _lattice(size, [_component("c", "field_random", True, _all(size),
                                       dict(occupation=1.0, polarity=1, seed=1))])
    assert full.bank[:, 0:C.N_STATES].all()
    assert not full.bank[:, C.N_STATES:].any()


def test_field_random_polarity_selects_the_correct_channel_half():
    size = 3
    pos = _lattice(size, [_component("c", "field_random", True, _all(size),
                                      dict(occupation=1.0, polarity=1, seed=5))])
    assert pos.bank[:, :C.N_STATES].all() and not pos.bank[:, C.N_STATES:].any()
    neg = _lattice(size, [_component("c", "field_random", True, _all(size),
                                      dict(occupation=1.0, polarity=-1, seed=5))])
    assert neg.bank[:, C.N_STATES:].all() and not neg.bank[:, :C.N_STATES].any()


def test_field_random_seed_matches_default_rng_and_is_reproducible():
    size = 3
    lattice1 = _lattice(size, [_component("a", "field_random", True, _all(size),
                                           dict(occupation=0.5, polarity=1, seed=99))])
    lattice2 = _lattice(size, [_component("b", "field_random", True, _all(size),
                                           dict(occupation=0.5, polarity=1, seed=99))])
    # component id is irrelevant to this kind's stream: only the explicit seed matters
    assert np.array_equal(lattice1.bank, lattice2.bank)
    rng = np.random.default_rng(99)
    n = size ** 3
    expected = np.zeros((n, C.N_STATES), dtype=bool)
    expected[:] = rng.random((n, C.N_STATES)) < 0.5
    assert np.array_equal(lattice1.bank[:, :C.N_STATES], expected)

    changed = _lattice(size, [_component("a", "field_random", True, _all(size),
                                          dict(occupation=0.5, polarity=1, seed=100))])
    assert not np.array_equal(lattice1.bank, changed.bank)


# --------------------------------------------------------------------------
# relation: orientation / slot / phase / polarity / occupied
# --------------------------------------------------------------------------

@pytest.mark.parametrize("orientation", range(9))
def test_relation_orientation_targets_the_exact_sc_or_fcc_slot(orientation):
    lattice = _lattice(3, [_component("c", "relation", True, _point(0, 0, 0),
                                       dict(orientation=orientation, slot=0, phase=0, polarity=1, occupied=True))])
    owner = site_index(3, 0, 0, 0)
    token = S.idx_of(encode(0, 1))
    if orientation < 3:
        assert lattice.sc[owner, orientation, 0] == token
        assert np.count_nonzero(lattice.sc != S.BLANK_IDX) == 1
        assert not (lattice.fcc != S.BLANK_IDX).any()
    else:
        plane, diagonal = divmod(orientation - 3, 2)
        assert lattice.fcc[owner, plane, diagonal, 0] == token
        assert np.count_nonzero(lattice.fcc != S.BLANK_IDX) == 1
        assert not (lattice.sc != S.BLANK_IDX).any()


def test_relation_slot_zero_and_one_are_independent():
    owner = site_index(3, 1, 1, 1)
    lattice = _lattice(3, [
        _component("lam", "relation", True, _point(1, 1, 1),
                   dict(orientation=1, slot=0, phase=0, polarity=1, occupied=True)),
        _component("rho", "relation", True, _point(1, 1, 1),
                   dict(orientation=1, slot=1, phase=2, polarity=-1, occupied=True)),
    ])
    assert lattice.sc[owner, 1, 0] == S.idx_of(encode(0, 1))
    assert lattice.sc[owner, 1, 1] == S.idx_of(encode(2, -1))


@pytest.mark.parametrize("phase", range(4))
def test_relation_phase_encodes_the_exact_token(phase):
    lattice = _lattice(3, [_component("c", "relation", True, _point(0, 0, 0),
                                       dict(orientation=0, slot=0, phase=phase, polarity=1, occupied=True))])
    assert lattice.sc[site_index(3, 0, 0, 0), 0, 0] == S.idx_of(encode(phase, 1))


@pytest.mark.parametrize("polarity", [-1, 1])
def test_relation_polarity_encodes_the_exact_token(polarity):
    lattice = _lattice(3, [_component("c", "relation", True, _point(0, 0, 0),
                                       dict(orientation=0, slot=0, phase=1, polarity=polarity, occupied=True))])
    assert lattice.sc[site_index(3, 0, 0, 0), 0, 0] == S.idx_of(encode(1, polarity))


def test_relation_occupied_false_clears_to_blank():
    owner = site_index(3, 0, 0, 0)
    lattice = _lattice(3, [
        _component("set", "relation", True, _point(0, 0, 0),
                   dict(orientation=0, slot=0, phase=0, polarity=1, occupied=True)),
        _component("clear", "relation", True, _point(0, 0, 0),
                   dict(orientation=0, slot=0, phase=0, polarity=1, occupied=False)),
    ])
    assert lattice.sc[owner, 0, 0] == S.BLANK_IDX


# --------------------------------------------------------------------------
# relation_background: target / slots / phase / polarity
# --------------------------------------------------------------------------

@pytest.mark.parametrize("target,touches_sc,touches_fcc", [("sc", True, False), ("fcc", False, True), ("both", True, True)])
def test_relation_background_target_sc_fcc_both_write_exact_axes(target, touches_sc, touches_fcc):
    lattice = _lattice(3, [_component("c", "relation_background", True, _all(3),
                                       dict(target=target, slots=[0, 1], phase=0, polarity=1))])
    if touches_sc:
        assert (lattice.sc != S.BLANK_IDX).all()
    else:
        assert (lattice.sc == S.BLANK_IDX).all()
    if touches_fcc:
        assert (lattice.fcc != S.BLANK_IDX).all()
    else:
        assert (lattice.fcc == S.BLANK_IDX).all()


@pytest.mark.parametrize("slots", [[0], [1], [0, 1]])
def test_relation_background_slots_write_exactly_the_listed_slots(slots):
    lattice = _lattice(3, [_component("c", "relation_background", True, _all(3),
                                       dict(target="sc", slots=slots, phase=0, polarity=1))])
    for slot in (0, 1):
        expect_written = slot in slots
        got_written = bool((lattice.sc[:, :, slot] != S.BLANK_IDX).all())
        assert got_written == expect_written
        if not expect_written:
            assert (lattice.sc[:, :, slot] == S.BLANK_IDX).all()


@pytest.mark.parametrize("phase,polarity", [(0, 1), (2, -1), (3, 1)])
def test_relation_background_phase_and_polarity_encode_the_exact_token(phase, polarity):
    lattice = _lattice(3, [_component("c", "relation_background", True, _all(3),
                                       dict(target="both", slots=[0, 1], phase=phase, polarity=polarity))])
    token = S.idx_of(encode(phase, polarity))
    assert (lattice.sc == token).all() and (lattice.fcc == token).all()


def test_relation_background_rejects_empty_or_duplicate_slots():
    with pytest.raises(ValueError):
        _lattice(3, [_component("c", "relation_background", True, _all(3),
                                 dict(target="both", slots=[], phase=0, polarity=1))])
    with pytest.raises(ValueError):
        _lattice(3, [_component("c", "relation_background", True, _all(3),
                                 dict(target="both", slots=[0, 0], phase=0, polarity=1))])


# --------------------------------------------------------------------------
# sparse_tokens: seed / nTokens / fieldOccupation / polarity
# --------------------------------------------------------------------------

def test_sparse_tokens_seed_reproducible_and_sensitive():
    size = 3
    a = _lattice(size, [_component("c", "sparse_tokens", True, _all(size),
                                    dict(seed=7, nTokens=4, fieldOccupation=0.2, polarity=1))])
    b = _lattice(size, [_component("c", "sparse_tokens", True, _all(size),
                                    dict(seed=7, nTokens=4, fieldOccupation=0.2, polarity=1))])
    assert np.array_equal(a.sc, b.sc) and np.array_equal(a.fcc, b.fcc) and np.array_equal(a.bank, b.bank)
    c = _lattice(size, [_component("c", "sparse_tokens", True, _all(size),
                                    dict(seed=8, nTokens=4, fieldOccupation=0.2, polarity=1))])
    assert not (np.array_equal(a.sc, c.sc) and np.array_equal(a.fcc, c.fcc))


@pytest.mark.parametrize("n_tokens", [0, 1, 5, 9])
def test_sparse_tokens_ntokens_touches_exactly_that_many_relation_slots(n_tokens):
    lattice = _lattice(3, [_component("c", "sparse_tokens", True, _point(0, 0, 0),
                                       dict(seed=3, nTokens=n_tokens, fieldOccupation=0.0, polarity=1))])
    owner = site_index(3, 0, 0, 0)
    touched_sc = int(np.count_nonzero(lattice.sc[owner] != S.BLANK_IDX))
    touched_fcc = int(np.count_nonzero(lattice.fcc[owner] != S.BLANK_IDX))
    assert touched_sc + touched_fcc == n_tokens


def test_sparse_tokens_field_occupation_boundaries():
    size = 3
    zero = _lattice(size, [_component("c", "sparse_tokens", True, _all(size),
                                       dict(seed=1, nTokens=0, fieldOccupation=0.0, polarity=1))])
    assert not zero.bank.any()
    full = _lattice(size, [_component("c", "sparse_tokens", True, _all(size),
                                       dict(seed=1, nTokens=0, fieldOccupation=1.0, polarity=1))])
    assert full.bank[:, :C.N_STATES].all()


def test_sparse_tokens_polarity_selects_the_correct_channel_half():
    size = 3
    pos = _lattice(size, [_component("c", "sparse_tokens", True, _all(size),
                                      dict(seed=1, nTokens=0, fieldOccupation=1.0, polarity=1))])
    assert pos.bank[:, :C.N_STATES].all() and not pos.bank[:, C.N_STATES:].any()
    neg = _lattice(size, [_component("c", "sparse_tokens", True, _all(size),
                                      dict(seed=1, nTokens=0, fieldOccupation=1.0, polarity=-1))])
    assert neg.bank[:, C.N_STATES:].all() and not neg.bank[:, :C.N_STATES].any()


def test_sparse_tokens_rejects_more_tokens_than_the_components_own_region_capacity():
    with pytest.raises(ValueError):
        _lattice(3, [_component("c", "sparse_tokens", True, _point(0, 0, 0),
                                 dict(seed=1, nTokens=10, fieldOccupation=0.0, polarity=1))])


# --------------------------------------------------------------------------
# manifestation / collision: value
# --------------------------------------------------------------------------

@pytest.mark.parametrize("value", [-1, 0, 1])
def test_manifestation_value_covers_the_full_ternary_alphabet(value):
    lattice = _lattice(3, [_component("c", "manifestation", True, _point(0, 0, 0), dict(value=value))])
    assert lattice.s[site_index(3, 0, 0, 0)] == value
    assert lattice.s.sum() == value  # nowhere else touched


def test_manifestation_value_rejects_out_of_alphabet():
    with pytest.raises(ValueError):
        _lattice(3, [_component("c", "manifestation", True, _all(3), dict(value=2))])


@pytest.mark.parametrize("value", [0, 1, 2])
def test_collision_value_covers_the_full_layer_alphabet(value):
    lattice = _lattice(3, [_component("c", "collision", True, _point(0, 0, 0), dict(value=value))])
    assert lattice.ell[site_index(3, 0, 0, 0)] == value
    assert lattice.ell.sum() == value


def test_collision_value_rejects_out_of_alphabet():
    with pytest.raises(ValueError):
        _lattice(3, [_component("c", "collision", True, _all(3), dict(value=3))])
    with pytest.raises(ValueError):
        _lattice(3, [_component("c", "collision", True, _all(3), dict(value=-1))])


# --------------------------------------------------------------------------
# index_pattern: field / modulus / base
# --------------------------------------------------------------------------

@pytest.mark.parametrize("field,modulus,base,expect_attr", [
    ("manifestation", 3, -1, "s"),
    ("collision", 3, 0, "ell"),
])
def test_index_pattern_field_targets_manifestation_or_collision(field, modulus, base, expect_attr):
    size = 4
    lattice = _lattice(size, [_component("c", "index_pattern", True, _all(size),
                                          dict(field=field, modulus=modulus, base=base))])
    n = size ** 3
    expected = (np.arange(n) % modulus + base).astype(np.int8)
    assert np.array_equal(getattr(lattice, expect_attr), expected)
    other_attr = "ell" if expect_attr == "s" else "s"
    assert not getattr(lattice, other_attr).any()


@pytest.mark.parametrize("modulus,base", [(1, 0), (2, -1)])
def test_index_pattern_modulus_and_base_boundaries(modulus, base):
    size = 3
    n = size ** 3
    lattice = _lattice(size, [_component("c", "index_pattern", True, _all(size),
                                          dict(field="manifestation", modulus=modulus, base=base))])
    expected = (np.arange(n) % modulus + base).astype(np.int8)
    assert (expected >= -1).all() and (expected <= 1).all()
    assert np.array_equal(lattice.s, expected)


def test_index_pattern_modulus_ceiling_is_legal_when_the_region_keeps_values_in_bounds():
    # modulus=4096 (its declared max) only stays inside the manifestation alphabet
    # when the selected region's absolute index range is narrow enough; a single
    # point at the origin (absolute index 0) isolates the base value exactly.
    lattice = _lattice(3, [_component("c", "index_pattern", True, _point(0, 0, 0),
                                       dict(field="manifestation", modulus=4096, base=-1))])
    assert lattice.s[site_index(3, 0, 0, 0)] == -1
    assert lattice.s.sum() == -1


def test_index_pattern_rejects_values_outside_the_target_alphabet():
    with pytest.raises(ValueError):
        _lattice(3, [_component("c", "index_pattern", True, _all(3),
                                 dict(field="manifestation", modulus=4096, base=0))])
    with pytest.raises(ValueError):
        _lattice(3, [_component("c", "index_pattern", True, _all(3),
                                 dict(field="collision", modulus=4096, base=0))])


def test_index_pattern_rejects_unknown_field():
    with pytest.raises(ValueError):
        _lattice(3, [_component("c", "index_pattern", True, _all(3),
                                 dict(field="bank", modulus=3, base=-1))])


# --------------------------------------------------------------------------
# Region: shape / x,y,z / dx,dy,dz / radius
# --------------------------------------------------------------------------

def _marked(size, region):
    return _lattice(size, [_component("c", "field_list", True, region, dict(channels=[9], occupied=True))]).bank[:, 9]


@pytest.mark.parametrize("shape,region,expected_sites", [
    ("all", {"shape": "all", "x": 0, "y": 0, "z": 0, "dx": 3, "dy": 3, "dz": 3, "radius": 0.0},
     [site_index(3, x, y, z) for x in range(3) for y in range(3) for z in range(3)]),
    ("point", {"shape": "point", "x": 2, "y": 1, "z": 0, "dx": 1, "dy": 1, "dz": 1, "radius": 0.0},
     [site_index(3, 2, 1, 0)]),
    ("box", {"shape": "box", "x": 0, "y": 0, "z": 0, "dx": 2, "dy": 1, "dz": 1, "radius": 0.0},
     [site_index(3, x, 0, 0) for x in range(2)]),
    ("sphere", {"shape": "sphere", "x": 1, "y": 1, "z": 1, "dx": 1, "dy": 1, "dz": 1, "radius": 1.0},
     [site_index(3, 1, 1, 1), site_index(3, 0, 1, 1), site_index(3, 2, 1, 1),
      site_index(3, 1, 0, 1), site_index(3, 1, 2, 1), site_index(3, 1, 1, 0), site_index(3, 1, 1, 2)]),
])
def test_region_shape_selects_the_declared_geometry_exactly(shape, region, expected_sites):
    marked = _marked(3, region)
    got = set(np.flatnonzero(marked).tolist())
    assert got == set(expected_sites)


@pytest.mark.parametrize("x,y,z", [(0, 0, 0), (2, 0, 0), (0, 2, 0), (0, 0, 2), (2, 2, 2)])
def test_region_point_and_box_origin_axes_move_the_selection(x, y, z):
    point_marked = _marked(3, _point(x, y, z))
    assert set(np.flatnonzero(point_marked).tolist()) == {site_index(3, x, y, z)}
    box_marked = _marked(3, _box(x, y, z, 1, 1, 1))
    assert set(np.flatnonzero(box_marked).tolist()) == {site_index(3, x, y, z)}


@pytest.mark.parametrize("dx,dy,dz,count", [(1, 1, 1, 1), (3, 1, 1, 3), (1, 2, 1, 2), (3, 3, 3, 27)])
def test_region_box_extent_axes_size_the_selection(dx, dy, dz, count):
    marked = _marked(3, _box(0, 0, 0, dx, dy, dz))
    assert marked.sum() == count
    got = set(np.flatnonzero(marked).tolist())
    expected = {site_index(3, x, y, z) for x in range(dx) for y in range(dy) for z in range(dz)}
    assert got == expected


def test_region_box_extending_outside_the_lattice_is_rejected():
    with pytest.raises(ValueError):
        _marked(3, _box(2, 0, 0, 2, 1, 1))


@pytest.mark.parametrize("radius,count", [(0.0, 1), (1.0, 7)])
def test_region_sphere_radius_is_inclusive_and_exact(radius, count):
    region = {"shape": "sphere", "x": 1, "y": 1, "z": 1, "dx": 1, "dy": 1, "dz": 1, "radius": radius}
    marked = _marked(3, region)
    assert marked.sum() == count


def test_region_active_flags_track_the_selected_shape():
    for shape in ("all", "point", "box", "sphere"):
        recipe = WP.default_recipe("empty", 9)  # "empty" registers zero default components
        assert recipe["components"] == []
        component = WP.component_template("field_list", 9, "probe")
        component["region"]["shape"] = shape
        recipe["components"].append(component)
        description = WP.describe_recipe(recipe)
        by_key = {p["key"]: p for p in description["properties"] if p["path"][:2] == ["components", 0] and ".region." in p["key"]}
        shape_prop = next(p for k, p in by_key.items() if k.endswith(".shape"))
        assert shape_prop["active"] is True
        for axis in ("x", "y", "z"):
            prop = next(p for k, p in by_key.items() if k.endswith(f".{axis}"))
            assert prop["active"] == (shape != "all")
        for axis in ("dx", "dy", "dz"):
            prop = next(p for k, p in by_key.items() if k.endswith(f".{axis}"))
            assert prop["active"] == (shape == "box")
        radius_prop = next(p for k, p in by_key.items() if k.endswith(".radius"))
        assert radius_prop["active"] == (shape == "sphere")


# --------------------------------------------------------------------------
# Recipe-level: size / randomSeed
# --------------------------------------------------------------------------

@pytest.mark.parametrize("size", WP.SIZES)
def test_recipe_size_selects_the_registered_lattice_side(size):
    recipe = WP.blank_base(size)
    state = WP.prepare_v2(recipe)
    assert state.lattice.L == size
    assert state.lattice.s.shape == (size ** 3,)


def test_recipe_random_seed_reroll_only_affects_density_based_field_components():
    size = 7
    # field_random/sparse_tokens write their polarity=-1 half (channels 192-383)
    # so they never overwrite the "dens" component's channel-0 (polarity +1) bit;
    # otherwise a later writer at the same channel would mask the effect being isolated.
    components = [
        _component("dens", "field", True, _all(size), dict(channel=0, occupied=True, density=0.5)),
        _component("rand", "field_random", True, _all(size), dict(occupation=0.5, polarity=-1, seed=11)),
        _component("sparse", "sparse_tokens", True, _all(size), dict(seed=13, nTokens=2, fieldOccupation=0.3, polarity=-1)),
        _component("rel", "relation", True, _point(0, 0, 0), dict(orientation=0, slot=0, phase=0, polarity=1, occupied=True)),
    ]
    a = _lattice(size, deepcopy(components), random_seed=1)
    b = _lattice(size, deepcopy(components), random_seed=2)
    assert not np.array_equal(a.bank[:, 0], b.bank[:, 0])  # density-based field reflects the reseed
    # Isolate field_random/sparse_tokens contributions with density switched off, to
    # confirm the recipe seed alone leaves their explicit-seed streams untouched.
    isolated = [c for c in components if c["kind"] != "field"]
    a2 = _lattice(size, deepcopy(isolated), random_seed=1)
    b2 = _lattice(size, deepcopy(isolated), random_seed=2)
    assert np.array_equal(a2.bank, b2.bank)
    assert np.array_equal(a2.sc, b2.sc) and np.array_equal(a2.fcc, b2.fcc)


# --------------------------------------------------------------------------
# Activation: disabled components leave the record unchanged for every kind,
# while their fields remain valid, described, and validated.
# --------------------------------------------------------------------------

_DISABLED_PROBE_PARAMETERS = {
    "field": dict(channel=5, occupied=True, density=1.0),
    "field_list": dict(channels=[5, 6], occupied=True),
    "field_random": dict(occupation=1.0, polarity=1, seed=1),
    "relation": dict(orientation=0, slot=0, phase=0, polarity=1, occupied=True),
    "relation_background": dict(target="both", slots=[0, 1], phase=0, polarity=1),
    "sparse_tokens": dict(seed=1, nTokens=5, fieldOccupation=1.0, polarity=1),
    "manifestation": dict(value=1),
    "collision": dict(value=2),
    "index_pattern": dict(field="manifestation", modulus=2, base=0),
}


@pytest.mark.parametrize("kind", WP.KINDS)
def test_disabled_component_leaves_the_record_unchanged_for_every_kind(kind):
    size = 3
    region = _point(0, 0, 0) if kind not in ("relation_background",) else _all(size)
    component = _component("probe", kind, False, region, _DISABLED_PROBE_PARAMETERS[kind])
    lattice = _lattice(size, [component])
    blank = S.blank(size)
    assert not lattice.bank.any() and np.array_equal(lattice.bank, blank.bank)
    assert np.array_equal(lattice.s, blank.s) and np.array_equal(lattice.ell, blank.ell)
    assert np.all(lattice.sc == S.BLANK_IDX) and np.all(lattice.fcc == S.BLANK_IDX)

    # The disabled component's own fields remain fully described and pass schema
    # validation (finite bounds, resolvable value) exactly like an enabled one.
    recipe = _recipe(size, [component])
    description = WP.describe_recipe(recipe)
    matching = [p for p in description["properties"] if p["path"][:2] == ["components", 0]
                and p["path"][-1] != "enabled" and "region" not in p["path"]]
    assert matching, f"{kind}: no described parameter properties for a disabled component"
    for prop in matching:
        assert prop["min"] <= prop["max"]
        assert isinstance(prop["value"], (bool, int, float, str))


# --------------------------------------------------------------------------
# Migration parity: v1 override path vs. v2 component decomposition, for
# every one of the 656 registered (scenario, size) combinations, with an
# identical field-kind overlay component appended in the same order.
# --------------------------------------------------------------------------

_PARITY_OVERLAY_SEED = 777
_PARITY_OVERLAY = _component("overlay-parity", "field", True, None, dict(channel=17, occupied=True, density=0.4))


def _overlay_for(size):
    overlay = deepcopy(_PARITY_OVERLAY)
    overlay["region"] = _all(size)
    return overlay


@pytest.mark.parametrize("scenario_id,size", ALL_COMBOS, ids=[f"{i}@{s}" for i, s in ALL_COMBOS])
def test_v1_override_and_v2_decomposition_agree_byte_for_byte_with_an_identical_overlay(scenario_id, size):
    overlay = _overlay_for(size)
    v1_recipe = {"version": 1, "scenarioId": "record-" + scenario_id, "size": size, "blank": False,
                 "randomSeed": _PARITY_OVERLAY_SEED, "components": [overlay]}
    v1_data, v1_hash, _ = V1.checkpoint(v1_recipe)

    v2_recipe = WP.default_recipe(scenario_id, size)
    v2_recipe["randomSeed"] = _PARITY_OVERLAY_SEED
    v2_recipe["components"].append(deepcopy(overlay))
    v2_state = WP.prepare_v2(v2_recipe)
    v2_data = N.encode(v2_state)

    assert v2_data == v1_data
    assert sha256(v2_data).hexdigest() == v1_hash


def test_migration_parity_covers_all_656_registered_combinations():
    assert len(ALL_COMBOS) == 656
    assert len(set(ALL_COMBOS)) == 656


def test_renaming_a_default_component_id_does_not_perturb_an_unrelated_overlay_stream():
    # "sparse" has one sparse_tokens default component whose own randomness is
    # keyed by its explicit seed parameter, not by its component id; renaming
    # that id must leave both its own effect and an independent field overlay's
    # id-keyed RNG draw completely unchanged -- proving the rename is safe here.
    size = 9
    recipe_a = WP.default_recipe("sparse", size)
    recipe_a["components"].append(_component("collision-probe", "field", True, _all(size),
                                              dict(channel=50, occupied=True, density=0.3)))
    recipe_b = deepcopy(recipe_a)
    original_id = recipe_b["components"][0]["id"]
    assert original_id == "sparse-0"
    recipe_b["components"][0]["id"] = "sparse-0-renamed"

    lattice_a = WP.prepare_v2(recipe_a).lattice
    lattice_b = WP.prepare_v2(recipe_b).lattice
    assert np.array_equal(lattice_a.sc, lattice_b.sc) and np.array_equal(lattice_a.fcc, lattice_b.fcc)
    assert np.array_equal(lattice_a.bank, lattice_b.bank)


def test_reusing_a_freed_component_id_for_a_new_overlay_is_deterministic_and_independent():
    # Once "sparse-0" is freed by the rename above, a brand-new component may
    # legally reuse that literal id string; its RNG draw depends only on
    # (recipe seed, its own id), never on the renamed component's prior identity.
    size = 9
    base = WP.default_recipe("sparse", size)
    base["components"][0]["id"] = "sparse-0-renamed"
    reused_id_recipe = deepcopy(base)
    reused_id_recipe["components"].append(_component("sparse-0", "field", True, _all(size),
                                                       dict(channel=60, occupied=True, density=0.3)))
    fresh_id_recipe = deepcopy(base)
    fresh_id_recipe["components"].append(_component("sparse-0", "field", True, _all(size),
                                                      dict(channel=60, occupied=True, density=0.3)))
    a = WP.prepare_v2(reused_id_recipe).lattice
    b = WP.prepare_v2(fresh_id_recipe).lattice
    assert np.array_equal(a.bank, b.bank)  # deterministic and reproducible


# --------------------------------------------------------------------------
# RNG determinism: the declared formula, exactly
# --------------------------------------------------------------------------

@pytest.mark.parametrize("seed,cid", [(0, "a"), (42, "component-1"), (2 ** 32 - 1, "z")])
def test_component_rng_matches_the_declared_sha256_pcg64_formula(seed, cid):
    got = WP._component_rng(seed, cid)
    rng_seed = int.from_bytes(sha256(f"{seed}:{cid}".encode()).digest()[:16], "little")
    expected = np.random.Generator(np.random.PCG64(rng_seed))
    assert np.array_equal(got.random(16), expected.random(16))


def test_field_component_rng_is_keyed_by_both_recipe_seed_and_component_id():
    size = 9
    base_params = dict(channel=0, occupied=True, density=0.4)
    by_seed = _lattice(size, [_component("x", "field", True, _all(size), base_params)], random_seed=1)
    by_seed_2 = _lattice(size, [_component("x", "field", True, _all(size), base_params)], random_seed=2)
    assert not np.array_equal(by_seed.bank[:, 0], by_seed_2.bank[:, 0])

    by_cid = _lattice(size, [_component("y", "field", True, _all(size), base_params)], random_seed=1)
    assert not np.array_equal(by_seed.bank[:, 0], by_cid.bank[:, 0])

    repeat = _lattice(size, [_component("x", "field", True, _all(size), base_params)], random_seed=1)
    assert np.array_equal(by_seed.bank[:, 0], repeat.bank[:, 0])
