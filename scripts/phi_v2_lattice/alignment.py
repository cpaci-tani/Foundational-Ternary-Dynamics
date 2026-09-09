"""Selected finite alignment collision, a separately identified research law.

Implements SPEC_PHI_ALIGNMENT_SUCCESSOR_PROPOSAL_V1, without changing the
original table. Formation is designed into this rule; binding is not inferred.
"""
from __future__ import annotations

from collections import Counter
from functools import lru_cache
from itertools import combinations
from numbers import Integral
from types import MappingProxyType

import numpy as np

from . import channels as C, tick as T

LAW_ID = "phi-alignment-staged-research-1"
TABLE_ID = "phi-phase-directed-alignment-table-1"
BASE_COLLISION_HASH = C.COLLISION_HASH
# Hash uses channels._hash_tables' documented sorted UTF-8 row encoding.
COLLISION_HASH = "DE2664FCEA03F5460D9FCA17D57156C887BB857AEA6F674D032B4F13203C3FD9"


def _pair(pair):
    try:
        pair = tuple(pair)
    except TypeError as exc:
        raise ValueError("expected two distinct internal channels") from exc
    if len(pair) != 2 or any(isinstance(c, bool) or not isinstance(c, Integral)
                             or not 0 <= c < C.N_STATES for c in pair) or pair[0] == pair[1]:
        raise ValueError("expected two distinct internal channels")
    return tuple(sorted(map(int, pair)))


def alignment_pair(pair):
    """A_0: copy the unique adjacent-phase predecessor's handedness."""
    a, b = _pair(pair)
    (da, na, ha), ka = C.STATES[a]
    (db, nb, hb), kb = C.STATES[b]
    if da != db or na != nb or ha == hb or (kb - ka) % 4 not in (1, 3):
        return a, b
    if (kb - ka) % 4 == 1:
        b = C.STATE_INDEX[((db, nb, ha), kb)]
    else:
        a = C.STATE_INDEX[((da, na, hb), ka)]
    return tuple(sorted((a, b)))


def _map_pair(pair, permutation):
    return tuple(sorted(permutation[c] for c in pair))


def _generate_tables():
    baseline = C.load_collision_tables()
    pairs = tuple(combinations(range(C.N_STATES), 2))
    base = {pair: baseline[0][alignment_pair(pair)] for pair in pairs}
    tables = []
    for q in range(3):
        r = (-q) % 3
        forward = tuple(range(C.N_STATES))
        for _ in range(r):
            forward = tuple(C.U(c) for c in forward)
        inverse = tuple(int(c) for c in np.argsort(forward))
        tables.append(MappingProxyType({pair: _map_pair(base[_map_pair(pair, inverse)], forward)
                                        for pair in pairs}))
    return tuple(tables)


@lru_cache(maxsize=1)
def load_collision_tables():
    """Immutable successor tables; verify the fixed identity on first load."""
    tables = _generate_tables()
    digest = C._hash_tables(tables)
    if digest != COLLISION_HASH:
        raise RuntimeError(f"alignment collision hash {digest} != frozen {COLLISION_HASH}")
    return tables


def collide_row(row, layer):
    """Pure complete Boolean bank map, independent for the two polarities."""
    if (not isinstance(row, np.ndarray) or row.dtype != np.dtype(bool)
            or row.shape != (C.N_CHANNELS,) or np.any(row.view(np.uint8) > 1)):
        raise ValueError("expected canonical Boolean bank of shape (384,)")
    if isinstance(layer, bool) or not isinstance(layer, Integral) or not 0 <= layer < 3:
        raise ValueError("collision input layer must be integer 0, 1 or 2")
    return T.collide_row(row, layer, load_collision_tables())


