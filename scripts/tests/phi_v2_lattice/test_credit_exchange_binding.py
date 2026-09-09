"""Frozen nineteen-stage law: complete records, covariance and fixed witnesses.

The full 941184-state scientific census belongs to the independent recovery
module. These are upstream acceptance controls, with their actual matrices
explicit below; no choice is tuned to a measured trajectory.
"""
import base64
from dataclasses import fields, replace
import hashlib
from itertools import product
import json
from pathlib import Path
import sys

import numpy as np
import pytest

from phi_v2_lattice import balanced_matching as B
from phi_v2_lattice import credit_exchange_binding as E
from phi_v2_lattice import flux_binding as OLD
from phi_v2_lattice import recorded_matching as M

# Frozen independent geometry/fixture helpers, never transition decisions.
import test_recorded_matching as R


def lift(state, eta=0, patterned=False):
    attempted = np.zeros_like(state.direction)
    if patterned:
        attempted[:] = (np.indices(attempted.shape).sum(axis=0) % 2).astype(np.uint8)
        attempted[state.direction == 0] = 0
    result = E.initialize(state.L, state.direction, state.credit, state.flux,
                          state.matching_frame, E.seed_charge_frame(state.L, eta), attempted)
    return replace(result, microtick=state.microtick)


def blank(eta=0, code=0, L=4):
    return lift(R.blank(L, code), eta)


def same(left, right):
    assert left.L == right.L and left.microtick == right.microtick
    for name in E.NAMES:
        np.testing.assert_array_equal(getattr(left, name), getattr(right, name))


def conjugate(state):
    return E.ExchangeState(state.L, state.microtick, state.direction[:, ::-1].copy(),
                           state.credit[:, ::-1].copy(), -state.flux,
                           state.matching_frame.copy(), state.charge_frame ^ 1,
                           state.attempted[:, ::-1].copy())


def spatial(state, g=R.IDENTITY, offset=(0, 0, 0)):
    transformed = R.transform(state, g, offset)
    L = state.L
    xyz = np.indices((L, L, L)).reshape(3, -1).T
    moved = (xyz @ np.asarray(g).T + np.asarray(offset)) % L
    index = (moved[:, 0] * L + moved[:, 1]) * L + moved[:, 2]
    charge, attempted = np.empty_like(state.charge_frame), np.empty_like(state.attempted)
    charge[index], attempted[index] = state.charge_frame, state.attempted
    return E.ExchangeState(L, state.microtick, transformed.direction, transformed.credit,
                           transformed.flux, transformed.matching_frame, charge, attempted)


def normalized(events):
    return tuple(tuple(sorted(getattr(events, field.name))) for field in fields(E.ExchangeEvents))


def conjugate_events(events):
    result = E.ExchangeEvents()
    result.moves = [(-eps, src, dst, own, axis, -q0, -q1, k0, k1)
                    for eps, src, dst, own, axis, q0, q1, k0, k1 in events.moves]
    result.redirects = [(-eps, x, v, w, reason) for eps, x, v, w, reason in events.redirects]
    result.capacity_holds = [(-eps, x, axis) for eps, x, axis in events.capacity_holds]
    result.attempt_marks = [(-eps, x, reason) for eps, x, reason in events.attempt_marks]
    result.attempt_expiries = [(-eps, x) for eps, x in events.attempt_expiries]
    result.onsite_alignments = [(x, -eps, minus, plus, w)
                               for x, eps, plus, minus, w in events.onsite_alignments]
    result.credit_exchanges = [(own, axis, -de, d, -ae, a, d0, a0, d1, a1, dv, av, w)
                              for own, axis, de, d, ae, a, d0, a0, d1, a1, dv, av, w
                              in events.credit_exchanges]
    return result


def events_spatial(events, L, g=R.IDENTITY, offset=(0, 0, 0)):
    result = E.ExchangeEvents()
    result.moves, result.redirects, result.capacity_holds = R.events_transform(events, L, g, offset)
    direction_map, _, edge_map = R.ACTION_DATA[g]
    def moved(x):
        return R.site(L, R.add(R.vector(g, R.xyz(L, x)), offset))
    result.attempt_marks = [(eps, moved(x), reason) for eps, x, reason in events.attempt_marks]
    result.attempt_expiries = [(eps, moved(x)) for eps, x in events.attempt_expiries]
    result.onsite_alignments = [(moved(x), eps, direction_map[p], direction_map[m], direction_map[w])
                               for x, eps, p, m, w in events.onsite_alignments]
    for own, axis, de, d, ae, a, d0, a0, d1, a1, dv, av, w in events.credit_exchanges:
        new_axis, sign = edge_map[axis]
        point = R.add(R.vector(g, R.xyz(L, own)), offset)
        if sign < 0:
            point = R.add(point, R.V[2 * new_axis + 1])
        result.credit_exchanges.append((R.site(L, point), new_axis, de, moved(d), ae, moved(a),
                                        d0, a0, d1, a1, direction_map[dv], direction_map[av], direction_map[w]))
    return result


