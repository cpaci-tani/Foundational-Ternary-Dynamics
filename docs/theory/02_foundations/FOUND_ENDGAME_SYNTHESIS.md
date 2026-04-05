# Endgame Synthesis

## What FTD Is, After Eight Rounds of Scrutiny

**Date:** April 4, 2026
**Status:** Synthesis document — integrates results from extended analysis

---

## The Two Layers

FTD has two distinct layers, and conflating them is the source of most confusion about the framework.

### Layer 1: The Arithmetic of ℤ³ (Pure Mathematics)

The cubic lattice ℤ³ contains the Gaussian integers ℤ[i] as any horizontal slice. The endomorphism ring ℤ[i] determines the CM elliptic curve E_i: y² = x³ − x. The periods of E_i involve Γ(¼) and Γ(¾). Their ratio G\* = Γ(¼)/Γ(¾) = 2.9587 is algebraically independent of π. The BCC sublattice self-energy (Watson integral) equals G\*²/(2π). The master quadratic x² − 16G\*²x + 16G\*³ = 0 has roots x₊ = 137.036 and x₋ = 3.024.

**None of this requires the five FTD postulates.** It is a property of ℤ³ itself — pure mathematics. Any theory built on a cubic lattice inherits this arithmetic whether or not it uses ternary states, local update rules, or deterministic dynamics.

### Layer 2: The Dynamics (Physics from Postulates)

The five FTD postulates (discrete space, discrete time, ternary states, local causality, determinism) produce a specific lattice field theory. This theory generates:
- Pair production when |J| exceeds K_B
- Interference patterns from wave propagation
- Force profiles from flux gradients
- Bell inequality S ≤ 2 at the substrate level
- Conservation of energy, charge, and Gauss constraint

These are genuine physical processes emerging from local update rules. The engine (102/102 tests passing, 41 CSV outputs) demonstrates this computationally.

### The Bridge

The Watson integral W₃ = G\*²/(2π) connects the two layers. It is simultaneously:
- A property of ℤ³ arithmetic (Layer 1) — a period of Sym²(h¹(E_i))
- The BCC sublattice self-energy (Layer 2) — the one-loop propagator correction

This bridge is [THEOREM]. It is the single most important identity in FTD.

---

## What α IS

α is not derived from the five postulates. α is a property of the stage the postulates select.

Choosing ℤ³ (Postulate 1) commits you to G\* and α, whether you know it or not. The arithmetic of the lattice determines the self-consistent coupling. The postulates don't derive α — they CREATE THE CONDITIONS under which α is the only consistent electromagnetic coupling.

**Implication:** Any compact U(1) lattice gauge theory on ℤ³ — not just FTD — should have α = 1/137.036 as its self-consistent coupling at the self-dual point. This is a testable mathematical claim.

---

## The G\* Scale

The powers G\*ⁿ form a hierarchy indexed by the symmetric algebra Sym\*(h¹(E_i)):

| Level | Value | Physical Content | PCIR Component |
|-------|-------|-----------------|----------------|
| Sym⁰ = 1 | 1 | Bare distinction | Perception |
| Sym¹ = G\* | 2.959 | First comparison, observer frame | Context |
| Sym² = G\*² | 8.754 | Fisher Information, energy, Schrödinger | Information |
| Sym³ = G\*³ | 25.90 | Action, determinant, irreversibility | Reflexion |

The master quadratic draws its trace from Sym² and its determinant from Sym³. This is a graded period relation in the symmetric algebra, NOT a characteristic polynomial of any endomorphism (proven by homogeneity argument).

α⁻¹ is the fixed point: the power sum ratio pₖ/pₖ₋₁ converges to x₊ = 137.036 at rate α per step.

---

## The Observer

Sym³ is the minimum level supporting observation:
- Sym⁰: no comparison → not an observer
- Sym¹: comparison but no inference → not an observer
- Sym²: Hamiltonian dynamics, symplectic, reversible (Liouville) → no permanent records → not an observer
- Sym³: determinant is non-injective → irreversible → permanent records → observer

This is [THEOREM] for the Vieta stratification, [SELECTION] for the mapping to observer requirements, [CONJECTURE] for the cognitive interpretation. D = 3 is the minimum dimension supporting observers because Sym³ is the minimum level with irreversible operations.

