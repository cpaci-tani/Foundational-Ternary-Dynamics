"""proof_pl1_deviation.py — FTD-0359: PL-1 quantified deviation (Rice vs Born).

STATUS / SCOPE BANNER (read before citing)
------------------------------------------
Everything verified here is a statement ABOUT THE [IMPOSED] LANGEVIN MODEL of
DERIV_BORN_PROPORTIONALITY.md §3 (Gaussian noise ensemble sigma_n added by hand
to the deterministic substrate), NOT a theorem about the FTD substrate itself.
Per the FTD-0356 demotion banner and LEDGER FTD-0187/FTD-0200:

  * FTD-0187 T1c (probability = energy density) remains [OPEN].
  * FTD-0200 is [CLOSED NEGATIVE for Born] in the 6-neighbour substrate; the
    canonical 26-neighbour engine question requires a PRE-REGISTERED v2 test.
  * This script performs ZERO promotions. It quantifies the deviation
    structure of the already-recorded Rice law, forward-derived (no fitting
    to any known experiment).

WHAT IS VERIFIED (all within the imposed model)
-----------------------------------------------
Setup: scalar flux J = J_coh + dJ, dJ a stationary differentiable Gaussian
process, variance sigma_n^2; manifestation = crossing of +K_B (upward) or
-K_B (downward). Dimensionless: beta = K_B/sigma_n, x = |J_coh|/sigma_n,
Itilde = x^2 = I/sigma_n^2.

  [T1] Exact rate law:  nu_tot(x)/nu_0 = exp(-x^2/2) * cosh(beta*x),
       where nu_0 = 2R exp(-beta^2/2), R = sigma_Jdot/(2*pi*sigma_n).
  [T2] Hermite-tower theorem: nu_tot/nu_0 = sum_{m>=0} He_{2m}(beta) x^{2m}/(2m)!
       (probabilists' Hermite; via generating function e^{tx-x^2/2}).
       => a2 = He_2(beta)/2!  = (beta^2-1)/2                (leading, Born-like)
          a4 = He_4(beta)/4!  = (beta^4-6beta^2+3)/24       (first correction)
          a6 = He_6(beta)/6!  = (beta^6-15beta^4+45beta^2-15)/720
  [T3] Linear-term (odd-order) cancellation: rate even in J_coh, exactly.
  [T4] Sign condition: a2 > 0 iff beta > 1 (sigma_n < K_B); anti-Born otherwise.
  [T5] Born-mimicking point: a4 = 0 at beta^2 = 3 + sqrt(6)
       (beta* ~ 2.3344); there the leading deviation is a6 < 0 at O(x^6).
  [T6] Fractional deviation from Born:
       D(x) = [ (f(x)-1) / (a2 x^2) ] - 1 = (a4/a2) x^2 + O(x^4).
  [T7] Observable projections (quadratic-response regime):
       - balanced two-beam fringe second harmonic  c2/c1 = eps/(4(1+eps)),
         eps = (a4/a2)*Itilde_peak;
       - unbalanced-fringe visibility shift V_meas = V(1+2e)/(1+e(1+V^2)),
         e = (a4/a2)*Itilde_mean  =>  dV/V ~ e(1-V^2);
       - source-statistics calibration split (same mean intensity):
         excess_thermal/excess_coherent - 1 ~ (a4/a2)*Itilde*(g2-1), g2_th = 2.
  [T8] Saturation/rollover: exact rate peaks at x* solving x = beta tanh(beta x)
       (J*_coh ~ K_B from below), peak value ~ (1/2)exp(beta^2/2) * nu_0,
       and collapses ~ (1/2)nu_0 at J_coh = 2 K_B, ->0 beyond (anti-Born tail).
  [MC] Rice's formula itself validated by direct Monte-Carlo level-crossing
       counts on a synthesized stationary Gaussian process (no fitting).

Companion doc:
  docs/theory/03_derivations/quantum_mechanics/ANALYSIS_PL1_QUANTIFIED_DEVIATION.md

Run:  python scripts/proofs/proof_pl1_deviation.py
"""

