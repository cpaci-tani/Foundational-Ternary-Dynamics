#!/usr/bin/env python3
"""FTD-0850 verifier-only repair of FTD-0849 C19."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_SCRIPT = ROOT / "scripts/proofs/proof_production_ternary_latch_equivalence.py"
PARENT_PROTOCOL = (
    ROOT / "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/PREREG_PRODUCTION_TERNARY_LATCH_EQUIVALENCE_v1.md"
)

EXPECTED_SCRIPT = "BABEB15BEB639D947F664D05972D38E9246CAFBDDB5908FD79479D5894A491B9"
EXPECTED_PROTOCOL = "26FBECC8E52DB8D523AB5B6EB889D7F6679AE93541BB9DC209F901E96AB3BD51"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


if digest(PARENT_SCRIPT) != EXPECTED_SCRIPT:
    raise SystemExit("FTD-0850 fail closed: invalid parent script hash")
if digest(PARENT_PROTOCOL) != EXPECTED_PROTOCOL:
    raise SystemExit("FTD-0850 fail closed: invalid parent protocol hash")

source = PARENT_SCRIPT.read_text(encoding="utf-8")

old_c19 = """check("C19 single and dual branches do not share one event-level latch transaction",
      sp.ask(sp.Q.positive(field_withdrawal)) is True
      and zero(sp.Integer(0)))
"""
new_c19 = """check("C19 single and dual branches do not share one event-level latch transaction",
      zero(field_withdrawal - k * (x + k / 2))
      and k.is_positive is True
      and (x + k / 2).is_positive is True
      and zero(sp.Integer(0)))
"""

if source.count(old_c19) != 1:
    raise SystemExit("FTD-0850 fail closed: C19 repair anchor is not unique")

source = source.replace(old_c19, new_c19)

namespace = {
    "__name__": "__main__",
    "__file__": str(PARENT_SCRIPT),
}
exec(compile(source, str(PARENT_SCRIPT), "exec"), namespace)

print("FTD-0850 CERTIFICATE_REPAIR_ONLY_C19_EXACT_POSITIVE_FACTORIZATION")
