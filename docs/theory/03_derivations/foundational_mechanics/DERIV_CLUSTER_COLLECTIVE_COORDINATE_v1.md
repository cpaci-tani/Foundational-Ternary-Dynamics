# DERIV — Cluster inertia N·M_REST from the collective coordinate: v1 attempt

**LEDGER row:** FTD-0349 (row written by the controller; this document does not edit the LEDGER)
**Date:** 2026-07-01
**Tag:** **[PARTIAL — obstruction named]** — the reduction is **[DERIVED conditional on GNC]** (the Gradient-Normalization Condition, §3), and GNC **fails for both flux profiles the framework currently pins down** (§4, §5) while GNC-satisfying textures **exist** (§7), so the obstruction is dynamical, not kinematic. FTD-0110 and FTD-0250 tags are **unchanged** by this document.
**Verification:** [`scripts/proofs/proof_cluster_collective_coordinate.py`](../../../../scripts/proofs/proof_cluster_collective_coordinate.py) — **28/28 PASS** (run of record 2026-07-01; every algebraic identity asserted below is numerically verified there, test ids T1–T8 cited inline).
**Read first:** [FTD-0250 / FTD-0110 LEDGER rows](../../07_assessment/core_ledgers/LEDGER.md); [`SPEC_FTD_LAGRANGIAN.md`](../../01_reference/SPEC_FTD_LAGRANGIAN.md) §3–§4; [`DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`](DERIV_K_FROM_OH_A1G_MULTIPLICITY.md); [`EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md`](EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md); [`engine/tests/test_cluster_inertia.cpp`](../../../../engine/tests/test_cluster_inertia.cpp); FTD-0277 [CLOSED NEGATIVE] and [`AUDIT_FTD0110_2026-05-27_RESOLUTION.md`](../../07_assessment/audits/AUDIT_FTD0110_2026-05-27_RESOLUTION.md).

---

## §0 · Verdict

> **PARTIAL.** Rigid translation of a locked, Gauss-dressed N-voxel cluster costs co-moving momentum N·M_REST·**v** **if and only if** the dressed flux profile satisfies the Gradient-Normalization Condition
>
> $$\text{GNC:}\qquad \sum_{\mathbf{x}\in\text{supp}(w)} (\Delta^+_i J_a)(\mathbf{x})\,(\Delta^+_j J_a)(\mathbf{x}) \;=\; N\,K_B^2\,\delta_{ij},$$
>
> and **nothing in the FTD action or the Gauss constraint forces GNC**. The two profiles the framework does pin down both fail it — the minimal Coulomb dressing by *coefficient* and *tensor structure* (not, remarkably, by N-scaling: its trace obeys a new exact lattice identity, §4), and the amplitude-pinned uniform core by *N-scaling* (exact surface law ∝ N^{2/3}, §5). GNC-satisfying interior textures exist (θ = 2π/3 rotational textures, §7), so the wall is dynamical: the substrate does not force the texture. GNC is the spatial twin of the clock-hypothesis [AXIOM] (§8). A falsifiable engine measurement that would close or confirm the wall is specified in §9.

Sub-verdicts, tagged:

| # | Claim | Tag | Verified |
|---|-------|-----|----------|
| 1 | Collective kinetic coefficient of the rigid ansatz = the gradient quadratic form 𝕄 (Eq. 2) | [THEOREM] | T3, T4 |
| 2 | Rest sector = N·M_REST exactly (count-weighted), given premise P1 | [THEOREM given P1] | — (bookkeeping) |
| 3 | Engine law 𝕄 = N·M_REST·𝟙 ⟺ GNC; full γ_FTD resummation ⟺ pointwise GNC-s | [THEOREM] | T2, T7 |
| 4 | Engine's ~6 ppm a·N residual = γ_FTD correction with c = C_SPEED = 1/√3 | [VERIFIED] | T1 |
| 5 | Minimal Coulomb dressing: exact trace identity tr 𝕄_dress·K_B = Σρ̃² = Nq²(1−N/L³) | **[THEOREM — new lattice identity]** | T5b, T5e-ii |
| 6 | Coulomb-dressing route to GNC fails (coefficient q²/3 ≠ K_B²; tensor anisotropy for non-cubic clusters; member-weighted form N-drifting) | [CLOSED NEGATIVE — this route] | T5b–T5e |
| 7 | Amplitude-pinned uniform core: exact surface law 6K_B²N^{2/3}; volume law impossible | [CLOSED NEGATIVE — this route] | T6 |
| 8 | GNC-satisfying textures exist: J = K_B R**x**, R ∈ SO(3), tr R = 0 ⟺ θ = 2π/3 | [THEOREM — existence] | T7, T8 |
| 9 | Whether engine cluster profiles realize GNC | **[OPEN]** — the v2 measurement, §9 | — |

