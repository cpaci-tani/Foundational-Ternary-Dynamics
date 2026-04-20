# AUDIT — Constants Sweep, Session 4 (final)

**Date:** 2026-04-19
**Scope:** verify that the Session 2 + Session 3 renames (`alpha_inf` → `alpha_largeL`) and the new `a_phys ≡ ℓ_P` calibration declaration in `docs/SPEC_FTD.md` left the project numerically and notationally consistent. Adjacent spot-checks on `G_N` toy/physical, master-quadratic root quotation, and `K_B`/`c_lat`/`c_phys` pairing.
**Method:** programmatic grep of `alpha_inf|α_∞|α_inf|alphaINF`, `alpha_largeL`, `a_phys`, `K_B`, `C_SPEED`, `137.035999*`, `G_N`, `ALPHA_G_APPROX` across all `.py`, `.cpp`, `.h`, `.cu`, `.md`, `.tex`, `.lean` files. Comparisons against the canonical sources `scripts/constants.py`, `engine/include/ftd/ontic/*.h`, and the new SPEC_FTD calibration section.
**Severity:** HIGH = factual drift / live tech claim references retired notation. MEDIUM = live tech doc using stale terminology that would confuse a reader. LOW = cosmetic / changelog-historical references that legitimately preserve the old name.

---

## Summary

| Category | HIGH | MEDIUM | LOW | OK |
|---|---|---|---|---|
| Session-2/3 rename residue (`alpha_inf` / `α_∞` / `α_inf` / `\alphaINF`) | **1** | **6** | 7 | — |
| `a_phys` calibration consistency | 0 | **1** | — | 8 |
| `G_N` toy-vs-physical awareness | 0 | 0 | 0 | OK across engine + tools |
| Master-quadratic root quotation | 0 | 0 | 1 | OK across canonical files |
| `K_B = 0.511`, `c_lat = 1/√3`, `c_phys = 2.998e8` pairing | 0 | 0 | 0 | OK |
| `ALPHA_INV_NIST` literal drift (CODATA 2018 vs 2022) | 0 | 1 | 6 | partial |

Engine source tree (`engine/`) and dissemination tree (`dissemination/`) are CLEAN of `alpha_inf` / `α_∞` / `\alphaINF` — the Session-2/3 renames landed completely there. All remaining residue lives in `docs/theory/`, `README.md`, `CHANGELOG.md`, `META_DOCUMENTATION_MAP.md`.

---

## §1 · `alpha_inf` / `α_∞` rename residue

### HIGH

| # | Location | Line(s) | Severity | Finding | Recommended fix |
|---|---|---|---|---|---|
| 1.H1 | `README.md` | 158, 171 | HIGH | Project front-page README still cites `α_∞ ∈ [3.35×, 3.74×] α_ref` and "Continuum-limit α_eff(∞)" as live phase-F headline. This is the principal reader-entry point; the rename was specifically to retire the "∞" framing under the undefined-boundary commitment. | Restate as "α_largeL ∈ [3.35×, 3.74×] α_ref" or "α at the largest measured L (L=384) ∈ …", with one sentence noting that the value is the 1/L² fit at the largest tested L and not a literal "limit". |

### MEDIUM (live technical docs in `docs/theory/`)

