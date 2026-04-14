"""
Look-Elsewhere Monte Carlo Analysis for FTD
===========================================

Purpose:
    Quantify the probability of finding a 1.26 ppm match to the fine structure constant (1/alpha)
    by pure chance within a defined search space of "plausible" geometric constructions.

The Claim:
    The FTD Master Quadratic: x^2 - 16(G*)^2 x + 16(G*)^3 = 0
    yields x_plus = 137.03617... which matches 1/alpha to 1.26 ppm.

The Skeptic's Argument:
    "You just tried random coefficients until it worked!"

The Counter-Proof:
    This script randomly generates "plausible" alternative equations and counts how many matches occur.
    
    Search Space Definition:
    1. Form:  x^2 - A*(G*)^n * x + B*(G*)^m = 0
    2. Coefficients A, B: Integers in [-32, 32] (covering the lattice DoF range)
    3. Powers n, m: Integers in [1, 4] (dimensional constraints)
    4. Constants: G* (Lemniscate) or pi or e (Common bases)

Usage:
    python look_elsewhere_monte_carlo.py --samples 1000000
"""

# Phase 8b (FTD Test Bench) -- converted to PyTorch with CUDA default.
# Original NumPy path preserved as fallback when torch is unavailable.
# Also fixes a pre-existing IndexError in the match-reporting code where
# filtered-array indices were incorrectly treated as chunk-global indices
# (see the `hits1`/`hits2` blocks below). The bug triggered at 1M samples
# because finding any match required indexing into x1 with the wrong shape.

import os
import sys
import numpy as np
import argparse
import time
from scipy.special import gamma

_SCRIPTS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if _SCRIPTS_DIR not in sys.path:
    sys.path.insert(0, _SCRIPTS_DIR)
try:
    from constants import TORCH, DEVICE, DTYPE
except ImportError:
    TORCH = None
    DEVICE = None
    DTYPE = None

print(f"[backend] device={DEVICE}, torch={TORCH is not None}")

# =============================================================================
# CONSTANTS & TARGETS
# =============================================================================

# The Target: Inverse Fine Structure Constant (CODATA 2022)
ALPHA_INV_TARGET = 137.035999177
TOLERANCE_PPM = 1.26  # The match quality we achieved
TOLERANCE_ABS = ALPHA_INV_TARGET * (TOLERANCE_PPM / 1e6)

# The Base Constant: Lemniscatic Constant G*
# Derived from G* = sqrt(2)*Gamma(1/4)^2 / (2*pi)
GAMMA_QUARTER = gamma(0.25)
G_STAR = np.sqrt(2) * GAMMA_QUARTER**2 / (2 * np.pi)

# Other "plausible" bases a numerologist might try
BASES = {
    'G*': G_STAR,
    'pi': np.pi,
    'e': np.e,
    'phi': (1 + np.sqrt(5)) / 2
}

# =============================================================================
# MONTE CARLO ENGINE
# =============================================================================

