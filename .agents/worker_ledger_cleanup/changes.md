# FTD Ledger Cleanup and Reconciliation Campaign Changes

This document lists the exact modifications made during this session to reconcile the Foundational Ternary Dynamics (FTD) ledger, index files, and campaign/audit/pre-reg files.

## 1. LEDGER.md Updates
**File:** `docs/theory/07_assessment/LEDGER.md`
- Assigned `FTD-0232` to MC-T4.3 alpha-readout independent audit (previously duplicate `FTD-0224`).
- Maintained `FTD-0224` exclusively for "Color Excess closed form & Blocked Effective Action Flow".
- Added five new rows for the late-May alpha-readout campaign:
  - `FTD-0230`: BCC Complex Readout (`FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md`).
  - `FTD-0231`: Candidate C Quantization/Readout (`FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md`).
  - `FTD-0233`: Determinant Grading (`AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md`).
  - `FTD-0234`: Odd Period via J-twisted det_ζ (`AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md`).
  - `FTD-0235`: det↔det_ζ operator identity (`AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md`).
- Renumbered Ginsparg-Wilson / Chiral Anomaly from `FTD-0230` to `FTD-0236` to make space.

## 2. Systematic Renumbering in Campaign, Audit, and Pre-Reg Files
To ensure zero collisions and exact synchronization with the canonical ledger, the following 12 files were systematically renumbered:
- **`FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md`**: Renumbered from provisional `FTD-0215` to canonical `FTD-0230`. Synchronized all internal reference IDs.
- **`PREREG_ALPHA_READOUT_BCC_BRIDGE_v1.md`**: Synchronized reference IDs to canonical `FTD-0230`.
- **`FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md`**: Renumbered from provisional `FTD-0216` to canonical `FTD-0231`. Synchronized all internal reference IDs.
- **`PREREG_ALPHA_READOUT_QUANTIZATION_v1.md`**: Synchronized reference IDs to canonical `FTD-0231`.
- **`AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md`**: Synchronized reference IDs to canonical `FTD-0231`.
- **`AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md`**: Renumbered from duplicate `FTD-0224` to canonical `FTD-0232` (MC-T4.3 independent review). Corrected all internal references.
- **`AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md`**: Renumbered from provisional `FTD-0217` to canonical `FTD-0233`. Synchronized all internal references.
- **`PREREG_ALPHA_READOUT_DETERMINANT_GRADING_v1.md`**: Synchronized reference IDs to canonical `FTD-0233`.
- **`AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md`**: Renumbered from provisional `FTD-0218` to canonical `FTD-0234`. Synchronized all internal references.
- **`PREREG_ALPHA_READOUT_ODD_PERIOD_v1.md`**: Synchronized reference IDs to canonical `FTD-0234`.
- **`AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md`**: Renumbered from provisional `FTD-0219` to canonical `FTD-0235`. Synchronized all internal references.
- **`PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md`**: Synchronized reference IDs to canonical `FTD-0235`.

## 3. Downstream Document Updates
To ensure all index files are perfectly aligned:
- **`docs/theory/10_eft_program/INDEX_FTD_NATIVE_EFT.md`**:
  - Updated renumbered IDs for existing entries (`FTD-0215` → `FTD-0230`, `FTD-0216` → `FTD-0231`).
  - Added new pre-registration entries (`PREREG_ALPHA_READOUT_DETERMINANT_GRADING_v1.md`, `PREREG_ALPHA_READOUT_ODD_PERIOD_v1.md`, `PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md`).
  - Added new audit entries (`AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md`, `AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md`, `AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md`, `AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md`).
  - Updated live document count from `44` to `51` (excl. index).
- **`docs/theory/META_INDEX.md`**:
  - Reconciled existing entry references under section 10 with new IDs.
  - Appended new index entries for `10.68` through `10.74` representing the newly added audits and pre-registrations.
- **`docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md`**:
  - Updated Chiral Anomaly closure row identifier to `FTD-0236` (instead of old `FTD-0230`).

## 4. Verification and Node Map Rebuilding
- Ran `python scripts/verification/build_math_node_map.py` to regenerate the full mathematical node map `scripts/verification/results/math_node_map.json`.
- The parser processed all 215 ledger rows, 13 spine theorems, and 930 code-witessed mathematical identities, verifying 100% syntactic structure and link integrity.
- Rebuilding completed successfully with exit code 0.
