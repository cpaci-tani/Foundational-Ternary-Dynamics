#!/usr/bin/env python3
"""
FTD-0097 deterministic look-elsewhere scan runner.

Implements docs/theory/07_assessment/PROTOCOL_LOOK_ELSEWHERE_SCAN.md
verbatim. Generates all monomials c · a_1 · a_2 · ... · a_d for
d ∈ {1,2,3,4}, c ∈ {-3,-2,-1,1,2,3}, atoms picked with repetition
from the 38-atom catalog. Reports hits against 20 dimensionless
physics targets at tolerances ε ∈ {1e-3, 1e-4, 1e-5, 1e-6}.

Determinism guarantees:
  * No random.seed, no np.random anywhere.
  * Iteration order over ATOMS, TARGETS, COEFFICIENT_INTEGERS uses
    the protocol-declared lexicographic / declared order (Python
    tuples, not sets).
  * itertools.combinations_with_replacement gives deterministic
    multiset enumeration.
  * All output files write data in iteration order.

Author isolation: per PROTOCOL §6(b), this runner enumerates ALL
hits at ε ≤ 1e-3, closing the post-hoc cherry-picking attack channel.

Output: engine/results/look_elsewhere_2026-04-27/
  hits_eps_1e-3.csv         — full hit list at ε ≤ 1e-3
  per_target_counts.json    — per-target hit counts at each ε
  residual_histogram.png    — log10|residual| distribution
  degree_scatter.png        — degree vs log10|residual|, per-target
  meta.json                 — run metadata + atom/target hashes

Usage: python tools/scan_look_elsewhere.py
       (No CLI args. Atom set, targets, polynomial form, tolerances
       are all FROZEN in PROTOCOL_LOOK_ELSEWHERE_SCAN.md.)
"""

from __future__ import annotations

import hashlib
import itertools
import json
import math
import os
import sys
from pathlib import Path
from typing import List, Tuple

import numpy as np


# ============================================================
# §1.1 ATOMS — locked in PROTOCOL_LOOK_ELSEWHERE_SCAN.md
# ============================================================
# Order matters for determinism. Tuples not sets.

# Constants at protocol-required precision
G_STAR = 2.95867511918863889  # Γ(1/4)/Γ(3/4), 30 digits per protocol §1.1
ALPHA = 1.0 / 137.035999084  # CODATA 2022 per protocol §1.1
PI = math.pi
E = math.e
SQRT_PI = math.sqrt(PI)
SQRT_2PI = math.sqrt(2.0 * PI)
INV_SQRT_3 = 1.0 / math.sqrt(3.0)  # c_lat per FTD axiom

# Build atom list in declared / lexicographic order
INTEGERS_LIST: List[Tuple[str, float]] = [
    ("1", 1.0),
    ("2", 2.0),
    ("3", 3.0),
    ("4", 4.0),
    ("5", 5.0),
    ("6", 6.0),
    ("7", 7.0),
    ("8", 8.0),
    ("9", 9.0),
    ("10", 10.0),
    ("11", 11.0),
    ("12", 12.0),
    ("13", 13.0),
    ("16", 16.0),
    ("17", 17.0),
    ("27", 27.0),
    ("47", 47.0),
    ("55", 55.0),
    ("59", 59.0),
    ("64", 64.0),
    ("141", 141.0),
]

G_POWERS_LIST: List[Tuple[str, float]] = [
    ("Gstar", G_STAR),
    ("Gstar^2", G_STAR ** 2),
    ("Gstar^3", G_STAR ** 3),
    ("1/Gstar", 1.0 / G_STAR),
    ("1/Gstar^2", 1.0 / (G_STAR ** 2)),
]

ALPHA_POWERS_LIST: List[Tuple[str, float]] = [
    ("alpha", ALPHA),
    ("alpha^2", ALPHA ** 2),
    ("alpha^11", ALPHA ** 11),
    ("alpha^20", ALPHA ** 20),
]

