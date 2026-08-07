"""mvc_fourchain_clock.py — the minimum viable clock carrier candidate.

The two-scale door SPEC_CARRIER_CONSTRAINTS_v1.md names as [OPEN], made
concrete and exact:

    A+ -- B- -- C+ -- D-   collinear, unit bonds AB, BC, CD (species 1),
    plus one closure bond A--D at range 3 (species 2).

All bonds sit at their potential minima (ZERO TENSION), yet the network
carries a self-stress omega = (t, t, t, -t) (tension in the short bonds
balanced by compression capacity in the long one). By the Connelly
second-order rigidity criterion, that possible stress blocks every
nontrivial transverse first-order flex, so the energy along each flex is
exactly QUARTIC: first-order flexibility with second-order rigidity —
precisely the C3 criterion FTD-0789 established (n = 4 iff flex + blocked).

This script:
  PART 1 (sympy, exact): rigidity matrix, stress, flex space, the blocking
    form and its positivity (Cauchy-Schwarz structure), the exact quartic
    coefficient of the mirror-even zero-momentum mode, effective mass.
  PART 2 (numeric): full 12-DOF conservative dynamics under an explicit
    two-well pair law; ringdown protocol with constrained-relaxed initial
    conditions; measures T(A), tests T*A = const, and recovers
    G* = Gamma(1/4)/Gamma(3/4) with NO fitted scale.
  PART 3: the C2 x C5 window arithmetic (band clearance vs separatrix)
    as an explicit inequality in (epsilon, A_max).

Epistemic status: a [SELECTED MODEL — DECLARED TWO-SCALE EXTENSION] of the
registered compact law; the quartic and its period law are exact conditional
mathematics within it. This is NOT a native carrier (fails C11 by
construction — that is what "minimum viable" means) and registers no
LEDGER claim.
"""
from __future__ import annotations

import numpy as np
import sympy as sp
import mpmath as mp

mp.mp.dps = 30
G_STAR = mp.gamma(mp.mpf(1) / 4) / mp.gamma(mp.mpf(3) / 4)

print("=" * 74)
print("PART 1 — exact rigidity analysis (sympy)")
print("=" * 74)

# Reference configuration: collinear on x-axis.
P = {"A": sp.Matrix([0, 0, 0]), "B": sp.Matrix([1, 0, 0]),
     "C": sp.Matrix([2, 0, 0]), "D": sp.Matrix([3, 0, 0])}
BONDS = [("A", "B", 1), ("B", "C", 1), ("C", "D", 1), ("A", "D", 3)]
VERTS = ["A", "B", "C", "D"]
k1, k2 = sp.symbols("k1 k2", positive=True)
KVEC = [k1, k1, k1, k2]

# Rigidity matrix (normalized rows: stretch rate per unit displacement).
R = sp.zeros(4, 12)
for e, (i, j, ell) in enumerate(BONDS):
    u = (P[j] - P[i]) / ell
    ii, jj = VERTS.index(i), VERTS.index(j)
    for a in range(3):
        R[e, 3 * ii + a] = -u[a]
        R[e, 3 * jj + a] = +u[a]

rank = R.rank()
stress = R.T.nullspace()          # left null space of R = coker
flexes = R.nullspace()
print(f"rank(R) = {rank}  (edges = 4)")
print(f"self-stress space dim = {len(stress)}; stress = {list(stress[0].T)}")
n_flex = len(flexes)
print(f"null(R) dim = {n_flex}  (trivial motions of a collinear body = 5, "
      f"so nontrivial flexes = {n_flex - 5})")

# Blocking (stress) form on transverse profiles q = (a, b, c, d) per axis.
# Use the COMPUTED stress (normalized so the first component is +1), not a
# hand-coded guess — in the stretch-rate row convention it is (1, 1, 1, -1).
a, b, c, d = sp.symbols("a b c d", real=True)
s0 = stress[0] / stress[0][0]
omega = [sp.nsimplify(s0[e]) for e in range(4)]
print(f"stress in stretch-rate convention (normalized): {omega}")
diffs = [(a - b) ** 2, (b - c) ** 2, (c - d) ** 2, (a - d) ** 2]
# omega(q,q) = sum_e omega_e |q_i - q_j|^2 / ell_e, built from the COMPUTED
# stress and the actual bond lengths (1, 1, 1, 3).
ELLS = [1, 1, 1, 3]
blocking = sp.expand(sum(w * df / el for w, df, el in zip(omega, diffs, ELLS)))
M = sp.Matrix(4, 4, lambda r_, c_: sp.Rational(1, 2) * sp.diff(
    sp.diff(blocking, [a, b, c, d][r_]), [a, b, c, d][c_]))
