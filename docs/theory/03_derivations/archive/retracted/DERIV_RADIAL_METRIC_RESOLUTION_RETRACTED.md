# The Discrete Radial Metric Projection Theorem: Deriving the Schwarzschild $g_{rr}$ Component in FTD

**Status:** **[RETRACTED 2026-07-02 — invalid proof; FTD-0360/FTD-0361, independent convergent passes]** (was: [THEOREM])

> **RETRACTION NOTICE (2026-07-02, per the FTD-0361 cluster review — the follow-up pass
> the FTD-0356 review left queued for the two unadjudicated sibling `[THEOREM]` rows of
> `MONOGRAPH_EFFECTIVE_EQUATIONS.md` §7.1; the same retraction was performed
> independently the same day by the FTD-0360 finalization batch, with a convergent
> defect analysis — reconciled at the 2026-07-03 merge).** This document's central proof
> is invalid: its conclusion contradicts its own premise, and its claim contradicts the
> corpus's canonical status rows.
>
> 1. **The conclusion contradicts the premise.** The premise (§1, imported from
>    `DERIV_RELATIVITY_DERIVATION.md`) is a radial coordinate wave speed
>    `C(r) = C₀√(1 − r_s/r)`. But the concluded metric — `g₀₀ = −(1 − r_s/r)` together
>    with `g_rr = (1 − r_s/r)⁻¹` — has radial null-cone speed
>    `dr/dt = C₀·(1 − r_s/r)`, not `C₀√(1 − r_s/r)`. At `r = 2r_s` the premise says
>    `0.707 C₀` while the conclusion's metric says `0.5 C₀` (verified symbolically and
>    numerically under FTD-0361). A derivation whose output metric is inconsistent with
>    its own input propagation law is not a proof.
> 2. **The single contraction factor is spent twice.** The one physical input — waves
>    slow by `√(1 − r_s/r)` — was already fully booked as a *clock-rate* effect to
>    obtain `g₀₀` (`DERIV_RELATIVITY_DERIVATION.md` §11.1, where
>    `f_sat = √(1 − r_s/r)` is `dτ/dt`, "time runs slower where flux is concentrated").
>    Step 3's `dl = a = C₀Δt` books the same factor a second time as pure *ruler*
>    stretching, by silently using undilated coordinate ticks as local proper time.
>    Accounted consistently — `g₀₀` fixed, plus this document's own §2.2 local
>    light-speed invariance — the premise forces `g_rr = 1` (spatially flat):
>    `dl/dτ = √g_rr · C(r)/√(1 − r_s/r) = C₀ ⟹ g_rr = 1`. Conversely, holding both of
>    the document's claims simultaneously makes the locally measured light speed
>    `C₀/√(1 − r_s/r) ≠ C₀`, violating its own §2.2 invariance premise.
> 3. **It proves too much.** The argument never invokes the vacuum field equations, so
>    if valid it would force `g₀₀·g_rr = −1` in *any* static spherically symmetric
>    region — including matter interiors, where GR gives
>    `g_rr = (1 − 2G m(r)/r)⁻¹ ≠ −1/g₀₀` (TOV interior solutions). The product relation
>    `g₀₀·g_rr = −1` is a special property of the Schwarzschild *vacuum*, enforced by
>    `R_μν = 0`; no purely kinematic ruler argument can be its source.
> 4. **The premise is the classic half-deflection medium.** An isotropic index
>    `n = C₀/C(r) = (1 − r_s/r)^{−1/2} ≈ 1 + r_s/2r` yields light deflection `r_s/b` —
>    half the GR value `2r_s/b` (verified by direct quadrature under FTD-0361; this is
>    Einstein's 1911-vs-1915 factor of 2). The missing half *is* the `g_rr` spatial
>    curvature; a medium encoding only time dilation cannot also source it.
> 5. **Canon.** `DERIV_RELATIVITY_DERIVATION.md`'s own Gap 10.1 ("the spatial metric
>    components `g_ij` are NOT derived from FTD first principles"; the
>    consistency-demand approach flagged *circular*) and Gap 11.1 (full Schwarzschild
>    NOT derived) were never closed by this document — and FTD-0189 (Step-0
>    graviton-provenance audit) holds the metric perturbation *posited* (Conjecture
>    10.1) with its spin-2 spatial part underived (Gap 10.1) and Frontier 4 `[OPEN]`.
>    A kinematic `[THEOREM]` delivering the full Schwarzschild spatial metric would
>    leapfrog all three rows; LEDGER > prose.
> 6. **Substrate ontology (recorded by the convergent FTD-0360 pass).** On FTD's uniform
>    lattice with the declared calibration $a_{\text{phys}} \equiv \ell_P$, coordinate
>    distance and physical distance coincide by construction (rulers are voxel counts);
>    the $dl \neq dr$ split would require a derived ruler-contraction mechanism, which
>    this document does not attempt.
>
> **Frontier 3 (the spatial metric component) reverts to [OPEN]** — exactly
> `DERIV_RELATIVITY_DERIVATION.md` §12's listed remaining work ("show $g_{rr} = -1/g_{00}$
> from area preservation in flux"), which stands unresolved. Same vintage and failure
> family as the retracted non-commutativity and Born path-integral siblings in this
> directory.
>
> Not covered by this retraction: `DERIV_LATTICE_BLACK_HOLES.md` Part A claims `g_rr`
> via a *different* route (velocity-cost amplification, `v²/f` computational-budget
> reasoning), which is what `DERIV_RELATIVITY_DERIVATION.md` §18.2's "GAP-1 RESOLVED"
> line cites. That sibling argument has not received this review and is not adjudicated
> either way here.
>
> Preserved for provenance per the Documentation Cleanup Discipline. Do not cite as a
> live result.

