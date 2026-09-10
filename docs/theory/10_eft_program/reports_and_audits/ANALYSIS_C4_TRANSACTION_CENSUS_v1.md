# ANALYSIS — The Legacy Engine Does Not Realise the `C₄` Cycle

**Status:** `[MEASURED — SITE-LEVEL TRANSITION CENSUS, DETERMINISTIC]` +
`[CLOSED NEGATIVE — "the legacy engine law realises C₄", at the registered scope]` +
`[MEASURED — POLARITY ASYMMETRY OF THE LEGACY READOUT]` +
`[OPEN — whether v3's selected Φ realises C₄; that is FTD-1028's `hodge_expiry` path, not this run]`
**Date:** 2026-09-10 · **Lock:**
[`PREREG_C4_TRANSACTION_CENSUS_v1.md`](../preregistrations/engine_transaction_census/PREREG_C4_TRANSACTION_CENSUS_v1.md)
(declared 2026-09-04, committed blob SHA256 `4a4cd523a3cb0ce54f64213c2c1f3791decfa2d3fbb18c92d452cb7cb925c533`)
**Instrument:** `engine/tests/test_c4_transaction_census.cpp`
(SHA256 `625f312a361c84fbfc3f5e793fc4541af119cc14cb0414ff9773412c77c98e22`,
CTest target `c4_transaction_census`)
**Evidence:** [`engine/docs/evidence/c4-transaction-census-v1-2026-09-10.json`](../../../../engine/docs/evidence/c4-transaction-census-v1-2026-09-10.json)
(verbatim stdout of both instruments plus machine-parsed statistics; generator
`scripts/experiments/build_c4_census_evidence.py`)
**Parents:** FTD-1027 (schedule-type census, §F5), FTD-1028 (which `Φ` is canonical).
**Production impact:** none. No constant changed, no toggle added, no tag moved.

---

## 0. What was run, and what the record can and cannot witness

The lock declares the run: `L = 16`, `4000` ticks, CPU backend, default
toggles, the deterministic multi-patch seed `langevin_seed = seed_rng =
424242`, nothing tuned. The instrument executes exactly that, reads the
history journal, reconstructs site-level readout transitions, and computes
every statistic in code (lock D0.2).

Executed here on a Linux CPU-only Release build (Ninja, GCC 13.3.0, `-DFTD_ENABLE_CUDA=OFF`).
The instrument calls `force_cpu()`, so the backend is the registered one.
Three independent replays produced **byte-identical stdout**
(SHA256 `14a552a4c72ec27f142c3ede5c63d0266b249394a69aaa922f38e15e4e475bab` ×3).
Through CTest: `c4_transaction_census ... Passed, 58.29 s`. The exit status
reports instrument validity, not the theory verdict, as the instrument itself
declares.

**Lock D0.1 confirmed by cross-check.** The lock requires that the run under
test is the run that produced FTD-1027's "min complete cycle 36 ticks". The
sibling `test_history_journal_drain` was built and run from the same tree and
reports `complete +1<->0<->-1 cycles: count=10 min=36 median=111.5`, and
`reconstructed nulls count=255052` — the same null population the census
reconstructs. The two instruments see one run.

**One limitation, stated plainly.** The lock declares itself LOCK 2026-09-04
"on branch `integration-ptime-tx`, uncommitted at lock time". Lock and
instrument entered git together in the 2026-09-08 bulk import (`cc81ad4`), and
the lock carries no git tag and no entry in `REF_PREREGISTER_MANIFEST.md`. Git
therefore does **not** independently witness that the gates were fixed before
the first run; that rests on the lock's own declaration. The gates are
nonetheless unambiguous, and this run did not touch them.

## 1. Result: `C₄`-NOT-REALISED

| id | statistic | gate | measured | verdict |
|---|---|---|---|---|
| **T1** | reversal fraction at reconstructed nulls | `≥ 0.5` | **0.000180355** | **FAIL** |
| **T3** | fraction of polarity reversals that skip the null | `< 0.1` | **0.570093** | **FAIL** |
| **T2a** | χ² of half-cycle lengths mod 4 / mod 8 | `> 11.345` / `> 18.475` | n = 25 | underpowered, no verdict |
| **T2b** | χ² of positive null dwell times mod 4 / mod 8 | same | **85.54** / **360.36** | PASS |

Per the lock's Outcomes section, `T1` or `T3` failing gives
**`C₄`-NOT-REALISED**, whatever T2 does. Both fail, and neither is marginal:
T1 misses its gate by more than three orders of magnitude, T3 by a factor of
5.7 in the wrong direction. The instrument prints the verdict itself.

This is the outcome the lock declared **expected** at D0.4, on the strength of
FTD-1027 §F5 (the engine's reversal event, `WeakTransmutation`, is an in-place
sign flip). The census turns that reading from one observed mechanism into a
counted population.

## 2. What the numbers say, claim by claim

**(B) the null is a passage, not a bounce — refuted for this law.** Of 255,052
reconstructed nulls, **46** exit to the opposite polarity (the `C₄` passage)
and **255,006** return to the polarity they came from. The null in the legacy
engine is overwhelmingly a *bounce*: a site vacates and is re-occupied with the
same sign. It is a latency, not a phase of an oriented cycle.

