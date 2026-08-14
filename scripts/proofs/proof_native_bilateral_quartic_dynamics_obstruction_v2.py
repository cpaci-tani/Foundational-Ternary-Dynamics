"""FTD-0838 tooling-only repair of the FTD-0837 exact certificate.

The frozen parent is hash-checked.  Exactly two structural SymPy equalities
are replaced by equality after exact simplification; no source, equation,
gate, tolerance, or outcome rule changes.
"""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT = ROOT / "scripts/proofs/proof_native_bilateral_quartic_dynamics_obstruction.py"
PARENT_SHA256 = "4EE5CA8EE94B9B99D14A267D55A431EDAE76A2CA1143C42837123CA5DBDBD768"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


if sha256(PARENT) != PARENT_SHA256:
    raise SystemExit("FTD-0838 parent hash mismatch")

source = PARENT.read_text(encoding="utf-8")
repairs = (
    (
        "sp.diff(E_damped, E) == (1 - g) ** 2",
        "sp.simplify(sp.diff(E_damped, E) - (1 - g) ** 2) == 0",
    ),
    (
        "stability_margin == 4 * eta * (1 - eta)",
        "sp.simplify(stability_margin - 4 * eta * (1 - eta)) == 0",
    ),
)

for old, new in repairs:
    if source.count(old) != 1:
        raise SystemExit(f"FTD-0838 repair target count is not one: {old}")
    source = source.replace(old, new)

source = source.replace(
    "FTD-0837 native bilateral/quartic dynamics",
    "FTD-0838 native bilateral/quartic dynamics repaired certificate",
)

exec(compile(source, str(PARENT), "exec"), {"__name__": "__main__", "__file__": str(PARENT)})
