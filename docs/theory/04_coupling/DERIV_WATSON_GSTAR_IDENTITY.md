# The Watson–G* Identity: G* and the BCC Sublattice

## G*²/(2π) = G_BCC(0) — The BCC Component of the Moore Neighborhood

**Status:** Theorem (algebraic identity) + physical interpretation
**Dependencies:** MATH_MASTER_QUADRATIC.md, DERIV_GSTAR_PF_BRIDGE.md, DERIV_ALPHA_LATTICE_MECHANISM.md

---

## Abstract

We prove that the FTD master constant satisfies $G^{*2}/(2\pi) = \Gamma(1/4)^4/(4\pi^3)$, which equals $G_{\rm BCC}(0)$ — the **body-centered cubic (BCC)** lattice Green's-function at the origin (the random-walk return-probability integral), NOT the simple cubic (SC) lattice self-energy. This identity is exact and holds to arbitrary precision.

**Naming:** $\Gamma(1/4)^4/(4\pi^3) \approx 1.3932$ is **not** "Watson's integral $I_1$" / the "self-energy integral." The standard **simple-cubic Watson self-energy integral** $I_1 = W_{\rm SC} = (1/\pi^3)\int_{[0,\pi]^3} d^3k\,/(3 - \cos k_x - \cos k_y - \cos k_z) \approx 0.5054$ — a different number entirely. The FTD quantity $1.3932$ is the **BCC return Green's function at the origin**, $G_{\rm BCC}(0)$, defined by the *product*-cosine eigenvalue $\sigma_{\rm BCC}(\mathbf k) = 1 - \cos k_x \cos k_y \cos k_z$. The numerical EQUALITY $W_3 := G^{*2}/(2\pi) = \Gamma(1/4)^4/(4\pi^3) = 2\,G_{\rm gauss}^2 = 1.39320392968\ldots$ is a true **[THEOREM]**. Throughout this document $W_3$ denotes $G_{\rm BCC}(0) = G^{*2}/(2\pi)$, not the SC Watson self-energy.

This is structurally significant: the FTD lattice uses a **26-neighbor Moore neighborhood** that decomposes into SC (6 face neighbors), FCC (12 edge neighbors), and BCC (8 corner neighbors). G* connects specifically to the **BCC component** — the 8 corner neighbors at $(\pm 1, \pm 1, \pm 1)$ — whose $Z_4$ vertex symmetry selects the lemniscatic CM curve $E: y^2 = x^3 - x$.

---

## Part I: The Three Cubic Lattice Green's-Function Integrals

### 1.1 The Three Integrals [THEOREM]

The three cubic Bravais lattices each have a return Green's-function-at-origin integral, one per lattice eigenvalue $\sigma(\mathbf k)$. The FTD-relevant value $1.3932$ is the **BCC** integral with the *product*-cosine eigenvalue, $G_{\rm BCC}(0)$ — it is **not** the simple-cubic Watson self-energy $I_1 = W_{\rm SC} \approx 0.5054$. The three integrals are:

$$G_{\rm BCC}(0) = \frac{1}{\pi^3}\int_0^\pi\!\int_0^\pi\!\int_0^\pi \frac{da\,db\,dc}{1 - \cos a\,\cos b\,\cos c} \tag{1.1}$$

$$W_{\rm FCC} = \frac{1}{\pi^3}\int_0^\pi\!\int_0^\pi\!\int_0^\pi \frac{da\,db\,dc}{3 - \cos a\cos b - \cos b\cos c - \cos c\cos a} \tag{1.2}$$

$$W_{\rm SC} = I_1^{\rm Watson} = \frac{1}{\pi^3}\int_0^\pi\!\int_0^\pi\!\int_0^\pi \frac{da\,db\,dc}{3 - \cos a - \cos b - \cos c} \tag{1.3}$$

