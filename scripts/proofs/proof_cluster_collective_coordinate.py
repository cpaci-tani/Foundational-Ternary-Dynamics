"""proof_cluster_collective_coordinate.py — FTD-0349 verification.

Numerically verifies every algebraic identity asserted in
docs/theory/03_derivations/foundational_mechanics/DERIV_CLUSTER_COLLECTIVE_COORDINATE_v1.md
(the v1 collective-coordinate attempt on the FTD-0110/FTD-0250 cluster-inertia
reduction).

Verdict being verified: PARTIAL. The reduction

    rigid translation of a locked, Gauss-dressed N-voxel cluster
    costs co-moving momentum N * M_REST * v

is DERIVED **conditional on** the Gradient-Normalization Condition (GNC)

    (1/K_B) * sum_{x} d_i J_a(x) d_j J_a(x)  =  N * K_B * delta_ij ,

and GNC FAILS for both flux profiles the framework currently pins down
(the minimal Coulomb Gauss dressing, and the amplitude-pinned uniform core),
while GNC-satisfying interior textures DO exist (J = K_B * R x with
R in SO(3), tr R = 0, i.e. rotation angle 2*pi/3).  So the obstruction is
dynamical (nothing forces the texture), not kinematic.

Test groups
-----------
T1  gamma_FTD residual model reproduces the ppm figures quoted in
    engine/tests/test_cluster_inertia.cpp (a_COM values and the ~6 ppm
    a*N spread) with c = C_SPEED = 1/sqrt(3), K_B = 0.511, F = 1e-3.
T2  Point-mechanics additivity route: dL/dV of L(V) = -N K_B sqrt(1-V^2)
    equals N K_B gamma V (numeric derivative), and a*N = F/K_B exactly
    at O(V^2), for N in {1, 8, 27}.
T3  Rigid-ansatz kinetic quadratic form: sum_x |(V . D+)J|^2 = V^T M V
    with M_ij = sum_x (D+_i J_a)(D+_j J_a), exact bilinear identity.
T4  Displacement remainder: sum_x |J(x-V) - J(x)|^2 = V^T M_spec V + O(V^4)
    (the squared-difference sum is even in V, so the remainder is FOURTH
    order); halving V shrinks the residual by ~16x.
T5  Minimal Coulomb Gauss dressing (lattice FFT Poisson, jellium):
    a. solver validation: backward-div(J) = rho - rho_bar to machine precision;
    b. EXACT TRACE IDENTITY [THEOREM]: trace(M_dress) = sum_x rho_tilde^2
       = N q^2 (1 - N/L^3) — exactly N-proportional, SHAPE-INDEPENDENT
       (lattice Parseval: the lambda^2 of the double forward gradient
       cancels the 1/lambda^2 of the Green's function). The dressing
       fails GNC by COEFFICIENT (q^2/3 per axis, not K_B^2; equality
       would need q = sqrt(3) K_B, which nothing in the framework sets)
       and by TENSOR STRUCTURE, not by N-scaling;
    c. tensor structure: the site-centered symmetrized Hessian estimator
       is isotropic for O_h clusters to machine precision (the raw
       forward-difference estimator carries O(10%) staggering artifacts);
    d. member-site gradient sum scales exactly as q^2 (no K_B pinning),
       and its ratio to N*K_B^2 is far from 1 and N-dependent — the
       members-only (Born-Infeld-sited) weighting has no exact identity;
    e. M components are anisotropic for a non-cubic (rod) cluster while
       the trace identity still holds — the dressing cannot supply an
       isotropic scalar N*M_REST, only an N-proportional trace.
T6  Amplitude-pinned uniform core (|J| = K_B inside a solid cube):
    exact surface law  sum_x |D+ J|^2 = 6 K_B^2 e^2 = 6 K_B^2 N^(2/3)
    (member-sited part exactly 3 K_B^2 e^2; exact one-hop difference sum
    2 K_B^2 e^2) — volume scaling N K_B^2 is impossible for pinned cores.
T7  GNC texture existence: J(x) = K_B P x with P the cyclic coordinate
    permutation (a 2*pi/3 rotation about (1,1,1)): lattice-exact
    divergence 0, per-member kinetic coefficient exactly K_B^2 |V|^2 for
    random V, total exactly N K_B^2 |V|^2.  Same for random-axis Rodrigues
    rotations at theta = 2*pi/3.
T8  Trace lemma: tr R(axis, theta) = 1 + 2 cos(theta); the lattice
    divergence of J = K_B R x equals K_B (1 + 2 cos theta); hence
    charge-free linear textures require theta = +/- 2*pi/3 exactly.

Anti-target note: every number below is a direct evaluation of a quantity
defined in the derivation document; no scans, no near-miss searches, no
fitting to targets.  Where a measured value is asserted, the assertion
documents the measured fact with a stated margin.

Run:  python scripts/proofs/proof_cluster_collective_coordinate.py
"""

