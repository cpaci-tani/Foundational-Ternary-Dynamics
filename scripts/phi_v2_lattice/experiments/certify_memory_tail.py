"""Independently gated, append-only, supervised exact W2 certification."""
from __future__ import annotations

import argparse
from contextlib import contextmanager
from datetime import datetime, timezone
import importlib.metadata
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
from phi_v2_lattice import recovery_memory_tail as M
from phi_v2_lattice import recovery_evidence_chain as PRESERVE
from phi_v2_lattice.experiments import certify_full_memory as SUPERVISOR

OWNED = ("scripts/phi_v2_lattice/recovery_memory_tail.py",
         "scripts/phi_v2_lattice/experiments/certify_memory_tail.py",
         "scripts/tests/phi_v2_lattice/test_recovery_memory_tail.py",
         "scripts/tests/phi_v2_lattice/test_memory_tail_execution.py",
         "engine/docs/SPEC_STRICT_MEMORY_TAIL_EXECUTION_V1.md")
BASELINE = "engine/docs/evidence/strict-recovery-wave8-baseline-2026-09-08.json"
OLD = "engine/docs/evidence/strict-recovery-wave7-2026-09-08/"
PINNED = {
    OWNED[-1]: "55026b0737bf00902c57b483352fc23a8694fd1aeb519da230e4dedbb4a69a73",
    BASELINE: "4319e3faa45906df3f769a8f071b093d9e9156ba681535eb41b94a9dc0b20201",
    "engine/docs/CONTRACT_STRICT_RECOVERY_WAVE8_V1.md": "39cf1a5380057c65b2a3f52df61f1d5f60d319fc53c2303d9c1258e685f6c38e",
    "engine/docs/PROPOSAL_STRICT_MEMORY_TAIL_V1.md": "58681c487389780ef486ac76786c6496a0f7d4b3ede3b509b527738fdb9ef9b1",
    "engine/docs/AUDIT_STRICT_MEMORY_TAIL_PLAN_V1.md": "bf40a55eaf0a9ddfbf05fd5b0448104efdcd4e20cfc92fa3fa7bea57bdfe566f",
    "engine/docs/AUDIT_STRICT_MEMORY_TAIL_EXECUTION_PLAN_V1.md": "4270eca0254a958c7efd935e49864e37ad33f119af652de9aac265caf63e5fc3",
    "engine/docs/AUDIT_STRICT_FULL_MEMORY_RESULTS_V2.md": "96dbc6add3421e5c1854950e0ed124dd06826f6205a1f4654204e57e3cf7d444",
    "scripts/phi_v2_lattice/hydro_parity.py": "593bd2eb314bd4f712e4ac125600147518003fecd8ad98ca51cd8893f77a410b",
    "scripts/phi_v2_lattice/recovery_full_memory.py": "2bed818bc2272fcba65e8dde693b6e4ec715c59a75fa860eb45cb4536b49bf21",
    "scripts/phi_v2_lattice/experiments/certify_full_memory.py": "4d8ab048c892638e84ae09db8fcc09890f2ba6be62ac87eee467b7221db2d9e8",
    OLD+"full-memory-v2-attempt-1/certificate.json": "1ac7f40366305943441feb904a8d9c542c502c0456af644a15828d26d4207348",
    OLD+"full-memory-v2-attempt-1/report.json": "ff95b63788d2ab8d15fe7c9d958fdc41352f36024fca4a0a033f8dbf41e40859",
    OLD+"full-memory-v2-attempt-1/lock.json": "20276c5268c8fac4d5d8b9ce6b1d5075fde9f99e43e64375b95fe656cad10028",
    OLD+"local-tables-attempt-1/tables.json": "fd73a78da9cb9b423b03220f982ce3586caff4fcb422721ed0135a8719709197",
}
CERTIFICATE = OLD+"full-memory-v2-attempt-1/certificate.json"
CENSUS = OLD+"local-tables-attempt-1/tables.json"
EXTRA = ("scripts/phi_v2_lattice/__init__.py", "scripts/phi_v2_lattice/experiments/__init__.py",
         "scripts/phi_v2_lattice/recovery_evidence_chain.py",
         "scripts/tests/phi_v2_lattice/test_recovery_evidence_chain.py")
GATES = ("exact_arithmetic", "complete_support", "independent_contraction", "observable_bounds",
         "receipt_protocol", "resource_failure")
CEILING, MEMORY, WORKERS = 900, 8 << 30, 32
MAX_LINE = 1 << 20
PYTHON_OVERRIDES = ("PYTHONPYCACHEPREFIX", "PYTHONDONTWRITEBYTECODE", "PYTHONPATH", "PYTHONHOME", "PYTHONSTARTUP")


def _sha(path):
    return PRESERVE.sha256(Path(path))


def _read(path):
    return M.read_json(Path(path).read_bytes())


def _write(path, data):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("xb") as stream:
        stream.write(data)
        stream.flush()
        os.fsync(stream.fileno())


def _json(path, value):
    _write(path, M.canonical_bytes(value))


def _digest(value):
    if type(value) is not str or len(value) != 64 or any(c not in "0123456789abcdef" for c in value):
        raise ValueError("canonical SHA256 required")
    return value


