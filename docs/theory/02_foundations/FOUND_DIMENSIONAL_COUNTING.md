# Dimensional Counting: From 0.5D to D = 3

## A Constructive Argument for Why Three Spatial Dimensions

**Date:** March 17, 2026
**Framework:** Foundational Ternary Dynamics v5.28
**Status:** Constructive argument with mixed epistemic status
**Dependencies:** FOUND_SPACETIME_EMERGENCE.md, DERIV_WATSON_GSTAR_IDENTITY.md

---

## Abstract

We present a constructive counting argument that arrives at exactly three spatial dimensions from the pairing ontology of FOUND_SPACETIME_EMERGENCE.md. The argument proceeds in four steps: (1) a single point in void contributes 0.5D, (2) a second point forces a paired axis giving 1D, (3) orthogonal depth adds 1D giving 2D, and (4) state evolution (time) as an independent axis completing the causal structure adds the final 1D giving D = 3. The resulting 3D structure uniquely determines the 26-neighbor Moore neighborhood, whose BCC sublattice produces G* via the Watson integral identity.

**Honest assessment:** Steps 1-3 are well-motivated within FTD's existing ontology. Step 4 (time contributing to effective spatial dimensionality) is the weakest link and requires careful treatment given FTD's ontological separation of space and time.

---

## Part I: The 0.5D Point

### 1.1 Starting Point

From FOUND_SPACETIME_EMERGENCE.md (DIM-1), a single axis without reference is 0.5D — it exists but is undetermined. We adopt this as our starting point.

A single point in void:
- **Exists** (ontological presence)
- **Has no orientation** (no reference frame)
- **Has no extent** (no distance measure)
- **Has no relation** (nothing else exists)

This is precisely the 0.5D ontology: potential without actuality.

**Epistemic status:** [AXIOM] — inherited from DIM-1.

### 1.2 Why 0.5 and Not 0 or 1?

A point is not 0D (it exists, so it has more structure than nothing). It is not 1D (a single point cannot define direction, distance, or coordinate). The assignment D = 0.5 captures this intermediate status: **present but undetermined**.

---

## Part II: Pairing Forces 1D

### 2.1 The Second Point

A second point introduces **relation**. From FOUND_SPACETIME_EMERGENCE.md (DIM-2, DIM-3), pairing (X ⊗ Y) differs fundamentally from stacking (X + Y):

| Operation | Result |
|-----------|--------|
| Two isolated 0.5D points (stacking) | 0.5 + 0.5 = 1.0, but still undetermined |
| Two related 0.5D points (pairing) | 0.5 × 2 = 1D actual dimension |

Pairing creates:
- **Direction** (from point A to point B)
- **Distance** (the separation between them)
- **Orientation** (the axis AB is now determined)

Both X and Y axes are instantiated equally by the pairing — the line AB simultaneously defines the axis and the measure along it.

**Epistemic status:** [THEOREM] — follows from the pairing principle (DIM-2, DIM-3).

### 2.2 The Counting

$$0.5D \text{ (point)} + 0.5D \text{ (second point)} \xrightarrow{\text{pairing}} 1D$$

---

## Part III: Orthogonal Depth

### 3.1 The Z-Axis

Given one established dimension (the X-axis from pairing), a third point not on this axis defines orthogonal depth. This is the standard geometric construction: two points define a line, a third point off the line defines a plane.

But in FTD's ontology, this is more than geometry. The Z-axis is orthogonal to both the X-axis and the paired XY relation. It adds genuine new structure:

$$1D + 1D_\perp = 2D$$

### 3.2 Why Exactly 1D More?

One orthogonal direction is the **minimum** needed to escape the line. Any additional orthogonal directions at this stage would be arbitrary — there is no mechanism to generate more than one orthogonal axis from a single off-axis point.

**Epistemic status:** [THEOREM] — standard dimensional construction.

---

## Part IV: Time as the Third Spatial Dimension

### 4.1 The Claim