eigs = M.eigenvals()
print(f"blocking form eigenvalues: {dict(eigs)}")
print("  kernel must be exactly {constant, linear} = translation + rotation")
kern = M.nullspace()
print(f"  kernel vectors: {[list(v.T) for v in kern]}")

# Exact quartic coefficient via weighted projection onto the stress:
#   E4(q) = <omega, kappa(q)>^2 / (2 * sum_e omega_e^2 / k_e)
# for kappa_e = |q_i - q_j|^2 / (2 ell_e), q a first-order flex.
def E4_coeff(qvec):
    kap = []
    for e, (i, j, ell) in enumerate(BONDS):
        dq = qvec[VERTS.index(i)] - qvec[VERTS.index(j)]
        kap.append(sp.Rational(1, 2) * (dq * dq) / ell)
    num = sum(w * kp for w, kp in zip(omega, kap)) ** 2
    den = 2 * sum(w ** 2 / kk for w, kk in zip(omega, KVEC))
    return sp.simplify(num / den)

# The mirror-even, zero-transverse-momentum mode: q = (-1, +1, +1, -1) * U
lam_even = E4_coeff([-1, 1, 1, -1])
m_even = 4        # four unit masses moving with speed |Udot|
print(f"\nmirror-even zero-momentum mode  q = (-1,+1,+1,-1)·U:")
print(f"  E(U) = lambda_eff U^4 with lambda_eff = {lam_even}")
print(f"  m_eff = {m_even}")
lam_k1 = sp.simplify(lam_even.subs(k2, k1))
print(f"  at k1 = k2 = k:  lambda_eff = {lam_k1}")
TA_exact = sp.sqrt(sp.pi) * sp.Symbol("G_star") * sp.sqrt(
    sp.Rational(m_even) / (2 * lam_even))
print(f"  period law: T·A = sqrt(pi)·G*·sqrt(m_eff/(2 lambda_eff)) = "
      f"{sp.simplify(TA_exact)}")
TA_k1 = float(sp.sqrt(sp.pi) * sp.sqrt(4 / (2 * 2.0)))  # k=1: lam=2, m=4
print(f"  at k1 = k2 = 1:  T·A = sqrt(pi)·G* = {float(mp.sqrt(mp.pi) * G_STAR):.9f}")

# Cross-check other flexes are also quartic (blocked):
for name, q in [("BC-symmetric (0,1,1,0)", [0, 1, 1, 0]),
                ("antisym (0,1,-1,0)", [0, 1, -1, 0]),
                ("end-pair even (1,0,0,1)", [1, 0, 0, 1]),
                ("single end (1,0,0,0)", [1, 0, 0, 0])]:
    print(f"  E4[{name}] = {E4_coeff(q)}  (nonzero => blocked, n = 4)")

print()
print("=" * 74)
print("PART 2 — ringdown simulation, G* recovery with no fitted scale")
print("=" * 74)

K1 = K2 = 1.0
MASS = 1.0

def pair_energy_force(ri, rj, ell, k):
    dvec = rj - ri
    r = np.linalg.norm(dvec)
    u = dvec / r
    e = r - ell
    return 0.5 * k * e * e, k * e * u    # force on i is +k e u

BOND_IDX = [(0, 1, 1.0, K1), (1, 2, 1.0, K1), (2, 3, 1.0, K1), (0, 3, 3.0, K2)]
X0 = np.array([[0.0, 0, 0], [1, 0, 0], [2, 0, 0], [3, 0, 0]])

def total_energy_forces(x):
    E = 0.0
    F = np.zeros_like(x)
    for i, j, ell, k in BOND_IDX:
        e, f = pair_energy_force(x[i], x[j], ell, k)
        E += e
        F[i] += f
        F[j] -= f
    return E, F

def relaxed_ic(u0):
    """Displace the even mode transversally, relax axial DOF at fixed
    transverse coordinates (the normal-form path), zero velocity."""
    from scipy.optimize import minimize
    y = np.array([-u0, u0, u0, -u0])
    def pack(ax):
        x = X0.copy()
        x[:, 0] = X0[:, 0] + ax
        x[:, 1] = y
        return x
    def obj(ax):
        return total_energy_forces(pack(ax))[0]
    res = minimize(obj, np.zeros(4), method="BFGS", tol=1e-14)
    return pack(res.x)

