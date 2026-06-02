# Per-Paper Checklist

Use this checklist for each paper (or engine source file, derivation note, etc.) as it passes through the deployment phases.

## Phase 2: Classification
- [ ] Classifier agent has run on the artifact.
- [ ] `AUDIT_<artifact>.md` exists.
- [ ] Every section of the artifact was examined (spot-check).
- [ ] Findings have all seven required fields.
- [ ] AMBIGUOUS findings are explicitly flagged.

## Phase 3: Triage
- [ ] Artifact's findings appear in `FLAGGED_PASSAGES.md`.
- [ ] Each finding has an assigned triage action in `TRIAGE.md`.
- [ ] High-priority findings are in `TRIAGE_ATTENTION.md` and have been reviewed first.
- [ ] AMBIGUOUS findings have been resolved through human judgment.

## Phase 4: Restatement / Re-derivation
- [ ] Every RESTATE finding has a restatement output.
- [ ] Every RE-DERIVE finding has a re-derivation output (success, modified, or failure).
- [ ] Every RETRACT finding has a retraction proposal.
- [ ] Restatement/re-derivation outputs are stored in `restatements/` keyed by finding ID.
- [ ] Failed re-derivations are honest (labeled CANNOT RESTATE or FAILED).

## Phase 5: Engine Audit (if artifact is code)
- [ ] Engine Audit agent has run on the file.
- [ ] `ENGINE_AUDIT_<file>.md` exists.
- [ ] Parameter-free check is filled in.
- [ ] HIGH risk findings are in the summary.
- [ ] Every numerical constant traced.

## Phase 6: Integration
- [ ] Devil's Advocate has reviewed every restatement for this artifact.
- [ ] All reviews resulted in APPROVED (items needing SEND BACK returned to Phase 4).
- [ ] Consistency agent ran in per-paper mode.
- [ ] `CONSISTENCY_<artifact>.md` reports no unresolved issues for this artifact.
- [ ] Ledger agent updated ledger entries for this artifact's claims.
- [ ] `LEDGER_CHANGELOG.md` has entries for every update from this artifact.

## Phase 7: Verification (applies in the final portfolio-wide pass)
- [ ] Artifact was included in the portfolio-mode consistency check.
- [ ] No cross-paper issues involve this artifact unresolved.
- [ ] Artifact's citations still resolve to claims with appropriate tags.

## Artifact-level exit
- [ ] You have read the post-reframe version of the artifact end-to-end.
- [ ] The artifact reads coherently.
- [ ] No passages that should have been flagged were missed (spot check).
- [ ] Cross-references to other papers work.
- [ ] Ledger tags match artifact tags.

If any item on this checklist cannot be checked, the artifact is not done. Return to the appropriate earlier phase.
