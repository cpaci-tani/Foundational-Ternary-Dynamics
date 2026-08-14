#!/usr/bin/env python3
"""Fail-closed FTD-0915/0916 source and runner preflight."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_PRODUCTION_TERNARY_PLAQUETTE_QUARTER_TURN_RECURRENCE_CENSUS_v1.md"
)
RUNNER_LOCK = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_PRODUCTION_TERNARY_PLAQUETTE_RECURRENCE_RUNNER_LOCK_v2.md"
)
RUNNER = ROOT / (
    "engine/tests/"
    "campaign_production_ternary_plaquette_recurrence_census.cpp"
)

LOCKS = {
    PROTOCOL: "C302319900BAC4920277FACCC3A9164F0AE64DCAC8FBD256A4F36B48E7CC970C",
    RUNNER_LOCK: "809AE1059F20B2AC2FD18E642ECD8D028EF2DC4B1F3265785815445AADE0B26D",
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
    ROOT / "engine/include/ftd/eft/native_ternary_plaquette_quarter_turn.h":
        "3A970B82EF0BDCCC457D5DDA049CAF971C2318429970E696E64DB84CEB7D1D09",
    ROOT / "engine/src/eft/native_ternary_plaquette_quarter_turn.cpp":
        "E7891C5099D2DCA1F20DF72E6B37F29A60FE63A7A9E7E645D8AC6E2DF73E1F4C",
    ROOT / (
        "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_NATIVE_TERNARY_PLAQUETTE_QUARTER_TURN_RECURSION_v1.md"
    ): "DC98BB8E8A0CF39E832F7399631F831FF71D3216ED104B6C76384EEEF9100B26",
    RUNNER: "20E00A0BB988A72FEED7851A854846F3D1F18440CCA91AF9DFFC105A840F301D",
    ROOT / "engine/CMakeLists.txt":
        "C895673132434DE830A15EE41676A446FCEF6D26D7C3819ED491E536D37BB745",
    ROOT / "engine/build/Release/campaign_production_ternary_plaquette_recurrence_census.exe":
        "8CDCCE805C5721B1266D16B3A0B01D8857A85D42ED93EC3AFBCE8A7849147B64",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


checks: list[tuple[str, bool]] = []
for path, expected in LOCKS.items():
    checks.append((f"source lock {path.relative_to(ROOT)}", path.is_file() and sha256(path) == expected))

protocol_text = PROTOCOL.read_text(encoding="utf-8")
runner_lock_text = RUNNER_LOCK.read_text(encoding="utf-8")
runner_text = RUNNER.read_text(encoding="utf-8")

checks.extend([
    ("protocol is locked pre-run", "STATUS=LOCKED_PRE_RUN" in protocol_text),
    ("protocol fixes all elementary plaquettes", "SUPPORT=ALL_ELEMENTARY_CARDINAL_PLAQUETTES" in protocol_text),
    ("protocol fixes identity key", "IDENTITY_KEY=FIXED_SUPPORT_PLUS_POSITIVE_AND_NEGATIVE_PARTICLE_IDS" in protocol_text),
    ("protocol fixes four-transition cycle", "FULL_CYCLE=FOUR_CONSECUTIVE_SAME_DIRECTION_QUARTER_TURNS" in protocol_text),
    ("protocol fixes volumes", "VOLUMES=21,27" in protocol_text),
    ("protocol fixes 128 ticks", "TICKS_PER_ARM=128" in protocol_text),
    ("protocol fixes eight seeds", "SEED_COUNT=8" in protocol_text),
    ("protocol fixes 64 arms", "ARM_COUNT=64" in protocol_text),
    ("protocol fixes six-of-eight gate", "CELL_GATE=6_OF_8" in protocol_text),
    ("runner lock precedes execution", "RUNNER_LOCKED_BEFORE_EXECUTION=TRUE" in runner_lock_text),
    ("runner embeds protocol hash", LOCKS[PROTOCOL] in runner_text),
    ("runner fixes 128 ticks", "constexpr int kTicks = 128;" in runner_text),
    ("runner fixes six-of-eight gate", "constexpr int kCellSeedGate = 6;" in runner_text),
    ("runner fixes held-out volumes", "constexpr std::array<int, 2> kVolumes{{21, 27}};" in runner_text),
    ("runner fixes first seed", "0x09150001u" in runner_text),
    ("runner fixes last seed", "0x09150008u" in runner_text),
    ("runner forces CPU", "bridge.force_cpu();" in runner_text),
    ("observer receives const bridge", "const ftd::RenderBridge& bridge, int volume, Family family" in runner_text),
    ("observer hashes voxels", "observation.voxel_hash_before = hash_voxels(bridge.voxels());" in runner_text),
    ("observer hashes RNG", "observation.rng_hash_before = bridge.rng_state_hash();" in runner_text),
    ("runner checks exact enumeration", "3LL * volume * volume * volume" in runner_text),
    ("runner checks direct closure", "closure = after_word == track.start_word;" in runner_text),
    ("runner emits production firewall", "PRODUCTION_TICK_MODIFIED=FALSE" in runner_text),
    ("runner emits G-star firewall", "GSTAR_READ=FALSE" in runner_text),
    ("runner emits Born/Bell firewall", "BORN_BELL_TARGET_READ=FALSE" in runner_text),
    ("runner has no parameter sweep", "parameter_sweep" not in runner_text and "near_miss" not in runner_text),
])

passed = sum(ok for _, ok in checks)
for name, ok in checks:
    print(f"{'PASS' if ok else 'FAIL'}  {name}")

print(f"\nFTD-0915/0916 preflight: {passed}/{len(checks)} checks passed")
print("RUNNER_LOCKED_BEFORE_EXECUTION=TRUE")
print("PRODUCTION_SOURCE_DRIFT=FALSE" if passed == len(checks) else "PROTOCOL_INVALID=TRUE")
raise SystemExit(0 if passed == len(checks) else 1)
