#!/usr/bin/env python3
"""FTD-0880 exact-anchor verifier repair of FTD-0878 C48."""

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
FAILED_WRAPPER = ROOT / "scripts/proofs/proof_gauss_record_canonical_reduction_v3.py"
FAILED_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_GAUSS_RECORD_CANONICAL_REDUCTION_COMMENT_MARKER_REPAIR_v3.md"
)

EXPECTED = {
    PARENT_WRAPPER: "226B5D1B417725FD97F3A29A7EF2A7C60536BBB85A61532AD56FA301137F4B76",
    PARENT_PROTOCOL: "6625F17CEC5FA2EF0BD294990FE949E70129B270D0C49D86528677DF3BFB52C9",
    FAILED_WRAPPER: "4FBF611151D4F7139BCB79C38FE491A01380F3A1EA0C1BC62D67BEDE0E00661A",
    FAILED_PROTOCOL: "A7CE50DAC58D3D45E71CEEC8E3708562CABCAB4636052A39F793C81385C96915",
}


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


for path, expected in EXPECTED.items():
    if digest(path) != expected:
        raise SystemExit(f"FTD-0880 fail closed: invalid hash for {path.name}")

source = PARENT_WRAPPER.read_text(encoding="utf-8")
old_assignment = '''new_c48 = (
    'check("production SOR declares an 18-point Laplacian", '
    '"18-point isotropic Laplacian" in poisson_cpp and '
    '"INV3 * face_sum + INV6 * edge_sum - source[idx]" '
    'in " ".join(poisson_cpp.split()))'
)'''
new_assignment = '''new_c48 = (
    'check("production SOR declares an 18-point Laplacian", '
    '"isotropic 18-point Poisson stencil" in poisson_cpp and '
    '"INV3 * face_sum + INV6 * edge_sum - source[idx]" '
    'in " ".join(poisson_cpp.split()))'
)'''

if source.count(old_assignment) != 1:
    raise SystemExit("FTD-0880 fail closed: exact new_c48 anchor is not unique")
source = source.replace(old_assignment, new_assignment)

namespace = {"__name__": "__main__", "__file__": str(PARENT_WRAPPER)}
exec(compile(source, str(PARENT_WRAPPER), "exec"), namespace)

print("FTD-0880 CERTIFICATE_REPAIR_ONLY_C48_EXACT_COMMENT_ANCHOR")
