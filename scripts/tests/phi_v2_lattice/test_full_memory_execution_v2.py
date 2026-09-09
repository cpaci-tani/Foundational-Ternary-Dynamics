"""V2 evidence/failure controls; no newly evaluated nonempty central range."""
import json
import os
from pathlib import Path
import subprocess
import sys
import time

import pytest

from phi_v2_lattice.experiments import certify_full_memory_v2 as V2

V1, M = V2.V1, V2.M
PINS = {"job_sha256": V2.JOB_HASH, "binary_sha256": V2.BINARY_HASH}


def row(start, stop, done, status="RUNNING"):
    return {"schema": "strict-full-memory-native-range-1", "start": start, "stop": stop,
            "completed_stop": done, "checked": done-start, "ordered_pairs": V1._ordered(start, done),
            "denominator_exponent": 432, "numerators": ["0"]*14, "status": status}


def log(path, rows, tail=b""):
    V2._write(path, b"".join(M.canonical_bytes(r) for r in rows)+tail)


def test_observed_replace_denial_does_not_affect_v2_progress(tmp_path, monkeypatch):
    target = tmp_path/"legacy.progress.json"
    V1._json(target, {"first": True})
    original = os.replace
    def deny_existing(source, destination):
        if Path(destination).exists():
            raise PermissionError(5, "synthetic WinError5 live replacement denial")
        return original(source, destination)
    monkeypatch.setattr(os, "replace", deny_existing)
    with pytest.raises(PermissionError):
        V1._json(target, {"first": False}, replace=True)
    assert V1._read(target) == {"first": True}
    assert list(tmp_path.glob("legacy.progress.json.*.tmp"))
    for sequence in range(80):
        V2.append_progress(tmp_path, 0, sequence, row(0, 100, sequence), PINS)
    paths = sorted((tmp_path/"ranges/00000000/progress").glob("*.json"))
    assert len(paths) == 80
    assert [V1._read(p)["native"]["checked"] for p in paths] == list(range(80))


def test_all_v2_receipt_creation_avoids_replace_and_refuses_duplicates(tmp_path, monkeypatch):
    def forbidden(*args):
        raise AssertionError("V2 must not call replace")
    monkeypatch.setattr(os, "replace", forbidden)
    p = V2.append_progress(tmp_path, 0, 0, row(0, 10, 0), PINS)
    before = p.read_bytes()
    with pytest.raises(FileExistsError):
        V2.append_progress(tmp_path, 0, 0, row(0, 10, 1), PINS)
    assert p.read_bytes() == before
    result = V2.complete_receipts(tmp_path, "synthetic unstarted", PINS)
    assert len(result["range_receipts"]) == 128 and len(result["control_receipts"]) == 255


@pytest.mark.parametrize("tail", (b'{"partial":', b"broken\n", b"\n", b"{}\n"))
def test_stdout_recovery_stops_at_first_bad_tail(tmp_path, tail):
    first = row(0, 100, 0)
    next_row = row(0, 100, 7)
    good = M.canonical_bytes(first)+M.canonical_bytes(next_row)
    raw = good+tail+M.canonical_bytes(row(0, 100, 100, "PASS"))
    path = tmp_path/"native.stdout"
    V2._write(path, raw)
    result = V2.recover_stdout(path, 0, 100)
    assert result["native"] == next_row and result["prefix_lines"] == 2
    assert result["prefix_bytes"] == len(good) and result["tail_error"]
    assert result["stdout_sha256"] == M.sha256(raw) and path.read_bytes() == raw


def test_no_newline_cannot_be_a_durable_native_record(tmp_path):
    path = tmp_path/"native.stdout"
    V2._write(path, M.canonical_bytes(row(0, 10, 10, "PASS")).rstrip(b"\n"))
    out = V2.recover_stdout(path, 0, 10)
    assert out["native"] is None and out["prefix_bytes"] == 0 and out["tail_error"]


