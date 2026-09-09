"""Exact geometric first-contact support on the homogeneous occupied background.

This observer changes no law. There are two FIELD slots in addition to 18 L^3
occupied relation slots. Free motion describes the actual same-polarity pair
only through first contact; opposite polarities never collide in this sector.
Contact, capture, restoring response and physical binding are different claims.
"""
from __future__ import annotations

from collections import Counter
import hashlib
from math import gcd
from numbers import Integral

from . import channels as C, staged as P, state as S
from ._proofs import relation_tick, rotate

CERTIFICATE_ID = "strict-contact-reachability-1"


def _integer(value, name, minimum=0, maximum=None):
    if (isinstance(value, bool) or not isinstance(value, Integral)
            or value < minimum or (maximum is not None and value > maximum)):
        raise ValueError(f"invalid {name}")
    return int(value)


def _inputs(c1, c2, L):
    return (_integer(c1, "first channel", maximum=C.N_CHANNELS - 1),
            _integer(c2, "second channel", maximum=C.N_CHANNELS - 1),
            _integer(L, "periodic size", minimum=3))


def _prefixes(channel):
    """Integer displacements after zero, one, two and three free hops."""
    result = [(0, 0, 0)]
    for _ in range(3):
        result.append(tuple(x + d for x, d in zip(result[-1], C.tangent(channel))))
        channel = C.U(channel)
    return tuple(result)


def _relative_data(c1, c2):
    first, second = _prefixes(c1), _prefixes(c2)
    return tuple(tuple(b - a for a, b in zip(x, y)) for x, y in zip(first, second))


def contact_offsets(c1, c2, L):
    """Return a frozenset of all initial second-minus-first contact offsets.

    Offsets lie in {0,...,L-1}^3 and include geometric contact at hop zero.
    For c1 == c2, offset zero is a geometric value, not a legal preparation of
    two distinct exclusion slots. Equal channels at different sites are legal.
    Both polarities are accepted; geometry alone does not imply a collision.
    Time is counted in streaming hops, not the four physical microticks/hop.
    No state-space or long-trajectory search is performed; storage is O(L).
    """
    c1, c2, L = _inputs(c1, c2, L)
    prefixes = _relative_data(c1, c2)
    drift = prefixes[3]
    order = L // gcd(L, *drift)  # also gives one for zero relative drift
    return frozenset(tuple((-prefix[a] - q * drift[a]) % L for a in range(3))
                     for prefix in prefixes[:3] for q in range(order))


def free_relative_offset(c1, c2, r0, hops, L):
    """Exact immutable free-motion offset after any nonnegative integer hops.

    r0 may have signed, unwrapped integer coordinates. The result is canonical
    modulo L. This is a free prediction, not continuation past an actual
    same-polarity collision. From a phase-zero boundary, elapsed microticks t
    contain (t+1)//4 hops. The absolute clock and internal channel period are
    not quotiented by this spatial formula.
    """
    c1, c2, L = _inputs(c1, c2, L)
    hops = _integer(hops, "hop count")
    try:
        r0 = tuple(r0)
    except TypeError as exc:
        raise ValueError("offset requires three integer coordinates") from exc
    if len(r0) != 3 or any(isinstance(x, bool) or not isinstance(x, Integral) for x in r0):
        raise ValueError("offset requires three integer coordinates")
    prefixes = _relative_data(c1, c2)
    quotient, remainder = divmod(hops, 3)
    return tuple((int(r0[a]) + quotient * prefixes[3][a] + prefixes[remainder][a]) % L
                 for a in range(3))


