"""Fixed synthetic topology matrix; these are not interaction-tier outcomes."""
import ast
from dataclasses import asdict, replace
import hashlib
from itertools import permutations, product
import json
from pathlib import Path
import time

import numpy as np
import pytest

from phi_v2_lattice import sparse_credit_exchange_q4 as S
from phi_v2_lattice import credit_exchange_binding as D


def topology_fixtures(L=6):
    """Exact fixture records fixed in source before the first matrix execution.

    Paths list physical integer vertices with unit directed positive flow.
    Carrier tuples are (point, slot, credit). Headings/attempts use the stated
    deterministic fixture/record formulas, with no outcome-based tuning.
    """
    o, x, y, xx, xxx, xxxx = (0, 0, 0), (1, 0, 0), (0, 1, 0), (2, 0, 0), (3, 0, 0), (4, 0, 0)
    a, b = (0, 3, 0), (1, 3, 0)
    neutral = lambda p, ka=0, kb=0: ((p, 0, ka), (p, 1, kb))
    loop = (o, x, (1, 1, 0), y, o)
    definitions = (
        ("two_neutral", (), neutral(o, 1, 1)+neutral(a, 1, 1)),
        ("edge_neutral", ((o, x),), ((o, 0, 1), (x, 1, 0))+neutral(a, 1, 1)),
        ("two_edges_equal", ((o, x), (a, b)), ((o, 0, 1), (x, 1, 0), (a, 0, 0), (b, 1, 1))),
        ("two_edges_unequal", ((o, x), (a, b)), ((o, 0, 1), (x, 1, 1), (a, 0, 0), (b, 1, 0))),
        ("path2_neutral", ((o, x, xx),), ((o, 0, 0), (xx, 1, 0))+neutral(a, 1, 1)),
        ("two_paths2", ((o, x, xx), (a, b, (2, 3, 0))), ((o, 0, 0), (xx, 1, 0), (a, 0, 0), ((2, 3, 0), 1, 0))),
        ("path3_neutral", ((o, x, xx, xxx),), ((o, 0, 1), (xxx, 1, 0))+neutral(a)),
        ("degree3", (((-1, 0, 0), o, x), (o, y)), (((-1, 0, 0), 0, 0), (o, 0, 1), (x, 1, 0), (y, 1, 0))),
        ("degree4", (((-1, 0, 0), o, x), ((0, -1, 0), o, y)),
         (((-1, 0, 0), 0, 0), ((0, -1, 0), 0, 0), (x, 1, 0), (y, 1, 0))),
        ("detached_loop", (loop,), neutral((3, 0, 0))+neutral((3, 2, 0))),
        ("occupied_loop", (loop,), neutral(o)+neutral((1, 1, 0))),
        ("path4_internal_neutral", ((o, x, xx, xxx, xxxx),), ((o, 0, 0), (xxxx, 1, 0))+neutral(xx)),
    )
    result = []
    for i, (name, paths, carriers) in enumerate(definitions):
        cs = sorted((S.site_index(L, point), slot, credit) for point, slot, credit in carriers)
        cs = tuple(S.Carrier(site, slot, 1+(i+j)%6, credit, (i+j)%2) for j, (site, slot, credit) in enumerate(cs))
        es = []
        for path in paths:
            for u, v in zip(path, path[1:]):
                delta = tuple(b-a for a, b in zip(u, v))
                axis = next(a for a in range(3) if delta[a])
                assert sum(abs(t) for t in delta) == 1
                es.append(S.Edge(S.site_index(L, u if delta[axis] == 1 else v), axis, delta[axis]))
        result.append((name, S.initialize(L, cs, es, 0, 0)))
    return tuple(result)


def winding_fixture():
    cs = tuple(S.Carrier(S.site_index(4, p), slot, 1+slot, 0, slot) for p in ((0, 2, 0), (2, 2, 0)) for slot in range(2))
    es = tuple(S.Edge(S.site_index(4, (x, 0, 0)), 0, 1) for x in range(4))
    return S.initialize(4, cs, es, 0, 0)


def assert_parity(state):
    frozen = S.checkpoint(state)
    dense = S.to_dense(state)
    expected, ee = D.step(dense)
    actual, ae = S.step(state)
    assert S.checkpoint(state) == frozen
    assert actual == S.from_dense(expected)
    restored = S.to_dense(actual)
    for name in D.NAMES:
        a = getattr(restored, name)
        assert np.array_equal(a, getattr(expected, name)), name
        assert a.flags.owndata and a.flags.c_contiguous
        assert not any(np.shares_memory(a, getattr(dense, n)) for n in D.NAMES)
    assert actual.microtick == expected.microtick
    assert asdict(ae) == asdict(ee)
    return actual, ae


