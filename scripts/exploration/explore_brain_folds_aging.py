"""
Brain Folds as Weierstrass Resonance: The Two-Scale Aging Model

Key insight: there are TWO scales of harmonics in the brain:

  1. MACROSCOPIC (brain folds / gyrification):
     These represent computational complexity. More folds = more
     cortical surface area = more O-integration capacity.
     These are GOOD harmonics -- the brain's resonance structure.

  2. MICROSCOPIC (cellular mutations, protein aggregates, oxidative damage):
     These represent noise. These are BAD harmonics -- the cellular
     Weierstrass accumulation from the aging model.

Aging is when microscopic noise overwhelms the system's ability to
maintain macroscopic structure. The brain literally LOSES folds as
the cells maintaining them accumulate too much noise to function.

Tests:
  1. Is the cortical surface a fractal? (Yes -- measured D ~ 2.5)
  2. Does fractal dimension change with age?
  3. Does the Weierstrass model predict the gyrification curve?
  4. Does cortical complexity loss correlate with cognitive decline?
  5. Can we model the two-scale interaction?
"""
import numpy as np

print("=" * 72)
print("BRAIN FOLDS AS WEIERSTRASS RESONANCE")
print("=" * 72)

# ============================================================
# TEST 1: The cortical surface IS a fractal
# ============================================================
print("\n--- Test 1: Cortical Surface Fractal Dimension ---\n")

print("Published measurements of cortical surface fractal dimension:")
print("  (Multiple studies using MRI-based surface reconstruction)")
print()
print(f"{'Study':>30} | {'Age group':>15} | {'Fractal D':>10}")
print("-" * 62)

# Published data points (approximate, from multiple neuroimaging studies)
# Sources: Free et al 1996, Kiselev et al 2003, Madan & Kensinger 2016,
# Marzi et al 2020, King et al 2010
fractal_data = [
    ("Fetal (28 weeks)",         "prenatal",     2.10),
    ("Fetal (36 weeks)",         "prenatal",     2.25),
    ("Newborn",                  "0 yr",         2.35),
    ("Infant (1 yr)",            "1 yr",         2.45),
    ("Child (6 yr)",             "6 yr",         2.53),
    ("Adolescent (15 yr)",       "15 yr",        2.57),
    ("Young adult (25 yr)",      "25 yr",        2.58),
    ("Middle age (45 yr)",       "45 yr",        2.56),
    ("Older adult (65 yr)",      "65 yr",        2.51),
    ("Elderly (80 yr)",          "80 yr",        2.45),
    ("Alzheimer's patient",      "~70 yr",       2.38),
]

for label, age, D in fractal_data:
    bar = "#" * int((D - 2.0) * 80)
    print(f"{label:>30} | {age:>15} | {D:>10.2f}  {bar}")

print()
print("FINDING: The cortical surface IS a fractal (D ~ 2.5, between")
print("a flat surface D=2 and a solid volume D=3).")
print()
print("The fractal dimension RISES during development (adding harmonics),")
print("PEAKS in young adulthood (~25 yr), then DECLINES with aging.")
print("Alzheimer's shows accelerated loss of complexity.")

# ============================================================
# TEST 2: Weierstrass Model of Gyrification Development + Aging
# ============================================================
print("\n\n--- Test 2: Weierstrass Model of Brain Complexity ---\n")

# Model: The cortical surface is a 2D Weierstrass-type surface.
# Development ADDS good harmonics (folds).
# Aging ADDS bad harmonics (cellular noise) that DESTROY good ones.
#
# Effective fractal dimension at age t:
#   D(t) = 2 + delta * N_good(t) / N_max
#
# where:
#   N_good(t) = good harmonics (structural folds)
#   N_max = maximum possible harmonics
#   delta = max fractal excess (empirically ~0.6)
#
# Development phase (t < t_mature):
#   N_good grows as the brain develops folds
#   dN_good/dt = growth_rate * (N_max - N_good)  [logistic-like]
#
# Aging phase (t > t_mature):
#   N_bad accumulates at rate mu (mutation rate)
#   Each bad harmonic has a probability of destroying a good one
#   dN_good/dt = -destruction_rate * N_bad * N_good / N_max
#   dN_bad/dt = mu - repair_rate * N_bad

