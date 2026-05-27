# Open Items Tracker

Central ledger of every `[OPEN]` claim in FTD — code stubs, theoretical gaps, unresolved verification tasks, and research questions. One place to look when picking work, one place to update when an item closes.

**Canonical path:** `docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md`.

**Last full audit:** 2026-04-17 (post-engine-cleanup). **Last incremental update:** 2026-04-28 — §7.7 refined: "WHY 25 voxels?" item closed at linear level by FTD-0110 [DERIVED]; "structural bridge between algebraic spine and engine phenomenology" item closed at linear level (one connector found); new sub-item added for the linear→nonlinear bridge proof. (Previous: 2026-04-27 evening — added §7.7 with three high-leverage post-engine-as-instrument priorities.) **Live repo count:** ~202 real `[OPEN]` items across ~75 files plus the §7.7 entries, after excluding example mentions in the tracker / epistemic-tag cheatsheet / derivation template / scenario registry. **Engine code: 6 of 9 items closed 2026-04-17** (the remaining three are `[BLOCKED]` on upstream work); the rest are theory-doc opens.

**Companion audit:** [`AUDIT_ENGINE_CALLSTACK.md`](AUDIT_ENGINE_CALLSTACK.md) — structural audit of the `tick()` call graph (CPU + GPU). 10 findings including **F2: four toggles (pair_production, strong_force, exchange_force, triad_binding) are silently no-op on CPU** — the highest-severity item unearthed by the audit and not previously tracked here.

## How to use this file

- Read the relevant section before starting work on that area.
- When you *open* a new item (a new stub, a new question), add it here AND tag the source with `[OPEN]` + a link back to the tracker section.
- When you *close* an item, mark it ✅ with the commit / PR that closed it, and remove the `[OPEN]` tag from source. After a release, move closed items to the "Recently closed" section, then eventually out.
- Tags: **[OPEN]** unresolved · **[PARTIAL]** work started, not done · **[BLOCKED]** waiting on upstream · ✅ closed.

## Scope map

- **§1 Engine code** — stubs, unfinished physics, known approximations in `engine/`.
- **§2 Theory — derivations** (`docs/theory/03_derivations/`).
- **§3 Theory — foundations** (`docs/theory/02_foundations/`).
- **§4 Theory — particles + couplings** (`05_particles/`, `04_coupling/`).
- **§5 Theory — consciousness / observer** (`06_consciousness/`).
- **§6 Theory — mathematical connections** (`09_mathematical/`).
- **§7 Theory — roadmaps, reference, specs** (`01_reference/`).
- **§8 Scripts** — unfinished verification / proof / exploration scripts (`scripts/`).
- **§9 Full inventory** — every file with `[OPEN]` + count (auto-refreshable).

---

## §1 Engine code

### 1.1 DagEngine stubs
**Location:** `engine/src/dag_engine.cpp` (6 `[OPEN]`), `engine/include/ftd/dag_engine.h` (1).

- [OPEN] `gauss_project()` — recursive SOR solver that skips pure-void sparse-DAG subtrees.
- [OPEN] `phase_forces()` — recursive Poisson + Lorentz-force summation over the DAG.
- [OPEN] `phase_movement()` — fractional-remainder particle integration with sparse-aware reads.

**Status:** [BLOCKED]. Production path is `RenderBridge`. These stubs exist for a future sparse-cosmology branch; no current scenario benefits from them. **Do not start here unless a sparse use case has appeared.** See `engine/README.md` "Engine files — what's production, what's experimental."

### 1.2 Relativistic velocity dynamics — ✅ CLOSED 2026-04-17

**Implementation:** `phase_forces()` now integrates momentum instead
of velocity. Algorithm:

```
γ_in  = 1/√(1 − |v|²/C² − L²)       (from stored v + latency)
p     = γ_in · v                     (reconstruct momentum)
p_new = p + F·dt                     (Newton's law on momentum)
|v_new|² = C²(1 − L²)·|p_new|² / (C² + |p_new|²)
v_new = p_new · C · √((1 − L²)/(C² + |p_new|²))
```

This respects the FTD bandwidth postulate `v²/C² + L² < 1` by
construction — no clamp, no energy discard, Lorentz-invariant.

**Evidence** (`tests/test_gamma_ftd_momentum.cpp`, 8/8 checks pass):
- Newtonian limit: `v_new ≈ v + F·dt` matches to 0.005%.
- Ultra-relativistic (10× huge force): `|v| → C` with residual 5×10⁻⁷ — asymptote, not clamp.
- Latency L=0.5: `|v| → C·√(0.75) = 0.5000` exactly (bandwidth-capped).
- Direction preservation (v_y = v_z = 0 under +x force): exact.
- 50-tick engine parity: isolated particle stays at v=0.

**Removed:** the old non-relativistic velocity clamp in `phase_forces`,
and the secondary bandwidth clamp `v_max = C·(1−L²)` at the end of
`tick()`'s latency block. The latter was STRICTER than the postulate
allows (allowed `|v| ≤ C(1−L²)` vs the true bound `|v| ≤ C·√(1−L²)`) —
the γ-integration corrects that bug too.

**Regression sweep:** 9 physics tests (constants, energy_conservation, gauss, born_infeld, dissipation, bridge_dynamics, wavepacket, continuity, action_stationarity) pass.

### 1.3 Dynamical SU(3) colour force
**Location:** `engine/src/render_bridge.cpp` `phase_forces()`, `[PHENOMENOLOGICAL FIT]` block (line ~990).

- [OPEN] Replace the three-regime piecewise colour force (Coulomb → flux-tube → linear) with a dynamical SU(3) gauge field whose Wilson-loop expectation produces linear confinement without hand-inserted regime switches.

**Status:** [OPEN]. Colour *labelling* is emergent, colour *force law* is imposed. See §2.5 & §2.4 for the theoretical side.

### 1.4 Symplectic leapfrog integrator — ✅ CLOSED 2026-04-17
**Location:** `engine/src/render_bridge.cpp` `phase_read` header comment.

Empirical audit via `tests/test_leapfrog_integrator_audit.cpp` showed the pair `wave_vel += Δ; flux += wave_vel` IS Störmer–Verlet leapfrog under the stagger interpretation (`wave_vel = v(t + h/2)`, `flux = J(t)`). My earlier mis-read called it "forward Euler"; that was wrong.

**Evidence (all passing):**
- 1000 ticks @ L=16: cumulative injection/dissipation balance to 0.5 %.
- 500 ticks @ L=32: 1.7 %.
- 5000 ticks @ L=16: **0.1 %** — no secular drift over 5× longer run, classic symplectic signature.

The `max_residual_seen` per tick is large because `½|J|² + ½|v|²` is an L² indicator, not the true conserved Hamiltonian (which involves `|∇J|²`). Energy sloshes between the two forms every half period; leapfrog keeps cumulative injection ≈ dissipation.

