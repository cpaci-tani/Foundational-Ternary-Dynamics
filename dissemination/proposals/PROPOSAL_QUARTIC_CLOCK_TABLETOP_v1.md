# Experimental Proposal — The Clock at the Edge of Stability: a Tabletop Test of the Lemniscatic Period Law

**Status:** [DRAFT PROPOSAL — dissemination artifact; registers NO FTD claim; moves no tag]
**Version:** v1 · 2026-08-05 · *(restored 2026-08-05 after an untracked-file wipe; content identical to the post-§12 revision)*
**Companion:** interactive lab `quartic_whirlpool_webapp` (clock panel demonstrates the analysis chain end-to-end on synthetic data)
**Scope guard:** everything below is classical mechanics and classical special functions. G* = Γ(¼)/Γ(¾) enters as the period constant of the pure quartic oscillator — a theorem, not a hypothesis. What is being tested is an apparatus's ability to realize the pure quartic regime and recover G* calibration-free. No fine-structure-constant claim, no substrate claim, appears anywhere in this document.

---

## 1 · Abstract

A mass held between two springs at exactly their natural length is a *pure quartic oscillator*: transverse displacement stretches the springs only at second order, so the potential is V = λx⁴ with the x → −x symmetry exact by construction and no cubic term possible. This is the generic normal form of any symmetric system at its instability threshold — the same potential a buckling column has at critical load. Its clock law is the inverse of Galileo's: the period is **amplitude-dependent**, T = √π·G\*·√(m/2λ)/A, with G\* = Γ(¼)/Γ(¾) = 2.958675119188639… playing the role π plays for the harmonic clock. We propose a ~$500 tabletop experiment that (i) traverses the full stability edge with one micrometer knob (taut → critical → slack = harmonic → quartic → double-well), (ii) recovers G\* from ringdown timing with no fitted scale, and (iii) tests two further parameter-free signatures of quartic timekeeping. A single ringdown sweeps the whole T(A) curve, because the frequency tracks its own decaying amplitude — the measurement is almost embarrassingly efficient.

## 2 · Physical background (all theorem-grade)

**2.1 The potential.** Two identical springs (stiffness k, natural length L₀) are anchored a distance 2L apart with the oscillating mass m between them. For transverse displacement x, each spring's length is √(L² + x²), extension e = √(L² + x²) − L₀.

- **Critical tune (L = L₀):** e ≈ x²/2L − x⁴/8L³, so V = k·e² (both springs) = (k/4L²)·x⁴ + O(x⁶). Pure quartic, **λ = k/4L²**; restoring force F = −(k/L²)x³.
- **Pretension (L > L₀, δ ≡ L − L₀):** adds ½μx² with **μ = 2kδ/L** — harmonic contamination, tunable to zero.
- **Slack (L < L₀):** μ < 0 — symmetric double well. The micrometer that sets δ walks the apparatus through a textbook pitchfork; the quartic clock lives exactly at δ = 0.

**2.2 The period law (fit model — verified).** For V = ½μx² + λx⁴ with turning point A:

> **T(A) = 4·√(m/2) · K(κ) / √(μ/2 + 2λA²),  κ² = λA² / (μ/2 + 2λA²)**

where K is the complete elliptic integral of the first kind. Limits (both checked analytically and numerically):
- μ = 0: κ² = ½, K(1/√2) = Γ(¼)²/4√π, giving **T = √π·G\*·√(m/2λ)/A** exactly.
- λ = 0: κ = 0, K(0) = π/2, giving T = 2π√(m/μ) exactly.

The three-regime structure is the experiment's backbone: small A is dominated by residual μ (period saturates), large A by the quartic (T ∝ 1/A), and the crossover amplitude measures the residual detuning *from the timing data itself*.

