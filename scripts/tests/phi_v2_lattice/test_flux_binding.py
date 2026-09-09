"""Finite account, restoring regressions and complete-state lifecycle gates.

The six first-return values were observed in a disclosed pre-spec draft.
These are frozen regressions, not fresh predictions or physical binding tests.
"""
from dataclasses import replace
import base64
from itertools import product
import json
import sys

import numpy as np
import pytest

from phi_v2_lattice import flux_binding as F

DIRECTIONS = ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1))
RETURNS = (19, 8, 22, 14, 24, 14)


def site(L, xyz):
    x, y, z = (a % L for a in xyz)
    return (x * L + y) * L + z


def xyz(L, index):
    return index // (L * L), (index // L) % L, index % L


def plus(a, b, L=None):
    value = tuple(x + y for x, y in zip(a, b))
    return tuple(x % L for x in value) if L else value


def edge(x, d, L):
    axis = next(i for i, value in enumerate(d) if value)
    return (x if d[axis] > 0 else plus(x, d, L), axis), d[axis]


def preparation(delta=(0, 0, 0), origin=(4, 4, 4), L=8):
    direction = np.zeros((L ** 3, 2), dtype=np.uint8)
    credit = np.zeros_like(direction)
    flux = np.zeros((L ** 3, 3), dtype=np.int8)
    left, right = site(L, origin), site(L, plus(origin, delta, L))
    direction[left, 0], direction[right, 1] = 2, 1
    credit[left, 0], credit[right, 1] = 1, int(delta == (0, 0, 0))
    if delta != (0, 0, 0):
        (owner, axis), sign = edge(origin, delta, L)
        flux[site(L, owner), axis] = sign
    return F.initialize(L, direction, credit, flux)


def independent_oracle(state):
    particles = {}
    for slot, epsilon in enumerate((1, -1)):
        pos = int(np.flatnonzero(state.direction[:, slot])[0])
        particles[epsilon] = [xyz(state.L, pos), DIRECTIONS[int(state.direction[pos, slot]) - 1],
                              int(state.credit[pos, slot])]
    flux = {(xyz(state.L, int(owner)), int(axis)): int(state.flux[owner, axis])
            for owner, axis in np.argwhere(state.flux)}
    return particles, flux


def oracle_tick(particles, flux, tick, L, *, guide=True):
    """Sparse coordinate oracle; no candidate shift, guide or step helper."""
    epsilon = 1 if tick % 12 < 6 else -1
    axis, parity = (tick % 6) // 2, tick % 2
    x, v, k = particles[epsilon]
    direction = tuple((1 if x[axis] % 2 == parity else -1) if a == axis else 0 for a in range(3))
    if v != direction:
        return
    key, eta = edge(x, direction, L)
    before = flux.get(key, 0)
    after = before - epsilon * eta
    next_credit = k - (after * after - before * before)
    if abs(after) <= 1 and 0 <= next_credit <= 1:
        particles[epsilon] = [plus(x, direction, L), v, next_credit]
        if after:
            flux[key] = after
        else:
            flux.pop(key, None)
    else:
        ports = []
        for d in DIRECTIONS:
            key, eta = edge(x, d, L)
            if eta * flux.get(key, 0) == epsilon:
                ports.append(d)
        particles[epsilon][1] = ports[0] if guide and k == 0 and len(ports) == 1 else tuple(-a for a in v)


def compare_oracle(state, particles, flux):
    direction = np.zeros_like(state.direction)
    credit = np.zeros_like(state.credit)
    expected_flux = np.zeros_like(state.flux)
    for slot, epsilon in enumerate((1, -1)):
        x, v, k = particles[epsilon]
        direction[site(state.L, x), slot] = DIRECTIONS.index(v) + 1
        credit[site(state.L, x), slot] = k
    for (owner, axis), value in flux.items():
        expected_flux[site(state.L, owner), axis] = value
    for name, expected in zip(F.NAMES, (direction, credit, expected_flux)):
        np.testing.assert_array_equal(getattr(state, name), expected, err_msg=name)


