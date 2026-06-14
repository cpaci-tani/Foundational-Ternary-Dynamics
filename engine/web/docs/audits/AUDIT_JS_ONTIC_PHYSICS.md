# JS Ontic-Physics Audit (2026-06-13)

**Scope:** all 309 `.js` files under `engine/web/js/`  
**Question:** do they cohere with FTD's two-layer ontology, epistemic discipline, and
active web contracts (`CONTRACTS.md`, `SPEC_SCALE0_*`, telemetry hub)?  
**Method:** full-tree grep + targeted reads of bridge, scale controllers, overlays,
pedagogy modules, and constants SSOT. Cross-checked against
[`AUDIT_WEB_ENGINE_2026-05-27.md`](AUDIT_WEB_ENGINE_2026-05-27.md) and
[`AUDIT_TELEMETRY_ORGANIZATION.md`](AUDIT_TELEMETRY_ORGANIZATION.md).

**Verdict summary**

| Tier | Count (approx.) | Meaning |
|------|-----------------|--------|
| **A — aligned** | ~240 files | UI shell, viewport renderers, descriptors, most bridge scenarios |
| **B — acceptable with caveats** | ~45 files | Pedagogy copy, scale 1–6 simulators, intentional mock simplifications |
| **C — drift / fix queue** | ~24 files | Energy-channel mismatch, hub bypass, epistemic tag errors, P0 tickets |

No file was found that contradicts the **five postulates** at the code level; drift is
in **telemetry semantics**, **epistemic labeling**, and **sim-vs-theory boundaries**.

---

## 1. Ontic reference model (what “logical ontic physics” means here)

FTD's web dashboard implements a **discrete ontology + computational EFT**, not a
continuous QFT port:

| Layer | Ontic role | Web representation |
|-------|------------|-------------------|
| **State `s ∈ {−1,0,+1}`** | Manifestation (actual) | Particle list, ternary overlays, inspector |
| **Flux `J ∈ ℝ³`** | Disposition (potential flow) | `_fluxJ` grids, field overlays, wave engine |
| **Genesis** | Manifestation event (s: 0→±1) | Toggle-gated; cluster counters C++-only today |
| **Gauss projection** | Coupling flux ↔ state | Audit `gaussViolation`; non-variational leak documented |
| **Tick phases** | Native discrete time | 10-phase cycle in constants + C++ parity |

**Rules for “making sense” in JS:**

1. Do not treat `|J|²` dashboards as identical to Born probabilities without the
   `[SELECTION]` / lattice-bias caveat.
2. Do not conflate **sim units** (E*, implicit k_B=1 at scales 2–4) with MeV/Kelvin
   without labeling.
3. Prefer **hub → harness → capabilities → bridge** read order on Scale 0.
4. Epistemic tags in UI must match `LEDGER` / `config/scenarios.js` — not promote
   conjectures to `[THEOREM]`.

---

## 2. Architecture compliance

### 2.1 Single write path (telemetry)

| Pattern | Status | Files |
|---------|--------|-------|
| Scale-0 tick collection via hub | **PASS** | `scales/scale0/runtime/diagnostics.js`, `telemetry/demand.js` |
| Panel descriptors read hub | **PASS** | `ui/panels/diagnostics-panel/`, `charts-panel/`, `lagrangian-panel/`, `telemetry-grid/` |
| Overlay panels poll bridge directly | **DRIFT** | See §4 |
| Mock flux-owner vs idle bridge | **DOCUMENTED** | `conservation-micropanel.js`, `state/store.js` |

### 2.2 PhysicsHarness as canonical Scale-0 surface

| Consumer | Uses harness? | Notes |
|----------|---------------|-------|
| Scenario loaders (`vacuum`, `s0-seed`, …) | **YES** | Correct — mutations via harness setters |
| Conservation micropanel | **YES** | Hub-first `getConservationTotals()` |
| P1 observables (Coulomb, Bell, …) | **PARTIAL** | Mix of harness + direct bridge reads |
| Thermo / time / spectrum overlays | **NO** | Direct `bridge.getDiagnostics()` |

### 2.3 Capability factory vs raw bridge

**PASS:** Most scale controllers use `bridge.capabilities.scaleN.*`.  
**FAIL (known):** `WebSocketBridge` lacks capability getter (AUDIT P0-4).  
**DRIFT:** `spectrum-comparator.js` reads `bridge._fluxJ`, `bridge._toggles` directly
(violates CONTRACTS §1 rule 5 — acceptable only inside bridge/scenario impl, not
overlay consumers).