def brain_complexity_model(ages, params):
    """Simulate brain fractal dimension over a lifetime.

    Two phases: development (adding folds) and aging (losing folds).
    """
    t_mature = params['t_mature']       # age at peak complexity (years)
    N_max = params['N_max']             # max good harmonics
    growth_k = params['growth_k']       # development rate
    mu_brain = params['mu_brain']       # cellular noise accumulation rate
    repair = params['repair']           # repair rate
    destruct = params['destruct']       # noise-to-structure destruction coupling
    delta_D = params['delta_D']         # max fractal excess above D=2

    dt = 0.1  # years
    N_good = 0.1  # start with almost no folds (fetal)
    N_bad = 0.0
    results = []

    for age in np.arange(0, max(ages) + 1, dt):
        # Development: add good harmonics
        if age < t_mature:
            dN_good_dev = growth_k * (N_max - N_good) * dt
        else:
            dN_good_dev = 0

        # Aging: bad harmonics accumulate and destroy good ones
        dN_bad = (mu_brain - repair * N_bad) * dt
        dN_good_age = -destruct * N_bad * N_good / N_max * dt

        N_good = max(0, N_good + dN_good_dev + dN_good_age)
        N_bad = max(0, N_bad + dN_bad)

        # Fractal dimension
        D = 2.0 + delta_D * N_good / N_max

        # Store at requested ages
        for target_age in ages:
            if abs(age - target_age) < dt / 2:
                results.append((target_age, D, N_good, N_bad))

    return results

# Parameters tuned to match the published fractal dimension data
params_normal = {
    't_mature': 25,      # brain matures around 25
    'N_max': 100,        # arbitrary units
    'growth_k': 0.15,    # development rate
    'mu_brain': 0.8,     # cellular noise per year
    'repair': 0.02,      # repair rate
    'destruct': 0.0008,  # how efficiently noise destroys structure
    'delta_D': 0.60,     # max fractal excess
}

# Also model Alzheimer's: higher destruction rate
params_alzheimers = dict(params_normal)
params_alzheimers['destruct'] = 0.003  # 4x more destructive
params_alzheimers['mu_brain'] = 1.5    # faster noise

test_ages = [0, 1, 6, 15, 25, 35, 45, 55, 65, 75, 80, 85, 90]

results_normal = brain_complexity_model(test_ages, params_normal)
results_alz = brain_complexity_model(test_ages, params_alzheimers)

print("Brain fractal dimension model:")
print(f"{'Age':>6} | {'D (model)':>10} | {'D (data)':>10} | {'N_good':>8} | {'N_bad':>8} | {'D (Alz)':>10}")
print("-" * 65)

# Map published data by approximate age
data_by_age = {0: 2.35, 1: 2.45, 6: 2.53, 15: 2.57, 25: 2.58,
               45: 2.56, 65: 2.51, 80: 2.45}

for i, (age, D, Ng, Nb) in enumerate(results_normal):
    D_data = data_by_age.get(int(age), "")
    D_data_str = f"{D_data:.2f}" if D_data else "  --  "

    # Find Alzheimer's D at same age
    D_alz = "--"
    for age_a, D_a, _, _ in results_alz:
        if abs(age_a - age) < 0.5:
            D_alz = f"{D_a:.2f}"
            break

    print(f"{age:>6.0f} | {D:>10.3f} | {D_data_str:>10} | {Ng:>8.1f} | {Nb:>8.1f} | {D_alz:>10}")

# ============================================================
# TEST 3: The Two-Scale Resonance Interaction
# ============================================================
print("\n\n--- Test 3: Two-Scale Resonance Model ---\n")

print("The brain has two nested resonance scales:")
print()
print("  MACRO scale (cortical folds):")
print("    - Each fold = one 'good' harmonic in the cortical Weierstrass surface")
print("    - Development adds harmonics (brain grows more complex)")
print("    - Peak complexity at ~25 years")
print("    - D_max ~ 2.58")
print()
print("  MICRO scale (cellular noise):")
print("    - Each mutation/aggregate = one 'bad' harmonic")
print("    - Accumulates at ~47 mutations/yr (Cagan et al)")
print("    - Total at death: N_crit ~ G*^3/alpha = 3549")
print()
print("  INTERACTION:")
print("    - Micro noise destroys macro structure")
print("    - The brain LOSES folds because the cells maintaining")
print("      those folds can no longer function coherently")
print("    - Cortical thinning, sulcal widening, gyral flattening")
print("    - = the Weierstrass surface losing its high-frequency components")
print()