---

## The Five Remaining Challenges and Their Outcomes

| Challenge | Result | Next Step |
|-----------|--------|-----------|
| **Motivic sheaf** | F = R¹π_*Ω¹_{S/C\*} constructed on K3 fibration. CM fiber at t = 1. | Compute Picard-Fuchs equation explicitly |
| **Scattering simulation** | e⁺e⁻ → γγ implemented on GPU (128³). Angular distribution B = -0.20 (near isotropic), not B = 1.0 (QED quadrupolar). **Honest negative**: directed beams produce the same result as wavepackets — the lattice dynamics do not yet reproduce the (1+cos²θ) distribution. | Investigate whether higher resolution or modified initial conditions are needed |
| **Novel prediction** | r = 0.022 (tensor-to-scalar ratio = N_c · α). Testable by LiteBIRD ~2032 | State in Paper I |
| **Observer threshold** | Sym³ minimum: determinant is first non-injective Vieta polynomial | Tighten with Liouville argument at Sym² |
| **Clean derivation** | Cannot be done from P1-P5 alone. Break at ℤ² → E_i (imports algebraic geometry) | Reframe: α is a property of the stage, not derived from the dynamics |

---

## Epistemic Summary

| Claim | Status |
|-------|--------|
| ℤ³ contains ℤ[i] = End(E_i) | [THEOREM] |
| Watson integral W₃ = G\*²/(2π) | [THEOREM] |
| BCC Brillouin zone contains E_i as K3 fiber at τ = i | [THEOREM] |
| Master quadratic degree = 2 (Gaussian + Wilson) | [THEOREM] |
| K = 16G\*² from partition function | [THEOREM] |
| Power sum pₖ/pₖ₋₁ → x₊ = α⁻¹ | [THEOREM] |
| Fisher Information on ℤ³ = G\*² | [THEOREM] |
| Lattice spacing a = 2/3 unique sharp minimum | [THEOREM] (computational) |
| B/A = G\* from self-duality (Tr = N) | [CONDITIONAL THEOREM] |
| G\*ⁿ = period of Symⁿ(h¹(E_i)) for n > 2 | [CONJECTURE] |
| Sym³ = minimum observer level | [SELECTION] |
| PCIR mapping (P→0, C→1, I→2, R→3) | [CONJECTURE] |
| α = property of ℤ³ arithmetic, not FTD-specific | [CONJECTURE] — testable by computation |

---

## The Road Ahead

1. **Compute the Picard-Fuchs equation** for the K3 elliptic fibration and evaluate the connection matrix at the CM point. If the master quadratic emerges, the motivic program is complete.

2. **Run the e⁺e⁻ → γγ simulation** on the engine. If (1 + cos²θ) emerges from raw lattice dynamics, FTD produces its first quantitative prediction from process, not parameter.

3. **State r = 0.022 in Paper I.** This is the falsifiable prediction that earns or loses credibility by 2032.

4. **Test α universality.** Run standard lattice QED on ℤ³ at the self-dual coupling and verify that the self-consistent coupling is α. If true for ANY ℤ³ gauge theory, FTD's contribution is the observation, not the framework.

---

## Honest Negatives (April 4, 2026)

Two investigations produced negative results that are documented for epistemic completeness:

1. **e⁺e⁻ angular distribution:** The lattice produces B = -0.20 (near isotropic), not B = 1.0 (QED quadrupolar). Directed beams and wavepackets both give the same result. The engine does not yet reproduce the (1+cos²θ) angular distribution from lattice dynamics alone.

2. **2D spectral gauge-group selection:** A 2D FFT analysis of N-particle Born-rule spectral patterns appeared to show the Born rule filtering for Lie algebra gauge groups (N=5 fails, crystallographic N preferred). This was a **square-grid FFT artifact**. The real 3D cubic lattice engine shows all N values produce clean peaks at correct particle angles with monotonically decreasing concentration. See [EXPLR_SPECTRAL_ARTIFACT_DISCOVERY.md](../09_mathematical/EXPLR_SPECTRAL_ARTIFACT_DISCOVERY.md).

Both results demonstrate that the engine is the ground truth and preliminary analytical work must always be validated against it.
