# Telemetry Organization Audit (2026-06-13)

**Scope:** C++ diagnostics/audit structs, CTest NDJSON telemetry, web
`telemetry-hub.js`, panel descriptors, overlay read paths, and mock-bridge
parity.

**Goal:** Single write path (hub), no duplicate panel stacks, documented
energy semantics, and a maintainable module layout.

---

## 1. Four telemetry stacks

| Stack | Write path | Read path | Status |
|-------|------------|-----------|--------|
| **C++ runtime** | `diagnostics_compute.cpp`, `energy_ledger_compute.cpp`, `lagrangian.cpp` | WASM Embind → bridge getters | Canonical physics |
| **Web hub** | `telemetry-hub.js` collectors (`collectScale0*`) | Panels, status bar, overlays via `hub.s0.*` | **Single web write path** (CONTRACTS.md §5) |
| **CTest NDJSON** | `ftd/test_telemetry.h` + `tests/support/test_telemetry.cpp` | FTD Test Bench runner | Isolated from dashboard |
| **Campaign observables** | Per-campaign C++ (e.g. genesis counters) | Analysis docs / CSV | Not wired to web |

---

## 2. Issues found (pre-cleanup)

| # | Issue | Severity | Resolution |
|---|-------|----------|------------|
| 1 | `Diagnostics.total_energy` ≠ `EnergyAudit.total_energy` (Born–Infeld sum vs ½ budget) | Doc | Comment in `render_bridge_diagnostics.h`; mock diag unified with audit (2026-06-13) |
| 2 | Dual diagnostics UI: legacy `DiagnosticsPanel` + `DiagnosticsPanelComponent` | High | **Removed** legacy panel from `app.js` + runtime |
| 3 | Overlays re-called `bridge.getDiagnostics()` bypassing hub | Medium | **Fixed** thermo/time/spectrum via `telemetry/scale0-read.js` |
| 4 | Missing `test_telemetry_snapshot.h` | Medium | **Implemented** RenderBridge overload |
| 5 | FTD-0267 genesis/evaporation counters C++-only | Low | **Open** — WASM export deferred |
| 6 | `getEnergyLedger` WASM-bound, not in wasm-bridge/hub | Low | **Open** |
| 7 | `mock-diagnostics.js` partial audit mirror | Acceptable | Documented in descriptors; not duplicated in UI |
| 8 | Telemetry-grid `CHANNELS['0']` duplicated descriptor trends | Medium | **Extracted** to `telemetry/registry/scale0-grid-channels.js` |
| 9 | Hidden legacy DOM block (~110 lines) in diagnostics template | Low | **Removed** |
| 10 | `pe-telemetry.js` imported canvas Sparkline from legacy `diagnostics.js` | Low | **Moved** to `ui/charts/canvas-sparkline.js` |

---

## 3. Target module layout (implemented)

```
engine/web/js/
  telemetry-hub.js              # hub singleton (unchanged location — wide import graph)
  telemetry/
    index.js                      # barrel: hub + demand + registries
    demand.js                     # Scale-0 on-demand collector gating
    registry/
      scale0-grid-channels.js     # telemetry-grid channel defs (scale 0)
  ui/charts/
    canvas-sparkline.js           # legacy canvas sparklines (PE panel only)
    sparkline.js                  # uPlot sparklines (descriptor trend cells)
```

**Import guidance:** New code should import from `telemetry/demand.js` or
`telemetry/index.js`. Direct `telemetry-hub.js` imports remain valid (large
existing surface).

---

## 4. Energy semantics (load-bearing)

| Field | Struct | Definition |
|-------|--------|------------|
| `total_energy` | `Diagnostics` | Σ \|born_infeld_core\| over lattice — flux proxy, **not** ½·\|J\|² |
| `total_energy` | `EnergyAudit` | `field_energy + wave_energy + particle_ke` (canonical ½ convention) |
| Status bar "Energy" | Web | Maps to `Diagnostics.total_energy` via hub (`s0.diag.totalEnergy`) |
| Conservation panel `E` | Web | Prefers hub diag, falls back to audit `totalEnergy` |

Do not compare Diagnostics and EnergyAudit totals numerically without
knowing which functional is displayed.

---

## 5. Demand gating (Scale 0)

When `PerfFlags.telemetryOnDemand` is true (default):

1. `collectScale0` — always (cheap; status bar + primary ring buffers)
2. `collectScale0Audit` — when diagnostics/charts/lagrangian/grid or conservation overlay visible, and field version changed or panel opened
3. `collectScale0Lagrangian` — when charts/lagrangian/grid visible, same version gate

Logic lives in `telemetry/demand.js`; invoked from
`scales/scale0/runtime/diagnostics.js`.

---

## 6. Descriptor ownership

| Consumer | Source of truth |
|----------|-----------------|
| Diagnostics table rows | `ui/panels/diagnostics-panel/descriptors/scale0.js` |
| Charts panel series | `ui/panels/charts-panel/descriptors/scale0.js` |
| Lagrangian panel rows | `ui/panels/lagrangian-panel/descriptors/scale0.js` |
| Telemetry grid cards (scale 0) | `telemetry/registry/scale0-grid-channels.js` |
| Scales 1–5 grid channels | Inline in `telemetry-grid/component.js` (future: per-scale registries) |

Adding a metric: extend C++ audit if needed → hub collector → descriptor/registry → catalog row in `TELEMETRY_CATALOG_SCALE0.md`.

---

## 7. Remaining open items

| Item | Effort | Notes |
|------|--------|-------|
| Export genesis/evaporation counters to WASM + hub | M | FTD-0267 observation counters |
| Wire `getEnergyLedger` through wasm-bridge + hub | S | Ledger drift row for panels |
| Route conservation/thermo/time overlays fully through hub | M | Some still poll bridge at 2–4 Hz |
| Scale 1–5 grid channel registries | S | Same pattern as scale0 registry |
| Playwright lifecycle re-run after telemetry refactor | S | `lifecycle-harness.spec.js` |

---

## 8. Verification

After changes:

```powershell
# C++ golden + determinism unchanged
cd engine/build
ctest -R "render_bridge_golden|determinism" -C Release --output-on-failure

# Web smoke (manual)
python engine/web/serve.py 8080
# Open Scale 0 → Diagnostics / Charts / Telemetry Grid tabs; confirm live values
```

No physics or golden-hash changes in this cleanup arc.

---

## 9. Related docs

- [`../TELEMETRY_CATALOG_SCALE0.md`](../TELEMETRY_CATALOG_SCALE0.md)
- [`../SPEC_SCALE0_PERF_TELEMETRY_PANELS.md`](../SPEC_SCALE0_PERF_TELEMETRY_PANELS.md)
- [`../../../../CONTRACTS.md`](../../../../CONTRACTS.md) §5 Telemetry Contract
- [`audits/AUDIT_SCALE0_SCENARIO_HARNESS_DRY.md`](AUDIT_SCALE0_SCENARIO_HARNESS_DRY.md)