def pure_step(state):
    before = F.checkpoint(state)
    result, events = F.step(state)
    assert F.checkpoint(state) == before
    for name in F.NAMES:
        value = getattr(result, name)
        assert value.flags.owndata
        assert all(not np.shares_memory(value, getattr(state, old)) for old in F.NAMES)
    return result, events


def test_local_account_and_routing_alphabets():
    certificate = F.local_account_certificate()
    assert (certificate["hop_cases"], certificate["accepted"], certificate["refused"]) == (24, 8, 16)
    assert certificate["guide_cases"] == 17496
    assert not certificate["physical_energy_identified"]
    assert not certificate["full_restoring_quotient_enumerated"]


@pytest.mark.parametrize("index", range(7))
def test_frozen_144_tick_restoring_histories_complete_records_and_exchange(index):
    delta = (0, 0, 0) if index == 6 else DIRECTIONS[index]
    state = preparation(delta)
    particles, flux = independent_oracle(state)
    unwrapped = {epsilon: tuple(particles[epsilon][0]) for epsilon in (1, -1)}
    first_return = None
    departed = delta != (0, 0, 0)
    moves = redirects = 0
    resource_levels = set()
    for tick in range(1, 145):
        before = state
        oracle_tick(particles, flux, tick - 1, state.L)
        state, events = pure_step(state)
        compare_oracle(state, particles, flux)
        assert state.microtick == tick
        assert F.populations(state) == (1, 1)
        assert F.account_units(state) == 2
        np.testing.assert_array_equal(F.manifestation(state),
                                      (state.direction[:, 0] != 0).astype(np.int8)
                                      - (state.direction[:, 1] != 0).astype(np.int8))
        for epsilon, source, destination, owner, axis, q0, q1, k0, k1 in events.moves:
            assert q1 * q1 - q0 * q0 == k0 - k1
            slot = 0 if epsilon == 1 else 1
            d = DIRECTIONS[int(before.direction[source, slot]) - 1]
            unwrapped[epsilon] = plus(unwrapped[epsilon], d)
            assert site(state.L, unwrapped[epsilon]) == destination
            moves += 1
        redirects += len(events.redirects)
        distance = sum(abs(b - a) for a, b in zip(unwrapped[1], unwrapped[-1]))
        assert distance <= 2
        resource_levels.add(int(np.abs(state.flux).sum()))
        if distance:
            departed = True
        elif departed and first_return is None:
            first_return = tick
    assert moves and redirects
    assert resource_levels == {0, 1, 2}
    if index < 6:
        assert first_return == RETURNS[index]
    else:
        assert first_return == 19


def test_displacement_is_explicit_local_field_work_not_an_imposed_pair_marker():
    baseline = preparation()
    for delta in DIRECTIONS:
        displaced = preparation(delta)
        assert F.account_units(displaced) == F.account_units(baseline) == 2
        assert int(np.abs(displaced.flux).sum()) == int(np.abs(baseline.flux).sum()) + 1
        assert int(displaced.credit.sum()) == int(baseline.credit.sum()) - 1
        assert len(np.argwhere(displaced.flux)) == 1
        assert set(F.NAMES) == {"direction", "credit", "flux"}
    invalid = preparation(DIRECTIONS[0])
    invalid.flux.fill(0)
    with pytest.raises(ValueError, match="divergence"):
        F.validate(invalid)


