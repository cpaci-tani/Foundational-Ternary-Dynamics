"""
explore_ternary_matrix_iteration.py — pre-registered ternary-matrix BCC-snap test.

Hash-locked under git tag: preregister-ternary-matrix-bcc-snap-v1
Pre-registration manifest: docs/theory/09_mathematical/PREREG_TERNARY_MATRIX_BCC_SNAP_v1.md

Tests whether iterates of D_T = A + B * Theta (a 3x3 matrix family proposed
in a 2026-05-23 user synthesis) converge under normalized power iteration to
a BCC primitive direction (+/-1, +/-1, +/-1) / sqrt(3) on the unit sphere.

The construction, sweep grid, falsifiable prediction, and outcome-to-tag
mapping are all FROZEN in the manifest. This script is the mechanical
realization. It must not be edited at the locked tag.

Carrier: R^3, mpmath.mpf at 50-digit precision.
Iteration: v_{k+1} = (A + B * Theta) v_k / || ... ||_2
Termination: ||v_{k+1} - v_k|| < 1e-12 or 500 steps.
"""

from __future__ import annotations

import csv
import hashlib
import json
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import List, Tuple

import numpy as np
from mpmath import mp, mpf, mpc, sqrt as mp_sqrt, gamma as mp_gamma, pi as mp_pi

mp.dps = 50

REPO_ROOT = Path(__file__).resolve().parents[2]
RESULTS_DIR = REPO_ROOT / "scripts" / "exploration" / "results"
RESULTS_DIR.mkdir(parents=True, exist_ok=True)


def G_star() -> mpf:
    """G* = Gamma(1/4) / Gamma(3/4), the lemniscatic Gamma ratio. 2.9586751..."""
    return mp_gamma(mpf("0.25")) / mp_gamma(mpf("0.75"))


def varpi() -> mpf:
    """Lemniscate constant pi-bar = Gamma(1/4)^2 / (2*sqrt(2)*sqrt(pi)). 2.6220575..."""
    g14 = mp_gamma(mpf("0.25"))
    return g14 * g14 / (mpf(2) * mp_sqrt(mpf(2)) * mp_sqrt(mp_pi))


def mp_matrix_3x3(entries: List[List[mpf]]) -> List[List[mpf]]:
    """Hold a 3x3 matrix as a list of rows for mpmath compatibility."""
    return [[mpf(x) if not isinstance(x, mpf) else x for x in row] for row in entries]


def mp_matvec_3(M: List[List[mpf]], v: List[mpf]) -> List[mpf]:
    return [
        M[0][0] * v[0] + M[0][1] * v[1] + M[0][2] * v[2],
        M[1][0] * v[0] + M[1][1] * v[1] + M[1][2] * v[2],
        M[2][0] * v[0] + M[2][1] * v[1] + M[2][2] * v[2],
    ]


def mp_norm2(v: List[mpf]) -> mpf:
    return mp_sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])


def mp_matmul_3(M1: List[List[mpf]], M2: List[List[mpf]]) -> List[List[mpf]]:
    return [
        [sum(M1[i][k] * M2[k][j] for k in range(3)) for j in range(3)]
        for i in range(3)
    ]


def mp_matadd_3(M1: List[List[mpf]], M2: List[List[mpf]]) -> List[List[mpf]]:
    return [[M1[i][j] + M2[i][j] for j in range(3)] for i in range(3)]


def normalize_3(v: List[mpf]) -> List[mpf]:
    n = mp_norm2(v)
    return [v[0] / n, v[1] / n, v[2] / n]


def vec_diff_norm(u: List[mpf], v: List[mpf]) -> mpf:
    return mp_norm2([u[0] - v[0], u[1] - v[1], u[2] - v[2]])


# ----- BCC and axis direction definitions -----

