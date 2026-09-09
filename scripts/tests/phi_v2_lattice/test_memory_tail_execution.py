"""Synthetic transport/failure controls; no accepted scientific input is read."""
import json
import os
from pathlib import Path
import sys
import time
import py_compile
from types import SimpleNamespace

import pytest

from phi_v2_lattice import recovery_memory_tail as M
from phi_v2_lattice.experiments import certify_memory_tail as R


BINDING = {"law_id": M.LAW_ID, "prepared_sha256": "1"*64, "source_identity_sha256": "2"*64}


def record(index, binding=BINDING):
    return {"schema": "strict-memory-tail-coefficient-1", **binding, "index": index,
            "displacement": list(M.SUPPORT[index]), "denominator_exponent": 864,
            "numerators": [["0"]*24 for _ in range(24)], "independent_equal": True,
            "counters": M.counters(index)}


def raw_range(number, binding=BINDING):
    start, stop = M.RANGES[number]
    return b"".join(M.canonical_bytes(record(i, binding)) for i in range(start, stop))+M.canonical_bytes(R._terminal(start, stop, binding))


def fixture_range(directory, number=0, normal=True):
    start, stop = M.RANGES[number]
    folder = directory/"ranges"/f"{number:04d}"
    folder.mkdir(parents=True)
    R._json(folder/"started.json", R._started(number, BINDING))
    R._write(folder/"stdout", raw_range(number))
    R._write(folder/"stderr", b"")
    if normal:
        prefix = R.recover_stdout(folder/"stdout", start, stop, BINDING)
        for local, meta in enumerate(prefix["records"]):
            name = f"coefficients/{meta['record']['index']:04d}.json"
            R._json(directory/name, meta["record"])
            R._json(folder/"progress"/f"{local:04d}.json", R._progress(meta, BINDING, name, R._sha(directory/name)))
        final = R._normal_final(directory, number, BINDING, SimpleNamespace(returncode=0), ("stdout", "stderr"))
        R._json(folder/"final.json", final)
    return folder


def test_writes_are_exclusive_and_never_replace(tmp_path, monkeypatch):
    def forbidden(*args):
        raise AssertionError("replacement forbidden")
    monkeypatch.setattr(os, "replace", forbidden)
    path = tmp_path/"progress"/"0000.json"
    R._json(path, {"a": 1})
    old = path.read_bytes()
    with pytest.raises(FileExistsError):
        R._json(path, {"a": 2})
    assert path.read_bytes() == old


def test_valid_raw_prefix_and_normal_range(tmp_path):
    folder = fixture_range(tmp_path)
    start, stop = M.RANGES[0]
    prefix = R.recover_stdout(folder/"stdout", start, stop, BINDING)
    assert prefix["completed_stop"] == stop and prefix["terminal"]["status"] == "PASS"
    assert prefix["valid_prefix_bytes"] == (folder/"stdout").stat().st_size
    assert R._valid_final(tmp_path, 0, BINDING)


@pytest.mark.parametrize("bad", ["truncated", "duplicate", "identity", "counter", "float", "oversized", "late"])
def test_longest_prefix_never_skips_malformed_bytes(tmp_path, bad):
    start, stop = M.RANGES[0]
    first = M.canonical_bytes(record(start))
    second = record(start+1)
    if bad == "duplicate": second["index"] = start
    elif bad == "identity": second["prepared_sha256"] = "9"*64
    elif bad == "counter": second["counters"]["site_pairs"] += 1
    elif bad == "float": second["numerators"][0][0] = 1.0
    malformed = M.canonical_bytes(second)
    if bad == "truncated": malformed = malformed[:-1]
    elif bad == "oversized": malformed = b"x"*(R.MAX_LINE+1)
    if bad == "late":
        raw = raw_range(0)+b"{}\n"
    else:
        raw = first+malformed+(M.canonical_bytes(record(start+1)) if bad not in ("truncated", "oversized") else b"")
    path = tmp_path/"stdout"
    R._write(path, raw)
    prefix = R.recover_stdout(path, start, stop, BINDING)
    assert prefix["tail_error"]
    assert prefix["completed_stop"] == (stop if bad == "late" else start+1)
    assert path.read_bytes() == raw


