"""
sim_quartic_clock_validation_v1.py

Pre-build validation simulations for PROPOSAL_QUARTIC_CLOCK_TABLETOP_v1.md.
Synthetic truth = FULL two-spring geometry (no quartic truncation)
+ bifilar-suspension gravity term + linear damping.
Analysis chain = exactly the procedure declared in the proposal (Sec. 8).

S1  Fit-model verification: elliptic T(A; mu, lambda) vs direct quadrature.
S2  Truncation bias: full-geometry period vs pure-quartic law at A/L = 0.1/0.2/0.3.
S3  End-to-end H1: ringdown -> per-cycle (T, A) -> slope gate + G* recovery,
    naive vs mu-corrected, with mu mis-pin sensitivity.
S4  H2 edge traversal: five detunings, shared-lambda global fit, mu(delta) linearity.
S5  Waveform functional B4 = 2<x^2>/<x'^2>: exact synthetic + ringdown segments.

(Restored 2026-08-05 after an untracked-file wipe; identical to the audited version.)

Run:  python sim_quartic_clock_validation_v1.py
"""

import numpy as np
from scipy.integrate import solve_ivp, quad
from scipy.optimize import least_squares
from scipy.special import ellipk, gamma

G_STAR = gamma(0.25) / gamma(0.75)

# ---------------- design point (proposal Sec. 4) ----------------
K_SPRING = 500.0     # N/m per spring
L_HALF = 0.12        # m anchor half-separation
MASS = 0.2           # kg
ELL_SUSP = 1.5       # m bifilar suspension length
G_ACC = 9.81
LAMBDA_TRUE = K_SPRING / (4 * L_HALF**2)      # quartic coefficient of truncation
MU_G = MASS * G_ACC / ELL_SUSP                # suspension harmonic term  [N/m]
DAMP_C = 0.006                                 # kg/s linear damping
WINDOW = (0.010, 0.024)                        # m, declared amplitude window

# ---------------- potentials and forces ----------------
def spring_force(x, delta):
    """Transverse restoring force of the two-spring system, FULL geometry.
    delta = L - L0 (pretension > 0, slack < 0). Two springs, k each."""
    L0 = L_HALF - delta
    r = np.sqrt(L_HALF**2 + x * x)
    e = r - L0
    return -2.0 * K_SPRING * e * x / r

def total_force(x, v, delta, mu_extra=MU_G, c=DAMP_C):
    return spring_force(x, delta) - mu_extra * x - c * v

def V_model_quartic(x, mu):
    return 0.5 * mu * x * x + LAMBDA_TRUE * x**4

# ---------------- the proposal's fit model (Sec. 2.2) ----------------
def T_model(A, lam, mu, m):
    """T = 4 sqrt(m/2) K(kappa) / sqrt(mu/2 + 2 lam A^2), kappa^2 = lam A^2/(mu/2+2 lam A^2).
    scipy ellipk takes the PARAMETER m_ell = kappa^2."""
    A = np.asarray(A, dtype=float)
    den = 0.5 * mu + 2.0 * lam * A * A
    kap2 = lam * A * A / den
    return 4.0 * np.sqrt(m / 2.0) * ellipk(kap2) / np.sqrt(den)

def T_pure(A, lam, m):
    return np.sqrt(np.pi) * G_STAR * np.sqrt(m / (2 * lam)) / A

# ---------------- direct quadrature period for arbitrary V ----------------
def period_quadrature(Vfun, A, m):
    E = Vfun(A)
    def integrand_u(u):
        q = A * np.sin(u)
        d = E - Vfun(q)
        if d <= 0:
            return 0.0
        return A * np.cos(u) / np.sqrt(d)
    val, _ = quad(integrand_u, 0, np.pi / 2, limit=200)
    return 4.0 * np.sqrt(m / 2.0) * val

# ---------------- ringdown simulation + per-cycle extraction ----------------
def simulate_ringdown(delta, x0, t_max, mu_extra=MU_G, c=DAMP_C, rtol=1e-10):
    def rhs(t, y):
        return [y[1], total_force(y[0], y[1], delta, mu_extra, c) / MASS]

    def ev_upcross(t, y):
        return y[0]
    ev_upcross.direction = 1.0

    def ev_peak(t, y):
        return y[1]
    ev_peak.direction = -1.0  # velocity falling through zero = positive peak

    sol = solve_ivp(rhs, (0.0, t_max), [x0, 0.0], events=[ev_upcross, ev_peak],
                    rtol=rtol, atol=1e-13, dense_output=True, max_step=0.05)
    t_up = sol.t_events[0]
    t_pk = sol.t_events[1]
    x_pk = np.array([sol.sol(t)[0] for t in t_pk])
    return sol, t_up, t_pk, x_pk

