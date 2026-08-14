#!/usr/bin/env python3
"""Fail-closed FTD-0915/0917 raw-telemetry runner preflight."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme"
PROTOCOL = BASE / "PREREG_PRODUCTION_TERNARY_PLAQUETTE_QUARTER_TURN_RECURRENCE_CENSUS_v1.md"
REPAIR = BASE / "PREREG_PRODUCTION_TERNARY_PLAQUETTE_RECURRENCE_RAW_TELEMETRY_REPAIR_v3.md"
RUNNER = ROOT / "engine/tests/campaign_production_ternary_plaquette_recurrence_census.cpp"

LOCKS = {
    PROTOCOL: "C302319900BAC4920277FACCC3A9164F0AE64DCAC8FBD256A4F36B48E7CC970C",
    REPAIR: "26D4488E2BB8EB6783C1C7F6B4D413D79D487D78A9A43A98D793F2B02D55DF44",
    ROOT / "engine/include/ftd/voxel.h": "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    ROOT / "engine/include/ftd/render_bridge.h": "560CB59E2FCD6E174640CA6BF048FD16AEC36AD2B13EE97FA31E301CF373D91C",
    ROOT / "engine/src/render_bridge.cpp": "BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724",
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp": "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    ROOT / "engine/include/ftd/constants.h": "5C9E4EA46DE1D5E0BF4479AA9E115520E70B729E7E81335FCEF08CE99704BAB0",
    ROOT / "engine/include/ftd/eft/native_ternary_plaquette_quarter_turn.h": "3A970B82EF0BDCCC457D5DDA049CAF971C2318429970E696E64DB84CEB7D1D09",
    ROOT / "engine/src/eft/native_ternary_plaquette_quarter_turn.cpp": "E7891C5099D2DCA1F20DF72E6B37F29A60FE63A7A9E7E645D8AC6E2DF73E1F4C",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_TERNARY_PLAQUETTE_QUARTER_TURN_RECURSION_v1.md": "DC98BB8E8A0CF39E832F7399631F831FF71D3216ED104B6C76384EEEF9100B26",
    RUNNER: "D24970F34346167197D53681F1E6231A68C5E81F0515E6CA85B7335FBED83F21",
    ROOT / "engine/CMakeLists.txt": "C895673132434DE830A15EE41676A446FCEF6D26D7C3819ED491E536D37BB745",
    ROOT / "engine/build/Release/campaign_production_ternary_plaquette_recurrence_census.exe": "E02B56E25F8FD38C0E12815A30D342378E7E9CC072DD0A7011CB71A80548249D",
    ROOT / "engine/results/ftd_0915/ftd_0915_summary_v1.json": "53CC7D0C78BB5EB050B1D0F45F1CAD0F6118C48C1092CA6CAACFC3A6915D204E",
    ROOT / "engine/results/ftd_0915/ftd_0915_tick_census_v1.csv": "F006ADACDABFEF970F4DE4914ADDBE3DCE2B812E49993596CAB23ED1AA80AA47",
    ROOT / "engine/results/ftd_0915/ftd_0915_transition_census_v1.csv": "27291B4A36F82ED3C0168DBD514ED63510A99AD849343A1411682326FE60B49C",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


checks: list[tuple[str, bool]] = []
for path, expected in LOCKS.items():
    checks.append((f"lock {path.relative_to(ROOT)}", path.is_file() and sha256(path) == expected))

protocol_text = PROTOCOL.read_text(encoding="utf-8")
repair_text = REPAIR.read_text(encoding="utf-8")
runner_text = RUNNER.read_text(encoding="utf-8")

checks.extend([
    ("parent protocol remains locked", "STATUS=LOCKED_PRE_RUN" in protocol_text),
    ("repair is telemetry only", "TELEMETRY_ONLY_REPAIR=TRUE" in repair_text),
    ("parent outcome map unchanged", "PARENT_OUTCOME_MAP_CHANGED=FALSE" in repair_text),
    ("parent arms unchanged", "PARENT_ARMS_CHANGED=FALSE" in repair_text),
    ("parent thresholds unchanged", "PARENT_THRESHOLDS_CHANGED=FALSE" in repair_text),
    ("physics unchanged", "PHYSICS_CHANGED=FALSE" in repair_text),
    ("v3 locked before execution", "RUNNER_V3_LOCKED_BEFORE_EXECUTION=TRUE" in repair_text),
    ("runner embeds parent protocol hash", LOCKS[PROTOCOL] in runner_text),
    ("runner fixes 128 ticks", "constexpr int kTicks = 128;" in runner_text),
    ("runner fixes six-of-eight gate", "constexpr int kCellSeedGate = 6;" in runner_text),
    ("runner fixes volumes", "constexpr std::array<int, 2> kVolumes{{21, 27}};" in runner_text),
    ("runner fixes all eight seeds", all(f"0x0915000{i}u" in runner_text for i in range(1, 9))),
    ("runner stores raw vertex type", "struct VertexSample" in runner_text),
    ("runner stores state", "int state = 0;" in runner_text),
    ("runner stores particle identity", "int particle_id = -1;" in runner_text),
    ("runner stores flux", "std::array<double, 3> flux" in runner_text),
    ("runner stores wave velocity", "std::array<double, 3> wave_velocity" in runner_text),
    ("runner writes exposure corpus", "ftd_0915_exposure_census_v3.csv" in runner_text),
    ("runner writes before raw sites", "write_vertices(transitions, before.vertices);" in runner_text),
    ("runner writes after raw sites", "write_vertices(transitions, after_vertices);" in runner_text),
    ("runner writes isolated v3 directory", '"ftd_0915" / "v3"' in runner_text),
    ("runner forces CPU", "bridge.force_cpu();" in runner_text),
    ("observer receives const bridge", "const ftd::RenderBridge& bridge, int volume, Family family" in runner_text),
    ("observer verifies nonmutation", "observation.voxel_hash_before == observation.voxel_hash_after" in runner_text),
    ("runner checks exact enumeration", "3LL * volume * volume * volume" in runner_text),
    ("runner checks direct closure", "closure = after_word == track.start_word;" in runner_text),
    ("runner emits production firewall", "PRODUCTION_TICK_MODIFIED=FALSE" in runner_text),
    ("runner emits G-star firewall", "GSTAR_READ=FALSE" in runner_text),
    ("runner emits Born/Bell firewall", "BORN_BELL_TARGET_READ=FALSE" in runner_text),
    ("runner contains no search hook", "parameter_sweep" not in runner_text and "near_miss" not in runner_text),
])

passed = sum(ok for _, ok in checks)
for name, ok in checks:
    print(f"{'PASS' if ok else 'FAIL'}  {name}")
print(f"\nFTD-0915/0917 v3 preflight: {passed}/{len(checks)} checks passed")
print("RUNNER_V3_LOCKED_BEFORE_EXECUTION=TRUE")
print("PHYSICS_CHANGED=FALSE")
print("PRODUCTION_SOURCE_DRIFT=FALSE" if passed == len(checks) else "PROTOCOL_INVALID=TRUE")
raise SystemExit(0 if passed == len(checks) else 1)
