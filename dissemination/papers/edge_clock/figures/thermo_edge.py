"""thermo_edge.py — exact equilibrium statistical mechanics of the marginal mode.
V(x) = (1/2) mu x^2 + lambda x^4 in a bath at temperature T; beta = 1/kBT.
Everything configurational is a universal function of gamma = mu*sqrt(beta/lambda).

Derives + numerically verifies:
  T1  Scaling: <x^2>*sqrt(beta*lambda) = f2(gamma);   f2(0) = 1/G*.
  T2  Binder ratio U(gamma) = <x^4>/<x^2>^2;  U(0) = G*^2/4 = Gamma(1/4)^4/(8 pi^2);
      limits: U(+inf) = 3 (Gaussian), U(-inf) = 1 (two-state).
  T3  Generalized equipartition: <E>_crit = (3/4) kBT;  C_crit = (3/4) kB;
      C(gamma) universal, -> kB in both far limits.
  T4  Laplace bridge: Z_osc(beta) = int_0^inf T(E) e^{-beta E} dE equals the
      phase-space integral of e^{-beta H} (mu >= 0 branch), i.e. the
      thermodynamics is the Laplace transform of the clock curve 4kK.
  T5  Thermal lineshape: undamped thermal ensemble of a critical mode has
      frequency distribution with peak scaling f_peak ~ T^{1/4}
      (harmonic mode: T-independent). Exact reduced lineshape derived.
"""
import numpy as np
import mpmath as mp
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

mp.mp.dps = 30
G_STAR = mp.gamma(mp.mpf(1)/4) / mp.gamma(mp.mpf(3)/4)

def line(s=""):
    print(s, flush=True)

# ---------- reduced configurational measure: w(u) = exp(-(g/2) u^2 - u^4) ----------
def moment(g, n):
    f = lambda u: u**n * mp.e**(-(mp.mpf(g)/2)*u*u - u**4)
    num = mp.quad(f, [0, mp.inf]) if n % 2 == 0 else 0
    den = mp.quad(lambda u: mp.e**(-(mp.mpf(g)/2)*u*u - u**4), [0, mp.inf])
    return num/den

line("T1/T2 — critical values (gamma = 0):")
m2_0 = moment(0, 2); m4_0 = moment(0, 4)
line(f"  <u^2>(0) = {mp.nstr(m2_0, 12)}   vs 1/G* = {mp.nstr(1/G_STAR, 12)}")
line(f"  U(0) = <u^4>/<u^2>^2 = {mp.nstr(m4_0/m2_0**2, 12)}")
line(f"  G*^2/4              = {mp.nstr(G_STAR**2/4, 12)}")
line(f"  Gamma(1/4)^4/(8pi^2) = {mp.nstr(mp.gamma(mp.mpf(1)/4)**4/(8*mp.pi**2), 12)}")

line("\nT2 — Binder limits:")
for g, expect in [(60, "-> 3 (Gaussian)"), (-60, "-> 1 (two-state)")]:
    m2 = moment(g, 2); m4 = moment(g, 4)
    line(f"  U(gamma={g:+d}) = {mp.nstr(m4/m2**2, 8)}   {expect}")

# ---------- T3: energy and heat capacity ----------
# <V> in reduced units: kBT * <(g/2)u^2 + u^4>_w ; total <E> = kBT/2 (kinetic) + <V>.
# C/kB = d<E>/d(kBT) at fixed mu, lambda  -> compute numerically in physical units.
def avgE_phys(kT, mu, lam):
    """<E> per mode (kinetic + potential), direct quadrature in physical units."""
    b = 1.0/kT
    V = lambda x: 0.5*mu*x*x + lam*x**4
    Zx = mp.quad(lambda x: mp.e**(-b*V(x)), [0, mp.inf])
    EV = mp.quad(lambda x: V(x)*mp.e**(-b*V(x)), [0, mp.inf]) / Zx
    return 0.5*kT + EV