---

## §1 · Problem statement and premises

**What the engine imposes (FTD-0250 [IMPOSED]).** `phase_forces_integrate_clusters` (engine/src/render_bridge_phases/phase_forces.cpp) flood-fills a locked, same-sign, 26-connected cluster of N manifested voxels and integrates its centre of mass at inertial mass N·M_REST via the γ_FTD momentum scheme. The a∝1/N falsifier holds to ~6 ppm across N ∈ {1, 8, 27} (`test_cluster_inertia.cpp` CI-1).

**What must be shown.** From the per-voxel Born-Infeld action ([`SPEC_FTD_LAGRANGIAN.md`](../../01_reference/SPEC_FTD_LAGRANGIAN.md) §3.3, §4.1), derive that rigid translation of the dressed cluster at velocity **V** costs co-moving momentum N·M_REST·**V** — i.e. derive the cluster-level inertia from field-level structure rather than imposing it.

**Premises, stated explicitly:**

- **P0 (framework).** The FTD action of SPEC_FTD_LAGRANGIAN §3.3 in the weak-field regime f = 1 (latency off), matching the engine test's Newtonian-clean configuration. [AXIOM-level input]
- **P1 (interpretive, FTD-0250's own reading).** The Born-Infeld core contributes one −K_B per **manifested** voxel; the field kinetic sector (§3.6 terms 5/6) carries weight 1/(2K_B) per site in J-units (equivalently weight 1/2 after canonical normalization Ĵ = J/√K_B, §5.4). [IMPOSED — the spec is not fully explicit about the BI core's site support; both the members-only and all-sites weightings are analyzed below and both readings hit the same wall.]
- **P2 (collective-coordinate ansatz).** Rigid transport: J(**x**, t) = J_cl(**x** − **R**(t)), s(**x**, t) = s_cl(**x** − **R**(t)), with **Ṙ** = **V**, |**V**| ≪ 1, and profile-readjustment fluctuations η set to zero at v1. [Standard collective-coordinate idealization; if J_cl is a constrained energy minimum, η corrections enter the kinetic coefficient beyond leading order. The continuous-velocity idealization matches the engine (voxel velocities are continuous; position hops are Phase-3 [BOUNDARY], per the FTD-0250 row).]

**What this is methodologically (one sentence, per the FTD-0277 hazard):** this is a *kinetic-coefficient* computation — N is an **input** and no genesis counting, static gating, or manifested-site census is performed anywhere; the FTD-0277 [CLOSED NEGATIVE] counting route is not reused. No A_{1g} purity is assumed anywhere (the known falsifier: Gauss projection drives f_A1g 1.0 → 0.15 by tick ~100, [`DERIV_FTD0110_NONLINEAR_BRIDGE.md`](DERIV_FTD0110_NONLINEAR_BRIDGE.md) §5.1); the only symmetry input is exact O_h invariance of symmetric configurations. The [DISPUTED] Orbit-Equipartition rescue is likewise not used.

---

## §2 · Step 1 — the collective kinetic coefficient is a gradient quadratic form [THEOREM]

Under P2, the discrete time difference of the flux at any site is

