# AUDIT — LEDGER FTD-NNNN ID Collisions (cleanup flag)

**Tag:** `[AUDIT FINDING]` — documentation hygiene only. **No claim is promoted, demoted, or
re-tagged here. No renumbering is performed.** This doc *flags* a set of duplicate / over-loaded
`FTD-NNNN` identifiers in `core_ledgers/LEDGER.md` for a dedicated, owner-approved cleanup pass.

**Created:** 2026-06-09
**Why a separate doc:** `docs/WHERE_WE_LEFT_OFF.md` §0.22 has flagged an "FTD-0189 ↔ FTD-0243 ID
double-booking" for "a dedicated cleanup pass." This audit pins down *exactly* what is and is not
confirmable, so the owner can renumber from facts rather than from a one-line memo. Renumbering is
substantive ledger surgery (it touches pre-reg provenance, git tags, and many cross-references) and
is deliberately **not** attempted here, per the project's Documentation Cleanup Discipline ("Do not
promote claims during cleanup"; "Update all navigation layers in the same cleanup").

---

## 1 · CONFIRMED collision — `FTD-0189` is over-loaded (≥ 2 distinct results)

`grep -oE "FTD-0189 \| ..." core_ledgers/LEDGER.md` returns **two row-defining entries**, and the
LEDGER *prose* uses the same ID for a **third, different** result:

| Usage | What it labels | Where | Date stamp |
|---|---|---|---|
| **A** | **Step-0 graviton-provenance audit** — "FTD's massless spin-2 field h_μν is posited, not derived" (`[AUDIT FINDING]`; Conjecture 10.1 / Gap 10.1) | LEDGER row-defining `FTD-0189 \| Step-0 graviton-provenance audit …` | NEW 2026-05-21 |
| **B** | **Graviton-substrate-mode measurement** — PREREG v2 `preregister-graviton-substrate-mode-v2` (commit `bb354b6`); engine L∈{32,64}. The row's own prose says it was *"renumbered to FTD-0193 to resolve a duplicate-id collision with … FTD-0190"* — i.e. the ID column and the prose disagree. | LEDGER row-defining `FTD-0189 \| NEW 2026-05-22 …` | NEW 2026-05-22 |
| **C** | **Adversarial look-elsewhere scan** — master quadratic the unique dual-matcher across 2.65M degree-2 polynomials (the strongest structural evidence for `x₊=1/α`). Per `CLAUDE.md`, this was *"renumbered from FTD-0187 on 2026-05-21."* Referenced as "the FTD-0189 adversarial look-elsewhere scan." | LEDGER prose lines ~362, ~439; `CLAUDE.md` 5.40-era baseline | renumbered 2026-05-21 |

**Diagnosis.** Three independent things (graviton-provenance audit, graviton-substrate-mode run,
look-elsewhere scan) are all attached to the string `FTD-0189`, with a partial/aborted renumber to
`FTD-0193` recorded inside usage **B**'s own row text. The `0187 → 0189` renumber of usage **C**
(noted in `CLAUDE.md`) appears to have *collided* with usages **A/B** rather than resolving cleanly.

**This is a genuine collision** and is the load-bearing half of the WHERE_WE_LEFT_OFF flag.

---

## 2 · NOT confirmable as stated — the alleged `FTD-0243` double-booking

`grep -oE "FTD-0243 \| ..." core_ledgers/LEDGER.md` returns **exactly one** row-defining entry:

> `FTD-0243 | RSI Leg 3 conditional theorem + operator-assembly independence | [THEOREM —
> conditional; OPEN — universal negative 3c] …` (2026-06-01)

All other `FTD-0243` hits in the LEDGER (lines ~256, ~258, ~260) are **inbound cross-references**
from *other* rows (FTD-0251, FTD-0253) pointing back to this single canonical row — not a second
definition.

**Diagnosis.** Within `LEDGER.md`, `FTD-0243` is **not** double-booked. The WHERE_WE_LEFT_OFF
§0.22 phrasing "FTD-0189 ↔ FTD-0243 ID double-booking (each id labels two distinct results)" is
**not confirmable for the 0243 half** from the ledger as it stands. Two readings are possible and
the owner should disambiguate:

1. The "↔" was meant as a *range* ("the 0189–0243 ID tangle"), pointing at the broader cluster of
   renumbering scars in the 0187–0205 window (see §3), not at 0243 specifically; **or**
2. A second `FTD-0243` usage lives in a doc *outside* `LEDGER.md` (an audit/pre-reg) that the
   memo's author had in mind. A corpus-wide `rg "FTD-0243"` pass is the way to settle this.

**Action: confirm before touching 0243.** Do not renumber 0243 on the strength of the memo alone.

---

## 3 · Adjacent renumbering scars in the 0187–0205 window (context, not new collisions)

These are *recorded, resolved-in-prose* renumbers that share the same neighborhood and are worth
reading together during the cleanup, because they explain how the 0189 tangle arose:

- **FTD-0187** — `x₊=1/α` IS the Born-rule consolidation row (per `CLAUDE.md`), yet usage **C**
  above says the look-elsewhere scan was renumbered *from* 0187. Check 0187 is now singular.
- **FTD-0190** — Q10 finite-neutral-lock (`454b2f2`, chronologically first) vs a parallel-session
  draft (`0b06ab6`) renumbered **0190 → 0193**. Recorded in usage **B**'s row prose.
- **FTD-0193** — the canonical home claimed for usage **B**; verify the row that *should* be
  FTD-0193 is not still wearing the FTD-0189 label in column 1.
- **FTD-0198 → FTD-0200** — pre-reg frozen-provenance renumber: the immutable git tag/runner keep
  the literal `FTD-0198` string (registration-time provenance, intentionally not edited); the
  canonical LEDGER ID is `FTD-0200`. **This one is correctly handled** — listed only so the cleanup
  pass does not "fix" it by mistake.

---

## 4 · Recommended cleanup procedure (for the owner, not executed here)

1. **Corpus-wide census first:** `rg -n "FTD-0189|FTD-0193|FTD-0243|FTD-0187|FTD-0190" docs/ engine/
   scripts/ CLAUDE.md` → build the full reference graph before editing anything.
2. **Assign clean IDs** to the three usages of §1 (graviton-provenance audit / graviton-substrate
   mode / look-elsewhere scan). The look-elsewhere scan is the most cross-referenced (papers cite
   it for `x₊=1/α` evidence), so prefer giving it the *stable* canonical ID and moving the graviton
   rows.
3. **Preserve pre-reg/git-tag provenance:** never rewrite a hash-locked registration string (cf. the
   FTD-0198→0200 pattern). Add a "canonical ID = X; registration-time string = Y" note instead.
4. **Update every nav layer in the same commit:** `LEDGER.md`, `TRACKER_OPEN_ITEMS.md`,
   `TRACKER_ONTIC_TRUTH.md`, `META_INDEX.md`, `SPEC_DOCTRINE_LEDGER.md`, `CLAUDE.md` baseline notes,
   and any paper that cites the look-elsewhere scan.
5. **Verify:** `git diff --check` + a re-run of the §1 census `rg` to confirm zero dangling
   old-ID references.

---

*This is an `[AUDIT FINDING]` / cleanup flag. It changes no claim and renumbers nothing. Source of
truth for IDs remains `core_ledgers/LEDGER.md`.*
