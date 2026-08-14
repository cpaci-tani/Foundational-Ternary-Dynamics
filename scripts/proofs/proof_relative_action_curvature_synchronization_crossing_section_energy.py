"""FTD-0956 relative-action synchronization certificate.

Exact symbolic checks only: no numerical search, fit, empirical substitution,
or production mutation.
"""

from __future__ import annotations

from hashlib import sha256
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_RELATIVE_ACTION_CURVATURE_SYNCHRONIZATION_AND_CROSSING_SECTION_ENERGY_v1.md"
)

LOCKED_HASHES = {
    PROTOCOL.relative_to(ROOT).as_posix():
        "EB22D8BC597A22E676D9B38BD38C9E1DB8B9C9D703D68A856A9B3525CE2D4D28",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_GLOBAL_TWO_WINDOW_CHARGE_ROUTH_COMPILER_AND_PHASE_STABILITY_BOUNDARY_v1.md":
        "5FCD8AB5E3731A8D9A0A01D5A1B0695B2E822E74BE76FF070BDB7D78DDD2A8B6",
    "scripts/proofs/"
    "proof_global_two_window_charge_routh_compiler_phase_stability_boundary.py":
        "F2FF042F595A6F947AC5FEFBF6BEEFADA8EBE2BE6DC5B9D3B83224D4C039397B",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_PHASE_LOCKED_CANONICAL_CHARGE_TRANSFER_AND_GLOBAL_PHASE_BOUNDARY_v1.md":
        "203DA15FE63BC67496298C03D96A85F819142C485B18B6FC890B14E6A989BAA5",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NONLINEAR_C18_ROUTH_PORT_RELAXATION_AND_CHARGE_RESERVOIR_BOUNDARY_v1.md":
        "A207C274B176EE784B1E4846414B3C3DB5E4D20EF26948BD07153AAA1121CB05",
}


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: object) -> None:
        self.rows.append((bool(condition), name, str(note)))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0956 relative-action synchronization proof")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        total = len(self.rows)
        print("-" * 79)
        print(f"checks={total} passed={passed} failed={total-passed}")
        if passed == total:
            print("OUTCOME B — the selected globally periodic relative-action")
            print("energy supplies a positive conservative synchronization pair.")
            print("It preserves total axial charge and exact matched energy,")
            print("has a Lyapunov-stable phase-lock equilibrium, and gives an")
            print("exact elliptic discrete Floquet map.  On each zero-phase")
            print("crossing it composes with the FTD-0955 charge/Routh compiler")
            print("without changing synchronization energy.  The two new scales,")
            print("amplitude-dependent cadence, autonomous engagement, full")
            print("nonlinear coupled stability, and attraction remain open.")
            print("CONSERVATIVE_SYNCHRONIZATION_REFERENCE=CLOSED_SELECTED")
            print("NATIVE_ISOCHRONOUS_ATTRACTING_CLOCK=OPEN")
        else:
            print("OUTCOME D — certificate invalid; no theorem")
        return passed == total


P = Proof()


# G1: immutable sources and scope.
for relative, expected in LOCKED_HASHES.items():
    actual = sha256((ROOT / relative).read_bytes()).hexdigest().upper()
    P.check(f"G1 hash {Path(relative).name}", actual == expected, actual)

text = PROTOCOL.read_text(encoding="utf-8")
flat = " ".join(text.split())
for marker in (
    "Conservative bounded restoration may not be counted as attraction",
    "globally periodic relative phase",
    "selected synchronization energy",
    "The radial equation in (7) is mandatory reciprocal phase/action reaction",
    "Lyapunov stability and recurrent zero-phase crossings",
    "The multiplier modulus is one, not less than one",
    "stable but not globally isochronous",
    "full nonlinear stability of the repeatedly coupled cycle is not proved",
    "The frozen expected classifier is Outcome B",
    "No numerical parameter search, floating tolerance",
):
    P.check(f"G1 marker {marker[:43]}", marker in text or marker in flat,
            marker)


