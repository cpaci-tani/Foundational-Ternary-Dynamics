"""Complete recorded backgrounds, genuine covariance and unchanged edge decisions."""
import base64
from dataclasses import replace
import hashlib
from itertools import permutations, product
import json
from pathlib import Path
import sys

import numpy as np
import pytest

from phi_v2_lattice import flux_binding as OLD
from phi_v2_lattice import recorded_matching as M


V = ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1))
PERMS = tuple(permutations(range(3)))
TRIADS = tuple(tuple(tuple(((-1 if color >> a & 1 else 1) if j == axes[a] else 0)
                           for j in range(3)) for a in range(3))
               for axes in PERMS for color in range(8))


def site(L, p):
    x, y, z = (int(v) % L for v in p)
    return (x * L + y) * L + z


def xyz(L, i):
    return i // (L * L), (i // L) % L, i % L


def add(x, v):
    return tuple(a + b for a, b in zip(x, v))


def blank(L=4, code=0):
    return M.initialize(L, np.zeros((L ** 3, 2), dtype=np.uint8),
                        np.zeros((L ** 3, 2), dtype=np.uint8),
                        np.zeros((L ** 3, 3), dtype=np.int8), M.seed_background(L, code))


def loop(state, origin=(0, 0, 0), a=0, b=1):
    x, va, vb = origin, V[2 * a], V[2 * b]
    state.flux[site(state.L, x), a] += 1
    state.flux[site(state.L, add(x, va)), b] += 1
    state.flux[site(state.L, add(x, vb)), a] -= 1
    state.flux[site(state.L, x), b] -= 1


def rich(code=0, L=4):
    state = blank(L, code)
    rng = np.random.default_rng(1729)
    occupied = rng.integers(0, 3, size=L ** 3) == 0
    state.direction[occupied] = rng.integers(1, 7, size=(int(occupied.sum()), 2), dtype=np.uint8)
    state.credit[occupied] = rng.integers(0, 2, size=(int(occupied.sum()), 2), dtype=np.uint8)
    loop(state)
    loop(state, (2, 2, 2), 1, 2)
    M.validate(state)
    return state


def same(left, right):
    assert left.L == right.L and left.microtick == right.microtick
    for name in M.NAMES:
        np.testing.assert_array_equal(getattr(left, name), getattr(right, name))


def pure_step(state):
    before = {name: getattr(state, name).tobytes() for name in M.NAMES}
    result, events = M.step(state)
    for name in M.NAMES:
        array = getattr(result, name)
        assert array.flags.owndata and array.flags.c_contiguous
        assert getattr(state, name).tobytes() == before[name]
        assert all(not np.shares_memory(array, getattr(state, old)) for old in M.NAMES)
        assert all(not np.shares_memory(array, getattr(result, other))
                   for other in M.NAMES if other != name)
    return result, events


def matrix_action(axes, signs):
    return tuple(tuple(signs[j] if k == axes[j] else 0 for k in range(3)) for j in range(3))


def vector(g, v):
    return tuple(sum(row[j] * v[j] for j in range(3)) for row in g)


ACTIONS = tuple(matrix_action(axes, signs) for axes in PERMS for signs in product((-1, 1), repeat=3))
IDENTITY = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
# Independent finite group action data, not imported from the runtime.
ACTION_DATA = {}
for g in ACTIONS:
    direction_map = (0,) + tuple(V.index(vector(g, v)) + 1 for v in V)
    background_map = tuple(TRIADS.index(tuple(vector(g, v) for v in triad)) for triad in TRIADS)
    edge_map = tuple((next(j for j, value in enumerate(vector(g, V[2 * a])) if value),
                      next(value for value in vector(g, V[2 * a]) if value)) for a in range(3))
    ACTION_DATA[g] = direction_map, background_map, edge_map


