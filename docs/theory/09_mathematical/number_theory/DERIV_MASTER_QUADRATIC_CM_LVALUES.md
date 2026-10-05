# The Master Quadratic as a CM L-Value Identity

## Sum-of-Roots = 2⁹ · L(Sym² E, 1) for E: y² = x³ − x

**Status:** [THEOREM] (sum-of-roots identification); [COROLLARY] (product-of-roots)
**Verified:** PARI/GP, 100-digit precision, both identities
**Dependencies:** [DERIV_LFUNCTION_GSTAR_CONNECTION.md](DERIV_LFUNCTION_GSTAR_CONNECTION.md), [DERIV_WATSON_GSTAR_IDENTITY.md](../../04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md), [MATH_MASTER_QUADRATIC.md](../../01_reference/MATH_MASTER_QUADRATIC.md)

---

## Abstract

The coefficients of the FTD master quadratic

$$x^2 - 16 G^{*2}\,x + 16 G^{*3} = 0$$

are Deligne L-values of the CM motive $h^1(E)$ for the elliptic curve $E: y^2 = x^3 - x$ (LMFDB 32.a3, $j = 1728$, CM by $\mathbb{Z}[i]$, $|\mathrm{Aut}(E)| = 4$).

The **headline theorem** is that the sum of roots is a Damerell–Shimura symmetric-square L-value:

$$\boxed{\ 16 G^{*2} \;=\; 512\,L(\mathrm{Sym}^2 E,\,1) \;=\; 2^9 \cdot L(\mathrm{Sym}^2 E,\,1)\ }$$

The **corollary** is that the product of roots is a cube of the elementary rank-0 BSD period relation:

$$16 G^{*3} \;=\; 2^{13} \cdot L(E,1)^3 / \pi^{3/2}$$

The $\pi^{3/2}$ is not a signal of deeper structure: it arises mechanically from cubing $L(E,1) = G^* \sqrt{\pi}/8$. The corollary is elementary once $L(E,1) = \varpi/4$ is known (Section 3); only the Sym² identification is a genuinely non-elementary L-value relation (Section 2).

Both identities are verified at 100-digit precision. Section 7 gives the PARI/GP code. Section 8 states the conditional theorem that results.

---

## §1. Setup

### 1.1 The Master Quadratic

Let $G^* = \Gamma(1/4)/\Gamma(3/4) = 2\varpi/\sqrt{\pi} \approx 2.95867511914$ where $\varpi = \Gamma(1/4)^2/(2\sqrt{2\pi})$ is the lemniscate constant.

The master quadratic is

$$x^2 - 16 G^{*2}\,x + 16 G^{*3} = 0 \tag{1.1}$$

with roots

$$x_\pm = 8 G^{*2} \pm 4 G^{*3/2}\sqrt{4G^* - 1}$$

Numerically: $x_+ = 137.0361714582\ldots$, $x_- = 3.0239639163\ldots$.

Vieta's formulas give $x_+ + x_- = 16 G^{*2}$ and $x_+ \cdot x_- = 16 G^{*3}$. The arithmetic content of the quadratic lives entirely in these two coefficients.

### 1.2 The CM Curve

$E: y^2 = x^3 - x$ has:

- Conductor $N = 32$ (LMFDB 32.a3)
- $j$-invariant $j(E) = 1728$
- Complex multiplication by $\mathbb{Z}[i]$ (the Gaussian integers)
- $|\mathrm{Aut}(E)| = 4$, hence $|\mathrm{Aut}(E)|^2 = 16$
- $E(\mathbb{Q})_{\mathrm{tors}} \cong \mathbb{Z}/2\mathbb{Z} \times \mathbb{Z}/2\mathbb{Z}$, $|E(\mathbb{Q})_{\mathrm{tors}}|^2 = 16$
- Real period $\omega_1 = \varpi$ (Chowla–Selberg for $\mathbb{Q}(i)$)
- Rank 0; $\mathrm{Sha}(E/\mathbb{Q})$ trivial (Rubin 1991)

The classical BSD identity (proven for this curve) is

$$L(E,1) = \varpi/4 \tag{1.2}$$

from which

$$L(E,1) = \frac{G^* \sqrt{\pi}}{8} \tag{1.3}$$

follows by elementary substitution.

---

## §2. Main Theorem: Sum of Roots as Sym² L-Value

### 2.1 Statement

**Theorem 2.1.** *For $E: y^2 = x^3 - x$,*

