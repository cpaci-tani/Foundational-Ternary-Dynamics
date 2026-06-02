# SPEC · Open Math by Physics Sector

**Tag:** [REFERENCE] / canonical research-questions queue (replaces tier-aligned CHECKLIST)
**Date:** 2026-05-08
**Version:** 1.0
**Replaces:** `CHECKLIST_MATH_COMPLETE.md` (now archived; tier-aligned organisation preserved there for provenance)
**LEDGER:** FTD-0146 [SYNTHESIS] — sector-organised consolidation; introduces no new theorems
**Companion docs:**
- [`SPEC_DOCTRINE_LEDGER.md`](SPEC_DOCTRINE_LEDGER.md) — single-page status map (closed + open)
- [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) — canonical algebraic-spine reference (nine numbered results: six theorem-grade + three honestly-tiered, see §0)
- [`SPEC_FQCR.md`](SPEC_FQCR.md) — FQCR Models I–V
- [`SPEC_MATH_FIRST_ONTOLOGY.md`](SPEC_MATH_FIRST_ONTOLOGY.md) — primitives -> invariants -> readouts -> physics ordering principle
- [`SPEC_ALPHA_READOUT_CONTRACT.md`](SPEC_ALPHA_READOUT_CONTRACT.md) — MC-T4.3 closure contract / "earn the map" criteria
- [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) — atomic per-claim provenance
- [`../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md`](../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md) — T1–T5 tiers
- [`../07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md`](../07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md) — atomic file-level `[OPEN]` markers (non-math TODOs)

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
9. Engine ↔ Algebra bridge
10. Cross-cutting / Foundational obstructions

Each sector has: **scope · status snapshot · open math table · closed-negative reminders (if any) · sources**. Closed items are noted briefly for sector context only; full provenance lives in `LEDGER.md`.

---

# §1 · Pure mathematics / Algebraic spine

**Scope:** Number-theoretic and algebraic content of FTD's algebraic spine (nine numbered results: seven theorem-grade + two honestly-tiered, see `SPEC_ALGEBRAIC_SPINE.md` §0); independent of physics interpretation.

**Status snapshot:** Spine: nine numbered results — seven theorem-grade + two honestly-tiered (see `SPEC_ALGEBRAIC_SPINE.md` §0). Tier I 5/5 closed (2026-05-02). Tier II 4/4 closed; 0 structural theorems [OPEN]. 1 pre-registered scan awaiting execution. 1 small closed-form gap.

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| MC-T1.1-ext | `L ≥ 3` ultralocality proof or disproof for matched-stencil with Gauss-constraint-allowed configurations | [CLOSED NEGATIVE — disproof established 2026-05-23 per `scripts/proofs/proof_phase_j_general_L.py`; Theorem 7 retagged `[DISCONFIRMED for general L]` in SPEC_ALGEBRAIC_SPINE.md §7] | W (closed) | — |
| MC-T2.3-4 | Structural theorem for `d = −4` privilege beyond 63-discriminant numerical scan. **Effort downgraded 2026-05-08 (W2.6 audit, M → D-W)**: FTD-0122 / OT-1.5 already established `Z[BCC] ⊗ Q ≅ V_triv² ⊕ V_sign² ⊕ V_complex²` with `V_complex` carrying natural `Z[i]`-module structure. Among class-number-1 imaginary-quadratic fields `{d=1,2,3,7,11,19,43,67,163}`, **only `d=4` (`Q(i)`) has `\|O^×\| = 4`** (others have order 2 or 6 (`Q(ω)`)). The closure is a one-page argument under the `\|Z[i]^×\| = 4` unit-group condition: `d = −4` is the unique CM ring whose unit group has order 4, and FTD-0122's complex structure on V_complex requires exactly this. Ancillary routes (`L(E, 1)` analytical structure, Galois-theoretic) remain optional. | [CLOSED — resolved 2026-05-26 via unit-group uniqueness theorem in SPEC_ALGEBRAIC_SPINE.md §3] | D-W (closed) | FTD-0122 (already closed) |
| δ_c | `δ_c = x_- − 3 ≈ 0.024`: closed-form for the residual between the smaller root `x_- = 16G*³α` and the integer 3. (Historical framing: `δ_c = x_- − N_c`; **superseded** by v1.4 §5 retirement of the `x_- ↔ N_c` identification — LEDGER FTD-0014 removed in commit `ca7eb61`. The numerical question — closed form for `16G*³α − 3` — stands as a pure-math question independent of any physics identification.) Three candidate expressions match only 0.65–5% (engine `ontic.h` Layer 4) | [OPEN] | W–M | — |

**Closed (sector context):** Theorems 1–9 (FTD-0001, OT-1.1–1.8, 2.1–2.3, 4.1, 3.4 partial), MC-T1.1 (route b, Theorem 7 retagged `[THEOREM at L=2]`), **MC-T1.1-ext (CLOSED NEGATIVE 2026-05-23, general-L ultralocality disproved per `proof_phase_j_general_L.py`; Theorem 7 spine entry retagged `[DISCONFIRMED for general L]`)**, MC-T1.2 (Theorem 3 retagged `[NUMERICAL FACT, h=1 only]`), MC-T1.3 (Q(G*) verification), MC-T1.4 (per-voxel mass gap), MC-T1.5 (BCC complex-structure Roles 1+3 [DERIVED]; Roles 2+4 NO-GO), MC-T2.1 + MC-T2.2 (extended polynomial scan, ~4×10⁵:1 Bayes), MC-T2.3 items 1–3 (63-disc Γ-product null at h ≥ 2), MC-T2.3-4 (CLOSED 2026-05-26, unit-group uniqueness proof integrated).

**Sources:** SPEC_ALGEBRAIC_SPINE.md §§1–10; TRACKER_ONTIC_TRUTH.md OT-1.x, OT-2.x; SPEC_DOCTRINE_LEDGER.md §§1–4.

---

# §2 · Electromagnetism / Fine structure (α)

