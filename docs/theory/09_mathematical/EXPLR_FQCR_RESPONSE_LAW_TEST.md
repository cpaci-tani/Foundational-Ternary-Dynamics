# Exploration · FQCR Response-Law Comparison

**Date:** 2026-05-08
**Status:** [EXPLORATORY] — structural-stability test of the additive response law $R_N(t) = 1 + \lambda_N(4it) + A_N(t)$ currently tagged [SELECTION] in [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) §3.3.
**Tag impact:** none. The [SELECTION] tag stands. This test was *not* pre-registered; criteria were declared in the same session as execution.
**Companion:** [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md), [`AUDIT_DUAL_SUBSTRATE_PROVENANCE.md`](../07_assessment/AUDIT_DUAL_SUBSTRATE_PROVENANCE.md), [`scripts/exploration/explore_fqcr_response_laws.py`](../../../scripts/exploration/explore_fqcr_response_laws.py).

---

## §1 — Setup

The FQCR Model V transfer matrix gives the modulated master quadratic

$$
x^2 - 16(G_N^*)^2 x + 16(G_N^*)^3 R_N(t) = 0,
$$

with the dominant root

$$
x_+(N, t) = 8(G_N^*)^2 + 4(G_N^*)^{3/2}\sqrt{4 G_N^* - R_N(t)}.
$$

The factor $R_N(t) = 1 + \lambda_N(4it) + A_N(t)$ is currently [SELECTION]. Two natural alternative laws preserve the same first-order Taylor expansion:

$$
R_\mathrm{add}(t) = 1 + \lambda + A, \qquad
R_\mathrm{mult}(t) = (1+\lambda)(1+A), \qquad
R_\mathrm{exp}(t) = e^{\lambda + A}.
$$

All three agree as $\lambda + A \to 0$ to second order: $R_\mathrm{mult} - R_\mathrm{add} = \lambda A$, and $R_\mathrm{exp} - R_\mathrm{add} = (\lambda+A)^2/2 + O(\lambda^3, A^3)$.

The test: rank the three laws by structural-stability criteria across $t \in [0.3, 3.0]$, $N \in \{32, 128, 512, 1024\}$, with criteria declared in advance (this section, before script execution): real-domain validity, smoothness, monotonicity, finite-N convergence, and law-distinguishability at $t=1$.

---

## §2 — At $t=1$ (the [SELECTION] base point), the test is degenerate

Numerical results from [`explore_fqcr_response_laws.py`](../../../scripts/exploration/explore_fqcr_response_laws.py) at $N = 512$:

$$
\lambda_N(4i) \approx 5.580 \times 10^{-5}, \qquad A_N(1) \approx -8.12 \times 10^{-8}.
$$

Both are tiny. The pairwise differences between the three $x_+$ values at $t=1$ are:

| Pair | Difference at $t = 1$ | Difference at $t = 0.3$ |
|---|---:|---:|
| $x_\mathrm{add} - x_\mathrm{mult}$ | $-1.4 \times 10^{-11}$ | $-1.7 \times 10^{-2}$ |
| $x_\mathrm{add} - x_\mathrm{exp}$ | $+4.8 \times 10^{-9}$ | $+1.5 \times 10^{-1}$ |
| $x_\mathrm{mult} - x_\mathrm{exp}$ | $+4.8 \times 10^{-9}$ | $+1.6 \times 10^{-1}$ |

**At $t=1$ the three laws agree to ~10 digits.** This is structurally forced: $\lambda A \approx 4.5 \times 10^{-12}$ and $(\lambda + A)^2/2 \approx 1.6 \times 10^{-9}$ at the base point, so the second-order corrections that distinguish the laws are already below the relevant precision.

**Implication for the §8 test:** the base point is *not* the place to discriminate. Any law-comparison criterion using $t=1$ alone is insensitive. The test must use $t < 1$ where $\lambda + A$ is $O(0.1)$ and the second-order corrections matter.

This is itself a structural finding: $t = 1$ is degenerate for response-law selection, which means the "9-digit match at $t=1$" cannot, by itself, pick out the additive law. **The match at $t=1$ is consistent with all three laws.**

