"""Exact verifier for the nonlinear delta-IND v2 lock (FTD-0396).

The verifier is deliberately spec-level.  It does not import the engine, its
hardcoded alpha, floating-point constants, CODATA, or any reachability result.
It recomputes two rational nonlinear transition anchors and audits whether the
bounded/unrestricted closure rungs have earned the properness gate required
before the v1 delta-valuation argument may be applied.

Run only after the tag ``preregister-nonlinear-delta-ind-v2`` exists.
"""

from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction as Q
import sys


Vec = tuple[Q, Q, Q]


def vadd(a: Vec, b: Vec) -> Vec:
    return tuple(x + y for x, y in zip(a, b))  # type: ignore[return-value]


def vscale(a: Vec, k: Q) -> Vec:
    return tuple(k * x for x in a)  # type: ignore[return-value]


def mag2(a: Vec) -> Q:
    return sum((x * x for x in a), Q(0))


@dataclass(frozen=True)
class GenesisResult:
    threshold_pass: bool
    accepted: bool
    flux: Vec
    wave_vel: Vec
    state: int


def rational_genesis_transition(
    flux: Vec,
    wave_vel: Vec,
    threshold: Q,
    kinetic_drain: Q,
    polarity_signal: Q,
    accepted_branch: bool,
) -> GenesisResult:
    """Recompute the single-substrate spec transition in exact arithmetic.

    The accepted/rejected branch is part of the finite branch transcript.  No
    exponential probability value is promoted to a closure generator here.
    The axis-aligned anchor makes |J| rational, so the threshold and flux drain
    are exact rather than floating approximations.
    """

    assert flux[1:] == (Q(0), Q(0)) and flux[0] > 0
    density = flux[0]
    threshold_pass = mag2(flux) > threshold * threshold
    if not (threshold_pass and accepted_branch):
        return GenesisResult(threshold_pass, False, flux, wave_vel, 0)

    flux_factor = max(Q(0), Q(1) - threshold / density)
    post_flux = vscale(flux, flux_factor)
    post_wave = vscale(wave_vel, Q(1) - kinetic_drain)
    state = 1 if polarity_signal > 0 else -1
    return GenesisResult(True, True, post_flux, post_wave, state)


@dataclass(frozen=True)
class CollisionResult:
    carried_remainder: Vec
    source_state: int
    target_state: int
    source_remainder: Vec
    target_remainder: Vec
    source_neighbor_burst: Vec
    target_neighbor_burst: Vec


def rational_annihilation_transition(
    source_state: int,
    target_state: int,
    source_remainder: Vec,
    source_velocity: Vec,
    target_remainder: Vec,
    source_flux: Vec,
    target_flux: Vec,
    dt: Q,
) -> CollisionResult:
    """Recompute remainder carry followed by opposite-sign annihilation."""

    assert source_state in (-1, 1) and target_state == -source_state
    drifted = vadd(source_remainder, vscale(source_velocity, dt))
    assert drifted[0] >= 1 and -1 < drifted[1] < 1 and -1 < drifted[2] < 1
    carried = (drifted[0] - 1, drifted[1], drifted[2])

    # The movement rule computes ``carried`` before testing the occupied
    # target.  Opposite signs then reset both records and distribute each
    # pre-collision flux equally over its own six face neighbours.
    return CollisionResult(
        carried_remainder=carried,
        source_state=0,
        target_state=0,
        source_remainder=(Q(0), Q(0), Q(0)),
        target_remainder=(Q(0), Q(0), Q(0)),
        source_neighbor_burst=vscale(source_flux, Q(1, 6)),
        target_neighbor_burst=vscale(target_flux, Q(1, 6)),
    )


def semantic_word_count(alphabet_size: int, budget: int) -> int:
    """Number of event-type words of length at most B."""

    return sum(alphabet_size**n for n in range(budget + 1))


def decorated_one_event_transcripts(horizon: int) -> set[tuple[str, int, int]]:
    """Space-time-decorated words allowed for one origin-site event.

    The triple is (event type, site offset, event tick).  Even after fixing the
    event alphabet, the site offset, and B=1, a polynomial horizon leaves an
    unbounded tick label as L grows.
    """

    return {("genesis", 0, tick) for tick in range(horizon + 1)}