@pytest.mark.parametrize("epsilon", (1, -1))
@pytest.mark.parametrize("direction_code", range(1, 7))
@pytest.mark.parametrize("operation", ("extend", "retract", "credit_capacity"))
def test_every_polarity_and_signed_axis_local_exchange(epsilon, direction_code, operation):
    L, origin = 4, (0, 0, 0)
    state = F.initialize(L)
    d = DIRECTIONS[direction_code - 1]
    destination = plus(origin, d, L)
    (owner, axis), eta = edge(origin, d, L)
    slot = 0 if epsilon == 1 else 1
    source_index, destination_index = site(L, origin), site(L, destination)
    state.direction[source_index, slot] = direction_code
    if operation == "extend":
        state.direction[source_index, 1 - slot] = 1
        state.credit[source_index, :] = 1
    else:
        state.direction[destination_index, 1 - slot] = 1
        state.credit[destination_index, 1 - slot] = 1
        state.credit[source_index, slot] = int(operation == "credit_capacity")
        state.flux[site(L, owner), axis] = epsilon * eta
    phase = slot * 6 + axis * 2 + owner[axis] % 2
    state = replace(state, microtick=phase)
    account = F.account_units(state)
    out, events = pure_step(state)
    assert F.account_units(out) == account
    assert F.populations(out) == (1, 1)
    if operation == "credit_capacity":
        assert not events.moves
        assert events.redirects == [(epsilon, source_index, direction_code,
                                      direction_code + 1 if direction_code % 2 else direction_code - 1,
                                      "credit_capacity")]
        np.testing.assert_array_equal(out.credit, state.credit)
        np.testing.assert_array_equal(out.flux, state.flux)
    else:
        assert len(events.moves) == 1 and not events.redirects
        assert out.direction[source_index, slot] == 0
        assert out.direction[destination_index, slot] == direction_code
        assert int(out.credit[destination_index, slot]) == int(operation == "retract")
        assert out.flux[site(L, owner), axis] == (-epsilon * eta if operation == "extend" else 0)


def test_flux_capacity_refusal_uses_only_local_guide_and_preserves_account():
    state = F.initialize(4)
    add_flux_loop(state, (0, 0, 0))
    state.direction[0, :] = (3, 1)  # plus attempts +y against existing -flux
    state = replace(state, microtick=2)
    out, events = pure_step(state)
    assert events.redirects == [(1, 0, 3, 1, "flux_capacity")]
    assert not events.moves
    assert F.account_units(out) == F.account_units(state) == 4
    np.testing.assert_array_equal(out.credit, state.credit)
    np.testing.assert_array_equal(out.flux, state.flux)


def test_declared_free_routing_counterfactual_does_not_restore():
    # This reference deletes flux/Gauss and credit-limited transport. It is not
    # a legal candidate history and makes no conserved physical-energy claim.
    for delta in DIRECTIONS:
        positions = {1: (4, 4, 4), -1: plus((4, 4, 4), delta)}
        velocities = {1: (-1, 0, 0), -1: (1, 0, 0)}
        for tick in range(24):
            epsilon = 1 if tick % 12 < 6 else -1
            axis, parity = (tick % 6) // 2, tick % 2
            x, v = positions[epsilon], velocities[epsilon]
            attempted = tuple((1 if x[axis] % 2 == parity else -1) if a == axis else 0 for a in range(3))
            if v == attempted:
                positions[epsilon] = plus(x, v)
        assert sum(abs(b - a) for a, b in zip(positions[1], positions[-1])) > 2


@pytest.mark.parametrize("delta", DIRECTIONS[2:])
def test_guidance_disabled_control_preserves_account_but_never_restores_transverse_offset(delta):
    # Explicit separate rule: flux-hop-credit-reflection-control-1. Only the
    # velocity guide is replaced by reversal; this is not candidate replay.
    # Its directions stay on the x axis analytically, so the nonzero transverse
    # displacement can never vanish at any finite future time.
    prepared = preparation(delta)
    particles, flux = independent_oracle(prepared)
    axis = next(i for i, value in enumerate(delta) if value)
    for tick in range(144):
        oracle_tick(particles, flux, tick, prepared.L, guide=False)
        assert all(v in ((1, 0, 0), (-1, 0, 0)) for _, v, _ in particles.values())
        assert (particles[-1][0][axis] - particles[1][0][axis]) % prepared.L == delta[axis] % prepared.L
        assert particles[-1][0] != particles[1][0]
        assert sum(q * q for q in flux.values()) + sum(row[2] for row in particles.values()) == 2
        divergence = {}
        for (owner, a), q in flux.items():
            head = plus(owner, DIRECTIONS[2 * a], prepared.L)
            divergence[owner] = divergence.get(owner, 0) + q
            divergence[head] = divergence.get(head, 0) - q
        charge = {}
        for epsilon, (position, _, _) in particles.items():
            charge[position] = charge.get(position, 0) + epsilon
        assert {x: value for x, value in divergence.items() if value} == {x: value for x, value in charge.items() if value}


