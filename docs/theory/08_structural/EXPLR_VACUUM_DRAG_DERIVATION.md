# Vacuum Drag: Toward Deriving gamma = alpha

**Version:** 1.0
**Date:** February 10, 2026
**Status:** [CONJECTURE] -- Proposed derivation path for the dissipation rate
**Epistemic Tag:** [CONJECTURE]

> The dissipation rate gamma = alpha is currently the most epistemically unsatisfying parameter in FTD. It is imposed (ASSUMP.6), not derived. This document proposes a geometric mechanism -- vacuum drag -- that could close the gap.

---

## 1. The Problem

FTD's action principle includes a dissipation term with rate gamma (CLAUDE.md, Section 4.3):

```
flux(v,t+1) *= (1 - gamma)
```

The identification gamma = alpha = 0.00729... is marked [IMPOSED] in the Assumption Ledger. The motivation is that electromagnetic coupling governs irreversible transitions, but this is circular reasoning -- *why* should the dissipation rate equal the electromagnetic coupling?

We need a derivation of the form:

> gamma = f(lattice geometry, G*, {3,4,7,13})

that produces alpha without presupposing it.

---

## 2. Geometric Friction: The Concept

Consider a manifested voxel (state +/- 1) propagating through the void lattice. The flux field carried by the particle must "negotiate" the discrete geometry at each step. This negotiation is not free -- it costs energy proportional to the geometric mismatch between:

1. **The continuous flux wave** (a smooth, rotationally symmetric field)
2. **The discrete lattice** (a grid with finite coordination and specific symmetry)

This mismatch is the **vacuum drag**: the fraction of energy lost per tick due to projecting continuous potential onto discrete actuality.

### The Analogy

A ball rolling on a perfectly smooth surface experiences no friction. A ball rolling on a tiled floor loses energy at each tile edge. The energy loss per tile depends on:
- The ball's coupling to the surface (how much it "feels" the gaps)
- The tile geometry (how often and how severe the gaps are)

In FTD:
- The "ball" is the manifested particle's flux field
- The "tiles" are the lattice voxels
- The "gaps" are the points where the discrete Laplacian departs from the continuum Laplacian
- The coupling is the state-flux coupling g_c ~ sqrt(alpha)

---

## 3. Isotropy Error as Friction Source

The discrete Laplacian on a lattice departs from the continuum Laplacian by an **isotropy error** epsilon that depends on the lattice geometry:

| Lattice | Coordination | Isotropy Error epsilon |
|---------|-------------|----------------------|
| Simple cubic (Von Neumann, 6-point) | 6 | ~15% |
| Simple cubic (Moore, 26-point) | 26 | ~8% |
| FCC (cuboctahedral, 12-point) | 12 | ~3% |
| Ideal continuum | infinity | 0% |

The isotropy error measures how much the discrete wave operator deviates from perfect spherical symmetry. This deviation creates directional artifacts in flux propagation, which manifest as energy loss (the "missing" energy goes into lattice heating -- sub-resolution excitations that cannot be tracked).

### The Dissipation Formula (Proposed)

If dissipation arises from isotropy error, then:

```
gamma ~ epsilon * g_c^2
```

where g_c ~ sqrt(alpha) is the state-flux coupling. For FCC:

```
gamma ~ 0.03 * alpha ~ 2.2e-4
```

This is too small by a factor of ~30. The simple product doesn't work.

### Alternative: Packing Fraction

The FCC lattice has packing fraction eta = pi/(3*sqrt(2)) = 0.7405. The void fraction is 1 - eta = 0.2595. A particle traversing the lattice encounters void (information-losing) gaps at rate proportional to the void fraction:

```
gamma ~ (1 - eta) * g_c^2 ~ 0.26 * 0.00729 ~ 0.0019
```

Still not alpha. Closer, but off by a factor of ~4.

---

## 4. The AGM Approach

The Arithmetic-Geometric Mean (AGM) connects to the lemniscatic constant via:

```
G* = Gamma(1/4) / Gamma(3/4) = 2.9587...
[equivalently Gamma(1/4)^2/(sqrt(2)*Gamma(1/2)^2) = 2*varpi/sqrt(pi) = sqrt(2)*Gamma(1/4)^2/(2*pi)]
```

where varpi = pi / AGM(1, sqrt(2)) is the lemniscate constant. Gauss's constant is:

```
G = 1/AGM(1, sqrt(2)) = 2*varpi/pi = 0.8346...
```

**Key observation:** G represents the convergence rate of the AGM iteration. The AGM starts with two values (a_0 = 1, g_0 = sqrt(2)) and iterates:

```
a_{n+1} = (a_n + g_n) / 2       (arithmetic mean)
g_{n+1} = sqrt(a_n * g_n)       (geometric mean)
```