def check_exact_anchors() -> None:
    genesis = rational_genesis_transition(
        flux=(Q(5, 2), Q(0), Q(0)),
        wave_vel=(Q(3, 4), Q(-1, 2), Q(1, 4)),
        threshold=Q(3, 2),
        kinetic_drain=Q(1, 3),
        polarity_signal=Q(-2, 5),
        accepted_branch=True,
    )
    assert genesis == GenesisResult(
        threshold_pass=True,
        accepted=True,
        flux=(Q(1), Q(0), Q(0)),
        wave_vel=(Q(1, 2), Q(-1, 3), Q(1, 6)),
        state=-1,
    )

    rejected = rational_genesis_transition(
        flux=(Q(3, 2), Q(0), Q(0)),
        wave_vel=(Q(1), Q(0), Q(0)),
        threshold=Q(3, 2),
        kinetic_drain=Q(1, 3),
        polarity_signal=Q(1),
        accepted_branch=True,
    )
    assert rejected == GenesisResult(
        threshold_pass=False,
        accepted=False,
        flux=(Q(3, 2), Q(0), Q(0)),
        wave_vel=(Q(1), Q(0), Q(0)),
        state=0,
    )

    branch_rejected = rational_genesis_transition(
        flux=(Q(5, 2), Q(0), Q(0)),
        wave_vel=(Q(3, 4), Q(-1, 2), Q(1, 4)),
        threshold=Q(3, 2),
        kinetic_drain=Q(1, 3),
        polarity_signal=Q(-2, 5),
        accepted_branch=False,
    )
    assert branch_rejected.state == 0
    assert branch_rejected.flux == (Q(5, 2), Q(0), Q(0))
    assert branch_rejected.wave_vel == (Q(3, 4), Q(-1, 2), Q(1, 4))

    collision = rational_annihilation_transition(
        source_state=1,
        target_state=-1,
        source_remainder=(Q(3, 4), Q(0), Q(0)),
        source_velocity=(Q(1, 2), Q(0), Q(0)),
        target_remainder=(Q(-2, 7), Q(1, 9), Q(0)),
        source_flux=(Q(6, 5), Q(-3, 5), Q(0)),
        target_flux=(Q(-3, 10), Q(9, 10), Q(3, 5)),
        dt=Q(1),
    )
    assert collision == CollisionResult(
        carried_remainder=(Q(1, 4), Q(0), Q(0)),
        source_state=0,
        target_state=0,
        source_remainder=(Q(0), Q(0), Q(0)),
        target_remainder=(Q(0), Q(0), Q(0)),
        source_neighbor_burst=(Q(1, 5), Q(-1, 10), Q(0)),
        target_neighbor_burst=(Q(-1, 20), Q(3, 20), Q(1, 10)),
    )

    try:
        rational_annihilation_transition(
            source_state=1,
            target_state=1,
            source_remainder=(Q(3, 4), Q(0), Q(0)),
            source_velocity=(Q(1, 2), Q(0), Q(0)),
            target_remainder=(Q(0), Q(0), Q(0)),
            source_flux=(Q(1), Q(0), Q(0)),
            target_flux=(Q(1), Q(0), Q(0)),
            dt=Q(1),
        )
    except AssertionError:
        pass
    else:
        raise AssertionError("same-sign control incorrectly entered annihilation")


def check_bounded_transcript_gate() -> tuple[bool, str]:
    # The finite event alphabet does give finitely many *undecorated* words.
    assert semantic_word_count(alphabet_size=6, budget=2) == 43

    # It does not give an L-independent finite set of effective branch
    # transcripts.  H(L)=L is polynomial and the exact transcript count L+1
    # is unbounded, already with B=1 and a fixed site offset.
    counts = []
    for lattice_size in (3, 5, 9, 17, 33):
        words = decorated_one_event_transcripts(horizon=lattice_size)
        assert len(words) == lattice_size + 1
        counts.append(len(words))
    assert counts == sorted(counts) and len(set(counts)) == len(counts)

    # This is a failed sufficiency gate, not an IMPROPER witness: it proves
    # that B alone cannot carry the v1 characterization.  It proves neither
    # universality nor non-universality of the actual native transition system.
    return False, (
        "B bounds event-type word length but not the space-time-decorated "
        "transcript class; the v1 period/valuation upper bound does not "
        "follow from the stated bounded-activity hypotheses"
    )


def check_unrestricted_properness_gate() -> tuple[bool, str]:
    # The execution packet contains neither a universal-computation embedding
    # nor a structural non-universality obstruction.  Exact transition anchors
    # establish adequacy only; they cannot decide expressive power.
    return False, (
        "no universal-computation embedding and no structural "
        "non-universality obstruction has been established"
    )


def delta_valuation_after_properness() -> None:
    """Reserved gate: v1 valuation may run only after properness succeeds."""

    # There is intentionally no call on the v2 execution path booked here.
    # If a later, separately locked proof supplies properness, it must import
    # and rerun the v1 recomputing verifier rather than transcribe a verdict.
    raise RuntimeError("delta valuation called before properness")


def main() -> int:
    check_exact_anchors()
    bounded_proper, bounded_reason = check_bounded_transcript_gate()
    unrestricted_proper, unrestricted_reason = check_unrestricted_properness_gate()

    assert not bounded_proper and not unrestricted_proper
    # Correct precedence after validity gates: neither rung has a universality
    # witness (IMPROPER), a proper construction (REFUTED), nor a properness
    # theorem permitting valuation (PROVEN/PROVEN-CONDITIONAL).  Both therefore
    # land in BLOCKED-ESCAPE without evaluating delta.
    bounded_verdict = "BLOCKED-ESCAPE"
    unrestricted_verdict = "BLOCKED-ESCAPE"

    print("FTD-0396 nonlinear delta-IND v2 exact verifier")
    print("ANCHOR genesis: PASS (exact Fraction arithmetic)")
    print("ANCHOR annihilation: PASS (exact Fraction arithmetic)")
    print(f"N_bounded: {bounded_verdict}")
    print(f"  reason: {bounded_reason}")
    print(f"N_unrestricted: {unrestricted_verdict}")
    print(f"  reason: {unrestricted_reason}")
    print("DELTA_VALUATION: NOT RUN (properness gate did not succeed)")
    print("STANDING TAGS: x+=1/alpha [SMC]; MC-T4.3 [FOUNDATIONAL OBSTRUCTION]; FC-W [AXIOM]")
    return 0


if __name__ == "__main__":
    sys.exit(main())
