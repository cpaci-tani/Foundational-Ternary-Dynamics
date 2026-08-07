"""Independent serialized-record certificate for FTD-0750.

This certificate reads the frozen FTD-0745 baseline and all 24 FTD-0750
records.  It does not call the C++ verdict function or rerun dynamics.
"""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/engine_infrastructure_rg/PREREG_ORDERED_CURRENT_DETERMINISTIC_OBSERVER_CUDA_v1.md"
PREEXEC = ROOT / "docs/theory/07_assessment/engine_infrastructure_rg/AUDIT_ORDERED_CURRENT_DETERMINISTIC_OBSERVER_CUDA_PREEXEC_v1.md"
RUNNER = ROOT / "engine/tests/campaign_ordered_current_observer_cuda.cpp"
CUDA_HEADER = ROOT / "engine/include/ftd/eft/cuda_matched_field_pipeline.h"
CUDA_SOURCE = ROOT / "engine/cuda/cuda_matched_field_pipeline.cu"
UNIT = ROOT / "engine/tests/test_cuda_ordered_current_observer.cpp"
PARENT_0748 = ROOT / "engine/tests/campaign_canonical_current_horizon_cuda.cpp"
PARENT_0747 = ROOT / "engine/tests/campaign_causal_horizon_environmental_persistence_cuda.cpp"
BASELINE = ROOT / "engine/results/ftd_0745/ftd_0745_finite_support_environmental_closure_v1.csv"
RESULTS = ROOT / "engine/results/ftd_0750"
STEM = "ftd_0750_ordered_current_observer_cuda_v1"

HASHES = {
    PROTOCOL: "C3A3E787A201F1E429E1ED8D8D81B9F06B508A413B41A6B5E2584ED1BFD13385",
    PREEXEC: "9A2C739BBA3225971425A0DE7AF5ADF2816E31BDCB02C4D4600ABA8DF8BB7618",
    RUNNER: "D7ABFE3E6E8D255F17920CC2510CA9B150389FBC2092C4DE8113E354E9A15963",
    CUDA_HEADER: "B7EBCF382BEDED20921267FD30BC3B7AF501BF4DDD933E272D66CC799B79B5C5",
    CUDA_SOURCE: "62080A7CC52560DDCB0F0F6F69CB6CF41C18C02A930F5C037540C40875246022",
    UNIT: "000D91CEF745F3490968E2965A9AA205BEC9137C492CDEE710FB2ED9A7921EDA",
    PARENT_0748: "70948B76A359DC01A92DC2BD46289DDA1D318009B51DB63889D95413DDC2EED8",
    PARENT_0747: "85E4FBE7D0A3A21EB760C3D9F173CAA9BE7F9699596A93609FABD50683462F14",
    BASELINE: "58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C",
}

