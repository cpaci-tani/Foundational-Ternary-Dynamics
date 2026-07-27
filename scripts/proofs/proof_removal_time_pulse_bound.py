#!/usr/bin/env python3
"""Independent theorem/result verifier for FTD-0589.

The operator, volumes, histories, source fixtures, seeds, and tolerances are
pre-registered.  This script evaluates the locked exact pulse cancellation,
finite-volume bounds, and run records.  It performs no geometry, schedule,
amplitude, constant, or near-match search.
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
PREREG_SHA = "F438DBB1950E009641B1332D57B23B2EDFC23CD522A4E23C17E5FCC967AF5A33"
VOLUMES = (9, 17, 33, 65)
G_C = 0.0854245431028543695
C2 = 1.0 / 3.0
K_GENESIS = 1.5163860591519780
TOL = 1.0e-12

LOCKED_HASHES = {
    "preregistration": (
        "docs/theory/10_eft_program/preregistrations/"
        "PREREG_REMOVAL_TIME_PULSE_BOUND_v1.md",
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
        print("FTD-0589 removal-time pulse bound")
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


def spectral_bound(lattice_size: int) -> dict[str, float | int]:
    step_sum = 0.0
    pulse_sum = 0.0
    common_sum = 0.0
    maximum_a = 0.0
    for nx, ny, nz in itertools.product(range(lattice_size), repeat=3):
        kx, ky, kz = (
            2.0 * math.pi * n / lattice_size for n in (nx, ny, nz)
        )
        cx, cy, cz = math.cos(kx), math.cos(ky), math.cos(kz)
        sx, sy, sz = math.sin(kx), math.sin(ky), math.sin(kz)
        symbol = (
            4.0
            - (2.0 / 3.0) * (cx + cy + cz)
            - (2.0 / 3.0) * (cx * cy + cx * cz + cy * cz)
        )
        if symbol <= 1.0e-14:
            continue
        gradient = math.sqrt(sx * sx + sy * sy + sz * sz)
        a = C2 * symbol
        secant = 1.0 / math.sqrt(1.0 - a / 4.0)
        step_envelope = 1.0 + secant
        maximum_a = max(maximum_a, a)
        step_sum += step_envelope * gradient / symbol
        pulse_sum += 2.0 * secant * gradient / symbol
        common_sum += step_envelope * step_envelope / symbol
    volume = lattice_size**3
    step = G_C * step_sum / (C2 * volume)
    pulse = G_C * pulse_sum / (C2 * volume)
    common = G_C / C2 * math.sqrt(common_sum / volume)

    def history_maximum(source_count: int) -> tuple[float, int]:
        values = [
            common * math.sqrt(source_count - removed) + removed * pulse
            for removed in range(source_count + 1)
        ]
        maximizing = max(range(source_count + 1), key=values.__getitem__)
        return values[maximizing], maximizing

    count = 1
    while history_maximum(count)[0] + TOL < K_GENESIS:
        count += 1
    closed_bound, closed_removed = history_maximum(count - 1)
    open_bound, open_removed = history_maximum(count)
    return {
        "maximum_a": maximum_a,
        "step": step,
        "pulse": pulse,
        "common": common,
        "closed_count": count - 1,
        "first_open_count": count,
        "closed_removed": closed_removed,
        "open_removed": open_removed,
        "closed_bound": closed_bound,
        "closed_margin": K_GENESIS - closed_bound,
        "open_bound": open_bound,
        "open_margin": K_GENESIS - open_bound,
        "relaxation": (count - 1) * pulse + common * common / (4.0 * pulse),
    }


P = Proof()

for name, (relative, expected) in LOCKED_HASHES.items():
    actual = sha256(relative)
    P.check(f"frozen hash {name}", actual == expected, actual)

# Exact cancellation: first use sum/difference identities, then absorb the
# tan(theta/2) term into a shifted sine.  These are algebraic identities in
# independent real angles, not sampled numerical equalities.
A, B, h = sp.symbols("A B h", real=True)
t = sp.tan(h)
direct = -sp.cos(A) + sp.cos(A - B) + t * (
    sp.sin(A) - sp.sin(A - B)
)
half_factored = 2 * sp.sin(B / 2) * (
    sp.sin(A - B / 2) + t * sp.cos(A - B / 2)
)
P.check(
    "exact rectangular-pulse difference factorization",
    sp.trigsimp(sp.expand_trig(direct - half_factored)) == 0,
    "r_n-r_(n-T) loses the constant step term",
)
X = sp.symbols("X", real=True)
shift_identity = (
    sp.sin(X) + sp.tan(h) * sp.cos(X)
    - sp.sin(X + h) / sp.cos(h)
)
P.check(
    "exact secant shifted-sine identity",
    sp.trigsimp(sp.expand_trig(shift_identity)) == 0,
    "sin(X)+tan(h)cos(X)=sec(h)sin(X+h)",
)

# Completing the square proves the continuous relaxation.
y, common_symbol, pulse_symbol = sp.symbols(
    "y common pulse", nonnegative=True
)
square_gap = (
    pulse_symbol * y**2 - common_symbol * y
    + common_symbol**2 / (4 * pulse_symbol)
)
P.check(
    "continuous history relaxation is a completed square",
    sp.simplify(
        square_gap
        - (common_symbol - 2 * pulse_symbol * y) ** 2
          / (4 * pulse_symbol)
    ) == 0,
    "P*y^2-C*y+C^2/(4P)=(2Py-C)^2/(4P)",
)

bounds = {volume: spectral_bound(volume) for volume in VOLUMES}
for volume, row in bounds.items():
    P.check(
        f"L={volume} stable spectrum",
        0.0 < float(row["maximum_a"]) < 4.0,
        f"a_max={float(row['maximum_a']):.17g}",
    )
    P.check(
        f"L={volume} exact pulse is sharper than doubled step",
        float(row["pulse"]) < 2.0 * float(row["step"]),
        f"P={float(row['pulse']):.17g} 2S={2*float(row['step']):.17g}",
    )
    P.check(
        f"L={volume} arbitrary removals close through six",
        row["closed_count"] == 6 and float(row["closed_margin"]) > 0.0,
        f"bound={float(row['closed_bound']):.17g} "
        f"margin={float(row['closed_margin']):.17g}",
    )
    P.check(
        f"L={volume} seven is first unexcluded count",
        row["first_open_count"] == 7 and float(row["open_margin"]) < 0.0,
        f"bound={float(row['open_bound']):.17g} "
        f"margin={float(row['open_margin']):.17g}",
    )
    P.check(
        f"L={volume} exact discrete maximum beats relaxation",
        float(row["closed_bound"]) <= float(row["relaxation"]),
        f"exact={float(row['closed_bound']):.17g} "
        f"relaxed={float(row['relaxation']):.17g}",
    )

binary = ROOT / "engine/build/Release/test_removal_time_pulse_bound.exe"
P.check("native result binary exists", binary.exists(), str(binary))
output = ""
if binary.exists():
    completed = subprocess.run(
        [str(binary)], cwd=ROOT, capture_output=True, text=True, check=False
    )
    P.check(
        "native result binary exits cleanly",
        completed.returncode == 0,
        f"returncode={completed.returncode}",
    )
    output = completed.stdout
else:
    P.check("native result binary exits cleanly", False, "binary missing")

P.check(
    "native verdict matches the registered outcome",
    "verdict=ARBITRARY_REMOVAL_N_LE_6_CLOSED_"
    "NEXT_COUNT_7_UNRESOLVED" in output,
    "registered verdict",
)

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
        ("one_source_step_triangle_bound", "step"),
        ("exact_one_source_pulse_bound", "pulse"),
        ("common_step_coefficient", "common"),
        ("uniform_closed_source_count", "closed_count"),
        ("first_source_count_not_excluded", "first_open_count"),
        ("maximizing_removed_at_closed_count", "closed_removed"),
        ("maximizing_removed_at_first_open_count", "open_removed"),
        ("closed_count_history_bound", "closed_bound"),
        ("closed_count_margin", "closed_margin"),
        ("first_open_count_history_bound", "open_bound"),
        ("first_open_count_margin", "open_margin"),
        ("continuous_relaxation_at_closed_count", "relaxation"),
    ):
        label = f"L{volume}_{suffix}"
        observed = scalars.get(label, math.nan)
        expected = float(row[key])
        P.check(
            f"native/independent {label}",
            abs(observed - expected) <= 5.0e-15,
            f"native={observed:.17g} independent={expected:.17g}",
        )

required_scalars = {
    "uniform_closed_source_count": 6,
    "first_source_count_not_excluded": 7,
    "pulse_identity_checks": 8736,
    "gram_checks": 48,
    "proper_cubic_rotation_arms": 24,
    "prescribed_history_arms": 64,
    "native_unlocked_arms": 32,
    "total_arms": 96,
    "total_ticks": 12288,
    "genesis_events": 0,
    "analytic_contradiction_events": 0,
    "unlocked_cells_with_complete_removal": 4,
    "maximum_bound_excess": 0,
    "maximum_velocity": 0,
    "maximum_remainder": 0,
}
for key, expected in required_scalars.items():
    observed = scalars.get(key, math.nan)
    P.check(
        f"native gate {key}", observed == expected,
        f"observed={observed} expected={expected}",
    )

P.check(
    "native histories evolve a nonzero field",
    0.0 < scalars.get("maximum_observed_flux", 0.0) < K_GENESIS,
    str(scalars.get("maximum_observed_flux", math.nan)),
)

for key in (
    "maximum_pulse_identity_residual",
    "maximum_gram_residual",
    "maximum_translation_residual",
    "maximum_cubic_covariance_residual",
):
    observed = scalars.get(key, math.inf)
    P.check(f"native residual {key}", observed <= TOL, str(observed))

json_path = ROOT / "engine/results/ftd_0589/windows_msvc_cpu.json"
csv_path = ROOT / "engine/results/ftd_0589/windows_msvc_cpu.csv"
P.check("JSON run record exists", json_path.exists(), str(json_path))
P.check("CSV run record exists", csv_path.exists(), str(csv_path))
if json_path.exists():
    record = json.loads(json_path.read_text(encoding="utf-8"))
    P.check(
        "JSON identifier and preregistration lock",
        record.get("identifier") == "FTD-0589"
        and record.get("preregistration_sha256") == PREREG_SHA,
        str(record.get("verdict")),
    )
    P.check(
        "JSON structural result is valid",
        record.get("structural_valid") is True,
        str(record.get("structural_valid")),
    )
if csv_path.exists():
    with csv_path.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    P.check("CSV contains exactly 96 registered arms", len(rows) == 96,
            f"rows={len(rows)}")
    histories = {row["history"] for row in rows}
    P.check(
        "CSV contains all five fixed history classes",
        histories == {
            "permanent_step", "synchronous_pulse", "staggered_pulse",
            "paired_pulse", "native_unlocked",
        },
        str(sorted(histories)),
    )
    P.check(
        "every CSV arm is valid and genesis-free",
        all(row["valid"] == "1" and row["genesis_events"] == "0"
            for row in rows),
        "all rows",
    )

observer_source = (
    ROOT / "engine/src/eft/removal_time_pulse_bound.cpp"
).read_text(encoding="utf-8")
for forbidden in (
    "gauss_projection = true",
    "matched_gauss_dynamics = true",
    "forces = true",
    "movement = true",
    "dual_substrate = true",
    "common_action_face_dynamics",
):
    P.check(
        f"forbidden mechanism absent: {forbidden}",
        forbidden not in observer_source,
        forbidden,
    )
P.check(
    "source counts, volumes, schedules, and seeds are fixed",
    "for (int source_count : {5, 6})" in observer_source
    and "SPECTRAL_VOLUMES{{9, 17, 33, 65}}" in observer_source
    and "{8, 8, 16, 16, 24, 24}" in observer_source
    and "0x05890000u" in observer_source,
    "registered N={5,6}, L={9,17,33,65}, fixed histories",
)
P.check(
    "no production toggle is introduced",
    "term_toggles" not in observer_source,
    "observer-only implementation",
)

raise SystemExit(0 if P.report() else 1)
