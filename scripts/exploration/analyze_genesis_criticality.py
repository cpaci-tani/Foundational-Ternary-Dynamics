#!/usr/bin/env python3
"""
analyze_genesis_criticality.py -- ORDER of the FTD genesis/manifestation transition.

Reads the per-tick manifested-fraction series produced by
campaign_genesis_criticality (crit_<tag>_L<L>.csv, columns L,T,seed,tick,manifested,m)
and runs a finite-size-scaling (FSS) analysis to decide whether the genesis
transition is 2nd-order/critical (a relevant operator with a scaling spectrum)
or 1st-order/trivial (no RG-derived spectrum).

Three theory-fixed discriminators (frozen in PREREG_GENESIS_CRITICALITY_v1):

  D1  P(m) MODALITY at the susceptibility peak T_c(L), for the largest L:
        BIMODAL (two separated peaks, dip < 0.6*min(peak)) -> FIRST-ORDER
        UNIMODAL                                            -> CRITICAL

  D2  BINDER cumulant U4 = 1 - <m^4>/(3<m^2>^2) near T_c:
        deep MINIMUM that DEEPENS with L (U4_min(largest L) < 0.4 and
        decreasing in L)                                    -> FIRST-ORDER
        U4(L,T) curves CROSS at a common T_c with U4* in (0.4, 0.67)
        (spread of crossing values < 0.1)                   -> CRITICAL

  D3  SUSCEPTIBILITY peak scaling chi_max(L) ~ L^a (chi = N*Var(m)):
        a >= 2.6 (volume-like, ~D=3)                        -> FIRST-ORDER
        a <= 2.2 (anomalous, gamma/nu < D)                  -> CRITICAL

Verdict (majority of the 3 discriminators; ties / mixed -> INCONCLUSIVE):
  GENESIS-FIRST-ORDER : >=2 of {D1,D2,D3} say FIRST-ORDER
  GENESIS-CRITICAL    : >=2 of {D1,D2,D3} say CRITICAL
  INCONCLUSIVE        : otherwise

Usage:
  python analyze_genesis_criticality.py crit_run_L16.csv crit_run_L24.csv crit_run_L32.csv
"""

import csv
import math
import sys
from collections import defaultdict


def load(paths):
    # data[L][T] = list of m samples (pooled over seeds x ticks)
    data = defaultdict(lambda: defaultdict(list))
    for p in paths:
        with open(p, newline="") as fh:
            for row in csv.DictReader(fh):
                L = int(row["L"]); T = round(float(row["T"]), 6); m = float(row["m"])
                data[L][T].append(m)
    return data


def moments(ms):
    n = len(ms)
    if n == 0:
        return dict(n=0)
    m1 = sum(ms) / n
    m2 = sum(x * x for x in ms) / n
    m4 = sum(x ** 4 for x in ms) / n
    var = m2 - m1 * m1
    U4 = 1.0 - m4 / (3.0 * m2 * m2) if m2 > 1e-18 else float("nan")
    return dict(n=n, mean=m1, var=var, m2=m2, m4=m4, U4=U4)


def histogram(ms, nbins=40):
    if not ms:
        return [], []
    lo, hi = min(ms), max(ms)
    if hi - lo < 1e-9:
        hi = lo + 1e-9
    counts = [0] * nbins
    for x in ms:
        b = min(nbins - 1, int((x - lo) / (hi - lo) * nbins))
        counts[b] += 1
    centers = [lo + (i + 0.5) * (hi - lo) / nbins for i in range(nbins)]
    return centers, counts


def is_bimodal(ms):
    """Two separated peaks with a dip < 0.6 of the smaller peak. Smooths the
    histogram lightly before peak finding to suppress single-bin noise."""
    centers, counts = histogram(ms, nbins=40)
    if not counts or sum(counts) < 50:
        return False, "too few samples"
    # 3-bin moving average
    sm = [(counts[max(0, i - 1)] + counts[i] + counts[min(len(counts) - 1, i + 1)]) / 3.0
          for i in range(len(counts))]
    # find local maxima
    peaks = [i for i in range(1, len(sm) - 1) if sm[i] > sm[i - 1] and sm[i] >= sm[i + 1] and sm[i] > 0]
    peaks = [i for i in peaks if sm[i] > 0.05 * max(sm)]  # ignore tiny bumps
    if len(peaks) < 2:
        return False, f"{len(peaks)} peak(s)"
    # take the two tallest peaks; require a real valley between them
    peaks.sort(key=lambda i: sm[i], reverse=True)
    p1, p2 = sorted(peaks[:2])
    valley = min(sm[p1:p2 + 1])
    smaller_peak = min(sm[p1], sm[p2])
    sep = abs(centers[p2] - centers[p1])
    bimodal = (valley < 0.6 * smaller_peak) and (sep > 0.08)
    return bimodal, f"peaks@{centers[p1]:.3f},{centers[p2]:.3f} valley/peak={valley/smaller_peak:.2f} sep={sep:.3f}"


def fit_loglog(xs, ys):
    lx = [math.log(x) for x in xs]
    ly = [math.log(y) for y in ys]
    n = len(lx)
    mx = sum(lx) / n; my = sum(ly) / n
    sxx = sum((x - mx) ** 2 for x in lx)
    sxy = sum((lx[i] - mx) * (ly[i] - my) for i in range(n))
    return sxy / sxx if sxx > 0 else float("nan")


