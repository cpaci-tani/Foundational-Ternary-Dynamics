"""Append-only orchestration of the unchanged exact full-memory calculation."""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import json
import multiprocessing as mp
import os
from pathlib import Path
import queue
import subprocess
import sys
import threading
import time
import traceback

ROOT = Path(__file__).resolve().parents[3]
if str(ROOT/"scripts") not in sys.path:
    sys.path.insert(0, str(ROOT/"scripts"))
from phi_v2_lattice.experiments import certify_full_memory as V1

M = V1.M
OWNED = (
    "scripts/phi_v2_lattice/experiments/certify_full_memory_v2.py",
    "scripts/tests/phi_v2_lattice/test_full_memory_execution_v2.py",
    "engine/docs/SPEC_STRICT_FULL_MEMORY_EXECUTION_V2.md",
)
EVIDENCE = ROOT/"engine/docs/evidence/strict-recovery-wave7-2026-09-08"
PRIOR_REPORT = EVIDENCE/"full-memory-attempt-1/report.json"
PRIOR_HASH = "70062ec64b79f87a67897793e253afc20ddcbb9589bc5035f79eef234ba44d1d"
V1_ACCEPTANCE_HASH = "c05c08815e5d076c50b094ff488a3af2093cc02b38e4f03da1bc4aa1b895aea7"
BINARY_HASH = "d6e6562c52f4b603d2ccdd4bc1f7c5b78c1661e0f4a069a77553e64f32da399b"
JOB_HASH = "722c02a81ccbec705c6c01e0850ba0ad13cdb1aa1a5715e4011df9dfbea5fabc"
GATES = (*V1.REQUIRED_GATES, "append_only_recovery")


def _write(path, data):
    """Exclusive durable write; interrupted bytes remain at their original path."""
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("xb") as stream:
        stream.write(data)
        stream.flush()
        os.fsync(stream.fileno())


def _json(path, value):
    _write(path, M.canonical_bytes(value))


def prior_attempt():
    if V1._sha(PRIOR_REPORT) != PRIOR_HASH:
        raise ValueError("failed V1 report changed")
    report = V1._read(PRIOR_REPORT)
    if report.get("status") != "FULL_K1_INCOMPLETE" or len(report.get("artifact_sha256", {})) != 1489:
        raise ValueError("wrong failed V1 attempt")
    paths = {str(PRIOR_REPORT): PRIOR_HASH}
    for name, digest in report["artifact_sha256"].items():
        path = (PRIOR_REPORT.parent/name).resolve()
        if not path.is_relative_to(PRIOR_REPORT.parent) or V1._sha(path) != digest:
            raise ValueError("retained V1 artifact changed: "+name)
        paths[str(path)] = digest
    pins = {"source_sha256": report["source_sha256"], "path_sha256": report["path_sha256"]}
    if V1._identity_outcome(pins)["status"] != "PASS":
        raise ValueError("V1 locked prerequisite drift")
    paths.update(report["path_sha256"])
    return report, paths