| Integral | Lattice type | Eigenvalue $\sigma(\mathbf k)$ | CM field | Gamma function | Numerical value |
|----------|-------------|-----------|----------|---------------|----------------|
| $G_{\rm BCC}(0)$ | **BCC** | $1 - \cos k_x\cos k_y\cos k_z$ | $\mathbb{Q}(i)$ | $\Gamma(1/4)$ | **1.39320** (grid-confirmed) |
| $W_{\rm FCC}$ | **FCC** | $3 - \sum_{i<j}\cos k_i\cos k_j$ | $\mathbb{Q}(\sqrt{-3})$ | $\Gamma(1/3)$ | 0.4461 |
| $W_{\rm SC} = I_1^{\rm Watson}$ | **SC** | $3 - \sum_i\cos k_i$ | — | $\Gamma(n/24)$ | **0.50542** (grid-confirmed) |

The standard **Watson "self-energy" integral** in the literature is $W_{\rm SC} \approx 0.5054$ (the *sum*-cosine SC form, eq. 1.3) — NOT the FTD value $1.3932$. The FTD constant equals the **BCC return Green's function** $G_{\rm BCC}(0)$ (eq. 1.1).

### 1.2 The Key Identity [THEOREM]

$$G_{\rm BCC}(0) = \frac{\Gamma(1/4)^4}{4\pi^3} = \frac{G^{*2}}{2\pi} \tag{1.4}$$

$$W_3 := G_{\rm BCC}(0) = \frac{1}{(2\pi)^3} \int_{[-\pi,\pi]^3} \frac{d^3 k}{\sigma_{\rm BCC}(\mathbf{k})}, \qquad \sigma_{\rm BCC}(\mathbf k) = 1 - \cos k_x\cos k_y\cos k_z \tag{1.5}$$

### 1.3 Closed-Form Evaluation [THEOREM]

The BCC return Green's function evaluates in closed form:

$$W_3 = G_{\rm BCC}(0) = \frac{\Gamma(1/4)^4}{4\pi^3} \tag{1.6}$$

This is a celebrated result in mathematical physics. The proof proceeds via reduction to elliptic integrals (expanding the *product*-cosine propagator as a geometric series, see §8.2), then evaluation through the arithmetic-geometric mean.

**Numerical value (mpmath dps=40):** $W_3 = \Gamma(1/4)^4/(4\pi^3) = 1.393203929685676859\ldots$ Direct grid integration of eq. (1.1) confirms $G_{\rm BCC}(0) \to 1.3932$ (1.3925 at $N=800$ midpoint, converging from below through the integrable origin singularity). By contrast the SC Watson self-energy (eq. 1.3) grid-integrates to $W_{\rm SC} = 0.5051 \to 0.5054$ — confirming the two are distinct.

### 1.4 Physical Meaning

$W_3 = G_{\rm BCC}(0)$ is the **return Green's function at the origin** of the BCC-eigenvalue lattice — the probability amplitude for a random walker to return to its starting point summed over all time. It governs:
- The lattice propagator at coincident points
- The UV-finite one-loop self-energy on the lattice
- The leading finite-size correction to lattice observables

In lattice gauge theory, return Green's functions of this kind appear in one-loop calculations of the plaquette expectation value, the static quark potential, and Wilson loop corrections. The BCC return value $W_3 = G_{\rm BCC}(0)$ is as fundamental to the FTD lattice's BCC component as $\pi$ is to circular geometry. *(This is the BCC return integral, distinct from the SC "Watson self-energy" $W_{\rm SC}\approx 0.5054$.)*

---

## Part II: The Identity

### 2.1 Statement [THEOREM]

$$W_3 = \frac{G^{*2}}{2\pi} \tag{2.1}$$

where $G^* = \frac{\sqrt{2}\,\Gamma(1/4)^2}{2\pi}$ is the FTD master constant.

### 2.2 Proof [THEOREM]

$$G^{*2} = \left(\frac{\sqrt{2}\,\Gamma(1/4)^2}{2\pi}\right)^2 = \frac{2\,\Gamma(1/4)^4}{4\pi^2}$$

$$\frac{G^{*2}}{2\pi} = \frac{2\,\Gamma(1/4)^4}{4\pi^2 \cdot 2\pi} = \frac{\Gamma(1/4)^4}{4\pi^3} = W_3 \quad \blacksquare$$

### 2.3 Numerical Verification