$$\Delta_t \mathbf{J}(\mathbf{x},t) \;=\; J_{cl}(\mathbf{x}-\mathbf{R}-\mathbf{V}) - J_{cl}(\mathbf{x}-\mathbf{R}) \;=\; -(\mathbf{V}\!\cdot\!\nabla)J_{cl} + O(V^2), \tag{1}$$

and the O(V²) part of the summed kinetic term is exactly a quadratic form in **V** (verified as an exact bilinear identity for the lattice forward-difference form, T3; the remainder of the full displacement sum is **fourth** order in V — the squared-difference sum is even in **V** — verified by the ratio-16 halving test, T4):

$$L_{\text{coll}}(\mathbf{V}) \;=\; -N K_B \;+\; \tfrac12\,\mathbf{V}^{\!\top}\mathbb{M}\,\mathbf{V} + O(V^4), \qquad \mathbb{M}_{ij} \;=\; \frac{1}{K_B}\sum_{\mathbf{x}\in\text{supp}(w)} (\Delta^+_i J_a)(\Delta^+_j J_a). \tag{2}$$

The weight support supp(w) is the members-only set under the BI-sited reading of P1, or all sites under the field-sector reading; both are carried below. The −N·K_B constant is the rest sector: **count-weighted**, exactly N·M_REST given P1. [THEOREM given P1]

The derivation is thereby reduced to one sharp question: **is the inertial (gradient-weighted) coefficient equal to the rest (count-weighted) coefficient?** That equality at the collective level *is* the equivalence of rest and inertial mass FTD-0250 wants as a substrate theorem, and it holds iff

$$\text{GNC-w (summed):}\quad \sum_{\mathbf{x}} (\Delta^+_i J_a)(\Delta^+_j J_a) = N K_B^2\,\delta_{ij}; \qquad \text{GNC-s (pointwise):}\quad \big|(\hat V\!\cdot\!\nabla)J_{cl}(\mathbf{x}_m)\big| = K_B \;\;\forall\, m,\ \forall\,\hat V. \tag{3}$$

GNC-w delivers the Newtonian-limit inertia N·K_B; GNC-s additionally resums the per-member Born-Infeld cores to the engine's exact relativistic form, since v_m = |Δ_t J(x_m)|/K_B = |**V**| at every member gives Σ_m [−K_B√(1−v_m²)] = −N K_B√(1−V²), whose momentum is N·K_B·γ_FTD·**V** — precisely the cluster integrator's algebra (T2). [THEOREM]

**Engine tie-in [VERIFIED].** The γ_FTD model with c = C_SPEED = 1/√3 reproduces the `test_cluster_inertia.cpp` header values a_COM(N ∈ {1,8,27}) to all six printed digits, and its predicted a·N spread across N = 1→27 is 5.74×10⁻⁶ — the test's quoted "~6 ppm" residual is the relativistic correction, not noise (T1).

**Both derivation routes hit the same wall.** The point-mechanics route (each manifested voxel an independent BI carrier with its *own* transport velocity; rigid lock as holonomic constraint; additivity gives P = N K_B γ V trivially, T2) presupposes that a member's flux velocity equals its transport velocity — which under P2 is Eq. (1) evaluated at the member, i.e. GNC-s. The field route computes the coefficient directly and needs GNC-w. GNC is the single load-bearing gap in either formulation. [THEOREM-level reduction of the problem]

---

## §3 · What the engine law requires, restated

With M_REST = K_B (FTD-0130 unified-mass identification), the imposed law a_COM = F/(N·M_REST) is equivalent to 𝕄 = N·M_REST·𝟙, i.e. GNC-w with isotropic tensor structure. Note what GNC asks of the profile: *transporting a member voxel by one lattice site must swing the local flux by exactly K_B, isotropically.* The trace of GNC-w says the profile's total gradient energy is pinned at Σ|∇J|² = 3NK_B² — a volume law with a per-voxel quantum. Whether any framework-pinned profile does this is the content of §4–§6.

---

## §4 · Route A — the minimal Gauss-dressed (Coulomb) profile [CLOSED NEGATIVE for GNC, with a new exact identity]

