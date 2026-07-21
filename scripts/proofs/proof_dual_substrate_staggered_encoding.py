"""Verifier for EXPLR_DUAL_SUBSTRATE_STAGGERED_ENCODING.md (2026-07-17).

Adjudicates the staggered re-encoding conjecture for the engine's dual
substrate: (flux_L, flux_R) <-> (F, D) with F = J_L + J_R (observable) and
D = J_L - J_R (difference / chirality field).

Checks
------
T1  Re-encoding equivalence: an engine-faithful dual tick (independent 18-pt
    Laplacians + leapfrog + damping + 50/50 coupling split + gauss-style
    F-only correction + delta-split injection + weak L/R swap) evolved in
    (J_L, J_R) coordinates matches the same dynamics evolved in (F, D)
    coordinates to floating-point rounding, and F obeys the single-substrate
    update exactly.  Exact-arithmetic identity is a [THEOREM] (linear change
    of basis); the FP deviation is measured and reported honestly, including
    the bit-exactness booleans (expected False for independently evolved
    encodings -- rounding order differs).
T2  F-sector equivalence: the dual-run F trajectory matches an independent
    single-substrate run with the full G_C source (the engine's non-dual
    branch) to rounding.
T3  Chirality identity: voxel.h chirality_density == F_perp . D_perp
    (both the legacy z-projection and the velocity-projected variant).
T4  Checkerboard conjugation: eps L18[eps u] == L18_conj[u] bit-exactly,
    where eps = (-1)^(x+y+z) and L18_conj flips the sign of the 6 axis
    (odd-parity) terms only; plane-wave eigenvalues match -4*sigma18(k) and
    -4*sigma18(k+pi) respectively; sigma18(k+pi) != sigma18(k) generically;
    the FCC/edge (e2-only) stencil IS twist-invariant; the BCC corner
    average anticommutes with eps.
T5  Strong-form register refutation: evolving G = eps*D under the PLAIN 18-pt
    operator does NOT reproduce eps*(D evolved under the 18-pt operator);
    the mismatch is O(1) (witness printed).  Hence "chirality = the corner
    register component of one field under the single 18-pt dynamics" fails;
    only the kinematic relabeling survives.

Coefficient values (G_C, damping, delta, omega0) are immaterial to the
algebra; representative values are used.  Engine reference files:
  engine/src/render_bridge_phases/phase_read.cpp   (dual branch, 77-144)
  engine/src/render_bridge_phases/phase_write.cpp  (dual branch, 187-227)
  engine/src/poisson_solvers.cpp                   (half_corr split, 212-216)
  engine/src/injection.cpp                         (DELTA_APPROX split, 102-112)
  engine/src/transmutation_phases.cpp              (weak swap, 26-38)
  engine/include/ftd/voxel.h                       (chirality_density, 92-109)
"""

import numpy as np

L = 8
CW2 = 1.0 / 3.0          # C_WAVE^2 (CFL)
G_C = 0.0854245431       # sqrt(alpha) -- value immaterial
DAMP = 0.999             # 1 - DAMPING
DELTA = 0.9568           # engine DELTA_APPROX (master_quadratic.h:133)
OMEGA0 = 0.05            # de Broglie clock rate (diagonal term exercise)
TICKS = 50

rng = np.random.default_rng(20260717)

failures = []


def check(name, ok, detail=""):
    tag = "PASS" if ok else "FAIL"
    print(f"  {name}: {tag}" + (f"  ({detail})" if detail else ""))
    if not ok:
        failures.append(name)


# ---------------------------------------------------------------- operators
def roll(a, shift, axis):
    return np.roll(a, shift, axis=axis)


def lap18(f):
    """Engine 18-pt Laplacian: (1/3)*sum_axis + (1/6)*sum_face_diag - 4*center.
    f has shape (L,L,L) (one vector component)."""
    ax = sum(roll(f, s, a) for a in range(3) for s in (+1, -1))
    dg = sum(
        roll(roll(f, s1, a1), s2, a2)
        for (a1, a2) in ((0, 1), (0, 2), (1, 2))
        for s1 in (+1, -1)
        for s2 in (+1, -1)
    )
    return ax / 3.0 + dg / 6.0 - 4.0 * f


