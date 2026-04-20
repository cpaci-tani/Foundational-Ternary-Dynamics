# Session 2 Wrapup — Reframe Deployment, 2026-04-19 (evening, owner-approved decisions)

**Read this in addition to** `SESSION_WRAPUP_2026_04_19.md` (Session 1).
**Status:** Phase 0 → 6 fully complete for `docs/theory/`; Phase 1 → 4 partially complete for `docs/papers/`. Substantive owner-judgment items queued for Session 3.

---

## What you authorized + what landed

| Decision | Status | Output |
|---|---|---|
| 1. `α_inf` rename across engine + Python + EFT paper | ✅ DONE | 23 edits across 4 files (`benchmark_dynamical_sm.cpp`, `analyze_convergence.py`, `PAPER_FTD_AS_WILSONIAN_EFT.tex`, bonus `continuum_extrapolate.py`); 1/L² fit math unchanged; TODO planted for empirical residual band emission |
| 2. EFT FLAGs (5 items) — logical choice per item | ✅ DONE | Restatement A on Flag 1 (β-function), Restatement B on Flags 2-4 (calibration-conditional under `a_phys`), Path A retraction on Flag 5 (CM-conjecture). All 5 markers replaced; `TRACKER_REFRAME_FLAGS.md` updated |
| 3. Devil's-advocate PASS-WITH-NOTES queue | ✅ DONE | Path-integral residual phrases (lines 281, 337) + §5.3 β → 0/∞ + §5.5 `{F_N}` totalisation + SPEC_FTD §14.2 Lorentz-emergence — all rewritten finitarily |
| 4. `a_phys ≡ ℓ_P` declaration | ✅ DONE | Added to `SPEC_FTD.md` between Postulate 2 and Postulate 3; new section "LATTICE ↔ PHYSICAL CALIBRATION" with Planck-length declaration + `K_B = m_e` mass anchor + calibration discipline |
| 5. Paper Phase 4 sweep (top-5 tractable papers) | ✅ DONE | 11 edits across 6 files (PAPER_2A, gauge couplings TeX+MD pair, Softplus-ReLU duality, Discrete-Continuous bridge, Finitude theorem preamble) |
| 6. PDF-only papers source recovery | ✅ ANALYSED | `TRACKER_PDF_ONLY_PAPERS.md` written; 13 papers triaged (2 HIGHEST, 2 HIGH, 3 likely-superseded, 6 unknown). Recovery options enumerated |
| 7. manuscript_v1 ↔ v2 propagation rule | ✅ DONE | `PROPAGATION_RULE.md` written; verified vol1/vol2 are NOT symlinks (already diverged); `src/chapters/` declared canonical for v2 |

**Plus:** YM/NS RE-DERIVE assessment (`REDERIVE_REPORT_YM_NS.md`); Millennium-problems portfolio audit (clean except `docs/papers/README.md`, now updated).

---

## What is now in source (concrete deliverables)

### New files (Session 2)

| File | Role |
|---|---|
| `dissemination/manuscript_v2/PROPAGATION_RULE.md` | Authoritative manuscript propagation rule |
| `docs/theory/07_assessment/archive_session_outputs/TRACKER_PDF_ONLY_PAPERS.md` | 13 PDF-only papers triage |
| `docs/theory/07_assessment/archive_session_outputs/REDERIVE_REPORT_YM_NS.md` | Yang-Mills + Navier-Stokes assessment |
| `docs/theory/07_assessment/SESSION_WRAPUP_2026_04_19_evening.md` | This file |

### Files modified (Session 2)

| File | Change |
|---|---|
| `engine/tests/benchmark_dynamical_sm.cpp` | `alpha_inf` → `alpha_largeL` rename + CSV row label restated |
| `scripts/benchmarks/analyze_convergence.py` | Narrative-string update |
| `scripts/benchmarks/continuum_extrapolate.py` | Variable rename + docstring + interpretation strings |
| `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex` | LaTeX macro rename + 13 call-sites + headline restated |
| `docs/SPEC_FTD.md` | New "LATTICE ↔ PHYSICAL CALIBRATION" section (a_phys ≡ ℓ_P); §14.2 Lorentz-emergence rewritten finitarily |
| `docs/theory/03_derivations/DERIV_PATH_INTEGRAL_CONSTRUCTION.md` | Lines 281, 337 + §5.3 + §5.5 finitary rewrites |
| `docs/theory/10_eft_program/DERIV_BETA_FUNCTION_MEASURED.md` | Flag 1 → Restatement A |
| `docs/theory/10_eft_program/DERIV_DYNAMICAL_SM_EMERGENCE.md` | Flag 2 → Restatement B |
| `docs/theory/10_eft_program/DERIV_DAY2_CAMPAIGN.md` | Flags 3 + 4 → Restatement B (×2) |
| `docs/theory/09_mathematical/CONJ_ALPHA_FROM_CM.md` | Flag 5 → Path A retracted |
| `docs/papers/src/PAPER_2A_MASTER_QUADRATIC.tex` | 2 edits (future-work passages) |
| `docs/papers/PAPER_GAUGE_COUPLINGS_FROM_LATTICE_GEOMETRY.tex` + `.md` | 3 edits each (siblings in sync) |
| `docs/papers/src/DERIV_SOFTPLUS_RELU_DUALITY.tex` | RG fixed-point rewritten as ε-μ statement |
| `docs/papers/src/FTD_Discrete_Continuous_Bridge.tex` | Lattice-spacing-to-zero rewritten finitarily |
| `docs/papers/speculative/FTD_Finitude_Theorem.tex` | Canonical-status preamble added |
| `docs/papers/README.md` | YM/NS/Riemann/Finitude entries updated with post-reframe status |
| `docs/theory/07_assessment/archive_session_outputs/TRACKER_REFRAME_FLAGS.md` | All 5 rows marked RESOLVED |
| `docs/theory/07_assessment/LEDGER.md` | FTD-0030 + FTD-0034 RESOLVED; new rows FTD-0041 → FTD-0045 |
| `docs/theory/07_assessment/CHANGELOG_REFRAME.md` | Session 2 entry appended |
| `docs/theory/META_INDEX.md` | 4 new rows (7.27 → 7.30) |
| `CLAUDE.md` | 5 new key-navigation pointers; calibration declaration noted |

