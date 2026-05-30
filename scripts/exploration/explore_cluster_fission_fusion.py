#!/usr/bin/env python3
"""
explore_cluster_fission_fusion.py — EXPLORATORY analyzer for Exp-A (P2 + P3).

EXPLORATORY, NOT a pre-registered verdict. See .claude/plans/lazy-conjuring-marble.md.
Prints per-prediction SIGNAL / NO-SIGNAL / AMBIGUOUS (signal-detection only; NOT evidence).

P2 fission: spontaneous (occurs at delta=0) + conservative (|delta_fis| small).
P2 fusion : compatibility-gated  P(merge | same-sign vs opposite-sign, detuning).
P3 fusion : lossy   lambda = org_merged / (org1+org2) < 1.

Usage: python explore_cluster_fission_fusion.py [results_dir]
"""
import sys, os, csv

def load(path):
    if not os.path.exists(path): return None
    with open(path) as f:
        return list(csv.DictReader(f))

def fnum(r, k):
    try: return float(r[k])
    except: return float("nan")

def main():
    rdir = sys.argv[1] if len(sys.argv) > 1 else "engine/results/cluster_thermo_2026-05-30"
    print("="*70)
    print("  EXPLORATORY analysis — Exp-A fission/fusion (P2, P3)")
    print("  NOT pre-registered; signal-detection only.")
    print("="*70)

    # ----------------------- P2 fission -----------------------
    fis = load(os.path.join(rdir, "fission_events.csv"))
    print("\n[P2 fission] spontaneous + conservative?")
    if not fis:
        print("  (no fission_events.csv)")
    else:
        tot_fis = sum(int(float(r["n_fission"])) for r in fis)
        tot_death = sum(int(float(r["n_death"])) for r in fis)
        spont = [r for r in fis if int(float(r["driven"])) == 0]
        driven = [r for r in fis if int(float(r["driven"])) == 1]
        fis_spont = sum(int(float(r["n_fission"])) for r in spont)
        fis_driven = sum(int(float(r["n_fission"])) for r in driven)
        print(f"  total fission events = {tot_fis}  (spontaneous delta=0: {fis_spont}, "
              f"driven delta>0: {fis_driven});  total deaths = {tot_death}")
        if tot_fis == 0:
            print("  => clusters NEVER split into two persistent children — they persist or die wholesale.")
            print("  P2 fission VERDICT: NO-SIGNAL (no fission events; the pillar is not exhibited here).")
        else:
            dfis = [fnum(r, "delta_fis_mean") for r in fis if int(float(r["n_fission"])) > 0]
            med = sorted(dfis)[len(dfis)//2] if dfis else float("nan")
            print(f"  median delta_fis (size change on split) = {med:+.3f}")
            verdict = "SIGNAL (conservative + spontaneous)" if (abs(med) < 0.15 and fis_spont > 0) \
                      else "AMBIGUOUS (non-conservative or driver-dependent)"
            print(f"  P2 fission VERDICT: {verdict}")

    # ----------------------- P2 fusion gate + P3 lossiness -----------------------
    fus = load(os.path.join(rdir, "fusion_outcomes.csv"))
    print("\n[P2 fusion] compatibility-gated?   [P3] lossy?")
    if not fus:
        print("  (no fusion_outcomes.csv yet — fusion run may still be in progress)")
    else:
        def same_sign(r): return float(r["s1"]) == float(r["s2"])
        same = [r for r in fus if same_sign(r)]
        opp  = [r for r in fus if not same_sign(r)]
        def merge_rate(rs):
            return (sum(1 for r in rs if r["outcome"] == "merge")/len(rs)) if rs else float("nan")
        from collections import Counter
        print(f"  outcomes overall: {dict(Counter(r['outcome'] for r in fus))}")
        print(f"  same-sign (compatible) merge rate   = {merge_rate(same)*100:.0f}%  (n={len(same)})")
        print(f"  opposite-sign (incompatible) merge  = {merge_rate(opp)*100:.0f}%  (n={len(opp)})")
        # detuning trend among same-sign
        merges = [r for r in same if r["outcome"] == "merge"]
        if same:
            det_merge = [fnum(r, "detuning") for r in merges]
            det_nomerge = [fnum(r, "detuning") for r in same if r["outcome"] != "merge"]
            mm = (sum(det_merge)/len(det_merge)) if det_merge else float("nan")
            nm = (sum(det_nomerge)/len(det_nomerge)) if det_nomerge else float("nan")
            print(f"  same-sign mean detuning: merged={mm:.3f}  not-merged={nm:.3f} "
                  f"(gate predicts merged < not-merged)")
        gated = merge_rate(same) > merge_rate(opp) + 0.2 if (same and opp) else False
        print(f"  P2 fusion VERDICT: {'SIGNAL (same-sign merges, opposite-sign does not)' if gated else 'NO-SIGNAL/AMBIGUOUS (no clear gate)'}")

        # P3 lossiness
        lam = []
        for r in merges:
            denom = fnum(r, "org1") + fnum(r, "org2")
            if denom > 1e-9 and fnum(r, "org_merged") > 0:
                lam.append(fnum(r, "org_merged")/denom)
        if lam:
            med = sorted(lam)[len(lam)//2]
            print(f"  [P3] median lambda = org_merged/(org1+org2) = {med:.3f}  (lossy if <1; n_merge={len(lam)})")
            print(f"  P3 VERDICT: {'SIGNAL (fusion lossy)' if med < 0.95 else 'NO-SIGNAL (not lossy)'}")
        else:
            print("  [P3] no merges with measurable org -> cannot assess lossiness.")

    print("\n" + "="*70)
    print("  (Exploratory. Positives require a fresh PRE-REGISTERED run before counting")
    print("   as evidence. Nulls map a boundary and are themselves useful.)")
    print("="*70)

if __name__ == "__main__":
    main()
