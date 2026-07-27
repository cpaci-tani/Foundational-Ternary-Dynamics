"""Verify the FTD-0430 preregistration and source lock."""

from __future__ import annotations

from hashlib import sha256
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LOCK = ROOT / "scripts/proofs/native_retarded_polarity_transport_lock.json"


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
        accepted = actual == expected or (
            successor.get("identifier") == "FTD-0430-POSTRUN-CTEST-DEFAULT"
            and actual == successor.get("sha256")
            and "default volume" in successor.get("scope", "")
        ) or (
            successor.get("identifier") in {"FTD-0431", "FTD-0432", "FTD-0433", "FTD-0434"}
            and actual == successor.get("sha256")
            and "Registers the FTD-0431" in successor.get("scope", "")
        ) or any(
            item.get("identifier") in {"FTD-0431", "FTD-0432", "FTD-0433",
                                       "FTD-0434", "FTD-0436"}
            and actual == item.get("sha256")
            for item in ([successor] + successor.get("prior", []))
        )
        checks.append((f"LOCK {relative}", accepted))

    prereg = prereg_path.read_text(encoding="utf-8")
    prereg_words = " ".join(prereg.split())
    observer = (
        ROOT / "engine/include/ftd/eft/native_retarded_polarity_response.h"
    ).read_text(encoding="utf-8")
    campaign = (
        ROOT / "engine/tests/campaign_native_retarded_polarity_transport.cpp"
    ).read_text(encoding="utf-8")
    cmake = (ROOT / "engine/CMakeLists.txt").read_text(encoding="utf-8")

    checks.extend(
        [
            ("DOC identifier frozen", lock["identifier"] in prereg),
            ("DOC records v1 analysis invalidity",
             "outcome D at the analysis- specification layer" in prereg_words
             and "No v1 scalar measurement is reused" in prereg_words),
            ("DOC freezes production movement step by inheritance",
             "Sections 1–5 and 7 of v1 remain normative" in prereg_words
             and "production one-cell hop" in prereg_words),
            ("DOC keeps both Gauss mechanisms off",
             "both Gauss mechanisms OFF" in prereg_words),
            ("DOC locks exact pole and static coefficient",
             "exact native pole, susceptibility" in prereg_words),
            ("DOC locks step residue",
             "step-residue predictions" in prereg_words
             and "`10^-5`" in prereg_words),
            ("DOC locks causal dependency cone",
             "causal-support estimators" in prereg_words
             and "`10^-11`" in prereg_words),
            ("DOC locks FTD-0429 equality gate",
             "Z_0^(FTD-0429)" in prereg
             and "<= 0.002" in prereg),
            ("DOC restores canonical normalized h4",
             "h_4=\\frac{\\sum_a k_a^4}{q^2}" in prereg
             and "single feature definition locked" in prereg_words),
            ("OBSERVER is read-only",
             "const RenderBridge& moving" in observer
             and ".set_state" not in observer
             and ".tick()" not in observer),
            ("OBSERVER batches the nine locked modes",
             "modes.reserve(9)" in observer
             and "for (int n : {1, 2, 3})" in observer),
            ("OBSERVER uses central divergence symbol",
             "std::sin(mode.k[axis])" in observer
             and "std::complex<double>(0.0, 1.0)" in observer),
            ("CAMPAIGN enables native movement",
             "toggles.movement = true" in campaign
             and "toggles.gauss_projection = true" not in campaign
             and "toggles.matched_gauss_dynamics = true" not in campaign),
            ("CAMPAIGN primes one production hop",
             "1.0 - speed" in campaign
             and "stationary.tick();" in campaign
             and "moving.tick();" in campaign),
            ("CAMPAIGN locks volumes and profiles",
             "args.L != 48 && args.L != 96" in campaign
             and "args.L == 48 && args.profile != \"full\"" in campaign
             and "args.L == 96 && args.profile != \"infrared\"" in campaign),
            ("BUILD registers unit and campaign",
             "ftd_add_test(test_native_retarded_polarity_response" in cmake
             and "ftd_add_test(campaign_native_retarded_polarity_transport"
             in cmake),
            ("LOCK sealed before first campaign run",
             lock["revision"] == 2
             and lock["locked_before_campaign_execution"] is True
             and lock["state"]
             == "V2_SOURCE_LOCKED_BEFORE_FIRST_L48_L96_RUN"),
        ]
    )

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(
        f"\nNative retarded polarity-transport lock checks: "
        f"{len(checks) - failed}/{len(checks)} passed"
    )
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
