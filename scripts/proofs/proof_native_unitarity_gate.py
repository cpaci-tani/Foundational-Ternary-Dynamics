"""FTD-0425 exact linear-sector and full-tick injectivity audit."""

from __future__ import annotations

import json
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
CATALOG = Path(__file__).with_name("_native_tick_injectivity.json")


def require(condition: bool, label: str) -> None:
    if not condition:
        raise AssertionError(label)
    print(f"PASS  {label}")


def main() -> None:
    checks = 0
    c2 = 1.0 / 3.0
    m_max = 16.0 / 3.0
    x_max = c2 * m_max
    require(x_max < 4.0, "L1 full-band production wave parameter lies inside the symplectic interval")
    checks += 1

    for x in np.linspace(1e-8, x_max, 1024):
        transfer = np.array([[1.0 - x, 1.0], [-x, 1.0]])
        require(abs(np.linalg.det(transfer) - 1.0) < 2e-15,
                f"L2 determinant-one witness x={x:.6g}") if x == x_max else None
        eig = np.linalg.eigvals(transfer)
        if np.max(np.abs(np.abs(eig) - 1.0)) >= 2e-12:
            raise AssertionError("L3 unit-modulus transfer roots across the band")
        energy_metric = np.array([[x, -0.5 * x], [-0.5 * x, 1.0]])
        if np.min(np.linalg.eigvalsh(energy_metric)) <= 0.0:
            raise AssertionError("L4 positive exact tick-energy metric away from zero mode")
    checks += 3
    print("PASS  L3 unit-modulus transfer roots across the complete band")
    print("PASS  L4 exact tick-energy metric is positive away from the zero mode")

    catalog = json.loads(CATALOG.read_text(encoding="utf-8"))
    phases = {row["phase"]: row for row in catalog["phases"]}
    require(len(phases) == 14, "I1 every preregistered tick phase/class is catalogued")
    checks += 1
    require(phases["evaporation"]["injectivity"] == "no"
            and phases["annihilation"]["injectivity"] == "no",
            "I2 state-erasing reaction maps are explicitly non-injective")
    checks += 1
    require(phases["Gauss projection"]["injectivity"] == "no",
            "I3 constraint projection is not mislabelled as reversible evolution")
    checks += 1
    require(phases["weak flip plus L/R swap"]["injectivity"] == "yes-event-map",
            "I4 the locally involutive weak event is distinguished from acceptance history")
    checks += 1

    source = (ROOT / "engine/tests/test_native_injectivity_gate.cpp").read_text(encoding="utf-8")
    require("two distinct signed preimages reach one evaporation image" in source
            and "annihilation erases distinct spin/color preimages" in source,
            "S1 engine counterexamples cover two independent information-loss routes")
    checks += 1
    audit = (ROOT / "docs/theory/07_assessment/lorentz_recovery_causal_structure/AUDIT_NATIVE_UNITARITY_GATE.md").read_text(encoding="utf-8")
    require("LINEAR-SECTOR-REVERSIBLE" in audit and "FULL-TICK-NON-INJECTIVE" in audit,
            "S2 scoped verdict separates the free sector from the full tick")
    checks += 1
    require("spectral-density" in audit and "not measured" in audit,
            "S3 missing manifested spectral-positivity gate remains explicit")
    checks += 1

    print(f"\nNative unitarity gate checks: {checks}/{checks} passed")
    print("VERDICT  LINEAR-SECTOR-REVERSIBLE; FULL-TICK-NON-INJECTIVE")


if __name__ == "__main__":
    main()