import math

import numpy as np
import sympy as sp

CHECKS = []


def check(name, ok, detail=""):
    CHECKS.append((name, bool(ok)))
    status = "PASS" if ok else "FAIL"
    msg = f"[{status}] {name}"
    if detail:
        msg += f"  ({detail})"
    print(msg)


# ----------------------------------------------------------------------
# Core closed forms
# ----------------------------------------------------------------------

def f_exact(x, beta):
    """Exact normalized total event rate nu_tot/nu_0 = e^{-x^2/2} cosh(beta x)."""
    return np.exp(-np.asarray(x, dtype=float) ** 2 / 2.0) * np.cosh(beta * np.asarray(x, dtype=float))


def He(n, t):
    """Probabilists' Hermite polynomial He_n(t) via recurrence (numeric)."""
    if n == 0:
        return np.ones_like(np.asarray(t, dtype=float))
    hm, h = np.ones_like(np.asarray(t, dtype=float)), np.asarray(t, dtype=float).copy()
    for k in range(1, n):
        hm, h = h, np.asarray(t, dtype=float) * h - k * hm
    return h


def a_coeff(m, beta):
    """Series coefficient a_{2m} = He_{2m}(beta)/(2m)! of nu_tot/nu_0 in x."""
    return float(He(2 * m, beta)) / math.factorial(2 * m)


# ----------------------------------------------------------------------
# [T1] Exact rate law from the two shifted Rice Gaussians
# ----------------------------------------------------------------------
print("=" * 72)
print("[T1] Exact symmetrized rate law")
print("=" * 72)

rng = np.random.default_rng(20260702)
ok = True
for _ in range(200):
    beta = rng.uniform(0.3, 12.0)
    x = rng.uniform(-3.0 * beta, 3.0 * beta)
    lhs = 0.5 * (np.exp(-(beta - x) ** 2 / 2) + np.exp(-(beta + x) ** 2 / 2)) / np.exp(-beta ** 2 / 2)
    rhs = f_exact(x, beta)
    if not np.isclose(lhs, rhs, rtol=1e-12, atol=1e-300):
        ok = False
        break
check("T1 exact identity: (up@+K_B + down@-K_B)/nu_0 == e^{-x^2/2}cosh(beta x)",
      ok, "200 random (beta,x)")

# Symbolic confirmation (rewrite cosh -> exp, expand the squared exponents)
xs, bs = sp.symbols("x beta", real=True)
lhs_sym = sp.Rational(1, 2) * (sp.exp(sp.expand(-(bs - xs) ** 2 / 2)) + sp.exp(sp.expand(-(bs + xs) ** 2 / 2))) / sp.exp(-bs ** 2 / 2)
rhs_sym = sp.exp(-xs ** 2 / 2) * sp.cosh(bs * xs)
diff_sym = sp.expand((lhs_sym - rhs_sym.rewrite(sp.exp)), power_exp=True)
ok_sym = sp.simplify(diff_sym) == 0
if not ok_sym:
    ok_sym = (lhs_sym - rhs_sym).equals(0) is True
check("T1 symbolic identity (sympy)", ok_sym)

# ----------------------------------------------------------------------
# [T2] Hermite-tower series theorem
# ----------------------------------------------------------------------
print()
print("=" * 72)
print("[T2] Hermite tower: nu/nu_0 = sum He_{2m}(beta) x^{2m} / (2m)!")
print("=" * 72)

# Symbolic: series of e^{-x^2/2} cosh(beta x) vs Hermite recurrence, to x^10
series = sp.series(rhs_sym, xs, 0, 12).removeO().expand()


def He_sym(n, t):
    if n == 0:
        return sp.Integer(1)
    hm, h = sp.Integer(1), t
    for k in range(1, n):
        hm, h = h, sp.expand(t * h - k * hm)
    return h


tower = sum(He_sym(2 * m, bs) * xs ** (2 * m) / sp.factorial(2 * m) for m in range(6))
check("T2 symbolic series == Hermite tower through x^10",
      sp.expand(series - tower) == 0)

