"""verify_latency_gate_mechanism.py — T3 slow-gate candidacy, Stage B:
does a latency-timescale additive channel shift threshold weighting
toward Born, with the fast flux noise still present?

Locked instrument for PREREG_LATENCY_SLOW_GATE_v1.md. The engine Stage A
(campaign_latency_slow_gate) measures tau_lat, the correlation time of
latency fluctuations under native churn, and tau_flux ~ 1.5 ticks for
the band-locked flux noise. This instrument runs the two-mode
equal-occupation discrimination with COMPOSITE per-site noise,

    xi = sqrt(1-f) * xi_fast(tau_fast)  +  sqrt(f) * xi_slow(tau_slow),

tau_fast = 1.5 (the flux value), tau_slow = tau_lat (Stage A's locked
output, passed as argv[1]), total std fixed at sigma_n = 0.17, and scans
the slow share f in {0, 0.25, 0.5, 0.75, 1.0}. Registered statistic per
f: Born-fraction BF with 99% bootstrap CI (the estimator of the executed
Born preregistrations).

Pre-blessed outcomes (see the prereg):
    V  VIABLE-ADDITIVE : BF nondecreasing in f AND BF(1) CI-floor >= 0.5
    P  PARTIAL         : nondecreasing AND BF(1) < 0.5
    N  NOT VIABLE      : BF(1) - BF(0) <= 0.02 or non-monotone
    D  INVALID         : any cell killed
Mechanism-level, quick-check platform, [IMPOSED] ensembles throughout;
registers nothing by itself.
"""
from __future__ import annotations

import sys
import numpy as np

C = 0.57735026918962576451
L = 4096
K_THRESH = 0.5054620197
SIGMA_N = 0.17
TAU_FAST = 1.5
LAM1, LAM2 = 16, 8
A1 = 0.10
T_TICKS = 20000
N_SEEDS = 32
MASTER = 20260807
BOOT = 2000
F_GRID = (0.0, 0.25, 0.5, 0.75, 1.0)

x = np.arange(L)
Om = lambda k: 2 * np.arcsin(np.clip(C * np.sin(k / 2), -1, 1))


def run_share(f, tau_slow, seed_base):
    k1, k2 = 2 * np.pi / LAM1, 2 * np.pi / LAM2
    O1, O2 = Om(k1), Om(k2)
    A2 = A1 * np.sqrt(O1 / O2)
    R_AMP = O1 / O2
    prof1, prof2 = np.cos(k1 * x), np.cos(k2 * x)
    aF = np.exp(-1.0 / TAU_FAST)
    aS = np.exp(-1.0 / tau_slow)
    sF = SIGMA_N * np.sqrt(1 - f) * np.sqrt(1 - aF * aF)
    sS = SIGMA_N * np.sqrt(f) * np.sqrt(1 - aS * aS)
    th = np.random.default_rng(seed_base + 1).uniform(0, 2 * np.pi, 2)

    def coh(t):
        return (A1 * prof1 * np.cos(O1 * t + th[0])
                + A2 * prof2 * np.cos(O2 * t + th[1]))

    excess = np.zeros((N_SEEDS, L))
    tot_sig = tot_ctl = 0
    for s in range(N_SEEDS):
        rng = np.random.default_rng(seed_base + 100 + s)
        dF = rng.standard_normal((T_TICKS, L))
        dS = rng.standard_normal((T_TICKS, L))
        sig_counts = None
        for mode, acc in (("sig", 1.0), ("ctl", 0.0)):
            xf = np.zeros(L)
            xs = np.zeros(L)
            prev = acc * coh(0)
            counts = np.zeros(L)
            for t in range(1, T_TICKS):
                xf = aF * xf + sF * dF[t]
                xs = aS * xs + sS * dS[t]
                F = xf + xs + acc * coh(t)
                counts += (prev < K_THRESH) & (F >= K_THRESH)
                prev = F
            if mode == "sig":
                sig_counts = counts
                tot_sig += counts.sum()
            else:
                excess[s] = sig_counts - counts
                tot_ctl += counts.sum()
    X = np.column_stack([np.ones(L), np.cos(2 * k1 * x), np.cos(2 * k2 * x)])

    def bf(ex):
        c, *_ = np.linalg.lstsq(X, ex, rcond=None)
        return ((c[2] / c[1]) - R_AMP) / (1.0 - R_AMP), c[1], c[2]

    BF, c1, c2 = bf(excess.mean(axis=0))
    rngb = np.random.default_rng(seed_base + 999)
    bs = []
    for _ in range(BOOT):
        pick = rngb.integers(0, N_SEEDS, N_SEEDS)
        b, *_ = bf(excess[pick].mean(axis=0))
        bs.append(b)
    lo, hi = np.percentile(bs, [0.5, 99.5])
    net = tot_sig - tot_ctl
    kill = []
    if net < 5000:
        kill.append("excess<5000")
    if c1 <= 0 or c2 <= 0:
        kill.append("coef<=0")
    print(f"  f={f:4.2f}: excess {int(net):>10,}  BF = {BF:7.4f}  "
          f"99% CI [{lo:.4f}, {hi:.4f}]{'  KILL ' + str(kill) if kill else ''}")
    return None if kill else dict(f=f, bf=BF, lo=lo, hi=hi)


def main():
    if len(sys.argv) < 2:
        print("usage: verify_latency_gate_mechanism.py <tau_lat>")
        return 1
    tau_slow = float(sys.argv[1])
    print("=" * 72)
    print(f"Stage B — composite-noise discrimination "
          f"(tau_fast={TAU_FAST}, tau_slow={tau_slow} from Stage A)")
    print(f"  L={L} T={T_TICKS} seeds={N_SEEDS} sigma_n={SIGMA_N} "
          f"K={K_THRESH} modes=({LAM1},{LAM2})")
    print("=" * 72)
    res = []
    for i, f in enumerate(F_GRID):
        r = run_share(f, tau_slow, MASTER + 300000 + 1000 * i)
        if r is None:
            print("OUTCOME D — EXECUTION INVALID")
            return 1
        res.append(r)
    bfs = [r["bf"] for r in res]
    mono = all(bfs[i + 1] >= bfs[i] - 0.01 for i in range(len(bfs) - 1))
    print("-" * 72)
    print("BF(f):", "  ".join(f"{b:.4f}" for b in bfs))
    if mono and res[-1]["lo"] >= 0.5:
        print("OUTCOME V — VIABLE-ADDITIVE: a latency-timescale channel "
              "carries the weighting to Born dominance")
    elif mono and bfs[-1] - bfs[0] > 0.02:
        print(f"OUTCOME P — PARTIAL: Born-ward, monotone, but "
              f"BF(1) = {bfs[-1]:.3f} < 0.5 at these parameters")
    elif bfs[-1] - bfs[0] <= 0.02:
        print("OUTCOME N — NOT VIABLE: slow share does not move the "
              "weighting")
    else:
        print("OUTCOME D — INDETERMINATE (non-monotone)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
