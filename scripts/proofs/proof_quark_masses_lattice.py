"""
QUARK MASS RATIOS FROM LATTICE INTEGERS — HONEST EXPLORATION

Lepton mass ratios emerge from clean integer arithmetic on {3, 4, 7, 13}:
  m_mu/m_e  = 3*B_3*(B_3+N_C) - N_C             = 3*7*10 - 3    = 207
  m_tau/m_e = (N_EFF+N_BASE)*MU_RATIO - 2*N_C*B_3 = 17*207 - 42  = 3477

This script asks: do quark mass ratios emerge from the SAME integer set
using structurally analogous combinations?

Approach:
  1. Verify lepton formulas as a baseline (these are proven)
  2. Enumerate structurally motivated quark ratio candidates — only formulas
     that parallel the lepton pattern (products/sums of framework integers)
  3. Compare candidates with experimental quark mass ratios
  4. Honestly report what works and what doesn't

What this proves:
  [THEOREM]   Lepton mass ratios reproduce from integer arithmetic (baseline)
  [THEOREM]   Framework integers and their basic combinations are computed correctly
  [OPEN]      Whether quark mass ratios emerge from the same integer patterns
"""

import sys
import os
import io
import math

if sys.stdout.encoding != 'utf-8':
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import (
    ProofSuite, N_C, N_BASE, B_3, N_EFF, N_GEN, N_F,
    ALPHA, X_MINUS, G_STAR, G_N, SIN2_WEINBERG,
    MU_RATIO, TAU_RATIO,
    MACHINE_EPS, PERCENT_1, PERCENT_5, PERCENT_10, PERCENT_15,
)

suite = ProofSuite("Quark Mass Ratios from Lattice Integers")

print("=" * 78)
print("  QUARK MASS RATIOS FROM LATTICE INTEGERS -- HONEST EXPLORATION")
print("=" * 78)
print()


# ============================================================================
# EXPERIMENTAL QUARK MASSES (PDG 2024, MS-bar at 2 GeV)
# ============================================================================

# Current quark masses in MeV (MS-bar at mu = 2 GeV)
EXP_M_U = 2.16    # +0.49 -0.26 MeV
EXP_M_D = 4.67    # +0.48 -0.17 MeV
EXP_M_S = 93.4    # +8.6  -3.4  MeV
EXP_M_C = 1270.0  # +20   -20   MeV (at m_c scale, conventional)
EXP_M_B = 4180.0  # +30   -20   MeV (at m_b scale, conventional)
EXP_M_T = 172760.0  # +300 MeV (pole mass)

# Experimental mass ratios (the quantities we want to explain)
EXP_MU_MD = EXP_M_U / EXP_M_D      # ~ 0.463
EXP_MS_MD = EXP_M_S / EXP_M_D      # ~ 20.0
EXP_MC_MS = EXP_M_C / EXP_M_S      # ~ 13.6
EXP_MB_MC = EXP_M_B / EXP_M_C      # ~ 3.29
EXP_MT_MB = EXP_M_T / EXP_M_B      # ~ 41.3
EXP_MC_MD = EXP_M_C / EXP_M_D      # ~ 271.9
EXP_MT_MU = EXP_M_T / EXP_M_U      # ~ 79981

# Electron mass for absolute scale
EXP_M_E = 0.511  # MeV

print("  Experimental quark masses (PDG 2024, MS-bar at 2 GeV):")
print(f"    m_u = {EXP_M_U} MeV      m_d = {EXP_M_D} MeV")
print(f"    m_s = {EXP_M_S} MeV     m_c = {EXP_M_C} MeV")
print(f"    m_b = {EXP_M_B} MeV    m_t = {EXP_M_T} MeV")
print()
print("  Experimental mass ratios:")
print(f"    m_u/m_d = {EXP_MU_MD:.3f}")
print(f"    m_s/m_d = {EXP_MS_MD:.1f}")
print(f"    m_c/m_s = {EXP_MC_MS:.1f}")
print(f"    m_b/m_c = {EXP_MB_MC:.2f}")
print(f"    m_t/m_b = {EXP_MT_MB:.1f}")
print()


# ============================================================================
# SECTION 1: LEPTON BASELINE (VERIFIED)
# ============================================================================

