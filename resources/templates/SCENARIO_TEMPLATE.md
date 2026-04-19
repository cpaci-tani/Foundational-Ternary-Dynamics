# SCENARIO — `<scenario-id>`

> Template for a new Scale-0 scenario. Fill in each section, register in `engine/web/js/config/scenarios.js`, delete these callouts, ship.

## Identity

- **ID:** `<scenario-id>` — kebab-case, matches the key in `scenarios.js`
- **Display name:** `<Human-Friendly Name>` — shown in the scenario dropdown
- **Group:** `<e.g. Dynamics / Collisions / Fields / Substrate / ...>` — used to cluster in the dropdown
- **Primary demonstrates:** `<one-sentence what the scenario shows>`
- **Author / date:** `<name — YYYY-MM-DD>`

## Physics spec

> What initial condition does this scenario seed, and what do you expect to happen?

**Initial flux field:**
```
<e.g. Gaussian pulse at center, amplitude 2, width 3 voxels>
<e.g. Two dipoles at (8, 16, 16) and (24, 16, 16), opposite signs>
```

**Initial state field:** `<default: all zeros unless specified>`

**Expected dynamics:**
- **Tick 0–N:** `<what should happen first>`
- **Tick N+1–M:** `<next phase>`
- **Long-term:** `<equilibrium or runaway>`

## Parameters the scenario overrides

> Which sliders, toggles, and constants deviate from engine defaults?

| Parameter | Default | Scenario value | Why |
|---|---|---|---|
| `K_B` | 0.511 | `<value>` | `<rationale>` |
| `G_N` | 0.01 | `<value>` | `<rationale>` |
| `gravity` toggle | off | on | `<rationale>` |
| … | | | |

## Recommended overlays

> Which toggles should be on for the scenario to read well?

- [ ] `Φ potential`
- [ ] `E field`
- [ ] `Dual J`
- [ ] …

## Implementation

### 1. Scenario-loader entry

Add to `engine/web/js/config/scenarios.js` under the appropriate group:

```js
'<scenario-id>': {
    id: '<scenario-id>',
    name: '<Human-Friendly Name>',
    group: '<group>',
    description: `<1-2 sentence user-facing description>`,
    epistemic: '<[DEMO] | [VALIDATED] | [EXPLORATORY]>',
    load: ({ bridge, capabilities }, params) => {
        // Pin the boundary, seed the flux field, place particles, tune knobs.
        bridge.reset(<N>);
        capabilities.setBoundaryShape('<cube|sphere|torus>');
        // ... seed the initial condition
        <SEED_CODE>
    },
},
```

### 2. Default toggles (if any)

If your scenario needs specific toggles on/off by default, add to the scenario-override map in `engine/web/js/config/toggles.js`:

```js
'<scenario-id>': {
    'gravity':      true,
    'confinement': false,
    // ...
}
```

### 3. Seed helper (if complex)

If the seed logic is more than ~15 lines, extract it to a helper in `engine/web/js/scales/scale0/seeds/` and call it from `load()`.

## Validation / sanity

Describe what the user (or a test) can check to confirm the scenario behaves as intended:

- **Visual:** `<e.g. "two blobs merge at tick 180">`
- **Diagnostic:** `<e.g. "Total E stays within 0.1% — energy conservation check">`
- **Benchmark:** `<e.g. "matches analytical 1/r² to B+ in engine-theory bridge">`

## Known limitations

- `<scenario breaks at lattice size > 64 because…>`
- `<requires a specific boundary setting>`

## Screenshots (optional)

Drop PNG(s) of the scenario at characteristic tick counts into `engine/web/assets/scenarios/<id>/`. Reference them here:

![Tick 0](../../engine/web/assets/scenarios/<id>/t0.png)
![Tick 200](../../engine/web/assets/scenarios/<id>/t200.png)

## Cross-references

- Registration: `engine/web/js/config/scenarios.js`
- Toggle overrides: `engine/web/js/config/toggles.js`
- Related theorem: `docs/theory/.../DERIV_<NAME>.md` (if the scenario is a visualization of a proven result)
- Benchmark: `engine/tests/benchmark_<NAME>.cpp` (if the scenario is validated numerically)
- Other scenarios in this group: `<list>`

## Checklist before committing

- [ ] ID and name added to `scenarios.js`
- [ ] `load()` runs clean (no console errors) on fresh page load
- [ ] All referenced overlays actually exist
- [ ] Default parameters documented in the description
- [ ] Dropdown grouping matches convention
- [ ] Epistemic tag on the description matches scenario status
- [ ] If the scenario demonstrates a theorem, the theorem is linked
- [ ] Added to `resources/scenarios/RECIPES.md` if it's worth a walkthrough