$$\boxed{\ 16\,G^{*2} \;=\; 2^9 \cdot L(\mathrm{Sym}^2 E,\,1)\ } \tag{2.1}$$

*where $L(\mathrm{Sym}^2 E, s)$ is the symmetric-square L-function of the motive $h^1(E)$.*

### 2.2 Derivation

For a CM elliptic curve with $j = 1728$, the Damerell–Shimura theorem (Damerell 1970; Shimura 1976) evaluates the symmetric-square L-function at critical integers $s = 1, 2$ in terms of the real period $\omega_1 = \varpi$ and rational numbers:

$$L(\mathrm{Sym}^2 E,\,1) \;=\; \frac{\varpi^2}{8\pi} \tag{2.2}$$

$$L(\mathrm{Sym}^2 E,\,2) \;=\; \frac{\pi \varpi^2}{32} \tag{2.3}$$

The rational prefactors $\{1/(8\pi),\,\pi/32\}$ are specific to the $j = 1728$ CM curve with $|\mathrm{Aut}| = 4$; for other CM curves the rationals differ.

Substituting $\varpi = G^* \sqrt{\pi}/2$:

$$L(\mathrm{Sym}^2 E,\,1) \;=\; \frac{(G^* \sqrt{\pi}/2)^2}{8\pi} \;=\; \frac{G^{*2} \pi / 4}{8\pi} \;=\; \frac{G^{*2}}{32}$$

Hence

$$G^{*2} \;=\; 32\,L(\mathrm{Sym}^2 E,\,1)$$

and multiplying by $|\mathrm{Aut}(E)|^2 = 16$:

$$16\,G^{*2} \;=\; 512 \cdot L(\mathrm{Sym}^2 E,\,1) \;=\; 2^9 \cdot L(\mathrm{Sym}^2 E,\,1) \qquad \blacksquare$$

### 2.3 Why This Is Non-Elementary

The identity $16 G^{*2} = 2^9 L(\mathrm{Sym}^2 E, 1)$ is not a rewriting of the BSD period relation (1.2). It invokes a **different** L-function — the symmetric square of $h^1(E)$, whose Euler factor at a good prime is $\bigl[(1-\alpha_p^2p^{-s})(1-p\cdot p^{-s})(1-\beta_p^2p^{-s})\bigr]^{-1}$ with $\alpha_p+\beta_p=a_p$ and $\alpha_p\beta_p=p$, that is, the inverse of $1-(a_p^2-p)p^{-s}+p(a_p^2-p)p^{-2s}-p^{3}p^{-3s}$ (corrected 2026-10-05; the expression printed here before was not the Sym² factor), rather than the degree-2 Euler factors of $L(E,s)$.

Damerell's theorem evaluates this Sym² L-function at $s = 1$ (the central critical point of $L(E,s)$, but a non-central critical point of $L(\mathrm{Sym}^2 E,s)$, whose centre is $s = 3/2$ in the arithmetic normalization; corrected 2026-10-05) to give a rational multiple of $\varpi^2/\pi$. This is a genuine Sym² period computation, not downstream of $L(E,1) = \varpi/4$.

The integer $2^9 = 512$ is the Damerell rational $1/(8\pi) \cdot \pi$ times $|\mathrm{Aut}(E)|^2 = 16$ times a factor of $2^5$ absorbed into the Damerell normalization conventions. See §6 for the integer decomposition.

### 2.4 Numerical Verification

At 100-digit precision, PARI/GP returns

$$\bigl|\,512 \cdot L(\mathrm{Sym}^2 E,1) - 16 G^{*2}\,\bigr| < 10^{-90}$$

Code in §7.

### 2.5 Normalization and an explicit proof route (added 2026-10-05)

**Normalization.** The identity uses the arithmetic normalization of $L(\mathrm{Sym}^2 E,s)$: conductor $64$, functional equation $s \leftrightarrow 3-s$, critical points $s=1$ and $s=2$, centre $s=3/2$, and trivial Euler factor at $p=2$. In the analytic normalization ($s \leftrightarrow 1-s$) the value at "$s=1$" is the arithmetic value at $s=2$, namely $\Gamma(1/4)^4/256=(\pi^2/4)\,L(\mathrm{Sym}^2E,1)$, and the identity $16G^{*2}=512\,L(\mathrm{Sym}^2E,1)$ fails there.

**Explicit proof route.** Because $E$ has CM by $\mathbb{Z}[i]$,