def per_cycle_data(t_up, t_pk, x_pk):
    """Period from successive up-crossings; amplitude = geometric mean of the
    positive peaks bracketing each cycle (proposal Sec. 6 R1)."""
    T, Amp, t_mid = [], [], []
    for i in range(len(t_up) - 1):
        mask = (t_pk > t_up[i]) & (t_pk < t_up[i + 1])
        if mask.sum() < 1:
            continue
        p_in = x_pk[mask][0]
        after = t_pk[t_pk > t_up[i + 1]]
        if len(after) == 0:
            continue
        p_next = x_pk[t_pk > t_up[i + 1]][0]
        if p_in <= 0 or p_next <= 0:
            continue
        T.append(t_up[i + 1] - t_up[i])
        Amp.append(np.sqrt(p_in * p_next))
        t_mid.append(0.5 * (t_up[i] + t_up[i + 1]))
    return np.array(T), np.array(Amp), np.array(t_mid)

# ---------------- report helpers ----------------
def line(s=""):
    print(s, flush=True)

def section(title):
    line()
    line("=" * 72)
    line(title)
    line("=" * 72)

# ================= S1: fit model vs quadrature =================
section("S1  Fit model (elliptic) vs direct quadrature  [expect ~machine agreement]")
rng_err = 0.0
for mu in [0.0, 0.5, 1.308, 5.0, -0.3]:
    for A in [0.008, 0.015, 0.024]:
        if 0.5 * mu + 2 * LAMBDA_TRUE * A * A <= 0:
            continue
        if mu < 0 and V_model_quartic(A, mu) <= 0:
            continue
        Tq = period_quadrature(lambda q: V_model_quartic(q, mu), A, MASS)
        Tm = float(T_model(A, LAMBDA_TRUE, mu, MASS))
        rel = abs(Tm / Tq - 1)
        rng_err = max(rng_err, rel)
        line(f"  mu={mu:7.3f}  A={A*100:5.2f} cm   T_quad={Tq:.9f}  T_model={Tm:.9f}  rel={rel:.2e}")
line(f"  S1 max relative deviation: {rng_err:.2e}  -> {'PASS' if rng_err < 1e-8 else 'FAIL'}")

# ================= S2: full-geometry truncation bias =================
section("S2  Full two-spring geometry vs pure-quartic law (delta=0, no gravity)")
def V_full(x):
    r = np.sqrt(L_HALF**2 + x * x)
    return K_SPRING * (r - L_HALF) ** 2   # two springs, k each: 2*(1/2)k e^2

for frac in [0.1, 0.2, 0.3]:
    A = frac * L_HALF
    Tf = period_quadrature(V_full, A, MASS)
    Tq = T_pure(A, LAMBDA_TRUE, MASS)
    line(f"  A = {frac:.1f} L = {A*100:5.2f} cm:  T_full/T_quartic - 1 = {Tf/Tq-1:+.4%}")
line("  (proposal Sec 7.4: +1.2% booked at A = 0.2 L)")

# ================= S3: end-to-end H1 =================
section("S3  End-to-end H1: ringdown, window, slope gate, G* recovery")
sol, t_up, t_pk, x_pk = simulate_ringdown(delta=0.0, x0=0.0265, t_max=140.0)
T, Amp, t_mid = per_cycle_data(t_up, t_pk, x_pk)
inwin = (Amp >= WINDOW[0]) & (Amp <= WINDOW[1])
line(f"  ringdown: {len(T)} cycles total, {inwin.sum()} inside window "
     f"[{WINDOW[0]*100:.1f}, {WINDOW[1]*100:.1f}] cm")
Tw, Aw = T[inwin], Amp[inwin]

pk_in = (x_pk >= WINDOW[0]) & (x_pk <= WINDOW[1])
if pk_in.sum() > 3:
    tt, aa = t_pk[pk_in], x_pk[pk_in]
    gam = -2 * np.polyfit(tt, np.log(aa), 1)[0]
    Q_est = (2 * np.pi / np.median(Tw)) / gam
    line(f"  effective Q in window ~ {Q_est:.0f}  (gate: > 30)")

