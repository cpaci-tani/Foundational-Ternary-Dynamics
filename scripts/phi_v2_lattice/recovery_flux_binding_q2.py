"""Complete two-slot Q=2 quotient of the frozen selected flux law.

Independent compact transitions; the microscopic arrays remain authoritative.
See DERIV_STRICT_FLUX_BINDING_Q2_V1.md for the pre-enumeration contract.
"""
from __future__ import annotations

from collections import Counter, deque
from concurrent.futures import ProcessPoolExecutor
from dataclasses import dataclass
from functools import lru_cache
import hashlib
from itertools import product
import json
from math import gcd, lcm
from numbers import Integral
from pathlib import Path

import numpy as np

LAW_ID = "phi-flux-hop-credit-candidate-1"
RUNTIME_SHA256 = "33203442187324ebf8d0f4c47668ffe0f3c5aa2d918284dab46fdb4aa6991740"
# Independent integer encoding: (+x,-x,+y,-y,+z,-z), zero based.
V = ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1))
COLORS = tuple(product(range(2), repeat=3))


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


def validate(state):
    if type(state) is not QuotientState or type(state.record) is not Record:
        raise ValueError("expected exact quotient record")
    _integer(state.phase, "phase", high=11)
    if type(state.color) is not tuple or len(state.color) != 3:
        raise ValueError("invalid position color")
    for c in state.color:
        _integer(c, "color", high=1)
    r = state.record
    if type(r.path) is not tuple or len(r.path) > 2:
        raise ValueError("invalid path")
    for d in r.path:
        _integer(d, "path direction", high=5)
    if len(r.path) == 2 and r.path[0] == (r.path[1] ^ 1):
        raise ValueError("a flux path cannot immediately backtrack")
    if type(r.velocities) is not tuple or len(r.velocities) != 2:
        raise ValueError("invalid velocity tuple")
    if type(r.credits) is not tuple or len(r.credits) != 2:
        raise ValueError("invalid credit tuple")
    for v in r.velocities:
        _integer(v, "velocity", high=5)
    for k in r.credits:
        _integer(k, "credit", high=1)
    if sum(r.credits) + len(r.path) != 2:
        raise ValueError("record is outside the Q=2 sector")


@lru_cache(maxsize=1)
def records():
    paths = ((),) + tuple((a,) for a in range(6)) + tuple(
        (a, b) for a, b in product(range(6), repeat=2) if a != (b ^ 1))
    return tuple(Record(path, (vp, vm), (kp, km))
                 for path in paths for kp, km in product(range(2), repeat=2)
                 if kp + km == 2 - len(path)
                 for vp, vm in product(range(6), repeat=2))


@lru_cache(maxsize=1)
def _record_index():
    return {record: i for i, record in enumerate(records())}


STATE_COUNT = 148608


def decode(index):
    index = _integer(index, "node", high=STATE_COUNT - 1)
    pc, r = divmod(index, len(records()))
    phase, color = divmod(pc, 8)
    return QuotientState(phase, COLORS[color], records()[r])


def encode(state):
    validate(state)
    return (state.phase * 8 + COLORS.index(state.color)) * len(records()) + _record_index()[state.record]


def _transition(state):
    """Independent compact rule; return successor and positive translation."""
    r = state.record
    slot = state.phase // 6
    axis, parity = (state.phase % 6) // 2, state.phase % 2
    color = state.color
    active_color = color[axis]
    if slot:
        active_color = (active_color + sum(V[d][axis] for d in r.path)) % 2
    matched = 2 * axis + int(active_color != parity)
    v = r.velocities[slot]
    next_phase = (state.phase + 1) % 12
    if v != matched:
        return QuotientState(next_phase, color, r), (0, 0, 0)
    retract = (r.path[0] if slot == 0 else r.path[-1] ^ 1) if r.path else None
    path, velocities, credits = r.path, list(r.velocities), list(r.credits)
    displacement = (0, 0, 0)
    moved = False
    if v == retract:
        if credits[slot] == 0:
            path = path[1:] if slot == 0 else path[:-1]
            credits[slot] = 1
            moved = True
        else:
            velocities[slot] = v ^ 1
    elif credits[slot] == 1:
        path = (v ^ 1,) + path if slot == 0 else path + (v,)
        credits[slot] = 0
        moved = True
    else:
        if retract is None:
            raise AssertionError("Q=2 forbids a zero-credit co-located state")
        velocities[slot] = retract
    if moved and slot == 0:
        displacement = V[v]
        color = tuple((color[a] + displacement[a]) % 2 for a in range(3))
    result = QuotientState(next_phase, color, Record(path, tuple(velocities), tuple(credits)))
    return result, displacement