State evolution (the tick) is orthogonal to all spatial axes. In FTD, time is the discrete counter $t \in \mathbb{N}$ that drives state updates. This temporal axis contributes the final degree of freedom to the causal structure:

$$2D + 1D_{\text{time}} = 3D_{\text{effective}}$$

### 4.2 The Tension

FOUND_SPACETIME_EMERGENCE.md Part VII establishes that space ($\mathbb{Z}^3$) and time ($\mathbb{N}$) are **ontologically separate**. How can time "contribute" a spatial dimension if it is categorically different from space?

### 4.3 Resolution: Causal Dimensionality vs. Spatial Dimensionality

The resolution is that time completes the **causal structure's dimensionality** without being spatial itself:

| Concept | What it means | D contribution |
|---------|---------------|----------------|
| Spatial dimensionality | Axes of the lattice $\mathbb{Z}^D$ | Direct |
| Causal dimensionality | Independent degrees of freedom in the causal structure | Includes time |
| Effective dimensionality | What determines lattice geometry | = causal D |

The argument is not that time IS a spatial dimension, but that the **number of spatial dimensions must match the causal dimensionality**. The tick provides an independent causal degree of freedom, so the lattice must have D = 3 to accommodate it.

### 4.4 Supporting Evidence

1. **The BCC connection:** The 8 BCC neighbors at $(\pm 1, \pm 1, \pm 1)$ involve all three axes simultaneously — they are the "triple product" neighbors. This is precisely the structure that corresponds to fully coupled causal evolution (all dimensions interacting at once).

2. **Watson's integral:** $I_1 = \Gamma(1/4)^4/(4\pi^3) = G^{*2}/(2\pi)$ uses the BCC dispersion $\lambda = 1 - \cos k_1 \cos k_2 \cos k_3$, which is the **product** of all three cosines. This multiplicative structure requires exactly 3 factors.

3. **The gap equation:** The master quadratic emerges from the gap equation $x^2 = 16 \cdot 2\pi \cdot G^{\text{BCC}}_L(0) \cdot (x - G^*)$ in the thermodynamic limit. The coefficient $16 \cdot 2\pi \cdot I_1 = 16 G^{*2}$ is the Vieta sum for the master quadratic. This only works with a 3D BCC lattice.

### 4.5 Honest Assessment

This step is the **weakest** in the chain. The claim that causal dimensionality determines spatial dimensionality is [CONJECTURE]. The supporting evidence is suggestive but not logically necessary — one could have D = 3 for other reasons with time still separate.

**Epistemic status:** [CONJECTURE]

---

## Part V: From D = 3 to Moore Neighborhood to G*

### 5.1 D = 3 Determines the Moore Neighborhood

Given a 3D cubic lattice, the Moore neighborhood is uniquely determined:

$$|\mathcal{M}| = 3^D - 1 = 3^3 - 1 = 26$$

This decomposes into three cubic sublattices (see DERIV_WATSON_GSTAR_IDENTITY.md):

| Sublattice | Neighbors | Offsets | Structure factor |
|------------|-----------|---------|-----------------|
| **SC** | 6 (faces) | $(\pm 1, 0, 0)$ etc. | $\cos k_1 + \cos k_2 + \cos k_3$ |
| **FCC** | 12 (edges) | $(\pm 1, \pm 1, 0)$ etc. | $\cos k_1 \cos k_2 + \cos k_1 \cos k_3 + \cos k_2 \cos k_3$ |
| **BCC** | 8 (corners) | $(\pm 1, \pm 1, \pm 1)$ | $\cos k_1 \cos k_2 \cos k_3$ |

### 5.2 G* Emerges from the BCC Sublattice

The Watson–G* identity (DERIV_WATSON_GSTAR_IDENTITY.md) establishes:

$$G^{*2}/(2\pi) = I_1 = \frac{\Gamma(1/4)^4}{4\pi^3} = 1.39320...$$

