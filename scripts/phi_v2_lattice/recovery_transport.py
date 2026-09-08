"""Exact prepared-singleton transport in the existing four-stage finite law.

This sector has homogeneous, doubly occupied relations and exactly one global
field token. It is collision-free transport, not a Maxwell/matter certificate.
Fractions and continuous charts here are external mathematical observations.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction
from math import lcm
from numbers import Integral

import numpy as np

from . import channels as C, geometry as G, staged as P, state as S
from ._proofs import encode, rotate


def _integer(value, name, minimum=0, maximum=None):
    if (isinstance(value, bool) or not isinstance(value, Integral)
            or value < minimum or (maximum is not None and value > maximum)):
        raise ValueError(f"invalid finite {name}")
    return int(value)


def _fraction(value, name, positive=False):
    if isinstance(value, bool) or not isinstance(value, (Fraction, Integral)):
        raise ValueError(f"{name} must be an exact integer or Fraction")
    value = Fraction(value)
    if value < 0 or (positive and value == 0):
        raise ValueError(f"invalid {name}")
    return value


def _orbit(start, successor, ceiling):
    result, seen = [], set()
    item = start
    while item not in seen:
        if len(result) >= ceiling:
            raise ValueError("finite map did not close within its alphabet")
        result.append(item)
        seen.add(item)
        item = successor(item)
    if item != start:
        raise ValueError("finite map has a transient rather than a prepared orbit")
    return tuple(result)


def channel_orbits() -> tuple[tuple[int, ...], ...]:
    """Exhaust the actual 384-channel permutation without fitting any target."""
    seen, orbits = set(), []
    for channel in range(C.N_CHANNELS):
        if channel not in seen:
            orbit = _orbit(channel, C.U, C.N_CHANNELS)
            if seen.intersection(orbit):
                raise ValueError("channel map is not a disjoint permutation")
            seen.update(orbit)
            orbits.append(orbit)
    return tuple(orbits)


def lifted_displacement(channel: int, microticks: int) -> tuple[int, int, int]:
    """Finite lifted path at an integer elapsed tick, starting in phase zero.

    Hops finish at elapsed ticks 3,7,11,...; no transaction is skipped in the
    actual runtime. This closed form is an observer's exact sector solution.
    """
    channel = _integer(channel, "channel", maximum=C.N_CHANNELS-1)
    microticks = _integer(microticks, "microticks")
    orbit = _orbit(channel, C.U, C.N_CHANNELS)
    hops = (microticks + 1) // 4
    periods, remainder = divmod(hops, len(orbit))
    directions = tuple(C.tangent(c) for c in orbit)
    return tuple(periods * sum(d[axis] for d in directions)
                 + sum(d[axis] for d in directions[:remainder]) for axis in range(3))


@dataclass(frozen=True)
class OrbitCertificate:
    channel: int
    channel_orbit: tuple[int, ...]
    layer_orbit: tuple[int, ...]
    background_orbit: tuple[int, ...]
    period_microticks: int
    period_displacement: tuple[int, int, int]
    velocity: tuple[Fraction, Fraction, Fraction]
    integer_tick_ripple_linf: Fraction
    held_time_ripple_linf: Fraction
    law_id: str = P.LAW_ID
    collision_hash: str = C.COLLISION_HASH
    status: str = "exact_prepared_singleton_transport"


def orbit_certificate(channel: int, layer: int = 0, background: int | None = None) -> OrbitCertificate:
    channel = _integer(channel, "channel", maximum=C.N_CHANNELS-1)
    layer = _integer(layer, "layer", maximum=2)
    if background is None:
        background = S.idx_of(encode(0, +1))
    background = _integer(background, "background", maximum=len(S.A9_LIST)-1)
    if background == S.BLANK_IDX:
        raise ValueError("singleton sector requires doubly occupied background")
    channels = _orbit(channel, C.U, C.N_CHANNELS)
    layers = _orbit(layer, lambda q: (q-1) % 3, 3)
    backgrounds = _orbit(background, lambda z: S.idx_of(rotate(S.z_of(z))), len(S.A9_LIST))
    period = 4 * lcm(len(channels), len(layers), len(backgrounds))
    displacement = lifted_displacement(channel, period)
    velocity = tuple(Fraction(d, period) for d in displacement)
    integer_error, held_error = Fraction(0), Fraction(0)
    for tick in range(period):
        actual = lifted_displacement(channel, tick)
        for axis in range(3):
            left = abs(actual[axis] - tick * velocity[axis])
            right = abs(actual[axis] - (tick+1) * velocity[axis])
            integer_error = max(integer_error, left)
            # On each held interval the absolute affine error is maximized at
            # an endpoint (right endpoint understood as a one-sided limit).
            held_error = max(held_error, left, right)
    return OrbitCertificate(channel, channels, layers, backgrounds, period,
                            displacement, velocity, integer_error, held_error)


def prepare_single_token(L: int, site: int = 0, channel: int = 0,
                         layer: int = 0, background: int | None = None) -> P.StagedState:
    L = _integer(L, "lattice size", minimum=3)
    site = _integer(site, "site", maximum=L**3-1)
    certificate = orbit_certificate(channel, layer, background)
    lattice = S.blank(L)
    lattice.ell[:] = layer
    lattice.sc[:] = certificate.background_orbit[0]
    lattice.fcc[:] = certificate.background_orbit[0]
    lattice.bank[site, channel] = True
    return P.initialize(lattice)


def _initial(state):
    P.validate(state)
    if state.phase != 0 or np.any(state.lattice.s):
        raise ValueError("initial singleton sector requires phase zero and s=0")
    st = state.lattice
    occupied = np.argwhere(st.bank)
    if len(occupied) != 1:
        raise ValueError("exactly one global field token is required")
    layer, background = int(st.ell[0]), int(st.sc.flat[0])
    if (np.any(st.ell != layer) or background == S.BLANK_IDX
            or np.any(st.sc != background) or np.any(st.fcc != background)):
        raise ValueError("uniform layer and homogeneous doubly occupied background required")
    site, channel = map(int, occupied[0])
    return site, orbit_certificate(channel, layer, background)


def _check_snapshot(state, origin, certificate, elapsed):
    """Inspect all live/pending fields, not merely the manifestation quotient."""
    P.validate(state)
    st = state.lattice
    hops, rotations = (elapsed+1)//4, (elapsed+2)//4
    displacement = lifted_displacement(certificate.channel, elapsed)
    xyz = G.coords(st.L, origin)
    expected_site = G.site_index(st.L, *(xyz[a]+displacement[a] for a in range(3)))
    expected_channel = certificate.channel_orbit[hops % len(certificate.channel_orbit)]
    actual = np.argwhere(st.bank)
    if len(actual) != 1 or tuple(map(int, actual[0])) != (expected_site, expected_channel):
        raise ValueError("actual singleton channel/position differs from finite orbit")
    layer = certificate.layer_orbit[rotations % len(certificate.layer_orbit)]
    background = certificate.background_orbit[rotations % len(certificate.background_orbit)]
    if (np.any(st.s) or np.any(st.ell != layer) or np.any(st.sc != background)
            or np.any(st.fcc != background) or np.any(state.admitted_sc)):
        raise ValueError("actual state left homogeneous prepared sector")
    old_displacement = lifted_displacement(certificate.channel, elapsed - elapsed % 4)
    old_site = G.site_index(st.L, *(xyz[a]+old_displacement[a] for a in range(3)))
    for owner in range(st.L**3):
        for axis in range(3):
            expected = bool(elapsed % 4) and old_site not in G.sc_endpoints(st.L, owner, axis)
            if bool(state.gate_sc[owner, axis]) != expected:
                raise ValueError("saved SC gate does not match the complete sector state")
        for plane in range(3):
            for diagonal in range(2):
                expected = bool(elapsed % 4) and old_site not in G.fcc_endpoints(st.L, owner, plane, diagonal)
                if bool(state.gate_fcc[owner, plane, diagonal]) != expected:
                    raise ValueError("saved FCC gate does not match the complete sector state")


@dataclass(frozen=True)
class TrajectoryAudit:
    certificate: OrbitCertificate
    lattice_size: int
    tick_start: int
    tick_end: int
    initial_position: tuple[int, int, int]
    lifted_positions: tuple[tuple[int, int, int], ...]
    observed_max_tick_error_linf: Fraction
    token_count: int


def audit_singleton_trajectory(initial: P.StagedState, microticks: int, tables=None) -> TrajectoryAudit:
    """Run the ACTUAL staged law and compare its finite lifted history.

    The unwrapped position is reconstructed from observed neighboring site
    indices at every tick. All intermediate pending records are checked.
    """
    microticks = _integer(microticks, "microticks")
    origin, certificate = _initial(initial)
    state = P.restore(P.checkpoint(initial))
    _check_snapshot(state, origin, certificate, 0)
    first = tuple(G.coords(state.lattice.L, origin))
    positions = [first]
    previous = first
    error = Fraction(0)
    total = P.work_units(state)
    for elapsed in range(1, microticks+1):
        state, events = P.step(state, tables)
        if any(getattr(events, name) for name in vars(events)):
            raise ValueError("prepared singleton unexpectedly triggered an interaction event")
        _check_snapshot(state, origin, certificate, elapsed)
        if state.microtick != initial.microtick + elapsed or P.work_units(state) != total:
            raise ValueError("actual physical clock or token accounting changed")
        site = int(np.argwhere(state.lattice.bank)[0,0])
        current = tuple(G.coords(state.lattice.L, site))
        L = state.lattice.L
        delta = tuple((current[a]-previous[a]+L//2) % L-L//2 for a in range(3))
        if sum(abs(d) for d in delta) != (1 if elapsed % 4 == 3 else 0):
            raise ValueError("actual trajectory violated its charged one-hop schedule")
        lifted = tuple(positions[-1][a]+delta[a] for a in range(3))
        positions.append(lifted)
        previous = current
        measured = max(abs(lifted[a]-first[a]-elapsed*certificate.velocity[a]) for a in range(3))
        if measured > certificate.integer_tick_ripple_linf:
            raise ValueError("actual trajectory exceeds exact uniform transport bound")
        error = max(error, measured)
    return TrajectoryAudit(certificate, state.lattice.L, initial.microtick, state.microtick,
                           first, tuple(positions), error, total)


@dataclass(frozen=True)
class EmpiricalTransportBound:
    total_multiplicity: int
    horizon_microticks: int
    spacing: Fraction
    tick_duration: Fraction
    measured_grid_time_pairing_bound: Fraction
    uniform_held_time_pairing_bound: Fraction
    physical_velocities: tuple[tuple[Fraction, Fraction, Fraction], ...]
    norm: str = "W1 with L-infinity transport cost; also unit-Lipschitz test observables"
    preparation_kind: str = "external finite counting of separate singleton trajectories"
    interpretation: str = "collision-free characteristic advection; no physical probability identification"


def _validate_audit_report(audit):
    """Check an external report's mathematics; this is not run authentication."""
    certificate = audit.certificate
    if not isinstance(certificate, OrbitCertificate):
        raise ValueError("invalid trajectory certificate")
    if not certificate.layer_orbit or not certificate.background_orbit:
        raise ValueError("incomplete trajectory certificate")
    expected = orbit_certificate(certificate.channel, certificate.layer_orbit[0],
                                 certificate.background_orbit[0])
    if certificate != expected:
        raise ValueError("trajectory certificate differs from the finite law")
    L = _integer(audit.lattice_size, "lattice size", minimum=3)
    start = _integer(audit.tick_start, "initial tick")
    end = _integer(audit.tick_end, "final tick", minimum=start)
    if start % 4 or len(audit.lifted_positions) != end-start+1:
        raise ValueError("trajectory report has inconsistent physical clock/history")
    if len(audit.initial_position) != 3:
        raise ValueError("invalid initial position")
    initial = tuple(_integer(v, "initial coordinate", maximum=L-1) for v in audit.initial_position)
    measured = Fraction(0)
    for tick, position in enumerate(audit.lifted_positions):
        if len(position) != 3 or any(isinstance(v, bool) or not isinstance(v, Integral) for v in position):
            raise ValueError("invalid lifted position")
        displacement = lifted_displacement(certificate.channel, tick)
        if tuple(position) != tuple(initial[a]+displacement[a] for a in range(3)):
            raise ValueError("trajectory report does not follow its exact finite orbit")
        measured = max(measured, *(abs(displacement[a]-tick*certificate.velocity[a]) for a in range(3)))
    recorded = _fraction(audit.observed_max_tick_error_linf, "reported error")
    if measured != recorded or _integer(audit.token_count, "token count") != 18*L**3+1:
        raise ValueError("trajectory report has inconsistent error or accounting")


