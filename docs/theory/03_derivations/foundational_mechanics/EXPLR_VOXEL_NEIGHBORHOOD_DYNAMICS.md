# EXPLR: Voxel and Moore-Neighborhood Dynamics — the Complete Microdynamical Reference

**Status:** [SYNTHESIS] of engine-exact microdynamics + [DERIVED — lattice correctness] session results (2026-07-15/16) + two engine-fidelity defects surfaced in the same pass.
**Scope:** everything that happens *at one voxel and its 26-site Moore neighborhood* in a single tick — the free wave sector, the sourced (Gauss/coupling) sector, the state-transition kinetics (genesis/evaporation), the zero-temperature threshold structure, and the stability window the substrate's own geometry imposes on `K_B` and `K_GENESIS`.
**Provenance discipline:** the new results below were derived and numerically verified in-session (scripts under the session scratchpad; multi-agent adversarial review), cross-checked against the corpus, and are **provisional AI-derived content, not externally reviewed**. No LEDGER rows are minted by this document; no existing tag is promoted. Where a claim reproduces an existing corpus result, the corpus citation governs.

---

## §1. The voxel state vector

A voxel carries (engine/include/ftd/voxel.h):

| Field | Type | Role | Sector |
|---|---|---|---|
| `s` (`state`) | ternary {−1, 0, +1} | manifested charge/state; the ontic actual | discrete |
| `J` (`flux`) | ℝ³ | flux field; the dispositional layer | continuous |
| `wave_vel` | ℝ³ | Δ_t J — the field's conjugate momentum | continuous |
| `flux_L`, `flux_R` (+ `wave_vel_L/R`) | ℝ³ ×4 | independent left/right substrate fields (dual mode, default ON); `J = J_L + J_R` reconstructed each tick | continuous [IMPOSED] |
| `latency` (𝓛) | scalar | gravity sector, `f = 1 − 𝓛²` | continuous, quasi-static |
| `velocity`, `locked`, `spin`, `color`, `particle_id` | — | particle-kinematics and identity attributes of manifested voxels | discrete |

The dual split is genuinely independent field content — each of `J_L`, `J_R` carries its own Laplacian and its own leapfrog update (phase_read.cpp:77-144; phase_write.cpp:187-208), isolation verified by test_dual_substrate.cpp DS-WAVE (energy injected into L stays out of R to <1%). Its chirality physics is **[IMPOSED]** (FOUND_EPISTEMIC_SYMMETRIES_AND_CHIRALITY.md §7: "left-handed parity violation is an input parameter choice"); the in-code citation "Montanez & Claude 2026, *The Algebraic Identity of Two Substrates*" resolves to no file in the repository (open item, §9).

## §2. The tick at one voxel

Order of operations affecting a single site (render_bridge.cpp:541-754): `phase_read` (compute `Δ_j = c²·L₁₈J` from the 18 stencil neighbors) → `phase_write` (leapfrog `wave_vel += Δ_j; flux += wave_vel`, then damping/Langevin, then genesis/evaporation) → `pair_production` → `gauss_project` → latency solve → `phase_forces` → `phase_movement` → boundary → weak/triad → proper time.

## §3. Free wave sector — exact and theorem-grade

**Operator.** The dynamical Laplacian is the 18-point (SC+FCC)/2 stencil: 6 face neighbors at weight 1/3, 12 edge neighbors at weight 1/6, **8 corner (BCC) neighbors at weight exactly 0** (lagrangian.h:81-85; constants.h:379-383, Patra–Karttunen isotropic weights). The corner exclusion is the bit-exact discriminant separating the dynamical operator from the full 26-Moore neighborhood; the BCC sector — where the master quadratic's Watson integral lives — is deliberately not part of the wave dynamics (AUDIT_LINK8_CLOSURE.md). The symbol is σ₁₈(k) = 1 − (1/6)Σcos kᵢ − (1/6)Σcos kᵢ cos kⱼ, spectrum bounded in [−16/3, 0], band bottom σ₁₈ = |k|²/4 − |k|⁴/48 + O(k⁶) — the O(k⁴) term is isotropic but **nonzero**; isotropy at that order is a property of the engineered weight choice, not forced by P1–P5 (the native 26-Moore coupling is O(k⁴)-anisotropic).

