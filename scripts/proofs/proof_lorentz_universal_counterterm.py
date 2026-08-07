"""One-calibration threshold test in the frozen FTD-0419 step scheme.

This is not the missing on-shell calculation. It asks the narrower, already
computable question: after one universal eta cancels the N_f=1 step-scheme
coefficient, what does the same eta predict when the active fermion count
changes? No parameter is refitted.
"""

from __future__ import annotations

import csv
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from scripts.constants import ALPHA_EFT  # noqa: E402

BZ_DATA = Path(__file__).with_name("_lorentz_full_bz_matching.csv")
TRAJECTORY_DATA = Path(__file__).with_name("_lorentz_universal_counterterm.csv")


def require(condition: bool, label: str) -> None:
    if not condition:
        raise AssertionError(label)
    print(f"PASS  {label}")


def limit(key: str) -> float:
    with BZ_DATA.open(newline="", encoding="utf-8") as handle:
        rows = [row for row in csv.DictReader(handle)
                if row["scheme"] == "qedl_step" and int(row["N"]) >= 96]
    n = np.array([float(row["N"]) for row in rows])
    y = np.array([float(row[key]) for row in rows])
    design = np.column_stack([
        np.ones_like(n), np.log(n) / n**2, 1.0 / n**2, 1.0 / n**4
    ])
    return float(np.linalg.lstsq(design, y, rcond=None)[0][0])


def main() -> None:
    matter = limit("Zs_minus_Zt")
    photon = limit("ZB_minus_ZE")
    bare_one = matter - 0.5 * photon
    eta = -bare_one
    require(abs(bare_one + 0.32696905665) < 1e-9,
            "C1 reference coefficient reproduces FTD-0419")
    require(abs(eta + bare_one) < 1e-15,
            "C2 one universal eta cancels the N_f=1 reference")

    with TRAJECTORY_DATA.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    require([int(row["N_f"]) for row in rows] == [1, 2, 3, 6],
            "C3 trajectory contains the preregistered multiplicities")

    for row in rows:
        nf = int(row["N_f"])
        bare = matter - 0.5 * nf * photon
        residual = bare + eta
        require(abs(bare - float(row["bare_match_per_g2"])) < 2e-9,
                f"T{nf} bare threshold is independently reproduced")
        require(abs(residual - float(row["residual_per_g2"])) < 2e-9,
                f"R{nf} residual uses the same eta without retuning")

    residual_two = -0.5 * photon
    translated = abs(residual_two * ALPHA_EFT)
    after_rg = translated / 137.0**3
    require(1e-4 < translated < 1.1e-4,
            "P1 first threshold leaves an O(10^-4) selected-alpha mismatch")
    require(after_rg / 1e-15 > 4e4,
            "P2 optimistic FTD-0416 running misses the declared tolerance by >4e4")

    audit = (ROOT / "docs/theory/07_assessment/lorentz_recovery_causal_structure/AUDIT_LORENTZ_ON_SHELL_COUNTERTERM.md").read_text(encoding="utf-8")
    require("not an on-shell result" in audit and "scheme-specific" in audit,
            "S1 surrogate is not promoted to physical pole matching")
    require("ONE-CALIBRATION-SURROGATE-FAILS-THRESHOLD" in audit,
            "S2 scoped threshold verdict is explicit")
    contract = (ROOT / "engine/include/ftd/eft/pole_matching.h").read_text(encoding="utf-8")
    require("PoleMatchResult" in contract and "CountertermTrajectory" in contract
            and "calibrate_once" in contract,
            "S3 scheme-carrying one-calibration interface is present")

    print("\nUniversal-counterterm surrogate checks: 16/16 passed")
    print(f"matter difference/g^2 = {matter:.12f}")
    print(f"photon difference/g^2 = {photon:.12f}")
    print(f"eta(N_f=1)/g^2       = {eta:.12f}")
    print(f"N_f=2 residual/g^2   = {residual_two:.12f}")


if __name__ == "__main__":
    main()
