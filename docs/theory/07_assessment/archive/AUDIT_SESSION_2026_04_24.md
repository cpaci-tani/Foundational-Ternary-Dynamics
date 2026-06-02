# Session Math/Logic Audit — 2026-04-24

**Date:** 2026-04-24 (session wrap)
**Status:** [AUDIT]
**Purpose:** Verify every numerical and algebraic claim made during the 2026-04-24 session, correct errors, and honestly state which claims survive.

---

## 1. Verified exact identities

| # | Claim | Verification | Status |
|---|---|---|---|
| 1 | $G^* = \Gamma(1/4)/\Gamma(3/4)$ | Definition | EXACT |
| 2 | $\varpi = \Gamma(1/4)^2/(2\sqrt{2\pi})$ | Definition of Bernoulli lemniscatic constant | EXACT |
| 3 | $\Gamma(1/4)\Gamma(3/4) = \pi\sqrt{2}$ | Euler reflection at $z = 1/4$: $\pi/\sin(\pi/4) = \pi\sqrt{2}$ | EXACT |
| 4 | $G^*/\varpi = 2/\sqrt{\pi}$ | Proof below | **EXACT** |
| 5 | Master quadratic $x^2 - 16G^{*2}x + 16G^{*3} = 0$ has $x_+ x_- = 16G^{*3}$ and $x_+ + x_- = 16G^{*2}$ | Vieta | EXACT |
| 6 | $16 = 2^D(D-1)!$ has unique positive-integer solution $D=3$ | $D{=}1{:}2$, $D{=}2{:}4$, $D{=}3{:}16$ ✓, $D{=}4{:}96$, $D{=}5{:}768$ | **EXACT** |
| 7 | $c_{\rm SC} = 1/6$, $c_{\rm BCC} = 1/2$, $c_{\rm FCC} = 1/3$, $c_{\rm 18} = 1/4$ | Taylor expansion of $p_L(\vec k)$ at $k=0$ | EXACT |

### Proof of #4

$$
\frac{G^*}{\varpi} = \frac{\Gamma(1/4)/\Gamma(3/4)}{\Gamma(1/4)^2/(2\sqrt{2\pi})}
= \frac{2\sqrt{2\pi}}{\Gamma(1/4) \cdot \Gamma(3/4)}
= \frac{2\sqrt{2\pi}}{\pi\sqrt{2}}
= \frac{2}{\sqrt{\pi}}. \quad \square
$$

## 2. Verified numerical identities

| # | Claim | Value | Reference | Error |
|---|---|---|---|---|
| 8 | $G^* = 2.95868$ | $\Gamma(1/4)/\Gamma(3/4)$ = $3.62561/1.22542$ = 2.95868 | — | 0 |
| 9 | $\varpi = 2.62206$ | $\Gamma(1/4)^2/(2\sqrt{2\pi})$ = $13.1451/5.01326$ = 2.62206 | — | 0 |
| 10 | $\sqrt[3]{18} = 2.62074$ | $18^{1/3}$ direct | vs $\varpi$: 0.05% | 0.0013 low |
| 11 | $\sqrt[3]{26} = 2.96250$ | $26^{1/3}$ direct | vs $G^*$: 0.13% | 0.0038 high |
| 12 | $G^{*3} = 25.906$ | $2.95868^3$ | vs 26: 0.36% low | — |
| 13 | $(G^*/\varpi)^3 = 8/\pi^{3/2} = 1.4372$ | $(2/\sqrt{\pi})^3$ | vs $26/18 = 1.4444$: 0.50% low | — |
| 14 | $16 G^{*3} = 414.50$ | $16 \times 25.906$ | vs $16 \times 26 = 416$: 0.36% | — |
| 15 | $16 G^{*2} = 140.06$ | $16 \times 8.754$ | (master quadratic linear coef, EXACT by Vieta for the polynomial roots) | — |

## 3. Numerical Watson integrals (new calculation)

Via `engine/tests/test_watson_integrals.cpp` (trapezoidal quadrature, N=200 grid):

| Stencil | Computed | Reference | Err |
|---|---|---|---|
| $W_{\rm SC}$ | 1.51777 | 1.51639 (Watson 1939) | 0.09% |
| $W_{\rm BCC}$ | 1.39028 | $G^{*2}/(2\pi) = 1.39320$ | 0.21% |
| $W_{\rm FCC}$ | 1.34366 | 1.34466 (Joyce 1994) | 0.07% |
| $W_{\rm M18}$ | **1.26886** | (no published reference — first computation) | — |

Ratios:
$$ W_{\rm M18}/W_{\rm BCC} = 0.913 $$

