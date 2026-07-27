"""Apply the frozen FTD-0426 gates to the CPU/CUDA run-of-record files."""

from __future__ import annotations

import csv
from hashlib import sha256
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "engine/results/ftd_0426/manifest.json"


def load_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def samples(rows: list[dict[str, str]], q: int, stage: str, body: str) -> list[float]:
    selected = [
        float(row["boundary_flux"])
        for row in rows
        if int(row["orientation"]) == q
        and row["stage"] == stage
        and row["body"] == body
    ]
    assert len(selected) == 4
    return selected


def mean(values: list[float]) -> float:
    return sum(values) / len(values)


def plateau(values: list[float]) -> float:
    return (max(values) - min(values)) / max(1e-15, abs(mean(values)))


def stage_means(rows: list[dict[str, str]], q: int, stage: str) -> tuple[float, float]:
    return mean(samples(rows, q, stage, "A")), mean(samples(rows, q, stage, "B"))


def readout_arm(rows: list[dict[str, str]], q: int) -> bool:
    neutral = samples(rows, q, "neutral", "A") + samples(rows, q, "neutral", "B")
    qa = samples(rows, q, "projected", "A")
    qb = samples(rows, q, "projected", "B")
    ma, mb = mean(qa), mean(qb)
    gauss = [
        abs(float(row["gauss_residual"]))
        for row in rows
        if int(row["orientation"]) == q and row["stage"] == "projected"
    ]
    return (
        max(map(abs, neutral)) <= 0.10
        and ma * q > 0
        and mb * q < 0
        and min(abs(ma), abs(mb)) >= 0.50
        and abs(ma + mb) <= 0.10
        and plateau(qa) <= 0.15
        and plateau(qb) <= 0.15
        and max(gauss) <= 0.15
    )


def live_arm(rows: list[dict[str, str]], q: int) -> bool:
    pa, pb = stage_means(rows, q, "projected")
    qa = samples(rows, q, "live", "A")
    qb = samples(rows, q, "live", "B")
    ma, mb = mean(qa), mean(qb)
    gauss = [
        abs(float(row["gauss_residual"]))
        for row in rows
        if int(row["orientation"]) == q and row["stage"] == "live"
    ]
    return (
        ma * q > 0
        and mb * q < 0
        and min(abs(ma), abs(mb)) >= 0.50
        and abs(ma + mb) <= 0.10
        and plateau(qa) <= 0.15
        and plateau(qb) <= 0.15
        and max(gauss) <= 0.15
        and abs(ma - pa) <= 0.10
        and abs(mb - pb) <= 0.10
    )


def mirror(rows: list[dict[str, str]], stage: str) -> bool:
    positive = stage_means(rows, +1, stage)
    negative = stage_means(rows, -1, stage)
    return all(abs(p + n) <= 0.10 for p, n in zip(positive, negative))


def main() -> None:
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    checks: list[tuple[str, bool]] = []
    datasets: dict[str, list[dict[str, str]]] = {}

    for run in manifest["runs"]:
        path = ROOT / run["output"]
        checks.append((f"HASH {run['backend']}", sha256(path.read_bytes()).hexdigest() == run["sha256"]))
        rows = load_rows(path)
        datasets[run["backend"]] = rows
        checks.extend(
            [
                (f"{run['backend']} row contract", len(rows) == 48),
                (f"{run['backend']} actual backend recorded",
                 {row["backend"] for row in rows} == {run["backend"]}),
                (f"{run['backend']} transport/observer valid",
                 all(row["valid"] == "1" for row in rows)),
                (f"{run['backend']} exact central telescope",
                 max(abs(float(row["telescope_residual"])) for row in rows)
                 <= 1e-12 * (1 + max(abs(float(row["divergence_sum"])) for row in rows))),
                (f"{run['backend']} readout + arm", readout_arm(rows, +1)),
                (f"{run['backend']} readout - arm", readout_arm(rows, -1)),
                (f"{run['backend']} projected mirror", mirror(rows, "projected")),
                (f"{run['backend']} autonomous + arm fails", not live_arm(rows, +1)),
                (f"{run['backend']} autonomous - arm fails", not live_arm(rows, -1)),
            ]
        )

    cpu = datasets["cpu"]
    gpu = datasets["gpu"]
    for stage in ("projected", "live"):
        for q in (+1, -1):
            cm = stage_means(cpu, q, stage)
            gm = stage_means(gpu, q, stage)
            checks.append((
                f"CPU/CUDA {stage} q={q} agreement",
                max(abs(a - b) for a, b in zip(cm, gm)) <= 0.10,
            ))

    checks.append((
        "locked verdict is outcome B",
        manifest["verdict"] == "B_SELECTED_GAUSS_CONSTRAINT_REALIZATION",
    ))

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed

    for backend, rows in datasets.items():
        p = stage_means(rows, +1, "projected")
        live = stage_means(rows, +1, "live")
        live_pa = plateau(samples(rows, +1, "live", "A"))
        live_pb = plateau(samples(rows, +1, "live", "B"))
        max_live_gauss = max(
            abs(float(row["gauss_residual"]))
            for row in rows if row["stage"] == "live"
        )
        print(
            f"{backend}: projected={p}, live={live}, "
            f"live_plateau=({live_pa:.9g},{live_pb:.9g}), "
            f"max_live_gauss={max_live_gauss:.9g}"
        )

    print(f"\nEmergent static-charge result checks: {len(checks) - failed}/{len(checks)} passed")
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
