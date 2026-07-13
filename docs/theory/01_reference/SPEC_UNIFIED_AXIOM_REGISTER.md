# SPEC — The Unified Axiom Register and the Unified Conditional Statement (Stage U1)

**Tag:** [SYNTHESIS] — consolidates the framework's postulates, commitments, and imported types into a single register and states the content that register licenses, each row at its tag of record. Introduces no new results; adopts nothing; modifies no epistemic tags.
**LEDGER id:** FTD-0386 · **Date:** 2026-07-13 · **Charter:** `SCOPE_CONSUMPTION_PROGRAM.md` §5.5 (the Unification Annex), Stage U1. Stage U0's pricing-rule decisions ride Arcs 2–4 and gate the Stage-U2 candidates.
**Data:** [`unified_axiom_register.json`](unified_axiom_register.json) (canonical) · **Verifier:** `scripts/proofs/proof_unified_register.py`.
**Precedence:** LEDGER > constitution > this document > other prose. On L2/L3/L4 membership and on all prices and falsifiers, `import_ledger.json` (FTD-0371) is authoritative; this register must reconcile to it exactly (§4).

---

## §0 · Scope and interpretive constraints

The framework's commitments have to date been stated across several documents: the postulates in the constitution, the framework commitments in five separate declarations, the selections and calibrations in the import ledger, the clock hypothesis in `SPEC_FTD_LAGRANGIAN.md` §4.3, and the model posits within their respective analysis documents. This document consolidates them into a single register (§1), with membership verified against the constitution's inventory and the import ledger by the §4 verifier and known gaps recorded in place. §2 then states, in one table, the content the register licenses.

Three interpretive constraints govern the document. First, every §2 row whose premises extend beyond P1–P5 + FC-0 is conditional on the entries named in its `given` column and is never tagged bare `[THEOREM]`. Second, the designation **FTD-U** is reserved by the charter for the Stage-U3 deliverable, which is defined only once at least three of the §3 candidates have been adopted; no candidate is adopted at the time of writing, and the present register licenses neither general relativity, nor QCD dynamics, nor the Born rule, nor any cosmological observable (§3). Third, an adoption is registered as an adoption and is never presented as a derivation (FTD-0242 §8).

## §1 · The register

**L0 — The five postulates** `[AXIOM]` (constitution §1.1): P1 discrete undefined-boundary space · P2 discrete time (absolute substrate simultaneity) · P3 ternary states, J-primary · P4 Moore-local causality · P5 determinism (not reversibility).

**L1 — The five framework commitments** (declarations; falsification criteria in constitution §6.2 except where noted):

| FC | Content | Tag of record | Ledger line |
|---|---|---|---|
| FC-0 | the ℤ[i] reading — the spine is forced by P1–P5 **together with FC-0**, not by the ontology alone | [AXIOM]-class | SS-2 (self-set side) |
| FC-1 | **declines** the measurement map M; the M-independence theorem (`THEOREM_COMMUTATIVITY_INDEPENDENCE.md` — this citation carries the known FTD-0243↔0189 id double-booking, cleanup owner-pending) establishes both branches consistent; FC-1 selects the branch | [AXIOM-class — FTD-0255] | DEC-1 |
| FC-2 | arrow native; global reversibility **declined**; Lorentzian metric emergent-IR, sector-scoped | [AXIOM-class — FTD-0256] | DEC-2 |
| FC-3 | scale-ratio covariance (only internal ratios are physical) | [AXIOM]-class (FTD-0304) | — (⚠ no falsification criterion in the constitution; recorded gap) |
| FC-W | the adopted α-binding law: an external ℤ/2 twist realizing √(G\*(4G\*−1)); consequence: x₊ = 1/α becomes **[CONDITIONAL THEOREM given W]**, never bare [DERIVED]. Declared-but-conditional: full commitment requires W's carrier to force independent structural content ([OPEN], constitution §3.5; owner-pending) | [AXIOM-class — FTD-0315] | IMP-B1 (the single adopted bit) |

**L2 — The four selected types** `[SELECTION]` (prices and falsifiers at their import-ledger lines): IMP-S1 D=3 `[SELECTION — declared]` · IMP-S2 the singlet · IMP-S3 the ℭ generator set · IMP-S4 A_μ = 𝒫_T J_μ.

**L3 — The three calibrations** `[IMPOSED — calibration]`: IMP-K1 a_phys ≡ ℓ_P (under the electron-primary gauge, [DERIVED ~0.19%] — FTD-0137/0385) · IMP-K2 t_phys = ℓ_P/(√3·c) = t_P/√3 · IMP-K3 K_B = m_e.

**L4 — The four named external results**: IMP-C1 Chudnovsky 1976 `[EXTERNAL — proven]` · IMP-C2 CM h=1 `[NUMERICAL FACT, h=1 only]` · IMP-C3 E1 `[OPEN — exported as Problem P1 of REF_EXPORTED_PROBLEMS_E1_E2.md]` · IMP-C4 E\*/E\*\* `[OPEN]`.