### 2.4 Bridge state contract (live reference)

**PASS:** `mock-diagnostics.js`, `mock-lattice-samplers.js` keep live `state` ref.  
**DRIFT:** `physics-harness.js` reads `_toggles`, `_particles` for toggle/inject
papering — documented escape hatch; should shrink over time.

---

## 3. Energy and flux semantics (highest-impact physics drift)

### 3.1 Three “total energy” channels on Scale 0

| Channel | Definition | Where |
|---------|------------|-------|
| **C++ `Diagnostics::total_energy`** | Σ \|born_infeld_core\| | Native WASM before adapter |
| **C++ `EnergyAudit::total_energy`** | ½\|J\|² + ½\|w\|² + particle KE | `diagnostics_compute.cpp` |
| **Mock `getDiagnostics().totalEnergy`** | field + wave + particle KE (matches audit) | `mock-diagnostics.js` |
| **WASM adapter `totalEnergy`** | Rewritten from audit; baseline → `vacuumBaselineEnergy` | `wasm-bridge.js` |

**Contract intent** (`CONTRACTS.md` §5.2): dashboard physical energy = audit channel.  
**Status (2026-06-13):** MockBridge diag and audit totals are aligned; hub/overlays
read via `telemetry/scale0-read.js`. Native WASM still exposes Born–Infeld baseline
separately as `vacuumBaselineEnergy`.

### 3.2 Flux vs energy naming

| File | Issue |
|------|-------|
| `ontic-observatory.js` | Scales 1/2 use separate `activityLabel` (fixed 2026-06-13) |
| `mock-diagnostics.js:288` | `totalFlux` derived from RMS of field energy, not Σ\|J\| |
| `knowledge-base/data.js` | Correctly separates ternary state vs flux J |

### 3.3 Constants that must stay aligned

| Constant | SSOT | Drift instances |
|----------|------|-----------------|
| `C_SPEED = 1/√3` | `constants.js` | `thermo-panel.js` uses literal `C2=1/3` (same value, wrong import) |
| `C_SPEED` | `constants.js` | `molecules.js`, `meta-unit.js`, `field-renderer.js` use `1/Math.sqrt(3)` |
| `G_HELIOCENTRIC` | `constants.js:275` | Scale 4 mock still uses `G_N=0.01` (P0-1) |
| Cosmology fractions | `constants.js` | Scale 5 scenarios vs panel labels (P0-8) |

~55 files import `constants.js` correctly; ~15 pedagogy/config files use literals
for display-only copy (acceptable if not used in computation).

---

## 4. Overlay panel read paths (hub bypass map)

Self-polling overlays on Scale 0 — **ontically fine to sample fields**, but
**diagnostic scalars should prefer hub** to avoid duplicate O(N³) work and
mock/WASM split-brain:

| Panel | Rate | Reads | Hub? | Ontic note |
|-------|------|-------|------|------------|
| `thermo-panel.js` | 4 Hz | hub via `scale0-read` + field slice | **Yes** | Thermodynamic **[MEASURED]** labels OK |
| `time-panel.js` | 2 Hz | hub diag + gravity agg samples | **Yes** (diag) | Latency = real Poisson field ✓ |
| `spectrum-panel.js` | 2 Hz | hub diag/audit + FFT samplers | **Yes** | Spectroscopy = flux sector ✓ |
| `gravity-panel.js` | 4 Hz | field samples, metric agg | No | Real `voxel.latency` path ✓ |
| `flux-slice-panel.js` | rAF | field slices + tick diag | No | Field visualization ✓ |
| `conservation-micropanel.js` | 4 Hz | harness totals | **Partial** | Correct pattern |
| `p1-observables/*` | rAF | bridge + harness mix | No | EM/Bell = emergent overlays |
| `wave-lab-panel.js` | rAF | spectrum comparator metrics | No | Wave sector ✓ |

**Recommendation:** route scalar rows (E, T_kin, drift, tick) through
`telemetryHub.s0` or `PhysicsHarness`; keep field samplers on capabilities.

---

## 5. Epistemic tag audit (UI / scenarios / constants)

### 5.1 Correctly disciplined (examples)

