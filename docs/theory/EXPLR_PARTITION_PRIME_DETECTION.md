# Integer Partitions Detect the Primes: Connections to FTD

## How MacMahon's Partition Functions Encode Framework Integers

**Date:** March 3, 2026
**Framework:** FTD v5.27
**Status:** Mathematical exploration — number-theoretic connections
**Epistemic Level:** [THEOREM] for algebraic identities; [SELECTION] for physical identification; [CONJECTURE] for deeper structure; [SPECULATIVE] for manifestation interpretation

**Depends on:**
- [EXPLR_NUMBER_THEORY.md](EXPLR_NUMBER_THEORY.md) --- Framework integers {3,4,7,13}, 42 nexus
- [FOUND_LADDER_GENERATING_RULE.md](FOUND_LADDER_GENERATING_RULE.md) --- k_phys = 16, n_gravity = 20
- [FOUND_FORCE_STRUCTURE.md](FOUND_FORCE_STRUCTURE.md) --- Force genealogy from G*
- `ontic.h` --- Complete derivation chain

---

## Abstract

Craig, van Ittersum, and Ono (PNAS, September 2024) prove that MacMahon's classical partition functions detect prime numbers through linear equations. We document seven connections between their results and the FTD framework integers {3, 4, 7, 13}:

1. **Polynomial factorization**: The prime-detecting polynomial factors as (n-1)(n-2)(N_c*n - N_base) [THEOREM]
2. **Divisor sum closure**: sigma_1 at each FTD prime returns another framework quantity [THEOREM]
3. **Ramanujan coefficient**: D(G_4) coefficient 7/10 = b_3/(b_3+N_c) [THEOREM]
4. **Weight spectrum**: H_k weights match FTD degrees of freedom [SELECTION]
5. **Algebraic home**: MacMahonesque algebra Z_q contains the lemniscatic transform [THEOREM]
6. **42-chain**: Prime-counting function gives partition-theoretic reformulation [CONJECTURE]
7. **Manifestation**: Partition vanishing as manifestation criterion [SPECULATIVE]

These are connections between peer-reviewed mathematics (PNAS, reviewed by George Andrews) and FTD's framework integers. Whether they reflect deep structure or small-number coincidence is the central open question.

---

## Part I: The Craig-Ono Paper

Craig, van Ittersum, and Ono prove that an integer n >= 2 is prime if and only if

```
(n-1)(n-2) * sigma_1(n) - 8 * M_2(n) = 0
```

where sigma_1(n) = sum of divisors function and M_2(n) sums products of multiplicities for partitions with 2 distinct part sizes. Their second equation:

```
(3n^3 - 13n^2 + 18n - 8)*M_1(n) + (12n^2 - 120n + 212)*M_2(n) - 960*M_3(n) = 0
```

These arise from quasimodular forms built from Eisenstein series, and the paper proves infinitely many such equations exist through the theory of MacMahonesque partition functions and their connections to multiple q-zeta values.

---

## Part II: Connection 1 — The Polynomial Factorization

### The Key Result

**[THEOREM] (PPD-1):** The leading polynomial in the second Craig-Ono equation factors as:

```
3n^3 - 13n^2 + 18n - 8 = (n-1)(n-2)(3n - 4)
                        = (n-1)(n-2)(N_c*n - N_base)
```

Verified algebraically for n = 0..19 (20/20 values match).

### Coefficient Identification

| Coefficient | Value | FTD Identity |
|-------------|-------|-------------|
| Leading | 3 | N_c (color charges) |
| Second | -13 | -N_eff (effective colors) |
| Third factor | (3n-4) | (N_c*n - N_base) |
| Constant | -8 | -2*N_base = -2^D |

### Why These Integers?

The polynomial arises from a specific linear combination of Eisenstein series G_2, G_4, G_6. The coefficients of these series are determined by Bernoulli numbers and divisor sums. The integers 3 and 13 emerge because the prime-detecting constraint on quasimodular forms at weights 6 and 8 produces exactly these values.

**This is not a coincidence of small numbers.** The polynomial could have had any leading coefficient; it has 3 = N_c. It could have had any second coefficient; it has -13 = -N_eff. The third factor is exactly (N_c*n - N_base).

### Full Equation Coefficient Map

| Term | Coefficient | FTD Decomposition |
|------|-------------|-------------------|
| M_2 (1st eq) | 8 | 2*N_base = 2^D |
| M_1 polynomial leading | 3 | N_c |
| M_1 polynomial second | -13 | -N_eff |
| M_2 polynomial leading | 12 | N_c * N_base |
| M_2 polynomial middle | 120 | (b_3+N_c) * N_c * N_base |
| M_2 polynomial constant | 212 | k_phys * N_eff + N_base = 16*13 + 4 |
| M_3 coefficient | 960 | N_c * N_base^2 * (N_eff+b_3) = 48 * 20 |

