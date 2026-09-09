from dataclasses import replace
from itertools import product
import hashlib
import json
import sys

import numpy as np
import pytest

from phi_v2_lattice import recorded_routing as R
from phi_v2_lattice import recorded_routing_response as Q
from phi_v2_lattice import routing_diffusion as OLD


def arrays(L, code):
    return (np.zeros((L, L, L, 2, 6), dtype=bool),
            np.zeros((L, L, L), dtype=np.uint16), R.seed_background(L, code))


def heterogeneous(L, code, phase=0):
    bank, routers, background = arrays(L, code)
    bank[:] = np.arange(bank.size).reshape(bank.shape) % 11 < 5
    routers[:] = (np.arange(L ** 3).reshape(L, L, L) * 29) % 720
    return replace(R.initialize(bank, routers, background), microtick=phase)


def assert_arrays_equal(a, b):
    assert a.L == b.L and a.microtick == b.microtick
    for name in ("bank", "routers", "corner_direction"):
        assert np.array_equal(getattr(a, name), getattr(b, name)), name


def independent_transform(state, axes, signs, translation=(0, 0, 0)):
    """Scalar site/channel action, without the runtime's transform/action tables."""
    bank, routers, background = arrays(state.L, 0)
    act = lambda v: tuple(signs[j] * v[axes[j]] for j in range(3))
    channel_action = [R.VELOCITIES.index(act(v)) for v in R.VELOCITIES]
    for x in product(range(state.L), repeat=3):
        y = tuple((value + translation[j]) % state.L for j, value in enumerate(act(x)))
        permutation = R.PERMUTATIONS[int(state.routers[x])]
        conjugated = [0] * 6
        for c in range(6):
            conjugated[channel_action[c]] = channel_action[permutation[c]]
            bank[y + (slice(None), channel_action[c])] = state.bank[x + (slice(None), c)]
        routers[y] = R.PERMUTATIONS.index(tuple(conjugated))
        background[y] = R.CORNERS.index(act(R.CORNERS[int(state.corner_direction[x])]))
    return replace(R.initialize(bank, routers, background), microtick=state.microtick)


@pytest.mark.parametrize("action", R.CUBE_ACTIONS)
def test_every_cube_action_all_eight_backgrounds_and_both_phases(action):
    axes, signs = action
    for code, phase in product(range(8), range(2)):
        state = heterogeneous(3, code, phase)
        transformed = independent_transform(state, axes, signs)
        assert_arrays_equal(R.transform(state, axes, signs), transformed)
        assert_arrays_equal(R.step(transformed), independent_transform(R.step(state), axes, signs))


@pytest.mark.parametrize("code,phase", product(range(8), range(2)))
def test_unit_translations_polarity_exchange_and_local_scalar_rule(code, phase):
    state = heterogeneous(4, code, phase)
    output = R.step(state)
    expected_bank, expected_router, expected_background = arrays(4, code)
    for x in product(range(4), repeat=3):
        direction = R.CORNERS[int(state.corner_direction[x])]
        source = tuple((x[j] - direction[j]) % 4 for j in range(3))
        expected_router[x] = state.routers[source]
        for polarity, c in product(range(2), range(6)):
            if phase == 0:
                target = R.PERMUTATIONS[int(state.routers[x])][c]
                expected_bank[x + (polarity, target)] = state.bank[x + (polarity, c)]
            else:
                y = tuple((x[j] + R.VELOCITIES[c][j]) % 4 for j in range(3))
                expected_bank[y + (polarity, c)] = state.bank[x + (polarity, c)]
    assert np.array_equal(output.bank, expected_bank)
    assert np.array_equal(output.routers, expected_router)
    assert np.array_equal(output.corner_direction, expected_background)
    for translation in ((1, 0, 0), (0, 1, 0), (0, 0, 1), (-1, 1, 2)):
        shifted = independent_transform(state, (0, 1, 2), (1, 1, 1), translation)
        assert_arrays_equal(R.step(shifted), independent_transform(output, (0, 1, 2), (1, 1, 1), translation))
    flipped = replace(state, bank=state.bank[..., ::-1, :].copy())
    flipped_output = R.step(flipped)
    assert np.array_equal(flipped_output.bank, output.bank[..., ::-1, :])
    assert np.array_equal(flipped_output.routers, output.routers)


