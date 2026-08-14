#!/usr/bin/env python3
"""FTD-0911 locked-source/protocol preflight; does not run the campaign."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_HELD_OUT_PAIR_SPECIFIC_PHASE_WEDGE_AND_CENTRALITY_v1.md"
)
LOCKS = {
    PROTOCOL: "D0C7976FE334EA5D814D40DADEDBEF9CB8419B0A518AFE0492C2F3A183FF88FE",
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
    check(f"source lock {path.relative_to(ROOT)}", path.is_file() and sha256(path) == expected)

protocol = PROTOCOL.read_text(encoding="utf-8")
check("held-out volumes are 19 and 23", "VOLUMES=19,23" in protocol)
check("held-out tick count is 128", "TICKS_PER_ARM=128" in protocol)
check("eight seeds are frozen", "SEEDS=0X09110001..0X09110008" in protocol)
check("64-arm matrix is frozen", "ARM_COUNT=64" in protocol)
check("four inherited families are frozen", all(
    marker in protocol for marker in (
        "`axial_live`", "`diagonal_live`", "`axial_no_bath`", "`empty_control`"
    )
))
check("pair qualification freezes eight ticks", "PAIR_MINIMUM_RUN=8" in protocol)
check("common support freezes 32 ticks", "COMMON_SUPPORT_MINIMUM=32" in protocol)
check("cell gate freezes six of eight", "PAIR_CELL_GATE=6/8" in protocol)
check("all cyclic derangements are frozen", "DERANGEMENTS=ALL_NONZERO_FIXED_CYCLIC_SHIFTS" in protocol)
check("pair discriminator is exact integer ordering", "PAIR_DISCRIMINATOR=EXACT_SAME_SIGN_COUNT_ORDER" in protocol)
check("midpoint wedge ledger is parameter-free", "CENTRALITY=PARAMETER_FREE_MIDPOINT_WEDGE_LEDGER" in protocol)
check("no-bath central qualification is frozen", "CENTRAL_NO_BATH_QUALIFICATION=12/16" in protocol)
check("five outcomes plus invalid are frozen", all(
    marker in protocol for marker in (
        "Outcome A (`P+C+`)", "Outcome B (`P+C-`)",
        "Outcome C (`P-C+`)", "Outcome D (`P-C-`)",
        "Outcome U", "PROTOCOL_INVALID_NO_PAIR_OR_CENTRALITY_VERDICT",
    )
))
check("production tick remains unchanged", "PRODUCTION_TICK_MODIFIED=FALSE" in protocol)
check("Gstar read remains forbidden", "GSTAR_READ=FALSE" in protocol)
check("context outcome Born read remains forbidden", "CONTEXT_OUTCOME_BORN_READ=FALSE" in protocol)
check("no perturbation is applied", "PERTURBATION_APPLIED=FALSE" in protocol)
check("work and erasure remain open", "MAINTENANCE_ERASURE_WORK_CLOSED=FALSE" in protocol)
check("no selected type is added", "NO_NEW_SELECTED_TYPE=TRUE" in protocol)
check("protocol remains pre-run", "STATUS=LOCKED_PRE_RUN" in protocol)

passed = sum(ok for _, ok in checks)
for name, ok in checks:
    print(f"{'PASS' if ok else 'FAIL'}  {name}")
print(f"\nFTD-0911 preflight: {passed}/{len(checks)} checks passed")
print("CAMPAIGN_EXECUTED=FALSE")
print("PAIR_SPECIFICITY_VERDICT=NOT_YET_AVAILABLE")
print("CENTRALITY_VERDICT=NOT_YET_AVAILABLE")
raise SystemExit(0 if passed == len(checks) else 1)
