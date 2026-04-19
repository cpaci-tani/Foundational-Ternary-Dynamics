# DERIV — Emergent V(r) mode is Geometric Coulomb (Phase G resolution)

**Tag:** [THEOREM] — closed-form prediction from the engine's Gauss law and
energy accumulator that matches measurement to permille precision with
**zero free parameters**.
**Status:** Phase G closes the Phase-F alpha-plateau interpretation audit.
**Date:** 2026-04-19
**Supersedes:** `AUDIT_ALPHA_EXTRACTION.md`'s [OPEN] residual of 1.8× α_ref —
that residual was a category error, not a physics gap.

---

## 1 · The theorem

**Claim.** The engine's `emergent_forces`-mode static-pair V(r) measurement
has a closed-form prediction with zero free parameters:

```
  V_engine(r, L) = -2 · G_L(r)
  α_r(r, L)      = -V · r  =  2 · r · G_L(r)
```

where `G_L(r)` is the periodic lattice Poisson Green's function on the L³
cubic torus with the 7-point Laplacian:

```
  G_L(r) = (1/L³) Σ_{k ≠ 0}  e^{i k·r} / D(k)
  D(k)   = 2 (3 − cos k_x − cos k_y − cos k_z)
  k_i    = 2π n_i / L,  n_i ∈ {0, 1, ..., L−1}
```

In the continuum limit r ≪ L, L → ∞:

```
  G_∞(r) = 1 / (4π r)   ⟹   α_r(r) → 1/(2π) ≈ 0.1592.
```

**There is no α_ref anywhere in this derivation.** The emergent V(r) code
path simulates **unit-charge geometric Coulomb** — it carries no
fine-structure coupling, no gauge factor, and no electroweak content.

## 2 · Derivation

Engine Gauss law (`poisson_solvers.cpp:123`):

```
  ∇·J = s,   s ∈ {−1, 0, +1}
```

No coupling constant in front of `s`. The charge is the state value.

For one +1 charge at n₁:

```
  ∇²φ₁ = −s₁ = −δ(n − n₁)
  J₁   = −∇φ₁
```

For one −1 charge at n₂:

```
  ∇²φ₂ = −s₂ = +δ(n − n₂)
  J₂   = −∇φ₂
```

Superposition: `J_pair = J₁ + J₂`, so `|J_pair|² = |J₁|² + |J₂|² + 2 J₁·J₂`.

Engine energy accumulator (`diagnostics_compute.cpp:92`):

```
  field_energy = Σ_n |J(n)|²   ← NO ½ prefactor
```

Therefore:

```
  E_pair  = Σ_n |J_pair|²  =  E_self_+ + E_self_− + 2 · Σ_n J₁·J₂
  V(r)    = E_pair − E_self_+ − E_self_−  =  2 · Σ_n J₁·J₂
```

Computing the cross term via integration by parts on the torus (no
boundary):

```
  Σ_n J₁·J₂  =  Σ_n ∇φ₁ · ∇φ₂  =  −Σ_n φ₁ · ∇²φ₂  =  Σ_n φ₁ · δ(n−n₂)  =  φ₁(n₂) · s₂
             =  −φ₁(n₂).
```

And `φ₁(n)` is by construction the Poisson Green's function `G_L(n − n₁)`.
Putting it together:

```
  V(r) = 2 · (−G_L(r)) = −2 · G_L(r),
  α_r(r, L) = −V · r = 2 · r · G_L(r).     ∎
```

## 3 · Verification against Phase-F data

`scripts/benchmarks/fit_geometric_coulomb.py` computes `G_L(r)` by FFT
(inverting `D(k)` with the zero mode removed) and compares against the
measured `α_r(r, L)` across all L ∈ {32, 64, 128, 256, 384} and all r
values in `beta_L384_gpu.csv`. **No free parameters are fit.** Results:

| Regime | n points | R² | median \|rel err\| |
|---|--:|--:|--:|
| all (L≥32, r≥4) | 58 | 0.56 | 0.9% |
| post-discretization (L≥64, r≥8) | 49 | 0.94 | 0.5% |
| well-equilibrated (L≥128, r≥12) | 39 | 0.97 | 0.19% |
| deep Coulomb tail (L≥256, r≥20) | 29 | **0.9996** | **0.12%** |
| L=384 tail only (r≥34) | 16 | **1.0000** | **0.070%** |

