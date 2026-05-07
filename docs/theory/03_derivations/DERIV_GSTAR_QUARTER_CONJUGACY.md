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

## §5 — Relation to FTD-0127 (parity-twist)

`DERIV_G_STAR_PARITY_TWIST.md` (FTD-0127, 2026-05-03) establishes that $G^*$ is the ratio of Archimedean Γ-factors at the critical-line center of the even-parity Riemann $\zeta$ vs the odd-parity Dirichlet $L(s, \chi_{-4})$. Operationally:

$$ G^* = \frac{\Gamma_\zeta(s)\big|_{s=1/2}}{\Gamma_{\chi_{-4}}(s)\big|_{s=1/2}} \quad \text{(parity-twist reading).} $$

This is the **L-function / number-theoretic lens** on $G^*$.

The present derivation establishes

$$ G^* = \frac{\det_\zeta D_{3/4}}{\det_\zeta D_{1/4}} \quad \text{(quarter-conjugacy / operator-theoretic lens).} $$

**These are complementary, not competing, derivations.** Both lenses bottom out at the same $\Gamma(1/4)/\Gamma(3/4)$ identity; they differ in *which structure* is taken as primary:

- FTD-0127: primary structure is the parity decomposition of $\zeta(s) = \sum 1/n^s$ vs $L(s, \chi_{-4}) = \sum \chi_{-4}(n)/n^s$, with the χ_{-4} character encoding $i$-tower phases.
- This derivation: primary structure is the quarter-conjugacy operator $J$ with $J^2 = -I$, with the spectral-shift $\{1/4, 3/4\}$ inheriting the eigenvalue phases of $\pm i$.

A fully unified treatment would link the χ_{-4} character to the $J$-action via $\chi_{-4}(n) = i^{n-1}$ (or equivalent), making explicit that the parity-twist and the quarter-conjugacy are two readings of the same $Z_4$ structure. That unification is left as a follow-up; the present derivation establishes the operator-theoretic provenance independently.

---

## §6 — Implications for the algebraic spine

The 9-theorem spine in `SPEC_ALGEBRAIC_SPINE.md` is preserved as a fixed canonical reference; this derivation lands as a **subsidiary in §10 (Subsidiaries)** of that document, not as a "Theorem 10". Specifically:

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
