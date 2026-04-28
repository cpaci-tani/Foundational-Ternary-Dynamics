# Viewport refactor map — Phase 3 extraction guide [CLOSED 2026-04-27]

> **STATUS: CLOSED.** Phase 3 of the refactor sweep completed in 5 commits
> (8b4732d, 1506079, 1499a11, 506805b, plus 848e839 for this map). The
> guide below is preserved as historical reference for the design decisions
> made during the in-flight extraction. The current state of the viewport
> module is documented in [META_PROJECT_ATLAS.md](../../../../META_PROJECT_ATLAS.md) §2,
> [CONTRACTS.md §10–11](../../../../CONTRACTS.md), and ADR-0001/0010/0011.
>
> Final viewport.js: **1256 LOC orchestrator** (was 3953). Sub-renderers:
> scene-core.js (500), flux-renderer.js (416), particle-renderer.js (503),
> field-renderer.js (2273). All pre-existing modules (molecular-renderer,
> boundary-geometry, topology-sheet-renderer, color-ramps, spin-arrow-manager)
> remained as delegators per the plan.

**Authoritative reference for Phase 3 of `.claude/plans/i-want-to-try-crispy-charm.md`.**

`engine/web/js/viewport.js` is 3953 LOC with 169 methods on a single
`Viewport` class. This document maps every method to a target sub-renderer
or to the orchestrator (the slimmed Viewport class), enumerates the
`this.xxx` state each method reads/writes, and records the design
decisions that reduce silent-breakage risk during extraction.

## Already-extracted modules (preserve as delegators)

| Module | File | Purpose |
|---|---|---|
| `MolecularRenderer` | `viewport/molecular-renderer.js` | Bonds, orbital shells, AE forces, labels |
| `SpinArrowManager` | `viewport/spin-arrow-manager.js` | Tracked-particle spin arrows |
| `TopologySheetRenderer` | `viewport/topology-sheet-renderer.js` | 10 topology sheets + grav potential |
| `boundary-geometry.js` | `viewport/boundary-geometry.js` | `buildBoundary`, `insideBoundary` (pure) |
| `color-ramps.js` | `viewport/color-ramps.js` | Stateless colormap functions |
| `field-overlays.js` | `scales/scale0/runtime/field-overlays.js` | Overlay computation (consumed by Viewport) |

Viewport delegates to all five — keep these delegations as one-line
forwarders during extraction. Do NOT re-extract their methods.

## Cross-cutting state (lives on orchestrator)

| Field | Read by | Write by | Notes |
|---|---|---|---|
| `scene` | All sub-renderers (mesh add/remove) | Constructor only | Three.js root |
| `camera` | 3a, 3d | Constructor + camera presets | |
| `renderer` | 3a only (render, resize) | Constructor only | |
| `controls` | 3a only | Constructor only | OrbitControls |
| `latticeSize` | All | Ctor + `setLatticeSize` | Synchronized via `onLatticeSizeChanged()` callback |
| `_halfN` | All | Ctor + `setLatticeSize` | Cached `latticeSize/2` |
| `_boundaryShape` | 3a, 3b, 3c (clipping) | Ctor + `setBoundaryShape` | |
| `_boundaryMode` | 3a, 3b, 3c | Ctor + `_buildBoundary` | `'lattice' | 'origin'` |
| `_engineMode` | 3a (axes/grid toggles, scenario scale) | Ctor + `setEngineMode` | `'lattice' | 'pe' | 'ae' | 'consciousness' | 'cosmic' | 'meta'` |
| `_insideBoundary(nx,ny,nz)` | 3b, 3c, 3d (mesh clipping) | — | Method delegating to boundary-geometry.js |
| `visualSettings` | 3d | Ctor + `setOpacity` | `{globalScale, manifestedSize, voidSize, opacity}` |

## `setLatticeSize` mega-cascade contract

`setLatticeSize(size)` (currently 71 LOC at line 417-488) cascades through
every sub-renderer's meshes. Design decision: **the orchestrator owns the
cascade**. After Phase 3:

```js
setLatticeSize(size) {
    this.latticeSize = size;
    this._halfN = size * 0.5;
    // Sub-renderers each rebuild their meshes:
    this._sceneCore.onLatticeSizeChanged(size);     // boundary, axes, void box
    this._fluxRenderer.onLatticeSizeChanged(size);  // flux volume, slice, streamlines
    this._fieldRenderer.onLatticeSizeChanged(size); // 27 field meshes
    this._particleRenderer.onLatticeSizeChanged(size); // particles, trails, vectors
    // Already-extracted:
    this._molRenderer?.onLatticeSizeChanged?.(size);
    this._topoRenderer?.onLatticeSizeChanged?.(size);
    this.spinArrowManager?.dispose(); // rebuild on next update
    // Camera + scenario scale:
    this._applyScenarioScale();
    if (this._boundaryMode === 'lattice') { /* recenter camera */ }
}
```

Every sub-renderer MUST implement `onLatticeSizeChanged(size)` even if
it's a no-op (with a comment). Missing the callback = silent stale geometry.

## Sub-renderer assignments

### 3a — `viewport/scene-core.js` (~600 LOC, 27 methods)

**Owns:** camera, lights, scene lifecycle, boundary/axes/grid, render loop,
post-processing, dispose.

**Methods to MOVE (with line ranges):**
- Constructor 197-290 (the parts that set up scene/camera/renderer/controls/wireframe/axes/post-processing — split from constructor; particle init goes to 3d)
- `_disposeBoundary` 326-335
- `_buildBoundary` 337-377 (delegates to `boundary-geometry.js`)
- `setBoundaryShape` 384-386
- `_insideBoundary` 396 (also delegates)
- `_buildAxes` 398-415
- `toggleWireframe` 490-493
- `setCameraPreset` 508-530
- `zoomToFit` 531-553 (reads `_fluxVolume` — needs callback to FluxRenderer for fitness check, OR orchestrator owns the fit logic)
- `setWireframeBrightness` 554-563
- `toggleAxes` 564-574
- `setVoxelHighlight` 575-595
- `setSymmetryHighlights` 596-639
- `toggleGrid` 640-654
- `_buildPEAxes` 3617-3672
- `enablePostProcessing` 3761-3782
- `disablePostProcessing` 3783-3790
- `getBloomPass` 3791-3797
- `setBloomParams` 3798-3804
- `render` 3806-3828
- `_onResize` 3830-3841
- `dispose` 3843-3952 (large; calls every sub-renderer's `dispose()`)

**State owned:** `wireframe`, `_wireframeBrightness`, `_showAxes`, `_showGrid`,
`axes`, `peAxes`, `peGrid`, `_voxelHighlight`, `_symHighlights`, `_composer`,
`_bloomPass`, `_usePostProcessing`, `_resizeObserver`.

**Constructor:** `new ViewportSceneCore({ scene, camera, renderer, controls, container, latticeSize, halfN, boundaryShape, boundaryMode, engineMode })`.

### 3b — `viewport/flux-renderer.js` (~200 LOC, 14 methods)

**Owns:** flux volume, flux slice, flux streamlines.

**Methods to MOVE:**
- `_buildFluxVolume` 1116-1154
- `updateFluxVolume` 1163-1282 (large — boundary-clipping + threshold)
- `updateFluxSlice` 1283-1339 (uses `_fieldHeatmap` from 3c — needs cross-renderer reference)
- `toggleFluxVolume` 1340-1346
- `toggleFluxSlice` 1347-1355
- `setFluxOpacity` 1356-1360
- `setFluxShape` 1361-1365
- `setFluxPointScale` 1366-1370
- `setFluxThreshold` 1371-1375
- `setScenarioScale` 1376-1387
- `setFluxLatticeSpacing` 1388-1400
- `_buildFluxStreamlines` 1749-1753
- `updateFluxStreamlines` 1754-1763
- `toggleFluxStreamlines` 1764-1771

**State owned:** `_fluxVolume`, `_fluxVolumeSize`, `_fluxStreamlines`,
`_fluxPointScale`, `_fluxThreshold`, `_scenarioScale`, `showFlux`.

**Cross-renderer dependency:** `updateFluxSlice` writes to `_fieldHeatmap`
(owned by 3c). Resolution: pass a getter `() => fieldRenderer.heatmapMesh()`
into FluxRenderer's constructor, OR leave `updateFluxSlice` on the
orchestrator as a thin wrapper that calls `fieldRenderer.writeFluxSlice(...)`.
Pick at extraction time.

**Constructor:** `new ViewportFluxRenderer({ scene, latticeSize, halfN, boundaryShape, insideBoundary, getFieldHeatmap })`.

### 3c — `viewport/field-renderer.js` (~1800 LOC, 66 methods)

**Owns:** all field overlays (E, B, Poynting, divergence, force volumes,
gravity, strong/weak, dark matter, damping, genesis, confinement, dual
flux, chirality, light, horizon, plus quantum field / phase / Lagrangian /
entropy density).

**Methods to MOVE** (grouped):
- Field heatmap: `_buildFieldHeatmap` 817, `updateFieldHeatmap` 842, `toggleFieldHeatmap` 868
- Field vectors: `_buildFieldVectors` 876, `updateFieldVectors` 892, `toggleFieldVectors` 929
- PE streamlines: `_buildPEStreamlines` 937, `updatePEStreamlines` 955, `togglePEStreamlines` 991
- Gravity vectors (PE): `_buildGravityVectors` 1003, `updateGravityVectors` 1007, `toggleGravityVectors` 1043
- E-field lines: `_buildEFieldLines` 1559, `updateEFieldLines` 1564, `toggleEFieldLines` 1572
- B-field lines: `_buildBFieldLines` 1579, `updateBFieldLines` 1584, `toggleBFieldLines` 1592
- Poynting: `_buildPoyntingVectors` 1599, `updatePoyntingVectors` 1617, `togglePoyntingVectors` 1664
- Divergence: `_buildDivergenceField` 1671, `updateDivergenceField` 1695, `toggleDivergenceField` 1742
- EM force volume: `_buildForceVolume` 1772, `updateForceVolume` 1776, `toggleForceVolume` 1785, `updateEMForceField` 1868, `showEMForce` 1869
- Gravity field: `_buildGravityField` 1792, `updateGravityField` 1810, `toggleGravityField` 1861, `updateGravityForceField` 1872, `showGravityForce` 1873
- Strong force: `_buildStrongForce` 1877, `updateStrongForceField` 1881, `toggleStrongForce` 1890, `showStrongForce` 1896
- Weak force: `_buildWeakField` 1920, `updateWeakField` 1946, `toggleWeakField` 1995, `showWeakField` 2001
- Force heatmap: `_buildForceHeatmap` 2011, `initForceHeatmap` 2063, `updateForceHeatmap` 2065, `showForceHeatmap` 2114
- Force streamlines: `_buildForceStreamlines` 2121, `initForceStreamlines` 2152, `updateForceStreamlines` 2159, `animateForceStreamlines` 2201, `showForceStreamlines_vis` 2209
- Force glyphs: `_buildForceGlyphMesh` 2228, `_ensureForceGlyphInfra` 2260, `_buildForceGlyphs` 2279, `initForceGlyphs` 2280, `updateForceGlyphs` 2282, `showForceGlyphs` 2377
- Force-style helpers: `hideAllForceStyles` 2397, `showArrowForces` 2416
- Dark matter halo: `_buildDarkMatterHalo` 2424, `updateDarkMatterHalo` 2447, `toggleDarkMatterHalo` 2494
- Event horizon: `_buildEventHorizon` 2502, `setEventHorizon` 2531
- Damping zones: `_buildDampingZones` 2545, `updateDampingZones` 2563, `toggleDampingZones` 2604
- Genesis isosurface: `_buildGenesisIsosurface` 2611, `updateGenesisIsosurface` 2634, `toggleGenesisIsosurface` 2677
- Confinement: `_buildConfinementStrings` 2687, `updateConfinementStrings` 2705, `toggleConfinement` 2774
- Dual flux: `_buildDualFluxVolume` 2781, `updateDualFluxVolume` 2805, `toggleDualFluxVolume` 2868
- Chirality: `_buildChiralityField` 2875, `updateChiralityField` 2899, `toggleChiralityField` 2944
- Light field: `_buildLightField` 2951, `updateLightField` 2975, `toggleLightField` 3022
- Quantum scaffolding: `_buildSoftDiscTexture` 3036, `_buildQuantumField` 3059, `_quantumSetVisibility` 3092, `_populateQuantumField` 3105, `_animateQuantumField` 3419
- Quantum overlays: `togglePsiSquaredField` 3150, `updatePsiSquaredField` 3155, `_buildPhaseNeedles` 3171, `togglePhaseField` 3194, `updatePhaseField` 3202, `toggleLagrangianDensityField` 3247, `updateLagrangianDensityField` 3252, `toggleEntropyDensityField` 3270, `updateEntropyDensityField` 3276
- Horizon field: `_buildHorizonField` 3353, `toggleHorizonField` 3372, `updateHorizonField` 3377

**Helper methods (shared by 3b/3c — keep in 3c, re-export for 3b):**
- `_buildStreamlineMesh` 1423-1449 (factory)
- `_buildArrowFieldMesh` 1450-1479 (factory)
- `_writeArrowFieldIntoMesh` 1480-1523 (used by E, B, gravity, forces)
- `_writeStreamlinesIntoMesh` 1524-1558 (used by E, B, PE)

**State owned:** all `_eFieldLines`, `_bFieldLines`, `_poyntingVectors`,
`_divField`, `_forceVolume`, `_gravityField`, `_strongForce`, `_weakField`,
`_forceHeatmap`, `_forceStreamlinePool`, `_forceGlyphMeshes`,
`_darkMatterHalo`, `_eventHorizonSphere`, `_eventHorizonRing`,
`_dampingZones`, `_genesisIsosurface`, `_confinementStrings`,
`_dualFluxVolume`, `_chiralityField`, `_lightField`, `_quantumField`,
`_quantumFieldTexture`, `_quantumAnimationClock`, `_phaseNeedles`,
`_horizonField`, `_fieldHeatmap`, `_fieldVectors`, `_peStreamlines`,
`_gravityVectors`, `_magCache`, `showHeatmap`.

**Constructor:** `new ViewportFieldRenderer({ scene, latticeSize, halfN, boundaryShape, insideBoundary })`.

### 3d — `viewport/particle-renderer.js` (~300 LOC, 13 methods)

**Owns:** particle positions, trails, velocity vectors, per-particle force
vectors. **Delegates to** MolecularRenderer for atoms/bonds/orbitals/labels.

**Methods to MOVE:**
- `_initParticles` 292-322
- `_buildVelocityVectors` 655-671
- `updateVelocityVectors` 672-712
- `toggleVelocityVectors` 713-719
- `_buildTrails` 720-737
- `updateTrails` 738-795
- `toggleTrails` 796-801
- `clearTrails` 802-811
- `_buildParticleForces` 1051-1067
- `updateParticleForces` 1068-1103
- `toggleParticleForces` 1104-1110
- `updateParticles` 3673-3714
- `setPointShape` 3715-3720
- `setOpacity` 3721-3728
- `applyParticleColors` 3735-3760

**Delegations to MolecularRenderer (keep as one-line forwarders on
orchestrator):** `updateBondLines`, `toggleBondLines`,
`updateNucleusShells`, `toggleNucleusShells`, `updateBondCylinders`,
`toggleBondCylinders`, `updateOrbitalShells`, `toggleOrbitalShells`,
`updateOrbitalLobes`, `toggleOrbitalLobes`, `updateAEForces`,
`toggleAEForce*`, `updateElementLabels`, `toggleElementLabels`,
`clearElementLabels`, `clearMolecularMeshes`.

**State owned:** `particles`, `velocityVectors`, `trails`, `_particleForces`,
`visualSettings`.

**Constructor:** `new ViewportParticleRenderer({ scene, latticeSize, halfN, insideBoundary, visualSettings })`.

### 3e — DROPPED

The Phase 0 plan called for a `ViewportVizSettings` module, but the
mapping shows only 3 truly-orthogonal cache-only setters
(`setFluxPointScale`, `setFluxThreshold`, `setScenarioScale`). These
make more sense fielded onto FluxRenderer (which actually uses them)
than as their own module. **Decision:** drop 3e, fold these setters
into 3b.

## Extraction order (leaf-first to derisk)

| Order | Sub-renderer | Why first/later |
|---:|---|---|
| 1 | **3b FluxRenderer** | Smallest (200 LOC); narrow concern; cross-cutting issue (`updateFluxSlice` writes `_fieldHeatmap`) is explicit and small |
| 2 | **3d ParticleRenderer** | Clean leaf; touches discrete meshes; many delegations to MolecularRenderer already factored |
| 3 | **3a SceneCore** | Establishes the cascade pattern (`onLatticeSizeChanged()`); orchestrator owns `setLatticeSize` |
| 4 | **3c FieldRenderer** | Biggest (1800 LOC); do last when sub-renderer pattern is settled |

## Verification per sub-phase

After each extraction:

1. **Module imports cleanly** — `await import('/js/viewport/<name>.js')`
2. **Hydrogen scenario renders** — particles visible, Coulomb PE non-zero
3. **Flux-pulse with reflective=OFF** — energy drains 70%+ over 30 ticks
4. **Toggle integrity** — flip every relevant toggle, verify mesh visibility flips
5. **Scenario reload integrity** — switch between 3 scenarios, no console errors
6. **Lattice resize integrity** — change lattice from 32 → 64 → 32, no stale geometry
7. **Console error count** — must remain at the pre-existing baseline (toggle-validation noise from scenario reset is expected)

## Risk register

- **Silent stale geometry** if `onLatticeSizeChanged` callback missed →
  enforce by making the orchestrator's `setLatticeSize` call ALL
  sub-renderers' callbacks unconditionally.
- **`updateFluxSlice` writes `_fieldHeatmap`** → cross-renderer reference;
  resolve via getter callback in FluxRenderer ctor.
- **`_insideBoundary` used by 3b/3c/3d** → keep as method on orchestrator;
  pass as `insideBoundary` callback to each sub-renderer ctor.
- **`zoomToFit` reads `_fluxVolume`** → use a getter or move logic to
  orchestrator (it has access to all sub-renderers).
- **Disposal completeness** → orchestrator's `dispose()` MUST call
  every sub-renderer's `dispose()`, plus `_molRenderer.dispose()`,
  `_topoRenderer.dispose()`, `spinArrowManager.dispose()`.
- **Visual silent breakage** → after each sub-phase, manually verify in
  the browser dashboard with at least 3 scenarios (flux-pulse, hydrogen,
  flux-dual-substrate).

## References

- [META_PROJECT_ATLAS.md](../../../../META_PROJECT_ATLAS.md) §2 (directory tree, viewport/)
- [CONTRACTS.md](../../../../CONTRACTS.md) §1 (live-reference pattern — applies to FluxRenderer's `getFieldHeatmap` callback)
- [docs/adr/0001-viewport-decomposition.md](../../../../docs/adr/0001-viewport-decomposition.md)
- [.claude/plans/i-want-to-try-crispy-charm.md](../../../../.claude/plans/i-want-to-try-crispy-charm.md) Phase 3
