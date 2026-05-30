# FTD Ledger Numbering Tangle & Mapping Analysis

## 1. Executive Summary

This report presents a thorough, read-only exploration and analysis of the Foundational Ternary Dynamics (FTD) ledger (`docs/theory/07_assessment/LEDGER.md`), indexing files, mapping scripts, and late-May 2026 theoretical documents. The objective is to map out the numbering tangle and duplicate identifiers introduced during late-May 2026 research.

### Core Findings
1. **Duplicate Identifier `FTD-0224`**: The ID `FTD-0224` is used for two separate, unrelated entries in `LEDGER.md`:
   - **Line 222**: *Color Excess closed form & Blocked Effective Action Flow*
   - **Line 235**: *MC-T4.3 alpha-readout FOUND audit + correction*
2. **Missing Ledger Entries**: Several late-May 2026 documents that claim specific FTD IDs internally have no dedicated rows in `LEDGER.md`. Specifically, IDs `FTD-0215`, `FTD-0216`, `FTD-0217`, and `FTD-0218` do not exist as rows in the ledger.
3. **Severe ID Collisions**: There is a mismatch between the internal IDs claimed by documents and the master ledger assignments:
   - `FTD-0211` represents *W5 Moore-shell DM weighting confirmation* in the ledger (Line 214), but is internally claimed by `AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md`.
   - `FTD-0212` represents *Lemniscatic K_2-regulator closed-form derivation* in the ledger (Line 215), but is internally claimed by `DERIV_BCC_ALGEBRAIC_READOUT.md`.
   - `FTD-0219` represents *Absolute Mass Scale Calibration (μ) generation loopholes* (retracted) in the ledger (Line 218) and is mapped to `DERIV_PYTHAGOREAN_FERMAT_BRIDGE.md` in `META_INDEX.md`, but is also claimed provisionally by `PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md`.
   - `FTD-0217` and `FTD-0218` are provisional IDs claimed by alpha-readout documents, but they collide with parallel native program campaigns (Confinement and Stochastic Effective Action, respectively) in `META_INDEX.md` and `INDEX_FTD_NATIVE_EFT.md`.
4. **Parser & Mapping Failure**: The duplicate `FTD-0224` row causes `scripts/verification/parsers/ledger_parser.py` (and consequently `scripts/verification/build_math_node_map.py`) to silently skip the `MC-T4.3 alpha-readout FOUND audit + correction` entry due to deduplication logic (`seen_ids`). Consequently, this critical correction is completely omitted from the math node graph.

---

## 2. Duplicate `FTD-0224` Rows in `LEDGER.md`

`LEDGER.md` contains two separate, duplicate occurrences of the identifier `FTD-0224`.

### Occurrence 1: Line 222
*   **Line Number**: 222
*   **Exact Verbatim Content**:
    ```markdown
    | FTD-0224 | Color Excess closed form & Blocked Effective Action Flow | **[CLOSED RESOLVED]** — Explores the 100-digit precision arithmetic of the color excess $\delta_c$ and bounds its algebraic transcendence over $\mathbb{Q}(G^*, \pi, \alpha)$ using PSLQ; rigorously maps the Onsager-Machlup history flow and the multi-scale blocking equations to the standard physical Maxwell action $F_{\mu\nu}F^{\mu\nu}$ as a stable IR fixed point. | NEW 2026-05-27 — `docs/theory/09_mathematical/EXPLR_COLOR_EXCESS_CLOSED_FORM.md` and `docs/theory/10_eft_program/DERIV_FTD_NATIVE_NONLINEAR_FLOW.md`. High-precision explore runner script `scripts/exploration/explore_color_excess.py` confirms transcendence relations. |
    ```

