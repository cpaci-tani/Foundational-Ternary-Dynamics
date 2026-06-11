# FTD Web Engine — Scale-0 BridgeUI Wiring Audit (2026-06-03)

Focused audit of the Scale-0 bridge read-surface after the WASM64 + worker-physics
upgrade (`38c2bb63`) and the cross-origin-isolation enablement that followed
(`b4844c7a` COOP/COEP server, `cbeabf41` coi-serviceworker). Triggered by a report
that "chart wirings got messed up." Every finding below was verified by reading the
named source (no inferred breakage — cf. the over-count lesson in
`AUDIT_WEB_ENGINE_2026-05-27` follow-up).

**Perimeter:** the Scale-0 bridge read-surface and its UI consumers under the
**worker path** (`MockBridgeProxy`), plus a WASM64 pointer-safety spot-check on the
JS bridge boundary. Other scales, `WasmBridge`/`WebSocketBridge` general coverage,
and the inspector are out of scope this pass.

---

## TL;DR — root cause

The breakage is **not** a WASM64 pointer bug (see A5). It is a **worker-proxy
read-surface gap**:

- Under cross-origin isolation, `flux-*` / `s0-seed-*` / `s0-field-*` scenarios run
  Scale-0 physics in a Web Worker; the main thread reads through
  `MockBridgeProxy` (`bridge/mock-bridge-proxy.js`).
- The proxy wires the **capability layer** (`bridge.capabilities.scale0.*`) to a
  "shadow" `MockBridge` whose flux/state buffers point at the worker's
  SharedArrayBuffers (`mock-bridge-proxy.js:74`). **Capability-path reads work.**
- The proxy forwards only **5 direct reads** (`getFluxVolume`, `getFluxSlice`,
  `getParticleData`, `getDiagnostics`, `inspectVoxel`). Consumers that call
  sampler / audit / particle-list methods **directly on the bridge object** get
  `undefined` / empty — silently blanking the chart. (An *unguarded* direct call
  throws outright; the one such site, `inspectVoxel`, was already patched in
  `68024ba1`. Every other consumer is `?.`- or `typeof`-guarded, so it degrades to
  empty rather than throwing — see A2.)
- The read-surface is hand-maintained and drifts behind the bridge:
  `inspectVoxel` was patched in one-at-a-time in `68024ba1`.

---

## A1 — Canonical Scale-0 read-surface

Defined by `bridge/capabilities/scale0.js` and `bridge/bridge-contract.js`, backed
by `MockBridge`:

| Group | Methods | MockBridge impl |
|---|---|---|
| Field/vector samplers | `getEFieldSampled`, `getBFieldSampled`, `getPoyntingSampled`, `getDivJSampled`, `getFluxVectorSampled`, `getCurlJSampled` | `mock-bridge.js:1600-1606` |
| Scalar samplers | `getVorticitySampled`, `getHelicitySampled`, `getKretschmannSampled`, `getLatencySampled`, `getFisherSampled`, `getCoherenceSampled`, `getStateFieldSampled`, `getGaussResidualSampled` | `mock-bridge.js:1604-1623` |
| Force fields | `getEMForceField`, `getGravityForceField`, `getStrongForceField` | `mock-bridge.js:1619-1621` |
| Audit / Lagrangian | `getEnergyAudit`, `getLagrangian` | `mock-bridge.js:788-789` |
| Per-voxel / per-point | `inspectVoxel`, `getForceAt` | `mock-bridge.js` (1279/1285) |
| Buffers / list | `getScale0LatticeBuffer`, `getScale0FluxBuffer`, `getScale0WaveBuffer`, `getScale0ParticleList` | `mock-bridge.js` (980-1010) |
| Volume / slice / frame / diag | `getFluxVolume`, `getFluxSlice`, `getParticleData`, `getDiagnostics` | `mock-bridge.js` |

The `WasmBridge` mirrors this surface through `_wasmCallOr(...)` typed-view returns
(`wasm-bridge.js:400-499`, `344/355/370`).

---

## A2/A3 — Consumer inventory + coverage matrix

"Access" = how the consumer reaches the method. **Capability** = via
`…capabilities.scale0.*` (proxy-safe, shadow-wired). **Direct** = called on the raw
bridge/fluxMock object (proxy-gap). "Worker status" is the behaviour when the
fluxMock is the `MockBridgeProxy`.

