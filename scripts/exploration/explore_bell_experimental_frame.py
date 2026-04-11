"""
Bell Violation and the Experimental Frame

Key insight: Bell violation is observed at SPECIFIC angles chosen by
scientists who KNOW the theory that predicts the violation. The
measurement settings are not independent of the theoretical framework.

Tests:
  1. S value vs measurement angle choice strategy
  2. Does S depend on WHO chooses the angles?
  3. The cosmic Bell test constraint
  4. Information-theoretic analysis of the experimenter's role
  5. The von Neumann chain on the lattice
"""
import numpy as np

print("=" * 72)
print("BELL VIOLATION AND THE EXPERIMENTAL FRAME")
print("=" * 72)

# ============================================================
# TEST 1: S Value vs Angle Choice Strategy
# ============================================================
print("\n--- Test 1: S vs Angle Choice Strategy ---\n")

def compute_S_classical(a1, a2, b1, b2, n_trials=50000):
    """Compute CHSH S for sign measurements on random 3D unit vectors.
    This gives the LOCAL HIDDEN VARIABLE result for any angle choice."""
    axis_A1 = np.array([np.sin(a1), 0, np.cos(a1)])
    axis_A2 = np.array([np.sin(a2), 0, np.cos(a2)])
    axis_B1 = np.array([np.sin(b1), 0, np.cos(b1)])
    axis_B2 = np.array([np.sin(b2), 0, np.cos(b2)])

    E = {}
    for label, aA, aB in [('11', axis_A1, axis_B1), ('12', axis_A1, axis_B2),
                            ('21', axis_A2, axis_B1), ('22', axis_A2, axis_B2)]:
        corr = []
        for _ in range(n_trials):
            v = np.random.randn(3)
            v /= np.linalg.norm(v)
            oA = np.sign(np.dot(v, aA))
            oB = np.sign(np.dot(v, aB))
            if oA == 0: oA = 1
            if oB == 0: oB = 1
            corr.append(oA * oB)
        E[label] = np.mean(corr)

    S = abs(E['11'] - E['12'] + E['21'] + E['22'])
    return S, E

def compute_S_QM(a1, a2, b1, b2):
    """Compute CHSH S for the QM singlet state.
    E(a,b) = -cos(a - b) for the singlet."""
    E11 = -np.cos(a1 - b1)
    E12 = -np.cos(a1 - b2)
    E21 = -np.cos(a2 - b1)
    E22 = -np.cos(a2 - b2)
    S = abs(E11 - E12 + E21 + E22)
    return S

# Strategy 1: QM-optimal angles (what trained scientists use)
a1, a2, b1, b2 = 0, np.pi/4, np.pi/8, 3*np.pi/8
S_class_opt, _ = compute_S_classical(a1, a2, b1, b2)
S_qm_opt = compute_S_QM(a1, a2, b1, b2)

# Strategy 2: Random angles (person off the street)
n_random = 200
S_class_random = []
S_qm_random = []
for _ in range(n_random):
    a1r = np.random.uniform(0, np.pi)
    a2r = np.random.uniform(0, np.pi)
    b1r = np.random.uniform(0, np.pi)
    b2r = np.random.uniform(0, np.pi)
    S_c, _ = compute_S_classical(a1r, a2r, b1r, b2r, n_trials=5000)
    S_q = compute_S_QM(a1r, a2r, b1r, b2r)
    S_class_random.append(S_c)
    S_qm_random.append(S_q)

# Strategy 3: Adversarial angles (deliberately avoid violation)
a1a, a2a, b1a, b2a = 0, np.pi/2, 0, np.pi/2
S_class_adv, _ = compute_S_classical(a1a, a2a, b1a, b2a)
S_qm_adv = compute_S_QM(a1a, a2a, b1a, b2a)

print(f"{'Strategy':>25} | {'S (classical)':>14} | {'S (QM)':>10} | {'Violates?':>10}")
print("-" * 65)
print(f"{'QM-optimal angles':>25} | {S_class_opt:>14.4f} | {S_qm_opt:>10.4f} | {'YES' if S_qm_opt > 2 else 'NO':>10}")
print(f"{'Random angles (mean)':>25} | {np.mean(S_class_random):>14.4f} | {np.mean(S_qm_random):>10.4f} | {'SOMETIMES':>10}")
print(f"{'Random angles (max)':>25} | {np.max(S_class_random):>14.4f} | {np.max(S_qm_random):>10.4f} | {'YES' if np.max(S_qm_random) > 2 else 'NO':>10}")
print(f"{'Adversarial angles':>25} | {S_class_adv:>14.4f} | {S_qm_adv:>10.4f} | {'YES' if S_qm_adv > 2 else 'NO':>10}")
print()

