#!/usr/bin/env python3
"""Independent certificate for FTD-0619."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_SPLINE_POYNTING_NOETHER_DEFECT_v1.md"
PARENT = ROOT / "engine/results/ftd_0618/ftd_0618_closed_symmetry_balanced_gait_v1.json"
RESULT_DIR = ROOT / "engine/results/ftd_0619"
RESULT = RESULT_DIR / "ftd_0619_spline_poynting_noether_defect_v1.json"
SOURCE = RESULT_DIR / "ftd_0619_source_free_v1.csv"
CHANNELS = RESULT_DIR / "ftd_0619_channels_v1.csv"
EXPECTED_PROTOCOL = "F2E97844E14B77C152E986CD2CA317337FEE04E2367F73AD4A73FD76FE61E107"
EXPECTED_PARENT = "5F04E64DFD7CBFD10CE3AC779361C4124654C817320DFC81E6D5A482889F54D3"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def protocol_hash() -> str:
    raw = PREREG.read_bytes()
    marker = b"`protocol_sha256="
    return hashlib.sha256(raw[: raw.index(marker)]).hexdigest().upper()


def vector(row: dict[str, str], prefix: str) -> tuple[float, float, float]:
    return tuple(float(row[prefix + axis]) for axis in ("x", "y", "z"))


def add(*values: tuple[float, float, float]) -> tuple[float, float, float]:
    return tuple(sum(value[i] for value in values) for i in range(3))


def subtract(lhs: tuple[float, float, float],
             rhs: tuple[float, float, float]) -> tuple[float, float, float]:
    return tuple(lhs[i] - rhs[i] for i in range(3))


def norm(value: tuple[float, float, float]) -> float:
    return math.sqrt(sum(component * component for component in value))


def maximum_component(value: tuple[float, float, float]) -> float:
    return max(abs(component) for component in value)


record = json.loads(RESULT.read_text(encoding="utf-8"))
source_rows = list(csv.DictReader(SOURCE.open(encoding="utf-8", newline="")))
channel_rows = list(csv.DictReader(CHANNELS.open(encoding="utf-8", newline="")))

checks: dict[str, bool] = {}
checks["protocol_prefix"] = protocol_hash() == EXPECTED_PROTOCOL
checks["record_protocol"] = record["protocol_sha256"] == EXPECTED_PROTOCOL
checks["parent_hash"] = sha256(PARENT) == EXPECTED_PARENT
checks["record_parent"] = record["parent_result_sha256"] == EXPECTED_PARENT
checks["verdict"] = record["verdict"] == "CONTINUOUS_TRANSLATION_DEFECT_MEASURED"
checks["production_unchanged"] = record["production_changed"] is False
checks["overlap"] = record["overlap_pass"] == 1 and record["overlap_residual"] <= 1e-12
checks["source_coverage"] = record["source_coverage"] == 1 and len(source_rows) == 12
checks["source_axes_signs"] = {
    (int(row["L"]), int(row["axis"]), int(row["direction"]))
    for row in source_rows
} == {(volume, axis, sign) for volume in (16, 17)
     for axis in range(3) for sign in (-1, 1)}
checks["selected_source_invariant"] = all(
    float(row["selected_absolute_drift"]) <= 1e-10
    and float(row["selected_relative_drift"]) <= 1e-10
    for row in source_rows
)
checks["spline_source_invariant"] = record["source_spline_pass"] == 1 and all(
    float(row["spline_absolute_drift"]) <= 1e-10
    and float(row["spline_relative_drift"]) <= 1e-10
    and float(row["spline_transverse"]) <= 1e-12
    for row in source_rows
)
checks["source_covariance"] = record["source_covariance_residual"] <= 1e-12
checks["channel_coverage"] = record["channel_coverage"] == 1 \
    and len(record["channels"]) == 3 and len(channel_rows) == 384
checks["channel_ticks"] = all(
    sum(int(row["sign"]) == sign for row in channel_rows) == 128
    for sign in (-1, 0, 1)
)

matter_residual = 0.0
binding_residual = 0.0
selected_identity = 0.0
spline_identity = 0.0
for row in channel_rows:
    dp = vector(row, "dp")
    ie = vector(row, "ie")
    ib = vector(row, "ib")
    ibind = vector(row, "ibind")
    dsel = vector(row, "dsel")
    dspl = vector(row, "dspl")
    rsel = vector(row, "rsel")
    rspl = vector(row, "rspl")
    matter_residual = max(
        matter_residual, norm(subtract(dp, add(ie, ib, ibind))))
    binding_residual = max(binding_residual, norm(ibind))
    selected_identity = max(
        selected_identity, maximum_component(subtract(rsel, add(dp, dsel))))
    spline_identity = max(
        spline_identity, maximum_component(subtract(rspl, add(dp, dspl))))

checks["matter_impulse_identity"] = matter_residual <= 1e-12
checks["binding_cancellation"] = binding_residual <= 1e-12
checks["selected_defect_identity"] = selected_identity <= 1e-15
checks["spline_defect_identity"] = spline_identity <= 1e-15

def parse_json_vector(values: list[float]) -> tuple[float, float, float]:
    return tuple(float(value) for value in values)


final_channel_residual = 0.0
for arm in record["channels"]:
    reconstructed = add(
        parse_json_vector(arm["cumulative_electric"]),
        parse_json_vector(arm["cumulative_magnetic"]),
        parse_json_vector(arm["cumulative_binding"]),
    )
    final_channel_residual = max(
        final_channel_residual,
        norm(subtract(parse_json_vector(arm["final_matter_delta"]), reconstructed)),
    )
checks["cumulative_channel_identity"] = final_channel_residual <= 1e-12
checks["selected_does_not_close"] = record["selected_closes"] == 0 \
    and record["maximum_cumulative_selected"] > 1e-10
checks["spline_does_not_close"] = record["spline_closes"] == 0 \
    and record["maximum_cumulative_spline"] > 1e-10
checks["spline_not_improvement"] = (
    record["maximum_cumulative_spline"]
    >= record["maximum_cumulative_selected"]
)
checks["active_sign_mirror"] = record["active_sign_mirror_residual"] <= 1e-10

passed = sum(checks.values())
for name, ok in checks.items():
    print(f"{'PASS' if ok else 'FAIL'} {name}")
print(f"{passed}/{len(checks)} checks passed")
print(json.dumps({
    "ftd_id": "FTD-0619",
    "protocol_sha256": protocol_hash(),
    "parent_sha256": sha256(PARENT),
    "result_sha256": sha256(RESULT),
    "matter_impulse_residual": matter_residual,
    "binding_residual": binding_residual,
    "selected_identity_residual": selected_identity,
    "spline_identity_residual": spline_identity,
    "final_channel_residual": final_channel_residual,
    "passed": passed,
    "total": len(checks),
}, indent=2))
raise SystemExit(0 if passed == len(checks) else 1)