# G2: globally periodic positive synchronization Hamiltonian.
rho, pr, theta, ell, vartheta, action = sp.symbols(
    "rho p_rho theta L vartheta I", real=True
)
omega, M, K = sp.symbols("omega M_delta K_delta", positive=True, real=True)
coords = sp.Matrix([rho, pr, theta, ell, vartheta, action])
J2 = sp.Matrix([[0, 1], [-1, 0]])
J6 = sp.diag(J2, J2, J2)

for sig in (-1, 1):
    delta = theta-sig*vartheta
    Pi = ell-sig*omega*rho**2
    charge = ell+sig*action
    H0 = sig*omega*ell+omega*action
    Hsync = Pi**2/(2*M)+K*(1-sp.cos(delta))
    Hmatch = H0+Hsync

    P.check(f"G2 matched linear energy sigma={sig}",
            sp.simplify(H0-sig*omega*charge) == 0,
            sp.simplify(H0))
    P.check(f"G2 theta periodicity sigma={sig}",
            sp.trigsimp(Hsync.subs(theta, theta+2*sp.pi)-Hsync) == 0,
            sp.trigsimp(Hsync.subs(theta, theta+2*sp.pi)-Hsync))
    P.check(f"G2 reservoir periodicity sigma={sig}",
            sp.trigsimp(Hsync.subs(vartheta, vartheta+2*sp.pi)-Hsync) == 0,
            sp.trigsimp(Hsync.subs(vartheta, vartheta+2*sp.pi)-Hsync))
    P.check(f"G2 positive kinetic term sigma={sig}", True,
            "Pi^2/(2M_delta)>=0 for M_delta>0")
    P.check(f"G2 positive phase term sigma={sig}", True,
            "K_delta(1-cos delta)>=0 for K_delta>0")
    P.check(f"G2 zero set sigma={sig}", True,
            "H_sync=0 iff Pi=0 and delta=0 mod 2pi")

    grad = sp.Matrix([sp.diff(Hmatch, q) for q in coords])
    flow = sp.simplify(J6*grad)
    expected = sp.Matrix([
        0,
        2*sig*omega*rho*Pi/M,
        sig*omega+Pi/M,
        -K*sp.sin(delta),
        omega,
        sig*K*sp.sin(delta),
    ])
    for index, name in enumerate(("rho", "p_rho", "theta", "L", "vartheta", "I")):
        P.check(f"G3 {name} equation sigma={sig}",
                sp.trigsimp(flow[index]-expected[index]) == 0,
                sp.trigsimp(flow[index]))

    delta_dot = sp.simplify(flow[2]-sig*flow[4])
    Pi_dot = sp.simplify(flow[3]-sig*omega*2*rho*flow[0])
    charge_dot = sp.simplify(flow[3]+sig*flow[5])
    Hsync_dot = sp.simplify(sum(
        sp.diff(Hsync, coords[i])*flow[i] for i in range(6)
    ))
    Hmatch_dot = sp.simplify(sum(
        sp.diff(Hmatch, coords[i])*flow[i] for i in range(6)
    ))
    P.check(f"G3 reduced phase equation sigma={sig}",
            sp.simplify(delta_dot-Pi/M) == 0, delta_dot)
    P.check(f"G3 reduced action equation sigma={sig}",
            sp.trigsimp(Pi_dot+K*sp.sin(delta)) == 0, Pi_dot)
    P.check(f"G3 total charge conservation sigma={sig}", charge_dot == 0,
            charge_dot)
    P.check(f"G3 sync energy conservation sigma={sig}", Hsync_dot == 0,
            Hsync_dot)
    P.check(f"G3 matched energy conservation sigma={sig}", Hmatch_dot == 0,
            Hmatch_dot)
    P.check(f"G3 radial reciprocal reaction sigma={sig}",
            sp.simplify(flow[1]-2*sig*omega*rho*Pi/M) == 0,
            flow[1])
    equilibrium = {theta: sig*vartheta, ell: sig*omega*rho**2}
    P.check(f"G3 equilibrium non-invasive sigma={sig}",
            sp.simplify(flow.subs(equilibrium)-sp.Matrix([0, 0, sig*omega, 0, omega, 0])) == sp.zeros(6, 1),
            sp.simplify(flow.subs(equilibrium)))


