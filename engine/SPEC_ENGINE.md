# FTD Simulation Engine Reference

**Living document for AI agents and developers.**
**Engine version:** 2.18.0 (single-sourced as `ftd::ENGINE_VERSION` in `include/ftd/constants.h`; mirrored by CMake `project(VERSION)`, `ftd_sim --version`, and the WASM `getEngineVersion()` binding — revision 6.1)
**Golden regression pins:** `GOLDEN_HASH=0xc54ffbeda5a3ea63`, `GOLDEN_STATE_HASH=0xe9633be07656e741`, and `GOLDEN_AUDIT_HASH=0x48bd8b3fc2efdba3` for the frozen L=17 profile in `test_render_bridge_golden`. These pins cover only that profile's folded fields; they do not cover off-profile toggles, larger lattices, or horizons beyond its 100 ticks. Rationale and pin history live in `test_render_bridge_golden.cpp`.
**Test surface:** C++ tests, Playwright specs, and Python-adjacent verification helpers are registered through CMake and the web test harness. CTest uses the `unit`/`physics`/`golden`/`slow`/`gpu` label scheme; CUDA targets are conditional on `FTD_ENABLE_CUDA`.

## 0. System Narrative: From Field Capacity to Manifested Events

Scale 0 is the engine's discrete substrate. It does not begin with continuous
space plus particles. It begins with a finite cubic lattice, local update loops,
and two coupled voxel layers:

| Layer | Engine fields | Interpretation in the simulation |
|---|---|---|
| Discrete state | `state in {-1, 0, +1}` | Void, negative manifestation, positive manifestation |
| Flux field | `flux`, `wave_vel` | Dispositional vector field and its staggered wave velocity |
| Kinematics | `velocity`, `remainder` | Sub-lattice motion registers for manifested sites |
| Labels | `particle_id`, `pair_id`, `spin`, `color`, `flavor`, `locked` | Identity, event-pair tracking, internal labels, bound-state locks |
| Optional sectors | `flux_L/R`, `wave_vel_L/R`, `flux_strong`, `wave_vel_strong`, `flux_weak`, `wave_vel_weak`, `latency`, `tau`, `phase` | Dual/strong/weak substrate and latency/proper-time/clock extensions |

The continuous-looking physics in the dashboard and diagnostics is an emergent
large-scale behavior of repeated local steps. Each tick stages the work so that
parallel field loops read stable snapshots, while collision and lifecycle events
mutate state only after the relevant local information has been collected.

### 0.1 Runtime Loop Families

The Scale 0 engine is best understood as a composition of loops:

| Loop family | Main implementation | Mutability discipline | Role |
|---|---|---|---|
| Field read | `phase_read.cpp` | Read voxel snapshot, write delta buffers | 18-point Moore Laplacian, state-flux coupling, curl source |
| Field write | `phase_write.cpp` | Parallel voxel mutation | Staggered wave commit, damping/noise, genesis, evaporation |
| Constraint | `poisson_solvers.cpp` | Project flux | Gauss, Coulomb, and latency Poisson solves |
| Force | `phase_forces.cpp` | Update force diagnostics and velocities | Field-mediated force integration with bandwidth-limited velocity |
| Movement | `phase_movement.cpp` | Sequential guarded mutation | Integer moves, bounces, annihilation, self-field transfer |
| Lifecycle extensions | `transmutation_phases.cpp` | Toggle-gated mutation | Pair production, weak transmutation, triad binding, proper time |

This "loop dynamics" language refers to the engine execution loops and local
lattice dynamics. Theory-side perturbative loop-coefficient derivations live in
the theory/proof corpus and are not runtime kernels in Scale 0.

### 0.2 Manifestation Lifecycle

Manifestation is the state transition from high latent flux in a void cell to
an actual ternary site. The main path lives in `phase_write()`:

```
void site
  + flux density above K_GENESIS
  + deterministic stochastic draw below p
  -> state = +1 or -1
  -> spin/color inferred from local field geometry
  -> pending particle_id assigned
  -> deterministic ID resolution in voxel-index order
```

After manifestation, later phases can project the constraint, apply forces,
move the particle, bounce it, annihilate it with an opposite sign, evaporate it
back to void, flip it through weak transmutation, create correlated pairs, or
lock compact triads when the relevant toggles are enabled.

### 0.3 CPU/GPU Parity Model

The CPU path keeps `std::vector<Voxel>` as the authoritative array-of-structures
state. The CUDA path mirrors those fields into structure-of-arrays device
buffers. Host mutations are flushed to the GPU before a device tick; host reads
download the device state lazily. The paths share a broad phase order, but
CPU-scoped fallbacks and the discrete movement-conflict divergence can change
which terms run and which particle wins a contended site. See the §14
discrete-outcome/missing-term table.
Separately, supported Poisson paths use different numerical solvers (CPU
warm-started SOR versus CUDA spectral/FFT machinery).

---

## 1. Architecture: Logic-First Engine (v2.0)

The engine was rewritten from ~1382 lines of phenomenological code to a logic-first design. Only behaviors derivable from the axioms {3D lattice, ternary states, flux field, local causality, action principle} remain. Everything else was archived to `archive/engine_v1_phenomenological/`.

**Core rules (default Scale 0 substrate):**

1. **Flux wave equation**: dJ/dt = c^2 nabla^2 J (only possible local linear dynamics for a vector field)
2. **State-flux coupling**: source term `-g_c*grad(s)+g_c*curl(s*v)`.
   FTD-0574 derives it for a prescribed source from
   `I_source=g_c<s,div J>+g_c<curl J,s*v>`. The second term is not the
   J-variation of the diagnostic onsite interaction `-g_c<s*v,J>` and does
   not establish a common dynamic matter/field action. Electric sign per
   `lagrangian.h` Term 2, amended 2026-07-18 — the pre-amendment
   `+g_c*grad(s)` fought the Gauss constraint at charge sites, measured live
   equilibrium `f=-0.095`.
   FTD-0575 derives the source action's reciprocal Hodge force using
   `Phi_J=-g_c*div(J)` and `A_J=g_c*curl(J)`. Its Lorentz-form algebra is exact,
   but its static kernel is bounded (`0<=R(k)<=3`): the massless pole cancels,
   equal polarities attract, and the soft residue vanishes as `O(k^2)`. This
   channel is therefore not native Coulomb electromagnetism. Gauss projection
   and Poisson Coulomb remain separately selected mechanisms.
   FTD-0576 further derives the exact driven-tick work coordinate
   `R=J-wave_vel/2` and a conditional continuity-based total-energy identity.
   A cardinal one-site hop cannot supply its required central current locally:
   the symbol is `-2z/(z+1)`. The exact face current cannot be mapped into the
   native site current by a finite-range commuting projection either. Mobile
   closure therefore requires a separate matched face dynamics, a staggered
   primitive, nonlocality, or a different nonlinear carrier.
   FTD-0577 constructs the minimal separable nonlinear-carrier sidecar: the
   normalized symmetric coat `B=(T^-1+2+T)/4` on each axis. Its tensor product
   is a positive 27-site Moore coupling representation, and
   `Q_i=((1+T_i^-1)/2) product_(j!=i) B_j K_i` maps the exact face current to
   exact local central continuity. Primitive `s` remains one ternary site, but
   coupling is explicitly non-cardinal. This observer result does not select
   a production force or repair the FTD-0575 static pole/sign obstruction.
   FTD-0578 gives this coat an exact temporal split and one common action in
   the native work coordinate `R=J-wave_vel/2`; source deposition and the
   orbit-side gather are adjoints of that action. The unmodified compact
   carrier nevertheless fails as free mobile matter: edge/body diagonal
   time averages disagree with the FTD-0576 energy-centered source by exact
   squared norms `1/1536` and `5/3072`, and its reciprocal self-field produces
   a positive, polarity-even Peierls potential `C_i r(1-r)`. No production
   action, force, or movement phase implements this observer.
   FTD-0579 closes finite rigid smearing as an exact repair. For a nonzero
   finite carrier symbol `A`, the diagonal defect factorizes as `B_M A M_d`
   and cannot vanish in the Laurent integral domain. Its Peierls coefficient
   is also strictly positive. Smooth binomial envelopes suppress both defects
   only as `O(R_rms^-2)` relative to the carrier scale; the observer does not
   derive such an envelope from production.
   FTD-0580 resolves the diagonal-centering branch by replacing the trilinear
   diagonal coupling sidecar with the positive endpoint chord
   `p_t=(1-t)delta_0+t delta_d`. A democratic average over all shortest face
   routes gives exact local and central continuity without choosing an axis
   order, and its time-exact action is endpoint-energy-centered. The same
   finite chord still has a positive Peierls barrier, so it is not a gapless
   production particle.
   FTD-0581 derives the corresponding production-dispersion depinning
   threshold and closes stable passive dressing as its cure. The Hodge field
   plus source completes to the relaxed Peierls curve plus a nonnegative
   quadratic deformation energy, while any locally Lipschitz response of a
   stable dressing begins at `O(r^2)` and cannot cancel the curve's linear
   integer-site cusp. A zero-momentum active traversal candidate must carry
   at least `C_d/4` finite internal excitation and derive a recurrent
   phase-resolved exchange from the frozen action; that dynamics is open and
   is not implemented here.
   FTD-0582 then audits that remaining possibility against the actual tick.
   With selected forces disabled, `phase_read`/`phase_write` evolve `(J,W)`
   but never write manifested `velocity` or `remainder`; ordinary movement
   only integrates the velocity it was already given. In 144 energetic
   phase/direction/polarity/volume arms every field evolved while matter
   response stayed bit-exact zero. The current flux dressing may follow and
   trail prescribed motion, but it is not a reciprocal mover in production.
   A future common-action branch would be new selected dynamics, not an
   observer-only promotion of the frozen tick.
   FTD-0583 then classifies the existing matched real face/edge sidecar before
   treating topology as an escape. The periodic complex has
   `H^2(T_L^3;R)=R^3`, represented only by three global continuously-valued
   plane fluxes. Every localized divergence-free zero-harmonic face field
   contracts continuously to vacuum with quadratic energy, and real periodic
   Gauss dipoles scale continuously. Thus the current noncompact fields do
   not supply a localized protected bundle/defect carrier or compact-`U(1)`
   charge quantization. A nonlinear deforming core or a genuinely new
   compact/singular variable remains a separate ontology choice. Production
   is unchanged and no FTD-0481 toggle/scenario is licensed.
   FTD-0584 strengthens that result after the source is fixed: Gauss and
   harmonic constraints define an affine fibre of the ordinary real field
   arrays, so every nonempty fibre contracts explicitly to a base solution.
   The same holds for finite-support and finite-energy fields on an uncontained
   lattice. The ternary alphabet labels disconnected snapshots, but production
   events connect those labels and the registered additive transition basis has
   rank four/nullity zero. The free vacuum is the point `s=0,J=W=0`, so it has
   no wall, vortex, hedgehog, or texture homotopy. A static same-variable core
   with only onsite and two-derivative energy shrinks by Derrick scaling; a
   time-periodic active core or an explicit compact/constrained order parameter
   remains open and would be selected new dynamics/ontology. Production and
   scenarios remain unchanged.
   FTD-0585 then separates moving support from transported matter. The exact
   history identity is `Delta M=sum_faces I+sum_x x S`: the same endpoint
   snapshots can be produced by a face current or by balanced local death and
   rebirth. Reaction-free rest arms remain exactly static, while initialized
   ballistic controls hop. The source audit and live CPU replay also expose a
   production confound: evaporation clears `s` and visible labels but preserves
   `velocity`/`remainder`; genesis can remanifest those hidden values exactly.
   Reaction-front research must sanitize this memory and carry an explicit
   source/reservoir ledger. No production fix, toggle, scenario, or particle
   promotion is made by the observer.
   FTD-0586 performs the first sanitized causal-source gate. For the
   single-substrate wave/coupling/genesis sector with zero initial field and
   kinematics, the exact modal step response has a finite pointwise envelope;
   a source removed once by evaporation costs at most twice that envelope.
   On `L={9,17,33,65}`, any arrangement/signs of at most three sources obeys
   `|J|<=1.1598848941400712<K_GENESIS`. A first-event induction therefore
   forbids endogenous genesis in that class. Ninety-six live arms conform,
   while four external supercritical controls fire. In FTD-0586 alone, `N=4`
   was unresolved rather than sufficient; FTD-0588/0589 supersede that count
   boundary below. Gauss projection, dual genesis, and external packets remain
   outside the theorem. Production and scenarios are unchanged.
   FTD-0587 then replays the externally ignited FTD-0474 dispersal tail and
   separates inherited field, native causal coupling, and selected Gauss
   support. Every continuation without repeated Gauss projection passes
   `0/24`; cleared Gauss passes `18/24` but only four of six registered cells;
   intact field plus repeated Gauss alone reproduces `20/24` and five cells.
   The qualified tail records zero genesis and 204 evaporations. It is an
   externally prepared Gauss-stabilized evaporative remnant, not a native
   self-sustaining reaction front. The projector remains selected and no
   movement, common action, toggle, or scenario is promoted.
   FTD-0588 removes the apparent four-source opening left by FTD-0586's
   sourcewise triangle bound. The exact production symbols satisfy
   `sum_a sin(k_a)^2 < M(k)` off the zero mode, and distinct source characters
   satisfy `sum_k |S(k)|^2=L^3 N`. Common source histories therefore obey a
   `sqrt(N)` pointwise envelope. On `L={9,17,33,65}`, all common histories
   through `N=5` and all independently removed histories through `N=4` are
   theorem-subcritical. For asynchronous `N=5`, genesis is excluded until the
   final original source disappears; FTD-0588 itself left the all-off residual
   field open. All 128 registered live arms record zero genesis, and all 64
   unlocked arms reach complete source removal.
   FTD-0589 closes that residual tail exactly. For a removal at tick `T`, the
   finite-pulse response is `2 sec(theta/2) sin(T theta/2)
   sin((n-(T-1)/2)theta)`, so the constant step pieces cancel. The combined
   history envelope `C_L sqrt(N-r)+rP_L` is subcritical for every arbitrary
   one-time-removal history through `N=6` on the registered volumes. Its 96
   observer arms/12,288 ticks record zero genesis and 176 native evaporations.
   FTD-0590 then closes the former `N=7` boundary without selecting a source
   geometry or removal schedule. The normalized pulse factor is constant on
   signed-permutation mode orbits, giving the exact relaxation
   `C_L sqrt(N-r)+Q_L sqrt(r+mu_L r(r-1))`. Exhaustive mode/displacement-orbit
   norms give `mu_L=0.3610...0.36274` and worst seven-source bound
   `1.2142763824<K_GENESIS`. FTD-0591 and FTD-0592 evaluate the unchanged
   bound at `N=8` and `N=9`: all 40 nine-source removal partitions are
   subcritical, with worst value `1.4801131738` at `L=65,r=8`. FTD-0593
   evaluates `N=10`; its unchanged orbit bound is inconclusive on every
   volume, with worst upper bound `1.6127738812` at `L=65,r=9`. This is not a
   witness history. FTD-0594 performs exact shared-`M` grouping with integer
   cyclotomic keys; at `L=65` all 6,544 cubic orbits are singleton shells, so
   the decisive bound is unchanged. FTD-0595 then enforces the exact axial-pair
   capacity obtained from all connected cubic animals through size nine.
   Although at most 13 of the 36 `r=9` pairs can be axial, the complementary
   shell coherence is nearly as large; all four refined bounds remain above
   threshold, with worst value `1.6115888534` at `L=65`. This is inconclusive,
   not a witness history. FTD-0596 retains the complete cubic-orbit distance
   distribution and imposes every autocorrelation Fourier-positivity
   constraint. Its 32 padded dual certificates shift the worst partition to
   `r=8` and lower the four maxima to `1.5218539833...1.5932999259`; every
   value remains above threshold. The Delsarte relaxation is therefore also
   inconclusive and supplies no configuration or history. FTD-0597 then uses
   the exact same-time identity `-1/4<=u_i u_j<=1` to retain signed exact-shell
   cancellation before applying the same Delsarte polytope. All four maxima
   occur at `r=8` and are subcritical; the worst is `1.4577559408` at `L=65`,
   margin `0.0586301184`. First-event induction therefore closes arbitrary
   one-time-removal histories through `N=10` in this frozen sector. These remain
   observer theorems/numerically certified facts; production, toggles, and
   scenarios are unchanged.
3. **Gauss projection**: selected constraint step targeting `div(J) = s` each
   tick. It realizes a polarity-to-Gauss-charge map; it is not a proof of
   charge conservation or a logical consequence of the five postulates
   (FTD-0421/0426).
4. **Manifestation/Evaporation**: |J| > K_GENESIS -> manifest; stochastic evaporation with per-tick probability p = exp(-E_local/K_MANIFEST^2) * K_EVAP_RATE * (dtau/dt), where E_local is the 7-site energy (particle + 6 face-neighbors; locked voxels exempt) and dtau/dt = sqrt(max(1-B,0)), B=|u|^2/C_SPEED^2+L^2, is the shared selected proper-time rate (`ftd/causal_kinematics.h`; FTD-0402). Stored `u` is raw nodes/tick. This implements the clock/bandwidth axiom; it is not a covariance theorem.
5. **Field-mediated forces**: F = -alpha * s * grad(phi_C) + G_N * grad(rho) + alpha * s * (v x B) where B = curl(J) (Poisson Coulomb + Lorentz magnetic + gravity)
6. **Movement + Collision**: remainder accumulation, speed limit C_SPEED = C_WAVE = 1/sqrt(3), annihilation on contact

These are six **conceptual core rule families**, not a count of constructor
defaults. The shipping Scale-0 profile is the 13 `TermToggles` member
initializers set to true: `wave_propagation`, `coupling`, `damping`, `genesis`,
`gauss_projection`, `forces`, `gravity`, `poisson_coulomb`, `movement`,
`lorentz_force`, `selective_damping`, `dual_substrate`, and
`weak_transmutation`. The first ten implement or select paths within the six
families; the final three are promoted extensions.

**What was removed from the default core** (archived in `archive/engine_v1_phenomenological/`):
- Pairwise Coulomb, Yukawa, exchange, Lorentz forces
- QCD running coupling, color Yukawa
- Noetic/reference frame context coupling in the Scale 0 runtime
- Earlier always-on phenomenological latency/bandwidth/proper-time machinery

**Toggle-gated extensions** (default OFF, for pedagogy and exploration):
- Larmor radiation: acceleration-dependent damping (v2.11)
- Color forces, strong force, triad binding, pair production, exchange force
- Latency field and proper-time accumulation when `latency_field` is enabled

Three extension toggles are *promoted to default ON* and run in the shipping
tick: `dual_substrate` (J_L + J_R chirality),
`selective_damping`, and `weak_transmutation` (stress-gated polarity flip).
`weak_transmutation` is a third J↔s coupling not named by the two-channel
ontology (FTD-0257); whether it should remain default-on is an open governance
question, not a settled rule.

### Scale 5: Cosmic Engine (v2.12)

N-body + SPH cosmic simulation with Barnes-Hut octree gravity. Its configured
constants mix engine parameters, imposed calibrations, and selected or
parametric theory-side values; using them in the implementation does not make
them derived or parameter-free. Consult the LEDGER and this specification's
active/reference constant notes for each value's status:
- **9 body types**: Dark matter, gas, stars, neutron stars, black holes, quasars, nebulae, white dwarfs, dark energy field
- **18-phase cosmic tick cycle**: octree build, gravity, SPH density/forces, Friedmann expansion, dark energy, accretion, jets, star formation, stellar evolution, magnetic fields, radiation pressure, gravitational waves, Verlet integration
- **14 toggles**: gravity, sph_gas, hubble_expansion (core ON); dark_energy, dark_matter_halos, black_hole_accretion, cosmic_radiation, star_formation, stellar_evolution, galaxy_mergers, magnetic_fields, radiation_pressure, relativistic_jets, gravitational_waves (extensions OFF)
- **FTD constants**: G_N=0.01, Omega_Lambda=2/3, DM_frac=17/27, gamma=5/3, c=1/sqrt(3)

### Abstract Base Class: ScaleEngine (v2.12)

All scale engines (ParticleEngine, CosmicEngine) inherit from `ScaleEngine`, providing:
- Unified `tick()`, `run()`, `current_tick()`, `dt()`, `set_dt()` interface
- String-based `get_toggle(name)` / `set_toggle(name, value)` for unified registry
- `base_diagnostics()` returning common metrics across all scales
- `scale_level()` and `scale_name()` for runtime type identification

### Scaling and Performance Constraints

**Lattice Engine (Scale 0)**
Forces are $O(N)$ field-mediated (single loop over manifested particles summing their interactions with the local lattice neighborhood) instead of $O(N^2)$ explicit pairwise. Inherently faster for large particle counts processing raw flux.

**Macro Engines (Scales 1, 2, 5)**
The `ParticleEngine`, `AtomEngine`, and `CosmicEngine` all rely on a dynamically re-calculated **Barnes-Hut Octree** (see `barnes_hut.h`) to approximate macroscopic limits of long-range $1/r^2$ isotropic potentials (e.g. Gravity and Coulomb).
- Achieves $\mathcal{O}(N \log N)$ computation scaling by terminating monopole traversals at a critical opening angle threshold ($\theta < 0.5$).
- `AtomEngine`'s discrete covalent interactions traverse a fully pre-separated $O(N)$ topographical linked-list ensuring that discrete bounds like `Angle Strain` do not invoke continuous $O(N^2)$ matrices.
---

## 2. Repository Inventory

This is an architecture map, not a frozen file listing. Extracted modules, test
registrations, and line totals change frequently; use the generated inventory
artifacts below for current detail.

```
engine/
  CMakeLists.txt             # Build graph and target registration
  include/ftd/               # Public engine types, contracts, and constants
  src/                       # CPU implementation, phases, scales, and scenarios
  cuda/                      # CUDA backends, kernels, and device utilities
  wasm/                      # Emscripten bindings
  config/                    # Runtime toggle and scenario data
  tests/                     # Native unit, parity, benchmark, and campaign sources
  web/                       # Browser application, bridge layer, assets, and web tests
  tools/print_ontic.py       # Ontic-chain inspection utility
  docs/                      # Generated inventory plus maintained architecture maps
  archive/README.md          # Provenance index for retired engine artifacts
```

Canonical navigation:

- `docs/ENGINE_FILE_MANIFEST.json` is the machine-readable current tracked
  code-file inventory; `docs/ENGINE_FILE_MANIFEST.md` is its human-readable
  rendering.
