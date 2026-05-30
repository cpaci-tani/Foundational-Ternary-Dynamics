## 2026-05-29T21:28:38-05:00

You are teamwork_preview_worker.
Your working directory is: c:\Users\cpaci\Desktop\ftd\.agents\worker_ledger_cleanup

MANDATORY INTEGRITY WARNING:
DO NOT CHEAT. All implementations must be genuine. DO NOT hardcode test results, create dummy/facade implementations, or circumvent the intended task. A Forensic Auditor will independently verify your work. Integrity violations WILL be detected and your work WILL be rejected.

Your task is to implement the cleanup and reconciliation of the Foundational Ternary Dynamics (FTD) ledger and downstream files. You must perform the edits with absolute precision.

Here is the step-by-step implementation instruction:

1. **Modify `docs/theory/07_assessment/LEDGER.md`**:
   - Keep `FTD-0224` exclusively for "Color Excess closed form & Blocked Effective Action Flow" (Line 222).
   - Locate the row for "MC-T4.3 alpha-readout FOUND audit + correction (2026-05-28 session)" (Line 235) and change its ID from `FTD-0224` to `FTD-0232`.
   - Append five new, separate canonical rows at the end of `LEDGER.md` for the late-May 2026 documents with their final honest statuses exactly matched.
     - **BCC Algebraic Bridge Readout (ARC-B2)** (incorporating `FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md` / `PREREG_ALPHA_READOUT_BCC_BRIDGE_v1.md`) -> Assign unique canonical ID `FTD-0230`. Status: `[UNDERDETERMINED]`.
     - **Alpha Quantization Readout (ARC-C1)** (incorporating `FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md` / `PREREG_ALPHA_READOUT_QUANTIZATION_v1.md` / `AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md`) -> Assign unique canonical ID `FTD-0231`. Status: `[UNDERDETERMINED]`.
     - **Determinant Grading Pre-Reg & Audit** (`PREREG_ALPHA_READOUT_DETERMINANT_GRADING_v1.md` / `AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md`) -> Assign unique canonical ID `FTD-0233`. Status: `[CLOSED NEGATIVE — scoped]`.
     - **Odd Period Pre-Reg & Audit** (`PREREG_ALPHA_READOUT_ODD_PERIOD_v1.md` / `AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md`) -> Assign unique canonical ID `FTD-0234`. Status: `[UNDERDETERMINED]`.
     - **Det Identity Pre-Reg & Audit** (`PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md` / `AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md`) -> Assign unique canonical ID `FTD-0235`. Status: `[UNDERDETERMINED]`.
     Make sure these rows conform exactly to the Markdown table format in `LEDGER.md`. Make their descriptions detailed, noting the file names and hashes/relationships if appropriate.

2. **Renumber Internal Headers and Text References in Campaign/Audit/Pre-Reg Files**:
   - Locate and edit each of the following files, substituting old/colliding/provisional IDs for their new canonical ones in the headers, frontmatter, links, and body text:
     - `docs/theory/10_eft_program/FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md` -> renumber internally to `FTD-0230`
     - `docs/theory/10_eft_program/PREREG_ALPHA_READOUT_BCC_BRIDGE_v1.md` -> renumber internally to `FTD-0230`
     - `docs/theory/10_eft_program/FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md` -> renumber internally to `FTD-0231`
     - `docs/theory/10_eft_program/PREREG_ALPHA_READOUT_QUANTIZATION_v1.md` -> renumber internally to `FTD-0231`
     - `docs/theory/10_eft_program/AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md` -> renumber internally from colliding `FTD-0211` to `FTD-0231` (as it is associated with the new `FTD-0231` row)
     - `docs/theory/10_eft_program/AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md` -> renumber internally from duplicate `FTD-0224` to `FTD-0232` (along with any references in that file)
     - `docs/theory/10_eft_program/AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md` -> renumber internally to `FTD-0233`
     - `docs/theory/10_eft_program/PREREG_ALPHA_READOUT_DETERMINANT_GRADING_v1.md` -> renumber internally to `FTD-0233`
     - `docs/theory/10_eft_program/AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md` -> renumber internally to `FTD-0234`
     - `docs/theory/10_eft_program/PREREG_ALPHA_READOUT_ODD_PERIOD_v1.md` -> renumber internally to `FTD-0234`
     - `docs/theory/10_eft_program/AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md` -> renumber internally to `FTD-0235`
     - `docs/theory/10_eft_program/PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md` -> renumber internally to `FTD-0235`

3. **Update Downstream Indexes and Tracker**:
   - `docs/theory/META_INDEX.md`
     - Update references for `FTD-0230` to point to the two BCC Algebraic Bridge documents.
     - Update references for `FTD-0231` to point to the two Alpha Quantization Readout documents (and the Charge Quantization audit document if needed).
     - Add entries for `FTD-0232` (pointing to `AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md`), `FTD-0233`, `FTD-0234`, `FTD-0235` pointing to their respective pre-reg and audit documents.
     - Resolve the collisons for `FTD-0211` (only W5 Moore-shell should remain here), `FTD-0217` (only Color Confinement should remain here), `FTD-0218` (only Stochastic Effective Action should remain here), `FTD-0219` (only Pythagorean-Fermat Bridge should remain here).
   - `docs/theory/10_eft_program/INDEX_FTD_NATIVE_EFT.md`
     - Synchronize the IDs and paths to reflect the correct assignments.
     - Add `FTD-0232` (MC-T4.3 independent review), `FTD-0233`, `FTD-0234`, `FTD-0235` to this index with their correct paths.
     - Resolve any colliding mappings.
   - `docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md`
     - Synchronize IDs for these entries, ensuring the correct statuses are represented.

4. **Verify and Rebuild the Math Node Map**:
   - Run the builder script:
     ```powershell
     .venv\Scripts\python.exe scripts/verification/build_math_node_map.py
     ```
   - Verify that it completes successfully without any Python traceback or validation errors.
   - Verify that the resulting `scripts/verification/results/math_node_map.json` and interactive HTML files (e.g. `dissemination/interactive/math_node_map.html`) are regenerated, and carry the new canonical IDs (`FTD-0230` through `FTD-0235`).

Write a comprehensive summary of all your edits and the outputs of the `build_math_node_map.py` execution in `c:\Users\cpaci\Desktop\ftd\.agents\worker_ledger_cleanup\changes.md` and a formal handoff.md in your working directory. Then, send a message back to the orchestrator when you are finished!