@pytest.mark.parametrize("damage", ["checked", "exit", "eof", "coefficient", "progress", "terminal", "identity"])
def test_normal_pass_must_match_every_durable_fact(tmp_path, damage):
    folder = fixture_range(tmp_path)
    final = R._read(folder/"final.json")
    if damage == "checked": final["checked"] = 999
    elif damage == "exit": final["exit_code"] = 1
    elif damage == "eof": final["stdout_eof"] = False
    elif damage == "terminal": final["terminal"]["checked"] = 999
    elif damage == "identity": final["prepared_sha256"] = "f"*64
    elif damage == "coefficient": final["coefficient_sha256"] = {}
    else: final["progress_sha256"] = {}
    # Deliberate corruption of synthetic fixture bytes; no evidence source.
    (folder/"final.json").write_bytes(M.canonical_bytes(final))
    assert not R._valid_final(tmp_path, 0, BINDING)
    before = (folder/"final.json").read_bytes()
    mapping = R.complete_receipts(tmp_path, BINDING, "synthetic corrupted final")
    assert len(mapping) == 64
    assert R._read(tmp_path/mapping["0000"])["status"] == "FAIL"
    assert (folder/"final.json").read_bytes() == before


def test_full_raw_pass_without_normal_finalization_remains_failure(tmp_path):
    folder = fixture_range(tmp_path, normal=False)
    mapping = R.complete_receipts(tmp_path, BINDING, "synthetic interruption")
    row = R._read(tmp_path/mapping["0000"])
    assert row["status"] == "FAIL" and row["completed_stop"] == M.RANGES[0][1]
    assert row["worker_terminal"]["status"] == "PASS" and not row["normal_finalization"]
    hashes = {p.relative_to(tmp_path).as_posix(): R._sha(p) for p in tmp_path.rglob("*") if p.is_file()}
    assert R.complete_receipts(tmp_path, BINDING, "different later explanation") == mapping
    assert {p.relative_to(tmp_path).as_posix(): R._sha(p) for p in tmp_path.rglob("*") if p.is_file()} == hashes
    assert (folder/"stdout").read_bytes() == raw_range(0)


def test_all_unstarted_ranges_get_terminal_receipts(tmp_path):
    mapping = R.complete_receipts(tmp_path, BINDING, "never started")
    assert set(mapping) == {f"{i:04d}" for i in range(64)}
    assert all(R._read(tmp_path/p)["checked"] == 0 for p in mapping.values())


@pytest.mark.parametrize("failure", ["stdout", "directory", "fallback_write"])
def test_one_unavailable_range_does_not_suppress63_other_dispositions(tmp_path, monkeypatch, failure):
    folder = fixture_range(tmp_path, normal=False)
    raw = (folder/"stdout").read_bytes()
    original_open, original_mkdir, original_write = Path.open, Path.mkdir, R._write
    def unreadable(path, *args, **kwargs):
        if path == folder/"stdout":
            raise PermissionError(5, "synthetic unreadable stdout")
        return original_open(path, *args, **kwargs)
    def denied_directory(path, *args, **kwargs):
        if path == folder:
            raise PermissionError(5, "synthetic range directory denial")
        return original_mkdir(path, *args, **kwargs)
    def denied_fallback(path, data):
        if "range-unavailable" in Path(path).parts:
            raise PermissionError(5, "synthetic fallback receipt denial")
        return original_write(path, data)
    if failure == "directory":
        monkeypatch.setattr(Path, "mkdir", denied_directory)
    else:
        monkeypatch.setattr(Path, "open", unreadable)
    if failure == "fallback_write":
        monkeypatch.setattr(R, "_write", denied_fallback)
    mapping = R.complete_receipts(tmp_path, BINDING, "range-local failure")
    assert set(mapping) == {f"{i:04d}" for i in range(64)}
    row = mapping["0000"] if failure == "fallback_write" else R._read(tmp_path/mapping["0000"])
    assert row["status"] == "FAIL" and row["work_count_status"] == "UNAVAILABLE"
    assert row["completed_stop"] is None and row["checked"] is None and row["counters"] is None
    assert "denial" in row["recovery_error"] or "unreadable" in row["recovery_error"]
    if failure == "fallback_write":
        assert "fallback receipt denial" in row["receipt_write_error"]
    assert all(R._read(tmp_path/mapping[f"{i:04d}"])["checked"] == 0 for i in range(1, 64))
    assert not R._all_ranges_pass(tmp_path, mapping)
    with original_open(folder/"stdout", "rb") as stream:
        assert stream.read() == raw
    if failure != "fallback_write":
        assert R.complete_receipts(tmp_path, BINDING, "later explanation") == mapping


