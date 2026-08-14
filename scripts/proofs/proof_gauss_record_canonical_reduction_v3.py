#!/usr/bin/env python3
"""FTD-0879 verifier-only repair of FTD-0878 C48 comment marker."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_WRAPPER = ROOT / "scripts/proofs/proof_gauss_record_canonical_reduction_v2.py"
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_GAUSS_RECORD_CANONICAL_REDUCTION_CERTIFICATE_REPAIR_v2.md"
)

EXPECTED_WRAPPER = "226B5D1B417725FD97F3A29A7EF2A7C60536BBB85A61532AD56FA301137F4B76"
EXPECTED_PROTOCOL = "6625F17CEC5FA2EF0BD294990FE949E70129B270D0C49D86528677DF3BFB52C9"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


if digest(PARENT_WRAPPER) != EXPECTED_WRAPPER:
    raise SystemExit("FTD-0879 fail closed: invalid FTD-0878 wrapper hash")
if digest(PARENT_PROTOCOL) != EXPECTED_PROTOCOL:
    raise SystemExit("FTD-0879 fail closed: invalid FTD-0878 protocol hash")

source = PARENT_WRAPPER.read_text(encoding="utf-8")
old_marker = "18-point isotropic Laplacian"
new_marker = "isotropic 18-point Poisson stencil"
if source.count(old_marker) != 1:
    raise SystemExit("FTD-0879 fail closed: comment-marker anchor is not unique")
source = source.replace(old_marker, new_marker)

namespace = {"__name__": "__main__", "__file__": str(PARENT_WRAPPER)}
exec(compile(source, str(PARENT_WRAPPER), "exec"), namespace)

print("FTD-0879 CERTIFICATE_REPAIR_ONLY_C48_COMMENT_MARKER")
