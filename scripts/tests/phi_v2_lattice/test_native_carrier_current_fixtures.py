"""Prospective finite fixture checks; optional native parity is never implicit."""
from collections import Counter, defaultdict
from dataclasses import replace
import os
from types import SimpleNamespace

import pytest

from phi_v2_lattice import native_carrier_current_fixtures as F


@pytest.fixture(scope="module")
def records():
    return tuple(F.fixtures())


def translated(state, offset):
    def moved(site):
        return F.S.site_index(state.L, tuple(a + b for a, b in zip(
            F.S.coordinates(state.L, site), offset)))
    carriers = tuple(sorted(replace(c, site=moved(c.site)) for c in state.carriers))
    edges = tuple(sorted(replace(e, owner=moved(e.owner)) for e in state.edges))
    axes = F.Q.AXES[state.origin_code // 8]
    origin = state.origin_code ^ sum((offset[axis] % 2) << a for a, axis in enumerate(axes))
    return replace(state, carriers=carriers, edges=edges, origin_code=origin)


def test_frozen_matrix_and_real_reached_phases(records):
    assert len(records) == 243
    assert sum(len(f.events) for f in records) == 11491
    assert Counter(f.case.kind for f in records) == {
        "seam": 144, "origin": 8, "continuation": 19,
        "straight-outward": 24, "bent-opposed": 24, "bent-transverse": 24,
    }
    seams = [f for f in records if f.case.kind == "seam"]
    assert len({(f.case.heading, f.case.order, f.case.eta, f.case.permutation) for f in seams}) == 144
    baseline = next(f for f in records if f.case.id == "origin-0")
    controls = [f for f in records if f.case.kind == "continuation"]
    assert {f.states[0].phase for f in controls} == set(range(19))
    for fixture in controls:
        start = 1 + fixture.case.reached
        assert fixture.states == baseline.states[start:start + 2]
        assert fixture.events == baseline.events[start:start + 1]
    for fixture in records:
        if fixture.case.kind != "continuation":
            assert fixture.states[0].microtick == 0
            assert fixture.states[1].microtick == 1


def test_translating_payload_19_full_background_38_and_both_polarity_transport(records):
    for fixture in records:
        if fixture.case.kind not in ("seam", "origin"):
            continue
        start = fixture.states[1]
        velocity = F._vector(fixture.case, fixture.case.heading)
        after19 = fixture.states[20]
        expected19 = translated(start, velocity)
        assert after19.carriers == expected19.carriers, fixture.case.id
        assert after19.edges == expected19.edges, fixture.case.id
        assert after19.origin_code != expected19.origin_code
        after38 = fixture.states[39]
        expected38 = replace(translated(start, tuple(2 * x for x in velocity)), microtick=39)
        assert after38 == expected38, fixture.case.id
        moves = [row for bundle in fixture.events[1:] for row in bundle[0]]
        assert Counter(row[0] for row in moves) == {1: 4, -1: 4}
        for row in moves:
            source = F.S.coordinates(F.L, row[1])
            destination = F.S.coordinates(F.L, row[2])
            assert tuple((d - s) % F.L for s, d in zip(source, destination)) == tuple(v % F.L for v in velocity)
        # Exact torus phase exponents for exp(-i k.x); no float dispersion fit.
        for wave in ((0, 0, 0), (1, 0, 0), (2, 3, 5), (15, 1, 8)):
            residue = -sum(k * (2 * v) for k, v in zip(wave, velocity)) % F.L
            for carrier in start.carriers:
                source = F.S.coordinates(F.L, carrier.site)
                target_point = tuple((x + 2 * v) % F.L for x, v in zip(source, velocity))
                target = next(c for c in after38.carriers
                              if c.site == F.S.site_index(F.L, target_point) and c.slot == carrier.slot)
                destination = F.S.coordinates(F.L, target.site)
                observed_residue = -sum(k * (y - x) for k, x, y in zip(wave, source, destination)) % F.L
                assert observed_residue == residue
                if wave == (0, 0, 0):
                    assert observed_residue == 0
        if fixture.case.kind == "seam":
            assert any(any(abs(a - b) == F.L - 1 for a, b in zip(
                F.S.coordinates(F.L, row[1]), F.S.coordinates(F.L, row[2]))) for row in moves)


def test_fixed_extended_controls_restore_by_registered_38_and_76_ticks(records):
    for fixture in records:
        if fixture.case.kind not in ("straight-outward", "bent-opposed", "bent-transverse"):
            continue
        assert len(fixture.quotients[1].record.path) == 2
        assert fixture.quotients[1].record.credits == (0, 0)
        contraction = next((elapsed for elapsed, q in enumerate(fixture.quotients[1:])
                            if len(q.record.path) <= 1), None)
        translating = next((elapsed for elapsed, q in enumerate(fixture.quotients[1:])
                            if F.Q.translating_family(q)), None)
        assert contraction is not None and contraction <= 38, (fixture.case.id, contraction)
        assert translating is not None and translating <= 76, (fixture.case.id, translating)


def test_complete_records_stay_separated_and_observations_are_read_only(records):
    for fixture in records:
        for state, q, anchor in zip(fixture.states, fixture.quotients, fixture.anchors):
            before = F.native_state_wire(state)
            assert len(state.carriers) == 4
            assert Counter(c.slot for c in state.carriers) == {0: 2, 1: 2}
            assert sum(c.credit for c in state.carriers) + sum(e.q ** 2 for e in state.edges) == 4
            first = F._component(fixture.case, q, anchor)
            second = F._component(fixture.case, q, F._add(anchor, F._offset(fixture.case)))
            def support(component):
                carriers, edges = component
                return {c.site for c in carriers} | {e.owner for e in edges} | {
                    F.S.shift(F.L, e.owner, e.axis, 1) for e in edges}
            a, b = support(first), support(second)
            for x in a:
                for y in b:
                    distances = [min((u - v) % F.L, (v - u) % F.L) for u, v in zip(
                        F.S.coordinates(F.L, x), F.S.coordinates(F.L, y))]
                    assert max(distances) > 2, fixture.case.id
            assert F._color(fixture.case, anchor) == F._color(fixture.case, F._add(anchor, F._offset(fixture.case)))
            assert F.native_state_wire(state) == before


def test_exact_local_number_flux_and_owned_account_event_balance(records):
    for fixture in records:
        for before, after, groups in zip(fixture.states, fixture.states[1:], fixture.events):
            number, account, flux = defaultdict(int), defaultdict(int), defaultdict(int)
            for sign, state in ((-1, before), (1, after)):
                for c in state.carriers:
                    number[c.site, c.slot] += sign
                    account[c.site] += sign * c.credit
                for e in state.edges:
                    account[e.owner] += sign * e.q ** 2
                    flux[e.owner, e.axis] += sign * e.q
            for epsilon, source, destination, owner, axis, q0, q1, k0, k1 in groups[0]:
                head = F.S.shift(F.L, owner, axis, 1)
                sigma = 1 if source == owner else -1
                assert (source, destination) == ((owner, head) if sigma == 1 else (head, owner))
                assert q1 - q0 == -epsilon * sigma
                assert k1 + q1 ** 2 == k0 + q0 ** 2
                slot = 0 if epsilon == 1 else 1
                number[owner, slot] += sigma
                number[head, slot] -= sigma
                flux[owner, axis] += epsilon * sigma
                current = k1 if sigma == 1 else -k0
                account[owner] += current
                account[head] -= current
            for owner, axis, _, donor, _, recipient, *rest in groups[6]:
                head = F.S.shift(F.L, owner, axis, 1)
                current = 1 if donor == owner else -1
                assert (donor, recipient) == ((owner, head) if current == 1 else (head, owner))
                account[owner] += current
                account[head] -= current
            assert not any(number.values()), fixture.case.id
            assert not any(account.values()), fixture.case.id
            assert not any(flux.values()), fixture.case.id
            F.native_event_wire(groups)


def test_oracle_never_calls_sparse_or_dense_evolution(monkeypatch):
    def prohibited(*args, **kwargs):
        raise AssertionError("runtime evolution is not an independent oracle")
    monkeypatch.setattr(F.S, "step", prohibited)
    monkeypatch.setattr(F.S.dense, "step", prohibited)
    monkeypatch.setattr(F.Q, "materialize", prohibited)
    for kind in ("seam", "straight-outward", "bent-opposed", "bent-transverse"):
        F.trace(next(case for case in F.cases() if case.kind == kind))


def test_compact_stream_is_exact_and_deterministic(tmp_path, records):
    a, b = tmp_path / "a.txt", tmp_path / "b.txt"
    first = F.write_fixtures(a, records)
    assert first == F.write_fixtures(b, records)
    assert first["cases"] == 243 and first["transactions"] == 11491
    assert first["fixture_bytes"] < 10 * 1024 * 1024
    lines = a.read_text(encoding="ascii").splitlines()
    assert lines[:2] == [F.MAGIC, "243"]
    offset = 2
    for fixture in records:
        assert lines[offset] == f"CASE {fixture.case.id} {len(fixture.events)}"
        assert bytes.fromhex(lines[offset + 1]) == F.native_state_wire(fixture.states[0])
        for line, state, events in zip(lines[offset + 2:], fixture.states[1:], fixture.events):
            state_hex, event_hex = line.split(" ")
            assert bytes.fromhex(state_hex) == F.native_state_wire(state)
            assert bytes.fromhex(event_hex) == F.native_event_wire(events)
        offset += 2 + len(fixture.events)
    assert offset == len(lines)
    assert F.validate_fixture_file(a) == {"cases": 243, "transactions": 11491}


def test_duplicate_missing_and_corrupted_fixtures_reject(tmp_path, records):
    with pytest.raises(ValueError, match="identity/order"):
        F.write_fixtures(tmp_path / "duplicate.txt", (records[0], records[0], *records[2:]))
    with pytest.raises(ValueError, match="incomplete"):
        F.write_fixtures(tmp_path / "missing.txt", records[:-1])
    path = tmp_path / "corrupted.txt"
    F.write_fixtures(path, records)
    data = path.read_bytes()
    # A valid wire hex byte changed in an otherwise complete expected stream.
    lines = data.splitlines(keepends=True)
    lines[4] = (b"0" if lines[4][:1] != b"0" else b"1") + lines[4][1:]
    path.write_bytes(b"".join(lines))
    with pytest.raises(ValueError, match="successor/event"):
        F.validate_fixture_file(path)


def test_native_current_observer_matches_every_independent_fixture(tmp_path, records):
    binary = os.environ.get("FTD_CARRIER_NATIVE_PROBE")
    if not binary:
        pytest.skip("native parity unavailable: set FTD_CARRIER_NATIVE_PROBE to the canonically built test executable")
    fixture_path = tmp_path / "native-fixtures.txt"
    F.write_fixtures(fixture_path, records)
    report = F.run_parity(binary, fixture_path)
    assert report["returncode"] == 0
    assert report["transactions"] == 11491


def test_missing_explicit_native_executable_is_an_error(tmp_path):
    with pytest.raises(FileNotFoundError, match="executable required"):
        F.run_parity(tmp_path / "missing.exe", tmp_path / "missing-fixtures.txt")


@pytest.mark.parametrize("stdout", ["", "FTD_CARRIER_CURRENT_FIXTURES_PASS cases=1 transactions=1\n",
    "FTD_CARRIER_CURRENT_FIXTURES_PASS cases=243 transactions=11491\n" * 2])
def test_zero_exit_without_exact_native_totals_is_not_parity(tmp_path, monkeypatch, stdout):
    binary, fixture = tmp_path / "stub.exe", tmp_path / "fixture.txt"
    binary.write_bytes(b"stub binary")
    fixture.write_bytes(b"fixture")
    monkeypatch.setattr(F, "source_hashes", lambda: {"test": "unchanged"})
    monkeypatch.setattr(F, "validate_fixture_file", lambda path: {"cases": 243, "transactions": 11491})
    monkeypatch.setattr(F.subprocess, "run", lambda *a, **k: SimpleNamespace(returncode=0, stdout=stdout, stderr=""))
    with pytest.raises(RuntimeError):
        F.run_parity(binary, fixture)


@pytest.mark.parametrize("mutated", ["binary", "fixture"])
def test_mutation_during_successful_native_call_invalidates_receipt(tmp_path, monkeypatch, mutated):
    binary, fixture = tmp_path / "stub.exe", tmp_path / "fixture.txt"
    binary.write_bytes(b"stub binary")
    fixture.write_bytes(b"fixture")
    monkeypatch.setattr(F, "source_hashes", lambda: {"test": "unchanged"})
    monkeypatch.setattr(F, "validate_fixture_file", lambda path: {"cases": 243, "transactions": 11491})
    def invoke(*args, **kwargs):
        (binary if mutated == "binary" else fixture).write_bytes(b"changed")
        return SimpleNamespace(returncode=0,
            stdout="FTD_CARRIER_CURRENT_FIXTURES_PASS cases=243 transactions=11491\n", stderr="")
    monkeypatch.setattr(F.subprocess, "run", invoke)
    with pytest.raises(RuntimeError, match="changed during native parity"):
        F.run_parity(binary, fixture)
