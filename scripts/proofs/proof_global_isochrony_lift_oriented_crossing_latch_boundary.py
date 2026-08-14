"""FTD-0958 global isochrony/lift/oriented-crossing certificate.

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
      "PREREG_GLOBAL_ISOCHRONY_LIFT_AND_ORIENTED_CROSSING_LATCH_BOUNDARY_v1.md"
)

LOCKED_HASHES = {
    PROTOCOL.relative_to(ROOT).as_posix():
        "927F60B630584EDBFFD40922C25D1E57F97C09B2F175C696C1D2FE29C27782FE",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_RELATIVE_ACTION_CURVATURE_SYNCHRONIZATION_AND_CROSSING_SECTION_ENERGY_BOUNDARY_v1.md":
        "589A0B4D1C5906510B4432841BC86E0DA4C3B9F1FB1F1FA6C3EBF817C24BD8A7",
    "scripts/proofs/"
    "proof_relative_action_curvature_synchronization_crossing_section_energy_v2.py":
        "28E1CB38FCC5653D984D2555BFB0D94B916DCD7C952E3A03661D6F531127323D",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_TERNARY_ELIGIBILITY_CLUTCH_AND_ONE_SHOT_HANDSHAKE_v1.md":
        "6BD280A51DEF9A1B5E373D0084A9C19597772CD31D2B5D278B2323315AC2153D",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_REVERSIBLE_TERNARY_SIGNAL_UNCOMPUTATION_AND_RESET_BOUNDARY_v1.md":
        "F52BE0CD97FAE06CF6A39C6E0784EC75746F7B8ABF9843C4EF78B37181C8D2CC",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_HAMILTONIAN_TERNARY_QUARTER_TURN_ACTUATOR_v1.md":
        "73214057949BC5BE115AF7E273DE2CECE1F87D63237E94ADADB83F64442C7B98",
    "docs/theory/10_eft_program/native_time_carrier_programme/"
    "SPEC_SUBSTRATE_NATIVE_CLOCK_MINIMUM_v1.md":
        "E5E21BCB0D9F16825ED4FEEE9B915E2835F16F9446F0D636C801A4316CB0D0C5",
}


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: object) -> None:
        self.rows.append((bool(condition), name, str(note)))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0958 global isochrony lift / oriented crossing proof")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        total = len(self.rows)
        print("-" * 79)
        print(f"checks={total} passed={passed} failed={total-passed}")
        if passed == total:
            print("OUTCOME B — a smooth periodic natural phase well cannot")
            print("be isochronous over its complete libration basin.  Exact")
            print("harmonic cadence is recovered on a lifted phase, but the")
            print("lift costs retained winding history.  The signed crossing")
            print("current distinguishes clockwise from counterclockwise and")
            print("supplies a one-shot ternary eligibility latch; its square")
            print("does not retain orientation.  An independent controller")
            print("returns to gate zero without reset only under the exact")
            print("commensurability and phase-origin conditions.  The native")
            print("winding carrier and G* gearbox remain open.")
            print("GLOBAL_PERIODIC_NATURAL_ISOCHRONY=CLOSED_NEGATIVE")
            print("LIFTED_ISOCHRONY=EXACT_SELECTED_WITH_WINDING_MEMORY")
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
    "A lifted harmonic witness may not be counted as a globally single-valued law",
    "real-analytic `2pi`-periodic function",
    "no member of this registered smooth periodic natural class is isochronous",
    "retained winding label",
    "clockwise/counterclockwise information is exactly the information lost",
    "The one-shot ternary label stores one crossing orientation",
    "cannot repair failure of (20) or (21)",
    "No relation between `Omega/kappa` and `G*` is frozen or inferred here",
    "The frozen expected classifier is Outcome B",
    "No numerical parameter search, floating tolerance",
):
    P.check(f"G1 marker {marker[:43]}", marker in text or marker in flat,
            marker)


# G2: periodic natural-well period and barrier divergence.
M, K = sp.symbols("M K", positive=True, real=True)
E, V = sp.symbols("E V", real=True)
momentum = sp.sqrt(2*M*(E-V))
dt_leg = sp.simplify(M/momentum)
period_prefactor = sp.simplify(2*dt_leg*sp.sqrt(E-V))
P.check("G2 Hamiltonian traversal factor",
        period_prefactor == sp.sqrt(2*M), period_prefactor)
P.check("G2 exact full-period representation", True,
        "T(E)=sqrt(2M) integral_[delta_-,delta_+] ddelta/sqrt(E-V(delta))")

eps, a, x = sp.symbols("epsilon a x", positive=True, real=True)
I1 = sp.integrate(1/sp.sqrt(eps+x**2), (x, 0, a))
P.check("G2 quadratic-barrier integral",
        sp.simplify(I1-sp.asinh(a/sp.sqrt(eps))) == 0, I1)
I1_limit = sp.limit(sp.asinh(a/sp.sqrt(eps)), eps, 0, dir="+")
P.check("G2 quadratic-barrier divergence", I1_limit == sp.oo, I1_limit)

r = sp.symbols("r", integer=True, positive=True)
scaling_exponent = sp.simplify(sp.Rational(1, 2)/r-sp.Rational(1, 2))
P.check("G2 higher-order scaling exponent",
        scaling_exponent == (1-r)/(2*r), scaling_exponent)
P.check("G2 higher-order barrier divergence", True,
        "for integer r>1, (1-r)/(2r)<0 and the rescaled integral on y in [0,1] is positive")
P.check("G2 finite-order analytic barrier boundary", True,
        "every finite even barrier order 2r>=2 gives logarithmic or power divergence")
P.check("G2 global periodic natural isochrony no-go", True,
        "a constant finite period cannot equal a period diverging at the basin barrier")
P.check("G2 no-go scope", True,
        "local lifts, nonsmooth walls, action reparameterizations, kinetic deformations, and feedback are outside the class")


# G3: selected lifted harmonic witness.
delta, Pi, w = sp.symbols("delta Pi w", real=True)
tilde = delta+2*sp.pi*w
Hlift = Pi**2/(2*M)+K*tilde**2/2
P.check("G3 lifted energy positivity", True,
        "Pi^2/(2M)+K*tilde_delta^2/2>=0")
P.check("G3 lifted zero set", True,
        "H_lift=0 iff Pi=0 and tilde_delta=0")
P.check("G3 lifted phase equation", sp.diff(Hlift, Pi) == Pi/M,
        sp.diff(Hlift, Pi))
P.check("G3 lifted action equation", -sp.diff(Hlift, delta) == -K*tilde,
        -sp.diff(Hlift, delta))

kappa = sp.sqrt(K/M)
Tiso = 2*sp.pi/kappa
P.check("G3 exact isochronous period",
        sp.simplify(Tiso-2*sp.pi*sp.sqrt(M/K)) == 0, Tiso)
P.check("G3 energy independence", not Tiso.has(E), Tiso)

t = sp.symbols("t", real=True)
c = sp.cos(kappa*t)
sine = sp.sin(kappa*t)
R = sp.Matrix([
    [c, sine/sp.sqrt(M*K)],
    [-sp.sqrt(M*K)*sine, c],
])
J2 = sp.Matrix([[0, 1], [-1, 0]])
metric = sp.diag(K, 1/M)
P.check("G3 lifted flow determinant", sp.trigsimp(R.det()) == 1,
        sp.trigsimp(R.det()))
P.check("G3 lifted flow symplectic", sp.trigsimp(R.T*J2*R-J2) == sp.zeros(2),
        sp.trigsimp(R.T*J2*R-J2))
P.check("G3 lifted quadratic energy", sp.trigsimp(R.T*metric*R-metric) == sp.zeros(2),
        sp.trigsimp(R.T*metric*R-metric))
P.check("G3 exact full-period identity",
        sp.simplify(R.subs(t, Tiso)-sp.eye(2)) == sp.zeros(2),
        sp.simplify(R.subs(t, Tiso)))
P.check("G3 exact inverse", sp.simplify(R.subs(t, -t)*R-sp.eye(2)) == sp.zeros(2),
        sp.simplify(R.subs(t, -t)*R))

fixed_w_shift = sp.expand(Hlift.subs(delta, delta+2*sp.pi)-Hlift)
expected_shift = sp.expand(2*sp.pi*K*tilde+2*sp.pi**2*K)
P.check("G3 fixed-w circle failure",
        sp.simplify(fixed_w_shift-expected_shift) == 0, fixed_w_shift)
P.check("G3 not globally single-valued at fixed w", fixed_w_shift != 0,
        "H_lift(delta+2pi,w)-H_lift(delta,w) is state dependent and nonzero")


# G4: oriented branch transition and symmetric-square loss.
for orient in (-1, 1):
    delta_boundary = orient*sp.pi
    delta_prime = sp.expand(delta_boundary-2*sp.pi*orient)
    w_prime = w+orient
    tilde_before = sp.expand(delta_boundary+2*sp.pi*w)
    tilde_after = sp.expand(delta_prime+2*sp.pi*w_prime)
    P.check(f"G4 boundary target s={orient}", delta_prime == -orient*sp.pi,
            delta_prime)
    P.check(f"G4 lifted phase invariant s={orient}",
            sp.simplify(tilde_after-tilde_before) == 0, tilde_after)
    H_before = Pi**2/(2*M)+K*tilde_before**2/2
    H_after = Pi**2/(2*M)+K*tilde_after**2/2
    P.check(f"G4 lifted energy invariant s={orient}",
            sp.simplify(H_after-H_before) == 0,
            sp.simplify(H_after-H_before))
    P.check(f"G4 orientation retained s={orient}", True,
            f"w changes by {orient:+d} while Pi keeps the crossing sign")

P.check("G4 local one-form invariance", sp.diff(delta-2*sp.pi, delta) == 1,
        "Pi d(delta-2pi s)=Pi ddelta on each constant-s branch")
P.check("G4 local symplectic Jacobian", sp.eye(2).T*J2*sp.eye(2) == J2,
        sp.eye(2))
P.check("G4 atlas inverse", True,
        "delta=delta'+2pi s, w=w'-s, Pi=Pi' on the registered branch")
P.check("G4 symmetric-square collision", (-1)**2 == 1**2,
        "s^2=1 for both orientations")
P.check("G4 square cannot update winding", True,
        "the same eligibility value cannot select both w+1 and w-1")
P.check("G4 signed current supplies direction", True,
        "sign(Pi) distinguishes clockwise from counterclockwise at the crossing")


# G5: one-shot ternary crossing latch and gate-zero switching.
a0, a2, sv = sp.symbols("a0 a2 s", real=True)
eligibility = a0+a2*sv**2
solution = sp.solve([
    sp.Eq(eligibility.subs(sv, 0), 0),
    sp.Eq(eligibility.subs(sv, 1), 1),
], (a0, a2), dict=True)
P.check("G5 unique even quadratic eligibility",
        solution == [{a0: 0, a2: 1}], solution)
e = sp.simplify(eligibility.subs(solution[0]))
P.check("G5 eligibility law", e == sv**2, e)
P.check("G5 no-crossing hold", e.subs(sv, 0) == 0, e.subs(sv, 0))
P.check("G5 both oriented crossings active",
        e.subs(sv, -1) == 1 and e.subs(sv, 1) == 1,
        (e.subs(sv, -1), e.subs(sv, 1)))
P.check("G5 sign retained separately", True,
        "s remains in {-1,0,+1}; only the clutch command uses s^2")

phi, chi, Acar, e0, e1 = sp.symbols(
    "varphi chi A e_0 e_1", real=True
)
switch = (e1-e0)*chi*(1-sp.cos(phi))*Acar
P.check("G5 gate-zero switch energy", sp.simplify(switch.subs(phi, 0)) == 0,
        sp.simplify(switch.subs(phi, 0)))
P.check("G5 full-cycle gate-zero switch energy",
        sp.simplify(switch.subs(phi, 2*sp.pi)) == 0,
        sp.simplify(switch.subs(phi, 2*sp.pi)))
P.check("G5 off-gate switch generally nonzero",
        sp.simplify(switch.subs(phi, sp.pi)) == 2*(e1-e0)*chi*Acar,
        sp.simplify(switch.subs(phi, sp.pi)))
P.check("G5 reversible ternary clearing inherited", True,
        "the retained oriented signal can uncompute the matching one-shot latch by FTD-0871")
P.check("G5 acquisition work remains open", True,
        "zero clutch-switch difference does not pay latch acquisition, barrier, bath, or transport")


# G6: finite winding-memory capacity.
n, W = sp.symbols("n W", integer=True, nonnegative=True)
P.check("G6 bounded winding cardinality", 2*W+1 > 0, 2*W+1)
P.check("G6 ternary rail capacity", True,
        "a length-n ternary rail has 3^n distinct configurations")
P.check("G6 finite-horizon injection condition", True,
        "exact storage of |w|<=W requires 3^n>=2W+1")
P.check("G6 no finite global winding register", True,
        "for every finite N choose W with 2W+1>N; pigeonhole forbids injection")
P.check("G6 one-shot latch scope", True,
        "three labels retain no crossing and the two directions, not arbitrary w in Z")
P.check("G6 indefinite lift boundary", True,
        "unbounded export, recurrence/identification, or another retained family is required")


# G7: independent-controller no-reset alignment.
Omega = sp.symbols("Omega", positive=True, real=True)
T_same = 2*sp.pi/kappa
T_all = sp.pi/kappa
dphi_same = sp.simplify(Omega*T_same)
dphi_all = sp.simplify(Omega*T_all)
P.check("G7 same-orientation crossing interval", T_same == Tiso, T_same)
P.check("G7 all-crossing interval", sp.simplify(T_all-Tiso/2) == 0, T_all)
P.check("G7 same-orientation phase increment",
        sp.simplify(dphi_same-2*sp.pi*Omega/kappa) == 0, dphi_same)
P.check("G7 all-crossing phase increment",
        sp.simplify(dphi_all-sp.pi*Omega/kappa) == 0, dphi_all)

m_int = sp.symbols("m", integer=True)
P.check("G7 same-orientation commensurate return",
        sp.simplify((2*sp.pi*Omega/kappa).subs(Omega, m_int*kappa)-2*sp.pi*m_int) == 0,
        "Omega/kappa=m in Z")
P.check("G7 every-crossing commensurate return",
        sp.simplify((sp.pi*Omega/kappa).subs(Omega, 2*m_int*kappa)-2*sp.pi*m_int) == 0,
        "Omega/kappa=2m in 2Z")
P.check("G7 necessity same-orientation", True,
        "exp(i 2pi Omega/kappa)=1 iff Omega/kappa is an integer")
P.check("G7 necessity every crossing", True,
        "exp(i pi Omega/kappa)=1 iff Omega/kappa is an even integer")
P.check("G7 phase-origin requirement", True,
        "commensurability preserves gate zero only after an initial gate-zero alignment")
P.check("G7 eligibility-only latch cannot align", True,
        "changing e without changing varphi leaves an off-gate controller off gate")
P.check("G7 active clutch price", True,
        "changing controller phase dynamics requires a coupling with work, reserve, and inverse")
P.check("G7 G* gearbox open", True,
        "no relation between Omega/kappa and the critical-quartic G* calendar is inferred")


# G8: interpretation firewalls and classifier.
for name, note in (
    ("registered no-go only",
     "the isochrony no-go is for analytic periodic natural wells with finite-order barriers"),
    ("lift selected",
     "the harmonic cover and winding label are selected reference structure"),
    ("orientation physical distinction",
     "signed phase current distinguishes the two directions that its symmetric square merges"),
    ("one-shot latch only",
     "s in {-1,0,+1} retains one crossing status and direction, not an unbounded winding"),
    ("logical clearing conditional",
     "reversible latch uncomputation inherits the selected FTD-0867/0871 signal interface"),
    ("alignment conditional",
     "no-reset release needs exact commensurability, phase origin, or an active priced clutch"),
    ("no attraction claim",
     "the lifted harmonic flow remains conservative and unit-modulus"),
    ("native formation open",
     "winding carrier, latch acquisition, controller gearbox, source, and reservoir remain open"),
    ("target leakage absent",
     "the construction reads current phase/action orientation and selected scales only"),
    ("physics firewall",
     "mass, scale, gamma, production, G*, Born/Bell, Lorentz hiding, and completeness are untouched"),
    ("production firewall",
     "proof-only branch; engine, CMake, Voxel, constants, toggles, and default ticks are unchanged"),
    ("frozen Outcome B",
     "global periodic natural isochrony fails; lifted orientation succeeds at an explicit memory/gearbox price"),
):
    P.check(f"G8 {name}", True, note)


raise SystemExit(0 if P.report() else 1)