def pure_step(state):
    before = {name: getattr(state, name).tobytes() for name in E.NAMES}
    result, events = E.step(state)
    for name in E.NAMES:
        array = getattr(result, name)
        assert array.flags.owndata and array.flags.c_contiguous
        assert before[name] == getattr(state, name).tobytes()
        assert all(not np.shares_memory(array, getattr(state, old)) for old in E.NAMES)
        assert all(not np.shares_memory(array, getattr(result, other)) for other in E.NAMES if name != other)
    return result, events


def add_oriented_edge(state, start, v, epsilon=1):
    axis = next(a for a, n in enumerate(v) if n)
    sign = v[axis]
    owner = start if sign == 1 else R.add(start, v)
    state.flux[R.site(state.L, owner), axis] += epsilon * sign


def pair(path=((1, 0, 0),), heads=(1, 1), credits=(1, 0), flags=(0, 0),
         *, origin=(0, 0, 0), eta=0, code=0, L=6, phase=0):
    state = blank(eta, code, L)
    plus, minus = origin, origin
    for v in path:
        add_oriented_edge(state, minus, v)
        minus = R.add(minus, v)
    for slot, point in enumerate((plus, minus)):
        x = R.site(L, point)
        state.direction[x, slot], state.credit[x, slot], state.attempted[x, slot] = heads[slot], credits[slot], flags[slot]
    state = replace(state, microtick=phase)
    E.validate(state)
    return state


def locations(state):
    return tuple(int(np.flatnonzero(state.direction[:, p])[0]) for p in range(2))


def separation(state):
    p, m = locations(state)
    a, b = R.xyz(state.L, p), R.xyz(state.L, m)
    return sum(min((x - y) % state.L, (y - x) % state.L) for x, y in zip(a, b))


def in_M(state):
    if state.phase != 1 or np.any(state.attempted) or int(np.abs(state.flux).sum()) != 1:
        return False
    places = locations(state)
    full = next((p for p in range(2) if state.credit[places[p], p] == 1), None)
    if full is None:
        return False
    donor, recipient = places[full], places[1 - full]
    heading = int(state.direction[donor, full])
    return (state.direction[recipient, 1 - full] == heading
            and R.site(state.L, R.add(R.xyz(state.L, donor), R.V[heading - 1])) == recipient)


def kick_full_endpoint(state, v):
    result = E.restore(E.checkpoint(state))
    places = locations(result)
    slot = next(p for p in range(2) if result.credit[places[p], p] == 1)
    source = places[slot]
    point = R.xyz(result.L, source)
    destination = R.site(result.L, R.add(point, v))
    assert destination != places[1 - slot]
    # The selected comparison preparation is exactly the local hop-account
    # arithmetic; it is not another stage or runtime actuator.
    add_oriented_edge(result, point, v, -1 if slot == 0 else 1)
    result.direction[destination, slot] = result.direction[source, slot]
    result.attempted[destination, slot] = result.attempted[source, slot]
    result.credit[destination, slot] = 0
    result.direction[source, slot] = result.credit[source, slot] = result.attempted[source, slot] = 0
    E.validate(result)
    assert E.account_units(result) == E.account_units(state) == 2
    assert separation(result) == 2
    return result


def test_preimplementation_spec_and_frozen_dependency_identities():
    expected = ((B.__file__, "5bd68b23b7dfa089103e4c593cbb4a739d0ed395e828112ab84d2dc5bc7e2c62"),
                (M.__file__, "f81c72673159142158f863d06e8d5b3847d4e239f8df8752c2efbbf0e6a97172"),
                (OLD.__file__, "33203442187324ebf8d0f4c47668ffe0f3c5aa2d918284dab46fdb4aa6991740"),
                (R.__file__, "b2eec1c5d7fb4f180df56626b24e531e1eb2c3c238b09e966f11a8c94abacb52"))
    for filename, expected_hash in expected:
        assert hashlib.sha256(Path(filename).read_bytes()).hexdigest() == expected_hash
    spec = Path(E.__file__).parents[2] / "engine/docs/SPEC_STRICT_CREDIT_EXCHANGE_BINDING_V1.md"
    assert hashlib.sha256(spec.read_bytes()).hexdigest() == "13141e269ff91458b96744f3ec2b2ea87cc4ee13b3d7f3fe75df80308ea8e106"
    assert E.seed_background is M.seed_background and E.seed_charge_frame is B.seed_charge_frame
    assert E.FRAME_HASH == M.FRAME_HASH and len(E.TRIADS) == 48
    with pytest.raises(TypeError):
        E.RULE_DESCRIPTION["law"] = "foreign"