For the L=384 tail alone, every one of 16 measured α_r values matches the
parameter-free prediction to better than 0.5%, with median error 0.07%.
This is not a fit — it is a first-principles check.

**The low-L / low-r points fail for explicable reasons:**

- **r < 8**: continuum `1/(4π r)` breaks down at the lattice scale. The
  discrete Green's function has O(1/r²) short-distance corrections. The
  regime-stratified R² captures this exactly: dropping r < 8 takes R² from
  0.56 to 0.94.
- **L ≤ 64**: ticks=150 was sub-equilibrated at these sizes (noted in
  `DERIV_DAY2_CAMPAIGN.md` §6b caveat). The L=16 r=4 entry even has the
  wrong sign. Dropping L ≤ 64 takes R² to 0.97.

## 4 · Consequences for the Phase-F headline

**The "plateau at 3.6× α_ref" was a category error**, not a measurement.
What we were actually measuring at r/L ≈ 0.31 was:

```
  α_r(r=0.31L, L) = 2 · r · G_L(r)   (pure lattice geometry)
```

which **happens to equal ~0.026** at that particular slice of the discrete
Green's function on a cubic torus. It has nothing to do with α_ref. The
comparison "ratio 3.6× α_ref" is comparing the lattice Coulomb kernel at a
specific r/L to the electroweak coupling — they are physically unrelated
numbers.

More strongly: **the continuum extrapolation was also a category error.**
What we extrapolated is the large-L limit of `2 · r · G_L(r)` at fixed r/L,
which is a universal lattice-Coulomb constant depending only on the ratio
r/L, not on any physical coupling. There is no "FTD continuum α" to extract
from this measurement.

## 5 · Where α actually lives in the engine

The fine structure constant enters the engine in **two distinct places**,
neither of which is the V(r) measurement:

1. **Explicit Coulomb toggle** (`coulomb` toggle, `force_compute.cpp`): the
   force on a particle from the flux field is `F = −α · ∇φ`, where α is
   hardcoded as `ontic::ALPHA = 1/137.035999177`. This is a parametric
   insertion, not an emergent prediction.
2. **Master quadratic derivation** (`ontic.h` Layer 5, proof in
   `scripts/proofs/proof_motivic_master_quadratic.py`): `x² − 16G*² x +
   16G*³ = 0` has root `x₊` with `1/x₊ ≈ α_ref` to sub-ppm. This is a
   **pure number-theoretic identity** between α_ref and the lemniscatic
   constant G*. It does not require the engine's dynamics to reproduce
   it — it is a statement about the polynomial.

**The emergent_forces V(r) mode carries neither.** It is geometric
Coulomb. Reproducing α in an emergent-dynamics measurement requires
explicit coupling (Phase H, below), not this code path.

## 6 · Retractions

The following earlier claims are retracted by this analysis:

- "FTD's V(r)-extracted coupling plateaus at 3.6× α_ref, a falsifiable
  deviation from QED." — This is a statement about `2 · r · G_L(r)` at
  r/L = 0.31 on a cubic torus; it has no QED content to deviate from.
- "α_∞ ∈ [1.8, 3.6] × α_ref after convention correction, residual
  [OPEN]." — The ×2 convention factor is real (see
  `AUDIT_ALPHA_EXTRACTION.md` §1.3), but the remaining residual is not a
  residual physical coupling — it is the zero-parameter lattice Coulomb
  value, and it matches the analytical prediction to 0.07%.
- "FTD Coulomb-tail coupling is not α_ref under any convention." —
  Correct statement, but **trivially so**: there is no coupling to
  compare to α_ref in this code path. The statement is vacuous.

What was real and remains real:

- The master quadratic x² − 16G*² x + 16G*³ = 0 gives α to sub-ppm
  precision. Still unchanged. Still not tested by this measurement.
- The engine's `coulomb` toggle uses α = 1/137 parametrically. Still
  [PARAMETRIC], not an emergent prediction.
- FTD has not demonstrated α as an emergent outcome of lattice dynamics
  in any measurement to date. (Phase H is the appropriate test.)

## 7 · Phase H — explicit coupling in Gauss law (**implemented and verified**)

