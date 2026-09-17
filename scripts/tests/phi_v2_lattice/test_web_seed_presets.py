"""v2 editable constructor decomposition: exact parity, schema completeness, and real effects."""
from copy import deepcopy
import numpy as np
import pytest

from phi_v2_lattice import web_seed_presets as WP, web_seeding as V1, web_scenarios as W
from phi_v2_lattice import native_codec as N, state as S
from phi_v2_lattice._proofs import encode

ALL_COMBOS = WP.all_ids_and_sizes()


# --------------------------------------------------------------------------
# Exact legacy parity: every one of the 656 registered variants
# --------------------------------------------------------------------------

@pytest.mark.parametrize("scenario_id,size", ALL_COMBOS, ids=[f"{i}@{s}" for i, s in ALL_COMBOS])
def test_v2_default_recipe_is_byte_identical_to_the_frozen_checkpoint(scenario_id, size):
    recipe = WP.default_recipe(scenario_id, size)
    before = deepcopy(recipe)
    state = WP.prepare_v2(recipe)
    got = N.encode(state)
    want, want_hash = W.checkpoint(scenario_id, size)
    assert got == want
    from hashlib import sha256
    assert sha256(got).hexdigest() == want_hash
    assert recipe == before
    assert state.microtick == 0


def test_v2_recipe_never_uses_an_opaque_checkpoint_override():
    # v2 always starts from a blank lattice and rebuilds every field from its
    # own ordered components; it never calls the frozen web_scenarios.prepare.
    import inspect
    source = inspect.getsource(WP.prepare_v2)
    assert "W.prepare(" not in source
    assert "S.blank(size)" in source


def test_all_656_combinations_are_covered_exactly_once():
    assert len(ALL_COMBOS) == 656
    assert len(set(ALL_COMBOS)) == 656
    assert sum(1 for row in W.scenarios()) == 629


def test_explicit_override_metadata_preserves_exact_records_and_default_reset_targets():
    recipe = WP.default_recipe('relation', 7)
    reference = N.encode(WP.prepare_v2(recipe))
    component = recipe['components'][0]
    recipe['explicitOverrides'] = [f'["component","{component["id"]}","region","x"]']
    assert N.encode(WP.prepare_v2(recipe)) == reference
    # The metadata tracks whether a value follows a dependent default. It is
    # not a hidden record write or a new stream seed.
    assert WP.describe_recipe(recipe)['recipe']['explicitOverrides'] == recipe['explicitOverrides']
    component['region']['x'] = 0
    p = next(p for p in WP.describe_recipe(recipe)['properties'] if p['path'] == ['components', 0, 'region', 'x'])
    assert p['value'] == 0
    assert p['default'] == WP.default_recipe('relation', 7)['components'][0]['region']['x']


def test_new_component_resets_to_canonical_template_instead_of_edited_values():
    recipe = WP.default_recipe('empty', 7)
    component = WP.component_template('relation', 7, 'new-component')
    expected = deepcopy(component)
    component['region']['x'] = 0
    component['parameters']['phase'] = 3
    recipe['components'].append(component)
    properties = WP.describe_recipe(recipe)['properties']
    for path in [('region', 'x'), ('parameters', 'phase')]:
        prop = next(p for p in properties if p['path'] == ['components', 0, *path])
        assert prop['default'] == expected[path[0]][path[1]]
        assert prop['value'] == component[path[0]][path[1]]


def test_resolved_geometric_support_distinguishes_masks_from_record_addresses_and_realized_writes():
    recipe=WP.default_recipe('empty',7)
    first=WP.component_template('field',7,'first')
    first['enabled']=True
    first['region']=WP._point(1,2,3)
    first['parameters']['density']=0
    second=deepcopy(first); second['id']='second'; second['parameters']['channel']=1
    inactive=deepcopy(first); inactive['id']='inactive'; inactive['enabled']=False
    recipe['components']=[first,second,inactive]
    support=WP.describe_recipe(recipe)['geometrySupport']
    assert support['selectedSites']==support['overlappingSites']==1
    assert [c['overlapWithEarlier'] for c in support['components']]==[0,1,0]
    assert [c['selectedSites'] for c in support['components']]==[1,1,0]
    assert not WP.prepare_v2(recipe).lattice.bank.any(), 'Geometry is selected even when zero-density writes leave no field tokens'


