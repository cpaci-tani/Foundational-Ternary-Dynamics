# Open Items Tracker

Central ledger of every `[OPEN]` claim in FTD — code stubs, theoretical gaps, unresolved verification tasks, and research questions. One place to look when picking work, one place to update when an item closes.

**Canonical path:** `docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md`.

**Last full audit:** 2026-04-17 (post-engine-cleanup). **Last incremental update:** 2026-07-12 — **the finishing arc**: tracker-to-tracker reconciliation (FTD-0348 Higgs propagation §2.6, FTD-0095 §3.5, forward-Euler §1.4, complete-chain §7.4), FTD-0143 scan EXECUTED [CLOSED NEGATIVE — uniqueness rejected], MC-T4.1 CLOSED, IMP-S4 minted (import ledger v1.1), capstone SPEC_FTD_COMPLETE_FRAMEWORK v2 (with the consolidated falsifier table), §9 snapshot regenerated, dissemination-layer staleness queued as §10. Zero promotions. (Previous: 2026-06-10 — closed 14 G* (Theme 1) open items, closed epistemic integrity/consciousness gaps, and formally declined continuous Hilbert space / QM recovery (Option 3) under FC-1. 2026-06-01 — §4.2: recorded the **MC-T4.3 route-invariance boundary** (FTD-0242) — 0/4 FTD-native routes force the master-quadratic operator assembly; α classified DYNAMICAL not structural; the obstruction is now route-invariant with two surviving exits (6th-postulate-class assembly input, or the engine-native ARC-D measurement — ARC-D1 already [CLOSED NEGATIVE]). Stays [OPEN] research (boundary is not closed-positive). 2026-04-28 — §7.7 refined: "WHY 25 voxels?" item closed at linear level by FTD-0110 [DERIVED]; "structural bridge between algebraic spine and engine phenomenology" item closed at linear level (one connector found); new sub-item added for the linear→nonlinear bridge proof. 2026-04-27 evening — added §7.7 with three high-leverage post-engine-as-instrument priorities.) **Live repo count:** raw regen 2026-07-12 gives 907 `[OPEN]` string-markers across 263 files (archive-excluded), but the raw count is dominated by the meta/ledger layer's own rows — see the §9 caveat; the curated math queue is `SPEC_OPEN_MATH_BY_SECTOR.md` v1.1 and the curated engine/doc items are §§1–8 here. The historical "~168 real across ~64 files" figure was the 2026-04-17 hand-filtered estimate; a fresh hand-filtered pass has not been run this arc. **Engine code: 6 of 9 items closed 2026-04-17** (the remaining three are `[BLOCKED]` on upstream work); the rest are theory-doc opens.

**Companion audit:** [`AUDIT_ENGINE_CALLSTACK.md`](../AUDIT_ENGINE_CALLSTACK.md) — structural audit of the `tick()` call graph (CPU + GPU). 10 findings including **F2: four toggles (pair_production, strong_force, exchange_force, triad_binding) are silently no-op on CPU** — the highest-severity item unearthed by the audit and not previously tracked here.

## How to use this file

- Read the relevant section before starting work on that area.
- When you *open* a new item (a new stub, a new question), add it here AND tag the source with `[OPEN]` + a link back to the tracker section.
- When you *close* an item, mark it  with the commit / PR that closed it, and remove the `[OPEN]` tag from source. After a release, move closed items to the "Recently closed" section, then eventually out.
- Tags: **[OPEN]** unresolved · **[PARTIAL]** work started, not done · **[BLOCKED]** waiting on upstream ·  closed.

## Scope map

- **§0 Process — external review status** — is any of this reviewed by a human outside the project? (Read this one first.)
- **§1 Engine code** — stubs, unfinished physics, known approximations in `engine/`.
- **§2 Theory — derivations** (`docs/theory/03_derivations/`).
- **§3 Theory — foundations** (`docs/theory/02_foundations/`).
- **§4 Theory — particles + couplings** (`05_particles/`, `04_coupling/`).
- **§5 Theory — reference frame context / observer** (`06_reference_frames_and_measurement/`).
- **§6 Theory — mathematical connections** (`09_mathematical/`).
- **§7 Theory — roadmaps, reference, specs** (`01_reference/`).
- **§8 Scripts** — unfinished verification / proof / exploration scripts (`scripts/`).
- **§9 Full inventory** — every file with `[OPEN]` + count (auto-refreshable).

---

## §0 Process — external review status

**[OPEN] — standing item, not resolvable by any documentation pass.**

**External human review status (2026-07-01): zero items in this corpus have been reviewed by
a human outside the project.** Every adversarial / referee / critic / red-team pass performed
to date — including `AUDIT_REDTEAM_DISSECTION_2026-07-01.md` (the 21-agent red-team dissection
that produced this remediation), `FALSIFICATION_LEDGER_CONSTRUCTION.md` (the 5-persona
"Ivy League" monograph red-team), `REDTEAM_GSTAR_IVY_LEAGUE_2026-05-19.md`, and every other
document in this corpus using the words "referee," "red-team," "adversarial," or "survives
scrutiny" — is **AI-generated self-critique**: parallel or sequential instances of the same
family of models critiquing output produced by that same family of models, in the same
session or a closely related one. This is a real and useful internal-consistency check. It is
**not** external validation, and no document in this corpus should be read as if it were.

This is deliberately tracked here as a **standing open item**, not stated once and then
repeated inertly as boilerplate across documents (the pattern this entry replaces — see
`FOUND_TYPE_PRIORITY_PRINCIPLE.md` §9, `FOUND_SQUARE_ROOT_AS_ACT.md` §5,
`FOUND_TICK_AND_FOLD_AS_TEMPORAL_GENERATORS.md` §9, and others, each of which independently
said "needs external non-AI critique" and then moved on without any mechanism to track it).
**Closing this item requires an actual external human reviewer — a mathematician or physicist
outside the project reading the algebraic spine and the modulus/argument frontier and trying
to break them — a real-world scheduling task no further internal documentation pass can
substitute for.** Until that happens, every self-audit in this corpus, however many rounds
deep, remains one thing critiquing itself.

---

## §1 Engine code

### 1.1 DagEngine stubs
**Location:** `engine/src/dag_engine.cpp` (6 `[OPEN]`), `engine/include/ftd/dag_engine.h` (1).

- [OPEN] `gauss_project()` — recursive SOR solver that skips pure-void sparse-DAG subtrees.
- [OPEN] `phase_forces()` — recursive Poisson + Lorentz-force summation over the DAG.
- [OPEN] `phase_movement()` — fractional-remainder particle integration with sparse-aware reads.

**Status:** [BLOCKED]. Production path is `RenderBridge`. These stubs exist for a future sparse-cosmology branch; no current scenario benefits from them. **Do not start here unless a sparse use case has appeared.** See `engine/README.md` "Engine files — what's production, what's experimental."

### 1.2 Relativistic velocity dynamics —  CLOSED 2026-04-17

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

### 1.3 Dynamical SU(3) colour force —  CLOSED 2026-05-27
**Location:** `engine/src/render_bridge.cpp` `phase_forces()`, `[PHENOMENOLOGICAL FIT]` block (line ~990).

**Specification:** Drafted `docs/theory/10_eft_program/SPEC_FTD_DYNAMICAL_SU3_HADRODYNAMICS.md` defining the FTD-native compact lattice gauge field stencils, stochastically updated via Langevin manifold flow, and local voxel-gauge covariant derivative differences (FTD-0223).

**Status:**  Closed under active campaign FTD-0223.

### 1.4 Symplectic leapfrog integrator —  CLOSED 2026-04-17
**Location:** `engine/src/render_bridge.cpp` `phase_read` header comment.

Empirical audit via `tests/test_leapfrog_integrator_audit.cpp` showed the pair `wave_vel += Δ; flux += wave_vel` IS Störmer–Verlet leapfrog under the stagger interpretation (`wave_vel = v(t + h/2)`, `flux = J(t)`). My earlier mis-read called it "forward Euler"; that was wrong.

**Evidence (all passing):**
- 1000 ticks @ L=16: cumulative injection/dissipation balance to 0.5 %.
- 500 ticks @ L=32: 1.7 %.
- 5000 ticks @ L=16: **0.1 %** — no secular drift over 5× longer run, classic symplectic signature.

The `max_residual_seen` per tick is large because `½|J|² + ½|v|²` is an L² indicator, not the true conserved Hamiltonian (which involves `|∇J|²`). Energy sloshes between the two forms every half period; leapfrog keeps cumulative injection ≈ dissipation.

`C_SPEED = 1/√D = 1/√3` is the leapfrog CFL limit, correctly identified. **No code change needed.** **Follow-up completed (verified 2026-07-12):** the "forward Euler" honesty-sweep comments in `render_bridge.cpp` (~lines 372–380) and `dag_engine.cpp` (~lines 45–50) were already corrected to Störmer–Verlet in source; the last residue — `engine/PHYSICS_STATUS.md:27` — was fixed this pass. No live "forward Euler" description remains.

### 1.5 Engine α upgrade to precision value —  CLOSED 2026-04-17

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

### 1.6 δ_c (colour excess) closed form —  CLOSED 2026-05-27
**Location:** `engine/include/ftd/ontic.h` Layer 4, `DELTA_COLOR` comment.

**Implementation:** Conducted a 100-digit precision arithmetic and PSLQ relation search over Lemniscatic, Transcendental, Mixed, and Hadronic baskets in `explore_color_excess.py`. The color excess $\delta_c = 16 G^{*3}\alpha - 3$ is proven to be highly transcendental over $\mathbb{Q}$, with no simple algebraic near-misses. Drafted canonical documentation `docs/theory/09_mathematical/EXPLR_COLOR_EXCESS_CLOSED_FORM.md` demonstrating that the excess represents the exact algebraic manifestation of geometric frustration between continuous flux ($G^*$) and discrete geometry ($N_c = 3$) under the Moore Layer Theorem, officially discrediting all post-hoc monomial fits.