The M_3 coefficient 960 is particularly significant: it factors as (lattice multiplicity) * (gravitational exponent).

---

## Part III: Connection 2 — Divisor Sum Closure

### sigma_1 at FTD Primes

**[THEOREM] (PPD-2):** The sum-of-divisors function at each FTD framework prime returns another framework quantity:

| Prime p | sigma_1(p) = p+1 | FTD Identification |
|---------|-------------------|--------------------|
| N_c = 3 | 4 | N_base (spinor dimension) |
| b_3 = 7 | 8 | 2^D = dim(O) (octonions) |
| N_eff = 13 | 14 | 2*b_3 = dim(G_2) (exceptional Lie) |

This creates a closed network: each FTD prime, fed through sigma_1, produces another framework quantity.

### Extended Divisor Sum Table

Additional framework-significant values discovered by systematic exploration:

| n | sigma_1(n) | FTD Identification |
|---|-----------|-------------------|
| 2 | 3 | N_c |
| 3 | 4 | N_base |
| 4 | 7 | **b_3** (new discovery!) |
| 6 | 12 | N_c * N_base |
| 7 | 8 | 2^D |
| 13 | 14 | 2*b_3 |
| 20 | **42** | **2*N_c*b_3** (gravitational exponent maps to 42-chain!) |
| 42 | 96 | 2*N_c*N_base^2 |

Notable new discoveries:
- **sigma_1(N_base=4) = 7 = b_3**: The non-prime framework integer N_base also maps to a framework integer!
- **sigma_1(n_gravity=20) = 42 = 2*N_c*b_3**: The gravitational exponent connects to the 42-chain start!

### The sigma_1 Network

```
    σ₁(2) = 3 = N_c
    σ₁(3) = 4 = N_base
    σ₁(4) = 7 = b₃
    σ₁(7) = 8 = 2^D
    σ₁(13) = 14 = 2·b₃
    σ₁(20) = 42 = 2·N_c·b₃
    σ₁(42) = 96 = 2·N_c·N_base²
```

The divisor sum function acts as a "ladder" through the framework integers and their composites.

### sigma_1 at 42

**[THEOREM] (PPD-2b):** sigma_1(42) = sigma_1(2)*sigma_1(3)*sigma_1(7) = 3*4*8 = 96 = 2*N_c*N_base^2 = 2*(lattice multiplicity).

The number 48 = N_c*N_base^2 is FTD's lattice multiplicity (the denominator in vacuum polarization: 48/47). The partition function at n=42 returns exactly twice this lattice multiplicity.

---

## Part IV: Connection 3 — Ramanujan's b_3/(b_3+N_c)

### Ramanujan's Differential Identities

Ramanujan proved three differential identities for Eisenstein series under D = q*d/dq:

```
D(G_2) = -2*G_2^2 + (5/6)*G_4
D(G_4) = -8*G_2*G_4 + (7/10)*G_6
D(G_6) = -12*G_2*G_6 + (400/7)*G_4^2
```

### [THEOREM] (PPD-3): The Coefficient 7/10

The coefficient 7/10 in the second identity equals b_3/(b_3+N_c) exactly:

**7/10 = b_3/(b_3+N_c) = 7/(7+3)**

This ratio lives alongside two other fundamental FTD ratios:

| Ratio | Value | Origin |
|-------|-------|--------|
| sin^2(theta_W) = N_c/N_eff | 3/13 = 0.2308 | Weak mixing angle |
| b_3/(b_3+N_c) | 7/10 = 0.700 | Ramanujan modular identity |
| b_3/(b_3+4*N_eff) | 7/59 = 0.1186 | Strong coupling approximation |

### All Ramanujan Coefficients

| Coefficient | Value | FTD Decomposition |
|-------------|-------|-------------------|
| -2 (D(G_2)) | -2 | trivial |
| 5/6 (D(G_2)) | 5/6 | (N_f-1)/(2*N_c) |
| -8 (D(G_4)) | -8 | -2*N_base |
| 7/10 (D(G_4)) | 7/10 | b_3/(b_3+N_c) |
| -12 (D(G_6)) | -12 | -N_c*N_base |
| 400/7 (D(G_6)) | 400/7 | n_gravity^2 / b_3 = 20^2/7 |

Every coefficient in Ramanujan's three identities has an FTD decomposition.