def transform(state, g=IDENTITY, offset=(0, 0, 0)):
    """Independent pullback of full arrays, including reversed flux ownership."""
    L = state.L
    coordinates = np.indices((L, L, L)).reshape(3, -1).T
    mapped = (coordinates @ np.asarray(g).T + np.asarray(offset)) % L
    destinations = (mapped[:, 0] * L + mapped[:, 1]) * L + mapped[:, 2]
    direction_map, background_map, edge_map = ACTION_DATA[g]
    direction = np.empty_like(state.direction)
    credit = np.empty_like(state.credit)
    flux = np.empty_like(state.flux)
    background = np.empty_like(state.matching_frame)
    direction[destinations] = np.asarray(direction_map, dtype=np.uint8)[state.direction]
    credit[destinations] = state.credit
    background[destinations] = np.asarray(background_map, dtype=np.uint8)[state.matching_frame]
    for old_axis, (new_axis, sign) in enumerate(edge_map):
        owners = mapped.copy()
        if sign < 0:
            owners[:, new_axis] = (owners[:, new_axis] - 1) % L
        targets = (owners[:, 0] * L + owners[:, 1]) * L + owners[:, 2]
        flux[targets, new_axis] = sign * state.flux[:, old_axis]
    return M.MatchingState(L, state.microtick, direction, credit, flux, background)


def events_transform(events, L, g=IDENTITY, offset=(0, 0, 0)):
    direction_map, _, edge_map = ACTION_DATA[g]
    def moved(index):
        return site(L, add(vector(g, xyz(L, index)), offset))
    def oriented(owner, axis):
        new_axis, sign = edge_map[axis]
        point = add(vector(g, xyz(L, owner)), offset)
        if sign < 0:
            point = add(point, V[2 * new_axis + 1])
        return site(L, point), new_axis, sign
    moves = []
    for epsilon, source, destination, owner, axis, q0, q1, k0, k1 in events.moves:
        tail, new_axis, sign = oriented(owner, axis)
        moves.append((epsilon, moved(source), moved(destination), tail, new_axis,
                      sign * q0, sign * q1, k0, k1))
    redirects = [(epsilon, moved(x), direction_map[v], direction_map[w], reason)
                 for epsilon, x, v, w, reason in events.redirects]
    holds = []
    for epsilon, owner, axis in events.capacity_holds:
        tail, new_axis, _ = oriented(owner, axis)
        holds.append((epsilon, tail, new_axis))
    return tuple(sorted(moves)), tuple(sorted(redirects)), tuple(sorted(holds))


def normalized(events):
    return tuple(sorted(events.moves)), tuple(sorted(events.redirects)), tuple(sorted(events.capacity_holds))


def test_frozen_reference_source_identity_and_immutable_tables():
    assert hashlib.sha256(Path(OLD.__file__).read_bytes()).hexdigest() == (
        "33203442187324ebf8d0f4c47668ffe0f3c5aa2d918284dab46fdb4aa6991740")
    assert M.TRIADS == TRIADS and len(set(TRIADS)) == 48
    assert M.FRAME_DIRECTIONS == tuple(tuple(V.index(v) + 1 for v in row) for row in TRIADS)
    assert M.FRAME_DIRECTIONS[0] == (1, 3, 5)
    with pytest.raises(TypeError):
        M.RULE_DESCRIPTION["law"] = "foreign"


@pytest.mark.parametrize("code", range(48))
def test_every_background_seed_both_neighbor_signs_and_twelve_matchings(code):
    L = 4
    background = M.seed_background(L, code)
    assert background.flags.owndata and background.dtype == np.uint8
    assert background[0] == code
    state = blank(L, code)
    for x in product(range(L), repeat=3):
        B = TRIADS[int(background[site(L, x)])]
        for a, sign in product(range(3), (-1, 1)):
            neighbor = add(x, tuple(sign * n for n in B[a]))
            other = TRIADS[int(background[site(L, neighbor)])]
            assert other == tuple(tuple(-n for n in v) if j == a else v for j, v in enumerate(B))
    for phase in range(12):
        state = replace(state, microtick=phase)
        M.validate(state)
        touched = []
        for x in product(range(L), repeat=3):
            a, b = (phase % 6) // 2, phase % 2
            u = tuple((-1 if b else 1) * v for v in TRIADS[int(background[site(L, x)])][a])
            y = add(x, u)
            by = TRIADS[int(background[site(L, y)])][a]
            assert tuple((-1 if b else 1) * v for v in by) == tuple(-v for v in u)
            if next(v for v in u if v) > 0:
                touched += [site(L, x), site(L, y)]
        assert sorted(touched) == list(range(L ** 3))
    background.fill(0)
    assert np.any(state.matching_frame)


