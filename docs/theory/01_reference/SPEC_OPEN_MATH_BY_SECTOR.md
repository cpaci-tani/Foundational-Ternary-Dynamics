# SPEC · Open Math by Physics Sector

**Tag:** [REFERENCE] / canonical research-questions queue (replaces tier-aligned CHECKLIST)
**Replaces:** `CHECKLIST_MATH_COMPLETE.md` (archived; tier-aligned organisation preserved there for provenance)
**LEDGER:** FTD-0146 [SYNTHESIS] — sector-organised consolidation; introduces no new theorems
**Version:** v1.1 (2026-07-12 sector-status reconciliation — rows aligned to LEDGER closures: δ_c/FTD-0224, MC-T1.1-ext/FTD-0350, Higgs-manif/FTD-0268, BH/TRACKER §2.1, MC-T3.4/FTD-0095, MC-T4.1)
**Companion docs:**
- [`SPEC_DOCTRINE_LEDGER.md`](SPEC_DOCTRINE_LEDGER.md) — single-page status map (closed + open)
- [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) — canonical algebraic-spine reference (nine numbered results: seven theorem-grade + two honestly-tiered — Theorem 3 at its arithmetic core only; see §0 count convention)
- [`SPEC_FQCR.md`](SPEC_FQCR.md) — FQCR Models I–V
- [`SPEC_MATH_FIRST_ONTOLOGY.md`](SPEC_MATH_FIRST_ONTOLOGY.md) — primitives -> invariants -> readouts -> physics ordering principle
- [`SPEC_ALPHA_READOUT_CONTRACT.md`](SPEC_ALPHA_READOUT_CONTRACT.md) — MC-T4.3 closure contract / "earn the map" criteria
- [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) — atomic per-claim provenance
- [`../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md`](../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md) — T1–T5 tiers
- [`../07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md`](../07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md) — atomic file-level `[OPEN]` markers (non-math TODOs)
- [`../10_eft_program/derivations/FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md`](../10_eft_program/derivations/FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md) — FTD-0244 K-BIND closure
- [`../../../evaluation/AUDIT_WEAKNESSES_MASTER.md`](../../../evaluation/AUDIT_WEAKNESSES_MASTER.md) — historical W-CRIT / W-COSMO weakness audit source

---

## §0 · How to read this tracker

This tracker organises **mathematical research gaps** by physics sector (10 sectors). It replaces the tier-aligned `CHECKLIST_MATH_COMPLETE.md` while preserving the load-bearing structure (effort codes + dependency graph + foundational-obstruction framing).

### Status key (canonical LEDGER tags)

| Tag | Meaning |
|---|---|
| **[OPEN]** | Unresolved research question |
| **[OPEN — documentation]** | Editorial alignment, not new mathematics |
| **[FOUNDATIONAL OBSTRUCTION]** | Closure may require ontology extension beyond the 5 axioms |
| **[CLOSED NEGATIVE]** | Hypothesis tested and falsified; preserved to prevent re-attempt |
| **[CLOSED]** | Resolved; entry kept for sector context |
| **[BLOCKED]** | Waiting on upstream gap |
| **[PRE-REGISTRATION]** | Awaiting separate-session execution |

### Effort code (from CHECKLIST_MATH_COMPLETE)

| Code | Scale |
|---|---|
| **D** | 1–5 days session work |
| **W** | 1–3 weeks focused work |
| **M** | 1–3 months focused work |
| **RP** | Research program (3+ months, open timeline) |
| **FO** | Foundational obstruction — closure may require ontology change |

### Sector list

1. Pure mathematics / Algebraic spine
2. Electromagnetism / Fine structure (α)
3. Electroweak / Higgs
4. QCD / Strong / Color
5. Flavor / Masses
6. Gravity / GR
7. Quantum foundations / Bell / Lorentz
8. Cosmology
9. Engine  Algebra bridge
10. Cross-cutting / Foundational obstructions

Each sector has: **scope · status snapshot · open math table · closed-negative reminders (if any) · sources**. Closed items are noted briefly for sector context only; full provenance lives in `LEDGER.md`.

---

# §1 · Pure mathematics / Algebraic spine

**Scope:** Number-theoretic and algebraic content of FTD's algebraic spine (nine numbered results: seven theorem-grade + two honestly-tiered, see `SPEC_ALGEBRAIC_SPINE.md` §0); independent of physics interpretation.

**Status snapshot:** Spine: nine numbered results — seven theorem-grade + two honestly-tiered (see `SPEC_ALGEBRAIC_SPINE.md` §0). Tier I 5/5 closed. Tier II 4/4 closed; 0 structural theorems [OPEN]. Pre-registered scan queue empty (FTD-0143 executed 2026-07-12 — uniqueness rejected). δ_c closed-form gap CLOSED scan-negative (FTD-0224); MC-T1.1-ext CLOSED (FTD-0350, [THEOREM at all L ≥ 2]).

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| MC-T1.1-ext | `L ≥ 3` ultralocality proof or disproof for matched-stencil with Gauss-constraint-allowed configurations | [CLOSED per FTD-0350 — **[THEOREM at all L ≥ 2]**, matched-stencil / Gauss-realizable scope, conditional on the exact-constraint AXIOM + stencil-consistency SELECTION; spine §7 carries the verbatim scope; theorem-bucket move pending owner decision] | W (closed) | — |
| MC-T2.3-4 | Structural theorem for `d = −4` privilege beyond 63-discriminant numerical scan. **Effort D-W (W2.6 audit)**: FTD-0122 / OT-1.5 already established `Z[BCC] ⊗ Q ≅ V_triv² ⊕ V_sign² ⊕ V_complex²` with `V_complex` carrying natural `Z[i]`-module structure. Among class-number-1 imaginary-quadratic fields `{d=1,2,3,7,11,19,43,67,163}`, **only `d=4` (`Q(i)`) has `\|O^×\| = 4`** (others have order 2 or 6 (`Q(ω)`)). The closure is a one-page argument under the `\|Z[i]^×\| = 4` unit-group condition: `d = −4` is the unique CM ring whose unit group has order 4, and FTD-0122's complex structure on V_complex requires exactly this. Ancillary routes (`L(E, 1)` analytical structure, Galois-theoretic) remain optional. | [CLOSED — resolved via unit-group uniqueness theorem in SPEC_ALGEBRAIC_SPINE.md §3] | D-W (closed) | FTD-0122 (already closed) |
| δ_c | `δ_c = x_- − 3 ≈ 0.024`: closed-form for the residual between the smaller root `x_- = 16G*³α` and the integer 3. (Historical framing: `δ_c = x_- − N_c`; **superseded** by v1.4 §5 retirement of the `x_-  N_c` identification — LEDGER FTD-0014 removed in commit `ca7eb61`.) | [CLOSED — scan-negative for a simple closed form (FTD-0224 [CLOSED RESOLVED]: 100-digit PSLQ across four declared baskets found no low-height relation; the earlier post-hoc monomial fits discredited; see `EXPLR_COLOR_EXCESS_CLOSED_FORM.md` + TRACKER §1.6). Scan evidence, not a transcendence proof.] | W–M (closed) | — |