# --------------------------------------------------------------------------
# v1 continues to work exactly as before, unaffected by the new module
# --------------------------------------------------------------------------

@pytest.mark.parametrize("row", W.scenarios(), ids=lambda r: r.id)
def test_v1_web_seeding_prepare_is_unaffected_by_the_v2_module(row):
    for size in row.sizes:
        recipe = dict(version=1, scenarioId="record-" + row.id, size=size, blank=False,
                       randomSeed=0, components=[])
        data, digest, _ = V1.checkpoint(recipe)
        assert (data, digest) == W.checkpoint(row.id, size)


# --------------------------------------------------------------------------
# Schema completeness: every descriptor resolves to the real recipe value
# --------------------------------------------------------------------------

def _resolve(recipe, path):
    node = recipe
    for step in path:
        node = node[step]
    return node


REQUIRED_KEYS = {"key", "label", "description", "group", "type", "units", "default", "value",
                  "min", "max", "step", "recommended", "recommendationBasis", "binding",
                  "options", "path"}

SUPPORTED_TYPES = {"real", "integer", "choice"}


def _assert_finite_numeric_bounds(prop):
    assert prop["type"] in SUPPORTED_TYPES
    assert isinstance(prop["min"], (int, float)) and not isinstance(prop["min"], bool)
    assert isinstance(prop["max"], (int, float)) and not isinstance(prop["max"], bool)
    assert prop["min"] <= prop["max"]
    assert isinstance(prop["step"], (int, float)) and not isinstance(prop["step"], bool) and prop["step"] > 0
    lo, hi = prop["recommended"]
    assert isinstance(lo, (int, float)) and not isinstance(lo, bool)
    assert isinstance(hi, (int, float)) and not isinstance(hi, bool)
    assert lo <= hi
    assert prop["min"] <= lo and hi <= prop["max"]
    assert prop["recommendationBasis"]


def _assert_boolean_choice_is_false_off_then_true_on(prop):
    keys_ = [opt[0] for opt in prop["options"]]
    if len(keys_) == 2 and all(isinstance(k, bool) for k in keys_) and set(keys_) == {False, True}:
        assert keys_ == [False, True], f"{prop['key']}: boolean options must list false before true"
        assert "off" in prop["options"][0][1].lower()
        assert "on" in prop["options"][1][1].lower()


def _assert_numeric_choice_is_sorted(prop):
    keys_ = [opt[0] for opt in prop["options"]]
    if keys_ and all(isinstance(k, (int, float)) and not isinstance(k, bool) for k in keys_):
        assert keys_ == sorted(keys_), f"{prop['key']}: numeric choice options must be sorted ascending"
        assert len(set(keys_)) == len(keys_), f"{prop['key']}: duplicate choice option skipped"


@pytest.mark.parametrize("scenario_id,size", ALL_COMBOS, ids=[f"{i}@{s}" for i, s in ALL_COMBOS])
def test_every_descriptor_resolves_to_its_real_recipe_value(scenario_id, size):
    description = WP.describe(scenario_id, size)
    assert description["schemaVersion"] == 2
    assert description["schemaIdentity"] == "finite-seed-2"
    assert description["scenarioId"] == "record-" + scenario_id
    recipe = description["recipe"]
    assert recipe == WP.default_recipe(scenario_id, size)
    assert len(description["properties"]) > 0
    assert "blank" not in {tuple(p["path"]) for p in description["properties"]}
    seen_keys = set()
    for prop in description["properties"]:
        assert REQUIRED_KEYS <= set(prop)
        assert prop["key"] not in seen_keys
        seen_keys.add(prop["key"])
        assert prop["label"] and prop["description"]
        assert len(prop["description"]) > 20
        assert prop["path"]
        assert prop["path"][:1] != ["blank"]
        assert prop["binding"] and isinstance(prop["binding"], str)
        _assert_finite_numeric_bounds(prop)
        resolved = _resolve(recipe, prop["path"])
        assert resolved == prop["value"] == prop["default"]
        if prop["type"] == "choice":
            assert prop["options"]
            assert prop["value"] in [opt[0] for opt in prop["options"]]
            _assert_boolean_choice_is_false_off_then_true_on(prop)
            _assert_numeric_choice_is_sorted(prop)


