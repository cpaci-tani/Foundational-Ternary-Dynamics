"""Capture complete pytest evidence without assigning physical certification.

The caller freezes the test list and all direct sources. This component owns
new execution artifacts only; prior evidence is verified through its seals.
"""
from __future__ import annotations

from collections import Counter
import hashlib
import importlib.metadata
import inspect
import json
import os
from pathlib import Path
import platform
import subprocess
import sys
import time
import traceback
import xml.etree.ElementTree as ET

from .recovery_evidence_chain import sha256, verify_preservation

PLUGIN = "scripts.phi_v2_lattice.recovery_regression"
MODULE = "scripts/phi_v2_lattice/recovery_regression.py"
CHAIN = "scripts/phi_v2_lattice/recovery_evidence_chain.py"
PREFIX = "FTD_RECOVERY_REGRESSION_"
PROPERTY = "ftd_recovery_nodeid"
_DISCOVERED, _DESELECTED, _REPORTS = [], [], []


def _bytes(value):
    return (json.dumps(value, sort_keys=True, separators=(",", ":"),
                       ensure_ascii=True, allow_nan=False)+"\n").encode("ascii")


def _write(path, value):
    with Path(path).open("xb") as stream:
        stream.write(value if isinstance(value, bytes) else _bytes(value))
        stream.flush()
        os.fsync(stream.fileno())


def _read(path):
    def unique(pairs):
        result = {}
        for name, value in pairs:
            if name in result:
                raise ValueError("duplicate JSON key")
            result[name] = value
        return result
    def invalid(value):
        raise ValueError("nonfinite JSON value: "+value)
    return json.loads(Path(path).read_bytes(), object_pairs_hook=unique, parse_constant=invalid)


def _path(root, name):
    if (type(name) is not str or not name or "\\" in name or ":" in name
            or any(part in ("", ".", "..") for part in name.split("/"))):
        raise ValueError("expected canonical repository-relative path")
    result = (root/name).resolve()
    if not result.is_relative_to(root) or not result.is_file():
        raise ValueError("missing or external input: "+name)
    return result


def _digest(value):
    if type(value) is not str or len(value) != 64 or any(c not in "0123456789abcdef" for c in value):
        raise ValueError("invalid SHA256")
    return value


def pytest_sessionstart(session):
    _DISCOVERED.clear()
    _DESELECTED.clear()
    _REPORTS.clear()


def pytest_itemcollected(item):
    _DISCOVERED.append(item.nodeid)
    if any(name == PROPERTY for name, _ in item.user_properties):
        raise ValueError("reserved recovery testcase property already present")
    item.user_properties.append((PROPERTY, item.nodeid))


def pytest_deselected(items):
    _DESELECTED.extend(item.nodeid for item in items)


def _plugins(config):
    result = []
    for name, plugin in config.pluginmanager.list_name_plugin():
        if plugin is None:
            continue
        module = plugin if inspect.ismodule(plugin) else inspect.getmodule(type(plugin))
        path = getattr(module, "__file__", None)
        path = str(Path(path).resolve()) if path else None
        result.append({"name": str(name), "module": getattr(module, "__name__", None),
                       "path": path, "sha256": sha256(path) if path and Path(path).is_file() else None})
    return sorted(result, key=lambda row: row["name"])


def pytest_collection_finish(session):
    output = os.environ.get(PREFIX+"OUTPUT")
    if output:
        _write(Path(output)/"collection.json", {
            "schema": "strict-regression-collection-1", "discovered": _DISCOVERED,
            "nodes": [item.nodeid for item in session.items], "deselected": _DESELECTED,
            "plugins": _plugins(session.config), "sys_path": list(sys.path),
            "optimize": sys.flags.optimize, "cache_prefix": sys.pycache_prefix})


def pytest_runtest_logreport(report):
    _REPORTS.append({"nodeid": report.nodeid, "when": report.when,
                     "outcome": report.outcome, "wasxfail": getattr(report, "wasxfail", None)})


def pytest_sessionfinish(session, exitstatus):
    output = os.environ.get(PREFIX+"OUTPUT")
    if output:
        _write(Path(output)/"reports.json", {"schema": "strict-regression-reports-1",
            "exitstatus": int(exitstatus), "reports": _REPORTS,
            "plugins": _plugins(session.config), "sys_path": list(sys.path),
            "optimize": sys.flags.optimize, "cache_prefix": sys.pycache_prefix})