The engine's `gauss_project` is an L²-projection of J onto the constraint surface ∇⁻·J = ρ; for a configuration whose flux is entirely constraint-generated, the profile is the minimal dressing J_C = −Δ⁺φ with (Δ⁻·Δ⁺)φ = −ρ̃ (lattice Poisson; solver validated to machine precision against the lattice Gauss law, T5a). This treats the Gauss non-locality *exactly*: under transport the projection slaves the dressing to the source position, the dressing co-moves, and eliminating the slaved variables adds their full gradient quadratic form to 𝕄 — the dressing's inertia is finite and computed below, not estimated. (Classical precedent: the electromagnetic-mass/4⁄3 problem — the co-moving momentum of a Coulomb field is a bookkeeping-sensitive quantity that need not match naive energy accounting. Here it is exactly computable.)

**Lemma (dressing trace identity) [THEOREM — new].** On the periodic lattice (ℤ/L)³, let ρ̃ have zero mean, φ solve the 6-point lattice Poisson equation, J_C = −Δ⁺φ. Then

$$\sum_{\mathbf{x}}\sum_{i,a}\big(\Delta^+_i J_{C,a}\big)^2 \;=\; \sum_{\mathbf{x}} \tilde\rho(\mathbf{x})^2. \tag{4}$$

*Proof.* Fourier: Δ⁺_i has symbol s_i = e^{ik_i}−1 and the Laplacian symbol is −λ, λ = Σ_i|s_i|². Then Δ⁺_i J_{C,a} has symbol s_i s_a φ̂ (up to sign), so Σ_{i,a}|s_i s_a φ̂|² = λ²|φ̂|² = |ρ̂̃|² since φ̂ = ρ̂̃/λ. Parseval on both sides. ∎

Verified to 6×10⁻¹⁴ relative for solid cubes N ∈ {1, 8, 27, 64, 125} and for an 8×1×1 rod, including the exact jellium factor: tr = Nq²(1 − N/L³) (T5b-i, T5e-ii). It is the lattice analog of ∫|∇∇φ|² = ∫ρ², and its structural content is striking: **the all-site dressing inertia trace is exactly N-proportional, shape-independent, and sign-blind** (ρ² counts +1 and −1 charges identically — a mass-like, not charge-like, count). Provenance note: the pre-computation expectation was an N·ln N cross-term growth; the computation refuted it — the Hessians of the lattice Green's function at distinct sources are exactly L²-orthogonal because λ²·(1/λ)² = 1. The expectation is reported here because the honest surprise is part of the result.

**Why the route still fails GNC:**

1. **Coefficient.** Per axis, the all-site dressing coefficient is q²/3, not K_B² (T5b-ii). Equality would need q = √3·K_B; nothing in the framework sets the ternary unit charge to that value, and since K_B = 0.511 is the FTD-0041 MeV-anchored calibration number, the numerical proximity of q²/(3K_B²) = 1.28 to 1 at q = 1 is calibration-relative and is **not** to be read as structure. [Anti-target discipline applied.]
2. **Tensor structure.** Only the *trace* is universal. The components are shape-dependent: an 8×1×1 rod has M_xx vs M_yy anisotropic by 87% while its trace still obeys Eq. (4) exactly (T5e). The imposed law is an isotropic scalar for every cluster shape; the dressing cannot supply that. (For O_h-symmetric clusters the symmetrized site-centered Hessian tensor is isotropic to machine precision, T5c — the raw forward-difference estimator carries ~10% staggering artifacts, a discretization note, not physics.)
3. **Member weighting.** Under the BI-sited (members-only) reading of P1, the exact identity is lost: the member-summed form is 0.20 → 0.46 × NK_B² and drifts with N by 2.3× across N = 1→125, scaling exactly as q² with no K_B-pinning mechanism (T5d, table below).

| N (solid cube, q = 1) | 1 | 8 | 27 | 64 | 125 |
|---|---|---|---|---|---|
| all-site tr𝕄·K_B/(3N) per axis | 0.33333 | 0.33331 | 0.33325 | 0.33314 | 0.33296 |
| member-sum / (N K_B²) per axis | 0.1996 | 0.3923 | 0.4327 | 0.4519 | 0.4637 |

