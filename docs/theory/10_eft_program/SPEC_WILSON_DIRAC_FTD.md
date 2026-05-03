# Specification — Wilson-Dirac Matter Sector for FTD

**Status:** [SELECTION] specification (Phase II.1 deliverable; first internal pre-registration milestone of `PREREG_PHASE_II_WILSON_DIRAC_G2.md`).
**Date:** 2026-05-03
**Pre-reg root:** `PREREG_PHASE_II_WILSON_DIRAC_G2.md` (tag `preregister-phase-ii-wilson-dirac-g2-v1`).
**Branch:** Branch-B matter sector per `SPEC_FTD_EFT_BRIDGE_CONTRACT.md`.

---

## 1 · Why Wilson-Dirac (not native FTD fermions)

**Native FTD fermion emergence is closed-negative.** FTD-0073 (mode-erasure no-go theorem) proves that site-local 0-form state-field readouts on FTD's ternary lattice cannot support Clifford structure under any pointwise-threshold dynamics tested (genesis, pair_production, weak_transmutation, etc.). FTD-0076 confirms that FTD's spontaneous matter emergence produces colored quarks (single-voxel, integer-charge, single-color states), not electrons. Therefore: any QED-sector test requires inserting Dirac fermions explicitly. This is a **Branch-B selection**, not a derivation from FTD axioms.

Wilson's 1974 lattice-QED prescription is the standard, well-validated method for putting Dirac fermions on a cubic lattice. It removes fermion doublers via a momentum-dependent mass (the Wilson term), with explicitly-broken chiral symmetry as the trade-off. For the g − 2 measurement at single-electron precision, chiral symmetry is not the load-bearing structure — magnetic moment is — so Wilson-Dirac is appropriate.

## 2 · Action

On a 4D Euclidean lattice (or 3+1 Minkowski; both versions specified below) with sites `n ∈ Z⁴`, lattice spacing `a` ≡ `ℓ_P`, and 4-component spinor field `ψ(n)`:

### 2.1 Free Wilson-Dirac (no FTD coupling yet)

$$
S_F^{\mathrm{free}} = a^4 \sum_n \bar\psi(n) D_W \psi(n)
$$

with the Wilson-Dirac operator

$$
D_W \psi(n) = \left(m + \frac{4r}{a}\right) \psi(n) - \frac{1}{2a} \sum_{\mu = 1}^{4} \left[ (r - \gamma^\mu)\, U_\mu(n)\, \psi(n + \hat\mu) + (r + \gamma^\mu)\, U_\mu^\dagger(n - \hat\mu)\, \psi(n - \hat\mu) \right]
$$

where:
- `m` is the bare quark/lepton mass parameter (set to `m_e` for electron sector)
- `r` is the Wilson parameter (canonical: `r = 1`)
- `γ^μ` are Euclidean γ-matrices (or Minkowski equivalents); standard chiral basis
- `U_μ(n)` are the gauge links — these encode the magnetic field and FTD coupling

### 2.2 FTD-coupled Wilson-Dirac

The gauge link `U_μ(n)` is constructed from the FTD flux field via:

$$
U_\mu(n) = \exp\!\left[ i a g_{\mathrm{FTD}}\, A_\mu(n) \right], \qquad g_{\mathrm{FTD}} = \sqrt{1 / x_+}
$$

where `g_FTD` is the FTD-native coupling [DERIVED from master quadratic, FTD-0125] and `A_μ(n)` is the gauge field projected from the FTD flux. The projection convention is per `DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md`:

$$
A_\mu(n) = \mathcal{P}_T J_\mu(n)
$$

where `P_T` is the transverse projector that extracts the gauge-equivalence-class representative from the lattice flux (i.e., the part that carries the U(1) connection structure, modulo the longitudinal Coulomb contribution which is the gauss-projected longitudinal mode).

**Cleanly stated:** the fermion's gauge connection is the transverse component of FTD's flux field, scaled by the master-quadratic coupling.

### 2.3 Magnetic field configuration

