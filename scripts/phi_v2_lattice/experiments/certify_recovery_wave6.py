"""Freeze wave-6 finite research evidence and run the complete strict suite.

Scoped covariance, restoring transport and prepared diffusion are distinct
gates. Passing this integration never adopts a law or recovers the full stack.
"""
from __future__ import annotations

import argparse
import hashlib
import importlib.metadata
import json
import os
from pathlib import Path
import platform
import subprocess
import sys
import time
import traceback
import xml.etree.ElementTree as ET

from scripts.phi_v2_lattice import recorded_routing, recorded_routing_response
from scripts.phi_v2_lattice import recovery_momentum_memory
from scripts.phi_v2_lattice import recovery_flux_binding_q3 as Q3

ROOT = Path(__file__).resolve().parents[3]
BASELINE = ROOT/"engine/docs/evidence/strict-recovery-wave6-baseline-2026-09-08.json"
EVIDENCE = ROOT/"engine/docs/evidence/strict-recovery-wave6-2026-09-08"
PREVIOUS = ROOT/"engine/docs/evidence/strict-recovery-wave5-2026-09-08/manifest.json"
DOCS = (
    "CONTRACT_STRICT_RECOVERY_WAVE6_V1.md", "AMENDMENT_STRICT_WAVE6_BALANCED_ORDER_V1.md",
    "SPEC_STRICT_RECORDED_MATCHING_V1.md", "AUDIT_STRICT_RECORDED_MATCHING_V1.md",
    "SPEC_STRICT_BALANCED_MATCHING_V1.md", "AUDIT_STRICT_BALANCED_MATCHING_V1.md",
    "SPEC_STRICT_RECORDED_ROUTING_V1.md", "AUDIT_STRICT_RECORDED_ROUTING_V1.md",
    "DERIV_STRICT_MOMENTUM_MEMORY_V1.md", "AUDIT_STRICT_MOMENTUM_MEMORY_V1.md",
    "DERIV_STRICT_FLUX_BINDING_Q3_V1.md", "AUDIT_STRICT_FLUX_BINDING_Q3_V1.md",
)
CERTIFICATES = {
    "recorded-routing-finite": recorded_routing.certificate,
    "recorded-routing-continuum": recorded_routing_response.certificate,
    "momentum-memory": recovery_momentum_memory.certificate,
}


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def write_json(path, value):
    path.write_bytes((json.dumps(value, indent=2, sort_keys=True)+"\n").encode())


def cleanup_signature():
    return hashlib.sha256(subprocess.check_output(["git", "diff", "--binary", "HEAD", "--"], cwd=ROOT)).hexdigest()


def local_dependencies():
    paths = set()
    for module in tuple(sys.modules.values()):
        name = getattr(module, "__file__", None)
        if name:
            path = Path(name).resolve()
            if path.is_relative_to(ROOT) and path.is_file():
                paths.add(path)
    return paths


