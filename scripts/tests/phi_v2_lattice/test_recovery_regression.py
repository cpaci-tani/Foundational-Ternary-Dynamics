"""Adversarial collection/accounting controls with actual isolated pytest runs."""
from copy import deepcopy
from pathlib import Path
import os
import py_compile
import shutil
import subprocess
import sys
import xml.etree.ElementTree as ET

import pytest

from scripts.phi_v2_lattice import recovery_regression as R


FILES = ("tests/test_sample.py",)
NODES = [FILES[0]+"::test_one", FILES[0]+"::test_two"]


def evidence(tmp_path):
    collection = {"schema": "strict-regression-collection-1", "nodes": list(NODES),
                  "discovered": list(NODES), "deselected": [], "optimize": 0}
    reports = {"schema": "strict-regression-reports-1", "exitstatus": 0, "optimize": 0,
               "reports": [{"nodeid": n, "when": phase, "outcome": "passed", "wasxfail": None}
                           for n in NODES for phase in ("setup", "call", "teardown")]}
    root = ET.Element("testsuites")
    suite = ET.SubElement(root, "testsuite", tests="2", failures="0", errors="0", skipped="0")
    for n in NODES:
        case = ET.SubElement(suite, "testcase", name=n)
        props = ET.SubElement(case, "properties")
        ET.SubElement(props, "property", name=R.PROPERTY, value=n)
    path = tmp_path/"junit.xml"
    ET.ElementTree(root).write(path, encoding="utf-8")
    return collection, deepcopy(collection), reports, path


def test_complete_node_phase_and_junit_accounting(tmp_path):
    assert R.validate_test_evidence(*evidence(tmp_path), FILES) == {
        "tests": 2, "passed": 2, "failures": 0, "errors": 0, "skipped": 0,
        "phase_reports": 6, "registered_files": 1}


@pytest.mark.parametrize("mutation", ["suppression", "duplicate_node", "reorder", "deselected",
    "different_execution", "missing_phase", "duplicate_phase", "foreign_phase", "foreign_node",
    "skipped_phase", "xfail", "session_failure", "session_bool"])
def test_collection_and_execution_faults_cannot_pass(tmp_path, mutation):
    c, a, r, p = evidence(tmp_path)
    if mutation == "suppression":
        c["nodes"].pop()
        a["nodes"].pop()
    elif mutation == "duplicate_node":
        c["nodes"].append(NODES[0])
    elif mutation == "reorder":
        c["nodes"].reverse()
    elif mutation == "deselected":
        c["deselected"].append("tests/test_sample.py::hidden")
    elif mutation == "different_execution":
        a["nodes"].reverse()
        a["discovered"].reverse()
    elif mutation == "missing_phase":
        r["reports"].pop()
    elif mutation == "duplicate_phase":
        r["reports"].append(r["reports"][0])
    elif mutation == "foreign_phase":
        r["reports"][0]["when"] = "hidden"
    elif mutation == "foreign_node":
        r["reports"][0]["nodeid"] = "foreign"
    elif mutation == "skipped_phase":
        r["reports"][0]["outcome"] = "skipped"
    elif mutation == "xfail":
        r["reports"][0]["wasxfail"] = "expected"
    elif mutation == "session_failure":
        r["exitstatus"] = 1
    else:
        r["exitstatus"] = False
    with pytest.raises(ValueError):
        R.validate_test_evidence(c, a, r, p, FILES)


@pytest.mark.parametrize("mutation", ["missing_property", "duplicate_property", "wrong_identity",
    "failure", "error", "skip", "aggregate", "duplicate_case", "missing_case"])
def test_junit_faults_cannot_pass(tmp_path, mutation):
    c, a, r, p = evidence(tmp_path)
    tree = ET.parse(p)
    suite, case = tree.getroot()[0], tree.getroot()[0][0]
    if mutation == "missing_property":
        case.remove(case[0])
    elif mutation == "duplicate_property":
        case[0].append(deepcopy(case[0][0]))
    elif mutation == "wrong_identity":
        case[0][0].set("value", "unknown")
    elif mutation in ("failure", "error", "skip"):
        ET.SubElement(case, "skipped" if mutation == "skip" else mutation)
    elif mutation == "aggregate":
        suite.set("tests", "100")
    elif mutation == "duplicate_case":
        suite.append(deepcopy(case))
    else:
        suite.remove(case)
    tree.write(p)
    with pytest.raises(ValueError):
        R.validate_test_evidence(c, a, r, p, FILES)


