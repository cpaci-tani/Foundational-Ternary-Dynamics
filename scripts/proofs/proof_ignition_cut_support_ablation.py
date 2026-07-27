#!/usr/bin/env python3
"""Independent lock/result verifier for FTD-0587.

This script re-evaluates only the preregistered arms and predicates.  It does
not search amplitudes, cuts, tolerances, or physical constants.
"""

from __future__ import annotations

import csv
import hashlib
import json
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG_SHA = "C2417CD829E665C6A4936D37DFA7C83F790925E5395FA387C34C03F27C857B2B"
VERDICT = "MIXED_OR_UNRESOLVED"

LOCKED_HASHES = {
    "preregistration": (
        "docs/theory/10_eft_program/preregistrations/"
        "PREREG_IGNITION_CUT_SUPPORT_ABLATION_v1.md",
        PREREG_SHA,
    ),
    "render_bridge": (
        "engine/src/render_bridge.cpp",
        "A822E0FAFAF71FE5458B2A7450868A8414B1C8564089BF6C6484FC34B7559359",
    ),
    "phase_read": (
        "engine/src/render_bridge_phases/phase_read.cpp",
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    ),
    "phase_write": (
        "engine/src/render_bridge_phases/phase_write.cpp",
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    ),
    "poisson_solver": (
        "engine/src/poisson_solvers.cpp",
        "AF43DC1DDE2DDF4A47C87B6D552DB053D7D25038FF801D5CB929401E681B4264",
    ),
    "toggles": (
        "engine/include/ftd/term_toggles.h",
        "2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA",
    ),
    "ftd0474_campaign": (
        "engine/tests/campaign_emergent_boundary_mechanism.cpp",
        "04F4D0D72879427EFC6BB1354B3D904C8F2214BE4B9F70912E5362F22F66135F",
    ),
    "ftd0474_observer": (
        "engine/include/ftd/eft/emergent_boundary_observer.h",
        "913789453A934EF8765414B9F523078E1BFA6E542BB67C0CA8D608EA7B651FC2",
    ),
    "observer_header": (
        "engine/include/ftd/eft/ignition_cut_support_ablation.h",
        "08A03E3848B0E0D58BA41E8AE51D1A853E29CC31FCFFCDEC6857C9B4C63771A4",
    ),
    "observer_source": (
        "engine/src/eft/ignition_cut_support_ablation.cpp",
        "FBCEB5B1C5AAF6E4AFC5E519F46ABF2E76DA213D81B53EA76E2ED98F6AFD868F",
    ),
    "native_test": (
        "engine/tests/test_ignition_cut_support_ablation.cpp",
        "5B4F15A8695E6C06C6D2FBDFBA63087C9FA9AF311F0856E366FC51187851B9D9",
    ),
    "run_json": (
        "engine/results/ftd_0587/windows_msvc_cpu.json",
        "38C510C3ED92BEE73962C3740FDA50E26F5273B583E24C3E86AFB287E54F4CFB",
    ),
    "run_csv": (
        "engine/results/ftd_0587/windows_msvc_cpu.csv",
        "E73D48F374AF4FCE4FDA44A29D8EDC92C32D9BEE86AB49BB90CF4DF5790A3B19",
    ),
}

ARMS = (
    "intact_reservoir",
    "intact_causal",
    "intact_projected",
    "cleared_control",
    "cleared_causal",
    "cleared_projected",
)
VOLUMES = (24, 32)
AMPLITUDES = (12, 20, 40)
SEEDS = (3759153152, 3759153153, 3759153154, 3759153155)


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: str) -> None:
        self.rows.append((bool(condition), name, note))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0587 ignition-cut support ablation")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        print("-" * 79)
        print(f"checks={len(self.rows)} passed={passed} "
              f"failed={len(self.rows)-passed}")
        print(f"verdict={VERDICT}")
        return passed == len(self.rows)


def sha256(relative: str) -> str:
    return hashlib.sha256((ROOT / relative).read_bytes()).hexdigest().upper()


P = Proof()
for name, (relative, expected) in LOCKED_HASHES.items():
    actual = sha256(relative)
    P.check(f"frozen hash {name}", actual == expected, actual)