s_naive = -np.polyfit(np.log(Aw), np.log(Tw), 1)[0]
g_naive = np.median(Tw * Aw * np.sqrt(2 * LAMBDA_TRUE / (np.pi * MASS)))
line(f"  NAIVE (ignore mu):     slope s = {s_naive:.4f}   "
     f"G*_exp = {g_naive:.5f}  ({(g_naive/G_STAR-1)*100:+.2f}% vs G*)")

def corrected_gstar(mu_pin):
    corr = T_pure(Aw, LAMBDA_TRUE, MASS) / T_model(Aw, LAMBDA_TRUE, mu_pin, MASS)
    Tc = Tw * corr
    s_c = -np.polyfit(np.log(Aw), np.log(Tc), 1)[0]
    g_c = np.median(Tc * Aw * np.sqrt(2 * LAMBDA_TRUE / (np.pi * MASS)))
    return s_c, g_c

for tag, mu_pin in [("mu pinned exactly", MU_G),
                    ("mu mis-pinned +5%", 1.05 * MU_G),
                    ("mu mis-pinned -5%", 0.95 * MU_G)]:
    s_c, g_c = corrected_gstar(mu_pin)
    ok = "PASS" if (0.97 <= s_c <= 1.03 and abs(g_c / G_STAR - 1) <= 0.02) else "FAIL"
    line(f"  CORRECTED ({tag:18s}): slope s = {s_c:.4f}   "
         f"G*_exp = {g_c:.5f}  ({(g_c/G_STAR-1)*100:+.2f}%)   H1 gate: {ok}")

# ================= S4: H2 edge traversal =================
section("S4  H2 traversal: five detunings, shared-lambda fit, mu(delta) linearity")
deltas_mm = np.array([-0.4, -0.2, 0.0, 0.2, 0.4])
data = []
for d_mm in deltas_mm:
    _, tu, tp, xp = simulate_ringdown(delta=d_mm * 1e-3, x0=0.0265, t_max=140.0)
    Td, Ad, _ = per_cycle_data(tu, tp, xp)
    m_ = (Ad >= WINDOW[0]) & (Ad <= WINDOW[1])
    data.append((Td[m_], Ad[m_]))
    line(f"  delta = {d_mm:+.1f} mm: {m_.sum()} window cycles, "
         f"T range [{Td[m_].min():.3f}, {Td[m_].max():.3f}] s")

def resid(p):
    lam = p[0]
    mus = p[1:6]
    r = []
    for (Td, Ad), mu in zip(data, mus):
        r.append((T_model(Ad, lam, mu, MASS) - Td) / Td)
    return np.concatenate(r)

p0 = np.array([LAMBDA_TRUE, 0.0, 0.6, 1.3, 2.0, 2.6])
fit = least_squares(resid, p0, method="lm")
lam_fit, mus_fit = fit.x[0], fit.x[1:6]
slope_th = 2 * K_SPRING / L_HALF
pf = np.polyfit(deltas_mm * 1e-3, mus_fit, 1)
mu_res = mus_fit - np.polyval(pf, deltas_mm * 1e-3)
ss_tot = np.sum((mus_fit - mus_fit.mean()) ** 2)
r2 = 1 - np.sum(mu_res**2) / ss_tot
line(f"  shared lambda fit: {lam_fit:.1f}  (true {LAMBDA_TRUE:.1f}, "
     f"{(lam_fit/LAMBDA_TRUE-1)*100:+.2f}%)")
line(f"  mu(delta):  slope = {pf[0]:.0f} N/m/m  (theory 2k/L = {slope_th:.0f}, "
     f"{(pf[0]/slope_th-1)*100:+.2f}%)")
line(f"              intercept = {pf[1]:.3f} N/m  (suspension mu_g = {MU_G:.3f}, "
     f"{(pf[1]/MU_G-1)*100:+.2f}%)")
line(f"              linearity R^2 = {r2:.6f}  (gate >= 0.99)")
chi2_red = np.sum(fit.fun**2) / (len(fit.fun) - 6)
line(f"  global fit reduced chi^2 (unit weights, fractional): {chi2_red:.2e}")