| # | Location | Line(s) | Severity | Finding | Recommended fix |
|---|---|---|---|---|---|
| 1.M1 | `docs/theory/10_eft_program/DERIV_DYNAMICAL_SM_EMERGENCE.md` | 145, 155, 172, 175, 191 | MEDIUM | Phase 4C results table and "Honest conclusion" paragraph still use `α_inf = 0.0214` and "Pre-registered target α_inf = 1/137 ± 1%". This is the body text of an active derivation document, not a changelog. | Rename inline to `α_largeL = 0.0214` and "Pre-registered target α_largeL = 1/137 ± 1%". The fit-form comment `α(L) = α_inf + b/L²` should be `α(L) = α_largeL + b/L²`. |
| 1.M2 | `docs/theory/10_eft_program/DERIV_DAY2_CAMPAIGN.md` | 222, 345, 352, 371, 380, 449 | MEDIUM | Live Day-2 derivation document uses `\alpha_\infty = 2.94 α_ref`, "α_∞ ∈ [3.35, 3.74] × α_ref", "α_∞(classical) ≈ 0.013", and the retracted "α_∞ = 1.23×" line. The retraction text at 449 can keep the historical name (it is quoting the retracted claim verbatim); the others are live results. | Lines 222, 345, 352, 371, 380: rename to `α_largeL`. Line 449 may stay (historical quote) but should add "(retracted; called `α_∞` at the time)". |
| 1.M3 | `docs/theory/10_eft_program/AUDIT_ALPHA_EXTRACTION.md` | 4, 134, 137, 168 | MEDIUM | Live audit document uses "Phase F headline α_∞ ≈ 3.6× α_ref" and `α_∞ = 0.02566`. | Rename to `α_largeL`. The audit's verdict text on line 168 ("Phase F headline 'α_∞ ≈ 3.6× α_ref' is correct for the engine's…") may quote the original claim in scare-quotes, but should also restate in the new notation. |
| 1.M4 | `docs/theory/10_eft_program/OPEN_A_PHYS_DERIVATION.md` | 81 | MEDIUM | One occurrence in active prose: "The EFT recovery program reports α_∞ ≈ 3.6 × α_ref across L ∈ {64, 128, 256, 384}." | Rename to `α_largeL` (matches the surrounding calibration discourse, which is the new framing). |
| 1.M5 | `docs/theory/10_eft_program/DERIV_EMERGENT_COULOMB_GEOMETRIC.md` | 174 | MEDIUM | Active prose: `"α_∞ ∈ [1.8, 3.6] × α_ref after convention correction, residual"`. | Rename to `α_largeL`. |
| 1.M6 | `docs/theory/10_eft_program/STATUS_CUDA_BUILD.md` | 95, 100 | MEDIUM | Active build/status doc carries "α_∞ = 1.23× α_ref from 1/L extrapolation" and "α_∞ ∈ [1.8, 3.6] × α_ref". | Lines 95 (which describes the retracted CPU measurement) may stay if framed as a historical retraction; line 100 is a live conclusion and should rename. |

### LOW (legitimate historical / changelog / audit-trail references — keep as-is)

These files reference `α_∞` / `α_inf` in the context of *describing the rename itself* or quoting a retracted claim. No change recommended.

| Location | Reason to keep |
|---|---|
| `CHANGELOG.md` line 367, 391 | Headline retraction prose explicitly quotes retired terminology. |
| `META_DOCUMENTATION_MAP.md` line 219 | One-line catalog entry summarising DERIV_DAY2_CAMPAIGN.md headline. (Optional rename for consistency.) |
| `docs/theory/META_INDEX.md` line 335, 339 | Phase-4 / Phase-F catalog entries describing the historical claim. |
| `docs/theory/07_assessment/archive_session_outputs/ENGINE_AUDIT_REFRAME.md` lines 41–56, 225–230 | The audit document that *recommended* the rename — must preserve the old name in the recommendation text. |
| `docs/theory/07_assessment/CHANGELOG_REFRAME.md` lines 119, 193, 195, 199, 201 | The dedicated changelog of the rename — must preserve the old name. |
| `docs/theory/07_assessment/SESSION_WRAPUP_2026_04_19{,_evening,_session3}.md` | Wrap-ups documenting what changed. |
| `docs/theory/07_assessment/PARKING_LOT.md` line 52 | Describes the rename agent's planted TODO. |
| `CLAUDE.md` line 202 | Project-instructions catalog entry referring to the rename event. |

### Engine and Python — clean