**Epistemic Standard (original claim, retracted):** Strictly compliant with FTD Epistemic Discipline (`AGENTS.md`).  

---

## 1. The Frontier: Deriving the Spatial Metric

In FTD, general relativity emerges from the Deser self-gravitating iterative bootstrap of the vector flux field. In `DERIV_RELATIVITY_DERIVATION.md`, the time-time Schwarzschild metric component is derived as:
$$g_{00} = -\left(1 - \frac{r_s}{r}\right)$$
This is obtained because the local wave propagation speed is slowed down by the concentration of flux energy density $\bar{\rho} \equiv |\mathbf{J}|$, giving the local time-dilation wave speed:
$$C(r) = C_0 \sqrt{1 - \frac{r_s}{r}}$$

However, deriving the radial spatial component of the metric:
$$g_{rr} = \left(1 - \frac{r_s}{r}\right)^{-1}$$
is **Frontier 3**. How does this spatial curvature emerge on a uniform coordinate lattice $\mathbb{Z}^3$?

This document resolves Frontier 3 by proving that **$g_{rr} = (1 - r_s/r)^{-1}$ emerges as a direct consequence of coordinate interval contraction** on a discrete substrate.

---

## 2. Mathematical Formalization

### 2.1 Substrate Temporal and Spatial Intervals [AXIOM]
* The substrate temporal progression proceeds in uniform, integer ticks: $t \in \mathbb{N}$ with constant coordinate step $\Delta t = 1$.
* The discrete spatial substrate coordinate grid has a fixed uniform spacing $a$ (coordinate distance between adjacent voxels).
* The local wave propagation is governed by the discrete wave equation, defining the coordinate distance traveled by a wave packet in one tick $\Delta t$ along the radial direction as:
  $$dr = C(r) \Delta t$$
  where $C(r)$ is the local coordinate speed of light.

### 2.2 Local Inertial Frame and Physical Length Invariance [THEOREM]
In local inertial frames, the physical lattice spacing $dl$ is invariant and represents the constant speed of light wave-propagation distance:
$$dl \equiv a = C_0 \Delta t$$
where $C_0$ is the background speed of light.
The physical spatial interval $dl^2$ is related to the radial coordinate coordinate interval $dr^2$ by the metric radial component $g_{rr}$:
$$dl^2 = g_{rr} dr^2$$

---

## 3. Proof of the Discrete Radial Metric Projection Theorem [THEOREM]

**Theorem 1.** *The spatial Schwarzschild radial metric component $g_{rr}$ is the exact inverse of the wave speed contraction factor, yielding:*
$$g_{rr} = \left(1 - \frac{r_s}{r}\right)^{-1}$$

**Proof.**
1. The local coordinate speed of light is slowed down in the presence of flux energy concentration by the factor:
   $$C(r) = C_0 \sqrt{1 - \frac{r_s}{r}}$$
2. The coordinate radial interval $dr$ traversed by a wave packet in a single tick $\Delta t$ is:
   $$dr = C(r) \Delta t = C_0 \sqrt{1 - \frac{r_s}{r}} \Delta t$$
3. Since $dl = a = C_0 \Delta t$ is the constant physical distance traveled in the local frame (Section 2.2), we substitute $C_0 \Delta t = a$ into the expression for $dr$:
   $$dr = \sqrt{1 - \frac{r_s}{r}} a$$
4. The physical metric radial component $g_{rr}$ maps the coordinate radial interval $dr$ to the physical lattice spacing $dl = a$:
   $$a^2 = g_{rr} dr^2$$
5. Substituting the expression for $dr$ into this relation:
   $$a^2 = g_{rr} \left( \sqrt{1 - \frac{r_s}{r}} a \right)^2$$
   $$a^2 = g_{rr} \left(1 - \frac{r_s}{r}\right) a^2$$
6. Dividing both sides by $a^2 \left(1 - \frac{r_s}{r}\right)$:
   $$g_{rr} = \left(1 - \frac{r_s}{r}\right)^{-1} \quad \blacksquare$$

---

## 4. Emergent Schwarzschild Spacetime [THEOREM]

Combining the time-time component $g_{00}$ and the newly derived radial component $g_{rr}$, the physical spacetime metric is:
$$ds^2 = g_{00} c^2 dt^2 + g_{rr} dr^2 + r^2 d\Omega^2$$
$$ds^2 = -\left(1 - \frac{r_s}{r}\right) c^2 dt^2 + \left(1 - \frac{r_s}{r}\right)^{-1} dr^2 + r^2 d\Omega^2$$

This completes the FTD derivation of the **Schwarzschild metric** from the substrate. It demonstrates that the spatial curvature component ($g_{rr}$) is not an independent dynamical degree of freedom, but is **algebraically forced** by the local coordinate propagation contraction $dr/a = \sqrt{-g_{00}}$ on a uniform coordinate grid.

---

*Topic: Derivation of the Schwarzschild $g_{rr}$ spatial metric component.*  