@pytest.mark.parametrize("phase", range(12))
@pytest.mark.parametrize("origin", [(2, 2, 2), (0, 0, 0)])
def test_complete_phase_owned_replay_and_sparse_oracle_across_seams(phase, origin):
    state = replace(preparation((0, -1, 0), origin, 4), microtick=phase)
    snapshot = F.checkpoint(state)
    restored = F.restore(snapshot)
    particles, flux = independent_oracle(state)
    assert F.checkpoint(restored) == snapshot
    for name in F.NAMES:
        assert getattr(restored, name).flags.owndata
        assert all(not np.shares_memory(getattr(restored, name), getattr(state, old)) for old in F.NAMES)
    for tick in range(phase, phase + 24):
        oracle_tick(particles, flux, tick, state.L)
        state, left = pure_step(state)
        restored, right = pure_step(restored)
        assert left == right
        assert F.checkpoint(state) == F.checkpoint(restored)
        compare_oracle(state, particles, flux)
        assert F.account_units(state) == 2


@pytest.mark.parametrize("phase", range(12))
def test_huge_ordinal_all_phases_without_global_integer_limit_change(phase):
    limit = sys.get_int_max_str_digits()
    state = replace(F.initialize(4), microtick=12 * 10 ** 5000 + phase)
    saved = F.checkpoint(state)
    restored = F.restore(saved)
    assert restored.microtick == state.microtick
    actual, events = F.step(restored)
    assert actual.microtick == state.microtick + 1
    assert events == F.FluxEvents()
    assert F.checkpoint(restored) == saved
    assert sys.get_int_max_str_digits() == limit


@pytest.mark.parametrize("phase", range(12))
def test_matching_partition_has_disjoint_endpoint_writes(phase):
    L = 4
    axis, parity = (phase % 6) // 2, phase % 2
    sites = []
    for coords in product(range(L), repeat=3):
        if coords[axis] % 2 == parity:
            sites.extend((site(L, coords), site(L, plus(coords, DIRECTIONS[2 * axis], L))))
    assert sorted(sites) == list(range(L ** 3))


@pytest.mark.parametrize("phase", range(12))
def test_capacity_hold_and_inactive_polarity_are_explicit(phase):
    state = F.initialize(4)
    slot, axis, parity = phase // 6, (phase % 6) // 2, phase % 2
    owner_coords = tuple(parity if a == axis else 0 for a in range(3))
    owner = site(4, owner_coords)
    head = site(4, plus(owner_coords, DIRECTIONS[2 * axis], 4))
    # Neutral co-located pairs at both ends are legal with zero flux.
    state.direction[[owner, head], :] = 2 * axis + 1
    state.credit[[owner, head], :] = 1
    state = replace(state, microtick=phase)
    out, events = pure_step(state)
    assert events.capacity_holds == [(1 if slot == 0 else -1, owner, axis)]
    assert not events.moves and not events.redirects
    for name in F.NAMES:
        np.testing.assert_array_equal(getattr(out, name), getattr(state, name))


