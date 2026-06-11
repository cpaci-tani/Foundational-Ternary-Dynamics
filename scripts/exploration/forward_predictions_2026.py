#!/usr/bin/env python3
"""
forward_predictions_2026.py — computed numbers for SPEC_PREDICTIONS_FORWARD_2026.md.

Every number quoted in the forward-predictions registry is COMPUTED here (GTCA
tool discipline: no recalled high-precision values). Sections:

  A. FP-2 Higgs sector — the FTD chain v = M_P*sqrt(2pi)*alpha^8, lambda = 3/23
     (chain tags: [SELECTION] + [PARAMETRIC FTD-0018 via sin^2(theta_W) = 3/13]),
     loop factor (1-alpha) applied-as-principle; sigma vs BOTH the canonical
     PDG 2024 combined (125.20 +/- 0.11, scripts/constants.py Experimental) and
     the older average the source doc used (125.25 +/- 0.17). Plus kappa_lambda.
  B. EP-2 N(A) blind interpolation — broken power law (knee A=16 frozen,
     FTD-0261 ANALYSIS_NA_LAW_CURRENT_STACK_v1.md table) -> predicted N(35), N(60)
     with +/-2*RMS(dex) bands. Registered to run later on the FTD-0261 rig.
  C. EP-3 anisotropy extrapolation — power-law fit of the closed-form phase-speed
     spread over L in [32,256] (PL-5 / AUDIT_LORENTZ_ANISOTROPY), extrapolated to
     L in {512, 768}, then checked against the exact 18-point Moore symbol.

No near-miss searching, no new identities: every formula already exists in the
corpus at its stated tag; this script only evaluates and extrapolates them.
"""
import math
import sys

try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:
    pass

print("=" * 78)
print("A. FP-2 — Higgs sector numbers (chain: [SELECTION] + [PARAMETRIC] input)")
print("=" * 78)

# FTD chain, computed from scratch
G_STAR = math.gamma(0.25) / math.gamma(0.75)
x_plus = 8.0 * G_STAR**2 + math.sqrt(64.0 * G_STAR**4 - 16.0 * G_STAR**3)
alpha = 1.0 / x_plus
M_P = 1.220890e19  # GeV, scripts/constants.py M_PLANCK (CODATA)

v_ftd = M_P * math.sqrt(2.0 * math.pi) * alpha**8
lam_ftd = 3.0 / 23.0                      # from sin^2(theta_W)=3/13 [PARAMETRIC]
m_tree = v_ftd * math.sqrt(2.0 * lam_ftd)
m_loop = m_tree * math.sqrt(1.0 - alpha)  # loop factor applied-as-principle

print(f"G*               = {G_STAR:.7f}")
print(f"1/alpha (x+)     = {x_plus:.6f}")
print(f"v_FTD            = {v_ftd:.2f} GeV   (PDG v = 246.21965 GeV from G_F)")
print(f"lambda_FTD       = 3/23 = {lam_ftd:.6f}")
print(f"m_H tree         = {m_tree:.2f} GeV")
print(f"m_H loop (1-a)   = {m_loop:.2f} GeV")

for label, mh, err in [("PDG 2024 combined (canonical, constants.py)", 125.20, 0.11),
                       ("older average (used by source doc)", 125.25, 0.17)]:
    s_tree = (m_tree - mh) / err
    s_loop = (m_loop - mh) / err
    print(f"  vs {label}: m_H = {mh} +/- {err}")
    print(f"     tree: {m_tree - mh:+.2f} GeV = {s_tree:+.2f} sigma | "
          f"loop: {m_loop - mh:+.2f} GeV = {s_loop:+.2f} sigma")

v_exp = 246.21965
lam_sm = 125.20**2 / (2.0 * v_exp**2)
kappa = lam_ftd / lam_sm
print(f"lambda_SM (m_H=125.20, v={v_exp}) = {lam_sm:.6f}")
print(f"kappa_lambda = lambda_FTD/lambda_SM = {kappa:.4f}  "
      f"(SM=1; HL-LHC ~ +/-50%, FCC ~ +/-5%)")
lam_hhh = 3.0 * m_loop**2 / v_exp
print(f"trilinear 3*m_H^2/v (with m_loop)   = {lam_hhh:.1f} GeV")

print()
print("=" * 78)
print("B. EP-2 — N(A) blind interpolation at A in {35, 60} (FTD-0261 law)")
print("=" * 78)

# Frozen run-of-record table (ANALYSIS_NA_LAW_CURRENT_STACK_v1.md §1, arm N)
A_meas = [10, 12, 14, 16, 20, 25, 30, 40, 50, 70, 90]
N_meas = [4.0, 8.4, 16.4, 21.6, 27.4, 32.6, 45.0, 91.8, 130.2, 260.2, 383.3]
A_KNEE = 16.0

