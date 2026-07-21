# LEMMA: The Degree Quarantine — Why the Engine's Dynamics Cannot Reach the BCC Sector

**Status:** [THEOREM — lattice operator algebra] for T1/T2 (verified `scripts/proofs/proof_degree_quarantine.py`, 6/6); [FACT — engine inventory] for T3 (amended 2026-07-17 — scoped to the primary substrate; see §3); the firewall corollary is the interpretive payoff and inherits the weakest tag of its clauses. Provisional AI-derived content, not externally reviewed; provenance: the 2026-07-17 tri-lattice re-intuition (savant synthesis) sharpened under adversarial self-check — the naive "dynamics can't reach degree 3" claim has a composition loophole, closed here by restricting to the engine's realized operation set.
**Zero promotions:** nothing here touches x₊ = 1/α [SMC], MC-T4.3, or Link-8's [CLOSED NEGATIVE] — the corollary *reinforces* the last. No sector-ratio construction of any spine constant appears or is licensed.

---

## §0. Setting

Axis cosines c_i = cos k_i on the Brillouin torus; elementary symmetric polynomials e₁ = c₁+c₂+c₃ (SC/face symbol), e₂ = Σ_{i<j} c_i c_j (FCC/edge symbol), e₃ = c₁c₂c₃ (BCC/corner symbol). The engine's dynamical Laplacian symbol is the degree-≤2 object −4σ₁₈ with σ₁₈(k) = 1 − e₁/6 − e₂/6 (AUDIT_LINK8_CLOSURE §"Engine's stencil is NOT BCC"); the corner weight is exactly 0. "Multidegree" of a cosine monomial ∏_{i∈S} c_i is its support S ⊆ {1,2,3}.

## §1. T1 — Twisted-linear closure [THEOREM]

**Claim.** Half-period translations k → k + πv, v ∈ {0,1}³ (including the checkerboard conjugation v = (1,1,1), i.e. multiplication by (−1)^{x+y+z} in position space) act on the cosines by per-axis sign flips c_i → (−1)^{v_i} c_i, hence act **diagonally** on the monomial basis and preserve every monomial's multidegree. Consequently the linear span of *all* half-period twists of the engine family {1, e₁, e₂} is exactly the 7-dimensional space span{1, c₁, c₂, c₃, c₁c₂, c₁c₃, c₂c₃}, and **e₃ is not in it**.

**Proof.** cos(k_i + πv_i) = (−1)^{v_i} cos k_i; a diagonal action cannot populate a basis coordinate (the (1,2,3)-multidegree monomial) that is zero in every generator. Constructive verification: the 24-element twist orbit has rank exactly 7, its e₃-coordinate is identically zero, and adjoining e₃ raises the rank to 8 (proof script, T1a–c). ∎

Note the twisted span is *maximal* below the corner: every individual monomial c_i and c_ic_j is reachable (rank 7 of 7), so the quarantine is precisely and only of the degree-3 coordinate.

## §2. T2 — Spectral-functional blindness [THEOREM, exact witness]

**Claim.** Every object built as a function of the dynamical operator — dispersion, propagator, heat kernel, any F(L₁₈), hence every linear-response and Green's-function observable of the wave sector — has symbol F(σ₁₈(k)). The coordinate e₃ is **not a function of σ₁₈**, so no such object measures, encodes, or generates it.

**Proof.** Exact witness on one level set: the cosine triples P = (½, −½, ¼) and Q = (0, 0, 0) both satisfy e₁ + e₂ = 0, hence σ₁₈ = 1 identically, while e₃(P) = −1/16 ≠ 0 = e₃(Q). Both triples are realized by real k-points (cos is onto [−1,1]). A function constant on σ₁₈-level sets cannot equal e₃. (Script T2a–c; the witness family is one-parameter: c = (t, −t, t²) gives σ₁₈ = 1, e₃ = −t⁴.) ∎

## §3. T3 — The composition loophole, and why the engine does not use it [FACT — engine inventory]

**The loophole, stated honestly:** the full shift-generated operator algebra trivially contains the corner shift — T_x T_y T_z *is* a BCC translation, and symbol products reach e₃ (e.g. c₁ · c₂c₃ = e₃). Any claim that "no operation whatsoever" reaches the corner sector would be false.

