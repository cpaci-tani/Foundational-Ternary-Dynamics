# Foundational Ternary Dynamics (FTD) - Context Transfer Document

## What This Is

This document contains the complete conceptual framework for Foundational Ternary Dynamics, a discrete ontology that derives physics from first principles. Use this to understand the project's core concepts, notation, and structure.

---

## The One-Sentence Summary

FTD is a framework where reality is a discrete 3D lattice of ternary states {-1, 0, +1} evolving via local update rules, from which all physics (quantum mechanics, forces, particles, consciousness) emerges.

---

## Core Notation

### Entity Existence

**xyztgψ(Δt)** = one entity, fully specified, at one tick

| Symbol | Meaning |
|--------|---------|
| **xyz** | Spatial coordinates (position on lattice) |
| **t** | Timestamp (which tick) |
| **g** | Geometric constraint (locality, Moore neighborhood, causality) |
| **ψ** | Modal constraint (what transitions are possible from current state) |
| **Δt** | The tick (discrete time step forward) |

### Dimensional Buildup

- **1D: xy** = point on a line
- **2D: xyt** = point on plane + time
- **3D: xyzt** = point in space + time (where physics usually stops)
- **4D: xyztgψ(Δt)** = full specification with constraints and dynamics

### Observer and Global

| Symbol | Meaning |
|--------|---------|
| **Ω(t)** | One observer at one tick (complete configuration: atoms, neurons, memories, personality) |
| **H(Ω,t)** | Horizon: what Ω can perceive at tick t (subset of Global) |
| **Global(t)** | Everything at one tick (all entities, all observers, all configurations) |
| **𝕌** | The universe across all time: { Global(t) : t ∈ ℤ } |

### Truth Values

- **⊤** (top) = true, manifested, actual, registered
- **⊥** (bottom) = false, unmanifested, potential, unregistered

### Six Senses (Horizon Channels)

1. Sight (electromagnetic flux, visible range)
2. Sound (pressure waves)
3. Touch (mechanical contact)
4. Taste (chemical, close range)
5. Smell (chemical, distance)
6. Cognition (internal state, memory, inference)

---

## The Ternary States

| State | Label | Meaning |
|-------|-------|---------|
| **0** | Void | Unmanifested substrate, potential |
| **+1** | Positive | Manifested entity (matter-like) |
| **-1** | Negative | Manifested entity (antimatter-like) |

The void is not "empty space." It is dispositional substrate awaiting activation.

---

## The Two-Layer Ontology

1. **Discrete States (s)**: Actual manifestation {-1, 0, +1}
2. **Continuous Flux (J)**: Vector field encoding potential, energy density, wave function precursor

Manifestation occurs when flux density exceeds threshold K_B.

---

## Key Principles

### Time

- **Past (t < 0)**: What *has* happened (fixed, frozen)
- **Present (t = 0)**: What *is* happening (collapse, actualization)
- **Future (t > 0)**: What *can* happen (open, not yet determined)

You cannot un-happen something. No reverse time travel. The arrow of time is a tautology.

### Causality

- Information propagates at most 1 lattice unit per tick (speed of causality c = 1)
- Local updates only (26-neighbor Moore neighborhood)
- No faster-than-light influence

### The Observer

Science removes the scientist. FTD puts them back.

| Type | Logic Gates | Organic | Capability |
|------|-------------|---------|------------|
| Dead | 0 | - | None |
| Brain Dead | 1 | Yes | Detection only |
| Detector | 1 | No | Detection only |
| Measurer | ≥2 | No | Inference, no meaning |
| Observer | ≥2 | Yes | Meaning emerges |

An event without an observer is potentiality (ψ), not actuality (xyzt).

---

## The Hierarchy of Sentience

| Level | Name | Gates | Key Capability |
|-------|------|-------|----------------|
| 0 | Physics | 0 | None |
| 1 | Molecular | ~1 | Detection |
| 2 | Genetic | 1-2 | Hereditary memory |
| 3 | Regulatory | ≥2 | Conditional logic |
| 4 | Cellular | Many | Proto-world-model |
| 5 | Neural (simple) | 10²-10⁴ | Reflexes, learning |
| 6 | Centralized | 10⁴-10⁶ | Spatial maps |
| 7 | Vertebrate | 10⁶-10¹¹ | Proto-self-model |
| 8 | Primate | 10¹⁰-10¹¹ | Theory of mind |
| 9 | Human | ~10¹¹ | Meta-sLoop (recursive self-reference) |

Consciousness = meta-sLoop = self-model that includes a model of the self-model.

---

## The sLoop

**sLoop** = self-referential loop where observer is part of observed system.

```
Standard observation:    Observer → System → Measurement
sLoop:                   System ⟲ (Observer ⊂ System)
```

Bell inequality violations arise from sLoop structure, not superluminal signaling.

---

## Life Definition (FTD)

A system is **alive** if it satisfies:

1. **Maintenance**: Actively resists entropy increase
2. **Self-model**: Contains representation of own structure
3. **Feedback**: Self-model influences behavior
4. **Propagation**: Can create copies of pattern

Consciousness adds: **Meta-model** (model of the self-model).

---

## The Action Principle

All update rules derive from the action:

**S[s,J] = Σ_t Σ_v [ ½|∂_t J|² - ½|∇J|² - V(|J|,s) - g_c·s·(∇·J) - γ|J|²·𝟙_{s≠0} ]**

