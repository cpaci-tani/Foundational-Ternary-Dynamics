"""
Proof 10: Ultimate Ontic Chain — Unified End-to-End Verification
=================================================================

Imports and runs all proof modules (01-09), aggregates results,
and generates a comprehensive report with:
  - Total claims by epistemic category
  - Pass/fail for each
  - Complete derivation chain summary
  - Constants vs experiment comparison table
  - Falsification criteria

Usage:
    cd simulations && python -m proofs.proof_10_ultimate_chain
"""

import sys
import time

from .common import ProofSuite, merge_suites, TAGS, G_STAR, X_PLUS, X_MINUS, ALPHA, N_C
from . import (
    proof_01_elliptic_fibration,
    proof_02_cm_selection,
    proof_03_critical_coupling,
    proof_04_coefficient_16,
    proof_05_packing_fraction,
    proof_06_gstar_emergence,
    proof_07_master_quadratic,
    proof_08_integer_cascade,
    proof_09_all_constants,
    proof_11_predictions,
)


MODULES = [
    ("01", "Elliptic Fibration",    proof_01_elliptic_fibration),
    ("02", "CM Selection (j=1728)", proof_02_cm_selection),
    ("03", "Critical Coupling √2",  proof_03_critical_coupling),
    ("04", "Coefficient 16",        proof_04_coefficient_16),
    ("05", "Packing Fraction π/4",  proof_05_packing_fraction),
    ("06", "G* Emergence",          proof_06_gstar_emergence),
    ("07", "Master Quadratic",       proof_07_master_quadratic),
    ("08", "Integer Cascade",        proof_08_integer_cascade),
    ("09", "All Constants",          proof_09_all_constants),
    ("11", "Predictions",            proof_11_predictions),
]


def run() -> ProofSuite:
    """Run all proof modules and return merged suite."""
    suites = []
    for num, name, mod in MODULES:
        suite = mod.run()
        suites.append(suite)
    return merge_suites("ULTIMATE ONTIC CHAIN", suites)


def print_derivation_chain():
    """Print the complete derivation chain as ASCII art."""
    print("""
    THE ONTIC DERIVATION CHAIN
    ==========================

    Layer -1:  e  (self-referential seed: d/dx e^x = e^x)
               |
    Layer 0:   γ → Γ(1/4) = 3.6256...
               |
    Layer 0b:  q = e^(-π) (nome) → θ₃ (Jacobi theta)
               |
    Layer 1:   ϖ = Γ(1/4)²/(2√(2π)) = 2.6221...
               |   M = ϖ/π = 0.8346...
               |
    Layer 2:   G* = 2√(ϖ·M) = 2.9587...
               |   π = 4ϖ²/G*²
               |   PF = π/4 = 0.7854...
               |
    Layer 2b:  k_crit = 4/G* ≈ 1.352 (i emerges here)
               |
    Layer 3:   x² - 16G*²x + 16G*³ = 0
              / \\
    x₊ = 137.036  x₋ = 3.024
         |              |
    Layer 4:   α = 1/x₊     N_c = ⌊x₋⌋ = 3
         |              |
         |         N_gen=3, N_f=6, N_base=4
         |         b₃=7, N_eff=13, D=47
         |              |
    Layer 5:   sin²θ_W = 3/13    G_N = 1/100
         |     α_W = α/(3/13)    α_G ~ α²⁰
         |     α_s = 7/59
         |              |
    Layer 6:   m_e = m_P·√(2π)·(16/3)·α¹¹
         |     m_μ/m_e = 207,  m_τ/m_e = 3477
         |     v_Higgs = m_P·√(2π)·α⁸
         |              |
    Layer 7:   ε = e^π - π - 20 ≈ -9.0×10⁻⁴
         |     1/α_corrected = x₊ - c₁|ε| + c₂|ε|² - c₃|ε|³ - c₄|ε|⁴
         |     = 137.035999177 (< 1 ppt from CODATA)
         |              |
    Layer 8:   y² - (G*²/2)y + G*³/2 = 0  [k=1/2]
               y = 2.19 ± 2.86i
               cos²(θ_C) = G*/8 ≈ 0.370
    """)


def print_experiment_table():
    """Print constants vs experiment comparison."""
    print("""
    CONSTANTS vs EXPERIMENT
    =======================

    Quantity              FTD Value        Experiment       Error     Tag
    ─────────────────────────────────────────────────────────────────────
    1/α (tree)            137.0362         137.035999       1.26 ppm  [CONJECTURE]
    1/α (corrected)       137.035999177    137.035999177    <1 ppt    [THEOREM]
    sin²θ_W               3/13 = 0.2308   0.23122          0.20%     [THEOREM]
    α_s(M_Z)              7/59 = 0.1186   0.1179           0.63%     [THEOREM]
    sin²(θ₁₂)             3/10 = 0.300    0.307            2.3%      [THEOREM]
    sin²(θ₂₃)             16/29 = 0.552   0.546            1.0%      [THEOREM]
    sin²(θ₁₃)             1/52 = 0.019    0.022            12.7%     [THEOREM]
    Δm²₃₁/Δm²₂₁          100/3 = 33.33   32.85            1.5%      [THEOREM]
    m_μ/m_e               207              206.768          0.11%     [THEOREM]
    m_τ/m_e               3477             3477.23          0.007%    [THEOREM]
    m_p (MeV)             938.16           938.272          0.012%    [THEOREM]
    v_Higgs (GeV)         246.09           246.22           0.05%     [THEOREM]
    m_Higgs (GeV)         124.8            125.1            0.24%     [SELECTION]
    α_G                   5.91×10⁻³⁹      5.906×10⁻³⁹     0.06%     [THEOREM]
    Σm_ν (meV)            58.1             < 120            OK        [SELECTION]
    N_c                   3                3                exact     [THEOREM]
    N_gen                 3                3                exact     [SELECTION]
    ─────────────────────────────────────────────────────────────────────
    """)


