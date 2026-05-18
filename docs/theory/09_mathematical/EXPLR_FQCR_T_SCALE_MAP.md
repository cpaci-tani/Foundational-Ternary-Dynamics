# Exploration · FQCR Model V $t \leftrightarrow$ Scale Map Candidates

**Date:** 2026-05-08
**Status:** [EXPLORATORY] — addresses the gating question for [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) §6 Test 3 (running behaviour). **Does not close the question.**
**Tag impact:** none. The [SELECTION] tag on $t = 1$ in SPEC_FQCR §3.2 stands. The "physical interpretation of $t$" listed under SPEC_FQCR §7 (out of scope) remains open.
**Companion:** [`EXPLR_FQCR_RESPONSE_LAW_TEST.md`](EXPLR_FQCR_RESPONSE_LAW_TEST.md), [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md), [`scripts/exploration/explore_fqcr_t_scale_map.py`](../../../scripts/exploration/explore_fqcr_t_scale_map.py).

---

## §1 — The question

[`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) §3.2 declares $t = 1$ as the [SELECTION] base point at which the FQCR Model V branch readout is identified with $\alpha^{-1}$:

$$
\alpha^{-1} \;\stackrel{?}{=}\; \lim_{N \to \infty} x_+(N, 1).
$$

§6 Test 3 is the natural follow-up — *running behaviour* — but it is gated on a physical interpretation of $t$ that the framework does not yet provide. Without a $t \leftrightarrow$ scale map, $x_+(t)$ at $t \neq 1$ is mathematics, not physics.

This doc proposes three candidate maps with structural arguments, tests the most natural one against QED running, and reports an honest verdict on closability.

---

## §2 — Three candidate maps

### Map A — Heat-kernel proper time, $\mu = m_*/t$

**Structural argument.** The FQCR operators $L_{1/4,N}$, $L_{3/4,N}$ are diagonal Hamiltonians with quarter-integer spectra. Their heat kernels are

$$
\mathrm{Tr}(e^{-\beta L_a}) = \frac{e^{-a\beta}}{1 - e^{-\beta}},
$$

which is the canonical 0+1-dimensional thermal partition function with inverse temperature $\beta$. The FQCR convention $Q = e^{-2\pi t}$ identifies $t = \beta/(2\pi)$. For a 0+1 spectrum (no kinetic Laplacian), the natural mass-scale identification is $\mu \sim 1/\beta \sim 1/t$. Pinning $t = 1 \leftrightarrow \mu = m_*$ for $m_*$ an FTD natural mass unit gives the map.

**FTD natural mass unit.** Per [`SPEC_DIMENSIONAL_MAP.md`](../01_reference/SPEC_DIMENSIONAL_MAP.md), $K_B \equiv m_e$ in the calibration. So $m_* = m_e$. Then $t = 1 \leftrightarrow \mu = m_e$, and CODATA $\alpha^{-1}(m_e) \approx 137.036$ matches $x_+(1) = 137.036$ by design.

### Map B — Logarithmic / RG, $\mu = m_e \cdot e^{(1 - t)/c}$

**Structural argument.** In QFT, RG variables are typically logarithmic: $t_\mathrm{RG} = \log(\mu/\mu_0)$. Inverting: $\mu = \mu_0 \cdot e^{t_\mathrm{RG}}$. Pinning $t = 1 \leftrightarrow \mu = m_e$ requires shifting and scaling: $\mu = m_e \cdot \exp((1 - t)/c)$ for some natural $c$. With $c = 1$, $t \to 0 \Rightarrow \mu \to e \cdot m_e$, and $t \to \infty \Rightarrow \mu \to 0$.

This map gives a much-compressed $\mu$ range over $t \in [0.06, 1]$ — about a factor of $e$, compared to factor 17 for Map A.

### Map C — Lattice cutoff, $\mu = m_P / \sqrt{t}$

**Structural argument.** In FTD's lattice ontology with $a_\mathrm{phys} \equiv \ell_P$, the natural mass cutoff is $1/\ell_P = m_P$. If $t$ scales as $a^2/a_0^2$ for some reference spacing $a_0$, then $\mu = 1/a = m_P/\sqrt{t}$. Then $t = 1 \leftrightarrow \mu = m_P$ (Planck scale), and small $t$ is sub-Planck (continuum limit $a \to 0$).

This map identifies the FQCR base point with the Planck scale, not $m_e$.

---

## §3 — Test: comparison against one-loop QED running

QED one-loop with leptons (electron, muon, tau):

$$
\alpha^{-1}(\mu) = \alpha^{-1}(m_e) - \frac{2}{3\pi} \sum_{l\,:\,m_l < \mu} \log(\mu/m_l).
$$

Slope: $d\alpha^{-1}/d(\log\mu) = -2/(3\pi) \cdot N_\mathrm{active}(\mu)$. Numerical values: $-0.212$ (electron only, $\mu < m_\mu$); $-0.424$ ($e + \mu$); $-0.637$ (all three).

### §3.1 — Map A numerical results

At $N = 1024$, $t \in \{0.1, 0.3, 0.5, 0.8, 1.0, \ldots\}$:

| $t$ | $\mu$ (MeV) | $x_+$ (FQCR) | $\alpha^{-1}$ (QED) | $x_+ - \alpha^{-1}$ |
|---:|---:|---:|---:|---:|
| 0.10 | 5.110 | 126.39 | 136.55 | $-10.16$ |
| 0.30 | 1.703 | 136.13 | 136.78 | $-0.65$ |
| 0.50 | 1.022 | 136.95 | 136.89 | $+0.06$ |
| 0.80 | 0.639 | 137.034 | 136.989 | $+0.045$ |
| **1.00** | **0.511** | **137.036** | **137.036** | $\mathbf{0.000}$ |
| 1.50 | 0.341 | 137.0362 | 137.036 | $+0.0002$ |
| ≥ 2.00 | ≤ 0.256 | 137.0362 | 137.036 | $+0.0002$ |

At $t = 1$ the match is by construction. Above $t = 1$ (deep IR), both sides saturate. **The interesting regime is $t < 1$ (UV-running region):**

- Near $t = 1$: matches improves up to ~few percent of QED slope.
- Far from $t = 1$ (small $t$): FQCR runs *much faster than QED*.

### §3.2 — Slope comparison

Numerical $dx_+/d(\log\mu)$ under Map A versus QED prediction:

| $t$ | $\mu$ (MeV) | FQCR slope | QED slope (active leptons) |
|---:|---:|---:|---:|
| 1.50 | 0.341 | $-0.000$ | $0$ (μ < m_e) |
| **1.00** | **0.511** | **−0.002** | **0** (at threshold) |
| 0.80 | 0.639 | $-0.021$ | $-0.212$ (e only, μ slightly > m_e) |
| 0.50 | 1.022 | $-0.541$ | $-0.212$ (e only) |
| 0.30 | 1.703 | $-2.857$ | $-0.212$ (e only) |

**At $\mu \approx 1$ MeV the FQCR slope is $-0.541$, vs. QED's electron-only $-0.212$. FQCR's running is $\approx 2.5\times$ steeper than QED predicts.** At $\mu \approx 1.7$ MeV the ratio is $13.5\times$.

This rules out a clean identification of the FQCR-EM branch readout with the QED running coupling under Map A.

### §3.3 — Map B and Map C results

**Map B (RG-log, $c = 1$):** compresses $\mu$ range from $\mu(t=0) \approx 1.39$ MeV to $\mu(t=\infty) \to 0$. The Landau-like point at $t \approx 0.062$ corresponds to $\mu \approx 1.31$ MeV. But the FQCR slope vs $\log\mu$ becomes near-vertical, since a small change in $t$ is a small change in $\log\mu$ in this map. The mismatch with QED is qualitatively the same as Map A: FQCR running is much faster than QED.

**Map C (lattice cutoff):** $t = 1 \leftrightarrow \mu = m_P$. Then $\alpha^{-1}(m_P)$ from full-SM running of CODATA ≈ 107 (lepton-only one-loop), but $x_+(t=1) = 137.036$. **30-unit mismatch at the base point.** Map C is structurally inconsistent with the FQCR base value at $t = 1$.

---

## §4 — The structural Landau-like point

The FQCR-EM branch has a hard structural feature: at the value of $t$ where $R_N(t) = 4 G_N^*$, the discriminant $\sqrt{4G^* - R}$ vanishes and $x_+$ merges with $x_-$. Numerically (additive law, $N = 1024$):

$$
t_* \approx 0.0618, \qquad R(t_*) \approx 11.835 = 4 G_N^*.
$$

Physically this is "where the EM branch coalesces with the color branch" — where $1/\alpha$ becomes equal to $N_c$ in the master quadratic readout.

Under each candidate map, $t_*$ corresponds to:

| Map | $\mu(t_*)$ | Physical interpretation |
|---|---:|---|
| A (heat-kernel) | 8.27 MeV | $\sim 16\,m_e$, just above electron threshold |
| B (RG-log) | 1.31 MeV | $\sim 2.5\,m_e$, near electron mass |
| C (lattice) | $4.9 \times 10^{22}$ MeV | $\sim 100\,m_P$, Planck-scale |

**For comparison, the QED Landau pole** (electron-only one-loop): $\mu_L = m_e \exp(3\pi/(2\alpha)) \approx 10^{280}$ MeV. **None of the three maps puts the FQCR Landau-like point at the QED Landau pole.**

This is structurally diagnostic. If FQCR-EM faithfully represented QED running, $t_*$ should map to $\sim 10^{280}$ MeV. Under Map A it maps to $\sim 10$ MeV (270 orders too low). Under Map C it maps to $\sim 10^{22}$ MeV (still 258 orders off).

---

## §5 — Verdict on closability

**The simplest reading of the data:** the FQCR-EM branch readout $x_+(N, t)$ is **not** a faithful representation of QED's running coupling $\alpha^{-1}(\mu)$ as a function of any natural $t \leftrightarrow \mu$ map.

What it *is*:

1. At $t = 1$, $x_+(1) = \alpha^{-1}(m_e)$ to 9-digit precision (after the $\lambda_N + A_N$ corrections) — the dual prediction of FTD's master quadratic.
2. At $t \neq 1$, $x_+(t)$ is a mathematical perturbation around the base point. The slope under Map A is in the right *direction* (decreasing $\alpha^{-1}$ with increasing $\mu$) and roughly QED's order of magnitude near $t = 1$, but accelerates much faster than QED at smaller $t$.
3. The structural Landau-like point at $t_* \approx 0.062$ corresponds to $\alpha = N_c$ in the master quadratic, not to QED's Landau pole.

So the framework gives a value at $t = 1$ but does not naturally give *running*. This is consistent with [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) §3.2's [SELECTION] tag and with §7's listing of "physical interpretation of $t$" as out-of-scope.

**The closability question:** to give physical meaning to $x_+(t)$ at $t \neq 1$, one of the following must happen:

- **(C1)** A first-principles derivation of the FQCR-to-QED-running correspondence, with explicit fermion-loop content baked into the operator stack, such that $x_+(t)$ at small $t$ matches one-loop QED with the correct $b_0$ coefficient. This requires extending the operator stack beyond the current four layers.
- **(C2)** A reinterpretation of $t$ as something other than an RG scale parameter — e.g., a topological / loop-order index, a worldsheet modulus, or a holographic radial coordinate — under which $x_+(t)$ predicts something measurable that isn't running $\alpha$.
- **(C3)** Acceptance that the FQCR Model V identification is fixed at $t = 1$ only, with $t \neq 1$ being mathematically meaningful but physically inert. In this reading, SPEC_FQCR §6 Test 3 (running behaviour) is permanently OPEN as long as FQCR Model V is the only operator stack.

**My assessment:** (C3) is the honest current state. (C1) is the path that would actually upgrade the FTD-0013 [SMC] tag, but requires substantial new operator content. (C2) is interesting but speculative.

---

## §6 — What the slope match near $t = 1$ might mean

One genuinely curious feature: at $\mu \approx 1$ MeV (slightly above $m_e$), Map A gives FQCR slope $-0.541$ versus QED 3-lepton slope $-0.637$. The ratio is $0.85$ — within 15% of full-three-lepton QED.

This is provocative because at $\mu = 1$ MeV, only the electron is kinematically active in QED (muon at 105.7 MeV, tau at 1.78 GeV). But the FQCR slope behaves as if all three leptons are running — *as if* the FQCR-EM branch internally encodes all-lepton vacuum polarization at low energies.

This is a *single data point*, not a trend, and the FQCR slope diverges sharply from QED at smaller $t$. So the apparent match is more likely numerical coincidence than structural agreement. But it is the kind of pattern that, if it survived a more rigorous test (multiple $\mu$ values across lepton thresholds), would suggest path (C1) is achievable.

**Concrete test (open work):** compute $dx_+/d(\log\mu)$ under Map A at $\mu$ above the muon and tau thresholds. If the slope-magnitude pattern matches QED's full-lepton running, that's a structural hint. If it doesn't, the apparent match at 1 MeV is coincidence.

This script does not run that test (because under Map A, $t < 1$ is bounded and $\mu = m_\tau$ requires $t \approx m_e/m_\tau \approx 0.0003$, well below the Landau-like point at $t_* = 0.062$ — i.e., **the muon and tau thresholds are inaccessible under Map A**, which is itself a structural mismatch with QED).

---

## §7 — Status

| Item | Statement | Tag |
|---|---|---|
| TSM-1 | Three candidate maps proposed; heat-kernel reading (Map A) is the most structurally natural | [SELECTION] (proposal) |
| TSM-2 | Map A: $x_+(t=1) = \alpha^{-1}(m_e)$ matches CODATA by construction | [THEOREM] (numerical, $N \to \infty$) |
| TSM-3 | Under Map A, FQCR slope at $\mu \approx 1$ MeV is $\approx 2.5\times$ QED electron-only slope | [NUMERICAL FACT] |
| TSM-4 | Map A's range can't reach muon or tau thresholds; FQCR Landau-like point at $\mu \sim 10\,m_e$ | [STRUCTURAL OBSERVATION] |
| TSM-5 | None of the three candidate maps puts the FQCR Landau-like point at QED's Landau pole | [STRUCTURAL OBSERVATION] |
| TSM-6 | The FQCR-EM branch readout $x_+(t)$ is not a faithful representation of QED running under any candidate map | [TENTATIVE — pending §6 follow-up test] |
| TSM-7 | The $t$-scale map question remains [OPEN] under the current operator stack | [OPEN] |

---

## §8 — Cross-references

- [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) §3.2, §6 Test 3, §7 — the canonical [SELECTION] tag and the explicit OPEN status of running behaviour.
- [`EXPLR_FQCR_RESPONSE_LAW_TEST.md`](EXPLR_FQCR_RESPONSE_LAW_TEST.md) — the Q1 response-law test that reached a similar honest-OPEN conclusion.
- [`SPEC_DIMENSIONAL_MAP.md`](../01_reference/SPEC_DIMENSIONAL_MAP.md) — calibration declarations ($a_\mathrm{phys} \equiv \ell_P$, $K_B = m_e$) used in Maps A and C.
- [`AUDIT_DUAL_SUBSTRATE_PROVENANCE.md`](../07_assessment/AUDIT_DUAL_SUBSTRATE_PROVENANCE.md) — the same epistemic pattern: structural feature consistent with data without being forced.
- [`scripts/exploration/explore_fqcr_t_scale_map.py`](../../../scripts/exploration/explore_fqcr_t_scale_map.py) — the script that produced the numbers.
