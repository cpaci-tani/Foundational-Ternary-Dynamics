# DEPRECATED — these notebooks do not execute

**Status:** all 12 notebooks in this directory import the `ternary_matrix.{config, model.grid, physics}` Python package, which **does not exist in the current repository**. The current canonical Python module is `scripts/constants.py`. None of these notebooks will execute without first porting the imports to the new API.

**Decision (2026-05-03):** these notebooks are deprecated. They are kept on disk for their narrative/markdown content (pedagogical commentary), but should not be referenced by current docs as runnable artifacts.

**Replacements:**

- For verification of any spine theorem: `scripts/proofs/proof_*.py`
- For derivation walkthroughs: `docs/theory/03_derivations/`
- For canonical reference: `docs/theory/07_assessment/TRACKER_ONTIC_TRUTH.md`
- For published-grade exposition: `dissemination/papers/PAPER_A_PI_FREE_GENERATOR.tex` + `PAPER_B_BCC_COMPLEX_STRUCTURE.tex` + `PAPER_FTD_AS_WILSONIAN_EFT.tex`

If a notebook is to be revived, the porting effort is to replace `ternary_matrix.*` imports with the equivalent functions from `scripts/constants.py` and run end-to-end. Estimated effort: 1-2 days per notebook.

**Documentation builder note:** the documentation-builder agent's audit (2026-05-02) flagged these as `will fail on execute`. Build pipelines should skip this directory.
