"""Locked finite successor histories and complete-state lifecycle controls.

Matrix fixed before first trajectory execution: L=7; 56 microticks; the two
proposal witnesses; all eight nonblank backgrounds; both polarities; interior
and seam translations. The same Cartesian choices apply to the separated
same-flag negative control. Cubic/phase-shift full histories are NOT claimed.
"""
import base64
from dataclasses import replace
import json
import sys

import numpy as np
import pytest

from phi_v2_lattice import channels as C, geometry as G, staged as P, state as S, tick as T
from phi_v2_lattice import staged_alignment as A
from phi_v2_lattice._proofs import readout, rotate, encode

BACKGROUNDS = tuple(i for i in range(9) if i != S.BLANK_IDX)
HORIZON = 56
PLACEMENTS = ((3, 3, 3), (0, 0, 6))


def records(state):
    return {name: getattr(state.lattice if name in A.NAMES[:5] else state, name) for name in A.NAMES}


def compare(left, right, exclude=()):
    assert left.microtick == right.microtick
    assert left.lattice.L == right.lattice.L
    for name, value in records(left).items():
        if name not in exclude:
            np.testing.assert_array_equal(value, records(right)[name], err_msg=name)


def original_fixture(state):
    # Test-only, explicitly labelled cross-law preparation. Production restore
    # accepts neither this type nor its serialized bytes as a successor state.
    return P.StagedState(state.microtick, S.copy(state.lattice), state.admitted_sc.copy(),
                         state.gate_sc.copy(), state.gate_fcc.copy())


def preparation(witness, background, eps, center):
    st = S.blank(7)
    st.sc.fill(background)
    st.fcc.fill(background)
    st.ell.fill(2 if witness == "separated" else 0)
    x, y, z = center
    offset = 192 if eps == -1 else 0
    if witness == "contact":
        entries = [(center, 0), (center, 5)]
    elif witness == "separated":
        entries = [((x, (y - 1) % 7, z), 113), ((x, (y + 1) % 7, z), 94)]
    elif witness == "nonrestoring":
        entries = [(center, 0), (((x + 1) % 7, y, z), 1)]
    else:
        raise ValueError(witness)
    for coords, channel in entries:
        st.bank[G.site_index(7, *coords), channel + offset] = True
    return A.initialize(st)


def field_records(state):
    xs, cs = np.nonzero(state.lattice.bank)
    return sorted((G.coords(7, int(x)), int(c)) for x, c in zip(xs, cs))


def pair_predicate(state):
    entries = field_records(state)
    return (len(entries) == 2 and entries[0][0] == entries[1][0]
            and C.STATES[entries[0][1] % 192][0] == C.STATES[entries[1][1] % 192][0])


def inventory(state):
    st = state.lattice
    signs = np.asarray([readout(S.z_of(i))[1] if i != S.BLANK_IDX else 0 for i in range(9)])
    return tuple(int(st.bank[:, start:start + 192].sum())
                 + int((signs[st.sc] == eps).sum()) + int((signs[st.fcc] == eps).sum())
                 for eps, start in ((1, 0), (-1, 192)))


def advance_owned(state):
    before = A.checkpoint(state)
    out, events = A.step(state)
    assert A.checkpoint(state) == before
    for value in records(out).values():
        assert value.flags.owndata
        assert all(not np.shares_memory(value, old) for old in records(state).values())
    return out, events