print("=" * 78)
print("  SECTION 1: Lepton Mass Ratios -- Baseline [THEOREM]")
print("=" * 78)
print()
print("  These are the proven lepton formulas that define the pattern we seek")
print("  to extend to quarks.")
print()

# Lepton formulas
mu_ratio_calc = 3 * B_3 * (B_3 + N_C) - N_C
tau_ratio_calc = (N_EFF + N_BASE) * MU_RATIO - 2 * N_C * B_3

print(f"  m_mu/m_e = 3*B_3*(B_3+N_C) - N_C")
print(f"           = 3*{B_3}*({B_3}+{N_C}) - {N_C}")
print(f"           = 3*{B_3}*{B_3+N_C} - {N_C}")
print(f"           = {3*B_3*(B_3+N_C)} - {N_C} = {mu_ratio_calc}")
print(f"  Experimental: 206.77  (error: {abs(mu_ratio_calc - 206.77)/206.77*100:.2f}%)")
print()
print(f"  m_tau/m_e = (N_EFF+N_BASE)*MU_RATIO - 2*N_C*B_3")
print(f"            = ({N_EFF}+{N_BASE})*{MU_RATIO} - 2*{N_C}*{B_3}")
print(f"            = {N_EFF+N_BASE}*{MU_RATIO} - {2*N_C*B_3}")
print(f"            = {(N_EFF+N_BASE)*MU_RATIO} - {2*N_C*B_3} = {tau_ratio_calc}")
print(f"  Experimental: 3477.2  (error: {abs(tau_ratio_calc - 3477.2)/3477.2*100:.3f}%)")
print()

suite.assert_equal(
    "mu_ratio = 3*B_3*(B_3+N_C) - N_C = 207",
    float(mu_ratio_calc), 207.0,
    tag="[THEOREM]"
)

suite.assert_equal(
    "tau_ratio = (N_EFF+N_BASE)*MU_RATIO - 2*N_C*B_3 = 3477",
    float(tau_ratio_calc), 3477.0,
    tag="[THEOREM]"
)

suite.assert_close(
    "mu_ratio vs experiment (206.77)",
    float(mu_ratio_calc), 206.77,
    tol=PERCENT_1,
    tag="[THEOREM]"
)

suite.assert_close(
    "tau_ratio vs experiment (3477.2)",
    float(tau_ratio_calc), 3477.2,
    tol=PERCENT_1,
    tag="[THEOREM]"
)


# ============================================================================
# SECTION 2: FRAMEWORK INTEGER CATALOG
# ============================================================================

print("=" * 78)
print("  SECTION 2: Framework Integer Catalog [THEOREM]")
print("=" * 78)
print()
print("  The available framework integers (all derived from D=3 + varpi):")
print(f"    N_C    = {N_C}    (color number, floor of x_-)")
print(f"    N_BASE = {N_BASE}    (2^((D+1)//2))")
print(f"    B_3    = {B_3}    (one-loop beta coefficient)")
print(f"    N_EFF  = {N_EFF}   (b_3 + 2*N_c)")
print(f"    N_GEN  = {N_GEN}    (= N_c)")
print(f"    N_F    = {N_F}    (= 2*N_gen)")
print()
print("  Key derived products appearing in lepton formulas:")
print(f"    B_3 + N_C       = {B_3 + N_C}")
print(f"    N_EFF + N_BASE  = {N_EFF + N_BASE}")
print(f"    2*N_C*B_3       = {2*N_C*B_3}")
print(f"    3*B_3           = {3*B_3}")
print(f"    B_3*(B_3+N_C)   = {B_3*(B_3+N_C)}")
print()

# Verify framework integer relationships
suite.assert_equal("N_C = 3", float(N_C), 3.0, tag="[THEOREM]")
suite.assert_equal("N_BASE = 4", float(N_BASE), 4.0, tag="[THEOREM]")
suite.assert_equal("B_3 = 7", float(B_3), 7.0, tag="[THEOREM]")
suite.assert_equal("N_EFF = 13", float(N_EFF), 13.0, tag="[THEOREM]")
suite.assert_equal("N_EFF = B_3 + 2*N_C", float(N_EFF), float(B_3 + 2*N_C), tag="[THEOREM]")
suite.assert_equal("N_F = 2*N_GEN = 6", float(N_F), 6.0, tag="[THEOREM]")


