"""qm_edge.py — the quantum rung of the edge clock.
Reduced Hamiltonian  h = -d2/du2 + (g/2) u^2 + u^4   (hbar=1, 2m=1),
g = mu (2m)^{1/3} / (lambda^{2/3} hbar^{2/3})  the quantum detuning.

Q1  Reduction/universality: two physical (m, mu, lambda, hbar) sets at equal g
    give identical spectra after the energy rescale.
Q2  Pure quartic spectrum vs WKB G*-law  E_n ~ [3 sqrt(pi) (n+1/2)/G*]^{4/3};
    ratio -> 1; level-ratio table incl. (E2-E1)/(E1-E0) for quarton-type devices.
Q3  Quantum pitchfork is rounded: tunneling splitting E1-E0 closes smoothly
    (exponentially) for g<0 — no sharp transition at finite hbar.
Q4  Quantum -> classical weld: thermal C(T) -> 3/4 k_B and
    U(T)=<x^4>/<x^2>^2 -> G*^2/4 at high T (the Sec-thermo values).
"""
import numpy as np
from scipy.linalg import eigh_tridiagonal
from scipy.special import gamma

G_STAR = gamma(0.25)/gamma(0.75)

def solve(g, L=9.0, N=3000, nstates=200):
    """FD eigenproblem for h = -d2/du2 + (g/2)u^2 + u^4 on [-L, L]."""
    u = np.linspace(-L, L, N)
    h = u[1]-u[0]
    V = 0.5*g*u*u + u**4
    diag = 2.0/h**2 + V
    off = -np.ones(N-1)/h**2
    E, psi = eigh_tridiagonal(diag, off, select='i', select_range=(0, nstates-1))
    # normalize eigenvectors on the grid
    psi = psi / np.sqrt(h)
    return u, h, E, psi

print("Q1 — universality of the reduction (two physical systems, same g):", flush=True)
# physical spectrum: solve directly in physical units via scaled solver
# E_phys = eps * E_reduced with eps = lambda * a^4, a = (hbar^2/(2 m lambda))^{1/6}
def physical_spectrum(m, mu, lam, hbar, n=6):
    a = (hbar**2/(2*m*lam))**(1/6.0)
    eps = lam*a**4
    g = mu*a*a/eps
    _, _, E, _ = solve(g, nstates=n)
    return g, eps*E[:n]
g1, E1 = physical_spectrum(m=1.0, mu=0.7, lam=1.0, hbar=1.0)
g2, E2 = physical_spectrum(m=2.0, mu=0.7*(2*2.0)**(-1/3.0)*(3.0)**(2/3.0)*(0.5)**(2/3.0)*2**(1/3.0), lam=3.0, hbar=0.5)
# choose mu2 so g2 == g1: mu2 = g1 * lam2^{2/3} hbar2^{2/3} / (2 m2)^{1/3}
mu2 = g1 * 3.0**(2/3.0) * 0.5**(2/3.0) / (2*2.0)**(1/3.0)
g2, E2 = physical_spectrum(m=2.0, mu=mu2, lam=3.0, hbar=0.5)
r = (E1[:6]/E1[0]) / (E2[:6]/E2[0])
print(f"  g1 = {g1:.6f}  g2 = {g2:.6f}   spectra ratio (should be 1): max dev {np.abs(r-1).max():.2e}", flush=True)

print("\nQ2 — pure quartic (g=0): exact vs WKB G*-law:", flush=True)
u, h, E0s, psi0 = solve(0.0, nstates=60)
wkb = lambda n: (3*np.sqrt(np.pi)*(n+0.5)/G_STAR)**(4/3.0)
for n in [0, 1, 2, 5, 10, 20, 40]:
    print(f"  n={n:3d}  E={E0s[n]:12.6f}   WKB={wkb(n):12.6f}   ratio={E0s[n]/wkb(n):.6f}", flush=True)
r21 = (E0s[2]-E0s[1])/(E0s[1]-E0s[0])
r32 = (E0s[3]-E0s[2])/(E0s[2]-E0s[1])
print(f"  ground state (p^2+u^4 convention): E0 = {E0s[0]:.10f}", flush=True)
print(f"  level-spacing ratios: (E2-E1)/(E1-E0) = {r21:.6f}   (E3-E2)/(E2-E1) = {r32:.6f}", flush=True)
print(f"  WKB spacing-ratio limit at n->inf: -> ((n+5/2)^(4/3)-(n+3/2)^(4/3))/((n+3/2)^(4/3)-(n+1/2)^(4/3))", flush=True)

print("\nQ3 — quantum-rounded pitchfork: splitting E1-E0 vs g < 0:", flush=True)
gs_neg = [0.0, -2, -4, -6, -8, -10, -12]
splits = []
for g in gs_neg:
    _, _, Eg, _ = solve(g, L=10.0, nstates=4)
    splits.append(Eg[1]-Eg[0])
    print(f"  g={g:+6.1f}   E1-E0 = {Eg[1]-Eg[0]:.6e}", flush=True)