**Update and speed.** Leapfrog with dt = 1, `c² = 1/3` (CFL-derived, c = 1/√3 [DERIVED] from {D=3, cubic lattice, leapfrog}; gauge_couplings.h:210-224). Exact leapfrog dispersion `sin²(ω/2) = c²σ₁₈(k)`; session verification to 5.8×10⁻¹⁴ across the Brillouin zone; engine verification <0.1% (campaign_dispersion.cpp). The free cone is massless and linear at the band bottom, ω → |k|/√3.

**Single-voxel kernel [DERIVED — lattice correctness, session-verified].** A unit perturbation `J₀ = 1` at one voxel is pure neighbor-difference potential energy, PE(0) = 2c²J₀² = 2/3 (matched to 10⁻¹⁵). One tick later the amplitudes are bit-exact: center −1/3, each face neighbor +1/9, each edge neighbor +1/18, every corner 0.0. The disturbance fans out one Moore shell per tick (L∞ front radius = tick number, exactly, for ticks 1–23 at L=48) — the lattice light cone, P4 causality made visible. Linearity makes the general solution a superposition of these kernels; interference is native [THEOREM].

**Conserved energy.** The exactly conserved tick quantity is E_tick = ½WᵀW + ½JᵀKJ − ½WᵀKJ (K ≡ −c²L₁₈), carrying a cross term absent from the naive continuum energy; the naive ½W² + ½Jᵀ(−K)J drifts (~0.80 over a comparison run) while E_tick is conserved to ~10⁻¹⁵ (DERIV_DISCRETE_TICK_ENERGY_INVARIANT, FTD-0292–0297; scope: source-free linear sector). The proof is a direct transfer-matrix invariant construction (AᵀMA = M); reading it as a discrete Noether charge is correct physics but is an explanatory gloss this corpus does not itself make — the discrete Noether bridge for the full engine remains open (FOUND_EPISTEMIC_SYMMETRIES_AND_CHIRALITY, [CONJECTURE]).

**Envelope reduction [CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT].** With an imposed rest-mass clock (−ω₀²J; the native flux is massless — FTD-0270 stands), the exact discrete-time substitution ψ = φe^{−iΩ₀n} at integer ticks yields the identity cosΩ₀·Δ_t²φ − 2i sinΩ₀·D_t^c φ = c²L₁₈φ with the exact lattice rest frequency sin²(Ω₀/2) = M²/4, and, after the slow-envelope drop (|k| ≪ 1), a discrete Schrödinger equation on the same 18-point stencil with **m_eff = sinΩ₀/c² = 3 sinΩ₀** — the exact discrete correction to the naive continuum 3Ω₀ (−3.2% at Ω₀ ≈ 0.505). Session numerics: envelope spreading matches the Schrödinger σ(t) law to −4.24% at σ₀ = 6, converging as 1/σ₀² (the isotropic k⁴ band-flattening — a native, falsifiable lattice deviation, not an error); group velocity is lattice-exact, not continuum. The complexification is [SELECTION] (instance of the declined map M; no native L², FTD-0208); Born stays declined under FC-1. The Schrödinger *physics* remains [PARAMETRIC — imported standard QM, conditional] per FTD-0356; the identity chain above is the lattice-exact spine only.

## §4. Sourced sector — Gauss self-field of a manifested voxel

The ternary axiom fixes the charge quantum |s| = 1 exactly, and the Gauss projection (default ON, every tick) targets div J = charge_coupling·(s − mean_charge) (poisson_solvers.cpp:164; charge_coupling default 1.0). The static self-field of an isolated manifested voxel is therefore **pure lattice geometry with zero free parameters**. Session computation (L=64, exact solvers, three operator conventions):

| Quantity | Engine central-difference ops | Spec/link (forward-difference) ops |
|---|---|---|
| `c₀` = \|J\| at the particle site | 0 (symmetry) | 0 |
| `c₁` = \|J\| at each face neighbor | **0.3332 ≈ 1/3** | 0.1049 |
| `c₂` = E_local (self + 6 neighbors, Σ\|J\|²) | **0.6662 ≈ 2/3** | 0.0660 |
| ratio `c₂/c₁²` | **6 (exact)** | 6 (exact) |

The value 1/3 is forced by arithmetic: central-difference div at the charge site reads only the six neighbors, so div = 3·J_nbr = 1 ⇒ J_nbr = 1/3. The 3.2× disagreement between operator conventions is another face of the documented divergence/gradient stencil mismatch (constants.h:328-353). `c₂/c₁² = 6` — six face neighbors — is convention-independent and carries the window structure below.