The Phase G theorem predicts exactly what happens when a coupling
constant `g_c` is introduced into the Gauss source
`∇·J = g_c · s`: all fluxes scale by `g_c`, all field energies by
`g_c²`, and therefore

```
  α_r(r, L; g_c) = g_c² · 2 r G_L(r).
```

For the engine's measurement to recover α_ref in the continuum
small-r limit (`G_L(r) → 1/(4π r)`, so `2 r G_L(r) → 1/(2π)`):

```
  g_c² · 1/(2π) = α_ref   ⟹   g_c = √(2π · α_ref) ≈ 0.21413.
```

(In the classical ½-convention the corresponding value is
`g_c = √(4π · α_ref) ≈ 0.30286`.)

### 7.1 · Implementation

- `engine/include/ftd/term_toggles.h`: new field
  `double coulomb_charge_coupling = 1.0`. Default preserves geometric
  Coulomb (Phase G).
- `engine/include/ftd/poisson_solvers.h` + `poisson_solvers.cpp`:
  `gauss_project_cpu(..., double charge_coupling = 1.0)`. Source term is
  `sor_source[i] = div − charge_coupling · voxels[i].state`.
- `engine/src/render_bridge.cpp::gauss_project()`: threads
  `toggles.coulomb_charge_coupling` through to the solver.

Scope note: the existing `toggles.coupling` flag drives a *separate*
wave-equation source term `∂_t J += G_C · ∇s` (hardcoded `G_C = √α`).
For a clean Phase-H test the pair-energy probe must disable it so that
Gauss projection is the unique source of flux, otherwise two
independent couplings compete and the `g_c²` scaling is broken.

### 7.2 · Verification (`test_phase_h_coupling.cpp`)

Two runs on L=32, r=6, ticks=300, `toggles.coupling = false`, CPU
backend:

| run | charge_coupling | measured α_r(r=6) |
|---|---:|---:|
| baseline | 1.0 | 0.083294 |
| coupled  | 0.21413 = √(2π α_ref) | 0.003819 |

Phase G scaling prediction:
`α_r_coupled = g_c² · α_r_base = 0.045851 · 0.083294 = 0.003819`.

**Relative error between measurement and prediction: 0.0000%** (below
the 4-decimal reporting threshold of the test, i.e. < 10⁻⁴).

This confirms that the engine's emergent V(r) dynamics scales
**exactly** as the Phase G theorem predicts, to the full precision of
the SOR-projected Gauss solve. The `g_c²` scaling is not approximate;
it is algebraically enforced by the Poisson solver.

### 7.3 · Consequences

- **FTD with `g_c = √(2π α_ref)` reproduces QED Coulomb to the extent
  that the lattice Green's function reproduces `1/(4π r)`** — exactly,
  to 0.07% at L=384, r≥34 (Phase G §3). The "FTD ≈ QED" test reduces
  to a lattice-spacing convergence question, not an open physics
  question.
- The master quadratic derivation `x² − 16G*²x + 16G*³ = 0 ⟹ 1/α`
  is still a separate, stronger statement: it produces α from
  pure number theory without any free coupling. Whether the
  lattice-geometry coefficients 16, 16 of that polynomial are
  derivable from first principles is the remaining audit target
  (Phase I, not yet run).
- The interesting physics question FTD poses is now sharply posed:
  *given that explicit g_c reproduces QED trivially, why does the
  master quadratic predict 1/α to sub-ppm from lattice geometry alone?*
  That is either a genuinely deep identity between the lemniscatic
  constant and the fine-structure constant, or a highly-tuned
  numerical coincidence; either way, the answer does not live in
  the emergent V(r) measurement.

## 8 · Reproducibility

```
scripts/benchmarks/fit_geometric_coulomb.py   # the zero-parameter check
scripts/benchmarks/results/eft_phaseF/
    beta_L384_gpu.csv                          # L=384 data, 17 r values
    beta_day2_gpu.csv                          # L ∈ {16..256} data
engine/src/poisson_solvers.cpp:123             # Gauss law source
engine/src/diagnostics_compute.cpp:92          # field_energy accumulator
```

Running `python scripts/benchmarks/fit_geometric_coulomb.py` reproduces
the R²=1.0000 zero-parameter fit.