def test_inline_unavailable_range_is_retained_in_durable_failed_report(tmp_path, monkeypatch):
    monkeypatch.setattr(R, "ROOT", tmp_path)
    R._json(tmp_path/R.BASELINE, {"prior_seals": []})
    monkeypatch.setattr(R, "_source_pins", lambda: {})
    def denied(*args):
        raise ValueError("synthetic rejected admission")
    monkeypatch.setattr(R, "accepted_inputs", denied)
    original_mkdir, original_write = Path.mkdir, R._write
    output = tmp_path/"attempt"
    def denied_directory(path, *args, **kwargs):
        if path == output/"ranges"/"0000":
            raise PermissionError(5, "synthetic range directory denial")
        return original_mkdir(path, *args, **kwargs)
    def denied_fallback(path, data):
        if "range-unavailable" in Path(path).parts:
            raise PermissionError(5, "synthetic fallback receipt denial")
        return original_write(path, data)
    monkeypatch.setattr(Path, "mkdir", denied_directory)
    monkeypatch.setattr(R, "_write", denied_fallback)
    report = R.run(tmp_path/"prepared", tmp_path/"acceptance", "1"*64, output)
    assert report == R._read(output/"report.json")
    assert report["status"] == "MEMORY_TAIL_INCOMPLETE"
    assert len(report["range_receipts"]) == 64
    assert report["range_receipts"]["0000"]["checked"] is None
    assert report["range_receipts"]["0000"]["status"] == "FAIL"
    assert all(type(report["range_receipts"][f"{i:04d}"]) is str for i in range(1, 64))


def test_malformed_recovery_is_retained_with_distinct_new_receipt(tmp_path):
    folder = fixture_range(tmp_path, normal=False)
    R._write(folder/"recovery.json", b'{"partial":')
    mapping = R.complete_receipts(tmp_path, BINDING, "fixture")
    assert mapping["0000"].endswith("recovery-0001.json")
    assert (folder/"recovery.json").read_bytes() == b'{"partial":'


def fake_pool(tmp_path, monkeypatch):
    payload = {"deadline": time.monotonic()+60, "source_sha256": {}, "path_sha256": {}, "prepared_sha256": "1"*64}
    binding = R._binding(payload)
    data = {str(a): raw_range(i, binding).decode("ascii") for i, (a, b) in enumerate(M.RANGES)}
    R._json(tmp_path/"transport-fixture.json", data)
    script = tmp_path/"synthetic-worker.py"
    script.write_text("import json,sys\nfrom pathlib import Path\nx=json.loads(Path(sys.argv[1]).read_text())\nsys.stdout.buffer.write(x[sys.argv[2]].encode('ascii'))\nsys.stdout.buffer.flush()\n", encoding="utf-8")
    monkeypatch.setattr(R, "_worker_command", lambda directory, a, b: [sys.executable, "-B", str(script), str(tmp_path/"transport-fixture.json"), str(a)])
    return payload, binding


def test_actual64_range_transport_uses_unique_progress_and_complete_outputs(tmp_path, monkeypatch):
    payload, binding = fake_pool(tmp_path, monkeypatch)
    R._pool(payload, tmp_path)
    mapping = R.complete_receipts(tmp_path, binding, "synthetic complete pool")
    assert len(mapping) == 64
    assert all(R._read(tmp_path/path)["status"] == "PASS" for path in mapping.values())
    assert len(list((tmp_path/"coefficients").glob("*.json"))) == 185
    assert len(list((tmp_path/"ranges").glob("*/progress/*.json"))) == 185
    assert sum(R._read(tmp_path/path)["checked"] for path in mapping.values()) == 185
    assert sum(R._read(tmp_path/path)["counters"]["primary_products"] for path in mapping.values()) == 15054336


