# Dark Sector Dynamics: Dark Energy and Dark Matter from Lattice Mechanics

## Unifying the Coupling Source and Selective Damping

**Document Version:** 1.0
**Date:** March 17, 2026
**Status:** [SELECTION] (subsections vary; see Claims Table S8)
**Framework:** Foundational Ternary Dynamics v5.28

> **Per FTD-0331 (governing)** the matter-driven flux injection carries **NO
> L-dependence** and is **NOT a viable Λ source** — the source is **[OPEN]** and
> FTD predicts **Λ=0**; the α¹⁶/α⁵⁷ value-match is **[PARAMETRIC]**. See
> [DERIV_LAMBDA_SCALE_COVARIANT.md](DERIV_LAMBDA_SCALE_COVARIANT.md).

**Depends on:**

- [SPEC_FTD_LAGRANGIAN.md](../01_reference/SPEC_FTD_LAGRANGIAN.md) -- Lagrangian, coupling term, Rayleigh dissipation
- [DERIV_COSMOLOGICAL_CONSTANT.md](../04_coupling/DERIV_COSMOLOGICAL_CONSTANT.md) -- rho_Lambda = m_e^4 * alpha^16 * G*^2
- [SPEC_ENGINE.md](../../../engine/SPEC_ENGINE.md) -- Tick cycle, selective damping, phase structure

---

## Abstract

We show that the FTD engine's two competing energy mechanisms -- coupling injection and selective damping -- naturally produce the dark sector of cosmology. Manifested particles continuously source flux into their neighborhood (dark energy), while only near-particle flux is damped; far-field flux propagates losslessly through the vacuum, constituting gravitationally active but electromagnetically invisible matter (dark matter). The self-field halo around every particle, measured at r_eff ~ 15 voxels with a 1% boundary at ~23 voxels, is the FTD analog of a dark matter halo.

This document connects the engine's measured behaviors to the cosmological constant formula rho_Lambda = m_e^4 * alpha^16 * G*^2 and provides 6 testable predictions verified by the `campaign_dark_sector` test suite.

---

# S1 -- The Coupling Source as Energy Injector

## 1.1 The Mechanism [THEOREM]

The FTD Lagrangian contains a coupling term (SPEC_FTD_LAGRANGIAN.md S3.4, L-7):

    L_coupling = -g_c * s * (div J)

where g_c = sqrt(alpha) ~ 0.0854 and s in {-1, 0, +1}.

The Euler-Lagrange equations yield the source term in the wave equation:

    d^2 J / dt^2 = c^2 * laplacian(J) + g_c * grad(s) + g_c * curl(s * v)

For a manifested particle at site x_0 with s(x_0) = +/-1 and s = 0 everywhere else, the gradient grad(s) is nonzero at the 6 face-neighbors of x_0. This is implemented in `render_bridge.cpp` phase_read (line 325):

    delta_j_[i] += gradient_state(i) * G_C;     // Source: grad(s)
    delta_j_[i] += curl_state_velocity(i) * G_C; // Source: curl(s*v)

**Every tick, each manifested particle injects new flux into its 6 face-neighbors.** This is not a numerical artifact -- it follows directly from the Lagrangian coupling term.

## 1.2 Injection Rate [THEOREM]

The energy injected per tick per particle is:

    dE/dt|_inject = g_c^2 * |grad(s)|^2 = alpha * |grad(s)|^2

Since |grad(s)| is O(1) at face-neighbor boundaries (the state field jumps from 0 to +/-1 over one lattice spacing), the injection rate is O(alpha) per tick per particle.

With alpha ~ 0.00730, a single locked particle injects roughly 0.007 * K_B^2 of flux energy per tick into its neighborhood. This is verified by engine test DS-3.

---

# S2 -- Rayleigh Dissipation as Energy Sink

## 2.1 The Damping Term [THEOREM]

