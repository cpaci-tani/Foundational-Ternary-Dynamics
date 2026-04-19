#!/usr/bin/env python3
"""
measure_beta_function.py — Phase 2C of the EFT Recovery Program.

Pipeline:

    1. Run engine/build/Release/benchmark_beta_function.exe (produces CSV
       of V(r) and α_fit values at three lattice sizes L ∈ {16, 32, 64}).
    2. For each L, fit three candidate forms:
         (a) pure Coulomb    V = -α/r + C                   (slope fit)
         (b) Yukawa           V = -α·exp(-r/λ)/r + C         (nonlinear)
         (c) asymptotic α_r  extracted from large-r plateau.
    3. Compute β(g) from the scale-dependent α_eff values.
    4. Compare the measured β against QED one-loop β_QED(g) = g³/(12π²).
    5. Write report + CSV + matplotlib plot to
       scripts/benchmarks/results/eft_beta/.

Epistemic honesty
-----------------
The engine's V(r) is not pure 1/r — it shows Yukawa-like screening. The
slope fit of V vs 1/r therefore does NOT give the continuum α. We quote
all three extraction methods and let the reader judge which tracks the
continuum EFT coupling most faithfully.

Usage
-----
    python scripts/benchmarks/measure_beta_function.py
    python scripts/benchmarks/measure_beta_function.py --quick
    python scripts/benchmarks/measure_beta_function.py --csv path/to/data.csv

With --csv, reads a pre-computed CSV and skips the engine invocation.
"""

from __future__ import annotations

import argparse
import csv
import json
import math
import pathlib
import subprocess
import sys
from dataclasses import dataclass, asdict, field
from typing import Dict, List, Optional, Tuple

# Import FTD canonical constants (α, G_C, etc.).
SCRIPT_DIR = pathlib.Path(__file__).resolve().parent
PROJ_ROOT = SCRIPT_DIR.parent.parent
sys.path.insert(0, str(PROJ_ROOT / "scripts"))

try:
    from constants import ALPHA_PRECISION as ALPHA_REF
except Exception:
    # Fallback so the script runs without FTD scripts package.
    ALPHA_REF = 1.0 / 137.035999177

PI = math.pi


# ─────────────────────────────────────────────────────────────────────────
# Data model
# ─────────────────────────────────────────────────────────────────────────

@dataclass
class VofRPoint:
    L: int
    r: int
    V: float
    alpha_r: float  # = -V * r


@dataclass
class ScaleMeasurement:
    L: int
    ticks: int
    points: List[VofRPoint] = field(default_factory=list)
    alpha_slope: Optional[float] = None       # slope fit V vs 1/r
    r2_slope: Optional[float] = None
    alpha_yukawa: Optional[float] = None      # V = -α exp(-r/λ)/r fit
    lambda_yukawa: Optional[float] = None
    r2_yukawa: Optional[float] = None
    alpha_asymptotic: Optional[float] = None  # mean α_r over large-r plateau
    asymptotic_r_min: Optional[int] = None
    asymptotic_r_max: Optional[int] = None

    def coupling_summary(self) -> Dict[str, Optional[float]]:
        """Best available α_eff for β(g) extraction, per method."""
        return {
            "slope": self.alpha_slope,
            "yukawa": self.alpha_yukawa,
            "asymptotic": self.alpha_asymptotic,
        }


# ─────────────────────────────────────────────────────────────────────────
# CSV loading
# ─────────────────────────────────────────────────────────────────────────

