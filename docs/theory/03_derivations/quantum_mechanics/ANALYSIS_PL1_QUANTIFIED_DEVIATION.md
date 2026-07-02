# ANALYSIS — PL-1 Quantified: The Hermite Deviation Tower of Rice Statistics vs Born

**Tag:** `[DERIVED — conditional on the [IMPOSED] Langevin model]` — every result below is a theorem *of the model* defined in [`DERIV_BORN_PROPORTIONALITY.md`](../DERIV_BORN_PROPORTIONALITY.md) §3, not of the FTD substrate. Zero promotions: FTD-0187 T1c stays `[OPEN]`; FTD-0200 stays `[CLOSED NEGATIVE for Born]` (6-neighbour); the PL-1 row of [`SPEC_PREDICTION_LEDGER_DEVIATIONS.md`](../../01_reference/SPEC_PREDICTION_LEDGER_DEVIATIONS.md) keeps its recorded tags.
**Id:** FTD-0359 (this document mints no LEDGER row; owner to reconcile ids at next LEDGER pass).
**Verification:** [`scripts/proofs/proof_pl1_deviation.py`](../../../../scripts/proofs/proof_pl1_deviation.py) — **35/35 checks pass** (symbolic identities via sympy, numeric closed forms, Monte-Carlo level-crossing counts on a synthesized Gaussian process, all table values). Every number in this document is script-verified; none is recalled or estimated.
**Method note (anti-target compliance):** forward derivation only. No fitting to any known experiment was performed; no experimental dataset was consulted to tune any coefficient. The deviation formulas are consequences of the already-recorded Rice law (FTD-0200) pushed to higher order.

---

## 0. Conditionality register (load-bearing, read first)

The claims below hold **inside** the following stack, and their epistemic weight is capped by its weakest member:

1. **`[IMPOSED]` Langevin ensemble.** A Gaussian white-noise field xi with stationary local variance sigma_n^2 is added by hand (DERIV_BORN_PROPORTIONALITY.md §3). The deterministic substrate (Postulate P5) nowhere supplies it — this is the FTD-0356 banner's count 1 and it caps everything here at "theorem of the model".
2. **Scalar-threshold construction.** Manifestation is modeled as the scalar flux J crossing +K_B upward or −K_B downward (the FTD-0200 protocol's construction). The canonical engine thresholds the vector norm |**J**| ≥ K_B; the vector-norm (Rician/chi) variant of everything below is `[OPEN]` (§7).
3. **Sign condition sigma_n < K_B** (beta > 1). Otherwise the model is anti-Born from the outset (§2, T4) — the FTD-0356 banner's count 3, now made exact.
4. **Background subtraction.** Born-like behavior refers to the *excess* rate over the constant nu_0 (dark-count convention) — banner count 4. The offset nu_0 is itself part of the falsifiable structure (§3).
5. **Manifestation-to-detection mapping** at its LEDGER tag (`[CONJECTURE]`-grade). The physical readings in §6 ride on it.
6. **The gate is the pre-registered engine test.** Per FTD-0200, only a pre-registered v2 run on the canonical 26-neighbour engine can close the substrate-level question. This document sharpens *what that test should measure and at what precision*; it does not substitute for it.

---

## 1. Setup and exact rate law

Model (from the demoted doc's §3–§4, whose mathematics the FTD-0356 review verified as sound): total flux J = J_coh + δJ with δJ a stationary, differentiable, zero-mean Gaussian process of variance σ_n²; manifestation events are upcrossings of +K_B plus downcrossings of −K_B. Rice's theorem (Rice 1944, external `[THEOREM]`; Monte-Carlo re-validated to < 1 % in the proof script, checks MC-1/MC-2) gives, for mean level m and threshold u,

$$\nu^+(u) \;=\; R\, e^{-(u-m)^2/2\sigma_n^2}, \qquad R \equiv \frac{\sigma_{\dot J}}{2\pi\,\sigma_n}.$$

Summing the two symmetric channels with m = J_coh (script check T1, numeric + symbolic):

$$\boxed{\;\frac{\nu_{\rm tot}(J_{\rm coh})}{\nu_0} \;=\; e^{-x^2/2}\,\cosh(\beta x)\;}\qquad
x \equiv \frac{|J_{\rm coh}|}{\sigma_n},\quad \beta \equiv \frac{K_B}{\sigma_n},\quad
\nu_0 \equiv 2R\,e^{-\beta^2/2}. \tag{1.1}$$

This is exact within the model — no expansion. It is the closed form the FTD-0356 banner quotes, and eq. (4.2) of the demoted doc is its single-channel factor.

---

## 2. The deviation tower `[THEOREM — of the imposed model]`

**Theorem (Hermite tower).** With He_n the probabilists' Hermite polynomials (He₂(t) = t²−1, He₄(t) = t⁴−6t²+3, He₆(t) = t⁶−15t⁴+45t²−15, …),

$$\frac{\nu_{\rm tot}}{\nu_0} \;=\; \sum_{m=0}^{\infty} \frac{{\rm He}_{2m}(\beta)}{(2m)!}\; x^{2m}
\;=\; \sum_{m=0}^{\infty} \frac{{\rm He}_{2m}(\beta)}{(2m)!}\left(\frac{I}{\sigma_n^2}\right)^{m},
\qquad I \equiv |J_{\rm coh}|^2. \tag{2.1}$$

*Proof.* The generating function of the probabilists' Hermite polynomials is $\sum_n {\rm He}_n(\beta)\,x^n/n! = e^{\beta x - x^2/2}$. Then $e^{-x^2/2}\cosh(\beta x) = \tfrac12\big(e^{\beta x - x^2/2} + e^{-\beta x - x^2/2}\big)$, and the odd terms cancel pairwise while the even terms double. QED. (Script: symbolic through x¹⁰, check T2; numeric to 10⁻¹⁰, T2; odd-order cancellation exactly, T3 — this is the ±K_B symmetry cancellation of the demoted doc's §4.1, extended to all odd orders.)

**Dimensional form** (background-subtracted excess rate; first two terms):

$$\Delta\nu(I) \;=\; \nu_0\left[\;
\underbrace{\frac{K_B^2-\sigma_n^2}{2\sigma_n^4}\,I}_{\text{Born-like (leading)}}
\;+\;
\underbrace{\frac{K_B^4 - 6K_B^2\sigma_n^2 + 3\sigma_n^4}{24\,\sigma_n^8}\,I^2}_{\text{first deviation}}
\;+\;\mathcal{O}(I^3)\right]. \tag{2.2}$$

The leading coefficient reproduces the demoted doc's eq. (4.10) exactly. The structural reading: **the model is "Born plus a Hermite tower"** — the Born term is He₂(β)/2! and every higher even Hermite polynomial evaluated at the threshold-to-noise ratio β is a deviation fingerprint. Standard QM's Born rule is the statement that the tower above m = 1 is identically zero. That contrast is the quantitative content of PL-1.

Corollaries (all script-verified):

- **Sign condition (T4).** The Born-like term has positive sign iff He₂(β) > 0, i.e. **β > 1** (σ_n < K_B) — the FTD-0356 banner's condition, now located exactly. For β ≤ 1 the exact rate is monotone *decreasing* in intensity: anti-Born everywhere, no Born-mimicking regime at all.
- **First-deviation sign flips at β\* = √(3+√6) ≈ 2.334414 (T5).** For 1 < β < β\* the a₄ coefficient is negative (rate approaches saturation already at second order); for β > β\* it is positive (super-linear onset before eventual saturation). Both roots of He₄ are β² = 3 ± √6; only 3+√6 lies in the Born-sign region β > 1.
- **Born-mimicking point (T5).** At exactly β = β\* the quartic deviation vanishes and the model imitates Born through O(I); the leading deviation is then sixth-order with a₆(β\*) = He₆(β\*)/720 = **−0.074158** < 0. The model can hide from a second-order nonlinearity test only at this single β, and is then exposed at third order in intensity with a definite (negative) sign.
- **Validity window.** The truncation (2.2) requires x ≪ 1 *and* βx ≪ 1, i.e. |J_coh| ≪ K_B/β² for β > 1 (script T6: the leading deviation law is accurate to 5 % for x ≤ 0.05). All statements outside this window below use the exact form (1.1).

---

## 3. The constant offset: ν₀ is part of the identity

The raw (unsubtracted) rate at zero coherent drive is ν₀ = 2R e^{−β²/2} — an **irreducible dark rate** with a specific threshold dependence:

$$\ln \nu_0 \;=\; \ln(2R) \;-\; \frac{\beta^2}{2} \;=\; \ln(2R) - \frac{K_B^2}{2\sigma_n^2}. \tag{3.1}$$

Two falsifiable consequences within the model: (i) the dark rate is Gaussian in the threshold (log-rate quadratic in K_B at fixed noise) — a one-parameter family any threshold-scan can test; (ii) the *same* σ_n appearing in (3.1) fixes every coefficient in (2.1) — the dark-rate scan and the intensity-response nonlinearity must agree on β, or the model is dead. Ideal Born detection has no intrinsic dark rate and no such consistency constraint.

---

## 4. Where Born and Rice diverge, observable by observable

Throughout, define the **deviation parameter**

$$\varepsilon(\beta, I) \;\equiv\; \frac{a_4}{a_2}\,\frac{I}{\sigma_n^2}
\;=\; \frac{{\rm He}_4(\beta)}{12\,{\rm He}_2(\beta)}\,\frac{I}{\sigma_n^2}
\;=\; \frac{\beta^4-6\beta^2+3}{12(\beta^2-1)}\,\frac{I}{\sigma_n^2}, \tag{4.1}$$

the leading fractional deviation of the excess rate from Born linearity. All four projections below are `[DERIVED — conditional]` and script-verified against the exact rate (1.1), not just against the quadratic truncation.

### 4.1 Rate-vs-intensity nonlinearity (the direct test)

$$D(I) \;\equiv\; \frac{\Delta\nu(I)}{\nu_0\, a_2\, I/\sigma_n^2} - 1 \;=\; \varepsilon(\beta,I) + \mathcal{O}(I^2).$$

Born: D = 0 identically at every intensity. Rice: D grows linearly in I with the sign of a₄. The worked table (§5) gives the exact intensity at which |D| reaches 1 % and 10 % for each β.

### 4.2 Balanced two-beam fringe: second-harmonic distortion (T7a)

For an interference pattern Ĩ(φ) = Ĩ₀(1+cos φ)/2 (full contrast), the background-subtracted count pattern acquires a **cos 2φ component that Born forbids**:

$$\frac{c_2}{c_1} \;=\; \frac{\varepsilon_{\rm peak}}{4\,(1+\varepsilon_{\rm peak})} \;\approx\; \frac{\varepsilon_{\rm peak}}{4}, \qquad \varepsilon_{\rm peak} = \frac{a_4}{a_2}\frac{I_0}{\sigma_n^2}.$$

Script: exact-rate Fourier analysis matches this to ≤ 2 % of the effect at Ĩ₀ = 0.02 (e.g. β = 3: c₂/c₁ = +1.548e-3 exact vs +1.553e-3 predicted); the Born reference response has c₂ = 0 to machine precision. Fringe *visibility* on a balanced fringe is insensitive (V stays 1 because the minima are dark) — the harmonic content, not the visibility, is the balanced-fringe discriminator.

### 4.3 Unbalanced fringe: visibility shift (T7b)

For Ĩ(φ) = Ī(1+V cos φ) with true visibility V < 1:

$$V_{\rm meas} \;=\; V\,\frac{1+2e}{1+e(1+V^2)}, \qquad e = \frac{a_4}{a_2}\frac{\bar I}{\sigma_n^2}
\qquad\Longrightarrow\qquad \frac{\Delta V}{V} \;\approx\; e\,(1-V^2).$$

Born: V_meas = V exactly. Rice: visibility shifts *down* for 1 < β < β\* and *up* for β > β\* (script: exact-rate check at V = 0.5, Ī = 0.02, e.g. β = 3: ΔV/V = +4.63e-3 exact vs +4.69e-3 predicted).

### 4.4 Source-statistics calibration split — the g² coupling (T7c)

Because the response is nonlinear in intensity, two sources with **equal mean intensity but different intensity statistics** yield different mean count rates:

$$\frac{\langle \Delta\nu\rangle_{\rm source}}{\langle \Delta\nu\rangle_{\rm coherent}} - 1 \;\approx\; \varepsilon(\beta, \bar I)\,\big(g^{(2)}(0)-1\big),$$

so a thermal source (g² = 2) is miscounted relative to a stable-intensity source by ε itself (script: exact integration over the exponential intensity distribution, β = 3, Ī = 0.01: split = +3.099e-3 vs predicted +3.125e-3). Born (linear response): mean rate depends on mean intensity only — the split is identically zero for *any* source statistics. This is a single-detector test; no coincidence circuit is needed.

### 4.5 Saturation and rollover — the strong-field signature (T8)

The exact envelope (1.1) is non-monotone for β > 1:

- **Peak at x\*** solving x = β tanh(βx), i.e. at coherent amplitude J\* = K_B·tanh(βx\*) — **strictly below threshold**, approaching it like 1 − 2e^{−2β²} (β = 1.5: J\*/K_B = 0.9755; β = 2: 0.99933). Peak height approaches ½e^{β²/2}·ν₀ for large β (β = 2: 3.696 ν₀; β = 3: 45.01 ν₀).
- **Exact landmark:** at J_coh = 2K_B the event rate equals $\nu_0\,(1+e^{-4\beta^2})/2$ — for any β ≥ 1.5, **half the dark rate** to better than 10⁻⁴ (symbolic + numeric, T8). A detector driven at twice threshold counts *less than in the dark*.
- Beyond, Gaussian collapse ∝ e^{−(x−β)²/2}: the rare-event tail is Gaussian in amplitude, not power-law in intensity.

Born: monotone linear growth at every intensity, forever. This is the qualitative divergence the PL-1 row already names ("saturation of the rate envelope"); the rows above make it exact.

---

## 5. Worked numeric table (all values script-verified; σ_n = K_B/β, K_B = 0.511)

| β = K_B/σ_n | a₂ | a₄ | a₄/a₂ | \|J_coh\|/K_B at 1 % dev. | at 10 % dev. | J\*/K_B (peak) | peak ν/ν₀ | ν/ν₀ at 2K_B |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1.2 | 0.2200 | −0.1486 | −0.6755 | 0.1015 | 0.3256 | 0.83391 | 1.098 | 0.5016 |
| 1.5 | 0.6250 | −0.2266 | −0.3625 | 0.1109 | 0.3569 | 0.97550 | 1.558 | 0.5001 |
| 2.0 | 1.5000 | −0.2083 | −0.1389 | 0.1338 | 0.4165 | 0.99933 | 3.696 | 0.5000 |
| β\* ≈ 2.3344 | 2.2247 | 0 | 0 | 0.3232 | 0.6043 | 0.99996 | 7.626 | 0.5000 |
| 3.0 | 4.0000 | +1.2500 | +0.3125 | 0.0597 | 0.1921 | 1.00000 | 45.01 | 0.5000 |
| 5.0 | 12.000 | +19.917 | +1.6597 | 0.0155 | 0.0484 | 1.00000 | 1.34e5 | 0.5000 |
| 10.0 | 49.500 | +391.79 | +7.9150 | 0.0035 | 0.0110 | 1.00000 | 2.59e21 | 0.5000 |

Reading of the table: **the quieter the noise (larger β), the earlier Born fails.** At β = 10 a coherent amplitude of just 0.35 % of threshold already produces a 1 % deviation from Born linearity; at β = 2 the 1 % point sits at 13.4 % of threshold. The deviation-reach columns are exact (bisection on the exact D, not the leading law). The β\* row shows the Born-mimicking window: the 1 % point is pushed out ~2.4x relative to its neighbours because the onset there is sixth-order. The J\*/K_B entries for β ≥ 3 print as 1.00000 because the analytic gap 2e^{−2β²} (< 3e-8) is below float64 resolution of the fixed point; the strict inequality J\* < K_B is analytic (tanh < 1).

---

## 6. The falsifiable statement

> **Conditional on the §0 register** (imposed Langevin ensemble with σ_n < K_B, scalar-threshold construction, manifestation = detection at its `[CONJECTURE]` tag):
>
> **If the substrate is right,** a threshold detector at noise ratio β = K_B/σ_n, driven at weak coherent intensity I, shows — with one shared parameter β fixing all five —
> 1. background-subtracted count rate deviating from Born linearity by **D = [He₄(β)/(12 He₂(β))]·(I/σ_n²)** (e.g. β = 2: −1 % at \|J_coh\| = 0.134 K_B, −10 % at 0.417 K_B);
> 2. a **cos 2φ fringe harmonic** c₂/c₁ = ε/4 on a balanced two-beam fringe;
> 3. a **visibility shift** ΔV/V = ε(1−V²) on an unbalanced fringe;
> 4. a **source-statistics split** ε(g²−1) between equal-mean-intensity sources;
> 5. a **rate envelope that peaks just below \|J_coh\| = K_B and falls to exactly (1+e^{−4β²})/2 of the dark rate at \|J_coh\| = 2K_B**, with an irreducible dark rate obeying ln ν₀ ∝ −K_B²/2σ_n².
>
> **Standard QM sees zero:** for ideal Born detection every one of 1–4 vanishes identically at every intensity, and 5's envelope is monotone linear with no intrinsic dark floor.

**Kill conditions (both directions).** *Against FTD/PL-1:* the pre-registered v2 canonical-engine run returning Born (D consistent with 0 across the weak-field window at the achieved statistics) kills the PL-1 deviation claim and revives the Born-emergence program — exactly the PL-1 row's falsification clause, now with the required measurement precision quantified by §5. *Against the model but not PL-1:* a v2 run showing a deviation tower with coefficients inconsistent with the He_{2m}(β) ladder (after fitting the single parameter β from the dark rate) kills the imposed-Langevin mechanism while leaving the raw Rice-vs-Born discrimination of FTD-0200 intact. *Internal consistency check:* β extracted from the dark-rate threshold scan (3.1) must equal β extracted from the nonlinearity (4.1); disagreement kills the model without any external comparison.

**Regime honesty.** The engine-native reading is testable now (the v2 pre-registration is the gate; the FTD-0200 protocol already bins event rates by local intensity — the v2 analysis should fit the full form (1.1) rather than the single-sided envelope). The physical reading additionally rides the manifestation-to-detection `[CONJECTURE]` and the calibration register; no claim is made here that laboratory photodetectors realize this mechanism or operate in this regime — the statement is what the substrate-plus-imposed-ensemble predicts *if* its manifestation statistics are detection statistics.

---

## 7. Open refinements

- **Vector-norm variant `[OPEN]`.** The canonical engine manifests on \|**J**\| ≥ K_B (3-component norm). The crossing theory of the norm of a vector Gaussian process (Rician/chi statistics) changes ν₀ and the coefficient ladder but preserves evenness in J_coh and a leading \|J_coh\|² excess; the corresponding tower has not been derived.
- **Noise-spectrum dependence.** R = σ_J̇/(2πσ_n) sets only the overall scale; every *normalized* result above is spectrum-independent within the stationary-Gaussian class. Non-Gaussian substrate structure (the FTD-0200 "conjecture for engine v2" list: BCC multiplicative eigenvalue, manifestation back-reaction, lattice arithmetic) would break the tower — which is precisely what would make the v2 test informative.
- **Back-reaction.** The model treats manifestation as a passive counter. The engine's manifestation feeds back on J (pair production, Gauss projection); whether the feedback preserves the Gaussian fixed point is the central open question FTD-0200 §3 already flags.

---

## 8. Verification record

`python scripts/proofs/proof_pl1_deviation.py` — **35/35 PASS** (2026-07-02): T1 exact/symbolic rate law; T2 Hermite tower (symbolic through x¹⁰ + numeric 1e-10); T3 odd-order cancellation; T4 sign condition; T5 β\* root structure; T6 deviation law accuracy; T7a/b/c observable projections against the exact rate; T8 saturation fixed point, peak asymptote, exact 2K_B landmark; MC direct level-crossing counts on a synthesized Gaussian process matching Rice to < 1 %; full §5 table regeneration with structural sanity checks.
