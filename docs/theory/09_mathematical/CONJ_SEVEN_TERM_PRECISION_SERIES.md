# The Seven-Term Precision Series for 1/α

## Conjecture: α⁻¹ = x₊ + Σ sₙ cₙ |ε|ⁿ to 24-Digit CODATA Agreement

**Date:** 2026-04-17
**Status:** [CONJECTURE] — coefficients are lattice-structural but uniqueness unaudited
**Precision claim:** 24-digit agreement with CODATA 2022 (needs independent re-verification)
**Dependencies:** [DERIV_MASTER_QUADRATIC_CM_LVALUES.md](DERIV_MASTER_QUADRATIC_CM_LVALUES.md), [DERIV_ONE_LOOP_LATTICE_ALPHA.md](../04_coupling/DERIV_ONE_LOOP_LATTICE_ALPHA.md), [DERIV_LFUNCTION_GSTAR_CONNECTION.md](DERIV_LFUNCTION_GSTAR_CONNECTION.md)

---

## §1. Statement

**Conjecture (Seven-Term Series).** *The inverse fine-structure constant is given by*

$$\frac{1}{\alpha} \;=\; x_+ \;+\; \sum_{n=1}^{7} s_n\, c_n\, |\varepsilon|^n \tag{1.1}$$

*where $x_+ = 8G^{*2} + 4G^{*3/2}\sqrt{4G^* - 1} = 137.0361714582\ldots$ is the larger root of the master quadratic, $\varepsilon = e^\pi - \pi - 20 \approx -9.000 \times 10^{-4}$ is the structural expansion parameter (small: $|\varepsilon| \sim 10^{-3}$), and the $(s_n, c_n)$ pairs are:*

| $n$ | $s_n$ | $c_n$ | Integer form | Decimal |
|-----|-------|-------|--------------|---------|
| 1 | $-$ | $9/47$ | $N_c^2 / D$ | 0.19149 |
| 2 | $+$ | $5/64$ | $(N_{\mathrm{eff}} - 2 N_{\mathrm{base}}) / N_{\mathrm{base}}^3$ | 0.07813 |
| 3 | $-$ | $4/141$ | $N_{\mathrm{base}} / (N_c \cdot D)$ | 0.02837 |
| 4 | $-$ | $141/11$ | $(N_c \cdot D) / (b_3 + N_{\mathrm{base}})$ | 12.818 |
| 5 | $-$ | $1472/21$ | $(2 N_{\mathrm{eff}} - N_c)\, N_{\mathrm{base}}^3 / (N_c \cdot b_3)$ | 70.095 |
| 6 | $-$ | $416/21$ | $2\, N_{\mathrm{eff}}\, N_{\mathrm{base}}^2 / (N_c \cdot b_3)$ | 19.810 |
| 7 | $+$ | $299/8$ | $N_{\mathrm{eff}}\,(2 N_{\mathrm{eff}} - N_c) / \mathrm{BCC}$ | 37.375 |

*where the lattice-structural integers are*

$$N_c = 3,\quad N_{\mathrm{base}} = 4,\quad b_3 = 7,\quad N_{\mathrm{eff}} = 13,\quad D = N_c N_{\mathrm{base}}^2 - 1 = 47,\quad \mathrm{BCC} = 8.$$

*The claim is that (1.1) matches CODATA 2022 $\alpha^{-1} = 137.035999177(21)$ to 24 significant digits.*

---

## §2. Status: Why This Is [CONJECTURE]

Three conditions would be needed to upgrade (1.1) to [THEOREM]:

1. **Integer rigidity.** The denominators $\{47, 64, 141, 11, 21, 8\}$ must be **uniquely determined** by the lattice structure. If alternative rational combinations of the same base integers $\{N_c, N_{\mathrm{base}}, b_3, N_{\mathrm{eff}}, D, \mathrm{BCC}\}$ can produce different denominators that also match CODATA to comparable precision, the result is a fit rather than a derivation.

2. **Precision independence of CODATA.** The coefficients must be derivable *before* seeing CODATA values. Currently the coefficients were *selected* to match CODATA, so the 24-digit agreement is circular unless rigidity is established.

3. **Independent verification.** The 24-digit match claim needs independent re-computation from the coefficient table. The current source is a handoff document; no reproduction script has been audited in-tree.

None of the three conditions are currently satisfied. Conditions 1 and 3 are tractable; condition 2 requires a derivation of the coefficients from lattice first principles, which is the deep open problem.

---

## §3. Rigidity Audit: the Falsifier

### 3.1 The critical question

The denominators $\{47, 64, 141, 11, 21, 8\}$ are written as:

- $47 = N_c \cdot N_{\mathrm{base}}^2 - 1$ (the integer $D$)
- $64 = N_{\mathrm{base}}^3$
- $141 = N_c \cdot D$
- $11 = b_3 + N_{\mathrm{base}}$
- $21 = N_c \cdot b_3$
- $8 = \mathrm{BCC}$ (BCC lattice coordination; also $2^3$)

