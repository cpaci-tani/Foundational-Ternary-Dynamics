"""Verify the FTD-0429 preregistration and source lock."""

from __future__ import annotations

from hashlib import sha256
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LOCK = ROOT / "scripts/proofs/native_dynamic_polarity_response_lock.json"
PREREG = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
    / "PREREG_NATIVE_DYNAMIC_POLARITY_RESPONSE_v2.md"
)


def main() -> None:
    lock = json.loads(LOCK.read_text(encoding="utf-8"))
    checks: list[tuple[str, bool]] = []

    prereg_hash = sha256(PREREG.read_bytes()).hexdigest()
    checks.append(("LOCK preregistration", prereg_hash == lock["preregistration_sha256"]))
    for relative, expected in lock["sources"].items():
        actual = sha256((ROOT / relative).read_bytes()).hexdigest()
        successor = lock.get("qualified_successors", {}).get(relative, {})
        accepted = actual == expected or (
            successor.get("identifier") in {"FTD-0430", "FTD-0431", "FTD-0432", "FTD-0433", "FTD-0434", "FTD-0436"}
            and actual == successor.get("sha256")
        ) or any(
            item.get("identifier") in {"FTD-0430", "FTD-0431", "FTD-0432", "FTD-0433", "FTD-0434", "FTD-0436"}
            and actual == item.get("sha256")
            for item in successor.get("prior", [])
        )
        checks.append((f"LOCK {relative}", accepted))

    prereg = PREREG.read_text(encoding="utf-8")
    prereg_words = " ".join(prereg.split())
    observer = (
        ROOT / "engine/include/ftd/eft/native_dynamic_polarity_response.h"
    ).read_text(encoding="utf-8")
    campaign = (
        ROOT / "engine/tests/campaign_native_dynamic_polarity_response.cpp"
    ).read_text(encoding="utf-8")
    cmake = (ROOT / "engine/CMakeLists.txt").read_text(encoding="utf-8")

    checks.extend(
        [
            ("DOC identifier frozen", lock["identifier"] in prereg),
            ("DOC preserves v1 estimators and thresholds",
             "Sections 1–7 of v1 remain normative" in prereg_words
             and "every `10^-8`, `10^-7`, `10^-6`" in prereg_words),
            ("DOC freezes zero-projector native sector",
             "both Gauss mechanisms off" in prereg_words),
            ("DOC locks zero-intercept competitor",
             "constant-versus-zero infrared model" in prereg_words),
            ("DOC records v1 invalid timeout without physics inference",
             "outcome D at the instrumentation layer" in prereg_words
             and "no `L=64` response value was observed" in prereg_words),
            ("OBSERVER is read-only",
             "const RenderBridge& bridge" in observer
             and "bridge.set_" not in observer
             and "bridge.inject" not in observer),
            ("OBSERVER uses exact central-divergence symbol",
             "std::sin(out.k[axis])" in observer
             and "std::complex<double>(0.0, 1.0)" in observer),
            ("OBSERVER contains no Gauss solve",
             "gauss_project" not in observer and "poisson" not in observer.lower()),
            ("CAMPAIGN enables only wave and coupling",
             "toggles.wave_propagation = true" in campaign
             and "toggles.coupling = true" in campaign
             and "gauss_projection = true" not in campaign),
            ("CAMPAIGN locks directions and harmonics",
             "{1, 0, 0}, {1, 1, 0}, {1, 1, 1}" in campaign
             and "std::vector<int>{1, 3}" in campaign
             and "std::vector<int>{2}" in campaign),
            ("CAMPAIGN locks mirror and amplitude controls",
             "for (int orientation : {1, -1})" in campaign
             and "for (int duty : {1, 4})" in campaign),
            ("CAMPAIGN accepts only preregistered volumes",
             "args.L != 32 && args.L != 64" in campaign),
            ("CAMPAIGN locks versioned profiles",
             "args.L == 32 && args.profile != \"full\"" in campaign
             and "args.L == 64 && args.profile != \"infrared\"" in campaign
             and "arm.duty == 2 && arm.orientation == 1" in campaign),
            ("BUILD registers unit and campaign targets",
             "ftd_add_test(test_native_dynamic_polarity_response" in cmake
             and "ftd_add_test(campaign_native_dynamic_polarity_response" in cmake),
            ("LOCK was sealed before first run",
             lock["revision"] == 2
             and lock["locked_before_implementation_run"] is True
             and lock["state"] == "V2_SOURCE_LOCKED_BEFORE_FIRST_L64_RERUN"),
        ]
    )

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(
        f"\nNative dynamic polarity-response lock checks: "
        f"{len(checks) - failed}/{len(checks)} passed"
    )
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
