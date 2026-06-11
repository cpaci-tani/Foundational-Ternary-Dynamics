# The Master Quadratic: Algebraic Identity and Physical Match

## What the Polynomial Is, Where Its Coefficients Come From, and What It Predicts

**Status:** [THEOREM] for the algebra; [STRONGLY MOTIVATED CONJECTURE] for the physical identification.
**Dependencies:** DERIV_WATSON_GSTAR_IDENTITY.md, DERIV_QUADRATIC_NECESSITY.md, DERIV_DUAL_DERIVATION_OF_16.md, FOUND_DIMENSIONAL_COUNTING.md, FOUND_BORN_RULE_NULL_CONE.md.
**See also:** AUDIT_INFINITY_REFRAME.md (undefined-boundary ontology), `../10_eft_program/archive/resolved/OPEN_A_PHYS_DERIVATION.md` (closed calibration question).

---

## Abstract

The master quadratic

$$x^2 - 16\,G^{*2}\,x + 16\,G^{*3} \;=\; 0 \qquad \text{with}\qquad G^* \;=\; \frac{\sqrt{2}\,\Gamma(1/4)^2}{2\pi}$$

is a **pure algebraic object**: every coefficient is computable from G\* to arbitrary finite precision. Its larger root x₊ = 137.036 matches 1/α to 1.26 ppm. The smaller root x₋ = 3.024 is a mathematical artifact of the polynomial; the historical identification `x₋  N_c` (which previously read this root as the QCD colour count) is **RETIRED** per FTD/FQCR Cleanup Taxonomy v1.4 §5 (LEDGER FTD-0014 removed in commit `ca7eb61`). `N_c = 3` in FTD is independently sourced — see `DERIV_NC_FROM_TOPOLOGY.md` (four routes) and the Moore Layer Theorem.

This document records (i) the algebraic identity itself; (ii) the multiple finite-combinatorial routes to each of its coefficients (G\* via Watson–Chowla–Selberg, the leading 16 via two independent counts, the constant term via Vieta); (iii) the discriminant trichotomy that partitions one quadratic into bosonic, fermionic, and measurement regimes; and (iv) the empirical match plus polynomial-template structural-uniqueness evidence (FTD-0189 adversarial scan: 0 non-G\* dual-matchers across 2.65M degree-2 polynomials over an 18-constant FTD-undesigned basket) that motivates the single live physics identification x₊  1/α (FTD-0013).

What this document does **not** do: derive the master quadratic as the L → ∞ limit of any finite-L self-consistency equation. The framework is undefined-boundary (no completed-totality lattice), and the explicit finite-L gap-equation scan and the L = 2 partition-function calculation jointly close the limit-route. The polynomial is what it is — an algebraic identity — and the physical identification x₊  1/α rests on the empirical match (1.26 ppm) + polynomial-template uniqueness (FTD-0189), not on a dynamical derivation.

---

## Part I: The Polynomial

### 1.1 Form

$$x^2 - 16\,G^{*2}\,x + 16\,G^{*3} \;=\; 0 \tag{1}$$

Equivalently, in factored form,