def test_environment(environment, output, mode):
    """Return a copied child environment; never mutate the caller's mapping."""
    if mode not in ("collection", "execution"):
        raise ValueError("unknown regression subprocess mode")
    result = dict(environment)
    removed = sorted(key for key in result if key.startswith(PREFIX) or key in (
        "PYTHONPATH", "PYTHONHOME", "PYTHONSTARTUP", "PYTHONPYCACHEPREFIX", "PYTEST_CURRENT_TEST",
        "PYTHONOPTIMIZE", "PYTHONINSPECT", "PYTHONSAFEPATH"))
    for key in removed:
        result.pop(key)
    overrides = {"PYTEST_ADDOPTS": "", "PYTEST_PLUGINS": "", "PYTEST_DISABLE_PLUGIN_AUTOLOAD": "1",
                 "PYTHONDONTWRITEBYTECODE": "1", "PYTHONHASHSEED": "0",
                 "PYTHONPYCACHEPREFIX": str(Path(output).resolve()/"fresh-pycache"),
                 PREFIX+"OUTPUT": str(Path(output).resolve()), PREFIX+"MODE": mode}
    result.update(overrides)
    return result, {"removed": removed, "set": overrides}


def _collection(value, expected_files):
    if (type(value) is not dict or value.get("schema") != "strict-regression-collection-1"
            or value.get("deselected") != [] or type(value.get("optimize")) is not int or value["optimize"] != 0):
        raise ValueError("invalid or deselected test collection")
    nodes = value.get("nodes")
    if (type(nodes) is not list or not nodes or any(type(n) is not str for n in nodes)
            or len(nodes) != len(set(nodes)) or value.get("discovered") != nodes
            or {n.split("::", 1)[0] for n in nodes} != set(expected_files)):
        raise ValueError("missing, duplicate, suppressed or reordered test collection")
    return nodes


def validate_test_evidence(collection, actual_collection, reports, junit_path, expected_files):
    """Accept only complete passing runs. Failure records remain retained raw."""
    if (not isinstance(expected_files, (list, tuple)) or not expected_files
            or len(expected_files) != len(set(expected_files))):
        raise ValueError("unique registered test files required")
    nodes = _collection(collection, expected_files)
    if _collection(actual_collection, expected_files) != nodes:
        raise ValueError("execution collection differs from discovery")
    if (type(reports) is not dict or reports.get("schema") != "strict-regression-reports-1"
            or type(reports.get("exitstatus")) is not int or reports["exitstatus"] != 0
            or type(reports.get("optimize")) is not int or reports["optimize"] != 0
            or type(reports.get("reports")) is not list):
        raise ValueError("test session failed or execution report missing")
    expected = Counter((node, phase) for node in nodes for phase in ("setup", "call", "teardown"))
    actual = Counter()
    for row in reports["reports"]:
        if (type(row) is not dict or row.get("outcome") != "passed" or row.get("wasxfail") is not None
                or type(row.get("nodeid")) is not str or type(row.get("when")) is not str):
            raise ValueError("nonpassing or malformed phase report")
        actual[row["nodeid"], row["when"]] += 1
    if actual != expected:
        raise ValueError("missing, duplicate or foreign test phase")
    document = ET.parse(junit_path).getroot()
    if document.tag not in ("testsuites", "testsuite"):
        raise ValueError("unrecognized JUnit root")
    cases = list(document.iter("testcase"))
    identities = []
    for case in cases:
        properties = [p.get("value") for p in case.findall("./properties/property") if p.get("name") == PROPERTY]
        if len(properties) != 1 or any(case.find(tag) is not None for tag in ("failure", "error", "skipped")):
            raise ValueError("failed or unidentified JUnit testcase")
        identities.append(properties[0])
    if Counter(identities) != Counter(nodes):
        raise ValueError("JUnit node identities differ from actual execution")
    for suite in document.iter():
        if suite.tag not in ("testsuites", "testsuite"):
            continue
        count = len(list(suite.iter("testcase")))
        for key, expected_count in (("tests", count), ("failures", 0), ("errors", 0), ("skipped", 0)):
            value = suite.get(key)
            if suite.tag == "testsuites" and value is None:
                continue
            if value != str(expected_count):
                raise ValueError("JUnit suite aggregates differ from cases")
    return {"tests": len(nodes), "passed": len(nodes), "failures": 0, "errors": 0,
            "skipped": 0, "phase_reports": 3*len(nodes), "registered_files": len(expected_files)}


