"""Gate A diagnostics: distinguish hidden-state dependence from noise dominance
and from a varying summation index set. Read-only."""
import numpy as np, pandas as pd, os, json

REPO = __import__("pathlib").Path(__file__).resolve().parents[2]
RAW = REPO / "engine" / "results" / "gstar_qactive_pilot_20260802" / "raw"

for A in [10, 12, 14, 16]:
    df = pd.read_csv(os.path.join(str(RAW), f"native_L32_A{A}_seed1.csv"))
    q = df["q_active"].to_numpy(float)
    p = df["p_active"].to_numpy(float)
    ac = df["active_count"].to_numpy(float)
    v = np.gradient(q)
    a = np.gradient(v)

    print(f"=== arm A={A} ===")
    # (1) does the summation index set vary?
    print(f"  active_count: min={ac.min():.0f} max={ac.max():.0f} mean={ac.mean():.3f} "
          f"std={ac.std():.3f}  n_changes={int((np.diff(ac)!=0).sum())} / {len(ac)-1}")

    # (2) is the acceleration tick-scale noise?
    #     lag-1 autocorrelation: white noise -> ~0 ; smooth dynamics -> ~1
    def ac1(x):
        x = x - x.mean()
        return float((x[:-1] * x[1:]).sum() / (x * x).sum())
    print(f"  lag-1 autocorr:  q={ac1(q):+.4f}  v={ac1(v):+.4f}  a={ac1(a):+.4f}")

    # fraction of a's power at the Nyquist half-band (pure tick-scale jitter)
    F = np.abs(np.fft.rfft(a - a.mean())) ** 2
    hi = F[len(F) // 2:].sum() / F.sum()
    print(f"  a: power fraction in upper half-band = {hi:.4f}   (1.0 = pure Nyquist noise)")

    # (3) magnitudes: is there any slow component at all?
    from numpy.polynomial import polynomial as P
    w = 501
    ker = np.ones(w) / w
    q_slow = np.convolve(q, ker, mode="same")
    print(f"  q: std={q.std():.4f}   std of 501-tick moving mean={q_slow.std():.4f}  "
          f"ratio={q_slow.std()/q.std():.4f}")

    # (4) does |q| envelope decay, and over what scale?
    nb = 20
    seg = len(q) // nb
    env = [float(np.abs(q[i*seg:(i+1)*seg]).max()) for i in range(nb)]
    print(f"  |q|max over 20 segments: first={env[0]:.3f} mid={env[nb//2]:.3f} last={env[-1]:.3f}")

    # (5) exploratory: do the OTHER channels close any better?
    for col in ["q_all", "q_center"]:
        x = df[col].to_numpy(float)
        vx = np.gradient(x); axx = np.gradient(vx)
        lo, hi_ = np.percentile(x, [0.1, 99.9])
        edges = np.linspace(lo, hi_, 201)
        idx = np.digitize(x, edges) - 1
        ok = (idx >= 0) & (idx < 200)
        cnt = np.bincount(idx[ok], minlength=200)
        s = np.bincount(idx[ok], weights=axx[ok], minlength=200)
        good = cnt >= 20
        mean = np.zeros(200); mean[good] = s[good] / cnt[good]
        keep = good[idx[ok]]
        res = axx[ok][keep] - mean[idx[ok][keep]]
        tot = axx[ok][keep] - axx[ok][keep].mean()
        r2 = 1 - res.var() / tot.var()
        print(f"  [exploratory] {col:9s}: a=F(x) R2 = {r2:.4f}   lag-1 autocorr(a) = {ac1(axx):+.4f}")
    print()
