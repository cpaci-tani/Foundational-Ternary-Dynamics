"""Analyze manifestation-flow JSON output and apply the SC3 test.

Input JSON schema (list of rows):
    {"L": int, "n": float, "level": int, "seed": int,
     "C_L": float, "C_L_R2": float, "K_T": float, "K_T_R2": float,
     "gauss_violation_pre": float, "gauss_violation_post": float,
     "wall_seconds": float, "status": "ok" | "invalid_fit" | "settle_crash"}

Output:
    <basename>_report.json  -- rollup + SC3 verdict
    <basename>_plot.pdf     -- C_L(n, level) and K_T(n, level) with error bars
"""
import argparse
import json
import math
import os
from collections import defaultdict


def rollup(rows, min_r2=0.95):
    """Group by (L, n, level); return mean, stderr, count."""
    groups = defaultdict(list)
    for r in rows:
        if r.get("status", "ok") != "ok":
            continue
        if r.get("C_L_R2", 0.0) < min_r2:
            continue
        groups[(r["L"], r["n"], r["level"])].append(r)
    out = {}
    for key, gs in groups.items():
        cls = [g["C_L"] for g in gs]
        kts = [g["K_T"] for g in gs]
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
        out[key] = {"count": n, "mean_C_L": mean_cl, "se_C_L": se_cl,
                    "mean_K_T": mean_kt, "se_K_T": se_kt}
    return out


def sc3_test(groups, sigma_threshold):
    """Apply SC3. Returns dict with verdict and details.

    SC3.a: |mean(n) - mean(0)| > sigma_threshold * se at any (L, n, level)
    SC3.b: linear regression dC_L/dlog(n) at each (L, level). Reports slope.
    """
    by_L_level = defaultdict(list)
    for (L, n, level), stats in groups.items():
        by_L_level[(L, level)].append((n, stats))

    triggered = []
    slopes = {}
    for (L, level), rows in by_L_level.items():
        rows.sort(key=lambda r: r[0])
        baseline = next((s for n, s in rows if n == 0.0), None)
        if baseline is None:
            continue
        for n, s in rows:
            if n == 0.0:
                continue
            delta_cl = abs(s["mean_C_L"] - baseline["mean_C_L"])
            se_val = s["se_C_L"]
            se = 0.0 if math.isnan(se_val) else se_val
            if se > 0 and delta_cl > sigma_threshold * se:
                triggered.append({"L": L, "n": n, "level": level,
                                  "delta_C_L": delta_cl, "se": se,
                                  "sigma_units": delta_cl / se})
        # SC3.b slope: regress mean_C_L on log(n) for n > 0
        pts = [(math.log(n), s["mean_C_L"]) for n, s in rows if n > 0 and s["count"] > 0]
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
        cls = [r[1]["mean_C_L"] for r in rows]
        cls_se = [r[1]["se_C_L"] for r in rows]
        kts = [r[1]["mean_K_T"] for r in rows]
        kts_se = [r[1]["se_K_T"] for r in rows]
        ax1.errorbar(ns, cls, yerr=cls_se, marker="o",
                     label=f"L={L} level={level}")
        ax2.errorbar(ns, kts, yerr=kts_se, marker="o",
                     label=f"L={L} level={level}")
    for ax, name in ((ax1, "C_L^FTD"), (ax2, "K_T^FTD")):
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
