# SPEC — The Unified Axiom Register + the Unified Conditional Statement (Stage U1)

**Tag:** [SYNTHESIS] — assembles the types FTD has set or bought into one register and states the content that register licenses, each row at its tag of record; **introduces no theorem, adopts nothing, promotes nothing.**
**LEDGER id:** FTD-0386 (booked in the same commit that lands this spec) · **Date:** 2026-07-13 · **Charter:** `SCOPE_CONSUMPTION_PROGRAM.md` §5.5 (the Unification Annex), Stage U1 — the immediate assembly stage; U0's pricing-rule sittings ride Arcs 2–4 and gate the Stage-U2 purchases.
**Data:** [`unified_axiom_register.json`](unified_axiom_register.json) (canonical) · **Verifier:** `scripts/proofs/proof_unified_register.py`.
**Precedence:** LEDGER > constitution > this doc > other prose. On L2/L3/L4 membership and all prices/falsifiers, `import_ledger.json` (FTD-0371) is authoritative and this register must reconcile to it exactly (§4).

---

## §0 · What this is, and the sentence that keeps it honest

Every prior statement of FTD's commitments was scattered: postulates in the constitution, commitments across five FC declarations, selections and calibrations in the import ledger, the clock hypothesis in a Lagrangian section, the imposed-model posits inside their analysis docs. **This is the first assembly of the axiom system in force** — membership verified against the constitution's inventory and the import ledger by the §4 verifier, known gaps booked in-line (§1 notes), not hidden — and the Unified Conditional Statement (§2) is the first single-page statement of what that system licenses.

> **Reading guard.** Every §2 row whose `given` goes beyond P1–P5 + FC-0 is `[CONDITIONAL given the named entries]` — never bare `[THEOREM]`. The charter reserves the name **FTD-U** for the Stage-U3 deliverable, which exists only once ≥3 purchases are adopted; **zero purchases are adopted today**, and this register licenses no GR, no QCD dynamics, no Born rule, and no cosmology (§3). An adoption is never a derivation (FTD-0242 §8 pricing rule).

## §1 · The register (the axiom system in force)

**L0 — The five postulates** `[AXIOM]` (constitution §1.1): P1 discrete undefined-boundary space · P2 discrete time (absolute substrate simultaneity) · P3 ternary states, J-primary · P4 Moore-local causality · P5 determinism (not reversibility).

**L1 — The five framework commitments** (declarations; kill lists in constitution §6.2 except where noted):

| FC | Content | Tag of record | Ledger line |
|---|---|---|---|
| FC-0 | the ℤ[i] reading — the spine is forced by P1–P5 **+ FC-0**, not by the ontology alone | [AXIOM]-class | SS-2 (self-set side) |
| FC-1 | **declines** the measurement map M; the M-independence theorem (`THEOREM_COMMUTATIVITY_INDEPENDENCE.md` — citation carries the known FTD-0243↔0189 id double-booking flag, cleanup owner-pending) establishes both branches consistent; FC-1 picks the branch | [AXIOM-class — FTD-0255] | DEC-1 |
| FC-2 | arrow native; global reversibility **declined**; Lorentzian metric emergent-IR, sector-scoped | [AXIOM-class — FTD-0256] | DEC-2 |
| FC-3 | scale-ratio covariance (only internal ratios physical) | [AXIOM]-class (FTD-0304) | — (⚠ no constitution falsifier; honest gap booked, not repaired here) |
| FC-W | the adopted α-binding law: external ℤ/2 twist realizing √(G\*(4G\*−1)); consequence: x₊ = 1/α becomes **[CONDITIONAL THEOREM given W]**, never bare [DERIVED]. Declared-but-conditional: earns full commitment only if W's carrier forces independent structural content ([OPEN], constitution §3.5; owner-pending) | [AXIOM-class — FTD-0315] | IMP-B1 (the 1 adopted bit) |

**L2 — The four selected types** `[SELECTION]` (prices + falsifiers at their import-ledger lines): IMP-S1 D=3 `[SELECTION — declared]` · IMP-S2 the singlet · IMP-S3 the ℭ generator set · IMP-S4 A_μ = 𝒫_T J_μ.

**L3 — The three calibrations** `[IMPOSED — calibration]`: IMP-K1 a_phys ≡ ℓ_P (under electron-primary, [DERIVED ~0.19%] — FTD-0137/0385) · IMP-K2 t_phys = ℓ_P/(√3·c) = t_P/√3 · IMP-K3 K_B = m_e.

**L4 — The four named external results**: IMP-C1 Chudnovsky 1976 `[EXTERNAL — proven]` · IMP-C2 CM h=1 `[NUMERICAL FACT, h=1 only]` · IMP-C3 E1 `[OPEN — exported as Problem P1 of REF_EXPORTED_PROBLEMS_E1_E2.md]` · IMP-C4 E\*/E\*\* `[OPEN]`.