def verify_q3(directory):
    report = json.loads((directory/"report.json").read_bytes())
    lock = json.loads((directory/"lock.json").read_bytes())
    accepted = Q3._accepted_upstream(ROOT, EVIDENCE/"balanced-matching-acceptance.json",
                                    lock["acceptance_receipt_sha256"], sha(ROOT/"scripts/phi_v2_lattice/balanced_matching.py"))
    if (report.get("schema") != "strict-flux-binding-q3-execution-result-1"
            or lock.get("schema") != "strict-flux-binding-q3-execution-lock-1"
            or lock.get("law") != Q3.LAW_ID or report.get("canonical_adoption") is not False
            or report.get("resource_ceiling_met") is not True
            or not 0 <= report.get("elapsed_seconds", -1) <= 1800
            or any(lock.get(k) != v for k, v in {"full_runtime_L": 8, "states": Q3.STATE_COUNT,
                    "ranges": 96, "range_size": 7776, "coherent_instances": 38016,
                    "intended_resource_ceiling_seconds": 1800}.items())):
        raise ValueError("Q3 schemas, domain or resource disposition differ")
    expected_artifacts = {"lock.json", "graph.json", "coherent-preparations.json", "transitions.npz"}
    expected_artifacts |= {f"runtime-ranges/{i:07d}.json" for i in range(0, Q3.STATE_COUNT, Q3.RELATIVE_COUNT)}
    if set(report.get("artifact_sha256", {})) != expected_artifacts:
        raise ValueError("Q3 artifact inventory is not complete")
    required_sources = {str(p.resolve()) for p in accepted}
    required_sources |= {str((ROOT/p).resolve()) for p in (
        "scripts/phi_v2_lattice/recovery_flux_binding_q3.py",
        "scripts/tests/phi_v2_lattice/test_recovery_flux_binding_q3.py",
        "engine/docs/DERIV_STRICT_FLUX_BINDING_Q3_V1.md",
        "engine/docs/PROPOSAL_STRICT_BOUND_COMPOSITE_TRANSPORT_V1.md",
        "engine/docs/AMENDMENT_STRICT_WAVE6_BALANCED_ORDER_V1.md")}
    if not required_sources.issubset(lock.get("source_sha256", {})):
        raise ValueError("Q3 executed-source inventory is not complete")
    for name, expected in report["artifact_sha256"].items():
        if sha(directory/name) != expected:
            raise ValueError("Q3 execution artifact changed: "+name)
    for name, expected in lock["source_sha256"].items():
        if sha(name) != expected:
            raise ValueError("Q3 executed input changed: "+name)
    if (report["engineering_complete"] is not True or report["source_drift"]
            or report["failure"] is not None or report["law"] != Q3.LAW_ID
            or report["complete_arrays_and_ordinal_equal"] is not True
            or report["observed_quotient_equal"] is not True
            or report["runtime_states_checked"] != Q3.STATE_COUNT):
        raise ValueError("Q3 complete implementation comparison not accepted")
    cursor = 0
    for row in report["ranges"]:
        if (row["start"] != cursor or row["stop"] <= cursor or row["L"] != 8
                or row["checked"] != row["stop"]-cursor or row["status"] != "PASS"
                or len(row["complete_outputs_sha256"]) != 64
                or json.loads((directory/"runtime-ranges"/f"{cursor:07d}.json").read_bytes()) != row):
            raise ValueError("invalid complete Q3 runtime coverage")
        cursor = row["stop"]
    if cursor != Q3.STATE_COUNT or len(report["ranges"]) != 96:
        raise ValueError("incomplete Q3 state ranges")
    graph = json.loads((directory/"graph.json").read_bytes())
    if graph["states"] != Q3.STATE_COUNT or graph["coherent_instances"] != 38016:
        raise ValueError("registered Q3 domain changed")
    for gate in ("existence_gate", "global_restoration_gate", "coherent_robustness_gate"):
        if graph[gate] != report[gate]:
            raise ValueError("Q3 physical disposition differs from graph")
    # Negative physical results remain accepted evidence; they are not retuned.
    return {key: graph[key] for key in ("states", "components", "recurrent_states", "transported_components",
            "restoring_transported_components", "existence_gate", "global_restoration_gate", "coherent_robustness_gate")}


def junit_counts(path):
    """Count actual outcomes and reject contradictory or empty suite headers."""
    root = ET.parse(path).getroot()
    suites = [root] if root.tag == "testsuite" else root.findall("testsuite")
    totals = dict(tests=0, errors=0, failures=0, skipped=0)
    if not suites:
        raise ValueError("JUnit contains no suites")
    for suite in suites:
        cases = suite.findall("testcase")
        actual = dict(tests=len(cases), errors=0, failures=0, skipped=0)
        for case in cases:
            outcomes = {tag: len(case.findall(tag)) for tag in ("error", "failure", "skipped")}
            if sum(outcomes.values()) > 1:
                raise ValueError("JUnit case has contradictory outcomes")
            actual["errors"] += outcomes["error"]
            actual["failures"] += outcomes["failure"]
            actual["skipped"] += outcomes["skipped"]
        if any(int(suite.attrib[key]) != value for key, value in actual.items()):
            raise ValueError("JUnit summary differs from actual cases")
        for key, value in actual.items():
            totals[key] += value
    valid = totals["tests"] > totals["skipped"] and totals["errors"] == totals["failures"] == 0
    return totals, valid


