# PLAN — Scale 0 Scenario Subsystem Modularization

**Status:** roadmap (no code executed; awaiting a direction decision).
**Inputs:** [`SPEC_SCALE0_SCENARIO_ARCHITECTURE.md`](SPEC_SCALE0_SCENARIO_ARCHITECTURE.md) (the
foundation) + [`audits/AUDIT_SCALE0_SCENARIO_LIFECYCLE_2026-06-05.md`](audits/AUDIT_SCALE0_SCENARIO_LIFECYCLE_2026-06-05.md)
(the findings, by id `A1`/`B*`/`C*`).
**Decision record:** [`adr/0002-scenario-architecture.md`](adr/0002-scenario-architecture.md)
(stub — fill once a direction is chosen).

LOC and risk are rough, pre-implementation estimates. Tickets cite the audit finding they close.

---

## Recommendation

A **phased** path, A-before-B:

1. **Tier 0 — safe wins now.** A regression fix (`A1`, urgent — it's broken in the current WIP)
   plus a handful of cheap dedups/cleanups. Direction-agnostic; do regardless.
2. **Tier 1 — Direction A (sync contracts + CI guards).** Make every definition edge *fail
   loudly* without restructuring the four layers. Low risk, high safety payoff. This is the
   recommended near-term target and a prerequisite for any larger move.
3. **Tier 2 — Direction B (unify), selectively.** After Tier 1, the audit leans toward doing
   **T2.1 (toggle unification)** because `B3` is a real leak and `B2` is genuine duplication.
   The **full descriptor unification (T2.2)** is worth it only if maintaining four layers stays
   painful *after* the guards are in — defer the call to post-Tier-1 evidence.

The unifying principle is already in-tree: `state/store.js:56-64` `createFieldFlags()` derives a
bag from one key-list *because the hand-maintained mirror drifted*. Every `B*` finding is that
same hazard one module over. Tier 1 generalizes the guard; Tier 2 generalizes the *source*.

---

## Tier 0 — safe wins (do regardless of direction)

| Ticket | Closes | Change | LOC | Risk |
|---|---|---|---|---|
| **T0.1** | `A1` | **Regression fix (urgent).** `scenario-loader.js:400`: pass a real harness, not `{ bridge }` — `getScale0Scenario(id).load(getPhysicsHarness(bridge), { id })` (`getPhysicsHarness` already imported `:1`). Re-run `scale0-resize-guard.spec.js` → green. | ~2 | Low |
| **T0.2** | `B1` | Delete the loader's private `FIELD_BUTTON_IDS`/`FIELD_BUTTON_TO_FLAG` (`scenario-loader.js:47-119`); drive capture/restore + `resetScale0VisualState` from `ui/dom.js` `FIELD_TOGGLE_BINDINGS` (the 40-entry canonical map). | ~−35 | Low-Med |
| **T0.3** | `B4` | Resolve the `s0-seed-2-hydrogen-atoms` orphan — either add it (registry + `s0_seed` impl + C++) or remove the 4 dangling refs (`toggles.js:167`, `knowledge-base/data.js:1327`, `p1-observables-panel.js:63,1210`). | ~5 | Low |
| **T0.4** | `C4` | Reconcile the two stale toggle-order docs to the settled loader (defaults reset *before* `setupScenario`): `bridge/scenarios/README.md:53` and `CONTRACTS.md:294-295`; while there, refresh `CONTRACTS.md` §4 for the 6-group structure + the `(name, harness, ctx)` signature. Do **after** the refactor is committed so the docs match a stable order. | ~8 | None |

**T0.1 is a regression in uncommitted work** — recommend landing it before the rest of the WIP
is committed so the resize-guard spec stays green.

---

## Tier 1 — Direction A: make drift fail loudly

| Ticket | Closes | Change | LOC | Risk |
|---|---|---|---|---|
| **T1.1** | `B5` | Broaden the parity registry extractor (`scenario-parity.spec.js:112-123`) to also match custom-literal `id:` entries; add a **registrymetadata** coverage assertion (every registry id either has metadata or is on an explicit "no-metadata-by-design" allowlist). | ~25 | Low |
| **T1.2** | `B1` | Add a lockstep test: the overlay capture/restore set must equal `store.js` `FIELD_TOGGLE_KEYS` (catches the next B1-class drift). Pairs with T0.2. | ~15 | Low |
| **T1.3** | `C5` | Invoke `validateScale0ScenarioRegistry()` — a dev-mode `console.warn` at boot **and** a one-line unit assertion in the Playwright suite. | ~8 | Low |
| **T1.4** | `C3` | Collapse the dual injection surface: have the dispatcher's no-harness fallback (`index.js:70-84`) construct/borrow a `PhysicsHarness` instead of an ad-hoc literal, so the seed API is defined once. | ~−15 | Med |
| **T1.5** | `B6` | Decide metadata policy: either a coverage lint (warn on registry scenarios without metadata) or an explicit "metadata = s0-seed/quantum only" declaration documented in the SPEC. (Cheap either way; pick one.) | ~10 | Low |

Exit criterion for Tier 1: **no silent cross-layer drift is possible** — every edge (JSC++,
registryJS, registrymetadata, overlay-setcanonical-keys) is guarded by a test that fails on
divergence.

---

## Tier 2 — Direction B: unify the source (selective)

| Ticket | Closes | Change | LOC | Risk |
|---|---|---|---|---|
| **T2.1** | `B2`,`B3` | **Toggle unification (recommended after Tier 1).** Extend the override mechanism to carry param payloads (e.g. `langevin: {T, γ}`) and a research-toggle reset set; convert the 10 custom-`load()` scenarios to declarative profiles in `SCALE0_SCENARIO_OVERRIDES`. Kills the imperative path and the `langevin` leak. Add the B3 toggle-leak regression test. | ~−120 | Med-High |
| **T2.2** | `B6`,`C1`,`C2` | **Full descriptor unification (defer the call).** One canonical per-scenario record (`id, title, category, tags, epistemic, capabilities, toggleProfile, seedRef`) from which the registry, metadata, override table, and a parity manifest are generated/validated. Seed bodies stay hand-written (JS + C++); everything else derives from the record. | ~−250 net, large churn | High |
| **T2.3** | `C1`,`C2` | A single `ScenarioManager` owning current-scenario state + explicit `onEnter`/`onExit` hooks (makes T2.1's research-toggle restore a first-class lifecycle step, and removes the scattered-state problem). | ~+80 | Med |

Tier 2 is **optional and evidence-gated**: run it only if, after Tier 1, the four-layer
maintenance cost is still biting (new scenarios routinely missing a layer, metadata rot, etc.).
T2.1 is the high-value slice; T2.2/T2.3 are the comprehensive end-state.

---

## Sequencing & dependencies

```
T0.1 (regression) ─┐
T0.2 ──────────────┼─ T1.2 (lockstep test depends on T0.2's single map)
T0.3, T0.4 ────────┘
T1.1, T1.3, T1.5 ── independent (any order)
T1.4 ───────────── independent
            (Tier 1 complete: all edges guarded)
                     │
                     ▼  evidence check: still painful?
T2.1 ──────────── T2.3 ──────────── T2.2 (only if justified)
```

Land each ticket as its own small, revertable commit (the engine's post-`cccb38f` safety posture,
per ADR 0001). Every ticket that changes behavior must re-green the relevant Playwright spec
(`scenario-parity`, `scale0-resize-guard`, `wasm-scenario-coverage`, `lifecycle-harness`,
`scale0-worker-teardown`) before merge.

---

## ADR

Once a direction is chosen (A-only / A+T2.1 / full B), record it in
[`adr/0002-scenario-architecture.md`](adr/0002-scenario-architecture.md) (stub created alongside
this plan). The decision turns on one question the audit could not settle for you: **is the
four-layer definition a permanent shape worth guarding (Direction A), or a transitional one worth
collapsing (Direction B)?** Tier 1 buys the time to answer it with evidence rather than upfront.