RESULT_HASHES = {
    "body_a_support.csv": "1121D872D0E94909D26A74AD0D82669A03E6BFC2FE69631E5FE755093371EC8E",
    "body_a_support.json": "8A609B11AC28596A648A0F471F381C28542E0FE4B5B6E72CED89DF3463514C51",
    "body_a.csv": "ACA988489A10CA5F37B7D02D646FAA1194C0553380C2A08F3F6567E6EC01BB5A",
    "body_a.json": "4D2FF0C8ABC16701B8FD1DB8F00E1A84C56DFF7F81124277ACEE98157EF2475B",
    "body_b_support.csv": "1121D872D0E94909D26A74AD0D82669A03E6BFC2FE69631E5FE755093371EC8E",
    "body_b_support.json": "08951E8E994B2AB875670D53304D5425298F742FB278BA6CC2039BBA8EB976BA",
    "body_b.csv": "D82FBCD303E75597674244E069AAB941D62F37F4DAAA7CFC0C38E1E6BD335BDB",
    "body_b.json": "A5E3C7A7AE0DD80D6AAA89C4E62435C4187179E737E374B95117639F84D4546D",
    "edge_a_support.csv": "947379E1ECCF59838BA5CD4C5437F95D8FBAEF13F888E629FFD25DFFF566236D",
    "edge_a_support.json": "F6F55C61EC2061B3A9192E37917CCE1C8FBD3A80C95508FDAD0A2A93534A3CE8",
    "edge_a.csv": "8FE0ED948E408C6C067E57D28283A74FA08022C9A827DD3382FBC105060FCCBE",
    "edge_a.json": "4D9F3EBEEBE070AE0FC1BEAE098EFFF6AE77D810814D84BE7F2B846000F86FFA",
    "edge_b_support.csv": "947379E1ECCF59838BA5CD4C5437F95D8FBAEF13F888E629FFD25DFFF566236D",
    "edge_b_support.json": "5A507D6C447A87A027C2C4E8E94C7B9A53373A7B13992765651CDE6AE1134D2B",
    "edge_b.csv": "CA3FD1D4862A36C0DF2AE5A80F7EE65A977D6556D296BDA847F86E9F968A874F",
    "edge_b.json": "B62EB08CB8F37B85C55835603D0F2964AFBB2261315F154CE4D3468568E50A07",
    "face_a_support.csv": "39ECA05F23939BC40ED406ED7EADD72A368304007BFF2EEABFFC4877E3D56890",
    "face_a_support.json": "AF4BB747C5E5C923413257BFDF33C1A114A57E90A05318D1F22A8C52DC8E3F94",
    "face_a.csv": "3AC04D34B85C7FD1A3D39259C7501484BAD835E6DEDA94C337B6FEC166F9E72B",
    "face_a.json": "AC96E82A0DC6E741BCFD3D939AC9DF936BC6D4755D43F03DEC44BE0F6F10BC9D",
    "face_b_support.csv": "39ECA05F23939BC40ED406ED7EADD72A368304007BFF2EEABFFC4877E3D56890",
    "face_b_support.json": "4603BB417A435A0F7C31FF164A98ECABEA43BD2D4A973839A03DCDA7DB6454EF",
    "face_b.csv": "5E67DFEB491E4040C3EDB4370AB5011AD260417C7C6C32CD1DA6D7D852E924A2",
    "face_b.json": "F73F975FE266CC6B26754458B78926BF53C9A9A52A5376052426F54A876A611F",
}

RAYS = {"face": "0_0_1", "edge": "0_1_-1", "body": "1_1_1"}
EXPECTED_VERDICT = {
    "face": "CANONICAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE",
    "edge": "CANONICAL_HORIZON_PREFIX_DRIFT",
    "body": "CANONICAL_HORIZON_PREFIX_DRIFT",
}
RADII = (8, 12, 16, 24, 32, 48)
HORIZON = 312
PREFIX = 184
checks: list[tuple[str, bool]] = []


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(label: str, condition: bool) -> None:
    checks.append((label, bool(condition)))
    print(f"{'PASS' if condition else 'FAIL'}  {label}")


def number(row: dict[str, str], column: str) -> float:
    return float(row[column])


def integer(row: dict[str, str], column: str) -> int:
    return int(row[column])


def flag(row: dict[str, str], column: str) -> bool:
    return bool(integer(row, column))


for path, expected in HASHES.items():
    check(f"frozen SHA-256 {path.name}", path.is_file() and sha256(path) == expected)
check("pre-execution audit retains executed ELF hash", (
    "F5E423093D8AB69BCBAAB936F25EABA4EB38E3E5223DE9E4EE3A01A2347EDAD2"
    in PREEXEC.read_text(encoding="utf-8")
))
for suffix, expected in RESULT_HASHES.items():
    path = RESULTS / f"{STEM}_{suffix}"
    check(f"frozen SHA-256 {path.name}", path.is_file() and sha256(path) == expected)

with BASELINE.open(newline="", encoding="utf-8") as handle:
    baseline_rows = list(csv.DictReader(handle))
baseline = {
    (row["direction"], row["polarity"], integer(row, "tick")): row
    for row in baseline_rows
    if row["family"] == "unbound" and integer(row, "tick") <= PREFIX
}
scalar_columns = (
    "max_residual", "total_energy_residual", "recoil_defect", "speed_excess",
    "regional_residual", "outside_source_residual", "separation",
    "pair_energy", "field_energy",
) + tuple(
    f"{prefix}_{radius}" for radius in RADII
    for prefix in ("inside", "outside", "transport_into", "source_exchange",
                   "cumulative_outward")
)
discrete_columns = (
    "valid", "common", "regional_valid", "source_radius", "graph_inside",
)

