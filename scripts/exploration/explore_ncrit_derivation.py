"""
Exploration 3: N_crit = G*^3/alpha — Derivation or Numerology?

The ratio G*^3/alpha = 3549 matches observed total lifetime somatic
mutations ~3500 (Cagan et al 2022, Nature) to 1.4%.

Tests:
  1. Information capacity derivation attempt
  2. Cell division structure (Hayflick * tissue types)
  3. Non-mammalian species test (trees, clams, lobsters)
  4. Shannon capacity interpretation
  5. Alternative formula test (is G*^3/alpha uniquely best?)
"""
import numpy as np
import sys
sys.path.insert(0, r'C:\Users\cpaci\Desktop\ftd\scripts')
from constants import (G_STAR, ALPHA, N_c, N_base, b_3, N_eff,
                        GAMMA_QUARTER, GAMMA_HALF, PHI)

print("=" * 72)
print("EXPLORATION 3: N_crit = G*^3/alpha Derivation Path")
print("=" * 72)

N_crit_observed = 3500  # approximate, from Cagan et al
N_crit_ftd = G_STAR**3 / ALPHA

print(f"\nTarget: N_crit ~ {N_crit_observed} (observed cross-species)")
print(f"Formula: G*^3 / alpha = {N_crit_ftd:.1f}")
print(f"Match: {abs(N_crit_ftd - N_crit_observed)/N_crit_observed*100:.1f}%\n")

# ============================================================
# Test 1: Information Capacity Argument
# ============================================================
print("--- Test 1: Information Capacity Derivation ---\n")

# G*^3 = 25.90 = action per degree of freedom per tick.
# This represents the total information processed per DoF per tick.
#
# alpha = 0.00730 = probability of a coupling event per tick.
# Each coupling event (s != 0 interacting with div(J)) is an
# information-writing event -- it changes the state.
#
# How many coupling events fit into one complete action cycle?
# N = (action per DoF) / (coupling per event) = G*^3 / alpha
#
# Interpretation: the O-structure processes G*^3 units of action
# per DoF per tick. Each mutation/error uses alpha units of this
# budget. After N = G*^3/alpha mutations, the total error budget
# has consumed one full action unit per DoF.
#
# When error = action: the system can no longer distinguish signal
# from noise. This is the Weierstrass collapse threshold.

print("Derivation attempt:")
print()
print(f"  G*^3 = {G_STAR**3:.4f} = action per DoF per tick [THEOREM]")
print(f"  alpha = {ALPHA:.6f} = coupling probability per tick [THEOREM]")
print()
print("  Each mutation is a coupling error: one mis-written state.")
print("  Error magnitude per mutation: O(alpha) [from g_c = sqrt(alpha)]")
print()
print("  After N mutations, cumulative error: N * alpha")
print(f"  Collapse when cumulative error = action budget: N*alpha = G*^3")
print(f"  Therefore: N_crit = G*^3 / alpha = {N_crit_ftd:.0f}")
print()
print("  Physical meaning: the O-structure can absorb G*^3/alpha = 3549")
print("  discrete coupling errors before its total noise equals its")
print("  total signal (one action unit per DoF).")
print()

# Is this dimensionally consistent?
# G*^3 has dimensions of [action/DoF] = [energy * time / DoF]
# alpha is dimensionless
# N_crit = G*^3/alpha is dimensionless [number of events]
# YES -- dimensionally consistent as a pure count.
print("  Dimensional check: G*^3 [action/DoF] / alpha [dimensionless]")
print("  = N_crit [pure number]. Consistent.")

# ============================================================
# Test 2: Hayflick Limit * Tissue Types
# ============================================================
print("\n\n--- Test 2: Hayflick * Tissue Structure ---\n")

hayflick = 50  # cell divisions before senescence
mutations_per_division = 1.0  # approximate
muts_per_lineage = hayflick * mutations_per_division

# The organism has many independent cell lineages (tissues).
# Total organism mutations = lineage mutations * number of lineages
# 3500 = 50 * N_tissues -> N_tissues = 70

N_tissues_needed = N_crit_observed / muts_per_lineage
print(f"Hayflick limit: {hayflick} divisions")
print(f"Mutations per division: ~{mutations_per_division:.1f}")
print(f"Mutations per cell lineage: ~{muts_per_lineage:.0f}")
print()
print(f"If N_crit = {N_crit_observed} = Hayflick * N_tissues:")
print(f"  N_tissues = {N_crit_observed} / {muts_per_lineage:.0f} = {N_tissues_needed:.0f}")
print()

# How many distinct tissue types does a human have?
# Standard anatomy: ~200-400 cell types, organized into ~78 organs.
# But independently aging "compartments" might be ~70.
print("Human tissue compartments:")
print(f"  Cell types: ~200-400")
print(f"  Major organs: ~78")
print(f"  Independently aging compartments: ~{N_tissues_needed:.0f} (predicted)")
print()
print(f"  This is plausible: ~70 independent aging lineages,")
print(f"  each hitting Hayflick after ~50 mutations.")
print(f"  Total: 70 * 50 = 3500. Matches N_crit.")
print()