def bcc_directions() -> List[List[mpf]]:
    """The 8 BCC primitive directions (s1, s2, s3) / sqrt(3), s_i in {-1, +1}."""
    inv_sqrt3 = mpf(1) / mp_sqrt(mpf(3))
    out = []
    for s1 in (-1, 1):
        for s2 in (-1, 1):
            for s3 in (-1, 1):
                out.append([mpf(s1) * inv_sqrt3, mpf(s2) * inv_sqrt3, mpf(s3) * inv_sqrt3])
    return out


def axis_directions() -> List[List[mpf]]:
    """The 6 axis directions +-e_i."""
    out = []
    for i in range(3):
        for s in (-1, 1):
            v = [mpf(0), mpf(0), mpf(0)]
            v[i] = mpf(s)
            out.append(v)
    return out


def min_distance_to_set(v: List[mpf], target_set: List[List[mpf]]) -> mpf:
    """Min ||v - t|| over t in target_set, with antipodal identification (also try -v)."""
    best = mpf("inf")
    minus_v = [-v[0], -v[1], -v[2]]
    for t in target_set:
        d1 = vec_diff_norm(v, t)
        d2 = vec_diff_norm(minus_v, t)
        d = d1 if d1 < d2 else d2
        if d < best:
            best = d
    return best


# ----- A and B candidate constructions -----

def A_form_1() -> List[List[mpf]]:
    """A1 = diag(G*, G*^2, G*^3)."""
    g = G_star()
    return mp_matrix_3x3([
        [g, mpf(0), mpf(0)],
        [mpf(0), g * g, mpf(0)],
        [mpf(0), mpf(0), g * g * g],
    ])


def A_form_2() -> List[List[mpf]]:
    """A2 = diag(G*, varpi, pi)."""
    g = G_star()
    w = varpi()
    return mp_matrix_3x3([
        [g, mpf(0), mpf(0)],
        [mpf(0), w, mpf(0)],
        [mpf(0), mpf(0), mp_pi],
    ])


def Theta_matrix() -> List[List[mpf]]:
    """Theta = diag(1, 2, 3) -- the discrete Euler/theta operator."""
    return mp_matrix_3x3([
        [mpf(1), mpf(0), mpf(0)],
        [mpf(0), mpf(2), mpf(0)],
        [mpf(0), mpf(0), mpf(3)],
    ])


def B_form_1() -> List[List[mpf]]:
    """B1 = [[0,1,1],[1,0,1],[1,1,0]] -- all-positive symmetric."""
    return mp_matrix_3x3([
        [mpf(0), mpf(1), mpf(1)],
        [mpf(1), mpf(0), mpf(1)],
        [mpf(1), mpf(1), mpf(0)],
    ])


def B_form_2() -> List[List[mpf]]:
    """B2 = [[0,1,1],[-1,0,1],[1,-1,0]] -- asymmetric BCC sign."""
    return mp_matrix_3x3([
        [mpf(0), mpf(1), mpf(1)],
        [mpf(-1), mpf(0), mpf(1)],
        [mpf(1), mpf(-1), mpf(0)],
    ])


def B_form_3() -> List[List[mpf]]:
    """B3 = [[0,1,-1],[-1,0,1],[1,-1,0]] -- cyclic antisymmetric.

    This is the matrix of v -> (1,1,1)/sqrt(3) x v; kernel is (1,1,1)/sqrt(3) itself."""
    return mp_matrix_3x3([
        [mpf(0), mpf(1), mpf(-1)],
        [mpf(-1), mpf(0), mpf(1)],
        [mpf(1), mpf(-1), mpf(0)],
    ])


def B_form_4() -> List[List[mpf]]:
    """B4 = [[0,1,1],[-1,0,1],[-1,-1,0]] -- diagonal-Toeplitz sign(i-j)."""
    return mp_matrix_3x3([
        [mpf(0), mpf(1), mpf(1)],
        [mpf(-1), mpf(0), mpf(1)],
        [mpf(-1), mpf(-1), mpf(0)],
    ])


def B_zero() -> List[List[mpf]]:
    """Off-diagonals zeroed -- control 1."""
    return mp_matrix_3x3([[mpf(0)] * 3 for _ in range(3)])


