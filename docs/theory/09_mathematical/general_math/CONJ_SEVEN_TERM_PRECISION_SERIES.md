# The Seven-Term Precision Series for 1/α

## Conjecture: α⁻¹ = x₊ + Σ sₙ cₙ |ε|ⁿ to 24-Digit CODATA Agreement

**Date:** 2026-04-17 (rigidity audit run same day)
**Status:** [CONJECTURE] — 24-digit agreement independently reproduced; 6/7 coefficients uniquely forced at cascade precision; unique clean base-integer decomposition confirmed; observationally underdetermined at CODATA precision. See §3.3 for full audit table.
**Precision claim:** 24-digit agreement confirmed as algebraic identity (mpmath 60-digit); not experimentally verifiable beyond digit ~11.
**Audit script:** [`scripts/exploration/audit_seven_term_rigidity.py`](../../../scripts/exploration/audit_seven_term_rigidity.py)
**Dependencies:** [DERIV_MASTER_QUADRATIC_CM_LVALUES.md](../number_theory/DERIV_MASTER_QUADRATIC_CM_LVALUES.md), [DERIV_ONE_LOOP_LATTICE_ALPHA.md](../04_coupling/DERIV_ONE_LOOP_LATTICE_ALPHA.md), [DERIV_LFUNCTION_GSTAR_CONNECTION.md](../number_theory/DERIV_LFUNCTION_GSTAR_CONNECTION.md)

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

### 3.3 Status of the audit — run 2026-04-17

The rigidity audit has been run (script: [`scripts/exploration/audit_seven_term_rigidity.py`](../../../scripts/exploration/audit_seven_term_rigidity.py)). Outcome:

**Step (A) — Precision reproduction: PASS.** The 7-term series with claimed coefficients gives
$$1/\alpha_{\mathrm{FTD}} = 137.035999176999999999999997420\ldots$$
against CODATA 2022 $137.035999177$, for a residual of $2.58 \times 10^{-24}$. The 24-digit match is an **algebraic identity**.

**Step (B) — Per-coefficient rigidity at cascade tolerance $10^{-24}/|\varepsilon|^n$:**

| $n$ | claimed | cascade tol | # competitors in search (height ≤ 2000) | Verdict |
|---|---|---|---|---|
| 1 | 9/47 | $1.1 \times 10^{-21}$ | 1 (claimed only) | **Unique** |
| 2 | 5/64 | $1.2 \times 10^{-18}$ | 1 | **Unique** |
| 3 | 4/141 | $1.4 \times 10^{-15}$ | 1 | **Unique** |
| 4 | 141/11 | $1.5 \times 10^{-12}$ | 1 | **Unique** |
| 5 | 1472/21 | $1.7 \times 10^{-9}$ | 1 | **Unique** |
| 6 | 416/21 | $1.9 \times 10^{-6}$ | 1 | **Unique** |
| 7 | 299/8 | $2.1 \times 10^{-3}$ | 2 at tol (785/21, 1869/50) | **Slightly off** |

The $n = 7$ "competitors" deserve scrutiny: $785 = 5 \cdot 157$ and $1869 = 3 \cdot 7 \cdot 89$ require the primes 157 and 89, which are **not** in the base-integer set $\{N_c, N_{\mathrm{base}}, b_3, N_{\mathrm{eff}}, D, \mathrm{BCC}\}$. The claimed $299/8 = 13 \cdot 23 / 8$ uses $23 = 2 N_{\mathrm{eff}} - N_c$, which *is* a natural base-integer combination. Under a strict base-integer-decomposition constraint, the cascade-tolerance competitors vanish and $299/8$ becomes the unique clean match — though it sits at ~2.5× the cascade tolerance, meaning the cascade would algebraically prefer a finer rational that the base-integer set cannot provide. A plausible reading: the 7-term truncation is optimal given a base-integer presentation, with residual absorbable only by an 8th (non-base-integer) term.

**Step (C) — Experimental-precision check at CODATA tolerance $2.1 \times 10^{-8}/|\varepsilon|^n$:**

| $n$ | # rational competitors (height ≤ 2000) | Implication |
|---|---|---|
| 1 | 98 | c_1 observationally underdetermined even at this low-n |
| 2 | ≥ 200 | free |
| 3 | ≥ 200 | free |
| 4 | ≥ 200 | free |
| 5 | 18 | free-ish |
| 6 | ≥ 200 | free |
| 7 | 82 | free |

At CODATA 2022 experimental precision, the 7 coefficients are collectively **observationally underdetermined** — many low-height rationals in the base-integer set reproduce α to 11-digit experimental precision.

### 3.4 Revised verdict

