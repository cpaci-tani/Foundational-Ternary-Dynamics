"""Exact observations and finite weak conservation bounds for the frozen hydro law.

This module never advances a state. A cycle certificate checks conservation and
streaming constraints on three supplied complete checkpoints; it is not a proof
that the supplied collision was actually executed. Constitutive closure is absent.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction
from math import factorial
from collections.abc import Sequence

import numpy as np

from .. import geometry as G
from . import channels as H, codec, staged, state as S, tick

MOMENT_NAMES = ("N", "Px", "Py", "Pz", "Pxx", "Pxy", "Pxz", "Pyy", "Pyz", "Pzz")
_WEIGHTS = tuple((1, x, y, z, x*x, x*y, x*z, y*y, y*z, z*z)
                 for x, y, z in H.VELOCITIES)


def _rational(value, name, *, positive=False, nonnegative=False):
    if type(value) not in (int, Fraction):
        raise ValueError(f"{name} must be an exact int or Fraction, not float/bool")
    result = Fraction(value)
    if positive and result <= 0:
        raise ValueError(f"{name} must be positive")
    if nonnegative and result < 0:
        raise ValueError(f"{name} must be nonnegative")
    return result


def _occupancies(st):
    # Local sums are at most four; admitted exclusion makes them zero or one.
    return st.lattice.bank.reshape(-1, 2, 4, 24).sum(axis=2, dtype=np.int16)


@dataclass(frozen=True)
class MomentSnapshot:
    law: str
    table: str
    encoding: str
    boundary: str
    L: int
    microtick: int
    # moments[polarity][site][component], all owned immutable Python integers.
    moments: tuple
    per_site_relation_tokens: tuple[int, ...]
    field_tokens: tuple[int, int]
    relation_tokens: int
    fully_occupied_relations: bool

    @property
    def total_tokens(self):
        return sum(self.field_tokens) + self.relation_tokens


def _observe(st):
    occ = _occupancies(st)
    # Every local product/sum is bounded by 24, so int16 cannot overflow.
    local = occ @ np.asarray(_WEIGHTS, dtype=np.int16)
    moments = tuple(tuple(tuple(int(v) for v in row) for row in local[:, p])
                    for p in range(2))
    n = st.lattice.L ** 3
    rel = tuple(int(np.count_nonzero(st.lattice.sc[x] != S.BLANK_IDX))
                + int(np.count_nonzero(st.lattice.fcc[x] != S.BLANK_IDX))
                for x in range(n))
    totals = tuple(sum(row[0] for row in bank) for bank in moments)
    return MomentSnapshot(staged.LAW_ID, H.TABLE_HASH, H.ENCODING_HASH,
                          "periodic", st.lattice.L, st.microtick, moments, rel,
                          totals, sum(rel), all(value == 18 for value in rel))


def observe_checkpoint(data: bytes) -> MomentSnapshot:
    """Validate a complete checkpoint and publish immutable exact field moments."""
    return _observe(codec.restore(data))


def admit_conservative_sector(data: bytes) -> MomentSnapshot:
    """Admit the invariant fully occupied relation sector at a cycle boundary.

    Both slots of every relation must be nonblank. Their internal phases can
    rotate; 'frozen' means occupied support, not identical full checkpoints.
    """
    result = observe_checkpoint(data)
    if result.microtick % 4 or not result.fully_occupied_relations:
        raise ValueError("conservative sector requires cycle boundary and all relation slots occupied")
    return result


@dataclass(frozen=True)
class CycleCertificate:
    law: str
    table: str
    encoding: str
    L: int
    initial_microtick: int
    final_microtick: int
    cell_volume: Fraction
    # The exact identity is weak_change == weak_source + weak_stream.
    weak_change: tuple
    weak_source: tuple
    weak_stream: tuple
    # Nonnegative count and signed channel momentum removed at each source site.
    absorbed_moments: tuple
    relation_arrivals: tuple[int, ...]
    tokens: tuple[int, int, int]
    conservative_sector: bool


def certify_cycle(initial: bytes, prestream: bytes, final: bytes,
                  test_values: Sequence, *, cell_volume=Fraction(1)) -> CycleCertificate:
    """Check exact local and weak balances at q,q+2,q+4, where q mod 4 is zero.

    Source proposals are reconstructed from the complete initial phase/relations.
    Phase-resolved streaming is checked directly. Collision-table execution,
    gate evaluation and manifestation validity require a separate replay audit.
    """
    states = tuple(codec.restore(value) for value in (initial, prestream, final))
    a, b, c = states
    if a.microtick % 4 or (b.microtick, c.microtick) != (a.microtick+2, a.microtick+4):
        raise ValueError("cycle requires exact stage ordinals q,q+2,q+4 with q%4=0")
    if len({st.lattice.L for st in states}) != 1:
        raise ValueError("cycle lattice sizes differ")
    n = a.lattice.L ** 3
    if not isinstance(test_values, Sequence) or len(test_values) != n:
        raise ValueError("test_values must contain one exact rational per site")
    psi = tuple(_rational(v, "test value") for v in test_values)
    volume = _rational(cell_volume, "cell_volume", positive=True)
    observations = tuple(_observe(st) for st in states)
    old, before_stream, new = observations
    absorbed = [[[0]*4 for _ in range(n)] for _ in range(2)]
    arrivals = [0]*n
    admitted_sc = np.zeros_like(b.admitted_sc)
    admitted_fcc = np.zeros_like(b.admitted_fcc)
    for (kind, owner, idx), (site, channel) in tick.admitted_absorptions(a.lattice).items():
        polarity, _, velocity = H.unpack(channel)
        for component, weight in enumerate(_WEIGHTS[velocity][:4]):
            absorbed[polarity][site][component] += weight
        arrivals[owner] += 1
        target = admitted_sc if kind == "sc" else admitted_fcc
        target[(owner, *idx)] = True
    if not np.array_equal(b.admitted_sc, admitted_sc) or not np.array_equal(b.admitted_fcc, admitted_fcc):
        raise ValueError("prestream admission records disagree with initial source proposals")
    if not np.array_equal(a.lattice.s, b.lattice.s):
        raise ValueError("manifestation must not change before streaming")
    for p in range(2):
        for x in range(n):
            for k in range(4):
                if before_stream.moments[p][x][k] != old.moments[p][x][k]-absorbed[p][x][k]:
                    raise ValueError("local collision/source number or momentum balance failed")
    if any(before_stream.per_site_relation_tokens[x] != old.per_site_relation_tokens[x]+arrivals[x]
           for x in range(n)):
        raise ValueError("relation token arrival balance failed")
    for name in ("sc", "fcc"):
        if not np.array_equal(getattr(b.lattice, name), getattr(c.lattice, name)):
            raise ValueError("relations changed during streaming/manifestation")

    # Verify the complete phase-resolved permutation, with the declared half-turn.
    expected = np.zeros_like(b.lattice.bank)
    for x, channel in zip(*np.nonzero(b.lattice.bank)):
        x, channel = int(x), int(channel)
        polarity, phase, velocity = H.unpack(channel)
        y = G.shift(a.lattice.L, x, H.VELOCITIES[velocity])
        new_phase = (phase+1+2*int(b.lattice.s[x] != 0)) % 4
        expected[y, H.channel(polarity, new_phase, velocity)] = True
    if not np.array_equal(expected, c.lattice.bank):
        raise ValueError("phase-resolved one-hop streaming balance failed")
    tokens = tuple(obs.total_tokens for obs in observations)
    if len(set(tokens)) != 1:
        raise ValueError("field plus relation token accounting failed")

    occ = _occupancies(b)
    change, source, stream = [], [], []
    for p in range(2):
        cp, sp, fp = [], [], []
        for k in range(4):
            cp.append(volume*sum((new.moments[p][x][k]-old.moments[p][x][k])*psi[x] for x in range(n)))
            sp.append(-volume*sum(absorbed[p][x][k]*psi[x] for x in range(n)))
            value = Fraction(0)
            for x in range(n):
                for v in range(24):
                    if occ[x, p, v]:
                        y = G.shift(a.lattice.L, x, H.VELOCITIES[v])
                        value += _WEIGHTS[v][k]*(psi[y]-psi[x])
            fp.append(volume*value)
            if cp[-1] != sp[-1]+fp[-1]:
                raise ValueError("exact weak identity failed")
        change.append(tuple(cp))
        source.append(tuple(sp))
        stream.append(tuple(fp))
    return CycleCertificate(staged.LAW_ID, H.TABLE_HASH, H.ENCODING_HASH,
                            a.lattice.L, a.microtick, c.microtick, volume,
                            tuple(change), tuple(source), tuple(stream),
                            tuple(tuple(tuple(row) for row in bank) for bank in absorbed),
                            tuple(arrivals), tokens,
                            all(obs.fully_occupied_relations for obs in observations))


@dataclass(frozen=True)
class AcousticTaylorBound:
    microtick: int
    order: int
    h: Fraction
    c_ref: Fraction
    cycle_duration: Fraction
    microtick_duration: Fraction
    cell_volume: Fraction
    derivative_bound: Fraction | tuple
    weak_remainder: tuple
    rate_remainder: tuple


def acoustic_taylor_bound(prestream: bytes, *, h, c_ref, derivative_bound,
                          order=2, cell_volume=None) -> AcousticTaylorBound:
    """Bound the spatial Taylor remainder through degree order-1 (order 2 or 3).

    A scalar derivative_bound bounds every coordinate partial of total order `order`
    on every lifted streaming segment of a test function periodic with period L*h
    in every coordinate. For order=3 a globally Lipschitz
    Hessian with that coordinate bound also suffices. User-supplied derivative
    bounds are assumptions, not verified by checkpoint data. Four microticks
    comprise a cycle of externally selected duration h/c_ref. No temporal
    interpolation, constitutive law, or physical mass identification is made.
    Alternatively, a tuple of three bounds admits a separable test function
    psi(x)=sum_a psi_a(x_a), with no mixed derivatives; the a-th bound controls
    psi_a's order-th derivative (or its order-1 derivative's Lipschitz constant).
    Separability is a declared assumption, never inferred from checkpoint data.
    """
    st = codec.restore(prestream)
    if st.microtick % 4 != 2:
        raise ValueError("Taylor bounds require a prestream checkpoint")
    if type(order) is not int or order not in (2, 3):
        raise ValueError("Taylor order must be 2 or 3")
    h = _rational(h, "h", positive=True)
    speed = _rational(c_ref, "c_ref", positive=True)
    if type(derivative_bound) is tuple and len(derivative_bound) == 3:
        derivative = tuple(_rational(v, "separable derivative bound", nonnegative=True)
                           for v in derivative_bound)
    else:
        derivative = _rational(derivative_bound, "derivative_bound", nonnegative=True)
    volume = h**3 if cell_volume is None else _rational(cell_volume, "cell_volume", positive=True)
    dt = h/speed
    occ = _occupancies(st)
    remainder = []
    for p in range(2):
        component_sums = [Fraction(0)]*4
        for x in range(st.lattice.L**3):
            for v, velocity in enumerate(H.VELOCITIES):
                if occ[x, p, v]:
                    factor = (sum(derivative[a]*abs(velocity[a])**order for a in range(3))
                              if isinstance(derivative, tuple)
                              else derivative*sum(abs(q) for q in velocity)**order)
                    for k in range(4):
                        component_sums[k] += abs(_WEIGHTS[v][k])*factor
        remainder.append(tuple(volume*h**order*value/factorial(order)
                               for value in component_sums))
    bounds = tuple(remainder)
    return AcousticTaylorBound(st.microtick, order, h, speed, dt, dt/4,
                               volume, derivative, bounds,
                               tuple(tuple(value/dt for value in row) for row in bounds))