# ============================================================================
# SECTION 3: STRUCTURAL ANALOGY — QUARK MASS RATIO CANDIDATES
# ============================================================================

print("=" * 78)
print("  SECTION 3: Structurally Motivated Quark Ratio Candidates")
print("=" * 78)
print()
print("  Methodology: The lepton formulas have a clear structure:")
print("    - Products of 2-3 framework integers")
print("    - Small additive/subtractive corrections from the same set")
print("    - No free parameters, no fitting")
print()
print("  For quarks, we test the SAME structural patterns.")
print("  Quarks differ from leptons in carrying color charge (N_C = 3).")
print("  A natural hypothesis: quark formulas involve the same integers")
print("  but with color-dependent prefactors or combinatorial factors.")
print()

# --------------------------------------------------------------------------
# 3a: The lepton pattern uses bilinear products of framework integers.
#     We enumerate all such products and their experimental proximity
#     to quark mass ratios. This is NOT fitting — we compute everything
#     and check if any known ratio appears.
# --------------------------------------------------------------------------

print("  --- 3a: Bilinear products of framework integers ---")
print()

integers = {"N_C": N_C, "N_BASE": N_BASE, "B_3": B_3, "N_EFF": N_EFF, "N_F": N_F}
int_names = list(integers.keys())
int_vals = list(integers.values())

# All products a*b for framework integers (including a*a)
products = {}
for i, (n1, v1) in enumerate(integers.items()):
    for j, (n2, v2) in enumerate(integers.items()):
        if j >= i:
            key = f"{n1}*{n2}" if i != j else f"{n1}^2"
            products[key] = v1 * v2

# Also simple ratios a/b
ratios = {}
for n1, v1 in integers.items():
    for n2, v2 in integers.items():
        if v1 != v2:
            key = f"{n1}/{n2}"
            ratios[key] = v1 / v2

print("  Products:")
for name, val in sorted(products.items(), key=lambda x: x[1]):
    print(f"    {name:20s} = {val}")
print()
print("  Ratios:")
for name, val in sorted(ratios.items(), key=lambda x: x[1]):
    print(f"    {name:20s} = {val:.4f}")
print()

# --------------------------------------------------------------------------
# 3b: Check specific quark mass ratios against structurally motivated
#     combinations that parallel the lepton pattern.
# --------------------------------------------------------------------------

print("  --- 3b: Specific quark ratio candidates ---")
print()

# The lepton mu ratio has the form: N_C * B_3 * (B_3+N_C) - N_C
# i.e., a triple product minus a correction.
# Generalization: a * b * c +/- d for framework integers.

# For m_u/m_d ~ 0.47: Could this be a simple integer ratio?
# N_C/B_3 = 3/7 = 0.4286
# N_BASE/B_3 = 4/7 = 0.5714
# N_C/(N_F+1) = 3/7 = 0.4286
# N_C/N_F = 3/6 = 0.5

candidate_mu_md = [
    ("N_C/B_3", N_C/B_3),
    ("N_BASE/B_3", N_BASE/B_3),
    ("N_C/N_F", N_C/N_F),
    ("N_BASE/N_EFF", N_BASE/N_EFF),
    ("(N_C+1)/(B_3+1)", (N_C+1)/(B_3+1)),  # 4/8 = 0.5 (but +1 is not structural)
]

print(f"  m_u/m_d experimental: {EXP_MU_MD:.3f}")
print()
for name, val in candidate_mu_md:
    err = abs(val - EXP_MU_MD) / EXP_MU_MD * 100
    print(f"    {name:20s} = {val:.4f}  (error: {err:.1f}%)")
print()

# For m_s/m_d ~ 20:
# B_3*N_C = 21 (close!)
# N_EFF + B_3 = 20 (exact!)
# 4*N_C + N_EFF - N_BASE = 12 + 13 - 4 = 21
# N_BASE * N_C + N_EFF - N_F = 12 + 13 - 6 = 19

