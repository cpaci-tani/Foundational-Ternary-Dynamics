"""native_wheel_clock_sim.py — does the registered compact law's own
dynamics tick at G* on the hexagon wheel?

Integrates the ACTUAL registered pairwise law (DERIV_MINIMAL_MANY_BODY_
MATTER_NETWORK_v1.md eq. 1-3):

    U = sum_{a<b} A_ab V(q_ab),  q = |x_a - x_b|^2,  A_ab opposite-polarity
    V(q) = -16 eps (q - 3/2)^2 (q - 3/4)   for q < 3/2, else 0

on the N = 19 regular hexagon wheel (s = 2). The D6-symmetric radial
rim-bowing mode is a blocked flex (verified: <omega,kappa> != 0), lies on a
planar symmetric invariant manifold, and should therefore be a pure quartic
oscillator at leading order: T = sqrt(pi) G* sqrt(m_eff/(2 lambda_eff))/A.

Protocol: static constrained relaxation gives lambda_eff from the energy
curve (no hand formula); ringdown from relaxed ICs gives T(A); the product
T*A*sqrt(2 lambda_eff/m_eff)/sqrt(pi) is compared to
G* = Gamma(1/4)/Gamma(3/4) with NO fitted scale.

Also reports the C2 arithmetic for this native mode: Omega(A) vs the field
band top with the law's own stiffness (bond curvature 96 eps).

OUTCOME (2026-08-07): REFUTING ARTIFACT. The constrained static relaxation
collapses to ~1e-12 for u >= 0.03 — the escape is the coupled flex
(rim-bow u, spoke-buckle delta = u), a zero-energy mechanism cone of the
squared-difference quartic E ~ (sum kappa_rim - sum kappa_spoke)^2. The
wheel's compressed spokes are free-hinge chains of length 2 and buckle at
zero cost; the per-chain blocking gate (G4) was insufficient. See the
corrected G5 gate in native_chain_network_verify.py and the buckling
criterion in ANALYSIS_MINIMUM_VIABLE_CLOCK_CARRIER_v1.md §3.1. The
ringdown was not completed and no G* claim exists for the wheel.
[VERIFICATION INSTRUMENT — registers nothing]
"""
from __future__ import annotations

import numpy as np
from scipy.optimize import minimize
import mpmath as mp

mp.mp.dps = 30
G_STAR = float(mp.gamma(mp.mpf(1) / 4) / mp.gamma(mp.mpf(3) / 4))
EPS = 1.0
S = 2

# --- build the wheel (planar, z = 0) ---------------------------------
pos0 = [np.array([0.0, 0.0])]                      # hub
names = ["H"]
for k in range(6):
    a = k * np.pi / 3
    pos0.append(S * np.array([np.cos(a), np.sin(a)]))
    names.append(f"J{k}")
mid_idx = {}
for k in range(6):                                  # spoke midpoints
    a = k * np.pi / 3
    pos0.append(np.array([np.cos(a), np.sin(a)]))
    names.append(f"sm{k}")
rim_mid_start = len(pos0)
for k in range(6):                                  # rim midpoints
    p = 0.5 * (pos0[1 + k] + pos0[1 + (k + 1) % 6])
    pos0.append(p.copy())
    names.append(f"rm{k}")
P0 = np.array(pos0)
N = len(P0)
# polarity: hub/corners even (+), midpoints odd (-)
col = np.array([0] * 7 + [1] * 12)
OPP = (col[:, None] != col[None, :])
np.fill_diagonal(OPP, False)

def energy_forces(P):
    d = P[:, None, :] - P[None, :, :]
    q = np.sum(d * d, axis=2)
    act = OPP & (q < 1.5)
    qm = np.where(act, q, 1.0)
    V = -16 * EPS * (qm - 1.5) ** 2 * (qm - 0.75)
    Vp = -48 * EPS * (qm - 1.5) * (qm - 1.0)
    E = 0.5 * np.sum(np.where(act, V, 0.0))
    # F_a = -dU/dx_a = - sum_b Vp * 2 (x_a - x_b)
    W = np.where(act, Vp, 0.0)
    F = -2.0 * np.einsum("ab,abk->ak", W, d)
    return E, F

E0, F0 = energy_forces(P0)
print(f"wheel N = {N}: U0 = {E0:.6f} (= -24 eps: {abs(E0 + 24 * EPS):.2e}), "
      f"|F| at rest = {np.abs(F0).max():.2e}  (zero-tension equilibrium)")