line("\nT3 — generalized equipartition & heat capacity:")
kT = 1.0
E_crit = avgE_phys(kT, 0.0, 1.0)
line(f"  <E>_crit / kBT = {mp.nstr(E_crit/kT, 10)}   (exact 3/4)")
h = 1e-4
for name, mu in [("critical mu=0", 0.0), ("harmonic-ish mu=40", 40.0),
                 ("double-well mu=-8", -8.0)]:
    C = (avgE_phys(kT+h, mu, 1.0) - avgE_phys(kT-h, mu, 1.0)) / (2*h)
    line(f"  C/kB ({name:18s}) = {mp.nstr(C, 8)}")

# C(gamma) universal curve: C/kB = 1/2 + d<v>/dt where scaling handles it;
# compute via physical units sweeping mu at fixed kT=lam=1: gamma = mu.
gammas = np.linspace(-10, 14, 97)
Cvals = []
for g in gammas:
    C = (avgE_phys(kT+h, g, 1.0) - avgE_phys(kT-h, g, 1.0)) / (2*h)
    Cvals.append(float(C))
Cvals = np.array(Cvals)

# Binder curve
Uvals = np.array([float(moment(g, 4)/moment(g, 2)**2) for g in gammas])

# scaling-collapse check for f2: <x^2>sqrt(beta lam) same for two (kT, lam) at equal gamma
def x2_phys(kT, mu, lam):
    b = 1.0/kT
    V = lambda x: 0.5*mu*x*x + lam*x**4
    Zx = mp.quad(lambda x: mp.e**(-b*V(x)), [0, mp.inf])
    return mp.quad(lambda x: x*x*mp.e**(-b*V(x)), [0, mp.inf]) / Zx

g_test = 3.0
a = x2_phys(1.0, 3.0, 1.0) * mp.sqrt(1.0*1.0)          # kT=1, lam=1, mu=3 -> gamma=3
b_ = x2_phys(4.0, 3.0, 0.25/4) * mp.sqrt((1/4.0)*(0.25/4))**1  # gamma = mu sqrt(beta/lam)
# choose second point cleanly: kT=2, lam=2, mu = gamma*sqrt(lam/beta)= 3*sqrt(2*2)=6
c_ = x2_phys(2.0, 6.0, 2.0) * mp.sqrt((1/2.0)*2.0)
line(f"\nT1 — collapse check f2(gamma=3): {mp.nstr(a,10)} vs {mp.nstr(c_,10)} (different kT, lam)")

# ---------- T4: Laplace bridge (mu > 0 branch) ----------
line("\nT4 — Laplace bridge  Z_osc = int T(E) e^{-beta E} dE  vs  phase-space integral:")
m_mass, mu_t, lam_t, beta_t = 1.0, 2.0, 1.0, 1.3
from scipy.integrate import quad as squad
from scipy.special import ellipk

def T_of_E(E):
    # invert E = mu A^2/2 + lam A^4 for A^2, then T via 4kK form
    A2 = (-mu_t/2 + np.sqrt(mu_t**2/4 + 4*lam_t*E)) / (2*lam_t)
    k2 = 2*lam_t*A2 / (mu_t + 4*lam_t*A2)
    tau = 4*np.sqrt(k2)*ellipk(k2)          # = T*A*sqrt(2 lam/m)
    return tau / (np.sqrt(A2) * np.sqrt(2*lam_t/m_mass))

Z_laplace = squad(lambda E: T_of_E(E)*np.exp(-beta_t*E), 0, 60, limit=300)[0]
Zp = np.sqrt(2*np.pi*m_mass/beta_t)
Zx = 2*float(mp.quad(lambda x: mp.e**(-beta_t*(0.5*mu_t*x*x + lam_t*x**4)), [0, mp.inf]))
Z_direct = Zp*Zx
line(f"  int T(E)e^-bE dE = {Z_laplace:.10f}")
line(f"  phase-space Z    = {Z_direct:.10f}   ratio = {Z_laplace/Z_direct:.10f}")


