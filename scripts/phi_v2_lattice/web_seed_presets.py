"""Editable v2 constructor decomposition of every registered finite preset.

v1 (``web_seeding.py``) applies components as overrides on top of an opaque
checkpoint call to the frozen ``web_scenarios.prepare``. This module instead
rebuilds each of the 629 registered preparations, at all 656 registered
size combinations, as an ordered list of real, editable components whose
parameters are the actual arguments the frozen constructors use (owner
coordinates, orientation/slot/phase/polarity, RNG seeds, token counts,
occupation densities, channel lists, collision layers, index patterns). No
change is made to the frozen builders in ``prepare.py``, ``recovery_carriers.py``,
``recovery_mixed_response.py``, ``recovery_mixed_scattering.py`` or
``web_scenarios.py``; this module only reads their registered case tables and
reproduces their exact math through a small, general, and separately editable
component vocabulary. Byte-for-byte parity with the frozen checkpoints is
verified by ``scripts/tests/phi_v2_lattice/test_web_seed_presets.py``.
"""
from __future__ import annotations

from hashlib import sha256
import json
import numpy as np

from . import channels as C, geometry as G, state as S, staged as P, native_codec as N
from . import recovery_carriers as R, recovery_mixed_response as M, recovery_mixed_scattering as X
from . import web_scenarios as W
from ._proofs import encode
from .web_seeding import SIZES, MAX_COMPONENTS, keys, integer, number, mask

VERSION = 2
SCHEMA_IDENTITY = "finite-seed-2"

KINDS = ("field", "field_list", "field_random", "relation", "relation_background",
         "sparse_tokens", "manifestation", "collision", "index_pattern")

CORE_IDS = ("empty", "relation", "sparse", "r5", "seam", "witness-control",
            "witness", "expiry-a", "expiry-b")


# --------------------------------------------------------------------------
# Region helpers (same shape/keys web_seeding.mask() validates)
# --------------------------------------------------------------------------

def _all(size):
    return {"shape": "all", "x": 0, "y": 0, "z": 0, "dx": size, "dy": size, "dz": size, "radius": 0.0}


def _point(x, y, z):
    return {"shape": "point", "x": int(x), "y": int(y), "z": int(z), "dx": 1, "dy": 1, "dz": 1, "radius": 0.0}


def _box(x, y, z, dx, dy, dz):
    return {"shape": "box", "x": int(x), "y": int(y), "z": int(z),
            "dx": int(dx), "dy": int(dy), "dz": int(dz), "radius": 0.0}


# --------------------------------------------------------------------------
# Component parameter validation
# --------------------------------------------------------------------------

def _params(kind, p, size):
    if kind == "field":
        keys(p, ("channel", "occupied", "density"), kind)
        channel = integer(p["channel"], 0, C.N_CHANNELS - 1, "channel")
        if type(p["occupied"]) is not bool: raise ValueError("occupied must be Boolean")
        density = number(p["density"], 0, 1, "density")
        return channel, p["occupied"], density
    if kind == "field_list":
        keys(p, ("channels", "occupied"), kind)
        channels = p["channels"]
        if type(channels) is not list or not channels or len(channels) > C.N_CHANNELS:
            raise ValueError("channels: nonempty list of at most 384 channel indices required")
        if len(set(channels)) != len(channels):
            raise ValueError("channels: duplicate channel index")
        channels = [integer(c, 0, C.N_CHANNELS - 1, "channel") for c in channels]
        if type(p["occupied"]) is not bool: raise ValueError("occupied must be Boolean")
        return channels, p["occupied"]
    if kind == "field_random":
        keys(p, ("occupation", "polarity", "seed"), kind)
        occupation = number(p["occupation"], 0, 1, "occupation")
        polarity = p["polarity"]
        if polarity not in (-1, 1): raise ValueError("polarity must be -1 or +1")
        seed = integer(p["seed"], 0, 2 ** 32 - 1, "seed")
        return occupation, polarity, seed
    if kind == "relation":
        keys(p, ("orientation", "slot", "phase", "polarity", "occupied"), kind)
        orientation = integer(p["orientation"], 0, 8, "orientation")
        slot = integer(p["slot"], 0, 1, "slot")
        phase = integer(p["phase"], 0, 3, "phase")
        polarity = integer(p["polarity"], -1, 1, "polarity")
        if polarity == 0: raise ValueError("polarity must be -1 or +1")
        if type(p["occupied"]) is not bool: raise ValueError("occupied must be Boolean")
        return orientation, slot, phase, polarity, p["occupied"]
    if kind == "relation_background":
        keys(p, ("target", "slots", "phase", "polarity"), kind)
        target = p["target"]
        if target not in ("sc", "fcc", "both"): raise ValueError("target must be sc, fcc or both")
        slots = p["slots"]
        if (type(slots) is not list or not slots or any(s not in (0, 1) for s in slots)
                or len(set(slots)) != len(slots)):
            raise ValueError("slots: nonempty subset of {0,1}")
        phase = integer(p["phase"], 0, 3, "phase")
        polarity = integer(p["polarity"], -1, 1, "polarity")
        if polarity == 0: raise ValueError("polarity must be -1 or +1")
        return target, slots, phase, polarity
    if kind == "sparse_tokens":
        keys(p, ("seed", "nTokens", "fieldOccupation", "polarity"), kind)
        seed = integer(p["seed"], 0, 2 ** 32 - 1, "seed")
        n_tokens = integer(p["nTokens"], 0, size ** 3 * 9, "nTokens")
        field_occupation = number(p["fieldOccupation"], 0, 1, "fieldOccupation")
        polarity = p["polarity"]
        if polarity not in (-1, 1): raise ValueError("polarity must be -1 or +1")
        return seed, n_tokens, field_occupation, polarity
    if kind in ("manifestation", "collision"):
        keys(p, ("value",), kind)
        value = integer(p["value"], -1 if kind == "manifestation" else 0,
                        1 if kind == "manifestation" else 2, kind)
        return (value,)
    if kind == "index_pattern":
        keys(p, ("field", "modulus", "base"), kind)
        field = p["field"]
        if field not in ("manifestation", "collision"): raise ValueError("field must be manifestation or collision")
        modulus = integer(p["modulus"], 1, 4096, "modulus")
        base = integer(p["base"], -4096, 4096, "base")
        return field, modulus, base
    raise ValueError("Unknown component kind")