**(A) reversal passes through the null — refuted for this law.** Of 107 polarity
reversals in the whole run, **61 skip the null entirely** (in-place `+s → −s`)
and 46 go through it. And the 61 is not a coincidence of counting: the sibling
instrument reports exactly **61 `WeakTransmutation` events** over the same run.
Every null-skipping reversal in this run is a `WeakTransmutation`, one to one.
FTD-1027 §F5's mechanism accounts for the entire skip population.

**(C) a characteristic period — one-sided evidence, and it does not rescue the
verdict.** T2b is decisive on its own terms: dwell times mod 4 give
χ² = 85.54 against a critical 11.345, mod 8 give χ² = 360.36 against 18.475,
on n = 253,002. Dwell times are not uniform on residues. But the lock's
Outcomes make T2 conditional on T1 ∧ T3, and rightly: a period in the dwell
distribution of a *bounce* is a statement about re-occupation latency, not
about a four-phase oriented cycle. T2a, the statistic that would speak to
cycle length, is underpowered — 25 half-cycles against a required 30, and only
10 complete cycles exist in the run.

**Read T2b's structure carefully.** The mod-4 counts are
`[61720, 64847, 63726, 62709]`, a spread of about ±2.5 % around the mean. The
mod-8 counts `[29969, 33784, 32724, 32334, 31751, 31063, 31002, 30375]` fall
monotonically after residue 1. At n = 253,002 a χ² of 360 needs no large
effect. This is a small, highly significant departure from uniformity, and the
shape (a decaying tail, not a period-4 comb) is what a geometric-like dwell
distribution with a short-lag deficit produces. D2 supports that reading:
mean dwell 53.0 ticks, CV 1.149, with 181,625 of 255,052 dwells in the `17+`
bin. Nothing here licenses "period 4" or "period 8" as a physical claim.

## 3. The finding the lock did not ask for: the readout is polarity-asymmetric

D1 is descriptive in the lock, and it carries the sharpest number in the run:

| transition | count |
|---|---|
| `+ → 0` | 258,539 |
| `− → 0` | **38** |
| `0 → +` | 259,133 |
| `0 → −` | **15** |
| `+ → −` in place | 42 |
| `− → +` in place | 19 |

Negative-polarity manifestation is rarer than positive by roughly **four
decimal orders**, even though the seed alternates patch sign by construction
(`peak = ±2.2 · K_GENESIS`, `patch_index % 2`). The legacy engine at this
operating point populates one polarity and essentially not the other.

This is a *mechanism* for T1's failure, and it makes the failure structural
rather than statistical. A null entered from `+1` cannot exit to `−1` more
than 15 times in the run, because `0 → −` happens 15 times in total. The
`C₄` premise presupposes a two-polarity population; this run does not have one.
The arithmetic is consistent: 46 passages ≤ 15 (`+`-entered, exiting `−`) + 38
(`−`-entered, exiting `+`) = 53.

**Why** the asymmetry exists is not settled by this run and is not in scope
here. It is booked `[OPEN]`: either the negative patches decay before
manifesting, or the manifestation readout is sign-biased. A follow-on should
answer it before any further reading of the legacy engine's temporal structure,
because a one-polarity substrate cannot test any claim about polarity cycles.

## 4. What this closes, and what it does not

**Closes.** "The legacy engine law realises `C₄`" — at the registered scope
(`L = 16`, 4000 ticks, default toggles, that seed). The engine's nulls are not
oriented passages, and its reversals predominantly skip the null.

**Does not touch.** The algebraic content. `G*` from the quarter-sector
determinant is a theorem about an algebraic object and holds regardless of what
any engine does. The lock says this in its own closing section; it is repeated
here so the row cannot be mis-cited.

**Does not decide.** Whether v3's selected `Φ` realises `C₄`. v3's named expiry
acts on oriented `(normal, hand)` presentations, and nothing in this run
touches it. That is FTD-1028's `hodge_expiry` retirement path. `grep
hodge_expiry` over `engine/` returns no source hit today: the toggle does not
exist, so the comparison the retirement path calls for cannot yet be run.

**Reading rule (FTD-1028).** The engine is a probe of a legacy `Φ`, not of
v3's. A negative here is a statement about the probe. It does add one thing to
the FTD-1028 ledger line, though: the gap between the engine's law and v3's is
now measured on a specific structural claim, not only asserted. The engine's
temporal transaction structure is *not* the `C₄` cycle, and the priced gap is
that much more concrete.

## 5. Reproduction

```sh
cmake -S engine -B build-cpu -G Ninja -DCMAKE_BUILD_TYPE=Release -DFTD_ENABLE_CUDA=OFF
cmake --build build-cpu --target test_c4_transaction_census test_history_journal_drain
./build-cpu/test_c4_transaction_census        > run/c4_census_run1.txt
./build-cpu/test_history_journal_drain        > run/journal_drain_run1.txt
python3 scripts/experiments/build_c4_census_evidence.py run .
```

The evidence file embeds both stdouts verbatim, so every number above can be
checked against the bytes the instrument printed.

---

**Draft ledger row:** see
[`LEDGER_ROW_DRAFT_c4_transaction_census.md`](../../07_assessment/core_ledgers/LEDGER_ROW_DRAFT_c4_transaction_census.md).
Booking is an owner act; this analysis books nothing.
