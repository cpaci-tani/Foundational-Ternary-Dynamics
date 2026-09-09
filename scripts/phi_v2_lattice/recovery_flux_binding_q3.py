"""Exact Q=3 quotient and registered complete restoring-transport census.

All transitions are independent path/credit decisions, followed by a complete
array comparison to a separately frozen recorded-matching implementation.
"""
from __future__ import annotations

from collections import Counter, deque
from concurrent.futures import ProcessPoolExecutor, as_completed
from dataclasses import dataclass
from fractions import Fraction
from functools import lru_cache
import hashlib
from io import BytesIO
from itertools import permutations, product
import json
from math import gcd, lcm
from numbers import Integral
from pathlib import Path
import time
import traceback
import zipfile

import numpy as np

LAW_ID = "phi-flux-balanced-matching-candidate-1"
V = ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1))
COLORS = tuple(product(range(2), repeat=3))
AXES = tuple(permutations(range(3)))
STATE_COUNT = 746496
RELATIVE_COUNT = 7776
ARRAY_NAMES = ("direction", "credit", "flux", "matching_frame", "charge_frame")
# exchange row: kind, polarity, source dx/dy/dz, axis, q_before/q_after,
# k_before/k_after. kind:0 hold,1 extension,2 retraction,3 guide,4 credit cap.
EXCHANGE_COLUMNS = ("kind", "polarity", "source_x", "source_y", "source_z", "axis",
                    "flux_before", "flux_after", "credit_before", "credit_after")


def _integer(value, name, low=0, high=None):
    if (isinstance(value, bool) or not isinstance(value, Integral) or value < low
            or (high is not None and value > high)):
        raise ValueError(f"invalid {name}")
    return int(value)


@dataclass(frozen=True)
class Record:
    path: tuple[int, ...]
    velocities: tuple[int, int]
    credits: tuple[int, int]


@dataclass(frozen=True)
class QuotientState:
    phase: int
    color: tuple[int, int, int]
    record: Record


def validate(q):
    if type(q) is not QuotientState or type(q.record) is not Record:
        raise ValueError("expected exact Q=3 record")
    _integer(q.phase, "phase", high=11)
    if type(q.color) is not tuple or len(q.color) != 3:
        raise ValueError("invalid color tuple")
    for bit in q.color:
        _integer(bit, "color", high=1)
    r = q.record
    if type(r.path) is not tuple or not 1 <= len(r.path) <= 3:
        raise ValueError("Q=3 requires a path of length one through three")
    for d in r.path:
        _integer(d, "path direction", high=5)
    if any(a == (b ^ 1) for a, b in zip(r.path, r.path[1:])):
        raise ValueError("nonbacktracking path required")
    for name, values, maximum in (("velocity", r.velocities, 5), ("credit", r.credits, 1)):
        if type(values) is not tuple or len(values) != 2:
            raise ValueError(f"invalid {name} tuple")
        for v in values:
            _integer(v, name, high=maximum)
    if len(r.path) + sum(r.credits) != 3:
        raise ValueError("record is outside Q=3")


@lru_cache(maxsize=1)
def records():
    paths = tuple(word for length in (1, 2, 3) for word in product(range(6), repeat=length)
                  if not any(a == (b ^ 1) for a, b in zip(word, word[1:])))
    return tuple(Record(path, (vp, vm), (kp, km))
                 for path in paths for kp, km in product(range(2), repeat=2)
                 if kp + km == 3 - len(path)
                 for vp, vm in product(range(6), repeat=2))


@lru_cache(maxsize=1)
def _indices():
    return {r: i for i, r in enumerate(records())}


def decode(i):
    i = _integer(i, "node", high=STATE_COUNT-1)
    pc, r = divmod(i, RELATIVE_COUNT)
    phase, color = divmod(pc, 8)
    return QuotientState(phase, COLORS[color], records()[r])


def encode(q):
    q = _normalized(q)
    return (q.phase*8 + COLORS.index(q.color))*RELATIVE_COUNT + _indices()[q.record]


def endpoint(path):
    return tuple(sum(V[d][a] for d in path) for a in range(3))


