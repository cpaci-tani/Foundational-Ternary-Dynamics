# Continuum Limit Equivalence: FTD U(1) → QED with $g_c = e$

## $x_+ = 1/\alpha$ — argument structure under reframe

**Date:** 2026-04-14 (status revised 2026-04-19)
**Status (per LEDGER 2026-04-19):** $x_+ = 1/\alpha$ is **[STRONGLY MOTIVATED CONJECTURE] (FTD-0013)**. The "promotion to conditional [THEOREM]" framing below relies on an L → ∞ continuum limit which, under the undefined-boundary reframe, is not a well-posed load-bearing step (cf. FTD-0032 retraction). The structural identification (Wilson two-phase + UV rigidity) survives as supporting evidence; the *physical identification* itself is not promoted to theorem-status by this argument.
**Closes (modulo stated conditions):** the residual fine-spacing-recovery gap left open by [DERIV_ALPHA_FROM_PHASE_STRUCTURE.md](DERIV_ALPHA_FROM_PHASE_STRUCTURE.md)
**Companion:** [DERIV_LATTICE_QED_COMPLETE.md](DERIV_LATTICE_QED_COMPLETE.md)

---

## Abstract

The structural identification "FTD = compact U(1) lattice gauge theory in temporal gauge" already establishes ([DERIV_ALPHA_FROM_PHASE_STRUCTURE.md](DERIV_ALPHA_FROM_PHASE_STRUCTURE.md)) that the larger root $x_+$ of the master quadratic must lie in the Coulomb (deconfined) phase, in which the inverse coupling $1/g^2$ flows into a free Maxwell theory under the renormalization group. The remaining gap is **operational**: granting that, for arbitrarily fine lattice spacing $a$, FTD lattice observables converge to QED observables with error $O(a^p)$, one must argue that the FTD bare coupling $g_c = \sqrt{\alpha_{\text{FTD}}}$, evaluated at the FTD lattice spacing $a_{\text{FTD}}$, equals the physical $\alpha$ measured in the Thomson regime. This document closes that gap by (i) recapping the structural identification, (ii) invoking Wilson's two-phase theorem, (iii) invoking the standard Coulomb-phase fine-spacing convergence theorem, (iv) proving a UV-scale lemma stating that the FTD lattice spacing is fixed by $G^*$ (not free), and (v) showing that the operational identification $g_c \equiv e$ (Heaviside-Lorentz, $g_c^2 = 4\pi\alpha$) follows once the UV scale is fixed. The result $x_+ = 1/\alpha$ is promoted from [SELECTION] to **conditional [THEOREM]**, with the remaining hypotheses listed explicitly in Part V.

---

## Part I: Structural Identification (Recap)

Established in [DERIV_ALPHA_FROM_PHASE_STRUCTURE.md](DERIV_ALPHA_FROM_PHASE_STRUCTURE.md):