candidate_ms_md = [
    ("B_3*N_C", B_3*N_C),
    ("N_EFF + B_3", N_EFF + B_3),
    ("N_EFF + N_BASE + N_C", N_EFF + N_BASE + N_C),
    ("N_BASE*N_C + N_EFF - N_F", N_BASE*N_C + N_EFF - N_F),
]

print(f"  m_s/m_d experimental: {EXP_MS_MD:.1f}")
print()
for name, val in candidate_ms_md:
    err = abs(val - EXP_MS_MD) / EXP_MS_MD * 100
    print(f"    {name:20s} = {val:.4f}  (error: {err:.1f}%)")
print()

# For m_c/m_s ~ 13.6:
# N_EFF = 13 (close!)
# N_EFF + 1 = 14 (but +1 is not structural)
# (B_3+N_C)*N_C/N_BASE + N_C = 30/4 + 3 = 10.5 (no)

candidate_mc_ms = [
    ("N_EFF", float(N_EFF)),
    ("B_3 + N_F", float(B_3 + N_F)),
    ("N_EFF + N_C/N_BASE", N_EFF + N_C/N_BASE),
    ("N_BASE*N_C + N_C", float(N_BASE*N_C + N_C)),
]

print(f"  m_c/m_s experimental: {EXP_MC_MS:.1f}")
print()
for name, val in candidate_mc_ms:
    err = abs(val - EXP_MC_MS) / EXP_MC_MS * 100
    print(f"    {name:20s} = {val:.4f}  (error: {err:.1f}%)")
print()

# For m_b/m_c ~ 3.29:
# N_C = 3 (close!)
# X_MINUS = 3.024 (close!)

candidate_mb_mc = [
    ("N_C", float(N_C)),
    ("x_minus", X_MINUS),
    ("N_EFF/N_BASE", N_EFF/N_BASE),
    ("B_3/N_C + N_C/B_3", B_3/N_C + N_C/B_3),
]

print(f"  m_b/m_c experimental: {EXP_MB_MC:.2f}")
print()
for name, val in candidate_mb_mc:
    err = abs(val - EXP_MB_MC) / EXP_MB_MC * 100
    print(f"    {name:20s} = {val:.4f}  (error: {err:.1f}%)")
print()

# For m_t/m_b ~ 41.3:
# N_C*N_EFF + N_C = 42 (close!)
# 3*(N_EFF+1) = 42
# N_EFF*N_C = 39
# N_F*B_3 = 42

candidate_mt_mb = [
    ("N_F*B_3", float(N_F*B_3)),
    ("N_C*N_EFF + N_C", float(N_C*N_EFF + N_C)),
    ("N_C*(N_EFF+1)", float(N_C*(N_EFF+1))),
    ("N_EFF*N_C", float(N_EFF*N_C)),
    ("B_3*N_F - N_C + N_C", float(B_3*N_F)),
]

print(f"  m_t/m_b experimental: {EXP_MT_MB:.1f}")
print()
for name, val in candidate_mt_mb:
    err = abs(val - EXP_MT_MB) / EXP_MT_MB * 100
    print(f"    {name:20s} = {val:.4f}  (error: {err:.1f}%)")
print()


# ============================================================================
# SECTION 4: ASSESSMENT — WHAT SURVIVES SCRUTINY?
# ============================================================================

print("=" * 78)
print("  SECTION 4: Honest Assessment")
print("=" * 78)
print()

# Identify the best candidate for each ratio and assess honestly

# m_u/m_d: N_C/B_3 = 3/7 = 0.4286 vs 0.463 -- 7.4% off
# This is a simple ratio of two framework integers, structurally clean.
# But 7.4% is large, and quark mass ratios have ~10-20% uncertainty anyway.
mu_md_best = N_C / B_3
mu_md_err = abs(mu_md_best - EXP_MU_MD) / EXP_MU_MD

print("  m_u/m_d:")
print(f"    Best candidate: N_C/B_3 = 3/7 = {mu_md_best:.4f}")
print(f"    Experimental: {EXP_MU_MD:.3f} (uncertainty ~15%)")
print(f"    Error: {mu_md_err*100:.1f}%")
print(f"    Assessment: Within experimental uncertainty, but NOT compelling.")
print(f"    The large experimental uncertainty on light quark masses makes")
print(f"    this ratio poorly constrained. N_C/B_3 is structurally motivated")
print(f"    (color/beta ratio) but not uniquely selected.")
print()

