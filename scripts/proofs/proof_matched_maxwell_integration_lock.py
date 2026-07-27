"""Verify the frozen FTD-0428 protocol and integrated source lock."""

from __future__ import annotations

from hashlib import sha256
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LOCK = ROOT / "scripts/proofs/matched_maxwell_integration_lock.json"


def main() -> None:
    lock = json.loads(LOCK.read_text(encoding="utf-8"))
    checks: list[tuple[str, bool]] = []
    for relative, expected in lock["files"].items():
        actual = sha256((ROOT / relative).read_bytes()).hexdigest()
        checks.append((f"LOCK {relative}", actual == expected))

    prereg = (ROOT / "docs/theory/10_eft_program/preregistrations/"
                     "PREREG_MATCHED_MAXWELL_INTEGRATION_v1.md").read_text(
        encoding="utf-8"
    )
    header = (ROOT / "engine/include/ftd/eft/matched_gauss_transport.h").read_text(
        encoding="utf-8"
    )
    source = (ROOT / "engine/src/eft/matched_gauss_transport.cpp").read_text(
        encoding="utf-8"
    )
    toggles = (ROOT / "engine/include/ftd/term_toggles.h").read_text(
        encoding="utf-8"
    )
    bridge = (ROOT / "engine/src/render_bridge.cpp").read_text(encoding="utf-8")
    campaign = (ROOT / "engine/tests/campaign_matched_maxwell_integration.cpp").read_text(
        encoding="utf-8"
    )
    cmake = (ROOT / "engine/CMakeLists.txt").read_text(encoding="utf-8")

    checks.extend(
        [
            ("DOC identifier and selected status",
             lock["identifier"] in prereg and "[SELECTED ENGINE EXTENSION]" in prereg),
            ("DOC preserves native reaction closure",
             "FTD-0421 remains controlling" in prereg),
            ("DOC forbids per-tick projection",
             "No Poisson solve or projection is permitted after initialization" in prereg),
            ("HEADER owns staggered face/edge state",
             "MatchedFaceFlux electric_" in header and
             "MatchedEdgeField magnetic_half_" in header),
            ("SRC implements minimum-energy solve",
             "initialize_minimum_energy" in source and "apply_ddt" in source),
            ("SRC implements exact transpose update",
             "matched_curl_adjoint" in source and
             "magnetic_half_.x[i] -= scale" in source),
            ("TOGGLE default false and CPU-scoped",
             "bool matched_gauss_dynamics = false" in toggles and
             "ToggleBackend::CPU" in toggles),
            ("TOGGLE isolates legacy writers",
             "requires the isolated conservative movement sector" in toggles),
            ("BRIDGE routes production movement history",
             "extract_moore_history_from_snapshots" in bridge and
             "matched_gauss_dynamics_->advance" in bridge),
            ("BRIDGE mirrors centered field",
             "centered_electric_at" in bridge and
             "sync_matched_gauss_to_voxels" in bridge),
            ("CAMPAIGN exact preregistered sizes and schedule",
             "radius = 2; radius <= 5" in campaign and
             "tick <= 20" in campaign and "tick <= 32" in campaign),
            ("CAMPAIGN tests both polarities and three axes",
             "for (int polarity : {+1, -1})" in campaign and
             "for (int axis = 0; axis < 3" in campaign),
            ("CAMPAIGN projector never enabled",
             "gauss_projection = true" not in campaign),
            ("BUILD registers FTD-0428 unit and campaign targets",
             "ftd_add_test(test_matched_maxwell_integration" in cmake and
             "ftd_add_test(campaign_matched_maxwell_integration" in cmake),
        ]
    )

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(f"\nMatched Maxwell integration lock checks: "
          f"{len(checks) - failed}/{len(checks)} passed")
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