# ----- Seed generation (FROZEN: numpy default_rng seed=42 then normalize) -----

def primary_seeds() -> List[List[mpf]]:
    """5 random unit vectors from seed=42 Normal(0,1)^3 then normalized."""
    rng = np.random.default_rng(seed=42)
    raw = rng.normal(size=(5, 3))  # 5 x 3 array, deterministic
    seeds = []
    for row in raw:
        v = [mpf(float(row[0])), mpf(float(row[1])), mpf(float(row[2]))]
        seeds.append(normalize_3(v))
    return seeds


def random_B_matrices(n_instances: int, rng_seed: int = 12345) -> List[List[List[mpf]]]:
    """Generate n_instances random B matrices with entries in {-1, 0, +1}, zero diagonal."""
    rng = np.random.default_rng(seed=rng_seed)
    out = []
    for _ in range(n_instances):
        # 6 off-diagonal entries chosen uniformly from {-1, 0, +1}
        entries = rng.choice([-1, 0, 1], size=6)
        B = [
            [mpf(0), mpf(int(entries[0])), mpf(int(entries[1]))],
            [mpf(int(entries[2])), mpf(0), mpf(int(entries[3]))],
            [mpf(int(entries[4])), mpf(int(entries[5])), mpf(0)],
        ]
        out.append(B)
    return out


# ----- The iteration -----

@dataclass
class IterationResult:
    label: str
    A_form: str
    B_form: str
    seed_idx: int
    converged: bool
    n_steps: int
    d_BCC: float       # converted from mpf to float for CSV
    d_axis: float
    v_inf: Tuple[float, float, float]
    final_step_norm: float


def power_iterate(
    A: List[List[mpf]],
    B: List[List[mpf]],
    Theta: List[List[mpf]],
    v0: List[mpf],
    max_steps: int = 500,
    tol: mpf = mpf("1e-12"),
) -> Tuple[bool, int, List[mpf], mpf]:
    """Run normalized power iteration on D_T = A + B*Theta starting from v0."""
    BTheta = mp_matmul_3(B, Theta)
    DT = mp_matadd_3(A, BTheta)
    v = list(v0)
    final_step = mpf(0)
    for step in range(1, max_steps + 1):
        Mv = mp_matvec_3(DT, v)
        n = mp_norm2(Mv)
        if n == 0:
            # collapsed; not converged
            return False, step, v, mpf("inf")
        v_new = [Mv[0] / n, Mv[1] / n, Mv[2] / n]
        final_step = min(vec_diff_norm(v_new, v), vec_diff_norm([-v_new[0], -v_new[1], -v_new[2]], v))
        if final_step < tol:
            return True, step, v_new, final_step
        v = v_new
    return False, max_steps, v, final_step


def run_one(
    label: str,
    A_label: str,
    B_label: str,
    A: List[List[mpf]],
    B: List[List[mpf]],
    Theta: List[List[mpf]],
    seed_idx: int,
    seed_vec: List[mpf],
) -> IterationResult:
    converged, n_steps, v_inf, final_step = power_iterate(A, B, Theta, seed_vec)
    bcc_set = bcc_directions()
    axis_set = axis_directions()
    d_bcc = min_distance_to_set(v_inf, bcc_set)
    d_axis = min_distance_to_set(v_inf, axis_set)
    return IterationResult(
        label=label,
        A_form=A_label,
        B_form=B_label,
        seed_idx=seed_idx,
        converged=converged,
        n_steps=n_steps,
        d_BCC=float(d_bcc),
        d_axis=float(d_axis),
        v_inf=(float(v_inf[0]), float(v_inf[1]), float(v_inf[2])),
        final_step_norm=float(final_step),
    )


# ----- Sweep runner -----