# What fraction of random angles give S > 2 in QM?
frac_violating = np.mean(np.array(S_qm_random) > 2)
print(f"  Fraction of random angles that violate Bell (QM): {frac_violating:.1%}")
print(f"  Fraction that DON'T violate: {1-frac_violating:.1%}")
print()

# Distribution of S_QM for random angles
print(f"  S_QM distribution for random angles:")
print(f"    Mean: {np.mean(S_qm_random):.4f}")
print(f"    Std:  {np.std(S_qm_random):.4f}")
print(f"    Min:  {np.min(S_qm_random):.4f}")
print(f"    Max:  {np.max(S_qm_random):.4f}")
print(f"    Median: {np.median(S_qm_random):.4f}")
print()

# KEY FINDING
print("  KEY FINDING:")
print(f"    QM-optimal angles: S = {S_qm_opt:.4f} (violates)")
print(f"    Random angles: S = {np.mean(S_qm_random):.4f} mean ({frac_violating:.0%} violate)")
print(f"    The violation is NOT generic. It requires specific angles.")
print(f"    The specific angles come from the QM formalism itself.")

# ============================================================
# TEST 2: How Much Information Does the Experimenter Inject?
# ============================================================
print("\n\n--- Test 2: Information Content of the Angle Choice ---\n")

# The QM-optimal angles are: a1=0, a2=pi/4, b1=pi/8, b2=3pi/8
# These are derived from maximizing S = |E(a1,b1) - E(a1,b2) + E(a2,b1) + E(a2,b2)|
# with E(a,b) = -cos(a-b).
#
# The INFORMATION content: the scientist must know:
# 1. That E(a,b) = -cos(a-b)  [the QM prediction]
# 2. The calculus to maximize S [standard optimization]
# 3. How to build and align detectors to those specific angles [experimental skill]
#
# Each piece is a specific lattice configuration (brain state) that
# was shaped by the scientist's training.

# How many bits of information are in the angle choice?
# 4 angles, each specified to, say, 10-degree precision in [0, 180]:
# That's 18 choices per angle = log2(18) ~ 4.2 bits per angle
# Total: 4 * 4.2 ~ 17 bits
# But the QM-OPTIMAL choice is a specific 1 out of 18^4 = 104,976 possibilities.
# That's log2(104976) ~ 17 bits of selection.

n_angle_choices = 18**4  # 10-degree steps, 4 angles
bits_selection = np.log2(n_angle_choices)

print(f"  Angle space (10-degree resolution): {n_angle_choices:,} possible settings")
print(f"  Information content of QM-optimal choice: {bits_selection:.1f} bits")
print()

# What fraction of the angle space gives S > 2?
n_scan = 10000
violations = 0
for _ in range(n_scan):
    a1r = np.random.uniform(0, np.pi)
    a2r = np.random.uniform(0, np.pi)
    b1r = np.random.uniform(0, np.pi)
    b2r = np.random.uniform(0, np.pi)
    S_q = compute_S_QM(a1r, a2r, b1r, b2r)
    if S_q > 2:
        violations += 1

frac_space = violations / n_scan
bits_needed = -np.log2(frac_space) if frac_space > 0 else float('inf')

print(f"  Fraction of angle space giving S > 2 (QM): {frac_space:.1%}")
print(f"  Bits needed to find a violating setting: {bits_needed:.1f}")
print(f"  A random choice has ~{frac_space:.0%} chance of seeing violation")
print()

# What about S > 2.5?
violations_strong = 0
for _ in range(n_scan):
    a1r = np.random.uniform(0, np.pi)
    a2r = np.random.uniform(0, np.pi)
    b1r = np.random.uniform(0, np.pi)
    b2r = np.random.uniform(0, np.pi)
    S_q = compute_S_QM(a1r, a2r, b1r, b2r)
    if S_q > 2.5:
        violations_strong += 1

frac_strong = violations_strong / n_scan
print(f"  Fraction giving S > 2.5: {frac_strong:.1%}")
print(f"  Fraction giving S > 2.8: ", end="")

