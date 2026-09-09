"""Reject incomplete or internally contradictory scientific execution receipts."""
import json

import pytest

from scripts.phi_v2_lattice.experiments import certify_recovery_wave6 as W


@pytest.mark.parametrize("xml", (
    '<testsuite tests="1" failures="0" errors="0" skipped="0"/>',
    '<testsuite tests="1" failures="0" errors="0" skipped="0"><testcase><failure/></testcase></testsuite>',
    '<testsuite tests="1" failures="1" errors="0" skipped="1"><testcase><failure/><skipped/></testcase></testsuite>',
    '<testsuites/>', '<testsuite>',
))
def test_rejects_empty_contradictory_or_malformed_junit(tmp_path, xml):
    path = tmp_path/"pytest.xml"
    path.write_text(xml, encoding="utf-8")
    with pytest.raises((ValueError, W.ET.ParseError)):
        W.junit_counts(path)


@pytest.mark.parametrize("outcome,valid", (("", True), ("<failure/>", False), ("<error/>", False), ("<skipped/>", False)))
def test_actual_case_outcomes_determine_validation(tmp_path, outcome, valid):
    path = tmp_path/"pytest.xml"
    path.write_text('<testsuites><testsuite tests="1" failures="'+str(int("failure" in outcome))
                    +'" errors="'+str(int("error" in outcome))+'" skipped="'+str(int("skipped" in outcome))
                    +'">'+'<testcase>'+outcome+'</testcase></testsuite></testsuites>', encoding="utf-8")
    counts, passed = W.junit_counts(path)
    assert counts["tests"] == 1
    assert passed == valid


def test_unfrozen_backend_override_rejected(tmp_path, monkeypatch):
    monkeypatch.setenv("FTD_STRICT_NATIVE_CLI", str(tmp_path/"other.exe"))
    with pytest.raises(ValueError, match="override"):
        W.selected_backends({"source_sha256": {}})


@pytest.mark.parametrize("missing", ("artifacts", "sources"))
def test_empty_q3_inventories_cannot_approve_campaign(tmp_path, monkeypatch, missing):
    # Deliberately incomplete test-only receipts, not campaign artifacts.
    monkeypatch.setattr(W.Q3, "_accepted_upstream", lambda *args: [])
    lock = {"schema": "strict-flux-binding-q3-execution-lock-1", "law": W.Q3.LAW_ID,
            "acceptance_receipt_sha256": "unused-test-pin", "full_runtime_L": 8,
            "states": W.Q3.STATE_COUNT, "ranges": 96, "range_size": 7776,
            "coherent_instances": 38016, "intended_resource_ceiling_seconds": 1800, "source_sha256": {}}
    report = {"schema": "strict-flux-binding-q3-execution-result-1", "canonical_adoption": False,
              "resource_ceiling_met": True, "elapsed_seconds": 1, "artifact_sha256": {}}
    if missing == "sources":
        names = {"lock.json", "graph.json", "coherent-preparations.json", "transitions.npz"}
        names |= {f"runtime-ranges/{i:07d}.json" for i in range(0, W.Q3.STATE_COUNT, W.Q3.RELATIVE_COUNT)}
        report["artifact_sha256"] = {name: "unused-test-pin" for name in names}
    for name, value in (("lock.json", lock), ("report.json", report)):
        (tmp_path/name).write_text(json.dumps(value), encoding="utf-8")
    with pytest.raises(ValueError, match="inventory"):
        W.verify_q3(tmp_path)
