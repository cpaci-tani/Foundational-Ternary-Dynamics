"""Verify the FTD-0426 pre-measurement protocol and source lock."""

from __future__ import annotations

from hashlib import sha256
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LOCK = ROOT / "scripts/proofs/emergent_static_charge_lock.json"


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

    prereg = (ROOT / next(
        path for path in lock["files"] if path.startswith("docs/")
    )).read_text(encoding="utf-8")
    source = (ROOT / "engine/tests/campaign_emergent_static_charge.cpp").read_text(
        encoding="utf-8"
    )
    observer = (
        ROOT / "engine/include/ftd/eft/emergent_charge_surface.h"
    ).read_text(encoding="utf-8")
    cmake = (ROOT / "engine/CMakeLists.txt").read_text(encoding="utf-8")

    protocol = lock["protocol"]
    checks.extend(
        [
            ("DOC identifier frozen", lock["identifier"] in prereg),
            ("DOC distinguishes readout from autonomous dressing",
             "Gauss-readout gate" in prereg and "Autonomous-dressing gate" in prereg),
            ("DOC preserves exact-charge closure",
             "FTD-0421" in prereg and "cannot override" in prereg),
            ("DOC excludes circular force promotion",
             "No force-law test is used for promotion" in prereg),
            ("SRC radii match lock",
             "kRadii{3, 4, 5, 6}" in source and protocol["radii"] == [3, 4, 5, 6]),
            ("SRC tick schedule matches lock",
             all(
                 token in source
                 for token in (
                     f"kNeutralTicks = {protocol['neutral_ticks']}",
                     f"kProjectedTicks = {protocol['projected_ticks']}",
                     f"kLiveTicks = {protocol['live_ticks']}",
                     f"kSorIterations = {protocol['sor_iterations']}",
                 )
             )),
            ("SRC mirror arms are explicit",
             "run_arm(args, +1)" in source and "run_arm(args, -1)" in source),
            ("SRC uses production movement",
             "rb.toggles.movement = true" in source and "prime_positive_x_hop" in source),
            ("SRC does not enable Poisson-Coulomb force",
             "poisson_coulomb = true" not in source and "toggles.forces = true" not in source),
            ("OBS is read-only",
             all(token not in observer for token in (
                 "set_state(", "voxel_at(", "inject_", ".tick(", ".run("
             ))),
            ("OBS records telescope and Gauss residuals",
             "telescope_residual" in observer and "gauss_residual" in observer),
            ("BUILD retains FTD-0426 campaign registration",
             "ftd_add_test(campaign_emergent_static_charge" in cmake and
             "CTEST_NAME campaign_emergent_static_charge" in cmake),
        ]
    )

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(f"\nEmergent static-charge lock checks: {len(checks) - failed}/{len(checks)} passed")
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
