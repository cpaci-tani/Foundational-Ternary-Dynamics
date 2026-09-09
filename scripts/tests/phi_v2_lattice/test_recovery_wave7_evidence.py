"""Adversarial evidence-gate controls; no scientific campaign is rerun."""
from pathlib import Path
import json
import shutil
from types import SimpleNamespace

import numpy as np
import pytest

from scripts.phi_v2_lattice.experiments import certify_recovery_wave7 as validation


def write(path, data):
    Path(path).write_text(json.dumps(data), encoding="utf-8")


def census_copy(tmp_path):
    target = tmp_path/"census"
    shutil.copytree(validation.EVIDENCE/"local-tables-attempt-1", target)
    return target


def test_frozen_census_reduces_every_retained_count():
    result = validation.verify_census(validation.EVIDENCE/"local-tables-attempt-1")
    assert result["local_inputs_checked"] == 16777216
    assert result["group_tables"] == 324 and result["integer_counts"] == 2304
    assert result["eligible_count"] == 11740


@pytest.mark.parametrize("mutation", ["observable", "gram", "jacobian", "scope", "count", "group_order"])
def test_rehashed_census_cannot_change_observable_or_law(tmp_path, mutation):
    directory = census_copy(tmp_path)
    tables = validation.read(directory/"tables.json")
    if mutation == "observable":
        tables["joint_tables"][0]["input"] = [999]
    elif mutation == "gram":
        tables["eligible_gram"][0][1] += 1
    elif mutation == "jacobian":
        tables["jacobian_numerator"][0][0] += 1
    elif mutation == "scope":
        tables["complete_global_ensemble_enumerated"] = True
    elif mutation == "count":
        tables["joint_tables"][0]["counts"][0] += 1
    else:
        tables["groups"][0], tables["groups"][1] = tables["groups"][1], tables["groups"][0]
    write(directory/"tables.json", tables)
    report = validation.read(directory/"report.json")
    report["artifact_sha256"]["tables.json"] = validation.sha(directory/"tables.json")
    write(directory/"report.json", report)
    with pytest.raises(ValueError):
        validation.verify_census(directory)


def test_rehashed_census_cannot_omit_a_completed_range(tmp_path):
    directory = census_copy(tmp_path)
    report = validation.read(directory/"report.json")
    del report["artifact_sha256"]["ranges/00000000.json"]
    write(directory/"report.json", report)
    with pytest.raises(ValueError, match="incomplete"):
        validation.verify_census(directory)


def test_rehashed_range_cannot_shorten_actual_coverage(tmp_path):
    directory = census_copy(tmp_path)
    name = "ranges/00000000.json"
    row = validation.read(directory/name)
    row["checked"] -= 1
    write(directory/name, row)
    report = validation.read(directory/"report.json")
    report["artifact_sha256"][name] = validation.sha(directory/name)
    write(directory/"report.json", report)
    with pytest.raises(ValueError, match="incomplete local input"):
        validation.verify_census(directory)


def test_graph_cannot_drop_predicate_arrays_or_change_dtype():
    with pytest.raises(ValueError, match="twelve"):
        validation.artifact_graph_facts({"successor": np.zeros(1, dtype=np.int32)})
    names = ("successor", "plus_move", "minus_move", "operation", "translating_family", "path_length",
             "component", "transient_to_cycle", "first_translating_family", "relative_records",
             "first_adjacency", "fresh_phase1_R2")
    with pytest.raises(ValueError, match="dtype/shape"):
        validation.artifact_graph_facts({name: np.zeros(1, dtype=np.float64) for name in names})


def test_ordered_pair_account_distinguishes_short_orbits():
    ranges = validation.memory_ranges()
    assert len(ranges) == 128 and ranges[0][0] == 0 and ranges[-1][1] == 34462770
    assert all(left[1] == right[0] for left, right in zip(ranges, ranges[1:]))
    assert sum(validation.ordered_pairs(a, b) for a, b in ranges) == 11740**2 == 137827600
    assert validation.ordered_pairs(0, 1) == 2  # diagonal
    assert validation.ordered_pairs(1, 2) == 4
    assert validation.ordered_pairs(11739, 11740) == 2  # anti-diagonal


def test_hash_inventory_rejects_escape_and_empty_required_map(tmp_path):
    inner = tmp_path/"inside"
    inner.mkdir()
    outside = tmp_path/"outside.txt"
    outside.write_text("retained", encoding="utf-8")
    with pytest.raises(ValueError, match="invalid frozen"):
        validation.hashes_match(inner, {"../outside.txt": validation.sha(outside)})
    with pytest.raises(ValueError, match="missing"):
        validation.hashes_match(inner, {}, ["required.txt"])


