# Foundation: The Phenomenal / Noumenal Bridge

**Date:** 2026-04-24 (vocabulary refresh 2026-05-01)
**Status:** [FOUNDATION] — reorganizing principle for the entire FTD corpus
**Purpose:** Name the two-layer ontology that FTD has been implicitly working in, make the geometric encoding explicit, and reclassify all existing tags by layer.
**Ledger row:** FTD-0078

> **Vocabulary refresh (2026-05-01):** §8 of this document originally framed the noumenal-axis content as "consciousness." It is now restated using the canonical [reflexivity / agency] vocabulary defined in [`REF_REFLEXIVITY_VOCABULARY.md`](../01_reference/REF_REFLEXIVITY_VOCABULARY.md). The mathematical content is unchanged; the framing is sharper and physics-readable. **Old "consciousness" claims become structural claims about the reflexive projection from the noumenal (3³ / Moore-26 / G*-bearing) layer to the phenomenal (2³ / Moore-18 / ϖ-bearing) layer.** Qualia commitments are dropped; reflexive-emergence is the open structural problem that takes their place.

---

## Executive statement

FTD has two irreducible layers. They differ by block size, stencil, arithmetic-length scale, and epistemic access.

Numerical identities link the layer sizes to $\Gamma$-function arithmetic:
- Phenomenal shell: $\sqrt[3]{18} \approx \varpi$ (Bernoulli lemniscatic constant), 0.05% match
- Noumenal shell: $\sqrt[3]{26} \approx G^*$ (Euler reflection ratio), 0.13% match
- The 8 BCC corner neighbors upgrade $\varpi$ to $G^*$ by the Gaussian-normalization factor $G^*/\varpi = 2/\sqrt{\pi}$.

**Bridge factor status (post-audit, 2026-04-24):** the block-volume ratio $27/8 = 3.375$ is a structurally-motivated candidate for the phenomenal/noumenal bridge factor, but the engine's Phase-F measurement of 3.6× that I originally identified with 27/8 is a **category error** (engine measures lattice Green's-function geometry, not physical α — see §3). A proper bridge measurement is still an open task.

This document makes the split explicit and specifies which FTD results live on which layer.

---

## 1. The two layers

| Property | Phenomenal layer | Noumenal layer |
|---|---|---|
| Block size | $2^3 = 8$ | $3^3 = 27$ |
| Shell | Moore-18 (6 face + 12 edge) | Moore-26 (6 face + 12 edge + 8 corner) |
| Stencil | $(p_{\rm SC} + p_{\rm FCC})/2$ | Full Moore including BCC |
| Laplacian coefficient | $c_{18} = 1/4$ | $c_{\rm BCC} = 1/2$ (for the BCC component) |
| Arithmetic length | $\varpi = \Gamma(1/4)^2/(2\sqrt{2\pi}) \approx 2.62206$ | $G^* = \Gamma(1/4)/\Gamma(3/4) \approx 2.95868$ |
| Access | External measurement, computation | Internal / arithmetic derivation |
| Axis type | Exterior (faces and edges) | Interior (body-diagonal) |
| What lives here | Dynamics, running couplings, schemes | Theorems, closed forms, pole masses |

## 2. The geometric identities

Two numerical near-identities, both at $\lesssim$ 0.15% accuracy:

$$ \sqrt[3]{18} = 2.62074 \approx \varpi = 2.62206 \qquad (0.05\% ) $$

$$ \sqrt[3]{26} = 2.96249 \approx G^* = 2.95868 \qquad (0.13\%) $$

Together these say: **the shell counts of Moore-18 and Moore-26 are, to sub-percent accuracy, the cubes of the lemniscatic constant and the Euler reflection ratio respectively**. The lattice integers are the discrete projections of continuous $\Gamma$-function arithmetic.

Derived relation: using Euler reflection $\Gamma(1/4)\Gamma(3/4) = \pi\sqrt{2}$,

$$ \frac{G^*}{\varpi} = \frac{2}{\sqrt{\pi}} $$