csv_path = ROOT / "engine/results/ftd_0587/windows_msvc_cpu.csv"
with csv_path.open(newline="", encoding="utf-8") as handle:
    rows = list(csv.DictReader(handle))
record = json.loads(
    (ROOT / "engine/results/ftd_0587/windows_msvc_cpu.json")
    .read_text(encoding="utf-8")
)

P.check("registered Cartesian product has 144 rows", len(rows) == 144,
        f"rows={len(rows)}")
tuples = {
    (row["arm"], int(row["L"]), int(row["amplitude"]), int(row["seed"]))
    for row in rows
}
expected_tuples = {
    (arm, volume, amplitude, seed)
    for arm in ARMS
    for volume in VOLUMES
    for amplitude in AMPLITUDES
    for seed in SEEDS
}
P.check("no arm was added, omitted, or duplicated", tuples == expected_tuples,
        f"observed={len(tuples)} expected={len(expected_tuples)}")

prefixes: dict[tuple[int, int, int], set[tuple[str, str]]] = defaultdict(set)
supports_equal = True
structural_rows = True
stable_formula = True
for row in rows:
    key = (int(row["L"]), int(row["amplitude"]), int(row["seed"]))
    prefixes[key].add((row["prefix_state_hash"], row["prefix_rng_hash"]))
    supports_equal &= (
        row["support_hash_before"] == row["support_hash_after_intervention"]
    )
    volume = int(row["L"])
    minimum = int(row["minimum_sample_occupancy"])
    maximum = int(row["maximum_sample_occupancy"])
    recomputed_size = (
        int(row["sample_count"]) == 6
        and bool(int(row["all_samples_valid"]))
        and minimum >= 4
        and maximum <= int(0.01 * volume**3)
    )
    stable = (
        recomputed_size
        and float(row["occupancy_cv"]) <= 0.20
        and float(row["radius_cv"]) <= 0.15
    )
    stable_formula &= recomputed_size == bool(int(row["size_gate"]))
    stable_formula &= stable == bool(int(row["stable"]))
    structural_rows &= bool(int(row["finite"]))
    structural_rows &= bool(int(row["prefix_kinematics_clean"]))
    structural_rows &= float(row["maximum_velocity_before_rebase"]) == 0.0
    structural_rows &= float(row["maximum_remainder_before_rebase"]) == 0.0
    structural_rows &= float(row["maximum_velocity_after_rebase"]) == 0.0
    structural_rows &= float(row["maximum_remainder_after_rebase"]) == 0.0
    structural_rows &= int(row["movement_events"]) == 0
    structural_rows &= int(row["annihilation_events"]) == 0

P.check("24 prefixes are bit-identical across six replays",
        len(prefixes) == 24 and all(len(values) == 1 for values in prefixes.values()),
        f"prefixes={len(prefixes)} max_variants="
        f"{max(map(len, prefixes.values()), default=0)}")
P.check("all interventions preserve support and labels", supports_equal,
        "support hashes agree")
P.check("all per-run structural gates pass", structural_rows,
        "finite, zero kinematics, no movement/annihilation")
P.check("stability is independently reconstructed", stable_formula,
        "six samples, locked size and CV thresholds")

stable_runs: dict[str, int] = defaultdict(int)
passing_cells: dict[str, int] = defaultdict(int)
events: dict[str, tuple[int, int]] = {}
for arm in ARMS:
    arm_rows = [row for row in rows if row["arm"] == arm]
    stable_runs[arm] = sum(int(row["stable"]) for row in arm_rows)
    for volume in VOLUMES:
        for amplitude in AMPLITUDES:
            cell = [
                row for row in arm_rows
                if int(row["L"]) == volume
                and int(row["amplitude"]) == amplitude
            ]
            if sum(int(row["stable"]) for row in cell) >= 3:
                passing_cells[arm] += 1
    events[arm] = (
        sum(int(row["genesis_events"]) for row in arm_rows),
        sum(int(row["evaporation_events"]) for row in arm_rows),
    )

