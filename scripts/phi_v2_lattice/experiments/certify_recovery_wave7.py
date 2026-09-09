"""Verify wave7 exact evidence and run its registered targeted regressions.

This assembly does not execute a central-memory or binding graph campaign.
It preserves software, exact-sector and physical-recovery dispositions.
"""
from __future__ import annotations

import argparse
from bisect import bisect_left
from collections import Counter, deque
from fractions import Fraction
import hashlib
import importlib.metadata
import json
from math import gcd, lcm
import os
from pathlib import Path
import platform
import subprocess
import sys
import time
import traceback

import numpy as np

from scripts.phi_v2_lattice import recovery_credit_exchange_q2 as Q2
from scripts.phi_v2_lattice.experiments.certify_recovery_wave6 import junit_counts

ROOT = Path(__file__).resolve().parents[3]
EVIDENCE = ROOT/"engine/docs/evidence/strict-recovery-wave7-2026-09-08"
BASELINE = ROOT/"engine/docs/evidence/strict-recovery-wave7-baseline-2026-09-08.json"
PREVIOUS = ROOT/"engine/docs/evidence/strict-recovery-wave6-2026-09-08/manifest.json"
TESTS = tuple("scripts/tests/phi_v2_lattice/"+name for name in (
    "test_credit_exchange_binding.py", "test_recovery_credit_exchange_q2.py", "test_recovery_full_memory.py",
    "test_full_memory_execution_v2.py",
    "test_recovery_wave7_evidence.py", "test_flux_binding.py", "test_recorded_matching.py",
    "test_balanced_matching.py", "test_hydro_parity.py", "test_recovery_micro_correlation.py",
    "test_recovery_momentum_memory.py"))
PYTEST_PLUGIN = "scripts.phi_v2_lattice.experiments.certify_recovery_wave7"
_DESELECTED, _RUNTIME_REPORTS = [], []


def pytest_deselected(items):
    _DESELECTED.extend(item.nodeid for item in items)


def pytest_collection_finish(session):
    path = os.environ.get("FTD_WAVE7_COLLECTION_RECORD")
    if path:
        Q2._write_json(Path(path), {"schema": "strict-wave7-pytest-collection-1",
            "nodes": [item.nodeid for item in session.items], "deselected": _DESELECTED})


def pytest_runtest_logreport(report):
    _RUNTIME_REPORTS.append({"nodeid": report.nodeid, "when": report.when, "outcome": report.outcome})


def pytest_sessionfinish(session, exitstatus):
    path = os.environ.get("FTD_WAVE7_REPORTS_RECORD")
    if path:
        Q2._write_json(Path(path), {"schema": "strict-wave7-pytest-reports-1",
            "exitstatus": int(exitstatus), "reports": _RUNTIME_REPORTS})


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def read(path):
    return json.loads(Path(path).read_bytes())


def hashes_match(root, hashes, required=()):
    root = Path(root).resolve()
    if not isinstance(hashes, dict) or not set(required).issubset(hashes):
        raise ValueError("missing required hash inventory")
    paths = []
    for name, expected in hashes.items():
        path = (root/name).resolve()
        if not path.is_relative_to(root) or not path.is_file() or sha(path) != expected:
            raise ValueError("changed or invalid frozen path: "+str(name))
        paths.append(path)
    return paths


def verify_census(directory):
    from scripts.phi_v2_lattice import recovery_full_memory as M
    directory = Path(directory)
    report, lock, tables = (read(directory/name) for name in ("report.json", "lock.json", "tables.json"))
    if (tables.get("schema") != "independent-full-local-tables-1"
            or tables.get("canonical_adoption") is not False or tables.get("complete_global_ensemble_enumerated") is not False
            or tables.get("actual_counts_equal_walsh_formula") is not True or tables.get("eligible_count") != 11740):
        raise ValueError("local table schema or scope differs")
    M.verify_independent_tables(tables)
    expected = {"lock.json", "tables.json"} | {f"ranges/{start:08d}.json" for start in range(0, 1 << 24, 1 << 18)}
    if set(report.get("artifact_sha256", {})) != expected:
        raise ValueError("incomplete local census artifacts")
    hashes_match(directory, report["artifact_sha256"])
    required_sources = {str((ROOT/name).resolve()) for name in (
        "engine/docs/CONTRACT_STRICT_RECOVERY_WAVE7_V1.md", "scripts/phi_v2_lattice/hydro_parity.py",
        "engine/docs/evidence/strict-recovery-wave7-2026-09-08/audit_full_local_tables.py")}
    hashes_match(ROOT, lock.get("source_sha256", {}), required_sources)
    if (report.get("schema") != "independent-full-local-table-result-1"
            or report.get("passed") is not True or report.get("source_drift") != [] or report.get("failure") is not None
            or report.get("local_inputs_checked") != 1 << 24 or report.get("eligible_count") != 11740
            or report.get("group_tables") != 324 or report.get("actual_counts_equal_walsh_formula") is not True
            or not 0 <= report.get("elapsed_seconds", -1) <= 300
            or lock.get("schema") != "independent-full-local-table-lock-1"
            or lock.get("workers") != 32 or lock.get("chunk") != 1 << 18 or lock.get("local_inputs") != 1 << 24
            or lock.get("ranges") != 64):
        raise ValueError("local census disposition or domain differs")
    total = [[0]*len(table["counts"]) for table in tables["joint_tables"]]
    if len(total) != 324 or sum(map(len, total)) != 2304:
        raise ValueError("local table dimensions differ")
    eligible, coverage = [], []
    for start in range(0, 1 << 24, 1 << 18):
        stop = start+(1 << 18)
        row = read(directory/f"ranges/{start:08d}.json")
        if row.get("start") != start or row.get("stop") != stop or row.get("checked") != stop-start or row.get("status") != "PASS":
            raise ValueError("incomplete local input range")
        coverage.append([start, stop])
        if len(row["tables"]) != len(total):
            raise ValueError("local range table count differs")
        for destination, values in zip(total, row["tables"]):
            if len(values) != len(destination) or any(type(x) is not int or x < 0 for x in values) or sum(values) != stop-start:
                raise ValueError("local range integer counts invalid")
            for i, value in enumerate(values):
                destination[i] += value
        if any(type(mask) is not int or not start <= mask < stop for mask in row["eligible"]):
            raise ValueError("eligible mask outside enumerated range")
        eligible.extend(row["eligible"])
    if len(coverage) != 64 or eligible != tables["eligible_masks"] or len(eligible) != len(set(eligible)):
        raise ValueError("local census coverage or eligible input provenance differs")
    if len(eligible) != 11740 or total != [table["counts"] for table in tables["joint_tables"]]:
        raise ValueError("local tables differ from actual range counts")
    return {"local_inputs_checked": 1 << 24, "group_tables": 324, "integer_counts": 2304,
            "eligible_count": 11740, "report_sha256": sha(directory/"report.json"), "tables_sha256": sha(directory/"tables.json")}


