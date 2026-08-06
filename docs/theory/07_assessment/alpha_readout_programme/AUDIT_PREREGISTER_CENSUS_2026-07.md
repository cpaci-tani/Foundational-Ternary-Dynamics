# AUDIT — Pre-registration Tag Census (Arc 1 "Honest Mint", 2026-07-12)

**Tag:** [CRITIC SYNTHESIS / RECONCILIATION] — the first mechanical three-way reconciliation of the pre-registration registry: `git tag -l "preregister-*"` ↔ `REF_PREREGISTER_MANIFEST.md` ↔ docs/theory tag citations. **Promotes nothing; retraction is demotion-only.** Instrument: `tools/preregister_census.py` (names-only reconciliation — cannot be gamed by prose); dispositions: `docs/theory/10_eft_program/preregister_census_dispositions.json`. Standing gate per the Consumption Program charter (FTD-0383 AM-4) and LOCK-STD.

## §1 · Before-state (the first run, 2026-07-12)

**RED: 70 undispositioned failures** out of 114 tag-names seen anywhere — 34 ORPHAN-TAG (tag in git, no manifest row), 30 CITED-NONEXISTENT (cited in docs, tag never cut), 4 MANIFEST-GHOST (manifest row, no tag), plus 2 regex artifacts fixed in-run (tags containing capitals, e.g. `-L257-`, truncated by the first pattern). The manifest — self-described "single authoritative table mapping **every** pre-registered FTD measurement" — was not closed over the tag namespace. F10 (in both directions) was unauditable before this instrument existed.

## §2 · The counterfeit class (verdicts on locks that never existed)

1. **FTD-0217 / FTD-0218 — RETRACTED (owner ruling 2026-07-12, FTD-0042 precedent).** `PREREG_COLOR_CONFINEMENT_v1.md` and `PREREG_STOCHASTIC_EFFECTIVE_ACTION_v1.md` declared hash-lock tags that were never created and recorded **no content SHA**; `FOUND_COLOR_CONFINEMENT_RESOLUTION.md` and `FOUND_STOCHASTIC_EFFECTIVE_ACTION_RESOLUTION.md` booked [THEOREM]-grade FOUND verdicts on them, with no LEDGER rows (the provisional ids were reassigned per FTD-0232). FTD-0217's "confinement proven natively" additionally contradicts the confinement structural obstruction of record (FTD-0025) and the charter's own Front-D P5 expectation. Correction banners applied (see the retraction commit); the retraction is prior art for the future P5 confinement priced-no-go.
2. **Four tag-claim defects with RECOVERED content locks — repaired by late anchor, not retraction.** `FOUND_ACT_REDUCTION_COUNT.md`, `FOUND_ARROW_DIRECTION.md`, `FOUND_SM_ACT_COUNT.md`, `FOUND_MCT43_NATIVE_Z2_PERMANENCE.md` each claim "git tag `preregister-…`" that did not exist — but each recorded a prereg SHA256, and **git archaeology recovered every recorded SHA at a committed historical revision** (`99811f92`, `62df90cd`, `16a06a92`, `5fdaf9b5` respectively — line-ending-normalized matching). The protocol content was verifiably committed before the verdicts; only the tag-cutting step was skipped. Tags are now cut **at the recovered commits** (accurate anchoring). Disclosure of record: the tags did not exist at verdict time; the content locks did. FTD-0326 (which leans on the MCT43 permanence result) is therefore **unaffected in substance**, with this provenance note attached.

## §3 · The reconciliation actions (all mechanical, all disclosed)

- **24 late-anchor tags cut** at registration/recovered commits (the four §2.2 recoveries; the 4 manifest-ghosts incl. FTD-0185's `alpha-arithmetic-generativity`; the 5 executed alpha-readout ARC preregs FTD-0230/0231/0234/0235/0239; the written-but-unrun `damerell-scan-v1` — now properly anchored for its Arc-2 execution; and 10 further declared-but-uncut preregs). A late anchor is booked as `anchored-late`, never presented as an original lock.
- **64 dispositions**: 24 `executed-verdict-booked` (tag + verdict doc exist; manifest row owed via the §5 addendum) · 20 `anchored-late` · 11 `arc2-disposition-pending` (grandfather clause — Arc-2 run/retire/re-lock adjudication) · 7 `historical-superseded` (archived campaign/superseded/prose artifacts) · 2 `retracted`.
- **After-state: CENSUS GREEN** — 104 tags, 50 fully-reconciled OK, 64 dispositioned, 0 undispositioned.

## §4 · The 11 arc2-disposition-pending items (the honest residue)

Tags or citations whose execution status could not be adjudicated mechanically this pass — each gets an Arc-2 disposition row (run / retire / re-lock): `adversarial-look-elsewhere-v1`, `alpha-det-forcing-v2`, `born-equilibrium-preservation-v1`*, `chowla-selberg-higher-h-scan-v1` (relate to the Damerell Arc-2 execution), `polynomial-scan-extended-v1`*, `tower-level-scan-v1`*, and the remainder flagged in the dispositions file. (* = archived closed-negative/measurement docs exist but sit in archive paths the mechanical verdict-doc filter excludes — expected to resolve to `executed-verdict-booked` on human confirmation.)

## §5 · Standing obligations created

1. The census runs at every arc gate; RED blocks lock-cutting (charter AM-4).
2. The manifest gains a **census addendum section** pointing at the dispositions JSON as the closure of the tag namespace (rows for the 24 `executed-verdict-booked` items are owed there; the JSON is authoritative until each row is written).
3. Known double-booking (FTD-0284: LEDGER row = D=3 forced-escape vs manifest = dynamical-readout discriminator) and FTD-0107-G2 adjudication debt are booked in the FTD-0384 reconciliation row.
4. Methodology disclosure: the census reconciles **names**, not semantics; `executed-verdict-booked` was assigned by mechanical detection of non-archived FOUND/ANALYSIS/REPORT/AUDIT citers — a per-item human confirmation pass rides Arc 2.

*Zero promotions. x₊=1/α [SMC]; MC-T4.3 [FOUNDATIONAL OBSTRUCTION]; the α-wall chain (incl. FTD-0326) unaffected in substance, provenance sharpened. Golden untouched.*