def load_csv(path: pathlib.Path) -> Dict[int, ScaleMeasurement]:
    """Parse the CSV emitted by benchmark_beta_function.exe."""
    scales: Dict[int, ScaleMeasurement] = {}
    with path.open("r", newline="") as f:
        reader = csv.reader(f)
        # Header: method, L, ticks, r, V_or_alpha_fit, alpha_r_or_r2, flag
        header = next(reader, None)
        for row in reader:
            if not row or len(row) < 6:
                continue
            try:
                method = row[0]
                L = int(row[1])
                ticks = int(row[2])
                r_or_fit = row[3]
                val2 = float(row[4])
                val3 = float(row[5])
            except ValueError:
                continue
            if L not in scales:
                scales[L] = ScaleMeasurement(L=L, ticks=ticks)
            sm = scales[L]
            if r_or_fit == "fit":
                if row[6].strip() == "valid":
                    sm.alpha_slope = val2
                    sm.r2_slope = val3
            else:
                r = int(r_or_fit)
                V = val2
                alpha_r = val3
                sm.points.append(VofRPoint(L=L, r=r, V=V, alpha_r=alpha_r))
    return scales


# ─────────────────────────────────────────────────────────────────────────
# Fits
# ─────────────────────────────────────────────────────────────────────────

def fit_yukawa(points: List[VofRPoint]) -> Tuple[Optional[float], Optional[float], Optional[float]]:
    """
    Fit V(r) = -α · exp(-r/λ) / r  + C  via a crude 2-parameter log-linear
    transformation: ln(-V · r) = ln α - r/λ  when C ≈ 0.

    Returns (α, λ, R²) or (None, None, None) if not fittable.
    """
    xs = []
    ys = []
    for p in points:
        # Exclude non-binding (V ≥ 0) or near-zero V where log is unstable.
        if p.V >= 0:
            continue
        val = -p.V * p.r
        if val <= 0:
            continue
        xs.append(float(p.r))
        ys.append(math.log(val))
    if len(xs) < 3:
        return None, None, None

    n = len(xs)
    sx = sum(xs); sy = sum(ys)
    sxx = sum(x * x for x in xs); sxy = sum(x * y for x, y in zip(xs, ys))
    denom = n * sxx - sx * sx
    if abs(denom) < 1e-30:
        return None, None, None
    slope = (n * sxy - sx * sy) / denom
    intercept = (sy - slope * sx) / n

    if slope >= 0:
        # No Yukawa decay → screening length infinite; fall back to slope fit.
        return None, None, None

    lam = -1.0 / slope
    alpha_y = math.exp(intercept)

    # R²
    ybar = sy / n
    ss_tot = sum((y - ybar) ** 2 for y in ys)
    ss_res = sum((y - (intercept + slope * x)) ** 2 for x, y in zip(xs, ys))
    r2 = 1.0 - ss_res / ss_tot if ss_tot > 0 else 0.0

    return alpha_y, lam, r2


def fit_asymptotic(points: List[VofRPoint], fraction: float = 0.5) -> Tuple[Optional[float], Optional[int], Optional[int]]:
    """
    Average α_r = -V·r over the outer `fraction` of the data (default upper
    half of r-range) as a rough proxy for the IR coupling. Not statistically
    ideal but honest: a continuum α should show α_r ≈ const at large r.
    """
    if len(points) < 3:
        return None, None, None
    pts = sorted(points, key=lambda p: p.r)
    n_tail = max(2, int(len(pts) * fraction))
    tail = pts[-n_tail:]
    if not tail:
        return None, None, None
    vals = [p.alpha_r for p in tail]
    mean = sum(vals) / len(vals)
    return mean, tail[0].r, tail[-1].r


# ─────────────────────────────────────────────────────────────────────────
# β(g) computation
# ─────────────────────────────────────────────────────────────────────────

def qed_one_loop_beta(g: float) -> float:
    """
    β_QED(g) = g³ / (12π²) — one-loop continuum QED β-function, dimensionless
    coupling (g = √α in natural units matching FTD's G_C = √α convention).
    """
    return g * g * g / (12.0 * PI * PI)