def _normalized(q):
    """Every admitted Integral is converted before arithmetic with signed hops."""
    validate(q)
    return QuotientState(int(q.phase), tuple(map(int, q.color)),
                         Record(tuple(map(int, q.record.path)),
                                tuple(map(int, q.record.velocities)),
                                tuple(map(int, q.record.credits))))


def transition(q):
    """Exact independent one-tick rule with both displacement and exchange records."""
    q = _normalized(q)
    r = q.record
    slot, axis, parity = q.phase//6, (q.phase % 6)//2, q.phase % 2
    epsilon = 1 if slot == 0 else -1
    separation = endpoint(r.path)
    source = (0, 0, 0) if slot == 0 else separation
    active_color = (q.color[axis] + source[axis]) % 2
    matched = 2*axis + int(active_color != parity)
    v = r.velocities[slot]
    color, path = q.color, r.path
    velocities, credits = list(r.velocities), list(r.credits)
    shift_p = shift_m = (0, 0, 0)
    k = credits[slot]
    retract = path[0] if slot == 0 else path[-1] ^ 1
    matched_flux = epsilon*(1 if matched % 2 == 0 else -1) if matched == retract else 0
    exchange = (0, epsilon, *source, axis, matched_flux, matched_flux, k, k)
    if v == matched:
        eta = 1 if v % 2 == 0 else -1
        moved = False
        if v == retract:
            before = epsilon * eta
            if k == 0:
                path = path[1:] if slot == 0 else path[:-1]
                credits[slot] = 1
                moved = True
                exchange = (2, epsilon, *source, axis, before, 0, 0, 1)
            else:
                velocities[slot] = v ^ 1
                exchange = (4, epsilon, *source, axis, before, before, 1, 1)
        elif k == 1:
            path = (v ^ 1,) + path if slot == 0 else path + (v,)
            credits[slot] = 0
            moved = True
            exchange = (1, epsilon, *source, axis, 0, -epsilon*eta, 1, 0)
        else:
            velocities[slot] = retract
            exchange = (3, epsilon, *source, axis, 0, 0, 0, 0)
        if moved:
            if slot == 0:
                shift_p = V[v]
                color = tuple((color[a]+shift_p[a]) % 2 for a in range(3))
            else:
                shift_m = V[v]
    out = QuotientState((q.phase+1) % 12, color, Record(path, tuple(velocities), tuple(credits)))
    validate(out)
    if endpoint(out.record.path) != tuple(separation[a]+shift_m[a]-shift_p[a] for a in range(3)):
        raise AssertionError("relative displacement lost")
    if exchange[0] in (1, 2) and exchange[6]**2 + exchange[8] != exchange[7]**2 + exchange[9]:
        raise AssertionError("field/credit exchange lost")
    return out, shift_p, shift_m, exchange


