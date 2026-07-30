# AUDIT_2026-07_scale1-particle-engine

**Sweep type:** 5-dimension parallel multi-agent audit (Workflow tool, 99 agents total: 5 dimension
reviewers + 94 adversarial verifiers, one skeptic per raised finding) of `engine/web/js` Scale 1
(the Particle Engine / "PE"). Dimensions: architecture & code quality, physics fidelity & epistemic
honesty, UI/telemetry wiring correctness, rendering & catalog data integrity, scenarios & UX flow.

**Date:** 2026-07-29. **Trigger:** user request to audit Scale 1 ahead of a full redesign.

**Disposition:** this sweep does not drive a fix-in-place pass. Findings below fed directly into the
decision (made the same day, in the redesign brainstorm this audit supports) to retire the entire
general SM-particle sandbox — catalog browsing, the scenario library, cross-sections/decay-rates/
spectroscopy panels, the Hawking toy, the whole `mock-particle-engine.js` classical N-body path — and
replace Scale 1 with a native-EFT matter-dynamics frontier viewer. None of these findings are tracked
for individual fix because the files they live in are being deleted, not patched. They are preserved
here for provenance and as the evidentiary record for *why* a full rewrite was chosen over incremental
repair: 76 confirmed, independently-adversarially-verified defects in a ~5600 LOC subsystem, 23 of them
high severity, spanning every one of the 5 audited dimensions, is a systemic-quality signal, not a
punch list.

**Raised:** 94 findings across 5 dimensions. **Confirmed (survived adversarial refutation):** 76.
**Refuted:** 18. **By severity:** high=23, medium=37, low=16. **By dimension:** architecture=17,
physics-fidelity=17, rendering-catalog=12, scenarios-ux=13, ui-telemetry-wiring=17.

**Recurring patterns worth generalizing to the replacement's design** (see the design spec):
1. **Fabricated/frozen telemetry** — multiple independent readouts (force decomposition, energy
   drift, virial ratio, extended-data channels) that look live but are either structurally dead,
   compute an incomplete quantity, or freeze at a stale value under common conditions. This is the
   same failure mode the 2026-07-28 Scale-0 panel-wiring audit found — evidently not a one-off.
2. **Silent divergence between what the UI claims and what the integrator does** — overlay tooltips,
   diagnostics labels, and knowledge-base copy assert things (force sums, tag rules, mass provenance)
   that the actual code does not do, with no cross-check ever built to catch the drift.
3. **Epistemic-tag/honesty drift on the JS side of a C++->JS port** — `[IMPOSED]`/`[PARAMETRIC]`
   caveats present in the C++ source are dropped when ported to JS, and catalog `ftd_status` values
   contradict the file's own stated tagging rule for several entries.

## architecture (17 confirmed)

### [HIGH] Strong force is structurally unreachable: catalogColorId compares against color_charge values the catalog never contains

File: engine\web\js\bridge\mock-particle-engine.js:60

`catalogColorId(colorCharge)` maps `'r'`->1, `'g'`->2, `'b'`->3, else 0. The particle catalog's `color_charge` field only ever holds one of five literal values (verified by exhaustive grep over particle-catalog.js): `'none'`, `'octet'`, `'r/g/b'`, `'r-bar/g-bar/b-bar'`, `'singlet'`. There are ZERO entries equal to `'r'`, `'g'`, or `'b'`. Therefore `p.color === 0` for every particle ever created, and every strong-force gate is permanently false: `pe-force-kernel.js:90` (`toggles.strong && pi.color !== 0 && pj.color !== 0`) and `mock-particle-engine.js:282` (`toggles.strong && pi.color && pj.color`). The entire three-branch running-alpha_s ladder (Coulombic r<3, linear 3<=r<8, string-tension r>=8) in `pe-force-kernel.js:90-107` is dead code, as is `alphaSLattice`/`alphaSRunning` on the force path.

Impact: An entire advertised force term never executes. The `F_S` overlay button (scale1/ui/overlays/template.js:51-54, tooltip 'Strong / color force arrows -- F_S. Requires pe-strong toggle + colored particles.') can only ever draw a zero-length arrow field, and `peGetForceDecomposition().strong` (mock-particle-engine.js:310) is a guaranteed all-zeros Float32Array presented to the renderer as a measured force. `strongColorTint()` (pe-cloud-expander.js:81-86) is likewise dead, so quark color tinting never renders. This is exactly the Scale-0 audit failure mode: a panel/overlay displaying a number that is fabricated-by-construction rather than backed by what it claims to measure.

### [HIGH] peGetForceDecomposition().net is not the net force the integrator uses

File: engine\web\js\bridge\mock-particle-engine.js:313

`net[i3] = fcx + fgx + fsx + fmx + fsox` sums only Coulomb, gravity, strong, magnetic-dipole and spin-orbit. The actual integrator force, `computeForceOnParticle` in pe-force-kernel.js, additionally applies: the exchange/Pauli term (pe-force-kernel.js:84-88), the Lorentz v x B term (:122-143), radiation reaction (:218-232), and the relativistic 1/gamma rescale (:234-245). So `peGetForceDecomposition` is a second, independent, incomplete re-implementation of the force law that has already drifted from the canonical kernel. Meanwhile `peGetForces()` (mock-particle-engine.js:553-581) returns the true kernel buffer. Both are labelled 'net force' to consumers.

Impact: The F_net arrows drawn by controller.js:461-464 disagree with the forces actually integrated whenever exchange, lorentz, radiation or relativistic is enabled -- and `pe-w-pair` enables relativistic + relativistic_verlet by preset (scenarios.js:115), while `pe-hydrogen-fine` enables magnetic_dipole + spin_orbit (scenarios.js:107). The user sees an arrow that is not the force moving the particle, with no indication of the discrepancy. The `maxNet` value returned alongside it feeds overlay scaling, so the arrows are also mis-normalised.

### [HIGH] totalEnergy omits every potential term except Coulomb and gravity, making the Energy-Drift telemetry readout meaningless

File: engine\web\js\bridge\mock-particle-engine.js:598

`peGetDiagnostics()` computes potential energy in a private pair loop that accumulates only `pe_coulomb` (guarded by `state._pe.coulomb`) and `pe_gravity` (guarded by `state._pe.gravity`). Exchange, strong, magnetic-dipole and spin-orbit all contribute force but no potential term, so `totalEnergy = ke + pe_coulomb + pe_gravity` is not the conserved quantity of the system being integrated. Three further non-conservative sinks are also invisible to it: damping (:431-440), `clampSpeedLimit` (:370-385, silently rescales v whenever |v| > C_SPEED), and annihilation particle removal (:463-468). telemetry-hub.js:475-479 then computes `peEnergyDrift = (totalEnergy - _peInitialEnergy)/|_peInitialEnergy| * 100` against a t=0 baseline and pushes it into the `_s1_pe` ring buffer for charting.

Impact: The 'Energy drift %' chart is a headline conservation-quality metric that silently reports drift of an incomplete Hamiltonian. For `pe-hydrogen-fine` (magnetic_dipole + spin_orbit on) it will show large spurious drift that is really just unaccounted potential energy; for any scenario with damping or annihilation it conflates intentional dissipation with integrator error. `peVirial = 2*KE/|PE|` (telemetry-hub.js:474) inherits the same defect, so the virial ratio is also not the virial ratio of the simulated system.

### [HIGH] No toggle setter invalidates the force cache, so force overlays freeze at pre-toggle values while paused

File: engine\web\js\bridge\mock-particle-engine.js:659

`peSetCoulomb`, `peSetDamping`, `peSetGravity`, `peSetLorentz`, `peSetExchange`, `peSetStrong`, `peSetMagneticDipole`, `peSetSpinOrbit` (:659-666) and `peSetRadiation`, `peSetRelativistic`, `peSetRelativisticVerlet` (:685-687) plus `peSetSoftening` (:658) all mutate `state._pe` without setting `state._pe.forces = null`. Every other mutator in the file does invalidate: `peAddParticle` (:144), `peAddLockedParticle` (:158), annihilation (:467), `peSetSpinAxis` (:681). `peGetForces()` (:555) short-circuits on `!state._pe.forces || state._pe.forcesN !== particles.length` and otherwise returns the stale cached Float64Array.

Impact: With the simulation paused (a normal inspection workflow), unchecking Coulomb or changing softening leaves `peGetForces()` returning the pre-change force field indefinitely. That buffer drives both the manifest blink-rate (controller.js:380-385 -> buildPEManifestBlinkRate) and any consumer of `getScale1Forces` (capabilities/scale1.js:20), plus `peMaxForce`/`peMeanForce` telemetry. The user toggles a force off and the display keeps showing it as active. Only re-running the sim (peTick recomputes at :415) clears it.

### [HIGH] catalogSpin can never return -1: spin is a constant +1 for all particles, degenerating the exchange gate and pre-aligning all dipoles

File: engine\web\js\bridge\mock-particle-engine.js:67

`catalogSpin(entry)` returns `entry.spin > 0 ? 1 : -1`. The catalog stores spin MAGNITUDES only -- exhaustive grep of particle-catalog.js yields exactly four distinct values `0, 0.5, 1, 1.5` and zero negative entries. So the `-1` branch is unreachable and every particle receives `spin = +1` (or 0 for scalars). `initSpinAxis` (:78-82) consequently places every spin axis on `+z` with magnitude `2*|spin|`.

Impact: Two downstream consequences. (1) The exchange/Pauli gate `toggles.exchange && pi.spin !== 0 && pj.spin === pi.spin && pi.charge === pj.charge` (pe-force-kernel.js:84) collapses to 'both fermions and same charge' -- so the one advanced force actually exposed in the UI (pe-controls.js:49, whose label reads 'Exchange / Pauli exclusion: short-range repulsion between particles sharing both spin and charge') models same-charge repulsion, not exclusion, because the spin clause can never discriminate. (2) Every magnetic-dipole scenario starts with all moments co-aligned on +z, a degenerate configuration for `pairwiseMagneticDipoleForce` and `evolveParticleSpins`; `pe-hydrogen-fine` works around this by manually calling `peSetSpinAxis` (scenarios.js:170-171), which is the only reason that scenario shows any precession at all.

### [MEDIUM] Layering inversion: the bridge layer imports the scale layer, and the scale layer mutates engine-private state

File: engine\web\js\bridge\mock-particle-engine.js:57

`mock-particle-engine.js:57` does `import { applyEquilibriumOrbit } from '../scales/scale1/pe-dynamics.js'` -- the bridge/engine layer depending upward on a scale-specific UI-layer module. `pe-dynamics.js:7-10` imports back down from `../../bridge/pe-force-kernel.js`, closing the loop across the layer boundary. Worse, `pe-dynamics.js` reaches directly into engine-private state rather than going through the engine API: `state._pe.particles.findIndex` (:72), `state._pe.particles[idx]` (:77), `equilibriumOrbitSpeed(state._pe.particles, idx, state._pe, center)` (:79), and `state._pe.forces = null` (:83). It also temporarily zeroes and restores `p.vx/vy/vz` in place (:54-57) to probe the force kernel.

Impact: The STATE CONTRACT documented in the engine header (mock-particle-engine.js:13-27) lists `_pe` as engine-owned with a specific field set; a scale-layer module writes to it directly, so the contract is unenforceable. The circular cross-layer import means the bridge cannot be extracted or tested without dragging in scale1 UI code, and any future native-PE backend would have to reproduce `pe-dynamics`'s direct-state access rather than an API.

### [MEDIUM] _resetScale1Internal duplicates app.js's overlay reset and has drifted stale: three of four force-arrow layers survive it

File: engine\web\js\scales\scale1\controller.js:276

controller.js:271-282 clears viewport overlays on Scale-1 teardown, duplicating the identical list in app.js:314-325. The controller's copy calls `viewport.toggleParticleForces(false)` where app.js correctly calls all four of `togglePEForceCoulomb/Gravity/Strong/Net(false)`. `toggleParticleForces` is a one-line alias for `togglePEForceNet` only (viewport/particle-renderer.js:460: `toggleParticleForces(on) { this.togglePEForceNet(on); }`, reached via viewport.js:533), so `_peForceCoulomb`, `_peForceGravity` and `_peForceStrong` are never hidden by the controller's own reset. The four module-level flags `_showPEForceCoulomb/Gravity/Strong/Net` ARE cleared (:248-251), so the controller believes the overlays are off.

Impact: Any teardown path that calls `resetScale1`/`destroy` without also running app.js's `_resetAllVisualState` leaves three arrow layers rendered with stale geometry from the previous scenario, while the controller's state says they are off -- so no subsequent `updatePEForceDecomposition` call refreshes them. The duplication is the root cause: the same reset lives in two files and only one was updated when the single `forces` toggle was split into four.

### [MEDIUM] Four independent re-implementations of the same pairwise Coulomb+gravity loop, already drifted apart

File: engine\web\js\bridge\mock-particle-engine.js:635

The canonical force law lives in `pe-force-kernel.js:57-146`. Four copies exist: (a) `peGetForceDecomposition` (mock-particle-engine.js:261-307), which includes a verbatim duplicate of the kernel's three-branch strong ladder from :96-104 including the `rawR < 1.0` clamp and the `SIGMA_STRING` expression re-inlined as `(STRONG_ALPHA_S * K_B * K_B)` at :294; (b) `peGetExtendedData` (:635-648); (c) `peInspectParticle` (:718-734); (d) the potential-energy loop in `peGetDiagnostics` (:598-607). Copies (b), (c) and (d) implement Coulomb+gravity only.

