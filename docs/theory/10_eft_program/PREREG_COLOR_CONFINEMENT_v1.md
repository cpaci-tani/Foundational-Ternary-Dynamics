# Pre-Registration — MC-T5.1 Color Confinement Substrate Derivation (v1)

**Tag:** [PRE-REGISTRATION] — this document locks the design of the strong-force color confinement substrate-derivation attempt against the Priority-5 open frontier. All three outcomes — FOUND / UNDERDETERMINED / CLOSED-NEGATIVE — are pre-blessed.

**Date:** 2026-05-27  
**Hash-lock target tag:** `preregister-color-confinement-v1`  
**LEDGER row reservation:** FTD-0217  
**Companion docs:** `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md`, `../03_derivations/DERIV_CONFINEMENT_FROM_GAP_EQUATION.md`.

---

## §1 — Context and Doctrine

### 1.1 The Color Confinement Challenge
In the FTD C++ Cuda/CPU engine, color force interactions are currently handled via a phenomenological piecewise potential (Coulomb $\to$ flux-tube $\to$ linear). While color charge labeling is emergent, the confinement force law is imposed. 

To achieve a true first-principles substrate derivation, we must show that the non-Abelian $\text{SU}(3)$ stencils on the cubic lattice under the Langevin stationary ensemble natively yield area-law Wilson loops with positive string tension:
$$ \langle W(C) \rangle \sim e^{-\sigma A(C)} $$
with $\sigma \approx 0.209$ at the confined root $x_- = 3.024$.

---

## §2 — The Question

**Q-ARC-CONFINEMENT.** Does there exist an operational lattice gauge theory formulation of FTD's local stencils such that:
1. the gauge field dynamics are governed strictly by edge-based link variables $U_\mu(x) \in \text{SU}(3)$;
2. the Langevin stationary distribution on the compact links naturally produces an area law for the Wilson loop $W(C) = \text{Tr} \prod_{e \in C} U_e$;
3. the string tension matches the confined root expectation $\sigma \approx 0.209$ forward from first principles without circular parameter tuning?

---

## §3 — Definitions & Admissible Primitives

- **D1 — SU(3) Link Variables:** Edge-based link fields $U_\mu(x) \in \text{SU}(3)$ satisfying the 26-neighbor Moore connection.
- **D2 — Wilson Loop:** A closed rectangular path $C$ of dimensions $R \times T$ on the 3D cubic lattice.
- **D3 — Admissible Primitives:** Strictly local $O_h$-invariant gauge-field stencils and Langevin updates defined on the compact group $\text{SU}(3)$.

---

## §4 — The Three Pre-Blessed Outcomes

### FOUND
An analytical or numerical proof is established showing that SU(3) gauge links under Langevin stationary flow yield a positive string tension $\sigma \approx 0.209$ at the confined root $x_-$. The area-law decay is proved and matches the strong-coupling expansion.

### UNDERDETERMINED
A partial area-law decay is shown, but the exact string tension remains un-operationalized or relies on phenomenological assumptions.

### CLOSED-NEGATIVE
The non-Abelian stencils fail to produce confinement or require manual scale-switching equivalent to tuning.

---

## §5 — The Falsifier Checklist

The attempt is immediately falsified if:
- **F-a:** Uses experimental QCD string tension or running coupling as input.
- **F-b:** Employs a hand-inserted step function or scale switch to force the linear potential.
- **F-c:** Fails to relate the string tension to the master quadratic confined root $x_-$.
- **F-j:** Reverse-engineers the lattice action by importing the target string tension $0.209$ as a scaffold.

---

## §6 — The 10-Step Method

The closure attempt must execute exactly these steps in order:
1. Define the compact $\text{SU}(3)$ lattice action on FTD.
2. Formulate the Langevin stochastic updates on the link variables.
3. Construct the Wilson loop operator $\text{Tr} \prod U$.
4. Apply the strong-coupling character expansion to the non-Abelian partition function.
5. Derive the leading-order area-law term.
6. Compute the string tension $\sigma$ at the confined root $\beta = x_- = 3.024$.
7. Show that the tension matches the Bessel function ratio:
   $$ \sigma = -\ln \left( \frac{I_1(x_-)}{I_0(x_-)} \right) \approx 0.209 $$
8. Demonstrate that the string tension vanishes at the Coulomb root $x_+$.
9. Check the falsifier rules mechanically.
10. Report the final verdict (FOUND / UNDERDETERMINED / CLOSED-NEGATIVE).
