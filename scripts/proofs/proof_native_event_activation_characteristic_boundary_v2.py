#!/usr/bin/env python3
"""FTD-0858 verifier-only repair for the execution-invalid FTD-0857 parent."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT = ROOT / "scripts/proofs/proof_native_event_activation_characteristic_boundary.py"
EXPECTED_PARENT_SHA256 = (
    "6D7B2FC2B6BA432976D359A2C104EAB15FAB175BC5F721B7B6B84BA8D13D17A2"
)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


if digest(PARENT) != EXPECTED_PARENT_SHA256:
    raise SystemExit("FTD-0858 INVALID: parent verifier hash mismatch")

source = PARENT.read_text(encoding="utf-8")
repairs = (
    (
        'phase_write.index("// ---- Sequential post-pass", event_start)',
        'phase_write.index("void phase_write_assign_pending_ids", event_start)',
    ),
    (
        'time_reversed == (-outgoing, -incoming)',
        'sp.simplify(time_reversed[0] + outgoing) == 0\n'
        '    and sp.simplify(time_reversed[1] + incoming) == 0',
    ),
    (
        'trace_defect == 4 * s2 * (1 - c2)',
        'sp.simplify(trace_defect - 4 * s2 * (1 - c2)) == 0',
    ),
    (
        '"not thereby\\n+derived physical law"',
        '"not thereby\\nderived physical law"',
    ),
)

for old, new in repairs:
    count = source.count(old)
    if count != 1:
        raise SystemExit(
            f"FTD-0858 INVALID: expected one repair fragment, found {count}: {old!r}"
        )
    source = source.replace(old, new, 1)

namespace = {"__file__": str(PARENT), "__name__": "__main__"}
exec(compile(source, str(PARENT), "exec"), namespace)