# Explicit low-order coefficients
a2_sym = sp.expand(He_sym(2, bs) / sp.factorial(2))
a4_sym = sp.expand(He_sym(4, bs) / sp.factorial(4))
a6_sym = sp.expand(He_sym(6, bs) / sp.factorial(6))
check("T2 a2 == (beta^2 - 1)/2",
      sp.expand(a2_sym - (bs ** 2 - 1) / 2) == 0)
check("T2 a4 == (beta^4 - 6 beta^2 + 3)/24",
      sp.expand(a4_sym - (bs ** 4 - 6 * bs ** 2 + 3) / 24) == 0)
check("T2 a6 == (beta^6 - 15 beta^4 + 45 beta^2 - 15)/720",
      sp.expand(a6_sym - (bs ** 6 - 15 * bs ** 4 + 45 * bs ** 2 - 15) / 720) == 0)

# Numeric convergence of the truncated tower
ok = True
for beta in (1.5, 2.0, 3.0):
    for x in (0.05, 0.1, 0.2):
        exact = float(f_exact(x, beta))
        trunc = sum(a_coeff(m, beta) * x ** (2 * m) for m in range(8))
        if abs(trunc - exact) > 1e-10 * abs(exact):
            ok = False
check("T2 numeric: 8-term tower matches exact to 1e-10 (weak field)", ok)

# ----------------------------------------------------------------------
# [T3] Odd-order (linear-term) cancellation
# ----------------------------------------------------------------------
print()
print("=" * 72)
print("[T3] Evenness: the +/-K_B symmetry kills all odd orders in J_coh")
print("=" * 72)

ok = True
for _ in range(100):
    beta = rng.uniform(0.3, 10.0)
    x = rng.uniform(0.0, 3.0 * beta)
    if not np.isclose(f_exact(x, beta), f_exact(-x, beta), rtol=0, atol=1e-15):
        ok = False
check("T3 f(x) == f(-x) exactly (rate even in J_coh)", ok)
check("T3 symbolic: d f/dx at x=0 is 0",
      sp.simplify(sp.diff(rhs_sym, xs).subs(xs, 0)) == 0)
check("T3 symbolic: d^3 f/dx^3 at x=0 is 0",
      sp.simplify(sp.diff(rhs_sym, xs, 3).subs(xs, 0)) == 0)

# ----------------------------------------------------------------------
# [T4] Sign condition for the Born-like leading term
# ----------------------------------------------------------------------
print()
print("=" * 72)
print("[T4] Sign condition: Born-sign leading term iff beta > 1")
print("=" * 72)

check("T4 a2 < 0 at beta = 0.9 (anti-Born)", a_coeff(1, 0.9) < 0,
      f"a2 = {a_coeff(1, 0.9):+.4f}")
check("T4 a2 = 0 at beta = 1 exactly", a_coeff(1, 1.0) == 0.0)
check("T4 a2 > 0 at beta = 1.1", a_coeff(1, 1.1) > 0,
      f"a2 = {a_coeff(1, 1.1):+.4f}")
# beta < 1: rate is monotone DEcreasing in intensity (anti-Born globally)
xg = np.linspace(0, 5, 2001)
fg = f_exact(xg, 0.8)
check("T4 beta = 0.8: exact rate monotone decreasing in x (anti-Born)",
      bool(np.all(np.diff(fg) <= 1e-15)))

# ----------------------------------------------------------------------
# [T5] Born-mimicking point beta* = sqrt(3 + sqrt(6))
# ----------------------------------------------------------------------
print()
print("=" * 72)
print("[T5] a4 root structure")
print("=" * 72)

beta_star = math.sqrt(3 + math.sqrt(6))
check("T5 symbolic: He_4 roots at beta^2 = 3 +/- sqrt(6)",
      sp.simplify(He_sym(4, bs).subs(bs ** 2, 3 + sp.sqrt(6))) == 0
      and sp.simplify(He_sym(4, bs).subs(bs ** 2, 3 - sp.sqrt(6))) == 0)
