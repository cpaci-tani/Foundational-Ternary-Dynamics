"""Verify the FTD-0434 revision-2 vacuum-photon diagnostic source lock."""

from __future__ import annotations

from hashlib import sha256
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LOCK = ROOT / "scripts/proofs/vacuum_photon_scenario_diagnostic_lock.json"


def main() -> None:
    lock = json.loads(LOCK.read_text(encoding="utf-8"))
    prereg_path = ROOT / lock["preregistration"]
    checks: list[tuple[str, bool]] = [
        ("LOCK preregistration",
         sha256(prereg_path.read_bytes()).hexdigest()
         == lock["preregistration_sha256"]),
    ]
    for relative, expected in lock["sources"].items():
        actual = sha256((ROOT / relative).read_bytes()).hexdigest()
        successor = lock.get("qualified_successors", {}).get(relative, {})
        accepted = actual == expected or any(
            item.get("identifier") == "FTD-0436"
            and actual == item.get("sha256")
            and "Registers the FTD-0436" in item.get("scope", "")
            for item in [successor] + successor.get("prior", [])
        )
        checks.append((f"LOCK {relative}", accepted))

    prereg = prereg_path.read_text(encoding="utf-8")
    words = " ".join(prereg.split())
    scenario = (ROOT / "engine/src/scenarios/vacuum.cpp").read_text(
        encoding="utf-8")
    campaign = (ROOT / "engine/tests/"
                "campaign_vacuum_photon_scenario_diagnostic.cpp").read_text(
                    encoding="utf-8")
    cmake = (ROOT / "engine/CMakeLists.txt").read_text(encoding="utf-8")
    checks.extend([
        ("DOC identifier frozen", "FTD-0434" in prereg),
        ("DOC records revision-1 dashboard invalidity",
         "revision-1 output is preserved" in words
         and "No revision-1 dashboard number is reused" in words),
        ("DOC locks exact scenario and L", "s0-vacuum-photon" in prereg
         and "L=33" in prereg),
        ("DOC locks dashboard and wave-only arms",
         "`dashboard`" in prereg and "`wave_only`" in prereg),
        ("DOC locks right-moving relation",
         "W_z\\simeq-C_{\\rm WAVE}D_xJ_z" in prereg),
        ("DOC freezes browser dashboard profile",
         "SCALE0_TOGGLES" in prereg
         and "FluxBoundaryMode::Dispersal" in prereg),
        ("DOC locks translation discriminator",
         "TRANSLATING PACKET" in prereg and "at least 8 sites" in words),
        ("DOC refuses visualization substitution",
         "rendered streamlines" in words
         and "plane-wave tests elsewhere cannot substitute" in words),
        ("SCENARIO declares Jz but injects Wx",
         "IF(rb, x, y, z, 0.0, 0.0, g)" in scenario
         and "IW(rb, x, y, z, g, 0.0, 0.0)" in scenario),
        ("CAMPAIGN dispatches exact scenario twice",
         "dispatch_scenario(bridge, \"s0-vacuum-photon\")" in campaign
         and "run_arm(\"dashboard\")" in campaign
         and "run_arm(\"wave_only\")" in campaign),
        ("CAMPAIGN reproduces browser dashboard defaults",
         "configure_dashboard_defaults" in campaign
         and "t.flux_boundary = ftd::FluxBoundaryMode::Dispersal" in campaign
         and "t.lorentz_force = false" in campaign
         and "t.dual_substrate = false" in campaign),
        ("CAMPAIGN locks tick set", "kFinalTick = 24" in campaign
         and "tick <= kFinalTick" in campaign),
        ("CAMPAIGN measures component energies and divergence",
         "flux_component" in campaign and "wave_component" in campaign
         and "divergence_normalized" in campaign),
        ("CAMPAIGN measures translation and width",
         "displacement_x" in campaign and "width_x" in campaign
         and "best_shift_overlap" in campaign),
        ("CAMPAIGN keeps observer read-only",
         "set_state" not in campaign and "inject_flux" not in campaign),
        ("BUILD registers disabled diagnostic",
         "ftd_add_test(campaign_vacuum_photon_scenario_diagnostic" in cmake
         and "set_tests_properties(campaign_vacuum_photon_scenario_diagnostic"
         in cmake),
        ("LOCK sealed before first run",
         lock["revision"] == 2
         and lock["locked_before_diagnostic_execution"] is True
         and lock["state"]
         == "V2_SOURCE_LOCKED_BEFORE_CORRECTED_DASHBOARD_RUN"),
    ])

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(f"\nVacuum-photon diagnostic lock checks: "
          f"{len(checks) - failed}/{len(checks)} passed")
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
