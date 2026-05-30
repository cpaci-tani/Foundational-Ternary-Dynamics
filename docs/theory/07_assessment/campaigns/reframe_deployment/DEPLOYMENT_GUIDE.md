# FTD Reframe Deployment Guide

## Operational plan for updating the FTD portfolio to replace completed-infinity reasoning with undefined-boundary finitism

**Date:** April 2026
**Scope:** All FTD papers, derivations, engine code, and ledger entries
**Goal:** Systematically identify every invocation of completed infinity in the portfolio, triage each one (survives, restate, re-derive, or retract), execute the fix, and update cross-references. Exit with a consistent portfolio whose foundational commitments match the undefined-boundary stance.

---

## The Core Problem This Plan Addresses

An agent-driven rewrite at this scale has exactly one dominant failure mode: drift across independent subagent invocations. Two subagents processing two different papers will produce subtly inconsistent restatements of the same underlying concept unless they are anchored to a common canonical document. The single most important discipline in this deployment is that every subagent reads the same canonical reframe document before doing any work. Everything else is procedural.

The second-biggest failure mode is accidental regression. Agents restating claims may weaken them unnecessarily, or may miss that a given "infinity" reference is actually algebraic (permitted) rather than limiting (proscribed). The plan uses devil's-advocate passes and a central ledger to catch this.

---

## Seven Phases

### Phase 0: Canonical Reframe Document

Before any agent runs, commit to `CANONICAL_REFRAME.md` the single authoritative statement of the reframe. Every subsequent agent invocation begins by reading this file. It does not change during the project except in response to explicit user decisions, and changes are version-stamped.

This document must state:
1. The philosophical commitment (completed infinity is rejected; undefined boundary is the alternative).
2. Proscribed moves (with examples).
3. Permitted moves (with examples).
4. How to tell a proscribed move from a permitted one when the case is subtle.
5. Tag semantics (THEOREM, SELECTION PRINCIPLE, HYPOTHESIS, CONJECTURE).
6. The decision procedure for any flagged item (survive / restate / re-derive / retract).

A draft of this document is in `CANONICAL_REFRAME.md`. Read it, revise until you are satisfied, and freeze it before Phase 1.

### Phase 1: Inventory

Walk the entire portfolio and produce a flat list of every artifact: papers (drafts, published, archived), derivation notes, engine source files, ledger entries, blog posts, Medium drafts, slide decks. For each artifact, record: file path, last-modified date, current portfolio status, whether it makes load-bearing claims, and whether those claims are cited elsewhere.

Output: `INVENTORY.md`, one row per artifact.

Agent used: Inventory agent (see `agents/01_inventory.md`).

### Phase 2: Classification (per-artifact audit)

For each artifact in the inventory, run the Classifier agent (`agents/02_classifier.md`). The agent reads the artifact and `CANONICAL_REFRAME.md`, and produces an audit report listing every passage that invokes completed infinity, distinguishing:
- Passages that are genuinely proscribed and need action.
- Passages that look like infinity but are actually algebraic/finitary (no action needed, but flag for cross-check).
- Passages where the status is ambiguous (escalate to the user).

Output: one `AUDIT_<artifact>.md` per artifact, plus one consolidated `FLAGGED_PASSAGES.md` listing all proscribed items across the portfolio.

Critical: run the classifier on every artifact independently, even if they share content. Do not let one agent carry context across artifacts. The classifier is stateless by design because carrying context is the drift vector.

### Phase 3: Triage

A single human pass (you, not an agent) over `FLAGGED_PASSAGES.md`. For each flagged item, assign a triage action:
- **SURVIVES**: the passage is actually fine (classifier was overzealous).
- **RESTATE**: the passage's underlying content is sound but the framing invokes completed infinity. Rewrite in finitary terms.
- **RE-DERIVE**: the passage's content depends on a limiting argument that cannot be simply restated. Requires a new proof.
- **RETRACT**: the passage makes a claim that does not survive the reframe. Remove and adjust surrounding text.

Output: `TRIAGE.md`, a master spreadsheet with one row per flagged item and a column for assigned action.

Why this is manual: triage is the judgment step where your framework understanding matters most. An agent can propose triage actions, but final assignment should be yours. Agents process; you decide.

### Phase 4: Restatement and Re-Derivation

For each item in `TRIAGE.md`:

- **RESTATE items**: run Restatement agent (`agents/04_restatement.md`). Input: the original passage and the surrounding context (paragraph or section). Output: a proposed rewrite that preserves content and removes completed-infinity framing.
- **RE-DERIVE items**: run Re-derivation agent (`agents/05_rederivation.md`). Input: the claim being re-derived, the original proof sketch, and any framework axioms needed. Output: a new proof attempt, or a failure report identifying what cannot be re-derived.
- **RETRACT items**: run Restatement agent in retraction mode. Input: the original passage and what depends on it. Output: proposed removal plus proposed adjustments to any dependent text.