**Engine-fidelity defect (surfaced this pass, chip task_92dc33a4):** the engine's *measured* settled self-field is J(r=1) ≈ 9.9×10⁻³ (DERIV_KCOMP_VOLUMETRIC_SHELL.md, 128³ GPU), ~34× below the constraint-required 1/3 — the Gauss constraint is only ~3% realized at particle sites. Traced candidate causes: the flux correction skips manifested sites (poisson_solvers.cpp:198; kernels_poisson.cu:346), the 18-pt-solve/6-pt-divergence mismatch, selective damping draining near-particle flux between projections, and 6 SOR iterations/tick on CPU. Until resolved, the "axiom-level" and "engine-realized" columns of §7 both apply, and they differ.

The earlier corpus figure E_field/K_B² = 0.118 (KCOMP) is not in tension with these numbers: that run *injected* a wavepacket L2-normalized to Σ|J|² = K_B² and measured the surviving fraction after 1000 damped ticks — a decay measurement of an injected packet, not the Gauss-forced self-field of a bare charge.

## §5. State-transition kinetics — genesis

**Trigger and rate (dual-substrate default; phase_write.cpp:326-338).** A void voxel fires iff |J| > K_GENESIS (hard gate), then manifests with per-tick probability p = 1 − exp(−(|J| − K_GENESIS)/K_MANIFEST). Sign from `chirality_density()` (≥0 → +1); **no field quantity is modified** — genesis in the default mode is a pure label event, verified to 10⁻¹² (test_native_manifestation_ledger.cpp NML-1c/2c). The single-substrate branch differs: divergence sign, and a real drain (wave_vel ×(1−kinetic_drain), flux ×(1−K_GENESIS/|J|)).

**Why 1 − e^{−Γ} [session derivation].** The genesis draw is recomputed fresh each tick from the current excess, with no persistent above-threshold age variable anywhere — the process is memoryless by construction. For a memoryless process, 1 − e^{−Γ} is the unique subdivision-consistent form: partitioning one tick into N micro-steps of hazard Γ/N gives survival (1 − Γ/N)^N → e^{−Γ} independent of N (verified numerically to 9 digits). Any other smooth curve would give answers that depend on how finely time is imagined subdivided. The corpus's own justification ("chosen for smoothness", [SELECTION]) is thereby strengthened to: the only shape consistent with the code's own memoryless structure.

**Why the hard gate.** The zero-below-threshold cutoff is not part of the memorylessness argument; it is the classical nucleation barrier of a substrate with no native softening mechanism — no thermal bath in contact with genesis (the Langevin thermostat is dead code on the default dual path and unwired to K_MANIFEST), no tunneling. A finite-temperature Boltzmann treatment was tested and excluded: it smooths the threshold *itself* (nonzero probability below the gate), which the code's exact zero contradicts. The correct reading is barrier-plus-rate — classical nucleation kinetics for the first-order transition the corpus already asserts genesis to be (FTD-0272: maximal hysteresis, strongly first-order).

**Why Γ is linear in the excess [session derivation, corrected].** Linearity follows from a constant hazard-rate density: independent memoryless hazard contributions compose by adding exponents, so any partition of a fixed total excess X yields exactly 1 − e^{−X/K_MANIFEST} (verified to machine precision across even/uneven/10⁴-piece partitions). A literal fixed-size-quantum counting picture is excluded — it predicts a staircase (zero marginal hazard below each completed K_MANIFEST quantum) that the smooth formula contradicts. What remains assumed rather than derived is the constancy of the rate density itself; classical nucleation theory would generically make the rate exponential in the driving force, so the linear law may itself be the leading form of a fuller kinetics [OPEN].

**Spectral sensitivity (session numerics).** The per-tick formula reads only the scalar |J|, but real dynamics make outcomes spectrum-dependent through dispersion: at matched peak |J| = 1.65, a broadband 25-mode packet manifests less often than a coherent single-mode packet (P = 0.204 ± 0.007 vs 0.239 ± 0.008, 3.3σ, boundary-safe window) because its components disperse from the site faster, tracking the time-integrated excess. A dispersion consequence, not a modification of the rate law.

## §6. State-transition kinetics — evaporation

CPU rule (phase_write.cpp:362-379): for unlocked manifested voxels, E_local = Σ(|J|² + |wave_vel|²) over self + 6 face neighbors; p_evap = K_EVAP_RATE · exp(−E_local/K_MANIFEST²), K_EVAP_RATE = 0.1 (an unpriced bare literal — no epistemic tag anywhere in the corpus; extracted from a magic number in the 2026-04 pre-refactor audit). Locked voxels are exempt.