# ---------- T4b: COMPLETED bridge for mu < 0 (two intra-well families + over-barrier) ----------
line("")
line("T4b — completed double-well bridge (mu < 0): both orbit families required:")
mu_n = -2.0
Emin_n = -mu_n**2/(16*lam_t)
def V_n(x): return 0.5*mu_n*x*x + lam_t*x**4
def T_over_n(E):
    A2 = (-mu_n/2 + np.sqrt(mu_n**2/4 + 4*lam_t*E))/(2*lam_t); A = np.sqrt(A2)
    f = lambda u: A*np.cos(u)/np.sqrt(max(E - V_n(A*np.sin(u)), 1e-300))
    return 4*np.sqrt(m_mass/2)*squad(f, 0, np.pi/2, limit=300)[0]
def T_well_n(E):
    disc = np.sqrt(mu_n**2/4 + 4*lam_t*E)
    yl = (-mu_n/2 - disc)/(2*lam_t); yr = (-mu_n/2 + disc)/(2*lam_t)
    xl, xr = np.sqrt(yl), np.sqrt(yr)
    f = lambda th: (xr-xl)/2*np.sin(th)/np.sqrt(max(E - V_n((xl+xr)/2 - (xr-xl)/2*np.cos(th)), 1e-300))
    return 2*np.sqrt(m_mass/2)*squad(f, 0, np.pi, limit=400)[0]
Z_sub  = squad(lambda E: 2*T_well_n(E)*np.exp(-beta_t*E), Emin_n, -1e-9, limit=400)[0]
Z_ovr  = squad(lambda E: T_over_n(E)*np.exp(-beta_t*E), 1e-12, 80, limit=400)[0]
Zx_n = 2*float(mp.quad(lambda x: mp.e**(-beta_t*(0.5*mu_n*x*x + lam_t*x**4)), [0, mp.inf]))
Z_dir_n = np.sqrt(2*np.pi*m_mass/beta_t)*Zx_n
line(f"  Z(two wells) + Z(over) = {Z_sub+Z_ovr:.10f}   direct = {Z_dir_n:.10f}   ratio = {(Z_sub+Z_ovr)/Z_dir_n:.10f}")
line(f"  over-barrier family alone omits {100*(1-Z_ovr/Z_dir_n):.2f}% of Z  (the E in [{Emin_n:.2f}, 0) intra-well orbits)")

# ---------- T5: thermal lineshape of the critical mode ----------
# each orbit of energy E has frequency f(E) = 1/T(E) prop-to E^{1/4} (pure quartic);
# density of states weight: dxdp shell = T(E) dE. Frequency distribution:
#   P(f) df prop-to T(E) e^{-beta E} dE  with  E prop-to f^4  =>  P(f) prop-to f^{-1} e^{-c f^4} f^3 df
#   => P(f) prop-to f^2 exp(-(f/f_T)^4),  f_T prop-to (kBT)^{1/4}.
# peak at f* = (1/2)^{1/4} f_T  -> f_peak prop-to T^{1/4}.
line("\nT5 — critical-mode thermal frequency distribution: P(f) prop-to f^2 exp(-(f/f_T)^4)")
# verify by direct Monte Carlo over phase space (pure quartic, m=lam=1, kT=1 and kT=16):
rng = np.random.default_rng(3)
def sample_freqs(kT, n=200000):
    # sample (x, p) ~ e^{-beta H} by rejection using x ~ quartic marginal, p gaussian
    b = 1.0/kT
    # sample x via inverse-free rejection: proposal normal with sigma matched
    sig = (kT)**0.25
    xs = rng.normal(0, 2*sig, size=4*n)
    w = np.exp(-b*xs**4 + xs**2/(2*(2*sig)**2))
    keep = rng.random(4*n) < w/w.max()
    xs = xs[keep][:n]
    ps = rng.normal(0, np.sqrt(kT), size=len(xs))
    E = ps**2/2 + xs**4
    Tper = np.sqrt(np.pi)*float(G_STAR)*np.sqrt(1/2.0)/E**0.25   # A = E^{1/4}
    return 1.0/Tper