---

## §3 — Real-domain validity

The branch $\sqrt{4 G_N^* - R_N(t)}$ goes complex when $R > 4 G_N^* \approx 11.83$. Across $t \in [0.3, 3.0]$, all three laws stay real-valid. Pushing toward $t = 0$, the smallest $t$ at which each law first crosses the real-domain boundary:

| Law | First-failure $t$ | $R$ at failure |
|---|---:|---:|
| $R_\mathrm{add}$ | $\approx 0.06$ | $12.66$ |
| $R_\mathrm{mult}$ | $\approx 0.07$ | $15.77$ |
| $R_\mathrm{exp}$ | $\approx 0.11$ | $12.27$ |

(Stable across $N \in \{32, 128, 512\}$ to the displayed precision.)

The exponential law fails first as $t$ decreases. The multiplicative law fails next, but with a much steeper $R$ overshoot ($R = 15.77$ vs. $4 G_N^* = 11.83$ — a 33% violation as soon as it crosses). The additive law is most conservative, both failing latest and with the smallest overshoot magnitude.

**Verdict on criterion (a) real-domain:** mild advantage to additive (largest real-valid range, smoothest crossing).

---

## §4 — Monotonicity and smoothness

Across $t \in [0.3, 3.0]$ at $N = 512$:

| Law | Monotonic? | Max $|x_+(t_{i+1}) - 2 x_+(t_i) + x_+(t_{i-1})|$ |
|---|:---:|---:|
| Additive | yes | 0.405 |
| Multiplicative | yes | 0.390 |
| Exponential | yes | 0.524 |

All three are monotonic in $t$ (no sign changes in $dx_+/dt$). Multiplicative is marginally smoothest; exponential has visibly larger second differences near small $t$ where $\lambda + A$ is largest.

**Verdict on criteria (b) smoothness, (c) monotonicity:** all three pass; multiplicative wins on smoothness by a small margin; exponential is third.

---

## §5 — Finite-N convergence

At $t = 1$, $x_+(N)$ for the additive law:

| $N$ | $G_N^*$ | $x_+(N, 1)$ | Gap to CODATA $\alpha^{-1} = 137.0359991770$ |
|---:|---:|---:|---:|
| 16 | 2.95883500 | 137.0509770 | $+109.30$ ppm |
| 64 | 2.95868606 | 137.0370242 | $+7.48$ ppm |
| 256 | 2.95867582 | 137.0360647 | $+0.479$ ppm |
| 1024 | 2.95867516 | 137.0360033 | $+0.031$ ppm |
| 4096 | 2.95867512 | 137.0359994 | $+0.003$ ppm |

The convergence is well-behaved; the gap closes as $N^{-2}$ from the $G_N^*$ convergence (FTD-0142). At $N \to \infty$ with the actually-converged $G^*$, the residual gap is $\approx 0.001$ ppm = 1 ppt — **not** the "<0.001 ppt" claim attached to the 7-term precision series, which is a separate framework. The branch-equation reading at $t = 1$ matches CODATA to about 1 ppt at infinite $N$.

The same convergence shape holds for multiplicative and exponential to ~$10^{-9}$.

**Verdict on criterion (d) finite-N convergence:** all three pass equally.

---

## §6 — Where the laws actually distinguish themselves

At $t = 0.3$, $N = 512$:

| Law | $R(0.3)$ | $x_+(512, 0.3)$ |
|---|---:|---:|
| Additive | $1.2910$ | $136.130$ |
| Multiplicative | $1.2856$ | $136.147$ |
| Exponential | $1.3402$ | $135.984$ |

Spread: $0.16$ in $x_+$. This is the regime where response-law selection actually has measurable consequences.

Whether *physics* discriminates at $t = 0.3$ requires a $t \leftrightarrow$ scale map. SPEC_FQCR §6 Test 3 ("running behaviour") is currently [OPEN — out of scope until $t$ has a-priori interpretation], because without a physical reading of $t$ as inverse-scale, the predictions at $t \neq 1$ are mathematical, not falsifiable.