---

## Open work queue for Session 3 (owner judgment + extension)

### Owner-judgment items (decision needed)

1. **YM paper disposition** (`FTD_Yang_Mills_Mass_Gap.tex`) — choose: SPLIT into smaller honest paper around per-voxel mass gap (FTD-0044) / DEMOTE Clay-eligibility framing / RETRACT entirely.
2. **NS paper disposition** (`FTD_Navier_Stokes.tex`) — choose: RETRACT or REFRAME as ontological-position paper. No surviving Clay-eligible content.
3. **Riemann paper** (`FTD_Riemann_Hypothesis.tex`) — classifier flagged for deeper read; needs your scrutiny.

### Source-recovery items (mechanical)

4. **PDF-only TeX recovery** — run `git log --all --diff-filter=D --name-only -- 'docs/papers/*.tex' 'docs/papers/src/*.tex'` to find any deleted source. For sources recovered, run classifier + restatement pipeline.
5. **Manuscript divergence audit** — run the diff-sweep from `PROPAGATION_RULE.md` §"Known divergences" to enumerate all currently-diverged files between `src/chapters/` and `vol1/`/`vol2/`.

### Extension items (more reframe work)

6. **Manuscript v1 + v2 reframe sweep** — apply propagation rule; ~92 + 83 chapters need classifier + restatement. Dispatch parallel agents using the `reframe_deployment/agents/` prompts.
7. **Whitepaper reframe** — `dissemination/whitepaper/FTD_Whitepaper.tex` not yet touched.
8. **Notebooks + interactive HTML reframe** — Phase 1 inventory complete; Phases 2–4 not started.

### Engineering items

9. **`α_largeL` empirical residual band emission** — TODO planted in `engine/tests/benchmark_dynamical_sm.cpp`; needs multi-seed ensemble run + CSV column addition.

---

## Headline metrics (whole deployment, Sessions 1+2)

| | Session 1 | Session 2 | Total |
|---|---:|---:|---:|
| Theory docs touched | 52 | 7 | 59 |
| Mechanical edits | 126 | ~65 | ~191 |
| Substantive rewrites | 5 | 1 (master quadratic finalisation in §4.2) | 6 |
| Same-day blocking fixes | 3 | 0 | 3 |
| Same-day HIGH fixes | 2 | 0 | 2 |
| HIGH deferred to owner | 1 (`α_inf` rename) | 0 (resolved Session 2) | 0 |
| LEDGER rows added | 40 | 5 | 45 |
| LEDGER rows resolved (status change) | n/a | 2 (FTD-0030, FTD-0034) | 2 |
| Broader-portfolio artifacts inventoried | 280 | n/a | 280 |
| TeX papers classified | 34 | n/a | 34 |
| Papers Phase-4 restated | 0 | 5 | 5 |
| EFT FLAGs resolved | 0 | 5 | 5 |
| New deliverable docs | 9 | 4 | 13 |
| Calibrations declared | 0 | 1 (`a_phys ≡ ℓ_P`) | 1 |

**Foundational reframe deployment status:**
- **`docs/theory/`:** Phases 0–7 complete (with same-day fixes for the 3 BLOCKING + 2 HIGH issues found by Phase 6.1 + Phase 5).
- **`docs/papers/`:** Phases 1–4 complete for the 5 tractable papers; Phases 5–7 + YM/NS owner judgment queued.
- **`dissemination/manuscript`, `manuscript_v2`, `whitepaper`, `notebooks`, `interactive`:** Phase 1 inventory complete; Phases 2–7 queued.
- **Engine:** Phases 0–6 complete with 2 same-day HIGH fixes; α_inf rename now done; one TODO (residual band) queued.

**Parameter-free claim status:** the framework now has **one explicit calibration** (`a_phys ≡ ℓ_P`, paired with `K_B = m_e`). Dimensionless predictions remain calibration-independent; dimensional predictions are conditional on the declared calibration. This is the same epistemic position as every effective field theory, made explicit.

---

## Reading order when resuming

1. **`SESSION_WRAPUP_2026_04_19_evening.md`** (this file) — what changed in Session 2.
2. **`SESSION_WRAPUP_2026_04_19.md`** — what changed in Session 1.
3. **`CHANGELOG_REFRAME.md`** — append-only record of every change across both sessions.
4. **`LEDGER.md`** — single source of truth for claim status (45 rows now).
5. **`REDERIVE_REPORT_YM_NS.md`** — your decision on the YM and NS papers.
6. **`TRACKER_PDF_ONLY_PAPERS.md`** — 13 PDF-only papers needing source recovery or archive.
7. **`PROPAGATION_RULE.md`** — read before any manuscript chapter edit.

For agent-driven Session 3 work:
- **`reframe_deployment/CANONICAL_REFRAME.md`** — every agent reads this first.
- **`reframe_deployment/agents/`** — 9 stateless agent prompts; use these directly when dispatching.
- **`reframe_deployment/checklists/per_paper.md`** — per-artifact tracking checklist for the manuscript / whitepaper sweeps.