suite.assert_close(
    "m_u/m_d ~ N_C/B_3 = 3/7 [CONJECTURE]",
    mu_md_best, EXP_MU_MD,
    tol=PERCENT_15,
    tag="[CONJECTURE]"
)

# m_s/m_d: N_EFF + B_3 = 20 vs 20.0 -- suspiciously close
# But is N_EFF + B_3 = 13 + 7 structurally motivated?
# It equals B_3 + (B_3 + 2*N_C) = 2*B_3 + 2*N_C = 2*(B_3+N_C) = 20.
# This is just 2*(B_3+N_C), which IS a clean structural expression.
ms_md_best = float(N_EFF + B_3)
ms_md_err = abs(ms_md_best - EXP_MS_MD) / EXP_MS_MD

print("  m_s/m_d:")
print(f"    Best candidate: N_EFF + B_3 = 2*(B_3+N_C) = {int(ms_md_best)}")
print(f"    Experimental: {EXP_MS_MD:.1f} (uncertainty ~15%)")
print(f"    Error: {ms_md_err*100:.1f}%")
print(f"    Assessment: The identity N_EFF + B_3 = 2*(B_3+N_C) = 20 is")
print(f"    structurally clean. However, the experimental value has large")
print(f"    uncertainty (~15%), so this match is suggestive but not definitive.")
print()

suite.assert_close(
    "m_s/m_d ~ 2*(B_3+N_C) = 20 [CONJECTURE]",
    ms_md_best, EXP_MS_MD,
    tol=PERCENT_15,
    tag="[CONJECTURE]"
)

# m_c/m_s: N_EFF = 13 vs 13.6 -- decent
# B_3 + N_F = 13 also -- same value, different combination
mc_ms_best = float(N_EFF)
mc_ms_err = abs(mc_ms_best - EXP_MC_MS) / EXP_MC_MS

print("  m_c/m_s:")
print(f"    Best candidate: N_EFF = {N_EFF}")
print(f"    Experimental: {EXP_MC_MS:.1f} (uncertainty ~10%)")
print(f"    Error: {mc_ms_err*100:.1f}%")
print(f"    Assessment: N_EFF = 13 is structurally central (total rotation")
print(f"    axes, effective beta parameter). The 4.4% error is within")
print(f"    experimental+renormalization uncertainty but not a clean match.")
print()

suite.assert_close(
    "m_c/m_s ~ N_EFF = 13 [CONJECTURE]",
    mc_ms_best, EXP_MC_MS,
    tol=PERCENT_10,
    tag="[CONJECTURE]"
)

# m_b/m_c: N_EFF/N_BASE = 13/4 = 3.25 vs 3.29 -- good
# Also X_MINUS = 3.024 vs 3.29 -- worse
mb_mc_best = N_EFF / N_BASE
mb_mc_err = abs(mb_mc_best - EXP_MB_MC) / EXP_MB_MC

print("  m_b/m_c:")
print(f"    Best candidate: N_EFF/N_BASE = 13/4 = {mb_mc_best:.4f}")
print(f"    Experimental: {EXP_MB_MC:.2f} (uncertainty ~2%)")
print(f"    Error: {mb_mc_err*100:.1f}%")
print(f"    Assessment: 13/4 is a clean ratio of framework integers.")
print(f"    The 1.2% error is small, but m_b/m_c is one of the better-known")
print(f"    ratios, so this should be taken seriously. However, a ratio of")
print(f"    the two most prominent integers could be coincidental.")
print()

suite.assert_close(
    "m_b/m_c ~ N_EFF/N_BASE = 13/4 [CONJECTURE]",
    mb_mc_best, EXP_MB_MC,
    tol=PERCENT_5,
    tag="[CONJECTURE]"
)

# m_t/m_b: N_F*B_3 = 42 vs 41.3 -- decent
# Also N_C*(N_EFF+1) = 42 -- but "+1" is not purely structural
# N_F*B_3 = 2*N_GEN*B_3 = 2*N_C*B_3 = 42 IS structural (it's the same
# correction term appearing in the tau formula!)
mt_mb_best = float(N_F * B_3)
mt_mb_err = abs(mt_mb_best - EXP_MT_MB) / EXP_MT_MB

