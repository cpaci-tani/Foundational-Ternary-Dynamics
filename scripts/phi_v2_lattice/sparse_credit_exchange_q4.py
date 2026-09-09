"""Exact Q4 compression of the frozen nineteen-tick credit-exchange law.

This is an offline backend, not a new law or a physical matter identification.
All nonzero edges are records, including detached circulation and junctions.
"""
from dataclasses import dataclass, field, replace
from itertools import permutations
import json

import numpy as np

from . import credit_exchange_binding as dense

LAW_ID, RULE_HASH = dense.LAW_ID, dense.RULE_HASH
FRAME_HASH, DENSE_ENCODING = dense.FRAME_HASH, dense.ENCODING_HASH
BACKEND_ID = "python-credit-exchange-q4-sparse-1"
SPARSE_ENCODING = "credit-exchange-q4-sparse-records-1"
SCHEMA = "ftd-credit-exchange-q4-sparse-checkpoint-1"
MAGIC = b"FTD-CREDIT-EXCHANGE-Q4-SPARSE-1\n"
ACCOUNT_ID = dense.ACCOUNT_ID
AXES = tuple(permutations(range(3)))
V = ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1))
_INTEGER_TYPES = frozenset((int, np.int8, np.int16, np.int32, np.int64,
                            np.uint8, np.uint16, np.uint32, np.uint64,
                            np.intp, np.uintp, np.longlong, np.ulonglong))


def integer(x, name, low=0, high=None):
    if not any(type(x) is allowed for allowed in _INTEGER_TYPES):
        raise ValueError("integer required: " + name)
    x = int(x)
    if x < low or (high is not None and x > high):
        raise ValueError("integer outside domain: " + name)
    return x


@dataclass(frozen=True, order=True)
class Carrier:
    site: int
    slot: int
    direction: int
    credit: int
    attempted: int


@dataclass(frozen=True, order=True)
class Edge:
    owner: int
    axis: int
    q: int


@dataclass(frozen=True)
class SparseQ4State:
    L: int
    microtick: int
    carriers: tuple
    edges: tuple
    origin_code: int
    charge_frame: int
    law_id: str = field(default=LAW_ID, init=False)
    rule_hash: str = field(default=RULE_HASH, init=False)
    frame_hash: str = field(default=FRAME_HASH, init=False)
    encoding_hash: str = field(default=DENSE_ENCODING, init=False)
    boundary: str = field(default="periodic", init=False)
    backend_id: str = field(default=BACKEND_ID, init=False)
    sparse_encoding: str = field(default=SPARSE_ENCODING, init=False)

    @property
    def phase(self):
        return int(self.microtick) % 19


