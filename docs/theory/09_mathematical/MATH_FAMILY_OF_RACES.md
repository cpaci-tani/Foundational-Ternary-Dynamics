# The Family of Residue-Class Races: Gamma Ratios, Duplication, and the Universal Odd-Zeta Spine

**Status:** [THEOREM] for the product identities, duplication law, and odd-zeta spine; [SELECTION] for the physical interpretation of q=4

---

## 1. Definition

For each integer q >= 2, define the **q-th race constant**:

$$R_q := \lim_{N \to \infty} N^{(2-q)/q} \prod_{k=0}^{N-1} \frac{qk + q - 1}{qk + 1}$$

**Theorem (Closed Form).** [THEOREM]

$$R_q = \frac{\Gamma(1/q)}{\Gamma(1 - 1/q)}$$

*Proof.* The product is the Pochhammer ratio $((q-1)/q)_N / (1/q)_N$. By Stirling, $(a)_N \sim N^a \cdot \Gamma(N)/\Gamma(a)$, so the ratio grows as $N^{(q-2)/q} \cdot \Gamma(1/q)/\Gamma((q-1)/q)$. Dividing by $N^{(q-2)/q}$ gives the result. $\square$

The first cases are:

$$R_2 = 1, \qquad R_3 = \frac{\Gamma(1/3)}{\Gamma(2/3)}, \qquad R_4 = \frac{\Gamma(1/4)}{\Gamma(3/4)} = G^*$$

Thus the classical Wallis normalization, the cubic equianharmonic ratio, and the lemniscatic bridge constant all arise as members of a single residue-class family.

---

## 2. The First Constants

| q | R_q | Lattice / Number Field | CM Curve | |Aut| |
|---|-----|----------------------|----------|-------|
| 2 | 1.0000 | trivial | — | — |
| 3 | 1.9784 | hexagonal, Z[omega], j=0 | y^2=x^3+1 | 6 |
| 4 | 2.9587 = G* | square, Z[i], j=1728 | y^2=x^3-x | 4 |
| 5 | 3.9432 | pentagonal | — | — |
| 6 | 4.9312 | factors through q=3 | — | — |
| 7 | 5.9218 | heptagonal | — | — |
| 8 | 6.9141 | factors through q=4 | — | — |

---

## 3. The Reflection Pairing

Each race has a **product** and a **ratio**:

$$\text{Product:} \quad \Gamma(1/q) \cdot \Gamma(1 - 1/q) = \frac{\pi}{\sin(\pi/q)}$$

$$\text{Ratio:} \quad R_q = \frac{\Gamma(1/q)}{\Gamma(1 - 1/q)}$$

The product is fixed by the reflection formula — it encodes the angle pi/q. The ratio is the remaining degree of freedom — it reflects arithmetic data modulo q (Dirichlet characters, cyclotomic structure). For q=3 and q=4 this aligns precisely with the equianharmonic and lemniscatic elliptic cases; for general q it captures the residue-class arithmetic without a direct elliptic interpretation.

---

## 4. Duplication Hierarchy

If R(s) := Gamma(s)/Gamma(1-s), then:

$$R\!\left(\frac{s}{2}\right) = 2^{1-2s} \; R(s) \; R\!\left(\frac{1-s}{2}\right)$$

**[THEOREM]** — follows from the Legendre duplication formula.

Hence the family is hierarchical rather than flat: constants at finer denominators factor through constants at coarser denominators, up to explicit algebraic powers of 2.

Specific instances:

$$R_6 = R(1/6) = 2^{1/3} \cdot R_3^2$$

$$R_8 = R(1/8) = \sqrt{2} \cdot R_4 \cdot R(3/8)$$

$$R_{10} = R(1/10) = 2^{3/5} \cdot R_5 \cdot R(2/5)$$

**Remark.** Many composite levels factor through simpler levels via multiplication formulas, with duplication giving the cleanest hierarchy. The full structure of which levels decompose and how is governed by the Gauss multiplication formula for Gamma.

---

## 5. Universal Odd-Zeta Spine

For |s| < 1:

$$\log R(s) = -\log s - 2\gamma s - 2\sum_{m=1}^{\infty} \frac{\zeta(2m+1)}{2m+1} \; s^{2m+1}$$

**[THEOREM]**

*Proof.* From the Taylor series log Gamma(1+z) = -gamma*z + sum_{n>=2} (-1)^n zeta(n)/n * z^n, we obtain:

- log Gamma(s) = -log(s) - gamma*s + sum_{n>=2} (-1)^n zeta(n)/n * s^n
- log Gamma(1-s) = gamma*s + sum_{n>=2} zeta(n)/n * s^n

The difference log R(s) = log Gamma(s) - log Gamma(1-s) has coefficients [(-1)^n - 1] * zeta(n)/n. For even n this vanishes; for odd n = 2m+1 it equals -2*zeta(2m+1)/(2m+1). $\square$

Specializing to s = 1/q:

$$\log R_q = \log q - \frac{2\gamma}{q} - 2\sum_{m=1}^{\infty} \frac{\zeta(2m+1)}{2m+1} \; q^{-(2m+1)}$$

### The Critical Observation

Every race carries the **same** universal odd-zeta tower. The even zeta values (zeta(2) = pi^2/6, zeta(4) = pi^4/90, ...) cancel identically in log R(s). Only the odd values survive.

