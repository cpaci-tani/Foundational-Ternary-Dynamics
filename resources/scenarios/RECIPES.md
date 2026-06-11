# Scale 0 Scenario Recipes

Curated walkthroughs for the Scale-0 browser engine. Each recipe names a canonical scenario (from `engine/web/js/config/scenarios.js`), suggests overlay toggles and parameter knobs, and tells you what to look for.

Run with: `python -m http.server 8080 -d engine/web` → open http://localhost:8080 → Scale 0.

## 1. First genesis — watching matter appear

**Scenario:** `flux-pulse` (default on load)

**Controls:**
- Keep defaults. `K_B = 0.511`, `G_N = 0.01`, `Damp = 0.0073`.
- Overlay panel → Topology column → enable `Φ potential`.

**Play:**
- Press `Space`.
- The flux wavefront spreads from the seed point.
- When the pulse amplitude crosses `K_GENESIS = 1.533`, voxels flip to `±1` — a particle pair manifests.

**Look for:**
- Rubber sheet (Φ) deforms as mass accumulates — a **gravitational well** appears at the particle's location.
- Flux streamlines (enable in Fields column) shoot outward like iron filings around a charge.

## 2. Bell violation — `S = 2√2` from two flips

**Scenario:** `entangled-pair`

**Controls:**
- Overlay panel → Phenomena → `Dual J`. You'll see J_L (warm) and J_R (cool) separate.
- Diagnostics panel → watch the `Bell S` row.

**Play:**
- Let the sim run for ~200 ticks.
- At the measurement step, `S` should approach **Tsirelson's bound ≈ 2.828**.

**Look for:**
- `S < 2` during initial chirality equilibration.
- `S → 2.83` once the singlet establishes — matches `2√2`.
- No "nonlocal" action — the correlation is baked in at pair-creation.

See: `docs/theory/03_derivations/DERIV_SINGLET_FROM_VOID_EVENT.md`.

## 3. Confinement string — quarks refuse to separate

**Scenario:** `particle-collision` with strong force enabled

**Controls:**
- Overlay panel → Forces → enable `Strong` + style `Flow`.
- Phenomena → enable `Confinement`.

**Play:**
- Two colored particles seed, heading apart.
- At separation ≈ `x₋ = 3.024` voxels, a **flux tube** snaps into existence between them.

**Look for:**
- Strong-force flow lines bunch into a narrow tube.
- Particle separation stalls — energy goes into extending the tube.
- If you push harder (increase kick in `Inject` → Particle), the string eventually breaks with a new pair at the break point.

## 4. Coulomb convergence demo

**Scenario:** `force-law-profile`

**Controls:**
- Overlay panel → Fields → `E field` + `flux lines`.
- Forces → `EM` with `Arrows` style.

**Play:**
- Single charge at origin.
- Enable scrub → render 10 s → scrub back and forth to see the field stabilize.

**Look for:**
- `|E| · r²` → constant as `r` grows (Coulomb `1/r²`).
- Engine-theory benchmark suite expects B+ grade on this (see `engine/tests/benchmark_engine_theory.cpp`).

## 5. Hydrogen spectrum — energy-level hunt

**Scenario:** `hydrogen-atom`

**Controls:**
- Keep defaults.
- Overlay panel → Quantum → enable `|ψ|²`.

**Play:**
- Let the sim relax for ~500 ticks.
- Charts panel → "Radial probability density" chart.

**Look for:**
- `|ψ|²` cloud organizes into shells.
- Energy eigenvalues in Diagnostics match `E_n = −13.6/n² eV` (A+ benchmark).

## 6. Topology overlay tour

**Scenario:** anything with structure — try `flux-vortex` or `dipole-radiation`.

**Controls:**
- Overlay panel → Topology column → turn on all four rubber sheets:
  - `Φ potential` (bottom)
  - `EM energy u`
  - `Charge ρ`
  - `Vorticity ω` (top)

**Play:**
- Start sim.
- Watch the four stacked sheets respond to different aspects of the same flux field.

**Look for:**
- Φ dips under mass; charge ρ has red peaks at sources, blue wells at sinks; vorticity lights up where flux swirls; EM energy peaks where fields concentrate.
- All four go **flat in stillness** — reset the scenario and watch them smooth out.

## 7. Scrub-playback workflow