def empirical_transport_bound(audits, multiplicities, *, spacing=Fraction(1),
                              tick_duration=Fraction(1)) -> EmpiricalTransportBound:
    """Deterministic label pairing supplies an upper bound on transport distance.

    The continuum reference starts at the same scaled initial positions and
    follows each certified velocity. Any additional initial-measure approximation
    error must be added explicitly; it is not supplied by these finite records.
    """
    audits, weights = tuple(audits), tuple(multiplicities)
    if not audits or len(audits) != len(weights) or not all(isinstance(a, TrajectoryAudit) for a in audits):
        raise ValueError("matching nonempty audited trajectories and multiplicities required")
    for audit in audits:
        _validate_audit_report(audit)
    weights = tuple(_integer(w, "multiplicity", minimum=1) for w in weights)
    horizons = {a.tick_end-a.tick_start for a in audits}
    if len(horizons) != 1:
        raise ValueError("finite counting comparison requires one elapsed horizon")
    spacing = _fraction(spacing, "spacing", positive=True)
    duration = _fraction(tick_duration, "tick duration", positive=True)
    total = sum(weights)
    measured = spacing * sum(w*a.observed_max_tick_error_linf for w,a in zip(weights,audits)) / total
    uniform = spacing * sum(w*a.certificate.held_time_ripple_linf for w,a in zip(weights,audits)) / total
    velocities = tuple(tuple(spacing/duration*v for v in a.certificate.velocity) for a in audits)
    return EmpiricalTransportBound(total, next(iter(horizons)), spacing, duration, measured, uniform, velocities)
