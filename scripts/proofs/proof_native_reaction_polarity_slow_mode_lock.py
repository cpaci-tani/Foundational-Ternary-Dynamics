"""Verify the FTD-0431 preregistration and source lock."""

from __future__ import annotations

from hashlib import sha256
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LOCK = ROOT / "scripts/proofs/native_reaction_polarity_slow_mode_lock.json"


def main() -> None:
    lock = json.loads(LOCK.read_text(encoding="utf-8"))
    prereg_path = ROOT / lock["preregistration"]
    checks: list[tuple[str, bool]] = []

    checks.append((
        "LOCK preregistration",
        sha256(prereg_path.read_bytes()).hexdigest()
        == lock["preregistration_sha256"],
    ))
    for relative, expected in lock["sources"].items():
        actual = sha256((ROOT / relative).read_bytes()).hexdigest()
        successor = lock.get("qualified_successors", {}).get(relative, {})
        candidates = [successor] + successor.get("prior", [])
        accepted = actual == expected.lower() or any(
            item.get("identifier") in {"FTD-0434", "FTD-0436"}
            and actual == item.get("sha256")
            and "Registers the FTD-04" in item.get("scope", "")
            for item in candidates
        )
        checks.append((f"LOCK {relative}", accepted))

    prereg = prereg_path.read_text(encoding="utf-8")
    prereg_words = " ".join(prereg.split())
    observer = (
        ROOT / "engine/include/ftd/eft/native_reaction_polarity_slow_mode.h"
    ).read_text(encoding="utf-8")
    campaign = (
        ROOT / "engine/tests/campaign_native_reaction_polarity_slow_mode.cpp"
    ).read_text(encoding="utf-8")
    cmake = (ROOT / "engine/CMakeLists.txt").read_text(encoding="utf-8")

    checks.extend(
        [
            ("DOC identifier frozen", lock["identifier"] in prereg),
            ("DOC isolates the evaporation discriminator",
             "isolated evaporation" in prereg_words
             and "coupled evaporation" in prereg_words
             and "locked control" in prereg_words),
            ("DOC locks exact isolated decay",
             "0.105360515657826" in prereg
             and "gamma_{\\rm isolated}" in prereg),
            ("DOC locks source rather than field persistence",
             "homogeneous field roots remain unit-modulus source-free wave modes"
             in prereg_words
             and "They do not count as a conserved charge mode" in prereg_words),
            ("DOC locks the two infrared models",
             "M_0:" in prereg and "M_{\\rm cons}:" in prereg
             and "C/L^3" in prereg),
            ("DOC locks finite-decay outcome A",
             "Outcome A requires all of" in prereg_words
             and "gamma_0 > 0.02" in prereg
             and "gamma_0-1.96 sigma_J" in prereg),
            ("DOC locks CPU and CUDA profiles",
             "L=32`, profile `full" in prereg
             and "L=64`, profile `infrared" in prereg),
            ("DOC locks exact field recurrence",
             "D_{t+1}=(2-C_{\\rm WAVE}^2M_{18}(k))D_t-D_{t-1}"
             in prereg_words
             and "+G_C\\sum_a\\sin^2(k_a)S_t" in prereg_words),
            ("OBSERVER is read-only",
             "const RenderBridge& bridge" in observer
             and ".set_state" not in observer
             and ".tick()" not in observer),
            ("OBSERVER uses phase-referenced source amplitude",
             "source * std::conj(initial_source)" in observer),
            ("OBSERVER fits decay by ordinary least squares",
             "fit_native_source_decay" in observer
             and "std::log(amplitude)" in observer),
            ("CAMPAIGN activates only locked reaction sectors",
             "bridge.toggles.evaporation = true" in campaign
             and "bridge.toggles.genesis" not in campaign
             and "bridge.toggles.pair_production" not in campaign),
            ("CAMPAIGN records CPU reaction history",
             "bridge.enable_history_journal(true)" in campaign
             and "HistoryEventKind::Evaporation" in campaign),
            ("CAMPAIGN locks directions, modes, seeds, and ticks",
             "{1, 0, 0}, {1, 1, 0}, {1, 1, 1}" in campaign
             and "for (int n : {1, 2, 3})" in campaign
             and "for (int seed = 0; seed < 8; ++seed)" in campaign
             and "for (int tick = 1; tick <= kFinalTick; ++tick)" in campaign),
            ("CAMPAIGN accepts only locked profiles",
             "args.L != 32 && args.L != 64" in campaign
             and "args.L == 32 && args.profile != \"full\"" in campaign
             and "args.L == 64 && args.profile != \"infrared\"" in campaign),
            ("BUILD registers unit and disabled campaign",
             "ftd_add_test(test_native_reaction_polarity_slow_mode" in cmake
             and "ftd_add_test(campaign_native_reaction_polarity_slow_mode"
             in cmake
             and "set_tests_properties(campaign_native_reaction_polarity_slow_mode"
             in cmake),
            ("LOCK sealed before first campaign run",
             lock["revision"] == 1
             and lock["locked_before_campaign_execution"] is True
             and lock["state"]
             == "V1_SOURCE_LOCKED_BEFORE_FIRST_CAMPAIGN_RUN"),
        ]
    )

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(
        f"\nNative reaction-polarity slow-mode lock checks: "
        f"{len(checks) - failed}/{len(checks)} passed"
    )
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