- `engine/**` — zero matches for `alpha_inf` / `α_∞` / `\alphaINF`. All five token forms are gone.
- `dissemination/**` — zero matches.
- `scripts/**` — zero `alpha_inf` matches; `scripts/benchmarks/continuum_extrapolate.py` cleanly uses `alpha_largeL`.
- `engine/tests/benchmark_dynamical_sm.cpp` confirmed at lines 176, 179, 217, 300, 303, 306, 310–313: `struct LargeLFit`, `alpha_largeL`, `largeL_extrap` CSV row label all in place.

---

## §2 · `a_phys ≡ ℓ_P` calibration consistency

The new SPEC_FTD declaration (lines 221–238) reads:

- `a_phys ≡ ℓ_P ≈ 1.616 × 10⁻³⁵ m` (one voxel = one Planck length)
- `t_phys = √3 · ℓ_P / c ≈ 9.34 × 10⁻⁴⁴ s` (from `c_lat = 1/√3` paired with `c_phys = 2.998 × 10⁸ m/s`)
- mass anchor `K_B = m_e ≈ 0.511 MeV/c²`, hence `M_unit ≈ 1.783 × 10⁻³⁰ kg`

### OK

| # | Check | Result |
|---|---|---|
| 2.OK1 | Engine `K_B = 0.511` (`engine/include/ftd/ontic/particle_masses.h:32`) matches SPEC's `K_B = m_e ≈ 0.511 MeV/c²`. | ✅ exact |
| 2.OK2 | Python `K_B = 0.511` (`scripts/constants.py:364`) matches. | ✅ exact |
| 2.OK3 | Engine `C_SPEED = 0.57735026918962576451 = 1/√3` (`gauge_couplings.h:168, 179`) matches SPEC's `c_lat = 1/√3`. | ✅ exact |
| 2.OK4 | SPEC `c_phys = 2.998 × 10⁸ m/s` is consistent with the standard-physics literal (CODATA defines exactly `299 792 458 m/s` ≈ `2.998 × 10⁸`). 4-digit rounding only. | ✅ within rounding |
| 2.OK5 | SPEC `t_phys = √3 · ℓ_P / c ≈ 9.34 × 10⁻⁴⁴ s` numerically: √3 · 1.616e-35 / 2.998e8 = 9.336e-44 s. | ✅ matches to 3 sig figs |
| 2.OK6 | Cross-doc consistency: `OPEN_A_PHYS_DERIVATION.md`, `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md`, `LEDGER.md` (FTD-0030, FTD-0035) all converge on the `a_phys ≡ ℓ_P` calibration. | ✅ |
| 2.OK7 | `WHERE_WE_LEFT_OFF.md` lines 99, 103, 135, 140, 142, 177, 300 still describe `a_phys` as an open problem ("Scoping doc pending", "Phase K, not started", "priority is the a_phys scoping doc"). | NEEDS UPDATE — see §2.M1 |
| 2.OK8 | No file outside SPEC_FTD declares an explicit numerical `a_phys =` value that contradicts the Planck-length anchor. (The only competing values appear in `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` lines 100–102 as a *table of failed candidates*, correctly framed as Mechanism-γ counterexamples.) | ✅ |

### MEDIUM

| # | Location | Line(s) | Severity | Finding | Recommended fix |
|---|---|---|---|---|---|
| 2.M1 | `docs/WHERE_WE_LEFT_OFF.md` | 99, 103, 135–142, 177, 300 | MEDIUM | Document still treats `a_phys` as an OPEN scoping problem and lists "Option 2 — Formalize the a_phys question" as a pending priority. After the SPEC_FTD declaration this text is stale: the calibration has been *declared*, not deferred. | Replace the OPEN status with "RESOLVED-BY-CALIBRATION (a_phys ≡ ℓ_P, declared 2026-04-19 in SPEC_FTD.md §LATTICE↔PHYSICAL CALIBRATION)". The Mechanism-γ derivation attempt remains closed-as-derivation; reference `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md`. |