@pytest.mark.parametrize("fixture", range(13))
@pytest.mark.parametrize("phase", range(19))
def test_all_topology_background_phase_parity(fixture, phase):
    base = topology_fixtures()[fixture][1] if fixture < 12 else winding_fixture()
    for origin, eta in product(range(48), range(2)):
        assert_parity(replace(base, microtick=phase, origin_code=origin, charge_frame=eta))


@pytest.mark.parametrize("attempts", tuple(product(range(2), repeat=4)))
@pytest.mark.parametrize("heading", range(1, 7))
def test_all_attempts_headings_local_branches(attempts, heading):
    # Two adjacent neutral sites exercise addressed and unaddressed capacity.
    cs = tuple(S.Carrier(x, slot, heading, 1, attempts[j]) for j, (x, slot) in enumerate(((0, 0), (0, 1), (36, 0), (36, 1))))
    base = S.initialize(6, cs, (), 0, 0)
    for phase, eta in product(range(19), range(2)):
        assert_parity(replace(base, microtick=phase, charge_frame=eta))


def cubic_actions():
    for axes in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            yield tuple(tuple(signs[row] if col == axes[row] else 0 for col in range(3)) for row in range(3))


@pytest.mark.parametrize("g", tuple(cubic_actions()))
def test_all_768_labelled_actions(g):
    for origin, eta in product(range(8), range(2)):
        base = replace(topology_fixtures()[7][1], origin_code=origin, charge_frame=eta)
        for phase in range(19):
            a = replace(base, microtick=phase)
            transformed = S.transform(a, g, (1, -1, 1), conjugate=True)
            expected = S.transform(S.step(a)[0], g, (1, -1, 1), conjugate=True)
            actual, _ = assert_parity(transformed)
            assert actual == expected


@pytest.mark.parametrize("L", (4, 6, 8, 64))
@pytest.mark.parametrize("kind", (int, np.int64, np.uint8, np.uint64))
def test_seams_scalars_and_ordinal(L, kind):
    base = S.transform(winding_fixture() if L == 4 else topology_fixtures(L)[7][1],
                       ((1, 0, 0), (0, 1, 0), (0, 0, 1)), (L-1, L-1, L-1))
    for phase in range(19):
        s = replace(base, L=kind(L), microtick=kind(phase))
        assert S.restore(S.checkpoint(s)) == S._normalized(s)
        assert_parity(s)


@pytest.mark.parametrize("phase", range(19))
def test_complete_replay_all_phases(phase):
    s = replace(topology_fixtures()[11][1], microtick=10**5000+((phase-10**5000)%19))
    a, b = s, S.restore(S.checkpoint(s))
    for _ in range(19):
        a, ea = S.step(a)
        b, eb = S.step(b)
        assert a == b and asdict(ea) == asdict(eb)


def test_huge_L_hex_codec_without_dense_or_global_limit_change():
    import sys
    limit = sys.get_int_max_str_digits()
    L = 2*10**5000
    cs = tuple(S.Carrier(site, slot, 1, 1, 0) for site in (0, L**3-1) for slot in range(2))
    s = S.initialize(L, cs, (), 47, 1)
    s = replace(s, microtick=10**6000)
    assert S.restore(S.checkpoint(s)) == s
    assert S.step(S.restore(S.checkpoint(s)))[0] == S.step(s)[0]
    assert sys.get_int_max_str_digits() == limit


@pytest.mark.parametrize("value", ("", "00", "01", "A", "0x1", "-1", "+1", " 1", "1 ", 1, True, 1.0))
def test_noncanonical_hex_rejected(value):
    s = topology_fixtures()[0][1]
    p = json.loads(S.checkpoint(s)[len(S.MAGIC):])
    for name in ("L_hex", "microtick_hex"):
        q = dict(p, **{name: value})
        with pytest.raises(ValueError):
            S.restore(S.MAGIC+json.dumps(q, sort_keys=True, separators=(",", ":")).encode())


def test_corrupt_codec_and_foreign_law_fail_before_mutation():
    s = topology_fixtures()[0][1]
    raw = S.checkpoint(s)
    p = json.loads(raw[len(S.MAGIC):])
    malformed = (raw+b"\n", raw.replace(b'"backend":', b'"backend":"x","backend":'),
                 D.checkpoint(S.to_dense(s)), S.MAGIC+b"[]")
    for data in malformed:
        with pytest.raises(ValueError):
            S.restore(data)
    for name in ("schema", "backend", "law", "rule", "frame", "dense_encoding", "sparse_encoding"):
        q = dict(p, **{name: "foreign"})
        with pytest.raises(ValueError):
            S.restore(S.MAGIC+json.dumps(q, sort_keys=True, separators=(",", ":")).encode())
    for name, rows in (("carriers", p["carriers"]*1000), ("edges", [["0", 0, 1]]*5)):
        with pytest.raises(ValueError):
            S.restore(S.MAGIC+json.dumps(dict(p, **{name: rows})).encode())
    for name, value in (("carriers", list(s.carriers)), ("edges", []), ("L", True), ("microtick", -1), ("origin_code", 48), ("charge_frame", 2)):
        with pytest.raises(ValueError):
            S.step(replace(s, **{name: value}))
    bad = replace(s)
    object.__setattr__(bad, "law_id", "foreign")
    with pytest.raises(ValueError):
        S.step(bad)
    assert S.checkpoint(s) == raw
    with pytest.raises(ValueError):
        D.restore(raw)