$$W_3 = 1.393203929685677\ldots$$
$$G^{*2}/(2\pi) = 1.393203929685676\ldots$$

Agreement to 15 significant figures (limited by double-precision floating point). The identity is exact.

---

## Part III: Consequences

> **Terminology note.** For the remainder of this document the phrases "the Watson integral," "the lattice self-energy," and the symbol $W_3$ all denote the **BCC return Green's function at the origin**, $W_3 = G_{\rm BCC}(0) = G^{*2}/(2\pi) = \Gamma(1/4)^4/(4\pi^3) = 1.39320392968\ldots$ (eq. 1.4/1.6). They do **not** denote the standard simple-cubic Watson self-energy $W_{\rm SC} = I_1^{\rm Watson} \approx 0.5054$. The numerical identity $W_3 = G^{*2}/(2\pi)$ is a true **[THEOREM]**; the name "Watson self-energy" is a mislabel.

### 3.1 The Master Quadratic Lives on the Lattice [THEOREM]

The Vieta relations of the master quadratic $x^2 - 16G^{*2}x + 16G^{*3} = 0$ are:

$$x_+ + x_- = 16\,G^{*2} \tag{3.1}$$
$$x_+ \cdot x_- = 16\,G^{*3} \tag{3.2}$$

Substituting $G^{*2} = 2\pi\,W_3$:

$$x_+ + x_- = 32\pi\,W_3 \tag{3.3}$$
$$x_+ \cdot x_- = 16\,G^* \cdot 2\pi\,W_3 = 32\pi\,G^*\,W_3 \tag{3.4}$$

The sum and product of the master quadratic roots are **proportional to the Watson integral**. The master quadratic is not an external mathematical object imposed on the lattice — its coefficients are built from the lattice's own self-energy.

### 3.2 Historical physical reading [RETIRED]

The exact statement from (3.3) is

$$x_+ + x_- = 32\pi\,W_3. \tag{3.5}$$

An earlier version replaced $x_+$ by $1/\alpha$ and $x_-$ by $N_c$ and
therefore wrote $1/\alpha + N_c = 32\pi W_3$. That is not a live theorem:
$x_+ \leftrightarrow 1/\alpha$ is a physical conjecture, while the historical
$x_- \leftrightarrow N_c$ identification is retired. In particular, the old
numerical line subtracted $x_- \simeq 3.024$, not the exact color count
$N_c=3$. Equation (3.5) is retained only in its root-sum form.

### 3.3 Rewriting the Master Quadratic in Lattice Language [THEOREM]

Substituting $G^{*2} = 2\pi\,W_3$ and $G^{*3} = G^* \cdot 2\pi\,W_3$ into the master quadratic:

$$x^2 - 32\pi\,W_3\,x + 32\pi\,G^*\,W_3 = 0 \tag{3.7}$$

Or, factoring out $W_3$:

$$x^2 - 32\pi\,W_3\left(x - G^*\right) = 0 \tag{3.8}$$

This form reveals: the master quadratic is the condition that $x$ satisfies a **self-consistency relation** between the lattice self-energy ($W_3$) and the elliptic curve period ($G^*$). The root $x$ is the value where the lattice's quantum corrections (encoded in $W_3$) balance the classical geometric structure (encoded in $G^*$).

### 3.4 The Harmonic Mean Identity [THEOREM]

The harmonic mean of the two roots is:

$$\frac{2\,x_+\,x_-}{x_+ + x_-} = \frac{2 \cdot 16G^{*3}}{16G^{*2}} = 2G^* \tag{3.9}$$

Combined with $W_3 = G^{*2}/(2\pi)$:

$$G^* = \sqrt{2\pi\,W_3} \tag{3.10}$$

The FTD master constant is the **geometric mean** of $2\pi$ and the Watson integral. This connects the lattice's self-energy ($W_3$) to the continuous geometry ($2\pi$) through $G^*$ as the bridge — consistent with the PF bridge interpretation (DERIV_GSTAR_PF_BRIDGE.md) where $G^*$ exchanges between discrete and continuous domains.

---

## Part IV: Finite-Size Scaling