def fake_run(tmp_path, monkeypatch, fault):
    source = tmp_path/"frozen-source.txt"
    source.write_text("frozen", encoding="utf-8")
    baseline = tmp_path/"baseline.json"
    write(baseline, {})
    monkeypatch.setattr(validation, "BASELINE", baseline)
    initial = {"head": "synthetic", "index": [], "status": []}
    def inventory(*args):
        if fault == "preflight":
            raise ValueError("injected preflight rejection")
        return [source], initial
    monkeypatch.setattr(validation, "input_inventory", inventory)
    monkeypatch.setattr(validation, "preserved_baseline", lambda b: ([], initial))
    monkeypatch.setattr(validation, "verify_census", lambda p: {"synthetic": True})
    monkeypatch.setattr(validation, "verify_binding", lambda p: {"synthetic": True})
    monkeypatch.setattr(validation, "verify_memory", lambda p: {"synthetic": True})
    after_test = False
    def process(command, **kwargs):
        nonlocal after_test
        env = kwargs["env"]
        assert env["PYTEST_ADDOPTS"] == env["PYTEST_PLUGINS"] == ""
        assert env["PYTEST_DISABLE_PLUGIN_AUTOLOAD"] == "1"
        nodes = [name+"::synthetic" for name in validation.TESTS]
        if fault == "filtered_collection":
            nodes = nodes[:-1]
        collection = {"schema": "strict-wave7-pytest-collection-1", "nodes": nodes, "deselected": []}
        write(env["FTD_WAVE7_COLLECTION_RECORD"], collection)
        if "--collect-only" in command:
            return SimpleNamespace(returncode=0)
        xml = next(argument.split("=", 1)[1] for argument in command if argument.startswith("--junitxml="))
        if fault != "xml":
            Path(xml).write_text(f'<testsuites><testsuite tests="{len(validation.TESTS)}" failures="0" errors="0" skipped="0">'+
                                 ''.join(f'<testcase name="synthetic{i}"/>' for i in range(len(validation.TESTS)))+
                                 '</testsuite></testsuites>', encoding="utf-8")
        reports = [{"nodeid": node, "when": "call", "outcome": "passed"} for node in nodes]
        if fault == "filtered_execution":
            reports = reports[:-1]
        write(env["FTD_WAVE7_REPORTS_RECORD"], {"schema": "strict-wave7-pytest-reports-1", "exitstatus": 0, "reports": reports})
        after_test = True
        return SimpleNamespace(returncode=0)
    monkeypatch.setattr(validation.subprocess, "run", process)
    real_sha = validation.sha
    def hashing(path):
        if after_test and ((fault == "input_hash" and Path(path) == source) or
                           (fault == "artifact_hash" and Path(path).name == "pytest.log")):
            raise OSError("injected post-execution read failure")
        return real_sha(path)
    monkeypatch.setattr(validation, "sha", hashing)
    def checkout():
        if fault == "git":
            raise OSError("injected git failure")
        return initial
    monkeypatch.setattr(validation, "checkout", checkout)
    return tmp_path/"validation"


@pytest.mark.parametrize("fault", ["preflight", "xml", "input_hash", "artifact_hash", "git", "filtered_collection", "filtered_execution"])
def test_every_parent_failure_retains_a_failed_report(tmp_path, monkeypatch, fault):
    output = fake_run(tmp_path, monkeypatch, fault)
    with pytest.raises(Exception):
        validation.run(output, tmp_path/"binding", tmp_path/"memory")
    report = validation.read(output/"report.json")
    assert report["passed"] is False
    assert report["canonical_adoption"] is False
    if fault == "preflight":
        assert report["status"] == "REJECTED_BEFORE_EXECUTION"
    elif fault in ("xml", "filtered_collection", "filtered_execution"):
        assert report["failure"]
    else:
        assert report["postcheck_failures"]


def test_successful_software_receipt_keeps_physics_open(tmp_path, monkeypatch):
    output = fake_run(tmp_path, monkeypatch, None)
    result = validation.run(output, tmp_path/"binding", tmp_path/"memory")
    assert result["passed"] is True
    assert result["test_counts"] == {"tests": len(validation.TESTS), "errors": 0, "failures": 0, "skipped": 0}
    assert result["full_physics_recovery"] is result["interacting_fluid_recovered"] is False
    with pytest.raises(ValueError, match="existing"):
        validation.run(output, tmp_path/"binding", tmp_path/"memory")