The Rayleigh dissipation function (SPEC_FTD_LAGRANGIAN.md S3.5, L-8):

    R = (alpha/2) * |dJ/dt|^2

produces the damping force -alpha * dJ/dt in the equations of motion. In the engine, this is implemented as multiplicative damping per tick:

    J <- J * (1 - alpha)

removing a fraction alpha ~ 0.00730 of the flux energy each tick.

## 2.2 Selective Damping: The Critical Distinction [THEOREM]

**With selective_damping = true (the engine default), damping applies ONLY within 1 lattice hop of a manifested particle.** The implementation in `render_bridge.cpp` (line 389):

    const bool should_damp = !selective || near_particle_[i];

The `near_particle_` mask is precomputed each tick: a site is marked if it contains a manifested particle (s != 0) or is a face-neighbor (6-connected) of one.

**Consequence:** Far-field flux -- all flux beyond 1 lattice spacing from any manifested particle -- propagates **losslessly**. The vacuum is transparent. Electromagnetic waves in the FTD vacuum do not decay.

This is not a numerical convenience; it reflects the physical principle that **dissipation requires interaction with matter**. Vacuum flux has no state field to interact with, so no damping channel exists.

---

# S3 -- The Energy Balance = Dark Energy

## 3.1 Steady-State Balance [SELECTION]

At steady state around a locked manifested particle:

- **Injection** (coupling source): g_c * grad(s) adds ~alpha energy per tick to the 6 face-neighbors
- **Dissipation** (Rayleigh): -alpha * J removes ~alpha * |J|^2 from the same 6 neighbors plus the particle site itself

The near-field self-field equilibrates when injection ~ dissipation locally. But energy that has **propagated beyond the 1-hop damping radius** escapes into the lossless vacuum. This is a net energy leak from every manifested particle into the vacuum.

## 3.2 The Cosmological Constant Mechanism [SELECTION — superseded by FTD-0331]

The net energy leak per particle per tick represents **vacuum energy injection** -- the dynamical origin of the cosmological constant. Summed over all manifested particles in the universe, this continuous energy injection drives the accelerating expansion.

> **Per FTD-0331 (governing):** matter-injection has **no L-dependence** ⇒ it is
> **not a native Λ source**; the source is **[OPEN]** and FTD predicts **Λ=0**. The
> "dynamical origin of the cosmological constant / drives the accelerating expansion"
> reading above is superseded.

The rate is suppressed by:

- alpha per coupling event (from g_c^2 = alpha)
- The geometric fraction of injected energy that escapes beyond 1-hop before being damped

This connects to DERIV_COSMOLOGICAL_CONSTANT.md: the alpha^16 factor counts 16 physical degrees of freedom, each coupling to gravity with strength alpha. The dynamical interpretation is that each DOF contributes one channel through which coupling-injected energy leaks into the gravitational vacuum.

## 3.3 Reconciling alpha^16 and alpha^57 [PARAMETRIC]

Two formulas appear in FTD documents:

1. **DERIV_COSMOLOGICAL_CONSTANT.md:** rho_Lambda = m_e^4 * alpha^16 * G*^2
2. **10.3-dark-energy.qmd:** Lambda/Lambda_Planck ~ alpha^57

These are **consistent, not contradictory**. When m_e is expressed in Planck units via m_e = M_P * sqrt(2pi) * (16/3) * alpha^11 (DERIV_ELECTRON_MASS.md):

    m_e^4 * alpha^16 = M_P^4 * (2pi)^2 * (16/3)^4 * alpha^(44+16) = M_P^4 * C_pf * alpha^60

where C_pf = (2pi)^2 * (16/3)^4 * G*^2 ~ 2.2 x 10^5.

In logarithmic terms: log_{1/alpha}(Lambda/Lambda_P) = 60 - log_{1/alpha}(8pi * C_pf) ~ 60 - 2.5 = 57.5

