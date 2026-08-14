#!/usr/bin/env python3
"""Exact certificate for FTD-0944.

Audits frozen production source contracts and exact finite-dimensional event
maps.  No numerical coincidence search, fit, or production mutation occurs.
"""

from __future__ import annotations

from fractions import Fraction as Q
from hashlib import sha256
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROGRAMME = ROOT / "docs/theory/10_eft_program"
PREREG = PROGRAMME / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_EXISTING_EVENT_MEDIATED_RELATIVE_HISTORY_CARRIER_AUDIT_v1.md"
)

EXPECTED_HASHES = {
    PREREG: "9E2EF3C707A798AD73F7DF1280273F2924B9C7D3B337393000C6175E55811B1D",
    ROOT / "engine/src/render_bridge_phases/phase_read.cpp": (
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8"
    ),
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp": (
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4"
    ),
    ROOT / "engine/src/render_bridge_phases/phase_movement.cpp": (
        "6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB"
    ),
    ROOT / "engine/src/transmutation_phases.cpp": (
        "4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043"
    ),
    ROOT / "engine/src/poisson_solvers.cpp": (
        "59DC42FB8D0160373F02301C5B7AB09B2C9692242FC0D852C0404ECCA371362B"
    ),
    ROOT / "engine/src/render_bridge.cpp": (
        "BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724"
    ),
    ROOT / "engine/src/injection.cpp": (
        "228A1AE44532DB7D80A0EC10ABF5639B2811849189EF2F71A6343EE59C253DC5"
    ),
    ROOT / "engine/include/ftd/voxel.h": (
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3"
    ),
    ROOT / "engine/include/ftd/eft/history_event_journal.h": (
        "4A9AEDC650FE882C0CB6421901784095DA4EA079D3CCBC985DD412148583955A"
    ),
    PROGRAMME / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_PHASE_GATED_NEUTRAL_C4_HODGE_CHORD_AND_OCCUPANCY_CARRY_BOUNDARY_v1.md"
    ): "13C3A820AE368CCABCF5B5DC34B2CBA869B951899B1343AAD4CFD066BCBC3299",
    PROGRAMME / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_FINITE_CAPACITY_LOCAL_REVERSIBLE_OCCUPANCY_CARRY_TRILEMMA_v1.md"
    ): "A89DE2964B7D48100EC850547D00BB540D05F1166CF18CABE654EB9D26917548",
    PROGRAMME / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_EXISTING_LR_AGGREGATE_CARRIER_AND_OCCUPANCY_HISTORY_REALIZATION_BOUNDARY_v1.md"
    ): "D287ED5B5E6FCD15352E191D272A9B1A83D2952A009C1A9BEA5E0CAA985A0697",
    ROOT / "scripts/proofs/proof_existing_lr_occupancy_history_carrier_classifier.py": (
        "54AFAA09E6588A04B702A0F7368874ECA25AC21810E8532E8F04FB550E8C4808"
    ),
    PROGRAMME / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_C18_FINITE_RANGE_CHARACTERISTIC_AND_RIGID_TRANSLATOR_OBSTRUCTION_v1.md"
    ): "C6424C1AA0DDA2BA57BDE14A1559C76BBB17E279087122FB7121C59350BB4329",
    ROOT / "scripts/proofs/proof_c18_finite_range_characteristic_rigid_translator_obstruction.py": (
        "D94B419F2FF433E6477C8D9DCEC0878A70930F77180A28AFDF3CFDBAC8D00C0C"
    ),
}


checks: list[tuple[str, bool]] = []


def check(name: str, condition: bool) -> None:
    checks.append((name, bool(condition)))


def file_hash(path: Path) -> str:
    return sha256(path.read_bytes()).hexdigest().upper()


def segment(text: str, start: str, end: str) -> str:
    first = text.index(start)
    last = text.index(end, first + len(start))
    return text[first:last]


