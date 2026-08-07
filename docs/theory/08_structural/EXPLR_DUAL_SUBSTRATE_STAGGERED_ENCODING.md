# EXPLR: The Dual Substrate as a Sum/Difference Register — the Staggered Re-encoding Adjudicated

**Status:** [THEOREM — lattice operator algebra] for the (F, D) decoupling (§1–§4, §6) and the checkerboard-conjugation computation (§8); [FACT — engine inventory] for the touchpoint census (§5) and the T3 amendment trigger (§9); [CONJECTURE — semantics only] for the corner-register reading in its surviving weak form (§8.3). Verifier: `scripts/proofs/proof_dual_substrate_staggered_encoding.py`, **15/15**. Provisional AI-derived content, not externally reviewed; provenance: the 2026-07-17 staggered re-encoding investigation chartered off [`LEMMA_DEGREE_QUARANTINE.md`](LEMMA_DEGREE_QUARANTINE.md) and [`AUDIT_DUAL_SUBSTRATE_PROVENANCE.md`](../07_assessment/spine_gstar_cm_modular/AUDIT_DUAL_SUBSTRATE_PROVENANCE.md).
**Zero promotions:** the dual substrate stays **[IMPOSED]** (engine design choice); no LEDGER row is minted; no α content appears anywhere in this note; the corpus-level dual-substrate objects (E_L, E_R, δ² = 1 − 1/(4G\*)) are cited at their provenance-audit standing (DSP-3/DSP-4: Vieta dressing, not independent constraints). The BCC/checkerboard connection is adjudicated *against* its strong form here and survives only as a kinematic relabeling.

---

## §0. Setting and scope

The engine's dual-substrate mode carries two genuinely independent per-voxel vector fields `flux_L`, `flux_R` (with velocities `wave_vel_L`, `wave_vel_R`), each stepped by its own 18-point Laplacian and leapfrog, with the observable reconstructed every tick as `flux = flux_L + flux_R` ([`voxel.h:77-109`](../../../engine/include/ftd/voxel.h), [`phase_read.cpp:77-144`](../../../engine/src/render_bridge_phases/phase_read.cpp), [`phase_write.cpp:187-227`](../../../engine/src/render_bridge_phases/phase_write.cpp)). `dual_substrate = true` is the **default** toggle, as are `wave_propagation`, `coupling`, `damping`, `genesis`, `gauss_projection`, and `weak_transmutation` (`term_toggles.h:54-60, 187-202`) — everything analyzed below is the canonical golden-gated tick, not an exotic branch.

Define the invertible change of register

  F := J_L + J_R  (observable sum),  D := J_L − J_R  (difference field),
  V_F := W_L + W_R,       V_D := W_L − W_R,

with inverse J_L = (F+D)/2, J_R = (F−D)/2. The question adjudicated: is (F, D) an **exact re-encoding** of the dual substrate — F carrying the entire single-substrate dynamics, D decoupling everywhere except a short declared list of chirality touchpoints — and does the checkerboard-weighted variant G = (−1)^{x+y+z} D make the "chirality lives in the corner register" reading literal?

The CPU `RenderBridge` tick is the reference dynamics; CUDA parity is inventoried where it diverges. The scope is the C++ engine only (the JS MockBridge is out of scope). Notation follows the quarantine lemma: c_i = cos k_i, e₁, e₂, e₃ the elementary symmetric polynomials, σ₁₈(k) = 1 − e₁/6 − e₂/6 the 18-point symbol, L₁₈ the operator with plane-wave eigenvalue −4σ₁₈(k).

## §1. The decoupling lemma [THEOREM]

**Lemma.** Let an update act on the pair (J_L, J_R) as the *same* affine-linear map on each register with a *shared* source b:

  J_L ↦ A J_L + b,  J_R ↦ A J_R + b,

where A is any linear operator (possibly site-dependent, possibly state-dependent, the same for both) and b any vector field. Then in (F, D) coordinates the update is block-diagonal:

  F ↦ A F + 2b,  D ↦ A D.

D receives no source and evolves under the homogeneous part alone; F receives the doubled source. *Proof:* linearity of the basis change. ∎

Every dual-substrate operation in the engine is either of this symmetric form (§2, §5.1 — D-invisible), or one of finitely many declared exceptions (§5.2 — the chirality touchpoints). That dichotomy, verified against the complete code surface, is the content of this note.

