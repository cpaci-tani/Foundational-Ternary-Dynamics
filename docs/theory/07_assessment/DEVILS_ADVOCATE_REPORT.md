# Devil's Advocate Report — Reframe Restatements (2026-04-19)

**Reviewer mandate:** attempt to falsify the six rewrites against `CANONICAL_REFRAME.md` (frozen v1.0) and `AUDIT_INFINITY_REFRAME.md`. Findings below are adversarial; "PASS" is reserved for genuinely clean rewrites.

---

## Executive verdict

| # | File | Verdict |
|---|---|---|
| 1 | FOUND_AXIOM_ZERO.md | **NEEDS-REVISION** |
| 2 | DERIV_MASTER_QUADRATIC_GAP_EQUATION.md | **PASS-WITH-NOTES** |
| 3 | DERIV_PATH_INTEGRAL_CONSTRUCTION.md | **PASS-WITH-NOTES** |
| 4 | DERIV_VON_NEUMANN_CONSTRUCTION.md | **PASS-WITH-NOTES** (cross-doc inconsistency is the main blocker) |
| 5 | DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md | **PASS** |
| 6 | SPEC_FTD.md (P1, DEF.1, §14.2) | **PASS-WITH-NOTES** |

The biggest single risk is **cross-doc inconsistency**: VON_NEUMANN_CONSTRUCTION has been demoted to [HYPOTHESIS], but its dependents (`DERIV_COLLAPSE_MECHANISM.md`, `FOUND_THE_EXISTENCE_FILTER.md`, `FOUND_VON_NEUMANN_CHAIN.md`, etc.) still treat Type III₁ as a working assumption inherited from this document. The file under review now disowns a claim those dependents lean on.

---

## Per-file findings

### File 1: FOUND_AXIOM_ZERO.md

**Verdict:** NEEDS-REVISION.