# --- radial rim-bowing mode ------------------------------------------
rim_ids = np.arange(rim_mid_start, rim_mid_start + 6)
rhat = np.array([P0[i] / np.linalg.norm(P0[i]) for i in rim_ids])

def with_mode(u, relax=True):
    """Displace rim midpoints radially by u; relax everything else."""
    P = P0.copy()
    for t, i in enumerate(rim_ids):
        P[i] = P0[i] + u * rhat[t]
    if not relax:
        return P
    free = [i for i in range(N) if i not in set(rim_ids)]
    def pack(x):
        Q = P.copy()
        Q[free] = x.reshape(-1, 2)
        return Q
    res = minimize(lambda x: energy_forces(pack(x))[0],
                   P[free].ravel(), method="BFGS",
                   options={"gtol": 1e-13, "maxiter": 2000})
    return pack(res.x)

# static quartic coefficient
us = np.array([0.02, 0.03, 0.04, 0.06, 0.08])
Es = []
for u in us:
    Pu = with_mode(u)
    Es.append(energy_forces(Pu)[0] - E0)
Es = np.array(Es)
lam_fit = np.polyfit(us ** 4, Es, 1)[0]
print("\nstatic energy curve (constrained relaxation):")
for u, e in zip(us, Es):
    print(f"  u = {u:.3f}:  dE = {e:.6e}   dE/u^4 = {e / u ** 4:.4f}")
print(f"lambda_eff (fit) = {lam_fit:.4f}   [law units, eps = {EPS}]")
m_eff = 6.0

# --- ringdown --------------------------------------------------------
print("\nringdown under the registered law:")
print(f"{'u0':>7} {'T':>11} {'A':>9} {'T*A':>9} {'G*_exp':>11} {'cyc':>4}")
results = []
for u0 in (0.02, 0.03, 0.05, 0.08):
    P = with_mode(u0)
    Vel = np.zeros_like(P)
    T_pred = np.sqrt(np.pi) * G_STAR * np.sqrt(m_eff / (2 * lam_fit)) / u0
    dt = min(2e-3, T_pred / 6000)
    E, F = energy_forces(P)
    def mode_coord(P):
        return np.mean([ (P[i] - P0[i]) @ rhat[t]
                         for t, i in enumerate(rim_ids) ])
    prev = mode_coord(P)
    tnow, crossings, amps, cur = 0.0, [], [], abs(prev)
    steps = int(9 * T_pred / dt)
    for _ in range(steps):
        Vel += 0.5 * dt * F
        P += dt * Vel
        E, F = energy_forces(P)
        Vel += 0.5 * dt * F
        tnow += dt
        uc = mode_coord(P)
        cur = max(cur, abs(uc))
        if prev < 0 <= uc:
            frac = -prev / (uc - prev)
            crossings.append(tnow - dt + frac * dt)
            amps.append(cur)
            cur = 0.0
        prev = uc
    Tm = np.diff(crossings).mean() if len(crossings) > 1 else np.nan
    Am = np.mean(amps[1:]) if len(amps) > 1 else u0
    g = Tm * Am * np.sqrt(2 * lam_fit / m_eff) / np.sqrt(np.pi)
    results.append((Am, g))
    print(f"{u0:>7.3f} {Tm:>11.3f} {Am:>9.5f} {Tm * Am:>9.4f} "
          f"{g:>11.6f} {len(crossings) - 1:>4}")
As = np.array([r[0] for r in results])
Gs = np.array([r[1] for r in results])
g0 = np.polyfit(As ** 2, Gs, 1)[1]
print(f"\n  A->0 extrapolated G*_exp = {g0:.6f}   vs G* = {G_STAR:.9f}"
      f"   (dev {abs(g0 - G_STAR) / G_STAR * 100:.3f}%)")

# --- C2 arithmetic for the native mode -------------------------------
print("\nC2 arithmetic (native mode, law stiffness prop. to eps):")
wF = 2 * np.arcsin(1 / np.sqrt(3.0))     # field one-axis band top 1.230959
# Omega(A) = 2 sqrt(pi) A sqrt(2 lam/m) / G*, lam prop. to eps
coef = 2 * np.sqrt(np.pi) * np.sqrt(2 * lam_fit / m_eff) / G_STAR
print(f"  Omega(A) = {coef:.4f} * sqrt(eps) * A   (this run: eps = 1)")
for Amax in (0.10, 0.20, 0.30):
    eps_min = (wF / (coef * Amax)) ** 2
    print(f"  band clearance at A_max = {Amax:.2f} needs eps > {eps_min:.2f}")