def compute_beta_from_scales(measurements: Dict[int, ScaleMeasurement], method: str) -> List[Tuple[int, int, float, float, float]]:
    """
    Given per-scale α_eff measurements, compute β(g) at each consecutive
    scale pair. Returns list of (L_fine, L_coarse, g_fine, beta, beta_QED).

    β(g) ≡ [g(scale·2) − g(scale)] / ln 2   where scale ~ L^{-1}
    We go from fine to coarser; g = √α.
    """
    out: List[Tuple[int, int, float, float, float]] = []
    sizes = sorted(measurements.keys(), reverse=True)  # [64, 32, 16]
    for i in range(len(sizes) - 1):
        Lf, Lc = sizes[i], sizes[i + 1]
        if Lc * 2 != Lf:
            continue
        a_fine = getattr(measurements[Lf], f"alpha_{method}")
        a_coarse = getattr(measurements[Lc], f"alpha_{method}")
        if a_fine is None or a_coarse is None:
            continue
        if a_fine <= 0 or a_coarse <= 0:
            continue  # g = √α only defined for α > 0
        g_fine = math.sqrt(a_fine)
        g_coarse = math.sqrt(a_coarse)
        # β = Δg / Δln(scale); scale halves, so Δln = ln(1/2) = -ln2. We
        # convert to the physicist convention β = dg/d(lnμ) with μ = 1/a,
        # so blocking by factor 2 DECREASES μ and the sign of β is reversed.
        # Report d g / d ln μ (positive for asymptotic freedom, negative for
        # screening).
        beta_measured = (g_fine - g_coarse) / math.log(2.0)
        beta_ref = qed_one_loop_beta(g_fine)
        out.append((Lf, Lc, g_fine, beta_measured, beta_ref))
    return out


# ─────────────────────────────────────────────────────────────────────────
# Reporting
# ─────────────────────────────────────────────────────────────────────────

def format_float(v: Optional[float], fmt: str = ".6f") -> str:
    if v is None or not math.isfinite(v):
        return "n/a"
    return format(v, fmt)


