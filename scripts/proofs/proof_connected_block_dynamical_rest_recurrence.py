"""Independent record and DFT certificate for FTD-0627."""

from __future__ import annotations

import csv
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0627"
PROTOCOL = "72B38166003A90DF92FFEFEF90F2F363A00A96CFEA4EEDDB8BBC57EE3CAF0A4A"
PARENT = "DEDFF2C31C510A7944CF5FD213E1165172342324B6C38432D599F4F212570308"


checks: list[tuple[str, bool]] = []


def check(name: str, condition: bool) -> None:
    checks.append((name, bool(condition)))


def read_csv(name: str) -> list[dict[str, str]]:
    with (RESULTS / name).open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


with (RESULTS / "ftd_0627_connected_block_dynamical_rest_v1.json").open(
    encoding="utf-8"
) as handle:
    summary = json.load(handle)
arms = read_csv("ftd_0627_connected_block_dynamical_rest_arms_v1.csv")
ticks = read_csv("ftd_0627_connected_block_dynamical_rest_ticks_v1.csv")
spectra = read_csv("ftd_0627_connected_block_dynamical_rest_spectra_v1.csv")

arms_by_label = {row["label"]: row for row in arms}
ticks_by_label: dict[str, list[dict[str, str]]] = {}
for row in ticks:
    ticks_by_label.setdefault(row["label"], []).append(row)
spectra_by_key = {(row["label"], row["observable"]): row for row in spectra}


check("record id", summary["ftd_id"] == "FTD-0627")
check("protocol", summary["protocol_sha256"] == PROTOCOL)
check("parent", summary["parent_result_sha256"] == PARENT)
check("production unchanged", summary["production_changed"] is False)
check("coverage", summary["coverage_pass"] == 1)
check("execution", summary["execution_pass"] == 1)
check("bounded", summary["bounded_pass"] == 1)
check("no recurrence", summary["recurrence_pass"] == 0 and summary["common_period"] == 0)
check("spectral conjunction fails", summary["spectral_concentration_pass"] == 0)
check("covariance", summary["covariance_pass"] == 1 and float(summary["covariance_residual"]) <= 1e-8)
check("verdict", summary["verdict"] == "CONNECTED_BLOCK_BOUNDED_IRREGULAR_REST_OPEN")
check("common action", float(summary["worst_common_residual"]) <= 1e-10)
check("energy", float(summary["worst_energy_drift"]) <= 1e-8)
check("recovery", float(summary["worst_recovery"]) <= 1e-8)

check("two arms", set(arms_by_label) == {"rest_x", "rest_y"})
check("256 samples", all(len(ticks_by_label[label]) == 256 for label in arms_by_label))
check("forward and reverse", all(row["forward"] == "1" and row["reverse"] == "1" for row in arms))
check("metadata", all(row["metadata"] == "1" for row in arms))
check("arm bounded flags", all(row["bounded"] == "1" for row in arms))
check("multiplicity", max(int(row["max_multiplicity"]) for row in arms) == 2)
check("shared separation", min(float(row["min_separation"]) for row in arms) >= 0.90)
check("centre displacement", max(float(row["max_displacement"]) for row in arms) <= 1e-8)
check("centre momentum", max(float(row["max_momentum"]) for row in arms) <= 1e-8)
check("shape bounded", max(float(row["max_shape"]) for row in arms) <= 1e-2)
check("strain bounded", max(float(row["max_strain"]) for row in arms) <= 3e-2)
check("no arm period", all(row["period"] == "0" for row in arms))

for label, rows in ticks_by_label.items():
    check(f"{label} tick coverage", [int(row["tick"]) for row in rows] == list(range(1, 257)))
    check(f"{label} recurrence recomputation", not any(
        float(rows[p - 1]["state_distance"]) <= 1e-6
        and float(rows[2 * p - 1]["state_distance"]) <= 1e-6
        for p in range(16, 129)
    ))
    check(f"{label} fibre tick gate", all(
        int(row["multiplicity"]) <= 2 and float(row["separation"]) >= 0.90
        for row in rows
    ))
    check(f"{label} centre tick gate", all(
        math.sqrt(sum(float(row[key]) ** 2 for key in ("dx", "dy", "dz"))) <= 1e-8
        and math.sqrt(sum(float(row[key]) ** 2 for key in ("px", "py", "pz"))) <= 1e-8
        for row in rows
    ))


observable_columns = {"Q1": "q1", "Q2": "q2", "Q3": "q3", "interface": "interface"}
for label, rows in ticks_by_label.items():
    for observable, column in observable_columns.items():
        values = [float(row[column]) for row in rows]
        mean = sum(values) / len(values)
        powers: list[tuple[float, int]] = []
        for k in range(1, 129):
            real = 0.0
            imaginary = 0.0
            for n, value in enumerate(values):
                angle = 2.0 * math.pi * k * n / 256.0
                centered = value - mean
                real += centered * math.cos(angle)
                imaginary -= centered * math.sin(angle)
            powers.append((real * real + imaginary * imaginary, k))
        powers.sort(key=lambda item: (-item[0], item[1]))
        total = sum(power for power, _ in powers)
        concentration = sum(power for power, _ in powers[:8]) / total
        record = spectra_by_key[(label, observable)]
        record_bins = [int(item) for item in record["bins"].split(";")]
        check(f"{label} {observable} bins", record_bins == [k for _, k in powers[:8]])
        check(f"{label} {observable} total", math.isclose(float(record["total_power"]), total, rel_tol=2e-10, abs_tol=1e-20))
        check(f"{label} {observable} concentration", math.isclose(float(record["concentration"]), concentration, rel_tol=2e-10, abs_tol=1e-14))

concentrations = [float(row["concentration"]) for row in spectra]
check("some spectra below gate", min(concentrations) < 0.90)
check("interface spectra above gate", all(
    float(row["concentration"]) >= 0.90 for row in spectra if row["observable"] == "interface"
))
check("cyclic bin sets", all(
    set(spectra_by_key[("rest_x", observable)]["bins"].split(";"))
    == set(spectra_by_key[("rest_y", observable)]["bins"].split(";"))
    for observable in observable_columns
))


failed = [name for name, passed in checks if not passed]
for name, passed in checks:
    print(f"{'PASS' if passed else 'FAIL'}: {name}")
print(f"\n{sum(passed for _, passed in checks)}/{len(checks)} checks pass")
if failed:
    raise SystemExit("failed checks: " + ", ".join(failed))