- `docs/ENGINE_CODE_MAP.md` maps the major subsystems and extracted modules.
- `CALLSTACKS.md` traces feature-level runtime paths through the engine.
- `web/js/bridge/wasm-bridge.js` is the browser WASM/mock bridge implementation.

Historical components remain preserved through `archive/README.md` and the
archive paths it indexes. They are provenance, not part of the active
architecture, and are intentionally not expanded into a second manifest here.

---

## 3. Build and Run

### Tests only

```bash
# Canonical Windows-native build -- MUST use the MSVC 14.44 toolset:
# VS 18's default toolset (14.51+) crashes CUDA 13.0's cudafe++ on every .cu.
# The wrapper locates VS via vswhere, enters vcvarsall x64 -vcvars_ver=14.44,
# and drives engine/CMakePresets.json (Ninja Multi-Config -> engine/build):
engine\build_native.bat
# Raw cmake equivalent (ONLY inside a vcvars 14.44 shell):
#   cmake --preset native              (run in engine/)
#   cmake --build --preset native-release
ctest --test-dir engine/build -j 24 --output-on-failure -C Release
```

### WASM build (browser dashboard)

```bash
# Requires Emscripten SDK installed
emcmake cmake -S engine -B engine/build_wasm -DCMAKE_BUILD_TYPE=Release
emmake cmake --build engine/build_wasm --target ftd_wasm
# Outputs: engine/build_wasm/wasm/ftd_core.js + ftd_core.wasm
# Copy to: engine/web/wasm/
cp engine/build_wasm/wasm/ftd_core.{js,wasm} engine/web/wasm/
# Serve:
python engine/web/serve.py 8080
# Open: http://localhost:8080
```

### CLI simulation
```bash
./engine/build/Release/ftd_sim.exe [scenario] [lattice_size] [num_ticks]
```
Scenarios: `A` (Coulomb electron-proton), `B` (pair production from flux), `D` (locked particle stability), `E` (helium atom), `F` (gravitational cluster), `G` (scale stress test), `H`/`I`/`J` (CSV export variants), `K` (force law profile).

---

## 4. The Tick Cycle

Each call to `RenderBridge::tick()` validates the active `TermToggles`, syncs
the ternary mirrors if needed, then either delegates to the GPU backend or runs
the CPU phase ladder. Not every phase is enabled in every run; the bracketed
toggle tells where a phase enters.

```
RenderBridge::tick()
  0.   normalize/validate toggles; select CPU backend where required
  0b.  sync_ternary_from_voxels_if_needed()
  1.   apply EW background drive      [ew_background_sweep; before read]
  1a.  solve_coulomb_poisson()        [db_clock_coulomb; before read]
  1b.  phase_read()                   [wave_propagation || coupling || de_broglie_clock]
  2.   phase_write()                  [skipped by matched_gauss_dynamics]
  2a.  phase_read() + second half-kick [verlet_wave_integrator]
  2b.  pair_production_cpu()          [pair_production]
  3.   gauss_project()                [gauss_projection]
  3a.  begin_strong_energy_step()     [strong_stress_energy]
  3b.  solve_latency_poisson()        [latency_field]
  4.   phase_forces()                 [forces]
  4b.  snapshot matched state         [matched_gauss_dynamics]
  5.   phase_movement()               [movement; reflective_boundary controls face exits]
  5a.  extract current + advance/sync [matched_gauss_dynamics; after movement]
  5b.  complete_strong_energy_step()  [strong_stress_energy && movement]
  5c.  absorbing/flux boundary passes [absorbing_boundary / flux_boundary]
  6.   weak_transmutation_cpu()       [weak_transmutation]
  7.   triad_binding_cpu()            [triad_binding]
  7b.  relax_su2/su3_links_cpu()      [su2_gauge / su3_gauge; links only]
  8.   accumulate proper time/phase   [latency_field || de_broglie_clock]
  9.   physical_time_ += dt_; ++tick_
  10.  record settled knot telemetry  [knot_tracking]
  11.  sync/dirty flags/energy ledger updates
```

### 4.1 CPU phase details

| Phase | Main toggles | What it does |
|-------|--------------|-------------|
| EW/pre-read setup | `ew_background_sweep`, `db_clock_coulomb` | The optional uniform EW drive is injected before the field read. The Coulomb-clock diagnostic similarly pre-solves its live Coulomb potential before the read. |
| `phase_read` | `wave_propagation`, `coupling`, `de_broglie_clock` | Runs when any listed term is enabled. The parallel read-only voxel loop computes `delta_J` from the 18-point Moore Laplacian, `-G_C * grad(state)`, curl source, and the optional clock term. Dual-substrate mode computes separate L/R deltas and recombines observables after write. |
| `phase_write` / Verlet completion | `matched_gauss_dynamics`, `damping`, `genesis`, `evaporation`, `selective_damping`, `larmor_radiation`, `langevin`, `symplectic_leapfrog`, `verlet_wave_integrator` | The matched-Gauss branch skips this legacy writer. Otherwise the parallel commit loop advances the field, applies damping/noise, and performs genesis/evaporation. Verlet mode then re-runs `phase_read` on the post-drift field and applies its second half-kick before pair production and Gauss projection. |
| `pair_production_cpu` | `pair_production` | Creates neighboring correlated `-1/+1` pairs from high-flux voids, consumes local wave/flux energy, assigns shared pair IDs, and conserves charge locally. |
| `gauss_project` | `gauss_projection`, `exact_dual_gauss` | Builds `source = div(J) - coulomb_charge_coupling * state`, solves a warm-started SOR Poisson problem, then subtracts `grad(phi)` from flux. Ordinary mode skips manifested sites during correction; exact dual mode synchronizes the split L/R fields. |
| `solve_latency_poisson` | `latency_field`, `field_energy_gravity` | Builds a mass/field-energy source, solves a latency potential, and stores bounded `latency` values for time dilation and bandwidth accounting. |
| `phase_forces` | `forces`, `poisson_coulomb`, `emergent_forces`, `gravity`, `lorentz_force`, `color_forces`, `strong_force`, `exchange_force`, `cluster_inertia` | Iterates manifested sites. EM/gravity/Lorentz stay gated on `forces`. Colour, Yukawa (`strong_force`), and exchange share host/device helpers with the CUDA kernels. Writes `ForceDiag` and integrates velocity with the `gamma_FTD` bandwidth budget; `cluster_inertia` then integrates locked Moore clusters, including exchange in `F_cluster`. |
| `phase_movement` | `movement`, `symmetric_movement_order`, `reflective_boundary` | Sequential guarded mutation. Accumulates sub-lattice remainders, moves into void targets, bounces same-sign collisions, annihilates opposite signs, carries self-field to moved particles, and bursts field energy on annihilation. An attempted lattice-face crossing mirror-bounces the crossed velocity components and clears the remainder when `reflective_boundary` is on; when it is off, the particle and its local fields/labels are cleared so they exhaust into the void rather than wrapping periodically. |
| matched-Gauss current/advance | `matched_gauss_dynamics` | Snapshots ternary state before movement, extracts the conservative routed current from the post-movement difference, advances the oriented face/edge state, and synchronizes it back to voxels. |
| `apply_absorbing_boundary` | `absorbing_boundary` | Applies an imposed D-deep quadratic damping sponge after movement. It is not a derived reflection-free radiation condition. |
| flux-boundary pass | `flux_boundary` | `Periodic` leaves the toroidal wave map unchanged; `Reflective` copies an interior Neumann ghost shell; `Dispersal` multiplies the outer shell by `1-C_SPEED`. These are computational finite-box laws, not ontological boundaries. |
| `weak_transmutation_cpu` | `weak_transmutation` | Stress-threshold stochastic polarity flips. In dual-substrate mode the L/R fluxes are swapped with the flip. |
| `triad_binding_cpu` | `triad_binding` | Detects compact same-sign triples and locks them as bound structures. |
| `relax_su2/su3_links_cpu` | `su2_gauge`, `su3_gauge` | One Jacobi double-buffered Wilson staple sweep per tick over the SU(2)/SU(3) edge links ([IMPOSED] lattice-gauge import; see §8.1). Write-only w.r.t. the substrate — links feed nothing downstream. Buffers lazily allocated on first use. |
| `accumulate_proper_time` | `latency_field`, `de_broglie_clock` | Updates `tau` for manifested sites using the latency/speed bandwidth factor and advances the optional de Broglie phase. |
| settled telemetry | `knot_tracking` | Records observation-only knot telemetry after physical time and the tick counter advance, so it reads settled state. |

### 4.2 GPU phase ladder

When a CUDA backend is active, `RenderBridge::tick()` delegates to
`backend_->tick()`. `GpuBackend` flushes host mutations to device buffers, copies
the current toggles into `GpuEngine`, runs the device tick, and marks the host
shadow dirty until diagnostics or voxel access require a download.

For terms implemented on-device, `GpuEngine::record_tick_body()` uses this
order:

```
launch_ew_background_sweep() [ew_background_sweep; before phase_read]
gpu_phase_read()/write()     [skipped by matched_gauss_dynamics]
gpu_phase_read() + launch_verlet_second_half_kick()  [verlet_wave_integrator]
gpu_pair_production()        [pair_production]
gpu_gauss_project()          [gauss_projection]
launch_gauss_sync_dual()     [gauss_projection && dual_substrate]
launch_begin_strong_energy() [strong_stress_energy]
gpu_solve_latency_poisson()  [latency_field; T00/c² when strong_stress_energy]
gpu_phase_forces()           [forces]
gpu_build_particle_list()    [color/strong/exchange]
gpu_particle_forces()        [color/strong/exchange; remainder colour if FTD-0406]
launch_integrate_forces()    [any force channel]
launch_cluster_inertia()     [cluster_inertia; after integrate, before movement]
gpu_phase_movement()         [movement; reflective particle-face behavior]
launch_matched_gauss_advance() [matched_gauss_dynamics]
launch_complete_strong_energy() / launch_strong_t00()  [strong_stress_energy]
absorbing/flux boundaries    [absorbing_boundary / flux_boundary]
gpu_weak_transmutation()     [weak_transmutation]
gpu_build_particle_list() + gpu_triad_detection()  [triad_binding; after movement+weak]
gpu_gauge_relax()            [su2_gauge / su3_gauge; primed links only]
accumulate_proper_time()     [latency_field || de_broglie_clock; phase optional]
advance device tick
```

The absorbing sponge and reflective/dispersal flux-boundary passes are native
CUDA kernels and run after movement. Gauge relaxation runs only after
`GpuBackend` has primed the lazily allocated link buffers; driving `GpuEngine`
directly without that priming skips the phase. Proper time and optional
de Broglie phase advance on-device exactly once per tick.

This is not blanket CPU/GPU feature parity. Pairwise force channels
(`color_forces`, `strong_force`, `exchange_force`, `confinement`),
`cluster_inertia`, `triad_binding`, `strong_stress_energy`,
`matched_gauss_dynamics`, `verlet_wave_integrator`, the two Floquet
prototypes, and `symmetric_movement_order` are native CUDA.
`knot_tracking` still uses host post-processing after a full
device-to-host mirror; interactive CUDA rejects that host-scoped path
above L=64. These are term-availability and discrete-outcome differences.
The §14 discrete-outcome/missing-term table records the live CUDA contract
and the remaining movement-conflict divergence.

Separately, for supported Poisson paths, the CPU uses warm-started SOR while
CUDA uses spectral/FFT solvers. Their convergence and floating-point behavior
are numerical differences, subject to the production Gauss stencil limitation
documented in §14.

### 4.3 Integration-scheme notes

- The field advance is a staggered update. The default unit-tick form is
  `wave_vel += delta_J; flux += wave_vel`. The `symplectic_leapfrog` toggle
  applies the same staggered update with explicit `dt` factors.
- The 18-point Moore Laplacian uses face weight `1/3`, edge weight `1/6`, and
  self weight `-4`, giving the documented isotropy behavior through the
  tested order.
- The velocity update in `phase_forces` uses the `gamma_FTD` bandwidth budget
  rather than a late hard clamp in `phase_movement`.

---

## 5. Constants Hierarchy

The ontic reference chain computes many framework constants from **D = 3**
(spatial dimensions) and **varpi** (lemniscate constant). Active engine kernels
use a smaller subset of those values plus simulation-scale parameters. This
section documents code usage; derivation status and parameter/derivation
distinctions are tracked in the theory ledgers.

### Ontic chain summary (ontic.h)

| Layer | Constants | Source |
|-------|-----------|--------|
| -1 | `EULER_E` | Self-referential seed (e) |
| 0 | `EULER_GAMMA`, `GAMMA_QUARTER` | Transcendental seeds |
| 0b | `NOME_LEMNISCATIC`, `THETA_LEMNISCATIC` | Modular selection |
| 1 | `VARPI`, `GAUSS_CONSTANT_M`, `PI` | Elliptic geometry |
| 2 | `PF`, `G_STAR`, `SQRT_GSTAR` | Universal operator: G* = Gamma(1/4)/Gamma(3/4) ≈ 2.9587 |
| 2b | `K_CRIT`, `X_BORN` | Euler's identity / emergence of i |
| 3 | `COEFFICIENT` (16 G*^2), `X_PLUS` (larger algebraic root), `X_MINUS` (retired as an `N_C` source; mathematical root only) | Master quadratic |
| 3b | `DELTA_SQUARED`, `DELTA_APPROX` | Dual-substrate splitting: delta^2 = (4G*-1)/(4G*) |
| 4 | `D_SPATIAL`=3, `N_C`=3, `N_GEN`=3, `N_F`=6, `N_BASE`=4, `B_3`=7, `N_EFF`=13 | Framework integers |
| 5 | `ALPHA`, `G_C`, `G_N`=0.01, `SIN2_WEINBERG` | Coupling constants |
| 6 | `K_B`=0.511 (imposed mass calibration), `M_INERTIAL=K_B`, `E_REST=K_B/3`, `M_GRAVITATIONAL=K_B`; `K_MANIFEST := W_SC`=0.505462 (FTD-0388), `K_GENESIS=N_C*K_MANIFEST`=1.516386 | Explicit mass roles and manifestation scale |
| 7 | Mass ratios, mixing angles, CP violation | Particle physics |
| 8 | Cosmological parameters, reference frame context | Extended hierarchy |
| sim | `C_SPEED`=`C_WAVE`=1/sqrt(3), `DAMPING`=alpha | Simulation parameters |

### Active vs reference constants

**Active (used in engine kernels)**:

| Constant | Value | Used in |
|----------|-------|---------|
| `ALPHA` | `1/X_PLUS_PRECISION` | Coulomb force, damping, exchange force, and downstream wave/coupling constants |
| `ALPHA_EFT` | `G_C²` (≡ ALPHA by construction) | Same two-vertex force paths; consistency alias |
| `K_B` | 0.511 | Imposed calibration used by wavepacket amplitude, Larmor scale, and de Broglie frequency |
| `M_INERTIAL` | `K_B` | Particle and cluster momentum integration |
| `E_REST` | `M_INERTIAL*C_SPEED² = K_B/3` | Born–Infeld rest term and particle energy audit |
| `M_GRAVITATIONAL` | `K_B` | Latency Poisson source; numerical equality to `M_INERTIAL` remains imposed |
| `K_MANIFEST` | 0.5054620197 (:= W_SC [SELECTION — ADOPTED, FTD-0388]) | Boltzmann evaporation scale (p = exp(-E/K_MANIFEST²)·K_EVAP_RATE·dτ/dt since 2026-07-19), genesis probability ramp |
| `G_C` | sqrt(ALPHA) | State-flux coupling (phase_read) |
| `G_N` | 0.01 (lattice toy — see §5 gravity banner) | Gravitational force |
| `C_WAVE` | 1/sqrt(3) | Wave propagation speed (Laplacian coefficient) |
| `C_SPEED` | 1/sqrt(3) | Movement speed limit |
| `K_GENESIS` | `N_C*K_MANIFEST` = 1.516386 (FTD-0388) | Genesis threshold |
| `DAMPING` | alpha | Flux dissipation rate |
| `DELTA_APPROX` | 0.9568 | Dual-substrate splitting |
| `WEAK_THRESHOLD` | K_GENESIS | Weak transmutation stress threshold |
| `K_LARMOR` | `4*N_EFF/(3*K_B)` | Larmor radiation modulation |
| `LARMOR_FLOOR` | 0.01 | Minimum Larmor factor |
| `ALPHA_S` | 1.0 [IMPOSED] | Fixed lattice-scale strong-force prefactor; distinct from `ALPHA_S_MZ` |
| `M_YUKAWA` | 1.0 [IMPOSED] | Inverse Yukawa range in lattice units |
| `N_C` | 3 | Color charge count |

`ALPHA = 1/X_PLUS_PRECISION` is the active engine input, and
`ALPHA_PRECISION` is an alias to that same value. The physical identification
`x_+ = 1/alpha` remains **[STRONGLY MOTIVATED CONJECTURE]** (FTD-0013).
`ALPHA_TREE = 1/X_PLUS` is reference-only and feeds no production force, wave,
or damping path. Consequently, agreement of engine results cannot evidence the
master-quadratic identification (FTD-0792).

**Scale-0 inactive constants and broader-engine consumers**:

The constants below are not read by the production Scale-0 CPU/CUDA kernels.
Some nevertheless have explicit consumers elsewhere in the engine repository,
as identified per row.

| Constant | Purpose |
|----------|---------|
| `ALPHA_TREE` | `1/X_PLUS`; tree-level reference value with no production force/wave/damping consumer |
| `ALPHA_G_ELECTRON` | Canonical electron gravitational fine-structure ratio `alpha_G(e,e)=(m_e/m_P)^2`, approximately `1.745e-45` (FTD-0131; epistemic floor inherits FTD-0015) |
| `ALPHA_G_APPROX` | `5.91e-39`; legacy/reference approximation, not the canonical electron ratio |
| `PHI`, `BINDING_ENERGY` | Reference/test quantities with test/audit consumers including `test_sloop.cpp`, `test_triad_confinement.cpp`, `campaign_triad_binding.cpp`, and the ontic audit; production triad detection uses geometric radius/shape thresholds and does not read them |
| `MU_RATIO`, `TAU_RATIO`, etc. | Mass-ratio constants exported by the C++ ontic chain and read by `campaign_triad_binding.cpp`; browser-side counterparts feed catalog/formula metadata. They are not Scale-0 lattice dynamics inputs. |
| `SIN2_WEINBERG`, `SIN2_THETA12`, `SIN2_THETA13`, `SIN2_THETA23`, `WZ_MIXING_ANGLE_COS` | Exported mixing-angle quantities |
| `G_STAR`, `PF`, `X_PLUS`, `X_MINUS` | Master-quadratic intermediates; the roots are algebraic objects, while `x_+ = 1/alpha` is the separate [STRONGLY MOTIVATED CONJECTURE] identification |
| `COS2_THETA_C`, `SIN2_THETA_C` | Exported reference-frame-context fractions |
| `EULER_E`, `EULER_GAMMA`, `GAMMA_QUARTER` | Mathematical seeds |

---

## 6. Voxel Structure

Each lattice site is represented by the `Voxel` struct (`voxel.h`):

### Core fields

| Field | Type | Description |
|-------|------|-------------|
| `state` | int8_t | Ternary: -1, 0, +1 |
| `flux` | Vec3 | Continuous vector field |
| `wave_vel` | Vec3 | Wave velocity (flux propagation) |
| `velocity` | Vec3 | Lattice velocity (nodes per G*-tick) |
| `remainder` | Vec3 | Sub-lattice position remainder |
| `particle_id` | int32_t | Persistent identity (-1 = no particle) |
| `pair_id` | int | Shared ID for particles created by the same pair-production event (-1 = none); event-pair tracking, not proof of quantum entanglement |
| `spin` | int8_t | Z_2 from lemniscate topology (+1/-1/0) |
| `color` | int8_t | Stored labels: 0 = colorless; 1, 2, 3 = the three color labels. The four stored values are not identified with `Z/3Z`. |
| `flavor` | int8_t | Weak-sector label: 0 = none, 1 = e, 2 = mu, 3 = tau |
| `locked` | bool | Part of a bound structure? |
| `accel_mag` | double | Acceleration magnitude (for Larmor) |
| `phase` | double | De Broglie clock phase diagnostic, advanced when its toggle is enabled |
| `flux_strong` | Vec3 | Strong-substrate field |
| `wave_vel_strong` | Vec3 | Strong-substrate wave velocity |
| `flux_weak` | Vec3 | Weak-substrate field |
| `wave_vel_weak` | Vec3 | Weak-substrate wave velocity |

### Dual-substrate fields (active when `dual_substrate = true`)

| Field | Type | Description |
|-------|------|-------------|
| `flux_L` | Vec3 | Left substrate flux |
| `flux_R` | Vec3 | Right substrate flux |
| `wave_vel_L` | Vec3 | Left substrate wave velocity |
| `wave_vel_R` | Vec3 | Right substrate wave velocity |

Observable: `flux = flux_L + flux_R`. Chirality: `chirality_density() = |psi_L|^2 - |psi_R|^2`.

### Latency and proper-time fields

`latency` is active when `latency_field` is enabled and is a bounded
gravitational-potential proxy solved by the latency Poisson path. `tau`
accumulates proper time for manifested particles whenever either
`latency_field` or `de_broglie_clock` is enabled; `phase` advances with `tau`
when `de_broglie_clock` is enabled. Older noetic/reference frame context fields
such as `drag`, `attention`, and sLoop markers are not part of the current
`Voxel` runtime surface.

### Derived quantities

| Method | Formula |
|--------|---------|
| `density()` | `|flux|` |
| `speed()` | `|velocity|` |
| `causal_budget()` | `B = |u|^2/C_SPEED^2 + latency^2`, with stored `u` in raw nodes/tick |
| `bandwidth_used()` | `(|u|^2/C_SPEED^2) / f`, where `f = 1 - latency^2`; reaches one at the selected local boundary |
| `gamma_ftd()` | `1/sqrt(1-B)` on `B<1` |
| `born_infeld_core()` | `-E_REST * sqrt(max(1-B,0))`, where `E_REST=M_INERTIAL*C_SPEED^2=K_B/3` |

### ForceDiag struct

Per-particle force breakdown stored in a separate buffer (`force_diag_`) for UI diagnostics:

| Field | Type | Description |
|-------|------|-------------|
| `f_coulomb` | Vec3 | Electromagnetic (Poisson Coulomb) |
| `f_strong` | Vec3 | Strong nuclear (Yukawa) |
| `f_magnetic` | Vec3 | Lorentz magnetic (v x B) |
| `f_gravity` | Vec3 | Gravitational (grad rho) |
| `f_exchange` | Vec3 | Fermi exchange (Pauli) repulsion |