print("\nQ4 — quantum->classical weld at g=0 (reduced units, kB=1):", flush=True)
# thermal averages from spectrum + diagonal matrix elements
x2_d = np.array([np.sum(psi0[:, n]**2 * u**2)*h for n in range(len(E0s))])
x4_d = np.array([np.sum(psi0[:, n]**2 * u**4)*h for n in range(len(E0s))])
print(f"  ground state: <u^2> = {x2_d[0]:.6f}  U_0 = <u^4>/<u^2>^2 = {x4_d[0]/x2_d[0]**2:.6f}", flush=True)
for T in [0.5, 2.0, 8.0, 30.0, 45.0]:
    b = 1.0/T
    w = np.exp(-b*(E0s-E0s[0])); Z = w.sum()
    Ebar = (E0s*w).sum()/Z
    E2bar = (E0s**2*w).sum()/Z
    C = b*b*(E2bar-Ebar**2)
    Uth = ((x4_d*w).sum()/Z) / ((x2_d*w).sum()/Z)**2
    print(f"  T={T:6.1f}:  C = {C:.4f} (classical 3/4)   U = {Uth:.4f} (classical G*^2/4 = {G_STAR**2/4:.4f})", flush=True)

# ---------------- Figure 5 ----------------
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
plt.rcParams.update({"font.size": 9, "axes.titlesize": 9.5, "legend.fontsize": 8,
                     "axes.spines.top": False, "axes.spines.right": False})
C1, C2, C4 = "#2a78d6", "#eb6834", "#4a3aa7"
fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(9.6, 2.7))

ns = np.arange(0, 41)
ax1.plot(ns, E0s[:41]/np.array([wkb(n) for n in ns]), "o-", ms=3, lw=1, color=C4)
ax1.axhline(1.0, color="k", lw=0.7, ls=":")
ax1.set_xlabel("level $n$")
ax1.set_ylabel(r"$E_n \,/\, [3\sqrt{\pi}(n+\frac{1}{2})/G^{*}]^{4/3}$")
ax1.set_title("(a) quartic spectrum vs WKB $G^{*}$-law")

g_scan = np.linspace(-12, 8, 41)
levels = []
for g in g_scan:
    _, _, Eg, _ = solve(g, L=10.0, N=2400, nstates=6)
    levels.append(Eg[:6])
levels = np.array(levels)
for j in range(6):
    ax2.plot(g_scan, levels[:, j], lw=1.4,
             color=[C1, C1, C2, C2, C4, C4][j], alpha=0.85)
ax2.axvline(0, color="#eb6834", lw=0.8, ls=":")
ax2.set_xlabel(r"quantum detuning $g=\mu(2m)^{1/3}/(\lambda^{2/3}\hbar^{2/3})$")
ax2.set_ylabel(r"$E_n/\varepsilon$")
ax2.set_title("(b) the quantum-rounded pitchfork")
ax2.text(-11.5, levels[:, :2].max()*0.35, "doublets\n(tunneling\nsplitting)",
         fontsize=7.5, color=C1)

Ts = np.logspace(-0.5, 1.65, 60)  # capped: >200-state truncation corrupts higher T
Cs, Us = [], []
for T in Ts:
    b = 1.0/T
    w = np.exp(-b*(E0s-E0s[0])); Z = w.sum()
    Ebar = (E0s*w).sum()/Z; E2bar = (E0s**2*w).sum()/Z
    Cs.append(b*b*(E2bar-Ebar**2))
    Us.append(((x4_d*w).sum()/Z)/(((x2_d*w).sum()/Z)**2))
ax3.semilogx(Ts, Cs, lw=1.7, color=C4, label=r"$C/k_B$")
ax3.semilogx(Ts, np.array(Us)/4, lw=1.7, color=C2, label=r"$U/4$")
ax3.axhline(0.75, color=C4, lw=0.7, ls=":")
ax3.axhline(float(G_STAR**2/16), color=C2, lw=0.7, ls=":")
ax3.text(30, 0.762, r"classical $\frac{3}{4}$", fontsize=7.5, color=C4)
ax3.text(30, float(G_STAR**2/16)+0.012, r"classical $G^{*2}/16$", fontsize=7.5, color=C2)
ax3.set_xlabel(r"temperature $k_BT/\varepsilon$")
ax3.set_title("(c) quantum freeze-out $\\to$ classical edge values")
ax3.legend(frameon=False, loc="center right")
fig.tight_layout()
fig.savefig("fig5_quantum.pdf")
fig.savefig("fig5_quantum.png", dpi=200)
print("\nfigure written: fig5_quantum", flush=True)
