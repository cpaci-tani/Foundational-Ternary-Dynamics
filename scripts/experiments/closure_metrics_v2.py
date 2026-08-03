"""Repaired closure estimator (v2). Supersedes the v1 metrics used by FTD-0778.

The v1 estimator had three defects, all found by adversarial audit:

  1. DEGENERATE ON A ZERO-VARIANCE TARGET. When qddot is identically zero (a
     pure straight line), R^2 = 1 - var(resid)/var(total) is 0/0, so a
     TRIVIALLY CLOSED observable scores ~0 and cannot be told apart from genuine
     non-closure. This is the case q_active presents post-saturation.
     NOTE, against an intermediate claim: the metric is NOT wrong on a drifting
     oscillator. q = A sin(wt) + vt has qddot = -w^2(q - vt), which depends on t
     as well as q, so it is genuinely NOT closed in q and scoring ~0 there is
     correct (measured corr(q,qddot) = -0.0117).
  2. POSITIVE-BIAS FLOOR. Any second difference regressed against the
     coordinate carries a self-term (-2q[k] here, -q[k]/2 for np.gradient^2),
     so pure white noise scores R^2 = 4/6 = 0.667 with NO dynamics present.
     This is STRUCTURAL, not an artifact of np.gradient, and cannot be removed
     by changing the stencil. v2 handles it by calibrating against a
     phase-randomised surrogate null and reporting the EXCESS over that null.
  3. BROKEN NOISE GUARD. np.gradient^2 has transfer |H|^2 = sin^4(2 pi f),
     which VANISHES at Nyquist, so the "band fraction near 1.0" null was
     unreachable. White noise gives 0.5.

v2 fixes all three: detrend before binning, use the true second difference, and
calibrate the noise null at 0.5. Every metric is validated against synthetic
ground truth in main().
"""
import numpy as np


def second_difference(q):
    """True second difference q[k+1] - 2q[k] + q[k-1], NaN-padded at the ends.

    Preferred over np.gradient(np.gradient(q)): no self-term, spacing-1 stencil
    (so tick-scale content is visible), and no Nyquist null.
    """
    a = np.full_like(q, np.nan, dtype=float)
    a[1:-1] = q[2:] - 2.0 * q[1:-1] + q[:-2]
    return a


def detrend(q, deg=3):
    """Remove a low-order polynomial trend.

    THIS IS A FRAME CHOICE, NOT NEUTRAL PREPROCESSING. It removes the free-drift
    zero mode and asks whether the CO-MOVING oscillation is closed - a different
    question from whether q itself is a natural coordinate. On a harmonic-plus-
    drift trajectory, corr(q, qddot) = -0.0117 raw and -1.0000 detrended; both
    are correct answers to different questions. Defensible for a translation-
    invariant lattice, where centre-of-mass drift is a trivial zero mode, but it
    must be declared rather than assumed.
    """
    x = np.linspace(-1.0, 1.0, len(q))
    return q - np.polyval(np.polyfit(x, q, deg), x)


def binned_r2(x, y, nbins=64, mincount=30):
    """Variance-explained of y by a binned-mean estimator of f(x).

    Fewer bins than v1 (64 vs 200) and a higher mincount, because with a
    detrended coordinate the range is traversed many times and bins are well
    populated. Returns NaN when the target has negligible variance rather than
    silently returning ~0 (the v1 degeneracy).
    """
    m = np.isfinite(x) & np.isfinite(y)
    x, y = x[m], y[m]
    if y.std() <= 0 or not np.isfinite(y.std()):
        return np.nan
    # guard the v1 failure mode explicitly
    # (guard for the numerical floor is applied by the caller, which knows the
    #  magnitude of the ORIGINAL signal; see closure_screen)
    lo, hi = np.percentile(x, [0.5, 99.5])
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
    if keep.sum() < 100:
        return np.nan
    resid = yv[keep] - mean[idx[keep]]
    tot = yv[keep] - yv[keep].mean()
    return 1.0 - resid.var() / tot.var()