- `config/scenarios.js` — header warns against promoting mass IDs to `[THEOREM]`
- `ui/app-ontic.js` — X_MINUS labeled RETIRED for N_c identification
- `ontic-observatory.js` — cpaci-tani propositions demoted from “Theorem”
- `faq/data.js` — Born rule, Bell, Ω_Λ tagged `[SELECTION]` / `[PARAMETRIC]` with honesty blocks
- `atlas-content.js` — Born rule marked unresolved
- `p1-observables/bell.js` — classical ≤ 2 vs Tsirelson labeled, not claimed derived

### 5.2 Tag violations found (fix queue)

| File | Line / context | Claim | Should be |
|------|----------------|-------|-----------|
| `bridge/scenarios/vacuum-scenarios.js` | μ/e, τ/e comment | ~~`[THEOREM]`~~ | **Fixed** → `[STRONGLY MOTIVATED CONJECTURE]` |
| `constants.js` | Ω_Λ, DM_FRACTION tags | `[THEOREM]` | `[PARAMETRIC]` / `[SELECTION]` (P0-15; FAQ contradicts) |
| `meta-pedagogy.js` | symmetry order 1296 | `[THEOREM]` | verify against spine before display |

### 5.3 Pedagogy vs engine (intentionally separate)

These **correctly** use MeV/Kelvin as teaching units, not lattice E*:

- `spectroscopy.js`, `particle-catalog.js`, `pe-telemetry.js`, `units.js`
- `ui/components/knowledge-base/data.js` — narrative glossary, not runtime physics

---

## 6. Module tier assessment (by folder)

### Tier A — aligned with ontic physics

| Folder | Files | Notes |
|--------|-------|-------|
| `telemetry/` | 3 | Demand gating + registry; hub contract |
| `ui/panels/diagnostics-panel/` | ~8 | Descriptor-driven; documents scenario-conditional metrics |
| `ui/panels/charts-panel/`, `lagrangian-panel/` | ~10 | Hub-backed |
| `bridge/scenarios/` (most) | 11 | Harness + helpers; vacuum honesty on visualization |
| `scales/scale0/runtime/` | 11 | Tick/frame-sync/diagnostics pipeline |
| `physics/` | 2 | Harness canonical surface |
| `config/toggles.js` | 1 | Mirrors C++ `TermToggles` |
| `viewport/` (render) | 12 | Visualization only; no false derivation claims |

### Tier B — simulators / pedagogy (honest boundaries)

| Folder | Ontic role |
|--------|------------|
| `bridge/mock-particle-engine.js`, `mock-atom-engine.js` | Scale 1–3 **EFT simulators** — not substrate derivations |
| `bridge/mock-scale4.js`, `mock-scale5.js` | N-body demos; several P0 unit/physics gaps |
| `pe-telemetry.js`, scale1/2 controllers | MeV labeled; PE physics separate from lattice |
| `ui/components/faq/`, `knowledge-base/` | Epistemic glossary — generally honest |
| `meta-pedagogy.js`, `atlas/` | Conceptual maps |

### Tier C — fix queue (ontology or contract drift)

| File | Issue |
|------|-------|
| `bridge/mock-diagnostics.js` | Diag vs audit energy split (§3.1) |
| `bridge/wasm-bridge.js` | Adapter fixes WASM; mock still diverges |
| `scales/scale0/ui/overlays/thermo-panel.js` | Hub bypass + hardcoded C2 |
| `scales/scale0/ui/overlays/time-panel.js` | Hub bypass |
| `ontic-observatory.js` | `_totalFlux` naming |
| `bridge/scenarios/vacuum-scenarios.js` | Epistemic overclaim (§5.2) |
| `bridge/scenarios/spectrum-comparator.js` | Private `_fluxJ` access from scenario code |
| `telemetry-hub.js` | Scale 5 Hubble key mismatch (P0-3) |
| `bridge/mock-scale4.js` | Heliocentric G (P0-1) |
| `bridge/mock-scale5.js` | Cosmology static H(t), mass labels (P0-6, P0-9) |
| `ws-bridge.js` | Missing capabilities (P0-4) |

---

## 7. Two-layer ontology spot checks