**Engine-consistency defect (chip task_0f1009e5):** the GPU implements *different physics* — a deterministic cutoff, evaporate iff E_local < K_MANIFEST²·10⁻⁶ (kernels_stencil_single.cu:532-533). At any settled self-field, GPU particles never evaporate while CPU particles die at ~9%/tick (test_boundary_modes_golden.cpp:76-80: mean isolated lifetime ~11 ticks, L=17). Every lifetime claim is backend-dependent until reconciled. The lifetime structure below uses the CPU (theory-documented) rule.

## §7. The stability window — what the substrate itself pins

Combining §4's geometric constants with §5/§6's kinetic rules gives a dimensionless window for K_B in charge-quantum units (physical, not gauge: |s| = 1 is axiomatic). With K_GENESIS = N·K_B:

- **Vacuum stability (no cascade):** a lone particle must not auto-trigger neighbor genesis: c₁ < N·K_B ⇒ **K_B > c₁/N** (= 1/9 ≈ 0.111 at axiom level, N = 3; current margin K_GENESIS/c₁ ≈ 4.6×).
- **Persistence:** p_evap ≤ ln2/τ ⇒ **K_B ≤ √(c₂/X_τ)**, X_τ = ln(0.1τ/ln2): 0.366 (τ=10³), 0.237 (10⁶), 0.161 (10¹²).
- **The current K_B = 0.511 sits above the persistence band**: settled-isolated half-life ≈ 89 ticks at the axiom-level field, ≈ 8–11 ticks at the engine-realized field — quantitatively reproducing the independently established metastability verdict (FTD-0301 proton UNFORCED-METASTABLE; FTD-0267 survival telemetry). Newborns are protected while they still hold their ≥K_GENESIS birth flux (exponent ≥ 9.25, p ≤ 10⁻⁵); fragility begins only after the birth flux disperses.
- **Threshold-multiplier structure (convention-independent):** because c₂/c₁² = 6 exactly, the minimal integer N keeping the window non-empty is N_min(τ) = √(X_τ/6): N=1 suffices for 10³ ticks, N=2 for 10⁶, **N=3 for 10¹²**; N=3 caps isolated persistence at ~2×10²⁴ ticks. This is arithmetic, and the lifetime target is a free choice: a consonance observation for N_c = 3, **not** a derivation of it (provenance of record remains the Moore-layer routes). In physical time even the N=3 cap is ~10⁻¹⁹ s — isolated particles are ephemeral for any N; persistence in FTD is necessarily collective (ambient flux, locking, binding), consistent with observed cluster flooding (SPEC_CLASS_B) and the latched condensate regime (FTD-0272), where ~5+ superposed self-fields overcome the 4.6× single-particle margin.
- Prior art: the corpus had the mechanism sentence (FTD-0267) and the collective-runaway regimes, but the explicit single-particle no-cascade inequality and the K_B window had not been assembled; the one prior attempt to derive the lattice-unit 0.511 (EXPLR_FTD_MASS_CHAIN) is red-teamed [COORDINATE COINCIDENCE] and is unrelated to this route.

Tags: c₁, c₂, c₂/c₁² = 6, and the window inequalities are [DERIVED — lattice geometry + stated kinetic rules]; the window's location is conditional on the [IMPOSED] evaporation formula and the unpriced K_EVAP_RATE; K_B's MeV value stays [IMPOSED — calibration] (FTD-0059/0096 close that question permanently).

## §8. Zero-temperature threshold structure — the slaved-variable result

The ternary state carries no kinetic term anywhere in the action; it is a constrained (slaved) variable in the sense of electromagnetism's A₀, and the correct variational treatment is instantaneous extremization over s ∈ {−1, 0, +1} at fixed J. For the s-dependent terms that exist in the action — coupling −g_c·s·(div J) and Gauss −λ_G(div J − s)² — the argmin is [DERIVED, session]:

U(s) = g_c·s·(div J) + λ_G(div J − s)², threshold at div J = λ_G/(2λ_G − g_c) = **0.5002136526** (λ_G = 100, g_c = √α), argmin flipping 0 → sign(div J) there, reversibly and without hysteresis at zero temperature; genesis and evaporation are two faces of one energy criterion on this construction.