def test_lattice_size_property_is_a_choice_of_exactly_the_five_legal_sizes():
    description = WP.describe("witness", 9)
    size_prop = next(p for p in description["properties"] if p["key"] == "size")
    assert size_prop["type"] == "choice"
    assert [opt[0] for opt in size_prop["options"]] == [3, 4, 7, 9, 17]


def test_region_fields_carry_finite_size_scoped_bounds_and_dependencies():
    description = WP.describe("relation", 9)
    by_key = {p["key"]: p for p in description["properties"] if ".region." in p["key"]}
    shape_key = next(k for k in by_key if k.endswith(".shape"))
    assert by_key[shape_key]["active"] is True
    for axis in ("x", "y", "z"):
        prop = next(p for k, p in by_key.items() if k.endswith(f".{axis}"))
        assert prop["min"] == 0 and prop["max"] == 8  # size - 1 at size=9
        assert prop["dependencies"] == [shape_key]
        assert prop["active"] is True  # shape=point for the "relation" preset
    for axis in ("dx", "dy", "dz"):
        prop = next(p for k, p in by_key.items() if k.endswith(f".{axis}"))
        assert prop["min"] == 1 and prop["max"] == 9  # size at size=9
        assert prop["active"] is False  # shape=point, box fields inactive
    radius_prop = next(p for k, p in by_key.items() if k.endswith(".radius"))
    assert radius_prop["min"] == 0 and radius_prop["max"] == 9
    assert radius_prop["active"] is False  # shape=point, sphere field inactive


def test_ntokens_bound_reflects_the_components_own_region_not_the_whole_domain():
    recipe = WP.default_recipe("sparse", 9)  # region is "all" at size 9: 729 sites
    recipe["components"][0]["region"] = {"shape": "point", "x": 0, "y": 0, "z": 0,
                                          "dx": 1, "dy": 1, "dz": 1, "radius": 0.0}
    description = WP.describe_recipe(recipe)
    n_tokens_prop = next(p for p in description["properties"] if p["key"].endswith("nTokens"))
    assert n_tokens_prop["max"] == 9  # one site x 9 relation slots, not 729 x 9
    whole_domain = WP.describe("sparse", 9)
    n_tokens_whole = next(p for p in whole_domain["properties"] if p["key"].endswith("nTokens"))
    assert n_tokens_whole["max"] == 9 ** 3 * 9


def test_field_channel_description_retains_full_internal_state_identity():
    description = WP.describe_base(9)
    channel_prop = next(p for p in description["properties"] if p["key"].endswith("parameters.channel"))
    text = channel_prop["description"].lower()
    assert "tangent" in text
    assert "phase" in text
    assert "polarity" in text
    assert "flag" in text or "internal" in text


# --------------------------------------------------------------------------
# integer_list flattening (item 3): row properties + collections template
# --------------------------------------------------------------------------

def test_field_list_channels_is_flattened_into_integer_row_properties():
    description = WP.describe("seam", 3)
    field_list_component = next(i for i, c in enumerate(description["recipe"]["components"]) if c["kind"] == "field_list")
    channels = description["recipe"]["components"][field_list_component]["parameters"]["channels"]
    row_props = [p for p in description["properties"]
                 if p["path"][:4] == ["components", field_list_component, "parameters", "channels"]]
    assert len(row_props) == len(channels)
    for row_index, prop in enumerate(row_props):
        assert prop["path"] == ["components", field_list_component, "parameters", "channels", row_index]
        assert prop["type"] == "integer"
        assert prop["value"] == channels[row_index]
        assert prop["min"] == 0 and prop["max"] == 383
    assert not any(p["type"] == "integer_list" for p in description["properties"])
    collection = next(c for c in description["collections"]
                       if c["path"] == ["components", field_list_component, "parameters", "channels"])
    assert collection["itemType"] == "integer"
    assert collection["minRows"] == 1 and collection["maxRows"] == 384
    assert collection["defaultRow"] == 0
    assert collection["rows"] == len(channels)


