# Experimental Proposal — The Clock at the Edge of Stability: a Tabletop Test of the Lemniscatic Period Law

**Status:** [DRAFT PROPOSAL — dissemination artifact; registers NO FTD claim; moves no tag]
**Version:** v1 · 2026-08-05 · *(master copy in session scratchpad; repo copies were twice destroyed by a concurrently running agent session (since finished) — restore by plain file copy and COMMIT)*
**Companion:** interactive lab `quartic_whirlpool_webapp` + paper draft `dissemination/papers/edge_clock/`
**Scope guard:** everything below is classical mechanics and classical special functions. G* = Γ(¼)/Γ(¾) enters as the period constant of the pure quartic oscillator — a theorem, not a hypothesis. What is being tested is an apparatus's ability to realize the pure quartic regime and recover G* calibration-free. No fine-structure-constant claim, no substrate claim, appears anywhere in this document.

---

## 1 · Abstract

A mass held between two springs at exactly their natural length is a *pure quartic oscillator*: transverse displacement stretches the springs only at second order, so the potential is V = λx⁴ with the x → −x symmetry exact by construction and no cubic term possible. This is the generic normal form of any symmetric system at its instability threshold — the same potential a buckling column has at critical load. Its clock law is the inverse of Galileo's: the period is **amplitude-dependent**, T = √π·G\*·√(m/2λ)/A, with G\* = Γ(¼)/Γ(¾) = 2.958675119188639… playing the role π plays for the harmonic clock. We propose a ~$500 tabletop experiment that (i) traverses the full stability edge with one micrometer knob (taut → critical → slack = harmonic → quartic → double-well), (ii) recovers G\* from ringdown timing with no fitted scale, and (iii) tests two further parameter-free signatures of quartic timekeeping. A single ringdown sweeps the whole T(A) curve, because the frequency tracks its own decaying amplitude.

## 2 · Physical background (all theorem-grade)

**2.1 The potential.** Two identical springs (stiffness k, natural length L₀) anchored 2L apart, mass m between them. Transverse displacement x: spring length √(L² + x²), extension e = √(L² + x²) − L₀.

- **Critical tune (L = L₀):** e ≈ x²/2L − x⁴/8L³, so V = k·e² (both springs) = (k/4L²)·x⁴ + O(x⁶). Pure quartic, **λ = k/4L²**; F = −(k/L²)x³.
- **Pretension (δ ≡ L − L₀ > 0):** adds ½μx² with **μ = 2kδ/L**.
- **Slack (δ < 0):** μ < 0 — symmetric double well. The micrometer walks the apparatus through a pitchfork; the quartic clock lives exactly at δ = 0.

**2.2 The period law (fit model — verified).** For V = ½μx² + λx⁴, turning point A:

> **T(A) = 4·√(m/2) · K(κ) / √(μ/2 + 2λA²),  κ² = λA² / (μ/2 + 2λA²)**

Limits verified: μ = 0 → κ² = ½, K(1/√2) = Γ(¼)²/4√π → **T = √π·G\*·√(m/2λ)/A** exactly; λ = 0 → T = 2π√(m/μ). Equivalent universal form (paper Result 1): **T·A·√(2λ/m) = 4kK(k)** with k² = 2λA²/(μ+4λA²).

**2.3 The three parameter-free signatures.**
- **S1 — the clock law:** on the quartic branch, T·A constant; G\*ₑₓₚ = T·A·√(2λ/πm) with no fitted scale.
- **S2 — frequency–amplitude linearity:** log f vs log A slope +1 (harmonic: 0).
- **S3 — waveform shape:** ℬ = 2⟨x²⟩/⟨(dx/dθ)²⟩; sinusoid 2 exactly, quartic ℬ₄ = 48π/G\*⁴ = 1.9678953151…; along the crossover the exact curve is ℬ(k) = (3π²/2K²)·[E−(1−k²)K]/[(1−k²)K+(2k²−1)E].

## 3 · Hypotheses and quantitative gates

**H1 (primary).** At critical tune, over the declared window (§6): fitted exponent s ∈ [0.97, 1.03] in T ∝ A⁻ˢ, and |G\*ₑₓₚ/G\* − 1| ≤ 2%.
**H0₁:** period plateaus over the full window, or recovery misses by > 5%.