**Closed (sector context):** Theorems 1–9 (FTD-0001, OT-1.1–1.8, 2.1–2.3, 4.1, 3.4 partial), MC-T1.1 (route b), **MC-T1.1-ext (CLOSED per FTD-0350 — Theorem 7 [THEOREM at all L ≥ 2], matched-stencil/Gauss-realizable scope, conditional; theorem-bucket move pending owner decision)**, MC-T1.2 (Theorem 3 retagged `[NUMERICAL FACT, h=1 only]`), MC-T1.3 (Q(G*) verification), MC-T1.4 (per-voxel mass gap), MC-T1.5 (BCC complex-structure Roles 1+3 [DERIVED]; Roles 2+4 NO-GO), MC-T2.1 + MC-T2.2 (extended polynomial scan; the "~4×10⁵:1 Bayes" figure is retracted to [NUMERICAL FACT] — not runner-computed, ~19× scan-size), MC-T2.3 items 1–3 (63-disc Γ-product null at h ≥ 2), MC-T2.3-4 (CLOSED, unit-group uniqueness proof integrated), δ_c (CLOSED scan-negative, FTD-0224).

**Sources:** SPEC_ALGEBRAIC_SPINE.md §§1–10; TRACKER_ONTIC_TRUTH.md OT-1.x, OT-2.x; SPEC_DOCTRINE_LEDGER.md §§1–4.

---

# §2 · Electromagnetism / Fine structure (α)