### Calibration-conditional dimensional predictions — spot check

The SPEC mandates that every dimensional prediction be tagged "conditional on `a_phys ≡ ℓ_P` and `K_B = m_e`". Three high-visibility dimensional predictions checked:

| Prediction | Source | Tagged? |
|---|---|---|
| Higgs mass `M_HIGGS = 124.8 GeV` | `scripts/constants.py:376`, README, CLAUDE.md | NOT tagged anywhere outside SPEC. Cosmetic — the calibration was declared today; tagging propagation is a separate retrofit task. |
| Electron mass `m_e = 0.511 MeV` | engine `K_B`, Python `K_B` | NOT tagged. Per SPEC §3 this is the *anchor* for `M_unit`, so it cannot itself be calibration-conditional. ✅ correct as-is. |
| Engine α plateau "3.6× α_ref" | benchmark_dynamical_sm.cpp, README, paper | DERIV_DAY2_CAMPAIGN.md §6b lines 314, 316, 403–404 correctly tag conditionally. ✅ |

No drift; only a propagation backlog. Not flagged in this audit (out of session-4 scope).

---

## §3 · `G_N` toy-vs-physical awareness

The toy-banner in `engine/include/ftd/ontic/gauge_couplings.h:67–96` is explicit, well-formatted, and identifies G_N(lattice) = 0.01 vs ALPHA_G_APPROX ≈ 5.91 × 10⁻³⁹ as TOY vs PHYSICAL.

### OK — sampled call sites

| Location | Line(s) | Awareness check |
|---|---|---|
| `engine/include/ftd/cosmic_engine.h:22` | doc-comment "G_N = 1/(b_3+N_c)^2 = 0.01 (gravity)" | OK — engine-internal use, value matches |
| `engine/include/ftd/atom_engine.h:12` | "No gravity — alpha_G ~ 6e-39 is negligible at atomic scales." | OK — explicitly invokes physical α_G |
| `engine/SPEC_ENGINE.md:391` | "ALPHA_G_APPROX 5.9e-39 — *physical* gravitational coupling. Engine uses G_N = 0.01 instead (see §5 gravity banner)." | OK — explicit toy/physical distinction |
| `engine/tests/campaign_gravity_hierarchy.cpp:11–82` | Header comment names `G_N = 1/(b₃ + N_c)² = 0.01 [DERIVED]` and asserts hierarchy | OK |
| `engine/tests/campaign_coulomb_force_law.cpp:308` | `rb.toggles.gravity = false; // Isolate EM: G_N=0.01 >> α/(4π) contaminates` | OK — actively guards against toy-G_N contamination |
| `engine/src/ontic_audit.cpp:201, 211–220` | runs both `G_N == 0.01` and `alpha_G ≈ 5.906e-39` checks with cross-domain commentary | OK |
| `engine/cuda/kernels_poisson.cu:448` | `FOUR_PI_G_K_B = 4.0 * PI * G_N * K_B` (CUDA Poisson) | TOY by construction (uses lattice G_N); not a physical-gravity claim |
| `engine/README.md:150` | "G_N=0.01" in cosmic-engine description | OK — engine-context only |
| `scripts/constants.py:377` | `G_N = 1.0 / (b_3 + N_c)**2  # 0.01 — gravitational coupling on lattice` | OK — comment correctly tags as "lattice" |

No call site silently treats `G_N = 0.01` as physical. The `OPEN_A_PHYS_DERIVATION.md` analysis explicitly re-confirms (lines 100–102) that under `a_phys ≡ ℓ_P`, lattice `G_N = 0.01` becomes "a separate engine-toy parameter" — consistent with the banner and the Mechanism-γ negative result.

---

## §4 · Master-quadratic root quotation