def _component_rng(seed, cid):
    rng_seed = int.from_bytes(sha256(f"{seed}:{cid}".encode()).digest()[:16], "little")
    return np.random.Generator(np.random.PCG64(rng_seed))


# --------------------------------------------------------------------------
# Component application (each kind mirrors the exact math of the frozen
# constructor it decomposes; parameters are the real constructor inputs)
# --------------------------------------------------------------------------

def _apply(lattice, size, selected, kind, params, seed, cid):
    if kind == "field":
        channel, occupied, density = params
        if density < 1:
            rng = _component_rng(seed, cid)
            sel = selected & (rng.random(size ** 3) < density)
        else:
            sel = selected
        lattice.bank[sel, channel] = occupied
    elif kind == "field_list":
        channels, occupied = params
        for channel in channels:
            lattice.bank[selected, channel] = occupied
    elif kind == "field_random":
        occupation, polarity, rseed = params
        rng = np.random.default_rng(rseed)
        lo = 0 if polarity > 0 else C.N_STATES
        idxs = np.flatnonzero(selected)
        if idxs.size:
            lattice.bank[idxs[:, None], lo + np.arange(C.N_STATES)] = (
                rng.random((idxs.size, C.N_STATES)) < occupation)
    elif kind == "relation":
        orientation, slot, phase, polarity, occupied = params
        value = S.idx_of(encode(phase, polarity)) if occupied else S.BLANK_IDX
        if orientation < 3:
            lattice.sc[selected, orientation, slot] = value
        else:
            plane, diagonal = divmod(orientation - 3, 2)
            lattice.fcc[selected, plane, diagonal, slot] = value
    elif kind == "relation_background":
        target, slots, phase, polarity = params
        value = S.idx_of(encode(phase, polarity))
        if target in ("sc", "both"):
            for axis in range(3):
                for slot in slots:
                    lattice.sc[selected, axis, slot] = value
        if target in ("fcc", "both"):
            for plane in range(3):
                for diagonal in range(2):
                    for slot in slots:
                        lattice.fcc[selected, plane, diagonal, slot] = value
    elif kind == "sparse_tokens":
        rseed, n_tokens, field_occupation, polarity = params
        rng = np.random.default_rng(rseed)
        idxs = np.flatnonzero(selected)
        slots = [("sc", i, (a,)) for i in idxs for a in range(3)] + \
                [("fcc", i, (p, q)) for i in idxs for p in range(3) for q in range(2)]
        if n_tokens > len(slots):
            raise ValueError("nTokens exceeds the relation slots available in the selected region")
        for k in rng.choice(len(slots), size=n_tokens, replace=False):
            kk, i, idx = slots[k]
            z = S.idx_of(encode(int(rng.integers(0, 4)), int(rng.choice([-1, 1]))))
            which = int(rng.integers(0, 2))
            if kk == "sc":
                lattice.sc[i, idx[0], which] = z
            else:
                lattice.fcc[i, idx[0], idx[1], which] = z
        if field_occupation > 0 and idxs.size:
            lo = 0 if polarity > 0 else C.N_STATES
            lattice.bank[idxs[:, None], lo + np.arange(C.N_STATES)] = (
                rng.random((idxs.size, C.N_STATES)) < field_occupation)
    elif kind in ("manifestation", "collision"):
        (value,) = params
        getattr(lattice, "s" if kind == "manifestation" else "ell")[selected] = value
    elif kind == "index_pattern":
        field, modulus, base = params
        idxs = np.flatnonzero(selected)
        if idxs.size:
            values = (idxs % modulus) + base
            lo, hi = (-1, 1) if field == "manifestation" else (0, 2)
            if values.min() < lo or values.max() > hi:
                raise ValueError("index_pattern values fall outside the target alphabet; adjust modulus/base")
            getattr(lattice, "s" if field == "manifestation" else "ell")[idxs] = values
    else:
        raise ValueError("Unknown component kind")


