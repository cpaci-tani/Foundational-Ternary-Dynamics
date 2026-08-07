"""Independent serialized-record certificate for FTD-0749.

The proof reads the frozen FTD-0745 baseline and all 24 FTD-0749 records.  It
does not call a C++ verdict function or rerun dynamics.  Successful exit
certifies the registered mixed/negative result, including the expected failed
D0 and face-H1 gates.
"""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/engine_infrastructure_rg/PREREG_DETERMINISTIC_CANONICAL_CURRENT_CUDA_v1.md"
PREEXEC = ROOT / "docs/theory/07_assessment/engine_infrastructure_rg/AUDIT_DETERMINISTIC_CANONICAL_CURRENT_CUDA_PREEXEC_v1.md"
RUNNER = ROOT / "engine/tests/campaign_deterministic_canonical_current_cuda.cpp"
CUDA_HEADER = ROOT / "engine/include/ftd/eft/cuda_matched_field_pipeline.h"
CUDA_SOURCE = ROOT / "engine/cuda/cuda_matched_field_pipeline.cu"
AGG_HEADER = ROOT / "engine/include/ftd/eft/quadratic_coat_face_current.h"
AGG_SOURCE = ROOT / "engine/src/eft/quadratic_coat_face_current.cpp"
PARENT_0748 = ROOT / "engine/tests/campaign_canonical_current_horizon_cuda.cpp"
PARENT_0747 = ROOT / "engine/tests/campaign_causal_horizon_environmental_persistence_cuda.cpp"
UNIT = ROOT / "engine/tests/test_cuda_canonical_current_deposition.cpp"
BASELINE = ROOT / "engine/results/ftd_0745/ftd_0745_finite_support_environmental_closure_v1.csv"
RESULTS = ROOT / "engine/results/ftd_0749"
STEM = "ftd_0749_deterministic_canonical_current_cuda_v1"

HASHES = {
    PROTOCOL: "6C0BE1E8109DBD17451FF3A21F426A75583120810EB8C0C9B9077056AE86BB83",
    PREEXEC: "0FBA25775BF0E594B44ECF9D06DA6E932C12401CFE84BE89464E023364D7D405",
    RUNNER: "A1D8E0FA9DCFCF07E99DE87FB1CCDC0653A373BE23C34ED28C404A80B76C83B3",
    CUDA_HEADER: "FB14626C32BC1F8EA4667E9FFE3982455E319F469C3F418831EF38A55A8DE312",
    CUDA_SOURCE: "86C1E5C9A4F12F761258706026CA8A8EDB2B061BA6319636980F954A7C046D9D",
    AGG_HEADER: "77E67E4EBC4B27F7A70B8289195EA2D3A398A862C04DA29041C4ED33B8DA7409",
    AGG_SOURCE: "DD39B5776D74F9D942F0F5BA7518ED2D4B97E42927E361D314E5C7D6F1D0F1D0",
    PARENT_0748: "70948B76A359DC01A92DC2BD46289DDA1D318009B51DB63889D95413DDC2EED8",
    PARENT_0747: "85E4FBE7D0A3A21EB760C3D9F173CAA9BE7F9699596A93609FABD50683462F14",
    UNIT: "75DA11585CBA008C44A639BBE09F754394327B399D3BC88C55782CF2E4715039",
    BASELINE: "58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C",
}

