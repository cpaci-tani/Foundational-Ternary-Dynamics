# FTD-0803 — G\* sits at a saddle of an SL₂(ℤ)-invariant, giving a log density divergence of bounded consequence

**Status:** `[THEOREM — CLOSED FORM, saddle character and log coefficient]` +
`[MEASURED — coefficient confirmed to 0.2%]` +
`[CLOSED NEGATIVE — the "structurally guaranteed uninformative" consequence is REFUTED]` +
`[CORRECTION — five statements in v1 of this row were wrong or overstated]`
**Verdict:** `SADDLE_AND_LOG_DIVERGENCE_CONFIRMED_BUT_THE_ENHANCEMENT_IS_BOUNDED_AT_4_TO_5x`
**Parent:** FTD-0321. **Audited:** independent refute-by-default pass, 2026-08-05.
**Production impact:** none.

---

## 1. What survives, and it is stronger than v1 claimed

**`G*(τ) = √(8π·|η(τ)|⁴·Im τ)` is SL₂(ℤ)-invariant.** Two independent `η`
implementations (theta-quotient and q-product) agree to `3.9e-61`; random
SL₂(ℤ) words over 66 γ-images and 5 base points give worst deviation `9.2e-59`.

**`τ = i` is a critical point, by symmetry.** `S(τ) = −1/τ` is holomorphic with
`S'(i) = −1`, so its real Jacobian at `i` is `−I₂`. For real-valued `G` with
`G∘S = G`, the chain rule gives `dG_i∘(−I) = dG_i`, hence `dG_i = 0`. No
holomorphy of `G` is used.

**It is a saddle — provable in closed form.** v1 reported this only as a
measurement. `log G* = ½log(8π) + ½log y + 2log|η|`, and `log|η|` is harmonic
(η is non-vanishing holomorphic), so

```
Δ log G* = −1/(2y²)   exactly   ⟹   tr Hess(G*) = −G*/(2y²) at any critical point
```

With `u := Re(log η)''(i) = π²E₄(i)/72 − 1/8 = 0.0745528312288587`,

```
Hess(G*)|_i = G*(i)·diag(2u, −2u − 1/2),    d²G/dx∂y = 0 exactly
```

the mixed partial vanishing being *forced* by the `x → −x` mirror. Hence

```
saddle  ⟺  9/π² < E₄(i) < 27/π²  ⟺  0.91189 < 1.455763 < 2.73567   ✓ strictly inside
```

**Only two critical points exist in the fundamental domain.**
`|∇ log G*| = (π/6)|E₂*(τ)|`, so critical points are exactly the zeros of the
non-holomorphic weight-2 Eisenstein series. A 2401×4001 grid finds only `i` and
`ρ`; excluding disks of radius 0.05/0.1/0.2 the minimum `|E₂*|` rises
monotonically 0.0141/0.0276/0.0493.

**`τ = ρ` is a nondegenerate local maximum** (Hessian `−0.9967·I`, exactly
isotropic as order-3 symmetry forces) — and this is the control that makes the
whole argument work; see §2.

**The divergence is logarithmic with a parameter-free coefficient.** Reduced
forms with `|d| ≤ D` equidistribute w.r.t. `dxdy/y²` (elementary lattice count,
verified: predicted 212,086 vs actual 202,198 in a box around `i`). Because
`|τ| ≥ 1`, only the *half*-neighbourhood of `i` is populated:

```
A = c(D)/√(λ₁|λ₂|),    c(D) = 3N/π,    √(λ₁|λ₂|) = 0.920453909591
```

| D | A_fit | A_theory | ratio | rms(log) | rms(free power) |
|---:|---:|---:|---:|---:|---:|
| 25,000 | 367,273 | 709,231 | 0.518 | 0.084 | 0.098 |
| 100,000 | 4,624,779 | 5,700,076 | 0.811 | 0.056 | 0.082 |
| 400,000 | 44,973,027 | 45,703,943 | **0.984** | 0.0041 | 0.040 |
| 1,600,000 | 365,283,767 | 366,045,113 | **0.998** | **0.0007** | 0.038 |

Monotone convergence to theory as the finite-`D` cutoff recedes. A 3×10⁸-sample
Monte Carlo from `dxdy/y²` (no arithmetic granularity at all) shows a clean log
over **eight decades** (`w = 1e-1 … 1e-9`), `A/N = 1.0768` vs theory `1.0375`.
A constrained `w^(−1/2)` law is **50× worse** (rel rms 0.689 vs 0.0136). Power
laws are decisively excluded.

**The `ρ` control behaves exactly as 2D theory demands:** no divergence at the
extremum (MC density flat at ~1.35 across six decades; arithmetic `A_fit/A_theory
→ 0`), log divergence at the saddle.

## 2. The v1 reasoning was invalid — a critical point is not enough

v1 argued: *"near a critical point the map is degenerate, so the density
diverges."* **That is false in 2D**, and `ρ` is the counterexample: it is a
forced critical point of the same invariant, and the density there does **not**
diverge. In 2D an extremum gives a finite step; only a **saddle** gives the
logarithmic divergence. The conclusion is rescued by the saddle-specific
statement, but the general argument v1 gave does not support it.

