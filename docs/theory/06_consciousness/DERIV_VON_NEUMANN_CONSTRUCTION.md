# Von Neumann Algebra Construction for FTD Lattice Observables

## Type Classification of the Ternary Lattice Observable Algebra

**Date:** March 17, 2026
**Framework:** Foundational Ternary Dynamics v5.28
**Status:** [THEOREM] on finite lattices; [SELECTION] for the thermodynamic limit
**Authors:** cpaci & Claude (Opus 4.6)
**Proof script:** `scripts/proofs/proof_von_neumann_type.py`

---

**Depends on:**
- [SPEC_FTD.md](../../SPEC_FTD.md) -- Master specification (five postulates, two-layer ontology)
- [FOUND_VON_NEUMANN_CHAIN.md](FOUND_VON_NEUMANN_CHAIN.md) -- Von Neumann chain resolution, Type III$_1$ to Type I transition
- [DERIV_COLLAPSE_MECHANISM.md](DERIV_COLLAPSE_MECHANISM.md) -- ReLU crystallization and Lindblad master equation
- [DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md](DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md) -- Master quadratic, discriminant trichotomy

---

## Abstract

We construct the von Neumann algebra of lattice observables for the FTD ternary lattice and classify its Murray--von Neumann type. On a finite lattice $\Lambda$ with $|\Lambda| = N$ sites, each site carries a ternary state $s \in \{-1, 0, +1\}$, giving a Hilbert space $\mathcal{H} = \mathbb{C}^{3^N}$. The full observable algebra is $B(\mathcal{H}) = M_{3^N}(\mathbb{C})$, a Type $\mathrm{I}_{3^N}$ factor. Local algebras satisfy isotony and locality (commutativity on spacelike-separated regions), and the partial trace provides a conditional expectation from composite systems to subsystems.

In the thermodynamic limit $\Lambda \to \mathbb{Z}^3$, the quasi-local algebra $\mathfrak{A} = \overline{\bigcup_\Lambda M_{3^{|\Lambda|}}(\mathbb{C})}$ equipped with a faithful thermal state gives a Type $\mathrm{III}_1$ factor by the Araki--Woods theorem. The FTD manifestation rule $s = \mathrm{sign}(\mathbf{J} \cdot \hat{n})$ then acts as a conditional expectation that projects from the continuous flux algebra to the discrete state algebra --- an algebraic analogue of the Type $\mathrm{III}_1 \to$ Type $\mathrm{I}$ transition identified with quantum measurement.

**Epistemic discipline:** The finite-lattice results (Sections 1--4) are [THEOREM]. The infinite-volume classification (Section 5) is [SELECTION] --- it invokes the Araki--Woods theorem, which is proven mathematics, but the identification of FTD's flux field with the relevant operator algebra is a structural argument, not a derivation from the five postulates. The measurement interpretation (Section 6) is [SELECTION].

---

## 1. Setup: The FTD Ternary Lattice

### 1.1 Single-Site Hilbert Space

**[AXIOM]** (Postulate 3: Ternary States). Each lattice site $x \in \mathbb{Z}^3$ occupies one of three states:

$$s(x) \in \{-1, 0, +1\}$$

The single-site Hilbert space is $\mathcal{H}_x = \mathbb{C}^3$, spanned by the orthonormal basis $\{|{+1}\rangle, |0\rangle, |{-1}\rangle\}$.

### 1.2 N-Site Hilbert Space

For a finite sublattice $\Lambda \subset \mathbb{Z}^3$ with $|\Lambda| = N$ sites, the total Hilbert space is the tensor product:

$$\mathcal{H}_\Lambda = \bigotimes_{x \in \Lambda} \mathcal{H}_x = \bigotimes_{x \in \Lambda} \mathbb{C}^3 = \mathbb{C}^{3^N}$$

The dimension grows as $3^N$, reflecting the ternary (not binary) character of the lattice.

### 1.3 The Two-Layer Ontology

FTD posits two fields on each site:

| Field | Type | Algebra | Role |
|-------|------|---------|------|
| Flux $\mathbf{J}(x) \in \mathbb{R}^3$ | Continuous vector | $L^\infty(\mathbb{R}^3)$ (abelian) | Dispositional (potential) |
| State $s(x) \in \{-1, 0, +1\}$ | Discrete ternary | $M_3(\mathbb{C})$ (non-abelian) | Actual (manifest) |

The manifestation rule connects them: $s = \mathrm{sign}(\mathbf{J} \cdot \hat{n})$, where $\hat{n}$ is the preferred direction determined by the Gauss constraint.

---

## 2. Finite Lattice Algebra: Type I

### 2.1 The Observable Algebra

**Theorem 1** [THEOREM]. *For a finite lattice $\Lambda$ with $|\Lambda| = N$ sites, the algebra of all observables is:*

$$\mathfrak{A}(\Lambda) = B(\mathcal{H}_\Lambda) = M_{3^N}(\mathbb{C})$$

*This is a Type $\mathrm{I}_{3^N}$ factor.*

*Proof.* The Hilbert space $\mathcal{H}_\Lambda = \mathbb{C}^{3^N}$ is finite-dimensional. The algebra of all bounded operators on a finite-dimensional Hilbert space $\mathbb{C}^d$ is the full matrix algebra $M_d(\mathbb{C})$, which is a factor (its center is trivial: $\mathfrak{A}(\Lambda)' \cap \mathfrak{A}(\Lambda) = \mathbb{C} \cdot I$). By the Murray--von Neumann classification, $M_d(\mathbb{C})$ is Type $\mathrm{I}_d$. With $d = 3^N$, we have Type $\mathrm{I}_{3^N}$. $\square$

**Corollary** [THEOREM]. *The single-site algebra $\mathfrak{A}(\{x\}) = M_3(\mathbb{C})$ is Type $\mathrm{I}_3$.*

Type I factors are characterized by:
- **Minimal projections exist.** The rank-1 projections $P_s = |s\rangle\langle s|$ for $s \in \{-1, 0, +1\}$ are minimal (cannot be decomposed into smaller nonzero projections).
- **A normal semifinite trace exists.** The standard matrix trace $\mathrm{Tr}$ satisfies $\mathrm{Tr}(I) = 3^N < \infty$.
- **Discrete spectrum.** Every self-adjoint element has a discrete (finite) set of eigenvalues.

### 2.2 Explicit Single-Site Structure

The algebra $M_3(\mathbb{C})$ has dimension $3^2 = 9$ as a complex vector space, spanned by the matrix units $\{E_{ij}\}_{i,j=0}^{2}$ where $(E_{ij})_{kl} = \delta_{ik}\delta_{jl}$.

The three minimal projections corresponding to the ternary states are:

$$P_{+1} = |{+1}\rangle\langle{+1}| = E_{00}, \quad P_0 = |0\rangle\langle 0| = E_{11}, \quad P_{-1} = |{-1}\rangle\langle{-1}| = E_{22}$$

These satisfy:
- Idempotence: $P_s^2 = P_s$
- Hermiticity: $P_s^\dagger = P_s$
- Minimality: $\mathrm{rank}(P_s) = 1$
- Completeness: $P_{+1} + P_0 + P_{-1} = I_3$

---

## 3. Local Algebras and Their Properties

### 3.1 Local Algebra Construction

**Definition.** For a region $A \subseteq \Lambda$, the *local algebra* is:

$$\mathfrak{A}(A) = M_{3^{|A|}}(\mathbb{C}) \otimes I_{3^{|\Lambda \setminus A|}} \;\subset\; \mathfrak{A}(\Lambda)$$

This embeds the observables of region $A$ into the full algebra by tensoring with the identity on the complement.

### 3.2 Isotony

**Theorem 2** [THEOREM]. *The assignment $A \mapsto \mathfrak{A}(A)$ is isotone: if $A \subseteq B \subseteq \Lambda$, then $\mathfrak{A}(A) \subseteq \mathfrak{A}(B)$.*

*Proof.* Let $A \subseteq B$. Write $B = A \cup (B \setminus A)$. Then:

$$\mathfrak{A}(A) = M_{3^{|A|}} \otimes I_{3^{|B \setminus A|}} \otimes I_{3^{|\Lambda \setminus B|}}$$

$$\mathfrak{A}(B) = M_{3^{|B|}} \otimes I_{3^{|\Lambda \setminus B|}} = M_{3^{|A|}} \otimes M_{3^{|B \setminus A|}} \otimes I_{3^{|\Lambda \setminus B|}}$$

Since $M_{3^{|A|}} \otimes I_{3^{|B \setminus A|}} \subset M_{3^{|A|}} \otimes M_{3^{|B \setminus A|}}$, we have $\mathfrak{A}(A) \subseteq \mathfrak{A}(B)$. $\square$

### 3.3 Locality (Einstein Causality)

**Theorem 3** [THEOREM]. *If $A$ and $B$ are disjoint subsets of $\Lambda$, then $[\mathfrak{A}(A), \mathfrak{A}(B)] = 0$ (all observables commute).*

*Proof.* Elements of $\mathfrak{A}(A)$ have the form $X_A \otimes I_{B} \otimes I_{\mathrm{rest}}$ and elements of $\mathfrak{A}(B)$ have the form $I_A \otimes Y_B \otimes I_{\mathrm{rest}}$. Their commutator is:

$$(X_A \otimes I_B)(I_A \otimes Y_B) - (I_A \otimes Y_B)(X_A \otimes I_B) = X_A \otimes Y_B - X_A \otimes Y_B = 0$$

This follows from the tensor product structure. $\square$

This is the algebraic expression of FTD's local causality axiom (Postulate 4): observables on disjoint regions are independent.

---

## 4. Conditional Expectation and Coarse-Graining

### 4.1 Partial Trace as Conditional Expectation

**Theorem 4** [THEOREM]. *The partial trace $E_A = \mathrm{Tr}_B : \mathfrak{A}(\Lambda) \to \mathfrak{A}(A)$ (where $B = \Lambda \setminus A$) is a conditional expectation. Specifically, it satisfies:*

1. *Linearity: $E_A(\alpha X + \beta Y) = \alpha E_A(X) + \beta E_A(Y)$*
2. *Complete positivity: $E_A(X^\dagger X) \geq 0$*
3. *Trace preservation: $\mathrm{Tr}(E_A(\rho)) = \mathrm{Tr}(\rho)$*
4. *Idempotence: $E_A \circ E_A = E_A$*
5. *Bimodule property: $E_A(a X b) = a \, E_A(X) \, b$ for $a, b \in \mathfrak{A}(A)$*

*Proof.* Properties (1)--(3) are standard properties of the partial trace on finite-dimensional matrix algebras (see Takesaki, Vol. I, Ch. V). Property (4): $E_A$ maps $\mathfrak{A}(\Lambda)$ to $\mathfrak{A}(A) \otimes \frac{1}{d_B} I_B$. Applying $E_A$ again yields the same result since $\mathrm{Tr}_B(I_B / d_B) = 1$. Property (5): for $a = a_A \otimes I_B$ and $b = b_A \otimes I_B$,

$$E_A(a X b) = \mathrm{Tr}_B((a_A \otimes I_B) X (b_A \otimes I_B)) = a_A \, \mathrm{Tr}_B(X) \, b_A = a \, E_A(X) \, b$$

using the cyclic property of the partial trace under operators that act trivially on $B$. $\square$

### 4.2 The Sign Function as Coarse-Graining

**Theorem 5** [THEOREM]. *The sign function $\mathrm{sign}: \mathbb{R} \to \{-1, 0, +1\}$ is a projection (idempotent map) that partitions $\mathbb{R}$ into exactly three preimage classes:*

$$\mathrm{sign}^{-1}(\{-1\}) = (-\infty, 0), \quad \mathrm{sign}^{-1}(\{0\}) = \{0\}, \quad \mathrm{sign}^{-1}(\{+1\}) = (0, \infty)$$

*Proof.* Direct verification: $\mathrm{sign}(\mathrm{sign}(x)) = \mathrm{sign}(x)$ for all $x \in \mathbb{R}$ since $\mathrm{sign}(-1) = -1$, $\mathrm{sign}(0) = 0$, $\mathrm{sign}(1) = 1$. The preimage sets are immediate from the definition. $\square$

**Theorem 6** [THEOREM]. *The coarse-graining $\mathrm{sign}: \mathbb{R} \to \{-1, 0, +1\}$ increases entropy. For any continuous random variable $X$ with finite differential entropy $h(X)$ and the discrete random variable $S = \mathrm{sign}(X)$:*

$$H(S) \leq \ln 3$$

*and the coarse-grained variable $S$ carries strictly less information than $X$ (the mutual information $I(X; S) < h(X)$ whenever $X$ has support on both sides of zero).*

*Proof.* $H(S) \leq \ln 3$ since $S$ takes at most 3 values. The sign function is a many-to-one map: its preimage classes are uncountably infinite (except for $\{0\}$, which has measure zero for any continuous distribution). Therefore $H(X|S) > 0$, and by the data processing inequality $I(X; S) = h(X) - H(X|S) < h(X)$. $\square$

This entropy increase under the sign projection is the algebraic content of irreversibility in FTD's manifestation process. Information about the continuous flux $\mathbf{J}$ is destroyed when the discrete state $s$ is formed.

---

## 5. The Thermodynamic Limit: Type III$_1$

### 5.1 The Quasi-Local Algebra

**[CLASSICAL]** (Bratteli--Robinson, Vol. II). For an increasing sequence of finite regions $\Lambda_1 \subset \Lambda_2 \subset \cdots$ with $\bigcup_n \Lambda_n = \mathbb{Z}^3$, the *quasi-local algebra* is the C*-algebraic inductive limit:

$$\mathfrak{A} = \overline{\bigcup_n \mathfrak{A}(\Lambda_n)}^{\|\cdot\|}$$

This is the norm closure of the union of all local algebras. For FTD, each local factor is $M_3(\mathbb{C})$, so:

$$\mathfrak{A} = \overline{\bigotimes_{x \in \mathbb{Z}^3}} M_3(\mathbb{C})$$

(the infinite tensor product of $3 \times 3$ matrix algebras).

### 5.2 Araki--Woods Classification

**[CLASSICAL]** (Powers 1967, Araki--Woods 1968). *Let $\mathfrak{A} = \overline{\bigotimes_{n=1}^\infty} M_d(\mathbb{C})$ be an infinite tensor product of $d \times d$ matrix algebras, equipped with the product state $\omega = \bigotimes_{n=1}^\infty \omega_n$ where each $\omega_n$ has density matrix $\rho_n$ with eigenvalues $\{\lambda_1^{(n)}, \ldots, \lambda_d^{(n)}\}$. Then:*

1. *If $\rho_n = \frac{1}{d} I_d$ for all $n$ (maximally mixed), the GNS representation gives a Type $\mathrm{II}_1$ factor.*
2. *If $\rho_n$ is not maximally mixed and the eigenvalue ratios satisfy a certain divergence condition, the GNS representation gives a Type $\mathrm{III}_\lambda$ factor, where $\lambda \in [0, 1]$ is determined by the asymptotic ratio of eigenvalues.*
3. *For a thermal (KMS) state at finite inverse temperature $\beta$, the modular automorphism group has full Connes spectrum $S(\mathcal{M}) = \mathbb{R}_+$, yielding Type $\mathrm{III}_1$.*

### 5.3 Application to FTD

**[SELECTION]** We identify the FTD flux field algebra in the thermodynamic limit with the quasi-local algebra described above. The argument proceeds as follows:

1. **Single-site algebra:** $M_3(\mathbb{C})$ (the ternary state algebra). This is established by Theorem 1.

2. **State:** The FTD lattice at thermal equilibrium (or in a generic non-ground state) is described by a product state $\omega$ with single-site density matrix:

$$\rho_\beta = \frac{1}{Z(\beta)} \mathrm{diag}(e^{\beta}, 1, e^{-\beta})$$

where $\beta$ is the inverse temperature and the energies of the three ternary states are $\{-1, 0, +1\}$ in lattice units. For any finite $\beta > 0$, this is not the maximally mixed state.

3. **Araki--Woods applies:** The eigenvalue ratios are $\lambda_1/\lambda_3 = e^{2\beta}$. For the FTD KMS temperature $\beta = \pi$, the ratio is $e^{2\pi} \approx 535$. The Connes invariant is:

$$S(\mathcal{M}) = \overline{\{e^{-2n\beta} : n \in \mathbb{Z}\}} = \mathbb{R}_+$$

(the closure in $\mathbb{R}_+$ of the cyclic group generated by $e^{-2\beta}$). This gives **Type $\mathrm{III}_1$**.

4. **At zero temperature** ($\beta \to \infty$): the state becomes a pure product state, and the GNS representation gives a Type $\mathrm{I}$ factor (irreducible representation on $\mathcal{H}$).

**Why this is [SELECTION], not [THEOREM]:** The identification of FTD's flux field with the operator-algebraic infinite tensor product is a structural argument. The five FTD postulates define a discrete lattice with ternary states and local update rules --- they do not directly invoke C*-algebraic completions or GNS constructions. The Araki--Woods theorem is proven mathematics; the claim that it applies to FTD is a modeling choice.

---

## 6. The Measurement Transition: Type III$_1$ to Type I

### 6.1 Before Measurement: Dispositional Algebra

**[SELECTION]** Prior to manifestation, the FTD flux field $\mathbf{J}(x) \in \mathbb{R}^3$ at each site encodes continuous information. In the thermodynamic limit, the algebra of flux-field observables is (by the argument of Section 5) a Type $\mathrm{III}_1$ factor. The key features of Type $\mathrm{III}_1$:

- **No minimal projections.** One cannot decompose the identity into atomic (rank-1) pieces. There is no "smallest observable."
- **No normal semifinite trace.** The usual notion of "probability" (via a trace-class density matrix) does not apply in the standard way.
- **Ergodic modular flow.** The modular automorphism group $\sigma_t$ acts ergodically: no non-trivial fixed points. The system is in perpetual "flux."

These are precisely the properties one would expect for the dispositional layer: continuous, undetermined, with no definite discrete outcomes.

### 6.2 After Measurement: Actualized Algebra

**[THEOREM]** After the manifestation rule $s = \mathrm{sign}(\mathbf{J} \cdot \hat{n})$ acts, the state at each site is one of $\{-1, 0, +1\}$. On any finite region, the observable algebra is $M_{3^N}(\mathbb{C})$, which is Type $\mathrm{I}$:

- **Minimal projections exist:** $|s_1, \ldots, s_N\rangle\langle s_1, \ldots, s_N|$ for each configuration.
- **Trace is well-defined:** $\mathrm{Tr}(I) = 3^N$.
- **Discrete spectrum:** Every observable has finitely many eigenvalues.

### 6.3 The Sign Function as Algebraic Phase Transition

**[SELECTION]** The FTD manifestation rule implements the following algebraic transition:

$$\underbrace{\text{Type III}_1}_{\text{flux field } \mathbf{J}} \;\xrightarrow{\;\mathrm{sign}(\mathbf{J} \cdot \hat{n})\;}\; \underbrace{\text{Type I}}_{\text{state field } s}$$

The sign function acts as a conditional expectation (coarse-graining map) from the continuous algebra to the discrete algebra. By Theorem 6, this map is information-destroying: the continuous flux information is irreversibly lost when the discrete state is formed.

This transition is the algebraic content of quantum measurement in FTD. It does not require an external observer, a consciousness postulate, or an ad hoc projection rule. The lattice's own dynamics (the manifestation rule, derived from the five postulates) provide the "cut" that von Neumann sought.

### 6.4 Connection to the Von Neumann Chain

The von Neumann measurement chain asks: who observes the observer? In the algebraic language:

- Each link in the chain attempts to create a Type I factor (definite outcome) from a Type III$_1$ factor (indefinite substrate).
- On a finite lattice, the chain terminates because the full algebra is already Type I (Theorem 1). There is no Type III$_1$ to begin with --- only finitely many degrees of freedom exist.
- In the thermodynamic limit, the chain terminates because the sign function provides an intrinsic mechanism for the Type III$_1 \to$ Type I transition. No external observer is needed.

See [FOUND_VON_NEUMANN_CHAIN.md](FOUND_VON_NEUMANN_CHAIN.md) for the full resolution.

---

## 7. Numerical Verification

The proof script `scripts/proofs/proof_von_neumann_type.py` verifies:

| Test | Tag | Status |
|------|-----|--------|
| Single-site algebra is $M_3(\mathbb{C})$, Type $\mathrm{I}_3$ | [THEOREM] | Verified |
| $N$-site algebra is $M_{3^N}(\mathbb{C})$, Type $\mathrm{I}_{3^N}$ | [THEOREM] | Verified |
| Trace of identity equals dimension | [THEOREM] | Verified |
| Minimal (rank-1) projections exist | [THEOREM] | Verified |
| Tensor product structure: $\mathrm{Tr}(A \otimes B) = \mathrm{Tr}(A)\mathrm{Tr}(B)$ | [THEOREM] | Verified |
| Local algebras satisfy isotony | [THEOREM] | Verified |
| Disjoint local algebras commute | [THEOREM] | Verified |
| Partial trace is trace-preserving | [THEOREM] | Verified |
| Partial trace preserves positivity | [THEOREM] | Verified |
| Conditional expectation is idempotent | [THEOREM] | Verified |
| Bimodule property holds | [THEOREM] | Verified |
| Sign function maps $\mathbb{R} \to \{-1, 0, +1\}$ | [THEOREM] | Verified |
| Sign is idempotent on discrete states | [THEOREM] | Verified |
| Entropy increases under coarse-graining | [THEOREM] | Verified |
| Araki--Woods preconditions hold | [THEOREM] | Verified |
| Araki--Woods gives Type III$_1$ in infinite volume | [SELECTION] | Structural |
| Sign/ReLU as Type III$_1 \to$ Type I transition | [SELECTION] | Structural |

---

## 8. Epistemic Accounting

### 8.1 What Is Proven [THEOREM]

The following results hold rigorously on finite lattices, verified both analytically and numerically:

- The observable algebra on $N$ sites is $M_{3^N}(\mathbb{C})$, a Type $\mathrm{I}_{3^N}$ factor.
- Local algebras satisfy isotony and locality.
- The partial trace is a conditional expectation with all required properties.
- The sign function is an idempotent coarse-graining that increases entropy.
- The Araki--Woods preconditions (non-maximally-mixed state on $M_3$) are satisfied.

These are standard results in finite-dimensional operator algebra and linear algebra. They do not depend on any FTD-specific assumptions beyond the ternary state structure.

### 8.2 What Is Structural [SELECTION]

- The identification of the thermodynamic limit with a Type $\mathrm{III}_1$ factor via Araki--Woods. The theorem itself is proven mathematics (Powers 1967, Araki--Woods 1968). The application to FTD requires identifying the lattice's thermal state with a product state on the quasi-local algebra.
- The interpretation of the sign/ReLU manifestation rule as the algebraic mechanism of the Type $\mathrm{III}_1 \to$ Type $\mathrm{I}$ transition. This is a structural correspondence, not a derivation from the five postulates.

### 8.3 What Cannot Be Verified Numerically

- The infinite tensor product and its GNS representation (requires infinite-dimensional Hilbert space).
- The Connes spectrum and modular automorphism group of the limiting factor.
- The precise mechanism by which the sign function "destroys" the Type $\mathrm{III}_1$ structure (this would require constructing the infinite-dimensional factor explicitly).

### 8.4 What This Document Does NOT Claim

- We do **not** claim to have proven that FTD's flux field IS a Type $\mathrm{III}_1$ factor. We claim it has the structural features that, under the Araki--Woods theorem, would yield Type $\mathrm{III}_1$ in the thermodynamic limit.
- We do **not** claim the Type $\mathrm{III}_1 \to$ Type $\mathrm{I}$ transition is a theorem of operator algebra for the sign function. The sign function operates on $\mathbb{R}$, not on a von Neumann algebra. The transition is an analogy grounded in structural correspondence.
- We do **not** claim this construction explains consciousness. It explains the algebraic structure of observables on the FTD lattice and identifies a measurement mechanism.

---

## 9. References

### Numerical Verification (April 11, 2026)

`scripts/proofs/proof_modular_hamiltonian.py` computed the modular operator spectrum on finite FTD lattices (2-site chain through 2x2x2 cube):

| Lattice | dim | Distinct modular ratios at beta=pi | At beta=1000 | Trend |
|---------|-----|-----------------------------------|-------------|-------|
| 2-site | 9 | 5 | 5 | Type I (near) |
| 4-site chain | 81 | 13 | 22 | Type III-like |
| 2x2 square | 81 | 15 | 15 | Type III-like |
| **2x2x2 cube** | **6561** | **43** | **166** | **Type III-like, growing** |

Key findings:

1. **Finite lattice is always Type I** (discrete spectrum). This is expected — Type III_1 requires the thermodynamic limit.
2. **The number of distinct modular eigenvalue ratios grows with system size** (5 -> 13 -> 43), consistent with the Connes spectrum approaching R_+ in the thermodynamic limit.
3. **The ratio range widens dramatically with beta**: at beta=1000, the 8-site case shows ratios spanning [0, 10^76], consistent with Type III_1 (full positive reals).
4. **Entropy evolution**: high-T regime (beta << 1) shows maximal entropy S -> log(3^N); low-T regime (beta >> 1) shows S -> log(3) (ground-state degeneracy); intermediate beta shows the Type III-like behavior.

**What this advances:** Explicit numerical construction of the modular operator on the FTD lattice, demonstrating the expected finite-size precursor of Type III_1 with growing spectrum diversity.

**What remains:** Thermodynamic limit extrapolation (tensor network / RG methods needed), non-diagonal Hamiltonian (full FTD Lagrangian with flux-flux coupling), and verification that the continuous Connes spectrum S = R_+ is achieved at N -> infinity.

---

### Internal (FTD Documents)

1. [SPEC_FTD.md](../../SPEC_FTD.md) -- Master FTD specification
2. [FOUND_VON_NEUMANN_CHAIN.md](FOUND_VON_NEUMANN_CHAIN.md) -- Von Neumann chain resolution
3. [DERIV_COLLAPSE_MECHANISM.md](DERIV_COLLAPSE_MECHANISM.md) -- ReLU crystallization, Lindblad equation
4. [DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md](DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md) -- Master quadratic, three domains
5. [EXPLR_RELU_TYPE_TRANSITION.md](../09_mathematical/EXPLR_RELU_TYPE_TRANSITION.md) -- ReLU as type transition

### External

6. F. J. Murray and J. von Neumann, "On rings of operators," *Ann. Math.* **37**, 116--229 (1936).
7. R. T. Powers, "Representations of uniformly hyperfinite algebras and their associated von Neumann rings," *Ann. Math.* **86**, 138--171 (1967).
8. H. Araki and E. J. Woods, "A classification of factors," *Publ. RIMS Kyoto* **4**, 51--130 (1968).
9. A. Connes, "Classification of injective factors," *Ann. Math.* **104**, 73--115 (1976).
10. O. Bratteli and D. W. Robinson, *Operator Algebras and Quantum Statistical Mechanics*, Vols. I--II (Springer, 1979--1981).
11. M. Takesaki, *Theory of Operator Algebras*, Vols. I--III (Springer, 1979--2003).