**Scope:** `x_+ ↔ 1/α` identification (FTD's central physics claim); QED bridge from FTD substrate.

**Status snapshot:** `x_+ = 137.0362` matches `1/α` to 1.26 ppm as **[STRONGLY MOTIVATED CONJECTURE]** (OT-5.1, FTD-0013). Coefficient 16 = `\|Aut(E)\|²` is structural identification at OT-4.1 [T4]. Doctrine §7 bivector/Dirac/QED bridge sector all [OPEN] per FTD-0073 mode-erasure closure.

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| **MC-T4.3** | **Operational alpha-readout mechanism (CENTRAL).** A closure proof would have to derive the physical identification `x_+ = 1/alpha` from FTD structure rather than insert it. All natural action-level routes [CLOSED NEGATIVE]. `SPEC_ALPHA_READOUT_CONTRACT.md` now formalizes the closure contract and four candidate classes (boundary-condition / observable-selection / quantization-readout / discrete-native measurement); ARC-B1 observable-selection is the first proof obligation | **[FOUNDATIONAL OBSTRUCTION]** | **FO** | MC-T4.1, MC-T4.2 |
| **TEST4-GEN** | **Alpha arithmetic generativity test.** Pre-registers the Balmer-to-Bohr gate: the same lemniscatic CM/FQCR rigidity that produces the alpha candidate must generate one additional independent physical dimensionless observable or relation with no new tuned freedom. *(Historical exclusion: the `x_- ≈ N_c` match was previously called out as not counting as the prize; that identification is now **RETIRED** entirely per v1.4 §5 — see `DERIV_NC_FROM_TOPOLOGY.md` for the independent `N_c = 3` sources.)* Target declaration must precede numerical comparison. | [PRE-REGISTRATION] | D-W | MC-T4.3; `PREREG_ALPHA_ARITHMETIC_GENERATIVITY_v1.md` |
| §7-bivector | Lorentzian signature from bivector duality on FTD lattice. Requires non-site-local Clifford construction compatible with FTD-0073 (site-local Clifford [CLOSED NEGATIVE] under pointwise-threshold dynamics) | [OPEN] | RP | — |
| §7-dirac | Tree-level `g = 2` from FTD-substrate Dirac (currently [OPEN — IMPORTED scaffold]) | [OPEN] | M | §7-bivector |
| §7-loop | One-loop `a^{(1)} = α_FQCR/(2π)`. Requires `α_FQCR ↔ α` physical identification (= FTD-0013 [SMC]) | [OPEN] | RP | MC-T4.3 |
| §7-qed | Full QED `g − 2` precision (= MC-T4.4 in old checklist) | [OPEN] | W–M | §7-dirac |
| α-from-CM | `CONJ_ALPHA_FROM_CM.md` Step 3 (Z₄ symmetry selects this CM curve) and Step 8 (larger root = 1/α specifically) remain [STRONGLY MOTIVATED CONJECTURE], not [THEOREM] | [OPEN] | RP | — |
| Watson-G* | `DERIV_WATSON_GSTAR_IDENTITY.md` epilogue carries 1 [OPEN] | [OPEN] | W | — |
| α-lattice | `DERIV_ALPHA_LATTICE_MECHANISM.md` Steps 3 + 8 selection issue (same as α-from-CM) | [OPEN] | M | α-from-CM |

**Closed-negative — do not re-attempt:**
- R1 transverse stiffness; R2 source-current normalization; R3 two-sector response eigenvalue; R4 projected Dirac matter (all in `archive/closed_negative/`).
- Z-factor reading (FTD-0116, Q4a); RG-running; algebraic combinations; 1/√d; Langevin-equipart; monomial scans (FTD-0097 look-elsewhere).
- BZ²/9.6-ppb/two-loop α numerical closure (superseded by native-electrodynamics pivot; `DERIV_LATTICE_QED_COMPLETE.md` fully closed 2026-04-22).

**Sources:** SPEC_DOCTRINE_LEDGER.md §5, §7; LEDGER FTD-0013, FTD-0073, FTD-0116, FTD-0097; TRACKER_ONTIC_TRUTH.md OT-5.1, OT-4.1; TRACKER_OPEN_ITEMS.md §§4.2, 4.4, 6.5, 7.7.

---

# §3 · Electroweak / Higgs

**Scope:** SU(2) × U(1) sector; electroweak symmetry breaking; Higgs mechanism; weak-sector masses.

**Status snapshot:** Doctrine §8 establishes GUT-lock `sin²θ_W = 3/8` (standard SU(5) trace-normalisation; **3:5 ratio is [IMPORTED] per FTD-0149** — no FTD substrate ingredient enters; the 3/8 value is [THEOREM once 3:5 imported] but the IMPORT is doing the work, not FTD content). Canonical IR fit `sin²θ_W ≈ 3/13` [PARAMETRIC, FTD-0018] (3.5% off CODATA 0.22290(30); demoted 2026-04-19, M_Z scale annotated 2026-05-08 per FTD-0150). RG running between scales [OPEN/HARDENING]. Doctrine §9 `v = √2 m_t` [BORROWED EMPIRICAL] (textbook `y_t ≈ 1`, not novel). Higgs `χ_H = 2 − 3 Ξ_t + Ξ_bos` is scaffold; computation [OPEN].

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| **MC-T3.6** | Substrate-derive β-coefficients `b_Y = 41/6`, `b_2 = −19/6` from finite spectra (RG running of `sin²θ_W^lock` from GUT to M_Z; the IR `sin²θ_W` should fall out from running the GUT-lock 3/8 — currently the IR fit FTD-0018 [PARAMETRIC] 3/13 is independent). New ID introduced 2026-05-08 to resolve W2.5 MC-T3.5 collision (old MC-T3.5 = §9 FTD-0110 multi-scale boundary correction) | [OPEN/HARDENING] | M–RP | — |
| §8-running (alias of MC-T3.6) | Same as MC-T3.6 above; this row preserved as the doctrine §14 priority-2 cross-link target | [OPEN/HARDENING] | M–RP | MC-T3.6 |
| §9-chiH | `χ_H` derivation from FTD substrate. No canonical anchor | [OPEN] | M | — |
| Higgs-manif | `DERIV_HIGGS_FROM_MANIFESTATION.md` carries 3 [OPEN] (manifestation mechanism of EWSB) | [OPEN] | M | — |
| SU(2)-weak | `DERIV_LATTICE_SU2_WEAK.md` carries 5 [OPEN] (chiral structure, left-handed doublets, weak mixing via ungerade sector) | [OPEN] | RP | — |

**Closed (sector context):** SM gauge group `G_SM = (SU(3) × SU(2) × U(1))/Z_6` adopted as [IMPORTED structural match]; `Z_6` center closure [THEOREM within scaffold]; `Q = T_3 + Y` [IMPORTED]; neutral Higgs lock preserves `U(1)_EM` [THEOREM within scaffold].

**Sources:** SPEC_DOCTRINE_LEDGER.md §§8–9; LEDGER FTD-0017, FTD-0018; TRACKER_OPEN_ITEMS.md §§2.5, 2.6.

---

# §4 · QCD / Strong / Color

**Scope:** SU(3)_c sector; confinement; color charge; strong CP.

**Status snapshot:** `N_c = 3` is independently sourced — see `DERIV_NC_FROM_TOPOLOGY.md` (four routes) and the Moore Layer Theorem. *(The historical identification `x_- ↔ N_c` (0.80%, OT-5.2, FTD-0014) is **RETIRED** per v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`.)* `b_3 = (11 N_c − 2 n_f)/3 = 7` [IMPORTED COEFFICIENT, THEOREM once formula imported]. `α_s = 7/59` [PARAMETRIC, FTD-0020 demoted 2026-04-19]. Confinement substrate-derivation has **structural obstruction recognised 2026-05-03 night audit**: no Phase-G analog for area-law behavior because confinement is intrinsically non-classical (lives in `Z = ∫dU exp(−S)`) and FTD substrate is deterministic.

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| §11-confine | Substrate-derive QCD trace-gap confinement. Compact-U(1) link-variable formulation imported wholesale from textbook lattice gauge theory; `β = x_-` insertion is selection. Closing path: substrate-derive an effective compact-U(1) sector with inverse coupling flowing to `x_-` in the IR (parallel to Phase G's Gauss→Poisson chain) | [OPEN, structural obstruction] | RP–FO | — |
| §11-thetaQCD | Strong CP `θ_QCD = 0` by finite discrete orientation closure | [CONJECTURE / NEEDS THEOREM PACKAGING] | M | — |
| SU(3)-gauge | `DERIV_LATTICE_SU3_GAUGE.md` 5 [OPEN] (theoretical counterpart to engine §1.3) | [OPEN] | RP | — |
| eng-SU3 | Engine `phase_forces()` three-regime piecewise color force still imposed; replace with dynamical SU(3) gauge field whose Wilson-loop expectation produces linear confinement without hand-inserted regime switches | [OPEN] | M–RP | §11-confine |
| chiral-anom | `DERIV_LATTICE_CHIRAL_ANOMALY.md` 3 [OPEN] | [OPEN] | M | — |
| δ_c-color | `δ_c = x_- − 3 ≈ 0.024` closed form (cross-listed from §1; the historical `x_- − N_c` framing is **retired** v1.4 §5 — the pure-math question stands independently of any physics identification) | [OPEN] | W–M | — |

**Closed-negative — do not re-attempt:**
- All three first-principles routes for `g_c` (Mechanisms A, B, C; FTD-0031, FTD-0093). `g_c` remains [PARAMETRIC].
- Three substrate-derivation routes for confinement attempted 2026-05-03 night: (1) BCC eigenvalue triple-cosine product at `x_-`; (2) discriminant trichotomy phase argument; (3) Phase J ultralocality as confinement signature — all CLOSED NEGATIVE.

**Sources:** SPEC_DOCTRINE_LEDGER.md §11; `DERIV_NC_FROM_TOPOLOGY.md` (independent `N_c = 3` routes); LEDGER FTD-0020, FTD-0025 (2026-05-03 night annotation), FTD-0029, FTD-0031, FTD-0093; (FTD-0014 retired per v1.4 §5, row removed in commit `ca7eb61`); TRACKER_OPEN_ITEMS.md §§1.3, 2.4, 2.8.

---

# §5 · Flavor / Masses

**Scope:** Charged-lepton + quark mass hierarchy; CKM matrix; flavor structure.

**Status snapshot:** `m_e/m_P = √(2π)·(16/3)·α¹¹` to 0.19% [STRONGLY MOTIVATED CONJECTURE, FTD-0015]. **Exponent `n = 11` [DERIVED]** (MC-T3.2 closure 2026-05-02, given multiset theorem FTD-0084 + 2 SM-hierarchy SELECTIONs). `m_p/m_e` to 174 ppm [STRONGLY MOTIVATED CONJECTURE, FTD-0016]. Mass ratios `m_μ/m_e`, `m_τ/m_e` to ~5%. CKM order-of-magnitude only. Doctrine §10 depth matrices [PARAMETRIC candidate scaffold].

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| §10-depths | Explicit transfer matrices forcing `N_E = diag(9,3,0)`, `N_U = diag(12,5,0)`, `N_D = diag(7,4,0)`. Currently no canonical derivation — reverse-engineered from mass-ratio fits | [OPEN] | M | — |
| §10-depths-method | **Methodological honesty audit (added 2026-05-08).** The §10 depth matrices have **18 free integer slots** (9 in N_E∪N_U∪N_D + 9 per-fermion projection corrections C_F also tagged [PARAMETRIC]) — sufficient to fit any 3×3 hierarchy with 3 OOM spread. The "0" in third position is forced by `q*^0=1`, not by structure. The §10-depths target above is mis-framed as "find the matrices"; the actual methodological gap is **show this scaffold has predictive content beyond fit count**. Until a genuinely-predictive constraint (e.g., a transfer-matrix derivation that fixes ≥10 of 18 slots from substrate) is in hand, the [PARAMETRIC candidate scaffold] tag risks overstating the degree to which depth matrices are a substrate object vs a curve fit | [OPEN — methodological] | W (audit) + M (predictive constraint) | — |
| me-prefactor | Substrate justification of FTD-0015 prefactor `√(2π)·(16/3)`. Promoting `α_G(e,e) ≈ 1.745 × 10⁻⁴⁵` from [DERIVED, postulate-conditional] to [DERIVED, axiom-conditional] requires this | [OPEN] | M–RP | — |
| quark-mass | `FOUND_DISCRETE_NATIVE_MASS_GENERATION.md` (retracted continuous QFT fits; replaced by native discrete mass paradigm — Class A voxel cardinality) | [OPEN] | RP | §10-depths |
| quark-bridge | `archive_proof_quark_masses_lattice.py` (archived post-hoc quark mass verification script) | [ARCHIVED] | — | — |

**Closed (sector context):** FTD-0015 `n = 11` exponent [DERIVED]; FTD-0016 `m_p/m_e` formula [STRONGLY MOTIVATED CONJECTURE]; FTD-0084 multiset theorem [DERIVED].

**Sources:** SPEC_DOCTRINE_LEDGER.md §10; LEDGER FTD-0015, FTD-0016, FTD-0084; TRACKER_OPEN_ITEMS.md §§4.1, 8.2.

---

# §6 · Gravity / GR

**Scope:** Newtonian limit, Schwarzschild, full GR, lattice black holes.

**Status snapshot:** **Partial closure 2026-05-03 (reconciled 2026-05-24 per `AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`)** (FTD-0131): `α_G(e,e) = (m_e/m_P)² ≈ 1.745 × 10⁻⁴⁵` matches measured to 0.38% as **[STRONGLY MOTIVATED CONJECTURE]** for the prediction (epistemic floor inherited from FTD-0015 [SMC] via `α_G = (m_e/m_P)²` tautology — the 0.38% precision is squared FTD-0015 precision, mechanical not new evidence) plus **[DERIVED]** for the chain steps that recover Schwarzschild leading-order from substrate (Phase G + cluster mass + linearized tick + **1 flagged interpretive step: the clock hypothesis** used in SPEC_FTD_LAGRANGIAN.md §4.3; the original two postulates are subsumed by SPEC §4.2 + §4.3 [THEOREM]s, Reading A confirmed). Arc B P2 v1 closure attempt UNDERDETERMINED (`AUDIT_CLOCK_HYPOTHESIS_v1_UNDERDETERMINED.md`); v2 attempt INVALIDATED on process + substance axes (`AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md`, 2026-05-25); v3 pre-reg queued (target: substrate-derivation of quadratic L²-norm bandwidth-budget-conservation primitive). **Arc C2 spin-2 boundary theorem free-theory derivation landed 2026-05-24** (`DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md` + `DERIV_J_BILINEAR_NO_SPIN2_POLE.md`); Arc C2 P3 pre-reg (`preregister-spin2-boundary-theorem-v1`, FTD-0209) hash-locked. Framework-integer `G_N = 1/(b_3 + N_c)² = 1/100` reading [CLOSED NEGATIVE per FTD-0131] — off by `~10²⁰` to `~10⁴³` under any natural calibration.

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| §12-clock-hypothesis | Substrate-derive the **single** flagged interpretive step of FTD-0131 post-2026-05-24 reconciliation: the clock hypothesis used in SPEC_FTD_LAGRANGIAN.md §4.3 (the identification "Born-Infeld action measure IS proper time"). Original 2 postulates (P1 ρ_g coupling form, P2 `2/c²` linearized tick-rate) are subsumed by SPEC §4.2 + §4.3 [THEOREM]s (Reading A per AUDIT). Closes the chain side; the prediction floor [SMC] still inherits from FTD-0015 via `α_G = (m_e/m_P)²` tautology (independent route would require deriving FTD-0015's `√(2π)·(16/3)` prefactor — see §5 me-prefactor). Pre-reg v1 hash-locked: `preregister-clock-hypothesis-derivation-v1` (FTD-0208) → v1 UNDERDETERMINED; v2 attempt INVALIDATED 2026-05-25 (no tag, process violation + budget-conservation primitive not substrate-derived) — see `AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md`; v3 pre-reg queued targeting the budget-conservation primitive itself | [OPEN; v1 UNDERDETERMINED, v2 INVALIDATED, v3 pre-reg queued] | W–M | — |
| §12-beyond-N | Beyond-leading-order GR: Mercury perihelion, light bending, gravitational waves (full nonlinear Einstein equations beyond Deser bootstrap [SELECTION, FTD-0026]) | [OPEN] | RP | §12-postulates |
| §12-EP | Equivalence-principle analogue from substrate. No canonical anchor | [OPEN] | RP | — |
| §12-mgcurv | Mass-gap to curvature source. No canonical anchor | [OPEN] | RP | — |
| BH | `DERIV_LATTICE_BLACK_HOLES.md` 11 [OPEN] — **highest-density derivation cluster in repo**. Horizon thermodynamics, Hawking radiation lattice derivation, information paradox at discrete scale, Kerr-Newman generalisation | [OPEN] | RP | — |
| MC-T4.4 | General-motion lattice Liénard-Wiechert: closed-form for general accelerating motion. Closed at uniform velocity [DERIVED]; sinusoidal Larmor case has Bessel infinite-series form (FTD-0120 Q5); general motion only formal Q5★ frequency-domain expression | [OPEN] | W–M | — |

**Closed (sector context):** FTD-0004 Phase G geometric Coulomb [THEOREM]; FTD-0110 cluster mass [DERIVED at linear level]; FTD-0131 leading-order Newton ([SMC] prediction floor inherited from FTD-0015; [DERIVED] chain steps); FTD-0113 retarded Green identity [DERIVED]; FTD-0115 lattice Cherenkov closed at uniform velocity.

**Closed-negative — do not re-attempt:** FTD-0035 Mechanism γ gravitational `a_phys` derivation (closed 2026-04-19; calibration `a_phys ≡ ℓ_P` recommended); "1/100" framework-integer reading (FTD-0131).

**Sources:** SPEC_DOCTRINE_LEDGER.md §12; `DERIV_NEWTON_FROM_SUBSTRATE.md`; LEDGER FTD-0004, FTD-0026, FTD-0110, FTD-0113, FTD-0115, FTD-0131; TRACKER_OPEN_ITEMS.md §§2.1, 3.2.

---

# §7 · Quantum foundations / Bell / Lorentz

**Scope:** Emergence of QM from FTD substrate; Bell violation; Lorentz invariance recovery; observer mechanisms.

**Status snapshot:** Bell `S = 2√2` [SELECTION, FTD-0023] — **not derived**. Moore-Laplacian isotropy verified at O(h²) and O(h⁴) (rotationally invariant correction `(h²/12)·(∇²)²f`); empirical 11–20% pairwise diff at L=48–64 (high-k dispersion artifact present in every cubic-lattice FD scheme). Continuum-limit Lorentz recovery [OPEN].

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| W-CRIT-3 | Lorentz invariance recovery: relational reinterpretation step from "isotropic Laplacian + emergent SR-like dispersion" to "Lorentz invariance" | [OPEN] | M–RP | — |
| W-CRIT-4 | Bell violation rigorous: explicit construction of an sLoop process producing `S > 2` from the 5 axioms; OR honest acceptance that FTD reproduces Bell *bound* (Tsirelson) via emergent QM but not the violation mechanism | [OPEN] | M–RP | — |
| QM-lattice | `DERIV_QM_FROM_LATTICE.md` 2 [OPEN] (Hilbert-space construction is constructed not emergent per QIS evaluation) | [OPEN] | RP | — |
| Wigner | `FOUND_WIGNERS_FRIEND_RESOLUTION.md` 3 [OPEN] | [OPEN] | M | — |
| vN-chain | `FOUND_VON_NEUMANN_CHAIN.md` 3 [OPEN] (related to N_meas = 18 = `\|SC\| + \|FCC\|` identification) | [OPEN] | M | — |
| Bell-mech | `DERIV_OBSERVER_BELL_MECHANISM.md` 1 [OPEN] | [OPEN] | M | W-CRIT-4 |
| Born | `FOUND_BORN_RULE_NULL_CONE.md` 1 [OPEN] | [OPEN] | W–M | — |
| Existence | `FOUND_THE_EXISTENCE_FILTER.md` 1 [OPEN] | [OPEN] | M | — |

**Sources:** SPEC_DOCTRINE_LEDGER.md §1, §7; LEDGER FTD-0023; AUDIT_WEAKNESSES_MASTER.md W-CRIT-3, W-CRIT-4; TRACKER_OPEN_ITEMS.md §§2.10, 5.1–5.6.

---

# §8 · Cosmology

**Scope:** ΛCDM-relevant predictions; inflation; dark matter; cosmic structure.

**Status snapshot:** Most cosmology predictions sit at [SELECTION] or [PARAMETRIC]. Per the 18-evaluation review (`AUDIT_WEAKNESSES_MASTER.md` W-COSMO 1–7): inflaton ad hoc; dark matter mechanism inconsistent; first-order EW transition assumed; `Λ = α^57` numerology without mechanism; no power spectrum/BAO predictions; NFW halo not derived.

> **Imported-content audit note (2026-05-08, W2.7).** §8 is the **most W-CRIT-1-vulnerable sector in the framework**: every cosmology entry is standard ΛCDM apparatus filled with FTD numerology, with the least substrate-derivation backing of any sector. Specifically: `Λ = α^57` is paradigm circularity (an FTD constant raised to a power chosen to match observation, with no substrate constraint on the exponent); inflaton-as-mean-flux is identification without dynamics; dark matter "mechanism inconsistent" indicates the W-COSMO-2 finding has not been resolved; NFW halo not derived (NFW is a phenomenological fit, and FTD currently neither derives nor contests it); power spectrum + BAO predictions are absent. Compared to §1 (algebraic-spine; theorems-grade), §2 (EM/α; conjecture with structural-uniqueness backing), §6 (gravity; partial closure FTD-0131), §8 stands out as **the sector where the most external structure has been imported without substrate justification** — and per the §13 doctrine non-circularity audit, this concentrates W-CRIT-1 risk. Closing W-COSMO-1 through W-COSMO-6 would require a substantive cosmological substrate-derivation program of multi-month-RP scale; in the meantime, manuscript chapters citing cosmological predictions should explicitly distinguish "imported ΛCDM apparatus + FTD numerology" from "substrate-derived". Cross-link: AUDIT_WEAKNESSES_MASTER.md W-COSMO-1 through W-COSMO-7.

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| W-COSMO-1 | Inflaton identification with mean flux is ad hoc | [OPEN] | M–RP | — |
| W-COSMO-2 | Dark matter mechanism: internally inconsistent | [OPEN] | M | — |
| W-COSMO-3 | First-order electroweak transition: assumed not derived | [OPEN] | M–RP | §3 §9 |
| W-COSMO-4 | `Λ = α^57`: numerology without mechanism | [OPEN] | RP | — |
| W-COSMO-5 | Power spectrum + BAO predictions: missing | [OPEN] | RP | — |
| W-COSMO-6 | NFW halo profile: not derived | [OPEN] | RP | — |
| stellar | `DERIV_STELLAR_LIFECYCLE_LATTICE.md` 3 [OPEN] | [OPEN] | M–RP | §6 |

**Sources:** AUDIT_WEAKNESSES_MASTER.md W-COSMO; TRACKER_OPEN_ITEMS.md §2.7.

---

# §9 · Engine ↔ Algebra bridge

**Scope:** Connecting algebraic spine theorems to engine empirical observations. The bridge that makes engine-as-instrument scientifically forceful rather than confirmation-bias-prone.

**Status snapshot:** Bridge exists at linear level (FTD-0110 `k = 1/N_base = 1/4` [DERIVED via O_h A_{1g} multiplicity], 2026-04-28). 1/5 closed; 3/5 NOT CLOSED; 1/5 BLOCKED.

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| **MC-T3.1** | FTD-0110 nonlinear bridge perturbation theorem. Mechanism γ crossover scale `A* = 12.8` matches empirical drift midpoint; **predicted slope `−1/A* ≈ −0.077` does NOT match empirical `−0.030` (~2.5× off)**. Discriminator experiments D3a–D3d on WSL2 RTX 5090 (~2 weeks GPU). Risk: HIGH — Mechanisms α + β already FALSIFIED | [OPEN] | M | — |
| **MC-T3.4** | Bridge Functional arithmetic-mean rule (FTD-0095): four candidate functionals (arithmetic / geometric / harmonic / quadratic mean) on master quadratic spectrum indistinguishable at available precision (~10–20% relative differences). Three closure routes (variational on σ_BCC, 't Hooft beable, Beilinson regulator) all research-program-scale | [OPEN] | M | — |
| MC-T3.5 | FTD-0110 multi-scale boundary-correction closure | [BLOCKED] | M | MC-T3.1 |
| L128-G2 | L=128 G2 follow-up to FTD-0107 — engine-side L-invariance test (32, 64, 128). Pre-registration template ready | [OPEN] | D–W | — |
| FTD-0110-NL | Linear→nonlinear bridge proof: instrument engine to log per-irrep energy fractions during steady-state run, verify {3/8, 1/8, 3/8, 1/8} A_{1g} distribution holds within Langevin-noise envelope. Closing this promotes FTD-0110's main claim from [STRONGLY MOTIVATED CONJECTURE] to [DERIVED]/[THEOREM]-grade | [OPEN] | W–M | — |

**Closed (sector context):** MC-T3.2 m_e exponent `n = 11` [DERIVED] (2026-05-02); MC-T3.3 (SC+FCC)/2 ↔ BCC bridge — **closed-negative for identity** (no Watson-integral identity), **closed-positive for symmetry** (shared O_h symmetry forces leading-order agreement); 25-voxel cluster size at canonical amplitude A=10 [DERIVED at linear level] (FTD-0110 closure 2026-04-28).

**Sources:** SPEC_DOCTRINE_LEDGER.md §13.5; `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`; LEDGER FTD-0107, FTD-0110, FTD-0095; TRACKER_OPEN_ITEMS.md §7.7.

---

# §10 · Cross-cutting / Foundational obstructions

**Scope:** Load-bearing methodological challenges that affect multiple sectors. The single most consequential gap (MC-T4.3) sits here.

**Status snapshot:** MC-T4.3 is the **central foundational obstruction**. All natural action-level α-injection routes [CLOSED NEGATIVE]. Convergent diagnostic across 4 independent engine tests confirms the master quadratic value `α = 1/x_+` does not flow into engine matter-sector dynamical observables under any classical-gauge protocol tested. Lead-physicist diagnosis: structural decoupling via Phase J ultralocality. **Closure may require ontology extension beyond the 5 axioms.**

**Update 2026-05-28 (FTD-0224):** the four ARC mechanism classes of `SPEC_ALPHA_READOUT_CONTRACT.md` have now each been attacked. **ARC-A** (boundary-condition) and **ARC-B1** (observable-selection, catalog items 4/6/7) closed `[CLOSED NEGATIVE]`. **ARC-B2 / ARC-C1** (BCC-bridge / quantization) reached **UNDERDETERMINED** — the 2026-05-27 "FOUND-at-ARC-2" verdicts were an **overclaim**, corrected 2026-05-28: the determinant grading `16G*³` is an *asserted* master-quadratic Vieta target, not a forward det↔det_ζ identity (the J-twisted ζ-reg determinant ratio `=G*` is a genuine clean odd source, but `Det = Tr·G*` is not forced — `16G*³ = x₊x₋` is an ordinary product, and a 2×2's trace and determinant are independent). See `AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md` + the three pre-registered attempts (FTD-0224). **Surviving route: ARC-D** (engine-native measurement) or a `[CONJECTURE — new postulate]`. MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`; no spine tag moved.

**Update 2026-06-01 (FTD-0242):** the operator-forcing question (W-CRIT-2) was sharpened to a **route-invariant boundary**. Four independent FTD-native routes — J-twisted ζ-determinant, BCC body-diagonal transfer operator, lemniscatic CM arithmetic of `E: y²=x³−x`, and a forced variational/period-ring/K-theory channel — were each force-attempted then adversarially refuted: **0 of 4 forced** (`cleanForcedRoutes = []`). Forward-forced `[DERIVED]`: the trace `16G*²` and the existence of a clean FTD-native odd source (`det_ζ(D_{3/4})/det_ζ(D_{1/4}) = G*`, which genuinely lifts the bare parity no-go so `16G*³ = 16G*²·G*` is *assemblable*). **Not** forced: the operator assembly itself — for a 2×2, trace and determinant are independent invariants, so the det_ζ ratio supplies the odd scalar but forces neither the gluing nor that it lands in the determinant slot (the imposed master-quadratic Vieta target). Conclusion: **α is dynamical, not structural**; the boundary is `[STRONGLY MOTIVATED CONJECTURE no-go]`, **not** `[THEOREM]` (RSI Leg 3 stays open). MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`; `x₊ = 1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`; no spine tag moved. See `../07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md` (FTD-0242).

| ID | Gap | Tag | Effort | Deps |
|---|---|---|---|---|
| **MC-T4.3** | **Algebraic-spine ↔ physical electromagnetic readout mechanism. Central FTD claim.** `SPEC_ALPHA_READOUT_CONTRACT.md` formalizes the closure contract: specify `(P, A_obs, O_EM, R, C)` before target-checking; avoid alpha input; survive structural-decoupling diagnostics; explain why the output is an operational EM coupling rather than a distinguished number. Candidate mechanisms: boundary-condition / observable-selection / quantization-readout / discrete-native measurement. Prior on near-term closure: low | **[FOUNDATIONAL OBSTRUCTION]** | **FO** | MC-T4.1, MC-T4.2 |
| MC-T4.1 | Two-layer ontology axiomatization. **Reframed 2026-05-02 to documentation alignment** — substantive ontology already J-primary via SPEC_FTD.md §1.1 graded-monism table + Genesis rule. Postulate 3 textual update remaining | [OPEN — documentation] | D | — |
| MC-T4.2 | Phase-2 EFT non-Gaussian flow at `b ≥ 4`. Gates 6/7 of bridge contract. Phase-2 b=4, b=8 measurements show Gaussian fixed point holding within 1σ; non-Gaussian mixing matrix uncomputed | [OPEN] | M–RP | — |
| FTD-0096-mass | µ-from-ℓ_P missing arrow (mass-unit version): mass-unit derivation from `ℓ_P` without passing through `m_e`. The LENGTH analogue is [CLOSED THEOREM-NEGATIVE]; the MASS-UNIT version is still [OPEN] | [OPEN] | M | — |
| W-CRIT-1 | **Circularity in framework integer identification.** Integers `{N_c=3, N_base=4, b_3=7, N_eff=13}` selected knowing target physics values. Constraint 11 of gtca: LEDGER tagging is not resolution. A reviewer who insists "you must derive these from axioms or the framework is empty" cannot be answered by current structural-uniqueness scans alone | [OPEN methodological] | (closes if MC-T4.3 closes) | MC-T4.3 |
| W-CRIT-2 | **Master quadratic imposed not derived.** Same root cause as W-CRIT-1; OT-3.3 polynomial-shape uniqueness is structural-uniqueness evidence, not derivation | [OPEN methodological] | (closes if MC-T4.3 closes) | MC-T4.3 |

## §10.1 · MC-T4.3 Candidate Mechanism Decomposition (2026-05-18)

The central obstruction is now narrow enough to split into candidate
mechanism classes. These are **not** claims and should not be cited as
derivations. They are work packages for making "non-action injection"
formal enough to fail or survive. The controlling contract is
`SPEC_ALPHA_READOUT_CONTRACT.md`: any proposed closure must define a
pre-target tuple `(P, A_obs, O_EM, R, C)` and pass the hard exclusion
rules before it can affect the `x_+ <-> 1/alpha` tag.

| Candidate | Formal target | Immediate falsifier | Tag |
|---|---|---|---|
| **A. Boundary-condition readout** | Specify a finite/undefined-boundary condition on the FTD lattice whose self-consistency spectrum has the master-quadratic root as the unique admissible electromagnetic readout. The rule must be stated without `α` or CODATA constants. | The boundary rule either has a free tunable parameter equivalent to `α`, or admits multiple comparable roots/readouts. | [OPEN] |
| **B. Observable-selection readout** | Define an FTD-native observable algebra or reference frame projection whose distinguished eigenmode is `x_+`, and show why that observable is what scattering/charge measurements access. | The selected observable is merely post-hoc, or cannot be tied to an operational measurement protocol. | [CLOSED-NEGATIVE 2026-05-23 for primary catalog items per AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE_SYNTHESIS.md (FTD-0205)] |
| **C. Quantization/readout rule** | Derive a discrete measurement rule that maps the FQCR/master-quadratic dominant eigenvalue to `g_c²` or `α` without passing through a continuous-QFT action. | The rule reduces to `g_c` insertion, imported QED normalization, or an already-closed topological/action route. | [OPEN] |
| **D. Discrete-native measurement path** | Bypass continuous-QFT reconstruction and compare engine-native cluster interaction/lifetime/spectrum observables directly to measured quantities. | The engine observable is not L-stable, calibration-independent, or operationally tied to an experiment. | [OPEN] |

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

**Sources:** SPEC_DOCTRINE_LEDGER.md §13.5; `FOUND_STRUCTURAL_DECOUPLING.md` (FTD-0129 [SYNTHESIS]); LEDGER FTD-0004, FTD-0005, FTD-0096, FTD-0125, FTD-0126, FTD-0129; AUDIT_WEAKNESSES_MASTER.md W-CRIT-1, W-CRIT-2.

**Candidate B pre-registration (2026-05-23):** the design of the first closure attempt against Candidate B is locked in [`../10_eft_program/PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md`](../10_eft_program/PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md), git tag `preregister-alpha-readout-observable-selection-v1`, LEDGER row FTD-0198 [PRE-REGISTRATION]. The pre-reg locks the question, the FTD-native non-site-local observable catalog (state field, flux field + dual substrate, bilinear link observables, plaquette bivectors, Wilson-loop traces, boundary-to-boundary transfer observables, reference frame projections), the MC-T4.3 contract benchmark, three pre-blessed outcomes (FOUND / UNDERDETERMINED / CLOSED-NEGATIVE), the falsifier F-a..F-j, the banned moves, and the 11-step method. **Closure attempts executed (2026-05-23):** The closure attempts for the three primary catalog items (plaquette bivectors, boundary-to-boundary transfer, and reference frame projections) were executed per the pre-reg's 11-step method. All three attempts resulted in a **CLOSED-NEGATIVE** verdict by categorical structural mismatch (FTD-0205, see companion audit synthesis `AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE_SYNTHESIS.md`). The FTD-native discrete lattice spectrum is mathematically of a different category than the lemniscatic-curve periods. No spine tag moves; FTD-0013 status unchanged.

---

# §11 · Pre-registered scans awaiting execution

| ID | Pre-reg tag | Search space | Pre-registered outcomes |
|---|---|---|---|
| **FTD-0143** | `preregister-fqcr-quotient-uniqueness-v1` | 7⁴ = 2,401 exponent quadruples in `{2,…,8}⁴` × 20 dimensionless physics targets × 4 tolerances `{10⁻³, 10⁻⁴, 10⁻⁵, 10⁻⁶}` | A: uniqueness confirmed → FQCR Model IV [SELECTION with uniqueness]; B: rejected → Model IV stays [SELECTION] without privileged-choice claim; C: partial → [PARTIAL] |

Effort: D (scan execution; analysis already templated).

---

# §12 · Dependency notes

```
§1 spine completion
   MC-T1.1-ext  ── (no deps)
   MC-T2.3-4    ── (no deps)
   δ_c          ── (no deps)

§2 EM / α
   MC-T4.3      ── needs MC-T4.1, MC-T4.2  [foundational obstruction]
   §7-bivector  ── (no deps; FTD-0073 closed-negative scopes the construction)
   §7-dirac     ── needs §7-bivector
   §7-loop      ── needs MC-T4.3
   §7-qed       ── needs §7-dirac
   α-from-CM    ── (no deps)
   α-lattice    ── needs α-from-CM

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
   §12-postulates ── (no deps)
   §12-beyond-N   ── needs §12-postulates
   §12-EP, §12-mgcurv, BH, MC-T4.4 ── (no deps)

§7 QM foundations: all (no deps) except Bell-mech needs W-CRIT-4

§8 cosmology
   W-COSMO-3    ── needs §3, §9
   stellar      ── needs §6

§9 engine ↔ algebra
   MC-T3.1      ── (no deps; supports MC-T3.5)
   MC-T3.4      ── (no deps)
   MC-T3.5      ── BLOCKED on MC-T3.1   [FTD-0110 multi-scale boundary correction; §9 engine-bridge]
   MC-T3.6      ── (no deps)             [β-coefficient substrate-derivation; §3 EW; new ID 2026-05-08 per W2.5]
   L128-G2      ── (no deps)
   FTD-0110-NL  ── (no deps)

§10 foundational
   MC-T4.3      ── critical path; ontology-extension question
   MC-T4.1      ── doc-alignment, no deps
   MC-T4.2      ── (no deps; provides input to MC-T4.3)
   W-CRIT-1, 2  ── close if MC-T4.3 closes
```

**Critical paths:**

- **Spine-complete:** §1 items are (no deps); all parallelisable. Estimate: ~3–6 weeks total small-team focused.
- **Engine-bridge-complete:** MC-T3.1 → MC-T3.5 → MC-T3.4 (with MC-T3.4 in parallel). Estimate: ~2 months focused engine + theory.
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
| FTD-0096 (length) | §10 | µ-from-ℓ_P LENGTH analogue [CLOSED THEOREM-NEGATIVE]; mass-unit version still OPEN |
| FTD-0116 | §2 | Z-factor reading falsified via Q4a numerical test |
| FTD-0131 (1/100) | §6 | Framework-integer `G_N = 1/(b_3 + N_c)² = 1/100` reading off by 2.5 to 43 orders of magnitude |
| FTD-0035 | §6 | Mechanism γ gravitational `a_phys` derivation closed; calibration `a_phys ≡ ℓ_P` recommended |
| FTD-0025 night-2026-05-03 | §4 | All three confinement substrate-derivation routes closed-negative |
| FTD-0018, FTD-0019, FTD-0020, FTD-0021, FTD-0022 | §3, §4 | sin²θ_W = 3/13, sin²θ_13 = 1/52, α_s = 7/59, PMNS angles, 7-term α series — all retagged [PARAMETRIC] or [STRUCTURALLY MOTIVATED PARAMETRIC] in 2026-04-19 demotion wave |
| FTD-0042, FTD-0043 | §4, §7 | Yang-Mills mass gap and Navier-Stokes regularity papers RETRACTED 2026-04-19; FTD-0044 per-voxel mass gap survives as the load-bearing residual theorem |
| FTD-0079 | §1, §9 | (SC+FCC)/2 ↔ BCC Watson-integral identity (no exact identity exists; finite-L stencil mismatch ~3% bounds engine accuracy) |

For complete closed-negative provenance see `LEDGER.md` per-row entries.

---

# §14 · Cross-references to atomic file-level TODOs

`TRACKER_OPEN_ITEMS.md` at `docs/theory/07_assessment/` carries ~200 atomic `[OPEN]` markers across ~75 files. **The math-relevant subset is captured above**; the rest are:

- **Engine code** (§1 of TRACKER_OPEN_ITEMS): mostly closed 2026-04-17; 3 [BLOCKED] DagEngine stubs awaiting sparse-cosmology branch trigger.
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

**Open math, sector-organised: §1 spine has 2 small gaps (`d=−4` structural theorem, `δ_c` closed form) post-2026-05-23 closure of `L≥3` ultralocality as DISCONFIRMED; §2 EM/α centers on MC-T4.3 (foundational obstruction, central FTD claim) plus the §7 bivector/Dirac bridge sector all OPEN per FTD-0073; §3 EW has GUT→IR running and `χ_H`; §4 QCD has confinement substrate with structural obstruction recognised; §5 flavor has depth matrices [PARAMETRIC scaffold] and `m_e` prefactor; §6 gravity has **1 flagged interpretive step of FTD-0131 (clock hypothesis; v1 attempt UNDERDETERMINED, v2 attempt INVALIDATED on process + substance, v3 pre-reg queued)** + Arc C2 spin-2 boundary theorem free-theory derivation landed + pre-reg locked (FTD-0209) + 11 lattice-BH items + general-motion LW; §7 QM-foundations has Bell mechanism and Lorentz recovery as the load-bearing items; §8 cosmology is mostly [SELECTION]/[PARAMETRIC] with 6 W-COSMO weaknesses; §9 engine-bridge has MC-T3.1 nonlinear slope mismatch (~2.5×) and MC-T3.4 four-mean indistinguishability; §10 cross-cutting carries MC-T4.3 (the central foundational obstruction) + MC-T4.1 doc + MC-T4.2 + W-CRIT-1/2 methodological challenges; 2 pre-registered closure attempts (FTD-0143 scan + FTD-0209 spin-2 boundary theorem) hash-locked, FTD-0208 v3 pre-reg queued; ~10 closed-negative items preserved for provenance.**