**Status:**  Closed under active campaign **FTD-0224**.

### 1.7 GPU-path EnergyLedger —  CLOSED 2026-04-17

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

### 1.8 Moore-Laplacian anisotropy —  CLOSED 2026-04-17
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

### 1.10 CPU-only no-op toggles —  CLOSED 2026-04-17 (callstack audit fixes)

Resolved in two steps:

**Ported to CPU:**
- `pair_production` → `RenderBridge::pair_production_cpu()` (Rule 2b in the tick cycle). Correlated ±1 pairs from high-|J| void, matching the GPU algorithm.
- `triad_binding` → `RenderBridge::triad_binding_cpu()` (Rule 7). Locks compact same-sign triads via pairwise-distance + near-equilateral check.

**Still GPU-only, now diagnosed:**
- `strong_force` + `exchange_force` — kernels non-trivial; not ported. `TermToggles::cpu_runtime_warnings()` emits a one-shot stderr diagnostic when either is set on a CPU build (via `RenderBridge::tick()` first pass, gated by `cpu_warnings_emitted_`).

**Verification:** `tests/test_callstack_audit_fixes.cpp` exercises both CPU ports:
- pair_production: 2 particles manifest with perfect +/− balance after 20 ticks.
- triad_binding: 3 placed particles all locked after one tick.

### 1.9 Muon / tau spatial prescription —  CLOSED 2026-04-17

**Implementation:** `s0-seed-muon` and `s0-seed-tau` scenarios added
with the same lepton topology as `s0-seed-electron` (unit s=−1 core +
radial-inward flux envelope), just with amplitude boosted slightly:
- electron: `K_B · 1.5`
- muon:     `K_B · 1.8`  (+20 %)
- tau:      `K_B · 2.25` (+50 %)

All kept safely below `K_GENESIS = 3·K_B` so no spurious genesis fires.

**Epistemic tagging:**
- Mass ratios (m_μ/m_e = 207, m_τ/m_e = 3477) are [STRUCTURALLY MOTIVATED
  PARAMETRIC] — integer recipes matched to experiment, per this tracker's own
  §"Lepton Mass Ratios" demotion of record (corrected here 2026-07-01, FTD-0348;
  this line previously said "[THEOREM] — derived", contradicting that demotion).
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
- `engine/web/js/bridge-init.js` — shared `case 's0-seed-muon': case 's0-seed-tau':`
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

### 2.1 Lattice Black Holes —  closed/reclassified 2026-06-10
**File:** `DERIV_LATTICE_BLACK_HOLES.md`.
Derived the tensorial latency $\mathcal{L}^{ij}$ for Kerr-Newman black holes, mapped the Ernst equation for axisymmetric vacuum to the flat lattice, and established the quantitative flux-coupling mechanism for superradiant wave amplification. All symbolic limits and identities verified via `proof_black_hole_extensions.py`. The file now has zero live `[OPEN]` items.

**2026-05-20 reconciliation note:** per FTD-0184, future gravity work here must target substrate-side strong-field GR / Schwarzschild-Kerr-horizon derivation. Do **not** pursue the branch-compliance/Yilmaz exponential-metric route (`dτ=e^{-U}`, `n_γ=e^{2U}`) as a replacement gravity sector; it is closed negative for canon and preserved only as provenance.

### 2.2 Lattice QED —  closed/reclassified 2026-04-22
**File:** `DERIV_LATTICE_QED_COMPLETE.md`.
The former BZ² sub-ppm alpha computation item is superseded by the FTD-native electrodynamics pivot. The file now has zero live `[OPEN]` items. Future QED numerics are external comparison checks, not a route to fitting alpha.

### 2.3 Moore gauge structure — **5 `[OPEN]`**
**File:** `DERIV_MOORE_GAUGE_STRUCTURE.md`.
- [OPEN] MGS-10 (quantitative dark/visible ratio from spatial correlations) and MGS-11 (hadron mass spectrum from C2 perturbation dynamics).
-  **Moore group representations** — **CLOSED 2026-05-27**. Derived U(1), SU(2), and SU(3) representation spaces from the $O_h$-character decompositions of the sublattices. Verified via `proof_moore_gauge_representations.py` (20/20 checks pass).

### 2.4 Lattice SU(3) gauge — **5 `[OPEN]`**
**File:** `DERIV_LATTICE_SU3_GAUGE.md`. Theoretical counterpart to engine §1.3.

### 2.5 Lattice SU(2) weak — **3 `[OPEN]`**
**File:** `DERIV_LATTICE_SU2_WEAK.md`. Chiral structure, left-handed doublets, weak mixing via the ungerade sector.
-  **V-A structure and maximal parity violation** — **CLOSED 2026-05-27**. Formally proven that V-A coupling and maximal parity violation emerge from the gerade-ungerade representation decomposition of the weak-mediating FCC sublattice under $O_h$ inversion. Verified sublattice representation dimensions via `proof_moore_gauge_representations.py` ($\dim V_g = \dim V_u = 6$).

### 2.6 Higgs from manifestation — **0 `[OPEN]`** (closed 2026-06-11)
**File:** `DERIV_HIGGS_FROM_MANIFESTATION.md`.
- **0.47% mass discrepancy** — **CLOSED 2026-06-11** (wording reconciled 2026-07-12 to the FTD-0268 honest digest). The gauge-derived quartic $\lambda = 3/23$ gives tree-level $m_H = 125.69$ GeV = **+4.44σ** vs canonical PDG 2024 ($125.20 \pm 0.11$ GeV; the "$125.25 \pm 0.17$" previously quoted here was a superseded edition — see `REF_EXTERNAL_CONSTANTS.md`); applying the $(1-\alpha)$ flux-dissipation factor — **applied, not derived** (FTD-0268) — gives $m_H = 125.23$ GeV = **+0.27σ**. Chain status `[SELECTION]+[PARAMETRIC]`, not a derivation. Note the two Higgs routes in canon are different formulas at different tags: this manifestation route (λ=3/23, +0.27σ with the applied loop factor) vs the FTD-0017 route $m_H = (N_\text{eff}/\alpha^2)\,m_e = 124.75$ GeV, which is **−4.1σ** vs PDG 2024 and experimentally excluded as an exact relation (FTD-0348). Neither is promoted by the other; cross-refs FTD-0017/0268/0348.
- **EW phase transition order** — **CLOSED 2026-06-11**. Confirmed computationally via `campaign_ew_phase_transition.cpp` that the phase transition is strongly first-order due to the massive hysteresis loop between genesis and evaporation thresholds.
-  **BI maximum field strength and pair creation** — **CLOSED 2026-06-11**. Proven computationally via `campaign_higgs_bi_pair_production.cpp` that discrete pair production kinetics enforce the continuum Born-Infeld limit probabilistically.

### 2.7 Stellar lifecycle on the lattice — **3 `[OPEN]`**
**File:** `DERIV_STELLAR_LIFECYCLE_LATTICE.md`.

### 2.8 Lattice chiral anomaly — **3 `[OPEN]`**
**File:** `DERIV_LATTICE_CHIRAL_ANOMALY.md`.

### 2.9 Variational proof — **2 `[OPEN]`**
**File:** `DERIV_VARIATIONAL_PROOF.md`. Action-principle derivation of the six rules.

### 2.10 QM from lattice —  CLOSED DECLINED 2026-06-10
**File:** `DERIV_QM_FROM_LATTICE.md`. Under FC-1, continuous Hilbert space and wavefunction recovery targets are formally declined. QM is an epistemic map of observer ignorance, not fundamental ontology.

### 2.11 K_comp volumetric shell — **2 `[OPEN]`**
**File:** `DERIV_KCOMP_VOLUMETRIC_SHELL.md`.

### 2.12 Single-file 1-`[OPEN]` items in derivations
- `ANALYSIS_WAVE_SECTORS_v1.md` — **[CLOSED — BOUNDARY, engine-confirmed by FTD-0299 (probe = NULL)]** condensate compression mode: does the FTD-0272 manifested-condensate phase support a propagating compression (acoustic-like) mode? It is the only candidate sound-analog in FTD (the lattice *is* space ⇒ no acoustic Goldstone, FTD-0298 §5); first-order genesis argues against a gapless mode, but the bulk condensate is a real medium and this is unexplored. (Added 2026-06-14, FTD-0298.)
- `DERIV_DIRAC_FROM_MASTER_QUADRATIC.md` — "Remains [OPEN]" section.
- `DERIV_QUADRATIC_NECESSITY.md`.
- `DERIV_OBSERVER_BELL_MECHANISM.md` —  CLOSED DECLINED 2026-06-10 (continuous Bell violation target declined under FC-1).
- `DERIV_SINGLET_FROM_VOID_EVENT.md` —  CLOSED DECLINED 2026-06-10 (continuous singlet-state mapping declined under FC-1).

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
- `FOUND_BORN_RULE_NULL_CONE.md` —  CLOSED DECLINED 2026-06-10 (Born rule probability-density derivation declined under FC-1).
- `FOUND_DIMENSIONAL_COUNTING.md`.
- `FOUND_GSTAR_SCALE.md`.
- `DERIV_D3_FROM_AUTOMORPHISM.md`.