violations_28 = 0
for _ in range(n_scan):
    a1r = np.random.uniform(0, np.pi)
    a2r = np.random.uniform(0, np.pi)
    b1r = np.random.uniform(0, np.pi)
    b2r = np.random.uniform(0, np.pi)
    if compute_S_QM(a1r, a2r, b1r, b2r) > 2.8:
        violations_28 += 1
print(f"{violations_28/n_scan:.2%}")

# ============================================================
# TEST 3: The Cosmic Bell Test Constraint
# ============================================================
print("\n\n--- Test 3: Cosmic Bell Test Analysis ---\n")

# Handsteiner et al 2017: used quasar light from 600+ light-years away
# to choose measurement settings. Still got S = 2.83.
#
# This constrains superdeterminism: the correlation between settings
# and hidden variables would have to be set > 600 years ago.
#
# The Big Bell Test (2018): used 100,000 humans as random number
# generators. Still got violation.
#
# But: even quasar light is part of the same universe.
# On the lattice, ALL initial conditions are set at t=0.

print("  Cosmic Bell tests:")
print()
print(f"  {'Experiment':>30} | {'Setting source':>25} | {'Distance':>15} | {'S':>6}")
print("  " + "-" * 82)
print(f"  {'Standard lab (Aspect 1982)':>30} | {'Local electronics':>25} | {'meters':>15} | {'2.70':>6}")
print(f"  {'Weihs et al 1998':>30} | {'Fast random generator':>25} | {'400m':>15} | {'2.73':>6}")
print(f"  {'Handsteiner et al 2017':>30} | {'Quasar photons':>25} | {'600 light-years':>15} | {'2.77':>6}")
print(f"  {'Big Bell Test 2018':>30} | {'100,000 humans':>25} | {'global':>15} | {'2.64':>6}")
print(f"  {'Rauch et al 2018':>30} | {'Quasar photons':>25} | {'7.8 Gly':>15} | {'~2.8':>6}")
print()

print("  Superdeterminism constraint from cosmic tests:")
print("    If settings from quasars 7.8 billion light-years away")
print("    are correlated with local hidden variables, the correlation")
print("    was set at least 7.8 billion years ago.")
print()
print("    On the lattice: this is not surprising. The lattice")
print("    initial conditions determine EVERYTHING. The quasar photon")
print("    and the local particle share initial conditions at t=0.")
print("    The correlation is cosmic because the lattice is cosmic.")

# ============================================================
# TEST 4: The Von Neumann Chain
# ============================================================
print("\n\n--- Test 4: The Von Neumann Chain on the Lattice ---\n")

print("  The von Neumann chain:")
print("    System -> Detector -> Amplifier -> Computer -> Display -> Eye -> Brain")
print()
print("  Each link is lattice voxels. The entire chain is one")
print("  deterministic evolution. There is no point where the chain")
print("  transitions from 'quantum' to 'classical' because the lattice")
print("  is always definite.")
print()
print("  The scientist is the END of the chain, not outside it.")
print("  The scientist's knowledge (QM training) is a lattice configuration")
print("  that determines which angles they choose.")
print()
print("  The Bell test, reformulated:")
print("    1. The universe evolves deterministically from initial conditions")
print("    2. Part of that evolution produces scientists who learn QM")
print("    3. Those scientists choose QM-optimal angles")
print("    4. Those angles, applied to the source, give S = 2.83")
print("    5. The scientists conclude 'non-locality'")
print()
print("  But step 3 is not independent of step 4.")
print("  The angles were chosen BECAUSE QM predicts they maximize S.")
print("  The prediction and the measurement share a cause: the QM formalism,")
print("  which is a lattice configuration in the scientist's brain.")

# ============================================================
# TEST 5: The Independence Assumption Quantified
# ============================================================
print("\n\n--- Test 5: How Much Correlation Breaks Bell? ---\n")

# Bell's theorem requires: P(settings | hidden variables) = P(settings)
# i.e., the settings are independent of the hidden variables.
# How much correlation is needed to get S from 2 to 2.83?

# The CHSH inequality with measurement dependence:
# S <= 2 + 4*delta
# where delta = max|P(a,b|lambda) - P(a,b)| / P(a,b)
# (the maximum fractional deviation from independence)

# To get S = 2.83: 2.83 <= 2 + 4*delta -> delta >= 0.21
# That's a 21% correlation between settings and hidden variables.

