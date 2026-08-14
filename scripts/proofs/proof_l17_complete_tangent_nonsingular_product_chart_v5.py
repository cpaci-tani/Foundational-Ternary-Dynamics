"""Independent replay entry point for preregistered FTD-0832."""

from __future__ import annotations

import os
import runpy
from pathlib import Path


os.environ["FTD_0832_NONSINGULAR_PRODUCT_CHART"] = "1"
runpy.run_path(
    str(Path(__file__).with_name("proof_l17_complete_tangent_candidate.py")),
    run_name="__main__",
)
