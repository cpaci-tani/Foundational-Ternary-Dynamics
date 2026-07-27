"""Verify the FTD-0433 preregistration and source lock."""

from __future__ import annotations

from hashlib import sha256
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LOCK = ROOT / "scripts/proofs/native_dressed_hazard_ir_scaling_lock.json"


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
        candidates = [successor] + successor.get("prior", [])
        accepted = actual == expected or any(
            item.get("identifier") in {"FTD-0434", "FTD-0436"}
            and actual == item.get("sha256")
            and "Registers the FTD-04" in item.get("scope", "")
            for item in candidates
        )
        checks.append((f"LOCK {relative}", accepted))

    prereg = prereg_path.read_text(encoding="utf-8")
    prereg_words = " ".join(prereg.split())
    campaign = (ROOT / "engine/tests/"
                "campaign_native_dressed_hazard_ir_scaling.cpp").read_text(
                    encoding="utf-8")
    observer = (ROOT / "engine/include/ftd/eft/"
                "native_evaporation_hazard_observer.h").read_text(
                    encoding="utf-8")
    cmake = (ROOT / "engine/CMakeLists.txt").read_text(encoding="utf-8")
    checks.extend([
        ("DOC identifier frozen", lock["identifier"] in prereg),
        ("DOC locks one axial fundamental family",
         "fixed axial fundamental source family" in prereg_words
         and "d=<100>, n=1" in prereg),
        ("DOC locks native pole phase",
         "t_L^*=\\operatorname{round}(\\pi/\\omega_L)-1" in prereg
         and "No measured hazard is used to select a tick" in prereg_words),
        ("DOC locks primary volumes and seeds",
         "L\\in\\{12,16,20,24,32,40,48\\}" in prereg
         and "seeds `0,...,7`" in prereg),
        ("DOC prohibits asymptotic overclaim",
         "not proof of a zero asymptotic decay rate" in prereg_words
         and "No polynomial intercept fit is authorized" in prereg_words),
        ("DOC locks conditional-expectation gates",
         "maximum `<=6`" in prereg and "RMS `<=2.5`" in prereg),
        ("DOC locks outcome A discriminators",
         "h_48^*/h_12^*<=0.25" in prereg
         and "p_32,40" in prereg and "A_48^*>0.1" in prereg),
        ("CAMPAIGN locks registered volumes",
         "kGpuVolumes{12, 16, 20, 24, 32, 40, 48}" in campaign
         and "return args.L == 32" in campaign),
        ("CAMPAIGN computes target from exact pole",
         "native_discrete_pole({k, 0.0, 0.0})" in campaign
         and "std::llround" in campaign
         and "target_transition + 1" in campaign),
        ("CAMPAIGN runs eight seeds through target",
         "seed < kSeeds" in campaign
         and "tick <= out.pole.target_transition" in campaign),
        ("CAMPAIGN activates only registered sectors",
         "bridge.toggles.evaporation = true" in campaign
         and "bridge.toggles.wave_propagation = true" in campaign
         and "bridge.toggles.coupling = true" in campaign
         and "toggle_contract" in campaign),
        ("CAMPAIGN journals CPU evaporation",
         "HistoryEventKind::Evaporation" in campaign
         and "record.history_evaporation == record.actual_removed" in campaign),
        ("CAMPAIGN enforces neutral full source",
         "out.initial_signed_state == 0" in campaign
         and "std::abs(out.initial_source) >= 0.3" in campaign),
        ("CAMPAIGN enforces positive projection",
         "before_projection > 0.0 && after_projection > 0.0" in campaign),
        ("OBSERVER remains RNG free",
         "voxel_uniform" not in observer and ".tick()" not in observer),
        ("BUILD registers disabled campaign",
         "ftd_add_test(campaign_native_dressed_hazard_ir_scaling" in cmake
         and "set_tests_properties(campaign_native_dressed_hazard_ir_scaling"
         in cmake),
        ("LOCK sealed before first campaign",
         lock["revision"] == 1
         and lock["locked_before_campaign_execution"] is True
         and lock["state"] == "V1_SOURCE_LOCKED_BEFORE_FIRST_CAMPAIGN_RUN"),
    ])

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(f"\nNative dressed-hazard IR lock checks: "
          f"{len(checks) - failed}/{len(checks)} passed")
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
