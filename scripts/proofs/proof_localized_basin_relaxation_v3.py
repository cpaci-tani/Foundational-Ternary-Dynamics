#!/usr/bin/env python3
"""Independent certificate for corrected FTD-0681 data.

The registered v3 verdict remains execution-invalid because its replication
gate was ill-conditioned.  This script separately certifies the corrected
target contract and recomputes the non-held-out core/shell observations.
"""

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
V2_CSV = ROOT / "engine/results/ftd_0679/ftd_0679_localized_basin_relaxation_ticks_v2.csv"
V3_JSON = ROOT / "engine/results/ftd_0681/ftd_0681_localized_basin_relaxation_v3.json"
V3_CSV = ROOT / "engine/results/ftd_0681/ftd_0681_localized_basin_relaxation_ticks_v3.csv"
PROTOCOL = "6E653BBD9D133F78ACE56E2E974EA322A275930C77E147478A8D4F31299D7E3A"


checks = 0


def require(condition: bool, message: str) -> None:
    global checks
    checks += 1
    if not condition:
        raise AssertionError(message)


def relative(left: float, right: float) -> float:
    return abs(left - right) / max(abs(left), abs(right), 1e-300)


def fit(records: list[dict[str, str]]) -> dict[str, float]:
    samples = [
        (float(row["tick"]), math.log(float(row["core_ratio"])))
        for row in records if 8 <= int(row["tick"]) <= 64
    ]
    require(len(samples) == 57, "locked fit sample count")
    n = float(len(samples))
    sx = sum(x for x, _ in samples)
    sy = sum(y for _, y in samples)
    sxx = sum(x * x for x, _ in samples)
    sxy = sum(x * y for x, y in samples)
    slope = (n * sxy - sx * sy) / (n * sxx - sx * sx)
    intercept = (sy - slope * sx) / n
    mean_y = sy / n
    rss_constant = sum((y - mean_y) ** 2 for _, y in samples)
    rss_linear = sum((y - intercept - slope * x) ** 2
                     for x, y in samples)
    delta_bic = (
        n * math.log(rss_constant / n) + math.log(n)
        - n * math.log(rss_linear / n) - 2.0 * math.log(n)
    )
    return {
        "gamma": -slope,
        "r2": 1.0 - rss_linear / rss_constant,
        "delta_bic": delta_bic,
    }


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def main() -> None:
    summary = json.loads(V3_JSON.read_text(encoding="utf-8"))
    with V3_CSV.open(newline="", encoding="utf-8") as stream:
        rows = list(csv.DictReader(stream))
    with V2_CSV.open(newline="", encoding="utf-8") as stream:
        prior = list(csv.DictReader(stream))

    require(summary["ftd_id"] == "FTD-0681", "identifier")
    require(summary["protocol_sha256"] == PROTOCOL, "protocol")
    require(summary["verdict"]
            == "LOCALIZED_BASIN_RELAXATION_V3_EXECUTION_INVALID",
            "registered verdict remains invalid")
    require(not summary["held_out"], "replication is not held out")
    require(len(rows) == 162, "two signs times 81 ticks")
    require(len(prior) == len(rows), "prior row count")

    by_sign: dict[int, list[dict[str, str]]] = {-1: [], 1: []}
    target_residual = 0.0
    partition_residual = 0.0
    for row in rows:
        require(row["ftd_id"] == "FTD-0681", "row identifier")
        require(row["protocol_sha256"] == PROTOCOL, "row protocol")
        require(row["valid"] == "1", "row validity")
        sign = int(row["sign"])
        by_sign[sign].append(row)
        target = float(row["target_energy"])
        ratio = float(row["target_ratio"])
        initial = float(summary[
            ("negative" if sign < 0 else "positive")
            + "_initial_target_energy"])
        target_residual = max(
            target_residual, relative(target, initial * ratio))
        shell_sum = sum(float(row[name]) for name in (
            "near_field", "intermediate_field", "far_field"))
        partition_residual = max(
            partition_residual, abs(shell_sum - float(row["total_field"])))

    require(target_residual == 0.0, "correct target energy/ratio contract")
    require(partition_residual <= 3.1e-26, "field partition closure")
    for sign in (-1, 1):
        by_sign[sign].sort(key=lambda row: int(row["tick"]))
        require([int(row["tick"]) for row in by_sign[sign]]
                == list(range(81)), "complete tick sequence")
        require(float(by_sign[sign][0]["target_ratio"]) == 1.0,
                "initial target ratio")
        require(float(by_sign[sign][0]["core_ratio"]) == 1.0,
                "initial core ratio")

    fits = {sign: fit(by_sign[sign]) for sign in (-1, 1)}
    declines = {
        sign: 1.0 - float(by_sign[sign][64]["core_ratio"])
        / float(by_sign[sign][8]["core_ratio"])
        for sign in (-1, 1)
    }
    for sign in (-1, 1):
        prefix = "negative" if sign < 0 else "positive"
        require(relative(fits[sign]["gamma"],
                         float(summary[prefix + "_gamma_core"])) <= 1e-12,
                "gamma recomputation")
        require(relative(fits[sign]["delta_bic"],
                         float(summary[prefix + "_delta_bic"])) <= 1e-12,
                "BIC recomputation")
        require(relative(fits[sign]["r2"],
                         float(summary[prefix + "_r_squared"])) <= 1e-12,
                "R2 recomputation")
        require(relative(declines[sign],
                         float(summary[prefix + "_decline_tick8_tick64"]))
                <= 1e-12, "decline recomputation")
        require(fits[sign]["gamma"] > 0.0, "positive core decay rate")
        require(fits[sign]["delta_bic"] >= 10.0, "exponential BIC gate")
        require(fits[sign]["r2"] >= 0.995, "exponential R2 gate")
        require(declines[sign] >= 0.20, "core decline gate")

    rate_difference = abs(fits[-1]["gamma"] - fits[1]["gamma"]) / max(
        abs(fits[-1]["gamma"]), abs(fits[1]["gamma"]))
    history_rms = math.sqrt(sum(
        (float(by_sign[-1][tick]["core_ratio"])
         - float(by_sign[1][tick]["core_ratio"])) ** 2
        for tick in range(81)) / 81.0)
    far_difference = abs(
        float(by_sign[-1][80]["far_fraction"])
        - float(by_sign[1][80]["far_fraction"]))
    require(rate_difference <= 1e-4, "rate polarity gate")
    require(history_rms <= 1e-5, "history polarity gate")
    require(far_difference <= 1e-4, "far polarity gate")

    for sign in (-1, 1):
        final = by_sign[sign][80]
        require(float(final["far_field"]) < float(final["near_field"]),
                "registered remote-dominance gate fails")
        require(float(final["intermediate_field"])
                > float(final["near_field"]),
                "intermediate shell dominates")

    stable_fields = ("core_ratio", "total_field")
    changed_shells = ("near_field", "intermediate_field", "far_field")
    stable_max = {name: 0.0 for name in stable_fields}
    shell_max_abs = {name: 0.0 for name in changed_shells}
    shell_scale = {name: 0.0 for name in changed_shells}
    for old, new in zip(prior, rows):
        require((old["sign"], old["tick"]) == (new["sign"], new["tick"]),
                "replication row alignment")
        for name in stable_fields:
            stable_max[name] = max(stable_max[name],
                                   abs(float(old[name]) - float(new[name])))
        for name in changed_shells:
            shell_max_abs[name] = max(
                shell_max_abs[name], abs(float(old[name]) - float(new[name])))
            shell_scale[name] = max(shell_scale[name], abs(float(old[name])),
                                    abs(float(new[name])))
    require(stable_max["core_ratio"] == 0.0, "core history exact replay")
    require(stable_max["total_field"] == 0.0,
            "total field history exact replay")
    require(any(shell_max_abs[name] > 0.0 for name in changed_shells),
            "decoder correction changes shell boundaries")

    print(
        "FTD-0681 corrected-data certificate: PASS\n"
        "registered_verdict=EXECUTION_INVALID_REPLICATION_GATE\n"
        "posthoc_classifier=REMOTE_FIELD_NOT_DOMINANT\n"
        f"checks={checks} rows={len(rows)} "
        f"target_residual={target_residual:.3e} "
        f"partition_residual={partition_residual:.3e}\n"
        f"gamma=({fits[-1]['gamma']:.12g},{fits[1]['gamma']:.12g}) "
        f"decline=({declines[-1]:.9g},{declines[1]:.9g})\n"
        f"final_fractions_negative=(near={float(by_sign[-1][80]['near_fraction']):.9g},"
        f"middle={float(by_sign[-1][80]['intermediate_field'])/float(by_sign[-1][80]['total_field']):.9g},"
        f"far={float(by_sign[-1][80]['far_fraction']):.9g})\n"
        f"shell_scaled_changes={{{', '.join(f'{name}:{shell_max_abs[name]/shell_scale[name]:.6g}' for name in changed_shells)}}}\n"
        f"json_sha256={sha256(V3_JSON)}\n"
        f"csv_sha256={sha256(V3_CSV)}"
    )


if __name__ == "__main__":
    main()