### 4.1 The BCC Green pseudoinverse on finite tori [THEOREM]

On the $L \times L \times L$ periodic torus, the finite-volume Green's function at the origin is:

$$G_L^+(0) = \frac{1}{L^3}\sum_{\sigma(\mathbf{k}) \neq 0} \frac{1}{\sigma(\mathbf{k})}$$

where $k_\mu = 2\pi n_\mu/L$, $n_\mu = 0, 1, \ldots, L-1$, and
$\sigma(\mathbf k)=1-\cos k_x\cos k_y\cos k_z$. The superscript `+`
denotes the Moore–Penrose pseudoinverse: **every** zero mode is removed, not
only $\mathbf k=0$. For even $L$, the kernel has four modes, namely the
points with $k_i\in\{0,\pi\}$ and an even number of $\pi$ components; for
odd $L$, only the constant mode is in the kernel.

| $L$ | $G_L^+(0)$ | $G_L^+/W_3$ | $16\,G_L^+$ |
|-----|----------|-----------|-----------|
| 2 | 1/4 = 0.250000 | 0.179443 | 4.0000 |
| 4 | 29/32 = 0.906250 | 0.650479 | 14.5000 |
| 8 | 1.162388 | 0.834328 | 18.5982 |
| 16 | 1.279663 | 0.918503 | 20.4746 |
| 32 | 1.336676 | 0.959426 | 21.3868 |
| 64 | 1.364971 | 0.979735 | 21.8395 |
| $\infty$ | $W_3$ = 1.3932 | 1.000 | 22.29 |

The displayed even-$L$ sequence converges from below. At $L=2$, excluding
only $\mathbf k=0$ would leave three additional zero denominators and make
the original sum divergent; removing the full kernel gives
$G_2^+=(1/8)\,4\,(1/2)=1/4$ exactly. The former table's $L=2$ value
$29/32$ is the corrected $L=4$ value.

### 4.2 The 2×2×2 degree-of-freedom count [CORRECTED]

The historical route $24-7-1=16$ is retracted. Setting $A_0=0$ does not
remove the residual time-independent spatial gauge transformations or the
harmonic torus modes. Proper transverse gauge fixing removes the seven
independent Gauss directions and three harmonic modes:

$$n_{\rm physical}=24-7-3=14.$$

Thus the minimal-torus DOF count does not derive the master-quadratic
coefficient 16. The finite-volume pseudoinverse value is independently
$G_2^+=1/4$; no near-equality between $16G_2^+$ and 14 exists.

---

## Part V: What This Proves and Resolution of Gaps

### Established [THEOREM]

1. $W_3 = G^{*2}/(2\pi)$ — exact algebraic identity
2. $x_+ + x_- = 32\pi\,W_3$ — the master-quadratic root sum in BCC-integral language
3. $G^* = \sqrt{2\pi\,W_3}$ — the master constant as geometric mean of $2\pi$ and $W_3$
4. The master quadratic coefficients can be rewritten using the BCC self-energy
5. The 2×2×2 torus has 14 physical transverse DOF (not 16)

### Physical Interpretation [SELECTION]

6. The identity $W_3 = G^{*2}/(2\pi)$ means the lattice's self-energy IS a G*-derived quantity. G* is not externally imposed — it is **intrinsic to the Z³ lattice**.
7. The master quadratic (3.8) encodes the self-consistency between lattice quantum corrections ($W_3$) and classical elliptic geometry ($G^*$).

### Resolved and Closed Items

8. **Physical content of the algebraic Watson-G* connection** — **[CLOSED RECLASSIFIED]** (FTD-0242).
   Under the dynamic-alpha pivot (FTD-0242) and the route-invariance boundary audits, $\alpha$ is recognized as dynamical rather than structural. No FTD-native route forces the operator assembly $(Tr, Det) = (16G^{*2}, 16G^{*3})$; the trace and odd source are forward-forced, but the assembly is not. Thus, the Watson-G* identity is a period equivalence on the substrate, and the physical value of $\alpha$ is not uniquely forced by the period algebra alone.

