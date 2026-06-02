# Readout-Structure Independence — the MC-T4.3 boundary theorem

> **STATUS: [THEOREM] — independence *relative to the admissible FTD-native construction set `𝔉`*
> (explicitly NOT strong forbiddance, per F-i).** Final whole-proof adversarial review **PASSED**
> (2026-05-31, two fresh independent reviewers): the FORCED-construction reviewer exhausted 11
> candidate operators and could build none; the logic/scope reviewer found the argument sound and no
> smuggled premise, flagging two wording over-reaches that are now corrected (the §3/§4 "only odd
> source" lines are qualified "within `𝔉`"; §4.1's L-A strand is marked non-load-bearing). The
> scoped independence verdict survives both. **Draft — pending LEDGER registration + commit (owner
> trigger); FTD-id to confirm against the contended 0238–0243 range.** No spine tag moves:
> `x₊ = 1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`.

**Tag:** [THEOREM] (scoped to `𝔉`; not strong forbiddance)
**Date:** 2026-05-31
**Closure attempt against:** `../preregistrations/PREREG_READOUT_STRUCTURE_INDEPENDENCE_v1.md`
(Leg 3c / FORCED-escape) and `../scopes_and_specs/SCOPE_DET_IDENTITY_ATTACK_v1.md` (Obligation A).
**LEDGER row:** *to assign at commit — next genuinely-free id; grep the whole `docs/` tree first
(0238–0243 contended by concurrent sessions).*
**Depends on (all imported at their canonical tags, none re-derived):** FTD-0122
(`V_complex ≅ Z[i]^2`), FTD-0234 (J-twisted det_ζ ratio = G\*), Watson bridge
(`G*^2/(2π) = G_BCC(0)`), Legs 1–2 (machine-checked here), Leg 3b (reduction-collapse, CONFIRMED).
**Verification artifacts:** `../../../../scripts/proofs/proof_readout_multE_zero.py` (Legs 1–2,
ALL PASS) and `../../../../scripts/proofs/proof_obligation_a_independence.py` (the analytic core
L-A/L-B/L-C, ALL PASS).
**Template:** `THEOREM_A_PHYS_NO_GO.md` (FTD-0059) — ring of derivables → target property
unforced → external input required; independence in the Euclid-parallel-postulate sense.

---

## §1 — Statement

> **Readout-Structure Independence (candidate).** Let `𝔉 = {five postulates} ∪ {algebraic spine}
> ∪ {O_h representation theory of the Moore/BCC module}` be the admissible FTD-native construction
> set, with forward-derived scalars `S = {16 = |μ₄|², G* = det_ζ ratio (FTD-0234),
> G*² = 2π·G_BCC(0) (Watson)}`. Then **no single FTD-native operator co-realizes the master-quadratic
> invariant pair `(Tr, Det) = (16G*², 16G*³)` as *forced* invariants**: the pair is realizable only
> by an external selection `W` (the master-quadratic companion matrix), which is consistent with `𝔉`
> but not derivable from it. Hence the readout operator structure is **logically independent** of `𝔉`.

This resolves **W-CRIT-2** ("master quadratic imposed not derived") as a boundary theorem and seals
the BCC/quantization observable-readout route to MC-T4.3 as `[CLOSED NEGATIVE — boundary]`.

**It is a boundary result, not a derivation of physics.** `x₊ = 1/α` (FTD-0013) is untouched and
remains `[STRONGLY MOTIVATED CONJECTURE]`; the theorem says α's value *cannot be derived* through
this route, not that it is.

---

## §2 — The readout model and proof obligation

The α-readout is modelled (FTD-0122/0231) as a `2×2` operator `T` on `V_complex ≅ Z[i]²` whose
characteristic polynomial is the master quadratic `x² − (Tr T)x + (Det T)`, so the claim
`(Tr T, Det T) = (16G*², 16G*³)` is the operator-structure assertion under test.

By the `THEOREM_A_PHYS_NO_GO` template, independence is the conjunction:
- **Q1 (not forced):** `𝔉` does not force the pair from one operator/preparation; and
- **Q2 (consistent):** the selection `W` fixing the pair is consistent with `𝔉` (exhibited model).

`Q1 ∧ Q2` ⟹ the pair is independent (the parallel-postulate sense). We establish both. Per
falsifier **F-i**, we do **not** claim strong forbiddance (no extension can ever realize the pair) —
that `W` *does* realize it (Q2) is precisely the independence signature.

---

## §3 — Established inputs

- **Leg 1 (trace side, machine-checked).** The 8-corner cube module of `O ≅ S₄` decomposes
  `A₁ ⊕ A₂ ⊕ T₁ ⊕ T₂`; `mult_O(E) = 0` (no `O`-symmetric 2-dim subspace). A definite complex
  structure `i = J` (`J²=−I`) therefore forces breaking `O` to a single `C₄` axis →
  `V_complex ≅ Z[i]²` of **rank 2**, with `C₃(⟨111⟩) ∉ Stab`. (`proof_readout_multE_zero.py`.)
- **Leg 2 (group core, machine-checked).** `⟨C₄(⟨001⟩), C₃(⟨111⟩)⟩ = O` (order 24); any
  stabilizer containing both is all of `O` = unbroken = no localized charge = no `V_complex`.
- **The clean odd source (FTD-0234).** The J-twisted operator `D_a` (spectrum `{n+a}`) has
  `det_ζ(D_a) = √(2π)/Γ(a)`; the ratio `det_ζ(D_{3/4})/det_ζ(D_{1/4}) = Γ(1/4)/Γ(3/4) = G*` at
  **degree 1** — the only odd-`G*` source **within the admissible set `𝔉`** (by enumeration of
  `𝔉`'s menu). *Scope note:* FTD-0237's broader statement that the det_ζ channel is the *unique*
  admissible odd supplier across all channels is tagged `[STRUCTURAL OBSERVATION]`, **not** a
  forcing theorem; this proof relies only on the within-`𝔉` enumeration, not on that broader claim.
- **Leg 3b (reduction-collapse, CONFIRMED).** Any `C₃`-equivariant linear reduction of a
  three-plane / rank-≥3 / infinite det_ζ object onto the rank-2 readout factors through the
  `C₃`-fixed diagonal, where `C₃` acts as the identity, collapsing `G*³ → G*¹`.

---

## §4 — Q1: the pair is not forced (a cluster of independent obstructions)

The odd determinant target `16G*³` is degree 3 in `G*`; the only odd source **within `𝔉`** is
degree 1 (§3). Reaching degree 3 forces three multiplicative factors, and every route to assembling
them on the *same* operator that carries the degree-2 trace fails:

### 4.1 — L-A: Bernoulli–Gamma trace dichotomy
The J-twisted operator that supplies the odd `G*` does so through its **determinant**
(`ζ_T'(0)`, Γ-bearing). Its **regularized traces** are a different regularization of the same
Hurwitz zeta: `ζ(−k, a) = −B_{k+1}(a)/(k+1)` is **rational** at every rational `a`. Exactly
`ζ(−1, 1/4) = ζ(−1, 3/4) = 1/96` (Bernoulli `B₂(a) = a²−a+1/6`), and all power-traces are rational.
So `D_a`'s **own** regularized trace is `G*`-free — `D_a` cannot do double duty as both the odd-`G*`
determinant source and a degree-2 `G*` trace carrier. *(Non-load-bearing strand: this is about
`D_a`'s own trace; the target trace `16G*²` lives on the separate Watson rank-2 operator. The full
co-realization impossibility does **not** rest on this strand — it is independently secured by L-B
and L-C below, which never identify the trace operator with `D_a`.)* (Verified:
`proof_obligation_a_independence.py` L-A.)

### 4.2 — L-B: degree ⟺ multiplicity (no single-operator shortcut)
For ζ-regularized determinants, a spectral power `m` and an eigenvalue multiplicity `m` coincide:
`det_ζ{(n+a)^m} = exp(−m·ζ_H'(0,a)) = (√(2π)/Γ(a))^m = (det_ζ{n+a})^m`. Hence
**`det_ζ = G*³` requires exactly three multiplicative factors** — there is no single-operator route
from the degree-1 source to degree 3. The three factors must be either C₃-permuted three planes
(*excluded* — Leg 3c sealed: requires `C₃ ∈ Stab`, incompatible with Leg 1's single axis) or a
multiplicity-3 / rank-≥3 object (§4.3). (Verified: L-B; the multiplicity-3 J-twist ratio `= G*³`.)

### 4.3 — L-C: rank-2 tautology + the rank/degree clash
For the finite rank-2 readout, `ζ_T(0) = 2` and `det_ζ T = exp(−ζ_T'(0)) = x₊x₋` = the **ordinary**
determinant — the ζ-regularization is **vacuous** at rank 2. So "`det_ζ = 16G*³`" imposes nothing
there; `16G*³ = x₊x₋` holds only by **choosing** the entries (the imposed Vieta target). To make
`det_ζ = G*³` a genuine spectral quantity one must leave rank 2 for a multiplicity-3 / rank-≥3
object — but then (i) its characteristic polynomial is degree ≥ 3, **not** the master quadratic, and
(ii) its trace is the rational Bernoulli value (§4.1) or divergent, **not** `16G*²`. The degree-2
rank-2 trace and the degree-3 determinant sit at **incompatible ranks**. (Verified: L-C.)

### 4.4 — The D=3 candidate does not escape
The strongest FORCED candidate is `D=3` multiplicity (three flux components `J ∈ ℝ³` →
`det_ζ = G*³`). It fails twice over: (a) the three Cartesian components **are** permuted by
`C₃(⟨111⟩)` (the flux is the `T₁` standard rep, `C₃` eigenvalues `{1, ω, ω̄}`), so this route shares
the C₃-orbit structure of the excluded three-plane reading and Leg 3b collapses it; and (b) even
granting multiplicity-3 on the determinant side, that operator's regularized trace is rational
(§4.1) — never `16G*²`. So `D=3` does not force the pair on one operator; selecting "product of three
components" to land the cube is an unforced assembly choice (fires B-1/F-a).

### 4.5 — Q1 conclusion
No single FTD-native operator co-realizes `(Tr, Det) = (16G*², 16G*³)` as forced spectral
invariants. The degree-2 trace lives on the finite rank-2 `Z[i]²` (Watson Green's function); the
degree-3 determinant requires a structurally different rank-≥3 / multiplicity-3 object whose trace
is `G*`-free. **Q1 = the pair is not forced.** ∎(Q1, modulo final review)

---

## §5 — Q2: consistency (the selection W is exhibited)

The master-quadratic companion matrix
`T_W = [[0, −16G*³], [1, 16G*²]]` has `Tr(T_W) = 16G*²` and `Det(T_W) = 16G*³` exactly, with the
master quadratic as characteristic polynomial. It is a concrete `ℂ`-linear operator over the
`V_complex ≅ Z[i]²` structure realizing the prescribed pair. So `𝔉 ∪ {W}` has a model and is
**consistent** (the pair *is* realizable — by a chosen `W`). This is exhibited, not asserted
(F-f). **Q2 = consistent.** ∎(Q2)

---

## §6 — Independence

`Q1` (not forced, §4) `∧ Q2` (consistent, §5) ⟹ the operator-structure selection `W` is **logically
independent** of `𝔉`: not derivable from it, yet consistent with it — exactly as the parallel
postulate is independent of Euclid's other four (`THEOREM_A_PHYS_NO_GO` template). The "extra
factor" `Det/Tr = G*` binding the two invariants is the imposed Vieta relation (W-CRIT-2), a
sixth-postulate-class selection, not a forced det↔det_ζ identity. ∎(modulo final review)

---

## §7 — Honest scope (what is and isn't claimed)

- **Independence is relative to the admissible FTD-native construction set** `𝔉` (Watson rank-2 +
  FTD-0234 det_ζ + `O_h` representation theory). It is **not** strong forbiddance: no claim that
  *no conceivable extension or trace-definition* could realize the pair — `W` demonstrably does
  (§5). That is the independence signature, per **F-i**.
- **No banned move.** `Det = 16G*³` / `Det = Tr·G*` appears only as the characterized target `W`,
  never as a premise (B-1/F-a clear). No multiplicity was chosen to claim FORCED — the multiplicity-3
  determinant was *granted* to the candidate and defeated on the independently-computed trace
  channel (B-2 clear). No master quadratic / α / CODATA inserted as a premise (B-5 clear). The
  obstruction is a co-realizability/degree clash on a **commutative** readout — kept distinct from
  the QM commutativity wall (B-6 clear).
- **Numerics are corroboration only** (F-g). The load-bearing steps are the Bernoulli rationality,
  the degree⟺multiplicity identity, the rank-2 tautology, and the group-theoretic Legs 1–2.

---

## §8 — What this closes (and what survives)

- **W-CRIT-2** ("master quadratic imposed not derived") → **resolved as a boundary theorem.**
  W-CRIT-1 (framework-integer circularity) is informed but not closed by it.
- **MC-T4.3's BCC/quantization observable-readout route** → `[CLOSED NEGATIVE — boundary]`.
- The standing FTD-0234/0235 `[UNDERDETERMINED]` det↔det_ζ verdict → **settled INDEPENDENT.**
- **Surviving positive route** for an α derivation: **ARC-D (engine-native measurement)** or a new
  postulate supplying `W`. Both are outside the admissible set `𝔉` of this theorem.
- **Spine untouched:** `x₊ = 1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`; the
  master quadratic and `G*` remain `[THEOREM]` as pure algebra.

---

## §9 — Provenance of the argument

Established across two pre-registered, GTCA-disciplined multi-agent prove+adversarial-verify
rounds (2026-05-31): the first localized the obstruction to Obligation A; the second (force-via-D3 /
refute-to-INDEPENDENT / neutral analytic, each broken by a fresh skeptic) converged — all three
settle-agents and all three skeptics returned NOT-FORCED, with every reviewer engaging the full
question landing INDEPENDENT (one conservative agent's UNDERDETERMINED was itself reviewed as
*under*-claiming). The two new lemmas (L-A Bernoulli rationality, L-B degree⟺multiplicity) and the
group-theoretic legs were independently re-verified by hand and by the two committed proof scripts.

**Final whole-proof review: PASSED** (2026-05-31, two fresh independent reviewers). The
FORCED-construction reviewer exhausted 11 candidate operators (multiplicity-3 J-twist, weighted
two-sector, Dirichlet-`L`, the literal BCC Laplacian, finite⊗infinite tensor, …) and built none;
the root obstruction it surfaced — `G*` is intrinsically a determinant-channel (`ζ'(0)`) object
whose trace-channel image `ζ(−1)` is a Bernoulli rational, and the `16 = |μ₄|²` multiplier has no
spectral home (it enters as an external `[SELECTION]`) — reinforces the verdict. The logic/scope
reviewer found the argument sound with no smuggled premise; its two flagged over-reaches are
corrected above (§3/§4 qualified "within `𝔉`"; §4.1 L-A marked non-load-bearing). `[THEOREM]`
(scoped to `𝔉`) earned. Pending LEDGER registration + commit (owner trigger).