def prepare_v2(recipe):
    """Compile a v2 recipe into a StagedState by rebuilding every component
    from scratch: v2 never starts from a checkpoint override, only from
    ``state.blank`` plus its own ordered, editable components."""
    expected = ("version", "scenarioId", "size", "blank", "randomSeed", "components")
    if isinstance(recipe, dict) and "explicitOverrides" in recipe:
        expected += ("explicitOverrides",)
        paths = recipe["explicitOverrides"]
        if type(paths) is not list or len(paths) > 4096 or any(type(k) is not str or len(k) > 240 for k in paths):
            raise ValueError("Invalid explicit seed property identities")
    keys(recipe, expected, "recipe")
    integer(recipe["version"], 2, 2, "version")
    size = integer(recipe["size"], 3, 17, "size")
    if size not in SIZES: raise ValueError("Unsupported preparation size")
    if type(recipe["blank"]) is not bool: raise ValueError("blank must be Boolean")
    seed = integer(recipe["randomSeed"], 0, 2 ** 32 - 1, "randomSeed")
    scenario_id = recipe["scenarioId"]
    if type(scenario_id) is not str or not scenario_id.startswith("record-"):
        raise ValueError("A registered record parent is required")
    if scenario_id != "record-base":
        parent = next((r for r in W.scenarios() if "record-" + r.id == scenario_id), None)
        if parent is None: raise ValueError("Unknown preparation parent")
    components = recipe["components"]
    if type(components) is not list or len(components) > MAX_COMPONENTS:
        raise ValueError("At most 64 components are supported")
    lattice = S.blank(size)
    ids = set()
    for component in components:
        keys(component, ("id", "kind", "enabled", "region", "parameters"), "component")
        cid = component["id"]
        if type(cid) is not str or not 1 <= len(cid) <= 80 or cid in ids:
            raise ValueError("Component IDs must be unique nonempty strings of at most 80 characters")
        ids.add(cid)
        if type(component["enabled"]) is not bool: raise ValueError("enabled must be Boolean")
        kind = component["kind"]
        if kind not in KINDS: raise ValueError("Unknown component kind")
        selected = mask(component["region"], size)
        params = _params(kind, component["parameters"], size)
        if component["enabled"]:
            _apply(lattice, size, selected, kind, params, seed, cid)
    state = P.initialize(lattice)
    P.validate(state)
    return state


def checkpoint_v2(recipe):
    state = prepare_v2(recipe)
    data = N.encode(state)
    recipe_digest = sha256(json.dumps(recipe, sort_keys=True, separators=(",", ":"), allow_nan=False).encode()).hexdigest()
    return data, sha256(data).hexdigest(), recipe_digest


# --------------------------------------------------------------------------
# Frozen-constructor decomposition: exact reproduction of every registered
# preset's real inputs as ordered (kind, region, parameters, enabled) rows
# --------------------------------------------------------------------------

def _core_components(scenario_id, size):
    L = size
    if scenario_id == "empty":
        return []
    if scenario_id == "relation":
        owner = (L ** 3) // 2
        x, y, z = G.coords(L, owner)
        return [("relation", _point(x, y, z),
                 dict(orientation=0, slot=1, phase=0, polarity=1, occupied=True), True)]
    if scenario_id == "sparse":
        return [("sparse_tokens", _all(L),
                 dict(seed=11, nTokens=12, fieldOccupation=.005, polarity=1), True)]
    if scenario_id == "r5":
        return [
            ("relation_background", _all(L), dict(target="both", slots=[0, 1], phase=0, polarity=1), True),
            ("field_random", _all(L), dict(occupation=0.5, polarity=1, seed=11), True),
        ]
    if scenario_id == "seam":
        ox, oy, oz = L - 1, L - 1, L - 1
        return [
            ("index_pattern", _all(L), dict(field="manifestation", modulus=3, base=-1), True),
            ("index_pattern", _all(L), dict(field="collision", modulus=3, base=0), True),
            ("relation", _point(ox, oy, oz), dict(orientation=0, slot=1, phase=0, polarity=-1, occupied=True), True),
            ("relation", _point(ox, oy, oz), dict(orientation=7, slot=0, phase=0, polarity=1, occupied=True), True),
            ("relation", _point(ox, oy, oz), dict(orientation=8, slot=1, phase=0, polarity=-1, occupied=True), True),
            ("field_list", _point(ox, oy, oz), dict(channels=[0, 34, 194], occupied=True), True),
        ]
    if scenario_id in ("witness", "witness-control"):
        ox, oy, oz = 1, 1, 1
        shifted = G.shift(L, G.site_index(L, ox, oy, oz), (1, 0, 0))
        sx, sy, sz = G.coords(L, shifted)
        return [
            ("field_list", _point(ox, oy, oz), dict(channels=[0, 34], occupied=True), True),
            ("field_list", _point(sx, sy, sz), dict(channels=[2], occupied=True), scenario_id == "witness"),
        ]
    if scenario_id in ("expiry-a", "expiry-b"):
        channels = [c for c in range(192) if C.phase(c) == 2 and C.tangent(c) == (1, 0, 0)]
        channel = channels[int(scenario_id == "expiry-b")]
        return [("field_list", _point(0, 0, 0), dict(channels=[channel], occupied=True), True)]
    raise ValueError("unknown core preparation")


def _carrier_components(case):
    L = R.L
    x, y, z = R.PLACEMENTS[case.placement]
    collision = ("collision", _all(L), dict(value=case.layer), True)
    if case.family == "relation":
        rel = ("relation", _point(x, y, z),
               dict(orientation=case.orientation, slot=case.slot, phase=case.phase,
                    polarity=case.polarity, occupied=True), True)
        return [collision, rel]
    direction = R.DIRECTIONS[case.orientation]
    channel = R._channel(direction, case.phase, case.polarity)
    components = [collision, ("field_list", _point(x, y, z), dict(channels=[channel], occupied=True), True)]
    if case.family in ("encounter", "separated"):
        second_channel = R._channel(direction, 1, case.second_polarity)
        if case.family == "separated":
            axis = next(i for i, value in enumerate(direction) if value)
            offset = tuple(4 if i == axis else 0 for i in range(3))
            second_site = G.shift(L, G.site_index(L, x, y, z), offset)
            sx, sy, sz = G.coords(L, second_site)
        else:
            sx, sy, sz = x, y, z
        components.append(("field_list", _point(sx, sy, sz), dict(channels=[second_channel], occupied=True), True))
    return components


