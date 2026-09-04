"""Periodic L^3 simple-cubic lattice and the nine relations per site (declared gauge G1)."""
from __future__ import annotations

E = ((1, 0, 0), (0, 1, 0), (0, 0, 1))


def site_index(L: int, x: int, y: int, z: int) -> int:
    return ((x % L) * L + (y % L)) * L + (z % L)


def coords(L: int, i: int):
    return (i // (L * L), (i // L) % L, i % L)


def shift(L: int, i: int, d) -> int:
    x, y, z = coords(L, i)
    return site_index(L, x + d[0], y + d[1], z + d[2])


def plane_axes(p: int) -> tuple[int, int]:
    a, b = [k for k in range(3) if k != p]
    return a, b


def sc_endpoints(L: int, i: int, a: int) -> tuple[int, int]:
    return i, shift(L, i, E[a])                                   # G1


def fcc_endpoints(L: int, i: int, p: int, q: int) -> tuple[int, int]:
    a, b = plane_axes(p)
    if q == 0:                                                     # diag+
        return i, shift(L, shift(L, i, E[a]), E[b])
    return shift(L, i, E[a]), shift(L, i, E[b])                    # diag-


def relations_at(L: int, i: int):
    """All 18 (relation, role) incidences at site i: 9 as tail (+1), 9 as head (-1)."""
    out = []
    for a in range(3):
        out.append(("sc", i, (a,), +1))
        out.append(("sc", shift(L, i, tuple(-v for v in E[a])), (a,), -1))
    for p in range(3):
        a, b = plane_axes(p)
        ma, mb = tuple(-v for v in E[a]), tuple(-v for v in E[b])
        out.append(("fcc", i, (p, 0), +1))                                       # diag+ tail at i
        out.append(("fcc", shift(L, shift(L, i, ma), mb), (p, 0), -1))           # diag+ head at i
        out.append(("fcc", shift(L, i, ma), (p, 1), +1))                         # diag- tail at i (owner i-e_a)
        out.append(("fcc", shift(L, i, mb), (p, 1), -1))                         # diag- head at i (owner i-e_b)
    assert len(out) == 18
    return out


def sc_edge_of(L: int, x: int, d) -> tuple[int, int]:
    a = [k for k in range(3) if d[k] != 0]
    assert len(a) == 1
    a = a[0]
    if d[a] > 0:
        return x, a
    return shift(L, x, d), a