The alpha^57 formula is thus a **rounded logarithmic approximation** of the precise alpha^16 derivation. The canonical formula is rho_Lambda = m_e^4 * alpha^16 * G*^2; the alpha^57 version is a useful mnemonic but should not be cited as an independent derivation.

---

# S4 -- Dark Matter = Self-Field Halo Tail

## 4.1 The Self-Field Envelope [MEASURED — INDETERMINATE; SPARC boundary] (FTD-0300)

Every manifested particle builds a self-field envelope through the coupling source. The GPU GP-KCOMP-SHELL test at 128^3 measured:

| Property | Value | Source |
|----------|-------|--------|
| r_eff (flux-weighted RMS) | 15.03 voxels | GP-KCOMP-SHELL Part A |
| 1% boundary | 23 voxels | Shell radius where <|J|> < 0.01 * J(r=1) |
| Self-field energy | 0.021 (= 0.08 * K_B^2) | sum |J|^2 over envelope |
| Peak flux | 0.0316 | J_peak at particle site neighbors |
| Power-law exponent | -0.69 | log-log fit for r >= 7 |

> **FTD-0300 (2026-06-13) — the -0.69 above is falsified as a forced value.** A
> pre-registered GPU L-grid {64,96,128,160} audit (`preregister-halo-forcedness-v1`)
> found -0.69 is the L=64 transient; the windowed exponent **converges to -1.25** at
> L >= 128, and the lossless (selective-ON) self-field **box-fills** the periodic
> lattice (`r_eff ~ L/2`, not a localized object). Frozen verdict **INDETERMINATE**
> (box-fill => not a forced localized halo; convergent => not a simple drift artifact);
> the only forced, localized self-field is the damped (selective-OFF) Coulomb near-field
> (~ -2.15). Consequence: the dark-matter halo is not a forced localized shape, so the
> SPARC rotation-curve target is not founded. See
> [`ANALYSIS_HALO_FORCEDNESS_v1.md`](../../10_eft_program/ANALYSIS_HALO_FORCEDNESS_v1.md).

## 4.2 Dark Matter Properties [SELECTION]

All lattice sites with r > 0 from the manifested seed have s = 0 (void). Their flux carries:

- **Density:** rho(x) = |J(x)| > 0 -- gravitationally active
- **No charge:** s = 0 implies Coulomb force F = -alpha * s * grad(phi_C) = 0
- **No color:** void sites carry no color charge
- **Stability:** with selective damping, far-field flux persists indefinitely (verified by DS-1)
- **Collisionless:** flux waves superpose linearly (verified by GP4 superposition test)

These properties match every observational requirement for dark matter:

| Requirement | FTD Mechanism |
|-------------|---------------|
| Gravitates | F_grav = G_N * grad(rho) where rho = |J| |
| Non-luminous | s = 0: no EM coupling |
| Collisionless | Linear wave superposition |
| Cold (non-relativistic) | Self-field propagates at c = 1/sqrt(3) but envelope is stationary |
| Stable | Selective damping: no decay channel in vacuum |
| No direct detection | Sub-threshold: |J| < K_B at halo distances |

## 4.3 The Halo Structure [CONJECTURE]

A single particle's self-field halo extends to ~23 voxels. In a cluster of N particles, the overlapping halos create a composite density field:

    rho_halo(r) = sum_i rho_i(|x - x_i|)

For a compact cluster, this produces a density profile that falls off **slower** than the individual particle profile at large r, because distant points see contributions from multiple particles. This is the FTD analog of a dark matter halo -- a gravitationally active but non-luminous envelope around luminous matter.

The rotation curve analog (DS-7) tests whether this composite halo produces a flatter-than-Keplerian radial acceleration profile.

---

# S5 -- Particles as Localized Phase Transitions

## 5.1 Genesis: The Phase Transition [THEOREM]