These are lattice-structural. But the same base integers admit other rational combinations of comparable numerical magnitude:

- $47$ could alternatively be $N_c + N_{\mathrm{eff}}^2 / (\text{something})$ or $b_3 \cdot \text{something} - 2$
- $64$ could alternatively be $N_{\mathrm{eff}}^2 - N_{\mathrm{base}}^3 \cdot k$ for some $k$
- $141$ has multiple factorizations: $3 \cdot 47$, $N_c \cdot D$, or $N_{\mathrm{eff}} \cdot N_{\mathrm{base}}^2 - 11$

### 3.2 The explicit falsifier

> **The conjecture upgrades to [THEOREM] if:** the denominators $\{47, 64, 141, 11, 21, 8\}$ are the **unique** lattice-structural expressions consistent with a natural ordering rule (e.g., lowest-complexity Kolmogorov form in $\{N_c, N_{\mathrm{base}}, b_3, N_{\mathrm{eff}}, D, \mathrm{BCC}\}$).
>
> **The conjecture is refuted as a fit if:** there exist alternative denominator assignments from the same base integers that match CODATA to comparable precision with the same sign pattern $\{-, +, -, -, -, -, +\}$.

This is a tractable computer-search question. The proof script at §7 is the recommended audit.

### 3.3 Status of the audit

As of 2026-04-17, the rigidity audit has not been run. The conjecture remains open in the strong sense: neither confirmed nor refuted at the uniqueness level. The 24-digit agreement is preserved as evidence but not as proof.

---

## §4. Structural Interpretation

### 4.1 The expansion parameter

$$\varepsilon \;=\; e^\pi - \pi - 20 \;\approx\; -9.000 \times 10^{-4}$$

This is a transcendental small parameter with $|\varepsilon| \sim 10^{-3}$, not a CM L-value (see [EXPLR_LVALUE_SPAN_CORRECTION_SEARCH.md](EXPLR_LVALUE_SPAN_CORRECTION_SEARCH.md) on why the correction is lattice-side). With $|\varepsilon| \sim 10^{-3}$, a 7-term series can in principle reach 24 digits since each term is ~10³ smaller than the last — convergence is not the issue. The choice $\varepsilon = e^\pi - \pi - 20$ itself deserves a derivation: the integer 20 is not obviously lattice-structural. If the expansion is genuine, $\varepsilon$ should emerge as a natural small parameter from the lattice tadpole integrals, not be inserted.

### 4.2 The base integers

The integers $\{N_c = 3, N_{\mathrm{base}} = 4, b_3 = 7, N_{\mathrm{eff}} = 13\}$ have lattice-structural derivations filed elsewhere:

- $N_c = 3$: [DERIV_NC_FROM_TOPOLOGY.md](../03_derivations/DERIV_NC_FROM_TOPOLOGY.md) (four independent routes to $N_c = 3$)
- $N_{\mathrm{base}} = 4$: cuboctahedral coordination number in $\mathbb{Z}^3$ (vertex figure)
- $b_3 = 7$: independent face pairs under parity in the 27-site Moore lattice
- $N_{\mathrm{eff}} = 13$: cuboctahedral coordination shell (12 surrounding + 1 center)

The derived integers $D = 47$ and $\mathrm{BCC} = 8$ follow. See [BRIDGE_QUADRATIC_PHYSICS.md](../01_reference/BRIDGE_QUADRATIC_PHYSICS.md) §SP5 for the cuboctahedral geometric origin.

### 4.3 Comparison with the one-loop lattice correction

The one-loop tadpole closes the gap from 1.26 ppm to 9.6 ppb — a 99.2% closure with a single correction term ([DERIV_ONE_LOOP_LATTICE_ALPHA.md](../04_coupling/DERIV_ONE_LOOP_LATTICE_ALPHA.md)). The 7-term series claims a 24-digit closure using seven terms. These are two different approaches:

| Approach | Mechanism | Terms | Precision | Status |
|----------|-----------|-------|-----------|--------|
| One-loop tadpole | Lattice QFT with selection $a = 2/D$ | 1 | 9.6 ppb | [SELECTION+THEOREM] |
| Seven-term series | Transcendental $\varepsilon$ expansion | 7 | claimed 24 digits | [CONJECTURE] |

The one-loop approach is more structurally founded but less precise. The 7-term series is more precise but less founded. A genuine reconciliation would derive the 7-term series coefficients from higher-loop lattice diagrams; this is an open problem.

---

## §5. What Would Close This

### 5.1 Near-term (audit-grade)

1. **Reproduce the 24-digit match independently.** Compute (1.1) to 30-digit precision from the coefficient table, compare against CODATA 2022. This is ~30 minutes of PARI work.

2. **Run the rigidity search.** Enumerate rational combinations of $\{N_c, N_{\mathrm{base}}, b_3, N_{\mathrm{eff}}, D, \mathrm{BCC}\}$ with bounded height (e.g., $\leq 10^3$ numerators/denominators), compute the best 7-term fit for each combination, report whether the claimed $\{47, 64, 141, 11, 21, 8\}$ is uniquely best.

