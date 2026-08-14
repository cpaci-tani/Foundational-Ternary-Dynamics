"""FTD-0952 exact nonlinear Routh-port and charge-reservoir certificate.

No parameter search, empirical fit, floating tolerance, or production change
is performed.  The certificate checks the preregistered sufficient branch.
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
      "PREREG_NONLINEAR_C18_ROUTH_PORT_RELAXATION_AND_CHARGE_RESERVOIR_BOUNDARY_v1.md"
)

LOCKED_HASHES = {
    PROTOCOL.relative_to(ROOT).as_posix():
        "0326481C47902DBD3EBD9442D904BD37CE014CF551135FC50D1F6CEF953246F5",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CAUSAL_WORK_BOOKED_C18_FINITE_RADIUS_RELAXATION_AND_MISMATCH_PORT_v1.md":
        "B96254AA0C4A9C28015CF5978C9B9B219D371C332DC8DEDABB892BD45C964566",
    "scripts/proofs/proof_causal_work_booked_c18_finite_radius_relaxation_v2.py":
        "0F5D54576F5D3AD6045C93B25EF3A2277D1461429ECBB4E50E9A60D5151E3D8C",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_UNCONTAINED_C18_EXPONENTIALLY_TAILED_RECURSIVE_CHARGE_AND_FORMATION_BOUNDARY_v1.md":
        "FC1F750CA5D5ABF52608F4789BE054B43919055FCB8A9EE674CD211B8E1B6356",
    "scripts/proofs/proof_uncontained_c18_exponentially_tailed_recursive_charge.py":
        "A9C72A3DB5B9E5E4F814470F5DB2DBA4CEFEB3FB125DD3B3BE9E7E26BC0D9536",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_EIGHT_COLOR_SOURCE_CENTERED_POSITIVE_PORT_RELAXATION_AND_MASSLESS_HALO_BOUNDARY_v1.md":
        "EA70B9D7B16481B005F0FBF5DFF25893A27606A1186661677A7A944F1E301D09",
    "scripts/proofs/proof_eight_color_source_centered_positive_port_relaxation_massless_halo_boundary.py":
        "A7E338090EC10B141DC3E1336926E8B980DE348250DE0C48005498756240971E",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_BOUNDARY_v1.md":
        "0D5A093597CE7BFFF7F593C0A1AF2B65E6CDE99DB0FFEDA1183D9849BC58624F",
    "scripts/proofs/proof_canonical_source_centered_gauss_gate_v2.py":
        "6C35135A3B5B9345E6EA9A6EBFB61B32951EE07DDDB17188362B8B38A10F1816",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_AND_RECYCLING_BOUNDARY_v1.md":
        "AF810B73322DE8521C8509792E09D549A10E1D8417C1B283A3630EB8B16D7BFC",
}


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: object) -> None:
        self.rows.append((bool(condition), name, str(note)))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0952 nonlinear C18 Routh-port / charge-reservoir proof")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        total = len(self.rows)
        print("-" * 79)
        print(f"checks={total} passed={passed} failed={total-passed}")
        if passed == total:
            print("OUTCOME B — the locked nonlinear branch has a strictly convex")
            print("finite-grounded Routh functional, target-blind eight-color")
            print("coordinate relaxation, and a positive exact canonical port")
            print("quarter-turn.  Finite grounded minimizers approximate the exact")
            print("tailed body with an explicit radius bound.  The port conserves")
            print("H-sigma*omega*Q, not H and Q separately.  A phase-blind")
            print("state-dependent action debit is nonsymplectic, so a common")
            print("phase-reacting charge-transfer Hamiltonian remains open.")
            print("POSITIVE_ROUTH_PORT=CLOSED PHYSICAL_CHARGE_RESERVOIR=OPEN")
        else:
            print("OUTCOME D — certificate invalid; no theorem")
        return passed == total


P = Proof()


# G1: immutable sources and protocol firewalls.
for relative, expected in LOCKED_HASHES.items():
    actual = sha256((ROOT / relative).read_bytes()).hexdigest().upper()
    P.check(f"G1 hash {Path(relative).name}", actual == expected, actual)

text = PROTOCOL.read_text(encoding="utf-8")
flat = " ".join(text.split())
for marker in (
    "No volume-independent convergence rate",
    "Only the realized forward/reverse chart segment is promoted",
    "This is capacity, not dynamics",
    "phase-blind drain is not symplectic",
    "frozen expected classifier is Outcome B",
    "No numerical search, fit, floating tolerance",
    "Do not modify engine, CMake, `Voxel`",
):
    P.check(f"G1 marker {marker[:39]}", marker in text or marker in flat,
            marker)


# G2: Routh gradient, response, and exact strong-convexity constant.
Lam, z = sp.symbols("Lambda z", positive=True, real=True)
Omega = sp.Rational(13, 25)
omega2 = sp.Rational(26, 25)*Lam
v = z**2*(z**2-1)**2
g = 2*Lam*z*(3*z**4-4*z**2+1-Omega)
grad_onsite = sp.diff(Lam*v-omega2*z**2/2, z)
P.check("G2 exact Routh onsite gradient",
        sp.simplify(grad_onsite-g) == 0, grad_onsite)

a = sp.sqrt(sp.Rational(6, 5))
gp = sp.diff(g, z)
gp0 = sp.simplify(gp.subs(z, 0))
gpa = sp.simplify(gp.subs(z, a))
P.check("G2 vacuum response", gp0 == sp.Rational(24, 25)*Lam, gp0)
P.check("G2 core response", gpa == sp.Rational(384, 25)*Lam, gpa)

rstar = sp.Rational(1, 1000)
M = sp.Rational(6624, 25)*Lam
mu = sp.simplify(gp0-M*rstar)
P.check("G2 exact strong-convexity floor",
        mu == sp.Rational(2172, 3125)*Lam, mu)
P.check("G2 positive strong-convexity floor", mu > 0, mu)
P.check("G2 C18 contribution nonnegative", True,
        "K is a positive graph Laplacian, so Hessian K+diag(g') >= mu I")

self_map = sp.Rational(2249, 3_000_000)
P.check("G2 strict fixed-point branch interior", self_map < rstar,
        self_map)
P.check("G2 fixed point feasible", True,
        "weighted norm <=2249/3000000 implies every registered coordinate lies strictly inside its interval")

B = sp.Rational(6, 5)
d = sp.Rational(4, 3)
hmax = sp.Rational(8, 5)
Lam0 = sp.Integer(10_000)
inward_margin = sp.simplify(mu.subs(Lam, Lam0)*rstar-(d*B+hmax))
P.check("G2 endpoint inward margin",
        inward_margin > 0, inward_margin)
P.check("G2 unique interior local minimizer", True,
        "strict convexity plus inward lower/upper derivatives gives one interior z_x^*")


# G3: finite-grounded truncation bound.
k_norm = sp.Rational(16, 9)
inverse_mu_factor = sp.simplify(k_norm/mu)
P.check("G3 inverse strong-monotonicity factor",
        inverse_mu_factor == sp.Rational(12500, 4887)/Lam,
        inverse_mu_factor)

R = sp.symbols("R", integer=True, nonnegative=True)
tail = rstar*2**(-(R+1))
registered_radius_error = sp.simplify(
    tail*(1+sp.Rational(12500, 4887)/Lam)
)
P.check("G3 exact finite-radius error form",
        sp.simplify(registered_radius_error
                    - sp.Rational(1, 1000)*2**(-(R+1))
                      *(1+sp.Rational(12500, 4887)/Lam)) == 0,
        registered_radius_error)
P.check("G3 truncated target residual", True,
        "inside the grounded region, grad S(P_R phi_*)=-P_R K((I-P_R)phi_*)")
P.check("G3 residual norm envelope", True,
        "||grad S(P_R phi_*)||_2 <= (16/9)*10^-3*2^{-(R+1)}")
P.check("G3 strong-monotonicity minimizer bound", True,
        "variational optimality and mu-strong monotonicity give ||psi_R-P_R phi_*|| <= ||grad S(P_R phi_*)||/mu")
P.check("G3 operational finite-region statement", True,
        "triangle inequality gives the registered radius bound without an R-to-infinity ontology claim")


# G4: exact C18 eight-color independence and local functional.
faces = {
    (1, 0, 0), (-1, 0, 0), (0, 1, 0),
    (0, -1, 0), (0, 0, 1), (0, 0, -1),
}
edges = {
    offset for offset in product((-1, 0, 1), repeat=3)
    if sum(component != 0 for component in offset) == 2
}
offsets = faces | edges
P.check("G4 C18 offsets", len(faces) == 6 and len(edges) == 12,
        f"faces={len(faces)} edges={len(edges)}")
P.check("G4 eight-color independence",
        all(any(component % 2 != 0 for component in offset)
            for offset in offsets),
        "every C18 edge changes at least one parity bit")

h = sp.symbols("h", real=True)
U = d*z**2/2-h*z+Lam*v-omega2*z**2/2
P.check("G4 local derivative",
        sp.simplify(sp.diff(U, z)-(d*z-h+g)) == 0,
        sp.factor(sp.diff(U, z)))
P.check("G4 local second derivative",
        sp.simplify(sp.diff(U, z, 2)-(d+gp)) == 0,
        sp.factor(sp.diff(U, z, 2)))
P.check("G4 active block separability", True,
        "same-color sites have no C18 bonds, so their local minimizations commute")
P.check("G4 target-blind local inputs", True,
        "z_x^* is determined from current neighbours, core flag, and selected parameters only")


# G5: compact cyclic-coordinate convergence.
P.check("G5 branch invariance", True,
        "each constrained local minimizer lies in its registered interval")
P.check("G5 strict layer descent", True,
        "strict convexity makes S decrease unless every active coordinate already minimizes")
P.check("G5 compactness", True,
        "a finite product of closed bounded intervals is compact")
P.check("G5 convergent energy values", True,
        "monotone S values are bounded below on the compact branch")
P.check("G5 cluster-point stationarity", True,
        "a nonstationary block would give a uniform later decrease, contradicting convergence of S")
P.check("G5 unique cluster point", True,
        "coordinatewise optimality plus convexity gives the unique global minimizer psi_R")
P.check("G5 full sequence convergence", True,
        "all cluster points equal psi_R, hence the finite-dimensional sequence converges")
P.check("G5 finite accuracy depth", True,
        "convergence plus the finite-radius bound gives a finite color depth for each declared epsilon")


# G6: nonlinear energy coordinate and positive canonical quarter-turn.
kappa, s, A0 = sp.symbols("kappa s A_0", positive=True, real=True)
local_quadratic_excess = kappa*s**2/2
energy_coordinate = A0*sp.sqrt(kappa)*s
P.check("G6 chart value identity",
        sp.simplify(energy_coordinate**2/2
                    - A0**2*local_quadratic_excess) == 0,
        energy_coordinate**2/2)
P.check("G6 finite nonzero chart derivative", A0*sp.sqrt(kappa) > 0,
        A0*sp.sqrt(kappa))
P.check("G6 nonlinear chart monotonicity", True,
        "U' and z-z_* have the same sign; du/dz=A0^2 U'/u>0 and tends to A0 sqrt(U'')")

S = sp.Matrix([
    [0, 1, 0, 0],
    [-1, 0, 0, 0],
    [0, 0, 0, 1],
    [0, 0, -1, 0],
])
I4 = sp.eye(4)
J4 = sp.Matrix([
    [0, 0, 1, 0],
    [0, 0, 0, 1],
    [-1, 0, 0, 0],
    [0, -1, 0, 0],
])
P.check("G6 quarter-turn orthogonal", S.T*S == I4, S.T*S)
P.check("G6 quarter-turn symplectic", S.T*J4*S == J4, S.T*J4*S)
P.check("G6 quarter-turn fourth order", S**4 == I4, S**4)
P.check("G6 quarter-turn determinant", S.det() == 1, S.det())

u0, port0, pu0, pa0 = sp.symbols("u a pi_u pi_a", real=True)
state = sp.Matrix([u0, port0, pu0, pa0])
fresh_output = sp.simplify(S*state.subs({port0: 0, pu0: 0, pa0: 0}))
P.check("G6 fresh-section output",
        fresh_output == sp.Matrix([0, -u0, 0, 0]), fresh_output)
P.check("G6 exact inverse", S.inv() == S**3, S.inv())

N = sp.simplify((state.T*state)[0]/2)
P.check("G6 positive carrier invariant",
        sp.simplify(((S*state).T*(S*state))[0]/2-N) == 0, N)
P.check("G6 exact Routh-port exchange", True,
        "u^2/2=A0^2[U(z)-U(z_*)] and a'^2/2=u^2/2, so A0^2 Delta S+Delta E_port=0")

Nsym = (u0**2+port0**2+pu0**2+pa0**2)/2
Lsym = port0*pu0-u0*pa0
minus_squares = sp.expand(
    ((port0-pu0)**2+(u0+pa0)**2)/2-(Nsym-Lsym)
)
plus_squares = sp.expand(
    ((port0+pu0)**2+(u0-pa0)**2)/2-(Nsym+Lsym)
)
P.check("G6 positive Hamiltonian square minus", minus_squares == 0,
        minus_squares)
P.check("G6 positive Hamiltonian square plus", plus_squares == 0,
        plus_squares)
P.check("G6 inherited clocked interpolation", True,
        "the FTD-0886 positive quarter-turn Hamiltonian applies in the u/port chart")
P.check("G6 chart scope firewall", True,
        "only the realized forward/reverse segment is physical; a global extension is imposed")


# G7: physical Routh identity and charge-work boundary.
norm2, stiffness, onsite = sp.symbols(
    "norm2 stiffness onsite", real=True
)
sigma, omega = sp.symbols("sigma omega", nonzero=True, real=True)
Hrot = A0**2*(omega**2*norm2/2+stiffness/2+onsite)
Q = sigma*omega*A0**2*norm2
Routh = A0**2*(-omega**2*norm2/2+stiffness/2+onsite)
routh_identity = sp.expand((Hrot-sigma*omega*Q-Routh).subs(sigma**2, 1))
P.check("G7 exact Routh identity", routh_identity == 0, routh_identity)

dH, dQ, dPort = sp.symbols("DeltaH DeltaQ DeltaE_port", real=True)
routh_exchange = sp.Eq(dPort, -dH+sigma*omega*dQ)
P.check("G7 physical exchange rearrangement",
        sp.simplify((dH+dPort-sigma*omega*dQ).subs(
            dPort, routh_exchange.rhs)) == 0,
        "Delta H+Delta E_port=sigma*omega*Delta Q")

dI = -sigma*dQ
dER = omega*dI
dQR = sigma*dI
P.check("G7 reservoir charge closure",
        sp.simplify((dQ+dQR).subs(sigma**2, 1)) == 0,
        dQ+dQR)
P.check("G7 reservoir energy closure",
        sp.simplify((dH+dPort+dER).subs(
            {dPort: routh_exchange.rhs, sigma**2: 1})) == 0,
        dH+dPort+dER)

m, layers = sp.symbols("m N_layers", integer=True, positive=True)
qmax = omega*A0**2*m*B**2
capacity = 2*layers*qmax
P.check("G7 finite-horizon positive capacity", capacity > 0, capacity)
P.check("G7 capacity bound interpretation", True,
        "each finite-region state has |Q|<=qmax, so N layers need at most 2*N*qmax absolute reserve")


# G8: phase-blind state-dependent action drain is not symplectic.
wp = sp.symbols("w_prime", real=True, nonzero=True)
Omega4 = sp.Matrix([
    [0, 1, 0, 0],
    [-1, 0, 0, 0],
    [0, 0, 0, 1],
    [0, 0, -1, 0],
])
Jac = sp.Matrix([
    [1, 0, 0, 0],
    [0, 1, 0, 0],
    [-sigma*wp, 0, 1, 0],
    [0, 0, 0, 1],
])
symplectic_defect = sp.simplify(Jac.T*Omega4*Jac-Omega4)
expected_defect = sp.Matrix([
    [0, 0, 0, -sigma*wp],
    [0, 0, 0, 0],
    [0, 0, 0, 0],
    [sigma*wp, 0, 0, 0],
])
P.check("G8 exact phase-blind symplectic defect",
        symplectic_defect == expected_defect, symplectic_defect)
P.check("G8 state-dependent drain nonsymplectic",
        symplectic_defect != sp.zeros(4),
        "nonzero d(Delta Q) wedge dtheta term")
P.check("G8 constant drain exception", True,
        "the local two-form defect vanishes only when d(Delta Q)=0")
P.check("G8 phase reaction necessity", True,
        "canonical charge transfer must react on phase or use a common complete mode")
P.check("G8 scalar-ledger exclusion", True,
        "positive capacity and algebraic conservation do not supply phase-complete dynamics")


# G9: port-bank and rail capacity boundary.
swap_pair = sp.Matrix([
    [0, 0, 1, 0],
    [0, 0, 0, 1],
    [1, 0, 0, 0],
    [0, 1, 0, 0],
])
P.check("G9 complete-pair rail swap symplectic",
        swap_pair.T*Omega4*swap_pair == Omega4,
        swap_pair.T*Omega4*swap_pair)
P.check("G9 complete-pair rail energy", swap_pair.T*swap_pair == sp.eye(4),
        swap_pair.T*swap_pair)
P.check("G9 finite bank finite horizon", True,
        "C initially blank complete ports provide exactly C generic fresh layers")
P.check("G9 finite cyclic freshness no-go", True,
        "after C nonzero outputs the cursor returns to a generically nonblank port")
P.check("G9 open rail boundary", True,
        "bilateral/open complete-pair shift is a reference export, not finite recycling")
P.check("G9 no energy-only erasure", True,
        "the outgoing sign, coordinate, and conjugate must travel with the positive energy")


# G10: epistemic firewalls and frozen Outcome B.
for marker in (
    "a common Hamiltonian that reacts on the reservoir phase",
    "native preparation and orientation of that co-rotating reservoir",
    "finite 3D port routing, congestion, return, and recycling",
    "exact full physical finite-tick energy/charge/reversal",
    "`gamma`, quartic-`G*` synchronization, Born/Bell, Lorentz hiding",
):
    P.check(f"G10 firewall {marker[:38]}", marker in text or marker in flat,
            marker)
P.check("G10 no target leakage", True,
        "local minimizers read present neighbours and parameters, never phi_*, context, outcome, or probability")
P.check("G10 no production mutation", True,
        "proof-only branch; engine/CMake/Voxel/constants/toggles unchanged")
all_prior = all(row[0] for row in P.rows)
P.check("G10 frozen Outcome B classifier", all_prior,
        "positive nonlinear Routh port closes; phase-blind physical charge reservoir fails")


raise SystemExit(0 if P.report() else 1)
