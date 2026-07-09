# MATH — The period-conjecture frame of FTD's import boundary

**Tag:** `[SYNTHESIS]` — a period-theoretic *framing* of FTD's import boundary. It restates, in the precise vocabulary of periods and the Grothendieck period conjecture, **which** transcendental data the substrate must import (G\*, δ) and **on exactly what proven ground** that import rests. It introduces no new theorem and **promotes no tag**.
**LEDGER id:** FTD-0375 · **Verifier:** [`scripts/proofs/proof_period_import_frontier.py`](../../../../scripts/proofs/proof_period_import_frontier.py) — algebraic identities only; transcendence facts are **cited to Chudnovsky 1976, not "verified."**
**Sits with:** the priced-import ledger ([`SPEC_IMPORT_LEDGER.md`](../../01_reference/SPEC_IMPORT_LEDGER.md), FTD-0371 — the *price* of the boundary); the δ-independence program ([`SCOPE_DELTA_INDEPENDENCE_PROGRAM.md`](../../02_foundations/SCOPE_DELTA_INDEPENDENCE_PROGRAM.md), FTD-0368) and its delivered verdicts ([`ANALYSIS_DELTA_IND_CLOSURE_v1.md`](../../02_foundations/ANALYSIS_DELTA_IND_CLOSURE_v1.md), FTD-0369, δ∉N PROVEN-CONDITIONAL; [`THEOREM_RAMIFICATION_LOCUS.md`](../../02_foundations/THEOREM_RAMIFICATION_LOCUS.md), FTD-0370); the modulus/argument frontier ([`FOUND_MODULUS_ARGUMENT_FRONTIER.md`](../../02_foundations/FOUND_MODULUS_ARGUMENT_FRONTIER.md), FTD-0336 — the *qualitative* boundary); and the open exported problems ([`REF_EXPORTED_PROBLEMS_E1_E2.md`](REF_EXPORTED_PROBLEMS_E1_E2.md), E1/E2).

---

## 0 · Reading guards (mandatory — this node sits one algebraic step from several standing tags)

This node tightens the *mathematics* of the boundary; it moves **no** tag. Read every statement through these guards:

- **Not a spine theorem.** A1/A4 are **citations of existing/external results**, not new entries to `SPEC_ALGEBRAIC_SPINE.md` (still nine numbered results). A1 = Spine Theorem 1 (FTD-0002).
- **No physics promoted.** `x₊ = 1/α` stays **[STRONGLY MOTIVATED CONJECTURE]** (FTD-0013); the master-quadratic operator assembly stays **[SELECTION]** (FTD-0242 route-invariant no-go; FTD-0244 K-BIND; see the 2026-07-09 reconciliation of `DERIV_MASTER_QUADRATIC_FROM_PERIOD_ALGEBRA.md`); **MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION]**; **no α is derived anywhere.**
- **A4/A5 strengthen only the *negative* side of MC-T4.3.** They sharpen *why* δ is unreachable natively; they supply **no** exit from the obstruction, positive or partial. More transcendence-rigor about the wall is **not** progress toward removing it.
- **FC-W stays an adopted [AXIOM].** Nothing here makes the δ-import derived or cheaper (see §3 and the containment de-fang). The priced-import ledger's "1 adopted bit" reading guard is unchanged.
- **Closure-independence ≠ logical independence.** "δ ∉ native closure N" is a period-ring/closure statement; it is **not** stated in proof-theoretic terms (δ-IND program §5 guard 1, inherited here).
- **"Motivic Galois group" (A4) ≠ "the master quadratic's Galois group."** A4's object is a Tannakian/period-theoretic group (dim 2, of the CM motive). The corpus's other "Galois group" — RSI Leg 3 / FTD-0243, the ℤ/2 splitting-field group of `x²−16G*²x+16G*³` — is an unrelated elementary object. Same name, different objects; no result about one transfers to the other.
- **Scope of "Chudnovsky-proven ground."** The phrase covers only the transcendence corridor feeding MC-T4.3/FC-W (δ-independence, K-BIND, carrier-narrowing, ramification locus). It supports **no** physics identification and must never be read as "FTD's physics is proven." This node touches none of the engine benchmarks, Bell/CHSH, the dispersion ceiling, the spin-2 boundary, or cosmology.