def materialize(q, L=8, positive_position=None, microtick=None, frame_permutation=0):
    """Exact external preparation; construct every background record explicitly."""
    from . import balanced_matching as runtime
    q = _normalized(q)
    L = _integer(L, "periodic size", low=8)
    if L % 2:
        raise ValueError("even periodic size required")
    frame_permutation = _integer(frame_permutation, "axis ordering", high=5)
    axes = AXES[frame_permutation]
    point = q.color if positive_position is None else positive_position
    if not isinstance(point, (tuple, list)) or len(point) != 3:
        raise ValueError("three position coordinates required")
    point = tuple(_integer(x, "position", high=L-1) for x in point)
    tick = q.phase if microtick is None else _integer(microtick, "ordinal")
    if tick % 12 != q.phase:
        raise ValueError("ordinal/phase mismatch")
    direction = np.zeros((L**3, 2), dtype=np.uint8)
    credit = np.zeros_like(direction)
    flux = np.zeros((L**3, 3), dtype=np.int8)
    origin_color = tuple(q.color[a] ^ (point[axes[a]] % 2) for a in range(3))
    background = np.fromiter((8*frame_permutation + sum(
        ((coords[axes[a]] % 2) ^ origin_color[a]) << a for a in range(3))
        for coords in product(range(L), repeat=3)), dtype=np.uint8, count=L**3)
    def index(p):
        return (p[0]*L+p[1])*L+p[2]
    def physical(d):
        return 2*axes[d//2]+d % 2
    current = point
    for d in q.record.path:
        d = physical(d)
        target = tuple((current[a]+V[d][a]) % L for a in range(3))
        owner = current if d % 2 == 0 else target
        axis = d//2
        if flux[index(owner), axis]:
            raise AssertionError("path edge reused")
        flux[index(owner), axis] = 1 if d % 2 == 0 else -1
        current = target
    for slot, p in enumerate((point, current)):
        direction[index(p), slot] = physical(q.record.velocities[slot])+1
        credit[index(p), slot] = q.record.credits[slot]
    charge_frame = np.zeros(L**3, dtype=np.uint8)
    return runtime.MatchingState(L, tick, direction, credit, flux, background, charge_frame)


def observe(state):
    """Recover the quotient from full arrays and actual local matching record."""
    from . import balanced_matching as runtime
    runtime.validate(state)
    if state.L < 8:
        raise ValueError("Q=3 observation requires even L>=8")
    if np.any(state.charge_frame):
        raise ValueError("this Q=3 chart is the explicit eta-zero sector")
    positions = [np.flatnonzero(state.direction[:, s]) for s in range(2)]
    if any(len(p) != 1 for p in positions) or runtime.account_units(state) != 3:
        raise ValueError("observation requires one carrier of each polarity and Q=3")
    plus, minus = (int(p[0]) for p in positions)
    L = int(state.L)
    def point(i):
        return (i//(L*L), (i//L) % L, i % L)
    def shift(i, d):
        p = tuple((x+v) % L for x, v in zip(point(i), V[d]))
        return (p[0]*L+p[1])*L+p[2]
    frame = int(state.matching_frame[plus])
    axes = AXES[frame//8]
    def internal(d):
        return 2*axes.index(d//2)+d % 2
    current, path = plus, []
    while current != minus:
        ports = []
        for d in range(6):
            owner = current if d % 2 == 0 else shift(current, d)
            outward = int(state.flux[owner, d//2])*(1 if d % 2 == 0 else -1)
            if outward == 1:
                ports.append(d)
        if len(ports) != 1 or len(path) >= 3:
            raise ValueError("not a unique short unit-flow path")
        path.append(internal(ports[0]))
        current = shift(current, ports[0])
    q = QuotientState(int(state.microtick % 12), tuple((frame >> a) & 1 for a in range(3)),
                      Record(tuple(path), tuple(internal(int(state.direction[p, s])-1)
                             for s, p in enumerate((plus, minus))),
                             tuple(int(state.credit[p, s]) for s, p in enumerate((plus, minus)))))
    validate(q)
    return q


def verify_runtime_range(start, stop, runtime_sha256, L=8):
    from . import balanced_matching as runtime
    start = _integer(start, "start", high=STATE_COUNT)
    stop = _integer(stop, "stop", low=start, high=STATE_COUNT)
    if runtime.LAW_ID != LAW_ID or hashlib.sha256(Path(runtime.__file__).read_bytes()).hexdigest() != runtime_sha256:
        raise ValueError("recorded-matching source/law differs from registered input")
    digest = hashlib.sha256()
    for i in range(start, stop):
        q = decode(i)
        before_state = materialize(q, L)
        before = tuple(getattr(before_state, name).tobytes() for name in ARRAY_NAMES)
        next_q, shift_p, _, _ = transition(q)
        actual, _ = runtime.step(before_state)
        point = tuple((q.color[a]+shift_p[a]) % L for a in range(3))
        expected = materialize(next_q, L, point, q.phase+1)
        if actual.microtick != expected.microtick or observe(actual) != next_q:
            raise AssertionError(f"ordinal/observed quotient mismatch at node {i}")
        for name, original in zip(ARRAY_NAMES, before):
            if not np.array_equal(getattr(actual, name), getattr(expected, name)):
                raise AssertionError(f"complete {name} mismatch at node {i}")
            if getattr(before_state, name).tobytes() != original:
                raise AssertionError(f"input mutation at node {i}")
            if np.shares_memory(getattr(actual, name), getattr(before_state, name)):
                raise AssertionError(f"input/output alias at node {i}")
            digest.update(getattr(actual, name).tobytes())
        digest.update(bytes((actual.microtick,)))
    return {"start": start, "stop": stop, "checked": stop-start, "L": L,
            "complete_outputs_sha256": digest.hexdigest()}


def _first_hit(successors, cycles, removed, target):
    hit = np.full(STATE_COUNT, -1, dtype=np.int32)
    for cycle in cycles:
        positions = [i for i, node in enumerate(cycle) if target[node]]
        if positions:
            # Two backward traversals give all cyclic distances without O(T^2).
            distance = -1
            for k in range(2*len(cycle)-1, -1, -1):
                node = cycle[k % len(cycle)]
                distance = 0 if target[node] else (distance+1 if distance >= 0 else -1)
                if k < len(cycle):
                    hit[node] = distance
    for i in reversed(removed):
        following = int(hit[int(successors[i])])
        hit[i] = 0 if target[i] else (following+1 if following >= 0 else -1)
    return hit


def _prepared_cases(component, cycle_rows, first_R1, first_adjacent, successors, R):
    rows = []
    baseline_id = 0
    for phase, color, u, heading in product(range(12), COLORS, range(6), range(6)):
        q = QuotientState(phase, color, Record((u,), (heading, heading), (1, 1)))
        base_node = encode(q)
        base_component = int(component[base_node])
        base_drift = cycle_rows[base_component]["drift"]
        cases = [("baseline", None, q)]
        for slot in range(2):
            inward = u if slot == 0 else u ^ 1
            for d in range(6):
                if d == inward:
                    continue
                path = (d ^ 1, u) if slot == 0 else (u, d)
                c = tuple((color[a]+V[d][a]) % 2 for a in range(3)) if slot == 0 else color
                credits = (0, 1) if slot == 0 else (1, 0)
                cases.append(("plus_departure" if slot == 0 else "minus_departure", d,
                              QuotientState(phase, c, Record(path, (heading, heading), credits))))
        for kind, direction, prepared in cases:
            node = encode(prepared)
            c = int(component[node])
            cycle = cycle_rows[c]
            rows.append({"baseline_id": baseline_id, "kind": kind, "direction": direction,
                         "node": node, "component": c, "first_R1": int(first_R1[node]),
                         "first_adjacency": int(first_adjacent[node]),
                         "initial_R": int(R[node]), "initial_distance": sum(abs(v) for v in endpoint(prepared.record.path)),
                         "restoring_transport": cycle["restoring_transport"], "drift": cycle["drift"],
                         "same_drift_as_baseline": cycle["drift"] == base_drift})
        baseline_id += 1
    if baseline_id != 3456 or len(rows) != 38016:
        raise AssertionError("registered coherent family incomplete")
    return rows


def graph():
    """Construct every finite transition and classify every recurrent basin."""
    rs = records()
    if len(rs) != RELATIVE_COUNT or len(set(rs)) != RELATIVE_COUNT:
        raise AssertionError("relative census mismatch")
    successor = np.empty(STATE_COUNT, dtype="<i4")
    plus_move = np.zeros((STATE_COUNT, 3), dtype=np.int8)
    minus_move = np.zeros_like(plus_move)
    exchange = np.zeros((STATE_COUNT, len(EXCHANGE_COLUMNS)), dtype=np.int8)
    R = np.empty(STATE_COUNT, dtype=np.uint8)
    separation = np.empty((STATE_COUNT, 3), dtype=np.int8)
    for i in range(STATE_COUNT):
        q = decode(i)
        out, dp, dm, work = transition(q)
        successor[i] = encode(out)
        plus_move[i], minus_move[i], exchange[i] = dp, dm, work
        R[i], separation[i] = len(q.record.path), endpoint(q.record.path)
    distances = np.abs(separation).sum(axis=1).astype(np.uint8)
    if not np.all((1 <= distances) & (distances <= R) & (R <= 3)):
        raise AssertionError("all-future distance bound failed")
    indegree = np.bincount(successor, minlength=STATE_COUNT).astype(np.int32)
    queue = deque(int(i) for i in np.flatnonzero(indegree == 0))
    removed = []
    while queue:
        i = queue.popleft()
        removed.append(i)
        j = int(successor[i])
        indegree[j] -= 1
        if indegree[j] == 0:
            queue.append(j)
    visited = np.zeros(STATE_COUNT, dtype=np.bool_)
    component = np.full(STATE_COUNT, -1, dtype="<i4")
    transient = np.zeros(STATE_COUNT, dtype="<i4")
    cycles, cycle_rows = [], []
    for initial in np.flatnonzero(indegree):
        i = int(initial)
        if visited[i]:
            continue
        cycle = []
        while not visited[i]:
            visited[i] = True
            component[i] = len(cycles)
            cycle.append(i)
            i = int(successor[i])
        if i != cycle[0] or len(cycle) % 12:
            raise AssertionError("phase-aware cycle decomposition failed")
        translation = tuple(int(x) for x in plus_move[cycle].sum(axis=0))
        other = tuple(int(x) for x in minus_move[cycle].sum(axis=0))
        if translation != other or any(x % 2 for x in translation):
            raise AssertionError("carrier translations or background colors disagree")
        minimum = [j for j, node in enumerate(cycle) if R[node] == 1]
        gap = max(((minimum[(j+1) % len(minimum)]-v) % len(cycle)) or len(cycle)
                  for j, v in enumerate(minimum)) if minimum else None
        drift = tuple(str(Fraction(d, len(cycle))) for d in translation)
        cycle_rows.append({"representative": min(cycle), "length": len(cycle),
                           "R_histogram": dict(sorted(Counter(map(int, R[cycle])).items())),
                           "distance_histogram": dict(sorted(Counter(map(int, distances[cycle])).items())),
                           "R1_states": len(minimum), "largest_R1_spacing": gap,
                           "translation": translation, "drift": drift,
                           "restoring_transport": bool(any(translation) and minimum),
                           "spatial_record_and_phase_period_L8": len(cycle)*lcm(*(8//gcd(8, abs(d)) for d in translation)),
                           "nodes_sha256": hashlib.sha256(np.asarray(cycle, dtype="<i4").tobytes()).hexdigest()})
        cycles.append(cycle)
    for i in reversed(removed):
        j = int(successor[i])
        component[i], transient[i] = component[j], transient[j]+1
    if np.any(component < 0):
        raise AssertionError("unclassified graph state")
    for row, size in zip(cycle_rows, np.bincount(component, minlength=len(cycles))):
        row["basin_states"] = int(size)
    first_R1 = _first_hit(successor, cycles, removed, R == 1)
    first_adjacent = _first_hit(successor, cycles, removed, distances == 1)
    first_outside = _first_hit(successor, cycles, removed, R != 1)
    following_hit = first_R1[successor]
    positive_return = np.where(following_hit < 0, -1, following_hit+1).astype("<i4")
    after_departure = np.full(STATE_COUNT, -1, dtype="<i4")
    for i in np.flatnonzero(R == 1):
        departure = int(first_outside[i])
        if departure > 0:
            j = int(i)
            for _ in range(departure):
                j = int(successor[j])
            if first_R1[j] >= 0:
                after_departure[i] = departure+first_R1[j]
    cases = _prepared_cases(component, cycle_rows, first_R1, first_adjacent, successor, R)
    for row in cases:
        i = row["node"]
        row["positive_time_R1_hit"] = int(positive_return[i])
        row["first_departure_from_R1"] = int(first_outside[i]) if R[i] == 1 else None
        row["return_after_actual_departure"] = int(after_departure[i]) if R[i] == 1 else None
    relative_table = np.full((RELATIVE_COUNT, 9), 255, dtype=np.uint8)
    for i, r in enumerate(rs):
        relative_table[i, 0] = len(r.path)
        relative_table[i, 1:1+len(r.path)] = r.path
        relative_table[i, 4:8] = (*r.velocities, *r.credits)
        relative_table[i, 8] = sum(abs(x) for x in endpoint(r.path))
    arrays = {"successor": successor, "plus_move": plus_move, "minus_move": minus_move,
              "exchange": exchange, "path_length": R, "endpoint_delta": separation,
              "component": component, "transient_to_cycle": transient, "first_R1": first_R1,
              "first_adjacency": first_adjacent, "first_outside_R1": first_outside,
              "positive_time_R1_hit": positive_return, "return_after_departure": after_departure,
              "relative_records": relative_table}
    histogram = lambda values: dict(sorted(Counter(map(int, values)).items()))
    returned_components = sum(c["R1_states"] > 0 for c in cycle_rows)
    transported = sum(any(c["translation"]) for c in cycle_rows)
    restoring_transport = sum(c["restoring_transport"] for c in cycle_rows)
    certificate = {
        "schema": "strict-flux-binding-q3-graph-1", "law": LAW_ID,
        "sector": "one plus, one minus, Q=3; explicit canonical matching background; even L>=8",
        "states": STATE_COUNT, "transitions": STATE_COUNT, "relative_records": RELATIVE_COUNT,
        "relative_records_by_R": histogram(len(r.path) for r in rs),
        "initial_R_histogram": histogram(R), "initial_distance_histogram": histogram(distances),
        "recurrent_states": int(np.count_nonzero(indegree)), "components": len(cycles),
        "cycle_length_histogram": histogram(c["length"] for c in cycle_rows),
        "maximum_transient": int(transient.max()), "first_R1_histogram": histogram(first_R1),
        "positive_time_R1_hit_histogram": histogram(positive_return),
        "R1_departure_return_histogram": histogram(after_departure[R == 1]),
        "states_never_reaching_R1": int(np.count_nonzero(first_R1 < 0)),
        "components_visiting_R1": returned_components, "transported_components": transported,
        "restoring_transported_components": restoring_transport,
        "existence_gate": restoring_transport > 0,
        "global_restoration_gate": bool(np.all(first_R1 >= 0) and returned_components == len(cycles)),
        "coherent_baselines": 3456, "coherent_instances": len(cases),
        "coherent_instance_kinds": dict(Counter(row["kind"] for row in cases)),
        "coherent_instances_passed": sum(row["restoring_transport"] for row in cases),
        "coherent_robustness_gate": all(row["restoring_transport"] for row in cases),
        "perturbations_matching_baseline_drift": sum(row["same_drift_as_baseline"] for row in cases if row["kind"] != "baseline"),
        "all_future_distance_bound": 3, "Q_each_transition": 3, "populations_each_transition": [1, 1],
        "exchange_columns": EXCHANGE_COLUMNS,
        "relative_table_columns": ("R", "path0", "path1", "path2", "vplus", "vminus", "kplus", "kminus", "distance"),
        "relative_table_missing_path_code": 255,
        "array_sha256": {name: hashlib.sha256(array.tobytes()).hexdigest() for name, array in arrays.items()},
        "cycles": cycle_rows,
        "limits": {"ordinal_periodic": False, "event_list_parity": False,
                   "physical_energy_identified": False, "many_particle_recovery": False,
                   "unified_matter_fluid_law": False, "canonical_adoption": False},
    }
    return certificate, arrays, cases


def _write_json(path, data):
    path.write_bytes((json.dumps(data, indent=2, sort_keys=True)+"\n").encode())


def _archive_arrays(path, arrays):
    with zipfile.ZipFile(path, "x", compression=zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        for name, array in sorted(arrays.items()):
            stream = BytesIO()
            np.save(stream, array, allow_pickle=False)
            info = zipfile.ZipInfo(name+".npy", date_time=(2026, 9, 8, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(info, stream.getvalue())
    with np.load(path, allow_pickle=False) as loaded:
        if set(loaded.files) != set(arrays) or any(not np.array_equal(loaded[k], v) for k, v in arrays.items()):
            raise AssertionError("transition archive roundtrip failed")


def _sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def _accepted_upstream(root, receipt_path, receipt_sha256, runtime_sha256):
    """Verify a separately reviewed, pinned disposition before execution."""
    root, receipt_path = Path(root).resolve(), Path(receipt_path).resolve()
    if not isinstance(receipt_sha256, str) or _sha(receipt_path) != receipt_sha256:
        raise ValueError("independent acceptance receipt hash mismatch")
    receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
    if (receipt.get("schema") != "strict-balanced-matching-acceptance-1"
            or receipt.get("law_id") != LAW_ID
            or receipt.get("runtime_sha256") != runtime_sha256
            or receipt.get("verdict") != "PASS_SCOPED_FINITE_RUNTIME"
            or receipt.get("canonical_adoption") is not False):
        raise ValueError("independent acceptance disposition or law mismatch")
    gates = receipt.get("gates", {})
    required_gates = ("local_causality", "accounting", "spatial_covariance",
                      "balanced_conjugation", "checkpoint_replay")
    if not isinstance(gates, dict) or any(gates.get(k) != "PASS" for k in required_gates):
        raise ValueError("independent upstream gates are not accepted")
    sources = receipt.get("source_sha256", {})
    required_sources = {"scripts/phi_v2_lattice/"+name for name in
                        ("balanced_matching.py", "recorded_matching.py", "flux_binding.py")}
    if not isinstance(sources, dict) or not required_sources.issubset(sources):
        raise ValueError("independent acceptance missing required source identities")
    if sources["scripts/phi_v2_lattice/balanced_matching.py"] != runtime_sha256:
        raise ValueError("accepted runtime identity mismatch")
    files = [receipt_path]
    for name, expected in list(sources.items()) + [(receipt.get("audit_path"), receipt.get("audit_sha256"))]:
        if not isinstance(name, str) or Path(name).is_absolute():
            raise ValueError("accepted source/audit must be repository relative")
        path = (root/name).resolve()
        if not path.is_relative_to(root) or not path.is_file() or _sha(path) != expected:
            raise ValueError("accepted source/audit hash mismatch")
        files.append(path)
    return files


def _collect_ranges(ranges, workers, runtime_sha256, directory):
    """Persist each success or failure as it completes, including partial runs."""
    rows = []
    directory.mkdir()

    def record(start, stop, result=None, error=None):
        row = {"start": start, "stop": stop, "checked": 0, "status": "FAIL"}
        if error is not None:
            row["error"] = "".join(traceback.format_exception(error))
        else:
            row.update(result)
            row["status"] = "PASS"
        _write_json(directory/f"{start:07d}.json", row)
        rows.append(row)
        if len(rows) % 12 == 0:
            print(json.dumps({"complete_ranges": len(rows), "states_checked": sum(r["checked"] for r in rows),
                              "failed_ranges": sum(r["status"] != "PASS" for r in rows)}), flush=True)

    if workers == 1:
        for start, stop in ranges:
            try:
                result = verify_runtime_range(start, stop, runtime_sha256)
            except Exception as error:
                record(start, stop, error=error)
            else:
                record(start, stop, result=result)
    else:
        with ProcessPoolExecutor(max_workers=workers) as pool:
            pending = {pool.submit(verify_runtime_range, a, b, runtime_sha256): (a, b) for a, b in ranges}
            for future in as_completed(pending):
                start, stop = pending[future]
                try:
                    result = future.result()
                except Exception as error:
                    record(start, stop, error=error)
                else:
                    record(start, stop, result=result)
    return sorted(rows, key=lambda row: row["start"])


def certify(directory, runtime_sha256, acceptance_audit, acceptance_sha256, workers=24):
    from . import balanced_matching as runtime
    workers = _integer(workers, "workers", low=1, high=32)
    directory = Path(directory)
    if directory.exists():
        raise ValueError("output exists; preserve every attempt")
    if (not isinstance(runtime_sha256, str) or len(runtime_sha256) != 64
            or hashlib.sha256(Path(runtime.__file__).read_bytes()).hexdigest() != runtime_sha256):
        raise ValueError("registered runtime hash mismatch")
    root = Path(__file__).resolve().parents[2]
    accepted_files = _accepted_upstream(root, acceptance_audit, acceptance_sha256, runtime_sha256)
    files = [Path(__file__).resolve(), Path(runtime.__file__).resolve(), *accepted_files,
             root/"scripts/tests/phi_v2_lattice/test_recovery_flux_binding_q3.py",
             root/"engine/docs/DERIV_STRICT_FLUX_BINDING_Q3_V1.md",
             root/"engine/docs/PROPOSAL_STRICT_BOUND_COMPOSITE_TRANSPORT_V1.md",
             root/"engine/docs/AMENDMENT_STRICT_WAVE6_BALANCED_ORDER_V1.md",
             root/"scripts/phi_v2_lattice/recorded_matching.py",
             root/"scripts/phi_v2_lattice/flux_binding.py"]
    hashes = {str(p): _sha(p) for p in files}
    directory.mkdir(parents=True)
    lock = {"schema": "strict-flux-binding-q3-execution-lock-1", "source_sha256": hashes,
            "law": runtime.LAW_ID, "workers": workers, "full_runtime_L": 8,
            "states": STATE_COUNT, "ranges": 96, "range_size": RELATIVE_COUNT,
            "coherent_instances": 38016, "intended_resource_ceiling_seconds": 1800,
            "ceiling_enforcement": "post-run rejection; no hard cancellation",
            "acceptance_receipt_sha256": acceptance_sha256,
            "scope": "exact finite graph and Python implementation check; not GPU or performance campaign"}
    _write_json(directory/"lock.json", lock)
    started = time.monotonic()
    ranges = [(start, min(start+RELATIVE_COUNT, STATE_COUNT)) for start in range(0, STATE_COUNT, RELATIVE_COUNT)]
    certificate, rows, failure = {}, [], None
    try:
        certificate, arrays, cases = graph()
        _write_json(directory/"graph.json", certificate)
        _write_json(directory/"coherent-preparations.json", {"cases": cases})
        _archive_arrays(directory/"transitions.npz", arrays)
        print(json.dumps({"graph_states": STATE_COUNT, "components": certificate["components"],
                          "transported_components": certificate["transported_components"],
                          "existence_gate": certificate["existence_gate"]}), flush=True)
        rows = _collect_ranges(ranges, workers, runtime_sha256, directory/"runtime-ranges")
    except BaseException as error:
        failure = "".join(traceback.format_exception(error))
        # Receipts already on disk survive failures in the parent or pool.
        rows = [json.loads(p.read_text(encoding="utf-8")) for p in sorted((directory/"runtime-ranges").glob("*.json"))]
    elapsed = time.monotonic()-started
    current = {p: _sha(p) if Path(p).is_file() else None for p in hashes}
    drift = [p for p, expected in hashes.items() if current[p] != expected]
    pairs = [(row["start"], row["stop"]) for row in rows]
    exact = (pairs == ranges and sum(r["checked"] for r in rows) == STATE_COUNT
             and all(r["status"] == "PASS" for r in rows) and not drift and failure is None)
    complete = exact and elapsed <= 1800
    result = {"schema": "strict-flux-binding-q3-execution-result-1", "law": runtime.LAW_ID,
              "elapsed_seconds": elapsed, "source_drift": drift, "resource_ceiling_met": elapsed <= 1800,
              "post_run_source_sha256": current, "failure": failure,
              "complete_arrays_and_ordinal_equal": exact, "observed_quotient_equal": exact,
              "runtime_states_checked": sum(r["checked"] for r in rows), "ranges": rows,
              "existence_gate": certificate.get("existence_gate"), "global_restoration_gate": certificate.get("global_restoration_gate"),
              "coherent_robustness_gate": certificate.get("coherent_robustness_gate"),
              "engineering_complete": complete, "canonical_adoption": False,
              "artifact_sha256": {p.relative_to(directory).as_posix(): _sha(p)
                                  for p in sorted(directory.rglob("*")) if p.is_file()}}
    _write_json(directory/"report.json", result)
    if not complete:
        raise ValueError("registered exhaustive campaign incomplete or changed")
    print(json.dumps({k: v for k, v in result.items() if k not in ("ranges", "artifact_sha256")}), flush=True)
    return result


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True)
    parser.add_argument("--runtime-sha256", required=True)
    parser.add_argument("--acceptance-audit", required=True)
    parser.add_argument("--acceptance-sha256", required=True)
    parser.add_argument("--workers", type=int, default=24)
    args = parser.parse_args()
    certify(args.output, args.runtime_sha256, args.acceptance_audit, args.acceptance_sha256, args.workers)