def certificate():
    """Check every channel and occupied-background primitive; return JSON data.

    Algebra proves the all-L contact-support bound; this finite check verifies
    its actual channel and background hypotheses. It neither loads collision
    tables nor enumerates global states, channel pairs or evolution histories.
    The report is evidence from these finite checks, not run authentication.
    """
    def require(condition, message):
        if not condition:
            raise ValueError(message)

    require(C.N_CHANNELS == 384 and C.N_STATES == 192, "changed channel alphabet")
    require({C.U(c) for c in range(C.N_CHANNELS)} == set(range(C.N_CHANNELS)),
            "internal channel map is not a permutation")
    flags, drift_counts, rows = set(), Counter(), []
    for channel in range(C.N_CHANNELS):
        current, orbit = channel, []
        flag = C.STATES[channel % C.N_STATES][0]
        flags.add(flag)
        for hop in range(12):
            require(current not in orbit, "internal channel period is shorter than twelve")
            orbit.append(current)
            require(C.polarity(current) == C.polarity(channel), "free update changes polarity")
            require(C.phase(current) == (C.phase(channel) + hop) % 4,
                    "free phase advance is not a uniform quarter turn")
            if hop == 3:
                require(C.STATES[current % C.N_STATES][0] == flag,
                        "flag does not return after three hops")
            current = C.U(current)
        require(current == channel, "internal channel period is not twelve")
        directions = tuple(C.tangent(c) for c in orbit)
        require(all(sum(abs(x) for x in d) == 1 for d in directions),
                "free tangent is not a signed SC unit direction")
        require(all(directions[j] == directions[j % 3] for j in range(12)),
                "tangent period is not three")
        require(len({next(a for a, x in enumerate(d) if x) for d in directions[:3]}) == 3,
                "three free hops do not use three distinct axes")
        drift = tuple(sum(d[a] for d in directions[:3]) for a in range(3))
        require(all(x in (-1, 1) for x in drift), "free drift is not signed BCC")
        drift_counts[drift] += 1
        half = C.half_turn(channel)
        require(C.STATES[half % C.N_STATES][0] == flag
                and C.polarity(half) == C.polarity(channel), "half-turn changes routing flag")
        rows.append(f"{channel}:{','.join(map(str, orbit))}:{directions!r}")
    require(len(flags) == 48 and len(drift_counts) == 8
            and set(drift_counts.values()) == {48}, "changed flag or drift multiplicities")
    background_checks = 0
    for code in range(len(S.A9_LIST)):
        if code == S.BLANK_IDX:
            continue
        z = S.z_of(code)
        for gate in (False, True):
            require(relation_tick(z, z, even_gate=gate) == (rotate(z), rotate(z)),
                    "homogeneous occupied background depends on saved parity gate")
            background_checks += 1
    bounds = []
    for L in (3, 4, 7, 8):
        upper = 3 * L // gcd(L, 2)
        require(upper < L ** 3, "contact bound fails to leave a nonempty complement")
        bounds.append(dict(L=L, possible_initial_offsets=L ** 3,
                           geometric_contact_offsets_upper=upper,
                           never_contact_offsets_lower=L ** 3 - upper))
    return dict(certificate_id=CERTIFICATE_ID, law_id=P.LAW_ID,
                channel_map_sha256=hashlib.sha256("\n".join(rows).encode("utf-8")).hexdigest(),
                channels_checked=C.N_CHANNELS, flags_checked=len(flags),
                internal_period_hops=12, flag_and_tangent_period_hops=3,
                complete_clock_period_claimed=False, microticks_per_streaming_hop=4,
                background_gate_checks=background_checks, occupied_relation_slots="18*L^3",
                field_slots=2, boundary="finite periodic integer L>=3",
                drift_multiplicities=[dict(drift=list(d), channels=n) for d, n in sorted(drift_counts.items())],
                contact_support="union_j=0,1,2 (-delta_a_j - <delta_b>) in (Z/LZ)^3",
                zero_relative_drift_contact_offsets_upper=3,
                nonzero_relative_drift_order="L/gcd(L,2)",
                contact_offsets_upper="3*L/gcd(L,2)",
                never_contact_offsets_lower="L^3-3*L/gcd(L,2)>0",
                same_flag_contact_offsets=[[0, 0, 0]], example_bounds=bounds,
                collision_tables_loaded=False, global_histories_enumerated=False,
                contact_is_capture=False, physical_binding_certified=False,
                continuum_recovery_certified=False,
                scope="actual free evolution through first contact; forever outside contact support; "
                      "opposite polarities remain free even at contact; any contact-only replacement "
                      "requires the same verified background and streaming hypotheses")
