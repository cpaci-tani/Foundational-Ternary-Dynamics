"""Exact complete first-return mathematics; no autonomous central execution.

SPEC_STRICT_FULL_MEMORY_EXECUTION_V1.md precedes this implementation.
Central reference evaluation belongs only to an accepted, locked run.
"""
from __future__ import annotations

from copy import deepcopy
from fractions import Fraction as F
from functools import lru_cache
import hashlib
from itertools import product, permutations
import json
from numbers import Integral
from pathlib import Path
import struct

import numpy as np

from . import hydro_parity as H

LAW_ID = "phi-hydro-parity-field-sector-1"
LABELS = tuple(v for v in product((-1, 0, 1), repeat=4) if sum(x*x for x in v) == 2)
VELOCITIES = tuple(v[:3] for v in LABELS)
GROUP_VELOCITIES = tuple(sorted(set(VELOCITIES)))
GROUPS = tuple(tuple(i for i, v in enumerate(VELOCITIES) if v == w) for w in GROUP_VELOCITIES)
FULL = (1 << 24) - 1
DEN = 1 << 24
N_ELIGIBLE = 11740
PAIR_ORBITS = 34462770
DEN_EXP = 432
MAGIC = b"FTDK1CENTRAL1\0\0\0"
JOB_BYTES = 48644
REP_DISPLACEMENTS = ((0, 0, 0), (1, 0, 0), (1, 1, 0), (1, 1, 1), (2, 0, 0))
EXPECTED_ORBITS = (14, 46, 70, 43, 46)
INPUT_PINS = {
    "scripts/phi_v2_lattice/hydro_parity.py": "593bd2eb314bd4f712e4ac125600147518003fecd8ad98ca51cd8893f77a410b",
    "scripts/phi_v2_lattice/recovery_micro_correlation.py": "9645b3872be8f45c9ac63b7d312ac430872cfb6fa20fa88ec988ee4cb6011dee",
    "scripts/phi_v2_lattice/recovery_momentum_memory.py": "0e22e538026855fedd5aee52f3296d3f54c41d011500a2c01194ce8e6ab6198d",
    "engine/docs/PROPOSAL_STRICT_FULL_MEMORY_RETURN_V1.md": "a4c79a90d44f9d03ed4cb3a54e5275de61c6996b491b9bb4e3d1ae7d7dd83217",
}


def _int(value, name, lo=0, hi=None):
    if isinstance(value, bool) or not isinstance(value, Integral) or value < lo or (hi is not None and value > hi):
        raise ValueError(f"invalid {name}")
    return int(value)


def _exact(value):
    if isinstance(value, bool) or not isinstance(value, (Integral, F)):
        raise ValueError("exact integer or Fraction matrix entry required")
    return F(value)


def _d(value):
    if not isinstance(value, (tuple, list)) or len(value) != 3:
        raise ValueError("three signed integer coordinates required")
    return tuple(_int(x, "coordinate", -3, 3) for x in value)


def canonical_bytes(value):
    return (json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False) + "\n").encode("ascii")


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def frozen_inputs():
    root = Path(__file__).resolve().parents[2]
    for name, expected in INPUT_PINS.items():
        if sha256((root / name).read_bytes()) != expected:
            raise ValueError(f"frozen input changed: {name}")
    if H.LAW_ID != LAW_ID or H.LIFTED_VELOCITIES != LABELS:
        raise ValueError("foreign velocity law")
    return dict(INPUT_PINS)


@lru_cache(maxsize=1)
def eligible():
    values = tuple(H.eligible_masks())
    if len(values) != N_ELIGIBLE or tuple(sorted(set(values))) != values:
        raise AssertionError("eligible census identity")
    for i, mask in enumerate(values):
        if mask.bit_count() != 12 or values[-1-i] != FULL ^ mask:
            raise AssertionError("eligible complement predicate")
        if any(sum(v[a] for c, v in enumerate(LABELS) if mask >> c & 1) for a in range(4)):
            raise AssertionError("eligible four-momentum")
    return values


def _mask(channels):
    return sum(1 << c for c in channels)


def _compress(mask, channels):
    return sum(((mask >> c) & 1) << j for j, c in enumerate(channels))