@pytest.mark.parametrize("eta,code", product(range(2), range(48)))
def test_all_cubic_actions_balanced_compositions_backgrounds_and_nineteen_phases(eta, code):
    # 2 eta x48 B x19 phases x48 cubic actions x2 C choices on a fixed,
    # nonuniform legal multiparticle fixture. This is not all global states.
    base = lift(R.rich(code), eta, patterned=True)
    for phase in range(19):
        state = replace(base, microtick=phase)
        output, events = E.step(state)
        for g in R.ACTIONS:
            transformed, expected = spatial(state, g, (3, 1, 3)), spatial(output, g, (3, 1, 3))
            actual, actual_events = E.step(transformed)
            same(actual, expected)
            assert normalized(actual_events) == normalized(events_spatial(events, 4, g, (3, 1, 3)))
            actual_c, events_c = E.step(conjugate(transformed))
            same(actual_c, conjugate(expected))
            assert normalized(events_c) == normalized(events_spatial(conjugate_events(events), 4, g, (3, 1, 3)))
            same(conjugate(transformed), spatial(conjugate(state), g, (3, 1, 3)))


@pytest.mark.parametrize("eta,code", product(range(2), range(48)))
def test_signed_unit_translations_and_C_all_phases(eta, code):
    base = lift(R.rich(code), eta, patterned=True)
    for phase in range(19):
        state = replace(base, microtick=phase)
        output, events = E.step(state)
        for delta in R.V:
            transformed = spatial(state, offset=delta)
            actual, actual_events = E.step(transformed)
            same(actual, spatial(output, offset=delta))
            assert normalized(actual_events) == normalized(events_spatial(events, 4, offset=delta))
            actual_c, events_c = E.step(conjugate(transformed))
            same(actual_c, conjugate(spatial(output, offset=delta)))
            assert normalized(events_c) == normalized(events_spatial(conjugate_events(events), 4, offset=delta))


@pytest.mark.parametrize("eta,code", product(range(2), range(48)))
def test_actual_credit_exchange_state_and_events_all_cubic_actions_and_C(eta, code):
    # The dense fixture above happens to emit no exchange event. Explicitly
    # populate both endpoints of its existing q=+1 edge, with unequal credit,
    # so the new event's covariance cannot pass on an empty event list.
    state = lift(R.rich(code), eta, patterned=True)
    owner, head = 0, R.site(4, (1, 0, 0))
    state.direction[owner], state.direction[head] = (3, 4), (5, 6)
    state.credit[owner, 0], state.credit[head, 1] = 1, 0
    column = next(a for a, v in enumerate(R.TRIADS[code]) if v[0])
    state = replace(state, microtick=1+2*column+((code >> column) & 1))
    output, events = E.step(state)
    assert events.credit_exchanges and any(e[0:2] == (owner, 0) for e in events.credit_exchanges)
    for g in R.ACTIONS:
        actual, actual_events = E.step(spatial(state, g, (3, 1, 3)))
        same(actual, spatial(output, g, (3, 1, 3)))
        assert normalized(actual_events) == normalized(events_spatial(events, 4, g, (3, 1, 3)))
        actual_c, events_c = E.step(conjugate(spatial(state, g, (3, 1, 3))))
        same(actual_c, conjugate(spatial(output, g, (3, 1, 3))))
        assert normalized(events_c) == normalized(events_spatial(conjugate_events(events), 4, g, (3, 1, 3)))


