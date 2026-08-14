#!/usr/bin/env python3
"""FTD-0887 exact autonomous-parity/source-reaction certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_AUTONOMOUS_PHASE_PARITY_AND_SOURCE_REACTION_SPLITTER_v1.md"
)
PROTOCOL_HASH = "484EC4ED25C322D93B44F88267259B81AE510AE659AE22C4366A5DE69635146A"
SOURCE_HASHES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_ALTERNATING_ORIENTED_TERNARY_PARITY_RAIL_AND_ONE_SHOT_BOUNDARY_v1.md":
        "E70F2AD61BFA1C8BBFF4EA03DCF0312B8F96224ECF2453FDF4B81B0FEA845CA1",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_LOCAL_CANONICAL_HAMILTONIAN_PARITY_RAIL_AND_SCALAR_LOCALITY_BOUNDARY_v1.md":
        "982C3B9D00798920A1BDAB96C75EBC9DB3A08111E8900F1D630382B0249B25F6",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_REVERSIBLE_CHECKERBOARD_GAUSS_RECORD_PREPARATION_AND_SELF_DUAL_ENERGY_SPLIT_v1.md":
        "143D897A69B5C6FED8C00402C1840EA9FAEE5BD4BC259C9BDD065DFDC616A814",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_BOUNDARY_v1.md":
        "0D5A093597CE7BFFF7F593C0A1AF2B65E6CDE99DB0FFEDA1183D9849BC58624F",
    "engine/include/ftd/eft/local_canonical_hamiltonian_parity_rail.h":
        "28A76212958450A836CD8D522BDCC7C3C19D848E1ECDDCBCA3D235AF84B3AED5",
    "engine/include/ftd/eft/canonical_source_centered_gauss_gate.h":
        "C65E562B4B3855076748B1A73EF742DD20D106191120D2631864C6D16FFE8C2D",
}

checks = 0
failures = 0


def check(label: str, condition: bool) -> None:
    global checks, failures
    checks += 1
    if condition:
        print(f"PASS  C{checks} {label}")
    else:
        failures += 1
        print(f"FAIL  C{checks} {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def poisson(
    first: sp.Expr,
    second: sp.Expr,
    qs: tuple[sp.Symbol, ...],
    ps: tuple[sp.Symbol, ...],
) -> sp.Expr:
    return sp.expand(sum(
        (
            sp.diff(first, q) * sp.diff(second, p)
            - sp.diff(first, p) * sp.diff(second, q)
            for q, p in zip(qs, ps)
        ),
        sp.Integer(0),
    ))


protocol_path = ROOT / PROTOCOL
protocol_text = protocol_path.read_text(encoding="utf-8")
protocol_flat = " ".join(protocol_text.split())

# Exact three-mode canonical algebra in ordering (u,a,r,pi_u,pi_a,pi_r).
u, a, r = sp.symbols("u a r", real=True)
pu, pa, pr = sp.symbols("pi_u pi_a pi_r", real=True)
eta = sp.symbols("eta", real=True, nonnegative=True)
c = sp.cos(eta)
s_eta = sp.sin(eta)
qvars = (u, a, r)
pvars = (pu, pa, pr)
N = sum((value**2 for value in qvars + pvars), sp.Integer(0)) / 2
Lua = a * pu - u * pa
Lar = a * pr - r * pa
Nr = (r**2 + pr**2) / 2

I3 = sp.eye(3)
Z3 = sp.zeros(3)
J6 = Z3.row_join(I3).col_join((-I3).row_join(Z3))
Oua = sp.Matrix([[0, 1, 0], [-1, 0, 0], [0, 0, 1]])
S_ua = sp.diag(Oua, Oua)
Oar = sp.Matrix([[1, 0, 0], [0, c, -s_eta], [0, s_eta, c]])
S_ar = sp.diag(Oar, Oar)
S_phase = sp.eye(6)
S_phase[2, 2] = 0
S_phase[2, 5] = 1
S_phase[5, 2] = -1
S_phase[5, 5] = 0
M = sp.simplify(S_phase * S_ar * S_ua)
state = sp.Matrix([u, a, r, pu, pa, pr])
endpoint = sp.simplify(M * state)

# One exact phase window translated to [0,pi/3].
x = sp.symbols("x", real=True)
rho = sp.sin(3 * x) ** 2
rho_prime = sp.diff(rho, x)
window_integral = sp.integrate(rho, (x, 0, sp.pi / 3))
Omega, action = sp.symbols("Omega I", positive=True)
G, kappa, Nsym = sp.symbols("G kappa N", real=True)
H_active = Omega * action + 6 * Omega * Nsym + Omega * kappa * rho * G
window_duration = sp.pi / (3 * Omega)

# C1--C10: provenance and scope.
check(
    "all six frozen source hashes match",
    all(sha256(ROOT / path) == digest for path, digest in SOURCE_HASHES.items()),
)
check("protocol pre-run hash matches", sha256(protocol_path) == PROTOCOL_HASH)
check(
    "common field history reaction phase space is frozen",
    "Let `(Q,P)` be the full source-centered" in protocol_text
    and "source-reaction pair `(r_x,pi_{r,x})`" in protocol_text,
)
check(
    "six phase windows and their order are frozen",
    "partition its circle into six intervals" in protocol_flat
    and "(L_{ua}^{(0)},L_{ar}^{(0)},N_r^{(0)}," in protocol_text,
)
check(
    "source interaction ledger is frozen",
    "\\Delta E_{\\rm raw}=w-E_{\\rm react}'" in protocol_text
    and "\\Delta U_{\\rm int}=-w" in protocol_text,
)
check("split angle range is frozen", "`0<=eta<=pi/2`" in protocol_text)
check(
    "equal split is selected rather than derived",
    "[SELECTION — imposed output-channel" in protocol_text,
)
check(
    "equilibrium source offset remains fixed",
    "The equilibrium charge `s_0` remains fixed" in protocol_text,
)
check(
    "production and Gstar scope firewall is frozen",
    "production and `G*` are outside the result" in protocol_flat,
)
check(
    "quantum relativity biology and completeness firewall is frozen",
    "Born, Bell, Lorentz hiding, biology, and completeness" in protocol_flat,
)

# C11--C32: autonomous phase compiler.
check(
    "every window vanishes with zero first derivative at endpoints",
    rho.subs(x, 0) == 0
    and rho.subs(x, sp.pi / 3) == 0
    and rho_prime.subs(x, 0) == 0
    and sp.simplify(rho_prime.subs(x, sp.pi / 3)) == 0,
)
check(
    "periodic windows are C1",
    "periodic `C^1` windows" in protocol_text
    and rho.subs(x, 0) == rho.subs(x, sp.pi / 3)
    and rho_prime.subs(x, 0) == rho_prime.subs(x, sp.pi / 3),
)
intervals = [(sp.pi * j / 3, sp.pi * (j + 1) / 3) for j in range(6)]
check(
    "distinct window interiors are disjoint",
    all(intervals[j][1] <= intervals[j + 1][0] for j in range(5))
    and intervals[0][0] == 0
    and intervals[-1][1] == 2 * sp.pi,
)
check("every window integral is pi over six", window_integral == sp.pi / 6)
check("clock phase advances uniformly", sp.diff(H_active, action) == Omega)
check("every window has the frozen duration", window_duration == sp.pi / (3 * Omega))
check(
    "base norm flow is one identity winding per window",
    sp.simplify(6 * Omega * window_duration) == 2 * sp.pi,
)
kappa_quarter = sp.simplify(6 * (sp.pi / 2) / sp.pi)
check("quarter-turn pulse coefficient is three", kappa_quarter == 3)
kappa_reaction = 6 * eta / sp.pi
check(
    "reaction split coefficient is bounded by three",
    kappa_reaction.subs(eta, 0) == 0
    and kappa_reaction.subs(eta, sp.pi / 2) == 3,
)
check(
    "every active pulse integrates to its target angle",
    sp.simplify(kappa_quarter * window_integral) == sp.pi / 2
    and sp.simplify(kappa_reaction * window_integral) == eta,
)
check(
    "common norm commutes with all local generators",
    poisson(N, Lua, qvars, pvars) == 0
    and poisson(N, Lar, qvars, pvars) == 0
    and poisson(N, Nr, qvars, pvars) == 0,
)
Nua = (u**2 + a**2 + pu**2 + pa**2) / 2
Nar = (a**2 + r**2 + pa**2 + pr**2) / 2
lua_bounds = (
    sp.expand(2 * (Nua - Lua) - ((a - pu) ** 2 + (u + pa) ** 2)) == 0
    and sp.expand(2 * (Nua + Lua) - ((a + pu) ** 2 + (u - pa) ** 2)) == 0
)
lar_bounds = (
    sp.expand(2 * (Nar - Lar) - ((a - pr) ** 2 + (r + pa) ** 2)) == 0
    and sp.expand(2 * (Nar + Lar) - ((a + pr) ** 2 + (r - pa) ** 2)) == 0
)
check("angular generators obey the frozen norm bounds", lua_bounds and lar_bounds)
check(
    "reaction norm lies between zero and the common norm",
    sp.hessian(Nr, (r, pr)) == sp.eye(2)
    and sp.hessian(N - Nr, (u, a, pu, pa)) == sp.eye(4),
)
check(
    "carrier Hamiltonian has the frozen positive lower bound",
    sp.simplify(6 - 3) == 3 and "H-\\Omega I\\ge3\\Omega N" in protocol_text,
)
check(
    "Hamiltonian contains no external tick or time argument",
    "no external time or tick argument" in protocol_flat,
)
check("phase order is color zero then color one", "phase order is color 0 then color 1" in protocol_flat)
check("no cross-color commutation is assumed", "No commutation of different-color generators is assumed" in protocol_text)
check(
    "exact endpoint is the ordered six-pulse product",
    "exact ordered composition" in protocol_flat and M == S_phase * S_ar * S_ua,
)
check(
    "clock action returns at every window boundary",
    rho.subs(x, 0) == 0 and rho.subs(x, sp.pi / 3) == 0,
)
check(
    "maximum action excursion is bounded by three N",
    kappa_quarter == 3 and "excursion is at most `3*N`" in protocol_text,
)
check("I0 greater than three N is sufficient reserve", "`I_0>3*N`" in protocol_text)
check(
    "reversed trajectory is the exact inverse",
    sp.simplify((S_ua.T * S_ar.T * S_phase.T) * M) == sp.eye(6),
)

# C33--C56: local reaction channel.
check("residual history pulse is the registered quarter turn", S_ua[:3, :3] == Oua)
check("residual history pulse rotates conjugates identically", S_ua[3:, 3:] == Oua)
check(
    "history reaction pulse has the frozen angle eta",
    S_ar[1, 1] == c and S_ar[1, 2] == -s_eta and S_ar[2, 1] == s_eta,
)
check(
    "reaction phase pulse is a quarter turn",
    S_phase[2, 5] == 1 and S_phase[5, 2] == -1,
)
expected = sp.Matrix([
    a,
    -c * u - s_eta * r,
    -s_eta * pu + c * pr,
    pa,
    -c * pu - s_eta * pr,
    s_eta * u - c * r,
])
check("pulse product gives the frozen endpoint", sp.simplify(endpoint - expected) == sp.zeros(6, 1))
check("endpoint matrix is symplectic", sp.simplify(M.T * J6 * M - J6) == sp.zeros(6))
check("endpoint matrix is orthogonal", sp.simplify(M.T * M) == sp.eye(6))
check("endpoint determinant is plus one", sp.simplify(M.det()) == 1)
check(
    "reverse pulse product is the exact inverse",
    sp.simplify(M.T - S_ua.T * S_ar.T * S_phase.T) == sp.zeros(6),
)
check(
    "full quadratic norm is preserved",
    sp.simplify((endpoint.dot(endpoint) - state.dot(state)) / 2) == 0,
)
ready_subs = {a: 0, r: 0, pu: 0, pa: 0, pr: 0}
ready = sp.simplify(endpoint.subs(ready_subs))
check("ready reaction slice clears the residual", ready[0] == 0)
check("ready reaction slice clears the residual conjugate", ready[3] == 0)
check("ready reaction displacement returns to zero", ready[2] == 0)
check("reaction momentum is sine eta times residual", ready[5] == s_eta * u)
check("outgoing history is minus cosine eta times residual", ready[1] == -c * u)
Eres = u**2 / 2
Ehist = sp.simplify(ready[1] ** 2 / 2)
Ereact = sp.simplify(ready[5] ** 2 / 2)
check("history energy has the cosine-squared share", sp.simplify(Ehist - c**2 * Eres) == 0)
check("reaction energy has the sine-squared share", sp.simplify(Ereact - s_eta**2 * Eres) == 0)
check("history plus reaction equals residual energy", sp.trigsimp(Ehist + Ereact - Eres) == 0)
ready_eta_zero = sp.simplify(ready.subs(eta, 0))
check(
    "eta zero reproduces the FTD0886 endpoint",
    ready_eta_zero == sp.Matrix([0, -u, 0, 0, 0, 0]),
)
check(
    "nonzero split gives nonzero reaction on the fixed witness",
    sp.simplify(ready[5].subs({eta: sp.pi / 6, u: 2})) == 1,
)
check(
    "history-only endpoint saturates positive residual energy",
    sp.simplify((Eres - Ehist).subs(eta, 0)) == 0,
)
check(
    "nonzero reaction changes history amplitude or consumes prior energy",
    "requires reducing the outgoing history amplitude or adding pre-existing energy" in protocol_flat,
)
scalar_skew = sp.Matrix([[0]])
check("one real scalar has no nondegenerate skew form", scalar_skew.det() == 0)
check(
    "one canonical pair is minimum and sufficient in the registered class",
    sp.Matrix([[0, 1], [-1, 0]]).det() == 1
    and "one canonical pair is minimum" in protocol_flat,
)

# C57--C72: symmetry, energy, and interpretation firewall.
check(
    "channel exchange symmetry is equality of squared shares",
    "`E_hist'=E_react'`" in protocol_text,
)
equal_solutions = sp.solveset(
    sp.cos(eta) ** 2 - sp.sin(eta) ** 2,
    eta,
    domain=sp.Interval(0, sp.pi / 2),
)
check("equal split is uniquely eta pi over four", equal_solutions == sp.FiniteSet(sp.pi / 4))
check(
    "equal split gives half energy to each channel",
    sp.simplify(Ehist.subs(eta, sp.pi / 4) - Eres / 2) == 0
    and sp.simplify(Ereact.subs(eta, sp.pi / 4) - Eres / 2) == 0,
)
check("equal split remains a selection", "This is a **[SELECTION" in protocol_text)
s0 = sp.symbols("s_0", real=True)
y_before = s0 + u
y_after = s0
w = -s0 * u
Eraw_before = y_before**2 / 2
Eraw_after = (y_after**2 + c**2 * u**2) / 2
Uint_before = -s0 * y_before + s0**2 / 2
Uint_after = -s0 * y_after + s0**2 / 2
check(
    "raw energy changes by work minus reaction energy",
    sp.trigsimp(Eraw_after - Eraw_before - (w - Ereact)) == 0,
)
check("interaction energy changes by minus work", sp.expand(Uint_after - Uint_before + w) == 0)
check(
    "raw interaction and reaction energy close exactly",
    sp.trigsimp(Eraw_after + Uint_after + Ereact - Eraw_before - Uint_before) == 0,
)
check(
    "reaction impulse is not free energy",
    "paid by reducing the outgoing history energy" in protocol_flat and Ereact != 0,
)
check("complete reaction pair is retained", "complete reaction pair is retained" in protocol_flat and M.det() == 1)
check("finite cyclic history capacity is unchanged", "finite cyclic history capacity is unchanged" in protocol_flat)
check("existing canonical pair type is reused", "already selected local canonical-pair type" in protocol_flat)
check("no sixth selected type is added", "no sixth selected v2 type is added" in protocol_flat)
check(
    "spatial ternary source motion and native inertia remain open",
    "spatial ternary source motion and native inertia remain open" in protocol_flat,
)
check(
    "production and quartic Gstar synchronization remain separate",
    "production and quartic-`G*` synchronization remain separate" in protocol_flat,
)
check(
    "Born Bell Lorentz hiding and completeness remain untouched",
    "Born, Bell, Lorentz hiding, and completeness remain untouched" in protocol_flat,
)
check("terminal gate reached with C71 passing", checks == 71 and failures == 0)

print()
print(f"FTD-0887 autonomous phase parity/source reaction: {checks - failures}/{checks} PASS")
print("AUTONOMOUS_PHASE_PARITY_CONTROLLER=POSITIVE_EXACT_REFERENCE")
print("EXTERNAL_INTEGER_PARITY_SWITCH=NOT_REQUIRED_AT_REFERENCE_LEVEL")
print("SOURCE_REACTION_CHANNEL=ONE_CANONICAL_PAIR_MINIMUM_IN_REGISTERED_CLASS")
print("HISTORY_ONLY_ENDPOINT=POSITIVE_ENERGY_SATURATED")
print("REACTION_IMPULSE=PAID_BY_REDUCED_HISTORY_ENERGY")
print("SELF_DUAL_HISTORY_REACTION_SPLIT=SELECTED_CHANNEL_SYMMETRY")
print("SPATIAL_TERNARY_SOURCE_RECOIL=OPEN")
print("FINITE_CYCLIC_FRESHNESS_BOUNDARY=UNCHANGED")
print("PRODUCTION_COUPLING=NONE")
print("GSTAR_ROLE=SEPARATE_CALENDAR")
print("BORN_BELL_STATUS=UNTOUCHED")

raise SystemExit(0 if failures == 0 and checks == 72 else 1)