check("T5 numeric: a4(beta*) ~ 0", abs(a_coeff(2, beta_star)) < 1e-14,
      f"beta* = {beta_star:.6f}")
a6_star = a_coeff(3, beta_star)
check("T5 a6(beta*) < 0 (next deviation is sixth-order, saturating)",
      a6_star < 0, f"a6(beta*) = {a6_star:+.6f}")
check("T5 a4 < 0 for 1 < beta < beta* (sub-linear onset)",
      a_coeff(2, 1.5) < 0 and a_coeff(2, 2.0) < 0)
check("T5 a4 > 0 for beta > beta* (super-linear onset)",
      a_coeff(2, 3.0) > 0 and a_coeff(2, 5.0) > 0)

# ----------------------------------------------------------------------
# [T6] Fractional deviation from Born D(x) and deviation-reach solver
# ----------------------------------------------------------------------
print()
print("=" * 72)
print("[T6] Fractional deviation from Born  D(x) = (f-1)/(a2 x^2) - 1")
print("=" * 72)


def D_exact(x, beta):
    a2 = a_coeff(1, beta)
    x = float(x)
    return (float(f_exact(x, beta)) - 1.0) / (a2 * x * x) - 1.0


def x_for_deviation(beta, target):
    """Smallest x > 0 with |D_exact(x)| = target (bisection)."""
    lo, hi = 1e-6, None
    x = 1e-3
    while x < 10 * beta + 10:
        if abs(D_exact(x, beta)) >= target:
            hi = x
            break
        lo = x
        x *= 1.25
    if hi is None:
        return float("nan")
    for _ in range(200):
        mid = 0.5 * (lo + hi)
        if abs(D_exact(mid, beta)) < target:
            lo = mid
        else:
            hi = mid
    return 0.5 * (lo + hi)


ok = True
for beta in (1.5, 2.0, 3.0, 5.0):
    r = a_coeff(2, beta) / a_coeff(1, beta)
    for x in (0.02, 0.05):
        lead = r * x * x
        exact = D_exact(x, beta)
        if abs(exact - lead) > 0.05 * abs(lead) + 1e-9:
            ok = False
check("T6 leading law D ~ (a4/a2) x^2 accurate to 5% for x <= 0.05", ok)

# ----------------------------------------------------------------------
# [T7] Observable projections
# ----------------------------------------------------------------------
print()
print("=" * 72)
print("[T7] Observables: fringe harmonics, visibility, source-statistics split")
print("=" * 72)

# --- 7a: balanced two-beam fringe, second-harmonic content -------------
def fringe_harmonics(beta, Ipeak, nphi=4096):
    """Fourier cos-coefficients of the exact excess rate over a balanced fringe.

    Itilde(phi) = Ipeak * (1+cos phi)/2 ;  excess = f(sqrt(Itilde)) - 1.
    """
    phi = np.linspace(0, 2 * np.pi, nphi, endpoint=False)
    it = Ipeak * (1 + np.cos(phi)) / 2
    rate = f_exact(np.sqrt(it), beta) - 1.0
    c1 = 2 * np.mean(rate * np.cos(phi))
    c2 = 2 * np.mean(rate * np.cos(2 * phi))
    return c1, c2


ok = True
detail = []
for beta in (2.0, 3.0, 5.0):
    Ipeak = 0.02  # weak-field operating point
    eps = (a_coeff(2, beta) / a_coeff(1, beta)) * Ipeak
    c1, c2 = fringe_harmonics(beta, Ipeak)
    pred = eps / (4 * (1 + eps))
    got = c2 / c1
    # tolerance: next order in Ipeak
    if abs(got - pred) > 0.15 * abs(pred) + 1e-7:
        ok = False
    detail.append(f"beta={beta}: c2/c1={got:+.3e} vs eps/4={pred:+.3e}")
check("T7a fringe second harmonic c2/c1 == eps/(4(1+eps)) (weak field, exact rate)",
      ok, "; ".join(detail))

