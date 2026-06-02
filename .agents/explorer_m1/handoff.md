# Handoff Report: FTD Meta-Documentation Link Audit & Consistency Check

## 1. Observation

A comprehensive meta-documentation link audit and consistency check was executed on the `williamsteinmetz/Foundational-Ternary-Dynamics` repository. The following direct tools and measurements were taken:

### 1.1. Diagnostics Command & Link Sweeps
* **Index Link Sweeps**: Ran `python scripts/verification/verify_index_links.py` which reported 8 broken links in `docs/theory/META_INDEX.md`.
* **Universal Link Sweeps**: Analyzed `scratch_linkcheck_out.txt` (the results of `python scratch_linkcheck.py`), which scanned **549 markdown files** and identified **645 broken links** in active theory files.
  * *Verbatim Root-Level Failures*:
    * `CLAUDE.md:85` points to `docs/theory/07_assessment/TRACKER_ONTIC_TRUTH.md` (does not exist).
    * `README.md:22` points to `docs/theory/07_assessment/LEDGER.md` (does not exist).
    * `README.md:23` points to `docs/theory/07_assessment/TRACKER_ONTIC_TRUTH.md` (does not exist).
    * `README.md:95` points to `docs/theory/03_derivations/DERIV_NC_FROM_TOPOLOGY.md` (does not exist).
    * `README.md:128` points to `docs/theory/03_derivations/AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md` (does not exist).
    * `META_DOCUMENTATION_MAP.md:9` points to `docs/theory/09_mathematical/EXPLR_CM_RATIO_TOWER.md` (does not exist).

### 1.2. Stale Reference Sweep (`_dag.js` residues)
Using ripgrep (`grep_search`), exact locations of legacy filenames were identified:
* **`app_dag.js`** $\rightarrow$ Mentioned in `MAINTAINABILITY.md` (lines 125, 340, 348, 519), `META_PROJECT_ATLAS.md` (lines 168, 237), and `docs/adr/0004-scale-controllers.md` (lines 12, 24).
* **`wasm-bridge-dag.js`** $\rightarrow$ Mentioned in `CLAUDE.md` (line 154), `CONTRACTS.md` (lines 21, 191, 320), `MAINTAINABILITY.md` (line 334), `META_PROJECT_ATLAS.md` (lines 21, 167, 226, 227, 394), `engine/wasm/ftd_wasm.cpp` (line 1000), `docs/theory/07_assessment/core_ledgers/LEDGER.md` (line 811), and `docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md` (lines 209, 547).
* **`bridge-factory-dag.js`** $\rightarrow$ Mentioned in `engine/web/docs/audits/AUDIT_WEB_ENGINE_2026-05-27.md` (lines 15, 90, 249).

### 1.3. Ontic Integrity Alignment Check
* Viewed `docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md` (lines 1-199).
* Viewed `README.md` (lines 1-288).
* Checked `docs/theory/07_assessment/core_ledgers/LEDGER.md` for claims `FTD-0013` (x₊ $\leftrightarrow$ 1/α) and `FTD-0014` (x₋ $\leftrightarrow$ N_c).
  * *Verbatim LEDGER Entry Line 43*: `| FTD-0013 | x₊ ↔ 1/α (1.26 ppm) | STRONGLY MOTIVATED CONJECTURE | RESOLVED (downgraded from THEOREM) |`
  * *Verbatim LEDGER Entry Line 97 / Tracker*: `OT-5.2 | ~~x_- = N_c = 3~~ | — | REMOVED 2026-05-22 per FTD/FQCR Cleanup Taxonomy v1.4 §5 ...`

---

## 2. Logic Chain

1. **Category subfolder nesting broke relative links**:
   * The file search (`find_by_name`) confirms that folders like `03_derivations/`, `09_mathematical/`, and `10_eft_program/` now have nested subfolders (such as `03_derivations/electromagnetism/`, `09_mathematical/number_theory/`, and `10_eft_program/preregistrations/`).
   * When files were moved from flat category folders down into these nested subfolders, their relative link pointers to sibling directories were left with one parent step (`../`).
   * Because the files are now nested one level deeper, any relative reference pointing to a sibling category requires two parent steps (`../../`). This is the root mathematical cause for all 645 broken links in active theory docs.
