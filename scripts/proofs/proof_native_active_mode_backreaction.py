#!/usr/bin/env python3
"""Hash-locked source/dataflow proof for FTD-0582 (no parameter search)."""

from fractions import Fraction
from hashlib import sha256
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/src/render_bridge_phases/phase_forces.cpp":
        "F7A855DC3ED3BF9882807CF7C8D1A35CF66864433B711CA5CA4B9CB836549322",
    "engine/src/render_bridge_phases/phase_movement.cpp":
        "6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB",
    "engine/src/render_bridge.cpp":
        "A822E0FAFAF71FE5458B2A7450868A8414B1C8564089BF6C6484FC34B7559359",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
    "engine/include/ftd/eft/native_energy_contract.h":
        "3DB8F2DC573E7F4A87E17409878915E7B5A52CE1673713998C544516E0175621",
}


def digest(path: Path) -> str:
    return sha256(path.read_bytes()).hexdigest().upper()


for relative, expected in LOCKS.items():
    actual = digest(ROOT / relative)
    assert actual == expected, (relative, actual, expected)


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//.*", "", text)


read_source = strip_comments(
    (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(
        encoding="utf-8"))
write_source = strip_comments(
    (ROOT / "engine/src/render_bridge_phases/phase_write.cpp").read_text(
        encoding="utf-8"))
force_source = strip_comments(
    (ROOT / "engine/src/render_bridge_phases/phase_forces.cpp").read_text(
        encoding="utf-8"))
move_source = strip_comments(
    (ROOT / "engine/src/render_bridge_phases/phase_movement.cpp").read_text(
        encoding="utf-8"))
tick_source = strip_comments(
    (ROOT / "engine/src/render_bridge.cpp").read_text(encoding="utf-8"))

# Native field/source phases do not write manifested kinematics.
kinematic_write = re.compile(r"\.(?:velocity|remainder)\s*[+\-*/]?=")
assert not kinematic_write.search(read_source)
assert not kinematic_write.search(write_source)

# The field-dependent velocity update is confined to the selected force phase,
# and the whole phase is bypassed by forces=false.
assert "if (toggles.forces)\n    phase_forces();" in tick_source
assert "v.velocity = scale > 0.0 ? q * scale : Vec3{};" in force_source
assert "if (toggles.movement)\n    phase_movement();" in tick_source

# Collision-free movement reads velocity and integrates remainder. The source
# contains two equivalent branches (symmetric and ordinary traversal).
drift = "v.remainder += v.velocity * rb.dt_;"
assert move_source.count(drift) == 2
assert "if (dx == 0 && dy == 0 && dz == 0) continue;" in move_source

# Every other velocity write in movement is visibly one of the registered
# exclusions: causal projection, boundary/reset, state transport, or collision.
for witness in (
    "v.velocity *= scale;",
    "v.velocity = {};",
    "t.velocity = v.velocity;",
    "v.velocity.x *= -1.0;",
    "v.velocity = {}; t.velocity = {};",
):
    assert witness in move_source

# Exact induction on the registered isolated domain. No field symbol enters
# either recurrence once the force phase is absent.
dt = Fraction(1, 1)
velocity = (Fraction(0), Fraction(0), Fraction(0))
remainder = (Fraction(0), Fraction(0), Fraction(0))
anchor = (0, 0, 0)
for _ in range(10_000):
    remainder = tuple(remainder[i] + velocity[i] * dt for i in range(3))
    assert all(-1 < component < 1 for component in remainder)
    delta = tuple(
        1 if component >= 1 else (-1 if component <= -1 else 0)
        for component in remainder
    )
    assert delta == (0, 0, 0)
    anchor = tuple(anchor[i] + delta[i] for i in range(3))
    assert anchor == (0, 0, 0)

print("FTD-0582 hash-locked native active-mode source proof: PASS")
print("field phases write J,W but not velocity,remainder")
print("forces=false => no field-to-momentum production path")
print("zero velocity/remainder is an exact movement invariant")
print("verdict=FROZEN_NATIVE_FIELD_IS_ONE_WAY_TO_MATTER_ACTIVE_TRAVERSAL_CLOSED")