def test_relation_background_slots_is_flattened_into_choice_row_properties():
    description = WP.describe("r5", 3)
    component_index = next(i for i, c in enumerate(description["recipe"]["components"]) if c["kind"] == "relation_background")
    slots = description["recipe"]["components"][component_index]["parameters"]["slots"]
    row_props = [p for p in description["properties"]
                 if p["path"][:4] == ["components", component_index, "parameters", "slots"]]
    assert len(row_props) == len(slots)
    for row_index, prop in enumerate(row_props):
        assert prop["type"] == "choice"
        assert prop["value"] == slots[row_index]
        assert [opt[0] for opt in prop["options"]] == [0, 1]
    collection = next(c for c in description["collections"]
                       if c["path"] == ["components", component_index, "parameters", "slots"])
    assert collection["itemType"] == "choice"
    assert collection["minRows"] == 1 and collection["maxRows"] == 2


def test_every_descriptor_schema_contains_no_unsupported_property_types():
    # Full sweep (all 656), not a sample: every property type is real/integer/choice.
    for scenario_id, size in ALL_COMBOS:
        description = WP.describe(scenario_id, size)
        for prop in description["properties"]:
            assert prop["type"] in SUPPORTED_TYPES, f"{scenario_id}@{size}: unsupported type {prop['type']} at {prop['key']}"


# --------------------------------------------------------------------------
# describe_recipe / component library / templates (item 4)
# --------------------------------------------------------------------------

def test_describe_recipe_covers_an_edited_recipe_not_just_the_default():
    recipe = WP.default_recipe("relation", 3)
    recipe["components"][0]["parameters"]["orientation"] = 5
    description = WP.describe_recipe(recipe)
    orientation_prop = next(p for p in description["properties"] if p["key"].endswith("parameters.orientation"))
    assert orientation_prop["value"] == 5
    # default must stay pinned to the frozen preset's own value, not drift to the edit
    assert orientation_prop["default"] == 0


def test_describe_recipe_default_matches_preset_by_component_id_not_position():
    recipe = WP.default_recipe("witness", 9)
    # reorder the components: id-matching must still find the right defaults
    recipe["components"] = list(reversed(recipe["components"]))
    description = WP.describe_recipe(recipe)
    for prop in description["properties"]:
        if len(prop["path"]) >= 2 and prop["path"][0] == "components":
            component_index = prop["path"][1]
            component_id = recipe["components"][component_index]["id"]
            default_component = next(c for c in WP.default_recipe("witness", 9)["components"] if c["id"] == component_id)
            resolved_default = default_component
            for step in prop["path"][2:]:
                resolved_default = resolved_default[step]
            assert prop["default"] == resolved_default


def test_describe_recipe_falls_back_to_kind_template_for_a_new_component():
    recipe = WP.default_recipe("empty", 3)
    recipe["components"].append(WP.component_template("collision", 3, "extra-collision"))
    new_index = len(recipe["components"]) - 1
    recipe["components"][new_index]["enabled"] = True
    recipe["components"][new_index]["parameters"]["value"] = 2
    description = WP.describe_recipe(recipe)
    value_prop = next(p for p in description["properties"]
                       if p["path"] == ["components", new_index, "parameters", "value"])
    assert value_prop["value"] == 2
    assert value_prop["default"] == 0  # the kind's own _DEFAULT_PARAMETERS template, no invented drift


def test_component_template_is_inactive_at_kind_defaults():
    template = WP.component_template("field", 9, "probe")
    assert template["id"] == "probe" and template["kind"] == "field" and template["enabled"] is False
    assert template["region"] == {"shape": "all", "x": 0, "y": 0, "z": 0, "dx": 9, "dy": 9, "dz": 9, "radius": 0.0}
    assert template["parameters"] == dict(channel=0, occupied=True, density=1.0)