def _word(code, channels):
    return sum(((code >> j) & 1) << c for j, c in enumerate(channels))


def _assignment_count(channels, code, other, value):
    left, right = _word(code, channels), _word(value, other)
    common = _mask(channels) & _mask(other)
    return 0 if (left ^ right) & common else 1 << (24 - len(set(channels) | set(other)))


@lru_cache(maxsize=1)
def _gram():
    masks = np.asarray(eligible(), dtype=np.uint32)
    spins = ((masks[:, None] >> np.arange(24, dtype=np.uint32)) & 1).astype(np.int64)*2-1
    return tuple(tuple(map(int, row)) for row in (spins.T @ spins))


@lru_cache(maxsize=1)
def _jnum():
    q = _gram()
    reflected = tuple(LABELS.index(v[:3] + (-v[3],)) for v in LABELS)
    return tuple(tuple(((int(a == i) + int(reflected[a] == i)) << 23) - 2*q[a][i]
                       for i in range(24)) for a in range(24))


def jacobian():
    return tuple(tuple(F(x, DEN) for x in row) for row in _jnum())


@lru_cache(maxsize=1)
def _local_tables():
    masks = np.asarray(eligible(), dtype=np.uint32)
    compact = []
    for channels in GROUPS:
        codes = np.zeros(len(masks), dtype=np.uint8)
        for j, c in enumerate(channels):
            codes |= (((masks >> c) & 1) << j).astype(np.uint8)
        compact.append(codes)
    reflected = tuple(LABELS.index(v[:3] + (-v[3],)) for v in LABELS)
    tables = []
    for gi, incoming in enumerate(GROUPS):
        m = len(incoming)
        for go, outgoing in enumerate(GROUPS):
            n = len(outgoing)
            values = []
            for y in range(1 << n):
                for x in range(1 << m):
                    raw = _assignment_count(incoming, x, outgoing, y)
                    raw += _assignment_count(incoming, x, tuple(reflected[c] for c in outgoing), y)
                    if raw % 2:
                        raise AssertionError("unbalanced free parity")
                    values.append(raw // 2)
            before = compact[gi].astype(np.int64) | (compact[go].astype(np.int64) << m)
            after = compact[gi].astype(np.int64) | ((compact[go].astype(np.int64) ^ ((1 << n)-1)) << m)
            correction = np.bincount(after, minlength=1 << (m+n)) - np.bincount(before, minlength=1 << (m+n))
            counts = tuple(int(v)+int(c) for v, c in zip(values, correction))
            if sum(counts) != DEN or min(counts) < 0:
                raise AssertionError("joint count probability")
            for x in range(1 << m):
                if sum(counts[x | (y << m)] for y in range(1 << n)) != DEN >> m:
                    raise AssertionError("input marginal")
            for y in range(1 << n):
                if sum(counts[x | (y << m)] for x in range(1 << m)) != DEN >> n:
                    raise AssertionError("output marginal")
            # Separate exact character expansion, including the same SC pair.
            for y, x in product(range(1 << n), range(1 << m)):
                numerator = DEN
                for j, c in enumerate(incoming):
                    for k, b in enumerate(outgoing):
                        numerator += _jnum()[b][c] * (2*((x >> j) & 1)-1) * (2*((y >> k) & 1)-1)
                if gi == go and m == 2:
                    numerator += DEN * (-1 if (x.bit_count()+y.bit_count()) % 2 else 1)
                if numerator % (1 << (m+n)) or numerator >> (m+n) != counts[x | (y << m)]:
                    raise AssertionError("joint character expansion")
            tables.append(counts)
    return tuple(tables)


def analytic_local_data():
    return {
        "schema": "strict-full-memory-local-data-1", "law_id": LAW_ID,
        "labels": [list(v) for v in LABELS],
        "velocities": [list(v) for v in GROUP_VELOCITIES],
        "groups": [list(c) for c in GROUPS],
        "joint_tables": [{"input": list(GROUPS[i]), "output": list(GROUPS[j]), "counts": list(_local_tables()[18*i+j])}
                         for i, j in product(range(18), repeat=2)],
        "eligible_masks": list(eligible()), "eligible_gram": [list(r) for r in _gram()],
        "jacobian_numerator": [list(r) for r in _jnum()], "jacobian_denominator": DEN,
        "frozen_inputs": frozen_inputs(),
    }


def verify_independent_tables(data):
    own = analytic_local_data()
    for key in ("labels", "velocities", "groups", "joint_tables", "eligible_masks", "eligible_gram",
                "jacobian_numerator", "jacobian_denominator"):
        if data.get(key) != own[key]:
            raise ValueError(f"independent local census mismatch: {key}")
    return True


@lru_cache(maxsize=1)
def _actions():
    result = []
    for axes, signs, fourth in product(tuple(permutations(range(3))), tuple(product((-1, 1), repeat=3)), (-1, 1)):
        mapping = tuple(LABELS.index(tuple(signs[j]*v[axes[j]] for j in range(3)) + (fourth*v[3],)) for v in LABELS)
        result.append((axes, signs, mapping))
    return tuple(result)


def _move(d, axes, signs):
    return tuple(signs[j]*d[axes[j]] for j in range(3))


@lru_cache(maxsize=16)
def _pair_orbits(d):
    maps = tuple(m for axes, signs, m in _actions() if _move(d, axes, signs) == d)
    remaining = set(product(range(24), repeat=2))
    rows = []
    while remaining:
        a, i = min(remaining)
        orbit = {(m[a], m[i]) for m in maps} | {(m[i], m[a]) for m in maps}
        if not orbit <= remaining:
            raise AssertionError("channel orbit overlap")
        remaining -= orbit
        rows.append(((a, i), tuple(sorted(orbit))))
    return tuple(rows)


def _separator(d):
    blocks = []
    for gi, v in enumerate(GROUP_VELOCITIES):
        other = tuple(d[j]-v[j] for j in range(3))
        if other in GROUP_VELOCITIES:
            go = GROUP_VELOCITIES.index(other)
            blocks.append((gi, go))
    left = tuple(c for gi, _ in blocks for c in GROUPS[gi])
    right = tuple(c for _, go in blocks for c in GROUPS[go])
    return left, right, tuple(blocks)


def geometry():
    displacements = tuple(sorted({tuple(a[j]+b[j] for j in range(3)) for a in GROUP_VELOCITIES for b in GROUP_VELOCITIES}))
    rows = []
    for d in displacements:
        left, right, blocks = _separator(d)
        typ = tuple(sorted(map(abs, d), reverse=True))
        rows.append({"displacement": list(d), "type": list(typ), "endpoint_bits": [len(left), len(right)],
                     "shared_sites": len(blocks), "structural_zero": typ in ((2, 1, 0), (2, 1, 1), (2, 2, 0))})
    counts = tuple(len(_pair_orbits(d)) for d in REP_DISPLACEMENTS)
    if len(rows) != 93 or counts != EXPECTED_ORBITS:
        raise AssertionError("registered geometry")
    return {"displacements": rows, "representative_pair_orbits": [
        {"displacement": list(d), "orbits": [{"representative": list(rep), "members": [list(x) for x in orbit]}
                                            for rep, orbit in _pair_orbits(d)]} for d in REP_DISPLACEMENTS]}


@lru_cache(maxsize=16)
def _endpoint_numerators(channels):
    s = len(channels)
    if not 1 <= s <= 12 or len(set(channels)) != s:
        raise ValueError("noncentral endpoint size")
    masks = np.asarray(eligible(), dtype=np.uint32)
    codes = np.zeros(len(masks), dtype=np.int64)
    for j, c in enumerate(channels):
        codes |= ((masks >> c) & 1).astype(np.int64) << j
    words = np.arange(1 << s, dtype=np.int64)
    out = np.zeros((24, 1 << s), dtype=np.int64)
    for i, v in enumerate(LABELS):
        ri = LABELS.index(v[:3] + (-v[3],))
        for c in (i, ri):
            if c in channels:
                out[i] += (2*((words >> channels.index(c)) & 1)-1) * (1 << (23-s))
        spin = 2*((masks >> i) & 1).astype(np.int64)-1
        np.add.at(out[i], codes, -2*spin)
    if np.any(np.abs(out) > (1 << (24-s))) or np.any(out.sum(axis=1)):
        raise AssertionError("conditional endpoint probability")
    return tuple(tuple(map(int, row)) for row in out)


def endpoint_counts(displacement):
    d = _d(displacement)
    if d not in REP_DISPLACEMENTS[1:]:
        raise ValueError("registered noncentral representative required")
    left, right, blocks = _separator(d)
    return {"left_channels": list(left), "right_channels": list(right), "blocks": [list(b) for b in blocks],
            "denominator": 1 << (24-len(left)), "left": [list(r) for r in _endpoint_numerators(left)],
            "right": [list(r) for r in _endpoint_numerators(right)]}


def _walsh_matrix(bits):
    return np.asarray([[(-1 if ((m.bit_count()-(x & m).bit_count()) % 2) else 1)
                        for x in range(1 << bits)] for m in range(1 << bits)], dtype=object)


def _walsh_rows(array):
    a = np.asarray(array, dtype=object).T.copy()
    width = 1
    while width < len(a):
        for start in range(0, len(a), 2*width):
            left, right = a[start:start+width].copy(), a[start+width:start+2*width].copy()
            a[start:start+width] = left+right
            a[start+width:start+2*width] = left-right
        width *= 2
    for mask in range(len(a)):
        if mask.bit_count() % 2:
            a[mask] = -a[mask]
    return a


def _tensor_apply(vectors, factors, dims, output_dims):
    out = np.asarray(vectors, dtype=object).copy()
    dims = list(dims)
    for j in sorted(range(len(factors)), key=lambda k: (output_dims[k] > dims[k], output_dims[k] >= dims[k], k)):
        stride = 1
        for size in dims[:j]:
            stride *= size
        higher = len(out) // (dims[j]*stride)
        block = out.reshape(higher, dims[j], stride, out.shape[1])
        changed = np.tensordot(factors[j], block, axes=(1, 1)).transpose(1, 0, 2, 3)
        dims[j] = output_dims[j]
        out = changed.reshape(higher*dims[j]*stride, out.shape[1])
    return out


def _projection(d):
    J = np.asarray(jacobian(), dtype=object)
    middle = np.zeros((24, 24), dtype=object)
    for gi, go in _separator(d)[2]:
        for c, b in product(GROUPS[gi], GROUPS[go]):
            middle[b, c] = J[b, c]
    return J @ middle @ J


def _walsh_contract(left, right, blocks):
    """Sparse character enumeration, independent of configuration tensor passes.

    Each block admits its constant moment, its explicitly labelled one-to-one
    moments, and only for the same SC group the fixed pair character.
    """
    choices = []
    incoming_offset = outgoing_offset = 0
    J = _jnum()
    for gi, go in blocks:
        incoming, outgoing = GROUPS[gi], GROUPS[go]
        block = [(0, 0, DEN)]
        block += [(1 << (incoming_offset+c), 1 << (outgoing_offset+b), int(J[label_b][label_c]))
                  for c, label_c in enumerate(incoming) for b, label_b in enumerate(outgoing)]
        if gi == go and len(incoming) == 2:
            block.append((3 << incoming_offset, 3 << outgoing_offset, DEN))
        choices.append(block)
        incoming_offset += len(incoming)
        outgoing_offset += len(outgoing)
    a, b = _walsh_rows(left), _walsh_rows(right)
    raw = np.zeros((24, 24), dtype=object)
    for terms in product(*choices):
        x = y = 0
        weight = 1
        for imask, omask, moment in terms:
            x |= imask
            y |= omask
            weight *= moment
        raw += np.multiply.outer(b[y], a[x])*weight
    return raw


@lru_cache(maxsize=8)
def _noncentral(d, basis):
    left, right, blocks = _separator(d)
    ln, rn = _endpoint_numerators(left), _endpoint_numerators(right)
    dims = [1 << len(GROUPS[gi]) for gi, _ in blocks]
    outs = [1 << len(GROUPS[go]) for _, go in blocks]
    factors = [np.asarray(_local_tables()[18*gi+go], dtype=object).reshape(no, ni)
               for (gi, go), ni, no in zip(blocks, dims, outs)]
    if basis == "configuration":
        vectors, endpoint = np.asarray(ln, dtype=object).T, np.asarray(rn, dtype=object)
        exponent = 2*(24-len(left))+24*len(blocks)
        raw = endpoint @ _tensor_apply(vectors, factors, dims, outs)
    elif basis == "walsh":
        exponent = 48+24*len(blocks)
        raw = _walsh_contract(ln, rn, blocks)
    else:
        raise ValueError("unknown contraction basis")
    projected = _projection(d)
    result = tuple(tuple(F(int(raw[a, i]), 1 << exponent)-projected[a, i] for i in range(24)) for a in range(24))
    return result


def noncentral_matrices():
    result = {}
    for d in REP_DISPLACEMENTS[1:]:
        first = _noncentral(d, "configuration")
        if first != _noncentral(d, "walsh"):
            raise AssertionError("independent contraction bases disagree")
        _validate_matrix(d, first)
        result[d] = first
    return result


def _validate_matrix(d, matrix):
    if len(matrix) != 24 or any(len(row) != 24 for row in matrix):
        raise ValueError("24 by24 matrix required")
    for row in matrix:
        for value in row:
            _exact(value)
    for _, orbit in _pair_orbits(d):
        values = {matrix[a][i] for a, i in orbit}
        if len(values) != 1:
            raise AssertionError("channel symmetry or reciprocity")
    weights = ((1,)*24,) + tuple(tuple(v[j] for v in VELOCITIES) for j in range(3))
    for w in weights:
        if any(sum(matrix[a][i]*w[i] for i in range(24)) for a in range(24)):
            raise AssertionError("right conserved annihilation")
        if any(sum(w[a]*matrix[a][i] for a in range(24)) for i in range(24)):
            raise AssertionError("left conserved annihilation")


def central_job_bytes():
    chunks = [MAGIC, struct.pack("<5I", 1, N_ELIGIBLE, 18, 14, DEN_EXP),
              struct.pack("<96b", *(x for v in LABELS for x in v)), struct.pack("<11740I", *eligible())]
    for gi, v in enumerate(GROUP_VELOCITIES):
        go = GROUP_VELOCITIES.index(tuple(-x for x in v))
        counts = _local_tables()[18*gi+go]
        chunks.append(struct.pack("<20I", _mask(GROUPS[gi]), _mask(GROUPS[go]), len(GROUPS[gi]),
                                  len(GROUPS[go]), *counts, *([0]*(16-len(counts)))))
    chunks.append(struct.pack("<28I", *(x for rep, _ in _pair_orbits((0, 0, 0)) for x in rep)))
    raw = b"".join(chunks)
    if len(raw) != JOB_BYTES:
        raise AssertionError("central binary layout")
    return raw


def _parse_job(job):
    if type(job) is not bytes or len(job) != JOB_BYTES or job[:16] != MAGIC:
        raise ValueError("foreign central job")
    if struct.unpack_from("<5I", job, 16) != (1, N_ELIGIBLE, 18, 14, DEN_EXP):
        raise ValueError("foreign central dimensions")
    if job != central_job_bytes():
        raise ValueError("central data differs from complete analytic job")
    offset = 36+96
    masks = struct.unpack_from("<11740I", job, offset)
    offset += 4*N_ELIGIBLE
    blocks = []
    for _ in range(18):
        incoming, outgoing, m, n, *counts = struct.unpack_from("<20I", job, offset)
        offset += 80
        blocks.append((tuple(c for c in range(24) if incoming >> c & 1),
                       tuple(c for c in range(24) if outgoing >> c & 1), tuple(counts[:1 << (m+n)])))
    reps = tuple(zip(*[iter(struct.unpack_from("<28I", job, offset))]*2))
    return masks, tuple(blocks), reps


def pair_at(index):
    index = _int(index, "pair orbit", 0, PAIR_ORBITS-1)
    lo, hi = 0, N_ELIGIBLE//2
    while lo+1 < hi:
        mid = (lo+hi)//2
        if mid*(N_ELIGIBLE+1-mid) <= index:
            lo = mid
        else:
            hi = mid
    return lo, lo+index-lo*(N_ELIGIBLE+1-lo)


def pair_members(i, j):
    i, j = _int(i, "eligible index", 0, N_ELIGIBLE-1), _int(j, "eligible index", 0, N_ELIGIBLE-1)
    if i > j or i+j >= N_ELIGIBLE:
        raise ValueError("noncanonical pair")
    return tuple(sorted({(i, j), (j, i), (N_ELIGIBLE-1-i, N_ELIGIBLE-1-j),
                         (N_ELIGIBLE-1-j, N_ELIGIBLE-1-i)}))


def registered_ranges():
    return tuple((j*PAIR_ORBITS//128, (j+1)*PAIR_ORBITS//128) for j in range(128))


def central_reference_range(job, start, stop):
    """Real central control: call only after independent acceptance and lock."""
    start = _int(start, "range start", 0, PAIR_ORBITS)
    stop = _int(stop, "range stop", start, PAIR_ORBITS)
    if stop-start > 32:
        raise ValueError("reference controls limited to32 orbits per call")
    masks, blocks, reps = _parse_job(job)
    totals = [0]*14
    ordered = 0
    for index in range(start, stop):
        i, j = pair_at(index)
        x, y = masks[i], masks[j]
        weight = 1
        for incoming, outgoing, counts in blocks:
            weight *= counts[_compress(x, incoming) | (_compress(y, outgoing) << len(incoming))]
        members = pair_members(i, j)
        ordered += len(members)
        for r, (a, c) in enumerate(reps):
            feature = sum(4*(2*((masks[u] >> c) & 1)-1)*(2*((masks[v] >> a) & 1)-1) for u, v in members)
            totals[r] += weight*feature
    return {"start": start, "stop": stop, "completed_stop": stop, "checked": stop-start,
            "ordered_pairs": ordered, "denominator_exponent": DEN_EXP, "numerators": [str(x) for x in totals]}


def _central_matrix(numerators):
    if not isinstance(numerators, (tuple, list)) or len(numerators) != 14:
        raise ValueError("fourteen central numerators required")
    nums = tuple(_int(n, "central numerator", -(1 << 434), 1 << 434) for n in numerators)
    hard = np.empty((24, 24), dtype=object)
    for value, (_, members) in zip(nums, _pair_orbits((0, 0, 0))):
        for a, i in members:
            hard[a, i] = F(value, 1 << DEN_EXP)
    J = np.asarray(jacobian(), dtype=object)
    G = np.asarray([[F(int(a == i)+int(LABELS[a][:3]+(-LABELS[a][3],) == LABELS[i]), 2)
                     for i in range(24)] for a in range(24)], dtype=object)
    leak = J-G
    middle = np.asarray([[J[b, c] if VELOCITIES[b] == tuple(-x for x in VELOCITIES[c]) else F(0)
                          for c in range(24)] for b in range(24)], dtype=object)
    small = G @ middle @ G + G @ middle @ leak + leak @ middle @ G
    correction = small - _projection((0, 0, 0))
    if np.any(correction != -(leak @ middle @ leak)):
        raise AssertionError("central g/h cancellation")
    result = tuple(tuple(x for x in row) for row in hard+correction)
    _validate_matrix((0, 0, 0), result)
    return result


def assemble_delta(central_numerators):
    representatives = noncentral_matrices()
    representatives[(0, 0, 0)] = _central_matrix(central_numerators)
    zero = tuple((F(0),)*24 for _ in range(24))
    result = {tuple(row["displacement"]): zero for row in geometry()["displacements"]}
    written = set()
    for d, matrix in representatives.items():
        for axes, signs, mapping in _actions():
            target = _move(d, axes, signs)
            transformed = [[F(0)]*24 for _ in range(24)]
            for a, i in product(range(24), repeat=2):
                transformed[mapping[a]][mapping[i]] = matrix[a][i]
            transformed = tuple(tuple(r) for r in transformed)
            if target in written and result[target] != transformed:
                raise AssertionError("inconsistent complete spatial orbit")
            result[target] = transformed
            written.add(target)
    if len(result) != 93 or len(written) != 33:
        raise AssertionError("incomplete displacement coverage")
    return dict(sorted(result.items()))


def labelled_kernel(delta):
    result = {}
    for d, matrix in delta.items():
        for a, v in enumerate(VELOCITIES):
            r = tuple(d[j]+v[j] for j in range(3))
            table = result.setdefault(r, [[F(0)]*24 for _ in range(24)])
            for i in range(24):
                table[a][i] = matrix[a][i]
    return {r: tuple(tuple(row) for row in table) for r, table in sorted(result.items())}


def _fraction_string(x):
    x = _exact(x)
    return f"{x.numerator}/{x.denominator}"


def conserved_response(delta):
    total = np.asarray([[sum((_exact(matrix[a][i]) for matrix in delta.values()), F(0)) for i in range(24)] for a in range(24)], dtype=object)
    weights = np.asarray([(1,)*24]+[tuple(v[j] for v in VELOCITIES) for j in range(3)], dtype=object).T
    norms = (24, 12, 12, 12)
    tensor = {}
    for mu, nu in product(range(3), repeat=2):
        left = weights * np.asarray([v[mu] for v in VELOCITIES], dtype=object)[:, None]
        right = weights * np.asarray([v[nu] for v in VELOCITIES], dtype=object)[:, None]
        # Basis coefficients: output normalized by its Gram norm.
        matrix = -(left.T @ total @ right)
        tensor[f"{mu},{nu}"] = [[_fraction_string(F(matrix[a, b], norms[a])) for b in range(4)] for a in range(4)]
    directions = (
        ("axis", (1, 0, 0), 1, (0, 1, 0), 1),
        ("axis_second", (1, 0, 0), 1, (0, 0, 1), 1),
        ("face_in_plane", (1, 1, 0), 2, (1, -1, 0), 2),
        ("face_normal", (1, 1, 0), 2, (0, 0, 1), 1),
        ("body_first", (1, 1, 1), 3, (1, -1, 0), 2),
        ("body_second", (1, 1, 1), 3, (1, 1, -2), 6),
    )
    shear = {}
    for name, n, n2, t, t2 in directions:
        vector = np.asarray([sum(n[j]*v[j] for j in range(3))*sum(t[j]*v[j] for j in range(3)) for v in VELOCITIES], dtype=object)
        shear[name] = _fraction_string(-sum(vector[a]*total[a, i]*vector[i] for a, i in product(range(24), repeat=2))/F(12*n2*t2))
    return {"basis": "(1,vx,vy,vz), output coefficient divided by Gram norm(24,12,12,12)",
            "quadratic_matrices": tensor, "normalized_shear": shear,
            "single_kernel_shear_isotropic": len(set(shear.values())) == 1,
            "complete_fluid_isotropy_claim": False}


def four_cycle_conserved(delta):
    """All four conserved outputs/inputs, retaining both endpoint streams."""
    weights = tuple((1, *v) for v in VELOCITIES)
    norms = (24, 12, 12, 12)
    def accumulate(items):
        result = {}
        for site, a, i, value in items:
            if not value:
                continue
            matrix = result.setdefault(site, [[F(0)]*4 for _ in range(4)])
            for mu, nu in product(range(4), repeat=2):
                matrix[mu][nu] += F(value*weights[a][mu]*weights[i][nu], norms[mu])
        return {site: tuple(tuple(row) for row in matrix) for site, matrix in sorted(result.items())
                if any(value for row in matrix for value in row)}
    direct = accumulate((tuple(d[j]+VELOCITIES[a][j]+VELOCITIES[i][j] for j in range(3)),
                         a, i, _exact(matrix[a][i]))
                        for d, matrix in delta.items() for a, i in product(range(24), repeat=2))
    via_kernel = accumulate((tuple(r[j]+VELOCITIES[i][j] for j in range(3)), a, i, matrix[a][i])
                            for r, matrix in labelled_kernel(delta).items() for a, i in product(range(24), repeat=2))
    if direct != via_kernel:
        raise AssertionError("two endpoint streams disagree")
    return direct


def _psd_exact(matrix):
    a = [[F(x) for x in row] for row in matrix]
    pivots = []
    for k in range(len(a)):
        if a[k][k] < 0:
            raise AssertionError("negative impulse PSD pivot")
        if a[k][k] == 0:
            if any(a[k][j] for j in range(k+1, len(a))):
                raise AssertionError("nonzero row at zero PSD pivot")
            pivots.append(F(0))
            continue
        pivot = a[k][k]
        pivots.append(pivot)
        for i in range(k+1, len(a)):
            for j in range(i, len(a)):
                a[j][i] = a[i][j] = a[i][j]-a[i][k]*a[k][j]/pivot
    return tuple(pivots)


def certificate(delta):
    expected = {tuple(x["displacement"]) for x in geometry()["displacements"]}
    if set(delta) != expected:
        raise ValueError("full93-site kernel required")
    for d, matrix in delta.items():
        _validate_matrix(d, matrix)
    kernel = labelled_kernel(delta)
    J = np.asarray(jacobian(), dtype=object)
    gram = sum((np.asarray(m, dtype=object).T @ np.asarray(m, dtype=object) for m in delta.values()),
               np.zeros((24, 24), dtype=object))
    pivots = _psd_exact(np.eye(24, dtype=object)-J @ J-gram)
    gamma = max(sum(abs(matrix[a][i]) for matrix in delta.values() for i in range(24)) for a in range(24))
    source, target = LABELS.index((1, 1, 0, 0)), LABELS.index((1, -1, 0, 0))
    known = F(139921773018153334181957509616241, 44601490397061246283071436545296723011960832)
    if delta[(2, 0, 0)][target][source] != known:
        raise AssertionError("frozen frontier response mismatch")
    momentum = F(0)
    for a, i in product(range(24), repeat=2):
        d = tuple((4, 0, 0)[j]-VELOCITIES[a][j]-VELOCITIES[i][j] for j in range(3))
        if d in delta:
            momentum += VELOCITIES[a][1]*VELOCITIES[i][1]*delta[d][a][i]/144
    if momentum/8 != -known/576:
        raise AssertionError("eight-microtick conserved regression")
    lifts = []
    for L, support in ((7, kernel), (8, kernel)):
        for origin in ((0, 0, 0), (L-1, L-1, L-1)):
            keys = [tuple((r[j]+origin[j]) % L for j in range(3)) for r in support]
            if len(set(keys)) != len(keys):
                raise AssertionError("K1 lift alias")
            lifts.append({"L": L, "origin": list(origin), "sites": len(keys)})
    conserved = four_cycle_conserved(delta)
    four_lifts = []
    for L in (9, 10):
        for origin in ((0, 0, 0), (L-1, L-1, L-1)):
            points = [tuple((r[j]+origin[j]) % L for j in range(3)) for r in conserved]
            if len(set(points)) != len(points) or any(max(map(abs, r)) > 4 for r in conserved):
                raise AssertionError("four-cycle conserved lift alias or radius")
            four_lifts.append({"L": L, "origin": list(origin), "microticks": 8, "sites": len(points)})
    serialize = lambda table: [[_fraction_string(x) for x in row] for row in table]
    return {
        "schema": "strict-full-memory-certificate-1", "law_id": LAW_ID, "frozen_inputs": frozen_inputs(),
        "delta": [{"displacement": list(d), "matrix": serialize(m)} for d, m in sorted(delta.items())],
        "labelled_K1": [{"displacement": list(d), "matrix": serialize(m)} for d, m in sorted(kernel.items())],
        "geometry": geometry(), "Gamma": _fraction_string(gamma),
        "impulse_PSD_pivots": [_fraction_string(x) for x in pivots],
        "conserved_response": conserved_response(delta), "periodic_K1_lifts": lifts,
        "four_cycle_conserved_kernel": [{"displacement": list(r), "matrix": serialize(m)}
                                        for r, m in conserved.items()],
        "periodic_four_cycle_lifts": four_lifts,
        "momentum_eight_microtick_signed_error": _fraction_string(momentum/8),
        "polarity": "identical independent24-channel blocks; cross-bank response zero",
        "central_second_complete_method": False, "sampling_error": "none",
        "uniform_memory_tail": False, "continuum_recovered": False, "canonical_adoption": False,
    }