---

## 1 · G\* as a CM period — the elementary tower `[THEOREM, elementary]`

**A1 (= Spine Theorem 1 / FTD-0002, restated).** By the Euler reflection formula `Γ(1/4)Γ(3/4) = π/sin(π/4) = π√2`,
$$G^* \;=\; \frac{\Gamma(1/4)}{\Gamma(3/4)} \;=\; \frac{\Gamma(1/4)^2}{\pi\sqrt 2} \;=\; \frac{2\varpi}{\sqrt\pi},$$
where `ϖ = Γ(1/4)²/(2√(2π))` is the lemniscate constant. This is elementary and machine-verifiable (60 digits, verifier §A1). **`G* ≈ 2.9587 ≠ ϖ ≈ 2.6221`** — G\* is a Γ-quotient in the CM-period class, *not* the lemniscate constant itself (FTD-0117).

**A2 `[THEOREM, classical]`.** `ϖ` is a genuine Kontsevich–Zagier period: the real **half**-period of the CM curve `E_lemn: y² = x³ − x` (the full real period is `2ϖ = 2∫₁^∞ dx/√(x³−x) = √2·K(1/√2)`). An algebraic integral over a semi-algebraic cycle of a ℚ-variety is a period by definition.

**A3 — ring membership, stated honestly `[THEOREM]` (algebra) + `[OPEN]` (strict-ring).** `G*² = 4ϖ²/π` is a period divided by an *integer* power of π, hence in the extended ring `𝒫[1/π]`. `G* = 2ϖ/√π` additionally carries `√π = Γ(1/2)`, the canonical **exponential** period (the Gaussian `∫_{−∞}^∞ e^{−x²}dx`), *not* a known classical KZ period. Whether G\* lies in the **strict** KZ ring `𝒫` versus the extended `𝒫[1/π]` is **[OPEN]**, entangled with the conjecture that `1/π` is not a period. **Nothing FTD uses depends on it:** the load-bearing facts are the *transcendence* of G\* and δ (A4/A5, Chudnovsky), which are period-ring-membership-independent.

---

## 2 · The Grothendieck period conjecture is a *theorem* for this one motive `[THEOREM — external, cited]`

**A4.** For the single CM motive `h¹(E_lemn)` (CM by `ℚ(i)`), the motivic Galois group is the **full** CM torus `G_mot = MT = Res_{ℚ(i)/ℚ} 𝔾_m` — a 2-dimensional torus (the Hodge/special-MT subgroup is the norm-1 subtorus `U¹`, dim 1; the weight cocharacter adds one, giving `dim = 2`; `G_mot = MT` unconditionally for abelian varieties by Deligne, "Hodge = absolute Hodge"). The Grothendieck period-conjecture **equality** holds **unconditionally**:
$$\operatorname{trdeg}_{\mathbb Q}\big(\text{period field of } h^1(E_{\mathrm{lemn}})\big) \;=\; \dim G_{\mathrm{mot}} \;=\; 2.$$
- **Upper bound `trdeg ≤ dim`** is automatic: the period torsor `Isom^⊗(H_dR, H_B)` is a `G_mot`-torsor over `ℚ̄`, and the comparison point's Zariski closure has dimension `≤ dim G_mot` (Huber–Müller-Stach, *Periods and Nori Motives* Ch. 13; André, §23–24).
- **Lower bound `trdeg ≥ 2`** is **Chudnovsky 1976** (algebraic independence of `Γ(1/4)` and `π`).
- **`2πi` adds no generator:** it is the determinant of the period matrix (Legendre relation), i.e. the period of `⋀²h¹ = ℚ(−1)`, already inside the Tannakian category `⟨h¹(E_lemn)⟩^⊗`.