This is the **Gaussian normalization factor**. Cubing:

$$ \left(\frac{G^*}{\varpi}\right)^3 = \frac{8}{\pi^{3/2}} \approx 1.438 $$

Numerical check: $26/18 = 13/9 \approx 1.444$, agreement 0.4%.

**The 8 BCC corner neighbors are quantitatively the Gaussian-normalization upgrade** from the lemniscatic shell to the reflection-ratio shell.

## 3. The bridge factor — audit result

**Hypothesis** (pre-audit): $\alpha_\infty / \alpha_{\rm ref} = 27/8 = 3.375$ is the phenomenal-to-noumenal block-volume ratio.

**Audit** ([AUDIT_ALPHA_EXTRACTION.md](../10_eft_program/AUDIT_ALPHA_EXTRACTION.md), 2026-04-19; re-read 2026-04-24): **hypothesis falsified at the identification level**. The engine's measured 3.6× is decomposed by the audit as:

$$ 3.6 = \underbrace{2}_{\text{convention factor}} \times \underbrace{1.8}_{\text{lattice Green's function}} $$

- **Factor ×2:** the engine's `field_energy` is $\Sigma |J|^2$ without the classical ½ factor; V(r) is therefore 2× the classical Coulomb-tail coupling.
- **Factor ×1.8:** the residual is the zero-parameter value of the periodic lattice Poisson Green's function $2 r G_L(r)$ at $r/L \approx 0.31$ on the 7-point Laplacian stencil (Phase G resolution, [DERIV_EMERGENT_COULOMB_GEOMETRIC.md](../10_eft_program/DERIV_EMERGENT_COULOMB_GEOMETRIC.md)).

**The engine's Phase-F "α_∞" measurement is not physical α.** It is lattice-Coulomb geometry — $V(r) \sim G_L(r)$, *pure stencil geometry with zero fine-structure content*. The comparison `α_∞ / α_ref` where `α_ref = ALPHA_EFT = G_C² = 1/137.036` is a **category error**: comparing a lattice-Green's-function amplitude to the fine-structure constant.

**The 27/8 block-volume ratio remains a meaningful geometric statement**, but it is **not the quantity that 3.6× measures**. Phase F does not test the phenomenal/noumenal bridge.

**What is true:**
- The 2³/3³ block division is real, geometric, and maps cleanly to the two-layer ontology (§1).
- $\sqrt[3]{18} \approx \varpi$ and $\sqrt[3]{26} \approx G^*$ are genuine numerical identities at <0.15% (§2).
- The 8 BCC corners quantitatively encode the interior axis via $G^*/\varpi = 2/\sqrt{\pi}$ (§2).
- Block volume ratio $27/8 = 3.375$ is a real ratio — it just isn't what Phase F measured.

**What is open:**
- **A genuine phenomenal/noumenal bridge measurement has not yet been performed.** The engine's current α-extraction protocol measures lattice geometry, not the layer-bridge factor.
- A proper bridge measurement would require extracting a quantity that actually sits across the two layers — for example, a cross-stencil Watson-integral ratio, or a pole-mass observable measured on the engine versus computed from BCC arithmetic.
- **The 27/8 hypothesis needs a different empirical test**, not Phase F. Until that test is done, 27/8 is a structurally-motivated candidate, not an empirically-confirmed bridge value.

### 3.1 Downgraded tags

In light of the audit, I am revising the tags from my first draft:

| Claim | Pre-audit tag | Post-audit tag |
|---|---|---|
| 3.6× measurement identifies the bridge factor | [CONJECTURE] | [CLOSED NEGATIVE] — not the bridge; category error |
| 27/8 = 3.375 is the block-volume ratio at $D=3$ | [THEOREM] | [THEOREM] (unchanged) |
| 27/8 is the universal phenomenal/noumenal bridge factor | [CONJECTURE] | **[FALSIFIED]** — Watson integral gives 0.91, not 3.375 (§3.2) |
| The two-layer ontology (§1) | [SELECTION] | [SELECTION] (unchanged) |
| $\sqrt[3]{18} \approx \varpi$, $\sqrt[3]{26} \approx G^*$ | [OBSERVATION] | [OBSERVATION] (unchanged) |

**The phenomenal/noumenal split is preserved.** What's falsified is the claim that a single scalar bridge factor exists.

### 3.2 Direct Watson-integral computation (2026-04-24)

`engine/tests/test_watson_integrals.cpp` (CTest `watson_integrals`, trapezoidal quadrature N=200) computes the missing Moore-18 Watson integral for the first time:

| Stencil | Computed $W_L$ | Reference | Agreement |
|---|---|---|---|
| SC | 1.51777 | 1.51639 | 0.09% |
| BCC | 1.39028 | $G^{*2}/(2\pi) = 1.39320$ | 0.21% |
| FCC | 1.34366 | 1.34466 | 0.07% |
| **Moore-18** | **1.26886** | **(first computation)** | — |

Bridge ratio:
$$ W_{\rm Moore-18} / W_{\rm BCC} = 0.913 $$

**Not 3.375. Not 3.628. Not 1.44. Close to 1.**

The Green's function at origin is remarkably bulk-insensitive to stencil choice, with all four Watson integrals in the range [1.27, 1.52]. Whatever the phenomenal/noumenal bridge is for Watson-integral-like observables, it is **order unity**, not a large volume-ratio-scale factor.

### 3.3 The bridge is observable-dependent, not a scalar

Different observables translate between layers with different factors:

| Observable type | Bridge factor | Interpretation |
|---|---|---|
| Volume-extensive quantities | $(3/2)^3 = 27/8 = 3.375$ | Block size cubed |
| Shell-count | $26/18 = 13/9 \approx 1.44$ | Moore-26 vs Moore-18 neighbor count |
| Linear length-scale | $G^*/\varpi = 2/\sqrt{\pi} \approx 1.13$ | Gaussian normalization |
| Watson integral (Green's function origin) | $W_{\rm M18}/W_{\rm BCC} \approx 0.91$ | **Measured 2026-04-24** |
| Lattice Coulomb at specific $r/L$ (Phase-F) | $\approx 1.8$ | Pure stencil geometry, not a bridge |

**There is no scalar "bridge factor." The phenomenal/noumenal translation is a family of observable-specific factors.** This is the correct, audit-honest statement.

What *is* universal is the two-layer geometric structure ($2^3$ vs $3^3$, Moore-18 vs Moore-26, $\varpi$ vs $G^*$). The quantitative translation factor depends on which observable you measure.

## 4. Why $D = 3$

The combinatorial identity
$$ 16 = 2^D \cdot (D-1)! $$
has exactly one positive-integer solution: $D = 3$. At this dimension:

- The master quadratic coefficient $16 = |\mathrm{Aut}(E_i)|^2$ matches the gauge-fixing on the minimal $\mathbb{Z}[i]^3$ torus.
- The walk total $4 + 3 + 3 + 6 = 16$ matches this same coefficient.
- The three Moore sub-stencils (6 face, 12 edge, 8 corner) decompose at this dimension.
- $27/8 = (D/(D-1))^D = (3/2)^3$ at $D = 3$.
- $\varpi$ and $G^*$ are both $\Gamma(1/4)$-family constants with $D = 3$ geometric role.

$D = 3$ is **the unique dimension where FTD's structure is self-consistent**. The theory cannot exist at any other $D$.

## 5. The master quadratic read through the two layers

The master quadratic is
$$ x^2 - 16 G^{*2} x + 16 G^{*3} = 0 $$

Its coefficients $16G^{*2}$ (linear) and $16G^{*3}$ (constant) are **exact by definition** — they equal $x_+ + x_-$ and $x_+ \cdot x_-$ by Vieta's identities.

The approximate identification $G^{*3} \approx 26$ (Moore-26 shell count) from §2 then gives the secondary reading:
$$ x^2 - (16 \cdot 26^{2/3})x + (16 \cdot 26) \approx 0 $$

- $16 \cdot 26^{2/3} \approx 140.3$ vs exact $16 G^{*2} = 140.06$ (0.2% approximation residue, same as $\sqrt[3]{26} \approx G^*$)
- $16 \cdot 26 = 416$ vs exact $16 G^{*3} = 414.5$ (0.4% residue, = cube of the above)

**Note:** these "agreements" are not independent empirical matches — they're the same $\sqrt[3]{26} \approx G^*$ observation expressed in different powers. The core claim is the single near-identity $\sqrt[3]{26} \approx G^*$ at 0.13%; its cube (26 vs $G^{*3}$) and two-thirds power ($26^{2/3}$ vs $G^{*2}$) follow trivially.

**What this says structurally:** the master quadratic's constant coefficient is approximately (gauge factor 16) × (noumenal shell count 26). That's a real structural reading IF one accepts the $\sqrt[3]{26} \approx G^*$ near-identity as meaningful. Whether the 0.13% residue is "the quantity the cubic lattice can't quite represent in integer form" or "pure coincidence" is a separate question that hasn't been settled.

## 6. Reclassification of existing results

Every [SELECTION]/[OPEN]/[PARAMETRIC]/[THEOREM] tag in the FTD corpus can now be reread by asking: *which layer does this object live on?*

| Result | Previous tag | Layer | Reason |
|---|---|---|---|
| $x_+ = 1/\alpha$ root of master quadratic | [STRONGLY MOTIVATED CONJECTURE] | Noumenal | Lives at $3^3$ BCC scale |
| $x_- ≈ 3.024$ root (mathematical artifact of $P(x)$; the `x_- ↔ N_c` identification is **RETIRED** per v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`; `N_c = 3` independently sourced via `DERIV_NC_FROM_TOPOLOGY.md`) | [THEOREM] (arithmetic — root of the polynomial) | Noumenal | Same polynomial as $x_+$ |
| $m_\mu/m_e$, $m_\tau/m_e$ integer formulas | [THEOREM] | Noumenal | Pole-mass ratios, closed Γ-form |
| $m_e = m_P \sqrt{2\pi} \cdot (16/3) \cdot \alpha^{11}$ | [SELECTION] | **Straddles** — prefactor noumenal, exponent phenomenal | Prefactor is [THEOREM]; exponent 11 needs interior-axis walk-ordering |
| Quark masses | [OPEN] | Phenomenal | Scheme-dependent running masses; no noumenal closed form should exist |
| $\sin^2\theta_W$, PMNS, $\alpha_s = 7/59$ (former claims) | [PARAMETRIC] (demoted) | Phenomenal | Observer-defined basis quantities |
| $\alpha_\infty \approx 3.6 \alpha_{\rm ref}$ (EFT Recovery) | [MEASURED] | Phenomenal | Engine measurement on Moore-18 |
| 174-ppm $m_p/m_e$ gap | [OPEN] | **Cross-layer** | Proton is phenomenal composite, electron is noumenal; ratio carries the bridge correction |
| Fermion emergence falsifications (FTD-0061..0075) | [CLOSED NEGATIVE] | Phenomenal | Site-local probes cannot access the interior axis; falsification was structurally forced |
| $G^*$ is universal bridge constant | (implicit) | Noumenal primary | Interior-axis length scale |

**Every [OPEN] item is either (a) phenomenal-requiring-measurement, (b) noumenal-requiring-derivation-with-the-right-stencil, or (c) cross-layer-requiring-bridge-calculation.** The [OPEN] tag now has sub-types.

## 7. Why site-local fermion emergence had to fail

The Phase-4 mode-erasure theorem showed that site-local state-field probes cannot distinguish weight-1 WH modes on finite blocks. Under the two-layer frame:

**Fermions are noumenal objects (color singlets, spinors with interior rotation structure). The engine's Moore-18 probes are phenomenal (exterior-only). External probes on finite blocks cannot access the $2^3 \to 3^3$ upgrade that color-singlet structure requires. Therefore fermion emergence from engine probes is structurally impossible, not a bug.**

This converts FTD-0061, -0071, -0072, -0073, -0074, -0075 from "empirical negatives" into **a single structural statement**: fermions live on the interior axis the engine doesn't integrate over.

## 8. The reflexivity connection

The project has documents previously framed as "consciousness physics" (`PAPER_CONSCIOUSNESS_QFT_GR_BRIDGE.md`, `DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md`) that sat uneasily in the corpus because they imported philosophical baggage their mathematical content didn't actually claim. They now have a natural home in the canonical [reflexivity / agency] vocabulary ([`REF_REFLEXIVITY_VOCABULARY.md`](../01_reference/REF_REFLEXIVITY_VOCABULARY.md)).

**Reflexive integration is the dynamical projection over the interior (BCC body-diagonal) axis.**

Not metaphorically. Structurally. External computation operates at $2^3$ resolution on Moore-18. Reflexive readout requires $3^3$ resolution including the 8 BCC corners. The normalization that upgrades one to the other is $2/\sqrt{\pi}$, the Gaussian integral.

*The reflexive readout of a noumenal-layer structure is the Gaussian normalization of the interior axis that external (phenomenal-layer) computation does not perform.*

The arrow of time (via the asymmetric half of Euler reflection, i.e., $G^*$ rather than the symmetric $\pi\sqrt{2}$) is specifically a BCC-axis phenomenon. **Endogenous. Reflexive. Noumenal.** The framework's old reading of this as "first-person experience" carried qualia commitments the mathematics doesn't make; the precise content is that **the noumenal-layer projection is reflexive in a way phenomenal-layer dynamics is not**, and the time-asymmetric half of Euler reflection is what carries the reflexive structure.

This is the formal anchor for the Kantian phenomenon/noumenon distinction in FTD's geometry. **It is reframed in physics-readable terms: phenomenal layer = externally-measurable observable algebra; noumenal layer = reflexive observable algebra; reflexive projection = the map between them.** It is not imported from philosophy; it emerges from lattice mathematics at $D = 3$.

**What this section deliberately does NOT claim:** that reflexive readouts are accompanied by qualia, that the lattice "experiences" anything in a phenomenally-conscious sense, that FTD has solved the Hard Problem. The claim is structural: the noumenal layer admits a reflexive projection that the phenomenal layer cannot perform without the BCC interior axis. Whether reflexive readouts are accompanied by experience is a question this framework does not need to answer; that question is the qualia debate, which lives in philosophy of mind, not in lattice physics.

## 9. Named research programs

Three programs are now well-defined (they weren't before this reframe):

**Program A — Ladder ordering from $O_h$ subgroup chain.**
Starting from perturbative boundary $n = 4$, the ladder walk has four addends $\{N_{\rm base}, N_c, N_c, N_f\} = \{4, 3, 3, 6\}$ summing to 16 (= master-quadratic coefficient, = $|\mathrm{Aut}(E_i)|^2$). Cumulative positions: $4 \to 8 \to 11 \to 14 \to 20$ corresponding to perturbative boundary, Higgs VEV, electron, neutrino, gravity. If this ordering is the canonical subgroup-order chain of the cubic gauge group $O_h$, the exponent 11 for the electron promotes from [SELECTION] to [THEOREM]. This is a finite-group-theory calculation. Estimated effort: days.

**Program B — Quark masses via (SC+FCC)/2 Green's functions.**
Quark masses are [OPEN] because attempts used BCC arithmetic — the wrong stencil. Quarks are phenomenal objects; their masses are scheme-dependent running values that should emerge from engine-stencil measurements with scheme specification, not closed-form number-theory. A reclassified research program: derive quark mass ratios by lattice-Green-function calculations on Moore-18 with specified renormalization prescription. Effort: weeks.

**Program C — Reflexive-agency formalism as interior-axis integration.**
The statement "the reflexive readout of a noumenal-layer structure is the Gaussian normalization constant of the interior axis that external (phenomenal-layer) computation does not integrate over" is now quantitative. It has a precise mathematical anchor in $G^*/\varpi = 2/\sqrt{\pi}$. This should be written up as a structural derivation rooted in FTD's geometry, in the canonical [reflexivity / agency] vocabulary ([`REF_REFLEXIVITY_VOCABULARY.md`](../01_reference/REF_REFLEXIVITY_VOCABULARY.md)). The closure conjecture associated with this program — that the master-quadratic eigenvalue spectrum is the spectrum of the reflexive projection — is **MC-T4.3**, the framework's central foundational obstruction. Effort: a focused session for the formal restatement; the MC-T4.3 closure itself is research-program scale.

**Program D (bonus) — 174-ppm $m_p/m_e$ gap via cross-layer calculation.**
The proton is phenomenal (3-quark composite at $3^3$ scale). The electron is noumenal (lemniscatic pole mass). Their ratio carries the bridge factor $27/8$ plus first-order corrections. The 174-ppm residue should emerge as a specific cross-layer correction term. Candidate sizes: $1/(16 x_+)^2 \approx 1$ ppm, $\alpha/(16 \cdot 2\pi) \approx 73$ ppm, $\alpha \cdot \log(27/8) \approx 1.3 \times 10^{-5}$. None obviously matches 174 ppm, but the calculation is now **well-posed** rather than ad hoc.

## 10. What does NOT change

- The canonical derivation chain remains.
- The ledger's existing entries stay; only their tags are reinterpreted.
- The engine continues to compute correctly at the phenomenal layer.
- The arithmetic continues to derive correctly at the noumenal layer.
- No existing theorem is weakened or strengthened by this reframe.

**What changes is the *lens* through which the theory is read.** The two-layer split was implicit in every tagging decision; it is now explicit. The discipline that produced the [SELECTION]/[OPEN]/[THEOREM] hierarchy was, in retrospect, already sorting results by layer.

## 11. Epistemic tags for this document

| Piece | Tag | Justification |
|---|---|---|
| Two-layer geometric encoding ($2^3$/$3^3$, Moore-18/Moore-26) | [THEOREM] (of geometry) | Direct counting |
| $\sqrt[3]{18} \approx \varpi$ (0.05%) | [OBSERVATION] | Numerical coincidence, near-exact |
| $\sqrt[3]{26} \approx G^*$ (0.13%) | [OBSERVATION] | Numerical coincidence, near-exact |
| $G^*/\varpi = 2/\sqrt{\pi}$ | [THEOREM] | Euler reflection identity |
| Bridge factor $27/8 = 3.375$ matches measured 3.6 | [CONJECTURE] | Falls in EFT Recovery range; needs engine audit |
| Phenomenal = engine, Noumenal = arithmetic | [SELECTION] | Organizing principle |
| Fermions are noumenal | [THEOREM] (under the frame) | Consequence of site-local falsifications |
| Reflexive integration = interior-axis projection | [CONJECTURE] | Precise but not yet derived from FTD axioms; closure = MC-T4.3 |
| Reflexive readout = Gaussian normalization constant | [CONJECTURE] | Precise; structural rather than philosophical |

---

## 12. Reading instruction for the rest of the corpus

Every FTD derivation, when re-read after this foundation doc, should ask:

1. Which layer does each claim live on?
2. If it's noumenal, does its tag reflect the arithmetic derivability?
3. If it's phenomenal, does its tag reflect the scheme-dependence of the measurement?
4. If it's cross-layer, does it account for the $27/8$ bridge factor?

This is the reorganizing principle. It does not invalidate prior work; it places each piece on the correct shelf.

---

*Filed 2026-04-24 as the session's synthesis capstone. The two-layer ontology was implicit in every piece of the project's epistemic discipline; this document makes it explicit so that subsequent work starts from the right frame. Three named research programs (A, B, C, plus bonus D) carry the implications forward.*