def _relative(path):
    return Path(path).resolve().relative_to(ROOT).as_posix()


def _repo(name):
    if type(name) is not str or not name or "\\" in name or ":" in name or Path(name).is_absolute() or any(x in ("", ".", "..") for x in name.split("/")):
        raise ValueError("canonical repository-relative path required")
    path = (ROOT/name).resolve()
    if not path.is_relative_to(ROOT):
        raise ValueError("path escapes repository")
    return path


def _check_pins(sources, paths=None):
    if type(sources) is not dict or type(paths or {}) is not dict:
        raise ValueError("identity maps required")
    for name, digest in sources.items():
        if _sha(_repo(name)) != _digest(digest):
            raise ValueError("source identity changed: "+name)
    for name, digest in (paths or {}).items():
        path = Path(name).resolve()
        if not path.is_relative_to(ROOT) and path != Path(sys.executable).resolve():
            raise ValueError("unregistered external input")
        if _sha(path) != _digest(digest):
            raise ValueError("path identity changed: "+name)


def _source_pins():
    pins = dict(PINNED)
    for name in OWNED+EXTRA:
        current = _sha(_repo(name))
        if name in pins and pins[name] != current:
            raise ValueError("frozen specification differs")
        pins[name] = current
    _check_pins(pins)
    return pins


def _environment():
    return {"python": sys.version, "python_executable": str(Path(sys.executable).resolve()),
            "python_sha256": _sha(sys.executable),
            "packages": {name: importlib.metadata.version(name) for name in ("numpy", "python-flint")}}


def prepare(certificate, census, output):
    output = Path(output).resolve()
    _relative(output)
    if output.exists():
        raise FileExistsError("new preparation directory required")
    for path, expected in ((certificate, PINNED[CERTIFICATE]), (census, PINNED[CENSUS])):
        _relative(path)
        if _sha(path) != expected:
            raise ValueError("accepted scientific input differs")
    sources = _source_pins()
    cbytes, tbytes = Path(certificate).read_bytes(), Path(census).read_bytes()
    if (M.sha256(cbytes), M.sha256(tbytes)) != (PINNED[CERTIFICATE], PINNED[CENSUS]):
        raise ValueError("input changed while preparing")
    data = M.prepare(M.read_json(cbytes), M.read_json(tbytes)).mapping()
    output.mkdir(parents=True, exist_ok=False)
    _json(output/"prepared.json", data)
    receipt = {"schema": "strict-memory-tail-prepare-receipt-1", "law_id": M.LAW_ID,
               "prepared_sha256": _sha(output/"prepared.json"), "source_sha256": sources,
               "certificate_sha256": PINNED[CERTIFICATE], "census_sha256": PINNED[CENSUS],
               "numerical_predicates_evaluated": False, "canonical_adoption": False}
    _check_pins(sources)
    _json(output/"prepare-receipt.json", receipt)
    return receipt


def _merge(target, extra):
    if type(extra) is not dict:
        raise ValueError("identity map required")
    for key, value in extra.items():
        _digest(value)
        if key in target and target[key] != value:
            raise ValueError("conflicting accepted identities")
        target[key] = value


def accepted_inputs(prepared, acceptance, acceptance_sha256):
    prepared, acceptance = Path(prepared).resolve(), Path(acceptance).resolve()
    _relative(prepared)
    _relative(acceptance)
    if _sha(acceptance) != _digest(acceptance_sha256):
        raise ValueError("explicit independent acceptance hash differs")
    a, p = _read(acceptance), _read(prepared/"prepare-receipt.json")
    if (a.get("schema") != "strict-memory-tail-execution-acceptance-1" or a.get("law_id") != M.LAW_ID
            or a.get("verdict") != "PASS_SCOPED_EXECUTION_READINESS"
            or a.get("canonical_adoption") is not False
            or any(a.get("gates", {}).get(k) != "PASS" for k in GATES)):
        raise ValueError("independent readiness gates incomplete")
    if (p.get("schema") != "strict-memory-tail-prepare-receipt-1" or p.get("law_id") != M.LAW_ID
            or p.get("numerical_predicates_evaluated") is not False
            or p.get("certificate_sha256") != PINNED[CERTIFICATE] or p.get("census_sha256") != PINNED[CENSUS]):
        raise ValueError("prepared provenance")
    data_hash, receipt_hash = _sha(prepared/"prepared.json"), _sha(prepared/"prepare-receipt.json")
    if (data_hash != p.get("prepared_sha256") or data_hash != a.get("prepared_sha256")
            or receipt_hash != a.get("prepare_receipt_sha256") or a.get("spec_sha256") != PINNED[OWNED[-1]]):
        raise ValueError("prepared/spec acceptance binding")
    if _sha(_repo(a["audit_path"])) != _digest(a["audit_sha256"]):
        raise ValueError("independent readiness audit changed")
    source_pins = _source_pins()
    for mapping in (p["source_sha256"], a["source_sha256"]):
        _merge(source_pins, mapping)
    # Every direct implementation input must be explicitly admitted, not added
    # after accepting a smaller source map.
    if any(a["source_sha256"].get(k) != v for k, v in _source_pins().items()):
        raise ValueError("acceptance omits direct implementation input")
    for path in (prepared/"prepared.json", prepared/"prepare-receipt.json", acceptance, _repo(a["audit_path"])):
        _merge(source_pins, {_relative(path): _sha(path)})
    paths = dict(a.get("path_sha256", {}))
    environment = _environment()
    _merge(paths, {environment["python_executable"]: environment["python_sha256"]})
    if a.get("environment") != environment:
        raise ValueError("accepted interpreter/package environment differs")
    seals = a.get("preservation_seals")
    if seals != _read(_repo(BASELINE))["prior_seals"]:
        raise ValueError("complete preservation seal registration differs")
    _check_pins(source_pins, paths)
    # Parse/align validation remains separate from all new numerical predicates.
    M.restore_prepared(_read(prepared/"prepared.json"))
    return {"law_id": M.LAW_ID, "prepared": _relative(prepared/"prepared.json"),
            "prepared_sha256": data_hash, "acceptance": _relative(acceptance), "acceptance_sha256": acceptance_sha256,
            "source_sha256": source_pins, "path_sha256": paths, "environment": environment,
            "preservation_seals": seals, "workers": WORKERS, "ceiling_seconds": CEILING,
            "memory_limit_bytes": MEMORY, "ranges": [list(x) for x in M.RANGES],
            "support": [list(x) for x in M.SUPPORT], "stresses": M.STRESSES,
            "fourier_controls": M.FOURIER, "horizons": M.HORIZONS,
            "delta_denominator_exponent": 432, "W2_denominator_exponent": 864,
            "coefficient_bound_bits": 877, "fourier_bound_bits": 885}