@pytest.mark.parametrize("eta,code", product(range(2), range(48)))
def test_every_hop_account_case_and_attempt_eligibility(eta, code):
    for r, q, k, sign, attempted in product(range(12), (-1, 0, 1), range(2), (-1, 1), range(2)):
        state = replace(blank(eta, code), microtick=7 + r)
        a, bit = (r % 6) // 2, r % 2
        slot = (r // 6) ^ eta
        epsilon = 1 if slot == 0 else -1
        axis = next(j for j, value in enumerate(R.TRIADS[code][a]) if value)
        point = tuple(int(((code >> a) & 1) != bit) if j == axis else 0 for j in range(3))
        owner, head = R.site(4, point), R.site(4, R.add(point, R.V[2 * axis]))
        source, destination = (owner, head) if sign == 1 else (head, owner)
        heading = 2 * axis + (1 if sign == 1 else 2)
        state.direction[source] = (1, 2)
        state.direction[source, slot] = heading
        state.credit[source, slot] = k
        state.attempted[source, slot] = attempted
        if q:
            R.loop(state, point, axis, (axis + 1) % 3)
            state.flux[:] *= q
        output, events = E.step(state)
        assert E.account_units(output) == E.account_units(state) and E.populations(output) == (1, 1)
        if attempted:
            same(output, replace(state, microtick=state.microtick + 1))
            assert normalized(events) == normalized(E.ExchangeEvents())
            continue
        new_q = q - epsilon * sign
        new_k = k - (new_q ** 2 - q ** 2)
        accepted = -1 <= new_q <= 1 and 0 <= new_k <= 1
        if accepted:
            assert events.moves == [(epsilon, source, destination, owner, axis, q, new_q, k, new_k)]
            assert events.attempt_marks == [(epsilon, source, "accepted")]
            assert output.direction[destination, slot] == heading and output.credit[destination, slot] == new_k
            assert output.attempted[destination, slot] == 1
            assert output.direction[source, slot] == output.credit[source, slot] == output.attempted[source, slot] == 0
        else:
            reason = "flux_capacity" if abs(new_q) > 1 else "credit_deficit" if new_k < 0 else "credit_capacity"
            assert events.attempt_marks == [(epsilon, source, reason)]
            outward = []
            for j in range(3):
                outward += [int(state.flux[source, j]), -int(state.flux[R.site(4, R.add(R.xyz(4, source), R.V[2*j+1])), j])]
            ports = [i + 1 for i, value in enumerate(outward) if value == epsilon]
            guide = ports[0] if k == 0 and len(ports) == 1 else (heading + 1 if heading % 2 else heading - 1)
            assert events.redirects == [(epsilon, source, heading, guide, reason)]
            assert output.attempted[source, slot] == 1
            np.testing.assert_array_equal(output.credit, state.credit)
            np.testing.assert_array_equal(output.flux, state.flux)


@pytest.mark.parametrize("eta,code", product(range(2), (0, 47)))
def test_exchange_admission_all_q_credit_presence_classes_and_control_bits(eta, code):
    for r, q, k0, k1, presence, bits in product(range(6), (-1, 0, 1), range(2), range(2), range(4), range(4)):
        state = replace(blank(eta, code), microtick=1 + r)
        a, bit = r // 2, r % 2
        axis = next(j for j, value in enumerate(R.TRIADS[code][a]) if value)
        point = tuple(int(((code >> a) & 1) != bit) if j == axis else 0 for j in range(3))
        owner, head = R.site(4, point), R.site(4, R.add(point, R.V[2 * axis]))
        left_slot, right_slot = (0, 1) if q >= 0 else (1, 0)
        if presence & 1:
            state.direction[owner] = (3, 4)
            state.credit[owner, left_slot] = k0
            state.attempted[owner, left_slot] = bits & 1
        if presence & 2:
            state.direction[head] = (5, 6)
            state.credit[head, right_slot] = k1
            state.attempted[head, right_slot] = bits >> 1
        if q:
            R.loop(state, point, axis, (axis + 1) % 3)
            state.flux[:] *= q
        output, events = E.step(state)
        assert E.account_units(output) == E.account_units(state)
        assert E.populations(output) == E.populations(state)
        np.testing.assert_array_equal(output.flux, state.flux)
        np.testing.assert_array_equal(output.attempted, state.attempted)
        if q and presence == 3 and k0 != k1:
            donor, recipient, dp, rp = (owner, head, left_slot, right_slot) if k0 else (head, owner, right_slot, left_slot)
            heading = 2 * axis + (1 if k0 else 2)
            assert events.credit_exchanges == [(owner, axis, 1 if dp == 0 else -1, donor,
                1 if rp == 0 else -1, recipient, 1, 0, 0, 1,
                int(state.direction[donor, dp]), int(state.direction[recipient, rp]), heading)]
            expected_credit, expected_heading = state.credit.copy(), state.direction.copy()
            expected_credit[donor, dp], expected_credit[recipient, rp] = 0, 1
            expected_heading[donor, dp] = expected_heading[recipient, rp] = heading
            np.testing.assert_array_equal(output.credit, expected_credit)
            np.testing.assert_array_equal(output.direction, expected_heading)
        else:
            same(output, replace(state, microtick=state.microtick + 1))
            assert not events.credit_exchanges


@pytest.mark.parametrize("eta", range(2))
def test_capacity_holds_consume_only_addressed_unattempted_slots(eta):
    for r, flags, pointed in product(range(12), range(4), range(4)):
        state = replace(blank(eta), microtick=7+r)
        axis, parity = (r % 6)//2, r % 2
        point = tuple(parity if j == axis else 0 for j in range(3))
        owner, head = R.site(4, point), R.site(4, R.add(point, R.V[2*axis]))
        slot = (r//6) ^ eta
        state.direction[owner] = state.direction[head] = (1, 2)
        for j, x in enumerate((owner, head)):
            state.direction[x, slot] = 2*axis+1+j if pointed >> j & 1 else 2*((axis+1)%3)+1
            state.attempted[x, slot] = flags >> j & 1
        out, events = E.step(state)
        for name in E.NAMES[:-1]:
            np.testing.assert_array_equal(getattr(out, name), getattr(state, name))
        expected = state.attempted.copy()
        for j, x in enumerate((owner, head)):
            if pointed >> j & 1:
                expected[x, slot] = 1
        np.testing.assert_array_equal(out.attempted, expected)
        assert not events.moves and not events.redirects


@pytest.mark.parametrize("phase,eta,code", product(range(19), range(2), (0, 47)))
def test_owned_complete_replay_and_account_every_phase(phase, eta, code):
    state = replace(lift(R.rich(code), eta, patterned=True), microtick=phase)
    saved = E.checkpoint(state)
    restored = E.restore(saved)
    same(state, restored)
    assert E.checkpoint(restored) == saved
    account, counts = E.account_units(state), E.populations(state)
    for name in E.NAMES:
        assert all(not np.shares_memory(getattr(restored, name), getattr(state, old)) for old in E.NAMES)
    for _ in range(38):
        state, events = pure_step(state)
        restored, replay = pure_step(restored)
        same(state, restored)
        assert events == replay and E.checkpoint(state) == E.checkpoint(restored)
        assert E.account_units(state) == account and E.populations(state) == counts
        np.testing.assert_array_equal(E.manifestation(conjugate(state)), -E.manifestation(state))


@pytest.mark.parametrize("phase,eta", product(range(19), range(2)))
def test_huge_ordinal_hex_replay_C_and_integral_normalization(phase, eta):
    limit = sys.get_int_max_str_digits()
    state = replace(lift(R.rich(47), eta, patterned=True), microtick=19*10**5000+phase)
    restored = E.restore(E.checkpoint(state))
    same(restored, state)
    same(conjugate(conjugate(state)), state)
    output, events = E.step(restored)
    expected, expected_events = E.step(replace(state, microtick=phase))
    same(replace(output, microtick=phase+1), expected)
    assert output.microtick == state.microtick+1 and events == expected_events
    out_c, events_c = E.step(conjugate(state))
    same(out_c, conjugate(output))
    assert normalized(events_c) == normalized(conjugate_events(events))
    uint = replace(state, L=np.uint64(state.L), microtick=np.uint64(phase))
    uint_out, uint_events = E.step(uint)
    same(uint_out, expected)
    assert uint_events == expected_events and type(uint_out.microtick) is int
    assert sys.get_int_max_str_digits() == limit


@pytest.mark.parametrize("eta,code", product(range(2), range(48)))
def test_constructive_38_charged_tick_complete_translation_all_directions_and_roles(eta, code):
    # 48 B x2 eta x6 directions x2 leading polarities, including seams.
    for v, plus_trails in product(R.V, (False, True)):
        origin = tuple(5 if n > 0 else 0 for n in v)
        state = pair((v,), (R.V.index(v)+1,)*2, (1, 0), origin=origin, eta=eta, code=code)
        if not plus_trails:
            state = conjugate(state)
            # Restore the requested eta while preserving physical roles.
            state = replace(state, charge_frame=E.seed_charge_frame(6, eta))
        state, reset_events = E.step(state)
        assert state.microtick == 1 and in_M(state)
        start = state
        moves, exchanges = [], []
        for _ in range(38):
            state, events = E.step(state)
            moves += events.moves
            exchanges += events.credit_exchanges
            assert E.account_units(state) == 2 and E.populations(state) == (1, 1)
            assert separation(state) <= 2
        expected = spatial(start, offset=tuple(2*n for n in v))
        same(state, replace(expected, microtick=39))
        assert in_M(state) and len(moves) == 4 and len(exchanges) == 2
        # The complete ordinal really advanced, so checkpoints do not recur.
        assert E.checkpoint(state) != E.checkpoint(start)


@pytest.mark.parametrize("eta,plus_trails,v", product(range(2), (False, True), R.V))
def test_five_prespecified_displacement_kicks_contract_and_reenter_moving_family(eta, plus_trails, v):
    # The sharper phase1 witness: all 6 headings, both physical leading roles
    # and orders, five legal kicks. Full backgrounds are covered by covariance;
    # the independent graph separately retains every preparation instance.
    state = pair((v,), (R.V.index(v)+1,)*2, (1, 0), eta=eta)
    if not plus_trails:
        state = conjugate(state)
        state = replace(state, charge_frame=E.seed_charge_frame(6, eta))
    state, _ = E.step(state)
    assert in_M(state)
    inward = v
    for kick in R.V:
        if kick == inward:
            continue
        displaced = kick_full_endpoint(state, kick)
        assert E.account_units(displaced) == 2 and separation(displaced) == 2
        first_contraction, first_M = None, None
        for tick in range(1, 77):
            displaced, _ = E.step(displaced)
            assert E.account_units(displaced) == 2 and separation(displaced) <= 2
            if first_contraction is None and separation(displaced) <= 1:
                first_contraction = tick
            if in_M(displaced):
                first_M = tick
                break
        assert first_contraction is not None and first_contraction <= 38
        assert first_M is not None and first_M <= 76
        before = displaced
        places = locations(before)
        full = next(p for p in range(2) if before.credit[places[p], p])
        drift = R.V[int(before.direction[places[full], full])-1]
        for _ in range(38):
            displaced, _ = E.step(displaced)
        same(displaced, replace(spatial(before, offset=tuple(2*n for n in drift)), microtick=before.microtick+38))


@pytest.mark.parametrize("eta", range(2))
def test_all_starting_phases_flags_coincident_and_bent_path_preparations_reach_M(eta):
    # Fixed complete phase/flag/heading matrices for R0 and a bent R2 path;
    # this is not a replacement for the parent's entire quotient census.
    for path, credits in (((), (1, 1)), (((1, 0, 0), (0, 1, 0)), (0, 0))):
        for phase, bits, headings in product(range(19), range(4), ((1, 1), (2, 4), (5, 6))):
            state = pair(path, headings, credits, (bits & 1, bits >> 1), eta=eta, phase=phase)
            reached = in_M(state)
            for _ in range(95):
                if reached:
                    break
                state, _ = E.step(state)
                reached = in_M(state)
            assert reached


@pytest.mark.parametrize("eta", range(2))
def test_named_noninjective_reset_and_heading_expiry_and_colocation_denial(eta):
    state = pair((), (1, 4), (1, 1), (1, 1), eta=eta)
    changed = E.restore(E.checkpoint(state))
    changed.attempted[:] = 0
    changed.direction[0, 1-eta] = 6
    left, events = E.step(state)
    right, other_events = E.step(changed)
    same(left, right)
    assert len(events.attempt_expiries) == 2 and not other_events.attempt_expiries
    chosen = (1, 4)[eta]
    assert tuple(left.direction[0]) == (chosen, chosen)
    all_moves, all_denials = [], []
    for _ in range(18):
        left, event = E.step(left)
        all_moves += event.moves
        all_denials += event.redirects
    assert len(all_moves) == len(all_denials) == 1
    assert all_denials[0][-1] == "credit_capacity"
    assert separation(left) == 1 and int(left.attempted.sum()) == 2
    # Credit exchange, with its heading expiry, actually changes the old
    # diagnostic anchor while conserving the declared count.
    edge = pair(((1, 0, 0),), (3, 6), (1, 0), phase=1, eta=eta)
    alternate = E.restore(E.checkpoint(edge))
    alternate.direction[0, 0] = 5
    alternate.direction[R.site(6, (1, 0, 0)), 1] = 2
    output, exchange = E.step(edge)
    same(output, E.step(alternate)[0])
    assert len(exchange.credit_exchanges) == 1
    assert output.credit[0, 0] == 0 and output.credit[R.site(6, (1, 0, 0)), 1] == 1
    assert E.account_units(output) == E.account_units(edge) == 2


@pytest.mark.parametrize("eta", range(2))
def test_legal_interventions_all_stages_stay_within_radius_one(eta):
    for phase, family in product(range(19), ("direction", "credit", "flux", "attempted")):
        state = replace(blank(eta, 47, 6), microtick=phase)
        state.direction[0] = (1, 2)
        state.credit[0] = (1, 1)
        altered = E.restore(E.checkpoint(state))
        if family == "direction":
            altered.direction[0, eta] = 5
        elif family == "credit":
            altered.credit[0, eta] = 0
        elif family == "attempted":
            altered.attempted[0, eta] = 1
        else:
            R.loop(altered)
        input_owners = set()
        for name in E.NAMES:
            delta = getattr(state, name) != getattr(altered, name)
            input_owners.update(map(int, np.flatnonzero(delta if delta.ndim == 1 else delta.any(axis=1))))
        allowed = {R.site(6, R.add(R.xyz(6, x), d)) for x in input_owners for d in product((-1, 0, 1), repeat=3)}
        left, _ = E.step(state)
        right, _ = E.step(altered)
        for name in E.NAMES:
            delta = getattr(left, name) != getattr(right, name)
            output_owners = set(map(int, np.flatnonzero(delta if delta.ndim == 1 else delta.any(axis=1))))
            assert output_owners <= allowed
    # A one-site context change is illegal, not a legal locality experiment.
    for name in ("matching_frame", "charge_frame"):
        value = getattr(state, name).copy()
        value[0] ^= 1
        with pytest.raises(ValueError):
            E.step(replace(state, **{name: value}))


def test_explicit_initialization_copy_ownership_and_all_phase_flag_admission():
    base = pair((), (1, 2), (1, 1))
    for phase, bits in product(range(19), range(4)):
        attempted = base.attempted.copy()
        attempted[0] = (bits & 1, bits >> 1)
        E.validate(replace(base, microtick=phase, attempted=attempted))
    arrays = [getattr(base, name).copy() for name in E.NAMES]
    views = [a.view() for a in arrays]
    initialized = E.initialize(base.L, *views)
    for name, supplied in zip(E.NAMES, views):
        assert getattr(initialized, name).flags.owndata
        assert not np.shares_memory(getattr(initialized, name), supplied)
        with pytest.raises(ValueError):
            E.validate(replace(base, **{name: supplied}))
    with pytest.raises(TypeError):
        E.initialize(base.L, *arrays[:-1])
    with pytest.raises(ValueError):
        E.initialize(base.L, *arrays[:-1], arrays[1])
    for external in arrays:
        external.fill(0)
    same(initialized, base)


@pytest.mark.parametrize("name", E.NAMES)
def test_invalid_array_storage_and_alphabets_fail_before_mutation(name):
    state = pair((), (1, 2), (1, 1))
    original = E.checkpoint(state)
    value = getattr(state, name)
    invalid = [value.astype(np.int16), value[:-1].copy(), np.asfortranarray(value) if value.ndim == 2 else value[::2]]
    alphabet = value.copy()
    alphabet.flat[0] = 2 if name in ("credit", "attempted", "charge_frame") else 7 if name == "direction" else 48 if name == "matching_frame" else 2
    invalid.append(alphabet)
    for bad in invalid:
        corrupt = replace(state, **{name: bad})
        with pytest.raises(ValueError):
            E.step(corrupt)
        with pytest.raises(ValueError):
            E.checkpoint(corrupt)
        assert E.checkpoint(state) == original
    if name in ("credit", "attempted"):
        bad = value.copy()
        bad[1, 0] = 1
        with pytest.raises(ValueError):
            E.validate(replace(state, **{name: bad}))


@pytest.mark.parametrize("field,value", [("L", True), ("L", 3), ("L", 0), ("L", 4.0),
    ("microtick", True), ("microtick", -1), ("microtick", 1.0)])
def test_invalid_domain_and_ordinals_reject(field, value):
    state = blank()
    saved = E.checkpoint(state)
    with pytest.raises(ValueError):
        E.step(replace(state, **{field: value}))
    assert E.checkpoint(state) == saved


@pytest.mark.parametrize("dtype", (np.uint8, np.uint64, np.int8, np.int64))
def test_numpy_scalar_sizes_and_clocks_all_phases_denials_and_seams(dtype):
    for L, phase, eta in product((4, 6, 8), range(19), range(2)):
        state = pair(((1, 0, 0),), (2, 1), (0, 1), eta=eta, code=1, L=L, phase=phase)
        admitted = replace(state, L=dtype(L), microtick=dtype(phase))
        with np.errstate(over="raise", invalid="raise"):
            output, events = E.step(admitted)
        expected, expected_events = E.step(state)
        same(output, expected)
        assert events == expected_events and type(output.L) is type(output.microtick) is int
        same(E.restore(E.checkpoint(admitted)), state)
    state = replace(state, microtick=np.uint64(2**64-1), L=np.uint64(8))
    output, _ = E.step(state)
    assert output.microtick == 2**64


def payload(state):
    return json.loads(E.checkpoint(state)[len(E.MAGIC):])


def encoded(value):
    return E.MAGIC + json.dumps(value, sort_keys=True, separators=(",", ":")).encode()


@pytest.mark.parametrize("value", ("", "00", "01", "A", "0x1", "+1", "-1", " 1", "1 ", "1_0", 1, None, True, []))
def test_noncanonical_hex_ordinals_reject(value):
    state = payload(blank())
    state["microtick_hex"] = value
    with pytest.raises(ValueError):
        E.restore(encoded(state))


def test_codec_rejects_foreign_missing_extra_duplicate_truncated_and_oversized_payloads():
    state = pair((), (1, 2), (1, 1), (1, 0))
    saved = E.checkpoint(state)
    old = B.initialize(6, state.direction, state.credit, state.flux, state.matching_frame, state.charge_frame)
    for foreign in (old, M.MatchingState(6, 0, *(getattr(state, n).copy() for n in M.NAMES))):
        with pytest.raises(ValueError):
            E.step(foreign)
    for raw in (B.checkpoint(old), bytearray(saved), saved[:-1], saved+b"x", E.MAGIC+b"[]"):
        with pytest.raises(ValueError):
            E.restore(raw)
    base = payload(state)
    for key in base:
        missing = dict(base)
        del missing[key]
        with pytest.raises(ValueError):
            E.restore(encoded(missing))
    with pytest.raises(ValueError):
        E.restore(encoded(dict(base, unknown=0)))
    duplicate = saved[len(E.MAGIC):].decode().replace('"L":6', '"L":6,"L":6', 1)
    with pytest.raises(ValueError):
        E.restore(E.MAGIC+duplicate.encode())
    for name in E.NAMES:
        p = payload(state)
        del p["arrays"][name]
        with pytest.raises(ValueError):
            E.restore(encoded(p))
        for replacement in (base["arrays"][name]+"AAAA", base["arrays"][name][:-4], "!"*len(base["arrays"][name])):
            p = payload(state)
            p["arrays"][name] = replacement
            with pytest.raises(ValueError):
                E.restore(encoded(p))
    for key in ("schema", "law", "rule", "frames", "encoding", "backend", "boundary"):
        p = payload(state)
        p[key] = "foreign"
        with pytest.raises(ValueError):
            E.restore(encoded(p))
    p = payload(state)
    raw = bytearray(base64.b64decode(p["arrays"]["attempted"]))
    raw[0] = 2
    p["arrays"]["attempted"] = base64.b64encode(raw).decode()
    with pytest.raises(ValueError):
        E.restore(encoded(p))
    assert E.checkpoint(state) == saved
    # Permitted whole-input whitespace normalization preserves payload identity.
    pretty = E.MAGIC + json.dumps(base, indent=2).encode()
    assert E.checkpoint(E.restore(pretty)) == saved
    # A correct decoded byte length is insufficient: ignored base64 padding
    # bits must still be zero. L4 has a one-byte final group in this array.
    padded = payload(blank())
    canonical = padded["arrays"]["matching_frame"]
    assert canonical.endswith("==")
    alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
    index = alphabet.index(canonical[-3])
    altered = canonical[:-3] + alphabet[index+1] + "=="
    assert base64.b64decode(altered) == base64.b64decode(canonical)
    padded["arrays"]["matching_frame"] = altered
    with pytest.raises(ValueError):
        E.restore(encoded(padded))


@pytest.mark.parametrize("name", ("law_id", "rule_hash", "frame_hash", "encoding_hash", "boundary"))
def test_fixed_identity_tampering_rejected(name):
    state = blank()
    object.__setattr__(state, name, "foreign")
    with pytest.raises(ValueError):
        E.step(state)
    with pytest.raises(ValueError):
        E.checkpoint(state)