**H2 (edge traversal).** Detunings δ = {+2, +1, 0, −1, −2}·δ_step: full T(A; μ) surface fits the §2.2 model with shared (λ, m); fitted μ linear in the micrometer with intercept = independently measured suspension term. Gates: shared-fit reduced χ² ≤ 2; μ(δ) linearity R² ≥ 0.99.
**H0₂:** μ(δ) non-monotone (imperfection/hysteresis dominates).

**H3 (secondary).** ℬ₄ from best segments ∈ [1.90, 2.00], statistically below 2 at 95%, trending toward 1.968 with amplitude.
**H0₃:** ℬ indistinguishable from 2 everywhere.

**Kill conditions (void the run, not the hypothesis):** Q < 30 in window; tracking noise > 2% of A_min; static F ∝ x³ calibration departs > 5% RMS in window.

## 4 · Apparatus (≈ $400–600)

| Item | Spec | Purpose |
|---|---|---|
| 2 × extension springs | k ≈ 300–1000 N/m, L₀ ≈ 10–15 cm, matched | quartic potential |
| Frame + 2 anchor stages | one micrometer slide (≥10 µm, ≥10 mm travel) | the δ knob |
| Mass | 100–300 g, retroreflective marker | m |
| Bifilar suspension | ℓ ≥ 1.5 m | one-axis constraint; g/ℓ term → §7.3 |
| Camera ≥ 240 fps | phone acceptable | tracking |
| Force gauge + stage | 0.01 N | static λ |
| Release solenoid / burn-wire | — | repeatable release |
| Scale, calipers, thermometer | — | m, geometry, drift |

**Design point:** k = 500 N/m, L = 0.12 m, m = 0.2 kg → λ ≈ 8.7 kN/m³; window A ∈ [1.0, 2.4] cm; periods 0.74–1.78 s (μ=0 idealization; 0.71–1.43 s with suspension term); spring strain ≤ 2%.

## 5 · Setup steps

1. Frame/anchors; frame resonances > 20× clock frequency (tap test).
2. Bifilar suspension; free-swing single clean mode; record T_g ±0.5% (measures g/ℓ).
3. Attach springs at equal heights; mass centered (caliper both sides).
4. Coarse δ≈0: slack until springs bow, advance to straight (±0.2 mm).
5. **Fine zero via physics:** smallest-window-amplitude release; μ>0 shows period saturation, μ<0 off-center rest; bisect. Record δ₀; H2 detunings relative to it.
6. Tracking calibration: ruler in plane, ≥3 positions.
7. Log temperature (steel k drifts ~−0.03%/°C).

## 6 · Measurement protocol

**Window declared before analysis:** A ∈ [A_min, A_max], A_max = 0.2L (x⁶ ≤ 1.2%), A_min = max(3 mm, model-dominated μ-correction bound). Design point: [1.0, 2.4] cm.

- **R1 (primary), 10 ringdowns:** release at 1.1·A_max, record to A_min/2. Per-cycle T from same-direction zero crossings (sub-frame interpolation); per-cycle A = geometric mean of bracketing peaks.
- **R2, 5×3 fixed-amplitude releases:** cross-check vs ringdown sweep.
- **R3:** one ringdown per detuning (five).
- **R4:** three best ringdowns at full resolution for ℬ over sliding 5-cycle segments.

## 7 · Calibration chain

1. **λ static:** clamp frame, force gauge, F(x) at ≥12 points both signs; fit F = −c₃x³ (+c₁x nuisance); λ = c₃/4. Gate: |c₁|·A_max ≤ 0.05·c₃·A_max³. Target σ_λ/λ ≤ 1%.
2. **m:** weigh (±0.1 g); spring-inertia correction ≤1.5%, carry ±½ as systematic; dynamic cross-check at large pretension (ω₀² = μ/m_eff).
3. **Suspension term (SIZED BY SIMULATION — large, handled):** μ_g = m·g/ℓ = 1.31 N/m at design point; shifts periods ~20% at A_min (more below); naive slope s ≈ 0.81, G\* −8%. ⚠ *v1 undersized this ("1–3%"); corrected 2026-08-05 from simulation.* The declared analysis models μ (pinned by T_g) and passes H1 even mis-pinned ±5% (§12 S3). Naive analysis is not a valid fallback.
4. **x⁶ term (measured):** +0.30% at 0.1L, **+1.17% at 0.2L** (§12 S2); book 1.2% at window edge; treatments (a) window+systematic, (b) analytic x⁶ in fit; agreement is a consistency check. The +0.28% recovered-G\* bias in §12 is this term.
5. **Damping:** Q from envelope; period bias O(1/Q²) negligible; per-cycle amplitude bias first-order cancelled by geometric-mean rule.

