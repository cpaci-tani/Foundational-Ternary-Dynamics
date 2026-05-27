# Pre-Registration — MC-T2.1 Stochastic Effective Action (v1)

**Tag:** [PRE-REGISTRATION] — this document locks the design of the stochastic effective action $S_{\text{eff}}$ derivation attempt under the FTD-native EFT program. All three outcomes — FOUND / UNDERDETERMINED / CLOSED-NEGATIVE — are pre-blessed.

**Date:** 2026-05-27  
**Hash-lock target tag:** `preregister-stochastic-effective-action-v1`  
**LEDGER row reservation:** FTD-0218  
**Companion docs:** `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md`, `../10_eft_program/DERIV_FTD_NATIVE_NONLINEAR_FLOW.md`.

---

## §1 — Context and Doctrine

### 1.1 The Stochastic Effective Action Challenge
In FTD, the microscopic partition function $Z[J^{\text{ext}}]$ is defined over the stationary Langevin history ensemble. While a Gaussian fixed point is measured at small scales ($b \le 8$), a formal, non-linear stochastic effective action $S_{\text{eff}}$ is needed to match the discrete lattice partition function to standard continuous gauge actions in the IR limit.

This pre-registration locks the design and decision criteria for deriving the closed-form generating functional $Z[J^{\text{ext}}]$ and the corresponding stochastic effective action $S_{\text{eff}}$ under Langevin flow.

---

## §2 — The Question

**Q-ARC-SEFF.** Does there exist a mathematically complete derivation of the stochastic effective action $S_{\text{eff}}$ such that:
1. the partition function $Z[J^{\text{ext}}]$ is explicitly solved for the interacting Langevin stationary ensemble;
2. the effective action $S_{\text{eff}}$ naturally reduces to the continuous QED action in the low-energy limit;
3. the construction is admissible under the hard exclusion rules of the Alpha Readout Contract?

---

## §3 — Definitions & Admissible Primitives

- **D1 — Langevin Generating Functional:** The source-coupled partition function:
  $$ Z[J^{\text{ext}}] = \left\langle \exp \left( \sum_{x, t} J^{\text{ext}} \cdot J \right) \right\rangle_{\text{Langevin}} $$
- **D2 — Stochastic Effective Action:** The Legendre transform of the generator:
  $$ S_{\text{eff}}[J] = \sup_{J^{\text{ext}}} \left( \sum J^{\text{ext}} \cdot J - \ln Z[J^{\text{ext}}] \right) $$
- **D3 — Admissible Primitives:** Strictly local FTD-native fields, Langevin stencils, and Gaussian white noise.

---

## §4 — The Three Pre-Blessed Outcomes

### FOUND
An explicit closed-form solution for the stochastic effective action $S_{\text{eff}}$ is derived from the Langevin stationary ensemble. The action naturally reproduces the continuous gauge kinetic terms and interactions in the IR limit, passing all 10 F-rules.

### UNDERDETERMINED
A formal partition function is defined, but the effective action cannot be solved in closed form or remains restricted to the Gaussian approximation.

### CLOSED-NEGATIVE
The Langevin flow fails to stabilize at a well-defined fixed point or requires manual parameter scaling violating the contract.

---

## §5 — The Falsifier Checklist

The attempt is immediately falsified if:
- **F-a:** Uses experimental QED parameters or coupling constants as input.
- **F-b:** Contains counterterms or regulators tuned post-hoc to force the Gaussian fixed point.
- **F-j:** Reverse-engineers the action by importing the target continuous action as a scaffold.

---

## §6 — The 10-Step Method

The closure attempt must execute exactly these steps in order:
1. Formulate the stochastic generating functional $Z[J^{\text{ext}}]$ under Langevin updates.
2. Define the path integral over the stochastic noise history.
3. Integrate out the Gaussian noise variables to obtain the effective history action.
4. Derive the Legendre transform to obtain the stochastic effective action $S_{\text{eff}}[J]$.
5. Expand $S_{\text{eff}}[J]$ in powers of the fields and spatial derivatives.
6. Match the quadratic term to the continuous gauge kinetic action.
7. Prove that the higher-order terms are irrelevant in the IR limit ($b \to \infty$).
8. Verify that the flow is consistent with the measured Gaussian fixed point at $b \le 8$.
9. Evaluate the completed derivation against all falsifier rules mechanically.
10. Report the final verdict (FOUND / UNDERDETERMINED / CLOSED-NEGATIVE).