**The honest finding:** the response-law selection cannot be made by structural-stability alone. All three pass. Distinguishing them requires a physical $t$-axis interpretation that the framework currently lacks.

---

## §7 — Composite verdict on the §8 test

| Criterion | Best law | Margin |
|---|---|---|
| Real-domain validity (range to small $t$) | Additive | small |
| Smoothness (max 2nd diff) | Multiplicative | very small |
| Monotonicity in $t$ | tied | none |
| Finite-$N$ convergence | tied | none |
| Distinguishability at $t = 1$ | tied (all $\sim 10^{-10}$) | n/a |
| Distinguishability at $t = 0.3$ | (need physics) | n/a |

**Net:** No law dominates. The additive law has the small structural-stability edge in real-domain extension; the multiplicative law has the small edge in smoothness; nothing decisive.

The [SELECTION] tag on $R_N(t) = 1 + \lambda_N(4it) + A_N(t)$ stays. This test does not promote it to [DERIVED]. It does, however, modestly support the additive choice: it is real-domain-conservative and not the worst on any criterion. That's "consistent with selection," not "uniquely forced."

---

## §8 — Why this matters for the broader epistemic stack

This is the same pattern as the [`AUDIT_DUAL_SUBSTRATE_PROVENANCE.md`](../07_assessment/AUDIT_DUAL_SUBSTRATE_PROVENANCE.md) finding from earlier today: a structural choice in FQCR Model V is *consistent with* the data without being *forced* by the data, and the canonical [SELECTION] tag is honest. The risk is rhetorical inflation — saying "the additive law is structurally preferred" — when the test really shows "the additive law is one of several that all pass."

The actually load-bearing test for FQCR Model V is **[FTD-0143 quotient uniqueness](../10_eft_program/PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md)** (the $7^4 = 2401$-quadruple scan over alternatives to $(4,6;3,2)$). That scan probes a higher-dimensional [SELECTION] knob ($\Psi_N$ exponent quadruple) than the response-law test does, and is pre-registered. Until it runs, the structural-uniqueness claim about the FQCR Model V machinery rests on numerical coincidences at $t = 1$, which this test has now shown to be law-degenerate.

---

## §9 — Engaging with the four open questions

The 2026-05-08 operator-stack discussion raised four open questions. Brief audit of each against existing FTD machinery:

### Q1 — Why $R_N(t) = 1 + \lambda_N + A_N$?

**Status from this test:** [SELECTION] confirmed; structural-stability test does not discriminate against natural alternatives; modest advantage on real-domain criterion. Open.

### Q2 — Why $16 = 4^2$?

The user's reading: "16 is the quadratic scale of the order-four clock" ($J^4 = I$, branch is degree 2, hence $4^2 = 16$).

This is consistent with the canonical provenance per [`DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`](../03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md) §2.2:
- Route A: $|\mathrm{Aut}(E)|^2 = 4^2 = 16$ — matches the user's clock-order reading exactly, since $\mathrm{Aut}(E) = \mathbb{Z}/4 = \langle J \rangle$.
- Route B: $z_\mathrm{BCC} \times 2 = 8 \times 2 = 16$ — coordination-times-non-void.

The user's "test other $m$" suggestion is already done structurally: [`EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md`](EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md) (META_INDEX 9.31, 2026-05-01) scanned 58 $(m, k)$ pairs of the natural Gaussian-integer-tower family, and $(m, k) = (2, 4) \Rightarrow m^k = 16$ is **rank 1 with a 5-orders-of-magnitude gap to rank 2**. So the empirical scan supports $4^2 = 16$ as structurally privileged in the natural family.

**Status:** [STRUCTURAL OBSERVATION] from FTD-0131 / EXPLR_TOWER_MULTIPLIER_UNIQUENESS, with an explicit clock-order-vs-quadratic-scale interpretation that is consistent with the user's reading. The full derivation chain through CM theory + ternary states is fairly tight; the only piece still [SELECTION] is which combinatorial route (A vs B) is "primary."