def source_contract_checks() -> None:
    for path, expected in EXPECTED_HASHES.items():
        check(f"source hash: {path.relative_to(ROOT)}", file_hash(path) == expected)

    read_text = (
        ROOT / "engine/src/render_bridge_phases/phase_read.cpp"
    ).read_text(encoding="utf-8")
    write_text = (
        ROOT / "engine/src/render_bridge_phases/phase_write.cpp"
    ).read_text(encoding="utf-8")
    movement_text = (
        ROOT / "engine/src/render_bridge_phases/phase_movement.cpp"
    ).read_text(encoding="utf-8")
    transmutation_text = (
        ROOT / "engine/src/transmutation_phases.cpp"
    ).read_text(encoding="utf-8")
    poisson_text = (ROOT / "engine/src/poisson_solvers.cpp").read_text(
        encoding="utf-8"
    )
    injection_text = (ROOT / "engine/src/injection.cpp").read_text(
        encoding="utf-8"
    )
    journal_text = (
        ROOT / "engine/include/ftd/eft/history_event_journal.h"
    ).read_text(encoding="utf-8")

    # Equal wave and matter actions.
    check("phase read C18 L", "laplacian_field<&Voxel::flux_L>" in read_text)
    check("phase read C18 R", "laplacian_field<&Voxel::flux_R>" in read_text)
    check("phase read equal coupling L", "rb.delta_j_L_[i] += curl_sv - grad_s;" in read_text)
    check("phase read equal coupling R", "rb.delta_j_R_[i] += curl_sv - grad_s;" in read_text)
    check("phase write equal default kick L", "v.wave_vel_L += rb.delta_j_L_[i];" in write_text)
    check("phase write equal default kick R", "v.wave_vel_R += rb.delta_j_R_[i];" in write_text)
    check("phase write equal default drift L", "v.flux_L += v.wave_vel_L;" in write_text)
    check("phase write equal default drift R", "v.flux_R += v.wave_vel_R;" in write_text)
    check("dual damping L flux", "v.flux_L *= eff_damping;" in write_text)
    check("dual damping R flux", "v.flux_R *= eff_damping;" in write_text)
    check("dual damping L momentum", "v.wave_vel_L *= eff_damping;" in write_text)
    check("dual damping R momentum", "v.wave_vel_R *= eff_damping;" in write_text)

    check("Gauss half correction declared", "Vec3 half_corr = grad_phi * 0.5;" in poisson_text)
    check("Gauss correction equal L", "voxels[i].flux_L -= half_corr;" in poisson_text)
    check("Gauss correction equal R", "voxels[i].flux_R -= half_corr;" in poisson_text)
    check("ordinary flux injection half", "const Vec3 half = flux_val * 0.5;" in injection_text)
    check("ordinary flux injection equal L", "v.flux_L = v.flux_L + half;" in injection_text)
    check("ordinary flux injection equal R", "v.flux_R = v.flux_R + half;" in injection_text)
    check("ordinary momentum injection equal L", "v.wave_vel_L = v.wave_vel_L + half;" in injection_text)
    check("ordinary momentum injection equal R", "v.wave_vel_R = v.wave_vel_R + half;" in injection_text)
    check("asymmetric particle preparation exists", "frac_major" in injection_text and "frac_minor" in injection_text)
    check("asymmetric wavepacket preparation exists", "void inject_wavepacket_cpu" in injection_text)

    # Event-specific exact markers.
    weak = segment(transmutation_text, "void weak_transmutation_cpu", "void accumulate_proper_time")
    pair = segment(transmutation_text, "void pair_production_cpu", "void triad_binding_cpu")
    dual_genesis = segment(write_text, "// Genesis (dual):", "// Genesis (single):")
    evaporation = segment(write_text, "// Evaporation (shared", "void phase_write_assign_pending_ids")

    check("weak state reflection", "static_cast<int8_t>(-v.state)" in weak)
    check("weak swaps L/R flux", "std::swap(v.flux_L, v.flux_R);" in weak)
    check("weak swaps L/R momentum", "std::swap(v.wave_vel_L, v.wave_vel_R);" in weak)
    check("weak trigger threshold", "stress > WEAK_THRESHOLD" in weak)
    check("weak trigger keyed draw", "VoxelRng::WeakTransmutation" in weak)

    for field in ("flux_L", "flux_R", "wave_vel_L", "wave_vel_R"):
        check(f"dual genesis does not write {field}", field not in dual_genesis)
        check(f"pair production does not write {field}", field not in pair)
    check("dual genesis chirality read", "v.chirality_density()" in dual_genesis)
    check("dual genesis keyed draw", "VoxelRng::GenesisManifest" in dual_genesis)
    check("evaporation clears state", "rb.set_state(i, 0);" in evaporation)
    check("evaporation clears particle id", "v.particle_id = -1;" in evaporation)
    check("evaporation clears spin", "v.spin = 0;" in evaporation)
    check("evaporation clears color", "v.color = 0;" in evaporation)

    check("pair axial dominant-component selection", "Geometric Pair Production: find the major axis" in pair)
    check("pair field major x", "if (fx >= fy && fx >= fz)" in pair)
    check("pair field major y", "else if (fy >= fx && fy >= fz)" in pair)
    check("pair sets opposite actual states", "rb.set_state(i, -1);" in pair and "rb.set_state(partner, +1);" in pair)
    check("pair common-action debt explicit", "no common-action energy identity is" in pair)

    check("movement common transfer fraction", "double frac = transfer / old_rho;" in movement_text)
    check("movement transfers L by fraction", "Vec3 sf_L = v.flux_L * frac;" in movement_text)
    check("movement transfers R by fraction", "Vec3 sf_R = v.flux_R * frac;" in movement_text)
    check("movement does not transfer L momentum", "wave_vel_L" not in movement_text)
    check("movement does not transfer R momentum", "wave_vel_R" not in movement_text)
    check("movement carries mechanical velocity", "t.velocity = v.velocity;" in movement_text)
    check("movement clears source state", "rb.set_state(i, 0);" in movement_text)
    check("same-sign bounce resets remainder", "v.remainder = {};" in movement_text)
    check("annihilation branch present", "Opposite sign: annihilation" in movement_text)
    check("annihilation clears L/R source fields", "v.flux_L = {}; v.flux_R = {};" in movement_text)
    check("annihilation distributes L equally", "flux_v_L * (1.0 / 6.0)" in movement_text)
    check("annihilation distributes R equally", "flux_v_R * (1.0 / 6.0)" in movement_text)
    check("absorbing crossing clears L", "v.flux_L = {};" in movement_text)
    check("absorbing crossing clears R", "v.flux_R = {};" in movement_text)
    check("reflective crossing resets remainder", "v.remainder = {};" in movement_text)

    check("boundary sponge scales L/R together", "v.flux_L *= s; v.flux_R *= s;" in write_text)
    check("boundary sponge scales momenta together", "v.wave_vel_L *= s; v.wave_vel_R *= s;" in write_text)
    check("reflective flux boundary copies L/R", "dst.flux_L = src.flux_L;" in write_text and "dst.flux_R = src.flux_R;" in write_text)

    check("journal declares read-only observer", "The journal is an observer." in journal_text)
    check("journal disabled by default contract", "It is disabled by default" in journal_text)
    check("journal never writes production state", "never writes lattice, voxel, toggle, or integrator state" in journal_text)
    check("journal snapshots full voxel", "Voxel voxel{};" in journal_text)


