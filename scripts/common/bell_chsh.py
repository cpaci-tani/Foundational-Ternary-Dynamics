"""
bell_chsh.py — Shared Bell/CHSH helpers for ftd_cern_*.py and bell_*.py.

Extracted from:
  scripts/experiments/bell_lattice_test.py    (compute_chsh_from_correlations,
                                                compute_correlation,
                                                random_unit_vectors,
                                                angle_to_axis)
  scripts/experiments/sloop_bell_experiment.py (compute_chsh,
                                                compute_correlation,
                                                random_unit_vectors,
                                                angle_to_axis)

The two scripts had byte-for-byte identical logic with two cosmetic diffs:
  - sloop named it `compute_chsh`; bell_lattice named it
    `compute_chsh_from_correlations`. We export BOTH as aliases.
  - sloop's angle_to_axis uses np.float32; bell_lattice uses default float64.
    We take dtype as a keyword argument, defaulting to float64, with the
    ability to opt into float32 at the call site.

Nothing here alters scientific logic.
"""
from __future__ import annotations

import numpy as np

# Canonical CHSH measurement angles used by both scripts.
CHSH_ANGLES: dict[str, float] = {
    "a1": 0.0,
    "a2": np.pi / 2,
    "b1": np.pi / 4,
    "b2": 3 * np.pi / 4,
}


def compute_chsh(E11: float, E12: float, E21: float, E22: float) -> float:
    """
    CHSH S-parameter from four correlation values.

        S = |E(a1,b1) - E(a1,b2)| + |E(a2,b1) + E(a2,b2)|

    Classical bound:  S <= 2
    Tsirelson bound:  S <= 2*sqrt(2) ~= 2.828
    """
    return abs(E11 - E12) + abs(E21 + E22)


# Alias matching the older bell_lattice_test.py name.
compute_chsh_from_correlations = compute_chsh


def compute_correlation(
    outcomes_A: np.ndarray, outcomes_B: np.ndarray
) -> tuple[float, float]:
    """
    Correlation E(a,b) = <A*B> from outcome arrays in {-1, 0, +1}.

    Null outcomes (0) are excluded; efficiency is the fraction of non-null
    pairs. Returns (correlation, detection_efficiency).

    If no valid pairs exist, returns (0.0, 0.0).
    """
    valid = (outcomes_A != 0) & (outcomes_B != 0)
    n_valid = int(np.sum(valid))
    if n_valid == 0:
        return 0.0, 0.0
    efficiency = n_valid / len(outcomes_A)
    correlation = np.mean(outcomes_A[valid] * outcomes_B[valid])
    return float(correlation), float(efficiency)


def random_unit_vectors(n: int, rng: np.random.Generator | None = None) -> np.ndarray:
    """
    Generate `n` unit vectors uniformly distributed on S^2 (Marsaglia).

    Returns an (n, 3) array. If `rng` is None, uses the default
    numpy global generator (preserving the original scripts' behavior).
    """
    if rng is None:
        z = np.random.uniform(-1, 1, n)
        phi = np.random.uniform(0, 2 * np.pi, n)
    else:
        z = rng.uniform(-1, 1, n)
        phi = rng.uniform(0, 2 * np.pi, n)
    r = np.sqrt(1 - z * z)
    x = r * np.cos(phi)
    y = r * np.sin(phi)
    return np.column_stack([x, y, z])


def angle_to_axis(theta: float, dtype: np.dtype | type = np.float64) -> np.ndarray:
    """
    Measurement angle -> unit vector in the x-z plane.

    `dtype` defaults to float64 (bell_lattice_test.py convention); pass
    np.float32 to match sloop_bell_experiment.py.
    """
    return np.array([np.sin(theta), 0.0, np.cos(theta)], dtype=dtype)
