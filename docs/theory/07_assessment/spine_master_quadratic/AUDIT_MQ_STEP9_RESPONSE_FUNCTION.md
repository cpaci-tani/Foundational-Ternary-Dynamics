# Audit — the response function at step 9 of the compact-phase route to the master quadratic

**Claim id:** FTD-1025 (drafted, unbooked)
**Verdict:** `[CLOSED NEGATIVE]` on the compact-phase/BCC-stiffness derivation route
**Tags moved:** none. `x₊ = 1/α` remains `[STRONGLY MOTIVATED CONJECTURE]`; the master
quadratic as algebra remains `[THEOREM]`; the Watson identity `W_BCC = G*²/(2π)` remains
`[THEOREM]` (SPEC_ALGEBRAIC_SPINE Theorem 5).
**Verifier:** `scripts/proofs/proof_mq_step9_response_function.py`
**Date:** 2026-09-03

---

## 1. Scope

A derivation route has been proposed which recovers the bare master quadratic

$$x^2 - 16G^{*2}x + 16G^{*3} = 0$$

as the characteristic equation of a four-stage construction: a `C₄` quarter-sector
determinant supplying `G*`, reflection-positive gluing supplying `√G*`, a unimodular
two-branch transfer supplying the reciprocal normalisation `y₊ + y₋ = 1`, and a BCC
compact-phase response supplying the scale `x = 16G*²y`. The first three stages are
verified here as stated. This audit concerns the fourth, which is the sole point at
which `G*` enters the linear coefficient.

The route's step 9 posits, for a compact phase `Φ` and branch allocation `y`,

$$\Gamma(\Phi) = 4\log\!\left[1 + 2W_{\mathrm{BCC}}\,y\,(1-\cos\Phi)\right],
\qquad \Gamma(\Phi) = \Gamma(0) + \tfrac12\beta\Phi^2 + O(\Phi^4),$$

whence `β = 8W_BCC·y`, and identifies `β` with the inverse coupling `1/e²` of a
unit-normalised compact `U(1)` field, giving `x = 4πβ = 16G*²y` by way of
`W_BCC = G*²/(2π)`.

Two questions are separable and are answered independently: what quantity the posited
`Γ(Φ)` computes, and what quantity the identification `β = 1/e²` requires.

## 2. Identification of the posited response

**Result 2.1.** The functional form of `Γ(Φ)` is the rank-one lattice defect free
energy. For a lattice operator `M₀` with local Green function `G₀(0,0)` and a
single-site potential of strength `v` at the origin, the matrix determinant lemma gives

$$\det\!\left(M_0 + v\,e_0e_0^{\mathsf T}\right) = \det(M_0)\left[1 + v\,G_0(0,0)\right].$$

Setting `v = 2y(1−cos Φ)` and `G₀(0,0) = W` reproduces the bracket of step 9 exactly;
the prefactor 4 counts four independent copies. The identity is verified numerically by
dense linear algebra on an `8³` torus for the simple-cubic, body-centred and
face-centred structure functions, with residuals `1.5e-13`, `2.2e-14` and `9.4e-16`
respectively.

The quantity `Γ(Φ)` therefore measures a **local susceptibility**: the free-energy cost
of a point defect, whose coefficient is the Brillouin-zone *integral* of the propagator.

## 3. The quantity the identification requires

**Result 3.1.** An inverse gauge coupling, equivalently a helicity modulus, is the
`k → 0` curvature of the *inverse* propagator,

$$1 - \lambda(k) \;\simeq\; s\,|k|^2, \qquad
s = \frac{1}{2z}\sum_{\delta} \delta_x^2 ,$$

a finite sum over integer bond vectors. Computed in the same convention that defines
each lattice's `λ(k)` — verified to `3e-16` against `λ(k) = z^{-1}\sum_\delta \cos(k\!\cdot\!\delta)`
— the coefficients are exactly rational and isotropic:

| lattice | `z` | stiffness `s` (exact) | `W` (BZ integral) |
|---|---|---|---|
| SC | 6 | `1/6` | 1.516386059152 |
| BCC | 8 | `1/2` | 1.393203929686 |
| FCC | 12 | `1/3` | 1.344661183165 |

**Corollary 3.2.** No finite-range structure function can place a Gamma-function value in
`s`, because `s` is a quadratic form in integer bond vectors. `G*` cannot appear in a
twist stiffness.

The two quantities are extracted from the *same* `λ(k)` by opposite operations — a
Taylor coefficient at the zone centre versus an integral over the whole zone. For BCC
their ratio is `W_BCC/s = 2W_BCC = 2.786408`, transcendental.

## 4. Consequence for the linear coefficient

The cancellation that produces `16G*²` is

$$A \;=\; \kappa\,z\,W_{\mathrm{BCC}} \;=\; 4\pi\cdot 8\cdot \frac{G^{*2}}{2\pi} \;=\; 16G^{*2},$$

in which `π` cancels **only** because `W_BCC` carries the factor `1/(2π)` supplied by the
Watson identity — that is, only because a Brillouin-zone integral was used. Substituting
the stiffness gives `A = 4π·8·(1/2) = 16π = 50.265482`, and across every lattice and
every standard gauge normalisation `κ ∈ {4π, 2π, π, 1}` the resulting `A` is a rational
multiple of `π`, with no value falling in the admissible window `(α⁻¹, 2α⁻¹)` required
for a root at `α⁻¹`.

**Result 4.1 (the dilemma).** Either