The genesis rule: a void site with |J| > K_GENESIS = 3 * K_B = 1.533 undergoes **spontaneous symmetry breaking**: the ternary state field transitions from s = 0 to s = +/-1. The polarity is determined by the local chirality (divergence of flux for single-substrate, chirality density for dual-substrate).

The probability of manifestation follows the Born rule:

    p = 1 - exp(-(|J| - K_GENESIS) / K_B)

This is the FTD formalization of wavefunction "collapse" -- not a mysterious process but a deterministic threshold crossing with stochastic polarity assignment.

## 5.2 The Dressed Particle [SELECTION]

Once manifested, the particle:

1. **Sources its own self-field** via the coupling term (g_c * grad(s))
2. **Builds an envelope** that extends ~15 voxels (r_eff)
3. **Interacts through the envelope**: Coulomb force from grad(phi_C) (Poisson equation sourced by div(J)), gravity from grad(rho) where rho = |J|
4. **Carries mass** K_B = m_e from the Born-Infeld rest energy, plus electromagnetic self-energy from the envelope

The particle IS the seed plus its envelope. What particle physics calls "an electron" is: {state: -1, flux: isotropic K_B envelope, particle_id: n}. The "bare" seed (s = +/-1 at one site) has no physical meaning without its dressing -- the coupling term ensures the envelope forms immediately.

---

# S6 -- The Omega_Lambda Collision [PARAMETRIC] / [BOUNDARY]

## 6.1 The Three Uncoordinated Values

**FTD has NO derived Ω_Λ.** Three uncoordinated numbers sit near the observed
0.685; their proximity is **coincidental**, and **none** may be cited as a
dark-energy density prediction:

| Source | Value | Origin | Status |
|--------|-------|--------|--------|
| DERIV_COSMOLOGICAL_CONSTANT.md S4.3 | 0.683 | rho_Lambda / rho_crit with the alpha^16 value-match | **[PARAMETRIC]** (FTD-0331; no L-dependence) |
| Engine constant OMEGA_LAMBDA_CONJ | 2/3 = 0.667 | Dual-substrate decomposition | **[CONJECTURE]** (engine round-number) |
| Moore dark-STATE count (Moore Layer Theorem) | 17/27 ≈ 0.63 | Hilbert-space dark-state count | **[THEOREM]** (a STATE count only) |
| Planck 2018 + BAO | 0.685 +/- 0.007 | Observation | -- |

## 6.2 The Canonical Position (per FTD-0331)

The 0.683 reading is a **[PARAMETRIC]** value-match downstream of the
rho_Lambda = m_e^4 * alpha^16 * G*^2 numerology (and an imported rho_crit via
H_0); it carries **no L-dependence**. The 2/3 engine constant is a
**[CONJECTURE]** round-number from the dual-substrate decomposition. The
17/27 ≈ 0.63 Moore reading is a **[THEOREM]** as a *dark-STATE count* only —
a Hilbert-space state count, **category-distinct** from an energy-density Ω_Λ,
and it **must NOT** be cited as a dark-energy fraction.

These three numbers are **uncoordinated** and mutually inconsistent; their
proximity to the observed 0.685 is coincidental, not a coordinated FTD output.
Per **FTD-0331** the Ω_Λ **VALUE is a [BOUNDARY]** (it needs L_H/ℓ_P, which
FTD-0059 proves is not native — no Axiom-Zero length). No "0.683 supersedes 2/3"
resolution is asserted.

---

# S7 -- Testable Predictions

| ID | Prediction | Engine Test | Status |
|----|-----------|-------------|--------|
| DS-P1 | Sub-threshold flux persists indefinitely under selective damping | DS-1, DS-2 | [THEOREM] |
| DS-P2 | Locked particle produces measurable net energy injection rate ~alpha*K_B^2 per tick | DS-3 | [THEOREM] |
| DS-P3 | Far-field self-field tail generates nonzero gravitational force on distant test particle | DS-4 | [SELECTION] |
| DS-P4 | Self-field halo: rho(r) > 0, monotonically decreasing, all sites void for r > 0 | DS-5 | [THEOREM] |
| DS-P5 | Multi-particle halo produces flatter-than-Keplerian radial acceleration | DS-7 | [CONJECTURE] |
| DS-P6 | alpha^16 and alpha^57 formulas are algebraically consistent | DS-8 | [PARAMETRIC] |