def run_monte_carlo(num_samples=1000000, seed=42):
    """
    Run the Monte Carlo simulation.
    
    We generate random quadratics of the form:
        x^2 + C1 * (Base)^p1 * x + C2 * (Base)^p2 = 0
        
    And checking for positive real roots matching ALPHA_INV_TARGET.
    """
    rng = np.random.default_rng(seed)
    
    print(f"Starting Monte Carlo Analysis...")
    print(f"Target: {ALPHA_INV_TARGET} +/- {TOLERANCE_PPM} ppm")
    print(f"Search Space:")
    print(f"  - Coefficients: integers [-32, 32] (excluding 0)")
    print(f"  - Powers: integers [1, 4]")
    print(f"  - Bases: {list(BASES.keys())}")
    print(f"Samples: {num_samples}")
    print("-" * 60)

    start_time = time.time()
    matches = []
    
    # Pre-compute bases to avoid dict lookups in loop
    base_names = list(BASES.keys())
    base_values = np.array(list(BASES.values()))
    num_bases = len(base_values)

    # Vectorized generation for speed
    # We'll do chunks to verify memory safety
    chunk_size = min(num_samples, 1000000)
    total_processed = 0
    
    while total_processed < num_samples:
        current_chunk = min(chunk_size, num_samples - total_processed)
        
        # 1. Select Bases (random index)
        base_indices = rng.integers(0, num_bases, size=current_chunk)
        bases = base_values[base_indices]
        
        # 2. Select Coefficients C1, C2 in [-32, 32], excluding 0
        # We generate in [-32, 33] and map 0 to 1 (simple rejection is slower)
        c1 = rng.integers(-32, 33, size=current_chunk)
        c2 = rng.integers(-32, 33, size=current_chunk)
        # Avoid zero coefficients (trivial equations)
        c1[c1 == 0] = 1
        c2[c2 == 0] = 1
        
        # 3. Select Powers p1, p2 in [1, 4]
        p1 = rng.integers(1, 5, size=current_chunk)
        p2 = rng.integers(1, 5, size=current_chunk)
        
        # 4. Construct Quadratic: x^2 + b*x + c = 0
        # Term 1: C1 * Base^p1
        # Term 2: C2 * Base^p2
        # Note: In FTD, the equation is x^2 - 16G*^2 x + 16G*^3 = 0
        # So b = -16G*^2, c = 16G*^3
        # Our generator covers this form (C1=-16, p1=2, C2=16, p2=3)

        if TORCH is not None:
            # GPU path: do the dominant FLOPs (b, c_term, discriminant) on
            # DEVICE, then pull them back to CPU for the downstream boolean
            # indexing logic. RNG draws stay on CPU so the seeded numpy
            # stream is preserved and the hit report is reproducible.
            bases_t = TORCH.tensor(bases, device=DEVICE, dtype=DTYPE)
            c1_t = TORCH.tensor(c1, device=DEVICE, dtype=DTYPE)
            c2_t = TORCH.tensor(c2, device=DEVICE, dtype=DTYPE)
            p1_t = TORCH.tensor(p1, device=DEVICE, dtype=DTYPE)
            p2_t = TORCH.tensor(p2, device=DEVICE, dtype=DTYPE)
            b_t = c1_t * TORCH.pow(bases_t, p1_t)
            c_term_t = c2_t * TORCH.pow(bases_t, p2_t)
            disc_t = b_t * b_t - 4 * c_term_t
            b = b_t.detach().cpu().numpy()
            c_term = c_term_t.detach().cpu().numpy()
            discriminant = disc_t.detach().cpu().numpy()
        else:
            b = c1 * (bases ** p1)
            c_term = c2 * (bases ** p2)

            # 5. Solve Quadratic: x = (-b +/- sqrt(b^2 - 4ac)) / 2a
            # a = 1

            discriminant = b**2 - 4 * c_term
        
        # Filter: Discriminant must be non-negative for real roots
        valid_d = discriminant >= 0
        
        if np.any(valid_d):
            sqrt_d = np.sqrt(discriminant[valid_d])
            b_valid = b[valid_d]
            
            # Root 1
            x1 = (-b_valid + sqrt_d) / 2
            # Root 2
            x2 = (-b_valid - sqrt_d) / 2
            
            # Check matches for x1
            diff1 = np.abs(x1 - ALPHA_INV_TARGET)
            hits1 = diff1 <= TOLERANCE_ABS
            
            # Check matches for x2
            diff2 = np.abs(x2 - ALPHA_INV_TARGET)
            hits2 = diff2 <= TOLERANCE_ABS
            
            # Record details of matches
            # We need to map back to the original parameters for reporting
            
            if np.any(hits1):
                indices = np.where(hits1)[0] # indices within valid_d subset
                # We need global indices within chunk to recover parameters
                # This is getting complex for vectorized. Let's precise loop for hits only?
                # Actually, simpler: just store the counts most of the time.
                # But we want to SEE the false positives if any.

                # Reconstruct parameters for hits
                # Map subset indices back to chunk indices
                valid_indices = np.where(valid_d)[0]
                hit_indices = valid_indices[indices]

                # Bug fix: iterate over both the subset-local index (into
                # the filtered `x1`) and the chunk-global index (into `c1`,
                # `p1`, etc). The original unconditionally indexed `x1` with
                # the chunk-global index, which is out of bounds whenever
                # at least one sample had a negative discriminant before a
                # hit (i.e. always, at 1e6 samples).
                for subset_idx, chunk_idx in zip(indices, hit_indices):
                    # Recompute the root from chunk-global b / c_term to
                    # avoid the filtered-vs-chunk confusion entirely.
                    d_val = b[chunk_idx]**2 - 4 * c_term[chunk_idx]
                    root_val = (-b[chunk_idx] + np.sqrt(d_val)) / 2
                    matches.append({
                        'base': base_names[base_indices[chunk_idx]],
                        'C1': c1[chunk_idx],
                        'p1': p1[chunk_idx],
                        'C2': c2[chunk_idx],
                        'p2': p2[chunk_idx],
                        'root': root_val,
                        'diff_ppm': abs(root_val - ALPHA_INV_TARGET)/ALPHA_INV_TARGET * 1e6
                    })

            if np.any(hits2):
                indices = np.where(hits2)[0]
                valid_indices = np.where(valid_d)[0]
                hit_indices = valid_indices[indices]
                
                for idx in hit_indices:
                     # Calculate x2 again to be sure
                    d_val = b[idx]**2 - 4*c_term[idx]
                    root_val = (-b[idx] - np.sqrt(d_val))/2
                    
                    matches.append({
                        'base': base_names[base_indices[idx]],
                        'C1': c1[idx],
                        'p1': p1[idx],
                        'C2': c2[idx],
                        'p2': p2[idx],
                        'root': root_val,
                        'diff_ppm': abs(root_val - ALPHA_INV_TARGET)/ALPHA_INV_TARGET * 1e6
                    })
        
        total_processed += current_chunk
    
    elapsed = time.time() - start_time
    
    # =========================================================================
    # REPORTING
    # =========================================================================
    print(f"\nAnalysis Complete in {elapsed:.2f} seconds.")
    print(f"Total Equations Checked: {num_samples}")
    print(f"Matches found (< {TOLERANCE_PPM} ppm): {len(matches)}")
    print("-" * 60)
    
    # Is the FTD match in the list? (Sanity check)
    # The generator is random, so it might NOT be if sample Size < Space size.
    # Space Size approx: 4 (bases) * 64 (C1) * 4 (p1) * 64 (C2) * 4 (p2) = 262,144
    # With 1M samples, we likely covered it multiple times.
    
    unique_matches = []
    seen = set()
    
    for m in matches:
        # Create a signature
        sig = (m['base'], m['C1'], m['p1'], m['C2'], m['p2'])
        if sig not in seen:
            seen.add(sig)
            unique_matches.append(m)
            
    print(f"Unique Matches: {len(unique_matches)}")
    print("\nListing Unique Matches:")
    print(f"{'Base':<6} {'Eq':<30} {'Root':<15} {'ppm':<10}")
    print("-" * 70)
    
    found_ftd = False
    
    for m in unique_matches:
        # Format: x^2 + C1*B^p1 * x + C2*B^p2 = 0
        eq_str = f"x^2 + {m['C1']}*{m['base']}^{m['p1']}*x + {m['C2']}*{m['base']}^{m['p2']}"
        print(f"{m['base']:<6} {eq_str:<30} {m['root']:<15.5f} {m['diff_ppm']:<10.4f}")
        
        # Check if this is the FTD equation
        # FTD: x^2 - 16(G*)^2 x + 16(G*)^3 = 0  => C1=-16, p1=2, C2=16, p2=3, base=G*
        if (m['base'] == 'G*' and m['C1'] == -16 and m['p1'] == 2 and 
            m['C2'] == 16 and m['p2'] == 3):
            found_ftd = True
            
    print("-" * 70)
    
    if found_ftd:
        print(">> FTD Master Quadratic CONFIRMED in search space.")
    else:
        print(f">> FTD Master Quadratic NOT hit (random chance).")
        
    # P-value calculation
    # We want the probability of finding a match this good or better.
    # Note: We include the FTD match in the count if it was found randomly.
    # If the FTD match is the *only* one, and we searched the whole space...
    
    # Approximate Search Space Size:
    space_size = len(BASES) * (64 * 4) * (64 * 4) # ~262k
    
    # If samples > space_size, we saturated the space.
    # The p-value is essentially (Unique Plausible Matches) / (Total Plausible Equations)
    
    p_value = len(unique_matches) / space_size
    print(f"\nEstimated Search Space Size: {space_size}")
    print(f"Raw P-Value (Unique Matches / Space Size): {p_value:.2e}")
    print(f"Interpretation: The probability of a random equation in this class matching alpha is {p_value:.2%}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="FTD Look-Elsewhere Monte Carlo")
    parser.add_argument("--samples", type=int, default=1000000, help="Number of random equations to generate")
    args = parser.parse_args()
    
    run_monte_carlo(num_samples=args.samples)