records: dict[tuple[str, str], dict[str, object]] = {}
for ray, direction in RAYS.items():
    for replicate in ("a", "b"):
        slug = f"{ray}_{replicate}"
        main_csv = RESULTS / f"{STEM}_{slug}.csv"
        main_json = RESULTS / f"{STEM}_{slug}.json"
        support_csv = RESULTS / f"{STEM}_{slug}_support.csv"
        support_json = RESULTS / f"{STEM}_{slug}_support.json"
        with main_csv.open(newline="", encoding="utf-8") as handle:
            rows = list(csv.DictReader(handle))
        with support_csv.open(newline="", encoding="utf-8") as handle:
            support = list(csv.DictReader(handle))
        summary = json.loads(main_json.read_text(encoding="utf-8"))
        support_summary = json.loads(support_json.read_text(encoding="utf-8"))
        ticks = list(range(HORIZON + 1))
        check(f"{slug}: 313 ordered main/support rows", (
            len(rows) == len(support) == HORIZON + 1
            and [integer(row, "tick") for row in rows] == ticks
            and [integer(row, "tick") for row in support] == ticks
        ))
        check(f"{slug}: frozen metadata", (
            summary["ftd_id"] == support_summary["ftd_id"] == "FTD-0750"
            and summary["protocol_sha256"] == support_summary["protocol_sha256"]
                == HASHES[PROTOCOL]
            and summary["backend"] == support_summary["backend"]
                == "wsl2_cuda_ordered_current_observer"
            and summary["arm"] == support_summary["arm"] == slug
            and summary["direction"] == direction
            and summary["volume"] == 321 and summary["horizon"] == HORIZON
            and summary["contact_tick"] == 313
        ))

        max_common = max(number(row, "max_residual") for row in rows)
        max_energy = max(number(row, "total_energy_residual") for row in rows)
        max_recoil = max(number(row, "recoil_defect") for row in rows)
        max_speed = max(number(row, "speed_excess") for row in rows)
        max_regional = max(number(row, "regional_residual") for row in rows)
        max_outside_source = max(number(row, "outside_source_residual") for row in rows)
        max_source = max(integer(row, "source_radius") for row in rows)
        pair = [number(row, "pair_energy") for row in rows]
        field = [number(row, "field_energy") for row in rows]
        balance = abs(pair[-1] - pair[0] + field[-1] - field[0])
        h0 = (
            all(flag(row, "valid") and flag(row, "common")
                and flag(row, "regional_valid") for row in rows)
            and max_common <= 1e-10 and max_energy <= 1e-8
            and max_recoil <= 1e-9 and max_speed <= 1e-12
            and max_regional <= 1e-10 and max_outside_source <= 1e-10
            and max_source <= 3 and balance <= 1e-8 and HORIZON < 313
        )
        check(f"{slug}: H0 execution", h0)

        max_moment = max(number(row, "moment_residual") for row in support)
        max_discarded = max(number(row, "discarded_l1") for row in support)
        a0 = (
            all(flag(row, "valid") for row in support)
            and max_moment <= 1e-12 and max_discarded <= 1e-10
            and max(integer(row, "source_radius") for row in support) <= 3
            and all(integer(s, "net_support") == integer(m, "source_entries")
                    and integer(s, "source_radius") == integer(m, "source_radius")
                    for m, s in zip(rows, support))
        )
        check(f"{slug}: A0 canonical aggregation", a0)

        mismatches: set[str] = set()
        prefix_difference = 0.0
        prefix_location = (-1, "")
        for tick in range(PREFIX + 1):
            now = rows[tick]
            old = baseline[(direction, "plus_minus", tick)]
            mismatches |= {column for column in discrete_columns
                           if now[column] != old[column]}
            for column in scalar_columns:
                difference = abs(number(now, column) - number(old, column))
                if difference > prefix_difference:
                    prefix_difference = difference
                    prefix_location = (tick, column)
        h1_discrete = not mismatches
        h1 = h1_discrete and prefix_difference <= 1e-10
        check(f"{slug}: corrected discrete prefix exact", h1_discrete)
        print(f"GATE  {slug}: H1 {'PASS' if h1 else 'FAIL'} "
              f"({prefix_difference:.12e}, {prefix_location})")
        check(f"{slug}: H1 status matches frozen output", h1 == (ray == "face"))

        graph = [flag(row, "graph_inside") for row in rows]
        onset = next((tick for tick in range(HORIZON + 1) if all(
            graph[later] and pair[later] < -1e-6
            for later in range(tick, HORIZON + 1))), -1)
        h2 = onset >= 0 and HORIZON - onset + 1 >= 160
        check(f"{slug}: H2 persistent core", h2)
        late = [number(rows[tick], "inside_8") for tick in range(281, 313)]
        h3 = min(late) >= 5e-4 and max(late) <= 4.0 * min(late)
        check(f"{slug}: H3 stable near field", h3)
        outside48 = [number(row, "outside_48") for row in rows]
        first48 = next((tick for tick, value in enumerate(outside48)
                        if value > 1e-8), -1)
        h4 = (outside48[0] <= 1e-12 and 0 <= first48 <= 300
              and max(outside48) > 1e-8 and max_outside_source <= 1e-10)
        check(f"{slug}: H4 radius-48 arrival at {first48}", h4)
        outward = [-number(rows[tick], "transport_into_48")
                   for tick in range(first48, HORIZON + 1)]
        h5 = (h4 and min(outward) >= -1e-10 and outside48[-1] > 1e-9
              and all(value > 1e-9 for value in outside48[301:313]))
        check(f"{slug}: H5 post-arrival persistence", h5)

        if not h0:
            verdict = "CANONICAL_HORIZON_EXECUTION_INVALID"
        elif not a0:
            verdict = "CANONICAL_HORIZON_CURRENT_AGGREGATION_INVALID"
        elif not h1:
            verdict = "CANONICAL_HORIZON_PREFIX_DRIFT"
        elif not h2:
            verdict = "CANONICAL_HORIZON_CORE_NOT_PERSISTENT"
        elif not h3:
            verdict = "CANONICAL_HORIZON_NEAR_FIELD_NOT_STABLE"
        elif not h4:
            verdict = "CANONICAL_HORIZON_R48_ARRIVAL_FAIL"
        elif not h5:
            verdict = "CANONICAL_HORIZON_POST_ARRIVAL_NOT_PERSISTENT"
        else:
            verdict = "CANONICAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE"
        check(f"{slug}: ordered verdict reconstructs", (
            verdict == EXPECTED_VERDICT[ray] == summary["verdict"]
            == support_summary["verdict"]
            and bool(summary["prefix_pass"]) == h1
            and bool(summary["core_pass"]) == h2
            and bool(summary["near_field_pass"]) == h3
            and bool(summary["arrival_pass"]) == h4
            and bool(summary["post_arrival_pass"]) == h5
            and math.isclose(summary["prefix_scalar_difference"],
                             prefix_difference, rel_tol=1e-13, abs_tol=1e-15)
        ))
        records[(ray, replicate)] = {
            "rows": rows, "support": support, "summary": summary,
            "support_summary": support_summary, "verdict": verdict,
            "prefix_difference": prefix_difference,
            "prefix_location": prefix_location, "onset": onset,
            "arrival": first48, "main_csv": main_csv,
            "support_csv": support_csv,
        }

