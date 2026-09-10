# LEDGER row draft: the `C₄` transaction census on the legacy engine law

Date: 2026-09-10. Draft only — **not booked into `LEDGER.md`**; booking is the
owner's act. This row records a locked, previously unreported experiment: the
instrument and its pre-registration were both committed on 2026-09-08 and the
run had never been executed or written up. Nothing here changes canonical Φ-v2,
v3, the engine, or any constant.

Source analysis:
[`docs/theory/10_eft_program/reports_and_audits/ANALYSIS_C4_TRANSACTION_CENSUS_v1.md`](../../10_eft_program/reports_and_audits/ANALYSIS_C4_TRANSACTION_CENSUS_v1.md).
Lock:
[`PREREG_C4_TRANSACTION_CENSUS_v1.md`](../../10_eft_program/preregistrations/engine_transaction_census/PREREG_C4_TRANSACTION_CENSUS_v1.md).
Parents: FTD-1027 (§F5, the engine's reversal event is an in-place sign flip),
FTD-1028 (the engine is a probe of a legacy `Φ`, not v3's).

**Next-free id.** `python scripts/audit/check_registry.py` (run 2026-09-10 from
the worktree root) reports:

```text
FTD ids referenced : 1034  (max FTD-1040)
NEXT FREE FTD ID   : FTD-1041   <- use this, not CLAUDE.md
FC register exists : ['FC-0', 'FC-1', 'FC-2', 'FC-3', 'FC-4']
RESULT: clean
```

The max FTD-1040 already counts the three unbooked hydro draft files
(FTD-1029–1033, FTD-1034–1037, FTD-1038–1040), which do not overlap. The single
id below (FTD-1041) is proposed for the owner; **re-run `check_registry.py`
immediately before booking**.

---

| FTD-1041 | **Does the legacy engine's transaction structure realise the `C₄` cycle `+1 → 0↓ → −1 → 0↑ → +1`?** | [CLOSED NEGATIVE at the registered scope] (T1 and T3 both fail) + [MEASURED] (the transition census, the dwell residue structure, and the polarity asymmetry) + [OPEN] (the cause of the polarity asymmetry; and whether v3's selected `Φ` realises `C₄`, which is FTD-1028's `hodge_expiry` path and is untouched here) | NEW 2026-09-10 — analysis [`ANALYSIS_C4_TRANSACTION_CENSUS_v1.md`](../../10_eft_program/reports_and_audits/ANALYSIS_C4_TRANSACTION_CENSUS_v1.md), lock [`PREREG_C4_TRANSACTION_CENSUS_v1.md`](../../10_eft_program/preregistrations/engine_transaction_census/PREREG_C4_TRANSACTION_CENSUS_v1.md) (declared LOCK 2026-09-04, committed blob SHA256 `4a4cd523a3cb0ce54f64213c2c1f3791decfa2d3fbb18c92d452cb7cb925c533`), instrument `engine/tests/test_c4_transaction_census.cpp` (SHA256 `625f312a361c84fbfc3f5e793fc4541af119cc14cb0414ff9773412c77c98e22`, CTest target `c4_transaction_census`), evidence `engine/docs/evidence/c4-transaction-census-v1-2026-09-10.json` (verbatim stdout of the census and of the sibling `test_history_journal_drain`, plus machine-parsed statistics; generator `scripts/experiments/build_c4_census_evidence.py`). Registered run, nothing tuned: `L = 16`, 4000 ticks, CPU backend (`force_cpu`), default toggles, `langevin_seed = seed_rng = 424242`. Executed on a Linux CPU-only Ninja/GCC-13.3.0 Release build with `FTD_ENABLE_CUDA=OFF`; three independent replays byte-identical (stdout SHA256 `14a552a4c72ec27f142c3ede5c63d0266b249394a69aaa922f38e15e4e475bab` ×3); through CTest `Passed` in 58.29 s, the exit status reporting instrument validity rather than the theory verdict as the instrument declares. **Lock D0.1 confirmed**: the sibling `test_history_journal_drain` on the same tree reports `complete cycles count=10 min=36 median=111.5` and `reconstructed nulls count=255052`, so the run under test is the run that produced FTD-1027's cited "min cycle 36 ticks". **T1 (claim B, the null is an oriented passage), gate reversal fraction ≥ 0.5: FAIL at 0.000180355** — of 255,052 reconstructed nulls, 46 exit to the opposite polarity and 255,006 return to the polarity they came from; the legacy null is a re-occupation latency, not a phase of a cycle. **T3 (claim A, reversal passes through the null), gate skip fraction < 0.1: FAIL at 0.570093** — of 107 polarity reversals in the run, 61 are in-place `+s → −s` and 46 pass through the null; the sibling instrument counts exactly **61 `WeakTransmutation` events**, so every null-skipping reversal in this run is a `WeakTransmutation`, one to one, and FTD-1027 §F5's mechanism accounts for the entire skip population. **T2a (claim C, characteristic cycle period): underpowered, no verdict per the lock** — 25 half-cycles against a required 30, and only 10 complete cycles exist. **T2b (dwell residues): PASS** — positive null dwell times mod 4 give `χ² = 85.54` (critical 11.345) and mod 8 give `χ² = 360.36` (critical 18.475) on n = 253,002; but the lock makes T2 conditional on T1 ∧ T3, and the departure is small in effect (mod-4 counts `[61720, 64847, 63726, 62709]`, ±2.5 % about the mean; mod-8 counts decay monotonically after residue 1 rather than forming a period-4 comb) and describes bounce latency, not cycle length — **do not cite it as "period 4" or "period 8"**. D2: mean dwell 53.0 ticks, CV 1.149, 181,625 of 255,052 dwells in the `17+` bin. **Verdict per the lock's Outcomes section: `C₄`-NOT-REALISED.** **Additional [MEASURED] finding, outside the lock's gates (D1 is descriptive there)**: the legacy readout is polarity-asymmetric by roughly four decimal orders — `+ → 0` 258,539 vs `− → 0` **38**, and `0 → +` 259,133 vs `0 → −` **15** — even though the multi-patch seed alternates sign by construction (`peak = ±2.2 · K_GENESIS`). This is a structural mechanism for T1's failure, not a statistical one: a null entered from `+1` cannot exit to `−1` more than 15 times in the run because `0 → −` occurs 15 times in total (and 46 passages ≤ 15 + 38 = 53, consistent). Whether the negative patches decay before manifesting or the manifestation readout is sign-biased is `[OPEN]` and should be answered before any further reading of the legacy engine's temporal structure, since a one-polarity substrate cannot test a claim about polarity cycles. **Scope, stated as the lock states it**: this closes "the legacy engine law realises `C₄`" at the registered scope. It does **not** touch the algebraic content (`G*` from the quarter-sector determinant is a theorem regardless of what any engine does), and it does **not** decide whether v3's selected `Φ` — whose named expiry acts on oriented `(normal, hand)` presentations — realises `C₄`; that is FTD-1028's `hodge_expiry` retirement path, and `grep hodge_expiry` over `engine/` returns no source hit, so that toggle does not exist and that comparison cannot yet be run. What the row does add to FTD-1028 is that the legacy-vs-v3 gap is now **measured on a specific structural claim** rather than asserted. **One provenance limitation, recorded rather than smoothed over**: the lock declares itself LOCK 2026-09-04 "on branch `integration-ptime-tx`, uncommitted at lock time"; lock and instrument entered git together in the 2026-09-08 bulk import (`cc81ad4`), the lock carries no git tag and no `REF_PREREGISTER_MANIFEST.md` entry, so git does not independently witness that the gates were fixed before the first run — that rests on the lock's own declaration. The gates are unambiguous and this run did not touch them. No production change, no constant changed, no toggle added, no tag moved elsewhere. |

Evidence hashes are of the committed LF-normalized blobs (`.gitattributes`
`*.json text eol=lf`); a Windows working copy carries CRLF and hashes
differently.
