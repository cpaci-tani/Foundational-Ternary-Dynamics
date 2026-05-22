# Provenance Audit: The Dual-Substrate Decomposition

**Date:** 2026-05-08
**Status:** [AUDIT FINDING] — F1-hygiene observation, no demotion of any spine claim.
**Tag impact:** none on spine. Trims rhetoric in [`EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md`](../09_mathematical/EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md); flags stale comment in [`engine/include/ftd/ontic/master_quadratic.h`](../../../engine/include/ftd/ontic/master_quadratic.h).
**Verifier numerics:** verified to ≥10 digits; computation reproduced in §3.

---

## §1 — Question

[`EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md`](../09_mathematical/EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md) §1 cites "five independent lines of evidence" for the identification $\psi = J_L + J_R = G^*$ per degree of freedom. The strongest of these are:

1. **Dual substrate**: $\psi = J_L + J_R = G^*$ exactly (per DoF).
2. **Vieta triad**: $S = x_+ + x_- = 16G^{*2}$, $P = x_+ \cdot x_- = 16G^{*3}$, $P/S = G^*$.

Claim: these are independent. Are they?

The motivation to ask: the J chain (FTD-0141) derives $G^*$ itself operator-theoretically — $G^* = \det_\zeta D_{3/4} / \det_\zeta D_{1/4}$ via Lerch's formula on the quarter-twisted spectra of $J^2 = -I$ ([`DERIV_GSTAR_QUARTER_CONJUGACY.md`](../03_derivations/DERIV_GSTAR_QUARTER_CONJUGACY.md), [THEOREM]). If the dual substrate $(J_L, J_R, \delta)$ is also independently J-derived, then the convergence with the master quadratic at $(S, P) = (16G^{*2}, 16G^{*3})$ is real coherence evidence. If not, lines 1 and 2 above are the same algebraic identity in two coordinate systems and the rhetoric overstates the case.

---

## §2 — What the J chain derives, with citations

| Object | Derivation chain | Status | Source |
|---|---|---|---|
| $G^* = \Gamma(1/4)/\Gamma(3/4)$ | $J^2 = -I \to$ quarter-twisted spectra $D_{1/4}, D_{3/4} \to$ Lerch's formula $\det_\zeta\{n+a\} = \sqrt{2\pi}/\Gamma(a) \to$ ratio | [THEOREM] | FTD-0141 / [DERIV_GSTAR_QUARTER_CONJUGACY.md §4](../03_derivations/DERIV_GSTAR_QUARTER_CONJUGACY.md) |
| $J^4 = I$, $Z_4$ structure | Algebraic from $J^2 = -I$ | [THEOREM] | FTD-0141 §1 / SPEC_FQCR §1 |
| Recurrence $z^2 - sz + 1 = 0$ | Möbius reduction of $u_{m+1} + u_{m-1} = s\,u_m$ | [THEOREM] | SPEC_FQCR Proposition 4 |

**Not derived from the J chain alone:** the specific values $(S, P) = (16G^{*2}, 16G^{*3})$. The J chain gives $G^*$; getting $(S, P)$ from $G^*$ requires *additional structural input* — the master-quadratic provenance (Routes A and B for the coefficient 16; see §3).

---

## §3 — The dual substrate's actual provenance

[`engine/include/ftd/ontic/master_quadratic.h`](../../../engine/include/ftd/ontic/master_quadratic.h) Layer 3b states:

```
S = E_L + E_R = 16·G*²     [THEOREM — 16 DoF × G*² per DoF]
P = E_L · E_R = 16·G*³     [PROPOSITION — spatiotemporal interaction]
δ² = (4G* - 1)/(4G*) = 1 - 1/(4G*) ≈ 0.91554
```

**The δ² value is forced once $S$ and $P$ are pinned.** From the algebraic identity $S^2 = D^2 + 4P$ where $D = E_L - E_R$:

$$
\delta^2 \;:=\; \frac{D^2}{S^2} \;=\; 1 - \frac{4P}{S^2} \;=\; 1 - \frac{4 \cdot 16G^{*3}}{(16G^{*2})^2} \;=\; 1 - \frac{1}{4G^*}.
$$

Verified: $\delta^2 = 0.9155027200\ldots$ via three routes (direct; $1 - 1/(4G^*)$; $(4G^*-1)/(4G^*)$); all agree to machine precision.

So $\delta^2$ is **algebraic dressing** on $(S, P)$, not an independent constraint. The non-trivial content is in the values of $S$ and $P$.

---

## §4 — Where do $S = 16G^{*2}$ and $P = 16G^{*3}$ come from?

### §4.1 — The 16: two independent provenance chains [THEOREM]

Per [`DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`](../03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md) §2.2:

- **Route A — CM-curve auto count.** The elliptic curve $E: y^2 = x^3 - x$ has $j = 1728$, CM by $\mathbb{Z}[i]$, and $\mathrm{Aut}(E) \cong \mathbb{Z}/4$. Hence $|\mathrm{Aut}(E)|^2 = 16$. (See [`DERIV_DUAL_DERIVATION_OF_16.md`](../08_structural/DERIV_DUAL_DERIVATION_OF_16.md).)
- **Route B — BCC coordination × ternary states.** $z_{\mathrm{BCC}} = 8$ on the Moore neighbourhood; non-void states $\{-1, +1\}$, count 2; product $8 \times 2 = 16$.

Both are finite-combinatorial. Neither invokes a limit. Their numerical agreement is multi-route evidence for 16 as the natural leading coefficient.

### §4.2 — Stale comment alert: Route C (temporal gauge) has been retracted

[`master_quadratic.h`](../../../engine/include/ftd/ontic/master_quadratic.h) lines 36–40 currently assert:

> DOF counting in TEMPORAL GAUGE (the ontological gauge of FTD): On the 2×2×2 torus: 24 total - 7 Gauss constraints - 1 pure gauge = 16. (Coulomb gauge gives 14, but temporal gauge is ontologically forced.)

But [`DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`](../03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md) §2.2 line 84 explicitly retracts this:

> *(A third historical route, the temporal-gauge DOF count $24 - 7 - 1 = 16$ on the $2^3$ torus, has been retracted as incorrect: proper Coulomb-gauge fixing on $T^3$ yields $14$, not $16$. See `AUDIT_MASTER_QUADRATIC.md`.)*

**The header comment in `master_quadratic.h` lines 36–40 is stale relative to the canonical theory layer.** This should be brought into sync. The canonical reading is now: 16 has Routes A and B (CM auto count, BCC × ternary), not a DoF count.

### §4.3 — The G* powers in the master quadratic

