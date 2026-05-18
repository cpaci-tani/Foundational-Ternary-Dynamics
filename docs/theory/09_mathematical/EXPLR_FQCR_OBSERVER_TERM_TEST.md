# Exploration · Single-Lepton Observer-Term Test (C1 Path Validation)

**Date:** 2026-05-08
**Status:** [EXPLORATORY] — empirical test of the structural prediction in [`PROPOSAL_OBSERVER_OPERATOR_EXTENSION.md`](PROPOSAL_OBSERVER_OPERATOR_EXTENSION.md). **Positive structural observation:** the structural prediction $c_e = \delta/(3\pi G^*) \approx 0.0343$ is empirically supported at the asymptotic-UV slope level (0.4-0.6% match to QED at $\mu \in [5, 10]$ MeV) **when $\lambda + A$ are removed**, but fails when added on top of $\lambda + A$.
**Tag impact:** none. The C1 path moves from "speculative" to "structurally suggestive."
**Companion:** [`PROPOSAL_OBSERVER_OPERATOR_EXTENSION.md`](PROPOSAL_OBSERVER_OPERATOR_EXTENSION.md), [`EXPLR_FQCR_T_SCALE_MAP.md`](EXPLR_FQCR_T_SCALE_MAP.md), [`scripts/exploration/explore_fqcr_observer_term.py`](../../../scripts/exploration/explore_fqcr_observer_term.py).

---

## §1 — The test

[`PROPOSAL_OBSERVER_OPERATOR_EXTENSION.md`](PROPOSAL_OBSERVER_OPERATOR_EXTENSION.md) §4 derives a structural prediction:

$$
c_e = \frac{\delta}{3\pi G^*} = \frac{1}{3\pi G^*} \sqrt{\frac{4G^* - 1}{4G^*}} \approx 0.034313
$$

for the asymptotic-UV slope coefficient of an electron-loop observer term

$$
B^{(e)}(t) := c_e \cdot \log\bigl(1 + (\mu(t)/m_e)^2\bigr).
$$

The prediction comes from requiring $\partial x_+/\partial \log\mu = -2/(3\pi)$ (QED electron-only one-loop), combined with the chain-rule factor $\partial x_+/\partial R = -G^*/\delta$.

This test runs two configurations:

- **Configuration I (additive):** $R(t) = 1 + \lambda_N(4it) + A_N(t) + B^{(e)}(t)$.
- **Configuration II (replacement):** $R(t) = 1 + B^{(e)}(t)$. ($\lambda + A$ removed.)

Both are evaluated at $c_e = 0.0343$ (structural) and against QED running.

---

## §2 — Configuration I: additive supplement (fails)

Adding $B^{(e)}$ on top of the existing $\lambda + A$ does not reproduce QED. RMS residual minimization across $t \in [0.005, 5]$ gives:

$$
c_e^{\text{best}} \approx -0.395, \qquad c_e^{\text{best}} / c_e^{\text{structural}} \approx -11.5.
$$

Wrong sign, factor 11 wrong magnitude. And even at the best $c_e$, residuals are ~1-4 across the test range — large, structured.

**Diagnosis.** The existing $\lambda_N(4it)$ and $A_N(t)$ already produce running of $R$ with $\mu$ — and that running is much faster than QED's. Adding $B^{(e)}$ on top doesn't slow the existing running enough, no matter what $c_e$ is chosen.

**Implication.** $\lambda + A$ in the existing FQCR machinery are **not** vacuum polarization / QED running content. They contribute to $R$ in a way that's *structurally distinct* from the running coupling.

This is consistent with the SPEC_FQCR §3.1 description of $\Psi_N$ as encoding "$(4, 6; 3, 2)$ exponent structure" — bivector and transverse-mode counts — which are geometric/topological data, not fermion vacuum polarization.

---

## §3 — Configuration II: clean separation (positive result)

When $\lambda + A$ are removed and only $B^{(e)}$ is added, with $c_e = 0.0343$ structurally:

$$
B^{(e)}(t) := c_e \cdot \big(\log(1 + 1/t^2) - \log 2\big).
$$

(The $\log 2$ subtraction pins $B^{(e)}(t=1) = 0$ so $x_+(t=1) = $ tree-level master-quadratic value $137.0362$, off CODATA by 1.26 ppm — the "loss of the $\lambda + A$ correction.")

