"""Figures for 'The clock at the edge of stability'. Reproducible from scratch.
Fig 1: the edge traversal (potential family) + period-vs-amplitude landscape.
Fig 2: simulated ringdown validation: calibration-free G* recovery + waveform functional.
Fig 3: the exact crossover pair (universal collapse 4kK + closed-form B(k)) — new results.
(Master copy lives in the session scratchpad; the repo copy has twice been destroyed
by a concurrently running agent session (since finished). Restore with a plain file copy.)
"""
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from scipy.integrate import solve_ivp
from scipy.special import ellipk, gamma

G_STAR = gamma(0.25) / gamma(0.75)
B4_EXACT = 48 * np.pi / G_STAR**4

# design point (matches proposal)
K_S, L, MASS, ELL, C_DAMP = 500.0, 0.12, 0.2, 1.5, 0.006
LAM = K_S / (4 * L**2)
MU_G = MASS * 9.81 / ELL
WINDOW = (0.010, 0.024)

plt.rcParams.update({
    "font.size": 9, "axes.labelsize": 9, "axes.titlesize": 9.5,
    "legend.fontsize": 8, "figure.dpi": 150,
    "axes.spines.top": False, "axes.spines.right": False,
})
C1, C2, C3, C4 = "#2a78d6", "#eb6834", "#1baf7a", "#4a3aa7"

# ---------------- Fig 1 ----------------
def V_of(x, delta):
    L0 = L - delta
    e = np.sqrt(L**2 + x**2) - L0
    return K_S * e**2 - K_S * (L - L0)**2   # zero at x=0

def T_model(A, lam, mu, m):
    A = np.asarray(A, float)
    den = 0.5 * mu + 2 * lam * A * A
    return 4 * np.sqrt(m / 2) * ellipk(lam * A * A / den) / np.sqrt(den)

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(6.6, 2.6))
x = np.linspace(-0.035, 0.035, 400)
for d_mm, c, lab in [(0.4, C1, r"taut $\mu>0$ (harmonic, $n=2$)"),
                     (0.0, C2, r"critical $\mu=0$ (quartic, $n=4$)"),
                     (-0.4, C3, r"slack $\mu<0$ (double well)")]:
    ax1.plot(x * 100, 1e3 * V_of(x, d_mm * 1e-3), color=c, lw=1.8, label=lab)
ax1.set_xlabel("displacement $x$ (cm)")
ax1.set_ylabel("$V(x)$ (mJ)")
ax1.set_title("(a) one micrometer knob crosses the edge")
ax1.legend(frameon=False, loc="upper center")

A = np.linspace(0.002, 0.030, 400)
for mu, c, lab in [(6.0, C1, r"$\mu=6$ N/m"), (1.5, C4, r"$\mu=1.5$ N/m"),
                   (0.3, C3, r"$\mu=0.3$ N/m"), (0.0, C2, r"$\mu=0$ (pure quartic)")]:
    ax2.loglog(A * 100, T_model(A, LAM, mu, MASS), color=c, lw=1.8, label=lab)
ax2.loglog(A * 100, np.sqrt(np.pi) * G_STAR * np.sqrt(MASS / (2 * LAM)) / A,
           ls=":", color="k", lw=1, label=r"$T=\sqrt{\pi}\,G^{*}\sqrt{m/2\lambda}/A$")
ax2.set_xlabel("amplitude $A$ (cm)")
ax2.set_ylabel("period $T$ (s)")
ax2.set_title("(b) harmonic plateau $\\to$ quartic clock law")
ax2.legend(frameon=False, loc="lower left", ncol=1)
fig.tight_layout()
fig.savefig("fig1_edge.pdf")
fig.savefig("fig1_edge.png", dpi=200)

# ---------------- ringdown simulation (full geometry + gravity + damping) ----------------
def rhs(t, y):
    r = np.sqrt(L**2 + y[0]**2)
    F = -2 * K_S * (r - L) * y[0] / r - MU_G * y[0] - C_DAMP * y[1]
    return [y[1], F / MASS]