@pytest.mark.parametrize("kind", ("regression", "foreign_range", "wrong_count", "post_terminal", "duplicate_key"))
def test_stdout_recovery_rejects_invalid_native_prefix_extension(tmp_path, kind):
    first = row(0, 100, 5)
    second = row(0, 100, 6)
    if kind == "regression": second = row(0, 100, 4)
    if kind == "foreign_range": second["start"] = 1
    if kind == "wrong_count": second["checked"] = 55
    if kind == "post_terminal": first = row(0, 100, 100, "PASS")
    tail = M.canonical_bytes(second)
    if kind == "duplicate_key": tail = tail.replace(b'{', b'{"checked":999,', 1)
    path = tmp_path/"native.stdout"
    log(path, [first], tail)
    out = V2.recover_stdout(path, 0, 100)
    assert out["native"] == first and out["prefix_lines"] == 1 and out["tail_error"]


def test_native_pass_without_normal_finalization_remains_failed(tmp_path):
    start, stop = M.registered_ranges()[0]
    native = row(start, stop, stop, "PASS")
    log(tmp_path/"ranges/00000000.stdout", [row(start, stop, 0), native])
    V2._write(tmp_path/"ranges/00000000.stderr", b"kept diagnostic")
    maps = V2.complete_receipts(tmp_path, "coordinator killed before finalization", PINS)
    result = V1._read(tmp_path/maps["range_receipts"]["00000000"])
    assert result["status"] == "FAIL" and result["native"]["status"] == "PASS"
    assert result["checked"] == stop and result["recovery"]["prefix_lines"] == 2
    assert len(maps["range_receipts"]) == 128 and len(maps["control_receipts"]) == 255
    before = {p.relative_to(tmp_path).as_posix(): V1._sha(p) for p in tmp_path.rglob("*") if p.is_file()}
    assert V2.complete_receipts(tmp_path, "second recovery", PINS) == maps
    assert before == {p.relative_to(tmp_path).as_posix(): V1._sha(p) for p in tmp_path.rglob("*") if p.is_file()}


@pytest.mark.parametrize("normal", ("partial", "missing_exit", "wrong_job", "stale_fail", "wrong_native", "bad_stdout_tail"))
def test_invalid_or_stale_normal_final_is_retained_with_separate_recovery(tmp_path, normal):
    start, stop = M.registered_ranges()[0]
    native = row(start, stop, stop, "PASS")
    path = tmp_path/"ranges/00000000.json"
    log(path.with_suffix(".stdout"), [row(start, stop, 0), native], b"bad tail\n" if normal == "bad_stdout_tail" else b"")
    V2._write(path.with_suffix(".stderr"), b"")
    final = {"schema": "strict-full-memory-range-receipt-1", "status": "PASS", "start": start, "stop": stop,
             "checked": stop, "native": native, **PINS, "exit_code": 0, "native_eof": True,
             "stdout_sha256": V1._sha(path.with_suffix(".stdout")), "stderr_sha256": V1._sha(path.with_suffix(".stderr"))}
    if normal == "partial": V2._write(path, b'{"unfinished":')
    else:
        if normal == "missing_exit": del final["exit_code"]
        if normal == "wrong_job": final["job_sha256"] = "0"*64
        if normal == "wrong_native": final["native"] = row(start, stop, 7)
        if normal == "stale_fail": final.update(status="FAIL", native=row(start, stop, 3), checked=3)
        V2._json(path, final)
    digest = V1._sha(path)
    maps = V2.complete_receipts(tmp_path, "synthetic interrupted final", PINS)
    selected = maps["range_receipts"]["00000000"]
    assert selected == "ranges/00000000.recovery.json" and V1._sha(path) == digest
    result = V1._read(tmp_path/selected)
    assert result["status"] == "FAIL" and result["checked"] == stop
    assert result["invalid_normal_receipt"]["sha256"] == digest
    assert V2.complete_receipts(tmp_path, "idempotent", PINS) == maps


def test_valid_normally_finalized_pass_is_never_replaced(tmp_path):
    start, stop = M.registered_ranges()[0]
    native = row(start, stop, stop, "PASS")
    path = tmp_path/"ranges/00000000.json"
    log(path.with_suffix(".stdout"), [row(start, stop, 0), native])
    V2._write(path.with_suffix(".stderr"), b"")
    V2._json(path, {"schema": "strict-full-memory-range-receipt-1", "status": "PASS", "start": start,
        "stop": stop, "checked": stop, "native": native, **PINS, "exit_code": 0, "native_eof": True,
        "stdout_sha256": V1._sha(path.with_suffix(".stdout")), "stderr_sha256": V1._sha(path.with_suffix(".stderr"))})
    digest = V1._sha(path)
    maps = V2.complete_receipts(tmp_path, "other range failed", PINS)
    assert maps["range_receipts"]["00000000"] == "ranges/00000000.json"
    assert V1._sha(path) == digest and not path.with_suffix(".recovery.json").exists()