**Scope:** `x_+  1/α` identification (FTD's central physics claim); QED bridge from FTD substrate.

**Status snapshot:** `x_+ = 137.0362` matches `1/α` to 1.26 ppm as **[STRONGLY MOTIVATED CONJECTURE]** (OT-5.1, FTD-0013). Coefficient 16 = `\|Aut(E)\|²` is structural identification at OT-4.1 [T4]. FTD-0244 closes K-BIND theorem-negative for the current substrate-native operator calculus; the physical α readout therefore remains MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`, with positive closure requiring a new W-class commitment or fresh ARC-D measurement. Doctrine §7 bivector/Dirac/QED bridge sector remains open per FTD-0073 mode-erasure closure.

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| **MC-T4.3** | **Operational alpha-readout mechanism (CENTRAL).** A closure proof would have to derive the physical identification `x_+ = 1/alpha` from FTD structure rather than insert it. All natural action-level/operator routes are closed negative through FTD-0244: ARC-A/B1 closed negative, ARC-B2/C1 sharpened to route-invariant/operator-calculus no-go, and K-BIND closed theorem-negative. The surviving positive exits are a new W-like framework commitment or a fresh ARC-D engine-native measurement. | **[FOUNDATIONAL OBSTRUCTION]** | **FO** | MC-T4.1, MC-T4.2 |
| **TEST4-GEN** | **Alpha arithmetic generativity test.** Pre-registers the Balmer-to-Bohr gate: the same lemniscatic CM/FQCR rigidity that produces the alpha candidate must generate one additional independent physical dimensionless observable or relation with no new tuned freedom. *(The `x_- ≈ N_c` match does not count as the prize; that identification is **RETIRED** entirely per v1.4 §5 — see `DERIV_NC_FROM_TOPOLOGY.md` for the independent `N_c = 3` sources.)* Target declaration must precede numerical comparison. | [PRE-REGISTRATION] | D-W | MC-T4.3; `PREREG_ALPHA_ARITHMETIC_GENERATIVITY_v1.md` |
| §7-bivector | Lorentzian signature from bivector duality on FTD lattice. Requires non-site-local Clifford construction compatible with FTD-0073 (site-local Clifford [CLOSED NEGATIVE] under pointwise-threshold dynamics). **Measured state 2026-07-10 (FTD-0379/0380, `../09_mathematical/algebra/ANALYSIS_VERTEX_DK_CLOSURE_v1.md`):** both dynamical-emergence branches CLOSED NEGATIVE at the protocols tested — FTD-0089's literal DK evolution fails (grades better described by KG than DK, 4/4) and su(2) closure does not recover under the FTD-0088-prescribed noise controls (FTD-0088's noise reading REFUTED; instrument-scope caveats in the analysis §1.3). Surviving substrate foothold: the *kinematic* Cl(3,0) grade skeleton (FTD-0088, subject to the effective-toggle audit) + matching-bivector signature (FTD-0086); the formal construction question stays open on that kinematic basis only | [OPEN — kinematic basis only] | RP | — |
| §7-dirac | Tree-level `g = 2` from FTD-substrate Dirac. **Branch-B route chartered 2026-07-10:** matter imported (Wilson–Dirac, `../10_eft_program/scopes_and_specs/SPEC_WILSON_DIRAC_FTD.md`), vertex coupling an [IMPOSED — calibration] = IMP-E1 ∘ IMP-E3 composed (`../10_eft_program/scopes_and_specs/SCOPE_VERTEX_PROGRAM.md` §2); execution = charter stage V1 (free-sector bring-up + tree g=2 behind the FTD-0126 Wilson-r-artifact gate); no longer gated on native §7-bivector closure | [OPEN — IMPORTED scaffold; Branch-B chartered] | M | SCOPE_VERTEX_PROGRAM V1 |
| §7-loop | One-loop `a^{(1)} = α_FQCR/(2π)`. Requires `α_FQCR  α` physical identification (= FTD-0013 [SMC]) | [OPEN] | RP | MC-T4.3 |
| §7-qed | Full QED `g − 2` precision (= MC-T4.4 in old checklist) | [OPEN] | W–M | §7-dirac |
| α-from-CM | `CONJ_ALPHA_FROM_CM.md` Step 3 (Z₄ symmetry selects this CM curve) and Step 8 (larger root = 1/α specifically) remain [STRONGLY MOTIVATED CONJECTURE], not [THEOREM] | [CLOSED RESOLVED / RECLASSIFIED] | RP | — |
| Watson-G* | `DERIV_WATSON_GSTAR_IDENTITY.md` epilogue carries 1 [OPEN] | [CLOSED RESOLVED / RECLASSIFIED] | W | — |
| α-lattice | `DERIV_ALPHA_LATTICE_MECHANISM.md` Steps 3 + 8 selection issue (same as α-from-CM) | [CLOSED RESOLVED / RECLASSIFIED] | M | α-from-CM |

**Closed-negative — do not re-attempt:**
- R1 transverse stiffness; R2 source-current normalization; R3 two-sector response eigenvalue; R4 projected Dirac matter (all in `archive/closed_negative/`).
- Z-factor reading (FTD-0116, Q4a); RG-running; algebraic combinations; 1/√d; Langevin-equipart; monomial scans (FTD-0097 look-elsewhere).
- BZ²/9.6-ppb/two-loop α numerical closure (superseded by native-electrodynamics pivot; `DERIV_LATTICE_QED_COMPLETE.md` fully closed).

**Sources:** SPEC_DOCTRINE_LEDGER.md §5, §7; LEDGER FTD-0013, FTD-0073, FTD-0116, FTD-0097; TRACKER_ONTIC_TRUTH.md OT-5.1, OT-4.1; TRACKER_OPEN_ITEMS.md §§4.2, 4.4, 6.5, 7.7.

---

# §3 · Electroweak / Higgs

**Scope:** SU(2) × U(1) sector; electroweak symmetry breaking; Higgs mechanism; weak-sector masses.

**Status snapshot:** Doctrine §8 establishes GUT-lock `sin²θ_W = 3/8` (standard SU(5) trace-normalisation; **3:5 ratio is [IMPORTED] per FTD-0149** — no FTD substrate ingredient enters; the 3/8 value is [THEOREM once 3:5 imported] but the IMPORT is doing the work, not FTD content). Canonical IR fit `sin²θ_W ≈ 3/13` [PARAMETRIC, FTD-0018] (3.5% off CODATA 0.22290(30); M_Z scale annotated per FTD-0150). RG running between scales [OPEN/HARDENING]. Doctrine §9 `v = √2 m_t` [BORROWED EMPIRICAL] (textbook `y_t ≈ 1`, not novel). Higgs `χ_H = 2 − 3 Ξ_t + Ξ_bos` is scaffold; computation [OPEN].

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| **MC-T3.6** | Substrate-derive β-coefficients `b_Y = 41/6`, `b_2 = −19/6` from finite spectra (RG running of `sin²θ_W^lock` from GUT to M_Z; the IR `sin²θ_W` should fall out from running the GUT-lock 3/8 — currently the IR fit FTD-0018 [PARAMETRIC] 3/13 is independent). Resolves the W2.5 MC-T3.5 collision (old MC-T3.5 = §9 FTD-0110 multi-scale boundary correction) | [OPEN/HARDENING] | M–RP | — |
| §8-running (alias of MC-T3.6) | Same as MC-T3.6 above; this row preserved as the doctrine §14 priority-2 cross-link target | [OPEN/HARDENING] | M–RP | MC-T3.6 |
| §9-chiH | `χ_H` derivation from FTD substrate. No canonical anchor | [OPEN] | M | — |
| Higgs-manif | `DERIV_HIGGS_FROM_MANIFESTATION.md` — the 3 in-doc [OPEN] items (mass discrepancy, EW transition order, BI/pair-creation link) | [CLOSED 2026-06-11 — see TRACKER §2.6; honest status per the FTD-0268 digest: the (1−α) loop factor is applied-not-derived, chain [SELECTION]+[PARAMETRIC], +0.27σ vs canonical PDG 2024 (tree +4.44σ); doc reconciled 2026-07-12] | M (closed) | — |
| SU(2)-weak | `DERIV_LATTICE_SU2_WEAK.md` carries 5 [OPEN] (chiral structure, left-handed doublets, weak mixing via ungerade sector) | [OPEN] | RP | — |

**Closed (sector context):** SM gauge group `G_SM = (SU(3) × SU(2) × U(1))/Z_6` adopted as [IMPORTED structural match]; `Z_6` center closure [THEOREM within scaffold]; `Q = T_3 + Y` [IMPORTED]; neutral Higgs lock preserves `U(1)_EM` [THEOREM within scaffold].

**Sources:** SPEC_DOCTRINE_LEDGER.md §§8–9; LEDGER FTD-0017, FTD-0018; TRACKER_OPEN_ITEMS.md §§2.5, 2.6.

---

# §4 · QCD / Strong / Color

**Scope:** SU(3)_c sector; confinement; color charge; strong CP.

**Status snapshot:** `N_c = 3` is independently sourced — see `DERIV_NC_FROM_TOPOLOGY.md` (four routes) and the Moore Layer Theorem. *(The identification `x_-  N_c` (0.80%, OT-5.2, FTD-0014) is **RETIRED** per v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`.)* `b_3 = (11 N_c − 2 n_f)/3 = 7` [IMPORTED COEFFICIENT, THEOREM once formula imported]. `α_s = 7/59` [PARAMETRIC, FTD-0020]. Confinement substrate-derivation has a **recognised structural obstruction**: no Phase-G analog for area-law behavior because confinement is intrinsically non-classical (lives in `Z = ∫dU exp(−S)`) and FTD substrate is deterministic.

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| §11-confine | Substrate-derive QCD trace-gap confinement. Compact-U(1) link-variable formulation imported wholesale from textbook lattice gauge theory; `β = x_-` insertion is selection. Closing path: substrate-derive an effective compact-U(1) sector with inverse coupling flowing to `x_-` in the IR (parallel to Phase G's Gauss→Poisson chain) | [OPEN, structural obstruction] | RP–FO | — |
| §11-thetaQCD | Strong CP `θ_QCD = 0` by finite discrete orientation closure | [CONJECTURE / NEEDS THEOREM PACKAGING] | M | — |
| SU(3)-gauge | `DERIV_LATTICE_SU3_GAUGE.md` 5 [OPEN] (theoretical counterpart to engine §1.3) | [OPEN] | RP | — |
| eng-SU3 | Engine `phase_forces()` three-regime piecewise color force still imposed; replace with dynamical SU(3) gauge field whose Wilson-loop expectation produces linear confinement without hand-inserted regime switches | [OPEN] | M–RP | §11-confine |
| chiral-anom | `DERIV_LATTICE_CHIRAL_ANOMALY.md` 3 [OPEN] | [OPEN] | M | — |
| δ_c-color | `δ_c = x_- − 3 ≈ 0.024` closed form (cross-listed from §1; the historical `x_- − N_c` framing is **retired** v1.4 §5) | [CLOSED — scan-negative for a simple closed form, FTD-0224; see §1 row] | W–M (closed) | — |

**Closed-negative — do not re-attempt:**
- All three first-principles routes for `g_c` (Mechanisms A, B, C; FTD-0031, FTD-0093). `g_c` remains [PARAMETRIC].
- Three substrate-derivation routes for confinement: (1) BCC eigenvalue triple-cosine product at `x_-`; (2) discriminant trichotomy phase argument; (3) Phase J ultralocality as confinement signature — all CLOSED NEGATIVE.

**Posture (map-and-consolidate):** `§11-confine` is an **accepted structural obstruction — declined-and-mapped, not actively chased.** Area-law confinement is a partition-function (non-classical-integral) object with no deterministic-substrate analog: no Phase-G-type chain exists (the three classical routes closed-negative above), and the full Yang–Mills derivation is RETRACTED (FTD-0042; `DERIV_YANG_MILLS_CONFINEMENT.md` reconciled to `[MEASURED at an inserted coupling [SELECTION]]` per FTD-0303) — only the per-voxel mass gap (FTD-0044) survives as the load-bearing residual `[THEOREM]`. This sits in the same accepted-boundary class as the clock-hypothesis (FTD-0208, gravity) and α (MC-T4.3, [`SPEC_ALPHA_DYNAMICAL_BOUNDARY.md`](SPEC_ALPHA_DYNAMICAL_BOUNDARY.md)): a recognized limit of what the discrete substrate determines, reopenable only by a new effective-sector construction (compact-U(1) flowing to `x_-`), not by the current program.

**Sources:** SPEC_DOCTRINE_LEDGER.md §11; `DERIV_NC_FROM_TOPOLOGY.md` (independent `N_c = 3` routes); LEDGER FTD-0020, FTD-0025, FTD-0029, FTD-0031, FTD-0093; (FTD-0014 retired per v1.4 §5, row removed in commit `ca7eb61`); TRACKER_OPEN_ITEMS.md §§1.3, 2.4, 2.8.

---

# §5 · Flavor / Masses

**Scope:** Charged-lepton + quark mass hierarchy; CKM matrix; flavor structure.

**Status snapshot:** `m_e/m_P = √(2π)·(16/3)·α¹¹` to 0.19% [STRONGLY MOTIVATED CONJECTURE, FTD-0015]. **Exponent `n = 11` [DERIVED]** (MC-T3.2 closure, given multiset theorem FTD-0084 + 2 SM-hierarchy SELECTIONs). `m_p/m_e` to 174 ppm [STRONGLY MOTIVATED CONJECTURE, FTD-0016]. Mass ratios `m_μ/m_e`, `m_τ/m_e` to ~5%. CKM order-of-magnitude only. Doctrine §10 depth matrices [PARAMETRIC candidate scaffold].

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| §10-depths | Explicit transfer matrices forcing `N_E = diag(9,3,0)`, `N_U = diag(12,5,0)`, `N_D = diag(7,4,0)`. Currently no canonical derivation — reverse-engineered from mass-ratio fits | [OPEN] | M | — |
| §10-depths-method | **Methodological honesty audit.** The §10 depth matrices have **18 free integer slots** (9 in N_E∪N_U∪N_D + 9 per-fermion projection corrections C_F also tagged [PARAMETRIC]) — sufficient to fit any 3×3 hierarchy with 3 OOM spread. The "0" in third position is forced by `q*^0=1`, not by structure. The §10-depths target above is mis-framed as "find the matrices"; the actual methodological gap is **show this scaffold has predictive content beyond fit count**. Until a genuinely-predictive constraint (e.g., a transfer-matrix derivation that fixes ≥10 of 18 slots from substrate) is in hand, the [PARAMETRIC candidate scaffold] tag risks overstating the degree to which depth matrices are a substrate object vs a curve fit | [OPEN — methodological] | W (audit) + M (predictive constraint) | — |
| me-prefactor | Substrate justification of FTD-0015 prefactor `√(2π)·(16/3)`. Promoting `α_G(e,e) ≈ 1.745 × 10⁻⁴⁵` from [DERIVED, postulate-conditional] to [DERIVED, axiom-conditional] requires this | [OPEN] | M–RP | — |
| quark-mass | `FOUND_DISCRETE_NATIVE_MASS_GENERATION.md` (retracted continuous QFT fits; replaced by native discrete mass paradigm — Class A voxel cardinality) | [OPEN] | RP | §10-depths |
| quark-bridge | `archive_proof_quark_masses_lattice.py` (archived post-hoc quark mass verification script) | [ARCHIVED] | — | — |

**Closed (sector context):** FTD-0015 `n = 11` exponent [DERIVED]; FTD-0016 `m_p/m_e` formula [STRONGLY MOTIVATED CONJECTURE]; FTD-0084 multiset theorem [DERIVED].

**Sources:** SPEC_DOCTRINE_LEDGER.md §10; LEDGER FTD-0015, FTD-0016, FTD-0084; TRACKER_OPEN_ITEMS.md §§4.1, 8.2.

---

# §6 · Gravity / GR

**Scope:** Newtonian limit, Schwarzschild, full GR, lattice black holes.

**Status snapshot:** **Partial closure** (FTD-0131; reconciled per `../07_assessment/audits/AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`): `α_G(e,e) = (m_e/m_P)² ≈ 1.745 × 10⁻⁴⁵` matches measured to 0.38% as **[STRONGLY MOTIVATED CONJECTURE]** for the prediction (epistemic floor inherited from FTD-0015 [SMC] via `α_G = (m_e/m_P)²` tautology — the 0.38% precision is squared FTD-0015 precision, mechanical not new evidence) plus **[DERIVED]** for the chain steps that recover Schwarzschild leading-order from substrate, conditional on the clock-hypothesis **[AXIOM]** used in SPEC_FTD_LAGRANGIAN.md §4.3. Arc B P2 closed **[CLOSED NEGATIVE, AXIOM-LEVEL]** (v3, after v1 UNDERDETERMINED and v2 INVALIDATED): the quadratic L² budget is structurally incompatible with Scale-0 primitives and must be posited at coordinate level. **Arc C2 spin-2 boundary theorem free-theory derivation** (`../10_eft_program/derivations/DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md` + `../10_eft_program/derivations/DERIV_J_BILINEAR_NO_SPIN2_POLE.md`); Arc C2 P3 pre-reg (`preregister-spin2-boundary-theorem-v1`, FTD-0209) hash-locked. Framework-integer `G_N = 1/(b_3 + N_c)² = 1/100` reading [CLOSED NEGATIVE per FTD-0131] — off by `~10²⁰` to `~10⁴³` under any natural calibration.

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| §12-clock-hypothesis | Substrate-derive the **single** flagged interpretive step of the FTD-0131 reconciliation: the clock hypothesis used in SPEC_FTD_LAGRANGIAN.md §4.3. Closed by v3: the Scale-0 substrate supports an L¹ linear ceiling, not the required L²/Pythagorean budget; the clock hypothesis is therefore an independent coordinate-level **[AXIOM]**. | [CLOSED NEGATIVE, AXIOM-LEVEL] | closed | — |
| §12-beyond-N | Beyond-leading-order GR: Mercury perihelion, light bending, gravitational waves (full nonlinear Einstein equations beyond Deser bootstrap [SELECTION, FTD-0026]) | [OPEN] | RP | clock-hypothesis AXIOM |
| §12-EP | Equivalence-principle analogue from substrate. No canonical anchor | [OPEN] | RP | — |
| §12-mgcurv | Mass-gap to curvature source. No canonical anchor | [OPEN] | RP | — |
| BH | `DERIV_LATTICE_BLACK_HOLES.md` — the historical 11-[OPEN] cluster (horizon thermodynamics, Hawking radiation, information paradox, Kerr-Newman) | [CLOSED/reclassified 2026-06-10 per TRACKER §2.1 — zero live in-doc [OPEN]; FTD-0184 guardrail stands: substrate-side strong-field GR (Schwarzschild/Kerr currently imported from GR) is the only sanctioned future route; the information-paradox gap remains a named wall (round-table residue §7.8)] | RP (closed as doc cluster) | — |
| MC-T4.4 | General-motion lattice Liénard-Wiechert: closed-form for general accelerating motion. Closed at uniform velocity [DERIVED]; sinusoidal Larmor case has Bessel infinite-series form (FTD-0120 Q5); general motion only formal Q5★ frequency-domain expression | [OPEN] | W–M | — |

**Closed (sector context):** FTD-0004 Phase G geometric Coulomb [THEOREM]; FTD-0110 cluster mass [DERIVED at linear level]; FTD-0131 leading-order Newton ([SMC] prediction floor inherited from FTD-0015; [DERIVED] chain steps conditional on clock-hypothesis AXIOM); FTD-0208 clock-hypothesis v3 [CLOSED NEGATIVE, AXIOM-LEVEL]; FTD-0113 retarded Green identity [DERIVED]; FTD-0115 lattice Cherenkov closed at uniform velocity.

**Closed-negative — do not re-attempt:** FTD-0035 Mechanism γ gravitational `a_phys` derivation (closed; calibration `a_phys ≡ ℓ_P` recommended); "1/100" framework-integer reading (FTD-0131); substrate derivation of the clock-hypothesis L² budget from Scale-0 primitives (FTD-0208 v3).

**Sources:** SPEC_DOCTRINE_LEDGER.md §12; `../03_derivations/gravity_and_cosmology/DERIV_NEWTON_FROM_SUBSTRATE.md`; `../03_derivations/archive/AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md`; LEDGER FTD-0004, FTD-0026, FTD-0110, FTD-0113, FTD-0115, FTD-0131, FTD-0208; TRACKER_OPEN_ITEMS.md §§2.1, 3.2.

---

# §7 · Quantum foundations / Bell / Lorentz

**Scope:** Emergence of QM from FTD substrate; Bell violation; Lorentz invariance recovery; observer mechanisms.

**Status snapshot:** Bell `S = 2√2` [CLOSED DECLINED] — the continuous Hilbert space and Bell violation recovery targets are formally declined under FC-1. Moore-Laplacian isotropy verified at O(h²) and O(h⁴) (rotationally invariant correction `(h²/12)·(∇²)²f`); empirical 11–20% pairwise diff at L=48–64 (high-k dispersion artifact present in every cubic-lattice FD scheme). Continuum-limit Lorentz recovery [OPEN].

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| W-CRIT-3 | Lorentz invariance recovery: relational reinterpretation step from "isotropic Laplacian + emergent SR-like dispersion" to "Lorentz invariance" | [OPEN] | M–RP | — |
| W-CRIT-4 | Bell violation rigorous: explicit construction of an sLoop process producing `S > 2` from the 5 axioms | [CLOSED DECLINED] | — | — |
| QM-lattice | `DERIV_QM_FROM_LATTICE.md` (Hilbert-space / QM recovery declined under FC-1) | [CLOSED DECLINED] | — | — |
| Wigner | `FOUND_WIGNERS_FRIEND_RESOLUTION.md` (Wigner's friend resolution declined under FC-1) | [CLOSED DECLINED] | — | — |
| vN-chain | `FOUND_VON_NEUMANN_CHAIN.md` (Von Neumann chain / continuous measurement mapping declined under FC-1) | [CLOSED DECLINED] | — | — |
| Bell-mech | `DERIV_OBSERVER_BELL_MECHANISM.md` (Bell mechanism declined under FC-1) | [CLOSED DECLINED] | — | — |
| Born | `FOUND_BORN_RULE_NULL_CONE.md` (Born rule null-cone geometry declined under FC-1) | [CLOSED DECLINED] | — | — |
| Existence | `FOUND_THE_EXISTENCE_FILTER.md` (Existence filter open items declined under FC-1) | [CLOSED DECLINED] | — | — |

**Sources:** SPEC_DOCTRINE_LEDGER.md §1, §7; LEDGER FTD-0023; `../../../evaluation/AUDIT_WEAKNESSES_MASTER.md` W-CRIT-3, W-CRIT-4; TRACKER_OPEN_ITEMS.md §§2.10, 5.1–5.6.

---

# §8 · Cosmology

**Scope:** ΛCDM-relevant predictions; inflation; dark matter; cosmic structure.

**Status snapshot:** Most cosmology predictions sit at [SELECTION] or [PARAMETRIC]. Per the 18-evaluation review (`../../../evaluation/AUDIT_WEAKNESSES_MASTER.md` W-COSMO 1–7): inflaton ad hoc; dark matter mechanism inconsistent; first-order EW transition assumed; `Λ = α^57` numerology without mechanism; no power spectrum/BAO predictions; NFW halo not derived.

> **Imported-content audit note (W2.7).** §8 is the **most W-CRIT-1-vulnerable sector in the framework**: every cosmology entry is standard ΛCDM apparatus filled with FTD numerology, with the least substrate-derivation backing of any sector. Specifically: `Λ = α^57` is paradigm circularity (an FTD constant raised to a power chosen to match observation, with no substrate constraint on the exponent); inflaton-as-mean-flux is identification without dynamics; dark matter "mechanism inconsistent" indicates the W-COSMO-2 finding has not been resolved; NFW halo not derived (NFW is a phenomenological fit, and FTD currently neither derives nor contests it); power spectrum + BAO predictions are absent. Compared to §1 (algebraic-spine; theorems-grade), §2 (EM/α; conjecture with structural-uniqueness backing), §6 (gravity; partial closure FTD-0131), §8 stands out as **the sector where the most external structure has been imported without substrate justification** — and per the §13 doctrine non-circularity audit, this concentrates W-CRIT-1 risk. Closing W-COSMO-1 through W-COSMO-6 would require a substantive cosmological substrate-derivation program of multi-month-RP scale; in the meantime, manuscript chapters citing cosmological predictions should explicitly distinguish "imported ΛCDM apparatus + FTD numerology" from "substrate-derived". Cross-link: `../../../evaluation/AUDIT_WEAKNESSES_MASTER.md` W-COSMO-1 through W-COSMO-7.

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| W-COSMO-1 | Inflaton identification with mean flux is ad hoc | [OPEN] | M–RP | — |
| W-COSMO-2 | Dark matter mechanism: internally inconsistent (dynamics [OPEN]/mapped-negative per **FTD-0300** — lossless self-field box-fills the periodic lattice, r_eff≈L/2; the −0.69 halo exponent is FALSIFIED → −1.25 at L≥128; SPARC not founded; verdict INDETERMINATE) | [OPEN] | M | FTD-0300 |
| W-COSMO-3 | First-order electroweak transition: assumed not derived | [OPEN] | M–RP | §3 §9 |
| W-COSMO-4 | `Λ = α^57`: numerology — **FC-1 dissolves the *old* catastrophe (`Λ=0`)**; FC-3 + holographic fix the *form* + *ceiling* (`Λ≲(ℓ_P/L_H)²`); **nonzero *source* `[OPEN]`** (FTD predicts `Λ=0`; condensate leaks `L⁻⁵`); value `[BOUNDARY]` (FTD-0059) | [PARTIAL] (dissolution `[DERIVED]`; source `[OPEN]`; value `[BOUNDARY]`) | RP | FTD-0331; `DERIV_LAMBDA_SCALE_COVARIANT.md` |
| W-COSMO-5 | Power spectrum + BAO predictions: missing | [OPEN] | RP | — |
| W-COSMO-6 | NFW halo profile: not derived | [OPEN] | RP | — |
| stellar | `DERIV_STELLAR_LIFECYCLE_LATTICE.md` 3 [OPEN] | [OPEN] | M–RP | §6 |

**Sources:** `../../../evaluation/AUDIT_WEAKNESSES_MASTER.md` W-COSMO; TRACKER_OPEN_ITEMS.md §2.7.

---

# §9 · Engine  Algebra bridge

**Scope:** Connecting algebraic spine theorems to engine empirical observations. The bridge that makes engine-as-instrument scientifically forceful rather than confirmation-bias-prone.

**Status snapshot:** Bridge exists at linear level (FTD-0110 `k = 1/N_base = 1/4` [DERIVED via O_h A_{1g} multiplicity]). Current-stack campaigns resolved L128-G2 and closed the exact `1/4` nonlinear scaling question negative; the live bridge gap is the FTD-0269 boundary: the engine reproduces the N(A) law's shape, but its calibration depends on non-framework engine constants (0.5 kinetic drain + Langevin friction γ).

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| **MC-T3.1** | FTD-0110 nonlinear bridge / N(A) law. FTD-0269 boundary map: framework-derived dynamics reproduce the law's shape (broken power, knee, Gauss boost), but calibration is set by non-framework engine constants; closure would require deriving the 0.5 kinetic drain + Langevin γ from the action, or showing they are convention. | [OPEN — boundary mapped] | M | — |
| **MC-T3.4** | Bridge Functional arithmetic-mean rule (FTD-0095): which of the four candidate mean functionals on the master-quadratic spectrum is forced | [CLOSED — LEDGER FTD-0095 upgraded [THEOREM] 2026-05-29 ('t Hooft beable equiprobability forces the arithmetic mean; `proof_bridge_functional_arithmetic_mean.py`). This row aligns prose to the existing LEDGER tag — no promotion by this sweep.] | M (closed) | — |
| MC-T3.5 | FTD-0110 multi-scale boundary-correction closure. Historical exact-`1/4` nonlinear scaling target is superseded by the FTD-0261/0263/0269 current-stack law; exact current-stack `k = 0.25` is closed negative. | [CLOSED NEGATIVE / SUPERSEDED] | closed | MC-T3.1 |
| L128-G2 | L=128 G2 follow-up to FTD-0107 — engine-side L-invariance test (32, 64, 128). GPU-native campaign confirmed Outcome B and locked L-invariance. | [RESOLVED] | closed | — |
| FTD-0110-NL | Current-stack collective-coordinate proof: derive the engine-emergent N(A) calibration, including the 0.5 kinetic drain and Langevin γ, or prove these are conventions rather than physics. FTD-0277 v1 closes the simple slosh-pass/static-gating counting route negative (pure A², 20–40× high, wrong geometry); successor attempts must be freshly pre-registered. | [OPEN — boundary mapped; v1 counting CLOSED NEGATIVE] | W–M | MC-T3.1 |
| FTD-0298-SOUND | Condensate compression (acoustic-like) mode: FTD has light but no acoustic Goldstone (the lattice *is* space — no spontaneously broken translation symmetry; FTD-0298 §5). The only candidate sound-analog is a propagating compression mode of the FTD-0272 manifested-condensate phase. First-order genesis argues against a gapless mode, but the bulk condensate is a real medium — instrument the engine for a propagating density/compression wave in the manifested phase | [CLOSED — BOUNDARY (FTD-0299: probe = NULL, no acoustic branch)] | W–M | FTD-0272 |

**Closed (sector context):** MC-T3.2 m_e exponent `n = 11` [DERIVED]; MC-T3.3 (SC+FCC)/2  BCC bridge — **closed-negative for identity** (no Watson-integral identity), **closed-positive for symmetry** (shared O_h symmetry forces leading-order agreement); 25-voxel cluster size at canonical amplitude A=10 [DERIVED at linear level] (FTD-0110 closure); L128-G2 resolved; exact nonlinear `1/4` current-stack scaling closed negative.

**Sources:** SPEC_DOCTRINE_LEDGER.md §13.5; `../03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`; `../03_derivations/foundational_mechanics/ANALYSIS_NA_LAW_CURRENT_STACK_v1.md`; `../03_derivations/foundational_mechanics/ANALYSIS_FTD0110_NA_LAW.md`; LEDGER FTD-0107, FTD-0110, FTD-0095; TRACKER_OPEN_ITEMS.md §7.7.

---

# §10 · Cross-cutting / Foundational obstructions

**Scope:** Load-bearing methodological challenges that affect multiple sectors. The single most consequential gap (MC-T4.3) sits here.

**Status snapshot:** MC-T4.3 is the **central foundational obstruction**. All natural action-level α-injection routes [CLOSED NEGATIVE]. Convergent diagnostic across 4 independent engine tests confirms the master quadratic value `α = 1/x_+` does not flow into engine matter-sector dynamical observables under any classical-gauge protocol tested. Lead-physicist diagnosis: structural decoupling via Phase J ultralocality. **Closure may require ontology extension beyond the 5 axioms.**

**FTD-0224:** the four ARC mechanism classes of `SPEC_ALPHA_READOUT_CONTRACT.md` have each been attacked. **ARC-A** (boundary-condition) and **ARC-B1** (observable-selection, catalog items 4/6/7) closed `[CLOSED NEGATIVE]`. **ARC-B2 / ARC-C1** (BCC-bridge / quantization) reach **UNDERDETERMINED** — a "FOUND-at-ARC-2" verdict would be an **overclaim**: the determinant grading `16G*³` is an *asserted* master-quadratic Vieta target, not a forward detdet_ζ identity (the J-twisted ζ-reg determinant ratio `=G*` is a genuine clean odd source, but `Det = Tr·G*` is not forced — `16G*³ = x₊x₋` is an ordinary product, and a 2×2's trace and determinant are independent). See `AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md` + the three pre-registered attempts (FTD-0224). **Surviving route: ARC-D** (engine-native measurement) or a `[CONJECTURE — new postulate]`. MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`; no spine tag moved.

**FTD-0242:** the operator-forcing question (W-CRIT-2) is sharpened to a **route-invariant boundary**. Four independent FTD-native routes — J-twisted ζ-determinant, BCC body-diagonal transfer operator, lemniscatic CM arithmetic of `E: y²=x³−x`, and a forced variational/period-ring/K-theory channel — were each force-attempted then adversarially refuted: **0 of 4 forced** (`cleanForcedRoutes = []`). Forward-forced `[DERIVED]`: the trace `16G*²` and the existence of a clean FTD-native odd source (`det_ζ(D_{3/4})/det_ζ(D_{1/4}) = G*`, which genuinely lifts the bare parity no-go so `16G*³ = 16G*²·G*` is *assemblable*). **Not** forced: the operator assembly itself — for a 2×2, trace and determinant are independent invariants, so the det_ζ ratio supplies the odd scalar but forces neither the gluing nor that it lands in the determinant slot (the imposed master-quadratic Vieta target). Conclusion: **α is dynamical, not structural**; the boundary is `[STRONGLY MOTIVATED CONJECTURE no-go]`, **not** `[THEOREM]` (RSI Leg 3 stays open). MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`; `x₊ = 1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`; no spine tag moved. See `../07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md` (FTD-0242).

**FTD-0244:** K-BIND is closed theorem-negative for the axiomatized substrate-native operator calculus `𝔠`. Trace and determinant data of every operator in `𝔠` remain in Q(G\*), while the master quadratic's root-splitting field is a degree-2 extension. Therefore no current substrate-native operator can force `(Tr,Det)=(16G*²,16G*³)` or select the physical root without an external W-like selection. This **closes W-CRIT-2's operator-forcing version**; it does not derive α and does not move FTD-0013 above `[STRONGLY MOTIVATED CONJECTURE]`.

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| **MC-T4.3** | **Algebraic-spine  physical electromagnetic readout mechanism. Central FTD claim.** `SPEC_ALPHA_READOUT_CONTRACT.md` formalizes the closure contract: specify `(P, A_obs, O_EM, R, C)` before target-checking; avoid alpha input; survive structural-decoupling diagnostics; explain why the output is an operational EM coupling rather than a distinguished number. All current action/operator/readout routes are exhausted or boundary-mapped; surviving positive exits are a new W-like commitment or fresh ARC-D measurement. | **[FOUNDATIONAL OBSTRUCTION]** | **FO** | MC-T4.1, MC-T4.2 |
| MC-T4.1 | Two-layer ontology axiomatization. **Reframed to documentation alignment** — substantive ontology already J-primary via SPEC_FTD.md §1.1 graded-monism table + Genesis rule. Postulate 3 textual update remaining | [OPEN — documentation] | D | — |
| MC-T4.2 | Phase-2 EFT non-Gaussian flow at `b ≥ 4`. Gates 6/7 of bridge contract. Phase-2 b=4, b=8 measurements show Gaussian fixed point holding within 1σ; non-Gaussian mixing matrix uncomputed | [OPEN] | M–RP | — |
| FTD-0096-mass | µ-from-ℓ_P missing arrow: mass-unit derivation from `ℓ_P` without passing through `m_e`. The type-theoretic no-go closes both the length analogue and mass-unit version; μ remains an external calibration. | [CLOSED THEOREM-NEGATIVE] | closed | — |
| W-CRIT-1 | **Circularity in framework integer identification.** Integers `{N_c=3, N_base=4, b_3=7, N_eff=13}` selected knowing target physics values. Constraint 11 of gtca: LEDGER tagging is not resolution. A reviewer who insists "you must derive these from axioms or the framework is empty" cannot be answered by current structural-uniqueness scans alone | [OPEN methodological] | (closes if MC-T4.3 closes) | MC-T4.3 |
| W-CRIT-2 | **Master quadratic operator assembly imposed not derived.** Closed theorem-negative for current substrate-native operator forcing by FTD-0244; the accepted boundary is that α remains dynamical, not structural, unless a new W-like commitment or ARC-D path is added. **Strengthened (FTD-0326):** no FTD-native ℤ/2 symmetry can supply the `δ`-selection (all are `ℚ`-entry, Galois-blind to `δ`); the W-like commitment can only be a *declaration* — adopted on main as **FC-W** (FTD-0315, the constitution's FC-4), not a native derivation. | [CLOSED THEOREM-NEGATIVE / ACCEPTED BOUNDARY] | closed | MC-T4.3 |

## §10.1 · MC-T4.3 Candidate Mechanism Decomposition

The central obstruction is narrow enough to split into candidate
mechanism classes. These are **not** claims and should not be cited as
derivations. They are work packages for making "non-action injection"
formal enough to fail or survive. The controlling contract is
`SPEC_ALPHA_READOUT_CONTRACT.md`: any proposed closure must define a
pre-target tuple `(P, A_obs, O_EM, R, C)` and pass the hard exclusion
rules before it can affect the `x_+ <-> 1/alpha` tag.

| Candidate | Formal target | Immediate falsifier | Tag |
|---|---|---|---|
| **A. Boundary-condition readout** | Specify a finite/undefined-boundary condition on the FTD lattice whose self-consistency spectrum has the master-quadratic root as the unique admissible electromagnetic readout. The rule must be stated without `α` or CODATA constants. | The boundary rule either has a free tunable parameter equivalent to `α`, or admits multiple comparable roots/readouts. | [CLOSED NEGATIVE for ARC-A1] |
| **B. Observable-selection readout** | Define an FTD-native observable algebra or reference frame projection whose distinguished eigenmode is `x_+`, and show why that observable is what scattering/charge measurements access. | The selected observable is merely post-hoc, or cannot be tied to an operational measurement protocol. | [CLOSED-NEGATIVE for primary catalog items per AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE_SYNTHESIS.md (FTD-0205)] |
| **C. Quantization/readout rule** | Derive a discrete measurement rule that maps the FQCR/master-quadratic dominant eigenvalue to `g_c²` or `α` without passing through a continuous-QFT action. | The rule reduces to `g_c` insertion, imported QED normalization, or an already-closed topological/action route. | [BOUNDARY / K-BIND operator route CLOSED THEOREM-NEGATIVE] |
| **D. Discrete-native measurement path** | Bypass continuous-QFT reconstruction and compare engine-native cluster interaction/lifetime/spectrum observables directly to measured quantities. | The engine observable is not L-stable, calibration-independent, or operationally tied to an experiment. | [OPEN; ARC-D1 CLOSED NEGATIVE] |

**Closure criterion for all four:** a successful mechanism must (1) be
stated before checking the target value, (2) avoid α as an input, (3)
survive the structural-decoupling diagnostics in `FOUND_STRUCTURAL_DECOUPLING.md`,
and (4) explain why the output is a physical electromagnetic coupling,
not merely a distinguished algebraic number.

**The 4-leg empirical diagnostic for MC-T4.3 (preserved for context):**

| Test | Domain | Result |
|---|---|---|
| FTD-0004 (Phase G) | Static V(r) | Lattice Poisson kernel; no fine-structure content |
| FTD-0005 (Phase J) | Partition function | Ultralocal at L=2; algebraic spine structurally decoupled from action |
| FTD-0125 (Phase I) | Dynamical V(r) | Engine V(r) does not carry `G_C²`; gauss-projection erases longitudinal `G_C` every tick |
| FTD-0126 (Phase II) | Matter-sector vertex | Wilson-Dirac + fixed B-field: `a_e` rel_err 683.95 vs Schwinger; outcome C |

**Sources:** SPEC_DOCTRINE_LEDGER.md §13.5; `../02_foundations/FOUND_STRUCTURAL_DECOUPLING.md` (FTD-0129 [SYNTHESIS]); `../10_eft_program/derivations/FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md` (FTD-0244); LEDGER FTD-0004, FTD-0005, FTD-0096, FTD-0125, FTD-0126, FTD-0129; `../../../evaluation/AUDIT_WEAKNESSES_MASTER.md` W-CRIT-1, W-CRIT-2.

**Candidate B pre-registration:** the design of the first closure attempt against Candidate B is locked in [`../10_eft_program/preregistrations/PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md`](../10_eft_program/preregistrations/PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md), git tag `preregister-alpha-readout-observable-selection-v1`, LEDGER row FTD-0198 [PRE-REGISTRATION]. The pre-reg locks the question, the FTD-native non-site-local observable catalog (state field, flux field + dual substrate, bilinear link observables, plaquette bivectors, Wilson-loop traces, boundary-to-boundary transfer observables, reference frame projections), the MC-T4.3 contract benchmark, three pre-blessed outcomes (FOUND / UNDERDETERMINED / CLOSED-NEGATIVE), the falsifier F-a..F-j, the banned moves, and the 11-step method. **Closure attempts executed:** The closure attempts for the three primary catalog items (plaquette bivectors, boundary-to-boundary transfer, and reference frame projections) were executed per the pre-reg's 11-step method. All three attempts resulted in a **CLOSED-NEGATIVE** verdict by categorical structural mismatch (FTD-0205, see companion audit synthesis [`../10_eft_program/archive/closed_negative/AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE_SYNTHESIS.md`](../10_eft_program/archive/closed_negative/AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE_SYNTHESIS.md)). The FTD-native discrete lattice spectrum is mathematically of a different category than the lemniscatic-curve periods. No spine tag moves; FTD-0013 status unchanged.

---

# §11 · Pre-registered scans awaiting execution

**None.** The queue's last entry executed 2026-07-12:

| ID | Pre-reg tag | Search space | Verdict |
|---|---|---|---|
| **FTD-0143** | `preregister-fqcr-quotient-uniqueness-v1` | 7⁴ = 2,401 exponent quadruples in `{2,…,8}⁴` × 20 targets × 4 tolerances | **EXECUTED 2026-07-12 — Outcome B [CLOSED NEGATIVE]: uniqueness rejected** (all 2401 quadruples match α⁻¹ at 10⁻⁵; canonical ranks 1333/2401; readout quadruple-insensitive at t=1). Model IV stays [SELECTION], no uniqueness backing. `reports_and_audits/ANALYSIS_FQCR_QUOTIENT_UNIQUENESS.md` |

---

# §12 · Dependency notes

```
§1 spine completion
   δ_c          ── CLOSED (FTD-0224, scan-negative)

§2 EM / α
   MC-T4.3      ── needs MC-T4.1, MC-T4.2  [foundational obstruction]
   §7-bivector  ── (no deps; FTD-0073 closed-negative scopes the construction; kinematic basis only per FTD-0379/0380)
   §7-dirac     ── needs SCOPE_VERTEX_PROGRAM V1 (Branch-B; de-gated from §7-bivector 2026-07-10)
   §7-loop      ── needs MC-T4.3
   §7-qed       ── needs §7-dirac

§3 EW / Higgs
   §8-running   ── (no deps)
   §9-chiH      ── (no deps)

§4 QCD
   §11-confine  ── (no deps; structural obstruction recognised)
   §11-thetaQCD ── (no deps)
   eng-SU3      ── needs §11-confine

§5 flavor
   §10-depths   ── (no deps)
   me-prefactor ── (no deps)
   quark-mass   ── needs §10-depths

§6 gravity
   §12-beyond-N   ── conditional on clock-hypothesis AXIOM
   §12-EP, §12-mgcurv, BH, MC-T4.4 ── (no deps)

§7 QM foundations: all (no deps) except Bell-mech needs W-CRIT-4

§8 cosmology
   W-COSMO-3    ── needs §3, §9
   stellar      ── needs §6

§9 engine  algebra
   MC-T3.1      ── (no deps; boundary mapped by FTD-0269)
   MC-T3.4      ── (no deps)
   MC-T3.6      ── (no deps)             [β-coefficient substrate-derivation; §3 EW; new ID per W2.5]
   FTD-0110-NL  ── needs MC-T3.1 current-stack calibration derivation

§10 foundational
   MC-T4.3      ── critical path; ontology-extension question
   MC-T4.1      ── doc-alignment, no deps
   MC-T4.2      ── (no deps; provides input to MC-T4.3)
   W-CRIT-1     ── broad methodological challenge; not closed by W-CRIT-2
   W-CRIT-2     ── closed theorem-negative for current operator forcing (FTD-0244)
```

**Critical paths:**

- **Spine-complete:** δ_c is the remaining small pure-math gap.
- **Engine-bridge-complete:** MC-T3.1 / FTD-0110-NL current-stack calibration derivation plus MC-T3.4 (parallel). Estimate: ~2 months focused engine + theory.
- **Bridge-complete (full):** MC-T4.1 → MC-T4.2 → MC-T4.3, with MC-T4.3 carrying foundational-obstruction risk that could halt closure indefinitely. **Unbounded.**

**Paper A is publishable from the current "complete" set** — none of §10's foundational items are required. §1 spine completion + §2 §7 § structural-uniqueness scans are sufficient.

---

# §13 · Closed-negative results (provenance preservation)

These are explicit no-go results. Re-attempting them without new structural insight wastes effort.

| Closure | Sector | What |
|---|---|---|
| FTD-0031, FTD-0093 | §4 | All three first-principles routes for `g_c` (Mechanisms A, B, C) closed; `g_c` remains [PARAMETRIC] |
| FTD-0050 | §1, §2 | Master quadratic as RG-step characteristic polynomial; engine stencil orthogonal to BCC |
| FTD-0073 | §2, §7 | Site-local Clifford on finite blocks; mode-erasure theorem constrains §7 doctrine bridge sector |
| FTD-0094 | §2 | L2 candidate identity `2·m_e/α = 16G*²` [PARAMETRIC] terminal |
| FTD-0096 | §10 | µ-from-ℓ_P missing-arrow length and mass-unit versions [CLOSED THEOREM-NEGATIVE]; μ remains an external calibration |
| FTD-0116 | §2 | Z-factor reading falsified via Q4a numerical test |
| FTD-0131 (1/100) | §6 | Framework-integer `G_N = 1/(b_3 + N_c)² = 1/100` reading off by 2.5 to 43 orders of magnitude |
| FTD-0035 | §6 | Mechanism γ gravitational `a_phys` derivation closed; calibration `a_phys ≡ ℓ_P` recommended |
| FTD-0025 | §4 | All three confinement substrate-derivation routes closed-negative |
| FTD-0018, FTD-0019, FTD-0020, FTD-0021, FTD-0022 | §3, §4 | sin²θ_W = 3/13, sin²θ_13 = 1/52, α_s = 7/59, PMNS angles, 7-term α series — all retagged [PARAMETRIC] or [STRUCTURALLY MOTIVATED PARAMETRIC] in the demotion wave |
| FTD-0042, FTD-0043 | §4, §7 | Yang-Mills mass gap and Navier-Stokes regularity papers RETRACTED; FTD-0044 per-voxel mass gap survives as the load-bearing residual theorem |
| FTD-0079 | §1, §9 | (SC+FCC)/2  BCC Watson-integral identity (no exact identity exists; finite-L stencil mismatch ~3% bounds engine accuracy) |
| FTD-0208 | §6 | Clock-hypothesis L² budget substrate derivation closed negative; clock hypothesis retained as coordinate-level AXIOM |
| FTD-0244 | §2, §10 | K-BIND / operator-calculus route closed theorem-negative; no substrate-native operator forces the master-quadratic assembly |

For complete closed-negative provenance see `LEDGER.md` per-row entries.

---

# §14 · Cross-references to atomic file-level TODOs

`TRACKER_OPEN_ITEMS.md` at `docs/theory/07_assessment/` carries ~200 atomic `[OPEN]` markers across ~75 files. **The math-relevant subset is captured above**; the rest are:

- **Engine code** (§1 of TRACKER_OPEN_ITEMS): mostly closed; 3 [BLOCKED] DagEngine stubs awaiting sparse-cosmology branch trigger.
- **Documentation TODOs**: doc-only items in foundations + reference docs, not new mathematics.
- **Exploration scripts** (§8 of TRACKER_OPEN_ITEMS): unfinished investigations; their conclusions stop short of closed derivations but do not block sector progress.

For non-math file-level work, navigate to `TRACKER_OPEN_ITEMS.md` directly.

---

# §15 · Sunset rule

This tracker is refreshed when:

- A sector's gap count changes (open → closed or new gap surfaces).
- An effort code changes (e.g., MC-T4.3 closure attempt produces new sub-tasks).
- A new sector emerges (e.g., a non-SM sector if FTD's predictions extend beyond particle physics).
- `LEDGER.md` ledger row tag changes for any cited claim.

The tracker **sunsets** (becomes archive-only) when:

- Every sector's [OPEN] items close OR are tagged [FOUNDATIONAL OBSTRUCTION] with explicit acceptance recorded in `SPEC_ALGEBRAIC_SPINE.md`.
- Replaced by a successor tracker that subsumes its content.

When refreshing, increment the version line in the header (v1.0 → v1.1). When sunsetting, move to `docs/theory/archive/` with a stub at the original location pointing at the successor.

---

# §16 · Single-line summary

**Open math, sector-organised: §1 spine is closed (`δ_c` closed-form gap CLOSED scan-negative per FTD-0224; MC-T1.1-ext CLOSED per FTD-0350); §2 EM/α centers on MC-T4.3 after K-BIND closed theorem-negative (positive exits now require W-like commitment or fresh ARC-D) plus the §7 bivector/Dirac bridge sector all OPEN per FTD-0073; §3 EW has GUT→IR running and `χ_H` (Higgs-manif doc cluster CLOSED at its honest [SELECTION]+[PARAMETRIC] status); §4 QCD has confinement substrate with structural obstruction recognised; §5 flavor has depth matrices [PARAMETRIC scaffold] and `m_e` prefactor; §6 gravity keeps beyond-leading-order GR, EP, curvature-source, and general-motion LW open while the clock hypothesis is [CLOSED NEGATIVE, AXIOM-LEVEL] and the lattice-BH doc cluster is CLOSED/reclassified (TRACKER §2.1; FTD-0184 guardrail); §7 QM-foundations has Lorentz recovery as the live load-bearing item after FC-1 declines Hilbert/Bell recovery targets; §8 cosmology is mostly [SELECTION]/[PARAMETRIC] with 6 W-COSMO weaknesses; §9 engine-bridge has the FTD-0269 current-stack nonlinear N(A) calibration boundary and FTD-0277 v1 counting closed negative (MC-T3.4 CLOSED — FTD-0095 [THEOREM]); §10 cross-cutting carries MC-T4.3 + MC-T4.1 doc + MC-T4.2 + W-CRIT-1, while W-CRIT-2 is closed theorem-negative for current operator forcing; the pre-registered scan queue is empty (FTD-0143 EXECUTED 2026-07-12 — uniqueness rejected [CLOSED NEGATIVE]); closed-negative provenance now includes FTD-0143, FTD-0208, FTD-0224, FTD-0244, and FTD-0277 v1.**
