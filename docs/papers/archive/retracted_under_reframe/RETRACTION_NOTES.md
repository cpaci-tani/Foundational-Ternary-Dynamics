# Retracted Under the Undefined-Boundary Reframe

**Status:** four papers retracted on 2026-04-19 because their load-bearing arguments rest on completed-infinity reasoning that the framework no longer admits. See `docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md` and `reframe_deployment/CANONICAL_REFRAME.md`.

This directory preserves the retracted artifacts for evidentiary record. **None of these papers should be cited as part of the FTD portfolio.** Where a paper's central claim survives in a finitary form, that surviving claim is recorded in `docs/theory/07_assessment/core_ledgers/LEDGER.md`.

---

## 1. `FTD_Yang_Mills_Mass_Gap.tex` + `.pdf`

**Original claim:** A proof of the Yang-Mills mass gap (Clay Millennium problem) for FTD's lattice formulation.

**Why retracted:**

- Foundational Axiom 1 (`Λ = ℤ³`) is exactly the proscribed completed-infinity move under the undefined-boundary reframe.
- UV-finiteness proof passes through "compact Brillouin zone" — requires ℤ³ totality.
- Proposition 5.2 ("mass gap persists in thermodynamic limit") is a load-bearing completed-infinity claim, needed to match Clay's problem statement.
- Without these steps, the paper does not address the Clay problem.

**What survives:** Theorem 5.1 (per-voxel mass gap from manifestation threshold K_B). This is a genuinely local criterion holding at every site of the cubic graph, independent of the lattice's global extent. It does not constitute a Clay-eligible result, but it is a real theorem about FTD.

- **LEDGER row:** FTD-0044 — "Per-voxel mass gap (THEOREM, survives reframe)."
- **Future work:** the surviving theorem could anchor a smaller, honest paper — focused on the per-voxel statement, with no Clay-eligibility framing. Owner discretion.

**Source assessment:** [`REDERIVE_REPORT_YM_NS.md`](../../../theory/07_assessment/archive_session_outputs/REDERIVE_REPORT_YM_NS.md).

---

## 2. `FTD_Navier_Stokes.tex` + `.pdf`

**Original claim:** A proof of Navier-Stokes regularity (Clay Millennium problem) via FTD's lattice-to-continuum bridge.

**Why retracted:**

- Theorem 4.2 (uniform energy bound) requires periodic-boundary integration-by-parts on a totalised domain — completed-infinity-adjacent.
- "No finite-time blow-up" depends on the global energy bound and on a continuum-limit comparison.
- Theorem 6.1 (continuum recovery of Navier-Stokes) is an explicit `a → 0` completed limit — proscribed.
- Theorem 6.2 (regularity ladder) appeals to Sobolev embedding on ℝ³ — a theorem about a completed continuum, not about FTD's actual finite regions.

**What survives:** **No standalone per-voxel theorem.** Unlike the Yang-Mills paper, no surviving Clay-eligible content was found. The paper's central technical work cannot be restated finitarily.

- **LEDGER row:** FTD-0043 — "Navier-Stokes regularity proof — RETRACTED 2026-04-19."
- **Recommended replacement (if any):** a one-paragraph philosophical-position paper articulating why classical Navier-Stokes regularity proofs do not transfer to undefined-boundary lattice substrates. Not currently planned.

**Source assessment:** [`REDERIVE_REPORT_YM_NS.md`](../../../theory/07_assessment/archive_session_outputs/REDERIVE_REPORT_YM_NS.md).

---

## 3. `FTD_Thermodynamic_Limit.pdf` (PDF-only — no recoverable TeX source)

**Original claim:** Title-level evidence ("Thermodynamic Limit") that the paper's premise is the completed-infinity construct that the reframe explicitly proscribes. Full text extracted via `pdftotext` and stored as `FTD_Thermodynamic_Limit_extracted.txt` for evidentiary record.

**Why retracted:**

- The thermodynamic limit (`N → ∞` as a completed object) is one of the named proscribed moves in `CANONICAL_REFRAME.md`.
- The paper's title and central concept are the proscribed move itself; restatement would require gutting the paper's premise.
- No TeX source was recoverable from git history (only figures were committed; the paper itself was a PDF artifact).

**What survives:** unknown without TeX source. The PDF-extracted text is preserved for any future re-authoring, but no reframe-compatible restatement is currently planned.

---

## 4. `DERIV_THERMODYNAMIC_REFLEXION.pdf` (PDF-only — no recoverable TeX source)

**Original claim:** Same title-level evidence as #3. Full text extracted via `pdftotext` and stored as `DERIV_THERMODYNAMIC_REFLEXION_extracted.txt`.

**Why retracted:** Same reasoning as #3.

**What survives:** unknown.

---

## Files in this directory

```
DERIV_THERMODYNAMIC_REFLEXION.pdf         — retracted PDF (PDF-only)
DERIV_THERMODYNAMIC_REFLEXION_extracted.txt — pdftotext extraction for evidentiary record
FTD_Navier_Stokes.tex                     — retracted TeX source (NS)
FTD_Navier_Stokes.pdf                     — retracted PDF (NS)
FTD_Thermodynamic_Limit.pdf               — retracted PDF (PDF-only)
FTD_Thermodynamic_Limit_extracted.txt     — pdftotext extraction
FTD_Yang_Mills_Mass_Gap.tex               — retracted TeX source (YM)
FTD_Yang_Mills_Mass_Gap.pdf               — retracted PDF (YM)
RETRACTION_NOTES.md                       — this file
```

---

## Maintenance rule

This directory is **read-only** going forward. If a retracted paper is ever resurrected in a reframe-compatible form, the new artifact lives in `docs/papers/` (not here), and a new LEDGER row is created with explicit dependency on the surviving / restated content. The retracted artifact stays here unchanged.

If new evidence indicates a previously-retracted paper *should* be returned to active status (i.e., the reframe was wrong about it), the change must be processed through:
1. A formal proposal in `docs/theory/07_assessment/CHANGELOG_REFRAME.md`.
2. Re-classification under `CANONICAL_REFRAME.md` Q1–Q4.
3. Owner sign-off.
4. New LEDGER row + reverse-link from retracted artifact.

Do not silently undo retractions.
