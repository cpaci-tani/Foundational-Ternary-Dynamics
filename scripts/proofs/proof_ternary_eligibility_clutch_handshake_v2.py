#!/usr/bin/env python3
"""FTD-0867 exact Hamiltonian-coordinate repair for FTD-0866."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT = ROOT / "scripts/proofs/proof_ternary_eligibility_clutch_handshake.py"
PARENT_SHA256 = "6EA25BC3071C2F23DC3B0FBFF640AC5F835D58467F31B1827B345A03D10B0677"

OLD = "hamiltonian = omega * I + nu * action + s**2 * chi * gate * relative_action"
NEW = (
    "hamiltonian = omega * I + nu * (action + relative_action) "
    "+ s**2 * chi * gate * relative_action"
)


def main() -> int:
    source_bytes = PARENT.read_bytes()
    digest = hashlib.sha256(source_bytes).hexdigest().upper()
    if digest != PARENT_SHA256:
        print("FAIL  FTD-0866 parent script hash mismatch")
        return 1

    source = source_bytes.decode("utf-8")
    if source.count(OLD) != 1 or NEW in source:
        print("FAIL  permitted Hamiltonian replacement count is not exactly one")
        return 1

    repaired = source.replace(OLD, NEW, 1)
    namespace = {
        "__name__": "ftd0866_repaired_in_memory",
        "__file__": str(PARENT),
    }
    exec(compile(repaired, str(PARENT), "exec"), namespace)
    result = int(namespace["main"]())
    if result != 0:
        print("FTD-0867 Hamiltonian repair: inherited certificate failed")
        return result

    print("FTD-0867 Hamiltonian repair: PASS")
    print("REPAIR_SCOPE=C14_MISSING_NU_RELATIVE_ACTION_TERM_ONLY")
    print("PARENT_FTD0866_PRESERVED_INVALID")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