@pytest.mark.parametrize("witness", ["contact", "separated"])
@pytest.mark.parametrize("background", BACKGROUNDS)
@pytest.mark.parametrize("eps", [1, -1])
@pytest.mark.parametrize("center", PLACEMENTS)
def test_locked_56_microtick_formation_histories(witness, background, eps, center):
    state = preparation(witness, background, eps, center)
    baseline = original_fixture(state)
    offset = 192 if eps == -1 else 0
    site = G.site_index(7, *center)
    if witness == "contact":
        baseline.lattice.bank[site, offset + 5] = False
        baseline.lattice.bank[site, offset + 1] = True
    original_tables = C.load_collision_tables()
    total, by_polarity = A.work_units(state), inventory(state)
    formation_tick = 2 if witness == "contact" else 6
    assert not pair_predicate(state)
    for tick in range(1, HORIZON + 1):
        if witness == "separated" and tick == 6:
            # Registered U^2({0,1}) aligned preimage, with all actual controls.
            baseline.lattice.bank[site] = False
            baseline.lattice.bank[site, [offset + C.U(C.U(c)) for c in (0, 1)]] = True
        state, events = advance_owned(state)
        baseline, expected = P.step(baseline, original_tables)
        compare(state, baseline, exclude=("bank",) if tick < formation_tick and witness == "contact" else ())
        if tick != formation_tick:
            assert events == expected
        else:
            incoming = (0, 5) if witness == "contact" else (167, 170)
            outgoing = (4, 5) if witness == "contact" else (166, 167)
            assert events.collisions == [(site, eps, incoming, outgoing)]
            assert events.absorptions == expected.absorptions
            assert events.crossings == expected.crossings
            assert events.gate_holds == expected.gate_holds
        assert not events.absorptions and not events.crossings and not events.gate_holds
        assert not state.admitted_sc.any()
        assert A.work_units(state) == total
        assert inventory(state) == by_polarity
        assert pair_predicate(state) == (tick >= formation_tick)
        assert not state.lattice.s.any()
        if tick == 3:
            x, y, z = center
            expected_site = ((x - 1) % 7, y, z) if witness == "contact" else center
            pair = (93, 94) if witness == "contact" else (167, 170)
            assert field_records(state) == sorted((expected_site, c + offset) for c in pair)
        if witness == "separated" and tick == 7:
            x, y, z = center
            assert field_records(state) == [((x, y, (z + 1) % 7), c + offset) for c in (4, 7)]
    restored = A.restore(A.checkpoint(state))
    compare(state, restored)


@pytest.mark.parametrize("background", BACKGROUNDS)
@pytest.mark.parametrize("eps", [1, -1])
@pytest.mark.parametrize("center", PLACEMENTS)
def test_locked_separated_same_flag_nonrestoring_control(background, eps, center):
    state = preparation("nonrestoring", background, eps, center)
    baseline = original_fixture(state)
    tables = C.load_collision_tables()
    initial_inventory = inventory(state)
    # Individual channels never collide in this control, so channel phase
    # follows U and labels may be tracked as external observations.
    channels = [c + (192 if eps == -1 else 0) for c in (0, 1)]
    for tick in range(1, HORIZON + 1):
        state, events = advance_owned(state)
        baseline, expected = P.step(baseline, tables)
        compare(state, baseline)
        assert events == expected == T.TickEvents()
        if tick % 4 == 3:
            channels = [C.U(c) for c in channels]
        positions = [G.coords(7, int(np.flatnonzero(state.lattice.bank[:, c])[0])) for c in channels]
        assert tuple((b - a) % 7 for a, b in zip(*positions)) == (1, 0, 0)
        assert not pair_predicate(state)
        assert inventory(state) == initial_inventory


@pytest.mark.parametrize("phase", range(4))
def test_checkpoint_owned_replay_all_phases_and_cross_law_rejection(phase):
    state = A.initialize(S.blank(3))
    state.lattice.bank[0, [0, 5, 192, 197]] = True
    for _ in range(phase):
        state, _ = A.step(state)
    data = A.checkpoint(state)
    restored = A.restore(data)
    assert A.checkpoint(restored) == data
    for value in records(restored).values():
        assert all(not np.shares_memory(value, original) for original in records(state).values())
    for _ in range(8):
        state, left = advance_owned(state)
        restored, right = advance_owned(restored)
        assert A.checkpoint(state) == A.checkpoint(restored)
        assert left == right
    original = P.initialize(S.blank(3))
    with pytest.raises(ValueError): A.restore(P.checkpoint(original))
    with pytest.raises(ValueError): P.restore(data)
    with pytest.raises(ValueError): A.step(original)
    with pytest.raises(ValueError): P.step(state)
    with pytest.raises(ValueError): A.initialize(original)


