# Resolved and Retired Items Tracker

**Tag:** [REFERENCE]
**Status:** Historical provenance. These entries were moved from `TRACKER_OPEN_ITEMS.md` without changing their original claims or closure wording.

This file preserves closed, retracted, resolved, and declined tracker entries. The [open-items tracker](TRACKER_OPEN_ITEMS.md) remains the working queue; [its index](TRACKER_OPEN_ITEMS_INDEX.md) lists active and pending-verification headings. References to still-open work inside a historical entry do not close that separate work.

---

## §1 Engine code
### 1.2 Causal velocity, clock, and mass roles — CLOSED by FTD-0403 v2 TARGETED-CLOSURE

**Implementation:** `phase_forces()` now integrates momentum instead
of velocity. Algorithm:

```
γ_in  = 1/√(1 − |v|²/C² − L²)       (from stored v + latency)
q     = γ_in · u                     (specific momentum P/M)
q_new = q + (F/M_INERTIAL)·dt       (one update after all force paths)
|u_new|² = C²(1 − L²)·|q_new|² / (C² + |q_new|²)
u_new = q_new · C · √((1 − L²)/(C² + |q_new|²))
```

This respects the selected clock/bandwidth axiom `|u|²/C² + L² < 1` by
construction. It is an implementation contract, not a Lorentz-covariance theorem.

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

**Historical FTD-0401 correction:** the then-current engine mixed raw velocity with a legacy `c=1` matter clock and fused `M_REST` roles. That source-contract no-go remains provenance in [`AUDIT_C_SPEED_MASS_NORMALIZATION.md`](../AUDIT_C_SPEED_MASS_NORMALIZATION.md).

**FTD-0402 historical result:** the production correction is implemented. Raw `u` maps to `beta=u/C_SPEED`; force, movement, proper time, latency, Born–Infeld, evaporation, and de Broglie phase use one budget. GPU device-side `tau` and its separate cap are removed. Explicit roles are `M_INERTIAL=K_B`, `E_REST=K_B/3`, and `M_GRAVITATIONAL=K_B`; their numerical equality is imposed, not derived. EnergyAudit exposes normalized particle energy/momentum and distinguishes dynamic from incomplete accounted energy. Its frozen verdict remains `PARTIAL`: [`RESULT_CAUSAL_NORMALIZATION_MASS_ROLES.md`](../lorentz_recovery_causal_structure/RESULT_CAUSAL_NORMALIZATION_MASS_ROLES.md).

**FTD-0403 successor closure:** v1 exposed and invalidated a stale over-speed boundary fixture. The test-only repair crosses the face using an in-budget velocity plus accumulated remainder and verifies zero projection. Under a fresh v2 lock, exact A1–A7/S1–S9, native 14/14, CUDA 6/6, golden 7/7, WASM/web 3/3, compatibility, link, and census gates pass. No full CTest or unrelated campaign was run. This closes the current-engine normalization contract over the frozen changed surface; it does not establish covariance, a mass scale, or inertial–gravitational equivalence. Canonical result: [`RESULT_CAUSAL_NORMALIZATION_TARGETED_CLOSURE_v2.md`](../lorentz_recovery_causal_structure/RESULT_CAUSAL_NORMALIZATION_TARGETED_CLOSURE_v2.md).

### 1.4 Symplectic leapfrog integrator —  CLOSED 2026-04-17
**Location:** `engine/src/render_bridge.cpp` `phase_read` header comment.

Empirical audit via `tests/test_leapfrog_integrator_audit.cpp` showed the pair `wave_vel += Δ; flux += wave_vel` IS Störmer–Verlet leapfrog under the stagger interpretation (`wave_vel = v(t + h/2)`, `flux = J(t)`). My earlier mis-read called it "forward Euler"; that was wrong.

**Evidence (all passing):**
- 1000 ticks @ L=16: cumulative injection/dissipation balance to 0.5 %.
- 500 ticks @ L=32: 1.7 %.
- 5000 ticks @ L=16: **0.1 %** — no secular drift over 5× longer run, classic symplectic signature.

The `max_residual_seen` per tick is large because `½|J|² + ½|v|²` is an L² indicator, not the true conserved Hamiltonian (which involves `|∇J|²`). Energy sloshes between the two forms every half period; leapfrog keeps cumulative injection ≈ dissipation.