**The inventory:** the engine's realized per-tick operation set is (i) leapfrog applications of the single fixed operator L₁₈ — everything dynamical is F(L₁₈)-type, covered by T2; (ii) the Gauss projector — central-difference div/grad (odd sin-symbols) composed through the 18-pt solve: every symbol involved is built from {sin²k_i sums, σ₁₈}, all even, degree-≤2 objects, covered by T1/T2's span; (iii) scalar per-site operations (damping, thresholds) — symbol-neutral; (iv) half-period/parity structure — covered by T1. Within this primary-substrate operation set (`flux`, `flux_L`/`flux_R` and their wave velocities), no phase composes an odd triple of distinct axis shifts. (Callstack of record: phase_read/phase_write/poisson_solvers; cf. AUDIT_ENGINE_CALLSTACK.)

**Amendment of record (2026-07-17, staggered re-encoding census — [`EXPLR_DUAL_SUBSTRATE_STAGGERED_ENCODING.md`](EXPLR_DUAL_SUBSTRATE_STAGGERED_ENCODING.md) §9).** The full-code census found exactly two items the original inventory sentence did not name, neither of which enters the primary substrate's operator algebra: (v) the GPU-only strong-field stencil `strong_field_stencil_kernel` (`engine/cuda/kernels_stencil_dual.cu:176-240`) — a realized corner-shift operator (8 stella-octangula vertex neighbors; symbol e₃ − 1) acting on the auxiliary field `flux_strong`, gated on `color_forces`/`strong_force` (both default OFF) with no CPU implementation: a *declared, toggle-gated corner-sector operator on an auxiliary field*, reaching the primary sector only through the nonlinear particle channel (forces → movement → state → source), never as an operator composition on the wave fields; (vi) particle movement's conditional self-field carry, which can transport field content along a corner displacement (`engine/src/render_bridge_phases/phase_movement.cpp:130-148`) — state-conditional and site-local, hence carrying no translation-invariant symbol and outside the T1/T2 operator algebra. T1/T2 are unaffected; the corollary below holds at its stated scope, now explicit: the primary wave-sector substrate. The dual-substrate difference field D = flux_L − flux_R evolves under the same degree-≤2 operator and adds no new symbol content (op. cit. §4).

## §4. Corollary — the firewall

Within the engine's realized operation set, **the BCC/e₃ coordinate is unreachable and unmeasurable**: no tuning, blocking, twisting, iteration, or constraint enforcement available to the dynamics could have adjusted — or even detected — the corner-sector structures. The spine's BCC-resident objects (W₃ = G*²/2π, the master-quadratic coefficients) are therefore *dynamically incorruptible* within this closure: the rigidity evidence for them (FTD-0319) cannot be an artifact of anything the machine does. This is Link-8's [CLOSED NEGATIVE] restated as a positive structural property and strengthened from "measured: no emergence" toward "provable: no channel," at the stated scope (T3's inventory, not arbitrary operator products).

**Semantic note on FC-W [CONJECTURE — gloss, moves nothing]:** under this lemma, "purchasing a corner-sector voice" acquires an operational meaning — adopting any dynamics term that composes an odd axis-shift triple (equivalently: any symbol with nonzero e₃ component) would breach the quarantine by declared act. That an *adoption* is the only way in is the content of the firewall; the price and the tags of any such adoption are governed by the standing pipeline (FC-W/P6C, FTD-0387 pending ratification), and nothing here changes them.

## §5. Falsifiers / scope

- T1/T2 are exact algebra; a counterexample to either (an orbit element with e₃ component; F with F(σ₁₈) = e₃) refutes the lemma outright.
- T3 is an inventory claim: exhibiting an engine code path whose symbol carries nonzero e₃ component (grep-level check: any composed odd shift-triple in a phase) voids the corollary's scope and must be booked as either a bug or an undeclared adoption. The 2026-07-17 census executed exactly this check and found one such path — the GPU strong-field stencil on the auxiliary `flux_strong` field — booked in §3's amendment as a declared, toggle-gated operator outside the primary substrate; the primary-substrate claim survives with its scope now explicit.
- The corollary's protective value is conditional on the engine remaining within the T3 closure; the staggered re-encoding investigation (delivered: [`EXPLR_DUAL_SUBSTRATE_STAGGERED_ENCODING.md`](EXPLR_DUAL_SUBSTRATE_STAGGERED_ENCODING.md) — the dual substrate is a sum/difference register whose difference field lives under the same degree-≤2 operator, so it cannot reach the corner sector) and any future corner-coupling proposal interact with this lemma by design, not by accident.