def lap18_conj(f):
    """Checkerboard-conjugated 18-pt operator: axis terms sign-flipped
    (odd |v|_1 = 1), face-diagonal terms unchanged (even |v|_1 = 2)."""
    ax = sum(roll(f, s, a) for a in range(3) for s in (+1, -1))
    dg = sum(
        roll(roll(f, s1, a1), s2, a2)
        for (a1, a2) in ((0, 1), (0, 2), (1, 2))
        for s1 in (+1, -1)
        for s2 in (+1, -1)
    )
    return -ax / 3.0 + dg / 6.0 - 4.0 * f


def lap_fcc(f):
    """Edge/FCC stencil (cuboctahedron): (1/12)*sum_face_diag - center.
    Pure e2 symbol -- even under every half-period twist."""
    dg = sum(
        roll(roll(f, s1, a1), s2, a2)
        for (a1, a2) in ((0, 1), (0, 2), (1, 2))
        for s1 in (+1, -1)
        for s2 in (+1, -1)
    )
    return dg / 12.0 - f


def bcc_avg(f):
    """Corner average (stella octangula vertex neighbors): (1/8)*sum_corners.
    Pure e3 symbol -- odd under the full checkerboard twist."""
    cr = sum(
        roll(roll(roll(f, s1, 0), s2, 1), s3, 2)
        for s1 in (+1, -1)
        for s2 in (+1, -1)
        for s3 in (+1, -1)
    )
    return cr / 8.0


def vlap(op, F):
    """Apply scalar operator to a vector field of shape (L,L,L,3)."""
    return np.stack([op(F[..., c]) for c in range(3)], axis=-1)


def grad_s(s):
    """Central-difference gradient of the state field (phase_read coupling)."""
    g = np.zeros(s.shape + (3,))
    for a in range(3):
        g[..., a] = (roll(s, -1, a) - roll(s, +1, a)) * 0.5
    return g


# --------------------------------------------------------------- scenario
state = np.zeros((L, L, L))
SITE_A = (2, 2, 2)   # +1 particle (delta-split injection, weak swap target)
SITE_B = (5, 5, 5)   # -1 particle
state[SITE_A] = 1
state[SITE_B] = -1
GS = grad_s(state)                     # static states => static source
CLOCK_MASK = (state != 0)[..., None]   # de Broglie clock at manifested sites
VOID_MASK = (state == 0)[..., None]    # gauss correction gated to void sites

phi = rng.standard_normal((L, L, L))
for _ in range(3):                     # smooth it a little
    phi = phi + 0.5 * lap18(phi)
GAUSS_CORR = grad_s(phi) * 0.01        # mock grad(phi) correction field

J0_A = np.array([0.30, -0.10, 0.20])   # injected flux at SITE_A (state +1)
J0_B = np.array([-0.20, 0.15, 0.10])   # injected flux at SITE_B (state -1)


def dual_tick(JL, JR, WL, WR, tick):
    """Engine-faithful dual tick (default integrator, dt=1).
    phase_read: dJ_X = cw2*lap18(J_X) + (G_C/2)*grad_s - omega0^2*J_X@matter
    phase_write: W_X += dJ_X; J_X += W_X; damp both by the same factor.
    Rule 3 (mock): F-only gauss correction split half/half at void sites.
    Rule 6 (tick 25): weak swap at SITE_A (L <-> R, flux and wave_vel)."""
    dL = CW2 * vlap(lap18, JL) + (G_C * 0.5) * GS - OMEGA0**2 * JL * CLOCK_MASK
    dR = CW2 * vlap(lap18, JR) + (G_C * 0.5) * GS - OMEGA0**2 * JR * CLOCK_MASK
    # engine order (phase_write.cpp:204-222): W += dJ; J += W; then damp both
    WL = WL + dL
    WR = WR + dR
    JL = (JL + WL) * DAMP
    JR = (JR + WR) * DAMP
    WL = WL * DAMP
    WR = WR * DAMP
    if tick % 10 == 9:                 # mock gauss projection event
        JL = JL - 0.5 * GAUSS_CORR * VOID_MASK
        JR = JR - 0.5 * GAUSS_CORR * VOID_MASK
    if tick == 25:                     # weak transmutation at SITE_A
        JL[SITE_A], JR[SITE_A] = JR[SITE_A].copy(), JL[SITE_A].copy()
        WL[SITE_A], WR[SITE_A] = WR[SITE_A].copy(), WL[SITE_A].copy()
    return JL, JR, WL, WR


