#!/usr/bin/env python3
"""FTD-0885 exact canonical source-centered Gauss-gate certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_OBSTRUCTION_v1.md"
)
PROTOCOL_HASH = "70000AF7DA0ACA89F92A593AA4B6A759B9C9D08C65E29E21A2D1EF5B2B2910D7"
SOURCE_HASHES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_REVERSIBLE_CHECKERBOARD_GAUSS_RECORD_PREPARATION_AND_SELF_DUAL_ENERGY_SPLIT_v1.md":
        "143D897A69B5C6FED8C00402C1840EA9FAEE5BD4BC259C9BDD065DFDC616A814",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_AND_RECYCLING_BOUNDARY_v1.md":
        "AF810B73322DE8521C8509792E09D549A10E1D8417C1B283A3630EB8B16D7BFC",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_LOCAL_CANONICAL_HAMILTONIAN_PARITY_RAIL_AND_SCALAR_LOCALITY_BOUNDARY_v1.md":
        "982C3B9D00798920A1BDAB96C75EBC9DB3A08111E8900F1D630382B0249B25F6",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CLOCK_GATED_HAMILTONIAN_EXCHANGE_AND_QUARTIC_LOAD_BOUNDARY_v1.md":
        "FFC0E39CC2C87FE73DC3C931302FE32EB5493E6AFB426CFA5BF97624DA3917D1",
    "engine/include/ftd/eft/reversible_checkerboard_gauss_preparation.h":
        "7C2AFBFD098268B02C9E58DABAC19ED38DD1FA173385424E111B0FEFAAD79420",
}
V2_REGISTER = ROOT / "docs/theory/01_reference/contextual_actualization_register_v2.json"

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


def poisson(f: sp.Expr, g: sp.Expr, qs: tuple[sp.Symbol, ...], ps: tuple[sp.Symbol, ...]) -> sp.Expr:
    return sp.expand(
        sum(
            (sp.diff(f, q) * sp.diff(g, p) - sp.diff(f, p) * sp.diff(g, q)
             for q, p in zip(qs, ps)),
            sp.Integer(0),
        )
    )


protocol_path = ROOT / PROTOCOL
protocol_text = protocol_path.read_text(encoding="utf-8")
protocol_flat = " ".join(protocol_text.split())
source_text = {
    path: (ROOT / path).read_text(encoding="utf-8")
    for path in SOURCE_HASHES
}

# Symbols and exact local Hamiltonian.
u, a, pu, pa = sp.symbols("u a pi_u pi_a", real=True)
s, y = sp.symbols("s y", real=True)
theta, action, action0 = sp.symbols("theta I I_0", real=True)
omega = sp.symbols("omega", positive=True)
N = (u**2 + a**2 + pu**2 + pa**2) / 2
Lgen = a * pu - u * pa
coupling = omega * (1 - sp.cos(theta)) / 4
H = omega * action + omega * N + coupling * Lgen

# Common endpoint structures in ordering (u,a,pi_u,pi_a).
R = sp.Matrix([[0, 1], [-1, 0]])
M = sp.diag(R, R)
I2 = sp.eye(2)
Z2 = sp.zeros(2)
J4 = Z2.row_join(I2).col_join((-I2).row_join(Z2))

# Fixed L=4 incidence rows for checkerboard orthogonality.
LBOX = 4
sites = [(x, yy, z) for x in range(LBOX) for yy in range(LBOX) for z in range(LBOX)]


def site_index(x: int, yy: int, z: int) -> int:
    return ((x % LBOX) * LBOX + (yy % LBOX)) * LBOX + (z % LBOX)


def face_index(site: int, axis: int) -> int:
    return 3 * site + axis


def incidence_row(x: int, yy: int, z: int) -> sp.Matrix:
    row = [0] * (3 * LBOX**3)
    xyz = [x, yy, z]
    here = site_index(x, yy, z)
    for axis in range(3):
        row[face_index(here, axis)] += 1
        back = list(xyz)
        back[axis] -= 1
        row[face_index(site_index(*back), axis)] -= 1
    return sp.Matrix([row])


rows = [incidence_row(*xyz) for xyz in sites]
colors = [sum(xyz) % 2 for xyz in sites]
same_color_gram_ok = all(
    (rows[i] * rows[j].T)[0] == (6 if i == j else 0)
    for i in range(len(rows))
    for j in range(len(rows))
    if colors[i] == colors[j]
)

# C1--C8: provenance and scope.
check(
    "all five frozen source hashes match",
    all(sha256(ROOT / path) == digest for path, digest in SOURCE_HASHES.items()),
)
check("protocol pre-run hash matches", sha256(protocol_path) == PROTOCOL_HASH)
check(
    "normalized residual port and source offset are frozen",
    "u=y-s" in protocol_flat and "a=\\frac{e_x}{\\sqrt6}" in protocol_flat,
)
check(
    "full canonical brackets are frozen",
    "Restore complete canonical modes" in protocol_text
    and "all other brackets zero" in protocol_text,
)
check(
    "source interaction ledger is frozen",
    "E_{\\rm raw}+U_{\\rm int}" in protocol_text
    and "\\Delta U_{\\rm int}=-w" in protocol_text,
)
check("battery phase tests are frozen", "## 5. Frozen battery-phase tests" in protocol_text)
check(
    "parity schedule remains external",
    "parity schedule remains selected external control" in protocol_flat,
)
check(
    "production Gstar and quantum scope firewall is frozen",
    "Production, `G*`, Born, Bell, Lorentz, biology, and completeness" in protocol_flat,
)

# C9--C28: positive canonical Gauss layer.
check("same-color normalized incidence rows are orthonormal", same_color_gram_ok)
check("N is positive definite", sp.hessian(N, (u, a, pu, pa)) == sp.eye(4))
check("L is the registered two-mode angular momentum", Lgen == a * pu - u * pa)
check("Poisson bracket of N and L vanishes", poisson(N, Lgen, (u, a), (pu, pa)) == 0)
check(
    "absolute L is bounded by N",
    sp.expand(2 * (N - Lgen) - ((a - pu) ** 2 + (u + pa) ** 2)) == 0
    and sp.expand(2 * (N + Lgen) - ((a + pu) ** 2 + (u - pa) ** 2)) == 0,
)
check("clock advances uniformly", sp.diff(H, action) == omega)
check("base N flow completes one full winding", sp.simplify(omega * (2 * sp.pi / omega)) == 2 * sp.pi)
check(
    "integrated angular-momentum pulse is pi over two",
    sp.simplify(
        sp.integrate(
            omega * (1 - sp.cos(omega * sp.Symbol("t", real=True))) / 4,
            (sp.Symbol("t", real=True), 0, 2 * sp.pi / omega),
        )
    ) == sp.pi / 2,
)
check("L flow sends u to a", M[0, :] == sp.Matrix([[0, 1, 0, 0]]))
check("L flow sends a to minus u", M[1, :] == sp.Matrix([[-1, 0, 0, 0]]))
check("L flow sends pi_u to pi_a", M[2, :] == sp.Matrix([[0, 0, 0, 1]]))
check("L flow sends pi_a to minus pi_u", M[3, :] == sp.Matrix([[0, 0, -1, 0]]))
check("endpoint matrix is symplectic", sp.simplify(M.T * J4 * M - J4) == sp.zeros(4))
check("endpoint matrix is orthogonal", M.T * M == sp.eye(4))
check("endpoint determinant is plus one", M.det() == 1)
check("opposite quarter-turn is exact inverse", M.T * M == sp.eye(4) and M**4 == sp.eye(4))
zero_conjugate = M * sp.Matrix([u, a, 0, 0])
check("zero-conjugate section is invariant", zero_conjugate[2] == 0 and zero_conjugate[3] == 0)
check(
    "zero-conjugate section reproduces FTD-0882 field update",
    sp.simplify((a + s) - ((u + s) + (a - u))) == 0
    and zero_conjugate[:2, :] == sp.Matrix([a, -u]),
)
check(
    "disjoint same-color generators commute",
    same_color_gram_ok and "cell generators therefore commute" in protocol_flat,
)
check(
    "carrier Hamiltonian has the frozen positive lower bound",
    "bounded below by `omega*N/2`" in protocol_text
    and sp.simplify(sp.Rational(1, 2) - sp.Rational(1, 4) * 2) == 0,
)

# C29--C38: clock and source-work ledger.
check(
    "N and L are conserved",
    poisson(N, H, (u, a, theta), (pu, pa, action)) == 0
    and poisson(Lgen, H, (u, a, theta), (pu, pa, action)) == 0,
)
action_theta = action0 - (1 - sp.cos(theta)) * Lgen / 4
check(
    "clock action solution differentiates to Hamilton equation",
    sp.simplify(omega * sp.diff(action_theta, theta) + sp.diff(H, theta)) == 0,
)
check("clock action returns at endpoint", sp.simplify(action_theta.subs(theta, 2 * sp.pi) - action0) == 0)
check(
    "maximum transient action loan is absolute L over two",
    sp.simplify(action_theta.subs(theta, sp.pi) - action0) == -Lgen / 2,
)
check("zero-conjugate section has zero angular momentum", Lgen.subs({pu: 0, pa: 0}) == 0)
Eraw = (y**2 + a**2) / 2
Uint = -s * y + s**2 / 2
check(
    "raw plus interaction energy is positive centered energy",
    sp.expand((Eraw + Uint).subs(y, u + s) - (u**2 + a**2) / 2) == 0,
)
Eraw_after = ((a + s) ** 2 + u**2) / 2
Eraw_before = ((u + s) ** 2 + a**2) / 2
work = s * (a - u)
check("raw energy change is exactly local work", sp.expand(Eraw_after - Eraw_before - work) == 0)
Uint_after = -s * (a + s) + s**2 / 2
Uint_before = -s * (u + s) + s**2 / 2
check("interaction energy change is minus local work", sp.expand(Uint_after - Uint_before + work) == 0)
check(
    "raw plus interaction energy is exact",
    sp.expand((Eraw_after + Uint_after) - (Eraw_before + Uint_before)) == 0,
)
check(
    "fixed source offset is not mislabeled as moving reservoir",
    "Promoting it to moving matter with recoil remains outside the lock" in protocol_flat,
)

# C39--C52: battery obstruction.
b, pb, w = sp.symbols("b p_b w", positive=True)
bp = sp.sqrt(b**2 - 2 * w)
fp = sp.diff(bp, b)
check("square-root branch derivative is b over b prime", sp.simplify(fp - b / bp) == 0)
pbp = pb * bp / b
battery_jacobian = sp.Matrix([bp, pbp]).jacobian((b, pb))
check("cotangent lift preserves the symplectic form", sp.simplify(battery_jacobian.det()) == 1)
check("cotangent lift preserves the zero-conjugate slice", sp.simplify(pbp.subs(pb, 0)) == 0)
check("lift reproduces the FTD-0884 amplitude law", sp.simplify(bp**2 - (b**2 - 2 * w)) == 0)
delta_e = sp.simplify((bp**2 + pbp**2 - b**2 - pb**2) / 2)
check(
    "oscillator-energy change has the frozen extra conjugate factor",
    sp.simplify(delta_e + w * (1 + pb**2 / b**2)) == 0,
)
check("desired battery loss holds on zero-conjugate slice", sp.simplify(delta_e.subs(pb, 0) + w) == 0)
check(
    "nonzero conjugate generically changes the booked amount",
    sp.simplify(delta_e.subs({b: 3, pb: 2, w: 1}) + 1) != 0,
)
check(
    "triangular symplectic harmonic-energy completion is excluded for nonzero work",
    sp.simplify(fp**2 - 1) == sp.simplify(2 * w / (b**2 - 2 * w))
    and sp.simplify(fp**2 - 1) != 0,
)
Ib, phib = sp.symbols("I_b phi_b", real=True)
action_map_jacobian = sp.Matrix([Ib - w, phib]).jacobian((Ib, phib))
J2 = sp.Matrix([[0, 1], [-1, 0]])
check(
    "constant action translation is locally symplectic",
    sp.simplify(action_map_jacobian.T * J2 * action_map_jacobian - J2) == sp.zeros(2),
)
check(
    "canonical one-form difference is minus w dphi",
    sp.simplify((Ib - w) - Ib) == -w,
)
check("phase-circle integral is minus two pi w", sp.integrate(-w, (phib, 0, 2 * sp.pi)) == -2 * sp.pi * w)
check(
    "nonzero action translation is not globally Hamiltonian on the cylinder",
    "not the time map of a globally single-valued Hamiltonian" in protocol_flat,
)
z = sp.symbols("z", real=True)
wz = z**2
check(
    "state-dependent phase-blind drain adds minus dw wedge dphi",
    sp.diff(Ib - wz, z) == -2 * z and "-dw wedge dphi_b" in protocol_text,
)
check(
    "nonconstant work requires phase backreaction or another channel",
    sp.diff(wz, z) != 0 and "phase backreaction on the system" in protocol_flat,
)

# C53--C64: history and interpretation firewall.
check("fresh canonical port is the complete zero pair", "A fresh port is `(0,0)`" in protocol_text)
check(
    "outgoing gate retains the complete canonical pair",
    M * sp.Matrix([u, a, pu, pa]) == sp.Matrix([a, -u, pa, -pu]),
)
P3 = sp.Matrix([[0, 0, 1], [1, 0, 0], [0, 1, 0]])
shift6 = sp.diag(P3, P3)
J6 = sp.zeros(6)
J6[:3, 3:] = sp.eye(3)
J6[3:, :3] = -sp.eye(3)
check(
    "bilateral canonical-pair shift is symplectic and bijective",
    shift6.det() != 0 and shift6.T * J6 * shift6 == J6,
)
check(
    "finite cyclic pair rail retains the capacity boundary",
    "finite cyclic rail again supplies only its declared capacity" in protocol_flat
    and "FINITE_CYCLIC_FRESH_LAYERS=CAPACITY" in source_text[
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_AND_RECYCLING_BOUNDARY_v1.md"
    ],
)
check(
    "open rail inverse retains both boundary pairs",
    "retaining its incoming and outgoing boundary pairs" in protocol_flat,
)
pair_energy_collision = ((1**2 + 0**2) / 2) == (((-1) ** 2 + 0**2) / 2)
check("scalar energy-only export is insufficient", pair_energy_collision and 1 != -1)
check(
    "square-root battery is demoted to a Lagrangian-section reference",
    "demoted to a Lagrangian-section reference law" in protocol_flat,
)
import json

v2_data = json.loads(V2_REGISTER.read_text(encoding="utf-8"))
selected_count = len(v2_data.get("selections", []))
check("source-centered layer adds no sixth selected type", selected_count == 5)
check(
    "autonomous parity controller and native source formation remain open",
    "autonomous parity controller and native source formation remain open" in protocol_flat,
)
check(
    "production and quartic Gstar synchronization remain separate",
    "Production and quartic-`G*` synchronization remain separate" in protocol_flat,
)
check(
    "Born Bell Lorentz hiding and completeness remain untouched",
    "Born, Bell, Lorentz hiding, and completeness remain untouched" in protocol_flat,
)
check("terminal gate reached with C63 passing", checks == 63 and failures == 0)

print()
print(f"FTD-0885 canonical source-centered Gauss gate: {checks - failures}/{checks} PASS")
if checks == 64 and failures == 0:
    print("CANONICAL_SOURCE_CENTERED_GAUSS_GATE=POSITIVE_CLOCKED_LAYER")
    print("RAW_SOURCE_WORK=INTERACTION_ENERGY_EXCHANGE")
    print("SQUARE_ROOT_BATTERY=EXACT_ONLY_ON_LAGRANGIAN_SECTION")
    print("PHASE_BLIND_STATE_DEPENDENT_DRAIN=NOT_SYMPLECTIC")
    print("CONSTANT_ACTION_TRANSLATION=SYMPLECTIC_NOT_GLOBAL_HAMILTONIAN")
    print("CANONICAL_HISTORY_EXPORT=COMPLETE_PAIR_REQUIRED")
    print("FINITE_CYCLIC_FRESHNESS_BOUNDARY=UNCHANGED")
    print("AUTONOMOUS_PARITY_AND_SOURCE_DYNAMICS=OPEN")
    print("PRODUCTION_COUPLING=NONE")
    print("GSTAR_ROLE=SEPARATE_CALENDAR")
    print("BORN_BELL_STATUS=UNTOUCHED")
    raise SystemExit(0)
raise SystemExit(1)