def test_expiry_merges_complete_states_and_preserves_forward_replay():
    left = preparation("contact", BACKGROUNDS[0], 1, PLACEMENTS[0])
    right = A.restore(A.checkpoint(left))
    site = G.site_index(7, *PLACEMENTS[0])
    right.lattice.bank[site, 5] = False
    right.lattice.bank[site, 1] = True
    assert A.checkpoint(left) != A.checkpoint(right)
    for _ in range(2):
        left, le = A.step(left)
        right, re = A.step(right)
    assert le.collisions != re.collisions
    assert A.checkpoint(left) == A.checkpoint(right)
    for _ in range(8):
        left, le = A.step(left)
        right, re = A.step(A.restore(A.checkpoint(right)))
        assert A.checkpoint(left) == A.checkpoint(right)
        assert le == re


@pytest.mark.parametrize("name,value", [("s", 2), ("ell", 3), ("sc", 9), ("fcc", -1),
                                         ("bank", 2), ("admitted_sc", 2), ("gate_sc", 2), ("gate_fcc", 2)])
def test_invalid_alphabets_fail_before_mutation(name, value):
    state = A.initialize(S.blank(3))
    array = records(state)[name]
    if array.dtype == bool:
        array.view(np.uint8).flat[0] = value
    else:
        array.flat[0] = value
    before = {key: val.tobytes() for key, val in records(state).items()}
    for operation in (A.step, A.checkpoint, A.work_units):
        with pytest.raises(ValueError): operation(state)
        assert {key: val.tobytes() for key, val in records(state).items()} == before


@pytest.mark.parametrize("name", ["law_id", "table_id", "collision_hash", "encoding_hash", "boundary"])
def test_forged_state_identity_rejected(name):
    state = A.initialize(S.blank(3))
    object.__setattr__(state, name, "foreign")
    with pytest.raises(ValueError): A.step(state)
    with pytest.raises(ValueError): A.checkpoint(state)


@pytest.mark.parametrize("clock", [-1, True, 0.0, None])
def test_invalid_clock_rejected(clock):
    state = replace(A.initialize(S.blank(3)), microtick=clock)
    with pytest.raises(ValueError): A.step(state)


def test_pending_payload_shapes_dtype_aliasing_and_initial_ownership():
    lattice = S.blank(3)
    state = A.initialize(lattice)
    lattice.bank[0, 0] = True
    assert not state.lattice.bank.any()
    state.gate_sc[0, 0] = True
    with pytest.raises(ValueError): A.step(state)
    state = replace(A.initialize(S.blank(3)), microtick=1)
    state.admitted_sc[0, 0] = True
    with pytest.raises(ValueError): A.step(state)
    state.lattice.sc[0, 0] = (S.BLANK_IDX, S.idx_of(rotate(encode(2, 1))))
    A.validate(state)
    for bad in (np.zeros((27, 3), dtype=np.int8), np.zeros((27, 2), dtype=bool), None):
        with pytest.raises(ValueError): A.step(replace(state, gate_sc=bad))
    with pytest.raises(ValueError): A.step(replace(state, gate_sc=state.admitted_sc))
    with pytest.raises(ValueError): A.initialize(S.blank(2))


def encoded(payload):
    return A.MAGIC + json.dumps(payload, sort_keys=True, separators=(",", ":")).encode()


@pytest.mark.parametrize("key", ["schema", "law", "table", "collision", "encoding", "backend", "boundary"])
def test_checkpoint_identity_fields_rejected(key):
    payload = json.loads(A.checkpoint(A.initialize(S.blank(3)))[len(A.MAGIC):])
    payload[key] = "foreign"
    with pytest.raises(ValueError): A.restore(encoded(payload))