3. **Derive $\varepsilon = e^\pi - \pi - 20$ from lattice structure.** The "20" is suspicious; if this quantity is a lattice Brillouin-zone integral at some order, it should emerge rather than be inserted.

### 5.2 Long-term (structural)

4. Derive each $c_n$ coefficient from an $n$-loop lattice diagram. $c_1 = 9/47$ should match a sunset diagram with specific topology; $c_2 = 5/64$ a two-loop tadpole; etc.

5. Verify the sign pattern $\{-, +, -, -, -, -, +\}$ from the loop-expansion structure (alternating signs are natural; the deviations need explanation).

---

## §6. Why the Conjecture Is Preserved Despite Being Unaudited

Three reasons to file rather than discard:

1. **The coefficients are too specific to lose.** The denominators $\{47, 64, 141, 11, 21, 8\}$ are not free parameters; each is a combination of well-defined lattice integers. If the conjecture is refuted, the refutation will be valuable (it shows which base-integer combinations are NOT the right basis). If the conjecture is confirmed, the theorem depends on preserving these coefficients.

2. **The one-loop mechanism already works.** Even if the 7-term series is a post-hoc fit, the FTD story does not depend on it. The one-loop tadpole ([DERIV_ONE_LOOP_LATTICE_ALPHA.md](../04_coupling/DERIV_ONE_LOOP_LATTICE_ALPHA.md)) closes 99.2% of the gap with a principled lattice computation. The 7-term series is a strong bonus, not a load-bearing claim.

3. **The 24-digit precision claim, if true, is consistent with $|\varepsilon| \sim 10^{-3}$ convergence.** Each term is roughly $10^{-3}$ smaller than the last, so 7 terms naturally reach ~21–24 digits of precision. This removes convergence as a red flag, but preserves the real concern: circularity of coefficients (§3) and the ad-hoc-looking integer 20 in $\varepsilon = e^\pi - \pi - 20$ (§4.1).

---

## §7. Recommended Audit Script

```python
# scripts/exploration/audit_seven_term_series.py  (to be written)
from mpmath import mp, mpf, e, pi, sqrt

mp.dps = 50

# Base integers (lattice-structural)
N_c, N_base, b3, N_eff = 3, 4, 7, 13
D = N_c * N_base**2 - 1        # 47
BCC = 8

# Conjectured coefficients
c = [mpf(9)/47,
     mpf(5)/64,
     mpf(4)/141,
     mpf(141)/11,
     mpf(1472)/21,
     mpf(416)/21,
     mpf(299)/8]
s = [-1, +1, -1, -1, -1, -1, +1]

# Expansion parameter
eps = e**pi - pi - 20
absEps = abs(eps)

# Master quadratic
Gstar = mpf('2.9586751191124372375...')
xplus = 8*Gstar**2 + 4*Gstar**(mpf(3)/2) * sqrt(4*Gstar - 1)

# Seven-term sum
correction = sum(s[n] * c[n] * absEps**(n+1) for n in range(7))
alpha_inv = xplus + correction

# Compare with CODATA 2022
codata = mpf('137.035999177')  # (21) uncertainty
print(f"7-term:  {alpha_inv}")
print(f"CODATA:  {codata}")
print(f"Gap:     {abs(alpha_inv - codata)}")
print(f"Gap/σ:   {abs(alpha_inv - codata) / mpf('21e-9')}")
```

A rigidity audit variant would wrap this in an enumeration over alternative denominator assignments and report how many match CODATA to 24 digits.

---

## §8. What This Does and Does Not Claim

**Claims:**

- The coefficient table in §1 exists and is specific.
- The coefficients are expressible in lattice-structural integers $\{N_c, N_{\mathrm{base}}, b_3, N_{\mathrm{eff}}, D, \mathrm{BCC}\}$.
- The handoff source reports 24-digit agreement with CODATA 2022.

**Does not claim:**

- The coefficients are uniquely determined (pending rigidity audit).
- The 24-digit agreement is physically meaningful (could be a fit).
- The series is convergent in the rigorous sense (formally asymptotic with $|\varepsilon| \sim 10^{-3}$; the large late-term coefficients $c_4, \ldots, c_7 \in [12, 70]$ are still dominated by $|\varepsilon|^n$ decay through $n = 7$, but higher $n$ may diverge — behavior of $n \geq 8$ terms is untested).

**Upgrade criterion:** [CONJECTURE] → [THEOREM] if and only if §3.2 rigidity audit passes.

**Refutation criterion:** [CONJECTURE] → [ARCHIVED] if §3.2 rigidity audit fails (i.e., alternative denominator assignments match CODATA to comparable precision).

---

## Document History

- **2026-04-17:** Created. Preserves the 7-term coefficient table with explicit rigidity-audit falsifier. Status [CONJECTURE] pending §3.2 audit and §5.1 reproduction. Cross-references the one-loop mechanism as the structurally grounded alternative.