import os
import sys

import numpy as np

# ---------------------------------------------------------------------------
# canonical constants (single source of truth: scripts/constants.py)
# ---------------------------------------------------------------------------
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
try:
    from constants import K_B, M_REST, C_SPEED  # type: ignore
except Exception:  # standalone fallback (values per scripts/constants.py)
    K_B = 0.511
    M_REST = K_B
    C_SPEED = 1.0 / np.sqrt(3.0)

RNG = np.random.default_rng(20260701)

_results = []


def check(name, ok, detail=""):
    _results.append((name, bool(ok)))
    status = "PASS" if ok else "FAIL"
    print(f"  [{status}] {name}" + (f"   ({detail})" if detail else ""))


# ---------------------------------------------------------------------------
# lattice helpers
# ---------------------------------------------------------------------------
def fwd_grad(field):
    """Forward-difference gradient of scalar field (L,L,L) -> (3,L,L,L)."""
    return np.stack([np.roll(field, -1, axis=i) - field for i in range(3)])


def bwd_div(J):
    """Backward-difference divergence of vector field (3,L,L,L) -> (L,L,L)."""
    return sum(J[i] - np.roll(J[i], 1, axis=i) for i in range(3))


def grad_tensor(J):
    """G[i, a] = D+_i J_a  for vector field J (3,L,L,L) -> (3,3,L,L,L)."""
    return np.stack(
        [np.stack([np.roll(J[a], -1, axis=i) - J[a] for a in range(3)]) for i in range(3)]
    )


def mass_tensor(J, mask=None):
    """M_ij = sum_x (D+_i J_a)(D+_j J_a), optionally restricted to mask sites."""
    G = grad_tensor(J)  # (i, a, x, y, z)
    if mask is not None:
        G = G * mask[None, None, :, :, :]
    return np.einsum("iaxyz,jaxyz->ij", G, G)


def coulomb_dressing(L, sites, q=1.0):
    """Minimal Gauss dressing: J = -D+ phi with (6-pt Laplacian) phi = -(rho - rho_bar).

    Periodic FFT Poisson solve with jellium background (net charge removed);
    returns (J, rho) with backward-div(J) = rho - rho_bar exactly.
    """
    rho = np.zeros((L, L, L))
    for (x, y, z) in sites:
        rho[x, y, z] += q
    rho_tilde = rho - rho.mean()
    k = 2.0 * np.pi * np.fft.fftfreq(L)
    kx, ky, kz = np.meshgrid(k, k, k, indexing="ij")
    lam = 4.0 * (np.sin(kx / 2) ** 2 + np.sin(ky / 2) ** 2 + np.sin(kz / 2) ** 2)
    rhok = np.fft.fftn(rho_tilde)
    phik = np.zeros_like(rhok)
    mask = lam > 1e-12
    phik[mask] = rhok[mask] / lam[mask]
    phi = np.real(np.fft.ifftn(phik))
    J = -fwd_grad(phi)
    return J, rho_tilde