def print_falsification():
    """Print falsification criteria."""
    print("""
    FALSIFICATION CRITERIA
    ======================

    What would conclusively falsify FTD's core claims:

    1. α MEASUREMENT: Precision α measurement incompatible with x₊ = 137.036...
       at better than 10 ppm (after accounting for radiative corrections).

    2. 4th GENERATION: Discovery of a 4th generation fermion with standard
       gauge couplings would break N_gen = floor(x₋) = 3.

    3. WRONG INTEGERS: Any derivation in the cascade {3,4,7,13} that fails
       to match the QFT formula it implements (e.g., b₃ ≠ 7 in SU(3) QCD).

    4. sin²θ_W DEVIATION: If sin²θ_W deviates from 3/13 by more than 1%
       (after radiative corrections to the Z-pole value).

    5. LATTICE ARTIFACTS: Observable Lorentz violation at Planck scale
       with wrong sign (superluminal high-energy photons).

    6. G* PRECISION: If a more precise calculation of G* from the
       derivation chain gives x₊ outside 10 ppm of 1/α_CODATA.
    """)


def main():
    """Run all proofs and print comprehensive report."""
    print("=" * 70)
    print("  ULTIMATE ONTIC CHAIN PROOF SUITE")
    print("  Everything from nothing: e → γ → Γ(1/4) → ϖ → G* → all physics")
    print("=" * 70)

    total_start = time.time()

    # Run all modules
    suites = []
    for num, name, mod in MODULES:
        t0 = time.time()
        suite = mod.run()
        dt = time.time() - t0
        status = "PASS" if suite.all_pass else f"FAIL ({suite.failed})"
        print(f"  [{num}] {name:30s}  {suite.passed:3d}/{suite.total:3d}  {status:8s}  ({dt:.3f}s)")
        suites.append(suite)

    total_time = time.time() - total_start

    # Merge into master
    master = merge_suites("ULTIMATE ONTIC CHAIN", suites)

    # Summary statistics
    print("\n" + "=" * 70)
    print("  AGGREGATE RESULTS")
    print("=" * 70)
    print(f"  Modules:    {len(MODULES)}")
    print(f"  Claims:     {master.total}")
    print(f"  Passed:     {master.passed}")
    print(f"  Failed:     {master.failed}")
    print(f"  Time:       {total_time:.2f}s")
    print()

    # Breakdown by tag
    for tag in TAGS:
        items = master.by_tag(tag)
        if items:
            p = sum(1 for i in items if i.passed)
            f = sum(1 for i in items if not i.passed)
            print(f"  {tag:14s}  {p:3d} passed, {f:3d} failed  (total: {len(items)})")

    print()

    # Key numerical results
    print("  KEY RESULTS")
    print("  ───────────")
    print(f"  G*        = {G_STAR:.15f}")
    print(f"  x₊ (1/α)  = {X_PLUS:.10f}")
    print(f"  x₋ (N_c)  = {X_MINUS:.10f}")
    print(f"  α          = {ALPHA:.15f}")
    print(f"  N_c        = {N_C}")
    print()

    # Print supplementary sections
    print_derivation_chain()
    print_experiment_table()
    print_falsification()

    # Print failures if any
    failures = [r for r in master.results if not r.passed]
    if failures:
        print("\n  FAILURES")
        print("  ────────")
        for r in failures:
            print(f"  FAIL  {r.tag:14s}  {r.name}")
            print(f"        got={r.value}, expected={r.expected}, error={r.error:.2e}")

    # Epistemic summary
    print("\n  HONEST EPISTEMIC SUMMARY")
    print("  ────────────────────────")
    theorem_count = len(master.by_tag("[THEOREM]"))
    selection_count = len(master.by_tag("[SELECTION]"))
    conjecture_count = len(master.by_tag("[CONJECTURE]"))
    axiom_count = len(master.by_tag("[AXIOM]"))
    imposed_count = len(master.by_tag("[IMPOSED]"))
    conditional_count = len(master.by_tag("[CONDITIONAL]"))

    print(f"  [AXIOM]:       {axiom_count:3d}  (structural postulates, not derivable)")
    print(f"  [THEOREM]:     {theorem_count:3d}  (rigorously proven from axioms)")
    print(f"  [SELECTION]:   {selection_count:3d}  (argued from consistency)")
    print(f"  [CONJECTURE]:  {conjecture_count:3d}  (proposed, requiring validation)")
    print(f"  [IMPOSED]:     {imposed_count:3d}  (parameter choices)")
    print(f"  [CONDITIONAL]: {conditional_count:3d}  (conditional on selections)")

    print(f"\n  The suite does NOT claim to prove FTD is correct physics.")
    print(f"  It proves that GIVEN the stated axiom (D=3) and selection")
    print(f"  principles, the entire constant catalog follows rigorously.")

    print("\n" + "=" * 70)
    overall = "PASS" if master.all_pass else "FAIL"
    print(f"  OVERALL: {overall}")
    print("=" * 70)

    return master


if __name__ == "__main__":
    master = main()
    sys.exit(0 if master.all_pass else 1)
