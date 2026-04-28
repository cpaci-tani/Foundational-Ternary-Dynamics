# 0009 — Epistemic tag system (7-tag vocabulary for claims)

**Status:** Accepted (load-bearing for CLAUDE.md epistemic discipline)
**Date:** 2026-04 (retroactive)
**Author:** codified 2026-04-27

## Context

FTD makes a wide range of claims: rigorous theorems (master quadratic
identity), strongly motivated conjectures (x_+ = 1/α to 1.26 ppm),
parametric insertions (electron mass formula coefficients), selection
arguments (D=3 from |Aut(E)|²), open questions (g_c first-principles
derivation), emergent behaviors (Bell violation S = 2√2). Without a
shared vocabulary, claims drift in epistemic status — derivations get
relabeled as theorems, parametric insertions get presented as
predictions. Critique becomes impossible.

## Decision

Adopt a 7-tag vocabulary, used in code comments, scenario descriptions,
documentation, and `LEDGER.md`:

| Tag | Meaning | Reviewer expectation |
|---|---|---|
| **[AXIOM]** | Structural postulate (not derivable) | Accept as model definition |
| **[THEOREM]** | Rigorously proven from axioms | Check proof |
| **[SELECTION]** | Argued from consistency, not uniquely proven | Critique argument |
| **[CONJECTURE]** | Proposed interpretation requiring validation | Demand evidence |
| **[IMPOSED]** | Parameter choice or model calibration | Note as input, not output |
| **[EMERGENT]** | Behavior arising from dynamics (not designed in) | Verify in simulation |
| **[OPEN]** | Unresolved question | Research opportunity |

Every load-bearing claim in `docs/theory/07_assessment/LEDGER.md` carries
one of these tags. Code comments cite LEDGER rows by number
(`// Implements LEDGER#C-NNN [TAG]`).

## Consequences

- (+) Reviewers can immediately calibrate scrutiny level
- (+) Drift between claim status and presentation becomes visible
- (+) AI agents have explicit guardrails: do NOT promote a tag without
  evidence, do NOT silently relabel
- (−) Discipline burden: every new claim must be tagged

## Alternatives considered

- No tagging — rejected: produces silent overclaiming.
- More granular taxonomy — rejected: 7 is the working set; finer slicing
  produces unused categories.

## References

- Files: `CLAUDE.md` §"Epistemic Tags", `docs/theory/07_assessment/LEDGER.md`
- Cross-refs: CONTRACTS.md §5 (cross-reference policy), all DERIV_*.md and
  AUDIT_*.md docs
