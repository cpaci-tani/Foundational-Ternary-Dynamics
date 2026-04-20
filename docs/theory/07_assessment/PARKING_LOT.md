# Parking Lot — Reframe Deployment Items Deferred to Future Sessions

**Status:** items the owner has explicitly parked, not items that are silently waiting. Each item has been triaged and is **deliberately deferred** — not forgotten. Anyone resuming reframe work should consult this list to understand what was intentionally left out of the current pass.

**Maintenance rule:** items move out of this list either when (a) the owner explicitly returns them to active work, or (b) a future session promotes them via an entry in `CHANGELOG_REFRAME.md`. Items should not silently leave the parking lot.

---

## Parked 2026-04-19 (Session 3)

### 3 — Riemann hypothesis paper (`FTD_Riemann_Hypothesis.tex`)

- **Status:** PARKED.
- **Background:** classifier flagged this paper for "deeper user read for latent completed-infinity reasoning that doesn't surface as keywords" (`FLAGGED_PASSAGES_PAPERS.md` methodological caveat). The Riemann hypothesis itself involves the analytic continuation of ζ(s) over the completed complex plane — possibly proscribed in its standard formulation.
- **Why parked:** owner judgment needed on whether to (a) re-derive in finitary terms, (b) demote to conjecture-paper status, or (c) retract entirely (parallel to YM/NS).
- **Where to start:** read `docs/papers/speculative/FTD_Riemann_Hypothesis.tex` end to end with `CANONICAL_REFRAME.md` Q1–Q4 in mind. Decide disposition.
- **Estimated effort:** 4–8 hours of owner reading + judgment.

### 5 — Manuscript v1 + v2 reframe sweep (~175 chapters across 4 locations)

- **Status:** PARKED.
- **Scope:** `dissemination/manuscript/src/chapters/` (92 files, v1) + `dissemination/manuscript_v2/src/chapters/` (83 files, v2 consolidated) + `dissemination/manuscript_v2/vol1/src/chapters/` (35 files) + `dissemination/manuscript_v2/vol2/src/chapters/` (45 files).
- **Why parked:** large mechanical sweep; not blocking the foundational reframe; can be dispatched in parallel-agent batches when convenient.
- **Pre-conditions before resuming:** `dissemination/manuscript_v2/PROPAGATION_RULE.md` (already written this session) must be followed; the manuscript divergence audit (item 8 below) should run first to baseline the v1/v2/vol1/vol2 state.
- **Estimated effort:** 10–20 owner hours + 30–50 agent hours.

### 6 — Whitepaper reframe (`dissemination/whitepaper/FTD_Whitepaper.tex`)

- **Status:** PARKED.
- **Why parked:** publication-grade document; reframe edits should be substantive (not mechanical) given the whitepaper's audience.
- **Pre-conditions before resuming:** establish the calibration narrative (already done this session — `a_phys ≡ ℓ_P` is declared); verify the whitepaper's claims align with the new LEDGER status (FTD-0013 / FTD-0014 / FTD-0030 / etc.).
- **Estimated effort:** 4–8 owner hours + 4–8 agent hours.

### 7 — Notebooks + interactive HTML reframe

- **Status:** PARKED.
- **Scope:** `dissemination/notebooks/` (12 Jupyter notebooks) + `dissemination/interactive/` (8+ HTML simulations).
- **Why parked:** notebooks are interactive demonstrations rather than load-bearing claim sources; HTML simulations are pedagogical. Reframe-compatibility checks would be quick but the effort is not blocking foundational work.
- **Estimated effort:** 4–6 owner hours + 4–8 agent hours.

### 8 — Manuscript divergence audit (vol1/vol2 vs src/chapters)

- **Status:** PARKED.
- **Background:** the propagation rule (`PROPAGATION_RULE.md`) confirmed via spot-check that vol1/src/chapters and src/chapters have diverged at least for `01-five-postulates.qmd`. The full divergence count is unknown.
- **Recommended command:** see `PROPAGATION_RULE.md` §"Known divergences" for the diff-sweep loop.
- **Why parked:** purely mechanical audit; can be done in 30 min once item 5 (manuscript reframe) is ready to begin.
- **Estimated effort:** ~1 owner hour to run + classify divergences.

### 9 — `α_largeL` empirical residual band emission (engine TODO)

- **Status:** PARKED.
- **Context:** the `α_inf` rename agent (Session 2) planted a TODO comment in `engine/tests/benchmark_dynamical_sm.cpp` to emit an empirical residual band for `α_largeL` once a multi-seed ensemble exists. The current single-seed run produces a point estimate; the ensemble run produces the band.
- **Why parked:** requires multi-seed ensemble run (compute time, not analytical work); not blocking the reframe.
- **Estimated effort:** ~2 hours engine compute + 1 hour to wire up the CSV column.

---

## Items already moved out of parking lot

(Append rows when items leave parking. Do not silently delete rows; mark with date moved.)

| Item | Date parked | Date moved out | Disposition |
|---|---|---|---|
| (none yet) | | | |

---

## Reading sequence when revisiting

1. Read this file.
2. Confirm the reasoning for each item is still valid (priorities may have shifted).
3. Move items back to active work via a `CHANGELOG_REFRAME.md` entry naming the move + the planned approach.
4. Update the table at the top of this file with the move-out date.