def test_old_rotation_witness_now_transforms_the_complete_background():
    bank, routers, background = arrays(5, 7)
    routers[0, 0, 0] = R.PERMUTATIONS.index((0, 1, 3, 2, 4, 5))
    state = R.initialize(bank, routers, background)
    for signs in ((-1, 1, 1), (1, -1, -1)):
        transformed = independent_transform(state, (0, 1, 2), signs)
        assert np.array_equal(transformed.routers, state.routers)
        assert not np.array_equal(transformed.corner_direction, state.corner_direction)
        expected_position = tuple(s % 5 for s in signs)
        output = R.step(transformed)
        assert np.argwhere(output.routers != 0).tolist() == [list(expected_position)]
        assert_arrays_equal(output, independent_transform(R.step(state), (0, 1, 2), signs))


@pytest.mark.parametrize("L", (3, 4, 7))
def test_plus_plus_plus_sector_matches_frozen_runtime_through_both_stages(L):
    state = heterogeneous(L, 7)
    old = OLD.initialize(state.bank, state.routers)
    for _ in range(12):
        state, old = R.step(state), OLD.step(old)
        assert state.microtick == old.microtick
        assert np.array_equal(state.bank, old.bank)
        assert np.array_equal(state.routers, old.routers)
        assert np.all(state.corner_direction == 7)
    with pytest.raises(ValueError): R.restore(OLD.checkpoint(old))
    with pytest.raises(ValueError): OLD.restore(R.checkpoint(state))
    with pytest.raises(ValueError): R.step(old)


def test_every_router_and_local_six_bit_pattern():
    bank, routers, background = arrays(36, 4)
    bits = ((np.arange(64)[:, None] >> np.arange(6)) & 1).astype(bool)
    flat = bank.reshape(-1, 2, 6)
    flat[:46080, 0] = np.tile(bits, (720, 1))
    flat[:46080, 1] = ~flat[:46080, 0]
    routers.reshape(-1)[:46080] = np.repeat(np.arange(720, dtype=np.uint16), 64)
    state = R.initialize(bank, routers, background)
    out = R.step(state)
    for index, permutation in enumerate(R.PERMUTATIONS):
        for source, target in enumerate(permutation):
            assert np.array_equal(out.bank.reshape(-1, 2, 6)[index*64:(index+1)*64, :, target],
                                  flat[index*64:(index+1)*64, :, source])
    assert np.array_equal(out.bank.sum(axis=-1), state.bank.sum(axis=-1))


@pytest.mark.parametrize("code", range(8))
def test_all_216_three_cycle_direction_words_execute_for_each_background(code):
    L, center, direction = 11, (5, 5, 5), R.CORNERS[code]
    choices = {(a, b): [i for i, p in enumerate(R.PERMUTATIONS) if p[a] == b]
               for a, b in product(range(6), repeat=2)}
    assert all(len(v) == 120 for v in choices.values())
    endpoints = {}
    for word in product(range(6), repeat=3):
        bank, routers, background = arrays(L, code)
        bank[center + (0, 0)] = True
        point, incoming, origins = center, 0, set()
        for k, outgoing in enumerate(word):
            origin = tuple((point[j] - 2*k*direction[j]) % L for j in range(3))
            assert origin not in origins
            origins.add(origin)
            routers[origin] = choices[incoming, outgoing][0]
            point = tuple(point[j] + R.VELOCITIES[outgoing][j] for j in range(3))
            incoming = outgoing
        state = R.initialize(bank, routers, background)
        for _ in range(6): state = R.step(state)
        assert state.bank[point + (0, incoming)] and int(state.bank.sum()) == 1
        offset = tuple(point[j] - center[j] for j in range(3))
        endpoints[offset] = endpoints.get(offset, 0) + 1
    assert endpoints == Q.endpoint_counts(3)
    assert sum(endpoints.values()) * Q.path_probability(L, 3, code) == 1