# ================= S5: waveform functional B4 =================
section("S5  Waveform functional  B = 2<x^2>/<x'^2>  (target: B2=2, B4=1.9678953)")

def bfunctional_from_series(ts, xs, t0, T_est, n_cycles=3):
    """Fixed-T resampling — RETAINED ONLY as the biased-control demonstration."""
    theta = np.linspace(0, 2 * np.pi * n_cycles, 4096, endpoint=False)
    tt = t0 + theta / (2 * np.pi) * T_est
    x = np.interp(tt, ts, xs)
    x = x - x.mean()
    dx = np.gradient(x, theta)
    return 2 * np.mean(x * x) / np.mean(dx * dx)

for name, mu_x, lam_x, expect in [("harmonic control", 5.0, 0.0, 2.0),
                                  ("pure quartic    ", 0.0, LAMBDA_TRUE, 48*np.pi/G_STAR**4)]:
    def rhs(t, y, mu_=mu_x, lam_=lam_x):
        return [y[1], -(mu_ * y[0] + 4 * lam_ * y[0] ** 3) / MASS]
    A0 = 0.02
    Tex = period_quadrature(lambda q: 0.5 * mu_x * q * q + lam_x * q**4, A0, MASS)
    solx = solve_ivp(rhs, (0, 8 * Tex), [A0, 0], rtol=1e-11, atol=1e-14,
                     dense_output=True, max_step=Tex / 200)
    ts = np.linspace(0, 8 * Tex, 20000)
    xs = solx.sol(ts)[0]
    B = bfunctional_from_series(ts, xs, 2 * Tex, Tex, 3)
    line(f"  {name}: B = {B:.7f}   (exact {expect:.7f}, dev {B-expect:+.1e})")

# ringdown segments (full apparatus truth, with gravity + damping)
ts = np.linspace(sol.t[0], sol.t[-1], 300000)
xs = sol.sol(ts)[0]

def bfunctional_per_cycle(ts, xs, t_bounds):
    """Chirp-robust variant: map EACH cycle onto its own [0, 2pi) before
    concatenating, so amplitude-dependent period drift cannot smear phase."""
    npts = 1024
    x_all = []
    for j in range(len(t_bounds) - 1):
        th = np.linspace(0, 2 * np.pi, npts, endpoint=False)
        tt = t_bounds[j] + th / (2 * np.pi) * (t_bounds[j + 1] - t_bounds[j])
        x_all.append(np.interp(tt, ts, xs))
    dx = np.concatenate([np.gradient(xc - xc.mean(),
                                     np.linspace(0, 2*np.pi, npts, endpoint=False))
                         for xc in x_all])
    xc_all = np.concatenate([xc - xc.mean() for xc in x_all])
    return 2 * np.mean(xc_all**2) / np.mean(dx**2)

line("  ringdown segments — FIXED-T resampling (biased by chirp) vs PER-CYCLE mapping:")
for i in np.linspace(5, len(T) - 8, 6).astype(int):
    B_fixed = bfunctional_from_series(ts, xs, t_up[i], T[i], 3)
    B_pc = bfunctional_per_cycle(ts, xs, t_up[i:i + 4])
    line(f"    A ~ {Amp[i]*100:5.2f} cm:  B_fixedT = {B_fixed:.5f}   B_percycle = {B_pc:.5f}")
line(f"  (harmonic pull -> 2 at small A; pure-quartic target {48*np.pi/G_STAR**4:.5f})")

def rhs_pq(t, y):
    return [y[1], -4 * LAMBDA_TRUE * y[0] ** 3 / MASS]
A0 = 0.02
Tex = T_pure(A0, LAMBDA_TRUE, MASS)
solq = solve_ivp(rhs_pq, (0, 5 * Tex), [A0, 0], rtol=1e-11, atol=1e-14,
                 dense_output=True, max_step=Tex / 200)
tsq = np.linspace(0, 5 * Tex, 40000)
xsq = solq.sol(tsq)[0]
bounds = [0.25 * Tex + k * Tex for k in range(4)]
B_ctl = bfunctional_per_cycle(tsq, xsq, bounds)
line(f"  extractor control (pure quartic, undamped): B_percycle = {B_ctl:.6f} "
     f"(exact {48*np.pi/G_STAR**4:.6f})")

section("Done.")