TRANSCENDENTALS_LIST: List[Tuple[str, float]] = [
    ("pi", PI),
    ("pi^2", PI ** 2),
    ("1/pi", 1.0 / PI),
    ("2pi", 2.0 * PI),
    ("sqrt(2pi)", SQRT_2PI),
    ("sqrt(pi)", SQRT_PI),
    ("e", E),
]

LATTICE_LIST: List[Tuple[str, float]] = [
    ("1/sqrt(3)", INV_SQRT_3),
]

ATOMS: Tuple[Tuple[str, float], ...] = tuple(
    INTEGERS_LIST + G_POWERS_LIST + ALPHA_POWERS_LIST + TRANSCENDENTALS_LIST + LATTICE_LIST
)
N_ATOMS = len(ATOMS)
assert N_ATOMS == 38, f"Expected 38 atoms per PROTOCOL §1.1, got {N_ATOMS}"


# ============================================================
# §1.2 Polynomial spec
# ============================================================
DEGREES: Tuple[int, ...] = (1, 2, 3, 4)
COEFFICIENT_INTEGERS: Tuple[int, ...] = (-3, -2, -1, 1, 2, 3)


# ============================================================
# §1.3 TARGETS — locked in PROTOCOL_LOOK_ELSEWHERE_SCAN.md
# ============================================================
# (name, value, experimental_uncertainty)
TARGETS: Tuple[Tuple[str, float, float], ...] = (
    ("alpha_inv",         137.035999084,    1.5e-10),
    ("m_e_in_MeV",        0.51099895069,    3e-10),
    ("m_p_over_m_e",      1836.15267343,    6e-11),
    ("m_n_over_m_e",      1838.68366173,    9e-10),
    ("m_mu_over_m_e",     206.7682830,      1.6e-8),
    ("m_tau_over_m_e",    3477.23,          5e-5),
    ("m_p_over_m_n",      0.99862347796,    7e-10),
    ("g_e_minus_2",       0.00231930437,    8e-13),
    ("a_mu",              0.0011659184,     6e-10),
    ("alpha_s_MZ",        0.1179,           1e-3),
    ("sin2_theta_W",      0.22290,          3e-5),
    ("Vud_squared",       0.94888,          5e-5),
    ("m_W_over_m_Z",      0.88147,          2e-5),
    ("m_b_over_m_c",      4.18,             0.05),
    ("m_t_over_v_higgs",  0.991,            0.001),
    ("Omega_b",           0.0493,           0.0006),
    ("Omega_dm",          0.265,            0.007),
    ("h_Hubble",          0.674,            0.005),
    ("Theta_13",          0.150,            0.001),
    ("delta_CP",          1.36,             0.17),
)
N_TARGETS = len(TARGETS)
assert N_TARGETS == 20, f"Expected 20 targets per PROTOCOL §1.3, got {N_TARGETS}"

# Diagnostic targets per PROTOCOL §3
DIAGNOSTIC_TARGETS = ("alpha_inv", "sin2_theta_W", "m_tau_over_m_e")
# Note: alpha · m_mu/m_e is composite, formed during scan from m_mu_over_m_e


# ============================================================
# §1.4 Tolerances
# ============================================================
TOLERANCES: Tuple[float, ...] = (1e-3, 1e-4, 1e-5, 1e-6)
HEADLINE_EPSILON = 1e-4


# ============================================================
# §4 Null hypothesis
# ============================================================
NULL_LAMBDA = 4.0  # E[total hits | null] across 20 targets
NULL_REJECT_LOW = 1   # ≤1 → selective (NULL REJECTED downward)
NULL_REJECT_HIGH = 11  # ≥11 → over-rich (NULL REJECTED upward)


# ============================================================
# Output paths
# ============================================================
REPO_ROOT = Path(__file__).resolve().parent.parent
OUTPUT_DIR = REPO_ROOT / "engine" / "results" / "look_elsewhere_2026-04-27"


