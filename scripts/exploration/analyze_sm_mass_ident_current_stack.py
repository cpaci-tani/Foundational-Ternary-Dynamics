"""FTD-0262: SM cluster<->mass identification re-assessment on the current stack.

Pre-registration: docs/theory/03_derivations/foundational_mechanics/
  PREREG_SM_MASS_IDENT_CURRENT_STACK_v1.md
Runner: engine/tests/campaign_thermostat_off_sweep.cpp (canonical protocol:
  thermostat on gamma=0.02 T=0.005, coupling on -- the FTD-0261 arm-N config).

Hash-locked WITH the pre-registration BEFORE the campaign runs. The design
separates three layers of identification content, frozen here:

  E  ANCHOR (non-circular): the electron (R=1) = the 1-voxel minimal
     manifestation. E-PASS iff every seed at every anchor amplitude yields
     largest cluster EXACTLY 1, time-stable (n_min = n_max = 1).
  S  SELF-CONSISTENCY (CIRCULAR -- flagged): injecting at the law-inverted
     amplitudes A_mu = 62.59, A_pi = 72.46 (frozen L2 fit c=0.0795, p=1.901,
     FTD-0261) must land N within the law's 2-sigma band of R:
     N_mean/R in [1/1.186, 1.186]. This tests law extrapolation, NOT the
     identification; it is pre-stated as such.
  P  SPECIALNESS (the real content): on the 7-point mu-window
     A in {56, 58, 60, 62.59, 64, 66, 68}, fit the local log-log slope
     p_local. SMOOTH iff p_local >= 0.95 (no attractor structure: the law
     just passes through R like any other value). PLATEAU-AT-R iff
     p_local < 0.95 AND the window-mean N within 10% of R_mu. The pi-window
     {70, 72.46, 74, 76} is descriptive support only.

Outcomes (priors stated in the pre-reg): IDENT-NULL (E-PASS + S-CONSISTENT +
SMOOTH); IDENT-STRUCTURE (PLATEAU-AT-R); IDENT-BROKEN (E-FAIL or
S-INCONSISTENT); MIXED otherwise.

Usage: python analyze_sm_mass_ident_current_stack.py --results-dir=PATH
"""
import argparse
import csv
import glob
import math
import os
import sys
from collections import defaultdict

R_MU, R_PI = 206.7683, 273.132
A_MU, A_PI = 62.59, 72.46
BAND = 1.186                  # 2-sigma of the FTD-0261 law residual
ANCHOR_AS = [1.5, 2.0, 3.0, 5.0]
MU_WINDOW = [56.0, 58.0, 60.0, A_MU, 64.0, 66.0, 68.0]
PI_WINDOW = [70.0, A_PI, 74.0, 76.0]
P_SMOOTH = 0.95               # local slope at/above this => SMOOTH
PLATEAU_NEAR = 0.10           # window-mean N within 10% of R for PLATEAU-AT-R
FLOOD_N = 1000.0


def load(results_dir):
    rows = []
    for path in glob.glob(os.path.join(results_dir, "sweep_*.csv")):
        with open(path, newline="") as fh:
            for r in csv.DictReader(fh):
                r["A"] = float(r["A"]); r["n_mean"] = float(r["n_mean"])
                r["n_min"] = int(r["n_min"]); r["n_max"] = int(r["n_max"])
                rows.append(r)
    return rows


def by_tag_A(rows):
    d = defaultdict(list)
    for r in rows:
        d[(r["tag"], round(r["A"], 2))].append(r)
    return d


def nbar(d, tag, A):
    rs = d.get((tag, round(A, 2)), [])
    return sum(r["n_mean"] for r in rs) / len(rs) if rs else None