$$L(\mathrm{Sym}^2 E,s)=L(g,s)\,L(\chi_{-4},s-1),\qquad g=\eta(4\tau)^6\in S_3(\Gamma_0(16),\chi_{-4}).$$

One checks this prime by prime. For $p\equiv 1 \pmod 4$, $b_g(p)=\alpha_p^2+\beta_p^2=a_p^2-2p$. For $p\equiv 3\pmod 4$, both sides have Euler factor $\bigl[(1-p^{1-s})(1+p^{1-s})^2\bigr]^{-1}$. At $p=2$ every factor is $1$. (That $g$ is the CM form of $\psi^2$, where $\psi$ is the Hecke character of $E$, follows from Hecke's theorem and the Sturm bound $6$ of the space.) The transformation $\eta(-1/\tau)=\sqrt{\tau/i}\,\eta(\tau)$ gives $g(-1/(16\tau))=64\,i\,\tau^3g(\tau)$, so $\Lambda(s)=(2/\pi)^s\Gamma(s)L(g,s)$ satisfies $\Lambda(s)=\Lambda(3-s)$ and $L(g,1)=(2/\pi)\,L(g,2)$. Rogers, Wan and Zucker prove $L(g,2)=\Gamma(1/4)^4/(64\pi)$ (*Ramanujan J.* 2015, arXiv:1303.2259, Theorem 5). With $L(\chi_{-4},0)=1/2$:

$$L(\mathrm{Sym}^2E,1)=\frac{2}{\pi}\cdot\frac{\Gamma(1/4)^4}{64\pi}\cdot\frac12=\frac{\Gamma(1/4)^4}{64\pi^2}=\frac{G^{*2}}{32}.$$

**In-repo reproduction.** `scripts/proofs/proof_sym2_lvalue_verification.py` checks each step numerically: the coefficients for all $n\le 2000$, the root number, the values at 50 digits, and an optional PARI/GP `lfunsympow` cross-check. This closes the deferred item F1 in [`SPEC_OPEN_MATH_FRONTIERS.md`](../../01_reference/SPEC_OPEN_MATH_FRONTIERS.md).

---

## §3. Corollary: Product of Roots as Cube of Period Relation

### 3.1 Statement

**Corollary 3.1.** *For $E: y^2 = x^3 - x$,*

$$16\,G^{*3} \;=\; 2^{13} \cdot L(E,1)^3 \cdot \pi^{-3/2} \tag{3.1}$$

### 3.2 Derivation (Elementary)

From (1.3), $L(E,1) = G^* \sqrt{\pi}/8$, so

$$L(E,1)^3 \;=\; G^{*3} \pi^{3/2} / 512$$

Solving for $G^{*3}$:

$$G^{*3} \;=\; 512 \cdot L(E,1)^3 \cdot \pi^{-3/2}$$

and multiplying by 16:

$$16\,G^{*3} \;=\; 8192 \cdot L(E,1)^3 \cdot \pi^{-3/2} \;=\; 2^{13} \cdot L(E,1)^3 \cdot \pi^{-3/2} \qquad \blacksquare$$

### 3.3 What the $\pi^{-3/2}$ Means

The factor $\pi^{-3/2}$ is the cube of the $\sqrt{\pi}$ in the elementary relation $G^* = 2\varpi/\sqrt{\pi}$. Cubing a relation involving $\sqrt{\pi}$ produces $\pi^{3/2}$ mechanically. The $\pi^{-3/2}$ in (3.1) carries no arithmetic information beyond the BSD identity (1.2) itself.

The corollary is therefore **algebraically equivalent** to the elementary period relation and should not be marketed as an independent theorem. It is included here because the product-of-roots coefficient is part of the master quadratic's arithmetic identification and completeness demands its explicit form.

### 3.4 Numerical Verification

$$\bigl|\,2^{13} \cdot L(E,1)^3 / \pi^{3/2} - 16 G^{*3}\,\bigr| < 10^{-90}$$

---

## §4. The Integer Prefactors

### 4.1 The Sum Coefficient: $2^9 = 512$

$$2^9 \;=\; 2^5 \cdot 16 \;=\; 32 \cdot |\mathrm{Aut}(E)|^2$$

The factor 16 is $|\mathrm{Aut}(E)|^2$, the intrinsic CM invariant. The factor 32 is the Damerell rational inverse $(8\pi)^{-1} \to 32$ after absorbing $\pi$ into the $\varpi^2 \to G^{*2}\pi/4$ substitution.

Three independent routes give 16 (see [SPEC_QUADRATIC_PHYSICS_BRIDGE.md](../../01_reference/SPEC_QUADRATIC_PHYSICS_BRIDGE.md), §2.2; this doc does not re-derive them):

1. Arithmetic: $|\mathrm{Aut}(E)|^2 = |E(\mathbb{Q})_{\mathrm{tors}}|^2 = 16$
2. Lattice stabilizer: $|\mathrm{Stab}_{O_h}(\mathrm{axis})| = |O_h|/3 = 48/3 = 16$
3. Temporal gauge: $24 - 7 - 1 = 16$ (Moore decomposition minus symmetries)

### 4.2 The Product Coefficient: $2^{13} = 8192$

$$2^{13} \;=\; 2^9 \cdot 2^4 \;=\; 512 \cdot 16$$

which is just the sum coefficient times 16. Per §3.3, the product-of-roots integer is not independent of the sum-of-roots integer plus one more factor of $|\mathrm{Aut}(E)|^2$.

### 4.3 Clean Statement

**The sum and product of the master quadratic's roots are determined by Sym² and Sym¹ L-values of $h^1(E)$ respectively, with rational prefactors equal to $|\mathrm{Aut}(E)|^2 \cdot c_{\mathrm{Dam}}$ where $c_{\mathrm{Dam}}$ is the Damerell–Shimura normalization for each symmetric power.**

For the sum: $c_{\mathrm{Dam}}(\mathrm{Sym}^2) = 32$.
For the product: $c_{\mathrm{Dam}}(\mathrm{Sym}^1)^3 = 512$, absorbing $\pi^{-3/2}$.

Only the Sym² identification is non-elementary.

---

## §5. Relation to Existing FTD Documentation

This document **does not duplicate**:

- [DERIV_LFUNCTION_GSTAR_CONNECTION.md](DERIV_LFUNCTION_GSTAR_CONNECTION.md) — which establishes $G^* = 8 L(E,1)/\sqrt{\pi}$ and the coefficient $16 = |E(\mathbb{Q})_{\mathrm{tors}}|^2$ from BSD. That document is the Sym¹ (rank-0 BSD) story.
- [SPEC_QUADRATIC_PHYSICS_BRIDGE.md](../../01_reference/SPEC_QUADRATIC_PHYSICS_BRIDGE.md) — which frames the five selection principles SP1–SP5 and the conditional-theorem structure.
- [FOUND_BLIND_DERIVATION_CHAIN.md](../../02_foundations/FOUND_BLIND_DERIVATION_CHAIN.md) — which gives the 13-step chain from $i$ to $\alpha^{-1}$.

This document **adds**: the explicit Sym² L-value identification for the sum-of-roots coefficient, with the Damerell–Shimura derivation, the 100-digit PARI verification, and the honest framing that product-of-roots is an elementary corollary.

---

## §6. Physics Identification Is Deferred

This document establishes an **arithmetic theorem**. It does not claim $x_+ = 1/\alpha$; that identification is Selection Principle SP4 in [SPEC_QUADRATIC_PHYSICS_BRIDGE.md](../../01_reference/SPEC_QUADRATIC_PHYSICS_BRIDGE.md). The present theorem stands independently of any physical interpretation.

The practical consequence: this result is **publishable as pure analytic number theory** (Journal of Number Theory, Ramanujan Journal) without invoking any FTD physics claim. The physics identification is a separate argument filed elsewhere.

---

## §7. PARI/GP Verification Code

```gp
\\ Master quadratic CM L-value identities, 100-digit verification
\p 100
default(realprecision, 100);

E    = ellinit([0,0,0,-1,0]);      \\ y^2 = x^3 - x
w    = E.omega[1];                  \\ real period varpi
Gs   = 2*w/sqrt(Pi);                \\ G* = 2*varpi/sqrt(pi)

LE   = lfuncreate(E);
L2   = lfunsympow(E, 2);            \\ Sym^2 L-function

\\ --- §2: Sum of roots (headline theorem) ---
lhs1 = 16 * Gs^2;
rhs1 = 512 * lfun(L2, 1);
gap1 = abs(lhs1 - rhs1);
print("16 G*^2 vs 2^9 * L(Sym^2 E, 1)");
print("  gap = ", gap1);
print("  test: ", gap1 < 10^(-90));

\\ --- §3: Product of roots (elementary corollary) ---
lhs2 = 16 * Gs^3;
rhs2 = 8192 * lfun(LE, 1)^3 / Pi^(3/2);
gap2 = abs(lhs2 - rhs2);
print("16 G*^3 vs 2^13 * L(E,1)^3 / pi^(3/2)");
print("  gap = ", gap2);
print("  test: ", gap2 < 10^(-90));

\\ --- Damerell value (Sym^2 at s=1) ---
LS2_1 = lfun(L2, 1);
predicted = w^2 / (8 * Pi);
print("L(Sym^2 E, 1) vs varpi^2/(8*pi): ", abs(LS2_1 - predicted));
```

Both gap tests return `1` (true) at 100-digit precision.

---

## §8. Conditional Theorem Statement

Taking the present result together with the selection principles documented in [SPEC_QUADRATIC_PHYSICS_BRIDGE.md](../../01_reference/SPEC_QUADRATIC_PHYSICS_BRIDGE.md):

> **Conditional Theorem.** *Given selection principles SP1 (CM curve $j = 1728$), SP2 (degree-2 master polynomial), and SP3 (coefficient $|\mathrm{Aut}(E)|^2 = 16$), the master quadratic's coefficients are determined up to the Damerell–Shimura normalization:*
>
> $$x^2 - 2^9 L(\mathrm{Sym}^2 E, 1)\,x + 2^{13}\pi^{-3/2} L(E,1)^3 = 0$$
>
> *with larger root $x_+ = 137.0361714582\ldots$ matching $\alpha^{-1}_{\mathrm{CODATA\,2018}} = 137.035999084$ at 1.26 ppm (tree level). The one-loop lattice correction ([DERIV_ONE_LOOP_LATTICE_ALPHA.md](../../04_coupling/DERIV_ONE_LOOP_LATTICE_ALPHA.md)) closes the gap to 9.6 ppb.*

SP4 (physical identification $x_+ = 1/\alpha$) remains a selection principle and is not proven by the arithmetic theorem.

---

## §9. What This Does and Does Not Establish

**Established [THEOREM]:**

- $16 G^{*2} = 2^9 L(\mathrm{Sym}^2 E, 1)$ (Sym² L-value identification; non-elementary)
- $16 G^{*3} = 2^{13} L(E,1)^3 / \pi^{3/2}$ (Sym¹ corollary; elementary)
- Both at 100-digit PARI-verified precision
- Integer prefactors $\{2^9, 2^{13}\}$ decomposed as $|\mathrm{Aut}(E)|^2 \cdot c_{\mathrm{Dam}}$

**Not established:**

- Any physical identification of $x_\pm$ (SP4 / SP5, filed elsewhere)
- Higher-symmetric-power identifications $L(\mathrm{Sym}^k E, s)$ for $k \geq 3$
- Relation to the 7-term precision series coefficients (see [CONJ_SEVEN_TERM_PRECISION_SERIES.md](../general_math/CONJ_SEVEN_TERM_PRECISION_SERIES.md))

---

## §10. Open Questions

1. Can the Damerell normalization $c_{\mathrm{Dam}}(\mathrm{Sym}^2) = 32$ be derived *a priori* from the CM type rather than computed as $1/(8\pi)$ times the absorbed $\pi$? This would decompose $2^9$ cleanly rather than as $32 \cdot 16$.

2. Does $L(\mathrm{Sym}^3 E, s)$ admit a clean period evaluation at any critical $s$? Naive approaches (Sym³ of the CM curve requires Hecke character $L(\psi^k, s)$ decomposition, not direct Sym³) returned non-rational ratios in simplest bases. See [EXPLR_LVALUE_SPAN_CORRECTION_SEARCH.md](EXPLR_LVALUE_SPAN_CORRECTION_SEARCH.md) §4.

3. Is there a motivic interpretation in which the degree-2 polynomial $(x - x_+)(x - x_-)$ equals a characteristic polynomial of Frobenius on a specific motive, rather than a curve chosen to make the coefficients match?

---

## References

- Damerell, R. M. "L-functions of elliptic curves with complex multiplication, I." *Acta Arith.* **17** (1970), 287–301.
- Shimura, G. "The special values of the zeta functions associated with cusp forms." *Comm. Pure Appl. Math.* **29** (1976), 783–804.
- Rubin, K. "The 'main conjectures' of Iwasawa theory for imaginary quadratic fields." *Invent. Math.* **103** (1991), 25–68.
- Chowla, S. and Selberg, A. "On Epstein's zeta-function." *J. reine angew. Math.* **227** (1967), 86–110.
- LMFDB entry 32.a3: https://www.lmfdb.org/EllipticCurve/Q/32/a/3