where $I_1$ is Watson's BCC lattice Green's function at the origin. This is verified numerically by finite-size scaling on periodic tori:

| L | $G^{\text{BCC}}_L(0)$ | Error vs $I_1$ |
|---|----------------------|-----------------|
| 2 | 0.2500 (= 1/4 exact) | 82.1% |
| 8 | 1.1624 | 16.6% |
| 32 | 1.3367 | 4.1% |
| 64 | 1.3650 | 2.0% |
| 128 | 1.3791 | 1.0% |
| 256 | 1.3861 | 0.5% |

Convergence is O(1/L) due to integrable singularities at the Brillouin zone corners. Richardson extrapolation from L=128,256 gives $I_1 = 1.3932$ to 0.0001% accuracy.

### 5.3 The Gap Equation Reproduces the Master Quadratic

The gap equation with finite-lattice BCC self-energy:

$$x^2 = 16 \cdot 2\pi \cdot G^{\text{BCC}}_L(0) \cdot (x - G^*)$$

In the thermodynamic limit $L \to \infty$, $G^{\text{BCC}}_L(0) \to I_1 = G^{*2}/(2\pi)$, giving:

$$x^2 = 16 G^{*2} \cdot x - 16 G^{*3}$$

which is exactly the master quadratic with roots $x_+ = 1/\alpha = 137.036$ and $x_- = N_c = 3.024$.

**Epistemic status:** [THEOREM] — the identity chain is algebraically exact. The gap equation form is [SELECTION].

### 5.4 The Coefficient 16: BCC + Ternary ReLU DOF Counting

The gap equation coefficient $n_{\text{DOF}} = 16$ was previously [OPEN]. It is now derived from the BCC lattice structure combined with the ternary ReLU threshold:

**Derivation:**
- The BCC sublattice has coordination number $z_{\text{BCC}} = 2^D = 2^3 = 8$
- The ternary state space $\{-1, 0, +1\}$ has 3 states, but the void state ($s = 0$) does not participate in interactions
- Only the 2 non-void states ($+1$ and $-1$) contribute degrees of freedom
- Therefore: $n_{\text{DOF}} = z_{\text{BCC}} \times (\text{non-void states}) = 8 \times 2 = 16$

**Four equivalent expressions:**

| Expression | Value | Origin |
|-----------|-------|--------|
| $z_{\text{BCC}} \times 2$ | $8 \times 2 = 16$ | BCC coordination $\times$ non-void ternary states |
| $N_{\text{base}}^2$ | $4^2 = 16$ | Framework integer squared |
| $2^{D+1}$ | $2^4 = 16$ | Binary DOF in $D+1$ causal dimensions |
| $|\text{Aut}(E)|^2$ | $4^2 = 16$ | Lemniscatic curve automorphism group |

These all equal 16 because $D = 3$ forces $z_{\text{BCC}} = 2^3 = 8$ and $N_{\text{base}} = 2^{(D+1)/2} = 4$.

**Self-referential ReLU closure:** The ternary threshold $K_B \sim \alpha^{11}$ is astronomically small compared to the flux scale $\sigma = \sqrt{I_1} \approx 1.18$. Therefore the manifested fraction $p \approx 1$ and $n_{\text{DOF}} = 16p^2 \approx 16$ to machine precision. The circle closes: $\alpha \to K_B \to p \approx 1 \to n_{\text{DOF}} = 16 \to \text{master quadratic} \to \alpha$.

**Epistemic status:** [THEOREM] — $16 = 8 \times 2$ from BCC coordination and non-void ternary states.

---

## Part VII: Ontic Forms of the Gap Equation

### 7.1 Pi Is Not Ontic

The standard Watson form of the gap equation contains $\pi$:

$$x^2 = 16 \cdot 2\pi \cdot I_1 \cdot (x - G^*)$$

But $\pi$ enters only through Watson's conventional $(2\pi)^{-3}$ Brillouin zone normalization. The Watson–G* identity $I_1 = G^{*2}/(2\pi)$ shows that the natural lattice quantity is $G^{*2}$, not $I_1$. Substituting eliminates $\pi$ entirely:

$$\boxed{x^2 = 16\,G^{*2}\,(x - G^*)}$$

This **pure G\* form** contains only:
- $G^*$ — the lemniscatic constant (ontic, from $\Gamma(1/4)$)
- $16 = z_{\text{BCC}} \times 2$ — BCC coordination × non-void ternary states
- The quadratic structure itself

### 7.2 Equivalent Pi-Free Forms

| Form | Expression | Constants |
|------|-----------|-----------|
| **Pure G\*** | $x^2 = 16G^{*2}(x - G^*)$ | G* only |
| **Varpi-M** | $x^2 = 64\varpi M(x - 2\sqrt{\varpi M})$ | $\varpi$, $M$ |
| **Dimensionless** ($y = x/G^*$) | $y^2 = 16G^*(y - 1)$ | G* only |
| **Ontic lattice** | $x^2 = 16\,G_{\text{ontic}}(0)\,(x - G^*)$ | $G_{\text{ontic}} \to G^{*2}$ |

The **ontic Green's function** is defined as $G_{\text{ontic}}(0) = 2\pi \cdot G_{\text{Watson}}(0)$, which converges to $G^{*2}$ in the thermodynamic limit, making the lattice form identical to the pure G* form.

### 7.3 The Vieta Sum = Product Identity

In the dimensionless form $y^2 = 16G^*(y - 1)$, the Vieta relations yield:

$$y_+ + y_- = y_+ \cdot y_- = 16G^*$$

This is remarkable: the sum and product of the rescaled roots are **equal**. Translating back:

$$\frac{1}{\alpha} + N_c = \frac{N_c}{\alpha \cdot G^*}$$

### 7.4 G* as Harmonic Ratio

The Vieta identity can be rearranged:

$$G^* = \frac{x_+ \cdot x_-}{x_+ + x_-} = \frac{(1/\alpha) \cdot N_c}{1/\alpha + N_c}$$

This is the "parallel resistance" formula. $G^*$ is the **harmonic combination** of $1/\alpha$ and $N_c$ — the two roots of the master quadratic. The bridge constant connecting electromagnetic coupling to color charge is their harmonic ratio.

### 7.5 The Ontic Constant Hierarchy

The irreducible seed is $\Gamma(1/4)$. All other constants derive from it:

$$\Gamma(1/4) \xrightarrow{} M = \frac{1}{\text{AGM}(1,\sqrt{2})} \xrightarrow{} \varpi = \pi M \xrightarrow{} G^* = 2\sqrt{\varpi M} \xrightarrow{} \pi = \frac{\Gamma(1/4)^2}{2\sqrt{2}\,\varpi}$$

$\pi$ is the **last** constant derived, not the first. The gap equation in its natural form never needs it.

### 7.6 Deep Structure: The One-Parameter Family

The Vieta sum = product condition constrains the dimensionless quadratic to a **one-parameter family**:

$$y^2 - Sy + S = 0, \qquad S = 16G^*$$

Given any $S \geq 4$, both roots are determined. FTD selects $S = 16G^*$ from lattice geometry. Everything else follows.

**Mobius involution:** The roots satisfy $y_- = y_+/(y_+ - 1)$, a self-inverse Mobius transformation with fixed points at 0 and 2. The cross-ratio $(y_+, y_-; 0, 2) = -1$ exactly — the roots are **harmonic conjugates** with respect to the pair $(0, 2)$.

**Shifted-root product identity:**

$$(y_+ - 1)(y_- - 1) = 1 \qquad \Longleftrightarrow \qquad (x_+ - G^*)(x_- - G^*) = G^{*2}$$

The deviations of $1/\alpha$ and $N_c$ from $G^*$ multiply to give exactly $G^{*2}$. On a logarithmic scale, $G^*$ is the **geometric midpoint** between $(x_+ - G^*)$ and $(x_- - G^*)$.

