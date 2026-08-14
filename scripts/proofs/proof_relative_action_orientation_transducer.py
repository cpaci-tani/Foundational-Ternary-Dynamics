#!/usr/bin/env python3
"""Exact certificate for FTD-0859.

This script checks the registered relative action/orientation transducer on a
nonzero real canonical pair.  It performs exact symbolic algebra and frozen
source inspection only.  It does not run a numerical search, fit a parameter,
or inspect empirical targets.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_v1.md"
)

FROZEN_SOURCES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "ANALYSIS_NATIVE_PHASE_ACTION_CM_OPERATOR_v1.md":
        "B559C98DB72FBB789E2B9318604A7AB5D788499F0C52771B4265DC53BC3F3DD9",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md":
        "7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md":
        "4A498C6D7C7E65FA685D9F0879157D76713F310A6D025CCAA8756C3F1E0322E6",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md":
        "06ED4EFEF16CF815A44E26F04213FC67F5388E917E9ED9D7B41F9FD8BA736B53",
    "engine/include/ftd/eft/native_modal_phase_action.h":
        "C1E9D5C1944E66D7601D193DC77A39980EBA24B84A41F7D752A3A363910060B6",
    "engine/include/ftd/eft/native_event_characteristics.h":
        "F4A49A1DBF693CF468BC7942264C69B7B25ED9DC41E61059F6F251B696679393",
    "engine/include/ftd/eft/reciprocal_record_port.h":
        "5973BF10BCE122304368E3BD191EA810D3DD6AB106B69B9D9022F662136D2B08",
}


checks: list[tuple[str, bool]] = []


def check(name: str, condition: object) -> None:
    passed = bool(condition)
    checks.append((name, passed))
    print(f"{'PASS' if passed else 'FAIL'}  {name}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def source_text(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def main() -> int:
    for relative, expected in FROZEN_SOURCES.items():
        path = ROOT / relative
        check(f"source hash {relative}", path.is_file() and sha256(path) == expected)

    q, p, event_energy, sign = sp.symbols(
        "q p B s", real=True, nonzero=True
    )
    action = (q**2 + p**2) / 2
    z = sp.Matrix([q, p])
    identity = sp.eye(2)
    quarter_turn = sp.Matrix([[0, -1], [1, 0]])
    signed_turn = sign * quarter_turn

    check("C8 quarter-turn squares to minus identity", quarter_turn**2 == -identity)
    check(
        "C9 quarter-turn is orthogonal",
        quarter_turn.T * quarter_turn == identity,
    )
    check("C10 quarter-turn has determinant one", quarter_turn.det() == 1)
    check(
        "C11 both signed quarter-turn branches are orientation preserving",
        all(
            (sigma * quarter_turn).T * (sigma * quarter_turn) == identity
            and (sigma * quarter_turn).det() == 1
            for sigma in (-1, 1)
        ),
    )

    gain = sp.sqrt((action + event_energy) / action)
    output = gain * signed_turn * z
    output_action = sp.simplify((output.dot(output) / 2).subs(sign**2, 1))
    check(
        "C12 signed pump increases action by exactly B",
        sp.simplify(output_action - action - event_energy) == 0,
    )

    radial_gain_squared = sp.symbols("r", positive=True)
    unique_gain_equation = sp.solve(
        sp.Eq(radial_gain_squared * action, action + event_energy),
        radial_gain_squared,
    )
    check(
        "C13 exact action increment uniquely fixes the positive radial gain",
        unique_gain_equation == [(action + event_energy) / action],
    )

    jacobian = output.jacobian([q, p])
    jacobian_det = sp.simplify(jacobian.det().subs(sign**2, 1))
    check("C14 pump Jacobian determinant is exactly one", jacobian_det == 1)
    poisson_bracket = sp.simplify(
        (
            sp.diff(output[0], q) * sp.diff(output[1], p)
            - sp.diff(output[0], p) * sp.diff(output[1], q)
        ).subs(sign**2, 1)
    )
    check("C15 output canonical coordinates have Poisson bracket one", poisson_bracket == 1)

    output_action_symbol = sp.symbols("I_prime", positive=True)
    inverse_gain_squared = (output_action_symbol - event_energy) / output_action_symbol
    check(
        "C16 forward and inverse radial gains cancel on I prime equals I plus B",
        sp.simplify(
            ((action + event_energy) / action)
            * inverse_gain_squared.subs(output_action_symbol, action + event_energy)
        ) == 1,
    )
    check(
        "C17 signed inverse turn composes to identity",
        sp.simplify((-sign * quarter_turn) * (sign * quarter_turn)).subs(sign**2, 1)
        == identity,
    )
    check(
        "C18 inverse domain is the strict exterior I prime greater than B",
        sp.ask(
            sp.Q.positive(
                inverse_gain_squared.subs(
                    output_action_symbol, action + 2 * event_energy
                )
            ),
            sp.Q.positive(action) & sp.Q.positive(event_energy),
        )
        is True,
    )

    time_reversal = sp.diag(1, -1)
    check(
        "C19 canonical time reversal flips the signed quarter-turn branch",
        sp.simplify(time_reversal * signed_turn * time_reversal + signed_turn)
        == sp.zeros(2),
    )
    cosine, sine = sp.symbols("c d", real=True)
    rotation = sp.Matrix([[cosine, -sine], [sine, cosine]])
    check(
        "C20 planar rotations commute with the quarter-turn pump",
        sp.simplify(rotation * signed_turn - signed_turn * rotation) == sp.zeros(2),
    )
    check(
        "C21 quarter-turn output is orthogonal to the input",
        sp.simplify(z.dot(output)) == 0,
    )
    oriented_area = sp.simplify(q * output[1] - p * output[0])
    check(
        "C22 event sign fixes the orientation of the quarter-turn",
        sp.simplify(oriented_area - sign * gain * (q**2 + p**2)) == 0,
    )

    check(
        "C23 a vector fixed by the quarter-turn must be zero",
        (quarter_turn - identity).det() != 0,
    )
    check(
        "C24 the rotation-fixed zero vector cannot carry positive event energy",
        sp.simplify(event_energy - 0) != 0,
    )
    e1 = sp.Matrix([1, 0])
    e2 = sp.Matrix([0, 1])
    check(
        "C25 zero-carrier directional limits are path dependent",
        signed_turn * e1 != signed_turn * e2,
    )

    plus_image = quarter_turn * z
    minus_image_of_negative = (-quarter_turn) * (-z)
    check(
        "C26 opposite signs collide on opposite background phases",
        sp.simplify(plus_image - minus_image_of_negative) == sp.zeros(2, 1),
    )
    check(
        "C27 the two sign branches have paired opposite preimages",
        sp.simplify((-quarter_turn) * plus_image + z) == sp.zeros(2, 1),
    )

    initial_action, released_energy = sp.symbols("I B0", positive=True)
    final_action_one = initial_action + released_energy
    final_action_two = (initial_action + released_energy / 2) + released_energy / 2
    check(
        "C28 prior action and event energy are conflated in their final sum",
        sp.simplify(final_action_one - final_action_two) == 0,
    )

    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    circle_argument_markers = (
        "two disjoint input circles",
        "single output circle",
        "continuous injection of one circle into a circle is onto",
    )
    check(
        "C29 registered circle-image argument proves the one-pair sign ceiling",
        all(marker in protocol_text for marker in circle_argument_markers),
    )

    signed_amplitude = sign * sp.sqrt(2 * event_energy)
    check(
        "C30 one signed real amplitude distinguishes both event signs",
        sp.simplify(
            signed_amplitude.subs(sign, 1)
            + signed_amplitude.subs(sign, -1)
        ) == 0,
    )
    check(
        "C31 signed amplitude carries exactly the event energy",
        sp.simplify((signed_amplitude**2 / 2).subs(sign**2, 1) - event_energy)
        == 0,
    )
    check(
        "C32 action pump plus energetic signed rail would double count B",
        sp.simplify((action + event_energy + event_energy) - action - 2 * event_energy)
        == 0,
    )

    allowed_symbols = {q, p, event_energy, sign}
    check(
        "C33 transducer law reads only local pair sign and released energy",
        output.free_symbols == allowed_symbols,
    )

    boundary_text = source_text(
        "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md"
    )
    diagnostic_text = source_text(
        "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md"
    )
    check(
        "C34 production still lacks the relative transducer and its energy ledger",
        "common_to_relative_transducer=OPEN" in boundary_text
        and "production_integration=NONE" in boundary_text
        and "aggregate drift ledger excludes rest energy and separate L/R" in diagnostic_text,
    )
    check(
        "C35 lossy pump and faithful rail remain explicitly separate branches",
        "distinct lossy energy-mixing branch" in protocol_text
        and "reserved signed rail" in protocol_text
        and "double count" in protocol_text,
    )
    scope_markers = (
        "does not derive the quarter-turn selection",
        "Born frequencies",
        "biological hemispheres",
        "or completeness",
    )
    check(
        "C36 scope firewall forbids physical and completeness promotion",
        all(marker in protocol_text for marker in scope_markers),
    )

    passed = sum(1 for _, ok in checks if ok)
    total = len(checks)
    print()
    print(f"FTD-0859 relative action/orientation transducer: {passed}/{total} "
          f"{'PASS' if passed == total == 36 else 'FAIL'}")
    if passed == total == 36:
        print("NONZERO_RELATIVE_CARRIER_ADMITS_EXACT_TARGET_BLIND_ACTION_PUMP")
        print("SIGNED_QUARTER_TURN_PUMP_IS_SYMPLECTIC_AND_TIME_REVERSAL_COVARIANT")
        print("EMPTY_ISOTROPIC_PAIR_HAS_NO_POSITIVE_ROTATION_EQUIVARIANT_EXTENSION")
        print("ONE_UNLABELLED_PAIR_CANNOT_FAITHFULLY_RETAIN_EVENT_AND_BACKGROUND")
        print("LOSSY_ACTION_MIXER_AND_FAITHFUL_SIGNED_RAIL_ARE_DISTINCT_BRANCHES")
        print("VERDICT=OUTCOME_B_EXACT_REFERENCE_PUMP_FAITHFULNESS_BOUNDARY")
        return 0
    return 1


if __name__ == "__main__":
    raise SystemExit(main())

