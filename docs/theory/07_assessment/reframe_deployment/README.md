# FTD Reframe Deployment Package

## What This Is

A comprehensive deployment package for systematically updating the Foundational Ternary Dynamics portfolio to replace completed-infinity reasoning with undefined-boundary finitism. The package is designed for use with Claude Code (or equivalent subagent-capable environment) and works best with the `gtca` Claude skill installed.

## Entry Point

Start with `DEPLOYMENT_GUIDE.md`. It describes the seven phases, the agent roles, and the quality gates.

Then read `CANONICAL_REFRAME.md`. This is the single source of truth about what the reframe means. Every agent reads this document before acting.

## Package Contents

```
ftd-reframe-deployment/
├── README.md                        # This file.
├── DEPLOYMENT_GUIDE.md              # The overall deployment plan (start here).
├── CANONICAL_REFRAME.md             # Authoritative statement of the reframe.
├── agents/
│   ├── 01_inventory.md              # Catalog portfolio artifacts.
│   ├── 02_classifier.md             # Flag completed-infinity passages per artifact.
│   ├── 03_triage.md                 # Consolidate findings, propose actions.
│   ├── 04_restatement.md            # Rewrite passages in finitary form.
│   ├── 05_rederivation.md           # Produce new proofs when restatement fails.
│   ├── 06_engine_audit.md           # Audit engine source for infinity and hidden couplings.
│   ├── 07_devils_advocate.md        # P4 falsification on every proposed change.
│   ├── 08_consistency.md            # Check cross-artifact consistency.
│   └── 09_ledger.md                 # Maintain master claim ledger.
├── templates/
│   ├── AUDIT_REPORT_TEMPLATE.md     # Format for classifier output.
│   ├── RESTATEMENT_TEMPLATE.md      # Format for restatement/re-derivation output.
│   ├── LEDGER_ENTRY_TEMPLATE.md     # Format for ledger entries.
│   └── CLAUDE_MD_TEMPLATE.md        # Project-level instructions for Claude Code.
└── checklists/
    ├── pre_flight.md                # Before starting.
    ├── per_paper.md                 # Per-artifact tracking.
    └── post_flight.md               # Exit criteria.
```

## How to Use

1. Read `DEPLOYMENT_GUIDE.md` to understand the overall plan.
2. Read `CANONICAL_REFRAME.md` and revise it to reflect your final commitments. Freeze version 1.0.
3. Complete `checklists/pre_flight.md`.
4. Set up your project with the CLAUDE.md from `templates/CLAUDE_MD_TEMPLATE.md`.
5. Install the `gtca` skill in `.claude/skills/gtca/`.
6. Work phase by phase per the deployment guide. Use the agent prompts and templates as specified.
7. Track per-artifact progress with `checklists/per_paper.md`.
8. Complete `checklists/post_flight.md` to exit.

## Non-Negotiable Commitments

Three things must hold throughout the deployment. They are the architecture of the deployment itself.

1. Every subagent reads `CANONICAL_REFRAME.md` before acting. This prevents drift across independent invocations.
2. The ledger is the single source of truth for claim status. Papers derive tags from the ledger; when they disagree, the ledger wins.
3. Every subagent invocation is stateless. No session accumulates context across artifacts.

Departing from any of these is the dominant failure mode the deployment is designed to prevent. If an optimization tempts you to share context across agents, do not.

## Scale Expectations

For a portfolio of 10-15 papers plus engine code, expect 30-60 hours of your time spread across 2-4 real-time weeks. Agents do the bulk of mechanical work; you do judgment-intensive work (triage, review, final sign-off).

## When to Stop

The deployment is complete when `checklists/post_flight.md` is fully checked and you have read at least one updated paper end-to-end and found it coherent. Submission readiness is a separate subsequent effort; this deployment only achieves reframe-consistency.

## Where to Get Help

- The `gtca` skill's `references/` folder has detailed specifications of each architecture component.
- The `gtca` skill's `scripts/claim_auditor.py` can be used during Phase 2 as a heuristic pre-screen.
- The GTCA skill's failure-mode catalog documents the specific drift, capture, and confabulation risks this deployment is designed to prevent.

## Recommendations

- Start with a small artifact to calibrate the pipeline before scaling up. Fifty-Two Faces is a good candidate because you just re-read it.
- Do not run more than 5 subagents in parallel until you have confidence in the quality gates.
- Take real breaks during triage. Triage fatigue is a real source of error.
- Keep the changelog current in real time; reconstructing it later is much more expensive.

## Good luck.
