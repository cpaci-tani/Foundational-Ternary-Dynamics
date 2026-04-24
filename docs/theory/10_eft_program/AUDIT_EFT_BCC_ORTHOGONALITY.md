# EFT Campaign — BCC-Orthogonality Audit

**Date:** 2026-04-20 (same day as Link 8 closure).
**Scope:** every Day-1/Day-2 EFT campaign claim that involves an engine-measured coupling.
**Question:** does the Link 8 finding — that the engine's 18-point coupling stencil is (SC+FCC)/2 and structurally orthogonal to the BCC sub-stencil — invalidate or require caveating any existing EFT-campaign conclusions?
**Verdict:** **NO** — the existing EFT documentation is already epistemically honest in every place we checked. The caveat belongs on *future* claims, specifically Phase H (spec'd but not measured) and any statement of the form "FTD-engine-derives α_QED".

---

## Context

Link 8 closure (FTD-0050, `AUDIT_LINK8_CLOSURE.md`) established analytically:

- The engine's 18-point coupling stencil σ_18(k) = ½(σ_SC(k) + σ_FCC(k)) has **zero weight** on the 8 corners of the Moore neighbourhood.
- The master quadratic's coefficient 16G*² is tied exactly to the BCC (corners-only) Watson integral: 16·2π·W_BCC = 16G*² = 140.06.
- Any engine-measured observable flowing through the coupling stencil is computed on an operator that is *orthogonal* to the BCC sub-structure.

User flagged a concern (2026-04-20): **do any existing EFT-campaign claims depend on engine observables being read as physical couplings that should match QED** — in which case the BCC-orthogonality would be a hidden structural gap in the interpretation?

---

## Methodology

Systematic read of the EFT campaign's load-bearing claim documents:
- `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex` (the published headline paper)
- `docs/theory/10_eft_program/DERIV_BETA_FUNCTION_MEASURED.md` (β-function extraction)
- `docs/theory/10_eft_program/DERIV_DAY2_CAMPAIGN.md` (Day-2 Rutherford + EWSB + Ward)
- `docs/theory/10_eft_program/DERIV_EMERGENT_COULOMB_GEOMETRIC.md` (Phase G analytical result)
- `docs/theory/10_eft_program/AUDIT_ALPHA_EXTRACTION.md` (Phase F audit)
- `docs/theory/10_eft_program/DERIV_DYNAMICAL_SM_EMERGENCE.md` (dynamical SM tests)

Checked each for: claims that engine observables are physical couplings; claims of convergence to α_QED / β_QED; framing of the 3.6× plateau; Phase H status.

---

## Findings

### α_∞ plateau at 3.6× α_ref — FRAMING IS HONEST

**Source:** `PAPER_FTD_AS_WILSONIAN_EFT.tex` lines 115–126.

Verbatim from the abstract:

> The measured α_r(r, L) is **not a coupling constant at all**: the engine's emergent-mode Gauss law is ∇·J = s with no coupling constant, so α_r(r, L) = 2r G_L(r) is a parameter-free lattice-Green's-function prediction whose zero-free-parameter match to measurement is R² = 1.0000 (median 0.07% relative error) across 16 points at L=384 in the Coulomb-tail regime. The "plateau at 3.6× α_ref" is the value of the periodic lattice Poisson kernel at r/L ≈ 0.31 on a cubic torus — pure geometry, no fine-structure content. This code path carries no α.

**Assessment:** this IS the BCC-orthogonality-aware framing. The paper explicitly identifies the 3.6× "plateau" as a geometric property of the cubic lattice Poisson kernel, NOT as a physical α comparison. There is no claim here that engine-α converges to QED-α; instead there's an explicit disclaimer that "this code path carries no α". ✅ No caveat needed.

### β-function measurement — FRAMING IS HONEST

**Source:** `DERIV_BETA_FUNCTION_MEASURED.md` lines 8–20 and line 30.

Headline:

> The FTD lattice engine produces a *screened* two-charge potential, not a pure Coulomb potential... the measured coupling α_eff shows scale dependence (evidence of RG flow), but neither its magnitude nor its running matches QED one-loop to the quantitative threshold pre-registered in SPEC_EFT_RECOVERY_PROGRAM.md §5. Phase 2 therefore ends with a **qualitative match** (negative β, consistent with screening/asymptotic freedom as continuum QED) and a **quantitative gap** of two to three orders of magnitude that is attributed to finite-size effects and the Yukawa-like screening envelope present at the lattice scale.

Pre-registration / measurement table:

| Pre-reg | Measured | Verdict |
|---|---|---|
| α_eff(L=64) = 1/137.036 ± 15% | 0.120 (16× α_ref); asymptotic 0.033 (4.5× α_ref) | **✗ magnitude off** |
| β(g) matches one of QED/QCD/trivial/new | β_measured / β_QED ≈ −160 | **⚠ qualitative only** |

**Assessment:** the β-function result is flagged EXPLICITLY as a quantitative mismatch of 2–3 orders of magnitude. "This document reports the measurements against the pre-registration without retrofitting either to match the other." ✅ No caveat needed.

### Day-2 Rutherford α-extraction — CONSISTENT WITH ABOVE

**Source:** `DERIV_DAY2_CAMPAIGN.md` line 217, 254.

> **Conclusion:** the ~5× gap between measured α and α_ref is [...] a genuine engine physics result, not a methodology artefact.

Rutherford α = 0.042 ± 0.005 is declared 5.8× α_ref and [MEASURED]-tagged.

**Assessment:** The 5× gap is explicitly named as an engine-physics result, not claimed to converge to α_QED. Phase G analytical result shows α_r is lattice-kernel geometry. ✅ No caveat needed.

### Phase H (EXPLICIT α COUPLING) — WOULD NEED THE CAVEAT

**Source:** `PAPER_FTD_AS_WILSONIAN_EFT.tex` lines 124–126:

> A parametric coupling added to Gauss's law (Phase H, spec'd but not yet measured) would be the appropriate test of whether FTD's dynamics can emergently reproduce α_ref.

