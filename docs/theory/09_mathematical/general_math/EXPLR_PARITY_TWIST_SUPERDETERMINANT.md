# EXPLR — The Parity Twist as a Superdeterminant: det/Ber Structure on the r=4 Sector Pair

**Tag:** [STRUCTURAL OBSERVATION] — elementary exact identities on an external model (CHPS 2018), machine-verified 14/14; frontier readings capped at [coherent-interpretation]
**LEDGER row:** FTD-0381
**Verifier:** `scripts/proofs/proof_parity_twist_superdeterminant.py` (14/14; sympy-exact incl. actual Hankel determinants + one 50-dps cross-check)
**Adversarial review (2026-07-11, same day, AI-simulated):** ftd-math-redteam pass applied before registration — §3 re-scoped to the exact support (the first draft's all-N det/Ber display was false for the matrix object off N ≡ 0 mod 4; the N=1 "anchor" is an amplitude identity, not a determinant), §2's conjugation sentence replaced by the correct Galois-on-weights statement, §5's orientation bit **withdrawn** (the structure is canonically oriented — more rigid than first claimed), §4 scoped to the monomial module. Two lock caveats recorded: the criteria lock is self-attested (same uncommitted artifacts; low stakes — exact identities, no search surface), and the review is not external human review.
**Companions:** [`EXPLR_GSTAR_MATRIX_MODELS.md`](EXPLR_GSTAR_MATRIX_MODELS.md) (FTD-0366 — the sector grading and parity twist this upgrades), [`MATH_REFLECTION_FLOW_PARITY.md`](MATH_REFLECTION_FLOW_PARITY.md) (FTD-0367 — the flow-level sibling), FTD-0127 (the χ₋₄ parity twist), [`ANALYSIS_VERTEX_DK_CLOSURE_v1.md`](../algebra/ANALYSIS_VERTEX_DK_CLOSURE_v1.md) (FTD-0379/0380 — the fermion-closure guard cited in §0)

---

## 0 · Question and locked adjudication frame

**Q:** does the CHPS r=4 monomial matrix model carry a *native* ℤ/2-graded (super) structure under which the χ₋₄-even and χ₋₄-odd combinations of the conjugate sectors (a = 1, 3) arise as the **determinant** and **Berezinian** of a single model-native object — upgrading the FTD-0127/0366 parity twist (product → √2π, ratio → G\*) from bookkeeping to structure?

The trap this question must not fall into is manufacturability: for *any* pair (A, B), the diagonal graded operator diag(A, B) has det = AB and Ber = A/B (verifier P12). So the content lives **only** in three naturality criteria, locked with verdict map and priors before evaluation (recorded verbatim in the verifier header):

- **N1** — the grading character is model-native (no choice made to define it);
- **N2** — the graded object is model-native (same construction, both sectors; residual freedom exactly one declared orientation bit);
- **N3** — an odd operator exists natively (an observable already in the model that exchanges the sectors, anticommutes with the grading, and has a meaningful square).

Verdict map: all three ⇒ STRUCTURAL YES (tag ceiling [STRUCTURAL OBSERVATION]); N1+N2 only ⇒ PARTIAL ([coherent-interpretation], twist stays bookkeeping); N1 or N2 fail ⇒ NON-BRIDGE. Priors: YES 45%, PARTIAL 40%, NON-BRIDGE 15%. Anti-targets (binding): no fermion claim of any kind — FTD-0379/0380 closed native fermion emergence at the tested protocols and a superdeterminant is Grassmann-free bookkeeping; no α/x₊/master-quadratic content; the orientation bit is **not** identified with FC-W's δ bit; no numerical search.

## 1 · Setup (CHPS conventions, inherited from FTD-0366)

Sector-a moment functional (CHPS eq 3.14): φ_a(x^q) = ∫_{C_{4,a}} x^q e^{−x⁴} dx = [4 | q+1−a] · Γ((q+1)/4). The Mellin index q+1 is the model's own label for a monomial: the selection rule is q+1 ≡ a (mod 4) (verifier P1). Sector Γ-classes (P2): a = 1 moments ∈ ℚ·Γ(1/4); a = 3 moments ∈ ℚ·Γ(3/4). Pure-phase partition functions are Andreief moment determinants; this doc works throughout in the **stripped (plain-dx Hankel) convention** det[φ_a(x^{i+j})] = δ_{4,a}(N)·A(4,a,N) with A(4,1,N) ∈ ℚ·Γ(1/4)^N, A(4,3,N) ∈ ℚ·Γ(3/4)^N — CHPS Theorem 1 carries an additional (2π)^{−N} prefactor, declared here and accounted for in §3(ii)'s normalization note (FTD-0366 C3/C10).

## 2 · N1 — the grading character is the Dirichlet character χ₋₄, evaluated on the model's own label [DERIVED]

Define ε(x^q) = χ₋₄(q+1), with χ₋₄ the unique nontrivial character of (ℤ/4)^× (χ₋₄(1) = +1, χ₋₄(3) = −1, zero on evens). Then (verifier P4):

- monomials pairing with sector 1 (q ≡ 0 mod 4) carry ε = +1 = χ₋₄(1);
- monomials pairing with sector 3 (q ≡ 2) carry ε = −1 = χ₋₄(3);
- q ≡ 1 monomials pair with the π-class sector a = 2 and carry ε = 0; q ≡ 3 monomials pair with **no** sector (all three sector moments vanish there) — the character itself declares both classes non-graded.

No choice is made anywhere: the argument q+1 is the Mellin index the moment formula already uses (not a convention of ours); χ₋₄ is the *unique* nontrivial character of the unit group (ℤ/4)^× = {1, 3} (the multiplicative label group — not an additive subgroup of ℤ/4); χ₋₄(q) dies identically on the graded monomials and χ₋₄(q−1) is not a character of the label (it would not fix the identity label), so q+1 is forced. **Sector exchange (corrected after adversarial review):** geometric complex conjugation of the weighted contour chain *fixes* each sector — the sector functionals are real-valued, and a real functional is self-conjugate — so conjugation of the chain does NOT exchange them. What exchanges the pair is the **Galois action i ↦ −i on the ℤ₄-Fourier weights alone**: the sector pair {1, 3} is a Gal(ℚ(i)/ℚ)-orbit of weight systems, which is the more precise form of the χ₋₄/arithmetic reading.

## 3 · The superdeterminant identities [THEOREM-grade, elementary given CHPS Thm 1]

Two statements, kept separate (the adversarial review caught their conflation in this doc's first draft):

**(i) Amplitude level [all N].** The CHPS Γ-amplitudes satisfy A₁A₃ ∈ ℚ·(√2π)^N and A₁/A₃ ∈ ℚ·G\*^N for all N (verifier P5/P6, N ≤ 8 exact), with N = 1 giving exactly Γ(1/4)Γ(3/4) = √2π and Γ(1/4)/Γ(3/4) = G\*. These are *identities about the amplitudes* — the FTD-0366 parity twist itself — **not** determinants of the native object: packaging two numbers as det/Ber of diag(A₁, A₃) is exactly the contentless move the P12 guard names.

**(ii) Superdeterminant proper [exactly N ≡ 0 mod 4].** Let M = M₁ ⊕ M₃ be the *actual* Hankel moment matrices (M_a)ᵢⱼ = φ_a(x^{i+j}), regarded as an even operator on the graded module V₊ ⊕ V₋ (sector 1 even — the orientation is canonical, §5). The Hankel determinants carry the δ-support, so det M and Ber M exist non-degenerately **exactly on N ≡ 0 (mod 4)** (P8; off support the sector-3 block is singular — P6b checks det M₃ = 0 at N = 1). On support (P6b, P7, exact):

$$\det M \;\in\; \mathbb{Q}\cdot(\sqrt{2}\,\pi)^N, \qquad \operatorname{Ber} M \;\in\; \mathbb{Q}\cdot (G^*)^N, \qquad \text{e.g. } \operatorname{Ber} M\big|_{N=4} = \tfrac{1}{48}\,G^{*4},$$

and the δ signs cancel identically in both invariants (δ₁ = δ₃ on the common support). **The bare constants √2π and G\* are attained by the native object at no matrix size** — the native outputs are the ℚ-classes; the exact constants live at the amplitude level (i). Normalization note: under CHPS's own (2π)^{−N} convention the *even* output's class shifts (ℚ·2^{N/2}π^{−N}); the **Berezinian class is convention-invariant** — G\* is the robust invariant of the pair. **π-class is the even (determinant) output; G\*-class is the odd (Berezinian) output of one graded object, at every fourth matrix size.** This is the parity twist as the two canonical invariants of a ℤ/2-graded operator — weaker than the first draft claimed, and exact where claimed.

## 4 · N3 — the odd operator: Q = x², squaring to the action-density insertion [DERIVED]

Multiplication by x² — the model's own quadratic observable, whose normalized expectations FTD-0366 C5 proved lie in ℚ(G\*) — exchanges the two graded **monomial classes** q ≡ 0 ↔ q ≡ 2 (mod 4); for a ±1-grading this *is* anticommutation with ε ({ε, Q} = 0, verifier P9 — the two phrasings are definitionally equivalent, as the review noted, so P9's content is the exchange property itself). Scope precision (review finding): ε and Q act on the *monomial module*; the matrices M of §3(ii) act on the Andreief index space, graded by contour-sector label — the two gradings are linked by the selection rule, not identical, and Q does **not** rationally intertwine M₃ with M₁ (the transport ratios φ₁(x^{m+2})/φ₃(x^m) are themselves ∈ ℚ·G\* — irrational, and itself a G\*-graded fact). Any x^{4k+2}-multiplication exchanges the classes; what singles out Q = x² is minimality plus its square: Q² inserts x⁴ — the action density of S = Tr X⁴ — and acts on the sector moments as the Euler/Γ-recurrence (P10, exact):

$$\varphi_a(x^{q+4}) \;=\; \tfrac{q+1}{4}\,\varphi_a(x^q),$$

which is precisely **integration by parts** — ∮ d(x^{q+1}e^{−x⁴}) = 0 — i.e. the model's own Schwinger–Dyson / string equation. That is N3's real content: *the odd operator's square is the model's Ward-identity operator.* The triple (ε, Q, Q²) has the super-quantum-mechanical *shape* — odd charge squaring to a distinguished even operator — and only that is claimed: no dynamical "Hamiltonian," no Grassmann variables, no fermionic statistics (the anti-target stands; cf. FTD-0379/0380).

## 5 · Orientation: canonical, not free [corrected after adversarial review]

The first draft claimed a residual one-bit orientation freedom (which sector is even), with Ber defined up to inversion (P11 records the trivial algebra: block swap inverts Ber). The review showed this is **inconsistent with N1's own rigidity, in the flattering direction**: sector 1 is the *identity element* of (ℤ/4)^×, every character sends the identity to +1, and the universal super-convention places the +1 eigenspace in the even summand — so the orientation is **forced**, Ber ∈ ℚ·G\*^{+N} absolutely, and no model symmetry can implement the swap (the Galois exchange of §2 relabels contours; the character re-pins the even block to the Γ(1/4) data). Consequence, recorded honestly: the structure is *more* rigid than first claimed, and the first draft's shape-consonance paragraph (Arrow orientation / FTD-0336 argument-half / FC-W δ bit) is **withdrawn — there is no free bit here to be consonant with.**

## 6 · Verdict (per the locked map, at review-corrected scope) and what does NOT move

**N1 ✓ (over-delivered: orientation forced), N2 ✓ in qualified form (the native object exists non-degenerately exactly on N ≡ 0 mod 4; residual freedom zero — within the locked "at most one declared bit"), N3 ✓ under the monomial-class reading, its content being Q² = the Ward-identity operator ⇒ STRUCTURAL YES, restricted and rigid.** The parity twist is upgraded from bookkeeping to structure: *on the χ₋₄-graded moment module of the r=4 monomial model, at every fourth matrix size, the reflection product and ratio **classes** are the determinant and Berezinian of one native, canonically oriented graded operator, and the model's quadratic observable is an odd operator squaring to the model's own string-equation operator.* The exact constants √2π and G\* themselves live at the amplitude level (§3(i)), outside the superdeterminant proper. Tag: [STRUCTURAL OBSERVATION] on an external exactly-solved model — a statement about the model's bookkeeping made structural, not a theorem about FTD's substrate and not physics.

Untouched, explicitly: x₊ = 1/α [SMC] (FTD-0013); MC-T4.3 [FOUNDATIONAL OBSTRUCTION]; FC-W [AXIOM]; FTD-0379/0380 (no fermion content here — a Berezinian is not a fermion); the FTD-0366 non-bridges (48 ≠ |O_h|; no 16G\*² from sector ratios — the anti-target held: neither appears above); the boson/fermion ↔ product/ratio *analogy* stays [coherent-interpretation] — what is now exact is only that the even/odd combinations are det/Ber of one graded object *in this model*.

## 7 · Relation to the reflection-formula reading

This gives the "π is the even-side output, G\* the odd-side output" reading (FTD-0127/0366/0367; the sin/cos tower observation) its first realization as the two invariants of a single graded object rather than two separately-assembled combinations. The natural follow-up questions, left open and unregistered: (i) does the flow-level dichotomy (FTD-0367: c_P differentially algebraic, c_R hypertranscendental) interact with the Q-structure — e.g. is there a graded flow whose even/odd projections are the P/R flows? (ii) does the r = 3 (equianharmonic) sibling carry the analogous structure with (ℤ/3)-characters and Γ(1/3)/Γ(2/3)? Both are exact-algebra questions in the same discipline; neither is claimed here.
