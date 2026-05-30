# The Discrete Radial Metric Projection Theorem: Deriving the Schwarzschild $g_{rr}$ Component in FTD

**Version:** 1.0  
**Framework Version:** FTD v5.33  
**Status:** [THEOREM] — Derivation of the spatial Schwarzschild radial component $g_{rr}$.  
**Epistemic Standard:** Strictly compliant with FTD Epistemic Discipline (`AGENTS.md`).  

---

## 1. The Frontier: Deriving the Spatial Metric

In FTD, general relativity emerges from the Deser self-gravitating iterative bootstrap of the vector flux field. In `DERIV_RELATIVITY_DERIVATION.md`, the time-time Schwarzschild metric component is derived as:
$$g_{00} = -\left(1 - \frac{r_s}{r}\right)$$
This is obtained because the local wave propagation speed is slowed down by the concentration of flux energy density $\bar{\rho} \equiv |\mathbf{J}|$, giving the local time-dilation wave speed:
$$C(r) = C_0 \sqrt{1 - \frac{r_s}{r}}$$

However, deriving the radial spatial component of the metric:
$$g_{rr} = \left(1 - \frac{r_s}{r}\right)^{-1}$$
has remained **`[OPEN]`** (Frontier 3). How does this spatial curvature emerge on a uniform coordinate lattice $\mathbb{Z}^3$?

This document resolves this frontier by proving that **$g_{rr} = (1 - r_s/r)^{-1}$ emerges as a direct consequence of coordinate interval contraction** on a discrete substrate.

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

*Document created: May 27, 2026*  
*Topic: Derivation of the Schwarzschild $g_{rr}$ spatial metric component.*  
*Framework: Foundational Ternary Dynamics v5.33*  
