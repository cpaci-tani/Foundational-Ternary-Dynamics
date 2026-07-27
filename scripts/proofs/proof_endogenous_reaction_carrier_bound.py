#!/usr/bin/env python3
"""Independent proof and result verifier for FTD-0586.

The volumes, source counts, and thresholds are preregistered.  This script
re-derives the modal response and evaluates only those fixed structural sums;
it performs no near-miss or physical-constant search.
"""

from __future__ import annotations

import hashlib
import itertools
import math
import re
import subprocess
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PREREG_SHA = "2AB91067BD68FC995BDF0318843E074ADF027ADE899F9F2DF1688C0D07F64251"
VERDICT = (
    "ENDOGENOUS_N_LE_3_AUTOCATALYSIS_CLOSED_"
    "BOUND_INCONCLUSIVE_AT_N_GE_4"
)
VOLUMES = (9, 17, 33, 65)
G_C = 0.0854245431028543695
C2 = 1.0 / 3.0
K_GENESIS = 1.5163860591519780
TOL = 1.0e-12

LOCKED_HASHES = {
    "preregistration": (
        "docs/theory/10_eft_program/preregistrations/"
        "PREREG_ENDOGENOUS_REACTION_CARRIER_BOUND_v1.md",
        PREREG_SHA,
    ),
    "render_bridge": (
        "engine/src/render_bridge.cpp",
        "A822E0FAFAF71FE5458B2A7450868A8414B1C8564089BF6C6484FC34B7559359",
    ),
    "phase_read": (
        "engine/src/render_bridge_phases/phase_read.cpp",
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    ),
    "phase_write": (
        "engine/src/render_bridge_phases/phase_write.cpp",
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    ),
    "toggles": (
        "engine/include/ftd/term_toggles.h",
        "2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA",
    ),
    "couplings": (
        "engine/include/ftd/ontic/gauge_couplings.h",
        "BC862D8120E0F3D83B7FAD0201F8D4DF46B5BAD5E7D52CD571AF68BECA3EB0F3",
    ),
    "masses": (
        "engine/include/ftd/ontic/particle_masses.h",
        "E43E01D5F1F870EE019754BEA7E932529346C9B8EB704B40215CA559FC5A4F57",
    ),
}


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: str) -> None:
        self.rows.append((bool(condition), name, note))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0586 endogenous reaction-carrier bound")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        print("-" * 79)
        print(f"checks={len(self.rows)} passed={passed} "
              f"failed={len(self.rows)-passed}")
        print(f"verdict={VERDICT}")
        return passed == len(self.rows)


def sha256(relative: str) -> str:
    return hashlib.sha256((ROOT / relative).read_bytes()).hexdigest().upper()


def spectral_bound(lattice_size: int) -> dict[str, float]:
    total = 0.0
    maximum_a = 0.0
    for nx, ny, nz in itertools.product(range(lattice_size), repeat=3):
        kx = 2.0 * math.pi * nx / lattice_size
        ky = 2.0 * math.pi * ny / lattice_size
        kz = 2.0 * math.pi * nz / lattice_size
        cx, cy, cz = math.cos(kx), math.cos(ky), math.cos(kz)
        sx, sy, sz = math.sin(kx), math.sin(ky), math.sin(kz)
        symbol = (
            4.0
            - (2.0 / 3.0) * (cx + cy + cz)
            - (2.0 / 3.0) * (cx * cy + cx * cz + cy * cz)
        )
        if symbol <= 1.0e-14:
            continue
        a = C2 * symbol
        maximum_a = max(maximum_a, a)
        mode_bound = 1.0 + 1.0 / math.sqrt(1.0 - a / 4.0)
        gradient = math.sqrt(sx * sx + sy * sy + sz * sz)
        total += mode_bound * gradient / symbol
    step = G_C * total / (C2 * lattice_size**3)
    pulse = 2.0 * step
    return {
        "maximum_a": maximum_a,
        "step": step,
        "pulse": pulse,
        "three": 3.0 * pulse,
        "margin": K_GENESIS - 3.0 * pulse,
    }


P = Proof()

for name, (relative, expected) in LOCKED_HASHES.items():
    actual = sha256(relative)
    P.check(f"frozen hash {name}", actual == expected, actual)

# Symbolic recurrence and amplitude identities.
a, n, theta = sp.symbols("a n theta", real=True)
cosine = 1 - a / 2
P.check("production stability ceiling", sp.Rational(16, 9) < 4,
        "a_max=16/9<4")

# The 18-point symbol is multi-affine in the three cosines, so its extrema on
# [-1,1]^3 occur at the eight vertices.
vertex_symbols = []
for cx, cy, cz in itertools.product((-1, 1), repeat=3):
    symbol = (
        sp.Integer(4)
        - sp.Rational(2, 3) * (cx + cy + cz)
        - sp.Rational(2, 3) * (cx * cy + cx * cz + cy * cz)
    )
    vertex_symbols.append(symbol)
P.check("exact 18-point symbol range",
        min(vertex_symbols) == 0 and max(vertex_symbols) == sp.Rational(16, 3),
        f"vertices={vertex_symbols}")