### Q3 — Why $(4, 6; 3, 2)$ in $\Psi_N(t)$?

**Status from existing machinery:** The $7^4 = 2401$-alternative scan is [PRE-REGISTERED] as FTD-0143 / [`PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md`](../10_eft_program/PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md) and gated on a separate session. The user's red-team alternatives $(4,6)/(3,1)$, $(4,6)/(4,2)$, $(4,4)/(3,2)$, $(2,3)/(3,2)$ are subsets of the 2401-element scan space.

**Recommendation:** the load-bearing question is the FTD-0143 scan, not a hand-picked alternative comparison. If FTD-0143 confirms $(4,6;3,2)$ uniqueness at the strict tolerance, the [SELECTION] tag upgrades to "[SELECTION with uniqueness backing]" per the pre-registered protocol.

The user's interpretive reading $(4, 6) = $ quarter clock + six bivector modes ($\dim \Lambda^2(\mathbb{R}^4) = 6$); $(3, 2) = $ spatial projection + two transverse modes — is consistent with FTD's spacetime ontology and is already implicit in [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) §3.1. The interpretation does not promote the tag, but it makes the [SELECTION] less arbitrary.

### Q4 — Why $t = 1$ physical?

The user notes $t = 1$ is the fixed point of $t \mapsto 1/t$ — the modular self-dual tick. Mathematically clean, physically a [SELECTION].

**Status:** [SELECTION] per [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) §3.2; the physical interpretation of $t$ is open and gates the FTD-0143 follow-up Test 3 (running behaviour).

The §2 finding above adds support to $t = 1$ as the natural base point for a different reason: it is the point at which the response-law $R(t)$'s second-order corrections vanish to relevant precision — i.e., $t = 1$ is the point where the FQCR Model V branch readout is *insensitive to response-law choice*. That could be re-read as "$t = 1$ is the unique base point where all natural response laws agree to physical precision." Whether that re-reading lifts the [SELECTION] tag depends on whether one accepts insensitivity-of-readout as a structural principle. I would argue it doesn't — insensitivity makes $t = 1$ a comfortable place to evaluate, but doesn't force the physical identification.

---

## §10 — Status table

| Item | Statement | Tag |
|---|---|---|
| RLT-1 | At $t = 1$ the three laws agree to $\sim 10^{-10}$; the base point is law-degenerate | [THEOREM] (numerically verified, structural reason) |
| RLT-2 | Across $t \in [0.3, 3.0]$, all three laws are real-valid and monotonic | [THEOREM] (numerically verified) |
| RLT-3 | The additive law has the largest real-valid range as $t \to 0$ | [STRUCTURAL OBSERVATION] |
| RLT-4 | The multiplicative law has the smoothest second derivative | [STRUCTURAL OBSERVATION] |
| RLT-5 | The §8 stability test does not discriminate between laws at $t = 1$ | [THEOREM] |
| RLT-6 | The additive law's [SELECTION] tag (SPEC_FQCR §3.3) stands | [SELECTION] (unchanged) |
| RLT-7 | The branch-equation reading at $t=1$ matches CODATA to ~1 ppt at $N \to \infty$ (not 0.001 ppt) | [NUMERICAL FACT] |
| RLT-8 | Real distinguishability requires a physical $t \leftrightarrow$ scale map (Test 3, [OPEN]) | [OPEN] |

---

## §11 — Cross-references

- [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) §3.3 — the [SELECTION] tag this test is auditing.
- [`AUDIT_DUAL_SUBSTRATE_PROVENANCE.md`](../07_assessment/AUDIT_DUAL_SUBSTRATE_PROVENANCE.md) — the same epistemic pattern (structural choice consistent with data without being forced by it).
- [`PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md`](../10_eft_program/PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md) (FTD-0143) — the load-bearing structural-uniqueness test.
- [`EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md`](EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md) — the (m, k) = (2, 4) → 16 uniqueness scan that addresses Q2.
- [`scripts/exploration/explore_fqcr_response_laws.py`](../../../scripts/exploration/explore_fqcr_response_laws.py) — the script that produced these numbers.