def test_filesystem_denial_preserves_raw_completed_prefix(tmp_path, monkeypatch):
    payload, binding = fake_pool(tmp_path, monkeypatch)
    original = R._write
    def deny_progress(path, data):
        if "progress" in Path(path).parts:
            raise PermissionError(5, "synthetic observed Windows denial")
        return original(path, data)
    monkeypatch.setattr(R, "_write", deny_progress)
    with pytest.raises(PermissionError):
        R._pool(payload, tmp_path)
    mapping = R.complete_receipts(tmp_path, binding, "filesystem denial")
    assert len(mapping) == 64
    assert all(R._read(tmp_path/path)["status"] == "FAIL" for path in mapping.values())
    assert sum(R._read(tmp_path/path)["checked"] for path in mapping.values()) >= 1


def _sleep_target(payload, barrier):
    barrier.wait()
    time.sleep(10)


def _memory_target(payload, barrier):
    barrier.wait()
    allocation = bytearray(512 << 20)
    time.sleep(10)
    assert allocation[0] == 0


@pytest.mark.skipif(sys.platform != "win32", reason="actual Windows Job Object gate")
def test_actual_job_deadline_kills_owned_child():
    result = R.SUPERVISOR.supervise(_sleep_target, {}, 0.8, R.MEMORY)
    assert result["actual_job_assignment"] and result["failure"]
    assert not result["coordinator_alive_after_cleanup"]
    assert result["peak_aggregate_private_bytes"] > 0
    assert result["elapsed_seconds"] < 8


@pytest.mark.skipif(sys.platform != "win32", reason="actual Windows Job Object gate")
def test_failed_assignment_does_not_leave_unassigned_child(monkeypatch):
    original = R.SUPERVISOR.WinJob
    class Denied(original):
        def assign(self, pid):
            raise OSError("synthetic Job assignment denial")
    monkeypatch.setattr(R.SUPERVISOR, "WinJob", Denied)
    result = R.SUPERVISOR.supervise(_sleep_target, {}, 5, R.MEMORY)
    assert not result["actual_job_assignment"]
    assert "assignment denial" in result["failure"]
    assert not result["coordinator_alive_after_cleanup"]


@pytest.mark.skipif(sys.platform != "win32", reason="actual Windows Job Object gate")
def test_actual_job_memory_limit_includes_parent_and_stops_allocation():
    limit = R.SUPERVISOR._parent_memory()+(96 << 20)
    result = R.SUPERVISOR.supervise(_memory_target, {}, 8, limit)
    assert result["actual_job_assignment"]
    assert result["exit_code"] != 0
    assert result["failure"] != "global monotonic deadline"
    assert result["memory_limit_bytes"] == limit
    assert result["peak_aggregate_private_bytes"] > 0
    assert not result["coordinator_alive_after_cleanup"]


def test_direct_identity_change_is_not_silently_rebased(tmp_path, monkeypatch):
    monkeypatch.setattr(R, "ROOT", tmp_path)
    (tmp_path/"x").write_bytes(b"original")
    pins = {"x": R._sha(tmp_path/"x")}
    (tmp_path/"x").write_bytes(b"changed")
    with pytest.raises(ValueError, match="changed"):
        R._check_pins(pins)


def test_admission_failure_report_keeps64_receipts_and_identity_maps(tmp_path, monkeypatch):
    monkeypatch.setattr(R, "ROOT", tmp_path)
    baseline = tmp_path/R.BASELINE
    baseline.parent.mkdir(parents=True)
    R._json(baseline, {"prior_seals": [{"fixture": "no scientific evidence"}]})
    monkeypatch.setattr(R, "_source_pins", lambda: {"fake-source": "a"*64})
    def denied(*args):
        raise ValueError("synthetic rejected admission")
    monkeypatch.setattr(R, "accepted_inputs", denied)
    monkeypatch.setattr(R, "_identity", lambda payload: {"status": "FAIL", "checked_direct_identities": 1})
    monkeypatch.setattr(R.PRESERVE, "verify_preservation", lambda *args: {"status": "PASS", "synthetic": True})
    output = tmp_path/"attempt"
    report = R.run(tmp_path/"prepared", tmp_path/"acceptance", "1"*64, output)
    assert report["status"] == "MEMORY_TAIL_INCOMPLETE"
    assert report["source_sha256"] == {"fake-source": "a"*64}
    assert "rejected admission" in report["error"]
    assert len(report["range_receipts"]) == 64 and report["artifact_sha256"]
    assert report["supervision"] is None