### Occurrence 2: Line 235
*   **Line Number**: 235
*   **Exact Verbatim Content**:
    ```markdown
    | FTD-0224 | MC-T4.3 alpha-readout **FOUND audit + correction** (2026-05-28 session): independent adversarial review of the ARC-C1/ARC-B2 "FOUND-at-ARC-2" verdicts + three pre-registered rescue attempts | **[AUDIT + CORRECTION]** — the ARC-C1 (`FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md`) and ARC-B2 (`FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md`) "FOUND-at-ARC-2" verdicts are an **overclaim**; honest status **UNDERDETERMINED**. The determinant grading `16G*³` is an **asserted** master-quadratic Vieta target, not a forward derivation. **No spine claim promoted or demoted**; `x₊=1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`. | NEW 2026-05-28. **Review:** `docs/theory/10_eft_program/AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md` — found the FOUND rests on §3 "Selection 1" (the transfer-matrix trace/det `=16G*²`/`16G*³` are "selections designed to couple to the master quadratic … rather than first-principles derivations"), which fires F-j and fails the no-cheat audit's Gate 4. **Three pre-registered closure attempts** (commit deferred per owner; SHAs recorded in-session): (1) `PREREG_ALPHA_READOUT_DETERMINANT_GRADING_v1.md` SHA256 `f55c7504401a1e5eb4a61ae18380d10c0ae8a4d407cfb1fc48da45e91918abd7` → `AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md` **[CLOSED NEGATIVE — scoped]** (G\*-degree parity within the frozen set *excluding* the det_ζ ratio; lifted once det_ζ = G\* is admitted in attempt 2, so not the operative obstruction); verified `scripts/proofs/proof_determinant_grading_parity.py` (11/11). (2) `PREREG_ALPHA_READOUT_ODD_PERIOD_v1.md` SHA256 `a5c97b7363a1e389ea5e2eff0f139a00f0bd04f8b0d21166845fefd38c53faa1` → `AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md` **[UNDERDETERMINED]** (the J-twisted ζ-reg determinant ratio = G\* is a *clean* forward odd source — owner's "lattice is J²" hint partially vindicated — but `Det=Tr·G*` stays asserted: OP3 fires); verified `scripts/proofs/proof_odd_period_jtwisted.py` (6/6). (3) `PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md` SHA256 `03b967c760fa38fffa8c7d08d5a75c34392dcd2c4c546f24a9c58b4d97a78122` → `AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md` **[UNDERDETERMINED]** (corrected 2026-05-28 from CLOSED-NEGATIVE per owner review: `G_BCC(0)`/det_ζ are *scalars*, so the coefficients `16=|μ₄|²`, `G*²=2π·G_BCC(0)`, `G*` are forward-derived and `16G*³=16G*²·G*` IS assemblable; but a 2×2's Tr and Det are independent, so the readout's `(Tr,Det)` structure is the *unforced* imposed master quadratic — **W-CRIT-2** — not a hard no-go); verified `scripts/proofs/proof_det_identity.py` (7/7). **Correction applied:** both FOUND resolution docs received top **FOUND → UNDERDETERMINED** correction banners + §5/§6 "F-j PASS"/"from first principles" fixes; the genuine `[THEOREM]`/`[DERIVED]` kernel (`V_complex≅Z[i]²`, charge quantization, `16=|μ₄|²`, Watson `G*²/(2π)`, J-twisted det_ζ ratio=G\*, finite-block CLOSED-NEGATIVE) is preserved at grade. **Surviving MC-T4.3 space:** ARC-D (engine-native measurement) or a `[CONJECTURE — new postulate]`; ARC-A/B1 already closed-negative. **⚠ Pre-existing LEDGER numbering tangle flagged (not fixed here):** the alpha-readout-extension docs claim ids FTD-0210/0211/0212/0215/0216 (`SCOPE_ALPHA_READOUT_NEXT_STEPS`, `AUDIT_CHARGE_QUANTIZATION_NO_CHEAT`, `DERIV_BCC_ALGEBRAIC_READOUT`, `FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION`, `FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION`) but those numbers are assigned to the **parallel** 2026-05-27 workstream (x_-, W5, K₂-regulator) or are absent — i.e. the FOUND resolutions have **no canonical LEDGER rows** and their doc-internal ids collide. This session does **not** renumber that committed tangle; it is flagged for a dedicated cleanup. The three new pre-regs above carry doc-internal provisional ids FTD-0217/0218/0219 ("provisional; confirm at lock") which are **superseded by this consolidated FTD-0224 row** (0219 collides with the retracted mass-scale row). |
    ```

