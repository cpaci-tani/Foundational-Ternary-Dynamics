# Why x₊ = 1/α: The Phase Structure of U(1) Lattice Gauge Theory

## The Coulomb Phase Coupling IS the Fine Structure Constant

**Date:** March 17, 2026
**Status:** Historical argument; [SELECTION] pending FTD-to-EFT matching
**Does not close:** The `x_+ = 1/alpha` physical identification. As of the 2026-04-22 bridge audit, this document is supporting evidence only.

---

## Audit update (2026-04-22)

This document's phase-structure argument remains useful, but its original theorem-level framing was too strong. The bridge-span audits in `docs/theory/10_eft_program/DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md` and `docs/theory/10_eft_program/DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md` show:

```text
FTD state/flux variables -> source-coupled vector EFT              supported
FTD state/flux variables -> primitive compact U(1) gauge theory    not derived
transverse projection -> auxiliary U(1)-like redundancy            supported
```

In particular, the current `s div J` coupling is not invariant under `J -> J + grad chi` without an added constraint, transformation law, or projection. Therefore the statement "FTD is a compact U(1) lattice gauge theory" remains an EFT-level projection/matching claim, not a theorem from Axiom Zero alone.

Read the claims below as conditional:

```text
If transverse projected flux is represented by an auxiliary U(1) gauge potential,
then the standard U(1) phase-structure argument applies.
```

## Abstract

The identification $x_+ = 1/\alpha$ has been classified as [SELECTION] because the physical matching from FTD state/flux dynamics to electromagnetic QED is not uniquely derived. This document records the conditional phase-structure argument: if the FTD flux field `J` is selected or projected as a U(1) gauge potential in temporal gauge, then U(1) lattice gauge theory has a well-known two-phase structure (Wilson 1974, Polyakov 1977), the larger root $x_+$ lies in the Coulomb branch, and the corresponding U(1) coupling is the electromagnetic coupling within that selected EFT.

---

## Part I: FTD IS a U(1) Lattice Gauge Theory

### 1.1 The Structural Identification [SELECTION]

The FTD Lagrangian contains:

- A vector field $\mathbf{J} \in \mathbb{R}^3$ on the lattice Z³ (the flux field)
- A constraint $\nabla \cdot \mathbf{J} = \rho$ (the Gauss law)
- A coupling $-g_c \cdot s \cdot \nabla \cdot \mathbf{J}$ (minimal coupling to ternary charges)
- No temporal component of $\mathbf{J}$ (Postulate 2: discrete time = temporal gauge $A_0 = 0$)

This is precisely the definition of a **compact U(1) lattice gauge theory in temporal gauge** (Creutz, *Quarks, Gluons and Lattices*, Chapter 6; Rothe, *Lattice Gauge Theories*, Section 3.3):

| FTD | Lattice U(1) gauge theory |
|-----|--------------------------|
| Flux $\mathbf{J}$ | Vector potential $\mathbf{A}$ |
| $\nabla \cdot \mathbf{J} = \rho$ | Gauss law $\nabla \cdot \mathbf{E} = \rho$ |
| $g_c \cdot s \cdot \nabla \cdot \mathbf{J}$ | Minimal coupling $e \cdot \psi^\dagger \cdot \nabla \cdot \mathbf{A} \cdot \psi$ |
| No temporal $J_0$ | Temporal gauge $A_0 = 0$ |
| $s \in \{-1, 0, +1\}$ | Charge $q \in \{-1, 0, +1\}$ |

The identification is structural once the gauge-potential dictionary is selected, but that dictionary is not derived by this document. The minimal state/flux dictionary gives a physical vector flux coupled to a signed source. Promoting that vector to a U(1) gauge potential requires the extra matching step isolated in `DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md`.

### 1.2 The Coupling [SELECTION]

In the FTD Lagrangian, the coupling constant is $g_c = \sqrt{\alpha}$ where $\alpha = 1/x_+$ from the master quadratic. The inverse coupling squared is:

$$x = \frac{1}{g_c^2} = \frac{1}{\alpha}$$

The gap equation determines two self-consistent values of $x$: $x_+ = 137.036$ and $x_- = 3.024$.

---

## Part II: The Two Phases of U(1) on the Lattice

### 2.1 Wilson's Confinement Criterion [THEOREM — standard lattice gauge theory]

Wilson (1974) showed that lattice gauge theories exhibit phase transitions as a function of the coupling:

- **Weak coupling** (small $g^2$, large $x = 1/g^2$): The Wilson loop $\langle W(C) \rangle$ obeys a **perimeter law**. The static potential is $V(r) \sim -g^2/r$ (Coulomb). Charges are **deconfined**.

- **Strong coupling** (large $g^2$, small $x = 1/g^2$): The Wilson loop obeys an **area law**. The static potential is $V(r) \sim \sigma \cdot r$ (linear). Charges are **confined**.

For compact U(1) in 3+1 dimensions, the transition occurs at a critical coupling $g_c^2 \sim O(1)$ (i.e., $x_{\text{crit}} \sim O(1)$).

### 2.2 The Two Roots as Two Phases [THEOREM for the identification]

The gap equation $x^2 = 16G^{*2}(x - G^*)$ has two roots:

| Root | Value | $g^2 = 1/x$ | Phase | Force |
|------|-------|-------------|-------|-------|
| $x_+$ | 137.036 | 0.00730 | **Coulomb** (weak coupling) | Electromagnetism |
| $x_-$ | 3.024 | 0.331 | **Confined** (strong coupling) | Strong force |

