# PRE-REGISTRATION — does the engine's transaction structure realise the `C₄` cycle? (v1)

**Date:** 2026-09-04   **Status:** LOCK, on branch `integration-ptime-tx`, uncommitted at lock time.
**Instrument:** `engine/tests/test_c4_transaction_census.cpp` (native CTest, CPU backend,
default toggles, deterministic seed) reading the history journal (FTD-1027/1028 tracker).
**Theory under test:** the temporal cycle is `C₄`: `+1 → 0↓ → −1 → 0↑ → +1`. Its
content, stated as engine-checkable claims about site-level readout transitions:

- **(A) Reversal passes through the null.** A polarity reversal is never `+s → −s` in one
  step; it is `+s → 0 → −s`. (The premise that generates `C₄` and hence `G*`.)
- **(B) The null is a passage, not a bounce.** A site that enters `0` from `+s` exits to
  `−s` (the oriented continuation), not back to `+s`.
- **(C) The cycle has a characteristic period.** Four stages (or eight, with the
  half-step the eighth root `√i` implies), so transition timings carry period-4 or
  period-8 structure.

## D0 — Binding declarations

- **D0.1** Default toggle set, `L = 16`, `4000` ticks, the same deterministic multi-patch
  seed as `test_history_journal_drain.cpp` (`langevin_seed = seed_rng = 424242`). Nothing
  tuned. The run that produced FTD-1027's "min cycle 36 ticks" is the run under test.
- **D0.2** Every statistic is computed from journal rows by code; nothing transcribed.
- **D0.3** Gates and thresholds below are fixed before the first run. No re-gating.
- **D0.4** This tests the **legacy engine law** (FTD-1028: the engine is a probe of a
  legacy `Φ`, not v3's). A negative here is a statement about the probe, not about the
  theory's coherence — and is expected, since FTD-1027 §F5 already found the engine's
  reversal event (`WeakTransmutation`) is an in-place sign flip.

## Statistics

| id | statistic | claim tested | gate |
|---|---|---|---|
| **T1** | over all reconstructed nulls: fraction that exit to `−fromState` (reversal) vs `+fromState` (bounce) | (B) | reversal fraction **≥ 0.5** passes |
| **T2a** | χ² of half-cycle lengths (birth → next opposite-sign birth, same site) on residues mod 4 and mod 8 against uniform | (C) | `χ² > 11.345` (df 3, p<0.01) or `> 18.475` (df 7) passes |
| **T2b** | same on positive null dwell times | (C) | same critical values |
| **T3** | of all polarity reversals (in-place `+s→−s` rows + null-passing reversals), fraction that **skip** the null | (A) | skip fraction **< 0.1** passes |
| D1 | transition-type census (`+→0, −→0, 0→+, 0→−, +→−, −→+`) | descriptive | — |
| D2 | dwell histogram (bins 0,1,2,3,4,5–8,9–16,17+) and CV | descriptive | — |

Dropped as tautological: "null orientations alternate within a complete cycle" — cycle
detection requires births `(s, −s, s)`, which forces the two intervening nulls to be
entered from `s` and `−s` respectively. It cannot fail and therefore tests nothing.

## Outcomes (declared)

- **C₄-realised**: T1 ∧ T3 pass, and T2a or T2b passes.
- **C₄-not-realised**: T1 or T3 fails. Reading: the probe law's nulls are not oriented
  passages and/or its reversals skip zero — the `C₄` premise is not what this engine
  does, consistent with FTD-1027 §F5 and FTD-1028.
- **Period-only**: T1 ∧ T3 pass but no T2 passes. Reading: oriented passages without a
  characteristic period — `C₄` as structure, not as clock.
- **Underpowered**: fewer than 30 half-cycles or fewer than 100 positive dwells. T2 is
  then reported without a verdict.

## What a negative does and does not mean

A negative closes "the legacy engine realises `C₄`". It does not touch the algebraic
content (`G*` from the quarter-sector determinant is a theorem regardless of what any
engine does) and it does not decide whether v3's selected `Φ` — whose named expiry acts
on oriented `(normal, hand)` presentations — realises it; that is the `hodge_expiry`
retirement path of FTD-1028, not this run.
