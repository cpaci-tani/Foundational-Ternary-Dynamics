# FOUND — What is an Orientation Carrier? The Bilateral-Symmetry Criterion

**Tag:** [SYNTHESIS] + [STRUCTURAL OBSERVATION] (the group theory) + [coherent-interpretation] (the FTD-frontier mapping). Promotes nothing.
**LEDGER row:** FTD-0382
**Verifier:** `scripts/proofs/proof_bilateral_orientation_stabilizer.py` (12/12, sympy-exact)
**Companions:** FTD-0248 `FOUND_EPISTEMIC_SYMMETRIES_AND_CHIRALITY.md` (the Observer Postulate this sharpens), FTD-0341 `../07_assessment/audits/AUDIT_ANALYTIC_ORIENTATION_CARRIERS.md` (magnitude/phase theorem), FTD-0336 `FOUND_MODULUS_ARGUMENT_FRONTIER.md` (frontier), FTD-0314 `../07_assessment/audits/AUDIT_W_CARRIER_NARROWING.md` (carrier narrowing), FTD-0355 `DERIV_D3_FROM_AUTOMORPHISM.md` (axis-stabilizer arithmetic), FTD-0375 (rhyme-not-identification guard).

---

## 0 · Empirical anchor (load-bearing, hedged)

An orientation carrier, in the sense this document formalizes, is anything that carries a **signed direction** — an arrow, a distinguished sense along one axis. The physical world exhibits a sharp, falsifiable pattern about such things: **objects that undergo directed self-locomotion carry a bilateral (single-mirror) symmetry and nothing more.** People, cars, birds, fish are symmetric left↔right but *not* front↔back and *not* top↔bottom. Directed motion picks one axis, gives it an arrow (front ≠ back), and the only surviving symmetry is the single mirror transverse to travel.

This must be stated with **four qualifiers** or it is false; each is load-bearing:

1. **External / leading-order.** The claim is about the *external bauplan*. Real organisms are exactly **C₁** (no symmetry) — internal chirality (heart left, liver/gut coiling right, driven by the Nodal–Pitx pathway) breaks the mirror at higher order. Bilateral symmetry is C_s *to leading order*, C₁ exactly. [TEXTBOOK: "Bilaterally Symmetrical: To Be or Not to Be?", *Symmetry* 12:326.]
2. **Macroscopic / inertial (high-Reynolds).** At low Reynolds number the **scallop theorem** forbids net motion from reciprocal strokes; microswimmers (bacteria, sperm, spirochetes) propel by *chiral* helical motion — chirality is favored, bilateral is not. The pattern is a macroscopic/inertial phenomenon. [TEXTBOOK: Purcell; *Nat. Commun.* PMC4241991.]
3. **Manoeuvring, not merely translating.** The clean counter-case is the **jellyfish**: it swims directionally *along* its oral–aboral axis, which is its rotational-symmetry axis (C_n, not a left/right mirror pair) — and it turns poorly. The rule is really "movers *that must steer* are bilateral." Echinoderms make the same point in reverse: bilaterally-symmetric mobile larvae metamorphose into slow pentaradial adults. [TEXTBOOK: *PNAS* 2024 jellyfish; PMC3256158 echinoderms.]
4. **Whole-body, not sub-component.** Wheels, propellers, and screws have rotational symmetry about an axle lying on the *transverse* axis — but they are *parts*, carried in L/R pairs, and the hull keeps its sagittal mirror. No macroscopic animal has a free-spinning wheel (the vascular/neural rotary-joint problem); the only true biological rotary drive is the bacterial flagellar motor — microscopic, back to qualifier 2. [TEXTBOOK: rotating locomotion in living systems.] (Sidewinder snakes add a caveat: travel direction can *decouple* from the body's fore-aft axis; bilaterality is a property of the body, not of a fixed body-to-trajectory alignment. [*PNAS* 2015.])

**Attribution discipline** (kept explicit so the physics is not mistaken for the biology): the three body axes and their patterning (AP/Hox, DV/BMP-Chordin, LR/Nodal-Pitx), cephalization, and the C_s point-group label are **[TEXTBOOK]**. The mechanistic *why* — that manoeuvrability selects bilateral bodies (streamlined fore-aft, non-streamlined laterally, left/right turns equally available) — is the **[HYPOTHESIS]** of Holló & Novák 2012 (*Biology Direct*, PMC3438024), one group's live proposal, not consensus. The group-theoretic *packaging* below (signed-travel + signed-gravity + symmetric-transverse ⇒ C_s as a residual-symmetry/stabilizer statement) was **not found in the literature as a single unit** — it is this document's synthesis of established parts. (The active-matter "symmetry breaking" literature is a different phenomenon — collective order, not body-plan — and is deliberately not cited.)

## 1 · The formal criterion [MATH-FACT — verified P1/P2]

Strip the biology and the content is a clean statement in O(3).

> **Definition (orientation carrier).** In a space with a distinguished set of *special directions* (signed vectors), an object is an orientation carrier iff its residual symmetry group is the reduction obtained by fixing those vectors. The bilateral carrier fixes exactly two — a **travel vector v** (self-supplied) and a **gravity vector g** (environment-supplied).

**Lemma (bilateral = stabilizer of two vectors).** For independent **v**, **g** ∈ ℝ³,
$$\operatorname{Stab}_{O(3)}(\mathbf v, \mathbf g) \;=\; \{\,I,\ \sigma\,\} \;=\; C_s,$$
where σ is the reflection through the plane span(**v**, **g**) (the sagittal plane). *Proof:* an orthogonal map fixing two independent vectors fixes their 2-plane pointwise; on the 1-dimensional orthocomplement it acts as ±1; +1 is I, −1 is σ. Order exactly 2. (Verifier P1: σ built explicitly as a Householder reflection, σv = v, σg = g, σ² = I, det σ = −1, σ ≠ I; completeness from the 1-dim orthocomplement.)

**Orientation is more data than a line.** An unoriented axis is a line, whose intrinsic symmetry *includes* the reflection x ↦ −x. Orienting it — choosing a direction — selects one of two rays: an element of a **ℤ/2-torsor**, i.e. promoting the line to (the ray of) a **vector**, which *breaks* the reflection on that axis. This is the precise sense in which "an oriented axis is a vector, not just a line," and it is **direction-data — a ℤ/2 sign, magnitude-free** (§3 turns on this).

**Distinct from chirality.** C_s is *achiral*: it has a mirror. Chirality is C₁ — *no* mirror, left/right *also* broken. Orientation (one signed axis + surviving transverse mirror) and chirality (all mirrors gone) are different, stronger-vs-weaker symmetry breaks. Conflating them is the single most common error here; §3 uses the distinction to separate FTD's arrow from its handedness.

**The criterion singles out D = 3 [MATH-FACT — verified P2].** The stabilizer generalizes as Stab_{O(D)}(v, g) = I₂ ⊕ O(D−2). Its order is |O(D−2)|:

| D | Stab = I₂ ⊕ O(D−2) | order | point group |
|---|---|---|---|
| 2 | O(0) trivial | 1 | C₁ — *over-determined*, no mirror survives |
| **3** | **O(1) = {±1}** | **2** | **C_s — exactly one mirror** |
| 4 | O(2) | ∞ | a whole circle of rotations survives |

**Exactly-C_s residual symmetry holds iff D = 3.** In D = 2, orienting two independent directions over-determines and kills all symmetry (C₁); in D ≥ 4 a continuous subgroup survives. Only in three dimensions does "choose a travel arrow and a gravity arrow" leave *precisely one mirror*. This is a genuine consonance with FTD's spatial dimension — recorded as consonance in §5, **not** as a derivation of D = 3 (which is FTD-0355 [SELECTION — declared circular]).

## 2 · Sharpening FTD-0248 (the Observer Postulate)

FTD already carries this picture, diffusely. `FOUND_EPISTEMIC_SYMMETRIES_AND_CHIRALITY.md` (FTD-0248, [CONJECTURE]) describes a propagating bound state with a **signed longitudinal axis** (forward/back = "the spatial projection of time's arrow"), **left/right laterals related by a single mirror plane** (the longitudinal-vertical plane), and up/down "polar reflection" — its "four broken symmetries" table. This *is* the bilateral mover.

This document's contribution to FTD-0248 is precision plus an anchor: the **achiral orientation core** of its four-broken-symmetries description is **the point group C_s = Stab_{O(3)}(e_L, e_grav)** — the stabilizer of the longitudinal trajectory vector and the vertical (gravity/polar) vector — and the macroscopic empirical pattern of §0 is its physical witness. (Precisely: FTD-0248 breaks more than C_s does — its "imposed chiral preference" on the L/R laterals is a C₁ feature, and its inside/outside inversion is a radial structure that C_s, which retains the sagittal mirror and has no central inversion, does not carry. C_s is the orientation core; FTD-0248's chirality + inversion sit *below* it toward C₁ — the same C_s→C₁ descent §0 qualifier 1 and §3's [IMPOSED] handedness record.) FTD-0248's scope is preserved unchanged: this is an **emergent** property of observer/bound-state trajectories ([SELECTION]-level, spontaneous), not a native substrate symmetry — see §3.

## 3 · Frontier placement [coherent-interpretation]

Here the observation meets FTD's frontier apparatus, and the honest result is sharper than a flattering one would be.

**The substrate is O_h; a polar axis is never native.** The cubic lattice / Moore neighbourhood is O_h-symmetric (order 48) — maximal symmetry, the modulus/even side. A distinguished polar axis appears *only* when an observer selects a trajectory ([SELECTION]), a curl-dominant spin rule picks one (protocol-dependent [OBSERVATION]), or the engine hardcodes z (a flagged coordinate-bias). The substrate's update rules are reflection-invariant; every handedness is corpus-flagged **[IMPOSED]** (weak f_L ≈ 0.978, the neutrino chirality seed). So **bilateral orientation is emergent/state-level, not fundamental** — consistent with the whole frontier thesis that orientation is the imported argument half, never self-supplied.

**Bilateral C_s = phase-orientation = the *reachable* argument-phase.** FTD-0341 §3 proved the load-bearing distinction: the substrate "can choose a **direction (a phase)** but not the **size of the choice (a magnitude)**" — det_ζ and η are the magnitude and phase of one ζ-determinant; native orientations ("i, the arrow, chirality, η, the AGM branch — all **phases**") cannot equal a real magnitude-surd. The bilateral criterion carries exactly a **direction — a ℤ/2 sign, magnitude-free** (§1: C_s classification is blind to *how much* front differs from back; it registers only *that* it does). Therefore:

> **The bilateral pattern instantiates the *reachable* half of the modulus/argument frontier.** It is an empirical, macroscopic witness that movers carry a directional arrow with **no magnitude** — precisely mirroring the substrate, which chooses direction (phase) but not size (magnitude). It *illustrates* FTD-0341's magnitude/phase distinction from an independent (biological/geometric) direction — an analogy that sharpens intuition, not evidence that raises or lowers the theorem's (Chudnovsky-conditional) truth value.

**It does NOT reach δ, and *why not* is the same reason the substrate can't.** δ = √(G*(4G*−1)) is a real **magnitude** (FTD-0341 §4: the imported second factor √(4G*−1)). C_s is magnitude-free by construction, so no bilateral/reflection orientation carrier can supply δ — the identical obstruction the substrate faces. This document therefore **is consistent with, and does not weaken,** the standing verdict: the arrow (FC-2, phase-orientation, achiral C_s) is reachable/declared-native; the magnitude δ (FC-W) is imported. The bilateral criterion illuminates **the arrow, not δ** — and gives the arrow its first empirical/macroscopic characterization.

**Net for the frontier, in symmetry-group language:** high symmetry **O_h = modulus** (self-set, even); the reduction to **C_s = the reachable phase-orientation** (a signed axis, achiral); the surd **√(4G*−1) = the magnitude even C_s cannot supply** — the genuinely imported argument piece. Bilateral orientation is the second of these, not the third.

## 4 · The pre-registered δ / magnitude probe

The user commissioned an adversarial probe of the one place this could have been more: *can the symmetry-reduction itself ever produce the magnitude?*

**Gate (stated before the verdict).** Does the reduction O_h → C_s — or **any** reflection-group reduction the substrate can perform — ever produce a value in ℚ(G\*)(δ) \ ℚ(G\*) (the coset containing δ) rather than only phase/sign/index data?

**Verdict: CLOSED-NEGATIVE** (predicted). The argument has two steps; the second is the load-bearing one, and it is *imported*, not verified here.

*Step 1 — the reduction cannot manufacture transcendence.* A reflection (indeed any element the substrate's O_h symmetry can perform) is an **orthogonal map with algebraic — for the cubic lattice, rational/integer — entries.** Such a map is transcendence-degree-preserving: it cannot produce a transcendental output from algebraic inputs. The *group-theoretic* invariants a reduction reads off are correspondingly algebraic over ℚ (transcendence degree 0): orientation choices = ℤ/2-torsor signs ∈ {±1} ⊂ ℚ; subgroup indices such as [O_h : C_s] = 24 ∈ ℤ; character values ∈ cyclotomic ℚ(ζ_n) ⊂ ℚ^ab (verifier P5b checks one representative of each kind — samples, not the universal). *Metric* invariants of the reduced configuration (lengths, angles, volumes) can of course be transcendental — but only by carrying in a transcendental that was *already imported* (the calibration a_phys, or G\* itself from the even/magnitude sector); the reduction does not *generate* it. That the magnitude content lives in the even sector and the orientation/phase in the odd is exactly the **cited FTD-0341 magnitude/phase theorem** — which is the real completeness source here, not any self-contained enumeration.

*Step 2 — δ is transcendence-degree 1 (imported, conditional on Chudnovsky).* Over ℚ, δ = √(G\*(4G\*−1)) has transcendence degree 1 **conditional on Chudnovsky 1976** (G\* transcendental) — this is assumed, not machine-verified (sympy cannot certify transcendence; verifier P5c encodes it as a stated fact). What P5 *does* verify exactly: δ² = 4t²−t is square-free over ℚ(t) [t ↔ G\*], so δ ∉ ℚ(t) and δ genuinely generates a degree-2 extension.

A transcendence-degree-1 element cannot equal a transcendence-degree-0 one. Hence **no reflection-reduction carrier reaches δ** — given FTD-0341 (step 1's completeness) and Chudnovsky (step 2).

This is FTD-0314's narrowing exclusion restated in symmetry-reduction language, and it yields a genuine sub-result either way the probe fell: **the reflection side (order-2, algebraic) and the CM-magnitude side (order-4, transcendental G\*) are separated by exactly the algebraic/transcendental gap.** That separation is *why* the substrate's rich O_h symmetry — however drastically reduced toward C_s — still cannot reach δ, and it is the same gap that keeps FTD-0355's order-4↔reflection seam open (§5). Self-attested-lock caveat: the gate was stated before evaluation, but in the same artifact as the result; low stakes (exact algebra, no search surface) — same footing as FTD-0381.

## 5 · Order-2 / order-4 and the D=3 seam [OPEN-adjacent, NOT a derivation]

Two structural notes, both flagged as consonance under the FTD-0375 rhyme-not-identification guard.

**The mirror is a ℤ/2 shadow of the order-4 CM spine.** The user's criterion posits a *single* mirror — C_s, order 2. But the substrate's deepest native orientation generator is the **order-4** CM automorphism i (ℤ/4, i² = −1, J² = −I). Every order-2 object in the corpus (the χ₋₄ parity twist, matter/antimatter parity, the four analytic carriers) is a *derived branch* of that order-4 spine: ℤ/4 is cyclic with a **unique** involution i² (verified P4 — unlike the Klein four-group's three), so the mirror is ⟨i²⟩, a shadow, not an independent generator. The bilateral C_s is thus one ℤ/2 face of a ℤ/4 — consistent with "the arrow is a phase, i is native," and a reminder that the single-mirror picture is the emergent/reduced view, not the bottom layer.

**The D=3 seam.** §1's dimension result (exactly-C_s ⟺ D=3) and FTD-0355's arithmetic |B_D|/D = 2^D(D−1)! = |O_h|/3 = 16 (verified P3) are two faces of one fact: *choosing one distinguished axis in the hyperoctahedral reflection group*. FTD-0355 carries a standing **[OPEN]**: no non-circular link between the order-4 CM automorphism (the G\*/magnitude side) and the hyperoctahedral reflection group (the C_s/orientation side). The bilateral criterion lives squarely on the reflection side of exactly that seam, and §4 shows precisely what blocks the crossing — the algebraic/transcendental gap. This is a *sharpening of why the seam is open*, not a bridge across it. Dimension-forcing remains [SELECTION — declared circular]; the D=3/C_s consonance is consonance only.

## 6 · What does not move

FC-W stays [AXIOM]; δ stays imported; MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION]; x₊ = 1/α stays [SMC]. Nothing here derives α, δ, or D=3, and nothing reaches the magnitude side of the frontier. The document **sharpens** FTD-0248 (→ precise point group C_s + empirical anchor), **illustrates** FTD-0341 (→ an independent geometric picture of magnitude/phase), **restates** FTD-0336 (→ the reachable argument-phase, in symmetry-group language) and FTD-0314 (→ its exclusion in reduction-carrier language), and **sharpens why** FTD-0355's order-4↔reflection seam is open — and contradicts none of them. The biology is a tendency with four qualifiers; the group theory is exact; the FTD mapping is [coherent-interpretation].

---

*The answer to "what is an orientation carrier": structurally, an object reduced to the point group C_s = Stab_{O(3)}(v, g) — one signed axis, one surviving transverse mirror, achiral; empirically, a manoeuvring macroscopic mover; in FTD, an emergent phase-orientation on the reachable side of the frontier — the arrow, not the magnitude δ. Everything that moves forward carries a direction; nothing that moves forward carries a size. That is exactly the substrate's own predicament, seen in the mirror of biology.*
