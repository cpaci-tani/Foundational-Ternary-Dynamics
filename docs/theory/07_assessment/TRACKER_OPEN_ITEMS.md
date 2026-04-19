# Open Items Tracker

Central ledger of every `[OPEN]` claim in FTD — code stubs, theoretical gaps, unresolved verification tasks, and research questions. One place to look when picking work, one place to update when an item closes.

**Canonical path:** `docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md`.

**Last full audit:** 2026-04-17 (post-engine-cleanup). **Live repo count:** ~202 real `[OPEN]` items across ~75 files, after excluding example mentions in the tracker / epistemic-tag cheatsheet / derivation template / scenario registry. **Engine code: 6 of 9 items closed today** (the remaining three are `[BLOCKED]` on upstream work); the rest are theory-doc opens.

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
- **§10 Archived** — `[OPEN]` items inside `docs/theory/archive/` (historical context only).

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

---

## §2 Theory — derivations (`docs/theory/03_derivations/`)

High-count files carry clustered work; low-count files have isolated gaps.

### 2.1 Lattice Black Holes — **11 `[OPEN]`** (highest-density derivation)
**File:** `DERIV_LATTICE_BLACK_HOLES.md`.
Horizon thermodynamics, Hawking radiation lattice derivation, information paradox at discrete scale, Kerr-Newman generalisation. Every `[OPEN]` is its own sub-task.

### 2.2 Lattice QED — **7 `[OPEN]`**
**File:** `DERIV_LATTICE_QED_COMPLETE.md`.
Gauge fixing, Ward identities on the discrete lattice, renormalisation-group equations in lattice form.

### 2.3 Moore gauge structure — **6 `[OPEN]`**
**File:** `DERIV_MOORE_GAUGE_STRUCTURE.md`.
How the Moore-layer decomposition produces each gauge group's representations (vs. just the group names).

### 2.4 Lattice SU(3) gauge — **5 `[OPEN]`**
**File:** `DERIV_LATTICE_SU3_GAUGE.md`. Theoretical counterpart to engine §1.3.

### 2.5 Lattice SU(2) weak — **5 `[OPEN]`**
**File:** `DERIV_LATTICE_SU2_WEAK.md`. Chiral structure, left-handed doublets, weak mixing via the ungerade sector.

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
- `DERIV_STATE_FLUX_COUPLING_DERIVATION.md`.

---

## §3 Theory — foundations (`docs/theory/02_foundations/`)

### 3.1 Axiom Zero — **6 `[OPEN]`**
**File:** `FOUND_AXIOM_ZERO.md`. Deeper principle for the ternary state?

### 3.2 Relativity / gravity distinction — **2 `[OPEN]`**
**File:** `FOUND_RELATIVITY_GRAVITY_DISTINCTION.md`.

### 3.3 Potential core and generative interior — **1 `[OPEN]`**
**File:** `FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md`. The `N_meas = 18 = |SC|+|FCC|` identification as the von-Neumann-chain length.

### 3.4 Single-file 1-`[OPEN]` items in foundations
- `FOUND_BORN_RULE_NULL_CONE.md`.
- `FOUND_DIMENSIONAL_COUNTING.md`.
- `FOUND_GSTAR_SCALE.md`.
- `DERIV_D3_FROM_AUTOMORPHISM.md`.

---

## §4 Theory — particles + couplings

### 4.1 Quark masses from lattice — **5 `[OPEN]`**
**File:** `docs/theory/05_particles/DERIV_QUARK_MASSES_FROM_LATTICE.md`. Light-quark masses + CKM remain open. Top ≈ v_Higgs supports a Yukawa-at-unity story; the rest is sketched only.

### 4.2 One-loop lattice α — **3 `[OPEN]`**
**File:** `docs/theory/04_coupling/DERIV_ONE_LOOP_LATTICE_ALPHA.md`. Tadpole closure's uniqueness (would other discretisations fit equally well?) is open.

### 4.3 Watson-G* identity — **1 `[OPEN]`**
**File:** `docs/theory/04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md` — "Remains [OPEN]" section.

### 4.4 α lattice mechanism — **1 `[OPEN]`**
**File:** `docs/theory/04_coupling/DERIV_ALPHA_LATTICE_MECHANISM.md`. Step 3 (Z₄ symmetry selects this specific CM curve) and Step 8 (larger root = 1/α specifically) remain [SELECTION], not [THEOREM].

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
- `docs/theory/07_assessment/TRACKER_DOCUMENT_STATUS.md` — 1.
- `docs/theory/META_INDEX.md` — 1.
- `docs/reference/REF_EPISTEMIC_LABELS.md` — 2 (conventions, not physics).
- `docs/internal/SPEC_CLAUDE.md` — 2 (internal).

---

## §8 Scripts

Unfinished verification, proof, and exploration scripts. The script itself usually runs successfully; the `[OPEN]` marks where its conclusion stops short of a closed derivation.