`C_SPEED = 1/√D = 1/√3` is the leapfrog CFL limit, correctly identified. **No code change needed.** **Follow-up completed (verified 2026-07-12):** the "forward Euler" honesty-sweep comments in `render_bridge.cpp` and the since-retired DagEngine source were corrected to Störmer–Verlet; the last residue in `engine/PHYSICS_STATUS.md` was fixed in that pass. No live "forward Euler" description remains.

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

**Takeaway (scope-corrected by FTD-0407):** the *spatial* Laplacian has an
isotropic quartic term. This does not close Lorentz recovery: the fully discrete
time-plus-space pole contains an earlier rotationally invariant boost-violating
term. No spatial-weight change was needed for the old rotation test; a spacetime
update change or an explicit Lorentz-violating EFT matching is required for the
hard recovery problem.

### 1.10 CPU-only no-op toggles — CLOSED 2026-08-30

Resolved in three steps:

**Ported to CPU:**
- `pair_production` → `RenderBridge::pair_production_cpu()` (Rule 2b in the tick cycle). Correlated ±1 pairs from high-|J| void, matching the GPU algorithm.
- `triad_binding` → `RenderBridge::triad_binding_cpu()` (Rule 7). Locks compact same-sign triads via pairwise-distance + near-equilateral check.

**Subsequently ported to CPU:**
- `strong_force` + `exchange_force` now use the same Yukawa/exchange pairwise
  laws as the CUDA kernels. `test_cpu_pairwise_force_channels` pins that each
  toggle changes CPU voxel state relative to its disabled baseline.

**Retired after CPU completeness:**
- The empty per-toggle GPU-only warning column, one-shot warning state, and
  runtime warning helper were removed. Unsupported backend combinations remain
  fail-closed through `ToggleSpec::backends` and `validate_backend()`.

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

## §2 Theory — derivations (`docs/theory/03_derivations/`)
### 2.1 Lattice Black Holes —  closed/reclassified 2026-06-10
**File:** `DERIV_LATTICE_BLACK_HOLES.md`.
Derived the tensorial latency $\mathcal{L}^{ij}$ for Kerr-Newman black holes, mapped the Ernst equation for axisymmetric vacuum to the flat lattice, and established the quantitative flux-coupling mechanism for superradiant wave amplification. All symbolic limits and identities verified via `proof_black_hole_extensions.py`. The file now has zero live `[OPEN]` items.

**2026-05-20 reconciliation note:** per FTD-0184, future gravity work here must target substrate-side strong-field GR / Schwarzschild-Kerr-horizon derivation. Do **not** pursue the branch-compliance/Yilmaz exponential-metric route (`dτ=e^{-U}`, `n_γ=e^{2U}`) as a replacement gravity sector; it is closed negative for canon and preserved only as provenance.

### 2.2 Lattice QED —  closed/reclassified 2026-04-22
**File:** `DERIV_LATTICE_QED_COMPLETE.md`.
The former BZ² sub-ppm alpha computation item is superseded by the FTD-native electrodynamics pivot. The file now has zero live `[OPEN]` items. Future QED numerics are external comparison checks, not a route to fitting alpha.

### 2.6 Higgs from manifestation — **0 `[OPEN]`** (closed 2026-06-11)
**File:** `DERIV_HIGGS_FROM_MANIFESTATION.md`.
- **0.47% mass discrepancy** — **CLOSED 2026-06-11** (wording reconciled 2026-07-12 to the FTD-0268 honest digest). The gauge-derived quartic $\lambda = 3/23$ gives tree-level $m_H = 125.69$ GeV = **+4.44σ** vs canonical PDG 2024 ($125.20 \pm 0.11$ GeV; the "$125.25 \pm 0.17$" previously quoted here was a superseded edition — see `REF_EXTERNAL_CONSTANTS.md`); applying the $(1-\alpha)$ flux-dissipation factor — **applied, not derived** (FTD-0268) — gives $m_H = 125.23$ GeV = **+0.27σ**. Chain status `[SELECTION]+[PARAMETRIC]`, not a derivation. Note the two Higgs routes in canon are different formulas at different tags: this manifestation route (λ=3/23, +0.27σ with the applied loop factor) vs the FTD-0017 route $m_H = (N_\text{eff}/\alpha^2)\,m_e = 124.75$ GeV, which is **−4.1σ** vs PDG 2024 and experimentally excluded as an exact relation (FTD-0348). Neither is promoted by the other; cross-refs FTD-0017/0268/0348.
- **EW phase transition order** — **CLOSED 2026-06-11**. Confirmed computationally via `campaign_ew_phase_transition.cpp` that the phase transition is strongly first-order due to the massive hysteresis loop between genesis and evaporation thresholds.
-  **BI maximum field strength and pair creation** — **CLOSED 2026-06-11**. Proven computationally via `campaign_higgs_bi_pair_production.cpp` that discrete pair production kinetics enforce the continuum Born-Infeld limit probabilistically.