---

## 3. MC-T4.3 Row Details in `LEDGER.md`

*   **Line Number**: 235
*   **Current ID**: `FTD-0224` (Duplicate)
*   **Row Topic**: `MC-T4.3 alpha-readout FOUND audit + correction`
*   **Status**: `**[AUDIT + CORRECTION]**`
*   **Description summary**: Details the independent review of the ARC-C1 (`FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md`) and ARC-B2 (`FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md`) "FOUND-at-ARC-2" verdicts, showing they were overclaims and downgrading their status to `UNDERDETERMINED`. It documents the three rescue attempts:
    1.  `PREREG_ALPHA_READOUT_DETERMINANT_GRADING_v1.md` → `AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md` (CLOSED NEGATIVE - scoped)
    2.  `PREREG_ALPHA_READOUT_ODD_PERIOD_v1.md` → `AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md` (UNDERDETERMINED)
    3.  `PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md` → `AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md` (UNDERDETERMINED)
*   **Critical Footnote inside row**: Explicitly flags that the alpha-readout-extension docs internally claim `FTD-0210/0211/0212/0215/0216` but those are assigned to a parallel workstream (x_-, W5, K₂-regulator) or are absent.

---

## 4. Collision 1: `FTD-0211` (Charge Quantization vs W5 Cosmology)

### `AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md` Claim
*   **File Path**: `docs/theory/10_eft_program/AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md`
*   **Claimed ID**: `FTD-0211`
*   **Verbatim Context**:
    ```markdown
    **LEDGER Row Reference:** FTD-0211 (Audit / Non-circularity Checklist)
    ```

### `LEDGER.md` Assignment
*   **Line Number**: 214
*   **Assigned ID**: `FTD-0211`
*   **Verbatim Row**:
    ```markdown
    | FTD-0211 | W5 Moore-shell DM weighting confirmation | **[CLOSED UNDERDETERMINED per pre-reg §5 Outcome B]** — W5 predicts both primordial Helium fraction $Y_p$ and CMB acoustic scale $\ell_1$ within a $5.0\%$ family deviation threshold ($2.84\%$ and $0.91\%$ respectively), but fails the strict $1.5\%$ threshold on $Y_p$. W1 is strongly excluded by $> 16\sigma$. | NEW 2026-05-27 — `docs/theory/10_eft_program/FOUND_DM_BARYON_W5_CONFIRMATION.md`. Pre-reg `PREREG_DM_BARYON_W5_INDEPENDENT_CONFIRMATION_v1.md` hash-locked at commit `ae9996e` (tag `preregister-w5-confirmation-v1`, SHA256 `a771b279327b0e82d409b645416ca9b1a68633b129e0852e875790150dbaa2ee`). Runner script `scripts/exploration/verify_w5_cosmology.py` evaluated the parameters. |
    ```

### Nature of Collision
*   **Subject Matter Collision**: The master ledger assigns `FTD-0211` to the **W5 Moore-shell Dark Matter / Cosmology bridge** campaign. However, the theoretical document `AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md` (which covers electric charge quantization and non-circularity checks) internally claims `FTD-0211`.
*   **Result**: A direct collision. `LEDGER.md` has no entry representing the Charge Quantization audit.

---

## 5. Collision 2: Provisional ID Collisions (`FTD-0212` to `FTD-0219`)

In addition to `FTD-0211`, multiple other identifiers collide or are missing dedicated rows in `LEDGER.md`.

### Collision Matrix

