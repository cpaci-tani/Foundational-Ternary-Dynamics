"""verify_born_saturation_v2.py — T3 v2: does the Born-fraction saturate or
approach 1 as Omega*tau grows?

Locked instrument for PREREG_BORN_DENSITY_SATURATION_v2.md. Follow-up to
the v1.1 OUTCOME E (Born-fraction 0.024 -> 0.099 as Om*tau went 0.45-0.91
-> 1.81-3.56). Same mechanism model (deterministic two-mode standing
coherent field + per-site OU noise + threshold K = W_SC), five arms with
FRESH seed bases (arms 1-2 independently replicate v1.1's two points):

    arm  (lam1,lam2)  tau    Om*tau char (geometric mean)
    1    (64,32)      8      ~0.64
    2    (16,8)       8      ~2.5
    3    (16,8)       32     ~10.1
    4    (8,4)        32     ~19.6
    5    (8,4)        128    ~78.4

Registered statistic per arm: Born-fraction BF = (R - R_amp)/(1 - R_amp)
with 99% bootstrap CI. Locked outcomes:
    A SATURATES : BF5 CI-upper < 0.5 AND BF5 - BF4 <= 0.05
                  -> the mechanism class cannot reach Born weighting
    B APPROACHES: BF point estimates non-decreasing AND BF5 CI-lower >= 0.5
                  -> Born weighting as the fast-mode/slow-noise asymptote
    C RISING-UNRESOLVED: non-decreasing AND BF5 CI-upper < 0.5 AND
                  BF5 - BF4 > 0.05 -> direction confirmed, asymptote open
    D INVALID/INDETERMINATE: any arm killed, or the pattern matches none.
A descriptive two-parameter fit BF = B_inf * x^2/(c^2 + x^2) is reported
with NO decision weight. Mechanism-level quick-check platform; FTD-0200
closure still requires the engine campaign. Registers nothing by itself.
"""
from __future__ import annotations

import numpy as np

C = 0.57735026918962576451
DT = 1.0
L = 4096
K_THRESH = 0.5054620197
SIGMA_N = 0.17
A1 = 0.10
T_TICKS = 20000
N_SEEDS = 48
MASTER = 20260807
BOOT = 2000

ARMS = [(64, 32, 8.0), (16, 8, 8.0), (16, 8, 32.0),
        (8, 4, 32.0), (8, 4, 128.0)]

x = np.arange(L)
w_cont = lambda k: 2 * C * np.sin(k / 2)
Om = lambda k: 2 * np.arcsin(np.clip(w_cont(k) * DT / 2, -1, 1)) / DT


