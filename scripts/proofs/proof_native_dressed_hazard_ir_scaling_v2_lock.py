"""Verify the FTD-0436 preregistration and source lock (v2 campaign)."""

from __future__ import annotations

from hashlib import sha256
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LOCK = ROOT / "scripts/proofs/native_dressed_hazard_ir_scaling_v2_lock.json"


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
            actual == item.get("sha256") and item.get("identifier", "")
            and item.get("scope", "")
            for item in [successor] + successor.get("prior", [])
        )
        checks.append((f"LOCK {relative}", accepted))

    prereg = prereg_path.read_text(encoding="utf-8")
    prereg_words = " ".join(prereg.split())
    campaign = (ROOT / "engine/tests/"
                "campaign_native_dressed_hazard_ir_scaling_v2.cpp").read_text(
                    encoding="utf-8")
    observer = (ROOT / "engine/include/ftd/eft/"
                "native_evaporation_hazard_observer.h").read_text(
                    encoding="utf-8")
    results = (ROOT / "scripts/proofs/"
               "proof_native_dressed_hazard_ir_scaling_v2_results.py"
               ).read_text(encoding="utf-8")
    cmake = (ROOT / "engine/CMakeLists.txt").read_text(encoding="utf-8")
    checks.extend([
        ("DOC identifier frozen", lock["identifier"] in prereg
         and lock["identifier"] == "FTD-0436"),
        ("DOC extends FTD-0433 without reopening it",
         "extends FTD-0433" in prereg_words
         and "does not reopen or modify any FTD-0431/0432/0433 locked"
         in prereg_words),
        ("DOC locks phase-corrected estimator",
         "tau_L = pi/omega_L - 1" in prereg
         and "h_L^phase = h(t^*) + b1*d + a2*d^2" in prereg),
        ("DOC locks primary volumes and seeds",
         "L in {48, 64, 96, 128, 192}" in prereg
         and "seeds `0..7`" in prereg),
        ("DOC locks bracket recording",
         "t = 0 ... t_L^* + 2" in prereg),
        ("DOC locks model contest and thresholds",
         "BIC_1 = chi2_1 + 2 ln 5" in prereg
         and "BIC_2 = chi2_2 + 3 ln 5" in prereg
         and "dBIC >= +10" in prereg and "dBIC <= -10" in prereg
         and "{0.20, 0.22, ..., 3.00}" in prereg),
        ("DOC locks floor significance clause",
         "h_inf > 2 * max_L sigma_L" in prereg),
        ("DOC locks continuity gate G8",
         "0.00351325" in prereg and "0.00001889" in prereg),
        ("DOC locks plane-closure gate G9",
         "1e-12" in prereg and "expected_loss_source" in prereg_words),
        ("DOC prohibits asymptotic overclaim",
         "NOT an asymptotic theorem" in prereg_words
         and "No other functional families, grids, or estimator variants"
         in prereg_words),
        ("DOC registers non-gating surrogate comparators",
         "no gate references them" in prereg_words),
        ("CAMPAIGN locks registered volumes (A1 matrix)",
         "kGpuVolumes{48, 64, 96}" in campaign
         and "kCpuVolumes{48, 128, 192}" in campaign
         and "kCpuVolume = 48" in campaign),
        ("CAMPAIGN records interpolation bracket",
         "kBracket = 2" in campaign
         and "tick <= last_transition" in campaign),
        ("CAMPAIGN computes target from exact pole",
         "native_discrete_pole({k, 0.0, 0.0})" in campaign
         and "std::llround" in campaign),
        ("CAMPAIGN activates only registered sectors",
         "bridge.toggles.evaporation = true" in campaign
         and "bridge.toggles.wave_propagation = true" in campaign
         and "bridge.toggles.coupling = true" in campaign
         and "toggle_contract" in campaign),
        ("CAMPAIGN enforces plane closure at t*",
         "plane_closure_rel <= 1e-12" in campaign
         and "plane_loss_decomposition" in campaign),
        ("CAMPAIGN journals CPU evaporation",
         "HistoryEventKind::Evaporation" in campaign
         and "record.history_evaporation == record.actual_removed"
         in campaign),
        ("CAMPAIGN enforces neutral full source",
         "out.initial_signed_state == 0" in campaign
         and "std::abs(out.initial_source) >= 0.3" in campaign),
        ("OBSERVER remains RNG free",
         "voxel_uniform" not in observer and ".tick()" not in observer),
        ("RESULTS implements locked grid and thresholds",
         "P_GRID = [0.20 + 0.02 * i for i in range(141)]" in results
         and "dbic >= 10.0 and hinf > 2.0 * max_sig" in results
         and "dbic <= -10.0" in results),
        ("RESULTS implements locked estimator",
         "h[t_star] + b1 * d + a2 * d * d" in results),
        ("BUILD registers disabled v2 campaign",
         "ftd_add_test(campaign_native_dressed_hazard_ir_scaling_v2" in cmake
         and "set_tests_properties(campaign_native_dressed_hazard_ir_scaling_v2"
         in cmake),
        ("LOCK sealed before first campaign, amendment pre-data",
         lock["revision"] in (1, 2)
         and lock["locked_before_campaign_execution"] is True
         and lock["state"] in (
             "V2_SOURCE_LOCKED_BEFORE_FIRST_CAMPAIGN_RUN",
             "V2R2_AMENDMENT_A1_SEALED_BEFORE_ANY_L128_L192_DATA")
         and (lock["revision"] == 1
              or "Amendment A1" in prereg)),
    ])

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(f"\nNative dressed-hazard IR v2 lock checks: "
          f"{len(checks) - failed}/{len(checks)} passed")
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