converging to M = AGM(1, sqrt(2)) = 1.1981... in just ~4 iterations to machine precision.

### The Resolution Limit Interpretation

The AGM iteration represents the process of reconciling two scales:
- The arithmetic mean (additive, linear, macro-scale)
- The geometric mean (multiplicative, logarithmic, micro-scale)

Gauss's constant G = 1/M is the **resolution ratio** between these two scales at convergence. It encodes how much information is lost when the continuous (arithmetic) and discrete (geometric) descriptions are forced to agree.

### Proposed Connection

If vacuum drag is the cost of reconciling continuous flux with discrete states, then:

```
gamma = f(G, alpha, lattice geometry)
```

One candidate:

```
gamma = alpha * G / G* = alpha * (2*varpi/pi) / (2*varpi*sqrt(2)/pi) = alpha / sqrt(2)
```

This gives gamma = 0.00515, which is 29% below alpha. Not exact, but the structure is suggestive.

Another candidate using the Oh symmetry group order:

```
gamma = alpha * (|Oh| / |Oh|) = alpha
```

This is trivially true and unhelpful. The challenge is finding a non-trivial geometric factor that evaluates to exactly 1.

---

## 5. The Self-Consistency Argument

The most promising route may be self-consistency. Suppose gamma is *not* a free parameter but is determined by requiring that:

1. The dissipation rate preserves the action principle's variational structure
2. The steady-state energy balance between flux injection (manifestation) and flux dissipation (decay) reproduces the correct particle lifetimes
3. The thermal equilibrium of the lattice is consistent with the effective temperature implied by alpha

If the lattice thermal equilibrium constrains gamma, and the thermal fluctuation spectrum is determined by the lattice geometry (cuboctahedral vs cubic), then gamma becomes a derived quantity.

### Sketch of the Argument

In thermal equilibrium, the energy per degree of freedom is k_B*T / 2. The lattice has N_eff = 13 effective degrees of freedom per coordination shell. The total thermal energy per shell is 13*k_B*T / 2.

If the thermal energy equals the electromagnetic coupling energy (alpha per interaction), then:

```
13 * k_B * T / 2 = alpha * E_interaction
```

The dissipation rate that maintains this balance is:

```
gamma = alpha * (2 / N_eff) * f(geometry)
```

For f(geometry) = N_eff / 2 (self-consistent), this gives gamma = alpha. But this is again circular -- we assumed the answer.

---

## 6. Testable Prediction

Regardless of the exact derivation, vacuum drag makes a specific testable prediction:

> **If dissipation arises from lattice geometry, then switching from cubic to FCC lattice should change the effective dissipation rate.**

Specifically:
- Cubic lattice: gamma_cubic = alpha * f_cubic
- FCC lattice: gamma_FCC = alpha * f_FCC

where f_cubic != f_FCC because the isotropy errors differ (15% vs 3%).

**Simulation test:** Run identical initial conditions (a single particle decaying in vacuum) on both cubic and FCC lattices. Measure the effective decay rate. If gamma is geometry-dependent, the rates will differ. If gamma is truly = alpha independent of geometry, the rates will match.

This test is straightforward to implement once the cuboctahedral lattice code exists (Phase 3 of the implementation plan).

---

## 7. What This Would Resolve

If a vacuum drag derivation succeeds:

| Before | After |
|--------|-------|
| gamma = alpha is [IMPOSED] (ASSUMP.6) | gamma is [DERIVED] from lattice geometry |
| Dissipation is phenomenological | Dissipation has geometric origin |
| "Why does decay rate equal coupling constant?" is unanswered | Decay rate = coupling constant because both measure the same thing: geometric mismatch between continuous potential and discrete actuality |
| ASSUMP.6 remains in Assumption Ledger | ASSUMP.6 becomes a theorem |

---

## 8. Honest Assessment

The derivation attempts in Sections 3-5 all fall short. They produce expressions *near* alpha but not *equal* to alpha without circular reasoning. The self-consistency argument is suggestive but not rigorous.

**What we have:** A compelling physical picture (vacuum drag), a clear testable prediction (geometry-dependent dissipation), and several plausible but incomplete derivation paths.

**What we lack:** A clean, non-circular derivation of gamma = alpha from lattice geometry alone.

**Status:** [CONJECTURE]. The vacuum drag mechanism is proposed, not proven.

---

## References

- CLAUDE.md, Section 4.3 (Decay), ASSUMP.6 (Dissipation rate imposed)
- DERIV_ALPHA_PRECISION_FORMULA.md (G* derivation)
- EXPLR_CUBOCTAHEDRAL_GEOMETRY.md (Lattice geometry and isotropy)
- EXPLR_LOOP_GRID_DUALITY.md (Continuous vs discrete reconciliation)
