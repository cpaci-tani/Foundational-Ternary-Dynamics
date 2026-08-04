"""Independent replication of the FTD-0319 base rate."""
import numpy as np
from mpmath import mp, gamma, sqrt, pi, e, euler, log, zeta, catalan, agm, khinchin, glaisher

mp.dps = 30
g = gamma
basket = [
 ("G_star", g(mp.mpf(1)/4)/g(mp.mpf(3)/4)), ("pi", pi), ("e", e),
 ("sqrt2", sqrt(2)), ("sqrt3", sqrt(3)), ("sqrt5", sqrt(5)),
 ("golden_phi", (1+sqrt(5))/2), ("euler_gamma", euler), ("ln2", log(2)),
 ("apery_zeta3", zeta(3)), ("catalan", catalan),
 ("varpi_lemn", g(mp.mpf(1)/4)**2/(2*sqrt(2*pi))), ("gauss_G", 1/agm(1, sqrt(2))),
 ("sqrt_pi", sqrt(pi)), ("gamma_1_3", g(mp.mpf(1)/3)),
 ("R3_equianh", g(mp.mpf(1)/3)/g(mp.mpf(2)/3)), ("khinchin", khinchin),
 ("glaisher", glaisher)]
K = np.array([float(v) for _, v in basket]); names = [n for n, _ in basket]
c = np.arange(1, 65, dtype=float); ex = np.arange(0, 6, dtype=float)
ALPHA_INV, N_C, TOL_P, TOL_M = 137.035999177, 3.0, 2.0e-6, 1.0e-2

C1, A, C2, Bx = np.meshgrid(c, ex, c, ex, indexing="ij")
C1, A, C2, Bx = C1.ravel(), A.ravel(), C2.ravel(), Bx.ravel()
print(f"family size per constant: {C1.size};  total: {C1.size*len(K):,}")
xp_all, xm_all, src = [], [], []
for i, k in enumerate(K):
    b_ = C1 * k**A; cc = C2 * k**Bx
    disc = b_*b_ - 4*cc; m = disc > 0
    r = np.sqrt(disc[m])
    xp_all.append((b_[m]+r)/2); xm_all.append((b_[m]-r)/2)
    src.append(np.full(int(m.sum()), i))
xp = np.concatenate(xp_all); xm = np.concatenate(xm_all); src = np.concatenate(src)
print(f"real-rooted polynomials: {xp.size:,}")
rp = np.abs(xp - ALPHA_INV)/ALPHA_INV; rm = np.abs(xm - N_C)/N_C

print("\n=== LEG 1 ALONE (no x_- gate): observed vs linear null ===")
print(f"  {'tol':>11} {'observed':>9} {'null':>9} {'ratio':>7}")
base = np.sum(rp < 1e-3)
for tol in (1e-3, 1e-4, 1e-5, 2e-6, 1.2572e-6):
    obs = int(np.sum(rp < tol)); null = base*tol/1e-3
    print(f"  {tol:>11.3e} {obs:>9} {null:>9.2f} {(obs/null if null else 0):>7.2f}")

w = (xp > 136) & (xp < 138); dens = np.sum(w)/2.0; win = 2*TOL_P*ALPHA_INV
print(f"\n  roots in [136,138]: {int(np.sum(w)):,} -> density {dens:,.1f}/unit x")
print(f"  EXPECTED HITS at registered gate = {dens*win:.2f}   OBSERVED = {int(np.sum(rp<TOL_P))}")

print("\n=== DOES THE SECOND LEG ELIMINATE ANYTHING? ===")
print(f"  {'resid_+ gate':>13} {'no x_- gate':>12} {'with gate':>10} {'removed':>8}")
for tol in (1e-2, 1e-3, 1e-4, 1e-5, 2e-6):
    a_ = int(np.sum(rp < tol)); b2 = int(np.sum((rp < tol) & (rm < TOL_M)))
    print(f"  {tol:>13.1e} {a_:>12} {b2:>10} {a_-b2:>8}")

print("\n=== COUNTERFACTUAL: x_- near ANY integer 1..10 ===")
ints = np.arange(1, 11, dtype=float)
anyint = np.min(np.abs(xm[:, None] - ints[None, :])/ints[None, :], axis=1)
for tol in (1e-2, 1e-3, 2e-6):
    print(f"  resid_+ < {tol:.0e}:  x_-~3 = {int(np.sum((rp<tol)&(rm<TOL_M))):>5}"
          f"   x_-~any int = {int(np.sum((rp<tol)&(anyint<TOL_M))):>5}"
          f"   ungated = {int(np.sum(rp<tol)):>6}")

print("\n=== MONTE CARLO: random target near 137 ===")
rng = np.random.default_rng(20260803)
targets = rng.uniform(110, 170, 20000)
xs = np.sort(xp[(xp > 100) & (xp < 180)])
hits = np.searchsorted(xs, targets*(1+TOL_P)) - np.searchsorted(xs, targets*(1-TOL_P))
print(f"  mean hits/target = {hits.mean():.2f}   P(>=1 hit) = {np.mean(hits>=1):.3f}")

print("\n=== SURVIVOR AND RANK ===")
for i in np.where((rp < TOL_P) & (rm < TOL_M))[0]:
    print(f"  x_+={xp[i]:.9f} x_-={xm[i]:.9f} const={names[src[i]]} resid={rp[i]:.4e}")
gated = np.where(rm < TOL_M)[0]; order = gated[np.argsort(rp[gated])]
for j, i in enumerate(order[:3]):
    print(f"  rank {j+1}: resid_+={rp[i]:.6e} const={names[src[i]]} x_+={xp[i]:.6f}")
print(f"  rank2/rank1 = {rp[order[1]]/rp[order[0]]:.1f}x")