| Check | Result |
|-------|--------|
| Genesis treated as manifestation event, not continuous field | **PASS** in toggles + scenarios |
| Flux overlays distinguish E = −ẇ, B = curl J | **PASS** in descriptors + tooltips |
| State s drives charge/Gauss source | **PASS** in audit + mock field-derived cache |
| Dual substrate (J_L, J_R) gated by toggle | **PASS** — rows hidden when off |
| Void ≠ “empty continuum” | **PASS** — ternary 0 is void, undefined-boundary in docs |
| α derived in UI | **NOT CLAIMED** — fine-structure panel uses campaign residuals |
| MC-T4.3 α mechanism | **NOT CLAIMED** — G_C = √α shown as EFT readout |

---

## 8. Cross-reference: open P0 tickets from 2026-05-27 audit

Still unresolved in JS (verify before claiming “fixed”):

| ID | Summary |
|----|---------|
| P0-1 | Scale 4 heliocentric G |
| P0-3 | Scale 5 Hubble telemetry key |
| P0-4 | WebSocketBridge capabilities |
| P0-5 | Scale 4 pause snapshot |
| P0-6–9 | Scale 5 mass/H(t)/DM fraction presentation |
| P0-10–13 | Scale 2–3 unit/bond rendering |
| P0-14 | X_MINUS display (partially fixed in app-ontic) |
| P0-15 | constants.js cosmology tags vs FAQ |
| P0-16 | Born scenario tag (fixed in scenarios.js; verify UI) |
| P0-17 | Scale 6 BCC/FCC label swap |

Full ticket table: [`AUDIT_WEB_ENGINE_2026-05-27.md`](AUDIT_WEB_ENGINE_2026-05-27.md).

---

## 9. Recommended fix order (ontic-physics priority)

1. ~~Unify Scale-0 energy channel on MockBridge~~ — **DONE 2026-06-13**
2. ~~Fix epistemic tag in `vacuum-scenarios.js`~~ — **DONE 2026-06-13**
3. ~~Route overlay scalar polls through hub/harness~~ — **DONE 2026-06-13**
4. ~~Replace thermo `C2` literal~~ — **DONE 2026-06-13**
5. Cosmology constants — **already retagged** in `constants.js` (P0-15)
6. **Close P0 scale 4/5 physics tickets** — still open (see §8)
7. **Export FTD-0267 genesis counters to WASM/hub** — still open

---

## 10. Remediation log (2026-06-13)

| Change | Files |
|--------|-------|
| Mock diag `totalEnergy` = field + wave + particle KE | `bridge/mock-diagnostics.js` |
| Hub-first read helpers | `telemetry/scale0-read.js`, `telemetry/index.js` |
| Overlay panels use hub snapshots | `thermo-panel.js`, `time-panel.js`, `spectrum-panel.js` |
| Conservation E prefers audit | `physics-harness.js` |
| Epistemic tag fix (μ/τ mass ratios) | `bridge/scenarios/vacuum-scenarios.js` |
| Ontic observatory activity label | `ontic-observatory.js` |
| C² from `C_SPEED` | `thermo-panel.js`, `molecules.js`, `viewport/field-renderer.js` |
| Stale conservation header | `conservation-micropanel.js` |

---

## 11. Files that need no ontic changes

Shell/UI chrome (`ui/shell/`, `ui/components/play-bar/`, floating windows),
inspector scaffolding, audio synth, verify panel, most CSS-adjacent templates,
and test helpers — **309 files minus ~24 in Tier C** are structurally fine.

---

## 12. Related documents

- [`TELEMETRY_CATALOG_SCALE0.md`](../TELEMETRY_CATALOG_SCALE0.md)
- [`AUDIT_TELEMETRY_ORGANIZATION.md`](AUDIT_TELEMETRY_ORGANIZATION.md)
- [`AUDIT_WEB_ENGINE_2026-05-27.md`](AUDIT_WEB_ENGINE_2026-05-27.md)
- [`SPEC_SCALE0_BRIDGE_ARCHITECTURE.md`](../SPEC_SCALE0_BRIDGE_ARCHITECTURE.md)
- [`../../../CONTRACTS.md`](../../../CONTRACTS.md)
- Project epistemic rules: `CLAUDE.md` Epistemic Discipline

---

**Audit status:** remediation pass 2026-06-13 applied for Scale-0 energy unification,
hub-first overlay reads, epistemic tag fix, and C_SPEED consistency. Scale 4/5 P0
tickets and genesis-counter WASM export remain open.