| ID | Ledger Assignment (in `LEDGER.md`) | Late-May 2026 Document Claim | Parallel Mapping (in indexes) |
|---|---|---|---|
| **FTD-0212** | **Lemniscatic K_2-regulator closed-form derivation** (Line 215, `FOUND_LEMNISCATIC_K2_REGULATOR.md`) | **BCC Algebraic Readout Derivation** (`DERIV_BCC_ALGEBRAIC_READOUT.md`) | N/A |
| **FTD-0215** | *None* (Completely missing as a row) | **BCC Algebraic Bridge Resolution** (`FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md` and `PREREG_ALPHA_READOUT_BCC_BRIDGE_v1.md`) | BCC Algebraic Bridge Resolution |
| **FTD-0216** | *None* (Completely missing as a row) | **Alpha Quantization Readout Resolution** (`FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md` and `PREREG_ALPHA_READOUT_QUANTIZATION_v1.md`) | Alpha Quantization Readout Resolution |
| **FTD-0217** | *None* (Completely missing as a row) | **Determinant-Grading Forward Derivation Audit** (`AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md` and `PREREG_ALPHA_READOUT_DETERMINANT_GRADING_v1.md`) | **Color Confinement Resolution** (`FOUND_COLOR_CONFINEMENT_RESOLUTION.md` and `PREREG_COLOR_CONFINEMENT_v1.md`) |
| **FTD-0218** | *None* (Completely missing as a row) | **Odd-Period J-Twisted Audit** (`AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md` and `PREREG_ALPHA_READOUT_ODD_PERIOD_v1.md`) | **Stochastic Effective Action Resolution** (`FOUND_STOCHASTIC_EFFECTIVE_ACTION_RESOLUTION.md` and `PREREG_STOCHASTIC_EFFECTIVE_ACTION_v1.md`) |
| **FTD-0219** | **Absolute Mass Scale Calibration (μ) generation loopholes** (Line 218, retracted mass scale) | **det↔det_ζ Structural Identity Audit** (`AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md` and `PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md`) | **Pythagorean-Fermat Bridge** (`DERIV_PYTHAGOREAN_FERMAT_BRIDGE.md`) |

---

## 6. Late-May 2026 Document Inventory

The existence, exact file paths, internal headers, and actual statuses of the 10 late-May 2026 documents have been verified.

### 1. BCC Algebraic Bridge Readout (ARC-B2)
*   **Resolution File Path**: `docs/theory/10_eft_program/FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md`
    *   *Internal Header*: `FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md` (claims FTD-0215 via pre-reg link).
    *   *Actual Status*: **UNDERDETERMINED** (downgraded from `FOUND` via a prominent top correction banner added on 2026-05-28).
*   **Pre-Registration File Path**: `docs/theory/10_eft_program/PREREG_ALPHA_READOUT_BCC_BRIDGE_v1.md`
    *   *Internal Header*: `PREREG_ALPHA_READOUT_BCC_BRIDGE_v1.md` (claims FTD-0215 on Line 7).
    *   *Actual Status*: **[PRE-REGISTRATION]**

### 2. Alpha Quantization Readout (ARC-C1)
*   **Resolution File Path**: `docs/theory/10_eft_program/FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md`
    *   *Internal Header*: `FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md` (claims FTD-0216).
    *   *Actual Status*: **UNDERDETERMINED** (downgraded from `FOUND` via a prominent top correction banner added on 2026-05-28).
*   **Pre-Registration File Path**: `docs/theory/10_eft_program/PREREG_ALPHA_READOUT_QUANTIZATION_v1.md`
    *   *Internal Header*: `PREREG_ALPHA_READOUT_QUANTIZATION_v1.md` (claims FTD-0216 on Line 7).
    *   *Actual Status*: **[PRE-REGISTRATION]**

### 3. Determinant-Grading Forward Derivation Audit
*   **Audit File Path**: `docs/theory/10_eft_program/AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md`
    *   *Internal Header*: Claims provisional `FTD-0217` on Line 16.
    *   *Actual Status*: **[CLOSED NEGATIVE — scoped]** (retained as a scoped technical result; parity no-go does not apply if J-twisted det_ζ ratio is admitted).
*   **Pre-Registration File Path**: `docs/theory/10_eft_program/PREREG_ALPHA_READOUT_DETERMINANT_GRADING_v1.md`
    *   *Internal Header*: Claims provisional `FTD-0217` on Line 7.
    *   *Actual Status*: **[PRE-REGISTRATION]**