def write_report(measurements: Dict[int, ScaleMeasurement],
                 beta_table: Dict[str, List[Tuple[int, int, float, float, float]]],
                 out_dir: pathlib.Path) -> pathlib.Path:
    report_path = out_dir / "beta_report.md"
    lines: List[str] = []
    lines.append("# EFT Phase 2C — Measured β(g) Report")
    lines.append("")
    lines.append(f"**Reference α** (from `scripts/constants.py`): {ALPHA_REF:.12f}")
    lines.append(f"**Reference g = √α**: {math.sqrt(ALPHA_REF):.8f}")
    lines.append("")
    lines.append("## 1. α_eff by Scale and Extraction Method")
    lines.append("")
    lines.append("| L | method | α_eff | R² / range | vs α_ref |")
    lines.append("|---|---|---|---|---|")
    for L in sorted(measurements.keys(), reverse=True):
        sm = measurements[L]
        for label, val, aux in (
            ("slope fit V vs 1/r", sm.alpha_slope, sm.r2_slope),
            ("Yukawa V = -α·exp(-r/λ)/r", sm.alpha_yukawa, sm.r2_yukawa),
            ("asymptotic α_r mean", sm.alpha_asymptotic,
             (sm.asymptotic_r_min, sm.asymptotic_r_max)),
        ):
            if val is None:
                continue
            ratio = val / ALPHA_REF if ALPHA_REF != 0 else float("nan")
            aux_str = (format_float(aux, ".3f") if isinstance(aux, float)
                       else ("r=[%s,%s]" % aux if isinstance(aux, tuple) else "—"))
            lines.append(f"| {L} | {label} | {format_float(val)} | {aux_str} | {ratio:.1f}× α_ref |")
    lines.append("")

    lines.append("## 2. Measured β(g) vs QED One-Loop Prediction")
    lines.append("")
    lines.append("β(g) = [g(scale·2) − g(scale)] / ln 2 from blocked α_eff.")
    lines.append("β_QED(g) = g³/(12π²) (one-loop continuum, for comparison).")
    lines.append("")
    for method in ("slope", "yukawa", "asymptotic"):
        tbl = beta_table.get(method, [])
        lines.append(f"### Method: {method}")
        if not tbl:
            lines.append("_No valid β datapoints for this method._")
            lines.append("")
            continue
        lines.append("| L_fine | L_coarse | g(L_fine) | β_measured | β_QED(g) | ratio |")
        lines.append("|---|---|---|---|---|---|")
        for (Lf, Lc, g, bm, bq) in tbl:
            ratio = bm / bq if bq != 0 else float("nan")
            lines.append(f"| {Lf} | {Lc} | {g:.6f} | {bm:+.6e} | {bq:+.6e} | {ratio:+.3f} |")
        lines.append("")

    lines.append("## 3. Honest Interpretation")
    lines.append("")
    lines.append(
        "The engine's V(r) is not pure 1/r at these scales — `α_r = −V·r`\n"
        "is r-dependent, indicating a Yukawa-like screening with finite λ.\n"
        "Each extraction method has a different systematic:\n\n"
        "* **slope V vs 1/r** mixes the Coulomb part with the screening\n"
        "  envelope, giving an α that is 10-30× the continuum value.\n"
        "  Useful only for relative scale comparison.\n"
        "* **Yukawa fit** separates the Coulomb and screening-length\n"
        "  contributions when data extends past ~λ. Often more physical\n"
        "  but needs r-range > λ for convergence.\n"
        "* **asymptotic α_r** averages the plateau at large r. Closest to\n"
        "  the continuum α if the fit window reaches the screening-limited\n"
        "  regime — but on small lattices this regime may not exist.\n\n"
        "None of the three should be reported as \"the β function\" without\n"
        "caveats. The honest Phase 2 finding is:\n\n"
        "1. The engine exhibits screening at small to intermediate r.\n"
        "2. α_eff does vary with lattice size (evidence of RG flow).\n"
        "3. Matching to continuum QED β requires either a larger lattice\n"
        "   where asymptotic Coulomb emerges, or a refined measurement that\n"
        "   separates the screening length from the bare coupling."
    )

    report_path.write_text("\n".join(lines), encoding="utf-8")
    return report_path


def write_results_json(measurements: Dict[int, ScaleMeasurement],
                       beta_table: Dict[str, List[Tuple[int, int, float, float, float]]],
                       out_dir: pathlib.Path) -> pathlib.Path:
    payload: Dict[str, object] = {
        "alpha_ref": ALPHA_REF,
        "g_ref": math.sqrt(ALPHA_REF),
        "scales": {},
        "beta": {}
    }
    for L, sm in measurements.items():
        payload["scales"][str(L)] = {
            "ticks": sm.ticks,
            "alpha_slope": sm.alpha_slope,
            "r2_slope": sm.r2_slope,
            "alpha_yukawa": sm.alpha_yukawa,
            "lambda_yukawa": sm.lambda_yukawa,
            "r2_yukawa": sm.r2_yukawa,
            "alpha_asymptotic": sm.alpha_asymptotic,
            "asymptotic_range": [sm.asymptotic_r_min, sm.asymptotic_r_max],
            "n_points": len(sm.points),
        }
    for method, rows in beta_table.items():
        payload["beta"][method] = [
            {"L_fine": Lf, "L_coarse": Lc, "g_fine": g,
             "beta_measured": bm, "beta_qed_one_loop": bq}
            for (Lf, Lc, g, bm, bq) in rows
        ]

    path = out_dir / "beta_results.json"
    path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    return path


# ─────────────────────────────────────────────────────────────────────────
# Orchestration
# ─────────────────────────────────────────────────────────────────────────

def run_benchmark(exe_path: pathlib.Path, quick: bool, csv_out: pathlib.Path) -> None:
    """Invoke the native benchmark; stream stdout into csv_out."""
    args = [str(exe_path)]
    if quick:
        args.append("--quick")
    print(f"[measure_beta] running: {' '.join(args)}")
    with csv_out.open("w", encoding="utf-8") as f:
        proc = subprocess.run(args, stdout=f, stderr=sys.stderr, check=True)
    print(f"[measure_beta] wrote CSV: {csv_out}")


