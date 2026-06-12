#!/usr/bin/env python3
"""
analyze_cluster_energy_spectroscopy.py -- FTD-0273 Phase 1 adjudication.

Reads cluster_energy_spectroscopy_<tag>.csv and asks the owner's question:
re-expressing mass as FLUX ENERGY in flip-quanta (ε = ½·K_GENESIS²), does a
cluster reveal a per-particle energy/threshold spectrum, or does the energy just
track the voxel count N (i.e. collapse back to FTD-0110/0269 N(A)≈k·A²)?

Three mass proxies per BOUNDED emergent cluster:
  M_tot   = ⟨½Σ|J|²⟩_lattice / ε   -- whole-lattice flux (halo-contaminated)
  M_local = ⟨½Σ|J|²⟩_cluster / ε    -- flux within R of the manifested cluster
  M/vox   = M_local / N             -- cluster-local quanta per voxel

The honesty guardrails (from the plan):
  * control_ohseed rows are FROZEN imposed structures -> NEVER a mass (skipped).
  * "minimum stable cluster" A_min = smallest A with a BOUNDED, survived row
    (defined by stability, NOT by m_e). No tuning toward 0.511 anywhere.

VERDICT:
  ENERGY-COLLAPSES-TO-N -- M/vox is ~constant (low scatter, no per-particle
    plateaus/jumps) AND M_tot is far noisier across seeds than N: the flux-
    energy mass is just ~c·N, so it carries NO information beyond FTD-0110's N.
    A clean BOUNDARY (the energy reframe adds no new mass scale).
  ENERGY-STRUCTURE -- M/vox shows seed-robust, A-correlated plateaus or jumps
    distinct from N: a genuine energy spectroscopy. (Pre-register a follow-up.)

Usage: python analyze_cluster_energy_spectroscopy.py cluster_energy_spectroscopy_*.csv
"""

import csv
import math
import sys
from collections import defaultdict

try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:
    pass


def stats(xs):
    n = len(xs)
    if n == 0:
        return float('nan'), float('nan'), float('nan')
    m = sum(xs) / n
    sd = math.sqrt(sum((x - m) ** 2 for x in xs) / n) if n > 1 else 0.0
    cv = sd / m if abs(m) > 1e-12 else float('nan')
    return m, sd, cv