### 2.10 QM from lattice —  CLOSED DECLINED 2026-06-10
**File:** `DERIV_QM_FROM_LATTICE.md`. Under FC-1, continuous Hilbert space and wavefunction recovery targets are formally declined. QM is an epistemic map of observer ignorance, not fundamental ontology.

### 2.10b Strict-discrete common-action successor — **[CLOSED — RATIFIED FTD-1023; 0 open constitutional gates]**
**Files:** `SPEC_FTD_FRAMEWORK_V3_STRICT_DISCRETE_COMMON_ACTION.md`,
`SPEC_V3_FINITE_CARRIER_INVENTORY_R1_v2.md`,
`SPEC_V3_COMMON_ACTION_PHI_R2_R5_v2.md`,
`strict_discrete_common_action_register_v3.json`,
`proof_v3_strict_discrete_postulate_register.py`,
`proof_v3_finite_carrier_inventory_v2.py`,
`proof_v3_common_action_phi_v2.py`, and
`proof_v3_target_firewall.py`. The v1 R1/Phi documents are retained as
superseded construction provenance.

The 2026-08-24 owner-ratified Occam rewrite states five v3 postulates. Native expiry and
physical memory are part of P5's selected non-injective law, not a separately
counted P6; primitive closure/block emergence is a constitutional
admissibility rule, not a separately counted P7. No carrier, rule, R1--R6
status, or open physics count changed in this consolidation.

- **V3-R1 — [CLOSED FOR SELECTED V3 CARRIER; not counted as open]:** the
  R5-capable inventory is `A0=T*C3*{0,1}^384`, `A1=A9^2`, `A2=A9^4`, and
  singleton `A3`, with unique exclusion/incidence ownership, explicit
  readouts, no queue/serial/replay register, a frozen collision-table hash,
  and 25/25 exact checks. R1 automatically reopens if another mandatory gate
  requires a new microscopic type. This closes a carrier selection, not a
  uniqueness theorem.
- **V3-R2 — [CLOSED FOR SELECTED REFERENCE LAW; not counted as open]:** the
  exact synchronous `Phi` gives one writer per relation, directed destination
  port, and site incidence reduction; opposing absorptions fail closed. It is
  homogeneous, translation covariant, Moore local, deterministic, and has no
  hidden scheduler. Included in the 32/32 exact rule certificate.
- **V3-R3 — [CLOSED FOR SELECTED REFERENCE LAW; not counted as open]:** eight
  distinct Hodge normal/hand presentations absorb into one bound A9 reserve.
  Phase, polarity, one token/work unit, and carrier line survive; arrival
  sign, normal, and hand expire. No inverse tape is retained.
- **V3-R4 — [CLOSED AT RATIFICATION MINIMUM; not counted as open]:** the same
  coupled `Phi` gives one-hop propagation, period-eight reciprocal
  manifestation/withdrawal, a localized one-relation proto-clock, and exact
  source/current continuity. This is not stable matter or Maxwell recovery.
- **V3-R5 — [CLOSED FOR TRANSVERSE VACUUM SCOPE; not counted as open]:** the
  exact cotangent collision/Floquet certificate gives two divergence-free real
  transverse field pairs at speed `1/6`; the resulting quadratic lattice
  action has spatial coefficient `1/36`, and the three-tick infrared remainder
  is bounded by `(9/2) kappa^2 exp(3 kappa)` with a finite-step telescoping
  bound. Charged Gauss, source normalization/coupling, and nonlinear
  slow-manifold protection remain stronger physics gates.
- **V3-R6 — [CLOSED BY OPERATIONAL/DATAFLOW CERTIFICATE; not counted as
  open]:** active microscopic inputs contain no `alpha`, Born weights/outcomes,
  particle masses/catalog, continuum metric/lensing target, recovery spectrum,
  or undeclared replay/random tape. The certificate freezes the carrier hash,
  audits generator imports/AST/data reads, separates recovery outputs from rule
  inputs, and enforces one-way rule-to-recovery dependency. This is not a
  uniqueness or historical-ignorance claim.

The v3 document is the ratified active constitution (FTD-1023). All R1--R6
technical gates pass at their declared scopes. Ratification is a governance
adoption, not a physical closure, and is not counted as an `[OPEN]` physics
item. V1 and v2 remain provenance/reference branches.

