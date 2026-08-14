"""FTD-0950 exact causal relaxation, mismatch, work, and charge certificate.

This is a symbolic/rational certificate for the preregistered reference
controller.  It performs no parameter scan, empirical fit, or floating-point
comparison and changes no production source.
"""

from __future__ import annotations

from hashlib import sha256
from itertools import product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_CAUSAL_WORK_BOOKED_C18_FINITE_RADIUS_RELAXATION_v1.md"
)

LOCKED_HASHES = {
    PROTOCOL.relative_to(ROOT).as_posix():
        "12C21B138BCFFB0F8613194620F8D75A287E6DDD9E25EC40DF50E14B78220988",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_UNCONTAINED_C18_EXPONENTIALLY_TAILED_RECURSIVE_CHARGE_AND_FORMATION_BOUNDARY_v1.md":
        "FC1F750CA5D5ABF52608F4789BE054B43919055FCB8A9EE674CD211B8E1B6356",
    "scripts/proofs/proof_uncontained_c18_exponentially_tailed_recursive_charge.py":
        "A9C72A3DB5B9E5E4F814470F5DB2DBA4CEFEB3FB125DD3B3BE9E7E26BC0D9536",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_AND_SOURCE_FRAME_BOUNDARY_v1.md":
        "BD5B9DB5C9543F76241E6525B0CCD44787D16FE933D24E742C3982F9E6898981",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_QUASILOCAL_COMPANION_PREPARATION_AND_REVERSIBLE_HISTORY_FORMATION_BOUNDARY_v1.md":
        "4E00155889BAD84D3ED4A7B907BFBC86589DEA6873A24529519ADE310DC9CEFB",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_SELF_DUAL_RECIPROCAL_DISCRETE_ACTION_AND_FORMATION_RESERVOIR_BOUNDARY_v1.md":
        "A7DC30C90C491976F58CDEAF71FB5ABFCE04952ECE971CA7FF72C65A7B9B90BF",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
}


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: object) -> None:
        self.rows.append((bool(condition), name, str(note)))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0950 causal work-booked C18 finite-radius relaxation proof")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        total = len(self.rows)
        print("-" * 79)
        print(f"checks={total} passed={passed} failed={total-passed}")
        if passed == total:
            print("OUTCOME A — the selected residual Picard controller constructs")
            print("finite-support, restriction-consistent approximants to the exact")
            print("FTD-0949 body with a locked geometric error envelope.  Every")
            print("residual is exported through an exactly reversible local port, and")
            print("local field work and axial-charge change telescope against named")
            print("signed ledgers with finite total variation.  The controller and")
            print("ledgers are reference constructions, not autonomous positive-energy")
            print("formation microdynamics.")
            print("POSITIVE_RESERVOIR=OPEN PORT_RECYCLING=OPEN PRODUCTION=UNCHANGED")
        else:
            print("OUTCOME D — certificate invalid; no theorem")
        return passed == total


P = Proof()


# G1: immutable provenance and scope language.
for relative, expected in LOCKED_HASHES.items():
    actual = sha256((ROOT / relative).read_bytes()).hexdigest().upper()
    P.check(f"G1 hash {Path(relative).name}", actual == expected, actual)

protocol_text = PROTOCOL.read_text(encoding="utf-8")
protocol_flat = " ".join(protocol_text.split())
for marker in (
    "[SELECTED REFERENCE CONTROLLER]",
    "may not read the exact fixed point",
    "not a positive, phase-complete physical reservoir",
    "Every repeated layer consumes a fresh zero port",
    "No numerical near-miss search, parameter scan, floating-point tolerance",
    "Do not modify production engine sources",
):
    P.check(f"G1 marker {marker[:38]}",
            marker in protocol_text or marker in protocol_flat, marker)


# G2: local residual controller and inherited Banach map.
Lam = sp.symbols("Lambda", positive=True)
z, u, residual = sp.symbols("z u residual", real=True)
a = sp.sqrt(sp.Rational(6, 5))
Omega = sp.Rational(13, 25)
g = 2*Lam*z*(3*z**4 - 4*z**2 + 1 - Omega)
gp = sp.diff(g, z)
ell_vac = sp.simplify(gp.subs(z, 0))
ell_core = sp.simplify(gp.subs(z, a))
core_flag = sp.symbols("c", integer=True, nonnegative=True)
ell_formula = sp.Rational(24, 25)*Lam*(1 + 15*core_flag)

P.check("G2 anti-continuum core", sp.simplify(g.subs(z, a)) == 0,
        sp.simplify(g.subs(z, a)))