**F1 (over-application):** None significant. The §3.2 "algebraic identity" framing is consistent with `AUDIT_INFINITY_REFRAME.md` §2.1 (master quadratic SURVIVES as algebra). G* / Watson / Chowla–Selberg references are correctly retained — these are explicitly on the "Permitted Moves" list (Permitted Moves #2, #7).

**F2 (under-application):** Substantial. The boxed Axiom statement on line 17 still says **"Position: x in Z^3"**. The next paragraph (line 36) re-defines this as "undefined-boundary cubic graph." That is a *direct contradiction inside the axiom itself* — the axiom states Z^3 (a completed-totality object) and then immediately overrides it. Per CANONICAL §17 ("classical mathematics uses these objects constantly"), Z^3 is exactly the kind of totalized object the reframe proscribes when used as the domain of a primitive property.

   Further residual Z^3-as-totality language survives at lines 51, 118, 132, 138, 148, 172, 184, 186, 190, 192, 316, 414, 418, 422, 428, 430, 453, 494, 499 — many of these are stylistic, but several are load-bearing:
   - Line 184: "The Moore neighborhood is the unique neighborhood that is … minimal subject to (i) and (ii) **on Z^3** at range 1." This treats Z^3 as a single object on which uniqueness is asserted.
   - Line 186: "consequence of 'position is in Z^3' plus 'interactions are local and symmetric.'"
   - Line 316: "**The Z^3 lattice** with ternary states has a **unique** self-consistent coupling, given by the fixed point of **the gap equation**." This is doubly under-applied — both Z^3 and the gap-equation appeal.

**F3 (internal inconsistency):** Severe and load-bearing.
   - The boxed axiom (line 17) and its prose interpretation (line 36) contradict each other on the most basic ontological statement.
   - §4.2 contains *both* a retraction of the gap-equation (lines 344–353: "The master quadratic is an algebraic identity, not a dynamical limit … finite-L gap-equation scan does not converge") **and**, three sections later (§4.4 line 316, and the one-sentence summary at line 499), the *unretracted* claim that "the Z^3 lattice with ternary states has a unique self-consistent coupling, given by **the fixed point of the gap equation**." The summary at line 499 also says the master quadratic is delivered "through a self-consistent gap equation."
   - §3.2's table (lines 261–271) lists Step 0 correctly as "Cubic lattice with no defined boundary" — good — but Step 7 is now [THEOREM] for the coefficient 16, while §4.2 (line 339) explicitly says "Coefficient 16 is the weakest link" and the [THEOREM] tag on Step 7 contradicts the [SELECTION] gloss on the orbit-stabilizer and |Aut(E)|² routes one paragraph above it. Internal tag-ledger drift.

**F4 (cross-doc):** §3.2's Step 7 lists coefficient 16 as [THEOREM] with `n_DOF = z_BCC × 2 = 16`, while the dedicated rewrite of `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` §2.4 puts the constant term as [THEOREM by Vieta] and §2.2 lists the 16 from two routes. That part is consistent. But Axiom Zero §4.2 still endorses the orbit-stabilizer route (48/3 = 16) which the dependent rewrite **does not** use. Pick one canonical set.

**F5 (logical gap):** §4.5 ("you haven't derived alpha — you've noticed a numerical coincidence") concludes (line 422) "depends on closing the gap at Steps 6-7." But Steps 6–7 are exactly what was ruled out by `AUDIT_INFINITY_REFRAME.md` and by the rewritten master-quadratic doc as a derivation route. The objection's "until-then-it's-a-provocation" framing is now the permanent state of affairs under the reframe; the document still treats it as something to be closed in the future.

**F6 (content preservation):** Algebra and numerical predictions preserved (G*, x±, Vieta, discriminant trichotomy, integers {3,4,7,13}, c=1/√3, |Aut(E)|², z_BCC). Coefficient counts preserved. The discriminant-trichotomy section (lines 275–281) is good — three-regime claim survives as algebra.

**Recommended action:**
1. Rewrite line 17: "Position: x is a site in the undefined-boundary cubic graph" (drop Z^3).
2. Rewrite line 316 and line 499 to delete the "self-consistent gap equation / fixed point" framing — replace with the algebraic-identity language used in the dedicated master-quadratic doc.
3. Sweep the remaining 19 Z^3 occurrences and either replace with "the cubic graph" or qualify with "the cubic-graph geometry (which one may locally model on Z^3 at any specified region)."
4. Reconcile §3.2 Step 7 tag with §4.2 admission that 16 is "weakest link" — the inline [THEOREM] tag and the prose admission are at odds.

---

### File 2: DERIV_MASTER_QUADRATIC_GAP_EQUATION.md

**Verdict:** PASS-WITH-NOTES. The strongest of the six rewrites.

**F1:** None. The algebraic-identity framing correctly invokes Permitted Moves #2 (closed-form algebraic objects) and #7 (Chowla–Selberg). All "limit" language is bracketed in §4.3 as "what is *not* claimed."

**F2:** None load-bearing. The phrase "computable to arbitrary finite precision" recurs and is finitarily clean. No L → ∞ or thermodynamic-limit appears as proof structure.

**F3:** Tag ledger is internally consistent: Steps 0–8 are [THEOREM], Step 9 is [THEOREM/SELECTION] split, Step 10 is [STRONGLY MOTIVATED CONJECTURE]. Part VI summary matches the per-step table. The retraction in §2.2 of the temporal-gauge DOF-count route (24-7-1 = 16) is correctly cross-referenced to AUDIT_MASTER_QUADRATIC.md.

**F4:** Discrepancy with **Axiom Zero §3.2** (which still leans on the orbit-stabilizer 48/3 = 16 route, and gives the Step-7 derivation as `z_BCC × 2`). This rewrite drops 48/3 and uses two routes (|Aut(E)|² and z_BCC × 2). The dependent doc has dropped a route the upstream still endorses — fine for this doc, but the Axiom-Zero side needs to be updated to match. Also, the rewrite cites `OPEN_A_PHYS_DERIVATION.md` and `OPEN_GC_FROM_FIRST_PRINCIPLES.md`; these references should be checked for existence.

**F5:** §4.2 ("CM-curve uniqueness across class-number-1 fields") still leans on the Option-3 scan ("only one survives"). That scan is finite (9 fields), so it is finitarily clean — Permitted Moves #1. No gap.

**F6:** Algebraic content fully preserved: master quadratic, Vieta, discriminant trichotomy, dual-match, CM-curve uniqueness, two routes to 16. Tag downgrades from [THEOREM] to [STRONGLY MOTIVATED CONJECTURE] for the physical identification are stricter, not laxer — appropriate.

**Recommended action:** None blocking. The OPEN_*.md references should be confirmed to exist. Title change ("Algebraic Identity and Physical Match") matches content.

---

### File 3: DERIV_PATH_INTEGRAL_CONSTRUCTION.md

**Verdict:** PASS-WITH-NOTES.

**F1:** Mostly clean. §1.4 IR-finiteness corollary's restatement ("As $L_\mu$ is taken arbitrarily large, the FTD manifestation threshold $K_B > 0$ provides a natural mass gap…") preserves content while avoiding completed-limit language.

**F2:** Two residual issues:
   - Line 281: "In the **continuum limit** $|\mathbf{k}| \ll \pi$." The phrase "continuum limit" is on the proscribed list (CANONICAL §27 #2). Should be restated as "for $|\mathbf{k}| \ll \pi$, the propagator approximates $\delta_{\mu\nu}/k^2$ to relative error O(k²)" or similar.
   - Line 337: "the Ward identity holds exactly on the lattice, **not just in the continuum limit**." This is the same phrase. Both are stylistic, but the rewrite was intended to scrub them.
   - §5.5 line 447: notation $\{F_N\}_{N=1}^{\infty}$ uses a completed-totality index in the symbolic notation; the prose disclaims it ("the family of finite-$N$ free energies is admissible"), but the symbol still totalizes. Consider $\{F_N\}_{N \geq 1}$ or "for every $N$."
   - §5.3 "limiting behavior" subsections still use "$\beta \to 0$" and "$\beta \to \infty$" as completed limits to characterize phases. These should be restated as "for arbitrarily small / large $\beta$."

**F3:** §7.1 claims table ID PI-9 is correctly demoted from [THEOREM] to [SELECTION] for the "phase transition at K_B" — consistent with §5.4 prose. PI-1 through PI-8 still claimed as [THEOREM] — fine if all proofs run on the finite lattice (which they do). PI-12 (Hawking) remains [CONJECTURE]. Internally consistent.

**F4:** §6.5 cross-references DERIV_LATTICE_SCHWARZSCHILD.md (now in archive per the link path `../archive/ARCH_DERIV_LATTICE_SCHWARZSCHILD.md`); the cross-ref is to a deprecated doc. Worth checking whether the Hawking conjecture should now reference a current document.

**F5:** §5.5's "increasingly sharp crossovers" framing is logically clean — it correctly replaces "thermodynamic limit gives true phase transition" with "scaling property of the family." However: the *physical content* of the original phase-transition claim has been weakened; nothing in the new framing tells us whether FTD predicts a Higgs phase transition or only a smooth crossover. The dependent doc `DERIV_HIGGS_FROM_MANIFESTATION.md` may still claim a sharp transition; that needs a downstream sweep.

**F6:** All theorems for Z, W, Γ, KMS, modular Hamiltonian preserved. Numerical/algebraic content unchanged. Section structure intact.

**Recommended action:**
1. Replace "continuum limit" at lines 281 and 337 with finitary language.
2. Replace `{F_N}_{N=1}^∞` index notation with `{F_N}_{N≥1}` or "for every N."
3. Audit DERIV_HIGGS_FROM_MANIFESTATION.md downstream for residual "sharp phase transition" language that this doc no longer supports.

---

### File 4: DERIV_VON_NEUMANN_CONSTRUCTION.md

**Verdict:** PASS-WITH-NOTES — the rewrite itself is internally clean, but it creates a serious cross-doc inconsistency.

**F1:** None. The Type III₁-as-scaffold framing is exactly what `AUDIT_INFINITY_REFRAME.md` §2.3 and §2.4 prescribe. The demotion to [HYPOTHESIS] is appropriate given Powers/Araki–Woods inductive-limit dependency.

**F2:** Section 5.1 still uses "$\Lambda_1 \subset \Lambda_2 \subset \cdots$" and "infinite tensor product." Both are explicitly bracketed as the *scaffold being asked about*, not as a property of FTD — so this is permitted under Distinguishing Question #1 ("scaffold" framing makes it characterization, not definition). Borderline but defensible.

**F3:** Tag ledger is internally consistent: Sections 1–4 are [THEOREM], Section 5 demotes scaffold to [HYPOTHESIS], Section 7 verification table cleanly separates the two. Section 8.4's "what this document does NOT claim" is the cleanest disclaimer in the entire portfolio. Internal pass.

**F4 (cross-doc):** **This is the major issue.** The dependent docs have not been updated:

   - `DERIV_COLLAPSE_MECHANISM.md` (line 28): "The pre-measurement lattice flux field $\mathbf{J}$ lives in a Type III$_1$ von Neumann algebra (no pure states, no definite outcomes)." This is asserted as a working premise, not as a hypothesis-under-the-scaffold.
   - `DERIV_COLLAPSE_MECHANISM.md` (line 320): "The flux field $\mathbf{J}(v) \in \mathbb{R}^3$ at each voxel encodes the full dispositional content. In the operator-algebraic description, the algebra of flux observables **is (argued to be) Type III$_1$**." Parenthetical "argued to be" gives some hedge but the rest of the doc treats Type III₁ as load-bearing.
   - `DERIV_COLLAPSE_MECHANISM.md` (line 511): "**The entire derivation chain rests on one unproven step: that the flux field algebra is Type III$_1$.**" The dependent doc itself flags Type III₁ as the linchpin — and the upstream rewrite has now demoted it to a hypothesis about a scaffold the framework does not commit to.
   - The COLLAPSE_MECHANISM doc still tags claim 7 as [CONJECTURE] and claim 8 as [CONJECTURE], so it doesn't *contradict* the demoted upstream — but it was conjectural before, while the upstream was [SELECTION]; now both are essentially [HYPOTHESIS], and the wording in the dependent ("the algebra of flux observables is Type III$_1$") is sharper than the upstream now permits.

   The cleanest fix is a one-line note in COLLAPSE_MECHANISM stating that the Type III₁ premise is now the scaffold-hypothesis as restated in VON_NEUMANN_CONSTRUCTION.

**F5:** Section 6.3's "hypothesised type transition" cleanly handles the logical-gap risk. The previous "Type III₁ → Type I" claim has been correctly converted to "[HYPOTHESIS] under the scaffold." No new gap created.

**F6:** All finite-region theorems preserved (M_3(C), Type I_3^N, isotony, locality, partial trace, sign function, entropy increase). Numerical verification table in §7 preserves all 17 test entries. Section 9 numerical results from April 11 retained. Content preservation is strong.

**Recommended action:**
1. Add a one-line update to `DERIV_COLLAPSE_MECHANISM.md` line 28 and §5: "Type III₁ is the type the Araki–Woods inductive-limit scaffold *would* assign if applied to FTD; the framework's undefined-boundary ontology does not commit to that scaffold (see DERIV_VON_NEUMANN_CONSTRUCTION.md §5–8)."
2. Same for `FOUND_VON_NEUMANN_CHAIN.md` and `FOUND_THE_EXISTENCE_FILTER.md` if they assert Type III₁ as a working premise.
3. Optionally: a portfolio-level note that all "consciousness as Type III₁" prose is now [HYPOTHESIS], not [SELECTION].

---

### File 5: DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md

**Verdict:** PASS.

**F1:** Not applicable (the document is itself a careful negative result; no over-application possible).

**F2:** None. The dimensional chain works on physical units, not on lattice limits. No completed-infinity language.

**F3:** Internally consistent. Three readings of the result are explicit (§2). The conclusion ("Mechanism γ does not deliver a derivation") matches §3 ("why this does not constitute a derivation") and §5 ("does not deliver a first-principles a_phys"). Tag is [ATTEMPT], appropriate.

**F4:** Cleanly cross-references `OPEN_A_PHYS_DERIVATION.md` for the open problem and §4 for the fallback. Recommends `a_phys ≡ ℓ_P` (Planck-length anchor) — this should be reflected in `SPEC_FTD.md` if adopted; currently SPEC does not endorse a_phys ≡ ℓ_P.

**F5:** No logical gap. The negative result is the conclusion; no further argument to leak.

**F6:** Content preservation is N/A (new doc). The arithmetic at line 50 (`a_phys ≈ 4 × 10⁻⁵⁵ m`) and line 62 (`a_phys ≈ 6.7 × 10⁻⁷ m`) is verifiable; both numbers seem to be order-of-magnitude correct given the inputs cited.

**Recommended action:** Adopt the `a_phys ≡ ℓ_P` recommendation in `SPEC_FTD.md` (currently DEF.1 area does not declare a calibration), or note explicitly that `a_phys` is undeclared.

---

### File 6: SPEC_FTD.md (Postulate 1, DEF.1, §14.2 finite-size effects)

**Verdict:** PASS-WITH-NOTES.

**F1:** None. The new Postulate 1 wording (line 211) and DEF.1 (line 1600) match the canonical undefined-boundary language exactly.

**F2:** §14.2 finite-size note (line 1208–1210) is correctly restated: "convergence claims are stated as scaling laws across L, not as L → ∞ limits." Good. However, §14.2 (lines 1199–1204) still contains the load-bearing claim "**At scales >> lattice spacing**: discreteness effects average out; the effective dynamics become rotationally symmetric; boost invariance emerges from the isotropy of large-scale flux distributions." This is a "completed-large-scale" appeal that is functionally similar to a continuum limit. Per CANONICAL Distinguishing Question #2 (defining a value vs. characterizing behavior), this should be "for any precision ε, there exists L such that the effective dynamics are rotationally symmetric within ε" — not the current "discreteness effects average out" framing.

**F3:** Postulate 1 (line 211), DEF.1 (line 1600), §14.2 (line 1210) all use mutually consistent language ("undefined boundary", "no commitment to completed totality", "arbitrarily large finite extent"). Internal pass.

**F4:** Postulate 1 motivation (line 214) cites "G* via Γ(1/4)" as an admissible algebraic identity — consistent with `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` §2.1 and the canonical Permitted Moves #2/#7. The DEF.1 area is consistent with all dependent docs reviewed.

**F5:** §14.2's "Lorentz invariance emergence" subsection (lines 1186–1204) still presents emergence as if at "large scales" without restating in ε–L form. This leaves a logical gap: if FTD claims Lorentz emergence, it must be a finitary statement under the reframe. The "Open Question" at line 1206 partially absorbs this (cites OPEN.7), so it's not a hard inconsistency.

**F6:** Content preservation is good. The five-postulate structure is preserved, the ledger (DEF.1–DEF.7, ASSUMP.1–7) is intact. Only the wording of P1 / DEF.1 changed substantively.

**Recommended action:**
1. Restate the "at scales >> lattice spacing" passage in §14.2 in finitary ε–L form.
2. Optionally adopt the `a_phys ≡ ℓ_P` calibration from File 5 in DEF.1 or a new DEF.

---

## Cross-file inconsistencies

1. **Coefficient-16 routes diverge.** Axiom Zero §4.2 endorses three routes (|Aut(E)|², orbit-stabilizer 48/3, DOF count). Master-quadratic rewrite §2.2 endorses only two (|Aut(E)|², z_BCC × 2) and explicitly retracts the DOF count. The orbit-stabilizer route is retained in Axiom Zero but absent from the dedicated rewrite. **Pick a canonical set in one document and have the other reference it.**

2. **Type III₁ status drift.** VON_NEUMANN_CONSTRUCTION now [HYPOTHESIS] (scaffold-only). DERIV_COLLAPSE_MECHANISM still asserts "the flux field is Type III₁" as a working premise (lines 28, 320, 482, 511). FOUND_VON_NEUMANN_CHAIN, FOUND_THE_EXISTENCE_FILTER, DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS likely have similar exposure (greps confirm). **Either downstream sweep or insert a one-line caveat in each.**

3. **Gap-equation withdrawal incomplete.** Master-quadratic rewrite explicitly withdraws the gap-equation narrative ("not the L → ∞ limit of any finite-L self-consistency equation"). Axiom Zero §4.4 line 316 and the one-sentence summary at line 499 still attribute the master quadratic to "the fixed point of the gap equation" / "a self-consistent gap equation." **Axiom Zero needs a sweep.**

4. **a_phys disposition not yet in SPEC.** The Mechanism-γ document recommends `a_phys ≡ ℓ_P`; SPEC_FTD.md does not currently declare a_phys. If the recommendation is adopted, DEF section should be updated; if not, the recommendation should be marked as a proposal.

5. **Orphan archive cross-references.** DERIV_PATH_INTEGRAL_CONSTRUCTION.md links to several `../archive/ARCH_DERIV_*.md` files for downstream support of one-loop / Schwarzschild / etc. Whether those archived docs survive the reframe is unclear. **Sweep needed.**

---

## Recommendations to user (prioritized)

**P1 (blocking):** Fix the FOUND_AXIOM_ZERO.md internal contradiction. The boxed axiom on line 17 says "Position: x in Z^3" while the next paragraph says "undefined-boundary cubic graph." These are not equivalent. Pick one — and given the canonical reframe, it must be the latter.

**P2 (blocking):** Sweep `DERIV_COLLAPSE_MECHANISM.md`, `FOUND_VON_NEUMANN_CHAIN.md`, `FOUND_THE_EXISTENCE_FILTER.md`, `DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md` for "Type III₁" assertions and either downgrade them in the same way as VON_NEUMANN_CONSTRUCTION, or insert a one-line caveat that the Type III₁ premise is the scaffold-hypothesis. Otherwise the consciousness portfolio leans on a claim its upstream document has just disowned.

**P3 (blocking):** Remove the surviving "self-consistent gap equation / fixed point" claims from FOUND_AXIOM_ZERO.md (line 316, line 499). The rewrite's own §4.2 contradicts these surviving sentences.

**P4 (medium):** Reconcile the coefficient-16 ledger between FOUND_AXIOM_ZERO.md and DERIV_MASTER_QUADRATIC_GAP_EQUATION.md (orbit-stabilizer route in or out?).

**P5 (medium):** In DERIV_PATH_INTEGRAL_CONSTRUCTION.md, scrub the two surviving "continuum limit" phrases (lines 281, 337), the `{F_N}_{N=1}^∞` totalizing notation (line 447), and the `β → 0/∞` limit framings in §5.3.

**P6 (low):** SPEC_FTD.md §14.2 — restate the "at scales >> lattice spacing" Lorentz-emergence paragraph in finitary ε–L form.

**P7 (low):** Audit downstream consumers of DERIV_PATH_INTEGRAL_CONSTRUCTION.md (notably DERIV_HIGGS_FROM_MANIFESTATION.md) for residual "sharp phase transition" claims that the rewritten §5.5 no longer supports.

**P8 (low):** Decide a_phys disposition — adopt the Mechanism-γ recommendation `a_phys ≡ ℓ_P` in SPEC_FTD.md, or mark as undeclared.

---

## Closing assessment

The two strongest rewrites are **DERIV_MASTER_QUADRATIC_GAP_EQUATION.md** (cleanest) and **DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md** (honest negative result). The two weakest are **FOUND_AXIOM_ZERO.md** (internal contradictions, residual gap-equation language) and the **cross-doc fallout from VON_NEUMANN_CONSTRUCTION** (the rewrite is fine in isolation, but it disowns claims that downstream docs lean on).

The reframe is genuinely a foundational change, and a six-doc patch cannot fully absorb it. The findings above are not signs that the reframe is wrong — they are signs that the deployment is approximately 70 % done. The remaining 30 % is an Axiom-Zero rewrite plus a consciousness-portfolio sweep.

**End of report.**