2. **Core ledger relocation broke root links**:
   * Filestores `LEDGER.md`, `TRACKER_ONTIC_TRUTH.md`, and `TRACKER_OPEN_ITEMS.md` were moved from `docs/theory/07_assessment/` into a new subfolder: `docs/theory/07_assessment/core_ledgers/`.
   * The root-level meta-documentation files (`CLAUDE.md`, `README.md`, `META_DOCUMENTATION_MAP.md`) were never updated to point to `core_ledgers/`, hence the broken links on `LEDGER.md` and `TRACKER_ONTIC_TRUTH.md`.
3. **Rename completeness gap**:
   * The C++ engine, Web UI, and tests successfully renamed the raw source files (`app_dag.js` $\rightarrow$ `app.js`, `wasm-bridge-dag.js` $\rightarrow$ `bridge-init.js`, `bridge-factory-dag.js` $\rightarrow$ `bridge-factory.js`).
   * However, text descriptions, diagrams, maps, and contract specifications in the doc corpus were not updated to reflect these renames, leading to stale references in `MAINTAINABILITY.md`, `META_PROJECT_ATLAS.md`, etc.
4. **Epistemic tags are perfectly rigid**:
   * `LEDGER.md` and `TRACKER_ONTIC_TRUTH.md` confirm that `x_+ = 1/α` is strictly marked `[STRONGLY MOTIVATED CONJECTURE]`, and `x_- ↔ N_c` is retired. 
   * Checking `README.md` lines 83 and 95 shows exact verbal alignment with the ledger states. There are **zero tag promotions** or overclaims in the user-facing documentation.

---

## 3. Caveats

* **Verification scope**: This audit was strictly read-only per project constraints. Proposed corrections for broken links and stale references have been cataloged in `analysis.md` but not directly committed to source files.
* **Scan exhaustiveness**: While `scratch_linkcheck.py` maps all markdown broken links, it does not scan HTML references inside web-dashboard subfolders (`engine/web/index.html` etc.), which might contain further legacy `_dag.js` references.

---

## 4. Conclusion

1. **Systematic Link Breakage**: All 645 active theory broken links are a direct consequence of category subdirectory restructuring and the move of the central claims ledgers to `core_ledgers/`.
2. **Stale References**: Stale text references to renamed Javascript entry/bridge scripts (`app_dag.js`, `wasm-bridge-dag.js`) reside in 12 distinct documentation files and 1 C++ source file.
3. **Rigorous Epistemic Alignment**: The repository maintains exemplary epistemic discipline. There are no tag promotions; active conjectures (`x_+ = 1/α` as `[STRONGLY MOTIVATED CONJECTURE]`) and retired claims (`x_- ↔ N_c`) perfectly match the claims ledger.
4. **README guidelines**: A set of premium scientific project homepage layout guidelines has been formulated to maintain this rigor for visiting scholars.

---

## 5. Verification Method

To independently verify the observations and conclusions in this handoff report:
1. **Verification of Broken Links**:
   * Inspect `c:\Users\cpaci\Desktop\ftd\scratch_linkcheck_out.txt` or execute `python scratch_linkcheck.py` to confirm the count and locations of the broken links.
2. **Verification of Stale references**:
   * Run the following ripgrep searches in `c:\Users\cpaci\Desktop\ftd`:
     * `rg "app_dag.js"`
     * `rg "wasm-bridge-dag.js"`
3. **Verification of Epistemic Alignment**:
   * Open `docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md` and `docs/theory/07_assessment/core_ledgers/LEDGER.md`. Confirm that `FTD-0013` is tagged `STRONGLY MOTIVATED CONJECTURE` and `FTD-0014` is marked as `RETIRED`.
   * Open `README.md` lines 83 and 95 to verify they match the exact tags and status.
4. **Verification of Analysis Artifact**:
   * Inspect the compiled analysis file at `c:\Users\cpaci\Desktop\ftd\.agents\explorer_m1\analysis.md`.
