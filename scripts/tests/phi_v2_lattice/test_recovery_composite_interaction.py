"""Bounded exact observer controls, never registered H304 campaign outcomes."""
from dataclasses import asdict, replace
from itertools import islice, product
from pathlib import Path

import numpy as np
import pytest

from phi_v2_lattice import recovery_composite_interaction as I
from phi_v2_lattice import sparse_credit_exchange_q4 as S
from phi_v2_lattice import recovery_credit_exchange_q2 as Q
from test_sparse_credit_exchange_q4 import topology_fixtures, assert_parity, cubic_actions


@pytest.fixture(scope="module")
def catalogue():
    root = Path(__file__).resolve().parents[3]
    return I.load_catalogue(root / "engine/docs/evidence/strict-recovery-wave7-2026-09-08/q2-attempt-1/transitions.npz", I.CATALOGUE_SHA256)


def control(point, path=(0,), credit=(1, 0), heads=(0, 0), attempted=(0, 0), phase=0, origin=0, eta=0, L=64):
    axes = I.AXES[origin//8]
    color = tuple(((origin >> a) & 1) ^ (point[axes[a]] % 2) for a in range(3))
    node = Q.encode(Q.QuotientState(phase, color, Q.Record(path, heads, credit, attempted)))
    return I.Q2Control(L, phase, origin, eta, point, node)


def test_catalogue_identity_layout_and_immutable_bytes(catalogue):
    assert len(catalogue.arrays) == 12
    assert catalogue.arrays["successor"].shape == (941184,)
    assert len(catalogue.pins) == 5
    for a in catalogue.arrays.values():
        assert not a.flags.writeable
        with pytest.raises(ValueError):
            a.setflags(write=True)
    with pytest.raises(ValueError):
        I.load_catalogue(catalogue.path, "foreign")


def test_catalogue_parses_the_exact_hashed_snapshot(catalogue, tmp_path, monkeypatch):
    from io import BytesIO
    path = tmp_path / "transitions.npz"
    path.write_bytes(Path(catalogue.path).read_bytes())
    path.with_name("report.json").write_bytes(Path(catalogue.path).with_name("report.json").read_bytes())
    original = I.np.load
    def replacing_path_before_parse(stream, **kwargs):
        assert isinstance(stream, BytesIO)
        path.write_bytes(b"concurrent replacement after byte-snapshot admission")
        return original(stream, **kwargs)
    monkeypatch.setattr(I.np, "load", replacing_path_before_parse)
    loaded = I.load_catalogue(path, I.CATALOGUE_SHA256)
    assert np.array_equal(loaded.arrays["successor"], catalogue.arrays["successor"])
    assert path.read_bytes().startswith(b"concurrent replacement")


@pytest.mark.parametrize("tier,count", tuple(I.TIER_COUNTS.items()))
def test_entire_preparation_id_inventory(tier, count):
    actual = 0
    first = last = None
    for p in I.preparation_ids(tier):
        if first is None:
            first = p
        last = p
        actual += 1
    assert actual == count
    assert first.stage_advance == 0 and first.color == (0, 0, 0)
    assert last.stage_advance == 18 and last.color == (1, 1, 1)
    assert count % 64 == 0
    assert (count//64) == {"head_on": 114, "head_on_interventions": 1254, "full_impact": 17100}[tier]


def test_all_registered_geometry_dimensions_static_bounds():
    count = 0
    for va, vb, m, n, branch in product(range(1, 7), range(1, 7), range(-2, 3), range(-2, 3), range(2)):
        pid = I.PreparationID("full_impact", (0, 0, 0), va, vb, 1, 1, branch, m, n, 0, 0)
        d = I.preparation_displacement(pid)
        # Leading positions and negative edge owners lie between endpoints.
        a = I.V[va-1]
        b = I._add(d, I.V[vb-1])
        assert max(abs(x) for point in ((0, 0, 0), a, d, b) for x in point) <= 9
        assert max(abs(x) for x in d) >= 4
        count += 1
    assert count == 1800


@pytest.mark.parametrize("phase", range(19))
@pytest.mark.parametrize("axis_order", range(6))
@pytest.mark.parametrize("eta", range(2))
def test_isolated_catalogue_stage_lift_event_parity(catalogue, phase, axis_order, eta):
    # Fixed bent Q2 paths exercise denial and both schedule banks. Two
    # separated copies make a legal Q4 comparison without partner inference.
    origin = 8*axis_order+5
    a = control((1, 1, 1), (0, 2), (0, 0), (5, 3), (0, 0), phase, origin, eta, 8)
    b = control((5, 5, 5), (0,), (1, 0), (0, 0), (1, 0), phase, origin, eta, 8)
    before = I.superpose_controls(a, b, catalogue)
    assert type(before) is S.SparseQ4State
    aa, ea = I.advance_control(a, catalogue, True)
    bb, eb = I.advance_control(b, catalogue, True)
    expected = I.superpose_controls(aa, bb, catalogue)
    actual, events = assert_parity(before)
    assert actual == expected
    assert I.event_record(events) == I.event_record(I._merge_events(ea, eb, before))


@pytest.mark.parametrize("phase", range(19))
def test_all_phase_huge_control_clock(catalogue, phase):
    a = control((20, 20, 20), phase=phase)
    t = 10**5000
    a = replace(a, microtick=t+(phase-t)%19)
    out, events = I.advance_control(a, catalogue, True)
    assert out.microtick == a.microtick+1
    assert Q.decode(out.node).stage == out.microtick % 19


@pytest.mark.parametrize("kind", (np.int64, np.uint8, np.uint64))
def test_observer_unsigned_normalization(catalogue, kind):
    tick = int(np.iinfo(kind).max)
    phase = tick % 19
    a = replace(control((20, 20, 20), phase=phase), microtick=tick)
    b = replace(control((40, 40, 40), phase=phase), microtick=tick)
    before = I.superpose_controls(a, b, catalogue)
    following = tuple(I.advance_control(c, catalogue) for c in (a, b))
    after, events = S.step(before)
    with np.errstate(over="raise", invalid="raise"):
        wrapped = replace(before, L=kind(64), microtick=kind(tick))
        assert I.observe_step(wrapped, after, events, (a, b), following, catalogue) == I.observe_step(before, after, events, (a, b), following, catalogue)
        lift = I.initial_lift(before)
        raw = I.IntegerLift(tuple(tuple(kind(v) for v in p) for p in lift.carriers),
                            tuple(tuple(kind(v) for v in p) for p in lift.edges))
        assert I.advance_lift(wrapped, after, events, raw) == I.advance_lift(before, after, events, lift)
        assert I.validate_lift(wrapped, raw) == I.validate_lift(before, lift)


@pytest.mark.parametrize("phase", range(19))
def test_complete_field_stencil_radius_one(phase):
    for _, base in topology_fixtures():
        for eta in range(2):
            s = replace(base, microtick=phase, charge_frame=eta)
            stencils = I.read_stencils(s)
            assert len({row.output for row in stencils}) == len(stencils)
            for row in stencils:
                x = S.coordinates(s.L, row.output[1])
                for key in row.inputs:
                    y = S.coordinates(s.L, key[1])
                    assert all(min((a-b)%s.L, (b-a)%s.L) <= 1 for a, b in zip(x, y))
                if phase >= 7 and row.output[0] in ("credit", "attempted", "flux"):
                    assert len({key[1] for key in row.inputs}) <= 2


@pytest.mark.parametrize("heading_b,effect", ((0, False), (2, True)))
def test_alignment_event_difference_distinct_from_state_effect(catalogue, heading_b, effect):
    a = control((10, 10, 10))
    b = control((11, 10, 10), (2,), (1, 0), (heading_b, heading_b))
    before = I.superpose_controls(a, b, catalogue)
    after, events = S.step(before)
    controls_after = tuple(I.advance_control(c, catalogue) for c in (a, b))
    o = I.observe_step(before, after, events, (a, b), controls_after, catalogue)
    assert o.structural_contact and o.event_difference and o.state_effect == effect
    assert not o.counterfactual_invalid
    assert len(events.onsite_alignments) == 1


def test_empty_midpoint_conservative_contact_without_effect(catalogue):
    # The empty midpoint (11,10,10) has a +x matched partner in A and a
    # possible own-star edge from B. Its source-denial union is conservative.
    a = control((10, 10, 10), (2,), (1, 0), (2, 2), phase=7)
    b = control((11, 9, 10), (2,), (1, 0), (2, 2), phase=7)
    before = I.superpose_controls(a, b, catalogue)
    after, events = S.step(before)
    following = tuple(I.advance_control(c, catalogue) for c in (a, b))
    o = I.observe_step(before, after, events, (a, b), following, catalogue)
    assert o.structural_contact and not o.state_effect
    assert any(key[1] == S.site_index(64, (11, 10, 10)) for key in o.contact_outputs)


def test_invalid_superpositions_keep_separate_diagnostic(catalogue):
    a = control((10, 10, 10))
    same = I.superpose_controls(a, a, catalogue)
    assert isinstance(same, I.SuperpositionConflict)
    # Opposite edge orientation is still an account-changing overlap.
    b = control((11, 10, 10), (1,), (1, 0), (1, 1))
    assert isinstance(I.superpose_controls(a, b, catalogue), I.SuperpositionConflict)
    assert isinstance(I.superpose_controls(a, replace(a, origin_code=8), catalogue), I.SuperpositionConflict)
    # The actual state remains lawful even when the isolated reference fails.
    before = topology_fixtures()[0][1]
    after, events = S.step(before)
    following = tuple(I.advance_control(c, catalogue) for c in (a, a))
    # Mismatched joint/control metadata must not silently compare arrays.
    with pytest.raises(ValueError):
        I.observe_step(before, after, events, (a, a), following, catalogue)


@pytest.mark.parametrize("variant", range(11))
@pytest.mark.parametrize("stage", range(19))
def test_fixed_preparation_lift_kicks_stages(catalogue, variant, stage):
    p = I.PreparationID("head_on_interventions", (1, 0, 1), 1, 2, -1, 1, 1, 0, 0, stage, variant)
    case = I.prepare(p, catalogue)
    assert case.state.microtick == stage+1
    assert S.observe(case.state).Q == 4
    assert tuple(c.node for c in case.controls) == dict(case.provenance)["final_nodes"]
    I.check_no_wrap(case.state, I.initial_lift(case.state))


def test_interval_formula_against_bounded_integer_enumeration():
    for w in product(range(-6, 7, 2), repeat=3):
        for slope in product((-4, 0, 4), repeat=3):
            actual = I._interval(w, slope)
            expected = [n for n in range(12) if all(abs(w[j]+n*slope[j]) <= 2 for j in range(3))]
            assert (actual is not None) == bool(expected)
            if actual is not None:
                assert actual[0] == expected[0]
                assert actual[1] is None or actual[1] == expected[-1]


@pytest.mark.parametrize("phase", range(19))
def test_escape_chart_retains_all_declared_input_and_output_owners(phase):
    # A seam lift must retain coordinates on both sides of the fundamental
    # box, rather than silently replacing them by shortest torus separation.
    c = control((0, 0, 0), (0, 0), (0, 0), (2, 3), phase=phase, L=6)
    chart = I._control_support(c)
    residues = {S.site_index(6, p) for p in chart}
    stencils = I._read_stencils(I.control_payload(c))
    required = {row.output[1] for row in stencils} | {key[1] for row in stencils for key in row.inputs}
    assert required <= residues
    assert any(any(x < 0 for x in p) for p in chart)


def moving_pair(catalogue, heading_a, heading_b, pos_a, pos_b):
    a = control(pos_a, (heading_a,), (1, 0), (heading_a, heading_a), phase=1)
    b = control(pos_b, (heading_b,), (1, 0), (heading_b, heading_b), phase=1)
    return I.superpose_controls(a, b, catalogue)


@pytest.mark.parametrize("co_moving", (False, True))
def test_infinite_escape_is_not_torus_escape(catalogue, co_moving):
    state = moving_pair(catalogue, 0 if co_moving else 1, 0, (20, 20, 32), (44, 44 if co_moving else 20, 32))
    result = I.escape_certificate(state, catalogue)
    assert result.applicable and result.infinite_lift
    assert result.periodic_torus == co_moving
    # Independent direct support/offset/residue enumeration, all 38*P.
    a, b = result.controls
    advanced = (a, b)
    for _ in range(38):
        advanced = tuple(I.advance_control(c, catalogue) for c in advanced)
    blocks = tuple(I._sub(n.chart_plus_position, c.chart_plus_position) for c, n in zip((a, b), advanced))
    slope = I._sub(blocks[1], blocks[0])
    hit = False
    for aa, bb in result.support_charts:
        for n in range(result.modular_period):
            # Broadcast directly over both retained support sets.
            delta = np.array(bb)[None, :, :]-np.array(aa)[:, None, :]+n*np.array(slope)
            mod = delta % 64
            hit |= bool(np.any(np.all(np.minimum(mod, 64-mod) <= 2, axis=2)))
    assert result.periodic_torus == (not hit)


def test_junction_and_detached_cycle_have_no_Q2_escape_label(catalogue):
    for index in (8, 9, 10):
        s = topology_fixtures()[index][1]
        assert not I.escape_certificate(s, catalogue).applicable


def test_lift_charge_incidence_and_all_phase_replay():
    state = S.transform(topology_fixtures(64)[8][1], I.IDENTITY, (32, 32, 32))
    lift = I.initial_lift(state)
    original = I.validate_lift(state, lift)
    reference, reference_lift = state, lift
    displacement = [0, 0, 0]
    for _ in range(38):
        after, events = S.step(state)
        following = I.advance_lift(state, after, events, lift)
        for epsilon, source, dest, owner, axis, *_ in events.moves:
            displacement[axis] += epsilon*(1 if source == owner else -1)
        charge = I.validate_lift(after, following)
        assert charge == tuple(original[a]+displacement[a] for a in range(3))
        observation = I.observe_lift(after, following, reference, reference_lift)
        assert observation.charge_dipole == charge
        assert observation.signed_displacement == tuple(displacement)
        assert observation.signed_displacement == I._sub(observation.plus_displacement, observation.minus_displacement)
        state, lift = after, following
    bad = replace(lift, carriers=tuple((p[0]+state.L, p[1], p[2]) if i == 0 else p for i, p in enumerate(lift.carriers)))
    with pytest.raises(ValueError):
        I.validate_lift(state, bad)


def test_capture_checks_even_complete_translation_and_clock():
    state = S.transform(topology_fixtures(64)[8][1], I.IDENTITY, (32, 32, 32))
    history, lifts = [state], [I.initial_lift(state)]
    for _ in range(19):
        following, events = S.step(state)
        lifts.append(I.advance_lift(state, following, events, lifts[-1]))
        history.append(following)
        state = following
    result = I.capture_certificate(tuple(history), tuple(lifts))
    assert all(w.period % 19 == 0 and all(v % 2 == 0 for v in w.translation) for w in result.witnesses)
    # Even spatial translation preserves the copied matching background;
    # odd payload-only translation cannot be complete-array recurrence.
    original = history[0]
    even = S.transform(original, I.IDENTITY, (2, 0, 0))
    odd = S.transform(original, I.IDENTITY, (1, 0, 0))
    assert even.origin_code == original.origin_code
    assert odd.origin_code != original.origin_code
    # These endpoint comparisons are synthetic algebra controls, explicitly
    # not a claim that the fixed law traverses this displacement in 19 ticks.
    assert I.relative_translation_matches(original, lifts[0], replace(even, microtick=19), I.initial_lift(even)) == ((2, 0, 0),)
    assert I.relative_translation_matches(original, lifts[0], replace(odd, microtick=19), I.initial_lift(odd)) == ()
    forged = tuple(history[:-1])+(replace(even, microtick=19),)
    with pytest.raises(ValueError, match="trajectory"):
        I.capture_certificate(forged, tuple(lifts[:-1])+(I.initial_lift(even),))
    broken = tuple(history[:-1])+(replace(history[-1], microtick=100),)
    with pytest.raises(ValueError):
        I.capture_certificate(broken, tuple(lifts))


def test_registered_evaluator_rejects_tuning_without_execution(catalogue):
    p = next(I.preparation_ids("head_on"))
    for H, L in ((303, 64), (304, 32), (True, 64)):
        with pytest.raises(ValueError):
            I.evaluate_case(p, catalogue, H=H, L=L)


def test_failure_receipt_preserves_prefix_without_running_registered_case():
    s = topology_fixtures()[0][1]
    raw = S.checkpoint(s)
    e = I.EvaluationFailure(next(I.preparation_ids("head_on")), [raw], [], "synthetic pre-first-tick failure")
    assert e.completed_ticks == 0 and e.checkpoints == (raw,) and e.hash_chain == ()
    assert S.restore(e.checkpoints[-1]) == s