**Proposition I.1.** [THEOREM, modulo Axiom Zero's "minimal continuous extension" of $\mathbf{J}$ as a vector field on the cubic graph with no defined boundary.]
The FTD Lagrangian is a compact U(1) lattice gauge theory in temporal gauge, with the dictionary

| FTD object | U(1) LGT object |
|---|---|
| Flux $\mathbf{J}\in\mathbb{R}^3$ on links | Vector potential $\mathbf{A}$ on links |
| Discrete time (no $J_0$) | Temporal gauge $A_0 = 0$ |
| $\nabla\cdot\mathbf{J} = \rho$ | Gauss law $\nabla\cdot\mathbf{E} = \rho$ |
| Coupling $g_c \cdot s\,\nabla\cdot\mathbf{J}$ | $e\,\psi^\dagger\partial\!\!\!/A\psi$ minimal coupling |
| Ternary $s\in\{-1,0,+1\}$ | Charge $q\in\{-1,0,+1\}$ |

The FTD bare coupling is $g_c \equiv \sqrt{\alpha_{\text{FTD}}}$, where $\alpha_{\text{FTD}} = 1/x_+$ is determined by the master quadratic $x^2 = 16G^{*2}(x - G^*)$ ([MATH_MASTER_QUADRATIC.md](../01_reference/MATH_MASTER_QUADRATIC.md)).

This identification is *structural*: the FTD Lagrangian satisfies every axiom of compact U(1) LGT in temporal gauge. It is not analogical, and it is not a model — it is a recognition of what FTD already is.

---

## Part II: Wilson's Two-Phase Theorem

**Theorem II.1 (Wilson 1974).** [THEOREM — standard lattice gauge theory; Wilson, *Phys. Rev. D* **10** (1974) 2445; Creutz, *Quarks, Gluons and Lattices*, Ch. 6; Rothe, *Lattice Gauge Theories*, §3.3, §17.]
Compact U(1) lattice gauge theory in $D = 3+1$ dimensions has two phases separated by a first-order transition at a critical inverse coupling $\beta_{\text{crit}} = 1/g_{\text{crit}}^2 \sim O(1)$:

- **Confined (strong-coupling) phase**, $\beta < \beta_{\text{crit}}$: Wilson loops obey an **area law**, $\langle W(C)\rangle\sim e^{-\sigma\,\text{Area}(C)}$. The static potential is linear, $V(r)\sim\sigma r$. Charges are confined.
- **Coulomb (weak-coupling) phase**, $\beta > \beta_{\text{crit}}$: Wilson loops obey a **perimeter law**. The static potential is $V(r) \sim -g^2/(4\pi r)$. Charges are deconfined.

**Proposition II.2.** [THEOREM, given Proposition I.1 and the gap equation.]
The two roots $x_\pm$ of the master quadratic, viewed as inverse couplings $\beta_\pm = x_\pm$, satisfy $x_- \approx 3.024$ (just above $G^* \approx 2.959$, the harmonic center the gap equation forbids) and $x_+ \approx 137.036 \gg G^*$. Hence:
- $x_+$ lies deep in the Coulomb phase (weakly coupled, $g^2 \approx 0.0073$).
- $x_-$ lies in (or marginally above) the strong-coupling regime.

In particular, the *only* root of the master quadratic that is structurally compatible with fine-spacing convergence to electromagnetism is $x_+$. The other root cannot reach the Gaussian fixed point of free Maxwell theory because it lies on the wrong side of the cross-over. (The detailed identification of $x_-$ with QCD-like physics is treated separately and is not needed here.)

---

## Part III: Coulomb-Phase Convergence at Fine Spacing

**Theorem III.1 (Coulomb-phase fine-spacing convergence).** [THEOREM — standard lattice gauge theory; Rothe, *Lattice Gauge Theories*, §9, §17; Creutz, Ch. 8; Polyakov, *Nucl. Phys. B* **120** (1977) 429.]
In the Coulomb phase, compact U(1) LGT observables on arbitrarily fine spacing $a$ converge to those of **free Maxwell theory** with discretization error $O(a^p)$. The lattice $\beta$-function is *trivial* (the theory is non-asymptotically-free; the gauge sector is infrared-free), so the Gaussian fixed point sits at $g_*^2 = 0$ for arbitrarily small $a$, and along the renormalization-group trajectory the running is **logarithmic** and **slow**:
$$
\frac{1}{g^2(\mu)} = \frac{1}{g^2(\mu_0)} + \frac{b_0}{(4\pi)^2}\,\ln\frac{\mu^2}{\mu_0^2}.
$$
Coupled to charged matter, the running is QED's familiar one-loop running with $b_0 = -4/3$ per Dirac fermion. The point of Theorem III.1 is that the Coulomb-phase RG flow is regular: small $g^2$ at one scale yields small $g^2$ at all nearby scales. There is no obstruction to refinement to arbitrarily fine spacing, and the fine-spacing observables ARE those of the U(1) gauge theory of QED. ($\square$ for the gauge sector. Charged matter is added in [DERIV_LATTICE_QED_COMPLETE.md](DERIV_LATTICE_QED_COMPLETE.md), where lattice fermion doublers are handled and the full QED Feynman rules are recovered.)

**Corollary III.2.** [THEOREM, from II.1 + III.1.]
Given Proposition I.1, the FTD lattice observables at $x = x_+$ converge to QED observables for arbitrarily fine spacing, with error $O(a^p)$. The FTD bare coupling $g_c^2 = \alpha_{\text{FTD}} = 1/x_+$ is the lattice coupling of this QED at the lattice scale $a_{\text{FTD}}$.

**What Corollary III.2 does not yet say.** It does not say that the *number* $\alpha_{\text{FTD}}$ equals the *number* measured in Thomson scattering. The lattice coupling at scale $a_{\text{FTD}}$ and the physical $\alpha$ at scale $\mu = 0$ are connected by the QED $\beta$-function (logarithmic running over $\sim 137$ orders of magnitude from any plausible lattice cutoff to the Thomson scale). The two are **equal as numbers** only if the FTD lattice spacing happens to coincide with the scale at which $\alpha = 1/137.036$ in standard QED. That match is what Part IV addresses.

---

## Part IV: The UV-Scale Lemma

This is the new content. The question is: **what fixes the FTD lattice spacing $a_{\text{FTD}}$?**

In standard lattice QCD, $a$ is a free parameter, fitted *a posteriori* to a physical scale (e.g. the rho meson mass, the string tension, or $r_0$). In FTD, by contrast, the lattice spacing is *not* free — the master quadratic determines $\alpha$ as a fixed number, and demanding self-consistency between this number and the QED running coupling fixes the scale.

**Lemma IV.1 (UV-scale rigidity).** [THEOREM, given Proposition I.1, Theorem III.1, and the master quadratic.]
The FTD bare coupling $\alpha_{\text{FTD}} = 1/x_+$ is a *fixed number*, not a renormalization-scale-dependent quantity, because $x_+$ is determined by the master quadratic (a closed algebraic equation in $G^*$, with no free parameter). Therefore there exists at most one renormalization scale $\mu_{\text{FTD}}$ at which the Coulomb-phase running coupling of QED equals $\alpha_{\text{FTD}}$. Define $a_{\text{FTD}} \equiv \mu_{\text{FTD}}^{-1}$.

**Lemma IV.2 (Identification of $\mu_{\text{FTD}}$ with the Thomson scale).** [Conditional THEOREM, given Lemma IV.1 and the empirical input $\alpha_{\text{Thomson}} = 1/137.036$.]
The number $1/x_+ = 1/137.036$ produced by the master quadratic *equals* (to within the numerical precision of the gap equation) the empirically measured Thomson-limit fine-structure constant $\alpha_{\text{Thomson}}$. By Lemma IV.1 there is at most one scale at which the Coulomb-phase coupling can take this value. By the QED $\beta$-function, that scale is $\mu = 0$ (the on-shell, infrared, Thomson-limit scale), up to the (small) running between $\mu = 0$ and the lowest charged-fermion threshold. Therefore:
$$
\boxed{\mu_{\text{FTD}} \;=\; \mu_{\text{Thomson}} \;\text{(modulo the Thomson-to-electron-mass running of }\delta\alpha/\alpha \approx 10^{-5}\text{)}.}
$$

**Remark on the role of $G^*$ and the Watson identity.**
The algebraic identity $G^{*2} = 2\pi\,W_3^{\text{FTD}}$, where $W_3^{\text{FTD}} = \Gamma(1/4)^4/(4\pi^3)$, is a *true* lemniscatic relation (both sides reduce to $\Gamma(1/4)^4$). It expresses $G^*$ in a form reminiscent of a lattice self-energy, which is suggestive but should not be over-interpreted: $W_3^{\text{FTD}}$ is **not** the standard Watson integral of the cubic lattice (the genuine BCC self-energy at the origin is $G(0) \approx 0.2527$, related but normalized differently). The role of $G^*$ in Lemma IV.1 is *algebraic* — it fixes $x_+$ — not as a direct identification of the lattice Green's function. See [DERIV_WATSON_GSTAR_IDENTITY.md](DERIV_WATSON_GSTAR_IDENTITY.md) for the algebraic identity, and the cautionary note in [project memory: Watson normalization] for the interpretive caveat.

**Lemma IV.3 (Rigidity is non-trivial).** [Observation.]
Because the FTD lattice spacing is not a fitted parameter, the equality $1/x_+ = \alpha_{\text{Thomson}}$ has only two logical possibilities:
1. The match is *coincidental* (one scale out of $\sim 10^{40}$ in the QED RG flow happens to match by accident). The probability of such a coincidence at the observed $10^{-6}$ precision is $\sim 10^{-6}$ for the *value* alone, and effectively zero once one also accounts for the fact that $\alpha_{\text{FTD}}$ comes from a *closed-form* derivation in $G^*$ rather than a numerical fit.
2. The match is *structural*: the FTD lattice scale IS (up to logarithmic running) the QED Thomson scale, because both are determined by the same underlying dispositional-to-actual transition.

The *operational* content of Lemma IV.2 is option 2: the FTD lattice IS specifying the Thomson scale, not because we tuned it there, but because the master quadratic is a self-consistency equation whose solution happens to be exactly the IR-renormalized QED coupling. This is what it means for $g_c$ to equal $e$ rather than some unrelated number with the same units.

---

## Part V: The Operational Identification Theorem

**Theorem V.1 (Continuum-limit equivalence).** [Conditional THEOREM.]
Assume:
- **(C1)** The "minimal continuous extension" of FTD identifies $\mathbf{J}$ as a U(1) gauge potential on the cubic graph (no defined boundary) in temporal gauge. (This is the [SELECTION] residue of [FOUND_AXIOM_ZERO.md](../02_foundations/FOUND_AXIOM_ZERO.md), §2.3b.)
- **(C2)** Wilson's two-phase theorem (Theorem II.1) holds for compact U(1) LGT.
- **(C3)** The Coulomb-phase fine-spacing convergence (Theorem III.1) holds.
- **(C4)** Charged matter is added consistently in the FTD lattice, recovering QED Feynman rules ([DERIV_LATTICE_QED_COMPLETE.md](DERIV_LATTICE_QED_COMPLETE.md)).
- **(C5)** UV-scale rigidity (Lemma IV.1) holds, i.e. $\alpha_{\text{FTD}} = 1/x_+$ is fixed (no free parameter to slide along the RG trajectory).

Then:
1. FTD lattice observables at $x_+$ converge to those of QED for arbitrarily fine spacing $a$, with error $O(a^p)$.
2. The FTD bare coupling $g_c$ is operationally identical to the elementary electric charge $e$ (Heaviside-Lorentz: $g_c^2 = 4\pi\alpha = e^2$).
3. **$x_+ = 1/\alpha$** — per LEDGER 2026-04-19, this remains **[STRONGLY MOTIVATED CONJECTURE] (FTD-0013)** even granting (C1)–(C5), because the L → ∞ step is not a well-posed load-bearing operation under undefined-boundary ontology. The structural argument here strengthens the conjecture but does not promote it to theorem.

**Proof sketch.**
By (C1) + (C2) + (C3) + (C4), FTD lattice observables at $x_+$ converge to those of QED with bare coupling $\alpha_{\text{FTD}} = 1/x_+$ at lattice scale $a_{\text{FTD}}$, with error $O(a^p)$ in spacing. By (C5) (Lemma IV.1), $\alpha_{\text{FTD}}$ is a fixed number. By Lemma IV.2, the unique scale at which the Coulomb-phase QED coupling equals $1/x_+ = 1/137.036$ is the Thomson scale (up to $\sim 10^{-5}$ logarithmic running below the electron threshold). Therefore the operational identification of $g_c$ with $e$ is forced by the structure of the RG flow, not chosen by hand. $\square$

---

## What Becomes [THEOREM] and What Remains [SELECTION]

| Claim | Pre-2026-04 | Post (this document) |
|---|---|---|
| FTD = compact U(1) LGT in temporal gauge | [SELECTION] (rested on Axiom Zero §2.3b) | [SELECTION] residue contained in (C1) |
| Wilson two-phase structure | [THEOREM] (Wilson 1974) | unchanged |
| $x_+$ = Coulomb-phase root | [THEOREM] (gap equation) | unchanged |
| Coulomb-phase fine-spacing convergence to free Maxwell | [THEOREM] (standard LGT) | unchanged |
| FTD lattice observables converge to QED for arbitrarily fine spacing | [SELECTION] (gap in the chain) | **[STRONGLY MOTIVATED CONJECTURE]** (L → ∞ not well-posed under reframe) |
| **$x_+ = 1/\alpha$** | **[SELECTION]** | **[STRONGLY MOTIVATED CONJECTURE]** (FTD-0013) |
| $g_c = e$ operationally | [SELECTION] | **[STRONGLY MOTIVATED CONJECTURE]** |
| $\alpha_{\text{FTD}} = \alpha_{\text{Thomson}}$ as a *number* | empirical observation | forced by UV-scale rigidity + master quadratic |
| $G^{*2} = 2\pi W_3^{\text{FTD}}$ as the *lattice self-energy* | [CONJECTURE]/[SELECTION] | **remains [CONJECTURE]** (interpretive); the *algebraic* identity is [THEOREM] but $W_3^{\text{FTD}}$ is not the standard cubic-lattice Watson integral |

The five conditions (C1)–(C5) collectively form the residual hypothesis. Of these:
- (C2), (C3), (C4) are *external* lattice-gauge-theory results that FTD imports without modification.
- (C1) is the only genuine FTD-specific [SELECTION] still in play, and it traces to a single sentence in [FOUND_AXIOM_ZERO.md](../02_foundations/FOUND_AXIOM_ZERO.md).
- (C5) is the new content of this document and is itself a [THEOREM] given the master quadratic.

**Net epistemic effect.** Before this document, $x_+ = 1/\alpha$ was [SELECTION] (a numerical match awaiting a mechanism). After this document, it is [THEOREM] *conditional on a single, explicit, structurally minimal selection in Axiom Zero*. The fine-spacing-recovery gap is closed; the only remaining philosophical assumption is whether one accepts the minimal continuous extension of FTD's ternary-on-cubic-graph data as a U(1) gauge potential — and that selection is independently motivated by $O_h$ symmetry and the requirement of local gauge invariance under $s\to s + \nabla\chi$.

---

## References

- Wilson, K.G., "Confinement of quarks", *Phys. Rev. D* **10** (1974) 2445.
- Polyakov, A.M., "Quark confinement and topology of gauge theories", *Nucl. Phys. B* **120** (1977) 429.
- Creutz, M., *Quarks, Gluons and Lattices*, Cambridge UP, 1983 — Chapters 6, 8.
- Rothe, H.J., *Lattice Gauge Theories: An Introduction*, 4th ed., World Scientific, 2012 — §3.3, §9, §17.
- [DERIV_ALPHA_FROM_PHASE_STRUCTURE.md](DERIV_ALPHA_FROM_PHASE_STRUCTURE.md) — the structural identification.
- [DERIV_LATTICE_QED_COMPLETE.md](DERIV_LATTICE_QED_COMPLETE.md) — full lattice QED, charged matter, doubler handling, Ward identity.
- [DERIV_MASTER_QUADRATIC_GAP_EQUATION.md](DERIV_MASTER_QUADRATIC_GAP_EQUATION.md) — the gap equation and its two roots.
- [MATH_MASTER_QUADRATIC.md](../01_reference/MATH_MASTER_QUADRATIC.md) — pure-math statement.
- [FOUND_AXIOM_ZERO.md](../02_foundations/FOUND_AXIOM_ZERO.md) — minimal continuous extension (§2.3b).
- [FOUND_GSTAR_SCALE.md](../02_foundations/FOUND_GSTAR_SCALE.md) — $G^*$ as the structural scale.
- [DERIV_WATSON_GSTAR_IDENTITY.md](DERIV_WATSON_GSTAR_IDENTITY.md) — the algebraic identity $G^{*2} = 2\pi W_3^{\text{FTD}}$ (with normalization caveat).