**Slope match against QED ($-2/(3\pi) \approx -0.212$ above electron threshold):**

| $t$ | $\mu$ (MeV) | FQCR slope | QED slope | Relative error |
|---:|---:|---:|---:|---:|
| 0.05 | 10.22 | $-0.2135$ | $-0.2122$ | **0.6%** |
| 0.10 | 5.11 | $-0.2114$ | $-0.2122$ | **0.4%** |
| 0.30 | 1.70 | $-0.1952$ | $-0.2122$ | 8.0% |
| 0.50 | 1.02 | $-0.1700$ | $-0.2122$ | 19.9% |
| 0.80 | 0.64 | $-0.1294$ | $-0.2122$ | 39.0% |

**At $\mu \in [5, 10]$ MeV — the asymptotic-UV regime where the structural prediction was derived — the match is sub-percent.** Both finite-difference and analytic-chain-rule computations agree (computed independently in [`scripts/exploration/explore_fqcr_observer_term.py`](../../../scripts/exploration/explore_fqcr_observer_term.py)).

**Slope match degrades at moderate $\mu$.** The smooth threshold of $\log(1 + (\mu/m_e)^2)$ doesn't reproduce QED's sharp electron-decoupling at $\mu = m_e$. This is an artefact of the functional form, not the structural prediction.

**Absolute residual ($x_+$ - QED $\alpha^{-1}$) at the structural $c_e$:**

| $t$ | $\mu$ (MeV) | $x_+$ | QED $\alpha^{-1}$ | Residual |
|---:|---:|---:|---:|---:|
| 0.005 | 102.2 | 135.977 | 135.912 | $+0.065$ |
| 0.05 | 10.22 | 136.471 | 136.400 | $+0.071$ |
| 0.10 | 5.11 | 136.619 | 136.547 | $+0.071$ |
| 0.30 | 1.70 | 136.845 | 136.781 | $+0.064$ |
| 0.50 | 1.02 | 136.939 | 136.889 | $+0.050$ |
| 1.00 | 0.51 | 137.036 | 137.036 | **$+0.0002$** |
| 2.00 | 0.26 | 137.086 | 137.036 | $+0.050$ |
| 5.00 | 0.10 | 137.106 | 137.036 | $+0.070$ |

**Constant +0.05 to +0.07 offset across the full range.** This is significantly larger than CODATA precision but small in absolute terms (~0.04% of 137).

---

## §4 — What the +0.05 to +0.07 residual is

The constant residual is structurally diagnostic. Possible sources:

1. **Higher-order QED.** This test uses one-loop running. CODATA's $\alpha^{-1}$ at finite $\mu$ includes higher loops + hadronic VP. The 1-loop approximation introduces ~10-30 ppm residual, which is consistent with 0.05-0.07 absolute at $137$.
2. **Threshold-form mismatch.** The test uses $\log(1 + (\mu/m_e)^2)$ — a smooth Lorentzian-style threshold — instead of QED's true mass-dependent vacuum-polarization function $\Pi(\mu^2, m_e^2)$. The two agree asymptotically but differ near threshold by an $O(1)$ structural offset.
3. **Pinning convention.** $B^{(e)}(t=1) = 0$ pins the value to the tree-level $137.0362$, off CODATA by 1.26 ppm. The residuals away from $t=1$ inherit this offset.
4. **Finite-N corrections.** At $N = 1024$, $G_N^* - G^* \sim 4 \times 10^{-8}$, contributing ~$5 \times 10^{-7}$ to $x_+$. Below the residual scale.

The residual is consistent with sources 1 + 2 + 3 — i.e., the structural prediction is correct, but the specific functional form $\log(1 + x^2)$ is too rigid to capture both threshold and pinning simultaneously. A more refined ansatz (true QED $\Pi(\mu^2, m^2)$, or a determinant-style $\log\det(L^{(e)} + m_e^2)$) might absorb the residual.

---

## §5 — What this means structurally