| Consumer (file:line) | Method | Access | Worker status | Tier |
|---|---|---|---|---|
| `scales/scale0/runtime/field-overlays.js:51-87` | all field samplers | Capability |  works (shadow/SAB) | — |
| `telemetry-hub.js:164` `collectScale0` | `getScale0Diagnostics` | Capability (proxy overrides → `_lastDiag`) |  works | — |
| `telemetry-hub.js:201/256` audit+Lagrangian | `getScale0EnergyAudit` / `getScale0Lagrangian` | Capability → shadow |  field terms live; **particle terms empty** (shadow `_particles=[]`) | 2 |
| `scales/scale0/ui/overlays/flux-slice-panel.js:91/101/111/121` | `getEFieldSampled` / `getBFieldSampled` / `getPoyntingSampled` / `getDivJSampled` | **Direct** (`?.`) |  **blank |E| |B| |S| ∇·J slices** | 1 |
| `scales/scale0/ui/overlays/spectrum-panel.js:88` | `getScale0ParticleList` | **Direct** (`?.`) |  empty spectrum | 2 |
| `scales/scale0/ui/overlays/p1-observables-panel.js:780` | `getLatencySampled` | **Direct** (`?.`) |  empty (graceful) | 1 |
| `p1-observables-panel.js:1084/1262/1302/1327` | `getScale0ParticleList` | **Direct** (`?.`) |  empty particle reads | 2 |
| `physics/physics-harness.js:107/117` | `getEnergyAudit` | **Direct** (`?.`) |  null (graceful) | 1/2 |
| `physics/physics-harness.js:138` | `getScale0ParticleList` | **Direct** (`?.`) |  empty | 2 |
| `physics/physics-harness.js:167/172/177` | `getEFieldSampled` / `getBFieldSampled` / `getLatencySampled` | **Direct** (`?.`) |  empty (graceful) | 1 |
| `physics/physics-harness.js:201` | `getEFieldSampled(1)` | **Direct** (`typeof` guard at :200) |  null before fix (guard short-circuits) | 1 |
| `inspector/scales/lattice.js:102` | `getForceAt` | **Direct** (`typeof` guard) |  no force (guarded) | 2 |

**Tier 1 (field/flux/state-derived):** computable on the shadow straight from the
SAB buffers → fixable by forwarding the direct read to `this._shadow`.

**Tier 2 (particle-dependent):** `getScale0ParticleList`, the particle terms of
`getEnergyAudit`/`getLagrangian`, and `getForceAt`'s particle contribution. The
shadow's `_particles` is `[]`; the worker owns particles and ships only the
render-frame `parts` (`mock-bridge.worker.js:34-49`), never a list. Needs a
worker-sourced particle list.

---

## A4 — Harness/bridge resolution (verify-before-claiming)

`getPhysicsHarness(bridge)` (`physics/index.js:25-31`) attaches one harness to
**whatever bridge it is handed**. The Scale-0 overlays hand it their active bridge,
and that accessor returns the fluxMock proxy under a worker scenario:

```js
// p1-observables-panel.js:1397-1402
const getBridge = () => { … if (state?.useFluxMock && state?.fluxMock) return state.fluxMock; … };
```

So under a worker flux scenario the harness **wraps the `MockBridgeProxy`**, and its
direct reads (`getEnergyAudit`, `getScale0ParticleList`, `getEFieldSampled`) run on
the proxy and returned empty/null before this fix. (The `:201` call is **guarded** by
a `typeof` check at `:200`, and the `sampleVAtRay` probe at `:350` likewise — both
degrade to null rather than throwing, unlike the unguarded `inspectVoxel` call already
patched in `68024ba1`.) B1 (covering the proxy read-surface) therefore repairs the
harness reads with no harness edit required.

---

## A5 — WASM64 pointer-safety spot-check

**Clean.** The build sets `-sMEMORY64=1 -sWASM_BIGINT=1`
(`engine/wasm/CMakeLists.txt:97`); `wasm-bridge.js` selects the wasm64 module via
`supportsMemory64()`. A scan of `bridge/wasm-bridge.js` finds **no** manual JS
pointer arithmetic (`>> 2`, `* 4 + off`, `getValue(ptr,…)`, raw `HEAP*[]` indexing,
`.subarray`); every heap read returns an Emscripten `typed_memory_view`, which is
memory-model-agnostic. No 64-bit-pointer wiring hazard on the JS side.

**Deferred (flag, not fixed):** `peGetExtendedData()` is stubbed to `null`
(`wasm-bridge.js`), so the Scale-1 PE extended-data path (particle table / orbital
mechanics in `pe-telemetry.js`) has no live 64-bit binding yet. When that binding
lands it must return via `typed_memory_view` (not raw pointers) and be re-checked.

---

## Fix mapping

| Fix | Closes |
|---|---|
| **B1** generic read-delegation on `MockBridgeProxy` (forward the canonical read-surface to the shadow; mutators still post to the worker) | flux-slice E/B/S/∇·J, p1 latency, harness field reads, field terms of audit/Lagrangian — all Tier-1 rows |
| **B2** worker-sourced particle list (`postFrame` payload → proxy `_lastParticleList` → `getScale0ParticleList`) | spectrum panel, p1 particle reads, harness particle reads, particle terms of audit — Tier-2 rows |
| **B3** — *no edit needed*: `physics-harness.js:201` is already guarded at `:200` (as is `sampleVAtRay` at `:350`); B1/B2 restore the data path. No unguarded throw remains in the direct-read set. | — |
| **B4** canonical direct-read list in `bridge-contract.js` consumed by B1 + the contract test | future drift (new sampler not forwarded → CI fails, not a silent blank chart) |

**Verification:** Playwright `scale0-worker.spec.js` under `serve.py 8081 --cache`
(COOP/COEP) — see `engine/web/PLAN_SCALE0_PHYSICS_WORKER.md` and the plan file for
this work.