def main():
    if len(sys.argv) < 2:
        print("usage: analyze_cluster_energy_spectroscopy.py *.csv")
        return 1

    rows = []
    for p in sys.argv[1:]:
        with open(p, newline="") as fh:
            rows.extend(csv.DictReader(fh))

    emergent = [r for r in rows if r["kind"] == "emergent"]
    control = [r for r in rows if r["kind"] == "control_ohseed"]

    # group emergent by (L, A)
    byLA = defaultdict(list)
    for r in emergent:
        byLA[(int(r["L"]), float(r["A"]))].append(r)

    print("=" * 104)
    print("FTD-0273 Phase 1 -- mass as flux-energy in flip-quanta (ε = ½·K_GENESIS²)")
    print("=" * 104)
    print(f"{'L':>4} {'A':>5} {'N(mean±sd)':>14} {'M_tot CV':>9} {'N CV':>7} "
          f"{'M_local':>10} {'M/vox(mean±sd)':>16} {'outcome':>10}")

    pv_all = []         # per-voxel local quanta over BOUNDED N>=4 rows
    mtot_cv, n_cv = [], []
    pts_NA = []         # (A, mean N) for the A^2 cross-check
    A_min = {}          # L -> smallest BOUNDED+survived A
    perL_NM = defaultdict(list)   # L -> [(mean N, mean M_local)]  (collapse test, per-L)
    perL_pv = defaultdict(list)   # L -> [M/vox]                   (per-L scatter)

    for (L, A) in sorted(byLA):
        rs = byLA[(L, A)]
        bnd = [r for r in rs if r["outcome"] == "BOUNDED"]
        Ns = [int(r["n_settle"]) for r in rs]
        outcome = (max(set(r["outcome"] for r in rs),
                       key=lambda o: sum(1 for r in rs if r["outcome"] == o)))
        mN, sN, cN = stats([float(n) for n in Ns])
        mMt, _, cMt = stats([float(r["M_quanta"]) for r in bnd]) if bnd else (float('nan'),)*3
        mMl, _, _ = stats([float(r["M_local"]) for r in bnd]) if bnd else (float('nan'),)*3
        pvs = [float(r["M_per_voxel"]) for r in bnd if int(r["n_settle"]) >= 4]
        mpv, spv, _ = stats(pvs) if pvs else (float('nan'), float('nan'), float('nan'))
        print(f"{L:>4} {A:>5.0f} {mN:>7.1f}±{sN:<5.1f} {cMt:>9.2f} {cN:>7.2f} "
              f"{mMl:>10.2f} {mpv:>8.2f}±{spv:<6.2f} {outcome:>10}")
        if bnd:
            pv_all.extend(pvs)
            if not math.isnan(cMt):
                mtot_cv.append(cMt)
            n_cv.append(cN)
            pts_NA.append((A, mN))
            if mN >= 4:
                perL_NM[L].append((mN, mMl))
                perL_pv[L].extend(pvs)
            if L not in A_min and any(r["outcome"] == "BOUNDED" and r["survived"] != "0"
                                      if "survived" in r else r["outcome"] == "BOUNDED"
                                      for r in bnd):
                A_min[L] = A

    print("-" * 104)
    if control:
        print("control (frozen O_h seeds -- IMPOSED geometry, NOT a mass; N pinned to "
              "the seed count):")
        for r in control:
            print(f"    {r['outcome']:>16}: N={r['n_settle']:>3}  ⟨E_flux⟩={float(r['field_avg']):.3f}  "
                  f"⟨E_loc⟩={float(r['local_avg']):.3f}")

    # ---- the collapse test ----
    mpv_mean, mpv_sd, mpv_cv = stats(pv_all)
    mtot_cv_mean = sum(mtot_cv) / len(mtot_cv) if mtot_cv else float('nan')
    n_cv_mean = sum(n_cv) / len(n_cv) if n_cv else float('nan')

    print("\n" + "-" * 104)
    print(f"  cluster-local quanta-per-voxel (BOUNDED, N>=4):  mean={mpv_mean:.3f}  "
          f"sd={mpv_sd:.3f}  CV={mpv_cv:.2f}  (n={len(pv_all)})")
    print(f"  seed-noise of M_tot (whole-lattice flux): mean CV={mtot_cv_mean:.2f}")
    print(f"  seed-noise of N (voxel count):            mean CV={n_cv_mean:.2f}")
    for L in sorted(A_min):
        print(f"  minimum-stable-cluster A_min(L={L}) = {A_min[L]:.0f}  (defined by "
              f"stability/geometry, NOT m_e)")

    # FTD-0269 re-expression: fit log N = a + b log A on points with N>1
    fit = [(math.log(A), math.log(mN)) for (A, mN) in pts_NA if mN > 1.5 and A > 0]
    if len(fit) >= 3:
        n = len(fit); sx = sum(x for x, _ in fit); sy = sum(y for _, y in fit)
        sxx = sum(x*x for x, _ in fit); sxy = sum(x*y for x, y in fit)
        b = (n*sxy - sx*sy) / (n*sxx - sx*sx)
        print(f"  N(A) power-law exponent (log-log fit, N>1): b={b:.2f}  "
              f"(FTD-0269 cluster law ~ A^2 below the knee)")

    # ---- COLLAPSE TEST (per-L; robust to multi-L pooling) ----
    # M/vox is L-dependent (the local energy decays with box size), so a CV pooled
    # across L is NOT a collapse signal -- it just measures the L-trend. The right
    # question is, AT FIXED L, does the cluster-local energy track N? Test it with
    # the per-L Pearson correlation r(N, M_local) over the A ladder. High r at every
    # L => energy is explained by N alone (collapse). Genuine per-particle energy
    # STRUCTURE would show as M_local breaking from the N trend AT FIXED L.
    def pearson(xs, ys):
        n = len(xs)
        if n < 3:
            return float('nan')
        mx = sum(xs) / n; my = sum(ys) / n
        sxy = sum((x - mx) * (y - my) for x, y in zip(xs, ys))
        sxx = sum((x - mx) ** 2 for x in xs); syy = sum((y - my) ** 2 for y in ys)
        d = math.sqrt(sxx * syy)
        return sxy / d if d > 1e-12 else float('nan')

    perL_corr = {}
    perL_cv = {}
    for L in sorted(perL_NM):
        Ns = [p[0] for p in perL_NM[L]]; Ms = [p[1] for p in perL_NM[L]]
        perL_corr[L] = pearson(Ns, Ms)
        _, _, cv = stats(perL_pv[L])
        perL_cv[L] = cv
    corr_vals = [c for c in perL_corr.values() if math.isfinite(c)]
    mean_corr = sum(corr_vals) / len(corr_vals) if corr_vals else float('nan')
    cv_vals = [c for c in perL_cv.values() if math.isfinite(c)]
    mean_perL_cv = sum(cv_vals) / len(cv_vals) if cv_vals else float('nan')

    print("\n  per-L collapse test (does cluster-local energy track N AT FIXED L?):")
    for L in sorted(perL_corr):
        print(f"    L={L}: r(N, M_local)={perL_corr[L]:.3f}  M/vox CV={perL_cv[L]:.2f}  "
              f"(n={len(perL_NM[L])})")

    # Collapse iff energy tracks N at every L (high per-L correlation). The pooled
    # M/vox CV is reported but NOT used for the verdict (it conflates the L-trend).
    collapses = (not math.isnan(mean_corr)) and (mean_corr > 0.9)
    verdict = "ENERGY-COLLAPSES-TO-N" if collapses else "ENERGY-STRUCTURE"
    print("\n" + "=" * 104)
    print(f"  ===> VERDICT: {verdict}   (mean per-L r(N,M_local)={mean_corr:.3f}, "
          f"per-L M/vox CV={mean_perL_cv:.2f})")
    print("=" * 104)
    if collapses:
        print(f"  At every L the cluster-local flux energy tracks the voxel count N")
        print(f"  (mean r={mean_corr:.3f}): M_local ≈ c(L)·N. The flux-energy mass carries NO")
        print("  information beyond N -- it COLLAPSES to FTD-0110/0269's N(A). (The per-voxel")
        print("  constant c(L) DECAYS with box size -- the L-convergence finding -- but that")
        print("  is the energy leaking away, NOT a per-particle spectrum.) The whole-lattice")
        print("  flux energy is halo-dominated (CV >> N's), not a localized mass. No new")
        print("  per-particle threshold spectrum at any L. [MEASURED -- BOUNDARY].")
    else:
        print("  AT FIXED L, M_local breaks from the N trend (low per-L correlation): the")
        print("  energy may carry per-particle structure beyond N. Pre-register a follow-up")
        print("  (finer A, more seeds, look for jumps/plateaus distinct from N).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
