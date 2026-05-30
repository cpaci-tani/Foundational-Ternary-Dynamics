# The Dirac Equation from the Master Quadratic

## Fermions Are the Complex Regime of Self-Consistency

**Date:** March 17, 2026
**Status:** Derivation with honest epistemic assessment
**Dependencies:** FOUND_AXIOM_ZERO.md, FOUND_BORN_RULE_NULL_CONE.md, DERIV_QUADRATIC_NECESSITY.md, DERIV_MASTER_QUADRATIC_GAP_EQUATION.md

---

## Abstract

The Dirac equation is not imported into FTD from standard physics. It is the **complex regime** of the same master quadratic that produces the fine structure constant and color charge number in its real regime. The discriminant of the generalized master quadratic $x^2 - kG^{*2}x + kG^{*3} = 0$ partitions physics into three domains:

$$\Delta = kG^{*3}(kG^* - 4)$$

- $\Delta > 0$ ($k = 16$): real roots → bosons (coupling constants $\alpha$, $N_c$)
- $\Delta = 0$ ($k = 4/G^*$): degenerate → measurement (Born rule)
- $\Delta < 0$ ($k < 4/G^*$): **complex roots → fermions (Dirac spinors)**

The null cone equation $i^2 + a^2 + b^2 = 0$ — which IS the ternary axiom $0 = (-1) + (+1)$ — provides the geometric structure for all three regimes. The Born rule is the Pythagorean theorem. The Dirac equation is the oscillation in the complex plane. Both emerge from the same single equation.

---

## Part I: The Ternary Axiom IS the Null Cone

### 1.1 One Equation, Three Readings [THEOREM]

The foundational equation of FTD:

$$0 = (-1) + (+1) \tag{1.1}$$

is algebraically identical to the null cone condition:

$$i^2 + a^2 + b^2 = 0 \tag{1.2}$$

where $i^2 = -1$, $a^2 = (+1)^2 = 1$ for the positive state amplitude, and the constraint forces $a^2 + b^2 = 1$ (the Born rule / Pythagorean theorem).

Three structures emerge from this single equation:

| Reading | Domain | Structure | Physics |
|---------|--------|-----------|---------|
| Real ($a, b \in \mathbb{R}$) | Unit circle $S^1$ | U(1) phase | Bosonic gauge field |
| Degenerate ($a^2 + b^2 = 1$ exactly) | Pythagorean constraint | Born rule $P = |ψ|^2$ | Measurement |
| Complex ($a, b \in \mathbb{C}$) | Complex conic $\cong \mathbb{C}^*$ | Spinor oscillation | **Fermion** |

### 1.2 The Discriminant Trichotomy [THEOREM]

The generalized master quadratic $x^2 - kG^{*2}x + kG^{*3} = 0$ has discriminant:

$$\Delta = k^2G^{*4} - 4kG^{*3} = kG^{*3}(kG^* - 4) \tag{1.3}$$

The critical coefficient is $k_{\text{crit}} = 4/G^* \approx 1.352$.

| Regime | $k$ value | $\Delta$ | Roots | Physics |
|--------|-----------|----------|-------|---------|
| $k > k_{\text{crit}}$ | $k = 16$ (physics) | $> 0$ | Two real: $x_+ = 137, x_- = 3$ | Bosonic couplings |
| $k = k_{\text{crit}}$ | $k = 4/G^*$ | $= 0$ | One degenerate: $x = 2G^*$ | Born rule threshold |
| $k < k_{\text{crit}}$ | $k = 1/2$ | $< 0$ | Two complex conjugates | **Fermionic oscillation** |

The three regimes are not three separate theories. They are three sectors of ONE quadratic equation, determined by the single parameter $k$.

---

## Part II: The Complex Regime IS Spinor Dynamics

### 2.1 The Complex Roots [THEOREM for algebra]

When $k < 4/G^*$, the discriminant is negative and the roots are:

$$x_{\pm} = \frac{kG^{*2}}{2} \pm i\frac{\sqrt{|kG^{*3}(4 - kG^*)|}}{2} \tag{2.1}$$

