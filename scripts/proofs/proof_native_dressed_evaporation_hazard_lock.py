"""Verify the FTD-0432 preregistration and source lock."""

from __future__ import annotations

from hashlib import sha256
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LOCK = ROOT / "scripts/proofs/native_dressed_evaporation_hazard_lock.json"


def main() -> None:
    lock = json.loads(LOCK.read_text(encoding="utf-8"))
    prereg_path = ROOT / lock["preregistration"]
    checks: list[tuple[str, bool]] = []
    checks.append(("LOCK preregistration",
                   sha256(prereg_path.read_bytes()).hexdigest()
                   == lock["preregistration_sha256"]))
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
    observer = (ROOT / "engine/include/ftd/eft/"
                "native_evaporation_hazard_observer.h").read_text(
                    encoding="utf-8")
    unit = (ROOT / "engine/tests/"
            "test_native_evaporation_hazard_observer.cpp").read_text(
                encoding="utf-8")
    campaign = (ROOT / "engine/tests/"
                "campaign_native_dressed_evaporation_hazard.cpp").read_text(
                    encoding="utf-8")
    cmake = (ROOT / "engine/CMakeLists.txt").read_text(encoding="utf-8")
    checks.extend([
        ("DOC identifier frozen", "FTD-0432" in prereg),
        ("DOC locks exact production hazard",
         "p_i=K_{\\rm EVAP\\_RATE}" in prereg
         and "exp[-E_i/K_{\\rm MANIFEST}^2]" in prereg),
        ("DOC locks conditional source expectation",
         "E[S_k(t+1)|X_t]=S_k(t)-L_k" in prereg_words),
        ("DOC prohibits conservation inference",
         "cannot establish conservation" in prereg_words
         and "may not reuse a finite-time plateau" in prereg_words),
        ("DOC locks modes, seeds, and transitions",
         "d=<100>, n=1" in prereg and "d=<110>, n=2" in prereg
         and "d=<111>, n=3" in prereg
         and "seeds `0,...,7`" in prereg
         and "32 transitions" in prereg_words),
        ("DOC locks standardized residual gates",
         "maximum standardized residual `<= 6`" in prereg
         and "RMS standardized residual `<= 2.5`" in prereg),
        ("OBSERVER is counterfactual and RNG-free",
         "bridge.prepare_delta_j()" in observer
         and "voxel_uniform" not in observer
         and ".tick()" not in observer),
        ("OBSERVER reproduces standard wave write",
         "voxels[index].wave_vel + delta[index]" in observer
         and "voxels[index].flux" in observer
         and "+ predicted_velocity[index]" in observer),
        ("OBSERVER evaluates six-neighbor energy and proper time",
         "lattice.neighbors_6(index)" in observer
         and "proper_time_rate" in observer
         and "K_EVAP_RATE" in observer),
        ("UNIT pins state and RNG neutrality",
         "bridge_state_hash(control) == bridge_state_hash(observed)" in unit
         and "control.rng_state_hash() == observed.rng_state_hash()" in unit),
        ("CAMPAIGN locks representative matrix",
         "{{{1, 0, 0}}, 1}" in campaign
         and "{{{1, 1, 0}}, 2}" in campaign
         and "{{{1, 1, 1}}, 3}" in campaign
         and "seed < 8" in campaign
         and "kTransitions = 32" in campaign),
        ("CAMPAIGN journals CPU evaporation",
         "HistoryEventKind::Evaporation" in campaign
         and "record.history_evaporation == record.actual_removed" in campaign),
        ("BUILD registers unit and disabled campaign",
         "ftd_add_test(test_native_evaporation_hazard_observer" in cmake
         and "ftd_add_test(campaign_native_dressed_evaporation_hazard" in cmake
         and "set_tests_properties(campaign_native_dressed_evaporation_hazard"
         in cmake),
        ("LOCK sealed before campaign",
         lock["revision"] == 1
         and lock["locked_before_campaign_execution"] is True
         and lock["state"] == "V1_SOURCE_LOCKED_BEFORE_FIRST_CAMPAIGN_RUN"),
    ])

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(f"\nNative dressed-hazard lock checks: "
          f"{len(checks) - failed}/{len(checks)} passed")
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
