"""Bounded custom initial records for the dashboard; no changes to the law."""
from hashlib import sha256
import json
import math
import numpy as np
from . import state as S, staged as P, native_codec as N
from . import web_scenarios as W
from ._proofs import encode

SIZES = (3, 4, 7, 9, 17)
MAX_COMPONENTS = 64


def keys(value, expected, name):
    if type(value) is not dict or set(value) != set(expected):
        raise ValueError(f"{name}: unexpected or missing properties")


def integer(value, low, high, name):
    if type(value) is not int or not low <= value <= high:
        raise ValueError(f"{name}: integer in {low}..{high} required")
    return value


def number(value, low, high, name):
    if type(value) not in (float, int) or not low <= value <= high or not math.isfinite(value):
        raise ValueError(f"{name}: finite number in {low}..{high} required")
    return value


def mask(region, size):
    keys(region, ('shape', 'x', 'y', 'z', 'dx', 'dy', 'dz', 'radius'), 'region')
    shape = region['shape']
    if shape not in ('all', 'point', 'box', 'sphere'):
        raise ValueError('Unknown region shape')
    x, y, z = [integer(region[k], 0, size - 1, k) for k in ('x', 'y', 'z')]
    dx, dy, dz = [integer(region[k], 1, size, k) for k in ('dx', 'dy', 'dz')]
    radius = number(region['radius'], 0, size, 'radius')
    # The finite codec uses geometry.site_index: z fastest, then y, then x.
    xx, yy, zz = np.indices((size, size, size))
    if shape == 'all': selected = np.ones_like(xx, dtype=bool)
    elif shape == 'point': selected = (xx == x) & (yy == y) & (zz == z)
    elif shape == 'sphere': selected = (xx-x)**2 + (yy-y)**2 + (zz-z)**2 <= radius**2
    else:
        if x+dx > size or y+dy > size or z+dz > size:
            raise ValueError('Box extends outside the lattice; adjust its origin or extent')
        selected = (xx >= x) & (xx < x+dx) & (yy >= y) & (yy < y+dy) & (zz >= z) & (zz < z+dz)
    return selected.ravel()


def prepare(recipe):
    if isinstance(recipe, dict) and recipe.get('version') == 2:
        from .web_seed_presets import prepare_v2
        return prepare_v2(recipe)
    keys(recipe, ('version', 'scenarioId', 'size', 'blank', 'randomSeed', 'components'), 'recipe')
    integer(recipe['version'], 1, 1, 'version')
    size = integer(recipe['size'], 3, 17, 'size')
    if size not in SIZES: raise ValueError('Unsupported preparation size')
    if type(recipe['blank']) is not bool: raise ValueError('blank must be Boolean')
    seed = integer(recipe['randomSeed'], 0, 2**32-1, 'randomSeed')
    scenario_id = recipe['scenarioId']
    if type(scenario_id) is not str or not scenario_id.startswith('record-'):
        raise ValueError('A registered record parent is required')
    parent = next((r for r in W.scenarios() if 'record-' + r.id == scenario_id), None)
    if parent is None: raise ValueError('Unknown preparation parent')
    components = recipe['components']
    if type(components) is not list or len(components) > MAX_COMPONENTS:
        raise ValueError('At most 64 components are supported')
    state = P.initialize(S.blank(size)) if recipe['blank'] else W.prepare(parent.id, size)
    ids = set()
    for component in components:
        keys(component, ('id', 'kind', 'enabled', 'region', 'parameters'), 'component')
        cid = component['id']
        if type(cid) is not str or not 1 <= len(cid) <= 80 or cid in ids:
            raise ValueError('Component IDs must be unique nonempty strings of at most 80 characters')
        ids.add(cid)
        if type(component['enabled']) is not bool: raise ValueError('enabled must be Boolean')
        selected = mask(component['region'], size)
        p, kind = component['parameters'], component['kind']
        if kind == 'field':
            keys(p, ('channel', 'occupied', 'density'), kind)
            channel = integer(p['channel'], 0, 383, 'channel')
            if type(p['occupied']) is not bool: raise ValueError('occupied must be Boolean')
            density = number(p['density'], 0, 1, 'density')
            # Stable component streams; adding another component cannot reroll this one.
            rng_seed = int.from_bytes(sha256(f'{seed}:{cid}'.encode()).digest()[:16], 'little')
            rng = np.random.Generator(np.random.PCG64(rng_seed))
            selected &= rng.random(size**3) < density
            if component['enabled']: state.lattice.bank[selected, channel] = p['occupied']
        elif kind == 'relation':
            keys(p, ('orientation', 'slot', 'phase', 'polarity', 'occupied'), kind)
            orientation = integer(p['orientation'], 0, 8, 'orientation')
            slot = integer(p['slot'], 0, 1, 'slot')
            phase = integer(p['phase'], 0, 3, 'phase')
            polarity = integer(p['polarity'], -1, 1, 'polarity')
            if polarity == 0: raise ValueError('polarity must be -1 or +1')
            if type(p['occupied']) is not bool: raise ValueError('occupied must be Boolean')
            value = S.idx_of(encode(phase, polarity)) if p['occupied'] else S.BLANK_IDX
            if component['enabled']:
                if orientation < 3: state.lattice.sc[selected, orientation, slot] = value
                else:
                    plane, diagonal = divmod(orientation - 3, 2)
                    state.lattice.fcc[selected, plane, diagonal, slot] = value
        elif kind in ('manifestation', 'collision'):
            keys(p, ('value',), kind)
            value = integer(p['value'], -1 if kind == 'manifestation' else 0,
                            1 if kind == 'manifestation' else 2, kind)
            if component['enabled']:
                getattr(state.lattice, 's' if kind == 'manifestation' else 'ell')[selected] = value
        else:
            raise ValueError('Unknown component kind')
    P.validate(state)
    return state


def checkpoint(recipe):
    state = prepare(recipe)
    data = N.encode(state)
    recipe_digest = sha256(json.dumps(recipe, sort_keys=True, separators=(',', ':'), allow_nan=False).encode()).hexdigest()
    return data, sha256(data).hexdigest(), recipe_digest