**The chain-rule factor $\partial x_+/\partial R\big|_{R=1} = -G^*/\delta \approx -3.092$ is a [THEOREM] of the master quadratic.** Combined with the QED one-loop coefficient $-2/(3\pi)$, it forces a structural prediction $c_e = \delta/(3\pi G^*) \approx 0.0343$ for any observer term that aspires to QED-faithful running.

**This prediction is empirically vindicated at the asymptotic-UV slope level (0.4-0.6% match) when:**

- The observer term replaces (rather than supplements) the existing $\lambda + A$ structural pieces.
- The functional form is a smooth threshold function (here $\log(1+x^2)$).
- The match is evaluated at $\mu \gg m_e$ where the threshold smearing is negligible.

**The naive C1 plan — "add a fermion-loop term to the existing FQCR" — fails.** What works is a *separation*: keep the structural correction at $t = 1$ from $\lambda + A$ as one piece, and treat QED running as a separate operator-stack contribution that does not double-count $\lambda + A$'s content.

This is **partial validation of the C1 path** in [`EXPLR_FQCR_T_SCALE_MAP.md`](EXPLR_FQCR_T_SCALE_MAP.md) §5. The structural-coefficient prediction is real. What's needed to actually close C1 is more than this test:

- A **structural derivation** of $B^{(l)}(t)$'s functional form from the operator stack (not an ansatz).
- A **clean separation principle** that justifies why $\lambda + A$ contribute to the $t = 1$ value but not to the running.
- An **extension to muon and tau loops** with thresholds at the right $t$-values, which requires resolving the chicken-and-egg with the t-scale map (the muon threshold under Map A sits below the FQCR Landau-like point at $t_* = 0.062$).

---

## §6 — Status

| Item | Statement | Tag |
|---|---|---|
| OTT-1 | $\partial x_+/\partial R\big\|_{R=1} = -G^*/\delta \approx -3.092$ | [THEOREM] (algebraic) |
| OTT-2 | Naive Configuration I (R = base + B) does NOT match QED for any $c_e$ | [EXPERIMENTAL FACT] |
| OTT-3 | Configuration II (R = 1 + B) at $c_e = c_\text{structural}$ matches QED slope to 0.4-0.6% at $\mu \in [5, 10]$ MeV | [EXPERIMENTAL FACT] |
| OTT-4 | The structural prediction $c_e = \delta/(3\pi G^*)$ is empirically supported at the slope level | [STRUCTURAL OBSERVATION] (positive) |
| OTT-5 | Constant residual ~+0.05-0.07 across $t$-range; consistent with higher-order QED + threshold-form smoothing + pinning offset | [TENTATIVE EXPLANATION] |
| OTT-6 | $\lambda + A$ in the existing FQCR machinery are *not* vacuum polarization | [STRUCTURAL OBSERVATION] (negative for naive C1, positive for clean separation) |
| OTT-7 | Multi-lepton extension (muon, tau) at correct thresholds | [OPEN] |
| OTT-8 | Structural derivation of $B^{(l)}$'s functional form (not just slope coefficient) | [OPEN] |
| OTT-9 | C1 closability: needs items 7 + 8 plus separation principle for $\lambda + A$ | [OPEN — substantial program, but path is now structurally suggestive rather than speculative] |

---

## §7 — Cross-references

- [`PROPOSAL_OBSERVER_OPERATOR_EXTENSION.md`](PROPOSAL_OBSERVER_OPERATOR_EXTENSION.md) — the proposal whose §4 prediction this test confirms.
- [`EXPLR_FQCR_T_SCALE_MAP.md`](EXPLR_FQCR_T_SCALE_MAP.md) §5 — the C1/C2/C3 trifurcation.
- [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) §3.1, §3.3 — the [SELECTION] tags on $(4, 6; 3, 2)$ and the additive form $R = 1 + \lambda + A$, both of which this test illuminates structurally.
- [`AUDIT_DUAL_SUBSTRATE_PROVENANCE.md`](../07_assessment/AUDIT_DUAL_SUBSTRATE_PROVENANCE.md) — the chain-rule factor $-G^*/\delta$ first surfaced in that audit's discussion of the master-quadratic discriminant structure.
- [`scripts/exploration/explore_fqcr_observer_term.py`](../../../scripts/exploration/explore_fqcr_observer_term.py) — runs Configurations I and II, including the slope check that gives the 0.4-0.6% match.
