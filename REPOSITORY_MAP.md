# Repository Map

This is the short entry point for a tracked checkout of Foundational Ternary Dynamics. `git ls-files` is the exhaustive file inventory; this map groups those files so an agent can open the smallest relevant area. Counts are a 2026-09-24 index snapshot (8,385 paths), not a test-pass claim; rerun `git ls-files` after later commits.

The [2026-09-24 repository audit](docs/reference/REF_REPOSITORY_AUDIT_2026_09_24.md) records measured verification, public-checkout gaps, and cleanup still requiring a policy decision.

| Area | Tracked files | Scope | Start here |
| --- | ---: | --- | --- |
| `engine/` | 4,184 | C++ engine, CUDA, native UI, browser dashboard, tests, and recorded campaigns | `engine/SPEC_ENGINE.md`, `engine/CMakeLists.txt`, `engine/web/index.html` |
| `docs/` | 1,948 | Specifications, theory, references, papers, and assessment records | `docs/SPEC_FTD.md`, `docs/WHERE_WE_LEFT_OFF.md`, `docs/theory/META_INDEX.md` |
| `scripts/` | 1,680 | Constants, proofs, verification, experiments, exploration, visualization, and tests | `scripts/constants.py`, `scripts/tests/` |
| `dissemination/` | 487 | Manuscripts, papers, notebooks, and interactive publications | `dissemination/manuscript_v2/`, `dissemination/interactive/` |
| `lean/` | 36 | Lean formalization | `lean/` |
| `evaluation/` | 17 | Assessment inputs and reports | `evaluation/` |
| `tools/`, `resources/`, `.github/`, and root files | 33 | Maintenance, shared assets, CI, and project policy | `CLAUDE.md`, `.github/workflows/` |

## Ownership

- **Project rules:** `CLAUDE.md` is the tracked instruction source. For claim status, use `docs/theory/07_assessment/core_ledgers/LEDGER.md` before prose or older constitutions.
- **Theory:** `docs/theory/` has 1,856 tracked files. Most are in `10_eft_program/` (1,290); the other areas are derivations, foundations, mathematics, references, assessment, structural work, coupling, particles, and reference frames. Search `docs/theory/META_INDEX.md` for the relevant canonical document, then check its ledger entry. Working items and closed provenance are separate `TRACKER_OPEN_ITEMS.md` and `TRACKER_RESOLVED_ITEMS.md` files under `07_assessment/core_ledgers/`.
- **Engine:** `engine/include/ftd/` declares the simulation API; `engine/src/` implements CPU behavior; `engine/cuda/` has GPU paths; `engine/wasm/` binds the browser runtime; `engine/native/` is the desktop UI; `engine/strict/` hosts strict experiments. `engine/cmake/` owns test registration. `engine/tests/` is source; `engine/results/` contains tracked campaign provenance.
- **Browser:** `engine/web/js/` holds runtime modules, `css/` styling, `assets/` media, `wasm/` generated deployment binaries, and `tests/` browser and Node checks. `scripts/assistant/stage_site.py` selects the public Pages artifact. Test source is part of the repository; test output is local.
- **Cross-scale boundary:** [Scale ownership and recovery](docs/reference/REF_SCALE_OWNERSHIP_AND_RECOVERY.md) records the separate dashboard state owners and the evidence required for a v3 coarse-graining claim. A shared UI bridge is not a common microscopic law.
- **Scripts:** `scripts/proofs/` (812) and `scripts/verification/` (75) check formal and numerical claims; `scripts/exploration/` (268) is exploratory work; `scripts/experiments/` (167) is experimental code; `scripts/visualization/` (55) generates figures; `scripts/tests/` (136) contains Python test files. Do not promote exploratory or parametric output to a theorem.
- **Publications:** `dissemination/` and `docs/papers/` carry publication sources. Generated document renders and caches are ignored unless explicitly tracked.

## Exact Inventory

Run `git ls-files` for every public checkout path. Narrow it with `git ls-files -- engine/web/tests` or `git ls-files -- docs/theory/10_eft_program`. Use `rg --files` for working-tree source discovery, and `git status --short` to distinguish new work from tracked content. The root `.gitignore` excludes local credentials, agent state, caches, builds, and regenerated results; it does not conceal files already tracked or present in Git history.

`python scripts/verification/verify_index_links.py` checks whether the two main theory indexes resolve in the working tree. Add `--tracked` to check whether their targets would exist in a clean Git checkout. The latter currently reports local-only audit documents linked by public indexes; do not publish ignored audits without reviewing their contents and intended audience.

For a question about physics, start in the ledger and the active constitution. For implementation, start in the owning source directory and its focused tests. Recorded campaign data and evidence are provenance, not substitutes for running their source or checking epistemic tags.
