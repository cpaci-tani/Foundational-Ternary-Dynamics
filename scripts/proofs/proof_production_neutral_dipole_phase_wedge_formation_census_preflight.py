#!/usr/bin/env python3
"""FTD-0908 locked-source and protocol preflight; does not run the campaign."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_PRODUCTION_NEUTRAL_DIPOLE_PHASE_WEDGE_FORMATION_CENSUS_v1.md"
)

LOCKS = {
    PROTOCOL: "53348A90021C609E3EBA5DC7D565F6EA78832498C206D0D4B3F1964CCC7C4993",
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
voxel = (ROOT / "engine/include/ftd/voxel.h").read_text(encoding="utf-8")
phase_write = (ROOT / "engine/src/render_bridge_phases/phase_write.cpp").read_text(
    encoding="utf-8"
)

check("Voxel exposes ternary state", "int8_t state = 0;" in voxel)
check("Voxel exposes flux", "Vec3 flux;" in voxel)
check("Voxel exposes wave velocity", "Vec3 wave_vel;" in voxel)
check("Voxel exposes persistent particle ID", "int32_t particle_id = -1;" in voxel)
check("production assigns deterministic pending IDs", "phase_write_assign_pending_ids" in phase_write)
check("protocol freezes Moore pair support", "PAIR_SUPPORT=MOORE_NEIGHBOUR_PLUS_MINUS" in protocol)
check("protocol freezes production ID tracking", "PAIR_TRACKING=PRODUCTION_PARTICLE_IDS" in protocol)
check("protocol freezes wedge tolerance", "PHASE_WEDGE_TOLERANCE=1E-11" in protocol)
check("protocol freezes persistence threshold", "PERSISTENCE_THRESHOLD_TICKS=8" in protocol)
check("protocol freezes volumes", "VOLUMES=17,25" in protocol)
check("protocol freezes tick count", "TICKS_PER_ARM=96" in protocol)
check("protocol freezes arm count", "ARM_COUNT=32" in protocol)
check("protocol freezes three live and one empty family", all(
    marker in protocol for marker in (
        "`axial_live`", "`diagonal_live`", "`axial_no_bath`", "`empty_control`"
    )
))
check("protocol freezes observer-only transformation controls", all(
    marker in protocol for marker in (
        "signed-cubic transform", "canonical time reversal", "Gram determinant"
    )
))
check("protocol freezes A/B/C and invalid outcomes", all(
    marker in protocol for marker in (
        "Outcome A", "Outcome B", "Outcome C", "PROTOCOL_INVALID_NO_FORMATION_VERDICT"
    )
))
check("protocol forbids production tick modification", "PRODUCTION_TICK_MODIFIED=FALSE" in protocol)
check("protocol forbids Gstar read", "GSTAR_READ=FALSE" in protocol)
check("protocol forbids context outcome Born read", "CONTEXT_OUTCOME_BORN_READ=FALSE" in protocol)
check("protocol does not claim central-law test", "CENTRAL_MEMORY_LAW_TESTED=FALSE" in protocol)
check("protocol keeps maintenance work open", "MAINTENANCE_ERASURE_WORK_CLOSED=FALSE" in protocol)
check("protocol adds no selected type", "NO_NEW_SELECTED_TYPE=TRUE" in protocol)
check("protocol remains pre-run", "STATUS=LOCKED_PRE_RUN" in protocol)

passed = sum(ok for _, ok in checks)
for name, ok in checks:
    print(f"{'PASS' if ok else 'FAIL'}  {name}")
print(f"\nFTD-0908 preflight: {passed}/{len(checks)} checks passed")
print("CAMPAIGN_EXECUTED=FALSE")
print("PRODUCTION_FORMATION_VERDICT=NOT_YET_AVAILABLE")
raise SystemExit(0 if passed == len(checks) else 1)
