# DERIV · G\* via Quarter-Conjugacy Determinant Bridge

**Tag:** [THEOREM] (subject to standard zeta-regularization conventions)
**Date:** 2026-05-06
**LEDGER row reservation:** FTD-0141
**Companion docs:** [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md), [`DERIV_GSTAR_FINITE_APPROX.md`](DERIV_GSTAR_FINITE_APPROX.md), [`DERIV_G_STAR_PARITY_TWIST.md`](DERIV_G_STAR_PARITY_TWIST.md) (FTD-0127), [`DERIV_LFUNCTION_GSTAR_CONNECTION.md`](../09_mathematical/DERIV_LFUNCTION_GSTAR_CONNECTION.md), [`SPEC_ALGEBRAIC_SPINE.md`](../01_reference/SPEC_ALGEBRAIC_SPINE.md) §10.
**Purpose:** Establishes an operator-theoretic provenance for $G^*$ — namely, that $G^*$ is the $\zeta$-regularized determinant ratio of two quarter-twisted spectra arising from a primitive conjugacy operator $J$ with $J^2 = -I$. This is **Model I** of the Finite Quarter-Conjugacy Recurrence (FQCR) framework. The result is complementary to FTD-0127's parity-twist (number-theoretic / L-function lens); both land $G^*$ from different angles without one subsuming the other.

---

## §1 — The quarter-conjugacy operator

> **Definition 1.** Let $J$ be a finite-state conjugacy operator on a 2-real-dimensional space (or, equivalently, multiplication-by-$i$ on $\mathbb{C}$) satisfying
> $$ J^2 = -I. $$

**Immediate consequences.**

- $J^4 = I$, so $J$ generates a cyclic group of order 4: $\{I, J, -I, -J\}$.
- The eigenvalues of $J$ on $\mathbb{C}$ are $\pm i = e^{\pm 2\pi i/4}$, the two primitive fourth roots of unity.
- The action of $J$ partitions any $J$-equivariant function space into eigenspaces with $\tfrac{1}{4}$- and $\tfrac{3}{4}$-shifted Fourier modes.

This is the algebraic source of the **quarter split** that pervades the FTD algebraic spine: the $i$-cycle ontology in `FOUND_COGITO_AXIOM_AND_FULL_TRACE.md`, the framework integer $N_\text{base} = 4$ in `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` (FTD-0110 linear closure), and the $(1+i)$-tower of FTD-0111 (Theorem 8) all share this $Z_4$ structure as their common anchor.

---

## §2 — Quarter-twisted spectra

Take a duality clock coordinate $\phi \in S^1 \cong \mathbb{R}/2\pi\mathbb{Z}$ and a wavefunction $\psi : S^1 \to \mathbb{C}^2$ subject to the quarter-twisted boundary condition

$$ \psi(\phi + 2\pi) = J\,\psi(\phi). $$

Decomposing $\psi = \psi_+ \oplus \psi_-$ along the $J$-eigenbasis ($J\psi_\pm = \pm i\,\psi_\pm$):

$$ \psi_+(\phi + 2\pi) = e^{i\pi/2}\,\psi_+(\phi), \qquad \psi_-(\phi + 2\pi) = e^{-i\pi/2}\,\psi_-(\phi) = e^{3i\pi/2}\,\psi_-(\phi). $$

Solving for the allowed Fourier modes $\psi_\pm \propto e^{i\lambda\phi}$, the quarter-twist boundary forces

$$ \lambda \in \{n + \tfrac{1}{4}\}_{n\in\mathbb{Z}} \quad \text{(plus sector)}, \qquad \lambda \in \{n + \tfrac{3}{4}\}_{n\in\mathbb{Z}} \quad \text{(minus sector)}. $$

Restricting to the half-line $n \ge 0$ defines two spectral progressions

$$ D_{1/4} := \{n + \tfrac{1}{4}\}_{n\ge 0}, \qquad D_{3/4} := \{n + \tfrac{3}{4}\}_{n\ge 0}. $$

These are the spectra of two disjoint differential-like operators on a domain compatible with the quarter-twisted boundary condition. The full-line spectra are obtained by adjoining $\{-(n + \tfrac{1}{4})\}_{n\ge 1}$ and similarly for $D_{3/4}$; for the determinant ratio of interest (§3), the half-line restriction is the canonical choice and avoids the $\zeta$-regularization sign-ambiguity that pairs of negative eigenvalues would introduce.

---

## §3 — Lerch's formula recap