def selected_backends(baseline):
    """This invocation accepts only the executables already frozen in wave 5."""
    paths = {}
    for variable, default in (("FTD_STRICT_NATIVE_CLI", "engine/build_strict_native/ftd_strict_cli.exe"),
                              ("FTD_STRICT_CUDA_CLI", "engine/build_strict_cuda/ftd_strict_cuda_cli")):
        chosen = Path(os.environ.get(variable, ROOT/default)).resolve()
        expected = (ROOT/default).resolve()
        if chosen != expected or sha(chosen) != baseline["source_sha256"][default]:
            raise ValueError("backend override differs from frozen executable: "+variable)
        paths[variable] = {"path": str(chosen), "sha256": sha(chosen)}
    if os.environ.get("FTD_STRICT_CUDA_REQUIRED") != "1":
        raise ValueError("integrated WSL2 CUDA parity must be required")
    if os.environ.get("FTD_STRICT_WASM_MODULE"):
        raise ValueError("unfrozen optional WASM module is outside this invocation")
    return paths


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--q3-output", type=Path, required=True)
    args = parser.parse_args()
    directory, q3_directory = args.output.resolve(), args.q3_output.resolve()
    if directory.exists():
        raise ValueError("output exists; preserve all attempts")
    if not q3_directory.is_relative_to(ROOT):
        raise ValueError("research evidence must be inside the repository")
    baseline = json.loads(BASELINE.read_bytes())
    if sha(PREVIOUS) != baseline["wave5_manifest_sha256"]:
        raise ValueError("prior evidence manifest changed")
    previous = json.loads(PREVIOUS.read_bytes())
    prior_artifacts = {str(PREVIOUS.parent/name): expected for name, expected in previous["artifact_sha256"].items()}
    for name, expected in baseline["source_sha256"].items():
        if sha(ROOT/name) != expected:
            raise ValueError("prior frozen input changed: "+name)
    for name, expected in prior_artifacts.items():
        if sha(name) != expected:
            raise ValueError("prior evidence changed: "+name)
    cleanup = cleanup_signature()
    if cleanup != baseline["tracked_cleanup_diff_sha256"]:
        raise ValueError("preexisting tracked cleanup changed")
    if subprocess.check_output(["git", "diff", "--cached", "--name-only"], cwd=ROOT).strip():
        raise ValueError("unexpected staged changes")
    backend_artifacts = selected_backends(baseline)
    q3_status = verify_q3(q3_directory)
    paths = (local_dependencies() | {Path(__file__).resolve(), BASELINE, PREVIOUS}
             | {ROOT/name for name in baseline["source_sha256"]}
             | set((ROOT/"scripts/phi_v2_lattice").rglob("*.py"))
             | set((ROOT/"scripts/tests/phi_v2_lattice").rglob("*.py"))
             | {ROOT/"engine/docs"/name for name in DOCS}
             | {p for p in q3_directory.rglob("*") if p.is_file()}
             | {p for p in EVIDENCE.iterdir() if p.is_file()})
    hashes = {p.relative_to(ROOT).as_posix(): sha(p) for p in sorted(paths)}
    command = [sys.executable, "-B", "-m", "pytest", "scripts/tests/phi_v2_lattice", "-q", "-rs",
               "--junitxml", str(directory/"pytest.xml")]
    directory.mkdir(parents=True)
    write_json(directory/"lock.json", {
        "schema": "strict-recovery-wave6-validation-lock-1", "source_sha256": hashes,
        "prior182_inputs": len(baseline["source_sha256"]), "prior_artifacts_sha256": prior_artifacts,
        "tracked_cleanup_diff_sha256": cleanup, "certificate_names": sorted(CERTIFICATES),
        "test_command": command, "python": sys.version, "platform": platform.platform(),
        "dependencies": {name: importlib.metadata.version(name) for name in ("numpy", "sympy", "python-flint", "pytest")},
        "backend_configuration": {name: os.environ.get(name) for name in
             ("FTD_STRICT_CUDA_REQUIRED", "FTD_STRICT_CUDA_CLI", "FTD_STRICT_NATIVE_CLI", "FTD_STRICT_WASM_MODULE")},
        "backend_scope": "new research laws Python only; native and WSL2 CUDA tests cover unchanged staged law",
        "selected_backend_artifacts": backend_artifacts,
        "freeze_scope": "all package/tests, prior182 inputs, completed Q3 evidence, independent audits; third-party binaries not hermetically frozen",
    })
    started, failure, returncode = time.monotonic(), None, None
    try:
        for name, function in CERTIFICATES.items():
            value = function()
            if name == "momentum-memory":
                audit = json.loads((EVIDENCE/"momentum-independent-full-count.json").read_bytes())
                if (audit["local_inputs"] != 2**24 or len(value["cases"]) != 8
                        or len(value["endpoint_terms"]) != 4 or len(audit["terms"]) != 4
                        or any(row["signed_weak_error"] != audit["signed_expectation_error"] for row in value["cases"])):
                    raise ValueError("independent full local counting differs from momentum diagnostic")
                for term, counted in zip(value["endpoint_terms"], audit["terms"]):
                    for field, reference in (("source", "source"), ("target", "target"), ("displacement", "displacement"),
                                             ("actual_walsh", "actual"), ("projected", "projected"), ("returned", "return")):
                        if term[field] != counted[reference]:
                            raise ValueError("independent endpoint term differs: "+field)
            write_json(directory/(name+".json"), value)
            print(json.dumps({"certificate": name, "sha256": sha(directory/(name+".json"))}), flush=True)
        with (directory/"pytest.log").open("wb") as log:
            run = subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
        returncode = run.returncode
        print((directory/"pytest.log").read_text(encoding="utf-8", errors="replace"), flush=True)
    except BaseException as error:
        failure = "".join(traceback.format_exception(error))
    drift, prior_drift, new_dependencies, post_errors = [], [], set(), []
    counts, junit_valid, cleanup_unchanged, staged_empty = dict(tests=0, errors=0, failures=0, skipped=0), False, False, False
    for name, expected in hashes.items():
        try:
            if sha(ROOT/name) != expected:
                drift.append(name)
        except Exception as error:
            drift.append(name)
            post_errors.append(f"hash {name}: {error}")
    for name, expected in prior_artifacts.items():
        try:
            if sha(name) != expected:
                prior_drift.append(name)
        except Exception as error:
            prior_drift.append(name)
            post_errors.append(f"prior hash {name}: {error}")
    try:
        new_dependencies = local_dependencies()-paths
        counts, junit_valid = junit_counts(directory/"pytest.xml")
    except Exception as error:
        post_errors.append("JUnit/dependency validation: "+str(error))
    try:
        cleanup_unchanged = cleanup_signature() == cleanup
        staged_empty = not subprocess.check_output(["git", "diff", "--cached", "--name-only"], cwd=ROOT).strip()
    except Exception as error:
        post_errors.append("checkout verification: "+str(error))
    evidence_hashes = {}
    for path in sorted(directory.rglob("*")):
        if path.is_file():
            try:
                evidence_hashes[path.relative_to(directory).as_posix()] = sha(path)
            except Exception as error:
                post_errors.append(f"evidence hash {path}: {error}")
    expected_outputs = {"lock.json", "pytest.log", "pytest.xml"} | {name+".json" for name in CERTIFICATES}
    if set(evidence_hashes) != expected_outputs:
        post_errors.append("execution output inventory differs from the six required artifacts")
    passed = returncode == 0 and junit_valid and not (failure or drift or prior_drift or new_dependencies or post_errors) and cleanup_unchanged and staged_empty
    result = {"schema": "strict-recovery-wave6-validation-result-1", "elapsed_seconds": time.monotonic()-started,
              "lock_sha256": evidence_hashes.get("lock.json"), "failure": failure, "post_run_errors": post_errors,
              "evidence_sha256": evidence_hashes,
              "pytest_returncode": returncode, "test_counts": counts, "junit_valid": junit_valid,
              "source_drift": drift, "prior_artifact_drift": prior_drift,
              "new_local_certificate_dependencies": sorted(str(p.relative_to(ROOT)) for p in new_dependencies),
              "tracked_cleanup_unchanged": cleanup_unchanged, "staged_empty": staged_empty,
              "Q3": q3_status, "engineering_checks_passed": passed,
              "momentum_memory": "exact nonzero cycle4 weak error independently counted; no long-time upper bound",
              "passive_continuum": "full prepared ensemble; constant recorded corner; L>3T; diffusive scaling; weak C4",
              "interacting_fluid_continuum_recovered": False, "transported_physical_matter_recovered": False,
              "unified_stack_recovered": False, "canonical_adoption": False}
    write_json(directory/"report.json", result)
    print(json.dumps(result), flush=True)
    if not passed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
