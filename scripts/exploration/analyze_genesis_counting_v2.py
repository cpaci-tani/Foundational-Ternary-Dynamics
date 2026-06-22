#!/usr/bin/env python3
"""
FTD-0309 adjudicator: the v2 collective-coordinate (O_h shell) genesis-counting model.

Runs scripts/exploration/genesis_counting_model_v2.py against the SAME frozen gates as
the FTD-0277 v1 adjudicator (the FTD-0261/0269 run-of-record targets) -- the gate bands
are INHERITED UNCHANGED from PREREG_GENESIS_COUNTING_v1.md, so v2 faces exactly the test
v1 faced (no goalpost-moving). It evaluates BOTH boost modes (monopole, local) to record
the knife-edge that is the FTD-0309 obstruction.

HONESTY NOTE: this is NOT a blind pre-registration. The v2 model was developed and the
angular (dipole) obstruction was diagnosed before this lock; the gates are inherited
(not new), and this script provides a reproducible adjudication + diagnosis, not a blind
verdict. The expected, prior-stated outcome is a BOUNDARY (the scalar reduction passes
the derivable-structure pieces and fails the angular/dipole-dependent gates).
"""

from __future__ import annotations

import argparse
import importlib.util
import json
import math
from pathlib import Path
import sys
from typing import Iterable

import numpy as np

SCRIPT_DIR = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location(
    "gcm2", str(SCRIPT_DIR / "genesis_counting_model_v2.py"))
gcm2 = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gcm2)
ShellBurst = gcm2.ShellBurst

# ---- frozen targets + gates, INHERITED UNCHANGED from FTD-0277 --------------
A_GRID = [10, 12, 14, 16, 20, 25, 30, 40, 50, 70, 90]
FTD0261_TARGET = {10: 4.0, 12: 8.4, 14: 16.4, 16: 21.6, 20: 27.4, 25: 32.6,
                  30: 45.0, 40: 91.8, 50: 130.2, 70: 260.2, 90: 383.3}
ENGINE_A14_SHELL_PROFILE = {"center": 0.059701, "SC": 0.358209, "FCC": 0.134328,
                            "BCC": 0.373134, "SC2": 0.074627, "outer": 0.0}
KNEE_BAND = (14.0, 18.0)
P_LO_BAND = (3.3, 4.1)
P_HI_BAND = (1.6, 2.1)
LOG10_RMS_MAX = 0.15
A10_COUNT_BAND = (3.0, 7.0)
SHELL_L1_MAX = 0.30
DRAIN_EXP_BAND = (-1.20, -0.70)

BURST_KW = dict(max_ticks=120, quiet=4)


def fit_power(x: Iterable[float], y: Iterable[float]) -> float:
    xs = np.log(np.asarray(list(x), dtype=float))
    ys = np.log(np.asarray(list(y), dtype=float))
    A = np.vstack([np.ones_like(xs), xs]).T
    coef, *_ = np.linalg.lstsq(A, ys, rcond=None)
    return float(coef[1])


def fit_broken_power(values: dict[int, float]) -> dict[str, float]:
    xs = np.asarray(A_GRID, dtype=float)
    ys = np.log(np.asarray([values[a] for a in A_GRID], dtype=float))
    best = None
    for knee in np.linspace(10.0, 30.0, 81):
        lx = np.log(xs / knee)
        X = np.vstack([np.ones_like(xs), np.minimum(lx, 0.0), np.maximum(lx, 0.0)]).T
        coef, *_ = np.linalg.lstsq(X, ys, rcond=None)
        rmse = math.sqrt(float(np.mean((ys - X @ coef) ** 2)))
        cand = {"knee": float(knee), "p_lo": float(coef[1]),
                "p_hi": float(coef[2]), "rmse_ln": rmse}
        if best is None or rmse < best["rmse_ln"]:
            best = cand
    return best


def shell_name(r2: int) -> str:
    return {0: "center", 1: "SC", 2: "FCC", 3: "BCC", 4: "SC2"}.get(r2, "outer")


def model_shell_profile(model: "ShellBurst", A: int) -> dict[str, float]:
    n = model.burst(A, **BURST_KW)
    counts = {k: 0.0 for k in ENGINE_A14_SHELL_PROFILE}
    for i, r2 in enumerate(model.r2s):
        counts[shell_name(r2)] += float(n[i])
    total = max(sum(counts.values()), 1e-9)
    return {k: counts[k] / total for k in counts}


def l1(a: dict[str, float], b: dict[str, float]) -> float:
    return sum(abs(a.get(k, 0.0) - b.get(k, 0.0)) for k in set(a) | set(b))