Scope, stated exactly: this reproduces the sign-and-threshold structure of the **single-substrate (non-default) divergence path only**. The default path is out of reach of the existing action for two now-precise reasons: (i) the trigger variable |J| and the sign variable (chirality) appear nowhere in the action — bare |J|² has no energy interpretation in any of the six Lagrangian terms; (ii) the Born-Infeld rest term is s-independent in the implemented Hamiltonian (h_bi depends only on particle speed), so the rest-mass cost K_B structurally cannot enter any s-comparison — the missing ingredient is a rest-mass term conditioned on s ≠ 0 [OPEN]. A static energy-budget reading of K_GENESIS (local Term-6 gradient energy ≈ rest cost at the trigger amplitude) was tested and failed: the near-match at single-site measurement (0.434 vs 0.511) collapses (5.26, >3×) under a neighborhood measure — the agreement is measure-dependent, hence not real.

## §9. Open items surfaced by this pass

1. Gauss under-enforcement at manifested sites (~3% realized) — chip task_92dc33a4; load-bearing for every electrostatics-adjacent engine claim.
2. CPU/GPU evaporation physics split — chip task_0f1009e5; all lifetime measurements backend-dependent until reconciled.
3. Phantom citations: `test_variational_proof.cpp` (cited as 60-check verification of DERIV_VARIATIONAL_PROOF.md's [THEOREM] header; never committed in git history) and "Montanez & Claude 2026" (dual-substrate origin paper; no file in repository). Same failure class as the FTD-0217/0218 retraction; both need correction or retraction of the dependent tags.
4. K_EVAP_RATE = 0.1 unpriced — needs an ASSUMP-ledger row or an import-ledger line.
5. The default-path genesis derivation gap, now precisely characterized (§8): a conditional rest-mass term is the nameable missing content; the chirality sign rule is a decided [IMPOSED] import, not an open derivation target.
6. The linear-vs-exponential rate-law question for Γ (§5) — whether constant hazard density is itself the leading approximation of a fuller nucleation kinetics.
7. **State-sector action attempt — adversarially REFUTED before adoption (2026-07-16, dual math+physics redteam; recorded to prevent re-attempt).** A proposed term L_state = −K_B·s²(1−|J|/K_GENESIS) with downhill-only linear-hazard kinetics reproduced the genesis formula exactly but died on five findings: (i) the argmin silently drops the λ_G constraint term, O(100) at flip time; (ii) the integrated-out constraint yields a *potential* threshold |φ_ext| > E_self, physically inequivalent to the engine's *flux-modulus* threshold (discriminator: the midpoint between two like charges — potential near-maximal, |J| ≈ 0); the proposal mixed two mutually exclusive readings (FTD-0361 class); (iii) N_c cancels identically from all observables — the "K_GENESIS = N_c·K_B emerges as a coupling ratio" reading is empty (definitional, particle_masses.h:46); (iv) the same potential predicts ~95%/tick evaporation at settled sites — an opposite-sign energy landscape vs the engine's Arrhenius rule, an inconsistency not a scope gap; (v) δL/δJ implies an s²Ĵ force ≈ 4× the EM coupling, absent from the engine, which would break the spec §3.6 machine-epsilon EL check. **Surviving [THEOREM]-grade math** (name the constant W_SC — the SC Watson integral, NOT the spine's BCC W₃ = 1.3932): E_half(L) = S(L/2) exactly → W_SC = 0.5054620197 with law W_SC − ξ/(πL) + O(1/L³); Term-6 gradient energy of the same field = (4/3)·E_half exactly; forward pseudo-inverse → W_SC/4; link-average convention → (3W_SC−1)/8. The "K_B ≈ W_SC" 1.1% value match is inadmissible ([COORDINATE COINCIDENCE] class per AUDIT_MASS_CHAIN_REDTEAM Axis B, which demoted a 67-ppm match; calibration noise floor: 2W_SC² sits 32 ppm from 0.511). **The one live salvage** [CONJECTURE — falsifiable]: the prescriptive substrate-internal form — K_MANIFEST := the measured self-energy of the Gauss projector's unit-charge fixed point in a pinned convention — decidable by a pre-registered GF-A-protocol measurement with frozen analytic predictions for the SC vs 18-pt vs ±2h Green's functions (FTD-0116 precedent governs: a non-W_SC result closes the line permanently).

### §9.1 Frozen prediction — W_SC persistence under the Gauss-enforcement repair (registered 2026-07-17, BEFORE the fidelity investigation's fix lands)

The tri-lattice role reading (SC = measure sector; see LEMMA_DEGREE_QUARANTINE.md) makes a falsifiable commitment about the in-flight Gauss under-enforcement repair (chip task_92dc33a4): **whatever mechanism is repaired, the projector's unit-charge fixed-point self-energy must remain at W_SC(L)** (tracker convention, the FTD-0388 values: 0.478917/0.491780/0.498515 at L=17/33/65), because the divergence read that defines the constraint is SC-shaped (6-neighbor central difference) and the repair targets *enforcement strength*, not the operator's sector. **Falsifier:** a post-repair fixed-point self-energy drifting toward an 18-pt-mixture value (P2 family, ~0.157) or any other constant would break the "SC = measure sector" role map and require reopening FTD-0388's geometric attribution. Companion open derivation: tie the odd-L convergence cost (applications ∝ L², measured 1,380/4,300/13,240) analytically to the sin²(π/L) gap at the near-checkerboard modes — converting the §9-adjacent mechanism finding from observed to [DERIVED].

**Outcome of record (2026-07-18) — prediction UPHELD.** The repair landed as the Term-2 electric-coupling sign amendment (`SPEC_FTD_LAGRANGIAN.md` §3.3: $-g_c\,s\,(\nabla\cdot J) \to +g_c\,s\,(\nabla\cdot J)$; `phase_read` source $+g_c\nabla s \to -g_c\nabla s$), after the fidelity investigation identified the coupling term as in sign conflict with the constraint term at charge sites. The repair touches the *dynamics only*: the projector operator is bit-identical pre/post (GF-A isolation replicates $f = +0.9992$ exactly), so its unit-charge fixed point — and therefore its self-energy — remains at W_SC(L) by construction *and* by re-measurement; no drift toward the ~0.157 P2 family. Live shipping-default equilibrium moved from $f = -0.095$ (wrong-signed, inward flux at a $+1$ charge) to $f = +0.114$ (constraint-aligned, outward). The residual enforcement gap (0.114 vs 1.0) is the leapfrog's `wave_vel` longitudinal reservoir, which the flux-only projector never cleans; completing enforcement would require projecting the velocity field's longitudinal sector (constraint transport $\nabla\cdot v = \Delta_t\rho$) — a substrate-wide scope decision (it would also extinguish longitudinal vacuum modes engine-wide) recorded as [OPEN], not part of the amendment.

## §10. Claim ledger for this document

| Claim | Tag |
|---|---|
| 18-pt stencil weights, corner-zero, spectrum, dispersion sin²(ω/2)=c²σ₁₈ | [THEOREM] / engine-verified |
| Single-voxel kernel (−1/3, +1/9, +1/18, 0), light cone, PE(0)=2/3 | [DERIVED — lattice correctness, session-verified] |
| E_tick invariant with cross term | [THEOREM] (FTD-0292–0297), source-free linear scope |
| Envelope identity, sin²(Ω₀/2)=M²/4, m_eff=3sinΩ₀ | [CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]; Schrödinger physics stays [PARAMETRIC] (FTD-0356) |
| c₁=1/3, c₂=2/3, c₂/c₁²=6 (axiom-level self-field) | [DERIVED — lattice geometry, session] |
| Engine realizes ~3% of Gauss at particle sites | [MEASURED — corpus values vs constraint requirement]; mechanism RESOLVED 2026-07-18 (Term-2 coupling sign conflict; amended, live f −0.095 → +0.114 constraint-aligned); full-enforcement completion (wave_vel longitudinal projection) [OPEN] |
| 1−e^{−Γ} unique under memorylessness; hard gate = nucleation barrier; Γ-linearity ⟺ constant hazard density | [DERIVED, session] with the constancy assumption [OPEN] |
| Spectral (dispersion-mediated) genesis dependence, 3.3σ | [MEASURED — session sim] |
| Stability window; K_B=0.511 above persistence band; metastability reproduced | [DERIVED given [IMPOSED] kinetics, session] |
| N_min(τ)=√(X_τ/6); N=3 ⟺ 10¹²-tick persistence | consonance observation, NOT a derivation of N_c |
| Argmin threshold 0.5002, sign=sign(div J), no hysteresis | [DERIVED, session; non-default path only] |
| Default-path trigger/sign underivable from existing action | [DERIVED — structural absence]; missing term named [OPEN] |
