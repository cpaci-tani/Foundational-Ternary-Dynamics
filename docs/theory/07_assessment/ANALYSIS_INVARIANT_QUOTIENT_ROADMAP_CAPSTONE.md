# Invariant/Quotient Roadmap Capstone — FTD-0395 through FTD-0399

**Status:** [RECONCILIATION — COMPLETE]
**Date:** 2026-07-20
**Scope:** Five independently locked arcs; no new framework type, selected type, calibration, or Framework Commitment adopted.

## 1. Result ledger

| ID | Frozen question | Verdict | Lock / tag | Result | Reconciliation |
|---|---|---|---|---|---|
| FTD-0395 | Is the complete public-API-admissible one-tick update non-injective, rather than only its readout? | **FULL-NONINJECTIVE** | `30bf2216` / `preregister-full-state-irreversibility-v1` | `a8e0c9bb` | `0d039779` |
| FTD-0396 | Does nonlinear native closure preserve the delta-IND v1 characterization? | **bounded BLOCKED-ESCAPE; unrestricted BLOCKED-ESCAPE** | `8b6003d3` / `preregister-nonlinear-delta-ind-v2` | `4706a8ae` | `28183f4f` |
| FTD-0397 | Can permutation-invariant data on `{3,3,4,6}` select the `n=11` ordering? | **PROVEN-SCOPED** | `7012c12b` / `preregister-n11-order-type-no-go-v1` | `d65cedbd` | `2aa9f336` |
| FTD-0398 | Is the frozen local topological charge colocated, transported, or destroyed on scaled octahedra? | **UNDERDETERMINED** | `993d78c5` / `preregister-topological-charge-transport-v1` | `6e0c1b04` | `d6a40b9f` |
| FTD-0399 | Do target-blind A/C/E histories converge to one localized species before any mass observable is tried? | **INVALID — G2 manifestation portability** | `c4f7af98` / `preregister-target-blind-particlehood-v1` | `2c21c827` | `fea7ac7c` |

Every lock was committed and tagged before its registered execution. Every result and corpus reconciliation was committed separately. The preregistration census is GREEN. Raw campaign outputs remain under the existing ignored `engine/results/` convention, with hashes and execution metadata recorded in the corresponding analysis documents.

## 2. The five distinctions

### 2.1 Readout `R` versus update `F`

FTD-0394's genesis witness is a collision only in the discrete readout `R`: three different continuous flux magnitudes survive as different `J` fields after producing the same `(state,color,spin)` record. FTD-0395 separately witnesses non-injectivity of the complete update `F`: two public-API-admissible states differing exactly in spin/color evaporate on the same tick to bit-identical persistent voxel and public causal state, and remain identical for sixteen ticks. The theorem is confined to the current engine map and admissible domain. FC-2 remains `[AXIOM]`; the broader statement that manifestation destroys all information is not licensed.

### 2.2 Bounded versus unrestricted nonlinear native closure

FTD-0396's rational genesis and annihilation anchors pass, so the nonlinear rule representation is adequate for the two required transitions. For `N_bounded`, a fixed event-count bound limits undecorated event-type words but not the allowed space-time-decorated transcript family over polynomial horizons; the v1 period/valuation bound therefore does not follow. For `N_unrestricted`, neither a universal-computation embedding nor a structural non-universality obstruction is proved. Properness fails to close on both rungs, so the delta valuation is not run. This is explicit underdetermination, not evidence that delta is native or non-native. `x+=1/alpha` stays `[SMC]`, MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`, and FC-W stays `[AXIOM]`.

### 2.3 Unordered multiset versus order-bearing data

FTD-0397 proves that the twelve distinct orderings of `{3,3,4,6}` form one permutation orbit. Every permutation-invariant multiset datum is constant on that orbit, while cumulative positions vary over `{10,11,13,14}`. Selecting `n=11` therefore needs an additional order-bearing datum or independently derived symmetry-breaking dynamics. The result is scoped: it does not prohibit a future dynamical derivation. The exponent remains `[SELECTION]`.

### 2.4 Local versus transported topology

FTD-0398 preserves the registered Berg–Lüscher convention and validates it against radial, inverted, rotated, and positively rescaled controls. In the A/C/E campaign, charges appear transiently at several radii and signs, but satisfy none of the frozen COLOCALIZED, TRANSPORTED, or ZERO-CROSSING/DESTROYED predicates. The terminal verdict is UNDERDETERMINED. It supplies no mass evidence, opens no analytic energy-bound proof, and licenses no further shell geometry for this route.

