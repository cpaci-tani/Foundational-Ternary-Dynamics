# Handoff Report — explorer_ledger_cleanup

This handoff report is prepared in accordance with the 5-Component Handoff Protocol to detail the read-only investigation into the Foundational Ternary Dynamics (FTD) ledger and document numbering tangle.

---

## 1. Observation

### 1.1 Ledger Occurrences of `FTD-0224`
In `docs/theory/07_assessment/LEDGER.md`, there are two separate rows using the `FTD-0224` identifier:
*   **Line 222**:
    ```markdown
    | FTD-0224 | Color Excess closed form & Blocked Effective Action Flow | **[CLOSED RESOLVED]** — Explores the 100-digit precision arithmetic of the color excess $\delta_c$ and bounds its algebraic transcendence over $\mathbb{Q}(G^*, \pi, \alpha)$ using PSLQ; rigorously maps the Onsager-Machlup history flow and the multi-scale blocking equations to the standard physical Maxwell action $F_{\mu\nu}F^{\mu\nu}$ as a stable IR fixed point. | NEW 2026-05-27 — `docs/theory/09_mathematical/EXPLR_COLOR_EXCESS_CLOSED_FORM.md` and `docs/theory/10_eft_program/DERIV_FTD_NATIVE_NONLINEAR_FLOW.md`. High-precision explore runner script `scripts/exploration/explore_color_excess.py` confirms transcendence relations. |
    ```
*   **Line 235**:
    ```markdown
    | FTD-0224 | MC-T4.3 alpha-readout **FOUND audit + correction** (2026-05-28 session): independent adversarial review of the ARC-C1/ARC-B2 "FOUND-at-ARC-2" verdicts + three pre-registered rescue attempts | **[AUDIT + CORRECTION]** — the ARC-C1 (`FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md`) and ARC-B2 (`FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md`) "FOUND-at-ARC-2" verdicts are an **overclaim**; honest status **UNDERDETERMINED**. The determinant grading `16G*³` is an **asserted** master-quadratic Vieta target, not a forward derivation. **No spine claim promoted or demoted**; `x₊=1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`. | NEW 2026-05-28. ... |
    ```

### 1.2 Ledger Assignment of `FTD-0211`
*   **Line 214** of `docs/theory/07_assessment/LEDGER.md` assigns `FTD-0211` to:
    ```markdown
    | FTD-0211 | W5 Moore-shell DM weighting confirmation | **[CLOSED UNDERDETERMINED per pre-reg §5 Outcome B]** ... | NEW 2026-05-27 — `docs/theory/10_eft_program/FOUND_DM_BARYON_W5_CONFIRMATION.md`. ... |
    ```

### 1.3 Document Claim of `FTD-0211`
*   In `docs/theory/10_eft_program/AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md`, the internal header references `FTD-0211`:
    ```markdown
    **LEDGER Row Reference:** FTD-0211 (Audit / Non-circularity Checklist)
    ```

### 1.4 Other Provisional ID Claims and Collisions
*   **`FTD-0212`**: Represents *Lemniscatic K_2-regulator closed-form derivation* (`docs/theory/10_eft_program/FOUND_LEMNISCATIC_K2_REGULATOR.md`) in `LEDGER.md` (Line 215), but is claimed internally by `docs/theory/10_eft_program/DERIV_BCC_ALGEBRAIC_READOUT.md`.
*   **`FTD-0215`** and **`FTD-0216`**: Genuinely missing as rows in `LEDGER.md`. However, they are claimed internally by the BCC Algebraic Bridge (`FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md`) and Alpha Quantization Readout (`FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md`) respectively.
*   **`FTD-0217`** and **`FTD-0218`**: Genuinely missing as rows in `LEDGER.md`. Claimed internally by Determinant-Grading Audit (`AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md`) and Odd-Period J-Twisted Audit (`AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md`) respectively, but map to Confinement and Stochastic Effective Action in the index files.
*   **`FTD-0219`**: Assigned to retracted mass scale calibration loopholes in `LEDGER.md` (Line 218) and mapped to `DERIV_PYTHAGOREAN_FERMAT_BRIDGE.md` in `META_INDEX.md`, but claimed internally by det↔det_ζ Identity Audit (`AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md`).

### 1.5 Ledger Parser Deduplication Logic
*   In `scripts/verification/parsers/ledger_parser.py` (around Line 187), the `parse_ledger` function processes ledger rows sequentially. It keeps track of unique IDs in a set called `seen_ids`.
*   If a row has an ID already present in `seen_ids`, it is silently skipped:
    ```python
    if ftd_id in seen_ids:
        # duplicate id guard; silently skip
        continue
    ```

---

## 2. Logic Chain

1.  Because `LEDGER.md` assigns `FTD-0224` to both the Color Excess row (Line 222) and the MC-T4.3 audit row (Line 235), `seen_ids` records the first instance (Line 222).
2.  When the parser reaches the duplicate `FTD-0224` row on Line 235, the deduplication guard blocks it and skips the entire row.
3.  Consequently, the parser fails to return the MC-T4.3 audit row as a node.
4.  As a result, the `build_math_node_map.py` script generates a `math_node_map.json` that completely lacks any trace of the `MC-T4.3 alpha-readout FOUND audit + correction` row, breaking dependent visualization chains.
5.  Furthermore, the late-May 2026 documents claim provisional IDs internally that either do not exist as rows in `LEDGER.md` or collide with parallel workstreams (e.g. W5 cosmology, Confinement, Stochastic Effective Action, and retracted mass scale), creating severe indexing drift across `META_INDEX.md`, `INDEX_FTD_NATIVE_EFT.md`, and `TRACKER_OPEN_ITEMS.md`.

---

## 3. Caveats

*   **Read-Only Scoping**: This investigation was strictly read-only. No documentation edits or script modifications were made.
*   **Verification Range**: All file paths and statuses of the late-May 2026 documents have been verified, but actual mathematical proofs inside verification scripts (e.g. `proof_det_identity.py`) were not audited, only their presence and success bounds.

---

## 4. Conclusion

The FTD theoretical ledger and its mapping systems suffer from a clear numbering tangle that breaks automated graph visualization. To resolve the issue, the duplicate `FTD-0224` row at Line 235 (MC-T4.3 Audit & Correction) should be reassigned to the sequence-free identifier `FTD-0231`. This will allow the ledger parser to parse the entry successfully. Furthermore, a clean, structured ledgerization of all late-May 2026 documents using newly reserved, non-colliding ledger IDs (`FTD-0232` onwards) is required to restore metadata consistency.

---

## 5. Verification Method

To verify the findings and test the logic chain:
1.  **Duplicate Check**: Run the ledger parser or grep for `FTD-0224` in `LEDGER.md` to confirm the two occurrences on Lines 222 and 235.
2.  **Parser Execution**: Run the math node builder:
    ```powershell
    python scripts/verification/build_math_node_map.py
    ```
    Inspect `scripts/verification/results/math_node_map.json`. Search for `FTD-0224` to confirm that only the Color Excess entry is present and the MC-T4.3 Audit entry is entirely missing.
3.  **Renaming Invalidation Condition**: If the second row (Line 235) is renamed to a unique identifier (such as `FTD-0231`), running the map builder again will successfully add the MC-T4.3 entry, proving the logic chain.