---

## 7. Force Computation

Forces are computed in `phase_forces()` as **field-mediated** interactions. No pairwise forces exist in the core engine.

### Force pipeline (per manifested particle)

1. **Electromagnetic (Coulomb-like)** -- two modes controlled by `toggles.poisson_coulomb`:

   **Poisson mode (default)**: `F_EM = -ALPHA * state * gradient_scalar(idx, phi_coulomb_)`
   - Solves nabla^2 phi_C = -s via warm-started SOR (`SOR_OMEGA=1.75`, default `SOR_ITERATIONS=6`); `set_sor_iterations(n)` overrides the iteration count at runtime
   - Measured exponent: **-2.25** (ideal: -2.0). GPU: **-2.067**
   - Isotropy ratio: **1.0** at r=5

   **Legacy mode** (`poisson_coulomb = false`): `F_EM = -ALPHA * state * gradient_divergence(idx)`

2. **Gravitational**: `F_grav = G_N * gradient_density(idx)` (tier-2 stencil, r=2)

3. **Lorentz (magnetic)** -- gated by `toggles.lorentz_force`:
   `F_Lorentz = ALPHA * state * cross(velocity, B)` where `B = curl(J)`

### Toggle-gated extensions (default OFF)

| Force | Toggle | Formula |
|-------|--------|---------|
| Color | `color_forces` | SU(3)-inspired pairwise color force |
| Strong | `strong_force` | Yukawa short-range nuclear force |
| Exchange | `exchange_force` | Pauli exclusion (same-spin repulsion) |

### E/B Field Diagnostics

`em_field_at(idx)` returns `{E, B}` where:
- **E = -wave_vel**: Electric field (negative time-derivative of flux)
- **B = curl(J)**: Magnetic field (curl of flux)

`poynting_vector(idx)` returns the Hamiltonian-consistent flux S = c²(E x B).
`EnergyAudit` includes `e_field_energy`, `b_field_energy`, and `total_poynting`.

---

## 8. TermToggles

The `TermToggles` struct is a table-driven Scale 0 runtime registry. It contains
**43 boolean toggles** in `TOGGLE_SPECS[]` plus typed configuration fields that
are intentionally kept outside the boolean table.

Adding a new boolean toggle requires a struct field and one registry row; the
helper methods (`validate`, `enable_all`, `disable_all`,
`cpu_runtime_warnings`) consume the table.

### 8.1 Boolean toggle groups

| Group | Toggles | Role |
|---|---|---|
| Core field/state | `wave_propagation`, `coupling`, `damping`, `genesis`, `evaporation`, `gauss_projection` | Wave propagation, state coupling, dissipation, manifestation/evaporation, Gauss projection |
| Forces and motion | `forces`, `gravity`, `poisson_coulomb`, `emergent_forces`, `lorentz_force`, `movement` | Field-mediated force modes and kinematic update |
| Field extensions | `dual_substrate`, `exact_dual_gauss`, `matched_gauss_dynamics`, `latency_field`, `field_energy_gravity`, `de_broglie_clock`, `db_clock_coulomb`, `symplectic_leapfrog`, `verlet_wave_integrator`, `lorentz_period2_floquet`, `lorentz_bcc_time_floquet`, `ew_background_sweep` | Split/matched field evolution, latency/clock sectors, alternate wave integration, and background drive |
| Damping/noise/boundary | `selective_damping`, `larmor_radiation`, `langevin`, `absorbing_boundary`, `reflective_boundary`, `symmetric_movement_order` | Damping modes, stochastic thermostat, field/particle boundary controls, traversal artifact control |
| Particle-sector extensions | `color_forces`, `strong_stress_energy`, `weak_transmutation`, `strong_force`, `triad_binding`, `pair_production`, `exchange_force`, `cluster_inertia`, `confinement` | Color/strong/exchange explorations, weak flips, pair production, bound clusters, optional linear colour string |
| Gauge/validation/telemetry | `su2_gauge`, `su3_gauge`, `knot_tracking`, `strict_validation` | Per-tick SU(2)/SU(3) link staple relaxation (tick Rule 7b), observation-only knot telemetry, and strict validation behavior |

**Non-Abelian gauge sector (revision 0.9 option a — wired 2026-07-02).**
`su2_gauge` / `su3_gauge` (default OFF) gate one Jacobi-double-buffered
Wilson-action staple sweep per tick over the SU(2)/SU(3) edge link variables:
CPU `relax_su2/su3_links_cpu` (tick Rule 7b, `transmutation_phases.cpp`), GPU
`kernels_gauge.cu` via `GpuEngine::gpu_gauge_relax()` with `GpuBackend`
marshalling host↔device (upload once on activation, download each gauge tick).
Epistemic status: **[IMPOSED]** — the staple/plaquette relaxation form and its
rate calibrations (`GAUGE_RELAX_DT`, `GAUGE_RELAX_BETA` in `constants.h`) are
imported from standard lattice gauge theory, not derived from the postulates.
The links are **write-only w.r.t. the substrate**: nothing downstream consumes
them (`color_forces` uses per-voxel color labels, not links), so the wired
sector is measurement infrastructure — a live link field on which
plaquette/Wilson-loop observables can later be defined against engine state —
and **no LEDGER claim rides on it** (the Moore-layer gauge-group results are
independent of this code path and gain no evidence from it). Guarantees, all
test-enforced: toggles-OFF runs are bit-identical to every pinned golden;
toggles-ON runs leave the substrate fold unchanged (`test_gauge_links` G1a)
and reproduce the pinned gauge golden profile `GAUGE_GOLDEN_HASH` (G1b,
ADR-0012, bit-identical MSVC↔WSL2-gcc); link buffers are lazily allocated
(528 B/site only when the sector is used, revision 4.1b); CPU/GPU agree to
machine-epsilon scale with bit-exact GPU determinism (`test_gauge_gpu_parity`,
WSL2-canonical).

### 8.2 Defaults and validation

`lorentz_period2_floquet` is the FTD-0408 default-off CPU prototype. It retains
the existing 18-point nearest-Moore read but replaces the constant wave kick by
`+3/13` on even ticks and `-1/13` on odd ticks. Its exact bare two-tick pole is
`sin²(theta)=M18/13+3M18²/676`; it is full-band stable and has no q4 term in
`theta²`. The period-two clock and anti-kick are selected architecture. The
prototype's leading speed is `1/sqrt(13)`, so it does not share the production
matter-budget speed `C_SPEED=1/sqrt(3)`. It conflicts with both alternate wave
integrators and is not implemented on GPU/WASM.

`lorentz_bcc_time_floquet` is the FTD-0411 default-off CPU IR prototype for a
selected two-domain hypothesis: BCC supplies normalized temporal return
structure while SC+FCC supplies production physical-space propagation. The
exact kernel forces `c²=1/7` under q4 cancellation, but its irreducible
cube-root branch excludes finite-state positive-norm linear/unitary
localization rational in `M18`. The live path therefore uses stable kicks
`(1+sqrt(2))/7,(1-sqrt(2))/7`, with exact pole
`sin²(theta)=M18/7+M18²/196` and endpoint `400/441<1`. It matches the BCC clock
through q4, differs at isotropic q6, conflicts with every other alternate wave
integrator, and is not implemented on GPU/WASM.

`lorentz_common_cone` is the FTD-0412 native diagnostic, not a runtime toggle.
It verifies the corrected Hermitian Wilson Hamiltonian, exposes the retired
spatial-`D_W` energy oracle with an explicit counterexample, tests selected
leading-slope alignment at `c_s²=1/7`, and enforces the remaining q4 mismatch:
axis cancellation requires `r²=4/3`, while a face diagonal requires `r²=2/3`.
The production `C_SPEED=C_WAVE=1/sqrt(3)` defaults are unchanged. Wilson matter
remains standalone; gauge and native latency gravity still have no measured
propagating poles.

`lorentz_common_cone_improved` is the FTD-0413 native diagnostic. It enlarges
only the standalone Hermitian Wilson kinetic term to the existing SC+FCC Moore
shell, with free symbol
`K_i=sin(q_i)[(1-2b)+b(cos(q_j)+cos(q_k))]`. Exact q4 cancellation uniquely
fixes `b=1/3` and `r²=4/3` in this normalized ansatz. Together with selected
`c_s²=1/7`, the free Wilson-matter and BCC-time flux poles share a cone through
q4, while all seven Wilson doublers remain gapped. Face-diagonal gauge
transport averages the two shortest oriented paths; random-link covariance
and Hermiticity are tested. The parameter defaults to `b=0`, the prototype is
not RenderBridge-integrated, and the flux/matter poles differ at q6.

`lorentz_ir_envelope` is the FTD-0414 analytic/native diagnostic. It corrects
the sixth-order invariant basis to
`M18=S2-S2²/12+S2 Q4/72-Q6/90+O(q8)` and compares the exact live period-two
flux phase with the selected unit-step RK4 Wilson-matter phase. The resulting
leading all-sector speed spread is `11 q^4/540`; the largest same-direction
matter/flux gap is `65 q^4/3969`. `lorentz_ir_q_limit(epsilon)` in
`lorentz_ir_envelope.h` inverts the first expression. These are asymptotic
free-sector diagnostics. The documented `a=ell_P` calibrations allow a
conditional `q=E/E_P` estimate, but do not establish carrier identification,
finite-q control, interactions, or radiative protection.

FTD-0415 is a theory-side radiative gate, not an engine toggle. Exact `O_h`
enumeration shows that the engine's declared spatial/CPT/gauge symmetries allow
independent dimension-four temporal/spatial kinetic coefficients and a
cubic-only native-vector gradient invariant. No interacting 1PI or blocking
coefficient matrix is implemented or measured.

FTD-0416 is also theory-side. It imports the one-loop anisotropic-continuum-QED
velocity flow as an optimistic IR surrogate and proves that its attraction is
only power-law in the endpoint coupling ratio. It neither implements an engine
interaction nor calculates the full-Brillouin-zone threshold. In particular,
the selected `A=P_T J` bridge remains a spatially nonlocal projection rather
than a local RenderBridge action. FTD-0419 later calculates one off-shell
full-zone threshold for the separate local-link branch.

FTD-0417 is likewise theory-side and changes no engine behavior. It freezes a
separate candidate with an independent real link connection and noncompact
unit-plaquette gauge action. The free photon pole is exactly local and stable
at inherited selected `c_A²=1/7`, but the new field is not present in
`RenderBridge`, and its
nearest-link dispersion loses the FTD-0413/0414 q4 improvement. Against that
matter prototype the leading maximum group-speed gap is `3(ka)²/28`.
FTD-0421 later closes the preregistered additive native-current basis negative;
the reaction ledger is bookkeeping, not a link current.

FTD-0418 is also theory-side and changes no engine behavior. It freezes a
nearest-neighbour four-dimensional Euclidean Wilson regulator for the
FTD-0417 links, with `r_0=nu_0=1` and
`r_i=nu_i=1/sqrt(7)`. The selected action has one massless spacetime corner,
15 positive doubler gaps, and exact one-/two-photon Ward vertices. It is not
the live continuous-time/RK4 Wilson module, is not wired into `RenderBridge`,
and does not itself calculate the full-Brillouin-zone matching coefficient;
FTD-0419 supplies the separate off-shell step-scheme integral.

FTD-0419 remains theory-side and changes no engine behavior. A separate CUDA
quadrature integrates the complete exchange, Wilson seagull, fermion bubble,
and two-photon contact terms in a frozen `xi=1` QED_L-like step scheme. It
finds `delta_match/g²=-0.32696906(5)`, so the FTD-0417/0418 bare cone requires
a dimension-four counterterm in that scheme. The coefficient is off-shell and
scheme-specific; no `RenderBridge` coefficient, toggle, or physical on-shell
renormalization is added.

FTD-0420–0425 add observer/test infrastructure without changing production
dynamics. `HistoryEventJournal` records accepted movement, genesis,
evaporation, pair, annihilation, and weak events; enabling it leaves selected
state and `BridgeRng::state_hash()` unchanged. `ConservedChargeBasis` performs
exact rational transition algebra over the frozen discrete basis and finds
rank four, nullity zero (FTD-0421), so native charged-pole and native
dimension-four-flow campaigns are dependency-closed without execution.
`PoleMatchResult` and `CountertermTrajectory` carry scheme metadata and enforce
one on-shell calibration only; no physical on-shell calculation is claimed.
The free source-free transfer is determinant-one and energy-preserving, while
evaporation and annihilation provide exact non-injectivity witnesses for the
full production tick (FTD-0425). All of these APIs are instrumentation-layer
observers/contracts; none changes event ordering, RNG calls, `RenderBridge`
dynamics, or defaults.

FTD-0426 adds a second read-only charge observer and a target-blind engine
campaign. Production movement separates initially neutral polarity pairs, and
the selected Gauss projection produces equal/opposite closed-surface flux:
`Q_A=+0.9993`, `Q_B=-0.9991` on CPU `L=32` and `+0.9972/-0.9968` on WSL2 CUDA
`L=64`. This is a selected constraint realization, not native `U(1)`
emergence. Under the frozen live wave/coupling/Gauss profile the field loses
radius independence (37–55% spread; Gauss residual about 0.338), so autonomous
static electromagnetic dressing is closed negative at that scope. Production
rules and defaults remain unchanged.

FTD-0427 adds `MatchedFaceFlux` as a default-disconnected experimental
sidecar. Its positive-face divergence and backward-difference curl form an
exact discrete complex, and one-tick production movement histories supply the
signed face current in `J_next=J-current+curl(B)`. With Gauss projection off,
the sidecar preserves `div(J)=s` below `1.9e-15` for both signs and all axial
directions at `L=32,64` under MSVC and WSL2 GCC. This is a selected mechanism
test: it does not write `Voxel::flux`, enter the force or energy phases, relax
a Coulomb field, or support reaction events. Production rules and defaults
remain unchanged.

FTD-0428 promotes that mechanism only to a **default-off selected engine
branch**, not to the shipping profile. `MatchedGaussDynamics` stores electric
flux on oriented faces and magnetic flux on oriented edges. Explicit
initialization solves `(D D^T)phi=s`, sets `E=D^T phi`, and mirrors the centered
face field into `Voxel::flux`. With `matched_gauss_dynamics=true`, the CPU tick
skips legacy field writers, snapshots signed state around production movement,
extracts its oriented current, advances

```
B_half -= C_SPEED * dt * C^T E
E      += C_SPEED * dt * C B_half - current
```

and mirrors centered `E` back to the voxel field. Validation requires the
isolated periodic single-substrate conservative-movement sector; reactions and
all competing writers/projectors are rejected. The branch fails closed before
explicit initialization and forces a synchronized CPU fallback from a GPU
backend. Its isolation errors are fatal even when general toggle validation is
advisory, and a process override that prevents CPU fallback rejects the branch.
The selected branch also requires the locked unit tick (`dt=1`). Default-off
golden behavior is unchanged.

The source-free staggered map preserves its discrete modified quadratic energy,
and the one-time initialization is the minimum-norm face field for neutral
source data. FTD-0428 campaigns pass at `L=32,64` under MSVC/GCC (worst Gauss
residual `9.15e-13`, voxel mirror residual zero). These are selected finite-
lattice and implementation results. No production force reads this field; no
reaction-complete charge, photon quantization, common cone, or radiative
protection is claimed.

FTD-0429 adds only a read-only Fourier observer and campaign. In the existing
single-substrate linear sector with `wave_propagation=true`, `coupling=true`,
zero initial field, and every Gauss/damping/movement/reaction/force path off,
the production tick generates a longitudinal mode response

```
Z(k) = (div_c J)_k / s_k
     = (G_C / C_WAVE^2) * sum_a sin(k_a)^2 / M_18(k)
Z(k -> 0) = 3 * G_C
```

The time-domain campaign fits the oscillatory response rather than projecting
or solving the offset. MSVC/WSL2-CUDA `L=32` controls and WSL2-CUDA `L=64`
infrared points give `Z0=0.256247622955862`, within `1.01e-4` relative of
`3G_C`, and reject a zero intercept by `Delta BIC=279.14`. This establishes a
finite native dynamical polarity susceptibility in the reaction-free linear
sector. It does not add a microscopic charge register or change defaults.

FTD-0430 adds a second read-only observer/campaign around the same unchanged
tick. A sparse neutral pair is primed to execute exactly one normal movement
hop while a locked copy supplies the counterfactual source history. With only
wave, coupling, and movement active, the moving-minus-stationary divergence is
zero immediately after the hop, appears on the next field tick, stays within
`r_infinity <= tau+1`, and fits

```
R_k(tau) = Z_k + B_k cos(omega_k tau) + C_k sin(omega_k tau)
Z_k       = (G_C / C_WAVE^2) * sum_a sin(k_a)^2 / M_18(k)
sqrt(|B_k|^2 + |C_k|^2) / |Z_k| = 1 / cos(omega_k / 2)
```

Fresh MSVC/WSL2-CUDA `L=48,96` records pass the exact hop, causal support,
pole, residue, mirror, and backend gates. The v2 infrared intercept is
`0.256268547570661`; `Delta BIC=336.88` against zero. This is a measured
reaction-free moving-source property of the production tick. It adds no charge
register, projector, field state, force, or default-on behavior.

FTD-0431 adds a read-only reaction-mode observer and a disabled campaign
target. With `evaporation=true`, the isolated source reproduces the exact
per-tick survival law. With production wave/coupling also enabled, `S_k(t)` is
not a single exponential: the locked normalized-RMS gate fails by factors of
about 2.1 to 11.1. The generated flux contributes to `local_energy` in
`phase_write`, thereby lowering later evaporation probabilities. The admitted
`L=32` MSVC/WSL2-CUDA records agree and satisfy the exact field recurrence to
`2.41e-13`. FTD-0431 is outcome D at the analysis-model layer; it changes no
tick rule or default and licenses neither conservation nor an infrared decay
rate. Its per-arm `L=64` path is also too slow for reuse; the next observer must
batch modes and measure the dressed reaction hazard directly.

FTD-0432 implements that hazard observer without changing the production
phase. `prepare_delta_j()` supplies the diagnostic acceleration buffer; scratch
arrays reconstruct the standard unit-tick flux/velocity write, after which the
observer evaluates the exact six-neighbor-plus-site energy, proper-time factor,
and `K_EVAP_RATE*exp(-E/K_MANIFEST^2)` probability. The observer consumes no
RNG and is state/RNG neutral over 32 ticks. Full `L=32` MSVC/WSL2-CUDA records
pass conditional source and occupancy expectation gates. The low-mode hazard
is suppressed to `0.001225` during field dressing. This is a measured
production-feedback mechanism, not a new field, reaction rule, conservation
law, infrared limit, or default-on behavior.

FTD-0433 reuses the same observer without modification in the disabled
`campaign_native_dressed_hazard_ir_scaling` target. It initializes one neutral
full-occupancy `<100>,n=1` square source and chooses the last recorded
transition from the exact `M18` pole, not from measured hazard data. WSL2 CUDA
runs `L={12,16,20,24,32,40,48}` with eight seeds; Windows CPU reproduces the
full `L=32` journal. All execution gates pass, but the first-antinode hazards
are nonmonotonic and the locked endpoint suppression fails. FTD-0433 is
outcome C (`UNRESOLVED_SCALING`) and changes no production phase, toggle
default, constant, or RNG behavior.

FTD-0478--0539 add observer-only exact subcell/face-current instrumentation and
close its frozen mobile-matter candidate. `SubcellPolarityShape` maps ternary
state plus the existing remainder to trilinear site weights;
`FaceCurrentSegment` analytically deposits the straight worldline on oriented
faces. The centered cell field remains a rendering/compatibility projection.
The implicit-action observers reconstruct endpoint-split connection equations,
solve corner and edge Legendre sectors, and evaluate both registered energies.
All non-derivative current/field/Gauss identities remain at roughly `1e-14` or
better, but the action is not a production law: smooth corner roots fail
energy, and edge roots have a converged reflection-plane cusp with no unique
algebraic force; edge energy also fails. `common_action_face_dynamics` was not
added. These APIs do not enter `RenderBridge`, change phase ordering, or alter
defaults, scenarios, forces, RNG, CPU/GPU routing, or golden hashes.

FTD-0540--0549 add ten further observer-only APIs. `LocalPolarityRegularity`
proves that the compact cardinal hat/trilinear representation necessarily has
an integer-plane cusp and prices the smooth alternatives.
`QuadraticPolarityCoat` and `QuadraticCoatFaceCurrent` implement the selected
positive non-cardinal alternative: a tensor quadratic B-spline coat and its
polynomial-exact straight-segment `B2/B1` oriented-face current. Continuity,
current moment, polarity, reversal, translations, cubic covariance, periodic
crossing, and the locked inactive-plane C1 gate pass.
`QuadraticCoatSpacetimeCurrent` adds the derived endpoint currents and temporal
coat, while `QuadraticCoatGaugeActionResult` verifies their common interaction
and exact open-worldline gauge endpoint identity. `FixedStepEnergyScopeResult`
proves that this common interaction is not yet an energy-preserving mobile
law: fixed-step configuration stationarity lacks the time-node equation, and
the exact registered counterexample changes endpoint energy by `1/8`.
`MatchedMidpointPoyntingResult` proves the auxiliary field update nevertheless
has exact work exchange `Delta U_field=-<Ebar,K>` and exact Gauss transport.
`QuadraticCoatMatterWorkResult` analytically varies the same coat action and
maps its canonical endpoints to the production kinetic dispersion. The locked
uniform harmonic witness is gauge covariant but has nonzero matter-work defect
up to `4.10e-5`, closing the universal fixed-step identity negative.
`QuadraticCoatNeutralPairWorkResult` performs the required periodic,
Gauss-realizable two-worldline follow-up. Exact source/field algebra survives,
but total energy misses by up to `9.68e-9`; the frozen minimal common action is
closed negative as an exact-energy mobile law.
`AcceleratedWorldlineEnergyResult` integrates the production dispersion under
a constant collinear force and replaces the frozen midpoint displacement by
the exact energy secant. Its work identity closes to `2.72e-20` in 144 arms.
`AcceleratedCoatSpacetimeCurrent` deposits that nonuniform trajectory into the
same quadratic coat. Total face current remains endpoint-only, while the
endpoint-weighted currents and temporal occupation change with the schedule;
continuity, gauge, and reversal residuals stay below `2.19e-14`.
`EndpointScheduleUnderdeterminationResult` then supplies an exact polynomial
witness that endpoint and midpoint kinematics do not determine those deposits:
two monotone schedules with identical listed kinematics and total current
differ by `q*d*epsilon/30` in their source moments. All 96 arms pass below
`3.35e-17`.
These APIs remain in `ftd_eft`; they are not a `RenderBridge` state path and
supply no production force, completed self-consistent matter-energy
transaction, toggle, scenario, or default change.