The polynomial degree (2) is forced by the CM field $\mathbb{Q}(i)$ having $[\mathbb{Q}(i):\mathbb{Q}] = 2$ and Schneider–Chudnovsky algebraic-relation bounds (`DERIV_QUADRATIC_NECESSITY.md`). The powers $G^{*2}$ and $G^{*3}$ in the linear and constant coefficients are forced by the period structure of $E$: $G^{*2}/(2\pi) = W_3$ (Watson's BCC integral), and $G^{*3}$ is the corresponding cubic period invariant. [THEOREM] for the algebra; see [`DERIV_WATSON_GSTAR_IDENTITY.md`](../04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md).

---

## §5 — The verdict

### §5.1 — Two genuinely independent provenance chains

For the **whole stack** $\{G^*, 16G^{*2}, 16G^{*3}, \text{master quadratic}, (J_L, J_R, \delta)\}$, the provably independent inputs are:

1. **$G^*$ itself**, via the J chain (FTD-0141, [THEOREM]).
2. **The integer 16**, via two independent finite-combinatorial routes (CM auto count, BCC × ternary).

Once you have these two, everything else is algebraic forcing:
- The master quadratic $x^2 - 16G^{*2}x + 16G^{*3} = 0$ is forced by $(G^*, 16, $ degree 2 from CM field, period structure$)$.
- The Vieta relations $S = 16G^{*2}$, $P = 16G^{*3}$ are forced.
- The dual-substrate identity $J_L + J_R = G^*$, $J_L \cdot J_R = G^*/16$ is the same Vieta relations rescaled by $16G^*$.
- The asymmetry $\delta^2 = (4G^*-1)/(4G^*)$ is algebraic dressing on $(S, P)$.

### §5.2 — Trim recommendation for [`EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md`](../09_mathematical/EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md)

The "five independent lines of evidence" framing in §1 needs trimming. Lines 1 (dual substrate $\psi = G^*$) and 2 (Vieta sum/product) are **the same algebraic identity in two coordinate systems**. The Claims Table (§Claims) is internally honest about this — GFT-1 cites the dual substrate, GFT-2 cites Vieta sum, GFT-3 cites Vieta product, GFT-4 cites $P/S$ — but the framing language "five independent lines" reads as five separate confirmations when it is really one identity rewritten.

**Suggested replacement language:** "five readings of the same algebraic structure." Same content, no overclaim.

### §5.3 — Drift recommendation for [`master_quadratic.h`](../../../engine/include/ftd/ontic/master_quadratic.h)

Header lines 36–40: replace the temporal-gauge DOF count with the CM auto count + BCC coordination justification. Same conclusion ($|\mathrm{Aut}(E)|^2 = 16 = z_\mathrm{BCC} \times 2$), no retracted reading. The Layer 3b comment "[THEOREM — 16 DoF × G*² per DoF]" should also be re-tagged: the value $S = 16G^{*2}$ is [THEOREM] as Vieta sum of the master quadratic, not as a per-DoF count (since the DoF count was Route C, retracted).

---

## §6 — What this audit does NOT do

- **Does NOT demote the dual substrate as an interpretive lens.** The chirality reading $J_L \leftrightarrow J_R$ as CPT, the asymmetric flux split, and the projection onto the $\pm i$ eigenspaces of $J$ are interpretively valuable — they connect to the Z₄ algebraic spine and to the (1+i)-tower of Theorem 8 (FTD-0111). The lens is preserved.
- **Does NOT change any spine [THEOREM] or [SMC] tag.** $G^*$ stays at FTD-0001; the master quadratic stays at FTD-0014; $x_+ = 1/\alpha$ stays at FTD-0013 [STRONGLY MOTIVATED CONJECTURE]. Nothing in the algebra is wrong — only some doc-rhetoric overstates "independence."
- **Does NOT close the question of whether the J chain *can* be extended to derive $S, P$ independently.** That is genuinely open (§7).

---

## §7 — Open question: can the J chain reach $S = 16G^{*2}$ and $P = 16G^{*3}$ independently?

Currently the J chain derives $G^*$ via a *first-order* determinant ratio (Lerch on $\zeta_H'(0, a)$). The natural question: do *higher-order* determinant invariants of the same J operator give $S = 16G^{*2}$ and $P = 16G^{*3}$ from an independent starting point?

Candidates worth probing:
- Quadratic determinant invariants $\det_\zeta D_{1/4} \cdot \det_\zeta D_{3/4}$ or sums; these would land at $\Gamma(1/4)\Gamma(3/4) = \pi\sqrt{2}$ (the *product* channel) at first order. Higher-order invariants — second derivatives at $s = 0$, regulator-style — might yield $G^{*2}$ or $G^{*3}$ but require the appropriate Beilinson / regulator setup ([`MATH_LOG_GSTAR_IDENTITY.md`](../09_mathematical/MATH_LOG_GSTAR_IDENTITY.md) §3.5 frames $G^*$ as a generating function for this sector at level 4).
- The (1+i)-tower at depth $k = 4$ (Theorem 8 / FTD-0111) gives a harmonic invariant $1/y_+ + 1/y_- = 1$ with anomaly $A_4 \notin \mathbb{Q}$. Whether this anomaly contributes the factor 16 (= $|N_\mathrm{base}|^2$) at the polynomial level is unverified.
- The BCC complex-structure decomposition $V_\mathrm{complex}^2 \cong \mathbb{Z}[i]^2$ (FTD-0122) gives a J-equivariant decomposition of the BCC sublattice. This is the natural geometric realization of $J$'s eigenspace split. Whether it forces a specific *energy* ratio matching $16G^{*2} : 16G^{*3}$ is unverified.

**If any of the three closes**: then the dual substrate would have an independent provenance chain via the J chain extended, and the convergence with the master quadratic would be real coherence (not a re-parameterization).

**If none closes**: then the dual substrate stays a useful interpretive lens for the master quadratic, but the "five independent lines of evidence" framing needs the trim above.

---

## §8 — Status

| Claim | Statement | Tag |
|---|---|---|
| DSP-1 | The J chain derives $G^*$ via Lerch on $D_{1/4}, D_{3/4}$ | [THEOREM] (FTD-0141; this audit does not modify) |
| DSP-2 | The integer 16 has two independent provenance chains (CM auto count, BCC × ternary) | [THEOREM] (canonical; this audit does not modify) |
| DSP-3 | The Vieta relations $(S, P) = (16G^{*2}, 16G^{*3})$ follow from the master quadratic; the dual-substrate identity $(J_L + J_R = G^*, J_L \cdot J_R = G^*/16)$ is the rescaled form of these | [THEOREM] (algebraic; new wording, content unchanged) |
| DSP-4 | $\delta^2 = (4G^*-1)/(4G^*)$ is algebraic dressing on $(S, P)$, not an independent constraint | [THEOREM] (this audit) |
| DSP-5 | The "five independent lines of evidence" framing in EXPLR_GSTAR_ARITHMETIC_IDENTITIES §1 is overclaim | [F1 HYGIENE] (recommendation: trim language; no tag changes downstream) |
| DSP-6 | The header comment in `master_quadratic.h` lines 36–40 (temporal-gauge DoF count) is stale relative to DERIV_MASTER_QUADRATIC_GAP_EQUATION.md §2.2 line 84 | [DOC DRIFT] (recommendation: sync header to canonical theory layer) |
| DSP-7 | Whether the J chain extends to derive $(S, P)$ from independent operator-theoretic invariants | [OPEN] |

---

## §9 — Cross-references

- [`DERIV_GSTAR_QUARTER_CONJUGACY.md`](../03_derivations/DERIV_GSTAR_QUARTER_CONJUGACY.md) — the J chain that derives $G^*$ (FTD-0141).
- [`DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`](../03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md) §2.2 — provenance of 16 (Routes A, B; Route C retracted).
- [`AUDIT_MASTER_QUADRATIC.md`](AUDIT_MASTER_QUADRATIC.md) — the source of the Route C retraction.
- [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) — full FQCR framework; the dual substrate sits structurally adjacent to Models III–V.
- [`engine/include/ftd/ontic/master_quadratic.h`](../../../engine/include/ftd/ontic/master_quadratic.h) Layer 3b — the C++ codification of the dual substrate; carries the stale header comment.
- [`EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md`](../09_mathematical/EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md) — the doc that uses the "five independent lines" framing.
- [`DERIV_BCC_COMPLEX_STRUCTURE.md`](../09_mathematical/DERIV_BCC_COMPLEX_STRUCTURE.md) — geometric realization of $J$ on the BCC sublattice (FTD-0122).
- [`SPEC_ALGEBRAIC_SPINE.md`](../01_reference/SPEC_ALGEBRAIC_SPINE.md) — Theorem 8 (1+i)-tower (FTD-0111) and Theorem 9 ($Q(G^*)$, FTD-0112).
