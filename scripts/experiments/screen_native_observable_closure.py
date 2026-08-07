"""FTD-0778 - Gate A (natural-coordinate closure) screen on the FTD-0776 corpus.

Read-only. Executes exactly the metrics locked in
docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_NATIVE_OBSERVABLE_CLOSURE_SCREEN_v1.md
before any inspection of the data. No engine execution; no artifact modified.
"""
import numpy as np, pandas as pd, json, os, pathlib

REPO = pathlib.Path(__file__).resolve().parents[2]
RAW = REPO / "engine" / "results" / "gstar_qactive_pilot_20260802" / "raw"
ARMS = [10, 12, 14, 16]


def binned_r2(x, y, nbins, mincount=20):
    """R^2 of y against a 1-D binned-mean estimator of f(x)."""
    lo, hi = np.percentile(x, [0.1, 99.9])
    edges = np.linspace(lo, hi, nbins + 1)
    idx = np.digitize(x, edges) - 1
    ok = (idx >= 0) & (idx < nbins)
    idx, yv = idx[ok], y[ok]
    cnt = np.bincount(idx, minlength=nbins)
    s = np.bincount(idx, weights=yv, minlength=nbins)
    good = cnt >= mincount
    mean = np.zeros(nbins)
    mean[good] = s[good] / cnt[good]
    keep = good[idx]
    resid = yv[keep] - mean[idx[keep]]
    tot = yv[keep] - yv[keep].mean()
    return 1.0 - resid.var() / tot.var(), keep.sum()


def binned_r2_2d(x1, x2, y, nb, mincount=20):
    """R^2 of y against a 2-D binned-mean estimator of f(x1,x2)."""
    out = []
    for v in (x1, x2):
        lo, hi = np.percentile(v, [0.1, 99.9])
        out.append(np.linspace(lo, hi, nb + 1))
    i1 = np.digitize(x1, out[0]) - 1
    i2 = np.digitize(x2, out[1]) - 1
    ok = (i1 >= 0) & (i1 < nb) & (i2 >= 0) & (i2 < nb)
    i1, i2, yv = i1[ok], i2[ok], y[ok]
    flat = i1 * nb + i2
    cnt = np.bincount(flat, minlength=nb * nb)
    s = np.bincount(flat, weights=yv, minlength=nb * nb)
    good = cnt >= mincount
    mean = np.zeros(nb * nb)
    mean[good] = s[good] / cnt[good]
    keep = good[flat]
    resid = yv[keep] - mean[flat[keep]]
    tot = yv[keep] - yv[keep].mean()
    return 1.0 - resid.var() / tot.var(), keep.sum()


def savgol(y, w=21, order=3):
    from scipy.signal import savgol_filter
    return savgol_filter(y, w, order)


