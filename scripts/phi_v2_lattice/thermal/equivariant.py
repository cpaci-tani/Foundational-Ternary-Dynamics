"""Canonical O_h-equivariant pair involution, independently audited construction.

Matches whole unused group orbits only when actual stabilizers and P/E2 agree.
No random ordering or fitted coefficients. The full selected invariant census is
checked separately; it does not follow merely from having symmetric velocities.
"""
from __future__ import annotations

from collections import defaultdict
from functools import lru_cache
from hashlib import sha256
from itertools import combinations, permutations, product
from pathlib import Path
import struct

import numpy as np

from .runtime import VELOCITIES, CHANNELS

LAW_ID = "phi-thermal-equivariant-candidate-2"
PAIR_INDEX_HASH = "43aa85387a040e5a64dd8b8a62b33f39c5b08aec573ec0b91981d9e2aff05e36"
OUTPUT_PAIR_HASH = "2d3b92167b5c2ac4ce64802d82e37e4ff5a5b0a6ba4c01156176add3d9697bfc"


@lru_cache(maxsize=1)
def _construction_bytes():
    velocity = np.array(VELOCITIES, dtype=np.int64)
    pairs = np.array(list(combinations(range(CHANNELS), 2)), dtype=np.int64)
    a, b = pairs[:, 0], pairs[:, 1]
    identity = np.arange(len(pairs), dtype=np.int32)
    momentum = velocity[a]+velocity[b]
    energy = np.sum(velocity[a]**2+velocity[b]**2, axis=1)
    by_code = np.full(CHANNELS*CHANNELS, -1, dtype=np.int32)
    by_code[a*CHANNELS+b] = identity
    images, stabilizers = [], np.zeros(len(pairs), dtype=np.uint64)
    for axes in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            transformed = velocity[:, axes]*np.array(signs)
            channel_map = (transformed[:, 0]+3)*49+(transformed[:, 1]+3)*7+transformed[:, 2]+3
            ga, gb = channel_map[a], channel_map[b]
            image = by_code[np.minimum(ga, gb)*CHANNELS+np.maximum(ga, gb)]
            stabilizers[image == identity] |= np.uint64(1) << np.uint64(len(images))
            images.append(image)
    images = np.array(images, dtype=np.int32)
    representatives = images.min(axis=0)
    keys = [(*map(int, momentum[i]), int(energy[i]), int(stabilizers[i])) for i in range(len(pairs))]
    buckets = defaultdict(list)
    for i, key in enumerate(keys):
        buckets[key].append(i)
    successor, matched = identity.copy(), set()
    for representative in np.unique(representatives):
        representative = int(representative)
        if representative in matched:
            continue
        for partner in buckets[keys[representative]]:
            partner_orbit = int(representatives[partner])
            if partner_orbit == representative or partner_orbit in matched:
                continue
            successor[images[:, representative]] = images[:, partner]
            successor[images[:, partner]] = images[:, representative]
            matched.update((representative, partner_orbit))
            break
    if sha256(successor.astype('<u4').tobytes()).hexdigest() != PAIR_INDEX_HASH:
        raise ValueError("equivariant construction drift")
    if sha256(pairs[successor].astype('<u2').tobytes()).hexdigest() != OUTPUT_PAIR_HASH:
        raise ValueError("equivariant output encoding drift")
    # Cache only immutable bytes and metadata. Merely clearing NumPy's writable
    # flag leaves owned storage mutable through setflags(write=True).
    return tuple((array.dtype.str, array.shape, array.tobytes())
                 for array in (pairs, successor, images, momentum, energy))


def construction():
    # Fresh views prevent a caller's shape/dtype edits from changing cached
    # metadata. Immutable byte owners prevent writable aliases to the law.
    return tuple(np.frombuffer(data, dtype=dtype).reshape(shape)
                 for dtype, shape, data in _construction_bytes())


@lru_cache(maxsize=1)
def collision_map():
    from types import MappingProxyType
    pairs, successor, *_ = construction()
    return MappingProxyType({tuple(map(int, pair)): tuple(map(int, pairs[j])) for pair, j in zip(pairs, successor)})


def collision_identity() -> str:
    digest = sha256()
    for (a, b), (c, d) in collision_map().items():
        digest.update(struct.pack('<4H', a, b, c, d))
    return digest.hexdigest()


def audit() -> dict:
    pairs, successor, images, momentum, energy = construction()
    identity = np.arange(len(pairs))
    assert np.array_equal(successor[successor], identity)
    assert np.array_equal(momentum[successor], momentum)
    assert np.array_equal(energy[successor], energy)
    for image in images:
        assert np.array_equal(successor[image], image[successor])
    return {"law": LAW_ID, "table_hash": collision_identity(), "pairs": len(pairs),
            "moved": int(np.count_nonzero(successor != identity)),
            "fixed": int(np.count_nonzero(successor == identity)),
            "symmetry_comparisons": int(images.size), "involution": True,
            "scope": "exact collision symmetry/accounting; continuum isotropy and relaxation OPEN"}


def write_header(path: Path) -> None:
    pairs, successor, *_ = construction()
    output = pairs[successor]
    lines = ['#pragma once', '// Generated by scripts.phi_v2_lattice.thermal.equivariant; do not edit.',
             '// Frozen phi-thermal-equivariant-candidate-2. No physical adoption.',
             '#include <cstdint>', 'namespace ftd::thermal::generated {',
             f'inline constexpr char TABLE_HASH[] = "{collision_identity()}";',
             'inline constexpr std::uint16_t EQUIVARIANT_SUCCESSORS[58653][2] = {']
    for start in range(0, len(output), 8):
        lines.append('    '+', '.join('{'+str(int(a))+','+str(int(b))+'}' for a, b in output[start:start+8])+',')
    lines += ['};', '}  // namespace ftd::thermal::generated', '']
    path.write_text('\n'.join(lines), encoding='ascii')


if __name__ == '__main__':
    import argparse
    import json
    parser = argparse.ArgumentParser()
    parser.add_argument('--header', type=Path)
    args = parser.parse_args()
    if args.header:
        write_header(args.header)
    print(json.dumps(audit(), indent=2))