# QM/Born reference: linear response has zero second harmonic on a balanced fringe
phi = np.linspace(0, 2 * np.pi, 4096, endpoint=False)
it = 0.02 * (1 + np.cos(phi)) / 2
c2_born = 2 * np.mean(it * np.cos(2 * phi))
check("T7a Born reference: linear-in-I response has c2 == 0",
      abs(c2_born) < 1e-12)

# --- 7b: unbalanced-fringe visibility shift ----------------------------
def vis_measured_quadratic(beta, Imean, V):
    a2, a4 = a_coeff(1, beta), a_coeff(2, beta)
    Rmax = a2 * Imean * (1 + V) + a4 * (Imean * (1 + V)) ** 2
    Rmin = a2 * Imean * (1 - V) + a4 * (Imean * (1 - V)) ** 2
    return (Rmax - Rmin) / (Rmax + Rmin)


ok = True
detail = []
for beta in (2.0, 3.0, 5.0):
    Imean, V = 0.02, 0.5
    e = (a_coeff(2, beta) / a_coeff(1, beta)) * Imean
    got = vis_measured_quadratic(beta, Imean, V)
    pred = V * (1 + 2 * e) / (1 + e * (1 + V ** 2))
    if abs(got - pred) > 1e-12:
        ok = False
    # exact-rate cross-check of the same observable
    Rmax_ex = float(f_exact(math.sqrt(Imean * (1 + V)), beta)) - 1
    Rmin_ex = float(f_exact(math.sqrt(Imean * (1 - V)), beta)) - 1
    got_ex = (Rmax_ex - Rmin_ex) / (Rmax_ex + Rmin_ex)
    if abs(got_ex - pred) > 0.10 * abs(pred - V) + 1e-7:
        ok = False
    detail.append(f"beta={beta}: dV/V={(got_ex - V) / V:+.3e} vs {e * (1 - V ** 2):+.3e}")
check("T7b visibility shift V_meas = V(1+2e)/(1+e(1+V^2)); dV/V ~ e(1-V^2)",
      ok, "; ".join(detail))

# --- 7c: source-statistics calibration split ---------------------------
_trapz = getattr(np, "trapezoid", None) or np.trapz


def thermal_excess(beta, Imean):
    """<f(sqrt(I))>-1 over exponential (thermal, g2=2) intensity distribution.

    integral_0^inf e^{-t} f(sqrt(Imean*t)) dt, dense trapezoid on [0, 60]
    (tail weight e^{-60} ~ 1e-26, negligible; integrand bounded since
    f(sqrt(I)) = e^{-I/2} cosh(beta sqrt(I)) <= e^{beta^2/2})."""
    t = np.linspace(0.0, 60.0, 240001)
    vals = np.exp(-t) * f_exact(np.sqrt(Imean * t), beta)
    return float(_trapz(vals, t)) - 1.0


ok = True
detail = []
for beta in (2.0, 3.0, 5.0):
    Imean = 0.01
    e = (a_coeff(2, beta) / a_coeff(1, beta)) * Imean
    exc_th = thermal_excess(beta, Imean)
    exc_coh = float(f_exact(math.sqrt(Imean), beta)) - 1.0
    got = exc_th / exc_coh - 1.0
    pred = e  # (g2 - 1) * e with g2 = 2
    if not (math.isfinite(got) and abs(got - pred) <= 0.20 * abs(pred) + 1e-6):
        ok = False
    detail.append(f"beta={beta}: split={got:+.3e} vs e={pred:+.3e}")
check("T7c thermal-vs-coherent split == (a4/a2) Imean (g2-1), g2=2",
      ok, "; ".join(detail))

# ----------------------------------------------------------------------
# [T8] Saturation / rollover of the exact rate envelope
# ----------------------------------------------------------------------
print()
print("=" * 72)
print("[T8] Saturation: peak at x* = beta tanh(beta x*), collapse beyond")
print("=" * 72)


def x_peak(beta):
    """Nonzero fixed point of x = beta tanh(beta x) (exists for beta > 1)."""
    lo, hi = 1e-9, 2.0 * beta
    g = lambda x: beta * math.tanh(beta * x) - x
    # g(0+) > 0 for beta > 1; g(2 beta) < 0
    for _ in range(200):
        mid = 0.5 * (lo + hi)
        if g(mid) > 0:
            lo = mid
        else:
            hi = mid
    return 0.5 * (lo + hi)


