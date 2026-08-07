#!/usr/bin/env python3
"""
FTD-0277 v1 adjudicator: collective-coordinate / genesis-counting model.

This script encodes the frozen gates in
docs/theory/10_eft_program/preregistrations/sm_constants_mass_flavour/PREREG_GENESIS_COUNTING_v1.md.
It evaluates the already-declared analytic firing-rank model against the
FTD-0261/0269 run-of-record targets. It performs no parameter fitting beyond
the declared broken-power diagnostic.
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
import sys
from typing import Iterable

import numpy as np

SCRIPT_DIR = Path(__file__).resolve().parent
ROOT = SCRIPT_DIR.parents[1]
sys.path.insert(0, str(SCRIPT_DIR))

from genesis_counting_model import CountingModel  # noqa: E402


A_GRID = [10, 12, 14, 16, 20, 25, 30, 40, 50, 70, 90]
FTD0261_TARGET = {
    10: 4.0,
    12: 8.4,
    14: 16.4,
    16: 21.6,
    20: 27.4,
    25: 32.6,
    30: 45.0,
    40: 91.8,
    50: 130.2,
    70: 260.2,
    90: 383.3,
}

ENGINE_A14_SHELL_PROFILE = {
    "center": 0.059701,
    "SC": 0.358209,
    "FCC": 0.134328,
    "BCC": 0.373134,
    "SC2": 0.074627,
    "outer": 0.0,
}

KNEE_BAND = (14.0, 18.0)
P_LO_BAND = (3.3, 4.1)
P_HI_BAND = (1.6, 2.1)
LOG10_RMS_MAX = 0.15
A10_COUNT_BAND = (3.0, 7.0)
SHELL_L1_MAX = 0.30
DRAIN_EXP_BAND = (-1.20, -0.70)


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
        pred = X @ coef
        rmse = math.sqrt(float(np.mean((ys - pred) ** 2)))
        candidate = {
            "knee": float(knee),
            "p_lo": float(coef[1]),
            "p_hi": float(coef[2]),
            "rmse_ln": rmse,
        }
        if best is None or rmse < best["rmse_ln"]:
            best = candidate
    assert best is not None
    return best


def shell_name(dx: int, dy: int, dz: int) -> str:
    r2 = dx * dx + dy * dy + dz * dz
    if r2 == 0:
        return "center"
    if r2 == 1:
        return "SC"
    if r2 == 2:
        return "FCC"
    if r2 == 3:
        return "BCC"
    if r2 == 4:
        return "SC2"
    return "outer"


def shell_profile(sites: list[tuple[int, int, int]]) -> dict[str, float]:
    counts = {key: 0 for key in ENGINE_A14_SHELL_PROFILE}
    for dx, dy, dz in sites:
        counts[shell_name(dx, dy, dz)] += 1
    total = max(sum(counts.values()), 1)
    return {key: counts[key] / total for key in counts}


def l1_distance(a: dict[str, float], b: dict[str, float]) -> float:
    keys = set(a) | set(b)
    return sum(abs(a.get(key, 0.0) - b.get(key, 0.0)) for key in keys)


def evaluate_default() -> dict[str, object]:
    model = CountingModel(drain=0.5, gamma=0.02)
    values = {}
    gated = {}
    for amp in A_GRID:
        n_model, n_gated = model.count(amp)
        values[amp] = float(n_model)
        gated[amp] = int(n_gated)

    curve_rms = math.sqrt(
        sum((math.log10(values[a] / FTD0261_TARGET[a])) ** 2 for a in A_GRID)
        / len(A_GRID)
    )
    broken = fit_broken_power(values)

    gated_a14 = model.gated_region(14)
    model_profile = shell_profile(gated_a14)
    shell_l1 = l1_distance(model_profile, ENGINE_A14_SHELL_PROFILE)

    return {
        "values": values,
        "gated": gated,
        "curve_log10_rms": curve_rms,
        "broken_power": broken,
        "a10_count": values[10],
        "a10_gated": gated[10],
        "model_a14_shell_profile": model_profile,
        "engine_a14_shell_profile": ENGINE_A14_SHELL_PROFILE,
        "shell_l1": shell_l1,
    }


def evaluate_drain() -> dict[str, object]:
    drains = [0.125, 0.25, 0.375, 0.5, 0.625, 0.75]
    rows = []
    for drain in drains:
        model = CountingModel(drain=drain, gamma=0.02)
        n_model, _ = model.count(12)
        rows.append({"drain": drain, "N": float(n_model), "k_eff": float(n_model / 12**2)})
    exponent = fit_power([row["drain"] for row in rows], [row["k_eff"] for row in rows])
    return {"rows": rows, "exponent": exponent}


def evaluate_gamma() -> dict[str, object]:
    gammas = [0.0, 0.10]
    rows = []
    for gamma in gammas:
        model = CountingModel(drain=0.5, gamma=gamma)
        values = {}
        for amp in A_GRID:
            n_model, _ = model.count(amp)
            values[amp] = float(n_model)
        broken = fit_broken_power(values)
        high_ratio = float(np.mean([values[a] / FTD0261_TARGET[a] for a in A_GRID if a >= 30]))
        rows.append(
            {
                "gamma": gamma,
                "knee": broken["knee"],
                "p_hi": broken["p_hi"],
                "high_ratio": high_ratio,
            }
        )
    gamma_pass = rows[-1]["knee"] >= rows[0]["knee"] and rows[-1]["high_ratio"] < rows[0]["high_ratio"]
    return {"rows": rows, "direction_pass": gamma_pass}


def adjudicate(default: dict[str, object], drain: dict[str, object], gamma: dict[str, object]) -> dict[str, object]:
    broken = default["broken_power"]
    assert isinstance(broken, dict)

    checks = {
        "shape_knee": KNEE_BAND[0] <= broken["knee"] <= KNEE_BAND[1],
        "shape_p_lo": P_LO_BAND[0] <= broken["p_lo"] <= P_LO_BAND[1],
        "shape_p_hi": P_HI_BAND[0] <= broken["p_hi"] <= P_HI_BAND[1],
        "curve_rms": default["curve_log10_rms"] <= LOG10_RMS_MAX,
        "a10_count": A10_COUNT_BAND[0] <= default["a10_count"] <= A10_COUNT_BAND[1],
        "shell_geometry": default["shell_l1"] <= SHELL_L1_MAX,
        "drain_exponent": DRAIN_EXP_BAND[0] <= drain["exponent"] <= DRAIN_EXP_BAND[1],
        "gamma_direction": bool(gamma["direction_pass"]),
    }
    primary = [
        "shape_knee",
        "shape_p_lo",
        "shape_p_hi",
        "curve_rms",
        "a10_count",
        "shell_geometry",
    ]
    all_pass = all(checks.values())
    primary_failures = [key for key in primary if not checks[key]]
    if all_pass:
        verdict = "CONDITIONAL_DERIVED_GIVEN_IMPOSED_INPUT"
    elif primary_failures:
        verdict = "COUNTING_MODEL_V1_CLOSED_NEGATIVE"
    else:
        verdict = "PARTIAL_BOUNDARY"
    return {"checks": checks, "primary_failures": primary_failures, "verdict": verdict}


def format_report(result: dict[str, object]) -> str:
    default = result["default"]
    drain = result["drain"]
    gamma = result["gamma"]
    adjudication = result["adjudication"]
    broken = default["broken_power"]

    lines = []
    lines.append("FTD-0277 GENESIS COUNTING v1 ANALYSIS")
    lines.append("")
    lines.append(f"VERDICT: {adjudication['verdict']}")
    lines.append("")
    lines.append("Default register: drain=0.5 gamma=0.02 G_C=sqrt(alpha) charge_coupling=1")
    lines.append(f"Broken power: knee={broken['knee']:.2f} p_lo={broken['p_lo']:.3f} p_hi={broken['p_hi']:.3f} rmse_ln={broken['rmse_ln']:.3f}")
    lines.append(f"Curve log10 RMS vs FTD-0261 targets: {default['curve_log10_rms']:.3f} (gate <= {LOG10_RMS_MAX})")
    lines.append(f"A=10 count: N_model={default['a10_count']:.3f}, gated={default['a10_gated']} (count gate {A10_COUNT_BAND[0]}..{A10_COUNT_BAND[1]})")
    lines.append(f"A=14 shell L1 vs engine run-of-record: {default['shell_l1']:.3f} (gate <= {SHELL_L1_MAX})")
    lines.append("")
    lines.append("Sweep:")
    lines.append("A,N_model,gated,target,ratio")
    for amp in A_GRID:
        values = default["values"]
        gated = default["gated"]
        lines.append(f"{amp},{values[amp]:.6f},{gated[amp]},{FTD0261_TARGET[amp]:.6f},{values[amp] / FTD0261_TARGET[amp]:.6f}")
    lines.append("")
    lines.append(f"Drain exponent at A=12: {drain['exponent']:.3f} (gate {DRAIN_EXP_BAND[0]}..{DRAIN_EXP_BAND[1]})")
    lines.append("drain,N_model,k_eff")
    for row in drain["rows"]:
        lines.append(f"{row['drain']:.3f},{row['N']:.6f},{row['k_eff']:.6f}")
    lines.append("")
    lines.append(f"Gamma direction pass: {gamma['direction_pass']}")
    lines.append("gamma,knee,p_hi,high_A_ratio")
    for row in gamma["rows"]:
        lines.append(f"{row['gamma']:.3f},{row['knee']:.2f},{row['p_hi']:.6f},{row['high_ratio']:.6f}")
    lines.append("")
    lines.append("Checks:")
    for key, value in adjudication["checks"].items():
        lines.append(f"{key}: {'PASS' if value else 'FAIL'}")
    lines.append("")
    lines.append("Machine JSON:")
    lines.append(json.dumps(result, indent=2, sort_keys=True))
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", type=Path, default=None, help="Optional path for the text report.")
    parser.add_argument("--json-out", type=Path, default=None, help="Optional path for machine-readable JSON.")
    args = parser.parse_args()

    default = evaluate_default()
    drain = evaluate_drain()
    gamma = evaluate_gamma()
    adjudication = adjudicate(default, drain, gamma)
    result = {"default": default, "drain": drain, "gamma": gamma, "adjudication": adjudication}

    report = format_report(result)
    print(report, end="")
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(report, encoding="utf-8")
    if args.json_out:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        args.json_out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