FTD-0599 adds the final locked observer for the no-new-persistent-variable R0
site hop. `SiteOnticAtomicState` uses only the existing site, half-open
remainder, velocity, polarity, `J`, and `W`. The observer deposits the exact
FTD-0577 Moore current, applies the native source kick and drift, and sets
matter impulse equal to native translation-momentum recoil. The first
ballistic arm has one independently Arb-certified root and closes continuity
and recoil, but misses both independent native energy gates by `6.3504e-6`
against `1e-12`. Verdict:
`SITE_ONTIC_NATIVE_RECOIL_MAP_FAILS_ATOMIC_COMPATIBILITY`. The inverse and
repeated campaign are not run after this conjunctive failure. The observer is
not a toggle or production movement law and does not adopt a new primitive.

FTD-0600 adds the observer-only R2 escalation in
`constituent_complete_charged_trimer.{h,cpp}`. `ChargedTrimerState` retains
three explicit `(charge, anchor+remainder, momentum)` constituent records and
matched face-electric/edge-magnetic fields; aggregate quadratic coats and
face currents are derived only inside the coupling evaluation. A local
nine-component implicit solve advances constituent endpoints, exact currents,
the matched field, field impulses, and selected quartic binding impulses in
one transaction. Its reverse API accepts the final state rather than a stored
forward record. All locked common-action and repeated-motion gates pass,
including 24 legitimate site hops and energy drift `1.67e-15`; the independent
matched pseudomomentum defect is `5.69e-3`, so isolated recoil is not licensed.
This module is not wired to `RenderBridge`, a toggle, a scenario, CUDA, or
WASM, and it does not define physical particle composition or binding.

FTD-0601 extends that observer to six dynamical constituents arranged as two
charge-conjugate trimers with no fixed compensator. Common-action identities,
state-only inversion, repeated bound motion, and nine site hops remain
constructive, while the isolated matched pseudomomentum defect is `1.03e-2`.
FTD-0602 replaces only the initial Gauss representative with the unique
zero-mean minimum-energy longitudinal field. It restores an inward impulse at
the registered placement and preserves 16-step reversal, but leaves a
`1.91e-4` pseudomomentum defect. FTD-0603 then replays the same transaction at
168 fractional placements. Fourteen of 32 principal-axis phases are
non-attractive, so compact rigid force robustness closes negative; the phase-
mean momentum classification remains unresolved at `2.57e-8`. These are
observer-only diagnostics and introduce no production path.

FTD-0604 adds the observer-only symmetric breathing discriminator in
`test_symmetric_breathing_matter_core.cpp`. It minimizes the unchanged
minimum-field plus binding energy over one scale coordinate already contained
in constituent positions. Exact algebra gives `V_bind=6(lambda^2-1)^2` and
curvature `48`; the optimized scale changes by about `4.5e-5`, lowers the
static barrier only `0.00507%`, and leaves 14/32 phases non-attractive. The
locked static branch closes negative because its finite-difference
stationarity gate also misses. This is not wired to production, a toggle, a
scenario, RenderBridge, CUDA, or WASM.

FTD-0605 adds `test_full_mirrored_internal_shape_core.cpp`, an observer-only
six-coordinate zero-centroid shape relaxation. A zero-mean periodic Green
kernel accelerates static energy evaluation, but every returned minimum is
independently rebuilt by the direct Gauss solver. The exact selected binding
Hessian has rank three, leaving three rigid-rotation zero modes. Twenty-nine
of 32 optimizations exhaust the locked 900 evaluations; the other three hit
the `0.20` local chart boundary with nonzero downhill gradients while their
pair distances stay near `sqrt(2)`. Fast/direct energies agree to `2.22e-17`,
and executed common/inverse arms close near `1e-15`. The registered local
static branch is closed negative; global `SO(3)` orientation remains open.
Nothing is wired to production, a toggle, scenario, RenderBridge, CUDA, or
WASM.

FTD-0770 adds `coupled_quartic_clock_field.{h,cpp}` to the `ftd_eft` leaf
library as an isolated selected-extension probe. It supplies the general
even-power action law, positive-action clock sites, oriented fixed phase
connections, compliance factors, a kick--drift--kick graph Hamiltonian, and a
finite-graph connection-integrability checker. The focused test verifies
quadratic/quartic/sextic periods, quartic chain dispersion, sextic controls,
total action, compliance, gauge covariance, fixed holonomy, and action-boundary
rollback. The API is not a `Voxel` field or a RenderBridge phase; it has no
toggle, CUDA/WASM path, scenario, production consumer, or golden-state effect.
Its `G*` result is scoped negative: the registered dimensionless linear ratio
retains the power-law exponent but cancels the local period normalization.

FTD-0776 adds no production API or checked-in observation target. In an
isolated detached worktree it built a transfer-supplied dumper against
unchanged production physics source and ran four `L=32`, seed-1 CPU/SOR arms
with wave propagation, the production state--flux operator at the
selected/parametric `G_C=sqrt(alpha)`, Gauss projection, and
genesis/evaporation enabled; Langevin and imposed de Broglie/Coulomb clocking
were disabled. The preselected aggregate `q_active` yielded zero complete
cycles in every 200,000-tick arm. This is an observable- and configuration-
scoped engine fact, not a universal no-clock result. The artifact bundle is
`engine/results/gstar_qactive_pilot_20260802/`; no toggle, RenderBridge phase,
CUDA/WASM path, scenario, calibration, or golden state changed.

FTD-0840 adds `eft/native_pair_energy_recursion.h` as a header-only isolated
selected-extension reference. It implements the globally monotone implicit
discrete-gradient step for
`H=p^2/(2m)+lambda*q^4=lambda[(q|q|)^2+y^2]`, returning the signed pair
coordinates, equation residuals, energy residual, swept-area orientation, and
fail-closed solver status. The focused CTest verifies both square sheets,
forward and signed-step reversal, physical momentum reversal, strict nonzero
orientation, invalid/nonconvergent rejection, and 20,000-step compact-shell
confinement. Maximum observed equation, one-step energy, reverse-state, and
long-run energy residuals are `1.10e-14`, `2.67e-15`, `7.44e-15`, and
`4.06e-13`. The header contains no `G*`, target period, `Voxel`,
`RenderBridge`, toggle, production phase, CUDA/WASM path, or scenario. It is
reference mechanics for an adopted pair coupling, not evidence that the
production substrate supplies that coupling or an exact finite-tick cadence.

FTD-0841 audits the local vector lift without adding an engine path. Existing
`Voxel::flux` and `Voxel::wave_vel` already provide the local canonical type
`(J,W)`. The self-pair tensor `J otimes J` has Frobenius energy `|J|^4`, and
the associated vector discrete-gradient recursion is uniquely solvable,
reversible, energy- and angular-momentum-conserving, oriented, and bounded
conditional on selecting the radial onsite coupling. No such term is present
in the production phase or energy ledger. Cubic symmetry alone permits an
anisotropic quartic, only invariant linearly polarized sectors have the scalar
`G*` period law, and no combined onsite-plus-spatial energy transaction has
been implemented. FTD-0841 therefore closes a source-type audit, not a
production feature.

FTD-0842 audits the next coupled reference without adding an implementation.
The selected simultaneous discrete gradient for production `K` plus onsite
`lambda|J|^4` is uniquely solvable and exactly energy closed, but its linear
control inverts `2mI+h^2K/2`; that inverse is dense on a connected quotient.
It therefore cannot be inserted as one P4-local production tick. Positive
edge energy also leaves only the box-wide constant zero mode, so a bounded
profile is not an exact critical-quartic clock. At `lambda=0` the selected map
is implicit midpoint, not the existing kick--drift map or its cross-term tick
invariant. No toggle, field, solver, phase, or energy-ledger entry was added.

FTD-0844 constructs a selected two-channel reference without changing the
engine. In the orthogonal common/relative chart, the required spatial metric
has rank one: only the common mode propagates, while each relative site runs
the isolated quartic recursion. This would preserve the existing common-mode
tick invariant plus positive relative onsite energies with one-shell
dependencies. The frozen dual path instead computes separate L/R Laplacians,
equivalent to `b=0`, not the required `b=a`; no cross-gradient, relative
quartic, readout, toggle, or production ledger exists. FTD-0843's invalid
`26/28` parent and FTD-0844's repaired `28/28` certificate affect no binary.

FTD-0846 similarly changes no binary. Its selected onsite reference adds an
exchange-odd pointer `(r,pi)` and the quartic difference interaction
`kappa(r-q)^4/4`; the exact proof closes a reversible three-account energy
transaction and exposes its phase torque. No `Voxel` member represents that
pointer, no phase implements the coupling, no continuous pointer history is
converted to `state in {-1,0,+1}`, and no common-field propagation or toggle
consumes it. FTD-0845's invalid `31/32` parent and FTD-0846's repaired `32/32`
certificate are theory-only.

FTD-0848 also changes no binary. Its selected mathematical reference adds a
continuous latch coordinate with sextic three-well potential, an AVF damped
transaction, scalar bath/controller ledgers, and a basin quotient to
`state in {-1,0,+1}`. Production `Voxel::state` supplies only that codomain:
no member stores the latch coordinate or bath ledger, no tick phase evaluates
the sextic/AVF update or coupling schedule. FTD-0847's invalid first run and
FTD-0848's repaired `30/30` certificate are theory-only.

FTD-0850 source-locks and classifies the current production alternative; it
also changes no binary. With seed/site/tick retained, genesis is deterministic
and context blind, its nonzero divergence sign is odd, and the single-field
branch withdraws positive radial field energy. Evaporation is a real
many-to-one `+/-1 -> 0` map. These are partial ternary acquisition/loss
fragments, not the FTD-0848 latch: every finite-energy unlocked state has
positive evaporation hazard, dual genesis omits the single-branch withdrawal,
and no event-level bath/controller receiver consumes genesis/evaporation in the
aggregate audit or retains erased metadata. FTD-0849 is preserved invalid at
`28/30`; the verifier-only FTD-0850 repair passes `30/30`. No production code,
toggle, layout, or tick ordering changed.

FTD-0851 also changes no binary. It proves that an all-energy receiver for
`+/-1 -> 0` needs an odd label plus nonnegative energy account; when exported
energy `B` is positive, one signed amplitude `a=s*sqrt(2B)` carries both. The
selected balanced representation `L=s*sqrt(B), R=-s*sqrt(B)` is a reference
formula only. Production `flux_L/flux_R` is not assigned that event pulse and
is not included as a separate receiver energy by `update_energy_ledger_cpu`.
The current same-sign collision is a remainder-reset barrier fragment,
annihilation emission is sign blind at fixed continuous input, and the full
event journal is observation-only. No new public type, receiver storage,
event transaction, or propagation phase is implemented.

FTD-0852 again changes no binary. It proves a selected exact history-carrier
update and audits the existing dual path. Production already contains a
homogeneous relative candidate: `phase_read` applies identical L/R operators
and equal matter sources, while `phase_write` advances the channels separately
before rebuilding their sums. However, no event writes the FTD-0851 odd pulse
into that difference channel; the stencil propagates both ways rather than as
the exact injective clearing shift; and `update_energy_ledger_cpu` squares only
the reconstructed common `flux`/`wave_vel`, so `R=-L` relative energy is
unaccounted. No event hook, dual-energy term, history rail, barrier, or public
receiver type was added.

FTD-0853 also changes no binary and adds no public engine type. It proves that
the existing six face neighbours and dual wave-velocity coordinates are
sufficient for a selected reference deposit: opposite L/R radial impulses of
magnitude `sqrt(B/6)` preserve the common field and transfer exactly `B` when
the local pre-event port coordinate `Q0` vanishes. Production does not yet
derive event energy `B`, test the ready port, write the impulse, account dual
relative energy/current, prove clearing under its bidirectional stencil, or
encode the complete erased state. No event hook, ledger term, barrier, toggle,
phase, or C++ interface was added.

FTD-0855 changes no binary and adds no public type. It observes that the
existing diagnostics already assign a manifested record the matter energy
`B=E_REST+flat_particle_kinetic_energy=gamma*E_REST`, which disappears from
that diagnostic account when evaporation clears the state. It then proves at
reference scope that `D=Q/sqrt(12)` maps the selected six-face radial mode
isometrically to the odd history rail, so a one-cell outward shift forms the
next receiver port. The production drift ledger still excludes rest and
separate dual energy; the shared bidirectional dual field is not a reserved
directed rail; and no event routing, reciprocal barrier, or full-state receiver
exists. No ledger term, event hook, field partition, phase, toggle, or C++
interface was added.

FTD-0856 changes no production binary behavior. It proves the minimum reference
contract for a reciprocal record boundary: a distinct hold/exchange eligibility
value, retained incoming/outgoing characteristic orientation, and a controlled
identity/swap matrix between matter amplitude and the incident relative pulse.
The contract now has an isolated public witness in
`ftd::eft::scatter_reciprocal_record_port`; its focused Release CTest passes
`1/1`. `Voxel::locked` supplies only the hold distinction. The dual `flux_L/R`
plus `wave_vel_L/R` type has capacity for a relative conjugate pair, but
production does not construct protected characteristics or apply the
scatterer. The same-sign movement reset remains nonreciprocal under FTD-0506.
No production event hook, characteristic buffer, energy term, gate controller,
phase, toggle, or tick-phase consumer was added.

FTD-0857 is preserved execution-invalid: its seven source hashes passed before
the verifier aborted on a nonexistent event-slice marker. The narrowly frozen
FTD-0858 repair passes `40/40`. It proves that the existing fixed-input genesis
and evaporation acceptances are deterministic, Moore-local, and target blind,
but operate on common `flux`/`wave_vel`. Antisymmetric L/R perturbations leave
those trigger inputs fixed while changing the relative receiver, so acceptance
does not determine an on-shell reciprocal port. The relative pair nevertheless
admits an exact incoming/outgoing energy-current chart. On the axial
plane-symmetric sector, C18 reduces to the 1D Laplacian, but selected
`C_WAVE^2=1/3` gives trace defect `(8/3)sin^2(k/2)` from exact one-cell rails.
The isolated `ftd::eft` implementation and focused Release CTest pass `1/1`.
No production event hook, common-to-relative transducer, protected buffer,
relative-energy ledger, controller state, toggle, or tick-phase consumer was
added.

FTD-0859 is preserved execution-invalid at `31/36`; all seven source hashes
passed before five verifier-only structural/sign/whitespace checks failed. The
narrow FTD-0860 repair passes `36/36`. Its isolated
`ftd::eft::pump_relative_action` reference applies
`z'=sqrt((I+B)/I)sJz` on a nonzero canonical pair, adds exactly `B` to action,
has unit symplectic Jacobian, and has a known-event inverse. It fails closed on
an empty carrier. The unlabelled result is intentionally nonfaithful because
opposite event signs collide on opposite background phases; faithful signed
history remains the separate reserved rail. The focused Release CTest passes
`1/1`. No production event hook, protected local pair, relative/loss ledger,
controller, export law, toggle, default, or tick-phase consumer was added.

FTD-0861 is preserved execution-invalid at `35/36`; every source and exact
mathematical gate passed, while one production-boundary prose marker failed.
The one-repair FTD-0862 wrapper passes `36/36`. Its isolated
`ftd::eft::step_phase_referenced_action_rail` reference supplies a prepared
nonzero phase calendar, loads the FTD-0860 signed quarter-turn, shifts each
pair one cell outward, and exports the complete tail pair. Calendar compliance
is `spatial_twist-temporal_advance=0 mod 2pi`; on that selected subspace the
readout recovers `event_energy=I-I_*` and event sign from the oriented area.
The finite excess-action ledger closes and the input/tail-completed step is
injective. The focused Release CTest passes `1/1`. The API is not C18 and no
production phase source, protected/cubic rail, event hook, relative/tail
ledger, controller, `Voxel`, toggle, default, or tick-phase consumer was added.

FTD-0863 adds the isolated
`ftd::eft::step_catalytic_phase_reference` realization. A nonzero canonical
reference rotates autonomously with conserved action and defines an orthonormal
phase frame for a separate signal pair. The FTD-0856 controlled identity/swap
exchanges matter amplitude with the phase-orthogonal signal coefficient, so an
initially-zero signal can receive exactly the event energy and orientation; the
same involution absorbs it and the reference action is unchanged. The focused
Release CTest passes `1/1`. This refines the existing selected phase-rail type;
it does not add a production `Voxel` field, event hook, C18 mode, controller,
backreaction, toggle, default, tick phase, `G*` cadence, or energy-current
ledger.

FTD-0864 is preserved execution-invalid at `39/40`; FTD-0865's one-repair
wrapper passes the inherited `40/40`. The isolated
`ftd::eft::evolve_clock_gated_hamiltonian_cycle` API evaluates the exact
one-cycle flow of the imposed harmonic reference Hamiltonian on full matter
and signal modes. The registered frequency/coupling winding produces exact
identity or full-mode swap, reports the common/relative actions, strict minimum
reference reserve, interaction-energy/reference-loan equality, and endpoint
energy residual, and fails closed when the reference reserve is insufficient.
Its focused Release CTest passes `1/1`. The API explicitly reports that
eligibility is frozen and a quartic load-blind controller is not established.
No production field, `Voxel`, event hook, controller state, compensation,
quartic clock, C18 rail, toggle, default, or tick phase was added.

FTD-0866 is preserved execution-invalid at `39/40`: its C14 certificate
expression omitted the `nu*I_r` term of the already-registered Hamiltonian.
FTD-0867 freezes exactly that one coordinate correction and passes the
inherited `40/40`. The isolated
`ftd::eft::execute_ternary_eligibility_handshake` API derives hold/exchange
eligibility from the existing ternary latch as `s^2`, requires a zero incoming
signal and a reference-orthogonal matter mode whose oriented sign matches the
latch, inherits the strict reference-reserve gate, and performs one exact
clock cycle. Its output signal decodes the original sign and energy, and the
API reports a gate-zero request to release the clutch with zero clutch-switch
work. The focused Release CTest passes `1/1`. The API explicitly reports that
microscopic latch reset, autonomous acknowledgement, clock synchronization,
and cubic production coupling are not supplied. No production field, `Voxel`,
event hook, bath state, toggle, default, tick phase, `G*` cadence, or selector
weight was added.

FTD-0868 is preserved execution-invalid after recording `25/26` checks through
C26 and aborting during C27 construction on two verifier-representation
defects. FTD-0869 freezes exactly those repairs and passes inherited `44/44`.
The isolated `ftd::eft::execute_signal_acknowledged_two_stroke_reset` API
implements the registered endpoint contract: the completed local signal is the
target-blind acknowledgement, the compressed exchange enforces `I_0>B`, the
selected nonsmooth cusp resets the latch within the second half-cycle, and the
controller/scalar-bath ledger closes at `kappa*A`. A final full-mode handoff to
an initially empty output port leaves the local latch and matter/signal modes
ready while the output decoder retains sign and event energy. The focused
Release CTest passes `1/1`.

This API is not a subgradient time integrator, microscopic bath, protected
cubic transport path, production latch coupling, or native `G*` synchronizer.
It changes no `Voxel`, production field, toggle, default, or tick phase. Those
physical mechanisms and perturbative robustness remain open.

FTD-0870 is preserved execution-invalid at `39/40`; its only failure was a
whitespace-sensitive protocol-prose marker. FTD-0871 freezes one C35
whitespace-normalization repair and passes inherited `40/40`. The isolated
`ftd::eft::execute_reversible_ternary_signal_uncomputation` API decodes the
completed oriented signal, verifies that it matches the ternary latch, and
applies the exact `Z_3` operation `s_after=s-d(E)`. It also records the inverse
`s=s_after+d(E)`, leaves signal energy unchanged, and hands the signal to an
initially empty output port. The focused Release CTest passes `1/1` for both
signs, the no-event state, inverse recovery, mismatch/backpressure failures,
and scope flags.

The API recomputes acknowledgement and adds no persistent acknowledgement bit,
reset-history trit, or logical bath. It explicitly does not implement the
continuous `x`-latch trajectory, controller work, protected cubic transport,
production coupling, or native `G*` synchronization. No `Voxel`, production
field, toggle, default, or tick phase changed.

FTD-0872 closes the isolated actual-layer permutation itself. The
`ftd::eft::apply_oriented_ternary_quarter_turn` API acts on the ordered ternary
latch/port pair by identity when ineligible, by `R(s,o)=(-o,s)` in the forward
orientation, and by `R^-1(s,o)=(o,-s)` in the reverse orientation. It verifies
the exact inverse, label-norm preservation, sign-reversal equivariance,
nine-state bijection, ready emission, and reciprocal absorption. A nonempty
port undergoes the all-domain reciprocal exchange; the API does not implement
the provably noninjective “empty-port transfer, otherwise identity” wrapper.
The focused Release CTest passes `1/1`, and the coupled actualization/rail set
passes `11/11`.

The label norm is not a physical energy scale. At the FTD-0872 boundary the
API supplies no continuous actuator, controller-work ledger, protected cubic
transport, production coupling, or native `G*` synchronization.

FTD-0873 adds the isolated
`ftd::eft::evolve_hamiltonian_ternary_quarter_turn_cycle` reference actuator.
It embeds the ternary latch/port as one canonical carrier pair, adds one
independent clock pair, and evaluates the exact imposed harmonic full-cycle
flow. The inactive branch holds; the active orientations produce `R` and
`R^-1`. The result reports the imposed record-energy scale, strict
bidirectional reserve, maximum clock-action excursion, transient
reference/interaction energy exchange, gate-zero and antiphase switching
accounts, and endpoint energy residual. The focused Release CTest passes
`1/1`; the coupled chain passes `12/12`.

This is not a production controller. The amplitude/frequency scale and
Hamiltonian are imposed, eligibility/orientation are frozen over a cycle, and
repeat/norm-only gating is not a one-shot schedule. Native formation, dynamic
gate-zero control, protected cubic transport, production coupling, robustness,
and synchronization to the separate quartic `G*` calendar remain open. The
implementation changes no `Voxel`, production field, toggle, default, or tick
phase.

