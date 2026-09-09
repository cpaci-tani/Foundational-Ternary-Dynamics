"""Spatial and same-clock sign covariance of the complete five-array state."""
import base64
from dataclasses import replace
import hashlib
from itertools import product
import json
from pathlib import Path
import sys

import numpy as np
import pytest

from phi_v2_lattice import balanced_matching as B
from phi_v2_lattice import recorded_matching as M
from phi_v2_lattice import flux_binding as OLD

# Reuse the frozen independent spatial action/fixture oracle, not runtime
# decision helpers. Its source identity is pinned below.
import test_recorded_matching as R


def lift(state, eta=0):
    result = B.initialize(state.L, state.direction, state.credit, state.flux,
                          state.matching_frame, B.seed_charge_frame(state.L, eta))
    return replace(result, microtick=state.microtick)


def blank(eta=0, code=0):
    return lift(R.blank(code=code), eta)


def conjugate(state, *, toggle=True):
    charge = (state.charge_frame ^ 1) if toggle else state.charge_frame.copy()
    return B.MatchingState(state.L, state.microtick, state.direction[:, ::-1].copy(),
                           state.credit[:, ::-1].copy(), -state.flux,
                           state.matching_frame.copy(), charge)


def spatial(state, g=R.IDENTITY, offset=(0, 0, 0)):
    transformed = R.transform(state, g, offset)
    L = state.L
    coordinates = np.indices((L, L, L)).reshape(3, -1).T
    moved = (coordinates @ np.asarray(g).T + np.asarray(offset)) % L
    index = (moved[:, 0] * L + moved[:, 1]) * L + moved[:, 2]
    charge = np.empty_like(state.charge_frame)
    charge[index] = state.charge_frame
    return B.MatchingState(L, state.microtick, transformed.direction, transformed.credit,
                           transformed.flux, transformed.matching_frame, charge)


def same(left, right):
    R.same(left, right)
    np.testing.assert_array_equal(left.charge_frame, right.charge_frame)


def conjugate_events(events):
    result = B.MatchingEvents()
    result.moves = [(-epsilon, source, destination, owner, axis, -q0, -q1, k0, k1)
                    for epsilon, source, destination, owner, axis, q0, q1, k0, k1 in events.moves]
    result.redirects = [(-epsilon, source, old, new, reason)
                        for epsilon, source, old, new, reason in events.redirects]
    result.capacity_holds = [(-epsilon, owner, axis) for epsilon, owner, axis in events.capacity_holds]
    return result


def pure_step(state):
    before = {name: getattr(state, name).tobytes() for name in B.NAMES}
    result, events = B.step(state)
    for name in B.NAMES:
        array = getattr(result, name)
        assert array.flags.owndata and array.flags.c_contiguous
        assert before[name] == getattr(state, name).tobytes()
        assert all(not np.shares_memory(array, getattr(state, old)) for old in B.NAMES)
        assert all(not np.shares_memory(array, getattr(result, other))
                   for other in B.NAMES if other != name)
    return result, events


def test_frozen_reference_and_independent_oracle_identities():
    expected = ((M.__file__, "f81c72673159142158f863d06e8d5b3847d4e239f8df8752c2efbbf0e6a97172"),
                (R.__file__, "b2eec1c5d7fb4f180df56626b24e531e1eb2c3c238b09e966f11a8c94abacb52"),
                (OLD.__file__, "33203442187324ebf8d0f4c47668ffe0f3c5aa2d918284dab46fdb4aa6991740"))
    for path, digest in expected:
        assert hashlib.sha256(Path(path).read_bytes()).hexdigest() == digest
    assert B.TRIADS is M.TRIADS and B.FRAME_HASH == M.FRAME_HASH
    with pytest.raises(TypeError):
        B.RULE_DESCRIPTION["law"] = "foreign"