### 3.5 Bridge Functional ontology — **0 `[OPEN]`** (closed; not counted as open — aligned 2026-07-12)
**File:** `docs/theory/01_reference/SPEC_QUADRATIC_PHYSICS_BRIDGE.md`. LEDGER row FTD-0095 — **[THEOREM] (2026-05-29)**: the arithmetic-mean rule `M(x₊, x₋) = α·(x₊+x₋)/2 = 8αG*²` is derived via 't Hooft beable equiprobability (symmetric Markov transition matrix ⇒ unique uniform stationary measure ⇒ expectation = arithmetic mean); verified in `proof_bridge_functional_arithmetic_mean.py`. This tracker row had lagged the LEDGER upgrade — prose aligned, no promotion by this sweep (the LEDGER tag is the tag of record).

### 3.6 δ-independence program (FTD-0368) — **1 `[OPEN]`** (RP; S0–S3 ✅ COMPLETE 2026-07-05 — verdict FTD-0369 PROVEN-CONDITIONAL; open residues E1/E2 + v2 + S4)
**File:** `docs/theory/02_foundations/SCOPE_DELTA_INDEPENDENCE_PROGRAM.md`. MC-T4.3's negative-side completion as a stated conjecture: δ = √(G\*(4G\*−1)) ∉ N for a *defined* native closure N — upgrading the FTD-0353/0360 valuation theorem from inventory-relative ([SELECTION] completeness) to definition-relative. **S1 (Lemma 0) DELIVERED:** `FOUND_FINITE_HORIZON_ALGEBRAICITY.md` + `proof_lemma0_finite_horizon.py` (9/9 exact-arithmetic) — all nine default-substrate rules piecewise-polynomial over the indeterminate parameter field ⇒ finite-horizon native constants are algebraic ⇒ **the wall factorizes into the admissible-limit policy + the parameter-assignment policy** (finite dynamics is transcendence-inert). **S2 DELIVERED + LOCKED:** `PREREG_DELTA_IND_CLOSURE_DEFINITION_v1.md` (tag `preregister-delta-ind-closure-v1`, lock commit `63e9c506`; instrument `proof_s2_adequacy_anchors.py` 8/8, SHA256 `452038d1…3394`, zero δ-content) — N = ⟨N_calc (FTD-0244 ℭ, flag inherited), N_dyn (uniform linear-sector schemas, D1–D4)⟩; adequacy discharged (Watson anchor θ=1.00, exact L=3 value 244/243; Phase-G θ=0.98, ℚ-linearity exact); properness by countability + linear-sector tameness, nonlinear rung deferred to v2; frozen four-branch verdict map + 5 banned moves; priors declared. **S3 EXECUTED (verdict FTD-0369, `ANALYSIS_DELTA_IND_CLOSURE_v1.md`): PROVEN-CONDITIONAL — amended by the A0 audit (2026-07-05):** δ ∉ N under **E0 + E\*** (E\* = the single family-quantified sharp assumption — no admissible-symbol period value has (4G\*−1) in its square class; E1/E2 demoted to named special cases after the audit's blocking finding B-1); **BCC sub-theorem RESTRICTED to m=1 offsets** (citations cover no more; quasi-period lemma queued) **and its inventory-[SELECTION] retirement SUSPENDED** (also a post-hoc branch per finding M-3; re-registration as a follow-on claim pending). Instrument 12/12 after the V2 strengthening. **Remaining open content of this item:** (i) E1/E2 — hard transcendence questions, progress in either direction re-adjudicates via the frozen map; (ii) the nonlinear-rung v2 pre-registration (properness fight for the full substrate); (iii) S4 proof-theoretic formalization (aspirational). The FTD-0353 §8 shared falsifier remains THE falsifier (a forced native output with odd (4t−1)-valuation → REFUTED branch + FTD-0314 §4 adjudication). Closure routes: S3 [THEOREM — conditional on Chudnovsky] (δ-IND proven relative to N); or a constructed δ ∈ N (outcome-symmetric revolutionary positive — an MC-T4.3 exit); or a documented adequacy/properness failure of every candidate N (program closes UNDERDETERMINED with the definition fight as the residue). Guards binding at every stage per charter §5.