def accepted_inputs(prepared, binary, binary_hash, v1_acceptance, v1_acceptance_hash,
                    acceptance, acceptance_hash, census, census_hash):
    if binary_hash != BINARY_HASH or v1_acceptance_hash != V1_ACCEPTANCE_HASH:
        raise ValueError("V2 requires the unchanged accepted V1 binary and receipt")
    base = V1.accepted_inputs(prepared, binary, binary_hash, v1_acceptance,
                              v1_acceptance_hash, census, census_hash)
    if base["job_sha256"] != JOB_HASH:
        raise ValueError("V2 requires the unchanged V1 job")
    prior, retained_paths = prior_attempt()
    acceptance = Path(acceptance).resolve()
    if V1._sha(acceptance) != acceptance_hash:
        raise ValueError("V2 acceptance hash mismatch")
    receipt = V1._read(acceptance)
    expected = {"schema": "strict-full-memory-execution-v2-acceptance-1", "law_id": M.LAW_ID,
                "verdict": "PASS_SCOPED_EXACT_EXECUTION", "binary_sha256": binary_hash,
                "job_sha256": JOB_HASH, "analytic_data_sha256": base["analytic_data_sha256"],
                "independent_census_sha256": census_hash, "v1_acceptance_sha256": v1_acceptance_hash,
                "prior_attempt_report_sha256": PRIOR_HASH}
    if any(receipt.get(key) != value for key, value in expected.items()) or receipt.get("canonical_adoption") is not False:
        raise ValueError("V2 independent execution disposition mismatch")
    if any(receipt.get("gates", {}).get(key) != "PASS" for key in GATES):
        raise ValueError("V2 independent gates incomplete")
    source = receipt.get("source_sha256", {})
    if not (set(OWNED) | set(base["source_sha256"]) | set(prior["source_sha256"])) <= set(source):
        raise ValueError("V2 independent source acceptance incomplete")
    V1._check_pins(source)
    audit = receipt.get("audit_path")
    if not isinstance(audit, str) or Path(audit).is_absolute():
        raise ValueError("V2 audit must be repository relative")
    V1._check_pins({audit: receipt.get("audit_sha256")})
    merged = dict(base["source_sha256"])
    for addition in (prior["source_sha256"], source, {audit: receipt["audit_sha256"]}):
        for name, digest in addition.items():
            if name in merged and merged[name] != digest:
                raise ValueError("conflicting accepted source pin")
            merged[name] = digest
    paths = dict(base["path_sha256"])
    for name, digest in retained_paths.items():
        if name in paths and paths[name] != digest:
            raise ValueError("conflicting retained artifact pin")
        paths[name] = digest
    paths[str(acceptance)] = acceptance_hash
    return dict(base, source_sha256=merged, path_sha256=paths,
                v1_acceptance=base["acceptance"], v1_acceptance_sha256=v1_acceptance_hash,
                acceptance=str(acceptance), acceptance_sha256=acceptance_hash,
                audit_path=audit, audit_sha256=receipt["audit_sha256"],
                prior_attempt_report=str(PRIOR_REPORT), prior_attempt_report_sha256=PRIOR_HASH)


def recover_stdout(path, start, stop):
    """Only the contiguous newline-terminated, valid prefix carries progress."""
    path = Path(path)
    result = {"native": None, "prefix_bytes": 0, "prefix_lines": 0,
              "tail_error": None, "stdout_sha256": None, "stdout_bytes": 0}
    if not path.exists():
        return result
    raw = path.read_bytes()
    result.update(stdout_sha256=M.sha256(raw), stdout_bytes=len(raw))
    for line in raw.splitlines(keepends=True):
        if not line.endswith(b"\n"):
            result["tail_error"] = "unterminated final native line"
            break
        try:
            def unique(pairs):
                value = {}
                for key, item in pairs:
                    if key in value:
                        raise ValueError("duplicate native JSON key")
                    value[key] = item
                return value
            row = V1.validate_progress(json.loads(line, object_pairs_hook=unique), start, stop, result["native"])
        except Exception as error:
            result["tail_error"] = str(error)
            break
        result["native"] = row
        result["prefix_bytes"] += len(line)
        result["prefix_lines"] += 1
    return result


def append_progress(directory, start, sequence, native, pins):
    sequence = M._int(sequence, "progress sequence", 0)
    path = Path(directory)/f"ranges/{start:08d}/progress/{sequence:06d}.json"
    _json(path, {"schema": "strict-full-memory-v2-progress-1", "sequence": sequence,
                 "native": native, "job_sha256": pins["job_sha256"], "binary_sha256": pins["binary_sha256"]})
    return path


