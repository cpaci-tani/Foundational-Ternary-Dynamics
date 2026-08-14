#!/usr/bin/env python3
"""FTD-0865 verifier-only repair for the FTD-0864 exact certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT = ROOT / "scripts/proofs/proof_clock_gated_hamiltonian_exchange.py"
PARENT_SHA256 = "8B6192E2EBA9D9C2F9B121BA5E34C8141D2456C723781DCD7D515DB5BD740374"

OLD = "reversed_hamiltonian == hamiltonian"
NEW = "sp.simplify(reversed_hamiltonian - hamiltonian) == 0"


def main() -> int:
    source_bytes = PARENT.read_bytes()
    digest = hashlib.sha256(source_bytes).hexdigest().upper()
    if digest != PARENT_SHA256:
        print("FAIL  FTD-0864 parent script hash mismatch")
        return 1

    source = source_bytes.decode("utf-8")
    if source.count(OLD) != 1 or NEW in source:
        print("FAIL  permitted replacement count is not exactly one")
        return 1

    repaired = source.replace(OLD, NEW, 1)
    namespace = {
        "__name__": "ftd0864_repaired_in_memory",
        "__file__": str(PARENT),
    }
    exec(compile(repaired, str(PARENT), "exec"), namespace)
    result = int(namespace["main"]())
    if result != 0:
        print("FTD-0865 certificate repair: inherited certificate failed")
        return result

    print("FTD-0865 certificate repair: PASS")
    print("REPAIR_SCOPE=C34_EXACT_SIMPLIFIED_DIFFERENCE_ONLY")
    print("PARENT_FTD0864_PRESERVED_INVALID")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