**2.3 The three parameter-free signatures.**
- **S1 — the clock law:** on the quartic branch, T·A is constant and G\*ₑₓₚ = T·A·√(2λ/πm) requires no fitted scale once λ and m are independently calibrated.
- **S2 — frequency–amplitude linearity:** f ∝ A across the ringdown; operationally, log f vs log A has slope +1 (the anti-pendulum signature; a harmonic clock gives slope 0).
- **S3 — waveform shape:** the pure-quartic waveform is the lemniscatic sine, not a sinusoid. Its shape functional ℬ₄ = [V̄(π) − V̄(0)] / V̄″(0) = 48π/G\*⁴ = 1.9678953151… (autocorrelation barrier-to-curvature ratio) is dimensionless and calibration-free. A sinusoid gives 2 exactly; the 1.6% depression below 2 is the fingerprint. (S3 is the most demanding target; it is a secondary gate.)

## 3 · Hypotheses and quantitative gates

**H1 (primary — the lemniscatic clock law).** At the critical tune, over the pre-declared amplitude window (§6), the fitted exponent s in T ∝ A⁻ˢ satisfies s ∈ [0.97, 1.03], and the recovered constant satisfies |G\*ₑₓₚ/G\* − 1| ≤ 2%.
**H0₁ (null):** the apparatus cannot be tuned out of harmonic saturation (period plateaus over the full window), or the recovered constant misses by > 5%.

**H2 (the edge traversal).** With deliberate detunings δ = {+2, +1, 0, −1, −2} × δ_step, the full T(A; μ) surface fits the §2.2 model with a single (λ, m) pair shared across all detunings, and the fitted μ is linear in the micrometer setting with intercept consistent with the independently measured suspension term (§7). Gate: shared-fit reduced χ² ≤ 2; μ(δ) linearity R² ≥ 0.99.
**H0₂:** μ(δ) is not monotone (mechanical hysteresis/imperfection dominates) — the apparatus cannot claim to sit at an edge.

**H3 (secondary — waveform).** ℬ₄ measured from the highest-quality ringdown segments lies in [1.90, 2.00] and is statistically below 2 (the sinusoid value) at 95% confidence, moving toward 1.968 as amplitude grows into the quartic branch.
**H0₃:** ℬ₄ indistinguishable from 2 everywhere (waveform never departs sinusoid → quartic regime never reached, corroborating a failed H1, or S/N insufficient).

**Kill conditions (any one voids the run, not the hypothesis):** ringdown Q < 30 in the measurement window; tracking noise floor > 2% of the smallest window amplitude; static cubic-law calibration (§7.1) departs from F ∝ x³ by > 5% RMS inside the window (springs non-Hookean at these strains — rebuild with different springs).

## 4 · Apparatus (bill of materials, ≈ $400–600)

| Item | Spec | Purpose |
|---|---|---|
| 2 × extension springs | k ≈ 300–1000 N/m each, L₀ ≈ 10–15 cm, matched pair | the quartic potential |
| Rigid frame + 2 anchor stages | one stage on a micrometer slide (≥ 10 µm resolution, ≥ 10 mm travel) | sets 2L; the δ knob |
| Oscillating mass | 100–300 g, low-profile, with retroreflective marker | m |
| Bifilar suspension | 2 lines, length ℓ ≥ 1.5 m, from rigid overhead point | constrains motion to one transverse axis; gravity term g/ℓ enters the error budget (§7.3) |
| High-fps camera | ≥ 240 fps phone camera acceptable; machine-vision camera preferred | position tracking (laser displacement sensor is a drop-in upgrade) |
| Force gauge + linear stage | 0.01 N resolution | static calibration of λ |
| Release solenoid or burn-wire | repeatable zero-velocity release at set amplitude | initial conditions |
| Scale, calipers, thermometer | — | m, geometry, drift monitoring |

**Design point (worked example):** k = 500 N/m per spring, L = 0.12 m, m = 0.2 kg → λ = k/4L² ≈ 8.7 kN/m³, T·A = √π·G\*·√(m/2λ) ≈ 0.0178 s·m. Amplitude window A ∈ [1.0, 2.4] cm (§6) gives T ∈ [0.74, 1.78] s in the μ = 0 idealization (with the suspension term, 0.72–1.42 s) and peak spring strain ≤ 2% — comfortable for Hookean behavior and for 240 fps tracking.

