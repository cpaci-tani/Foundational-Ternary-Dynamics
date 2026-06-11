# Audit — Scale 0 Scenario Lifecycle (2026-06-05)

**Scope:** the Scale-0 scenario subsystem as mapped in
[`../SPEC_SCALE0_SCENARIO_ARCHITECTURE.md`](../SPEC_SCALE0_SCENARIO_ARCHITECTURE.md) — the four
definition layers, the load→init→tick→teardown lifecycle, and the contracts that bind them.
**Baseline:** the **settled working tree** as of 2026-06-05 (the subsystem was mid-refactor;
HEAD `7b228d93` predates ~840 lines of uncommitted scenario work). **Remediation roadmap:**
[`../PLAN_SCALE0_SCENARIO_MODULARIZATION.md`](../PLAN_SCALE0_SCENARIO_MODULARIZATION.md).

**Method — verify before claiming.** Every finding was checked against the named source, and
the runtime-dependent ones by running a test. The verification status is tagged per finding:

- **[VERIFIED·runtime]** — reproduced by executing a Playwright spec or the page.
- **[VERIFIED·static]** — confirmed by reading the settled source (file:line cited).
- **[STATIC·needs-runtime]** — strong static evidence; a runtime check would harden it further.

(This discipline is deliberate: the 2026-05-27 web audit over-counted dead code by ~10 items
by inferring rather than reading; the 06-03/06-04 audits adopted "no inferred breakage." So
does this one.)

---

## Headline

1. **A regression in the in-flight refactor: resizing the lattice is broken for every
   factory scenario (including the default `flux-pulse`).** `resizeScale0Lattice` re-seeds via a
   bare `{ bridge }` where the refactored factory `load` now expects a `PhysicsHarness`. An
   **existing test that was green in the 06-04 audit now fails.** This is fresh, actionable
   feedback on uncommitted work — see **A1**.
2. **JSC++ parity is healthy** — the guard passes 5/5 and the unique scenario sets match
   exactly (95 = 95). An earlier raw-occurrence-count "drift" reading was wrong; corrected in
   the SPEC. The real definition-layer gaps are narrower and live in the *unguarded* edges
   (registry custom-literals, registrymetadata) — see **B5/B6**.
3. **The subsystem's coupling is concentrated in three hand-maintained mirrors** that have each
   drifted or duplicated: the overlay round-trip list (**B1**), the two parallel toggle
   mechanisms (**B2/B3**), and the orphaned scenario id (**B4**). All three are exactly the
   "hand-maintained mirror" hazard that `store.js`'s `createFieldFlags()` already solved
   programmatically — the precedent the roadmap generalizes.

No claim here is stronger than its verification tag. Counts are re-derived from the settled
tree (parity inventory: `UI 86 / JS 96 / C++ 95 / shared 95`).

---

## Resolution (same session, 2026-06-05)

Most findings were fixed and verified in the same pass; the remaining are
architecture-*decision* items (the A-vs-B call) deferred per the roadmap + ADR 0002.

**Fixed + verified:**
- **A1** — resize now passes a real harness (`scenario-loader.js:400`). `scale0-resize-guard.spec.js`
  **3/3 green** (was failing: `mockLattice` 33 ≠ 145).
- **B1** — the loader's overlay maps now **derive** from `ui/dom.js` `FIELD_TOGGLE_BINDINGS` (the
  canonical 36-entry map); the 4 new overlays are covered and it can't drift again. Boots clean
  (resize-guard 3/3, 0 console errors).
- **B4** — all four `s0-seed-2-hydrogen-atoms` references removed (`toggles.js`,
  `knowledge-base/data.js`, `p1-observables-panel.js` ×2).
- **B5 / B6** — parity registry extractor now also matches custom-literal `id:` entries (UI
  inventory **86 → 96**) + a new orphan-metadata assertion. `scenario-parity.spec.js` **6/6 green**.
- **C4** — `bridge/scenarios/README.md` rewritten + `CONTRACTS.md` §4 reconciled to the settled
  loader (toggle-order, 6 groups, `(name, harness, ctx)` signature).