def test_private_worker_does_not_accept_an_unadmitted_payload(monkeypatch):
    def reject(*args):
        raise ValueError("synthetic missing readiness")
    monkeypatch.setattr(R, "accepted_inputs", reject)
    with pytest.raises(ValueError, match="missing readiness"):
        R._worker({"prepared": "p/prepared.json", "acceptance": "a.json", "acceptance_sha256": "1"*64}, 0, 2)


def admission_fixture(tmp_path, monkeypatch):
    monkeypatch.setattr(R, "ROOT", tmp_path)
    for path, data in ((tmp_path/"source.py", b"synthetic source"), (tmp_path/"spec.md", b"synthetic spec"),
                       (tmp_path/"audit.md", b"synthetic independent review"), (tmp_path/"extra.py", b"extra accepted input")):
        path.write_bytes(data)
    sources = {"source.py": R._sha(tmp_path/"source.py"), "spec.md": R._sha(tmp_path/"spec.md")}
    monkeypatch.setattr(R, "_source_pins", lambda: dict(sources))
    pins = {R.OWNED[-1]: sources["spec.md"], R.CERTIFICATE: "c"*64, R.CENSUS: "d"*64}
    monkeypatch.setattr(R, "PINNED", pins)
    seals = [{"manifest": "synthetic.json", "sha256": "f"*64, "source_count": 1, "artifact_count": 1}]
    R._json(tmp_path/R.BASELINE, {"prior_seals": seals})
    zero = tuple((0,)*24 for _ in range(24))
    j = tuple(tuple((1 << 24) if a == i else 0 for i in range(24)) for a in range(24))
    data = M.Prepared(M.LABELS, j, tuple((d, zero) for d in M.DELTA_SUPPORT), "0/1", ("0/1",)*24).mapping()
    prepared = tmp_path/"prepared"
    R._json(prepared/"prepared.json", data)
    data_hash = R._sha(prepared/"prepared.json")
    receipt = {"schema": "strict-memory-tail-prepare-receipt-1", "law_id": M.LAW_ID,
               "prepared_sha256": data_hash, "source_sha256": sources,
               "certificate_sha256": "c"*64, "census_sha256": "d"*64,
               "numerical_predicates_evaluated": False}
    R._json(prepared/"prepare-receipt.json", receipt)
    a = {"schema": "strict-memory-tail-execution-acceptance-1", "law_id": M.LAW_ID,
         "verdict": "PASS_SCOPED_EXECUTION_READINESS", "canonical_adoption": False,
         "gates": dict.fromkeys(R.GATES, "PASS"), "prepared_sha256": data_hash,
         "prepare_receipt_sha256": R._sha(prepared/"prepare-receipt.json"), "spec_sha256": sources["spec.md"],
         "audit_path": "audit.md", "audit_sha256": R._sha(tmp_path/"audit.md"),
         "source_sha256": dict(sources, **{"extra.py": R._sha(tmp_path/"extra.py")}), "path_sha256": {},
         "environment": R._environment(), "preservation_seals": seals}
    return prepared, a


def test_complete_admission_preserves_extra_accepted_identities(tmp_path, monkeypatch):
    prepared, a = admission_fixture(tmp_path, monkeypatch)
    path = tmp_path/"acceptance.json"
    R._json(path, a)
    payload = R.accepted_inputs(prepared, path, R._sha(path))
    assert payload["source_sha256"]["extra.py"] == R._sha(tmp_path/"extra.py")
    assert payload["source_sha256"]["audit.md"] == R._sha(tmp_path/"audit.md")
    assert payload["source_sha256"]["prepared/prepare-receipt.json"] == R._sha(prepared/"prepare-receipt.json")
    assert payload["workers"] == 32 and payload["ceiling_seconds"] == 900


@pytest.mark.parametrize("damage", ["gate", "missing_source", "extra_drift", "prepared", "receipt", "spec", "audit", "environment", "seals", "adoption"])
def test_admission_rejects_missing_or_changed_readiness_inputs(tmp_path, monkeypatch, damage):
    prepared, a = admission_fixture(tmp_path, monkeypatch)
    if damage == "gate": a["gates"].pop("independent_contraction")
    elif damage == "missing_source": a["source_sha256"].pop("source.py")
    elif damage == "extra_drift": (tmp_path/"extra.py").write_bytes(b"changed extra")
    elif damage == "prepared": a["prepared_sha256"] = "0"*64
    elif damage == "receipt": a["prepare_receipt_sha256"] = "0"*64
    elif damage == "spec": a["spec_sha256"] = "0"*64
    elif damage == "audit": a["audit_sha256"] = "0"*64
    elif damage == "environment": a["environment"]["python"] = "different interpreter"
    elif damage == "seals": a["preservation_seals"] = []
    else: a["canonical_adoption"] = True
    path = tmp_path/"acceptance.json"
    R._json(path, a)
    with pytest.raises(ValueError):
        R.accepted_inputs(prepared, path, R._sha(path))