for ray in RAYS:
    left = records[(ray, "a")]
    right = records[(ray, "b")]
    check(f"{ray}: D0 support CSV byte identity", (
        left["support_csv"].read_bytes() == right["support_csv"].read_bytes()
    ))
    check(f"{ray}: D0 every main CSV cell except arm identical", all(
        row_a[column] == row_b[column]
        for row_a, row_b in zip(left["rows"], right["rows"])
        for column in row_a if column != "arm"
    ))
    support_a = dict(left["support_summary"])
    support_b = dict(right["support_summary"])
    support_a.pop("arm"); support_b.pop("arm")
    check(f"{ray}: D0 support JSON physical identity", support_a == support_b)
    main_a = dict(left["summary"])
    main_b = dict(right["summary"])
    main_a.pop("arm"); main_b.pop("arm")
    check(f"{ray}: D0 main JSON physical identity", main_a == main_b)
    check(f"{ray}: D0 verdict identity", left["verdict"] == right["verdict"])

strict_d0 = all(
    all(row_a[column] == row_b[column]
        for row_a, row_b in zip(records[(ray, "a")]["rows"],
                                records[(ray, "b")]["rows"])
        for column in row_a if column != "arm")
    for ray in RAYS
)
check("three-ray D0 strict replay identity is constructive", strict_d0)
physics_conjunction = all(
    record["verdict"] == "CANONICAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE"
    for record in records.values()
)
check("six-record D0+H0--H5 conjunction closes negative on edge/body H1",
      not physics_conjunction)

passed = sum(condition for _, condition in checks)
print(f"\nFTD-0750 independent certificate: {passed}/{len(checks)} checks passed")
for key, record in records.items():
    print(key, json.dumps({name: value for name, value in record.items()
                           if name not in {"rows", "support", "summary",
                                           "support_summary", "main_csv",
                                           "support_csv"}}, sort_keys=True))
raise SystemExit(0 if passed == len(checks) else 1)