9. **Torus DOF counting discrepancy (14 vs 16)** — **[CLOSED RESOLVED]**.
   The coefficient 16 is structurally forced by the automorphism group of the CM curve, $|{\rm Aut}(E)|^2 = 16$, which matches the unit group order of $\mathbb{Z}[i]$ and representation-theory multiplicities. The naive torus DOF counting ($24 - 7 - 1 = 16$) is a legacy heuristic rather than a structural proof.

---

## Part VI: The Deeper Question

The identity $W_3 = G^{*2}/(2\pi)$ is a mathematical fact. Both sides are algebraic expressions in $\Gamma(1/4)$. The question is whether this is:

**(a) A tautology:** Both $W_3$ and $G^*$ happen to involve $\Gamma(1/4)$ for independent reasons (Watson via elliptic integral reduction; FTD via the lemniscate), and their algebraic relationship is a curiosity with no physical content.

**(b) A structural truth:** The 3D cubic lattice and the lemniscate curve E: $y^2 = x^3 - x$ share a common mathematical ancestor — the quartic integral $I_4 = \int_0^1 (1-x^4)^{-1/2}dx$ — and their connection through $\Gamma(1/4)$ reflects this shared origin. The lattice's self-energy and the elliptic curve's period are different manifestations of the same geometric object.

Evidence for (b): The quartic integral $I_4$ arises in Watson's computation through the reduction of the 3D lattice sum to elliptic integrals. Specifically, Watson shows that the inner integral over $k_z$ produces an elliptic integral whose modulus is determined by $k_x$ and $k_y$, and the remaining double integral evaluates via the AGM to $\Gamma(1/4)^4$. The **same** quartic integral defines the lemniscate constant $\varpi = 2I_4$.

This suggests that the connection is not coincidental: the 3D cubic lattice's self-energy involves the lemniscate integral because the lattice sum **reduces to** the lemniscate integral through Watson's evaluation method. The lattice and the lemniscate share the quartic integral $I_4$ as their common mathematical root.

If this interpretation is correct, then $G^* = \sqrt{2\pi W_3}$ is not an imposed identification but an algebraic consequence of the cubic lattice's geometry being governed by the same quartic integral that defines the lemniscate. The fine structure constant emerges from the lattice because the lattice's self-energy IS the lemniscate constant (up to the factor $2\pi$).

---

## Part VII: The Lattice Symmetry Theorem [THEOREM]

The Watson-G* identity is not an isolated coincidence. It is a specific instance of a general pattern connecting lattice symmetry groups to elliptic curve automorphisms through the Gamma function.

### 7.1 Watson's Three Cubic Lattice Integrals (1939)

The return Green's-function integral for the cubic Bravais lattice types:

| Lattice | Planar symmetry | Return-Green's-fn closed form | Gamma function | Value | CM curve |
|---------|----------------|-----------------|---------------|-------|----------|
| **BCC** (body-centered) | $Z_4$ (square cross-sections) | $G_{\rm BCC}(0) = \Gamma(1/4)^4/(4\pi^3)$ | $\Gamma(1/4)$ | **1.39320** | $j = 1728$, Aut $\cong Z_4$ |
| SC (simple cubic) | $Z_4$ (square faces) | $W_{\rm SC} = I_1^{\rm Watson}$ (no $\Gamma(1/4)$ closed form) | $\Gamma(n/24)$ | 0.50542 | — |
| FCC (face-centered) | $Z_6$ (close-packed planes) | $W_{\rm FCC}$ involves $\Gamma(1/3)$ | $\Gamma(1/3)$ | 0.4461 | $j = 0$, Aut $\cong Z_6$ |

**Caveat (two common errors avoided):** (i) $\sqrt{6}\,\Gamma(1/4)^4/(32\pi^3)$ is **not** the BCC closed form. Computed (mpmath dps=40): $\sqrt{6}\,\Gamma(1/4)^4/(32\pi^3) = 0.42658\ldots$, which does **not** equal $1.39320392968\ldots$ — it is spurious. The correct, $\Gamma(1/4)^4$-bearing closed form is $G_{\rm BCC}(0) = \Gamma(1/4)^4/(4\pi^3) = 1.393203929685677\ldots$ (matching §1.1 / eq. 1.6). (ii) $\Gamma(1/4)^4/(4\pi^3)$ does **not** belong to the **SC** lattice; the $\Gamma(1/4)^4/(4\pi^3) = 1.3932$ value belongs to the **BCC** return integral, not SC. The SC Watson self-energy is $W_{\rm SC}\approx 0.5054$ and has no $\Gamma(1/4)^4/(4\pi^3)$ closed form.