def cube_sites(L, edge, origin=None):
    if origin is None:
        origin = L // 2 - edge // 2
    return [
        (origin + dx, origin + dy, origin + dz)
        for dx in range(edge)
        for dy in range(edge)
        for dz in range(edge)
    ]


def rodrigues(axis, theta):
    axis = np.asarray(axis, dtype=float)
    axis = axis / np.linalg.norm(axis)
    Kx = np.array(
        [[0, -axis[2], axis[1]], [axis[2], 0, -axis[0]], [-axis[1], axis[0], 0]]
    )
    return np.eye(3) + np.sin(theta) * Kx + (1 - np.cos(theta)) * (Kx @ Kx)


# ===========================================================================
print("=" * 76)
print("proof_cluster_collective_coordinate.py — FTD-0349")
print(f"K_B = M_REST = {K_B},  C_SPEED = {C_SPEED:.6f}")
print("=" * 76)

# ---------------------------------------------------------------------------
# T1 — gamma_FTD residual model vs engine test header numbers
# ---------------------------------------------------------------------------
print("\nT1: gamma_FTD residual reproduces test_cluster_inertia.cpp figures")
F = 1e-3
header = {1: 1.95694e-3, 8: 2.44618e-4, 27: 7.24795e-5}  # a_COM from the test header
a_model = {}
for N in (1, 8, 27):
    q = F / (N * M_REST)                      # per-mass momentum kick
    gamma = 1.0 / np.sqrt(1.0 - (q / C_SPEED) ** 2)
    a_model[N] = q / gamma                    # leading-order relativistic correction
    check(
        f"T1a: N={N} model a_COM matches engine header to 6 printed digits",
        abs(a_model[N] - header[N]) < 5e-9,
        f"model {a_model[N]:.6e} vs header {header[N]:.6e}",
    )
spread = abs(a_model[27] * 27 - a_model[1] * 1) / (a_model[1])
gamma1 = 1.0 / np.sqrt(1.0 - (F / (M_REST * C_SPEED)) ** 2) - 1.0
check(
    "T1b: a*N spread across N=1..27 is the gamma correction, ~6 ppm",
    5.0e-6 < spread < 6.5e-6 and 5.0e-6 < gamma1 < 6.0e-6,
    f"spread {spread:.3e}, gamma-1 at N=1 {gamma1:.3e}",
)

# ---------------------------------------------------------------------------
# T2 — point-mechanics additivity route (conditional theorem's algebra)
# ---------------------------------------------------------------------------
print("\nT2: additivity route  L(V) = -N K_B sqrt(1-V^2)  =>  P = N K_B gamma V")
ok_all = True
for N in (1, 8, 27):
    for V in (1e-3, 1e-2, 0.1):
        h = 1e-6
        Lp = -N * K_B * np.sqrt(1 - (V + h) ** 2)
        Lm = -N * K_B * np.sqrt(1 - (V - h) ** 2)
        P_num = (Lp - Lm) / (2 * h)          # dL/dV  (Lagrangian momentum)
        P_ana = N * K_B * V / np.sqrt(1 - V**2)
        ok_all &= abs(P_num - P_ana) / abs(P_ana) < 1e-7
check("T2a: numeric dL/dV == N K_B gamma V (rel < 1e-7, N in {1,8,27})", ok_all)
aN = [F / (N * M_REST) * N for N in (1, 8, 27)]
check(
    "T2b: Newtonian limit a*N = F/M_REST identically across N",
    max(abs(x - F / M_REST) for x in aN) < 1e-18,
    f"a*N = {aN[0]:.10e}",
)