| Root | Canonical value | Quotation across project |
|---|---|---|
| `x₊` (tree level) | `137.0361714582…` (`scripts/constants.py:319`, exact analytical via `master_quadratic_roots()`) | Quoted as "137.036" in README, CLAUDE, papers — matches to 6 sig figs |
| `x₊` (one-loop corrected) | `137.036000` (`scripts/constants.py:319`) | Quoted as "137.036000" in README:82 ✅ |
| `x₋` | `3.024` (≈ `3.0239639163`) | Quoted consistently as "3.024" or "3.0239639163" across README, META_INDEX, SPEC_NOVEL_PREDICTIONS, SPEC_FTD_REFERENCE, MATH_findings, RECIPES.md ✅ |

### LOW

| # | Location | Line(s) | Severity | Finding | Recommended fix |
|---|---|---|---|---|---|
| 4.L1 | `docs/theory/01_reference/SPEC_FTD_REFERENCE.md` | 933 | LOW | `Color number x₋ = 3.0239639163 | 0.80% error | ✅ VERIFIED` paired with the literal `0.80%` is consistent, but the row tag still reads `✅ VERIFIED`. Per the SPEC_FTD §1 stale-tag notice, "C2: x₋ → N_c = 3" is now [STRONGLY MOTIVATED CONJECTURE], not a "VERIFIED" identification. The numerical value is correct; the epistemic tag is stale. | Re-tag as [STRONGLY MOTIVATED CONJECTURE] consistent with the 2026-04-19 audit. (Out of scope for this session — the SPEC_FTD body itself is queued as Option 4 in WHERE_WE_LEFT_OFF.md.) |

---

## §5 · `ALPHA_INV_NIST` literal drift (CODATA 2018 vs 2022)

Canonical: `scripts/constants.py:325` defines `ALPHA_INV_NIST = 137.035999177` (CODATA 2022). The `Experimental.alpha_inv` class attribute (line 401) is also `137.035999177`.

### Sites still using CODATA 2018 (`137.035999084`)

| Location | Line(s) | Severity |
|---|---|---|
| `scripts/exploration/derive_numbers.py` | 74, 75 | LOW |
| `scripts/exploration/derive_deeper.py` | 358 | LOW |
| `scripts/exploration/test_all_physics.py` | 181, 192 | LOW |
| `scripts/tests/test_verify_manifest_builder.py` | 28, 33, 69, 89 | LOW |
| `scripts/benchmarks/benchmark_engine_vs_theory.py` | 41 | LOW |
| `scripts/benchmarks/analyze_convergence.py` | 16 (the FALLBACK branch only — primary path imports from `constants`) | LOW |
| `docs/theory/09_mathematical/EXPLR_LVALUE_SPAN_CORRECTION_SEARCH.md` | 14, 34, 67 | LOW (paper explicitly cites "CODATA 2018"; correct in context) |
| `docs/theory/09_mathematical/DERIV_MASTER_QUADRATIC_CM_LVALUES.md` | 259 | LOW (explicit "CODATA 2018" tag) |
| `docs/theory/03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` | 47, 126 | LOW |
| `docs/theory/02_foundations/FOUND_BLIND_DERIVATION_CHAIN.md` | 145, 149 | LOW (says "NIST 2018 CODATA value" — internally consistent) |
| `docs/theory/07_assessment/LEDGER.md` | 140 | MEDIUM — ledger entry should match the project's canonical `137.035999177` (CODATA 2022) since the rest of the project moved |

### MEDIUM

| # | Location | Line(s) | Severity | Finding | Recommended fix |
|---|---|---|---|---|---|
| 5.M1 | `docs/theory/07_assessment/LEDGER.md` | 140 | MEDIUM | Ledger statement: "x₊ = 137.036 is identified with 1/α (CODATA 137.035999084)" uses CODATA 2018 while `scripts/constants.py:325` and the `Experimental` class use CODATA 2022 (`137.035999177`). The 9.3 × 10⁻¹⁰ difference is below all current FTD precision claims, but the LEDGER is the authoritative claim-tracker and should match the canonical literal. | Update to `137.035999177` (CODATA 2022) and append "(CODATA 2022)". |