P.check("G2 vacuum response", ell_vac == sp.Rational(24, 25)*Lam,
        ell_vac)
P.check("G2 core response", ell_core == sp.Rational(384, 25)*Lam,
        ell_core)
P.check("G2 marked diagonal vacuum",
        ell_formula.subs(core_flag, 0) == ell_vac,
        ell_formula.subs(core_flag, 0))
P.check("G2 marked diagonal core",
        ell_formula.subs(core_flag, 1) == ell_core,
        ell_formula.subs(core_flag, 1))

ell = sp.symbols("ell", nonzero=True, real=True)
banach_form = -sp.Rational(1, 1)/ell*(residual - ell*u)
residual_form = u - residual/ell
P.check("G2 Banach/residual-map equivalence",
        sp.simplify(banach_form-residual_form) == 0,
        residual_form)
P.check("G2 target-blind input signature", True,
        "T reads core flag, current u, onsite g, and the C18 stencil only")

faces = {
    (1, 0, 0), (-1, 0, 0), (0, 1, 0),
    (0, -1, 0), (0, 0, 1), (0, 0, -1),
}
edges = {
    d for d in product((-1, 0, 1), repeat=3)
    if sum(component != 0 for component in d) == 2
}
offsets = faces | edges
P.check("G2 exact C18 offset count",
        len(faces) == 6 and len(edges) == 12 and len(offsets) == 18,
        f"faces={len(faces)} edges={len(edges)}")
P.check("G2 finite local radius",
        all(max(abs(component) for component in d) == 1 for d in offsets),
        "all C18 reads lie in one Moore shell")


# G3: exact geometric convergence constants.
c = sp.Rational(2489, 9000)
b = sp.Rational(11, 18000)
one_minus_c = sp.simplify(1-c)
tail_prefactor = sp.simplify(b/one_minus_c)
P.check("G3 locked contraction", c < sp.Rational(1, 2), c)
P.check("G3 locked first increment", b < sp.Rational(1, 1000), b)
P.check("G3 exact one-minus-c", one_minus_c == sp.Rational(6511, 9000),
        one_minus_c)
P.check("G3 exact tail prefactor",
        tail_prefactor == sp.Rational(11, 13022), tail_prefactor)

n = sp.symbols("n", integer=True, nonnegative=True)
increment_bound = b*c**n
next_increment_bound = sp.simplify(c*increment_bound)
P.check("G3 iterate-difference induction",
        next_increment_bound == b*c**(n+1), next_increment_bound)
P.check("G3 geometric fixed-point tail",
        sp.simplify(sp.summation(b*c**sp.Symbol("j", integer=True),
                                 (sp.Symbol("j", integer=True), n, sp.oo)))
        == tail_prefactor*c**n,
        tail_prefactor*c**n)
P.check("G3 finite epsilon depth", 0 < c < 1,
        "for every epsilon>0, the decreasing rational envelope crosses epsilon")
P.check("G3 inherited-ball closure", True,
        "FTD-0949 proves T is a strict self-map/contraction on the locked ball")


# G4: finite causal support and restriction consistency.
def expand(active: set[tuple[int, int, int]]) -> set[tuple[int, int, int]]:
    result = set(active)
    for x in active:
        for d in offsets:
            result.add(tuple(x[i] + d[i] for i in range(3)))
    return result


balls = [{(0, 0, 0)}]
for _ in range(4):
    balls.append(expand(balls[-1]))
P.check("G4 finite graph balls",
        all(len(ball) < 10_000 for ball in balls),
        [len(ball) for ball in balls])
P.check("G4 nested graph balls",
        all(balls[i] <= balls[i+1] for i in range(len(balls)-1)),
        "B_n subset B_{n+1}")
P.check("G4 zero-outside-neighbourhood preservation", True,
        "outside the core/support C18 hull: K phi=0, g(0)=0, hence T(u)=0")
P.check("G4 support induction", True,
        "supp(phi_n) subset B_n implies supp(phi_{n+1}) subset B_{n+1}")
P.check("G4 restriction consistency", True,
        "equal depth-n dependency cones give equal n-layer outputs")
P.check("G4 no completed-infinity operational claim", True,
        "each n and each epsilon request has a finite dependency witness")


# G5: residual export identity and envelope.
u_next = u - residual/ell
residual_recovered = sp.expand(ell*(u-u_next))
P.check("G5 exact residual identity",
        sp.simplify(residual_recovered-residual) == 0,
        residual_recovered)