Impact: The copies have already diverged from the kernel (see findings 2 and 3). Concretely: the Inspector panel's `fNetMag` and `acceleration` fields (:764, :769) and `peGetExtendedData().forces`/`.accelerations` (:649-651) report a Coulomb+gravity-only force, so for `pe-hydrogen-fine` the Inspector shows an acceleration that is not the particle's acceleration. Any future change to the force law must be made in five places or the panels silently desync -- there is no test or assertion tying them together.

### [MEDIUM] peGetForceDecomposition and peGetExtendedData allocate fresh typed arrays every call, breaking the module's own buffer-reuse pattern

File: engine\web\js\bridge\mock-particle-engine.js:234

`peGetForceDecomposition` allocates seven `Float32Array(n*3)` (:234-240) on every invocation, and controller.js:459-465 invokes it every frame whenever any of the four force-overlay toggles is on. `peGetExtendedData` allocates nine typed arrays (:617-624) AND runs its own private O(N^2) Coulomb+gravity loop (:635-648); controller.js:479 calls it through `telemetryHub.collectScale1Extended` every 3rd frame unconditionally -- there is no check of `activeTab`, unlike the chart-drawing switch immediately below at :514-525. By contrast `peGetParticleData` (:483-499), `peGetFieldSources` (:533-540) and `peGetForces` (:559-565) all use grow-only cached buffers (`_peBufs`, `_peFieldBufs`, `_peForcesBufs`).

Impact: Two of the five data-export methods leak the buffer-reuse discipline the other three follow, with no comment explaining the asymmetry. `peGetExtendedData`'s redundant O(N^2) pass runs 20x/second even when the diagnostics panel is closed, purely to feed ring buffers nobody is looking at. Combined with finding 10 this makes the per-frame allocation profile of Scale 1 substantially worse than the code comments suggest.

### [MEDIUM] _peParticleTypes is never pruned on particle removal, and the Hawking emitter adds particles without bound

File: engine\web\js\bridge\mock-particle-engine.js:466

Annihilation removes particles with `state._pe.particles = ps.filter((_, idx) => !toRemove.has(idx))` but there is no corresponding `_peParticleTypes.delete(id)`. Grep confirms the map's only mutations are `new Map()` (:121), `.clear()` (:133) and two `.set()` calls (:145, :159) -- no delete anywhere in the repo. Compounding this, controller.js:357-373 emits an electron/positron pair every `_BH_HAWKING_INTERVAL = 300` ticks for the `pe-micro-bh` scenario with no particle cap, no despawn, and no escape check. Meanwhile peTick runs four O(N^2) passes per tick: `_peComputeForces` at :393 and :415, `evolveParticleSpins` at :445 (which itself calls `totalBFieldAtParticle` per particle, pe-spin-dynamics.js:39-57), and the annihilation scan at :449-462.

Impact: A long-running `pe-micro-bh` session grows both the particle list and the type map without bound. Quadratic cost then grows unboundedly too, and `expandPEToCloud` silently truncates at `MAX_CLOUD_TOTAL = 100000` (pe-cloud-expander.js:14, loop guard :289) so particles beyond the cap stop rendering with no warning. The emitted pairs are given `v_out = C_SPEED * 0.60` outward but the boundary reflector at :411 keeps them inside radius 35 forever, so nothing ever leaves.

### [MEDIUM] WasmBridge.reset() nulls this._pe -- which is now the live JS particle state -- under a comment describing a C++ handle that no longer exists

File: engine\web\js\bridge\wasm-bridge.js:233

The comment at :233-238 states that '`this._pe`/`this._ae` are [C++ ParticleEngine/AtomEngine handles] bound to the OLD RenderBridge' and line 239 does `if (this._pe) { this._pe = null; }`. Since the JS path became canonical, `this._pe` is the plain object `{particles, nextId, tick, dt, soft, coulomb, ...}` created by `initPE()` (mock-particle-engine.js:115-121) -- it has no relationship to the RenderBridge at all. `dispose()` at :279 still calls `this._pe.delete?.()`, a vestige of the embind era that is now a silent no-op on a plain object.

Impact: A lattice-size `reset()` (a Scale-0 concern) silently destroys all Scale-1 particles. Afterwards `peTick()` short-circuits at mock-particle-engine.js:388 (`if (!state._pe) return`) so the sim appears frozen with no error, while `state._peParticleTypes` is left populated (reset never clears it). The comment actively misdirects anyone diagnosing this, since it asserts a C++ ownership model that was removed.

### [MEDIUM] mock-particle-engine.js is the sole PE engine for both bridges; its name and header both assert the opposite

File: engine\web\js\bridge\mock-particle-engine.js:2

File header line 2 reads 'Scale-1 Particle Engine (PE) -- MockBridge side only.' and the STATE CONTRACT block at :13-14 says '`state` must be the MockBridge instance'. In fact wasm-bridge.js:110 constructs `this._peEngine = createParticleEngine(this)` in the WasmBridge constructor, and wasm-bridge.js:779-814 forwards all 36 `peXxx()` methods to it. There is no MockBridge-conditional behavior anywhere in the module. The only in-repo acknowledgement is a three-line comment at wasm-bridge.js:107-109.

Impact: A maintainer reading the module name or its own header will conclude that WASM mode uses a native ParticleEngine and that this file is a fallback. It is the production code path in every mode. Any 'is Scale 1 lattice-backed?' question answered from the filename gets the wrong answer, and the STATE CONTRACT is documented against a class that is not the primary caller.

### [MEDIUM] Six of eleven PE force checkboxes are commented out of the DOM while presets, reset code and app wiring still drive them

File: engine\web\js\scales\scale1\ui\controls\pe-controls.js:54

The `<details>` block containing `pe-lorentz-p`, `pe-strong`, `pe-magnetic-dipole`, `pe-spin-orbit`, `pe-radiation` and `pe-relativistic` is inside an HTML comment (:54-87). Three layers still act as if those elements exist: app.js:1085-1093 registers change handlers for all six via `peToggleMap` (silently skipped by the `if (el)` guard); controller.js:152-158 calls `setCheckbox` for `pe-lorentz-p`, `pe-strong`, `pe-magnetic-dipole`, `pe-spin-orbit`, `pe-radiation`, `pe-relativistic` (no-ops via the `if (el)` guard in setCheckbox at :113-116); and `applyPEPhysicsPreset` (:132-138) still pushes all eleven values into the bridge. Separately, `relativistic_verlet` has no DOM id anywhere in the repo -- grep finds it only in JS -- yet it switches the integrator between momentum-space and velocity-space updates (mock-particle-engine.js:344-366).

Impact: `pe-hydrogen-fine` enables magnetic_dipole + spin_orbit and `pe-w-pair` enables relativistic + relativistic_verlet by preset (scenarios.js:107, :115). Those terms run in the integrator with no UI affordance to observe or disable them, and the checkbox-sync calls that were supposed to reflect them are dead. A user cannot determine from the UI which forces are active. The in-file comment at pe-controls.js:33-45 documents the intent ('stay hidden until each is checked the same way exchange was') but nothing prevents scenarios from switching them on behind the hidden controls.

### [MEDIUM] The advertised flat-buffer optimisation is defeated by per-pair object allocation inside the force kernel

File: engine\web\js\bridge\pe-force-kernel.js:145

mock-particle-engine.js:41-43 claims the flat `Float64Array(N*3)` layout 'avoids N object allocations per tick and gives ~2x speedup via cache locality on the O(N^2) pair loop'. But the call chain `computeAllForces` (:251-262) -> `computeForceOnParticle` (:209-216) -> `computePairwiseForceOnI` (:57-146) returns a fresh `{fx, fy, fz}` object literal for EVERY PAIR, i.e. N(N-1) allocations per force evaluation and 2N(N-1) per tick (peTick calls `_peComputeForces` twice). When magnetic_dipole or spin_orbit is on, `pairwiseMagneticDipoleForce` (:170-180) and `pairwiseSpinOrbitForce` (:203) each allocate a further object per pair. `evolveParticleSpins` adds two more per pair via `magneticMoment` (pe-spin-dynamics.js:16) and `dipoleFieldAt` (:32-36).

Impact: The optimisation removes O(N) allocations while leaving O(N^2) in place, so the header's performance claim is inverted at any N where it matters. At N=100 with dipoles on this is ~60k short-lived objects per tick, all GC pressure on the render thread. Anyone reading the header will believe the hot path is already allocation-free and look elsewhere for frame drops.

### [LOW] expandPEToCloud allocates two 3-element arrays per cloud point per frame

File: engine\web\js\scales\scale1\pe-cloud-expander.js:328

The inner point loop destructures the return values of `rotateOffset` (:121-134) and `stretchOffset` (:104-118) with `[ox, oy, oz] = rotateOffset(...)` and `[ox, oy, oz] = stretchOffset(...)`. Both helpers return a fresh 3-element array literal on every call, including the early-out paths at :107 and :122 which return `[ox, oy, oz]`. The loop runs once per emitted cloud point, up to `MAX_CLOUD_TOTAL = 100000` (:14). Additionally `buildPEManifestBlinkRate` allocates a `new Float32Array(n)` (:213) and a `new Set()` (:200) on every frame, and `modulateColor` (:155-163) plus `chargeFallbackColor`/`strongColorTint` allocate an array per particle per frame.

Impact: Up to 200,000 short-lived array allocations per rendered frame at the module's own documented ceiling, on the main render thread. This is the single largest allocation source in the Scale-1 frame path and is not mentioned in the file header, which describes the module purely in terms of visual semantics.

### [LOW] Physics containment radius is a magic literal duplicated from the visual boundary constant, and is skipped for two boundary shapes

File: engine\web\js\bridge\mock-particle-engine.js:408

peTick does `if (state._boundaryShape !== 'cube' && state._boundaryShape !== 'none') { ... state._reflectIntoBoundary(p, 0, 0, 0, 35); }`. The literal `35` is duplicated from `PE_VIS_BOUNDARY_R = 35` in viewport/constants.js:30, which is what particle-renderer.js:569 uses to draw the boundary sphere. Nothing links them. Additionally the containment is entirely skipped when `_boundaryShape` is `'cube'` or `'none'` -- a Scale-0 lattice-boundary setting leaking into Scale-1 particle containment.

Impact: Changing the rendered boundary radius silently desyncs it from the physics reflector, so particles would bounce off an invisible sphere or escape a visible one. With `_boundaryShape === 'none'` (a legitimate Scale-0 configuration) Scale-1 particles have no containment at all and drift out of view permanently -- notably the Hawking emitter (controller.js:366-372) launches particles outward at 0.6c and relies on this reflector to keep them on screen.

### [LOW] Accumulated dead code: unused imports, a duplicated batch API with zero callers, two exported no-op helpers, and two exported names for one function

File: engine\web\js\bridge\mock-particle-engine.js:46

Verified by repo-wide grep. (1) mock-particle-engine.js:46 imports `ALPHA` -- never referenced in the file. (2) :51-52 import `computeForceOnParticle` and `computePairwiseForceOnI` -- never referenced; only `computeAllForces` is called (:211). (3) `peApplyEquilibriumOrbitBatch` (:178-187) duplicates `applyEquilibriumOrbitBatch` (pe-dynamics.js:95-104) line-for-line and has zero callers -- scenarios.js:422 uses the pe-dynamics version; wasm-bridge.js:784 forwards the dead one. (4) `buildPEForceActivity` (pe-cloud-expander.js:263-265) is an exported alias with zero consumers. (5) `ensureCloudTemplate(_catalogId, _mass_mev)` (:267-269) is exported with zero consumers, ignores both parameters, and returns the shared unit template -- its signature implies per-species templates that do not exist. (6) `PE_VIS_BOUNDARY_R` re-export (:17) has zero consumers. (7) scenarios.js:150 destructures `G_PE` and `C_SPEED` and uses neither (controller.js:570 passes them in). (8) `_statusCache.ptime` (controller.js:82, :268) is initialised and reset but never read or written; line 487 writes `dom.statusPtime` keyed on `_statusCache.tick`. (9) `Scale1LifecycleController.mount()` (controller.js:214-216) is an empty placeholder, and `resetScale1` (:234) and `destroy` (:230) are two exported names for the identical call.

Impact: Roughly 60 lines of code and six exported symbols that no consumer reaches, plus two APIs (`ensureCloudTemplate`, `peApplyEquilibriumOrbitBatch`) whose signatures advertise capabilities the implementations do not have. A maintainer extending per-species cloud templates would reasonably start from `ensureCloudTemplate` and find it inert; one adding a batch orbit path would have to determine which of the two identical `...OrbitBatch` functions is live.

## physics-fidelity (17 confirmed)

### [HIGH] "Total Energy" / "Energy Drift" / "Virial" are Coulomb+gravity-only but labelled as the system Hamiltonian

File: engine\web\js\bridge\mock-particle-engine.js:602

peGetDiagnostics() builds totalPE from exactly two terms: `if (state._pe.coulomb) pe_coulomb += COULOMB_K_FORCE*q_i*q_j/r` (line 602) and `if (state._pe.gravity) pe_gravity -= G_PE*m_i*m_j/r` (line 604), then returns `totalEnergy: ke + pe_val` (line 609). The integrator (pe-force-kernel.js:57-146, 209-248) applies nine terms: Coulomb, gravity, exchange, strong, magnetic dipole, spin-orbit, Lorentz, radiation reaction and the relativistic rescale. None of the last seven contribute any potential-energy term. The diagnostics descriptor (ui/panels/diagnostics-panel/descriptors/scale1.js:34) titles the section "Active Hamiltonian" and exposes rows 'Potential Energy', 'Total Energy' and 'Energy Drift %' from these values; telemetry-hub.js:469-480 computes peEnergyDrift from the same number, and pe-telemetry.js:258-262 charts it as "Energy" / "Total". Scenario 'pe-hydrogen-spin' (scales/scale1/scenarios.js:107) turns magnetic_dipole and spin_orbit ON, so this is reachable from a shipped preset, not a hypothetical.

