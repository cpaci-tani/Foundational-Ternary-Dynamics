# Post-Flight Checklist

Complete all items to declare the deployment done.

## Ledger completeness
- [ ] Every load-bearing claim in the portfolio has a ledger entry.
- [ ] Every ledger entry has: claim_id, statement, tag, tag_history, proof_status, dependencies, dependents, last_reviewed, reframe_status, citations.
- [ ] No orphan claims (claims with no citations anywhere).
- [ ] No dangling dependencies (dependencies pointing to non-existent claim_ids).
- [ ] Dependencies and dependents are symmetric.

## Artifact completeness
- [ ] Every artifact in INVENTORY.md has been through Phases 2, 3, and 6.
- [ ] Every RESTATE and RE-DERIVE action is committed.
- [ ] Every RETRACT action is committed and surrounding text is adjusted.
- [ ] No artifact is in a partially-restated state.

## Verification passes
- [ ] Consistency agent ran in portfolio mode and produced `CONSISTENCY_PORTFOLIO.md`.
- [ ] No unresolved findings in the consistency report.
- [ ] Verification agent confirms no untagged claims in updated artifacts.
- [ ] No framework-level contradictions reported.

## Changelog
- [ ] `LEDGER_CHANGELOG.md` has a complete record.
- [ ] A summary document `DEPLOYMENT_SUMMARY.md` exists, documenting:
  - Number of findings total.
  - Distribution across SURVIVES, RESTATE, RE-DERIVE, RETRACT.
  - Number of claims whose tags changed.
  - Number of papers that had substantive changes.
  - List of failed re-derivations (what the framework lost).
  - List of retracted claims (what is no longer in the portfolio).

## End-to-end reading
- [ ] You have read at least one complete paper end-to-end in its post-reframe state.
- [ ] The paper reads coherently.
- [ ] You would be comfortable showing it to a peer.
- [ ] You have read the master ledger end-to-end.
- [ ] The ledger accurately reflects the portfolio.

## Framework self-consistency
- [ ] No "L → ∞", "thermodynamic limit", "continuum limit", or similar phrases remain in the updated artifacts unless they appear with explicit finitary clarification.
- [ ] The parameter-free claim for the engine is either verified (all constants traced) or explicitly downgraded to CONJECTURE with notes.
- [ ] The α-as-dictionary-output reframe is reflected in any paper discussing α.
- [ ] The Tier ontology descriptions are consistent with undefined-boundary phrasing.

## Engine consistency
- [ ] All HIGH risk engine findings are resolved (either fixed or explicitly acknowledged as open).
- [ ] The engine's output under the reframe is documented.
- [ ] The 3.6× gap (if still present) is documented as an OPEN question with finitary interpretations (A, C, D) rather than invoking interpretation B.

## Git state
- [ ] All changes committed on `reframe-undefined-boundary` branch.
- [ ] Per-paper sub-branches merged or archived.
- [ ] Commit messages reference the relevant finding IDs.
- [ ] Ready to merge `reframe-undefined-boundary` to main (if user desires).

## Portfolio-level documents
- [ ] A "Framework Overview" document (portfolio-level summary) exists and reflects the post-reframe state.
- [ ] The statement of what FTD is and does is updated.
- [ ] Any "achievements" or "headline results" list is updated with correct tags.

## Known-open items
- [ ] A `OPEN_ITEMS.md` file lists everything that was not resolved by this deployment.
- [ ] Items are categorized: RE-DERIVATION_FAILED, RETRACTED_WITH_GAP, ENGINE_HIGH_RISK, CROSS_CHECK_PENDING, OTHER.
- [ ] Each item has a note about what would resolve it.

## Deployment retrospective (optional but recommended)
- [ ] A `RETROSPECTIVE.md` document exists capturing:
  - What worked well.
  - What caused friction.
  - What would be done differently next time.
  - Lessons for future portfolio-wide updates.

## Final sign-off
- [ ] All quality gates satisfied.
- [ ] You are willing to represent the portfolio's post-reframe state as the framework's current position.
- [ ] You are comfortable that any paper-reviewer reading an updated paper would not catch an obvious completed-infinity reference.

Once all items are checked, the deployment is complete. The portfolio is ready for subsequent work (submission, extension, further derivation) on a consistent foundation.