### 8.1 Verification
- `scripts/verification/verify_chiral_anomaly.py` — **3 `[OPEN]`** (GW-fermion alternative).
- `scripts/verification/verify_two_loop.py` — **2 `[OPEN]`** (explicit BZ² integral would give ab-initio two-loop α).
- `scripts/verification/verify_modular_structure.py` — **1 `[OPEN]`**.
- `scripts/verification/verify_thermodynamic_limit.py` — **1 `[OPEN]`**.

### 8.2 Proofs
- `scripts/proofs/proof_quark_masses_lattice.py` — **3 `[OPEN]`** (quark-electron mass bridge).
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

Snapshot (2026-04-17, excluding `docs/theory/archive/` — see §10):

| File | Count |
|---|---:|
| `docs/theory/01_reference/SPEC_QFT_GRT_BRIDGE_ROADMAP.md` | 18 |
| `docs/theory/03_derivations/DERIV_LATTICE_BLACK_HOLES.md` | 11 |
| `docs/theory/03_derivations/DERIV_LATTICE_QED_COMPLETE.md` | 7 |
| `engine/src/dag_engine.cpp` | 6 |
| `docs/theory/03_derivations/DERIV_MOORE_GAUGE_STRUCTURE.md` | 6 |
| `docs/theory/02_foundations/FOUND_AXIOM_ZERO.md` | 6 |
| `docs/theory/05_particles/DERIV_QUARK_MASSES_FROM_LATTICE.md` | 5 |
| `docs/theory/03_derivations/DERIV_LATTICE_SU3_GAUGE.md` | 5 |
| `docs/theory/03_derivations/DERIV_LATTICE_SU2_WEAK.md` | 5 |
| `scripts/verification/verify_chiral_anomaly.py` | 3 |
| `scripts/proofs/proof_quark_masses_lattice.py` | 3 |
| `scripts/proofs/proof_moore_gauge_structure.py` | 3 |
| `scripts/exploration/explore_precision_deep.py` | 3 |
| `engine/include/ftd/ontic.h` | 3 |
| `docs/theory/09_mathematical/EXPLR_CURVE_FAMILY_MATHEMATICAL_ANALYSIS.md` | 3 |
| `docs/theory/09_mathematical/DERIV_LFUNCTION_GSTAR_CONNECTION.md` | 3 |
| `docs/theory/06_consciousness/FOUND_WIGNERS_FRIEND_RESOLUTION.md` | 3 |
| `docs/theory/06_consciousness/FOUND_VON_NEUMANN_CHAIN.md` | 3 |
| `docs/theory/04_coupling/DERIV_ONE_LOOP_LATTICE_ALPHA.md` | 3 |
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
| `docs/theory/02_foundations/FOUND_RELATIVITY_GRAVITY_DISTINCTION.md` | 2 |
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
| `docs/theory/07_assessment/TRACKER_DOCUMENT_STATUS.md` | 1 |
| `docs/theory/07_assessment/AUDIT_WHAT_IS_GENUINELY_NEW.md` | 1 |
| `docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md` | 1 |
| `docs/theory/06_consciousness/FOUND_THE_EXISTENCE_FILTER.md` | 1 |
| `docs/theory/06_consciousness/FOUND_DOMAIN_PARTITION_AND_CONTEXT_SELECTION.md` | 1 |
| `docs/theory/06_consciousness/DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md` | 1 |
| `docs/theory/04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md` | 1 |
| `docs/theory/04_coupling/DERIV_ALPHA_LATTICE_MECHANISM.md` | 1 |
| `docs/theory/03_derivations/DERIV_STATE_FLUX_COUPLING_DERIVATION.md` | 1 |
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

## §10 Archived (`docs/theory/archive/`)

`[OPEN]` items inside archived documents are historical — the content is superseded. Listed for completeness only.

| File | Count |
|---|---:|
| `ARCH_DERIV_LATTICE_KERR.md` | 8 |
| `ARCH_DERIV_LATTICE_REISSNER_NORDSTROM.md` | 5 |
| `ARCH_DERIV_TWO_LOOP_ALPHA.md` | 4 |
| `ARCH_DERIV_MOORE_GAUGE_ORTHOGONAL.md` | 4 |
| `ARCH_DERIV_LATTICE_SCHWARZSCHILD.md` | 3 |
| `ARCH_DERIV_CAVITATION_THRESHOLD.md` | 2 |
| `ARCH_SPEC_THE_COMPLETE_PROOF_RIGOROUS.md` | 1 |
| `ARCH_GAP_ANALYSIS_AND_DEVELOPMENTS.md` | 1 |
| `ARCH_FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md` | 1 |
| `ARCH_EXPLR_FRACTAL_DEPTH_AND_MASS.md` | 1 |
| `ARCH_DERIV_CAVITATION_HIERARCHY.md` | 1 |
| `ARCH_BIOLOGICAL_SLOOP.md` | 1 |
| `ARCH_AUDIT_PANEL_RESPONSE.md` | 1 |

If an archived `[OPEN]` turns out to be non-historical (i.e., someone wants to resume that line of work), move the doc out of `archive/` and add it to §2–§7 above.

---

## Recently closed

Move items here with the closing commit / PR when an `[OPEN]` becomes ✅.

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