ok = True
detail = []
for beta in (1.5, 2.0, 3.0, 5.0, 10.0):
    xs_ = x_peak(beta)
    # verify it's the argmax of the exact rate
    grid = np.linspace(1e-4, 3 * beta, 30001)
    xg_ = grid[np.argmax(f_exact(grid, beta))]
    if abs(xs_ - xg_) > 2e-3 * beta:
        ok = False
    detail.append(f"beta={beta}: J*/K_B={xs_ / beta:.6f}")
check("T8 peak location solves x = beta tanh(beta x); matches grid argmax",
      ok, "; ".join(detail))

ok = True
for beta in (3.0, 5.0, 10.0):
    pk = float(f_exact(x_peak(beta), beta))
    asym = 0.5 * math.exp(beta ** 2 / 2)
    if abs(pk / asym - 1) > 2e-2:
        ok = False
check("T8 peak height -> (1/2) e^{beta^2/2} (large beta, 2%)", ok)

ok = True
for beta in (3.0, 5.0, 10.0):
    val = float(f_exact(2 * beta, beta))
    if abs(val - 0.5) > 0.01:
        ok = False
check("T8 rate/nu_0 -> 1/2 at J_coh = 2 K_B (large beta)", ok)

check("T8 anti-Born collapse: f(3 beta, beta) << f(x*, beta)",
      float(f_exact(3 * 3.0, 3.0)) < 1e-3 * float(f_exact(x_peak(3.0), 3.0)))

# Exact landmark: at J_coh = 2 K_B the rate is EXACTLY nu_0 (1+e^{-4 beta^2})/2
ok = True
for beta in (1.2, 1.5, 2.0, 3.0, 5.0):
    if not np.isclose(float(f_exact(2 * beta, beta)),
                      0.5 * (1 + math.exp(-4 * beta ** 2)), rtol=1e-12):
        ok = False
check("T8 exact landmark: f(2 beta, beta) == (1 + e^{-4 beta^2})/2", ok)
_lmk = sp.expand(rhs_sym.subs(xs, 2 * bs).rewrite(sp.exp), power_exp=True) \
    - (1 + sp.exp(-4 * bs ** 2)) / 2
check("T8 symbolic landmark identity", sp.simplify(_lmk) == 0)

# ----------------------------------------------------------------------
# [MC] Monte-Carlo validation of the Rice base formula (no fitting)
# ----------------------------------------------------------------------
print()
print("=" * 72)
print("[MC] Direct level-crossing counts on a synthesized Gaussian process")
print("=" * 72)


def mc_rice(beta, x, n=2 ** 22, dt=0.25, seed=7):
    """Count (+K_B up)+( -K_B down) crossings on a spectral-synthesis Gaussian
    process with unit variance and measured derivative variance; return
    (empirical_rate, rice_rate)."""
    g = np.random.default_rng(seed)
    freqs = np.fft.rfftfreq(n, d=dt)          # cycles / time
    omega = 2 * np.pi * freqs
    band = (omega >= 0.2) & (omega <= 1.2)
    Z = np.zeros(len(freqs), dtype=complex)
    m = int(band.sum())
    Z[band] = g.standard_normal(m) + 1j * g.standard_normal(m)
    X = np.fft.irfft(Z, n=n)
    scale = 1.0 / X.std()
    X *= scale                                # unit sigma_X
    Xdot = np.fft.irfft(Z * 1j * omega, n=n) * scale
    sig_dot = Xdot.std()
    Y = X + x                                  # mean shift = coherent amplitude
    u = beta
    up = np.count_nonzero((Y[:-1] < u) & (Y[1:] >= u))
    dn = np.count_nonzero((Y[:-1] > -u) & (Y[1:] <= -u))
    T = n * dt
    emp = (up + dn) / T
    rice = (sig_dot / (2 * np.pi)) * (math.exp(-(u - x) ** 2 / 2) + math.exp(-(u + x) ** 2 / 2))
    return emp, rice