expected_stable = {
    "intact_reservoir": (0, 0),
    "intact_causal": (0, 0),
    "intact_projected": (20, 5),
    "cleared_control": (0, 0),
    "cleared_causal": (0, 0),
    "cleared_projected": (18, 4),
}
for arm, (run_count, cell_count) in expected_stable.items():
    P.check(f"{arm} locked support count",
            stable_runs[arm] == run_count
            and passing_cells[arm] == cell_count,
            f"runs={stable_runs[arm]} cells={passing_cells[arm]}")

cleared_control = [row for row in rows if row["arm"] == "cleared_control"]
cleared_causal = [row for row in rows if row["arm"] == "cleared_causal"]
P.check("cleared state-only control fully evaporates",
        all(int(row["final_occupancy"]) == 0 for row in cleared_control)
        and all(float(row["final_quadratic_field_norm"]) == 0.0
                for row in cleared_control),
        f"events={events['cleared_control']}")
P.check("causal source regenerates field but not support",
        all(int(row["final_occupancy"]) == 0 for row in cleared_causal)
        and any(float(row["maximum_quadratic_field_norm"]) > 0.0
                for row in cleared_causal),
        f"events={events['cleared_causal']}")

support_qualified = {
    arm: passing_cells[arm] >= 5 for arm in ARMS
}
reservoir_sufficient = support_qualified["intact_reservoir"]
causal_sufficient = (
    support_qualified["cleared_causal"]
    and not support_qualified["cleared_control"]
)
gauss_sufficient = (
    support_qualified["cleared_projected"]
    and not support_qualified["cleared_control"]
)
state_only = support_qualified["cleared_control"]
sufficient_count = sum((
    reservoir_sufficient, causal_sufficient, gauss_sufficient, state_only
))
interaction_without_isolated_component = (
    support_qualified["intact_causal"]
    and not support_qualified["intact_reservoir"]
    and not support_qualified["cleared_causal"]
) or (
    support_qualified["intact_projected"]
    and not support_qualified["intact_reservoir"]
    and not support_qualified["cleared_projected"]
)
mixed = sufficient_count > 1 or interaction_without_isolated_component
no_single = sufficient_count == 0

P.check("no isolated registered mechanism qualifies",
        no_single and not reservoir_sufficient and not causal_sufficient
        and not gauss_sufficient and not state_only,
        "reservoir=0 causal=0 gauss=0 state=0")
P.check("intact projector combination creates interaction verdict",
        interaction_without_isolated_component and mixed,
        "intact_projected=5 cells; components=0 and 4 cells")
P.check("run JSON carries the independently recomputed verdict",
        record["verdict"] == VERDICT
        and record["structural_valid"] is True
        and record["mechanisms"]["mixed_or_unresolved"] is mixed
        and record["mechanisms"]["no_registered_support_mechanism"] is no_single,
        record["verdict"])

json_arms = {arm["name"]: arm for arm in record["arms"]}
for arm in ARMS:
    P.check(f"JSON/CSV summary {arm}",
            json_arms[arm]["stable_runs"] == stable_runs[arm]
            and json_arms[arm]["passing_cells"] == passing_cells[arm]
            and json_arms[arm]["genesis_events"] == events[arm][0]
            and json_arms[arm]["evaporation_events"] == events[arm][1],
            f"stable={stable_runs[arm]} cells={passing_cells[arm]} "
            f"events={events[arm]}")

source = (ROOT / "engine/src/eft/ignition_cut_support_ablation.cpp").read_text(
    encoding="utf-8"
)
P.check("prefix cut and registered cells are source-fixed",
        "constexpr int PREFIX_TICKS = 150;" in source
        and "constexpr int FINAL_TICK = 300;" in source
        and "constexpr std::array<int, 2> VOLUMES{{24, 32}};" in source
        and "constexpr std::array<int, 3> AMPLITUDES{{12, 20, 40}};" in source,
        "no runtime parameter or scan")
P.check("field clear cannot mutate ternary state",
        "void zero_field(RenderBridge& bridge)" in source
        and "voxel.state" not in source[source.index("void zero_field"):
                                         source.index("void rebase_kinematics")],
        "zero_field writes field channels only")
P.check("production extension remains absent",
        "common_action_face_dynamics" not in source
        and "forces = true" not in source
        and "movement = true" not in source,
        "observer/campaign only")

raise SystemExit(0 if P.report() else 1)
