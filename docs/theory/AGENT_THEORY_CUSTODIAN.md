# Agent: FTD Theory Custodian

**Purpose:** Living theory knowledge agent for the FTD corpus. Semantic correction of conflations and inaccuracies — **not** replacing quantum mechanics or general relativity.

## Installation

Personal Cursor skill (not in this repo):

```
C:\Users\cpaci\.cursor\skills\ftd-theory-custodian\
```

Invoke by name when working on `docs/theory/`, claim status, epistemic tags, navigation, conflation audits, or recent theory changes.

## Session Ritual

From the FTD repo root:

```powershell
python scripts/theory/sync_theory_briefing.py --since 7d
```

Optional cache (gitignored):

```powershell
python scripts/theory/sync_theory_briefing.py --since 7d --output docs/internal/theory_briefing_latest.md
```

Then read `docs/WHERE_WE_LEFT_OFF.md` header and `LEDGER.md` for any specific `FTD-NNNN` claim.

## Canonical Authority

| Question | Document |
|----------|----------|
| Claim tag | `07_assessment/core_ledgers/LEDGER.md` |
| Truth tier | `07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md` |
| Framework commitments | `01_reference/SPEC_FTD_FRAMEWORK_V1.md` |
| Hierarchy rules | `META_STRUCTURE.md` |

**Conflict precedence:** LEDGER > constitution > other prose.

## Related Agents (not replaced)

| Agent | Role |
|-------|------|
| **ftd-theory-custodian** (this) | Theory corpus knowledge, freshness, conflation audits |
| **epistemic-auditor** | Periodic tag-coverage / META_INDEX sync sweep |
| **constants-sentinel** | `constants.py` / `ontic.h` / `constants.js` drift |

See `META_STRUCTURE.md` § Periodic consistency check.

## Skill Contents

- `SKILL.md` — charter, ritual, forbidden moves, output modes
- `reference/` — hierarchy, read-order, tags, clusters, semantic-correction playbook
- `examples/query-patterns.md` — worked response templates

## Epistemic Discipline (inherited)

- No near-miss numerical searches
- No substitution identities
- No parametric insertions labeled as derivations
- No silent tag promotion