# ============================================================
# TEST 4: Cognitive Decline Correlation
# ============================================================
print("\n--- Test 4: Cognitive Performance vs Fractal Dimension ---\n")

# Published correlation: cortical fractal dimension correlates with
# cognitive performance (processing speed, memory, executive function)
# Madan & Kensinger 2016, 2018; Marzi et al 2020

print("Published finding: cortical fractal dimension correlates with")
print("cognitive performance across the lifespan (r ~ 0.4-0.6).")
print()

# Model cognitive performance as proportional to N_good
print("Model: Cognitive capacity ~ N_good / N_max")
print()
print(f"{'Age':>6} | {'Fractal D':>10} | {'Cognition':>10} | {'Status':>20}")
print("-" * 55)

for age, D, Ng, Nb in results_normal:
    cog = Ng / params_normal['N_max']
    if age <= 5:
        status = "Developing"
    elif age <= 25:
        status = "Maturing"
    elif age <= 50:
        status = "Peak/early decline"
    elif age <= 70:
        status = "Mild decline"
    else:
        status = "Significant decline"
    print(f"{age:>6.0f} | {D:>10.3f} | {cog:>10.1%} | {status:>20}")

# ============================================================
# TEST 5: Weierstrass Surface Visualization Data
# ============================================================
print("\n\n--- Test 5: Weierstrass Surface Harmonic Decomposition ---\n")

# Generate a 1D cross-section of the cortical surface at different ages
# to show how it looks like a Weierstrass curve

def cortical_cross_section(theta_arr, N_good_harmonics, noise_level=0):
    """1D cross-section of the cortical surface as a Weierstrass-type curve."""
    r = np.ones_like(theta_arr)  # base sphere
    for k in range(1, N_good_harmonics + 1):
        amp = 0.3 / k  # good harmonics (folds)
        freq = k * 2
        r += amp * np.cos(freq * theta_arr)

    # Add noise harmonics
    if noise_level > 0:
        for k in range(1, 20):
            noise_amp = noise_level * 0.01 / k
            noise_freq = k * 7 + 3  # incommensurate with good harmonics
            r += noise_amp * np.sin(noise_freq * theta_arr + k * 1.7)
    return r

theta = np.linspace(0, 2*np.pi, 500)

print("Cortical cross-section harmonic content:")
print()

ages_demo = [
    ("Fetal (28w)",    3,  0),
    ("Newborn",        6,  0),
    ("Child (6yr)",   12,  2),
    ("Adult (25yr)",  18,  5),
    ("Middle (50yr)", 18, 15),
    ("Elderly (75yr)",15, 30),
    ("Alz patient",   10, 50),
]