**Not** 3.375 (block volume ratio), **not** 3.628 ($2\pi/\sqrt{3}$), **not** 1.44 (shell ratio). Watson integrals are bulk-insensitive; their ratio is close to 1.

## 4. Errors found and corrected

### Error A — Ladder walk sum (foundation doc §9)

**Wrote:** "walk addends $\{N_{\rm base}, N_{\rm base}, N_c, N_c, N_f\} = \{4, 4, 3, 3, 6\}$"

**Correct:** The walk has 4 addends, not 5. Starting from $n = 4$ (perturbative base):

$$ 4 \xrightarrow{+N_{\rm base} = 4} 8 \xrightarrow{+N_c = 3} 11 \xrightarrow{+N_c = 3} 14 \xrightarrow{+N_f = 6} 20 $$

Sum of addends: $4 + 3 + 3 + 6 = 16$. Final cumulative: $4 + 16 = 20$.

The extra $+4$ I included in the "walk sum" was double-counting the starting value.

**Fix needed:** update the foundation doc to show the walk as 4 addends (not 5).

### Error B — Foundation doc §3 bridge identification

**Wrote:** "$\alpha_\infty/\alpha_{\rm ref} = 27/8 = 3.375$" as the identified bridge factor matching Phase-F measurement of 3.6.

**Correct per [AUDIT_ALPHA_EXTRACTION.md](../10_eft_program/AUDIT_ALPHA_EXTRACTION.md):**
- The 3.6× decomposes as $2 \times 1.8$
- The 2× is a convention artifact (engine `field_energy` has no ½ factor)
- The 1.8× is the lattice Green's function $2 r G_L(r)$ at $r/L \approx 0.31$
- The Phase-F measurement has **zero fine-structure content** (Phase-G resolution)
- Comparing to $\alpha_{\rm ref} = 1/137.036$ is a category error

**Further falsified by Watson-integral computation (§3 above):** even if one were to propose the bridge factor as a Watson ratio, it would be $W_{\rm M18}/W_{\rm BCC} \approx 0.91$, not $27/8 \approx 3.38$.

**Fix already applied:** §3 of foundation doc was corrected.

### Error C — §5 "16·26 = 416 matches x+·x− = 414.4 at 0.4%"

**Wrote:** this as evidence the master quadratic "is" $x^2 - 16 \cdot 26^{2/3} x + 16 \cdot 26$.

**Correct:** the master quadratic coefficients are EXACTLY $16G^{*2}$ and $16G^{*3}$ by definition. The fact that $16G^{*3} \approx 16 \cdot 26 = 416$ at 0.4% is the near-miss $G^{*3} \approx 26$ from the $\sqrt[3]{26} \approx G^*$ observation, NOT an independent empirical match. Restating as two identities makes the same point once.

**Fix needed:** clarify that the "0.4% match" is algebraically the same observation as $\sqrt[3]{26} \approx G^*$.

### Error D — My earlier conflation of Phase-F measurement with structural bridge

I conflated three different quantities:
1. Block volume ratio $27/8 = 3.375$ (structural, geometric)
2. Phase-F measured $\alpha_\infty/\alpha_{\rm ref} \approx 3.6$ (engine measurement, category error per audit)
3. Watson-integral ratio $W_{\rm M18}/W_{\rm BCC} \approx 0.91$ (just computed)

These are three **different quantities measuring three different things**. Their numerical near-coincidence for (1) and (2) was the basis for my original hypothesis; the audit falsifies (2) as a meaningful comparison to anything, and the Watson computation shows (3) is distinct from (1) as well.

**Lesson:** "numerical value near 3.6" is not the same as "measures the same thing." I fell into a near-miss-fitting pattern that the project's own discipline is designed to prevent.

## 5. What actually survives after the audit

| Claim | Pre-session status | Post-audit status |
|---|---|---|
| $G^* = \Gamma(1/4)/\Gamma(3/4)$ generates the master quadratic | [THEOREM] | [THEOREM] unchanged |
| Master quadratic roots give $1/\alpha$ (x+) and $N_c$ (x−) | [STRONGLY MOTIVATED CONJECTURE] / [THEOREM] arithmetic | unchanged |
| $16 = 2^D(D-1)!$ selects $D=3$ uniquely | — | [THEOREM] (proved §1) |
| $G^*/\varpi = 2/\sqrt{\pi}$ exact identity | — | **[THEOREM]** (proved §1) |
| $\sqrt[3]{18} \approx \varpi$ (0.05%) | — | [OBSERVATION] — genuine |
| $\sqrt[3]{26} \approx G^*$ (0.13%) | — | [OBSERVATION] — genuine |
| 8 BCC corners = Gaussian normalization upgrade | — | [OBSERVATION] + structural interpretation |
| Phenomenal/noumenal two-layer ontology | — | [SELECTION] — still useful framing |
| 27/8 is *the* phenomenal-to-noumenal bridge factor | — | **[FALSIFIED]** — two independent tests reject |
| Phase-F 3.6× measures physical α | — | **[CLOSED NEGATIVE]** — category error per earlier audit |
| Watson-integral ratio ≈ 27/8 | — | **[FALSIFIED]** — $W_{\rm M18}/W_{\rm BCC} = 0.913$ |
| Reference frame context = interior-axis integration | — | [CONJECTURE] — philosophical anchor only |
| Fermion emergence from site-local probes fails | — | [THEOREM] (from FTD-0061..0075) |

