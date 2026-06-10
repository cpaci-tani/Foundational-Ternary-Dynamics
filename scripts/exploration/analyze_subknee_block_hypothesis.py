"""FTD-0263: sub-knee onset mechanism — the 27-block geometric hypothesis.

Pre-registration: docs/theory/03_derivations/foundational_mechanics/
  PREREG_SUBKNEE_BLOCK_HYPOTHESIS_v1.md
Runner: engine/tests/campaign_thermostat_off_sweep.cpp (adds --dir=axial|diag).

HYPOTHESIS (stated before the run): the FTD-0261 knee at A~16 is the
27-block boundary — below it the cluster fills the injection voxel's own
Moore block (orbit shells cumulate 1 -> 7 -> 19 -> 27); above it the cluster
grows into the bulk under the wave-envelope threshold. Motivating (post-hoc)
observations: knee-N ~ 22 (axial, thermostat ON) and ~22-25 (thermostat OFF)
— same N-band under different dynamics, friction already excluded as author.

If the knee is block GEOMETRY, its N-location must be (i) in the block band,
(ii) injection-direction invariant, (iii) lattice-size invariant in the
sub-knee regime. All three criteria frozen here; the block hypothesis is
maximally attractive to this framework, so the verdict hangs ONLY on these
kill-tests (aesthetic-capture guard). The fine-grid staircase table is
DESCRIPTIVE ONLY (never verdict-bearing — anti-apophenia).

Usage: python analyze_subknee_block_hypothesis.py --results-dir=PATH
"""
import argparse
import csv
import glob
import math
import os
import sys
from collections import defaultdict

# frozen reference: FTD-0261 arm-N bulk points (run of record, canonical axial L=32)
REF_BULK = [(20.0, 27.4), (25.0, 32.6), (30.0, 45.0), (40.0, 91.8),
            (50.0, 130.2), (70.0, 260.2), (90.0, 383.34)]
KNEE_N_BAND = (19.0, 33.0)        # block band: through-edges (19) .. full block+ (33)
DIR_BAND = (0.6, 1.67)            # N_diag/N_axial at A in {14, 16}
DIR_AS = [14.0, 16.0]
LINV_AS = [10.0, 12.0, 14.0]      # clearly sub-knee
LINV_BAND = (0.65, 1.54)          # +-35% band vs L=32
LINV_MIN = 5                      # of 6 comparisons (3 A x 2 L)
TRIVIAL_N = 1.5


def load(results_dir):
    rows = []
    for path in glob.glob(os.path.join(results_dir, "sweep_*.csv")):
        with open(path, newline="") as fh:
            for r in csv.DictReader(fh):
                r["A"] = float(r["A"]); r["n_mean"] = float(r["n_mean"])
                rows.append(r)
    return rows


def tagged(rows, tag):
    d = defaultdict(list)
    for r in rows:
        if r["tag"] == tag:
            d[round(r["A"], 2)].append(r["n_mean"])
    return {a: sum(v) / len(v) for a, v in d.items()}


def fit_power(pts):
    xs = [math.log10(a) for a, n in pts]
    ys = [math.log10(n) for a, n in pts]
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