def test_all_runtime_project_imports_are_directly_pinned():
    pins = R._source_pins()
    # Source/byte identity only; no scientific certificate is evaluated.
    required = {"scripts/phi_v2_lattice/__init__.py", "scripts/phi_v2_lattice/experiments/__init__.py",
                "scripts/phi_v2_lattice/hydro_parity.py", "scripts/phi_v2_lattice/recovery_full_memory.py",
                "scripts/phi_v2_lattice/recovery_memory_tail.py", "scripts/phi_v2_lattice/recovery_evidence_chain.py",
                "scripts/phi_v2_lattice/experiments/certify_full_memory.py",
                "scripts/phi_v2_lattice/experiments/certify_memory_tail.py"}
    assert required <= pins.keys()


def test_inventory_includes_nested_report_and_retains_hash_errors(tmp_path, monkeypatch):
    R._write(tmp_path/"report.json", b"outer")
    R._write(tmp_path/"nested"/"report.json", b"retained inner")
    R._write(tmp_path/"retained.bin", b"unreadable synthetic fixture")
    original = R._sha
    def denied(path):
        if Path(path).name == "retained.bin":
            raise OSError("synthetic hash denial")
        return original(path)
    monkeypatch.setattr(R, "_sha", denied)
    result = R._artifacts(tmp_path)
    assert "nested/report.json" in result["sha256"] and "report.json" not in result["sha256"]
    assert "hash denial" in result["errors"]["retained.bin"]


def test_optional_corrupt_json_with_hash_denial_is_reportable(tmp_path, monkeypatch):
    R._write(tmp_path/"partial.json", b'{"partial":')
    def denied(path):
        raise OSError("synthetic unreadable metadata")
    monkeypatch.setattr(R, "_sha", denied)
    result = R._optional_json(tmp_path/"partial.json")
    assert result["status"] == "INVALID_RETAINED_JSON" and result["sha256"] is None
    assert "unreadable metadata" in result["hash_error"]


def test_hash_failure_does_not_escape_run_or_erase_failure_report(tmp_path, monkeypatch):
    monkeypatch.setattr(R, "ROOT", tmp_path)
    R._json(tmp_path/R.BASELINE, {"prior_seals": [{"synthetic": True}]})
    monkeypatch.setattr(R, "_source_pins", lambda: {"fake": "f"*64})
    def reject(*args):
        R._write(tmp_path/"attempt"/"retained.bin", b"retain these bytes")
        raise ValueError("synthetic admission failure")
    monkeypatch.setattr(R, "accepted_inputs", reject)
    monkeypatch.setattr(R, "_identity", lambda payload: {"status": "FAIL"})
    monkeypatch.setattr(R.PRESERVE, "verify_preservation", lambda *args: {"status": "PASS", "synthetic": True})
    original = R._sha
    def denied(path):
        if Path(path).name == "retained.bin":
            raise OSError("synthetic artifact hash failure")
        return original(path)
    monkeypatch.setattr(R, "_sha", denied)
    report = R.run(tmp_path/"p", tmp_path/"a", "1"*64, tmp_path/"attempt")
    assert report["status"] == "MEMORY_TAIL_INCOMPLETE"
    assert "retained.bin" in report["artifact_inventory_errors"]
    assert (tmp_path/"attempt"/"report.json").is_file()
    assert (tmp_path/"attempt"/"retained.bin").read_bytes() == b"retain these bytes"


def _cache_probe_target(payload, barrier):
    barrier.wait()
    import importlib
    sys.path.insert(0, payload["module_directory"])
    module = importlib.import_module("memory_tail_stale_fixture")
    Path(payload["result"]).write_text(json.dumps({"value": module.VALUE, "prefix": sys.pycache_prefix,
                                                 "writes_disabled": sys.dont_write_bytecode}), encoding="utf-8")


