#!/usr/bin/env python3
"""FTD-0912 pre-run instrument lock for the FTD-0911 campaign."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_HELD_OUT_PAIR_SPECIFIC_PHASE_WEDGE_AND_CENTRALITY_v1.md"
)
RUNNER = ROOT / "engine/tests/campaign_held_out_pair_specific_phase_wedge_centrality.cpp"
ADJUDICATOR = ROOT / "scripts/proofs/proof_held_out_pair_specific_phase_wedge_and_centrality_result.py"
CMAKE = ROOT / "engine/CMakeLists.txt"
EXECUTABLE = ROOT / "engine/build/Release/campaign_held_out_pair_specific_phase_wedge_centrality.exe"

LOCKS = {
    PROTOCOL: "D0C7976FE334EA5D814D40DADEDBEF9CB8419B0A518AFE0492C2F3A183FF88FE",
    RUNNER: "092954834F568DF2CCCB0F4908CE3E6E0212C45CAE2CFAEF568518C27ED7CE5D",
    ADJUDICATOR: "7FB9F3575E3965108B3A35E05C6799D5CC24555A250611ED3FF0E2A1CACF5CEA",
    CMAKE: "DFB9E52B9BA43B10344C806BCD8B2B0936F71BF5F1A1632428939EF58F1D544D",
    EXECUTABLE: "D2DEFE4F4D540EDC044CFD8C7E0802CD40CE60BD153D4473347E99C65042AD60",
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
check("runner embeds protocol hash", LOCKS[PROTOCOL] in runner)
check("runner freezes held-out dimensions", all(
    marker in runner for marker in (
        "kTicks = 128", "kVolumes{{19, 23}}", "0x09110001u", "0x09110008u"
    )
))
check("runner freezes qualification and cell gates", all(
    marker in runner for marker in (
        "kMinimumPairRun = 8", "kMinimumCommonSupport = 32",
        "kCellSeedGate = 6", "kCentralQualifiedSeedGate = 12",
    )
))
check("runner implements all cyclic derangements", "for (int shift = 1; shift < result.retained_pairs; ++shift)" in runner)
check("runner implements strict actual-greater ordering", "result.actual_same > result.maximum_null_same" in runner)
check("runner implements midpoint wedge identity", "delta_wedge - torque_p - torque_q" in runner)
check("runner audits state and RNG nonmutation", all(
    marker in runner for marker in (
        "voxel_hash_before == observation.voxel_hash_after",
        "rng_hash_before == observation.rng_hash_after",
    )
))
check("runner keeps perturbation/work/tick firewalls", all(
    marker in runner for marker in (
        "PERTURBATION_APPLIED=FALSE", "MAINTENANCE_ERASURE_WORK_CLOSED=FALSE",
        "PRODUCTION_TICK_MODIFIED=FALSE",
    )
))
check("CMake registers exact held-out target", all(
    marker in cmake for marker in (
        "ftd_add_test(campaign_held_out_pair_specific_phase_wedge_centrality",
        "tests/campaign_held_out_pair_specific_phase_wedge_centrality.cpp",
        "CTEST_NAME held_out_pair_specific_phase_wedge_centrality",
    )
))
check("adjudicator embeds runner lock", LOCKS[RUNNER] in adjudicator)
check("adjudicator reconstructs derangements and midpoint ledger", all(
    marker in adjudicator for marker in (
        "reported_derangements", "longest_common_interval", "torque_p",
        "OUTCOME_D_NOT_PAIR_SPECIFIC_NOT_EXACT_CENTRAL",
    )
))

passed = sum(ok for _, ok in checks)
for name, ok in checks:
    print(f"{'PASS' if ok else 'FAIL'}  {name}")
print(f"\nFTD-0912 runner lock preflight: {passed}/{len(checks)} checks passed")
print("CAMPAIGN_EXECUTED=FALSE")
print("RUNNER_AND_ADJUDICATOR_FROZEN=TRUE")
print("PAIR_CENTRALITY_VERDICT=NOT_YET_AVAILABLE")
raise SystemExit(0 if passed == len(checks) else 1)