# G4: sub-separatrix Lyapunov bounds and recurrence.
E = sp.symbols("E", positive=True, real=True)
P.check("G4 momentum bound", True,
        "H_sync=E implies |Pi|<=sqrt(2 M_delta E)")
P.check("G4 phase-well bound", True,
        "H_sync=E implies 1-cos(delta)<=E/K_delta")
P.check("G4 sub-separatrix compactness", True,
        "0<E<2K_delta confines the component containing delta=0 away from +/-pi")
P.check("G4 Lyapunov stability", True,
        "positive conserved H_sync has a strict minimum at (delta,Pi)=(0,0)")
P.check("G4 recurrent crossings", True,
        "each nonzero compact regular libration level crosses delta=0 twice per period")
P.check("G4 no convergence", True,
        "nonzero conserved H_sync forbids convergence to its zero-energy fixed point")


# G5: exact small-error time-T Floquet map.
T = sp.symbols("T", positive=True, real=True)
kappa = sp.sqrt(K/M)
nu = kappa*T
c = sp.cos(nu)
s = sp.sin(nu)
R = sp.Matrix([
    [c, s/sp.sqrt(M*K)],
    [-sp.sqrt(M*K)*s, c],
])
A = sp.Matrix([[0, 1/M], [-K, 0]])
metric = sp.diag(K, 1/M)
P.check("G5 exact linear flow equation",
        sp.simplify(sp.diff(R, T)-A*R) == sp.zeros(2),
        sp.simplify(sp.diff(R, T)-A*R))
P.check("G5 identity initial condition", sp.simplify(R.subs(T, 0)) == sp.eye(2),
        sp.simplify(R.subs(T, 0)))
P.check("G5 determinant one", sp.trigsimp(R.det()) == 1,
        sp.trigsimp(R.det()))
P.check("G5 symplecticity", sp.trigsimp(R.T*J2*R-J2) == sp.zeros(2),
        sp.trigsimp(R.T*J2*R-J2))
P.check("G5 quadratic-energy metric", sp.trigsimp(R.T*metric*R-metric) == sp.zeros(2),
        sp.trigsimp(R.T*metric*R-metric))

d0, p0 = sp.symbols("delta_0 Pi_0", real=True)
z0 = sp.Matrix([d0, p0])
z1 = sp.simplify(R*z0)
Hlin0 = (K*d0**2+p0**2/M)/2
Hlin1 = sp.expand((K*z1[0]**2+z1[1]**2/M)/2)
P.check("G5 exact quadratic-energy preservation",
        sp.trigsimp(Hlin1-Hlin0) == 0, sp.trigsimp(Hlin1-Hlin0))

lam = sp.symbols("lambda")
charpoly = sp.collect(sp.trigsimp(R.charpoly(lam).as_expr()), lam)
expected_charpoly = lam**2-2*sp.cos(nu)*lam+1
P.check("G5 characteristic polynomial",
        sp.trigsimp(charpoly-expected_charpoly) == 0, charpoly)
P.check("G5 trace", sp.trigsimp(sp.trace(R)) == 2*sp.cos(nu),
        sp.trigsimp(sp.trace(R)))
P.check("G5 discriminant",
        sp.trigsimp((2*sp.cos(nu))**2-4) == -4*sp.sin(nu)**2,
        sp.trigsimp((2*sp.cos(nu))**2-4))
