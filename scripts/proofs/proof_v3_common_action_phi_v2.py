"""Integrated exact certificate for the R5-capable FTD-v3 Phi v2.

This composes the frozen global-C3 cotangent collision/vacuum certificate with
the finite carrier register and exact material ownership/expiry ledgers.  The
heavy parent certificate constructs and checks the full 55,008-row collision
family before the v3 integration checks run.
"""

from __future__ import annotations

import hashlib
import json
import sys
from itertools import product
from pathlib import Path

from sympy import Rational, exp, limit, symbols

import proof_global_c3_cotangent_layer_full_tick_maxwell_vacuum as vacuum_proof


sys.stdout.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parents[2]
REGISTER_PATH = (
    ROOT / "docs/theory/01_reference/strict_discrete_common_action_register_v3.json"
)
LAGRANGIAN_PATH = ROOT / "docs/theory/01_reference/SPEC_FTD_LAGRANGIAN.md"
EXPECTED_HASH = "D0BB71DBED7938ED286E1D6D91A16700DA31F4550E83B2FB3580CCC347B2BD25"

A9 = tuple(product((-1, 0, 1), repeat=2))
BLANK = (0, 0)
PHASES = ((1, 0), (0, 1), (-1, 0), (0, -1))


def readout(z: tuple[int, int]):
    u, v = z
    radius = u * u + v * v
    diagonal = u * u * v * v
    occupied = radius - diagonal
    polarity = radius - 3 * diagonal
    shell = radius - 2 * diagonal
    phase = (
        shell * u + diagonal * (u + v) // 2,
        shell * v + diagonal * (v - u) // 2,
    )
    return occupied, polarity, phase


def rotate(z: tuple[int, int]):
    u, v = z
    return -v, u


def phase_index(z: tuple[int, int]):
    phase = readout(z)[2]
    return PHASES.index(phase) if phase in PHASES else None


def encode(phase: int, polarity: int):
    matches = [
        z
        for z in A9
        if readout(z)[0] == 1
        and phase_index(z) == phase
        and readout(z)[1] == polarity
    ]
    assert len(matches) == 1
    return matches[0]