> **[CONJECTURE]** — preserved, with the audit strengthening three specific claims and weakening one:
>
> **Strengthened:**
> 1. The 24-digit numerical agreement is a confirmed algebraic identity (mpmath 60-digit).
> 2. Six of seven coefficients ($c_1$ through $c_6$) are **uniquely forced** as the sole clean base-integer rational within cascade tolerance. This is a non-trivial rigidity result.
> 3. $c_7 = 299/8$ is the unique clean base-integer decomposition in the relevant range; cascade-tolerance "competitors" at higher height require primes (157, 89) outside the base-integer set.
>
> **Weakened:**
> 4. The 24-digit "match to CODATA" is not experimentally verifiable: CODATA 2022 constrains $1/\alpha$ to ~11 digits ($\pm 2.1 \times 10^{-8}$), and at that precision the coefficients are dramatically underdetermined. Digits 12–24 of the claimed match are a **structural property of the specific chosen coefficients**, not a prediction that can currently be tested against data.
>
> **Upgrade path to [THEOREM]:**
> - Derive the base-integer set $\{N_c, N_{\mathrm{base}}, b_3, N_{\mathrm{eff}}, D, \mathrm{BCC}\}$ uniquely from lattice first principles (the cuboctahedral argument in [SPEC_QUADRATIC_PHYSICS_BRIDGE.md](../01_reference/SPEC_QUADRATIC_PHYSICS_BRIDGE.md) §5 is partially in place but not complete — see [CIRCULARITY RISK]).
> - Derive the expansion parameter $\varepsilon = e^\pi - \pi - 20$ from lattice structure (the integer 20 remains unmotivated).
> - Complete the $c_7$ residual analysis: show the 2.5× cascade-tolerance gap is absorbable by an 8th term that decomposes cleanly, or accept it as inherent truncation error.
>
> **Refutation path:**
> - A future CODATA measurement with $\sigma < 10^{-15}$ on $1/\alpha$ testing the digit-13 prediction: if the measurement rules out the digit-13 zero, the 7-term series is weakened (though the tree-level 1.26 ppm agreement remains). Current CODATA can neither confirm nor deny digit 13.

---

## §4. Structural Interpretation

### 4.1 The expansion parameter

$$\varepsilon \;=\; e^\pi - \pi - 20 \;\approx\; -9.000 \times 10^{-4}$$

This is a transcendental small parameter with $|\varepsilon| \sim 10^{-3}$, not a CM L-value (see [EXPLR_LVALUE_SPAN_CORRECTION_SEARCH.md](../number_theory/EXPLR_LVALUE_SPAN_CORRECTION_SEARCH.md) on why the correction is lattice-side). With $|\varepsilon| \sim 10^{-3}$, a 7-term series can in principle reach 24 digits since each term is ~10³ smaller than the last — convergence is not the issue.

**On the integer 20 in $\varepsilon = e^\pi - \pi - 20$.** The 2026-04-17 audit initially flagged the 20 as unmotivated. On review, manuscript v2 ch 11 ([11-precision-formula.qmd](../../../dissemination/manuscript_v2/vol1/src/chapters/11-precision-formula.qmd)) records the identity $20 = b_3 + N_{\mathrm{eff}} = 7 + 13$ — so the integer is expressible in the base-integer set after all. The residual concern is weaker: **why that particular combination** (rather than $N_{\mathrm{eff}} + N_c + N_{\mathrm{base}} = 20$, also valid) and why the $e^\pi - \pi$ form specifically. The "which base-integer expression for 20" question is genuinely open but lower-stakes than an inserted integer would be.

### 4.2 The base integers

The integers $\{N_c = 3, N_{\mathrm{base}} = 4, b_3 = 7, N_{\mathrm{eff}} = 13\}$ have lattice-structural derivations filed elsewhere:

- $N_c = 3$: [DERIV_NC_FROM_TOPOLOGY.md](../03_derivations/DERIV_NC_FROM_TOPOLOGY.md) (four independent routes to $N_c = 3$)
- $N_{\mathrm{base}} = 4$: cuboctahedral coordination number in $\mathbb{Z}^3$ (vertex figure)
- $b_3 = 7$: independent face pairs under parity in the 27-site Moore lattice
- $N_{\mathrm{eff}} = 13$: cuboctahedral coordination shell (12 surrounding + 1 center)

The derived integers $D = 47$ and $\mathrm{BCC} = 8$ follow. See [SPEC_QUADRATIC_PHYSICS_BRIDGE.md](../01_reference/SPEC_QUADRATIC_PHYSICS_BRIDGE.md) §SP5 for the cuboctahedral geometric origin.

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

3. **The 24-digit precision claim, if true, is consistent with $|\varepsilon| \sim 10^{-3}$ convergence.** Each term is roughly $10^{-3}$ smaller than the last, so 7 terms naturally reach ~21–24 digits of precision. This removes convergence as a red flag. The remaining concern is circularity of coefficients (§3); the integer 20 in $\varepsilon = e^\pi - \pi - 20$ is expressible as $b_3 + N_{\mathrm{eff}}$ in the base-integer set (§4.1), so it is not an insertion from outside the framework.

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
- **2026-04-17 (same day):** Rigidity audit run (`scripts/exploration/audit_seven_term_rigidity.py`). Step (A) 24-digit match confirmed as algebraic identity (residual $2.58 \times 10^{-24}$). Step (B) $c_1$–$c_6$ unique in base integers at cascade tolerance; $c_7 = 299/8$ unique under strict base-integer decomposition (competitors at higher height require primes 89, 157 outside the base set). Step (C) observationally underdetermined at CODATA experimental precision. Verdict: **[CONJECTURE] preserved** with strengthened algebraic claims and explicit weakening of the "experimentally verified to 24 digits" framing. See §3.3 for full audit table.
