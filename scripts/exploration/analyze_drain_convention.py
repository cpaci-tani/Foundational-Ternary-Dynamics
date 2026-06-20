#!/usr/bin/env python3
"""
FTD-0110 Convention Audit — drain-arm adjudicator (SHA-locked pre-reg artifact).

Question (exit ii): is the kinetic_drain calibration of the N(A) cluster-mass law
pure CONVENTION (only the dimensionless shape is physical; the rest is an affine
rescaling of (A, N)) or PHYSICAL (drain changes the dimensionless shape)?

Discriminator (rigorous): a broken power law N = c * A^p has exponents INVARIANT
under any affine rescaling A -> A/lambda, N -> N/mu (only the knee location and the
prefactor move). So:
  * if p_lo(drain) and p_hi(drain) are constant across drain AND all curves collapse
    onto one master curve under a single per-drain affine rescaling  => CONVENTION.
  * if either exponent moves with drain beyond the reseeding-noise band, OR the
    curves do not collapse                                            => PHYSICAL.

Input : a drain_scan CSV with columns  drain,A,seed,N,settle,L  (campaign_drain_scan.cpp).
Output: per-drain (knee, p_lo, p_hi, bootstrap CIs), the exponent-spread test, the
        collapse residual, and a mechanical CONVENTION / PHYSICAL / UNDETERMINED verdict.

Gates (pre-registered, STRICT band per owner decision 2026-06-19):
  CONVENTION  iff  spread(p_lo) < 0.10 AND spread(p_hi) < 0.10            (exponent-invariant)
                   AND collapse_medianCV < 0.05                          (one-rescaling collapse)
  PHYSICAL    iff  spread(p_lo) >= 0.10 OR spread(p_hi) >= 0.10
                   OR collapse_medianCV >= 0.05
  where spread(p) = (max_d p(d) - min_d p(d)) / mean_d p(d).
  UNDETERMINED is impossible by construction here (the two gates partition); a
  separate FIT-FAIL flag fires if any per-drain broken-power fit is ill-conditioned
  (a segment with <2 points or a non-finite slope), in which case no verdict is emitted.

Deterministic: bootstrap uses a fixed integer seed; no Date/random-without-seed.
"""

import sys
import argparse
import numpy as np


def load(path):
    rows = {}
    with open(path) as fh:
        header = fh.readline().strip().split(",")
        ci = {name: i for i, name in enumerate(header)}
        for line in fh:
            line = line.strip()
            if not line:
                continue
            parts = line.split(",")
            drain = float(parts[ci["drain"]])
            A = float(parts[ci["A"]])
            N = float(parts[ci["N"]])
            seed = int(parts[ci["seed"]])
            rows.setdefault(drain, {}).setdefault(A, []).append((seed, N))
    return rows


def broken_power_fit(A, N):
    """Segmented log-log fit. Returns (knee, p_lo, p_hi, rms, ok).

    Searches the knee over interior grid points; each side needs >=2 points and a
    finite slope. Picks the knee minimizing total log10 RMS.
    """
    A = np.asarray(A, float)
    N = np.asarray(N, float)
    mask = (A > 0) & (N > 0)
    A, N = A[mask], N[mask]
    order = np.argsort(A)
    A, N = A[order], N[order]
    x, y = np.log10(A), np.log10(N)
    n = len(A)
    if n < 4:
        return (np.nan, np.nan, np.nan, np.inf, False)

    best = None
    # knee candidate = each interior point that leaves >=2 points on each side
    for k in range(2, n - 1):
        xs_lo, ys_lo = x[: k + 1], y[: k + 1]
        xs_hi, ys_hi = x[k:], y[k:]
        if len(xs_lo) < 2 or len(xs_hi) < 2:
            continue
        try:
            plo, blo = np.polyfit(xs_lo, ys_lo, 1)
            phi, bhi = np.polyfit(xs_hi, ys_hi, 1)
        except Exception:
            continue
        if not (np.isfinite(plo) and np.isfinite(phi)):
            continue
        res = np.concatenate([ys_lo - (plo * xs_lo + blo),
                              ys_hi - (phi * xs_hi + bhi)])
        rms = float(np.sqrt(np.mean(res ** 2)))
        if best is None or rms < best[3]:
            best = (float(A[k]), float(plo), float(phi), rms, True)
    if best is None:
        return (np.nan, np.nan, np.nan, np.inf, False)
    return best


def nbar_curve(per_A, seed_subset=None):
    """Mean N(A) over seeds (optionally a bootstrap subset of seed indices)."""
    As, Ns = [], []
    for A in sorted(per_A):
        vals = [N for (_s, N) in per_A[A]]
        if seed_subset is not None:
            vals = [vals[i] for i in seed_subset if i < len(vals)]
        if not vals:
            continue
        As.append(A)
        Ns.append(float(np.mean(vals)))
    return np.array(As), np.array(Ns)


def bootstrap_exponents(per_A, B=400, seed=12345):
    rng = np.random.RandomState(seed)
    nseed = max(len(v) for v in per_A.values())
    plos, phis, knees = [], [], []
    for _ in range(B):
        idx = rng.randint(0, nseed, size=nseed)
        As, Ns = nbar_curve(per_A, seed_subset=list(idx))
        knee, plo, phi, _rms, ok = broken_power_fit(As, Ns)
        if ok:
            plos.append(plo); phis.append(phi); knees.append(knee)
    f = lambda a: (float(np.mean(a)), float(np.std(a))) if a else (np.nan, np.nan)
    return f(plos), f(phis), f(knees)