def main():
    seeds = primary_seeds()
    Theta = Theta_matrix()
    A_options = [("A1_Gstar_powers", A_form_1()), ("A2_Gstar_varpi_pi", A_form_2())]
    B_natural = [
        ("B1_pos_sym", B_form_1()),
        ("B2_asym_BCC", B_form_2()),
        ("B3_cyclic_antisym", B_form_3()),
        ("B4_toeplitz_sign", B_form_4()),
    ]
    B_zero_M = B_zero()
    B_random = random_B_matrices(n_instances=10, rng_seed=12345)

    results: List[IterationResult] = []

    # Primary sweep: 2 A x 4 B x 5 seeds = 40
    for A_label, A in A_options:
        for B_label, B in B_natural:
            for seed_idx, seed_vec in enumerate(seeds):
                results.append(run_one("primary", A_label, B_label, A, B, Theta, seed_idx, seed_vec))

    # Control 1: B = 0
    for A_label, A in A_options:
        for seed_idx, seed_vec in enumerate(seeds):
            results.append(run_one("control_Bzero", A_label, "B_zero", A, B_zero_M, Theta, seed_idx, seed_vec))

    # Control 2: random B (10 instances per A, 5 seeds each = 100)
    for A_label, A in A_options:
        for r_idx, B in enumerate(B_random):
            B_lab = f"B_random_{r_idx:02d}"
            for seed_idx, seed_vec in enumerate(seeds):
                results.append(run_one("control_Brandom", A_label, B_lab, A, B, Theta, seed_idx, seed_vec))

    # ---- write CSV ----
    csv_path = RESULTS_DIR / "ternary_matrix_iteration_2026-05-23.csv"
    with csv_path.open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow([
            "label", "A_form", "B_form", "seed_idx", "converged", "n_steps",
            "d_BCC", "d_axis", "v_inf_x", "v_inf_y", "v_inf_z", "final_step_norm",
        ])
        for r in results:
            w.writerow([
                r.label, r.A_form, r.B_form, r.seed_idx, int(r.converged), r.n_steps,
                f"{r.d_BCC:.6e}", f"{r.d_axis:.6e}",
                f"{r.v_inf[0]:.10f}", f"{r.v_inf[1]:.10f}", f"{r.v_inf[2]:.10f}",
                f"{r.final_step_norm:.6e}",
            ])

    # ---- outcome interpretation per manifest §4.3 ----
    BCC_THRESHOLD = 1e-6
    BCC_THRESHOLD_LOOSE = 1e-3
    RANDOM_RATE_CEILING = 0.10  # 10%

    primary = [r for r in results if r.label == "primary"]
    bzero = [r for r in results if r.label == "control_Bzero"]
    brandom = [r for r in results if r.label == "control_Brandom"]

    # P1 check: for each (A, B) pair from primary, do all 5 seeds satisfy d_BCC < 1e-6 and d_BCC < d_axis?
    p1_passing_AB = []
    by_AB = {}
    for r in primary:
        by_AB.setdefault((r.A_form, r.B_form), []).append(r)
    for (A_label, B_label), runs in by_AB.items():
        all_snap = all(r.converged and r.d_BCC < BCC_THRESHOLD and r.d_BCC < r.d_axis for r in runs)
        if all_snap:
            p1_passing_AB.append((A_label, B_label))

    # B=0 control: any snap?
    bzero_snap = sum(1 for r in bzero if r.d_BCC < BCC_THRESHOLD_LOOSE)
    # B-random: rate of d_BCC < 1e-6
    brandom_snap = sum(1 for r in brandom if r.d_BCC < BCC_THRESHOLD)
    brandom_rate = brandom_snap / len(brandom) if brandom else 0

    # Determine outcome
    all_converged_primary = all(r.converged for r in primary)
    if p1_passing_AB and bzero_snap == 0 and brandom_rate < RANDOM_RATE_CEILING:
        outcome = "A"
        outcome_text = (
            "BCC-snap on natural (A, B) pairs, ABSENT in B=0 control, RARE in random-B controls. "
            "[NUMERICAL FACT] + [OBSERVATION] -- the structural claim survives this test."
        )
    elif p1_passing_AB and brandom_rate >= RANDOM_RATE_CEILING:
        outcome = "C"
        outcome_text = (
            f"BCC-snap on natural (A, B) but random-B controls show BCC-snap at "
            f"{brandom_rate:.1%} -- the snap is generic, NOT BCC-specific. [CLOSED NEGATIVE] on BCC-specificity."
        )
    elif not p1_passing_AB and all_converged_primary:
        outcome = "B"
        outcome_text = (
            "Iterates converge but NOT to BCC directions. [CLOSED NEGATIVE] on BCC-snap; the actual "
            "attractor structure is reported in the CSV."
        )
    else:
        outcome = "D"
        outcome_text = (
            "Iteration fails to converge under at least one natural (A, B). [CLOSED NEGATIVE] on the "
            "construction as stated; would need a different iteration rule."
        )

    # ---- write MD interpretation ----
    md_path = RESULTS_DIR / "ternary_matrix_iteration_2026-05-23.md"
    with md_path.open("w", encoding="utf-8") as f:
        f.write("# Ternary-Matrix BCC-Snap Test -- Results\n\n")
        f.write("**Date:** 2026-05-23\n")
        f.write("**Pre-registration:** docs/theory/09_mathematical/PREREG_TERNARY_MATRIX_BCC_SNAP_v1.md\n")
        f.write("**Git tag:** preregister-ternary-matrix-bcc-snap-v1\n\n")
        f.write("---\n\n## Outcome\n\n")
        f.write(f"**Outcome {outcome}.** {outcome_text}\n\n")
        f.write("---\n\n## Summary statistics\n\n")
        f.write(f"- Primary sweep runs: {len(primary)} (2 A x 4 B x 5 seeds)\n")
        f.write(f"- B=0 control runs: {len(bzero)} (2 A x 5 seeds)\n")
        f.write(f"- Random-B control runs: {len(brandom)} (2 A x 10 random B x 5 seeds)\n\n")
        f.write(f"- (A, B) pairs satisfying P1 (all 5 seeds snap to BCC at threshold {BCC_THRESHOLD:g}): "
                f"**{len(p1_passing_AB)}** out of 8\n")
        for ab in p1_passing_AB:
            f.write(f"  - {ab[0]} x {ab[1]}\n")
        f.write(f"- B=0 control snaps (threshold {BCC_THRESHOLD_LOOSE:g}, loose): "
                f"{bzero_snap} out of {len(bzero)}\n")
        f.write(f"- Random-B BCC-snap rate (threshold {BCC_THRESHOLD:g}): "
                f"{brandom_snap} / {len(brandom)} = {brandom_rate:.1%}\n\n")
        f.write("---\n\n## Per-(A, B) primary-sweep summary\n\n")
        f.write("| A | B | seeds converged | mean d_BCC | mean d_axis | snap rate (d_BCC < 1e-6) |\n")
        f.write("|---|---|---|---|---|---|\n")
        for (A_label, B_label), runs in sorted(by_AB.items()):
            n_conv = sum(1 for r in runs if r.converged)
            mean_bcc = sum(r.d_BCC for r in runs) / len(runs)
            mean_axis = sum(r.d_axis for r in runs) / len(runs)
            snap_count = sum(1 for r in runs if r.d_BCC < BCC_THRESHOLD)
            f.write(f"| {A_label} | {B_label} | {n_conv}/{len(runs)} | "
                    f"{mean_bcc:.3e} | {mean_axis:.3e} | {snap_count}/{len(runs)} |\n")
        f.write("\n---\n\n## Full data\n\n")
        f.write(f"See `ternary_matrix_iteration_2026-05-23.csv` ({len(results)} rows).\n")

    print(f"Outcome: {outcome}")
    print(outcome_text)
    print(f"\nCSV: {csv_path}")
    print(f"MD:  {md_path}")
    print(f"\nP1-passing (A, B) pairs: {p1_passing_AB}")
    print(f"B=0 snaps (loose):  {bzero_snap} / {len(bzero)}")
    print(f"Random-B snap rate: {brandom_snap} / {len(brandom)} = {brandom_rate:.1%}")


if __name__ == "__main__":
    main()
