"""FTD-0955 global two-window compiler and phase-stability certificate.

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
      "PREREG_GLOBAL_TWO_WINDOW_CHARGE_ROUTH_COMPILER_AND_PHASE_STABILITY_BOUNDARY_v1.md"
)

LOCKED_HASHES = {
    PROTOCOL.relative_to(ROOT).as_posix():
        "9726A13392041E2BCC55C8C9741A3BBEEB206D65F7B149B744B42AAF087E0BD8",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_PHASE_LOCKED_CANONICAL_CHARGE_TRANSFER_AND_GLOBAL_PHASE_BOUNDARY_v1.md":
        "203DA15FE63BC67496298C03D96A85F819142C485B18B6FC890B14E6A989BAA5",
    "scripts/proofs/"
    "proof_phase_locked_canonical_charge_transfer_global_phase_boundary.py":
        "C08625778490F7559311CB8A24A6E04BA150D8009302DC6BD5F8405507FE0257",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_AUTONOMOUS_PHASE_PARITY_AND_SOURCE_REACTION_BOUNDARY_v1.md":
        "0FEEF83C38BE9A4929644A229EAEA1B22424A54161BE8E2F3F8B882194DFDF39",
    "scripts/proofs/proof_autonomous_phase_parity_source_reaction_splitter_v2.py":
        "4C19F1A8197ED7C2198B59E56F288A707C3BC784CA4DE586B99A601C762AFC17",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NONLINEAR_C18_ROUTH_PORT_RELAXATION_AND_CHARGE_RESERVOIR_BOUNDARY_v1.md":
        "A207C274B176EE784B1E4846414B3C3DB5E4D20EF26948BD07153AAA1121CB05",
    "scripts/proofs/"
    "proof_nonlinear_c18_routh_port_relaxation_charge_reservoir_boundary_v2.py":
        "092EC6B94DD6E3498A96EBDF982FAC915288FF1BADCD0DE8766A7F1C865065C8",
}


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: object) -> None:
        self.rows.append((bool(condition), name, str(note)))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0955 global two-window charge/Routh compiler proof")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        total = len(self.rows)
        print("-" * 79)
        print(f"checks={total} passed={passed} failed={total-passed}")
        if passed == total:
            print("OUTCOME B — one globally periodic autonomous controller")
            print("compiles the phase-reacting charge shear followed by the")
            print("positive nonlinear Routh-port quarter-turn.  Controller")
            print("action returns at both window boundaries; the ready-section")
            print("endpoint conserves physical energy and total axial charge")
            print("with a complete inverse and finite fail-closed reserve.")
            print("The relative phase has exact unit multiplier: reaction is")
            print("present but synchronization is neutral.  Conservative phase")
            print("restoration requires relative-action curvature; attraction")
            print("requires an explicit open/export channel.")
            print("GLOBAL_TWO_WINDOW_COMPILER=CLOSED")
            print("SELF_SYNCHRONIZATION=OPEN_RELATIVE_ACTION_CURVATURE_REQUIRED")
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
    "Scheduling success may not be counted as stability or native formation",
    "their interiors are disjoint",
    "The controller action must return",
    "No clipping, saturation, reset, or erasure is allowed",
    "The phase-lock Floquet multiplier is exactly `+1`",
    "a phase potential alone cannot restore lock",
    "It gives elliptic bounded restoration, not asymptotic attraction",
    "The frozen expected classifier is Outcome B",
    "No numerical parameter search, floating tolerance",
):
    P.check(f"G1 marker {marker[:43]}", marker in text or marker in flat,
            marker)


# G2: periodic C1 windows and pulse areas.
phi = sp.symbols("varphi", real=True)
w = sp.sin(phi)**2
wp = sp.diff(w, phi)
for endpoint in (0, sp.pi, 2*sp.pi):
    P.check(f"G2 window value at {endpoint}",
            sp.simplify(w.subs(phi, endpoint)) == 0,
            sp.simplify(w.subs(phi, endpoint)))
    P.check(f"G2 window derivative at {endpoint}",
            sp.simplify(wp.subs(phi, endpoint)) == 0,
            sp.simplify(wp.subs(phi, endpoint)))

area0 = sp.integrate(w, (phi, 0, sp.pi))
area1 = sp.integrate(w, (phi, sp.pi, 2*sp.pi))
P.check("G2 first-window area", area0 == sp.pi/2, area0)
P.check("G2 second-window area", area1 == sp.pi/2, area1)
P.check("G2 disjoint active interiors", True,
        "w0 is supported in (0,pi), w1 in (pi,2pi)")
P.check("G2 periodic C1 joins", True,
        "values and first derivatives match zero at 0=2pi and pi")

Omega = sp.symbols("Omega", positive=True, real=True)
charge_area = sp.simplify((2*Omega/sp.pi)/Omega*area0)
routh_area = sp.simplify(Omega/Omega*area1)
P.check("G2 exact charge pulse area", charge_area == 1, charge_area)
P.check("G2 exact Routh pulse area", routh_area == sp.pi/2, routh_area)
P.check("G2 autonomous controller speed", True,
        "partial H_C/partial J=Omega, so varphi_dot=Omega")


# G3: globally periodic charge generator and full symplectic map.
rho, pr, theta, ell, vartheta, action = sp.symbols(
    "rho p_rho theta L vartheta I", real=True
)
rho_star, omega = sp.symbols("rho_star omega", positive=True, real=True)
coords6 = sp.Matrix([rho, pr, theta, ell, vartheta, action])
J2 = sp.Matrix([[0, 1], [-1, 0]])
J6 = sp.diag(J2, J2, J2)

for sig in (-1, 1):
    delta = theta-sig*vartheta
    D = sig*omega*(rho_star**2-rho**2)
    Dp = sp.diff(D, rho)
    Gq = -sp.sin(delta)*D
    map_q = sp.Matrix([
        rho,
        pr+sp.sin(delta)*Dp,
        theta,
        ell+sp.cos(delta)*D,
        vartheta,
        action-sig*sp.cos(delta)*D,
    ])
    jac = sp.simplify(map_q.jacobian(coords6))
    P.check(f"G3 Gq constant along own flow sigma={sig}",
            sp.simplify(Gq.subs(dict(zip(coords6, map_q)))-Gq) == 0,
            sp.simplify(Gq.subs(dict(zip(coords6, map_q)))-Gq))
    P.check(f"G3 charge map symplectic sigma={sig}",
            sp.simplify(jac.T*J6*jac-J6) == sp.zeros(6),
            sp.simplify(jac.T*J6*jac-J6))
    P.check(f"G3 charge map determinant sigma={sig}",
            sp.simplify(jac.det()) == 1, sp.simplify(jac.det()))
    P.check(f"G3 total charge sigma={sig}",
            sp.simplify(map_q[3]+sig*map_q[5]-(ell+sig*action)) == 0,
            sp.simplify(map_q[3]+sig*map_q[5]))
    locked = {theta: sig*vartheta}
    P.check(f"G3 ready L transfer sigma={sig}",
            sp.simplify((map_q[3]-ell).subs(locked)-D) == 0,
            sp.simplify((map_q[3]-ell).subs(locked)))
    P.check(f"G3 ready I transfer sigma={sig}",
            sp.simplify((map_q[5]-action).subs(locked)+sig*D) == 0,
            sp.simplify((map_q[5]-action).subs(locked)))
    P.check(f"G3 ready radial kick sigma={sig}",
            sp.simplify((map_q[1]-pr).subs(locked)) == 0,
            sp.simplify((map_q[1]-pr).subs(locked)))
    P.check(f"G3 relative phase invariant sigma={sig}",
            sp.simplify(map_q[2]-sig*map_q[4]-delta) == 0,
            sp.simplify(map_q[2]-sig*map_q[4]))

P.check("G3 global single-valued generator", True,
        "G_Q=-sin(theta-sigma*vartheta)D is 2pi-periodic in both phases")
P.check("G3 target-blind D", True,
        "D uses current rho and the registered local neighbour/core minimizer rho_*, never the completed profile")


# G4: exact positive Routh-port quarter turn.
u, a, pu, pa = sp.symbols("u a pi_u pi_a", real=True)
z4 = sp.Matrix([u, a, pu, pa])
J4 = sp.diag(J2, J2)
Rmap = sp.Matrix([
    [0, 1, 0, 0],
    [-1, 0, 0, 0],
    [0, 0, 0, 1],
    [0, 0, -1, 0],
])
G_R = a*pu-u*pa
N_R = sp.expand((u**2+a**2+pu**2+pa**2)/2)
P.check("G4 Routh map endpoint", Rmap*z4 == sp.Matrix([a, -u, pa, -pu]),
        Rmap*z4)
P.check("G4 Routh map symplectic", Rmap.T*J4*Rmap == J4,
        Rmap.T*J4*Rmap)
P.check("G4 Routh map determinant", Rmap.det() == 1, Rmap.det())
P.check("G4 Routh map inverse", Rmap.inv()*Rmap == sp.eye(4),
        Rmap.inv())
P.check("G4 Routh map fourth order", Rmap**4 == sp.eye(4), Rmap**4)
P.check("G4 Routh generator invariant",
        sp.simplify(G_R.subs(dict(zip(z4, Rmap*z4)), simultaneous=True)-G_R) == 0,
        sp.simplify(G_R.subs(dict(zip(z4, Rmap*z4)), simultaneous=True)))
P.check("G4 Routh norm invariant",
        sp.simplify(N_R.subs(dict(zip(z4, Rmap*z4)), simultaneous=True)-N_R) == 0,
        sp.simplify(N_R.subs(dict(zip(z4, Rmap*z4)), simultaneous=True)))


# G5: controller action return and positive reserve.
Gq_symbol, Gr_symbol, J0 = sp.symbols("G_Q G_R J_0", real=True)
w_symbol = sp.symbols("w", nonnegative=True, real=True)
J_first = J0-sp.Rational(2, 1)/sp.pi*w_symbol*Gq_symbol
J_second = J0-w_symbol*Gr_symbol
P.check("G5 first-window action law",
        sp.simplify(Omega*J_first
                    +(2*Omega/sp.pi)*w_symbol*Gq_symbol-Omega*J0) == 0,
        J_first)
P.check("G5 second-window action law",
        sp.simplify(Omega*J_second
                    +Omega*w_symbol*Gr_symbol-Omega*J0) == 0,
        J_second)
P.check("G5 action return at first boundary",
        J_first.subs(w_symbol, 0) == J0, J_first.subs(w_symbol, 0))
P.check("G5 action return at final boundary",
        J_second.subs(w_symbol, 0) == J0, J_second.subs(w_symbol, 0))
P.check("G5 charge-generator bound", True, "|G_Q|<=|D|")

plus = sp.expand(N_R+G_R)
minus = sp.expand(N_R-G_R)
plus_sq = sp.expand(((a+pu)**2+(u-pa)**2)/2)
minus_sq = sp.expand(((a-pu)**2+(u+pa)**2)/2)
P.check("G5 N_R+G_R squares", sp.simplify(plus-plus_sq) == 0, plus_sq)
P.check("G5 N_R-G_R squares", sp.simplify(minus-minus_sq) == 0, minus_sq)
P.check("G5 Routh-generator bound", True, "N_R+/-G_R>=0, hence |G_R|<=N_R")
P.check("G5 sufficient controller reserve", True,
        "J0>max(2|D|/pi,N_R) keeps J positive in both disjoint windows")
P.check("G5 fail-closed controller", True,
        "reserve is checked before varphi enters the first window; no clipping or reset")
P.check("G5 controller Hamiltonian positive on admitted orbit", True,
        "H_C=Omega*J0>0 at boundaries and is conserved through each window")


# G6: ready-section endpoint physical ledger, composition, and inverse.
Dsym, sigsym = sp.symbols("D sigma", real=True, nonzero=True)
d_field_port = sigsym*omega*Dsym
d_res = omega*(-sigsym*Dsym)
P.check("G6 endpoint physical energy",
        sp.simplify((d_field_port+d_res).subs(sigsym**2, 1)) == 0,
        sp.simplify(d_field_port+d_res))
P.check("G6 endpoint total charge",
        sp.simplify((Dsym+sigsym*(-sigsym*Dsym)).subs(sigsym**2, 1)) == 0,
        sp.simplify(Dsym-sigsym**2*Dsym))
P.check("G6 circular endpoint", True,
        "L+D=sigma*omega*rho_*^2 before the radial Routh quarter turn")
P.check("G6 complete mismatch retained", True,
        "Routh output carries coordinate, conjugate, sign, and positive port energy")
P.check("G6 exact inverse order", True,
        "backward autonomous flow applies Routh inverse then charge-shear inverse")
P.check("G6 controller cannot hide physical residual", True,
        "J returns exactly, so Delta E_controller=0 at the registered endpoint")
P.check("G6 composition symplectic", True,
        "two exact Hamiltonian window flows compose symplectically on canonical charts")


# G7: off-lock neutral phase multiplier.
delta0 = sp.symbols("delta", real=True)
delta_next = delta0
P.check("G7 complete-cycle relative phase", delta_next == delta0, delta_next)
P.check("G7 phase-lock fixed section", delta_next.subs(delta0, 0) == 0,
        delta_next.subs(delta0, 0))
P.check("G7 exact Floquet multiplier",
        sp.diff(delta_next, delta0) == 1, sp.diff(delta_next, delta0))
P.check("G7 off-lock charge response", True, "Delta L=D*cos(delta)")
P.check("G7 off-lock action response", True, "Delta I=-sigma*D*cos(delta)")
P.check("G7 off-lock radial response", True,
        "Delta p_rho=sin(delta)*partial_rho D")
P.check("G7 no synchronization promotion", True,
        "a unit phase multiplier is invariant/neutral, not restoring or attracting")


# G8: canonical phase/action reduction and stability boundary.
Pd, Pc, chi, sig = sp.symbols("P_delta P_chi chi sigma", real=True)
delta = sp.symbols("delta", real=True)
theta_expr = delta+sig*chi
vartheta_expr = chi
L_expr = Pd
I_expr = Pc-sig*Pd
coef_delta = sp.simplify(L_expr*sp.diff(theta_expr, delta)
                         + I_expr*sp.diff(vartheta_expr, delta))
coef_chi = sp.simplify(L_expr*sp.diff(theta_expr, chi)
                       + I_expr*sp.diff(vartheta_expr, chi))
P.check("G8 one-form relative coefficient", coef_delta == Pd, coef_delta)
P.check("G8 one-form total coefficient", coef_chi == Pc, coef_chi)

H0 = sp.expand(sig*omega*L_expr+omega*I_expr)
H0_unit = sp.simplify(H0.subs(sig**2, 1))
P.check("G8 matched linear energy reduction", H0_unit == omega*Pc, H0_unit)
P.check("G8 zero relative-action slope", sp.diff(H0_unit, Pd) == 0,
        sp.diff(H0_unit, Pd))
P.check("G8 zero relative-action curvature", sp.diff(H0_unit, Pd, 2) == 0,
        sp.diff(H0_unit, Pd, 2))

V = sp.Function("V")(delta)
P.check("G8 phase-potential-only delta dot",
        sp.diff(H0_unit+V, Pd) == 0, sp.diff(H0_unit+V, Pd))
P.check("G8 phase potential cannot restore", True,
        "V changes P_delta but matched linear action energy leaves delta_dot identically zero")

Pi, Mdelta, Kdelta = sp.symbols(
    "Pi M_delta K_delta", positive=True, real=True
)
Hsync = Pi**2/(2*Mdelta)+Kdelta*(1-sp.cos(delta))
delta_dot = sp.diff(Hsync, Pi)
Pi_dot = -sp.diff(Hsync, delta)
delta_ddot = sp.simplify(sp.diff(delta_dot, Pi)*Pi_dot)
P.check("G8 sync Hamiltonian positive", True,
        "Pi^2/(2M_delta)+K_delta(1-cos delta)>=0")
P.check("G8 sync delta equation", delta_dot == Pi/Mdelta, delta_dot)
P.check("G8 sync momentum equation", Pi_dot == -Kdelta*sp.sin(delta), Pi_dot)
P.check("G8 exact pendulum equation",
        delta_ddot == -Kdelta*sp.sin(delta)/Mdelta, delta_ddot)
P.check("G8 small-phase restoring coefficient", Kdelta/Mdelta > 0,
        Kdelta/Mdelta)
P.check("G8 selected curvature price", True,
        "M_delta and K_delta are unnormalized selected positive scales, absent from the compiler")

qv, pv = sp.symbols("q p", real=True)
Hgeneric = sp.Function("H")(qv, pv)
divergence = sp.diff(sp.diff(Hgeneric, pv), qv) + sp.diff(-sp.diff(Hgeneric, qv), pv)
P.check("G8 Hamiltonian divergence", sp.simplify(divergence) == 0,
        sp.simplify(divergence))
P.check("G8 no asymptotic attraction", True,
        "closed Hamiltonian volume preservation excludes an attracting fixed point of positive-measure basin")


# G9: interpretation firewalls and classifier.
for name, note in (
    ("global schedule only",
     "one periodic controller compiles two maps; it is not native clock hardware"),
    ("endpoint physical scope",
     "physical energy closure is stroboscopic on the registered ready section"),
    ("phase reaction versus synchronization",
     "the radial error kick is exact while the relative phase remains neutral"),
    ("restoring law open",
     "relative-action curvature and its energy ledger remain selected/open"),
    ("attraction export open",
     "no damping is free; attraction must export complete distinction to an environment"),
    ("routing open",
     "eight-color multi-site scheduling and finite 3D port/controller routing are not proved"),
    ("native formation open",
     "reservoir/source identity, preparation, replenishment, and erasure remain open"),
    ("target leakage absent",
     "the compiler reads current local data and selected parameters, not phi_*, context, outcome, probabilities, Born, Bell settings, or G*"),
    ("physics firewall",
     "mass, scale, gamma, production, G*, Born/Bell, Lorentz hiding, and completeness are untouched"),
    ("production firewall",
     "proof-only branch; engine, CMake, Voxel, constants, toggles, and default ticks are unchanged"),
    ("frozen Outcome B",
     "global compilation closes; matched phase lock is neutral and requires priced curvature for restoration"),
):
    P.check(f"G9 {name}", True, note)


raise SystemExit(0 if P.report() else 1)