The remaining LOW-severity 2018-literal sites are either (a) explicitly tagged as "CODATA 2018" in their text (legitimate historical reference), or (b) in scripts whose precision tolerance is far looser than the 9.3 × 10⁻¹⁰ delta. Not urgent; suggest a one-pass `137.035999084` → `137.035999177` retrofit as a separate cleanup task.

---

## §6 · Other spot-check OKs

- `VARPI` quoted as `2.6220575…` everywhere it appears (CHANGELOG, META_INDEX, MONOGRAPH, PAPER_GAUGE_COUPLINGS, MATH_MASTER_QUADRATIC, SPEC_FTD_COMPLETE_CHAIN). Single value, no drift. ✅
- `G_STAR` engine-side: `engine/include/ftd/ontic/lemniscate.h` (umbrella include) — quoted `2.9586751…` in CLAUDE.md, README, and Python `G_STAR ≈ 2.9586751`. ✅
- `b_3 = 7`, `N_c = 3`, `N_base = 4`, `N_eff = 13` integers — exact match across `scripts/constants.py`, `engine/include/ftd/ontic/master_quadratic.h` (loaded via umbrella), engine `audit_ontic_phase0.py`, `print_ontic.py`. ✅
- `scripts/proofs/common.py` — independently re-derives `G_STAR = 2.0 * sqrt(VARPI * GAUSS_M)`, `X_PLUS`, `X_MINUS`, `N_C = 3`, `N_BASE = 4`, `B_3 = 7`, `N_EFF = 13`, `D_CONSTRAINT = 47`, `ALPHA = 1/X_PLUS`. Self-contained-by-design (header comment at line 4–6). Per audit charter not flagged. ✅

---

## Recommended action order

1. **Fix 1.H1** — README.md is the project's front door; the `α_∞` framing there contradicts the engine source it links to. (~5 min edit.)
2. **Fix 1.M1 – 1.M5** — five active `docs/theory/10_eft_program/` documents still using the retired notation in body prose. (~15 min batch edit.)
3. **Fix 1.M6** — STATUS_CUDA_BUILD.md line 100 (live conclusion, not retraction).
4. **Fix 2.M1** — `WHERE_WE_LEFT_OFF.md` should reflect that `a_phys` is now declared, not pending.
5. **Fix 5.M1** — LEDGER.md should match canonical CODATA 2022 literal.
6. **Defer 4.L1** — already queued under SPEC_FTD body rewrite (Option 4 in WHERE_WE_LEFT_OFF.md §3).
7. **Defer §5 LOW retrofits** — bulk `137.035999084` → `137.035999177` sweep, low priority.
8. **Defer dimensional-prediction calibration tagging** (Higgs mass, etc.) — separate retrofit task; not session-4 scope.

---

## Files explicitly verified clean

- `engine/**` — no `alpha_inf` residue
- `dissemination/**` — no `alpha_inf` residue
- `scripts/**` — no `alpha_inf` residue; `continuum_extrapolate.py` and `analyze_convergence.py` use `alpha_largeL` correctly
- `engine/tests/benchmark_dynamical_sm.cpp` — `struct LargeLFit` + `alpha_largeL` + `largeL_extrap` CSV labels confirmed
- `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex` — no residual `\alphaINF` macro hits
- All four canonical files (`scripts/constants.py`, `engine/include/ftd/ontic/{lemniscate,master_quadratic,gauge_couplings,particle_masses}.h`) numerically self-consistent
- `scripts/proofs/common.py` independently re-derives every framework integer + `G_STAR` + master-quadratic roots — values match `scripts/constants.py` (self-containment respected per audit charter)
