"""Adjudication inputs for PREREG_TWO_CLOCK_CONSISTENCY_v1.

Reads the campaign CSV and prints the frozen quantities (§3 predictions,
§4 outcome-map inputs, validity gates V1-V4). Prints no verdict word;
the verdict is applied by the analyst against prereg §4.

Usage: python analyze_two_clock_v1.py <csv>
"""
import math
import sys
from collections import defaultdict

CSV = sys.argv[1] if len(sys.argv) > 1 else "twoclock_v1.csv"

gates = {}
cohort = defaultdict(list)        # (arm, shell) -> [(lat, e_local, decay_tick)]
surv = defaultdict(dict)          # (arm, shell) -> {tick: alive}
pairsum = {}                      # shell -> (n_pair, n_diff, lbar, sqrt_term)
p_pred = None

for line in open(CSV, encoding="utf-8", errors="replace"):
    p = line.rstrip("\n").split(",")
    if p[0] == "GATE":
        gates[(p[1], p[2])] = float(p[3])
        if p[2] == "p_pred":
            p_pred = float(p[3])
    elif p[0] == "COHORT":
        cohort[(p[1], int(p[2]))].append((float(p[6]), float(p[7]), int(p[8])))
    elif p[0] == "SURV":
        surv[(p[1], int(p[2]))][int(p[3])] = int(p[4])
    elif p[0] == "PAIRSUM":
        # v1.1: PAIRSUM,tag,shell,n_pair,n_diff,lbar,sqrt_term
        pairsum[(p[1], int(p[2]))] = (int(p[3]), int(p[4]), float(p[5]), float(p[6]))

shells = sorted({s for (_, s) in cohort})

def fit_rate(series):
    """Least-squares slope of ln(alive) vs tick, over the range where the
    population is between 90% and 10% of its initial value (avoids the
    flat head and the noisy tail)."""
    if not series:
        return float("nan"), 0
    n0 = series.get(0, 0)
    if n0 < 5:
        return float("nan"), 0
    pts = [(t, n) for t, n in sorted(series.items())
           if 0.10 * n0 <= n <= 0.90 * n0 and n > 0]
    if len(pts) < 5:
        return float("nan"), len(pts)
    st = sy = stt = sty = 0.0
    for t, n in pts:
        y = math.log(n)
        st += t; sy += y; stt += t * t; sty += t * y
    k = len(pts)
    d = k * stt - st * st
    if abs(d) < 1e-30:
        return float("nan"), k
    return -(k * sty - st * sy) / d, k

print(f"=== HAZARD (uniform by design) ===")
print(f"  p_pred = {p_pred:.6e} per tick  (mean lifetime {1/p_pred:.1f} ticks)")

print("\n=== V4 hazard uniformity + V1 well (per shell) ===")
for arm in ("M", "F", "Z"):
    for s in shells:
        c = cohort[(arm, s)]
        if not c:
            continue
        lats = [x[0] for x in c]
        es = [x[1] for x in c]
        print(f"  {arm} shell r={s:2d}: n={len(c):4d}  "
              f"L in [{min(lats):.4f},{max(lats):.4f}] mean {sum(lats)/len(lats):.4f}  "
              f"E_local dev from 0.588 = {max(abs(e - 0.588) for e in es):.3e}")

print("\n=== V2 frozen field ===")
for arm in ("M", "F", "Z"):
    k = (arm, "max_flux_drift")
    if k in gates:
        print(f"  {arm}: max |dJ| over the window = {gates[k]:.3e}  (V2 needs < 1e-12)")

print("\n=== V3 statistics + rate fits ===")
rates = {}
for arm in ("M", "F", "Z"):
    for s in shells:
        ser = surv[(arm, s)]
        if not ser:
            continue
        n0 = ser.get(0, 0)
        nend = ser[max(ser)]
        frac_decayed = 1.0 - (nend / n0 if n0 else 0)
        r, k = fit_rate(ser)
        rates[(arm, s)] = r
        print(f"  {arm} r={s:2d}: n0={n0:4d}  decayed {frac_decayed*100:5.1f}%  "
              f"fit rate = {r:.6e}/tick  (fit pts {k})  "
              f"rate/p_pred = {r/p_pred if p_pred else float('nan'):.4f}")

TAGS = [("MvZ", "M", "Z"), ("MvF", "M", "F"), ("FvZ", "F", "Z")]
for tag, a, b in TAGS:
    label = "PRIMARY" if tag == "MvZ" else "secondary"
    print(f"\n=== {label}: bit-level pairing {tag} (prereg §3) ===")
    print(f"  {'shell':>6} {'n_pair':>7} {'n_diff':>7} {'L_bar('+a+')':>11} "
          f"{'sqrt(1-L^2)':>12} {'rate_'+a+'/rate_'+b:>14}")
    tot_pair = tot_diff = 0
    for s in shells:
        if (tag, s) not in pairsum:
            continue
        n_pair, n_diff, lbar, sq = pairsum[(tag, s)]
        tot_pair += n_pair
        tot_diff += n_diff
        ra, rb_ = rates.get((a, s), float("nan")), rates.get((b, s), float("nan"))
        ratio = ra / rb_ if rb_ and not math.isnan(rb_) else float("nan")
        print(f"  {s:>6} {n_pair:>7} {n_diff:>7} {lbar:>11.4f} {sq:>12.4f} {ratio:>14.4f}")
    print(f"  TOTAL: {tot_pair} paired voxels, {tot_diff} with differing decay tick")
    if tag == "MvZ":
        deep = [s for s in shells if (tag, s) in pairsum and pairsum[(tag, s)][2] > 0.1]
        if deep:
            s0 = min(deep)
            print(f"  Deepest shell with L_bar>0.1: r={s0}, "
                  f"L_bar={pairsum[(tag, s0)][2]:.4f}, "
                  f"sqrt(1-L^2)={pairsum[(tag, s0)][3]:.4f}  "
                  f"(Outcome A needs this <= 0.93 for the null to be meaningful)")

print("\n(verdict: apply prereg §4 to the numbers above)")
