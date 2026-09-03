# Audit — Scientific Scenario Inventory (2026-08-29)

**Status:** **[OPEN]** inventory and migration baseline; no scenario is
scientifically qualified by this document.

**Frozen production tree:** `eeea93ebf88ba47f55e2ccfad4aefd60d3dd234a`

**Governing plan:**
[`PLAN_SCIENTIFIC_SCENARIO_QUALIFICATION.md`](../PLAN_SCIENTIFIC_SCENARIO_QUALIFICATION.md)

This audit freezes the presentation surface before scenario-by-scenario
qualification begins. Counts distinguish executable scenarios, hidden
unfinished scenarios, and a structural exhibit. A familiar name or a passing
behavioral test is not evidence of physical identity.

> **Historical baseline note (2026-09-03):** The table below remains the
> frozen `eeea93e` inventory. The current Scale 2 selector has 146 entries
> (29 curated contracts plus 117 generated element references), and the
> current all-scale presentation total is 373. Current assertions live in
> `tests/scientific-scenario-inventory.spec.js`; the frozen counts are retained
> here as provenance rather than silently rewritten.

## 1. Frozen inventory

| Scale | Surface | Count | Runtime/backend | Current contract boundary |
|---|---|---:|---|---|
| Scale 0 — lattice | visible scenarios | 130 | C++ core through CPU/WASM/native paths | finite lattice records, flux, manifestation, and candidate structures |
| Scale 1 — particles | visible scenarios | 6 | native C++/WASM Particle Engine | coarse-grained or imposed particles; catalog injection is parametric |
| Scale 2 — atoms | selectable entries | 139 | JavaScript Atom Engine model | pedagogical/parametric atomic and interaction models |
| Scale 3 — molecules | selectable entries | 27 | shared JavaScript Atom Engine model | hand-authored molecular geometries and heuristic interactions |
| Scale 4 — planetary | visible scenarios | 8 | JavaScript Newtonian N-body | imposed/catalog-seeded classical experiments |
| Scale 5 — cosmic | hidden unfinished scenarios | 13 | JavaScript approximate N-body/subgrid model | not publication-qualified; remains unreachable |
| Scale 6 — meta | structural exhibit | 1 | procedural visualization; no physics tick | Moore-neighborhood geometry and qualified selections |
| Scales 7–11 | registered scenarios | 0 | none | no current product surface |

The frozen tree contains **324 scenario/presentation entries**: 311 reachable
entries plus 13 hidden Scale-5 entries. Scale 6 is counted as an exhibit, not a
simulation. The live Scale-0 catalog, menu, and C++ dispatcher agree on all 130
IDs with no duplicate, registry-only, native-only, or hidden IDs.

Authoritative enumeration points:

- Scale 0 catalog and evidence admission:
  `engine/web/js/scales/scale0/scenario-registry.js:183,1024-1033`.
- Scale 0 native ID mirror and dispatcher:
  `engine/src/scenarios.cpp:65-244`.
- Scale 1 registry:
  `engine/web/js/scales/scale1/scenario-registry.js:90-212`.
- Scale 2 curated and generated registry:
  `engine/web/js/scales/scale2/scenario-registry.js:16-253`.
- Scale 3 molecule library and special entries:
  `engine/web/js/molecules.js:58-560` and
  `engine/web/js/scales/scale3/controller.js:155-180`.
- Scale 4 toolbar registry:
  `engine/web/js/scales/scale4/ui/toolbar/template.js:7-18`.
- Scale 5 hidden selector and dispatcher:
  `engine/web/index.html:223`,
  `engine/web/js/scales/scale5/ui/toolbar/template.js:1-19`, and
  `engine/web/js/bridge/cosmic-scenarios/index.js:38-57`.
- Scale 6 structural controller:
  `engine/web/js/scales/scale6/controller.js:1-20`.

## 2. Scale-0 initialization ownership

| Family | Count | Native initialization owner |
|---|---:|---|
| `empty` | 1 | `engine/src/scenarios.cpp:230` |
| `flux-*` | 22 | `engine/src/scenarios/flux.cpp:27-525` |
| `light-*` | 4 | `engine/src/scenarios/light.cpp:21-116` |
| `quantum-*` | 9 | `engine/src/scenarios/quantum.cpp:23-229` |
| `s0-seed-*` | 57 | `engine/src/scenarios/s0_seed.cpp:29-998` |
| `s0-field-*` | 15 | `engine/src/scenarios/s0_field.cpp:21-380` |
| `s0-vacuum-*` | 22 | `engine/src/scenarios/vacuum.cpp:27-383` |

The existing toggle-matrix test covers all 130 IDs at
`engine/tests/test_toggle_matrix.cpp:175-324`. This is valuable behavioral
wiring evidence; it does not supply the missing mathematical, provenance,
backend, scale, UI-truth, or performance fields.

## 3. Manifest gap

The product has multiple registries but no canonical scientific manifest.
Identity and descriptive prose are relatively complete; scientific contracts
are fragmented across C++, toggle maps, loaders, overlay inference, docs, and
tests.

| Contract field | Scale-0 coverage at freeze | Finding |
|---|---:|---|
| ID and display name | 130/130 | present |
| Model relation and experimental role | 0/130 | menu category is neither axis |
| Mathematical model | 0/130 structured | implicit in executable sources |
| Initial conditions | 0/130 structured | implicit in C++ initializers |
| Enabled terms | 130/130 | external and duplicated |
| Parameter provenance | partial | mostly free-form prose |
| Native units/calibration | 0/130 structured | missing |
| Accepted/prohibited claims | partial | embedded in qualification prose |
| Falsification gates | 0/130 structured | missing |
| Backend support/tolerance | 0/130 per-scenario | global infrastructure only |
| Explicit resolution domain | 39/130 | 91 are not machine-specified |
| Explicit boundary domain | 97/130 | 33 inherit a runtime default |
| Cross-scale inputs | 0/130 structured | missing |
| Explicit visual defaults | 23/130 | remaining entries inherit preferences |
| Performance budget | 0/130 per-scenario | global caps only |