def run_arm(lam1, lam2, tau, seed_base, label):
    k1, k2 = 2 * np.pi / lam1, 2 * np.pi / lam2
    O1, O2 = Om(k1), Om(k2)
    amp2 = A1 * np.sqrt(O1 / O2)
    R_AMP = O1 / O2
    x_char = np.sqrt(O1 * O2) * tau
    prof1, prof2 = np.cos(k1 * x), np.cos(k2 * x)
    alpha = np.exp(-1.0 / tau)
    sig_step = SIGMA_N * np.sqrt(1 - alpha * alpha)
    theta = np.random.default_rng(seed_base + 1).uniform(0, 2 * np.pi, 2)

    def coh(t):
        return (A1 * prof1 * np.cos(O1 * t + theta[0])
                + amp2 * prof2 * np.cos(O2 * t + theta[1]))

    excess_per_seed = np.zeros((N_SEEDS, L))
    tot_sig = tot_ctl = 0
    for s in range(N_SEEDS):
        rng = np.random.default_rng(seed_base + 100 + s)
        draws = rng.standard_normal((T_TICKS, L))
        sig_counts = None
        for mode, acc in (("sig", 1.0), ("ctl", 0.0)):
            xi = np.zeros(L)
            prev = xi + acc * coh(0)
            counts = np.zeros(L)
            for t in range(1, T_TICKS):
                xi = alpha * xi + sig_step * draws[t]
                F = xi + acc * coh(t)
                counts += (prev < K_THRESH) & (F >= K_THRESH)
                prev = F
            if mode == "sig":
                sig_counts = counts
                tot_sig += counts.sum()
            else:
                excess_per_seed[s] = sig_counts - counts
                tot_ctl += counts.sum()
    X = np.column_stack([np.ones(L), np.cos(2 * k1 * x), np.cos(2 * k2 * x)])

    def bf(ex):
        c, *_ = np.linalg.lstsq(X, ex, rcond=None)
        R = c[2] / c[1]
        return (R - R_AMP) / (1.0 - R_AMP), c[1], c[2]

    BF, c1, c2 = bf(excess_per_seed.mean(axis=0))
    rngb = np.random.default_rng(seed_base + 999)
    bs = []
    for _ in range(BOOT):
        pick = rngb.integers(0, N_SEEDS, N_SEEDS)
        b, *_ = bf(excess_per_seed[pick].mean(axis=0))
        bs.append(b)
    lo, hi = np.percentile(bs, [0.5, 99.5])
    net = tot_sig - tot_ctl
    kill = []
    if net < 5000:
        kill.append("excess < 5000")
    if c1 <= 0 or c2 <= 0:
        kill.append("non-positive coefficient")
    print(f"arm {label}: lam=({lam1},{lam2}) tau={tau:.0f}  "
          f"Om*tau=({O1*tau:.1f},{O2*tau:.1f}) x_char={x_char:.2f}")
    print(f"   excess {int(net):,}  c=({c1:.3e},{c2:.3e})  "
          f"BF = {BF:.4f}  99% CI [{lo:.4f}, {hi:.4f}]"
          f"{'  KILL: ' + str(kill) if kill else ''}")
    return None if kill else dict(x=x_char, bf=BF, lo=lo, hi=hi)


def main():
    print("=" * 72)
    print("T3 v2 — Born-fraction saturation scan (locked)")
    print(f"  L={L}, T={T_TICKS}, seeds={N_SEEDS}, sigma_n={SIGMA_N}, "
          f"K={K_THRESH}, A1={A1}")
    print("=" * 72)
    res = []
    for i, (l1, l2, tau) in enumerate(ARMS, 1):
        r = run_arm(l1, l2, tau, MASTER + 100000 * i, str(i))
        if r is None:
            print("OUTCOME D — EXECUTION INVALID (arm killed)")
            return
        res.append(r)
    bfs = [r["bf"] for r in res]
    print("-" * 72)
    print("BF sequence:", "  ".join(f"{b:.4f}" for b in bfs))
    monotone = all(bfs[i + 1] >= bfs[i] - 1e-12 for i in range(len(bfs) - 1))
    late = bfs[4] - bfs[3]
    # descriptive fit (no decision weight)
    xs = np.array([r["x"] for r in res])
    ys = np.array(bfs)
    best = None
    for Binf in np.linspace(0.05, 1.0, 96):
        for c in np.geomspace(0.5, 300, 160):
            pred = Binf * xs ** 2 / (c ** 2 + xs ** 2)
            sse = ((pred - ys) ** 2).sum()
            if best is None or sse < best[0]:
                best = (sse, Binf, c)
    print(f"descriptive fit BF = B*x^2/(c^2+x^2): B_inf = {best[1]:.3f}, "
          f"c = {best[2]:.1f}, sse = {best[0]:.2e}  (no decision weight)")
    if monotone and res[4]["lo"] >= 0.5:
        print("OUTCOME B — APPROACHES: Born weighting is the fast-mode/"
              "slow-noise asymptote")
    elif res[4]["hi"] < 0.5 and late <= 0.05:
        print(f"OUTCOME A — SATURATES amplitude-side "
              f"(BF5 = {bfs[4]:.3f}, late rise {late:+.3f}): the mechanism "
              f"class cannot reach Born weighting")
    elif monotone and res[4]["hi"] < 0.5 and late > 0.05:
        print(f"OUTCOME C — RISING-UNRESOLVED (late rise {late:+.3f})")
    else:
        print("OUTCOME D — INDETERMINATE (no registered pattern)")


if __name__ == "__main__":
    main()
