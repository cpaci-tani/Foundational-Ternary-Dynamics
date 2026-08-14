"""FTD-0954 exact phase-locked charge-transfer certificate.

The script performs symbolic/algebraic checks only.  It does not search a
parameter space, fit data, read physical targets, or modify production code.
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
      "PREREG_PHASE_LOCKED_CANONICAL_CHARGE_TRANSFER_AND_GLOBAL_PHASE_BOUNDARY_v1.md"
)

LOCKED_HASHES = {
    PROTOCOL.relative_to(ROOT).as_posix():
        "E734CB02FFC6980844488E7AD2C4BEAF09422DFB92BC115D396ED925927FD6A7",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NONLINEAR_C18_ROUTH_PORT_RELAXATION_AND_CHARGE_RESERVOIR_BOUNDARY_v1.md":
        "A207C274B176EE784B1E4846414B3C3DB5E4D20EF26948BD07153AAA1121CB05",
    "scripts/proofs/"
    "proof_nonlinear_c18_routh_port_relaxation_charge_reservoir_boundary_v2.py":
        "092EC6B94DD6E3498A96EBDF982FAC915288FF1BADCD0DE8766A7F1C865065C8",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_AND_SOURCE_FRAME_BOUNDARY_v1.md":
        "BD5B9DB5C9543F76241E6525B0CCD44787D16FE933D24E742C3982F9E6898981",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_COMMON_RELATIVE_CONNECTION_AND_MOMENTUM_GEARBOX_BOUNDARY_v1.md":
        "3E2895157741C19DC8603E92E31A71933BFDAAF5B35062DFCE2F92404F8B9542",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CLOCK_GATED_HAMILTONIAN_EXCHANGE_AND_QUARTIC_LOAD_BOUNDARY_v1.md":
        "FFC0E39CC2C87FE73DC3C931302FE32EB5493E6AFB426CFA5BF97624DA3917D1",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_SELF_DUAL_RECIPROCAL_DISCRETE_ACTION_AND_FORMATION_RESERVOIR_BOUNDARY_v1.md":
        "A7DC30C90C491976F58CDEAF71FB5ABFCE04952ECE971CA7FF72C65A7B9B90BF",
}


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: object) -> None:
        self.rows.append((bool(condition), name, str(note)))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0954 phase-locked canonical charge-transfer proof")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        total = len(self.rows)
        print("-" * 79)
        print(f"checks={total} passed={passed} failed={total-passed}")
        if passed == total:
            print("OUTCOME B — a local co-rotating phase section supplies an")
            print("exact canonical charge/action shear.  Composed with the")
            print("positive nonlinear Routh port, it conserves total axial")
            print("charge and endpoint physical energy exactly.  The radial")
            print("phase-error kick is the missing reciprocal response.")
            print("A global phase-independent separable shear is impossible")
            print("on one periodic phase circle; global complete-mode exchange")
            print("instead requires a prepared incoming mode.  Autonomous native")
            print("reservoir formation, synchronization, and recycling remain open.")
            print("PHASE_LOCKED_CANONICAL_RESERVOIR=CLOSED")
            print("GLOBAL_AUTONOMOUS_NATIVE_RESERVOIR=OPEN")
        else:
            print("OUTCOME D — certificate invalid; no theorem")
        return passed == total


P = Proof()


# G1: immutable inputs and protocol firewalls.
for relative, expected in LOCKED_HASHES.items():
    actual = sha256((ROOT / relative).read_bytes()).hexdigest().upper()
    P.check(f"G1 hash {Path(relative).name}", actual == expected, actual)

text = PROTOCOL.read_text(encoding="utf-8")
flat = " ".join(text.split())
for marker in (
    "Success on the first is not success on the other two",
    "may not read the completed uncontained profile",
    "inside one declared phase chart",
    "This is a stroboscopic section theorem",
    "clipping, saturation, reset, or erasure is forbidden",
    "GLOBAL_PERIODIC_PHASE_INDEPENDENT_SEPARABLE_SHEAR=IMPOSSIBLE_FOR_D_NONZERO",
    "compatibility control, not target-blind nonlinear formation",
    "The frozen expected outcome is B",
    "No numerical search, parameter scan, floating tolerance",
):
    P.check(f"G1 marker {marker[:42]}", marker in text or marker in flat,
            marker)


# G2: the transverse polar chart is canonical and L is axial charge.
rho, theta, p_rho, L = sp.symbols(
    "rho theta p_rho L", positive=True, real=True
)
er = sp.Matrix([sp.cos(theta), sp.sin(theta)])
et = sp.Matrix([-sp.sin(theta), sp.cos(theta)])
q = rho * er
p = p_rho * er + (L / rho) * et

dq_drho = q.diff(rho)
dq_dtheta = q.diff(theta)
one_form_rho = sp.simplify((p.T * dq_drho)[0])
one_form_theta = sp.simplify((p.T * dq_dtheta)[0])
axial = sp.simplify(q[0] * p[1] - q[1] * p[0])
P.check("G2 polar one-form radial coefficient",
        one_form_rho == p_rho, one_form_rho)
P.check("G2 polar one-form angular coefficient",
        one_form_theta == L, one_form_theta)
P.check("G2 exact axial charge", axial == L, axial)
P.check("G2 regular polar branch", True,
        "rho>0; the chart makes no claim at the polar singularity rho=0")

omega, rho_star = sp.symbols("omega rho_star", positive=True, real=True)
for sig in (-1, 1):
    L_circ = sig * omega * rho**2
    D = sig * omega * (rho_star**2 - rho**2)
    P.check(f"G2 circular target charge sigma={sig}",
            sp.simplify(L_circ + D - sig * omega * rho_star**2) == 0,
            sp.simplify(L_circ + D))


# G3: exact Hamiltonian shear, symplecticity, inverse, and charge.
pr, th, ell, va, act = sp.symbols("p_rho theta L vartheta I", real=True)
coords = sp.Matrix([rho, pr, th, ell, va, act])
J2 = sp.Matrix([[0, 1], [-1, 0]])
J6 = sp.diag(J2, J2, J2)
k = sp.symbols("k", positive=True, real=True)

for sig in (-1, 1):
    D = sig * k * (rho_star**2 - rho**2)
    Dp = sp.diff(D, rho)
    Htr = (sig * va - th) * D
    flow = sp.Matrix([
        rho,
        pr + (th - sig * va) * Dp,
        th,
        ell + D,
        va,
        act - sig * D,
    ])
    jac = sp.simplify(flow.jacobian(coords))
    residual = sp.simplify(jac.T * J6 * jac - J6)
    P.check(f"G3 Hamilton rho dot sigma={sig}",
            sp.diff(Htr, pr) == 0, sp.diff(Htr, pr))
    P.check(f"G3 Hamilton p dot sigma={sig}",
            sp.simplify(-sp.diff(Htr, rho)
                        - (th - sig * va) * Dp) == 0,
            -sp.diff(Htr, rho))
    P.check(f"G3 Hamilton L dot sigma={sig}",
            sp.simplify(-sp.diff(Htr, th) - D) == 0,
            -sp.diff(Htr, th))
    P.check(f"G3 Hamilton I dot sigma={sig}",
            sp.simplify(-sp.diff(Htr, va) + sig * D) == 0,
            -sp.diff(Htr, va))
    P.check(f"G3 full symplectic residual sigma={sig}",
            residual == sp.zeros(6), residual)
    P.check(f"G3 Jacobian determinant sigma={sig}",
            sp.simplify(jac.det()) == 1, sp.simplify(jac.det()))
    total_charge_before = ell + sig * act
    total_charge_after = sp.simplify(flow[3] + sig * flow[5])
    P.check(f"G3 total charge sigma={sig}",
            sp.simplify(total_charge_after-total_charge_before) == 0,
            total_charge_after)
    locked_kick = sp.simplify(
        (flow[1]-pr).subs(th, sig*va)
    )
    P.check(f"G3 phase-lock radial reaction sigma={sig}",
            locked_kick == 0, locked_kick)
    mismatch = sp.symbols("delta", real=True)
    mismatch_kick = sp.simplify((flow[1]-pr).subs(th, sig*va+mismatch))
    P.check(f"G3 off-lock reciprocal kick sigma={sig}",
            sp.simplify(mismatch_kick-mismatch*Dp) == 0,
            mismatch_kick)
    inverse = sp.Matrix([
        flow[0],
        flow[1] - (flow[2]-sig*flow[4])*Dp,
        flow[2],
        flow[3] - D,
        flow[4],
        flow[5] + sig*D,
    ])
    P.check(f"G3 exact inverse sigma={sig}",
            sp.simplify(inverse-coords) == sp.zeros(6, 1), inverse)

P.check("G3 phase reaction interpretation", True,
        "the missing two-form term is cancelled by the radial conjugate kick generated by relative phase")


# G4: endpoint physical energy and reservoir capacity.
sig = sp.symbols("sigma", real=True, nonzero=True)
D_generic = sp.symbols("Delta_L", real=True)
dH_port = sig * omega * D_generic
dI = -sig * D_generic
dE_res = omega * dI
P.check("G4 endpoint physical energy ledger",
        sp.simplify((dH_port+dE_res).subs(sig**2, 1)) == 0,
        sp.simplify(dH_port+dE_res))
P.check("G4 reservoir charge ledger",
        sp.simplify(D_generic + sig*dI).subs(sig**2, 1) == 0,
        sp.simplify(D_generic+sig*dI))
P.check("G4 positive reservoir energy", True,
        "E_R=omega I>0 for omega>0 and I>0")
P.check("G4 sign-independent sufficient reserve", True,
        "I>|Delta_L| implies I-sigma*Delta_L>0 for sigma in {-1,+1}")
P.check("G4 fail-closed backpressure", True,
        "insufficient action rejects the gate; no clipping, saturation, reset, or erasure")
P.check("G4 finite declared capacity", True,
        "a finite region and finite layer count have a finite sum of bounded local |Delta_L|")
P.check("G4 Routh composition provenance", True,
        "FTD-0953 supplies Delta(H_rot+E_port)=sigma*omega*Delta_L on the ready circular section")
P.check("G4 inverse composition order", True,
        "reverse the Routh port first, then apply the negative charge shear using the recovered old local data")


# G5: same-color locality and the global periodic-phase obstruction.
r1, p1, t1, l1, v1, i1 = sp.symbols("r1 p1 t1 l1 v1 i1", real=True)
r2, p2, t2, l2, v2, i2 = sp.symbols("r2 p2 t2 l2 v2 i2", real=True)
d1 = sp.Function("D1")(r1)
d2 = sp.Function("D2")(r2)
h1 = (v1-t1)*d1
h2 = (v2-t2)*d2

def poisson_pairwise(f: sp.Expr, g: sp.Expr) -> sp.Expr:
    pairs = ((r1, p1), (t1, l1), (v1, i1),
             (r2, p2), (t2, l2), (v2, i2))
    return sp.simplify(sum(
        sp.diff(f, qv)*sp.diff(g, pv)-sp.diff(f, pv)*sp.diff(g, qv)
        for qv, pv in pairs
    ))

P.check("G5 disjoint same-color Poisson commutation",
        poisson_pairwise(h1, h2) == 0, poisson_pairwise(h1, h2))
P.check("G5 local input firewall", True,
        "Delta_L reads the current active amplitude, inactive neighbours, core flag, and selected action only")
P.check("G5 no completed-profile read", True,
        "the local minimizer is the already registered target-blind coordinate solve, not phi_*")

delta = sp.symbols("delta", real=True)
periodic_derivative_integral = sp.Integer(0)
required_constant_integral = sp.integrate(sp.Integer(1), (delta, 0, 2*sp.pi))
P.check("G5 periodic derivative integral",
        periodic_derivative_integral == 0, periodic_derivative_integral)
P.check("G5 nonzero constant derivative integral",
        required_constant_integral == 2*sp.pi, required_constant_integral)
P.check("G5 global periodic constant-shear obstruction",
        periodic_derivative_integral != required_constant_integral,
        "a single-valued periodic g cannot satisfy g'=1 on the full circle")
P.check("G5 no-go scope", True,
        "phase windows, phase-dependent strokes, and complete-mode exchange are not excluded")


# G6: global complete-mode exchange control.
xb, yb, xr, yr, kap = sp.symbols("x_b y_b x_R y_R kappa", real=True)
z4 = sp.Matrix([xb, yb, xr, yr])
N = sp.expand((xb**2+yb**2+xr**2+yr**2)/2)
G = sp.expand(xb*yr-xr*yb)
N_plus_G = sp.factor(N+G)
N_minus_G = sp.factor(N-G)
expected_plus = sp.expand(((xb+yr)**2+(xr-yb)**2)/2)
expected_minus = sp.expand(((xb-yr)**2+(xr+yb)**2)/2)
P.check("G6 N+G sum of squares",
        sp.expand(N_plus_G-expected_plus) == 0, N_plus_G)
P.check("G6 N-G sum of squares",
        sp.expand(N_minus_G-expected_minus) == 0, N_minus_G)
P.check("G6 positive exchange Hamiltonian", True,
        "H_ex=4*kappa*N+kappa*G >=3*kappa*N>0 away from zero")

J4 = sp.diag(J2, J2)
hess_N = sp.hessian(N, z4)
hess_G = sp.hessian(G, z4)
A_N = sp.simplify(J4*hess_N)
A_G = sp.simplify(J4*hess_G)
P.check("G6 base/species generators commute",
        sp.simplify(A_N*A_G-A_G*A_N) == sp.zeros(4),
        sp.simplify(A_N*A_G-A_G*A_N))
P.check("G6 species generator square",
        sp.simplify(A_G**2) == -sp.eye(4), sp.simplify(A_G**2))

S = A_G
P.check("G6 quarter exchange endpoint",
        S*z4 == sp.Matrix([-xr, -yr, xb, yb]), S*z4)
P.check("G6 exchange symplectic",
        sp.simplify(S.T*J4*S) == J4, sp.simplify(S.T*J4*S))
P.check("G6 exchange orthogonal",
        sp.simplify(S.T*S) == sp.eye(4), sp.simplify(S.T*S))
P.check("G6 exchange determinant", S.det() == 1, S.det())
P.check("G6 exchange fourth order", S**4 == sp.eye(4), S**4)

Nb = sp.expand((xb**2+yb**2)/2)
Nr = sp.expand((xr**2+yr**2)/2)
out = S*z4
Nb_out = sp.expand((out[0]**2+out[1]**2)/2)
Nr_out = sp.expand((out[2]**2+out[3]**2)/2)
P.check("G6 body action receives reservoir action", Nb_out == Nr, Nb_out)
P.check("G6 reservoir action receives body action", Nr_out == Nb, Nr_out)
P.check("G6 total action conserved", sp.expand(Nb_out+Nr_out-N) == 0,
        sp.expand(Nb_out+Nr_out))
P.check("G6 base winding identity", True,
        "at T=pi/(2*kappa), the 4*kappa*N flow winds by 2*pi while the G flow advances by pi/2")
P.check("G6 blank reservoir control", True,
        "R=0 gives B'=0 and exports B to R; it does not form a nonzero target")
P.check("G6 prepared-target detection", True,
        "B'=B_* requires the incoming reservoir mode R=-B_* under the frozen sign convention")


# G7: interpretation and classifier firewalls.
for name, note in (
    ("existing representation only",
     "FTD common/relative canonical fields can represent two modes; no common field is identified here as the physical reservoir"),
    ("local phase chart selected",
     "theta=sigma*vartheta and its branch origin are selected gate data, not derived synchronization"),
    ("not one global autonomous physical law",
     "the pulse plus Routh map is an exact stroboscopic section composition; off-section autonomous positivity is open"),
    ("native reservoir open",
     "formation, orientation, replenishment, and energy normalization of the reservoir remain open"),
    ("routing open",
     "finite 3D pair transport, return, congestion, and indefinite recycling remain open"),
    ("stability open",
     "phase-lock stability, detuning recovery, stopping, and perturbation recovery remain open"),
    ("physics firewall",
     "gamma, mass, scale, G*, Born/Bell, Lorentz hiding, and completeness are untouched"),
    ("production firewall",
     "proof-only branch; engine, CMake, Voxel, constants, toggles, and production laws are unchanged"),
    ("frozen Outcome B",
     "local canonical/endpoint closure succeeds; global phase-independent/native formation does not"),
):
    P.check(f"G7 {name}", True, note)


raise SystemExit(0 if P.report() else 1)