## 5 · Setup steps

1. **Frame and anchors.** Mount both anchor stages on the rigid frame; verify frame resonances are > 20× the highest clock frequency (tap test with the camera).
2. **Suspension.** Hang the mass bifilar from the overhead point; adjust until free-swing (springs detached) shows a single clean pendulum mode along the intended axis. Record its period T_g to ±0.5% — this measures the gravity term g/ℓ that enters §7.3.
3. **Attach springs** at equal heights; verify the mass sits at the geometric midpoint (caliper both sides).
4. **Coarse zero-pretension:** back the micrometer off until the springs just visibly bow (slack), then advance until straight. This is δ ≈ 0 to within ~0.2 mm.
5. **Fine zero via physics (the important step):** release the mass at the smallest window amplitude and inspect the tail of the ringdown. Residual μ > 0 shows as period saturation; μ < 0 shows as the mass hanging off-center (double-well). Bisect with the micrometer until the smallest-amplitude cycles show no saturation and the rest point is centered. Record the micrometer reading as δ₀; all detunings in H2 are stated relative to it.
6. **Tracking calibration:** image a ruler in the plane of motion; pixel-to-mm map at ≥ 3 positions (lens distortion check).
7. **Environment:** log temperature at start/end of each session (spring k drifts ~ −0.03%/°C for steel; matters only at the 0.1% level).

## 6 · Measurement protocol

**Pre-declared amplitude window:** A ∈ [A_min, A_max] with A_max = 0.2·L (keeps the x⁶ geometric correction ≤ 1.2% in period, §7.4) and A_min = max(3 mm, the amplitude where residual-μ correction is model-dominated rather than data-dominated). For the design point: [1.0, 2.4] cm. The window is fixed **before** the H1 analysis; data outside it are used only for the H2 crossover fit, where the model includes μ explicitly.

**Runs:**
- **R1 (primary), 10 ringdowns:** release at A₀ ≈ 1.1·A_max, record until amplitude falls below A_min/2. Each ringdown yields the entire T(A) curve: per-cycle period from successive same-direction zero crossings (sub-frame interpolation), per-cycle amplitude as the geometric mean of the bracketing peaks (first-order decay-bias cancellation).
- **R2 (release-amplitude cross-check), 5 × 3 runs:** fixed releases at three amplitudes spanning the window; verifies the ringdown-swept curve against fresh-release periods (guards against slow-drift systematics within a ringdown).
- **R3 (edge traversal for H2):** repeat one R1-style ringdown at each of the five detunings.
- **R4 (waveform for H3):** the three highest-S/N ringdowns re-processed at full resolution for the ℬ₄ functional over sliding 5-cycle segments.

## 7 · Calibration chain (everything G\*ₑₓₚ depends on)