RESULT_HASHES = {
    "body_a_support.csv": "7194499BB7E53537C2C64C953D8D1A13BF8112D8A73EE40EB8A41CC79C52B490",
    "body_a_support.json": "E96E060FD376D636DF33E1E72AF15A75E9E13A2664EB7118FF751076A20F5D1B",
    "body_a.csv": "A65783BFDD42EFD36B856088DA161D985598078A75D845334219D81E3BCBB840",
    "body_a.json": "2422B9C1EA0841746E620E18E36D45C6A41E50D47E441F361E91A18A93E15821",
    "body_b_support.csv": "7194499BB7E53537C2C64C953D8D1A13BF8112D8A73EE40EB8A41CC79C52B490",
    "body_b_support.json": "3706FB35C9DDB108664C8EAB3E64753AA061A54C754BEEF15CA3A81675770C82",
    "body_b.csv": "93259F232AAB8AB0AA99D8B2C06C52348CB781B91415B2973D5E92E668969562",
    "body_b.json": "D8F348FDE8E658DB17BFB6166C06AA4BB476B212C41BE2F74AE353415BD42A88",
    "edge_a_support.csv": "C46CD1C40BA8105A712ED0AC51A128EAF71BE65183D911D8038BDEFBE0208346",
    "edge_a_support.json": "A92D734E9D31808BD0D6DBFFDF956758A247D8A690756D52B576B780574FFF7C",
    "edge_a.csv": "530A7C1972D24165B74985EC5E4874EDD6C0A28DE697323B6605E272A848453C",
    "edge_a.json": "D5B016507C625AEA8356AF0662E498448079D8C07A895B8B51E2826DCF399A29",
    "edge_b_support.csv": "C46CD1C40BA8105A712ED0AC51A128EAF71BE65183D911D8038BDEFBE0208346",
    "edge_b_support.json": "FB44ED03FEE383830CD7247885685C0A5D926FEA4F9DD10508C3B2175326921B",
    "edge_b.csv": "050EB275862F0DB6889A8323DEEFD9529E76E17542006438F2B92EA72740E81F",
    "edge_b.json": "188E0AF937C657DB34E2AEFD527FA1EF34081FC032BCA3C1E3452D913DB9D375",
    "face_a_support.csv": "7AFCF55104B4D9EA2D2CB04E86F54639736593953F2D7DD87A14FEC309A04733",
    "face_a_support.json": "09255CD8124A98540A58F3BD08B2C7B2A08DDD27DCF30D3510F6330D3CEC8D5B",
    "face_a.csv": "E53C760F4B1341FD0E4CF95F4C0B473AB5F68858BABF05E2BFA45DAB2251CA9F",
    "face_a.json": "E7C6BB0FEDBF59752D37F7FDCB45A9B698A078D1B994B5DD277D72FFD27CE4F5",
    "face_b_support.csv": "7AFCF55104B4D9EA2D2CB04E86F54639736593953F2D7DD87A14FEC309A04733",
    "face_b_support.json": "9F71C26F4E15149F4512BBAB2C9A4AECFFB04332AC351B65B6277A3DB20FC41C",
    "face_b.csv": "A358BF536AFBBB3E8886B860C6704DC2C38233E4757A490F6897DF995361F99C",
    "face_b.json": "AB83619C2E12E003BB8012B4B3382A72F8C79A99043D7B731198067D4927489C",
}

