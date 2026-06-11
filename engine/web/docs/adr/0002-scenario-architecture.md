# ADR 0002 — Scale 0 scenario definition: guard four layers, or unify to one

**Status:** Proposed (STUB — awaiting a direction decision; do not treat as Accepted)
**Date:** 2026-06-05
**Decider:** _unassigned_
**Context:** Scale-0 scenario subsystem audit + modularization plan (2026-06-05)

## Context

A Scale-0 scenario is defined across **four parallel layers** keyed only by a string id — the UI
registry (`scales/scale0/scenario-registry.js`), the JS seed impl (`bridge/scenarios/*.js`), the
C++ seed impl (`engine/src/scenarios/*.cpp`), and the metadata (`config/scenarios.js`) — plus a
toggle profile (`config/toggles.js`). Full treatment in
[`../SPEC_SCALE0_SCENARIO_ARCHITECTURE.md`](../SPEC_SCALE0_SCENARIO_ARCHITECTURE.md).

The 2026-06-05 audit ([`../audits/AUDIT_SCALE0_SCENARIO_LIFECYCLE_2026-06-05.md`](../audits/AUDIT_SCALE0_SCENARIO_LIFECYCLE_2026-06-05.md))
found JSC++ parity healthy (guarded, green) but the *other* edges unguarded and drifting: a stale
overlay round-trip mirror (`B1`), two parallel toggle mechanisms one of which leaks (`B2`/`B3`),
an orphaned scenario id referenced in three places but implemented in none (`B4`), and partial
metadata with no registrymetadata check (`B5`/`B6`). The recurring shape is the
hand-maintained-mirror hazard that `store.js createFieldFlags()` already fixed programmatically.

The roadmap ([`../PLAN_SCALE0_SCENARIO_MODULARIZATION.md`](../PLAN_SCALE0_SCENARIO_MODULARIZATION.md))
sequences a Tier 0 (safe wins) → Tier 1 (Direction A) → Tier 2 (Direction B) path and recommends
deferring this decision until Tier 1's guards provide evidence.

## Decision

**TBD.** Choose one once Tier 1 lands:

- **Direction A — keep four layers, guard every edge.** Extend the parity guard to custom-literal
  registry entries, add registrymetadata + overlay-setcanonical-keys assertions, and document
  the toggle/metadata contracts as named, tested boundaries. Lower risk; the four layers remain.
- **Direction A + T2.1 — guard, then unify toggles only.** Direction A plus collapsing the 10
  imperative custom-`load()` toggle setups into declarative param-carrying profiles (closes the
  `langevin` leak). Recommended midpoint if A's guards confirm the toggle layer is the main pain.
- **Direction B — unify to one descriptor.** A single per-scenario record generates/validates the
  registry, metadata, override table, and parity manifest; only the JS+C++ seed bodies stay
  hand-written. Highest payoff, largest one-time churn across ~96 entries.

## Consequences

_To be completed when the decision is made — mirror the ADR 0001 Positive/Negative structure._

## Alternatives considered

_To be completed. The roadmap's Tier 2 "evidence-gated" framing is itself the leading argument for
choosing A first and revisiting B with data._

## References

- [`../SPEC_SCALE0_SCENARIO_ARCHITECTURE.md`](../SPEC_SCALE0_SCENARIO_ARCHITECTURE.md) §3 (four layers), §8 (the two directions)
- [`../audits/AUDIT_SCALE0_SCENARIO_LIFECYCLE_2026-06-05.md`](../audits/AUDIT_SCALE0_SCENARIO_LIFECYCLE_2026-06-05.md) (findings `A1`/`B*`/`C*`)
- [`../PLAN_SCALE0_SCENARIO_MODULARIZATION.md`](../PLAN_SCALE0_SCENARIO_MODULARIZATION.md) (tiered tickets)
- ADR [`0001-viewport-decomposition.md`](0001-viewport-decomposition.md) (house style + small-diff safety posture)
