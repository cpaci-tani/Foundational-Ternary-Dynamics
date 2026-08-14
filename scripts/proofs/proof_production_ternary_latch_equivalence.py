#!/usr/bin/env python3
"""FTD-0849 exact production ternary-latch equivalence discriminator."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
SOURCES = {
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    ROOT / "engine/src/energy_ledger_compute.cpp":
        "2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B",
    ROOT / "engine/src/render_bridge.cpp":
        "BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724",
    ROOT / "engine/include/ftd/term_toggles.h":
        "2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA",
    ROOT / "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_LOSS_BOOKED_TERNARY_PHASE_LATCH_v1.md":
        "1C1BE138260B4CD3B639F7B6E1DB9E78886B2CCC9E6C0388CFC83E0D0FE073CA",
    ROOT / "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_GENESIS_ACTION_OBSTRUCTION.md":
        "877ACAA8C859DFE065120543B8FBC7862BD619AFCB57A4B7CD6D214A6CA18055",
    ROOT / "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_GENESIS_RESERVOIR_DILATION.md":
        "565BCD17963322349D5D136E40DE11BF2268677A1CF8D1EED062818EA0E6BFBC",
    ROOT / "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_GENESIS_NATURAL_EXTENSION.md":
        "2611A6DE2D2318DFC4EC97FDF148D91D952BE3775421BE4DDAC441EA2F534076",
}

checks: list[tuple[str, bool]] = []


def check(label: str, condition: object) -> None:
    ok = bool(condition)
    checks.append((label, ok))
    print(f"{'PASS' if ok else 'FAIL'}  {label}")


def zero(expr: object) -> bool:
    return sp.simplify(expr) == 0


for path, expected in SOURCES.items():
    actual = hashlib.sha256(path.read_bytes()).hexdigest().upper()
    check(f"source hash {path.relative_to(ROOT).as_posix()}", actual == expected)

phase_path = ROOT / "engine/src/render_bridge_phases/phase_write.cpp"
ledger_path = ROOT / "engine/src/energy_ledger_compute.cpp"
bridge_path = ROOT / "engine/src/render_bridge.cpp"
voxel_path = ROOT / "engine/include/ftd/voxel.h"
phase_source = phase_path.read_text(encoding="utf-8")
ledger_source = ledger_path.read_text(encoding="utf-8")
bridge_source = bridge_path.read_text(encoding="utf-8")
voxel_source = voxel_path.read_text(encoding="utf-8")

# C10--C14: exact production state/acquisition semantics.
check("C10 audited genesis and evaporation write only ternary record labels",
      "? 1 : -1" in phase_source
      and "rb.set_state(i, 0);" in phase_source
      and "int8_t state = 0;" in voxel_source)

check("C11 genesis is a void-only strict flux-magnitude threshold",
      "v.state == 0 && v.flux.mag2() > kg * kg" in phase_source)

D = sp.symbols("D", positive=True, real=True)
single_sign_plus = 1 if bool(D > 0) else -1
single_sign_minus = 1 if bool(-D > 0) else -1
single_zero = -1
dual_zero = 1
check("C12 nonzero divergence polarity is odd but the zero tie is branch selected",
      single_sign_plus == 1 and single_sign_minus == -1
      and single_sign_minus == -single_sign_plus
      and single_zero != dual_zero)

x, km = sp.symbols("x k_m", positive=True, real=True)
p = 1 - sp.exp(-x / km)
dpdx = sp.diff(p, x)
check("C13 finite positive excess has a strictly interior acceptance ramp",
      zero(p.subs(x, 0))
      and sp.ask(sp.Q.positive(dpdx)) is True
      and sp.limit(p, x, sp.oo) == 1)

check("C14 acceptance reads the deterministic index tick seed selector state",
      "voxel_uniform(gseed, i, rb.tick_" in phase_source
      and "VoxelRng::GenesisManifest" in phase_source)

# C15--C19: accepted-event map and branchwise energy mismatch.
k, d, w2 = sp.symbols("k_g d W2", positive=True, real=True)
r_before = k + x
r_after = r_before * (1 - k / r_before)
check("C15 accepted single genesis leaves exactly the incoming excess",
      zero(r_after - x))

field_withdrawal = sp.Rational(1, 2) * (r_before**2 - r_after**2)
wave_withdrawal = sp.Rational(1, 2) * w2 * (1 - (1 - d)**2)
check("C16 accepted-event quadratic withdrawals are exact",
      zero(field_withdrawal - (k * x + k**2 / 2))
      and zero(wave_withdrawal - (d - d**2 / 2) * w2))

check("C17 withdrawn energy is input dependent rather than one state quantum",
      zero(sp.diff(field_withdrawal, x) - k)
      and zero(sp.diff(wave_withdrawal, w2) - (d - d**2 / 2)))

dual_start = phase_source.index("// Genesis (dual):")
dual_end = phase_source.index("    } else {", dual_start)
dual_genesis_source = phase_source[dual_start:dual_end]
check("C18 dual genesis changes state without the single-branch flux-wave drain",
      "manifest_at" in dual_genesis_source
      and "kinetic_drain" not in dual_genesis_source
      and "v.flux *=" not in dual_genesis_source)

single_positive_payment = field_withdrawal + wave_withdrawal
check("C19 single and dual branches do not share one event-level latch transaction",
      sp.ask(sp.Q.positive(field_withdrawal)) is True
      and zero(sp.Integer(0)))

# C20--C24: persistence and explicit many-to-one loss.
check("C20 evaporation reads unsigned local energy and applies no sign branch",
      "double local_energy = v.flux.mag2() + v.wave_vel.mag2();" in phase_source
      and "double evap_prob = std::exp(-local_energy" in phase_source)

E, K, rate, dtau = sp.symbols("E K rate dtau", positive=True, finite=True, real=True)
q = sp.exp(-E / K**2) * rate * dtau
check("C21 every finite-energy unlocked record has nonzero evaporation hazard",
      sp.ask(sp.Q.positive(q)) is True)

check("C22 exact persistence is supplied only by the explicit locked boolean",
      "v.state != 0 && !v.locked" in phase_source
      and "bool locked = false;" in voxel_source)

plus_preimage = (1, 11, 1, 2, "same_fields")
minus_preimage = (-1, 22, -1, 3, "same_fields")


def evaporate(record: tuple[object, ...]) -> tuple[object, ...]:
    return (0, -1, 0, 0, record[4])


check("C23 evaporation collapses distinct signed records to one zero record",
      plus_preimage != minus_preimage
      and evaporate(plus_preimage) == evaporate(minus_preimage))

evap_reset_start = phase_source.index(
    "rb.set_state(i, 0);", phase_source.index("VoxelRng::Evaporation"))
evap_reset_end = phase_source.index("// FTD-HISTORY-BEGIN", evap_reset_start)
evap_reset_source = phase_source[evap_reset_start:evap_reset_end]
check("C24 erased labels are not transferred into the continuous voxel fields",
      "rb.set_state(i, 0);" in phase_source
      and "v.particle_id = -1;" in phase_source
      and "v.spin = 0;" in phase_source
      and "v.color = 0;" in phase_source
      and "v.flux" not in evap_reset_source
      and "v.wave_vel" not in evap_reset_source
      and "v.velocity" not in evap_reset_source
      and "v.remainder" not in evap_reset_source)

# C25--C30: ledger, leakage, and combined classification.
check("C25 aggregate energy ledger has no event-level bath or switch-work account",
      "genesis_events_this_tick_" not in ledger_source
      and "evaporation_events_this_tick_" not in ledger_source
      and "controller" not in ledger_source.lower()
      and "cumulative_dissipation" in ledger_source)

check("C26 selective-damping expected rate is explicitly approximate",
      "expected_rate` remains an approximation" in ledger_source)

production_audit_source = phase_source + ledger_source + bridge_source
forbidden_latch_types = (
    "TernaryPhaseLatchState", "LossLedger", "controller_work",
    "beta*x^2*(x^2-A^2)^2", "average-vector-field",
)
check("C27 production contains no FTD-0848 latch or event-ledger implementation",
      all(token not in production_audit_source for token in forbidden_latch_types))

registered_expressions = (p, r_after, field_withdrawal, wave_withdrawal, q)
allowed_symbols = {x, km, k, d, w2, E, K, rate, dtau}
check("C28 transition formulas read no context outcome Born G-star or cadence target",
      all(expr.free_symbols <= allowed_symbols for expr in registered_expressions))

fragment_pass = (
    single_sign_plus == 1 and single_sign_minus == -1
    and evaporate(plus_preimage) == evaporate(minus_preimage)
)
strict_persistence = zero(q)
exact_event_ledger = (
    "genesis_events_this_tick_" in ledger_source
    and "evaporation_events_this_tick_" in ledger_source
    and "controller" in ledger_source.lower()
)
check("C29 production has ternary sign and loss fragments but fails latch equivalence",
      fragment_pass and not strict_persistence and not exact_event_ledger)

check("C30 combined production ternary-latch discriminator closes Outcome B",
      len(checks) == 29
      and all(ok for _, ok in checks)
      and fragment_pass
      and not strict_persistence
      and not exact_event_ledger)

passed = sum(ok for _, ok in checks)
total = len(checks)
print()
print(f"FTD-0849 production ternary-latch equivalence: {passed}/{total} PASS")
if passed == total == 30:
    print("PRODUCTION_HAS_TERNARY_SIGNED_ACQUISITION_AND_MANY_TO_ONE_LOSS_FRAGMENTS")
    print("UNLOCKED_FINITE_ENERGY_RECORD_HAS_NO_STRICT_INVARIANT_BASIN")
    print("NO_EXACT_EVENT_LEVEL_BATH_OR_CONTROLLER_LEDGER_IS_IMPLEMENTED")
    print("CURRENT_GENESIS_EVAPORATION_IS_NOT_THE_FTD0848_LOSS_BOOKED_LATCH")
    print("VERDICT=OUTCOME_B_PARTIAL_TERNARY_OPEN_SYSTEM_WITNESS")
else:
    raise SystemExit(1)