def _binding(payload):
    return {"law_id": M.LAW_ID, "prepared_sha256": payload.get("prepared_sha256", "unavailable"),
            "source_identity_sha256": M.sha256(M.canonical_bytes({"source_sha256": payload.get("source_sha256", {}),
                                                                 "path_sha256": payload.get("path_sha256", {})}))}


def _aggregate(start, stop):
    return {key: sum(M.counters(i)[key] for i in range(start, stop))
            for key in M.counters(0)}


def _terminal(start, stop, binding):
    return {"schema": "strict-memory-tail-worker-terminal-1", **binding, "status": "PASS",
            "start": start, "stop": stop, "completed_stop": stop, "checked": stop-start,
            "counters": _aggregate(start, stop)}


def _started(number, binding):
    start, stop = M.RANGES[number]
    return {"schema": "strict-memory-tail-range-started-1", **binding,
            "range_index": number, "start": start, "stop": stop,
            "python_cache_prefix": f"bytecode-isolation/worker-{start:04d}", "bytecode_writes": False}


def _python_environment(prefix):
    environment = dict(os.environ)
    for name in PYTHON_OVERRIDES:
        environment.pop(name, None)
    environment.update(PYTHONPYCACHEPREFIX=str(Path(prefix).resolve()), PYTHONDONTWRITEBYTECODE="1")
    return environment


@contextmanager
def _cache_isolation(prefix):
    """Fresh read origin as well as disabled writes, without changing supervisor."""
    prefix = Path(prefix).resolve()
    prefix.mkdir(parents=True, exist_ok=False)
    old_environment = {key: os.environ.get(key) for key in PYTHON_OVERRIDES}
    old_options, old_prefix, old_writes = dict(sys._xoptions), sys.pycache_prefix, sys.dont_write_bytecode
    try:
        environment = _python_environment(prefix)
        for key in PYTHON_OVERRIDES:
            if key in environment:
                os.environ[key] = environment[key]
            else:
                os.environ.pop(key, None)
        sys._xoptions["pycache_prefix"] = str(prefix)
        sys.pycache_prefix, sys.dont_write_bytecode = str(prefix), True
        yield prefix
    finally:
        for key, value in old_environment.items():
            if value is None:
                os.environ.pop(key, None)
            else:
                os.environ[key] = value
        sys._xoptions.clear()
        sys._xoptions.update(old_options)
        sys.pycache_prefix, sys.dont_write_bytecode = old_prefix, old_writes


def _check_cache_prefix(prefix):
    prefix = Path(prefix).resolve()
    if (sys.pycache_prefix != str(prefix) or not sys.dont_write_bytecode
            or not prefix.is_dir() or any(prefix.rglob("*"))):
        raise ValueError("fresh empty bytecode read origin and disabled writes required")


def _check_row(row, start, stop, index, binding):
    if any(row.get(key) != value for key, value in binding.items()):
        raise ValueError("worker source/preparation identity")
    if row.get("schema") == "strict-memory-tail-coefficient-1":
        if index >= stop:
            raise ValueError("coefficient after complete range")
        M.validate_coefficient(row, index)
        return "coefficient"
    expected = _terminal(start, stop, binding)
    if (row != expected or index != stop
            or any(type(row.get(k)) is not int for k in ("start", "stop", "completed_stop", "checked"))
            or any(type(v) is not int for v in row.get("counters", {}).values())):
        raise ValueError("invalid complete worker terminal")
    return "terminal"


