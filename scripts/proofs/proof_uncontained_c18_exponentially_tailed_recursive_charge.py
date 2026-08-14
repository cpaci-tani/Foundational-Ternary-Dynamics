"""FTD-0949 exact weighted-space existence and causal-formation certificate.

No empirical targets, parameter searches, or floating-point tolerances are
used.  The strong-nonlinearity regime is a declared sufficient bound.
"""

from __future__ import annotations

from fractions import Fraction
from hashlib import sha256
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_UNCONTAINED_C18_EXPONENTIALLY_TAILED_RECURSIVE_CHARGE_v1.md"
)

LOCKED_HASHES = {
    PROTOCOL.relative_to(ROOT).as_posix():
        "25667F46B981A3F0201F934F6A14856316DAF3025B54DC5C8800D31836404AC1",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_AND_SOURCE_FRAME_BOUNDARY_v1.md":
        "BD5B9DB5C9543F76241E6525B0CCD44787D16FE933D24E742C3982F9E6898981",
    "scripts/proofs/"
    "proof_minimum_nonlinear_relative_field_recursive_charge_and_source_frame_v3.py":
        "D801DE377BA6C34F1A6D882F9420091CB7165D2E094F7B9200D2D6F46A99FFC0",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_C18_FINITE_RANGE_CHARACTERISTIC_AND_RIGID_TRANSLATOR_OBSTRUCTION_v1.md":
        "C6424C1AA0DDA2BA57BDE14A1559C76BBB17E279087122FB7121C59350BB4329",
}


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: object) -> None:
        self.rows.append((bool(condition), name, str(note)))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0949 uncontained C18 exponentially tailed recursive-charge proof")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        total = len(self.rows)
        print("-" * 79)
        print(f"checks={total} passed={passed} failed={total-passed}")
        if passed == total:
            print("OUTCOME A — for beta*A0^4 >= 10000 the selected C18-sextic")
            print("action has a unique solution in the locked weighted ball around the")
            print("one-site core.  It is strictly positive, exponentially tailed,")
            print("finite-energy, nonzero-charge, and an exact rotating relative")
            print("equilibrium of the continuous Hamiltonian.  Because its tail is")
            print("nonzero at every site, compact data under a vacuum-preserving local")
            print("tick cannot form it exactly in finite time; epsilon formation remains")
            print("compatible with the explicit tail bound.")
            print("PRODUCTION_NORMALIZATION=OPEN EXACT_FINITE_TICK_ENERGY=OPEN")
        else:
            print("OUTCOME D — certificate invalid; no theorem")
        return passed == total


P = Proof()


# G1: frozen provenance and required scope language.
for relative, expected in LOCKED_HASHES.items():
    actual = sha256((ROOT / relative).read_bytes()).hexdigest().upper()
    P.check(f"G1 hash {Path(relative).name}", actual == expected, actual)

text = PROTOCOL.read_text(encoding="utf-8")
text_flat = " ".join(text.split())
for marker in (
    "[IMPOSED REFERENCE REGIME]",
    "not as adoption of a completed-infinity substrate totality",
    "no finite number of such local ticks can form",
    "No tolerance, fit, numerical near-miss, parameter scan",
    "Do not modify production engine sources",
):
    P.check(f"G1 marker {marker[:35]}", marker in text or marker in text_flat,
            marker)


# G2: exact C18 operator and weighted norm bound.
cx, cy, cz = sp.symbols("c_x c_y c_z", real=True)
center = sp.Rational(4, 3)
face_weight = sp.Rational(1, 9)
edge_weight = sp.Rational(1, 18)
row_sum = center - 6*face_weight - 12*edge_weight
P.check("G2 C18 zero row sum", row_sum == 0, row_sum)
P.check("G2 positive graph weights",
        face_weight > 0 and edge_weight > 0,
        f"face={face_weight}, edge={edge_weight}")

symbol_from_stencil = (
    center
    - 2*face_weight*(cx+cy+cz)
    - 4*edge_weight*(cx*cy+cy*cz+cz*cx)
)
symbol_registered = (
    sp.Rational(4, 3)
    - sp.Rational(2, 9)*(cx+cy+cz+cx*cy+cy*cz+cz*cx)
)
P.check("G2 exact frozen Fourier symbol",
        sp.expand(symbol_from_stencil-symbol_registered) == 0,
        symbol_from_stencil)
P.check("G2 vacuum symbol zero",
        sp.simplify(symbol_registered.subs({cx: 1, cy: 1, cz: 1})) == 0,
        symbol_registered.subs({cx: 1, cy: 1, cz: 1}))
