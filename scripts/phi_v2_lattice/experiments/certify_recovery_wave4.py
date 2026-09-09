"""Freeze and reproduce wave-4 exact certificates and the strict regression suite.

This is engineering/finite evidence, not a physical recovery certificate.
The separate alignment symmetry matrix retains its own pre-execution lock.
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

from phi_v2_lattice import alignment, hydro_complement, hydro_complement_response
from phi_v2_lattice import recovery_contact_reachability, recovery_hydro_successor_feasibility
from phi_v2_lattice import staged_alignment  # include checkpoint dependency in the lock

ROOT = Path(__file__).resolve().parents[3]
BASELINE = ROOT / "engine/docs/evidence/strict-recovery-wave4-baseline-2026-09-08.json"
CERTIFICATES = {
    "alignment-finite": alignment.finite_certificate,
    "hydro-feasibility": recovery_hydro_successor_feasibility.certificate,
    "contact-reachability": recovery_contact_reachability.certificate,
    "hydro-complement-finite": hydro_complement.certificate,
    "hydro-complement-response": hydro_complement_response.certificate,
}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path, value):
    path.write_bytes((json.dumps(value, indent=2, sort_keys=True) + "\n").encode("utf-8"))


def local_dependencies():
    result = set()
    for module in tuple(sys.modules.values()):
        file = getattr(module, "__file__", None)
        if file:
            path = Path(file).resolve()
            if path.is_relative_to(ROOT) and path.is_file():
                result.add(path)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    directory = args.output.resolve()
    if directory.exists():
        raise ValueError("output exists; retain every previous attempt")
    baseline = json.loads(BASELINE.read_bytes())
    drift = [p for p, expected in baseline["source_sha256"].items()
             if digest(ROOT / p) != expected.lower()]
    if drift:
        raise ValueError(f"baseline drift before validation: {drift}")
    dependencies = local_dependencies()
    paths = (dependencies | {Path(__file__).resolve(), BASELINE}
             | {ROOT / p for p in baseline["source_sha256"]}
             | set((ROOT / "scripts/phi_v2_lattice").glob("*.py"))
             | set((ROOT / "scripts/tests/phi_v2_lattice").glob("*.py")))
    hashes = {p.relative_to(ROOT).as_posix(): digest(p) for p in sorted(paths)}
    command = [sys.executable, "-m", "pytest", "scripts/tests/phi_v2_lattice",
               "-q", "-rs", "--junitxml", str(directory / "pytest.xml")]
    directory.mkdir(parents=True)
    lock = {
        "schema": "strict-recovery-wave4-validation-lock-1",
        "source_sha256": hashes, "baseline_drift": drift,
        "certificate_names": sorted(CERTIFICATES), "test_command": command,
        "python": sys.version, "platform": platform.platform(),
        "dependencies": {name: importlib.metadata.version(name)
                         for name in ("numpy", "sympy", "python-flint", "pytest")},
        "backend_configuration": {name: os.environ.get(name) for name in (
            "FTD_STRICT_CUDA_REQUIRED", "FTD_STRICT_CUDA_CLI", "FTD_STRICT_NATIVE_CLI",
            "FTD_STRICT_WASM_MODULE")},
        "source_freeze_scope": "top-level package/test modules, baseline entries and currently imported local dependencies; "
                               "not a freeze of all third-party binary files",
        "candidate_backend_scope": "new laws are Python-only; any native/CUDA/WASM tests exercise the old law",
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
    with (directory / "pytest.log").open("wb") as log:
        result = subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
    print((directory / "pytest.log").read_text(encoding="utf-8", errors="replace"), flush=True)
    drift = [p for p, expected in hashes.items() if digest(ROOT / p) != expected]
    new_dependencies = local_dependencies() - paths
    test_counts = {}
    if (directory / "pytest.xml").is_file():
        suites = ET.parse(directory / "pytest.xml").getroot().findall("testsuite")
        test_counts = {key: sum(int(suite.attrib[key]) for suite in suites)
                       for key in ("tests", "errors", "failures", "skipped")}
    for filename in ("pytest.log", "pytest.xml"):
        path = directory / filename
        if path.is_file():
            evidence[filename] = digest(path)
    summary = {
        "schema": "strict-recovery-wave4-validation-result-1",
        "elapsed_seconds": time.monotonic() - started,
        "lock_sha256": digest(directory / "lock.json"), "evidence_sha256": evidence,
        "pytest_returncode": result.returncode, "test_counts": test_counts,
        "source_drift": drift,
        "new_local_certificate_dependencies": sorted(str(p.relative_to(ROOT)) for p in new_dependencies),
        "engineering_checks_passed": result.returncode == 0 and not drift and not new_dependencies,
        "continuum_recovered": False, "material_binding_recovered": False,
        "canonical_adoption": False,
    }
    write_json(directory / "report.json", summary)
    print(json.dumps(summary), flush=True)
    if not summary["engineering_checks_passed"]:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