Citations: Chudnovsky 1976; Deligne (`G_mot = MT` for abelian varieties); Huber–Müller-Stach / André (automatic inequality); the CM instance as the *settled* case of an otherwise-open conjecture — Fresán–Jossen, *Exponential Motives*; Kawabe, arXiv:2303.05030.

**Scope wall (mandatory).** This is the **single-motive** GPC. The **general** Grothendieck period conjecture is **open**. A4 does **not** extend to any larger Tannakian package containing δ or G\* jointly with independent data, and it does **not** force the master-quadratic operator assembly (guard §0). "Unconditional" here means **"no assumption beyond the proven Chudnovsky theorem"** — consistent with the spine's standing conditionality on Chudnovsky, *not* "independent of Chudnovsky."

---

## 3 · Where δ lives, and where it does not

**A5.** Let `δ = √(G*(4G*−1))` (FC-W's imported datum; the discriminant surd selecting the α-root: `x₊ = 8G*² + 4G*δ`).

- **`δ² = 4G*² − G* ∈ ℤ[G*] ⊂ ℚ(G*)` `[DERIVED]`** (elementary; verifier §A5).
- **Field tower (corrected).** Over the elliptic period field `F₀ = ℚ̄(ϖ, π)` (trdeg 2): `G*² = 4ϖ²/π ∈ F₀`, but `G* = 2ϖ/√π ∉ F₀` — G\* generates a quadratic (`√π`) extension `F₁ = F₀(G*)`, and δ is at most quadratic over `ℚ(G*)`, i.e. **at most degree 4 over `F₀`**. So δ is **not** "inside the trdeg-2 field"; it sits one √π-step, then one further quadratic step, above it.
- **δ is transcendental over ℚ `[THEOREM — conditional on Chudnovsky]`:** δ algebraic ⇒ `δ² = 4G*²−G*` algebraic ⇒ G\* algebraic, contradicting Chudnovsky.

**Containment de-fang (mandatory).** "δ is algebraic over the CM period field" is a field-membership fact about the **ambient algebraic closure** — true of *every* algebraic number — and by itself proves **nothing** about native accessibility. The load-bearing results are the **exclusion** theorems that place δ **outside** the native closure `N` and outside the Kummer hull `Ñ`: **FTD-0353** (δ ∉ Ñ, Chudnovsky-conditional), **FTD-0369** (δ ∉ N, PROVEN-CONDITIONAL), **FTD-0370** (ramification locus, whole √-family). **FC-W remains an adopted `[AXIOM]`; this node moves it zero distance toward `[DERIVED]`.**

**Conditionality tier (critical — do not collapse).** The `δ∉N` verdict does **not** rest on Chudnovsky alone. **Only** the hull result (R1) and the BCC-sector sub-theorem are Chudnovsky-only; the **general** native-closure exclusion additionally carries the **open** assumptions **E1** (SC/FCC Watson-class independence) and **E2/E\*\*** (exponential-period independence, Schanuel-adjacent), priced as separate open rows **IMP-C3 / IMP-C4** in the import ledger. State which tier (hull/BCC-only vs general-`N`) on every invocation of "Chudnovsky, not the open conjecture."

**Ownership.** The `δ∉N` proof is **not** this node's and is **not** a corollary of A4 — it is the delivered FTD-0369/0370. This node only *locates* δ period-conjecture-relative and points at the existing proof.

---

## 4 · The frame, priced `[SYNTHESIS]`

**Chudnovsky double-counting note (mandatory).** GPC-for-`E_lemn` (A4, `trdeg = 2`) and `δ∉N` (FTD-0369) rest on the **same** arithmetic input — **Chudnovsky 1976**. `trdeg = 2` *is* Chudnovsky (algebraic independence of `π, Γ(1/4)`); FTD-0369's δ∉N is also Chudnovsky-conditional (E0). So the period-conjecture framing is a **re-description** of the boundary's transcendence substrate, **not** independent corroboration of δ∉N. Presenting A4 as *extra evidence* for the import boundary would double-count Chudnovsky.

**What the frame actually contributes — it names the arena.** A4 (clean, unconditional-mod-Chudnovsky) settles the **transcendence** arena: `trdeg` is exactly 2. The boundary claim `δ∉N` lives in the larger **closure/ramification** arena, which borders the **general (open)** GPC and additionally needs open E1/E2. Therefore **A4 is the transcendence *substrate under* the boundary — explicitly not the closure of the arena where δ∉N is proved.** The import surface is priced in FTD-0371 (Chudnovsky proven + CM-`h=1` + E1/E2 open + the δ bit); **this node re-prices nothing.**

---

## 5 · Refuted framings kept out (standing guard) `[CLOSED NEGATIVE]`

Four Grothendieck-adjacent framings were externally refuted (adversarial deep-research + a four-lens panel, 2026-07-09) and are recorded here so they do not re-emerge:

1. **Topos-as-context.** A Grothendieck topos is the category of sheaves on a site (a generalized *space*); FTD is a single fixed cubic lattice — no site, no sheaves. "Topos = context" is metaphor, not structure.
2. **"Motivic derivation" of constants.** The general period conjecture's *completeness* layer (`trdeg = dim` in general) is open; the machinery is a transcendence **framework**, not a derivation **engine**. (This is exactly why `DERIV_MASTER_QUADRATIC_FROM_PERIOD_ALGEBRA.md`'s "unique derivation" claim was reconciled to `[SELECTION]`, 2026-07-09.)
3. **Modulus/argument-as-literal-period-structure.** The actual Betti/de Rham period structure does **not** decompose as a `|z|e^{iθ}` modulus/argument split. **Reconciliation with FTD-0336 (sharpens, does not contradict):** the modulus/argument frontier's "the split is exact, not analogical" refers to the **exact even/odd Euler-reflection parity** (`Γ(z)Γ(1−z)=π` even vs `Γ(z)/Γ(1−z)=G*` odd), which *is* exact — a **different object** from a claimed strict-period-ring-membership decomposition of period integrals. The frontier's exactness stands for the parity; only the period-integral-structure reading is refuted.
4. **Rising-sea-is-type-priority.** Grothendieck's "rising sea" is a generality-first *method*; it carries zero commitment to FTD's specific objects (G\*, CM curves), so it is a temperamental resonance, not a structural derivation of type-priority.