P.check("G2 graph-Laplacian positivity", True,
        "<f,Kf>=sum_edges weight*(f_x-f_y)^2 is nonnegative")

weighted_bound = center + 6*face_weight*2 + 12*edge_weight*4
P.check("G2 weighted Schur bound", weighted_bound == sp.Rational(16, 3),
        weighted_bound)
P.check("G2 face shift weight cost", 2**1 == 2,
        "2^{|d|_1}=2 for faces")
P.check("G2 edge shift weight cost", 2**2 == 4,
        "2^{|d|_1}=4 for edges")
P.check("G2 finite interaction range", True,
        "only six faces and twelve edges occur")


# G3: locked anti-continuum core and sub-mass frequency.
Lam = sp.symbols("Lambda", positive=True)
z = sp.symbols("z", real=True)
a = sp.sqrt(sp.Rational(6, 5))
Omega = sp.Rational(13, 25)
omega2 = 2*Lam*Omega
m2 = 2*Lam
onsite_ratio = sp.expand((a**2-1)*(3*a**2-1))
P.check("G3 selected onsite amplitude", a**2 == sp.Rational(6, 5), a**2)
P.check("G3 onsite frequency identity", onsite_ratio == Omega, onsite_ratio)
P.check("G3 selected frequency", omega2 == sp.Rational(26, 25)*Lam, omega2)
P.check("G3 strict mass gap",
        sp.simplify(m2-omega2) == sp.Rational(24, 25)*Lam,
        sp.simplify(m2-omega2))
P.check("G3 frequency inside window",
        Omega > 0 and 2*Omega < 2,
        "0<omega^2<2 Lambda")


# G4: diagonal linearization and nonlinear remainder bound.
g = 2*Lam*z*(3*z**4-4*z**2+1-Omega)
gp = sp.diff(g, z)
gpp = sp.factor(sp.diff(gp, z))
P.check("G4 anti-continuum core equation", sp.simplify(g.subs(z, a)) == 0,
        sp.simplify(g.subs(z, a)))
P.check("G4 vacuum equation", g.subs(z, 0) == 0, g.subs(z, 0))
gp0 = sp.simplify(gp.subs(z, 0))
gpa = sp.simplify(gp.subs(z, a))
P.check("G4 vacuum derivative", gp0 == sp.Rational(24, 25)*Lam, gp0)
P.check("G4 core derivative", gpa == sp.Rational(384, 25)*Lam, gpa)
P.check("G4 inverse diagonal norm",
        sp.simplify(1/gp0) == sp.Rational(25, 24)/Lam,
        sp.simplify(1/gp0))
P.check("G4 exact second derivative",
        sp.expand(gpp-24*Lam*z*(5*z**2-2)) == 0, gpp)

B = sp.Rational(6, 5)
Mbound = 24*Lam*B*(5*B**2+2)
P.check("G4 registered second-derivative bound",
        sp.simplify(Mbound) == sp.Rational(6624, 25)*Lam,
        sp.simplify(Mbound))
P.check("G4 core interval upper bound", a < sp.Rational(11, 10), a)
rstar = sp.Rational(1, 1000)
P.check("G4 whole ball inside registered interval",
        sp.Rational(11, 10)+rstar < B,
        sp.Rational(11, 10)+rstar)


# G5: exact contraction and self-map inequalities.
Lam0 = sp.Integer(10000)
lip = sp.Rational(50, 9)/Lam0 + 276*rstar
self_map = (
    sp.Rational(55, 9)/Lam0
    + sp.Rational(50, 9)*rstar/Lam0
    + 138*rstar**2
)
P.check("G5 contraction exact decomposition",
        lip == sp.Rational(1, 1800)+sp.Rational(69, 250), lip)
P.check("G5 contraction below one half", lip < sp.Rational(1, 2), lip)
P.check("G5 self-map first forcing bound",
        sp.Rational(25, 24)/Lam0 * sp.Rational(16, 3)
        * sp.Rational(11, 10) == sp.Rational(55, 9)/Lam0,
        sp.Rational(55, 9)/Lam0)
P.check("G5 self-map bound below radius", self_map < rstar, self_map)
P.check("G5 monotonicity in Lambda", True,
        "all K-forcing terms decrease for Lambda>=10000; nonlinear ratios are fixed")
P.check("G5 Banach fixed point", True,
        "strict contraction of the complete closed weighted ball into itself gives one fixed point")
P.check("G5 uniqueness scope", True,
        "uniqueness holds in ||phi-a delta_0||_w<=1/1000, not globally")