## §2. The exact per-phase update maps

Per tick (CPU, default integrator, dt = 1; `render_bridge.cpp:600-753` fixes the phase order), the operations that touch (J_L, J_R):

**Rule 1 — phase_read** (`phase_read.cpp:77-144`): for X ∈ {L, R}

  ΔJ_X = c² L₁₈ J_X + 𝟙_coupling · (G_C/2) · (−∇s + ∇×(s·v)) − 𝟙_clock · ω_eff² · J_X,

with c² = C_WAVE² = 1/3, the coupling source **split equally** (`G_C * 0.5` into each register, lines 122-126), and the de Broglie clock a shared scalar multiplier on each register's own value (lines 133-140). Both operators and both coefficients are identical across X. *(Electric-source sign per the Term 2 amendment of 2026-07-18, `SPEC_FTD_LAGRANGIAN.md` §3.3; transcription corrected 2026-08-04 to match `phase_read.cpp:208-213` and `kernels_stencil_dual.cu:141-147`. The §3/§4 theorems turn only on the 50/50 split being identical across L and R, so they are unaffected by this sign.)*

**Rule 2 — phase_write leapfrog + damping** (`phase_write.cpp:187-227`): W_X += ΔJ_X; J_X += W_X; then, where damping applies, (J_X, W_X) ↦ λ(x)·(J_X, W_X) with the *same* site-dependent factor λ(x) (Larmor-modulated or not) on both registers. All three integrator variants (default, `symplectic_leapfrog`, `verlet_wave_integrator` including the Rule-2a second half-kick at `render_bridge.cpp:646-652`) use identical coefficients on L and R. Observable sync: flux := J_L + J_R, wave_vel := W_L + W_R (lines 226-227) — the engine's observable *is* F by construction.

**Rule 3 — Gauss projection** (`poisson_solvers.cpp:210-216`): flux −= ∇φ at gated sites; in dual mode each register receives `half_corr = ∇φ/2`. The GPU formulation (`kernels_stencil_dual.cu:554-572`) computes delta = J_new − (J_L+J_R) and adds delta/2 to each — algebraically the same map, and its own comment states the invariant: "keeps chirality unchanged."

**Rule 5 — phase_movement** (`phase_movement.cpp:163-234`): the portable self-field carry multiplies both registers by the same F-dependent scalar frac = min(|F|, K_B)/|F| and transfers; annihilation distributes both registers by the same 1/6 weights to the same neighbors; face-crossing void reset and annihilation zero both registers at the site. Weak-transmutation and injection aside, every coefficient is shared.

**Rules 5b/5c — boundary passes** (`phase_write.cpp:411-498`): sponge and dispersal scale all six flux fields by one scalar; reflective copies all six — diagonal in any basis.

In (F, D) coordinates [THEOREM, by §1]:

  V_F += c² L₁₈ F + 𝟙_coupling · G_C · (−∇s + ∇×(s·v)) − 𝟙_clock · ω_eff² F;  F += V_F;  (F, V_F) ↦ λ(F, V_F)
  V_D += c² L₁₈ D − 𝟙_clock · ω_eff² D;              D += V_D;  (D, V_D) ↦ λ(D, V_D)
  Gauss: F −= ∇φ (gated sites); D untouched.
  Movement/annihilation/boundary: F and D transported/attenuated by identical coefficients.

## §3. The F-sector theorem [THEOREM]

The F-sector update above is **exactly the engine's single-substrate branch**: the same L₁₈, the same full-strength G_C source (the two half-sources recombine), the same clock term, the same damping, the same Gauss subtraction (`phase_read.cpp:145-205`, `phase_write.cpp:228-272`). Verifier T2 confirms this at the strongest possible level: the (F, D)-form F-trajectory and an independent single-substrate run are **bit-exact** (max deviation 0.0 over 50 ticks) — they are the same floating-point program, not merely equivalent algebra.

Consequently the dual substrate's *observable* sector adds nothing to the single-substrate dynamics. Whatever the dual substrate contributes beyond one field lives entirely in D.

## §4. The D-sector: a hidden free field [THEOREM]

Wherever the dynamics is L/R-symmetric, D obeys the **source-free** wave equation under the *identical* operator:

  V_D += c² L₁₈ D (− ω_eff² D when the clock is on);  D += V_D;