**Sokal–Bricmont calibration.** FTD's period/CM/motive vocabulary passes the "conceptual justification" bar — the terms carry their real technical content (A1–A5). The philosophical Grothendieck alignment (relative point of view, functor-of-points, graded conviction) is genuine **conceptual resonance**, held as analogy, not structural identity.

---

## 6 · Honest boundary and verifier `[grounded]`

- **What this is:** a `[SYNTHESIS]` framing that restates known/external mathematics in period-conjecture vocabulary and pins exactly what the import boundary rests on. It proves nothing new and promotes nothing.
- **External-review flag:** produced inside an AI session with a four-lens adversarial panel (math / corpus-coherence / tags / physics-boundary). The **A4 attribution** ("GPC is a theorem for CM elliptic curves, via Chudnovsky") rests on secondary literature + argument reconstruction, **not** a reading of Chudnovsky's 1976 original; confirm against the primary paper + a modern reference (Huber–Müller-Stach or Fresán–Jossen) before A4 is cited as `[THEOREM — external]` in any outward artifact.
- **Verifier** [`scripts/proofs/proof_period_import_frontier.py`](../../../../scripts/proofs/proof_period_import_frontier.py) checks only the algebraic identities of §1/§3 (A1 to 60 digits; `G*² = 4ϖ²/π`; `δ² = 4G*²−G*`; `x₊ = 8G*²+4G*δ`; the master-quadratic residual). Transcendence facts (A4, A5) are **cited to Chudnovsky 1976, not "verified"** — no near-miss or coincidence search is performed.