**L5 — The four dynamics posits** (one standing, three conditional-input — the latter consumed only by their named §2 rows):

| Posit | Content | Tag of record |
|---|---|---|
| ACT-1 | the second-order flux-wave action (Δ_t J)², standing, flux-sector-scoped per FC-2's arrow half; consumed by U-9/U-12 | [AXIOM — flux-wave-sector-scoped] |
| CLK-1 | the coordinate-level clock hypothesis (Lagrangian §4.3); FTD-0208 v3 CLOSED NEGATIVE: **not substrate-derivable** | [AXIOM — coordinate-level, independent] |
| CLK-2 | the rest-mass Klein–Gordon clock of FTD-0271 (A0: native flux is massless; default-OFF toggle) | [IMPOSED/SELECTION], not [FORCED] |
| MDL-1 | the Langevin noise model under which PL-1 is a theorem *of the model* | [IMPOSED — model] |

**Booked gaps (in-line, per the honesty rule):** (i) CLK-1/CLK-2/MDL-1/ACT-1 carry **no import-ledger line** — pricing awaits the D5 currency ruling (an import-ledger v1.2 mint); (ii) the engine-level [IMPOSED] dynamics inputs (latency field L(x), dual-substrate L/R, threshold kinetics K_GENESIS = N_c·K_MANIFEST — constitution §1.4/§3.2) carry no register line yet, same trigger; (iii) the FC-3 falsifier gap above.

**Non-members:** DEC-1 (M) and DEC-2 (reversibility) — declined falsifiable bets; and the five **chartered candidates** P6C-F/G/C/M/U of §3, **not adopted**, absent from every layer above. This list covers only the §5.5 Stage-U2 five; **P6C-R** (kinematic frame-invariance) remains chartered at charter §2/AM-5 as the minimal candidate on the DEC-2 bet.

## §2 · The Unified Conditional Statement

*Given the register, the following is content — each row at its tag of record. The `given` column names the register entries the row consumes; a `given` may also cite a priced import-ledger line that is not an axiom (the empirical bridges IMP-E1/E2/E3) — those clauses are the match surface, priced matches rather than register-licensed content.*