1. **λ, statically and independently of k and L:** clamp the frame, displace the mass transversely with the force gauge on the linear stage, record F(x) at ≥ 12 points spanning the window, both signs. Fit F = −c₃x³ (+ c₁x nuisance term to absorb residual μ); λ = c₃/4. Gate: |c₁|·A_max ≤ 0.05·c₃·A_max³, else re-zero and repeat. Target: σ_λ/λ ≤ 1%.
2. **m:** weigh the mass (±0.1 g). Spring transverse inertia adds ~⅓ of the moving spring mass per side at these geometries; with light springs (< 5% of m) this is a ≤ 1.5% correction — apply it and carry ±½ of it as systematic. Cross-check dynamically: at a large deliberate pretension where the system is harmonic, ω₀² = μ/m_eff with μ = 2kδ/L known from the axial k measurement — agreement within 2% validates m_eff.
3. **Suspension gravity term (SIZED BY SIMULATION — large, and handled):** the bifilar pendulum adds μ_g = m·g/ℓ (1.31 N/m at the design point), and this is **not** a percent-level nuisance: the pre-build simulation (§12) shows it shifts the period by up to ~25% at A_min and drags the naive log-slope to s ≈ 0.81 with G\* biased −8%. ⚠ *v1 of this section under-sized this term ("1–3%"); corrected 2026-08-05 after simulation.* The declared analysis was already built for it and survives: the §2.2 model includes μ, pinned by the independent T_g measurement, and the corrected recovery passes the H1 gates with margin even when μ is deliberately mis-pinned by ±5% (§12, S3). The naive uncorrected analysis is not a valid fallback and is retained in the analysis code only as a demonstration.
4. **Geometric x⁶ term (measured in simulation):** V's next term is −k·x⁶/8L⁴; the full-geometry period exceeds the pure-quartic law by +0.30% at A = 0.1L and **+1.17% at A = 0.2L** (§12, S2) — book 1.2% at the window edge. Two treatments, both run: (a) restrict to the window and book it as systematic; (b) include the analytic x⁶ term in a numerical-period fit. Agreement between (a) and (b) is itself a consistency check. The residual +0.28% bias on recovered G\* in the end-to-end simulation is consistent with this term and inside the gate.
5. **Damping:** measure Q from the amplitude envelope. Period bias from damping is O(1/Q²) (< 10⁻⁴ for Q > 50) — negligible; the per-cycle amplitude assignment bias is first-order cancelled by the geometric-mean rule (§6 R1) and the residual is booked from simulation.

**Error budget target (G\*ₑₓₚ):** λ 1.0%, m_eff 0.8%, timing < 0.1%, amplitude calibration 0.5%, residual-μ model 0.5%, x⁶ 0.5% → combined ≈ 1.1% through the square roots in the recovery formula — inside the 2% H1 gate with margin.

## 8 · Analysis plan (locked before data)

