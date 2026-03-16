# FTD Symbol Glossary

This document provides a comprehensive reference for all mathematical symbols used in Foundational Ternary Dynamics.

## Fundamental Constants

| Symbol | Name | Value | Definition |
|--------|------|-------|------------|
| G* | Lemniscatic constant | 2.9586751... | √2 × Γ(1/4)² / (2π) |
| α | Fine structure constant | 1/137.036 | Electromagnetic coupling |
| c | Speed of light | 1 (natural units) | Maximum propagation speed |
| ℏ | Reduced Planck constant | 1 (natural units) | Quantum of action |
| G_N | Newton's constant | ~6.7×10⁻³⁹ (nat.) | Gravitational coupling |

## Framework Integers

| Symbol | Name | Value | Origin |
|--------|------|-------|--------|
| N_c | Color charges | 3 | First FLT-forbidden exponent |
| N_base | Base dimension | 4 | Second FLT-forbidden exponent |
| b₃ | QCD beta coefficient | 7 | N_c + N_base |
| n_eff | Effective dimension | 13 | b₃ + 2×N_c = F₇ |

## Lattice Variables

| Symbol | Type | Domain | Description |
|--------|------|--------|-------------|
| v | Point | L ⊂ Z³ | Voxel position |
| s(v,t) | Function | {-1, 0, +1} | Ternary state |
| J(v,t) | Vector | R³ | Flux field |
| ρ(v,t) | Scalar | R⁺ | Flux density = |J| |
| t | Counter | N | Tick (discrete time) |

## Operators

| Symbol | Name | Definition |
|--------|------|------------|
| ∇f | Gradient | (∂f/∂x, ∂f/∂y, ∂f/∂z) |
| ∇·J | Divergence | ∂Jx/∂x + ∂Jy/∂y + ∂Jz/∂z |
| ∇×J | Curl | (∂Jz/∂y - ∂Jy/∂z, ...) |
| ∇²f | Laplacian | ∂²f/∂x² + ∂²f/∂y² + ∂²f/∂z² |

## Quantum Mechanics

| Symbol | Name | Description |
|--------|------|-------------|
| ψ | Wave function | Complexified flux: Jx + iJy |
| H_FTD | Hilbert space | L²(Lattice, C) |
| ρ | Density matrix | |ψ⟩⟨ψ| or Σ pᵢ|ψᵢ⟩⟨ψᵢ| |
| S_vN | von Neumann entropy | -Tr(ρ ln ρ) |
| ⟨·|·⟩ | Inner product | Σᵥ ψ*(v)φ(v) |

## Coupling Constants

| Symbol | Name | FTD Formula | Value |
|--------|------|-------------|-------|
| α | Fine structure | 1/x₊ | 1/137.036 |
| sin²θ_W | Weak mixing | N_c/n_eff | 3/13 = 0.2308 |
| α_s | Strong coupling | b₃/(b₃ + 4n_eff) | 7/59 = 0.1186 |
| g_c | State-flux coupling | ~√α | ~0.085 |

## Particle Physics

| Symbol | Name | Description |
|--------|------|-------------|
| m_e | Electron mass | m_P √(2π) (16/3) α¹¹ |
| m_P | Planck mass | √(ℏc/G_N) |
| v | Higgs VEV | 246 GeV |
| λ | Cabibbo angle | √(2 sin²θ_W α_s) |

## Mixing Matrices

### CKM Matrix Elements
| Symbol | Formula | Exp. Value |
|--------|---------|------------|
| V_ud | 1 - λ²/2 | 0.974 |
| V_us | λ | 0.226 |
| δ_CKM | arctan(b₃/N_c) | 68° |

### PMNS Angles
| Symbol | Name | FTD Value | Exp. |
|--------|------|-----------|------|
| θ₁₂ | Solar | 33.1° | 33.4° |
| θ₂₃ | Atmospheric | 46.2° | 45.0° |
| θ₁₃ | Reactor | 8.5° | 8.6° |

## Cosmology

| Symbol | Name | Description |
|--------|------|-------------|
| n_s | Spectral index | Scalar perturbation tilt |
| r | Tensor-to-scalar | Gravitational wave amplitude |
| η | Baryon asymmetry | (n_B - n_B̄)/n_γ |
| N_e | E-foldings | Inflation expansion factor |
| Ω | Density parameter | ρ/ρ_crit |

## Information Theory

| Symbol | Name | Formula |
|--------|------|---------|
| H | Shannon entropy | -Σ pᵢ log pᵢ |
| S | Boltzmann entropy | k_B ln Ω |
| S_vN | von Neumann entropy | -Tr(ρ ln ρ) |
| I | Mutual information | H(A) + H(B) - H(AB) |

## Greek Letters Summary

| Letter | Primary Use | Secondary Use |
|--------|-------------|---------------|
| α | Fine structure constant | QED coupling |
| β | Beta function coefficient | Inverse temperature |
| γ | Decay rate | Lorentz factor |
| δ | CP phase | Kronecker delta |
| ε | Small parameter | Levi-Civita symbol |
| θ | Mixing angles | Angular coordinate |
| λ | Cabibbo parameter | Wavelength |
| ρ | Density | Charge density |
| σ | Cross section | Pauli matrices |
| τ | Tau lepton | Lifetime |
| φ | Phase | Golden ratio |
| ψ | Wave function | Flux combination |
| Ω | Microstate count | Density parameter |

## Disambiguation

To avoid confusion:

| Ambiguous | Clarification |
|-----------|---------------|
| G | G* (lemniscatic) vs G_N (Newton) |
| g | g_c (state-flux) vs g_s (strong) vs g_μν (metric) |
| ρ | ρ (flux density) vs ρ (charge density) vs ρ (density matrix) |
| S | S (entropy) vs S (action) vs S (Bell parameter) |
| α | α (fine structure) vs α (phase angle) |

Context should disambiguate; when unclear, use subscripts.

## Natural Units Convention

FTD uses natural units where:
- c = 1 (speed of light)
- ℏ = 1 (reduced Planck constant)
- k_B = 1 (Boltzmann constant)

Planck units:
- Length: ℓ_P = √(ℏG/c³) = 1 voxel
- Time: t_P = √(ℏG/c⁵) = 1 tick
- Mass: m_P = √(ℏc/G) = 1 (energy unit)