def test_environment_clears_filters_without_mutating_parent(tmp_path):
    original = {"PYTEST_ADDOPTS": "-k absent", "PYTEST_PLUGINS": "foreign", "PYTHONPATH": "outside",
                "PYTHONHOME": "outside", "PYTHONPYCACHEPREFIX": "old-cache", "PYTHONOPTIMIZE": "2",
                "PYTHONINSPECT": "1", "PYTHONSAFEPATH": "1", R.PREFIX+"OUTPUT": "old", "KEEP": "yes"}
    prior = dict(original)
    child, changes = R.test_environment(original, tmp_path, "execution")
    assert original == prior and child["KEEP"] == "yes"
    assert child["PYTEST_ADDOPTS"] == child["PYTEST_PLUGINS"] == ""
    assert "PYTHONPATH" not in child and "PYTHONHOME" not in child
    assert not {"PYTHONOPTIMIZE", "PYTHONINSPECT", "PYTHONSAFEPATH"}.intersection(child)
    assert child[R.PREFIX+"OUTPUT"] == str(tmp_path.resolve())
    assert changes["set"]["PYTEST_DISABLE_PLUGIN_AUTOLOAD"] == "1"
    assert child["PYTHONPYCACHEPREFIX"] == str(tmp_path.resolve()/"fresh-pycache")


def project(tmp_path, test_text="def test_one(): pass\ndef test_two(): pass\n", conftest=None):
    root = tmp_path/"repository"
    real = Path(R.__file__).resolve().parents[2]
    for name in (R.MODULE, R.CHAIN):
        destination = root/name
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(real/name, destination)
    target = root/FILES[0]
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(test_text, encoding="utf-8")
    if conftest:
        (target.parent/"conftest.py").write_text(conftest, encoding="utf-8")
    pins = {p.relative_to(root).as_posix(): R.sha256(p) for p in root.rglob("*.py")}
    return root, pins


def test_real_passing_run_binds_complete_evidence_and_ignores_inherited_filters(tmp_path, monkeypatch):
    root, pins = project(tmp_path)
    monkeypatch.setenv("PYTEST_ADDOPTS", "-k absolutely_absent")
    monkeypatch.setenv("PYTEST_PLUGINS", "nonexistent_plugin")
    report = R.run(root, root/"attempt", FILES, pins, [])
    assert report["regression_passed"], report
    assert report["counts"]["tests"] == 2 and report["counts"]["phase_reports"] == 6
    assert report["scientific_completion_evaluated"] is report["canonical_adoption"] is False
    assert R._read(root/"attempt/report.json") == report
    assert set(report["artifact_sha256"]) == {
        "pytest.ini", "lock.json", "collection/stdout", "collection/stderr", "collection/collection.json",
        "collection/reports.json", "execution/stdout", "execution/stderr", "execution/collection.json",
        "execution/reports.json", "execution/pytest.xml"}
    for name, digest in report["artifact_sha256"].items():
        assert R.sha256(root/"attempt"/name) == digest