A uniform `B = B_0 ẑ` field is implemented in Landau gauge:

$$
A_x(n) = -B_0\, n_y a, \qquad A_y(n) = A_z(n) = A_t(n) = 0
$$

with corresponding gauge links `U_x(n) = exp(-i a g_FTD B_0 n_y a)` (a phase factor proportional to `n_y`).

This is held fixed during the dynamics; the fermion evolves in this background. Standard lattice prescription.

### 2.4 Time evolution

For the g − 2 measurement, we use the equation-of-motion approach:

$$
i \partial_t \psi(n) = D_W \psi(n)
$$

evolved via the leapfrog or RK4 time-integrator at time-step `τ ≪ a/c`. The lattice CFL bound on `τ` is the standard `τ ≤ a / (c \sqrt{D})` = `a / (c \sqrt{3})`. We choose `τ = a √3 / c` (the FTD canonical time-step, matching `t_phys` calibration in `SPEC_FTD.md`).

## 3 · Doubler handling

The Wilson term `(4r/a) ψ` in `D_W` lifts the 15 fermion doublers to mass `4r/a = 4/ℓ_P` (with `r = 1`), pushing them to the lattice cutoff scale where they are effectively decoupled from low-energy dynamics. This is the standard Wilson trick.

For the g − 2 measurement at electron mass `m_e ≪ 1/a`, doubler contamination is suppressed by `(m_e a)² ~ (m_e ℓ_P)² ~ 10⁻⁴⁴`. Negligible.

## 4 · Spin convention

The Dirac spinor `ψ` carries a 4-component spin-1/2 representation of the Lorentz group. The spin operator on the lattice is

$$
\Sigma^i = \frac{1}{2} \begin{pmatrix} \sigma^i & 0 \\ 0 & \sigma^i \end{pmatrix}
$$