tan_half_sq = sp.simplify((1 - cosine) / (1 + cosine))
P.check("half-angle identity",
        sp.simplify(tan_half_sq - a / (4 - a)) == 0,
        f"tan(theta/2)^2={tan_half_sq}")
oscillation_amplitude_sq = sp.simplify(1 + tan_half_sq)
P.check("step oscillation amplitude",
        sp.simplify(oscillation_amplitude_sq - 4 / (4 - a)) == 0,
        "sqrt(1+tan^2)=sec(theta/2)")

# Direct fixed-n recurrence checks avoid asking SymPy to reason about an
# integer-valued symbolic n under trigonometric assumptions.
for index in range(1, 7):
    def response(j: int) -> sp.Expr:
        return 1 - sp.cos(j * theta) + sp.tan(theta / 2) * sp.sin(j * theta)

    residual = sp.simplify(sp.expand_trig(
        response(index + 1) - 2 * response(index) + response(index - 1)
        + (2 - 2 * sp.cos(theta)) * response(index)
        - (2 - 2 * sp.cos(theta))
    ))
    P.check(f"forced recurrence n={index}", residual == 0,
            f"residual={residual}")

bounds = {volume: spectral_bound(volume) for volume in VOLUMES}
maximum_pulse = max(row["pulse"] for row in bounds.values())
for volume, row in bounds.items():
    P.check(f"L={volume} stable spectrum", 0.0 < row["maximum_a"] < 4.0,
            f"a_max={row['maximum_a']:.17g}")
    P.check(f"L={volume} exact pulse factor",
            abs(row["pulse"] - 2.0 * row["step"]) <= 1.0e-15,
            f"step={row['step']:.17g} pulse={row['pulse']:.17g}")
    P.check(f"L={volume} three-source margin", row["margin"] > 0.0,
            f"3B={row['three']:.17g} margin={row['margin']:.17g}")

closed_count = math.floor((K_GENESIS - TOL) / maximum_pulse)
P.check("triangle bound closes exactly through N=3", closed_count == 3,
        f"floor((K_GENESIS-tol)/max(B_L))={closed_count}")
P.check("N=4 is bound-inconclusive, not positive",
        4.0 * maximum_pulse > K_GENESIS,
        f"4B={4.0*maximum_pulse:.17g} K_GENESIS={K_GENESIS:.17g}")

binary = ROOT / "engine/build/Release/test_endogenous_reaction_carrier_bound.exe"
P.check("native result binary exists", binary.exists(), str(binary))
completed = subprocess.run(
    [str(binary)], cwd=ROOT, capture_output=True, text=True, check=False
)
P.check("native result binary exits cleanly", completed.returncode == 0,
        f"returncode={completed.returncode}")
output = completed.stdout
P.check("native verdict matches lock", f"verdict={VERDICT}" in output,
        VERDICT)

scalars: dict[str, float] = {}
for line in output.splitlines():
    if "=" not in line or line.startswith("verdict="):
        continue
    key, value = line.split("=", 1)
    try:
        scalars[key.strip()] = float(value.strip())
    except ValueError:
        pass

for volume, row in bounds.items():
    for suffix, key in (
        ("maximum_mode_eigenvalue", "maximum_a"),
        ("single_source_step_bound", "step"),
        ("single_source_pulse_bound", "pulse"),
        ("three_source_pulse_bound", "three"),
        ("threshold_margin", "margin"),
    ):
        label = f"L{volume}_{suffix}"
        observed = scalars.get(label, math.nan)
        P.check(f"native/independent {label}",
                abs(observed - row[key]) <= 5.0e-15,
                f"native={observed:.17g} independent={row[key]:.17g}")

required_scalars = {
    "endogenous_arms": 96,
    "endogenous_ticks": 12288,
    "constant_source_arms": 48,
    "pulse_source_arms": 48,
    "endogenous_genesis_events": 0,
    "endogenous_evaporation_events": 96,
    "external_control_genesis_events": 4,
    "maximum_bound_excess": 0,
    "maximum_velocity": 0,
    "maximum_remainder": 0,
    "minimum_sources_not_excluded": 4,
}
for key, expected in required_scalars.items():
    observed = scalars.get(key, math.nan)
    P.check(f"native gate {key}", observed == expected,
            f"observed={observed} expected={expected}")

observer_source = (
    ROOT / "engine/src/eft/endogenous_reaction_carrier_bound.cpp"
).read_text(encoding="utf-8")
for forbidden in (
    "gauss_projection = true",
    "matched_gauss_dynamics = true",
    "forces = true",
    "movement = true",
    "dual_substrate = true",
):
    P.check(f"forbidden endogenous mechanism absent: {forbidden}",
            forbidden not in observer_source, forbidden)
P.check("zero initial kinematics are explicit",
        "voxel.velocity = {};" in observer_source
        and "voxel.remainder = {};" in observer_source,
        "velocity/remainder sanitized before every live arm")
P.check("source counts are fixed, not scanned",
        "for (int source_count : {1, 3})" in observer_source,
        "registered N={1,3}; N=4 not executed")

raise SystemExit(0 if P.report() else 1)