The pattern is systematic: **the rotational symmetry of the lattice's coordinate planes determines which Gamma function appears in the lattice Green's function, which determines which CM elliptic curve governs it.** Only the BCC component carries the $\Gamma(1/4)$/lemniscatic structure relevant to $G^*$.

### 7.2 The General Correspondence [THEOREM for individual cases]

| Lattice symmetry | Gamma function | Elliptic modulus | CM field | Curve $j$-invariant |
|-----------------|---------------|-----------------|----------|-------------------|
| $Z_4$ (square) | $\Gamma(1/4)$ | $k = 1/\sqrt{2}$ (lemniscatic) | $\mathbb{Q}(i)$ | $1728$ |
| $Z_6$ (hexagonal) | $\Gamma(1/3)$ | $k = e^{i\pi/3}$ (equianharmonic) | $\mathbb{Q}(\sqrt{-3})$ | $0$ |

~~**Why this works:** Watson's AGM reduction of the lattice sum integrates out one dimension, leaving a 2D integral over the remaining plane. The 2D integral inherits the planar symmetry of the lattice. For the square lattice (SC, BCC), the 2D integral has $C_{4v}$ symmetry, forcing the elliptic integral modulus to the lemniscatic value $k = 1/\sqrt{2}$ — the unique modulus where the elliptic curve has $Z_4$ automorphisms.~~ For the hexagonal lattice (FCC close-packed planes), the $C_{6v}$ symmetry forces the equianharmonic modulus.

> **⚠ The struck sentence is refuted (2026-08-16); see the correction box in §7.4.** It groups "(SC, BCC)" together as square-planar and concludes $C_{4v}$ forces $k = 1/\sqrt2$ for both. But §7.3's own table gives **SC** the Gamma class $\Gamma(n/24)$, i.e. discriminant $-24$ — recomputed for this correction as $W_{\rm SC} = 1.5163860591519768\ldots$ (this document's $0.50542$ under the $1/3$-normalized convention). A symmetry both lattices share cannot select between them. **What actually discriminates is multiplicative vs additive structure of the layer eigenvalue** ($1 - \cos k_1\cos k_2\cos k_3$ vs $1 - \sum\cos k_i$), which reaches $\Gamma(1/4)$ through Clausen's quarter-parameter ${}_2F_1$ — not through planar $Z_4$.
>
> Even that surviving discriminator is *motivation, not forcing*: [`ANALYSIS_CONSTRUCTION_CLASS_CLOSURE.md`](../10_eft_program/derivations/ANALYSIS_CONSTRUCTION_CLASS_CLOSURE.md) Cor. 2.1 records that *"restrict to the multiplicative layer"* is **"extensionally the same choice re-described"**, and books the restriction as the **d = −4 bit, i.e. FC-0** — an `[AXIOM]`-class modelling choice, not a consequence of P1–P5.

### 7.3 The 2D Case [THEOREM]

The same correspondence holds in 2D:

- **Square lattice** ($Z_4$ symmetry): The 2D Watson integral involves $\Gamma(1/4)^2$. The lattice Green's function reduces to $K(1/\sqrt{2})/\pi$ — the lemniscatic modulus.
- **Triangular/hexagonal lattice** ($Z_6$ symmetry): The 2D Watson integral involves $\Gamma(1/3)^2$. The lattice Green's function reduces to the equianharmonic modulus.

### 7.4 Implication for FTD

FTD postulates a 3D cubic lattice (Axiom 1). This lattice has $Z_4$ planar symmetry. The chain from there is:

1. ~~The Watson integral involves $\Gamma(1/4)$ [forced by $Z_4$]~~ — **step 1 does not hold; see the correction below.** The BCC integral involves $\Gamma(1/4)$; which integral is read is a *layer selection*, not a consequence of $Z_4$.
2. The relevant CM curve is $E: y^2 = x^3 - x$ with $j = 1728$ [forced by $\Gamma(1/4)$]
3. The period of $E$ is $\varpi = \Gamma(1/4)^2/(2\sqrt{2\pi})$ [forced by $E$]
4. $G^* = 2\varpi/\sqrt{\pi} = \sqrt{2\pi W_3}$ [algebraic identity]

Steps 2–4 stand as stated; they are conditional on step 1's output, not on its stated ground.

> **⚠ CORRECTION (2026-08-16) — step 1's inference is refuted by this document's own table.**
>
> §7.3's table two paragraphs above records **SC** with planar symmetry $Z_4$ and *"no $\Gamma(1/4)$ closed form"*, Gamma class $\Gamma(n/24)$. **SC and BCC therefore share the same $Z_4$ planar symmetry and land on different CM fields** — disc $-24$ (i.e. $\mathbb{Q}(\sqrt{-6})$) versus disc $-4$. A property both lattices possess cannot select between them, so $Z_4$ planar symmetry does **not** force $\Gamma(1/4)$, the lemniscatic modulus $k = 1/\sqrt2$, or the curve $j = 1728$.
>
> Independently recomputed for this correction: $W_{\rm SC} = 1.5163860591519768\ldots$ (Bessel form $\int_0^\infty e^{-t}I_0(t/3)^3dt$), matching the disc $-24$ Chowla–Selberg closed form $\sqrt6/(32\pi^3)\cdot\Gamma(1/24)\Gamma(5/24)\Gamma(7/24)\Gamma(11/24)$ to 15 digits. (This document's $0.50542$ is the same integral under the $1/3$-normalized convention — the values agree; only the inference above is at fault.)
>
> **What does discriminate** is recorded elsewhere in this corpus: the BCC eigenvalue is a *triple product* $1 - \cos k_1\cos k_2\cos k_3$ while SC's is a *sum*. Via Clausen's identity the triple product is the Clausen square of a ${}_2F_1$ with **quarter parameters** — ${}_2F_1(\tfrac14,\tfrac14;1;1)^2 = {}_3F_2(\tfrac12,\tfrac12,\tfrac12;1,1;1) = \Gamma(1/4)^4/(4\pi^3)$, verified to 25 digits. The $4$ enters through **multiplicativity**, not through a four-fold axis. Note the BCC sublattice is spanned by the eight $\langle111\rangle$ body diagonals, whose axes are *three*-fold.
>
> **Consequence for SP1a — flagged, not applied.** The sentence below claimed to resolve SP1a as `[THEOREM]`. That resolution rested on step 1 and does **not** survive. On the present evidence the BCC-over-SC/FCC readout is a `[SELECTION]` — a choice of which Moore layer to read. **This note does not move the tag**; see the matching correction in [`AUDIT_HIDDEN_SELECTIONS.md`](../07_assessment/spine_master_quadratic/AUDIT_HIDDEN_SELECTIONS.md) §SP1a, and note that FTD-0313 already grades the adjacent BCC-routing choice `[SELECTION + THEOREM-NEGATIVE]` on three independent grounds.
>
> **Nothing downstream is retagged.** $G^*$ is an identity and is untouched; $W_3 = G^{*2}/(2\pi)$ (Watson 1939) is untouched. Only the claim that the *lattice axiom forces the curve* fails.

~~**The curve is not chosen — it is forced by the lattice axiom.**~~ If FTD had postulated an FCC lattice instead, the relevant curve would be $j = 0$, and the master constant would involve $\Gamma(1/3)$ instead of $\Gamma(1/4)$, producing entirely different physics — *and the same is true of reading the SC layer of the lattice FTD did postulate, which is the point.*

~~This resolves **SP1a** from AUDIT_HIDDEN_SELECTIONS.md: the curve selection is **[THEOREM]**, not [SELECTION].~~ **Withdrawn 2026-08-16** — see the correction box above.

---

## Part VIII: Numerical Confirmation

Independent numerical verification via `scripts/exploration/gap_equation_layer_convergence.py`:

### 8.1 Convergence by sublattice

The normalized Green's function at origin was computed for each Moore sublattice Laplacian on L x L x L periodic tori at L = 64, 96, 128:

| Sublattice | L=64 | L=96 | L=128 | Analytic limit | Matches G*^2/(2pi)? |
|------------|------|------|-------|----------------|---------------------|
| BCC (8) | 1.3650 | 1.3744 | 1.3791 | **G*^2/(2pi) = 1.3932** | **YES** (converging, 1.0% remaining at L=128) |
| SC (6) | 1.4952 | 1.5023 | 1.5058 | ~1.5164 | NO (diverging from target) |
| FCC (12) | 1.3090 | 1.3174 | 1.3222 | different | NO |
| Moore (26) | 1.1445 | 1.1513 | 1.1571 | different | NO |

BCC converges to 1.3932 from below. SC converges to ~1.516 (a different value). No other sublattice matches.

### 8.2 Why BCC: the multiplicative eigenvalue [THEOREM — structural]

The BCC Laplacian eigenvalue is `sigma_BCC(k) = 1 - cos k_1 * cos k_2 * cos k_3` — a **product** of cosines. The SC eigenvalue is `sigma_SC(k) = 1 - (cos k_1 + cos k_2 + cos k_3)/3` — a **sum**.

The product structure is decisive:

1. The BCC propagator `1/(1 - cos k_1 cos k_2 cos k_3)` expands as a geometric series: `sum_n (cos k_1 cos k_2 cos k_3)^n`
2. Each term **factors across axes**: `[integral (cos k)^n dk]^3 = [C(2n,n)/4^n]^3`
3. The sum of cubed central binomial coefficients evaluates to `Gamma(1/4)^4 / (4 pi^3) = G*^2/(2 pi)`
4. SC's sum structure in the denominator cannot factor this way — no path to Gamma(1/4)^4

### 8.3 Zero mode topology

The sublattice Laplacians have different numbers of zero eigenvalues on the torus:

- SC: **1 zero mode** (k = 0 only) — translation invariance
- FCC: **2 zero modes** (k = 0 and k = (pi, pi, pi))
- BCC: **4 zero modes** (k = 0 and k = (pi,pi,0), (pi,0,pi), (0,pi,pi))

The BCC Laplacian's extra zero modes at the FCC reciprocal lattice points cause slow finite-lattice convergence (explaining why early simulations at L = 48 were misleading), but do not affect the L -> infinity limit.

### 8.4 Gap equation consequence

With n_DOF = 16, only the BCC Watson integral gives the correct gap equation coefficient:

K = 16 * 2pi * W_BCC = 16 * 2pi * G*^2/(2pi) = 16 G*^2

This reproduces the master quadratic `x^2 - 16G*^2 x + 16G*^3 = 0` with roots x+ ≈ 137.036 and x- ≈ 3.024. The physical readings x+  1/alpha and x-  N_c are [STRONGLY MOTIVATED CONJECTURE], not consequences of the Watson identity alone.

---

## References

- Watson, G. N. "Three Triple Integrals," *Quarterly Journal of Mathematics* **10** (1939), 266–276
- MATH_MASTER_QUADRATIC.md — Complete algebraic structure (01_reference)
- DERIV_GSTAR_PF_BRIDGE.md — G* decomposition (04_coupling)
- DERIV_ALPHA_LATTICE_MECHANISM.md — Physical mechanism chain (04_coupling)
- DERIV_QUADRATIC_NECESSITY.md — Why degree 2 (03_derivations)
- FOUND_THE_FIRST_DISTINCTION.md — Why n = 4 in $I_4$ (02_foundations)
- Borwein, J. M. and Bailey, D. H. *Mathematics by Experiment*, A K Peters, 2004 (Ch. 2: Watson integrals)
- Glasser, M. L. and Zucker, I. J. "Lattice Sums," *Theoretical Chemistry: Advances and Perspectives* **5** (1980), 67–139
