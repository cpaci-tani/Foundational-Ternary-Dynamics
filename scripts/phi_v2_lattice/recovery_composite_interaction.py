"""Exact preparations and conservative Q4 observations for the frozen law.

No catalogue label enters evolution. Finite-torus and infinite-lift escape
are distinct sufficient certificates. Registered campaigns require their
separate execution/readiness lock; importing this module executes none.
"""
from dataclasses import dataclass, fields, replace
import hashlib
from io import BytesIO
from itertools import product
import json
from math import gcd, lcm
from pathlib import Path
from types import MappingProxyType

import numpy as np

from . import sparse_credit_exchange_q4 as S
from . import recovery_credit_exchange_q2 as Q2

CATALOGUE_SHA256 = "40f3eff75990315dd714bd7ea92dc66ac540337e377d04376f5be8696c29c1cc"
REPORT_SHA256 = "fad4744c13ccdb9a4c9431d35620301fb355cee317a443794fd4460c9b32480e"
AUDIT_SHA256 = "2823fa756aa87f13453275bf8ca487fbb16854b84e57f332133a245efe302e33"
Q2_SOURCE_SHA256 = "0330aa114bbb3d883642f104db71ffc8a08546e102b93c922bccdfee8589d40e"
DENSE_SOURCE_SHA256 = "9a317076455949c15b743e7a2576e2dcc9a0311c47825160bee188cc39f12489"
TIER_COUNTS = MappingProxyType({"head_on": 7296, "head_on_interventions": 80256, "full_impact": 1094400})
V, AXES, COLORS = S.V, S.AXES, Q2.COLORS
IDENTITY = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
EVENT_NAMES = tuple(f.name for f in fields(S.dense.ExchangeEvents))


def _sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


@dataclass(frozen=True)
class Catalogue:
    path: str
    sha256: str
    pins: tuple
    arrays: object


def load_catalogue(path, sha256):
    path = Path(path).resolve()
    archive = path.read_bytes()
    root = Path(__file__).resolve().parents[2]
    pins = ((path, CATALOGUE_SHA256), (path.with_name("report.json"), REPORT_SHA256),
            (root / "engine/docs/AUDIT_STRICT_CREDIT_EXCHANGE_Q2_RESULTS_V1.md", AUDIT_SHA256),
            (Path(Q2.__file__), Q2_SOURCE_SHA256), (Path(S.dense.__file__), DENSE_SOURCE_SHA256))
    if (sha256 != CATALOGUE_SHA256 or hashlib.sha256(archive).hexdigest() != CATALOGUE_SHA256
            or any(_sha(p) != h for p, h in pins[1:])):
        raise ValueError("unaccepted catalogue/source/report/audit identity")
    vector = (Q2.STATE_COUNT,)
    layout = {k: ("int32", vector) for k in ("component", "first_adjacency", "first_translating_family", "successor", "transient_to_cycle")}
    layout.update({k: ("bool", vector) for k in ("fresh_phase1_R2", "translating_family")})
    layout.update({k: ("uint8", vector) for k in ("operation", "path_length")})
    layout.update({k: ("int8", (Q2.STATE_COUNT, 3)) for k in ("minus_move", "plus_move")})
    layout["relative_records"] = ("uint8", (6192, 10))
    arrays = {}
    # Hash and parse one immutable byte snapshot; do not reopen a path that
    # another process could replace between verification and deserialization.
    with np.load(BytesIO(archive), allow_pickle=False) as z:
        if len(z.files) != 12 or set(z.files) != set(layout):
            raise ValueError("complete twelve-array catalogue required")
        for k, (dtype, shape) in layout.items():
            a = z[k]
            if a.dtype != np.dtype(dtype) or a.shape != shape:
                raise ValueError("catalogue layout mismatch: " + k)
            # Immutable bytes back each array; even setflags(write=True) fails.
            arrays[k] = np.frombuffer(a.tobytes(), dtype=dtype).reshape(shape)
    if (np.any(arrays["successor"] < 0) or np.any(arrays["successor"] >= Q2.STATE_COUNT)
            or np.any(arrays["component"] < 0) or np.any(arrays["component"] >= 48)
            or np.any(arrays["path_length"] > 2) or np.any(arrays["operation"] > 7)
            or any(np.any(np.abs(arrays[k]).sum(axis=1) > 1) for k in ("plus_move", "minus_move"))):
        raise ValueError("catalogue domain mismatch")
    return Catalogue(str(path), sha256, tuple((str(p), h) for p, h in pins), MappingProxyType(arrays))


@dataclass(frozen=True)
class Q2Control:
    L: int
    microtick: int
    origin_code: int
    charge_frame: int
    chart_plus_position: tuple
    node: int


@dataclass(frozen=True)
class _Payload:
    L: int
    microtick: int
    carriers: tuple
    edges: tuple
    origin_code: int
    charge_frame: int

    @property
    def phase(self):
        return self.microtick % 19


