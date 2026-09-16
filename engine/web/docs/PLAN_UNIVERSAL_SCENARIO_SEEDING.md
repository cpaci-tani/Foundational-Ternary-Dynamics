# Universal Scenario Seeding Panel

**Status:** proposed implementation plan; no panel or runtime changes made by this document.

**Inventory date:** 2026-09-15.

**Primary target:** the existing main Scale-0 web-engine lattice, its scenario menu, active runtime, viewport, and docked sidepanels.

## 1. Product contract

Add one **Seeding** panel to the existing dashboard. Every scenario is an editable, reproducible preparation in this panel. A named scenario supplies initial values; its name must not limit which compatible seed components can be configured.

Each category gets a first entry, **Base / Custom — [category]**. Its property list is the union of all seeding properties used by that category, plus universal spatial, composition, randomness, and validation controls. All these sections and fields are available on first opening, with no Advanced switch, hidden preset-only parameters, or requirement to load another scenario first. Components may start inactive so an empty base remains empty; their editors remain visible and configurable. Enable the component when ready to include it in the seed.

Named scenarios use the same editors, prefilled with their exact recipe. Add, remove, duplicate, reposition, and combine compatible components. An **All compatible properties** view exposes the rest of the active runtime's component library from any category. Category organization is navigation, not an artificial seeding restriction.

Full coverage means every registered scenario and every meaningful preparation input has an explicit schema, default, implementation binding, validation rule, and verification case. A generic amplitude/position form is insufficient. Backend work necessary to make a property functional is part of this plan.

There is one active simulation owner. Finite-record preparations run in the same lattice and panel as other preparations. Backend identity is shown as context; this feature introduces no separate lattice application or law-selection workflow. Incompatible state models cannot be combined in one seed without an explicit implemented conversion and recovery contract.

The main lattice is the mandatory complete delivery. Section 11 defines how the same panel extends to the other dashboard scales if that broader scope is selected. Cross-scale UI reuse does not establish microscopic physical recovery.

## 2. Verified starting point and work required

The current checkout has **143 effective-engine scenarios in 18 categories**, plus **629 finite-record preparations in four categories**: **772 scenarios across 22 categories**. The finite catalog has **656 registered preparation/size combinations**. Counts are a snapshot; implementation must enumerate registries and fail coverage checks when new entries lack seeding definitions.

| Existing path | Current behavior | Required change |
|---|---|---|
| `js/scales/scale0/scenario-registry.js` | Most descriptors have empty `defaultParams` and dispatch by scenario ID | Add typed recipe/schema references and exact preset defaults |
| `engine/src/scenarios/{flux,light,quantum,vacuum,s0_seed,s0_field,cell}.cpp` | Canonical native seed bodies; many meaningful values hardcoded | Extract typed native specifications and reusable constructors, preserving legacy defaults |
| `js/scales/scale0/ui/controls/substrate-controls.js` and `wire.js` | Small set of immediate particle/wave/flux/pair actions | Route these shortcuts into the same seeding commands; eliminate competing parameter state |
| `scripts/phi_v2_lattice/web_scenarios.py` | Frozen registered recipes and size allowlists | Keep those immutable; add separately identified validated custom recipe compilation |
| `js/bridge/finite-record-bridge.js` | Loads a verified prepared checkpoint through the local API | Accept verified custom preparation results through the existing compiled worker lifecycle |
| `js/scales/scale0/runtime/{scenario-loader,record-scenario-loader}.js` | Own reset, bridge selection, visuals, and initial observations | Support transactional prepared-state installation and restoration of the applied recipe |
| `js/ui/scale-registry/panel-registry.js` | Shared dock, panel visibility, mobile selection | Register Seeding and integrate its lifecycle, layout, and active-owner applicability |

Paths beginning `js/` are relative to `engine/web/`; other paths are repository-relative. The older Scale-0 scenario architecture audit is historical; live registries and current builders govern this inventory.

## 3. Panel layout and interaction

Use a long, categorized property sheet in the existing dock, with a sticky header and action footer:

```text
SEEDING                        [Scenario / category]
Preset name · Modified/Original · Draft/Applied
Search properties…             [Expand all] [Collapse all]

Domain and coordinates
Randomness and distributions
Seed components                [+ Add component]
  Component 1: background      [enabled] [duplicate] [remove]
    Geometry / placement / field or record properties / composition
  Component 2: probe …
  [remaining applicable category sections, expanded initially]
Initial-state constraints
Preparation protocol           (drives and run settings clearly labeled)
Initial display                (view-only)
Validation and seed summary

[Preview] [Seed lattice] [Seed & run]
[Restore preset] [Save recipe] [Import recipe]
```

- Search labels, aliases, units, component types, and descriptions. Matching fields open their section and keep the search result's component/category context visible.
- Base category sections start expanded. User collapse preferences may be remembered subsequently; **Expand all** must reveal the complete schema. Collapse never deletes or resets values.
- Use numeric inputs with units and optional sliders, coordinate/vector rows, enum selectors, distribution editors, and repeatable tables. Avoid forcing exact values through low-resolution sliders.
- Every field shows its current value and reset-to-preset action; changed fields get a compact marker. Dependent values distinguish editable inputs from calculated outputs.
- Editing changes a draft only. Preview compiles initial data without advancing the active run. Seed lattice installs the prepared state paused. Seed & run installs the same prepared state and then starts normal ticking.
- Restore preset restores the draft. Reset simulation reseeds the **last applied recipe**, including its random seed; it must not silently revert a custom run to the original scenario. Provide an explicit separate action for the original preset.
- Preserve drafts per scenario/runtime/schema version. Switching scenarios retains the prior draft; stale asynchronous previews cannot replace the newly selected scenario.
- Duplicate components carry new stable component IDs. Reordering has declared composition effects. Disabled components stay in the saved draft but contribute nothing.
- Expose full editors in narrow/mobile and floating panels; retain theme colors, keyboard traversal, accessible labels, and visible validation errors. Keep actions usable while scrolling a long property list.

## 4. Universal property families

These are required schema groups, not a claim that every backend already implements every field. The coverage manifest must distinguish existing native support, a required binding, a required constructor, and a property that does not exist under the active law. Missing implementation is a delivery task, not a permanent inert control.