def _valid_final(path, kind, start, stop, pins):
    row = V1._read(path)
    schema = "strict-full-memory-range-receipt-1" if kind == "ranges" else "strict-full-memory-control-receipt-1"
    if (row.get("schema") != schema or row.get("status") not in ("PASS", "FAIL")
            or type(row.get("start")) is not int or type(row.get("stop")) is not int
            or row["start"] != start or row["stop"] != stop
            or any(row.get(key) != pins[key] for key in ("job_sha256", "binary_sha256"))):
        raise ValueError("invalid terminal receipt identity")
    native = row.get("native")
    if native is not None:
        V1.validate_progress(native, start, stop)
    if row["status"] == "PASS" and (native is None or native["status"] != "PASS"):
        raise ValueError("terminal PASS lacks complete native record")
    if kind == "ranges" and row.get("checked") != (0 if native is None else native["checked"]):
        raise ValueError("terminal receipt progress mismatch")
    name = path.name.split(".", 1)[0]
    stdout = path.parent/(name+".stdout")
    recovery = recover_stdout(stdout, start, stop)
    if native != recovery["native"]:
        raise ValueError("terminal receipt differs from durable native prefix")
    for suffix in ("stdout", "stderr"):
        log = path.parent/(name+"."+suffix)
        if row.get(suffix+"_sha256") != (V1._sha(log) if log.exists() else None):
            raise ValueError("terminal log identity changed")
    if row["status"] == "PASS":
        if recovery["tail_error"] or recovery["prefix_lines"] == 0:
            raise ValueError("terminal PASS has invalid raw output")
        if row.get("exit_code") != 0 or type(row.get("exit_code")) is not int:
            raise ValueError("terminal PASS lacks normal process-exit proof")
        if kind == "ranges" and row.get("native_eof") is not True:
            raise ValueError("terminal PASS lacks normally drained EOF")
        if kind == "controls":
            expected = row.get("expected")
            expected_keys = {"start", "stop", "completed_stop", "checked", "ordered_pairs",
                             "denominator_exponent", "numerators"}
            if not isinstance(expected, dict) or set(expected) != expected_keys:
                raise ValueError("control PASS lacks exact reference fields")
            V1.validate_progress(dict(expected, schema="strict-full-memory-native-range-1", status="PASS"), start, stop)
            if any(expected[key] != native[key] for key in expected_keys):
                raise ValueError("control PASS differs from exact reference")
            expected_path = path.parent/(name+".expected.json")
            expected_bytes = expected_path.read_bytes()
            if (V1._read(expected_path) != expected or expected_bytes != M.canonical_bytes(expected)
                    or row.get("expected_sha256") != M.sha256(expected_bytes)):
                raise ValueError("control PASS reference file identity changed")
    return row


def complete_receipts(directory, reason, pins):
    """Keep valid finals; append FAIL for every unfinished/malformed terminal."""
    directory = Path(directory)
    mappings = {"range_receipts": {}, "control_receipts": {}}
    for kind, entries, key in (("ranges", [(f"{s:08d}", s, t) for s, t in M.registered_ranges()], "range_receipts"),
                               ("controls", [(f"{j:04d}", s, t) for j, (s, t) in enumerate(V1._control_cases())], "control_receipts")):
        for name, start, stop in entries:
            path = directory/kind/(name+".json")
            invalid = None
            if path.exists():
                try:
                    _valid_final(path, kind, start, stop, pins)
                    mappings[key][name] = path.relative_to(directory).as_posix()
                    continue
                except Exception as error:
                    invalid = {"path": path.relative_to(directory).as_posix(), "sha256": V1._sha(path), "error": str(error)}
                    path = path.with_suffix(".recovery.json")
            if path.exists():
                row = _valid_final(path, kind, start, stop, pins)
                if row["status"] != "FAIL":
                    raise ValueError("recovery receipt cannot promote success")
                mappings[key][name] = path.relative_to(directory).as_posix()
                continue
            stdout = directory/kind/(name+".stdout")
            recovery = recover_stdout(stdout, start, stop)
            stderr = directory/kind/(name+".stderr")
            row = {"schema": "strict-full-memory-range-receipt-1" if kind == "ranges" else "strict-full-memory-control-receipt-1",
                   "status": "FAIL", "start": start, "stop": stop, "error": reason,
                   "native": recovery["native"], "recovery": recovery, "invalid_normal_receipt": invalid,
                   "job_sha256": pins["job_sha256"], "binary_sha256": pins["binary_sha256"],
                   "stdout_sha256": recovery["stdout_sha256"], "stderr_sha256": V1._sha(stderr) if stderr.exists() else None}
            if kind == "ranges":
                row["checked"] = 0 if recovery["native"] is None else recovery["native"]["checked"]
            expected = directory/kind/(name+".expected.json")
            if kind == "controls" and expected.exists():
                try:
                    row["expected"] = V1._read(expected)
                except Exception:
                    row["invalid_expected_sha256"] = V1._sha(expected)
            _json(path, row)
            mappings[key][name] = path.relative_to(directory).as_posix()
    return mappings