FTD-0874 adds the isolated
`ftd::eft::step_alternating_oriented_ternary_parity_rail` and exact inverse
APIs. Existing integer global-tick parity selects the two alternating
disjoint nearest-neighbour matchings; each active bond applies
`(a,b)->(-b,a)`. The result exposes active bonds, label norm and support
counts, exact inverse recovery, ready transfers, occupied reciprocal
exchanges, locality, endpoint retention, and explicit scope flags.

For a prepared isolated sign at rail site zero, the focused test verifies
exact one-edge-per-tick transport through twelve ticks for both signs and then
reverses the history exactly. It also exhaustively checks all finite ternary
states through length six, a fixed-matching non-propagation control, occupied
backpressure, composition with the FTD-0873 actuator, and fail-closed invalid
inputs. The focused Release CTest passes `1/1`; the coupled chain passes
`13/13`.

This is a selected finite-horizon reference scheduler, not a production rail.
Occupied bonds retain information but do not guarantee readiness or progress.
The engine still supplies no native intersite Hamiltonian for the bond turn,
automatic cubic axis/routing law, sustained congestion resolution, reciprocal
finite-boundary completion, production event/energy-current coupling,
robustness, or synchronization to the separate quartic `G*` calendar. No
`Voxel`, production field, toggle, default, boundary mode, or tick phase is
changed.

FTD-0875 adds the isolated
`ftd::eft::evolve_local_canonical_hamiltonian_parity_rail_cycle` API. Each
rail site carries one reference canonical pair `(q,p)`. A complete selected
harmonic clock cycle integrates the disjoint-bond generator to the exact
forward or inverse FTD-0874 parity layer. The result exposes per-bond before/
after states, carrier and interaction energy, clock-action excursion, local
antisymmetric bond current, prepared-record energy transfer, endpoint
residuals, exact inverse recovery, and scope flags.

The implementation also exposes the finite scalar common symplectic form. It
pairs boundary-mirror sites and is therefore a nonlocal, length-dependent
control—not local substrate hardware. Exhaustive focused tests verify the
actual-section ternary map, generic continuous inverse, positivity and reserve
bounds, current antisymmetry and energy continuity, exact prepared-energy
transfer, and the special zero-clock-backreaction actual orbit. The focused
Release CTest passes `1/1`; the coupled chain passes `14/14`.

This is an imposed isolated reference Hamiltonian, not production dynamics.
The engine still does not form the onsite canonical doublet natively, recover
its amplitude/frequency scale, route in three dimensions, resolve sustained
congestion, close reciprocal finite boundaries, couple the current to
production event accounting, establish robustness, or synchronize the common
harmonic phase to the separate quartic `G*` calendar. No `Voxel`, production
field, toggle, default, boundary mode, or tick phase is changed.

FTD-0876 adds the isolated read-only
`ftd::eft::carrier_from_voxel`,
`ftd::eft::flux_history_to_markov_carrier`, and
`ftd::eft::evolve_free_wave_kick_drift` APIs. They identify the existing
production `flux`/`wave_vel` pair with the exact first-order Markov chart of a
two-slice flux history and verify symmetric-stiffness free-wave kick/drift plus
its inverse on all three vector components. The result exposes native pair
count, exact history recovery, inverse residual, vector bond generator,
damping pullback scale/phase determinant, and explicit non-promotion flags.

The focused test copies a real `Voxel` without mutation, recovers exact vector
history, evolves and reverses a symmetric three-site stiffness witness,
verifies the component-summed FTD-0875 generator and damping determinant, and
fails closed on nonsymmetric stiffness, malformed matrices, invalid steps, and
nonfinite data. The focused Release CTest passes `1/1`; the coupled chain
passes `15/15`.

This closes only native carrier-coordinate availability. The complete
production tick is not declared symplectic: damping, Langevin, Gauss
projection, manifestation/evaporation, and boundary maps are not included in
the exact free-wave map. No prepared ternary record, FTD-0875 production bond
actuator, amplitude/energy scale, constrained-phase Gauss proof, complete
environment ledger, route, or quartic-`G*` synchronization is supplied. No
`Voxel`, production field, toggle, default, boundary mode, or tick phase is
changed.

FTD-0877--0880 add the isolated
`ftd::eft::decompose_matched_gauss_canonical`,
`ftd::eft::make_static_ternary_gauss_record`, and
`ftd::eft::prepare_matched_gauss_record` APIs. They operate only on the
already selected `MatchedFaceFlux` incidence complex. For matched divergence
`D` and `L=DD^T`, the observer constructs `q=DJ`, `p=L^+DP`, the exact
longitudinal fields, and divergence-free transverse remainders. It verifies
reconstruction, orthogonality, and equality of the full and reduced
symplectic pairings.

The static-record constructor mean-subtracts periodic nonneutral probes
explicitly, solves the minimum-energy longitudinal representative, and sets
charge momentum to zero. The preparation observer returns both the prepared
field and the discarded longitudinal discrepancy; the pair reconstructs the
input exactly. It deliberately reports that unledgered preparation and an
environment dynamics have not been supplied.

The production-boundary observer records the exact symbol mismatch. The live
central divergence/gradient composition is `-sum sin^2(k_a)`, whereas the
18-point SOR stencil gives `-2` rather than `-1` at `(pi/2,0,0)` and `-4`
rather than `0` at `(pi,0,0)`. Finite SOR and the default manifested-site
skip are additional departures. Therefore the production pass is described
as approximate constraint relaxation, not an exact matched or canonical
projector.

The repaired exact certificate passes `66/66`; the invalid FTD-0877--0879
executions remain preserved. The focused Release CTest passes `1/1`, and the
coupled actualization chain passes `16/16`. No production `Voxel`, toggle,
default, or tick phase changed. No native dynamic record formation, uniformly
local charge conjugate, reversible environment dynamics, production parity
actuation, physical scale, routing, or quartic-`G*` synchronization follows.

FTD-0881--0882 add the isolated
`ftd::eft::apply_reversible_checkerboard_gauss_layer`,
`ftd::eft::reverse_reversible_checkerboard_gauss_layer`, and
`ftd::eft::ReversibleCheckerboardGaussPreparation` APIs. On even periodic
matched-face probes, one cell parity at a time rotates the local six-face
Gauss residual into an explicit signed environment port. Fresh-zero ports
make the layer an affine orthogonal projection; alternating parity layers
converge from empty flux to the FTD-0880 minimum-energy record without calling
the pseudoinverse in a local gate. Every outgoing port is retained, so reversing
the layer order reconstructs the exact finite input history.

The witness records field energy, incoming/outgoing environment energy, and
local source work. At its empty-field/fresh-port limiting boundary, field and
history energies are equal and each is one half of supplied source work. It
also reports that generic exact completion is not achieved in a fixed finite
number of local layers. The repaired certificate passes `60/60`; the invalid
FTD-0881 `58/60` execution remains preserved. The focused Release CTest passes
`1/1`, and the coupled actualization chain passes `17/17`. No `Voxel`,
production field, toggle, default, boundary mode, or tick phase changed. The
API supplies no autonomous fresh-port recycling, positive source reservoir,
moving-source continuity, finite-boundary completion, production coupling,
physical scale, Born target, or quartic-`G*` synchronization.

FTD-0883--0884 add the isolated `ftd::eft::FinitePortGaussBattery`
reference witness. A cyclic bank of `C` initially zero signed-port vectors
supplies exactly the first `C` fresh checkerboard layers while retaining the
field, every port, the cursor, and the reversal history. Once a generic
nonzero output returns to the cursor on layer `C+1`, that port is no longer
fresh. This is a capacity theorem for the explicit cyclic bank, not a
universal finite-dimensional memory no-go; compressed exact-real memories and
open or unbounded signed-history rails remain outside the result.

For source work `w_x=q_x(e_x-r_x)/6` and positive reference energy
`E_b=b_x^2/2`, the witness applies the unique sign-preserving strict-reserve
update

```text
b_x' = sgn(b_x) sqrt(b_x^2 - 2 w_x),
b_x  = sgn(b_x') sqrt((b_x')^2 + 2 w_x).
```

Battery-energy loss is exactly the local source work, so total
field+port-bank+battery energy is invariant and the full finite state reverses
exactly. The quadratic battery type, sign branch, and reserve scale are
`[IMPOSED]`; no canonical Hamiltonian or symplectic reservoir, native battery
formation/recharge, or natural scale has been derived. The repaired
certificate passes `56/56`; the invalid FTD-0883 `54/56` execution remains
preserved. The focused Release CTest passes `1/1`, and the coupled
actualization chain passes `18/18`. No production `Voxel`, field, toggle,
default, boundary mode, or tick phase changed. No universal memory no-go,
moving-source routing, production coupling, Born target, or quartic-`G*`
gearbox follows.

FTD-0885--0886 add the isolated
`ftd::eft::CanonicalSourceCenteredGaussGate` reference witness in
`include/ftd/eft/canonical_source_centered_gauss_gate.h`. For one normalized
active mode it uses

```text
y = d_x J / sqrt(6),  s = q_x / sqrt(6),  u = y - s,  a = e_x / sqrt(6)
N = (u^2 + a^2 + pi_u^2 + pi_a^2) / 2
L = a pi_u - u pi_a
H = omega I + omega N + sigma omega (1 - cos(theta)) L / 4.
```

Because `{N,L}=0` and `|L|<=N`, the carrier Hamiltonian is positive and one
clock cycle implements the exact canonical forward/reverse quarter-turn. On
the zero-conjugate section this is exactly the FTD-0882 residual/environment
gate. The raw-work audit uses
`E_raw=(y^2+a^2)/2` and `U_int=-s*y+s^2/2`; it verifies
`Delta E_raw=w` and `Delta U_int=-w`. At this fixed-source local scope, source
work is interaction-energy exchange rather than an independent battery drain.

The same API audits the FTD-0884 phase boundary. Its square-root coordinate
map has an exact cotangent lift, but that lift changes positive oscillator
energy by `-w(1+p_b^2/b^2)`, so the desired decrement is exact only on the
zero-conjugate Lagrangian section. A state-dependent phase-blind action drain
is not symplectic. `shift_open_canonical_history_right` supplies the separate
complete-pair open-history kinematic witness and exact inverse; it is not a
finite recycler or a production environment.

The invalid FTD-0885 `60/64` execution remains preserved. The FTD-0886 repair
changes only three certificate-marker defects (the fourth failure was
dependent) and passes the inherited `64/64`; the
focused Release CTest passes `1/1`, and the coupled actualization chain passes
`19/19`. No production `Voxel`, field, toggle, default, boundary mode, or tick
phase changed. The witness supplies no autonomous common parity Hamiltonian,
dynamical source formation/motion/recoil, physical open-history hardware, 3D
routing, production coupling, native scale, Born target, or quartic-`G*`
gearbox.

FTD-0887--0888 add the isolated
`ftd::eft::evolve_autonomous_phase_parity_source_reaction_cycle` reference API
in `include/ftd/eft/autonomous_phase_parity_source_reaction.h`. It uses one
autonomous phase circle split into six disjoint `C1` windows. In generator
order

```text
(L_ua^0, L_ar^0, N_r^0, L_ua^1, L_ar^1, N_r^1),
alpha = (pi/2, eta, pi/2, pi/2, eta, pi/2),
H = Omega I + 6 Omega N + Omega sum_j (6 alpha_j/pi) rho_j(theta) G_j.
```

Each base `N` flow makes an identity winding and each pulse integrates to its
target angle. Because the windows have disjoint interiors, the two-color
sequence needs no assumed commutation between different-color generators and
no external integer-parity switch. The carrier obeys
`H-Omega I >= 3 Omega N`; clock action returns at each full cycle.

The same API adds one source-reaction canonical pair and implements the exact
local splitter. On the ready slice it sends residual energy

```text
E_hist = cos^2(eta) E_res,
E_react = sin^2(eta) E_res.
```

Thus the FTD-0886 history-only endpoint is positive-energy saturated: nonzero
zero-initialized reaction energy must be paid by reduced history energy. One
scalar reaction coordinate cannot be symplectic, so one canonical pair is
minimum in the registered onsite-direct-sum class. This is another instance of
the existing selected canonical-pair type, not a new type. `eta=pi/4` is the
unique equal split only after output-channel exchange symmetry is imposed; the
equal split is `[SELECTION]`.

The invalid FTD-0887 `68/72` execution remains preserved. The FTD-0888 wrapper
repairs only three representation defects and passes inherited `72/72`; the
focused Release CTest passes `1/1`, and the coupled actualization chain passes
`20/20`. No production `Voxel`, field, toggle, default, boundary mode, or tick
phase changed. The reaction pair is not yet a physical ternary matter source:
native source identification, mass/inertia, intercell motion/recoil, native
phase-window formation/origin/scale, physical open history, 3D routing,
production coupling, Born targets, and the separate quartic-`G*` gearbox remain
open.

FTD-0889--0890 add the isolated
`ftd::eft::analyze_cubic_reaction_vector_source_transport` reference API in
`include/ftd/eft/cubic_reaction_vector_source_transport.h`. It makes the next
boundary executable without wiring production. Cubic symmetry forbids a
scalar-only reaction from choosing nonzero spatial recoil, and one
three-dimensional vector copy cannot carry a nondegenerate alternating form.
The minimum orientation-free carrier in the registered onsite class is
therefore three canonical pairs `(R,Pi) in T1u+T1u`. FTD-0888's one-pair
minimum remains valid for its scalar internal reaction channel; a spatial
one-pair slice is conditional on an independently supplied fixed direction.

For selected rest energy `E0` and limiting speed `c`, the API implements the
exact cotangent chart

```text
p = sqrt(E0 + |Pi|^2/4) Pi/c,
x = Dg(Pi)^(-T) R,
K(p) = sqrt(E0^2 + c^2 |p|^2) - E0 = |Pi|^2/2.
```

The physical source then follows exact free Hamiltonian drift; the centered
subcell quotient and `FaceCurrentSegment` provide reversible endpoint motion
and exact discrete continuity. A required matter impulse supplied by the
matched field momentum ledger orients the reaction. The available residual
energy fixes `sin^2(eta)=K_req/E_res`; insufficient energy fails closed, and
equal splitting is not universal.

The invalid FTD-0889 `64/68` execution remains preserved. FTD-0890 repairs
only three representation defects and passes inherited `68/68`; the focused
Release CTest passes `1/1`, and the isolated actualization chain passes
`21/21`. No production `Voxel`, field, toggle, default, boundary mode, or tick
phase changed. `E0`, `c`, the relativistic dispersion, and the vector reaction
role are selected/imposed. Native vector common-action formation and coupling,
mass-scale derivation, stable matter, production, Born targets, and the
separate quartic-`G*` gearbox remain open.

FTD-0891--0892 add the isolated
`ftd::eft::analyze_collective_reaction_triplet_inertia` reference API in
`include/ftd/eft/collective_reaction_triplet_inertia.h`. It does not change
production. Given selected constituent canonical positions/momenta, tangent
probes, positive constituent rest energies, and simultaneous impulses, it
constructs the orthogonal Helmert modes and verifies

```text
X = (1/N) sum x_a,
P = sum p_a,
sum p_a dot dx_a = P dot dX + sum_(mu>0) pi_mu dot dq_mu.
```

Thus the FTD-0890 three-pair reaction vector is already the exact collective
sector of the selected constituent phase space; no new selected vector type is
introduced. The API reconstructs every constituent coordinate/momentum and
checks that internal zero-sum impulses cancel while any external constituent
impulses change `P` by their exact sum.

For the selected per-constituent dispersion
`E_a=sqrt(epsilon_a^2+c^2|p_a|^2)`, the API evaluates the unique common-velocity
minimum at fixed `P` and verifies

```text
p_a = epsilon_a P / sum epsilon,
E_coll = sqrt((sum epsilon)^2 + c^2 |P|^2),
M_coll = sum epsilon / c^2.
```

This is conditional inertial additivity, not an absolute mass derivation. The
API explicitly reports the mismatch from a static binding offset that does not
participate in the boosted family. Its scope flags deny static-Hessian mass
identification, exact total field-matter continuous Noether momentum,
constituent formation, stable-pole derivation, production coupling, Born-target
use, and native `G*` synchronization. The invalid FTD-0891 `62/68` run is
preserved; FTD-0892 repairs only five verifier representations and passes the
inherited `68/68`. The focused Release CTest passes `1/1`.
The isolated actualization chain passes `22/22`.

FTD-0893 adds the isolated
`ftd::eft::analyze_dressed_boost_momentum_map` reference API in
`include/ftd/eft/dressed_boost_momentum_map.h`. For one matter-like and one
field-like time-odd amplitude per cubic axis it accepts

```text
A = [[a,g],[g,k]],
B = [b_m,b_f],
```

requires `a>0`, `k>0`, and `ak-g^2>0`, and evaluates

```text
M = B A^-1 B^T
  = (k b_m^2 - 2 g b_m b_f + a b_f^2)/(ak-g^2).
```

It returns the unique minimum-energy matter/field allocation at fixed imposed
total momentum, verifies `E_kin=|P|^2/(2M)`, exposes the exact ambiguity
`B->sB`, `M->s^2M`, and checks signed cubic covariance. It fails closed for a
non-positive energy Hessian, zero momentum row, zero scale control, invalid
tolerance, or nonfinite input. Static energy shifts are recorded but cannot
alter `M`. Public flags deny that the physical total-momentum map, absolute
mass, common-action Noether closure, stable pole, production coupling, Born
target, or native `G*` synchronization has been derived. The first locked
SymPy certificate passes `57/57`; focused Release CTest passes `1/1`. No
production source, `Voxel`, toggle, default, or tick phase changes. The
isolated actualization chain passes `23/23`.

FTD-0896 adds the isolated
`ftd::eft::analyze_bloch_quasimomentum_lift` reference API in
`include/ftd/eft/bloch_quasimomentum_lift.h`. It accepts two principal Bloch
triplets in `[-pi,pi)^3`, two explicit integer winding triplets, an imposed
momentum scale, and a finite sawtooth truncation order. Componentwise it
computes

```text
k_12 = wrap(k_1+k_2),
w_12 = w_1+w_2+carry(k_1+k_2),
k_tilde_12 = k_12+2 pi w_12 = k_tilde_1+k_tilde_2.
```

The analyzer records the reciprocal information lost by principal values,
the imposed candidate `P=p_* k_tilde`, and the periodic finite-range control
`2 sum_(r=1)^R (-1)^(r+1)sin(rk)/r`. It fails closed for nonfinite or
nonprincipal labels, nonpositive scale/tolerance, invalid truncation order,
or winding overflow. Public flags deny a continuous torus-to-real section, a
finite-range global unwrapped generator, derived winding dynamics, a derived
physical momentum scale, exact total field-matter momentum, absolute mass,
production coupling, Born-target use, or native `G*` synchronization. The
FTD-0894 and FTD-0895 invalid executions are preserved; the scoped FTD-0896
repair passes inherited `81/81`. Focused Release CTest passes `1/1`, and the
isolated actualization chain passes `24/24`. No production source, `Voxel`,
toggle, default, or tick phase changes.

FTD-0897 adds the isolated
`ftd::eft::apply_reciprocal_carry_transaction` reference API in
`include/ftd/eft/reciprocal_carry_reservoir.h`. It accepts two principal
Bloch-label triplets, one supplied opposite increment triplet, an integer
reciprocal reservoir, an imposed momentum scale, and a tolerance. For each
component it computes

```text
k_1' = wrap(k_1+q),  c_1 = carry(k_1+q),
k_2' = wrap(k_2-q),  c_2 = carry(k_2-q),
W' = W+c_1+c_2,
k_1'+k_2'+2 pi W' = k_1+k_2+2 pi W.
```

The result records the unique carry update, full inverse recovery, candidate
physical totals after the imposed scale, periodic-band energy before/after,
and the fact that band energy is blind to the reservoir. It fails closed for
nonfinite inputs, nonpositive tolerance/scale, nonprincipal labels,
unrepresentable carries, integer overflow, conservation failure, or reversal
failure. Public flags deny that the increment, substrate reservoir, reservoir
energy, physical scale, exact total field--matter momentum, absolute mass,
production coupling, Born target, or native `G*` synchronization has been
derived. The first locked exact certificate passes `89/89`; focused Release
CTest passes `1/1`, and the isolated actualization/EFT chain passes `25/25`.
No production source, `Voxel`, toggle, default, or tick phase changes.

FTD-0898 adds the isolated
`ftd::eft::analyze_quartic_relative_carry_gearbox` reference API in
`include/ftd/eft/quartic_relative_carry_gearbox.h`. It composes the existing
`advance_native_pair_energy` discrete-gradient relative quartic with the
FTD-0897 reciprocal-carry transaction. The endpoint generates

```text
Delta P_L = +Delta Pi/sqrt(2),
Delta P_R = -Delta Pi/sqrt(2),
q = Delta Pi/(sqrt(2) p_*),
P_L+P_R = sqrt(2) P_C = constant.
```

The result reports the exact relative-energy residual, generated increment,
before/after principal charts and windings, aggregate carry endpoint,
common-momentum residual, signed-step inverse residual, and the continuum
quartic period-amplitude product. It fails closed for nonfinite input, invalid
tolerance or scale, relative-solver failure, unrepresentable chart winding,
integer overflow, child carry failure, endpoint mismatch, or inverse failure.
Public flags deny common-mode coupling, matter--field identification, derived
`p_*`, carry energy, finite-tick `G*` cadence, absolute mass, production
coupling, Born-target use, or a new selected type. The first locked exact
certificate passes `97/97`; focused Release CTest passes `1/1`, and the
isolated actualization chain passes `26/26`. No production source, `Voxel`,
toggle, default, or tick phase changes.

FTD-0899--0901 add the isolated
`ftd::eft::analyze_common_relative_connection_gearbox` reference API in
`include/ftd/eft/common_relative_connection_gearbox.h`. FTD-0899 and FTD-0900
are preserved execution-invalid; the exactly scoped FTD-0901 repair passes the
inherited exact certificate `87/87`.

For the imposed reference law

```text
L = (M/2)|Cdot|^2 + (m/2)|Ddot|^2
    + gamma D.Cdot - lambda|D|^4,
P = M Cdot + gamma D,
K = P - gamma D,
Delta K = -gamma Delta D,
```

the analyzer solves the theorem's unique three-vector discrete-gradient
endpoint and reports equation, energy, canonical momentum, mechanical impulse,
equal/opposite channel impulse, canonical angular momentum, reciprocal carry,
signed-step inverse, connection-curvature, clock-Hessian, and clock-origin
tilt audits. The reciprocal chart remains conditional on the supplied
`momentum_scale=p_*`.