(in the chiral basis). The expectation value `⟨ψ | Σ^z | ψ⟩` for a single-electron state in `B = B_0 ẑ` precesses at the Larmor frequency `ω_s = g (eB_0)/(2 m_e)` where `g = 2` at tree level (Dirac's prediction). The anomaly `a_e = (g − 2)/2` arises from one-loop corrections — Schwinger 1948.

## 5 · Cyclotron + spin-precession measurement

For a single electron initialized at position `(x_0, y_0, z_0)` with momentum `p_x` perpendicular to `B`:

- **Cyclotron frequency** `ω_c = e B_0 / m_e` (tree level): track `⟨ψ | x | ψ⟩(t)` and `⟨ψ | y | ψ⟩(t)`; the orbit closes with period `T_c = 2π / ω_c`.
- **Spin-precession frequency** `ω_s = g (eB_0) / (2 m_e)`: track `⟨ψ | Σ^x | ψ⟩(t)` and `⟨ψ | Σ^y | ψ⟩(t)`; the spin vector rotates with period `T_s = 2π / ω_s`.

The anomaly is

$$
a_e = \frac{g - 2}{2} = \frac{\omega_s - \omega_c}{\omega_c}
$$

Schwinger's tree-level prediction is `a_e = α/(2π)` where `α = g_FTD² = 1/x_+` in the FTD-native convention. For `α ≈ 1/137`:

$$
a_e^{\mathrm{Schwinger}} \approx 1.16 \times 10^{-3}
$$

## 6 · What is inserted vs derived

| Quantity | Status | Source |
|---|---|---|
| Wilson-Dirac action `D_W` | Branch-B INSERTED | Wilson 1974; standard lattice QED |
| Wilson parameter `r = 1` | [SELECTION] | Canonical choice |
| Bare mass `m = m_e` | Calibration-conditional | FTD-0096 K_B = m_e |
| γ-matrices, chiral basis | Standard | Convention |
| Gauge field `A_μ` from flux projection | [SELECTION] | `DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md` |
| FTD-native coupling `g_FTD = √(1/x_+)` | [DERIVED] | Master quadratic [THEOREM] + Phase I (FTD-0125) |
| Magnetic field `B = B_0 ẑ` | Initial condition | Lab-style setup |
| Time-step `τ = √3 ℓ_P / c` | Calibration | FTD canonical (matches `t_phys`) |
| Tree-level `g = 2` | Standard Dirac | Will verify in Phase II.4 |
| Schwinger anomaly `a_e = α/(2π)` at α = 1/x_+ | TEST PREDICTION | Phase II.5 verdict per pre-reg |

## 7 · Engine implementation outline (Phase II.2)

### 7.1 New header `engine/include/ftd/wilson_dirac.h`

- `struct WilsonDiracField { array<complex<double>, 4> psi[L³]; }` — 4-component spinor field on the lattice
- `struct GaugeLinks { array<complex<double>, 4> U_mu[L³]; }` — 4 gauge links per site (one per spatial+temporal direction)
- `void initialize_electron_state(WilsonDiracField&, vec3 pos, vec3 momentum, vec3 spin_axis)`
- `void initialize_uniform_B_field(GaugeLinks&, double B0, char axis)` — Landau gauge
- `void apply_wilson_dirac_step(WilsonDiracField&, const GaugeLinks&, double dt)` — one time-step

### 7.2 Toggle integration

- New `TermToggles::wilson_dirac` (default false)
- New `TermToggles::wilson_r = 1.0`
- New `TermToggles::dirac_mass = K_B` (= m_e per FTD calibration)
- Integration into `phase_write` cascade: when toggle on, after substrate update, call `apply_wilson_dirac_step` using gauge links derived from current `J` field

### 7.3 CUDA implementation

Standard lattice-QED CUDA pattern: one thread per lattice site, applies the Wilson-Dirac operator using the 7 neighbors (4D: 8 neighbors) per stencil. Gauge links stored as a separate field updated each tick from the FTD flux. ~300-500 LOC for the core kernel + helpers.

## 8 · Validation milestones for Phase II.2

Before Phase II.3 starts, the implementation must pass:

1. **Free-fermion smoke test:** `B = 0`, no FTD coupling. Inject Gaussian wave packet; verify it propagates as Klein-Gordon-like dispersion (Wilson-Dirac → free particle in continuum limit).
2. **Wilson term verification:** spectrum of `D_W` at `B = 0` shows expected Wilson dispersion; doublers at correct masses.
3. **Gauge link verification:** uniform `B` field configuration reproduces magnetic-translation symmetry on small lattice.
4. **Coupling consistency:** with `B = 0` and minimal FTD flux background, `D_W` reduces to free Wilson-Dirac (gauge link → 1).
5. **CPU/GPU parity:** golden-tick gate at single-tick precision (per ADR-0012).

Each validation gets its own internal pre-registration milestone before declaring Phase II.2 complete.

## 9 · Open questions (acknowledged before implementation)

1. **Flux projection convention `P_T J → A_μ`** — `DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md` gives a sketch; details (including units and gauge-fixing convention) need to be tightened before II.2.
2. **Time-step stability** — Wilson-Dirac at FTD's `τ = √3 ℓ_P / c` may need sub-stepping for numerical stability at single-electron precision.
3. **Boundary conditions** — periodic BC in 3+1 standard, but for cyclotron orbits we may need anti-periodic BC in time direction or open BC in y/z.
4. **Image-charge artifacts** — on a torus, the electron sees its own images. For B-field measurement we need L large enough that orbit radius ≪ L.

These will be addressed in Phase II.2 implementation, with explicit tagging if any compromises change the pre-registered protocol.

---

## Closure criterion for Phase II.1

This document is "closed" (Phase II.1 complete) when:
1. The action specification in §2 is reviewed for self-consistency
2. The gauge-link convention in §2.2 is consistent with `DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md`
3. The cyclotron + spin-precession measurement protocol in §5 is unambiguous
4. The validation milestones in §8 are concrete
5. Open questions in §9 are flagged (not resolved)

Not closed by implementation; that's Phase II.2.