results = {}
for A in ARMS:
    f = os.path.join(str(RAW), f"native_L32_A{A}_seed1.csv")
    df = pd.read_csv(f, usecols=["tick", "q_active", "p_active"])
    q = df["q_active"].to_numpy(float)
    p = df["p_active"].to_numpy(float)

    # central differences, dt = 1 tick
    v = np.gradient(q)
    a_from_q = np.gradient(v)
    a_from_p = np.gradient(p)

    r = {"n_ticks": int(len(q)),
         "q_std": float(q.std()), "q_absmax": float(np.abs(q).max())}

    # P0 - momentum consistency: p = mu * dq/dt ?
    A_ = np.vstack([v, np.ones_like(v)]).T
    coef, *_ = np.linalg.lstsq(A_, p, rcond=None)
    pred = A_ @ coef
    r["P0_r2"] = float(1 - ((p - pred) ** 2).sum() / ((p - p.mean()) ** 2).sum())
    r["P0_mu"] = float(coef[0])

    for tag, acc in (("fromq", a_from_q), ("fromp", a_from_p)):
        m1, n1 = binned_r2(q, acc, 200)
        m2, n2 = binned_r2_2d(q, v, acc, 60)
        r[f"M1_{tag}"] = float(m1)
        r[f"M2_{tag}"] = float(m2)
        # M3 - stationarity of F(q) between first and last third
        t3 = len(q) // 3
        lo, hi = np.percentile(q, [5, 95])
        edges = np.linspace(lo, hi, 60)
        def prof(sl):
            i = np.digitize(q[sl], edges) - 1
            ok = (i >= 0) & (i < 59)
            c = np.bincount(i[ok], minlength=59)
            s = np.bincount(i[ok], weights=acc[sl][ok], minlength=59)
            m = np.full(59, np.nan)
            g = c >= 20
            m[g] = s[g] / c[g]
            return m
        f1, f2 = prof(slice(0, t3)), prof(slice(-t3, None))
        both = ~np.isnan(f1) & ~np.isnan(f2)
        if both.sum() > 5:
            scale = np.nanmax(np.abs(np.concatenate([f1[both], f2[both]])))
            r[f"M3_{tag}"] = float(np.max(np.abs(f1[both] - f2[both])) / scale)
            r[f"M3_{tag}_bins"] = int(both.sum())
        else:
            r[f"M3_{tag}"] = None

    # smoothed sensitivity check
    qs = savgol(q)
    vs = np.gradient(qs)
    accs = np.gradient(vs)
    r["M1_smooth"] = float(binned_r2(qs, accs, 200)[0])
    r["M2_smooth"] = float(binned_r2_2d(qs, vs, accs, 60)[0])

    # exploratory: envelope decay across thirds
    t3 = len(q) // 3
    r["env_thirds"] = [float(np.abs(q[i * t3:(i + 1) * t3]).max()) for i in range(3)]

    results[f"A{A}"] = r
    print(f"=== arm A={A} ===")
    print(f"  P0  p vs dq/dt      R2 = {r['P0_r2']:.4f}   (mu = {r['P0_mu']:.4f})")
    print(f"  M1  a = F(q)        R2 = {r['M1_fromq']:.4f} [from q]   {r['M1_fromp']:.4f} [from p]   {r['M1_smooth']:.4f} [smoothed]")
    print(f"  M2  a = F(q,qdot)   R2 = {r['M2_fromq']:.4f} [from q]   {r['M2_fromp']:.4f} [from p]   {r['M2_smooth']:.4f} [smoothed]")
    def fmt(x):
        return "n/a (insufficient overlapping bins)" if x is None else f"{x:.4f}"
    print(f"  M3  F drift 1st/3rd    = {fmt(r['M3_fromq'])} [from q]   {fmt(r['M3_fromp'])} [from p]")
    print(f"  env |q|max by third    = {[round(x,3) for x in r['env_thirds']]}")
    print()


out = REPO / "engine" / "results" / "gstar_qactive_pilot_20260802" / "ftd_0778_closure_screen.json"
with open(out, "w") as fh:
    json.dump(results, fh, indent=2)

print("=== PREREGISTERED DECISION RULE (FTD-0778) ===")
verdicts = []
for k, r in results.items():
    m1 = max(r["M1_fromq"], r["M1_fromp"])
    m2 = max(r["M2_fromq"], r["M2_fromp"])
    hi = max(r.get("hi_band", 0.0), 0.0)
    if m1 > 0.95:
        h, v = "H0 natural", "NATIVE_OBSERVABLE_CLOSURE_PASS"
    elif m2 > 0.95:
        h, v = "H1 dissipative", "NATIVE_OBSERVABLE_CLOSURE_DISSIPATIVE"
    else:
        h, v = "H2 hidden state", "NATIVE_OBSERVABLE_CLOSURE_FAILED"
    verdicts.append(v)
    print(f"  {k}: M1={m1:.4f}  M2={m2:.4f}  ->  {h}  [{v}]")

print()
print(f"Majority verdict: {max(set(verdicts), key=verdicts.count)}")
print("N1 noise control is reported by diag_native_observable_closure.py; a")
print("closure failure with upper-half-band power near 1.0 would instead be")
print("NATIVE_OBSERVABLE_CLOSURE_UNINFORMATIVE_NOISE.")
print("M3 reported n/a indicates the first- and last-third q ranges barely")
print("overlap, which is itself a monotone-drift signature.")