def _worker(payload, start, stop):
    # A private subprocess interface is not an ungated coefficient evaluator.
    admitted = accepted_inputs(_repo(payload["prepared"]).parent, _repo(payload["acceptance"]), payload["acceptance_sha256"])
    if any(M.canonical_bytes(payload.get(k)) != M.canonical_bytes(value) for k, value in admitted.items()):
        raise ValueError("worker payload differs from admitted immutable inputs")
    _check_cache_prefix(_repo(payload["output"])/"bytecode-isolation"/f"worker-{start:04d}")
    _check_pins(payload["source_sha256"], payload["path_sha256"])
    raw = _repo(payload["prepared"]).read_bytes()
    if M.sha256(raw) != payload["prepared_sha256"]:
        raise ValueError("worker prepared data changed")
    prepared = M.restore_prepared(M.read_json(raw))
    binding = _binding(payload)
    def emit(record):
        sys.stdout.buffer.write(M.canonical_bytes(dict(record, **binding)))
        sys.stdout.buffer.flush()
    M.coefficient_range(prepared, start, stop, emit)
    _check_pins(payload["source_sha256"], payload["path_sha256"])
    sys.stdout.buffer.write(M.canonical_bytes(_terminal(start, stop, binding)))
    sys.stdout.buffer.flush()


def recover_stdout(path, start, stop, binding):
    """Longest contiguous valid durable prefix, never skipping invalid bytes."""
    import hashlib
    path = Path(path)
    result = {"records": [], "terminal": None, "completed_stop": start,
              "valid_prefix_bytes": 0, "valid_prefix_sha256": hashlib.sha256(b"").hexdigest(),
              "tail_error": None, "stdout_sha256": _sha(path) if path.is_file() else None}
    digest, offset = hashlib.sha256(), 0
    if not path.is_file():
        return result
    with path.open("rb") as stream:
        while True:
            line = stream.readline(MAX_LINE+1)
            if not line:
                break
            try:
                if len(line) > MAX_LINE or not line.endswith(b"\n") or result["terminal"] is not None:
                    raise ValueError("oversized/truncated/late stdout record")
                row = M.read_json(line)
                if M.canonical_bytes(row) != line:
                    raise ValueError("noncanonical stdout record")
                kind = _check_row(row, start, stop, result["completed_stop"], binding)
                digest.update(line)
                offset += len(line)
                meta = {"record": row, "prefix_bytes": offset, "prefix_sha256": digest.hexdigest()}
                if kind == "coefficient":
                    result["records"].append(meta)
                    result["completed_stop"] += 1
                else:
                    result["terminal"] = row
                result["valid_prefix_bytes"], result["valid_prefix_sha256"] = offset, digest.hexdigest()
            except BaseException as error:
                result["tail_error"] = f"{type(error).__name__}: {error}"
                break
    return result


def _progress(meta, binding, coefficient_path, coefficient_hash):
    return {"schema": "strict-memory-tail-progress-1", **binding,
            "index": meta["record"]["index"], "raw_prefix_bytes": meta["prefix_bytes"],
            "raw_prefix_sha256": meta["prefix_sha256"], "coefficient_path": coefficient_path,
            "coefficient_sha256": coefficient_hash}


def _drain(number, kind, stream, logfile, messages):
    try:
        with Path(logfile).open("xb") as target:
            while True:
                data = stream.readline(MAX_LINE+1)
                if not data:
                    break
                target.write(data)
                target.flush()
                os.fsync(target.fileno())
                messages.put((number, kind, data))
            messages.put((number, kind+"_eof", None))
    except BaseException:
        messages.put((number, "error", traceback.format_exc()))
    finally:
        stream.close()


def _normal_final(directory, number, binding, process, eof):
    start, stop = M.RANGES[number]
    folder = directory/"ranges"/f"{number:04d}"
    if (folder/"started.json").read_bytes() != M.canonical_bytes(_started(number, binding)):
        raise ValueError("range start receipt mismatch")
    prefix = recover_stdout(folder/"stdout", start, stop, binding)
    if (prefix["completed_stop"] != stop or prefix["terminal"] is None or prefix["tail_error"]
            or process.returncode != 0 or set(eof) != {"stdout", "stderr"}):
        raise ValueError("range did not finalize normally")
    coefficient_hashes, progress_hashes = {}, {}
    for offset, meta in enumerate(prefix["records"]):
        index = meta["record"]["index"]
        name = f"coefficients/{index:04d}.json"
        if (directory/name).read_bytes() != M.canonical_bytes(meta["record"]):
            raise ValueError("persisted coefficient differs from worker")
        coefficient_hashes[name] = _sha(directory/name)
        p = folder/"progress"/f"{offset:04d}.json"
        expected = _progress(meta, binding, name, coefficient_hashes[name])
        if p.read_bytes() != M.canonical_bytes(expected):
            raise ValueError("progress receipt differs from raw prefix")
        progress_hashes[p.relative_to(directory).as_posix()] = _sha(p)
    return {"schema": "strict-memory-tail-range-1", **binding, "status": "PASS", "range_index": number,
            "start": start, "stop": stop, "completed_stop": stop, "checked": stop-start,
            "counters": _aggregate(start, stop), "normal_finalization": True, "exit_code": 0,
            "stdout_eof": True, "stderr_eof": True, "stdout_sha256": prefix["stdout_sha256"],
            "stderr_sha256": _sha(folder/"stderr"), "coefficient_sha256": coefficient_hashes,
            "started_sha256": _sha(folder/"started.json"),
            "progress_sha256": progress_hashes, "terminal": prefix["terminal"]}