| Group | Complete control surface to account for |
|---|---|
| Domain | Supported lattice dimensions/side length; computational boundary mode and supported per-axis choices; coordinate origin; absolute-cell versus normalized placement; scale-on-resize policy; domain support limits. Do not invent rectangular domains or nonperiodic finite-record laws that are not implemented. |
| Placement | Center or anchor `(x,y,z)`; relative-to-component placement; site/edge/face anchoring; translation; orientation; permitted lattice rotations/reflections; separation; impact offset; subcell remainder where supported; explicit rounding/sampling policy. |
| Geometry and masks | Point; explicit site list; line/segment; plane/sheet; slab; box; sphere/ellipsoid; shell; cylinder/tube; ring/torus; periodic array; polyhedral support; uploaded validated mask; union/intersection/subtraction; inner/outer radii; thickness; lengths; aperture; edge taper; support cutoff. |
| Multiplicity and layout | Count; spacing; repetitions on each axis; lattice/paired/radial/random layouts; population ratios; minimum separation; excluded volume; deterministic placement order; seam/edge placement; rejection-sampling limit and failure policy. |
| Continuous initial field | Full `J=(Jx,Jy,Jz)` and initial wave velocity `W`; additive background; localized envelope; amplitude; component ratios; signed direction; field support; dual-substrate `J_L,J_R,W_L,W_R` and their declared composition. These are effective-engine state fields, not finite-record primitives. |
| Waves and modes | Integer mode vector or admissible `k`; wavelength as a linked representation; initial phase; propagation direction; polarization basis, ellipticity and handedness where supported; longitudinal/transverse content; Gaussian widths along/transverse to propagation; profile/envelope; carrier; chirp/bandwidth; harmonic components with individual amplitudes/phases; traveling/standing construction; initial derivative convention. Generalized constructors are needed where existing helpers are axis-specific. |
| Sources and manifested sites | Ternary state; count; position; velocity; lock/free state; spin/color/flavor labels actually used by the runtime; source-envelope amplitude/width; constituent arrangement and polarity; paired provenance where supported. IDs are allocated by the engine, not editable scientific inputs. |
| Composite preparations | Repeatable constituent table; species/model recipe; relative coordinates and velocities; orientations; source strengths; separation; dressing field; bound/locked initialization; field support overlap; prepared versus dynamically forming configurations. A model name retains its existing physical-identification qualification. |
| Random preparation | Explicit seed; documented generator/version; stable per-component substreams; population sampling mode; exact count versus occupation probability; uniform/Gaussian/bounded distributions where meaningful; mean/variance; polarity ratio; phase/orientation mixture; correlation length or structured correlation generator; realizations selected explicitly. No implicit reroll on render, resize, or panel opening. |
| Flow and disordered fields | Mean drift; shear axis; layer center and width; opposing stream speeds; perturbation amplitude/modes; velocity or field variance; initial spectrum and cutoff; net-momentum/mean-field constraints. Temperature is only an input where a declared backend calibration defines it; otherwise expose variance/occupation. |
| Finite field bank | All 384 Boolean channels, selected by full flag/internal-state identity, phase and polarity; per-site or masked occupancy; exact channel lists; allowed direction/orientation; mixtures and correlations; explicit occupancy conflicts. Direction plus phase alone must not silently select one flag representative. |
| Finite relations | SC axis or FCC plane/diagonal; owning site; primary/reserve slot; blank versus occupied; phase `0..3`; polarity; per-slot background; spatial defects; density; exact count; correlated or independent slot distributions. Render the semantic token and its encoded value. |
| Finite local state | Collision layer `ell` in `0..2`, uniform or masked; stored manifestation `s` in `{-1,0,+1}`; explicit zero/current-readout/lagged-readout preparation convention. Preserve allowed lagged manifestation rather than automatically overwriting it. |
| Barriers and reservoirs | Walls/plates/shells; normals; separations; thickness; opening/slit count and width; torus major radius and tube width; circulation sign; alternating sign sectors; membrane material recipe; source/receiver geometry; region of interest. A locked marker wall is not automatically a reflective field boundary. |
| Initial clocks and gravity context | Mass/source distribution represented by the active model; probe positions/velocities; clock-enabled site selection; initial clock values only where part of supported initial data; initial latency preparation versus solver-derived latency; background and well geometry. Derived accumulators and diagnostics remain read-only. |
| Composition | Ordered components; per-field add/replace/clear operations; masks; explicit conflict rejection; Boolean set/clear/union semantics for finite banks; relation-slot replacement rules; duplicate-object rules; normalization order; deterministic summation order. Never sum Boolean occupation or add encoded relation tokens numerically. |
| Constraints and normalization | Total population; charge/polarity balance; mean removal; momentum balance; transverse projection or matched-potential construction; peak/RMS/integral normalization with one selected target; periodic compatibility; domain containment/wrapping; realized discrete counts. Any repair/projection must be requested, recorded, and included in the resulting recipe/state receipt. |
| Protocol, separate from initial state | Native toggle profile; applicable thermostat/bath settings; scheduled pumps, pulse intervals, durations, gates and port opening; preparation warmup if explicitly requested; run horizon. Timed actions execute in the authoritative runtime, not browser animation callbacks. Law constants, collision tables, expiry and transaction order are not seed sliders. |
| Initial display, separate from physics | Appropriate scalar/vector quantity; field/record filters; component outlines; camera fit; optional baseline/defect difference view. Preserve the universal first-positive flux threshold policy and existing visual controls; changing them must not change initial state. |

For periodic traveling modes, validate lattice-compatible `k` and compute the matched `W` from the actual discrete operator and time-step convention. Frequency, wavelength, and mode number cannot all be independent unconstrained inputs. For electric/magnetic preparation requests, use the engine's actual `J/W` representation and spatial operators; do not silently write arbitrary independent `E` and `B` arrays.

