"""Registered complete Q2 quotient for the 19-microtick exchange candidate.

This independent path/credit/attempt evolution is an exact-sector proposal
until every transition has been compared with the accepted complete runtime.
"""
from collections import Counter, deque
from concurrent.futures import ProcessPoolExecutor, as_completed
from dataclasses import dataclass, fields
from fractions import Fraction
from functools import lru_cache
import hashlib
from io import BytesIO
from itertools import permutations, product
import json
from math import gcd, lcm
from numbers import Integral
import os
from pathlib import Path
import time
import traceback
from types import MappingProxyType
import zipfile

import numpy as np

V = ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1))
COLORS = tuple(product(range(2), repeat=3))
AXES = tuple(permutations(range(3)))
RELATIVE_COUNT, STATE_COUNT = 6192, 941184
LAW_ID = "phi-flux-credit-exchange-binding-candidate-1"
ARRAY_NAMES = ("direction", "credit", "flux", "matching_frame", "charge_frame", "attempted")


def _int(value, name, low=0, high=None):
    if isinstance(value, bool) or not isinstance(value, Integral) or value < low or (high is not None and value > high):
        raise ValueError("invalid "+name)
    return int(value)


@dataclass(frozen=True)
class Record:
    path: tuple[int, ...]
    velocities: tuple[int, int]
    credits: tuple[int, int]
    attempted: tuple[int, int]


@dataclass(frozen=True)
class QuotientState:
    stage: int
    color: tuple[int, int, int]
    record: Record


def normalized(q):
    if type(q) is not QuotientState or type(q.record) is not Record:
        raise ValueError("expected exact Q2 exchange record")
    stage = _int(q.stage, "stage", high=18)
    if type(q.color) is not tuple or len(q.color) != 3:
        raise ValueError("invalid color")
    color = tuple(_int(x, "color", high=1) for x in q.color)
    r = q.record
    if type(r.path) is not tuple or len(r.path) > 2:
        raise ValueError("invalid path")
    path = tuple(_int(d, "path channel", high=5) for d in r.path)
    if any(a == (b ^ 1) for a, b in zip(path, path[1:])):
        raise ValueError("nonbacktracking path required")
    fields = []
    for name, values, high in (("velocity", r.velocities, 5), ("credit", r.credits, 1), ("attempt", r.attempted, 1)):
        if type(values) is not tuple or len(values) != 2:
            raise ValueError("invalid "+name+" tuple")
        fields.append(tuple(_int(value, name, high=high) for value in values))
    if len(path)+sum(fields[1]) != 2:
        raise ValueError("outside Q2")
    return QuotientState(stage, color, Record(path, *fields))


@lru_cache(maxsize=1)
def records():
    paths = tuple(p for length in range(3) for p in product(range(6), repeat=length)
                  if not any(a == (b ^ 1) for a, b in zip(p, p[1:])))
    return tuple(Record(p, v, k, attempts) for p in paths for k in product(range(2), repeat=2)
                 if len(p)+sum(k) == 2 for v in product(range(6), repeat=2) for attempts in product(range(2), repeat=2))


@lru_cache(maxsize=1)
def indices():
    return MappingProxyType({r: i for i, r in enumerate(records())})


def decode(node):
    node = _int(node, "node", high=STATE_COUNT-1)
    sc, record = divmod(node, RELATIVE_COUNT)
    stage, color = divmod(sc, 8)
    return QuotientState(stage, COLORS[color], records()[record])


def encode(q):
    q = normalized(q)
    return (q.stage*8+COLORS.index(q.color))*RELATIVE_COUNT+indices()[q.record]


def endpoint(path):
    return tuple(sum(V[d][a] for d in path) for a in range(3))


def translating_family(q):
    q = normalized(q)
    r = q.record
    if q.stage != 1 or len(r.path) != 1 or r.attempted != (0, 0):
        return False
    heading = r.path[0] if r.credits[0] else r.path[0] ^ 1
    return r.velocities == (heading, heading)