def controls(job, payload):
    directory = Path(payload["output"])
    for j, (start, stop) in enumerate(V1._control_cases()):
        prefix = directory/f"controls/{j:04d}"
        record = {"schema": "strict-full-memory-control-receipt-1", "start": start, "stop": stop,
                  "job_sha256": payload["job_sha256"], "binary_sha256": payload["binary_sha256"], "status": "FAIL"}
        _json(prefix.with_suffix(".started.json"), record)
        try:
            expected = M.central_reference_range(job, start, stop)
            record["expected"] = expected
            _json(prefix.with_suffix(".expected.json"), expected)
            record["expected_sha256"] = V1._sha(prefix.with_suffix(".expected.json"))
            left = payload["deadline"]-time.monotonic()
            if left <= 0:
                raise TimeoutError("deadline before control")
            with prefix.with_suffix(".stdout").open("xb") as stdout, prefix.with_suffix(".stderr").open("xb") as stderr:
                try:
                    completed = subprocess.run([payload["binary"], "--start", str(start), "--stop", str(stop),
                        "--seconds", str(min(left, V1.CEILING))], input=job, stdout=stdout, stderr=stderr,
                        timeout=left, creationflags=0x08000000)
                finally:
                    for stream in (stdout, stderr):
                        stream.flush()
                        os.fsync(stream.fileno())
            recovery = recover_stdout(prefix.with_suffix(".stdout"), start, stop)
            native = recovery["native"]
            record.update(native=native, exit_code=completed.returncode)
            if completed.returncode or recovery["tail_error"] or native is None or native["status"] != "PASS":
                raise ValueError("native reference control failed")
            if any(native[key] != value for key, value in expected.items()):
                raise AssertionError("independent central prefix/boundary disagreement")
            record["status"] = "PASS"
        except BaseException:
            record["error"] = traceback.format_exc()
            raise
        finally:
            for suffix in ("stdout", "stderr"):
                path = prefix.with_suffix("."+suffix)
                record[suffix+"_sha256"] = V1._sha(path) if path.exists() else None
            _json(prefix.with_suffix(".json"), record)