- `β` is the local susceptibility that `Γ(Φ)` actually computes — in which case `W` is the
  correct coefficient, `A = 16G*²` follows, and the identification `β = 1/e²` is
  unsupported, so the route yields no statement about `α`; or
- `β` is the twist stiffness that `β = 1/e²` requires — in which case the coefficient is
  the rational `s`, `G*` is absent from `A`, and the master quadratic does not arise.

Under neither reading does the route derive `x₊ = 1/α`.

## 5. Disposition

Closed as a derivation route. The following are **unaffected**:

- the master quadratic as algebra, and its roots `x_± = 8G^{*2} \pm 4G^{*}\delta` with
  `δ = √(G*(4G*−1))` the FTD-0784 surd (verified to 20 digits; the surd form holds at the
  bare point `R = 1` only);
- the Watson identity `W_BCC = Γ(1/4)⁴/(4π³) = G*²/(2π)`, independently reconfirmed here
  against Brillouin-zone sums with Richardson extrapolation (relative agreement `8.6e-8`;
  SC `7.6e-9`, FCC `2.9e-8`);
- the quarter-sector determinant ratio `det_ζ D_{3/4}/det_ζ D_{1/4} = G*` (Lerch), the
  `C₄` orbit algebra, and the reciprocal-branch normalisation, all verified as stated;
- the empirical residual: `x₊ = 137.036171458155` lies `1.2572` ppm from CODATA-2022
  `α⁻¹`, an unexplained coincidence which retains `[SMC]` status and, per FTD-0791 /
  FTD-0802, no numerical-uniqueness support.

The closure is of the *mechanism*, not of the coincidence. Read against
`SPEC_OPEN_MATH_BY_SECTOR`, this route does not evade MC-T4.3 (the non-action
α-injection obstruction); under reading (i) of §4 it instantiates it, the identification
`β = 1/e²` being precisely the unearned injection.

## 6. Forward falsifier — run, `[MEASURED]`

The two readings make incompatible predictions for the compact-phase twist stiffness of
the BCC neighbour set, in units of the bond coupling, requiring no knowledge of `α`:

| reading | predicted `Υ/J` | predicted `β = Υy₊/J` |
|---|---|---|
| step 9 as written, `zW` | `11.145631437` | `10.904992035` |
| twist stiffness, `zs`, `s = 1/2` | `4.000000000` | `3.913638127` |

**Result 6.1 (rigorous ceiling).** For `H = −J Σ_b cos Δθ_b` the standard estimator is
`Υ = V⁻¹[J⟨Σ_b (x̂·δ_b)² cos Δθ_b⟩ − βJ²⟨(Σ_b (x̂·δ_b) sin Δθ_b)²⟩]`. The first term is
bounded by the purely geometric `V⁻¹ J Σ_b (x̂·δ_b)²` and the second is non-negative, so
for the BCC neighbour set

$$\Upsilon/J \;\le\; 4 \qquad \text{at every temperature and in every state.}$$

The step-9 value `11.145631` exceeds this ceiling by exactly `2W_BCC = 2.786408`. It is
unreachable, not merely disfavoured.

**Result 6.2 (exact Gaussian twist).** Imposing `θ_r = φ_r + k·r` on the harmonic model,
the linear term vanishes for periodic `φ` and the curvature is the same bond sum;
`z(1−λ(k))/k² → 4.0000000` as `k → 0` (evaluated at `k = 10⁻²`, `10⁻³`, `10⁻⁴`).

**Result 6.3 (Monte Carlo).** Compact XY on the BCC neighbour set, heat-bath (von Mises)
two-colour updates, helicity modulus by the fluctuation estimator:

| `N` | `T` | `Υ/J` | `Υ/Υ_step9` |
|---|---|---|---|
| 12 | 0.20 | 3.898631 | 0.350 |
| 12 | 0.50 | 3.739396 | 0.336 |
| 12 | 1.00 | 3.448487 | 0.309 |
| 12 | 1.50 | 3.104044 | 0.279 |
| 12 | 2.00 | 2.652873 | 0.238 |
| 16 | 0.50 | 3.649548 | 0.327 |

`Υ/J` approaches the ceiling 4 from below as `T → 0` with a reduction linear in `T`
(`(4−Υ)/T = 0.505, 0.522, 0.552` at `T = 0.2, 0.5, 1.0`), the spin-wave expectation —
an internal check that the sampler is correct. No run approaches `11.146`, and none can.

The step-9 identification `β = zWy` is therefore **refuted as a twist response** by
three independent routes: a geometric bound, an exact deterministic computation, and a
sampled measurement. Verifier: `scripts/proofs/proof_bcc_twist_stiffness_measurement.py`.

## 7. Note on the dressed response `R`

Independently of the above, the dressed form `x² − 16G*²x + 16G*³R = 0` carries
`dx₊/dR = −3.0922`, so reproducing `α⁻¹` at CODATA precision requires `R` known to
`9.3×10⁻¹⁰`. The value required to hit `α⁻¹` exactly is `R = 1.000055714695013`; the
value quoted for the finite-response model agrees with it to twelve significant figures.
Absent an independent determination of the free parameter `λ` in
`R = 1 + λ + \tfrac13 \partial_t\log\Psi(t)`, `R`-dressed roots should be reported to no
more than twelve digits and recorded as calibration, not prediction.

## 8. Reproduction

```
python scripts/proofs/proof_mq_step9_response_function.py
```

Every constant is computed in-run from a formula; each lattice Watson value is produced
by a closed form and an independent Brillouin-zone route, both printed, per the
FTD-0802 discipline.