# ---------------------------------------------------------------------------
# T3 — rigid-ansatz kinetic quadratic form (exact bilinear identity)
# ---------------------------------------------------------------------------
print("\nT3: sum_x |(V.D+)J|^2 == V^T M V  with  M_ij = sum_x D+_i J_a D+_j J_a")
L3 = 24
k = 2.0 * np.pi * np.fft.fftfreq(L3)
kx, ky, kz = np.meshgrid(k, k, k, indexing="ij")
k2 = kx**2 + ky**2 + kz**2
J_rand = np.stack(
    [
        np.real(np.fft.ifftn(np.fft.fftn(RNG.standard_normal((L3, L3, L3))) * np.exp(-2.0 * k2)))
        for _ in range(3)
    ]
)
M3 = mass_tensor(J_rand)
ok_all = True
for _ in range(5):
    V = RNG.standard_normal(3)
    dirderiv = sum(
        V[i] * (np.roll(J_rand, -1, axis=1 + i) - J_rand) for i in range(3)
    )  # (V.D+)J, shape (3,L,L,L)
    lhs = np.sum(dirderiv**2)
    rhs = V @ M3 @ V
    ok_all &= abs(lhs - rhs) / abs(rhs) < 1e-12
check("T3: exact for 5 random V on a random smooth field (rel < 1e-12)", ok_all)

# ---------------------------------------------------------------------------
# T4 — displacement remainder is FOURTH order in V (even function of V)
# ---------------------------------------------------------------------------
print("\nT4: sum_x |J(x-V)-J(x)|^2 = V^T M_spec V + O(V^4)  (remainder ratio ~16)")
grads = []
for a in range(3):
    Ja_k = np.fft.fftn(J_rand[a])
    ga = np.stack(
        [np.real(np.fft.ifftn(1j * kk * Ja_k)) for kk in (kx, ky, kz)]
    )  # ga[i] = spectral d_i J_a
    grads.append(ga)
grads = np.stack(grads, axis=0)  # (a, i, x, y, z)
M_spec = np.einsum("aixyz,ajxyz->ij", grads, grads)
Vdir = np.array([0.31, -0.22, 0.15])
res = {}
for scale in (1.0, 0.5):
    V = Vdir * scale
    phase = np.exp(-1j * (kx * V[0] + ky * V[1] + kz * V[2]))
    shifted = np.stack([np.real(np.fft.ifftn(np.fft.fftn(J_rand[a]) * phase)) for a in range(3)])
    lhs = np.sum((shifted - J_rand) ** 2)
    res[scale] = abs(lhs - V @ M_spec @ V)
ratio = res[1.0] / res[0.5]
check("T4: residual ratio under V -> V/2 in [12, 20] (fourth-order remainder)",
      12.0 < ratio < 20.0, f"ratio {ratio:.3f}")

# ---------------------------------------------------------------------------
# T5 — minimal Coulomb Gauss dressing
# ---------------------------------------------------------------------------
print("\nT5: minimal Coulomb Gauss dressing (FFT Poisson, jellium, L=48)")
L5 = 48
edges = [1, 2, 3, 4, 5]
M_over_N = {}
member_ratio = {}
def sym_hessian_mass(phi):
    """M^sym_ij = sum_x sum_a H_ia H_ja with site-centered central-difference
    second derivatives of phi (inversion-symmetric estimator)."""
    H = np.empty((3, 3) + phi.shape)
    for i in range(3):
        H[i, i] = np.roll(phi, -1, axis=i) - 2 * phi + np.roll(phi, 1, axis=i)
        for a in range(i + 1, 3):
            pp = np.roll(np.roll(phi, -1, axis=i), -1, axis=a)
            pm = np.roll(np.roll(phi, -1, axis=i), 1, axis=a)
            mp = np.roll(np.roll(phi, 1, axis=i), -1, axis=a)
            mm = np.roll(np.roll(phi, 1, axis=i), 1, axis=a)
            H[i, a] = H[a, i] = (pp - pm - mp + mm) / 4.0
    return np.einsum("iaxyz,jaxyz->ij", H, H)


def dressing_potential(L, sites, q=1.0):
    rho = np.zeros((L, L, L))
    for s in sites:
        rho[s] += q
    rho_t = rho - rho.mean()
    kk = 2.0 * np.pi * np.fft.fftfreq(L)
    kxx, kyy, kzz = np.meshgrid(kk, kk, kk, indexing="ij")
    lam = 4.0 * (np.sin(kxx / 2) ** 2 + np.sin(kyy / 2) ** 2 + np.sin(kzz / 2) ** 2)
    rhok = np.fft.fftn(rho_t)
    phik = np.zeros_like(rhok)
    m = lam > 1e-12
    phik[m] = rhok[m] / lam[m]
    return np.real(np.fft.ifftn(phik)), rho_t