`LEDGER.md` FTD-0011 tags Phase H as [THEOREM]: "g_c² scales α_r". The theorem is purely about how the coupling rescales the measured α_r, not about α_r converging to α_QED.

**Assessment:** Phase H is currently (as of the audit date) a *measurement spec*, not a *derivation claim*. If Phase H is ever run and any of its outcomes is phrased as "engine-dynamics derive α_QED" or equivalent, **that claim would need the BCC-orthogonality caveat**: the engine's coupling operator is (SC+FCC)/2, which does not access the BCC sub-stencil where the master quadratic's 16G*² coefficient lives. Phase 1 (`link8_phase1_flow_matrix.py`) further established analytically that even a 2-coupling (g_SCFCC, g_BCC) engine extension does NOT produce the master quadratic as its characteristic polynomial under block-spin RG. So the right prior on Phase H is: "convergence to α_QED is structurally unlikely; any Phase H result that reports it should be scrutinized carefully."

**Recommendation:** add an inline note to `PAPER_FTD_AS_WILSONIAN_EFT.tex` §7 (Follow-up) or wherever Phase H is next discussed, cross-referencing Link 8 and this audit. Current framing doesn't claim convergence; keep it that way.

### EWSB, Ward identities, condensate mass — NO BCC OVERLAP

EWSB amplitude threshold, Ward identity floor, condensate spectroscopy (Day-2) all measure *field-theory-native* quantities (phase transition, divergence residuals, mass gaps). None is identified with a physical coupling α or with a QED observable. The (SC+FCC)/2 structure of the engine's operator is the thing being measured; the BCC sub-operator is not claimed as part of the measurement target. ✅ No caveat needed.

### SU(3) / color charge claims — DEPEND ON CONSISTENT FRAMING

`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` Part II identifies SU(3) with the BCC triple-cosine eigenvalue. But this derivation lives at the **counting / structural** layer (argument from symmetry and representation theory), NOT at the **engine-dynamics** layer. The engine's `color_forces` toggle implements a phenomenological SU(3)-inspired pairwise force; it does not claim to derive SU(3) dynamics from the coupling stencil. No EFT campaign document claims engine-measured αs matches the physical QCD coupling.

**Assessment:** Consistent with scope discipline (LEDGER FTD-0029 tags the BCC multiplicative structure as SELECTION, not THEOREM). ✅ No caveat needed on current framing.

---

## Bottom line

| Item | Status | Needs caveat? |
|---|---|---|
| α_∞ plateau at 3.6× α_ref | Explicitly labeled as "pure lattice geometry, no fine-structure content" in abstract | **No** |
| β-function measurement vs QED one-loop | Explicitly labeled as 2–3 orders of magnitude quantitative gap | **No** |
| Rutherford α = 5× α_ref | Labeled as "genuine engine physics, not methodology artefact"; no claim of QED convergence | **No** |
| Phase H (explicit α coupling) — spec'd | Current doc only states the *measurement* not a *derivation* | **Not yet; caveat needed if/when any Phase H result frames convergence to α_QED** |
| EWSB, Ward, condensate | Field-theory quantities not identified with physical couplings | **No** |
| SU(3) / BCC / color | Lives at structural layer, not engine-dynamics layer; not claimed from coupling stencil | **No** |
| Any future claim of the form "engine dynamics derive α_QED or α_s from first principles" | | **Yes — would collide with Link 8 closure** |

The EFT campaign's published documentation **does not currently need caveating**. The existing framing is honest about the (SC+FCC)/2 structure of what's being measured and about where convergence to QED fails.

The caveat belongs on **future** claims, specifically: if anyone proposes to publish a claim that FTD's engine-native dynamics produce α_QED or α_s or a β-function matching Standard-Model running, that claim is structurally ruled out by the BCC-orthogonality finding + Phase 1 analytical gate. The right framing remains "engine-measured quantities are parameter-free predictions of the lattice geometry; their relation to physical couplings requires additional structural input not currently in the engine".

---

## Action items

1. **No retractions needed.** No existing claim is overreach in light of the Link 8 findings.
2. **LEDGER cross-reference.** Add a `reviewer_note` pointer from FTD-0011 (Phase H coupling scaling theorem) and FTD-0045 (α_largeL ≈ 3.6× α_ref) to `AUDIT_LINK8_CLOSURE.md` so that any future editor working on those rows reads the BCC-orthogonality finding before touching the framing.
3. **Wilsonian paper §7 note.** If the paper is revised in a future pass, a one-paragraph note in §7 (Phase-H follow-up discussion) pointing to Link 8's analytical closure would help reviewers understand that structural α_QED convergence is not the expected target.
4. **Future reviewer discipline.** Any Phase H result presented as "FTD emergently reproduces α_ref" should be sent back through this audit before being accepted into papers or manuscripts.

---

## Artifact list

- This audit: `docs/theory/10_eft_program/AUDIT_EFT_BCC_ORTHOGONALITY.md`.
- Link 8 closure: `docs/theory/10_eft_program/AUDIT_LINK8_CLOSURE.md`.
- Phase 1 analytical gate: `scripts/exploration/link8_phase1_flow_matrix.py`.
- BCC structural derivation: `docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`.
- EFT publication: `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex`.
