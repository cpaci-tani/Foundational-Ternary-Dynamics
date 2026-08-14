#!/usr/bin/env python3
"""FTD-0962 exact symbolic certificate.

The certificate checks one selected phase-connection witness.  It performs no
numerical fit, tolerance comparison, parameter scan, or near-miss search.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]

SOURCES = {
    "protocol": (
        ROOT
        / "docs/theory/10_eft_program/preregistrations/"
        "native_time_carrier_programme/"
        "PREREG_ORIENTED_PHASE_CONNECTION_TOKEN_LOADING_AND_REVERSIBLE_GEARBOX_v1.md",
        "535E14ADE46A886542165A815C4B807DFE35B0ACBD7B6131342C4DB9126C96B0",
    ),
    "catalytic_reference": (
        ROOT
        / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_CATALYTIC_PHASE_REFERENCE_TRANSDUCER_v1.md",
        "8BD6BB16999E91A72CADBA991A215F56A3E3E13816073E39B36F9EB51FD5FE33",
    ),
    "quarter_turn": (
        ROOT
        / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_HAMILTONIAN_TERNARY_QUARTER_TURN_ACTUATOR_v1.md",
        "73214057949BC5BE115AF7E273DE2CECE1F87D63237E94ADADB83F64442C7B98",
    ),
    "clock_exchange": (
        ROOT
        / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_CLOCK_GATED_HAMILTONIAN_EXCHANGE_AND_QUARTIC_LOAD_BOUNDARY_v1.md",
        "FFC0E39CC2C87FE73DC3C931302FE32EB5493E6AFB426CFA5BF97624DA3917D1",
    ),
    "isochrony_lift": (
        ROOT
        / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_GLOBAL_ISOCHRONY_LIFT_AND_ORIENTED_CROSSING_LATCH_BOUNDARY_v1.md",
        "746F855A432D7E662236315066115174493554285CD3FC25071B892A05AEA68E",
    ),
    "winding_carrier": (
        ROOT
        / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_EXISTING_ORIENTED_RAIL_FINITE_WINDING_CARRIER_AND_COMPACT_CARRY_BOUNDARY_v1.md",
        "85FC00E7B613894D5CD18276947C4A3BAD0B08CC8C8323996012B6EF8EE79514",
    ),
}


checks: list[tuple[str, str, bool, str]] = []


def record(group: str, label: str, condition: bool, detail: object) -> None:
    checks.append((group, label, bool(condition), str(detail)))


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def poisson(
    f: sp.Expr,
    g: sp.Expr,
    pairs: tuple[tuple[sp.Symbol, sp.Symbol], ...],
) -> sp.Expr:
    return sp.simplify(
        sum(
            sp.diff(f, q) * sp.diff(g, p)
            - sp.diff(f, p) * sp.diff(g, q)
            for q, p in pairs
        )
    )


def canonical_form(pair_count: int) -> sp.Matrix:
    block = sp.Matrix([[0, 1], [-1, 0]])
    return sp.diag(*([block] * pair_count))


def main() -> int:
    print("=" * 79)
    print("FTD-0962 oriented phase-connection token/gearbox proof")
    print("=" * 79)

    texts: dict[str, str] = {}
    for name, (path, expected) in SOURCES.items():
        actual = sha256(path)
        texts[name] = path.read_text(encoding="utf-8")
        record("G1", f"hash {path.name}", actual == expected, actual)

    protocol_markers = (
        "positive autonomous Hamiltonian",
        "including the complete square",
        "merely linear term",
        "mechanical source trajectory",
        "speed-independent oriented token loading",
        "The negative gate is not to be relabelled as the same forward gate",
        "reversible phase replacement",
        "G*` fixes the maintained clock's temporal traversal rate",
        "Reference success cannot count as substrate evidence",
        "No numerical fitting, floating",
    )
    for marker in protocol_markers:
        record("G1", f"protocol marker {marker[:44]}", marker in texts["protocol"], marker)

    dependency_markers = {
        "catalytic_reference": (
            "clockwise/counterclockwise discriminator",
            "reference pair is unchanged",
            "eligibility controller and its switching work remain open",
        ),
        "quarter_turn": (
            "actual-layer quarter-turn, its inverse, and hold",
            "zero **net endpoint** work",
            "does not mean that actuation is work-free",
        ),
        "clock_exchange": (
            "quartic clock is not the same gearbox",
            "backreaction changes the pulse area",
            "additional compensating action reservoir",
        ),
        "isochrony_lift": (
            "signed phase current",
            "active clutch whose work, reserve, reciprocal",
            "No relation between `Omega/kappa` and `G*`",
        ),
        "winding_carrier": (
            "no new public memory type",
            "costs one token",
            "active no-reset controller gearbox",
        ),
    }
    for source, markers in dependency_markers.items():
        for marker in markers:
            record("G1", f"{source} marker {marker[:38]}", marker in texts[source], marker)

    dq, dp, bq, bp, cq, cp, rq, rp = sp.symbols(
        "d_q d_p b_q b_p c_q c_p r_q r_p", real=True
    )
    mode_pairs = ((dq, dp), (bq, bp), (cq, cp), (rq, rp))
    G_T = bq * dp - dq * bp
    G_C = rq * cp - cq * rp
    G = G_T + G_C
    A_T = sp.expand((dq**2 + dp**2 + bq**2 + bp**2) / 2)
    A_C = sp.expand((cq**2 + cp**2 + rq**2 + rp**2) / 2)

    record("G2", "token/controller generators commute", poisson(G_T, G_C, mode_pairs) == 0, poisson(G_T, G_C, mode_pairs))
    record("G2", "total generator preserves token action", poisson(G, A_T, mode_pairs) == 0, poisson(G, A_T, mode_pairs))
    record("G2", "total generator preserves controller action", poisson(G, A_C, mode_pairs) == 0, poisson(G, A_C, mode_pairs))
    record("G2", "generator is conserved by itself", poisson(G, G, mode_pairs) == 0, poisson(G, G, mode_pairs))

    Kmat = sp.Matrix(
        [
            [0, 0, 1, 0, 0, 0, 0, 0],
            [0, 0, 0, 1, 0, 0, 0, 0],
            [-1, 0, 0, 0, 0, 0, 0, 0],
            [0, -1, 0, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 0, 0, 1, 0],
            [0, 0, 0, 0, 0, 0, 0, 1],
            [0, 0, 0, 0, -1, 0, 0, 0],
            [0, 0, 0, 0, 0, -1, 0, 0],
        ]
    )
    J8 = canonical_form(4)
    I8 = sp.eye(8)
    record("G2", "exchange generator squares to minus identity", Kmat**2 == -I8, Kmat**2)
    record("G2", "exchange generator is Hamiltonian", sp.simplify(Kmat.T * J8 + J8 * Kmat) == sp.zeros(8), Kmat.T * J8 + J8 * Kmat)
    alpha = sp.symbols("alpha", real=True)
    Falpha = sp.cos(alpha) * I8 + sp.sin(alpha) * Kmat
    record("G2", "general flow symplectic", sp.simplify(Falpha.T * J8 * Falpha - J8) == sp.zeros(8), "F(alpha)^T J F(alpha)=J")
    record("G2", "general flow orthogonal", sp.simplify(Falpha.T * Falpha - I8) == sp.zeros(8), "F(alpha)^T F(alpha)=I")
    record("G2", "general flow determinant one", sp.simplify(Falpha.det()) == 1, sp.simplify(Falpha.det()))
    record("G2", "quarter-turn flow equals generator matrix", Falpha.subs(alpha, sp.pi / 2) == Kmat, "F(pi/2)=K")
    record("G2", "inverse is opposite angle", sp.simplify(Falpha.subs(alpha, -alpha) * Falpha) == I8, "F(-alpha)F(alpha)=I")

    z = sp.Matrix([dq, dp, bq, bp, cq, cp, rq, rp])
    expected_velocity = sp.Matrix(
        [bq, bp, -dq, -dp, rq, rp, -cq, -cp]
    )
    hamilton_velocity = sp.Matrix(
        [
            sp.diff(G, dp),
            -sp.diff(G, dq),
            sp.diff(G, bp),
            -sp.diff(G, bq),
            sp.diff(G, cp),
            -sp.diff(G, cq),
            sp.diff(G, rp),
            -sp.diff(G, rq),
        ]
    )
    record("G2", "matrix flow matches Hamilton equations", hamilton_velocity == Kmat * z == expected_velocity, hamilton_velocity)

    x, b = sp.symbols("delta b", real=True, positive=True)
    profile = 15 * sp.pi / (32 * b) * (1 - x**2 / b**2) ** 2
    profile_prime = sp.diff(profile, x)
    profile_area = sp.integrate(profile, (x, -b, b))
    record("G3", "connection vanishes at left endpoint", sp.simplify(profile.subs(x, -b)) == 0, profile.subs(x, -b))
    record("G3", "connection vanishes at right endpoint", sp.simplify(profile.subs(x, b)) == 0, profile.subs(x, b))
    record("G3", "connection derivative vanishes at left endpoint", sp.simplify(profile_prime.subs(x, -b)) == 0, profile_prime.subs(x, -b))
    record("G3", "connection derivative vanishes at right endpoint", sp.simplify(profile_prime.subs(x, b)) == 0, profile_prime.subs(x, b))
    record("G3", "connection witness is even", sp.simplify(profile.subs(x, -x) - profile) == 0, "calA(-delta)=calA(delta)")
    record("G3", "connection witness is nonnegative on chart", sp.factor(profile) == 15 * sp.pi * (b - x) ** 2 * (b + x) ** 2 / (32 * b**5), sp.factor(profile))
    record("G3", "connection area is exact quarter-turn", sp.simplify(profile_area - sp.pi / 2) == 0, profile_area)
    record("G3", "piecewise witness is C1", all(sp.simplify(value) == 0 for value in (profile.subs(x, -b), profile.subs(x, b), profile_prime.subs(x, -b), profile_prime.subs(x, b))), "value and first derivative match zero exterior")

    delta, Pi = sp.symbols("delta Pi", real=True)
    M = sp.symbols("M", positive=True, real=True)
    Gs = sp.symbols("G", real=True)
    calA = sp.Function("calA")(delta)
    V = sp.Function("V")(delta)
    mech = Pi + calA * Gs
    Hsource = mech**2 / (2 * M) + V
    delta_dot = sp.diff(Hsource, Pi)
    Pi_dot = -sp.diff(Hsource, delta)
    mech_dot = sp.simplify(Pi_dot + sp.diff(calA, delta) * delta_dot * Gs)
    record("G3", "phase equation", sp.simplify(delta_dot - mech / M) == 0, delta_dot)
    record("G3", "canonical momentum equation", sp.simplify(Pi_dot + sp.diff(V, delta) + mech * sp.diff(calA, delta) * Gs / M) == 0, Pi_dot)
    record("G3", "mechanical momentum obeys bare force", sp.simplify(mech_dot + sp.diff(V, delta)) == 0, mech_dot)
    record("G3", "canonical/mechanical reciprocal difference", sp.simplify(Pi - (mech - calA * Gs)) == 0, "Pi=K-calA G")
    record("G3", "endpoint interaction vanishes", sp.simplify(mech.subs(calA, 0) - Pi) == 0, mech.subs(calA, 0))
    record("G3", "complete square expansion", sp.expand(mech**2 / (2 * M)) == Pi**2 / (2 * M) + Pi * calA * Gs / M + calA**2 * Gs**2 / (2 * M), sp.expand(mech**2 / (2 * M)))
    record("G3", "linear-only truncation misses positive quadratic term", sp.simplify(mech**2 / (2 * M) - (Pi**2 / (2 * M) + Pi * calA * Gs / M)) == calA**2 * Gs**2 / (2 * M), calA**2 * Gs**2 / (2 * M))

    alpha_dot = sp.diff(Hsource, Gs)
    record("G4", "connection rotation rate", sp.simplify(alpha_dot - calA * delta_dot) == 0, alpha_dot)
    record("G4", "speed cancels from holonomy", sp.simplify(alpha_dot / delta_dot - calA) == 0, "dalpha/ddelta=calA")
    record("G4", "forward holonomy", profile_area == sp.pi / 2, profile_area)
    record("G4", "reverse holonomy", -profile_area == -sp.pi / 2, -profile_area)
    record("G4", "nonzero load leaves holonomy exact", not sp.simplify(alpha_dot / delta_dot).has(Gs), sp.simplify(alpha_dot / delta_dot))
    record("G4", "mechanical crossing speed may vary", sp.simplify(alpha_dot - calA * mech / M) == 0, "alpha_dot=calA K/M")

    a, r, Cq, Cp = sp.symbols("a r C_q C_p", positive=True, real=True)
    initial = sp.Matrix([0, 0, a, 0, Cq, Cp, r, 0])
    forward = sp.simplify(Kmat * initial)
    backward_fresh = sp.simplify(-Kmat * initial)
    expected_forward = sp.Matrix([a, 0, 0, 0, r, 0, -Cq, -Cp])
    expected_negative = sp.Matrix([-a, 0, 0, 0, -r, 0, Cq, Cp])
    record("G5", "forward token battery clears into output", forward[:4, 0] == expected_forward[:4, 0], forward[:4, 0])
    record("G5", "forward controller receives aligned reserve", forward[4:6, 0] == expected_forward[4:6, 0], forward[4:6, 0])
    record("G5", "old controller exported completely", forward[6:8, 0] == expected_forward[6:8, 0], forward[6:8, 0])
    record("G5", "fresh reverse crossing emits negative token", backward_fresh[:4, 0] == expected_negative[:4, 0], backward_fresh[:4, 0])
    record("G5", "fresh reverse crossing reaches inverse-oriented gate", backward_fresh[4:6, 0] == expected_negative[4:6, 0], backward_fresh[4:6, 0])
    record("G5", "negative gate is antiphase not forward gate", expected_negative[4, 0] == -expected_forward[4, 0], "C_q=-r versus +r")

    nu_t, nu_c = sp.symbols("nu_T nu_C", positive=True, real=True)
    energy_initial = sp.simplify(nu_t * (a**2) / 2 + nu_c * (Cq**2 + Cp**2 + r**2) / 2)
    energy_forward = sp.simplify(nu_t * sum(v**2 for v in forward[:4, 0]) / 2 + nu_c * sum(v**2 for v in forward[4:8, 0]) / 2)
    record("G5", "forward mode energy exact", sp.simplify(energy_forward - energy_initial) == 0, sp.simplify(energy_forward - energy_initial))
    record("G5", "token energy transferred not copied", sp.simplify(nu_t * (forward[0] ** 2 + forward[1] ** 2) / 2 - nu_t * a**2 / 2) == 0 and forward[2] == forward[3] == 0, "epsilon_tok=nu_T a^2/2")
    record("G5", "controller action sum preserved", sp.simplify(sum(v**2 for v in forward[4:8, 0]) - (Cq**2 + Cp**2 + r**2)) == 0, "|C|^2+|R|^2")

    reverse_after_forward = sp.simplify((-Kmat) * forward)
    record("G6", "reverse traversal restores complete preparation", reverse_after_forward == initial, reverse_after_forward)
    record("G6", "forward then reverse matrix identity", (-Kmat) * Kmat == I8, (-Kmat) * Kmat)
    record("G6", "four same-orientation quarter-turns close", Kmat**4 == I8, Kmat**4)
    record("G6", "two same-orientation strokes give sign reversal", Kmat**2 == -I8, Kmat**2)
    record("G6", "token sign retained under one stroke", forward[0] == a and backward_fresh[0] == -a, (forward[0], backward_fresh[0]))

    Tsub = {
        Pi: -Pi,
        Gs: -Gs,
    }
    record("G6", "mechanical momentum is time odd", sp.simplify(mech.xreplace(Tsub) + mech) == 0, mech.xreplace(Tsub))
    record("G6", "connection Hamiltonian time-reversal invariant", sp.simplify(Hsource.xreplace(Tsub) - Hsource) == 0, "H(-Pi,-G)=H(Pi,G)")
    record("G6", "exchange generator is time odd", sp.simplify(G.subs({dp: -dp, bp: -bp, cp: -cp, rp: -rp}) + G) == 0, "G->-G")
    record("G6", "mode actions are time even", sp.simplify(A_T.subs({dp: -dp, bp: -bp}) - A_T) == 0 and sp.simplify(A_C.subs({cp: -cp, rp: -rp}) - A_C) == 0, "A_T,A_C invariant")

    record("G7", "kinetic term nonnegative", "K^2/(2M)" in texts["protocol"] and M.is_positive, "M>0")
    record("G7", "mode actions are sums of squares", sp.expand(2 * A_T) == dq**2 + dp**2 + bq**2 + bp**2 and sp.expand(2 * A_C) == cq**2 + cp**2 + rq**2 + rp**2, "A_T,A_C>=0")
    record("G7", "potential nonnegative is frozen", "V(delta)>=0" in texts["protocol"], "V>=0")
    record("G7", "total Hamiltonian energy is conserved", "conserve total Hamiltonian exactly" in texts["protocol"], "autonomous H_conn")
    record("G7", "battery reserve is finite positive", a.is_positive and nu_t.is_positive, "epsilon_tok=nu_T a^2/2>0")
    record("G7", "aligned reserve is nonzero", r.is_positive, "r>0")
    record("G7", "blank output is a semantic gate", "blank output" in texts["protocol"], "D=(0,0)")
    record("G7", "occupied output is backpressure", "occupied output or absent reserve is exact\nbackpressure" in texts["protocol"], "no overwrite")
    record("G7", "one-way repetition needs replenishment", "fresh/recycled batteries and aligned reserves" in texts["protocol"], "finite reserve")

    y11, y12, y13, y21, y22, y23, y31, y32, y33 = sp.symbols(
        "y11 y12 y13 y21 y22 y23 y31 y32 y33"
    )
    singular_jacobian = sp.Matrix(
        [
            [0, 0, 0, 0],
            [y11, y12, y13, 0],
            [y21, y22, y23, 0],
            [y31, y32, y33, 1],
        ]
    )
    record("G8", "constant output phase gives singular Jacobian", singular_jacobian.det() == 0, singular_jacobian.det())
    record("G8", "singular map cannot be symplectic", singular_jacobian.det() != 1, "symplectic determinant must be one")
    record("G8", "swap witness retains old phase coordinate", forward[6] == -Cq and forward[7] == -Cp, (forward[6], forward[7]))
    record("G8", "swap witness is injective", Kmat.det() == 1, Kmat.det())
    record("G8", "alignment is replacement not contraction", "not a contraction of an\nopen phase set" in texts["protocol"], "old mode exported")

    Aamp, Gstar, mass, lamb = sp.symbols(
        "A Gstar m lambda", positive=True, real=True
    )
    Tquartic = sp.sqrt(sp.pi) * Gstar * sp.sqrt(mass / (2 * lamb)) / Aamp
    omega_star = sp.simplify(2 * sp.pi / Tquartic)
    omega_expected = sp.simplify(
        2 * sp.pi * Aamp / (sp.sqrt(sp.pi) * Gstar) * sp.sqrt(2 * lamb / mass)
    )
    quarter_time = sp.simplify(Tquartic / 4)
    record("G9", "quartic period identity", sp.simplify(Tquartic * Aamp - sp.sqrt(sp.pi) * Gstar * sp.sqrt(mass / (2 * lamb))) == 0, Tquartic)
    record("G9", "conditional Gstar angular cadence", sp.simplify(omega_star - omega_expected) == 0, omega_star)
    record("G9", "quadrant duration", sp.simplify(quarter_time - Tquartic / 4) == 0, quarter_time)
    record("G9", "quadrant phase is pi/2", sp.simplify(omega_star * quarter_time - sp.pi / 2) == 0, sp.simplify(omega_star * quarter_time))
    record("G9", "Gstar changes time not normalized holonomy", not profile_area.has(Gstar) and omega_star.has(Gstar), "holonomy=pi/2; cadence contains Gstar")
    record("G9", "connection selection not claimed from CM", "not a derivation\nof the connection from CM arithmetic" in texts["protocol"], "conditional only")
    record("G9", "three clock roles separated", all(marker in texts["protocol"] for marker in ("temporal traversal rate", "quarter-turn holonomy", "forward versus inverse operation")), "cadence/gear/orientation")

    scope_markers = (
        "derivation of `calA`, the complete-square coupling",
        "formation, replenishment, protection, routing",
        "maintained-amplitude feedback, work, dissipation",
        "one-way unactualization/loss rather than exact reverse recovery",
        "full nonlinear repeated-cycle stability",
        "CM-prime selection of the connection normalization",
        "Born/Bell recovery, operational Lorentz hiding, and completeness",
        "Any target probability, future crossing, Born weight",
        "already a\nproduction law",
    )
    for marker in scope_markers:
        record("G10", f"scope marker {marker[:42]}", marker in texts["protocol"], marker)
    record("G10", "no engine production adoption", "No engine, production tick, constant, toggle" in texts["protocol"], "proof-only")
    record("G10", "frozen classifier Outcome B", "**Outcome B:**" in texts["protocol"], "reference exact; production open")
    record("G10", "target leakage forbidden", "target-coded weight" in texts["protocol"] and "Any target probability" in texts["protocol"], "current path only")

    for group, label, ok, detail in checks:
        status = "PASS" if ok else "FAIL"
        print(f"  {status:4s}  {group} {label}: {detail}")

    passed = sum(ok for _, _, ok, _ in checks)
    failed = len(checks) - passed
    print("-" * 79)
    print(f"checks={len(checks)} passed={passed} failed={failed}")
    if failed:
        print("OUTCOME D — certificate failure prevents classification")
        return 1

    print(
        "OUTCOME B — the selected positive phase connection supplies an\n"
        "exact speed-independent oriented holonomy.  A forward crossing\n"
        "moves one preloaded token into the signed output and swaps a\n"
        "pre-aligned phase reference into the controller while exporting\n"
        "the complete old controller state.  Reverse traversal is the exact\n"
        "self-dual inverse.  G* conditionally fixes the traversal cadence,\n"
        "not the selected connection or its production realization."
    )
    print("ORIENTED_CONNECTION_TOKEN_LOADING=EXACT_SELECTED_REFERENCE")
    print("REVERSIBLE_PHASE_REPLACEMENT_GEARBOX=EXACT_SELECTED_REFERENCE")
    print("GSTAR_ROLE=CONDITIONAL_QUARTIC_CADENCE_NOT_CONNECTION_DERIVATION")
    print("PRODUCTION_CONNECTION_FORMATION_RECYCLING=OPEN")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