**Three classical means of $x_+$ and $x_-$:**

| Mean | Formula | Value |
|------|---------|-------|
| Arithmetic | $(x_+ + x_-)/2$ | $8G^{*2}$ |
| Geometric | $\sqrt{x_+ x_-}$ | $4G^{*3/2}$ |
| Harmonic | $2x_+ x_-/(x_+ + x_-)$ | $2G^*$ |

All three means are pure powers of $G^*$ times integers — a consequence of the S = P structure.

### 7.7 EM-QCD Duality via Mobius Involution

The Mobius map $f(y) = y/(y-1)$ that swaps the roots is an **inversion** in the deviation variable. Let $u = y - 1$ (deviation from the pole at $y = 1$, i.e., deviation of $x$ from $G^*$). Then:

$$f(1 + u) = 1 + \frac{1}{u}$$

The map sends $u \to 1/u$. Since $u_+ \cdot u_- = 1$ (epsilon product identity), we have:

$$u_+ = \frac{1/\alpha - G^*}{G^*} = 45.32, \qquad u_- = \frac{N_c - G^*}{G^*} = 0.0221$$

The electromagnetic sector is "large" ($u_+ \gg 1$) exactly as much as the color sector is "small" ($u_- \ll 1$), measured from $G^*$. The logarithmic deviations are perfectly antisymmetric:

$$\log u_+ = -\log u_- = 3.814$$

This is an **EM-QCD duality**: neither sector is more fundamental. $G^*$ is the self-dual bridge constant where the two sectors balance. The self-dual point $x = 2G^*$ (harmonic mean of the roots) is where $\alpha_{\text{dual}} = 1/(2G^*) \approx 0.169$.

### 7.8 Complete Causal Map

The master quadratic is fully determined by two inputs:

```
D = 3  ─────────────────────────────────────────────────────┐
  ├─> z_BCC = 2³ = 8                                       │
  ├─> Moore = 3³ − 1 = 26                                  │
  └─> BCC dispersion: λ = 1 − cos k₁ cos k₂ cos k₃        │
        └─> Watson integral I₁ = Γ(1/4)⁴/(4π³)             │
              └─> G* = √(2π I₁)                            │
                    │                                       │
{−1,0,+1} ternary ─┤                                       │
  └─> 2 non-void   │                                       │
        └─> n = 8 × 2 = 16                                 │
              └────────────> x² = 16G*²(x − G*)  [SELECTION]
                               ├─> x₊ = 1/α = 137.036
                               ├─> x₋ = Nc = 3.024
                               ├─> G* = x₊x₋/(x₊ + x₋)
                               ├─> (x₊ − G*)(x₋ − G*) = G*²
                               └─> Cross-ratio = −1
```

**Inputs:** $D = 3$, ternary states $\{-1, 0, +1\}$, quadratic form (see Part VIII).
**Outputs:** $\alpha$, $N_c$, $G^*$ as harmonic ratio, Mobius duality, all classical means.

**Epistemic status:** [THEOREM] — all identities are algebraically exact.

---

## Part VIII: Why the Gap Equation Is Quadratic

### 8.1 The Null Cone Argument

The equation $i^2 + a^2 + b^2 = 0$ — the null cone in $\mathbb{C}^3$ — is the structural seed. After substituting $i^2 = -1$:

$$a^2 + b^2 = 1$$

This is the Pythagorean constraint, readable simultaneously as:

| Reading | Object |
|---------|--------|
| $i = \sqrt{-1}$, $a,b \in \mathbb{R}$ | Unit circle $S^1$, U(1) phase |
| $i = \sqrt{-1}$, $a,b \in \mathbb{C}$ | Complex conic $\cong \mathbb{C}^*$ |
| $(i,a,b) \in \mathbb{C}^3$ null | Isotropic cone → Riemann sphere $\mathbb{CP}^1$ |
| Wick-rotated Minkowski | Null cone in (1+2)D: $t^2 = x^2 + y^2$ |