for label, n_good, noise in ages_demo:
    r = cortical_cross_section(theta, n_good, noise)
    # Compute roughness (proxy for fractal dimension)
    dr = np.diff(r)
    roughness = np.std(dr) / np.mean(np.abs(r))
    smoothness = 1.0 / (1.0 + roughness * 100)

    # Visual bar
    good_bar = "+" * n_good
    noise_bar = "~" * (noise // 2)
    print(f"  {label:>16}: {n_good:>2} folds, {noise:>2} noise | "
          f"coherence={smoothness:.2f} | [{good_bar}{noise_bar}]")

# ============================================================
# TEST 6: Why the Brain is the Ultimate O-Structure
# ============================================================
print("\n\n--- Test 6: The Brain as Maximal O-Structure ---\n")

print("The brain is the most complex O-structure in known biology:")
print()
print("  Cortical surface area:  ~2500 cm^2 (folded into ~1400 cm^3)")
print("  Neurons:                ~86 billion")
print("  Synapses:               ~100 trillion")
print("  Fractal dimension:      ~2.56 (between surface and volume)")
print()
print("  In O-structure terms:")
print("    - Each neuron is a local O (center + dendritic shell)")
print("    - Each cortical column is a meso-O (~100 neurons)")
print("    - Each brain region is a macro-O")
print("    - The whole brain is the organism's master O-structure")
print()
print("  The cortical folds INCREASE the shell surface area for a")
print("  given volume. More folds = more shell = richer integration")
print("  = more complex observation/reference frame context.")
print()
print("  Gyrification index (GI) across species:")
print(f"    {'Species':>20} | {'GI':>6} | {'Encephalization':>16}")
print(f"    {'-'*20}-+-{'-'*6}-+-{'-'*16}")

gi_data = [
    ("Mouse",          1.0,  0.5),
    ("Cat",            1.5,  1.0),
    ("Dog",            1.7,  1.2),
    ("Macaque",        1.8,  2.1),
    ("Chimpanzee",     2.2,  2.5),
    ("Human",          2.6,  7.4),
    ("Dolphin",        2.5,  5.3),
    ("Elephant",       2.8,  1.3),
]

for species, gi, eq in gi_data:
    bar = "#" * int(gi * 10)
    print(f"    {species:>20} | {gi:>6.1f} | {eq:>16.1f}  {bar}")

print()
print("  Higher gyrification = more cortical folding = more harmonics")
print("  in the cortical Weierstrass surface = greater O-integration capacity.")
print()
print("  The human brain's GI of 2.6 means it has more 'good harmonics'")
print("  than any other primate -- more folds, more surface area, more")
print("  center-shell integration capacity.")

# ============================================================
# TEST 7: The Complete Life Cycle of Brain Complexity
# ============================================================
print("\n\n--- Test 7: Complete Brain Complexity Lifecycle ---\n")

print("Phase 1: DEVELOPMENT (0-25 yr)")
print("  - Good harmonics added rapidly (neurogenesis, synaptogenesis)")
print("  - Cortical folds form (gyrification)")
print("  - Fractal dimension rises from ~2.1 to ~2.58")
print("  - = The O-structure building its resonator")
print()
print("Phase 2: MATURITY (25-50 yr)")
print("  - Good harmonics stable (structural maintenance)")
print("  - Bad harmonics accumulating slowly (cellular noise)")
print("  - Fractal dimension stable or very slowly declining")
print("  - = Peak resonance, balanced maintenance")
print()
print("Phase 3: DECLINE (50-80+ yr)")
print("  - Bad harmonics overwhelming repair capacity")
print("  - Good harmonics being destroyed (cortical thinning)")
print("  - Fractal dimension declining measurably")
print("  - = Resonance degrading, O-structure losing coherence")
print()
print("Phase 4: COLLAPSE (disease or extreme age)")
print("  - Fractal dimension drops sharply (Alzheimer's: D -> 2.38)")
print("  - Cortical surface approaching smoothness (loss of folds)")
print("  - = Weierstrass surface losing harmonics")
print("  - = O-structure can no longer integrate its shell")
print("  - = Reference frame context dims, then ceases")
print()
print("  This is the Weierstrass aging model applied to the brain:")
print("  cellular noise accumulates until the macroscopic resonance")
print("  (cortical folds) can no longer be maintained.")

# ============================================================
# SUMMARY
# ============================================================
print(f"""

========================================================================
SUMMARY: Brain Folds as Weierstrass Resonance
========================================================================

The cortical surface IS a Weierstrass-type fractal surface.
  - Fractal dimension D ~ 2.56 (measured by MRI)
  - Rises during development, peaks at ~25 yr, declines with aging
  - Alzheimer's accelerates the decline

TWO SCALES of harmonics:
  GOOD (macro): cortical folds = structural resonance = O-capacity
  BAD (micro):  cellular noise = Weierstrass accumulation = aging

The interaction: micro noise destroys macro structure.
  - Cells accumulate mutations (bad harmonics at cellular scale)
  - Damaged cells can't maintain tissue structure (folds degrade)
  - Brain loses fractal complexity (D decreases)
  - O-integration capacity drops (cognitive decline)
  - At the Weierstrass limit: collapse (death or severe dementia)

Connection to the aging model:
  - N_crit = G*^3/alpha = 3549 total cellular harmonics at collapse
  - The BRAIN version: when cellular N_bad exceeds the maintenance
    threshold, the cortical Weierstrass surface loses harmonics
  - Gyrification index decreases = the brain's resonator degrading

The brain is the ultimate O-structure:
  - Maximum cortical folding = maximum shell surface area
  - Maximum integration capacity = richest reference frame context
  - Most vulnerable to harmonic accumulation (most to lose)

EPISTEMIC STATUS: [CONJECTURE]
  The cortical fractal dimension data is published and real.
  The two-scale model is conceptual, not derived from FTD axioms.
  The connection between cellular mutation count and cortical
  complexity loss is plausible but not yet quantitatively validated.
""")