@pytest.mark.parametrize("code", range(48))
def test_all_48_cubic_actions_all_phases_all_backgrounds_complete_state_and_events(code):
    # Exhausts the 48x48x12 transformation matrix on one fixed nonuniform
    # legal carrier fixture. It does not enumerate every global carrier state.
    base = rich(code)
    for phase in range(12):
        original = replace(base, microtick=phase)
        expected_base, base_events = M.step(original)
        for g in ACTIONS:
            acted = transform(original, g, (3, 1, 3))
            actual, events = M.step(acted)
            same(actual, transform(expected_base, g, (3, 1, 3)))
            assert normalized(events) == events_transform(base_events, 4, g, (3, 1, 3))


@pytest.mark.parametrize("code", range(48))
def test_genuine_unit_translations_all_phases_and_both_signs(code):
    base = rich(code)
    for phase in range(12):
        original = replace(base, microtick=phase)
        expected, recorded = M.step(original)
        for delta in V:
            translated = transform(original, offset=delta)
            assert not np.array_equal(translated.matching_frame, original.matching_frame)
            actual, events = M.step(translated)
            same(actual, transform(expected, offset=delta))
            assert normalized(events) == events_transform(recorded, 4, offset=delta)


def test_old_unit_translation_counterexample_is_repaired_by_recorded_background():
    old = OLD.initialize(4)
    old.direction[0] = (1, 2)
    old.credit[0] = (1, 1)
    state = M.initialize(4, old.direction, old.credit, old.flux, M.seed_background(4))
    output, events = pure_step(state)
    assert events.moves
    translated = transform(state, offset=(1, 0, 0))
    actual, moved_events = pure_step(translated)
    same(actual, transform(output, offset=(1, 0, 0)))
    assert moved_events.moves
    fixed_background = OLD.initialize(4, translated.direction, translated.credit, translated.flux)
    wrong, old_events = OLD.step(fixed_background)
    assert not old_events.moves
    assert not np.array_equal(wrong.direction, actual.direction)


@pytest.mark.parametrize("phase", range(12))
def test_canonical_background_matches_old_complete_records_events_and_account(phase):
    state = replace(rich(), microtick=phase)
    original = OLD.FluxState(4, phase, state.direction.copy(), state.credit.copy(), state.flux.copy())
    inventory, account = M.populations(state), M.account_units(state)
    initial_background = state.matching_frame.copy()
    for _ in range(24):
        original, old_events = OLD.step(original)
        state, events = pure_step(state)
        assert old_events == events
        for name in OLD.NAMES:
            np.testing.assert_array_equal(getattr(state, name), getattr(original, name))
        np.testing.assert_array_equal(state.matching_frame, initial_background)
        assert M.populations(state) == inventory and M.account_units(state) == account
        np.testing.assert_array_equal(M.manifestation(state), OLD.manifestation(original))


@pytest.mark.parametrize("phase", range(12))
@pytest.mark.parametrize("code", (0, 13, 47))
def test_all_phase_owned_checkpoint_replay_with_live_carriers(phase, code):
    state = replace(rich(code), microtick=phase)
    saved = M.checkpoint(state)
    restored = M.restore(saved)
    same(state, restored)
    assert M.checkpoint(restored) == saved
    for name in M.NAMES:
        assert getattr(restored, name).flags.owndata
        assert all(not np.shares_memory(getattr(restored, name), getattr(state, old)) for old in M.NAMES)
    for _ in range(24):
        state, le = pure_step(state)
        restored, re = pure_step(restored)
        same(state, restored)
        assert le == re and M.checkpoint(state) == M.checkpoint(restored)


@pytest.mark.parametrize("phase", range(12))
def test_huge_ordinals_with_no_global_decimal_limit_change(phase):
    previous_limit = sys.get_int_max_str_digits()
    state = replace(rich(47), microtick=12 * 10 ** 5000 + phase)
    saved = M.checkpoint(state)
    restored = M.restore(saved)
    assert restored.microtick == state.microtick
    output, events = M.step(restored)
    expected, expected_events = M.step(replace(state, microtick=phase))
    assert output.microtick == state.microtick + 1
    same(replace(output, microtick=phase + 1), expected)
    assert events == expected_events and M.checkpoint(restored) == saved
    assert sys.get_int_max_str_digits() == previous_limit