def test_routing_expiry_has_distinct_complete_preimages_and_exact_forward_replay():
    left = preparation((0, -1, 0))
    origin = site(8, (4, 4, 4))
    left.credit[origin, 0] = 0
    left.direction[origin, 0] = 1  # denied +x departure; unique flux port is -y
    right = F.restore(F.checkpoint(left))
    right.direction[origin, 0] = 4  # already guided; holds at this active edge
    assert F.checkpoint(left) != F.checkpoint(right)
    left, le = F.step(left)
    right, re = F.step(right)
    assert le != re
    assert F.checkpoint(left) == F.checkpoint(right)
    for _ in range(12):
        left, le = F.step(left)
        right, re = F.step(F.restore(F.checkpoint(right)))
        assert le == re and F.checkpoint(left) == F.checkpoint(right)


def add_flux_loop(state, origin, first=0, second=1):
    a, b = DIRECTIONS[2 * first], DIRECTIONS[2 * second]
    state.flux[site(state.L, origin), first] += 1
    state.flux[site(state.L, plus(origin, a, state.L)), second] += 1
    state.flux[site(state.L, plus(origin, b, state.L)), first] -= 1
    state.flux[site(state.L, origin), second] -= 1


@pytest.mark.parametrize("phase", range(12))
def test_legal_interventions_cover_every_record_family_with_radius_one_output(phase):
    # Direction/credit perturbations are single-owner legal interventions.
    # Flux perturbations are closed plaquette loops preserving Gauss, so their
    # complete input-owner support is used. This is not an all-state proof.
    L, origin = 6, (0, 0, 0)
    for name in F.NAMES:
        left = F.initialize(L)
        x = site(L, origin)
        left.direction[x] = (1, 2)
        if name == "credit":
            left.credit[x] = (1, 1)
        left = replace(left, microtick=phase)
        right = F.restore(F.checkpoint(left))
        if name == "direction": right.direction[x, phase // 6] = 3
        elif name == "credit": right.credit[x, phase // 6] = 0
        else: add_flux_loop(right, origin)
        F.validate(left); F.validate(right)
        changed_inputs = set()
        for key in F.NAMES:
            changed_inputs.update(map(int, np.flatnonzero(np.any(getattr(left, key) != getattr(right, key), axis=1))))
        after_left, _ = F.step(left)
        after_right, _ = F.step(right)
        allowed = set()
        for owner in changed_inputs:
            for offset in product((-1, 0, 1), repeat=3):
                allowed.add(site(L, plus(xyz(L, owner), offset, L)))
        for key in F.NAMES:
            changes = set(map(int, np.flatnonzero(np.any(getattr(after_left, key) != getattr(after_right, key), axis=1))))
            assert changes <= allowed, (phase, name, key, changes - allowed)
        assert F.account_units(after_left) == F.account_units(left)
        assert F.account_units(after_right) == F.account_units(right)


def test_guide_star_can_change_only_source_direction_on_denial():
    state = preparation((0, -1, 0))
    origin = site(8, (4, 4, 4))
    state.direction[origin, 0] = 1
    state.credit[origin, 0] = 0
    out, events = F.step(state)
    assert events.redirects == [(1, origin, 1, 4, "credit_deficit")]
    np.testing.assert_array_equal(out.credit, state.credit)
    np.testing.assert_array_equal(out.flux, state.flux)
    assert np.argwhere(out.direction != state.direction).tolist() == [[origin, 0]]


@pytest.mark.parametrize("L", [True, 3, 5, 4.0, None, -2])
def test_invalid_probe_sizes_rejected(L):
    with pytest.raises(ValueError): F.initialize(L)


@pytest.mark.parametrize("name,value", [("direction", 7), ("credit", 2), ("flux", 2)])
def test_invalid_alphabet_rejected_before_mutation(name, value):
    state = F.initialize(4)
    getattr(state, name).flat[0] = value
    saved = {key: getattr(state, key).tobytes() for key in F.NAMES}
    for operation in (F.step, F.checkpoint, F.account_units, F.populations, F.manifestation):
        with pytest.raises(ValueError): operation(state)
        assert saved == {key: getattr(state, key).tobytes() for key in F.NAMES}


def test_shapes_dtype_gauss_empty_credit_and_ownership():
    state = preparation()
    clone = F.initialize(state.L, state.direction, state.credit, state.flux)
    assert all(not np.shares_memory(getattr(clone, name), getattr(state, old)) for name in F.NAMES for old in F.NAMES)
    with pytest.raises(ValueError): F.validate(replace(state, credit=state.direction))
    with pytest.raises(ValueError): F.validate(replace(state, direction=state.direction.astype(np.int8)))
    with pytest.raises(ValueError): F.validate(replace(state, flux=state.flux[:, :2]))
    with pytest.raises(ValueError): F.initialize(4, None, state.credit, state.flux)
    invalid = F.initialize(4); invalid.credit[0, 0] = 1
    with pytest.raises(ValueError, match="empty"): F.step(invalid)
    invalid = F.initialize(4); invalid.direction[0, 0] = 1
    with pytest.raises(ValueError, match="divergence"): F.step(invalid)


@pytest.mark.parametrize("clock", [-1, True, 0.0, None])
def test_invalid_clock_rejected(clock):
    with pytest.raises(ValueError): F.step(replace(F.initialize(4), microtick=clock))


def encode(payload):
    return F.MAGIC + json.dumps(payload, sort_keys=True, separators=(",", ":")).encode()


@pytest.mark.parametrize("key", ["schema", "law", "rule", "encoding", "backend", "boundary"])
def test_checkpoint_identity_rejection(key):
    payload = json.loads(F.checkpoint(F.initialize(4))[len(F.MAGIC):])
    payload[key] = "foreign"
    with pytest.raises(ValueError): F.restore(encode(payload))


@pytest.mark.parametrize("name", F.NAMES)
def test_checkpoint_all_arrays_required_canonical_bounded_and_legal(name):
    initial = F.checkpoint(F.initialize(4))
    for case in ("missing", "oversized", "truncated", "alphabet", "bad_base64"):
        payload = json.loads(initial[len(F.MAGIC):])
        if case == "missing": del payload["arrays"][name]
        elif case == "bad_base64": payload["arrays"][name] = "!"
        else:
            data = bytearray(base64.b64decode(payload["arrays"][name]))
            if case == "oversized": data.extend(b"\x00" * 3)
            elif case == "truncated": data.pop()
            else: data[0] = 127
            payload["arrays"][name] = base64.b64encode(data).decode()
        with pytest.raises(ValueError): F.restore(encode(payload))


@pytest.mark.parametrize("value", ["", "00", "01", "0x1", "A", "+1", "-1", "1 ", "1_0", None, 0, True])
def test_checkpoint_canonical_hex_ordinal(value):
    payload = json.loads(F.checkpoint(F.initialize(4))[len(F.MAGIC):])
    payload["microtick_hex"] = value
    with pytest.raises(ValueError): F.restore(encode(payload))


def test_foreign_states_checkpoints_duplicate_keys_and_forged_state_identity():
    from phi_v2_lattice import staged as P, state as S, staged_alignment as A
    original = P.initialize(S.blank(4))
    alignment = A.initialize(S.blank(4))
    for state, save in ((original, P.checkpoint), (alignment, A.checkpoint)):
        with pytest.raises(ValueError): F.step(state)
        with pytest.raises(ValueError): F.restore(save(state))
    with pytest.raises(ValueError): P.step(F.initialize(4))
    with pytest.raises(ValueError): A.step(F.initialize(4))
    with pytest.raises(ValueError): F.restore(F.MAGIC + b'{"L":4,"L":4}')
    for name in ("law_id", "rule_hash", "encoding_hash", "boundary"):
        state = F.initialize(4)
        object.__setattr__(state, name, "foreign")
        with pytest.raises(ValueError): F.step(state)
        with pytest.raises(ValueError): F.checkpoint(state)