@pytest.mark.parametrize("kind", ["suppressed", "reordered", "skip", "setup_failure", "teardown_failure", "call_failure", "xfail"])
def test_real_failed_runs_keep_raw_evidence(tmp_path, kind):
    conftest, tests = None, "def test_one(): pass\ndef test_two(): pass\n"
    if kind == "suppressed":
        conftest = "def pytest_collection_modifyitems(items): items[:] = items[:1]\n"
    elif kind == "reordered":
        conftest = "def pytest_collection_modifyitems(items): items.reverse()\n"
    elif kind == "skip":
        tests = "import pytest\ndef test_one(): pytest.skip('unavailable')\n"
    elif kind == "xfail":
        tests = "import pytest\n@pytest.mark.xfail(reason='open')\ndef test_one(): assert False\n"
    elif kind in ("setup_failure", "teardown_failure"):
        body = "assert False\n    yield" if kind == "setup_failure" else "yield\n    assert False"
        conftest = "import pytest\n@pytest.fixture(autouse=True)\ndef broken():\n    "+body+"\n"
    else:
        tests = "def test_one(): assert False\n"
    root, pins = project(tmp_path, tests, conftest)
    report = R.run(root, root/"failed", FILES, pins, [])
    assert not report["regression_passed"] and report["failures"]
    assert (root/"failed/collection/stdout").is_file()
    collection = R._read(root/"failed/collection/collection.json")
    if kind in ("suppressed", "reordered"):
        assert collection["discovered"] != collection["nodes"]
        assert "execution" not in report["exit_codes"]
    else:
        assert (root/"failed/execution/pytest.xml").is_file()
        actual = R._read(root/"failed/execution/reports.json")
        if kind == "setup_failure":
            assert not any(row["when"] == "call" for row in actual["reports"])
            observed = report["observed_outcomes"]["execution"]
            assert observed["available"] and observed["nodes_with_reports"] == 2
            assert {tuple(row.values()) for row in observed["phase_outcomes"]} == {
                ("setup", "failed", 2), ("teardown", "passed", 2)}


def test_coordinator_code_must_match_its_own_pin(tmp_path):
    root, pins = project(tmp_path)
    (root/R.MODULE).write_text("different coordinator", encoding="utf-8")
    pins[R.MODULE] = R.sha256(root/R.MODULE)
    report = R.run(root, root/"wrong-coordinator", FILES, pins, [])
    assert not report["admitted"] and "executing coordinator" in report["failures"][0]


@pytest.mark.parametrize("kind", ["pytest_rewritten", "ordinary_import"])
def test_stale_passing_bytecode_cannot_override_pinned_failing_source(tmp_path, kind):
    root, _ = project(tmp_path, "def test_one():\n    assert True \n")
    if kind == "pytest_rewritten":
        target = root/FILES[0]
        target.write_bytes(b"def test_one():\n    assert True \n")
        current = b"def test_one():\n    assert False\n"
        stamp = target.stat()
        environment = dict(os.environ, PYTEST_ADDOPTS="", PYTEST_DISABLE_PLUGIN_AUTOLOAD="1", PYTEST_PLUGINS="")
        for name in ("PYTHONDONTWRITEBYTECODE", "PYTHONPYCACHEPREFIX", "PYTHONPATH", "PYTHONHOME"):
            environment.pop(name, None)
        bootstrap = subprocess.run([sys.executable, "-m", "pytest", FILES[0], "-q"], cwd=root,
                                   env=environment, capture_output=True)
        assert bootstrap.returncode == 0, bootstrap.stdout
        caches = list(target.parent.glob("__pycache__/*pytest*.pyc"))
        assert len(caches) == 1
        cache = caches[0]
    else:
        (root/FILES[0]).write_text("from helper import VALUE\ndef test_one(): assert VALUE\n", encoding="utf-8")
        target = root/"helper.py"
        target.write_bytes(b"VALUE = True \n")
        current = b"VALUE = False\n"
        stamp = target.stat()
        cache = Path(py_compile.compile(str(target), doraise=True))
    assert len(current) == target.stat().st_size
    cache_hash = R.sha256(cache)
    target.write_bytes(current)
    os.utime(target, ns=(stamp.st_atime_ns, stamp.st_mtime_ns))
    pins = {p.relative_to(root).as_posix(): R.sha256(p) for p in root.rglob("*.py")}
    result = R.run(root, root/"stale-cache", FILES, pins, [])
    assert not result["regression_passed"] and result["exit_codes"]["execution"] == 1
    assert result["source_drift"] == [] and R.sha256(cache) == cache_hash
    assert "assert False" in (root/"stale-cache/execution/stdout").read_text()
    lock = R._read(root/"stale-cache/lock.json")
    prefixes = [lock["environment_overrides"][mode]["set"]["PYTHONPYCACHEPREFIX"] for mode in ("collection", "execution")]
    assert prefixes[0] != prefixes[1]