Every output from this phase is a proposal, not a committed change. Changes are staged for review.

### Phase 5: Engine Audit

Separate track, runs in parallel with Phase 4. The engine code needs an audit for:
- Explicit L→∞ or similar limiting operations.
- Implicit infinity assumptions (e.g., assuming averages converge).
- Hidden α insertions (the parameter-free claim).
- Assumptions about global state vs local state.

Run Engine Audit agent (`agents/06_engine_audit.md`) on each engine source file. Output: `ENGINE_AUDIT.md` with findings per file.

This phase feeds into Phase 6 (integration) because engine changes can invalidate paper claims and vice versa.

### Phase 6: Integration

The phase where agent proposals become committed portfolio changes. For each paper, each staged change goes through three checks:

1. **Devil's advocate pass**: run P4 Devil's Advocate agent (`agents/07_devils_advocate.md`) on the proposed restatement. The agent attempts to falsify the restatement as a valid replacement. If it succeeds, the restatement is sent back to Phase 4.
2. **Consistency check**: run Consistency agent (`agents/08_consistency.md`) to verify that the restatement does not create citation breakage, terminology conflict, or logical inconsistency with other papers.
3. **Ledger update**: run Ledger agent (`agents/09_ledger.md`) to update the master ledger with the new tag, revised claim wording, and any dependency changes.

Only items passing all three checks are committed. Items failing any check return to the appropriate earlier phase.

### Phase 7: Verification

Final pass over the whole updated portfolio. Three deliverables:

1. **Updated master ledger** with every claim re-tagged and every restatement reflected.
2. **Changelog** documenting every triage decision and every change made.
3. **Consistency report** verifying no dangling references, no contradictory statements across papers, no untagged claims.

Run Verification agent, which is the Consistency agent operating on the full portfolio rather than per-paper.

---

## Tooling Recommendation

This work is most efficiently done in Claude Code, because it has:
- Subagent orchestration (multiple agents in parallel on the same project).
- Direct file access to the portfolio.
- Git integration for branch-per-paper and staged commits.
- Persistent CLAUDE.md instructions at the project level.

The deployment assumes Claude Code. If you are in a different environment, the patterns still work but you will manage orchestration manually.

Setup checklist:
1. Create a git branch for the reframe: `git checkout -b reframe-undefined-boundary`.
2. Place `CANONICAL_REFRAME.md` at the project root.
3. Place the GTCA skill in `.claude/skills/gtca/` (or wherever your Claude Code installation expects skills).
4. Create a project-level `CLAUDE.md` that tells Claude Code to route all FTD work through the GTCA skill and to read `CANONICAL_REFRAME.md` before any audit or restatement task. A template is in `templates/CLAUDE_MD_TEMPLATE.md`.
5. Set up sub-branches per paper for isolation: `git checkout -b reframe/paper-<name>` for each paper in the inventory.

---

## Agent Invocation Pattern

Every subagent invocation follows the same pattern:

1. The parent session reads `CANONICAL_REFRAME.md` and the relevant agent prompt from `agents/`.
2. The parent session spawns a subagent with: (a) the agent prompt as its system context, (b) the canonical reframe as reference material, (c) the specific artifact to process as input, (d) a clear output location.
3. The subagent completes its task and writes output to the specified location.
4. The parent session does not edit the subagent's output; it either accepts, rejects (back to queue), or escalates.

Parallelism: up to 5 subagents in parallel is typically safe. Beyond that, the risk of resource contention on shared files rises. The inventory phase can run at higher parallelism (no shared-file contention) but the integration phase must be serialized (the ledger is a single file).

---

## Quality Gates

Between each phase, a gate that must pass before the next phase begins:

**Gate 0→1**: `CANONICAL_REFRAME.md` is frozen and version-stamped. You have read it end-to-end and approved it. No further edits during the deployment without explicit change-log.

**Gate 1→2**: `INVENTORY.md` is complete. You have spot-checked at least 10% of entries for accuracy. Missing artifacts are surfaced.

**Gate 2→3**: All artifacts have an `AUDIT_*.md`. `FLAGGED_PASSAGES.md` consolidates them. You have spot-checked the classifier's judgment on at least 5 random flagged items.

**Gate 3→4**: `TRIAGE.md` has an action assigned to every flagged item. No items are left "to be decided."

**Gate 4→6**: Every RESTATE and RE-DERIVE item has a proposed output. Failed re-derivations are explicit failure reports, not missing entries.

**Gate 6→7**: Every staged change has passed devil's advocate and consistency checks. Ledger is updated.

**Gate 7→exit**: Verification report is clean. Remaining issues are explicitly accepted and documented.

Gates are a discipline, not a ceremony. Their purpose is to prevent the common failure mode where later phases uncover problems that should have been caught earlier but weren't.

---

## Risk Management

**Risk: drift across subagents.** Mitigation: canonical document read by every agent. Verify by random sampling: ask 5 different subagent outputs "what is the proscribed move here?" and verify the wording is consistent.

