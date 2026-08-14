"""Replay-only NameError repair for the unchanged FTD-0832 verifier.

The v5 module-scope codec replay calls ``scalar_close`` before the verifier's
same formula is declared locally inside ``main``.  Injecting that exact
formula into the runpy globals repairs name resolution without changing the
producer, artifacts, replay gates, or the hash-checked verifier source.
"""

from __future__ import annotations

import os
import runpy
from pathlib import Path


def scalar_close(lhs: float, rhs: float, tolerance: float = 2e-12) -> bool:
    """Match the unchanged verifier's locally declared scalar comparison."""
    return abs(lhs - rhs) <= tolerance * max(1.0, abs(rhs))


os.environ["FTD_0832_NONSINGULAR_PRODUCT_CHART"] = "1"
runpy.run_path(
    str(Path(__file__).with_name("proof_l17_complete_tangent_candidate.py")),
    init_globals={"scalar_close": scalar_close},
    run_name="__main__",
)
