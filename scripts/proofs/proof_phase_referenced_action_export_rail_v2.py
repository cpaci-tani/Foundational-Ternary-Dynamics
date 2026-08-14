#!/usr/bin/env python3
"""FTD-0862 verifier-only repair for the invalid FTD-0861 parent."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT = ROOT / "scripts/proofs/proof_phase_referenced_action_export_rail.py"
EXPECTED_PARENT_SHA256 = (
    "098FA1885B72D60DD0B8DAE547CEAD73B96A8977D92EB11DD896EC4311840F09"
)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


if digest(PARENT) != EXPECTED_PARENT_SHA256:
    raise SystemExit("FTD-0862 INVALID: parent verifier hash mismatch")

source = PARENT.read_text(encoding="utf-8")
old = 'and "does not actuate the relative pair" in transducer_boundary'
new = (
    'and "its common-field trigger cannot determine the relative port" '
    'in transducer_boundary'
)
count = source.count(old)
if count != 1:
    raise SystemExit(
        f"FTD-0862 INVALID: expected one repair fragment, found {count}: {old!r}"
    )
source = source.replace(old, new, 1)

namespace = {"__file__": str(PARENT), "__name__": "__main__"}
exec(compile(source, str(PARENT), "exec"), namespace)