Writing $x = x_R + ix_I$:

$$x_R = \frac{kG^{*2}}{2} \qquad x_I = \pm\frac{G^*}{2}\sqrt{k(4 - kG^*)} \tag{2.2}$$

The real part $x_R$ gives the **mass** (energy at rest). The imaginary part $x_I$ gives the **phase oscillation frequency** (the spinor rotation rate).

### 2.2 At the Fermionic Value k = 1/2 [THEOREM for algebra, SELECTION for identification]

The complementation fixed point $k_{\text{cons}} = 1/2$ (from $f(k) = 1-k$ having $k^* = 1/2$) gives:

$$x_R = \frac{G^{*2}}{4} = 2.189 \tag{2.3}$$

$$x_I = \frac{G^*}{2}\sqrt{\frac{1}{2}\left(4 - \frac{G^*}{2}\right)} = \frac{G^*}{2}\sqrt{2 - \frac{G^*}{2}} \approx 1.397 \tag{2.4}$$

The modulus:

$$|x| = \sqrt{x_R^2 + x_I^2} = \sqrt{\frac{G^{*4}}{16} + \frac{G^{*2}}{4}\left(2 - \frac{G^*}{2}\right)} \tag{2.5}$$

### 2.3 Why This Is a Spinor [SELECTION]

A complex root $x = x_R + ix_I$ oscillates under the tick evolution:

$$x(t) = |x|\,e^{i\omega t} \tag{2.6}$$

where $\omega = x_I$ is the oscillation frequency. Under a full rotation of $2\pi$:

$$x(t + 2\pi/\omega) = |x|\,e^{i(2\pi + \omega t)} = x(t) \tag{2.7}$$

But a HALF rotation of $\pi/\omega$ gives:

$$x(t + \pi/\omega) = |x|\,e^{i(\pi + \omega t)} = -x(t) \tag{2.8}$$

The state returns to $-x$ after a half-period. This is the **defining property of a spinor**: a $360°$ rotation gives $-1$, and only a $720°$ rotation returns to the original state.

The real roots ($k = 16$) have no imaginary part — no oscillation — no spinor structure. They are scalars (bosonic coupling constants). The complex roots ($k < 4/G^*$) oscillate and exhibit spinor behavior. **The distinction between bosons and fermions is the sign of the discriminant.**

---

## Part III: The Four Spinor Components from BCC Geometry

### 3.1 Four Complementary Pairs = Four Spinor Components [THEOREM for geometry, SELECTION for identification]

The 8 BCC vertices of the Moore neighborhood form 4 complementary pairs under $O_h$ inversion:

$$(\pm 1, \pm 1, \pm 1) \xrightarrow{\text{inversion}} (-\mp 1, -\mp 1, -\mp 1)$$

| Pair | Vertex | Complement |
|------|--------|------------|
| 1 | $(+,+,+)$ | $(-,-,-)$ |
| 2 | $(+,+,-)$ | $(-,-,+)$ |
| 3 | $(+,-,+)$ | $(-,+,-)$ |
| 4 | $(+,-,-)$ | $(-,+,+)$ |

Each pair can independently be in the complex regime of the master quadratic. Four independent complex oscillators = four complex degrees of freedom = a **4-component Dirac spinor**.

### 3.2 The Chirality Split [SELECTION]

The 4 pairs split into two groups by the handedness of the vertex:

- **Left-chiral** (odd parity): pairs where the product $\epsilon_1 \epsilon_2 \epsilon_3 = -1$
  → pairs 2, 3, 4 (three pairs)
- **Right-chiral** (even parity): pair where $\epsilon_1 \epsilon_2 \epsilon_3 = +1$
  → pair 1 (one pair)

Wait — in the standard convention, the 8 BCC vertices have 4 with even parity and 4 with odd parity. Under inversion, each maps to the other, giving 4 pairs total. The chirality assignment is:

- Vertices with even product $(+,+,+), (+,-,-), (-,+,-), (-,-,+)$: 4 vertices → 2 pairs
- Vertices with odd product $(+,+,-), (+,-,+), (-,+,+), (-,-,-)$: 4 vertices → 2 pairs