def test_component_template_rejects_unknown_kind_or_size():
    with pytest.raises(ValueError):
        WP.component_template("unknown", 9, "x")
    with pytest.raises(ValueError):
        WP.component_template("field", 5, "x")


def test_component_library_covers_every_kind_from_default_parameters():
    library = WP.component_library()
    assert set(library) == set(WP.KINDS)
    for kind in WP.KINDS:
        entry = library[kind]
        assert entry["defaultParameters"] == WP._DEFAULT_PARAMETERS[kind]
        described_keys = {p["key"] for p in entry["properties"]} | {c["key"] for c in entry["collections"]}
        assert described_keys == set(WP._DEFAULT_PARAMETERS[kind])
        for prop in entry["properties"]:
            assert prop["type"] in SUPPORTED_TYPES
            assert isinstance(prop["min"], (int, float)) and not isinstance(prop["min"], bool)
            assert isinstance(prop["max"], (int, float)) and not isinstance(prop["max"], bool)
            assert prop["min"] <= prop["max"]


def test_all_component_templates_covers_every_legal_size_and_kind():
    templates = WP.all_component_templates()
    assert set(templates) == set(WP.SIZES)
    for size, by_kind in templates.items():
        assert set(by_kind) == set(WP.KINDS)
        for kind, template in by_kind.items():
            assert template["kind"] == kind and template["enabled"] is False
            assert template["region"]["dx"] == size


def test_describe_base_exposes_every_kind_inactive():
    base = WP.describe_base(9)
    assert base["scenarioId"] == "record-base"
    recipe = base["recipe"]
    assert recipe["blank"] is True
    assert {c["kind"] for c in recipe["components"]} == set(WP.KINDS)
    assert all(not c["enabled"] for c in recipe["components"])
    # every kind's full parameter family is present and inactive, not empty
    for component in recipe["components"]:
        assert component["parameters"]
    state = WP.prepare_v2(recipe)
    blank = S.blank(9)
    assert not state.lattice.bank.any()
    assert np.array_equal(state.lattice.s, blank.s)
    assert np.array_equal(state.lattice.ell, blank.ell)
    assert np.all(state.lattice.sc == S.BLANK_IDX) and np.all(state.lattice.fcc == S.BLANK_IDX)


# --------------------------------------------------------------------------
# Every real scalar constructor input actually affects the compiled record
# --------------------------------------------------------------------------

def test_relation_orientation_and_phase_property_edits_move_the_real_token():
    recipe = WP.default_recipe("relation", 3)
    lattice = WP.prepare_v2(recipe).lattice
    assert np.count_nonzero(lattice.sc != S.BLANK_IDX) == 1
    assert np.count_nonzero(lattice.fcc != S.BLANK_IDX) == 0

    edited = deepcopy(recipe)
    edited["components"][0]["parameters"]["orientation"] = 5  # fcc plane1/diag0
    lattice2 = WP.prepare_v2(edited).lattice
    assert np.count_nonzero(lattice2.sc != S.BLANK_IDX) == 0
    assert np.count_nonzero(lattice2.fcc != S.BLANK_IDX) == 1

    edited2 = deepcopy(recipe)
    edited2["components"][0]["parameters"]["phase"] = 3
    edited2["components"][0]["parameters"]["polarity"] = -1
    lattice3 = WP.prepare_v2(edited2).lattice
    owner = np.flatnonzero((lattice3.sc[:, 0, :] != S.BLANK_IDX).any(axis=-1))[0]
    assert lattice3.sc[owner, 0, 1] == S.idx_of(encode(3, -1))


def test_field_channel_list_and_occupied_toggle_are_wired():
    recipe = WP.default_recipe("seam", 3)
    lattice = WP.prepare_v2(recipe).lattice
    owner = 26  # (2,2,2) in a 3^3 lattice
    assert lattice.bank[owner, [0, 34, 194]].all()
    assert lattice.bank.sum() == 3

    edited = deepcopy(recipe)
    field_list = next(c for c in edited["components"] if c["kind"] == "field_list")
    field_list["parameters"]["channels"] = [1, 2, 3, 4]
    lattice2 = WP.prepare_v2(edited).lattice
    assert lattice2.bank[owner, [1, 2, 3, 4]].all()
    assert lattice2.bank.sum() == 4

    edited["components"][-1]["parameters"]["occupied"] = False
    lattice3 = WP.prepare_v2(edited).lattice
    assert not lattice3.bank.any()


