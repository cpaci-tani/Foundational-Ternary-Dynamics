# Pre-Flight Checklist

Complete all items before starting Phase 1 (Inventory).

## Canonical document
- [ ] Read `CANONICAL_REFRAME.md` end-to-end.
- [ ] Identified any rule in the canonical document that is unclear or needs adjustment.
- [ ] Applied desired adjustments and version-stamped the document.
- [ ] Frozen the canonical document for the current deployment.

## Environment
- [ ] Claude Code installed and configured.
- [ ] Project root contains `CANONICAL_REFRAME.md`.
- [ ] Project root contains `CLAUDE.md` (from `templates/CLAUDE_MD_TEMPLATE.md`, customized).
- [ ] `gtca` skill installed in `.claude/skills/gtca/`.
- [ ] Agent prompts from `agents/` are readable.
- [ ] Templates from `templates/` are readable.

## Version control
- [ ] Git repository initialized or working on an existing repo.
- [ ] Clean working directory (no uncommitted changes before starting).
- [ ] Branch created: `reframe-undefined-boundary`.
- [ ] Ready to create sub-branches per paper.

## Portfolio overview
- [ ] High-level list of what is in the portfolio (by rough category; the inventory phase will be exhaustive).
- [ ] Identified which artifacts are most load-bearing (expect to triage these first).
- [ ] Identified which papers are submission-targeted (these get extra scrutiny).

## Time and energy
- [ ] Blocked calendar time for the deployment.
- [ ] Understood that triage phase requires sustained focus and cannot be rushed.
- [ ] Have a plan for breaks between triage batches.
- [ ] Have a plan for what to do if something urgent interrupts the deployment.

## Backup
- [ ] Full backup of the portfolio before starting.
- [ ] Backup location is separate from the working copy.
- [ ] Can restore from backup if the deployment needs to restart.

## Success criteria clarity
- [ ] Read the "Exit Criteria" section of `DEPLOYMENT_GUIDE.md`.
- [ ] Comfortable with the definition of "done" for this deployment.
- [ ] Clear on which further work (e.g., submission-readiness) is outside the current deployment's scope.

## Optional but recommended
- [ ] Read the Phase I audit you already produced (`AUDIT_MASTER_QUADRATIC.md`) as a calibration example.
- [ ] Selected a small, well-understood artifact to process first as a pipeline test before scaling up.
- [ ] Written a brief (1 paragraph) statement of what you personally expect to learn from this deployment.

Once all required items are checked, proceed to Phase 1.