def broken_fit(pts):
    """Scan interior knee positions; return (rms, knee_A, knee_N, p_lo, p_hi)."""
    best = None
    for ik in range(2, len(pts) - 3):
        lo, hi = pts[:ik + 1], pts[ik + 1:]
        flo, fhi = fit_power(lo), fit_power(hi)
        if not flo or not fhi:
            continue
        rms = math.sqrt((flo[2] ** 2 * len(lo) + fhi[2] ** 2 * len(hi))
                        / (len(lo) + len(hi)))
        kA = pts[ik][0]
        kN = flo[0] * kA ** flo[1]
        if best is None or rms < best[0]:
            best = (rms, kA, kN, flo[1], fhi[1])
    return best


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--results-dir", required=True)
    args = ap.parse_args()
    rows = load(args.results_dir)
    if not rows:
        print("NO DATA"); sys.exit(2)

    F = tagged(rows, "F")        # fine axial grid, L=32
    D = tagged(rows, "D")        # body-diagonal, L=32
    L24 = tagged(rows, "L24")
    L48 = tagged(rows, "L48")

    print("=== arm F (fine axial grid, L=32) — DESCRIPTIVE staircase table ===")
    print("   A    | N_mean   (orbit milestones for reference: 1, 7, 19, 27)")
    for a in sorted(F):
        print(f"{a:6.1f} | {F[a]:.2f}")

    # ---- C1: knee-N localization (axial) ----
    pts = sorted([(a, n) for a, n in F.items() if n >= TRIVIAL_N]) + REF_BULK
    pts = sorted(pts)
    bf = broken_fit(pts)
    if bf is None:
        print("\nC1 knee-N: UNDETERMINED"); c1 = None
    else:
        rms, kA, kN, plo, phi = bf
        c1 = KNEE_N_BAND[0] <= kN <= KNEE_N_BAND[1]
        print(f"\nC1 knee localization (F + frozen FTD-0261 bulk): knee_A={kA:.1f} "
              f"knee_N={kN:.1f} p_lo={plo:.2f} p_hi={phi:.2f} (rms {rms:.3f})")
        print(f"C1 verdict: knee_N in block band [19, 33]? "
              f"{'YES — GEOMETRIC band' if c1 else 'NO'}")

    # ---- C2: direction invariance ----
    print("\n=== arm D (body-diagonal, L=32) ===")
    for a in sorted(D):
        print(f"{a:6.1f} | {D[a]:.2f}")
    c2_checks = []
    for a in DIR_AS:
        if a in D and a in F and F[a] > 0:
            r = D[a] / F[a]
            ok = DIR_BAND[0] <= r <= DIR_BAND[1]
            c2_checks.append(ok)
            print(f"C2 A={a}: N_diag/N_axial = {r:.3f} band {DIR_BAND} -> "
                  f"{'OK' if ok else 'FAIL'}")
    c2 = all(c2_checks) if len(c2_checks) == len(DIR_AS) else None
    print(f"C2 verdict: {'DIR-CONSISTENT' if c2 else ('UNDETERMINED' if c2 is None else 'DIR-DEPENDENT')}")
    dbf = broken_fit(sorted(D.items()))
    if dbf:
        print(f"[descriptive] diag broken fit: knee_A={dbf[1]:.1f} knee_N={dbf[2]:.1f}")

    # ---- C3: sub-knee L-invariance ----
    print("\n=== arms L24/L48 (axial) vs L=32 (arm F), sub-knee ===")
    hits, total = 0, 0
    for a in LINV_AS:
        for name, tab in (("L24", L24), ("L48", L48)):
            if a in tab and a in F and F[a] > 0:
                r = tab[a] / F[a]
                ok = LINV_BAND[0] <= r <= LINV_BAND[1]
                hits += ok; total += 1
                print(f"C3 A={a} {name}: N/N_L32 = {r:.3f} -> {'OK' if ok else 'FAIL'}")
    c3 = (hits >= LINV_MIN) if total == 6 else None
    print(f"C3 verdict: {hits}/{total} in band -> "
          f"{'L-INVARIANT-SUBKNEE' if c3 else ('UNDETERMINED' if c3 is None else 'L-DEPENDENT')}")
    for a in (20.0, 30.0):
        vals = [(nm, t[a]) for nm, t in (("L24", L24), ("L48", L48)) if a in t]
        if vals:
            print(f"[descriptive] bulk A={a}: " +
                  " ".join(f"{nm}={v:.1f}" for nm, v in vals) +
                  (f" (L32 ref {dict(REF_BULK).get(a, float('nan')):.1f})"))

    # ---- outcome ----
    print("\n================ OUTCOME ================")
    held = sum(1 for c in (c1, c2, c3) if c is True)
    und = any(c is None for c in (c1, c2, c3))
    if held == 3:
        print("GEOM-CONFIRMED: the knee is block geometry — knee-N in the 27-block "
              "band, direction-invariant, sub-knee L-invariant. The sub-knee onset "
              "is the Moore-block filling regime [MEASURED].")
    elif held == 2 and not und:
        print("GEOM-PARTIAL: two of three invariances hold; see component verdicts.")
    elif und:
        print("UNDETERMINED components present; no closure claimed.")
    else:
        print("GEOM-DISFAVORED: the block hypothesis fails its invariance tests.")


if __name__ == "__main__":
    main()