It fails closed for nonfinite data, nonpositive masses/coupling/tolerance or
scale, zero step, zero iteration limit, endpoint-solver failure,
unrepresentable chart winding, integer overflow, child-carry failure,
invariant failure, or reverse failure. Public flags state that the connection
law is imposed; `i` supplies orientation but does not derive `gamma`; physical
coordinates, `p_*`, absolute mass, integer-tick `G*`, a discrete variational
action, production coupling, and Born weights remain underived. It also
reports the exact registered boundary that continuous nonzero `gamma` adds
critical-clock Hessian `gamma^2/M`; `gamma=0` restores critical quarticity and
turns off the mechanical gearbox.

The pinned MSVC 14.44 Release build succeeds, focused CTest passes `1/1`, and
the isolated actualization chain passes `27/27`. No production source,
`Voxel`, renderer, boundary, toggle, default, or tick phase changes.

FTD-0902--0903 add the isolated
`ftd::eft::analyze_self_pair_connection_critical_gearbox` reference API in
`include/ftd/eft/self_pair_connection_critical_gearbox.h`. FTD-0902 is
preserved execution-invalid at `80/81`; the exactly one-substitution FTD-0903
repair passes the inherited exact certificate `81/81`.

The linearly polarized analyzer composes the existing quartic recursion and
reciprocal-carry witness in the canonical-common rest sector:

```text
U(D) = D |D|,
A(D) = gamma U(D),
Lambda = lambda + gamma^2/(2 M),
H_rest = Pi^2/(2 m) + Lambda D^4,
Delta K = -gamma Delta U.
```

It reports the bare/connection/effective quartic couplings, signed self-pair
endpoints, mixed connection derivatives, mechanical impulse, symmetric
common displacement, child quartic energy and carry, signed-step inverse,
zero origin Jacobian/Hessian, moving-sector quadratic-ray coefficient,
conditional equal-partition value, symmetric-cycle zero-drift boundary, and
the continuum `G*` period-amplitude product.

It fails closed for nonfinite input, nonpositive masses/bare coupling,
nonpositive tolerance or scale, zero step, zero iteration limit, effective-
coupling or signed-pair overflow, child recursion/carry failure, invariant
failure, or reverse failure. Public flags state that the connection law is
imposed; `i` does not derive `gamma`; equal self-dual partition is not adopted;
physical `p_*`, mass, net transport, finite-tick `G*`, production coupling,
and Born weights remain underived. Generic moving sectors are explicitly not
reported as exact quartic clocks.

The pinned MSVC 14.44 Release build succeeds, focused CTest passes `1/1`, and
the isolated actualization/EFT chain passes `28/28`. No production source,
`Voxel`, renderer, boundary, toggle, default, or tick phase changes.

FTD-0904 adds the isolated
`ftd::eft::analyze_oriented_even_self_pair_rectifier` reference API in
`include/ftd/eft/oriented_even_self_pair_rectifier.h`. It composes the same
quartic recursion/carry witness with the imposed polarized connection

```text
A(q,e,chi) = chi gamma q^2 e,
Lambda = lambda + gamma^2/(2 M),
H_rest = pi^2/(2 m) + Lambda q^4.
```

The analyzer requires a unit polar axis `e` and exact chirality `chi=+/-1`.
It reports connection endpoints, mechanical impulse, directed common
displacement, child energy/carry/reversal, moving-sector coefficient,
turning amplitude, continuum period-amplitude product, cycle displacement,
mean velocity, and mean gear ratio. Public flags preserve the symmetry no-go
for an even inversion-equivariant polar function of `D` alone and mark native
axis/chirality formation, `gamma`, physical scale, mass, production, Born,
and integer-tick cadence as open.

It fails closed for nonfinite input, invalid mass/coupling/step/tolerance/
scale/iteration count, nonunit axis, invalid chirality, overflow, child
failure, invariant failure, or reverse failure. The locked exact certificate
passes `74/74`; the focused Release CTest passes `1/1` and the isolated
actualization/EFT chain passes `29/29`. No production source, `Voxel`,
renderer, boundary, toggle, default, or tick phase changes.

FTD-0905--0907 add the isolated
`ftd::eft::analyze_native_ternary_dipole_phase_wedge_memory` reference API in
`include/ftd/eft/native_ternary_dipole_phase_wedge_memory.h`. It accepts a
finite collection of positions, ternary states, flux vectors, and
wave-velocity vectors. It requires a neutral region with exactly one `+1`
and one `-1` endpoint, then reports the origin-independent dipole axis,
projected bilateral coordinates/momenta, phase wedge, time-reversal and
signed-cubic audits, Gram determinant, imposed memory energy, wedge-
conservation residual, radial minimum/curvature, centrifugal term, and the
FTD-0840 swept-area parity control.

Public flags distinguish native-type representability from dynamics. They
record that the dipole axis and time-odd phase wedge use existing field
types, while production formation, maintenance, erasure, the central memory
law, rectifier coupling, `gamma`, physical scale, mass, production, Born, and
integer-tick cadence remain open. A nonzero wedge is explicitly incompatible
with treating the same central mode as the pure radial G* clock.

The analyzer fails closed for nonfinite input, invalid memory parameters,
empty/nonternary/nonneutral regions, missing or nonunique endpoints,
coincident endpoints, zero dipole, zero phase wedge, child-probe failure, or
invariant failure. The FTD-0907 exact certificate passes `75/75`; the focused
Release CTest passes `1/1` and the isolated actualization/EFT chain passes
`30/30`. No production source, `Voxel`, renderer, boundary, toggle, default,
or tick phase changes.

`TermToggles` member initializers own the shipping constructor defaults.
`TOGGLE_SPECS[]` owns table-driven metadata and bulk enable/disable defaults;
the member initializers and table rows must remain synchronized. The 13
constructor-default-ON fields are `wave_propagation`, `coupling`, `damping`, `genesis`,
`gauss_projection`, `forces`, `gravity`, `poisson_coulomb`, `movement`,
`lorentz_force`, `selective_damping`, `dual_substrate`, and
`weak_transmutation`. These shipping defaults are distinct from the six
conceptual core rule families in §1. The weak-transmutation governance caveat
there remains in force. Exploration
toggles such as `pair_production`, `latency_field`, `langevin`, color/strong
extensions, exact dual Gauss, and `matched_gauss_dynamics` are default off.

`validate()` enforces dependencies and conflicts. Important examples:

| Rule | Reason |
|---|---|
| `poisson_coulomb` conflicts with `emergent_forces` | Avoids running two mutually exclusive EM force models |
| `lorentz_force` requires `forces` | Lorentz is part of the force phase |
| `selective_damping` requires `damping` | It refines the damping path rather than replacing the master switch |
| `weak_transmutation` requires `dual_substrate` | Current CPU/GPU implementation uses chirality/split-substrate state |
| `triad_binding` requires `color_forces` | Triad detection depends on color labels/interaction context |
| `field_energy_gravity` requires `latency_field` | Field energy enters through the latency Poisson source |
| `lorentz_period2_floquet` requires `wave_propagation` | The prototype replaces only the free-wave kick coefficient |
| `lorentz_period2_floquet` conflicts with `verlet_wave_integrator` and `symplectic_leapfrog` | The proof assumes the unit-step default kick-drift map |
| `lorentz_bcc_time_floquet` requires `wave_propagation` and conflicts with every alternate wave integrator | The FTD-0411 surrogate owns the unit-step free-wave kick sequence |
| `matched_gauss_dynamics` requires the isolated periodic conservative-movement sector | The FTD-0428 face/edge state owns all field evolution and rejects unjournaled writers, projectors, forces, and reactions |

`enable_all()` applies each table row's default value for bulk-managed toggles;
it does not blindly turn every experimental flag on. `disable_all()` turns
bulk-managed booleans off while preserving direct control of non-bulk/internal
flags as defined by the registry.

### 8.3 Non-boolean configuration fields

| Field | Type | Purpose |
|---|---|---|
| `bcc_stencil` | `BccStencilMode` | Selects the sublattice stencil path for `phase_read`; non-default modes require single-substrate validation. |
| `langevin_site_filter` | `SiteClass` | Selects which parity/site class the Langevin thermostat targets. |
| `langevin_T` | `double` | Target Langevin temperature. |
| `langevin_gamma` | `double` | Langevin damping/noise rate. |
| `langevin_seed` | `unsigned int` | Deterministic stochastic seed. |
| `coulomb_charge_coupling` | `double` | Scalar in the Gauss-law source term. |

---

## 9. Lagrangian System

`lagrangian.h` exposes a partial six-term discrete field/kinematic action
diagnostic plus Rayleigh dissipation. It is not a complete variational
foundation for the production tick: optional matter-force branches remain
selected update rules, and the onsite velocity term does not generate
`phase_read`'s coded curl source.

| Term | Expression | Physics |
|------|-----------|---------|
| L_KINETIC | `0.5*|wave_vel|^2` | Flux-field kinetic term |
| L_GRADIENT | Pair-counted 18-point weighted link sum | Flux-field gradient term whose variation matches the production Laplacian |
| L_BI | `-E_REST*sqrt(1-|u|^2/C_SPEED^2-L^2)` | Selected causal kinematic core. The implementation clamps the radicand at zero and currently evaluates this diagnostic at every voxel, independently of `state`. |
| L_COUPLING | `+g_c*s*div(J)` | Diagnostic electric interaction; its J-variation gives the outward `-g_c*grad(s)` source |
| L_VELOCITY | `-g_c*s*(v*J)` | Selected onsite matter-side velocity coupling |
| L_GAUSS | `-lambda_G*(div(J)-rho)^2` | Selected Gauss penalty; not proof of full-event charge conservation or U(1) redundancy |
| R (dissipation) | (alpha/2) \|wave_vel\|^2 | Vacuum drag |

`compute_lagrangian_diagnostics()` returns `LagrangianDiag` with per-term sums, Gauss violation, conservation checks, and append-only `cell_volume` metadata. FTD-0404 makes the spatial sum explicit as `S=Σ_v L_density(v)·V_cell`, with `V_cell=a_lat³=1` for the production unit lattice. The local densities remain quadratic; the cubic power belongs to the integration measure. This leaves every historical numerical value unchanged and does not support arbitrary non-unit spacing without separately rescaling difference operators and couplings.

---

## 10. Three Simulation Scales

### Scale 0: Voxel (RenderBridge)

The lattice engine. Each site is a Voxel with ternary state + continuous flux.
Forces are field-mediated via discrete differential operators. The current tick
ladder is documented in §4: read/write, optional pair production, Gauss,
latency, forces, movement, boundary, weak/triad, proper-time, and ledger sync.

### Scale 1: Particle (ParticleEngine)

Lattice-free engine with continuous positions and analytical forces. All constants from `ontic.h`.

**Force convention** (matches Scale 0 Poisson solver):
```
F_EM   = -alpha * q_i * q_j * r_hat / (4pi * (r^2 + soft^2))
F_grav = +G_N * m_i * m_j * r_hat / (r^2 + soft^2)
```

**Velocity Verlet** (symplectic): half-kick -> drift -> recompute -> half-kick. `dt` and softening are configurable; the C++ default softening is 1.0 and web scenario presets commonly set 0.1 for atomic-scale demos.

Diagnostics report the active Hamiltonian only: Coulomb PE is zero when the Coulomb toggle is off, gravity PE is zero when gravity is off, and `total_pe = coulomb_pe + gravity_pe`. The WASM Scale 1 binding exposes particle positions, velocities, masses, locked flags, effective radii, charges, IDs, extended telemetry, and snapshot force vectors so browser overlays can be backend-true.

Files: `particle_engine.h`, `particle_engine.cpp`, `wasm/bindings_particle.cpp`, and `web/js/scales/scale1/*`.

### Scale 2: Atom (AtomEngine)

Composite atoms with inter-atomic forces and covalent bonding. Three forces:
- **Ionic** (Coulomb): F = -alpha * Q_i * Q_j * r_hat / (4pi * r^2_soft)
- **Van der Waals** (LJ 12-6): 24 eps [2(sigma/r)^12 - (sigma/r)^6] / r
- **Covalent** (harmonic spring): -k * (r - r_eq) * r_hat

Automatic bond formation (r < 1.2 sigma_avg) and breaking (r > 2 r_eq). `compute_atomic_properties(Z, N)` derives all parameters from ontic constants.

**Atomic closure-context vector (diagnostic/readout only).** `compute_atomic_properties(Z, N)` also returns `closure_context`, and `AtomEngine::closure_context_for(id, cfg)` exposes the same shell-context readout for live atoms. The vector records `Z`, `n_shell`, `z_eff`, `r_cloud`, `delta_valence`, `xi_orbital`, `tau_electronic`, and ratios such as `kappa`, `zeta`, `beta`, and `theta`. Its cloud scale follows the shell-context estimate `r_cloud = R_BOHR*n_shell^2/z_eff`: across a period, stronger screened return force contracts the cloud; at a new shell, the scale resets outward. This is a physics-facing scale diagnostic, not a force retuning. `Atom.radius` and `vdw_sigma` remain the legacy simulation/LJ interaction scales used by bonding, CUDA pair-force uploads, and scale bridges.