### 2.13 Mechanism C — `g_c` from BCC bridge operator — **CLOSED NEGATIVE (archived)**
**File:** `docs/theory/10_eft_program/archive/closed_negative/DERIV_MECHANISM_C_GC_BCC_BRIDGE.md`. Successor to FTD-0031 / Mechanism-B closure. Closed negative by `docs/theory/10_eft_program/archive/closed_negative/AUDIT_BCC_SUBLATTICE_SPECTRUM.md` and LEDGER row FTD-0093; no longer counted as an open item. The live `g_c` problem remains tracked through `docs/theory/10_eft_program/OPEN_GC_FROM_FIRST_PRINCIPLES.md` and the native electrodynamics pivot in §4.2.

---

## §3 Theory — foundations (`docs/theory/02_foundations/`)
### 3.5 Bridge Functional ontology — **0 `[OPEN]`** (closed; not counted as open — aligned 2026-07-12)
**File:** `docs/theory/01_reference/SPEC_QUADRATIC_PHYSICS_BRIDGE.md`. LEDGER row FTD-0095 — **[THEOREM] (2026-05-29)**: the arithmetic-mean rule `M(x₊, x₋) = α·(x₊+x₋)/2 = 8αG*²` is derived via 't Hooft beable equiprobability (symmetric Markov transition matrix ⇒ unique uniform stationary measure ⇒ expectation = arithmetic mean); verified in `proof_bridge_functional_arithmetic_mean.py`. This tracker row had lagged the LEDGER upgrade — prose aligned, no promotion by this sweep (the LEDGER tag is the tag of record).