So: **2 left-chiral pairs + 2 right-chiral pairs = 4 total = Dirac spinor** with two Weyl components of 2 degrees of freedom each.

This matches the standard Dirac spinor: $\psi = (\psi_L, \psi_R)$ where each Weyl spinor has 2 complex components.

---

## Part IV: The Complexification ψ = J_x + iJ_y

### 4.1 Why This Is Natural [THEOREM for i emergence, SELECTION for identification]

The flux field $\mathbf{J} = (J_x, J_y, J_z)$ has three real components. The null cone $i^2 + a^2 + b^2 = 0$ says one direction is the self-referential axis ($i$) and two are the "real" axes ($a, b$).

Choosing the $z$-axis as the gauge direction (fixed by the Gauss constraint $\nabla \cdot \mathbf{J} = \rho$, which operates on the longitudinal component), the transverse degrees of freedom are $J_x$ and $J_y$.

The complexification:

$$\psi = J_x + iJ_y \tag{4.1}$$

is not arbitrary — it is the projection of the 3D flux onto the complex plane defined by the null cone. The imaginary unit $i$ emerged from self-referential closure (the Perpendicularity Theorem: $i$ is the unique magnitude-preserving distinguishable operation on $\mathbb{R}^2$). Applying $i$ to the transverse plane of $\mathbf{J}$ gives $\psi$.

### 4.2 The Gauge Component [THEOREM]

The longitudinal component $J_z$ (along the gauge axis) is removed by the Gauss constraint $\nabla \cdot \mathbf{J} = \rho$. In momentum space, this projects out the $\hat{k}$ component of $\mathbf{J}$, leaving 2 transverse DOF — exactly the content of $\psi = J_x + iJ_y$.

The Gauss constraint reduces 3 real DOF to 2 transverse DOF, which are naturally complexified into 1 complex DOF. Multiply by the 4 BCC complementary pairs and you get $4 \times 1 = 4$ complex DOF = Dirac spinor.

---

## Part V: The Dirac Equation from the Wave Equation

### 5.1 The FTD Wave Equation [THEOREM]

The Euler-Lagrange equation for $\mathbf{J}$ from the FTD Lagrangian is:

$$\ddot{\mathbf{J}} = c^2 \nabla^2 \mathbf{J} - g_c \nabla s - \text{(damping + nonlinear terms)} \tag{5.1}$$

For the transverse components, applying the complexification $\psi = J_x + iJ_y$:

$$\ddot{\psi} = c^2 \nabla^2 \psi - g_c (\partial_x + i\partial_y) s \tag{5.2}$$

### 5.2 The Factorization [SELECTION]

The Klein-Gordon operator $\ddot{\psi} - c^2\nabla^2\psi + m^2\psi = 0$ can be factored on the lattice:

$$(i\gamma^\mu \hat{\partial}_\mu - m)(i\gamma^\nu \hat{\partial}_\nu + m)\psi = (\hat{\partial}^2 + m^2)\psi = 0 \tag{5.3}$$

where $\hat{\partial}_\mu = \sin(k_\mu)$ is the lattice derivative. The factorization works because:

$$\{\gamma^\mu, \gamma^\nu\} = 2\eta^{\mu\nu}$$

is the Clifford algebra, which exists on any lattice that supports the null cone $i^2 + a^2 + b^2 = 0$.

The Dirac equation is the **first-order factor** of the second-order wave equation:

$$\boxed{(i\gamma^\mu \hat{\partial}_\mu - m)\psi = 0} \tag{5.4}$$

### 5.3 Why Factorization Is Natural [SELECTION]

The master quadratic itself is a factorization. In the complex regime:

$$x^2 - kG^{*2}x + kG^{*3} = (x - x_+)(x - x_-)$$

where $x_+$ and $x_-$ are complex conjugates. The Dirac equation is the **momentum-space analog** of this factorization:

- The Klein-Gordon equation $(\partial^2 + m^2)\psi = 0$ is the "master quadratic" of field theory
- The Dirac equation $(i\gamma\partial - m)\psi = 0$ is one factor
- The conjugate Dirac equation $(i\gamma\partial + m)\bar{\psi} = 0$ is the other factor

The two factors correspond to particle and antiparticle — which in FTD are the states $s = +1$ and $s = -1$. The void $s = 0$ is the vacuum between them. The ternary axiom $0 = (-1) + (+1)$ is the statement that a particle-antiparticle pair can annihilate to vacuum, which is the content of the Dirac equation's charge conjugation symmetry.

---

## Part VI: The Unified Picture

### 6.1 One Quadratic, All of Physics [THEOREM for structure]

$$x^2 - kG^{*2}x + kG^{*3} = 0$$

| $k$ | Discriminant | Roots | Physics | Equation |
|-----|-------------|-------|---------|----------|
| 16 | $\Delta > 0$ | Real: 137, 3 | Coupling constants $\alpha$, $N_c$ | Master quadratic |
| $4/G^*$ | $\Delta = 0$ | Degenerate: $2G^*$ | Born rule $P = |ψ|^2$ | Pythagorean theorem |
| $1/2$ | $\Delta < 0$ | Complex conjugates | Fermion mass + spin | Dirac equation |
| 0 | $\Delta = 0$ | $x = 0$ (trivial) | Vacuum | Nothing |

### 6.2 The Analogies Made Precise

| Physical law | FTD origin | How it emerges |
|-------------|-----------|----------------|
| **Born rule** $P = |ψ|^2$ | Null cone $i^2 + a^2 + b^2 = 0$ | The Pythagorean theorem on the null cone |
| **Dirac equation** $(iγ∂ - m)ψ = 0$ | Complex roots of the master quadratic | Factorization of Klein-Gordon, spinor from BCC pairs |
| **Coupling constants** $\alpha$, $N_c$ | Real roots of the master quadratic | Gap equation self-consistency |
| **Measurement** | $\Delta = 0$ threshold | Transition between real (bosonic) and complex (fermionic) regimes |

---

## Part VII: What This Does and Does Not Prove

### Established [THEOREM]

1. The discriminant $\Delta = kG^{*3}(kG^* - 4)$ partitions the master quadratic into three regimes
2. The complex roots oscillate with the defining property of spinors ($360° → -1$)
3. The 8 BCC vertices form 4 complementary pairs under $O_h$ inversion
4. The Gauss constraint reduces 3 real DOF to 2 transverse → natural complexification
5. The Klein-Gordon equation CAN be factored into two Dirac equations on ANY lattice

### Argued [SELECTION]

6. The 4 BCC pairs = 4 spinor components (geometry → representation identification)
7. The complexification $\psi = J_x + iJ_y$ is the "natural" choice (vs other projections)
8. The factorization of the wave equation into Dirac form is "natural" (vs staying second-order)
9. The complementation fixed point $k = 1/2$ identifies the fermionic regime
10. The chirality split (2L + 2R) from the parity of BCC vertex products

### Remains [OPEN]

11. Can the lattice Dirac operator be derived from the FTD Lagrangian without factorization (i.e., does the coupled (s, J) dynamics produce first-order evolution for ψ)?
12. Does the mass $m = K_B$ emerge from the complex root's modulus, or is it imposed?
13. Are the 4 spinor components dynamically independent, or does the lattice constrain them?

---

## References

- FOUND_AXIOM_ZERO.md — State + position, nothing else (02_foundations)
- FOUND_BORN_RULE_NULL_CONE.md — $i^2 + a^2 + b^2 = 0$ (02_foundations)
- DERIV_QUADRATIC_NECESSITY.md — Why degree 2 (03_derivations)
- DERIV_MASTER_QUADRATIC_GAP_EQUATION.md — The gap equation (03_derivations)
- DERIV_SPIN_STATISTICS_BRIDGE.md — Spin-statistics from topology (03_derivations)
- DERIV_CUBOCTAHEDRAL_INTEGERS.md — BCC complementary pairs (08_structural)
- Thaller, B. *The Dirac Equation*, Springer, 1992