**L5 — The four dynamics posits** (one standing; three conditional-input, each consumed only by its named §2 row):

| Posit | Content | Tag of record |
|---|---|---|
| ACT-1 | the second-order flux-wave action (Δ_t J)², standing, flux-sector-scoped per FC-2's arrow half; consumed by U-9/U-12 | [AXIOM — flux-wave-sector-scoped] |
| CLK-1 | the coordinate-level clock hypothesis (Lagrangian §4.3); FTD-0208 v3 CLOSED NEGATIVE: not substrate-derivable | [AXIOM — coordinate-level, independent] |
| CLK-2 | the rest-mass Klein–Gordon clock of FTD-0271 (A0: the native flux is massless; default-OFF toggle) | [IMPOSED/SELECTION], not [FORCED] |
| MDL-1 | the Langevin noise model, under which PL-1 is a theorem *of the model* | [IMPOSED — model] |

**Recorded gaps.** (i) CLK-1, CLK-2, MDL-1, and ACT-1 carry no import-ledger line; their pricing awaits the D5 currency ruling (an import-ledger v1.2 revision). (ii) The engine-level [IMPOSED] dynamics inputs — the latency field L(x), the dual-substrate L/R decomposition, and the threshold kinetics K_GENESIS = N_c·K_MANIFEST (constitution §1.4/§3.2) — likewise carry no register line; same trigger. (iii) The FC-3 falsification-criterion gap noted above.

**Non-members.** DEC-1 (M) and DEC-2 (reversibility) are declined commitments, held as falsifiable bets. The five chartered candidates P6C-F/G/C/M/U of §3 are not adopted and appear in no layer above. That list covers only the §5.5 Stage-U2 candidates; **P6C-R** (kinematic frame-invariance) remains chartered at charter §2/AM-5 as the minimal candidate on the DEC-2 bet.

## §2 · The Unified Conditional Statement

Each row states content licensed by the register, at its tag of record. The `given` column lists the register entries the row depends on; a row may also cite a priced import-ledger line that is not an axiom (the empirical bridges IMP-E1/E2/E3), in which case the clause records a priced empirical match rather than register-licensed content.