@pytest.mark.parametrize("code,phase", product(range(8), range(2)))
def test_owned_complete_checkpoint_replay_counts_and_huge_ordinals(code, phase):
    state = replace(heterogeneous(3, code), microtick=10**5000 + phase)
    limit = sys.get_int_max_str_digits()
    original = R.checkpoint(state)
    replay = R.restore(original)
    population = R.populations(state)
    histogram = np.bincount(state.routers.ravel(), minlength=720)
    for _ in range(4):
        before = R.checkpoint(state)
        out, replay = R.step(state), R.step(replay)
        assert R.checkpoint(state) == before
        assert R.checkpoint(out) == R.checkpoint(replay)
        assert R.populations(out) == population
        assert np.array_equal(np.bincount(out.routers.ravel(), minlength=720), histogram)
        assert np.array_equal(out.corner_direction, state.corner_direction)
        for a in (state.bank, state.routers, state.corner_direction):
            for b in (out.bank, out.routers, out.corner_direction): assert not np.shares_memory(a, b)
        state = out
    assert sys.get_int_max_str_digits() == limit
    recovered = R.restore(original)
    observation = R.densities(recovered); observation[:] = 0
    assert R.checkpoint(recovered) == original


@pytest.mark.parametrize("phase", (0, 1))
def test_router_bank_and_background_causal_dependencies(phase):
    state = heterogeneous(7, 6, phase)
    perturbed = R.restore(R.checkpoint(state))
    perturbed.bank[3, 3, 3, 0, 0] ^= True
    perturbed.routers[3, 3, 3] = (int(perturbed.routers[3, 3, 3]) + 1) % 720
    before, after = R.step(state), R.step(perturbed)
    changed = (np.any(before.bank != after.bank, axis=(-1, -2))
               | (before.routers != after.routers) | (before.corner_direction != after.corner_direction))
    assert np.any(changed) and np.all(np.abs(np.argwhere(changed) - 3) <= 1)
    assert np.array_equal(after.corner_direction, perturbed.corner_direction)
    # A single D defect is outside the legal domain. Compare admitted uniform
    # D alternatives at one output using the exact local selector instead.
    for code in range(8):
        other = replace(state, corner_direction=R.seed_background(7, code))
        result = R.step(other)
        source = tuple((3-d) % 7 for d in R.CORNERS[code])
        assert result.routers[3, 3, 3] == other.routers[source]
        assert np.array_equal(result.corner_direction, other.corner_direction)


def rewrite(blob, metadata=None, payload=None, raw_header=None):
    body = blob[len(R.MAGIC)+32:]
    n = int.from_bytes(body[:4], "little")
    header = json.loads(body[4:4+n])
    if metadata is not None: metadata(header)
    raw = json.dumps(header, sort_keys=True, separators=(",", ":")).encode() if raw_header is None else raw_header
    data = body[4+n:] if payload is None else payload(body[4+n:])
    body = len(raw).to_bytes(4, "little") + raw + data
    return R.MAGIC + hashlib.sha256(body).digest() + body


@pytest.mark.parametrize("key,value", [("L", True), ("L", 2), ("law", "old"), ("schema", "foreign"),
    ("rule_hash", "0"*64), ("encoding_hash", "0"*64), ("tick_hex", "00"), ("tick_hex", "A"),
    ("tick_hex", "-1"), ("tick_hex", 1), ("tick_hex", "0x1"), ("unknown", 0)])