Impact: With any advanced force enabled the panel labelled "Active Hamiltonian / Total Energy" is not the system's energy, and "Energy Drift %" measures the omitted forces' work rather than integrator error. A user reading a large drift concludes the Velocity-Verlet integrator is unstable when in fact the conserved quantity was never assembled. This is the same failure mode as the 2026-07-28 Scale-0 "0.00% anisotropy" defect: a displayed number that is not backed by what its label claims.

### [HIGH] "Max Net Force" / "Mean Net Force" telemetry recomputes only Coulomb+gravity, ignoring the real force kernel

File: engine\web\js\bridge\mock-particle-engine.js:642

peGetExtendedData() re-derives forces from scratch instead of reading the kernel: `const fc = state._pe.coulomb ? -COULOMB_K_FORCE*p.charge*q.charge/r2s : 0; const fg = state._pe.gravity ? G_PE*p.mass*q.mass/r2s : 0;` (lines 642-643), writes them into `forces`/`accelerations` (649-651). telemetry-hub.js:566-570 reads `ext.forces` and feeds peMaxForce/peMeanForce (lines 626-627), which descriptors/scale1.js:84-87 renders as "Max Net Force" and "Mean Net Force". Meanwhile peGetForces() (line 553) returns the true nine-term kernel result. Two force APIs on the same object disagree by construction whenever exchange/strong/dipole/spin-orbit/Lorentz/radiation is on.

Impact: The Forces & Geometry diagnostics rows report a force that is not the force integrating the trajectories. For a colored-quark scenario with the strong force on, the strong term is the dominant contribution and is entirely absent from the reported "Max Net Force". Also note the unit is printed as 'Pl' (Planck) although the quantity is dimensionless-α × charge² / voxel², an unfounded unit label.

### [HIGH] F_net force-arrow overlay omits four enabled force terms while its tooltip claims it is their sum

File: engine\web\js\bridge\mock-particle-engine.js:313

peGetForceDecomposition() composes net as `net[i3] = fcx + fgx + fsx + fmx + fsox` (lines 313-315) — Coulomb + gravity + strong + magnetic-dipole + spin-orbit only. Exchange, Lorentz, radiation reaction and the relativistic 1/γ rescale are never computed in this function (there is no exchange channel at all). The overlay button tooltip at scales/scale1/ui/overlays/template.js:56 reads title="Net force arrows — sum of enabled force terms", and docs/USER_GUIDE.md:237 states "Scale 1 shows the current net particle force". Additionally, this function contains a third verbatim copy of the piecewise strong-force law (lines 282-297) duplicating pe-force-kernel.js:90-107, including the literals 3.0, 8.0, 0.5, -1.0 and `STRONG_ALPHA_S*K_B*K_B`.

Impact: The F_net arrows point in the wrong direction and have the wrong magnitude for any scenario using exchange/Lorentz/radiation, and the tooltip asserts they do not. Four independent force-law implementations now exist (pe-force-kernel.js full 9-term; peGetForceDecomposition 5-term; peGetExtendedData 2-term; peInspectParticle 2-term), so a change to the physics must be made in four places or the UI silently desynchronizes from the dynamics.

### [HIGH] Hydrogen energy-level diagram is rendered vertically inverted, contradicting its own inline comment

File: engine\web\js\spectroscopy.js:113

renderEnergyLevels maps energy to screen-y as `const y = topPad + (1 - E / E1) * (H - topPad - botPad);` (line 113), directly under the comment "// Map energy: E1 (most negative) at bottom, 0 at top". Since E_n/E1 = 1/n², the factor (1 - E/E1) is 0 at n=1 and →1 at large n, so in SVG coordinates (y increasing downward) the ground state is drawn at the TOP. Executed values: n=1 → y=15.0 (−13.61 eV), n=2 → y=138.8, n=3 → y=161.7, n=6 → y=175.4 (−0.38 eV). The E=0 ionization dashed line is drawn at `const y0 = topPad;` = y=15 (line 141), i.e. exactly on top of the n=1 level. The correct mapping is `topPad + (E/E1)*range`. This card is live — ui/app-ontic.js:79/99/242 renders it and rebinds it to the Physics Z slider, and scales/scale0/ui/overlays/p1-observables/hydrogen.js:35 renders it too.

Impact: Users see the hydrogen ladder upside-down: the −13.6 eV ground state at the top with the "ionized" line printed over it, and the −0.38 eV n=6 level at the bottom. The transition arrows inherit the inversion. This is a textbook diagram whose whole pedagogical content is the level ordering.

### [HIGH] Decay-rates panel footer tells the user all masses come from framework integers; half the rendered masses are PDG constants

File: engine\web\js\decay-rates.js:264

renderDecayRates emits the footer "Masses from integers {N_c=3, b_3=7, N_eff=13} via ontic chain." (line 264) and the card title "Decay Rates (Fermi theory + ontic masses)" (line 251). The table it renders (particleNames() line 215) contains six rows. Of those: neutron mass = `M_PROTON_PHYS + DELTA_NP` where M_PROTON_PHYS is re-exported PDG M_P_PHYS (lines 31-32); pion mass = M_PI_CH_PHYS, the PDG charged pion (line 33); proton mass = M_P_PHYS, explicitly commented "Physical mass (PDG)" (line 201). The electron row uses K_B, an [IMPOSED] calibration anchor, not an integer combination. Only muon and tau actually come from MU_RATIO/TAU_RATIO. constants.js:240-246 correctly labels these as "measured (Particle Data Group) values, NOT the FTD-derived framework values", so the panel copy contradicts the constants module it imports from.

Impact: A user-visible panel attributes PDG inputs to the FTD ontic chain — precisely the overclaim CLAUDE.md's epistemic discipline forbids. Three of six displayed masses are external empirical inputs presented as framework outputs.

### [HIGH] Gravity force arrows and gravity-field overlay silently render exactly zero for lepton scenarios (Float32 underflow)

File: engine\web\js\bridge\mock-particle-engine.js:236

peGetForceDecomposition allocates `const gravity = new Float32Array(n * 3);` (line 236) and fields.js:142 does the same for samplePEGravityField. G_PE = 5.3387e-46 (constants.js:203). Executed: electron-electron pairwise gravity at r=1 is 1.394e-46, which rounds to exactly 0.000e+0 in Float32 (min subnormal 1.401e-45); the field overlay value for a unit test mass from an electron source is 2.728e-46 → also exactly 0. Consequently maxGravity = 0, and viewport/particle-renderer.js:423 computes `arrowScale * Math.log(1 + mag/(maxForce*visGain + 1e-20)*10)` with mag = 0, drawing nothing. Multiplying by GRAVITY_VIS_GAIN = 1.87e43 afterwards cannot recover it (0 × gain = 0). Protons survive (4.700e-40) only because they are ~1836× heavier.

Impact: The "Gravity F" and "F_g" overlays render an empty scene for every electron/positron scenario, indistinguishable from the toggle being off or the physics being disabled. The tooltip (overlays/template.js:33) says "Arrows may be invisible at particle scale", which reads as a physical statement about gravity's weakness, but the actual cause is a buffer-precision underflow — the value was destroyed, not merely small. A Float64Array would preserve it and the log-normalised arrows would render.

### [MEDIUM] fineStructureCorrection is missing the Z² factor and scales as Z² instead of Z⁴

File: engine\web\js\spectroscopy.js:33

The implementation is `return E_n * ALPHA*ALPHA / n * (1.0/(j+0.5) - 3.0/(4.0*n));` (line 33). The standard result is ΔE_fs = E_n·(Zα)²/n²·[n/(j+½) − 3/4]; the code uses α² where (Zα)² is required. E_n itself already carries one Z², so the total scaling is Z² rather than Z⁴. Executed check: fineStructureCorrection(2, 0.5, 10)/fineStructureCorrection(2, 0.5, 1) = 100.0, where the correct ratio is 10000. The function takes a Z parameter and is exported, and the sibling renderEnergyLevels is bound to a live Z slider (ui/app-ontic.js:92-100), so Z ≠ 1 is a first-class UI state for this module.

Impact: The fine-structure splitting is wrong by a factor Z² for every hydrogen-like ion the Z slider can select — a factor of 100 at Z = 10 and 6724 at Z = 82. The function is currently not called by any renderer, so this is a latent defect rather than a live wrong readout, but it is an exported API of a module whose whole purpose is Z-parameterised hydrogenic spectra.

### [MEDIUM] Mott cross-section polar curve renders with negative radii, reflecting the curve through the plot origin

File: engine\web\js\cross-sections.js:206

renderCrossSections normalises both curves with limits taken from the Rutherford data only: `const logMin = Math.min(...ruthData); const logMax = Math.max(...ruthData); const norm = (v) => (v - logMin)/(logMax - logMin + 1e-10);` (lines 204-206), then applies norm() to mottData at line 212. Because Mott = Rutherford × (1 − β²sin²(θ/2)) ≤ Rutherford, log10(Mott) falls below logMin at large angles. Executed over the plotted range (10°–170° in 5° steps, 1 MeV, Z=79): 13 of 33 sampled angles produce a negative normalised radius, minimum −0.2166. A negative rM at lines 214-217 places the point on the opposite side of the centre.

Impact: The blue Mott curve draws a spurious lobe passing through and inverting about the plot origin — visually reading as a large backscatter feature that does not exist. The plot is also computed with the non-relativistic Rutherford formula (E_kin in the denominator) at 1 MeV where γ = 2.96 for electrons, so the underlying curve is outside its own validity domain.

### [MEDIUM] Six of nine force toggles have no UI surface at all yet scenarios enable them

File: engine\web\js\scales\scale1\ui\controls\pe-controls.js:56

The "Advanced Forces (Phase 2)" <details> block containing pe-lorentz-p, pe-strong, pe-magnetic-dipole, pe-spin-orbit, pe-radiation and pe-relativistic is entirely inside an HTML comment (lines 54-84), so those six inputs do not exist in the DOM. scales/scale1/controller.js:152-158 still calls setCheckbox() on all six ids; setCheckbox (line 113) no-ops on a null element. Meanwhile scenarios.js:107 sets `physics: { magnetic_dipole: true, spin_orbit: true }` and scenarios.js:115 sets `{ relativistic: true, relativistic_verlet: true }`, and controller.js:130-138 pushes these into the engine. The diagnostics descriptor (ui/panels/diagnostics-panel/descriptors/scale1.js:18-27) surfaces only coulomb, gravity, damping and relativistic — magnetic_dipole, spin_orbit, strong, exchange, lorentz and radiation appear in no row, although telemetry-hub.js:482-488 collects all eleven toggle states.

Impact: A shipped scenario can silently activate force terms that the user cannot see, cannot disable, and that are simultaneously excluded from the Total Energy, Energy Drift and Max Net Force readouts. There is no surface anywhere in the dashboard from which a user could determine that spin-orbit and magnetic-dipole forces are running.

### [MEDIUM] decay-rates.js header calls the lifetimes "genuine FTD predictions" while four of five consume PDG/lattice inputs

File: engine\web\js\decay-rates.js:6

The module header (lines 6-11) states "The Fermi coupling G_F is FTD's own tree-level value… so these lifetimes are genuine FTD predictions". Only muonLifetime() is FTD-only. neutronLifetime() imports V_UD = 0.974, G_A = 1.2756 and F_N = 1.6887 (constants.js:307-309, block-tagged "[PARAMETRIC PDG / lattice]"); pionLifetime() additionally uses F_PI = 130.2 (lattice QCD) and M_PI_CH_PHYS (PDG); tauLifetime() multiplies by an inline measured leptonic branching ratio `const BR_leptonic = 0.1785;` (line 86). I verified the header's *numerical* claims and they are accurate — executed ratios vs measured: muon 1.117, tau 1.128, neutron 1.174, pion 1.158, and G_FERMI = 1.0982e-5 GeV⁻² is 5.84% below CODATA, matching the stated "~5.8%" and "12–17% high". It is only the provenance framing that overreaches.

Impact: Three of the four non-muon lifetimes are FTD's G_F combined with four to six externally measured constants; describing them collectively as "genuine FTD predictions" conflates a parametric insertion with a derivation, which the same repo's constants.js explicitly warns against at lines 240-246.

### [MEDIUM] [IMPOSED]/[PARAMETRIC] tags and validity caveats present in the C++ source are dropped in the JS port that actually runs

File: engine\web\js\bridge\pe-force-kernel.js:15

Scale 1 is JS-only (wasm-bridge.js constructs _peEngine = createParticleEngine and forwards every peXxx()), so pe-force-kernel.js is the sole runtime path — yet it strips the epistemic annotations its C++ source carries. (a) alpha_s_running: engine/src/eft/qcd_one_loop_perturbative.cpp opens with "[IMPOSED] — Imported one-loop QCD running coupling from perturbative QFT. NOT a lattice-measured β-function… Epistemic tag: [IMPOSED] / [PARAMETRIC]… This formula is an external-physics insertion", plus "Valid for m_b < Q < m_t". The JS port (lines 15-34) reproduces the formula and the non-perturbative placeholder note but carries no tag and no validity range. (b) The relativistic correction: engine/src/particle_engine.cpp:351-359 carries a nine-line CAVEAT — "crude, NON-COVARIANT approximation… only matches the transverse case… wrong for the longitudinal direction (which should scale as 1/gamma^3)… neither transforms forces between frames nor conserves relativistic momentum… Kept only as a cheap 'mass grows near c' visual cue". The JS equivalent (lines 234-245) has no comment at all. Separately, alpha_s_lattice maps r ∈ [1,∞) → Q = 2/r ∈ (0,2] GeV, entirely outside the stated m_b < Q < m_t (4.18–172 GeV) validity window at every separation the engine uses, and B0_NF5 hardcodes n_f = 5 active flavors although Q ≤ 2 GeV implies n_f = 3 or 4.