def _mixed_response_components(case):
    components = [("relation_background", _all(M.L), dict(target="both", slots=[0], phase=0, polarity=1), True)]
    if case.defect != "reference":
        orientation = 0 if case.defect.startswith("sc") else 4
        if case.defect.endswith("phase"):
            components.append(("relation", _point(8, 8, 8),
                               dict(orientation=orientation, slot=0, phase=1, polarity=1, occupied=True), True))
        else:
            components.append(("relation", _point(8, 8, 8),
                               dict(orientation=orientation, slot=0, phase=0, polarity=1, occupied=False), True))
            components.append(("relation", _point(8, 8, 8),
                               dict(orientation=orientation, slot=1, phase=0, polarity=1, occupied=True), True))
    if case.probe:
        channel = next(c for c in range(384) if C.polarity(c) == 1 and C.phase(c) == 0 and C.tangent(c) == (1, 0, 0))
        components.append(("field_list", _box(7, 7, 7, 3, 3, 3), dict(channels=[channel], occupied=True), True))
    return components


def _mixed_scattering_components(case):
    components = _mixed_response_components(M.MixedCase("reference-probe", case.defect, False))
    if case.probe:
        components.append(("field_list", _box(7, 7, 7, 3, 3, 3), dict(channels=[0, 32], occupied=True), True))
    return components


def _dispatch(scenario_id, size):
    if scenario_id in CORE_IDS:
        return _core_components(scenario_id, size)
    case = next((c for c in R.cases() if c.case_id == scenario_id), None)
    if case is not None:
        return _carrier_components(case)
    case = next((c for c in M.cases() if c.case_id == scenario_id), None)
    if case is not None:
        return _mixed_response_components(case)
    case = next((c for c in X.cases() if c.case_id == scenario_id), None)
    if case is not None:
        return _mixed_scattering_components(case)
    raise ValueError("unregistered preparation")


def default_recipe(scenario_id, size):
    """The v2 recipe that reproduces ``web_scenarios.checkpoint(scenario_id, size)``
    exactly, through this module's own ordered, editable, component decomposition."""
    parent = next((row for row in W.scenarios() if row.id == scenario_id), None)
    if parent is None or size not in parent.sizes:
        raise ValueError("unregistered preparation or unsupported size")
    rows = _dispatch(scenario_id, size)
    if len(rows) > MAX_COMPONENTS:
        raise ValueError("component budget exceeded")
    components = [{"id": f"{scenario_id}-{i}", "kind": kind, "enabled": enabled,
                   "region": region, "parameters": parameters}
                  for i, (kind, region, parameters, enabled) in enumerate(rows)]
    return {"version": VERSION, "scenarioId": "record-" + scenario_id, "size": size,
            "blank": False, "randomSeed": 0, "components": components}


def all_ids_and_sizes():
    """All 656 registered (scenario_id, size) combinations, in registry order."""
    return [(row.id, size) for row in W.scenarios() for size in row.sizes]


# --------------------------------------------------------------------------
# Property/schema descriptors
# --------------------------------------------------------------------------

_DEFAULT_PARAMETERS = {
    "field": dict(channel=0, occupied=True, density=1.0),
    "field_list": dict(channels=[0], occupied=True),
    "field_random": dict(occupation=0.5, polarity=1, seed=0),
    "relation": dict(orientation=0, slot=0, phase=0, polarity=1, occupied=True),
    "relation_background": dict(target="both", slots=[0, 1], phase=0, polarity=1),
    "sparse_tokens": dict(seed=0, nTokens=0, fieldOccupation=0.0, polarity=1),
    "manifestation": dict(value=0),
    "collision": dict(value=0),
    "index_pattern": dict(field="manifestation", modulus=3, base=-1),
}