### 4. Odd-Period J-Twisted Audit
*   **Audit File Path**: `docs/theory/10_eft_program/AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md`
    *   *Internal Header*: Claims provisional `FTD-0218` on Line 5.
    *   *Actual Status*: **[UNDERDETERMINED]** (the J-twisted ζ-reg determinant ratio is a clean forward odd source `= G*`, but relating it to the finite operator determinant is unforced).
*   **Pre-Registration File Path**: `docs/theory/10_eft_program/PREREG_ALPHA_READOUT_ODD_PERIOD_v1.md`
    *   *Internal Header*: Claims provisional `FTD-0218` on Line 7.
    *   *Actual Status*: **[PRE-REGISTRATION]**

### 5. det↔det_ζ Structural Identity Audit
*   **Audit File Path**: `docs/theory/10_eft_program/AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md`
    *   *Internal Header*: Claims provisional `FTD-0219` (consolidated under `FTD-0224` row) on Line 5.
    *   *Actual Status*: **[UNDERDETERMINED]** (corrected from CLOSED-NEGATIVE; the determinant grading is unforced, not impossible).
*   **Pre-Registration File Path**: `docs/theory/10_eft_program/PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md`
    *   *Internal Header*: Claims provisional `FTD-0219` on Line 7.
    *   *Actual Status*: **[PRE-REGISTRATION]**

---

## 7. Indexing and Mapping File Mappings

### 1. `docs/theory/META_INDEX.md`
The master index registers the following mappings:
*   `FTD-0214` → `docs/theory/09_mathematical/DERIV_JONES_INDEX_THRESHOLD_RATIO.md`
*   `FTD-0215` → `docs/theory/10_eft_program/FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md` & `docs/theory/10_eft_program/PREREG_ALPHA_READOUT_BCC_BRIDGE_v1.md`
*   `FTD-0216` → `docs/theory/10_eft_program/FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md` & `docs/theory/10_eft_program/PREREG_ALPHA_READOUT_QUANTIZATION_v1.md`
*   `FTD-0217` → `docs/theory/10_eft_program/FOUND_COLOR_CONFINEMENT_RESOLUTION.md` & `docs/theory/10_eft_program/PREREG_COLOR_CONFINEMENT_v1.md` *(Collides with internal Determinant Grading ID)*
*   `FTD-0218` → `docs/theory/10_eft_program/FOUND_STOCHASTIC_EFFECTIVE_ACTION_RESOLUTION.md` & `docs/theory/10_eft_program/PREREG_STOCHASTIC_EFFECTIVE_ACTION_v1.md` *(Collides with internal Odd-Period ID)*
*   `FTD-0219` → `docs/theory/09_mathematical/DERIV_PYTHAGOREAN_FERMAT_BRIDGE.md` *(Collides with internal Determinant Identity ID)*
*   `FTD-0220` → `docs/theory/10_eft_program/FOUND_NO_4TH_GENERATION_NO_GO.md` & `docs/theory/10_eft_program/PREREG_NO_4TH_GENERATION_NO_GO_v1.md`
*   `FTD-0223` → `docs/theory/10_eft_program/SPEC_FTD_DYNAMICAL_SU3_HADRODYNAMICS.md`
*   `FTD-0224` → `docs/theory/09_mathematical/EXPLR_COLOR_EXCESS_CLOSED_FORM.md` *(Maps only to Color Excess, completely ignores the duplicate audit row)*

### 2. `docs/theory/10_eft_program/INDEX_FTD_NATIVE_EFT.md`
The program-specific index registers:
*   `FTD-0215` → `FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md` and `PREREG_ALPHA_READOUT_BCC_BRIDGE_v1.md`
*   `FTD-0216` → `FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md` and `PREREG_ALPHA_READOUT_QUANTIZATION_v1.md`
*   `FTD-0217` → `FOUND_COLOR_CONFINEMENT_RESOLUTION.md` and `PREREG_COLOR_CONFINEMENT_v1.md`
*   `FTD-0218` → `FOUND_STOCHASTIC_EFFECTIVE_ACTION_RESOLUTION.md` and `PREREG_STOCHASTIC_EFFECTIVE_ACTION_v1.md`
*   `FTD-0220` → `FOUND_NO_4TH_GENERATION_NO_GO.md` and `PREREG_NO_4TH_GENERATION_NO_GO_v1.md`
*   *(Note: The MC-T4.3 audit and correction is completely omitted from this index)*