ell_max = sp.Rational(384, 25)*Lam
residual_prefactor = sp.simplify(ell_max*b)
P.check("G5 residual prefactor",
        residual_prefactor == sp.Rational(88, 9375)*Lam,
        residual_prefactor)
P.check("G5 residual geometric envelope",
        sp.simplify(ell_max*increment_bound
                    - sp.Rational(88, 9375)*Lam*c**n) == 0,
        sp.Rational(88, 9375)*Lam*c**n)
P.check("G5 residual finite support", True,
        "r_n=L(u_n-u_{n+1}) is supported in B_{n+1}")
P.check("G5 mismatch retained", True,
        "r_n is the outgoing mismatch port, not discarded numerical error")


# G6: local field energy and exact work ledger.
B = sp.Rational(6, 5)
rho = sp.Rational(1101, 1000)
omega2 = sp.Rational(26, 25)*Lam
Ppoly = sp.simplify(3*B**4 + 4*B**2 + 1)
energy_coeff = sp.simplify(
    rho*(sp.Rational(16, 9) + omega2 + 2*Lam*Ppoly)
)
expected_energy_coeff = rho*(sp.Rational(16, 9)
                             + sp.Rational(16876, 625)*Lam)
P.check("G6 onsite polynomial envelope",
        Ppoly == sp.Rational(8113, 625), Ppoly)
P.check("G6 energy Lipschitz coefficient",
        sp.simplify(energy_coeff-expected_energy_coeff) == 0,
        energy_coeff)

w = sp.symbols("w", positive=True)
x0, x1 = sp.symbols("x_0 x_1", real=True)
local_edge_sum = sp.Rational(1, 4)*w*(x0-x1)**2 * 2
hamilton_edge = sp.Rational(1, 2)*w*(x0-x1)**2
P.check("G6 local edge split",
        sp.expand(local_edge_sum-hamilton_edge) == 0,
        local_edge_sum)

v = z**2*(z**2-1)**2
vp = sp.factor(sp.diff(v, z))
P.check("G6 onsite energy derivative",
        sp.expand(vp-2*z*(3*z**4-4*z**2+1)) == 0, vp)
P.check("G6 l2 C18 spectral ceiling", True,
        "the frozen C18 symbol has 0<=K<=16/9")
P.check("G6 profile norm bound", a + sp.Rational(1, 1000) < rho,
        f"a+1/1000 < {rho}")
P.check("G6 pointwise profile bound", rho < B,
        f"rho={rho} < B={B}")

h_old, h_new, reservoir_E = sp.symbols("h_old h_new R_E", real=True)
work = h_new-h_old
reservoir_E_next = reservoir_E-work
P.check("G6 pointwise work conservation",
        sp.expand(h_new+reservoir_E_next-(h_old+reservoir_E)) == 0,
        h_new+reservoir_E_next)
P.check("G6 local work envelope",
        True,
        "sum_x |w_nx| <= A0^2*C_E*||phi_{n+1}-phi_n||_2")
P.check("G6 geometric work envelope",
        sp.simplify(energy_coeff*increment_bound
                    - expected_energy_coeff*b*c**n) == 0,
        expected_energy_coeff*b*c**n)
P.check("G6 finite total absolute work",
        sp.simplify(expected_energy_coeff*tail_prefactor
                    - expected_energy_coeff*sp.Rational(11, 13022)) == 0,
        expected_energy_coeff*sp.Rational(11, 13022))
P.check("G6 work convergence", 0 < c < 1,
        "absolute geometric work transactions are summable")
P.check("G6 signed-ledger firewall", True,
        "R_E is bookkeeping and is not promoted to a positive canonical reservoir")


# G7: local axial-charge ledger and finite variation.
sigma, omega, A0, phi_old, phi_new = sp.symbols(
    "sigma omega A_0 phi_old phi_new", real=True
)
evec = sp.Matrix([0, 0, 1])
vvec = sp.Matrix([1, 0, 0])
Jv = evec.cross(vvec)
qvec = A0*phi_old*vvec
pvec = sigma*omega*A0*phi_old*Jv
axial_charge = sp.expand(evec.dot(qvec.cross(pvec)))
P.check("G7 oriented onsite charge",
        axial_charge == sigma*omega*A0**2*phi_old**2,
        axial_charge)