# Scalar (real/integer/choice) constructor parameters. field_list.channels and
# relation_background.slots are list-valued and are described separately
# through COLLECTION_META, never as a property here (only real/integer/choice
# are supported property types).
PROPERTY_META = {
    ("field", "channel"): dict(label="Field channel", type="integer", min=0, max=C.N_CHANNELS - 1, step=1,
        description="Which of the 384 Boolean field-token channels this component writes. A channel is the complete "
                     "internal-state identity of a one-particle field token: an internal flag (tangent direction, "
                     "secondary axis, and a handedness sign) crossed with phase k in 0-3, crossed with polarity "
                     "eps in {+1,-1}; index c encodes state i=c mod 192 and eps=(c<192 ? +1 : -1)."),
    ("field", "occupied"): dict(label="Occupied", type="choice", options=[[False, "Off — cleared"], [True, "On — occupied"]],
        description="Sets (true) or clears (false) the channel at every selected site."),
    ("field", "density"): dict(label="Occupation density", type="real", min=0, max=1, step=0.001, units="probability",
        description="Fraction of selected sites (0-1) that receive the channel, drawn from a component-scoped random stream keyed on the recipe seed and this component's id. 1 is exact and deterministic (no draw)."),
    ("field_list", "occupied"): dict(label="Occupied", type="choice", options=[[False, "Off — cleared"], [True, "On — occupied"]],
        description="Sets (true) or clears (false) every listed channel at every selected site."),
    ("field_random", "occupation"): dict(label="Field occupation", type="real", min=0, max=1, step=0.001, units="probability",
        description="Probability (0-1) that each of the 192 one-particle field states at the chosen polarity is occupied at every selected site, matching prepare.py:r5_vacuum's background field fill."),
    ("field_random", "polarity"): dict(label="Polarity", type="choice", options=[[-1, "-1"], [1, "+1"]],
        description="Which polarity half (eps=+1 or eps=-1) of the 384-channel bank receives the random draw."),
    ("field_random", "seed"): dict(label="Random seed", type="integer", min=0, max=2 ** 32 - 1, step=1,
        description="Seed for this component's own numpy.random.default_rng stream, independent of the recipe-level seed."),
    ("relation", "orientation"): dict(label="Relation orientation", type="integer", min=0, max=8, step=1,
        description="Which of the nine per-site relations this token occupies: SC axes 0-2 (x,y,z), then FCC plane/diagonal pairs 3-8 (3=plane0/diag0, 4=plane0/diag1, ..., 8=plane2/diag1)."),
    ("relation", "slot"): dict(label="Slot", type="choice", options=[[0, "primary (lambda)"], [1, "reserve (rho)"]],
        description="Primary (0, lambda) or reserve (1, rho) relation slot at the chosen orientation."),
    ("relation", "phase"): dict(label="Phase", type="integer", min=0, max=3, step=1,
        description="Relation phase 0-3 of the encoded token."),
    ("relation", "polarity"): dict(label="Polarity", type="choice", options=[[-1, "-1"], [1, "+1"]],
        description="Relation polarity, +1 or -1, of the encoded token."),
    ("relation", "occupied"): dict(label="Occupied", type="choice", options=[[False, "Off — blank"], [True, "On — occupied"]],
        description="Whether the slot holds the encoded phase/polarity token (true) or is cleared to blank (false)."),
    ("relation_background", "target"): dict(label="Relation family", type="choice",
        options=[["sc", "SC axes"], ["fcc", "FCC planes"], ["both", "SC and FCC"]],
        description="Which relation family this uniform background fills: the three SC axes, the three FCC planes, or both."),
    ("relation_background", "phase"): dict(label="Phase", type="integer", min=0, max=3, step=1,
        description="Phase 0-3 of the uniform background token written to every targeted slot."),
    ("relation_background", "polarity"): dict(label="Polarity", type="choice", options=[[-1, "-1"], [1, "+1"]],
        description="Polarity, +1 or -1, of the uniform background token."),
    ("sparse_tokens", "seed"): dict(label="Random seed", type="integer", min=0, max=2 ** 32 - 1, step=1,
        description="Seed for the single numpy.random.default_rng stream that governs both relation-token placement and the subsequent field fill, in that order, matching prepare.py:sparse_material exactly."),
    ("sparse_tokens", "nTokens"): dict(label="Token count", type="integer", min=0, step=1,
        description="Number of relation slots (3 SC axes + 6 FCC plane/diagonal slots per site) drawn without replacement from the selected region and set to a random phase/polarity token. The legal maximum is 9 times the number of sites in this component's own region, not the whole domain."),
    ("sparse_tokens", "fieldOccupation"): dict(label="Field occupation", type="real", min=0, max=1, step=0.001, units="probability",
        description="Probability (0-1) of occupying each field state at the chosen polarity after token placement, continuing the same random stream; 0 disables the field fill entirely."),
    ("sparse_tokens", "polarity"): dict(label="Field polarity", type="choice", options=[[-1, "-1"], [1, "+1"]],
        description="Which polarity half of the field bank receives the post-placement field fill, when fieldOccupation is greater than 0."),
    ("manifestation", "value"): dict(label="Manifestation", type="integer", min=-1, max=1, step=1,
        description="Stored ternary manifestation s in {-1,0,+1} written at every selected site."),
    ("collision", "value"): dict(label="Collision layer", type="integer", min=0, max=2, step=1,
        description="Collision layer ell in {0,1,2} written at every selected site."),
    ("index_pattern", "field"): dict(label="Target field", type="choice",
        options=[["manifestation", "manifestation s"], ["collision", "collision layer ell"]],
        description="Whether this deterministic linear-index pattern writes the stored manifestation or the collision layer."),
    ("index_pattern", "modulus"): dict(label="Modulus", type="integer", min=1, max=4096, step=1,
        description="Period of the linear-index pattern value(i) = (i mod modulus) + base, applied over the selected sites' absolute lattice index i."),
    ("index_pattern", "base"): dict(label="Base offset", type="integer", min=-4096, max=4096, step=1,
        description="Additive offset applied after the modulus reduction; must keep every produced value inside the target field's alphabet."),
}

# List-valued constructor parameters: described as flattened integer/choice
# row properties (one per current entry) plus this template, never as a
# single "integer_list" property (only real/integer/choice are supported).
COLLECTION_META = {
    ("field_list", "channels"): dict(itemType="integer", itemLabel="Channel", min=0, max=C.N_CHANNELS - 1, step=1,
        defaultRow=0, minRows=1, maxRows=C.N_CHANNELS,
        description="Explicit list of field-token channel indices (0-383) set or cleared together at every selected site, with no random draw.",
        recommendationBasis="codec legal alphabet: any of the 384 channel indices"),
    ("relation_background", "slots"): dict(itemType="choice", itemLabel="Slot",
        options=[[0, "primary (lambda)"], [1, "reserve (rho)"]], min=0, max=1, step=1,
        defaultRow=0, minRows=1, maxRows=2,
        description="Which relation slots (0=primary/lambda, 1=reserve/rho) receive the uniform background token; both is the fully occupied R5 background.",
        recommendationBasis="codec legal alphabet: primary slot, reserve slot, or both"),
}