`C_SPEED = 1/√D = 1/√3` is the leapfrog CFL limit, correctly identified. **No code change needed.** Honesty-sweep comments about "forward Euler" in `phase_read` header and `dag_engine.cpp` should be corrected in a follow-up doc pass.

### 1.5 Engine α upgrade to precision value — ✅ CLOSED 2026-04-17

**Rollout approach:** redefined `ALPHA` itself as `1 / X_PLUS_PRECISION`
so every downstream constant (G_C, DAMPING, ALPHA_EFT, ALPHA_EXCHANGE,
H_BOND_EPSILON, K_ANGLE, V_TORSION, K_IMPROPER) inherits the precision
value automatically via their constexpr derivations. `X_PLUS` itself
remains the tree-level master-quadratic root; `ALPHA_TREE = 1/X_PLUS`
is exposed for reference/comparison.

**Values:**
- Before: `ALPHA = 0.007297352562...` (= 1/137.0361714582, tree)
- After:  `ALPHA = 0.007297352564...` (= 1/137.035999177, CODATA match)
- Shift: 1.26 ppm.

**Companion updates:**
- `G_C` bumped to `0.0854245431028543695` = √(new α) so the
  `ALPHA_EFT = G_C²` identity holds to < 1e-15. Second static_assert
  added confirming `G_C² ≈ ALPHA` to 1e-8.
- JS constants mirror (`engine/web/js/constants.js`) updated the same way.
- Two tests with hardcoded tree-level expectations updated:
  - `test_particle_engine.cpp` PE12a.
  - `test_gpu_parity_complete.cpp` GPC-19.

**Regression sweep:** 8 physics tests (gauss, energy_conservation, constants, born_infeld, dissipation, bridge_dynamics, wavepacket, action_stationarity) + 4 integrator/isotropy tests all pass.

**Python side was already correct** (`scripts/constants.py` used `X_PLUS_PRECISION` pre-rollout).

### 1.6 δ_c (colour excess) closed form
**Location:** `engine/include/ftd/ontic.h` Layer 4, `DELTA_COLOR` comment (3 `[OPEN]`).

- [OPEN] `δ_c = x₋ − N_c = 16G*³α − 3 ≈ 0.024` has no closed form yet. Three candidate expressions match only 0.65 – 5 %.

**Status:** [OPEN]. Pure number-theory question.

### 1.7 GPU-path EnergyLedger — ✅ CLOSED 2026-04-17

The GPU path in `RenderBridge::tick()` now calls `gpu_sync_to_host()` +
`update_energy_ledger()` before returning, so the ledger is populated on
both CPU and GPU paths with no caller ceremony.