The schema introduced for this migration is
`engine/web/data/scientific-scenario-manifest.schema.json`. It is a contract,
not a completed manifest. Entries must be migrated without upgrading their
epistemic status.

## 4. Stop-ship findings for qualification claims

### P0 — boundary drift on Scale 0

Thirty-three Scale-0 scenarios omit an explicit boundary and receive runtime
dispersal from `scenario-loader.js:445`, while native behavioral tests can begin
from the engine's periodic default. A finite run may not reach the boundary,
but that assumption is not recorded. Dashboard and CTest therefore cannot yet
be called the same qualified experiment for those entries.

### P0 — backend-inconsistent legacy aliases

Thirteen historical aliases exist only in the WASM binding at
`engine/wasm/bindings_render_bridge.cpp:241-276`. The normal web resolver maps
unknown IDs to `flux-pulse` before the bridge sees them, and native WebSocket
dispatch has no alias table. The mappings for `force`, `scattering`, and
`cluster` are scientifically unsafe because historical proximity is not
experimental equivalence. Alias provenance must be explicit and fail closed.

### P0 — Scale-1 first-load race and orphan handoff

The native-WebSocket fallback facade begins WASM initialization asynchronously
but exposes synchronous Scale-1 methods. A first scale switch can seed before
the Particle Engine is ready and silently show an empty active scenario.
Additionally, the two promoted-lattice entries instruct users to invoke a
nonexistent “Scale up” UI action. A prerequisite failure currently does not
prevent the scenario from becoming visibly active.

Evidence: `engine/web/js/bridge/ws-scale-fallback-facade.js:12-32`,
`engine/web/js/scales/scale1/controller.js:540-560`,
`engine/web/js/bridge/native-particle-engine.js:149-176`, and
`engine/web/js/scales/scale1/promotion.js:244-316`.

### P0 — broken Scale-2 Rutherford label

The Rutherford target is created with zero charge while the implemented force
is proportional to the product of the two charges. The selected scenario
therefore has no Rutherford Coulomb interaction and cannot represent the named
experiment. It must remain failed/blocked until the model and wording agree.

Evidence: `engine/web/js/scales/scale2/scenarios.js:171` and
`engine/web/js/bridge/mock-atom-engine.js:225`.

### P0 — Scale-5 names exceed executable mechanisms

Scale 5 correctly remains hidden. Its “gravitational wave” entry inserts an
event record but has no propagating strain field or radiation reaction;
“baryogenesis” has no baryon/antibaryon distinction, CP violation, or asymmetry
observable; “emergent BH” is a hard-coded escape-speed transition detector.
Unseeded `Math.random()` in star formation also prevents replay-grade
reproducibility. These are not eligible for publication under their current
physical names.

Evidence: `engine/web/js/bridge/cosmic-scenarios/exotic.js:159-221` and
`engine/web/js/bridge/cosmic-postupdates.js:131-159`.

## 5. High-priority truth and provenance defects

- Scale-0 menu categories mix native behavior with closed-negative, imposed,
  conjectural, and open physical identifications. Behavioral validation must be
  separate from scientific class.
- Several Scale-0 selector names overstate their passed behavior, including
  soliton, spacetime-bending, photon, neutrino, and particle-transition labels.
- Exact/scaled template families need structured `alias_of`, `derived_from`,
  and parameter-delta provenance.
- The Standard Model overlay is correctly reference-only, but its short status
  chooses the last tag. This can display `EMERGENT` for a neutrino entry whose
  identity remains conjectural.
- Scale 1 documentation exposes retired `pe-*` identifiers instead of the six
  live `s1-*` entries.
- Scale 2 and 3 use a JavaScript model because native unit parity is unavailable;
  a global “WASM Engine” badge can conceal that provenance.
- Unsupported samplers can look like physically zero data. Scientific displays
  need explicit `live`, `derived-live`, `JS model`, `catalog`, `unsupported`,
  and `stale` origin states.
- Normal scale switching replaces controllers and loads an independent
  scenario. It is not a scientific cross-scale transformation unless a
  versioned handoff envelope was actually applied.
- URL/session state does not preserve scale, scenario, boundary, seed, active
  terms, scientific profile, or handoff provenance.

## 6. Architecture decision for the migration

The minimal boundary is:

1. one schema-validated `ScenarioManifest` for identity, claims, model,
   provenance, backends, gates, visual defaults, and performance;
2. an asynchronous `ScaleAdapter` with `ready`, `validate`, abortable `load`,
   `unload`, and atomic `snapshotHandoff`;
3. a versioned `ScenarioSession` with load generation, effective scientific
   profile hash, backend/capabilities, and separate presentation state;
4. a `HandoffEnvelope` carrying source/destination scales, source scenario,
   epistemic status, backend, epoch, boundary, units, terms, transformation,
   losses, and evidence;
5. a strict `idle -> loading -> ready | failed` transition in which the visible
   active scenario changes only after validation and load succeed.

Scientific state and presentation state remain separate. Unsupported data is
never represented as an empty or zero scientific result.

## 7. First scenario

Scenario 1 is `empty`. Its versioned preregistration is
[`scenarios/AUDIT_SCALE0_EMPTY_BASELINE_v1.md`](scenarios/AUDIT_SCALE0_EMPTY_BASELINE_v1.md).
No wave or physically named scenario may begin qualification until `empty` has
a recorded disposition and synchronized manifest, source, tests, UI, export,
and performance evidence.