Impact: The file a future reader will consult to understand what Scale 1 computes presents an imported perturbative-QFT formula and an admittedly non-covariant hack as untagged engine physics. Under CLAUDE.md's discipline the tag must travel with the formula; here it was lost exactly at the boundary where the formula became the production path.

### [MEDIUM] Exchange ("Pauli") gate is degenerate: catalogSpin() returns +1 for every particle, so it fires between distinguishable species and between identical bosons

File: engine\web\js\bridge\pe-force-kernel.js:84

The gate is `toggles.exchange && pi.spin !== 0 && pj.spin === pi.spin && pi.charge === pj.charge`. `p.spin` is set by mock-particle-engine.js:67-70: `catalogSpin(entry) { if (!entry || !entry.spin) return 0; return entry.spin > 0 ? 1 : -1; }`. Every spin value in particle-catalog.js is non-negative (measured distribution: 9× spin:0, 35× spin:0.5, 8× spin:1, 2× spin:1.5), so catalogSpin returns +1 for all 45 spin-carrying species and −1 for none. The condition therefore collapses to "both have spin AND charges are equal". Consequences: (a) an electron and a muon (both charge −1) repel via "Pauli exclusion" although they are distinguishable; (b) two W⁺ bosons (spin 1, charge +1) get fermionic repulsion; (c) peSetSpinAxis (mock-particle-engine.js:668-683) mutates spin_ax/ay/az but never p.spin, so two electrons the user has set anti-parallel still repel. The control tooltip (ui/controls/pe-controls.js:52) claims "short-range repulsion between particles sharing both spin and charge", and the surrounding comment at line 39 asserts "genuine Pauli-exclusion behavior".

Impact: The only advanced force exposed in the visible UI does not implement the exclusion principle it claims. It cannot distinguish identical from distinguishable fermions, ignores the spin orientation the user can set and the viewport draws as a purple arrow, and applies fermionic statistics to integer-spin particles. The interaction form is also a Gaussian exp(−r²/9)/r², not a Yukawa, though the hidden strong-force tooltip (pe-controls.js:65) calls the strong term "Yukawa".

### [MEDIUM] Strong force jumps discontinuously by 6.27× at r = 8 voxels, breaking Verlet energy behaviour

File: engine\web\js\bridge\pe-force-kernel.js:103

The piecewise law is α_s(r)·cf/r² for r<3, α_s(r)·cf/(3r) for 3≤r<8, and constant SIGMA_STRING·cf for r≥8 (lines 96-104). The 3-voxel seam is continuous (both branches give α_s/9). The 8-voxel seam is not. Executed: at r=7.99 the transition branch gives |F/cf| = 0.041719; at r=8.0 the string branch gives SIGMA_STRING = STRONG_ALPHA_S·K_B² = 1.0·0.511² = 0.261121. Jump factor = 6.266. SIGMA_STRING is defined inline at pe-force-kernel.js:13 with no comment and no epistemic tag; it is a dimensional analogy (a MeV² mass-squared used as a voxel-space force) unrelated to the LEDGER FTD-0025 confinement value σ = 0.209.

Impact: A step discontinuity in a conservative force makes Velocity-Verlet energy behaviour path-dependent: particles crossing r=8 gain or lose energy depending on crossing direction and timestep, and the effect is invisible in the Total Energy readout because the strong term is not in the potential sum at all (see finding 1). The confinement regime is also 6× stronger than the transition regime it is meant to continue.

### [MEDIUM] Hardcoded magic numbers duplicate constants.js exports that were created to prevent this drift

File: engine\web\js\bridge\pe-force-kernel.js:10

constants.js:399-415 declares a Strong-force block whose own comment states it was "promoted here so any tuning change propagates to every callsite at once", exporting STRONG_R_COULOMB=3.0, STRONG_R_LINEAR=8.0, STRONG_TRANSITION_DENOM=3.0, STRONG_COLOR_REPEL=0.5, STRONG_COLOR_ATTRACT=-1.0. pe-force-kernel.js imports none of them and instead writes the literals 3.0, 8.0, 3.0, 0.5, -1.0 inline (lines 91, 96, 99, 101). Four further constants exist only as JS literals with no constants.js counterpart: Q_LATTICE = 2.0 (line 10), EXCHANGE_RANGE_SQ = 9.0 (line 11), LAMBDA_QCD = 0.215 (line 21), and SIGMA_STRING = STRONG_ALPHA_S*K_B*K_B (line 13) — all four are hand-copies of C++ values (engine/include/ftd/constants.h:269, 338, 286 and engine/include/ftd/ontic/gauge_couplings.h:176, the last of which is tagged [SELECTION] in C++ and untagged here). constants.js:406 even refers to "a genuine linear SIGMA_STRING tension" for a constant it does not export. The same five strong-force literals are duplicated a second time in mock-particle-engine.js:283-294.

Impact: Eleven physics parameters in the Scale-1 force law are outside the documented single source of truth, three of them duplicated at two JS callsites and cross-copied by hand from C++. Any retune of the strong force in constants.js or the C++ header silently fails to reach the code that actually runs, and the two JS copies can diverge from each other.

### [LOW] Gamow factor approximates daughter mass number by atomic number, distorting tunneling probabilities several-fold

File: engine\web\js\decay-rates.js:144

gamowFactor computes `const m_daughter_MeV = Z * AMU_MEV; // approximate` (line 144), i.e. it sets the mass number A equal to the atomic number Z. For alpha emitters A ≈ 2.5·Z (e.g. Rn-222 daughter Z=86, A=218), so the daughter mass is understated by ~60%. This propagates into the reduced mass: for Z=86 the code gives m_red = 3560 MeV against the correct 3660 MeV, a 2.7% error, which enters the exponent as 2πη ≈ 80 → a ~1.4% shift in the exponent → roughly a factor-3 error in the returned tunneling probability. The rest of the formula is correct: η = Z_1·Z_2·α·√(m_red/2Q) with Z_2 = 2 hardcoded for the alpha particle, and T = exp(−2πη) is the standard Gamow form.

Impact: The exported gamowFactor returns tunneling probabilities wrong by roughly a factor of 3 for heavy alpha emitters. The function is currently not consumed by renderDecayRates, so it is a latent API defect, but the signature invites callers to pass a daughter Z and receive a physical probability.

### [LOW] tauLifetime docstring states the inverse of what the code does, and the branching ratio is an inline literal

File: engine\web\js\decay-rates.js:81

The docstring says "BR_correction accounts for hadronic channels: ~1/0.1785 for leptonic BR" (line 81) while the code multiplies: `return muonLifetime() * Math.pow(massRatio, 5) * BR_leptonic;` with BR_leptonic = 0.1785 (lines 86-88). The code is correct (τ_τ = τ_μ·(m_μ/m_τ)⁵·B(τ→eνν̄)); the comment describes a division that would make the result 31× too large. The value 0.1785 is a PDG measurement hardcoded inline rather than living in the constants.js "[PARAMETRIC PDG]" block alongside V_UD/G_A/F_N/F_PI. The catalog entry two functions below also reports `branching: '~35.7%'` (line 172), the two-channel total, next to a lifetime computed from the single-channel 17.85%.

Impact: A reader correcting the code to match its own comment would introduce a 31× error. The measured branching ratio is an undocumented external input in a module whose header claims the results are FTD predictions, and it sits outside the constants file that owns every other PDG input in this module.

### [LOW] Micro-BH Hawking demo creates mass-energy from nothing with no back-reaction, stepping the Total Energy chart

File: engine\web\js\scales\scale1\controller.js:365

Every _BH_HAWKING_INTERVAL = 300 ticks the controller injects an e⁻/e⁺ pair via bridge.peAddParticle at r = _BH_HORIZON_R + 0.5, each with mass _BH_TEST_MASS = K_B and speed 0.6·C_SPEED / 0.18·C_SPEED (lines 365-372). _BH_MASS = 5000 is a fixed constant that is never decremented, so the emitting body loses nothing. The toy is correctly and prominently tagged at lines 70-72 ("[IMPOSED] pedagogical toy values — the micro-BH demo is Newtonian gravity + a visual horizon/emission cadence, NOT a GR solver") and in docs/USER_GUIDE.md:155, which is good practice. The gap is downstream: nothing in the diagnostics or charts surface distinguishes injected energy from integrator drift.

Impact: In the pe-micro-bh scenario the "Total Energy" chart and "Energy Drift %" row climb in discrete steps every 300 ticks from particle creation, with no annotation. Because the energy accounting (finding 1) does not model the source, a viewer sees what looks like catastrophic integrator failure. Net charge is correctly conserved (e⁻ + e⁺), so that row stays clean.

## rendering-catalog (12 confirmed)

### [HIGH] formatMass() under-reports sub-keV masses by 1e3–1e9×; every neutrino renders as "0.0 neV"

File: engine\web\js\particle-catalog.js:677

Two of the five branches use the wrong power-of-ten. Line 679: `if (mass_mev < 1e-6) return (mass_mev * 1e6).toFixed(1) + ' neV'` — 1 MeV = 1e15 neV, not 1e6, so the value is 1e9× too small. Line 680: `if (mass_mev < 1e-3) return (mass_mev * 1e3).toFixed(1) + ' eV'` — 1 MeV = 1e6 eV, not 1e3, so the value is 1e3× too small (it actually prints keV labelled eV). Executed against the catalog's own constants (M_NU_E_PHYS=4.1e-9, M_NU_MU_PHYS=8.58e-9, M_NU_TAU_PHYS=4.955e-8 from constants.js:288-290): all three return the string "0.0 neV". A 500 eV mass returns "0.5 eV". The keV/MeV/GeV branches are correct, which is why this went unnoticed. A correct replacement already exists and is unused: `formatMassCompat` at engine/web/js/units.js:377. The buggy function is the one wired to both display sites: engine/web/js/zoo.js:81 (Zoo card mass field) and engine/web/js/inspector/scales/particles.js:55 (PE inspector mass field).

Impact: The Particle Zoo and the Scale-1 inspector display all three neutrino masses as "0.0 neV" — a fabricated zero — directly contradicting the catalog's own adjacent comments ("4.1 meV upper bound", particle-catalog.js:124-125) and the ftd_formula strings on the same card ("≈ 4.1 meV", "≈ 8.6 meV", "≈ 49.6 meV"). Any species between 1 eV and 1 keV would silently display 1000× low.

### [HIGH] ftd_accuracy is a hardcoded literal that contradicts ftd_formula by up to 71× for the entire quark/meson/hyperon block, while the Zoo tooltip presents it as a measured deviation

File: engine\web\js\particle-catalog.js:191

Nothing in the codebase ever evaluates `ftd_formula` (verified: the only consumers of ftd_formula/ftd_accuracy/ftd_status are zoo.js:66,67,84). `ftd_accuracy` is a literal typed next to each entry. I evaluated every formula string using the catalog's own imported constants (N_C=3, N_BASE=4, B_3=7, N_EFF=13, SIN2_WEINBERG=N_C/N_EFF=3/13, ALPHA, MU_RATIO=207, TAU_RATIO=3477 from constants.js:62-141) against the same file's mass_mev. The lepton/proton/W/Higgs rows reproduce their stated accuracy to three significant figures — muon 0.112% vs stated 0.11 (line 91), tau −0.006% vs 0.007 (line 109), proton +0.017% vs 0.017 (line 376), W −0.003% vs 0.02 (line 324), Higgs −0.361% vs 0.36 (line 359) — proving the column is meant literally as (formula − measured)/measured. The quark/meson/hyperon rows do not: up `m_e·N_base·sin²θ_W` = 0.4717 MeV vs mass_mev 2.16 → −78.2%, stated 5.0 (line 191); down `m_e·(b₃+N_c−1)·sin²θ_W` = 1.061 vs 4.67 → −77.3%, stated 3.0 (line 209); strange = 18.31 vs 93.4 → −80.4%, stated 2.0 (line 227); charm `m_e·MU_RATIO·b₃·sin²θ_W/α` = 23415 vs 1270 → +1744%, stated 1.5 (line 245); bottom `m_τ·N_c·sin²θ_W/α` = 168573 vs 4180 → +3933%, stated 1.0 (line 263); pion+ `m_e·MU_RATIO·N_c·sin²θ_W/α` = 10035 vs 139.57 → +7090%, stated 2.0 (line 519); Λ `m_p + m_s` = 1031.7 vs 1115.7 → −7.5%, stated 1.0 (line 421); J/ψ `2·m_c` = 2540 vs 3096.9 → −18.0%, stated 1.0 (line 609); Υ `2·m_b` = 8360 vs 9460.3 → −11.6%, stated 1.0 (line 619); neutron = 940.79 vs 939.57 → +0.130%, stated 0.02 (line 400). Nine further rows (Σ⁺/Σ⁰/Σ⁻/Ξ⁰/Ξ⁻/Ω⁻/Δ⁺⁺ at lines 441/453/465/475/485/495/505, kaons/η at 549/569/589) attach a 1.0–2.0% accuracy to a non-formula placeholder string ("quark model + FTD masses", "ChPT with FTD quark masses") that cannot be evaluated at all.