@pytest.mark.parametrize("eta,code", product(range(2), range(48)))
def test_full_spatial_and_balanced_compositions_all_backgrounds_and_phases(eta, code):
    # 2 charge seeds x48 triads x12 phases x48 spatial actions x2 C choices.
    # This is the full registered transformation matrix on one fixed legal
    # nonuniform carrier fixture, not an enumeration of all carrier states.
    base = lift(R.rich(code), eta)
    for phase in range(12):
        state = replace(base, microtick=phase)
        output, original_events = B.step(state)
        for g in R.ACTIONS:
            transformed = spatial(state, g, (3, 1, 3))
            expected = spatial(output, g, (3, 1, 3))
            actual, events = B.step(transformed)
            same(actual, expected)
            assert R.normalized(events) == R.events_transform(original_events, 4, g, (3, 1, 3))
            actual_c, events_c = B.step(conjugate(transformed))
            same(actual_c, conjugate(expected))
            assert R.normalized(events_c) == R.events_transform(conjugate_events(original_events), 4, g, (3, 1, 3))
            # The physical C and spatial action commute on every complete field.
            same(conjugate(transformed), spatial(conjugate(state), g, (3, 1, 3)))


@pytest.mark.parametrize("eta,code", product(range(2), range(48)))
def test_every_signed_unit_translation_and_C_composition_all_phases(eta, code):
    state = lift(R.rich(code), eta)
    for phase in range(12):
        state = replace(state, microtick=phase)
        output, original_events = B.step(state)
        for delta in R.V:
            translated = spatial(state, offset=delta)
            actual, events = B.step(translated)
            same(actual, spatial(output, offset=delta))
            assert R.normalized(events) == R.events_transform(original_events, 4, offset=delta)
            actual_c, events_c = B.step(conjugate(translated))
            same(actual_c, conjugate(spatial(output, offset=delta)))
            assert R.normalized(events_c) == R.events_transform(conjugate_events(original_events), 4, offset=delta)


def test_known_same_clock_C_failure_is_repaired_without_an_ordinal_shift():
    reference = R.blank()
    reference.direction[0] = (1, 1)
    reference.credit[0] = (1, 1)
    old_output, _ = M.step(reference)
    assert not np.array_equal(old_output.direction, old_output.direction[:, ::-1])
    state = lift(reference)
    complete_c = conjugate(state)
    assert complete_c.microtick == state.microtick == 0
    assert np.array_equal(complete_c.direction, state.direction)
    assert np.array_equal(complete_c.credit, state.credit)
    assert not np.array_equal(complete_c.charge_frame, state.charge_frame)
    output, events = pure_step(state)
    output_c, events_c = pure_step(complete_c)
    same(output_c, conjugate(output))
    assert events_c == conjugate_events(events)
    same(conjugate(complete_c), state)


@pytest.mark.parametrize("phase,eta,code", product(range(12), range(2), (0, 13, 47)))
def test_reference_conjugate_sector_and_owned_replay_histories(phase, eta, code):
    state = replace(lift(R.rich(code), eta), microtick=phase)
    saved = B.checkpoint(state)
    restored = B.restore(saved)
    assert B.checkpoint(restored) == saved
    account, counts = B.account_units(state), B.populations(state)
    for name in B.NAMES:
        assert all(not np.shares_memory(getattr(restored, name), getattr(state, old)) for old in B.NAMES)
    for _ in range(24):
        # Independently form the declared reference sector and reverse its
        # physical column/flux action without using the new conversion helper.
        physical = conjugate(state, toggle=False) if eta else state
        reference = M.MatchingState(state.L, state.microtick,
                                    *(getattr(physical, name).copy() for name in M.NAMES))
        expected, expected_events = M.step(reference)
        expected = lift(expected, eta)
        if eta:
            expected = conjugate(expected, toggle=False)
            expected_events = conjugate_events(expected_events)
        state, events = pure_step(state)
        restored, replay_events = pure_step(restored)
        same(state, expected)
        same(state, restored)
        assert events == expected_events == replay_events
        assert B.checkpoint(state) == B.checkpoint(restored)
        assert B.account_units(state) == account and B.populations(state) == counts
        np.testing.assert_array_equal(B.manifestation(conjugate(state)), -B.manifestation(state))