def test_ownership_order_duplicates_and_complete_graph():
    s = topology_fixtures()[8][1]
    graph = S.observe(s)
    assert len(graph.components) == 1 and len(graph.components[0].carriers) == 4
    assert (0, 2, 2) in graph.degrees
    loop = S.observe(topology_fixtures()[9][1])
    assert sorted((c.F, c.K, len(c.carriers)) for c in loop.components) == [(0, 0, 2), (0, 0, 2), (4, 0, 0)]
    cs, es = list(s.carriers), list(s.edges)
    out = S.initialize(s.L, cs, es, 0, 0)
    cs.clear(); es.clear()
    assert out == s
    for invalid in (replace(s, carriers=s.carriers[::-1]), replace(s, edges=s.edges[::-1]),
                    replace(s, carriers=(s.carriers[0],)*4)):
        with pytest.raises(ValueError):
            S.validate(invalid)
    d = S.to_dense(s)
    compressed = S.from_dense(d)
    d.direction.fill(0)
    assert compressed == s


def test_identity_hashes_are_frozen_reference():
    # Accept only the independently reviewed validation-boundary revision.
    # Original source provenance (not alternative accepted live revisions):
    # dense: 9a317076455949c15b743e7a2576e2dcc9a0311c47825160bee188cc39f12489
    # sparse: 417272af0ff258b92f31da26ef51a920a75de5dc3bdb3adf00335354b8d30b42
    # Structural AST digests were derived from those originals. Unlike
    # ast.dump defaults, this encoding is stable across Python 3.10-3.13;
    # only the empty type_params field introduced in 3.12 is omitted.
    # The audit retains the old/new comparison; pytest needs no ignored files.
    def structure(node):
        if isinstance(node, ast.AST):
            return [type(node).__name__, [[name, structure(value)]
                    for name, value in ast.iter_fields(node)
                    if not (name == "type_params" and value == [])]]
        if isinstance(node, list):
            return [structure(value) for value in node]
        return node

    revisions = (
        (D, "2050bb46d506d5caf486275ab42b64145e2bd5739302a87fbe0388dfad0429b8",
         ("_matching_edges", "step"),
         "7f36ef552eba4e6af8e3346a6b9699d019a747b06480e862ec2020d37eec9785"),
        (S, "92504661b92a4f64a6134cbb18da8afaa82ec30e47b9948d059c50da485d3734",
         ("_evolve", "step"),
         "210841b4357d28d05c6885682e3501ef45b25c5674f3b6d596fbc62360de6999"),
    )
    for module, repaired_sha, scientific, ast_sha in revisions:
        repaired = Path(module.__file__).read_bytes()
        assert hashlib.sha256(repaired).hexdigest() == repaired_sha
        nodes = {node.name: structure(node)
                 for node in ast.parse(repaired).body
                 if isinstance(node, (ast.FunctionDef, ast.ClassDef))}
        frozen = json.dumps({name: nodes[name] for name in scientific},
                            sort_keys=True, separators=(",", ":")).encode()
        assert hashlib.sha256(frozen).hexdigest() == ast_sha
        assert module.RULE_HASH == "ff47ad30df96caa97a0b2c9dc7126eb5b13a9cef6aa2a7105b4879d697baae5c"


def test_768_labels_map_to_exactly_96_physical_backgrounds():
    from collections import Counter
    images = Counter()
    base = topology_fixtures()[0][1]
    for g, origin, eta in product(tuple(cubic_actions()), range(8), range(2)):
        s = S.transform(replace(base, origin_code=origin, charge_frame=eta), g, (1, -1, 1), True)
        images[s.origin_code, s.charge_frame] += 1
    assert len(images) == 96 and set(images.values()) == {8}


def test_external_event_lists_are_owned_and_never_state():
    state = topology_fixtures()[0][1]
    a, ea = S.step(state)
    b, eb = S.step(state)
    raw = S.checkpoint(a)
    ea.attempt_expiries.append((999, 999))
    assert (999, 999) not in eb.attempt_expiries
    assert S.checkpoint(a) == raw and a == b
