#!/usr/bin/env python3
"""
explore_cluster_relaxation.py — EXPLORATORY analyzer for Exp-B (P4 N_internal + P1 cost<->N).

EXPLORATORY, NOT a pre-registered verdict. See .claude/plans/lazy-conjuring-marble.md.
Reads relaxation_summary.csv and prints per-prediction SIGNAL / NO-SIGNAL / AMBIGUOUS
(signal-detection only; results are NOT evidence).

P1 (cost<->N): does maintenance cost (E_wave_eq) co-vary with size N_eq?
  CONFOUND CHECK: the Langevin thermostat injects ~T/gamma per DOF, so E_wave ~ 3*N*T
  by equipartition alone. We test whether the cost->N link is anything BEYOND that.
P4 (N_internal): does tau_relax grow with T and N (kinematics fixed)?
  Uses non-censored runs only; reports censoring rate; checks the kinematic null.

Usage: python explore_cluster_relaxation.py [results_dir]
"""
import sys, os, csv, math

def load(path):
    rows = []
    with open(path) as f:
        for r in csv.DictReader(f):
            rows.append({k: float(v) for k, v in r.items()})
    return rows

def spearman(xs, ys):
    n = len(xs)
    if n < 3: return float("nan")
    def ranks(v):
        order = sorted(range(n), key=lambda i: v[i])
        rk = [0.0]*n; i = 0
        while i < n:
            j = i
            while j+1 < n and v[order[j+1]] == v[order[i]]: j += 1
            avg = (i+j)/2.0 + 1
            for k in range(i, j+1): rk[order[k]] = avg
            i = j+1
        return rk
    rx, ry = ranks(xs), ranks(ys)
    mx, my = sum(rx)/n, sum(ry)/n
    cov = sum((rx[i]-mx)*(ry[i]-my) for i in range(n))
    vx = math.sqrt(sum((r-mx)**2 for r in rx)); vy = math.sqrt(sum((r-my)**2 for r in ry))
    return cov/(vx*vy) if vx>0 and vy>0 else float("nan")

def main():
    rdir = sys.argv[1] if len(sys.argv) > 1 else "engine/results/cluster_thermo_2026-05-30"
    rows = load(os.path.join(rdir, "relaxation_summary.csv"))
    print("="*70)
    print("  EXPLORATORY analysis — Exp-B relaxation (P4 N_internal, P1 cost<->N)")
    print("  NOT pre-registered; signal-detection only.")
    print("="*70)
    formed = [r for r in rows if r["N_eq"] > 0]
    print(f"\nruns: {len(rows)}  | clusters formed (N_eq>0): {len(formed)}  | "
          f"no-cluster (A too low): {len(rows)-len(formed)}")

    # ---- kinematic null ----
    max_tau_kin = max(abs(r["mean_tau_kin"]) for r in rows)
    print(f"\n[kinematic null] max |mean_tau_kin| = {max_tau_kin:.3g}  "
          f"({'OK: latency off, no kinematic-tau contamination' if max_tau_kin < 1e-9 else 'WARN: nonzero'})")

    # ---- P1: cost vs N, with the equipartition confound ----
    print("\n[P1] cost (E_wave_eq) vs size (N_eq)")
    rho = spearman([r["N_eq"] for r in formed], [r["E_wave_eq"] for r in formed])
    print(f"  Spearman rho(E_wave_eq, N_eq) [all formed] = {rho:.3f}")
    print("  CONFOUND: E_wave/N per T (equipartition predicts ~const*T, i.e. cost/N independent of N):")
    Ts = sorted(set(r["T"] for r in formed))
    equipart = True
    for T in Ts:
        sub = [r for r in formed if r["T"] == T]
        eper = [r["E_wave_eq"]/r["N_eq"] for r in sub]
        mean_eper = sum(eper)/len(eper)
        rho_eperN = spearman([r["N_eq"] for r in sub], eper)
        print(f"    T={T:<6}: <E_wave/N>={mean_eper:.4f}  (3T={3*T:.4f})  rho(E/N, N)={rho_eperN:+.2f}")
        if abs(rho_eperN) > 0.5: equipart = False
    if equipart:
        print("  => cost/N is ~flat in N at each T and tracks ~3T -> the cost<->N link is")
        print("     TRIVIAL EQUIPARTITION (thermostat energy per voxel), NOT a deep dissipation law.")
        print("  P1 VERDICT: SIGNAL-but-TRIVIAL (confounded by equipartition) -> AMBIGUOUS / not evidence.")
    else:
        print("  => cost/N varies with N beyond equipartition -> a non-trivial cost law may exist.")
        print("  P1 VERDICT: SIGNAL (beyond equipartition) -> worth pre-registering.")

    # ---- P4: tau_relax vs T and N ----
    print("\n[P4] tau_relax vs T and N  (non-censored, kinematics fixed)")
    nonc = [r for r in formed if r["censored"] < 0.5]
    cens_rate = 1.0 - len(nonc)/len(formed) if formed else 1.0
    print(f"  censoring rate = {cens_rate*100:.0f}%  ({len(nonc)}/{len(formed)} recovered)")
    if len(nonc) < 8:
        print("  too few non-censored runs to assess a trend.")
        print("  P4 VERDICT: NO-SIGNAL (recovery rarely completes; operationalization too destructive).")
    else:
        logt = [math.log(r["tau_relax"]) for r in nonc]
        rho_T = spearman([r["T"] for r in nonc], logt)
        rho_N = spearman([r["N_eq"] for r in nonc], logt)
        print(f"  Spearman rho(log tau_relax, T)    = {rho_T:+.3f}")
        print(f"  Spearman rho(log tau_relax, N_eq) = {rho_N:+.3f}")
        sig_T = abs(rho_T) > 0.3
        sig_N = abs(rho_N) > 0.3
        if rho_T > 0.3 and rho_N > 0.3:
            print("  P4 VERDICT: SIGNAL (tau_relax grows with both T and N) -> pre-register.")
        elif sig_T or sig_N:
            print("  P4 VERDICT: PARTIAL (one of T/N trends; the other flat).")
        else:
            print("  P4 VERDICT: NO-SIGNAL (tau_relax flat in both T and N; high censoring/noise).")

    print("\n" + "="*70)
    print("  (Exploratory. A positive here would require a fresh PRE-REGISTERED run on")
    print("   new seeds before counting as evidence. A null is itself a useful boundary.)")
    print("="*70)

if __name__ == "__main__":
    main()
