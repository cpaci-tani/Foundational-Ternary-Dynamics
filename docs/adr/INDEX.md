# Architecture Decision Records (ADR) — Index

**Audience: LLM agents and humans deciding whether to deviate from established patterns.**
**Update trigger: any new architectural decision; every quarter for status review.**

This index lists every ADR. Each ADR is ≤200 words: Status / Context /
Decision / Consequences / Alternatives Considered. ADRs are immutable —
new decisions get new numbers; superseded decisions get a "Superseded by:"
header but the original record stays.

---

## Active decisions

| # | Title | Status | Date | One-line rationale |
|---|---|---|---|---|
| 0001 | [Viewport decomposition](0001-viewport-decomposition.md) | Accepted | 2026-04 | Sub-renderers extracted from monolithic Viewport class |
| 0002 | [Capability factories](0002-capability-factories.md) | Accepted | 2026-04 | Factory functions (not class inheritance) for scale capabilities |
| 0003 | [WASM bridge DAG refactor](0003-wasm-bridge-dag-refactor.md) | Accepted | 2026-04 | DAG scheduler over linear pipeline for tick orchestration |
| 0004 | [Scale controllers + 3-folder structure](0004-scale-controllers.md) | Accepted | 2026-04 | Each scale owns its own controller/runtime/ui/state package |
| 0005 | [Live-reference factory pattern](0005-live-reference-pattern.md) | Accepted | 2026-04 | Factories hold state ref; never destructure |
| 0006 | [Prefix-dispatch scenarios](0006-prefix-dispatch-scenarios.md) | Accepted | 2026-04 | Filename-prefix routing (flux-, light-, quantum-, s0-seed-, s0-field-) |
| 0007 | [CUDA helper consolidation](0007-cuda-helper-consolidation.md) | Accepted | 2026-04 | Shared headers (cuda_index.cuh) over per-kernel local helpers |
| 0008 | [R1-R5 phase extraction](0008-r1-r5-phase-extraction.md) | Accepted | 2026-04 | render_bridge.cpp phases extracted to focused TUs |
| 0009 | [Epistemic tag system](0009-epistemic-tag-system.md) | Accepted | 2026-04 | 7-tag vocabulary ([THEOREM]/[CONJECTURE]/...) for claims |
| 0010 | [Cascade callback pattern](0010-cascade-callback-pattern.md) | Accepted | 2026-04-27 | Sub-renderer lifecycle (onLatticeSizeChanged etc.) — Phase 3 viewport split |
| 0011 | [Mesh-factory callback](0011-mesh-factory-callback.md) | Accepted | 2026-04-27 | Single canonical home + ctor-time bound callbacks for cross-sub-renderer helpers |
| 0012 | [Golden-tick regression gate](0012-golden-tick-regression-gate.md) | Accepted | 2026-04-27 | 100-tick byte-hash gate for physics-touching extractions (Phase 4) |
| 0013 | [Toggle table-driven](0013-toggle-table-driven.md) | Accepted | 2026-04-27 | TOGGLE_SPECS[] replaces 5-place edit with 2-place (Phase 6) |

---

## ADR template (for new records)

```markdown
# NNNN — Title

**Status:** Accepted | Superseded by ADR-####  | Deprecated
**Date:** YYYY-MM-DD
**Author:** <handle or session>

## Context

What constraints/observations prompted this decision? ≤80 words.

## Decision

The chosen approach. ≤60 words.

## Consequences

What follows from this decision (positive and negative)? ≤40 words.

## Alternatives considered

Others rejected, with one-line rationale each. ≤40 words.

## References

- Files: paths to load-bearing implementations
- Cross-refs: related ADRs, CONTRACTS.md sections, SPEC docs
```

Total: ≤200 words per ADR.

---

## Lifecycle policy

- **Adding an ADR**: pick the next free number; add to this INDEX; cite in
  the related code's `@related` header.
- **Superseding**: never delete. Mark the old ADR `Status: Superseded by ADR-####`
  and link to the replacement; the new ADR has `Supersedes ADR-####`.
- **Deprecating without replacement**: `Status: Deprecated`, with reason.
- **Quarterly review**: scan for stale (>6 months) Accepted ADRs and confirm
  they still match reality; demote to Deprecated if not.

---

## When to write an ADR

- New architectural pattern emerges (e.g., "we should use factory functions
  for X").
- Existing pattern is being changed (e.g., "we're moving from inheritance
  to composition").
- A non-obvious design constraint that future agents would otherwise rediscover
  (e.g., "live-reference is required for cache invalidation").
- A decision rejected for non-obvious reasons that would be re-litigated
  without an ADR.

NOT every commit needs an ADR. Routine bug fixes, performance tweaks, and
minor refactors do not. Reserve ADRs for patterns that govern future work.