# ============================================================
# Polynomial generation (deterministic)
# ============================================================
def generate_monomials() -> Tuple[np.ndarray, List[Tuple[int, str, int]]]:
    """
    Generate all monomials c · ∏ a_i for d ∈ DEGREES, atoms picked
    with repetition (multisets), c ∈ COEFFICIENT_INTEGERS.

    Returns:
        values:      np.array of monomial values in iteration order.
        descriptions: list of (degree, atom_string, coefficient)
                     parallel to values.
    """
    atom_indices = tuple(range(N_ATOMS))
    atom_values = np.array([a[1] for a in ATOMS])
    atom_names = [a[0] for a in ATOMS]

    all_values: List[float] = []
    all_descriptions: List[Tuple[int, str, int]] = []

    for d in DEGREES:
        # combinations_with_replacement gives deterministic multiset enumeration
        for tup in itertools.combinations_with_replacement(atom_indices, d):
            base_value = 1.0
            for idx in tup:
                base_value *= atom_values[idx]
            atom_str = " * ".join(atom_names[i] for i in tup)
            for c in COEFFICIENT_INTEGERS:
                all_values.append(c * base_value)
                all_descriptions.append((d, atom_str, c))

    return np.array(all_values, dtype=np.float64), all_descriptions


# ============================================================
# Hit detection
# ============================================================
def find_hits(
    values: np.ndarray,
    descriptions: List[Tuple[int, str, int]],
) -> Tuple[dict, List[dict]]:
    """
    For each target, count hits at each tolerance. Also enumerate
    every (target, polynomial, residual) triple with residual ≤ 1e-3
    for cherry-picking closure per PROTOCOL §6(b).

    Returns:
        per_target_counts: {target_name: {epsilon_str: count}}
        all_hits_1e3:      list of dicts (target_name, polynomial_string,
                                          polynomial_value, target_value,
                                          residual, degree, coefficient)
    """
    per_target_counts: dict = {}
    all_hits_1e3: List[dict] = []

    for tname, tval, _texp in TARGETS:
        residuals = np.abs(values - tval) / abs(tval)
        per_target_counts[tname] = {}
        for eps in TOLERANCES:
            per_target_counts[tname][f"{eps:.0e}"] = int(np.sum(residuals < eps))

        # Enumerate all hits at ε ≤ 1e-3
        hit_indices = np.where(residuals < 1e-3)[0]
        for idx in hit_indices:
            d, atom_str, c = descriptions[idx]
            all_hits_1e3.append({
                "target_name": tname,
                "target_value": tval,
                "polynomial_value": float(values[idx]),
                "polynomial_string": f"{c:+d} * {atom_str}",
                "residual": float(residuals[idx]),
                "degree": d,
                "coefficient": c,
            })

    return per_target_counts, all_hits_1e3


# ============================================================
# Output emission
# ============================================================
def emit_hits_csv(hits: List[dict], path: Path) -> None:
    with path.open("w", encoding="utf-8") as f:
        f.write("target_name,target_value,polynomial_value,polynomial_string,residual,degree,coefficient\n")
        for h in hits:
            # Polynomial string may contain commas? "*" only — safe for CSV
            f.write(
                f"{h['target_name']},"
                f"{h['target_value']:.15e},"
                f"{h['polynomial_value']:.15e},"
                f"\"{h['polynomial_string']}\","
                f"{h['residual']:.15e},"
                f"{h['degree']},"
                f"{h['coefficient']:+d}\n"
            )


def emit_per_target_counts_json(counts: dict, path: Path) -> None:
    # Preserve target iteration order
    ordered = {}
    for tname, tval, _ in TARGETS:
        ordered[tname] = counts[tname]
    with path.open("w", encoding="utf-8") as f:
        json.dump(ordered, f, indent=2, sort_keys=False)