def test_manifestation_and_collision_scalar_edits_are_wired():
    recipe = WP.default_recipe("witness", 9)
    edited = deepcopy(recipe)
    edited["components"].append({"id": "manifest", "kind": "manifestation", "enabled": True,
                                 "region": {"shape": "point", "x": 4, "y": 4, "z": 4, "dx": 1, "dy": 1, "dz": 1, "radius": 0.0},
                                 "parameters": {"value": -1}})
    edited["components"].append({"id": "collide", "kind": "collision", "enabled": True,
                                 "region": {"shape": "point", "x": 4, "y": 4, "z": 4, "dx": 1, "dy": 1, "dz": 1, "radius": 0.0},
                                 "parameters": {"value": 2}})
    lattice = WP.prepare_v2(edited).lattice
    from phi_v2_lattice.geometry import site_index
    i = site_index(9, 4, 4, 4)
    assert lattice.s[i] == -1 and lattice.ell[i] == 2


def test_index_pattern_modulus_and_base_produce_the_real_periodic_pattern():
    recipe = WP.default_recipe("seam", 4)
    lattice = WP.prepare_v2(recipe).lattice
    n = 4 ** 3
    assert np.array_equal(lattice.s, (np.arange(n) % 3 - 1).astype(np.int8))
    assert np.array_equal(lattice.ell, (np.arange(n) % 3).astype(np.int8))

    edited = deepcopy(recipe)
    manifestation_pattern = next(c for c in edited["components"] if c["kind"] == "index_pattern"
                                 and c["parameters"]["field"] == "manifestation")
    manifestation_pattern["parameters"]["modulus"] = 2
    manifestation_pattern["parameters"]["base"] = -1
    lattice2 = WP.prepare_v2(edited).lattice
    assert np.array_equal(lattice2.s, (np.arange(n) % 2 - 1).astype(np.int8))


def test_index_pattern_rejects_values_outside_the_target_alphabet():
    recipe = WP.default_recipe("seam", 3)
    edited = deepcopy(recipe)
    pattern = next(c for c in edited["components"] if c["kind"] == "index_pattern"
                   and c["parameters"]["field"] == "manifestation")
    pattern["parameters"]["modulus"] = 4096
    pattern["parameters"]["base"] = 0
    with pytest.raises(ValueError):
        WP.prepare_v2(edited)


# --------------------------------------------------------------------------
# Mixed background / defect / probe: exact flags and slots, not just counts
# --------------------------------------------------------------------------

@pytest.mark.parametrize("scenario_id,defect,probe", [
    ("mixed_00", "reference", False), ("mixed_01", "reference", True),
    ("mixed_02", "sc_phase", False), ("mixed_03", "sc_phase", True),
    ("mixed_04", "sc_slot", False), ("mixed_05", "sc_slot", True),
    ("mixed_06", "fcc_phase", False), ("mixed_07", "fcc_phase", True),
    ("mixed_08", "fcc_slot", False), ("mixed_09", "fcc_slot", True),
])
def test_mixed_response_defect_and_probe_flags_match_the_frozen_case(scenario_id, defect, probe):
    recipe = WP.default_recipe(scenario_id, 17)
    lattice = WP.prepare_v2(recipe).lattice
    from phi_v2_lattice.geometry import site_index
    owner = site_index(17, 8, 8, 8)
    token = S.idx_of(encode(0, 1))
    if defect == "reference":
        assert lattice.sc[owner, 0, 0] == token and lattice.sc[owner, 0, 1] == S.BLANK_IDX
        assert lattice.fcc[owner, 0, 1, 0] == token and lattice.fcc[owner, 0, 1, 1] == S.BLANK_IDX
    elif defect == "sc_phase":
        assert lattice.sc[owner, 0, 0] == S.idx_of(encode(1, 1))
    elif defect == "sc_slot":
        assert lattice.sc[owner, 0, 0] == S.BLANK_IDX and lattice.sc[owner, 0, 1] == token
    elif defect == "fcc_phase":
        assert lattice.fcc[owner, 0, 1, 0] == S.idx_of(encode(1, 1))
    elif defect == "fcc_slot":
        assert lattice.fcc[owner, 0, 1, 0] == S.BLANK_IDX and lattice.fcc[owner, 0, 1, 1] == token
    # background must be untouched everywhere except the one defect owner
    other = 0 if owner != 0 else 1
    assert lattice.sc[other, 0, 0] == token and lattice.sc[other, 0, 1] == S.BLANK_IDX
    if probe:
        assert lattice.bank.any()
    else:
        assert not lattice.bank.any()