def collapse_residual(fits, curves):
    """Rescale each drain curve by A->A/knee, N->N/N(knee); measure cross-drain
    scatter on a shared log-A' grid. Returns median coefficient of variation."""
    rescaled = []
    for d, (knee, plo, phi, rms, ok) in fits.items():
        if not ok or not np.isfinite(knee) or knee <= 0:
            continue
        A, N = curves[d]
        # N at knee via log-log interpolation
        xs, ys = np.log10(A), np.log10(N)
        N_knee = 10 ** np.interp(np.log10(knee), xs, ys)
        if N_knee <= 0:
            continue
        rescaled.append((np.log10(A / knee), N / N_knee))
    if len(rescaled) < 2:
        return np.nan
    lo = max(r[0].min() for r in rescaled)
    hi = min(r[0].max() for r in rescaled)
    if not (hi > lo):
        return np.nan
    grid = np.linspace(lo, hi, 24)
    stacks = []
    for (xp, Np) in rescaled:
        order = np.argsort(xp)
        stacks.append(np.interp(grid, xp[order], Np[order]))
    stacks = np.array(stacks)               # (n_drain, n_grid)
    mean = stacks.mean(axis=0)
    cv = stacks.std(axis=0) / np.where(mean == 0, np.nan, mean)
    return float(np.nanmedian(cv))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("csv")
    ap.add_argument("--exp-band", type=float, default=0.10,
                    help="STRICT exponent-spread gate (fractional)")
    ap.add_argument("--collapse-band", type=float, default=0.05,
                    help="collapse median-CV gate (fractional)")
    ap.add_argument("--boot", type=int, default=400)
    args = ap.parse_args()

    data = load(args.csv)
    drains = sorted(data)
    print(f"FTD-0110 Convention Audit — drain arm")
    print(f"  csv={args.csv}  drains={drains}")
    print(f"  gates: exponent-spread < {args.exp_band:.2f} (STRICT) AND "
          f"collapse medianCV < {args.collapse_band:.2f}")
    print()

    fits, curves = {}, {}
    any_fail = False
    print(f"  {'drain':>7} {'knee':>6} {'p_lo':>7} {'p_hi':>7} "
          f"{'p_lo_boot':>14} {'p_hi_boot':>14} {'rms':>6}")
    for d in drains:
        As, Ns = nbar_curve(data[d])
        curves[d] = (As, Ns)
        knee, plo, phi, rms, ok = broken_power_fit(As, Ns)
        fits[d] = (knee, plo, phi, rms, ok)
        (plo_m, plo_s), (phi_m, phi_s), _kn = bootstrap_exponents(data[d], B=args.boot)
        if not ok:
            any_fail = True
        print(f"  {d:>7.3f} {knee:>6.1f} {plo:>7.3f} {phi:>7.3f} "
              f"{plo_m:>7.3f}+/-{plo_s:<5.3f} {phi_m:>7.3f}+/-{phi_s:<5.3f} {rms:>6.3f}")

    plos = np.array([fits[d][1] for d in drains if fits[d][4]])
    phis = np.array([fits[d][2] for d in drains if fits[d][4]])
    spread = lambda a: (a.max() - a.min()) / abs(a.mean()) if len(a) and a.mean() != 0 else np.nan
    s_lo, s_hi = spread(plos), spread(phis)
    cv = collapse_residual(fits, curves)

    print()
    print(f"  p_lo across drains: spread = {s_lo*100:5.1f}%  (gate < {args.exp_band*100:.0f}%)")
    print(f"  p_hi across drains: spread = {s_hi*100:5.1f}%  (gate < {args.exp_band*100:.0f}%)")
    print(f"  collapse median CV: {cv*100:5.1f}%  (gate < {args.collapse_band*100:.0f}%)")
    print()

    if any_fail:
        print("  VERDICT: FIT-FAIL — at least one per-drain broken-power fit was "
              "ill-conditioned; no convention verdict emitted. Inspect the A-grid.")
        return 2

    exponent_invariant = (s_lo < args.exp_band) and (s_hi < args.exp_band)
    collapses = np.isfinite(cv) and (cv < args.collapse_band)
    if exponent_invariant and collapses:
        print("  VERDICT: CONVENTION — drain is an affine (A,N) rescaling; only the "
              "dimensionless N(A) shape is physical. (Combine with gamma=PHYSICAL "
              "[established, Leg B] => the SPLIT boundary.)")
        return 0
    else:
        why = []
        if not exponent_invariant:
            why.append(f"exponents move (p_lo {s_lo*100:.0f}%, p_hi {s_hi*100:.0f}%)")
        if not collapses:
            why.append(f"no collapse (medianCV {cv*100:.0f}%)")
        print(f"  VERDICT: PHYSICAL — drain changes the dimensionless shape: "
              f"{'; '.join(why)}. Exit (ii) fails for drain too.")
        return 1


if __name__ == "__main__":
    sys.exit(main())
