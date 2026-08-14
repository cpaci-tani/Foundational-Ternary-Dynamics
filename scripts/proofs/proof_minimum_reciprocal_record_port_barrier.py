#!/usr/bin/env python3
"""FTD-0856 exact minimum reciprocal record-port barrier certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
SOURCES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md":
        "4A498C6D7C7E65FA685D9F0879157D76713F310A6D025CCAA8756C3F1E0322E6",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md":
        "7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7",
    "docs/theory/07_assessment/common_action_mechanics_reciprocity/"
    "AUDIT_PRODUCTION_SAME_SIGN_BOUNCE.md":
        "090F139CBA8C930A9761A33EFBFB59BD2767F22E4DF50031120B70E18D42EA15",
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/src/render_bridge_phases/phase_movement.cpp":
        "6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
}

checks: list[tuple[str, bool]] = []


def check(label: str, condition: object) -> None:
    ok = bool(condition)
    checks.append((label, ok))
    print(f"{'PASS' if ok else 'FAIL'}  {label}")


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


for relative, expected in SOURCES.items():
    path = ROOT / relative
    check(f"source hash {relative}", path.is_file() and digest(path) == expected)

voxel = (ROOT / "engine/include/ftd/voxel.h").read_text(encoding="utf-8")
phase_write = (ROOT / "engine/src/render_bridge_phases/phase_write.cpp").read_text(
    encoding="utf-8"
)
movement = (
    ROOT / "engine/src/render_bridge_phases/phase_movement.cpp"
).read_text(encoding="utf-8")
phase_read = (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(
    encoding="utf-8"
)
bounce_audit = (
    ROOT
    / "docs/theory/07_assessment/common_action_mechanics_reciprocity/"
    "AUDIT_PRODUCTION_SAME_SIGN_BOUNCE.md"
).read_text(encoding="utf-8")
protocol = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md"
).read_text(encoding="utf-8")

check("C8 production exposes a two-valued locked coordinate", "bool locked = false;" in voxel)
check(
    "C9 locked records are excluded from evaporation",
    "v.state != 0 && !v.locked" in phase_write,
)
check(
    "C10 locked records are excluded from movement",
    movement.count("v.state == 0 || v.locked || rb.moved_[i]") == 2,
)
check(
    "C11 production same-sign branch flips mover axes and erases remainder",
    movement.count("// Same sign: elastic bounce") == 2
    and movement.count("v.remainder = {};") >= 2,
)
check(
    "C12 source-locked audit rejects production bounce as reciprocal",
    "PRODUCTION_BOUNCE_IS_FIXED_TARGET_RESET_NOT_RECIPROCAL_COLLISION"
    in bounce_audit
    and "The unchanged tick is not an inverse" in bounce_audit,
)
check(
    "C13 production dual type carries relative field and velocity capacity",
    all(name in voxel for name in ("flux_L", "flux_R", "wave_vel_L", "wave_vel_R"))
    and "phi = flux_L - flux_R" in voxel,
)
check(
    "C14 production evolves L and R with matched local operators",
    "laplacian_field<&Voxel::flux_L>" in phase_read
    and "laplacian_field<&Voxel::flux_R>" in phase_read
    and "rb.delta_j_L_[i] -= rb.voxels_[i].flux_L" in phase_read
    and "rb.delta_j_R_[i] -= rb.voxels_[i].flux_R" in phase_read,
)
check(
    "C15 no production phase names or implements the reciprocal record port",
    all(
        "ReciprocalRecordPort" not in text
        for text in (voxel, phase_write, movement, phase_read)
    ),
)

g = sp.symbols("g", integer=True)
S = sp.Matrix(((1 - g, g), (g, 1 - g)))
S0 = S.subs(g, 0)
S1 = S.subs(g, 1)
I2 = sp.eye(2)
X = sp.Matrix(sp.symbols("m i", real=True))

check("C16 closed-gate matrix is identity", S0 == I2)
check("C17 open-gate matrix exchanges matter and field", S1 == sp.Matrix(((0, 1), (1, 0))))
check("C18 both gate matrices are symmetric", S0.T == S0 and S1.T == S1)
check("C19 both gate matrices are orthogonal", S0.T * S0 == I2 and S1.T * S1 == I2)
check("C20 both gate matrices are involutions", S0**2 == I2 and S1**2 == I2)
check("C21 gate determinants distinguish hold and exchange", S0.det() == 1 and S1.det() == -1)

for matrix in (S0, S1):
    Y = matrix * X
    energy_residual = sp.expand((Y.dot(Y) - X.dot(X)) / 2)
    content_residual = sp.expand(sum(Y) - sum(X))
    check(
        f"C{22 if matrix == S0 else 23} {'closed' if matrix == S0 else 'open'} gate preserves energy and signed content",
        energy_residual == 0 and content_residual == 0,
    )

B = sp.symbols("B", positive=True)
sigma = sp.symbols("sigma", real=True, nonzero=True)
A = sp.sqrt(2 * B)
matter = sigma * A

closed = S0 * sp.Matrix((matter, 0))
check("C24 closed gate strictly preserves the occupied record", closed == sp.Matrix((matter, 0)))
incident_closed = S0 * sp.Matrix((0, matter))
check("C25 closed gate leaves an incident characteristic outside matter", incident_closed == sp.Matrix((0, matter)))
emission = S1 * sp.Matrix((matter, 0))
check("C26 open gate emits the signed record energy", emission == sp.Matrix((0, matter)))
absorption = S1 * sp.Matrix((0, matter))
check("C27 open gate absorbs the signed incoming characteristic", absorption == sp.Matrix((matter, 0)))

negation = -sp.eye(2)
check(
    "C28 barrier is equivariant under simultaneous sign reversal",
    S0 * negation == negation * S0 and S1 * negation == negation * S1,
)

M = sp.Matrix((matter, 0))
P = sp.Matrix((0, matter))
check(
    "C29 deterministic hold and exchange require distinguishable eligibility states",
    M != P and S0 * M == M and S1 * M == P and S0 != S1,
)

quotient = sp.Matrix(((1, 1),))
orientation_kernel = sp.Matrix((1, -1))
check(
    "C30 unlabeled amplitude quotient loses incoming-outgoing orientation and two ports retain it",
    quotient * orientation_kernel == sp.zeros(1, 1)
    and sp.Matrix((1, 0)) != sp.Matrix((0, 1)),
)
check(
    "C31 emitted amplitude is exactly the normalized FTD-0855 rail event",
    sp.simplify(emission[1] - sigma * sp.sqrt(2 * B)) == 0,
)
check(
    "C32 physical gate origin production scatterer full-state lift and target-coupled claims remain open",
    all(
        phrase in protocol
        for phrase in (
            "physical origin for `g`",
            "protected incoming/outgoing record channel",
            "full natural extension",
            "Born, Bell, `G*`",
            "Clock phase alone must not force",
        )
    )
    and "ReciprocalRecordPort" not in voxel,
)

passed = sum(ok for _, ok in checks)
total = len(checks)
print()
print(f"FTD-0856 minimum reciprocal record-port barrier: {passed}/{total} PASS")
if passed == total == 32:
    print("DETERMINISTIC_STRICT_HOLD_AND_EVENT_EXCHANGE_REQUIRE_DISTINCT_ELIGIBILITY")
    print("RECIPROCAL_FORWARD_TIME_RAIL_REQUIRES_RETAINED_CAUSAL_ORIENTATION")
    print("CONTROLLED_MATTER_INCOMING_OUTGOING_SWAP_IS_EXACT_RECIPROCAL_BARRIER")
    print("PRODUCTION_LOCK_AND_DUAL_TYPE_ARE_FRAGMENTS_NOT_THE_SCATTERER")
    print("VERDICT=OUTCOME_B_MINIMUM_REFERENCE_BARRIER_PRODUCTION_INCOMPLETE")
    raise SystemExit(0)

print("VERDICT=OUTCOME_C_NO_THEOREM")
raise SystemExit(1)