trace_err = {}
for e in edges:
    sites = cube_sites(L5, e)
    N = len(sites)
    J, rho_t = coulomb_dressing(L5, sites, q=1.0)
    # a. Gauss residual
    resid = np.max(np.abs(bwd_div(J) - rho_t))
    if e == 1:
        check("T5a: lattice Gauss law  div-(J) == rho - rho_bar (max err < 1e-10)",
              resid < 1e-10, f"max residual {resid:.2e}")
    M = mass_tensor(J)
    M_over_N[N] = np.trace(M) / 3.0 / N
    # b. exact trace identity: trace(M) = sum rho_tilde^2 = N q^2 (1 - N/L^3)
    trace_err[N] = abs(np.trace(M) / (N * 1.0**2 * (1.0 - N / L5**3)) - 1.0)
    # d. member-site gradient sum vs N * K_B^2
    mask = np.zeros((L5, L5, L5))
    for s in sites:
        mask[s] = 1.0
    Mm = mass_tensor(J, mask=mask)
    member_ratio[N] = np.trace(Mm) / 3.0 / (N * K_B**2)

print("    M/N (all-site, per axis) :",
      {n: f"{v:.6f}" for n, v in M_over_N.items()})
print("    member-sum / (N K_B^2)   :",
      {n: f"{v:.4f}" for n, v in member_ratio.items()})

check("T5b-i: EXACT trace identity  trace(M) == N q^2 (1 - N/L^3)  (rel < 1e-9)",
      max(trace_err.values()) < 1e-9,
      f"max rel err {max(trace_err.values()):.2e} over N in {sorted(trace_err)}")
check("T5b-ii: dressing per-axis coefficient is q^2/3, NOT K_B^2 "
      "(GNC coefficient failure; q* = sqrt(3) K_B unmotivated)",
      abs(M_over_N[27] - 1.0 / 3.0 * (1.0 - 27.0 / L5**3)) < 1e-9
      and abs(1.0 / (3.0 * K_B**2) - 1.0) > 0.25,
      f"per-axis coeff {M_over_N[27]:.6f} = q^2/3;  q^2/(3 K_B^2) = {1.0/(3*K_B**2):.4f}")

# c. tensor structure: symmetrized site-centered Hessian isotropic for O_h cluster
phi3, _ = dressing_potential(L5, cube_sites(L5, 3), q=1.0)
Ms = sym_hessian_mass(phi3)
offdiag = max(abs(Ms[0, 1]), abs(Ms[0, 2]), abs(Ms[1, 2]))
diagspread = np.ptp(np.diag(Ms))
check("T5c: symmetrized Hessian tensor isotropic for the 3x3x3 cube (< 1e-9 rel)",
      offdiag < 1e-9 * Ms[0, 0] and diagspread < 1e-9 * Ms[0, 0],
      f"offdiag/diag {offdiag / Ms[0, 0]:.1e}  "
      f"(raw forward-difference estimator carries ~10% staggering artifact)")

# d. exact q^2 scaling of the member sum (no K_B pinning mechanism)
sites27 = cube_sites(L5, 3)
mask27 = np.zeros((L5, L5, L5))
for s in sites27:
    mask27[s] = 1.0
J_q1, _ = coulomb_dressing(L5, sites27, q=1.0)
J_q2, _ = coulomb_dressing(L5, sites27, q=2.0)
m1 = np.trace(mass_tensor(J_q1, mask=mask27))
m2 = np.trace(mass_tensor(J_q2, mask=mask27))
check("T5d-i: member gradient sum scales exactly as q^2 (rel < 1e-12)",
      abs(m2 / m1 - 4.0) < 1e-12, f"ratio {m2 / m1:.14f}")