def transition(q):
    """Independent one-real-tick rule; operation codes describe local decisions.

    Codes:0 hold,1 reset,2 reset+contact alignment,3 credit transfer+alignment,
    4 extension,5 retraction,6 zero-credit guidance,7 credit-cap reversal.
    """
    q = normalized(q)
    r, stage = q.record, q.stage
    path, color = r.path, q.color
    velocities, credits, attempted = list(r.velocities), list(r.credits), list(r.attempted)
    dp = dm = (0, 0, 0)
    operation = 0
    if stage == 0:
        attempted = [0, 0]
        operation = 1
        if not path:
            velocities[1] = velocities[0]
            operation = 2
    elif stage < 7:
        axis, parity = (stage-1)//2, (stage-1) % 2
        matched = 2*axis+int(color[axis] != parity)
        if len(path) == 1 and path[0] == matched:
            heading = path[0] if credits[0] else path[0] ^ 1
            credits.reverse()
            velocities = [heading, heading]
            operation = 3
    else:
        phase = stage-7
        slot, axis, parity = phase//6, (phase % 6)//2, phase % 2
        separation = endpoint(path)
        source = (0, 0, 0) if slot == 0 else separation
        matched = 2*axis+int((color[axis]+source[axis]) % 2 != parity)
        v, k = velocities[slot], credits[slot]
        if not attempted[slot] and v == matched:
            attempted[slot] = 1
            inward = (path[0] if slot == 0 else path[-1] ^ 1) if path else None
            moved = False
            if v == inward:
                if k == 0:
                    path = path[1:] if slot == 0 else path[:-1]
                    credits[slot] = 1
                    moved, operation = True, 5
                else:
                    velocities[slot] = v ^ 1
                    operation = 7
            elif k == 1:
                path = (v ^ 1,)+path if slot == 0 else path+(v,)
                credits[slot] = 0
                moved, operation = True, 4
            else:
                if inward is None:
                    raise AssertionError("Q2 zero-credit carrier has no inward guide")
                velocities[slot] = inward
                operation = 6
            if moved:
                if slot == 0:
                    dp = V[v]
                    color = tuple((color[a]+dp[a]) % 2 for a in range(3))
                else:
                    dm = V[v]
    out = normalized(QuotientState((stage+1) % 19, color,
                    Record(path, tuple(velocities), tuple(credits), tuple(attempted))))
    if endpoint(path) != tuple(endpoint(r.path)[a]+dm[a]-dp[a] for a in range(3)):
        raise AssertionError("relative displacement lost")
    return out, dp, dm, operation