REGION_META = {
    "shape": dict(label="Region shape", type="choice",
        options=[["all", "entire domain"], ["point", "single site"], ["box", "axis-aligned box"], ["sphere", "sphere"]],
        description="Geometric selection applied before this component writes: the entire periodic domain, a single site, an axis-aligned box, or a sphere."),
    "x": dict(label="Origin x", type="integer", step=1, units="lattice sites",
        description="Region origin or point x coordinate, in the finite codec's site-index order. Unused when shape=all."),
    "y": dict(label="Origin y", type="integer", step=1, units="lattice sites",
        description="Region origin or point y coordinate. Unused when shape=all."),
    "z": dict(label="Origin z", type="integer", step=1, units="lattice sites",
        description="Region origin or point z coordinate. Unused when shape=all."),
    "dx": dict(label="Extent x", type="integer", step=1, units="lattice sites",
        description="Box extent along x, measured from the origin; used only when shape=box."),
    "dy": dict(label="Extent y", type="integer", step=1, units="lattice sites",
        description="Box extent along y, measured from the origin; used only when shape=box."),
    "dz": dict(label="Extent z", type="integer", step=1, units="lattice sites",
        description="Box extent along z, measured from the origin; used only when shape=box."),
    "radius": dict(label="Radius", type="real", step=0.1, units="lattice units",
        description="Sphere radius in lattice units, centered on x,y,z; used only when shape=sphere."),
}

# Legal per-key bounds as a function of lattice size L, matching web_seeding.mask.
_REGION_BOUNDS = {
    "x": lambda L: (0, L - 1), "y": lambda L: (0, L - 1), "z": lambda L: (0, L - 1),
    "dx": lambda L: (1, L), "dy": lambda L: (1, L), "dz": lambda L: (1, L),
    "radius": lambda L: (0, L),
}


def _anchor_ranges():
    """Real observed numeric ranges for every (kind, key) and region key,
    scanned once across all 629 registered presets' actual constructor
    inputs. Backs the 'exploratory preset anchored range' recommendations;
    never invents a number, only reports what the frozen registry uses."""
    ranges = {}

    def _fold(bucket, value):
        if isinstance(value, bool) or not isinstance(value, (int, float)):
            return
        lo, hi = ranges.get(bucket, (value, value))
        ranges[bucket] = (min(lo, value), max(hi, value))

    for scenario_id, size in all_ids_and_sizes():
        for kind, region, parameters, _enabled in _dispatch(scenario_id, size):
            for key, value in parameters.items():
                _fold((kind, key), value)
            for key in ("x", "y", "z", "dx", "dy", "dz", "radius"):
                _fold(("region", key), region[key])
    return ranges


_ANCHOR_RANGES = _anchor_ranges()


def _choice_bounds(options):
    """Finite numeric [min, max] for a choice property's options, regardless
    of whether its option keys are booleans, integers, or strings (assertPropertyDescriptor
    requires Number.isFinite(min) and max even for options-validated properties)."""
    keys_ = [o[0] for o in options]
    if all(isinstance(k, bool) for k in keys_):
        return 0, 1
    if all(isinstance(k, int) and not isinstance(k, bool) for k in keys_):
        return min(keys_), max(keys_)
    return 0, len(options) - 1


def _recommended(low, high, anchor):
    """A finite, advisory [lo, hi] within [low, high] plus its basis: a real
    observed anchor from the registered presets when one exists, otherwise
    the full codec-legal range."""
    if anchor is not None:
        a_lo, a_hi = anchor
        lo, hi = max(low, a_lo), min(high, a_hi)
        if lo <= hi:
            basis = ("exploratory preset anchored range: single observed value across the 629 registered presets"
                      if lo == hi else
                      "exploratory preset anchored range: observed span across the 629 registered presets")
            return [lo, hi], basis
    return [low, high], "codec legal alphabet: full legal range"


def _prop(path, label, description, group, type_, value, default, low, high, step, recommended, basis,
          binding, options=None, units="", dependencies=None, active=None):
    key = ".".join(str(p) for p in path)
    prop = {"key": key, "label": label, "description": description, "group": group, "type": type_,
            "units": units, "default": default, "value": value, "min": low, "max": high, "step": step,
            "recommended": list(recommended), "recommendationBasis": basis, "binding": binding,
            "options": list(options or []), "path": list(path)}
    if dependencies:
        prop["dependencies"] = list(dependencies)
    if active is not None:
        prop["active"] = bool(active)
    return prop


def _scalar_property(path, kind, key, value, default, group, size):
    meta = PROPERTY_META[(kind, key)]
    low, high = meta.get("min"), meta.get("max")
    options = meta.get("options")
    if options:
        low, high = _choice_bounds(options)
    # nTokens has no static max: its legal bound is the caller's region
    # capacity, patched onto the returned property (max/recommended) below.
    recommended, basis = ([0, max(0, value)], "region capacity: patched by caller") if high is None \
        else _recommended(low, high, _ANCHOR_RANGES.get((kind, key)))
    return _prop(path, meta["label"], meta["description"], group, meta["type"], value, default,
                 low, high if high is not None else max(0, value), meta.get("step", 1 if meta["type"] != "real" else 0.001),
                 recommended, basis, f"{kind} constructor", options=options, units=meta.get("units", ""))


def _collection_rows(path_prefix, kind, key, values, default_values, group):
    """Flattened integer/choice row properties for a list-valued parameter,
    plus its collections template. Never a bare 'integer_list' property."""
    meta = COLLECTION_META[(kind, key)]
    low, high = meta["min"], meta["max"]
    options = meta.get("options")
    properties = []
    for row_index, value in enumerate(values):
        row_default = default_values[row_index] if row_index < len(default_values) else meta["defaultRow"]
        recommended, basis = _recommended(low, high, None)
        properties.append(_prop(list(path_prefix) + [row_index], f"{meta['itemLabel']} {row_index + 1}",
            f"Row {row_index + 1} of {len(values)} in the {meta['itemLabel'].lower()} list. {meta['description']}",
            group, meta["itemType"], value, row_default, low, high, meta.get("step", 1),
            recommended, meta["recommendationBasis"], f"{kind} constructor row", options=options))
    collection = {"path": list(path_prefix), "itemType": meta["itemType"], "min": low, "max": high,
                  "options": list(options or []), "step": meta.get("step", 1), "minRows": meta["minRows"],
                  "maxRows": meta["maxRows"], "defaultRow": meta["defaultRow"], "rows": len(values),
                  "label": meta["itemLabel"], "description": meta["description"],
                  "recommendationBasis": meta["recommendationBasis"]}
    return properties, collection


