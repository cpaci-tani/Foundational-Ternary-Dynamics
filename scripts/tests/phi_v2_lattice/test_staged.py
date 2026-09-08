import numpy as np
import pytest
from phi_v2_lattice import channels as C, geometry as G, state as S, tick as T, staged as P
from phi_v2_lattice._proofs import encode, rotate


@pytest.fixture(scope="module")
def tables():
    return C.load_collision_tables()


def _prepared(L=3, seed=19):
    rng = np.random.default_rng(seed)
    st = S.blank(L)
    st.s[:] = rng.integers(-1, 2, st.s.shape)
    st.ell[:] = rng.integers(0, 3, st.ell.shape)
    st.bank[:] = rng.random(st.bank.shape) < .006
    st.sc[:] = rng.integers(0, 9, st.sc.shape)
    st.fcc[:] = rng.integers(0, 9, st.fcc.shape)
    return st


@pytest.mark.parametrize("seed", [19, 71, 123])
def test_four_physical_ticks_match_baseline_with_pure_steps_and_work_balance(tables, seed):
    baseline = _prepared(seed=seed)
    candidate = P.initialize(baseline)
    work = P.work_units(candidate)
    for cycle in range(3):
        baseline, expected_events = T.tick(baseline, tables)
        actual_events = T.TickEvents()
        for phase in range(4):
            assert candidate.phase == phase
            frozen = P.checkpoint(candidate)
            next_state, events = P.step(candidate, tables)
            assert P.checkpoint(candidate) == frozen
            assert P.work_units(next_state) == work
            for field in vars(actual_events):
                getattr(actual_events, field).extend(getattr(events, field))
            candidate = next_state
        assert actual_events == expected_events
        for field in ("s", "ell", "bank", "sc", "fcc"):
            np.testing.assert_array_equal(getattr(candidate.lattice, field), getattr(baseline, field))
        assert candidate.microtick == 4 * (cycle + 1)


def test_expiry_erases_distinct_channel_presentations_from_complete_state(tables):
    choices = [c for c in range(192) if C.phase(c) == 2 and C.tangent(c) == (1, 0, 0)]
    assert len(choices) >= 2
    states = []
    for channel in choices[:2]:
        st = S.blank(3)
        st.bank[0, channel] = True
        states.append(P.initialize(st))
    assert P.checkpoint(states[0]) != P.checkpoint(states[1])
    outputs = [P.step(st, tables)[0] for st in states]
    assert P.checkpoint(outputs[0]) == P.checkpoint(outputs[1])
    assert P.work_units(outputs[0]) == 1


def _records(st):
    return {**{name: getattr(st.lattice, name) for name in ("s", "ell", "bank", "sc", "fcc")},
            **{name: getattr(st, name) for name in ("admitted_sc", "gate_sc", "gate_fcc")}}


@pytest.mark.parametrize("phase", range(4))
@pytest.mark.parametrize("location", [(2, 2, 2), (0, 0, 0)])
def test_every_record_family_has_sampled_radius_one_support(tables, phase, location):
    # Each input family is perturbed at exactly one stored site/anchor. Inspect
    # ALL output arrays, including pending controls; the clock/law is held fixed.
    base = P.initialize(_prepared(5))
    for _ in range(phase):
        base, _ = P.step(base, tables)
    x = G.site_index(5, *location)
    names = list(_records(base)) if phase else ["s", "ell", "bank", "sc", "fcc"]
    for name in names:
        a = P.restore(P.checkpoint(base))
        # Make a legal carrier pair for an independent admitted-bit intervention.
        a.admitted_sc[x, 0] = False
        if name == "admitted_sc":
            a.lattice.sc[x, 0] = (S.BLANK_IDX, S.idx_of(rotate(encode(2, +1))))
        b = P.restore(P.checkpoint(a))
        record = _records(b)[name]
        idx = (x,) + (0,) * (record.ndim - 1)
        if name == "s": record[idx] = 1 if record[idx] != 1 else -1
        elif name == "ell": record[idx] = (int(record[idx]) + 1) % 3
        elif name in ("sc", "fcc"): record[idx] = (int(record[idx]) + 1) % 9
        else: record[idx] = not record[idx]
        aa, _ = P.step(a, tables)
        bb, _ = P.step(b, tables)
        for output, values in _records(aa).items():
            changes = values != _records(bb)[output]
            sites = np.flatnonzero(changes.reshape(125, -1).any(axis=1))
            for site in sites:
                delta = [abs(u - v) for u, v in zip(location, G.coords(5, int(site)))]
                assert max(min(d, 5 - d) for d in delta) <= 1, (phase, name, output, site)


@pytest.mark.parametrize("field,value", [("s", 2), ("ell", 3), ("sc", 9), ("fcc", -1)])
def test_finite_payload_alphabets_reject_invalid_values(field, value):
    st = P.initialize(S.blank(3))
    getattr(st.lattice, field).flat[0] = value
    with pytest.raises(ValueError): P.validate(st)


def test_shape_dtype_clock_and_pending_validation():
    st = P.initialize(S.blank(3))
    st.lattice.bank = st.lattice.bank.astype(np.int8)
    with pytest.raises(ValueError): P.validate(st)
    st = P.initialize(S.blank(3)); st.microtick = -1
    with pytest.raises(ValueError): P.validate(st)
    st = P.initialize(S.blank(3)); st.gate_sc[0, 0] = True
    with pytest.raises(ValueError): P.validate(st)
    st = P.initialize(S.blank(3)); st.lattice.sc = st.lattice.sc[:, :2]
    with pytest.raises(ValueError): P.validate(st)
    with pytest.raises(ValueError): P.initialize(S.blank(2))
    for invalid in (None, [], [0]):
        lattice = S.blank(3); lattice.bank = invalid
        with pytest.raises(ValueError): P.initialize(lattice)


def test_collision_identity_is_enforced(tables):
    state, _ = P.step(P.initialize(S.blank(3)), tables)
    foreign = tuple(dict(table) for table in tables)
    pair = next(iter(foreign[0]))
    foreign[0][pair] = pair
    with pytest.raises(ValueError, match="collision table"):
        P.step(state, foreign)


def test_baseline_distance_two_witness_respects_elapsed_candidate_ticks(tables):
    st = S.blank(7)
    x, y, z = (G.site_index(7, v, 3, 3) for v in (3, 4, 2))
    st.bank[x, [0, 34]] = True
    a = P.initialize(st)
    st.bank[y, 2] = True
    b = P.initialize(st)
    for elapsed in range(1, 9):
        a, _ = P.step(a, tables)
        b, _ = P.step(b, tables)
        for name, values in _records(a).items():
            changes = (values != _records(b)[name]).reshape(343, -1).any(axis=1)
            for site in np.flatnonzero(changes):
                delta = [abs(u-v) for u,v in zip((4,3,3), G.coords(7,int(site)))]
                assert max(min(d, 7-d) for d in delta) <= elapsed
        if elapsed == 1:
            np.testing.assert_array_equal(a.lattice.bank[z], b.lattice.bank[z])
        if elapsed == 3:
            assert np.flatnonzero(a.lattice.bank[z]).tolist() == [113]
            assert np.flatnonzero(b.lattice.bank[z]).tolist() == [115]