Impact: engine/web/js/zoo.js:77 renders this number in every Zoo card with the tooltip "FTD-predicted mass deviation vs measured", color-coded by epistemic status. For roughly two thirds of the catalog the displayed percentage is not the deviation between the displayed formula and the displayed mass — it asserts 1–5% agreement where the actual disagreement is 8%–7090%. This is precisely the Scale-0 "fabricated reading presented as a positive result" failure mode, applied to the framework's headline claim (mass prediction) and shown to users as if it were computed.

### [HIGH] Up/down quark ftd_formula strings embed the PDG value as their "≈" annotation, so a 4.6× disagreement reads as an exact match

File: engine\web\js\particle-catalog.js:190

Line 190: `ftd_formula: 'm_e·N_base·sin²θ_W ≈ 2.16 MeV'`. Evaluated with the file's own imports, m_e·N_base·sin²θ_W = 0.511 × 4 × (3/13) = 0.4717 MeV. The annotated "≈ 2.16 MeV" is bit-identical to M_U_PHYS (constants.js:274, `export const M_U_PHYS = 2.16; // up quark`), i.e. the PDG value pasted into the prediction slot. Line 208 is the same pattern: `'m_e·(b₃+N_c−1)·sin²θ_W ≈ 4.67 MeV'` evaluates to 1.0613 MeV, while 4.67 is exactly M_D_PHYS (constants.js:275). These formula strings exist nowhere else in the repository (grep for `m_e·N_base·sin` across docs/, scripts/, engine/ returns only this file), so there is no derivation document that could license a different reading of the symbols.

Impact: The Zoo card shows the FTD prediction and the measured mass side by side and they agree to the digit, because the "prediction" is the measurement. A reader comparing the two columns cannot detect the 4.6× (up) and 4.4× (down) failure of the stated expression. This is the anti-pattern CLAUDE.md's Epistemic Discipline names explicitly — a substitution presented as a match.

### [MEDIUM] Cloud halo rotation uses ω = |r×v|/(m·r²) — a spurious 1/m — and multiplies by absolute wall-clock time instead of integrating

File: engine\web\js\scales\scale1\pe-cloud-expander.js:137

Two independent defects in `orbitalSweepAngle`. (1) Lines 138-140 compute `lx,ly,lz` as the bare cross product c×v, which is the SPECIFIC angular momentum L/m, not L. Line 145 then divides by mass again: `const omega = Lmag / (m * r2)`. The correct classical rate for r×v is |r×v|/r². The extra 1/m makes the halo spin rate scale as 1/mass with no physical basis: for the same orbit an electron cloud (m=0.511) spins ~2× too fast and a proton cloud (m=938.27) spins ~1836× slower than an electron's — effectively frozen. The docstring on line 136 states the formula the code does not implement, and engine/web/docs/USER_GUIDE.md:161 repeats it to users as "classical ω = |L|/mr²". (2) Line 148 returns `angle: omega * frameSec * 0.35` where `frameSec` is the absolute animation clock (controller.js:381, `now * 0.001`). The angle is ω(t)·t, not ∫ω dt. Any change in ω is amplified by the elapsed session time: after 10 minutes, a Δω of 1e-3 produces a 0.36 rad instantaneous jump in halo orientation.

Impact: A documented physics claim in the user guide is wrong in the code that implements it. Heavy-particle clouds appear rigid while light-particle clouds spin, a mass artifact a viewer would read as physics. The t-multiplication makes cloud orientation jitter progressively worse over a long session, and it is not reproducible across runs (the same scenario at t=30s and t=1800s renders different halo orientations for identical dynamics).

### [MEDIUM] Inspector picking maps a compacted geometry index through an uncompacted cloud→particle table, selecting the wrong particle whenever boundary clipping is active

File: engine\web\js\viewport\particle-renderer.js:562

`expandPEToCloud` writes `_cloudParticleMap[out] = pid` at the dense cloud index `out` (pe-cloud-expander.js:181, 346). `updateParticles` then re-COMPACTS: the loop at particle-renderer.js:562-605 iterates raw index `i` but writes to a separate `count` cursor, incrementing `count` only for points that survive `this._insideBoundary(nx,ny,nz)` (line 580). Once any point is clipped, geometry slot k no longer corresponds to cloud index k. The inspector then does `target._cloudParticleMap[intersects[0].index]` (engine/web/js/inspector/scales/particles.js:13-19) using the Three.js geometry index against the uncompacted map. Clipping is live at Scale 1: `_boundaryMode` is forced to 'origin' for particle mode (viewport.js:931) while `_boundaryShape` is whatever persisted from Scale 0 — settable by the user via `#boundary-select` and overridden per-scenario by `boundaryShapeFor` (scales/scale0/runtime/scenario-loader.js:42-45, 681). Neither `loadPEScenario` nor `_resetScale1Internal` (controller.js:238-283, 534-586) resets the boundary shape on entry to Scale 1.

Impact: With any non-cube boundary shape carried over from Scale 0, clicking a particle opens the inspector on a different particle — wrong name, symbol, mass, charge, velocity, forces — with no error and no visual cue. The offset grows with the number of clipped points, so the mis-selection is systematic, not sporadic.

### [MEDIUM] expandPEToCloud allocates two throwaway arrays per cloud point per frame — up to 200k allocations/frame at buffer saturation

File: engine\web\js\scales\scale1\pe-cloud-expander.js:328

Lines 328-329 call `rotateOffset(...)` and `stretchOffset(...)` inside the per-point inner loop; both return freshly-constructed array literals (line 129-133 and line 107/117 respectively) that are immediately destructured and discarded. The per-particle path adds three more: `catalog.display_color.slice()` (line 312), `strongColorTint` (lines 82-85), `modulateColor` (lines 158-162). On top of that, `buildPEManifestBlinkRate` allocates `new Set()` (line 200) and `new Float32Array(n)` (line 213) on every single frame, and controller.js:385-386 calls it unconditionally every frame. Measured against the module's own sizing functions (`pointCountForParticle`/`visualLocalizationRadius`): 40 protons → ~6.4k cloud points → ~12.9k array allocations per frame; ~800 electrons saturates MAX_CLOUD_TOTAL=100000 → ~200k allocations per frame. Everything else in this subsystem is carefully pre-allocated (the module-level `_cloudPos/_cloudCol/_cloudSize/_cloudPhase/_cloudRate` buffers at lines 19-24, the `_peBufs` reuse in mock-particle-engine.js:482-484), so the inner loop is the sole allocator.