1. Start any scenario. Press `Space`.
2. Let it run for 10 seconds of sim time.
3. Drag the scrub thumb backwards → the canvas re-shows earlier states (hydrated from the memory recorder).
4. Release near "now" → sim resumes from there.
5. Click the  gear in the scrub bar → pick **60s** duration.
6. Click the Render button → the canvas fast-forwards through 60 seconds of sim; the progress chip tracks it.
7. After "Render complete", scrub the newly-populated green band in the strip — that's the rendered future at 15 fps.

## 8. LHC Standard Model tour (added 2026-04-17)

The Scale-0 dropdown has three SM-themed groups: **SM Bosons**, **SM Quarks**, **SM Processes**. Each scenario is epistemically tagged — open the scenario description panel to see what's derived from framework integers vs what's visualization.

### 8a · Watch a Higgs boson sit in its vacuum

1. Load **`Higgs field vacuum (VEV background)`** → see the uniform flux sea; `Space` to play.
2. Switch to **`Higgs boson (H, m≈125 GeV)`** → see a localised scalar lump against the same background.
3. Overlay: enable `|ψ|²` (Quantum column) to visualise the localisation.

### 8b · Electroweak bosons side-by-side

Load W, Z, and gluon one after another. Look for:
- **W±**: chirality-biased flux (+30% along the dominant axis) + manifested charge at centre.
- **Z⁰**: no manifested core (neutral), bound radial-inward flux configuration.
- **Gluon**: massless transverse wave like the photon but `J_y` polarised + color-axis dominant.

Overlay `E field` + `B field` (Fields column) to see the transverse structure of the gluon.

### 8c · Quark-flavour catalog

Six scenarios in the **SM Quarks** group produce individual up, down, strange, charm, bottom, top quarks. Amplitude scaling (1.0 → 5.0× baseline) suggests generation hierarchy. Color labels rotate R/G/B across the doublets so the `Chirality` and `Confinement` overlays distinguish them.

**Honest note:** FTD does NOT derive individual quark masses. The amplitude scaling is a visualization cue. See the scenario description panel for the full epistemic breakdown.

### 8d · Beta decay — dynamic weak transmutation

Load **`Beta decay (n → p + e⁻ + ν̄, dynamic)`**. The scenario:
- Seeds a 3-vertex neutron-ish triangle with mixed charges.
- Pre-injects an electron and a neutrino offset along +z as the leptonic output.
- Enables `weak_transmutation` and `dual_substrate` so the engine can flip polarities under field stress.

Let it run for ~100 ticks. Watch for one of the negative vertices polarity-flipping when the stress at its site crosses `WEAK_THRESHOLD = K_GENESIS`. That flip is the FTD analogue of the neutron → proton transition — **real dynamics, not animation**.

### 8e · e⁺ e⁻ annihilation

Load **`e⁺e⁻ annihilation (collision → flux burst)`**. Two leptons are seeded on opposite faces with opposing velocities (±0.3 C_SPEED). They approach each other over ~20 ticks and collide at the centre.

What happens: `phase_movement`'s collision logic recognises opposite-sign contact → both annihilate, their flux bursts into the 6 face-neighbours. The resulting two opposing radial wavefronts visually resemble the `γγ` final state.

Overlay `EM energy u` (Topology column) to see the energy flow into the photonic field after annihilation.

## 9. Parameter play — reference frame context-phase exploration

**Scenario:** switch scale to **Scale 11 (Reference frame context)**.

**Controls:**
- Reference frame context panel → `θ_C` phase slider.
- Watch `cos²θ_C = G*/8 ≈ 0.370` as the default observable/subjective split.

**Look for:**
- The holographic figure rotates as the reference frame context phase rotates.
- The sLoop fixed point at `C = 1/G* ≈ 0.338` is the Mandelbrot-set attractor for the self-reference ring.

## Quick-toggle crib sheet

| Overlay | Best scenarios to pair with |
|---|---|
| `Φ potential` | flux-pulse, gravity-cluster, proton-electron |
| `EM energy u` | dipole, dipole-radiation, two-slit |
| `Charge ρ` | proton-electron, 4-source-interference, pair-production |
| `Vorticity ω` | flux-vortex, photon-race, genesis-cascade |
| `Confinement` | particle-collision, pair-production |
| `Dual J` | entangled-pair, dual-substrate, annihilation |
| `Chirality` | dual-substrate, weak force demos |

## Making your own

Copy `templates/SCENARIO_TEMPLATE.md` in this resources tree, fill it out, register the scenario in `engine/web/js/config/scenarios.js`. The template walks you through each required hook.
