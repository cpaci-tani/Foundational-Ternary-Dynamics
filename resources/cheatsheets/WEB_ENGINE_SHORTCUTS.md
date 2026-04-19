# Web Engine Shortcuts

Keyboard, mouse, and scrub-bar interactions for the browser dashboard at `engine/web/`.

Run the dashboard with:
```
python -m http.server 8080 -d engine/web
```
Then open http://localhost:8080.

## Playback (scrub-bar left group)

| Action | Shortcut | Button |
|---|---|---|
| Global play / pause (freeze whole sim) | `Space` | ▶ "global" |
| Local play / pause (freeze scenario, keep viz) | `Shift + Space` | ▷ "local" |
| Single step | `S` | ⏭ |
| Reset scenario | `R` | ↺ |
| Speed (ticks per frame) | — | slider (0–100 → 0×–3×) |

## Scrub timeline (scrub-bar centre)

| Action | How |
|---|---|
| Drag anywhere on strip | **pointer-down + drag** — snap to sim tick, hydrate engine from nearest snapshot |
| Snap back to live | ⟲ button OR **double-click** the strip |
| Memory zones (colour-coded) | LOD 0 (bright) = fresh, LOD 1 = downsampled, LOD 2 = coarse, LOD 3 = telemetry-only |
| Render clip indicator | thin green band = a rendered clip is available to scrub through |

## Render clip (scrub-bar right group)

| Action | Shortcut | Button |
|---|---|---|
| Render scenario forward (N seconds) | — | "Render" circle-dot |
| Choose render duration (10 / 30 / 60 s) | — | ⚙ gear → popover |
| Cancel an in-flight render | — | ✕ on the floating Render chip |

When a render is running:
- The main sim **freezes** (state.rendering = true).
- The canvas visibly **fast-forwards** through the clip as each slice uploads.
- The top-right chip shows `Rendering N s…` with a progress bar.
- When done, the engine snaps back to the pre-render state; the clip stays in the scrub buffer for playback.

## Console diagnostics

```js
// Active context (scale-specific)
window.__ftdCtx

// Force a render programmatically
window.__ftdStartRender(5)      // 5-second clip
window.__ftdCancelRender()       // abort

// Inspect the timeline buffers
const { getScale0MemoryRecorder, getScale0RenderController } = await import('/js/scales/scale0/controller.js');
getScale0MemoryRecorder()?.buffer.size          // snapshot count
getScale0RenderController()?.buffer?.size       // clip frame count
```

## Scale switches

The scale dropdown in the toolbar switches between:

| Option | Scale | What you get |
|---|---|---|
| Scale 0 (Lattice) | substrate | flux field, particles, force overlays, topology sheets |
| Scale 1 (Particles) | particle engine | Coulomb scattering, decay rates, spectroscopy |
| Scale 2 (Atoms) | atom engine | periodic table, orbital clouds, Slater shielding |
| Scale 3 (Molecules) | molecular engine | VSEPR, bonding, molecular library |
| Scale 4 (Planetary) | N-body sandbox | exoplanet systems, Kepler orbits, terrain |
| Scale 5 (Cosmic) | Λ-CDM | Hubble expansion, galaxy formation, black holes |
| Meta | existential unit | 27-site Moore polyhedra, O_h symmetry |
| Scale 11 (Consciousness) | φ-loop | consciousness phase, sLoop, holographic figures |

## Scenario picker (controls panel, Scale 0)

Scenarios live in `engine/web/js/config/scenarios.js`. Dropdown is grouped by phenomenon:
genesis cascades, collisions, dipole fields, photon race, dual substrate, pair production, gravity cluster, etc.

Load a scenario → scenario.load() seeds the flux field → memory recorder starts capturing at tick 1.

## Field overlays (viewport panel, Scale 0)

| Column | Overlays |
|---|---|
| **Volume** | flux volume, flux slice |
| **Fields** | E field, B field, Poynting, ∇·J, flux streamlines |
| **Forces** | EM, Gravity, Strong, Weak — with arrow / heatmap / flow / glyph style selector |
| **Quantum** | \|ψ\|², phase φ, ℒ(x), entropy s |
| **Topology** | Φ potential, EM energy u, charge ρ, vorticity ω — all rubber-sheet surfaces |
| **Phenomena** | Dual J, chirality, dark-matter halo, damping zones, genesis isosurface, confinement, light |

All topology overlays go **flat in stillness** and deform as structure develops.

## Panel dock

The panel dock at the bottom of the viewport houses:

- **Controls** — scenario picker + physics toggles + substrate knobs + flux-volume controls
- **Charts** — chip-picker grid of live time-series charts (uPlot)
- **Diagnostics** — 27-row physics-unit table with inline sparklines
- **Lagrangian** — stacked-area chart of the seven terms + action & constants sidecar

Resize handle at the top edge of `#panel-area`. Collapse/expand via the dock head.

## Quick tour

1. Open http://localhost:8080 → Scale 0 loads with the `flux-pulse` scenario.
2. `Space` to start.
3. Toggle **Φ potential** in the overlay panel → watch the rubber sheet deform under the flux wave.
4. `Shift+Space` to locally pause → the sheet freezes; global visualization keeps polling.
5. Click **Render** in the scrub bar → 30 s fast-forwards → drag the scrub thumb left/right to scrub through the rendered future.
6. `R` to reset. Try a different scenario.