Impact: Sustained minor-GC pressure proportional to cloud size at 60 fps. This is the dominant scaling risk in the render path and it defeats the pre-allocation strategy the rest of the module was written around; the fix is purely mechanical (write into the caller's locals) but the cost is invisible until particle counts rise.

### [MEDIUM] Cloud point count is systematically ~12–14% below pointCountForParticle because the in-ball rejection is counted against the loop bound

File: engine\web\js\scales\scale1\pe-cloud-expander.js:319

Line 319 sets `const n = Math.min(pointCountForParticle(mass, radius), tmpl.n, MAX_CLOUD_TOTAL - out)`, then the inner loop `for (let j = 0; j < n; j++) { if (!tmpl.inBall[j]) continue; ... }` (lines 321-322) consumes template slots that are then discarded. The template is a σ=0.42 Gaussian (line 55-57) tested against the unit ball (line 69); I reproduced the generator and measured the accept fraction at 0.863 over all 4000 slots, and 0.88 / 0.88 / 0.86 / 0.86 over the first 144 / 161 / 1000 / 2800 slots. So a particle whose requested count is 144 renders 127 points, and 2800 renders 2410. Because `inBall` is a fixed per-index mask regenerated with Math.random() at each `clearCloudAndTrails()` (line 397 nulls `_unitTemplate`), the shortfall is a different arbitrary fraction after each scenario reset. Separately, the shortfall is non-uniform across n: 91.7% accepted at n=24 versus 85.8% at n=500, so small (light, low-r_eff) particles lose proportionally fewer points than large ones.

Impact: Rendered cloud density is not the function of mass and radius the sizing code declares, and the discrepancy is neither constant nor reproducible between scenario loads. Anything that reads visual density as a proxy for the particle's mass or localization is reading a number the code did not intend to produce.

### [MEDIUM] Five overlay buffers silently truncate at hardcoded caps with no signal to the caller or the user

File: engine\web\js\viewport\particle-renderer.js:298

Each overlay clamps to a private literal and drops the remainder without returning a status: trails at 50 particles × 200 segments (line 298 `const MAX_SEGMENTS = 50 * 200`, enforced by `seg < maxSegments` at line 341 — the Map is iterated in insertion order, so the LAST particles added lose their trails entirely); velocity vectors at 200 (line 133, `Math.min(count, maxLines)` line 156); spin vectors at 200 (line 215); all four force-arrow layers at 200 (line 386, `Math.min(count, 200)` line 411); cloud points at MAX_CLOUD_TOTAL=100000 (pe-cloud-expander.js:14, 289) — and because that loop breaks on the OUTER particle index, particles past the cap render zero points and simply vanish from the view. MAX_CLOUD_TOTAL and MAX_PARTICLES (engine/web/js/viewport/constants.js:24) are declared in different modules at the same value 100000 with no assertion tying them; if MAX_CLOUD_TOTAL were ever raised, `updateParticles` would clip at `Math.min(data.count, MAX_PARTICLES)` (line 555) with no warning. Line 337 also re-hardcodes `const maxLen = 200` rather than importing TRAIL_MAX_LENGTH from pe-cloud-expander.js:15, so the two can drift apart. Note the Scale-1 black-hole demo is an unbounded particle source: controller.js:356-373 injects an e⁻/e⁺ pair every 300 ticks with no cap, removed only if they happen to overlap an opposite charge (mock-particle-engine.js:447-468).

Impact: Past 50 particles the view shows trails for some particles and not others; past 200 it shows velocity/spin/force vectors for some and not others. Both read as physics ("these particles are at rest / feel no force") rather than as a buffer limit. The cloud cap makes whole particles disappear.

### [MEDIUM] Quark ftd_status is 'selection' while W/Z/pion carrying the identical sin²θ_W dependence are 'parametric', contradicting the file's own stated rule

File: engine\web\js\particle-catalog.js:227

The file header states the rule at line 14: "Several are built on the [PARAMETRIC] sin²θ_W = 3/13 (demoted, FTD-0018)". The W and Z rows apply it explicitly, with audit provenance: "M_W is [STRUCTURALLY MOTIVATED PARAMETRIC] (depends on demoted SM-3 sin²θ_W = 3/13). Retagged 'derived'→'parametric'. Audit Section C, 2026-05-27" (lines 321-324, 343-346). But all ten quark rows whose formulas contain sin²θ_W keep `ftd_status: 'selection'` — up (191), anti-up (200), down (209), anti-down (218), strange (227), anti-strange (236), charm (245), anti-charm (254), bottom (263), anti-bottom (272) — and none carries a provenance comment. The clearest contradiction is charm at line 244, `'m_e·MU_RATIO·b₃·sin²θ_W/α'` tagged 'selection', versus pion+ at line 518, `'m_e·MU_RATIO·N_c·sin²θ_W/α'` tagged 'parametric' (line 519) — structurally the same expression with one integer swapped, on opposite sides of the tag boundary.

Impact: zoo.js:67-69 color-codes the accuracy figure directly from ftd_status — 'selection' renders as var(--warning) (a stronger claim), 'parametric' as var(--text-muted). Ten catalog rows are displayed one tier above the status their own dependency chain earns under the rule the file states in its own header. CLAUDE.md's discipline forbids exactly this upgrade-by-annotation.

### [LOW] Z boson's 0.01% accuracy is only reachable with the measured Weinberg angle, not the sin²θ_W = 3/13 its own comment names

File: engine\web\js\particle-catalog.js:346

Line 342 gives `ftd_formula: 'm_W/cos(θ_W) ≈ 91.2 GeV'` and lines 343-345 state the dependency: "M_Z is [STRUCTURALLY MOTIVATED PARAMETRIC] (M_W/cos θ_W; depends on demoted sin²θ_W)". Evaluating with the catalog's own SIN2_WEINBERG = N_C/N_EFF = 3/13 (constants.js:98): cos θ_W = √(10/13) = 0.877058, M_Z = 80369.2/0.877058 = 91634.99 MeV against M_Z_PHYS = 91187.6 → +0.491%, not the stated 0.01 (line 346). Using FTD's own predicted m_W (80366.5) gives +0.488%. The stated 0.01% is recoverable only with the measured on-shell sin²θ_W ≈ 0.2233, for which M_W/cos θ_W = M_Z is a definitional identity rather than a prediction.

Impact: A row whose comment credits FTD's 3/13 displays an accuracy that 3/13 cannot produce, understating the real deviation by a factor of ~49. The number shown is either a tautology or unsourced; either way the Zoo tooltip labels it "FTD-predicted mass deviation vs measured".

### [LOW] Higgs accuracy is stored unsigned as 0.36 with no note that the same relation is recorded as experimentally excluded at current precision

File: engine\web\js\particle-catalog.js:359

Line 358-359: `ftd_formula: 'm_e·N_eff/α² ≈ 124.8 GeV'`, `ftd_accuracy: 0.36, ftd_status: 'selection'`. The arithmetic checks out (124748.0 MeV vs M_HIGGS_PHYS = 125200.0 → −0.361%), so unlike the quark rows this figure is real. But CLAUDE.md records the canonical reading of this same relation: "−0.36% vs PDG 2024's 125.20 ± 0.11 GeV, a −4.1σ discrepancy at current precision (corrected 2026-07-01, FTD-0348: … at PDG-2024 precision the exact relation is experimentally excluded)". The catalog stores the magnitude only, drops the sign, and carries no comment — while structurally similar rows around it (proton line 373-375, W line 321-323, neutron line 397-399) all carry their LEDGER provenance inline.

Impact: zoo.js:77 renders "0.4%" in warning-yellow next to the Higgs formula, which reads as a near-match. The project's own ledger reads the identical number as a 4.1σ exclusion. A reader of the Zoo panel gets the opposite conclusion from the reader of the LEDGER.

### [LOW] applyParticleColors is dead code carrying an index-misalignment bug and dropping the global scale factor

File: engine\web\js\viewport\particle-renderer.js:672

No call site exists: grep across engine/web for `applyParticleColors` returns only the definition (particle-renderer.js:672), the thin forwarder (viewport.js:1006-1007), and two comments. Scale 1 instead gets its colors from the expander (pe-cloud-expander.js:312-314). Two defects are frozen in it: (a) the loop at line 678 writes `colAttr.array[i*3]` and `sizeAttr.array[i]` using the RAW particle index, whereas `updateParticles` compacts survivors into a separate `count` cursor (line 604) — so with any boundary clipping active the two would disagree, giving each particle another particle's color; (b) line 690 sets `sizeAttr.array[i] = Math.min(s, 40)` with no `* this.visualSettings.globalScale`, unlike line 596 in `updateParticles`, so running it would silently detach those points from the global scale slider.

Impact: No live impact today, but it is a loaded gun in shared render code: the function is exported through the public Viewport surface (viewport.js:1006) and looks like the natural API for anyone wiring catalog colors, and its two bugs are exactly the kind that produce plausible-looking wrong output rather than an error.

## scenarios-ux (13 confirmed)

### [HIGH] Strong force is unreachable at two independent levels — F_S overlay is a permanently dead control

File: engine/web/js/bridge/mock-particle-engine.js:60

`catalogColorId()` (mock-particle-engine.js:60-65) maps only the exact strings `'r'`, `'g'`, `'b'` to color ids 1/2/3. An enumeration of every `color_charge` value in particle-catalog.js yields only `'none'`, `'octet'`, `'singlet'`, `'r/g/b'`, `'r̄/ḡ/b̄'` — the exact strings 'r'/'g'/'b' appear nowhere. Therefore **every** particle in the catalog is created with `color === 0`. The strong branch in pe-force-kernel.js:90 (`toggles.strong && pi.color !== 0 && pj.color !== 0`) and the mirror in mock-particle-engine.js:282 can never fire. Independently, the second gate: the `pe-strong` checkbox is inside the commented-out HTML block at pe-controls.js:54-87, so `bridge.peSetStrong` has no reachable caller (its listener at app.js:1089 binds to a nonexistent element). No scenario in scenarios.js sets `strong: true` either. Meanwhile the overlay panel ships an `F_S` button (overlays/template.js:51-54) whose tooltip reads "Requires pe-strong toggle + colored particles", and `_updatePEForceArrowSet` with an all-zero `strong` array and `maxStrong === 0` silently emits zero-length line segments (particle-renderer.js:423-426) — the button toggles "active" and draws nothing, with no user feedback. The entire running-coupling apparatus (pe-force-kernel.js:15-47 `alphaSRunning`/`alphaSLattice`, `SIGMA_STRING`, the three-regime force at 90-107) plus `strongColorTint` (pe-cloud-expander.js:81-84) is unreachable dead code. USER_GUIDE.md:159 ("strong/Yukawa (colored quarks only)") and :161 ("Color charge tints quarks") both describe behavior that cannot occur.

Impact: A user can click F_S, see the button light up, and conclude the strong force is either zero or not present in this configuration — when in fact the code path is structurally impossible to reach. ~90 lines of QCD force kernel are dead. Two documented capabilities are false.

### [HIGH] Inspector displays catalog mass and charge, not the simulated particle's mass and charge

File: engine/web/js/inspector/scales/particles.js:55

`updatePEFields` resolves the catalog entry by type-map id and then writes `peFields.mass.textContent = formatMass(cat.mass_mev)` and `peFields.charge.textContent = chargeLabel(cat.charge)` (particles.js:55-56), discarding the live `data.mass` / `data.charge` returned by `peInspectParticle`. Every composite-nucleus scenario registers its nucleus under the `'proton'` (or `'neutron'`) catalog id while spawning a different mass and charge: `pe-helium` → `seedAtomicIon({Z:2, A:4})` → `spawnCompositeNucleus` (pe-dynamics.js:144-147) emits `peAddParticle('proton', 2, …, 4*mp)` = q +2, m 3753 MeV, but the inspector shows "Proton / 938.272 MeV / q +1". `pe-three-body` → q +2, m 1876.5 MeV shown as +1 / 938.272. `pe-deuteron` (scenarios.js:220) → m = mp+mn = 1877.8 MeV shown as 938.272. `pe-tritium` (scenarios.js:340) → 2817.4 MeV shown as 938.272. `pe-helion` → 2814.8 MeV shown as 938.272. `pe-micro-bh` (scenarios.js:390) spawns the BH anchor as `'neutron'` with mass 5000 MeV — the inspector reports 939.565 MeV. The mass/charge in the `else` branch (particles.js:62-63) uses the correct live values, but that branch only runs when the catalog lookup fails.

Impact: Clicking the nucleus in 6 of 26 scenarios reports a mass wrong by up to 5.3× and a charge wrong by a factor of 2, with no indication anything is approximate. This is precisely the 'panel shows a number not backed by what it claims to measure' failure mode.

### [HIGH] 'Micro Black Hole (Accretion)' exhibits no accretion, no orbits, and its 'in-fall' particles are given tangential escape velocities

File: engine/web/js/scales/scale1/scenarios.js:388

With `G_PE = G_DERIVED = 1/(4π·m_P²) ≈ 5.3e-46 MeV⁻²` (constants.js:203,206) and `BH_MASS = 5000`, `BH_TEST_MASS = K_B = 0.511`, `softening = 1.0`: the ring at r=16 sees F ≈ 5.3e-45, so `equilibriumOrbitSpeed` (pe-dynamics.js:63) returns v = sqrt(F·r/m) ≈ 4e-22 — the 8 ring particles and the 2 far particles are numerically frozen, and `peScaleVelocity(farId, 1.05)` scales ~0 by 1.05. Separately, the four particles named `r_fall`/`v_fall`/`angles_fall` (scenarios.js:392-399) are given velocity `(-v·sin a, 0, v·cos a)` at position `(r·cos a, 0, r·sin a)` — that vector is **tangential**, orthogonal to the radius, not infalling. With gravity negligible they travel in straight lines at 0.45c until they reflect off the r=35 containment sphere (mock-particle-engine.js:411) and bounce forever. The source comment at scenarios.js:385-387 already concedes "inspiral/accretion is unobservable on any tick budget — dynamics are negligible", yet the dropdown label is "Micro Black Hole (Accretion)" (toolbar/template.js:43) and the KB entry lists `notation: ['visual horizon','accretion','escape velocity']` (data.js:1648). Additionally the Hawking 'in-falling partner' (controller.js:370-372) coasts straight through the origin and out the far side, because Coulomb is off (scenarios.js:118) and gravity cannot capture it.

Impact: The one gravity scenario in Scale 1 demonstrates neither gravity nor accretion nor capture: a user sees 10 frozen dots, 4 dots bouncing off an invisible sphere, and pairs spawning at a decorative horizon. The scenario name and knowledge-base entry both promise accretion the code cannot produce.

### [HIGH] 'Net Force' telemetry, charts, and inspector are computed by three separate Coulomb+gravity-only loops that ignore every advanced force the integrator actually uses

File: engine/web/js/bridge/mock-particle-engine.js:642

The integrator's canonical force is `computeForceOnParticle` (pe-force-kernel.js:209-248): Coulomb + gravity + exchange + strong + magnetic dipole + spin-orbit + Lorentz + radiation reaction + relativistic rescale. Three separate reimplementations feed the UI and all of them stop at Coulomb+gravity: (1) `peGetExtendedData` (mock-particle-engine.js:642-643, `fc = coulomb ? … : 0; fg = gravity ? … : 0`), which telemetry-hub.js:566-571 reduces into `peMaxForce`/`peMeanForce`, rendered as "Max Net Force"/"Mean Net Force" (diagnostics/descriptors/scale1.js:84-87) and as the "Net Forces" chart (charts-panel/descriptors/scale1.js:32-41); (2) `peInspectParticle` (mock-particle-engine.js:726-727), which drives the inspector's `fnet` and `accel` fields (inspector/scales/particles.js:77,91); (3) `peGetForceDecomposition`, whose `net` sums only coulomb+gravity+strong+magnetic_dipole+spin_orbit (mock-particle-engine.js:313-315) — omitting Lorentz, radiation, exchange, and the relativistic rescale — and drives the F_net overlay arrows. `pe-hydrogen-fine` enables magnetic_dipole+spin_orbit (scenarios.js:107) and `pe-w-pair` enables relativistic+relativistic_verlet (scenarios.js:115); `pe-exchange` is user-toggleable (pe-controls.js:49).

Impact: In pe-hydrogen-fine the chart and diagnostics row labelled 'Net Force' silently exclude the two forces the scenario exists to demonstrate; with Exchange enabled the F_net arrow excludes the exchange term. Four divergent force implementations mean any future kernel change must be mirrored in three places or the UI drifts again.

### [MEDIUM] 'Three-Body (p⁺ p⁺ e⁻)' is a two-particle simulation

File: engine/web/js/scales/scale1/scenarios.js:209

The `pe-three-body` case calls `spawnCompositeNucleus(bridge, 2, 2, mp, RE)` (scenarios.js:211), which is a single `peAddParticle('proton', Z=2, …, mass = A*mp)` (pe-dynamics.js:144-147), then adds one electron — total particle count 2. The dropdown label is "Three-Body (p⁺ p⁺ e⁻)" (toolbar/template.js:38); the knowledge-base entry is titled "Three-Body (p⁺ p⁺ e−)" with `notation: ['few-body instability','Coulomb balance']` and the guide text "Three-body particle dynamics is where intuition starts to break. Small changes in initial condition can radically change whether the system binds, scatters, or reconfigures" (data.js:1505, 1645). The scenario's own source comment is honest — "composite Z=2 nucleus + 1 electron" (scenarios.js:208). Compounding this, the entry is filed under the "Scattering" optgroup even though the electron is given an equilibrium *orbit* (scenarios.js:213), while the genuine three-body Coulomb problem (`pe-helium`, 3 particles) is filed under "Leptons".

Impact: The one scenario advertised as demonstrating few-body chaos is a deterministic two-body Kepler orbit. A user selecting it to see three-body instability will observe a clean closed orbit and draw a false conclusion about the engine.

### [MEDIUM] Knowledge base describes 'locked' nuclear cores that no scenario has used since the composite-nucleus refactor

File: engine/web/js/ui/components/knowledge-base/data.js:1500

The KB says deuteron is "a locked proton-neutron core plus an electron" (data.js:1500), tritium "adds neutral ballast to the locked core" (:1501), helion "combines a locked 2p+n core" (:1502), with matching titles/notation at :1640-1642 ("a locked isotope-core composition demo", `notation: ['neutral ballast', 'Coulomb electron motion']`, "locked core"). scenarios.js:10 states "All scenarios use fully dynamic particles", and a repo-wide grep shows `peAddLockedParticle` has **zero** callers outside bridge plumbing (mock-particle-engine.js:150, wasm-bridge.js:782, ws-bridge.js:560, physics-harness.js:492). USER_GUIDE.md:156 already documents the correction ("composite dynamical nuclei … rather than locked multi-nucleon sculptures"), confirming the KB text is stale rather than intentional. Downstream, the "Locked / Mobile" diagnostics row (diagnostics/descriptors/scale1.js:38-39) and the "Locked" chart series (charts-panel/descriptors/scale1.js:50) are hard-wired to read 0 forever.

Impact: The in-app explanatory layer contradicts both the code and the project's own user guide for three scenarios, and two telemetry surfaces render a permanently-constant zero as if it were a measurement.

### [MEDIUM] Every honest Scale-1 caveat lives in files the dashboard never renders

File: engine/web/docs/USER_GUIDE.md:149

The `[IMPOSED]` black-hole caveat exists in exactly three places, none of which a dashboard user encounters: (a) a source comment (controller.js:70-72) that itself points at "USER_GUIDE §Scale 1"; (b) USER_GUIDE.md:149-163 — a grep across all .js/.html in engine/web shows the string `USER_GUIDE` appears only in that source comment, so the guide is never linked, fetched, or rendered by the app; (c) the knowledge-base entry (data.js:1508), reachable only by opening the KB sidebar (`open()` in knowledge-base/component.js:30) and searching for the scenario by name — the Scale-1 toolbar is a bare `<select>` with no info affordance (toolbar/template.js:1-51) and the KB component has no wiring to `pe-scenario-select`. The single always-visible caveat is the overlay footnote "Classical particle engine — sim units, not full QFT substrate" (overlays/template.js:84), which says nothing about the BH being a toy, nothing about gravity being dynamically inert, and nothing about Scale 1 never touching the lattice substrate.

Impact: A first-time user selecting 'Micro Black Hole (Accretion)', seeing a rendered event horizon and periodic e⁺e⁻ pair emission, has no path from the UI to the fact that this is a pedagogical visual with no GR and no working gravity. The epistemic-discipline layer exists but is invisible where it matters.

### [MEDIUM] Particle Zoo truncates fractional charge to ±1 while displaying the true fractional value

File: engine/web/js/zoo.js:160

`injectFromZoo` computes `const charge = p.charge > 0 ? 1 : p.charge < 0 ? -1 : 0` (zoo.js:160) and passes that to `peAddParticle` (zoo.js:162), while the card rendered directly above shows `q ${chargeLabel(p.charge)}` — the true catalog value (zoo.js:82). Quarks carry ±2/3 and ±1/3 (particle-catalog.js:188,197,206,215,…) and Δ⁺⁺ carries +2, so injecting an up quark yields a particle whose card reads "q +2/3" but whose Coulomb force is computed with q = +1 — a 1.5× error in force, 2.25× in the pair term. Separately, `canInject` requires `p.charge !== 0` (zoo.js:70), so every neutral species (neutron, π⁰, photon, all neutrinos, Z, Higgs, Λ⁰) is permanently un-injectable, with the disabled button still carrying the tooltip "Inject <name>" and no reason given. This is the only manual-spawn path, and `pe-custom` documents itself as "user injects manually via Zoo or controls" (scenarios.js:433).

Impact: The zoo shows one charge and simulates another. The 'Custom (Manual)' sandbox cannot construct any neutral-particle configuration, and injected particles start at v=0 (zoo.js:157) so opposite-charge species free-fall into contact and annihilate rather than orbiting.

### [MEDIUM] Scenario dropdown taxonomy does not describe its contents and encodes no pedagogical progression

File: engine/web/js/scales/scale1/ui/toolbar/template.js:6

All 26 `<option>` values map 1:1 onto the 26 `case` labels in scenarios.js (verified exhaustively — no orphans in either direction), but the grouping is ad hoc: the "Leptons" optgroup (template.js:6-15) contains Helium (nucleus + 2e⁻), Hydrogen and Tauonic Hydrogen (hadron-lepton bound states); "Hadrons" (:22-27) contains "Omega⁻ Scattering" while a separate "Scattering" group exists; "Scattering" (:36-41) contains pe-three-body, which is a bound orbit (scenarios.js:213); "Nuclear" (:28-32) contains three scenarios with no nuclear physics whatsoever — no strong force, no binding, just a heavier point charge in a Coulomb potential (USER_GUIDE.md:156 concedes "Nuclear binding is not emergent at this tier"); "Bosons" (:33-35) has a single entry that is a 80-GeV Coulomb two-body orbit. There is no complexity ordering, no marked starting point beyond `selected` on pe-hydrogen, and no baseline scenario (free particles / forces-off) to anchor comparisons.

Impact: A user browsing for a concept lands in the wrong group; 'Nuclear' in particular promises nuclear physics that the tier explicitly cannot produce. Nothing signals that pe-hydrogen is the intended entry point or that pe-tauonium and pe-kaonic-hydrogen are variations on the same two-body Coulomb solve.

### [LOW] Dead duplicate black-hole state block in app.js shadows the controller's live constants

File: engine/web/js/app.js:136

app.js declares `_bhActive`, `_bhHawkingTick`, `_BH_HAWKING_INTERVAL = 300`, `_BH_HORIZON_R = 3.0`, `_BH_MASS = 5000`, `_BH_TEST_MASS = K_B` at lines 136-141 under the header "Black hole scenario state (Scale 1 only) / [SELECTION] All BH constants are pedagogical choices". A grep for each identifier across app.js returns only these declaration lines — nothing reads or writes them. The live copies are controller.js:68-76, which are the ones actually consumed by `animatePE` and passed into `setupPEScenario` via `constants` (controller.js:568-569). Leftover from the app.js → scale1/controller.js extraction.

Impact: Two independent sources of truth for four physics constants; a future edit to app.js's copy would appear to work and change nothing. Also fragments the `[SELECTION]` provenance comment away from the code it annotates.

### [LOW] `getPEScenarioPreset().status` is computed for every scenario and never consumed

File: engine/web/js/scales/scale1/scenarios.js:135

`getPEScenarioPreset` returns `status: override.status || 'Scale 1 continuous N-body demo'` (scenarios.js:135). No entry in `PRESET_OVERRIDES` (scenarios.js:105-128) defines a `status` key, so the value is always the same literal. The sole caller, `loadPEScenario` (controller.js:538), reads only `preset.physics` and `preset.overlays` — `preset.status` is never referenced anywhere in the codebase. Likewise `_showPESystem = !!o.system` (controller.js:174) reads a key that none of the four overlay presets (ATOMIC/SCATTERING/GRAVITY/CUSTOM_OVERLAYS, scenarios.js:68-103) defines, so the System overlay is never preset-enabled by any scenario.

Impact: A per-scenario status/description hook exists in the data model and is silently discarded — plausibly the intended vehicle for the missing in-UI caveat text, left half-wired.

### [LOW] Hawking demo grows the particle set without bound while force arrows silently cap at 200

File: engine/web/js/scales/scale1/controller.js:355

While `_bhActive`, `animatePE` adds two particles every 300 ticks forever (controller.js:357-373) with no cap, no lifetime, and no removal path — annihilation only fires on opposite-charge contact within `r_eff` sums of 0.2 (mock-particle-engine.js:449-462), which the antipodal emission geometry makes rare. At the default 1 tick/frame and 60 fps that is ~24 particles/minute, and the force loop is O(N²) (pe-force-kernel.js:212-216) run twice per tick (mock-particle-engine.js:392,415) plus a third O(N²) pass in `peGetForceDecomposition` when force overlays are on. Independently, `_updatePEForceArrowSet` clamps to `const n = Math.min(count, 200)` (particle-renderer.js:412) with no indication that arrows beyond index 199 are omitted.

Impact: A BH scenario left running degrades quadratically with no warning; past 200 particles the force overlays silently stop representing part of the system while still appearing complete.

### [LOW] 23 of 26 scenarios have no automated coverage

File: engine/web/tests/scale1-particle-overlays.spec.js:103

Grepping every `pe-*` literal across engine/web/tests/*.spec.js yields exactly three scenario ids: `pe-hydrogen` (4 refs), `pe-micro-bh` (3 refs, scale1-particle-overlays.spec.js:107 and scale1-side-panels.spec.js:100), and `pe-w-pair` (2 refs). The remaining 23 scenarios — including every exotic atom, every hadron, all three nuclear scenarios, and all four scattering scenarios — are never loaded by a test. There is no assertion anywhere that the dropdown option set matches the `switch` case set in scenarios.js, so an option added to toolbar/template.js without a matching case would silently fall through to `default: break` (scenarios.js:434-436) and load an empty scene with no error.

Impact: Scenario-to-case drift is undetectable by CI; the sibling Scale-0 suite ships exactly such a guard (tests/scenario-parity.spec.js) and Scale 1 has no equivalent.

## ui-telemetry-wiring (17 confirmed)

### [HIGH] Every extended-data telemetry channel silently freezes at its last non-zero value when particle count hits zero

File: engine\web\js\telemetry-hub.js:541

`peGetExtendedData()` returns `null` when `N === 0` (mock-particle-engine.js:616). `collectScale1Extended` wraps its entire body in `if (ext) {` (telemetry-hub.js:541-542), so on a zero-particle tick none of the 14 `setLast()` calls (lines 622-635) run. Meanwhile `collectScale1` still pushes a fresh row that seeds those columns from `this.peLockedCount.last()`, `this.peMaxForce.last()`, … (lines 521-534) — i.e. the PREVIOUS tick's values. The stale value therefore copies itself forward forever. `this.s1.extended = ext` (line 543) is also inside the guard, so the snapshot stays stale too. Independently, `PETelemetryPanel.update` guards the same way — `if (ext) this._updateParticleTable(ext);` (pe-telemetry.js:293) — so the per-particle table keeps rendering rows for particles that no longer exist.

Impact: After annihilation empties the system (reachable via `pe-positronium`, `pe-antiprotonic-hydrogen`, and the Hawking-pair injection in `pe-micro-bh`), the diagnostics table shows `Particles: 0`, `Kinetic Energy: 0`, `Total Energy: 0` while `Locked / Mobile`, `Charge +/0/-`, `Max |v|/c`, `At Causal Cap`, `Max Net Force`, `Mean Net Force`, `RMS Velocity`, `System Radius`, `2-Body Separation` and `Radial Velocity` all keep displaying live-looking pre-annihilation numbers indefinitely, and the Per-Particle Data table lists ghost particles. This is exactly the fabricated-readout failure mode found in the Scale-0 panel audit: plausible numbers with no backing state.

### [HIGH] "Net Force" readouts are Coulomb+gravity only, while the integrator applies seven more force terms

File: engine\web\js\bridge\mock-particle-engine.js:642

The integrator's force path is `computeAllForces` (pe-force-kernel.js:251), which sums coulomb, gravity, exchange, strong, magnetic_dipole, spin_orbit, lorentz, radiation and relativistic. The telemetry path is a *separate, reduced* recomputation inside `peGetExtendedData()`: `const fc = state._pe.coulomb ? … : 0; const fg = state._pe.gravity ? … : 0;` (lines 642-643) — nothing else. `peInspectParticle` repeats the same two-term-only reduction (lines 726-733). Note `peGetForces()` (line 553) returns the *real* kernel forces, so two disagreeing force surfaces coexist.

Impact: With `pe-exchange` enabled (the one advanced toggle actually exposed in the controls card) or in `pe-hydrogen-fine` (magnetic_dipole + spin_orbit forced on by preset), the diagnostics rows `Max Net Force` / `Mean Net Force`, the `Net Forces` chart, the Per-Particle `|F|` and `|a|` columns, and the inspector's `Net |F|` / `Accel` fields all report a force that is not the force the integrator applied. The inspector tooltip literally reads "the net combined force vector acting on this particle" (panel-resources/template.js:248) — a false claim whenever any advanced term is active.

### [HIGH] Diagnostics formatter renders missing/unwired sources as "0" by design, making dead rows undetectable

File: engine\web\js\ui\panels\diagnostics-panel\formatters.js:16

`formatScalar` begins: `// Missing / not-yet-populated → render as 0 so the row is always "wired"` then `if (v === null || v === undefined) return '0';`. NaN falls through to an em-dash, but a source path that resolves to `undefined` (a typo in `source: 's1.…'`, a bridge method that vanished, a hub buffer renamed) renders as a clean `0`. DiagnosticsPanelComponent restates the policy: "Populate cells immediately so rows never show init em-dash (formatters render 0 for missing/null/undefined)" (component.js:55-56).

Impact: This is the mechanism that converts any Scale-1 wiring break into a plausible-looking measurement rather than a visible failure. `resolvePath` (table.js:20-28) returns `undefined` for any broken path, and the panel then prints `0` — indistinguishable from a genuine zero measurement. Every finding in this audit that produces a hard zero is rendered undiagnosable by this policy.

### [HIGH] PE time-series and phase-space buffers keep sampling while the simulation is paused, wiping real history

File: engine\web\js\pe-telemetry.js:292

`PETelemetryPanel.update(diag, ext)` is called from `animatePE` every 3rd frame regardless of `running` (controller.js:477-497 — `animatePE` runs every rAF frame; only the `bridge.peTick()` loop is gated on `running`). `_updateTimeSeries` unconditionally pushes to all four charts (lines 470-479) and `_updateOrbitalMechanics` unconditionally pushes to `_phaseBuf` (line 409). Neither checks `diag.tick`. This directly contradicts the hub next door, which *does* gate on tick change (`if (currentTick !== this._lastTick1)`, telemetry-hub.js:505).

Impact: Pause the sim and the energy/|p|/|L|/virial charts (TS_LEN = 200) are fully overwritten with duplicate frozen samples in ~10 s at 60 fps, and the phase-space plot (PHASE_BUF_CAPACITY = 300) in ~15 s. The user's entire recorded orbit history is destroyed by the act of pausing to look at it, and the charts flatten to a line that reads as "perfectly conserved".

### [HIGH] F_net overlay arrows omit exchange, lorentz, radiation and relativistic despite claiming to be the sum of enabled terms

File: engine\web\js\bridge\mock-particle-engine.js:313

`peGetForceDecomposition()` builds `net[i3] = fcx + fgx + fsx + fmx + fsox` (lines 313-315) — coulomb + gravity + strong + magnetic_dipole + spin_orbit. It never evaluates `toggles.exchange`, `toggles.lorentz`, `toggles.radiation` or `toggles.relativistic`, all of which the integrator's `computePairwiseForceOnI` / `computeForceOnParticle` do apply (pe-force-kernel.js:84-88, 122-143, 218-245). The overlay button tooltip says "Net force arrows — sum of enabled force terms" (scales/scale1/ui/overlays/template.js:57).

Impact: Enabling the `Exchange` checkbox changes the dynamics but not the F_net arrow, so the arrow visibly fails to explain the trajectory. In `pe-w-pair` (preset `relativistic: true`) the arrows likewise omit the relativistic rescale. The overlay presents itself as a complete force decomposition and is not one.

### [MEDIUM] Entire bridge.capabilities.scale1 factory and two TelemetryHub Scale-1 accessors are dead code, one reading a field that never exists

File: engine\web\js\bridge\capabilities\scale1.js:14

`createScale1Capabilities` exposes nine methods and its header claims `@consumers ./install.js, engine/web/js/scales/scale1/controller.js` (line 6). Grepping `tickScale1|getScale1ParticleFrame|getScale1Diagnostics|getScale1ExtendedData|getScale1Forces|getScale1FieldSources|getScale1Capabilities` across js/ and tests/ returns hits only inside the factory itself — controller.js calls `bridge.peTick()`, `bridge.peGetParticleData()`, `bridge.peGetForces()` etc. directly. Likewise `TelemetryHub.getScale1OrbitalMetrics()` returns `ext.orbital ?? ext.orbitParams ?? null` (telemetry-hub.js:857-861), but `peGetExtendedData` returns only `{count, ids, charges, masses, positions, velocities, forces, accelerations, locked}` (mock-particle-engine.js:653) — neither key exists on any path, so it always returns null. `getScale1Thermo()` (line 864) has zero callers, and `s1.runtime.capabilities` (line 501) is collected every tick and displayed by no descriptor.

Impact: Three abstraction layers claim to be the Scale-1 data contract and none of them is wired. The docstring's consumer list is factually wrong, so a future change routed through the capability factory would silently affect nothing. `getScale1OrbitalMetrics` is a null-returning stub that a panel could adopt and appear to be reading orbital data.

### [MEDIUM] F_S (strong force) overlay button is permanently dead — no preset and no control can ever enable the underlying toggle

File: engine\web\js\scales\scale1\ui\overlays\template.js:51

The `toggle-pe-force-strong` button's tooltip says "Requires pe-strong toggle + colored particles". The `pe-strong` checkbox is inside the commented-out block in the controls card (ui/controls/pe-controls.js:62-66), so it is not in the DOM and its listener in `peToggleMap` (app.js:1089) never binds. Every scenario preset inherits `strong: false` from `BASE_PHYSICS` and none overrides it (scales/scale1/scenarios.js:58 plus the full `PRESET_OVERRIDES` table at lines 103-125). The decomposition branch is gated on `toggles.strong && pi.color && pj.color` (mock-particle-engine.js:282), so the `strong` array stays all-zero.

Impact: The button toggles its `.active` class, calls `Scale1Controller.setPEForceStrong(true)` and `viewport.togglePEForceStrong(true)` (app.js:1254), and draws nothing — indistinguishable from "the strong force happens to be zero here". A user cannot tell a dead control from a null measurement.

### [MEDIUM] Overlay tooltips assert a false identity G_PE = α_G(e,e) and quote a value off by a factor of 3.28

File: engine\web\js\scales\scale1\ui\overlays\template.js:34

The Gravity-F button tooltip reads "G_PE = α_G(e,e) ≈ 1.75e-45 (FTD-0131)" and the Gravity-dyn button tooltip reads "G_PE ≈ 1.75e-45" (lines 34 and 70). constants.js defines them as distinct quantities: `G_DERIVED = 1/(4π·M_PLANCK_MEV²) ≈ 5.3e-46 MeV⁻²` and `ALPHA_G_ELECTRON = (K_B/M_PLANCK_MEV)² ≈ 1.75e-45` (dimensionless), with the header comment spelling out the relation (constants.js:195-206). They differ by 4π·m_e² ≈ 3.28. The diagnostics descriptor gets this right, listing them as two separate rows with different units (descriptors/scale1.js:26-29), and the controls-card tooltip also words it correctly (ui/controls/pe-controls.js:23).

Impact: The overlay states a numeric value for G_PE that is wrong by 3.28× and asserts an equality between a MeV⁻² coupling and a dimensionless ratio, while citing an FTD LEDGER id (FTD-0131) as authority. Two UI surfaces in the same scale contradict each other about a constant that carries a LEDGER claim.

### [MEDIUM] "2-Body Separation" and "Radial Velocity" report a hard 0 for every scenario that is not exactly two particles

File: engine\web\js\telemetry-hub.js:599

`separation` and `radialVelocity` are initialized to 0 and only assigned inside `if (n === 2)` (lines 599-610), then unconditionally written via `setLast` (lines 628-629). The diagnostics rows render them as ordinary scalars (descriptors/scale1.js:94-97), and `formatScalar` prints `0` rather than an em-dash.

Impact: `pe-helium` (3 bodies), `pe-deuteron`, `pe-tritium`, `pe-helion`, `pe-three-body` and `pe-delta-system` all display `2-Body Separation: 0 lu` and `Radial Velocity: 0 c`. A zero separation reads as "the particles are coincident", the opposite of the truth. The Two-Body Orbit chart is fed the same fabricated zeros. Unlike `_updateOrbitalMechanics`, which correctly hides its section when `ext.count !== 2` (pe-telemetry.js:341-343), these rows have no N/A state.

### [MEDIUM] Energy-drift baseline never re-latches, so any toggle/slider change or particle add/remove is reported as integrator drift

File: engine\web\js\telemetry-hub.js:475

`_peInitialEnergy` latches once per scenario (`if (this._peInitialEnergy === null && Math.abs(totalEnergy) > 1e-12)`, lines 475-477) and is cleared only by `resetScale(1)` (line 938), which only `loadPEScenario` calls (controller.js:549). The toggle handlers (app.js:1095-1116), the dt slider (1119-1127) and the softening slider (1129-1137) never re-baseline. Softening enters the potential energy directly (`r = sqrt(dx²+dy²+dz²+soft2)`, mock-particle-engine.js:601), and disabling Coulomb zeroes `coulombPE` outright (line 602). The `pe-micro-bh` scenario injects two new particles every 300 ticks (controller.js:358-372), monotonically adding energy.

Impact: Nudging the softening slider or unticking Coulomb produces a large `Energy Drift %` that reads as solver error. In `pe-micro-bh` the drift row grows without bound purely from Hawking-pair injection, presenting a scripted particle spawn as a conservation violation. The row is presented as an integrator-quality metric and is not one.

### [MEDIUM] Scenario Dynamics table omits three force/integrator toggles that presets switch on, misrepresenting the active Hamiltonian

File: engine\web\js\ui\panels\diagnostics-panel\descriptors\scale1.js:18

The `pe-runtime` section exposes only coulomb, gravity, damping and relativistic (lines 18-25). `collectScale1` actually harvests eleven toggles into `s1.runtime.toggles` (telemetry-hub.js:482-488), including `exchange`, `magnetic_dipole`, `spin_orbit` and `relativistic_verlet`. Preset `pe-hydrogen-fine` sets `{ magnetic_dipole: true, spin_orbit: true }` and `pe-w-pair` sets `{ relativistic: true, relativistic_verlet: true }` (scales/scale1/scenarios.js:107, 115). The corresponding checkboxes are commented out of the controls card (scales/scale1/ui/controls/pe-controls.js:54-87), so there is no other surface either.

Impact: Selecting "Hydrogen (spin + dipole demo)" runs two extra force terms that the diagnostics table reports nowhere and the controls panel cannot show or change — the table reads "Coulomb on / Gravity off / Damping off / Relativistic off" while five force terms are live. `relativistic_verlet` is worse: it swaps the integrator from velocity-Verlet to momentum-Verlet (mock-particle-engine.js:344-357) and has no row at all.

### [MEDIUM] Momentum labeled MeV/c and angular momentum labeled hbar with no conversion performed anywhere

File: engine\web\js\ui\panels\diagnostics-panel\descriptors\scale1.js:64

`peGetDiagnostics` accumulates `px += p.mass * p.vx` and `lx += p.y*mvz - p.z*mvy` in raw sim units — mass in MeV, velocity in lu/tick, position in lu (mock-particle-engine.js:592-596). No division by C_SPEED and no ℏ conversion happens on any path (grep for HBAR across telemetry-hub.js, pe-telemetry.js, mock-particle-engine.js and the scale1 descriptors returns nothing). Yet the descriptors declare `unit: 'MeV/c'` for Momentum/|p| (lines 60-63) and `unit: 'hbar'` for Angular Mom/|L| (lines 64-67); the charts descriptor repeats both (charts-panel/descriptors/scale1.js:27-28), as does pe-telemetry.js:264/268. The same h quantity is honestly labeled `lu²/tick` fifteen lines away in pe-telemetry.js:393.

Impact: |p| is off by 1/C_SPEED = √3 from its declared unit. |L| in "hbar" is an outright unbacked unit claim on a MeV·lu²/tick number — it invites reading a displayed 1.0 as "one quantum of angular momentum" in a classical N-body toy. This is a units fabrication of the kind CLAUDE.md's epistemic discipline forbids, and the codebase already has precedent for fixing it (units.js:268-276 documents relabeling the Scale-2 "K" temperature to "(sim)" for exactly this reason).

### [MEDIUM] Per-particle |v| column is labeled "(c)" but is not normalized by c, contradicting the inspector and the Max |v|/c row by a factor of √3

File: engine\web\js\pe-telemetry.js:330

`_updateParticleTable` computes `vMag = Math.sqrt(vx*vx+vy*vy+vz*vz)` from raw `ext.velocities` (lu/tick) and writes it straight to the cell: `cells[4].textContent = fmtShort(vMag)` (lines 320, 330). The column header is `|v| <span class="unit-hint">(c)</span>` (panel-resources/diagnostics-template.js:27). Two other surfaces for the same quantity DO divide by c: `formatVelocity` uses `value / C_LATTICE` (units.js:236, used by the inspector Speed field at inspector/scales/particles.js:74) and `peMaxBeta.setLast(Math.sqrt(maxV2) / C_SPEED)` (telemetry-hub.js:630).

Impact: For the same particle at the same instant, the Per-Particle table reads |v| = 0.30 "c" while the inspector Speed reads 0.52 c and the diagnostics `Max |v|/c` row reads 0.52 — a √3 ≈ 1.732 contradiction visible on one screen. Because C_SPEED = 1/√3 is the causal cap, a table value near 0.577 looks safely sub-luminal when the particle is actually pinned at the cap.

### [MEDIUM] Angular momentum is computed about the origin in diagnostics but about the center of mass in the System overlay — two different L on screen at once

File: engine\web\js\bridge\mock-particle-engine.js:594

`peGetDiagnostics` accumulates `lx += p.y * mvz - p.z * mvy` using absolute positions and lab-frame velocities (lines 594-596) — L about the origin. `computeSystemVectors` in the controller deliberately uses CoM-relative positions AND CoM-relative velocities, with a docstring explaining why ("Using velocities relative to the CoM makes L the intrinsic orbital-plane normal, independent of any bulk drift", controller.js:294-299, implementation at lines 321-332).

Impact: With the `System` overlay enabled, the magenta L axis drawn in the viewport and the `Angular Mom` vector / `|L|` chart in the diagnostics panel are different quantities whenever the system's center of mass is displaced from the origin or drifting — which is the normal case for the scattering scenarios. Neither surface labels which convention it uses.

### [LOW] Inspector "Coulomb |F|" is not gated on the Coulomb toggle, unlike the Net |F| shown beside it

File: engine\web\js\bridge\mock-particle-engine.js:742

In `peInspectParticle`, the net-force loop correctly gates each term (`const fc = state._pe.coulomb ? … : 0;`, line 726) but the nearest-neighbour Coulomb magnitude does not: `fCoulombNearest = Math.abs(COULOMB_K_FORCE * p.charge * nq.charge / (r2 + soft2))` (line 742) is computed unconditionally. The inspector renders it under the tooltip "The magnitude of the electrostatic Coulomb force acting on this particle" (panel-resources/template.js:246).

Impact: With Coulomb disabled — the default for `pe-micro-bh` (`coulomb: false`, scales/scale1/scenarios.js:118) — the inspector shows a non-zero "Coulomb |F|" for a force that is not being applied, directly above a "Net |F|" that correctly excludes it. The two adjacent fields silently use different toggle semantics.

### [LOW] Virial ratio silently reports exactly 0 when potential energy is zero, which reads as maximal violation of the 1.0 reference line

File: engine\web\js\telemetry-hub.js:474

`const virial = diag.virialRatio ?? (pe !== 0 ? (2 * ke / Math.abs(pe)) : 0);` (line 474; pe-telemetry.js:473 duplicates it). `diag.virialRatio` is never populated by `peGetDiagnostics`, so the fallback always runs. `pe` goes to exactly 0 whenever both the coulomb and gravity toggles are off, since both PE accumulations are toggle-gated (mock-particle-engine.js:602-605). The virial chart draws a dashed reference line at 1.0 (`this._tsVirial.setRefLine(1.0)`, pe-telemetry.js:275).

Impact: Unticking Coulomb makes the `Virial 2K/|U|` row and chart snap to 0 against a 1.0 equilibrium marker, which reads as a catastrophic dynamical result rather than "undefined — no potential". An undefined ratio is rendered as a specific, alarming measurement.

### [LOW] Each PE time-series canvas registers six hover listeners (pointer and mouse duplicated) that are never removed

File: engine\web\js\pe-telemetry.js:54

`TimeSeriesChart`'s constructor binds both the pointer and the legacy mouse family to the same handlers: `pointerenter/pointermove/pointerleave` plus `mouseenter/mousemove/mouseleave` (lines 54-59). Browsers fire both families for a mouse, so `_setHoverPoint` + `_renderTooltip` run twice per movement, and each `_setHoverPoint` call makes two synchronous `getBoundingClientRect()` calls (lines 80-81) — deliberately bypassing the `_rectCache` that `draw()` was given for exactly this reason (line 34). There is no `removeEventListener` and no `destroy()` on the class; `PETelemetryPanel.clear()` (line 490) does not detach them.

Impact: Four charts × six listeners × two forced layout reflows per mousemove is a measurable hover cost on the diagnostics panel, and the listeners hold the canvases and `ChartHoverTooltip` instances alive for the app lifetime with no teardown path if the panel is ever re-created.