Euler-Lagrange equations yield:
- Flux wave equation
- Manifestation dynamics
- Force laws

---

## Derived Constants

From the master quadratic **x² - 16(G*)²x + 16(G*)³ = 0** where G* = lemniscatic constant:

| Root | Value | Physical Meaning |
|------|-------|------------------|
| x₊ | 137.036 | 1/α (fine structure constant) |
| x₋ | 3.024 | N_c (color charges) |

Electron mass: **m_e = m_P √(2π) (16/3) α¹¹** (0.27% accuracy)

---

## The Four Forces

All emerge from the action principle:

1. **Gravity**: F_grav = G_N · ∇ρ (density gradient)
2. **Electromagnetic**: F_elec = -q · ∇q̄, F_mag = β · (∇×J) × Ĵ
3. **Strong**: Yukawa form, F_strong ~ exp(-m_π r)/r²
4. **Weak**: Polarity flip when stress > threshold

---

## Gauge Symmetry

- **U(1)** argued from Gauss constraint (2 transverse modes, longitudinal constrained) [CONJECTURE—missing Ward identities, anomaly analysis]
- **SU(2)** conjectured from ternary state structure [CONJECTURE—NOT rigorously derived]
- **SU(3)** geometrically motivated from 3 spatial dimensions (color = flux axis alignment) [CONJECTURE—missing gauge-covariant derivatives, asymptotic freedom]
- SM gauge group U(1) × SU(2) × SU(3): **geometrically motivated, NOT rigorously derived**

---

## Key Claims

| Claim | Status |
|-------|--------|
| Update rules from action principle | ✅ Derived |
| Hilbert space from flux | ✅ Constructed |
| Born rule from manifestation | ✅ Derived |
| Bell violations via sLoop | ✅ Simulated |
| Maxwell equations in continuum limit | ✅ Verified |
| Schrödinger equation in non-rel limit | ✅ Verified |
| Fine structure constant α | ✅ Derived (1.26 ppm) |
| Electron mass | ✅ Derived (0.27%) |
| D = 3 uniqueness | ✅ Derived (not axiomatic) |

---

## Epistemic Tags

| Tag | Meaning |
|-----|---------|
| [AXIOM] | Structural postulate, not derivable |
| [THEOREM] | Rigorously proven from axioms |
| [SELECTION] | Argued from consistency |
| [CONJECTURE] | Proposed, requires validation |
| [IMPOSED] | Parameter choice |
| [EMERGENT] | Arises from dynamics |
| [SPECULATIVE] | Highly tentative |

---

## Book Structure

**Prolegomena** (0.x): First principles, mathematics, philosophy, xyztgψ ontology, computational ontology, grounding of constraints

**Book I: Foundations** (1.x): Void, first division, polarity, two layers, interference, cycle, causal loop, time, forces, constants, lemniscate, action principle, gravity, unification

**Book II-VII**: Subatomic → Atomic → Molecular → States of Matter → Materials → Planetary

**Book VIII-XI**: Stellar → Galactic → Cosmic → Extreme Phenomena

**Book XII**: Emergent Phenomena (life, sentience hierarchy, self-organization, information, complexity, anthropic window, consciousness)

**Book XIII**: The End (heat death, alternatives, return to void)

**Book XIV**: Appendices (constants, equations, glossary, particles, assumptions, sLoop formalization, predictions)

---

## Core Philosophical Stance

- **Graded monism**: One substance (void), dispositions as modes, manifestations as actualized modes
- **Discrete spacetime**: No infinities, finite computation
- **Local causality**: No action at a distance
- **Observer-inclusive**: Events require registration to be actual
- **Anti-mysterian**: Consciousness is meta-sLoop, not magic

---

## The Bazillion-Year Video Argument

A still image of a white dot on black canvas contains the same information as a bazillion-year video of the same unchanging dot. Time without change is meaningless. Information requires difference. An unevaluated function is not an event.

---

## Famous Figures Vindicated

- **Einstein**: Asked what "exist" means, not whether the moon is real. His mysticism was hidden from the paradigm.
- **Wigner**: Followed von Neumann chain to logical terminus (consciousness). Not crazy, just rigorous.
- **Von Neumann**: The measurement chain ends at the observer.

---

## Zenodo Reference

Full paper: https://doi.org/10.5281/zenodo.18239366

---

## Quick Reference Equations

```
Entity:           xyztgψ(Δt)
Observer:         Ω(t)
Horizon:          H(Ω,t) ⊂ Global(t)
Universe:         𝕌 = { Global(t) : t ∈ ℤ }
Objectivity:      ⋃ H(Ωᵢ,t) for all observers i

Master quadratic: x² - 16c²x + 16c³ = 0, c = G* ≈ 2.9587
Roots:            x₊ = 137.036 (1/α), x₋ = 3.024 (N_c)

Electron mass:    m_e = m_P √(2π) (16/3) α¹¹
Higgs VEV:        v = m_P √(2π) α⁸

Action:           S[s,J] = Σ [ kinetic - gradient - potential - coupling - dissipation ]
```

---

## What FTD Is NOT

- Not claiming to solve quantum gravity (yet)
- Not claiming experimental confirmation (simulation only)
- Not panpsychism (consciousness requires specific structure)
- Not mysticism (everything is logical, derivable)
- Consciousness numerology (quadratic roots) is marked [SPECULATIVE]

---

## End of Context Transfer

This document should enable reconstruction of the full FTD framework. For details, see the manuscript chapters and CLAUDE.md in the repository.