def _valid_final(directory, number, binding):
    class Exited:
        returncode = 0
    folder = directory/"ranges"/f"{number:04d}"
    try:
        value = _read(folder/"final.json")
        expected = _normal_final(directory, number, binding, Exited(), ("stdout", "stderr"))
        return value == expected and (folder/"final.json").read_bytes() == M.canonical_bytes(expected)
    except BaseException:
        return False


def _complete_range(directory, number, binding, reason):
    start, stop = M.RANGES[number]
    # One range is a separate failure boundary; the caller retains all64.
    folder = directory/"ranges"/f"{number:04d}"
    folder.mkdir(parents=True, exist_ok=True)
    if _valid_final(directory, number, binding):
        return (folder/"final.json").relative_to(directory).as_posix()
    prefix = recover_stdout(folder/"stdout", start, stop, binding)
    recovered = {}
    for meta in prefix["records"]:
        index, data = meta["record"]["index"], M.canonical_bytes(meta["record"])
        base = directory/"coefficients"/f"{index:04d}.recovered.json"
        path, suffix = base, 0
        while path.exists() and path.read_bytes() != data:
            suffix += 1
            path = base.with_name(base.stem+f"-{suffix:04d}"+base.suffix)
        if not path.exists():
            _write(path, data)
        recovered[path.relative_to(directory).as_posix()] = _sha(path)
    value = {"schema": "strict-memory-tail-range-recovery-1", **binding, "status": "FAIL",
             "range_index": number, "start": start, "stop": stop,
             "completed_stop": prefix["completed_stop"], "checked": prefix["completed_stop"]-start,
             "counters": _aggregate(start, prefix["completed_stop"]),
             "normal_finalization": False, "reason": str(reason),
             "stdout_sha256": prefix["stdout_sha256"], "stderr_sha256": _sha(folder/"stderr") if (folder/"stderr").is_file() else None,
             "valid_prefix_bytes": prefix["valid_prefix_bytes"], "valid_prefix_sha256": prefix["valid_prefix_sha256"],
             "tail_error": prefix["tail_error"], "worker_terminal": prefix["terminal"],
             "recovered_coefficient_sha256": recovered,
             "invalid_normal_sha256": _sha(folder/"final.json") if (folder/"final.json").is_file() else None}
    path, suffix = folder/"recovery.json", 0
    while path.exists():
        try:
            old = _read(path)
            # A later caller's explanation does not rewrite a valid recovery.
            old_without_reason, new_without_reason = dict(old), dict(value)
            old_without_reason.pop("reason", None)
            new_without_reason.pop("reason", None)
            if old_without_reason == new_without_reason and path.read_bytes() == M.canonical_bytes(old):
                break
        except BaseException:
            pass
        suffix += 1
        path = folder/f"recovery-{suffix:04d}.json"
    else:
        _json(path, value)
    return path.relative_to(directory).as_posix()


def _unavailable_range(directory, number, binding, reason, error):
    """Unknown work stays null, with an inline fallback if storage also fails."""
    start, stop = M.RANGES[number]
    value = {"schema": "strict-memory-tail-range-unavailable-1", **binding, "status": "FAIL",
             "range_index": number, "start": start, "stop": stop,
             "completed_stop": None, "checked": None, "counters": None,
             "normal_finalization": False, "reason": str(reason),
             "recovery_error": f"{type(error).__name__}: {error}",
             "range_path": f"ranges/{number:04d}", "work_count_status": "UNAVAILABLE"}
    try:
        path, suffix = directory/"range-unavailable"/f"{number:04d}.json", 0
        while path.exists():
            try:
                old = _read(path)
                old_without_reason, new_without_reason = dict(old), dict(value)
                old_without_reason.pop("reason", None)
                new_without_reason.pop("reason", None)
                if old_without_reason == new_without_reason and path.read_bytes() == M.canonical_bytes(old):
                    return path.relative_to(directory).as_posix()
            except BaseException:
                pass
            suffix += 1
            path = directory/"range-unavailable"/f"{number:04d}-{suffix:04d}.json"
        _json(path, value)
        return path.relative_to(directory).as_posix()
    except BaseException as write_error:
        return dict(value, receipt_write_error=f"{type(write_error).__name__}: {write_error}")


def complete_receipts(directory, binding, reason):
    """Retain64 dispositions despite range-local errors; recovered work fails."""
    directory, mapping = Path(directory), {}
    for number in range(64):
        try:
            result = _complete_range(directory, number, binding, reason)
        except BaseException as error:
            result = _unavailable_range(directory, number, binding, reason, error)
        mapping[f"{number:04d}"] = result
    return mapping


def _all_ranges_pass(directory, mapping):
    if set(mapping) != {f"{number:04d}" for number in range(64)}:
        return False
    for result in mapping.values():
        if type(result) is not str:
            return False
        try:
            value = _read(directory/result)
            if type(value) is not dict or value.get("status") != "PASS":
                return False
        except BaseException:
            return False
    return True