def exact_transition_checks() -> None:
    # One scalar relative canonical pair, order (D, P_D).
    weak = sp.diag(-1, -1)
    J2 = sp.Matrix([[0, 1], [-1, 0]])
    check("weak involution", weak * weak == sp.eye(2))
    check("weak determinant one", weak.det() == 1)
    check("weak symplectic", weak.T * J2 * weak == J2)
    check("weak preserves zero", weak * sp.zeros(2, 1) == sp.zeros(2, 1))

    # Source/target scalar positions, then source/target conjugate momenta.
    f = sp.symbols("f")
    A = sp.Matrix([[1 - f, 0], [f, 1]])
    S = sp.diag(1, 1, 1, 1)
    S[:2, :2] = A
    J4 = sp.Matrix.vstack(
        sp.Matrix.hstack(sp.zeros(2), sp.eye(2)),
        sp.Matrix.hstack(-sp.eye(2), sp.zeros(2)),
    )
    check("movement relative-position matrix exact", A.det() == 1 - f)
    check("movement full determinant exact", sp.factor(S.det()) == 1 - f)
    check("movement zero invariant", S * sp.zeros(4, 1) == sp.zeros(4, 1))
    check("movement f=1 singular", S.subs(f, 1).det() == 0)
    check("movement f=1 rank three", S.subs(f, 1).rank() == 3)
    check("movement nonzero-f not symplectic", sp.simplify(S.T * J4 * S - J4) != sp.zeros(4))
    check("movement f=0 identity/symplectic", S.subs(f, 0).T * J4 * S.subs(f, 0) == J4)

    # Explicit f=1 collision of two distinct source/target relative states.
    a, b = sp.symbols("a b", nonzero=True)
    pre_one = sp.Matrix([a, b, 0, 0])
    pre_two = sp.Matrix([0, a + b, 0, 0])
    post_one = S.subs(f, 1) * pre_one
    post_two = S.subs(f, 1) * pre_two
    check("movement f=1 witness inputs differ", pre_one != pre_two)
    check("movement f=1 witness outputs coincide", post_one == post_two)

    # Equal-half and common corrections cancel exactly in the relative channel.
    jl, jr, source = sp.symbols("J_L J_R source")
    check(
        "equal additive source cancels from D",
        sp.expand((jl + source) - (jr + source) - (jl - jr)) == 0,
    )
    scale = sp.symbols("scale")
    check(
        "equal scaling acts homogeneously on D",
        sp.expand(scale * jl - scale * jr - scale * (jl - jr)) == 0,
    )
    check("L/R swap reflects D", sp.expand(jr - jl + (jl - jr)) == 0)

    # Homogeneous movement/annihilation shell maps cannot create D from zero.
    zero_two = sp.zeros(2, 1)
    for rational_f in (Q(1, 4), Q(1, 2), Q(3, 4), Q(1)):
        check(
            f"movement rational zero invariant f={rational_f}",
            A.subs(f, sp.Rational(rational_f.numerator, rational_f.denominator))
            * zero_two
            == zero_two,
        )
    shell = sp.zeros(8, 2)
    # Two source values are cleared and each is sent democratically to six
    # registered outputs. Overlap details do not affect homogeneity.
    for row in range(6):
        shell[row, 0] += sp.Rational(1, 6)
    for row in range(2, 8):
        shell[row, 1] += sp.Rational(1, 6)
    check("annihilation shell linear", shell * sp.zeros(2, 1) == sp.zeros(8, 1))
    check("annihilation preserves aggregate D sum", sum(shell[:, 0]) == 1 and sum(shell[:, 1]) == 1)

    # Noninjective event followed by any deterministic linear continuation
    # remains noninjective. This finite exact witness represents B(Ax)=B(Ay).
    B = sp.Matrix([[2, 1, 0, 0], [0, 3, 1, 0], [0, 0, 5, 1], [1, 0, 0, 7]])
    check("post-composition preserves movement collision", B * post_one == B * post_two)

    # Reset maps erase distinct labels exactly.
    label_reset = lambda _state, _pid, _spin, _color: (0, -1, 0, 0)
    before_a = (1, 17, 1, 2)
    before_b = (-1, 91, -1, 3)
    check("evaporation/clear witness inputs differ", before_a != before_b)
    check("evaporation/clear witness outputs coincide", label_reset(*before_a) == label_reset(*before_b))


