# CLAUDE.md Template for FTD Reframe Project

Place this file at the root of your FTD project when using Claude Code. It provides persistent instructions that Claude Code reads on every invocation, ensuring that every session in the project follows the reframe discipline.

## File content

```markdown
# FTD Project: Claude Code Instructions

## Project Context

This is the Foundational Ternary Dynamics research portfolio. The portfolio is undergoing a systematic reframe from completed-infinity to undefined-boundary reasoning. All work in this project must conform to the reframe discipline documented in `CANONICAL_REFRAME.md` at the project root.

## Mandatory Pre-Work for Every Session

Before doing any work in this project, read:

1. `CANONICAL_REFRAME.md` - the foundational commitment about completed infinity.
2. `DEPLOYMENT_GUIDE.md` - the overall plan for the reframe deployment.
3. The relevant agent prompt in `agents/` for the specific task being performed.

If the work involves FTD-specific content (derivations, proofs, framework claims), also use the `gtca` skill which is installed in `.claude/skills/gtca/`. The skill provides the epistemic tagging discipline, problem-class routing, and failure-mode detection needed for rigorous FTD work.

## Project Structure

- `papers/`: FTD papers in various states (draft, published, archived).
- `derivations/`: Derivation notes and session overviews.
- `engine/`: Computational engine source code.
- `ledger/`: Master claim ledger.
- `audits/`: Per-artifact audit reports from classifier.
- `restatements/`: Proposed restatements and re-derivations from Phase 4.
- `agents/`: Agent prompts for sub-agent invocations.
- `templates/`: Templates for audit reports, restatements, ledger entries.
- `CANONICAL_REFRAME.md`: The authoritative reframe document.
- `DEPLOYMENT_GUIDE.md`: The deployment plan.
- `LEDGER.yaml` (or `.md`): The master ledger.

## Agent Invocation Pattern

When spawning a subagent for any task in this project:

1. Read the relevant agent prompt from `agents/`.
2. Spawn the subagent with the agent prompt as its primary instructions.
3. Ensure the subagent can read `CANONICAL_REFRAME.md` and that it does so as its first action.
4. Provide the subagent with the specific artifact or task as input.
5. Do not accumulate state across subagent invocations. Each subagent is stateless.

## Proscribed Operations in This Project

Do NOT:
- Edit `CANONICAL_REFRAME.md` without explicit user approval and a version bump.
- Directly edit `LEDGER.yaml`; go through the ledger agent.
- Commit changes that have not passed devil's advocate and consistency checks.
- Make restatements without preserving the original passage for diff review.
- Produce derivations that invoke L → ∞, thermodynamic limits, path integrals over "all configurations," or any other completed-infinity operation.

## Required Operations

DO:
- Tag every substantive claim in any new content with an epistemic tag (THEOREM, SELECTION PRINCIPLE, HYPOTHESIS, CONJECTURE).
- Update the ledger when any claim's status changes.
- Run devil's advocate review on every restatement before committing.
- Keep the changelog current.

## Git Discipline

The reframe work is on a branch: `reframe-undefined-boundary`.
Per-paper work is on sub-branches: `reframe/paper-<name>`.
Merges to `reframe-undefined-boundary` require:
- Devil's advocate review passed.
- Consistency check passed.
- Ledger updated.

Merges from `reframe-undefined-boundary` to main require completion of Phase 7 verification.

## Collaboration

This deployment is single-user (you) with agent assistance. If collaborators are added later, they must:
1. Read `CANONICAL_REFRAME.md` and `DEPLOYMENT_GUIDE.md`.
2. Use the agent patterns, not direct editing.
3. Go through the same quality gates.

## When in Doubt

- If the reframe status of a claim is unclear, escalate to user rather than deciding.
- If an agent's output seems inconsistent with the canonical document, flag it rather than accepting.
- If the ledger and a paper disagree, trust the ledger until user rules otherwise.

## Success Criteria

The deployment is complete when:
- Every artifact in INVENTORY.md has a triage disposition.
- Every RESTATE, RE-DERIVE, and RETRACT action is executed and committed.
- The master ledger has an entry for every load-bearing claim.
- The changelog is complete.
- The verification agent reports no unresolved issues.
- At least one paper has been end-to-end reviewed and reads coherently under the reframe.
```

## How to Customize This Template

Replace the generic paths and filenames with your actual project layout. The core commitments (canonical doc read by every agent, ledger as single source of truth, no direct editing bypassing the agent pipeline) should not be customized.

If your project uses a different epistemic tagging system, replace the tag names accordingly. If using a different branching strategy, update the git discipline section. The reframe itself and the stateless-subagent principle must remain.