def test_mixed_scattering_probe_uses_the_exact_registered_channel_pair():
    recipe = WP.default_recipe("scatter_01", 17)
    lattice = WP.prepare_v2(recipe).lattice
    assert lattice.bank[:, [0, 32]].sum() == 27 * 2
    assert lattice.bank.sum() == 27 * 2


# --------------------------------------------------------------------------
# Geometry / numerics
# --------------------------------------------------------------------------

def test_carrier_placement_and_encounter_separated_geometry():
    recipe = WP.default_recipe("carrier_0288", 9)  # first field-family case
    lattice = WP.prepare_v2(recipe).lattice
    assert lattice.bank.sum() == 1
    recipe_sep = WP.default_recipe("carrier_0504", 9)  # a separated-family case
    lattice_sep = WP.prepare_v2(recipe_sep).lattice
    assert lattice_sep.bank.sum() == 2
    owners = np.flatnonzero(lattice_sep.bank.any(axis=1))
    assert len(owners) == 2 and owners[0] != owners[1]


@pytest.mark.parametrize("shape,origin,extent,radius,count", [
    ("all", (0, 0, 0), (1, 1, 1), 1, 27),
    ("box", (1, 0, 0), (2, 3, 1), 1, 6),
    ("sphere", (1, 1, 1), (1, 1, 1), 1, 7),
    ("sphere", (0, 0, 0), (1, 1, 1), 0, 1),
])
def test_field_list_masks_have_independent_geometric_counts(shape, origin, extent, radius, count):
    region = {"shape": shape, "x": origin[0], "y": origin[1], "z": origin[2],
              "dx": extent[0], "dy": extent[1], "dz": extent[2], "radius": radius}
    recipe = {"version": 2, "scenarioId": "record-empty", "size": 3, "blank": True, "randomSeed": 0,
              "components": [{"id": "c", "kind": "field_list", "enabled": True, "region": region,
                              "parameters": {"channels": [7], "occupied": True}}]}
    assert WP.prepare_v2(recipe).lattice.bank.sum() == count


# --------------------------------------------------------------------------
# Randomness: seed reproducibility, independence of unrelated components
# --------------------------------------------------------------------------

def test_field_random_component_is_repeatable_and_seed_sensitive():
    recipe = WP.default_recipe("r5", 4)
    lattice1 = WP.prepare_v2(deepcopy(recipe)).lattice
    lattice2 = WP.prepare_v2(deepcopy(recipe)).lattice
    assert np.array_equal(lattice1.bank, lattice2.bank)

    edited = deepcopy(recipe)
    field_random = next(c for c in edited["components"] if c["kind"] == "field_random")
    field_random["parameters"]["seed"] = 99
    lattice3 = WP.prepare_v2(edited).lattice
    assert not np.array_equal(lattice1.bank, lattice3.bank)


def test_sparse_tokens_component_is_repeatable_and_preserves_original_rng_sequence():
    recipe = WP.default_recipe("sparse", 3)
    lattice1 = WP.prepare_v2(deepcopy(recipe)).lattice
    lattice2 = WP.prepare_v2(deepcopy(recipe)).lattice
    assert np.array_equal(lattice1.sc, lattice2.sc) and np.array_equal(lattice1.bank, lattice2.bank)
    want, _ = W.checkpoint("sparse", 3)
    assert N.encode(WP.prepare_v2(recipe)) == want

    edited = deepcopy(recipe)
    edited["components"][0]["parameters"]["seed"] = 12
    lattice3 = WP.prepare_v2(edited).lattice
    assert not (np.array_equal(lattice1.sc, lattice3.sc) and np.array_equal(lattice1.fcc, lattice3.fcc))