**Risk: over-application of the reframe.** Mitigation: devil's advocate agent. Also, explicit "permitted moves" section in the canonical document, with examples.

**Risk: under-application.** Mitigation: classifier runs on every artifact; final verification agent looks for any untagged claim that might involve completed infinity.

**Risk: citation breakage after restatement.** Mitigation: consistency agent checks cross-references.

**Risk: ledger becomes inconsistent under many parallel updates.** Mitigation: serialize integration phase; all ledger updates go through ledger agent, never direct edit.

**Risk: agent session drift (F7).** Mitigation: each agent invocation is stateless; every invocation reads canonical document fresh.

**Risk: user fatigue during triage.** Mitigation: triage in batches of 50 items, not the full list at once. Take breaks. Do not attempt to triage more than 200 items in a single day.

**Risk: portfolio paralysis.** The work is large and can feel overwhelming. Mitigation: set phase targets by calendar week, not by completeness. A 60%-complete portfolio consistently reframed is better than a 100%-complete portfolio partially reframed and partially not.

---

## Exit Criteria

The deployment is complete when:

1. Every artifact in `INVENTORY.md` has a triage disposition.
2. Every RESTATE, RE-DERIVE, and RETRACT action has been executed and committed.
3. The master ledger has an entry for every load-bearing claim in the portfolio, each with a current tag and the reframe's verdict.
4. The changelog documents every decision and change.
5. The verification agent reports no dangling references or contradictory claims.
6. At least one complete paper (recommended: Hermitian Cope or Ontic Incompleteness) has been end-to-end reviewed and read by you as if you had never seen it. It should feel coherent under the reframe.

The exit criteria do not require that every paper be submission-ready. They require that every paper is consistent with the reframe. Submission-readiness is a separate subsequent effort.

---

## What Not to Do

- Do not start with the "most important" paper. Start with a small, well-understood artifact to calibrate the process. The Fifty-Two Faces paper is a good test case because you have just re-read it and the reframe has clean implications for it.
- Do not skip Phase 3 (human triage). The temptation to have an agent do triage is strong. Resist it. Triage is the judgment step.
- Do not run Phase 6 (integration) until Phase 4 is complete for the current batch. Integration assumes a complete set of proposals.
- Do not let the canonical reframe drift during deployment. If a new insight emerges mid-deployment, note it, finish the current deployment consistent with the original document, and handle the insight in a subsequent pass.
- Do not commit to submission timelines during the deployment. The work takes as long as it takes, and external pressure causes the failure modes above.

---

## Files in This Package

- `DEPLOYMENT_GUIDE.md` (this file)
- `CANONICAL_REFRAME.md` (the single source of truth for what the reframe means)
- `agents/01_inventory.md` through `agents/09_ledger.md` (nine agent prompts)
- `templates/AUDIT_REPORT_TEMPLATE.md`
- `templates/RESTATEMENT_TEMPLATE.md`
- `templates/LEDGER_ENTRY_TEMPLATE.md`
- `templates/CLAUDE_MD_TEMPLATE.md` (project-level instructions for Claude Code)
- `checklists/pre_flight.md`, `checklists/per_paper.md`, `checklists/post_flight.md`

The agent prompts are written to be copy-pasted directly into Claude Code subagent invocations. They are self-contained given that the canonical reframe is in context.

---

## Estimated Effort

For a portfolio of approximately 10-15 papers plus engine code:

- Phase 0 (canonical doc): 4-8 hours of your time.
- Phase 1 (inventory): 1-2 hours with agent assistance.
- Phase 2 (classification): 4-8 agent-hours parallelized; 2-4 hours of your oversight.
- Phase 3 (triage): 8-20 hours of focused your time, in batches.
- Phase 4 (restatement/re-derivation): 20-40 agent-hours; 5-10 hours of your review.
- Phase 5 (engine audit): 4-8 agent-hours; 2-4 hours of your review.
- Phase 6 (integration): 10-20 agent-hours; 4-8 hours of your serialized oversight.
- Phase 7 (verification): 2-4 agent-hours; 2-4 hours of your time.

Total: approximately 30-60 hours of your time spread across 2-4 weeks of real-time, with agents doing the bulk of the mechanical work.

This estimate assumes you are the sole human in the loop. If you have collaborators, parallelize triage and review.

---

## Monitor-Style Note on This Plan

This is a P5 strategy output. It is advisory. The plan itself is not a THEOREM about what will work; it is a SELECTION PRINCIPLE informed by experience with large-scale restructuring tasks. Specific phase orderings and agent roles can be adjusted to fit your situation. The two non-negotiable commitments are: (1) the canonical document read by every agent, (2) the ledger as single source of truth.

If you find during execution that the plan needs revision, revise it. The plan is a tool for the work, not a constraint on the work. The goal is a consistent portfolio, not fidelity to this document.