f1 = sample_freqs(1.0); f16 = sample_freqs(16.0)
r = np.median(f16)/np.median(f1)
line(f"  median f at kT=16 / kT=1 = {r:.4f}   (T^(1/4) law predicts 16^0.25 = 2.0)")
mode1 = np.argmax(np.histogram(f1, bins=200)[0])
line(f"  MC histogram peak vs analytic (1/2)^(1/4) f_T: qualitative check in figure")

# ---------- Figure 4 ----------
plt.rcParams.update({"font.size": 9, "axes.titlesize": 9.5, "legend.fontsize": 8,
                     "axes.spines.top": False, "axes.spines.right": False})
C1, C2, C4 = "#2a78d6", "#eb6834", "#4a3aa7"
fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(9.6, 2.7))

ax1.plot(gammas, Uvals, color=C4, lw=1.8)
ax1.axhline(3, color=C1, lw=0.7, ls=":"); ax1.axhline(1, color="gray", lw=0.7, ls=":")
ax1.axhline(float(G_STAR**2/4), color=C2, lw=0.7, ls=":")
ax1.plot([0], [float(G_STAR**2/4)], "o", ms=5, color=C2)
ax1.text(1.2, float(G_STAR**2/4)+0.06, r"$U(0)=G^{*2}/4=\Gamma(\frac{1}{4})^4/8\pi^2$",
         fontsize=7.5, color=C2)
ax1.text(9, 3.04, "Gaussian 3", fontsize=7.5, color=C1)
ax1.text(-9.5, 1.05, "two-state 1", fontsize=7.5, color="gray")
ax1.set_xlabel(r"thermal detuning $\gamma=\mu\sqrt{\beta/\lambda}$")
ax1.set_ylabel(r"$U=\langle x^4\rangle/\langle x^2\rangle^2$")
ax1.set_title("(a) Binder ratio across the edge")

ax2.plot(gammas, Cvals, color=C4, lw=1.8)
ax2.axhline(1.0, color=C1, lw=0.7, ls=":")
ax2.axhline(0.75, color=C2, lw=0.7, ls=":")
ax2.plot([0], [0.75], "o", ms=5, color=C2)
ax2.text(1.0, 0.755, r"$C(0)=\frac{3}{4} k_B$", fontsize=8, color=C2)
ax2.text(8, 1.005, r"harmonic $k_B$", fontsize=7.5, color=C1)
ax2.set_xlabel(r"$\gamma$")
ax2.set_ylabel(r"$C/k_B$ per mode")
ax2.set_title("(b) equipartition deficit at the edge")

fgrid = np.linspace(0.01, 3.2, 400)
for kTv, c in [(1.0, C1), (4.0, C4), (16.0, C2)]:
    fT = kTv**0.25 / (np.sqrt(np.pi)*float(G_STAR)*np.sqrt(0.5))
    P = fgrid**2*np.exp(-(fgrid/fT)**4)
    P /= P.max()
    ax3.plot(fgrid, P, color=c, lw=1.6, label=rf"$k_BT={kTv:g}$")
hist, edges = np.histogram(f1, bins=80, range=(0.01, 3.2), density=True)
ax3.plot(0.5*(edges[1:]+edges[:-1]), hist/hist.max(), ".", ms=3, color=C1, alpha=0.6)
ax3.set_xlabel(r"frequency $f$ (reduced)")
ax3.set_ylabel(r"$P(f)$ (norm.)")
ax3.set_title(r"(c) thermal lineshape, $f_T\propto T^{1/4}$")
ax3.legend(frameon=False)
fig.tight_layout()
fig.savefig("fig4_thermo.pdf")
fig.savefig("fig4_thermo.png", dpi=200)
line("\nfigure written: fig4_thermo")
