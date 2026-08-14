#!/usr/bin/env python3
"""FTD-0871 verifier-only repair for the FTD-0870 exact certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT = ROOT / "scripts/proofs/proof_reversible_ternary_signal_uncomputation.py"
PARENT_SHA256 = "EF83868D6120D97E0F99C1D7B049CD4A22AB481FF1FD402FE6C777045FE2ECCD"

OLD_C35 = (
    '"does not derive a zero-work physical trajectory" in protocol_text,'
)
NEW_C35 = (
    '"does not derive a zero-work physical trajectory" '
    'in " ".join(protocol_text.split()),'
)


def main() -> int:
    source_bytes = PARENT.read_bytes()
    digest = hashlib.sha256(source_bytes).hexdigest().upper()
    if digest != PARENT_SHA256:
        print("FAIL  FTD-0870 parent script hash mismatch")
        return 1

    source = source_bytes.decode("utf-8")
    if source.count(OLD_C35) != 1 or NEW_C35 in source:
        print("FAIL  C35 permitted replacement count is not exactly one")
        return 1

    repaired = source.replace(OLD_C35, NEW_C35, 1)
    namespace = {
        "__name__": "ftd0870_repaired_in_memory",
        "__file__": str(PARENT),
    }
    try:
        exec(compile(repaired, str(PARENT), "exec"), namespace)
    except SystemExit as exc:
        result = int(exc.code or 0)
    else:
        result = 0

    if result != 0:
        print("FTD-0871 certificate repair: inherited certificate failed")
        return result

    print("FTD-0871 certificate repair: PASS")
    print("REPAIR_SCOPE=C35_PROTOCOL_WHITESPACE_NORMALIZATION_ONLY")
    print("PARENT_FTD0870_PRESERVED_INVALID")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