def fd_tick(F, D, VF, VD, tick):
    """The same dynamics in (F, D) coordinates.
    F: single-substrate update with the FULL G_C source.
    D: source-free wave equation under the identical operator.
    Gauss: F-only.  Weak swap: D -> -D at the site (F untouched)."""
    dF = CW2 * vlap(lap18, F) + G_C * GS - OMEGA0**2 * F * CLOCK_MASK
    dD = CW2 * vlap(lap18, D) - OMEGA0**2 * D * CLOCK_MASK
    VF = VF + dF
    VD = VD + dD
    F = (F + VF) * DAMP
    D = (D + VD) * DAMP
    VF = VF * DAMP
    VD = VD * DAMP
    if tick % 10 == 9:
        F = F - GAUSS_CORR * VOID_MASK
    if tick == 25:
        D[SITE_A] = -D[SITE_A]
        VD[SITE_A] = -VD[SITE_A]
    return F, D, VF, VD


def single_tick(J, W, tick):
    """Engine single-substrate branch (full G_C source), for T2.
    Same F-sector event schedule as the dual runs: the gauss correction acts
    on the observable in both modes (weak swap is D-only, F-invisible)."""
    dJ = CW2 * vlap(lap18, J) + G_C * GS - OMEGA0**2 * J * CLOCK_MASK
    W = W + dJ
    J = (J + W) * DAMP
    W = W * DAMP
    if tick % 10 == 9:
        J = J - GAUSS_CORR * VOID_MASK
    return J, W

print("=" * 72)
print("T1 -- re-encoding equivalence: (J_L, J_R) vs (F, D), %d ticks, L=%d" % (TICKS, L))
print("=" * 72)

# Initial condition: smooth random background split 50/50 (D=0, V*=0), then
# the injection.cpp delta-split at the two particle sites.
BG = rng.standard_normal((L, L, L, 3)) * 0.02
for c in range(3):
    for _ in range(2):
        BG[..., c] = BG[..., c] + 0.4 * lap18(BG[..., c])

JL = BG * 0.5
JR = BG * 0.5
JL[SITE_A] = J0_A * (1.0 + DELTA) * 0.5   # state > 0: L major
JR[SITE_A] = J0_A * (1.0 - DELTA) * 0.5
JL[SITE_B] = J0_B * (1.0 - DELTA) * 0.5   # state < 0: R major
JR[SITE_B] = J0_B * (1.0 + DELTA) * 0.5
WL = np.zeros_like(JL)
WR = np.zeros_like(JR)

F = JL + JR
D = JL - JR
VF = WL + WR
VD = WL - WR

Js = F.copy()      # single-substrate comparison run (T2)
Ws = VF.copy()

for t in range(TICKS):
    JL, JR, WL, WR = dual_tick(JL, JR, WL, WR, t)
    F, D, VF, VD = fd_tick(F, D, VF, VD, t)
    Js, Ws = single_tick(Js, Ws, t)

sumJ = JL + JR
difJ = JL - JR
scale = np.max(np.abs(sumJ)) or 1.0
devF = np.max(np.abs(sumJ - F))
devD = np.max(np.abs(difJ - D))
devVF = np.max(np.abs((WL + WR) - VF))
devVD = np.max(np.abs((WL - WR) - VD))
bitF = bool(np.array_equal(sumJ, F))
bitD = bool(np.array_equal(difJ, D))