- **C5** — `validateScale0ScenarioRegistry()` now runs at module load (warns on drift).
- **C6** — `getScale0Scenario()` warns on an unknown id instead of silently defaulting.

**Deferred (architecture decisions — see PLAN + ADR 0002):**
- **B3 — CLOSED (not reproduced).** A regression test (`scale0-toggle-leak.spec.js`) shows
  `langevin` does **not** leak: every scenario load resets the bridge (via `setupScenario`), which
  clears it, so switching away from an emergent scenario leaves langevin OFF on both bridges
  (verified for a mock-owned next, `flux-pulse`, and a main-owned next, `quantum-tunnel`). The
  static concern doesn't survive runtime. Test kept as a permanent guard.
- **B2** — the two-toggle-mechanism *duplication* remains (declarative overrides vs imperative
  custom-`load()`), but with B3 closed it's a code-quality item, not a bug; fold into T2.1 if/when
  the descriptor unification happens.
- **C1 / C2 / C3** — scenario lifecycle hooks, a single `ScenarioManager`, and collapsing the dual
  injection surface are the larger Direction-B restructures; evidence-gate per the roadmap.

---

## Findings

> Statuses below are the **as-discovered** findings; see the Resolution section above for which
> are now fixed. (A1, B4, B5, C4, C5, C6 are landed; B1's drift is closed by derivation.)

| # | Sev | Status | Finding | Anchor |
|---|-----|--------|---------|--------|
| **A1** | **High** | VERIFIED·runtime | Resize re-seeds factory scenarios with a bare `{ bridge }`; the refactored `load` calls `harness.setupScenario` → throws → fluxMock never rebuilt → resize silently no-ops on flux-pulse | `scenario-loader.js:400` |
| **B1** | Med | VERIFIED·static | Overlay round-trip list (`FIELD_BUTTON_IDS`/`FIELD_BUTTON_TO_FLAG`, 32) was a stale 3rd mirror of the 36-entry canonical list; the 4 new 2026-06-03 overlays weren't captured/restored (FIXED — now derived) | `scenario-loader.js:47-119` |
| **B2** | Med | VERIFIED·static | Two parallel toggle mechanisms — declarative `SCALE0_SCENARIO_OVERRIDES` vs imperative `setToggle` in 10 custom-`load()` registry entries | `toggles.js:82` / `scenario-registry.js:74-375` |
| **B3** | ~~Med-High~~ **Closed** | VERIFIED·runtime (not reproduced) | Theorized `langevin` leak from custom-`load()` scenarios — runtime shows the bridge reset clears it on every load; no leak. Guard: `scale0-toggle-leak.spec.js` | `scenario-registry.js:88,154,…` |
| **B4** | Low-Med | VERIFIED·static | `s0-seed-2-hydrogen-atoms` is orphaned: it has a toggle-override, KB entry, and a panel that *instructs users to load it*, but no registry/impl entry — it can't be selected | `toggles.js:167` |
| **B5** | Low | VERIFIED·static | Parity guard's registry extractor regex matches only `makeScenario(...)` → the 10 custom-literal scenarios are unguarded; no registrymetadata assertion exists | `scenario-parity.spec.js:112-123` |
| **B6** | Low | VERIFIED·static | Metadata covers only `s0-seed-*`+`quantum-*`; `flux/light/s0-field/s0-vacuum` have none; registry `epistemicStatus` not synced to metadata tags | `config/scenarios.js:57` |
| **C1** | Med | VERIFIED·static | No scenario lifecycle hooks (`onEnter`/`onExit`/`dispose`); teardown is implicit via `reset()`/`setFluxMock` | `scenario-registry.js:1-15` |
| **C2** | Med | VERIFIED·static | "Current scenario" state is scattered (store, DOM `<select>`, `ctx`, bridge tick); unknown ids silently resolve to `flux-pulse` | `store.js:174` / `scenario-registry.js:402` |
| **C3** | Low-Med | VERIFIED·static | The seed injection surface is defined twice — the `PhysicsHarness` class and the inline fallback literal in the dispatcher | `index.js:70-84` |
| **C4** | Med | VERIFIED·static | The refactor **inverted** the toggle-vs-seed order (defaults now reset *before* `setupScenario`, not after) — silently changing a documented contract and leaving **two** docs stale: `bridge/scenarios/README.md:53` and `CONTRACTS.md:294-295` | `scenario-loader.js:320`<`:335` |
| **C5** | Low | VERIFIED·static | `validateScale0ScenarioRegistry()` is never invoked at runtime or in a test | `scenario-registry.js:429` |
| **C6** | Low | VERIFIED·static | Silent failure modes: unknown id → default scenario; unknown name → no-op in both dispatchers | `index.js:66-90` |

---

## Detail

### A1 — Resize is broken for factory scenarios *(High, VERIFIED·runtime)* — regression in the in-flight refactor

**What.** `resizeScale0Lattice` re-seeds the scenario after a resize with
```js
getScale0Scenario(scenarioId).load({ bridge }, { id: scenarioId });   // scenario-loader.js:400
```
The refactor routed the *load* path through a real harness (`scenario-loader.js:334`
`getPhysicsHarness(ctx.bridge)`), and updated the factory descriptor's `load` to call
`harness.setupScenario(...)` (`scenario-registry.js:11-13`). But the **resize** call site still
passes a bare object literal `{ bridge }`, which has no `setupScenario` → `TypeError:
harness.setupScenario is not a function`. `controller.resize` wraps the call in try/catch
(`controller.js:344-350`), so the throw is **swallowed and logged**, aborting
`resizeScale0Lattice` before the fluxMock rebuild at `:413-421`. The C++ bridge size advances
(set at `:388-399`, before the throw) but the JS MockBridge that actually owns flux/`s0-*`
scenarios is never reallocated.

The 10 custom-`load()` scenarios survive this (they defensively unwrap
`const bridge = harness.bridge || harness;`, `scenario-registry.js:84`), which is why the bug
hides on exactly the scenarios most users don't start on.

**Evidence (runtime).** `npx playwright test scale0-resize-guard.spec.js --grep "without refusal"`
**fails** on the settled tree:
```
Error: MockBridge reallocated at 145
  Expected: 145
  Received: 33      // scenario-loader.js:100 — fluxMock left at the initial size
```
`bridgeLattice` reached 145 (the C++ resize ran) but `mockLattice` stayed 33 (the fluxMock was
never rebuilt) — exactly the abort-after-`:399`-before-`:421` signature. This test was green in
`AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md §6` (pre-refactor).

**Recommendation (do-first).** Pass a harness, not a literal, at `:400` — mirror the load path:
`getScale0Scenario(scenarioId).load(getPhysicsHarness(bridge), { id: scenarioId });` (and
import `getPhysicsHarness`, already imported at `:1`). Then re-run the resize-guard spec to green.
Direction-agnostic; it's a straight regression fix.

### B1 — Overlay round-trip is a stale third mirror *(Med, VERIFIED·static)*

`scenario-loader.js` keeps its own `FIELD_BUTTON_IDS` (`:47-82`, 32 ids) and `FIELD_BUTTON_TO_FLAG`
(`:86-119`, 32 pairs) to capture/restore overlays across a scenario switch (`:237-286`). These
duplicate the canonical maps — `state/store.js` `FIELD_TOGGLE_KEYS` (`:3-47`, **36**) and
`scales/scale0/ui/dom.js` `FIELD_TOGGLE_BINDINGS` (`:39-42` add the four new ones, **36** total)
— and have drifted behind them. **(Fixed 2026-06-05: the loader now derives both maps from
`FIELD_TOGGLE_BINDINGS`.)** The 2026-06-03 overlays `showStateField` / `showLatency` /
`showGaussResidual` / `showMooreDecomp` are fully user-facing — buttons in
`ui/overlays/template.js:75,79,202`, bindings in `dom.js:39-42`, render paths in
`runtime/field-overlays.js:619-621` — but are **not** in the loader's round-trip maps, so toggling
one and then switching scenarios desyncs the button from its (reset) state flag and drops the
overlay. This is the exact hand-maintained-mirror hazard `store.js:56-64 createFieldFlags()`
documents fixing programmatically. **Fix:** have the loader consume `dom.js`'s
`FIELD_TOGGLE_BINDINGS` directly instead of its private copy (Direction A/B both want this).

### B2 / B3 — Two toggle mechanisms, one of them leaks *(Med / Med-High)*

**B2 (VERIFIED·static).** Scenario-specific toggles are expressed two ways: declaratively in
`SCALE0_SCENARIO_OVERRIDES` (`toggles.js:82-198`, applied by `applyToggleDefaults`
`scenario-loader.js:181-228`), and imperatively via `bridge.setToggle(...)` inside the 10
custom-`load()` registry entries (`scenario-registry.js:74-375`). Same job, two code paths, no
shared schema — and the imperative path runs *after* the declarative reset, so it can't be
expressed in the override table as-is because the override table has no slot for
`setLangevinParams(T, γ)`.

**B3 — CLOSED, not reproduced (VERIFIED·runtime 2026-06-05).** Those imperative loads set
`langevin` (and call `setLangevinParams`), and `langevin` is deliberately excluded from
`SCALE0_TOGGLES` (`toggles.js:49-51`) as a user-owned research control, so the whitelist reset
never clears it — which raised the static concern that it would leak. **It does not.** A regression
test (`tests/scale0-toggle-leak.spec.js`) confirms that after loading `s0-seed-emergent-ic1`
(langevin ON on its active bridge) and switching to another scenario, langevin is OFF on both the
main bridge and the flux mock — verified for both a mock-owned next (`flux-pulse`) and a main-owned
next (`quantum-tunnel`). Every scenario load resets the bridge via `setupScenario`, which clears it.
Finding closed; the test stays as a guard. (The *duplication* of the two toggle mechanisms — B2 —
remains a code-quality item, not a bug.)

### B4 — Orphaned scenario id `s0-seed-2-hydrogen-atoms` *(Low-Med, VERIFIED·static)*

The id is referenced in four places — a toggle-override (`toggles.js:167`), a knowledge-base
entry (`ui/components/knowledge-base/data.js:1327`), and the P1 observables panel's
`HYDROGEN_SCENARIOS` set + empty-state copy that tells the user to *"Load `s0-seed-2-hydrogen-atoms`"*
(`ui/overlays/p1-observables-panel.js:63,1210`) — but it exists in **neither** the registry
(which has `s0-seed-h2-bond-formation`, a different id) **nor** any JS/C++ impl (no `case` /
`name ==`). So the override is dead code, and the panel instructs users to load a scenario that
isn't in the dropdown. **Fix:** either add the scenario (registry + impl) or remove the four
dangling references. A registryreferences check (B5) would have caught it.

### B5 / B6 — Unguarded definition edges *(Low)*

**B5.** The parity guard's UI extractor (`scenario-parity.spec.js:112-123`) regex
`makeScenario\('[^']+',\s*'([^']+)'` matches only the *factory* form, so assertion 4 silently
skips the 10 custom-literal scenarios (they happen to be implemented today — nothing is broken —
but a future rename wouldn't be caught). There is **no** assertion that registry  metadata
agree (B4 is the symptom). **Fix:** broaden the extractor to also match `id:` literals and add a
registrymetadata coverage assertion (Direction A's core deliverable).

**B6.** `S0_SEED_SCENARIO_METADATA` (`config/scenarios.js:57-327`) covers only `s0-seed-*` (and
~22 live entries; many commented for provenance); `QUANTUM_SCENARIO_DESCRIPTIONS` covers
`quantum-*`. `flux/light/s0-field/s0-vacuum` have no metadata. The registry's per-entry
`epistemicStatus` field is a *separate* tag never reconciled with the metadata's `epistemic`
table. **Fix:** one descriptor carrying both (Direction B), or a coverage lint (Direction A).

### C1–C6 — Structural / hygiene *(Med→Low, VERIFIED·static)*

- **C1** No `onEnter`/`onExit`/`dispose` on a scenario; switch-teardown is implicit (next load
  `reset()`s; `setFluxMock` disposes the prior mock, `store.js:137-148`). Adding explicit hooks
  would make the toggle-leak (B3) and any future per-scenario resource a first-class concern.
- **C2** "Current scenario" lives in `store.currentScenarioId` (`store.js:174`), the DOM
  `#scenario-select`, `ctx`, and the bridge tick — no single owner. `getScale0Scenario`
  (`:402-404`) maps unknown ids silently to `flux-pulse`, so a typo loads the default with no
  signal.
- **C3** The seed injection API is defined twice: the `PhysicsHarness` class
  (`physics/physics-harness.js:330-386`) and the inline fallback `scenarioHarness` literal in the
  dispatcher (`bridge/scenarios/index.js:70-84`). They can drift independently.
- **C4** The refactor **inverted** the toggle-vs-seed order: HEAD ran `applyToggleDefaults`
  *after* `scenario.load`/`setupScenario`; the settled loader runs it *before* (`:320` precedes
  `:335`). This silently changed a documented contract — a scenario body's whitelisted-toggle
  mutations now run **last** and can override `SCALE0_SCENARIO_OVERRIDES`, where pre-refactor the
  override table had the last word — and left **two** docs stale: `bridge/scenarios/README.md:53`
  and `CONTRACTS.md:294-295` (§4 step 5), both still claiming defaults reset *after* `setupScenario`.
  (`CONTRACTS.md` §4 is further stale: it lists 5 groups not the current 6, and the pre-harness
  `setupXxxScenario(name, ctx)` signature.) A second loose end in the in-flight refactor (cf. A1).
- **C5** `validateScale0ScenarioRegistry()` (`:429-440`) is dead-but-callable — never run at boot
  or in a test, so its duplicate/scale/category checks never fire.
- **C6** Unknown names no-op silently in both dispatchers (`index.js:66-90`,
  `scenarios.cpp:71-77`) and unknown ids default silently — convenient, but it hides typos and the
  B4-class orphan.

---

## What's healthy (don't "fix")

- **JSC++ parity** — 5/5 green, 95 unique scenarios on each side fully shared
  (`scenario-parity.spec.js`). The guard does its job for the factory-form scenarios.
- **The bridge direct-read contract** — `bridge/bridge-contract.js` `SCALE0_DIRECT_READS`
  (`:80-110`) is the reference example of a contract done right: one named export consumed by both
  the worker proxy and its regression test. The scenario layers should aspire to this shape.
- **`createFieldFlags()` precedent** — `store.js:56-64` already derives a bag from a single key
  list *because* the hand-maintained mirror drifted. B1 is the same problem one module over.
- **Lifecycle controller + worker teardown** — verified valid end-to-end in
  `AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md` (71/71); not re-litigated here.

---

## Verification log (2026-06-05)

| Check | Command / source | Result |
|---|---|---|
| JSC++registry parity | `npx playwright test scenario-parity.spec.js` | **5/5 pass**; inventory `UI 86 / JS 96 / C++ 95 / shared 95` |
| Resize on factory scenario (A1) | `npx playwright test scale0-resize-guard.spec.js --grep "without refusal"` | **FAIL** — `mockLattice` 33 ≠ 145 (fluxMock not rebuilt) |
| Overlay set drift (B1) | grep `showStateField`/`toggle-state-field` across `js/` vs `scenario-loader.js:47-119` | 4 new overlays user-facing, absent from loader maps |
| Orphan id (B4) | grep `2-hydrogen-atoms` across `js/` | 3 references, 0 registry/impl entries |
| Counts | grep `case '` / `name == "` / `makeScenario(` + parity inventory | 101/99 raw occurrences, 95/95 unique, 96 registry |

**B3 now runtime-verified (2026-06-05):** `scale0-toggle-leak.spec.js` (2 tests) — langevin does
**not** leak (closed, not reproduced). Everything is now runtime-verified or a direct source read.