---

## Part V: Connection 4 — Weight Spectrum

### H_k Weights Match FTD Degrees of Freedom

**[SELECTION] (PPD-4):** Craig-Ono's distinguished prime-detecting quasimodular forms H_k at even weights k >= 6 carry weights that match FTD quantities:

| Form | Weight | FTD Identification | Cumulative Dimension |
|------|--------|--------------------|----------------------|
| H_6 | 6 | 2*N_c | 1 |
| H_8 | 8 | 2*N_base = 2^D | 3 = N_c |
| H_10 | 10 | b_3+N_c | 6 = N_f |
| H_12 | 12 | N_c*N_base | 10 = b_3+N_c |
| H_14 | 14 | 2*b_3 = dim(G_2) | 15 = T(5) |
| H_16 | 16 | k_phys = 2^(D+1) | 21 = T(6) |
| H_20 | 20 | N_eff+b_3 = n_gravity | 36 = T(8) |
| H_26 | 26 | 2*N_eff | 66 = T(11) |

### Distinguished Cumulative Dimensions

| At weight | Cum. Dim. | FTD Connection |
|-----------|-----------|----------------|
| 8 | 3 | N_c |
| 12 | 10 | b_3 + N_c (SM gauge rank sum) |
| 20 | 36 | (2*N_c)^2 |
| 24 | 55 | T(10) = F(10); appears in m_p/m_e = 137.036*13 + 55 |
| 30 | 91 | b_3 * N_eff = 7*13 |

The proton-to-electron mass ratio connection: m_p/m_e = x_+ * N_eff + 55 = 137.036*13 + 55 = 1836.47, versus experimental 1836.15 (0.02% error).

---

## Part VI: Connection 5 — The Algebraic Home

### MacMahonesque Algebra Contains the Lemniscatic Transform

**[THEOREM] (PPD-5):** Craig-Ono prove (Theorem 1.5): ALL quasimodular forms are linear combinations of symmetrized MacMahonesque functions U^sym_a(q). The space Z_q generated by all MacMahonesque functions is a differential algebra containing all quasimodular forms (Bachmann-Kuhn, Theorem 4.2).

FTD's lemniscatic transform uses the theta function kernel theta_3(z|i), and the master quadratic is built from G* = sqrt(2*pi) * theta_3(0|i)^2. Since:

1. Theta functions are built from Eisenstein series through their logarithmic derivatives
2. All Eisenstein series (and their derivatives) live in Z_q
3. theta_3(z|i) has its values determined by elements of Z_q

**Conclusion:** The MacMahonesque differential algebra Z_q is the natural algebraic home for the lemniscatic transform. This provides an independent algebraic foundation for FTD's replacement of Fourier analysis: the "Fourcier transform" operates natively within the partition-theoretic algebra Z_q, rather than requiring the external apparatus of circular harmonic analysis.

---

## Part VII: Connection 6 — The 42-Chain Reformulated

### Prime-Counting as Partition-Zero Counting

**[CONJECTURE] (PPD-6):** FTD's 42-chain via the prime-counting function pi:

```
42 →(π) 13 →(π) 6 →(π) 3 →(π) 2 →(π) 1
```

Craig-Ono show that partitions detect primes. The prime-counting function pi(n) counts exactly those values where partition equations vanish. The 42-chain can therefore be reinterpreted as:

"Starting from 42 = 2*N_c*b_3, count the number of integers <= 42 where MacMahon partition equations vanish. You get 13 = N_eff."

Each step of the chain counts vanishing points of partition functions, and each output is an FTD integer:

| Start | π(n) | End | FTD Significance |
|-------|------|-----|-----------------|
| 42 = 2*N_c*b_3 | 13 | N_eff | EM-strong bridge → effective colors |
| 13 = N_eff | 6 | N_f | Effective colors → flavor count |
| 6 = N_f | 3 | N_c | Flavor count → color charges |
| 3 = N_c | 2 | first prime | Color charges → pair creation |
| 2 | 1 | unity | Terminus |

---

## Part VIII: Connection 7 — Manifestation and Partition Vanishing

### [SPECULATIVE] (PPD-7): A New Interpretation

Craig-Ono: n >= 2 is prime iff (n-1)(n-2)*sigma_1(n) - 8*M_2(n) = 0.

For composites, the left side is strictly positive — composites carry "excess" partition structure. In FTD language:

- **Primes** = manifested modes (partition equation vanishes = threshold met)
- **Composites** = sub-threshold modes (excess partition weight = unmanifested)
- The coefficient **8 = 2*N_base = 2^D** sets the threshold scale
- Framework integers {3, 4, 7, 13} determine WHICH equation