damped by the same λ(x), transported by movement with the same coefficients, invisible to the Gauss constraint, invisible to forces, evaporation, pair production, and every F(L₁₈)-type observable. The state-flux coupling sources F only (the G_C/2 + G_C/2 split cancels in the difference), so **manifested matter does not source D**: chirality content enters D exclusively through the declared touchpoints of §5.2 and decays or radiates under the same damping/wave dynamics as F. Two structural corollaries:

- **Degree quarantine inherited.** D evolves under the same σ₁₈ (degree ≤ 2) symbol; the dual substrate introduces *no new operator*, so T1/T2 of [`LEMMA_DEGREE_QUARANTINE.md`](LEMMA_DEGREE_QUARANTINE.md) apply verbatim to D. The dual substrate cannot smuggle corner-sector (e₃) content into the dynamics.
- **Spectral identity.** D is a second copy of the same wave sector; (F, D) is a direct sum of two identical σ₁₈ sectors coupled only at the touchpoints, not a doubling of the Brillouin zone.

## §5. Complete touchpoint inventory [FACT — engine census, 2026-07-17]

Census method: every occurrence of `flux_L|flux_R|wave_vel_L|wave_vel_R|chirality_density` across `engine/{src,include,cuda,wasm}` (52 files, 473 matches) classified; all dynamics-relevant sites listed below with file:line.

### §5.1 L/R-symmetric operations (D-invisible by §1)

| Operation | Site | Form |
|---|---|---|
| 18-pt Laplacian, both registers | `phase_read.cpp:89-118`; `kernels_stencil_dual.cu:96-127` | same L₁₈ each |
| Coupling source split | `phase_read.cpp:122-126`; `kernels_stencil_dual.cu:129-156` | (G_C/2)(−∇s + ∇×(s·v)) to each ⇒ sources F only |
| de Broglie clock (both variants) | `phase_read.cpp:133-140`; `kernels_stencil_dual.cu:158-170` | −ω_eff²·(own register), shared scalar |
| Leapfrog, 3 integrator variants | `phase_write.cpp:188-208`; Rule-2a `render_bridge.cpp:646-652`; `kernels_stencil_dual.cu:328-343` | identical coefficients |
| Damping incl. Larmor modulation | `phase_write.cpp:211-223`; `kernels_stencil_dual.cu:345-359` | same λ(x) both registers |
| Observable sync F := L+R | `phase_write.cpp:226-227`; `render_bridge.cpp:336-341`; `kernels_stencil_dual.cu:361-367` | definition of F |
| Gauss projector correction | `poisson_solvers.cpp:210-216`; `kernels_stencil_dual.cu:554-572` | F −= ∇φ; D untouched |
| Movement self-field carry | `phase_movement.cpp:163-180, 273-290` | shared scalar frac(|F|) |
| Annihilation redistribution | `phase_movement.cpp:198-234, 307-344` | shared 1/6 weights |
| Void resets (face crossing, annihilation) | `phase_movement.cpp:81-85, 216-219` | both registers zeroed |
| Boundary sponge / reflective / dispersal | `phase_write.cpp:440-498` | one scalar / copy, all six fields |
| Flux/wave-vel add & set injections | `injection.cpp:50-53, 61-65, 71-75, 307-311`; `gpu_engine.cu:620-623`; `bindings_render_bridge.cpp:101-104` | half/half ⇒ F-writers, D + 0 |
| Background/EFT field copies | `coupling_measurement.h:117-130`; `manifestation_background.h:151-163` | copy both |
| Serialization | `engine_state.h:274-277` (FieldSoA) | stores both |

### §5.2 Asymmetric operations — the chirality touchpoints

This list is the deliverable: **every** place where L and R are treated differently, i.e. where D is written with a source or read as a signal.