$$x^2 \;=\; 16\,G^{*2}\,(x - G^*) \;=\; 32\pi\,W_3\,(x - G^*) \tag{1'}$$

where $W_3 = G^{*2}/(2\pi)$ is Watson's BCC integral.

### 1.2 Roots

By the quadratic formula, the roots are

$$x_\pm \;=\; 8\,G^{*2} \pm \sqrt{64\,G^{*4} - 16\,G^{*3}} \;=\; 8\,G^{*2}\!\left(1 \pm \sqrt{1 - \tfrac{1}{4G^*}}\right). \tag{2}$$

Numerically, with G\* = 2.95867…,

| Root | Value | Match | Accuracy |
|---|---|---|---|
| $x_+$ | $137.036$ | $1/\alpha$ (CODATA: $137.035999177$) | $1.26$ ppm |
| $x_-$ | $3.024$ | mathematical artifact of $P(x)$; no physics identification (the historical `x_-  N_c` identification is **RETIRED** per v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`; `N_c = 3` independently sourced via `DERIV_NC_FROM_TOPOLOGY.md`) | n/a |

### 1.3 Vieta relations

$$x_+ + x_- \;=\; 16\,G^{*2}, \qquad x_+\,x_- \;=\; 16\,G^{*3}.$$

The harmonic mean of the two roots is

$$\frac{2\,x_+\,x_-}{x_+ + x_-} \;=\; \frac{2 \cdot 16\,G^{*3}}{16\,G^{*2}} \;=\; 2\,G^*.$$

So G\* is half the harmonic mean of the two roots. This places G\* as the natural "centre" referenced by the displacement form (1′).

---

## Part II: Where the Coefficients Come From

The polynomial has three coefficients: the leading $1$ (trivial), the linear coefficient $-16\,G^{*2}$, and the constant $+16\,G^{*3}$. Both nontrivial coefficients factor as $16$ times a power of $G^*$.

### 2.1 G\* — algebraic identity [THEOREM]

$$G^* \;=\; \frac{\sqrt{2}\,\Gamma(1/4)^2}{2\pi}, \qquad \frac{G^{*2}}{2\pi} \;=\; W_3 \;=\; \frac{\Gamma(1/4)^4}{4\pi^3}.$$

Both expressions are closed-form in the Γ function; both are computable to arbitrary finite precision. The Chowla–Selberg formula realises $W_3$ as a period, which is what makes the second equality exact rather than approximate. Watson (1939) recognised the same integral as the BCC component of the cubic-lattice Green's function. (See DERIV_WATSON_GSTAR_IDENTITY.md.)

This algebraic identity does not require a "completed lattice"; it is a relation between mathematical objects that exist algebraically.

### 2.2 The coefficient 16 — two independent finite-combinatorial routes [THEOREM]

The leading coefficient is `16`, by two independent counts:

**Route A — automorphism count of the CM curve.** The elliptic curve $E\colon y^2 = x^3 - x$ has $j$-invariant $j = 1728$, complex multiplication by $\mathbb{Z}[i]$, and automorphism group $\mathrm{Aut}(E) = \mathbb{Z}/4\mathbb{Z}$. Hence $|\mathrm{Aut}(E)|^2 = 4^2 = 16$. (See DERIV_DUAL_DERIVATION_OF_16.md.)

**Route B — BCC coordination times non-void ternary states.** The BCC sublattice of the 26-Moore neighbourhood has coordination number $z_\mathrm{BCC} = 8$. The non-void ternary states are $\{-1, +1\}$, count $2$. Their product is $z_\mathrm{BCC} \cdot 2 = 16$. (See FOUND_DIMENSIONAL_COUNTING.md §5.4.)

Both routes are **finite-combinatorial**: a property of the CM curve's automorphism group on one hand, and of the local Moore neighbourhood plus the ternary state set on the other. Neither invokes a limit. Their numerical agreement is the multi-route evidence for `16` as the natural leading coefficient.

(A third historical route, the temporal-gauge DOF count $24 - 7 - 1 = 16$ on the $2^3$ torus, has been retracted as incorrect: proper Coulomb-gauge fixing on $T^3$ yields $14$, not $16$. See AUDIT_MASTER_QUADRATIC.md.)

### 2.3 The factor 2π and Watson I_1 [THEOREM]

The linear coefficient can be rewritten as $16\,G^{*2} = 32\pi \cdot W_3$, exposing the algebraic factorisation $32\pi = 16 \cdot 2\pi$ and $W_3 = G^{*2}/(2\pi)$. The $2\pi$ is the Chowla–Selberg normalisation of the period; the $16$ is the count above. Both pieces are independently algebraic.

### 2.4 The constant term [THEOREM by Vieta]

Given Vieta's relation $x_+ x_- = 16\,G^{*3}$, the constant term is determined once the leading coefficient and one root are fixed. Equivalently, the displacement form (1′) — built from the harmonic-centre interpretation of §1.3 — produces the constant term automatically.

---

## Part III: The Discriminant Trichotomy

Generalising the polynomial to

$$x^2 - k\,G^{*2}\,x + k\,G^{*3} \;=\; 0, \qquad k \in \mathbb{R}_{>0}, \tag{3}$$

the discriminant is

$$\Delta(k) \;=\; (k G^{*2})^2 - 4 k G^{*3} \;=\; k\,G^{*3}\,(k\,G^* - 4).$$

One quadratic, three regimes:

| Regime | Condition | Sign of $\Delta$ | Roots | Physical reading |
|---|---|---|---|---|
| **Bosonic** | $k > 4/G^*$ (e.g. $k = 16$) | $\Delta > 0$ | real, distinct | coupling constants $\alpha$, $N_c$ |
| **Critical** | $k = 4/G^*$ | $\Delta = 0$ | real, degenerate | Born-rule / measurement boundary (FOUND_BORN_RULE_NULL_CONE.md; Born-rule derivation status: LEDGER FTD-0187) |
| **Fermionic** | $k < 4/G^*$ | $\Delta < 0$ | complex conjugate, $x = a \pm b\,i$ | $e^{ibt}$ wavefunction oscillation — Dirac sector |

The same polynomial encodes bosons (real roots, the physical $k = 16$ case), the measurement boundary (degenerate roots), and the fermion sector (complex roots, oscillatory in time). The fermion sector is not imported from external physics; it is the complex-discriminant regime of the same equation. [THEOREM for the algebra; SELECTION for the physical readings of each regime.]

---

## Part IV: Physical Identification — Evidence Without Derivation

The larger root $x_+ = 137.036$ is identified with $1/\alpha$ at 1.26 ppm (FTD-0013 [STRONGLY MOTIVATED CONJECTURE]). The conjecture rests on the numerical match plus structural-uniqueness evidence (polynomial-template uniqueness across natural search families). The smaller root $x_- ≈ 3.024$ is a mathematical artifact of $P(x)$; the historical identification `x_-  N_c` is **RETIRED** per v1.4 §5 (LEDGER FTD-0014 removed in commit `ca7eb61`); `N_c = 3` in FTD is independently sourced — see `DERIV_NC_FROM_TOPOLOGY.md`.

### 4.1 The numerical match

- $x_+ = 137.036$ vs $1/\alpha = 137.035999177$ — agreement to 1.26 ppm.
- $x_- = 3.024$ — mathematical artifact of $P(x)$ (no live physics identification post-v1.4 §5).

*(Historical framing: the **simultaneous** match of both roots to two constants of unrelated physical origin (the "dual prediction") was previously load-bearing structural evidence. That framing is retired along with `x_-  N_c`; the polynomial-template-uniqueness fact below replaces it.)*

### 4.2 CM-curve uniqueness across class-number-1 fields

Among the nine imaginary quadratic fields with class number $1$, the discriminant $d = -4$ — corresponding to $E\colon y^2 = x^3 - x$ — is the **unique** curve whose associated polynomial reproduces the historical pair-match $(\,1/\alpha,\;N_c)$ at the original tolerances. (Historical framing reflects the pre-v1.4 target choice; the CM-uniqueness fact itself is independent of the physics interpretation of the second target.) Other class-number-1 CM curves give roots that miss both targets. (See `scripts/exploration/scan_cm_class_number_1.py` and the Option 3 results in CONJ_ALPHA_FROM_CM.md.)

This structural uniqueness — together with FTD-0189's polynomial-template uniqueness across 2.65 M degree-2 polynomials over an 18-constant FTD-undesigned basket — is what distinguishes the master quadratic from "any polynomial in $\Gamma(1/4)$ that hits 137." The space of comparable algebraic objects has been enumerated; only one survives.

### 4.3 What is *not* claimed

This document does not claim:

- That the master quadratic is the L → ∞ limit of a finite-L gap equation. (It is not. The finite-L scan does not converge to (137.036, 3.024); the L = 2 partition function carries no master-quadratic signature; the undefined-boundary ontology rules out load-bearing L → ∞ steps.)
- That the physical identification is a [THEOREM]. The single live identification `x_+  1/α` is [STRONGLY MOTIVATED CONJECTURE], anchored on the empirical match and polynomial-template uniqueness (FTD-0189) plus CM-curve uniqueness. The historical paired `x_-  N_c` identification is retired per v1.4 §5.
- That a dynamical mechanism for $x_+ = 1/\alpha$ has been derived from FTD's local update rules. No such mechanism is currently in hand; the open problem of deriving the Coulomb coupling $g_c$ from first principles is documented in `../10_eft_program/OPEN_GC_FROM_FIRST_PRINCIPLES.md`, and the conversion-factor question is closed by `../10_eft_program/THEOREM_A_PHYS_NO_GO.md` with provenance in `../10_eft_program/archive/resolved/OPEN_A_PHYS_DERIVATION.md`.

---

## Part V: The Derivation Chain

| Step | Content | Status |
|---|---|---|
| 0 | Cubic lattice with no defined boundary; Moore neighbourhood at every site (Axiom Zero) | [AXIOM] |
| 1 | $O_h$ point group; $\mathbb{Z}_4$ planar symmetry | [THEOREM] |
| 2 | Watson BCC integral $W_3 = \Gamma(1/4)^4/(4\pi^3)$ | [THEOREM] |
| 3 | Lemniscatic modulus $k = 1/\sqrt{2}$ forced by $\mathbb{Z}_4$ | [THEOREM] |
| 4 | CM curve $E\colon y^2 = x^3 - x$; $j = 1728$; $\mathrm{Aut}(E) = \mathbb{Z}/4\mathbb{Z}$ | [THEOREM] |
| 5 | Algebraic identity $G^{*2}/(2\pi) = W_3$ (Chowla–Selberg) | [THEOREM] |
| 6 | Coefficient $16$ from two independent routes ($\|\mathrm{Aut}(E)\|^2$ and $z_\mathrm{BCC}\cdot 2$) | [THEOREM] |
| 7 | Master quadratic $x^2 - 16 G^{*2} x + 16 G^{*3} = 0$ assembled algebraically | [THEOREM] |
| 8 | Roots $x_+ = 137.036$, $x_- = 3.024$ (quadratic formula) | [THEOREM] |
| 9 | Discriminant trichotomy: bosons (real), critical (degenerate), fermions (complex) | [THEOREM for the algebra; SELECTION for physical readings] |
| 10 | Physical identification $x_+ \leftrightarrow 1/\alpha$ | [STRONGLY MOTIVATED CONJECTURE] (numerical match + polynomial-template uniqueness FTD-0189 + CM uniqueness). The historical paired $x_- \leftrightarrow N_c$ identification is **RETIRED** per v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`; `N_c = 3` independently sourced via `DERIV_NC_FROM_TOPOLOGY.md`. |

Steps 0–8 are algebra and finite combinatorics, all [THEOREM]. Step 9 is algebraic for the trichotomy itself; identifying each regime with bosons / measurement / fermions is [SELECTION]. Step 10 is conjectural but evidentially constrained.

---

## Part VI: What This Establishes

1. **[THEOREM]** The master quadratic is an algebraic identity built from $G^*$ and the integer $16$, both finite-combinatorial / closed-form.
2. **[THEOREM]** The Watson–G\* identity $W_3 = G^{*2}/(2\pi)$ is exact via Chowla–Selberg.
3. **[THEOREM]** The coefficient $16$ has two independent finite-combinatorial routes.
4. **[THEOREM]** The discriminant trichotomy partitions one polynomial into bosonic, critical, and fermionic regimes.
5. **[STRONGLY MOTIVATED CONJECTURE]** The roots $x_+$ and $x_-$ identify with $1/\alpha$ and $N_c$, supported by the dual numerical match (1.26 ppm and 0.80 %) and CM-curve uniqueness across class-number-1 fields.

What it does **not** establish:

- A dynamical derivation of $1/\alpha$ from FTD's local update rules.
- Convergence of any finite-L self-consistency equation to the master quadratic. (The earlier "thermodynamic limit" framing is withdrawn; see AUDIT_INFINITY_REFRAME.md and AUDIT_MASTER_QUADRATIC.md.)
- A first-principles derivation of the Coulomb coupling $g_c$ (open: OPEN_GC_FROM_FIRST_PRINCIPLES.md).
- A first-principles derivation of the lattice-to-physical conversion $a_\mathrm{phys}$ (closed negative by `../10_eft_program/THEOREM_A_PHYS_NO_GO.md`; provenance: `../10_eft_program/archive/resolved/OPEN_A_PHYS_DERIVATION.md`).

---

## Part VII: Transfer-Matrix Interpretation [THEOREM, notational]

**Added 2026-05-06 (FQCR integration).**

The Finite Quarter-Conjugacy Recurrence framework (`SPEC_FQCR.md`) provides an **operator interpretation** of the master quadratic that complements the algebraic / number-theoretic framings of Parts I–IV. This is a notational identity, not a new physical claim.

### VII.1 — The transfer matrix

Define, for $N \ge 1$ and a real parameter $t$,

$$ M_N(t) \;:=\; 16\,(G_N^*)^2 \begin{pmatrix} 1 & -\dfrac{R_N(t)}{16\,G_N^*} \\[4pt] 1 & 0 \end{pmatrix} $$

where $G_N^*$ is the finite-N approximation to $G^*$ defined in `DERIV_GSTAR_FINITE_APPROX.md` and $R_N(t)$ is a finite renormalisation factor specified in `SPEC_FQCR.md` §3.3.

### VII.2 — Characteristic polynomial = master quadratic

By direct computation,

$$ \det(x I - M_N(t)) \;=\; x^2 \;-\; 16\,(G_N^*)^2\,x \;+\; 16\,(G_N^*)^3\,R_N(t). $$

**At $R_N(t) = 1$ and $N \to \infty$:** this reduces to

$$ x^2 - 16\,G^{*2}\,x + 16\,G^{*3} = 0, $$

which is **exactly** the master quadratic of Part I. The roots $x_+$ and $x_-$ are then the eigenvalues of $M_\infty(R = 1)$.

### VII.3 — What this notational equivalence does and does NOT establish

**Establishes:**

- The master quadratic admits an operator interpretation as the spectrum of a finite 2×2 transfer matrix.
- The "deformation parameter" $R_N(t)$ provides a structured family of polynomials sharing the master quadratic's algebraic shape; the canonical choice $R = 1$ recovers Theorem 2 exactly.
- Within FQCR, the dominant root reading $x_+ = \lambda_\text{max}(M_\infty(1))$ gives a notational anchor for the [STRONGLY MOTIVATED CONJECTURE] $\alpha^{-1} = 1/x_+$ (FTD-0013) without changing its tag.

**Does NOT establish:**

- That the transfer-matrix structure is *unique* or *forced* — one can always wrap a quadratic into a 2×2 companion matrix; the FQCR construction makes a specific choice for the off-diagonal placement of $R_N$.
- That $\alpha^{-1} = \lambda_\text{max}$ is *derived* — the physical identification stays at [STRONGLY MOTIVATED CONJECTURE] (FTD-0013), unchanged.
- That the renormalisation $R_N(t)$'s specific form $1 + \lambda_N + A_N$ is *necessary* — the Model IV exponent quadruple $(4, 6; 3, 2)$ is at [SELECTION] pending the uniqueness scan in `PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md` (FTD-0143).

The operator interpretation is therefore complementary to the algebraic / number-theoretic provenance in Parts I–IV; it does not replace them.

---

## References

- DERIV_WATSON_GSTAR_IDENTITY.md — $W_3 = G^{*2}/(2\pi)$ and the Chowla–Selberg connection.
- DERIV_QUADRATIC_NECESSITY.md — Why degree 2 is the minimal self-referential closure.
- DERIV_DUAL_DERIVATION_OF_16.md — $|\mathrm{Aut}(E)|^2 = 16$ and the dual-route argument.
- FOUND_DIMENSIONAL_COUNTING.md — $z_\mathrm{BCC} \cdot 2 = 16$ and the Moore decomposition.
- FOUND_BORN_RULE_NULL_CONE.md — Discriminant-zero / measurement-boundary identification.
- CONJ_ALPHA_FROM_CM.md — CM-curve uniqueness across class-number-1 fields.
- AUDIT_MASTER_QUADRATIC.md — Full epistemic audit of the master quadratic.
- AUDIT_INFINITY_REFRAME.md — Undefined-boundary ontology, position-property axiom.
- OPEN_GC_FROM_FIRST_PRINCIPLES.md — Coulomb-coupling derivation status.
- `../10_eft_program/archive/resolved/OPEN_A_PHYS_DERIVATION.md` — archived lattice-to-physical-length conversion status.