**Error budget (G\*ₑₓₚ):** λ 1.0%, m_eff 0.8%, timing <0.1%, amplitude 0.5%, residual-μ 0.5%, x⁶ 0.5% → ≈1.1% through the square roots — inside the 2% gate.

## 8 · Analysis plan (locked before data)

1. Per-cycle (Tᵢ, Aᵢ) extraction; code frozen + hash-recorded before R1 (webapp clock panel = reference implementation).
2. **H1:** log-log slope; G\*ₑₓₚ per cycle; run-median; bootstrap CI.
3. **H2:** global fit, shared (λ, m_eff), per-detuning μ; μ(δ) linearity; intercept vs §7.3.
4. **H3:** ℬ per segment vs amplitude, against 2 and 1.9679 and the exact ℬ(k) curve. **Mandatory rule (simulation-derived):** map each cycle onto its own [0, 2π) phase before averaging — fixed-period resampling is chirp-biased (+2–5%, pushes ℬ above 2). Per-cycle mapping is unbiased at 3×10⁻⁵ on exact controls.
5. Gates exactly as §3; no post-hoc tolerance adjustment.

## 9 · Outcome classes

- **A — CONFIRMED:** H1+H2 pass. Working lemniscatic clock; G\* at ≤2% calibration-free. Publishable regardless of H3.
- **B — QUARTIC BUT MISCALIBRATED:** s in gate, G\*ₑₓₚ off 2–5%. Iterate §7, do not touch §3.
- **C — EDGE UNREACHABLE:** H2 monotonicity fails. Imperfection dominates; document and redesign — quantifying how hard a real edge is to hold is honest content.
- **D — CLOCK LAW VIOLATED:** H2 passes, s outside [0.9, 1.1] on the quartic branch. V is not §2.1 (non-Hookean, hidden compliance); trace it.

## 10 · Extensions

MEMS/NEMS buckled beam at threshold (kHz–MHz, high Q); cQED quartic point (quantum E ∝ n^{4/3}); cold-atom quartic traps (historically built for fast-rotating BEC vortex lattices).

## 11 · Relation to FTD (internal readers only; delete from any submitted version)

Registers nothing; asserts no FTD claim. Internally: the δ-knob traversal physically realizes the Maxwell-trichotomy bracket (μ>0 → n=2; μ<0 → flat/multi-well; the measure-zero point between → n=4), reached here *by tuning*. The open question C3 is whether any substrate configuration sits there *without* a tuner; this experiment neither supports nor undermines that. Outcome C would quantify how hard edge-sitting is for a real system — honest context for C3's negative record.

## 12 · Pre-build validation (simulated, 2026-08-05)

Truth = full two-spring geometry + suspension gravity term + damping; analysis = §8 as declared. Script: `sim_quartic_clock_validation_v1.py` (this folder).

| Sim | Result | Verdict |
|---|---|---|
| S1 fit model vs quadrature | exact to 6×10⁻¹⁴ over (μ, A) grid incl. μ<0 | PASS |
| S2 truncation bias | +0.30% at 0.1L, **+1.17% at 0.2L**, +2.61% at 0.3L | book 1.2% |
| S3 end-to-end H1 | 122 cycles, 80 in window, Q≈300. Naive: s=0.805, G\* −8.1% (fails). Corrected: s=0.988, G\* **+0.28%**; mis-pin ±5% → +0.70%/−0.12% | **H1 PASS** |
| S4 edge traversal | shared-λ −1.4% (x⁶ absorption); μ(δ) slope −0.44% vs 2k/L; intercept = μ_g +3.2%; R² = 0.999994 | **H2 PASS** |
| S5 waveform ℬ | controls ℬ₂ = 2.0000141, ℬ₄ = 1.9679114; per-cycle-mapped ringdown ℬ = 1.9735→1.9941, below 2 throughout, on the exact ℬ(k) curve; fixed-T resampling biased (+2–5%) and prohibited | **H3 achievable** |

Corrections fed back into this document by simulation: §7.3 sizing; §8.4 extraction rule.

---

*Prepared 2026-08-05. Fit model verified analytically in both limits; constants verified at 25 dps (G\* digit string corrected same date by independent audit). §12 validation added same date.*
