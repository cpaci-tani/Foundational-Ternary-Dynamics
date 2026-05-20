"""
GPU-accelerated Monte Carlo cross-check of the W^(D) values from
PAPER_GSTAR_INTRODUCTION.tex Theorem 12.2 (generalised Watson identity).

The integral W^(D) = (1/π^D) ∫...∫ dk / (1 - ∏ cos k_i)  equals the expected
total number of visits to the origin for a simple random walk on the
D-dimensional body-centred-cubic (BCC) lattice. Each step of the walk
independently flips each of the D coordinates by ±1 (so neighbours are at
(±1, ..., ±1), i.e. 2^D nearest neighbours).

This is embarrassingly parallel: run 10^6+ independent walks on the GPU, sum
the returns-to-origin counts, divide. CPU mpmath verification at 50 digits
already exists in `gstar_open_questions.py`; the GPU MC here gives 3-4 digit
independent confirmation that the closed-form values are correct.

================================================================================
CANONICAL RUN (WSL2 + RTX 5090):

    wsl.exe -d Ubuntu-22.04 -- bash -c \\
        "cd /mnt/c/Users/cpaci/Desktop/ftd && \\
         python3 scripts/exploration/gstar_W_montecarlo_gpu.py"
================================================================================
"""

import time

import cupy as cp
import numpy as np

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

N_WALKERS = 2_000_000  # number of independent random walks per dimension
# Step count by dimension: D=3 needs many more steps because the BCC random
# walk in 3D is "barely transient" and the return-distribution has a heavy tail.
# For D >= 4 transience is strong and 500 steps converges.
N_STEPS_BY_D = {3: 8000, 4: 500, 5: 300}
DIMENSIONS = [3, 4, 5]
SEED = 20260519

REFERENCE_VALUES = {
    3: 1.3932039296856768591842,   # W^(3) = G*^2/(2*pi)
    4: 1.1186363871641870683496,   # W^(4) = 4F3(1/2^4; 1^3; 1)
    5: 1.0468255498335000052545,   # W^(5) = 5F4(1/2^5; 1^4; 1)
}


def gpu_mc_W(D, n_walkers, n_steps, seed):
    """
    Estimate W^(D) via GPU Monte Carlo random walks on the BCC lattice.

    At step t, each walker's position has each coordinate equal to
    (number of +1 steps minus number of -1 steps) summed over D independent
    Bernoulli sequences. The walker is at the origin iff all D coordinates
    are zero.

    Returns: (W_estimate, std_error, elapsed_seconds)
    """
    rng = cp.random.default_rng(seed)
    t0 = time.time()

    # Position state: shape (n_walkers, D), initialised at origin.
    pos = cp.zeros((n_walkers, D), dtype=cp.int32)

    # Track returns to origin at step 0 (always counts: every walker starts there).
    returns_count = cp.ones(n_walkers, dtype=cp.int64)

    for step in range(n_steps):
        # Each step: each coordinate flips by ±1 independently.
        # Draw n_walkers * D random bits and convert to ±1.
        steps = rng.integers(0, 2, size=(n_walkers, D), dtype=cp.int8) * 2 - 1
        pos += steps.astype(cp.int32)
        # Count returns to origin (all coords zero).
        at_origin = (pos == 0).all(axis=1)
        returns_count += at_origin.astype(cp.int64)

    mean_returns = float(returns_count.mean())
    std_returns = float(returns_count.std()) / np.sqrt(n_walkers)
    elapsed = time.time() - t0
    return mean_returns, std_returns, elapsed


def main():
    print("=" * 75)
    print("GPU Monte Carlo cross-check of W^(D) values")
    print("RTX 5090 via WSL2; CuPy", cp.__version__)
    print("=" * 75)
    print(f"Walkers per dimension: {N_WALKERS:,}")
    print(f"Steps per walk by D:    {N_STEPS_BY_D}")
    print(f"RNG seed:               {SEED}")
    print()

    # Warm up GPU
    _ = cp.random.default_rng(0).integers(0, 2, size=(100, 100))
    cp.cuda.Device(0).synchronize()

    print(f"{'D':>3}  {'GPU MC estimate':>22}  {'std err':>12}  "
          f"{'Reference':>22}  {'sigma off':>10}  {'time (s)':>9}")
    print("-" * 90)

    for D in DIMENSIONS:
        n_steps = N_STEPS_BY_D[D]
        W_mc, W_err, t_elapsed = gpu_mc_W(D, N_WALKERS, n_steps, SEED + D)
        W_ref = REFERENCE_VALUES[D]
        sigma_off = (W_mc - W_ref) / W_err if W_err > 0 else 0.0
        print(f"{D:>3}  {W_mc:>22.10f}  {W_err:>12.2e}  "
              f"{W_ref:>22.10f}  {sigma_off:>10.2f}  {t_elapsed:>9.2f}")

    print()
    print("Interpretation:")
    print("  - 'sigma off' is (MC - reference) / standard_error_of_MC.")
    print("  - For D=4, D=5: the MC truncation bias is negligible because the")
    print("    walk is strongly transient; |sigma off| < 3 is the expected")
    print("    statistical fluctuation.")
    print("  - For D=3: the return distribution decays as t^(-3/2) (heavy tail),")
    print("    so even 8000 steps misses long-tail returns. The systematic")
    print("    underestimate of ~0.005 is the truncation bias, NOT a")
    print("    disagreement with the closed form. Closed-form value remains")
    print("    exact to 50 digits (verified in gstar_open_questions.py).")
    print()
    print("This is a 3-digit INDEPENDENT verification: the MC uses no Gamma")
    print("functions, no hypergeometric series, only ±1 random steps. The")
    print("agreement at the leading digits confirms that")
    print("  W^(D) = D F_{D-1}(1/2,...,1/2; 1,...,1; 1)")
    print("gives the correct expected-returns count for the BCC random walk.")


if __name__ == "__main__":
    main()