def evaluate(boost_mode: str) -> dict[str, object]:
    model = ShellBurst(drain=0.5, gamma=0.02, boost_mode=boost_mode)
    values = {a: float(model.count(a, **BURST_KW)[0]) for a in A_GRID}
    curve_rms = math.sqrt(sum((math.log10(values[a] / FTD0261_TARGET[a])) ** 2
                              for a in A_GRID) / len(A_GRID))
    broken = fit_broken_power(values)
    prof = model_shell_profile(model, 14)
    shell_l1 = l1(prof, ENGINE_A14_SHELL_PROFILE)

    # drain exponent
    drain_rows = []
    for d in [0.125, 0.25, 0.375, 0.5, 0.625, 0.75]:
        mm = ShellBurst(drain=d, gamma=0.02, boost_mode=boost_mode)
        drain_rows.append((d, mm.count(12, **BURST_KW)[0] / 12 ** 2))
    drain_exp = fit_power([d for d, _ in drain_rows], [k for _, k in drain_rows])

    checks = {
        "shape_knee": KNEE_BAND[0] <= broken["knee"] <= KNEE_BAND[1],
        "shape_p_lo": P_LO_BAND[0] <= broken["p_lo"] <= P_LO_BAND[1],
        "shape_p_hi": P_HI_BAND[0] <= broken["p_hi"] <= P_HI_BAND[1],
        "curve_rms": curve_rms <= LOG10_RMS_MAX,
        "a10_count": A10_COUNT_BAND[0] <= values[10] <= A10_COUNT_BAND[1],
        "shell_geometry": shell_l1 <= SHELL_L1_MAX,
        "drain_exponent": DRAIN_EXP_BAND[0] <= drain_exp <= DRAIN_EXP_BAND[1],
    }
    primary = ["shape_knee", "shape_p_lo", "shape_p_hi", "curve_rms",
               "a10_count", "shell_geometry"]
    primary_fail = [k for k in primary if not checks[k]]
    return {"boost_mode": boost_mode, "values": values, "curve_rms": curve_rms,
            "broken": broken, "a10": values[10], "shell_l1": shell_l1,
            "model_a14_profile": prof, "drain_exp": drain_exp,
            "checks": checks, "primary_failures": primary_fail}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", type=Path, default=None)
    ap.add_argument("--json-out", type=Path, default=None)
    args = ap.parse_args()

    results = {mode: evaluate(mode) for mode in ("monopole", "local")}

    lines = ["FTD-0309 GENESIS COUNTING v2 ADJUDICATION (vs inherited FTD-0277 frozen gates)", ""]
    for mode, r in results.items():
        b = r["broken"]
        lines.append(f"== boost_mode = {mode} ==")
        lines.append(f"  broken power: knee={b['knee']:.2f} p_lo={b['p_lo']:.3f} "
                     f"p_hi={b['p_hi']:.3f} rmse_ln={b['rmse_ln']:.3f}")
        lines.append(f"  curve log10 RMS vs FTD-0261: {r['curve_rms']:.3f} (gate <= {LOG10_RMS_MAX})")
        lines.append(f"  A=10 count: {r['a10']:.2f} (gate {A10_COUNT_BAND})")
        lines.append(f"  A=14 shell L1 vs engine: {r['shell_l1']:.3f} (gate <= {SHELL_L1_MAX})")
        lines.append(f"  A=14 model profile: " +
                     " ".join(f"{k}:{v:.3f}" for k, v in r["model_a14_profile"].items()))
        lines.append(f"  drain exponent: {r['drain_exp']:.3f} (gate {DRAIN_EXP_BAND})")
        lines.append("  sweep A,N_model,target,ratio:")
        for a in A_GRID:
            v = r["values"][a]
            lines.append(f"    {a},{v:.3f},{FTD0261_TARGET[a]:.3f},{v/FTD0261_TARGET[a]:.3f}")
        lines.append("  checks: " +
                     ", ".join(f"{k}={'PASS' if v else 'FAIL'}" for k, v in r["checks"].items()))
        lines.append(f"  primary failures: {r['primary_failures']}")
        lines.append("")

    # Verdict: the obstruction is structural iff BOTH scalar modes fail the
    # angular-dependent primary gates (geometry and/or curve), while the
    # derivable-structure pieces (a super-knee exponent in band) can pass.
    geom_or_curve_fail = all(
        (not results[m]["checks"]["shell_geometry"]) or (not results[m]["checks"]["curve_rms"])
        for m in results)
    verdict = ("V2_SCALAR_REDUCTION_BOUNDARY_DIPOLE_OBSTRUCTION"
               if geom_or_curve_fail else "V2_UNEXPECTED_PASS_REVIEW")
    lines.append(f"VERDICT: {verdict}")
    lines.append("")
    lines.append("Machine JSON:")
    lines.append(json.dumps(results, indent=2, sort_keys=True, default=float))
    report = "\n".join(lines) + "\n"
    print(report, end="")
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(report, encoding="utf-8")
    if args.json_out:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        args.json_out.write_text(json.dumps(
            {"results": results, "verdict": verdict}, indent=2, sort_keys=True, default=float) + "\n",
            encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