def central_workers(payload, job):
    directory = Path(payload["output"])
    messages = queue.Queue()
    active, completed = {}, []
    pending = iter(enumerate(M.registered_ranges()))
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
                    "--seconds", str(min(remaining, V1.CEILING))], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                    stderr=err, creationflags=0x08000000)
                record = {"proc": proc, "stderr": err, "start": start, "stop": stop, "last": None,
                          "eof": False, "sequence": 0, "started": time.monotonic()}
                active[index] = record
                thread = threading.Thread(target=V1._drain, args=(index, proc.stdout,
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
                row = V1.validate_progress(json.loads(data), start, stop, record["last"])
                record["last"] = row
                append_progress(directory, start, record["sequence"], row, payload)
                record["sequence"] += 1
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
                    "stop": stop, "checked": stop-start, "native": row, "progress_records": record["sequence"],
                    "exit_code": code, "native_eof": True,
                    "elapsed_seconds": time.monotonic()-record["started"], "job_sha256": payload["job_sha256"],
                    "binary_sha256": payload["binary_sha256"],
                    "stdout_sha256": V1._sha(directory/f"ranges/{start:08d}.stdout"),
                    "stderr_sha256": V1._sha(directory/f"ranges/{start:08d}.stderr")}
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


def coordinator(payload, barrier):
    barrier.wait()
    directory = Path(payload["output"])
    try:
        job = (Path(payload["prepared"])/"central-job.bin").read_bytes()
        if M.sha256(job) != payload["job_sha256"]:
            raise ValueError("job changed after lock")
        controls(job, payload)
        ranges = central_workers(payload, job)
        if len(ranges) != 128 or sum(r["checked"] for r in ranges) != M.PAIR_ORBITS:
            raise ValueError("central coverage incomplete")
        if sum(r["native"]["ordered_pairs"] for r in ranges) != M.N_ELIGIBLE**2:
            raise ValueError("central weighted ordered coverage")
        numerators = [sum(int(row["native"]["numerators"][i]) for row in ranges) for i in range(14)]
        _json(directory/"central-totals.json", {"denominator_exponent": 432, "numerators": [str(x) for x in numerators],
              "orbits": M.PAIR_ORBITS, "ordered_pairs": M.N_ELIGIBLE**2})
        certificate = M.certificate(M.assemble_delta(numerators))
        _json(directory/"certificate.json", certificate)
        identity = V1._identity_outcome(payload)
        if identity["status"] != "PASS":
            raise ValueError("execution prerequisite drift: "+str(identity["drift"]))
        if time.monotonic() >= payload["deadline"]:
            raise TimeoutError("deadline during reduction")
        _json(directory/"worker-completion.json", {"status": "PASS", "certificate_sha256": V1._sha(directory/"certificate.json"),
              "central_totals_sha256": V1._sha(directory/"central-totals.json"), "post_run_identity": identity})
    except BaseException:
        reason = traceback.format_exc()
        identity = V1._identity_outcome(payload)
        mappings = complete_receipts(directory, reason, payload)
        _json(directory/"worker-completion.json", {"status": "FAIL", "error": reason,
              "post_run_identity": identity, **mappings})
        raise


def run(prepared, binary, binary_hash, v1_acceptance, v1_acceptance_hash,
        acceptance, acceptance_hash, census, census_hash, output):
    output = Path(output).resolve()
    output.mkdir(parents=True, exist_ok=False)
    started = time.monotonic()
    try:
        pins = accepted_inputs(prepared, binary, binary_hash, v1_acceptance, v1_acceptance_hash,
                               acceptance, acceptance_hash, census, census_hash)
    except BaseException:
        _json(output/"gate-rejection.json", {"status": "FAIL", "central_pairs_evaluated": 0, "error": traceback.format_exc()})
        raise
    lock = dict(pins, schema="strict-full-memory-execution-v2-lock-1", created_utc=datetime.now(timezone.utc).isoformat(),
                ranges=[list(r) for r in M.registered_ranges()], workers=32, memory_limit_bytes=V1.MEMORY_LIMIT,
                ceiling_seconds=V1.CEILING, controls={"prefix": [0, 16], "boundary_offsets": [-1, 0]},
                canonical_adoption=False, central_second_complete_method=False,
                progress_protocol="unique exclusive receipts; authoritative contiguous native stdout")
    _json(output/"lock.json", lock)
    payload = dict(pins, output=str(output))
    supervision = finished = error = None
    try:
        supervision = V1.supervise(coordinator, payload, V1.CEILING, V1.MEMORY_LIMIT)
        finished = V1._read(output/"worker-completion.json") if (output/"worker-completion.json").exists() else None
    except BaseException:
        error = traceback.format_exc()
    identity = V1._identity_outcome(pins)
    success = (error is None and supervision is not None and supervision["failure"] is None
               and supervision["exit_code"] == 0 and finished is not None and finished["status"] == "PASS"
               and identity["status"] == "PASS")
    mappings = {"range_receipts": {}, "control_receipts": {}}
    receipt_error = None
    try:
        mappings = complete_receipts(output, error or (supervision or {}).get("failure") or "unfinished terminal receipt", pins)
    except BaseException:
        receipt_error = traceback.format_exc()
        success = False
    for key in ("range_receipts", "control_receipts"):
        if any(V1._read(output/name)["status"] != "PASS" for name in mappings[key].values()):
            success = False
    report = {"schema": "strict-full-memory-execution-v2-result-1", "status": "PASS" if success else "FULL_K1_INCOMPLETE",
              "supervision": supervision, "worker_completion": finished, "canonical_adoption": False,
              "source_sha256": pins["source_sha256"], "path_sha256": pins["path_sha256"], "post_run_identity": identity,
              "error": error, "receipt_finalization_error": receipt_error,
              "lock_sha256": V1._sha(output/"lock.json"), "end_to_end_seconds": time.monotonic()-started,
              **mappings, "artifact_sha256": {p.relative_to(output).as_posix(): V1._sha(p)
                                              for p in sorted(output.rglob("*")) if p.is_file()}}
    _json(output/"report.json", report)
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="mode", required=True)
    execute = sub.add_parser("run")
    for name in ("prepared", "binary", "binary-hash", "v1-acceptance", "v1-acceptance-hash", "acceptance",
                 "acceptance-hash", "census", "census-hash", "output"):
        execute.add_argument("--"+name, required=True)
    args = vars(parser.parse_args())
    args.pop("mode")
    result = run(**args)
    print(json.dumps({"mode": "run", "status": result["status"]}), flush=True)
    if result["status"] != "PASS":
        raise SystemExit(2)


if __name__ == "__main__":
    mp.freeze_support()
    main()