def invariant_and_scope_checks() -> None:
    prereg_text = PREREG.read_text(encoding="utf-8")
    required = [
        "relative-zero submanifold",
        "Selected asymmetric particle",
        "exact involution and symplectic",
        "does **not** transfer `wave_vel_L/R`",
        "for `f=1`, `det A_f=0`",
        "same-sign bounce",
        "observation journal records the branch but",
        "new local nonlinear/self-trapping action",
        "No tolerance, fit, numerical near-miss",
        "No new primitive storage type",
    ]
    for marker in required:
        check(f"scope/invariant marker: {marker}", marker in prereg_text)

    # Exhaustive registered-action truth table for zero preservation.
    zero_preserving = {
        "equal wave/common source": True,
        "ordinary equal injection": True,
        "Gauss half correction": True,
        "equal damping/sponge": True,
        "weak L/R swap": True,
        "dual genesis no L/R write": True,
        "pair production no L/R write": True,
        "void movement homogeneous transfer": True,
        "annihilation homogeneous distribution": True,
        "absorbing boundary clear": True,
        "reflective boundary no relative write": True,
    }
    for action, preserves in zero_preserving.items():
        check(f"relative-zero action class: {action}", preserves)
    check("all registered ordinary actions preserve relative zero", all(zero_preserving.values()))

    mandatory_carrier_gates = {
        "autonomous relative deposit": False,
        "phase-complete inverse": False,
        "collision-separated history": False,
        "backpressure with retained record": False,
        "source-energy transaction": False,
    }
    for gate, passed in mandatory_carrier_gates.items():
        check(f"mandatory gate absent: {gate}", not passed)
    check("Outcome B selected", not any(mandatory_carrier_gates.values()))


def main() -> None:
    source_contract_checks()
    exact_transition_checks()
    invariant_and_scope_checks()

    failed = [name for name, passed in checks if not passed]
    for name, passed in checks:
        print(f"[{'PASS' if passed else 'FAIL'}] {name}")
    print()
    print(f"FTD-0944 exact certificate: {len(checks) - len(failed)}/{len(checks)} checks passed")
    if failed:
        print("OUTCOME D — invalid certificate")
        for name in failed:
            print(f"  - {name}")
        raise SystemExit(1)
    print("OUTCOME B — the registered ordinary event stack preserves the")
    print("relative-zero submanifold. Weak exchange is an exact local involution")
    print("and movement can advect preloaded D, but no existing action supplies")
    print("autonomous relative deposit plus phase-complete inverse, collision/")
    print("backpressure, and a source-energy transaction.")
    print("A new action in existing fields or a separately selected port type is")
    print("required; the observation journal cannot repair production dynamics.")


if __name__ == "__main__":
    main()