ev_up = lambda t, y: y[0]; ev_up.direction = 1.0
ev_pk = lambda t, y: y[1]; ev_pk.direction = -1.0
sol = solve_ivp(rhs, (0, 140), [0.0265, 0], events=[ev_up, ev_pk],
                rtol=1e-10, atol=1e-13, dense_output=True, max_step=0.05)
t_up, t_pk = sol.t_events[0], sol.t_events[1]
x_pk = np.array([sol.sol(t)[0] for t in t_pk])

T_list, A_list = [], []
for i in range(len(t_up) - 1):
    m1 = (t_pk > t_up[i]) & (t_pk < t_up[i + 1])
    after = t_pk[t_pk > t_up[i + 1]]
    if m1.sum() < 1 or len(after) == 0:
        continue
    p1, p2 = x_pk[m1][0], x_pk[t_pk > t_up[i + 1]][0]
    if p1 > 0 and p2 > 0:
        T_list.append(t_up[i + 1] - t_up[i])
        A_list.append(np.sqrt(p1 * p2))
T_arr, A_arr = np.array(T_list), np.array(A_list)

def T_pure(A):
    return np.sqrt(np.pi) * G_STAR * np.sqrt(MASS / (2 * LAM)) / A

g_naive = T_arr * A_arr * np.sqrt(2 * LAM / (np.pi * MASS))
corr = T_pure(A_arr) / T_model(A_arr, LAM, MU_G, MASS)
g_corr = T_arr * corr * A_arr * np.sqrt(2 * LAM / (np.pi * MASS))

ts = np.linspace(sol.t[0], sol.t[-1], 300000)
xs = sol.sol(ts)[0]
def B_percycle(bounds):
    npts = 1024
    xc_all, dx_all = [], []
    for j in range(len(bounds) - 1):
        th = np.linspace(0, 2 * np.pi, npts, endpoint=False)
        tt = bounds[j] + th / (2 * np.pi) * (bounds[j + 1] - bounds[j])
        xc = np.interp(tt, ts, xs)
        xc_all.append(xc - xc.mean())
        dx_all.append(np.gradient(xc - xc.mean(), th))
    xc = np.concatenate(xc_all); dx = np.concatenate(dx_all)
    return 2 * np.mean(xc**2) / np.mean(dx**2)

idx = np.arange(2, len(T_arr) - 4, 3)
B_vals = np.array([B_percycle(t_up[i:i + 4]) for i in idx])
A_B = A_arr[idx]

# ---------------- Fig 2 ----------------
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(6.6, 2.6))
win = (A_arr >= WINDOW[0]) & (A_arr <= WINDOW[1])
ax1.plot(A_arr * 100, g_naive, ".", ms=3, color=C2, label="uncorrected")
ax1.plot(A_arr * 100, g_corr, ".", ms=3, color=C1, label=r"suspension term modeled")
ax1.axhline(G_STAR, color="k", lw=0.8, ls=":")
ax1.text(2.62, G_STAR + 0.008, r"$G^{*}=\Gamma(\frac{1}{4})/\Gamma(\frac{3}{4})$",
         ha="right", va="bottom", fontsize=8)
ax1.axvspan(WINDOW[0] * 100, WINDOW[1] * 100, color=C1, alpha=0.07, lw=0)
ax1.set_xlabel("cycle amplitude $A$ (cm)")
ax1.set_ylabel(r"$T\,A\sqrt{2\lambda/\pi m}$")
ax1.set_ylim(2.55, 3.15)
ax1.set_title("(a) $G^{*}$ recovery, no fitted scale (simulated)")
ax1.legend(frameon=False, loc="lower right")

ax2.plot(A_B * 100, B_vals, "o-", ms=3.5, lw=1, color=C4)
ax2.axhline(2.0, color=C1, lw=0.8, ls=":")
ax2.axhline(B4_EXACT, color=C2, lw=0.8, ls=":")
ax2.text(1.0, 1.9993, r"harmonic $\mathcal{B}_2=2$", ha="left", va="top",
         fontsize=8, color=C1)
