# The Discrete Radial Metric Projection Theorem: Deriving the Schwarzschild $g_{rr}$ Component in FTD

**Status:** **[RETRACTED 2026-07-02 — invalid proof; FTD-0360]** (was: [THEOREM])

> **RETRACTION NOTICE (2026-07-02, per the FTD-0360 finalization batch, adjudicating the
> two monograph rows flagged by the FTD-0356 cluster review, `MONOGRAPH_EFFECTIVE_EQUATIONS.md`
> §7.2 "await their own review pass").** This document's central proof is invalid; each
> defect below was verified symbolically (sympy, exact) under FTD-0360:
>
> 1. **The premises force $g_{rr} = 1$, not $(1-r_s/r)^{-1}$.** For the metric
>    $ds^2 = -f\,c^2dt^2 + g_{rr}\,dr^2$ with $f = 1 - r_s/r$, a radial null ray has
>    coordinate speed $c\sqrt{f/g_{rr}}$. The document's own premise (step 1, inherited
>    from `DERIV_RELATIVITY_DERIVATION.md` §11's time-dilation-only saturation slowing):
>    coordinate wave speed $C(r) = C_0\sqrt{f}$. Requiring the locally measured light
>    speed to be $C_0$ then gives $\sqrt{f/g_{rr}} = \sqrt{f}$, i.e. $g_{rr} = 1$ — a
>    **flat** radial metric.
> 2. **The conclusion contradicts the premise.** The concluded metric
>    $\{g_{00} = -f,\; g_{rr} = 1/f\}$ propagates radial light at coordinate speed
>    $c\,f$ (**first** power), not the premise's $c\sqrt{f}$. Premise and conclusion
>    cannot both hold. This is the classic scalar-medium pitfall: the $g_{00}$/$g_{rr}$
>    split is precisely the physical content that doubles GR's light deflection over the
>    time-dilation-only value; a single scalar $C(r)$ cannot fix two metric functions.
> 3. **The load-bearing step begs the question and violates local light-speed constancy.**
>    §2.2's "physical length invariance" $dl \equiv a = C_0\Delta t$ *per coordinate
>    tick* is tagged [THEOREM] but never proven — $a^2 = g_{rr}\,dr^2$ **is** the
>    conclusion. Under $g_{00} = -f$ it implies a locally measured light speed
>    $C_0/\sqrt{f} > C_0$ (superluminal, divergent at the horizon), contradicting §2.2's
>    own local-inertial-frame language. The consistent proper-time reading
>    ($dl = C_0\,d\tau$) yields $g_{rr} = 1$ again.
> 4. **Substrate ontology.** On FTD's uniform lattice with the declared calibration
>    $a_{\text{phys}} \equiv \ell_P$, coordinate distance and physical distance coincide
>    by construction (rulers are voxel counts); the $dl \neq dr$ split would require a
>    derived ruler-contraction mechanism, which this document does not attempt.
>
> **Frontier 3 (the spatial metric component) reverts to [OPEN]** — exactly
> `DERIV_RELATIVITY_DERIVATION.md` §12's listed remaining work ("show $g_{rr} = -1/g_{00}$
> from area preservation in flux"), which stands unresolved. Same vintage and failure
> family as the retracted non-commutativity and Born path-integral siblings in this
> directory. Preserved for provenance per the Documentation Cleanup Discipline. Do not
> cite as a live result.

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