@pytest.mark.parametrize("phase,eta", product(range(12), range(2)))
def test_huge_clock_replay_and_C_involution_are_on_the_full_ordinal(phase, eta):
    limit = sys.get_int_max_str_digits()
    state = replace(lift(R.rich(47), eta), microtick=12 * 10 ** 5000 + phase)
    restored = B.restore(B.checkpoint(state))
    same(restored, state)
    assert B.checkpoint(conjugate(conjugate(state))) == B.checkpoint(state)
    assert conjugate(state).microtick == state.microtick
    output, events = B.step(restored)
    output_c, events_c = B.step(conjugate(state))
    assert output.microtick == state.microtick + 1
    same(output_c, conjugate(output))
    assert events_c == conjugate_events(events)
    assert sys.get_int_max_str_digits() == limit


def test_formal_pointwise_conversion_is_local_even_when_uniformity_is_not_assumed():
    # This is an array-algebra test, not an admitted nonuniform-eta trajectory.
    state = blank()
    state.direction[:] = (1, 2)
    state.credit[:] = (0, 1)
    state.flux[:] = (1, -1, 0)  # constant oriented flux has zero divergence
    eta = (np.arange(64) % 2).astype(np.uint8)
    with pytest.raises(ValueError, match="neighbor"):
        B.validate(replace(state, charge_frame=eta))
    converted = B._conjugated_payload(state, eta)
    for x in range(64):
        np.testing.assert_array_equal(converted[0][x], state.direction[x, ::-1] if eta[x] else state.direction[x])
        np.testing.assert_array_equal(converted[1][x], state.credit[x, ::-1] if eta[x] else state.credit[x])
        np.testing.assert_array_equal(converted[2][x], state.flux[x] * (-1 if eta[x] else 1))
        assert converted[3][x] == state.matching_frame[x]
    other_eta = eta.copy(); other_eta[37] ^= 1
    other = B._conjugated_payload(state, other_eta)
    for index in range(3):
        assert np.flatnonzero(np.any(converted[index] != other[index], axis=1)).tolist() == [37]
    np.testing.assert_array_equal(converted[3], other[3])
    intermediate = M.MatchingState(4, 0, *converted)
    recovered = B._conjugated_payload(intermediate, eta)
    for name, array in zip(M.NAMES, recovered):
        np.testing.assert_array_equal(array, getattr(state, name))
        assert array.flags.owndata