def _pool(payload, directory):
    import hashlib
    binding, messages = _binding(payload), queue.Queue()
    active, next_range = {}, 0
    try:
        while next_range < 64 or active:
            if time.monotonic() >= payload["deadline"]:
                raise TimeoutError("global coefficient deadline")
            while next_range < 64 and len(active) < WORKERS:
                number = next_range
                next_range += 1
                start, stop = M.RANGES[number]
                folder = directory/"ranges"/f"{number:04d}"
                folder.mkdir(parents=True, exist_ok=False)
                _json(folder/"started.json", _started(number, binding))
                prefix = directory/"bytecode-isolation"/f"worker-{start:04d}"
                prefix.mkdir(parents=True, exist_ok=False)
                command = _worker_command(directory, start, stop)
                process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=ROOT,
                                           env=_python_environment(prefix))
                state = {"process": process, "index": start, "terminal": False, "eof": set(),
                         "digest": hashlib.sha256(), "offset": 0, "threads": []}
                active[number] = state
                for kind, stream in (("stdout", process.stdout), ("stderr", process.stderr)):
                    thread = threading.Thread(target=_drain, args=(number, kind, stream, folder/kind, messages), daemon=True)
                    thread.start()
                    state["threads"].append(thread)
            try:
                number, kind, data = messages.get(timeout=0.05)
            except queue.Empty:
                number = None
            if number is not None:
                state, folder = active[number], directory/"ranges"/f"{number:04d}"
                start, stop = M.RANGES[number]
                if kind == "error":
                    raise OSError(data)
                if kind.endswith("_eof"):
                    state["eof"].add(kind[:-4])
                elif kind == "stdout":
                    if len(data) > MAX_LINE or not data.endswith(b"\n") or state["terminal"]:
                        raise ValueError("invalid worker stdout framing")
                    row = M.read_json(data)
                    if M.canonical_bytes(row) != data:
                        raise ValueError("worker noncanonical JSON")
                    row_kind = _check_row(row, start, stop, state["index"], binding)
                    state["digest"].update(data)
                    state["offset"] += len(data)
                    if row_kind == "coefficient":
                        index = state["index"]
                        name = f"coefficients/{index:04d}.json"
                        _write(directory/name, data)
                        meta = {"record": row, "prefix_bytes": state["offset"], "prefix_sha256": state["digest"].hexdigest()}
                        _json(folder/"progress"/f"{index-start:04d}.json", _progress(meta, binding, name, M.sha256(data)))
                        state["index"] += 1
                    else:
                        state["terminal"] = True
            for number, state in list(active.items()):
                process = state["process"]
                if process.poll() is not None and state["eof"] == {"stdout", "stderr"}:
                    for thread in state["threads"]:
                        thread.join(timeout=1)
                    _json(directory/"ranges"/f"{number:04d}"/"final.json",
                          _normal_final(directory, number, binding, process, state["eof"]))
                    del active[number]
    finally:
        for state in active.values():
            process = state["process"]
            if process.poll() is None:
                process.kill()
            process.wait(timeout=5)
            for thread in state["threads"]:
                thread.join(timeout=2)


def _worker_command(directory, start, stop):
    return [sys.executable, "-B", "-X", "pycache_prefix="+str((directory/"bytecode-isolation"/f"worker-{start:04d}").resolve()),
            str(Path(__file__).resolve()), "_worker", "--lock", str(directory/"lock.json"),
            "--lock-sha256", _sha(directory/"lock.json"), "--start", str(start), "--stop", str(stop)]


def _identity(payload):
    if not payload.get("source_sha256"):
        return {"status": "NOT_ADMITTED", "checked_direct_identities": 0}
    try:
        _check_pins(payload.get("source_sha256", {}), payload.get("path_sha256", {}))
        return {"status": "PASS", "checked_direct_identities": len(payload.get("source_sha256", {}))+len(payload.get("path_sha256", {}))}
    except BaseException:
        return {"status": "FAIL", "error": traceback.format_exc()}


def _coordinator(payload, barrier):
    barrier.wait()
    directory = _repo(payload["output"])
    _check_cache_prefix(directory/"bytecode-isolation"/"coordinator")
    _check_pins(payload["source_sha256"], payload["path_sha256"])
    _json(directory/"checkout-before.json", _checkout())
    _json(directory/"preservation-before.json", PRESERVE.verify_preservation(ROOT, payload["preservation_seals"]))
    _pool(payload, directory)
    mapping = complete_receipts(directory, _binding(payload), "coordinator completion")
    if not _all_ranges_pass(directory, mapping):
        raise ValueError("not all64 coefficient ranges finalized normally")
    prepared = M.restore_prepared(_read(_repo(payload["prepared"])))
    records = [_read(directory/"coefficients"/f"{i:04d}.json") for i in range(185)]
    certificate = M.certificate(prepared, records)
    certificate["upstream_sha256"] = {CERTIFICATE: PINNED[CERTIFICATE], CENSUS: PINNED[CENSUS],
                                     "results_audit": PINNED["engine/docs/AUDIT_STRICT_FULL_MEMORY_RESULTS_V2.md"]}
    _json(directory/"certificate.json", certificate)
    _check_pins(payload["source_sha256"], payload["path_sha256"])
    _json(directory/"preservation-after.json", PRESERVE.verify_preservation(ROOT, payload["preservation_seals"]))
    _json(directory/"checkout-after.json", _checkout())
    identity = _identity(payload)
    if identity["status"] != "PASS" or time.monotonic() >= payload["deadline"]:
        raise ValueError("late or changed scientific completion")
    _json(directory/"worker-completion.json", {"schema": "strict-memory-tail-worker-completion-1", "status": "PASS",
                                              "certificate_sha256": _sha(directory/"certificate.json"),
                                              "metadata_sha256": {name: _sha(directory/name) for name in
                                                                  ("preservation-before.json", "preservation-after.json", "checkout-before.json", "checkout-after.json")},
                                              "post_run_identity": identity, "range_receipts": mapping})