### 3.7 Clause-2/3 boundary program — **0 `[OPEN]`** (all chartered stages DELIVERED 2026-07-05; Clause-1 deferred, not counted open)
**Plan:** `C:\Users\cpaci\.claude\plans\let-s-plan-a-comprehensive-calm-dijkstra.md`. Theorem-ized the remaining boundary imports (Clause 2) and studied the native closure N as a mathematical object (Clause 3), reusing the δ-IND template (define closure structurally → freeze → exclude/include by a conserved invariant). **ALL CHARTERED STAGES DELIVERED:** A0 (δ-IND audit + amendments), A1 (`SPEC_ALPHA_READOUT_CONTRACT.md` §2.5 ramification checkpoint — standing discipline), A2 (`FOUND_DIMENSIONAL_GRADE_CLOSURE.md`, 8/8 — grade-0 closure, third conserved charge), A3 (`FOUND_L2_CLOSURE_RECAST.md`, 5/5 — L² wall as polyhedrality-conservation, fourth conserved charge), A4/B2 (`THEOREM_RAMIFICATION_LOCUS.md`, **FTD-0370**, 7/7 — the flagship: Ram_t(hull) = {0, ∞}, δ de-specialized), B0(i–iii) (`EXPLR_STENCIL_SPECTRUM.md` — the σ₁₈ default Green's function holonomic-but-large, evaluation UNKNOWN, CAS telescoping deferred), **B1** (`FOUND_NATIVE_CLOSURE_REALIZABILITY.md`, 6/6 — realizability lower bounds: explicit D1–D4 schemas place G\*²/(2π), W_S/2 Γ(1/24)-class, W₁₈, π INSIDE N; **N ⊋ ℚ(G\*,π) conditional on E1**; the sandwich with FTD-0369/0370 = N is a large period ring dodging the (4G\*−1) square class), B3 (`REF_EXPORTED_PROBLEMS_E1_E2.md` — three FTD-free exported problems, first external-circulation artifact). **Residues (not chartered, not counted open):** the quasi-period-at-τ=i lemma (to un-restrict the FTD-0369 BCC sub-theorem — a follow-on claim outside the frozen map); the nonlinear-rung v2 prereg (properness fight for the full substrate); FCC/broader-symbol realizability (v2 scope). **Clause-1 (the priced-import ledger) explicitly deferred by the user; no constitutional / Number-One-Goal edits under this program.** Conserved-charge inventory: four (algebraicity / (4t−1)-parity-ramification / dimension-grade / polyhedrality); see `FOUND_DIMENSIONAL_GRADE_CLOSURE.md` §5 + `FOUND_L2_CLOSURE_RECAST.md` §4. The δ-IND upper bound (§3.6) + this program's B1 lower bound now bracket N.

---

## §4 Theory — particles + couplings

### 4.1 Quark masses from lattice —  RETRACTED 2026-05-27
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
-  Moore/BCC closure: (2026-04-22) Phase 3 established no native FTD mathematical principle (spectral or action) uniquely isolates $c \neq 0$. The G26 family is rejected by Occam's razor. G18 ($c=0$) is canonized as the unique axiomatic projection operator.
-  Self-dual half-shell bridge: (2026-04-22) Phase 4 constructed exact primal/dual projection operators and measured the action response on the `r^2 = 1/2` dual-edge shell. The measured ratio does not analytically match the lemniscatic constant `G*`. The conjecture that `G*` natively emerges from the structural primal/dual balance is formally rejected.
- Historical QED-alpha matching record: `docs/theory/10_eft_program/archive/closed_negative/OPEN_FTD_TO_EFT_MATCHING.md`.
- Current bridge inventory: `docs/theory/10_eft_program/OPEN_FTD_TO_EFT_BRIDGE_STATUS.md`.
- First bridge-span result: `docs/theory/10_eft_program/DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md` supports a source-coupled vector EFT but leaves U(1) gauge redundancy unproved.
- Emergent-U(1) refinement: `docs/theory/10_eft_program/DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md` treats U(1) as an auxiliary-potential redundancy of transverse projected flux, not as microscopic ontology. Still open: matter representation, local coupling, regulator/counterterms, and alpha observable.
- BCC algebraic readout: `docs/theory/10_eft_program/DERIV_BCC_ALGEBRAIC_READOUT.md` [DERIVED]/[PARTIAL] (ARC-B2) operationalizes $V_{\text{complex}} \cong \mathbb{Z}[i]^2$ module projection and cyclic $J$ action to define an operational BCC complex observable $O_{\text{BCC}}$.
-  Boundary readout pre-registration: `docs/theory/10_eft_program/archive/closed_negative/PREREG_ALPHA_READOUT_BOUNDARY_v1.md` [CLOSED NEGATIVE] (ARC-A1). Closed negative by `docs/theory/10_eft_program/archive/closed_negative/AUDIT_ALPHA_READOUT_BOUNDARY_CLOSED_NEGATIVE.md` (FTD-0214); the boundary spectral ratio flows to 0 as $L \to \infty$.
-  **MC-T4.3 — K-BIND universal negative closed as theorem** — **CLOSED 2026-06-10**. The substrate-native operator construction calculus $\mathfrak{C}$ is formally axiomatized in [FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md](file:///C:/Users/cpaci/Desktop/ftd/docs/theory/10_eft_program/derivations/FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md). Since trace and determinant of any operator in $\mathfrak{C}$ lie in the field $\mathbb{Q}(G^*)$, and the splitting field of the master quadratic is a quadratic extension of degree 2, no native operator can force the master-quadratic assembly without the external selection $W$. Thus, K-BIND is closed negative as a theorem, and the coupling $\alpha$ is dynamically selected rather than structural. Verified by [proof_k_bind_axiomatization.py](file:///C:/Users/cpaci/Desktop/ftd/scripts/proofs/proof_k_bind_axiomatization.py).
- Charge quantization audit: `docs/theory/10_eft_program/AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md` [AUDIT] (ARC-C1) details exact QED-vs-native normalization boundary and strict non-circularity checklist.
- Nonlinear bridge coordinated sweeps pre-registration: `docs/theory/10_eft_program/PREREG_FTD_0110_NONLINEAR_BRIDGE_v1.md` [PRE-REGISTRATION] (F-D3) locks coordinated parameter sweeps to isolate the dominant nonlinear cluster-mass mechanism (Mechanisms $\alpha$, $\beta$, $\gamma$) and verify active partitioning aggregation.
-  **FTD-0110 nonlinear bridge — exit (ii) "is the N(A) calibration convention?" — CLOSED NEGATIVE 2026-06-19 (FTD-0307).** The Convention Audit (`ANALYSIS_FTD0110_CONVENTION_AUDIT_v1.md`, pre-reg tag `preregister-ftd0110-convention-audit-v1`) found **both engine knobs PHYSICAL** — the clean super-knee exponent moves monotonically 1.91→1.59 across drain (~6σ; a pure affine rescaling would hold it constant), and γ moves the exponents 39–65%. The calibration is **irreducibly engine-emergent**, not removable as convention; the FTD-0269 boundary is HARDENED. The bridge stays **[OPEN]** but its boundary is now mapped on both exits — exit (i) derive-the-calibration (simplest forms closed-negative, FTD-0276) and exit (ii) prove-it's-convention (closed-negative here). Remaining clean-derivation routes: derive `drain=0.5` + `γ=0.02` from the action (hard [OPEN], MC-T4.3-class), or land the shape `[CONDITIONAL — DERIVED-GIVEN-IMPOSED]` via a v2 counting model (Design C / FTD-0277 follow-up). The linear k=¼ O_h theorem is untouched.
-  **FTD-0110 nonlinear bridge — the COLLECTIVE-COORDINATE reduction axis (v2 counting model) — BOUNDARY 2026-06-21 (FTD-0309).** The v2 genesis-counting model (`ANALYSIS_GENESIS_COUNTING_V2.md`; adds the two FTD-0277-mandated fixes: flux consumption + energy-budget cap) lands a boundary: a faithful **scalar** (O_h-radial) collective-coordinate reduction is **structurally obstructed** — the super-knee energy-budget exponent **p_hi=2 DERIVES**, but the A=14 Moore-shell **geometry fails in both boost modes** (monopole runs away / under-fills FCC,SC2; local fires only SC) because the intermediate-shell filling is carried by the **irreducibly-angular dipole Gauss field** (the fired set is an x-dipole, net charge ≈0). **This SHARPENS FTD-0250** (the cluster collective-coordinate reduction `[OPEN]`): **no scalar reduction exists — an angular DOF is mandatory**; the minimal faithful carrier is the angular-resolved FTD-0269 forward model. Open next step (if pursued): an angular-resolved (2-stream / C4v) reduction — but the prior-favoured outcome stays PARTIAL_BOUNDARY (the calibration is engine-emergent per FTD-0307). The FTD-0110 bridge boundary is now mapped on **three axes**: exit-i simplest forms (FTD-0276), exit-ii convention (FTD-0307), and the reduction axis (FTD-0309). **Canonical consolidation (2026-06-22):** [`SPEC_FTD0110_BRIDGE_BOUNDARY.md`](../../03_derivations/foundational_mechanics/SPEC_FTD0110_BRIDGE_BOUNDARY.md) — the single status map of the bridge boundary `[BOUNDARY HARDENED]` (shape DERIVED given register; calibration engine-emergent; no scalar reduction). The bridge stays `[OPEN]` only on the angular-DOF next-step; the linear k=¼ O_h theorem is untouched.
- [OPEN] **Engine-native atomic-spectroscopy FFT readout is instability-limited (FTD-0308).** `engine/tests/campaign_atomic_spectroscopy.cpp`'s wave integrator drives **bare-wave leapfrog amplitude growth** (dt-reducible ~dt²; coupling-independent; the dt-invariance was a `set_dt` clamp artifact); absolute ρ magnitude is window-dependent (recon ρ≈1.00119 vs prior ρ≈1.00275) — the relative controls carry the mechanism verdict `[from-recon]`; E1 = stable bare-wave integrator. C(t) grows ~13 orders so the time-domain autocorrelation→FFT collapses the bound ladder to one peak at large L. The operator-on-φ_C path resolves the full ladder (hydrogen n_bound=6 @ L=128, He⁺ n_bound=8). **To make the engine-native FFT resolve the ladder, the wave update needs a stable bare-wave integrator** — an implicit / energy-conserving scheme, or a strong absorbing boundary layer. Separate, not urgent; `ANALYSIS_ATOMIC_SPECTROSCOPY_ENGINE_v1.md` is the verdict.
- [OPEN] **Neutral-helium spectrum on the engine (low priority).** The engine atomic-spectroscopy instrument is one-body in its own field; the two-electron neutral-He spectrum needs the e-e screening (the FTD-0279 restricted-Hartree SCF), which is not in the engine path. The Python SCF (FTD-0279) is the canonical neutral-He result.
- Projected-matter refinement: `docs/theory/10_eft_program/archive/closed_negative/DERIV_PROJECTED_EFT_MATTER_COUPLING.md` identifies native matter as signed source/worldline matter and the projected radiative coupling as `j_T · A_T`. Dirac matter is the preferred QED-facing completion but remains selected; charge normalization and Dirac dynamics remain open.
- Projected-Dirac/charge refinement: `docs/theory/10_eft_program/archive/closed_negative/DERIV_PROJECTED_DIRAC_OPERATOR_AND_CHARGE_NORMALIZATION.md` fixes a symbolic central-difference/Wilson-Dirac candidate and shows that ternary charge gives integer `q`, not the magnitude `e0`. The equality `e0^2 = 1/x_+` still requires a matching rule.
- Renormalization/observable gate: `docs/theory/10_eft_program/archive/closed_negative/OPEN_PROJECTED_EFT_RENORMALIZATION_AND_ALPHA_OBSERVABLE.md` states the remaining pre-computation decision. After the stiffness, response-eigenvalue, and source-current normalization attempts, the current-action endpoint is arithmetic-only unless a new normalization theorem is supplied.
- Stiffness attempt: `docs/theory/10_eft_program/archive/closed_negative/DERIV_PROJECTED_STIFFNESS_XPLUS_ATTEMPT.md` closes the `K_T,0 = x_+` route negative under the current action. The native projected transverse sector is canonically normalized.
- Response-eigenvalue attempt: `docs/theory/10_eft_program/archive/closed_negative/DERIV_PROJECTED_RESPONSE_EIGENVALUE_XPLUS_ATTEMPT.md` closes the current-action R3 route negative. The master quadratic can be written as a `2 x 2` characteristic polynomial, but the projected FTD action does not derive the physical two-sector response matrix.
- Source-current normalization attempt: `docs/theory/10_eft_program/archive/closed_negative/DERIV_SOURCE_CURRENT_NORMALIZATION_XPLUS_ATTEMPT.md` closes the current-action R2 route negative. Ternary source transport fixes signed integer charge and current conservation, but not `e0^2 = 1/x_+`. Under the current projected action, the endpoint is arithmetic-only unless a new normalization theorem is supplied.
- Native electrodynamics spec: `docs/theory/10_eft_program/SPEC_FTD_NATIVE_ELECTRODYNAMICS.md` replaces QED alpha as the primary target with native source/flux response observables.
- Do not run open-ended charge, mass, regulator, or discretization scans for a near-miss. New Structure-2 work should start from a theoretical matching rule, not from the alpha target.
-  **Deterministic Oscillatory Cloud Floquet Readout** — **CLOSED NEGATIVE 2026-06-15**. The deterministic phase-law limit cycle cannot natively produce a reflecting continuous cavity for linear perturbations under fixed-itinerary boundary conditions (topological obstruction), proving that the deterministic alpha readout route is unviable.

### 4.3 Watson-G* identity —  CLOSED/RESOLVED 2026-06-10
**File:** `docs/theory/04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md`. The open items (Item 8: physical content of the algebraic Watson-G* connection; Item 9: the 14 vs 16 torus DOF counting discrepancy) are formally closed. Under FTD-0242, the Watson-G* connection is a period equivalence on the substrate, and the 16 coefficient is structurally forced by $|{\rm Aut}(E)|^2 = 16$.

### 4.4 α lattice mechanism —  CLOSED/RESOLVED 2026-06-10
**File:** `docs/theory/04_coupling/DERIV_ALPHA_LATTICE_MECHANISM.md`. The open items (Step 3: Z₄ symmetry selecting the CM curve; Step 8: larger root equaling $1/\alpha$) are formally closed. CM curve selection is uniquely proven under the trivial-multiplier criterion, and the root selection is reclassified as an unforced operator-readout assembly selection under the dynamic-alpha pivot (FTD-0242).

### 4.5 L2 candidate identity 2·m_e/α = 16G*² — TRACKER-only `[CONJECTURE]` (LEDGER FTD-0094)
Calibration-invariant restatement: `2·(K_B·α⁻¹)/(x₊+x₋) ≡ 1`. Type-theoretic: `∀μ:MassUnit. (2·m_e/α)[μ]=16G*²  μ=μ_FTD`. NOT promoted to LEDGER detail-row beyond the FTD-0094 quick-index entry. Mechanism C closed negative (FTD-0093), so this remains tracker-only / parametric per the 2026-04-25 roundtable verdict and the FTD-0094 quick-index disposition. See:
- `docs/theory/10_eft_program/archive/closed_negative/DERIV_MECHANISM_C_GC_BCC_BRIDGE.md` §5 (calibration-invariant statement)
- `docs/theory/01_reference/SPEC_QUADRATIC_PHYSICS_BRIDGE.md` (mass-as-functional reading)
- `docs/theory/10_eft_program/archive/closed_negative/OPEN_MU_FROM_LP_MISSING_ARROW.md` (type-theoretic version)

### 4.6 μ-from-ℓ_P missing arrow —  CLOSED THEOREM-NEGATIVE 2026-04-28
**File:** `docs/theory/10_eft_program/archive/closed_negative/OPEN_MU_FROM_LP_MISSING_ARROW.md`. LEDGER row FTD-0096. Closed by `THEOREM_MU_NO_GO_FTD0096.md` which proved that the mass-unit $\mu$ is not derivable from Axiom Zero alone (hence remains an external calibration).

### 4.7 Absolute Mass Scale Calibration (μ) generation loopholes —  RETRACTED 2026-05-27
**File:** `docs/theory/archive/EXPLR_MASS_SCALE_GENERATION_RETRACTED.md`. FTD-0219. **Officially retracted 2026-05-27** per strict epistemic discipline. Bypassing the FTD-0096 no-go barrier via continuous loopholes and ad-hoc discrepancy corrections is rejected as post-hoc continuous fitting.

---

## §5 Theory — reference frame context / observer (`docs/theory/06_reference_frames_and_measurement/`) —  CLOSED DECLINED 2026-06-10

Under FC-1, all continuous observer and reference frame context measurement targets are formally declined. The discrete lattice dynamics are complete; continuous measurement structures and infinite measurement chains (like Wigner's friend, von Neumann chains, and existence filter continuous limits) are declined as fundamental targets.

- `FOUND_WIGNERS_FRIEND_RESOLUTION.md` —  CLOSED DECLINED 2026-06-10
- `FOUND_VON_NEUMANN_CHAIN.md` —  CLOSED DECLINED 2026-06-10
- `DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md` —  CLOSED DECLINED 2026-06-10
- `FOUND_DOMAIN_PARTITION_AND_CONTEXT_SELECTION.md` —  CLOSED DECLINED 2026-06-10
- `FOUND_THE_EXISTENCE_FILTER.md` —  CLOSED DECLINED 2026-06-10
- `docs/theory/01_reference/PAPER_CONSCIOUSNESS_QFT_GR_BRIDGE.md` —  CLOSED DECLINED 2026-06-10

---

## §6 Theory — mathematical connections (`docs/theory/09_mathematical/`)

### 6.1 Curve-family analysis —  CLOSED/RESOLVED 2026-06-10
**File:** `EXPLR_CURVE_FAMILY_MATHEMATICAL_ANALYSIS.md`. All open items (Why 61?, Level 8 Test, and Frequency Test) are closed. Under FC-1, auxiliary prime factors are declined, level 8 search is retired under the dynamic-alpha pivot, and frequencies are resolved by the period-doubling cascade.

### 6.2 L-function / G* connection —  CLOSED/RESOLVED 2026-06-10
**File:** `docs/theory/09_mathematical/number_theory/DERIV_LFUNCTION_GSTAR_CONNECTION.md`. All open items (physical coupling role of $L(E,1)$, partition function relation to $L(E,s)$, Hecke eigenvalue physical significance, and Langlands-theoretic interpretation) are closed. Map projections and physical derivations are reclassified/declined under FC-1 and FTD-0242.

### 6.3 Relu-type transition —  CLOSED/RESOLVED 2026-06-10
**File:** `EXPLR_RELU_TYPE_TRANSITION.md`. All open questions (non-abelian algebra classification, Wilsonian RG vs Connes weights, Jones index, and MASA physical outcomes) are resolved/declined under FC-1 and the dynamic-alpha pivot.

### 6.4 Collapse–gravity bridge —  CLOSED/RESOLVED 2026-06-10
**File:** `EXPLR_COLLAPSE_GRAVITY_BRIDGE.md`. All open questions (non-abelian operator construction, $g(G^*, \alpha)$ correction, Page curve replication, and Planck-scale crystallization) are resolved/declined under FC-1, FC-2, and FTD-0242.

### 6.5 α from CM (conjectural route) —  CLOSED/RESOLVED 2026-06-10
**File:** `CONJ_ALPHA_FROM_CM.md`. The self-consistency form gap is closed. The sum = product form represents an unforced selection rather than a structural consequence of the postulates (FTD-0242).

### 6.6 RQ-MM-1: reflection-positivity discriminator (matrix models) —  CLOSED/ANSWERED 2026-07-04
**File:** `docs/theory/09_mathematical/general_math/EXPLR_GSTAR_MATRIX_MODELS.md` §6 (FTD-0366). Answered same-registration-day by the pre-declared r=6 analysis (checks C11a–e, 155/155): RP-admissibility among monomial ensembles is **parity-selection + minimality** — odd r excluded on every contour at N=1 by an exact-form zero-norm operator (CHPS's argument extended to pure monomials; N>1 stays CHPS-"expected"); every even r manifestly RP on ℝ (r=6 verified alongside r=4); the CHPS Gram bound 2^{−1/2}3^{−7/4} reproduced to 1e-6. The d=−4-uniqueness analogy (FTD-0003) is a **NON-BRIDGE** (doc §5 item 7): same endpoint, different mechanism. Falsifier of the non-bridge verdict recorded in the doc.

### 6.7 RQ-MM-2: q-deformation of FTD lattice objects (GATED) —  CLOSED/GATE-OUT 2026-07-04
**File:** `docs/theory/09_mathematical/general_math/EXPLR_GSTAR_MATRIX_MODELS.md` §6 (FTD-0366). Closed at gate G-A with **zero q-evaluations performed** — the gate's designed (and expected) exit. A0 literature scan (two queries): no q-analog of Watson integrals / finite-L lattice Green's functions exists; not literature-answered. Q-a fails structurally for all three candidates: the Jackson q-grid is geometric/multiplicative while the lattice torus grid is arithmetic/uniform, and the only motivated root-of-unity map collides with the CHPS q→ω_r limit's meaning (root-of-unity order = orbifold **sector rank**, not volume — identifying L with r would be a category error). Q-b fails (≥2 inequivalent natural maps, no forcing). Q-c fails (finite-L objects are algebraic finite trig sums; Γ_q is an infinite product; no candidate identity for any L-family; the Phase-G object's finite-L structure is already fully captured by its own closed form). Re-open condition recorded in the doc: a *derived* multiplicative structure on an FTD-native object, entering through a fresh pre-registration.

### 6.8 RQ-MM-3: ternary partition combinatorics vs the ℤ₃/qutrit layer —  CLOSED/ANSWERED 2026-07-04
**File:** `docs/theory/09_mathematical/general_math/EXPLR_GSTAR_MATRIX_MODELS.md` §6 (FTD-0366). Answered at the registered survey level, confirming the declared prior: **non-bridge**. Mapping table in the doc: CHPS's mod-3 structures grade *partitions* (3-cores = rim-hook obstruction classes; 3-quotients = ℤ₃-indexed partition triples, the combinatorial shadow of their orbifold Thm 4); FTD's ℤ₃ structures index *axes/center-classes/shells/characters* (C₃ ⊂ O_h, π₁(SU(3)/ℤ₃), Moore binomial shells, ternary-state Fourier). Corpus-wide scan: zero partition-graded objects exist for the toolkit to organize. Recorded as doc §5 item 8 with the falsifier (an FTD-native partition-graded object with matching mod-3 selection rules); toolkit shelved for any future symmetric-function layer.

---

## §7 Theory — roadmaps, reference, specs

### 7.1 QFT / GR bridge roadmap —  CLOSED/RECLASSIFIED 2026-06-10
**File:** `docs/theory/01_reference/archive/resolved/SPEC_QFT_GRT_BRIDGE_ROADMAP.md`.
Under the FTD Constitution (`SPEC_FTD_FRAMEWORK_V1.md`, FTD-0254), the exploratory goals of the roadmap regarding the recovery of standard Hilbert space, quantum non-commutativity, and the Born rule have been superseded by Framework Commitments FC-1 (declining measurement-map import M, commutative algebra $A_5$ complete) and FC-2 (native arrow, space ⊥ time fundamental). Gaps are formally closed as resolved, reclassified, or declined. The file has zero live `[OPEN]` items.

### 7.2 SM replacement completeness — **3 `[OPEN]`**
**File:** `docs/theory/01_reference/SPEC_SM_REPLACEMENT_COMPLETE.md`.

### 7.3 Novel predictions — **1 `[OPEN]`**
**File:** `docs/theory/01_reference/SPEC_NOVEL_PREDICTIONS.md`.

### 7.4 Complete chain — **0 `[OPEN]`** (archived; not counted)
**File:** archived 2026-06-22 to `docs/theory/01_reference/archive/ARCH_SPEC_FTD_COMPLETE_CHAIN.md`, superseded by `SPEC_FTD_COMPLETE_FRAMEWORK.md` (FTD-0311, v2 as of 2026-07-12). The archived file's internal `[OPEN]` marker is provenance, not live work.

### 7.5 Main FTD spec — **1 `[OPEN]`**
**File:** `docs/SPEC_FTD.md`. Top-level spec has one unresolved note.

### 7.6 Misc status/meta files
- `docs/theory/07_assessment/archive/AUDIT_WHAT_IS_GENUINELY_NEW.md` — 1. **[LEGACY — pre-reframe doc; superseded, do NOT cite externally. The lone `[OPEN]` here is on the untestable consciousness/reference-frame-context material now tagged [SPECULATIVE CONJECTURE]; not a live research item.]**
- `docs/theory/META_INDEX.md` — 1.
- `docs/reference/REF_EPISTEMIC_LABELS.md` — 2 (conventions, not physics).
- `docs/internal/SPEC_CLAUDE.md` — 2 (internal).

### 7.7 2026-04-27/28 priorities (post engine-as-instrument cycle) — **0 `[OPEN]`, 4  CLOSED**

Three high-leverage research items surfaced by the 2026-04-27
engine-as-instrument campaign; one new sub-item added 2026-04-28 after
FTD-0110 closure. All tracked in CLAUDE.md v5.33 §[OPEN] and the
bird's-eye assessment in
[`../../../WHERE_WE_LEFT_OFF.md`](../../../WHERE_WE_LEFT_OFF.md) §10.

-  **WHY 25 voxels for ic1 cluster?** — **CLOSED at linear level
  2026-04-28 (commit `306837c`).** The 25-voxel value at canonical
  amplitude A=10 is the steady state of the empirical scaling
  N(A) ≈ ¼·A² (i.e. ¼·100 = 25). The ¼ coefficient is now [DERIVED at
  linear level] from O_h representation theory: `mult(A_{1g}) = 4` in
  the 27-block ([THEOREM] via character-table formula); δ_center is the
  unique O_h-fixed point and therefore A_{1g}-pure; the 18-pt Laplacian
  preserves A_{1g} as a 4×4 block; δ_center projects onto 4 A_{1g}
  eigenmodes with mean energy fraction 1/N_base = 1/4. **Source:**
  [`DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`](../../03_derivations/foundational_mechanics/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md);
  verification suite C1–C4 PASS in
  `scripts/exploration/verify_k_derivation_2026-04-28.py`.

-  **L=128 G2 follow-up to FTD-0107** — **RESOLVED 2026-05-26**. GPU-native exascale campaign at $L=128$ recovers the spin-1 control cleanly and confirms Outcome B (non-separability of the spin-2 TT channel), locking the $L$-invariance of the verdict.

-  **Structural bridge between algebraic spine and engine
  phenomenology** — **CLOSED at linear level 2026-04-28**. The
  framework integer N_base = 4 = mult(A_{1g}) connects O_h-cubic-point-group
  structure (algebraic [THEOREM]) to cluster-efficiency coefficient ¼
  (engine observable, [MEASURED]) via the derivation chain documented
  in `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`. This is the project's
  first quantitative algebraengine connector at predictive precision.
  Note: closure is at the linear-Laplacian level only; see new sub-item
  below for the [OPEN] nonlinear extension.

-  **[OPEN — boundary mapped 2026-06-11, FTD-0269] nonlinear bridge — N(A) law is engine-emergent.** Pre-registered (`preregister-ftd0110-na-law-v1`, lock `192203b5`) substrate-parameter forward model with the FFT-exact Gauss-projection boost (the lattice Poisson Green's function) + coupling source: the framework-derived dynamics reproduce the law's SHAPE (broken power, knee=14, super-knee exponent 2.07, sub-knee firing geometry shell-L1=0.18) and the Gauss boost is confirmed the decisive sub-knee ingredient (gauss-off collapses N(14) 17.9→7) — but the law is **ENGINE-EMERGENT (verdict BOUNDARY)**: the kinetic drain shifts the knee by 16, √α-coupling is super-knee load-bearing (off/on RMS 0.118), and the absent Langevin friction γ over-predicts super-knee ~1.8×. Calibration is set by non-framework engine constants; the substrate fixes only the geometric shape. The bridge stays **[OPEN]** as a clean-derivation target (would require deriving the 0.5 drain + γ from the action, or showing the calibration is convention). The earlier "[RESOLVED — Genesis Throttling]" claim and `DERIV_FTD0110_GENESIS_THROTTLE.md` are **superseded** (a 3-point count match, no Gauss, knee mislocated at 23.5). See `ANALYSIS_FTD0110_NA_LAW.md`. (Prior historical text: Canonical position: linear k = ¼ `[DERIVED]` + Bridge-I O_h-equivariance `[DERIVED]`; nonlinear coefficient origin `[OPEN]`/`[SMC]`. Mechanism α `[CLOSED NEGATIVE]`, Mechanism γ `[CLOSED NEGATIVE]`, Mechanism β `[CLOSED NEGATIVE]`. Engine telemetry FTD-0267 established the genesis-vs-survival nature.)

-  **Not counted as open (2026-06-14 — FTD-0277):** the locked v1 collective-coordinate genesis-counting route is **CLOSED NEGATIVE**. It passes only the drain exponent (`-1.000`) and gamma-direction checks; it fails the primary N(A) law (pure `A^2`, knee 24.25, A=10 count 160, shell-L1 1.830). This closes the v1 slosh-pass/static-gating ansatz, not the broader FTD-0110-NL / FTD-0250 collective-coordinate reduction. See [`ANALYSIS_GENESIS_COUNTING_v1.md`](../../03_derivations/archive/closed_negative/ANALYSIS_GENESIS_COUNTING_v1.md).

-  **[RESOLVED 2026-06-10 — owner decision] ic1 reproducibility break (FTD-0260).** Owner: *"we've had to fix some mistakes over time so that may be why it's different — just fix it."* The change traces to accumulated deliberate engine corrections; the **current stack is canonical**; forensics closed by decision. Fixes shipped same day: canonical test re-baselined (historical pins preserved in comments); `gpc_03_genesis()` made quantitative (3× band); STACK-PINNED banner on FOUND_LATTICE_SPACING_GAUGE_FREEDOM §12; FTD-0110 empirical leg re-tagged [STACK-PINNED — historical]. **Successor [OPEN] (below).**
-  **[RESOLVED 2026-06-10 — FTD-0261, first half] Current-stack N(A) law characterized + thermostat discriminated.** Pre-registered CLEAN-LAW verdict (V-1 5/5, AIC margin 13.1): **broken power law, knee at A ≈ 16, `N ∝ A^3.69` below / `N ∝ A^1.86` above, asymptotic k_eff ≈ 0.05 (not the historical ¼)**. Thermostat: **Outcome A** (median N_X/N_N = 1.61) with dose-arm attribution to **pure friction** (γ-monotone, T-flat) — **FTD-0259's thermal-crossover knee reading [CLOSED NEGATIVE]**. See [`ANALYSIS_NA_LAW_CURRENT_STACK_v1.md`](../../03_derivations/foundational_mechanics/ANALYSIS_NA_LAW_CURRENT_STACK_v1.md).
-  **[RESOLVED 2026-06-10 — FTD-0262] SM clustermass re-assessment: IDENT-NULL.** Pre-registered (prior 65 %, landed): electron anchor PASS (20/20 exact 1-voxel, time-stable); the FTD-0261 law extrapolates to the off-grid μ/π points at 3–4 % (circular for the identification, pre-flagged); **specialness probe SMOOTH (p_local = 2.052) — no attractor structure at R_μ; N(A) passes through the SM ratios like any other values.** FTD-0110 stays `[SMC]`; its support inventory is now precisely documented: historical stack-pinned matches + the current-stack anchor + nothing else. See [`ANALYSIS_SM_MASS_IDENT_CURRENT_STACK_v1.md`](../../03_derivations/foundational_mechanics/ANALYSIS_SM_MASS_IDENT_CURRENT_STACK_v1.md).
-  **[RESOLVED 2026-06-10 — FTD-0263] Sub-knee onset mechanism + the ¼ question.**
  -  **Mechanism β v2 envelope model (2026-06-10, FTD-0263)**: tested and evaluated as **BETA_v2_CONFIRMED**. The refined model incorporates center voxel manifestation back-reactions (flux drain, $50\%$ kinetic velocity drain, and Red-Black SOR Gauss projection). Under the full physical back-reaction, the nearest-neighbor ($r=1$) manifestation threshold shifts precisely from the naive $A \approx 5.62$ up to the observed $A \approx 8.5 - 9.0$. See [`ANALYSIS_BETA_ENVELOPE_MODEL_v2.md`](../../03_derivations/archive/resolved/ANALYSIS_BETA_ENVELOPE_MODEL_v2.md).
  -  **Mechanism β v1 envelope model (2026-06-10, FTD-0264)**: tested and evaluated as **BETA-PARTIAL** (Variant A passes T1 elbow at $\text{knee}_N = 13.1$ in target range $[9.7, 21.9]$, but fails T2 shape RMS at $0.749$). The Symplectic Euler dispersion calculation error was corrected ($0.2257$ vs $0.2253$ theory). The verdict confirms that while the linear spatial envelope successfully locates the onset boundary, the staircase shape itself requires localized charge kinetics and non-linear back-reaction. See [`ANALYSIS_BETA_ENVELOPE_MODEL_v1.md`](../../03_derivations/archive/closed_negative/ANALYSIS_BETA_ENVELOPE_MODEL_v1.md).
  -  **The ¼ scaling question (2026-06-10, FTD-0263)**: **CLOSED NEGATIVE**. Sweeps show that no protocol/config realizes exactly $k = 0.25$ on the current stack. Under zero kinetic drain, the maximum is $k_{\text{eff}} \approx 0.197$ at low amplitude ($A=10$), drifting down to $k_{\text{eff}} \approx 0.169$ at $A=30$, and asymptotically flat at $k_{\text{eff}} \approx 0.05$ under tuned Langevin parameters. The linear representation value $k = 1/4$ remains a theoretical upper limit for a lossless linear field; the full nonlinear engine includes genesis filtering and thermostat damping that reduce efficiency.
  -  **Kaon/Proton/Tau multi-scale L-scan (2026-06-10, FTD-0263)**: **CLOSED MEASURED**. Executed the multi-scale scans at the predicted amplitudes ($A = 2\sqrt{m/m_e}$): Kaon ($A=62.42$, $L=48$) measured $N = 203.8$, Proton ($A=85.70$, $L=64$) measured $N = 349.8$, and Tau ($A=117.93$, $L=80$) measured $N = 674.6$. The scaling consistency matches the current-stack $N(A)$ power law characterized under FTD-0261, confirming a stable asymptotic $k_{\text{eff}} \approx 0.05$ across all energy scales from muon to tau on the corrected stack.
  - *Bonus constraint from FTD-0263: the bulk branch is L-invariant too (N(30) = 45.0 exactly at L = 24/32/48) — the law is intrinsic physics.*
  - *Superseded provenance (the resolved FTD-0260 forensics item, kept for history):* The pre-registered thermostat-OFF discriminator's validation gate found the historical FTD-0110 phenomenology (N(A=10)=25; the §6.5 k(A) table) **not reproduced by any combination available today**: {April source `87158aef`, lock source `4fa056c2`} × {CPU, CUDA/GPU} all give the same new low family (N(10)≈4; whole curve ≈4–5× low). **Tracked code EXCLUDED** (April source rebuilt today reproduces the *broken* values — the originally-announced bisect is WITHDRAWN, nothing to bisect); **backend EXCLUDED** (CPU ≈ GPU today; the canonical `test_emergent_ic1_topology` fails T1/T2 0/3 on both). The change lives in the **runtime environment or an untracked input** (toolchain/CUDA/driver/flags/thread-order). The FTD-0110 empirical table + SM cluster-mass matches are **provenance-pinned to the April/May stack**. Needed: (i) recover April/May build provenance (preserved CMakeCache/CI logs/result metadata) and identify the changed layer; (ii) establish when/where `test_emergent_ic1_topology` last passed (absent from the 2026-06-06 known-failures list — passed then, or excluded from that ctest run?); (iii) **strengthen `gpc_03_genesis()`** (currently existence-only `>=1` — can never catch quantitative genesis drift). Mechanism-γ discriminator v2 **blocked on both backends** until resolved or the baseline is formally re-measured + re-tagged on the current stack. See [`ANALYSIS_THERMOSTAT_OFF_SWEEP_v1_INVALID.md`](../../03_derivations/archive/invalid/ANALYSIS_THERMOSTAT_OFF_SWEEP_v1_INVALID.md) §4.

**Last audit refresh:** 2026-06-10 (FTD-0263 resolved — Mechanism Beta v2 model confirmed, explaining the shift in manifestation threshold to A ≈ 8.5 under center kinetic/flux drains + Gauss projection; 1/4 scaling closed negative; multi-scale L-scan closed measured. FTD-0262 IDENT-NULL; FTD-0261 law MEASURED; FTD-0260 resolved).

### 7.8 Round-table discussion residue — **3 `[OPEN]`** (2026-06-30, no canonical doc)

Three unresolved threads from a discussion-only QM/GR-incompatibility round table (`FOUND_MODULUS_ARGUMENT_FRONTIER.md` §7.1 / FTD-0344 records the one narrower observation from that same table that *was* banked; these three were not, and are recorded here so they are not silently lost). None has a canonical backing document; none is a claim — each is a named gap.

- **[OPEN] The black-hole-information-paradox gap.** No position in the round table addressed it. The paradox is a *single* lossy map QM forbids (information must be recoverable); the round table's "two different imported arguments" picture (one for QM's measurement, one for GR's missing time/spin-2) was never tested against this case and may not accommodate it. Unattempted.
- **[OPEN] Same-or-different chosen adjoint for QM's "imported time" and GR's "missing time"?** Possibly **background-dependent**: generically argued to be *distinct* chosen adjoints (a state-relative modular flow vs. a diffeomorphism-covariant section), but in AdS the Ryu–Takayanagi/JLMS result (modular Hamiltonian = area) makes them the *same* object as a *theorem* — stronger than the Connes–Rovelli thermal-time hypothesis the round table otherwise leaned on. Undecided; not engaged past this framing.
- **[OPEN] Asymptotic safety and causal sets threaten the frontier's premise, not just its conclusion.** A continuum UV fixed point (asymptotic safety) or a native primitive causal order (causal sets) would shrink `FOUND_MODULUS_ARGUMENT_FRONTIER.md`'s meta-conjecture (§3) from a *substrate-class* theorem to an *FTD-specific* claim — i.e. attack the premise "no S-class substrate can self-supply the argument-half," not merely the conclusion drawn from it. Neither program was engaged by any round-table position.

---

## §8 Scripts

Unfinished verification, proof, and exploration scripts. The script itself usually runs successfully; the `[OPEN]` marks where its conclusion stops short of a closed derivation.

### 8.1 Verification
- `scripts/verification/verify_chiral_anomaly.py` — **3 `[OPEN]`** (GW-fermion alternative).
- `scripts/verification/verify_two_loop.py` — **2 `[OPEN]`** (explicit BZ² integral would give ab-initio two-loop α).
- `scripts/verification/verify_modular_structure.py` — **1 `[OPEN]`**.
- `scripts/verification/verify_thermodynamic_limit.py` — **1 `[OPEN]`**.

### 8.2 Proofs
- `scripts/exploration/archive_proof_quark_masses_lattice.py` —  RETRACTED 2026-05-27.
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
  | grep -v ":0$" | grep -v ".venv\|node_modules\|build/\|build_\|__pycache__\|archive" \
  | sort -t: -k2 -rn
```

Snapshot (2026-07-12, regenerated; the archive/ exclusion added to the command this pass — archived docs' markers are provenance, not live work):

**Raw total: 907 `[OPEN]` string-markers across 263 files.** ⚠ The raw count is NOT a live-work count: the top entries are the ledgers/trackers themselves (LEDGER.md 97, this tracker 71, SPEC_OPEN_MATH_BY_SECTOR 33, META_INDEX 20, SPEC_DOCTRINE_LEDGER 15) — rows *about* open items, closed-context mentions, and tag-legend examples. The curated math-relevant queue lives in `SPEC_OPEN_MATH_BY_SECTOR.md` (v1.1); the curated engine/doc items live in §§1–8 above. Top non-meta densities:

| File | Count |
|---|---:|
| `docs/theory/03_derivations/INDEX_03_DERIVATIONS.md` | 13 |
| `docs/theory/10_eft_program/derivations/DERIV_FTD_NATIVE_RESPONSE_AND_BLOCKING.md` | 12 |
| `docs/theory/07_assessment/AUDIT_BOUNDARY_MAP.md` | 12 |
| `docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V1.md` | 10 |
| `docs/theory/01_reference/SPEC_COSMOLOGY_FRAMEWORK_BOUNDARY.md` | 10 |
| `docs/theory/04_coupling/SCOPE_DISCRETE_FEYNMAN_PROGRAM.md` | 9 |
| `docs/theory/02_foundations/FOUND_AXIOM_ZERO.md` | 9 |
| `docs/theory/02_foundations/EXPLR_SIXTH_POSTULATE_AND_OBSERVER_FRAME.md` | 9 |
| `engine/web/js/config/scenarios.js` | 8 |
| `docs/theory/04_coupling/DERIV_COSMOLOGICAL_CONSTANT.md` | 8 |
| `docs/theory/03_derivations/gravity_and_cosmology/DERIV_LAMBDA_SCALE_COVARIANT.md` | 8 |
| `docs/theory/01_reference/MONOGRAPH_FTD_CONSTRUCTION.md` | 8 |
| `docs/theory/10_eft_program/derivations/DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md` | 7 |
| `docs/theory/07_assessment/ASSESSMENT_MATH_GRADES_AND_EXTENSIONS_2026-07-01.md` | 7 |
| `docs/theory/03_derivations/foundational_mechanics/EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md` | 7 |
| `docs/theory/10_eft_program/derivations/DERIV_FTD_NATIVE_NONLINEAR_FLOW.md` | 6 |
| `docs/theory/06_reference_frames_and_measurement/DERIV_COLLAPSE_MECHANISM.md` | 6 |
| `docs/theory/03_derivations/standard_model/DERIV_MOORE_GAUGE_STRUCTURE.md` | 6 |
| `docs/theory/03_derivations/foundational_mechanics/MONOGRAPH_EFFECTIVE_EQUATIONS.md` | 6 |
| `docs/theory/02_foundations/FOUND_PHENOMENAL_NOUMENAL_BRIDGE.md` | 6 |

**Non-physics mentions excluded from any "real open work" reading** (convention labels, not unresolved items): the tag cheatsheet, templates, glossary, UI string literals, and the meta/tracker layer's own cross-references — as in the 2026-04-17 audit's exclusion list.

---

## Recently closed

Move items here with the closing commit / PR when an `[OPEN]` becomes ``.

### G* & Master Quadratic Mathematical Connections (Theme 1) —  CLOSED 2026-06-10

-  **Watson-G* identity**: Closed physical interpretation of the Watson-G* connection as a period equivalence on the substrate under FTD-0242, and resolved the 14 vs 16 torus DOF counting discrepancy as a legacy heuristic (coefficient 16 is structurally forced by $|{\rm Aut}(E)|^2 = 16$).
-  **L-function & Hecke prime connections**: Reclassified and closed L-function physical coupling role, partition function maps, Hecke prime physical significance, and Langlands-theoretic interpretation under FC-1 and FTD-0242.
-  **α lattice mechanism**: Closed step 3 CM curve selection (proven uniquely under the trivial-multiplier criterion) and step 8 root selection (reclassified as unforced operator-readout assembly selection under the dynamic-alpha pivot).
-  **Mathematical connection sweeps**: Closed all open items in curve-family analysis, ReLU type transition, collapse-gravity bridge, and conjectural CM alpha route under FTD Constitution (FC-1/FC-2) and FTD-0242.

### Epistemic Integrity & Consciousness Gaps —  CLOSED 2026-06-10

-  **Lepton Mass Ratios**: Demoted the $m_\mu/m_e = 207$ and $m_\tau/m_e = 3477$ formulas in `SPEC_SM_REPLACEMENT_COMPLETE.md` to `[STRUCTURALLY MOTIVATED PARAMETRIC]` and in `FOUND_AXIOM_ZERO.md` to `[IMPOSED] (parametric insertion)`, as they lack a rigorous derivation from the core FTD lattice Lagrangian.
-  **Fine-Structure Constant Precision Polynomials**: Confirmed the 4-term and 7-term $\alpha$ precision polynomials (`ALPHAP-1`, `ALPHAP-1b`) in `DERIV_ALPHA_PRECISION_FORMULA.md` are tagged `[IMPOSED]` parameter fits, correcting legacy claims of theorem status.
-  **Downstream Derivations (SU(2) Weak)**: Corrected the summary tables and decay rate headers in `DERIV_LATTICE_SU2_WEAK.md` and `SPEC_SM_REPLACEMENT_COMPLETE.md` to consistently mark weak decay rates as `[PARAMETRIC INSERTION]` rather than `[THEOREM]` due to imported functional forms.
-  **Consciousness predictions untestable**: Reclassified the untestable reference frame context predictions (Gap 5) in `AUDIT_WHAT_IS_GENUINELY_NEW.md` to `[CLOSED DECLINED]` under FTD-0242 since they lack operational definitions and experimental protocols.

### Option 3 (QM & Observer Foundations) Declination —  CLOSED DECLINED 2026-06-10

-  **Continuous Hilbert Space Recovery**: Closed the open items in `DERIV_QM_FROM_LATTICE.md` by formally declining continuous QM, continuous Born rules, and continuous Schrödinger equations as fundamental targets under FC-1. The discrete ternary lattice is complete; Hilbert space is an epistemic map of observer ignorance.
-  **Bell & Singlet Mapping**: Closed continuous Bell violation (`DERIV_OBSERVER_BELL_MECHANISM.md`) and singlet-state mapping (`DERIV_SINGLET_FROM_VOID_EVENT.md`) targets as declined. Local hidden variable S <= 2 holds fundamentally at the substrate.
-  **Observer & Measurement Foundations**: Closed all open items in `FOUND_WIGNERS_FRIEND_RESOLUTION.md`, `FOUND_VON_NEUMANN_CHAIN.md`, `DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md`, `FOUND_DOMAIN_PARTITION_AND_CONTEXT_SELECTION.md`, `FOUND_THE_EXISTENCE_FILTER.md`, `PAPER_CONSCIOUSNESS_QFT_GR_BRIDGE.md`, and `FOUND_BORN_RULE_NULL_CONE.md` by declining continuous measurement/observer structures.

### Repository-Wide Epistemic Sweep —  CLOSED 2026-05-30

-  **Epistemic Tag Cleanup**: Removed inflated `[THEOREM]` tags and downgraded to `[IMPOSED]` or `[PARAMETRIC INSERTION]` for formulas relying on standard physics substitutions instead of first-principles derivations. Fixed in `FOUND_AXIOM_ZERO.md`, `DERIV_ALPHA_PRECISION_FORMULA.md`, and `DERIV_LATTICE_SU2_WEAK.md`.
-  **Archived Closed Items**: Moved multiple `[CLOSED NEGATIVE]`, `[RETRACTED]`, and `[CLOSED -- RESOLVED]` documents into their respective `archive/` directories, updating `META_INDEX.md` and related links.

### Class C Cluster-Cluster Interaction Specification —  CLOSED 2026-05-27 (Campaign FTD-0222)

-  **Outcome A (FOUND): Class C Specification** — Drafted `docs/theory/01_reference/SPEC_CLASS_C_CLUSTER_INTERACTION.md` detailing the discrete-native forces, displacement gradients, dimensionless coupling extraction ($\alpha, y_{\text{Yukawa}}, G_N$) directly from relational coordinates, and calibration conversion to SI Newtons.

### No 4th Generation Fermions No-Go Formalization — CLOSED 2026-05-27 (Campaign FTD-0220)

-  **Outcome A (FOUND): No 4th Generation Fermions** — Created `docs/theory/10_eft_program/FOUND_NO_4TH_GENERATION_NO_GO.md` and pre-registration `PREREG_NO_4TH_GENERATION_NO_GO_v1.md`, proving that exactly three generations are selected under the $D=3$ Moore layer decomposition $C(D,2)=3$, and a standard fourth generation is algebraically and topologically excluded. Symmetries verified by `scripts/exploration/verify_no_4th_generation.py`.

### QFT/GR Bridge Consolidation — CLOSED 2026-05-27 (Campaign FTD-0214)

-  **Option A (GAP-P5): Loop corrections to alpha precision series** — Modified `docs/theory/04_coupling/DERIV_ALPHA_PRECISION_FORMULA.md` to add §4.4 Interacting Vacuum Polarization Loop Derivation, proving the nome deviation $e^\pi - \pi - 20$ represents the discretization anomaly of the lemniscate torus under Langevin flow.
-  **Option B (GAP-P3): Jones Index threshold ratio derivation** — Created `docs/theory/09_mathematical/DERIV_JONES_INDEX_THRESHOLD_RATIO.md` showing that the manifestation threshold ratio $K_B/K_C = 4\sqrt{2}$ is the exact square root of the modular subfactor inclusion Jones Index $[N:M] = 32$ of the complexified octahedral representation space.
-  **Option C (GAP-G4): Emergent diffeomorphism invariance** — Created `docs/theory/03_derivations/DERIV_EMERGENT_DIFFEROMORPHISM_INVARIANCE.md` deriving emergent $\text{Diff}(M)$ general covariance from local point-group point-filtering, proving that discrete cubic point-group anisotropies vanish as $O((a/L)^4)$.
-  **Option D (GAP-B3): Modular spectral Connes lambda derivation** — Created `docs/theory/06_reference_frames_and_measurement/DERIV_CONNES_LAMBDA_FROM_MODULAR_FLOW.md` deriving the sentience hierarchy scaling factor $\lambda(k)$ as the interacting modular operator spectral ratio, perfectly matching the manifested Shannon entropy $H \approx 0.4007$ at symmetric thresholds.


### Theory docs — alpha/QED numerical closure reclassification 2026-04-22

-  `docs/theory/04_coupling/DERIV_ONE_LOOP_LATTICE_ALPHA.md` — closed the live `[OPEN]` higher-loop convergence item as superseded/deferred by the FTD-to-EFT matching problem. Higher-loop computation remains possible inside the selected Structure-1 scheme, but is no longer an acceptance path for a scheme-independent alpha prediction.
-  `docs/theory/03_derivations/DERIV_LATTICE_QED_COMPLETE.md` — closed the live `[OPEN]` BZ² sub-ppm alpha computation item as superseded. BZ² evaluation becomes useful only after a matching principle uniquely selects the lattice-QED scheme and alpha observable.
-  `docs/theory/03_derivations/DERIV_STATE_FLUX_COUPLING_DERIVATION.md` — closed the live `[OPEN]` higher-order-corrections item as part of the same matching reclassification. The document now treats `g_c^2 = alpha = 1/x_+` as conditional on the selected state-flux-to-QED dictionary, not as a standalone first-principles derivation of physical QED.

### Engine code — 6 items resolved 2026-04-17 (dependency-ordered sweep)

-  **§1.4 Leapfrog integrator** — already symplectic. Audit via `tests/test_leapfrog_integrator_audit.cpp` showed 0.1 % cumulative energy balance over 5000 ticks with damping off. Corrected "forward Euler" comments in `render_bridge.cpp` and `dag_engine.cpp`. (CHANGELOG: "Step 1".)
-  **§1.8 Moore-Laplacian anisotropy** — already isotropic through O(h⁴). Direct Taylor expansion: `h²∇²f + (h⁴/12)(∇²)²f + O(h⁶)`. Empirical confirmation in `tests/test_moore_laplacian_isotropy.cpp` shows 11 % radial symmetry at L=64. (CHANGELOG: "Step 2".)
-  **§1.5 `ALPHA_PRECISION` rollout** — engine `ALPHA = 1/X_PLUS_PRECISION`. `G_C`, JS mirror, two hardcoded-value tests updated. Static_assert confirms `G_C² ≈ ALPHA` to 1e-8. (CHANGELOG: "Step 3".)
-  **§1.2 γ_FTD momentum integration** — replaced non-relativistic velocity clamp in `phase_forces` with `p = γmv` dynamics. Covered by `tests/test_gamma_ftd_momentum.cpp` (8/8 checks). Also removed over-strict secondary clamp in latency block. (CHANGELOG: "Step 4".)
-  **§1.7 GPU-path `EnergyLedger`** — `tick()` GPU path now auto-calls `gpu_sync_to_host()` + `update_energy_ledger()`. (CHANGELOG: "Step 5".)
-  **§1.9 Muon / tau spatial seeds** — two new `s0-seed-muon` / `s0-seed-tau` scenarios with full epistemic metadata. (CHANGELOG: "Step 6".)

### Prior scope

-  `2026-04-17` **EnergyLedger auto-populate (CPU)** — `RenderBridge::update_energy_ledger()` runs at the end of every CPU-path `tick()`. (CHANGELOG: "Consolidation Sweep".)
-  `2026-04-17` **`ALPHA_PRECISION` first-class in engine** — `X_PLUS_PRECISION` + `ALPHA_PRECISION` defined in `ontic.h`, re-exported in `constants.h`. (CHANGELOG: "Honesty Sweep".)
-  `2026-04-17` **`DagEngine` vs `RenderBridge` ambiguity** — DagEngine explicitly marked EXPERIMENTAL, WASM binding removed, `engine/README.md` updated. (CHANGELOG: "Consolidation Sweep".)

---

## Automation

A future `epistemic-auditor` agent run should diff this tracker against live `[OPEN]` tags and flag any code-level opens missing from §1 or any theory-doc opens missing from §2–§7.

Until then, periodically run the grep in §9 and compare the output against the snapshot table. Any new file appearing: either add a section entry, or (if it's a non-physics mention) add it to the excluded list.