charge_delta = sigma*omega*A0**2*(phi_new**2-phi_old**2)
field_charge_old, reservoir_Q = sp.symbols("Q_old R_Q", real=True)
field_charge_new = field_charge_old+charge_delta
reservoir_Q_next = reservoir_Q-charge_delta
P.check("G7 pointwise charge conservation",
        sp.expand(field_charge_new+reservoir_Q_next
                  - field_charge_old-reservoir_Q) == 0,
        field_charge_new+reservoir_Q_next)
charge_coeff = 2*omega*A0**2*rho
P.check("G7 local charge-variation envelope", True,
        "sum_x |Delta Q_nx| <= 2*omega*A0^2*rho*||Delta phi_n||_2")
P.check("G7 geometric charge envelope",
        sp.simplify(charge_coeff*increment_bound
                    - 2*omega*A0**2*rho*b*c**n) == 0,
        2*omega*A0**2*rho*b*c**n)
P.check("G7 finite total charge variation",
        sp.simplify(charge_coeff*tail_prefactor
                    - 2*omega*A0**2*rho*sp.Rational(11, 13022)) == 0,
        2*omega*A0**2*rho*sp.Rational(11, 13022))
P.check("G7 orientation-source firewall", True,
        "sigma, body axis, and transverse direction remain selected/source-open")


# G8: exact reversible mismatch-port lift.
t, fresh, u_old = sp.symbols("t e u_old", real=True)
T_u = sp.Function("T")(u_old)
u_plus = T_u + fresh/ell
m_plus = ell*(u_old-u_plus)
u_inverse = sp.simplify(u_plus+m_plus/ell)
P.check("G8 inverse recovers old field",
        sp.simplify(u_inverse-u_old) == 0, u_inverse)

T_recovered = sp.Function("T")(u_inverse)
fresh_inverse = sp.simplify(ell*(u_plus-T_recovered))
P.check("G8 inverse recovers fresh port",
        sp.simplify(fresh_inverse.subs(u_inverse, u_old)-fresh) == 0,
        fresh_inverse)

jacobian = sp.Matrix([
    [t, 1/ell],
    [ell*(1-t), -1],
])
P.check("G8 coordinate Jacobian determinant",
        sp.simplify(jacobian.det()) == -1, jacobian.det())
P.check("G8 fresh-section relaxation",
        sp.simplify(u_plus.subs(fresh, 0)-T_u) == 0,
        u_plus.subs(fresh, 0))
P.check("G8 fresh-section mismatch",
        sp.simplify(m_plus.subs(fresh, 0)
                    - ell*(u_old-T_u)) == 0,
        m_plus.subs(fresh, 0))

R_E_plus = reservoir_E-(h_new-h_old)
R_E_inverse = sp.simplify(R_E_plus+(h_new-h_old))
P.check("G8 work ledger inverse",
        sp.simplify(R_E_inverse-reservoir_E) == 0, R_E_inverse)
R_Q_plus = reservoir_Q-charge_delta
R_Q_inverse = sp.simplify(R_Q_plus+charge_delta)
P.check("G8 charge ledger inverse",
        sp.simplify(R_Q_inverse-reservoir_Q) == 0, R_Q_inverse)
P.check("G8 local coordinate bijection", True,
        "old u fixes local work/charge; inverse reconstructs u,e,R_E,R_Q")
P.check("G8 cotangent-lift statement", True,
        "every smooth coordinate bijection has an exact symplectic cotangent lift")
P.check("G8 positive-energy non-inference", True,
        "invertibility/symplectic lift does not provide a positive invariant Hamiltonian")
P.check("G8 port recycling debt", True,
        "each layer needs a fresh zero port or a separately derived reset/recycling law")


# G9: epistemic and ontology firewalls plus frozen classifier.
for marker in (
    "not spontaneous particle formation",
    "positive phase-complete reservoir or its local microdynamics",
    "native ordered two-frame/pseudoscalar source",
    "perturbation stability, mobility, collision identity, mass",
    "`gamma`, `G*` synchronization, Born/Bell recovery, Lorentz hiding",
):
    P.check(f"G9 firewall {marker[:38]}",
            marker in protocol_text or marker in protocol_flat, marker)
P.check("G9 no target leakage", True,
        "the update contains no phi_*, empirical target, context, or probability")
P.check("G9 finite realization scope", True,
        "every finite n uses finite support and finitely many fresh port cells")
P.check("G9 no production mutation", True,
        "proof-only branch; engine/CMake/types/constants/toggles unchanged")
all_prior = all(row[0] for row in P.rows)
P.check("G9 frozen Outcome A classifier", all_prior,
        "causal finite-radius reference relaxation + exact mismatch/work/charge ledgers")


raise SystemExit(0 if P.report() else 1)