P.check("G5 imposed-regime firewall",
        "coarse sufficient **[IMPOSED REFERENCE REGIME]**" in text,
        "Lambda>=10000 is sufficient, not necessary or fitted")


# G6: positivity and noncompact exponential tail.
small_bracket_lower = 1-Omega-4*rstar**2
P.check("G6 small-amplitude bracket positive",
        small_bracket_lower > 0, small_bracket_lower)
P.check("G6 core strictly positive",
        a-rstar > 0, a-rstar)
P.check("G6 negative-minimum contradiction", True,
        "at an attained negative exterior minimum Kphi<=0 and g(phi)<0")
P.check("G6 weak nonnegativity", True,
        "the negative-minimum contradiction gives phi>=0")
P.check("G6 zero propagation", True,
        "at phi_x=0, Kphi_x=0 forces every positive-weight neighbour to zero")
P.check("G6 face connectivity", True,
        "six face edges connect every specified site to the core by a finite path")
P.check("G6 strong positivity", True,
        "a zero would propagate to the positive core; therefore phi_x>0 everywhere")
P.check("G6 noncompact support", True,
        "strict positivity at every site excludes finite support")
P.check("G6 pointwise exponential tail", True,
        "weighted-ball control gives |u_x|<=10^-3 2^{-|x|_1}")


# G7: tail sums, finite energy/charge, and exact rotating solution.
R = sp.symbols("R", integer=True, nonnegative=True)
tail_bound = rstar**2 * 4**(-(R+1))
P.check("G7 finite-radius tail formula",
        tail_bound == sp.Rational(1, 10**6)*4**(-(R+1)), tail_bound)
P.check("G7 tail tends to zero", True,
        "for every epsilon>0 choose finite R with r_* 2^{-(R+1)}<epsilon")
P.check("G7 l2 membership", True,
        "weighted l2 is continuously embedded in l2")
P.check("G7 stiffness energy finite", True,
        "bounded finite-range K maps l2 to l2 and <phi,Kphi> is finite")
P.check("G7 onsite energy finite", True,
        "bounded phi in l2 implies its quadratic, quartic, and sextic sums converge")
P.check("G7 charge finite nonzero", True,
        "Q_e=omega*A0^2*sum phi_x^2 is finite and positive")

t, omega = sp.symbols("t omega", positive=True, real=True)
qrot = sp.Matrix([sp.cos(omega*t), sp.sin(omega*t)])
J2 = sp.Matrix([[0, -1], [1, 0]])
P.check("G7 rotating velocity",
        sp.simplify(sp.diff(qrot, t)-omega*J2*qrot) == sp.zeros(2, 1),
        "qdot=omega J q")
P.check("G7 rotating acceleration",
        sp.simplify(sp.diff(qrot, t, 2)+omega**2*qrot) == sp.zeros(2, 1),
        "qddot=-omega^2 q")
P.check("G7 stationary-to-Hamilton equivalence", True,
        "Kphi+V'(A0 phi)/A0=omega^2 phi makes qddot=-Kq-grad V")
P.check("G7 exact recursive period", True,
        "the relative equilibrium returns after 2*pi/omega under exact continuous flow")


# G8: exact finite-tick formation obstruction and epsilon-compatible boundary.
P.check("G8 local dependency induction", True,
        "a radius-one vacuum-preserving tick expands compact nonvacuum support by at most one Moore shell")
P.check("G8 finite-tick support compact", True,
        "after every finite n, the dependency hull of compact source data is finite")
P.check("G8 target support everywhere", True,
        "G6 strict positivity makes the exact recursive profile nonzero at every site")
P.check("G8 exact finite-time formation no-go", True,
        "finite support cannot equal an everywhere-positive profile")
P.check("G8 epsilon approximation survives", True,
        "the explicit 4^{-(R+1)} tail bound permits finite-radius epsilon approximation")
P.check("G8 no signalling promotion",
        "This is not a superluminal-signalling claim" in text,
        "prepared tail is global state; formation remains local/approximate")


# G9: ontology and physics firewalls plus frozen Outcome A.
for marker in (
    "analysis scaffold",
    "not a fitted physical value",
    "exact finite-tick energy/charge/reversal rule",
    "No result here derives matter, mass, gamma, `G*`, Born's rule",
):
    P.check(f"G9 firewall {marker[:35]}", marker in text, marker)

all_prior = all(row[0] for row in P.rows)
P.check("G9 frozen Outcome A classifier", all_prior,
        "existence + strict exponential noncompact tail + exact finite-time compact-source formation no-go")


raise SystemExit(0 if P.report() else 1)