def _default_component_map(recipe):
    """id -> component from the matching frozen preset default, so edited
    recipes never let 'default' drift to the live edited value. Falls back
    to no match (new/renamed component) for ids the preset never declared."""
    scenario_id = recipe.get("scenarioId")
    size = recipe.get("size")
    if not isinstance(scenario_id, str) or not scenario_id.startswith("record-") or size not in SIZES:
        return {}
    base_id = scenario_id[len("record-"):]
    try:
        source = blank_base(size) if base_id == "base" or recipe.get("blank") else default_recipe(base_id, size)
    except ValueError:
        return {}
    return {c["id"]: c for c in source["components"]}


def _recipe_properties(recipe):
    size = recipe["size"]
    default_components = _default_component_map(recipe)
    properties = [
        _prop(["size"], "Lattice size",
              "Side length L of the periodic L^3 domain this preparation compiles into; only these five sizes have a registered finite codec.",
              "Domain", "choice", size, size, min(SIZES), max(SIZES), 1, [min(SIZES), max(SIZES)],
              "codec legal alphabet: the five registered lattice sizes", "state.lattice.L",
              options=[[s, str(s)] for s in sorted(SIZES)]),
        _prop(["randomSeed"], "Recipe random seed",
              "Seed available to density-based field components (parameters.density < 1) that derive a component-scoped random stream from this value and the component's id. Components with their own explicit seed parameter (field_random, sparse_tokens) ignore this field.",
              "Randomness", "integer", recipe["randomSeed"], 0, 0, 2 ** 32 - 1, 1,
              *_recommended(0, 2 ** 32 - 1, None), "component RNG derivation"),
    ]
    collections = []
    labels = _ingredient_labels(recipe)
    for i, component in enumerate(recipe["components"]):
        kind = component["kind"]
        group = f"Ingredient {i + 1}: {labels[i]}"
        default_component = default_components.get(component.get("id")) or component_template(kind, size, component["id"])
        default_enabled = default_component["enabled"] if default_component else False
        enabled_options = [[False, "Off — disabled"], [True, "On — enabled"]]
        enabled_low, enabled_high = _choice_bounds(enabled_options)
        properties.append(_prop(["components", i, "enabled"], "Enabled",
            "Whether this component contributes to the compiled record; disabled components stay in the recipe but write nothing.",
            group, "choice", component["enabled"], default_enabled, enabled_low, enabled_high, 1,
            [enabled_low, enabled_high], "codec legal alphabet: on or off", "component activation",
            options=enabled_options))

        shape = component["region"]["shape"]
        shape_key = f"components.{i}.region.shape"
        default_region = default_component["region"] if default_component else component["region"]
        for key, meta in REGION_META.items():
            if key == "shape":
                low, high = _choice_bounds(meta["options"])
                options = meta["options"]
                recommended, basis = [low, high], "codec legal alphabet: any of the four region shapes"
                active = True
                deps = None
            else:
                low, high = _REGION_BOUNDS[key](size)
                options = None
                recommended, basis = _recommended(low, high, _ANCHOR_RANGES.get(("region", key)))
                active = shape != "all" if key in ("x", "y", "z") else (
                    shape == "box" if key in ("dx", "dy", "dz") else shape == "sphere")
                deps = [shape_key]
            properties.append(_prop(["components", i, "region", key], meta["label"], meta["description"],
                group + " geometry", meta["type"], component["region"][key], default_region.get(key, component["region"][key]),
                low, high, meta.get("step", 1), recommended, basis, "region mask",
                options=options, units=meta.get("units", ""), dependencies=deps, active=active))

        default_parameters = default_component["parameters"] if default_component else {}
        for key, value in component["parameters"].items():
            if (kind, key) in COLLECTION_META:
                default_values = default_parameters.get(key, [])
                rows, collection = _collection_rows(["components", i, "parameters", key], kind, key, value,
                                                     default_values, group)
                properties.extend(rows)
                collections.append(collection)
                continue
            default_value = default_parameters.get(key, _DEFAULT_PARAMETERS[kind][key])
            prop = _scalar_property(["components", i, "parameters", key], kind, key, value, default_value, group, size)
            if key == "nTokens":
                try:
                    capacity = int(mask(component["region"], size).sum()) * 9
                except ValueError:
                    capacity = size ** 3 * 9
                prop["max"] = capacity
                recommended, basis = _recommended(0, capacity, _ANCHOR_RANGES.get((kind, key)))
                prop["recommended"], prop["recommendationBasis"] = recommended, basis
            properties.append(prop)
    return properties, collections


def _ingredient_labels(recipe):
    names = {"field": "Field channel", "field_list": "Exact field channels", "field_random": "Random field background",
             "relation": "Relation token", "relation_background": "Relation background", "sparse_tokens": "Sparse population",
             "manifestation": "Stored manifestation", "collision": "Collision layer", "index_pattern": "Index pattern"}
    mixed = recipe["scenarioId"].startswith(("record-mixed_", "record-scatter_")) and not recipe["blank"]
    return [("Background — " if c["kind"] == "relation_background" else "Probe — " if c["kind"] == "field_list" else "Defect — ")
            + names[c["kind"]] if mixed else names[c["kind"]] for c in recipe["components"]]


