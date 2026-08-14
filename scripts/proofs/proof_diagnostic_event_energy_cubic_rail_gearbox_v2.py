#!/usr/bin/env python3
"""FTD-0855 verifier-only repair of FTD-0854 C13/C25/C26."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_SCRIPT = (
    ROOT / "scripts/proofs/proof_diagnostic_event_energy_cubic_rail_gearbox.py"
)
PARENT_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md"
)

EXPECTED_SCRIPT = "60A0EE0CA003737ADD2B57FEDC82A6118B7543FABF6EA007A9D01BB7ECD280A8"
EXPECTED_PROTOCOL = "5397975BC0A6FE0312088B2E741D2F972894AE14EB4711E1BF465488541A6F2A"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


if digest(PARENT_SCRIPT) != EXPECTED_SCRIPT:
    raise SystemExit("FTD-0855 fail closed: invalid parent script hash")
if digest(PARENT_PROTOCOL) != EXPECTED_PROTOCOL:
    raise SystemExit("FTD-0855 fail closed: invalid parent protocol hash")

source = PARENT_SCRIPT.read_text(encoding="utf-8")

old_slice = '''evaporation = phase_write.split(
    "// Evaporation (shared single + dual):", maxsplit=1
)[1]
'''
new_slice = '''evaporation = phase_write.split(
    "ftd::atomic_inc(rb.evaporation_events_this_tick_);", maxsplit=1
)[1].split(
    "// FTD-HISTORY-BEGIN: observation-only native event journal.", maxsplit=1
)[0]
'''

old_c25 = '''    sp.simplify(after[0] ** 2 / 2 - Bp) == 0 and after[1:] == before,
'''
new_c25 = '''    sp.simplify((after[0] ** 2 / 2 - Bp).subs(sigma**2, 1)) == 0
    and after[1:] == before,
'''

old_c26 = '''    sp.simplify(after_energy - before_energy - Bp) == 0,
'''
new_c26 = '''    sp.simplify((after_energy - before_energy - Bp).subs(sigma**2, 1)) == 0,
'''

for label, old in (("C13", old_slice), ("C25", old_c25), ("C26", old_c26)):
    if source.count(old) != 1:
        raise SystemExit(f"FTD-0855 fail closed: {label} repair anchor is not unique")

source = source.replace(old_slice, new_slice)
source = source.replace(old_c25, new_c25)
source = source.replace(old_c26, new_c26)

namespace = {"__name__": "__main__", "__file__": str(PARENT_SCRIPT)}
exec(compile(source, str(PARENT_SCRIPT), "exec"), namespace)

print("FTD-0855 CERTIFICATE_REPAIR_ONLY_C13_SLICE_AND_C25_C26_SIGN_DOMAIN")