def test_all_original_failed_attempt_bytes_and_longer_native_prefixes_are_retained():
    report, paths = V2.prior_attempt()
    assert len(report["artifact_sha256"]) == 1489 and len(paths) >= 1490
    checked = started = 0
    for start, stop in M.registered_ranges():
        path = V2.PRIOR_REPORT.parent/f"ranges/{start:08d}.stdout"
        result = V2.recover_stdout(path, start, stop)
        if result["native"] is not None:
            started += 1
            checked += result["native"]["checked"]
            assert result["native"]["status"] == "PASS" and result["tail_error"] is None
    assert started == 36 and checked == 9692654
    assert sum(V1._read(V2.PRIOR_REPORT.parent/f"ranges/{s:08d}.json")["checked"] for s, _ in M.registered_ranges()) == 1982177
    assert V1._sha(V2.PRIOR_REPORT) == V2.PRIOR_HASH


def _synthetic_progress_tree(payload, barrier):
    barrier.wait()
    directory = Path(payload["output"])
    child = subprocess.Popen([sys.executable, "-c", "import time; time.sleep(30)"], creationflags=0x08000000)
    (directory/"child.pid").write_text(str(child.pid))
    start, stop = M.registered_ranges()[0]
    native = row(start, stop, 17)
    log(directory/"ranges/00000000.stdout", [row(start, stop, 0), native])
    V2.append_progress(directory, start, 0, native, PINS)
    V2._write(directory/"ranges/00000000.json", b'{"partial":')
    time.sleep(30)


@pytest.mark.skipif(os.name != "nt", reason="actual registered Windows supervisor")
def test_actual_kill_preserves_append_only_partial_evidence(tmp_path):
    result = V1.supervise(_synthetic_progress_tree, {"output": str(tmp_path)}, 3, V1.MEMORY_LIMIT)
    assert result["actual_job_assignment"] and result["failure"] == "global monotonic deadline"
    assert not result["coordinator_alive_after_cleanup"] and (tmp_path/"child.pid").exists()
    maps = V2.complete_receipts(tmp_path, result["failure"], PINS)
    final = V1._read(tmp_path/maps["range_receipts"]["00000000"])
    assert final["status"] == "FAIL" and final["checked"] == 17
    assert (tmp_path/"ranges/00000000.json").read_bytes() == b'{"partial":'
    assert V1._read(tmp_path/"ranges/00000000/progress/000000.json")["native"]["checked"] == 17


@pytest.fixture
def synthetic_admission(tmp_path):
    prepared, build = V2.EVIDENCE/"full-memory-prepared-1", V2.EVIDENCE/"full-memory-build-1"
    binary = build/"full_memory_native_v1.exe"
    old = V2.EVIDENCE/"full-memory-execution-acceptance-v1.json"
    census = V2.EVIDENCE/"local-tables-attempt-1/report.json"
    base = V1.accepted_inputs(prepared, binary, V2.BINARY_HASH, old, V2.V1_ACCEPTANCE_HASH, census, V1._sha(census))
    prior = V1._read(V2.PRIOR_REPORT)
    sources = {**base["source_sha256"], **prior["source_sha256"], **{name: V1._sha(V2.ROOT/name) for name in V2.OWNED}}
    audit = "engine/docs/AUDIT_STRICT_FULL_MEMORY_EXECUTION_V1.md"
    receipt = {"schema": "strict-full-memory-execution-v2-acceptance-1", "law_id": M.LAW_ID,
        "verdict": "PASS_SCOPED_EXACT_EXECUTION", "canonical_adoption": False,
        "source_sha256": sources, "audit_path": audit, "audit_sha256": V1._sha(V2.ROOT/audit),
        "gates": {key: "PASS" for key in V2.GATES}, "binary_sha256": V2.BINARY_HASH,
        "job_sha256": V2.JOB_HASH, "analytic_data_sha256": base["analytic_data_sha256"],
        "independent_census_sha256": V1._sha(census), "v1_acceptance_sha256": V2.V1_ACCEPTANCE_HASH,
        "prior_attempt_report_sha256": V2.PRIOR_HASH}
    path = tmp_path/"synthetic-v2-acceptance.json"
    args = dict(prepared=prepared, binary=binary, binary_hash=V2.BINARY_HASH, v1_acceptance=old,
                v1_acceptance_hash=V2.V1_ACCEPTANCE_HASH, acceptance=path, census=census, census_hash=V1._sha(census))
    return receipt, args