---

# S8 -- Claims Table

| ID | Claim | Status | Key Equation / Evidence |
|----|-------|--------|------------------------|
| DSD-1 | Coupling term injects energy into particle neighborhood each tick | **[THEOREM]** | delta_J = g_c * grad(s) from Lagrangian EL equations |
| DSD-2 | Injection rate is O(alpha) per particle per tick | **[THEOREM]** | dE/dt = g_c^2 * |grad s|^2 = alpha * O(1) |
| DSD-3 | Selective damping: far-field flux propagates losslessly | **[THEOREM]** | should_damp = !selective or near_particle_[i] |
| DSD-4 | Net energy leak into vacuum = dark energy mechanism | **[SELECTION — superseded by FTD-0331]** | Injection - dissipation at 1-hop boundary; no L-dependence ⇒ not a native Λ source; source [OPEN] |
| DSD-5 | alpha^16 describes 16 DOF mode-coupling suppression | **[SELECTION]** | DERIV_COSMOLOGICAL_CONSTANT.md S3.3 |
| DSD-6 | alpha^57 ~ alpha^60 / prefactors (logarithmic approximation) | **[PARAMETRIC]** | Re-spells a [PARAMETRIC] value-match; a substitution identity is not theorem-grade (FTD-0331) |
| DSD-7 | Self-field halo tail is dark matter | **[SELECTION]** | s = 0, rho > 0, gravitates, no EM |
| DSD-8 | Dark matter is stable (selective damping) | **[SELECTION]** | DS-1 test: energy ratio > 0.95 after 500 ticks |
| DSD-9 | Multi-particle halo flattens rotation curve | **[CONJECTURE]** | DS-7 test |
| DSD-10 | Omega_Lambda: 0.683 / 2/3 / 17÷27 are uncoordinated readings; FTD has NO derived Ω_Λ | **[PARAMETRIC] / [BOUNDARY]** | FTD-0331: 0.683 [PARAMETRIC], 2/3 [CONJECTURE], 17/27 [THEOREM] state-count only; value [BOUNDARY] (needs L_H) |

**Epistemic breakdown (post FTD-0331 reconciliation):** 3 [THEOREM], 3 [SELECTION] + 1 [SELECTION — superseded], 2 [PARAMETRIC]/[BOUNDARY] (DSD-6, DSD-10), 1 [CONJECTURE]

---

# S9 -- Cross-References

| Document | Relevant Content |
|----------|-----------------|
| [DERIV_COSMOLOGICAL_CONSTANT.md](../04_coupling/DERIV_COSMOLOGICAL_CONSTANT.md) | rho_Lambda formula, 16 DOF counting, Omega_Lambda derivation |
| [SPEC_FTD_LAGRANGIAN.md](../01_reference/SPEC_FTD_LAGRANGIAN.md) | Coupling term L-7, Rayleigh dissipation L-8 |
| [SPEC_ENGINE.md](../../../engine/SPEC_ENGINE.md) | Selective damping, tick cycle, phase structure |
| [DERIV_FORCE_EMERGENCE.md](../foundational_mechanics/DERIV_FORCE_EMERGENCE.md) | Gravitational force from density gradient |
| campaign_dark_sector.cpp | 8 verification tests (DS-1 through DS-8) |

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-03-17 | Initial: coupling injection, selective damping, dark sector unification |

---

*Document Version 1.0 -- March 17, 2026*
*Framework: Foundational Ternary Dynamics v5.28*