## 6. Bridge status — post-audit

**There is no single "bridge factor" between phenomenal and noumenal layers.** The two-layer ontology is real (block sizes 2³ vs 3³, stencil types Moore-18 vs BCC), but the quantitative relationship between observables measured on each layer is **observable-dependent**:

| Observable type | Bridge behavior |
|---|---|
| Volume-extensive | $(3/2)^3 = 27/8 = 3.375$ |
| Shell-count | $26/18 = 13/9 \approx 1.44$ |
| Linear length-scale | $G^*/\varpi = 2/\sqrt{\pi} \approx 1.13$ |
| Watson integral (Green's function at origin) | $W_{\rm M18}/W_{\rm BCC} \approx 0.91$ (computed) |
| Phase-F α extraction | $\approx 3.6$ (but this measures lattice Coulomb, not α — category error) |

**The "bridge" is not a scalar; it is a functor from observables-on-Moore-18 to observables-on-BCC, with different numerical translation factors depending on the scaling dimension and specific geometry of the observable.**

This is actually a more accurate (and more useful) picture than the single-factor hypothesis. It means the phenomenal/noumenal split has multiple quantitative relations, not one.

## 7. Updated research programs

Program D (proper bridge measurement) is now better-specified:

**Program D revised.** For each class of observable:

1. **Lepton pole masses** ($m_\mu/m_e$, $m_\tau/m_e$): already [THEOREM] from BCC arithmetic. Bridge check: engine measurement under Moore-18 should give close to the same ratios (up to stencil corrections of order $W_{\rm M18}/W_{\rm BCC} \approx 0.91$ or similar 5-10% effects). If engine gives very different ratios, the phenomenal/noumenal split breaks.

2. **Coulomb coefficient at long $r$**: natively lattice-geometric (Phase-G result). Not a bridge test.

3. **Shell density / Moore coordination**: scales with shell count ratio 26/18.

4. **Action-per-unit-volume**: scales with block volume 27/8.

Each gives a different bridge factor, and they should all be measurable.

## 8. What I want to keep explicit

Going forward:

- **The 27/8 was a premature identification.** The block-volume ratio is structurally meaningful but doesn't match any single engine measurement we have.
- **The 3.6× is not a bridge measurement.** It's lattice-Coulomb-geometry times a convention factor.
- **Watson-integral bridge is ≈1.** The Green's function at origin is bulk-insensitive to stencil.
- **The two-layer ontology stands**, but the bridge is now a *function of observable*, not a scalar.
- **The $\Gamma$-function identities survive** — these are real algebraic facts, not hypothesis-fits.
- **The near-identities** ($\sqrt[3]{18} \approx \varpi$, $\sqrt[3]{26} \approx G^*$) survive as genuine observations at 0.05% and 0.13%, but they are **not** exact theorems.

## 9. Verdict

The session's synthesis is **80% intact**: two-layer ontology, $\Gamma$-function identities, near-miss observations, tag reclassification all survive. 

The **20% that failed** was my attempt to identify a single bridge factor by matching the 27/8 volume ratio to the Phase-F measurement — a match that was both an audit-level category error and (independently) a Watson-integral mismatch.

**The foundation doc has been corrected in §3 and §6 (tag downgrades and bridge-status clarification).** The LEDGER row FTD-0078 has been updated to record the post-audit state.

Going forward, "the bridge" is treated as **a family of observable-specific factors**, not a single number. This is more honest and makes each factor separately testable.

---

*Filed 2026-04-24 as the session's math/logic audit. Identifies four specific errors (ladder walk sum, bridge identification, algebraic redundancy, quantity conflation), corrects each, preserves the 80% of the synthesis that is rigorous, and explicitly downgrades the 20% that was premature. This is exactly what the project's epistemic discipline is designed for: catching near-miss fits before they propagate into false theorems.*
