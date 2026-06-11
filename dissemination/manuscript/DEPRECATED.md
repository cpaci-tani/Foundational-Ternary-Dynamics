# DEPRECATED — manuscript v1

**Status:** Manuscript v1 (this directory) is deprecated. The canonical manuscript is **`dissemination/manuscript_v2/`** (physicist-targeted rewrite, April 2026).

**Decision (2026-05-03):** v1 is kept on disk for historical / archival purposes. Do not edit; do not cite as canonical.

**Build status:** v1 requires Jupyter for Quarto rendering. The current build environment does not have Jupyter installed; v1 build fails at render time. v2 builds clean (HTML).

**Replacements:**

- **Canonical manuscript:** `dissemination/manuscript_v2/src/` (single source of truth).
- **Per-chapter authoring:** see `dissemination/manuscript_v2/PROPAGATION_RULE.md` for the v2 src  vol1 propagation policy.
- **Published-grade companion papers:** `dissemination/papers/PAPER_A_PI_FREE_GENERATOR.tex` + `PAPER_B_BCC_COMPLEX_STRUCTURE.tex` + `PAPER_FTD_AS_WILSONIAN_EFT.tex`.

**Documentation builder note:** the documentation-builder agent's audit (2026-05-02) flagged v1 build as `FAIL — Jupyter not installed`. Build pipelines should skip this directory.

If v1 is to be revived, the cost is repairing the Jupyter dependency and accepting that v1's content will diverge from v2 (no PROPAGATION_RULE between v1 and v2; they are different products).