### 3. `docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md`
References:
*   `FTD-0214` → `AUDIT_ALPHA_READOUT_BOUNDARY_CLOSED_NEGATIVE.md` (Line 332)
*   `FTD-0219` → retracted mass scale `docs/theory/archive/EXPLR_MASS_SCALE_GENERATION_RETRACTED.md` (Line 360)
*   `FTD-0222` → active campaign `FTD-0222` for `SPEC_CLASS_C_CLUSTER_INTERACTION.md`
*   `FTD-0223` → active campaign `FTD-0223` for Hadrodynamics spec
*   `FTD-0224` → active campaign `FTD-0224` for Color Excess (`EXPLR_COLOR_EXCESS_CLOSED_FORM.md` / `DERIV_FTD_NATIVE_NONLINEAR_FLOW.md`)
*   `FTD-0220` → closed campaign for No 4th Generation Fermions

---

## 8. Math Node Map Generation Logic Analysis

The master script `scripts/verification/build_math_node_map.py` builds the complete algebraic/theoretical dependence map by extracting data from two primary inputs:
1.  The source code corpus (via AST-based and heuristic extractors in `scripts/verification/parsers/`).
2.  The master ledger `docs/theory/07_assessment/LEDGER.md` (via `scripts/verification/parsers/ledger_parser.py`).

### Ledger Parser Mechanics (`ledger_parser.py`)
*   The function `parse_ledger` reads `LEDGER.md` sequentially.
*   It splits lines by the `|` character to extract rows that match the ledger quick-index row format.
*   **The Deduplication Guard**: The parser maintains a set `seen_ids = set()`. If an extracted FTD ID (such as `FTD-0224`) is already present in `seen_ids`, the parser **silently skips** that row to prevent duplicate nodes.

### Resulting Breakdowns
*   When parsing the first `FTD-0224` row (Line 222 - Color Excess), the parser records `FTD-0224` in `seen_ids`.
*   When it reaches the duplicate `FTD-0224` row (Line 235 - MC-T4.3 alpha-readout audit), the deduplication guard blocks it. The row is skipped entirely.
*   Therefore, the **MC-T4.3 alpha-readout FOUND audit + correction is completely omitted from the math node graph**.
*   Any identities or theorems in the proofs that refer to the audit/correction of MC-T4.3 are left un-anchored in the map, producing broken layout dependencies in the visualizations (matplotlib, Mermaid, interactive HTML).

---

## 9. Recommendations for Cleanup

To resolve this numbering tangle and align all mapping systems without violating the read-only audit protocol:
1.  **Resolve Duplicate `FTD-0224`**: The duplicate row at Line 235 in `LEDGER.md` (MC-T4.3 Audit & Correction) should be assigned a fresh, unused FTD ID. Since the ledger currently goes up to `FTD-0230`, the next available sequence-free identifier is `FTD-0231`. Renaming this row to `FTD-0231` will allow the ledger parser to successfully extract it and build its dependencies in the math node map.
2.  **Ledgerize Missing Late-May 2026 Documents**: Rows should be added to `LEDGER.md` for the un-ledgered campaigns (e.g., BCC Algebraic Bridge, Alpha Quantization Readout, and their corresponding audits/pre-regs). They can be assigned canonical unused IDs starting from `FTD-0232` onwards.
3.  **De-collide Document Internal IDs**: Update the frontmatter and text references inside `FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md`, `FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md`, and the audit/pre-registration documents to refer to their newly assigned canonical ledger IDs instead of the provisional/colliding ones.
4.  **Align Indexing and Mapping Layer References**: Synchronize `META_INDEX.md`, `INDEX_FTD_NATIVE_EFT.md`, and `TRACKER_OPEN_ITEMS.md` to point to the resolved non-colliding IDs.