def emit_residual_histogram(values: np.ndarray, path: Path) -> None:
    """Plot log10|residual| distribution across all (poly, target) pairs."""
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib not available; skipping histogram", file=sys.stderr)
        return

    log_residuals: List[float] = []
    for _, tval, _ in TARGETS:
        residuals = np.abs(values - tval) / abs(tval)
        log_residuals.extend(np.log10(np.maximum(residuals, 1e-30)).tolist())

    fig, ax = plt.subplots(figsize=(9, 5))
    ax.hist(log_residuals, bins=80, edgecolor="black", linewidth=0.3)
    ax.set_xlabel("log10(|polynomial - target| / |target|)")
    ax.set_ylabel("count")
    ax.set_title("FTD-0097 residual distribution (all (polynomial, target) pairs)")
    ax.axvline(np.log10(1e-3), color="r", linestyle="--", label="ε=1e-3")
    ax.axvline(np.log10(1e-4), color="orange", linestyle="--", label="ε=1e-4 (headline)")
    ax.axvline(np.log10(1e-5), color="g", linestyle="--", label="ε=1e-5")
    ax.legend()
    fig.tight_layout()
    fig.savefig(path, dpi=110)
    plt.close(fig)


def emit_degree_scatter(hits: List[dict], path: Path) -> None:
    """Plot degree vs log10|residual| with per-target color encoding."""
    if not hits:
        return
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib not available; skipping scatter", file=sys.stderr)
        return

    target_names = [t[0] for t in TARGETS]
    color_map = {name: plt.cm.tab20(i / len(target_names)) for i, name in enumerate(target_names)}

    fig, ax = plt.subplots(figsize=(11, 6))
    for tname in target_names:
        xs = [h["degree"] for h in hits if h["target_name"] == tname]
        ys = [math.log10(max(h["residual"], 1e-30)) for h in hits if h["target_name"] == tname]
        if xs:
            ax.scatter(xs, ys, c=[color_map[tname]], label=tname, s=22, alpha=0.6)
    ax.set_xlabel("polynomial degree")
    ax.set_ylabel("log10(residual)")
    ax.set_title("FTD-0097 hits at ε ≤ 1e-3: degree vs residual, per target")
    ax.legend(bbox_to_anchor=(1.02, 1.0), loc="upper left", fontsize=8)
    ax.set_xticks([1, 2, 3, 4])
    fig.tight_layout()
    fig.savefig(path, dpi=110)
    plt.close(fig)


def compute_atom_set_hash() -> str:
    """SHA256 of canonical atom set + target set descriptors. Used in meta.json."""
    h = hashlib.sha256()
    for name, val in ATOMS:
        h.update(f"{name}={val:.20e}\n".encode("utf-8"))
    for name, val, exp in TARGETS:
        h.update(f"{name}={val:.20e};exp={exp:.6e}\n".encode("utf-8"))
    h.update(f"DEGREES={DEGREES}\n".encode("utf-8"))
    h.update(f"COEFFS={COEFFICIENT_INTEGERS}\n".encode("utf-8"))
    h.update(f"TOLERANCES={TOLERANCES}\n".encode("utf-8"))
    return h.hexdigest()


def emit_meta(values: np.ndarray, hit_count: int, per_target_counts: dict, path: Path) -> None:
    total_hits_1e4 = sum(c[f"{HEADLINE_EPSILON:.0e}"] for c in per_target_counts.values())
    total_hits_1e3 = hit_count
    total_hits_1e5 = sum(c[f"{1e-5:.0e}"] for c in per_target_counts.values())
    total_hits_1e6 = sum(c[f"{1e-6:.0e}"] for c in per_target_counts.values())

    # Cluster pattern: which targets get the headline-tolerance hits?
    cluster_hits_per_target = {
        tname: per_target_counts[tname][f"{HEADLINE_EPSILON:.0e}"]
        for tname in (t[0] for t in TARGETS)
    }

    # Verdict per §7
    if total_hits_1e4 <= NULL_REJECT_LOW:
        verdict = "NULL REJECTED downward (selective)"
    elif total_hits_1e4 >= NULL_REJECT_HIGH:
        verdict = "NULL REJECTED upward (over-rich)"
    else:
        verdict = "NULL HOLDS (chance-level) — consult chi-squared on per-target uniformity"

    meta = {
        "campaign": "look_elsewhere_2026-04-27",
        "ledger_row": "FTD-0097",
        "protocol": "docs/theory/07_assessment/PROTOCOL_LOOK_ELSEWHERE_SCAN.md",
        "pre_reg_tag": "preregister-look-elsewhere-scan-v1",
        "n_atoms": N_ATOMS,
        "n_targets": N_TARGETS,
        "degrees": list(DEGREES),
        "coefficient_integers": list(COEFFICIENT_INTEGERS),
        "tolerances": list(TOLERANCES),
        "headline_epsilon": HEADLINE_EPSILON,
        "n_polynomials_total": int(values.size),
        "atom_set_hash_sha256": compute_atom_set_hash(),
        "total_hits": {
            "1e-3": int(total_hits_1e3),
            "1e-4": int(total_hits_1e4),
            "1e-5": int(total_hits_1e5),
            "1e-6": int(total_hits_1e6),
        },
        "null_lambda_total_1e-4": NULL_LAMBDA,
        "null_rejection_thresholds": {
            "selective_at_or_below": NULL_REJECT_LOW,
            "over_rich_at_or_above": NULL_REJECT_HIGH,
        },
        "headline_verdict": verdict,
        "per_target_hits_at_1e-4": cluster_hits_per_target,
        "diagnostic_targets": list(DIAGNOSTIC_TARGETS),
    }
    with path.open("w", encoding="utf-8") as f:
        json.dump(meta, f, indent=2, sort_keys=False)