def main():
    paths = sys.argv[1:]
    if not paths:
        print("usage: analyze_genesis_criticality.py crit_*_L*.csv")
        return 1
    data = load(paths)
    Ls = sorted(data.keys())
    print("=" * 74)
    print("FTD genesis transition -- ORDER via finite-size scaling")
    print("=" * 74)

    # per (L,T) moments
    Tc_of_L = {}
    chi_max_of_L = {}
    U4_min_of_L = {}
    samples_at_Tc = {}
    for L in Ls:
        N = L ** 3
        Ts = sorted(data[L].keys())
        print(f"\nL={L}  (N={N})  T-points={len(Ts)}")
        print(f"  {'T':>7} {'<m>':>9} {'chi=N*Var':>12} {'U4':>8} {'nsamp':>8}")
        best_chi = -1; best_T = None; u4min = 1e9
        for T in Ts:
            mo = moments(data[L][T])
            chi = N * mo["var"]
            print(f"  {T:>7.4f} {mo['mean']:>9.5f} {chi:>12.4f} {mo['U4']:>8.4f} {mo['n']:>8d}")
            if chi > best_chi:
                best_chi = chi; best_T = T
            if not math.isnan(mo["U4"]) and mo["U4"] < u4min:
                u4min = mo["U4"]
        Tc_of_L[L] = best_T
        chi_max_of_L[L] = best_chi
        U4_min_of_L[L] = u4min
        samples_at_Tc[L] = data[L][best_T]
        print(f"  -> chi-peak at T_c(L={L}) = {best_T:.4f}  (chi_max={best_chi:.3f}, U4_min={u4min:.4f})")

    Lmax = Ls[-1]

    # ---- D1: P(m) modality at the largest-L susceptibility peak ----
    bim, info = is_bimodal(samples_at_Tc[Lmax])
    centers, counts = histogram(samples_at_Tc[Lmax], nbins=30)
    print(f"\n[D1] P(m) at T_c(L={Lmax}={Tc_of_L[Lmax]:.4f}):  {'BIMODAL' if bim else 'UNIMODAL'}  ({info})")
    # ascii sparkline of the histogram
    if counts:
        mx = max(counts) or 1
        bars = "".join(" .:-=+*#%@"[min(8, int(c / mx * 8))] for c in counts)
        print(f"     P(m) [{centers[0]:.3f} .. {centers[-1]:.3f}]: |{bars}|")
    d1 = "FIRST-ORDER" if bim else "CRITICAL"

    # ---- D2: Binder minimum vs crossing ----
    # FIRST-ORDER if U4_min(Lmax) deep (<0.4) and deepening with L.
    u4_deepens = all(U4_min_of_L[Ls[i]] >= U4_min_of_L[Ls[i + 1]] - 0.03 for i in range(len(Ls) - 1))
    deep = U4_min_of_L[Lmax] < 0.40
    print(f"[D2] Binder U4_min by L: " + ", ".join(f"L{L}={U4_min_of_L[L]:.3f}" for L in Ls))
    if deep and u4_deepens and len(Ls) >= 2:
        d2 = "FIRST-ORDER"
        print(f"     -> deep minimum ({U4_min_of_L[Lmax]:.3f}<0.40) deepening with L => FIRST-ORDER")
    elif U4_min_of_L[Lmax] >= 0.50:
        d2 = "CRITICAL"
        print(f"     -> shallow (U4_min={U4_min_of_L[Lmax]:.3f}>=0.50), crossing-like => CRITICAL")
    else:
        d2 = "INCONCLUSIVE"
        print(f"     -> ambiguous (U4_min={U4_min_of_L[Lmax]:.3f})")

    # ---- D3: chi_max(L) scaling exponent ----
    d3 = "INCONCLUSIVE"; a = float("nan")
    if len(Ls) >= 3:
        a = fit_loglog(Ls, [chi_max_of_L[L] for L in Ls])
        if a >= 2.6:
            d3 = "FIRST-ORDER"
        elif a <= 2.2:
            d3 = "CRITICAL"
        print(f"[D3] chi_max ~ L^{a:.3f}  (>=2.6 first-order / <=2.2 critical) => {d3}")
    else:
        print(f"[D3] need >=3 lattice sizes for chi_max scaling (have {len(Ls)}) => INCONCLUSIVE")

    # ---- verdict ----
    votes = [d1, d2, d3]
    nfo = votes.count("FIRST-ORDER")
    ncr = votes.count("CRITICAL")
    if nfo >= 2:
        verdict = "GENESIS-FIRST-ORDER"
    elif ncr >= 2:
        verdict = "GENESIS-CRITICAL"
    else:
        verdict = "INCONCLUSIVE"

    print("\n" + "=" * 74)
    print(f"  D1(modality)={d1}   D2(Binder)={d2}   D3(chi-scaling)={d3}")
    print(f"  ===> VERDICT: {verdict}")
    print("=" * 74)
    if verdict == "GENESIS-FIRST-ORDER":
        print("  Genesis is a FIRST-ORDER (discontinuous) transition: no diverging")
        print("  correlation length, no critical fixed point, no scaling spectrum.")
        print("  The cluster-mass ladder (N~A^2) is ENERGY-BUDGET / pattern formation,")
        print("  NOT an RG-flow-derived spectrum. [BOUNDARY] -- genesis is RG-IRRELEVANT")
        print("  as a spectrum generator.")
    elif verdict == "GENESIS-CRITICAL":
        print("  Genesis is a 2nd-order CRITICAL point: diverging xi, a scaling fixed")
        print("  point, non-trivial exponents. Genesis is a RELEVANT operator -- the")
        print("  cluster spectrum has genuine RG content. NEXT: extract nu, beta, gamma")
        print("  and test for a known universality class (directed percolation?).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
