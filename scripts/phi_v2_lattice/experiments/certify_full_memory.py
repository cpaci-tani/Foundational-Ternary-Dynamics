"""Prepared, independently gated exact central counting with a hard supervisor."""
from __future__ import annotations

import argparse
from bisect import bisect_left
import ctypes
from ctypes import wintypes as W
from datetime import datetime, timezone
import hashlib
import json
import multiprocessing as mp
import os
from pathlib import Path
import queue
import re
import shutil
import subprocess
import sys
import threading
import time
import traceback

ROOT = Path(__file__).resolve().parents[3]
if str(ROOT / "scripts") not in sys.path:
    sys.path.insert(0, str(ROOT / "scripts"))
from phi_v2_lattice import recovery_full_memory as M

OWNED = (
    "scripts/phi_v2_lattice/recovery_full_memory.py",
    "scripts/tests/phi_v2_lattice/test_recovery_full_memory.py",
    "scripts/phi_v2_lattice/experiments/certify_full_memory.py",
    "scripts/phi_v2_lattice/experiments/full_memory_native_v1.cpp",
    "engine/docs/SPEC_STRICT_FULL_MEMORY_EXECUTION_V1.md",
)
BASELINE = ROOT / "engine/docs/evidence/strict-recovery-wave7-baseline-2026-09-08.json"
CONTRACT = "engine/docs/CONTRACT_STRICT_RECOVERY_WAVE7_V1.md"
CEILING = 1800
MEMORY_LIMIT = 8 << 30
REQUIRED_GATES = ("arithmetic", "local_counts", "geometry_symmetry", "range_protocol", "failure_timeout")
DECIMAL = re.compile(r"(?:0|-[1-9][0-9]*|[1-9][0-9]*)\Z")
SHORT_ORBITS = tuple(sorted(p for i in range(M.N_ELIGIBLE//2)
                            for p in (i*(M.N_ELIGIBLE+1-i),
                                      i*(M.N_ELIGIBLE+1-i)+M.N_ELIGIBLE-2*i-1)))


def _sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def _write(path, data, *, replace=False):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists() and not replace:
        raise FileExistsError(f"retained evidence already exists: {path}")
    temporary = path.with_name(path.name + f".{os.getpid()}.{threading.get_ident()}.tmp")
    with temporary.open("xb") as stream:
        stream.write(data)
        stream.flush()
        os.fsync(stream.fileno())
    os.replace(temporary, path)


def _json(path, value, *, replace=False):
    _write(path, M.canonical_bytes(value), replace=replace)


def _read(path):
    def unique(pairs):
        value = {}
        for key, item in pairs:
            if key in value:
                raise ValueError("duplicate JSON key")
            value[key] = item
        return value
    return json.loads(Path(path).read_bytes(), object_pairs_hook=unique)


def _relative(path):
    return Path(path).resolve().relative_to(ROOT).as_posix()


def _source_pins():
    baseline = _read(BASELINE)
    pins = dict(baseline["source_sha256"])
    pins.update(M.frozen_inputs())
    pins.update({name: _sha(ROOT/name) for name in OWNED})
    pins[_relative(BASELINE)] = _sha(BASELINE)
    pins[CONTRACT] = _sha(ROOT/CONTRACT)
    _check_pins(pins)
    return pins


def _check_pins(pins):
    for name, expected in pins.items():
        path = (ROOT/name).resolve()
        if not path.is_relative_to(ROOT) or not path.is_file() or _sha(path) != expected:
            raise ValueError(f"input identity changed: {name}")


def _identity_outcome(payload):
    """Report every changed prerequisite without losing the original failure."""
    drift = []
    paths = {str((ROOT/name).resolve()): digest for name, digest in payload["source_sha256"].items()}
    for name, digest in payload["path_sha256"].items():
        if name in paths and paths[name] != digest:
            drift.append({"path": name, "error": "conflicting locked identity"})
        paths[name] = digest
    for name, digest in paths.items():
        try:
            actual = _sha(name)
            if actual != digest:
                drift.append({"path": name, "expected": digest, "actual": actual})
        except Exception as error:
            drift.append({"path": name, "expected": digest, "error": str(error)})
    return {"status": "PASS" if not drift else "FAIL", "checked_files": len(paths), "drift": drift}


def _build_identity(binary, binary_hash):
    """The explicit binary is admitted only through its exact sibling build receipt."""
    path = binary.parent/"build.json"
    record = _read(path)
    source = ROOT/"scripts/phi_v2_lattice/experiments/full_memory_native_v1.cpp"
    compiler = Path(record.get("compiler", "")).resolve()
    expected = [str(compiler), "-std=c++17", "-O3", "-DNDEBUG", str(source), "-o", str(binary)]
    if (record.get("schema") != "strict-full-memory-build-1" or record.get("status") != "PASS"
            or record.get("exit_code") != 0 or record.get("central_pairs_evaluated") != 0
            or record.get("source") != _relative(source) or record.get("source_sha256") != _sha(source)
            or record.get("binary") != str(binary) or record.get("binary_sha256") != binary_hash
            or record.get("command") != expected or not isinstance(record.get("compiler_version"), str)
            or not record["compiler_version"].strip() or record.get("compiler_sha256") != _sha(compiler)):
        raise ValueError("exact compiler/build/binary identity mismatch")
    return {str(path): _sha(path), str(compiler): record["compiler_sha256"], str(binary): binary_hash,
            str(binary.parent/"stdout.log"): _sha(binary.parent/"stdout.log"),
            str(binary.parent/"stderr.log"): _sha(binary.parent/"stderr.log")}


def prepare(directory):
    directory = Path(directory).resolve()
    directory.mkdir(parents=True, exist_ok=False)
    try:
        data = M.analytic_local_data()
        _json(directory/"local-data.json", data)
        _write(directory/"central-job.bin", M.central_job_bytes())
        matrices = M.noncentral_matrices()
        rows = [{"displacement": list(d), "matrix": [[M._fraction_string(x) for x in row] for row in matrix]}
                for d, matrix in matrices.items()]
        _json(directory/"noncentral.json", {"schema": "strict-full-memory-noncentral-1", "matrices": rows})
        _json(directory/"geometry.json", M.geometry())
        receipt = {"schema": "strict-full-memory-preparation-1", "law_id": M.LAW_ID,
                   "central_pairs_evaluated": 0, "source_sha256": _source_pins(),
                   "artifact_sha256": {p.name: _sha(p) for p in sorted(directory.iterdir()) if p.is_file()}}
        _json(directory/"prepare-receipt.json", receipt)
        return receipt
    except BaseException:
        _json(directory/"preparation-failure.json", {"status": "FAIL", "error": traceback.format_exc()})
        raise


def build(directory):
    directory = Path(directory).resolve()
    directory.mkdir(parents=True, exist_ok=False)
    compiler = shutil.which("g++")
    if not compiler:
        _json(directory/"build.json", {"status": "FAIL", "error": "g++ unavailable"})
        raise ValueError("standalone g++ compiler unavailable")
    compiler = str(Path(compiler).resolve())
    source = ROOT/"scripts/phi_v2_lattice/experiments/full_memory_native_v1.cpp"
    binary = directory/("full_memory_native_v1.exe" if os.name == "nt" else "full_memory_native_v1")
    args = [compiler, "-std=c++17", "-O3", "-DNDEBUG", str(source), "-o", str(binary)]
    record = {"schema": "strict-full-memory-build-1", "compiler": str(Path(compiler).resolve()),
              "compiler_sha256": _sha(compiler), "source": _relative(source), "source_sha256": _sha(source),
              "command": args, "binary": str(binary), "central_pairs_evaluated": 0}
    try:
        record["compiler_version"] = subprocess.run([compiler, "--version"], capture_output=True, text=True,
                                                    check=True, timeout=30).stdout
        result = subprocess.run(args, capture_output=True, timeout=120)
        _write(directory/"stdout.log", result.stdout)
        _write(directory/"stderr.log", result.stderr)
        record["exit_code"] = result.returncode
        if result.returncode != 0 or not binary.is_file():
            raise RuntimeError("offline helper build failed")
        record["binary_sha256"] = _sha(binary)
        record["status"] = "PASS"
    except BaseException:
        record["status"] = "FAIL"
        record["error"] = traceback.format_exc()
        _json(directory/"build.json", record)
        raise
    _json(directory/"build.json", record)
    return record


def _census(path, expected_hash, analytic):
    path = Path(path).resolve()
    if _sha(path) != expected_hash:
        raise ValueError("independent census receipt identity")
    report = _read(path)
    if (report.get("schema") != "independent-full-local-table-result-1" or report.get("passed") is not True
            or report.get("actual_counts_equal_walsh_formula") is not True
            or report.get("local_inputs_checked") != 1 << 24 or report.get("group_tables") != 324
            or report.get("eligible_count") != 11740 or report.get("failure") is not None
            or report.get("source_drift") != [] or not 0 <= report.get("elapsed_seconds", float("inf")) <= 300):
        raise ValueError("independent complete local census not accepted")
    artifacts = report.get("artifact_sha256", {})
    if "tables.json" not in artifacts or "lock.json" not in artifacts:
        raise ValueError("independent census artifact coverage")
    for name, digest in artifacts.items():
        target = (path.parent/name).resolve()
        if not target.is_relative_to(path.parent) or _sha(target) != digest:
            raise ValueError("independent census artifact changed")
    ranges = sorted(((_read(path.parent/name), name) for name in artifacts if name.startswith("ranges/")),
                    key=lambda x: x[0].get("start", -1))
    if len(ranges) != 64:
        raise ValueError("independent census range count")
    for j, (row, _) in enumerate(ranges):
        start, stop = j*(1 << 18), (j+1)*(1 << 18)
        if row.get("start") != start or row.get("stop") != stop or row.get("checked") != stop-start or row.get("status") != "PASS":
            raise ValueError("independent census incomplete range")
    lock = _read(path.parent/"lock.json")
    for name, digest in lock["source_sha256"].items():
        target = Path(name).resolve()
        if not target.is_relative_to(ROOT) or _sha(target) != digest:
            raise ValueError("independent census source changed")
    tables = _read(path.parent/"tables.json")
    M.verify_independent_tables(tables)
    for key in ("labels", "velocities", "groups", "joint_tables", "eligible_masks", "eligible_gram",
                "jacobian_numerator", "jacobian_denominator"):
        if analytic.get(key) != tables.get(key):
            raise ValueError("prepared input and independent table mismatch")
    return {"receipt": str(path), "receipt_sha256": expected_hash,
            "tables_sha256": artifacts["tables.json"], "source_sha256": lock["source_sha256"],
            "path_sha256": {str(path): expected_hash, **{str((path.parent/name).resolve()): digest
                                                         for name, digest in artifacts.items()}}}


def accepted_inputs(prepared, binary, binary_hash, acceptance, acceptance_hash, census, census_hash):
    prepared, binary, acceptance = Path(prepared).resolve(), Path(binary).resolve(), Path(acceptance).resolve()
    if _sha(binary) != binary_hash or _sha(acceptance) != acceptance_hash:
        raise ValueError("binary or acceptance receipt hash mismatch")
    path_pins = _build_identity(binary, binary_hash)
    prep = _read(prepared/"prepare-receipt.json")
    if prep.get("schema") != "strict-full-memory-preparation-1" or prep.get("law_id") != M.LAW_ID:
        raise ValueError("foreign prepared input")
    for name, digest in prep["artifact_sha256"].items():
        path = (prepared/name).resolve()
        if not path.is_relative_to(prepared) or _sha(path) != digest:
            raise ValueError("prepared artifact changed")
    required = {"local-data.json", "central-job.bin", "noncentral.json", "geometry.json"}
    if not required <= set(prep["artifact_sha256"]):
        raise ValueError("prepared artifact omission")
    _check_pins(prep["source_sha256"])
    analytic = _read(prepared/"local-data.json")
    if analytic != M.analytic_local_data() or (prepared/"central-job.bin").read_bytes() != M.central_job_bytes():
        raise ValueError("prepared data differs from exact implementation")
    independent = _census(census, census_hash, analytic)
    receipt = _read(acceptance)
    if (receipt.get("schema") != "strict-full-memory-acceptance-1" or receipt.get("law_id") != M.LAW_ID
            or receipt.get("verdict") != "PASS_SCOPED_EXACT_EXECUTION" or receipt.get("canonical_adoption") is not False
            or receipt.get("binary_sha256") != binary_hash
            or receipt.get("analytic_data_sha256") != _sha(prepared/"local-data.json")
            or receipt.get("independent_census_sha256") != census_hash):
        raise ValueError("independent execution disposition mismatch")
    if any(receipt.get("gates", {}).get(key) != "PASS" for key in REQUIRED_GATES):
        raise ValueError("independent execution gates incomplete")
    pins = receipt.get("source_sha256", {})
    if not set(OWNED) <= set(pins) or not set(M.INPUT_PINS) <= set(pins):
        raise ValueError("independent source acceptance incomplete")
    _check_pins(pins)
    audit = receipt.get("audit_path")
    if not isinstance(audit, str) or Path(audit).is_absolute():
        raise ValueError("audit path must be repository relative")
    _check_pins({audit: receipt.get("audit_sha256")})
    merged = _source_pins()
    for extra in (prep["source_sha256"], pins, {audit: receipt["audit_sha256"]},
                  {_relative(name): digest for name, digest in independent["source_sha256"].items()}):
        for name, digest in extra.items():
            if name in merged and merged[name] != digest:
                raise ValueError("conflicting prerequisite source identities")
            merged[name] = digest
    path_pins.update(independent["path_sha256"])
    path_pins.update({str(acceptance): acceptance_hash, str(prepared/"prepare-receipt.json"): _sha(prepared/"prepare-receipt.json")})
    path_pins.update({str((prepared/name).resolve()): digest for name, digest in prep["artifact_sha256"].items()})
    return {"source_sha256": merged, "path_sha256": path_pins, "prepared": str(prepared), "binary": str(binary),
            "binary_sha256": binary_hash, "acceptance": str(acceptance), "acceptance_sha256": acceptance_hash,
            "independent_census": independent, "job_sha256": _sha(prepared/"central-job.bin"),
            "analytic_data_sha256": _sha(prepared/"local-data.json"),
            "audit_path": audit, "audit_sha256": receipt["audit_sha256"]}


def _ordered(start, stop):
    return 4*(stop-start)-2*(bisect_left(SHORT_ORBITS, stop)-bisect_left(SHORT_ORBITS, start))


def validate_progress(row, start, stop, previous=None):
    required = {"schema", "start", "stop", "completed_stop", "checked", "ordered_pairs",
                "denominator_exponent", "numerators", "status"}
    if not isinstance(row, dict) or not required <= set(row) or set(row)-required-{"error"}:
        raise ValueError("native progress schema")
    if (row["schema"] != "strict-full-memory-native-range-1" or type(row["start"]) is not int
            or type(row["stop"]) is not int or row["start"] != start or row["stop"] != stop):
        raise ValueError("native range identity")
    done = M._int(row["completed_stop"], "completed stop", start, stop)
    if (type(row["checked"]) is not int or row["checked"] != done-start
            or type(row["ordered_pairs"]) is not int or row["ordered_pairs"] != _ordered(start, done)
            or row["denominator_exponent"] != 432 or row["status"] not in ("RUNNING", "PASS", "TIMEOUT", "FAIL")):
        raise ValueError("native range count or status")
    if row["status"] == "PASS" and done != stop:
        raise ValueError("short native PASS")
    if previous and (done < previous["completed_stop"] or previous["status"] != "RUNNING"):
        raise ValueError("native progress regression")
    if not isinstance(row["numerators"], list) or len(row["numerators"]) != 14:
        raise ValueError("native numerator dimension")
    for text in row["numerators"]:
        if not isinstance(text, str) or not DECIMAL.fullmatch(text) or abs(int(text)) > 1 << 434:
            raise ValueError("native exact numerator")
    return row


class _IO(ctypes.Structure):
    _fields_ = [(name, ctypes.c_uint64) for name in ("ReadOperationCount", "WriteOperationCount", "OtherOperationCount",
                                                    "ReadTransferCount", "WriteTransferCount", "OtherTransferCount")]


class _Basic(ctypes.Structure):
    _fields_ = [("PerProcessUserTimeLimit", ctypes.c_int64), ("PerJobUserTimeLimit", ctypes.c_int64),
                ("LimitFlags", W.DWORD), ("MinimumWorkingSetSize", ctypes.c_size_t),
                ("MaximumWorkingSetSize", ctypes.c_size_t), ("ActiveProcessLimit", W.DWORD),
                ("Affinity", ctypes.c_size_t), ("PriorityClass", W.DWORD), ("SchedulingClass", W.DWORD)]


class _Extended(ctypes.Structure):
    _fields_ = [("BasicLimitInformation", _Basic), ("IoInfo", _IO), ("ProcessMemoryLimit", ctypes.c_size_t),
                ("JobMemoryLimit", ctypes.c_size_t), ("PeakProcessMemoryUsed", ctypes.c_size_t),
                ("PeakJobMemoryUsed", ctypes.c_size_t)]


class _Memory(ctypes.Structure):
    _fields_ = [("cb", W.DWORD), ("PageFaultCount", W.DWORD)] + [
        (name, ctypes.c_size_t) for name in ("PeakWorkingSetSize", "WorkingSetSize", "QuotaPeakPagedPoolUsage",
                                            "QuotaPagedPoolUsage", "QuotaPeakNonPagedPoolUsage", "QuotaNonPagedPoolUsage",
                                            "PagefileUsage", "PeakPagefileUsage", "PrivateUsage")]


class WinJob:
    """Actual OS process-tree memory/termination boundary, fail-closed elsewhere."""
    def __init__(self, memory_limit):
        if os.name != "nt":
            raise RuntimeError("registered Windows supervisor required")
        self.k = ctypes.WinDLL("kernel32", use_last_error=True)
        self.k.CreateJobObjectW.argtypes = (ctypes.c_void_p, W.LPCWSTR)
        self.k.CreateJobObjectW.restype = W.HANDLE
        self.k.SetInformationJobObject.argtypes = (W.HANDLE, ctypes.c_int, ctypes.c_void_p, W.DWORD)
        self.k.QueryInformationJobObject.argtypes = (W.HANDLE, ctypes.c_int, ctypes.c_void_p, W.DWORD, ctypes.c_void_p)
        self.k.AssignProcessToJobObject.argtypes = (W.HANDLE, W.HANDLE)
        self.k.TerminateJobObject.argtypes = (W.HANDLE, W.UINT)
        self.k.IsProcessInJob.argtypes = (W.HANDLE, W.HANDLE, ctypes.POINTER(W.BOOL))
        self.k.OpenProcess.argtypes = (W.DWORD, W.BOOL, W.DWORD)
        self.k.OpenProcess.restype = W.HANDLE
        self.k.CloseHandle.argtypes = (W.HANDLE,)
        self.handle = self.k.CreateJobObjectW(None, None)
        if not self.handle:
            raise ctypes.WinError(ctypes.get_last_error())
        try:
            self.set_memory(memory_limit)
        except BaseException:
            self.close()
            raise

    def set_memory(self, limit):
        limit = M._int(limit, "job memory allowance", 1)
        info = _Extended()
        info.BasicLimitInformation.LimitFlags = 0x2000 | 0x200  # KILL_ON_JOB_CLOSE | JOB_MEMORY
        info.JobMemoryLimit = limit
        if not self.k.SetInformationJobObject(self.handle, 9, ctypes.byref(info), ctypes.sizeof(info)):
            raise ctypes.WinError(ctypes.get_last_error())

    def assign(self, pid):
        process = self.k.OpenProcess(0x0100 | 0x0001 | 0x0400, False, pid)
        if not process:
            raise ctypes.WinError(ctypes.get_last_error())
        try:
            if not self.k.AssignProcessToJobObject(self.handle, process):
                raise ctypes.WinError(ctypes.get_last_error())
            assigned = W.BOOL()
            if not self.k.IsProcessInJob(process, self.handle, ctypes.byref(assigned)) or not assigned.value:
                raise RuntimeError("coordinator not actually assigned to Job Object")
        finally:
            self.k.CloseHandle(process)

    def peak_memory(self):
        info = _Extended()
        if not self.k.QueryInformationJobObject(self.handle, 9, ctypes.byref(info), ctypes.sizeof(info), None):
            raise ctypes.WinError(ctypes.get_last_error())
        return int(info.PeakJobMemoryUsed)

    def terminate(self):
        if self.handle and not self.k.TerminateJobObject(self.handle, 124):
            raise ctypes.WinError(ctypes.get_last_error())

    def close(self):
        if self.handle:
            self.k.CloseHandle(self.handle)
            self.handle = None


def _parent_memory():
    k = ctypes.WinDLL("kernel32", use_last_error=True)
    k.GetCurrentProcess.restype = W.HANDLE
    p = ctypes.WinDLL("psapi", use_last_error=True)
    p.GetProcessMemoryInfo.argtypes = (W.HANDLE, ctypes.c_void_p, W.DWORD)
    info = _Memory()
    info.cb = ctypes.sizeof(info)
    if not p.GetProcessMemoryInfo(k.GetCurrentProcess(), ctypes.byref(info), ctypes.sizeof(info)):
        raise ctypes.WinError(ctypes.get_last_error())
    return int(info.PrivateUsage)


def supervise(target, payload, ceiling, memory_limit):
    """The killable child includes arithmetic, validation, reduction and hashing."""
    ceiling = float(ceiling)
    if not 0 < ceiling <= CEILING:
        raise ValueError("invalid registered ceiling")
    memory_limit = M._int(memory_limit, "memory ceiling", 1, MEMORY_LIMIT)
    context = mp.get_context("spawn")
    barrier = context.Event()
    started = time.monotonic()
    payload = dict(payload, deadline=started+ceiling)
    process = context.Process(target=target, args=(payload, barrier))
    peak_parent = _parent_memory()
    job = WinJob(memory_limit-peak_parent)
    reason = None
    assigned = False
    peak_total = peak_parent
    try:
        # Exact object arithmetic uses no BLAS worker pool. The coordinator's
        # imports must not allocate an unregistered extra pool of 32 threads.
        thread_keys = ("OPENBLAS_NUM_THREADS", "OMP_NUM_THREADS", "MKL_NUM_THREADS")
        inherited = {key: os.environ.get(key) for key in thread_keys}
        try:
            os.environ.update({key: "1" for key in thread_keys})
            process.start()
        finally:
            for key, value in inherited.items():
                if value is None:
                    os.environ.pop(key, None)
                else:
                    os.environ[key] = value
        job.assign(process.pid)
        assigned = True
        barrier.set()  # Child cannot execute central arithmetic before assignment.
        while process.is_alive():
            now = time.monotonic()
            peak_parent = max(peak_parent, _parent_memory())
            peak_total = max(peak_total, peak_parent+job.peak_memory())
            if now >= started+ceiling:
                reason = "global monotonic deadline"
                break
            if peak_total > memory_limit or peak_parent >= memory_limit:
                reason = "aggregate coordinator/worker memory ceiling"
                break
            job.set_memory(memory_limit-peak_parent)
            process.join(timeout=min(0.05, started+ceiling-now))
        if reason:
            job.terminate()
        process.join(timeout=5)
        if process.is_alive():
            process.kill()
            process.join(timeout=5)
        elapsed = time.monotonic()-started
        if elapsed > ceiling and reason is None:
            reason = "late completion past global deadline"
        peak_total = max(peak_total, peak_parent+job.peak_memory())
        if peak_total > memory_limit:
            reason = "aggregate coordinator/worker memory ceiling"
    except BaseException:
        reason = traceback.format_exc()
    finally:
        try:
            if process.pid is not None and process.is_alive():
                try:
                    job.terminate()
                except BaseException:
                    reason = (reason or "")+"\nJob termination error: "+traceback.format_exc()
                finally:
                    # Assignment can fail before the barrier opens: that child is
                    # not in the job and must still be explicitly killed.
                    if process.is_alive():
                        process.kill()
                    process.join(timeout=5)
        finally:
            job.close()
    return {"exit_code": process.exitcode, "failure": reason, "elapsed_seconds": time.monotonic()-started,
            "peak_aggregate_private_bytes": peak_total, "actual_job_assignment": assigned,
            "coordinator_pid": process.pid, "coordinator_alive_after_cleanup": process.is_alive(),
            "ceiling_seconds": ceiling, "memory_limit_bytes": memory_limit}


def _control_cases():
    cases = [(0, 16)]
    for boundary, _ in M.registered_ranges()[1:]:
        cases.extend(((boundary-1, boundary), (boundary, boundary+1)))
    return tuple(cases)


def _controls(job, binary, directory, deadline):
    for j, (start, stop) in enumerate(_control_cases()):
        record = {"schema": "strict-full-memory-control-receipt-1", "start": start, "stop": stop,
                  "job_sha256": M.sha256(job), "binary_sha256": _sha(binary), "status": "FAIL"}
        prefix = directory/f"controls/{j:04d}"
        _json(prefix.with_suffix(".started.json"), record)
        try:
            expected = M.central_reference_range(job, start, stop)
            record["expected"] = expected
            _json(prefix.with_suffix(".expected.json"), expected)
            left = deadline-time.monotonic()
            if left <= 0:
                raise TimeoutError("deadline before control")
            # Direct files retain emitted progress even on TimeoutExpired or a
            # supervisor tree kill; no captured pipe buffer disappears.
            with prefix.with_suffix(".stdout").open("xb") as stdout, prefix.with_suffix(".stderr").open("xb") as stderr:
                try:
                    completed = subprocess.run([binary, "--start", str(start), "--stop", str(stop),
                        "--seconds", str(min(left, CEILING))], input=job, stdout=stdout, stderr=stderr,
                        timeout=left, creationflags=0x08000000)
                finally:
                    for stream in (stdout, stderr):
                        stream.flush()
                        os.fsync(stream.fileno())
            previous = None
            for line in prefix.with_suffix(".stdout").read_bytes().splitlines():
                previous = validate_progress(json.loads(line), start, stop, previous)
            record["native"] = previous
            record["exit_code"] = completed.returncode
            if completed.returncode or previous is None or previous["status"] != "PASS":
                raise ValueError("native reference control failed")
            if any(previous[key] != value for key, value in expected.items()):
                raise AssertionError("independent central prefix/boundary disagreement")
            record["status"] = "PASS"
        except BaseException:
            record["error"] = traceback.format_exc()
            raise
        finally:
            for suffix in ("stdout", "stderr"):
                path = prefix.with_suffix("."+suffix)
                record[suffix+"_sha256"] = _sha(path) if path.exists() else None
            _json(prefix.with_suffix(".json"), record)


def _missing_controls(directory, reason, pins):
    directory = Path(directory)
    for j, (start, stop) in enumerate(_control_cases()):
        prefix = directory/f"controls/{j:04d}"
        if prefix.with_suffix(".json").exists():
            continue
        record = {"schema": "strict-full-memory-control-receipt-1", "status": "FAIL", "start": start,
                  "stop": stop, "error": reason, "job_sha256": pins["job_sha256"],
                  "binary_sha256": pins["binary_sha256"], "native": None}
        expected = prefix.with_suffix(".expected.json")
        if expected.exists():
            record["expected"] = _read(expected)
        for suffix in ("stdout", "stderr"):
            path = prefix.with_suffix("."+suffix)
            record[suffix+"_sha256"] = _sha(path) if path.exists() else None
        stdout = prefix.with_suffix(".stdout")
        if stdout.exists():
            try:
                for line in stdout.read_bytes().splitlines():
                    record["native"] = validate_progress(json.loads(line), start, stop, record["native"])
            except Exception:
                record["truncated_or_invalid_tail"] = True
        _json(prefix.with_suffix(".json"), record)


def _drain(index, stream, logfile, messages):
    try:
        with logfile.open("xb") as out:
            for line in iter(stream.readline, b""):
                out.write(line)
                out.flush()
                os.fsync(out.fileno())
                messages.put((index, "line", line))
        messages.put((index, "eof", None))
    except BaseException:
        messages.put((index, "error", traceback.format_exc()))
    finally:
        stream.close()


def _missing_ranges(directory, reason, pins):
    directory = Path(directory)
    for start, stop in M.registered_ranges():
        final = directory/f"ranges/{start:08d}.json"
        if final.exists():
            continue
        progress = directory/f"ranges/{start:08d}.progress.json"
        try:
            last = _read(progress)["native"] if progress.exists() else None
        except Exception:
            last = None
        row = {"schema": "strict-full-memory-range-receipt-1", "status": "FAIL", "start": start, "stop": stop,
               "error": reason, "native": last, "checked": 0 if last is None else last["checked"],
               "job_sha256": pins["job_sha256"], "binary_sha256": pins["binary_sha256"]}
        for suffix in ("stdout", "stderr"):
            path = directory/f"ranges/{start:08d}.{suffix}"
            row[f"{suffix}_sha256"] = _sha(path) if path.exists() else None
        _json(final, row)


def _central_workers(payload, job):
    directory = Path(payload["output"])
    ranges = M.registered_ranges()
    messages = queue.Queue()
    active, completed = {}, []
    pending = iter(enumerate(ranges))
    exhausted = False
    try:
        while active or not exhausted:
            if time.monotonic() >= payload["deadline"]:
                raise TimeoutError("central coordinator deadline")
            while len(active) < 32 and not exhausted:
                try:
                    index, (start, stop) = next(pending)
                except StopIteration:
                    exhausted = True
                    break
                remaining = payload["deadline"]-time.monotonic()
                if remaining <= 0:
                    raise TimeoutError("deadline before worker launch")
                error_path = directory/f"ranges/{start:08d}.stderr"
                error_path.parent.mkdir(exist_ok=True)
                err = error_path.open("xb")
                proc = subprocess.Popen([payload["binary"], "--start", str(start), "--stop", str(stop),
                                         "--seconds", str(min(remaining, CEILING))],
                                        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=err,
                                        creationflags=0x08000000)
                record = {"proc": proc, "stderr": err, "start": start, "stop": stop, "last": None, "eof": False,
                          "started": time.monotonic()}
                active[index] = record
                thread = threading.Thread(target=_drain, args=(index, proc.stdout,
                    directory/f"ranges/{start:08d}.stdout", messages), daemon=True)
                record["thread"] = thread
                thread.start()
                proc.stdin.write(job)
                proc.stdin.close()
            try:
                index, kind, data = messages.get(timeout=0.05)
            except queue.Empty:
                continue
            record = active[index]
            start, stop = record["start"], record["stop"]
            if kind == "error":
                raise RuntimeError(data)
            if kind == "line":
                row = validate_progress(json.loads(data), start, stop, record["last"])
                record["last"] = row
                _json(directory/f"ranges/{start:08d}.progress.json",
                      {"native": row, "job_sha256": payload["job_sha256"], "binary_sha256": payload["binary_sha256"]},
                      replace=True)
                if row["status"] in ("FAIL", "TIMEOUT"):
                    raise RuntimeError(f"native range {start} {row['status']}")
            else:
                record["eof"] = True
            if record["eof"]:
                code = record["proc"].wait(timeout=max(0.01, min(1, payload["deadline"]-time.monotonic())))
                row = record["last"]
                record["stderr"].flush()
                os.fsync(record["stderr"].fileno())
                record["stderr"].close()
                if code != 0 or row is None or row["status"] != "PASS":
                    raise RuntimeError("missing successful complete native range")
                receipt = {"schema": "strict-full-memory-range-receipt-1", "status": "PASS", "start": start,
                           "stop": stop, "checked": stop-start, "native": row,
                           "elapsed_seconds": time.monotonic()-record["started"],
                           "job_sha256": payload["job_sha256"], "binary_sha256": payload["binary_sha256"],
                           "stdout_sha256": _sha(directory/f"ranges/{start:08d}.stdout"),
                           "stderr_sha256": _sha(directory/f"ranges/{start:08d}.stderr")}
                _json(directory/f"ranges/{start:08d}.json", receipt)
                completed.append(receipt)
                del active[index]
                if len(completed) % 16 == 0:
                    print(json.dumps({"completed_ranges": len(completed),
                                      "checked_orbits": sum(r["checked"] for r in completed)}), flush=True)
        return sorted(completed, key=lambda r: r["start"])
    finally:
        for record in active.values():
            if record["proc"].poll() is None:
                record["proc"].kill()
        for record in active.values():
            try:
                record["proc"].wait(timeout=2)
                record["thread"].join(timeout=2)
            except Exception:
                pass
            if not record["stderr"].closed:
                record["stderr"].close()


def _coordinator(payload, barrier):
    barrier.wait()
    directory = Path(payload["output"])
    try:
        job = (Path(payload["prepared"])/"central-job.bin").read_bytes()
        if M.sha256(job) != payload["job_sha256"]:
            raise ValueError("job changed after lock")
        _controls(job, payload["binary"], directory, payload["deadline"])
        ranges = _central_workers(payload, job)
        if len(ranges) != 128 or sum(r["checked"] for r in ranges) != M.PAIR_ORBITS:
            raise ValueError("central coverage incomplete")
        if sum(r["native"]["ordered_pairs"] for r in ranges) != M.N_ELIGIBLE**2:
            raise ValueError("central weighted ordered coverage")
        numerators = [sum(int(row["native"]["numerators"][i]) for row in ranges) for i in range(14)]
        _json(directory/"central-totals.json", {"denominator_exponent": 432,
              "numerators": [str(x) for x in numerators], "orbits": M.PAIR_ORBITS, "ordered_pairs": M.N_ELIGIBLE**2})
        delta = M.assemble_delta(numerators)
        certificate = M.certificate(delta)
        _json(directory/"certificate.json", certificate)
        identity = _identity_outcome(payload)
        if identity["status"] != "PASS":
            raise ValueError("execution prerequisite identity changed: "+str(identity["drift"]))
        if time.monotonic() >= payload["deadline"]:
            raise TimeoutError("deadline during reduction")
        _json(directory/"worker-completion.json", {"status": "PASS", "certificate_sha256": _sha(directory/"certificate.json"),
              "central_totals_sha256": _sha(directory/"central-totals.json"), "post_run_identity": identity})
    except BaseException:
        reason = traceback.format_exc()
        identity = _identity_outcome(payload)
        _missing_controls(directory, reason, payload)
        _missing_ranges(directory, reason, payload)
        _json(directory/"worker-completion.json", {"status": "FAIL", "error": reason, "post_run_identity": identity})
        raise


def run(prepared, binary, binary_hash, acceptance, acceptance_hash, census, census_hash, output):
    output = Path(output).resolve()
    output.mkdir(parents=True, exist_ok=False)
    started = time.monotonic()
    try:
        pins = accepted_inputs(prepared, binary, binary_hash, acceptance, acceptance_hash, census, census_hash)
    except BaseException:
        _json(output/"gate-rejection.json", {"status": "FAIL", "central_pairs_evaluated": 0, "error": traceback.format_exc()})
        raise
    lock = dict(pins, schema="strict-full-memory-execution-lock-1",
                created_utc=datetime.now(timezone.utc).isoformat(), ranges=[list(r) for r in M.registered_ranges()],
                workers=32, memory_limit_bytes=MEMORY_LIMIT, ceiling_seconds=CEILING,
                controls={"prefix": [0, 16], "boundary_offsets": [-1, 0]},
                canonical_adoption=False, central_second_complete_method=False)
    _json(output/"lock.json", lock)
    payload = dict(pins, output=str(output))
    supervision, finished, error = None, None, None
    try:
        supervision = supervise(_coordinator, payload, CEILING, MEMORY_LIMIT)
        finished = _read(output/"worker-completion.json") if (output/"worker-completion.json").exists() else None
    except BaseException:
        error = traceback.format_exc()
    # Evidence finalization performs no scientific contraction after the global
    # deadline. It retains failed or killed process output and all identity drift.
    identity = _identity_outcome(pins)
    success = (error is None and supervision is not None and supervision["failure"] is None
               and supervision["exit_code"] == 0 and finished is not None and finished["status"] == "PASS"
               and identity["status"] == "PASS")
    if not success:
        reason = error or (supervision or {}).get("failure") or "coordinator or identity checks did not complete successfully"
        _missing_controls(output, reason, pins)
        _missing_ranges(output, reason, pins)
    report = {"schema": "strict-full-memory-execution-result-1", "status": "PASS" if success else "FULL_K1_INCOMPLETE",
              "supervision": supervision, "worker_completion": finished, "canonical_adoption": False,
              "source_sha256": pins["source_sha256"], "path_sha256": pins["path_sha256"], "post_run_identity": identity,
              "error": error, "lock_sha256": _sha(output/"lock.json"), "end_to_end_seconds": time.monotonic()-started,
              "artifact_sha256": {p.relative_to(output).as_posix(): _sha(p) for p in sorted(output.rglob("*"))
                                  if p.is_file()}}
    _json(output/"report.json", report)
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="mode", required=True)
    for mode in ("prepare", "build"):
        command = sub.add_parser(mode)
        command.add_argument("--output", required=True)
    execute = sub.add_parser("run")
    for name in ("prepared", "binary", "binary-hash", "acceptance", "acceptance-hash", "census", "census-hash", "output"):
        execute.add_argument("--"+name, required=True)
    args = vars(parser.parse_args())
    mode = args.pop("mode")
    if mode == "prepare":
        result = prepare(args["output"])
    elif mode == "build":
        result = build(args["output"])
    else:
        result = run(**{k.replace("-", "_"): v for k, v in args.items()})
    print(json.dumps({"mode": mode, "status": result.get("status", "PASS")}), flush=True)
    if result.get("status") == "FULL_K1_INCOMPLETE":
        raise SystemExit(2)


if __name__ == "__main__":
    mp.freeze_support()
    main()