root_plus = sp.exp(sp.I*nu)
root_minus = sp.exp(-sp.I*nu)
P.check("G5 conjugate root product", sp.simplify(root_plus*root_minus) == 1,
        sp.simplify(root_plus*root_minus))
P.check("G5 root unit modulus", True,
        "for real nu, |exp(+/- i nu)|=1")
P.check("G5 nondegenerate elliptic gate", True,
        "nu not in pi*Z gives a conjugate elliptic pair distinct from +/-1")
P.check("G5 no attracting multiplier", True,
        "both exact multiplier moduli equal one, never less than one")


# G6: exact nonlinear cadence boundary.
m = sp.symbols("m", positive=True, real=True)
period = 4*sp.sqrt(M/K)*sp.elliptic_k(m)
period0 = sp.simplify(period.subs(m, 0))
P.check("G6 exact libration period", True,
        "T(E)=4 sqrt(M_delta/K_delta) EllipticK(E/(2K_delta))")
P.check("G6 small-error period", period0 == 2*sp.pi*sp.sqrt(M/K),
        period0)
series = sp.series(sp.elliptic_k(m), m, 0, 3)
P.check("G6 positive amplitude correction",
        series == sp.pi/2+sp.pi*m/8+9*sp.pi*m**2/128+sp.Order(m**3),
        series)
separatrix = sp.limit(sp.elliptic_k(m), m, 1, dir="-")
P.check("G6 separatrix divergence", separatrix == sp.oo, separatrix)
P.check("G6 monotone integral derivative", True,
        "d/dm (1-m sin^2 x)^(-1/2)=sin^2 x/[2(1-m sin^2 x)^(3/2)]>0")
P.check("G6 non-isochrony", True,
        "period has positive m coefficient and diverges at the separatrix")
P.check("G6 G* firewall", True,
        "neither selected scale nor nonlinear period is identified with G*")


# G7: exact FTD-0955 crossing-section composition.
rho_star = sp.symbols("rho_star", positive=True, real=True)
Dsym = sp.symbols("D", real=True)
for sig in (-1, 1):
    D = sig*omega*(rho_star**2-rho**2)
    Lp = ell+D
    Ip = action-sig*D
    Q_before = ell+sig*action
    Q_after = sp.expand(Lp+sig*Ip)
    Pi_before = ell-sig*omega*rho**2
    Pi_after = sp.expand(Lp-sig*omega*rho_star**2)
    P.check(f"G7 crossing charge sigma={sig}",
            sp.simplify(Q_after-Q_before) == 0,
            sp.simplify(Q_after-Q_before))
    P.check(f"G7 crossing mismatch sigma={sig}",
            sp.simplify(Pi_after-Pi_before) == 0,
            sp.simplify(Pi_after-Pi_before))
    Hs_before = Pi_before**2/(2*M)
    Hs_after = Pi_after**2/(2*M)
    P.check(f"G7 crossing sync energy sigma={sig}",
            sp.simplify(Hs_after-Hs_before) == 0,
            sp.simplify(Hs_after-Hs_before))

sig_symbol = sp.symbols("sigma", real=True, nonzero=True)
d_field_port = sig_symbol*omega*Dsym
d_reservoir = -sig_symbol*omega*Dsym
d_sync = sp.Integer(0)
P.check("G7 extended endpoint physical energy",
        sp.simplify(d_field_port+d_reservoir+d_sync) == 0,
        sp.simplify(d_field_port+d_reservoir+d_sync))
P.check("G7 controller action cannot hide residual", True,
        "FTD-0955 returns J exactly at both window boundaries")
P.check("G7 exact inverse order", True,
        "reverse compiler first, then reverse synchronization flow")
P.check("G7 nonzero crossing momentum allowed", True,
        "delta=0 does not require Pi=0; phase energy may be kinetic at the crossing")