def materialize(q, L=6, positive_position=None, microtick=None, frame_permutation=0):
    """Independent exact lift, including every explicit background/control bit.

    The geometric chart follows the frozen Q3 lift convention; its state,
    credit budget, control records and nineteen-phase clock are this law's.
    """
    from . import credit_exchange_binding as runtime
    q = normalized(q)
    L = _int(L, "periodic size", low=6)
    if L % 2:
        raise ValueError("even periodic size required")
    frame_permutation = _int(frame_permutation, "axis order", high=5)
    axes = AXES[frame_permutation]
    point = q.color if positive_position is None else positive_position
    if not isinstance(point, (tuple, list)) or len(point) != 3:
        raise ValueError("three position coordinates required")
    point = tuple(_int(x, "position", high=L-1) for x in point)
    tick = q.stage if microtick is None else _int(microtick, "ordinal")
    if tick % 19 != q.stage:
        raise ValueError("ordinal/stage mismatch")
    direction = np.zeros((L**3, 2), dtype=np.uint8)
    credit, attempted = np.zeros_like(direction), np.zeros_like(direction)
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
        if flux[index(owner), d//2]:
            raise AssertionError("path reuses an edge")
        flux[index(owner), d//2] = 1 if d % 2 == 0 else -1
        current = target
    for slot, p in enumerate((point, current)):
        direction[index(p), slot] = physical(q.record.velocities[slot])+1
        credit[index(p), slot] = q.record.credits[slot]
        attempted[index(p), slot] = q.record.attempted[slot]
    charge_frame = np.zeros(L**3, dtype=np.uint8)
    return runtime.ExchangeState(L, tick, direction, credit, flux, background, charge_frame, attempted)


def observe(state):
    """Read a complete Q2 chart without using the evolution or a partner label."""
    from . import credit_exchange_binding as runtime
    runtime.validate(state)
    L = int(state.L)
    if L < 6 or np.any(state.charge_frame):
        raise ValueError("Q2 chart requires even L>=6 and explicit eta zero")
    positions = [np.flatnonzero(state.direction[:, slot]) for slot in range(2)]
    if any(len(p) != 1 for p in positions) or runtime.account_units(state) != 2:
        raise ValueError("Q2 chart requires one carrier per polarity and account two")
    plus, minus = (int(p[0]) for p in positions)
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
        if len(ports) != 1 or len(path) >= 2:
            raise ValueError("not a unique short unit-flow path")
        path.append(internal(ports[0]))
        current = shift(current, ports[0])
    if int(np.count_nonzero(state.flux)) != len(path):
        raise ValueError("flux outside the observed path")
    q = QuotientState(int(state.microtick % 19), tuple((frame >> a) & 1 for a in range(3)),
                     Record(tuple(path), tuple(internal(int(state.direction[p, slot])-1)
                            for slot, p in enumerate((plus, minus))),
                            tuple(int(state.credit[p, slot]) for slot, p in enumerate((plus, minus))),
                            tuple(int(state.attempted[p, slot]) for slot, p in enumerate((plus, minus)))))
    return normalized(q)


def verify_runtime_range(start, stop, runtime_sha256, L=6):
    """Full-array equality, ownership, accounting and observer for a fixed range.

    Failures retain the completed prefix and failing node. Returned event
    counts are diagnostic coverage, not a proof of event-list equivalence.
    """
    from . import credit_exchange_binding as runtime
    start = _int(start, "range start", high=STATE_COUNT)
    stop = _int(stop, "range stop", low=start, high=STATE_COUNT)
    began = time.monotonic()
    digest, counts = hashlib.sha256(), Counter()
    checked = 0
    failed = None
    try:
        if runtime.LAW_ID != LAW_ID or _sha(runtime.__file__) != runtime_sha256:
            raise ValueError("candidate source/law differs from registered input")
        for i in range(start, stop):
            q = decode(i)
            state = materialize(q, L)
            originals = tuple(getattr(state, name).tobytes() for name in ARRAY_NAMES)
            next_q, shift_p, _, operation = transition(q)
            actual, events = runtime.step(state)
            point = tuple((q.color[a]+shift_p[a]) % L for a in range(3))
            expected = materialize(next_q, L, point, q.stage+1)
            if actual.microtick != expected.microtick or observe(actual) != next_q:
                raise AssertionError("ordinal or observed quotient mismatch")
            if runtime.account_units(actual) != 2:
                raise AssertionError("integer account mismatch")
            outputs = tuple(getattr(actual, name) for name in ARRAY_NAMES)
            inputs = tuple(getattr(state, name) for name in ARRAY_NAMES)
            for j, (name, original) in enumerate(zip(ARRAY_NAMES, originals)):
                array = outputs[j]
                if not np.array_equal(array, getattr(expected, name)):
                    raise AssertionError("complete "+name+" mismatch")
                if inputs[j].tobytes() != original:
                    raise AssertionError("input mutation")
                if not array.flags.owndata or not array.flags.c_contiguous:
                    raise AssertionError("output does not own contiguous storage")
                if any(np.shares_memory(array, other) for other in inputs+outputs[:j]):
                    raise AssertionError("output alias")
                digest.update(array.tobytes())
            digest.update(bytes((actual.microtick,)))
            counts[str(operation)] += 1
            for field in fields(events):
                counts["external_"+field.name] += len(getattr(events, field.name))
            checked += 1
        if _sha(runtime.__file__) != runtime_sha256:
            raise ValueError("candidate source drift during range")
    except Exception:
        failed = traceback.format_exc()
    return {"schema": "credit-exchange-q2-runtime-range-1", "start": start, "stop": stop,
            "completed_stop": start+checked, "checked": checked, "L": L,
            "runtime_sha256": runtime_sha256, "complete_outputs_sha256": digest.hexdigest(),
            "operations": dict(counts), "seconds": time.monotonic()-began,
            "passed": failed is None and checked == stop-start, "failure": failed}


def first_hits(successor, cycles, removed, target):
    hit = np.full(len(successor), -1, dtype=np.int32)
    for cycle in cycles:
        if any(target[i] for i in cycle):
            distance = -1
            for j in range(2*len(cycle)-1, -1, -1):
                i = cycle[j % len(cycle)]
                distance = 0 if target[i] else distance+1 if distance >= 0 else -1
                if j < len(cycle):
                    hit[i] = distance
    for i in reversed(removed):
        d = int(hit[int(successor[i])])
        hit[i] = 0 if target[i] else d+1 if d >= 0 else -1
    return hit


def preparations(component, cycle_rows, first_target, first_adjacent):
    rows = []
    baseline = 0
    for stage, color, direction, heading, full_slot, attempts in product(
            range(19), COLORS, range(6), range(6), range(2), product(range(2), repeat=2)):
        credits = (1, 0) if full_slot == 0 else (0, 1)
        base = QuotientState(stage, color, Record((direction,), (heading, heading), credits, attempts))
        base_node = encode(base)
        base_drift = cycle_rows[int(component[base_node])]["drift"]
        moving_base = translating_family(base)
        cases = [("baseline", None, base)]
        inward = direction if full_slot == 0 else direction ^ 1
        for kick in range(6):
            if kick == inward:
                continue
            path = (kick ^ 1, direction) if full_slot == 0 else (direction, kick)
            c = tuple((color[a]+V[kick][a]) % 2 for a in range(3)) if full_slot == 0 else color
            cases.append(("plus_departure" if full_slot == 0 else "minus_departure", kick,
                          QuotientState(stage, c, Record(path, (heading, heading), (0, 0), attempts))))
        for kind, kick, q in cases:
            node = encode(q)
            label = int(component[node])
            cycle = cycle_rows[label]
            rows.append({"baseline_id": baseline, "kind": kind, "kick": kick, "node": node,
                         "component": label, "first_translating_family": int(first_target[node]),
                         "first_adjacency": int(first_adjacent[node]),
                         "baseline_in_translating_family": moving_base,
                         "drift": cycle["drift"], "same_drift_as_baseline": cycle["drift"] == base_drift,
                         "restoring_transport": cycle["restoring_transport"]})
        baseline += 1
    if baseline != 43776 or len(rows) != 262656:
        raise AssertionError("registered preparation family incomplete")
    return rows


def graph():
    """Complete finite classification, with no filtering by successful outcomes."""
    rs = records()
    if len(rs) != RELATIVE_COUNT or len(set(rs)) != RELATIVE_COUNT:
        raise AssertionError("complete record census differs")
    successor = np.empty(STATE_COUNT, dtype="<i4")
    plus = np.empty((STATE_COUNT, 3), dtype=np.int8)
    minus = np.empty_like(plus)
    operation = np.empty(STATE_COUNT, dtype=np.uint8)
    target = np.zeros(STATE_COUNT, dtype=np.bool_)
    path_length = np.empty(STATE_COUNT, dtype=np.uint8)
    for i in range(STATE_COUNT):
        q = decode(i)
        out, plus[i], minus[i], operation[i] = transition(q)
        successor[i] = encode(out)
        target[i], path_length[i] = translating_family(q), len(q.record.path)
    degree = np.bincount(successor, minlength=STATE_COUNT).astype(np.int32)
    queue = deque(map(int, np.flatnonzero(degree == 0)))
    removed = []
    while queue:
        i = queue.popleft()
        removed.append(i)
        j = int(successor[i])
        degree[j] -= 1
        if degree[j] == 0:
            queue.append(j)
    component = np.full(STATE_COUNT, -1, dtype="<i4")
    transient = np.zeros(STATE_COUNT, dtype="<i4")
    cycles, cycle_rows = [], []
    for start in np.flatnonzero(degree):
        i = int(start)
        if component[i] >= 0:
            continue
        cycle = []
        while component[i] < 0:
            component[i] = len(cycles)
            cycle.append(i)
            i = int(successor[i])
        if i != cycle[0] or len(cycle) % 19:
            raise AssertionError("cycle phase decomposition failed")
        dp, dm = (tuple(map(int, moves[cycle].sum(axis=0))) for moves in (plus, minus))
        if dp != dm or any(d % 2 for d in dp):
            raise AssertionError("full path/color periodicity differs from displacement")
        row = {"representative": min(cycle), "length": len(cycle), "translation": dp,
               "drift": tuple(str(Fraction(d, len(cycle))) for d in dp),
               "translating_family_states": int(target[cycle].sum()),
               "restoring_transport": bool(any(dp) and np.any(target[cycle])),
               "R_histogram": dict(Counter(map(int, path_length[cycle]))),
               "spatial_records_and_phase_period_L6": len(cycle)*lcm(*(6//gcd(6, abs(d)) for d in dp)),
               "nodes_sha256": hashlib.sha256(np.asarray(cycle, dtype="<i4").tobytes()).hexdigest()}
        cycles.append(cycle)
        cycle_rows.append(row)
    for i in reversed(removed):
        j = int(successor[i])
        component[i], transient[i] = component[j], transient[j]+1
    if np.any(component < 0):
        raise AssertionError("unclassified state")
    for row, size in zip(cycle_rows, np.bincount(component, minlength=len(cycles))):
        row["basin_states"] = int(size)
    first_target = first_hits(successor, cycles, removed, target)
    first_adjacent = first_hits(successor, cycles, removed, path_length <= 1)
    fresh_R2 = ((np.arange(STATE_COUNT)//(8*RELATIVE_COUNT) == 1) & (path_length == 2)
                & np.tile(np.array([r.attempted == (0, 0) for r in rs]), 19*8))
    if int(fresh_R2.sum()) != 8640:
        raise AssertionError("complete fresh phase-one R2 family differs")
    cases = preparations(component, cycle_rows, first_target, first_adjacent)
    moving_kicks = [row for row in cases if row["baseline_in_translating_family"] and row["kind"] != "baseline"]
    if len(moving_kicks) != 480:
        raise AssertionError("registered translating-family kick count differs")
    relative = np.full((RELATIVE_COUNT, 10), 255, dtype=np.uint8)
    for i, r in enumerate(rs):
        relative[i, 0] = len(r.path)
        relative[i, 1:1+len(r.path)] = r.path
        relative[i, 3:] = (*r.velocities, *r.credits, *r.attempted, sum(abs(v) for v in endpoint(r.path)))
    arrays = {"successor": successor, "plus_move": plus, "minus_move": minus, "operation": operation,
              "translating_family": target, "path_length": path_length, "component": component,
              "transient_to_cycle": transient, "first_translating_family": first_target, "relative_records": relative}
    arrays["first_adjacency"] = first_adjacent
    arrays["fresh_phase1_R2"] = fresh_R2
    convergence = bool(np.all(first_target >= 0) and int(first_target.max()) <= 95)
    predicted_cycles = (len(cycles) == 48 and all(c["length"] == 38 and sum(abs(d) for d in c["translation"]) == 2
                        and c["restoring_transport"] for c in cycle_rows) and int(np.count_nonzero(degree)) == 1824)
    summary = {"schema": "credit-exchange-q2-graph-1", "states": STATE_COUNT,
               "relative_records_with_attempts": RELATIVE_COUNT, "components": len(cycles),
               "cycle_length_histogram": dict(Counter(c["length"] for c in cycle_rows)),
               "recurrent_states": int(np.count_nonzero(degree)), "translating_family_states": int(target.sum()),
               "maximum_transient": int(transient.max()), "first_target_histogram": dict(Counter(map(int, first_target))),
               "maximum_first_target": int(first_target.max()), "unrestored_states": int(np.count_nonzero(first_target < 0)),
               "all_translating_family_states_recurrent": bool(np.all(degree[target] != 0)),
               "global95_tick_gate": convergence, "registered_cycle_prediction": predicted_cycles,
               "coherent_baselines": 43776, "coherent_instances": len(cases),
               "coherent_restoring_transport": all(row["restoring_transport"] for row in cases),
               "moving_family_kicks": len(moving_kicks),
               "moving_kick_maximum_family_hit": max(row["first_translating_family"] for row in moving_kicks),
               "moving_kick_maximum_adjacency_hit": max(row["first_adjacency"] for row in moving_kicks),
               "moving_kick76_tick_gate": all(0 <= row["first_translating_family"] <= 76 for row in moving_kicks),
               "moving_kick38_tick_adjacency_gate": all(0 <= row["first_adjacency"] <= 38 for row in moving_kicks),
               "fresh_phase1_R2_states": int(fresh_R2.sum()),
               "fresh_R2_maximum_family_hit": int(first_target[fresh_R2].max()),
               "fresh_R2_maximum_adjacency_hit": int(first_adjacent[fresh_R2].max()),
               "fresh_R2_76_tick_gate": bool(np.all((first_target[fresh_R2] >= 0) & (first_target[fresh_R2] <= 76))),
               "fresh_R2_38_tick_adjacency_gate": bool(np.all((first_adjacent[fresh_R2] >= 0) & (first_adjacent[fresh_R2] <= 38))),
               "perturbations_matching_baseline_drift": sum(row["same_drift_as_baseline"] for row in cases if row["kind"] != "baseline"),
               "array_sha256": {name: hashlib.sha256(value.tobytes()).hexdigest() for name, value in arrays.items()},
               "cycles": cycle_rows, "canonical_adoption": False, "physical_matter_identified": False}
    return summary, arrays, cases


def _sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def _write_json(path, data):
    """Write once and flush a receipt; existing attempts cannot be replaced."""
    with Path(path).open("xb") as handle:
        handle.write((json.dumps(data, sort_keys=True, indent=2)+"\n").encode())
        handle.flush()
        os.fsync(handle.fileno())


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


def _acceptance(root, receipt_path, receipt_sha256, runtime_sha256):
    """Check pinned independent runtime AND quotient/campaign acceptance."""
    root, receipt_path = Path(root).resolve(), Path(receipt_path).resolve()
    if _sha(receipt_path) != receipt_sha256:
        raise ValueError("independent acceptance receipt hash mismatch")
    receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
    if (receipt.get("schema") != "credit-exchange-q2-acceptance-1"
            or receipt.get("law_id") != LAW_ID
            or receipt.get("verdict") != "PASS_SCOPED_EXACT_EXECUTION"
            or receipt.get("canonical_adoption") is not False):
        raise ValueError("unaccepted independent disposition")
    gates = receipt.get("gates", {})
    required_gates = ("finite_law", "checkpoint_replay", "accounting", "causality", "symmetry", "quotient_campaign")
    if not isinstance(gates, dict) or any(gates.get(k) != "PASS" for k in required_gates):
        raise ValueError("independent gates not all accepted")
    sources, audits = receipt.get("source_sha256", {}), receipt.get("audits", {})
    required_sources = {"scripts/phi_v2_lattice/"+name for name in
                        ("credit_exchange_binding.py", "recovery_credit_exchange_q2.py", "balanced_matching.py",
                         "recorded_matching.py", "flux_binding.py")}
    required_sources.update({"engine/docs/"+name for name in
                             ("SPEC_STRICT_CREDIT_EXCHANGE_BINDING_V1.md", "DERIV_STRICT_CREDIT_EXCHANGE_Q2_V1.md",
                              "CONTRACT_STRICT_RECOVERY_WAVE7_V1.md")})
    required_sources.update({"scripts/tests/phi_v2_lattice/"+name for name in
                             ("test_credit_exchange_binding.py", "test_recovery_credit_exchange_q2.py")})
    required_audits = {"engine/docs/AUDIT_STRICT_CREDIT_EXCHANGE_"+name+"_V1.md" for name in ("BINDING", "Q2")}
    if not isinstance(sources, dict) or not required_sources.issubset(sources):
        raise ValueError("independent acceptance missing source identities")
    if not isinstance(audits, dict) or not required_audits.issubset(audits):
        raise ValueError("independent acceptance missing audit identities")
    if sources["scripts/phi_v2_lattice/credit_exchange_binding.py"] != runtime_sha256:
        raise ValueError("runtime identity differs from acceptance")
    paths = [receipt_path]
    for name, expected in list(sources.items())+list(audits.items()):
        if not isinstance(name, str) or Path(name).is_absolute():
            raise ValueError("accepted path must be repository relative")
        path = (root/name).resolve()
        if not path.is_relative_to(root) or not path.is_file() or _sha(path) != expected:
            raise ValueError("accepted source/audit hash mismatch")
        paths.append(path)
    return paths


def _collect_ranges(ranges, workers, runtime_sha256, directory):
    directory.mkdir()
    rows = []
    with ProcessPoolExecutor(max_workers=workers) as pool:
        pending = {pool.submit(verify_runtime_range, a, b, runtime_sha256): (a, b) for a, b in ranges}
        for future in as_completed(pending):
            start, stop = pending[future]
            try:
                row = future.result()
                if row["start"] != start or row["stop"] != stop:
                    raise ValueError("worker returned a different range")
            except Exception:
                row = {"schema": "credit-exchange-q2-runtime-range-1", "start": start, "stop": stop,
                       "checked": 0, "completed_stop": start, "passed": False, "failure": traceback.format_exc()}
            _write_json(directory/f"{start:07d}.json", row)
            rows.append(row)
            if len(rows) % 19 == 0:
                print(json.dumps({"ranges_recorded": len(rows), "states_checked": sum(r["checked"] for r in rows),
                                  "failed_ranges": sum(not r["passed"] for r in rows)}), flush=True)
    return sorted(rows, key=lambda row: row["start"])


def certify(directory, runtime_sha256, acceptance_receipt, acceptance_sha256, workers=32):
    """One preregistered full attempt. A failed gate is retained, never retuned."""
    from . import credit_exchange_binding as runtime
    if _int(workers, "workers", low=32, high=32) != 32:
        raise ValueError("registered worker count differs")
    directory = Path(directory)
    if directory.exists():
        raise ValueError("preserve every existing attempt")
    root = Path(__file__).resolve().parents[2]
    paths = _acceptance(root, acceptance_receipt, acceptance_sha256, runtime_sha256)
    if runtime.LAW_ID != LAW_ID or _sha(runtime.__file__) != runtime_sha256:
        raise ValueError("runtime source/law mismatch")
    hashes = {str(path): _sha(path) for path in paths}
    ranges = [(start, start+RELATIVE_COUNT) for start in range(0, STATE_COUNT, RELATIVE_COUNT)]
    directory.mkdir(parents=True)
    lock = {"schema": "credit-exchange-q2-execution-lock-1", "source_sha256": hashes,
            "law_id": LAW_ID, "runtime_sha256": runtime_sha256, "acceptance_receipt_sha256": acceptance_sha256,
            "workers": 32, "full_runtime_L": 6, "states": STATE_COUNT, "ranges": ranges,
            "coherent_instances": 262656, "intended_resource_ceiling_seconds": 1800,
            "ceiling_enforcement": "post-run rejection; no hard cancellation", "canonical_adoption": False}
    _write_json(directory/"lock.json", lock)
    began = time.monotonic()
    summary, rows, failure = {}, [], None
    try:
        summary, arrays, cases = graph()
        _write_json(directory/"graph.json", summary)
        _write_json(directory/"coherent-preparations.json", {"cases": cases})
        _archive_arrays(directory/"transitions.npz", arrays)
        print(json.dumps({"graph_states": summary["states"], "components": summary["components"],
                          "global95_tick_gate": summary["global95_tick_gate"],
                          "registered_cycle_prediction": summary["registered_cycle_prediction"]}), flush=True)
        rows = _collect_ranges(ranges, 32, runtime_sha256, directory/"runtime-ranges")
    except BaseException:
        failure = traceback.format_exc()
        rows = [json.loads(p.read_text(encoding="utf-8")) for p in sorted((directory/"runtime-ranges").glob("*.json"))]
    range_dir = directory/"runtime-ranges"
    range_dir.mkdir(exist_ok=True)
    recorded = {(r["start"], r["stop"]) for r in rows}
    for start, stop in ranges:
        if (start, stop) not in recorded:
            row = {"schema": "credit-exchange-q2-runtime-range-1", "start": start, "stop": stop,
                   "completed_stop": start, "checked": 0, "passed": False,
                   "failure": "No completed receipt; campaign or worker failed before durable completion."}
            _write_json(range_dir/f"{start:07d}.json", row)
            rows.append(row)
    rows.sort(key=lambda row: row["start"])
    elapsed = time.monotonic()-began
    current = {p: _sha(p) if Path(p).is_file() else None for p in hashes}
    drift = [p for p in hashes if current[p] != hashes[p]]
    exact = ([(r["start"], r["stop"]) for r in rows] == ranges
             and all(r["passed"] and r["checked"] == b-a and r["completed_stop"] == b for r, (a, b) in zip(rows, ranges))
             and not drift and failure is None)
    engineering = exact and elapsed <= 1800
    physical_gates = ("global95_tick_gate", "registered_cycle_prediction", "all_translating_family_states_recurrent",
                      "coherent_restoring_transport", "moving_kick76_tick_gate", "moving_kick38_tick_adjacency_gate",
                      "fresh_R2_76_tick_gate", "fresh_R2_38_tick_adjacency_gate")
    recovery = (engineering and summary.get("translating_family_states") == 96
                and all(summary.get(gate) is True for gate in physical_gates)
                and all(c["spatial_records_and_phase_period_L6"] == 114 for c in summary["cycles"]))
    result = {"schema": "credit-exchange-q2-execution-result-1", "law_id": LAW_ID,
              "elapsed_seconds": elapsed, "resource_ceiling_met": elapsed <= 1800,
              "source_drift": drift, "post_run_source_sha256": current, "failure": failure,
              "runtime_states_checked": sum(r["checked"] for r in rows), "ranges": rows,
              "complete_arrays_and_ordinal_equal": exact, "observed_quotient_equal": exact,
              "event_serialization_parity": "outside this campaign; independent runtime symmetry tests only",
              "engineering_complete": engineering, "registered_Q2_restoring_transport": recovery,
              "canonical_adoption": False, "physical_matter_identified": False,
              "artifact_sha256": {p.relative_to(directory).as_posix(): _sha(p)
                                  for p in sorted(directory.rglob("*")) if p.is_file()}}
    _write_json(directory/"report.json", result)
    print(json.dumps({k: v for k, v in result.items() if k not in ("ranges", "artifact_sha256", "post_run_source_sha256")}), flush=True)
    if not engineering:
        raise ValueError("registered exhaustive campaign incomplete or changed; see retained report")
    return result


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True)
    parser.add_argument("--runtime-sha256", required=True)
    parser.add_argument("--acceptance-receipt", required=True)
    parser.add_argument("--acceptance-sha256", required=True)
    args = parser.parse_args()
    result = certify(args.output, args.runtime_sha256, args.acceptance_receipt, args.acceptance_sha256)
    raise SystemExit(0 if result["registered_Q2_restoring_transport"] else 2)