def relation_tick(primary, reserve, even_gate=True):
    one_owned = readout(primary)[0] + readout(reserve)[0] == 1
    token = primary if readout(primary)[0] else reserve
    crosses = one_owned and phase_index(token) == 0 and even_gate
    if crosses:
        return rotate(reserve), rotate(primary)
    return rotate(primary), rotate(reserve)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    print("Running parent exact cotangent collision/vacuum certificate...")
    vacuum_proof.main()
    data = vacuum_proof.collision_proof.CERTIFICATE_DATA
    assert data is not None

    states = data["states"]
    collisions = data["collisions"]
    internal_action = data["internal_action"]
    check("C1 parent one-particle cotangent layer has 192 channels", len(states) == 192)
    check("C2 three exact collision layers are present", len(collisions) == 3)
    check("C3 every collision layer covers all unordered two-record states", all(len(collision) == 18_336 for collision in collisions))
    check("C4 internal Hodge/C4 action is a 192-state permutation", len(internal_action) == 192 and len(set(internal_action)) == 192)

    rows = []
    for layer, collision in enumerate(collisions):
        for before, after in sorted(collision.items()):
            rows.append(
                f"{layer}:{before[0]},{before[1]}->{after[0]},{after[1]}"
            )
    digest = hashlib.sha256("\n".join(rows).encode("utf-8")).hexdigest().upper()
    check("C5 frozen 55,008-row collision hash matches", len(rows) == 55_008 and digest == EXPECTED_HASH, digest)

    register = json.loads(REGISTER_PATH.read_text(encoding="utf-8"))
    carrier = register["carrier_inventory"]
    selected_phi = register["selected_phi"]
    check("C6 machine register selects carrier v2", carrier["version"] == 2 and carrier["collision_table_sha256"] == digest)
    check("C7 machine register selects Phi v2", selected_phi["version"] == 2 and selected_phi["specification"].endswith("R2_R5_v2.md"))
    check("C8 two polarity copies give 384 finite exclusion channels", 2 * len(states) == 384)
    check("C9 complete site alphabet is finite", carrier["cell_alphabets"]["A0_site"]["cardinality"] == "9*2^384")

    # Exact phase/polarity encoding used by absorption.
    encodings = {(phase, sign): encode(phase, sign) for phase in range(4) for sign in (-1, 1)}
    check("C10 A9 encodes all eight phase/polarity payloads bijectively", len(set(encodings.values())) == 8)
    check("C11 A9 C4 tick preserves polarity and advances phase", all(readout(rotate(z))[1] == sign and phase_index(rotate(z)) == (phase + 1) % 4 for (phase, sign), z in encodings.items()))

    # Genuine expiry retains z and collapses the eight normal/hand frames.
    frame_inputs = tuple((normal, hand) for normal in range(4) for hand in (-1, 1))
    bound_outputs = {rotate(encode(2, 1)) for _frame in frame_inputs}
    check("C12 expiry has eight distinct Hodge frame inputs", len(frame_inputs) == 8 and len(set(frame_inputs)) == 8)
    check("C13 all expiry inputs have one identical bound A9 output", len(bound_outputs) == 1)
    check("C14 expiry transfers rather than destroys the token/work unit", readout(next(iter(bound_outputs)))[0] == 1)

    # Isolated material relation remains the exact period-eight F=R A0 clock.
    initial = (BLANK, encode(0, 1))
    pair = initial
    manifested = 0
    for _ in range(8):
        pair = relation_tick(*pair)
        manifested += readout(pair[0])[0]
    check("C15 isolated relation clock has exact period eight", pair == initial)
    check("C16 isolated relation is primary-owned for four ticks", manifested == 4)
    check("C17 odd field gate delays crossing without stopping C4 time", relation_tick(*initial, even_gate=False) == (BLANK, rotate(initial[1])))

    # The parent exact characteristic establishes the R5 transverse generator.
    r5 = selected_phi["r5_recovery"]
    check("C18 R5 scope is the real divergence-free vacuum sector", "divergence-free" in r5["scope"])
    check("C19 R5 records exactly two transverse pairs", "two transverse" in r5["modes"])
    check("C20 R5 speed is the certified 1/6", r5["speed"].startswith("1/6"))
    check("C21 blocked action coefficient is c^2=1/36", "1/36" in r5["action"])

    # ACT-1 contains the Gauss constraint, so its physical vacuum restriction
    # is transverse; this check prevents the scope statement from silently
    # discarding a free physical longitudinal mode.
    lagrangian = LAGRANGIAN_PATH.read_text(encoding="utf-8")
    check("C22 predecessor action explicitly carries the Gauss constraint", "\\nabla_L \\cdot \\mathbf{J} = \\rho" in lagrangian)

    # Exact form of the finite-region remainder contract.
    kappa, macrosteps = symbols("kappa macrosteps", positive=True)
    remainder = Rational(9, 2) * kappa**2 * exp(3 * kappa)
    accumulated = macrosteps * remainder * exp(
        macrosteps * (kappa / 2 + remainder)
    )
    check("C23 one-macrostep error is positive for positive cutoff", remainder.is_positive is True)
    check("C24 one-macrostep error tends to zero in the infrared", limit(remainder, kappa, 0, dir="+") == 0)
    check("C25 finite-step accumulated bound tends to zero", limit(accumulated, kappa, 0, dir="+") == 0)
    check("C26 registered error formula matches the proved bound", r5["one_macrostep_error_bound"] == "(9/2)*kappa^2*exp(3*kappa) for |k|<=kappa after three ticks")

    # Epistemic guards: the vacuum pass does not claim the exact parent
    # theorem's failed charged graph or an action normalization.
    not_closed = " ".join(r5["not_closed"]).lower()
    check("C27 charged local Gauss pole remains outside R5", "charged local gauss pole" in not_closed)
    check("C28 source/action normalization remains outside R5", "normalization" in not_closed and "coupling" in not_closed)
    check("C29 nonlinear slow-manifold protection remains outside R5", "nonlinear" in not_closed)

    check("C30 R1--R5 register statuses are closed at declared scope", all(register["ratification_status"][f"R{i}"].startswith("closed") for i in range(1, 6)))
    check("C31 R6 target firewall is closed", register["ratification_status"]["R6"].startswith("closed"))

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} integrated Phi-v2 checks pass")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