The zeta-regularized determinant of an arithmetic progression $\{n + a\}_{n\ge 0}$ is given by Lerch's formula. Define the Hurwitz zeta function

$$ \zeta_H(s, a) := \sum_{n=0}^{\infty} \frac{1}{(n+a)^s}, \qquad \Re(s) > 1, $$

with analytic continuation to $s = 0$. The $\zeta$-regularized determinant is

$$ \det_\zeta\{n + a\}_{n\ge 0} := \exp\!\big(-\zeta_H'(0, a)\big), $$

where $\zeta_H'$ denotes derivative with respect to $s$. Lerch's formula (1894) states

$$ \zeta_H'(0, a) = \log \Gamma(a) - \tfrac{1}{2}\log(2\pi). $$

Therefore

$$ \boxed{\; \det_\zeta\{n + a\}_{n\ge 0} = \frac{\sqrt{2\pi}}{\Gamma(a)}. \;} $$

This is a standard result of analytic number theory; see e.g. Voros, *Spectral functions, special functions and the Selberg zeta function*, Comm. Math. Phys. 110 (1987) 439–465, or Quine–Heydari–Song, *Zeta regularized products*, Trans. AMS 338 (1993) 213–231.

---

## §4 — The determinant bridge

> **Proposition 1.** *The $\zeta$-regularized determinant ratio of the quarter-twisted spectra $D_{3/4}$ and $D_{1/4}$ equals the bridge constant:*
> $$ \boxed{\; \frac{\det_\zeta D_{3/4}}{\det_\zeta D_{1/4}} = \frac{\sqrt{2\pi}/\Gamma(3/4)}{\sqrt{2\pi}/\Gamma(1/4)} = \frac{\Gamma(1/4)}{\Gamma(3/4)} = G^*. \;} $$

**Proof.** Apply Lerch's formula (§3) to each spectrum:

$$ \det_\zeta D_{1/4} = \frac{\sqrt{2\pi}}{\Gamma(1/4)}, \qquad \det_\zeta D_{3/4} = \frac{\sqrt{2\pi}}{\Gamma(3/4)}. $$

Take the ratio and the $\sqrt{2\pi}$ factors cancel exactly:

$$ \frac{\det_\zeta D_{3/4}}{\det_\zeta D_{1/4}} = \frac{\sqrt{2\pi}/\Gamma(3/4)}{\sqrt{2\pi}/\Gamma(1/4)} = \frac{\Gamma(1/4)}{\Gamma(3/4)}. $$

By Theorem 1 of `SPEC_ALGEBRAIC_SPINE.md` (FTD-0001 [THEOREM]), $G^* = \Gamma(1/4)/\Gamma(3/4)$. The identity $\Gamma(1/4)/\Gamma(3/4) = \sqrt{2}\,\Gamma(1/4)^2/(2\pi)$ uses the reflection formula $\Gamma(1/4)\Gamma(3/4) = \pi/\sin(\pi/4) = \pi\sqrt{2}$. $\square$

**Remarks.**

1. *Choice of orientation.* Whether the ratio is taken as $\det D_{3/4} / \det D_{1/4}$ or the reciprocal is a convention. The orientation chosen here yields $G^* > 1$ (specifically $G^* \approx 2.9587$), consistent with the existing FTD definition.

2. *Cancellation of $\sqrt{2\pi}$.* The cancellation is not coincidental — it reflects that the $\zeta$-regularized "vacuum energy" of the half-line (the $\sqrt{2\pi}$ factor) is shared between the plus and minus sectors. The determinant *ratio* is the genuinely invariant content, independent of the regularization scheme's overall normalization. This is a robustness feature: $G^*$ does not depend on conventions choices that affect each $\det_\zeta$ individually.

3. *Half-line restriction.* Adjoining the negative-shift modes $\{-(n + \tfrac{1}{4})\}_{n\ge 1}$ would require regularizing $\det_\zeta\{-(n+a)\}$, which by definition involves $\zeta_H(s, -a)$. The functional equation gives a sign and a phase factor; under the convention that the full-line spectrum is symmetric, the additional contributions cancel between plus and minus sectors. The half-line restriction is the canonical choice and matches the boundary-value problem setup in §2.

---

## §5 — Relation to FTD-0127 (parity-twist) — two readings of the residue-class decomposition mod 4

The two derivations of $G^*$ are not merely complementary; they are **two readings of the same residue-class decomposition of $\mathbb{Z}_{>0}$ mod 4**, which is the same decomposition that controls the prime-splitting law of $\mathbb{Z}[i]$. This identification is sharper than the "complementary lenses" framing originally put in place; it is documented here in full.

### §5.1 — The shifts $1/4$ and $3/4$ are residue classes, not free parameters

Multiplying by 4, the quarter-twisted spectra of §2 become

$$ 4\,D_{1/4} = \{1, 5, 9, 13, 17, 21, \ldots\} = \{n \in \mathbb{Z}_{>0} : n \equiv 1 \pmod 4\}, $$

$$ 4\,D_{3/4} = \{3, 7, 11, 15, 19, 23, \ldots\} = \{n \in \mathbb{Z}_{>0} : n \equiv 3 \pmod 4\}. $$

These are **exactly the two non-trivial residue classes mod 4**. The shifts $\{1/4, 3/4\}$ aren't free parameters of the construction — they are forced by $J^2 = -I$, which fixes the eigenvalue phases of $\pm i$, which fixes the quarter-twisted boundary condition, which forces the spectral shifts to be $\{n + 1/4\}$ and $\{n + 3/4\}$. Once $J$ is committed to, the residue-class decomposition is a theorem of the construction.

### §5.2 — Restricted to primes, these classes encode $\mathbb{Z}[i]$ splitting

By Fermat's two-square theorem (1640, proved by Euler 1747; equivalently, the prime-splitting law of $\mathbb{Z}[i]$ in modern Gaussian-integer language):

| Rational prime $p$ | Behaviour in $\mathbb{Z}[i]$ | Residue class |
|---|---|---|
| $p = 2$ | **Ramified** ($p = -i\,(1+i)^2$) | $-$ |
| $p \equiv 1 \pmod 4$ | **Split** ($p = \pi\,\bar\pi$ with $\pi, \bar\pi$ non-associate Gaussian primes; equivalently $p = a^2 + b^2$) | $4\,D_{1/4}$ |
| $p \equiv 3 \pmod 4$ | **Inert** ($p$ stays prime in $\mathbb{Z}[i]$) | $4\,D_{3/4}$ |

The prime members of $4\,D_{1/4}$ are exactly the split primes of $\mathbb{Z}[i]$. The prime members of $4\,D_{3/4}$ are exactly the inert primes. **The quarter-twisted spectra of FQCR Model I segregate, on the prime layer, the split and inert primes of the smallest imaginary quadratic ring.**

### §5.3 — Hurwitz components and the Dirichlet L-function

The standard relations between the Hurwitz components and the global L-functions are:

$$ L(s, \chi_{-4}) \;=\; 4^{-s}\bigl[\zeta_H(s,\,1/4) - \zeta_H(s,\,3/4)\bigr], $$

$$ \zeta(s)\,(1 - 2^{-s}) \;=\; 4^{-s}\bigl[\zeta_H(s,\,1/4) + \zeta_H(s,\,3/4)\bigr]. $$

(The first follows from the definition $L(s, \chi_{-4}) = \sum_n \chi_{-4}(n)\,n^{-s}$ split into $n \equiv 1\;(4)$ and $n \equiv 3\;(4)$ subsums; the second from the analogous split of the odd-integer subsum of $\zeta$.) The Euler product

$$ L(s, \chi_{-4}) \;=\; \prod_{p\,\equiv\,1\,(4)}(1 - p^{-s})^{-1} \cdot \prod_{p\,\equiv\,3\,(4)}(1 + p^{-s})^{-1} $$

makes the prime-class structure explicit: split primes contribute $(1 - p^{-s})^{-1}$ and inert primes contribute $(1 + p^{-s})^{-1}$ — same arithmetic structure with a sign flip on the inert side.

### §5.4 — Two readings, one decomposition

Both derivations of $G^*$ are working in the same residue-class decomposition of $\mathbb{Z}_{>0}$ mod 4. They differ only in the **combination** of $\zeta_H(s, 1/4)$ and $\zeta_H(s, 3/4)$ each takes as primary:

- **FTD-0127 parity-twist (`DERIV_G_STAR_PARITY_TWIST.md`):** primary objects are the **sum** ($\zeta(s)\cdot(1-2^{-s})$, even-parity, encodes "split + inert combined") and the **difference** ($L(s, \chi_{-4})$, odd-parity, encodes "split − inert"). $G^*$ emerges as the ratio of Archimedean Γ-factors at $s = 1/2$.

- **FQCR Model I (this doc):** primary objects are the **individual Hurwitz components** $\zeta_H(s, 1/4)$ and $\zeta_H(s, 3/4)$. $G^*$ emerges as the ratio of $\zeta$-regularized determinants — equivalently, the ratio of $\exp[-\zeta_H'(0, 3/4)]$ to $\exp[-\zeta_H'(0, 1/4)]$, which is $\sqrt{2\pi}/\Gamma(3/4)$ over $\sqrt{2\pi}/\Gamma(1/4)$, with the $\sqrt{2\pi}$ canceling.

The two derivations are **two views of the same arithmetic content**: FTD-0127 looks at parity-symmetric combinations of the Hurwitz components; FQCR Model I looks at the components themselves. They had to agree because they bottom out at the same Lerch evaluation $-\zeta_H'(0, a) = \log[\sqrt{2\pi}/\Gamma(a)]$.

The unification one-line:

> $$ G^* \;=\; \frac{\sqrt{2\pi}/\Gamma(3/4)}{\sqrt{2\pi}/\Gamma(1/4)} \;=\; \exp\!\bigl[\zeta_H'(0, 1/4) - \zeta_H'(0, 3/4)\bigr] \;=\; \frac{\Gamma_\zeta(1/2)}{\Gamma_{\chi_{-4}}(1/2)}. $$

The middle expression is FQCR Model I; the right is FTD-0127; the left is the explicit Lerch evaluation. Identical content, three readings.

### §5.5 — One subtlety: integers vs primes in the determinant

The $\zeta$-regularized determinants in §4 evaluate over **all positive integers in each residue class**, not just primes. The prime-class structure (split/inert/ramified) lives in the **Euler product** of the underlying $L$-function, not directly in the determinant identity. Specifically, the Lerch evaluation $-\zeta'_H(0, a) = \log\Gamma(a) - \tfrac{1}{2}\log(2\pi)$ uses the analytic continuation of $\zeta_H(s, a)$ across the entire $s$-plane and evaluates a single derivative at $s = 0$; nothing in this calculation distinguishes prime from composite members of the residue class.

The connection to prime splitting therefore runs through the *Euler factorization of the L-function whose Hurwitz components are these determinants*, not through a direct prime-restricted determinant identity. This is what makes the two derivations "two readings": they share the same Hurwitz-component primary data, but the route from those components to $G^*$ doesn't pass through prime-restriction.

Stated honestly: **FQCR Model I is the operator-theoretic reading of the Hurwitz/$L$-function decomposition over the residue classes mod 4; FTD-0127 is the parity-symmetric reading of the same decomposition; the prime-splitting law of $\mathbb{Z}[i]$ is the arithmetic-geometric content underlying both readings, exposed via the Euler product of $L(s, \chi_{-4})$.**

### §5.6 — Curve-side bridge (compatibility paper of QCR trilogy, 2026-05-07)

The QCR trilogy's compatibility paper (see [`REF_QCR_TRILOGY_BRIDGE.md`](../09_mathematical/REF_QCR_TRILOGY_BRIDGE.md), FTD-0144) supplies a **curve-side geometric pairing** for the algebraic recurrence developed in the QC-Recurrences paper. The pairing is a sector-coloring map $c : \mathcal{S}_N \to \{z_+(t), z_-(t)\}$ from radial-minima sectors of a dyadic Fourier curve to the two reciprocal projective branches of the symmetric form $u_{m+1} + u_{m-1} = s_N(t)\,u_m$. The trilogy cross-confirms FQCR Model II's finite-N formula at the equation level (compatibility paper Definition 2.1 = FTD-0142) and supplies a concrete depth-4 numerical incarnation of FTD's $(1+i)$-tower — exactly the dyadic frequency ladder $\{2^k : k = 0, \ldots, 4\}$ that Theorem 8 (FTD-0111) indexes. The depth-4 example exhibits a $6 = 2 \times 3$ sector structure (two reciprocal branch labels across three radial doublets) that is observed numerically and flagged interpretation-free; FTD documents the correspondence without promoting any reading. See `REF_QCR_TRILOGY_BRIDGE.md` §4 for the depth-4 mapping and §5.5 for the trilogy's own conjectures.

### §5.7 — Why this matters for the SMC chain $G^* \to $ master quadratic $\to \alpha$

Saying "$G^* = \Gamma(1/4)/\Gamma(3/4)$" makes the constant look like a near-arbitrary special-function combination. Saying "$G^*$ is the regularized asymmetry between the split and inert prime classes of $\mathbb{Z}[i]$" — equivalently, the regularized log-ratio of Hurwitz components for the two non-trivial residue classes mod 4 — places $G^*$ as **the natural arithmetic invariant of the smallest imaginary quadratic ring**. Through the master quadratic (Theorem 2 of the algebraic spine), the FQCR Model V identification $\alpha^{-1} = x_+$ then identifies the fine-structure constant with an algebraic combination of this arithmetic invariant.

This does not promote the SMC tag — the load-bearing physical claim still inherits FTD-0013's [STRONGLY MOTIVATED CONJECTURE] status. But it does **substantially raise the prior** on the SMC reading by tying $G^*$ to the structurally simplest CM ring's deepest arithmetic fact (Fermat's two-square theorem). This is consistent with the existing 9-Heegner-discriminant rigidity scan (FTD-0123/0124) which already showed $\mathbb{Z}[i]$ structurally privileged at the curve level — the present observation gives a deeper *why* via the residue-class character structure.

---

## §6 — Implications for the algebraic spine

The algebraic spine in `SPEC_ALGEBRAIC_SPINE.md` — nine numbered results, six theorem-grade + three honestly-tiered below theorem grade (see §0) — is preserved as a fixed canonical reference; this derivation lands as a **subsidiary in §10 (Subsidiaries)** of that document, not as a "Theorem 10". Specifically:

1. Theorem 1 (FTD-0001) — $G^* = \Gamma(1/4)/\Gamma(3/4)$ — is now equipped with **two independent provenance chains**:
   - FTD-0127 parity-twist (number-theoretic).
   - FTD-0141 quarter-conjugacy determinant bridge (operator-theoretic, this doc).

2. Theorem 8 (FTD-0111, $(1+i)$-tower harmonic invariant) shares the $Z_4$ structural anchor with the $J$-operator in this derivation. A cleaner unification would place both under a common "$Z_4$ algebraic-spine" subsection of §10; that's a stylistic refactor for a future revision.

3. Theorem 9 (FTD-0112, field-theoretic $Q(G^*)$) is unaffected — the operator-theoretic derivation does not introduce new transcendentals or change the field-theoretic content of $G^*$.

The status tag for this result is **[THEOREM]** — the proof is straightforward modulo the standard zeta-regularization machinery; no new selection or conjecture is required.

---

## §7 — What this derivation does NOT establish

To prevent overclaim:

- **Does not derive $\alpha = 1/x_+$.** The quarter-conjugacy result establishes $G^*$ via an operator route; it does not bridge $G^*$ to the fine-structure constant. The latter remains tied to the master quadratic root $x_+$ and the FTD-0013 [STRONGLY MOTIVATED CONJECTURE].
- **Does not derive the master quadratic.** The polynomial $x^2 - 16G^{*2}x + 16G^{*3} = 0$ retains its existing provenance (FTD-0014, coefficient 16 from $|\text{Aut}(E)|^2$, etc.). The transfer-matrix interpretation (FQCR Model V) is a notational reframing of the same polynomial, not a new derivation.
- **Does not pin the half-line restriction physically.** The choice $n \ge 0$ vs full-line is canonical for the operator-determinant calculation but is not derived from FTD axioms. A future strengthening might tie this to a physical interpretation of "positive-frequency modes".
- **Does not prove uniqueness of $J$.** Multiple operators satisfy $J^2 = -I$ (any $90°$-rotation in any 2D real subspace works); the derivation extracts $G^*$ from any such $J$ but doesn't single out a canonical one. The framework integer $N_\text{base} = 4$ is a separate (also derived) anchor.

---

## §8 — Cross-references

| Cross-reference | Purpose |
|---|---|
| `SPEC_ALGEBRAIC_SPINE.md` Theorem 1 | Foundational $G^* = \Gamma(1/4)/\Gamma(3/4)$ identity that this derivation lands at via the operator route. |
| `DERIV_G_STAR_PARITY_TWIST.md` (FTD-0127) | Complementary L-function lens on the same constant. |
| `DERIV_LFUNCTION_GSTAR_CONNECTION.md` | $L$-function side of the parity-twist; the quarter-conjugacy lens does not duplicate this. |
| `DERIV_GSTAR_FINITE_APPROX.md` (FTD-0142) | Finite-N reframe of the same quarter-conjugacy chain; discharges the `AUDIT_INFINITY_REFRAME.md` ε-L obligation. |
| `SPEC_FQCR.md` | Capstone reference for the full FQCR framework; this derivation is its Model I. |
| `FOUND_COGITO_AXIOM_AND_FULL_TRACE.md` | $i$-cycle algebraic axiom; same $Z_4$ structure as $J$. |
| `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` (FTD-0110 linear) | $N_\text{base} = 4$ framework integer; another instance of the $Z_4$ structural anchor. |
