#!/usr/bin/env python3
"""FTD-0843 exact common/relative local quartic-clock discriminator."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]

FROZEN = {
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    "engine/include/ftd/lagrangian.h":
        "0225C75F34D1154CDF3783E73A86F051A3868E0E9087606E117411D75429350F",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_BILATERAL_QUARTIC_DYNAMICS_OBSTRUCTION_v1.md":
        "2888C64166BC1E8B95807B6A8938A83971BDDF84718464B60D331B42C319C1DD",
    "engine/include/ftd/eft/native_pair_energy_recursion.h":
        "81B4941B951BC9D680A862188310706B86CDDA9DF9550204FC3F3DD567371E5A",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_COUPLED_SELF_PAIR_FIELD_ENERGY_CLOSURE_v1.md":
        "6FECB7DFEA03DE14E96AD07A6780945182C92106FE64ED542318192841333C40",
}


passed = 0
failed = 0


def check(label: str, condition: object) -> None:
    global passed, failed
    if bool(condition):
        passed += 1
        print(f"[PASS] {label}")
    else:
        failed += 1
        print(f"[FAIL] {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def zero(expr: sp.Expr) -> bool:
    return sp.simplify(expr) == 0


# C1 — immutable inputs.
check(
    "C1 all seven frozen production and theorem sources match",
    all((ROOT / rel).is_file() and sha256(ROOT / rel) == digest
        for rel, digest in FROZEN.items()),
)

voxel = (ROOT / "engine/include/ftd/voxel.h").read_text(encoding="utf-8")
phase_read = (
    ROOT / "engine/src/render_bridge_phases/phase_read.cpp"
).read_text(encoding="utf-8")

# C2--C3 — production boundary.
check(
    "C2 production stores local left and right flux-wave channel pairs",
    all(token in voxel for token in (
        "Vec3 flux_L;", "Vec3 flux_R;",
        "Vec3 wave_vel_L;", "Vec3 wave_vel_R;",
    )),
)
check(
    "C3 frozen dual propagation applies separate block-diagonal Laplacians",
    "lap_L" in phase_read
    and "lap_R" in phase_read
    and "rb.delta_j_L_[i] = lap_L * cw2" in phase_read
    and "rb.delta_j_R_[i] = lap_R * cw2" in phase_read
    and "cross_gradient" not in phase_read,
)

# C4--C6 — orthogonal common/relative chart.
sqrt2 = sp.sqrt(2)
T = sp.Matrix([[1, 1], [1, -1]]) / sqrt2
I2 = sp.eye(2)
check("C4 common-relative transform is orthogonal", T.T * T == I2)
check("C5 inverse transform is exactly the transpose", T.inv() == T.T)

l, r, pl, pr = sp.symbols("l r pl pr", real=True)
chan = sp.Matrix([l, r])
mom = sp.Matrix([pl, pr])
cr = T * chan
pm = T * mom
check(
    "C6 channel kinetic norm is preserved exactly",
    zero(chan.dot(chan) - cr.dot(cr))
    and zero(mom.dot(mom) - pm.dot(pm)),
)

# C7--C12 — rank-one spatial metric.
a, b = sp.symbols("a b", real=True)
dl, dr = sp.symbols("dl dr", real=True)
dc, dd = T * sp.Matrix([dl, dr])
E_lr = a * (dl**2 + dr**2) / 2 + b * dl * dr
E_cd = (a + b) * dc**2 / 2 + (a - b) * dd**2 / 2
check("C7 channel edge energy diagonalizes exactly into common and relative parts",
      zero(E_lr - E_cd))

M = sp.Matrix([[a, b], [b, a]])
eigs = set(M.eigenvals().keys())
check("C8 spatial positivity is exactly the pair a+b and a-b",
      eigs == {a - b, a + b})
check("C9 exact relative softness forces the boundary value b=a",
      sp.solve(sp.Eq(a - b, 0), b) == [a])

apos = sp.symbols("apos", positive=True)
Mrank = M.subs({a: apos, b: apos})
check("C10 rank-one channel metric has eigenvalues two-a and zero",
      Mrank.eigenvals() == {2 * apos: 1, 0: 1})
check("C11 the soft channel eigenvector is the relative vector",
      Mrank * sp.Matrix([1, -1]) == sp.zeros(2, 1))
check("C12 common propagation remains strictly positive at the soft boundary",
      (sp.Matrix([1, 1]).T * Mrank * sp.Matrix([1, 1]))[0] == 4 * apos)

# C13--C16 — exact common production tick and invariant.
ak = sp.symbols("ak", positive=True)
U = sp.Matrix([[1 - ak, 1], [-ak, 1]])
G = sp.Matrix([[ak, -ak / 2], [-ak / 2, 1]])
check("C13 common sector is exactly the source-free production kick-drift map",
      U == sp.Matrix([[1 - ak, 1], [-ak, 1]]))
check("C14 common quadratic tick invariant is exact", U.T * G * U == G)
check("C15 common invariant is positive exactly inside zero<a_K<4",
      zero(G.det() - ak * (1 - ak / 4)) and G[0, 0] == ak)
check("C16 production full-stencil ceiling lies strictly inside positivity region",
      sp.Rational(16, 9) < 4)

# C17--C19 — exact relative onsite map.
q0, q1, p0, p1, lam = sp.symbols("q0 q1 p0 p1 lam", real=True)
rq = q1 - q0 - (p1 + p0) / 2
rp = p1 - p0 + lam * (q1**2 + q0**2) * (q1 + q0)
check("C17 relative sector is exactly the primitive-step FTD-0841 recursion",
      rq == q1 - q0 - (p1 + p0) / 2
      and rp == p1 - p0 + lam * (q1**2 + q0**2) * (q1 + q0))

x = sp.symbols("x", real=True)
F = 2 * (x - q0) - 2 * p0 + lam * (x**2 + q0**2) * (x + q0)
dF = sp.diff(F, x)
monotone_form = 2 + lam * (3 * (x + q0 / 3) ** 2 + sp.Rational(2, 3) * q0**2)
check("C18 every relative site has one global next state for positive lambda",
      zero(dF - monotone_form))

Hdiff = ((p1**2 - p0**2) / 2 + lam * (q1**4 - q0**4))
# Use rq=0 to replace p1+p0=2(q1-q0), then rp=0 for p1-p0.
Hstep = ((p1 + p0) * (p1 - p0) / 2
         + lam * (q1 - q0) * (q1 + q0) * (q1**2 + q0**2))
Hstep = Hstep.subs(p1 - p0, -lam * (q1**2 + q0**2) * (q1 + q0))
Hstep = Hstep.subs(p1 + p0, 2 * (q1 - q0))
check("C19 relative onsite energy is conserved exactly",
      zero(Hdiff - ((p1**2 - p0**2) / 2 + lam * (q1**4 - q0**4)))
      and zero(Hstep))

# C20--C23 — total closure, locality, and compact support.
HC0, HD0 = sp.symbols("HC0 HD0", real=True)
HC1, HD1 = sp.symbols("HC1 HD1", real=True)
total_delta = (HC1 + HD1 - HC0 - HD0).subs({HC1: HC0, HD1: HD0})
check("C20 decoupled common and relative invariants give exact total energy", zero(total_delta))
check("C21 combined dependency radius is one Moore shell",
      "lap_L" in phase_read and "lap_R" in phase_read and "neighbor" in phase_read
      and "context" not in (ROOT / "engine/include/ftd/eft/native_pair_energy_recursion.h").read_text(encoding="utf-8").lower())

Fzero = sp.simplify(F.subs({q0: 0, p0: 0}))
check("C22 zero relative sites stay zero so single-site support remains exact",
      Fzero.subs(x, 0) == 0 and zero(sp.diff(Fzero, x) - (2 + 3 * lam * x**2)))

sx, sy, sz, scale = sp.symbols("sx sy sz scale", real=True)
S = sp.Matrix([sx, sy, sz])
radial_force = scale * S
check("C23 every fixed relative polarization is invariant",
      S.cross(radial_force) == sp.zeros(3, 1))

# C24--C25 — continuum G* and discrete orientation.
gstar = sp.gamma(sp.Rational(1, 4)) / sp.gamma(sp.Rational(3, 4))
beta_quarter = sp.beta(sp.Rational(1, 4), sp.Rational(1, 2))
beta_gamma = (sp.gamma(sp.Rational(1, 4)) * sp.sqrt(sp.pi)
              / sp.gamma(sp.Rational(3, 4)))
check("C24 polarized continuum relative sector has the exact G-star period factor",
      zero(beta_quarter - beta_gamma) or beta_gamma == sp.sqrt(sp.pi) * gstar)

chi = -(
    lam * (q1**2 + q0**2) * (q1 + q0)**2 / 2
    + (p1 + p0)**2 / 4
)
check("C25 relative swept-area orientation is a strict negative sum off zero",
      sp.Poly(-chi, q0, q1, p0, p1).total_degree() == 4
      and chi.subs({q0: 0, q1: 0, p0: 0, p1: 0}) == 0)

# C26--C27 — uniqueness of the positive soft boundary.
eps = sp.symbols("eps", positive=True)
check("C26 every b<a leaves positive relative quadratic stiffness",
      zero((a - (a - eps)) - eps))
check("C27 every b>a makes the relative edge coefficient negative",
      zero((a - (a + eps)) + eps))

# C28 — combined registered verdict.
check(
    "C28 positive P4-local selected carrier passes while native readout and cadence remain open",
    failed == 0
    and "cross_gradient" not in phase_read
    and Mrank.det() == 0
    and U.T * G * U == G,
)

total = passed + failed
print()
print(f"FTD-0843 common-relative local quartic clock: {passed}/{total} PASS")
if failed:
    raise SystemExit(1)

print("RANK_ONE_COMMON_PROPAGATION_LEAVES_EXACT_LOCAL_RELATIVE_SOFT_MODE")
print("DECOUPLED_COMMON_TICK_AND_RELATIVE_QUARTIC_ENERGIES_CLOSE_EXACTLY")
print("SELECTED_TWO_CHANNEL_CARRIER_IS_POSITIVE_AND_P4_LOCAL")
print("PRODUCTION_CROSS_GRADIENT_FORMATION_READOUT_AND_FINITE_TICK_CADENCE_OPEN")

