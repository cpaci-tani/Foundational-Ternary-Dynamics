#!/usr/bin/env python3
"""FTD-0909 pre-run lock check for the FTD-0908 census instrument."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_PRODUCTION_NEUTRAL_DIPOLE_PHASE_WEDGE_FORMATION_CENSUS_v1.md"
)
RUNNER = ROOT / "engine/tests/campaign_production_orientation_memory_census.cpp"
ADJUDICATOR = ROOT / (
    "scripts/proofs/"
    "proof_production_neutral_dipole_phase_wedge_formation_census_result.py"
)
CMAKE = ROOT / "engine/CMakeLists.txt"
EXECUTABLE = ROOT / "engine/build/Release/campaign_production_orientation_memory_census.exe"

LOCKS = {
    PROTOCOL: "53348A90021C609E3EBA5DC7D565F6EA78832498C206D0D4B3F1964CCC7C4993",
    RUNNER: "4FBA0AF9F02440CCA7B166BFFD1A5C2875B18D86B4E402E004F23C4412CB9F34",
    ADJUDICATOR: "26FD25DA518F1FA000C3DCBC459CEAEC54871950267DAAD61CEB89946F0F2A6A",
    CMAKE: "51EFD78AEEEBBCCBD4CCC58FB96969C0826BCFF8BFAEB12A4BC79DDF5B05E841",
    EXECUTABLE: "83EE291952AFED3A70921A8DC1C6ABEF56275485B714961E7FB6BDDCBC644DD8",
    ROOT / "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    ROOT / "engine/include/ftd/render_bridge.h":
        "560CB59E2FCD6E174640CA6BF048FD16AEC36AD2B13EE97FA31E301CF373D91C",
    ROOT / "engine/src/render_bridge.cpp":
        "BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724",
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    ROOT / "engine/include/ftd/constants.h":
        "5C9E4EA46DE1D5E0BF4479AA9E115520E70B729E7E81335FCEF08CE99704BAB0",
    ROOT / "engine/include/ftd/eft/native_ternary_dipole_phase_wedge_memory.h":
        "BADAE9D26E5FED6FCD4317A7534648256AFF051E2CAADB7E6BEEA00603AEDF46",
    ROOT / "engine/src/eft/native_ternary_dipole_phase_wedge_memory.cpp":
        "AA021926D1DE32AE9D04FB72682379DBB7F6CD3A1BB150AADBA6A957DFBF20B5",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


checks: list[tuple[str, bool]] = []


def check(name: str, condition: bool) -> None:
    checks.append((name, bool(condition)))


for path, expected in LOCKS.items():
    check(f"lock {path.relative_to(ROOT)}", path.is_file() and sha256(path) == expected)

runner = RUNNER.read_text(encoding="utf-8")
adjudicator = ADJUDICATOR.read_text(encoding="utf-8")
cmake = CMAKE.read_text(encoding="utf-8")

check("runner embeds parent protocol hash", LOCKS[PROTOCOL] in runner)
check("runner freezes 96 ticks", "constexpr int kTicks = 96;" in runner)
check("runner freezes eight-tick persistence", "constexpr int kPersistenceTicks = 8;" in runner)
check("runner freezes 1e-11 analyzer tolerance", "constexpr double kTolerance = 1e-11;" in runner)
check("runner freezes volumes 17 and 25", "kVolumes{{17, 25}}" in runner)
check("runner freezes all four seeds", all(
    marker in runner for marker in (
        "0x09080001u", "0x09080002u", "0x09080003u", "0x09080004u"
    )
))
check("runner freezes all four arms", all(
    marker in runner for marker in (
        "AxialLive", "DiagonalLive", "AxialNoBath", "EmptyControl"
    )
))
check("runner uses production particle IDs", all(
    marker in runner for marker in (
        "positive.particle_id", "negative.particle_id", "PairKey"
    )
))
check("runner uses Moore neighbors", "neighbors_26" in runner)
check("runner audits voxel and RNG nonmutation", all(
    marker in runner for marker in (
        "voxel_hash_before == observation.voxel_hash_after",
        "rng_hash_before == observation.rng_hash_after",
    )
))
check("runner writes endpoint reconstruction fields", all(
    marker in runner for marker in (
        "q_plus,q_minus,p_plus,p_minus,ell,chi", "jpx,jpy,jpz", "wpx,wpy,wpz"
    )
))
check("runner keeps central-law and work firewalls", all(
    marker in runner for marker in (
        "CENTRAL_MEMORY_LAW_TESTED=FALSE",
        "MAINTENANCE_ERASURE_WORK_CLOSED=FALSE",
        "PRODUCTION_TICK_MODIFIED=FALSE",
    )
))
check("CMake registers only the frozen campaign runner", all(
    marker in cmake for marker in (
        "ftd_add_test(campaign_production_orientation_memory_census",
        "tests/campaign_production_orientation_memory_census.cpp",
        "CTEST_NAME production_orientation_memory_census",
    )
))
check("independent adjudicator embeds runner lock", LOCKS[RUNNER] in adjudicator)
check("independent adjudicator freezes A/B/C matrix", all(
    marker in adjudicator for marker in (
        "TICKS = 96", "PERSISTENCE_TICKS = 8", "VOLUMES = (17, 25)",
        "CROSS_VOLUME_PERSISTENT_ORIENTATION_MEMORY_CANDIDATES",
        "FORMATION_WITHOUT_CROSS_VOLUME_PERSISTENCE",
        "NO_OBSERVED_LOCAL_ORIENTATION_MEMORY_FORMATION",
    )
))
check("independent adjudicator reconstructs endpoint wedge", all(
    marker in adjudicator for marker in (
        "q_plus_rebuilt", "ell_rebuilt", "gram_det", "random_count"
    )
))

passed = sum(ok for _, ok in checks)
for name, ok in checks:
    print(f"{'PASS' if ok else 'FAIL'}  {name}")
print(f"\nFTD-0909 runner lock preflight: {passed}/{len(checks)} checks passed")
print("CAMPAIGN_EXECUTED=FALSE")
print("RUNNER_AND_ADJUDICATOR_FROZEN=TRUE")
print("PRODUCTION_FORMATION_VERDICT=NOT_YET_AVAILABLE")
raise SystemExit(0 if passed == len(checks) else 1)