## 3. The consequence is REFUTED — the enhancement is bounded

v1 concluded that FTD-0321's scan was *"structurally guaranteed to be
uninformative at scale."* **This is a quantitative non-sequitur.**

Sliding the true match window (half-width `1.843e-6`; the `x_-` leg is 1400×
looser and never binds) across `(2.90, 2.98)`:

| \|d\| ≤ | forms | at `G*(i)` | band mean | enhancement | P(≥1) at a *random* target |
|---:|---:|---:|---:|---:|---:|
| 200,000 | 15,561,211 | 640 | 164.5 | 3.9× | **1.0000** |
| 1,000,000 | 174,284,911 | 7,769 | 1,839 | 4.2× | **1.0000** |
| 4,000,000 | 1,395,267,675 | 67,603 | 14,715 | 4.6× | **1.0000** |
| 16,000,000 | 11,166,117,863 | 551,131 | 117,721 | **4.68×** | **1.0000** |

**The enhancement converges to a bounded ~4–5× and provably cannot grow:** `A`,
`B` and the generic density all scale as `c(D) = 3N/π`, so the ratio is
`D`-independent, while the raw CM count grows as `D^{3/2}` without bound.

**The scan was uninformative because there are ~10⁸ CM values per unit `G*`, not
because of the singularity.** A *randomly placed* target already gets
`P(≥1) = 1.0000` with ~165–200 matchers. Removing the singularity entirely would
not have saved it.

**And the registered scan was not saturated at all.** The crossover where a
random target expects one chance matcher is `|d| ≈ 6,676`. FTD-0321's registered
domain `|d| ≤ 907` is **~7× inside the informative regime** (expected chance
matchers ≈ 0.05, ≈ 0.2 with the van Hove factor). So the registered
UNIQUE-CONFIRMED result **was** informative; only the declared deep extension is
saturated, and v1's "at scale" qualifier was carrying the entire claim.

## 4. Corrections to v1 of this row

| # | v1 said | Corrected |
|---|---|---|
| 1 | critical point ⟹ density diverges | **False in 2D.** Saddle-specific; `ρ` is the counterexample (§2) |
| 2 | *"Only `dG/dy` isolates `τ = i`"* | **Same error class v1 had just rejected.** `\|τ\| = 1` is *also* a mirror (`τ → 1/τ̄`, anti-holomorphic, fixing the arc pointwise), so `dG/dr` vanishes along the **whole** unit arc. At `i` the arc normal *is* `y`, so `dG/dy(i) = 0` is exactly as trivial as `dG/dx = 0` on `x = 0`. **Honest statement: `τ = i` is the transverse intersection of two mirrors, each killing one gradient component.** Same for `ρ`. |
| 3 | slope ratio *"68× / 350× / unbounded"* | **Meaningless.** The denominator's true value is zero and the measured values are noise; at a different generic value (2.951234) the same ratios come out **negative**. v1 picked the generic point with the smallest accidental slope. Use `A_measured/A_theory` = 0.984, 0.998, and `A ≈ 0` elsewhere. |
| 4 | *"global max of the distribution = G\*(ρ)"* | Conflates the max of the **function** with a max of the **density**. Density at `G*(ρ)` is ~3× *lower* than at `G*(i)`. |
| 5 | *"density argument is numerical, not proved"*; Duke cited | **Under-claimed and mis-armed.** The saddle is closed-form provable and the log coefficient is a parameter-free prediction confirmed to 0.2%. Duke's theorem concerns a single `d → ∞`; this scan aggregates over all `|d| ≤ D`, for which equidistribution is an **elementary lattice count** — tested directly, no Duke needed. |

## 5. Incidental, and worth recording

The scan's target `x₊ = 137.0361714582` corresponds to `g = 2.958675119189114`
— `G*(i)` to `4.8e-13`, i.e. `2.6e-7` window half-widths. The target sits at the
singularity **by construction**. But CODATA `1/α = 137.035999177` corresponds to
`g = 2.9586732801`, displaced from `G*(i)` by `1.839e-6` = **0.998 window
half-widths**: the physical constant sits essentially exactly on the *edge* of
the registered tolerance, not at the singularity's peak.

## 6. Scope — unchanged and confirmed

The mechanism explains **CM-point scans only**. It does not transfer to
FTD-0319 or OT-3.3, whose nulls expect `1.42–1.67` and `0.0014` matchers — the
*opposite* regime. A crowding mechanism cannot explain a sparsity failure. The
audit confirmed this disclaimer is correctly stated in both analysis documents.

## 7. Cross-references

FTD-0321, FTD-0791 / FTD-0802 (whose results this does **not** explain — §6),
`ANALYSIS_DAMERELL_IDEAL_CLASS_SCAN_v1.md`, `TRACKER_ONTIC_TRUTH.md` OT-1.9 /
OT-5.1, `SPEC_ALGEBRAIC_SPINE.md` §3.