def test_checkpoint_metadata_rejected(key, value):
    blob = R.checkpoint(R.initialize(*arrays(3, 0)))
    bad = rewrite(blob, metadata=lambda h: h.__setitem__(key, value))
    with pytest.raises(ValueError): R.restore(bad)


def test_checkpoint_missing_and_invalid_background_payloads_and_encodings():
    blob = R.checkpoint(R.initialize(*arrays(3, 0)))
    for bad in (blob[:-1], bytearray(blob), b"OTHER"+blob,
                rewrite(blob, payload=lambda p:p[:-27]),
                rewrite(blob, payload=lambda p:p[:-27]+bytes([8])*27),
                rewrite(blob, payload=lambda p:p[:-1]+bytes([1])),
                rewrite(blob, payload=lambda p:p[:40]+bytes([128])+p[41:]),
                rewrite(blob, payload=lambda p:p[:41]+(720).to_bytes(2, 'little')+p[43:])):
        with pytest.raises(ValueError): R.restore(bad)
    body = blob[len(R.MAGIC)+32:];n=int.from_bytes(body[:4], 'little');header=body[4:4+n]
    for raw in (b' '+header, b'{"L":3,'+header[1:], b'[]'):
        with pytest.raises(ValueError): R.restore(rewrite(blob, raw_header=raw))
    corrupt = bytearray(blob);corrupt[-1] ^= 1
    with pytest.raises(ValueError): R.restore(bytes(corrupt))


@pytest.mark.parametrize("kind", ("clock", "boolbytes", "router", "corner", "mixed", "shape", "dtype", "law", "rule", "encoding"))
def test_invalid_complete_state_rejected_before_mutation(kind):
    state = R.initialize(*arrays(3, 0))
    if kind == "clock": state = replace(state, microtick=True)
    if kind == "boolbytes": state.bank.view(np.uint8).flat[0] = 2
    if kind == "router": state.routers.flat[0] = 720
    if kind == "corner": state.corner_direction.fill(8)
    if kind == "mixed": state.corner_direction.flat[0] = 1
    if kind == "shape": state = replace(state, corner_direction=state.corner_direction[:2])
    if kind == "dtype": state = replace(state, corner_direction=state.corner_direction.astype(np.int8))
    if kind == "law": state = replace(state, law_id="foreign")
    if kind == "rule": state = replace(state, rule_hash="foreign")
    if kind == "encoding": state = replace(state, encoding_hash="foreign")
    before = tuple(a.tobytes() for a in (state.bank, state.routers, state.corner_direction))
    with pytest.raises(ValueError): R.step(state)
    assert before == tuple(a.tobytes() for a in (state.bank, state.routers, state.corner_direction))


def test_initialization_requires_owned_explicit_background_and_disjoint_arrays():
    bank, routers, background = arrays(3, 5)
    state = R.initialize(bank, routers, background)
    bank[:] = True; routers[:] = 1; background[:] = 0
    assert not state.bank.any() and not state.routers.any() and np.all(state.corner_direction == 5)
    with pytest.raises(TypeError): R.initialize(bank, routers)
    with pytest.raises(ValueError): R.initialize(bank, routers, None)
    shared = np.zeros(324, dtype=np.uint8)
    aliased_bank = shared.view(bool).reshape(3, 3, 3, 2, 6)
    aliased_router = shared[:54].view(np.uint16).reshape(3, 3, 3)
    aliased_direction = shared[:27].reshape(3, 3, 3)
    with pytest.raises(ValueError): R.initialize(aliased_bank, aliased_router, aliased_direction)
    for code in (True, -1, 8, 1.0):
        with pytest.raises(ValueError): R.seed_background(3, code)


def test_runtime_certificate_and_scope_flags():
    report = R.certificate()
    assert report['complete_runtime_cube_comparisons'] == 48*8*2
    assert report['complete_runtime_translation_comparisons'] == 3*8*2
    assert report['covariance_checks_passed']
    assert not report['complete_v3_law_approved'] and not report['named_expiry_supplied']
