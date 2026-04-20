# Audit Report Template

Use this template when the classifier agent produces its per-artifact audit output. The file name convention is `AUDIT_<artifact_name>.md` placed in a directory mirroring the artifact's location.

```markdown
# Audit: <artifact path>

**Classifier agent run date**: <YYYY-MM-DD>
**Canonical reframe version**: <version from CANONICAL_REFRAME.md>
**Artifact type**: paper | derivation_note | engine_source | ledger | outreach | deck | other
**Current portfolio status**: draft | published | archived | working

## Summary
- Total passages examined: <n>
- Findings: <total>
  - PROSCRIBED: <count>
  - PERMITTED (flagged for record): <count>
  - AMBIGUOUS: <count>

## Findings

## Finding 1
- **Location**: <section, paragraph, line number, or equation reference>
- **Passage**: 

> <verbatim quote, 1-3 sentences>

- **Classification**: PROSCRIBED | PERMITTED | AMBIGUOUS
- **Rule invoked**: <rule number and brief quote from CANONICAL_REFRAME.md>
- **Reasoning**: <apply Questions 1-4 from canonical doc>
- **Proposed triage action**: SURVIVES | RESTATE | RE-DERIVE | RETRACT
- **Dependencies**: <claims that depend on or support this one>
- **Risk if unfixed**: LOW | MEDIUM | HIGH

## Finding 2
<same structure>

[...]

## Artifact-level notes
- <observation about the artifact's structure>
- <cross-references to check in parallel audits>
- <anything special about this artifact's role in the portfolio>

## Classifier self-check
- Every finding has all seven fields: YES | NO
- Cross-reference search was performed: YES | NO
- Ambiguous findings have been explicitly flagged: YES | NO
```