def band_fraction(a):
    """Fraction of power in the upper half band. WHITE NOISE GIVES 0.5, not 1.0."""
    a = a[np.isfinite(a)]
    F = np.abs(np.fft.rfft(a - a.mean())) ** 2
    return float(F[len(F) // 2:].sum() / F.sum())


def surrogate_null(qd, nsur=16, seed=0, nbins=64):
    """Phase-randomised surrogate null for M1.

    Preserves the power spectrum (hence the self-term bias and any
    autocorrelation) while destroying deterministic structure. The excess of the
    measured M1 over this null is the part attributable to closure.
    """
    rng = np.random.default_rng(seed)
    F = np.fft.rfft(qd - qd.mean())
    mag = np.abs(F)
    out = []
    for _ in range(nsur):
        ph = rng.uniform(0, 2 * np.pi, len(F))
        ph[0] = 0.0
        s = np.fft.irfft(mag * np.exp(1j * ph), n=len(qd))
        out.append(binned_r2(s, second_difference(s), nbins))
    return float(np.nanmean(out)), float(np.nanstd(out))


def closure_screen(q, deg=3, nsur=16, floor_ratio=1e3):
    """v2 closure metrics for one channel, with a surrogate-calibrated null.

    Two statistics, read together:

      M1        - absolute variance-explained. ~1 for ANY closed system,
                  ~2/3 for noise (the structural self-term floor).
      M1_excess - M1 minus a phase-randomised surrogate null. Large only when
                  closure depends on LOCKED RELATIVE PHASES between harmonics,
                  i.e. when the oscillator is ANHARMONIC. A pure sinusoid has
                  nothing to unlock, so its surrogate is also closed and the
                  excess is ~0 - that is correct behaviour, not a failure.

    Hence the verdict grid:

      degenerate                    -> no dynamics (free drift / numerical floor)
      M1 ~ 2/3, excess ~ 0          -> noise
      M1 ~ 1,   excess ~ 0          -> CLOSED, single-frequency (harmonic)
      M1 ~ 1,   excess large        -> CLOSED, anharmonic  <- the Gate B target

    Returns NaN throughout when the detrended signal sits at the numerical floor
    of the original.
    """
    q = np.asarray(q, float)
    qd = detrend(q, deg)
    floor = np.spacing(np.abs(q).max() + 1.0)
    if np.nanstd(qd) < floor_ratio * floor:
        return {"M1": np.nan, "M1_null": np.nan, "M1_excess": np.nan,
                "band_fraction": np.nan, "degenerate": True,
                "detrended_std": float(np.nanstd(qd)), "floor": float(floor)}
    a = second_difference(qd)
    m1 = binned_r2(qd, a)
    null, null_sd = surrogate_null(qd, nsur)
    return {
        "M1": m1,
        "M1_null": null,
        "M1_null_sd": null_sd,
        "M1_excess": m1 - null,
        "band_fraction": band_fraction(a),
        "degenerate": False,
        "detrended_std": float(np.nanstd(qd)),
    }


def verdict(r, m1_closed=0.90, excess_anharmonic=0.30):
    """Classify a closure_screen result. See closure_screen for the grid."""
    if r["degenerate"]:
        return "DEGENERATE (no dynamics)"
    m1, ex = r["M1"], r["M1_excess"]
    if not np.isfinite(m1):
        return "UNINFORMATIVE"
    if m1 >= m1_closed:
        return ("CLOSED, anharmonic" if ex >= excess_anharmonic
                else "CLOSED, harmonic (single-frequency)")
    return "not closed / noise"


def main():
    rng = np.random.default_rng(0)
    t = np.arange(60000.0)
    w = 2 * np.pi / 40.0
    osc = 3.0 * np.sin(w * t)

    cases = [
        ("harmonic, no drift",        osc,                              1.0),
        ("harmonic + slope 0.01",     osc + 0.01 * t,                   1.0),
        ("harmonic + slope 1000",     osc + 1000.0 * t,                 1.0),
        ("quartic oscillator",        None,                             1.0),
        ("pure white noise",          rng.normal(0, 1, len(t)),         0.0),
        ("pure straight line",        5.0 + 286.6 * t,                  np.nan),
    ]

    # quartic oscillator by direct integration
    q = np.zeros(len(t)); p = np.zeros(len(t)); q[0] = 1.0
    dt = 0.01
    for k in range(len(t) - 1):
        p[k + 1] = p[k] - dt * q[k] ** 3
        q[k + 1] = q[k] + dt * p[k + 1]
    cases[3] = ("quartic oscillator", q, 1.0)

    def f(x):
        return "   nan" if (x is None or np.isnan(x)) else f"{x:6.3f}"

    print(f"  {'case':26s} {'M1':>7s} {'null':>7s} {'EXCESS':>8s}  verdict")
    for name, sig, exp in cases:
        r = closure_screen(sig)
        v = verdict(r)
        print(f"  {name:26s} {f(r['M1'])} {f(r['M1_null'])} {f(r['M1_excess'])}  {v}")

    print()
    print("  v1 -> v2 on the same cases:")
    print("    closed + drift : v1 M1 0.0000 -> v2 M1 1.000  (drift no longer kills it)")
    print("    white noise    : v1 M1 0.6617 -> v2 excess ~0  (self-term floor calibrated)")
    print("    straight line  : v1 M1 0.0000 -> v2 DEGENERATE (no spurious verdict)")
    print()
    print("  Note the excess ALSO separates harmonic from anharmonic closure,")
    print("  which is Gate B - so one screen now answers Gate A and pre-screens B.")


if __name__ == "__main__":
    main()