# Continuous broken power law in log10 space, knee frozen at A=16:
# y = c + p_lo*min(x-xk,0) + p_hi*max(x-xk,0); linear least squares in (c,p_lo,p_hi)
xk = math.log10(A_KNEE)
rows = []
for A, N in zip(A_meas, N_meas):
    x = math.log10(A)
    rows.append((1.0, min(x - xk, 0.0), max(x - xk, 0.0), math.log10(N)))
# normal equations (3x3)
import itertools
M = [[sum(r[i] * r[j] for r in rows) for j in range(3)] for i in range(3)]
b = [sum(r[i] * r[3] for r in rows) for i in range(3)]
# solve 3x3 via Cramer
def det3(m):
    return (m[0][0]*(m[1][1]*m[2][2]-m[1][2]*m[2][1])
          - m[0][1]*(m[1][0]*m[2][2]-m[1][2]*m[2][0])
          + m[0][2]*(m[1][0]*m[2][1]-m[1][1]*m[2][0]))
D = det3(M)
sol = []
for i in range(3):
    Mi = [row[:] for row in M]
    for r in range(3):
        Mi[r][i] = b[r]
    sol.append(det3(Mi) / D)
c, p_lo, p_hi = sol
resid = [r[3] - (c + p_lo * r[1] + p_hi * r[2]) for r in rows]
rms = math.sqrt(sum(e * e for e in resid) / (len(rows) - 3))
print(f"registry fit (continuous, knee frozen @16, table-level): "
      f"p_lo = {p_lo:.3f}, p_hi = {p_hi:.3f}, N(16) = {10**c:.1f}, "
      f"RMS = {rms:.3f} dex")
print("  NOTE: the frozen FTD-0261 fitter (per-seed data, its own form) reports "
      "p_lo = 3.690, p_hi = 1.861, RMS = 0.037; the EP-2 prediction uses THIS "
      "reproducible table-level fit and its own (wider) band — no claim of "
      "reproducing the frozen fitter.")
for A_new in [35.0, 60.0]:
    x = math.log10(A_new)
    y = c + p_lo * min(x - xk, 0.0) + p_hi * max(x - xk, 0.0)
    lo, hi = 10 ** (y - 2 * rms), 10 ** (y + 2 * rms)
    print(f"  PREDICTED N({A_new:.0f}) = {10**y:.1f}   band [{lo:.1f}, {hi:.1f}] "
          f"(+/-2 RMS dex)")

print()
print("=" * 78)
print("C. EP-3 — anisotropy k^4 extrapolation to L in {512, 768} + exact check")
print("=" * 78)

# double precision underflows the speed spread beyond L~384 (delta ~ 1e-12 via
# cancellation of nearly-equal speeds) -> exact symbol evaluated in mpmath.
from mpmath import mp, mpf, cos as mcos, sqrt as msqrt, pi as mpi, log as mlog
mp.dps = 50

def neg_lap(kx, ky, kz):
    cx, cy, cz = mcos(kx), mcos(ky), mcos(kz)
    return 4 - mpf(2)/3*(cx+cy+cz) - mpf(2)/3*(cx*cy + cx*cz + cy*cz)
def speed_at(k, dx, dy, dz):
    n = msqrt(mpf(dx*dx + dy*dy + dz*dz))
    om2 = neg_lap(k*dx/n, k*dy/n, k*dz/n) / 3
    return msqrt(om2) / k
def aniso(k):
    cs = [speed_at(k,1,0,0), speed_at(k,1,1,0), speed_at(k,1,1,1)]
    return float((max(cs) - min(cs)) / (sum(cs)/3))

xs, ys = [], []
for L in [32, 48, 64, 96, 128, 192, 256]:
    k = 2*mpi/L
    xs.append(float(mlog(k))); ys.append(math.log(aniso(k)))
n = len(xs)
mx, my = sum(xs)/n, sum(ys)/n
sxx = sum((x-mx)**2 for x in xs)
slope = sum((xs[i]-mx)*(ys[i]-my) for i in range(n)) / sxx
A_pref = math.exp(my - slope*mx)
print(f"fit over L in [32,256]: p = {slope:.4f}, prefactor = {A_pref:.4e} "
      f"(PL-5 reference: p = 4.0008 +/- 0.0006)")
for L in [512, 768]:
    k = 2*mpi/L
    pred = A_pref * float(k) ** slope
    exact = aniso(k)
    ratio = pred / exact
    print(f"  L = {L}: predicted delta = {pred:.4e} | exact symbol = {exact:.4e} "
          f"| pred/exact = {ratio:.4f}")
print("EP-3 verdict rule (locked): extrapolation holds iff pred/exact in "
      "[0.95, 1.05] at both L.")