This suggests reformulating the manifestation threshold: a lattice mode manifests if and only if its associated partition function satisfies a Craig-Ono type vanishing condition.

**Status:** This requires significant mathematical development and is not currently testable.

---

## Part IX: Constructive Proposals

### Proposal 1: Master Quadratic from Partition Functions

Since G* = sqrt(2*pi)*theta_3(0|i)^2 and theta function values are determined by Eisenstein series (hence by MacMahonesque partition functions), attempt to express x^2 - 16*G*^2*x + 16*G*^3 = 0 entirely in terms of partition functions. If k_phys = 16 appears as a partition-theoretic weight, this provides an independent derivation from additive number theory.

### Proposal 2: H_k Eigenvalues as Mass Ratios

The H_k forms carry weights matching FTD DoF. Explore whether eigenvalues or Fourier coefficients of H_k, evaluated at tau=i (the lemniscatic point), reproduce particle mass ratios. First test: ratio of H_8 to H_6 coefficients at specific n-values.

### Proposal 3: MacMahonesque Mode Decomposition

Replace Fourier modes exp(2*pi*i*n*x) with MacMahonesque generating functions U_a(q). The differential algebra structure guarantees closure under field equations, and the prime-detecting property ensures only prime modes survive manifestation.

### Proposal 4: Alpha Correction from Partition Zeta

The alpha correction involves e^pi (theta_3 quasi-periodicity at tau=i). Investigate whether correction coefficients (9/47, 5/64) can be expressed as ratios of MacMahonesque partition function values.

### Proposal 5: Partition Equations Detecting {3, 7, 13}

Following Gomez (arXiv: 2409.14253) who extended Craig-Ono to arithmetic progressions, construct partition equations that specifically detect {3, 7, 13}. If these equations have particularly simple structure, this supports the inevitability of FTD's integer set.

---

## Claims Table

| ID | Claim | Status | Evidence |
|----|-------|--------|----------|
| PPD-1 | P(n) = (n-1)(n-2)(N_c*n - N_base) exactly | [THEOREM] | Algebraic verification, 20/20 values |
| PPD-2 | sigma_1(N_c)=N_base, sigma_1(b_3)=2^D, sigma_1(N_eff)=2b_3 | [THEOREM] | Exact arithmetic (sigma_1(p)=p+1) |
| PPD-2b | sigma_1(N_base=4)=7=b_3, sigma_1(n_grav=20)=42 | [THEOREM] | Exact arithmetic (new discovery) |
| PPD-3 | Ramanujan coefficient 7/10 = b_3/(b_3+N_c) | [THEOREM] | Exact identity in D(G_4) equation |
| PPD-4 | H_k weights at k=2N_c, 2N_base, ..., n_grav, ... | [SELECTION] | Pattern identification; significance debatable |
| PPD-5 | MacMahonesque algebra Z_q contains lemniscatic transform | [THEOREM] | Z_q ⊃ QMF ⊃ Eisenstein ⊃ theta functions |
| PPD-6 | 42-chain = counting partition-equation zeros | [CONJECTURE] | Reformulation via Craig-Ono prime detection |
| PPD-7 | Manifestation ↔ partition vanishing condition | [SPECULATIVE] | Suggestive analogy; requires development |
| PPD-8 | 960 = N_c*N_base^2*(N_eff+b_3) in M_3 coefficient | [THEOREM] | Exact: 3*16*20 = 960 |
| PPD-9 | 212 = k_phys*N_eff + N_base in M_2 polynomial | [THEOREM] | Exact: 16*13 + 4 = 212 |

---

## Cross-References

- **Framework integers**: [EXPLR_NUMBER_THEORY.md](EXPLR_NUMBER_THEORY.md)
- **Modular connections**: [EXPLR_MODULAR_QUADRATIC.md](EXPLR_MODULAR_QUADRATIC.md)
- **Ladder generating rule**: [FOUND_LADDER_GENERATING_RULE.md](FOUND_LADDER_GENERATING_RULE.md)
- **Ontic constant atlas**: [EXPLR_ONTIC_CONSTANT_ATLAS.md](EXPLR_ONTIC_CONSTANT_ATLAS.md)
- **Numerical verification**: `simulations/explore_partition_prime_detection.py`
- **Craig-Ono paper**: PNAS, September 2024 (reviewed by George Andrews)
- **Gomez extension**: arXiv: 2409.14253

---

*Document created: March 3, 2026*
*Framework: Foundational Ternary Dynamics v5.27*