@pytest.mark.parametrize("code", range(48))
def test_legal_carrier_interventions_stay_within_radius_one_all_phases(code):
    for phase, family in product(range(12), OLD.NAMES):
        state = replace(blank(6, code), microtick=phase)
        active = phase // 6
        u = tuple((-1 if phase % 2 else 1) * v for v in TRIADS[code][(phase % 6) // 2])
        state.direction[0] = (1, 2)
        state.direction[0, active] = V.index(u) + 1
        state.credit[0] = (1, 1)
        changed = M.restore(M.checkpoint(state))
        if family == "direction":
            changed.direction[0, active] = (int(state.direction[0, active]) + 1) % 6 + 1
        elif family == "credit":
            changed.credit[0, active] = 0
        else:
            loop(changed)
        source_owners = set()
        for name in M.NAMES:
            delta = getattr(state, name) != getattr(changed, name)
            source_owners.update(map(int, np.flatnonzero(delta if delta.ndim == 1 else delta.any(axis=1))))
        allowed = {site(6, add(xyz(6, x), offset)) for x in source_owners
                   for offset in product((-1, 0, 1), repeat=3)}
        left, _ = M.step(state)
        right, _ = M.step(changed)
        for name in M.NAMES:
            delta = getattr(left, name) != getattr(right, name)
            changed_owners = set(map(int, np.flatnonzero(delta if delta.ndim == 1 else delta.any(axis=1))))
            assert changed_owners <= allowed
        assert M.account_units(left) == M.account_units(state)
        assert M.account_units(right) == M.account_units(changed)


def test_background_local_perturbations_are_illegal_not_vacuous_causality_evidence():
    # Coherent seeds differ at every site. A localized B intervention is
    # outside this legal domain; per-output B causality has a structural proof.
    seeds = [M.seed_background(4, code) for code in range(48)]
    for i, j in product(range(48), repeat=2):
        if i != j:
            assert np.all(seeds[i] != seeds[j])
    state = blank()
    for replacement in range(1, 48):
        changed = state.matching_frame.copy()
        changed[0] = replacement
        with pytest.raises(ValueError, match="neighbors"):
            M.validate(replace(state, matching_frame=changed))


def test_one_matching_consistency_is_insufficient_both_signs_are_required():
    # Two y slabs choose opposite x-matchings. The y arrows only connect
    # 0<->1 and 2<->3, so each followed arrow sees the right triad; the
    # intervening y=1<->2 edge changes an unrelated x column illegally.
    state = blank()
    background = state.matching_frame.reshape(4, 4, 4).copy()
    background[:, 2:4, :] ^= 1
    broken = background.reshape(-1).copy()
    for x in product(range(4), repeat=3):
        B = TRIADS[int(broken[site(4, x)])]
        for a in range(3):
            y = add(x, B[a])
            other = TRIADS[int(broken[site(4, y)])]
            assert other == tuple(tuple(-v for v in column) if j == a else column
                                  for j, column in enumerate(B))
    with pytest.raises(ValueError, match="both-sign"):
        M.validate(replace(state, matching_frame=broken))


@pytest.mark.parametrize("code", range(48))
def test_every_hop_class_under_all_backgrounds_phases_polarities_and_edge_directions(code):
    # Complete q/k/eta classes use legal neutral occupation and optional
    # plaquette flux. Expected outcomes use the three relative-flux classes.
    for phase, q, k, eta in product(range(12), (-1, 0, 1), (0, 1), (-1, 1)):
        state = replace(blank(4, code), microtick=phase)
        internal, bit, active = (phase % 6) // 2, phase % 2, phase // 6
        epsilon = 1 if active == 0 else -1
        axis = next(a for a, value in enumerate(TRIADS[code][internal]) if value)
        owner_coords = tuple(int(((code >> internal) & 1) != bit) if a == axis else 0 for a in range(3))
        owner = site(4, owner_coords)
        head = site(4, add(owner_coords, V[2 * axis]))
        source, destination = (owner, head) if eta == 1 else (head, owner)
        attempted = 2 * axis + (1 if eta == 1 else 2)
        state.direction[source] = (1, 2)
        state.direction[source, active] = attempted
        state.credit[source, active] = k
        if q:
            loop(state, owner_coords, axis, (axis + 1) % 3)
            state.flux[:] *= q
        old_account = M.account_units(state)
        out, events = M.step(state)
        relation = epsilon * eta * q
        accepted = (relation == 0 and k == 1) or (relation == 1 and k == 0)
        assert M.account_units(out) == old_account and M.populations(out) == (1, 1)
        if accepted:
            assert len(events.moves) == 1 and not events.redirects
            expected_q, expected_k = (-epsilon * eta, 0) if relation == 0 else (0, 1)
            assert events.moves == [(epsilon, source, destination, owner, axis, q, expected_q, k, expected_k)]
            assert out.direction[source, active] == 0 and out.credit[source, active] == 0
            assert out.direction[destination, active] == attempted
            assert out.credit[destination, active] == expected_k
        else:
            assert not events.moves
            reason = "flux_capacity" if relation == -1 else "credit_deficit" if relation == 0 else "credit_capacity"
            x = xyz(4, source)
            outward = []
            for a in range(3):
                outward += [int(state.flux[source, a]), -int(state.flux[site(4, add(x, V[2 * a + 1])), a])]
            ports = [i + 1 for i, value in enumerate(outward) if value == epsilon]
            expected_v = ports[0] if k == 0 and len(ports) == 1 else (attempted + 1 if attempted % 2 else attempted - 1)
            assert events.redirects == [(epsilon, source, attempted, expected_v, reason)]
            np.testing.assert_array_equal(out.credit, state.credit)
            np.testing.assert_array_equal(out.flux, state.flux)
            expected_direction = state.direction.copy()
            expected_direction[source, active] = expected_v
            np.testing.assert_array_equal(out.direction, expected_direction)


def test_unchanged_complete_guide_and_hop_primitive_certificate():
    report = OLD.local_account_certificate()
    assert (report["hop_cases"], report["accepted"], report["refused"]) == (24, 8, 16)
    assert report["guide_cases"] == 17496


@pytest.mark.parametrize("bad", (True, -1, 48, 1.0, "0", None))
def test_seed_invalid_codes(bad):
    with pytest.raises(ValueError):
        M.seed_background(4, bad)


@pytest.mark.parametrize("bad", (True, 3, 5, -2, 4.0, None))
def test_invalid_sizes(bad):
    with pytest.raises(ValueError):
        M.seed_background(bad)


@pytest.mark.parametrize("bad", (True, -1, 0.0, None))
def test_invalid_ordinals(bad):
    with pytest.raises(ValueError):
        M.step(replace(blank(), microtick=bad))


@pytest.mark.parametrize("name,value", (("direction", 7), ("credit", 2), ("flux", 2), ("matching_frame", 48)))
def test_invalid_array_alphabet_fails_before_mutation(name, value):
    state = blank()
    getattr(state, name).flat[0] = value
    before = {key: getattr(state, key).tobytes() for key in M.NAMES}
    for operation in (M.step, M.checkpoint, M.validate, M.account_units, M.populations, M.manifestation):
        with pytest.raises(ValueError):
            operation(state)
        assert before == {key: getattr(state, key).tobytes() for key in M.NAMES}


def test_explicit_initialization_complete_shapes_ownership_and_gauss():
    state = rich()
    with pytest.raises(TypeError):
        M.initialize(4)
    for name in M.NAMES:
        supplied = {key: getattr(state, key) for key in M.NAMES}
        supplied[name] = None
        with pytest.raises(ValueError):
            M.initialize(4, **supplied)
        array = getattr(state, name)
        with pytest.raises(ValueError):
            M.validate(replace(state, **{name: array.view()}))
        with pytest.raises(ValueError):
            M.validate(replace(state, **{name: array[:-1].copy()}))
        with pytest.raises(ValueError):
            M.validate(replace(state, **{name: array.astype(np.int32)}))
    supplied = {name: getattr(state, name).view() for name in M.NAMES}
    copied = M.initialize(4, **supplied)
    same(state, copied)
    for name in M.NAMES:
        assert getattr(copied, name).flags.owndata
        assert all(not np.shares_memory(getattr(copied, name), array) for array in supplied.values())
    with pytest.raises(ValueError):
        M.validate(replace(state, credit=state.direction))
    with pytest.raises(ValueError):
        M.validate(replace(state, matching_frame=state.matching_frame[::-1]))
    invalid = blank(); invalid.credit[0, 0] = 1
    with pytest.raises(ValueError, match="empty"):
        M.step(invalid)
    invalid = blank(); invalid.direction[0, 0] = 1
    with pytest.raises(ValueError, match="divergence"):
        M.step(invalid)


def encoded(payload):
    return M.MAGIC + json.dumps(payload, sort_keys=True, separators=(",", ":")).encode()


@pytest.mark.parametrize("key", ("schema", "law", "rule", "frames", "encoding", "backend", "boundary"))
def test_foreign_checkpoint_identities(key):
    payload = json.loads(M.checkpoint(blank())[len(M.MAGIC):])
    payload[key] = "foreign"
    with pytest.raises(ValueError):
        M.restore(encoded(payload))


@pytest.mark.parametrize("name", M.NAMES)
@pytest.mark.parametrize("case", ("missing", "oversized", "truncated", "alphabet", "base64", "padding"))
def test_complete_codec_array_lengths_canonical_encoding_and_alphabets(name, case):
    payload = json.loads(M.checkpoint(blank())[len(M.MAGIC):])
    if case == "missing":
        del payload["arrays"][name]
    elif case == "base64":
        payload["arrays"][name] = "!" * len(payload["arrays"][name])
    elif case == "padding":
        payload["arrays"][name] += "="
    else:
        raw = bytearray(base64.b64decode(payload["arrays"][name]))
        if case == "oversized": raw.extend(b"\0" * 3)
        elif case == "truncated": raw.pop()
        else: raw[0] = 127
        payload["arrays"][name] = base64.b64encode(raw).decode()
    with pytest.raises(ValueError):
        M.restore(encoded(payload))


@pytest.mark.parametrize("value", ("", "00", "01", "0x1", "A", "+1", "-1", "1 ", "1_0", None, 0, True))
def test_canonical_hex_ordinals(value):
    payload = json.loads(M.checkpoint(blank())[len(M.MAGIC):])
    payload["microtick_hex"] = value
    with pytest.raises(ValueError):
        M.restore(encoded(payload))


def test_illegal_background_in_checkpoint_and_foreign_states_duplicates_unknown_fields():
    saved = M.checkpoint(blank())
    payload = json.loads(saved[len(M.MAGIC):])
    raw = bytearray(base64.b64decode(payload["arrays"]["matching_frame"]))
    raw[0] ^= 1
    payload["arrays"]["matching_frame"] = base64.b64encode(raw).decode()
    with pytest.raises(ValueError, match="neighbors"):
        M.restore(encoded(payload))
    old = OLD.initialize(4)
    with pytest.raises(ValueError): M.step(old)
    with pytest.raises(ValueError): M.restore(OLD.checkpoint(old))
    with pytest.raises(ValueError): OLD.step(blank())
    with pytest.raises(ValueError): OLD.restore(saved)
    with pytest.raises(ValueError): M.restore(M.MAGIC + b'{"L":4,"L":4}')
    with pytest.raises(ValueError): M.restore(M.MAGIC + b'{"arrays":{"flux":"","flux":""}}')
    for bad in (bytearray(saved), saved + b"x", M.MAGIC + b"[]", M.MAGIC + b"\xff"):
        with pytest.raises(ValueError): M.restore(bad)
    for target in ("root", "arrays"):
        extra = json.loads(saved[len(M.MAGIC):])
        (extra if target == "root" else extra["arrays"])["unknown"] = "0"
        with pytest.raises(ValueError): M.restore(encoded(extra))
    for name in ("law_id", "rule_hash", "frame_hash", "encoding_hash", "boundary"):
        state = blank()
        object.__setattr__(state, name, "foreign")
        with pytest.raises(ValueError): M.step(state)
        with pytest.raises(ValueError): M.checkpoint(state)
