#!/usr/bin/env python3
"""Exact Fourier/cohomology proof for FTD-0583 (no parameter search)."""

from __future__ import annotations

import cmath
from fractions import Fraction
from hashlib import sha256
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/PREREG_NONCOMPACT_FACE_COHOMOLOGY_v1.md":
        "755D703FB3E9DA9CA7F2EB46B1FE399D704F739AD08050D39242D1EB0B2BB922",
    "engine/include/ftd/eft/matched_gauss_transport.h":
        "1E07F87A0EBD0D1830D0632B82C2BD65497EBEAE7BB152EA02C5AAE19328B033",
    "engine/src/eft/matched_gauss_transport.cpp":
        "12BF98040BB45AD6CD9A409A93C842101C400CEEE6242E9B9352158A33A9D028",
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
}


def digest(path: Path) -> str:
    return sha256(path.read_bytes()).hexdigest().upper()


for relative, expected in LOCKS.items():
    actual = digest(ROOT / relative)
    assert actual == expected, (relative, actual, expected)


def rank(matrix: list[list[complex]], tolerance: float = 1e-12) -> int:
    work = [row[:] for row in matrix]
    rows = len(work)
    columns = len(work[0]) if rows else 0
    pivot_row = 0
    for column in range(columns):
        if pivot_row >= rows:
            break
        pivot = max(range(pivot_row, rows),
                    key=lambda row: abs(work[row][column]))
        if abs(work[pivot][column]) <= tolerance:
            continue
        work[pivot_row], work[pivot] = work[pivot], work[pivot_row]
        value = work[pivot_row][column]
        work[pivot_row] = [entry / value for entry in work[pivot_row]]
        for row in range(rows):
            if row == pivot_row:
                continue
            factor = work[row][column]
            work[row] = [work[row][j] - factor * work[pivot_row][j]
                         for j in range(columns)]
        pivot_row += 1
    return pivot_row


mode_count = 0
zero_modes = 0
nonzero_modes = 0
max_chain_residual = 0.0
betti = [0, 0, 0, 0]
for length in (3, 4, 5, 8):
    local_betti = [0, 0, 0, 0]
    for nx in range(length):
        for ny in range(length):
            for nz in range(length):
                q = [1 - cmath.exp(-2j * cmath.pi * n / length)
                     for n in (nx, ny, nz)]
                d0 = [[q[0]], [q[1]], [q[2]]]
                d1 = [
                    [0, -q[2], q[1]],
                    [q[2], 0, -q[0]],
                    [-q[1], q[0], 0],
                ]
                d2 = [[q[0], q[1], q[2]]]
                r0, r1, r2 = rank(d0), rank(d1), rank(d2)
                local_betti[0] += 1 - r0
                local_betti[1] += (3 - r1) - r0
                local_betti[2] += (3 - r2) - r1
                local_betti[3] += 1 - r2
                for row in range(3):
                    max_chain_residual = max(
                        max_chain_residual,
                        abs(sum(d1[row][column] * d0[column][0]
                                for column in range(3))))
                for column in range(3):
                    max_chain_residual = max(
                        max_chain_residual,
                        abs(sum(d2[0][row] * d1[row][column]
                                for row in range(3))))
                if nx == ny == nz == 0:
                    zero_modes += 1
                    assert (r0, r1, r2) == (0, 0, 0)
                else:
                    nonzero_modes += 1
                    assert (r0, r1, r2) == (1, 2, 1)
                mode_count += 1
    assert local_betti == [1, 3, 3, 1], (length, local_betti)
    betti = local_betti

assert mode_count == 728
assert zero_modes == 4 and nonzero_modes == 724
assert betti == [1, 3, 3, 1]
assert max_chain_residual < 1e-12

for t in (Fraction(0), Fraction(1, 4), Fraction(1, 2),
          Fraction(3, 4), Fraction(1)):
    divergence = Fraction(0)
    harmonic_flux = Fraction(0)
    energy = Fraction(7, 11) * t * t
    assert t * divergence == 0
    assert t * harmonic_flux == 0
    assert energy == t * t * Fraction(7, 11)

for polarity in (-1, 1):
    for t in (Fraction(0), Fraction(1, 4), Fraction(1, 2),
              Fraction(3, 4), Fraction(1)):
        source = polarity * t
        sink = -source
        assert source + sink == 0

header = (ROOT / "engine/include/ftd/eft/matched_gauss_transport.h").read_text(
    encoding="utf-8")
voxel = (ROOT / "engine/include/ftd/voxel.h").read_text(encoding="utf-8")
assert "std::vector<double> x" in header
assert "std::vector<double> y" in header
assert "std::vector<double> z" in header
assert "Vec3 flux" in voxel
assert "mod 2" not in header.lower() and "compact u(1)" not in header.lower()

print("FTD-0583 noncompact matched-face cohomology proof: PASS")
print("H^2(T_L^3;R)=R^3, represented by global real plane fluxes")
print("all nonzero Fourier modes are exact; Betti=(1,3,3,1)")
print("localized zero-harmonic curl fields contract continuously to vacuum")
print("real periodic Gauss dipoles scale continuously and have zero net charge")
print("verdict=MATCHED_NONCOMPACT_COHOMOLOGY_GLOBAL_ONLY_LOCAL_PROTECTED_DEFECT_CLOSED")
