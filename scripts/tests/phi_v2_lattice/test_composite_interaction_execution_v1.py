"""Independent V2 codec/observer controls; never registered H304 preparation.

Native tests require all six implemented opcodes. Missing prerequisites fail;
they never skip or silently pass with a codec-only milestone.
"""
from dataclasses import replace
import hashlib
import importlib.util
from itertools import product
import os
from pathlib import Path
import struct
import subprocess
import sys
import uuid

import pytest

from phi_v2_lattice.experiments import certify_composite_interaction_v1 as C
from phi_v2_lattice import recovery_composite_interaction as I
from phi_v2_lattice import sparse_credit_exchange_q4 as S
from phi_v2_lattice import recovery_credit_exchange_q2 as Q

ROOT = C.ROOT
EVIDENCE = ROOT / "engine/docs/evidence/strict-recovery-wave8-2026-09-08"
FIXTURES = ROOT / "scripts/tests/phi_v2_lattice/test_sparse_credit_exchange_q4.py"
if C.sha(FIXTURES) != "9bd09eec5b3789af833340667dfd42fd40ae8db04c4b9c7c17def1e889173477":
    raise ValueError("frozen topology fixture source changed")
_spec = importlib.util.spec_from_file_location("composite_frozen_topology_inputs", FIXTURES)
F = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(F)