def _checkout(root):
    if not (root/".git").exists():
        return {"synthetic_no_repository": True}
    def git(*args):
        return subprocess.check_output(["git", *args], cwd=root, stderr=subprocess.PIPE)
    return {"head": git("rev-parse", "HEAD").decode().strip(),
            "index": git("diff", "--cached", "--name-status").decode().splitlines(),
            "tracked_diff_sha256": hashlib.sha256(git("diff", "--binary", "HEAD", "--")).hexdigest(),
            "status": git("status", "--short", "--untracked-files=all").decode().splitlines()}


def _applicable_controls(root, test_files):
    result = set()
    for name in test_files:
        directory = _path(root, name).parent
        while directory.is_relative_to(root):
            for filename in ("conftest.py", "pytest.ini", ".pytest.ini", "pyproject.toml", "tox.ini", "setup.cfg"):
                path = directory/filename
                if path.is_file():
                    result.add(path.relative_to(root).as_posix())
            if directory == root:
                break
            directory = directory.parent
    return result


def _check_plugins(root, record, pins):
    plugins = record.get("plugins")
    if type(plugins) is not list or not plugins:
        raise ValueError("loaded plugin inventory missing")
    for row in plugins:
        if type(row) is not dict or type(row.get("name")) is not str:
            raise ValueError("malformed plugin inventory")
        if row.get("path"):
            path = Path(row["path"]).resolve()
            if path.is_relative_to(root):
                name = path.relative_to(root).as_posix()
                if name not in pins or row.get("sha256") != pins[name]:
                    raise ValueError("unexpected or changed local pytest plugin: "+name)


def _invoke(command, root, environment, directory):
    with (directory/"stdout").open("xb") as stdout, (directory/"stderr").open("xb") as stderr:
        try:
            return subprocess.run(command, cwd=root, env=environment, stdout=stdout, stderr=stderr).returncode
        finally:
            for stream in (stdout, stderr):
                stream.flush()
                os.fsync(stream.fileno())


def _observed_outcomes(output):
    """Describe retained partial/failing reports without upgrading their status."""
    result = {}
    for mode in ("collection", "execution"):
        path = output/mode/"reports.json"
        if not path.is_file():
            result[mode] = {"available": False}
            continue
        try:
            value = _read(path)
            rows = value["reports"]
            outcomes = Counter((row["when"], row["outcome"]) for row in rows)
            result[mode] = {"available": True, "exitstatus": value["exitstatus"],
                "phase_reports": len(rows), "nodes_with_reports": len({row["nodeid"] for row in rows}),
                "phase_outcomes": [{"when": phase, "outcome": outcome, "count": count}
                                   for (phase, outcome), count in sorted(outcomes.items())]}
        except BaseException:
            result[mode] = {"available": False, "retained_report_unreadable": traceback.format_exc()}
    return result


