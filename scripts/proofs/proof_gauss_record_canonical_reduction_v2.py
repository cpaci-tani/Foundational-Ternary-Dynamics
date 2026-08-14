#!/usr/bin/env python3
"""FTD-0878 verifier-only repair of FTD-0877 C48/C66 counts."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_SCRIPT = ROOT / "scripts/proofs/proof_gauss_record_canonical_reduction.py"
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_GAUSS_RECORD_CANONICAL_REDUCTION_v1.md"
)

EXPECTED_SCRIPT = "AC787BADE1050341B47AC5B96C525EB7F871082AFD5DA85BB7361A8CF634D0BF"
EXPECTED_PROTOCOL = "4F24779197A2DE93ABB10DCFC0F84D23EB528A80E96CC3D4F1A548A429F27F4A"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


if digest(PARENT_SCRIPT) != EXPECTED_SCRIPT:
    raise SystemExit("FTD-0878 fail closed: invalid parent script hash")
if digest(PARENT_PROTOCOL) != EXPECTED_PROTOCOL:
    raise SystemExit("FTD-0878 fail closed: invalid parent protocol hash")

source = PARENT_SCRIPT.read_text(encoding="utf-8")

old_c48 = (
    'check("production SOR declares an 18-point Laplacian", '
    '"18-point isotropic Laplacian" in poisson_cpp and '
    '"INV3 * face_sum + INV6 * edge_sum - source[idx]" in poisson_cpp)'
)
new_c48 = (
    'check("production SOR declares an 18-point Laplacian", '
    '"18-point isotropic Laplacian" in poisson_cpp and '
    '"INV3 * face_sum + INV6 * edge_sum - source[idx]" '
    'in " ".join(poisson_cpp.split()))'
)

old_terminal = 'check("terminal gate reached with C68 passing", failures == 0 and checks == 68)'
new_terminal = 'check("terminal gate reached with C65 passing", failures == 0 and checks == 65)'

old_success = "if failures == 0 and checks == 69:"
new_success = "if failures == 0 and checks == 66:"

anchors = ((old_c48, new_c48), (old_terminal, new_terminal), (old_success, new_success))
if any(source.count(old) != 1 for old, _ in anchors):
    raise SystemExit("FTD-0878 fail closed: repair anchors are not unique")

for old, new in anchors:
    source = source.replace(old, new)

namespace = {"__name__": "__main__", "__file__": str(PARENT_SCRIPT)}
exec(compile(source, str(PARENT_SCRIPT), "exec"), namespace)

print("FTD-0878 CERTIFICATE_REPAIR_ONLY_C48_WHITESPACE_AND_COUNTS")