def slope_loglog(points):
    xs = [math.log10(a) for a, n in points]
    ys = [math.log10(n) for a, n in points]
    n = len(xs)
    mx, my = sum(xs) / n, sum(ys) / n
    sxx = sum((x - mx) ** 2 for x in xs)
    sxy = sum((x - mx) * (y - my) for x, y in zip(xs, ys))
    return sxy / sxx if sxx else float("nan")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--results-dir", required=True)
    args = ap.parse_args()
    rows = load(args.results_dir)
    if not rows:
        print("NO DATA"); sys.exit(2)
    d = by_tag_A(rows)

    # ---- E: anchor ----
    print("=== E (electron anchor, R=1 <-> 1 voxel) ===")
    e_pass = True
    for A in ANCHOR_AS:
        rs = d.get(("E", round(A, 2)), [])
        if not rs:
            e_pass = False
            print(f"  A={A}: MISSING"); continue
        ok = all(r["n_min"] == 1 and r["n_max"] == 1 for r in rs)
        e_pass &= ok
        det = " ".join(f"[{r['n_min']},{r['n_max']}]" for r in rs)
        print(f"  A={A}: per-seed [n_min,n_max] = {det} -> {'OK' if ok else 'FAIL'}")
    print(f"E verdict: {'PASS' if e_pass else 'FAIL'}")

    # ---- S: self-consistency (circular; flagged) ----
    print("\n=== S (law-inverted SM points; SELF-CONSISTENCY ONLY — circular) ===")
    s_pass = True
    for name, A, R in (("mu", A_MU, R_MU), ("pi", A_PI, R_PI)):
        n = nbar(d, "S", A)
        if n is None:
            s_pass = False; print(f"  {name}: MISSING"); continue
        ratio = n / R
        ok = (1.0 / BAND) <= ratio <= BAND
        s_pass &= ok
        print(f"  {name}: A={A}  N_mean={n:.1f}  target R={R:.1f}  "
              f"N/R={ratio:.3f}  band [{1/BAND:.3f},{BAND:.3f}] -> {'OK' if ok else 'FAIL'}")
    print(f"S verdict: {'CONSISTENT' if s_pass else 'INCONSISTENT'}")

    # ---- P: specialness ----
    print("\n=== P (specialness probe: local slope on the mu-window) ===")
    mu_pts = []
    for A in MU_WINDOW:
        n = nbar(d, "P", A) or nbar(d, "S", A)
        if n is not None and n < FLOOD_N:
            mu_pts.append((A, n))
            print(f"  A={A:<6} N_mean={n:.1f}")
    if len(mu_pts) < 5:
        print("P verdict: UNDETERMINED (insufficient window points)")
        p_verdict = "UNDETERMINED"
    else:
        p_local = slope_loglog(mu_pts)
        wmean = sum(n for _, n in mu_pts) / len(mu_pts)
        near = abs(wmean - R_MU) / R_MU <= PLATEAU_NEAR
        print(f"  p_local = {p_local:.3f}  (law slope ~1.9; SMOOTH threshold {P_SMOOTH})")
        print(f"  window-mean N = {wmean:.1f}  (R_mu = {R_MU:.1f}; near = {near})")
        if p_local >= P_SMOOTH:
            p_verdict = "SMOOTH"
        elif near:
            p_verdict = "PLATEAU-AT-R"
        else:
            p_verdict = "STRUCTURED-ELSEWHERE"
        print(f"P verdict: {p_verdict}")
    pi_pts = []
    for A in PI_WINDOW:
        n = nbar(d, "P", A) or nbar(d, "S", A)
        if n is not None:
            pi_pts.append((A, n))
    if len(pi_pts) >= 3:
        print(f"[descriptive] pi-window slope = {slope_loglog(pi_pts):.3f} "
              f"({len(pi_pts)} pts)")

    # ---- outcome ----
    print("\n================ OUTCOME ================")
    if e_pass and s_pass and p_verdict == "SMOOTH":
        print("IDENT-NULL: anchor holds; law self-consistency holds; NO structural "
              "specialness at R_mu. The cluster<->mass identification gains no "
              "current-stack support beyond the electron anchor; its quantitative "
              "evidence basis remains historical (stack-pinned).")
    elif p_verdict == "PLATEAU-AT-R":
        print("IDENT-STRUCTURE: attractor plateau at R_mu — requires an independent "
              "confirmation campaign before any tag movement.")
    elif not e_pass or not s_pass:
        print("IDENT-BROKEN: the identification loses anchor and/or law "
              "self-consistency on the current stack.")
    else:
        print("MIXED: see component verdicts; no closure claimed.")


if __name__ == "__main__":
    main()