Provide an exact-data component as the completeness escape hatch: validated sparse coordinate/value tables and dimensioned dense arrays for every independently seedable state field. Users must be able to prepare irregular states that no geometric preset describes. Import includes channel/slot ordering, coordinate convention, dtype, units, dimensions and a content digest; reject mismatches and out-of-alphabet values. Offer a table editor and file import, not executable expressions. Derived caches and midcycle transaction state still use their explicit lifecycle contracts.

## 5. Required category bases and specialized properties

Every row gets a category base. All include domain, placement, geometry, repetition, composition, randomness, validation, and compatible background/components. The properties below are additional minimums, not an exhaustive exemption list: the constructor audit in section 9 must extract every further meaningful input.

| Existing category | Scenarios | Additional base properties and preset dimensions |
|---|---:|---|
| Baselines & Controls | 2 | Empty/constant/subthreshold support; amplitude relative to the fixed genesis threshold; null preparation; masks; zero-mean and paired controls. The universal empty base exposes every compatible component family. |
| Waves · Propagation | 11 | Full packet and mode editor; longitudinal/transverse widths; direction; polarization; amplitude; carrier phase; rainbow/multicomponent wavelength list; initial derivative; source/probe placement; boundary-forcing configuration. |
| Waves · Interference & Standing Modes | 9 | Arbitrary wave-source list; pair separation; relative amplitude/phase; reflection parity; orthogonal mode sets; node placement; slit geometry; dual-substrate partition; collision alignment. |
| Waves · Boundaries & Barriers | 4 | Barrier/plate geometry and actual mechanism; slab width; well extent; aperture; incident packet; mode mixture; loop/solenoid support and phase configuration. Preserve current limits on tunneling/Casimir/AB interpretations. |
| Fields · Sources & Electric Profiles | 5 | Source positions/polarities; source lock and velocity; dressing profile; uniform field vector; dipole separation/axis; screening/background distribution; prepared-field versus source-generated initialization. |
| Fields · Magnetic Profiles | 3 | Uniform-background construction; dipole/radial/azimuthal source geometry; orientation; strength; core regularization and cutoff; initial wave velocity; consistent boundary treatment. |
| Energy · Storage & Boundaries | 8 | Capacitor dimensions/gap; torus center, major radius, tube sigma, amplitude, circulation, sign sectors, cutoff; triad axes and per-arm fields; open/walled/membrane variants; inner radius and thickness. |
| Energy · Driving & Transfer | 4 | All reservoir controls; multiple source/receiver cells; pump spatial profile, amplitude, count, interval and phase; start/stop ticks; port center, normal, radius, surface offset and opening tick; observation region. |
| Collective Fields · Noise & Shear | 4 | Occupancy/distribution and RNG; field variance; spectrum/modes; correlation structure; shear axes/width/speed; perturbations; mean removal; bath configuration separately labeled. |
| State Dynamics · Genesis & Decay | 22 | Packet/cluster count and arrangement; amplitude relative to fixed thresholds; envelope; background; source polarity; directional/isotropic arrangements; collision geometry; drive; thermal-runaway preparation; cluster extent; field/velocity balance. Genesis and decay rules remain runtime settings/law, not desired outcome controls. |
| Particle Motion & Collisions | 11 | Per-body/packet positions, velocities, polarities and labels; separation; impact parameter; source lock/recoil choice; background field; pair/triad geometry; flux tube dressing; probe amplitude/direction; initial field versus matter content. |
| Particle Models · Leptons | 12 | Constituent marker recipe; polarity/conjugation; envelope radius/sigma/boost; source state; motion; spin/flavor where implemented; support cutoff; preparation multiplicity. |
| Particle Models · Quarks | 12 | Color/polarity/flavor labels; constituent placement; envelope/boost; motion; conjugation; source profile; multiplicity and color composition. |
| Particle Models · Bosons | 7 | Field-only versus marker/composite recipe; packet orientation/polarization; envelope/boost; constituent separation and polarity; model labels; background. |
| Particle Models · Hadrons | 5 | Constituent table including colors/polarities; two/three-body layout; spacing; dressing; velocities; locks; relative phases where represented. |
| Atomic & Molecular Models | 3 | Nucleus/source and electron-marker arrangements; per-constituent seeds; separation; orientation; initial drift; dressing; prepared composite versus formation initial conditions. Do not import an AtomEngine state as if it were the lattice state. |
| Gravity & Clocks | 8 | Source density/geometry; prepared flux; probe packet; source/probe separation; trajectory velocity; clock placement and supported initial phase; latency preparation policy; well and horizon-model parameters that actually exist. |
| Geometry & Topology | 13 | Ring/tube/loop dimensions; winding/circulation and field texture; polyhedron radius/orientation; selected Moore shells and parity classes; constituent labels; node amplitudes; boundaries and cutoff. |
| Finite records · Core and regression controls | 9 | Blank state; isolated relation owner/axis/phase/polarity/slot; sparse exact relation count; field occupation and RNG; R5 background in both slots; seam placement; lagged `s`; witness intervention; expiry channel presentation; per-site `ell`. |
| Finite records · Registered carrier controls | 600 | Relation/field/encounter/separated component family; all relation orientations or full field channels; phase, polarity, slot, placement; collision layer; second polarity/channel; separation; masks and population. |
| Finite records · Registered mixed response | 10 | All nine background relation orientations and both slots; reference/SC-phase/SC-slot/FCC-phase/FCC-slot defect; defect owner/orientation; probe enabled; exact channel; probe region/shape/size and occupancy; background/defect/probe as separately editable components. |
| Finite records · Registered mixed scattering | 10 | All mixed-response controls; independently specified multiple probe channels and their correlations/placement; default channel pair `[0,32]`; phase/flag mixture; background and defect controls. Preserve distinct flags even when counts agree. |