def synthetic_control(point, path=(0,), credit=(1, 0), heads=(0, 0), attempted=(0, 0), phase=0, origin=0, eta=0, L=64):
    axes = I.AXES[origin // 8]
    color = tuple(((origin >> a) & 1) ^ (point[axes[a]] % 2) for a in range(3))
    node = Q.encode(Q.QuotientState(phase, color, Q.Record(path, heads, credit, attempted)))
    return I.Q2Control(L, phase, origin, eta, point, node)


@pytest.fixture(scope="session")
def catalogue():
    return I.load_catalogue(ROOT / "engine/docs/evidence/strict-recovery-wave7-2026-09-08/q2-attempt-1/transitions.npz", I.CATALOGUE_SHA256)


@pytest.fixture(scope="session")
def evidence():
    path = Path(os.environ.get("FTD_COMPOSITE_EVIDENCE", str(EVIDENCE / ("composite-python-controls-" + uuid.uuid4().hex)))).resolve()
    path.mkdir(parents=True, exist_ok=False)
    C.write_new(path / "source-lock.json", C.canonical({"schema": "q4-composite-python-controls-1", "source_sha256": C.source_inventory(),
                  "test_sha256": C.sha(__file__), "registered_preparations": False, "maximum_synthetic_history_ticks": 38}))
    yield path
    C.write_new(path / "source-after.json", C.canonical({"source_sha256": C.source_inventory(), "test_sha256": C.sha(__file__)}))


@pytest.fixture(scope="session")
def exported(catalogue, evidence):
    path = evidence / "catalogue.bin"
    receipt = C.export_catalogue(catalogue.path, catalogue.sha256, path)
    return path, receipt["sha256"]


@pytest.fixture(scope="session")
def binary(evidence):
    explicit = os.environ.get("FTD_COMPOSITE_ADAPTER")
    if not explicit:
        raise ValueError("FTD_COMPOSITE_ADAPTER must identify an explicit frozen qualified build; no automatic prerequisite substitute")
    path = Path(explicit).resolve()
    receipt = C.verify_build(path, os.environ.get("FTD_COMPOSITE_BUILD_RECEIPT"), production=True)
    process = subprocess.run([str(path), "--identity"], capture_output=True, timeout=10, env=C.runtime_environment(compiler=receipt["compiler"]))
    C.write_new(evidence / "identity.stdout", process.stdout)
    C.write_new(evidence / "identity.stderr", process.stderr)
    assert process.returncode == 0
    row = C.parse_identity(process.stdout)
    assert row["schema"] == "strict-composite-native-adapter-identity-1"
    assert row["supported_probe_opcodes"] == [1, 2, 3, 4, 5, 6], "incomplete adapter prerequisite; no readiness skip"
    assert row["canonical_adoption"] is False
    yield path
    C.verify_build(path, os.environ.get("FTD_COMPOSITE_BUILD_RECEIPT"), production=True)


def test_production_build_qualification_rejects_incomplete_provenance(binary, evidence):
    receipt_path = Path(os.environ.get("FTD_COMPOSITE_BUILD_RECEIPT", str(binary.parent / "build.json")))
    original = C.parse_json(receipt_path.read_bytes())
    assert C.verify_build(binary, receipt_path, C.sha(receipt_path), production=True) == original
    mutations = []
    for field in ("tool_paths", "tool_version", "tool_sha256", "pre_tool_sha256", "post_tool_sha256",
                  "snapshot_sha256", "source_sha256", "include_sha256", "compiler_runtime_sha256"):
        row = C.parse_json(C.canonical(original))
        row[field].pop(next(iter(row[field])))
        mutations.append(("missing-" + field, row))
    row = C.parse_json(C.canonical(original)); row["schema"] = "q4-composite-adapter-build-1"; row["status"] = "PASS"
    mutations.append(("bounded-only-schema", row))
    row = C.parse_json(C.canonical(original)); row["command"].remove("-v")
    mutations.append(("missing-verbose-invocation", row))
    row = C.parse_json(C.canonical(original)); row["tool_version"]["collect2"]["exit_code"] = False
    mutations.append(("boolean-tool-exit", row))
    row = C.parse_json(C.canonical(original)); row["stderr_sha256"] = "0" * 64
    mutations.append(("unowned-include-log", row))
    retained = evidence / "build-qualification"
    retained.mkdir()
    for name, row in mutations:
        path = retained / (name + ".json")
        C.write_new(path, C.canonical(row))
        with pytest.raises((ValueError, KeyError)):
            C.verify_build(binary, path, C.sha(path), production=True)
    C.write_new(retained / "result.json", C.canonical({"qualified_build_sha256": C.sha(receipt_path),
                  "rejected_controls": [name for name, _ in mutations], "production_executed": False}))


def test_native_identity_is_distinct_from_verified_production_readiness(binary, evidence):
    raw = (evidence / "identity.stdout").read_bytes()
    row = C.validate_native_identity(0, raw)
    assert row["campaign_ready"] is False
    rejected = []
    for code in (1, -1, False, 0.0):
        with pytest.raises(ValueError, match="identity command"):
            C.validate_native_identity(code, raw)
        rejected.append("exit-" + repr(code))
    for field, value in (("schema", "foreign"), ("kernel_wire", "foreign"),
                         ("observation_encoding", "foreign"), ("compression", "foreign"),
                         ("supported_probe_opcodes", [1, 2, 3, 4, 5]),
                         ("supported_probe_opcodes", [1, 2, 3, 4, 5, 6, 6]),
                         ("supported_probe_opcodes", [1, 2, 3, 4, 5, 6.0]),
                         ("campaign_ready", 0), ("canonical_adoption", True)):
        changed = dict(row, **{field: value})
        with pytest.raises(ValueError):
            C.validate_native_identity(0, C.canonical(changed) + b"\n")
        rejected.append(field + "-" + repr(value))
    for malformed in (raw[:-1], raw + b"{}\n"):
        with pytest.raises(ValueError):
            C.validate_native_identity(0, malformed)
        rejected.append("malformed-wire")
    # Neither native flag can replace the separate exact production receipt.
    for native_flag in (False, True):
        identity = dict(row, campaign_ready=native_flag)
        assert C.validate_native_identity(0, C.canonical(identity) + b"\n") == identity
        with pytest.raises(ValueError):
            C.validate_production_readiness(C.canonical(identity), {}, "0" * 64)
    C.write_new(evidence / "native-identity-admission.json", C.canonical({
        "native_identity_sha256": hashlib.sha256(raw).hexdigest(),
        "native_campaign_ready": row["campaign_ready"], "negative_controls": rejected,
        "native_flags_cannot_supply_production_readiness": 2,
        "registered_preparations": 0, "scientific_transitions": 0}))


def test_real_coordinator_admission_stops_before_export(binary, catalogue, evidence, monkeypatch):
    import threading
    import time

    directory = evidence / "synthetic-coordinator-admission-only"
    directory.mkdir()
    marker = directory / "SYNTHETIC_NOT_CAMPAIGN_AUTHORITY.json"
    C.write_new(marker, C.canonical({"synthetic_admission_fixture_only": True,
                                   "campaign_authority": False, "scientific_calls": 0}))
    ref = lambda path: {"path": str(Path(path).resolve()), "sha256": C.sha(path)}
    build = Path(os.environ["FTD_COMPOSITE_BUILD_RECEIPT"])
    sources = C.source_inventory()
    pins = dict(zip(("npz", "report", "audit", "q2_source", "dense_source"),
                    (digest for _, digest in catalogue.pins)))
    specs = {key: ref(ROOT / "engine/docs" / name) for key, name in {
        "execution_v1": "SPEC_STRICT_COMPOSITE_INTERACTION_EXECUTION_V1.md",
        "execution_v2": "SPEC_STRICT_COMPOSITE_INTERACTION_EXECUTION_V2.md",
        "adapter_cli": "SPEC_STRICT_COMPOSITE_ADAPTER_CLI_V1.md",
        "kernel": "SPEC_STRICT_SPARSE_Q4_NATIVE_KERNEL_V1.md"}.items()}
    seam_calls = []

    class AdmissionSeamReached(Exception):
        pass

    def stop_before_export(*args, **kwargs):
        seam_calls.append(str(args[2]))
        raise AdmissionSeamReached("synthetic fixture stops before catalogue export/preparation")

    def forbidden(*args, **kwargs):
        raise AssertionError("no preparation or scientific step permitted in admission fixture")

    monkeypatch.setattr(C, "export_catalogue", stop_before_export)
    monkeypatch.setattr(C, "prepare_map", forbidden)
    monkeypatch.setattr(I, "prepare", forbidden)
    monkeypatch.setattr(S, "step", forbidden)
    outcomes = []
    for mode in ("accepted-identity-false", "missing-readiness", "stale-readiness"):
        attempt = directory / mode
        attempt.mkdir()
        storage = attempt / "storage.json"
        C.write_new(storage, C.canonical(C.disk_admission(attempt, ("head_on",))))
        ready = {"schema": "q4-composite-execution-readiness-2", "verdict": "PASS_SCOPED_EXACT_EXECUTION",
                 "campaign_ready": True, "canonical_adoption": False, "source_sha256": sources,
                 "build_sha256": C.sha(build), "gates": {name: "PASS" for name in C.PRODUCTION_GATES},
                 "integrated_suite": {"file_count": 19, "failed": 0, "errors": 0, "skipped": 0,
                                      "deselected": 0, "receipt": ref(marker)}}
        if mode == "stale-readiness":
            ready = dict(ready, source_sha256=dict(sources, **{str(Path(C.__file__).resolve()): "0" * 64}))
        ready_path = attempt / "SYNTHETIC_READINESS_NOT_CAMPAIGN_AUTHORITY.json"
        if mode != "missing-readiness":
            C.write_new(ready_path, C.canonical(ready))
            ready_ref = ref(ready_path)
        else:
            ready_ref = {"path": str(ready_path.resolve()), "sha256": "0" * 64}
        lock = {"schema": "q4-composite-execution-lock-2", "tier": "head_on", "N": 7296,
                "H": 304, "L": 64, "range_count": 64, "workers": 32, "deadline_seconds": 1800,
                "memory_bytes": 8 << 30, "source_sha256": sources, "catalogue_npz": ref(catalogue.path),
                "catalogue_pins": pins, "build": ref(build), "readiness": ready_ref,
                "preservation": ref(C.PRIOR_MANIFEST), "specifications": specs, "output": str(attempt),
                "storage": {"admission": ref(storage), "remaining_tiers": ["head_on"], "additional_copy_bytes": 0},
                "sorting": {"generations": 3, "fanin": 64, "record_cap": 2048, "chunk_records": 114,
                            "original_stream": "q4-original-checkpoint-ordered-controls-1",
                            "normalized_stream": "q4-normalized-checkpoint-1"}}
        lock_path = attempt / "lock.json"
        C.write_new(lock_path, C.canonical(lock))
        deadline = time.monotonic() + 60
        payload = {"prefix": sys.pycache_prefix, "lock_path": str(lock_path), "lock_sha256": C.sha(lock_path),
                   "deadline": deadline, "attempt_deadline": deadline}
        if mode == "accepted-identity-false":
            with pytest.raises(AdmissionSeamReached):
                C._production_coordinator(payload, threading.Barrier(1))
            assert C.parse_identity((attempt / "identity.stdout").read_bytes())["campaign_ready"] is False
            assert len(seam_calls) == 1
            assert not (attempt / "catalogue.bin").exists()
            assert not list(attempt.glob("range-*"))
            outcomes.append({"mode": mode, "actual_native_identity_sha256": C.sha(attempt / "identity.stdout"),
                             "reached_export_seam": True, "preparations": 0, "transitions": 0})
        else:
            with pytest.raises((OSError, ValueError)) as rejected:
                C._production_coordinator(payload, threading.Barrier(1))
            assert not (attempt / "identity.stdout").exists() and len(seam_calls) == 1
            outcomes.append({"mode": mode, "error": repr(rejected.value), "reached_export_seam": False,
                             "identity_invoked": False, "preparations": 0, "transitions": 0})
    C.write_new(directory / "result.json", C.canonical({"synthetic_admission_fixture_only": True,
        "campaign_authority": False, "real_coordinator_called": 3, "native_identity_invocations": 1,
        "export_seam_calls": len(seam_calls), "source_sha256": sources, "outcomes": outcomes}))


def compare(binary, exported, evidence, operations):
    path, digest = exported
    requests = [C.probe_request(op, **values) for op, values in operations]
    results = C.run_probe(binary, path, digest, requests, evidence / ("probe-" + uuid.uuid4().hex))
    return results


@pytest.mark.parametrize("L", (4, 6, 8, 32, 64))
def test_python_state_and_tick_codec_complete_contexts(L):
    base = F.winding_fixture() if L == 4 else F.topology_fixtures(L)[7][1]
    for origin, eta in product(range(48), range(2)):
        state = replace(base, origin_code=origin, charge_frame=eta, microtick=(1 << 16384) + 19)
        assert C.decode_state(C.state_wire(state)) == state
        raw = C.pack_payload(state)
        assert len(raw) == 13 + 3 * len(state.edges)
        assert C.unpack_payload(raw, state, 0) == state
        assert C.unpack_payload(raw, state, 304) == replace(state, microtick=state.microtick + 304)


@pytest.mark.parametrize("L", tuple(range(4, 65, 2)))
def test_python_codec_maximum_site_and_all_carrier_alphabets(L):
    for heading, attempted in product(range(1, 7), range(2)):
        state = S.initialize(L, tuple(S.Carrier(site, slot, heading, 1, attempted) for site in (0, L ** 3 - 1) for slot in range(2)), (), 47, 1)
        assert C.unpack_payload(C.pack_payload(state), state, 0) == state


@pytest.mark.parametrize("mutation", ("direction0", "direction7", "axis3", "reserved", "count5", "duplicate", "unordered", "site", "account", "gauss", "short", "extra"))
def test_python_codec_rejects_noncanonical_and_invalid_states(mutation):
    state = F.topology_fixtures(64)[2][1]
    raw = bytearray(C.pack_payload(state))
    if mutation in ("direction0", "direction7"):
        word = int.from_bytes(raw[:3], "little") & ~(7 << 19)
        raw[:3] = (word | ((7 if mutation.endswith("7") else 0) << 19)).to_bytes(3, "little")
    elif mutation == "axis3":
        raw[15] |= 12
    elif mutation == "reserved":
        raw[15] |= 128
    elif mutation == "count5":
        raw[12] = 5
    elif mutation == "duplicate":
        raw[3:6] = raw[:3]
    elif mutation == "unordered":
        raw[:3], raw[3:6] = raw[3:6], raw[:3]
    elif mutation == "site":
        state = replace(state, L=6)
        raw[:3] = ((int.from_bytes(raw[:3], "little") & ~0x3ffff) | 262143).to_bytes(3, "little")
    elif mutation == "account":
        raw[2] ^= 64
    elif mutation == "gauss":
        raw[13] ^= 1
    elif mutation == "short":
        del raw[-1:]
    else:
        raw += b"\0"
    with pytest.raises(ValueError):
        C.unpack_payload(raw, state, 0)


def test_catalogue_export_all_arrays_and_tamper(catalogue, exported):
    path, digest = exported
    raw = path.read_bytes()
    header, arrays = C.decode_export(raw, catalogue)
    assert C.sha(path) == digest
    assert sum(array.nbytes for array in arrays.values()) == 28297440
    assert [row["name"] for row in header["arrays"]] == sorted(catalogue.arrays)
    with pytest.raises(ValueError):
        C.decode_export(raw[:-1] + bytes([raw[-1] ^ 1]), catalogue)
    with pytest.raises(ValueError):
        C.decode_export(raw + b"\0", catalogue)


def test_action_tables_are_exact_group_and_state_actions():
    actions, inverse, products, backgrounds = C.action_tables()
    identity = actions.index((I.IDENTITY, 0))
    assert len(C.actions_wire()) == len(C.ACTION_MAGIC) + 19488
    state = S.transform(F.topology_fixtures(64)[7][1], I.IDENTITY, (32, 32, 32))
    for i, (matrix, charge) in enumerate(actions):
        assert products[i][inverse[i]] == identity == products[inverse[i]][i]
        shift = tuple(32 - 32 * sum(row) for row in matrix)
        for background in range(96):
            original = replace(state, origin_code=background // 2, charge_frame=background % 2)
            transformed = S.transform(original, matrix, shift, bool(charge))
            assert 2 * transformed.origin_code + transformed.charge_frame == backgrounds[i][background]
        for j in range(96):
            assert all(backgrounds[i][backgrounds[j][b]] == backgrounds[products[i][j]][b] for b in range(96))


def test_ordered_control_refinement_retains_state_groups(catalogue):
    # Adjacent independent R1 controls give an owner-key stable-sort tie.
    a = synthetic_control((32, 32, 32), phase=7)
    b = synthetic_control((32, 34, 32), phase=7)
    state = I.superpose_controls(a, b, catalogue)
    maps = C.equivalence_maps(((0, state, (a, b)), (1, state, (b, a)), (2, state, (a, b))))
    assert maps["G_state"] == 1 and maps["G_execution"] == 2 and maps["O"] == 1
    assert maps["state"] == ((0, 0, 3),) * 3
    assert maps["execution"] == ((0, 0, 2), (1, 1, 1), (0, 0, 2))
    record = C.original_sort_record(0, state, (a, b))
    assert C.decode_sort_record(record) == (0, S.checkpoint(state), (a, b))
    normalized = C.normalize_action(state)[0]
    assert C.decode_sort_record(C.normalized_sort_record(0, normalized), True) == (0, S.checkpoint(normalized), ())


def test_probe_response_rejects_order_missing_extra_and_oversize():
    good = struct.pack("<H", 2) + b"\x01" + C.blob(b"a") + b"\x02" + C.blob(b"b")
    assert C.decode_probe_output(b"\0" + C.blob(good), [5]) == ((0, {1: b"a", 2: b"b"}),)
    bad = (struct.pack("<H", 2) + b"\x02" + C.blob(b"a") + b"\x01" + C.blob(b"b"),
           good.replace(b"\x02" + C.blob(b"b"), b"\x01" + C.blob(b"b")),
           struct.pack("<H", 1) + b"\x01" + C.blob(b"a"), good + b"\0")
    for body in bad:
        with pytest.raises(ValueError):
            C.decode_probe_output(b"\0" + C.blob(body), [5])
    for body in (b"\x01" + C.blob(b"x" * 1025), b"\0" + struct.pack("<Q", (64 << 20) + 1), b"\x02" + C.blob(b"x")):
        with pytest.raises(ValueError):
            C.decode_probe_output(body, [5])


def test_actual_entry_cache_guard_and_direct_rejection(evidence, monkeypatch):
    monkeypatch.setenv("PYTHONOPTIMIZE", "1")
    monkeypatch.setenv("PYTHONINSPECT", "1")
    monkeypatch.setenv("PYTHONPATH", "untrusted-inherited-path")
    receipt = C.bootstrap_guard(evidence / "bootstrap")
    assert receipt["exit_code"] == 0 and receipt["cache_empty"]
    assert {"PYTHONOPTIMIZE", "PYTHONINSPECT", "PYTHONPATH"} <= set(receipt["removed_environment_names"])
    environment, _ = C.bootstrap_environment(evidence / "missing-prefix")
    process = subprocess.run([sys.executable, "-B", str(Path(C.__file__).resolve()), "--guard-entry", str(evidence / "missing-prefix")],
                             capture_output=True, env=environment, timeout=10)
    C.write_new(evidence / "direct-entry.stdout", process.stdout)
    C.write_new(evidence / "direct-entry.stderr", process.stderr)
    assert process.returncode != 0 and b"empty-prefix" in process.stderr


def test_all_remaining_storage_includes_spare_and_no_group_saving():
    assert C.reservation_bytes(tuple(C.TIER_COUNTS)) == 128411500544
    assert C.reservation_bytes(("full_impact",)) == 113093378048 + (2 << 30)
    assert 844 + 6 * 2050 <= 13 * 1024
    with pytest.raises(ValueError):
        C.reservation_bytes(("head_on", "head_on"))


def synthetic_begin(catalogue):
    controls = (synthetic_control((32, 32, 32), phase=1), synthetic_control((40, 32, 32), phase=1))
    state = I.superpose_controls(*controls, catalogue)
    # Metadata is a wire fixture; no registered preparation is generated.
    pid = I.PreparationID("head_on", (0, 0, 0), 1, 2, 1, 1, 0, 0, 0, 0, 0)
    return C.Begin(0, 0, 2, 0, pid, state, I.initial_lift(state), controls, (0, 0), 1)


def test_begin_original_and_orbit_record_bytes(catalogue):
    begin = synthetic_begin(catalogue)
    assert C.decode_begin(C.begin_wire(begin), "head_on") == begin
    row = {"index": 0, "preparation": begin.preparation, "checkpoint_sha256": hashlib.sha256(S.checkpoint(begin.state)).hexdigest(),
           "group": 0, "representative": 0, "orbit": 0, "background": 0, "owner_range": 0, "multiplicity": 2,
           "state_group": 0, "state_representative": 0, "state_multiplicity": 3}
    raw = C.original_row_wire(row)
    assert len(raw) == 76 and C.decode_original_row(raw, "head_on") == row
    normalized = C.normalize_action(begin.state)[0]
    assert C.decode_orbit_row(C.orbit_row_wire(0, normalized)) == (0, normalized)
    with pytest.raises(ValueError):
        C.original_row_wire(dict(row, owner_range=1))
    with pytest.raises(ValueError):
        C.decode_begin(C.begin_wire(begin) + b"\0", "head_on")


def test_original_alias_case_reconstruction_preserves_every_shared_field(catalogue, evidence, monkeypatch):
    # Synthetic transport only: this supplies no factory admission or H304
    # scientific CaseResult. Every evolutionary/preparation entry is forbidden.
    def forbidden(*args, **kwargs):
        raise AssertionError("alias transport must not prepare or evolve a history")
    for module, name in ((I, "prepare"), (I, "preparation_ids"), (I, "advance_control"), (S, "step")):
        monkeypatch.setattr(module, name, forbidden)
    begin = synthetic_begin(catalogue)
    controls = (begin.controls[0], synthetic_control((40, 32, 32), path=(1,), phase=1))
    state = I.superpose_controls(*controls, catalogue)
    seeds = (I._seed(1, 1, (32, 32, 32), 0).node, I._seed(2, 1, (40, 32, 32), 0).node)
    begin = replace(begin, state=state, lift=I.initial_lift(state), controls=controls, seed_nodes=seeds)
    alias_pid = replace(begin.preparation, sigma_A=-1)
    alias_seeds = (I._seed(1, -1, (32, 32, 32), 0).node, I._seed(2, 1, (40, 32, 32), 0).node)
    assert alias_seeds != seeds and controls[0].node != controls[1].node
    checkpoint = S.checkpoint(state)
    graph = S.observe(state)
    lift = I.observe_lift(state, begin.lift)
    escape = I.EscapeResult(False, False, False, (), (), (), None, None, 0, "synthetic alias transport")
    capture = I.CaptureResult(())
    provenance = (("catalogue_sha256", catalogue.sha256), ("seed_nodes", seeds), ("lift_steps", 1),
                  ("final_nodes", tuple(c.node for c in controls)))
    original_provenance = (("catalogue_sha256", catalogue.sha256), ("seed_nodes", alias_seeds), ("lift_steps", 1),
                           ("final_nodes", tuple(c.node for c in controls)))
    representative = I.CaseResult(begin.preparation, provenance, 304, (checkpoint,)*305, ("0"*64,)*304,
                  None, None, None, None, escape, capture, graph, ((graph.F, graph.K, graph.Q),)*304,
                  (), "UNRESOLVED_AT_304", (lift,)*305)
    expected = I.CaseResult(alias_pid, original_provenance, 304, (checkpoint,)*305, ("0"*64,)*304,
                  None, None, None, None, escape, capture, graph, ((graph.F, graph.K, graph.Q),)*304,
                  (), "UNRESOLVED_AT_304", (lift,)*305)
    row = {"index": 1, "preparation": alias_pid, "checkpoint_sha256": hashlib.sha256(checkpoint).hexdigest(),
           "group": 0, "representative": 0, "orbit": 0, "background": 0, "owner_range": 0, "multiplicity": 2,
           "state_group": 0, "state_representative": 0, "state_multiplicity": 2}
    restored = C.reconstruct_alias_case(row, begin, representative)
    assert restored == expected
    for field in I.CaseResult.__dataclass_fields__:
        assert getattr(restored, field) == getattr(expected, field)
        if field not in ("preparation_id", "provenance"):
            assert getattr(restored, field) is getattr(representative, field)
    representative_row = dict(row, index=0, preparation=begin.preparation)
    assert C.reconstruct_alias_case(representative_row, begin, representative) == representative
    wrong_rows = ({"group": 1}, {"representative": 1}, {"multiplicity": 3}, {"orbit": 1},
                  {"checkpoint_sha256": "f"*64}, {"background": 2},
                  {"preparation": replace(alias_pid, stage_advance=1)}, {"index": 0})
    for changed in wrong_rows:
        with pytest.raises(ValueError):
            C.reconstruct_alias_case(dict(row, **changed), begin, representative)
    wrong_provenances = (original_provenance,
                         (("catalogue_sha256", "0"*64),)+provenance[1:],
                         provenance[:2]+(("lift_steps", True),)+provenance[3:],
                         provenance[:-1]+(("final_nodes", tuple(reversed(provenance[-1][1]))),))
    for wrong in [replace(representative, preparation_id=alias_pid),
                  replace(representative, preparation_id=replace(begin.preparation, stage_advance=False)),
                  replace(representative, checkpoints=(b"foreign",)+representative.checkpoints[1:]),
                  replace(representative, completed_ticks=303),
                  *(replace(representative, provenance=p) for p in wrong_provenances)]:
        with pytest.raises(ValueError):
            C.reconstruct_alias_case(row, begin, wrong)
    folder = evidence / "original-alias-transport"
    folder.mkdir()
    C.write_new(folder / "result.json", C.canonical({"schema": "q4-original-alias-transport-control-1", "status": "PASS",
           "compared_case_result_fields": tuple(I.CaseResult.__dataclass_fields__), "distinct_original_provenance": True,
           "rejected_wrong_rows": len(wrong_rows), "rejected_wrong_results": 8,
           "preparation_calls": 0, "transition_calls": 0, "scientific_case_acceptance": False,
           "representative_provenance": repr(provenance), "original_provenance": repr(original_provenance)}))


def range_header():
    return {"schema": "q4-composite-range-2", "tier": "head_on", "range": 0, "start": 0, "stop": 114,
            "lock_sha256": "0" * 64, "input_descriptor_sha256": "1" * 64, "executable_sha256": "2" * 64,
            "group_map_sha256": "3" * 64, "H": 304, "L": 64, "kernel_wire": C.WIRE_ID,
            "observation_encoding": "frozen-python-case-result-1", "compression": C.CODEC}


def test_append_only_frame_recovery_never_skips_or_repairs(catalogue):
    begin = C.begin_wire(synthetic_begin(catalogue))
    header = C.RANGE_MAGIC + C.blob(C.canonical(range_header()))
    first = C.frame_wire(0, 1, begin, bytes(32))
    second = C.frame_wire(1, 2, b"synthetic body", first[-32:])
    good = header + first
    assert C.decode_frames(good)["accepted_bytes"] == len(good)
    for suffix in (second[:-1], second[:-1] + bytes([second[-1] ^ 1]), b"bad" + second,
                   C.FRAME_MAGIC + struct.pack("<QBQ", 1, 2, C.FRAME_CAP + 1),
                   C.frame_wire(2, 2, b"", first[-32:])):
        raw = good + suffix
        result = C.decode_frames(raw)
        assert result["accepted_bytes"] == len(good) and len(result["frames"]) == 1 and result["tail_error"]
        assert result["raw_sha256"] == hashlib.sha256(raw).hexdigest()
        assert result["tail_sha256"] == hashlib.sha256(suffix).hexdigest()
    v1 = header.replace(C.RANGE_MAGIC, b"FTD-Q4-COMPOSITE-RANGE-1\n") + first
    assert C.decode_frames(v1)["accepted_bytes"] == 0


def test_reconstruct_complete_synthetic_progress_and_incomplete_terminal(catalogue, evidence):
    begin = synthetic_begin(catalogue)
    case = C._new_case(begin)
    controls, state, lift = begin.controls, begin.state, begin.lift
    ticks, diagnostics, first, counts, chain = [], {}, [0] * 4, [0] * 4, bytes(32)
    for offset in range(1, 20):
        after, events = S.step(state)
        following = tuple(I.advance_control(c, catalogue) for c in controls)
        observation = I.observe_step(state, after, events, controls, following, catalogue)
        next_lift = I.advance_lift(state, after, events, lift)
        ledger = I.observe_lift(after, next_lift, begin.state, begin.lift)
        chain, _ = C.observation_chain(chain, after, events, observation, ledger)
        flags = C.flags_byte(observation)
        ticks.append(C.tick_wire(offset, after, next_lift, events, chain, flags))
        for bit in range(4):
            if flags & (1 << bit):
                counts[bit] += 1
                if not first[bit]:
                    first[bit] = offset
                    extra = b""
                    if bit == 0:
                        extra = C.keys_wire(observation.contact_outputs)
                    elif bit == 2:
                        isolated = I._merge_events(*(I.advance_control(c, catalogue, True)[1] for c in controls), I.superpose_controls(*controls, catalogue))
                        wire = C.event_wire(isolated)
                        extra = struct.pack("<H", len(wire)) + wire
                    elif bit == 3:
                        extra = C.conflict_wire(observation.conflict)
                    diagnostics[bit] = struct.pack("<H", offset) + extra
        state, controls, lift = after, following, next_lift
    body = (struct.pack("<IHH", 0, 0, 19) + b"".join(ticks) + bytes([sum(1 << bit for bit in diagnostics)])
            + b"".join(diagnostics[bit] for bit in sorted(diagnostics)) + C.controls_wire(controls, state))
    recovered = C._progress_body(body, case, catalogue)
    assert len(recovered["chain"]) == 19 and recovered["history"][-1] == state
    assert len(case["chain"]) == 0, "invalid frame must not mutate its durable predecessor"
    with pytest.raises(ValueError):
        C._progress_body(body[:-21] + bytes([body[-21] ^ 1]) + body[-20:], case, catalogue)
    graph = C.graph_wire(state)
    terminal = (struct.pack("<IHB", 0, 19, 1) + chain + struct.pack("<8H", *first, *counts)
                + struct.pack("<H", len(graph)) + graph + b"\0\0\0" + bytes(287)
                + struct.pack("<BHH", 3, 6, 8) + b"deadline")
    frames = ({"kind": 1, "body": C.begin_wire(begin)}, {"kind": 2, "body": body}, {"kind": 3, "body": terminal})
    result = C.reconstruct_case_frames(frames, catalogue, begin)
    assert result["completed_ticks"] == 19 and result["status"] == 1 and result["case_result"] is None
    stored = C._stored_progress(body, C._new_case(begin))
    assert C._stored_final(terminal, stored)["status"] == 1
    class FixtureMap:
        def begin(self, group):
            assert group == 0
            return begin
        def original(self, index):
            assert 0 <= index < 114
            return {"group": 0, "owner_range": 0}
    footer = {"range": 0, "status": 1, "start": 0, "stop": 114, "owned_groups": 1, "completed_groups": 0,
              "reported_executed_ticks": 19, "verified_durable_ticks": 19, "locally_covered_originals": 0,
              "unresolved_aliases": 114, "case_terminal_chain": hashlib.sha256(b"").hexdigest(),
              "group_map_sha256": "3" * 64, "error_code": 6, "error": "deadline"}
    prefix = C.RANGE_MAGIC + C.blob(C.canonical(range_header()))
    previous = bytes(32)
    for sequence, (kind, raw) in enumerate(((1, C.begin_wire(begin)), (2, body), (4, C.range_final_wire(footer)))):
        wire = C.frame_wire(sequence, kind, raw, previous)
        previous = wire[-32:]
        prefix += wire
    path = evidence / "synthetic-interrupted-range.bin"
    C.write_new(path, prefix)
    complete = bytearray(1)
    report = C.reduce_range(path, range_header(), FixtureMap(), (0,), complete,
                            lambda *args: (_ for _ in ()).throw(AssertionError("incomplete case cannot contribute")))
    assert report["verified_durable_ticks"] == 19 and report["completed_groups"] == 0
    assert report["accepted_frames"] == 3 and report["tail_bytes"] == 0 and report["status"] == "INTERRUPTED"
    assert not any(complete) and path.read_bytes() == prefix


def test_streamed_reader_preserves_semantically_rejected_suffix(catalogue, evidence):
    header = C.RANGE_MAGIC + C.blob(C.canonical(range_header()))
    first = C.frame_wire(0, 1, C.begin_wire(synthetic_begin(catalogue)), bytes(32))
    bad = C.frame_wire(1, 2, b"authenticated but semantically invalid", first[-32:])
    tail = bad + b"retained truncated suffix"
    path = evidence / "semantic-suffix.bin"
    C.write_new(path, header + first + tail)
    with C.RangeStream(path, range_header()) as reader:
        frame = reader.next_frame()
        reader.commit(frame)
        frame = reader.next_frame()
        assert frame["body"] == b"authenticated but semantically invalid"
        reader.reject(ValueError("synthetic semantic rejection"))
        result = reader.finish()
    assert result["accepted_bytes"] == len(header + first) and result["accepted_frames"] == 1
    assert result["tail_sha256"] == hashlib.sha256(tail).hexdigest()
    assert result["raw_sha256"] == C.sha(path) and path.read_bytes() == header + first + tail


@pytest.mark.parametrize("kind", ("success", "deadline", "memory", "quota", "log"))
def test_actual_frozen_supervisor_assignment_and_resource_cleanup(evidence, kind):
    directory = evidence / ("actual-supervisor-" + kind)
    result = C.supervisor_control(directory, kind)
    assert result["actual_job_assignment"] and result["descendant_alive_after_cleanup"] is False
    assert not result["coordinator_alive_after_cleanup"] and result["source_unchanged"]
    assert (directory / "assigned-entry.json").exists()
    if kind == "success":
        assert result["exit_code"] == 0 and result["failure"] is None
    else:
        assert result["exit_code"] != 0 or result["failure"]
        assert not (directory / "control-complete.json").exists()
    if kind == "log":
        assert 8192 < (directory / "stdout").stat().st_size <= 8192 + 4096


def test_offset_zero_requires_identical_full_state_and_lift(binary, exported, evidence, catalogue):
    begin = synthetic_begin(catalogue)
    different = replace(begin.state, carriers=(replace(begin.state.carriers[0], direction=2),) + begin.state.carriers[1:])
    changed_lift = replace(begin.lift, carriers=tuple(tuple(v + (64 if a == 0 else 0) for a, v in enumerate(p)) for p in begin.lift.carriers),
                           edges=tuple(tuple(v + (64 if a == 0 else 0) for a, v in enumerate(p)) for p in begin.lift.edges))
    requests = []
    for reference, reference_lift in ((different, begin.lift), (begin.state, changed_lift)):
        values = {"state": begin.state, "reference": reference, "lift": begin.lift, "reference_lift": reference_lift,
                  "controls": begin.controls, "previous_chain": bytes(32), "no_wrap": False}
        with pytest.raises(ValueError, match="offset-zero"):
            C.reference_probe(2, catalogue, **values)
        with pytest.raises(ValueError, match="offset-zero"):
            C.probe_request(2, **values)
        raw = (C.blob(C.state_wire(begin.state)) + C.blob(C.state_wire(reference)) + C.lift_wire(begin.state, begin.lift)
               + C.lift_wire(reference, reference_lift) + C.controls_wire(begin.controls, begin.state) + bytes(33))
        requests.append((2, raw))
    result = C.run_probe(binary, exported[0], exported[1], requests, evidence / "offset-zero-rejections")
    assert all(status == 1 for status, _ in result)


def test_real_entry_rejects_wrong_owned_source_digest(evidence):
    directory = evidence / "entry-source-rejection"
    directory.mkdir()
    prefix = directory / "empty-cache"
    prefix.mkdir()
    environment, _ = C.bootstrap_environment(prefix)
    command = [sys.executable, "-B", "-X", "pycache_prefix=" + str(prefix), str(Path(C.__file__).resolve()),
               "--guard-entry", str(prefix), "--entry-sha256", "0" * 64]
    result = subprocess.run(command, env=environment, cwd=ROOT, capture_output=True, timeout=10)
    C.write_new(directory / "stdout", result.stdout)
    C.write_new(directory / "stderr", result.stderr)
    assert result.returncode != 0 and b"actual entry source identity mismatch" in result.stderr
    assert not any(prefix.iterdir())


def test_actual_outer_monitor_os_cap_and_scientific_child_breakaway(evidence):
    result = C.supervisor_control(evidence / "actual-outer-memory", "outer_memory")
    assert result["actual_outer_assignment"] and result["outer_oversized_allocation_rejected"]
    assert result["child_in_outer_job"] is False and result["child_exit"] == 0
    assert result["child_alive_after_cleanup"] is False
    assert result["outer_peak_private_bytes"] <= C.OUTER_MONITOR_CAP
    assert result["child_in_cleanup_job"]


@pytest.mark.parametrize("kind", ("outer_loss", "supervisor_loss", "outer_timeout"))
def test_actual_metadata_parent_loss_cannot_orphan_breakaway_descendants(evidence, kind):
    result = C.supervisor_control(evidence / ("actual-" + kind), kind)
    assert result["surviving_descendants"] == []
    assert result["ready"]["supervisor_in_outer_cleanup"]
    assert result["ready"]["supervisor_in_outer_memory_job"] is False


def test_global_alias_counts_keep_state_and_ordered_execution_relations():
    class Map:
        header = {"N": 128, "G_execution": 3, "G_state": 2, "O": 1}
        bad = False
        def group_metadata(self, group):
            return group, group, (43, 43, 42)[group], 0
        def original(self, index):
            group = index % 3
            state = int(group == 2)
            return {"index": index, "group": group, "state_group": state, "orbit": 0, "background": 0,
                    "representative": group, "multiplicity": (43, 43, 42)[group], "owner_range": group // 2,
                    "state_representative": 2 if state else 0, "state_multiplicity": 42 if state else 86,
                    "checkpoint_sha256": ("f" if self.bad and index == 127 else str(state)) * 64}
    mapping = Map()
    counts, ranges = C.validate_all_aliases(mapping)
    assert tuple(counts) == (43, 43, 42) and ranges[0] == (0, 1) and ranges[1] == (2,)
    assert all(not values for values in ranges[2:])
    mapping.bad = True
    with pytest.raises(ValueError, match="state identity"):
        C.validate_all_aliases(mapping)


def test_native_invocation_accounting_transport_fields_only():
    report = {"range": 3, "status": "COMPLETE", "verified_durable_ticks": 304,
              "terminal": {"reported_executed_ticks": 608, "verified_durable_ticks": 304}}
    row = {"schema": "q4-composite-native-range-terminal-1", "range": 3, "complete": True,
           "reported_executed_ticks": 608, "primary_executed_ticks": 304, "certificate_replay_ticks": 304,
           "verified_durable_ticks": 304, "support_cache_used": False, "canonical_adoption": False}
    assert C.decode_native_terminal(C.canonical(row) + b"\n", report) == row
    for changed in ({"reported_executed_ticks": 304}, {"primary_executed_ticks": 303}, {"verified_durable_ticks": 305}, {"complete": False}):
        with pytest.raises(ValueError):
            C.decode_native_terminal(C.canonical(dict(row, **changed)) + b"\n", report)
    for bad in (True, False, 1.0, -1, 64):
        with pytest.raises(ValueError, match="native terminal range"):
            C.decode_native_terminal(C.canonical(dict(row, range=bad)) + b"\n", dict(report, range=int(bad)))


def test_full_h304_transport_shape_without_evolution_or_case_acceptance(catalogue, evidence, monkeypatch):
    begin = synthetic_begin(catalogue)
    def forbidden(*args, **kwargs):
        raise AssertionError("transport fixture must not execute or accept scientific history")
    monkeypatch.setattr(S, "step", forbidden)
    monkeypatch.setattr(I, "prepare", forbidden)
    monkeypatch.setattr(I, "capture_certificate", forbidden)
    monkeypatch.setattr(C, "_final_body", forbidden)
    empty_events = C.decode_events(bytes(8))
    header = C.RANGE_MAGIC + C.blob(C.canonical(range_header()))
    rows = [(1, C.begin_wire(begin))]
    snapshots, offsets = [begin.state], []
    for previous in range(0, 304, 19):
        ticks = []
        for offset in range(previous + 1, previous + 20):
            # These are positions in the transport grammar only. Payload is
            # deliberately held fixed and is never called a native trajectory.
            state = replace(begin.state, microtick=begin.state.microtick + offset)
            tick = C.tick_wire(offset, state, begin.lift, empty_events, bytes(32), 0)
            reader = C.Reader(tick)
            decoded = C.read_tick(reader, begin.state)
            reader.done()
            assert decoded[0] == offset and decoded[1] == state
            snapshots.append(state)
            offsets.append(offset)
            ticks.append(tick)
        controls = tuple(replace(control, microtick=snapshots[-1].microtick) for control in begin.controls)
        rows.append((2, struct.pack("<IHH", 0, previous, previous + 19) + b"".join(ticks) + b"\0" + C.controls_wire(controls, snapshots[-1])))
    pairs = tuple((start, end) for end in range(305) for start in range(end) if (end - start) % 19 == 0)
    assert len(snapshots) == 305 and offsets == list(range(1, 305)) and len(rows) == 17 and len(pairs) == 2296
    witnesses = tuple(I.CaptureWitness(start, end, begin.state.microtick + start, begin.state.microtick + end, end - start,
                                      (0, 0, 0), False, 0, ()) for start, end in pairs)
    bitmap = C.capture_wire(I.CaptureResult(witnesses))
    assert bitmap == bytes([255]) * 287
    for index, witness in enumerate(witnesses):
        one = C.capture_wire(I.CaptureResult((witness,)))
        assert one[index // 8] == 1 << (index % 8) and sum(byte.bit_count() for byte in one) == 1
    graph = C.graph_wire(snapshots[-1])
    terminal = (struct.pack("<IHB", 0, 304, 0) + bytes(32 + 16) + struct.pack("<H", len(graph)) + graph
                + b"\1" + struct.pack("<H", 7) + bytes(7) + bitmap + struct.pack("<BHH", 0, 0, 0))
    rows.append((3, terminal))
    previous_digest = bytes(32)
    wire = header
    for sequence, (kind, body) in enumerate(rows):
        frame = C.frame_wire(sequence, kind, body, previous_digest)
        wire += frame
        previous_digest = frame[-32:]
    footer = {"range": 0, "status": 0, "start": 0, "stop": 114, "owned_groups": 1, "completed_groups": 1,
              "reported_executed_ticks": 608, "verified_durable_ticks": 304, "locally_covered_originals": 114,
              "unresolved_aliases": 0, "case_terminal_chain": hashlib.sha256(previous_digest).hexdigest(),
              "group_map_sha256": "3" * 64, "error_code": 0, "error": ""}
    wire += C.frame_wire(18, 4, C.range_final_wire(footer), previous_digest)
    path = evidence / "h304-transport-only.bin"
    C.write_new(path, wire)
    parsed = C.decode_frames(wire, range_header())
    assert len(parsed["frames"]) == 19 and parsed["tail_error"] is None
    truncated = C.decode_frames(wire[:-1], range_header())
    assert len(truncated["frames"]) == 18 and truncated["tail_error"]
    with pytest.raises(ValueError):
        C.read_tick(C.Reader(bytes(2) + ticks[0][2:]), begin.state)
    C.write_new(evidence / "h304-transport-only.json", C.canonical({"schema": "q4-composite-synthetic-transport-control-1",
              "snapshot_positions": 305, "tick_records": 304, "progress_blocks": 16, "capture_bit_positions": 2296,
              "actual_joint_transitions": 0, "registered_preparation_calls": 0, "scientific_case_result_accepted": False,
              "wire_sha256": C.sha(path)}))


def test_native_codec_and_inspection(binary, exported, evidence, catalogue):
    operations = []
    for name, base in F.topology_fixtures(64):
        state = S.transform(base, I.IDENTITY, (32, 32, 32))
        for op in (1, 5):
            operations.append((op, {"state": state}))
        operations.append((6, {"begin": state, "offset": 304, "payload": C.pack_payload(state)}))
    winding = replace(F.winding_fixture(), microtick=1 << 16384)
    operations.append((5, {"state": winding}))
    results = compare(binary, exported, evidence, operations)
    for result, (op, values) in zip(results, operations):
        assert result == (0, C.reference_probe(op, catalogue, **values))


def test_native_observed_steps_all_stages_order_and_hash(binary, exported, evidence, catalogue):
    operations = []
    for phase in range(19):
        controls = (synthetic_control((32, 32, 32), phase=phase), synthetic_control((40, 32, 32), phase=phase))
        state = I.superpose_controls(*controls, catalogue)
        lift = I.initial_lift(state)
        for ordered in (controls, controls[::-1]):
            operations.append((2, {"state": state, "reference": state, "lift": lift, "reference_lift": lift,
                                   "controls": ordered, "previous_chain": bytes(32), "no_wrap": True}))
    results = compare(binary, exported, evidence, operations)
    for result, (op, values) in zip(results, operations):
        assert result == (0, C.reference_probe(op, catalogue, **values))


def test_native_bounded_capture_escape_and_nonzero_tick_offset(binary, exported, evidence, catalogue):
    controls = (synthetic_control((32, 32, 32)), synthetic_control((40, 32, 32)))
    state = I.superpose_controls(*controls, catalogue)
    history, lifts = [state], [I.initial_lift(state)]
    chain = bytes(32)
    operations = []
    for _ in range(38):
        values = {"state": history[-1], "reference": state, "lift": lifts[-1], "reference_lift": lifts[0],
                  "controls": controls, "previous_chain": chain, "no_wrap": True}
        expected = C.reference_probe(2, catalogue, **values)
        operations.append((2, values))
        following = C.decode_state(expected[1])
        controls = tuple(I.advance_control(c, catalogue) for c in controls)
        lifts.append(I.advance_lift(history[-1], following, C.decode_events(expected[2]), lifts[-1]))
        history.append(following)
        chain = expected[9]
    operations += [(3, {"history": tuple(history), "lifts": tuple(lifts)}), (4, {"state": history[-1]})]
    for result, (op, values) in zip(compare(binary, exported, evidence, operations), operations):
        assert result == (0, C.reference_probe(op, catalogue, **values))


def observed_values(state, controls):
    lift = I.initial_lift(state)
    return {"state": state, "reference": state, "lift": lift, "reference_lift": lift,
            "controls": controls, "previous_chain": bytes(32), "no_wrap": True}


def test_native_phase_axis_charge_matrix_and_huge_ordinals(binary, exported, evidence, catalogue):
    operations = []
    for phase, heading, eta in product(range(19), range(6), range(2)):
        controls = tuple(synthetic_control(point, path=(heading,), heads=(heading, heading), phase=phase, eta=eta)
                         for point in ((24, 24, 24), (40, 40, 40)))
        operations.append((2, observed_values(I.superpose_controls(*controls, catalogue), controls)))
    huge = 1 << 16384
    for phase in range(19):
        ordinal = huge + ((phase - huge) % 19)
        controls = tuple(replace(synthetic_control(point, phase=phase), microtick=ordinal) for point in ((24, 24, 24), (40, 40, 40)))
        operations.append((2, observed_values(I.superpose_controls(*controls, catalogue), controls)))
    for result, (op, values) in zip(compare(binary, exported, evidence, operations), operations):
        assert result == (0, C.reference_probe(op, catalogue, **values))


def test_native_stable_tie_alignment_midpoint_and_invalid_unions(binary, exported, evidence, catalogue):
    a = synthetic_control((32, 32, 32), credit=(0, 1), phase=7)
    b = synthetic_control((33, 32, 32), heads=(1, 1), phase=7)
    state = I.superpose_controls(a, b, catalogue)
    operations = [(2, observed_values(state, controls)) for controls in ((a, b), (b, a))]
    expected = [C.reference_probe(2, catalogue, **values) for _, values in operations]
    assert expected[0][11] != expected[1][11] and expected[0][6] == expected[1][6]
    assert expected[0][13][0] & 4
    maps = C.equivalence_maps(((0, state, (a, b)), (1, state, (b, a))))
    assert maps["G_state"] == 1 and maps["G_execution"] == 2
    for heading in (0, 2):
        controls = (synthetic_control((10, 10, 10)), synthetic_control((11, 10, 10), path=(2,), heads=(heading, heading)))
        values = observed_values(I.superpose_controls(*controls, catalogue), controls)
        flags = C.reference_probe(2, catalogue, **values)[13][0]
        assert flags & 4 and bool(flags & 2) == bool(heading)
        operations.append((2, values))
    controls = (synthetic_control((10, 10, 10), path=(2,), heads=(2, 2), phase=7),
                synthetic_control((11, 9, 10), path=(2,), heads=(2, 2), phase=7))
    operations.append((2, observed_values(I.superpose_controls(*controls, catalogue), controls)))
    a = synthetic_control((32, 32, 32))
    actual = S.transform(F.topology_fixtures(64)[0][1], I.IDENTITY, (32, 32, 32))
    invalid = observed_values(actual, (a, a))
    assert C.reference_probe(2, catalogue, **invalid)[13][0] & 8
    operations.append((2, invalid))
    for result, (op, values) in zip(compare(binary, exported, evidence, operations), operations):
        assert result == (0, C.reference_probe(op, catalogue, **values))


def test_native_distinct_infinite_and_torus_escape_and_graph_negatives(binary, exported, evidence, catalogue):
    operations = []
    for co_moving in (False, True):
        heading = 0 if co_moving else 1
        controls = (synthetic_control((20, 20, 32), path=(heading,), heads=(heading, heading), phase=1),
                    synthetic_control((44, 44 if co_moving else 20, 32), phase=1))
        state = I.superpose_controls(*controls, catalogue)
        result = I.escape_certificate(state, catalogue)
        assert result.applicable and result.infinite_lift and result.periodic_torus == co_moving
        decoded = C.decode_escape(C.escape_wire(state, result), state)
        assert decoded["drifts"] == result.drifts and decoded["infinite_lift"] == result.infinite_lift and decoded["periodic_torus"] == result.periodic_torus
        operations.append((4, {"state": state}))
    for index in (8, 9, 10):
        state = S.transform(F.topology_fixtures(64)[index][1], I.IDENTITY, (32, 32, 32))
        negative = I.escape_certificate(state, catalogue)
        assert not negative.applicable and not C.decode_escape(C.escape_wire(state, negative), state)["applicable"]
        operations.append((4, {"state": state}))
    for result, (op, values) in zip(compare(binary, exported, evidence, operations), operations):
        assert result == (0, C.reference_probe(op, catalogue, **values))


def test_native_actual_positive_capture_on_fixed_occupied_cycle(binary, exported, evidence, catalogue):
    # Two adjacent neutral sites on a four-edge circulation. Both polarities
    # face the occupied matching partner, so capacity holds block every hop.
    # Zero credits make every exchange an identity. Reset restores the same
    # attempt pattern each19 ticks: a registered synthetic positive, no search.
    site = lambda point: S.site_index(64, point)
    a, b, c, d = (32, 32, 32), (33, 32, 32), (33, 33, 32), (32, 33, 32)
    carriers = tuple(S.Carrier(site(point), slot, direction, 0, 0) for point, direction in ((a, 1), (b, 2)) for slot in range(2))
    edges = (S.Edge(site(a), 0, 1), S.Edge(site(b), 1, 1), S.Edge(site(d), 0, -1), S.Edge(site(a), 1, -1))
    state = S.initialize(64, carriers, edges, 0, 0)
    history, lifts = [state], [I.initial_lift(state)]
    for _ in range(38):
        following, events = S.step(history[-1])
        assert not events.moves
        lifts.append(I.advance_lift(history[-1], following, events, lifts[-1]))
        history.append(following)
    capture = I.capture_certificate(tuple(history), tuple(lifts))
    assert capture.witnesses and any(w.start == 1 and w.end == 20 and w.translation == (0, 0, 0) for w in capture.witnesses)
    operation = (3, {"history": tuple(history), "lifts": tuple(lifts)})
    assert compare(binary, exported, evidence, [operation])[0] == (0, C.reference_probe(3, catalogue, **operation[1]))


def test_external_sort_three_retained_generations_and_both_classes(catalogue, evidence):
    directory = evidence / "synthetic-sort"
    directory.mkdir()
    records = []
    for index in range(12):
        origin = index % 2
        controls = tuple(synthetic_control(point, phase=1, origin=origin) for point in ((24, 24, 24), (40, 40, 40)))
        if index % 3:
            controls = controls[::-1]
        records.append((index, I.superpose_controls(*controls, catalogue), controls))
    original, normalized = directory / "original.bin", directory / "normalized.bin"
    C.write_new(original, b"".join(C.original_sort_record(index, state, controls) for index, state, controls in records))
    C.write_new(normalized, b"".join(C.normalized_sort_record(index, C.normalize_action(state)[0]) for index, state, _ in records))
    a = C.external_sort_stream(original, directory / "original-sort", False, 3)
    b = C.external_sort_stream(normalized, directory / "normalized-sort", True, 3)
    for source, report in ((original, a), (normalized, b)):
        assert len(report["runs"]) == 4 and report["generations"] == 3 and report["records"] == 12
        assert report["retained_bytes"] == 3 * source.stat().st_size
        assert all(Path(path).is_file() for path in report["runs"])
    arrays, counts = C._class_arrays(a["merged"], b["merged"], 12, lambda: None)
    expected = C.equivalence_maps(records)
    for kind in ("state", "execution", "orbit"):
        assert tuple(C._class_row(arrays, kind, index) for index in range(12)) == expected[kind]
    assert counts["state"] == expected["G_state"] and counts["execution"] == expected["G_execution"]
    C.write_new(directory / "truncated.bin", original.read_bytes()[:-1])
    with pytest.raises(ValueError):
        list(C.iter_sort_records(directory / "truncated.bin"))


def test_complete_support_template_export_without_trajectories(catalogue, evidence):
    path = evidence / "templates.bin"
    receipt = C.export_templates(catalogue, path)
    assert C.sha(path) == receipt["sha256"] and path.stat().st_size < 3 << 20
    header = C.validate_templates(path.read_bytes(), catalogue)
    assert header["count"] == 1824
    raw = bytearray(path.read_bytes())
    raw[-1] ^= 1
    with pytest.raises(ValueError):
        C.validate_templates(raw, catalogue)


def test_admission_reenters_factory_and_rejects_forged_public_handle(catalogue, exported, evidence, monkeypatch):
    accepted, _, arrays = C.admit_export(catalogue.path, catalogue.sha256, exported[0], exported[1])
    assert accepted.pins == catalogue.pins and len(arrays) == 12
    changed = dict(catalogue.arrays)
    changed["operation"] = changed["operation"].copy()
    changed["operation"][0] ^= 1
    forged = I.Catalogue(catalogue.path, catalogue.sha256, catalogue.pins, changed)
    def forbidden(*args, **kwargs):
        raise AssertionError("no registered preparation may execute in this test")
    monkeypatch.setattr(I, "prepare", forbidden)
    with pytest.raises(ValueError, match="forged"):
        C.prepare_map(forged, "head_on", "0" * 64, "1" * 64, evidence / "must-not-exist", lambda: None)
    assert not (evidence / "must-not-exist").exists()


def test_transitive_import_closure_is_accepted_before_import(monkeypatch):
    closure = C.verify_import_closure()
    for name in ("balanced_matching", "recorded_matching", "flux_binding", "recovery_credit_exchange_q2"):
        assert "scripts/phi_v2_lattice/" + name + ".py" in closure
    real = C.sha
    monkeypatch.setattr(C, "sha", lambda path: "0" * 64 if Path(path).name == "balanced_matching.py" else real(path))
    with pytest.raises(ValueError, match="transitive"):
        C.verify_import_closure()


def test_actual_metadata_sizes_and_deterministic_generated_admission(catalogue, exported, evidence, binary):
    directory = evidence / "metadata-size-control"
    directory.mkdir()
    ref = lambda path: {"path": str(Path(path).resolve()), "sha256": C.sha(path)}
    build = Path(os.environ.get("FTD_COMPOSITE_BUILD_RECEIPT", str(Path(binary).parent / "build.json")))
    readiness = EVIDENCE / "independent-native-q4-review/readiness.json"
    storage = directory / "storage.json"
    C.write_new(storage, C.canonical(C.disk_admission(directory, tuple(C.TIER_COUNTS))))
    specs = {key: ref(ROOT / "engine/docs" / filename) for key, filename in {
        "execution_v1": "SPEC_STRICT_COMPOSITE_INTERACTION_EXECUTION_V1.md",
        "execution_v2": "SPEC_STRICT_COMPOSITE_INTERACTION_EXECUTION_V2.md",
        "adapter_cli": "SPEC_STRICT_COMPOSITE_ADAPTER_CLI_V1.md", "kernel": "SPEC_STRICT_SPARSE_Q4_NATIVE_KERNEL_V1.md"}.items()}
    common = {"tier": "head_on", "N": 7296, "H": 304, "L": 64, "range_count": 64, "workers": 32,
              "deadline_seconds": 1800, "memory_bytes": 8 << 30, "source_sha256": C.source_inventory(),
              "build": ref(build), "readiness": ref(readiness), "preservation": ref(C.PRIOR_MANIFEST)}
    pins = dict(zip(("npz", "report", "audit", "q2_source", "dense_source"), (digest for _, digest in catalogue.pins)))
    lock = dict(common, schema="q4-composite-execution-lock-2", catalogue_npz=ref(catalogue.path), catalogue_pins=pins,
                specifications=specs, output=str(directory), storage={"admission": ref(storage), "remaining_tiers": list(C.TIER_COUNTS), "additional_copy_bytes": 0},
                sorting={"generations": 3, "fanin": 64, "record_cap": 2048, "chunk_records": 114,
                         "original_stream": "q4-original-checkpoint-ordered-controls-1", "normalized_stream": "q4-normalized-checkpoint-1"})
    lock_path = directory / "lock.json"
    C.write_new(lock_path, C.canonical(lock))
    assert C.validate_lock(lock_path.read_bytes(), lock_path, True, False) == lock
    with pytest.raises(ValueError):
        C.validate_lock(lock_path.read_bytes(), lock_path, True, True)
    actions = directory / "actions.bin"
    C.write_new(actions, C.actions_wire())
    # This tests metadata byte binding only. The support artifact is the exact
    # export if already supplied, otherwise a separate scoped fixture marker.
    templates = directory / "templates-marker.bin"
    C.write_new(templates, b"SCOPED_METADATA_ONLY_NOT_PRODUCTION_TEMPLATES")
    descriptor = dict(common, schema="q4-composite-input-descriptor-2", catalogue=dict(ref(exported[0]), pins=pins),
                      actions=ref(actions), templates=ref(templates))
    descriptor_path = directory / "descriptor.json"
    C.write_new(descriptor_path, C.canonical(descriptor))
    arguments = {"lock_sha256": C.sha(lock_path), "descriptor_path": descriptor_path, "descriptor_sha256": C.sha(descriptor_path),
                 "map_path": actions, "map_sha256": C.sha(actions), "catalogue_path": exported[0], "catalogue_sha256": exported[1]}
    admission = C.generated_admission(lock_path, **arguments)
    C.write_new(directory / "admission.json", admission)
    assert C.verify_generated_admission(lock_path, **arguments) == hashlib.sha256(admission).hexdigest()
    sizes = {"source_map": len(C.canonical(common["source_sha256"])), "lock": lock_path.stat().st_size,
             "descriptor": descriptor_path.stat().st_size, "admission": len(admission), "referenced_build_receipt": build.stat().st_size}
    assert max(sizes[key] for key in ("source_map", "lock", "descriptor", "admission")) < 65536
    C.write_new(directory / "sizes.json", C.canonical({"kind": "syntactic-metadata-size-only", "admitted_for_execution": False, "bytes": sizes}))
    bad = dict(descriptor, catalogue=dict(descriptor["catalogue"], path="C:\\invalid\npath"))
    with pytest.raises(ValueError):
        C.validate_descriptor(C.canonical(bad), False)