### 2.5 Production history versus species invariant

FTD-0399 fails before its profile quotient can be formed. At both `L=33` and `L=65`, under dissipative and undamped protocols, C manifests at tick 2 while A and E do not manifest within 200 ticks. Duplicate histories are bit-identical, so the failure is reproducible rather than stochastic. Because no three-history aligned ensemble exists, raw distance, normalized shape distance, energy CV, and cross-size distance are not evaluated. The verdict invalidates this authorized comparator, not particlehood in general; under the roadmap stopping rule it opens no mass observable and stops current-engine first-principles mass generation. FTD-0096 remains unchanged.

## 3. Reconciled boundary

The roadmap yields one scoped engine theorem, one scoped group-action theorem, two explicit underdeterminations, and one invalid correctness-gate result. Negative and underdetermined results were not generalized beyond their locks. No mass scale, native particle species, topological mass anchor, nonlinear delta characterization, or new framework commitment was inferred.

At capstone close, the next registry ID was **FTD-0400** and this document allocated no successor. **Successor note (2026-07-21):** FTD-0400 was subsequently allocated to the static confinement-energy/gravity bridge audit, not to an automatic continuation of these five campaigns; its verdict is `SPLIT-BOOKKEEPING`. The next free id is FTD-0401.

## 4. Verification record

Verification used the canonical WSL2 build at `engine/build_wsl` with high CPU concurrency. The final incremental build completed successfully. The five new and directly related CTest targets passed 5/5; the golden label passed 7/7; and the selected GPU-parity set passed 7/7. The new targets were each run at least twice during their registered executions, and their duplicate artifacts were bit-identical where the lock required that gate.

The exact Python verification layer also closed cleanly:

- nonlinear delta-IND v2 recomputed both rational adequacy anchors and returned the two registered `BLOCKED-ESCAPE` verdicts;
- the order-type verifier returned `PROVEN-SCOPED` over all twelve distinct orderings;
- the finite-horizon lemma and S2 adequacy-anchor suites passed 9/9 and 8/8;
- the hedgehog convention validator passed 6/6;
- the topological-transport verifier recomputed all 162 registered CSV rows and returned `UNDERDETERMINED`.

The full CTest registry contained 364 targets: eight are disabled by the repository configuration and all 356 active targets were covered. The aggregate baseline was not green: 343 active targets passed, twelve failed or timed out, and one checkpointed active target lacked a normal pass/fail marker after the long-running CTest controller detached from its shell host. The twelve recorded failures were `gauss_law_fidelity`, `campaign_hydrogen_spectrum`, `variational_coulomb`, `portable_field`, `logic_engine`, `particle_lifetime`, `campaign_statistical_convergence`, `campaign_parity_violation`, `campaign_alpha_readout_scattering` (registered 3600-second timeout), `emergent_ic1_topology`, `campaign_free_dynamics`, and `campaign_gravity_profile`. None is one of the five roadmap targets; the targeted, golden, and parity gates above passed. This record does not recast the unrelated full-suite exceptions as roadmap evidence.

Documentation links for the changed indexes reported zero broken links, `git diff --check` passed, the stale-claim propagation search found only deliberate historical/provenance occurrences, and the final preregistration census was GREEN. The FTD-0399 profile verifier is inapplicable by its frozen protocol because correctness gate G2 aborts before a three-history profile CSV may be formed.

## 5. Canonical result documents

- [FTD-0395 full-state irreversibility](../02_foundations/ANALYSIS_FULL_STATE_IRREVERSIBILITY_v1.md)
- [FTD-0396 nonlinear delta-IND v2](../02_foundations/ANALYSIS_NONLINEAR_DELTA_IND_v2.md)
- [FTD-0397 order-type no-go](../05_particles/THEOREM_N11_ORDER_TYPE_NO_GO.md)
- [FTD-0398 topological transport](../03_derivations/foundational_mechanics/ANALYSIS_TOPOLOGICAL_CHARGE_TRANSPORT_v1.md)
- [FTD-0399 target-blind particlehood](../05_particles/ANALYSIS_TARGET_BLIND_PARTICLEHOOD_v1.md)