### Frozen finite preparation coverage

- Core IDs: `empty`, `relation`, `sparse`, `r5`, `seam`, `witness-control`, `witness`, `expiry-a`, `expiry-b`, each at sizes `3,4,7,9`.
- Carrier IDs: `carrier_0000` through `carrier_0599`, at size `9`. The registry contains 288 relation cases, 96 field cases, 108 encounter cases and 108 separated cases. Preserve each exact case, including its chosen full channel identity.
- Mixed response IDs: `mixed_00` through `mixed_09`, at size `17`; five defect choices times probe/control.
- Mixed scattering IDs: `scatter_00` through `scatter_09`, at size `17`; the corresponding heterogeneous-probe family.
- Dashboard IDs prepend `record-`. Do not replace these IDs or their checkpoint receipts with generalized custom IDs.

Editing a frozen recipe produces a custom preparation referencing its parent. Changing a size outside the registered set likewise becomes custom, and is allowed only after validation/resource checks for that size. It does not expand the original experiment's validated domain.

## 6. Typed recipe and coverage contracts

Create one versioned declarative recipe envelope, with native typed component payloads:

```text
schemaVersion, recipeId, revision, title
parentScenarioId, categoryId, runtimeFamily, lawIdentity
domain { dimensions, boundary, coordinateConvention }
randomness { algorithmVersion, seed, componentStreams }
components[] { id, type, enabled, transform, mask, parameters, combine }
constraints[]
protocol { nativeProfile, scheduledActions, preparationSteps }
viewDefaults                         // outside the physical recipe digest
```

Each property definition requires: stable key; label/description; group; value type; units/convention; legal finite alphabet or numeric range; default/default expression; dependency rules; native destination or constructor input; availability; validation; deterministic serialization; preset overrides; provenance; and test references. Distinguish chosen, derived, fixed-law and view-only values.

Maintain a generated **scenario-to-property-to-backend coverage manifest**. Each scenario row includes its ID, category, constructor, schema, exact defaults, supported sizes/runtime, all meaningful inputs, corresponding controls, and verification references. Each omitted native input needs an explicit reason such as derived, fixed law, or internal allocation. Auditing only already-exposed JavaScript parameters misses the current hardcoded native inputs.

Required invariants:

1. Every catalog scenario has a recipe and every category has a base schema.
2. The base schema includes the union of its member schemas, with shared property identity preserved.
3. Every editable field reaches a native preparation input; no accepted-but-ignored parameter.
4. Every semantically meaningful seed input is exposed or has a reviewed noneditable reason.
5. All initial defaults resolve in backend units. Resize-dependent defaults are explicit expressions, with rounding and override semantics.
6. Unknown recipe fields, unknown component types, NaN/infinity, invalid finite tokens and unsupported combinations are rejected; no fallback to an unrelated scenario.
7. Effective and finite-record digests describe their own complete initial state; display preferences and evidence tags cannot alter them.

## 7. Compilation, application, and ownership

Pipeline: **edit draft → validate → compile in a detached preparation context → inspect preview/receipt → install once through the active-owner lifecycle → normal engine ticks**.

- Effective-engine recipes compile through canonical C++ constructors. Add structured parameter dispatch while retaining the old ID-only dispatch as an exact-default wrapper. Bind the same typed request through in-thread WASM, worker WASM and applicable native bridges; do not recreate the physics constructors in browser JavaScript.
- Finite recipes compile through law-compatible preparation/codec validation into complete staged records. Retain the current immutable registered checkpoint endpoint. Add a bounded custom recipe endpoint initially, then a compiled browser preparation path if static-host feature parity is required. Neither may accept executable code or arbitrary server file paths.
- Finite initial state includes `s`, `ell`, `bank`, `sc`, `fcc` and the staged control context. Ordinary seeds begin at microtick zero with cleared pending controls. Midcycle state and pending bits belong to validated full checkpoint import; never expose independent controls that can create an invalid phase/control combination.
- Collision tables, encoding identity, transaction schedule and expiry mechanism remain fixed by the selected runtime. Hydro or thermal successor-law data must not be passed to the current candidate worker. Analytical thermal gates are not seedable simulation scenarios.
- Compile without mutating the live owner. New native staging/installation support is required where absent; do not emulate atomicity with a series of live injections and a hope of rollback.
- Installation checks recipe revision, preview generation and active-owner generation. Only the latest matching request can install. On failure retain the old state and applied recipe; do not show the failed draft as applied.
- Allocate/reset RNG state deliberately. Scenario RNG and runtime/thermostat RNG are separate recorded controls. Preserve the old default generator sequence during migration; custom random generators must be versioned and portable or serialize the resolved draws with provenance.
- Start a fresh diagnostic segment on install, update inspector/volume/slices/scene and all applicable sidepanels from the new owner, and clear stale asynchronous frames.
- Live injection can be a secondary **Apply to current state** action only for adapters with a complete atomic mutation contract. Capture the current revision and label it as an intervention. This is distinct from creating a fresh scenario and must not be silently emulated for unsupported owners.
- Draft undo/redo is local and inexpensive. Do not advertise undo of an evolved lattice; full state restoration requires an explicit compatible checkpoint.

## 8. Preview, display, reproducibility, and evidence

Preview shows component outlines and the compiled initial field/records in the existing viewport, with a visible Draft indicator. It must not tick the active run. Report actual discretized support, bounds, occupied sites/channels/slots, overlaps, realized counts, selected runtime, and validation errors before application.

Mixed preparations need bank/channel/phase/slot inspection and a background-versus-defect/probe view. A phase-only defect can have identical scalar counts; count visibility must not be presented as a failed seed. Registered mixed probes retain the field-token initial view; their controls retain the relation-token initial view. Difference overlays use compatible same-size states and exact labeled quantities.

Keep flux volume shape, opacity, spacing and other existing controls wired to the resulting data. Each seed/reset starts with the existing first-positive flux threshold, currently `2e-8`, obtained from the shared threshold policy rather than a new literal. An all-occupied background may correctly fill the lattice even at a positive threshold; a threshold cannot manufacture contrast that the selected observable lacks.

Export/import a versioned recipe separately from a complete resumable checkpoint. Save the parent scenario, defaults version, explicit overrides, generator/seed, ordered components, native artifact/law/encoding identities, compiled initial-state digest and protocol digest. A state-affecting recipe change marks the result **Custom preparation**. Preserve the parent reference and evidence links, but do not reuse its behavioral qualification, registered-case receipt, or transport claims for the changed run.