| # | Touchpoint | Site | (F, D) form |
|---|---|---|---|
| A1 | Particle injection δ-split | `injection.cpp:102-112`; `gpu_engine.cu:645-652` | F := J₀; **D := sign(state)·δ·J₀** (assignment), δ = `DELTA_APPROX` = 0.9568 (4-digit hardcode of √(1−1/(4G\*)) = 0.956819063…, `master_quadratic.h:133`) |
| A2 | Wavepacket injection δ-split | `injection.cpp:174-184`; `gpu_engine.cu:686-722` | **D += sign(state)·δ·ΔJ(x)** per shell site |
| A3 | Neutrino constructor stamp | `constructors_molecules.cpp:106-143` | L, R := (1±d)/2·g(r)·x̂ with **local d = ±0.1** (its own parameter, *not* the ontic δ); writes D without updating F (F lags one tick until the next observable sync) |
| A4 | Weak-transmutation **trigger** | `transmutation_phases.cpp:26-28` → `render_bridge.h:427` (`stress_field<&Voxel::flux_L>`) | reads stress[(F+D)/2] where stress = \|div\|+\|curl\|+\|∇\|·\|\| — an **L-only nonlinear readout** (the [IMPOSED] SU(2)-motivated V–A coupling; declared in `campaign_parity_violation.cpp:12-20`) |
| A5 | Weak-transmutation **action** | `transmutation_phases.cpp:34-38`; `kernels_aux.cu:174-185` | swap(L, R) ≡ **D ↦ −D** (flux *and* wave_vel), F exactly invariant; state ↦ −state |
| A6 | Genesis polarity readout (dual) | `phase_write.cpp:326-337` → `voxel.h:92-109`; `genesis_dual_kernel` `kernels_stencil_dual.cu:407-411` | reads sign(χ), χ = F_⊥·D_⊥ (§6) — an odd-in-D readout that writes the discrete state sign; fields untouched |

The closed loop of chirality flow is therefore: **injection writes D (A1–A3) → D radiates/damps as a free field (§4) → the weak sector reads (F+D)/2 and flips D's sign (A4–A5) → genesis reads sign(F_⊥·D_⊥) into the state sign (A6) → state sources F (never D)**. Chirality re-enters the symmetric sector only through the discrete state channel.