In FTD's lattice, each spatial dimension has U(1) phase structure: momentum $k_i \in [0, 2\pi]$ lives on a circle. The BCC structure factor $\cos k_1 \cos k_2 \cos k_3$ is the multiplicative coupling of three U(1) phases. The Green's function $G(0) = \sum_k (1 - \gamma)^{-1}$ involves quadratic propagators by construction.

The Dyson equation with self-energy $\Sigma \sim G^{*2}/x$ (from the quadratic dispersion) gives $x^2 = G^{*2} + c \cdot x$ after multiplying through — inherently quadratic, forced by the quadratic metric.

### 8.2 The Elementary Symmetric Polynomial Structure

The Moore neighborhood decomposition is not arbitrary — it is the **characteristic polynomial** of a $D \times D$ diagonal cosine matrix. For $D = 3$ with $C = \text{diag}(\cos k_1, \cos k_2, \cos k_3)$:

$$\det(C - \lambda I) = -\lambda^3 + e_1 \lambda^2 - e_2 \lambda + e_3$$

where the elementary symmetric polynomials are:

| Polynomial | Expression | Lattice sublattice | Neighbors |
|-----------|-----------|-------------------|-----------|
| $e_1$ | $\cos k_1 + \cos k_2 + \cos k_3$ | **SC** (faces) | 6 |
| $e_2$ | $\cos k_1 \cos k_2 + \cos k_1 \cos k_3 + \cos k_2 \cos k_3$ | **FCC** (edges) | 12 |
| $e_3$ | $\cos k_1 \cos k_2 \cos k_3$ | **BCC** (corners) | 8 |

The BCC sublattice corresponds to $e_3$, the **top** elementary symmetric polynomial of degree $D = 3$. The Watson integral $I_1$ is constructed from $e_3$. The master quadratic's small root $N_c \approx 3.024 \approx D$ because $N_c$ reflects the **degree of the characteristic polynomial** of the $D$-dimensional lattice.

### 8.3 Complex Multiplication and the Coefficient 16

The lemniscatic elliptic curve has **complex multiplication by $\mathbb{Z}[i]$** — the Gaussian integers. The norm form of $\mathbb{Z}[i]$ is $N(a + bi) = a^2 + b^2$, which is exactly the null cone equation restricted to the unit circle.

The CM endomorphism (multiplication by $i$) gives $|\text{Aut}(E)| = 4$ instead of the generic 2. Therefore $|\text{Aut}(E)|^2 = 16 = n_{\text{DOF}}$.

The entire structure traces from $i^2 + 1 = 0$:

```
i² + 1 = 0
  ├─> Null cone: i² + a² + b² = 0
  │     ├─> Quadratic metric: a² + b² = 1 (U(1) phases on lattice)
  │     ├─> Wick rotation: Euclidean ↔ Minkowski
  │     └─> Riemann sphere CP¹ (isotropic cone projectivization)
  │
  ├─> CM by Z[i]: lemniscatic elliptic curve
  │     ├─> ϖ (lemniscatic constant)
  │     ├─> |Aut(E)| = 4, so |Aut(E)|² = 16
  │     └─> G* = 2√(ϖM)
  │
  └─> D = 3 lattice: char poly of diag(cos k₁, cos k₂, cos k₃)
        ├─> e₁ (SC), e₂ (FCC), e₃ (BCC) = degree D = 3
        └─> Watson integral I₁ from e₃ → G*
```

### 8.4 Binary-Ternary Resonance and N_c = 3

Powers of 2 modulo 3 alternate: $2^k \mod 3 = \{1, 2, 1, 2, \ldots\}$ because $2 \equiv -1 \pmod{3}$. No power of 2 is ever divisible by 3. In a binary frequency system, exact 3-fold symmetry is structurally impossible — it can only emerge as **approximate** resonance from interference between frequencies $\equiv 1 \pmod{3}$ and $\equiv 2 \pmod{3}$.