def run(root, output, test_files, source_sha256, preservation_seals):
    """Run a caller-frozen matrix and retain a report even after gate failures."""
    original_root = Path(root)
    root, output = original_root.resolve(), Path(output).resolve()
    if (not original_root.is_absolute() or not root.is_dir() or output == root
            or not output.is_relative_to(root) or output.exists()):
        raise ValueError("existing absolute root and new exclusive internal output required")
    output.mkdir(parents=True)
    began = time.monotonic()
    failures, post_errors, pins, drift, receipts = [], [], {}, [], {}
    counts, exit_codes, before, after = {}, {}, None, None
    admitted = False
    try:
        if sys.flags.optimize != 0:
            raise ValueError("coordinator must start without Python optimization")
        if (not isinstance(test_files, (tuple, list)) or not test_files
                or any(type(name) is not str for name in test_files)
                or len(test_files) != len(set(test_files)) or type(source_sha256) is not dict
                or not isinstance(preservation_seals, (tuple, list))):
            raise ValueError("invalid test/source/preservation registration")
        required = {MODULE, CHAIN, *test_files} | _applicable_controls(root, test_files)
        if not required.issubset(source_sha256):
            raise ValueError("missing test, component or config/conftest input pin")
        for name, executing in ((MODULE, Path(__file__)), (CHAIN, Path(__file__).with_name("recovery_evidence_chain.py"))):
            if source_sha256[name] != sha256(executing):
                raise ValueError("executing coordinator component differs from source lock")
        physical_paths = set()
        for name, digest in source_sha256.items():
            path = _path(root, name)
            if path.is_relative_to(output) or path in physical_paths:
                raise ValueError("output overlap or duplicate physical input")
            physical_paths.add(path)
            pins[name] = _digest(digest)
            if sha256(path) != digest:
                raise ValueError("frozen input changed: "+name)
        if preservation_seals:
            receipts["before"] = verify_preservation(root, preservation_seals)
        before = _checkout(root)
        config = output/"pytest.ini"
        _write(config, b"[pytest]\n")
        command = [sys.executable, "-m", "pytest", "--rootdir="+str(root), "--confcutdir="+str(root),
                   "-c", str(config), "-o", "addopts=", "-p", "no:cacheprovider", "-p", PLUGIN,
                   *test_files, "-q"]
        commands, environments, overrides = {}, {}, {}
        for mode in ("collection", "execution"):
            directory = output/mode
            directory.mkdir()
            environments[mode], overrides[mode] = test_environment(os.environ, directory, mode)
            commands[mode] = command+(["--collect-only"] if mode == "collection" else ["--junitxml="+str(directory/"pytest.xml")])
        lock = {"schema": "strict-regression-lock-1", "test_files": list(test_files),
                "source_sha256": pins, "preservation_seals": list(preservation_seals),
                "commands": commands, "environment_overrides": overrides, "checkout": before,
                "python": sys.version, "python_sha256": sha256(sys.executable), "coordinator_optimize": sys.flags.optimize,
                "pytest_version": importlib.metadata.version("pytest"), "platform": platform.platform(),
                "generated_config": config.relative_to(output).as_posix(), "generated_config_sha256": sha256(config),
                "canonical_adoption": False}
        _write(output/"lock.json", lock)
        admitted = True
        exit_codes["collection"] = _invoke(commands["collection"], root, environments["collection"], output/"collection")
        collection = _read(output/"collection/collection.json")
        _collection(collection, test_files)
        for path in (output/"collection/collection.json", output/"collection/reports.json"):
            record = _read(path)
            _check_plugins(root, record, pins)
            if record.get("cache_prefix") != environments["collection"]["PYTHONPYCACHEPREFIX"]:
                raise ValueError("discovery interpreter cache prefix differs")
        if exit_codes["collection"] != 0 or _read(output/"collection/reports.json").get("exitstatus") != 0:
            raise ValueError("discovery subprocess did not pass")
        if sha256(config) != lock["generated_config_sha256"]:
            raise ValueError("generated config changed before test execution")
        exit_codes["execution"] = _invoke(commands["execution"], root, environments["execution"], output/"execution")
        actual, reports = _read(output/"execution/collection.json"), _read(output/"execution/reports.json")
        for record in (actual, reports):
            _check_plugins(root, record, pins)
            if record.get("cache_prefix") != environments["execution"]["PYTHONPYCACHEPREFIX"]:
                raise ValueError("execution interpreter cache prefix differs")
        counts = validate_test_evidence(collection, actual, reports, output/"execution/pytest.xml", test_files)
        if exit_codes["execution"] != 0:
            raise ValueError("test subprocess did not pass")
        if sha256(config) != lock["generated_config_sha256"]:
            raise ValueError("generated config changed during tests")
    except BaseException:
        failures.append(traceback.format_exc())
    # Every postcheck is attempted independently, including after test failure.
    for name, digest in pins.items():
        try:
            if sha256(root/name) != digest:
                drift.append(name)
        except BaseException:
            drift.append(name)
            post_errors.append({"input": name, "error": traceback.format_exc()})
    try:
        if preservation_seals:
            receipts["after"] = verify_preservation(root, preservation_seals)
    except BaseException:
        post_errors.append({"check": "preservation", "error": traceback.format_exc()})
    try:
        after = _checkout(root)
    except BaseException:
        post_errors.append({"check": "checkout", "error": traceback.format_exc()})
    artifacts = {}
    try:
        for path in sorted(output.rglob("*")):
            if path.is_file():
                try:
                    artifacts[path.relative_to(output).as_posix()] = sha256(path)
                except BaseException:
                    post_errors.append({"artifact": str(path), "error": traceback.format_exc()})
    except BaseException:
        post_errors.append({"check": "artifact inventory", "error": traceback.format_exc()})
    result = {"schema": "strict-regression-result-1",
              "regression_passed": admitted and not failures and not post_errors and not drift,
              "admitted": admitted, "counts": counts, "exit_codes": exit_codes, "failures": failures,
              "observed_outcomes": _observed_outcomes(output),
              "source_drift": drift, "postcheck_errors": post_errors, "preservation": receipts,
              "checkout_before": before, "checkout_after": after, "checkout_unchanged": before == after,
              "artifact_sha256": artifacts, "elapsed_seconds": time.monotonic()-began,
              "scientific_completion_evaluated": False, "full_physics_recovery": False, "canonical_adoption": False}
    _write(output/"report.json", result)
    return result