@pytest.mark.parametrize("name", A.NAMES)
def test_checkpoint_each_array_missing_truncated_or_invalid(name):
    raw = A.checkpoint(A.initialize(S.blank(3)))
    for operation in ("missing", "truncated", "oversized", "alphabet", "invalid-base64"):
        payload = json.loads(raw[len(A.MAGIC):])
        if operation == "missing":
            del payload["arrays"][name]
        elif operation == "invalid-base64":
            payload["arrays"][name] = "!"
        else:
            data = bytearray(base64.b64decode(payload["arrays"][name]))
            if operation == "truncated":
                data.pop()
            elif operation == "oversized":
                data.extend(b"\x00\x00\x00")
            else:
                data[0] = {"s": 2, "ell": 3, "sc": 9, "fcc": 9}.get(name, 2)
            payload["arrays"][name] = base64.b64encode(data).decode()
        with pytest.raises(ValueError): A.restore(encoded(payload))


def test_checkpoint_duplicate_fields_unknown_fields_and_bad_clock():
    raw = A.checkpoint(A.initialize(S.blank(3)))
    with pytest.raises(ValueError): A.restore(A.MAGIC + b'{"law":"x","law":"y"}')
    for key, value in (("unexpected", 1), ("L", True), ("microtick_hex", -1), ("microtick_hex", 1.5)):
        payload = json.loads(raw[len(A.MAGIC):])
        payload[key] = value
        with pytest.raises(ValueError): A.restore(encoded(payload))


@pytest.mark.parametrize("phase", range(4))
def test_checkpoint_unbounded_ordinal_hex_round_trip_and_replay(phase):
    digit_limit = sys.get_int_max_str_digits()
    state = replace(A.initialize(S.blank(3)), microtick=10 ** 5000 + phase)
    A.validate(state)
    raw = A.checkpoint(state)
    payload = json.loads(raw[len(A.MAGIC):])
    assert "microtick" not in payload
    assert payload["microtick_hex"] == format(state.microtick, "x")
    restored = A.restore(raw)
    assert restored.microtick == state.microtick
    assert A.checkpoint(restored) == raw
    for _ in range(4):
        state, left = advance_owned(state)
        restored, right = advance_owned(restored)
        assert left == right
        assert A.checkpoint(state) == A.checkpoint(restored)
    assert sys.get_int_max_str_digits() == digit_limit


@pytest.mark.parametrize("clock", ["", "00", "01", "0a", "A", "ABC", "0x1", "0X1",
                                   "+1", "-1", " 1", "1 ", "1\n", "1_0", "\uff11", "g", 0,
                                   True, None, [], "f" * 5000 + "G"])
def test_checkpoint_noncanonical_hex_clock_rejected(clock):
    payload = json.loads(A.checkpoint(A.initialize(S.blank(3)))[len(A.MAGIC):])
    payload["microtick_hex"] = clock
    with pytest.raises(ValueError, match="hexadecimal checkpoint clock"):
        A.restore(encoded(payload))


def test_checkpoint_legacy_decimal_schema_and_trailing_bytes_rejected():
    raw = A.checkpoint(A.initialize(S.blank(3)))
    payload = json.loads(raw[len(A.MAGIC):])
    assert payload["microtick_hex"] == "0"
    payload["schema"] = "ftd-alignment-checkpoint-1"
    payload["microtick"] = 0
    del payload["microtick_hex"]
    legacy = b"FTD-PHI-ALIGNMENT-1\n" + json.dumps(payload).encode()
    with pytest.raises(ValueError): A.restore(legacy)
    with pytest.raises(ValueError): A.restore(encoded(payload))
    with pytest.raises(ValueError): A.restore(raw + b"\x00")