*(Amendment 2026-07-18, commit `109dc45a` — §5.3 item 2 reconciliation: `create_entangled_pair` now applies the A1 state-signed δ-split at both pair sites, making it a fourth D-writing injection touchpoint of the same form as A1 (D := sign(state)·δ·J at each site). The closed-loop structure above is unchanged; the A1 row's convention simply covers one more injector.)*

### §5.3 D-erasers, register bypasses, and CPU/GPU divergences [FACT]

Operations symmetric in treatment but material to D's semantics, plus code-level findings surfaced by the census (flagged for owner attention; none affects §1–§4):

1. **D-erasers.** `inject_flux` (set) writes L = R = J₀/2, i.e. **D := 0** at the site (`injection.cpp:50-53`); `clearField` and the movement void resets zero D locally. A "set"-style injection destroys chirality memory; only the δ-split injectors create it.
2. **`create_entangled_pair_cpu` bypasses the register** (`injection.cpp:190-214`): it writes `flux` only, never L/R, so in dual mode the injected observable is overwritten by L+R at the next sync — the pair's flux imprint is transient (one tick), only states/pair_ids persist. *(RECONCILED 2026-07-18, commit `109dc45a`: the pair injector now applies the A1 state-signed δ-split at both sites — the pair is two manifested ±1 particle injections sharing a pair_id, extending the A1 convention to a fourth injection touchpoint. Register split + persistence pinned by `test_dual_substrate` DS-PAIR.)*
3. **`ew_background_sweep` is a dual-mode no-op** (`render_bridge.cpp:615-621`): it adds to `flux` only; dual phase_read reads L/R (never flux) and phase_write overwrites flux := L+R, so the swept background never enters the dual dynamics. (Single-substrate mode is unaffected. The wasm uniform-background injector does it correctly, `bindings_render_bridge.cpp:96-107`.) *(RECONCILED 2026-07-18, commit `826e3609`: symmetric half/half register split matching the wasm injector — D-neutral; pinned by `test_dual_substrate` DS-SWEEP. The sweep remains a CPU-tick-path feature: the GPU dispatch returns before it, a pre-existing backend gap outside this reconciliation.)*
4. **Weak-stress functional CPU/GPU divergence**: CPU `compute_stress_left` applies all three stress terms to J_L; the GPU kernel applies div and curl to J_L but the density-gradient term to the **observable** (`kernels_aux.cu:83-89, 130-152` — its comment declares the mixed convention). Same trigger semantics class, different functional — a parity gap in the weak sector's firing rate. *(RECONCILED 2026-07-18, commit `a8dc026f`: GPU ∇ρ now reads J_L — the CPU/A4 convention of record; WSL2 gpu_golden pin unchanged, GPC suite 81/81.)*
5. **Genesis chirality projection divergence**: CPU projects perpendicular to the local velocity when moving (`voxel.h:93-101`), GPU always uses the legacy z-projection (`kernels_stencil_dual.cu:407-410`). Reconciled in practice because genesis fires at void sites where velocity is zero, selecting the CPU fallback (`voxel.h:103-108`).
6. **GPU movement does not carry L/R** (no `flux_L` reference in `kernels_forces.cu`): the CPU self-field carry (§5.1) has no GPU counterpart, and any observable-flux transport by GPU movement is erased by the next obs := L+R sync — a dual-mode parity gap. *(RECONCILED 2026-07-18, commit `09c028d5`: the movement kernel now mirrors all three CPU branches — proportional L/R carry on move, register zero on boundary removal, 1/6 neighbor scatter on annihilation; WSL2 gpu_golden pin unchanged, parity suites green, annihilation_angular exercises the new paths.)*
7. **Langevin is single-substrate-only** (`phase_write.cpp:243-262`, in the non-dual branch): in dual mode the thermostat toggle leaves the L/R fields untouched — the dual path has no stochastic field term.
8. **Dual genesis does not drain**: the single-substrate genesis consumes wave energy and flux at manifestation (`phase_write.cpp:348-353`); the dual branch manifests without any drain (`phase_write.cpp:326-337`) — a mode divergence in the manifestation energetics.
9. **Pair production is chirality-neutral**: it writes states only (no δ-split, no field mutation; `kernels_aux.cu:196-`), so pair-produced particles carry no D imprint, unlike A1-injected ones.
10. **Stale toggle note**: the validation string "triad_binding requires dual_substrate (operates on J_L/J_R)" (`term_toggles.h:316-317`) is drift — `triad_binding_cpu` (`transmutation_phases.cpp:137-176`) is purely geometric (distances + same-sign states) and never reads any flux field. *(RECONCILED 2026-07-18, commit `6dd46a2a`: message reworded — the requirement itself is retained as declared; no validation behavior change.)*

## §6. The chirality identity [THEOREM]

The engine's chirality density (`voxel.h:92-109`) is, in (F, D) coordinates, exactly

  χ = \|(J_L)_⊥\|² − \|(J_R)_⊥\|² = **F_⊥ · D_⊥**,

the dot product of the transverse projections (perpendicular to the local velocity when moving; to ẑ in the legacy form). *Proof:* \|(F±D)/2\|² expansion; the quadratic terms cancel, the cross term survives. Verified to 3.2×10⁻¹⁵ (T3a/T3b). Immediate consequences: χ is bilinear and odd in each of F, D; where D = 0 (all symmetric injections) χ ≡ 0 and the genesis convention `χ ≥ 0 → +1` (`phase_write.cpp:75`) deterministically assigns +1; the weak swap D ↦ −D flips χ's sign at the site, consistent with its simultaneous state flip.

## §7. Adjudication of the conjecture

**Verdict: alternative (a) of the charter holds.** (F, D) is an exact re-encoding of (J_L, J_R):

1. **[THEOREM]** In exact arithmetic the two encodings generate identical trajectories, with F obeying the single-substrate dynamics (§3) and D a source-free identical-operator field (§4), coupled *only* at the declared touchpoints A1–A6 — no dynamical phase mixes L/R asymmetrically beyond that list (census §5).
2. **[FACT]** As floating-point programs the encodings are *not* bit-identical — L₁₈(J_L) + L₁₈(J_R) rounds differently from L₁₈(F) — but agree to rounding: max relative deviation 1.1×10⁻¹⁵ over 50 ticks across all four channels (T1). Internally the engine's observable *is* F bit-exactly (it is computed as L+R each tick), and the (F, D)-form F-program is bit-identical to the single-substrate branch (T2). The conjecture's "bit-identically" is therefore correct at the level it can be correct at: identity of the update maps, not identity of rounding schedules.
3. **Sharpened semantics of the [IMPOSED] dual substrate (no tag change):** the dual substrate ≡ *the single-substrate engine, plus one hidden source-free vector-field pair (D, V_D) under the same operator, plus the six declared chirality touchpoints of §5.2.* Its entire physical content beyond one field is the touchpoint list: δ-split injection (write), L-projected weak trigger (read), D-negating weak action, and the χ = F_⊥·D_⊥ genesis polarity (read). This restates — it does not re-price — the import; the [IMPOSED] booking and the corpus-level provenance (DSP-3/DSP-4) stand unchanged.

## §8. The checkerboard / corner-register form

### §8.1 The conjugation computation [THEOREM]

The checkerboard weighting ε(x) = (−1)^{x+y+z} acts on the 18-point operator by conjugation as the half-period twist k ↦ k+π (quarantine T1, diagonal on cosine monomials): axis terms (odd ‖v‖₁ = 1) flip sign, face-diagonal terms (even ‖v‖₁ = 2) are invariant, so

  ε L₁₈ ε = −(1/3)Σ_axis + (1/6)Σ_diag − 4I, symbol −4σ₁₈(k+π), σ₁₈(k+π) = 1 + e₁/6 − e₂/6 ≠ σ₁₈(k).

Verified bit-exactly (T4a), with plane-wave eigenvalues matching −4σ₁₈(k) and −4σ₁₈(k+π) to 1.4×10⁻¹³ (T4c/T4d) and an O(1) non-invariance witness (T4b: max deviation 5.3 on a random field; generic symbol gap ≥ 1.9×10⁻²).

### §8.2 Strong form refuted [THEOREM]

The strong register reading — "define Φ = F + εD; then Φ evolves under the single 18-point dynamics, with the chirality field as its corner-register (k+π) component" — **fails**: G = εD satisfies the *conjugated* equation, not the engine equation. One leapfrog step of G under the plain L₁₈ deviates from ε·(one step of D) at O(1) (T5 witness 1.7×10⁻¹, on 0.1-scale fields). Because σ₁₈ is not invariant under the twist, no single field carrying F in the k-register and D in the (k+π)-register can reproduce the dual dynamics.

### §8.3 What survives, honestly [CONJECTURE — semantics only]

The map D ↦ εD is a spectral relabeling k ↦ k+π (an exact unitary on the lattice torus). One may therefore *book* D's content in the corner register as a kinematic labeling convention — but nothing dynamical distinguishes that register: D evolves under the same σ₁₈ as F, and the corner-register/BCC association carries no operator content. "Chirality lives in the corner register" is licensed only in this labeling sense. Any dynamics-compatible identification would require a checkerboard-invariant evolution operator, which the 18-point stencil is not.

A parity trichotomy of the engine's realized stencils sharpens the picture [THEOREM, verified T4f/T4g]: the FCC/edge stencil (cuboctahedron, pure e₂) **commutes** with ε; the 18-point operator (e₁ + e₂) is **neither** invariant nor anti-invariant; the BCC corner average (stella octangula, pure e₃) **anticommutes**. The engine's auxiliary weak-field stencil is thus exactly checkerboard-invariant, while the corner register is dynamically distinguished only by the auxiliary strong-field stencil — which is what triggers §9.

## §9. Interaction with the degree quarantine — T3 amendment [FACT, flagged loudly]

The census turned up what the quarantine's charter demanded be flagged: **the engine's realized operation set does contain an odd-shift-triple composition** — the GPU strong-field stencil `strong_field_stencil_kernel` (`kernels_stencil_dual.cu:176-240`) propagates the *auxiliary* field `flux_strong` on the 8 stella-octangula vertex neighbors (x±1, y±1, z±1), whose symbol is the corner monomial: the operator is (1/8)Σ_corners − I with symbol e₃ − 1. Scope facts: GPU-only (no CPU implementation, `term_toggles.h:203`), gated on `color_forces`/`strong_force` (both default OFF, `gpu_engine.cu:381-384`), acting on `flux_strong` only — never composed with `flux`/`flux_L`/`flux_R`, whose dynamics remain inside the degree-≤2 closure (§4). Its output reaches the primary sector only through the nonlinear particle channel (forces → movement → state → F-source), never as an operator on the wave fields. Additionally, particle movement can transport field content along a *corner displacement* (dx, dy, dz all nonzero, `phase_movement.cpp:130-148`) — a state-conditional, site-local transfer with no translation-invariant symbol, hence outside the operator-algebra scope of T1/T2.

Accordingly, [`LEMMA_DEGREE_QUARANTINE.md`](LEMMA_DEGREE_QUARANTINE.md) §3 (T3) is amended in the same change as this note: the inventory sentence "No phase composes an odd triple of distinct axis shifts" is scoped to the **primary-substrate wave sector**, with the strong-field stencil booked as a *declared, toggle-gated corner-sector operator on an auxiliary field* (an [IMPOSED] phenomenological extension, per the engine's design policy) and the movement carry noted as symbol-less. T1/T2 are untouched; the firewall corollary holds at its stated scope (the primary substrate's operator algebra), now stated explicitly rather than implicitly.

## §10. Falsifiers and scope

- §1–§4 and §6 are exact algebra over the cited code: exhibiting any dual-substrate code path where L and R receive *different* linear operators or *different* coefficients outside §5.2 refutes the decoupling claim as stated (and would itself be the discovery — alternative (b) of the charter).
- The census (§5) is complete relative to the 2026-07-17 tree (52 files, 473 matches); any new `flux_L`/`flux_R` writer or reader added later must be classified into §5.1 or §5.2 or it voids the inventory.
- §8.2 is an exact computation; a checkerboard-invariant replacement for L₁₈ (pure-e₂ dynamics, e.g. the FCC stencil) would *revive* the strong register reading — at the price of changing the engine's wave sector, i.e. a declared adoption, not a reinterpretation.
- Nothing here derives, re-prices, or re-tags the dual substrate, δ, or any spine object; the note's positive content is the equivalence theorem plus the touchpoint census.

## §11. Status of claims

| ID | Claim | Tag |
|---|---|---|
| DSR-1 | (F, D) = (J_L+J_R, J_L−J_R) is an exact re-encoding: identical trajectories in exact arithmetic; touchpoints A1–A6 are the complete asymmetric surface | [THEOREM] (§1–§2, §5; verifier T1) |
| DSR-2 | The F-sector is the single-substrate dynamics, bit-exactly as a program | [THEOREM] (§3; verifier T2, deviation 0.0) |
| DSR-3 | D is source-free under the identical σ₁₈ operator wherever dynamics is L/R-symmetric; matter never sources D | [THEOREM] (§4) |
| DSR-4 | χ = F_⊥·D_⊥ | [THEOREM] (§6; T3a/b) |
| DSR-5 | Touchpoint census: 6 asymmetric touchpoints, 10 register quirks/parity divergences | [FACT — inventory 2026-07-17] (§5) |
| DSR-6 | The two encodings are not bit-identical as FP programs; agree to 1.1×10⁻¹⁵ rel over 50 ticks | [FACT — measured] (§7; T1) |
| DSR-7 | ε L₁₈ ε has symbol −4σ₁₈(k+π) ≠ −4σ₁₈(k); strong corner-register reading fails | [THEOREM] (§8.1–8.2; T4/T5) |
| DSR-8 | D ↦ εD as a corner-register booking is a kinematic relabeling only | [CONJECTURE — semantics, no dynamical content] (§8.3) |
| DSR-9 | Stencil parity trichotomy: FCC commutes, 18-pt neither, BCC corner average anticommutes with ε | [THEOREM] (§8.3; T4f/g) |
| DSR-10 | GPU strong-field stencil is a realized odd-shift-triple (e₃) operator on the auxiliary `flux_strong`; primary substrate stays inside the degree-≤2 closure | [FACT — inventory; triggers the T3 amendment] (§9) |

## §12. Cross-references

- [`LEMMA_DEGREE_QUARANTINE.md`](LEMMA_DEGREE_QUARANTINE.md) — the operator-algebra frame (σ₁₈, twist action, e₃ quarantine); its T3 amended per §9.
- [`AUDIT_DUAL_SUBSTRATE_PROVENANCE.md`](../07_assessment/spine_gstar_cm_modular/AUDIT_DUAL_SUBSTRATE_PROVENANCE.md) — corpus-level provenance of (E_L, E_R, δ); DSP-3/DSP-4 (Vieta dressing) unchanged by this note.
- [`AUDIT_LINK8_CLOSURE.md`](../10_eft_program/archive/closed_negative/AUDIT_LINK8_CLOSURE.md) — the engine-stencil ≠ BCC finding this note's §4/§8 reinforce.
- `engine/tests/campaign_parity_violation.cpp` — declared physics reading of the A4 touchpoint ([IMPOSED] SU(2) L-coupling; V–A from δ → 1).
- `engine/tests/test_dual_substrate.cpp`, `test_substrate_angle_probe.cpp`, `test_open5_legacy_flux_l.cpp` — the existing test surface over the register.
- `scripts/proofs/proof_dual_substrate_staggered_encoding.py` — verifier for every numbered check cited here (15/15).