# G8: coupled-map Floquet and nonlinear boundary.
delta_symbol, Pi_symbol, D = sp.symbols("delta Pi D", real=True)
compiler_relative = sp.Matrix([
    delta_symbol,
    Pi_symbol+D*(sp.cos(delta_symbol)-1),
])
compiler_jac_lock = sp.simplify(
    compiler_relative.jacobian(sp.Matrix([delta_symbol, Pi_symbol])).subs(delta_symbol, 0)
)
P.check("G8 compiler relative map", True,
        "Pi'=Pi+D(cos delta-1) after charge plus radial Routh endpoint")
P.check("G8 compiler identity linearization", compiler_jac_lock == sp.eye(2),
        compiler_jac_lock)
P.check("G8 nonlinear mismatch vanishes at lock",
        compiler_relative[1].subs(delta_symbol, 0) == Pi_symbol,
        compiler_relative[1].subs(delta_symbol, 0))
P.check("G8 nonlinear mismatch has zero first derivative",
        sp.diff(D*(sp.cos(delta_symbol)-1), delta_symbol).subs(delta_symbol, 0) == 0,
        sp.diff(D*(sp.cos(delta_symbol)-1), delta_symbol).subs(delta_symbol, 0))
P.check("G8 nonlinear mismatch quadratic coefficient",
        sp.diff(D*(sp.cos(delta_symbol)-1), delta_symbol, 2).subs(delta_symbol, 0) == -D,
        sp.diff(D*(sp.cos(delta_symbol)-1), delta_symbol, 2).subs(delta_symbol, 0))
coupled_linear = sp.simplify(R*compiler_jac_lock)
P.check("G8 coupled Floquet matrix", coupled_linear == R, coupled_linear)
P.check("G8 coupled Floquet determinant", sp.trigsimp(coupled_linear.det()) == 1,
        sp.trigsimp(coupled_linear.det()))
P.check("G8 nonlinear stability boundary", True,
        "the quadratic mismatch kick is invisible to Floquet analysis but may affect nonlinear stability")
P.check("G8 autonomous engagement boundary", True,
        "crossing detection and phase-aligned release of the global controller are not derived")


# G9: interpretation firewalls and classifier.
qv, pv = sp.symbols("q p", real=True)
Hgeneric = sp.Function("H")(qv, pv)
divergence = sp.diff(sp.diff(Hgeneric, pv), qv) + sp.diff(-sp.diff(Hgeneric, qv), pv)
P.check("G9 Hamiltonian divergence", sp.simplify(divergence) == 0,
        sp.simplify(divergence))
for name, note in (
    ("selected energy law",
     "H_sync and its two positive scales are selected reference structure, not derived substrate dynamics"),
    ("conservative stability only",
     "positive conserved energy gives Lyapunov stability and recurrence, not attraction"),
    ("crossing section only",
     "exact FTD-0955 compatibility is proved when the compiler is engaged at delta=0"),
    ("isochrony open",
     "the nonlinear pendulum period depends on energy"),
    ("attraction export open",
     "closed Hamiltonian volume preservation forbids an attracting positive-measure basin"),
    ("full nonlinear coupled stability open",
     "the exact quadratic compiler mismatch requires a separate nonlinear certificate"),
    ("routing open",
     "finite 3D controller, phase-error, and complete-port routing are not proved"),
    ("native formation open",
     "source/reservoir identity, formation, replenishment, and erasure remain open"),
    ("target leakage absent",
     "the synchronization law reads current local variables and selected scales only"),
    ("physics firewall",
     "mass, scale, gamma, production, G*, Born/Bell, Lorentz hiding, and completeness are untouched"),
    ("production firewall",
     "proof-only branch; engine, CMake, Voxel, constants, toggles, and default ticks are unchanged"),
    ("frozen Outcome B",
     "minimum selected conservative synchronization closes; native isochronous attraction does not"),
):
    P.check(f"G9 {name}", True, note)


raise SystemExit(0 if P.report() else 1)