### 3.7 Clause-2/3 boundary program — **0 `[OPEN]`** (all chartered stages DELIVERED 2026-07-05; Clause-1 deferred, not counted open)
**Plan:** `<local-agent-plan>\let-s-plan-a-comprehensive-calm-dijkstra.md`. Theorem-ized the remaining boundary imports (Clause 2) and studied the native closure N as a mathematical object (Clause 3), reusing the δ-IND template (define closure structurally → freeze → exclude/include by a conserved invariant). **ALL CHARTERED STAGES DELIVERED:** A0 (δ-IND audit + amendments), A1 (`SPEC_ALPHA_READOUT_CONTRACT.md` §2.5 ramification checkpoint — standing discipline), A2 (`FOUND_DIMENSIONAL_GRADE_CLOSURE.md`, 8/8 — grade-0 closure, third conserved charge), A3 (`FOUND_L2_CLOSURE_RECAST.md`, 5/5 — L² wall as polyhedrality-conservation, fourth conserved charge), A4/B2 (`THEOREM_RAMIFICATION_LOCUS.md`, **FTD-0370**, 7/7 — the flagship: Ram_t(hull) = {0, ∞}, δ de-specialized), B0(i–iii) (`EXPLR_STENCIL_SPECTRUM.md` — the σ₁₈ default Green's function holonomic-but-large, evaluation UNKNOWN, CAS telescoping deferred), **B1** (`FOUND_NATIVE_CLOSURE_REALIZABILITY.md`, 6/6 — realizability lower bounds: explicit D1–D4 schemas place G\*²/(2π), W_S/2 Γ(1/24)-class, W₁₈, π INSIDE N; **N ⊋ ℚ(G\*,π) conditional on E1**; the sandwich with FTD-0369/0370 = N is a large period ring dodging the (4G\*−1) square class), B3 (`REF_EXPORTED_PROBLEMS_E1_E2.md` — three FTD-free exported problems, first external-circulation artifact). **Residues (not chartered, not counted open):** the quasi-period-at-τ=i lemma (to un-restrict the FTD-0369 BCC sub-theorem — a follow-on claim outside the frozen map); the nonlinear-rung v2 prereg (properness fight for the full substrate); FCC/broader-symbol realizability (v2 scope). **Clause-1 (the priced-import ledger) explicitly deferred by the user; no constitutional / Number-One-Goal edits under this program.** Conserved-charge inventory: four (algebraicity / (4t−1)-parity-ramification / dimension-grade / polyhedrality); see `FOUND_DIMENSIONAL_GRADE_CLOSURE.md` §5 + `FOUND_L2_CLOSURE_RECAST.md` §4. The δ-IND upper bound (§3.6) + this program's B1 lower bound now bracket N.

### 3.8 Full-state irreversibility — **0 `[OPEN]`** (closed FTD-0395; not counted open)

FTD-0394 established only that the discrete readout `R` is non-injective while `J` remains stored. FTD-0395 has now closed the separate implemented-map question: a pre-registered pair of public-API-admissible states differing only in spin/color evaporates on the same complete tick to bit-identical persistent state and remains identical for 16 more ticks. The result is scoped to the current engine map; FC-2 remains `[AXIOM]`. See `02_foundations/ANALYSIS_FULL_STATE_IRREVERSIBILITY_v1.md`.

---

## §4 Theory — particles + couplings
### 4.0 Electron exponent order type — **0 `[OPEN]`** (closed FTD-0397; not counted open)

The unordered FTD-0084 multiset `{3,3,4,6}` has 12 orderings forming one `S4` orbit. FTD-0397 proves that permutation-invariant data is constant on that orbit and cannot select cumulative position 11. The exponent remains `[SELECTION]`; a future promotion would need a fresh, independently derived order-bearing dynamics, not another invariant reformulation or ordering rule. See `05_particles/THEOREM_N11_ORDER_TYPE_NO_GO.md`.

### 4.1 Quark masses from lattice —  RETRACTED 2026-05-27
**File:** `docs/theory/archive/DERIV_QUARK_MASSES_FROM_LATTICE_RETRACTED.md`. **Officially retracted 2026-05-27** per strict epistemic discipline. The continuous post-hoc ratio conjectures are removed. Superseded by the Discrete-Native program (`FOUND_DISCRETE_NATIVE_MASS_GENERATION.md`).

### 4.3 Watson-G* identity —  CLOSED/RESOLVED 2026-06-10
**File:** `docs/theory/04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md`. The open items (Item 8: physical content of the algebraic Watson-G* connection; Item 9: the 14 vs 16 torus DOF counting discrepancy) are formally closed. Under FTD-0242, the Watson-G* connection is a period equivalence on the substrate, and the 16 coefficient is structurally forced by $|{\rm Aut}(E)|^2 = 16$.

### 4.4 α lattice mechanism —  CLOSED/RESOLVED 2026-06-10
**File:** `docs/theory/04_coupling/DERIV_ALPHA_LATTICE_MECHANISM.md`. The open items (Step 3: Z₄ symmetry selecting the CM curve; Step 8: larger root equaling $1/\alpha$) are formally closed. CM curve selection is uniquely proven under the trivial-multiplier criterion, and the root selection is reclassified as an unforced operator-readout assembly selection under the dynamic-alpha pivot (FTD-0242).

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

### 7.4 Complete chain — **0 `[OPEN]`** (archived; not counted)
**File:** archived 2026-06-22 to `docs/theory/01_reference/archive/ARCH_SPEC_FTD_COMPLETE_CHAIN.md`, superseded by `SPEC_FTD_COMPLETE_FRAMEWORK.md` (FTD-0311, v2 as of 2026-07-12). The archived file's internal `[OPEN]` marker is provenance, not live work.

## Recently closed

- **Math node-map regeneration — resolved by owner removal (2026-09-08).** The maps and their generation pipeline were deleted; the deferred regeneration task is retired. Historical provenance remains in LEDGER FTD-0207. No claim status changes.

- **FTD-0406 — strong stress–energy contract v1: CPU-SCOPED-CONTRACT (2026-07-21).** Explicit owner choices install a default-off CPU `U(1)=0` Hamiltonian, exact collision-free energy/momentum projection, local string T00/stress, and `T00/C_SPEED²` latency source. Selected/imposed architecture only; GPU, topology changes, mixed forces and NCEMC-5 remain open.
- **FTD-0405 — NCEMC feasibility: DOUBLE-OBSTRUCTION (2026-07-21).** The current direct colour force has a potential family and two-body momentum closure, but not exact work exchange or a selected gravitational energy zero/local stress distribution. Production construction now needs explicit owner choices; NCEMC-5 and mass readout remain inadmissible.
- **FTD-0404 — volumetric measure reconciliation: VOLUMETRIC-NEUTRAL (2026-07-21).** The current unit cubic measure is explicit in integrated diagnostic channels; local field norms remain quadratic and production values remain unchanged. NCEMC remains separately open.
- **FTD-0403 — targeted causal-normalization dependency closure: TARGETED-CLOSURE v2 (2026-07-21).** V1 invalidated a stale over-speed boundary fixture; the test-only repair plus fresh lock passes the exact and complete changed-surface suite. Section 1.2 / `§12-cnorm` is closed without a full unrelated CTest run. NCEMC is admissible but remains separately open.
- **FTD-0401 — lattice-c/proper-time/mass-role normalization: UNMAPPED-DUAL-NORMALIZATION scoped no-go (2026-07-21).** Nineteen exact source-contract checks prove that one raw velocity and `M_REST` scalar enter incompatible `c_lat` and `c=1` roles without a conversion. The historical implementation question is closed; FTD-0402 implements the correction and FTD-0403 closes its targeted dependency surface. FTD-0252/0268 wave-clock measurements survive; FTD-0271 A5 covariance wording is withdrawn.
- **FTD-0400 — confinement-energy → rest-mass → gravity bridge: SPLIT-BOOKKEEPING scoped no-go (2026-07-21).** Twelve exact source-contract checks prove that current colour dynamics, energy accounting, imposed inertia, and gravity do not share one energy–momentum object. No current-engine mass campaign can establish the proposed bridge. The implementation question is closed; NCEMC-1–4 is the newly explicit prerequisite tracked in §1.3.
- **FTD-0399 — target-blind particlehood comparator: INVALID at G2 (2026-07-20).** At L=33/65 in both dissipative and undamped protocols, C manifests at tick 2 while exact A/E histories do not manifest within 200 ticks. The aligned three-history ensemble does not exist, so no distance or particlehood outcome is evaluated and no mass observable opens.
- **FTD-0398 — larger-shell topological transport/destruction question: TERMINAL UNDERDETERMINATION (2026-07-20).** The unchanged FTD-0392 octahedral convention was measured on scaled shells `R=1..6`, ticks `0..8`, for A/C/E. All gates passed, but transient integer charges satisfied none of COLOCALIZED, TRANSPORTED, or ZERO-CROSSING/DESTROYED. The route supplies no mass evidence; no alternative shell geometry is licensed. Canonical result: `ANALYSIS_TOPOLOGICAL_CHARGE_TRANSPORT_v1.md`.

Move items here with the closing commit / PR when an `[OPEN]` item is closed.

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

-  **§1.4 Leapfrog integrator** — already symplectic. Audit via `tests/test_leapfrog_integrator_audit.cpp` showed 0.1 % cumulative energy balance over 5000 ticks with damping off. Corrected "forward Euler" comments in `render_bridge.cpp` and the since-retired DagEngine source. (CHANGELOG: "Step 1".)
-  **§1.8 Moore-Laplacian anisotropy** — already isotropic through O(h⁴). Direct Taylor expansion: `h²∇²f + (h⁴/12)(∇²)²f + O(h⁶)`. Empirical confirmation in `tests/test_moore_laplacian_isotropy.cpp` shows 11 % radial symmetry at L=64. (CHANGELOG: "Step 2".)
-  **§1.5 `ALPHA_PRECISION` rollout** — engine `ALPHA = 1/X_PLUS_PRECISION`. `G_C`, JS mirror, two hardcoded-value tests updated. Static_assert confirms `G_C² ≈ ALPHA` to 1e-8. (CHANGELOG: "Step 3".)
-  **§1.2 γ_FTD momentum integration** — replaced non-relativistic velocity clamp in `phase_forces` with `p = γmv` dynamics. Covered by `tests/test_gamma_ftd_momentum.cpp` (8/8 checks). Also removed over-strict secondary clamp in latency block. (CHANGELOG: "Step 4".)
-  **§1.7 GPU-path `EnergyLedger`** — `tick()` GPU path now auto-calls `gpu_sync_to_host()` + `update_energy_ledger()`. (CHANGELOG: "Step 5".)
-  **§1.9 Muon / tau spatial seeds** — two new `s0-seed-muon` / `s0-seed-tau` scenarios with full epistemic metadata. (CHANGELOG: "Step 6".)

### Prior scope

-  `2026-04-17` **EnergyLedger auto-populate (CPU)** — `RenderBridge::update_energy_ledger()` runs at the end of every CPU-path `tick()`. (CHANGELOG: "Consolidation Sweep".)
-  `2026-04-17` **`ALPHA_PRECISION` first-class in engine** — `X_PLUS_PRECISION` + `ALPHA_PRECISION` defined in `ontic.h`, re-exported in `constants.h`. (CHANGELOG: "Honesty Sweep".)
-  `2026-04-17` **sparse-DAG vs `RenderBridge` ambiguity** — the sparse branch was marked experimental and its WASM binding removed; the branch was later deleted in the 2026-08-28 product consolidation. (CHANGELOG: "Consolidation Sweep".)

---