def describe_recipe(recipe):
    """Descriptor for an arbitrary current, possibly edited, ordered recipe
    (not only a preset default): every property's 'value' reflects the live
    recipe, while 'default' is pinned to the matching frozen preset's own
    value (matched by component id), so edits never drift the default."""
    properties, collections = _recipe_properties(recipe)
    ingredients = [dict(id=c["id"], label=label, enabled=c["enabled"], operation="ordered record writes")
                   for c, label in zip(recipe["components"], _ingredient_labels(recipe))]
    return {"schemaVersion": 2, "scenarioId": recipe["scenarioId"], "recipe": recipe, "ingredients": ingredients,
            "properties": properties, "collections": collections, "schemaIdentity": SCHEMA_IDENTITY,
            "geometrySupport": geometry_support(recipe)}


def geometry_support(recipe):
    """Exact discretized mask support, before density/population selection.

    Spatial overlap alone is not a record collision: components may address
    different fields, channels or slots at the same site. Count it separately
    from the compiled owner's realized populations, without inferring writes.
    """
    seen = np.zeros(recipe['size'] ** 3, dtype=np.uint8)
    components = []
    for component, label in zip(recipe['components'], _ingredient_labels(recipe)):
        selected = mask(component['region'], recipe['size']).reshape(-1)
        active = selected if component['enabled'] else np.zeros_like(selected)
        components.append(dict(id=component['id'], label=label, enabled=component['enabled'],
            regionSites=int(selected.sum()), selectedSites=int(active.sum()),
            overlapWithEarlier=int(np.count_nonzero(active & (seen > 0)))))
        seen += active.astype(np.uint8)
    return dict(selectedSites=int(np.count_nonzero(seen)),
                overlappingSites=int(np.count_nonzero(seen > 1)), components=components,
                meaning='Discretized geometry before stochastic selection; spatial overlap does not imply a shared record address.')


def describe(scenario_id, size):
    return describe_recipe(default_recipe(scenario_id, size))


def blank_base(size=9):
    """A fresh blank v2 recipe exposing the union of every finite component
    kind, all inactive: the category-base escape hatch, not a named preset."""
    if size not in SIZES: raise ValueError("unsupported preparation size")
    components = [{"id": f"base-{kind}", "kind": kind, "enabled": False,
                   "region": _all(size), "parameters": dict(_DEFAULT_PARAMETERS[kind])}
                  for kind in KINDS]
    return {"version": VERSION, "scenarioId": "record-base", "size": size, "blank": True,
            "randomSeed": 0, "components": components}


def describe_base(size=9):
    description = describe_recipe(blank_base(size))
    description["schemaIdentity"] = SCHEMA_IDENTITY + "-base"
    return description


def component_template(kind, size, id):
    """A ready-to-insert, inactive component of the given kind at its
    canonical constructor defaults: the same per-kind template blank_base
    uses, exposed publicly for any legal size and caller-chosen id."""
    if kind not in KINDS: raise ValueError("Unknown component kind")
    if size not in SIZES: raise ValueError("unsupported preparation size")
    if type(id) is not str or not 1 <= len(id) <= 80:
        raise ValueError("Component id must be a nonempty string of at most 80 characters")
    return {"id": id, "kind": kind, "enabled": False, "region": _all(size),
            "parameters": dict(_DEFAULT_PARAMETERS[kind])}


def component_library():
    """Per-kind constructor default parameters and their property metadata,
    the single source an 'add component' UI reads instead of duplicating
    _DEFAULT_PARAMETERS/PROPERTY_META in JS."""
    library = {}
    for kind in KINDS:
        parameters = dict(_DEFAULT_PARAMETERS[kind])
        properties, collections = [], []
        for key, value in parameters.items():
            if (kind, key) in COLLECTION_META:
                meta = COLLECTION_META[(kind, key)]
                collections.append({"key": key, "itemType": meta["itemType"], "min": meta["min"], "max": meta["max"],
                                     "options": list(meta.get("options", [])), "step": meta.get("step", 1),
                                     "minRows": meta["minRows"], "maxRows": meta["maxRows"],
                                     "defaultRow": meta["defaultRow"], "label": meta["itemLabel"],
                                     "description": meta["description"],
                                     "recommendationBasis": meta["recommendationBasis"]})
                continue
            meta = PROPERTY_META[(kind, key)]
            low, high = meta.get("min"), meta.get("max")
            options = meta.get("options")
            basis_note = ""
            if options:
                low, high = _choice_bounds(options)
            elif high is None:
                # nTokens: no static ceiling, the real legal bound is the component's own
                # region capacity at use (9 relation slots per selected site); the template
                # reports the largest registered lattice's whole-domain capacity as a ceiling.
                high = max(SIZES) ** 3 * 9
                basis_note = " (the real ceiling is the component's own region capacity at use, not this template's)"
            recommended, basis = _recommended(low, high, _ANCHOR_RANGES.get((kind, key)))
            properties.append({"key": key, "label": meta["label"], "description": meta["description"],
                                "type": meta["type"], "units": meta.get("units", ""), "min": low, "max": high,
                                "step": meta.get("step", 1 if meta["type"] != "real" else 0.001),
                                "recommended": recommended, "recommendationBasis": basis + basis_note,
                                "options": list(options or [])})
        library[kind] = {"defaultParameters": parameters, "properties": properties, "collections": collections}
    return library


def all_component_templates():
    """component_template(kind, size, 'new-<kind>') for every legal size,
    keyed by size then kind, for a browser 'add component' picker."""
    return {size: {kind: component_template(kind, size, f"new-{kind}") for kind in KINDS} for size in SIZES}