def test_v2_admission_preserves_old_failure_and_all_accepted_prerequisites(synthetic_admission):
    receipt, args = synthetic_admission
    V2._json(args["acceptance"], receipt)
    pins = V2.accepted_inputs(**args, acceptance_hash=V1._sha(args["acceptance"]))
    assert set(V2.OWNED) <= set(pins["source_sha256"])
    assert pins["path_sha256"][str(V2.PRIOR_REPORT)] == V2.PRIOR_HASH
    assert V1._identity_outcome(pins)["status"] == "PASS"
    assert all(str((V2.PRIOR_REPORT.parent/name).resolve()) in pins["path_sha256"]
               for name in V1._read(V2.PRIOR_REPORT)["artifact_sha256"])


@pytest.mark.parametrize("corruption", ("missing_source", "stale_audit", "wrong_binary", "wrong_job", "wrong_prior", "missing_gate"))
def test_v2_admission_rejects_incomplete_or_stale_scope(synthetic_admission, corruption):
    receipt, args = synthetic_admission
    if corruption == "missing_source": receipt["source_sha256"].pop(V2.OWNED[0])
    if corruption == "stale_audit": receipt["audit_sha256"] = "0"*64
    if corruption == "wrong_binary": receipt["binary_sha256"] = "0"*64
    if corruption == "wrong_job": receipt["job_sha256"] = "0"*64
    if corruption == "wrong_prior": receipt["prior_attempt_report_sha256"] = "0"*64
    if corruption == "missing_gate": receipt["gates"].pop("append_only_recovery")
    V2._json(args["acceptance"], receipt)
    with pytest.raises(ValueError):
        V2.accepted_inputs(**args, acceptance_hash=V1._sha(args["acceptance"]))


def test_missing_v2_acceptance_cannot_start_scientific_work(tmp_path, synthetic_admission, monkeypatch):
    _, args = synthetic_admission
    def forbidden(*args, **kwargs):
        raise AssertionError("scientific work before admission")
    monkeypatch.setattr(M, "central_reference_range", forbidden)
    monkeypatch.setattr(V1, "supervise", forbidden)
    out = tmp_path/"rejection"
    with pytest.raises(FileNotFoundError):
        V2.run(**args, acceptance_hash="0"*64, output=out)
    assert V1._read(out/"gate-rejection.json")["central_pairs_evaluated"] == 0
    assert not (out/"lock.json").exists()


def test_outer_failure_retains_terminal_maps_identity_and_inventory(tmp_path, monkeypatch):
    pins = {**PINS, "source_sha256": {}, "path_sha256": {}}
    monkeypatch.setattr(V2, "accepted_inputs", lambda *args: pins)
    def fail(*args):
        raise OSError("synthetic supervisor failure")
    monkeypatch.setattr(V1, "supervise", fail)
    out = tmp_path/"failed"
    result = V2.run(*(["unused"]*9), output=out)
    assert result["status"] == "FULL_K1_INCOMPLETE" and result["post_run_identity"]["status"] == "PASS"
    assert result["supervision"] is None and "synthetic supervisor failure" in result["error"]
    assert len(result["range_receipts"]) == 128 and len(result["control_receipts"]) == 255
    assert all(name in result["artifact_sha256"] for key in ("range_receipts", "control_receipts") for name in result[key].values())