def artifact_graph_facts(arrays):
    """Derive graph claims from retained arrays, without executing the law."""
    n, rcount = Q2.STATE_COUNT, Q2.RELATIVE_COUNT
    required = {"successor": ("<i4", (n,)), "plus_move": ("i1", (n, 3)), "minus_move": ("i1", (n, 3)),
                "operation": ("u1", (n,)), "translating_family": ("bool", (n,)), "path_length": ("u1", (n,)),
                "component": ("<i4", (n,)), "transient_to_cycle": ("<i4", (n,)),
                "first_translating_family": ("<i4", (n,)), "relative_records": ("u1", (rcount, 10)),
                "first_adjacency": ("<i4", (n,)), "fresh_phase1_R2": ("bool", (n,))}
    if set(arrays) != set(required):
        raise ValueError("binding requires exactly twelve registered arrays")
    for name, (dtype, shape) in required.items():
        if arrays[name].dtype != np.dtype(dtype) or arrays[name].shape != shape:
            raise ValueError("binding array dtype/shape differs: "+name)
    relative = np.full((rcount, 10), 255, dtype=np.uint8)
    for i, record in enumerate(Q2.records()):
        relative[i, 0] = len(record.path)
        relative[i, 1:1+len(record.path)] = record.path
        relative[i, 3:] = (*record.velocities, *record.credits, *record.attempted,
                           sum(abs(x) for x in Q2.endpoint(record.path)))
    if not np.array_equal(relative, arrays["relative_records"]):
        raise ValueError("binding complete record chart differs")
    nodes = np.arange(n)
    stage, color_index, record_index = nodes//(8*rcount), nodes//rcount % 8, nodes % rcount
    R = relative[record_index, 0]
    flags_zero = np.all(relative[record_index, 7:9] == 0, axis=1)
    heading = relative[record_index, 1] ^ (relative[record_index, 5] == 0)
    target = (stage == 1) & (R == 1) & flags_zero & (relative[record_index, 3] == heading) & (relative[record_index, 4] == heading)
    fresh = (stage == 1) & (R == 2) & flags_zero
    for name, expected in (("path_length", R), ("translating_family", target), ("fresh_phase1_R2", fresh)):
        if not np.array_equal(arrays[name], expected):
            raise ValueError("binding stored predicate differs from complete records: "+name)
    successor = arrays["successor"]
    if np.any(successor < 0) or np.any(successor >= n) or not np.array_equal(stage[successor], (stage+1) % 19):
        raise ValueError("binding successor or physical phase invalid")
    plus, minus = arrays["plus_move"], arrays["minus_move"]
    for moves in (plus, minus):
        if np.any(np.abs(moves.astype(np.int16)).sum(axis=1) > 1) or np.any(moves[stage < 7] != 0):
            raise ValueError("binding unwrapped moves exceed the one-hop stage")
    colors = np.asarray(Q2.COLORS, dtype=np.int8)
    if not np.array_equal(colors[color_index[successor]], (colors[color_index]+plus) % 2):
        raise ValueError("binding matching color does not follow the positive carrier")
    endpoints = np.asarray([Q2.endpoint(record.path) for record in Q2.records()], dtype=np.int8)
    if not np.array_equal(endpoints[record_index[successor]], endpoints[record_index]+minus-plus):
        raise ValueError("binding path and unwrapped displacements disagree")
    if np.any(arrays["operation"] > 7):
        raise ValueError("binding operation code outside registered alphabet")
    degree = np.bincount(successor, minlength=n)
    remaining = degree.copy()
    queue = deque(map(int, np.flatnonzero(remaining == 0)))
    while queue:
        node = queue.popleft()
        other = int(successor[node])
        remaining[other] -= 1
        if remaining[other] == 0:
            queue.append(other)
    recurrent = remaining != 0
    component = np.full(n, -1, dtype=np.int32)
    cycles = []
    for start in np.flatnonzero(recurrent):
        node = int(start)
        if component[node] >= 0:
            continue
        cycle = []
        while component[node] < 0:
            component[node] = len(cycles)
            cycle.append(node)
            node = int(successor[node])
        if node != cycle[0]:
            raise ValueError("invalid recurrent decomposition")
        cycles.append(cycle)
    parents = np.argsort(successor, kind="stable")
    offsets = np.concatenate(([0], np.cumsum(degree)))
    def distances(mask, propagate_components=False):
        result = np.full(n, -1, dtype=np.int32)
        result[mask] = 0
        queue = deque(map(int, np.flatnonzero(mask)))
        while queue:
            node = queue.popleft()
            for raw in parents[offsets[node]:offsets[node+1]]:
                parent = int(raw)
                if result[parent] < 0:
                    result[parent] = result[node]+1
                    if propagate_components:
                        component[parent] = component[node]
                    queue.append(parent)
        return result
    transient = distances(recurrent, True)
    first_target, first_adjacent = distances(target), distances(R <= 1)
    for name, expected in (("component", component), ("transient_to_cycle", transient),
                           ("first_translating_family", first_target), ("first_adjacency", first_adjacent)):
        if not np.array_equal(arrays[name], expected):
            raise ValueError("binding graph classification differs: "+name)
    basin = np.bincount(component, minlength=len(cycles))
    rows = []
    for i, cycle in enumerate(cycles):
        dp, dm = (tuple(map(int, moves[cycle].sum(axis=0))) for moves in (plus, minus))
        if dp != dm or any(d % 2 for d in dp):
            raise ValueError("binding cycle translations differ from complete recurrence")
        rows.append({"representative": min(cycle), "length": len(cycle), "translation": list(dp),
                     "drift": [str(Fraction(d, len(cycle))) for d in dp],
                     "translating_family_states": int(target[cycle].sum()),
                     "restoring_transport": bool(any(dp) and np.any(target[cycle])),
                     "R_histogram": {str(k): v for k, v in Counter(map(int, R[cycle])).items()},
                     "spatial_records_and_phase_period_L6": len(cycle)*lcm(*(6//gcd(6, abs(d)) for d in dp)),
                     "nodes_sha256": hashlib.sha256(np.asarray(cycle, dtype="<i4").tobytes()).hexdigest(),
                     "basin_states": int(basin[i])})
    return {"states": n, "relative_records_with_attempts": rcount, "components": len(cycles), "cycles": rows,
            "recurrent_states": int(recurrent.sum()), "translating_family_states": int(target.sum()),
            "cycle_length_histogram": {str(k): v for k, v in Counter(len(c) for c in cycles).items()},
            "maximum_transient": int(transient.max()), "maximum_first_target": int(first_target.max()),
            "first_target_histogram": {str(k): v for k, v in Counter(map(int, first_target)).items()},
            "unrestored_states": int(np.count_nonzero(first_target < 0)),
            "all_translating_family_states_recurrent": bool(np.all(recurrent[target])),
            "global95_tick_gate": bool(np.all(first_target >= 0) and first_target.max() <= 95),
            "registered_cycle_prediction": bool(len(cycles) == 48 and recurrent.sum() == 1824 and
                all(row["length"] == 38 and sum(abs(d) for d in row["translation"]) == 2 and row["restoring_transport"] for row in rows)),
            "fresh_phase1_R2_states": int(fresh.sum()), "fresh_R2_maximum_family_hit": int(first_target[fresh].max()),
            "fresh_R2_maximum_adjacency_hit": int(first_adjacent[fresh].max()),
            "fresh_R2_76_tick_gate": bool(np.all((first_target[fresh] >= 0) & (first_target[fresh] <= 76))),
            "fresh_R2_38_tick_adjacency_gate": bool(np.all((first_adjacent[fresh] >= 0) & (first_adjacent[fresh] <= 38)))}


def verify_binding(directory):
    directory = Path(directory)
    report, lock, graph = (read(directory/name) for name in ("report.json", "lock.json", "graph.json"))
    if (graph.get("schema") != "credit-exchange-q2-graph-1" or graph.get("canonical_adoption") is not False
            or graph.get("physical_matter_identified") is not False):
        raise ValueError("binding graph schema or physical scope differs")
    expected = {"lock.json", "graph.json", "coherent-preparations.json", "transitions.npz"}
    expected |= {f"runtime-ranges/{start:07d}.json" for start in range(0, Q2.STATE_COUNT, Q2.RELATIVE_COUNT)}
    if set(report.get("artifact_sha256", {})) != expected:
        raise ValueError("incomplete binding artifact inventory")
    hashes_match(directory, report["artifact_sha256"])
    source_paths = hashes_match(ROOT, lock.get("source_sha256", {}))
    receipt = EVIDENCE/"binding-execution-acceptance-v1.json"
    accepted = Q2._acceptance(ROOT, receipt, lock.get("acceptance_receipt_sha256"), lock.get("runtime_sha256"))
    if not set(accepted).issubset(source_paths):
        raise ValueError("binding campaign omitted accepted source pins")
    ranges = [[start, start+Q2.RELATIVE_COUNT] for start in range(0, Q2.STATE_COUNT, Q2.RELATIVE_COUNT)]
    if (report.get("schema") != "credit-exchange-q2-execution-result-1"
            or lock.get("schema") != "credit-exchange-q2-execution-lock-1"
            or any(lock.get(k) != v for k, v in {"law_id": Q2.LAW_ID, "workers": 32, "full_runtime_L": 6,
                    "states": Q2.STATE_COUNT, "ranges": ranges, "coherent_instances": 262656,
                    "intended_resource_ceiling_seconds": 1800}.items())
            or report.get("law_id") != Q2.LAW_ID or report.get("canonical_adoption") is not False
            or report.get("physical_matter_identified") is not False or report.get("engineering_complete") is not True
            or report.get("source_drift") != [] or report.get("failure") is not None
            or report.get("resource_ceiling_met") is not True or not 0 <= report.get("elapsed_seconds", -1) <= 1800
            or report.get("runtime_states_checked") != Q2.STATE_COUNT
            or report.get("complete_arrays_and_ordinal_equal") is not True or report.get("observed_quotient_equal") is not True):
        raise ValueError("binding runtime or execution disposition incomplete")
    if report.get("post_run_source_sha256") != lock.get("source_sha256") or len(report.get("ranges", [])) != 152:
        raise ValueError("binding source/range evidence differs")
    for row, (start, stop) in zip(report["ranges"], ranges):
        if (row.get("schema") != "credit-exchange-q2-runtime-range-1"
                or row.get("start") != start or row.get("stop") != stop or row.get("completed_stop") != stop
                or row.get("checked") != stop-start or row.get("L") != 6 or row.get("passed") is not True
                or row.get("failure") is not None or row.get("runtime_sha256") != lock["runtime_sha256"]
                or len(row.get("complete_outputs_sha256", "")) != 64
                or read(directory/f"runtime-ranges/{start:07d}.json") != row
                or sum(value for name, value in row["operations"].items() if name.isdecimal()) != stop-start):
            raise ValueError("binding range coverage or diagnostic counts invalid")
    if graph.get("states") != Q2.STATE_COUNT or graph.get("coherent_instances") != 262656:
        raise ValueError("binding graph domain differs")
    with np.load(directory/"transitions.npz", allow_pickle=False) as archive:
        if set(archive.files) != set(graph["array_sha256"]):
            raise ValueError("binding array inventory differs")
        arrays = {name: archive[name] for name in archive.files}
    for name, array in arrays.items():
        if hashlib.sha256(array.tobytes()).hexdigest() != graph["array_sha256"][name]:
            raise ValueError("binding graph array digest differs")
    facts = artifact_graph_facts(arrays)
    if any(graph.get(key) != value for key, value in facts.items()):
        raise ValueError("binding graph summary differs from artifact-only reconstruction")
    if arrays["successor"].shape != (Q2.STATE_COUNT,) or np.any(arrays["successor"] < 0) or np.any(arrays["successor"] >= Q2.STATE_COUNT):
        raise ValueError("binding transition domain invalid")
    cases = read(directory/"coherent-preparations.json")["cases"]
    if len(cases) != 262656:
        raise ValueError("binding preparation census incomplete")
    expected_cases = Q2.preparations(arrays["component"], facts["cycles"], arrays["first_translating_family"], arrays["first_adjacency"])
    if cases != expected_cases:
        raise ValueError("binding preparations differ from the registered complete family")
    baseline_count, kicks = 0, 0
    for i, row in enumerate(cases):
        node = row["node"]
        if row["baseline_id"] != i//6 or not 0 <= node < Q2.STATE_COUNT or ((i % 6 == 0) != (row["kind"] == "baseline")):
            raise ValueError("binding preparation provenance invalid")
        baseline_count += row["kind"] == "baseline"
        kicks += row["kind"] != "baseline"
        component = int(arrays["component"][node])
        cycle = graph["cycles"][component]
        if (row["component"] != component or row["first_translating_family"] != int(arrays["first_translating_family"][node])
                or row["first_adjacency"] != int(arrays["first_adjacency"][node]) or row["drift"] != cycle["drift"]
                or row["restoring_transport"] != cycle["restoring_transport"]):
            raise ValueError("binding preparation outcome differs from graph")
    if baseline_count != 43776 or kicks != 218880 or sum(c["basin_states"] for c in graph["cycles"]) != Q2.STATE_COUNT:
        raise ValueError("binding preparation/basin count differs")
    moving_kicks = [row for row in cases if row["baseline_in_translating_family"] and row["kind"] != "baseline"]
    preparation_facts = {
        "coherent_baselines": baseline_count, "coherent_instances": len(cases),
        "coherent_restoring_transport": all(row["restoring_transport"] for row in cases),
        "moving_family_kicks": len(moving_kicks),
        "moving_kick_maximum_family_hit": max(row["first_translating_family"] for row in moving_kicks),
        "moving_kick_maximum_adjacency_hit": max(row["first_adjacency"] for row in moving_kicks),
        "moving_kick76_tick_gate": all(0 <= row["first_translating_family"] <= 76 for row in moving_kicks),
        "moving_kick38_tick_adjacency_gate": all(0 <= row["first_adjacency"] <= 38 for row in moving_kicks),
        "perturbations_matching_baseline_drift": sum(row["same_drift_as_baseline"] for row in cases if row["kind"] != "baseline")}
    if len(moving_kicks) != 480 or any(graph.get(key) != value for key, value in preparation_facts.items()):
        raise ValueError("binding preparation predicates disagree with retained outcomes")
    gates = ("global95_tick_gate", "registered_cycle_prediction", "all_translating_family_states_recurrent",
             "coherent_restoring_transport", "moving_kick76_tick_gate", "moving_kick38_tick_adjacency_gate",
             "fresh_R2_76_tick_gate", "fresh_R2_38_tick_adjacency_gate")
    recovered = (graph.get("translating_family_states") == 96 and all(graph.get(gate) is True for gate in gates)
                 and all(c["spatial_records_and_phase_period_L6"] == 114 for c in graph["cycles"]))
    if report.get("registered_Q2_restoring_transport") is not recovered:
        raise ValueError("binding recovery disposition disagrees with fixed graph predicates")
    return {"states": Q2.STATE_COUNT, "components": graph["components"], "recurrent_states": graph["recurrent_states"],
            "maximum_first_target": graph["maximum_first_target"], "registered_Q2_restoring_transport": recovered,
            "registered_gates": {gate: graph[gate] for gate in gates}, "report_sha256": sha(directory/"report.json")}


def memory_ranges():
    return [(i*34462770//128, (i+1)*34462770//128) for i in range(128)]


def ordered_pairs(start, stop):
    # Independent diagonal/anti-diagonal indices in the registered triangular chart.
    n = 11740
    short = sorted(value for i in range(n//2) for value in (i*(n+1-i), i*(n+1-i)+n-1-2*i))
    return 4*(stop-start)-2*(bisect_left(short, stop)-bisect_left(short, start))


def verify_v2_terminals(directory, report, lock):
    """Revalidate all terminal mappings, including an incomplete V2 attempt."""
    from scripts.phi_v2_lattice.experiments import certify_full_memory_v2 as revision
    controls = [(0, 16)]
    for boundary, _ in memory_ranges()[1:]:
        controls.extend(((boundary-1, boundary), (boundary, boundary+1)))
    for kind, entries, key in (
        ("ranges", [(f"{start:08d}", start, stop) for start, stop in memory_ranges()], "range_receipts"),
        ("controls", [(f"{i:04d}", start, stop) for i, (start, stop) in enumerate(controls)], "control_receipts"),
    ):
        mapping = report.get(key)
        if not isinstance(mapping, dict) or set(mapping) != {name for name, start, stop in entries}:
            raise ValueError("V2 terminal map incomplete: "+key)
        for name, start, stop in entries:
            relative = mapping[name]
            if relative not in (f"{kind}/{name}.json", f"{kind}/{name}.recovery.json"):
                raise ValueError("V2 terminal path outside registered alternatives")
            row = revision._valid_final(Path(directory)/relative, kind, start, stop, lock)
            if relative.endswith(".recovery.json"):
                if row["status"] != "FAIL":
                    raise ValueError("V2 recovery receipt cannot promote success")
                normal = Path(directory)/kind/(name+".json")
                if not normal.is_file():
                    raise ValueError("V2 recovery lacks its retained invalid normal receipt")
                try:
                    revision._valid_final(normal, kind, start, stop, lock)
                except Exception:
                    original = row.get("invalid_normal_receipt", {})
                    if original.get("path") != f"{kind}/{name}.json" or original.get("sha256") != sha(normal):
                        raise ValueError("V2 recovery does not identify retained invalid bytes")
                else:
                    raise ValueError("V2 recovery masks a valid normal final")


def verify_memory(directory):
    """Reassemble retained exact totals; never enumerate central pairs here."""
    from scripts.phi_v2_lattice import recovery_full_memory as M
    from scripts.phi_v2_lattice.experiments import certify_full_memory as execution
    directory = Path(directory)
    report, lock = (read(directory/name) for name in ("report.json", "lock.json"))
    v2 = report.get("schema") == "strict-full-memory-execution-v2-result-1"
    result_schema = "strict-full-memory-execution-v2-result-1" if v2 else "strict-full-memory-execution-result-1"
    lock_schema = "strict-full-memory-execution-v2-lock-1" if v2 else "strict-full-memory-execution-lock-1"
    if (report.get("schema") != result_schema
            or report.get("canonical_adoption") is not False
            or lock.get("schema") != lock_schema
            or lock.get("workers") != 32 or lock.get("ranges") != [list(pair) for pair in memory_ranges()]
            or lock.get("memory_limit_bytes") != 8*1024**3 or lock.get("ceiling_seconds") != 1800
            or lock.get("canonical_adoption") is not False or lock.get("central_second_complete_method") is not False
            or report.get("lock_sha256") != sha(directory/"lock.json")):
        raise ValueError("full-memory lock/domain invalid")
    artifacts = report.get("artifact_sha256", {})
    actual_files = {p.relative_to(directory).as_posix() for p in directory.rglob("*")
                    if p.is_file() and p.name != "report.json"}
    if set(artifacts) != actual_files:
        raise ValueError("full-memory retained artifact inventory incomplete")
    hashes_match(directory, artifacts)
    hashes_match(ROOT, report.get("source_sha256", {}))
    if report.get("source_sha256") != lock.get("source_sha256"):
        raise ValueError("full-memory source map changed")
    # This accepted-input function verifies metadata and local artifacts only.
    # Its contract never launches a central control or full range.
    census = lock["independent_census"]
    if v2:
        from scripts.phi_v2_lattice.experiments import certify_full_memory_v2 as revision
        accepted = revision.accepted_inputs(lock["prepared"], lock["binary"], lock["binary_sha256"],
                    lock["v1_acceptance"], lock["v1_acceptance_sha256"], lock["acceptance"], lock["acceptance_sha256"],
                    census["receipt"], census["receipt_sha256"])
    else:
        accepted = execution.accepted_inputs(lock["prepared"], lock["binary"], lock["binary_sha256"],
                                             lock["acceptance"], lock["acceptance_sha256"], census["receipt"], census["receipt_sha256"])
    if any(lock.get(key) != value for key, value in accepted.items()):
        raise ValueError("memory execution lock differs from full accepted identities")
    execution._build_identity(Path(lock["binary"]), lock["binary_sha256"])
    if report.get("path_sha256") != lock.get("path_sha256"):
        raise ValueError("memory retained identity map differs from lock")
    for name, expected in lock["path_sha256"].items():
        if not Path(name).is_absolute() or sha(name) != expected:
            raise ValueError("memory retained identity changed")
    if v2:
        verify_v2_terminals(directory, report, lock)
    rows = []
    for start, stop in memory_ranges():
        if v2:
            mapping = report.get("range_receipts", {})
            if set(mapping) != {f"{a:08d}" for a, b in memory_ranges()}:
                raise ValueError("V2 terminal range map incomplete")
            name = mapping[f"{start:08d}"]
            if name not in (f"ranges/{start:08d}.json", f"ranges/{start:08d}.recovery.json"):
                raise ValueError("V2 terminal range path outside registered alternatives")
            row = read(directory/name)
        else:
            row = read(directory/f"ranges/{start:08d}.json")
        if row.get("schema") != "strict-full-memory-range-receipt-1" or row.get("start") != start or row.get("stop") != stop:
            raise ValueError("missing or incorrect full-memory range identity")
        if row.get("binary_sha256") != lock["binary_sha256"] or row.get("job_sha256") != lock["job_sha256"]:
            raise ValueError("central range executed wrong job or binary")
        checked, native = row.get("checked"), row.get("native")
        if type(checked) is not int or not 0 <= checked <= stop-start or row.get("status") not in ("PASS", "FAIL"):
            raise ValueError("central partial count or disposition invalid")
        if native is None:
            if checked != 0 or row["status"] == "PASS":
                raise ValueError("central range claims work without a native receipt")
        else:
            execution.validate_progress(native, start, stop)
            if checked != native["checked"] or (row["status"] == "PASS" and native["status"] != "PASS"):
                raise ValueError("central partial count differs from native progress")
        rows.append(row)
    identity_count = len({str((ROOT/name).resolve()) for name in lock["source_sha256"]} | set(lock["path_sha256"]))
    identity = report.get("post_run_identity", {})
    if identity.get("status") != "PASS" or identity.get("drift") != [] or identity.get("checked_files") != identity_count:
        raise ValueError("full-memory post-run identities incomplete")
    if report.get("status") == "FULL_K1_INCOMPLETE":
        supervision = report.get("supervision") or {}
        if supervision.get("coordinator_alive_after_cleanup") is not False:
            raise ValueError("incomplete memory attempt lacks cleanup evidence")
        return {"full_K1_complete": False, "status": "FULL_K1_INCOMPLETE", "report_sha256": sha(directory/"report.json"),
                "retained_ranges": 128, "checked_orbits_including_partial": sum(row.get("checked", 0) for row in rows),
                "post_run_identity": identity, "supervision": supervision, "continuum_recovered": False}
    supervision = report.get("supervision", {})
    if (report.get("status") != "PASS" or supervision.get("exit_code") != 0 or supervision.get("failure") is not None
            or supervision.get("actual_job_assignment") is not True
            or not 0 <= supervision.get("elapsed_seconds", -1) <= 1800
            or supervision.get("ceiling_seconds") != 1800 or supervision.get("memory_limit_bytes") != 8*1024**3
            or not 0 < supervision.get("peak_aggregate_private_bytes", -1) <= 8*1024**3):
        raise ValueError("full-memory supervision did not pass its actual limits")
    totals, ordered = [0]*14, 0
    for row, (start, stop) in zip(rows, memory_ranges()):
        native = row.get("native", {})
        execution.validate_progress(native, start, stop)
        if (row.get("status") != "PASS" or row.get("checked") != stop-start or native.get("status") != "PASS"
                or native.get("checked") != stop-start or native.get("completed_stop") != stop
                or native.get("ordered_pairs") != ordered_pairs(start, stop)):
            raise ValueError("central range incomplete or misweighted")
        previous = None
        native_records = []
        for line in (directory/f"ranges/{start:08d}.stdout").read_bytes().splitlines():
            previous = execution.validate_progress(json.loads(line), start, stop, previous)
            native_records.append(previous)
        if previous != native:
            raise ValueError("central receipt differs from retained native output")
        if v2:
            if row.get("exit_code") != 0 or row.get("native_eof") is not True:
                raise ValueError("V2 successful range lacks process/EOF evidence")
            progress_paths = sorted((directory/f"ranges/{start:08d}/progress").glob("*.json"))
            if len(progress_paths) != len(native_records):
                raise ValueError("V2 append-only progress coverage differs from stdout")
            if type(row.get("progress_records")) is not int or row["progress_records"] != len(native_records):
                raise ValueError("V2 terminal progress count differs from stdout")
            for sequence, (path, native_record) in enumerate(zip(progress_paths, native_records)):
                progress = read(path)
                if (path.name != f"{sequence:06d}.json" or progress.get("schema") != "strict-full-memory-v2-progress-1"
                        or progress.get("sequence") != sequence or progress.get("native") != native_record
                        or progress.get("job_sha256") != lock["job_sha256"] or progress.get("binary_sha256") != lock["binary_sha256"]):
                    raise ValueError("V2 append-only progress identity or sequence differs")
        for suffix in ("stdout", "stderr"):
            if sha(directory/f"ranges/{start:08d}.{suffix}") != row[suffix+"_sha256"]:
                raise ValueError("central stream digest differs")
        ordered += native["ordered_pairs"]
        for i, numerator in enumerate(native["numerators"]):
            totals[i] += int(numerator)
    recorded = read(directory/"central-totals.json")
    if (ordered != 137827600 or recorded.get("ordered_pairs") != ordered or recorded.get("orbits") != 34462770
            or recorded.get("denominator_exponent") != 432 or recorded.get("numerators") != list(map(str, totals))):
        raise ValueError("central exact reduction differs from complete native ranges")
    controls = [(0, 16)]
    for boundary, _ in memory_ranges()[1:]:
        controls.extend(((boundary-1, boundary), (boundary, boundary+1)))
    for i, (start, stop) in enumerate(controls):
        if v2:
            mapping = report.get("control_receipts", {})
            if set(mapping) != {f"{j:04d}" for j in range(255)}:
                raise ValueError("V2 terminal control map incomplete")
            name = mapping[f"{i:04d}"]
            if name not in (f"controls/{i:04d}.json", f"controls/{i:04d}.recovery.json"):
                raise ValueError("V2 terminal control path outside registered alternatives")
            control = read(directory/name)
        else:
            control = read(directory/f"controls/{i:04d}.json")
        if (control.get("schema") != "strict-full-memory-control-receipt-1" or control.get("status") != "PASS"
                or control.get("start") != start or control.get("stop") != stop
                or control.get("job_sha256") != lock["job_sha256"] or control.get("binary_sha256") != lock["binary_sha256"]):
            raise ValueError("central reference control failed")
        expected_fields = {"start", "stop", "completed_stop", "checked", "ordered_pairs", "denominator_exponent", "numerators"}
        if set(control.get("expected", {})) != expected_fields:
            raise ValueError("central reference control expected record incomplete")
        expected = control["expected"]
        if read(directory/f"controls/{i:04d}.expected.json") != expected:
            raise ValueError("central control reference file differs from terminal receipt")
        if (expected["start"] != start or expected["stop"] != stop or expected["completed_stop"] != stop
                or expected["checked"] != stop-start or expected["ordered_pairs"] != ordered_pairs(start, stop)
                or expected["denominator_exponent"] != 432 or not isinstance(expected["numerators"], list)
                or len(expected["numerators"]) != 14):
            raise ValueError("central reference control domain or coefficients incomplete")
        native = None
        for line in (directory/f"controls/{i:04d}.stdout").read_bytes().splitlines():
            native = execution.validate_progress(json.loads(line), start, stop, native)
        if native is None or native["status"] != "PASS" or any(native.get(k) != v for k, v in control["expected"].items()):
            raise ValueError("central control did not match reference")
        if control.get("native") != native or control.get("exit_code") != 0:
            raise ValueError("central control lacks matching process/native evidence")
        for suffix in ("stdout", "stderr"):
            if sha(directory/f"controls/{i:04d}.{suffix}") != control[suffix+"_sha256"]:
                raise ValueError("central control stream digest differs")
    certificate = read(directory/"certificate.json")
    reproduced = M.certificate(M.assemble_delta(totals))
    if certificate != reproduced:
        raise ValueError("full-memory certificate differs from retained exact total reassembly")
    completion = read(directory/"worker-completion.json")
    if (completion != report.get("worker_completion") or completion.get("status") != "PASS"
            or completion.get("certificate_sha256") != sha(directory/"certificate.json")
            or completion.get("central_totals_sha256") != sha(directory/"central-totals.json")
            or certificate.get("schema") != "strict-full-memory-certificate-1"
            or certificate.get("canonical_adoption") is not False or certificate.get("continuum_recovered") is not False
            or certificate.get("uniform_memory_tail") is not False or certificate.get("central_second_complete_method") is not False):
        raise ValueError("full-memory completion or physical scope differs")
    identity_count = len({str((ROOT/name).resolve()) for name in lock["source_sha256"]} | set(lock["path_sha256"]))
    for evidence in (report, completion):
        identity = evidence.get("post_run_identity", {})
        if identity.get("status") != "PASS" or identity.get("drift") != [] or identity.get("checked_files") != identity_count:
            raise ValueError("full-memory post-run identities incomplete")
    if supervision.get("coordinator_alive_after_cleanup") is not False:
        raise ValueError("memory coordinator remained alive after cleanup")
    return {"full_K1_complete": True, "status": "PASS", "central_pair_orbits": 34462770,
            "ordered_pairs": ordered, "controls": 255, "delta_sites": len(certificate["delta"]),
            "labelled_K1_sites": len(certificate["labelled_K1"]), "Gamma": certificate["Gamma"],
            "momentum_eight_microtick_signed_error": certificate["momentum_eight_microtick_signed_error"],
            "report_sha256": sha(directory/"report.json"), "continuum_recovered": False}


def checkout():
    def git(*args):
        return subprocess.check_output(["git", *args], cwd=ROOT)
    return {"head": git("rev-parse", "HEAD").decode().strip(),
            "tracked_cleanup_diff_sha256": hashlib.sha256(git("diff", "--binary", "HEAD", "--")).hexdigest(),
            "status": git("status", "--short", "--untracked-files=all").decode().splitlines(),
            "index": git("diff", "--cached", "--name-status").decode().splitlines()}


def preserved_baseline(baseline):
    if sha(PREVIOUS) != baseline["prior_manifest_sha256"]:
        raise ValueError("prior manifest changed")
    previous = read(PREVIOUS)
    sources = hashes_match(ROOT, baseline["source_sha256"])
    prior_sources = hashes_match(ROOT, previous["source_sha256"])
    artifacts = hashes_match(PREVIOUS.parent, previous["artifact_sha256"])
    if len(sources) != 328 or len(prior_sources) != 328 or len(artifacts) != 120:
        raise ValueError("previous frozen inventory differs")
    if sha(PREVIOUS.parent/previous["source_archive"]) != previous["source_archive_sha256"]:
        raise ValueError("previous source archive changed")
    current = checkout()
    if current["head"] != previous["git_head"] or current["index"] != previous["staged_paths"]:
        raise ValueError("preexisting head/index changed")
    if current["tracked_cleanup_diff_sha256"] != baseline["tracked_cleanup_diff_sha256"]:
        raise ValueError("preexisting tracked cleanup changed")
    if not set(baseline["preexisting_status"]).issubset(current["status"]):
        raise ValueError("preexisting checkout entries changed")
    return sources+artifacts+[PREVIOUS, PREVIOUS.parent/previous["source_archive"], BASELINE], current


def input_inventory(baseline, binding, memory):
    paths, initial_checkout = preserved_baseline(baseline)
    new_sources = ["scripts/phi_v2_lattice/"+name for name in (
        "credit_exchange_binding.py", "recovery_credit_exchange_q2.py", "recovery_full_memory.py",
        "experiments/certify_full_memory.py", "experiments/certify_full_memory_v2.py",
        "experiments/full_memory_native_v1.cpp", "experiments/certify_recovery_wave7.py")]
    new_sources += list(TESTS)
    new_sources += ["engine/docs/"+name for name in (
        "CONTRACT_STRICT_RECOVERY_WAVE7_V1.md", "CONTRACT_STRICT_RECOVERY_WAVE7_VALIDATION_V1.md",
        "SPEC_STRICT_CREDIT_EXCHANGE_BINDING_V1.md", "DERIV_STRICT_CREDIT_EXCHANGE_Q2_V1.md",
        "AUDIT_STRICT_CREDIT_EXCHANGE_BINDING_V1.md", "AUDIT_STRICT_CREDIT_EXCHANGE_Q2_V1.md",
        "AUDIT_STRICT_CREDIT_EXCHANGE_Q2_RESULTS_V1.md", "SPEC_STRICT_FULL_MEMORY_EXECUTION_V1.md",
        "AUDIT_STRICT_FULL_MEMORY_EXECUTION_V1.md", "AUDIT_STRICT_FULL_MEMORY_RESULTS_V1.md",
        "SPEC_STRICT_FULL_MEMORY_EXECUTION_V2.md", "AUDIT_STRICT_FULL_MEMORY_EXECUTION_V2.md",
        "AUDIT_STRICT_FULL_MEMORY_RESULTS_V2.md")]
    paths.extend((ROOT/name).resolve() for name in new_sources)
    paths.extend(EVIDENCE/name for name in ("audit_full_local_tables.py", "binding-execution-acceptance-v1.json"))
    for directory in (EVIDENCE/"local-tables-attempt-1", Path(binding), Path(memory)):
        paths.extend(p.resolve() for p in directory.rglob("*") if p.is_file())
    memory_lock = read(Path(memory)/"lock.json")
    paths.extend(Path(memory_lock[key]).resolve() for key in ("acceptance", "binary"))
    paths.extend(p.resolve() for p in Path(memory_lock["prepared"]).rglob("*") if p.is_file())
    paths.extend(Path(name).resolve() for name in memory_lock.get("path_sha256", {}))
    # All accepted transitive source pins are included, not only named top-level modules.
    for directory in (Path(binding), Path(memory)):
        paths.extend((ROOT/name).resolve() for name in read(directory/"lock.json")["source_sha256"])
    resolved = sorted(set(p.resolve() for p in paths))
    if any(not p.is_file() for p in resolved):
        raise ValueError("required source, review or evidence input missing")
    return resolved, initial_checkout


def run(output, binding, memory):
    output, binding, memory = (Path(path).resolve() for path in (output, binding, memory))
    if output.exists():
        raise ValueError("existing validation attempt must be preserved")
    output.mkdir(parents=True)
    base_command = [sys.executable, "-m", "pytest", "-p", PYTEST_PLUGIN, "-o", "addopts=", *TESTS, "-q"]
    command = base_command+["--junitxml="+str(output/"pytest.xml")]
    collection_command = base_command+["--collect-only"]
    environment = os.environ.copy()
    overrides = {"PYTEST_ADDOPTS": "", "PYTEST_PLUGINS": "", "PYTEST_DISABLE_PLUGIN_AUTOLOAD": "1"}
    environment.update(overrides)
    pins = {}
    try:
        baseline = read(BASELINE)
        paths, initial = input_inventory(baseline, binding, memory)
        for path in paths:
            pins[str(path)] = sha(path)
        lock = {"schema": "strict-recovery-wave7-validation-lock-1", "source_and_evidence_sha256": pins,
                "command": command, "collection_command": collection_command, "environment_overrides": overrides,
                "checkout": initial, "binding": str(binding), "memory": str(memory),
                "python": sys.version, "platform": platform.platform(), "python_executable_sha256": sha(sys.executable),
                "package_versions": {name: importlib.metadata.version(name) for name in ("numpy", "pytest")},
                "canonical_adoption": False, "central_campaign_rerun": False}
        Q2._write_json(output/"lock.json", lock)
    except BaseException:
        rejected = {"schema": "strict-recovery-wave7-validation-result-1", "passed": False,
                    "status": "REJECTED_BEFORE_EXECUTION", "failure": traceback.format_exc(),
                    "partial_input_hashes": pins, "central_campaign_rerun": False, "canonical_adoption": False}
        Q2._write_json(output/"report.json", rejected)
        raise
    began = time.monotonic()
    evidence, test_counts, exit_code, failure = {}, {}, None, None
    try:
        evidence["local_census"] = verify_census(EVIDENCE/"local-tables-attempt-1")
        evidence["binding"] = verify_binding(binding)
        evidence["original_memory_attempt"] = verify_memory(EVIDENCE/"full-memory-attempt-1")
        evidence["memory"] = verify_memory(memory)
        Q2._write_json(output/"evidence-verification.json", evidence)
        collection_env = dict(environment, FTD_WAVE7_COLLECTION_RECORD=str(output/"collection.json"))
        collection_env.pop("FTD_WAVE7_REPORTS_RECORD", None)
        with (output/"collection.log").open("xb") as handle:
            collected = subprocess.run(collection_command, cwd=ROOT, env=collection_env, stdout=handle, stderr=subprocess.STDOUT)
            handle.flush()
            os.fsync(handle.fileno())
        collection = read(output/"collection.json")
        nodes = collection.get("nodes", [])
        if (collected.returncode != 0 or collection.get("schema") != "strict-wave7-pytest-collection-1"
                or collection.get("deselected") != [] or not nodes or len(nodes) != len(set(nodes))
                or {node.split("::", 1)[0].replace("\\", "/") for node in nodes} != set(TESTS)):
            raise ValueError("registered test collection incomplete or deselected")
        test_env = dict(environment, FTD_WAVE7_COLLECTION_RECORD=str(output/"pytest-collection.json"),
                        FTD_WAVE7_REPORTS_RECORD=str(output/"pytest-reports.json"))
        with (output/"pytest.log").open("xb") as handle:
            completed = subprocess.run(command, cwd=ROOT, env=test_env, stdout=handle, stderr=subprocess.STDOUT)
            handle.flush()
            os.fsync(handle.fileno())
        exit_code = completed.returncode
        test_counts, tests_valid = junit_counts(output/"pytest.xml")
        test_collection, execution_reports = read(output/"pytest-collection.json"), read(output/"pytest-reports.json")
        call_reports = [row for row in execution_reports.get("reports", []) if row.get("when") == "call"]
        if (exit_code != 0 or not tests_valid or test_counts["skipped"] != 0 or test_counts["tests"] != len(nodes)
                or test_collection != collection or execution_reports.get("schema") != "strict-wave7-pytest-reports-1"
                or execution_reports.get("exitstatus") != 0 or len(call_reports) != len(nodes)
                or {row.get("nodeid") for row in call_reports} != set(nodes)
                or any(row.get("outcome") != "passed" for row in execution_reports["reports"])):
            raise ValueError("registered targeted test matrix did not pass without skips")
        preserved_baseline(baseline)
    except BaseException:
        failure = traceback.format_exc()
    current, postcheck_failures = {}, []
    for name in pins:
        try:
            current[name] = sha(name)
        except BaseException:
            current[name] = None
            postcheck_failures.append({"path": name, "failure": traceback.format_exc()})
    drift = [name for name in pins if pins[name] != current[name]]
    try:
        final_checkout = checkout()
    except BaseException:
        final_checkout = None
        postcheck_failures.append({"check": "final checkout", "failure": traceback.format_exc()})
    checkout_equal = final_checkout == initial
    artifacts = {}
    try:
        for path in sorted(output.rglob("*")):
            if path.is_file():
                try:
                    artifacts[path.relative_to(output).as_posix()] = sha(path)
                except BaseException:
                    postcheck_failures.append({"path": str(path), "failure": traceback.format_exc()})
    except BaseException:
        postcheck_failures.append({"check": "output inventory", "failure": traceback.format_exc()})
    required_outputs = {"lock.json", "evidence-verification.json", "collection.log", "collection.json",
                        "pytest.log", "pytest.xml", "pytest-collection.json", "pytest-reports.json"}
    if set(artifacts) != required_outputs:
        postcheck_failures.append({"check": "complete output inventory", "missing": sorted(required_outputs-set(artifacts)),
                                   "extra": sorted(set(artifacts)-required_outputs)})
    passed = failure is None and not postcheck_failures and not drift and checkout_equal and exit_code == 0
    result = {"schema": "strict-recovery-wave7-validation-result-1", "passed": passed,
              "test_counts": test_counts, "pytest_exit_code": exit_code, "failure": failure,
              "elapsed_seconds": time.monotonic()-began, "input_count": len(pins), "source_drift": drift,
              "postcheck_failures": postcheck_failures,
              "source_and_evidence_sha256": current, "checkout_unchanged": checkout_equal,
              "checkout_after": final_checkout, "evidence": evidence,
              "canonical_adoption": False, "interacting_fluid_recovered": False,
              "full_physics_recovery": False,
              "artifact_sha256": artifacts}
    Q2._write_json(output/"report.json", result)
    print(json.dumps({key: value for key, value in result.items() if key not in
                      ("source_and_evidence_sha256", "artifact_sha256", "checkout_after")}), flush=True)
    if not passed:
        raise ValueError("wave7 integrated validation did not pass; retained report identifies the gate")
    return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True)
    parser.add_argument("--binding", required=True)
    parser.add_argument("--memory", required=True)
    arguments = parser.parse_args()
    run(arguments.output, arguments.binding, arguments.memory)