# ============================================================
# Main
# ============================================================
def main() -> int:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    print(f"FTD-0097 Look-Elsewhere Scan — runner")
    print(f"  Atom catalog: {N_ATOMS} entries")
    print(f"  Target list:  {N_TARGETS} dimensionless physics ratios")
    print(f"  Degrees:      {DEGREES}")
    print(f"  Coefficients: {COEFFICIENT_INTEGERS}")
    print(f"  Tolerances:   {TOLERANCES}")
    print()

    print("Generating monomials...")
    values, descriptions = generate_monomials()
    print(f"  Total monomials: {len(values):,}")
    print(f"  Min value: {np.min(values):.6e}")
    print(f"  Max value: {np.max(values):.6e}")
    print()

    print("Scanning targets...")
    per_target_counts, all_hits_1e3 = find_hits(values, descriptions)
    print(f"  Total hits at ε ≤ 1e-3: {len(all_hits_1e3):,}")
    print()

    # Total hits at headline ε
    total_1e4 = sum(c[f"{HEADLINE_EPSILON:.0e}"] for c in per_target_counts.values())
    print(f"Headline result (ε = 1e-4):")
    print(f"  Total hits across 20 targets: {total_1e4}")
    print(f"  Null Poisson λ:               {NULL_LAMBDA}")
    if total_1e4 <= NULL_REJECT_LOW:
        print(f"  Verdict: NULL REJECTED downward (selective)")
    elif total_1e4 >= NULL_REJECT_HIGH:
        print(f"  Verdict: NULL REJECTED upward (over-rich)")
    else:
        print(f"  Verdict: NULL HOLDS (consult per-target chi-squared)")
    print()

    print("Per-target hits at ε = 1e-4:")
    for tname, tval, _ in TARGETS:
        n = per_target_counts[tname][f"{HEADLINE_EPSILON:.0e}"]
        marker = "  *DIAGNOSTIC*" if tname in DIAGNOSTIC_TARGETS else ""
        print(f"  {tname:25s}: {n}{marker}")
    print()

    print(f"Writing artifacts to {OUTPUT_DIR}/...")
    emit_hits_csv(all_hits_1e3, OUTPUT_DIR / "hits_eps_1e-3.csv")
    emit_per_target_counts_json(per_target_counts, OUTPUT_DIR / "per_target_counts.json")
    emit_residual_histogram(values, OUTPUT_DIR / "residual_histogram.png")
    emit_degree_scatter(all_hits_1e3, OUTPUT_DIR / "degree_scatter.png")
    emit_meta(values, len(all_hits_1e3), per_target_counts, OUTPUT_DIR / "meta.json")
    print("Done.")

    return 0


if __name__ == "__main__":
    sys.exit(main())