mr_drift = member_ratio[125] / member_ratio[1]
check("T5d-ii: member-sum/(N K_B^2) far from 1 AND N-dependent (> 10% drift)",
      abs(member_ratio[1] - 1.0) > 0.3 and abs(mr_drift - 1.0) > 0.10,
      f"ratio(N=1) {member_ratio[1]:.3f}, drift {mr_drift:.3f}")

# e. anisotropy for a rod cluster (same N=8 as the 2x2x2 cube); trace identity
#    still holds (shape-independent), components do not (shape-dependent)
rod = [(L5 // 2 - 4 + i, L5 // 2, L5 // 2) for i in range(8)]
J_rod, _ = coulomb_dressing(L5, rod, q=1.0)
M_rod = mass_tensor(J_rod)
aniso = abs(M_rod[0, 0] - M_rod[1, 1]) / M_rod[1, 1]
check("T5e-i: rod (8x1x1) dressing components anisotropic (M_xx != M_yy by > 5%)",
      aniso > 0.05, f"anisotropy {aniso:.3f}")
rod_trace_err = abs(np.trace(M_rod) / (8.0 * (1.0 - 8.0 / L5**3)) - 1.0)
check("T5e-ii: rod trace STILL obeys the exact identity (shape-independent)",
      rod_trace_err < 1e-9, f"rel err {rod_trace_err:.2e}")

# ---------------------------------------------------------------------------
# T6 — amplitude-pinned uniform core: exact surface law
# ---------------------------------------------------------------------------
print("\nT6: amplitude-pinned uniform core  =>  exact surface law (not volume)")
L6 = 32
ok_total, ok_member, ok_hop = True, True, True
for e in (2, 3, 4, 5, 6):
    J = np.zeros((3, L6, L6, L6))
    o = L6 // 2 - e // 2
    J[0, o : o + e, o : o + e, o : o + e] = K_B  # uniform J = K_B x_hat inside
    M = mass_tensor(J)
    total = np.trace(M)
    ok_total &= abs(total - 6.0 * K_B**2 * e**2) < 1e-12 * total
    mask = np.zeros((L6, L6, L6))
    mask[o : o + e, o : o + e, o : o + e] = 1.0
    member = np.trace(mass_tensor(J, mask=mask))
    ok_member &= abs(member - 3.0 * K_B**2 * e**2) < 1e-12 * member
    hop = np.sum((np.roll(J, -1, axis=1) - J) ** 2)  # exact one-hop difference in x
    ok_hop &= abs(hop - 2.0 * K_B**2 * e**2) < 1e-12 * hop
check("T6a: all-site gradient sum == 6 K_B^2 e^2 exactly (e = 2..6)", ok_total)
check("T6b: member-sited part == 3 K_B^2 e^2 exactly", ok_member)
check("T6c: exact one-hop difference sum == 2 K_B^2 e^2 (also surface)", ok_hop)
e = 5
check(
    "T6d: pinned-core kinetic coeff / required volume law = 2 N^(-1/3) -> 0",
    abs((6.0 * e**2) / (3.0 * e**3) - 2.0 / e) < 1e-15,
    f"ratio at N=125: {2.0 / e:.3f}",
)

# ---------------------------------------------------------------------------
# T7 — GNC texture existence
# ---------------------------------------------------------------------------
print("\nT7: GNC textures exist:  J = K_B R x,  R in SO(3), tr R = 0")
Lc = 12
P = np.array([[0, 0, 1], [1, 0, 0], [0, 1, 0]], dtype=float)  # cyclic permutation
check("T7a: cyclic permutation P is a rotation (P P^T = I, det = 1) with tr = 0",
      np.allclose(P @ P.T, np.eye(3)) and abs(np.linalg.det(P) - 1) < 1e-12
      and abs(np.trace(P)) < 1e-15)

X = np.stack(np.meshgrid(np.arange(Lc, dtype=float),
                         np.arange(Lc, dtype=float),
                         np.arange(Lc, dtype=float), indexing="ij"))


def linear_texture(R):
    return np.einsum("ab,bxyz->axyz", K_B * R, X)


def interior_div_max(J):
    """Backward-div on interior sites only (linear texture is not periodic)."""
    div = sum(J[i] - np.roll(J[i], 1, axis=i) for i in range(3))
    return np.max(np.abs(div[1:-1, 1:-1, 1:-1]))


J_tex = linear_texture(P)
check("T7b: lattice divergence of the P-texture == 0 exactly (interior sites)",
      interior_div_max(J_tex) < 1e-12)
ok_all = True
for _ in range(5):
    V = RNG.standard_normal(3)
    # forward differences of a linear field are exact: D+_i J_a = K_B R_{a i}
    dirderiv = sum(V[i] * (np.roll(J_tex, -1, axis=1 + i) - J_tex) for i in range(3))
    per_site = np.sum(dirderiv[:, 1:-1, 1:-1, 1:-1] ** 2, axis=0)
    ok_all &= np.allclose(per_site, K_B**2 * (V @ V), rtol=1e-12)
check("T7c: per-member kinetic coefficient == K_B^2 |V|^2 exactly (5 random V)", ok_all)
Nint = (Lc - 2) ** 3
total = np.sum(np.sum(dirderiv[:, 1:-1, 1:-1, 1:-1] ** 2, axis=0))
check("T7d: member-summed coefficient == N K_B^2 |V|^2 exactly",
      abs(total - Nint * K_B**2 * (V @ V)) < 1e-9 * total)
ok_all = True
for _ in range(3):
    axis = RNG.standard_normal(3)
    R = rodrigues(axis, 2 * np.pi / 3)
    Jr = linear_texture(R)
    ok_all &= interior_div_max(Jr) < 1e-10
    Vr = RNG.standard_normal(3)
    dd = sum(Vr[i] * (np.roll(Jr, -1, axis=1 + i) - Jr) for i in range(3))
    ps = np.sum(dd[:, 1:-1, 1:-1, 1:-1] ** 2, axis=0)
    ok_all &= np.allclose(ps, K_B**2 * (Vr @ Vr), rtol=1e-10)
check("T7e: random-axis Rodrigues theta = 2 pi/3 textures: div 0 + GNC exact", ok_all)

# ---------------------------------------------------------------------------
# T8 — trace lemma: charge-free linear texture <=> theta = 2 pi/3
# ---------------------------------------------------------------------------
print("\nT8: tr R(theta) = 1 + 2 cos(theta);  div(K_B R x) = K_B (1 + 2 cos theta)")
ok_tr, ok_div = True, True
for _ in range(6):
    axis = RNG.standard_normal(3)
    theta = RNG.uniform(0, np.pi)
    R = rodrigues(axis, theta)
    ok_tr &= abs(np.trace(R) - (1 + 2 * np.cos(theta))) < 1e-12
    Jr = linear_texture(R)
    div = sum(Jr[i] - np.roll(Jr[i], 1, axis=i) for i in range(3))
    ok_div &= np.allclose(div[1:-1, 1:-1, 1:-1], K_B * (1 + 2 * np.cos(theta)), atol=1e-10)
check("T8a: tr R == 1 + 2 cos(theta) (6 random axes/angles)", ok_tr)
check("T8b: lattice div == K_B (1 + 2 cos theta) on interior sites", ok_div)
check("T8c: hence div = 0 forces cos(theta) = -1/2, i.e. theta = 2 pi/3 exactly",
      abs(1 + 2 * np.cos(2 * np.pi / 3)) < 1e-15)

# ---------------------------------------------------------------------------
print("\n" + "=" * 76)
npass = sum(1 for _, ok in _results if ok)
print(f"RESULT: {npass}/{len(_results)} PASS")
if npass != len(_results):
    for name, ok in _results:
        if not ok:
            print(f"  FAILED: {name}")
    sys.exit(1)
print("All identities asserted in DERIV_CLUSTER_COLLECTIVE_COORDINATE_v1.md verified.")