def _checkout():
    baseline = _read(_repo(BASELINE))
    def git(*arguments):
        return subprocess.run(["git", *arguments], cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                              check=True, timeout=20).stdout
    head = git("rev-parse", "HEAD").decode("ascii").strip()
    staged = git("diff", "--cached", "--name-only").decode("utf-8").splitlines()
    diff_hash = M.sha256(git("diff", "--binary"))
    if (head != baseline["git_head"] or staged != baseline["staged_paths"]
            or diff_hash != baseline["prior_tracked_cleanup_diff_sha256"]):
        raise ValueError("baseline HEAD/index/tracked cleanup differs")
    return {"schema": "strict-memory-tail-checkout-1", "status": "PASS", "git_head": head,
            "staged_paths": staged, "tracked_cleanup_diff_sha256": diff_hash}


def _artifacts(directory):
    directory = Path(directory)
    hashes, errors = {}, {}
    def walk_error(error):
        errors[str(getattr(error, "filename", directory))] = f"{type(error).__name__}: {error}"
    for folder, _, names in os.walk(directory, onerror=walk_error):
        for name in sorted(names):
            path = Path(folder)/name
            if path == directory/"report.json":
                continue
            relative = path.relative_to(directory).as_posix()
            try:
                hashes[relative] = _sha(path)
            except BaseException as error:
                errors[relative] = f"{type(error).__name__}: {error}"
    return {"sha256": hashes, "errors": errors}


def _optional_json(path):
    if not Path(path).is_file():
        return None
    try:
        value = _read(path)
        expected = {"preservation-before.json": PRESERVE.SCHEMA,
                    "preservation-after.json": PRESERVE.SCHEMA,
                    "checkout-before.json": "strict-memory-tail-checkout-1",
                    "checkout-after.json": "strict-memory-tail-checkout-1"}.get(Path(path).name)
        if (type(value) is not dict or (expected is not None and
                (value.get("schema") != expected or value.get("status") not in ("PASS", "FAIL")))):
            raise ValueError("retained metadata object/schema/status required")
        return value
    except BaseException:
        result = {"status": "INVALID_RETAINED_JSON", "error": traceback.format_exc(), "sha256": None}
        try:
            result["sha256"] = _sha(path)
        except BaseException:
            result["hash_error"] = traceback.format_exc()
        return result


