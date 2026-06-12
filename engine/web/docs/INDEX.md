# FTD Web Documentation Index

**Status:** active navigation index.

Use this file to decide which web docs are current guidance, which are
historical provenance, and where new cleanup or architecture notes should land.

---

## Foundation

- [`../ARCHITECTURE.md`](../ARCHITECTURE.md) — current web-engine architecture map.
- [`../../SPEC_ENGINE.md`](../../SPEC_ENGINE.md) — C++ engine reference.
- [`../../../CONTRACTS.md`](../../../CONTRACTS.md) — cross-project interface summary.

## Active Scale-0 Specs

- [`SPEC_SCALE0_BRIDGE_ARCHITECTURE.md`](SPEC_SCALE0_BRIDGE_ARCHITECTURE.md) — bridge implementations, capability surfaces, worker proxy.
- [`SPEC_SCALE0_RUNTIME_PIPELINE.md`](SPEC_SCALE0_RUNTIME_PIPELINE.md) — per-frame tick/upload/overlay/render/telemetry pipeline.
- [`SPEC_SCALE0_SCENARIO_ARCHITECTURE.md`](SPEC_SCALE0_SCENARIO_ARCHITECTURE.md) — scenario definition layers and lifecycle.
- [`SPEC_SCALE0_PERF_TELEMETRY_PANELS.md`](SPEC_SCALE0_PERF_TELEMETRY_PANELS.md) — demand-gated telemetry and panel performance.
- [`SPEC_SCALE0_LATTICE_PERF.md`](SPEC_SCALE0_LATTICE_PERF.md) — lattice performance notes.
- [`SPEC_SCALE0_LATTICE_SPECTROSCOPY.md`](SPEC_SCALE0_LATTICE_SPECTROSCOPY.md) — lattice spectroscopy panel/protocol.
- [`SPEC_S0_QUANTUM_OVERLAYS.md`](SPEC_S0_QUANTUM_OVERLAYS.md) — Scale-0 quantum/topology overlay semantics.
- [`SPEC_VACUUM_PARTICLE_SCENARIOS.md`](SPEC_VACUUM_PARTICLE_SCENARIOS.md) — vacuum particle scenario family.
- [`SPEC_SPACETIME_FORCING_DEMO.md`](SPEC_SPACETIME_FORCING_DEMO.md) — spacetime forcing demo.

## User And Registry References

- [`USER_GUIDE.md`](USER_GUIDE.md) — dashboard user guide.
- [`TOGGLE_REGISTRY.md`](TOGGLE_REGISTRY.md) — toggle inventory.
- [`TELEMETRY_CATALOG_SCALE0.md`](TELEMETRY_CATALOG_SCALE0.md) — Scale-0 telemetry rows/channels.
- [`REF_DEBUG_GLOBALS.md`](REF_DEBUG_GLOBALS.md) — intentionally exposed debug globals.
- [`SPEC_VERIFICATION_LAB.md`](SPEC_VERIFICATION_LAB.md) — verification lab surface.

## UI And Theme References

- [`SPEC_UI_REFACTOR.md`](SPEC_UI_REFACTOR.md) — UI refactor record; still useful for migration provenance.
- [`SPEC_UI_DESIGN_METHODOLOGIES.md`](SPEC_UI_DESIGN_METHODOLOGIES.md) — design methodology notes.
- [`SPEC_THEME_DESIGN.md`](SPEC_THEME_DESIGN.md) — theme/token design.
- [`../css/THEMING.md`](../css/THEMING.md) — code-near CSS/theming policy.

## Active Plans

- [`PLAN_SCALE0_SCENARIO_MODULARIZATION.md`](PLAN_SCALE0_SCENARIO_MODULARIZATION.md) — scenario architecture cleanup roadmap.
- [`PLAN_SCALE0_PHYSICS_WORKER.md`](PLAN_SCALE0_PHYSICS_WORKER.md) — worker rollout provenance; keep until fully superseded by bridge/runtime specs.
- [`PLAN_SCALE0_SPARSE_TICK.md`](PLAN_SCALE0_SPARSE_TICK.md) — sparse tick implementation provenance; keep until sparse tick is fully documented in active perf specs.

## ADRs

- [`adr/0001-viewport-decomposition.md`](adr/0001-viewport-decomposition.md) — accepted; viewport decomposition target and prerequisites.
- [`adr/0002-scenario-architecture.md`](adr/0002-scenario-architecture.md) — proposed; scenario descriptor decision point.

## Audits

- [`audits/AUDIT_WEB_ARCHITECTURE_EPISTEMIC_STATUS.md`](audits/AUDIT_WEB_ARCHITECTURE_EPISTEMIC_STATUS.md)
- [`audits/AUDIT_WEB_ENGINE_2026-05-27.md`](audits/AUDIT_WEB_ENGINE_2026-05-27.md)
- [`audits/AUDIT_BRIDGE_WIRING_2026-06-03.md`](audits/AUDIT_BRIDGE_WIRING_2026-06-03.md)
- [`audits/AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md`](audits/AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md)
- [`audits/AUDIT_SCALE0_SCENARIO_HEALTH_2026-06-05.md`](audits/AUDIT_SCALE0_SCENARIO_HEALTH_2026-06-05.md)
- [`audits/AUDIT_SCALE0_SCENARIO_LIFECYCLE_2026-06-05.md`](audits/AUDIT_SCALE0_SCENARIO_LIFECYCLE_2026-06-05.md)
- [`audits/AUDIT_S0_OVERLAY_GROUNDING.md`](audits/AUDIT_S0_OVERLAY_GROUNDING.md)

Audits are point-in-time evidence. They are useful for provenance, not
necessarily active implementation guidance.

## Historical Provenance

- [`historical/PLAN_WASM64_UPGRADE.md`](historical/PLAN_WASM64_UPGRADE.md) — implemented upgrade plan.
- [`historical/CSS_RETIRE_AUDIT_2026-04-16.md`](historical/CSS_RETIRE_AUDIT_2026-04-16.md) — CSS retirement audit, moved out of the live style tree.
- [`historical/SPEC_AUDIT_COMPLETION.md`](historical/SPEC_AUDIT_COMPLETION.md) — historical audit completion plan.
- [`historical/SPEC_CSS_REVAMP.md`](historical/SPEC_CSS_REVAMP.md) — completed CSS revamp plan/provenance.
- [`historical/SPEC_OVERLAY_SEMANTICS.md`](historical/SPEC_OVERLAY_SEMANTICS.md) — older partial overlay semantics doc, superseded by active overlay specs/audits.

## Generated Artifacts Policy

Markdown is the tracked source of truth. Generated HTML renders and Quarto
sidecar folders under `engine/web/docs/` are ignored by the repository root
`.gitignore` and should not be committed.