FTD sits at the intersection of binary ($z_{\text{BCC}} = 2^3$) and ternary ($\{-1, 0, +1\}$, $3^3 - 1 = 26$ Moore neighbors) structure. $N_c = 3$ emerges not as a direct construction but as a resonance — the small root of the master quadratic where binary and ternary structure interfere.

### 8.5 Gaussian Exactness and the At-Most-Quadratic Constraint

The FTD Euclidean action $S_E[s, \mathbf{J}] = \frac{1}{2}\mathbf{J}^T M \mathbf{J} + g_c \mathbf{b}(s)^T \mathbf{J} + c(s)$ is **quadratic** in the flux field $\mathbf{J}$. This structural fact has three rigorous consequences:

1. **[THEOREM] Exact Gaussian J-integral.** The integral over $\mathbf{J}$ evaluates exactly — the "one-loop" result is the complete result, not an approximation. There are no higher-loop corrections because $S_E$ has no cubic or quartic terms in $\mathbf{J}$ (the Hessian $\partial^2 S_E / \partial \mathbf{J}^2 = M$ is $\mathbf{J}$-independent).

2. **[THEOREM] S_eff quadratic in s.** Because $\mathbf{b}(s)$ is linear in $s$, the effective action after integrating out $\mathbf{J}$ is $S_{\text{eff}}[s] = -\frac{g_c^2}{2} s^T G s + \text{const}$, exactly quadratic in $s$ with kernel $G = M^{-1}$.

3. **[THEOREM] At-most-quadratic gap equation.** A quadratic $S_{\text{eff}}$ constrains any self-consistency equation to be at most degree 2 in the coupling parameter $x = 1/g_c^2$.

This eliminates the "one-loop ansatz" previously listed as an assumption. The remaining gap is narrower: the specific self-consistency prescription $F(x) = K(1 - G^*/x)$ remains [SELECTION] — it is not derived from $S_{\text{eff}}$ but argued from the structure of one-loop screening. See `scripts/proofs/proof_self_energy_derivation.py` (10/10 tests pass).

**Epistemic status:** The null cone → quadratic chain is [THEOREM] (algebraic). The Gaussian exactness → at-most-quadratic constraint is [THEOREM] (DC-15a). The claim that the gap equation form is *uniquely* forced remains [CONJECTURE] (DC-15) — the self-consistency prescription is argued but not derived from first principles.

---

## Part VI: Critical Assessment

### 6.1 Strength of the Argument

| Step | Claim | Status | Confidence |
|------|-------|--------|------------|
| DC-1 | Single point = 0.5D | [AXIOM] | Definitional |
| DC-2 | Second point forces 1D via pairing | [THEOREM] | High (from DIM-2, DIM-3) |
| DC-3 | Orthogonal depth adds 1D → 2D | [THEOREM] | High (standard geometry) |
| DC-4 | Time adds 1D → D = 3 | [CONJECTURE] | Low (weakest link) |
| DC-5 | D = 3 → Moore → BCC → G* | [THEOREM] | High (algebraic identity) |
| DC-6 | Coefficient 16 = z_BCC × (non-void ternary) | [THEOREM] | High (combinatorial) |

### 6.2 DC-4: The Weakest Link

The argument that time contributes to effective dimensionality faces two objections:

1. **Ontological separation:** If space and time are truly categorically different (ST-1), why should time count toward spatial dimensionality at all?

2. **Circularity risk:** We assume D = 3 for the lattice, then argue time provides the third dimension. But the lattice structure is an axiom — we're offering a motivation for D = 3, not a derivation.

### 6.3 What This Argument Does and Doesn't Do

**Does:**
- Provides a constructive narrative from 0.5D → 1D → 2D → 3D
- Connects dimensional counting to the Moore neighborhood and G*
- Explains *why* the BCC sublattice (the triple-product structure) is singled out
- Offers a physical interpretation of the 0.5D ontology in action