def finite_certificate():
    """Exhaustive local-table checks; no full-runtime symmetry assertion.

    Includes every row, both polarity copies, all 48 cubic transformations,
    all four uniform phase shifts, and the three-layer U covariance.
    Raises ValueError on any failed gate (also under Python -O).
    """
    from proof_moore_bond_capacity_type_census import signed_permutation_matrices
    from proof_shared_edge_hodge_flag_bcc_propagation import transform_flag

    def require(condition, message):
        if not condition:
            raise ValueError(message)

    tables = load_collision_tables()
    baseline = C.load_collision_tables()
    pairs = tuple(combinations(range(C.N_STATES), 2))
    pair_set = set(pairs)
    domain = {p for p in pairs if alignment_pair(p) != p}
    aligned = {alignment_pair(p) for p in domain}
    require(len(domain) == len(aligned) == 192, "incorrect base expiry domains")
    source = np.asarray(pairs, dtype=np.int64)
    outputs = [np.asarray([table[p] for p in pairs], dtype=np.int64) for table in tables]
    lookup = np.full((C.N_STATES, C.N_STATES), -1, dtype=np.int64)
    lookup[source[:, 0], source[:, 1]] = np.arange(len(pairs))
    permutations = [tuple(C.STATE_INDEX[(transform_flag(g, flag), k)] for flag, k in C.STATES)
                    for g in signed_permutation_matrices()]
    phase_permutations = [tuple(C.STATE_INDEX[(flag, (k + shift) % 4)] for flag, k in C.STATES)
                          for shift in range(4)]
    changed = []
    fiber_counts = []
    for q, table in enumerate(tables):
        require(set(table) == pair_set and all(a in pair_set for a in table.values()),
                "table is not complete on distinct unordered pairs")
        forward = tuple(range(C.N_STATES))
        for _ in range((-q) % 3):
            forward = tuple(C.U(c) for c in forward)
        dq = {_map_pair(p, forward) for p in domain}
        pq = {_map_pair(p, forward) for p in aligned}
        fibers = Counter(table.values())
        require(all(fibers[p] == (0 if p in dq else 2 if p in pq else 1) for p in pairs),
                "incorrect collision preimage fibers")
        delta = {p for p in pairs if table[p] != baseline[q][p]}
        require(delta == dq, "changed rows differ from registered orbit")
        changed.append(len(delta))
        fiber_counts.append(dict(sorted(Counter(fibers[p] for p in pairs).items())))
        moments = np.asarray([C.layer_value_of(c, q) for c in range(C.N_STATES)], dtype=np.int64)
        require(np.array_equal(moments[source].sum(axis=1), moments[outputs[q]].sum(axis=1)),
                "input-layer moment mismatch")
        for perm in permutations + phase_permutations:
            perm = np.asarray(perm, dtype=np.int64)
            before = np.sort(perm[source], axis=1)
            after = np.sort(perm[outputs[q]], axis=1)
            require(np.array_equal(outputs[q][lookup[before[:, 0], before[:, 1]]], after),
                    "cubic or C4 collision covariance failure")
        perm = np.asarray([C.U(c) for c in range(C.N_STATES)])
        before = np.sort(perm[source], axis=1)
        after = np.sort(perm[outputs[q]], axis=1)
        require(np.array_equal(outputs[(q - 1) % 3][lookup[before[:, 0], before[:, 1]]], after),
                "three-layer U covariance failure")
        for pair in pairs:
            for offset, eps in ((0, 1), (192, -1)):
                row = np.zeros(C.N_CHANNELS, dtype=bool)
                row[np.asarray(pair) + offset] = True
                out, events = collide_row(row, q)
                require(tuple(np.flatnonzero(out)) == tuple(c + offset for c in table[pair])
                        and events == [(eps, pair, table[pair])] and int(row.sum()) == 2,
                        "polarity-copy or accounting mismatch")
    reversal = tuple(C.STATE_INDEX[(flag, (-k) % 4)] for flag, k in C.STATES)
    reversed_output = _map_pair(tables[0][(0, 5)], reversal)
    output_reversed_input = tables[0][_map_pair((0, 5), reversal)]
    require(reversed_output == (4, 7) and output_reversed_input == (0, 3),
            "registered phase-reversal counterexample changed")
    require(changed == [192, 192, 192], "incorrect changed-row total")
    return dict(law=LAW_ID, table=TABLE_ID, collision_hash=COLLISION_HASH,
                original_collision_hash=BASE_COLLISION_HASH, rows=3 * len(pairs),
                changed_rows=sum(changed), changed_rows_per_layer=changed,
                base_domain_size=len(domain), base_aligned_size=len(aligned),
                preimage_counts_per_layer=fiber_counts, polarity_row_checks=6 * len(pairs),
                cubic_transformations=48, phase_shifts=4, input_layer_moments=True,
                layer_covariance=True, phase_reversal_covariant=False,
                full_runtime_symmetry_certified=False, physical_binding_identified=False)
