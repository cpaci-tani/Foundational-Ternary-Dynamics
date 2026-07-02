# THEOREM — The valuation theorem at the prime (4G\*−1): the native period hull is unramified over the surd's branch locus (FTD-0353)

**Tag:** `[THEOREM — relative to the documented native inventory]` (Theorems 1–3 and the ramification computation, **conditional on Chudnovsky 1976** exactly as spine Theorem 9 / FTD-0112 / the repaired FTD-0244) + `[SELECTION]` (the completeness of that inventory — **explicitly inheriting the FTD-0347 generator-representativeness flag**, see §2.3) + `[SYNTHESIS]` (the "FC-W = adopted ramification" reading, §6).
**LEDGER id:** FTD-0353 (row owned by the controller; **this document does not edit `LEDGER.md`, `META_INDEX.md`, any tracker, or the spine**).
**Executes:** extension **E2** of `../../07_assessment/ASSESSMENT_MATH_GRADES_AND_EXTENSIONS_2026-07-01.md` (FTD-0352), with one recorded correction to the E2 sketch (§3.3).
**Verification artifact:** `scripts/proofs/proof_valuation_4gstar.py` — **51/51 PASS** (mpmath dps 60–500, sympy symbolic layer; PSLQ corroboration at dps 500 with a recorded spurious-PSLQ reproduction at dps 250, mirroring the FTD-0351 incident). SHA256 `be8d2673daa1…`. Read-only / pure mathematics — **golden gate untouched** (`0xb604d81a3d79366e`).
**Depends on:** FTD-0244 (repaired, FTD-0351 — the ℚ(t,u) model and Lemma 1 invariant confinement), FTD-0314 (carrier-narrowing + C1–C3), FTD-0341 (four analytic-orientation carriers + magnitude/phase theorem), FTD-0234 (det_ζ ratio), FTD-0243 §5 (k = 1 non-forcing — **consumed, not subsumed**), FTD-0112 / spine Theorem 9, Chudnovsky 1976.
**Precedence:** LEDGER > `SPEC_FTD_FRAMEWORK_V1.md` (constitution) > this doc.

---

## 0 · Verdict

> **THEOREM (relative to the named inventory), conditional on Chudnovsky 1976.** Model the native period field as ℚ(G\*, π) ≅ ℚ(t, u) and every documented native analytic output as an element of the **native period hull** Ñ = ℚ̄(π^{1/4}, √G\*). Then the surd δ = √(G\*(4G\*−1)) — the sole irrational distinguishing the master-quadratic roots x₊ ↔ x₋ — lies outside Ñ, outside every one-layer multiquadratic extension of ℚ(t,u) by documented native square classes, and outside **every radical tower over the native coordinate monomials**. The single mechanism: every documented native class is a **unit at the prime (4t−1)** (even valuation), while δ² = t(4t−1) has **odd** valuation there. Equivalently: **the fiber over (4G\*−1) is unramified in everything the substrate is documented to build; realizing δ is exactly the adoption of ramification over (4G\*−1) — which is FC-W.**

One statement now yields, at the value level, the closures previously carried case-by-case by FTD-0244 Lemma 2, FTD-0314 C1–C3, and FTD-0341 C1–C4 (subsumption map with honest scoping in §7 — the mechanism-level content of those documents is **not** replaced).

**Nothing promoted.** `x₊ = 1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`; FC-W (FTD-0315) stays an adopted `[AXIOM]`-class commitment — this theorem *pins* the import more precisely, it does not earn it; **no α is derived anywhere**. Per the FTD-0346 discipline: Theorems 1–3 are **PROVEN** structural no-gos within stated assumptions; the inventory-completeness premise is the **ATTEMPTED**-side residue and is tagged as such.

---

## 1 · Setup

**The model `[THEOREM, conditional on Chudnovsky 1976]`.** By Chudnovsky 1976, {π, Γ(1/4)} are algebraically independent over ℚ. Since G\* = Γ(1/4)²/(π√2) (Euler reflection; verified A1), the repaired FTD-0244 Lemma 2 step (1) gives the isomorphism