RAYS = {"face": "0_0_1", "edge": "0_1_-1", "body": "1_1_1"}
EXPECTED_VERDICT = {
    "face": "CANONICAL_HORIZON_PREFIX_DRIFT",
    "edge": "CANONICAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE",
    "body": "CANONICAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE",
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
    "13ECEF1C337BF1E50DD783D936F9263AB5F77DAD0697120B2CB7FB2C2280053D"
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
observer_columns = {"regional_residual", "outside_source_residual"} | {
    f"{prefix}_{radius}" for radius in RADII
    for prefix in ("inside", "outside", "transport_into", "source_exchange",
                   "cumulative_outward")
}

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
        label = slug
        ticks = list(range(HORIZON + 1))
        check(f"{label}: 313 ordered main/support rows", (
            len(rows) == len(support) == HORIZON + 1
            and [integer(row, "tick") for row in rows] == ticks
            and [integer(row, "tick") for row in support] == ticks
        ))
        check(f"{label}: metadata", (
            summary["ftd_id"] == support_summary["ftd_id"] == "FTD-0749"
            and summary["protocol_sha256"] == support_summary["protocol_sha256"]
                == HASHES[PROTOCOL]
            and summary["backend"] == support_summary["backend"]
                == "wsl2_cuda_unique_face_current"
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
        check(f"{label}: H0 execution", h0)

        max_moment = max(number(row, "moment_residual") for row in support)
        max_discarded = max(number(row, "discarded_l1") for row in support)
        max_support = max(integer(row, "net_support") for row in support)
        a0 = (
            all(flag(row, "valid") for row in support)
            and max_moment <= 1e-12 and max_discarded <= 1e-10
            and max(integer(row, "source_radius") for row in support) <= 3
            and all(integer(s, "net_support") == integer(m, "source_entries")
                    and integer(s, "source_radius") == integer(m, "source_radius")
                    for m, s in zip(rows, support))
        )
        check(f"{label}: A0 canonical aggregation", a0)

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
        check(f"{label}: corrected discrete prefix exact", h1_discrete)
        print(f"GATE  {label}: H1 {'PASS' if h1 else 'FAIL'} "
              f"({prefix_difference:.12e}, {prefix_location})")
        check(f"{label}: H1 status matches ray", h1 == (ray != "face"))

        graph = [flag(row, "graph_inside") for row in rows]
        onset = next((tick for tick in range(HORIZON + 1) if all(
            graph[later] and pair[later] < -1e-6
            for later in range(tick, HORIZON + 1))), -1)
        h2 = onset >= 0 and HORIZON - onset + 1 >= 160
        check(f"{label}: H2 persistent core", h2)
        late = [number(rows[tick], "inside_8") for tick in range(281, 313)]
        h3 = min(late) >= 5e-4 and max(late) <= 4.0 * min(late)
        check(f"{label}: H3 stable near field", h3)
        outside48 = [number(row, "outside_48") for row in rows]
        first48 = next((tick for tick, value in enumerate(outside48)
                        if value > 1e-8), -1)
        h4 = (outside48[0] <= 1e-12 and 0 <= first48 <= 300
              and max(outside48) > 1e-8 and max_outside_source <= 1e-10)
        check(f"{label}: H4 radius-48 arrival at {first48}", h4)
        outward = [-number(rows[tick], "transport_into_48")
                   for tick in range(first48, HORIZON + 1)]
        h5 = (h4 and min(outward) >= -1e-10 and outside48[-1] > 1e-9
              and all(value > 1e-9 for value in outside48[301:313]))
        check(f"{label}: H5 post-arrival persistence", h5)

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
        check(f"{label}: ordered verdict reconstructs", (
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
            "arrival": first48, "max_support": max_support,
            "main_csv": main_csv, "support_csv": support_csv,
        }

for ray in RAYS:
    left = records[(ray, "a")]
    right = records[(ray, "b")]
    rows_a = left["rows"]
    rows_b = right["rows"]
    support_bytes = left["support_csv"].read_bytes() == right["support_csv"].read_bytes()
    check(f"{ray}: D0 support CSV byte identity", support_bytes)
    support_json_a = dict(left["support_summary"])
    support_json_b = dict(right["support_summary"])
    support_json_a.pop("arm")
    support_json_b.pop("arm")
    check(f"{ray}: D0 support JSON physical identity", support_json_a == support_json_b)

    mismatch_columns: set[str] = set()
    max_difference = 0.0
    for row_a, row_b in zip(rows_a, rows_b):
        for column in row_a:
            if column == "arm" or row_a[column] == row_b[column]:
                continue
            mismatch_columns.add(column)
            max_difference = max(max_difference,
                abs(number(row_a, column) - number(row_b, column)))
    check(f"{ray}: D0 trajectory cells exact", (
        all(row_a[column] == row_b[column]
            for row_a, row_b in zip(rows_a, rows_b)
            for column in row_a
            if column != "arm" and column not in observer_columns)
    ))
    check(f"{ray}: D0 mismatch isolated to observer reductions", (
        mismatch_columns == observer_columns and max_difference <= 3e-17
    ))

    main_json_a = dict(left["summary"])
    main_json_b = dict(right["summary"])
    main_json_a.pop("arm")
    main_json_b.pop("arm")
    allowed_json = {
        "maximum_regional_residual", "maximum_outside_source",
        "late_inside_8_minimum", "late_inside_8_maximum",
        "maximum_outside", "final_outside", "minimum_outward_increment",
    }
    json_mismatches = {key for key in main_json_a
                       if main_json_a[key] != main_json_b[key]}
    check(f"{ray}: D0 JSON mismatch isolated to observer summaries", (
        bool(json_mismatches) and json_mismatches <= allowed_json
    ))
    strict_d0 = (support_bytes and support_json_a == support_json_b
                 and not mismatch_columns and main_json_a == main_json_b)
    print(f"GATE  {ray}: strict D0 {'PASS' if strict_d0 else 'FAIL'}")
    check(f"{ray}: registered strict D0 reconstructs negative", not strict_d0)

physics_conjunction = all(
    record["verdict"] == "CANONICAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE"
    for record in records.values()
)
check("six-record physics conjunction closes negative on face H1",
      not physics_conjunction)
print("GATE  six-record D0+H0--H5 conjunction FAIL")

passed = sum(condition for _, condition in checks)
print(f"\nFTD-0749 independent certificate: {passed}/{len(checks)} checks passed")
for key, record in records.items():
    print(key, json.dumps({name: value for name, value in record.items()
                           if name not in {"rows", "support", "summary",
                                           "support_summary", "main_csv",
                                           "support_csv"}}, sort_keys=True))
raise SystemExit(0 if passed == len(checks) else 1)

