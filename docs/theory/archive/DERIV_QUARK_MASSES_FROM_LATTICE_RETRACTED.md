# Quark Mass Ratios from Lattice Integers

## Exploring Whether Quark Masses Follow the Lepton Pattern

**Date:** March 17, 2026
**Framework:** Foundational Ternary Dynamics v5.28
**Status:** [OPEN] -- No clean derivation found; candidates cataloged for future work
**Proof script:** `scripts/proofs/proof_quark_masses_lattice.py`

---

## Executive Summary

FTD derives lepton mass ratios from exact integer arithmetic on the framework set {3, 4, 7, 13}. This document asks whether the same integers produce quark mass ratios through structurally analogous formulas. **The honest answer is: not yet.**

Five quark mass ratio candidates were tested. All are tagged **[CONJECTURE]** -- none achieves the clean, exact integer arithmetic that characterizes the lepton formulas. Several candidates are numerically suggestive, but no structural principle uniquely selects them, and the errors (while small) exceed what a true derivation should produce.

**This is an [OPEN] problem.**

---

## Part I: The Lepton Benchmark

### 1.1 What Works for Leptons

The lepton mass ratios are derived from framework integers with no free parameters:

| Ratio | Formula | Value | Experimental | Error |
|-------|---------|-------|-------------|-------|
| m_mu/m_e | 3 * B_3 * (B_3 + N_C) - N_C | 207 | 206.77 | 0.11% |
| m_tau/m_e | (N_EFF + N_BASE) * MU_RATIO - 2 * N_C * B_3 | 3477 | 3477.2 | 0.006% |

**Status:** [THEOREM] -- both formulas are exact integer expressions that agree with experiment to better than 0.2%.

Key features of the lepton pattern:
- Products and sums of 2--3 framework integers
- Small additive/subtractive corrections from the same integer set
- A recursive structure (the tau formula builds on the muon ratio)
- Results are exact integers

### 1.2 Framework Integers

All integers derive from D = 3 and the lemniscatic constant via the master quadratic:

| Integer | Symbol | Value | Origin |
|---------|--------|-------|--------|
| Color number | N_C | 3 | floor(x_-) from master quadratic |
| Base dimension | N_BASE | 4 | 2^((D+1)//2) |
| Beta coefficient | B_3 | 7 | (11*N_C - 2*N_F)/3 |
| Effective parameter | N_EFF | 13 | B_3 + 2*N_C |
| Generations | N_GEN | 3 | = N_C |
| Flavors | N_F | 6 | 2*N_GEN |

---

## Part II: Quark Mass Ratio Candidates

### 2.1 Experimental Values

Quark masses (PDG 2024, MS-bar scheme at mu = 2 GeV):

| Quark | Mass | Uncertainty |
|-------|------|-------------|
| u | 2.16 MeV | +0.49/-0.26 (~15%) |
| d | 4.67 MeV | +0.48/-0.17 (~10%) |
| s | 93.4 MeV | +8.6/-3.4 (~8%) |
| c | 1.27 GeV | +/- 20 MeV (~2%) |
| b | 4.18 GeV | +30/-20 MeV (~1%) |
| t | 172.76 GeV | +/- 0.3 GeV (<1%) |

**Critical note:** Unlike lepton masses (which are physical pole masses), quark masses are scheme- and scale-dependent. The "correct" ratios to match depend on the renormalization prescription. This is a fundamental obstacle for any framework attempting to derive quark masses from first principles without specifying its own renormalization procedure.

### 2.2 Candidates Tested

Only structurally motivated combinations were tested -- formulas paralleling the lepton pattern (products, sums, and ratios of framework integers). No numerical search or fitting was performed.

| Ratio | Candidate | Value | Experimental | Error | Motivation |
|-------|-----------|-------|-------------|-------|------------|
| m_u/m_d | N_C/B_3 | 3/7 = 0.429 | 0.463 | 7.3% | Color/beta ratio |
| m_s/m_d | 2*(B_3 + N_C) | 20 | 20.0 | 0.0% | Double of B_3+N_C sum |
| m_c/m_s | N_EFF | 13 | 13.6 | 4.4% | Effective parameter |
| m_b/m_c | N_EFF/N_BASE | 13/4 = 3.25 | 3.29 | 1.2% | Ratio of central integers |
| m_t/m_b | 2*N_C*B_3 | 42 | 41.3 | 1.7% | Same as tau correction term |

**All candidates are [CONJECTURE].**

### 2.3 Chained Consistency

If all five candidates were simultaneously correct, the total ratio m_t/m_u should be self-consistent:

- Chained: 42 * (13/4) * 13 * 20 / (3/7) = 82,810
- Experimental: m_t/m_u = 172,760/2.16 = 79,981
- Error: 3.5%

The chain is roughly consistent but accumulates non-trivial error, suggesting the individual candidates are not all simultaneously correct.

### 2.4 Absolute Scale

The quark-to-electron mass anchor was also explored:

| Ratio | Candidate | Value | Experimental | Error |
|-------|-----------|-------|-------------|-------|
| m_u/m_e | N_BASE | 4 | 4.23 | 5.4% |
| m_d/m_e | N_EFF - N_BASE | 9 | 9.14 | 1.5% |

Neither is compelling. The quark-electron mass bridge remains [OPEN].

---

## Part III: Honest Assessment

### 3.1 What Distinguishes This from the Lepton Derivation

The lepton mass formulas have qualities that the quark candidates lack:

1. **Exactness.** The lepton formulas produce exact integers (207, 3477). No quark candidate produces an exact match; all have residual errors.

2. **Uniqueness.** The lepton formulas are the simplest expressions from {3, 4, 7, 13} matching the data. For quarks, multiple candidates of similar complexity exist for each ratio, and no principle selects among them.

3. **Recursive structure.** The tau formula explicitly builds on the muon ratio, revealing a generational hierarchy. No analogous recursive structure was found for quarks.

4. **Scale independence.** Lepton masses are physical (pole) masses, unambiguous and precisely measured. Quark masses are renormalization-dependent, introducing an irreducible ambiguity.

### 3.2 The Fundamental Obstacle

Quark masses are not directly observable. They are defined within a renormalization scheme (typically MS-bar) at a specific energy scale. Different schemes and scales give different mass values and different ratios. FTD does not yet specify:

- A renormalization procedure for colored objects on the lattice
- At what scale its "natural" quark masses should be compared with MS-bar values
- Whether the relevant quantities are current masses, constituent masses, or something else

Without this specification, matching FTD integers to quark masses is fundamentally underdetermined.

### 3.3 Epistemic Tags

| Claim | Tag | Justification |
|-------|-----|---------------|
| Lepton mass ratios from integer arithmetic | [THEOREM] | Exact computation, verified |
| Framework integers {3,4,7,13} correctly computed | [THEOREM] | Derived from D=3 + varpi |
| m_t/m_b ~ 2*N_C*B_3 = 42 | [CONJECTURE] | Structurally motivated but not derived |
| m_b/m_c ~ N_EFF/N_BASE = 13/4 | [CONJECTURE] | Clean ratio but possibly coincidental |
| m_s/m_d ~ 2*(B_3+N_C) = 20 | [CONJECTURE] | Matches central value; large exp. uncertainty |
| m_c/m_s ~ N_EFF = 13 | [CONJECTURE] | 4.4% error; not clean |
| m_u/m_d ~ N_C/B_3 = 3/7 | [CONJECTURE] | 7.3% error; weakly motivated |
| Quark-electron mass anchor | [OPEN] | No clean expression found |
| Complete quark mass spectrum from FTD | [OPEN] | Requires renormalization framework |

---

## Part IV: What Would Be Needed

For quark masses to achieve the same status as lepton masses in FTD, the following would be required:

1. **A lattice renormalization procedure.** FTD must define how quark masses run on its discrete lattice, analogous to MS-bar in continuum QCD.

2. **A color-charge mass mechanism.** The lepton formulas do not involve color. Quarks carry color charge, and their masses should reflect this through a structural modification -- not just the same formula with different coefficients.

3. **A confinement-aware mass definition.** Quarks are confined. Their "mass" is always extracted from hadronic observables. FTD would need to derive hadron masses from quark-level dynamics, then extract quark mass ratios from the comparison.

4. **Exact integers.** Any legitimate quark mass formula should produce exact rational numbers (as the lepton formulas do), not approximate matches within experimental uncertainty.

None of these prerequisites currently exist in FTD.

---

## References

- Lepton mass formulas: `docs/theory/05_particles/DERIV_COMPLETE_PARTICLE_PHYSICS.md`
- Framework integers: `docs/theory/01_reference/SPEC_FTD_LAGRANGIAN.md`
- Master quadratic: `scripts/proofs/proof_07_master_quadratic.py`
- Integer cascade: `scripts/proofs/proof_08_integer_cascade.py`
- PDG 2024: Particle Data Group, Phys. Rev. D 110, 030001 (2024)