print("  m_t/m_b:")
print(f"    Best candidate: N_F*B_3 = 2*N_C*B_3 = {int(mt_mb_best)}")
print(f"    Experimental: {EXP_MT_MB:.1f} (uncertainty ~1%)")
print(f"    Error: {mt_mb_err*100:.1f}%")
print(f"    Assessment: 2*N_C*B_3 = 42 is the SAME combination appearing")
print(f"    as the correction term in the tau/electron formula. This is the")
print(f"    most structurally motivated candidate. The 1.7% error is notable")
print(f"    given the small experimental uncertainty on m_t/m_b.")
print()

suite.assert_close(
    "m_t/m_b ~ 2*N_C*B_3 = 42 [CONJECTURE]",
    mt_mb_best, EXP_MT_MB,
    tol=PERCENT_5,
    tag="[CONJECTURE]"
)


# ============================================================================
# SECTION 5: CONSISTENCY CHECK — CHAINED RATIOS
# ============================================================================

print("=" * 78)
print("  SECTION 5: Chained Ratio Consistency Check")
print("=" * 78)
print()
print("  If the individual ratio candidates were all correct, the chained")
print("  product should reconstruct the full mass hierarchy. Check:")
print()

# If m_u/m_d = 3/7, m_s/m_d = 20, m_c/m_s = 13, m_b/m_c = 13/4, m_t/m_b = 42:
# Then m_t/m_u = (m_t/m_b)*(m_b/m_c)*(m_c/m_s)*(m_s/m_d)*(m_d/m_u)
#              = 42 * (13/4) * 13 * 20 * (7/3)
#              = 42 * 3.25 * 13 * 20 * 2.333...
#              = 42 * 3.25 * 13 * 46.667
#              = 82,810

chain_mt_mu = mt_mb_best * mb_mc_best * mc_ms_best * ms_md_best / mu_md_best
exp_mt_mu = EXP_M_T / EXP_M_U
chain_err = abs(chain_mt_mu - exp_mt_mu) / exp_mt_mu

print(f"  Chained m_t/m_u = {mt_mb_best:.2f} * {mb_mc_best:.4f} * {mc_ms_best:.0f} * {ms_md_best:.0f} / {mu_md_best:.4f}")
print(f"                  = {chain_mt_mu:.0f}")
print(f"  Experimental m_t/m_u = {exp_mt_mu:.0f}")
print(f"  Error: {chain_err*100:.1f}%")
print()

if chain_err < 0.05:
    print("  Chained ratios are self-consistent within 5%.")
else:
    print("  Chained ratios show non-trivial accumulated error.")
    print("  This suggests individual candidates are not all simultaneously correct.")
print()

suite.assert_close(
    "Chained m_t/m_u consistency",
    chain_mt_mu, exp_mt_mu,
    tol=PERCENT_10,
    tag="[CONJECTURE]"
)


# ============================================================================
# SECTION 6: ABSOLUTE QUARK MASSES FROM ELECTRON MASS
# ============================================================================

print("=" * 78)
print("  SECTION 6: Absolute Scale Check [CONJECTURE]")
print("=" * 78)
print()
print("  The lepton formulas give masses relative to m_e = 0.511 MeV.")
print("  For quarks, we need an anchor. The lightest quark (m_u ~ 2.2 MeV)")
print("  has m_u/m_e ~ 4.3. Can this ratio be expressed in framework integers?")
print()

exp_mu_me = EXP_M_U / EXP_M_E  # ~ 4.3

candidate_mu_me = [
    ("N_BASE", float(N_BASE)),                    # 4
    ("N_C + 1", float(N_C + 1)),                  # 4 (but +1 is ad hoc)
    ("N_BASE + N_C/B_3", N_BASE + N_C/B_3),       # 4.429
    ("B_3 - N_C", float(B_3 - N_C)),              # 4
    ("2*N_C - N_GEN + N_BASE", float(2*N_C - N_GEN + N_BASE)),  # 7
]