| # | Content | Given | Tag of record |
|---|---|---|---|
| U-1 | the algebraic spine (G\* identity, master quadratic, CM arithmetic uniqueness, Watson, geometric Coulomb, harmonic tower, π-freeness) | L0 + FC-0 (+ IMP-C1 for the transcendence clauses) | [THEOREM] — 7 of 9 per spine §0; Theorem 4's forcing [SELECTION — declared, PERMANENT]; Theorem 7 conditional; the d=−4 dual-match privilege [NUMERICAL FACT — not a proof] |
| U-2 | N_c = 3 (four independent topological routes) | L0 + FC-0 | [THEOREM] |
| U-3 | the Moore-layer decomposition (octahedron + cuboctahedron + stella octangula), read physically as gauge-group slots U(1)×SU(2)×SU(3), 3×4 fermions, 17 dark states | L0 | combinatorial decomposition [THEOREM]; the physical reading **[SELECTION]** (LEDGER FTD-0028: "theorem-shaped argument; physical reading is selection") |
| U-4 | x₊ = 1/α at 1.26 ppm | L0 + FC-0 + FC-W + IMP-C1 + IMP-E1 | constitution §3.5: the mathematical consequence **[CONDITIONAL THEOREM given W]**; the physical identification is the priced bridge IMP-E1 and remains **[SMC]** (FTD-0013) — the two statements stand together; the continuum-limit route (Theorem V.1) is [Conditional THEOREM] on C1–C5, and even granting all five, FTD-0013 remains [SMC] (L→∞ ill-posed under the undefined-boundary ontology) |
| U-5 | the singlet, and Bell/Tsirelson S = 2√2 conditional on it | IMP-S2 + IMP-E3 | singlet [THEOREM, given Lemma 5]; Lemma 5 [SELECTION]; the bare substrate satisfies S ≤ 2 [THEOREM]; deriving Lemma 5 is [CLOSED DECLINED] under FC-1 |
| U-6 | detection statistics: Rice, not Born; the PL-1 deviation ladder with its five-observable discriminator | MDL-1 | Born scaling [CLOSED NEGATIVE in the 6-neighbour substrate; canonical 26-neighbour status OPEN] (FTD-0200); Rice [NUMERICAL FACT — 6-neighbour construction; 26-neighbour generalization [CONJECTURE]] (R² = 0.9923 vs 0.7137; FTD-0258 PL-1); deviation law [THEOREM — of the imposed Langevin model] + [CONJECTURE — manifestation↔detection] (FTD-0359) |
| U-7 | pilot-wave structure: de Broglie matter wave and a Schrödinger-*form* envelope scaling (s ≈ 2); guidance measured ABSENT | CLK-2 | [CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT] (FTD-0271); not an unconditional derivation; the unconditional result is the boundary FTD-0270 [MEASURED — BOUNDARY] (linear dispersion) |
| U-8 | Newton leading order and the Schwarzschild **g₀₀ component only** to leading order; α_G(e,e) = (m_e/m_P)² → 1.745×10⁻⁴⁵ vs 1.752×10⁻⁴⁵ (0.38%) | L0 + FC-0 + L3 + CLK-1 | [DERIVED] modulo the clock hypothesis; prediction floor [SMC] (FTD-0015); the **g_rr proof is RETRACTED** (FTD-0361), full Schwarzschild remains imported (pending P6C-G); G_N = 1/100 FALSIFIED (FTD-0131) |
| U-9 | time dilation: the residual law and its blind L=257 extension (7/9, PREDICTION_CONFIRMED) | L0 + ACT-1 | [MEASURED — blind extension] (FTD-0268); FTD-0252 [OBSERVATION]; depends on the posited second-order flux action; the metric *reading* of these measurements is the FC-2 commitment, not a derivation |
| U-10 | the Planck naming theorem: c·t_P = ℓ_P holds exactly, and uniquely among the √3-ladder namings, in the edge gauge (at the emergent wave speed); the naming is doubly selected | IMP-K1/K2 | [SYNTHESIS] + [DERIVED — formalization] (FTD-0385) |
| U-11 | the empirical bridge catalog (~131 [PARAMETRIC] + ~50 external) | IMP-E2/E3 | per-row tags in CATALOG_PARAMETRIC_INSERTIONS.md; exactly one identification is scan-rigid (U-4), where scan-rigidity denotes a tolerance-conditioned [NUMERICAL FACT] (FTD-0319), not a structural or Bayesian result |
| U-12 | confinement signatures: Wilson-loop area law at the **inserted** coupling β = x₋ (σ = 0.209 of record) | L0 + ACT-1 + IMP-E3 | [MEASURED at an inserted coupling [SELECTION]] (constitution §5.1 row 9); LEDGER FTD-0025: [THEOREM-within-compact-U(1)-LGT framework, PARAMETRIC at substrate level] — the gauge framework is imported and β is inserted, not derived; no substrate derivation is held (pending P6C-C); the FTD-0217 *retraction* (FTD-0384) is the named prior art for the Front-D P5 priced no-go |

## §3 · Chartered candidates (awaiting owner adopt-or-decline rulings; Stage-U0 pricing gates unmet)

Each candidate is subject to the Stage-U0 gates (D5/D6/D7) and the FC-W pipeline (narrowing theorem → independence proof → owner ruling). DECLINED and CLOSED NEGATIVE are admissible terminal outcomes.

| Target | Candidate | Status at time of writing |
|---|---|---|
| general relativity entire (full nonlinear EFE) | P6C-G | the Deser bootstrap *completes a posited* spin-2 field (FTD-0189 step-0 correction); the substrate spin-2 mode is [OPEN]; g₀₀ only today (U-8), g_rr RETRACTED (FTD-0361) |
| QCD dynamics / a confinement derivation | P6C-C | inserted-coupling engine signatures only (U-12) |
| Born sharpness / QM statistics beyond Rice | P6C-M | Rice of record (U-6); wholesale M adoption barred as a Front-B move (AM-5) |
| matter as axiom-grade content | P6C-F | Branch-B import via IMP-S4 at g² = 1/x₊; native fermion emergence [CLOSED NEGATIVE at every protocol tested] (FTD-0379/0380; M1 v2 named follow-up) |
| cosmological observables | P6C-U | none derived |

## §4 · Verification and maintenance

`python scripts/proofs/proof_unified_register.py` recomputes: the layer counts; every `ledger_ref` against `import_ledger.json` (the L2/L3/L4 layers must equal that file's selected-type, calibration, and named-result lines exactly); the absence of the §3 candidates from all layers; the resolution of every §2 `given` reference to a register entry or a priced import-ledger line; and the prohibition of bare `[THEOREM]` tags on beyond-core rows. **Maintenance triggers:** a change to the constitution or the import ledger; a LEDGER-booked adoption (a Stage-U2 adoption enters as an adoption — its P6C line moves from §3 into the register with a new LEDGER row; tags are never edited in place); and the D5 currency ruling, which prices the L5 posits and the §1 recorded gaps (import-ledger v1.2).

*Standing invariants: x₊ = 1/α remains [SMC]; MC-T4.3 remains [FOUNDATIONAL OBSTRUCTION]; FC-W remains [AXIOM]; D=3 remains [SELECTION — declared]; the clock hypothesis remains [AXIOM]; no tag moves.*