def test_field_density_stream_is_component_scoped_and_reroll_free():
    recipe = {"version": 2, "scenarioId": "record-empty", "size": 9, "blank": True, "randomSeed": 5,
              "components": [{"id": "a", "kind": "field", "enabled": True,
                              "region": WP._all(9), "parameters": {"channel": 0, "occupied": True, "density": 0.4}}]}
    baseline = WP.prepare_v2(deepcopy(recipe)).lattice.bank.copy()
    with_extra = deepcopy(recipe)
    with_extra["components"].append({"id": "b", "kind": "collision", "enabled": True,
                                     "region": WP._all(9), "parameters": {"value": 1}})
    assert np.array_equal(WP.prepare_v2(with_extra).lattice.bank, baseline)
    reseeded = deepcopy(recipe)
    reseeded["randomSeed"] = 6
    assert not np.array_equal(WP.prepare_v2(reseeded).lattice.bank, baseline)


# --------------------------------------------------------------------------
# Invalid bounds / rejection gates
# --------------------------------------------------------------------------

@pytest.mark.parametrize("mutate", [
    lambda r: r.update(version=1),
    lambda r: r.update(size=18),
    lambda r: r.update(randomSeed=-1),
    lambda r: r.update(scenarioId="record-missing"),
    lambda r: r["components"][0].update(kind="unknown"),
    lambda r: r["components"][0]["parameters"].update(channel=999),
    lambda r: r["components"][0]["parameters"].update(density=float("nan")),
    lambda r: r["components"][0]["parameters"].update(occupied=1),
])
def test_invalid_field_component_requests_are_rejected(mutate):
    recipe = WP.default_recipe("witness", 9)
    mutate(recipe)
    with pytest.raises(ValueError):
        WP.prepare_v2(recipe)


@pytest.mark.parametrize("mutate", [
    lambda p: p.update(target="unknown"),
    lambda p: p.update(slots=[]),
    lambda p: p.update(slots=[0, 0]),
    lambda p: p.update(polarity=0),
])
def test_relation_background_rejects_invalid_parameters(mutate):
    recipe = WP.default_recipe("r5", 3)
    background = next(c for c in recipe["components"] if c["kind"] == "relation_background")
    mutate(background["parameters"])
    with pytest.raises(ValueError):
        WP.prepare_v2(recipe)


def test_sparse_tokens_rejects_more_tokens_than_available_slots():
    recipe = WP.default_recipe("sparse", 3)
    recipe["components"][0]["region"] = {"shape": "point", "x": 0, "y": 0, "z": 0,
                                         "dx": 1, "dy": 1, "dz": 1, "radius": 0.0}
    recipe["components"][0]["parameters"]["nTokens"] = 10  # only 9 slots at one site
    with pytest.raises(ValueError):
        WP.prepare_v2(recipe)


def test_index_pattern_rejects_unknown_target_field():
    recipe = WP.default_recipe("seam", 3)
    pattern = next(c for c in recipe["components"] if c["kind"] == "index_pattern")
    pattern["parameters"]["field"] = "bank"
    with pytest.raises(ValueError):
        WP.prepare_v2(recipe)


def test_component_budget_and_duplicate_ids_are_rejected():
    recipe = WP.default_recipe("witness", 9)
    component = recipe["components"][0]
    recipe["components"] = [deepcopy(component) for _ in range(65)]
    with pytest.raises(ValueError):
        WP.prepare_v2(recipe)
    recipe["components"] = [component, deepcopy(component)]
    with pytest.raises(ValueError):
        WP.prepare_v2(recipe)


def test_blank_base_rejects_unsupported_size():
    with pytest.raises(ValueError):
        WP.blank_base(size=5)
    with pytest.raises(ValueError):
        WP.describe_base(size=5)