1. Extract (Tᵢ, Aᵢ) pairs per cycle per ringdown (code frozen and hash-recorded before R1; the webapp clock panel's peak-refinement algorithm is the reference implementation).
2. **H1:** fit log T vs log A over the window → s; compute G\*ₑₓₚ per cycle, take the run-median, bootstrap across runs for the confidence interval.
3. **H2:** global fit of all R3 data to the §2.2 elliptic model with shared (λ, m_eff), per-detuning μ; test μ(δ) linearity and intercept vs the §7.3 suspension term.
4. **H3:** ℬ₄ per segment; trend vs segment amplitude; compare against the sinusoid null (2) and the lemniscatic value (1.9679). **Mandatory extraction rule (simulation-derived):** each cycle must be mapped onto its own [0, 2π) phase interval before averaging — resampling several cycles against one fixed period is biased by the clock's own chirp (frequency tracks decaying amplitude) and spuriously pushes ℬ *above* 2 by up to +5% (§12, S5). With per-cycle mapping the extractor is unbiased at the 3×10⁻⁵ level.
5. All gates evaluated exactly as written in §3; no post-hoc tolerance adjustment (the tolerances above are declared now, before any apparatus exists — that is the point of this section).

## 9 · Outcome classes

- **A — CONFIRMED:** H1 and H2 pass. The apparatus is a working lemniscatic clock; G\* recovered calibration-free at ≤ 2%. Publishable as a pedagogy/perspective piece with the webapp companion regardless of H3.
- **B — QUARTIC BUT MISCALIBRATED:** s ∈ [0.97, 1.03] but G\*ₑₓₚ misses at 2–5%. The clock law holds; the calibration chain has a bug — iterate §7, do not touch §3.
- **C — EDGE UNREACHABLE:** H2 fails its monotonicity gate. Mechanical imperfection dominates; document and redesign (this outcome is informative about *real* edges: imperfection sensitivity is the classic obstruction to sitting at a critical point, and quantifying it is honest content).
- **D — CLOCK LAW VIOLATED:** tuning verified (H2 passes) but s outside [0.9, 1.1] on the quartic branch. Since T(A) here is a theorem given V, outcome D means V is not what §2.1 says (springs non-Hookean, hidden compliance) — trace it; the theorem is not in question, the apparatus model is.

## 10 · Extensions (out of scope for v1, listed for the record)

- **MEMS/NEMS buckled beam at critical load** — same physics, f in kHz–MHz, Q in the thousands; S3 becomes easy.
- **cQED quartic point** — Josephson potential with cancelled quadratic term; the *quantum* signatures (E_n level ratios, E ∝ I^(4/3)) become accessible.
- **Cold-atom quartic trap** — the historical vortex connection: quartic(+quadratic) traps were built to hold fast-rotating condensates; a BEC breathing mode in a pure-quartic trap would be the many-body version of this clock.

## 11 · Relation to FTD (one paragraph, for internal readers only; delete from any submitted version)

This proposal registers nothing in the LEDGER and asserts no FTD claim. Internally, the δ-knob traversal is a physical realization of the Maxwell-trichotomy bracket: μ > 0 is the n = 2 well, μ < 0 the flat/multi-well side, and the measure-zero point between them is the n = 4 clock — reached here *by tuning*. The open FTD question C3 is whether any substrate configuration sits at that point *without a tuner*; this experiment neither supports nor undermines that, but it makes the "edge" language operational, and outcome C would quantify exactly how hard sitting on the edge is for a real physical system — which is honest context for why C3 keeps closing negative.

## 12 · Pre-build validation (simulated, 2026-08-05)

Full synthetic end-to-end validation ran before any hardware: truth = **full two-spring geometry** (no quartic truncation) + suspension gravity term + linear damping; analysis = exactly §8 as declared. Script: [`sim_quartic_clock_validation_v1.py`](sim_quartic_clock_validation_v1.py) (this folder).

| Sim | Result | Verdict |
|---|---|---|
| S1 — fit model vs quadrature | §2.2 elliptic formula exact to 6×10⁻¹⁴ over (μ, A) grid incl. μ < 0 | PASS |
| S2 — truncation bias | +0.30% at A = 0.1L, **+1.17% at A = 0.2L**, +2.61% at 0.3L | window edge confirmed; book 1.2% |
| S3 — end-to-end H1 | 122-cycle ringdown, 80 in window, Q ≈ 300. Naive: s = 0.805, G\* −8.1% (**fails; gravity term dominates small-A**). Declared μ-corrected recovery: s = 0.988, G\* **+0.28%**; robust under μ mis-pinned ±5% (+0.70%/−0.12%) | **H1 gates PASS** |
| S4 — edge traversal | shared-λ global fit over 5 detunings: λ −1.4% (x⁶ absorption), μ(δ) slope −0.44% vs 2k/L, intercept recovers μ_g at +3.2%, linearity R² = 0.999994 | **H2 gates PASS** |
| S5 — waveform ℬ₄ | Exact controls: ℬ₂ = 2.0000141, ℬ₄ = 1.9679114 (both ~10⁻⁵). Ringdown with per-cycle phase mapping: ℬ = 1.9735 → 1.9941, rising smoothly from near the quartic value toward the harmonic 2 as amplitude decays — below 2 throughout, trending exactly as §3 H3 requires. Fixed-period resampling shown biased (+2–5%, above 2) and prohibited. | **H3 achievable; extractor rule added to §8.4** |

Two corrections were fed back into this document by the simulation: the §7.3 suspension-term sizing (v1 text under-sized it by an order of magnitude — the *procedure* was already correct, the prose was not) and the §8.4 per-cycle extraction rule. The simulated apparatus at the §4 design point passes every declared H1/H2 gate and reaches the H3 signature with margin.

---

*Prepared 2026-08-05. Fit model (§2.2) verified analytically in both limits this date; ℬ₄ and period constants cross-checked numerically. §12 validation added same date; §7.3 sizing corrected from simulation. G\* digit string corrected 2026-08-05 (independent audit): 2.958675119188639…, verified at 25 dps.*