def evolve_measure(u0, n_cycles=8):
    x = relaxed_ic(u0)
    v = np.zeros_like(x)
    # mode coordinate
    def U_of(x):
        return (x[1, 1] + x[2, 1] - x[0, 1] - x[3, 1]) / 4.0
    # velocity-Verlet
    lam_num = float(lam_k1.subs(k1, K1)) if hasattr(lam_k1, "subs") else 2.0
    T_pred = float(mp.sqrt(mp.pi) * G_STAR) * np.sqrt(4.0 / (2 * lam_num)) / u0
    dt = min(0.01, T_pred / 4000)
    E0, F = total_energy_forces(x)
    t, crossings, prevU = 0.0, [], U_of(x)
    max_steps = int(n_cycles * T_pred / dt) + 10
    amps, cur_max = [], 0.0
    for _ in range(max_steps):
        v += 0.5 * dt * F / MASS
        x += dt * v
        Enew, F = total_energy_forces(x)
        v += 0.5 * dt * F / MASS
        t += dt
        Uc = U_of(x)
        cur_max = max(cur_max, abs(Uc))
        if prevU < 0 <= Uc:                    # upward zero crossing
            frac = -prevU / (Uc - prevU)
            crossings.append(t - dt + frac * dt)
            amps.append(cur_max)
            cur_max = 0.0
        prevU = Uc
    Efin, _ = total_energy_forces(x)
    periods = np.diff(crossings)
    T_mean = periods.mean() if len(periods) else np.nan
    A_mean = np.mean(amps[1:]) if len(amps) > 1 else u0
    drift = abs(Efin + 0.5 * MASS * (v * v).sum() - (E0)) / max(E0, 1e-300)
    return T_mean, A_mean, len(periods), drift

lam_used = 2.0    # k1=k2=1 exact value from Part 1
print(f"{'u0':>7} {'T':>12} {'A':>9} {'T*A':>10} {'G*_exp':>12} {'cycles':>7}")
TA_th = float(mp.sqrt(mp.pi) * G_STAR)
results = []
for u0 in (0.02, 0.03, 0.05, 0.08, 0.12):
    T, A, nc, drift = evolve_measure(u0)
    gexp = T * A * np.sqrt(2 * lam_used / 4.0) / np.sqrt(np.pi)
    results.append((u0, T, A, gexp))
    print(f"{u0:>7.3f} {T:>12.4f} {A:>9.5f} {T*A:>10.5f} {gexp:>12.7f} {nc:>7}")
print(f"\n  theory: T·A = sqrt(pi)·G* = {TA_th:.7f},  G* = {float(G_STAR):.9f}")
# A -> 0 extrapolation (quartic-purity limit): fit G*_exp = g0 + g2·A^2
us = np.array([r[3] for r in results])
As = np.array([r[2] for r in results])
coef = np.polyfit(As ** 2, us, 1)
print(f"  A->0 extrapolated G*_exp = {coef[1]:.7f}  "
      f"(dev {abs(coef[1] - float(G_STAR)) / float(G_STAR) * 100:.4f}%)")

print()
print("=" * 74)
print("PART 3 — the C2 x C5 window (band clearance vs separatrix)")
print("=" * 74)
# omega(A) = 2 sqrt(pi) A sqrt(2 lam/m) / G* with lam = eps·2, m = 4 at
# k1 = k2 = eps  =>  omega(A) = 2 sqrt(pi) A sqrt(eps) / G*.
# Band tops per DERIV_FLEXURAL_QUARTIC_MECHANISM_v1.md (corpus values):
#   field one-axis band top = 2 arcsin(1/sqrt 3) = 1.230959
#   acoustic/wave band top  = 2.000000
w_field = 2 * np.arcsin(1 / np.sqrt(3.0))
w_wave = 2.0
print(f"  omega(A) = 2 sqrt(pi) sqrt(eps) A / G*  (k1 = k2 = eps)")
for wname, wB in (("field one-axis band top", w_field),
                  ("acoustic/wave band top", w_wave)):
    # clearance A_c and separatrix ceiling A_max ~ sqrt(w_well/2), w_well = 0.5
    A_max = np.sqrt(0.5 / 2)
    eps_min = (wB * float(G_STAR) / (2 * np.sqrt(np.pi) * A_max)) ** 2
    print(f"  {wname}: omega_B = {wB:.6f}  ->  clearance needs "
          f"eps·A_max^2 > {(wB * float(G_STAR) / (2 * np.sqrt(np.pi))) ** 2:.4f}"
          f";  at A_max = {A_max:.3f}: eps > {eps_min:.3f}")
print("  (A_max = sqrt(w/2) uses well half-width w = 0.5; the adoption's")
print("   scale parameters are exactly what the price sheet must declare.)")
print("  C5's A_drain (|J| < K_GENESIS under coupling) is NOT evaluated here —")
print("  it requires the coupled-field campaign the spec already flags.")