def coordinates(L, site):
    return (site // (L * L), (site // L) % L, site % L)


def site_index(L, point):
    x, y, z = (int(v) % L for v in point)
    return (x * L + y) * L + z


def shift(L, site, axis, sign):
    p = list(coordinates(L, site))
    p[axis] += sign
    return site_index(L, p)


def background_code(state, site):
    axes = AXES[int(state.origin_code) // 8]
    p = coordinates(int(state.L), int(site))
    return int(state.origin_code) ^ sum((p[axes[a]] % 2) << a for a in range(3))


def matching_edge(state, site, column, parity):
    code = background_code(state, site)
    axis = AXES[code // 8][column]
    owner = site if ((code >> column) & 1) == parity else shift(state.L, site, axis, -1)
    return owner, shift(state.L, owner, axis, 1), axis


def _normalized(state):
    if type(state) is not SparseQ4State:
        raise ValueError("expected exact SparseQ4State")
    for key, expected in (("law_id", LAW_ID), ("rule_hash", RULE_HASH),
                          ("frame_hash", FRAME_HASH), ("encoding_hash", DENSE_ENCODING),
                          ("boundary", "periodic"), ("backend_id", BACKEND_ID),
                          ("sparse_encoding", SPARSE_ENCODING)):
        if type(getattr(state, key)) is not str or getattr(state, key) != expected:
            raise ValueError("incompatible sparse identity: " + key)
    L = integer(state.L, "L", 4)
    if L % 2:
        raise ValueError("even periodic L required")
    tick = integer(state.microtick, "microtick")
    origin, eta = integer(state.origin_code, "origin_code", high=47), integer(state.charge_frame, "eta", high=1)
    if type(state.carriers) is not tuple or len(state.carriers) != 4:
        raise ValueError("four immutable carrier records required")
    if type(state.edges) is not tuple or len(state.edges) > 4:
        raise ValueError("at most four immutable edge records required")
    cs, es = [], []
    for c in state.carriers:
        if type(c) is not Carrier:
            raise ValueError("exact Carrier required")
        cs.append(Carrier(integer(c.site, "site", high=L**3-1), integer(c.slot, "slot", high=1),
                          integer(c.direction, "direction", 1, 6), integer(c.credit, "credit", high=1),
                          integer(c.attempted, "attempted", high=1)))
    for e in state.edges:
        if type(e) is not Edge:
            raise ValueError("exact Edge required")
        q = integer(e.q, "q", -1, 1)
        if q == 0:
            raise ValueError("zero edges are omitted")
        es.append(Edge(integer(e.owner, "owner", high=L**3-1), integer(e.axis, "axis", high=2), q))
    ck, ek = [(c.site, c.slot) for c in cs], [(e.owner, e.axis) for e in es]
    if ck != sorted(set(ck)) or ek != sorted(set(ek)):
        raise ValueError("strict canonical order and unique record keys required")
    if sum(c.slot == 0 for c in cs) != 2 or len(es) + sum(c.credit for c in cs) != 4:
        raise ValueError("outside two-plus/two-minus Q4 sector")
    residual = {}
    for c in cs:
        residual[c.site] = residual.get(c.site, 0) - (1 if c.slot == 0 else -1)
    for e in es:
        head = shift(L, e.owner, e.axis, 1)
        residual[e.owner] = residual.get(e.owner, 0) + e.q
        residual[head] = residual.get(head, 0) - e.q
    if any(residual.values()):
        raise ValueError("Gauss incidence mismatch")
    return SparseQ4State(L, tick, tuple(cs), tuple(es), origin, eta)


def validate(state):
    _normalized(state)


def initialize(L, carriers, edges, origin_code, charge_frame):
    # Copy the containers before sorting; duplicate keys remain errors.
    try:
        cs = tuple(sorted(tuple(carriers), key=lambda c: (c.site, c.slot)))
        es = tuple(sorted(tuple(edges), key=lambda e: (e.owner, e.axis)))
    except (TypeError, AttributeError) as exc:
        raise ValueError("invalid sparse records") from exc
    return _normalized(SparseQ4State(L, 0, cs, es, origin_code, charge_frame))


def from_dense(state):
    dense.validate(state)
    cs = tuple(Carrier(int(x), int(s), int(state.direction[x, s]), int(state.credit[x, s]),
                       int(state.attempted[x, s])) for x, s in np.argwhere(state.direction != 0))
    es = tuple(Edge(int(x), int(a), int(state.flux[x, a])) for x, a in np.argwhere(state.flux != 0))
    return _normalized(SparseQ4State(state.L, state.microtick, cs, es,
                                    int(state.matching_frame[0]), int(state.charge_frame[0])))


def to_dense(state):
    s = _normalized(state)
    n = s.L**3
    direction = np.zeros((n, 2), dtype=np.uint8)
    credit, attempted = np.zeros_like(direction), np.zeros_like(direction)
    flux = np.zeros((n, 3), dtype=np.int8)
    for c in s.carriers:
        direction[c.site, c.slot], credit[c.site, c.slot], attempted[c.site, c.slot] = c.direction, c.credit, c.attempted
    for e in s.edges:
        flux[e.owner, e.axis] = e.q
    background = np.fromiter((background_code(s, x) for x in range(n)), dtype=np.uint8, count=n)
    eta = np.full(n, s.charge_frame, dtype=np.uint8)
    result = dense.ExchangeState(s.L, s.microtick, direction, credit, flux, background, eta, attempted)
    dense.validate(result)
    return result


pack, unpack = from_dense, to_dense


def step(state):
    s = _normalized(state)
    carriers, edges, events = _evolve(s)
    result = SparseQ4State(s.L, s.microtick+1, carriers, edges, s.origin_code, s.charge_frame)
    validate(result)
    return result, events


def _evolve(s):
    """Record transaction shared with validated, external isolated Q2 controls.

    Public Q4 admission always happens in step; this helper has no cache and
    returns records, never an implicitly admitted cross-sector state.
    """
    cs = {(c.site, c.slot): c for c in s.carriers}
    es = {(e.owner, e.axis): e.q for e in s.edges}
    out, flux = dict(cs), dict(es)
    events = dense.ExchangeEvents()
    p, L = s.phase, s.L
    if p == 0:
        for c in s.carriers:
            out[c.site, c.slot] = replace(c, attempted=0)
            if c.attempted:
                events.attempt_expiries.append((1 if c.slot == 0 else -1, c.site))
        for x in sorted({c.site for c in s.carriers}):
            if (x, 0) in cs and (x, 1) in cs:
                heading = cs[x, s.charge_frame].direction
                for slot in range(2):
                    out[x, slot] = replace(out[x, slot], direction=heading)
                events.onsite_alignments.append((x, 1 if s.charge_frame == 0 else -1,
                                                 cs[x, 0].direction, cs[x, 1].direction, heading))
    elif p <= 6:
        column, parity = (p-1)//2, (p-1)%2
        for e in s.edges:
            owner, head, axis = matching_edge(s, e.owner, column, parity)
            if owner != e.owner or axis != e.axis:
                continue
            ls, rs = (0, 1) if e.q == 1 else (1, 0)
            left, right = cs.get((owner, ls)), cs.get((head, rs))
            if left is None or right is None or left.credit == right.credit:
                continue
            donor, recipient, heading = (left, right, 2*axis+1) if left.credit else (right, left, 2*axis+2)
            out[donor.site, donor.slot] = replace(donor, credit=0, direction=heading)
            out[recipient.site, recipient.slot] = replace(recipient, credit=1, direction=heading)
            events.credit_exchanges.append((owner, axis, 1 if donor.slot == 0 else -1, donor.site,
                                            1 if recipient.slot == 0 else -1, recipient.site, 1, 0, 0, 1,
                                            donor.direction, recipient.direction, heading))
    else:
        r = p-7
        slot, column, parity = (r//6)^s.charge_frame, (r%6)//2, r%2
        epsilon = 1 if slot == 0 else -1
        matching = sorted({matching_edge(s, c.site, column, parity) for c in s.carriers if c.slot == slot})
        for owner, head, axis in matching:
            left, right = cs.get((owner, slot)), cs.get((head, slot))
            if left is not None and right is not None:
                events.capacity_holds.append((epsilon, owner, axis))
                for c, pointed in ((left, 2*axis+1), (right, 2*axis+2)):
                    if c.direction == pointed and not c.attempted:
                        out[c.site, slot] = replace(c, attempted=1)
                        events.attempt_marks.append((epsilon, c.site, "capacity"))
                continue
            c, destination, sigma = (left, head, 1) if left is not None else (right, owner, -1)
            if c.attempted or c.direction != 2*axis+(1 if sigma == 1 else 2):
                continue
            q, k = es.get((owner, axis), 0), c.credit
            qn = q - epsilon*sigma
            kn = k - (qn*qn-q*q)
            if -1 <= qn <= 1 and 0 <= kn <= 1:
                del out[c.site, slot]
                out[destination, slot] = replace(c, site=destination, credit=kn, attempted=1)
                if qn:
                    flux[owner, axis] = qn
                else:
                    flux.pop((owner, axis), None)
                events.moves.append((epsilon, c.site, destination, owner, axis, q, qn, k, kn))
                reason = "accepted"
            else:
                ports = []
                for a in range(3):
                    if es.get((c.site, a), 0) == epsilon:
                        ports.append(2*a+1)
                    if -es.get((shift(L, c.site, a, -1), a), 0) == epsilon:
                        ports.append(2*a+2)
                heading = ports[0] if k == 0 and len(ports) == 1 else ((c.direction-1)^1)+1
                out[c.site, slot] = replace(c, direction=heading, attempted=1)
                reason = "flux_capacity" if abs(qn) > 1 else "credit_deficit" if kn < 0 else "credit_capacity"
                events.redirects.append((epsilon, c.site, c.direction, heading, reason))
            events.attempt_marks.append((epsilon, c.site, reason))
    return (tuple(out[k] for k in sorted(out)), tuple(Edge(*k, flux[k]) for k in sorted(flux)), events)


def _payload(s):
    return dict(schema=SCHEMA, backend=BACKEND_ID, law=LAW_ID, rule=RULE_HASH, frame=FRAME_HASH,
                dense_encoding=DENSE_ENCODING, sparse_encoding=SPARSE_ENCODING,
                L_hex=format(s.L, "x"), microtick_hex=format(s.microtick, "x"),
                origin_code=s.origin_code, charge_frame=s.charge_frame,
                carriers=[[format(c.site, "x"), c.slot, c.direction, c.credit, c.attempted] for c in s.carriers],
                edges=[[format(e.owner, "x"), e.axis, e.q] for e in s.edges])


def checkpoint(state):
    return MAGIC + json.dumps(_payload(_normalized(state)), sort_keys=True, separators=(",", ":")).encode("ascii")


def _unique(pairs):
    result = {}
    for k, v in pairs:
        if k in result:
            raise ValueError("duplicate JSON key")
        result[k] = v
    return result


def _hex(value):
    if (type(value) is not str or not value or (len(value) > 1 and value[0] == "0")
            or any(c not in "0123456789abcdef" for c in value)):
        raise ValueError("canonical lowercase hexadecimal required")
    return int(value, 16)


def restore(data):
    try:
        if type(data) is not bytes or not data.startswith(MAGIC):
            raise ValueError("incompatible sparse checkpoint magic")
        p = json.loads(data[len(MAGIC):], object_pairs_hook=_unique)
        keys = {"schema", "backend", "law", "rule", "frame", "dense_encoding", "sparse_encoding",
                "L_hex", "microtick_hex", "origin_code", "charge_frame", "carriers", "edges"}
        if type(p) is not dict or set(p) != keys:
            raise ValueError("invalid sparse checkpoint fields")
        for k, v in (("schema", SCHEMA), ("backend", BACKEND_ID), ("law", LAW_ID), ("rule", RULE_HASH),
                     ("frame", FRAME_HASH), ("dense_encoding", DENSE_ENCODING), ("sparse_encoding", SPARSE_ENCODING)):
            if p[k] != v:
                raise ValueError("foreign sparse checkpoint identity")
        if type(p["carriers"]) is not list or len(p["carriers"]) != 4 or type(p["edges"]) is not list or len(p["edges"]) > 4:
            raise ValueError("invalid sparse row collections")
        if any(type(row) is not list or len(row) != 5 for row in p["carriers"]) or any(type(row) is not list or len(row) != 3 for row in p["edges"]):
            raise ValueError("invalid sparse row width")
        s = _normalized(SparseQ4State(_hex(p["L_hex"]), _hex(p["microtick_hex"]),
                         tuple(Carrier(_hex(row[0]), *row[1:]) for row in p["carriers"]),
                         tuple(Edge(_hex(row[0]), *row[1:]) for row in p["edges"]), p["origin_code"], p["charge_frame"]))
        if checkpoint(s) != data:
            raise ValueError("noncanonical checkpoint bytes")
        return s
    except (TypeError, KeyError, UnicodeError, OverflowError, RecursionError) as exc:
        raise ValueError("malformed sparse checkpoint") from exc


def transform(state, signed_permutation, translation=(0, 0, 0), conjugate=False):
    s = _normalized(state)
    try:
        g = tuple(tuple(integer(x, "matrix", -1, 1) for x in row) for row in signed_permutation)
        t = tuple(integer(x, "translation", -abs(int(x))) for x in translation)
    except (TypeError, OverflowError) as exc:
        raise ValueError("invalid spatial action") from exc
    if (len(g) != 3 or any(len(row) != 3 or sum(abs(v) for v in row) != 1 for row in g)
            or any(sum(abs(g[i][j]) for i in range(3)) != 1 for j in range(3)) or len(t) != 3
            or type(conjugate) is not bool):
        raise ValueError("signed cubic permutation/translation required")
    def linear(p):
        return tuple(sum(g[i][j]*p[j] for j in range(3)) for i in range(3))
    def position(site):
        return site_index(s.L, tuple(x+y for x, y in zip(linear(coordinates(s.L, site)), t)))
    def direction(d):
        return V.index(linear(V[d-1]))+1
    cs = tuple(sorted(Carrier(position(c.site), c.slot ^ conjugate, direction(c.direction), c.credit, c.attempted)
                      for c in s.carriers))
    es = []
    for e in s.edges:
        d = direction(2*e.axis+1)-1
        owner = position(e.owner if d % 2 == 0 else shift(s.L, e.owner, e.axis, 1))
        es.append(Edge(owner, d//2, e.q*(-1 if d % 2 else 1)*(-1 if conjugate else 1)))
    preimage = tuple(-sum(g[j][i]*t[j] for j in range(3)) for i in range(3))
    old = background_code(s, site_index(s.L, preimage))
    axes, bits = [], 0
    for a, oldaxis in enumerate(AXES[old//8]):
        d = direction(2*oldaxis+1+((old >> a) & 1))-1
        axes.append(d//2)
        bits |= (d % 2) << a
    result = SparseQ4State(s.L, s.microtick, cs, tuple(sorted(es)), 8*AXES.index(tuple(axes))+bits,
                           s.charge_frame ^ conjugate)
    validate(result)
    return result


@dataclass(frozen=True)
class Component:
    sites: tuple
    carriers: tuple
    edges: tuple
    populations: tuple
    F: int
    K: int
    Q: int


@dataclass(frozen=True)
class Observation:
    components: tuple
    degrees: tuple
    populations: tuple
    F: int
    K: int
    Q: int
    gauss_residual: tuple
    carriers: tuple


def observe(state):
    s = _normalized(state)
    adjacency = {c.site: set() for c in s.carriers}
    degrees, residual = {}, {}
    for c in s.carriers:
        residual[c.site] = residual.get(c.site, 0) - (1 if c.slot == 0 else -1)
    for e in s.edges:
        u, v = e.owner, shift(s.L, e.owner, e.axis, 1)
        adjacency.setdefault(u, set()).add(v)
        adjacency.setdefault(v, set()).add(u)
        source, target = (u, v) if e.q == 1 else (v, u)
        degrees.setdefault(source, [0, 0])[1] += 1
        degrees.setdefault(target, [0, 0])[0] += 1
        residual[u] = residual.get(u, 0)+e.q
        residual[v] = residual.get(v, 0)-e.q
    pending, components = set(adjacency), []
    while pending:
        stack, vertices = [min(pending)], set()
        while stack:
            x = stack.pop()
            if x not in vertices:
                vertices.add(x)
                stack.extend(adjacency[x]-vertices)
        pending -= vertices
        cs, es = tuple(c for c in s.carriers if c.site in vertices), tuple(e for e in s.edges if e.owner in vertices)
        K = sum(c.credit for c in cs)
        components.append(Component(tuple(sorted(vertices)), cs, es,
                                    tuple(sum(c.slot == p for c in cs) for p in range(2)), len(es), K, len(es)+K))
    return Observation(tuple(components), tuple((x, *degrees.get(x, [0, 0])) for x in sorted(adjacency)),
                       (2, 2), len(s.edges), sum(c.credit for c in s.carriers), 4,
                       tuple(sorted(residual.items())), s.carriers)