The critical coupling for the phase transition lies between the two roots: $G^* = 2.959$ satisfies $x_- < G^* < x_+$, placing the transition point at the harmonic center (the point the gap equation forbids).

### 2.3 The Identification [CONDITIONAL]

1. The FTD Lagrangian is selected/projected as a U(1) lattice gauge theory [SELECTION]
2. U(1) lattice gauge theories have Coulomb and confined phases [THEOREM — Wilson 1974]
3. The Coulomb-phase coupling of a U(1) gauge theory IS the electromagnetic coupling $\alpha$ [DEFINITION]
4. The Coulomb phase corresponds to $x_+$ (weak coupling, $g^2 = 0.0073$) [THEOREM — from the gap equation]
5. Therefore $\alpha = 1/x_+ = 1/137.036$ within that selected EFT dictionary [CONDITIONAL]

The identification is not a separate postulate — it follows from recognizing what the FTD Lagrangian IS.

---

## Part III: x₋ and the Strong Force

### 3.1 The Confined Phase [SELECTION]

The smaller root $x_- = 3.024$ corresponds to strong coupling ($g^2 = 0.331$). In the confined phase, the static potential is linear: $V(r) \sim \sigma \cdot r$. This is the qualitative behavior of the strong nuclear force (quark confinement).

The identification $x_- \approx N_c = 3$ (the number of colors) is more subtle. In QCD, the number of colors enters through the gauge group SU($N_c$), not through the coupling strength. The FTD framework uses a U(1) gauge field, not SU(3). The connection between $x_- \approx 3$ and $N_c = 3$ is:

$$\text{floor}(x_-) = \text{floor}(3.024) = 3 = N_c$$

This is [SELECTION] — the interpretation of $x_-$ as a color count rather than just a strong-coupling value. However, the cuboctahedral geometry independently gives $N_c = 3$ from the number of square-face axis pairs [THEOREM], providing a second route to the same integer.

### 3.2 The Phase Transition at G* [THEOREM for structure]

The gap equation forbids $x = G^*$ (it would require $G^{*2} = 0$). The two roots sit on opposite sides of $G^*$:

- $x_+ = 137.036 > G^* = 2.959$ (Coulomb phase, far above transition)
- $x_- = 3.024 > G^* = 2.959$ (confined phase, just above transition)

Both roots are in the $x > G^*$ regime. The transition point $G^*$ itself is inaccessible — the gap equation has no solution there. This means the lattice cannot sit at the phase boundary; it must commit to one phase or the other.

The extreme asymmetry $(x_+ - G^*)/(x_- - G^*) \approx 2063$ means the Coulomb phase is far from the transition (EM is weakly coupled, perturbative) while the confined phase is near the transition (the strong force is marginally confined, explaining why $\alpha_s \sim 0.1$ is much larger than $\alpha \sim 0.007$).

---

## Part IV: The Probabilistic Interpretation

### 4.1 α as the Manifestation Probability [SELECTION]

In the two-layer ontology:
- The flux field $\mathbf{J}$ is dispositional (what could happen)
- The state $s$ is actual (what does happen)
- The coupling $g_c$ mediates between them

The Born rule (FOUND_BORN_RULE_NULL_CONE.md) gives $P = |g_c|^2 = \alpha$ as the probability of a single interaction — the probability that the dispositional becomes actual at one vertex.

The gap equation says: the manifestation probability ($\alpha$) that enters the dynamics must be the same probability that the dynamics produce. The self-consistent value is $\alpha = 1/x_+ = 1/137.036$.

This interpretation adds conceptual depth but is not independent of the gauge theory argument. It is the same identification ($g_c^2 = \alpha$) expressed in the language of the two-layer ontology.

---

## Part V: What This Does and Does Not Prove

### Established [THEOREM]

1. Conditional on the gauge-potential dictionary, the FTD Lagrangian satisfies the structural form of a U(1) lattice gauge theory in temporal gauge
2. U(1) lattice gauge theories have Coulomb and confined phases (Wilson 1974)
3. The Coulomb-phase root is $x_+$ (weak coupling)
4. The Coulomb-phase U(1) coupling IS $\alpha$ by definition
5. Therefore $x_+ = 1/\alpha$ inside the selected U(1) EFT dictionary

### The remaining assumption

The identification of microscopic $\mathbf{J}$ as a gauge field is not a consequence of the "minimal continuous extension" choice alone. Axiom Zero supports `J in R^3` as a minimal continuous vector field, but a vector field is not automatically a gauge potential. The better bridge is to treat U(1) as an emergent redundancy of auxiliary variables representing transverse projected flux. Given that EFT-level projection and a matter-coupling prescription, the standard U(1) phase-structure argument follows.

---

## References

- Wilson, K. G. "Confinement of quarks," *Physical Review D* **10** (1974), 2445
- Polyakov, A. M. "Quark confinement and topology of gauge theories," *Nuclear Physics B* **120** (1977), 429
- Creutz, M. *Quarks, Gluons and Lattices*, Cambridge University Press, 1983
- Rothe, H. J. *Lattice Gauge Theories: An Introduction*, World Scientific, 2012
- DERIV_MASTER_QUADRATIC_GAP_EQUATION.md — The gap equation (03_derivations)
- FOUND_AXIOM_ZERO.md — Axiom Zero (02_foundations)
- FOUND_BORN_RULE_NULL_CONE.md — Born rule from null cone (02_foundations)
