"""Verify the frozen FTD-0427 protocol and selected-mechanism source lock."""

from __future__ import annotations

from hashlib import sha256
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LOCK = ROOT / "scripts/proofs/matched_gauss_transport_lock.json"


def main() -> None:
    lock = json.loads(LOCK.read_text(encoding="utf-8"))
    checks: list[tuple[str, bool]] = []
    successor = lock.get("qualified_successor", {}).get("files", {})
    for relative, expected in lock["files"].items():
        actual = sha256((ROOT / relative).read_bytes()).hexdigest()
        original = actual == expected
        qualified = actual == successor.get(relative)
        label = "LOCK" if original else "QUALIFIED-SUCCESSOR"
        checks.append((f"{label} {relative}", original or qualified))

    prereg = (ROOT / "docs/theory/10_eft_program/preregistrations/"
                     "PREREG_MATCHED_GAUSS_TRANSPORT_v1.md").read_text(encoding="utf-8")
    header = (ROOT / "engine/include/ftd/eft/matched_gauss_transport.h").read_text(
        encoding="utf-8"
    )
    source = (ROOT / "engine/src/eft/matched_gauss_transport.cpp").read_text(
        encoding="utf-8"
    )
    campaign = (ROOT / "engine/tests/campaign_matched_gauss_transport.cpp").read_text(
        encoding="utf-8"
    )
    cmake = (ROOT / "engine/CMakeLists.txt").read_text(encoding="utf-8")
    protocol = lock["protocol"]

    checks.extend(
        [
            ("DOC identifier frozen", lock["identifier"] in prereg),
            ("DOC selected status is explicit",
             "[SELECTED MECHANISM]" in prereg and "not native charge emergence" in prereg),
            ("DOC preserves FTD-0421 closure",
             "FTD-0421" in prereg and "does not" in prereg),
            ("DOC forbids projector", "gauss_projection=false" in prereg),
            ("HEADER marks sidecar non-mutating",
             "does not replace or mutate" in header),
            ("SRC uses matched backward curl",
             "MatchedFaceFlux matched_curl" in source and "divergence_at" in source),
            ("SRC rejects reactions",
             "out.reaction_l1 != 0" in source),
            ("CAMPAIGN projector stays off",
             "gauss_projection = true" not in campaign and
             "!rb.toggles.gauss_projection" in campaign),
            ("CAMPAIGN production movement is enabled",
             "rb.toggles.movement = true" in campaign and "rb.tick()" in campaign),
            ("CAMPAIGN schedule matches lock",
             f"kMovingTicks = {protocol['moving_ticks']}" in campaign and
             f"kStationaryTicks = {protocol['stationary_ticks']}" in campaign and
             "kRadii{2, 3, 4}" in campaign),
            ("CAMPAIGN mirror and six directions",
             "for (int orientation : {+1, -1})" in campaign and
             all(f'{{"{direction}"' in campaign for direction in protocol["directions"])),
            ("BUILD registers both FTD-0427 targets",
             "ftd_add_test(test_matched_gauss_transport" in cmake and
             "ftd_add_test(campaign_matched_gauss_transport" in cmake),
        ]
    )

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(f"\nMatched Gauss transport lock checks: {len(checks) - failed}/{len(checks)} passed")
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