print(f"  field scale max|F| = {scale:.3e}")
print(f"  max |(J_L+J_R) - F|      = {devF:.3e}   (rel {devF/scale:.3e})")
print(f"  max |(J_L-J_R) - D|      = {devD:.3e}   (rel {devD/scale:.3e})")
print(f"  max |(W_L+W_R) - V_F|    = {devVF:.3e}")
print(f"  max |(W_L-W_R) - V_D|    = {devVD:.3e}")
print(f"  bit-exact F: {bitF}   bit-exact D: {bitD}   (expected False -- rounding order)")
check("T1a sum-channel rounding-level", devF / scale < 1e-12, f"rel {devF/scale:.2e}")
check("T1b diff-channel rounding-level", devD / scale < 1e-12, f"rel {devD/scale:.2e}")
check("T1c velocity channels rounding-level", max(devVF, devVD) / scale < 1e-12)

print()
print("=" * 72)
print("T2 -- F-sector == single-substrate dynamics (full G_C source)")
print("=" * 72)
devS = np.max(np.abs(Js - F))
print(f"  max |J_single - F_dual| = {devS:.3e}   (rel {devS/scale:.3e})")
print(f"  bit-exact: {bool(np.array_equal(Js, F))}")
check("T2 single-substrate match", devS / scale < 1e-12, f"rel {devS/scale:.2e}")

print()
print("=" * 72)
print("T3 -- chirality identity: chirality_density == F_perp . D_perp")
print("=" * 72)
N = 100000
FL = rng.standard_normal((N, 3))
DD = rng.standard_normal((N, 3))
Lv = (FL + DD) / 2.0
Rv = (FL - DD) / 2.0
# legacy z-projection form (voxel.h:105-107)
chi_engine = (Lv[:, 0] ** 2 + Lv[:, 1] ** 2) - (Rv[:, 0] ** 2 + Rv[:, 1] ** 2)
chi_fd = FL[:, 0] * DD[:, 0] + FL[:, 1] * DD[:, 1]
err_z = np.max(np.abs(chi_engine - chi_fd) / (1.0 + np.abs(chi_engine)))
# velocity-projected form (voxel.h:93-101)
vel = rng.standard_normal((N, 3))
e = vel / np.linalg.norm(vel, axis=1, keepdims=True)
Ld = np.sum(Lv * e, axis=1)
Rd = np.sum(Rv * e, axis=1)
chi_engine_v = (np.sum(Lv**2, axis=1) - Ld**2) - (np.sum(Rv**2, axis=1) - Rd**2)
Fd = np.sum(FL * e, axis=1)
Dd = np.sum(DD * e, axis=1)
chi_fd_v = np.sum(FL * DD, axis=1) - Fd * Dd
err_v = np.max(np.abs(chi_engine_v - chi_fd_v) / (1.0 + np.abs(chi_engine_v)))
print(f"  z-projection form:        max rel err = {err_z:.3e}")
print(f"  velocity-projected form:  max rel err = {err_v:.3e}")
check("T3a chi = F_perp.D_perp (z-projection)", err_z < 1e-10)
check("T3b chi = F_perp.D_perp (velocity-projected)", err_v < 1e-10)

print()
print("=" * 72)
print("T4 -- checkerboard conjugation of the 18-pt operator")
print("=" * 72)
x, y, z = np.meshgrid(np.arange(L), np.arange(L), np.arange(L), indexing="ij")
eps = ((-1.0) ** (x + y + z))
u = rng.standard_normal((L, L, L))

conj_applied = eps * lap18(eps * u)
direct_conj = lap18_conj(u)
bit_conj = bool(np.array_equal(conj_applied, direct_conj))
print(f"  eps*L18[eps u] == L18_conj[u] bit-exact: {bit_conj}")
check("T4a conjugation identity (bit-exact)", bit_conj)

wit = np.max(np.abs(lap18(u) - lap18_conj(u)))
print(f"  max |L18[u] - L18_conj[u]| = {wit:.3e}  (O(1) witness: operator NOT twist-invariant)")
check("T4b 18-pt operator not checkerboard-invariant", wit > 0.1, f"witness {wit:.2e}")

# plane-wave eigenvalue check on the dual grid


def sigma18(k):
    c = np.cos(k)
    e1 = c.sum()
    e2 = c[0] * c[1] + c[0] * c[2] + c[1] * c[2]
    return 1.0 - e1 / 6.0 - e2 / 6.0


