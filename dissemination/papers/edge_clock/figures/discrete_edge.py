"""discrete_edge.py — Paper-II exploration: the discreteness edge of the quartic
clock, and the two-clock (self-trapping) edge.  ALL results exploratory/pre-audit.

D1  Exact scaling theorem (verified): leapfrog for xdd = -(4 lam/m) x^3 is
    equivariant under (x, p, dt) -> (s x, s^2 p, dt/s); hence every discrete
    phenomenon depends only on rho = dt / T(A).
D2  The universal discreteness edge: instability of the leapfrog quartic map
    vs rho.  Hand prediction from turning-point stiffness:
    rho_c = 2/(sqrt(6) sqrt(pi) G*) = 0.15570  (harmonic clock: 1/pi = 0.31831).
D3  The (k^2, rho) phase diagram: the discreteness edge across the whole
    pitchfork crossover (detuned leapfrog, mu != 0).
D4  Two coupled quartic clocks: energy-transfer fraction vs chi = kappa/(2 lam A^2);
    self-detuning localization (self-trapping) edge chi_c.
"""
import numpy as np
from scipy.special import gamma
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

G_STAR = float(gamma(0.25)/gamma(0.75))
TAU4 = np.sqrt(np.pi)*G_STAR          # = T A sqrt(2 lam/m) at the quartic point
LAM, M = 1.0, 1.0

def T_pure(A):
    return TAU4/(A*np.sqrt(2*LAM/M))

def line(s=""):
    print(s, flush=True)

# ---------------- D1: scaling equivariance ----------------
line("D1 — exact scaling equivariance of the leapfrog quartic map:")
def leapfrog_traj(x0, p0, dt, n):
    x, p = x0, p0
    out = np.empty((n, 2))
    for i in range(n):
        p = p - dt*4*LAM*x**3/2      # kick half
        x = x + dt*p/M               # drift
        p = p - dt*4*LAM*x**3/2      # kick half
        out[i] = (x, p)
    return out
s = 2.0
t1 = leapfrog_traj(1.0, 0.0, 0.01, 2000)
t2 = leapfrog_traj(s*1.0, 0.0, 0.01/s, 2000)
dev = np.max(np.abs(t2[:, 0]/s - t1[:, 0])) + np.max(np.abs(t2[:, 1]/s**2 - t1[:, 1]))
line(f"  (A, dt) vs (2A, dt/2): max orbit deviation after 2000 steps = {dev:.2e}  (exact: 0)")

# ---------------- D2: universal instability edge vs rho ----------------
line("\nD2 — universal discreteness edge of the pure quartic clock:")
rho_pred = 2.0/(np.sqrt(6)*TAU4)
line(f"  turning-point-stiffness prediction: rho_c = 2/(sqrt6 * sqrt(pi) G*) = {rho_pred:.5f}")
line(f"  harmonic-clock reference: 1/pi = {1/np.pi:.5f}")

A0 = 1.0
T0 = T_pure(A0)
rhos = np.linspace(0.02, 0.22, 600)
dts = rhos*T0
# vectorized leapfrog over all rho simultaneously
x = np.full_like(dts, A0); p = np.zeros_like(dts)
E0 = LAM*A0**4
maxdev = np.zeros_like(dts)          # max |E - E0|/E0
blown = np.zeros_like(dts, dtype=bool)
NSTEP = 60000
for i in range(NSTEP):
    p = p - dts*4*LAM*x**3/2
    x = x + dts*p/M
    p = p - dts*4*LAM*x**3/2
    E = p*p/(2*M) + LAM*x**4
    maxdev = np.maximum(maxdev, np.abs(E-E0)/E0)
    b = (np.abs(x) > 50*A0) | ~np.isfinite(x)
    blown |= b
    x = np.where(blown, 0.0, x); p = np.where(blown, 0.0, p)   # freeze blown
unstable = blown | (maxdev > 1.0)
if unstable.any():
    rho_c_meas = rhos[np.argmax(unstable)]
    line(f"  measured instability onset: rho_c = {rho_c_meas:.5f}  ({NSTEP} steps, blowup/energy>100% criterion)")
else:
    line("  no instability found in scanned range")
# resonance-tongue fine structure: energy-error profile saved for figure
maxdev_profile = maxdev.copy(); rho_profile = rhos.copy(); unstable_profile = unstable.copy()

# ---------------- D3: (k^2, rho) phase diagram ----------------
line("\nD3 — discreteness edge across the crossover (k^2 vs rho phase map):")
from scipy.special import ellipk
def T_detuned(A, mu):
    den = 0.5*mu + 2*LAM*A*A
    k2 = LAM*A*A/den
    return 4*np.sqrt(M/2)*ellipk(k2)/np.sqrt(den), k2

k2_targets = np.linspace(0.05, 0.95, 46)
rho_grid = np.linspace(0.02, 0.40, 60)
edge = np.full(len(k2_targets), np.nan)
A_fix = 1.0
for j, k2t in enumerate(k2_targets):
    # choose mu to hit k2t at amplitude A_fix:  k2 = lam A^2/(mu/2+2 lam A^2)
    mu = 2*LAM*A_fix**2*(1-2*k2t)/k2t
    Tt, k2chk = T_detuned(A_fix, mu)
    # guard: double-well side needs E above barrier: E = mu A^2/2 + lam A^4 > 0
    if 0.5*mu*A_fix**2 + LAM*A_fix**4 <= 0:
        continue
    dts_j = rho_grid*Tt
    x = np.full_like(dts_j, A_fix); p = np.zeros_like(dts_j)
    blown_j = np.zeros_like(dts_j, dtype=bool)
    for i in range(20000):
        p = p - dts_j*(mu*x + 4*LAM*x**3)/2
        x = x + dts_j*p/M
        p = p - dts_j*(mu*x + 4*LAM*x**3)/2
        b = (np.abs(x) > 50*A_fix) | ~np.isfinite(x)
        blown_j |= b
        x = np.where(blown_j, 0.0, x); p = np.where(blown_j, 0.0, p)
    if blown_j.any():
        edge[j] = rho_grid[np.argmax(blown_j)]
