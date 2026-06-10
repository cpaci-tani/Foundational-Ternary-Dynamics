"""FTD-0261: current-stack N(A) law characterization + thermostat discriminator v2.

Pre-registration: docs/theory/03_derivations/foundational_mechanics/
  PREREG_NA_LAW_CURRENT_STACK_v1.md
Runner: engine/tests/campaign_thermostat_off_sweep.cpp (--coupling=on protocol)

Hash-locked WITH the pre-registration BEFORE the campaign runs. All rules
below are mechanical; no thresholds may be adjusted post-run.

Usage: python analyze_na_law_current_stack.py --results-dir=PATH
"""
import argparse
import csv
import glob
import math
import os
import sys
from collections import defaultdict

# ---- frozen design constants (PREREG sections 2-4) ----
A_GRID = [2, 4, 6, 8, 10, 12, 14, 16, 20, 25, 30, 40, 50, 70, 90]
FLOOD_N = 1000.0          # F-1: n_mean above this => FLOODED, excluded from fits
TRIVIAL_N = 1.5           # F-2: n_mean below this => sub-threshold/trivial, excluded
# V-1 rig anchors: canonical-test T5b values on the current stack (GPU,
# coupling=on, t=700 single-shot; measured 2026-06-10). Loose band -- catches
# rig breakage, tolerates protocol difference (windowed mean vs single-shot).
T5B_REF = {10.0: 0.040, 15.0: 0.088, 20.0: 0.068, 30.0: 0.050, 50.0: 0.052}
V1_NEAREST = {10.0: 10, 15.0: 14, 20.0: 20, 30.0: 30, 50.0: 50}  # grid mapping
V1_RELTOL = 0.6           # |k - ref| <= max(0.02, 0.6*ref) at >= 4/5 anchors
V1_MIN = 4
# Law-fit candidates (fit on log N vs log A over valid points):
#   L1: N = k * A^2          (1 param)
#   L2: N = c * A^p          (2 params)
#   L3: knee: N = c1*A^p1 (A<=A_KNEE), c2*A^p2 (A>A_KNEE), A_KNEE in grid scan
RMS_CLEAN = 0.10          # log10-RMS for CLEAN-LAW
RMS_NOLAW = 0.25          # above this for all candidates => NO-LAW
AIC_MARGIN = 2.0          # winner must beat runner-up by >= 2 AIC units
# Discriminator (gamma): ratio R(A) = k_X / k_N on common valid grid
GAMMA_BAND_LO, GAMMA_BAND_HI = 0.8, 1.25   # Outcome B if ALL R inside
GAMMA_STRONG = 1.5                          # Outcome A if median R >= 1.5 or <= 1/1.5


def load(results_dir):
    rows = []
    for path in glob.glob(os.path.join(results_dir, "sweep_*.csv")):
        with open(path, newline="") as fh:
            for r in csv.DictReader(fh):
                r["A"] = float(r["A"]); r["n_mean"] = float(r["n_mean"])
                r["k_mean"] = float(r["k_mean"])
                rows.append(r)
    return rows


def arm(rows, tag):
    by_A = defaultdict(list)
    for r in rows:
        if r["tag"] == tag:
            by_A[r["A"]].append(r)
    return by_A


def nbar(by_A, A):
    rs = by_A.get(float(A), [])
    return sum(r["n_mean"] for r in rs) / len(rs) if rs else None


def valid_points(by_A):
    pts = []
    for A in A_GRID:
        n = nbar(by_A, A)
        if n is None:
            continue
        if n > FLOOD_N:
            pts.append((A, n, "FLOODED"))
        elif n < TRIVIAL_N:
            pts.append((A, n, "TRIVIAL"))
        else:
            pts.append((A, n, "OK"))
    return pts


def fit_power(pts):
    """log10 N = log10 c + p*log10 A least squares; returns (c, p, rms)."""
    xs = [math.log10(a) for a, n, s in pts]
    ys = [math.log10(n) for a, n, s in pts]
    n = len(xs)
    if n < 3:
        return None
    mx, my = sum(xs) / n, sum(ys) / n
    sxx = sum((x - mx) ** 2 for x in xs)
    sxy = sum((x - mx) * (y - my) for x, y in zip(xs, ys))
    p = sxy / sxx if sxx else 0.0
    c = 10 ** (my - p * mx)
    rms = math.sqrt(sum((math.log10(c) + p * x - y) ** 2
                        for x, y in zip(xs, ys)) / n)
    return c, p, rms


def fit_fixed2(pts):
    """N = k*A^2: log10 k = mean(log10 N - 2 log10 A); returns (k, rms)."""
    resid = [math.log10(n) - 2 * math.log10(a) for a, n, s in pts]
    n = len(resid)
    lk = sum(resid) / n
    rms = math.sqrt(sum((r - lk) ** 2 for r in resid) / n)
    return 10 ** lk, rms