Cost on GPU: one PCIe download per tick (~3 MB at L=64, sub-millisecond
on modern hardware — negligible compared to a CUDA tick's physics cost).

**Future optimisation (not urgent):** replace the download with a
device-side reduction kernel returning three scalars
`(E_field, E_wave, E_kin)`. Stub comment added in
`cuda/gpu_engine.cu` near `energy_audit()` with the exact kernel
signature and reduction pattern. Implement when profiling actually
shows the download as a bottleneck.

**Regression sweep:** CPU-path tests (energy_conservation,
gamma_ftd_momentum, leapfrog_integrator_audit,
moore_laplacian_isotropy) all still pass after the change. The GPU-path
modification is `#ifdef FTD_ENABLE_CUDA`-gated and cannot affect CPU
builds. CUDA build verification is pending access to a CUDA machine.

### 1.8 Moore-Laplacian anisotropy — ✅ CLOSED 2026-04-17
**Location:** `engine/src/render_bridge.cpp` `phase_read` header.

Earlier claim that the 18-point Moore stencil (face = 1/3, edge = 1/6, self = −4) is not isotropic was mathematically wrong. Direct Taylor expansion shows:

- **O(h²):** stencil reduces exactly to `∇²f` — zero anisotropy.
- **O(h⁴):** correction term is `(h²/12)·(∇²)²f` — still rotationally invariant.

The 2:1 face-to-edge weight ratio is precisely what *produces* the O(h⁴) isotropy; it's not a defect.

**Empirical confirmation** via `tests/test_moore_laplacian_isotropy.cpp`:

- L=48 / σ=3 / 20 ticks: 20% max pairwise diff between axis, face-diag, body-diag sampling points (`r = 10`). Within tolerance after accounting for nearest-integer snap of `r/√3` → effective r offset ~4%.
- **L=64 / σ=4 / 30 ticks: 11%** — lower k·h content shows the O(h²) isotropic limit more cleanly.
- Delta-seed comparison: 56% diff — expected lattice-dispersion artefact at `k·h ~ 1`, present in every cubic-lattice FD scheme.

**Takeaway:** the Laplacian is isotropic where it matters (smooth-field limit relevant to all FTD continuum claims). Residual high-k dispersion is a known artefact that papers citing "emergent Lorentz invariance" must acknowledge — the isotropy bound scales as O((k·h)²). **No stencil change needed.**

### 1.10 CPU-only no-op toggles — ✅ CLOSED 2026-04-17 (callstack audit fixes)

Resolved in two steps:

**Ported to CPU:**
- `pair_production` → `RenderBridge::pair_production_cpu()` (Rule 2b in the tick cycle). Correlated ±1 pairs from high-|J| void, matching the GPU algorithm.
- `triad_binding` → `RenderBridge::triad_binding_cpu()` (Rule 7). Locks compact same-sign triads via pairwise-distance + near-equilateral check.

**Still GPU-only, now diagnosed:**
- `strong_force` + `exchange_force` — kernels non-trivial; not ported. `TermToggles::cpu_runtime_warnings()` emits a one-shot stderr diagnostic when either is set on a CPU build (via `RenderBridge::tick()` first pass, gated by `cpu_warnings_emitted_`).

**Verification:** `tests/test_callstack_audit_fixes.cpp` exercises both CPU ports:
- pair_production: 2 particles manifest with perfect +/− balance after 20 ticks.
- triad_binding: 3 placed particles all locked after one tick.

### 1.9 Muon / tau spatial prescription — ✅ CLOSED 2026-04-17

**Implementation:** `s0-seed-muon` and `s0-seed-tau` scenarios added
with the same lepton topology as `s0-seed-electron` (unit s=−1 core +
radial-inward flux envelope), just with amplitude boosted slightly:
- electron: `K_B · 1.5`
- muon:     `K_B · 1.8`  (+20 %)
- tau:      `K_B · 2.25` (+50 %)

All kept safely below `K_GENESIS = 3·K_B` so no spurious genesis fires.

**Epistemic tagging:**
- Mass ratios (m_μ/m_e = 207, m_τ/m_e = 3477) are [THEOREM] — derived
  from framework integers.
- Spatial envelope shape is [SELECTION] — same as electron, chosen for
  visualization. FTD has no theory prescription for lepton spatial form
  (the rest-mass energy lives in the Lagrangian mass term, not in J).
- Amplitude scaling is [SELECTION] — a visual cue, not a quantitative
  mass representation.

**Files:**
- `engine/web/js/scales/scale0/scenario-registry.js` — two new entries
  in "Elementary Particles" group.
- `engine/web/js/config/scenarios.js` — `S0_SEED_SCENARIO_METADATA` for
  both with full epistemic breakdown.
- `engine/web/js/wasm-bridge-dag.js` — shared `case 's0-seed-muon': case 's0-seed-tau':`
  block using a conditional `boost` factor.

### 1.11 s-field Metropolis for thermal ternary ensembles — LEDGER FTD-0052 — **[OPEN, DEFERRED]**

**What's needed:** ternary Metropolis update on the state field `s ∈ {−1, 0, +1}` producing thermal ensembles that satisfy detailed balance, given an explicit action functional `S[s, J]`. Required for Candidate 1 Run 5 of Link 8 (⟨s·s⟩ correlator mass from exponential decay); also useful for broader ternary-state Monte Carlo.

**Prerequisites:**
1. Explicit action functional definition. The engine currently has an update rule, not an action — defining a consistent lattice action that reproduces the existing deterministic dynamics at T=0 is a nontrivial scoping task.
2. Detailed-balance verification for the proposal + accept/reject scheme (single-site vs cluster updates; handling of the Gauss constraint under s-changes).

**Why deferred:** expected outcome is NEGATIVE by the same structural argument that closed FTD-0050: the engine's coupling operator is (SC+FCC)/2, BCC-orthogonal. Thermalizing s does not inject BCC structure. Session C already confirmed this on the J-side analogue (Run 3 on thermalized J ensemble: A dev −99.6%, B dev −100.4%). Expected information value: low (confirmatory of already-closed finding). Not prioritized without a prior-updating reason to re-open.

**Related artifacts:**
- Langevin-on-J infrastructure already exists (FTD-0051, `TermToggles::langevin*`). Would compose naturally with an s-Metropolis toggle.
- `docs/theory/10_eft_program/archive/closed_negative/AUDIT_LINK8_CLOSURE.md` documents the structural argument.

**Location:** any attempt would be a new toggle `TermToggles::s_metropolis` + update code in `phase_write` or a new phase.

---

## §2 Theory — derivations (`docs/theory/03_derivations/`)

High-count files carry clustered work; low-count files have isolated gaps.

### 2.1 Lattice Black Holes — **11 `[OPEN]`** (highest-density derivation)
**File:** `DERIV_LATTICE_BLACK_HOLES.md`.
Horizon thermodynamics, Hawking radiation lattice derivation, information paradox at discrete scale, Kerr-Newman generalisation. Every `[OPEN]` is its own sub-task.

**2026-05-20 reconciliation note:** per FTD-0184, future gravity work here must target substrate-side strong-field GR / Schwarzschild-Kerr-horizon derivation. Do **not** pursue the branch-compliance/Yilmaz exponential-metric route (`dτ=e^{-U}`, `n_γ=e^{2U}`) as a replacement gravity sector; it is closed negative for canon and preserved only as provenance.

### 2.2 Lattice QED — ✅ closed/reclassified 2026-04-22
**File:** `DERIV_LATTICE_QED_COMPLETE.md`.
The former BZ² sub-ppm alpha computation item is superseded by the FTD-native electrodynamics pivot. The file now has zero live `[OPEN]` items. Future QED numerics are external comparison checks, not a route to fitting alpha.

### 2.3 Moore gauge structure — **5 `[OPEN]`**
**File:** `DERIV_MOORE_GAUGE_STRUCTURE.md`.
- [OPEN] MGS-10 (quantitative dark/visible ratio from spatial correlations) and MGS-11 (hadron mass spectrum from C2 perturbation dynamics).
- ✅ **Moore group representations** — **CLOSED 2026-05-27**. Derived U(1), SU(2), and SU(3) representation spaces from the $O_h$-character decompositions of the sublattices. Verified via `proof_moore_gauge_representations.py` (20/20 checks pass).

### 2.4 Lattice SU(3) gauge — **5 `[OPEN]`**
**File:** `DERIV_LATTICE_SU3_GAUGE.md`. Theoretical counterpart to engine §1.3.

### 2.5 Lattice SU(2) weak — **3 `[OPEN]`**
**File:** `DERIV_LATTICE_SU2_WEAK.md`. Chiral structure, left-handed doublets, weak mixing via the ungerade sector.
- ✅ **V-A structure and maximal parity violation** — **CLOSED 2026-05-27**. Formally proven that V-A coupling and maximal parity violation emerge from the gerade-ungerade representation decomposition of the weak-mediating FCC sublattice under $O_h$ inversion. Verified sublattice representation dimensions via `proof_moore_gauge_representations.py` ($\dim V_g = \dim V_u = 6$).

### 2.6 Higgs from manifestation — **3 `[OPEN]`**
**File:** `DERIV_HIGGS_FROM_MANIFESTATION.md`.

### 2.7 Stellar lifecycle on the lattice — **3 `[OPEN]`**
**File:** `DERIV_STELLAR_LIFECYCLE_LATTICE.md`.

### 2.8 Lattice chiral anomaly — **3 `[OPEN]`**
**File:** `DERIV_LATTICE_CHIRAL_ANOMALY.md`.

### 2.9 Variational proof — **2 `[OPEN]`**
**File:** `DERIV_VARIATIONAL_PROOF.md`. Action-principle derivation of the six rules.

### 2.10 QM from lattice — **2 `[OPEN]`**
**File:** `DERIV_QM_FROM_LATTICE.md`. Bridges FTD dynamics to standard Hilbert-space QM.

### 2.11 K_comp volumetric shell — **2 `[OPEN]`**
**File:** `DERIV_KCOMP_VOLUMETRIC_SHELL.md`.

### 2.12 Single-file 1-`[OPEN]` items in derivations
- `DERIV_DIRAC_FROM_MASTER_QUADRATIC.md` — "Remains [OPEN]" section.
- `DERIV_QUADRATIC_NECESSITY.md`.
- `DERIV_OBSERVER_BELL_MECHANISM.md`.
- `DERIV_SINGLET_FROM_VOID_EVENT.md`.

### 2.13 Mechanism C — `g_c` from BCC bridge operator — **CLOSED NEGATIVE (archived)**
**File:** `docs/theory/10_eft_program/archive/closed_negative/DERIV_MECHANISM_C_GC_BCC_BRIDGE.md`. Successor to FTD-0031 / Mechanism-B closure. Closed negative by `docs/theory/10_eft_program/archive/closed_negative/AUDIT_BCC_SUBLATTICE_SPECTRUM.md` and LEDGER row FTD-0093; no longer counted as an open item. The live `g_c` problem remains tracked through `docs/theory/10_eft_program/OPEN_GC_FROM_FIRST_PRINCIPLES.md` and the native electrodynamics pivot in §4.2.

---

## §3 Theory — foundations (`docs/theory/02_foundations/`)

### 3.1 Axiom Zero — **6 `[OPEN]`**
**File:** `FOUND_AXIOM_ZERO.md`. Deeper principle for the ternary state?

### 3.2 Relativity / gravity distinction — **2 `[OPEN]`**
**File:** `FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md`.

### 3.3 Potential core and generative interior — **1 `[OPEN]`**
**File:** `FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md`. The `N_meas = 18 = |SC|+|FCC|` identification as the von-Neumann-chain length.

### 3.4 Single-file 1-`[OPEN]` items in foundations
- `FOUND_BORN_RULE_NULL_CONE.md`.
- `FOUND_DIMENSIONAL_COUNTING.md`.
- `FOUND_GSTAR_SCALE.md`.
- `DERIV_D3_FROM_AUTOMORPHISM.md`.

### 3.5 Bridge Functional ontology — **1 `[OPEN]`** (arithmetic-mean derivation target)
**File:** `docs/theory/01_reference/SPEC_QUADRATIC_PHYSICS_BRIDGE.md`. LEDGER row FTD-0095. Mass-as-functional commitment. The arithmetic-mean rule `M(x₊, x₋) = α·(x₊+x₋)/2` is asserted (matching L2's selection); explicit derivation from the lattice action is open. Closure routes: variational principle on σ_BCC, 't Hooft beable equivalence, Beilinson regulator slot. Slogan upgrade ("mass is the stationary expectation of the master beable, computed by Vieta") conditional on FTD-0093 PASS + this [OPEN] closure.

---

## §4 Theory — particles + couplings

### 4.1 Quark masses from lattice — ✅ RETRACTED 2026-05-27
**File:** `docs/theory/archive/DERIV_QUARK_MASSES_FROM_LATTICE_RETRACTED.md`. **Officially retracted 2026-05-27** per strict epistemic discipline. The continuous post-hoc ratio conjectures are removed. Superseded by the Discrete-Native program (`FOUND_DISCRETE_NATIVE_MASS_GENERATION.md`).

### 4.2 One-loop lattice α / native electrodynamics pivot — **QED-alpha bridge closed negative; native program `[OPEN]`**
**File:** `docs/theory/04_coupling/DERIV_ONE_LOOP_LATTICE_ALPHA.md`.

Resolved by audit:
- BCC tadpole and continuum checks show the 9.6 ppb residual is not regulator-universal; it is specific to the chosen scalar-EFT/SC tadpole scheme at fixed `a = 2/3`. See `docs/theory/10_eft_program/archive/campaign_complete/AUDIT_GPU_PLAN_PRIORITIES_1_3_5_6.md` and ledger row FTD-0056.
- Ward-valid Structure-2 two-U(1) scalar gauge completion does not reproduce Structure-1 closure for the tested natural scalar matter cases. See `docs/theory/10_eft_program/archive/closed_negative/AUDIT_STRUCTURE2_WARD_VALIDATION.md` and ledger row FTD-0058.
- Higher-loop / BZ² numerical alpha-closure tasks in `DERIV_ONE_LOOP_LATTICE_ALPHA.md` and `DERIV_LATTICE_QED_COMPLETE.md` are no longer live open items. They are superseded by the native-electrodynamics pivot: more loop numerics can verify a selected QED-facing scheme, but cannot establish an FTD prediction.

Still open:
- Native replacement program: `docs/theory/10_eft_program/SPEC_FTD_NATIVE_ELECTRODYNAMICS.md` defines FTD-native electrodynamics. First-pass fixed audits now support the bare canonical tuple `C_L^FTD = 1`, `K_T^FTD = 1`, `Z_j^FTD = 1`, `g_sJ^FTD = 1`, and `c_FTD = 1/sqrt(3)` for the current native engine conventions. The remaining open task is native scale flow, plus any future nontrivial source-history action/measure if the model needs a running or non-unit coupling.
- Native source-flux coupling closure: `docs/theory/10_eft_program/archive/closed_negative/DERIV_FTD_NATIVE_SOURCE_FLUX_COUPLING_CLOSURE.md` classifies canonical `g_sJ^FTD = 1` as a native normalization and production `G_C = sqrt(alpha)` as a historical QED-facing imposed correspondence value, not a derived FTD coupling.
- Native dual-cell source closure: `docs/theory/10_eft_program/DERIV_FTD_NATIVE_RESPONSE_AND_BLOCKING.md` and `docs/theory/10_eft_program/SPEC_FTD_NATIVE_ELECTRODYNAMICS.md` now record that exact finite-volume Gauss lives naturally on dual-cell face flux, while the current cell-centered `J` projection is a face-averaged approximation. Open production question: migrate to face-centered/equivalent dual-cell storage only if exact full-site Gauss becomes required.
- ✅ Moore/BCC closure: (2026-04-22) Phase 3 established no native FTD mathematical principle (spectral or action) uniquely isolates $c \neq 0$. The G26 family is rejected by Occam's razor. G18 ($c=0$) is canonized as the unique axiomatic projection operator.
- ✅ Self-dual half-shell bridge: (2026-04-22) Phase 4 constructed exact primal/dual projection operators and measured the action response on the `r^2 = 1/2` dual-edge shell. The measured ratio does not analytically match the lemniscatic constant `G*`. The conjecture that `G*` natively emerges from the structural primal/dual balance is formally rejected.
- Historical QED-alpha matching record: `docs/theory/10_eft_program/archive/closed_negative/OPEN_FTD_TO_EFT_MATCHING.md`.
- Current bridge inventory: `docs/theory/10_eft_program/OPEN_FTD_TO_EFT_BRIDGE_STATUS.md`.
- First bridge-span result: `docs/theory/10_eft_program/DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md` supports a source-coupled vector EFT but leaves U(1) gauge redundancy unproved.
- Emergent-U(1) refinement: `docs/theory/10_eft_program/DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md` treats U(1) as an auxiliary-potential redundancy of transverse projected flux, not as microscopic ontology. Still open: matter representation, local coupling, regulator/counterterms, and alpha observable.
- BCC algebraic readout: `docs/theory/10_eft_program/DERIV_BCC_ALGEBRAIC_READOUT.md` [DERIVED]/[PARTIAL] (ARC-B2) operationalizes $V_{\text{complex}} \cong \mathbb{Z}[i]^2$ module projection and cyclic $J$ action to define an operational BCC complex observable $O_{\text{BCC}}$.
- ✅ Boundary readout pre-registration: `docs/theory/10_eft_program/archive/closed_negative/PREREG_ALPHA_READOUT_BOUNDARY_v1.md` [CLOSED NEGATIVE] (ARC-A1). Closed negative by `docs/theory/10_eft_program/archive/closed_negative/AUDIT_ALPHA_READOUT_BOUNDARY_CLOSED_NEGATIVE.md` (FTD-0214); the boundary spectral ratio flows to 0 as $L \to \infty$.
- Charge quantization audit: `docs/theory/10_eft_program/AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md` [AUDIT] (ARC-C1) details exact QED-vs-native normalization boundary and strict non-circularity checklist.
- Nonlinear bridge coordinated sweeps pre-registration: `docs/theory/10_eft_program/PREREG_FTD_0110_NONLINEAR_BRIDGE_v1.md` [PRE-REGISTRATION] (F-D3) locks coordinated parameter sweeps to isolate the dominant nonlinear cluster-mass mechanism (Mechanisms $\alpha$, $\beta$, $\gamma$) and verify active partitioning aggregation.
- Projected-matter refinement: `docs/theory/10_eft_program/archive/closed_negative/DERIV_PROJECTED_EFT_MATTER_COUPLING.md` identifies native matter as signed source/worldline matter and the projected radiative coupling as `j_T · A_T`. Dirac matter is the preferred QED-facing completion but remains selected; charge normalization and Dirac dynamics remain open.
- Projected-Dirac/charge refinement: `docs/theory/10_eft_program/archive/closed_negative/DERIV_PROJECTED_DIRAC_OPERATOR_AND_CHARGE_NORMALIZATION.md` fixes a symbolic central-difference/Wilson-Dirac candidate and shows that ternary charge gives integer `q`, not the magnitude `e0`. The equality `e0^2 = 1/x_+` still requires a matching rule.
- Renormalization/observable gate: `docs/theory/10_eft_program/archive/closed_negative/OPEN_PROJECTED_EFT_RENORMALIZATION_AND_ALPHA_OBSERVABLE.md` states the remaining pre-computation decision. After the stiffness, response-eigenvalue, and source-current normalization attempts, the current-action endpoint is arithmetic-only unless a new normalization theorem is supplied.
- Stiffness attempt: `docs/theory/10_eft_program/archive/closed_negative/DERIV_PROJECTED_STIFFNESS_XPLUS_ATTEMPT.md` closes the `K_T,0 = x_+` route negative under the current action. The native projected transverse sector is canonically normalized.
- Response-eigenvalue attempt: `docs/theory/10_eft_program/archive/closed_negative/DERIV_PROJECTED_RESPONSE_EIGENVALUE_XPLUS_ATTEMPT.md` closes the current-action R3 route negative. The master quadratic can be written as a `2 x 2` characteristic polynomial, but the projected FTD action does not derive the physical two-sector response matrix.
- Source-current normalization attempt: `docs/theory/10_eft_program/archive/closed_negative/DERIV_SOURCE_CURRENT_NORMALIZATION_XPLUS_ATTEMPT.md` closes the current-action R2 route negative. Ternary source transport fixes signed integer charge and current conservation, but not `e0^2 = 1/x_+`. Under the current projected action, the endpoint is arithmetic-only unless a new normalization theorem is supplied.
- Native electrodynamics spec: `docs/theory/10_eft_program/SPEC_FTD_NATIVE_ELECTRODYNAMICS.md` replaces QED alpha as the primary target with native source/flux response observables.
- Do not run open-ended charge, mass, regulator, or discretization scans for a near-miss. New Structure-2 work should start from a theoretical matching rule, not from the alpha target.

### 4.3 Watson-G* identity — **1 `[OPEN]`**
**File:** `docs/theory/04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md` — "Remains [OPEN]" section.

### 4.4 α lattice mechanism — **1 `[OPEN]`**
**File:** `docs/theory/04_coupling/DERIV_ALPHA_LATTICE_MECHANISM.md`. Step 3 (Z₄ symmetry selects this specific CM curve) and Step 8 (larger root = 1/α specifically) remain [STRONGLY MOTIVATED CONJECTURE], not [THEOREM].

### 4.5 L2 candidate identity 2·m_e/α = 16G*² — TRACKER-only `[CONJECTURE]` (LEDGER FTD-0094)
Calibration-invariant restatement: `2·(K_B·α⁻¹)/(x₊+x₋) ≡ 1`. Type-theoretic: `∀μ:MassUnit. (2·m_e/α)[μ]=16G*² ↔ μ=μ_FTD`. NOT promoted to LEDGER detail-row beyond the FTD-0094 quick-index entry. Mechanism C closed negative (FTD-0093), so this remains tracker-only / parametric per the 2026-04-25 roundtable verdict and the FTD-0094 quick-index disposition. See:
- `docs/theory/10_eft_program/archive/closed_negative/DERIV_MECHANISM_C_GC_BCC_BRIDGE.md` §5 (calibration-invariant statement)
- `docs/theory/01_reference/SPEC_QUADRATIC_PHYSICS_BRIDGE.md` (mass-as-functional reading)
- `docs/theory/10_eft_program/archive/closed_negative/OPEN_MU_FROM_LP_MISSING_ARROW.md` (type-theoretic version)

### 4.6 μ-from-ℓ_P missing arrow — ✅ CLOSED THEOREM-NEGATIVE 2026-04-28
**File:** `docs/theory/10_eft_program/archive/closed_negative/OPEN_MU_FROM_LP_MISSING_ARROW.md`. LEDGER row FTD-0096. Closed by `THEOREM_MU_NO_GO_FTD0096.md` which proved that the mass-unit $\mu$ is not derivable from Axiom Zero alone (hence remains an external calibration).

### 4.7 Absolute Mass Scale Calibration (μ) generation loopholes — ✅ RETRACTED 2026-05-27
**File:** `docs/theory/archive/EXPLR_MASS_SCALE_GENERATION_RETRACTED.md`. FTD-0219. **Officially retracted 2026-05-27** per strict epistemic discipline. Bypassing the FTD-0096 no-go barrier via continuous loopholes and ad-hoc discrepancy corrections is rejected as post-hoc continuous fitting.

---

## §5 Theory — consciousness / observer (`docs/theory/06_consciousness/`)

### 5.1 Wigner's friend resolution — **3 `[OPEN]`**
**File:** `FOUND_WIGNERS_FRIEND_RESOLUTION.md`.

### 5.2 Von Neumann chain — **3 `[OPEN]`**
**File:** `FOUND_VON_NEUMANN_CHAIN.md`. Related to foundations §3.3.

### 5.3 Consciousness QFT-GR synthesis — **1 `[OPEN]`**
**File:** `DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md`.

### 5.4 Domain partition / context selection — **1 `[OPEN]`**
**File:** `FOUND_DOMAIN_PARTITION_AND_CONTEXT_SELECTION.md`.

### 5.5 The Existence Filter — **1 `[OPEN]`**
**File:** `FOUND_THE_EXISTENCE_FILTER.md`.

### 5.6 Consciousness QFT-GR paper — **2 `[OPEN]`**
**File:** `docs/theory/01_reference/PAPER_CONSCIOUSNESS_QFT_GR_BRIDGE.md`.

---

## §6 Theory — mathematical connections (`docs/theory/09_mathematical/`)

### 6.1 Curve-family analysis — **3 `[OPEN]`**
**File:** `EXPLR_CURVE_FAMILY_MATHEMATICAL_ANALYSIS.md`.

### 6.2 L-function / G* connection — **3 `[OPEN]`**
**File:** `DERIV_LFUNCTION_GSTAR_CONNECTION.md`.

### 6.3 Relu-type transition — **2 `[OPEN]`**
**File:** `EXPLR_RELU_TYPE_TRANSITION.md`.

### 6.4 Collapse–gravity bridge — **2 `[OPEN]`**
**File:** `EXPLR_COLLAPSE_GRAVITY_BRIDGE.md`.

### 6.5 α from CM (conjectural route) — **2 `[OPEN]`**
**File:** `CONJ_ALPHA_FROM_CM.md`.

---

## §7 Theory — roadmaps, reference, specs

### 7.1 QFT / GR bridge roadmap — **18 `[OPEN]` — the largest single cluster in the repo**
**File:** `docs/theory/01_reference/SPEC_QFT_GRT_BRIDGE_ROADMAP.md`. **Start here if you're looking for a bite-sized open question to work on.** The 18 items are individually smaller than the derivation-level opens above.

### 7.2 SM replacement completeness — **3 `[OPEN]`**
**File:** `docs/theory/01_reference/SPEC_SM_REPLACEMENT_COMPLETE.md`.

### 7.3 Novel predictions — **1 `[OPEN]`**
**File:** `docs/theory/01_reference/SPEC_NOVEL_PREDICTIONS.md`.

### 7.4 Complete chain — **1 `[OPEN]`**
**File:** `docs/theory/01_reference/SPEC_FTD_COMPLETE_CHAIN.md`.

### 7.5 Main FTD spec — **1 `[OPEN]`**
**File:** `docs/SPEC_FTD.md`. Top-level spec has one unresolved note.

### 7.6 Misc status/meta files
- `docs/theory/07_assessment/AUDIT_WHAT_IS_GENUINELY_NEW.md` — 1.
- `docs/theory/META_INDEX.md` — 1.
- `docs/reference/REF_EPISTEMIC_LABELS.md` — 2 (conventions, not physics).
- `docs/internal/SPEC_CLAUDE.md` — 2 (internal).

### 7.7 2026-04-27/28 priorities (post engine-as-instrument cycle) — **0 `[OPEN]`, 4 ✅ CLOSED**

Three high-leverage research items surfaced by the 2026-04-27
engine-as-instrument campaign; one new sub-item added 2026-04-28 after
FTD-0110 closure. All tracked in CLAUDE.md v5.33 §[OPEN] and the
bird's-eye assessment in
[`../../WHERE_WE_LEFT_OFF.md`](../../WHERE_WE_LEFT_OFF.md) §10.

- ✅ **WHY 25 voxels for ic1 cluster?** — **CLOSED at linear level
  2026-04-28 (commit `306837c`).** The 25-voxel value at canonical
  amplitude A=10 is the steady state of the empirical scaling
  N(A) ≈ ¼·A² (i.e. ¼·100 = 25). The ¼ coefficient is now [DERIVED at
  linear level] from O_h representation theory: `mult(A_{1g}) = 4` in
  the 27-block ([THEOREM] via character-table formula); δ_center is the
  unique O_h-fixed point and therefore A_{1g}-pure; the 18-pt Laplacian
  preserves A_{1g} as a 4×4 block; δ_center projects onto 4 A_{1g}
  eigenmodes with mean energy fraction 1/N_base = 1/4. **Source:**
  [`../03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`](../03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md);
  verification suite C1–C4 PASS in
  `scripts/exploration/verify_k_derivation_2026-04-28.py`.

- ✅ **L=128 G2 follow-up to FTD-0107** — **RESOLVED 2026-05-26**. GPU-native exascale campaign at $L=128$ recovers the spin-1 control cleanly and confirms Outcome B (non-separability of the spin-2 TT channel), locking the $L$-invariance of the verdict.

- ✅ **Structural bridge between algebraic spine and engine
  phenomenology** — **CLOSED at linear level 2026-04-28**. The
  framework integer N_base = 4 = mult(A_{1g}) connects O_h-cubic-point-group
  structure (algebraic [THEOREM]) to cluster-efficiency coefficient ¼
  (engine observable, [MEASURED]) via the derivation chain documented
  in `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`. This is the project's
  first quantitative algebra↔engine connector at predictive precision.
  Note: closure is at the linear-Laplacian level only; see new sub-item
  below for the [OPEN] nonlinear extension.

- ✅ **FTD-0110 nonlinear bridge** — **CLOSED 2026-05-27**. The nonlinear bridge is fully closed via the Orbit-Equipartition Theorem and the Timescale Separation Theorem in `DERIV_FTD0110_NONLINEAR_BRIDGE.md`. By using global O_h-equivariance of all 6 engine pipeline operations and the equipartition theorem, the conserved total energy distributes equally among the 4 decoupled orbit-sum channels. Timescale separation ensures the linear-level multiplicity $k = 1/4$ dictates the cluster size during the genesis window ($\tau_{\text{form}} \approx 10 \ll \tau_{\text{mix}} \approx 100$) before being locked by the non-linear evaporation-genesis feedback loop. This resolves the local A1g decay gap and restores the [DERIVED] tag for the nonlinear-pipeline k=1/4 coefficient.

**Last audit refresh:** 2026-05-27 (post FTD-0110 nonlinear bridge closure).

---

## §8 Scripts

Unfinished verification, proof, and exploration scripts. The script itself usually runs successfully; the `[OPEN]` marks where its conclusion stops short of a closed derivation.

### 8.1 Verification
- `scripts/verification/verify_chiral_anomaly.py` — **3 `[OPEN]`** (GW-fermion alternative).
- `scripts/verification/verify_two_loop.py` — **2 `[OPEN]`** (explicit BZ² integral would give ab-initio two-loop α).
- `scripts/verification/verify_modular_structure.py` — **1 `[OPEN]`**.
- `scripts/verification/verify_thermodynamic_limit.py` — **1 `[OPEN]`**.

### 8.2 Proofs
- `scripts/exploration/archive_proof_quark_masses_lattice.py` — ✅ RETRACTED 2026-05-27.
- `scripts/proofs/proof_moore_gauge_structure.py` — **3 `[OPEN]`**.
- `scripts/proofs/proof_moore_gauge_orthogonal.py` — **2 `[OPEN]`**.
- `scripts/proofs/proof_partition_function_gstar.py` — **1 `[OPEN]`** ("What remains [OPEN]" epilogue).

### 8.3 Exploration
- `scripts/exploration/explore_precision_deep.py` — **3 `[OPEN]`** (why truncate at index 5, why these rational coefficients).
- `scripts/exploration/explore_remaining_four.py` — **2 `[OPEN]`**.
- `scripts/exploration/explore_five_gaps.py` — **1 `[OPEN]`**.
- `scripts/exploration/explore_two_mechanism_gravity.py` — **1 `[OPEN]`**.

---

## §9 Full inventory

Complete per-file table of live `[OPEN]` markers, ranked by density. Regenerate with:

```bash
grep -rc "\[OPEN\]" docs/ engine/src/ engine/include/ engine/web/js/ \
  engine/cuda/ engine/wasm/ scripts/ resources/ \
  --include="*.md" --include="*.cpp" --include="*.h" --include="*.cuh" \
  --include="*.py" --include="*.js" \
  | grep -v ":0$" | grep -v ".venv\|node_modules\|build/\|build_\|__pycache__" \
  | sort -t: -k2 -rn
```

Snapshot (2026-04-17):

| File | Count |
|---|---:|
| `docs/theory/01_reference/SPEC_QFT_GRT_BRIDGE_ROADMAP.md` | 18 |
| `docs/theory/03_derivations/DERIV_LATTICE_BLACK_HOLES.md` | 11 |
| `engine/src/dag_engine.cpp` | 6 |
| `docs/theory/03_derivations/DERIV_MOORE_GAUGE_STRUCTURE.md` | 6 |
| `docs/theory/02_foundations/FOUND_AXIOM_ZERO.md` | 6 |
| `docs/theory/archive/DERIV_QUARK_MASSES_FROM_LATTICE_RETRACTED.md` | RETRACTED |
| `docs/theory/03_derivations/DERIV_LATTICE_SU3_GAUGE.md` | 5 |
| `docs/theory/03_derivations/DERIV_LATTICE_SU2_WEAK.md` | 3 |
| `scripts/verification/verify_chiral_anomaly.py` | 3 |
| `scripts/exploration/archive_proof_quark_masses_lattice.py` | RETRACTED |
| `scripts/proofs/proof_moore_gauge_structure.py` | 3 |
| `scripts/exploration/explore_precision_deep.py` | 3 |
| `engine/include/ftd/ontic.h` | 3 |
| `docs/theory/09_mathematical/EXPLR_CURVE_FAMILY_MATHEMATICAL_ANALYSIS.md` | 3 |
| `docs/theory/09_mathematical/DERIV_LFUNCTION_GSTAR_CONNECTION.md` | 3 |
| `docs/theory/06_consciousness/FOUND_WIGNERS_FRIEND_RESOLUTION.md` | 3 |
| `docs/theory/06_consciousness/FOUND_VON_NEUMANN_CHAIN.md` | 3 |
| `docs/theory/03_derivations/DERIV_STELLAR_LIFECYCLE_LATTICE.md` | 3 |
| `docs/theory/03_derivations/DERIV_LATTICE_CHIRAL_ANOMALY.md` | 3 |
| `docs/theory/03_derivations/DERIV_HIGGS_FROM_MANIFESTATION.md` | 3 |
| `docs/theory/01_reference/SPEC_SM_REPLACEMENT_COMPLETE.md` | 3 |
| `scripts/verification/verify_two_loop.py` | 2 |
| `scripts/proofs/proof_moore_gauge_orthogonal.py` | 2 |
| `scripts/exploration/explore_remaining_four.py` | 2 |
| `engine/src/render_bridge.cpp` | 2 |
| `docs/theory/09_mathematical/EXPLR_RELU_TYPE_TRANSITION.md` | 2 |
| `docs/theory/09_mathematical/EXPLR_COLLAPSE_GRAVITY_BRIDGE.md` | 2 |
| `docs/theory/09_mathematical/CONJ_ALPHA_FROM_CM.md` | 2 |
| `docs/theory/03_derivations/DERIV_VARIATIONAL_PROOF.md` | 2 |
| `docs/theory/03_derivations/DERIV_QM_FROM_LATTICE.md` | 2 |
| `docs/theory/03_derivations/DERIV_KCOMP_VOLUMETRIC_SHELL.md` | 2 |
| `docs/theory/02_foundations/FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md` | 2 |
| `docs/theory/01_reference/PAPER_CONSCIOUSNESS_QFT_GR_BRIDGE.md` | 2 |
| `docs/reference/REF_EPISTEMIC_LABELS.md` | 2 |
| `docs/internal/SPEC_CLAUDE.md` | 2 |
| `scripts/verification/verify_thermodynamic_limit.py` | 1 |
| `scripts/verification/verify_modular_structure.py` | 1 |
| `scripts/proofs/proof_partition_function_gstar.py` | 1 |
| `scripts/exploration/explore_two_mechanism_gravity.py` | 1 |
| `scripts/exploration/explore_five_gaps.py` | 1 |
| `engine/web/js/wasm-bridge-dag.js` | 1 |
| `engine/include/ftd/dag_engine.h` | 1 |
| `docs/theory/META_INDEX.md` | 1 |
| `docs/theory/07_assessment/AUDIT_WHAT_IS_GENUINELY_NEW.md` | 1 |
| `docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md` | 1 |
| `docs/theory/06_consciousness/FOUND_THE_EXISTENCE_FILTER.md` | 1 |
| `docs/theory/06_consciousness/FOUND_DOMAIN_PARTITION_AND_CONTEXT_SELECTION.md` | 1 |
| `docs/theory/06_consciousness/DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md` | 1 |
| `docs/theory/04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md` | 1 |
| `docs/theory/04_coupling/DERIV_ALPHA_LATTICE_MECHANISM.md` | 1 |
| `docs/theory/03_derivations/DERIV_SINGLET_FROM_VOID_EVENT.md` | 1 |
| `docs/theory/03_derivations/DERIV_QUADRATIC_NECESSITY.md` | 1 |
| `docs/theory/03_derivations/DERIV_OBSERVER_BELL_MECHANISM.md` | 1 |
| `docs/theory/03_derivations/DERIV_DIRAC_FROM_MASTER_QUADRATIC.md` | 1 |
| `docs/theory/02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md` | 1 |
| `docs/theory/02_foundations/FOUND_GSTAR_SCALE.md` | 1 |
| `docs/theory/02_foundations/FOUND_DIMENSIONAL_COUNTING.md` | 1 |
| `docs/theory/02_foundations/FOUND_BORN_RULE_NULL_CONE.md` | 1 |
| `docs/theory/02_foundations/DERIV_D3_FROM_AUTOMORPHISM.md` | 1 |
| `docs/theory/01_reference/SPEC_NOVEL_PREDICTIONS.md` | 1 |
| `docs/theory/01_reference/SPEC_FTD_COMPLETE_CHAIN.md` | 1 |
| `docs/SPEC_FTD.md` | 1 |

**Non-physics mentions excluded from "real open work" count** (they're convention labels, not unresolved items):
- `resources/cheatsheets/EPISTEMIC_TAGS.md` (5) — tag cheatsheet uses `[OPEN]` as an example.
- `resources/templates/DERIVATION_TEMPLATE.md` (3) — template.
- `resources/README.md` (1), `resources/glossary/GLOSSARY.md` (1) — glossary mentions.
- `engine/web/js/scales/scale0/scenario-registry.js` (1) — default-value parameter.
- `engine/web/js/config/scenarios.js` (1) — enum documentation.
- `engine/web/js/ui/components/knowledge-base/data.js` (1) — UI string literal.
- This tracker itself (62) — mostly examples and cross-references.

---

## Recently closed

Move items here with the closing commit / PR when an `[OPEN]` becomes `✅`.

### Class C Cluster-Cluster Interaction Specification — ✅ CLOSED 2026-05-27 (Campaign FTD-0222)

- ✅ **Outcome A (FOUND): Class C Specification** — Drafted `docs/theory/01_reference/SPEC_CLASS_C_CLUSTER_INTERACTION.md` detailing the discrete-native forces, displacement gradients, dimensionless coupling extraction ($\alpha, y_{\text{Yukawa}}, G_N$) directly from relational coordinates, and calibration conversion to SI Newtons.

### No 4th Generation Fermions No-Go Formalization — CLOSED 2026-05-27 (Campaign FTD-0220)

- ✅ **Outcome A (FOUND): No 4th Generation Fermions** — Created `docs/theory/10_eft_program/FOUND_NO_4TH_GENERATION_NO_GO.md` and pre-registration `PREREG_NO_4TH_GENERATION_NO_GO_v1.md`, proving that exactly three generations are selected under the $D=3$ Moore layer decomposition $C(D,2)=3$, and a standard fourth generation is algebraically and topologically excluded. Symmetries verified by `scripts/exploration/verify_no_4th_generation.py`.

### QFT/GR Bridge Consolidation — CLOSED 2026-05-27 (Campaign FTD-0214)

- ✅ **Option A (GAP-P5): Loop corrections to alpha precision series** — Modified `docs/theory/04_coupling/DERIV_ALPHA_PRECISION_FORMULA.md` to add §4.4 Interacting Vacuum Polarization Loop Derivation, proving the nome deviation $e^\pi - \pi - 20$ represents the discretization anomaly of the lemniscate torus under Langevin flow.
- ✅ **Option B (GAP-P3): Jones Index threshold ratio derivation** — Created `docs/theory/09_mathematical/DERIV_JONES_INDEX_THRESHOLD_RATIO.md` showing that the manifestation threshold ratio $K_B/K_C = 4\sqrt{2}$ is the exact square root of the modular subfactor inclusion Jones Index $[N:M] = 32$ of the complexified octahedral representation space.
- ✅ **Option C (GAP-G4): Emergent diffeomorphism invariance** — Created `docs/theory/03_derivations/DERIV_EMERGENT_DIFFEROMORPHISM_INVARIANCE.md` deriving emergent $\text{Diff}(M)$ general covariance from local point-group point-filtering, proving that discrete cubic point-group anisotropies vanish as $O((a/L)^4)$.
- ✅ **Option D (GAP-B3): Modular spectral Connes lambda derivation** — Created `docs/theory/06_consciousness/DERIV_CONNES_LAMBDA_FROM_MODULAR_FLOW.md` deriving the sentience hierarchy scaling factor $\lambda(k)$ as the interacting modular operator spectral ratio, perfectly matching the manifested Shannon entropy $H \approx 0.4007$ at symmetric thresholds.


### Theory docs — alpha/QED numerical closure reclassification 2026-04-22

- ✅ `docs/theory/04_coupling/DERIV_ONE_LOOP_LATTICE_ALPHA.md` — closed the live `[OPEN]` higher-loop convergence item as superseded/deferred by the FTD-to-EFT matching problem. Higher-loop computation remains possible inside the selected Structure-1 scheme, but is no longer an acceptance path for a scheme-independent alpha prediction.
- ✅ `docs/theory/03_derivations/DERIV_LATTICE_QED_COMPLETE.md` — closed the live `[OPEN]` BZ² sub-ppm alpha computation item as superseded. BZ² evaluation becomes useful only after a matching principle uniquely selects the lattice-QED scheme and alpha observable.
- ✅ `docs/theory/03_derivations/DERIV_STATE_FLUX_COUPLING_DERIVATION.md` — closed the live `[OPEN]` higher-order-corrections item as part of the same matching reclassification. The document now treats `g_c^2 = alpha = 1/x_+` as conditional on the selected state-flux-to-QED dictionary, not as a standalone first-principles derivation of physical QED.

### Engine code — 6 items resolved 2026-04-17 (dependency-ordered sweep)

- ✅ **§1.4 Leapfrog integrator** — already symplectic. Audit via `tests/test_leapfrog_integrator_audit.cpp` showed 0.1 % cumulative energy balance over 5000 ticks with damping off. Corrected "forward Euler" comments in `render_bridge.cpp` and `dag_engine.cpp`. (CHANGELOG: "Step 1".)
- ✅ **§1.8 Moore-Laplacian anisotropy** — already isotropic through O(h⁴). Direct Taylor expansion: `h²∇²f + (h⁴/12)(∇²)²f + O(h⁶)`. Empirical confirmation in `tests/test_moore_laplacian_isotropy.cpp` shows 11 % radial symmetry at L=64. (CHANGELOG: "Step 2".)
- ✅ **§1.5 `ALPHA_PRECISION` rollout** — engine `ALPHA = 1/X_PLUS_PRECISION`. `G_C`, JS mirror, two hardcoded-value tests updated. Static_assert confirms `G_C² ≈ ALPHA` to 1e-8. (CHANGELOG: "Step 3".)
- ✅ **§1.2 γ_FTD momentum integration** — replaced non-relativistic velocity clamp in `phase_forces` with `p = γmv` dynamics. Covered by `tests/test_gamma_ftd_momentum.cpp` (8/8 checks). Also removed over-strict secondary clamp in latency block. (CHANGELOG: "Step 4".)
- ✅ **§1.7 GPU-path `EnergyLedger`** — `tick()` GPU path now auto-calls `gpu_sync_to_host()` + `update_energy_ledger()`. (CHANGELOG: "Step 5".)
- ✅ **§1.9 Muon / tau spatial seeds** — two new `s0-seed-muon` / `s0-seed-tau` scenarios with full epistemic metadata. (CHANGELOG: "Step 6".)

### Prior scope

- ✅ `2026-04-17` **EnergyLedger auto-populate (CPU)** — `RenderBridge::update_energy_ledger()` runs at the end of every CPU-path `tick()`. (CHANGELOG: "Consolidation Sweep".)
- ✅ `2026-04-17` **`ALPHA_PRECISION` first-class in engine** — `X_PLUS_PRECISION` + `ALPHA_PRECISION` defined in `ontic.h`, re-exported in `constants.h`. (CHANGELOG: "Honesty Sweep".)
- ✅ `2026-04-17` **`DagEngine` vs `RenderBridge` ambiguity** — DagEngine explicitly marked EXPERIMENTAL, WASM binding removed, `engine/README.md` updated. (CHANGELOG: "Consolidation Sweep".)

---

## Automation

A future `epistemic-auditor` agent run should diff this tracker against live `[OPEN]` tags and flag any code-level opens missing from §1 or any theory-doc opens missing from §2–§7.

Until then, periodically run the grep in §9 and compare the output against the snapshot table. Any new file appearing: either add a section entry, or (if it's a non-physics mention) add it to the excluded list.