def transition(state):
    validate(state)
    result, displacement = _transition(state)
    validate(result)
    return result, displacement


def materialize(state, L=6, positive_position=None, microtick=None):
    """Independent exact lift for comparing the full frozen implementation."""
    from . import flux_binding as runtime
    validate(state)
    L = _integer(L, "periodic size", low=6)
    if L % 2:
        raise ValueError("even periodic size required")
    position = state.color if positive_position is None else positive_position
    if not isinstance(position, (tuple, list)) or len(position) != 3:
        raise ValueError("three position coordinates required")
    position = tuple(_integer(c, "position", high=L - 1) for c in position)
    if tuple(c % 2 for c in position) != state.color:
        raise ValueError("position does not have retained color")
    tick = state.phase if microtick is None else _integer(microtick, "ordinal")
    if tick % 12 != state.phase:
        raise ValueError("ordinal does not have retained phase")
    direction = np.zeros((L**3, 2), dtype=np.uint8)
    credit = np.zeros_like(direction)
    flux = np.zeros((L**3, 3), dtype=np.int8)
    def index(p):
        return (p[0] * L + p[1]) * L + p[2]
    current = position
    for d in state.record.path:
        target = tuple((current[a] + V[d][a]) % L for a in range(3))
        owner = current if d % 2 == 0 else target
        axis = d // 2
        if flux[index(owner), axis]:
            raise AssertionError("reused path edge")
        flux[index(owner), axis] = 1 if d % 2 == 0 else -1
        current = target
    for slot, point in enumerate((position, current)):
        direction[index(point), slot] = state.record.velocities[slot] + 1
        credit[index(point), slot] = state.record.credits[slot]
    return runtime.FluxState(L, tick, direction, credit, flux)


def verify_runtime_range(start, stop, L=6):
    """Every array/ordinal; no runtime decision helper is used by the oracle."""
    from . import flux_binding as runtime
    start = _integer(start, "range start", high=STATE_COUNT)
    stop = _integer(stop, "range stop", low=start, high=STATE_COUNT)
    runtime_path = Path(runtime.__file__)
    if hashlib.sha256(runtime_path.read_bytes()).hexdigest() != RUNTIME_SHA256:
        raise ValueError("registered runtime source changed")
    checked = 0
    digest = hashlib.sha256()
    for i in range(start, stop):
        q = decode(i)
        actual_input = materialize(q, L)
        before = tuple(getattr(actual_input, name).tobytes() for name in runtime.NAMES)
        successor, shift = transition(q)
        new_position = tuple((q.color[a] + shift[a]) % L for a in range(3))
        expected = materialize(successor, L, new_position, q.phase + 1)
        actual, _ = runtime.step(actual_input)
        if actual.microtick != expected.microtick:
            raise AssertionError(f"ordinal mismatch at quotient node {i}")
        for name, old_bytes in zip(runtime.NAMES, before):
            if not np.array_equal(getattr(actual, name), getattr(expected, name)):
                raise AssertionError(f"{name} mismatch at quotient node {i}")
            if getattr(actual_input, name).tobytes() != old_bytes:
                raise AssertionError(f"input mutation at quotient node {i}")
            if np.shares_memory(getattr(actual_input, name), getattr(actual, name)):
                raise AssertionError(f"output aliases input at quotient node {i}")
            digest.update(getattr(actual, name).tobytes())
        digest.update(bytes((actual.microtick,)))
        checked += 1
    return {"start": start, "stop": stop, "L": L, "checked": checked,
            "complete_outputs_sha256": digest.hexdigest()}