def test_all_255_control_orientations_and_exact_comparisons_without_real_pairs(tmp_path, monkeypatch):
    called = []
    def reference(job, start, stop):
        called.append((start, stop))
        return {key: value for key, value in row(start, stop, stop, "PASS").items() if key not in ("schema", "status")}
    def native(command, **kwargs):
        start, stop = int(command[2]), int(command[4])
        kwargs["stdout"].write(M.canonical_bytes(row(start, stop, start))+M.canonical_bytes(row(start, stop, stop, "PASS")))
        return subprocess.CompletedProcess(command, 0)
    monkeypatch.setattr(M, "central_reference_range", reference)
    monkeypatch.setattr(V2.subprocess, "run", native)
    payload = {**PINS, "output": str(tmp_path), "binary": "synthetic-only", "deadline": time.monotonic()+60}
    V2.controls(b"synthetic", payload)
    assert tuple(called) == V1._control_cases() and len(called) == 255
    maps = V2.complete_receipts(tmp_path, "unstarted production", PINS)
    assert all(V1._read(tmp_path/name)["status"] == "PASS" for name in maps["control_receipts"].values())
    assert all(V1._read(tmp_path/name)["status"] == "FAIL" for name in maps["range_receipts"].values())


def test_control_timeout_recovers_output_without_overwriting_failed_final(tmp_path, monkeypatch):
    def reference(job, start, stop):
        return {key: value for key, value in row(start, stop, stop, "PASS").items() if key not in ("schema", "status")}
    def native(command, **kwargs):
        kwargs["stdout"].write(M.canonical_bytes(row(0, 16, 3)))
        kwargs["stderr"].write(b"synthetic timeout diagnostic")
        raise subprocess.TimeoutExpired(command, 1)
    monkeypatch.setattr(M, "central_reference_range", reference)
    monkeypatch.setattr(V2.subprocess, "run", native)
    payload = {**PINS, "output": str(tmp_path), "binary": "synthetic-only", "deadline": time.monotonic()+60}
    with pytest.raises(subprocess.TimeoutExpired):
        V2.controls(b"synthetic", payload)
    old = tmp_path/"controls/0000.json"
    digest = V1._sha(old)
    maps = V2.complete_receipts(tmp_path, "timeout", PINS)
    assert maps["control_receipts"]["0000"] == "controls/0000.recovery.json"
    recovered = V1._read(tmp_path/maps["control_receipts"]["0000"])
    assert recovered["status"] == "FAIL" and recovered["native"]["checked"] == 3
    assert V1._sha(old) == digest and recovered["expected"]["checked"] == 16


@pytest.mark.parametrize("corruption", ("missing_expected", "wrong_numerator", "short_numerators", "extra_key",
    "boolean_count", "missing_file", "wrong_file_content", "wrong_file_hash"))
def test_control_pass_requires_exact_expected_record_and_file(tmp_path, corruption):
    start, stop = V1._control_cases()[0]
    native = row(start, stop, stop, "PASS")
    expected = {key: value for key, value in native.items() if key not in ("schema", "status")}
    path = tmp_path/"controls/0000.json"
    log(path.with_suffix(".stdout"), [row(start, stop, start), native])
    V2._write(path.with_suffix(".stderr"), b"")
    reference_path = path.with_suffix(".expected.json")
    file_record = dict(expected)
    if corruption == "wrong_file_content": file_record["numerators"] = ["1"]+["0"]*13
    if corruption != "missing_file": V2._json(reference_path, file_record)
    final = {"schema": "strict-full-memory-control-receipt-1", "status": "PASS", "start": start, "stop": stop,
        "native": native, "expected": dict(expected), "exit_code": 0, **PINS,
        "expected_sha256": V1._sha(reference_path) if reference_path.exists() else "0"*64,
        "stdout_sha256": V1._sha(path.with_suffix(".stdout")), "stderr_sha256": V1._sha(path.with_suffix(".stderr"))}
    if corruption == "missing_expected": del final["expected"]
    if corruption == "wrong_numerator": final["expected"]["numerators"] = ["1"]+["0"]*13
    if corruption == "short_numerators": final["expected"]["numerators"] = ["0"]*13
    if corruption == "extra_key": final["expected"]["foreign"] = 0
    if corruption == "boolean_count": final["expected"]["checked"] = True
    if corruption == "wrong_file_hash": final["expected_sha256"] = "0"*64
    V2._json(path, final)
    digest = V1._sha(path)
    maps = V2.complete_receipts(tmp_path, "invalid control reference", PINS)
    assert maps["control_receipts"]["0000"] == "controls/0000.recovery.json"
    recovered = V1._read(tmp_path/maps["control_receipts"]["0000"])
    assert recovered["status"] == "FAIL" and recovered["native"]["status"] == "PASS"
    assert V1._sha(path) == digest