@pytest.mark.parametrize("case", ["admission", "crossing", "odd-gate", "manifested-stream"])
@pytest.mark.parametrize("eps", [1, -1])
def test_active_unchanged_stages_and_pending_controls(case, eps):
    # Added after the fixed main matrix passed: orthogonal lifecycle coverage,
    # with no change to the selected law or the preregistered witnesses.
    lattice = S.blank(3)
    offset = 192 if eps == -1 else 0
    if case == "admission":
        lattice.bank[0, offset + 2] = True
    elif case in ("crossing", "odd-gate"):
        lattice.sc[0, 0, 0] = S.idx_of(encode(0, eps))
        if case == "odd-gate":
            lattice.bank[0, offset] = True
    else:
        lattice.bank[0, offset] = True
        lattice.s[0] = 1
    state = A.initialize(lattice)
    baseline = P.initialize(lattice)
    tables = C.load_collision_tables()
    initial_inventory = inventory(state)
    for tick in range(1, 9):
        state, events = advance_owned(state)
        baseline, expected = P.step(baseline, tables)
        compare(state, baseline)
        assert events == expected
        assert inventory(state) == initial_inventory
        if case == "admission" and tick == 1:
            owner, axis = G.sc_edge_of(3, 0, C.tangent(offset + 2))
            assert events.absorptions == [(0, offset + 2, owner, axis)]
            assert state.admitted_sc[owner, axis]
            assert tuple(state.lattice.sc[owner, axis]) == (S.BLANK_IDX, S.idx_of(rotate(encode(2, eps))))
        if case == "admission" and tick in (2, 3):
            assert state.admitted_sc.any()
        if case == "crossing" and tick == 2:
            assert ("sc", 0, (0,), 1) in events.crossings
        if case == "odd-gate" and tick == 2:
            assert ("sc", 0, (0,)) in events.gate_holds
            assert ("sc", 0, (0,), 1) not in events.crossings
        if case == "manifested-stream" and tick == 3:
            owner = G.shift(3, 0, C.tangent(offset))
            assert state.lattice.bank[owner, C.half_turn(C.U(offset))]
        if tick % 4 == 0:
            assert not state.admitted_sc.any() and not state.gate_sc.any() and not state.gate_fcc.any()


@pytest.mark.parametrize("phase", range(4))
@pytest.mark.parametrize("location", [(2, 2, 2), (0, 0, 0)])
def test_sampled_all_record_family_radius_one_support(phase, location):
    # Bounded interventions, not an exhaustive proof of full-state support.
    lattice = S.blank(5)
    lattice.sc.fill(BACKGROUNDS[0]); lattice.fcc.fill(BACKGROUNDS[0])
    lattice.bank[G.site_index(5, *location), [0, 5]] = True
    base = A.initialize(lattice)
    for _ in range(phase):
        base, _ = A.step(base)
    site = G.site_index(5, *location)
    for name in A.NAMES if phase else A.NAMES[:5]:
        left = A.restore(A.checkpoint(base))
        left.admitted_sc[site, 0] = False
        if name == "admitted_sc":
            left.lattice.sc[site, 0] = (S.BLANK_IDX, S.idx_of(rotate(encode(2, 1))))
        right = A.restore(A.checkpoint(left))
        array = records(right)[name]
        idx = (site,) + (0,) * (array.ndim - 1)
        if name == "s": array[idx] = 1 if array[idx] != 1 else -1
        elif name == "ell": array[idx] = (int(array[idx]) + 1) % 3
        elif name in ("sc", "fcc"): array[idx] = (int(array[idx]) + 1) % 9
        else: array[idx] = not array[idx]
        out_left, _ = A.step(left)
        out_right, _ = A.step(right)
        for key, value in records(out_left).items():
            changed = np.flatnonzero((value != records(out_right)[key]).reshape(125, -1).any(axis=1))
            for owner in changed:
                delta = [abs(a - b) for a, b in zip(location, G.coords(5, int(owner)))]
                assert max(min(d, 5 - d) for d in delta) <= 1, (phase, name, key, owner)
