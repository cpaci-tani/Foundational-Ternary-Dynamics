"""Freeze wave-5 research sources, exact certificates and strict regressions.

The binding quotient and prepared passive diffusion have scoped positive
results. Interacting fluid, moving matter and unified-stack recovery stay open.
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
import xml.etree.ElementTree as ET

from scripts.phi_v2_lattice import flux_binding, hydro_parity, hydro_parity_response
from scripts.phi_v2_lattice import hydro_complement, recovery_micro_correlation
from scripts.phi_v2_lattice import recovery_flux_binding_q2, routing_diffusion_response

ROOT = Path(__file__).resolve().parents[3]
BASELINE = ROOT / "engine/docs/evidence/strict-recovery-wave5-baseline-2026-09-08.json"
PRIOR_Q2 = ROOT / "engine/docs/evidence/strict-recovery-wave5-2026-09-08/flux-binding-q2.json"
CERTIFICATES = {
    "flux-binding-local": flux_binding.local_account_certificate,
    "flux-binding-q2-graph": recovery_flux_binding_q2.graph_certificate,
    "hydro-parity-finite": hydro_parity.certificate,
    "hydro-parity-response": hydro_parity_response.certificate,
    "micro-correlation-parity": lambda: recovery_micro_correlation.certificate(hydro_parity),
    "micro-correlation-complement": lambda: recovery_micro_correlation.certificate(hydro_complement),
    "routing-diffusion-continuum": routing_diffusion_response.certificate,
}
DOCS = (
    "SPEC_STRICT_FLUX_BINDING_CANDIDATE_V1.md", "AUDIT_STRICT_FLUX_BINDING_V1.md",
    "DERIV_STRICT_FLUX_BINDING_Q2_V1.md", "AUDIT_STRICT_FLUX_BINDING_Q2_V1.md",
    "SPEC_STRICT_HYDRO_PARITY_CANDIDATE_V1.md", "AUDIT_STRICT_HYDRO_PARITY_V1.md",
    "DERIV_STRICT_MICRO_CORRELATION_V1.md", "AUDIT_STRICT_MICRO_CORRELATION_V1.md",
    "SPEC_STRICT_ROUTING_DIFFUSION_V1.md", "AUDIT_STRICT_ROUTING_DIFFUSION_V1.md",
)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path, data):
    path.write_bytes((json.dumps(data, indent=2, sort_keys=True) + "\n").encode())


def cleanup_signature():
    return hashlib.sha256(subprocess.check_output(
        ["git", "diff", "--binary", "HEAD", "--"], cwd=ROOT)).hexdigest()


def local_dependencies():
    result = set()
    for module in tuple(sys.modules.values()):
        filename = getattr(module, "__file__", None)
        if filename:
            path = Path(filename).resolve()
            if path.is_relative_to(ROOT) and path.is_file():
                result.add(path)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    directory = args.output.resolve()
    if directory.exists():
        raise ValueError("output exists; preserve every prior attempt")
    baseline = json.loads(BASELINE.read_bytes())
    for name, expected in baseline["source_sha256"].items():
        if digest(ROOT / name) != expected.lower():
            raise ValueError(f"wave-4 frozen source changed: {name}")
    paths = (local_dependencies() | {Path(__file__).resolve(), BASELINE, PRIOR_Q2}
             | {ROOT / p for p in baseline["source_sha256"]}
             | set((ROOT / "scripts/phi_v2_lattice").rglob("*.py"))
             | set((ROOT / "scripts/tests/phi_v2_lattice").rglob("*.py"))
             | {ROOT / "engine/docs" / name for name in DOCS})
    backend_candidates = {
        "native": Path(os.environ.get("FTD_STRICT_NATIVE_CLI", ROOT / "engine/build_strict_native/ftd_strict_cli.exe")),
        "WSL2_CUDA": Path(os.environ.get("FTD_STRICT_CUDA_CLI", ROOT / "engine/build_strict_cuda/ftd_strict_cuda_cli")),
    }
    backend_artifacts = {name: {"path": str(path.resolve()), "sha256": digest(path)}
                         for name, path in backend_candidates.items() if path.is_file()}
    paths |= {path.resolve() for path in backend_candidates.values() if path.is_file()}
    hashes = {p.relative_to(ROOT).as_posix(): digest(p) for p in sorted(paths)}
    command = [sys.executable, "-m", "pytest", "scripts/tests/phi_v2_lattice",
               "-q", "-rs", "--junitxml", str(directory / "pytest.xml")]
    cleanup = cleanup_signature()
    directory.mkdir(parents=True)
    lock = {
        "schema": "strict-recovery-wave5-validation-lock-1", "source_sha256": hashes,
        "wave4_baseline_sources": len(baseline["source_sha256"]), "baseline_drift": [],
        "tracked_cleanup_diff_sha256": cleanup, "certificate_names": sorted(CERTIFICATES),
        "test_command": command, "python": sys.version, "platform": platform.platform(),
        "dependencies": {name: importlib.metadata.version(name)
                         for name in ("numpy", "sympy", "python-flint", "pytest")},
        "backend_configuration": {name: os.environ.get(name) for name in (
            "FTD_STRICT_CUDA_REQUIRED", "FTD_STRICT_CUDA_CLI", "FTD_STRICT_NATIVE_CLI", "FTD_STRICT_WASM_MODULE")},
        "backend_artifacts": backend_artifacts,
        "freeze_scope": "all package/test Python files, imported local dependencies, prior144 sources, "
                        "five specifications and independent audits, prior complete Q2 runtime evidence; "
                        "third-party binary identities are not fully frozen",
        "backend_scope": "new research laws are Python-only; native/CUDA/WASM tests exercise old law",
    }
    write_json(directory / "lock.json", lock)
    started = time.monotonic()
    evidence = {}
    for name, function in CERTIFICATES.items():
        report = function()
        path = directory / (name + ".json")
        write_json(path, report)
        evidence[path.name] = digest(path)
        print(json.dumps({"certificate": name, "sha256": evidence[path.name]}), flush=True)
    prior = json.loads(PRIOR_Q2.read_bytes())
    graph = json.loads((directory / "flux-binding-q2-graph.json").read_bytes())
    runtime_evidence = prior["runtime"]
    ranges = runtime_evidence["ranges"]
    cursor = 0
    for row in ranges:
        if (row["start"] != cursor or row["stop"] <= cursor or row["L"] != 6
                or row["checked"] != row["stop"] - row["start"]
                or len(row["complete_outputs_sha256"]) != 64):
            raise ValueError("invalid complete runtime range coverage")
        cursor = row["stop"]
    if (graph != prior["graph"] or runtime_evidence["checked"] != 148608
            or cursor != 148608 or runtime_evidence["L"] != 6
            or runtime_evidence["complete_arrays_and_ordinal_equal"] is not True):
        raise ValueError("complete Q2 campaign does not match frozen graph")
    with (directory / "pytest.log").open("wb") as log:
        result = subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
    print((directory / "pytest.log").read_text(encoding="utf-8", errors="replace"), flush=True)
    drift = [p for p, expected in hashes.items() if digest(ROOT / p) != expected]
    new_dependencies = local_dependencies() - paths
    suites = ET.parse(directory / "pytest.xml").getroot().findall("testsuite") if (directory / "pytest.xml").is_file() else []
    counts = {key: sum(int(s.attrib[key]) for s in suites) for key in ("tests", "errors", "failures", "skipped")}
    junit_valid = (bool(suites) and counts["tests"] > counts["skipped"]
                   and counts["errors"] == 0 and counts["failures"] == 0)
    for name in ("pytest.log", "pytest.xml"):
        path = directory / name
        if path.is_file():
            evidence[name] = digest(path)
    cleanup_unchanged = cleanup_signature() == cleanup
    summary = {
        "schema": "strict-recovery-wave5-validation-result-1", "elapsed_seconds": time.monotonic()-started,
        "lock_sha256": digest(directory / "lock.json"), "evidence_sha256": evidence,
        "pytest_returncode": result.returncode, "test_counts": counts, "junit_valid": junit_valid, "source_drift": drift,
        "new_local_certificate_dependencies": sorted(str(p.relative_to(ROOT)) for p in new_dependencies),
        "tracked_cleanup_unchanged": cleanup_unchanged,
        "engineering_checks_passed": result.returncode == 0 and junit_valid and not drift and not new_dependencies and cleanup_unchanged,
        "Q2_restoring_recurrence": "all148608 legal quotient states, contact within46 microticks; selected law",
        "passive_continuum": "exact prepared ensemble diffusion and weak C4 heat bound, L>3T and diffusive rescaling",
        "interacting_fluid_continuum_recovered": False, "transported_material_composite_recovered": False,
        "unified_stack_recovered": False, "canonical_adoption": False,
    }
    write_json(directory / "report.json", summary)
    print(json.dumps(summary), flush=True)
    if not summary["engineering_checks_passed"]:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
