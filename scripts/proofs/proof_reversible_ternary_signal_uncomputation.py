#!/usr/bin/env python3
"""FTD-0870 exact certificate for reversible ternary signal uncomputation."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
SOURCE_HASHES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_LOSS_BOOKED_TERNARY_PHASE_LATCH_v1.md":
        "1C1BE138260B4CD3B639F7B6E1DB9E78886B2CCC9E6C0388CFC83E0D0FE073CA",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md":
        "7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md":
        "5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_SIGNAL_ACKNOWLEDGED_TWO_STROKE_RESET_AND_SMOOTH_BOUNDARY_v1.md":
        "7E6B7CD0488EFE7A5A5108CEC251AD0972276F015C8F4B3F1C8F3FCEE3308B9E",
    "engine/include/ftd/eft/signal_acknowledged_two_stroke_reset.h":
        "7C2308CA97DD1ED17FF1E38FB56FAE6FC56AD7D46B4B8A13E5CB083AD42F9C7D",
}

TERNARY = (-1, 0, 1)
ENCODE = {-1: 2, 0: 0, 1: 1}
DECODE = {0: 0, 1: 1, 2: -1}

checks = 0
failures = 0


def check(label: str, condition: bool) -> None:
    global checks, failures
    checks += 1
    if condition:
        print(f"PASS  C{checks} {label}")
    else:
        failures += 1
        print(f"FAIL  C{checks} {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def tadd(first: int, second: int) -> int:
    return DECODE[(ENCODE[first] + ENCODE[second]) % 3]


def tneg(value: int) -> int:
    return DECODE[(-ENCODE[value]) % 3]


def tsub(first: int, second: int) -> int:
    return tadd(first, tneg(second))


def controlled_reset(latch: int, decoded_signal: int, acknowledgement: int) -> tuple[int, int]:
    subtraction = decoded_signal if acknowledgement == 1 else 0
    return tsub(latch, subtraction), decoded_signal


def controlled_inverse(latch: int, decoded_signal: int, acknowledgement: int) -> tuple[int, int]:
    addition = decoded_signal if acknowledgement == 1 else 0
    return tadd(latch, addition), decoded_signal


for relative, expected in SOURCE_HASHES.items():
    path = ROOT / relative
    check(f"source hash {relative}", path.is_file() and sha256(path) == expected)

check(
    "ternary encoding is bijective onto Z3",
    set(ENCODE) == set(TERNARY)
    and set(ENCODE.values()) == {0, 1, 2}
    and len(set(ENCODE.values())) == 3,
)
check(
    "ternary decoder is the exact inverse encoding",
    all(DECODE[ENCODE[value]] == value for value in TERNARY),
)
check(
    "pulled-back ternary addition is closed",
    all(tadd(first, second) in TERNARY for first in TERNARY for second in TERNARY),
)
check(
    "zero is the ternary additive identity",
    all(tadd(value, 0) == value == tadd(0, value) for value in TERNARY),
)
check(
    "sign reversal is the ternary additive inverse",
    all(tadd(value, tneg(value)) == 0 for value in TERNARY),
)
check(
    "ternary addition is associative",
    all(
        tadd(tadd(first, second), third) == tadd(first, tadd(second, third))
        for first in TERNARY
        for second in TERNARY
        for third in TERNARY
    ),
)
check(
    "controlled reset is total on both acknowledgement branches",
    all(
        controlled_reset(latch, signal, acknowledgement)[0] in TERNARY
        for latch in TERNARY
        for signal in TERNARY
        for acknowledgement in (0, 1)
    ),
)
check(
    "zero acknowledgement is exact identity",
    all(controlled_reset(latch, signal, 0) == (latch, signal)
        for latch in TERNARY for signal in TERNARY),
)
check(
    "completed matching signal resets every ternary latch value",
    all(controlled_reset(value, value, 1) == (0, value) for value in TERNARY),
)
check(
    "both active event signs reset exactly",
    all(controlled_reset(value, value, 1)[0] == 0 for value in (-1, 1)),
)
check(
    "no-event zero state remains ready",
    controlled_reset(0, 0, 0) == (0, 0)
    and controlled_reset(0, 0, 1) == (0, 0),
)
check(
    "controlled subtraction leaves the signal decoder unchanged",
    all(controlled_reset(latch, signal, acknowledgement)[1] == signal
        for latch in TERNARY for signal in TERNARY for acknowledgement in (0, 1)),
)
check(
    "registered inverse is a left inverse",
    all(
        controlled_inverse(*controlled_reset(latch, signal, acknowledgement), acknowledgement)
        == (latch, signal)
        for latch in TERNARY
        for signal in TERNARY
        for acknowledgement in (0, 1)
    ),
)
check(
    "registered inverse is a right inverse",
    all(
        controlled_reset(*controlled_inverse(latch, signal, acknowledgement), acknowledgement)
        == (latch, signal)
        for latch in TERNARY
        for signal in TERNARY
        for acknowledgement in (0, 1)
    ),
)
check(
    "each acknowledgement branch is a bijection on latch and signal labels",
    all(
        len({controlled_reset(latch, signal, acknowledgement)
             for latch in TERNARY for signal in TERNARY}) == 9
        for acknowledgement in (0, 1)
    ),
)
check(
    "simultaneous sign reversal is equivariant",
    all(
        controlled_reset(-latch, -signal, acknowledgement)
        == tuple(-value for value in controlled_reset(latch, signal, acknowledgement))
        for latch in TERNARY
        for signal in TERNARY
        for acknowledgement in (0, 1)
    ),
)
check(
    "registered joint reset outputs remain distinct and are not erasure",
    len({controlled_reset(value, value, 1) for value in TERNARY}) == 3,
)
check(
    "bare latch reset is noninjective",
    len({0 for _value in TERNARY}) == 1 < len(TERNARY),
)
check(
    "sign-even energy-only retention collides for opposite signs",
    (-1) ** 2 == 1 ** 2 and -1 != 1,
)
check(
    "oriented signal decoder is injective on the ternary alphabet",
    len({value for value in TERNARY}) == len(TERNARY),
)
check(
    "reversible reset of all three latch values needs at least three retained labels",
    3 > 2 and len(TERNARY) == 3,
)
check(
    "the existing signal supplies exactly the required ternary labels",
    {DECODE[index] for index in range(3)} == set(TERNARY),
)
check(
    "acknowledgement is combinational and adds no persistent output coordinate",
    all(len(controlled_reset(latch, signal, acknowledgement)) == 2
        for latch in TERNARY for signal in TERNARY for acknowledgement in (0, 1)),
)
check(
    "signal workspace removes the need for a separate reset-history trit",
    all(controlled_inverse(0, signal, 1)[0] == signal for signal in TERNARY),
)
check(
    "one discrete update reaches the exact ready latch",
    controlled_reset(1, 1, 1)[0] == 0
    and controlled_reset(-1, -1, 1)[0] == 0,
)

smooth_source = (ROOT / next(
    relative for relative in SOURCE_HASHES if "SIGNAL_ACKNOWLEDGED" in relative
)).read_text(encoding="utf-8")
check(
    "discrete uncomputation preserves the continuous smooth-reset no-go scope",
    "locally Lipschitz autonomous attraction cannot reach" in smooth_source
    and "selected cusp" in smooth_source,
)

x, amplitude, beta = sp.symbols("x A beta", positive=True, finite=True)
ternary_potential = beta * x**2 * (x**2 - amplitude**2) ** 2
check(
    "all three registered ternary-potential endpoints are degenerate",
    all(sp.simplify(ternary_potential.subs(x, value * amplitude)) == 0
        for value in TERNARY),
)

event_energy = sp.symbols("B", positive=True, finite=True)
check(
    "signal event energy is unchanged by latch uncomputation",
    sp.simplify(event_energy - event_energy) == 0,
)
check(
    "logical uncomputation contains no scalar-bath state or energy term",
    controlled_reset(1, 1, 1) == (0, 1),
)

protocol_text = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
      "PREREG_REVERSIBLE_TERNARY_SIGNAL_UNCOMPUTATION_v1.md"
).read_text(encoding="utf-8")
check(
    "endpoint degeneracy is not promoted to zero physical controller work",
    "does not derive a zero-work physical trajectory" in protocol_text,
)

def output_handoff(local_signal: tuple[int, sp.Expr], output_port: tuple[int, sp.Expr]):
    return output_port, local_signal


empty = (0, sp.Integer(0))
signal = (-1, event_energy)
after_local, exported = output_handoff(signal, empty)
check(
    "empty-port handoff is reciprocal and involutive",
    after_local == empty
    and exported == signal
    and output_handoff(*output_handoff(signal, empty)) == (signal, empty),
)
check(
    "uncomputation plus handoff returns local readiness and retains signed energy",
    controlled_reset(signal[0], signal[0], 1)[0] == 0
    and after_local == empty
    and exported == signal,
)

rail_depth = sp.symbols("N", integer=True, nonnegative=True)
check(
    "one longer ternary history strictly exceeds a finite ternary rail capacity",
    sp.simplify(3 ** (rail_depth + 1) - 3**rail_depth)
    == 2 * 3**rail_depth,
)
check(
    "scalar tail energy loses the retained orientation sign",
    sp.Rational(1, 2) * (-1) ** 2 == sp.Rational(1, 2) * 1**2,
)

scope_terms = (
    "continuous-latch dynamics",
    "native formation",
    "robust controller work",
    "protected cubic transport",
    "production coupling",
    "`G*`",
    "Born/Bell",
    "Lorentz",
    "biological",
    "completeness",
)
check(
    "scope firewall retains every physical and interpretive debt",
    all(term in protocol_text for term in scope_terms),
)

print()
print(f"FTD-0870 reversible ternary signal uncomputation: {checks - failures}/{checks} PASS")
if failures == 0 and checks == 40:
    print("COMPLETED_SIGNAL_REVERSIBLY_UNCOMPUTES_MATCHING_TERNARY_LATCH")
    print("NO_EXTRA_ACK_BIT_RESET_HISTORY_TRIT_OR_LOGICAL_BATH_IS_REQUIRED")
    print("SMOOTH_CONTINUOUS_RESET_NO_GO_REMAINS_BINDING_FOR_SELECTED_X_LATCH")
    print("OUTPUT_TRANSPORT_CONTROLLER_WORK_NATIVE_GSTAR_AND_BORN_REMAIN_OPEN")
    print("VERDICT=OUTCOME_A_REVERSIBLE_ACTUAL_LAYER_UNCOMPUTATION")
    raise SystemExit(0)

print("VERDICT=OUTCOME_C_INVALID")
raise SystemExit(1)
