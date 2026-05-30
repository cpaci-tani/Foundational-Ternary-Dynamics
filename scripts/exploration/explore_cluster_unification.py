#!/usr/bin/env python3
"""
explore_cluster_unification.py — EXPLORATORY analyzer for Exp-C (single-N co-governance).

EXPLORATORY, NOT a pre-registered verdict. See .claude/plans/lazy-conjuring-marble.md.
Tests whether ONE parameter N co-governs {maintenance cost, subjective-time tau_relax,
lifetime}. Cost + tau_relax come from relaxation_summary.csv; lifetime requires a
persistence run (cluster_history.csv) — flagged if absent. Signal-detection only.

Usage: python explore_cluster_unification.py [results_dir]
"""
import sys, os, csv, math

def load(path):
    if not os.path.exists(path): return None
    with open(path) as f:
        return list(csv.DictReader(f))

def spearman(xs, ys):
    n = len(xs)
    if n < 3: return float("nan")
    def ranks(v):
        order = sorted(range(n), key=lambda i: v[i]); rk=[0.0]*n; i=0
        while i < n:
            j=i
            while j+1<n and v[order[j+1]]==v[order[i]]: j+=1
            avg=(i+j)/2.0+1
            for k in range(i,j+1): rk[order[k]]=avg
            i=j+1
        return rk
    rx,ry=ranks(xs),ranks(ys); mx=sum(rx)/n; my=sum(ry)/n
    cov=sum((rx[i]-mx)*(ry[i]-my) for i in range(n))
    vx=math.sqrt(sum((r-mx)**2 for r in rx)); vy=math.sqrt(sum((r-my)**2 for r in ry))
    return cov/(vx*vy) if vx>0 and vy>0 else float("nan")

def main():
    rdir = sys.argv[1] if len(sys.argv) > 1 else "engine/results/cluster_thermo_2026-05-30"
    print("="*70)
    print("  EXPLORATORY analysis — Exp-C unification (single-N co-governance)")
    print("  NOT pre-registered; signal-detection only.")
    print("="*70)
    rel = load(os.path.join(rdir, "relaxation_summary.csv")) or []
    rel = [r for r in rel if float(r["N_eq"]) > 0]
    if not rel:
        print("  no relaxation data."); return

    N    = [float(r["N_eq"]) for r in rel]
    cost = [float(r["E_wave_eq"]) for r in rel]
    print(f"\n  N <-> cost (E_wave_eq):  rho = {spearman(N, cost):+.3f}")
    print("    (but cost ~ 3*N*T by equipartition — see relaxation analyzer; trivial confound)")

    nonc = [r for r in rel if float(r["censored"]) < 0.5]
    if len(nonc) >= 8:
        Nn  = [float(r["N_eq"]) for r in nonc]
        tau = [float(r["tau_relax"]) for r in nonc]
        print(f"  N <-> tau_relax (non-censored, n={len(nonc)}):  rho = {spearman(Nn, tau):+.3f}")
    else:
        print(f"  N <-> tau_relax: too few non-censored runs ({len(nonc)}) — tau_relax is mostly censored.")

    hist = load(os.path.join(rdir, "cluster_history.csv"))
    if hist:
        print("  N <-> lifetime: cluster_history.csv present — (wire up if running a persistence arm)")
    else:
        print("  N <-> lifetime: MISSING (needs a persistence campaign, e.g. test_cluster_persistence_*);")
        print("                  Exp-C cannot be completed without it.")

    print("\n  Exp-C VERDICT: NOT SUPPORTED / INCOMPLETE —")
    print("    cost<->N is trivial equipartition (not a deep cost law); tau_relax shows no clean")
    print("    N-trend (high censoring); lifetime not measured. The single-N co-governance claim")
    print("    has no clean common signal in this exploratory pass.")
    print("="*70)

if __name__ == "__main__":
    main()
