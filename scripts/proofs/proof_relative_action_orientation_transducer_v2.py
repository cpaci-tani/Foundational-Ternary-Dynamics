#!/usr/bin/env python3
"""FTD-0860 verifier-only repair for the invalid FTD-0859 parent."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT = ROOT / "scripts/proofs/proof_relative_action_orientation_transducer.py"
EXPECTED_PARENT_SHA256 = (
    "E9B2C1D33730DA6A72EE4F1446FCCA7C0467E33D5A5E6456375CA76BDDE825D8"
)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


if digest(PARENT) != EXPECTED_PARENT_SHA256:
    raise SystemExit("FTD-0860 INVALID: parent verifier hash mismatch")

source = PARENT.read_text(encoding="utf-8")
repairs = (
    (
        "unique_gain_equation == [(action + event_energy) / action],",
        "len(unique_gain_equation) == 1\n"
        "        and sp.simplify(\n"
        "            unique_gain_equation[0] - (action + event_energy) / action\n"
        "        ) == 0,",
    ),
    (
        'check("C15 output canonical coordinates have Poisson bracket one", poisson_bracket == 1)',
        'check(\n'
        '        "C15 output canonical coordinates have Poisson bracket one",\n'
        '        all(\n'
        '            sp.simplify(poisson_bracket.subs(sign, sigma) - 1) == 0\n'
        '            for sigma in (-1, 1)\n'
        '        ),\n'
        '    )',
    ),
    (
        "sp.simplify((-quarter_turn) * plus_image + z) == sp.zeros(2, 1),",
        "sp.simplify((-quarter_turn) * plus_image - z) == sp.zeros(2, 1),",
    ),
    (
        '''    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    circle_argument_markers = (
        "two disjoint input circles",
        "single output circle",
        "continuous injection of one circle into a circle is onto",
    )
    check(
        "C29 registered circle-image argument proves the one-pair sign ceiling",
        all(marker in protocol_text for marker in circle_argument_markers),
    )''',
        '''    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    flat_protocol_text = " ".join(protocol_text.split())
    circle_argument_markers = (
        "two disjoint input circles",
        "the single output circle",
        "A continuous injection of one circle into a circle is onto",
    )
    check(
        "C29 registered circle-image argument proves the one-pair sign ceiling",
        all(marker in flat_protocol_text for marker in circle_argument_markers),
    )''',
    ),
    (
        '''    scope_markers = (
        "does not derive the quarter-turn selection",
        "Born frequencies",
        "biological hemispheres",
        "or completeness",
    )
    check(
        "C36 scope firewall forbids physical and completeness promotion",
        all(marker in protocol_text for marker in scope_markers),
    )''',
        '''    scope_markers = (
        "does not derive the quarter-turn selection",
        "Born frequencies",
        "biological hemispheres",
        "or completeness",
    )
    check(
        "C36 scope firewall forbids physical and completeness promotion",
        all(marker in flat_protocol_text for marker in scope_markers),
    )''',
    ),
)

for old, new in repairs:
    count = source.count(old)
    if count != 1:
        raise SystemExit(
            f"FTD-0860 INVALID: expected one repair fragment, found {count}: {old!r}"
        )
    source = source.replace(old, new, 1)

namespace = {"__file__": str(PARENT), "__name__": "__main__"}
exec(compile(source, str(PARENT), "exec"), namespace)