Checkpoint compatibility includes all state required for faithful continuation, not just positions or a rendered frame. Where a backend lacks a complete checkpoint contract, label recipe export as reproducible reseeding only. Immutable prior evidence stays unchanged.

## 9. Implementation sequence and deliverables

| Step | Deliverable and likely locations | Exit gate |
|---|---|---|
| 1. Full constructor inventory | Generated manifest under `engine/config/`; inspect all seven C++ groups, shared helpers, registry custom loaders, toggle/drive setup, finite preparation builders and actual state validators; capture baseline initial-state receipts before refactoring constructors | Every live scenario accounted for; every seed input classified; category unions mechanically checked |
| 2. Schema and defaults | Shared recipe/property/component contracts; proposed `engine/web/js/seeding/{schema,registry,recipe,validation}.js`; native schema/spec definitions near `engine/include/ftd/scenarios.h` | Defaults reproduce existing inputs; unknown and incompatible inputs fail explicitly |
| 3. Native preparation API | Parameterize constructors in `engine/src/scenarios/`; add structured bridge/binding methods in `engine/wasm/ftd_wasm.cpp`, capabilities, harness and worker/native transport | Same recipe reaches the actual owner on every advertised backend; detached compile and atomic install work |
| 4. Finite custom preparation API | New bounded recipe builder beside `scripts/phi_v2_lattice/web_scenarios.py`; codec/staged validation; `engine/web/serve.py`; finite bridge/compiled preparation support as needed | All 656 frozen variants unchanged; legal custom records install; illegal states fail before owner mutation |
| 5. Panel and category bases | `js/ui/scale-registry/panel-registry.js`; proposed `js/seeding/ui/`; panel host/theme/dock lifecycle; existing scenario picker and loaders | All 22 bases expose full category properties; every named preset populates the same form |
| 6. Complete component library | Fill every missing constructor and binding from the inventory, including generalized wave orientations, field derivative controls, masks, composite sources, reservoirs and exact finite channels | No placeholder controls; property-to-native coverage complete for all in-scope scenarios |
| 7. Integrated application | Scenario store/loaders, mutation qualification, active-owner helpers, record panel model, viewport and existing injection shortcuts | Reset/custom recipe behavior, stale-request handling, view quantities, and all applicable sidepanels agree |
| 8. Persistence and portability | Recipe import/export/version migration, native receipt, preset restoration; static/local capability messaging | Identical supported recipe round trips; unavailable preparation capability fails visibly |
| 9. Coverage release gate | Native and finite preparation tests, Node schema tests, actual-browser UI/application tests; update scenario architecture docs | Section 10 passes across the complete catalog; publish exact supported backend/property matrix |

Implement representative vertical slices first—one wave, one mixed preparation, one reservoir—but these are integration milestones, not completion of universal coverage. The final gate includes all scenarios. The implementation inventory must expose remaining work explicitly instead of treating an early slice as a universal release.

## 10. Verification and acceptance