def graph_certificate():
    """Exhaust every transition/component; report counterexamples without filtering."""
    rs = records()
    if len(rs) != 1548 or len(set(rs)) != 1548:
        raise AssertionError("relative-record census failed")
    successor = np.empty(STATE_COUNT, dtype=np.int32)
    moves = np.zeros((STATE_COUNT, 3), dtype=np.int8)
    contact = np.zeros(STATE_COUNT, dtype=np.bool_)
    for i in range(STATE_COUNT):
        q = decode(i)
        out, shift = transition(q)
        j = encode(out)
        successor[i], moves[i] = j, shift
        contact[i] = not q.record.path
        # With at most two non-backtracking edges there is no wrapped shortcut.
        distance = sum(abs(sum(V[d][a] for d in out.record.path)) for a in range(3))
        if distance != len(out.record.path) or distance > 2:
            raise AssertionError("distance/account bound failed")
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
    first_hit = np.full(STATE_COUNT, -1, dtype=np.int32)
    transient = np.zeros(STATE_COUNT, dtype=np.int32)
    component = np.full(STATE_COUNT, -1, dtype=np.int32)
    cycles = []
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
        if i != cycle[0]:
            raise AssertionError("invalid recurrent component")
        locations = [k for k, node in enumerate(cycle) if contact[node]]
        if locations:
            for k, node in enumerate(cycle):
                first_hit[node] = min((j-k) % len(cycle) for j in locations)
            gap = max((locations[(k+1) % len(locations)] - j) % len(cycle)
                      or len(cycle) for k, j in enumerate(locations))
        else:
            gap = None
        translation = tuple(int(v) for v in moves[cycle].sum(axis=0))
        if any(v % 2 for v in translation):
            raise AssertionError("a quotient cycle must return its color")
        repeats_L6 = lcm(*(6 // gcd(6, abs(v)) for v in translation))
        cycles.append({"representative": min(cycle), "length": len(cycle),
                       "contact_states": len(locations), "largest_contact_spacing": gap,
                       "positive_translation": translation,
                       "full_state_period_L6": repeats_L6 * len(cycle),
                       "nodes_sha256": hashlib.sha256(np.asarray(cycle, dtype="<i4").tobytes()).hexdigest()})
    for i in reversed(removed):
        j = int(successor[i])
        component[i] = component[j]
        transient[i] = transient[j] + 1
        first_hit[i] = 0 if contact[i] else (first_hit[j] + 1 if first_hit[j] >= 0 else -1)
    if np.any(component < 0):
        raise AssertionError("not every state was classified")
    basins = np.bincount(component, minlength=len(cycles))
    for c, basin in zip(cycles, basins):
        c["basin_states"] = int(basin)
    nonreturn = [i for i, c in enumerate(cycles) if c["contact_states"] == 0]
    return {
        "schema": "strict-flux-binding-q2-graph-v1", "law": LAW_ID,
        "runtime_sha256": RUNTIME_SHA256, "periodic_sizes": "every even L>=6 by structural lift",
        "relative_records": len(rs), "states": STATE_COUNT, "transitions": STATE_COUNT,
        "records_by_path_length": dict(sorted(Counter(len(r.path) for r in rs).items())),
        "initial_contact_states": int(contact.sum()),
        "successors_sha256": hashlib.sha256(successor.astype("<i4").tobytes()).hexdigest(),
        "move_cocycle_sha256": hashlib.sha256(moves.tobytes()).hexdigest(),
        "recurrent_states": int(np.count_nonzero(indegree)), "components": len(cycles),
        "cycle_length_histogram": dict(sorted(Counter(c["length"] for c in cycles).items())),
        "maximum_transient_to_cycle": int(transient.max()),
        "first_contact_histogram": dict(sorted(Counter(map(int, first_hit)).items())),
        "states_that_never_contact": int(np.count_nonzero(first_hit < 0)),
        "noncontact_recurrent_components": nonreturn,
        "components_with_nonzero_translation": sum(any(c["positive_translation"]) for c in cycles),
        "all_future_distance_bound": 2,
        "cycles": cycles,
        "limits": {"physical_energy_identified": False, "unified_matter_fluid_law": False,
                   "many_particle_recovery": False, "canonical_adoption": False,
                   "quotient_cycle_is_automatically_absolute_cycle": False},
    }


def certify(output, workers=24):
    """Registered exhaustive graph plus every full runtime transition at L=6."""
    workers = _integer(workers, "workers", low=1, high=32)
    graph = graph_certificate()
    # Fixed partition independent of worker count, so evidence is reproducible.
    ranges = [(start, min(start + 1548, STATE_COUNT)) for start in range(0, STATE_COUNT, 1548)]
    if workers == 1:
        rows = [verify_runtime_range(a, b) for a, b in ranges]
    else:
        with ProcessPoolExecutor(max_workers=workers) as pool:
            futures = [pool.submit(verify_runtime_range, a, b) for a, b in ranges]
            rows = [f.result() for f in futures]
    checked = sum(row["checked"] for row in rows)
    if checked != STATE_COUNT:
        raise AssertionError("runtime comparison did not cover every state")
    result = {"graph": graph, "runtime": {"L": 6, "checked": checked,
               "complete_arrays_and_ordinal_equal": True, "workers": workers, "ranges": rows}}
    Path(output).write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return result


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True)
    parser.add_argument("--workers", type=int, default=24)
    args = parser.parse_args()
    result = certify(args.output, args.workers)
    print(json.dumps({k: v for k, v in result["graph"].items() if k != "cycles"}, indent=2))
    print(f"Complete runtime transitions compared: {result['runtime']['checked']}")
