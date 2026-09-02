# Scale 1 Overlay Visual-Language Replacement Plan

**Status:** active implementation plan
**Scope:** Scale 1 effective-particle viewport presentation
**Epistemic boundary:** presentation only. These visuals expose registered
engine state and effective kernels; they do not constitute substrate recovery
or promote any physics claim.

## 1. Design rule

An overlay's geometry must communicate the kind of quantity being shown.
Color alone is not enough, and a raw line segment is not a complete visual
language. Lines remain appropriate as construction primitives, but the user
should see a recognizable glyph, surface, pulse, path history, or event—not a
pile of undifferentiated strokes.

The replacement system must preserve four distinctions:

1. **history** versus instantaneous state;
2. **particle-local vectors** versus spatial fields;
3. **scalar fields** versus vector fields;
4. **measured state** versus presentation-only qualification/provenance.

## 2. Current audit and intended replacements

| Overlay | Data type | Current presentation | Replacement | Encodings |
|---|---|---|---|---|
| Trails | temporal path | line segments | switchable breadcrumbs, particle-colored lines, or kinetic-energy-density heat lines **implemented** | position = history; brightness = age; spacing = tick stride; retained fade = record removal age; heat hue = log-normalized `KE/V_eff` |
| Velocity | particle-local vector | bare segment | instanced arrowhead + short tapered stem | orientation = direction; length/color = `|v|/c`; head shape = instantaneous state |
| Spin | axial vector | bare segment | oriented spin disc/torus with handed tick marks | normal = spin axis; rotation sense = sign; halo intensity = magnitude |
| Coulomb potential | signed scalar field | heat plane plus arrows | signed contour/height sheet | height = potential; diverging hue = sign; contour spacing = gradient |
| Electric field | vector field | continuous streamlines | sparse animated streamlets | motion = direction; density = sampling only; speed/brightness = normalized magnitude |
| Gravity field | vector field derived from potential | arrow grid | potential-well sheet with sparse inward drift particles | depth = potential proxy; drift = force direction; legend exposes visual gain |
| Force components | particle-local vectors | many colored segments | compact force rosette around each particle | petal direction = force direction; petal area = normalized magnitude; channel color = registered component |
| Net force | resultant vector | bare segment | one emphasized solid arrow glyph | orientation = direction; length = normalized magnitude; outline = resultant |
| Center of mass | system point | crossed lines | pulsing reticle/orb | position = energy/mass-weighted center; pulse is presentation only |
| Total momentum | system vector | bare segment | broad translational chevron | direction = total momentum; length = normalized magnitude |
| Angular momentum | axial system vector | bare segment | oriented orbital torus | axis = `L`; radius/brightness = `|L|` |
| Identity/admissibility | qualification state | ring line | translucent shell/aura | green solid shell = qualified record; amber dither = candidate |
| Provenance | source record | billboard text | retained billboard with leader only on hover/selection | text = source record; leader is interaction chrome, not physics |

## 3. Shared visual primitives

Implement reusable, pooled primitives rather than one renderer per toggle:

- `InstancedVectorGlyph`: cone/chevron plus bounded stem, with per-instance
  direction, magnitude, color, and selection emphasis.
- `TemporalBreadcrumbField`: point history with tick stamps, age fade, and
  post-removal retention. This is the completed trail replacement.
- `ScalarSheet`: reusable XZ surface with signed height and contour bands.
- `StreamletField`: short advected dashes or particles sampled from a vector
  field; no long-lived full-screen polylines.
- `AxialGlyph`: oriented torus/disc for spin and angular momentum.
- `QualificationShell`: instanced translucent shell with solid/dither status.

All primitives must use bounded reusable buffers, demand-gated updates, and
the existing selection-focus particle-ID filter.

## 4. Staged implementation

### Stage 1 — trajectory history

Status: **implemented**.

- Support mutually exclusive breadcrumb, particle-colored line, and
  kinetic-energy-density heat-line presentations.
- Sample against particle-engine ticks, independent of display FPS.
- Expose history span, tick stride, despawn fade, opacity, and point size.
- Retain removed-particle history until its configured fade completes.
- Ground heat-line color in native snapshot kinetic energy divided by the
  effective particle volume `(4/3)πr_eff³`; label the result `MeV/lu³` and
  expose the live visible range.
- Keep the overlay setting unchanged during particle/cluster inspection.

### Stage 2 — instantaneous and system glyphs

- Build `InstancedVectorGlyph` and migrate velocity and net force first.
- Build `AxialGlyph` and migrate spin and angular momentum.
- Replace the center-of-mass cross with a reticle/orb.
- Keep all lengths normalized and display the normalization in overlay
  telemetry; never imply raw sim units when a visual gain is applied.

### Stage 3 — force decomposition

- Replace nine simultaneous force-line families with one force-rosette pool.
- Preserve the registered channel colors and per-channel toggles.
- In unselected system view, show compact local rosettes only.
- Under particle/cluster inspection, expand the selected rosette and suppress
  all unrelated instances through the existing focus contract.
- Keep net force visually dominant and component petals subordinate.

### Stage 4 — scalar and vector fields

- Migrate potential to `ScalarSheet` with signed contours.
- Migrate electric field to animated, short-lived `StreamletField` samples.
- Migrate gravity to a potential-well sheet plus sparse direction drift.
- Provide a field-density budget; increasing lattice or particle count must
  reduce sampling density rather than silently reducing simulation tick rate.

### Stage 5 — identity and provenance

- Replace admissibility rings with qualification shells.
- Keep provenance billboards, but render leaders only on hover or inspection.
- Ensure screen-space labels avoid overlap and never hide the selected object.

## 5. Line and clutter budget

- A default scenario may contain at most one full-scene continuous line family.
- Particle-local overlays must use bounded glyphs whose screen footprint is
  capped relative to the particle cloud radius.
- Enabling multiple force components must reuse one rosette at each particle,
  not stack independent scene-spanning strokes.
- Field sampling density is a presentation budget, not a physics toggle.
- Selection mode must render only the selected particle or live cluster's
  local overlays and field sources.

## 6. Verification gates for every replacement

1. **Semantic gate:** geometry, motion, color, and legend encode the documented
   data type and do not make a stronger physics claim.
2. **Source gate:** every rendered value comes from the native bridge,
   registered snapshot, or an explicitly labeled presentation transform.
3. **Lifecycle gate:** toggle, scenario change, scale change, selection clear,
   detach/reattach, and renderer disposal leave no stale geometry.
4. **Selection gate:** particle and cluster focus remove unrelated instances
   without mutating overlay or physics settings.
5. **Layout gate:** controls, legends, and tooltips remain visible and scrollable
   in docked and detached single-column panels.
6. **Performance gate:** measure hardware WebGL across representative particle
   counts and overlay combinations; no software renderer may certify FPS.
7. **Regression gate:** retain native physics/telemetry tests and add geometry
   type, draw-count, normalization, buffer-reuse, and console-error tests.

## 7. Completion definition

The redesign is complete when users can distinguish trajectory, velocity,
spin, force decomposition, scalar potential, vector field, system momentum,
qualification, and provenance by shape and motion before reading the legend;
when selecting a particle or cluster leaves no unrelated overlay geometry; and
when the representative hardware performance matrix remains above the project
frame-rate gate.