ax2.text(1.0, B4_EXACT + 0.0008, r"lemniscatic $\mathcal{B}_4=48\pi/G^{*4}$",
         ha="left", va="bottom", fontsize=8, color=C2)
ax2.set_xlabel("segment amplitude $A$ (cm)")
ax2.set_ylabel(r"$\mathcal{B}=2\langle x^2\rangle/\langle x'^2\rangle$")
ax2.set_ylim(1.9645, 2.003)
ax2.set_title("(b) waveform functional")
fig.tight_layout()
fig.savefig("fig2_recovery.pdf")
fig.savefig("fig2_recovery.png", dpi=200)

print("G* =", G_STAR)
print("windowed corrected median:", np.median(g_corr[win]),
      f"({(np.median(g_corr[win])/G_STAR-1)*100:+.2f}%)")
print("B range:", B_vals.min(), "->", B_vals.max())

# ================= Fig 3: the exact crossover pair (NEW RESULTS) =================
from scipy.special import ellipe
from scipy.integrate import quad

def B_exact(k2):
    """Closed-form waveform functional along the crossover, x = A cn(u,k)."""
    k2 = np.asarray(k2, float)
    K = ellipk(k2); E = ellipe(k2)
    kp2 = 1 - k2
    return (3 * np.pi**2 / (2 * K**2)) * (E - kp2 * K) / (kp2 * K + (2 * k2 - 1) * E)

def tau_exact(k2):
    """Universal period curve: T*A*sqrt(2 lambda/m) = 4 k K(k)."""
    return 4 * np.sqrt(k2) * ellipk(k2)

# --- verification block (printed; the paper's claims rest on these) ---
print("\n[verify] B_exact(1/2)  =", float(B_exact(0.5)), " target", B4_EXACT)
print("[verify] B_exact(k2->0)=", float(B_exact(1e-9)), " target 2")
print("[verify] tau_exact(1/2)=", float(tau_exact(0.5)), " target sqrt(pi)*G* =",
      np.sqrt(np.pi) * G_STAR)
mu_t, A_t = 2.0, 0.017
k2_t = 2 * LAM * A_t**2 / (mu_t + 4 * LAM * A_t**2)
E_t = 0.5 * mu_t * A_t**2 + LAM * A_t**4
def integ(u):
    q = A_t * np.sin(u)
    d = E_t - (0.5 * mu_t * q * q + LAM * q**4)
    return A_t * np.cos(u) / np.sqrt(d) if d > 0 else 0.0
T_t = 4 * np.sqrt(MASS / 2) * quad(integ, 0, np.pi / 2, limit=200)[0]
print("[verify] generic point: T*A*sqrt(2lam/m) =", T_t * A_t * np.sqrt(2 * LAM / MASS),
      " vs 4kK =", float(tau_exact(k2_t)))
mu_x, A_x = 2.0, 0.017
def rhs_x(t, y, mu_=mu_x):
    return [y[1], -(mu_ * y[0] + 4 * LAM * y[0] ** 3) / MASS]
Tex = T_t
solx = solve_ivp(rhs_x, (0, 6 * Tex), [A_x, 0], rtol=1e-11, atol=1e-14,
                 dense_output=True, max_step=Tex / 300)
tsx = np.linspace(0, 6 * Tex, 40000)
xsx = solx.sol(tsx)[0]
theta = np.linspace(0, 2 * np.pi, 4096, endpoint=False)
ttx = 1.0 * Tex + theta / (2 * np.pi) * Tex
xw = np.interp(ttx, tsx, xsx); xw -= xw.mean()
dxw = np.gradient(xw, theta)
B_num = 2 * np.mean(xw**2) / np.mean(dxw**2)
print("[verify] generic point: B_sim =", B_num, " vs B_exact =", float(B_exact(k2_t)))