def run(prepared, acceptance, acceptance_sha256, output):
    began = time.monotonic()
    directory = Path(output).resolve()
    _relative(directory)
    directory.mkdir(parents=True, exist_ok=False)
    payload = {"source_sha256": dict(PINNED), "path_sha256": {}, "preservation_seals": []}
    supervision, error, completion, mapping = None, None, None, {}
    try:
        payload["preservation_seals"] = _read(_repo(BASELINE))["prior_seals"]
        payload["source_sha256"] = _source_pins()
        payload["path_sha256"] = {str(Path(sys.executable).resolve()): _sha(sys.executable)}
        payload = accepted_inputs(prepared, acceptance, acceptance_sha256)
        payload.update(schema="strict-memory-tail-execution-lock-1", output=_relative(directory),
                       created_utc=datetime.now(timezone.utc).isoformat(), canonical_adoption=False)
        _json(directory/"lock.json", payload)
        # The coordinator-owned stdlib launcher must establish a fresh prefix
        # before importing this runner. Child isolation cannot authenticate
        # arbitrary modules that a caller imported earlier from an old cache.
        if sys.pycache_prefix is None:
            raise ValueError("use the accepted fresh-prefix launch driver")
        _check_cache_prefix(sys.pycache_prefix)
        _json(directory/"bytecode-isolation.json", {"schema": "strict-memory-tail-bytecode-isolation-1",
                                                   "entry_prefix": sys.pycache_prefix,
                                                   "coordinator_prefix": "bytecode-isolation/coordinator",
                                                   "worker_prefix_pattern": "bytecode-isolation/worker-{start:04d}",
                                                   "cleared_overrides": list(PYTHON_OVERRIDES), "writes_disabled": True})
        with _cache_isolation(directory/"bytecode-isolation"/"coordinator"):
            supervision = SUPERVISOR.supervise(_coordinator, payload, CEILING, MEMORY)
        if supervision["failure"] or supervision["exit_code"] != 0 or not supervision["actual_job_assignment"]:
            raise RuntimeError("supervised computation failed: "+str(supervision))
        completion = _read(directory/"worker-completion.json")
        if (completion.get("status") != "PASS" or completion.get("certificate_sha256") != _sha(directory/"certificate.json")
                or completion.get("post_run_identity", {}).get("status") != "PASS"):
            raise ValueError("invalid scientific completion receipt")
        if completion.get("metadata_sha256") != {name: _sha(directory/name) for name in
                                                 ("preservation-before.json", "preservation-after.json", "checkout-before.json", "checkout-after.json")}:
            raise ValueError("scientific completion metadata differs")
    except BaseException:
        error = traceback.format_exc()
    cleanup_began = time.monotonic()
    post_identity = _identity(payload)
    if post_identity["status"] != "PASS":
        error = (error or "")+"\npost-run direct identity failure"
    finalization_error = None
    try:
        mapping = complete_receipts(directory, _binding(payload), error or "normal completion")
        if not _all_ranges_pass(directory, mapping):
            error = (error or "")+"\nnot all64 ranges accepted"
    except BaseException:
        finalization_error = traceback.format_exc()
        error = (error or "")+"\nreceipt finalization failed"
    preservation_after = None
    if (directory/"preservation-after.json").is_file():
        preservation_after = _optional_json(directory/"preservation-after.json")
        if preservation_after.get("status") != "PASS":
            error = (error or "")+"\ninvalid preservation completion"
    if preservation_after is None and payload.get("preservation_seals"):
        # Metadata-only failure cleanup; no coefficients are evaluated here.
        try:
            preservation_after = PRESERVE.verify_preservation(ROOT, payload["preservation_seals"])
            _json(directory/"preservation-after-recovery.json", preservation_after)
        except BaseException:
            preservation_after = {"status": "FAIL", "error": traceback.format_exc()}
            error = (error or "")+"\npost-run preservation failed"
    if (type(preservation_after) is not dict or preservation_after.get("schema") != PRESERVE.SCHEMA
            or preservation_after.get("status") != "PASS"):
        error = (error or "")+"\ncomplete preservation not established"
    inventory = _artifacts(directory)
    if inventory["errors"]:
        error = (error or "")+"\nretained artifact inventory has read/hash errors"
    optional = {name: _optional_json(directory/name) for name in
                ("preservation-before.json", "checkout-before.json", "checkout-after.json")}
    if any(value and value.get("status") == "INVALID_RETAINED_JSON" for value in optional.values()):
        error = (error or "")+"\nretained optional metadata is invalid or unreadable"
    report = {"schema": "strict-memory-tail-execution-result-1", "law_id": M.LAW_ID,
              "status": "PASS_SCOPED_EXACT_TAIL_ENERGY" if error is None else "MEMORY_TAIL_INCOMPLETE",
              "error": error, "receipt_finalization_error": finalization_error,
              "lock_sha256": inventory["sha256"].get("lock.json"),
              "range_receipts": mapping, "certificate_sha256": inventory["sha256"].get("certificate.json"),
              "source_sha256": payload.get("source_sha256", {}), "path_sha256": payload.get("path_sha256", {}),
              "preservation_seals": payload.get("preservation_seals", []), "post_run_identity": post_identity,
              "preservation_before": optional["preservation-before.json"],
              "checkout_before": optional["checkout-before.json"],
              "checkout_after": optional["checkout-after.json"],
              "preservation_after": preservation_after, "supervision": supervision,
              "worker_completion": completion, "artifact_sha256": inventory["sha256"],
              "artifact_inventory_errors": inventory["errors"],
              "end_to_end_seconds": time.monotonic()-began, "cleanup_seconds": time.monotonic()-cleanup_began,
              "uniform_absolute_memory_tail": False, "continuum_recovered": False, "canonical_adoption": False}
    _json(directory/"report.json", report)
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    p = sub.add_parser("prepare")
    for name in ("certificate", "census", "output"):
        p.add_argument("--"+name, required=True)
    for name in ("accepted_inputs", "run"):
        p = sub.add_parser(name)
        for key in ("prepared", "acceptance", "acceptance-sha256"):
            p.add_argument("--"+key, required=True)
        if name == "run":
            p.add_argument("--output", required=True)
    p = sub.add_parser("_worker", help=argparse.SUPPRESS)
    for name in ("lock", "lock-sha256", "start", "stop"):
        p.add_argument("--"+name, required=True, type=int if name in ("start", "stop") else str)
    args = parser.parse_args()
    if args.command == "prepare":
        result = prepare(args.certificate, args.census, args.output)
    elif args.command == "accepted_inputs":
        result = accepted_inputs(args.prepared, args.acceptance, args.acceptance_sha256)
    elif args.command == "run":
        result = run(args.prepared, args.acceptance, args.acceptance_sha256, args.output)
    else:
        raw = Path(args.lock).read_bytes()
        if M.sha256(raw) != _digest(args.lock_sha256):
            raise ValueError("worker execution lock identity changed")
        payload = M.read_json(raw)
        if payload.get("schema") != "strict-memory-tail-execution-lock-1" or (args.start, args.stop) not in M.RANGES:
            raise ValueError("worker requires registered locked range")
        _worker(payload, args.start, args.stop)
        return
    print(M.canonical_bytes(result).decode("ascii"), end="")


if __name__ == "__main__":
    main()