def locate_exe() -> pathlib.Path:
    candidates = [
        PROJ_ROOT / "engine" / "build" / "Release" / "benchmark_beta_function.exe",
        PROJ_ROOT / "engine" / "build" / "benchmark_beta_function",
        PROJ_ROOT / "engine" / "build" / "Release" / "benchmark_beta_function",
    ]
    for c in candidates:
        if c.exists():
            return c
    raise SystemExit("benchmark_beta_function binary not found in engine/build/. "
                     "Build with: cmake --build engine/build --target benchmark_beta_function --config Release")


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--quick", action="store_true",
                        help="skip L = 64 scale (fast development mode)")
    parser.add_argument("--csv", type=pathlib.Path,
                        help="read an existing CSV instead of invoking the engine")
    parser.add_argument("--out-dir", type=pathlib.Path,
                        default=PROJ_ROOT / "scripts" / "benchmarks" / "results" / "eft_beta",
                        help="output directory for CSV, JSON, and markdown report")
    args = parser.parse_args()

    args.out_dir.mkdir(parents=True, exist_ok=True)
    csv_path = args.csv if args.csv else args.out_dir / "beta_raw.csv"

    if not args.csv:
        exe = locate_exe()
        run_benchmark(exe, quick=args.quick, csv_out=csv_path)
    else:
        print(f"[measure_beta] using pre-existing CSV: {csv_path}")

    measurements = load_csv(csv_path)
    if not measurements:
        raise SystemExit("no measurements parsed from CSV")

    # Per-scale refinements: Yukawa + asymptotic extractions
    for L, sm in measurements.items():
        a_y, lam, r2_y = fit_yukawa(sm.points)
        sm.alpha_yukawa = a_y
        sm.lambda_yukawa = lam
        sm.r2_yukawa = r2_y
        a_asy, r_lo, r_hi = fit_asymptotic(sm.points, fraction=0.5)
        sm.alpha_asymptotic = a_asy
        sm.asymptotic_r_min = r_lo
        sm.asymptotic_r_max = r_hi

    beta_table = {
        "slope": compute_beta_from_scales(measurements, "slope"),
        "yukawa": compute_beta_from_scales(measurements, "yukawa"),
        "asymptotic": compute_beta_from_scales(measurements, "asymptotic"),
    }

    json_path = write_results_json(measurements, beta_table, args.out_dir)
    md_path = write_report(measurements, beta_table, args.out_dir)
    print(f"[measure_beta] wrote JSON: {json_path}")
    print(f"[measure_beta] wrote report: {md_path}")

    # Console summary — ASCII-only so Windows cp1252 console doesn't choke.
    print("\n=== Summary ===")
    print(f"Reference alpha = {ALPHA_REF:.12f}")
    for L in sorted(measurements.keys(), reverse=True):
        sm = measurements[L]
        print(f"L={L}: alpha_slope={format_float(sm.alpha_slope)}, "
              f"alpha_Yukawa={format_float(sm.alpha_yukawa)} "
              f"(lambda={format_float(sm.lambda_yukawa)}), "
              f"alpha_asymptotic={format_float(sm.alpha_asymptotic)}")
    for method in ("slope", "yukawa", "asymptotic"):
        rows = beta_table[method]
        if not rows:
            print(f"beta via {method}: no valid datapoints")
            continue
        for (Lf, Lc, g, bm, bq) in rows:
            ratio = bm / bq if bq else float("nan")
            print(f"beta via {method} at L {Lf}->{Lc}: g={g:.4f}  "
                  f"beta_meas={bm:+.4e}  beta_QED={bq:+.4e}  ratio={ratio:+.3f}")


if __name__ == "__main__":
    main()