1. **Catalog completeness:** enumerate all 772 current IDs, regenerate on change, and check each recipe/schema/default/constructor binding. Check all 22 category unions and defaults, not a handpicked subset. Add explicit support disposition for eligible test fixtures; invalid-state rejection tests and analytical calculations are not runtime seeds.
2. **Exact legacy defaults:** compare complete finite checkpoints for all 656 registered variants. For all 143 effective recipes, compare the complete supported initial-state representation, toggle/profile state, RNG and scheduled actions against the old default path at supported test sizes. Use exact comparisons on the same implementation; cross-platform float tolerance must be explicit and justified, never silently called byte parity.
3. **Actual property effects:** for every editable property use an independent expected state feature or constructor result to verify its wiring. Check representative combinations, conditional branches and boundaries. For phase/slot/flag edits inspect records, not only counts or a whole-state digest. Check derivation dependencies and realized rounding.
4. **Geometry/numerics:** zero population, maximum supported population, seams, edges, masks, duplicate slots, smallest/largest supported sizes, odd/even dimensions, cutoff boundaries, allowed rotations, periodic mode compatibility, invalid distributions, overlaps and contradictory constraints. No silent clamping; any requested projection is reported.
5. **Randomness:** identical recipe/seed gives identical prepared state; changing the seed changes stochastic components; deterministic components remain unchanged; component substreams survive unrelated component edits as specified. Restoring old presets preserves their old sequence.
6. **Transactional lifecycle:** dirty drafts, preview without mutation/ticking, seed/reset, seed & run, cancel, rapid scenario switches, resize, worker failure, late responses, import failure, disposal and active-owner switching. Check owner identity and state digest as well as UI text.
7. **Browser coverage:** automatically load every scenario form and every base; verify expected controls, defaults, units, errors and application receipts. Exercise every component editor in the real lattice with both effective and finite owners. Include mixed response/scattering and scheduled reservoirs as dedicated regressions.
8. **Sidepanels/visuals:** check all applicable existing panels after seed/apply/reset, quantity selection, exact finite distinctions, threshold policy, and absence of stale frames. View-only changes preserve state digest. An attractive screenshot alone is insufficient.
9. **UI usability:** long-list search, expand/collapse, keyboard-only entry, visible focus/errors, number precision, dark/light themes, narrow/floating panels, long scenario names, footer access, draft persistence and repeatable component ordering.
10. **Evidence/claims:** exact registered presets retain references; edited seeds visibly become custom; no new physical identity, recovery or transport qualification from UI coverage. Frozen evidence and preregistration files remain immutable.
11. **Resource limits and deployment:** preflight memory/site/component/import-size budgets; bounded compilation; cancellation; local API and static-host capability paths. Do not accept input then show an empty lattice because a required capability is unavailable.

Run focused native preparation tests with the repository's parallel build/CTest conventions, relevant Python finite tests, Node schema/coverage tests and Playwright against the actual WASM/native paths. Run shared-port browser suites sequentially. A planning document itself does not require rerunning the physics campaigns.

## 11. Other dashboard scales: extension contract

If scope includes every dashboard scale, reuse this panel, recipe envelope, component model and coverage gates. Add typed adapters to the existing owners rather than new independent simulators. Inventory the actual live registries; do not rely on legacy architecture counts or obsolete MockBridge descriptions.

| Scale | Additional preparation families to inventory and expose |
|---|---|
| 1 — Particles | Full supported particle catalog, positions/velocities, counts/species mixtures, charge/spin/other writable labels, beam distributions, pair/collision geometry, boundaries, sources and supported native background fields |
| 2 — Atoms | Element/isotope and charge states actually supported, atom counts/layout, positions/velocities, crystal basis/cell spacing/defects, nuclei/neutrons where represented, prepared bond graph, collision and thermal initial distributions |
| 3 — Molecules | Molecule identity and copies, constituent coordinates, translation/orientation, bond graph/order, conformer and supported internal modes, initial vibration/rotation, intermolecular separation/collision, fluid/crystal arrangements and initial thermal distributions |
| 4 — Planetary | Body mass/radius/type, position/velocity or consistently converted orbital elements, central object, inclinations/phases, system hierarchy, spin where used, units and actual gravity model; source provenance for catalog systems |
| 5 — Cosmic | Actual supported body/population catalog, positions/velocities/mass profiles, gas/dark-matter component distributions, galaxy/cluster structure, stellar populations and cosmological initial context under the selected effective model |
| Geometry/reference-context views | Display geometry, orientations, shell selections and reference parameters. Views without an evolving state owner get configuration editors, not fabricated physical seeding capabilities. |

These effective-scale recipes must not be described as recovered states of the strict microscopic owner. A future microscopic lifting/restriction implementation needs its own explicit contract and evidence. No lossy handoff or independent effective engine is silently presented as the same causal lattice.

## 12. Definition of done

From any in-scope scenario, the user can inspect and edit every legitimate seeding input, add compatible components, preview the exact discretized preparation, install it into the real active engine, reset it reproducibly and save its recipe. Every category base exposes its complete property union by default. There are no unbound sliders, hidden meaningful constructor knobs, silent substitutions, stale sidepanels, or reused evidence claims for edited preparations.