print(f"  m_u/m_e experimental: {exp_mu_me:.2f}")
print()
for name, val in candidate_mu_me:
    err = abs(val - exp_mu_me) / exp_mu_me * 100
    print(f"    {name:30s} = {val:.4f}  (error: {err:.1f}%)")
print()

# N_BASE = 4 is close to 4.3 but 7% off.
# B_3 - N_C = 4 likewise.
# None are compelling.

mu_me_best = float(N_BASE)
mu_me_err = abs(mu_me_best - exp_mu_me) / exp_mu_me

print(f"  Best candidate: N_BASE = {N_BASE} vs {exp_mu_me:.2f} ({mu_me_err*100:.1f}% error)")
print(f"  Assessment: Not a clean match. The quark-to-electron mass anchor")
print(f"  does not emerge naturally from framework integers alone.")
print()

# Also check m_d/m_e ~ 9.1
exp_md_me = EXP_M_D / EXP_M_E  # ~ 9.1
print(f"  m_d/m_e experimental: {exp_md_me:.2f}")
print(f"    B_3 + N_C = {B_3 + N_C}  (error: {abs(B_3+N_C - exp_md_me)/exp_md_me*100:.1f}%)")
print(f"    N_EFF - N_BASE = {N_EFF - N_BASE}  (error: {abs(N_EFF-N_BASE - exp_md_me)/exp_md_me*100:.1f}%)")
print()
print("  Neither is compelling. The quark-electron mass bridge is [OPEN].")
print()


# ============================================================================
# SECTION 7: BRUTALLY HONEST SUMMARY
# ============================================================================

print("=" * 78)
print("  SECTION 7: Brutally Honest Summary")
print("=" * 78)
print()
print("  WHAT WAS FOUND:")
print()
print("  Of the five quark mass ratios tested, all candidates are [CONJECTURE].")
print("  None achieve the clean, exact integer arithmetic of the lepton formulas.")
print()
print("  Ranking by structural motivation:")
print()
print("    1. m_t/m_b ~ 2*N_C*B_3 = 42  (1.7% error)")
print("       STRONGEST: same combination as tau formula correction term.")
print("       But error exceeds experimental precision.")
print()
print("    2. m_b/m_c ~ N_EFF/N_BASE = 13/4  (1.2% error)")
print("       Clean ratio of two central integers.")
print("       But could easily be coincidence in a small integer set.")
print()
print("    3. m_s/m_d ~ 2*(B_3+N_C) = 20  (0.0% central value)")
print("       Clean structural expression.")
print("       But experimental uncertainty is ~15%, so this is weakly constrained.")
print()
print("    4. m_c/m_s ~ N_EFF = 13  (4.4% error)")
print("       N_EFF is the most prominent framework integer.")
print("       Moderate error; could be coincidence.")
print()
print("    5. m_u/m_d ~ N_C/B_3 = 3/7  (7.4% error)")
print("       Simple ratio, but 7.4% error is not small even given uncertainties.")
print()
print("  WHAT WAS NOT FOUND:")
print()
print("    - No quark ratio emerges as EXACT integer arithmetic")
print("      (unlike the lepton formulas which are exact integers)")
print("    - No structural principle selects one formula over another")
print("      (the lepton formulas have a clear recursive pattern)")
print("    - No quark-electron mass anchor (m_u/m_e or m_d/m_e) is clean")
print("    - No color-charge modification of the lepton pattern was found")
print("      that works across all six quark flavors simultaneously")
print()
print("  CONCLUSION:")
print()
print("    [OPEN] Quark mass ratios from FTD lattice integers remain an")
print("    open problem. The candidates identified here are suggestive but")
print("    do not constitute derivations. The fundamental obstacle is that")
print("    quark masses are strongly renormalization-scale dependent (unlike")
print("    lepton masses), so the 'correct' ratios to match are themselves")
print("    scheme-dependent. A proper treatment would require FTD to specify")
print("    its own renormalization procedure for colored objects — which does")
print("    not yet exist in the framework.")
print()
print("    The lepton mass formulas remain FTD's strongest mass predictions.")
print("    Extending them to quarks is a research program, not a solved problem.")
print()


# ============================================================================
# SUMMARY
# ============================================================================

suite.print_summary()
sys.exit(0 if suite.all_pass else 1)