def aic(rms, npts, nparam):
    # Gaussian log-likelihood AIC on log-residuals (constant terms cancel
    # between candidates with the same data).
    if rms <= 0:
        rms = 1e-9
    return 2 * nparam + 2 * npts * math.log(rms)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--results-dir", required=True)
    args = ap.parse_args()
    rows = load(args.results_dir)
    if not rows:
        print("NO DATA"); sys.exit(2)

    N = arm(rows, "N")    # characterization arm (thermostat on, coupling on)
    X = arm(rows, "X")    # thermostat-off arm (coupling on)

    print("=== arm N (thermostat ON) — per-amplitude ===")
    print("   A   |  N_mean  |  k=N/A^2 | status")
    npts = valid_points(N)
    for a, n, s in npts:
        print(f"{a:5.0f} | {n:8.2f} | {n/(a*a):8.4f} | {s}")
    ok_n = [(a, n, s) for a, n, s in npts if s == "OK"]

    # ---- V-1 rig gate ----
    hits = 0
    for Aref, kref in T5B_REF.items():
        Ag = V1_NEAREST[Aref]
        n = nbar(N, Ag)
        if n is None:
            continue
        k = n / (Ag * Ag)
        if abs(k - kref) <= max(0.02, V1_RELTOL * kref):
            hits += 1
    v1 = hits >= V1_MIN
    print(f"\nV-1 rig gate vs current-stack T5b anchors: {hits}/5 "
          f"-> {'PASS' if v1 else 'FAIL (RUN INVALID)'}")
    if not v1:
        print("VERDICT: INVALID RUN (V-1). No characterization or mechanism "
              "outcome may be claimed.")
        sys.exit(3)

    # ---- law fits on arm N ----
    print("\n=== law fits (arm N, OK points only) ===")
    if len(ok_n) < 4:
        print("VERDICT: NO-LAW (insufficient valid points)"); sys.exit(0)
    k1, rms1 = fit_fixed2(ok_n)
    a1 = aic(rms1, len(ok_n), 1)
    c2, p2, rms2 = fit_power(ok_n)
    a2 = aic(rms2, len(ok_n), 2)
    print(f"L1  N = k*A^2        : k={k1:.4f}            log10-RMS={rms1:.4f}  AIC={a1:.1f}")
    print(f"L2  N = c*A^p        : c={c2:.4f}, p={p2:.3f}  log10-RMS={rms2:.4f}  AIC={a2:.1f}")
    best_knee = None
    for ik in range(2, len(ok_n) - 2):
        lo, hi = ok_n[:ik + 1], ok_n[ik + 1:]
        flo, fhi = fit_power(lo), fit_power(hi)
        if not flo or not fhi:
            continue
        nlo, nhi = len(lo), len(hi)
        rms = math.sqrt((flo[2] ** 2 * nlo + fhi[2] ** 2 * nhi) / (nlo + nhi))
        if best_knee is None or rms < best_knee[0]:
            best_knee = (rms, ok_n[ik][0], flo, fhi)
    if best_knee:
        rms3, Ak, flo, fhi = best_knee
        a3 = aic(rms3, len(ok_n), 5)
        print(f"L3  knee @A={Ak:<4.0f}      : p_lo={flo[1]:.3f}, p_hi={fhi[1]:.3f}  "
              f"log10-RMS={rms3:.4f}  AIC={a3:.1f}")
    cands = [("L1", a1, rms1), ("L2", a2, rms2)]
    if best_knee:
        cands.append(("L3", a3, rms3))
    cands.sort(key=lambda t: t[1])
    win, run = cands[0], cands[1]
    if all(c[2] > RMS_NOLAW for c in cands):
        law_verdict = "NO-LAW (all candidates log10-RMS > 0.25)"
    elif win[2] <= RMS_CLEAN and (run[1] - win[1]) >= AIC_MARGIN:
        law_verdict = f"CLEAN-LAW: {win[0]} wins (AIC margin {run[1]-win[1]:.1f})"
    else:
        law_verdict = (f"AMBIGUOUS: best={win[0]} (log10-RMS={win[2]:.3f}, "
                       f"AIC margin {run[1]-win[1]:.1f} < {AIC_MARGIN})")
    print(f"\nCHARACTERIZATION VERDICT: {law_verdict}")

    # ---- thermostat discriminator on the common valid grid ----
    print("\n=== arm X (thermostat OFF) — per-amplitude ===")
    xpts = valid_points(X)
    for a, n, s in xpts:
        print(f"{a:5.0f} | {n:8.2f} | {n/(a*a):8.4f} | {s}")
    common = []
    for a, n, s in ok_n:
        nx = nbar(X, a)
        if nx is not None and TRIVIAL_N <= nx <= FLOOD_N:
            common.append((a, nx / n))
    if len(common) < 3:
        print("\nGAMMA VERDICT: UNDETERMINED (fewer than 3 common valid points "
              "— flooding/triviality ate the overlap)")
    else:
        ratios = sorted(r for _, r in common)
        med = ratios[len(ratios) // 2]
        all_in = all(GAMMA_BAND_LO <= r <= GAMMA_BAND_HI for _, r in common)
        print("\ncommon-grid ratios R(A) = N_X/N_N:")
        for a, r in common:
            print(f"   A={a:<4.0f} R={r:.3f}")
        print(f"median R = {med:.3f} over {len(common)} points")
        if med >= GAMMA_STRONG or med <= 1.0 / GAMMA_STRONG:
            print("GAMMA VERDICT: OUTCOME A — thermostat materially shapes the "
                  "current-stack N(A) (Mechanism gamma ACTIVE at engine level).")
        elif all_in:
            print("GAMMA VERDICT: OUTCOME B — thermostat-independent within the "
                  "band (Mechanism gamma INACTIVE for the current-stack law).")
        else:
            print("GAMMA VERDICT: OUTCOME C — partial/structured thermostat "
                  "effect; see per-A ratios. No closure claimed.")

    # ---- descriptive dose arms ----
    for tag in sorted({r["tag"] for r in rows}):
        if tag in ("N", "X"):
            continue
        tb = arm(rows, tag)
        for A, rs in sorted(tb.items()):
            for r in rs:
                print(f"[descriptive] {tag}: A={A:.0f} gamma={r['gamma']} "
                      f"T={r['T']} n_mean={r['n_mean']:.2f} k={r['k_mean']:.4f}")


if __name__ == "__main__":
    main()