# --- collapse data: five detunings from full-geometry ringdowns ---
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(6.6, 2.6))
k2_grid = np.linspace(0.02, 0.93, 300)
ax1.plot(k2_grid, tau_exact(k2_grid), "k-", lw=1.2,
         label=r"$4kK(k)$ (exact)")
markers = ["o", "s", "^", "D", "v"]
for (d_mm, mk) in zip([-0.4, -0.2, 0.0, 0.2, 0.4], markers):
    mu_tot = 2 * K_S * (d_mm * 1e-3) / L + MU_G
    def rhs_d(t, y, mu_=d_mm * 1e-3):
        r = np.sqrt(L**2 + y[0]**2)
        F = -2 * K_S * (r - (L - mu_)) * y[0] / r - MU_G * y[0] - C_DAMP * y[1]
        return [y[1], F / MASS]
    ev_u = lambda t, y: y[0]; ev_u.direction = 1.0
    ev_p = lambda t, y: y[1]; ev_p.direction = -1.0
    sd = solve_ivp(rhs_d, (0, 140), [0.0265, 0], events=[ev_u, ev_p],
                   rtol=1e-10, atol=1e-13, dense_output=True, max_step=0.05)
    tu, tp = sd.t_events[0], sd.t_events[1]
    xp = np.array([sd.sol(t)[0] for t in tp])
    Td, Ad = [], []
    for i in range(len(tu) - 1):
        m1 = (tp > tu[i]) & (tp < tu[i + 1])
        if m1.sum() < 1 or (tp > tu[i + 1]).sum() < 1:
            continue
        p1, p2 = xp[m1][0], xp[tp > tu[i + 1]][0]
        if p1 > 0 and p2 > 0:
            Td.append(tu[i + 1] - tu[i]); Ad.append(np.sqrt(p1 * p2))
    Td, Ad = np.array(Td), np.array(Ad)
    mwin = (Ad >= 0.006) & (Ad <= 0.024)
    Td, Ad = Td[mwin], Ad[mwin]
    k2_d = 2 * LAM * Ad**2 / (mu_tot + 4 * LAM * Ad**2)
    tau_d = Td * Ad * np.sqrt(2 * LAM / MASS)
    ax1.plot(k2_d[::4], tau_d[::4], mk, ms=3, alpha=0.75,
             label=rf"$\delta={d_mm:+.1f}$ mm")
ax1.axvline(0.5, color="#eb6834", lw=0.8, ls=":")
ax1.text(0.30, 8.2, "quartic point\n$k^2=\\frac{1}{2}$", fontsize=7.5, color="#eb6834")
ax1.set_xlabel(r"elliptic modulus $k^{2}=2\lambda A^{2}/(\mu+4\lambda A^{2})$")
ax1.set_ylabel(r"$T\,A\sqrt{2\lambda/m}$")
ax1.set_title("(a) universal collapse of the crossover")
ax1.legend(frameon=False, fontsize=7, loc="lower right")

ax2.plot(k2_grid, B_exact(k2_grid), "k-", lw=1.2, label=r"$\mathcal{B}(k)$ exact")
k2_B = 2 * LAM * A_B**2 / (MU_G + 4 * LAM * A_B**2)
ax2.plot(k2_B, B_vals, "o", ms=3.5, color="#4a3aa7",
         label="simulated ringdown")
ax2.axvline(0.5, color="#eb6834", lw=0.8, ls=":")
ax2.axhline(B4_EXACT, color="#eb6834", lw=0.6, ls=":")
ax2.axhline(2.0, color="#2a78d6", lw=0.6, ls=":")
ax2.set_xlabel(r"$k^{2}$")
ax2.set_ylabel(r"$\mathcal{B}$")
ax2.set_title("(b) waveform functional: exact, zero param.")
ax2.legend(frameon=False, fontsize=7.5, loc="lower left")
fig.tight_layout()
fig.savefig("fig3_crossover.pdf")
fig.savefig("fig3_crossover.png", dpi=200)
print("figures written")