**Doesn't:**
- Prove that D = 3 is the unique possibility (other arguments, e.g. Arg 1-7 in FOUND_SPACETIME_EMERGENCE.md, are needed)
- Uniquely derive the gap equation *form* from first principles (Part VIII provides a structural argument [CONJECTURE] that the quadratic form is forced by $i^2 = -1$; DC-15a proves the gap equation is at-most-quadratic [THEOREM]; but the specific self-consistency prescription $F(x) = K(1 - G^*/x)$ remains [SELECTION])
- Resolve the ontological status of time's contribution to dimensionality

---

## Claims Summary

| Claim ID | Statement | Status |
|----------|-----------|--------|
| **DC-1** | Single point in void = 0.5D | **[AXIOM]** |
| **DC-2** | Second point forces 1D via pairing | **[THEOREM]** |
| **DC-3** | Orthogonal depth adds 1D → 2D | **[THEOREM]** |
| **DC-4** | Time completes causal dimensionality → D = 3 | **[CONJECTURE]** |
| **DC-5** | D = 3 → Moore(26) → BCC(8) → G* via Watson | **[THEOREM]** |
| **DC-6** | Coefficient 16 = z_BCC × (non-void ternary states) = 8 × 2 | **[THEOREM]** |
| **DC-7** | Self-referential ReLU closure: α → K_B → p ≈ 1 → n_DOF = 16 → α | **[THEOREM]** |
| **DC-8** | Ontic gap equation: x² = 16G*²(x − G*) is pi-free | **[THEOREM]** |
| **DC-9** | Dimensionless Vieta: y₊ + y₋ = y₊·y₋ = 16G* | **[THEOREM]** |
| **DC-10** | G* = x₊·x₋/(x₊ + x₋) (harmonic ratio of 1/α and N_c) | **[THEOREM]** |
| **DC-11** | (x₊ − G*)(x₋ − G*) = G*² (shifted-root product) | **[THEOREM]** |
| **DC-12** | Roots are Mobius harmonic conjugates: cross-ratio = −1 | **[THEOREM]** |
| **DC-13** | EM-QCD duality: u₊·u₋ = 1, log-symmetric deviations from G* | **[THEOREM]** |
| **DC-14** | SC, FCC, BCC = elementary symmetric polynomials e₁, e₂, e₃ | **[THEOREM]** |
| **DC-15** | Gap equation quadratic from null cone i² + a² + b² = 0 | **[CONJECTURE]** |
| **DC-15a** | Exact Gaussian J-integral → S_eff quadratic → gap eq at most quadratic | **[THEOREM]** |
| **DC-16** | N_c ≈ D from degree of characteristic polynomial of D-dim lattice | **[CONJECTURE]** |

---

## Verification

- Finite-size scaling and gap equation: `scripts/proofs/proof_partition_function_gstar.py`
- Gaussian exactness and at-most-quadratic constraint: `scripts/proofs/proof_self_energy_derivation.py`
- Watson–G* identity: `scripts/proofs/proof_watson_gstar.py` (if present)
- Dimensional emergence: `scripts/verification/verify_dimensional_emergence.py`

---

## Cross-References

- **0.5D ontology and pairing:** [FOUND_SPACETIME_EMERGENCE.md](FOUND_SPACETIME_EMERGENCE.md) (Parts I-III)
- **Watson–G* identity:** [DERIV_WATSON_GSTAR_IDENTITY.md](../04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md)
- **Master quadratic:** [MATH_MASTER_QUADRATIC.md](../01_reference/MATH_MASTER_QUADRATIC.md)
- **Moore decomposition:** [DERIV_ALPHA_LATTICE_MECHANISM.md](../04_coupling/DERIV_ALPHA_LATTICE_MECHANISM.md)
- **G* as bridge constant:** [DERIV_GSTAR_PF_BRIDGE.md](../04_coupling/DERIV_GSTAR_PF_BRIDGE.md)

---

*Document created: March 17, 2026*
*Framework: Foundational Ternary Dynamics v5.28*