def test_optimization_cannot_suppress_assertions_in_ordinary_modules(tmp_path, monkeypatch):
    root, pins = project(tmp_path, "from helper import verify\ndef test_one(): verify()\n")
    helper = root/"helper.py"
    helper.write_bytes(b"def verify():\n    assert False\n")
    pins["helper.py"] = R.sha256(helper)
    monkeypatch.setenv("PYTHONOPTIMIZE", "2")
    report = R.run(root, root/"optimized-parent-env", FILES, pins, [])
    assert not report["regression_passed"] and report["exit_codes"]["execution"] == 1
    assert report["source_drift"] == []
    for mode in ("collection", "execution"):
        for name in ("collection.json", "reports.json"):
            assert R._read(root/"optimized-parent-env"/mode/name)["optimize"] == 0


def test_unpinned_conftest_rejects_before_execution_and_preserves_attempt(tmp_path):
    root, pins = project(tmp_path, conftest="def pytest_configure(config): pass\n")
    del pins["tests/conftest.py"]
    report = R.run(root, root/"rejected", FILES, pins, [])
    assert not report["admitted"] and not report["regression_passed"]
    original = (root/"rejected/report.json").read_bytes()
    with pytest.raises(ValueError, match="exclusive"):
        R.run(root, root/"rejected", FILES, pins, [])
    assert (root/"rejected/report.json").read_bytes() == original


def test_loaded_unpinned_local_plugin_is_rejected(tmp_path):
    root, pins = project(tmp_path, conftest="pytest_plugins = ['test_extra_plugin']\n")
    extra = root/"test_extra_plugin.py"
    extra.write_text("def pytest_configure(config): pass\n", encoding="utf-8")
    report = R.run(root, root/"foreign-plugin", FILES, pins, [])
    assert not report["regression_passed"]
    assert "unexpected or changed local pytest plugin" in report["failures"][0]


@pytest.mark.parametrize("failure", ["source_drift", "source_missing", "preservation_post", "checkout_post"])
def test_postcheck_failures_leave_explicit_report(tmp_path, monkeypatch, failure):
    root, pins = project(tmp_path)
    original_invoke, original_checkout = R._invoke, R._checkout
    def invoke(command, directory, environment, output):
        code = original_invoke(command, directory, environment, output)
        if output.name == "execution" and failure in ("source_drift", "source_missing"):
            source = root/FILES[0]
            if failure == "source_drift":
                source.write_text("changed", encoding="utf-8")
            else:
                source.unlink()
        return code
    monkeypatch.setattr(R, "_invoke", invoke)
    seen = []
    def preservation(*args):
        seen.append("preservation")
        if len(seen) == 2 and failure == "preservation_post":
            raise ValueError("injected preservation postcheck failure")
        return {"synthetic": True}
    monkeypatch.setattr(R, "verify_preservation", preservation)
    checkouts = []
    def checkout(path):
        checkouts.append(path)
        if len(checkouts) == 2 and failure == "checkout_post":
            raise OSError("injected checkout postcheck failure")
        return original_checkout(path)
    monkeypatch.setattr(R, "_checkout", checkout)
    report = R.run(root, root/"postcheck", FILES, pins, [{"synthetic": True}])
    assert not report["regression_passed"] and len(seen) == len(checkouts) == 2
    assert (root/"postcheck/report.json").is_file()
    assert report["source_drift"] or report["postcheck_errors"]


def test_unrelated_checkout_change_is_recorded_separately(tmp_path, monkeypatch):
    root, pins = project(tmp_path)
    states = iter(({"status": []}, {"status": ["?? unrelated.html"]}))
    monkeypatch.setattr(R, "_checkout", lambda path: next(states))
    report = R.run(root, root/"external-change", FILES, pins, [])
    assert report["regression_passed"] and not report["checkout_unchanged"]
    assert report["source_drift"] == []