(The all-site row's drift is *exactly* the jellium factor 1 − N/L³ at L = 48 — the identity is exact.)

---

## §5 · Route B — the amplitude-pinned uniform core [CLOSED NEGATIVE]

The naive reading of manifestation pinning — interior flux uniform at threshold amplitude, J = K_B x̂ inside a solid cube of edge e, zero outside — gives an **exact surface law** (T6): all-site gradient sum = 6K_B²e², member-sited part = 3K_B²e², exact one-hop difference sum = 2K_B²e². All three accountings scale as e² = N^{2/3}; the required volume law 3NK_B² is missed by the factor 2N^{−1/3} → 0. **A uniform-core cluster is asymptotically massless under rigid translation relative to the imposed law**: the field-level kinetic cost of transporting a constant interior is paid only at the surface, because the flux time-derivative vanishes wherever the profile has no gradient. This holds for any transport microdynamics (the exact one-hop sum is also surface-scaling), so it is robust to the continuous-velocity idealization of P2. It also flags a tension worth recording: a strictly uniform pinned core is not even Gauss-consistent with per-member point sources (its divergence is concentrated on faces), so the physical profile must carry interior gradient structure — which is exactly what GNC quantifies and the framework does not pin.

---

## §6 · The Gauss non-locality, explicitly dispatched

The known killer for A_{1g}-based arguments (projection-driven purity decay) does not bite here because no irrep decomposition is used; the projection enters only as the slaving of the dressing to the source, and §4 accounts for the slaved variables' inertia *exactly* via Eq. (4). What survives of the "killer" is the honest accounting it forces: the dressing contributes a finite, N-proportional, **anisotropic-in-general, q²-weighted** term to 𝕄 that the engine's imposed N·M_REST omits entirely. At engine-typical charge normalization this term is O(q²/K_B) per member against the imposed K_B per member — numerically comparable at q = 1 in lattice-charge units (see the calibration caveat in §4.1) — so the imposed law and the field-level accounting are *not* reconciled even approximately for non-cubic clusters, and the discrepancy is measurable in principle (§9).

---

## §7 · GNC textures exist — the obstruction is dynamical, not kinematic [THEOREM — existence]

Let J(**x**) = K_B R **x** with R ∈ SO(3). Forward differences of a linear field are exact, so (V·∇)J = K_B R**V** and every site satisfies GNC-s isotropically: |(V̂·∇)J| = K_B for all directions (orthogonality of R). The lattice divergence is K_B·tr R (T8b), and tr R(θ) = 1 + 2cos θ (T8a), so the texture is charge-free iff θ = 2π/3 exactly (T8c) — realized e.g. by the cyclic coordinate permutation (the 2π/3 rotation about (1,1,1)), verified divergence-free with per-member kinetic coefficient exactly K_B²|V|² and member-summed coefficient exactly N·K_B²|V|² (T7a–T7d; random-axis Rodrigues rotations at θ = 2π/3 likewise, T7e).

So a locked cluster whose interior carries a θ = 2π/3 rotational flux texture of slope K_B, with the Gauss charge carried by a separate dressing, satisfies GNC-s exactly and its Born-Infeld cores resum to the engine's −NK_B√(1−V²). **Existence means the reduction's failure is not a no-go**: the wall is that nothing in the substrate dynamics is known to *force* member-site gradient pinning at K_B — not that no profile could realize it. Whether genesis + locking + Gauss projection dynamically produce gradient-pinned textures is **[OPEN]** and is exactly the v2 measurement (§9). No physical identification of the 2π/3 angle is made or implied here; it enters solely as the trace-zero condition on SO(3). [Anti-target discipline applied.]

---

## §8 · Relation to the known walls

- **Clock hypothesis (temporal twin).** The clock hypothesis — an internal per-voxel flux swing at rate K_B per tick — is an independent, non-derivable [AXIOM] ([`SPEC_FTD_LAGRANGIAN.md`](../../01_reference/SPEC_FTD_LAGRANGIAN.md) §4.3; [`AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md`](../archive/AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md)). GNC is its **spatial twin**: a per-voxel flux swing of K_B per site of transport. Both are per-voxel internal scales the substrate does not force; together they are the two legs of a Lorentz-covariant rest-mass structure (temporal swing = rest energy; spatial swing = inertia). That the v1 reduction terminates on the spatial leg of the same wall the proper-time derivation terminates on (temporal leg) is evidence the wall is one wall, and is the honest structural finding of this attempt. [SYNTHESIS-level observation; no promotion implied.]
- **FTD-0309 (angular obstruction).** The v2 genesis-counting attempt found the scalar collective-coordinate reduction of N(A) "irreducibly angular". The same signature recurs here in the inertia problem: the scalar (trace) channel of the dressing behaves cleanly (exact identity, right N-scaling), while the failure is concentrated in the angular/tensor channel (87% shape anisotropy). [Observation]
- **FTD-0277.** Not reused: no counting, no static gating; N is an input (§1).

---

## §9 · Scope, and what a v2 needs

**Scope of every positive claim here:** weak field (f = 1, latency off); O(V²) linear response (inertia), O(V⁴) remainder verified for smooth profiles; rigid ansatz (no profile readjustment, no radiation/retardation); continuous-velocity idealization matching the engine integrator; periodic-lattice Poisson with jellium for the dressing computations (L = 48; jellium corrections tracked exactly).

**The v2 discriminator (falsifiable, bounded):** measure, on actual engine clusters (post-genesis, locked, Gauss-projected, thermalized; N ∈ {8, 27, 64}, multiple seeds), the member-site gradient quadratic form

$$Q_{ij} \;=\; \frac{1}{N K_B^2} \sum_{m} (\Delta^+_i J_a)(\mathbf{x}_m)\,(\Delta^+_j J_a)(\mathbf{x}_m).$$

- If Q_ij ≈ δ_ij within pre-registered gates: GNC is engine-real, and this document's conditional chain upgrades FTD-0250 to [DERIVED given engine-verified GNC] with the equivalence principle a substrate theorem at that scope.
- If Q ≪ 𝟙 or N-drifting (the §4/§5 profiles predict exactly this): the wall is confirmed, FTD-0250 stays [IMPOSED], and GNC joins the clock hypothesis as a named, boundary-mapped imported type (modulus/argument-frontier bookkeeping: the count N is substrate-native; the per-count inertia scale is imported).

This is a measurement of a defined quantity with gates fixed in advance — pre-register before running (engine campaign; per project discipline the CUDA/WSL2 engine is the canonical instrument, Python is quick-check). Secondary v2 questions, flagged [OPEN] and not pursued here: whether any framework-native charge normalization sets the dressing coefficient q²/3 → K_B²; whether genesis + locking dynamics can produce θ = 2π/3 gradient-pinned textures; the mixed-sign-cluster inertia question raised by the sign-blindness of Eq. (4) (the engine's cluster definition is same-sign, so the current comparison point does not probe it).

---

## §10 · Verification artifacts

[`scripts/proofs/proof_cluster_collective_coordinate.py`](../../../../scripts/proofs/proof_cluster_collective_coordinate.py) — 28/28 PASS. T1 engine-residual model (6-digit header match + ~6 ppm spread); T2 additivity-route algebra; T3 exact kinetic bilinear identity; T4 fourth-order remainder (ratio 15.96 ∈ [12, 20]); T5 Coulomb dressing (Gauss law to 10⁻¹⁶; trace identity to 6×10⁻¹⁴ incl. jellium factor and rod shape-independence; symmetrized-Hessian isotropy to 10⁻¹⁶; exact q² member scaling; member-ratio drift 2.3×; rod anisotropy 0.87); T6 exact surface laws (three accountings); T7 GNC-texture existence (machine precision); T8 trace lemma and the θ = 2π/3 forcing.

*No engine campaigns were run for this document; the engine numbers cited are from the committed `test_cluster_inertia.cpp` header and the FTD-0250 LEDGER row.*