ok = True
detail = []
for (beta, x) in ((1.5, 0.0), (1.5, 0.8), (2.0, 0.5)):
    emp, rice = mc_rice(beta, x)
    rel = emp / rice - 1
    detail.append(f"(beta={beta}, x={x}): MC/Rice-1 = {rel:+.3%}")
    if abs(rel) > 0.03:
        ok = False
check("MC crossing counts match Rice closed form to 3%", ok, "; ".join(detail))

# MC check of evenness: x -> -x gives same rate within statistics
emp_p, _ = mc_rice(1.5, 0.8, seed=11)
emp_m, _ = mc_rice(1.5, -0.8, seed=11)
check("MC evenness: rate(x) == rate(-x) within 2%",
      abs(emp_p / emp_m - 1) < 0.02, f"ratio-1 = {emp_p / emp_m - 1:+.3%}")

# ----------------------------------------------------------------------
# Worked numeric table (transcribed into the ANALYSIS doc)
# ----------------------------------------------------------------------
print()
print("=" * 72)
print("WORKED TABLE (verified values; sigma_n = K_B/beta, K_B = 0.511)")
print("=" * 72)
hdr = (f"{'beta':>7} {'a2':>9} {'a4':>10} {'a4/a2':>9} "
       f"{'x@1%':>8} {'(J/K_B)@1%':>11} {'x@10%':>8} {'(J/K_B)@10%':>12} "
       f"{'J*/K_B':>8} {'peak f':>11} {'f@2K_B':>8}")
print(hdr)
rows = []
for beta in (1.2, 1.5, 2.0, beta_star, 3.0, 5.0, 10.0):
    a2 = a_coeff(1, beta)
    a4 = a_coeff(2, beta)
    r = a4 / a2
    x1 = x_for_deviation(beta, 0.01)
    x10 = x_for_deviation(beta, 0.10)
    xp = x_peak(beta)
    pk = float(f_exact(xp, beta))
    f2 = float(f_exact(2 * beta, beta))
    row = (beta, a2, a4, r, x1, x1 / beta, x10, x10 / beta, xp / beta, pk, f2)
    rows.append(row)
    print(f"{beta:7.4f} {a2:9.4f} {a4:10.4f} {r:9.4f} "
          f"{x1:8.4f} {x1 / beta:11.4f} {x10:8.4f} {x10 / beta:12.4f} "
          f"{xp / beta:8.5f} {pk:11.4g} {f2:8.4f}")

# structural sanity on the table itself
check("TABLE x@1% < x@10% for every beta",
      all(row[4] < row[6] for row in rows if not math.isnan(row[4])))
# Analytically J*/K_B = tanh(beta x*) < 1 strictly, approaching 1 like
# 1 - 2 e^{-2 beta^2}; for beta >= 3 the gap (< 3e-8) is below float64
# resolution of the fixed point, so assert strictness only where resolvable.
check("TABLE J*/K_B <= 1 for every beta; strictly < 1 where fp-resolvable",
      all(row[8] <= 1.0 + 1e-12 for row in rows)
      and all(row[8] < 1.0 for row in rows if row[0] <= 2.5))
check("TABLE beta* row: 1% reach pushed out (sixth-order onset)",
      rows[3][4] > rows[2][4] and rows[3][4] > rows[4][4],
      f"x@1%: beta=2: {rows[2][4]:.3f}, beta*: {rows[3][4]:.3f}, beta=3: {rows[4][4]:.3f}")

# ----------------------------------------------------------------------
# Summary
# ----------------------------------------------------------------------
print()
print("=" * 72)
n_pass = sum(1 for _, ok_ in CHECKS if ok_)
n_tot = len(CHECKS)
print(f"RESULT: {n_pass}/{n_tot} checks passed")
print("=" * 72)
if n_pass != n_tot:
    for name, ok_ in CHECKS:
        if not ok_:
            print(f"  FAILED: {name}")
    raise SystemExit(1)