| # | Content | Given | Tag of record |
|---|---|---|---|
| U-1 | the algebraic spine (G\* identity, master quadratic, CM arithmetic uniqueness, Watson, geometric Coulomb, harmonic tower, π-freeness) | L0 + FC-0 (+ IMP-C1 for the transcendence clauses) | [THEOREM] — 7 of 9 per spine §0; Theorem 4's forcing [SELECTION — declared, PERMANENT]; Theorem 7 conditional; d=−4 dual-match privilege [NUMERICAL FACT — not a proof] |
| U-2 | N_c = 3 (four independent topological routes) | L0 + FC-0 | [THEOREM] |
| U-3 | the Moore-layer decomposition (octahedron + cuboctahedron + stella octangula), read physically as gauge-group slots U(1)×SU(2)×SU(3), 3×4 fermions, 17 dark states | L0 | combinatorial decomposition [THEOREM]; the physical reading **[SELECTION]** (LEDGER FTD-0028: "theorem-shaped argument; physical reading is selection") |
| U-4 | x₊ = 1/α at 1.26 ppm | L0 + FC-0 + FC-W + IMP-C1 + IMP-E1 | constitution §3.5: the mathematical consequence **[CONDITIONAL THEOREM given W]**; the physical identification is the priced bridge IMP-E1 and stays **[SMC]** (FTD-0013) — both stand; continuum-limit Theorem V.1 [Conditional THEOREM] on C1–C5, and even granting all five, FTD-0013 stays [SMC] (L→∞ ill-posed) |
| U-5 | the singlet, and Bell/Tsirelson S = 2√2 riding on it | IMP-S2 + IMP-E3 | singlet [THEOREM, given Lemma 5]; Lemma 5 [SELECTION]; bare substrate S ≤ 2 [THEOREM]; deriving Lemma 5 [CLOSED DECLINED] under FC-1 |
| U-6 | detection statistics: Rice, not Born; the PL-1 deviation ladder + five-observable kill switch | MDL-1 | Born scaling [CLOSED NEGATIVE in the 6-neighbour substrate; canonical 26-neighbour status OPEN] (FTD-0200); Rice [NUMERICAL FACT — 6-neighbour construction; 26-neighbour generalization [CONJECTURE]] (R² = 0.9923 vs 0.7137; FTD-0258 PL-1); deviation law [THEOREM — of the imposed Langevin model] + [CONJECTURE — manifestation↔detection] (FTD-0359) |
| U-7 | pilot-wave structure: de Broglie matter wave + Schrödinger-*form* envelope scaling (s ≈ 2); guidance measured ABSENT | CLK-2 | [CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT] (FTD-0271); never headlined as "derives Schrödinger"; the unconditional result is the boundary FTD-0270 [MEASURED — BOUNDARY] (wrong dispersion) |
| U-8 | Newton leading order + Schwarzschild **g₀₀ only** to leading order; α_G(e,e) = (m_e/m_P)² → 1.745×10⁻⁴⁵ vs 1.752×10⁻⁴⁵ (0.38%) | L0 + FC-0 + L3 + CLK-1 | [DERIVED] modulo the clock hypothesis; floor [SMC] (FTD-0015); **g_rr proof RETRACTED** (FTD-0361), full Schwarzschild imported (pending P6C-G); G_N = 1/100 FALSIFIED (FTD-0131) |
| U-9 | time dilation: the residual law + blind L=257 extension (7/9, PREDICTION_CONFIRMED) | L0 + ACT-1 | [MEASURED — blind extension] (FTD-0268); FTD-0252 [OBSERVATION]; consumes the posited second-order flux action; the metric *reading* is FC-2, not a derivation |
| U-10 | the Planck naming theorem: c·t_P = ℓ_P exactly, and uniquely among the √3-ladder namings, in the edge gauge (at the emergent wave speed); naming doubly selected | IMP-K1/K2 | [SYNTHESIS] + [DERIVED — formalization] (FTD-0385) |
| U-11 | the empirical bridge catalog (~131 [PARAMETRIC] + ~50 external) | IMP-E2/E3 | per-row tags in CATALOG_PARAMETRIC_INSERTIONS.md; exactly ONE identification scan-rigid (U-4), where "scan-rigid" = a tolerance-conditioned [NUMERICAL FACT] (FTD-0319), not a structural/Bayes result |
| U-12 | confinement signatures: Wilson-loop area law at the **inserted** coupling β = x₋ (σ = 0.209 of record) | L0 + ACT-1 + IMP-E3 | [MEASURED at an inserted coupling [SELECTION]] (constitution §5.1 row 9); LEDGER FTD-0025: [THEOREM-within-compact-U(1)-LGT framework, PARAMETRIC at substrate level] — gauge framework imported, β inserted not derived; derivation not held (pending P6C-C); the FTD-0217 *retraction* (FTD-0384) is the named prior art for the Front-D P5 priced no-go |

## §3 · The pending map (chartered candidates awaiting owner adopt-or-decline rulings; Stage-U0 pricing gates unmet)

Every candidate is gated by U0 (D5/D6/D7) + the FC-W pipeline (narrowing theorem → independence proof → owner ruling); **DECLINED / CLOSED NEGATIVE are first-class terminals.**

| Target | Candidate | Status today |
|---|---|---|
| GR entire (full nonlinear EFE) | P6C-G | Deser bootstrap *completes a posited* spin-2 (FTD-0189 step-0 correction); substrate spin-2 mode [OPEN]; g₀₀ only today (U-8), g_rr RETRACTED (FTD-0361) |
| QCD dynamics / confinement derivation | P6C-C | inserted-coupling engine signatures only (U-12) |
| Born sharpness / QM beyond Rice | P6C-M | Rice of record (U-6); wholesale M barred as a Front-B move (AM-5) |
| matter as axiom-grade content | P6C-F | Branch-B import via IMP-S4 at g² = 1/x₊; native fermion emergence [CLOSED NEGATIVE **at every protocol tested**] (FTD-0379/0380; M1 v2 named follow-up) |
| cosmological observables | P6C-U | none derived |

## §4 · Verification and maintenance

`python scripts/proofs/proof_unified_register.py` recomputes: layer counts; every `ledger_ref` against `import_ledger.json` (the L2/L3/L4 layers must equal that file's selected-type/calibration/named-result lines *exactly*); pending candidates absent from all layers; every §2 `given` reference resolving to a register entry **or a priced import-ledger line** (rows may consume priced non-axiom lines, e.g. the empirical bridges); and the no-bare-[THEOREM] guard on beyond-core rows. **Maintenance triggers:** a constitution or import-ledger change; a LEDGER-booked adoption (a Stage-U2 adoption enters as **an adoption**, moving its P6C line from §3 into the register with a new LEDGER row, never by editing a tag); and the D5 currency ruling, which prices the L5 posits and the §1 booked gaps (import-ledger v1.2).

*Standing invariants: x₊ = 1/α stays [SMC]; MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION]; FC-W stays [AXIOM]; D=3 stays [SELECTION — declared]; the clock hypothesis stays [AXIOM]; no tag moves.*