@pytest.mark.skipif(sys.platform != "win32", reason="actual isolated Windows spawned coordinator")
def test_fresh_child_prefix_rejects_matching_stale_pyc_and_restores_parent(tmp_path, monkeypatch):
    module = tmp_path/"memory_tail_stale_fixture.py"
    module.write_text("VALUE=1\n", encoding="utf-8")
    py_compile.compile(str(module), cfile=str(tmp_path/"__pycache__"/("memory_tail_stale_fixture."+sys.implementation.cache_tag+".pyc")), doraise=True)
    stat = module.stat()
    module.write_text("VALUE=2\n", encoding="utf-8")
    os.utime(module, ns=(stat.st_atime_ns, stat.st_mtime_ns))
    monkeypatch.setenv("PYTHONPYCACHEPREFIX", str(tmp_path/"inherited-old-cache"))
    monkeypatch.setenv("PYTHONPATH", "synthetic-inherited-override")
    old_options, old_prefix, old_writes = dict(sys._xoptions), sys.pycache_prefix, sys.dont_write_bytecode
    prefix, result_file = tmp_path/"fresh-child", tmp_path/"result.json"
    with R._cache_isolation(prefix):
        assert "PYTHONPATH" not in os.environ
        result = R.SUPERVISOR.supervise(_cache_probe_target,
                                      {"module_directory": str(tmp_path), "result": str(result_file)}, 8, R.MEMORY)
    assert result["actual_job_assignment"] and result["exit_code"] == 0 and result["failure"] is None
    observed = json.loads(result_file.read_text())
    assert observed == {"value": 2, "prefix": str(prefix.resolve()), "writes_disabled": True}
    assert not list(prefix.rglob("*"))
    assert os.environ["PYTHONPATH"] == "synthetic-inherited-override"
    assert os.environ["PYTHONPYCACHEPREFIX"] == str(tmp_path/"inherited-old-cache")
    assert (dict(sys._xoptions), sys.pycache_prefix, sys.dont_write_bytecode) == (old_options, old_prefix, old_writes)


def test_worker_command_overrides_inherited_cache_location(tmp_path):
    R._json(tmp_path/"lock.json", {"synthetic": True})
    command = R._worker_command(tmp_path, 0, 2)
    assert command[1] == "-B" and command[2] == "-X"
    assert command[3] == "pycache_prefix="+str((tmp_path/"bytecode-isolation"/"worker-0000").resolve())
    env = R._python_environment(tmp_path/"fresh")
    assert env["PYTHONDONTWRITEBYTECODE"] == "1"
    assert not {"PYTHONPATH", "PYTHONHOME", "PYTHONSTARTUP"}.intersection(env)


@pytest.mark.parametrize("value", [["invalid metadata type"], None, True, {"status": "PASS"}, {"schema": "wrong", "status": "PASS"}])
def test_optional_metadata_requires_a_typed_schema_object(tmp_path, value):
    path = tmp_path/"checkout-before.json"
    R._json(path, value)
    result = R._optional_json(path)
    assert result["status"] == "INVALID_RETAINED_JSON"
    assert path.read_bytes() == M.canonical_bytes(value)


def test_nondict_metadata_cannot_escape_failure_report(tmp_path, monkeypatch):
    monkeypatch.setattr(R, "ROOT", tmp_path)
    R._json(tmp_path/R.BASELINE, {"prior_seals": [{"synthetic": True}]})
    monkeypatch.setattr(R, "_source_pins", lambda: {"fake": "f"*64})
    def reject(*args):
        R._json(tmp_path/"attempt"/"checkout-before.json", ["invalid metadata type"])
        R._json(tmp_path/"attempt"/"preservation-after.json", ["invalid metadata type"])
        raise ValueError("synthetic admission failure")
    monkeypatch.setattr(R, "accepted_inputs", reject)
    monkeypatch.setattr(R, "_identity", lambda payload: {"status": "FAIL"})
    report = R.run(tmp_path/"p", tmp_path/"a", "1"*64, tmp_path/"attempt")
    assert report["status"] == "MEMORY_TAIL_INCOMPLETE"
    assert report["checkout_before"]["status"] == "INVALID_RETAINED_JSON"
    assert report["preservation_after"]["status"] == "INVALID_RETAINED_JSON"
    assert (tmp_path/"attempt"/"report.json").is_file()