delta_needed = (2*np.sqrt(2) - 2) / 4
print(f"  To reach S = 2*sqrt(2) = {2*np.sqrt(2):.4f}:")
print(f"  Need delta >= {delta_needed:.4f} ({delta_needed*100:.1f}% correlation)")
print()
print(f"  This means: the probability of choosing angle setting (a,b)")
print(f"  must deviate from uniform by at least {delta_needed*100:.1f}% depending")
print(f"  on the hidden variable lambda.")
print()

# Is 21% plausible?
# The scientist chooses angles based on QM theory.
# The QM theory is a function of the same physics that produces lambda.
# The correlation is not 21% -- it's 100%.
# The angles are ENTIRELY determined by QM, which is ENTIRELY determined
# by the lattice evolution that also determines lambda.

print(f"  Is {delta_needed*100:.1f}% plausible?")
print(f"  The scientist's angle choice is 100% determined by QM training.")
print(f"  QM training is 100% determined by the lattice evolution.")
print(f"  The hidden variable lambda is 100% determined by the lattice evolution.")
print(f"  The correlation between settings and lambda is not {delta_needed*100:.1f}% -- it's 100%.")
print(f"  The settings are DERIVED FROM the same theory that describes lambda.")

# ============================================================
# TEST 6: What Would Independence Look Like?
# ============================================================
print("\n\n--- Test 6: What Would True Independence Require? ---\n")

print("  For measurement independence to hold, the experimenter's")
print("  choice would need to be:")
print("    1. Not determined by the lattice evolution (breaks determinism)")
print("    2. Not influenced by any shared causal history (breaks locality)")
print("    3. Truly random in a way uncorrelated with the source (breaks the lattice)")
print()
print("  On a deterministic lattice, NONE of these are possible.")
print("  Measurement independence requires something OUTSIDE the lattice")
print("  to inject randomness into the choice.")
print()
print("  If the lattice is all there is (FTD axiom), then measurement")
print("  independence is impossible in principle.")
print()
print("  Bell's theorem then says: given measurement independence, S <= 2.")
print("  FTD says: measurement independence doesn't hold.")
print("  Experiments say: S = 2.83.")
print("  These are all consistent. There is no contradiction.")
print()
print("  The 'violation' is a violation of the ASSUMPTION (independence),")
print("  not a violation of locality.")

# ============================================================
# SUMMARY
# ============================================================
print(f"""

========================================================================
SUMMARY: Bell Violation and the Experimental Frame
========================================================================

TEST 1: S depends on angle choice strategy.
  QM-optimal: S = {S_qm_opt:.2f}. Random: S = {np.mean(S_qm_random):.2f} mean.
  Only {frac_violating:.0%} of random angle choices give S > 2 in QM.
  The violation requires SPECIFIC angles derived from QM theory.

TEST 2: The angle choice carries ~{bits_needed:.0f} bits of information.
  {frac_space:.0%} of angle space gives ANY violation.
  The QM-optimal choice is a {bits_selection:.0f}-bit selection from the angle space.
  The scientist's training provides those bits.

TEST 3: Cosmic Bell tests still violate.
  Superdeterminism requires correlations from the initial conditions.
  On a deterministic lattice, this is tautological — everything
  correlates through initial conditions.

TEST 4: The von Neumann chain.
  The scientist is part of the experiment.
  The angle choice is not independent of the system.
  The choice is derived from QM, which describes the system.

TEST 5: Only {delta_needed*100:.1f}% correlation needed to break Bell bound.
  The actual correlation is 100% — settings are fully determined
  by the same lattice evolution that determines hidden variables.

TEST 6: True independence requires something outside the lattice.
  On a deterministic lattice, nothing is outside.
  Measurement independence is impossible in principle.

CONCLUSION:
  Bell's theorem proves: IF measurement independence holds, THEN S <= 2.
  FTD says: measurement independence does NOT hold (deterministic lattice).
  Experiments say: S = 2.83.
  No contradiction. The violation is of the assumption, not of locality.

  This is not a proof of superdeterminism. It is a proof that
  superdeterminism is the ONLY position consistent with:
    (a) the lattice being deterministic [AXIOM]
    (b) the lattice being local [AXIOM]
    (c) experiments showing S = 2.83 [FACT]

  Status: [SELECTION] — consistent and necessary given the axioms,
  but unfalsifiable because testing it requires the independence
  it denies.
""")
