"""Analyze manifestation-flow JSON output and apply the SC3 test.

Input JSON schema (list of rows):
    {"L": int, "n": float, "level": int, "seed": int,
     "flux_energy_ratio": float, "flux_energy_fine": float,
     "source_conserved": bool, "gauss_preserved": bool,
     "status": "ok" | "invalid_fit" | "settle_crash" | "harness_crash" | "parse_error"}

Output:
    <basename>_report.json  -- rollup + SC3 verdict
    <basename>_plot.pdf     -- flux_energy_ratio(n, level) and flux_energy_fine(n, level) with error bars

SC3.a baseline is the analytical Gaussian value of 1.0 for flux_energy_ratio
(not an empirical n=0 baseline, which is trivially zero with no sources).
"""
import argparse
import json
import math
import os
from collections import defaultdict


def rollup(rows):
    """Group by (L, n, level); return mean, stderr, count.

    Skips rows where status != 'ok'. Also skips rows where flux_energy_fine
    is non-positive (n=0 rows with trivially zero flux: the baseline for SC3
    is instead the theoretical value 1 at n > 0, not these empty-bridge rows).
    """
    groups = defaultdict(list)
    for r in rows:
        if r.get("status", "ok") != "ok":
            continue
        fe = r.get("flux_energy_fine", 0.0)
        if fe is None or fe <= 0.0:
            continue
        groups[(r["L"], r["n"], r["level"])].append(r)
    out = {}
    for key, gs in groups.items():
        cls = [g["flux_energy_ratio"] for g in gs]
        kts = [g["flux_energy_fine"] for g in gs]
        n = len(cls)
        mean_cl = sum(cls) / n
        mean_kt = sum(kts) / n
        if n > 1:
            var_cl = sum((c - mean_cl) ** 2 for c in cls) / (n - 1)
            var_kt = sum((k - mean_kt) ** 2 for k in kts) / (n - 1)
            se_cl = math.sqrt(var_cl / n)
            se_kt = math.sqrt(var_kt / n)
        else:
            se_cl = se_kt = float("nan")
        out[key] = {"count": n, "mean_ratio": mean_cl, "se_ratio": se_cl,
                    "mean_energy_fine": mean_kt, "se_energy_fine": se_kt}
    return out


def sc3_test(groups, sigma_threshold):
    """Apply SC3. Returns dict with verdict and details.

    SC3.a: |mean_ratio(n) - 1.0| > sigma_threshold * se_ratio at any (L, n, level).
           The analytical Gaussian value (1.0) is the theoretical baseline.
    SC3.b: linear regression d(mean_ratio)/d(log n) at each (L, level). Reports slope.
    """
    by_L_level = defaultdict(list)
    for (L, n, level), stats in groups.items():
        by_L_level[(L, level)].append((n, stats))

    triggered = []
    slopes = {}
    for (L, level), rows in by_L_level.items():
        rows.sort(key=lambda r: r[0])
        for n, s in rows:
            if n <= 0.0:
                continue
            se_val = s["se_ratio"]
            if se_val is None or (isinstance(se_val, float) and math.isnan(se_val)):
                continue
            se = se_val
            delta = abs(s["mean_ratio"] - 1.0)
            if se > 0 and delta > sigma_threshold * se:
                triggered.append({"L": L, "n": n, "level": level,
                                  "delta_ratio": delta, "se": se,
                                  "sigma_units": delta / se})
        # SC3.b slope: regress mean_ratio on log(n) for n > 0
        pts = [(math.log(n), s["mean_ratio"]) for n, s in rows if n > 0 and s["count"] > 0]
        if len(pts) >= 3:
            xs = [p[0] for p in pts]
            ys = [p[1] for p in pts]
            xbar = sum(xs) / len(xs)
            ybar = sum(ys) / len(ys)
            num = sum((x - xbar) * (y - ybar) for x, y in pts)
            den = sum((x - xbar) ** 2 for x in xs)
            slope = num / den if den > 0 else float("nan")
            slopes[f"L={L}_level={level}"] = slope
    return {"triggered": triggered, "slopes": slopes,
            "verdict": ("SC3.a triggers" if triggered else "Gaussian fixed point survives")}


def write_plot(groups, out_pdf):
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print(f"matplotlib not available; skipping plot {out_pdf}")
        return
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))
    by_L_level = defaultdict(list)
    for (L, n, level), stats in groups.items():
        by_L_level[(L, level)].append((n, stats))
    for (L, level), rows in sorted(by_L_level.items()):
        rows.sort(key=lambda r: r[0])
        ns = [r[0] for r in rows]
        cls = [r[1]["mean_ratio"] for r in rows]
        cls_se = [r[1]["se_ratio"] for r in rows]
        kts = [r[1]["mean_energy_fine"] for r in rows]
        kts_se = [r[1]["se_energy_fine"] for r in rows]
        ax1.errorbar(ns, cls, yerr=cls_se, marker="o",
                     label=f"L={L} level={level}")
        ax2.errorbar(ns, kts, yerr=kts_se, marker="o",
                     label=f"L={L} level={level}")
    ax1.axhline(1.0, color="k", ls="--", lw=0.7, alpha=0.5, label="Gaussian")
    for ax, name in ((ax1, "flux_energy_ratio"), (ax2, "flux_energy_fine")):
        ax.set_xscale("symlog", linthresh=1e-5)
        ax.set_xlabel("manifestation density n = N/V")
        ax.set_ylabel(name)
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.3)
    fig.suptitle("Manifestation-induced scale flow of the FTD native response tuple")
    fig.tight_layout()
    fig.savefig(out_pdf)
    print(f"wrote {out_pdf}")


def main():
    p = argparse.ArgumentParser()
    p.add_argument("input_json", help="raw measurement JSON (Tier-1 or Tier-2)")
    p.add_argument("--sigma", type=float, default=5.0,
                   help="sigma threshold for SC3.a (2 for Tier-1 gate, 5 for Tier-2)")
    p.add_argument("--out-report", default=None)
    p.add_argument("--out-plot", default=None)
    args = p.parse_args()

    with open(args.input_json) as f:
        rows = json.load(f)
    groups = rollup(rows)
    report = {
        "input": args.input_json,
        "sigma_threshold": args.sigma,
        "groups": {f"L={L}_n={n}_level={l}": v
                   for (L, n, l), v in groups.items()},
        "sc3": sc3_test(groups, args.sigma),
    }
    base = os.path.splitext(args.input_json)[0]
    out_report = args.out_report or f"{base}_report.json"
    out_plot = args.out_plot or f"{base}_plot.pdf"
    with open(out_report, "w") as f:
        json.dump(report, f, indent=2)
    print(f"wrote {out_report}")
    write_plot(groups, out_plot)
    print(f"SC3 verdict: {report['sc3']['verdict']}")
    if report["sc3"]["triggered"]:
        print(f"  {len(report['sc3']['triggered'])} triggers")


if __name__ == "__main__":
    main()