def _control(c):
    if type(c) is not Q2Control or type(c.chart_plus_position) is not tuple or len(c.chart_plus_position) != 3:
        raise ValueError("exact complete Q2Control required")
    L, tick = S.integer(c.L, "L", 6), S.integer(c.microtick, "ordinal")
    if L % 2:
        raise ValueError("even catalogue box required")
    origin, eta = S.integer(c.origin_code, "origin", high=47), S.integer(c.charge_frame, "eta", high=1)
    point = tuple(_signed(v) for v in c.chart_plus_position)
    node = S.integer(c.node, "node", high=Q2.STATE_COUNT-1)
    q = Q2.decode(node)
    color = tuple(((origin >> a) & 1) ^ (point[AXES[origin//8][a]] % 2) for a in range(3))
    if q.stage != tick % 19 or q.color != color:
        raise ValueError("control chart/ordinal/background mismatch")
    return Q2Control(L, tick, origin, eta, point, node)


def _signed(v):
    # No decimal-string conversion and no fixed word width.
    return S.integer(v, "signed coordinate", -abs(int(v)))


def _add(a, b):
    return tuple(x+y for x, y in zip(a, b))


def _sub(a, b):
    return tuple(x-y for x, y in zip(a, b))


def _physical(c, vector):
    out = [0, 0, 0]
    for a, axis in enumerate(AXES[c.origin_code//8]):
        out[axis] = int(vector[a])
    return tuple(out)


def control_payload(control):
    c = _control(control)
    q, position = Q2.decode(c.node), c.chart_plus_position
    edges = []
    for d in q.record.path:
        vector = _physical(c, V[d])
        target = _add(position, vector)
        axis = next(a for a in range(3) if vector[a])
        sign = vector[axis]
        owner = position if sign == 1 else target
        edges.append(S.Edge(S.site_index(c.L, owner), axis, sign * (-1 if c.charge_frame else 1)))
        position = target
    carriers = []
    for chartslot, point in enumerate((c.chart_plus_position, position)):
        direction = V.index(_physical(c, V[q.record.velocities[chartslot]]))+1
        carriers.append(S.Carrier(S.site_index(c.L, point), chartslot ^ c.charge_frame, direction,
                                  q.record.credits[chartslot], q.record.attempted[chartslot]))
    return _Payload(c.L, c.microtick, tuple(sorted(carriers)), tuple(sorted(edges)), c.origin_code, c.charge_frame)


def advance_control(control, catalogue, with_events=False):
    c = _control(control)
    dp = _physical(c, catalogue.arrays["plus_move"][c.node])
    result = _control(replace(c, microtick=c.microtick+1, chart_plus_position=_add(c.chart_plus_position, dp),
                              node=int(catalogue.arrays["successor"][c.node])))
    if not with_events:
        return result
    # Catalogue dictates the complete isolated successor. The event serializer
    # is independently checked against those records, never used to select it.
    carriers, edges, events = S._evolve(control_payload(c))
    expected = control_payload(result)
    if carriers != expected.carriers or edges != expected.edges:
        raise AssertionError("isolated event serializer differs from catalogue successor")
    return result, events


@dataclass(frozen=True)
class SuperpositionConflict:
    reason: str
    keys: tuple


def superpose_controls(a, b, catalogue):
    pa, pb = control_payload(a), control_payload(b)
    if (pa.L, pa.microtick, pa.origin_code, pa.charge_frame) != (pb.L, pb.microtick, pb.origin_code, pb.charge_frame):
        return SuperpositionConflict("context_or_clock", ())
    ck = set((c.site, c.slot) for c in pa.carriers) & set((c.site, c.slot) for c in pb.carriers)
    ek = set((e.owner, e.axis) for e in pa.edges) & set((e.owner, e.axis) for e in pb.edges)
    if ck or ek:
        return SuperpositionConflict("same_slot_or_shared_edge", (tuple(sorted(ck)), tuple(sorted(ek))))
    s = S.SparseQ4State(pa.L, pa.microtick, tuple(sorted(pa.carriers+pb.carriers)),
                        tuple(sorted(pa.edges+pb.edges)), pa.origin_code, pa.charge_frame)
    S.validate(s)
    return s


@dataclass(frozen=True, order=True)
class PreparationID:
    tier: str
    color: tuple
    v_A: int
    v_B: int
    sigma_A: int
    sigma_B: int
    branch: int
    m: int
    n: int
    stage_advance: int
    variant: int


def preparation_ids(tier):
    if tier not in TIER_COUNTS:
        raise ValueError("unknown registered tier")
    impacts = range(-2, 3) if tier == "full_impact" else (0,)
    variants = range(11) if tier == "head_on_interventions" else (0,)
    # Public heading IDs are 1..6, in the frozen physical direction order.
    for color in COLORS:
        for va in range(1, 7):
            vb_values = range(1, 7) if tier == "full_impact" else (((va-1)^1)+1,)
            for vb, sa, sb, branch, m, n, stage, variant in product(vb_values, (1, -1), (1, -1),
                                                                    range(2), impacts, impacts, range(19), variants):
                yield PreparationID(tier, color, va, vb, sa, sb, branch, m, n, stage, variant)


def _preparation(pid):
    if type(pid) is not PreparationID or pid.tier not in TIER_COUNTS:
        raise ValueError("exact registered PreparationID required")
    if type(pid.color) is not tuple or len(pid.color) != 3:
        raise ValueError("invalid preparation color")
    color = tuple(S.integer(v, "color", high=1) for v in pid.color)
    va, vb = S.integer(pid.v_A, "v_A", 1, 6), S.integer(pid.v_B, "v_B", 1, 6)
    sa, sb = S.integer(pid.sigma_A, "sigma_A", -1, 1), S.integer(pid.sigma_B, "sigma_B", -1, 1)
    if not sa or not sb:
        raise ValueError("nonzero trailing polarity required")
    branch, m, n = S.integer(pid.branch, "branch", high=1), S.integer(pid.m, "m", -2, 2), S.integer(pid.n, "n", -2, 2)
    stage = S.integer(pid.stage_advance, "stage", high=18)
    variant = S.integer(pid.variant, "variant", high=10 if pid.tier == "head_on_interventions" else 0)
    if pid.tier != "full_impact" and (vb != ((va-1)^1)+1 or m or n):
        raise ValueError("preparation outside registered tier")
    return PreparationID(pid.tier, color, va, vb, sa, sb, branch, m, n, stage, variant)


def preparation_displacement(pid):
    p = _preparation(pid)
    a, b = V[p.v_A-1], V[p.v_B-1]
    others = [V[2*j] for j in range(3) if a[j] == 0]
    if a == b:
        return tuple(4*others[0][j] + p.m*others[1][j] + (p.n+p.branch)*a[j] for j in range(3))
    if b == tuple(-v for v in a):
        u, v = others
    else:
        u = _add(a, b)
        v = (a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0])
    return tuple(4*(a[j]-b[j])+p.m*u[j]+p.n*v[j]+p.branch*b[j] for j in range(3))


def _seed(v, sigma, trail, origin, kick=None):
    lead, displaced = _add(trail, V[v-1]), trail if kick is None else _add(trail, V[kick-1])
    if sigma == 1:
        plus = displaced
        path = (() if kick is None else ((kick-1)^1,)) + (v-1,)
        credits = (1 if kick is None else 0, 0)
    else:
        plus = lead
        path = ((v-1)^1,) + (() if kick is None else (kick-1,))
        credits = (0, 1 if kick is None else 0)
    color = tuple(((origin >> a) & 1) ^ (plus[a] % 2) for a in range(3))
    q = Q2.QuotientState(0, color, Q2.Record(path, (v-1, v-1), credits, (0, 0)))
    return Q2Control(64, 0, origin, 0, plus, Q2.encode(q))


@dataclass(frozen=True)
class PreparedCase:
    preparation_id: PreparationID
    state: S.SparseQ4State
    controls: tuple
    provenance: tuple


def prepare(preparation_id, catalogue):
    p = _preparation(preparation_id)
    origin = sum(v << a for a, v in enumerate(p.color))
    a, b = (32, 32, 32), _add((32, 32, 32), preparation_displacement(p))
    ka = kb = None
    if 1 <= p.variant <= 5:
        ka = tuple(v for v in range(1, 7) if v != p.v_A)[p.variant-1]
    elif p.variant >= 6:
        kb = tuple(v for v in range(1, 7) if v != p.v_B)[p.variant-6]
    controls = (_seed(p.v_A, p.sigma_A, a, origin, ka), _seed(p.v_B, p.sigma_B, b, origin, kb))
    seeds = tuple(c.node for c in controls)
    for _ in range(1+p.stage_advance):
        controls = tuple(advance_control(c, catalogue) for c in controls)
    state = superpose_controls(*controls, catalogue)
    if type(state) is not S.SparseQ4State:
        raise AssertionError("registered independent preparation superposition failed")
    check_no_wrap(state, initial_lift(state))
    return PreparedCase(p, state, controls, (("catalogue_sha256", catalogue.sha256),
                         ("seed_nodes", seeds), ("lift_steps", 1+p.stage_advance),
                         ("final_nodes", tuple(c.node for c in controls))))


@dataclass(frozen=True)
class Stencil:
    output: tuple
    inputs: tuple


def _owners(payload):
    return {c.site for c in payload.carriers} | {e.owner for e in payload.edges}


def _neighbors(L, owners):
    return {S.site_index(L, _add(S.coordinates(L, x), d)) for x in owners for d in product((-1, 0, 1), repeat=3)}


def _slot_keys(x, slots=(0, 1)):
    return {(name, x, slot) for name in ("direction", "credit", "attempted") for slot in slots}


def _contexts(*sites):
    return {(name, x, 0) for x in sites for name in ("matching_frame", "charge_frame")}


def read_stencils(state):
    return _read_stencils(S._normalized(state))


def _read_stencils(s):
    outputs = []
    for x in sorted(_neighbors(s.L, _owners(s))):
        for name in ("matching_frame", "charge_frame"):
            key = (name, x, 0)
            outputs.append(Stencil(key, (key,)))
        for slot in range(2):
            for name in ("direction", "credit", "attempted"):
                key = (name, x, slot)
                support = {key}
                if s.phase == 0 and name in ("direction", "attempted"):
                    support |= _slot_keys(x) | {("charge_frame", x, 0)}
                elif 1 <= s.phase <= 6 and name in ("direction", "credit"):
                    owner, head, axis = S.matching_edge(s, x, (s.phase-1)//2, (s.phase-1)%2)
                    support |= _slot_keys(owner) | _slot_keys(head) | _contexts(owner, head) | {("flux", owner, axis)}
                elif s.phase >= 7 and slot == (((s.phase-7)//6) ^ s.charge_frame):
                    r = s.phase-7
                    owner, head, axis = S.matching_edge(s, x, (r%6)//2, r%2)
                    support |= _slot_keys(owner, (slot,)) | _slot_keys(head, (slot,)) | _contexts(owner, head) | {("flux", owner, axis)}
                    if name == "direction":
                        support |= {("flux", y, a) for a in range(3) for y in (x, S.shift(s.L, x, a, -1))}
                outputs.append(Stencil(key, tuple(sorted(support))))
        for axis in range(3):
            key, support = ("flux", x, axis), {("flux", x, axis)}
            if s.phase >= 7:
                r = s.phase-7
                owner, head, active = S.matching_edge(s, x, (r%6)//2, r%2)
                if owner == x and active == axis:
                    slot = (r//6) ^ s.charge_frame
                    support |= _slot_keys(owner, (slot,)) | _slot_keys(head, (slot,)) | _contexts(owner, head)
            outputs.append(Stencil(key, tuple(sorted(support))))
    return tuple(sorted(outputs, key=lambda row: row.output))


def _active_keys(payload):
    keys = {(name, c.site, c.slot) for c in payload.carriers for name in ("direction", "credit", "attempted")}
    return keys | {("flux", e.owner, e.axis) for e in payload.edges}


def event_record(events):
    if type(events) is not S.dense.ExchangeEvents:
        raise ValueError("complete frozen ExchangeEvents required")
    return tuple((name, tuple(tuple(row) for row in getattr(events, name))) for name in EVENT_NAMES)


def _merge_events(a, b, state):
    """Restore the frozen positive-owner serialization order, field by field."""
    result = S.dense.ExchangeEvents()
    for name in EVENT_NAMES:
        rows = list(getattr(a, name)) + list(getattr(b, name))
        if name == "attempt_expiries":
            key = lambda row: (row[1], 0 if row[0] == 1 else 1)
        elif name == "onsite_alignments":
            key = lambda row: row[0]
        elif name == "credit_exchanges":
            key = lambda row: row[0]
        elif name == "moves":
            key = lambda row: row[3]
        elif name == "capacity_holds":
            key = lambda row: row[1]
        else:
            r = state.phase-7
            key = lambda row: (S.matching_edge(state, row[1], (r%6)//2, r%2)[0],
                               0 if row[1] == S.matching_edge(state, row[1], (r%6)//2, r%2)[0] else 1)
        setattr(result, name, sorted(rows, key=key))
    return result


@dataclass(frozen=True)
class StepObservation:
    graph: S.Observation
    structural_contact: bool
    state_effect: bool
    event_difference: bool
    counterfactual_invalid: bool
    conflict: object
    contact_outputs: tuple


def observe_step(before, after, events, controls_before, controls_after, catalogue):
    before, after = S._normalized(before), S._normalized(after)
    if after.microtick != before.microtick+1 or len(controls_before) != 2 or len(controls_after) != 2:
        raise ValueError("one complete advancing step and two controls required")
    if (after.L, after.origin_code, after.charge_frame) != (before.L, before.origin_code, before.charge_frame):
        raise ValueError("joint step changed fixed context")
    for state, controls in ((before, controls_before), (after, controls_after)):
        for c in controls:
            c = _control(c)
            if (c.L, c.microtick, c.origin_code, c.charge_frame) != (state.L, state.microtick, state.origin_code, state.charge_frame):
                raise ValueError("joint/control context or ordinal mismatch")
    baseline_before = superpose_controls(*controls_before, catalogue)
    baseline_after = superpose_controls(*controls_after, catalogue)
    expected_after, isolated_events = [], []
    for c in controls_before:
        n, e = advance_control(c, catalogue, True)
        expected_after.append(n)
        isolated_events.append(e)
    if tuple(expected_after) != tuple(controls_after):
        raise ValueError("counterfactual successor not from exact catalogue")
    pa, pb = (control_payload(c) for c in controls_before)
    ka, kb = _active_keys(pa), _active_keys(pb)
    # Include actual and isolated active neighborhoods after divergence.
    stencil_state = (_Payload(pa.L, pa.microtick, pa.carriers+pb.carriers, pa.edges+pb.edges,
                              pa.origin_code, pa.charge_frame) if isinstance(baseline_before, SuperpositionConflict) else baseline_before)
    contact = tuple(st.output for st in _read_stencils(stencil_state)
                    if ka.intersection(st.inputs) and kb.intersection(st.inputs))
    invalid = isinstance(baseline_after, SuperpositionConflict)
    conflict = baseline_after if invalid else baseline_before if isinstance(baseline_before, SuperpositionConflict) else None
    effect = not invalid and S.checkpoint(after) != S.checkpoint(baseline_after)
    event_difference = False
    if not invalid and not isinstance(baseline_before, SuperpositionConflict):
        event_difference = event_record(events) != event_record(_merge_events(*isolated_events, baseline_before))
    return StepObservation(S.observe(after), bool(contact) or conflict is not None, effect, event_difference,
                           invalid, conflict, contact)


@dataclass(frozen=True)
class IntegerLift:
    carriers: tuple
    edges: tuple


def initial_lift(state):
    state = S._normalized(state)
    return IntegerLift(tuple(S.coordinates(state.L, c.site) for c in state.carriers),
                       tuple(S.coordinates(state.L, e.owner) for e in state.edges))


def _normalized_lift(lift):
    if type(lift) is not IntegerLift or type(lift.carriers) is not tuple or type(lift.edges) is not tuple:
        raise ValueError("explicit immutable integer lift required")
    rows = []
    for positions in (lift.carriers, lift.edges):
        points = []
        for p in positions:
            if type(p) is not tuple or len(p) != 3:
                raise ValueError("three immutable integer coordinates required")
            points.append(tuple(_signed(v) for v in p))
        rows.append(tuple(points))
    return IntegerLift(*rows)


def validate_lift(state, lift):
    state, lift = S._normalized(state), _normalized_lift(lift)
    if len(lift.carriers) != 4 or len(lift.edges) != len(state.edges):
        raise ValueError("lift row count mismatch")
    for positions, records, key in ((lift.carriers, state.carriers, "site"), (lift.edges, state.edges, "owner")):
        for p, record in zip(positions, records):
            if type(p) is not tuple or len(p) != 3 or S.site_index(state.L, tuple(_signed(v) for v in p)) != getattr(record, key):
                raise ValueError("lift row does not match complete state")
    charge = tuple(sum((1 if c.slot == 0 else -1)*p[a] for c, p in zip(state.carriers, lift.carriers)) for a in range(3))
    flux = tuple(-sum(e.q for e in state.edges if e.axis == a) for a in range(3))
    if charge != flux:
        raise ValueError("lift does not satisfy unwrapped charge-dipole incidence")
    # Site lifts must agree for every incidence, not merely after cancellation.
    assigned = {}
    for c, point in zip(state.carriers, lift.carriers):
        if c.site in assigned and assigned[c.site] != point:
            raise ValueError("inconsistent colocated lift")
        assigned[c.site] = point
    for e, point in zip(state.edges, lift.edges):
        for site, p in ((e.owner, point), (S.shift(state.L, e.owner, e.axis, 1), _add(point, V[2*e.axis]))):
            if site in assigned and assigned[site] != p:
                raise ValueError("inconsistent edge endpoint lift")
            assigned[site] = p
    return charge


def advance_lift(before, after, events, lift):
    before, after, lift = S._normalized(before), S._normalized(after), _normalized_lift(lift)
    validate_lift(before, lift)
    points = {(c.site, c.slot): p for c, p in zip(before.carriers, lift.carriers)}
    edges = {(e.owner, e.axis): p for e, p in zip(before.edges, lift.edges)}
    original = dict(points)
    for epsilon, source, destination, owner, axis, q, qn, k, kn in events.moves:
        slot = 0 if epsilon == 1 else 1
        sigma = 1 if source == owner else -1
        p = original[source, slot]
        points.pop((source, slot))
        points[destination, slot] = _add(p, V[2*axis+(sigma == -1)])
        if qn:
            edges[owner, axis] = p if sigma == 1 else _add(p, V[2*axis+1])
        else:
            edges.pop((owner, axis), None)
    out = IntegerLift(tuple(points[c.site, c.slot] for c in after.carriers),
                      tuple(edges[e.owner, e.axis] for e in after.edges))
    validate_lift(after, out)
    return out


def check_no_wrap(state, lift):
    state, lift = S._normalized(state), _normalized_lift(lift)
    charge = validate_lift(state, lift)
    points = list(lift.carriers)+list(lift.edges)
    points += [_add(p, V[2*e.axis]) for e, p in zip(state.edges, lift.edges)]
    if state.L != 64 or any(not 2 <= v <= 62 for p in points for v in p):
        raise ValueError("registered no-wrap support bound failed")
    return charge


@dataclass(frozen=True)
class LiftObservation:
    charge_dipole: tuple
    carrier_span: tuple
    plus_coordinate_sum: tuple
    minus_coordinate_sum: tuple
    plus_displacement: tuple
    minus_displacement: tuple
    signed_displacement: tuple


def observe_lift(state, lift, reference_state=None, reference_lift=None):
    """Exact coordinate ledger; none of these sums is mechanical momentum."""
    state, lift = S._normalized(state), _normalized_lift(lift)
    charge = validate_lift(state, lift)
    sums = tuple(tuple(sum(p[a] for c, p in zip(state.carriers, lift.carriers) if c.slot == slot)
                       for a in range(3)) for slot in range(2))
    span = tuple(max(p[a] for p in lift.carriers)-min(p[a] for p in lift.carriers) for a in range(3))
    if (reference_state is None) != (reference_lift is None):
        raise ValueError("both reference state and lift are required")
    if reference_state is None:
        displacement = ((0, 0, 0), (0, 0, 0))
    else:
        reference_state, reference_lift = S._normalized(reference_state), _normalized_lift(reference_lift)
        validate_lift(reference_state, reference_lift)
        if (state.L, state.origin_code, state.charge_frame) != (reference_state.L, reference_state.origin_code, reference_state.charge_frame):
            raise ValueError("displacement reference context mismatch")
        previous = tuple(tuple(sum(p[a] for c, p in zip(reference_state.carriers, reference_lift.carriers) if c.slot == slot)
                               for a in range(3)) for slot in range(2))
        displacement = tuple(_sub(a, b) for a, b in zip(sums, previous))
    return LiftObservation(charge, span, *sums, *displacement, _sub(*displacement))


def _component_control(state, component):
    if component.populations != (1, 1) or component.Q != 2 or len(component.edges) > 2:
        raise ValueError("component is not a short Q2 chart")
    eta = state.charge_frame
    plus = next(c for c in component.carriers if c.slot == eta)
    minus = next(c for c in component.carriers if c.slot != eta)
    edges = {(e.owner, e.axis): e.q*(-1 if eta else 1) for e in component.edges}
    current, path, used = plus.site, [], set()
    axes = AXES[state.origin_code//8]
    while current != minus.site:
        ports = []
        for d in range(6):
            owner = current if d % 2 == 0 else S.shift(state.L, current, d//2, -1)
            if edges.get((owner, d//2), 0)*(1 if d % 2 == 0 else -1) == 1:
                ports.append((d, owner))
        if len(ports) != 1 or len(path) >= 2:
            raise ValueError("component lacks a unique Q2 path")
        d, owner = ports[0]
        used.add((owner, d//2))
        path.append(2*axes.index(d//2)+d%2)
        current = S.shift(state.L, current, d//2, 1 if d % 2 == 0 else -1)
    if used != set(edges):
        raise ValueError("component contains extra edges")
    code = S.background_code(state, plus.site)
    internal = lambda d: 2*axes.index((d-1)//2)+(d-1)%2
    q = Q2.QuotientState(state.phase, tuple((code >> a) & 1 for a in range(3)),
          Q2.Record(tuple(path), (internal(plus.direction), internal(minus.direction)),
                    (plus.credit, minus.credit), (plus.attempted, minus.attempted)))
    return _control(Q2Control(state.L, state.microtick, state.origin_code, eta,
                              S.coordinates(state.L, plus.site), Q2.encode(q)))


def _control_support(control):
    control = _control(control)
    payload = control_payload(control)
    # Exact unwrapped owners are reconstructed from the short path, then
    # the actual phase's declared output AND input owners are retained.
    # These guard supports are conservative; they need not be active data.
    q, position = Q2.decode(control.node), control.chart_plus_position
    owners = {position}
    for d in q.record.path:
        vector = _physical(control, V[d])
        target = _add(position, vector)
        owners.add(position if 1 in vector else target)
        position = target
    owners.add(position)
    centers = {_add(x, d) for x in owners for d in product((-1, 0, 1), repeat=3)}
    by_site = {}
    for point in centers:
        by_site.setdefault(S.site_index(control.L, point), set()).add(point)
    support = set(owners) | centers
    for stencil in _read_stencils(payload):
        output = S.coordinates(control.L, stencil.output[1])
        for key in stencil.inputs:
            point = S.coordinates(control.L, key[1])
            delta = tuple((point[a]-output[a]+1) % control.L-1 for a in range(3))
            if any(abs(v) > 1 for v in delta):
                raise AssertionError("declared stencil input exceeds radius one")
            support.update(_add(center, delta) for center in by_site[stencil.output[1]])
    return tuple(sorted(support))


def _interval(w, slope):
    low, high = 0, None
    for x, d in zip(w, slope):
        if d == 0:
            if abs(x) > 2:
                return None
            continue
        if d > 0:
            # ceil(a/d) == -((-a)//d), independent of signed numerator.
            lo, hi = -((2+x)//d), (2-x)//d
        else:
            lo, hi = -((2-x)//(-d)), (-2-x)//d
        low = max(low, lo)
        high = hi if high is None else min(high, hi)
        if high < low:
            return None
    return (low, high)


@dataclass(frozen=True)
class EscapeResult:
    applicable: bool
    infinite_lift: bool
    periodic_torus: bool
    controls: tuple
    drifts: tuple
    support_charts: tuple
    interval_witness: object
    residue_witness: object
    modular_period: int
    reason: str


def escape_certificate(state, catalogue):
    s = S._normalized(state)
    components = S.observe(s).components
    if len(components) != 2:
        return EscapeResult(False, False, False, (), (), (), None, None, 0, "not_two_components")
    try:
        controls = tuple(_component_control(s, c) for c in components)
    except ValueError:
        return EscapeResult(False, False, False, (), (), (), None, None, 0, "not_two_Q2_components")
    if any(int(catalogue.arrays["transient_to_cycle"][c.node]) != 0 for c in controls):
        return EscapeResult(False, False, False, controls, (), (), None, None, 0, "nonrecurrent_component")
    current, charts = controls, []
    for _ in range(38):
        charts.append(tuple(_control_support(c) for c in current))
        current = tuple(advance_control(c, catalogue) for c in current)
    block = tuple(_sub(b.chart_plus_position, a.chart_plus_position) for a, b in zip(controls, current))
    if any(a.node != b.node for a, b in zip(controls, current)) or any(sum(abs(x) for x in d) != 2 or any(x % 2 for x in d) for d in block):
        raise AssertionError("accepted recurrent catalogue cycle/cocycle differs")
    drifts = tuple(tuple(v//2 for v in d) for d in block)
    slope = _sub(block[1], block[0])
    period = lcm(*(s.L//gcd(s.L, abs(v)) if v else 1 for v in slope))
    infinite_hit = torus_hit = None
    for phase, (aa, bb) in enumerate(charts):
        # Exact difference set removes duplicate pairs; no position rounding.
        differences = sorted({_sub(b, a) for a in aa for b in bb})
        for w in differences:
            if infinite_hit is None:
                interval = _interval(w, slope)
                if interval is not None:
                    infinite_hit = (phase, w, slope, interval)
            if torus_hit is None:
                for n in range(period):
                    residue = tuple((w[j]+n*slope[j]) % s.L for j in range(3))
                    if all(min(v, s.L-v) <= 2 for v in residue):
                        torus_hit = (phase, w, slope, n, residue)
                        break
            if infinite_hit is not None and torus_hit is not None:
                break
        if infinite_hit is not None and torus_hit is not None:
            break
    return EscapeResult(True, infinite_hit is None, torus_hit is None, controls, tuple(sorted(drifts)),
                         tuple(charts), infinite_hit, torus_hit, period, "exact_sufficient_support_test")


@dataclass(frozen=True)
class CaptureWitness:
    start: int
    end: int
    ordinal_start: int
    ordinal_end: int
    period: int
    translation: tuple
    translating: bool
    maximum_diameter: int
    joined_offsets: tuple


@dataclass(frozen=True)
class CaptureResult:
    witnesses: tuple


def relative_translation_matches(a, la, b, lb):
    """Complete endpoint equality only; this alone does not certify a period."""
    a, b = S._normalized(a), S._normalized(b)
    la, lb = _normalized_lift(la), _normalized_lift(lb)
    validate_lift(a, la)
    validate_lift(b, lb)
    if (a.L, a.origin_code, a.charge_frame) != (b.L, b.origin_code, b.charge_frame):
        return ()
    if b.microtick <= a.microtick or (b.microtick-a.microtick) % 19:
        return ()
    candidates = {_sub(pb, pa) for ca, pa in zip(a.carriers, la.carriers) for cb, pb in zip(b.carriers, lb.carriers)
                  if (ca.slot, ca.direction, ca.credit, ca.attempted) == (cb.slot, cb.direction, cb.credit, cb.attempted)}
    target_c = sorted((p, c.slot, c.direction, c.credit, c.attempted) for c, p in zip(b.carriers, lb.carriers))
    target_e = sorted((p, e.axis, e.q) for e, p in zip(b.edges, lb.edges))
    matches = []
    for d in sorted(candidates):
        if any(v % 2 for v in d):
            continue
        if sorted((_add(p, d), c.slot, c.direction, c.credit, c.attempted) for c, p in zip(a.carriers, la.carriers)) != target_c:
            continue
        if sorted((_add(p, d), e.axis, e.q) for e, p in zip(a.edges, la.edges)) != target_e:
            continue
        shifted = S.transform(a, IDENTITY, d)
        if S.checkpoint(replace(shifted, microtick=b.microtick)) == S.checkpoint(b):
            matches.append(d)
    return tuple(matches)


def capture_certificate(history, integer_lifts):
    if len(history) != len(integer_lifts):
        raise ValueError("complete history/lift correspondence required")
    history = tuple(S._normalized(s) for s in history)
    integer_lifts = tuple(_normalized_lift(lift) for lift in integer_lifts)
    for i, (s, lift) in enumerate(zip(history, integer_lifts)):
        validate_lift(s, lift)
        if i and (s.microtick != history[i-1].microtick+1 or (s.L, s.origin_code, s.charge_frame) !=
                  (history[0].L, history[0].origin_code, history[0].charge_frame)):
            raise ValueError("consecutive complete history required")
        if i:
            expected, events = S.step(history[i-1])
            if expected != s or advance_lift(history[i-1], s, events, integer_lifts[i-1]) != lift:
                raise ValueError("capture history is not the complete local-law trajectory")
    diameters = [max(max(p[a] for p in lift.carriers)-min(p[a] for p in lift.carriers) for a in range(3)) for lift in integer_lifts]
    joined = [any(len(c.carriers) == 4 for c in S.observe(s).components) for s in history]
    witnesses = []
    for end, b in enumerate(history):
        for start in range(end):
            a, T = history[start], b.microtick-history[start].microtick
            if T % 19 or max(diameters[start:end+1]) > 8 or not any(joined[start:end+1]):
                continue
            la, lb = integer_lifts[start], integer_lifts[end]
            for d in relative_translation_matches(a, la, b, lb):
                witnesses.append(CaptureWitness(start, end, a.microtick, b.microtick, T, d, any(d),
                                  max(diameters[start:end+1]), tuple(i-start for i in range(start, end+1) if joined[i])))
    return CaptureResult(tuple(witnesses))


@dataclass(frozen=True)
class CaseResult:
    preparation_id: PreparationID
    provenance: tuple
    completed_ticks: int
    checkpoints: tuple
    hash_chain: tuple
    first_contact: object
    first_effect: object
    first_event_difference: object
    first_counterfactual_invalid: object
    escape: EscapeResult
    capture: CaptureResult
    graph: S.Observation
    accounts: tuple
    drift_multiset: tuple
    classification: str
    lift_observations: tuple


class EvaluationFailure(RuntimeError):
    """Retained reconstructible prefix; never a completed scientific case."""
    def __init__(self, preparation_id, checkpoints, chain, failure):
        super().__init__("incomplete composite evaluation: " + failure)
        self.preparation_id = preparation_id
        self.checkpoints = tuple(checkpoints)
        self.hash_chain = tuple(chain)
        self.completed_ticks = len(self.hash_chain)
        self.failure = failure


def evaluate_case(preparation_id, catalogue, H=304, L=64):
    if S.integer(H, "H") != 304 or S.integer(L, "L") != 64:
        raise ValueError("registered evaluator requires H304/L64")
    prepared = prepare(preparation_id, catalogue)
    state, controls = prepared.state, prepared.controls
    history, lifts = [state], [initial_lift(state)]
    checkpoints, chain = [S.checkpoint(state)], []
    first, digest, accounts = [None]*4, bytes(32), []
    lift_observations = [observe_lift(state, lifts[0])]
    for offset in range(304):
        try:
            after, events = S.step(state)
            following = tuple(advance_control(c, catalogue) for c in controls)
            observation = observe_step(state, after, events, controls, following, catalogue)
            lift = advance_lift(state, after, events, lifts[-1])
            check_no_wrap(after, lift)
            lift_observation = observe_lift(after, lift, history[0], lifts[0])
            raw = S.checkpoint(after)
            blob = repr((event_record(events), observation, lift_observation)).encode("ascii")
            next_digest = hashlib.sha256(digest+len(raw).to_bytes(8, "big")+raw+blob).digest()
        except Exception as exc:
            raise EvaluationFailure(prepared.preparation_id, checkpoints, chain,
                                    type(exc).__name__+": "+str(exc)) from exc
        flags = (observation.structural_contact, observation.state_effect, observation.event_difference, observation.counterfactual_invalid)
        for i, flag in enumerate(flags):
            if flag and first[i] is None:
                first[i] = offset+1
        # All 305 checkpoints are retained: complete bounded reconstruction,
        # no hidden snapshot elision or early termination after certificates.
        digest = next_digest
        chain.append(digest.hex())
        checkpoints.append(raw)
        accounts.append((observation.graph.F, observation.graph.K, observation.graph.Q))
        history.append(after)
        lifts.append(lift)
        lift_observations.append(lift_observation)
        state, controls = after, following
    try:
        escape = escape_certificate(state, catalogue)
        capture = capture_certificate(tuple(history), tuple(lifts))
    except Exception as exc:
        raise EvaluationFailure(prepared.preparation_id, checkpoints, chain,
                                type(exc).__name__+": "+str(exc)) from exc
    classification = "RELATIVE_PERIODIC_CAPTURE" if capture.witnesses else "CERTIFIED_INDEPENDENT_ESCAPE" if escape.infinite_lift or escape.periodic_torus else "UNRESOLVED_AT_304"
    return CaseResult(prepared.preparation_id, prepared.provenance, 304, tuple(checkpoints), tuple(chain), *first,
                      escape, capture, S.observe(state), tuple(accounts), escape.drifts, classification, tuple(lift_observations))