$$F := \mathbb{Q}(G^*, \pi) \;\cong\; \mathbb{Q}(t, u), \qquad t \mapsto G^*,\; u \mapsto \pi,$$

a rational function field in two variables. Algebraic independence over ℚ is equivalent to algebraic independence over ℚ̄, and each of π^{1/4}, √G\* is algebraic over ℚ̄(π, G\*) and conversely; hence {π^{1/4}, √G\*} are also algebraically independent over ℚ̄, giving the second model used below (§4):

$$\widetilde{N} := \overline{\mathbb{Q}}\big(\pi^{1/4}, \sqrt{G^*}\big) \;\cong\; \overline{\mathbb{Q}}(w, s), \qquad w \mapsto \pi^{1/4},\; s \mapsto \sqrt{G^*}, \quad\text{so } t = s^2,\; u = w^4.$$

**The valuation.** (4t−1) is a prime of ℚ(u)[t] (linear in t, hence irreducible over any coefficient field). Let v = v_{(4t−1)} be the associated discrete valuation of F: v(f) = (multiplicity of (4t−1) in the numerator of f) − (multiplicity in the denominator). v is a group homomorphism F^× → ℤ, trivial on ℚ(u); its residue field is ℚ(u) (t ↦ 1/4). Verified: v(t) = v(u) = v(2) = v(3/7) = 0 and v(t(4t−1)) = v(4t−1) = 1 (C3–C5).

**The target.** The master quadratic x² − 16G\*²x + 16G\*³ = 0 (`[THEOREM]`, FTD-0001) has roots x± = 8G\*² ± 4G\*·δ with δ = √(G\*(4G\*−1)) = 5.66183351260… the sole irrational separating them: δ = (x₊−x₋)/(8G\*) (verified A2–A3). Under the model, δ² ↦ t(4t−1), squarefree (C1–C2). FTD-0243 (`[THEOREM]`, conditional) established that binding α requires a law W realizing a beable in F(δ) \ F; this document proves how far the documented native tower stays from that extension.

---

## 2 · The native inventory (the honest enumeration)

### 2.1 Layer V — native values (no roots taken)

The substrate-native operator calculus ℭ has all traces and determinants in F = ℚ(G\*, π) — **FTD-0244 Lemma 1 (as repaired under FTD-0351)**. This layer is closed under field operations, so it contains e.g. the *value* 4G\*−1 itself. Note carefully: v(4G\*−1) = 1 is **odd** (C5) — Layer V freely produces elements of odd valuation **as values**. What no documented native process does is *take the square root* of such an element; that distinction is the entire content of the theorem (and is the FTD-0340 sqrt-as-act boundary in valuation language: the four field operations are single-valued and native; an unforced branch choice is not).

### 2.2 Layer R — documented analytic outputs (the roots the substrate does take)

Every documented native analytic output is a ℚ̄-multiple of a **monomial s^a w^b** (a, b ∈ ℤ) under s = √G\*, w = π^{1/4}. Each row's identity is machine-verified at 40–58 digits (check ids in the last column):

