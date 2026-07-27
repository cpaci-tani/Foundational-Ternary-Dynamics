#!/usr/bin/env python3
"""Independent theorem/result verifier for FTD-0588.

The source counts, volumes, histories, seeds, and moment-isotropic fixtures are
pre-registered.  This script evaluates only the locked operator norms and run
records; it performs no geometry, amplitude, or near-match search.
"""

from __future__ import annotations

import csv
import hashlib
import itertools
import json
import math
import subprocess
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PREREG_SHA = "06DE9E8B896272044D847FF5BEC53A342928E3B210B61AC4D3AD605D9D36692E"
VOLUMES = (9, 17, 33, 65)
G_C = 0.0854245431028543695
C2 = 1.0 / 3.0
K_GENESIS = 1.5163860591519780
TOL = 1.0e-12

LOCKED_HASHES = {
    "preregistration": (
        "docs/theory/10_eft_program/preregistrations/"
        "PREREG_COLLECTIVE_SOURCE_HISTORY_BOUND_v1.md",
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
        print("FTD-0588 collective source-history bound")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        print("-" * 79)
        print(f"checks={len(self.rows)} passed={passed} "
              f"failed={len(self.rows)-passed}")
        return passed == len(self.rows)


def sha256(relative: str) -> str:
    return hashlib.sha256((ROOT / relative).read_bytes()).hexdigest().upper()


def spectral_bound(lattice_size: int) -> dict[str, float]:
    triangle_sum = 0.0
    common_sum = 0.0
    maximum_a = 0.0
    maximum_ratio = 0.0
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
        gradient2 = sx * sx + sy * sy + sz * sz
        a = C2 * symbol
        maximum_a = max(maximum_a, a)
        maximum_ratio = max(maximum_ratio, gradient2 / symbol)
        mode_bound = 1.0 + 1.0 / math.sqrt(1.0 - a / 4.0)
        triangle_sum += mode_bound * math.sqrt(gradient2) / symbol
        common_sum += mode_bound * mode_bound / symbol
    volume = lattice_size**3
    one_step = G_C * triangle_sum / (C2 * volume)
    common_step = G_C / C2 * math.sqrt(common_sum / volume)
    return {
        "maximum_a": maximum_a,
        "one_step": one_step,
        "common_step": common_step,
        "common_five": 2.0 * common_step * math.sqrt(5.0),
        "common_six": 2.0 * common_step * math.sqrt(6.0),
        "common_five_margin": K_GENESIS
        - 2.0 * common_step * math.sqrt(5.0),
        "async_four": 2.0 * common_step + 4.0 * one_step,
        "async_four_margin": K_GENESIS
        - (2.0 * common_step + 4.0 * one_step),
        "five_remaining": common_step * math.sqrt(5.0) + 4.0 * one_step,
        "five_remaining_margin": K_GENESIS
        - (common_step * math.sqrt(5.0) + 4.0 * one_step),
        "five_all_removed": common_step * math.sqrt(5.0) + 5.0 * one_step,
        "five_all_removed_margin": K_GENESIS
        - (common_step * math.sqrt(5.0) + 5.0 * one_step),
        "maximum_ratio": maximum_ratio,
    }


P = Proof()

for name, (relative, expected) in LOCKED_HASHES.items():
    actual = sha256(relative)
    P.check(f"frozen hash {name}", actual == expected, actual)

# Exact stencil domination identity.
cx, cy, cz = sp.symbols("cx cy cz", real=True)
p = cx + cy + cz
q = cx**2 + cy**2 + cz**2
symbol = (
    sp.Integer(4)
    - sp.Rational(2, 3) * (cx + cy + cz)
    - sp.Rational(2, 3) * (cx * cy + cx * cz + cy * cz)
)
gradient2 = 3 - q
decomposition = 4 * (q - p**2 / 3) + (p - 3) ** 2 / 3
P.check(
    "exact gradient/stencil decomposition",
    sp.expand(3 * (symbol - gradient2) - decomposition) == 0,
    "3(M-g^2)=4(q-p^2/3)+(p-3)^2/3",
)
P.check(
    "decomposition terms are nonnegative",
    sp.expand(
        3 * q - p**2
        - ((cx - cy) ** 2 + (cx - cz) ** 2 + (cy - cz) ** 2)
    ) == 0,
    "q-p^2/3 is the coordinate variance",
)

# Exact moment selection of the live fixture; no geometry search.
tetra = (
    sp.Matrix((1, 1, 1)),
    sp.Matrix((1, -1, -1)),
    sp.Matrix((-1, 1, -1)),
    sp.Matrix((-1, -1, 1)),
)
first_moment = sum(tetra, sp.zeros(3, 1))
second_moment = sum((r * r.T for r in tetra), sp.zeros(3, 3))
P.check("tetrahedral first moment vanishes", first_moment == sp.zeros(3, 1),
        str(first_moment.T))
P.check("tetrahedral second moment is isotropic",
        second_moment == 4 * sp.eye(3), str(second_moment))

# Finite-group Parseval follows from character orthogonality.  Verify each
# one-dimensional geometric sum exactly in the cyclotomic quotient rather
# than relying on floating roots of unity or heuristic simplification.
x = sp.symbols("x")


def character_factor(lattice_size: int, displacement: int) -> int:
    if displacement % lattice_size == 0:
        return lattice_size
    polynomial = sum(
        x ** ((n * displacement) % lattice_size)
        for n in range(lattice_size)
    )
    remainder = sp.rem(
        polynomial, sp.cyclotomic_poly(lattice_size, x), x,
        domain=sp.ZZ,
    )
    return 0 if remainder == 0 else -1


for volume in VOLUMES:
    for displacement in ((0, 0, 0), (1, 0, 0), (1, -2, 3)):
        factors = [character_factor(volume, d) for d in displacement]
        character_sum = math.prod(factors)
        expected = volume**3 if all(d % volume == 0 for d in displacement) else 0
        P.check(
            f"L={volume} character orthogonality {displacement}",
            character_sum == expected,
            f"factors={factors} expected={expected}",
        )

bounds = {volume: spectral_bound(volume) for volume in VOLUMES}
for volume, row in bounds.items():
    P.check(f"L={volume} stable spectrum", 0.0 < row["maximum_a"] < 4.0,
            f"a_max={row['maximum_a']:.17g}")
    P.check(f"L={volume} strict nonzero-mode domination",
            row["maximum_ratio"] < 1.0,
            f"max(g^2/M)={row['maximum_ratio']:.17g}")
    P.check(f"L={volume} common N=5 subcritical",
            row["common_five_margin"] > 0.0,
            f"bound={row['common_five']:.17g} "
            f"margin={row['common_five_margin']:.17g}")
    P.check(f"L={volume} asynchronous N=4 subcritical",
            row["async_four_margin"] > 0.0,
            f"bound={row['async_four']:.17g} "
            f"margin={row['async_four_margin']:.17g}")
    P.check(f"L={volume} N=5 closed while an original remains",
            row["five_remaining_margin"] > 0.0,
            f"bound={row['five_remaining']:.17g} "
            f"margin={row['five_remaining_margin']:.17g}")
    P.check(f"L={volume} all-off N=5 envelope is inconclusive",
            row["five_all_removed_margin"] < 0.0,
            f"bound={row['five_all_removed']:.17g} "
            f"margin={row['five_all_removed_margin']:.17g}")

P.check("N=6 common pulse not uniformly excluded",
        bounds[65]["common_six"] > K_GENESIS,
        f"L65 bound={bounds[65]['common_six']:.17g}")

binary = ROOT / "engine/build/Release/test_collective_source_history_bound.exe"
P.check("native result binary exists", binary.exists(), str(binary))
output = ""
if binary.exists():
    completed = subprocess.run(
        [str(binary)], cwd=ROOT, capture_output=True, text=True, check=False
    )
    P.check("native result binary exits cleanly", completed.returncode == 0,
            f"returncode={completed.returncode}")
    output = completed.stdout
else:
    P.check("native result binary exits cleanly", False, "binary missing")

P.check("native verdict is one registered residual-tail outcome",
        "verdict=COMMON_N_LE_5_ASYNC_N_LE_4_CLOSED_"
        "N5_RESIDUAL_TAIL_" in output,
        "registered verdict prefix")

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
        ("one_source_step_triangle_bound", "one_step"),
        ("common_step_coefficient", "common_step"),
        ("common_pulse_five_source_bound", "common_five"),
        ("common_pulse_six_source_bound", "common_six"),
        ("common_five_source_margin", "common_five_margin"),
        ("asynchronous_four_source_bound", "async_four"),
        ("asynchronous_four_source_margin", "async_four_margin"),
        ("five_source_while_original_remains_bound", "five_remaining"),
        ("five_source_while_original_remains_margin", "five_remaining_margin"),
        ("five_source_all_removed_envelope", "five_all_removed"),
        ("five_source_all_removed_margin", "five_all_removed_margin"),
        ("maximum_gradient_stencil_ratio", "maximum_ratio"),
    ):
        label = f"L{volume}_{suffix}"
        observed = scalars.get(label, math.nan)
        P.check(
            f"native/independent {label}",
            abs(observed - row[key]) <= 5.0e-15,
            f"native={observed:.17g} independent={row[key]:.17g}",
        )

required_scalars = {
    "common_history_arms": 64,
    "native_unlocked_arms": 64,
    "total_arms": 128,
    "total_ticks": 16384,
    "common_history_genesis_events": 0,
    "asynchronous_four_source_genesis_events": 0,
    "analytic_contradiction_events": 0,
    "maximum_bound_excess": 0,
    "maximum_velocity": 0,
    "maximum_remainder": 0,
}
for key, expected in required_scalars.items():
    observed = scalars.get(key, math.nan)
    P.check(f"native gate {key}", observed == expected,
            f"observed={observed} expected={expected}")

P.check("native unlocked arms exercise complete removal",
        scalars.get("unlocked_arms_all_sources_removed", 0) > 0,
        f"arms={scalars.get('unlocked_arms_all_sources_removed', math.nan)}")

json_path = ROOT / "engine/results/ftd_0588/windows_msvc_cpu.json"
csv_path = ROOT / "engine/results/ftd_0588/windows_msvc_cpu.csv"
P.check("JSON run record exists", json_path.exists(), str(json_path))
P.check("CSV run record exists", csv_path.exists(), str(csv_path))
if json_path.exists():
    record = json.loads(json_path.read_text(encoding="utf-8"))
    P.check("JSON identifier and preregistration lock",
            record.get("identifier") == "FTD-0588"
            and record.get("preregistration_sha256") == PREREG_SHA,
            str(record.get("verdict")))
    P.check("JSON structural result is valid",
            record.get("structural_valid") is True,
            str(record.get("structural_valid")))
if csv_path.exists():
    with csv_path.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    P.check("CSV contains exactly 128 registered arms", len(rows) == 128,
            f"rows={len(rows)}")
    histories = {row["history"] for row in rows}
    P.check("CSV contains all three history classes",
            histories == {"locked_step", "synchronous_pulse", "native_unlocked"},
            str(sorted(histories)))

observer_source = (
    ROOT / "engine/src/eft/collective_source_history_bound.cpp"
).read_text(encoding="utf-8")
for forbidden in (
    "gauss_projection = true",
    "matched_gauss_dynamics = true",
    "forces = true",
    "movement = true",
    "dual_substrate = true",
):
    P.check(f"forbidden mechanism absent: {forbidden}",
            forbidden not in observer_source, forbidden)
P.check("source counts and volumes are fixed",
        "for (int source_count : {4, 5})" in observer_source
        and "SPECTRAL_VOLUMES{{9, 17, 33, 65}}" in observer_source,
        "registered N={4,5}, L={9,17,33,65}")
P.check("no production toggle is introduced",
        "common_action_face_dynamics" not in observer_source,
        "observer-only implementation")

raise SystemExit(0 if P.report() else 1)
