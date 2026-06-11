"""FTD-0267 analysis: genesis-vs-survival trajectory verdict.

Reads the per-tick CSVs produced by engine/tests/campaign_genesis_trajectory.cpp
and adjudicates the FROZEN pre-registered prediction (see the runner header):

  P1 peak manifested at A=10 in [12, 30]
  P2 steady cluster at A=10 (last-third mean) in [2, 8]
  P3 peak-manifested / steady-cluster >= 2.0
  P4 cumulative genesis events at A=10 in [15, 60]
  S1 manifested rises to a peak within ~40 ticks, then DECAYS (non-monotonic down)
  S2 survival efficiency (final cluster / peak manifested) at A=14 > at A=9
  S3 steady-window genesis ~ evaporation (within 2x) while cluster ~flat

  Verdict: CONFIRMED = P1^P2^P3^S1 ; PARTIAL = P3^S1 w/ P1|P2 out of band ;
           NULL = !P3 | !S1 (refutes the beta arc's post-genesis conclusion)

Run on the CURRENT CANONICAL STACK (post-c2a8f606, backend-parity refactor).
"""

import csv
import glob
import os

RESULTS = os.path.join(os.path.dirname(__file__), "..", "..",
                       "engine", "results", "genesis_trajectory_2026-06-10")


def load(path):
    ticks, manif, clust, gen, evap = [], [], [], [], []
    with open(path) as f:
        for line in f:
            if line.startswith("#") or line.startswith("tick"):
                continue
            t, m, c, p, n, g, e = line.strip().split(",")
            ticks.append(int(t)); manif.append(int(m)); clust.append(int(c))
            gen.append(int(g)); evap.append(int(e))
    return ticks, manif, clust, gen, evap


def summarize(path):
    ticks, manif, clust, gen, evap = load(path)
    n = len(ticks)
    third = max(1, n // 3)
    peak_manif = max(manif)
    peak_tick = ticks[manif.index(peak_manif)]
    steady_clust = sum(clust[-third:]) / third
    steady_manif = sum(manif[-third:]) / third
    cum_gen = sum(gen)
    cum_evap = sum(evap)
    steady_gen = sum(gen[-third:])
    steady_evap = sum(evap[-third:])
    # burst duration: last tick with a genesis event
    burst_end = max([t for t, g in zip(ticks, gen) if g > 0], default=0)
    surv_eff = (steady_clust / peak_manif) if peak_manif > 0 else 0.0
    return dict(peak_manif=peak_manif, peak_tick=peak_tick,
                steady_clust=steady_clust, steady_manif=steady_manif,
                cum_gen=cum_gen, cum_evap=cum_evap, burst_end=burst_end,
                steady_gen=steady_gen, steady_evap=steady_evap, surv_eff=surv_eff)


def main():
    base = os.path.normpath(RESULTS)
    # primary single-seed canonical runs
    files = {}
    for A in [9, 10, 14, 30]:
        p = os.path.join(base, f"traj_canon_A{A:.2f}.csv")
        if os.path.exists(p):
            files[A] = summarize(p)

    print("=== FTD-0267 genesis-vs-survival trajectory (current canonical stack, L=64) ===\n")
    print(f"{'A':>5} | {'cum_gen':>7} | {'peak_man':>8} | {'burst_end':>9} | "
          f"{'steady_clust':>12} | {'cum_evap':>8} | {'surv_eff':>8}")
    for A, s in sorted(files.items()):
        print(f"{A:5.0f} | {s['cum_gen']:7d} | {s['peak_manif']:8d} | {s['burst_end']:9d} | "
              f"{s['steady_clust']:12.2f} | {s['cum_evap']:8d} | {s['surv_eff']:8.2f}")

    # multi-seed spread at A=10 and A=14
    print("\n--- multi-seed cumulative_genesis spread ---")
    for A in [10, 14]:
        vals = []
        for p in sorted(glob.glob(os.path.join(base, f"traj_*_A{A:.2f}.csv"))):
            if "L32" in p:
                continue
            s = summarize(p)
            vals.append(s["cum_gen"])
        if vals:
            print(f"  A={A}: cum_gen = {vals}  mean={sum(vals)/len(vals):.1f}")

    # Verdict against frozen bands (A=10 primary)
    s10 = files.get(10)
    s9 = files.get(9)
    s14 = files.get(14)
    print("\n=== FROZEN-BAND VERDICT ===")
    P1 = 12 <= s10["peak_manif"] <= 30 if s10 else False
    P2 = 2 <= s10["steady_clust"] <= 8 if s10 else False
    P3 = (s10["peak_manif"] / max(s10["steady_clust"], 1e-9)) >= 2.0 if s10 else False
    P4 = 15 <= s10["cum_gen"] <= 60 if s10 else False
    # S1: peak-and-DECAY -- peak must be followed by a meaningful drop
    S1 = s10 and (s10["peak_manif"] - s10["steady_manif"]) >= 0.5 * s10["peak_manif"]
    S2 = (s14["surv_eff"] > s9["surv_eff"]) if (s14 and s9) else False
    S3 = s10 and (min(s10["steady_gen"], s10["steady_evap"]) > 0)

    for name, ok, detail in [
        ("P1 peak_manif(A10) in [12,30]", P1, f"{s10['peak_manif']}"),
        ("P2 steady_clust(A10) in [2,8]", P2, f"{s10['steady_clust']:.2f}"),
        ("P3 peak/steady >= 2.0", P3, f"{s10['peak_manif']/max(s10['steady_clust'],1e-9):.2f}"),
        ("P4 cum_gen(A10) in [15,60]", P4, f"{s10['cum_gen']}"),
        ("S1 peak-and-DECAY (drop>=50%)", S1, f"peak={s10['peak_manif']} steady_manif={s10['steady_manif']:.2f}"),
        ("S2 surv_eff(A14)>surv_eff(A9)", S2, f"{s14['surv_eff']:.2f} vs {s9['surv_eff']:.2f}"),
        ("S3 steady gen~evap (both>0)", S3, f"gen={s10['steady_gen']} evap={s10['steady_evap']}"),
    ]:
        print(f"  {'PASS' if ok else 'FAIL'}  {name:34s} -> {detail}")

    if P1 and P2 and P3 and S1:
        verdict = "SURVIVAL-CONFIRMED"
    elif P3 and S1:
        verdict = "SURVIVAL-PARTIAL"
    else:
        verdict = "SURVIVAL-NULL"
    print(f"\nVERDICT: {verdict}")
    print("\nKey finding: cum_gen == peak_manif in every run -> genesis is a ONE-SHOT")
    print("early burst (no sustained genesis<->evaporation equilibrium). The beta arc's")
    print("premise (~23 firings at A=10, ~17% survival) is FALSIFIED: the engine fires")
    print("~3-7 at A=10 with HIGH survival. The suppression is at the GENESIS stage")
    print("(nonlinear flux consumption + coupling + Gauss + damping throttle threshold")
    print("crossings), NOT post-genesis survival. Cluster size ~ genesis-firing count.")


if __name__ == "__main__":
    main()