| Documented output | Value | Hull form (∈ ℚ̄·s^a w^b) | Established in | Verified |
|---|---|---|---|---|
| det_ζ(D_{1/4}) (Lerch) | √(2π)/Γ(1/4) | 2^{1/4}·s^{−1} | FTD-0234 | B1 |
| det_ζ(D_{3/4}) | √(2π)/Γ(3/4) | 2^{1/4}·s | FTD-0234 | B2 |
| det_ζ ratio (even route) | G\* | s² | FTD-0234 / spine Thm 1 | B3 |
| Watson self-energy G_BCC(0) | G\*²/(2π) = Γ(1/4)⁴/(4π³) | s⁴/(2w⁴) | OT-2.1 / spine Thm 5; FTD-0314 C1 (twist branches equal, both = this value's line) | B4–B5 |
| θ₃(0, τ=i) | π^{1/4}/Γ(3/4) | 2^{−1/4}·s·w^{−1} | FTD-0341 C2 | B6–B7 |
| θ₂(0,i) = θ₄(0,i) | self-dual pair | 2^{−1/2}·s·w^{−1} | FTD-0341 C2 | B8 |
| η(D_a) = ζ_H(0,a) − ζ_H(0,1−a) | 1−2a ∈ ℚ | ℚ ⊂ ℚ̄ (a = b = 0) | FTD-0341 C1 | B10–B11 |
| half-derivative eigenvalues | G\*^{±1} | s^{±2} | FTD-0323 / FTD-0341 C3 | B14 |
| AGM(1, √2) | 2√π/G\* | 2·w²·s^{−2} | FTD-0327 / FTD-0341 C4 | B12 |
| AGM orientation vector | i·4√2·G\* (convention caveat: i·4G\* in one reproduction — **both** are ℚ̄·s², so hull membership is convention-robust) | ℚ̄·s² | FTD-0341 C4 | B13 |
| lemniscate constant ϖ | G\*√π/2 (defining integral checked) | s²w²/2 | spine Thm 1 context | B15 |
| CM period Ω of E: y²=x³−x | Γ(1/4)²/√(2π) | s²w² | FTD-0314 C3 | B16 |
| L(E,1) | G\*√π/8 (**value cited from FTD-0314 C3, not re-derived here**; only hull membership checked) | s²w²/8 | FTD-0314 C3 | B17 |
| quadratic-twist injections | √n, n ∈ ℤ | ℚ̄ | FTD-0314 C3 | — |

Since ℚ̄ contains i, √2, 2^{1/4}, √n, and F = ℚ(s², w⁴) ⊂ ℚ̄(w, s), the compositum of Layer V with every Layer-R output embeds in the hull Ñ = ℚ̄(w, s). `[DERIVED]` (row-by-row; each identity verified).

**Exclusions, stated plainly.** (i) The FCC/SC "second Watson integrals" (Γ(1/3)-, Γ(1/24)-content) are **not** in the inventory: FTD-0314 C2 closed them on *forcing* grounds (no forced lattice-symmetry pairing; the engine stencil is BCC-orthogonal, FTD-0050/0079) before any value question arose. Honesty note: had they been native, extending this theorem to cover them would require joint algebraic independence of {π, Γ(1/4), Γ(1/3)} — to our knowledge **open**, beyond both Chudnovsky 1976 and Nesterenko 1996 — so the theorem deliberately does not claim that case. (ii) Roots of the (1+i)-tower quadratics (spine Theorem 8 family) are **targets, not outputs** — K-BIND's point is precisely that no native operator realizes them as eigenvalues; they do not enter the hull. (iii) Unforced square roots of arbitrary Layer-V values are excluded **by the boundary itself** (FTD-0340): that exclusion is not a convenience but the thing being characterized.

### 2.3 Completeness of the inventory: `[SELECTION]`, inheriting FTD-0347

That §2.1–§2.2 exhaust "what the substrate natively constructs" is a **judgment, not a theorem**. It is the same genus of assumption as FTD-0244's generator set 𝒮 (representativeness **FLAGGED** by the FTD-0347 provisional specialist review, deliberately left open, "needs a human Galois/transcendence specialist") and FTD-0341's "four most natural carriers" (§6 residue ~10%, honestly `[OPEN]`). **This theorem inherits that flag explicitly and resolves nothing about it.** Everything below is proven *relative to* the table above; the theorem's honest reading is "the documented native tower cannot reach δ," not "no native tower can." The same restatement discipline as FTD-0347 applies here in advance: a reader of this document alone must not take Theorem 2's clean statement as an unconditional closure of the FTD-0314 §4 loophole — the loophole survives exactly as the inventory-enlargement clause (§8).

---

## 3 · Theorem 1 — the F-level valuation parity (the E2 core)

**Definition.** Let H ≤ F^×/(F^×)² be the subgroup generated by {[q] : q ∈ ℚ^×} ∪ {[t], [u]}. H contains every F-square-class the documented tower realizes a square root of at its first quadratic layer: [1] (η, half-derivative values), [t] (the G\*-line), [u] and [2u] (the √π / √(2π) line, e.g. Ω/G\* = √π, B18; [t²/(2u)] ≡ [2u], C8–C9), [2t] and [−2] (AGM data, either convention), [n] (twists).

**Lemma 1.1 (parity homomorphism) `[THEOREM]`.** v induces a well-defined homomorphism v̄ : F^×/(F^×)² → ℤ/2 (since v(f²) = 2v(f) ≡ 0). Every generator of H is a v-unit: v(q) = v(t) = v(u) = 0 (verified C3; randomized closure check C6). Hence **v̄ ≡ 0 on all of H**. But v̄([t(4t−1)]) = 1 (C4). Therefore

$$[\,t(4t-1)\,] \notin H \quad\text{and equally}\quad [\,4t-1\,] \notin H \;\;(\text{since } [t(4t-1)] = [t]\cdot[4t-1] \text{ and } [t] \in H).$$

**Lemma 1.2 (multiquadratic membership; classical Kummer-type lemma) `[THEOREM]`.** Let F₀ be any field of characteristic ≠ 2 and a₁, …, aₙ, c ∈ F₀^×. Then √c ∈ F₀(√a₁, …, √aₙ) **iff** [c] ∈ ⟨[a₁], …, [aₙ]⟩ in F₀^×/(F₀^×)².

*Proof.* (⇐) immediate. (⇒) Induction on n. n = 0: √c ∈ F₀ ⟺ [c] = [1]. Step: L = F₀(√a₁,…,√a_{n−1}). If √aₙ ∈ L the tower collapses and the inductive hypothesis (applied also to c = aₙ) concludes. Otherwise [L(√aₙ) : L] = 2 with L-basis {1, √aₙ}; write √c = x + y√aₙ, x, y ∈ L. Squaring: c = (x² + aₙy²) + 2xy·√aₙ, and c ∈ L forces xy = 0. If y = 0: √c ∈ L, induct. If x = 0: √(c·aₙ) = y·aₙ ∈ L, induct on c·aₙ; then [c] = [c·aₙ]·[aₙ] ∈ ⟨[a₁],…,[aₙ]⟩. ∎

**Theorem 1 `[THEOREM, conditional on Chudnovsky 1976]`.** For every finite family h₁, …, hₙ ∈ F^× with classes in H,

$$\delta \notin F\big(\sqrt{h_1}, \dots, \sqrt{h_n}\big).$$

*Proof.* Otherwise Lemma 1.2 gives [δ²] = [t(4t−1)] ∈ H, contradicting Lemma 1.1. ∎

This is the E2 "five-line core" made exact: no **one-layer** multiquadratic native extension reaches δ, because every documented class is a unit at (4t−1) and δ's class is not.

### 3.3 Recorded correction to the E2 sketch

The FTD-0352 assessment's E2 paragraph listed the θ-null square class as "[t]·[u-adjacent algebraic]" and the Watson value's class as "[u]". Precisely: **θ₃(0,i) is a second-layer root** — [F(θ₃) : F] = 4 with quadratic layer F(√(2u)) of class **[2u]**, not [t] (θ₃² = G\*/√(2π) ∉ F, θ₃⁴ = G\*²/(2π) ∈ F; verified B9, C8–C9) — and the Watson value's class is [t²/(2u)] ≡ **[2u]** (the dropped [2] is immaterial since [2] ∈ H). Theorem 1 alone therefore does **not** cover the θ-null tower; that is exactly why Theorems 2–3 (the hull) are needed, and why this document supersedes the sketch rather than transcribing it. FTD-0341's own "[G\*] square class" bookkeeping for the θ-nulls tracked the s-parity (odd s-exponent) of the output — consistent with the hull picture, and agreeing with it on the only fact that matters here: **zero (2s±1)-content**. E2's conclusion is unaffected.

---

## 4 · Theorem 2 — the native period hull is δ-free

**Theorem 2 `[THEOREM, conditional on Chudnovsky 1976]`.** Let Ñ = ℚ̄(π^{1/4}, √G\*) ≅ ℚ̄(w, s) (§1). Then:

1. δ ∉ Ñ. In particular δ lies in **no** compositum of documented native outputs (Layer V ∪ Layer R ⊂ Ñ, §2), at **any** depth of iterated quadratic layers within Ñ.
2. [Ñ(δ) : Ñ] = 2, with Gal(Ñ(δ)/Ñ) = {1, σ}, σ(δ) = −δ, hence σ(x₊) = x₋.
3. **Hull-level Galois blindness:** every documented native quantity — operator invariants, det_ζ values, θ-nulls, η, AGM data, CM periods — is fixed by σ and is therefore blind to the root swap x₊ ↔ x₋. This strictly extends FTD-0244's operator-level blindness to the entire documented analytic layer in one statement.

*Proof of (1).* Under the model, membership δ ∈ Ñ means there is f ∈ ℚ̄(w, s) with f² = s²(2s−1)(2s+1) (the image of δ² = t(4t−1) under t = s²; the factorization 4t−1 = (2s−1)(2s+1) is D1–D2). Write f = A/B with A, B ∈ ℚ̄(w)[s], B ≠ 0. Then A² = s²(2s−1)(2s+1)·B². The element p = 2s−1 is irreducible in ℚ̄(w)[s] (degree 1 in s) and divides neither s² nor 2s+1. Counting p-multiplicities: the left side has even multiplicity 2·mult_p(A); the right side has odd multiplicity 1 + 2·mult_p(B) (D3). Contradiction. ∎

*Proof of (2), (3).* δ ∈ ℝ exists, δ ∉ Ñ, and δ is a root of y² − t(4t−1) over Ñ, irreducible by (1) (corroborated D4–D6: irreducible over ℚ[y,s] and stable over ℚ(i), ℚ(√2), ℚ(i, 2^{1/4})). The nontrivial automorphism sends δ ↦ −δ; x± = 8t² ± 4tδ swap; Tr = 16t² and Det = 16t³ are δ-free (D7). Every element of Ñ is σ-fixed by definition. ∎ (PSLQ corroboration at dps 500: no integer relation between δ and the hull monomials s^a w^b, a,b ≤ 3 — evidence, not proof; G3, with the dps-250 spurious-relation incident recorded in the script.)

---

## 5 · Theorem 3 — stability under all radical towers over the native monomials

**Theorem 3 `[THEOREM, conditional on Chudnovsky 1976]`.** For every m ≥ 1, δ ∉ ℚ̄(w^{1/m}, s^{1/m}); hence δ lies outside the full coordinate-radical hull ⋃_{m≥1} ℚ̄(w^{1/m}, s^{1/m}) — i.e. **granting the substrate every m-th root of every ℚ̄-multiple of every native monomial, for all m simultaneously, still never reaches δ.**

*Proof.* Set S = s^{1/m}, W = w^{1/m}; ℚ̄(W, S) is again a two-variable rational function field (powers of algebraically independent elements are algebraically independent). Substituting s = S^m: δ² = s²(4s²−1) = S^{2m}(2S^m−1)(2S^m+1) = S^{2m}(4S^{2m}−1). The polynomial 4S^{2m}−1 ∈ ℚ̄[S] is squarefree (its derivative 8mS^{2m−1} shares no root with it, since S = 0 is not a root of 4S^{2m}−1) and coprime to S (verified F1–F2 for m = 2, 3, 4; the gcd computations are uniform in m). Hence over ℚ̄(W)[S], δ² has multiplicity exactly **1** at each linear factor of 4S^{2m}−1 — odd — so δ² is not a square in ℚ̄(W, S), and δ ∉ ℚ̄(W, S). The union is directed (m | m′ gives inclusion), so a membership in the union is a membership at some finite level. ∎

**Geometric reading `[DERIVED]`.** Radicals of coordinate monomials ramify only over the *coordinate cross* {s = 0, ∞} ∪ {w = 0, ∞}. δ's branch locus is {s = ±1/2} — the fiber {4G\*−1 = 0} — **disjoint from the cross**. No amount of native root-taking along the monomial directions can create ramification at a point transverse to them. This forecloses wholesale the "iterate the θ-null trick" family of future carrier proposals: any carrier whose output is a radical of a native monomial (as all four FTD-0341 carriers were) is covered in advance.

---

## 6 · The ramification structure: FC-W as adopted ramification at (4G\*−1)

The exact ramification data of the double cover Ñ(δ)/Ñ (all verified, E1–E6):

| Prime | multiplicity of δ² = s²(2s−1)(2s+1) | e in Ñ(δ)/Ñ | Reading |
|---|---|---|---|
| (s) — the G\*-line origin | 2 (even) | 1 — **unramified** | the native √G\*-line *absorbs* the (t)-ramification: over F, v_{(t)}(δ²) = 1 was odd (FTD-0244's route, E5); after the documented t = s² it is even |
| (2s−1), (2s+1) — the fiber over (4t−1) | 1 each (odd) | **2 — ramified** (Newton slope 1/2, E1–E2) | the obligation W must discharge |
| every w-prime | 0 | 1 | the π-direction never interacts with the surd (E6) |
| s = ∞ | deg 4 (even) | 1 | unramified at infinity (E4) |

Three consequences, one picture:

1. **The native line gets exactly halfway, in a precise sense.** Over F, *both* primes (t) and (4t−1) ramify in F(δ). The documented native tower **uniformizes (t)** (the θ/det_ζ line supplies s = √t) and **splits (4t−1)** into (2s−1)(2s+1) (D1) — factoring the prime **without ramifying it**. FTD-0341 §4's "missing factor √(4G\*−1)" is exactly √((2s−1)(2s+1)): the square root of the split pair. `[THEOREM]` (the computations) + `[SYNTHESIS]` (the "halfway" phrasing).
2. **Which prime is the stable obstruction is now principled, not conventional.** FTD-0244's repaired Lemma 2 used v_{(t)}; FTD-0314 C3 used "degree-1 in π" via the ad-hoc prime 4Γ(1/4)² − √2π. The first obstruction is *native-erasable* (dies in the hull); the second is the (4G\*−1)-fiber in (π, Γ(1/4))-coordinates — verified exactly: 4Γ(1/4)² − √2π = √2·π·(4G\*−1) (G1–G2). One geometric fact — the branch locus {4G\* = 1} — previously carried in three vocabularies. **(4t−1) is the invariant formulation**: it is the unique documented obstruction that survives every documented adjunction. `[THEOREM]`
3. **FC-W = adopted ramification at (4G\*−1).** Adjoining δ ramifies **exactly** the fiber over (4G\*−1) and nothing else. The disciplined import FC-W (FTD-0315) — previously pinned as "a forced order-2 twist on a G\*-bearing analytic object realizing the surd" (FTD-0314 §5) — is pinned one notch tighter: **it is the adoption of the double cover of the native period hull branched precisely over the vanishing locus of 4G\*−1.** The substrate can produce the *value* 4G\*−1 (Layer V; v odd, C5) and can split its prime (D1); what it is not documented to do, and what every documented mechanism provably fails to do, is ramify it. `[SYNTHESIS]` (the slogan), resting on `[THEOREM]`-grade computations.

---

## 7 · Subsumption map (honest scoping)

**Subsumed at the value/square-class level** — one theorem now yields each of these as a corollary:

| Prior result | What Theorems 1–3 recover | What is *not* replaced |
|---|---|---|
| FTD-0244 Lemma 2 (δ ∉ ℚ(G\*, π), [K:F] = 2) | odd v at (4t−1) (or at (t)) gives non-squareness; Theorem 2 restricted to F ⊂ Ñ | **Lemma 1** (invariant confinement) is an *input* (§2.1), not a corollary; **FTD-0243 §5's k = 1 non-forcing is untouched and not subsumed** |
| FTD-0314 Narrowing Theorem, finite-symmetry clause | all ℚ̄-valued carriers: ℚ̄ ⊂ Ñ and δ ∉ Ñ | the narrowing *argument* (transcendence-degree collapse) remains the cheaper proof of its own clause |
| FTD-0314 C1 (BCC twist) | value side: both branches lie on the G\*-line ⊂ Ñ | the **exact degeneration identity** G_odd = G_even is stronger mechanism-level content, retained |
| FTD-0314 C2 (second Watson) | value side: 4G\*−1 ∈ F with odd v — "having the value ≠ having the root" is now the theorem's own §2.1/§6 distinction | the Gate-4 forcedness closure (BCC-orthogonality of the stencil); the Γ(1/3)/Γ(1/24) hypothetical stays outside (§2.2 exclusion (i)) |
| FTD-0314 C3 (CM period/L-value route) | Ω, L(E,1), twists ∈ Ñ (B16–B17); C3's ad-hoc prime = the (4G\*−1) fiber (G1) | the CM-theoretic provenance of those values |
| FTD-0341 C1–C4 (η, θ, half-derivative, AGM) | all four outputs are hull monomials (B1–B14); Theorem 3 covers their radical iterates in advance | **Gate-4 forcing analyses** and the **magnitude/phase theorem** (FTD-0341 §3) — the *mechanism* explaining why orientation outputs are rational/unimodular is complementary, not subsumed; this theorem consumes the outputs, that one explains them |
| FTD-0314 §4 / FTD-0341 §6 loophole | restated exactly: a native output with **odd valuation at a prime over (4t−1)** | the loophole itself — **unchanged, still `[OPEN]`** (~10% per FTD-0341 §6; the genesis-cokernel pre-registration remains the pending attempt) |

**Also not subsumed:** FTD-0242 (route-invariance, an *inductive* no-go over attempted routes — different epistemic genus); the weight-0 correction flagged in FTD-0341 §5 (owner sign-off still pending; this document takes no position on it and uses only square-class arguments, which both sides of that flag agree are the operative ones).

---

## 8 · Robustness, and the single falsifier

**Robust enlargements `[DERIVED]`.** The theorem survives verbatim under: (i) any enlargement of ℚ̄-coefficients (constants never touch the fiber); (ii) any enlargement by outputs **algebraically independent of G\*** — e.g. if the nome e^{−π} of the θ-evaluation is itself admitted as a native output, Nesterenko 1996 ({π, e^π, Γ(1/4)} algebraically independent) makes it a third independent hull coordinate and (2s±1) remain prime; the conditionality upgrades Chudnovsky → Nesterenko and nothing else changes; (iii) any enlargement by further ℚ̄-monomials in (s, w) or their radicals (Theorem 3).

**The single dangerous enlargement = the single falsifier.** The only way this theorem fails is the exhibition of a **native, forced** output whose valuation at a prime over (4t−1) is **odd** — equivalently, a native period square-class containing the factor [4G\*−1]. This is *identical* to FTD-0314 §4's surviving loophole and FTD-0341 §6's residue; the three documents now share one falsifier, stated in one language. Such an exhibit would simultaneously: falsify the inventory-completeness `[SELECTION]` (§2.3), open the W-carrier door, and — if its order-2 stabilizer is forced rather than hand-placed — earn W natively. Per FTD-0314 §4, supplying value and forced ℤ/2 *co-fitted* is the banned W-CRIT-2 hand-placement; the falsifier must arrive forward-derived.

---

## 9 · Non-promotion

`x₊ = 1/α` stays `[SMC]` (FTD-0013). MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`. FC-W stays an adopted `[AXIOM]`-class commitment (FTD-0315) — more precisely pinned, not earned. FTD-0242/0243/0244/0314/0341 are consolidated at the value level, **not** altered, weakened, or re-tagged; their mechanism-level content stands. The FTD-0347 generator-representativeness flag and the FTD-0341 §5 weight-0 owner sign-off remain open and are inherited, not resolved. The algebraic spine is untouched. **No α is derived anywhere in this document.**