@pytest.mark.parametrize("eta,code", product(range(2), range(48)))
def test_all_hop_classes_in_physical_polarity_and_background(eta, code):
    for phase, q, k, sign in product(range(12), (-1, 0, 1), (0, 1), (-1, 1)):
        state = replace(blank(eta, code), microtick=phase)
        internal, bit = (phase % 6) // 2, phase % 2
        active = (phase // 6) ^ eta
        epsilon = 1 if active == 0 else -1
        axis = next(a for a, value in enumerate(R.TRIADS[code][internal]) if value)
        owner_xyz = tuple(int(((code >> internal) & 1) != bit) if a == axis else 0 for a in range(3))
        owner, head = R.site(4, owner_xyz), R.site(4, R.add(owner_xyz, R.V[2 * axis]))
        source, destination = (owner, head) if sign == 1 else (head, owner)
        direction = 2 * axis + (1 if sign == 1 else 2)
        state.direction[source] = (1, 2)
        state.direction[source, active] = direction
        state.credit[source, active] = k
        if q:
            R.loop(state, owner_xyz, axis, (axis + 1) % 3)
            state.flux[:] *= q
        account = B.account_units(state)
        result, events = B.step(state)
        relative = epsilon * sign * q
        accepted = (relative == 0 and k == 1) or (relative == 1 and k == 0)
        assert B.account_units(result) == account and B.populations(result) == (1, 1)
        if accepted:
            q_new, k_new = (-epsilon * sign, 0) if relative == 0 else (0, 1)
            assert events.moves == [(epsilon, source, destination, owner, axis, q, q_new, k, k_new)]
            assert not events.redirects
            assert result.direction[source, active] == 0 and result.direction[destination, active] == direction
        else:
            assert not events.moves and len(events.redirects) == 1
            assert events.redirects[0][0] == epsilon
            reason = "flux_capacity" if relative == -1 else "credit_deficit" if relative == 0 else "credit_capacity"
            assert events.redirects[0][-1] == reason
            np.testing.assert_array_equal(result.credit, state.credit)
            np.testing.assert_array_equal(result.flux, state.flux)


@pytest.mark.parametrize("eta", range(2))
def test_legal_carrier_interventions_all_phases_keep_radius_one(eta):
    for phase, family in product(range(12), OLD.NAMES):
        state = replace(blank(eta, 47), microtick=phase)
        active = phase // 6 ^ eta
        u = tuple((-1 if phase % 2 else 1) * v for v in R.TRIADS[47][(phase % 6) // 2])
        state.direction[0] = (1, 2)
        state.direction[0, active] = R.V.index(u) + 1
        state.credit[0] = (1, 1)
        changed = B.restore(B.checkpoint(state))
        if family == "direction": changed.direction[0, active] = (int(state.direction[0, active]) + 1) % 6 + 1
        elif family == "credit": changed.credit[0, active] = 0
        else: R.loop(changed)
        owners = set()
        for name in B.NAMES:
            delta = getattr(state, name) != getattr(changed, name)
            owners.update(map(int, np.flatnonzero(delta if delta.ndim == 1 else delta.any(axis=1))))
        allowed = {R.site(4, R.add(R.xyz(4, x), offset)) for x in owners for offset in product((-1, 0, 1), repeat=3)}
        left, _ = B.step(state); right, _ = B.step(changed)
        for name in B.NAMES:
            delta = getattr(left, name) != getattr(right, name)
            actual = set(map(int, np.flatnonzero(delta if delta.ndim == 1 else delta.any(axis=1))))
            assert actual <= allowed


@pytest.mark.parametrize("bad", (True, -1, 2, 1.0, "0", None))
def test_invalid_charge_seed_values(bad):
    with pytest.raises(ValueError): B.seed_charge_frame(4, bad)


@pytest.mark.parametrize("name", B.NAMES)
def test_complete_arrays_required_and_owned_without_aliases(name):
    state = blank()
    supplied = {key: getattr(state, key) for key in B.NAMES}
    supplied[name] = None
    with pytest.raises(ValueError): B.initialize(4, **supplied)
    array = getattr(state, name)
    for malformed in (array.view(), array[:-1].copy(), array.astype(np.int32)):
        with pytest.raises(ValueError): B.validate(replace(state, **{name: malformed}))
    views = {key: getattr(state, key).view() for key in B.NAMES}
    copied = B.initialize(4, **views)
    for key in B.NAMES:
        assert getattr(copied, key).flags.owndata
        assert all(not np.shares_memory(getattr(copied, key), old) for old in views.values())


def test_invalid_charge_gradient_shape_alias_and_alphabet_fail_before_mutation():
    state = blank()
    defects = []
    wrong_bit = state.charge_frame.copy(); wrong_bit[0] = 2; defects.append(wrong_bit)
    local = state.charge_frame.copy(); local[0] = 1; defects.append(local)
    defects += [state.matching_frame, state.charge_frame[::-1]]
    for charge in defects:
        bad = replace(state, charge_frame=charge)
        before = {name: getattr(bad, name).tobytes() for name in B.NAMES}
        for operation in (B.step, B.validate, B.checkpoint, B.account_units, B.populations, B.manifestation):
            with pytest.raises(ValueError): operation(bad)
            assert before == {name: getattr(bad, name).tobytes() for name in B.NAMES}
    for value in (True, -1, 0.0, None):
        with pytest.raises(ValueError): B.step(replace(state, microtick=value))
    for L in (True, 3, 5, 4.0, None):
        with pytest.raises(ValueError): B.seed_charge_frame(L)
    with pytest.raises(TypeError): B.initialize(4)


def encode(payload):
    return B.MAGIC + json.dumps(payload, sort_keys=True, separators=(",", ":")).encode()


@pytest.mark.parametrize("name", B.NAMES)
@pytest.mark.parametrize("case", ("missing", "oversized", "truncated", "alphabet", "base64", "padding"))
def test_complete_codec_payload_lengths_canonical_bytes_and_alphabets(name, case):
    payload = json.loads(B.checkpoint(blank())[len(B.MAGIC):])
    if case == "missing": del payload["arrays"][name]
    elif case == "base64": payload["arrays"][name] = "!" * len(payload["arrays"][name])
    elif case == "padding": payload["arrays"][name] += "="
    else:
        raw = bytearray(base64.b64decode(payload["arrays"][name]))
        if case == "oversized": raw.extend(b"\0" * 3)
        elif case == "truncated": raw.pop()
        else: raw[0] = 127
        payload["arrays"][name] = base64.b64encode(raw).decode()
    with pytest.raises(ValueError): B.restore(encode(payload))


@pytest.mark.parametrize("value", ("", "00", "01", "0x1", "A", "+1", "-1", "1 ", "1_0", None, 0, True))
def test_checkpoint_canonical_hex_ordinal(value):
    payload = json.loads(B.checkpoint(blank())[len(B.MAGIC):])
    payload["microtick_hex"] = value
    with pytest.raises(ValueError): B.restore(encode(payload))


@pytest.mark.parametrize("key", ("schema", "law", "rule", "frames", "encoding", "backend", "boundary"))
def test_foreign_checkpoint_identity(key):
    payload = json.loads(B.checkpoint(blank())[len(B.MAGIC):])
    payload[key] = "foreign"
    with pytest.raises(ValueError): B.restore(encode(payload))


def test_foreign_laws_unknown_duplicate_and_nonuniform_charge_checkpoint():
    state = blank()
    saved = B.checkpoint(state)
    for foreign, module in ((R.blank(), M), (OLD.initialize(4), OLD)):
        with pytest.raises(ValueError): B.step(foreign)
        with pytest.raises(ValueError): B.restore(module.checkpoint(foreign))
        with pytest.raises(ValueError): module.step(state)
        with pytest.raises(ValueError): module.restore(saved)
    for name in ("law_id", "rule_hash", "frame_hash", "encoding_hash", "boundary"):
        wrong = blank()
        object.__setattr__(wrong, name, "foreign")
        with pytest.raises(ValueError): B.step(wrong)
        with pytest.raises(ValueError): B.checkpoint(wrong)
    payload = json.loads(saved[len(B.MAGIC):])
    raw = bytearray(base64.b64decode(payload["arrays"]["charge_frame"]))
    raw[0] = 1
    payload["arrays"]["charge_frame"] = base64.b64encode(raw).decode()
    with pytest.raises(ValueError, match="neighbor"): B.restore(encode(payload))
    for payload in (B.MAGIC + b'{"L":4,"L":4}', B.MAGIC + b'{"arrays":{"flux":"","flux":""}}',
                    B.MAGIC + b"[]", B.MAGIC + b"\xff", saved + b"x", bytearray(saved)):
        with pytest.raises(ValueError): B.restore(payload)
    for nested in (False, True):
        payload = json.loads(saved[len(B.MAGIC):])
        (payload["arrays"] if nested else payload)["unknown"] = "0"
        with pytest.raises(ValueError): B.restore(encode(payload))