line(f"  edge at k2=0.5 (quartic): rho_c = {edge[np.argmin(np.abs(k2_targets-0.5))]:.4f}")
line(f"  edge at k2=0.05 (near-harmonic): rho_c = {edge[1]:.4f}  (harmonic bound 1/pi = {1/np.pi:.4f})")
line(f"  edge at k2=0.95 (deep over-barrier): rho_c = {edge[-1]:.4f}")

# ---------------- D4: two coupled quartic clocks — self-trapping edge ----------------
line("\nD4 — two coupled quartic clocks: energy-transfer vs chi = kappa/(2 lam A^2):")
chis = np.linspace(0.005, 0.5, 80)
kappas = chis*2*LAM*A0**2
dt4 = 0.002*T0
x1 = np.full_like(kappas, A0); p1 = np.zeros_like(kappas)
x2 = np.zeros_like(kappas);    p2 = np.zeros_like(kappas)
maxfrac = np.zeros_like(kappas)
NS4 = 400000
for i in range(NS4):
    F1 = -4*LAM*x1**3 - kappas*(x1-x2)
    F2 = -4*LAM*x2**3 - kappas*(x2-x1)
    p1 += dt4*F1/2; p2 += dt4*F2/2
    x1 += dt4*p1/M; x2 += dt4*p2/M
    F1 = -4*LAM*x1**3 - kappas*(x1-x2)
    F2 = -4*LAM*x2**3 - kappas*(x2-x1)
    p1 += dt4*F1/2; p2 += dt4*F2/2
    E1 = p1*p1/2 + LAM*x1**4
    E2 = p2*p2/2 + LAM*x2**4
    maxfrac = np.maximum(maxfrac, E2/(E1+E2+1e-300))
# localization edge: first chi where clock 2 ever receives > 45% of the energy
idx = np.where(maxfrac > 0.45)[0]
chi_c = chis[idx[0]] if len(idx) else np.nan
line(f"  duration: {NS4*dt4/T0:.0f} periods of clock 1")
line(f"  max energy fraction reaching clock 2: {maxfrac.min():.3f} (smallest chi) ... {maxfrac.max():.3f} (largest)")
line(f"  self-trapping edge (first chi with >45% transfer): chi_c ~ {chi_c:.4f}")

# ---------------- Figure 6 ----------------
plt.rcParams.update({"font.size": 9, "axes.titlesize": 9.5, "legend.fontsize": 8,
                     "axes.spines.top": False, "axes.spines.right": False})
C1, C2, C4 = "#2a78d6", "#eb6834", "#4a3aa7"
fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(9.8, 2.8))

ax1.semilogy(rho_profile, np.maximum(maxdev_profile, 1e-10), lw=1.0, color=C4)
ax1.axvline(rho_pred, color=C2, lw=0.9, ls=":")
ax1.text(rho_pred+0.003, 1e-6, r"$\frac{2}{\sqrt{6}\,\sqrt{\pi}G^{*}}$",
         fontsize=8.5, color=C2)
ax1.set_xlabel(r"$\rho = \Delta t/T$ (steps$^{-1}$ per period)")
ax1.set_ylabel("max relative energy error")
ax1.set_title("(a) discreteness edge, pure quartic clock")

ax2.plot(edge, k2_targets, "o-", ms=3, lw=1.2, color=C4)
ax2.axvline(1/np.pi, color=C1, lw=0.9, ls=":")
ax2.text(1/np.pi-0.005, 0.1, r"harmonic $1/\pi$", rotation=90, fontsize=7.5, color=C1)
ax2.axhline(0.5, color=C2, lw=0.8, ls=":")
ax2.text(0.05, 0.515, r"quartic point $k^2=\frac{1}{2}$", fontsize=7.5, color=C2)
ax2.set_xlabel(r"$\rho_c$ (instability onset)")
ax2.set_ylabel(r"$k^{2}$")
ax2.set_title("(b) the edge across the crossover")

ax3.plot(chis, maxfrac, lw=1.6, color=C4)
ax3.axhline(0.5, color="gray", lw=0.7, ls=":")
if np.isfinite(chi_c):
    ax3.axvline(chi_c, color=C2, lw=0.9, ls=":")
    ax3.text(chi_c+0.008, 0.15, rf"$\chi_c\approx{chi_c:.3f}$", fontsize=8.5, color=C2)
ax3.set_xlabel(r"coupling $\chi=\kappa/(2\lambda A^{2})$")
ax3.set_ylabel("max energy fraction in clock 2")
ax3.set_title("(c) two clocks: self-trapping edge")
fig.tight_layout()
fig.savefig("fig6_discrete.pdf")
fig.savefig("fig6_discrete.png", dpi=200)
line("\nfigure written: fig6_discrete")