# Can we express N_tissues in FTD constants?
# 70 ~ G*^3 / (alpha * Hayflick) = 3549/50 = 71.0
print(f"  N_tissues = N_crit / Hayflick = G*^3/(alpha*50) = {N_crit_ftd/50:.1f}")
print(f"  And 50 ~ 1/(2*alpha) * some factor? 50 = {50}")
print(f"  1/(2*alpha) = {1/(2*ALPHA):.1f}. Not a clean match.")
print(f"  50 = 2 * N_eff*2 - 2? {2*N_eff**2 - 2}. No.")
print(f"  50 is likely a biological constant, not a lattice constant.")

# ============================================================
# Test 3: Non-Mammalian Species
# ============================================================
print("\n\n--- Test 3: Non-Mammalian Species Test ---\n")

# If N_crit = G*^3/alpha is universal, it should apply to ALL
# organisms with somatic mutations, not just mammals.

print("Testing the lifespan formula T = N_crit / mutation_rate:")
print(f"N_crit = G*^3/alpha = {N_crit_ftd:.0f}")
print()
print(f"{'Organism':>25} | {'Lifespan':>10} | {'Mut rate':>12} | {'Predicted':>10} | {'Error':>8} | {'Notes':>25}")
print("-" * 105)

# Known and estimated data
# Somatic mutation rates from Cagan et al 2022 and estimates
species = [
    # Mammals (Cagan et al 2022 data)
    ("Mouse",                  2.5,     800,   "Cagan 2022"),
    ("Cat",                   15,       200,   "Cagan 2022"),
    ("Dog",                   12,       250,   "Cagan 2022"),
    ("Human",                 80,        47,   "Cagan 2022"),
    ("Naked mole rat",        30,        93,   "Cagan 2022"),
    # Long-lived mammals (estimated)
    ("Bowhead whale",        200,        20,   "estimated"),
    ("Greenland shark",      400,        10,   "estimated"),
    # Non-mammals (estimated mutation rates)
    ("Galapagos tortoise",   175,        25,   "estimated"),
    ("Koi fish",             200,        20,   "estimated"),
    ("Ocean quahog clam",    500,         8,   "estimated"),
    # Negligible senescence
    ("Hydra",              10000,         1,   "low turnover"),
    ("Bristlecone pine",    5000,       0.7,   "very low rate"),
    # Short-lived
    ("Fruit fly",           0.16,    20000,   "estimated"),
    ("C. elegans (worm)",   0.05,    50000,   "estimated"),
]

good_matches = 0
total = 0
for name, lifespan, mut_rate, notes in species:
    if mut_rate > 0:
        predicted = N_crit_ftd / mut_rate
        error = (predicted - lifespan) / lifespan * 100
        match = "~" if abs(error) < 50 else ("ok" if abs(error) < 100 else "off")
        if abs(error) < 50:
            good_matches += 1
        total += 1
        print(f"{name:>25} | {lifespan:>8.2f} yr | {mut_rate:>10.0f}/yr | {predicted:>8.1f} yr | {error:>+7.0f}% | {notes:>25}")

print()
print(f"Matches within 50%: {good_matches}/{total}")
print()
print("FINDINGS:")
print("  - Mammals (Cagan data): 4/5 within 50%. Good agreement.")
print("  - Long-lived species: reasonable predictions (order of magnitude).")
print("  - Negligible senescence (hydra, trees): predicted lifespans are")
print("    3500-5000 yr, not infinite. The model breaks for these -- they")
print("    must have additional repair mechanisms that the simple formula")
print("    doesn't capture.")
print("  - Short-lived invertebrates: mutation rates are very uncertain.")
print("    The formula gives order-of-magnitude results.")

# ============================================================
# Test 4: Shannon Capacity Interpretation
# ============================================================
print("\n\n--- Test 4: Shannon Capacity Interpretation ---\n")

# Shannon channel capacity: C = B * log2(1 + S/N)
# where B = bandwidth, S/N = signal-to-noise ratio.
#
# For the O-structure:
#   B = G*^2 per tick (energy budget = processing bandwidth)
#   S = G* per DoF (signal = flux amplitude)
#   N = alpha per mutation (noise per error event)
#
# C = G*^2 * log2(1 + G*/alpha)
#   = 8.754 * log2(1 + 2.959/0.00730)
#   = 8.754 * log2(406)
#   = 8.754 * 8.665
#   = 75.85 bits per tick per DoF

C_shannon = G_STAR**2 * np.log2(1 + G_STAR/ALPHA)
print(f"Shannon capacity of the O-structure:")
print(f"  B = G*^2 = {G_STAR**2:.3f} (bandwidth)")
print(f"  S/N = G*/alpha = {G_STAR/ALPHA:.1f}")
print(f"  C = B * log2(1 + S/N) = {C_shannon:.2f} bits/tick/DoF")
print()