**JS <-> C++ constant divergence (deliberate, [IMPOSED] both sides).** The C++ AtomEngine derives force prefactors from the ontic chain (Coulomb `ALPHA/(4pi)` in `atom/atom_forces.cpp`; bond spring `ALPHA*K_B/r_eq^2*order` in `atom_engine.cpp`), while the web mock (`web/js/bridge/mock-atom-engine.js`) uses visualization-scale MD tunings from `web/js/constants.js` (`AE_K_COULOMB = 2.0`, `AE_K_BOND = 50.0`, plus a 3.5*r_eq break threshold vs C++'s 2*r_eq). Both parameter sets are calibrations, not derivations; force magnitudes and equilibrium time scales are NOT expected to match across backends. **The JS mock is the production Scale-2/3 backend** — `wasm-bridge.js` `_aeHasWasm` is deliberately disabled (audit P1-2, deferred feature D-11) until a Planck-unit <-> Bohr-unit conversion shim exists, so every browser Scale-2/3 readout comes from the JS engine. Cross-backend numeric comparisons of AE outputs are meaningless until that shim lands.

Files: `atomic_closure_context.h`, `atom_engine.h`, `atom_engine.cpp`, `src/atom/atom_forces.cpp`, `web/js/atomic-props.js`, `web/js/bridge/mock-atom-engine.js` (production web backend).

### Scale Bridge

`coarsen()` extracts particles from lattice voxels. `refine()` calls `inject_wavepacket()` to reconstruct lattice state. Round-trip fidelity: position error = 0, velocity exact, energy error ~7e-13%.

`coarsen_to_atoms()` / `refine_to_particles()` for Scale 1 <-> 2.

Files: `scale.h`, `scale_bridge.cpp`.

---

## 11. Test Catalog

### Summary

`engine/CMakeLists.txt` and its included CMake registration logic are the
authority for native test targets; source-file totals are not test totals.
A configured `engine/build` reported **610 CTest registrations on 2026-08-18**
via `ctest -N -C Release`. This is a dated registration snapshot, not a pass
count, and it varies with configure options and platform. In particular, CUDA
tests are registered conditionally when `FTD_ENABLE_CUDA` is enabled. CTest
labels include `unit`, `physics`, `golden`, `slow`, and `gpu`.

The categories below are explicitly representative, not a complete or frozen
target registry. Names outside sections marked historical are current CTest
names in CMake at the date above.

### Test categories

**Core infrastructure:**
- `constants` -- Ontic chain values, alpha precision, G* verification
- `lorentz` -- Lorentz factor, bandwidth limit, speed capping
- `lattice` -- Periodic wrapping, neighbor enumeration
- `voxel_properties` -- Voxel derived quantities (density, speed, bandwidth, gamma, Born-Infeld)
- `lattice_operators` -- Lattice topology, corner wrapping, neighbor symmetry, coord round-trip
- `discrete_operators` -- Laplacian, divergence, curl, gradient accuracy and symmetry
- `bridge_dynamics` -- RenderBridge tick cycle integration (vacuum stability, injection, propagation)
- `scale_ratio` -- FC-3 identity criterion: `ScaleRatio` value object (χ = ξ/R, β = δ/R), `is_phenomenon()`, `observe()`; header-only, NO_CORE, α-blind (23 assertions; `engine/include/ftd/scale_ratio.h`)

**Lagrangian verification:**
- `born_infeld`, `energy_conservation`, `gauss`, `stress_energy`, `thermodynamics`, `lagrangian`

**Ontic physics:**
- `ontic_chain`, `genesis`, `gravity_dynamics`, `annihilation`, `annihilation_conservation`, `wave_collapse`

**Wave and field:**
- `campaign_wave_dynamics`, `gauge`, `polarization`, `momentum`, `lorentz`, `flux_mediated`, `campaign_quantum_correlations`

**Lagrangian forces:**
- `variational_coulomb`, `lorentz`, `dissipation`, `portable_field`

**Perfected Electromagnetism:**
- `maxwell` -- 6 sections (M1-M6): div(B)=0, Faraday, E perp B, Coulomb 1/r^2, wave equation, Ampere-Maxwell
- `em_energy_conservation` -- Vacuum EM energy conserved (drift < 0.01% over 2000 ticks)
- `continuity` -- Charge conservation exact through all dynamics
- `poynting` -- Poynting vector S = c²(E x B) verified (direction, magnitude, symmetry)
- `larmor` -- Acceleration-dependent damping (power proportional to a^2)
- `em_fields` -- E/B field diagnostics, E perp B for propagating waves
- `lorentz` -- Magnetic/Lorentz-force checks consolidated with the Lorentz suite
- `selective_damping` -- Vacuum wave preservation, near-particle damping

**Poisson Coulomb (Phase 3):**
- `campaign_coulomb_force_law`, `energy_conservation`

**Energy Conservation (Phase 4):**
- `energy_conservation` (12 checks), `annihilation_conservation`

**Free Dynamics (Phase 5):**
- `campaign_free_dynamics` (10 checks), `particle_lifetime`

**Flux-Aggregate Particles (Phase 6):**
- `selffield_profile`, `wavepacket`, `campaign_bound_lifetime`

**Multi-Scale (Phase 7):**
- `particle_engine` (22 checks), `scale_bridge` (9), `campaign_hydrogen_spectrum`
- `campaign_cross_scale`, `campaign_born_ensemble`

**Atom Engine (Phase 8):**
- `atom_engine` (properties, closure context, forces, bonding), `atom_scale_bridge`, `campaign_h2_molecule`

**Dual Substrate:**
- `dual_substrate` -- Identity, chirality, conservation, backward compatibility

**Comprehensive logic engine:**
- `logic_engine` -- **42 checks** across 6 sections (Field Dynamics, Manifestation, Forces, Movement, Emergence, Lagrangian)

**Historical 10-Phase Proof-Out campaign snapshot (2026-03-16)** (125+
assertion checks):

`PASS` in this list means that the campaign executable's coded assertions
passed when this snapshot was recorded. It does not mean that a physical
quantity was derived from the substrate or independently confirmed.

- Phase 1: `campaign_statistical_convergence`
- Phase 2: current consolidated targets `campaign_dispersion`, `campaign_coulomb_force_law`, `campaign_wave_dynamics`
- Phase 3: current consolidated targets `campaign_quantum_correlations`, `campaign_born_rule` (the former `campaign_bell_substrate`, `campaign_epr_correlation`, and `test_entanglement` were merged into `campaign_quantum_correlations`)
- Phase 4: `campaign_hydrogen_binding`, `campaign_triad_energy`, `campaign_inertial_mass`, `campaign_structure_stability`
- Phase 5: historical `campaign_color_force`, `campaign_color_neutral`, `campaign_confinement`, `campaign_baryon_formation` executables; these names are retained as provenance and are not current CTest registrations
- Phase 6: `campaign_weak_transmutation`, `campaign_parity_violation`, `campaign_weak_decay`
- Phase 7: `campaign_gravitational_wave`, `campaign_gravity_profile`, `campaign_gravity_hierarchy`
- Phase 8: `campaign_triad_binding`, `campaign_neutrino_sector`
- Phase 9: `campaign_cosmological_predictions`
- Phase 10: `campaign_novel_predictions`

**Historical Phase-11 campaign assertions (2026-03-16):**
- `test_falsifiability` (12 checks) -- Wrong parameters produce wrong physics
- `campaign_integer_sweep` (7 checks) -- recorded one passing
  `{3,4,7,13}` combination among 315 tested, but one of its five gates is
  `floor(x_-)=N_C`; that physical identification was retired with FTD-0014.
  The sweep therefore cannot support a current uniqueness claim or grade.
- `campaign_hydrogen_spectrum` (8 checks) -- classical Kepler/virial
  consistency for an imposed `1/r` force (including the recorded 0.0004%
  radius error); generic to classical `1/r` dynamics, not evidence for a
  quantum spectrum or eigenvalue derivation (FTD-0270;
  `AUDIT_ATOMIC_DYNAMICS_STATUS.md`).
- `campaign_wave_dynamics` (historical two-slit checks consolidated here) -- Interference fringes from two coherent sources

**Readout admissibility (scale-context gate):**
- `scale_context` -- read-only, α-blind scale-context gate (`engine/src/scale_context.cpp`):
  per-regime classification (Evaporating / UVLocked / BoundedAdmissible /
  ShellDominated / Percolating), Φ-balance sign, and tracker stationarity. The
  module is external to `tick()` so the golden hash is unchanged. See
  `docs/theory/01_reference/SPEC_SCALE_CONTEXT_READOUT.md`.

**GPU/CUDA** (conditional on `FTD_ENABLE_CUDA`):
- `gpu_parity` -- As of 2026-08-18, `tests/test_gpu_parity.cpp` contains 24 `CHECK`/`CHECK_CLOSE` invocations covering SoA round-trip, single-tick, vacuum-wave, wavepacket, energy-audit, and Gauss comparisons. This is a source count; no current pass state is asserted here.
- `gpu_benchmark` -- GPU performance timing
- `gpu_physics` -- 26 campaigns, 100+ checks: GP-COULOMB, GP-GAUSS, GP-WAVE-SPEED, GP-ENERGY-LONG, GP-GRAVITY, GP-ANNIHILATION, GP-MAXWELL-AMPERE, GP-EM-ENERGY, GP-CONTINUITY, GP-KCOMP-SHELL, GP-WEAK, GP-COLOR, GP-STRONG, GP-TRIAD, GP-PAIRS, GP-EXCHANGE, GP-BOUNCE, GP-DUAL-SUBSTRATE. GP-GAUSS is a bounded production-residual campaign, not an exact FFT Gauss proof.
- `gpu_gauss_law_fidelity` -- Pins the production 18-point solve / 6-point correction-and-residual stencil contract
- `gpu_experiments` -- Extended GPU experiments (timeout: 1800s)

**Historical Five Minds campaign assertions (recorded 2026-04-05 in commit
`c1d2597b`)** (15/15 assertions reported PASS):
- `campaign_plato` -- Ontological faithfulness (dispositional ratio, genesis threshold, void energy)
- `campaign_einstein` -- Conservation & covariance (energy conservation, Lorentz contraction, gravitational redshift)
- `campaign_vonneumann` -- Computational convergence (Coulomb scaling, wave speed, hydrogen binding)
- `campaign_wigner` -- Symmetry (octahedral O_h, parity violation, CPT invariance)
- `campaign_grothendieck` -- Structural universality (color force running, scale bridge, alpha from scattering)

These names and counts are retained as campaign provenance. In particular,
hydrogen binding is classical, and an alpha/scattering assertion run with the
inserted runtime alpha cannot evidence the master quadratic (FTD-0792).

---

## 12. Key Design Decisions

1. **Field-mediated force implementation**:
   `F = -alpha*s*grad(phi_C) + G_N*grad(rho)` (Poisson, default), with no
   pairwise-force formula in this path. This states the implemented dynamics.
   Resulting observations are engine facts or `[MEASURED]` outcomes only where
   the LEDGER records them as such; any physical identification or
   engine/theory bridge retains its separate LEDGER tag and is not established
   merely because the coded dynamics produces a behavior.

2. **Damping hierarchy**: Default: uniform flux decay at rate alpha. With `selective_damping`: only near-particle sites damp. With `larmor_radiation` (requires `selective_damping`): acceleration-modulated damping proportional to a^2 (correct Larmor scaling).

3. **No self-field floor (Phase 4)**: Particles are naturally stable via coupling source g_c*grad(s). Removing the floor eliminated ~4146% energy injection.

4. **Manifestation-kinetics scales**: `K_MANIFEST := W_SC` is the adopted
   selection of FTD-0388, and `K_GENESIS = N_C*K_MANIFEST`. This records the
   selected engine contract; it introduces no new derivation claim and does
   not identify the kinetics scale with the imposed mass calibration `K_B`.

5. **Selected production wave speed**: `C_WAVE=1/sqrt(3)` is linearly stable but
   does not saturate the production 18-point stencil's exact CFL ceiling
   `C_WAVE²<=3/4` (FTD-0407). It is `[SELECTED]`, not derived from the live
   stencil. The FTD-0408 default-off period-two prototype has a different bare
   leading speed, `1/sqrt(13)`. The FTD-0411 selected BCC-time branch instead
   derives `1/sqrt(7)` from its chosen temporal kernel and q4 cancellation, but
   its live stable localization is not exact at q6.

6. **Tier-2 gravity gradient**: F_grav uses r=2 stencil to avoid self-field contamination.

7. **Neighborhood energy evaporation**: 7-site energy (particle + 6 face-neighbors) smooths the leapfrog oscillation; the rule is stochastic (since 2026-04-23) — survival is Boltzmann-weighted, p_evap = exp(-E_local/K_MANIFEST^2) * K_EVAP_RATE per tick.

8. **Gauss exclusion at particle sites**: ordinary projection skips manifested
   sites because centered `div(J)(i)` does not read `J(i)`. This is an
   implementation convention, not a physical theorem; the FTD-0426 live
   campaign measures the resulting operator-split fidelity directly.

9. **Poisson-based Coulomb**: SOR warm-started solver gives 1/r^2 force (exponent -2.25, isotropy 1.0). Replaces legacy double-gradient (exponent -3.8, isotropy 0.40).

10. **Sequential movement with moved_ guard**: Prevents double-processing after index-order moves.

11. **Lorentz magnetic force**: F = alpha*s*(v x B) does zero work (v*F = 0). Toggle-gated.

12. **E/B field decomposition**: E = -wave_vel, B = curl(J). The
Hamiltonian-consistent energy-flow diagnostic is S = c²(E x B).

13. **Backward compatibility**: Removed phase functions exist as no-op stubs. Removed toggles exist as deprecated fields. Removed Lagrangian terms return 0.

14. **Double damping is intentional (Rayleigh dissipation)**: Both `flux` and `wave_vel` are damped by `(1-ALPHA)` each tick in `phase_write`. This is deliberate Rayleigh dissipation -- it damps both the position-like degree of freedom (flux) and the velocity-like degree of freedom (wave_vel). Damping only one would leave undamped oscillatory modes. The dual damping ensures monotonic energy decay in the field, which is required for stable self-field buildup and physically correct radiation loss.

15. **Selected causal budget enforced by shared momentum integration** (FTD-0402 implementation; FTD-0403 targeted closure; TRACKER §1.2): stored velocity `u` is raw nodes/tick and `B=|u|²/C_SPEED²+L²<1`. CPU and GPU accumulate every enabled force contribution before one `P/M_INERTIAL` update; `|u|` asymptotes to `C_SPEED·√(1−L²)`. Movement-entry projection is a counted repair only for externally injected or directly mutated invalid velocities; ordinary force evolution produces zero repairs. On CPU, `RenderBridge::tick()` advances `tau` and optional de Broglie phase in its host phase ladder. On CUDA, `GpuEngine::record_tick_body()` advances both device-side; there is no common host proper-time post-pass. Exact, native, CUDA, golden, WASM/web, and compatibility gates close the frozen changed surface. This is exact conformance to the selected engine contract, not a theorem of Lorentz covariance.

16. **Explicit cubic cell measure** (FTD-0404): `VOXEL_EDGE_LENGTH=1`, `VOXEL_FACE_AREA=1`, and `VOXEL_VOLUME=1` are named in a CUDA-safe interface. CPU/GPU volume-density diagnostics integrate with `V_cell`; EnergyAudit also exposes the pre-integration field/wave density sums. Local latency gravity continues to read density, while point-particle and constraint channels are unscaled. The unit measure is numerically neutral; no force or update rule changes.

17. **Colour-energy contract is selected only on a default-off isolated v1 domain** (FTD-0405/0406): FTD-0405 remains the scoped no-go for the unmodified CPU/GPU direct-force tick and for any claim that its additive zero/localization were already derived. After explicit owner authorization, `strong_stress_energy` adopts `U_ij(r)=-c_f∫_1^r g(s)ds`, retains the existing force/movement position proposal, and deterministically projects only relative physical momenta so `K+U` and total momentum close on an unchanged coloured-particle topology. The same U enters EnergyAudit/EnergyLedger, midpoint-CIC string T00 plus Irving–Kirkwood central stress, and the latency source as gravitational mass density `T00/C_SPEED²`. The toggle defaults off. Native CUDA matches the CPU isolated colour sector (`gpu_strong_stress_parity`). Collision/state-transition, moving-latency, and mixed-force contracts remain open. The selected zero, localization and projection are not substrate theorems and derive no mass scale.

---

## 13. RenderBridge Public API

### Core

| Method | Description |
|--------|-------------|
| `tick()` | Advance one tick through the current toggle-gated phase ladder |
| `diagnostics()` | Returns `Diagnostics` struct (counts, flux totals, charge) |
| `energy_audit()` | Returns `EnergyAudit`: volume-integrated field/wave channels, their local-density sums plus `cell_volume`, exact normalized particle KE, particle rest/total energy, vector particle momentum, `dynamic_energy`, explicitly incomplete accounted `total_energy`, and Gauss violation |
| `energy_ledger()` | Returns `const EnergyLedger&` — per-tick conservation drift (auto-populated on CPU path). Tests assert `abs(.residual) < tol` to refuse energy-drift regressions. GPU: call `update_energy_ledger()` manually after a device→host sync. |
| `update_energy_ledger()` | Populate the ledger (called automatically by `tick()` on CPU path) |
| `inject_particle(x,y,z, state, flux_val)` | Inject single particle at lattice site (`flux_val` required; optional trailing `spin`, `color`) |
| `inject_wavepacket(x,y,z, state, sigma, amplitude)` | Inject Gaussian wavepacket |
| `inject_flux(x,y,z, fx,fy,fz)` | Raw flux injection (overwrites site) |
| `inject_flux_add(x,y,z, flux_val)` | Additive flux injection — accumulates instead of overwriting. Required by ported JS scenarios that sum overlapping Gaussians. |
| `inject_wave_vel_add(x,y,z, wv_val)` | Additive wave-velocity injection — same additive semantics, for wave-equation initial conditions. |
| `create_entangled_pair(x,y,z, flux_val)` | Pair production with shared event-pair tracking; the legacy API name is not proof of quantum entanglement |

### Diagnostics

| Method | Returns |
|--------|---------|
| `force_diag_at(idx)` | `ForceDiag` -- per-particle force breakdown (`force_diag()` returns the whole vector) |
| `em_field_at(idx)` | `EMFieldDiag {E, B}` |
| `poynting_vector(idx)` | `Vec3` (S = c²(E x B)) |
| `aggregate_profile(center, threshold)` | `AggregateProfile` (CoM, energy, r_eff, radial profile) |

### Configuration

| Method | Description |
|--------|-------------|
| `physical_time()` | Current tick * dt |
| `dt()` / `set_dt(val)` | Get/set timestep |
| `seed_rng(seed)` | Set RNG seed for reproducibility |
| `toggles` | Public `TermToggles` struct (43 boolean toggles + typed config fields) |

### Scenario library

`ftd::dispatch_scenario(RenderBridge& rb, const std::string& name)`
(declared in `include/ftd/scenarios.h`) is the public C++ entry point for
scenario setup. The thin router and shared RNG live in `src/scenarios.cpp`;
scenario bodies are split by responsibility under `src/scenarios/`. It is a
straight port of the browser-side JS scenario library under
`engine/web/js/bridge/scenarios/` — the two code paths stay in lockstep
so that WASM, CLI, and native hosts all seed the lattice identically.

After handling the exact `empty` baseline, dispatch tries six prefix groups in
order and returns `true` on the first match:

1. `flux-*` — pure-flux field initial conditions
2. `light-*` — photon-like wavepackets and coherent-state probes
3. `quantum-*` — superposition, entanglement, and measurement setups
4. `s0-vacuum-*` — vacuum particle-candidate presets
5. `s0-seed-*` — Scale-0 manifested-particle seeds
6. `s0-field-*` — Scale-0 background-field presets

Returning `false` means no prefix matched; `wasm/ftd_wasm.cpp` falls
through to its legacy scenario `switch` for backward-compatibility with
older scenario names still referenced by UI code. The scenarios use the
new additive injectors (`inject_flux_add`, `inject_wave_vel_add`)
because many of them accumulate overlapping Gaussians and cannot use
the overwriting `inject_flux`.

---

## 14. CUDA GPU Engine

The GPU engine (`GpuEngine`) is a drop-in alternative to `RenderBridge`. All field data resides on the device; host transfers only diagnostics.

### Architecture

```
Host (CPU)                          Device (GPU)
inject_particle()  ---upload--->    d_state, d_flux_*, ...
inject_wavepacket()                 d_wave_vel_*, d_velocity_*
                   <--download---
diagnostics()                       tick() loop:
energy_audit()                        1. phase_read
sync_to_host()                        2. phase_write
                                      2b. pair production [optional]
                                      3. gauss / coulomb / latency solves
                                      4. forces + optional particle sectors
                                      5. movement
                                      6. weak/triad/proper-time extensions
```

### FFT Poisson Solver

Replaces CPU's iterative SOR with spectral method via cuFFT:
- **Not exact — a structural floor, not a precision limit.** The GPU Green's
  function is the 18-point Laplacian symbol (`kernel_precompute_green` in
  `gpu_buffers.cu`) while the correction applied is a 6-point central difference
  (`gauss_correction_kernel` in `kernels_poisson.cu`)
  and the residual is measured with the matching 6-point divergence. Over the
  L=24 Brillouin zone the surviving per-mode factor |1 - S_DG/S_18| has median
  ~0.62 (min 0.006, p90 0.84, max 1.0): the projection leaves most of each mode's
  residual standing, and this is iteration- and precision-independent. The engine
  already documents this same stencil-mismatch floor honestly for the CPU at
  `constants.h:331-347` and `eft/matched_poisson.h:7-19`.
- The ~1e-8 constraint figure belongs to the `[TOOLING]`-tagged matched-stencil CG
  solver, which is explicitly NOT the production Gauss path; it does not describe
  the cuFFT solver.
- **Single-pass**: No iteration count to tune
- Precomputed Green's function reused every tick

### Backend discrete-outcome/missing-term table

Several former CUDA gaps are now closed in live source:

| Term / rule | Current CUDA behavior | Scope note |
|---|---|---|
| `field_energy_gravity` | `compute_latency_rhs` includes local flux/wave energy | Uses the CUDA FFT latency solver rather than CPU SOR |
| `exact_dual_gauss` | `gauss_correction_kernel` corrects manifested sites when enabled | The remaining Gauss limitation is the documented stencil-mismatch floor |
| `absorbing_boundary`, `flux_boundary` | Native post-movement absorbing and reflective/dispersal kernels | No longer degrade silently to periodic |
| `ew_background_sweep` | Native pre-read drive | Deliberately graph-ineligible |
| `verlet_wave_integrator` | Native KDK: half-kick + drift in `phase_write`, post-drift `phase_read`, second half-kick | Honors `dt<1`; default OFF; golden-neutral |
| `lorentz_period2_floquet`, `lorentz_bcc_time_floquet` | Native period-two wave kick from live `d_tick` | Unit tick; default OFF; golden-neutral |
| `symmetric_movement_order` | Native serial commit walks a SplitMix64 Fisher-Yates permutation from live `d_tick` | Default OFF; golden-neutral; same RNG as CPU |
| `confinement` | Native colour-force r≥8 shell: `F = SIGMA_STRING · cf` when on | Requires `color_forces`. [SELECTION], not FTD-0025. Default OFF; `color_forces` alone stays harmonic |
| `strong_force`, `exchange_force` | Native Yukawa / same-spin exchange; CPU uses the same helpers | Default OFF; golden-neutral |
| `cluster_inertia` | Native serial 26-Moore flood-fill after integrate, before movement | Needs a force channel (EM, colour, Yukawa, or exchange). `F_cluster` includes exchange. Default OFF; golden-neutral |
| `strong_stress_energy` | Native remainder colour, 1-thread Hamiltonian projection, CIC T00; latency RHS uses T00/c² | Isolated colour sector; default OFF; FTD-0406 |
| `matched_gauss_dynamics` | Native Faraday/Ampere + ledger current; skips the legacy wave writer; host CG init once | Isolated conservative-movement sector; default OFF; FTD-0428 |
| `triad_binding` | Native detection after movement + weak, on a rebuilt particle list | Matches CPU Rule 7 order |

Default-order particle movement (P11) is a serial CUDA commit with a `moved[]` arrival guard, matching CPU lowest-index-wins. `symmetric_movement_order` shuffles that traversal on both backends.

The toggle bitmask is **not** the drift: `term_toggles.h` declares it
informational. Live implementation class lives in `gpu_term_contract.h`.

**Numerical parity note:** CPU and CUDA use different Poisson solvers (SOR
versus FFT), but neither production Gauss projection is exact: both combine an
18-point Poisson operator with a 6-point correction/residual operator and
therefore retain the structural stencil-mismatch floor described above.
The `gpu_parity` campaign has historically printed a displayed residual with
the label `(FFT, exact)`; that label describes neither an exact production
Gauss inverse nor removal of the mismatch floor. A displayed zero from that
campaign's residual meter is likewise not evidence that the full production
constraint is exact. The dedicated `gpu_physics` GP-GAUSS gate instead requires
the long-run maximum residual to remain in its documented nonzero bounded band,
and `gpu_gauss_law_fidelity` pins the operator mismatch directly.
Benchmarks must compare against the backend-specific contract rather than
assuming roundoff-level CPU/CUDA agreement.

### SoA Memory Layout

~200 bytes/voxel (26+ separate device arrays for coalesced access). At 128^3: ~400 MB.

### Build

```bash
# CUDA is ON by default in the canonical engine/build tree (FTD_ENABLE_CUDA
# native default; --allow-unsupported-compiler + arch 89;120 are set by
# CMakeLists). MUST use MSVC 14.44 -- the wrapper pins it via vcvars:
engine\build_native.bat
```

The legacy separate `engine/build_cuda` tree is retired; the deprecated `build_cuda.bat`/`build_cuda_fast.bat`
delegation shims were themselves deleted 2026-08-18 — use `build_native.bat` directly.

Requirements: CUDA 13.0+, compute capability >= 8.9. Target architectures: "89;120" (Ada + Blackwell).

### Benchmarks (GPU)

| Lattice | CPU (ms/tick) | GPU (ms/tick) | Speedup |
|---------|---------------|---------------|---------|
| 16^3 | -- | -- | 18.6x |
| 32^3 | -- | -- | 41x |
| 48^3 | -- | -- | 193x |
| 64^3 | 134 | 0.37 | not recorded |

### GPU Physics Campaigns

26 campaigns and 100+ checks exercise CUDA behavior at large lattice sizes.
They do not establish blanket CPU/GPU parity; the §14 backend table governs.

| Campaign | Lattice | Key Result |
|----------|---------|------------|
| GP-COULOMB | 128^3 | Force exponent -2.067, R^2=0.9999 |
| GP-GAUSS | 128^3 | Production maximum residual remains in the documented nonzero partial-projection band; total charge preserved over 1000 ticks |
| GP-WAVE-SPEED | 128^3 | Axial 0.700 voxel/tick (1.21x CFL) |
| GP-ENERGY-LONG | 64^3 | 50K ticks, max drift 4.96%, charge exact |
| GP-GRAVITY | 128^3 | 20 particles, RMS shrinkage 12.6% |
| GP-ANNIHILATION | 64^3 | 20->2 particles, Q=0 exact |
| GP-MAXWELL-AMPERE | 128^3 | Standing wave E/B verification |
| GP-EM-ENERGY | 64^3 | Undamped vacuum bounded oscillation |
| GP-CONTINUITY | 128^3 | 10 pairs, Q=0 at all checkpoints |
| GP-DUAL-SUBSTRATE | 64^3 | Identity 3e-16, partition, backward compat |
| GP-KCOMP-SHELL | 128^3 | K_comp volumetric shell 10/10 |
| GP-BOUNCE | 64^3 | Same-sign elastic bounce verified |
| GP-WEAK/COLOR/STRONG/TRIAD/PAIRS/EXCHANGE | 64^3 | Toggle-gated physics extensions |

### Stable source paths

The generated `docs/ENGINE_FILE_MANIFEST.json` is authoritative for the current
file inventory and line counts. Stable CUDA responsibilities are:

| Path | Responsibility |
|------|----------------|
| `include/ftd/gpu_engine.h` | `GpuEngine` orchestration interface |
| `include/ftd/gpu_buffers.h` | Structure-of-arrays device-buffer contract |
| `cuda/gpu_buffers.cu` | Device allocation and host/device transfer |
| `cuda/gpu_engine.cu` | Tick orchestration, synchronization, and graph capture |
| `cuda/kernels_stencil_single.cu`, `cuda/kernels_stencil_dual.cu` | Single- and dual-substrate field read/write kernels |
| `cuda/kernels_poisson.cu` | FFT Poisson paths |
| `cuda/kernels_forces.cu` | Force, particle-list, triad, and movement kernels |
| `cuda/kernels_proper_time.cu` | Device proper-time and phase advance |
| `cuda/kernels_gauge.cu` | SU(2)/SU(3) link relaxation |
| `cuda/CMakeLists.txt` | CUDA source registration and build rules |

---

## 15. Web UI (Browser Dashboard)

The C++ engine compiles to WASM via Emscripten. The browser dashboard provides zero-install access with Three.js 3D visualization.

### Architecture

```
ftd_core (C++ library)
    |
    +-- WASM Bindings (wasm/ftd_wasm.cpp, Embind)
    |       |
    |       +-- Browser Frontend (web/)
    |           +-- Three.js 3D viewport
    |           +-- Canvas 2D charts
    |           +-- Vanilla JS (ES modules, zero build step)
    |
    +-- CLI (src/main.cpp, native)
```

### Windows desktop shell

`engine/desktop/` provides the first-class Windows interface. It is a WPF
application with an embedded WebView2 surface; a small in-process Kestrel host
serves `engine/web` on loopback without cache, and `EngineHost` supervises the
canonical WSL2 `engine/build_wsl/ws_server` process. The WebSocket bridge accepts
the desktop-supplied `?wsPort=<port>` query parameter (9100 remains the browser
default).

The desktop status is runtime-accurate: `ws_server` reports the active
`RenderBridge::backend_kind()` as `backend: "cuda"` or `backend: "cpu"`, along
with the single-sourced `ENGINE_VERSION`. The shell refuses a CPU response
instead of inferring GPU availability from `FTD_ENABLE_CUDA`. This changes no
physics path and does not broaden the per-toggle GPU support matrix described
in Section 14.

The native Scale-0 socket protocol is load-bounded. `tick` and `run` return
typed completion messages, and `WebSocketBridge` permits one simulation command
in flight. Real-time animation ticks coalesce to one pending follow-up when CUDA
cannot match the display cadence; paused Step/+N requests remain exact. A Pause
also cancels pending playback demand. Flux-volume visualization uses a binary
`FTV1` header plus uint32 count and float32 magnitudes, avoiding the main-thread
cost and bandwidth of an N^3 decimal JSON array. Native Scale 0 is single-owner:
its scenario/toggle/injection commands are not mirrored into the lazy WASM
fallback, which is reserved for the standalone Scale 1/2 engines.

Large-lattice construction is resource-gated and transactional. The server
samples available WSL2 host RAM and CUDA memory, reserves safety headroom, and
reports the estimate through `preflight_resize`. `resize_scenario` combines the
formerly duplicated resize/reset/setup allocations into one candidate bridge;
the active bridge is replaced only after allocation and scenario dispatch both
succeed. CUDA/cuFFT failures throw through this boundary and become correlated
WebSocket errors rather than process-wide `exit`/`abort` calls. Request IDs keep
those errors from resolving an unrelated asynchronous diagnostic request.

Interactive CUDA ticks keep the device authoritative and use selective visual
readback: one byte per voxel for ternary state and one float32 per voxel for
flux magnitude. Full AoS synchronization remains available to explicit
diagnostics and scientific/audit callers, while the desktop render loop no
longer transfers hundreds of bytes per voxel after every tick. During active
playback at `L >= 113`, the web bridge defers those full scientific snapshots
until the current CUDA work drains. Binary manifested-particle frames are
deterministically sampled to at most 300,000 visual points, bounding a frame at
about 8.4 MiB while leaving the authoritative simulation unchanged. The default
non-interactive `RenderBridge` behavior is unchanged for tests and campaigns.

### Dashboard Layout

```
+----------------------------------------------------------------+
|  FTD Engine v2.18     [Engine ▼]                     []       |  Toolbar
+----------------------------------------------------------------+
|                                    [Visualization ▾]           |  Overlay (collapsible)
|                                     VOLUME  FIELDS  FORCES     |
|                                     QUANTUM PHENOMENA          |
|                                                                 |
|              Three.js 3D Viewport                               |  ~60%
|         (particles, wireframe, field overlays)                  |
|                                                                 |
|   ┌──────────────────── Scrub Bar ────────────────────┐         |
|   │ [] [▷] [⏵] [↺] │ Speed─●─ │ ⟲ [──timeline──] t  │         |
|   │  global  local            │       Render      │         |
|   └─────────────────────────────────────────────────────┘      |
+----+----+-----+----+----+----+----+----+-----------------------+
| Ctrl|Diag|Chart|Lag |Insp|Zoo |Hrk |QL  | Dock tabs            |
+----+----+-----+----+----+----+----+----+-----------------------+
|                Active Tab Panel                                 |  ~35%
+----------------------------------------------------------------+
| Running | Tick: 1,234 | Particles: 12 | 60 fps                  |  Status
+----------------------------------------------------------------+
```

Key changes from v2.11:
- **Toolbar** now hosts only branding, the Engine (scale) selector, and Settings. All playback controls moved to the floating scrub bar.
- **Floating Scrub Bar** (`js/ui/components/scrub-bar/`) — a 44-px glass pill at the viewport bottom with four semantic sections:
  1. *Controls*: global play (pill, accent fill) · local play (outline square, pulses when local-paused-global-running) · step · reset. Captions `global` / `local` beneath.
  2. *Speed*: uppercase `SPEED` · 90-px range slider · mono tick-per-frame readout.
  3. *Timeline*: reset-playhead button · LOD-shaded memory strip (sharp / blurry / static) · green render band on the right when a clip is present · time badge.
  4. *Actions*: `● Render` button and a settings kebab.
- **Overlay panel** (visualization toggles) has a chevron collapse affordance in its header that persists per-scale in localStorage (`ftd.overlay.scale0.collapsed`, etc.).
- **Panel dock** (bottom tabs) supports `data-panel-mount="bottom|left|right"` and `data-panel-width="narrow|normal|wide"` via the pre-paint hydration script in `<body>`.

### Playback Timeline (working-memory + render mode)

The scrub bar is backed by two capture strategies that share a single `TimelineBuffer` primitive (`js/scales/scale0/timeline/`):

- **MemoryRecorder** — live rolling window with LOD-tiered age decay. Snapshots enter at LOD 0 and are progressively block-averaged to LOD 1 (2× downsample) / LOD 2 (4×) / LOD 3 (audit-only) as they age across tier boundaries. Tier schedule auto-derives from a user-configurable byte budget (default 30 MB, ≈ 27 s of window at a 32³ lattice).
- **RenderController** — offline dense capture. User clicks the Render button; the controller runs ticks in ≤ 12 ms idle slices (`setTimeout(0)`) while sampling every `sampleEveryTicks = 4` ticks (15 fps @ 60 TPS). A budget-aware LOD picker selects the coarsest LOD (0 / 1 / 2) whose byte-cost × sample-count fits the render budget, then the whole clip is captured at that LOD — guaranteeing a dense, uniformly-sampled buffer for smooth forward and backward scrubbing. Emits `start / progress / done / cancel / error`. Cancellation restores the original engine state; partial clips are discarded.

Hydration uses two Scale 0 bridge capabilities:
- `getScale0Snapshot()` → `{ tick, lod, lattice, flux, wave, particles, audit }` (copies of MockBridge's `_stateGrid`, `_fluxJ`, `_fluxWV`, `_particles`).
- `loadScale0Snapshot(s)` — writes arrays back into the engine buffers. Accepts **any LOD**; LOD 1/2 inputs are upsampled nearest-neighbor to N³ before write (the JS-side `timeline/lod.js#upsampleScalar / upsampleVec3` helpers are published on `window.__ftdTimelineLod`). LOD 3 is telemetry-only and rejected.

Scrubbing is a pure "load, don't re-simulate" operation: `hydrateToTick(tick)` picks the nearest snapshot by tick from the render buffer (if an active clip exists) else the memory buffer, and loads it directly. No fast-forward ticks run during a drag, so the cost per scrub frame is one upsample + one buffer write — latency is independent of scrub distance. Pointer moves are coalesced to one hydrate per animation frame via `requestAnimationFrame`, so 240 Hz trackpads cannot saturate the loader. Live simulation resumes on pointerup (`onScrubEnd`).

### Panels

The three Scale 0 dashboard tabs are built on a shared chart/table primitive set:

- **Charts primitives** (`js/ui/charts/`): vendored uPlot 1.6.30, a theme reader that maps CSS custom properties into uPlot config, and three primitive classes:
  - `UPlotChart` — line/area using bulk `flattenInto()` extraction from SoA MultiRingBuffers for O(1) contiguous typed-array render passes. DPR + ResizeObserver handling, localStorage-persisted series-hidden state.
  - `Sparkline` — axis-free micro chart for table Trend cells.
  - `StackedAreaChart` — custom `paths` renderer that cumulatively sums same-x points across series.
- **Diagnostics panel** (`js/ui/panels/diagnostics-panel/`): descriptor-driven `<table>` sections with `Metric | Value | Unit | Trend` columns, tabular-nums typography, zebra striping, digit-change pulse animation, and inline sparklines per row. The single Scale 0 descriptor declares 5 sections × 27 rows with physics-accurate units (`ct`, `E*`, `|J|`, `nat`, `|S|`, `ℏ`, `E*²`, `|w|²`).
- **Charts panel** (`js/ui/panels/charts-panel/`): horizontally-scrollable chip picker + auto-fit card grid. Chip toggles fully destroy / recreate chart cards — no leaked uPlot instances. Active-chart set persists in localStorage (`ftd.charts.active`).
- **Lagrangian panel** (`js/ui/panels/lagrangian-panel/`): StackedAreaChart with 7 bands · term-row checkboxes that two-way sync with the uPlot legend · `Action & Constraints` + `Ontic Constants` sidecar tables reusing `DiagnosticsTable`.

All three panels read live data from `TelemetryHub` (`js/telemetry-hub.js`), which utilizes `MultiRingBuffer` (Structure-of-Arrays) allocations across all 5 scales. Core buffers (`hub._s0_core`, `hub._s0_aud`, `hub._s1_pe`, etc.) are populated via unified `.push()` objects. The `WasmBridge` bypasses Embind object allocations by extracting native `Float64Array` zero-copy views (`getDiagnosticsView`, `getEnergyAuditView`, `getLagrangianView`) directly from the WASM engine.

### Scenarios (23+)

**Scale 0 (Lattice):** Flux Pulse, Dipole, Proton+Electron, Genesis Cascade, Damping Demo, 4-Source Interference, Flux Vortex, Particle Collision, Pair Production, Hydrogen Atom, Gravity Cluster, Random Genesis, Rainbow, Lattice Prism, Dipole Radiation, Two-Slit, Photon Race, Dual Substrate, Entangled Pair, Annihilation, Force Law Profile

**Scale 1 (ParticleEngine):** Leptons: Hydrogen, Helium, Positronium, Muonium, True Muonium, Tauonium, Tauonic Hydrogen. Exotic Atoms: Pionic H, Kaonic H, Σ⁺ Atom, Protonium. Hadrons: Pionium, Kaonium, Δ⁺⁺ System, Ω⁻ Scattering. Nuclear: Deuteron, Tritium, Helion. Bosons: W⁺W⁻ Pair. Scattering: p-e, Three-body, π⁺-p, μ⁻-p. Custom. (23 scenarios)

**Scale 2 (AtomEngine):** Individual elements (118), Periodic Table. Noble Gas Clusters: He/Ar/Mix. Ionic Formation: NaCl/MgF₂/Lattice. Covalent Formation: H₂/O₂/CH₄. H-Bonding: Water Dimer/Pentamer. VSEPR Geometry: CO₂/CH₄/H₂O. Thermal Dynamics: Gas/Collision. Metallic Clusters: Fe BCC/Cu FCC. Custom. Phase 3 forces (JS MockBridge): H-bonds, angle strain, dipole-dipole, thermostat, electronegativity. Scale 3 molecules: 25-molecule library + NaCl Crystal

### Field Visualization Overlays (5 categorical groups)

The Scale 0 overlay panel is organised into five semantic columns; each column groups related toggles so the flat "9 keys" layout no longer scales. Hidden by default behind a collapse chevron; state persists per scale in `ftd.overlay.<scale>.collapsed`.

| Column | Toggles |
|--------|---------|
| **Volume** | Flux Volume (points), Flux Slice (XZ plane), Flux Lines (streamlines), ∇·J (divergence source/sink heatmap) |
| **Fields** | E Field, B Field, Poynting S, Light (photon bloom from \|S\|) |
| **Forces** | Force style selector (Arrows / Heatmap / Flow / Glyphs) applied to: EM, Gravity, Strong, Weak |
| **Quantum** | \|ψ\|², Phase φ, ℒ(x), Entropy s, Φ potential |
| **Phenomena** | Dual J, Chirality, DM Halo, Genesis, Damping, Confinement |

The Weak force shares the force-style selector but its "Arrows" mode renders additive-blended radial sprites (`PointsMaterial` + CanvasTexture gradient), not arrows — transmutation sites pulse along the intensity palette.

### Scale 2/3 Atom & Molecule Visualization (6 features)

Enhanced pedagogical visualization for Scale 2 (atoms) and Scale 3 (molecules):

| Feature | Implementation | Controls |
|---------|---------------|----------|
| **Enhanced nucleus** | Denser proton/neutron clouds (8 pts/nucleon), white center glow, larger radius | Always on |
| **Strong force shells** | Translucent orange InstancedMesh spheres (100 pool), AdditiveBlending, radius = 0.5 × cbrt(A) × 1.8 | Shells checkbox (default ON) |
| **Thick styled bonds** | CylinderGeometry InstancedMesh (1500 pool) with single/double/triple order support, CPK-blended colors | Bond style dropdown (Thick/Thin/Off) |
| **Bonding electron clouds** | Gaussian ellipsoidal point clouds along bond axes (8 × order points per bond, light cyan) | Clouds checkbox |
| **Orbital shell boundaries** | Translucent spheres per principal quantum number using Slater Z_eff (n=1 blue, n=2 green, n=3 orange, n=4+ pink) | Bounds checkbox (default OFF) |
| **Shaped orbital lobes** | Elongated ellipsoid InstancedMesh (2000 pool) for p/d/f valence orbitals, AdditiveBlending | Lobes checkbox (default OFF) |
| **Per-atom force arrows** | 4 LineSegments sets: Coulomb (red), vdW (green), Bond (orange), Net (white), log-compressed scaling | F_C / F_vdW / F_B / F_net toggle buttons |

Force decomposition computed via `aeGetForceDecomposition()` in MockBridge (ionic, vdW, bond, net). Arrows updated every 2nd frame for performance. All features auto-hidden on Scale 0/1 transitions via CSS `scale23-only` class and `setEngineMode()` cleanup.

### Boundary Containment (7 shapes)

Cube (periodic), Sphere, Octahedron, Dodecahedron, Icosahedron, Cylinder, Torus, None.

### Environment Backgrounds (6)

None, Star Field (default), Nebula, Quantum Foam, The Beyond, Flux Storm.

---

## 16. Dual-Substrate Mode

When `toggles.dual_substrate = true`, the single flux field J is replaced by two independent substrates J_L and J_R:

- **Observable**: psi = J_L + J_R (maintained automatically)
- **Chirality**: phi = J_L - J_R
- **Splitting**: delta^2 = (4G*-1)/(4G*) ≈ 0.9155; DELTA_APPROX ≈ 0.9568

**CPU implementation**: Independent Laplacians and leapfrog for L/R in phase_read/write. Gauss sync distributes correction equally.

**GPU implementation**: Dedicated dual kernels (`phase_read_dual_kernel`, `phase_write_dual_kernel`, `gauss_sync_dual_kernel`). Identity J = J_L + J_R maintained to machine precision (3.19e-16).

---

## 17. Historical 10-Phase Campaign Snapshot (2026-03-16)

This table preserves the proof-out campaign record as it stood on 2026-03-16.
All ten phases' campaign executables reported their 125+ coded assertions as
passing. Here
`PASS` means only **the assertions passed**; it is not a `[THEOREM]`,
`[DERIVED]`, `[EMERGENT]`, or external-confirmation verdict. Current claim
status comes from the LEDGER, not from this historical scorecard.

| Phase | Campaign | Checks | Result |
|-------|----------|--------|--------|
| 1 | Statistical convergence | 5/5 | PASS |
| 2 | Continuum limit | 15/15 | PASS |
| 3 | Bell/Born assertions (mixed native and imported-reference checks) | 18/18 | PASS |
| 4 | Mass spectrum | 20/20 | PASS |
| 5 | Color dynamics | 16/16 | PASS |
| 6 | Weak sector | 12/12 | PASS |
| 7 | Gravitational sector | 13/13 | PASS |
| 8 | Particle Zoo | 13/13 | PASS |
| 9 | Cosmological predictions | 6/6 | PASS |
| 10 | Novel-prediction/falsifiability assertions | 7/7 | PASS |

### Interpretation boundaries

- **Bell:** the native deterministic/commutative substrate campaign is a
  local-hidden-variable check and satisfies `S <= 2`. Standalone cosine,
  singlet, and `2*sqrt(2)`/Tsirelson checks are imported QM or mathematical
  identities; they do not establish a lattice Bell violation. The physical
  `S=2*sqrt(2)` identification remains `[SELECTION]` (FTD-0023), and FTD-0347
  records the benchmark re-grading.
- **Hydrogen:** the campaign's Kepler radius and virial/`1/n^2` checks use
  classical `1/r` dynamics. They are generic classical consistency checks,
  not quantum-spectrum or eigenvalue evidence (FTD-0270). The later FTD-0278
  result is separately `[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]`, narrowed
  to its 1s statement; it does not upgrade this historical campaign.
- **Alpha:** `ALPHA = 1/X_PLUS_PRECISION = 1/137.035999177` is the inserted
  CODATA-fitted runtime value. `ALPHA_TREE = 1/X_PLUS` is reference-only and
  has no production force, wave, or damping consumer. Engine agreement
  therefore cannot evidence the master quadratic (FTD-0792). The master
  quadratic itself is `[THEOREM]` algebra; `x_+ = 1/alpha` remains
  `[STRONGLY MOTIVATED CONJECTURE]` (FTD-0013).
- **Integer sweep:** the historical 315-combination result required
  `floor(x_-)=N_C`. That identification (FTD-0014) is retired, so this sweep
  cannot support a current uniqueness grade.

### Historical comparison values

The numbers below are retained verbatim as campaign provenance. Their presence
does not override their current LEDGER tags or make them engine derivations.

| Observable | Historical FTD value | Comparison value | Recorded precision |
|------------|----------------------|------------------|--------------------|
| 4-term 1/alpha | 137.035999177 | 137.035999177(21) | **0.325 ppt** |
| Spectral index n_s | 0.9645 | 0.9649 +/- 0.0042 | **0.096 sigma** |
| sin^2 theta_W | 3/13 = 0.2308 | 0.2312 | **0.19%** |
| alpha_s(M_Z) | 7/59 = 0.1186 | 0.1179 +/- 0.0009 | **0.63%** |

### Six historical falsification criteria

1. No fourth generation of fermions with standard gauge couplings
2. Normal neutrino mass hierarchy (not inverted)
3. Proton decay with tau_p ~ 10^35 years
4. Tensor-to-scalar ratio r ~ 0.022
5. No WIMPs, no supersymmetry, no extra dimensions
6. Digit 13 of 1/alpha = 0

---

## 18. Historical Mechanisms and Outcomes Snapshot (2026-03-16)

The previous heading grouped implemented rules and measured responses under one
epistemic label. The split below preserves the historical observations without
assigning a current epistemic tag.

### Implemented or imposed mechanisms

| Mechanism | Historical implementation fact |
|-----------|--------------------------------|
| Electromagnetic attraction/repulsion | The Poisson Coulomb force law is coded with inserted `ALPHA`; charge signs select direction |
| Gravity attraction | The attractive density-force branch is coded with imposed lattice `G_N` |
| Pair production | The lifecycle extension creates `+/-` pairs when its coded threshold is crossed |
| Wave propagation | The wave update propagates pulses using selected `C_WAVE` |
| Gauss projection | The solver projects `div(J)` toward its configured target |

These rows establish that mechanisms are implemented. They are not, by that
fact alone, measured emergent outcomes or substrate derivations.

### Historical measured outcomes

| Outcome | Recorded observation |
|---------|----------------------|
| Force profile | Poisson Coulomb exponent -2.25 (CPU), -2.067 (GPU) |
| Force isotropy | Ratio 1.0 at r=5 |
| Short-lived binding | Opposite charges survive 300+ ticks |
| Interference | Two coherent sources create fringes |
| Self-field response | Coupling source builds a steady-state EM envelope |
| Causal support | No flux observed beyond `C_WAVE * ticks` |
| Numerical energy accounting | 0.01% drift (Scale 0), 10^-10% (Scale 1) |

### Historical unresolved observations

- Spontaneous triad formation without binding code -- not observed.
- Stable orbits with radiation damping -- electrons spiral outward in the
  recorded setup; this is an observation, not a correctness confirmation.
- Sub-ppm alpha precision from higher-order corrections -- not demonstrated
  in the engine; the runtime instead uses the inserted precision alpha
  described above.

This list is provenance, not the current open-work register. Consult
`docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md` for current open
work.

---

## 19. Current Scientific-Status Boundaries

The obsolete overall `C+` and per-category letter grades are retired from this
specification: they mixed software assertions, engine measurements, imported
physics, and theory claims into one scale.

- **Claim status and epistemic tags:**
  `docs/theory/07_assessment/core_ledgers/LEDGER.md`
- **Current open work:**
  `docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md`
- **Engine/theory call-path and implementation boundary:**
  `docs/theory/07_assessment/engine_infrastructure_rg/AUDIT_ENGINE_CALLSTACK.md`
- **Atomic/hydrogen boundary:**
  `docs/theory/07_assessment/engine_emergence_campaigns/AUDIT_ATOMIC_DYNAMICS_STATUS.md`
- **Alpha extraction and runtime-input boundary:**
  `docs/theory/07_assessment/audits/AUDIT_ALPHA_EXTRACTION.md` and LEDGER
  FTD-0792
- **Bell benchmark re-grading and imported-QM boundary:**
  LEDGER FTD-0347 (with FTD-0023 for the physical identification)

Historical campaign counts and numbers remain in §§11, 17, and 18 for
provenance; they do not define current scientific status.

## CUDA tick execution model (Component A, 2026-08-17)

`GpuEngine::tick()` no longer performs any host/device round trip. Its
contract:

- **Stream.** `GpuBuffers::stream` is a blocking stream created in
  `allocate()` and drained/destroyed in `free()`. Every tick-path kernel,
  memset, D2D copy, CUB call and cuFFT plan is bound to it. Everything not
  migrated (compact diagnostics, injection kernels, AoS downloads, visual
  capture) stays on the legacy default stream and remains correctly ordered
  because a blocking stream implicitly synchronizes with the legacy stream.
- **No blocking reads.** Poisson `mean_charge` is device-resident
  (`d_poisson_mean_charge`); the pairwise/triad launches are fixed-capacity
  (`MAX_PARTICLES = 8192` threads, device-side bound, populated via a
  deterministic `cub::DeviceSelect::Flagged` compaction rather than an
  atomic-ordered scatter) instead of being sized from a host readback of
  `d_num_particles`; `reset_continuity_ledger()` no longer calls
  `cudaDeviceSynchronize()`; the ledger, force-diag and movement resets use
  `cudaMemsetAsync`.
- **Capacity overflow** is a sticky device flag (`d_particle_overflow`) set by
  the particle-list compaction path and surfaced as the same
  `std::runtime_error` at `ensure_host_synced()` / `causal_projection_events()`
  — i.e. at synchronization boundaries that already copy scalars, never in
  the tick.
- **Tick counter.** `GpuBuffers::d_tick` mirrors `GpuEngine::tick_`. Every
  RNG-salted kernel reads it through a pointer so a replayed graph advances
  its SplitMix64 streams exactly as a direct-launch tick does.
- **Graph capture.** `graph_capture_enabled` (default true) captures the tick
  body once per `graph_key()` — a hash of every topology toggle plus every
  host-derived scalar kernel argument except the tick — and replays the
  cached `cudaGraphExec_t` thereafter. Capture uses
  `cudaStreamCaptureModeThreadLocal` — chosen as a defensive, forward-looking
  choice safe even if a future caller becomes multi-threaded; the stream
  being a *blocking* stream (see above) is what makes any stray
  legacy-stream or allocating call fail the capture loudly, so an
  un-migrated kernel is caught rather than silently baked into a wrong
  graph. A failed capture caches a null exec and falls back to direct launch
  for that key permanently. `ew_background_sweep`'s drive is device-resident
  (read from `d_tick`, not host-computed) and could technically be captured;
  it stays graph-ineligible only because it's a research toggle outside this
  task's tested profiles, revisit if a future profile needs it. `su2_gauge` /
  `su3_gauge` are graph-ineligible for the same class of reason:
  `gpu_gauge_relax()`'s src/scratch ping-pong is a host-side `std::swap` of
  device pointers, which stream capture cannot record, so a captured graph
  would keep replaying the one buffer pairing baked in at capture time
  instead of alternating every tick.
  `test_gpu_graph_capture` gates bit-identity between replay and direct
  launch across four toggle topologies, plus cache-eviction correctness
  past `MAX_GRAPH_CACHE`, plus (G8) that the gauge sector never enters the
  graph cache.
- **Out of scope of the graph.** `GpuBackend::tick()`'s post-tick
  `causal_projection_events()` D2H stays outside the captured region: it reads
  a result of the completed tick and is the native app's per-tick completion
  fence.