This means:
- **zeta(3)** appears in every log R_q with coefficient -2/3 * q^{-3}
- **zeta(5)** appears in every log R_q with coefficient -2/5 * q^{-5}
- The particular modulus-q refinements enter only through the Dirichlet-character decomposition of rational-point local data

### Why Odd Zeta Values Are Hard

The even L-values (Catalan's constant G = L(2, chi_{-4}), L(2, chi_{-3}), ...) are **particular**: each belongs to one tower and one number field.

The odd zeta values (zeta(3), zeta(5), zeta(7), ...) are **universal**: they belong to all towers and all number fields simultaneously.

Particular constants live in one house. Universal constants live in every house at once. That is why no single house can determine them.

- zeta(2) = pi^2/6 is "easy" because it is determined by the q=2 tower alone.
- zeta(3) is "hard" because it appears in every tower but is determined by none.

Apery proved zeta(3) is irrational (1978), but its algebraic nature remains unknown precisely because it is not the invariant of any single number field — it is the invariant of the entire family.

---

## 6. The Two Elliptic Cases: q=3 and q=4

The values q=3 and q=4 are distinguished by their direct connection to elliptic curves with extra automorphisms: j=0 (|Aut|=6) and j=1728 (|Aut|=4) respectively. These are the two elliptic curves over Q whose automorphism groups exceed the generic Z/2Z.

### q = 3: The Hexagonal Lattice

- Number field: Q(omega), omega = e^{2*pi*i/3}, discriminant -3
- CM curve: E_3: y^2 = x^3 + 1, j-invariant = 0
- |Aut(E_3)| = 6 (Z/6Z, generated by (x,y) -> (omega*x, -y))
- R_3 = Gamma(1/3)/Gamma(2/3) = 1.97836...
- Hypothetical quadratic: x^2 - 36*R_3^2*x + 36*R_3^3 = 0
  - x+ = 138.894 (not 1/alpha)
  - x- = 2.007, floor(x-) = 2 (not 3)
  - Harmonic mean = 2 = [Q(omega):Q]

### q = 4: The Square Lattice

- Number field: Q(i), discriminant -4
- CM curve: E_4: y^2 = x^3 - x, j-invariant = 1728
- |Aut(E_4)| = 4 (Z/4Z, generated by (x,y) -> (-x, iy))
- R_4 = Gamma(1/4)/Gamma(3/4) = G* = 2.95868...
- Master quadratic: x^2 - 16*G*^2*x + 16*G*^3 = 0
  - x+ = 137.036 = 1/alpha (1.26 ppm)
  - x- = 3.024, floor(x-) = 3 = N_c
  - Harmonic mean = 2 = [Q(i):Q]

### Why q = 4 and Not q = 3?

The hexagonal case fails the physical test on both outputs:
1. x+ = 138.89, off from alpha^{-1} by ~1.4% (vs 1.26 ppm for q=4)
2. floor(x-) = 2, which would give 2 colors (vs 3 for q=4)

Both q=3 and q=4 produce valid quadratics with H = 2. As a model-selection heuristic, q=4 fits the observed coupling constant and color number while q=3 does not. Whether this selection can be derived from an independent physical principle (rather than imposed by comparison with experiment) remains open.

**[SELECTION]**: The cubic lattice Z^3 (q=4) rather than the hexagonal lattice A_2 x Z (q=3) is identified with the physical vacuum structure. This identification is supported by the numerical match but not derived from first principles.

---

## 7. The Composite Nature of Varpi (Generalized)

At each q, the lemniscate-like constant is:

$$\varpi_q = R_q \cdot \frac{\sqrt{\pi/\sin(\pi/q)}}{2}$$

For q=4: varpi_4 = G* * sqrt(pi)/2 = varpi (the classical lemniscate constant).

Each varpi_q is composite — it factors into the arithmetic piece R_q and the geometric piece sqrt(pi/sin(pi/q)). No single Wallis product produces varpi_q for q >= 3; at least two independent races are required.

---

## 8. Summary

```
q=2:  R = 1         Trivial. No arithmetic content.
q=3:  R = 1.978...  Hexagonal. CM curve j=0. x+ = 138.89 (wrong physics).
q=4:  R = 2.959...  Square. CM curve j=1728. x+ = 137.036 = 1/alpha.  <-- PHYSICS
q=5:  R = 3.943...  Pentagonal. No CM curve (class number > 1).
q=6:  Factors through q=3.
q=7:  R = 5.922...  Heptagonal. No CM curve.
q=8:  Factors through q=4.
...
q->inf: R -> q - 2*gamma + O(1/q).
```

---

## References

- Gauss, C. F. (1812). *Disquisitiones generales circa seriem infinitam*.
- Legendre, A.-M. (1811). *Exercices de calcul integral*.
- Apery, R. (1978). Irrationalite de zeta(2) et zeta(3). *Asterisque* 61, 11-13.
- Nesterenko, Yu. V. (1996). Modular functions and transcendence questions. *Mat. Sb.* 187, 65-96.
- Zucker, I. J. & Joyce, G. S. (2001). Special values of the hypergeometric series. *Math. Proc. Cambridge Phil. Soc.* 131, 309-319.