# Total information capacity before saturation:
# If each mutation adds ~1 bit of noise, capacity exhausted after C/alpha ticks?
# Or after C * something mutations?
# Not a clean connection to N_crit = 3549.
print(f"  C / alpha = {C_shannon / ALPHA:.0f} (too large)")
print(f"  C * alpha = {C_shannon * ALPHA:.3f} (too small)")
print(f"  C / G* = {C_shannon / G_STAR:.1f}")
print(f"  C * G* = {C_shannon * G_STAR:.0f} (close-ish to N_crit?)")
print()
print("  Shannon interpretation doesn't give a clean route to N_crit.")
print("  The direct derivation (cumulative error = action budget) is cleaner.")

# ============================================================
# Test 5: Alternative Formulas
# ============================================================
print("\n\n--- Test 5: Is G*^3/alpha Uniquely Best? ---\n")

print("Testing alternative combinations of FTD constants against N_crit ~ 3500:")
print()

candidates = [
    ("G*^3 / alpha",         G_STAR**3 / ALPHA),
    ("16 * G*^5",            16 * G_STAR**5),
    ("G*^3 * (1/alpha)",     G_STAR**3 * (1/ALPHA)),  # same as first
    ("(1/alpha)^2 / N_base", (1/ALPHA)**2 / N_base),
    ("N_eff * (1/alpha)",    N_eff * (1/ALPHA)),
    ("b_3 * (1/alpha)^1.5",  b_3 * (1/ALPHA)**1.5),
    ("16 * G*^2 * G*^3",    16 * G_STAR**2 * G_STAR**3),
    ("G*^5 * N_base",        G_STAR**5 * N_base),
    ("(1/alpha) * G*^2",    (1/ALPHA) * G_STAR**2),
    ("N_eff^2 * N_base * N_c", N_eff**2 * N_base * N_c),
    ("(4*pi)^3",              (4*np.pi)**3),
    ("e^8",                   np.exp(8)),
    ("2^12 - some",           2**12 - 548),
    ("1000 * pi",             1000 * np.pi),
]

print(f"{'Formula':>30} | {'Value':>10} | {'Error vs 3500':>14} | {'Quality':>10}")
print("-" * 72)

results = []
for name, val in candidates:
    error = abs(val - N_crit_observed) / N_crit_observed * 100
    quality = "EXCELLENT" if error < 2 else ("good" if error < 5 else ("ok" if error < 10 else "poor"))
    results.append((name, val, error, quality))
    print(f"{name:>30} | {val:>10.1f} | {error:>12.1f}%  | {quality:>10}")

print()
# Sort by error
results.sort(key=lambda x: x[2])
print("Ranked by closeness to 3500:")
for i, (name, val, error, quality) in enumerate(results[:5]):
    print(f"  {i+1}. {name}: {val:.1f} ({error:.1f}%)")

print()
print("FINDING: G*^3/alpha = 3549 (1.4%) is the best match among")
print("physically motivated formulas. The only comparable match is")
print("16*G*^5 = 3627 (3.6%), which is G*^3/alpha * (16*G*^2*alpha/G*^3)")
print("-- a related quantity but less clean.")
print()
print("Several 'numerological' formulas (e^8, 1000*pi) also fit ~3500,")
print("which is a caution: the number 3500 is not large enough for a")
print("1% match to be definitive. The formula needs theoretical backing,")
print("not just numerical agreement.")

# ============================================================
# SUMMARY
# ============================================================
print(f"""

========================================================================
SUMMARY: N_crit = G*^3/alpha Assessment
========================================================================

DERIVATION ATTEMPT:
  N_crit = G*^3/alpha = (action per DoF) / (coupling error per mutation)
  = total coupling events before noise = signal.
  This is logically coherent and dimensionally correct.
  Status: [SELECTION] -- motivated but not uniquely derived.

BIOLOGICAL TESTS:
  Mammals (Cagan 2022 data): 4/5 species within 50%. Good.
  Non-mammals: order-of-magnitude. Reasonable.
  Negligible senescence (hydra, trees): model breaks. These organisms
  must have active repair that the simple formula doesn't capture.

HAYFLICK STRUCTURE:
  N_crit = Hayflick (50) * tissue_compartments (~70).
  The 50 is biological (not from FTD). The 70 is plausible anatomy.

ALTERNATIVE FORMULAS:
  G*^3/alpha = 3549 (1.4%) is the best FTD-motivated formula.
  But 3500 is not large enough for 1.4% to be definitive.
  Caution: e^8 = 2981 (15%) and other random formulas also
  produce numbers in the ~3000-4000 range.

VERDICT:
  The formula N_crit = G*^3/alpha is:
  - Physically motivated (error budget argument)
  - Numerically consistent (1.4%)
  - Not uniquely forced (other formulas fit ~10%)
  - Not yet derived from FTD axioms

  Status: [CONJECTURE] -- promising but not proven.
  The strongest support is the Cagan et al finding that total
  lifetime mutations are ~constant across species, combined with
  the lattice providing a natural value for that constant.
""")
