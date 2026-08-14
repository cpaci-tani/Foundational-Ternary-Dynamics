"""Replay-only scope repair v2 for the unchanged FTD-0832 verifier.

The new v5 module-scope codec block references two helpers that the verifier
declares only inside ``main``.  Inject their unchanged semantics into the
runpy globals without changing the producer, artifacts, replay gates, or the
hash-checked verifier source.
"""

from __future__ import annotations

import math
import os
import runpy
from pathlib import Path


def scalar_close(lhs: float, rhs: float, tolerance: float = 2e-12) -> bool:
    """Match the unchanged verifier's locally declared scalar comparison."""
    return abs(lhs - rhs) <= tolerance * max(1.0, abs(rhs))


def required(row: dict[str, str], field: str) -> float:
    """Match the successful-value semantics of the verifier's local helper."""
    payload = row[field].strip()
    if payload == "":
        raise RuntimeError(f"missing required replay field {field}")
    value = float(payload)
    if not math.isfinite(value):
        raise RuntimeError(f"nonfinite required replay field {field}")
    return value


os.environ["FTD_0832_NONSINGULAR_PRODUCT_CHART"] = "1"
runpy.run_path(
    str(Path(__file__).with_name("proof_l17_complete_tangent_candidate.py")),
    init_globals={"scalar_close": scalar_close, "required": required},
    run_name="__main__",
)