LK = 16
xk, yk, zk = np.meshgrid(np.arange(LK), np.arange(LK), np.arange(LK), indexing="ij")


def lap18_L(f):
    ax = sum(np.roll(f, s, axis=a) for a in range(3) for s in (+1, -1))
    dg = sum(
        np.roll(np.roll(f, s1, axis=a1), s2, axis=a2)
        for (a1, a2) in ((0, 1), (0, 2), (1, 2))
        for s1 in (+1, -1)
        for s2 in (+1, -1)
    )
    return ax / 3.0 + dg / 6.0 - 4.0 * f


max_sym_err = 0.0
max_conj_err = 0.0
min_sym_gap = np.inf
for trial in range(6):
    n = rng.integers(1, LK - 1, size=3)
    k = 2.0 * np.pi * n / LK
    w = np.cos(k[0] * xk + k[1] * yk + k[2] * zk + 0.37)
    lw = lap18_L(w)
    mask = np.abs(w) > 0.3
    lam = (lw[mask] / w[mask])
    max_sym_err = max(max_sym_err, np.max(np.abs(lam - (-4.0 * sigma18(k)))))
    epsK = ((-1.0) ** (xk + yk + zk))
    lcw = epsK * lap18_L(epsK * w)
    lamc = (lcw[mask] / w[mask])
    kpi = k + np.pi
    max_conj_err = max(max_conj_err, np.max(np.abs(lamc - (-4.0 * sigma18(kpi)))))
    min_sym_gap = min(min_sym_gap, abs(sigma18(k) - sigma18(kpi)))
print(f"  plane-wave eigenvalue vs -4*sigma18(k):        max err = {max_sym_err:.3e}")
print(f"  conjugated eigenvalue vs -4*sigma18(k+pi):     max err = {max_conj_err:.3e}")
print(f"  min |sigma18(k) - sigma18(k+pi)| over trials   = {min_sym_gap:.3e}")
check("T4c symbol matches -4*sigma18(k)", max_sym_err < 1e-9)
check("T4d conjugated symbol = -4*sigma18(k+pi)", max_conj_err < 1e-9)
check("T4e sigma18(k+pi) != sigma18(k) generically", min_sym_gap > 1e-3)

fcc_inv = bool(np.array_equal(eps * lap_fcc(eps * u), lap_fcc(u)))
bcc_anti = bool(np.array_equal(eps * bcc_avg(eps * u), -bcc_avg(u)))
print(f"  FCC/edge (e2) stencil twist-invariant (bit-exact): {fcc_inv}")
print(f"  BCC corner average anticommutes with eps (bit-exact): {bcc_anti}")
check("T4f FCC stencil twist-invariant", fcc_inv)
check("T4g BCC corner average anticommutes", bcc_anti)

print()
print("=" * 72)
print("T5 -- strong-form register reading refuted")
print("=" * 72)
# One leapfrog step: G = eps*D under PLAIN L18 vs eps*(D under L18).
D5 = rng.standard_normal((L, L, L)) * 0.1
V5 = np.zeros_like(D5)
D_next = D5 + (V5 + CW2 * lap18(D5))
G5 = eps * D5
VG = eps * V5
G_next_plain = G5 + (VG + CW2 * lap18(G5))
mismatch = np.max(np.abs(G_next_plain - eps * D_next))
print(f"  one-step mismatch |plain-evolved G  -  eps*(evolved D)| = {mismatch:.3e}  (O(1))")
print("  => G = eps*D obeys the CONJUGATED operator (symbol sigma18(k+pi)),")
print("     not the engine operator; the checkerboard weighting is a spectral")
print("     relabeling k -> k+pi, not a dynamics-compatible component of one field.")
check("T5 strong form fails (O(1) witness)", mismatch > 1e-3, f"witness {mismatch:.2e}")

print()
n_total = 15
n_fail = len(failures)
print("=" * 72)
print(f"RESULT: {n_total - n_fail}/{n_total} PASS" + ("" if n_fail == 0 else f"  FAILURES: {failures}"))
print("=" * 72)
raise SystemExit(0 if n_fail == 0 else 1)
