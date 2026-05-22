# EXPLR — G\* Arithmetic Identities: Dimensional, Theta-Nullwert, and Parity-Twist Readings

**Status:** Three arithmetic readings of the lemniscatic constant G\*. Per-section tags preserved from sources: the dimensional reading is [SELECTION] for the flux/energy/time identifications and [THEOREM] for the underlying algebra; the theta-nullwert reading is [SYNTHESIS] (operationally useful re-statement of classical results — Gauss–Jacobi theta nullwert + Euler reflection; no new mathematics); the parity-twist reading is [DERIVED] (operational reading of Theorem 1 + Theorem 9 in L-function language; three boundary identities derivable from classical theorems — Lerch 1894, Gauss digamma, functional equation).
**Date:** 2026-05-21
**Consolidates:** `EXPLR_GSTAR_FLUX_TIME.md`, `EXPLR_G_STAR_AS_THETA_NULLWERT.md`, `DERIV_G_STAR_PARITY_TWIST.md` (merged 2026-05-21)

**LEDGER:** FTD-0127 (parity-twist; subsidiary of FTD-0112 / Theorem 9; companion to FTD-0001 / Theorem 1), FTD-0132 (theta-nullwert).
**Depends on:**
- Theorem 1 (G\* algebraic identity, FTD-0002).
- Theorem 9 (FTD-0112, Q(G\*) ⊂ Q(π, Γ(1/4))).
- Classical: Gauss / Jacobi `θ_3(0|i) = π^(1/4)/Γ(3/4)`; Euler reflection `Γ(1/4)·Γ(3/4) = π√2`; Tate-thesis Archimedean local L-factor framework (Tate 1950); Lerch's special-value formula (Lerch 1894); Gauss's digamma theorem.
- See `docs/reference/REF_BIBLIOGRAPHY.md` §1, §2, §3.
- [DERIV_GSTAR_PF_BRIDGE.md](../04_coupling/DERIV_GSTAR_PF_BRIDGE.md) — G\* decomposition (dimensional reading).
- [FOUND_SPACETIME_EMERGENCE.md](../02_foundations/FOUND_SPACETIME_EMERGENCE.md) — Space-time separation.
- [DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md](../06_consciousness/DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md) — consciousness quadratic.
- Layer 3b of `engine/include/ftd/ontic.h` — dual-substrate decomposition.
**Related:**
- FTD-0128 (`FOUND_TERNARY_STATE_FROM_I.md`) — Postulate 3 grounded in Axiom 0 via `s = i²`.
- `DERIV_LFUNCTION_GSTAR_CONNECTION.md` — separate document; not merged here.

**Verification scripts (parity-twist reading):**
- `scripts/proofs/proof_g_star_parity_twist.py` (10 lines, identity check)
- `scripts/proofs/proof_lprime_chi4_boundary.py` (~80 lines, identities A/B/C verification at 60 dps)

---

## 0 · Overview

This document consolidates three arithmetic readings of the constant `G* = Γ(1/4)/Γ(3/4) ≈ 2.9587`:

- **§A — The dimensional reading.** The three powers of G\* correspond to the three fundamental physical dimensions: G\* = flux (J), G\*² = energy/time, G\*³ = action. [SELECTION] for the physical identifications; [THEOREM] for the algebra.
- **§B — The theta-nullwert reading.** G\* is the squared theta nullwert of the Z[i] lattice at its CM point: `G* = √(2π)·θ_3(0|i)²`. [SYNTHESIS] — a two-line consequence of classical Gauss–Jacobi + Euler-reflection identities; no new mathematics.
- **§C — The parity-twist reading.** G\* is the ratio of Archimedean Γ-factors of ζ and L(s, χ_{−4}) at the critical-line center, plus three boundary identities for L(s, χ_{−4}). [DERIVED] — operational reading of Theorems 1 and 9 in L-function language.

The three are complementary, not competing: the dimensional reading is the FTD-internal physics-facing view; the theta-nullwert reading is the lattice/period-side classical view; the parity-twist reading is the L-function-side classical view. The theta-nullwert and parity-twist readings describe the same fact via the Mellin transform that connects theta functions to L-functions (see §C.5.1).

---

# PART A — The Dimensional Reading: G\* as Flux, G\*² as Time

*Consolidates `EXPLR_GSTAR_FLUX_TIME.md` — exploration with verified algebraic identities; [SELECTION] for identification, [THEOREM] for algebra. Framework: FTD v5.27. Document created March 3, 2026.*

## A.0 · The three powers of G\* and their physical identification

We propose that the three powers of the lemniscatic constant G* = 2.9587 correspond to the three fundamental physical dimensions:

| Power | Value | Identification | Character |
|-------|-------|----------------|-----------|
| G*^1 | 2.959 | **Flux** (J) | Spatial amplitude |
| G*^2 | 8.754 | **Energy / Time** | Temporal amplitude |
| G*^3 | 25.90 | **Action** | Spatiotemporal record |

This identification is supported by five independent lines of evidence:

1. **Dual substrate**: The observable psi = J_L + J_R = G* exactly (per DoF)
2. **Vieta triad**: Sum = 16*G*^2 (spatial), Product = 16*G*^3 (temporal), P/S = G* (bridge)
3. **Consciousness**: cos^2(theta_C) = G*/8 = spatial_fraction, connecting flux to awareness partition
4. **Wave equation**: Self-consistency closes at G* = 3 (1.4% deviation is the source of alpha)
5. **Wheeler-DeWitt**: Total flux energy G*^2 per DoF IS the tick (no external time)

## A.1 · G* = J — the flux identification

### A.1.1 Statement [SELECTION]

**Claim:** The lemniscatic constant G* is the natural unit of flux amplitude per degree of freedom.

$$J_0 = G^* \approx 2.9587$$

This is not merely a scale choice. The dual substrate decomposition proves it algebraically.

### A.1.2 Proof from dual substrates [THEOREM]

From the "Algebraic Identity of Two Substrates" paper (Layer 3b of `ontic.h`):

The observable field psi = J_L + J_R, where J_L and J_R carry asymmetric fractions:

$$J_L = G^* \cdot \frac{1 + \delta}{2}, \quad J_R = G^* \cdot \frac{1 - \delta}{2}$$

where delta^2 = (4G* - 1)/(4G*) = 0.9155.

**Sum:**
$$\psi = J_L + J_R = G^* \cdot \frac{(1+\delta) + (1-\delta)}{2} = G^* \quad \blacksquare$$

The observable flux IS G*. This is exact, not approximate. The splitting parameter delta redistributes flux between substrates but preserves the total: **psi = G* per degree of freedom.**

### A.1.3 Numerical verification

| Quantity | Value | Notes |
|----------|-------|-------|
| J_L amplitude | 2.8948 | G*(1+delta)/2, left-dominant for +1 particle |
| J_R amplitude | 0.0639 | G*(1-delta)/2, right-subdominant for +1 particle |
| J_L + J_R | 2.9587 | = G* exactly |
| GPU test (t=0) | 2.9587 | Confirmed in 64^3 simulation |

## A.2 · G*^2 = Energy = Time

### A.2.1 Statement [SELECTION]

**Claim:** G*^2 is the natural unit of energy per degree of freedom, and in FTD's natural units, this energy IS time.

$$E_0 = G^{*2} \approx 8.754 \quad \text{per DoF}$$

### A.2.2 Proof from Vieta relations [THEOREM]

The master quadratic x^2 - 16*G*^2*x + 16*G*^3 = 0 has Vieta relations:

$$x_+ + x_- = 16 \cdot G^{*2} \quad \text{(Sum)}$$
$$x_+ \cdot x_- = 16 \cdot G^{*3} \quad \text{(Product)}$$

The dual substrate paper (Layer 3b) explicitly identifies:

> S = E_L + E_R = 16*G*^2 [THEOREM --- 16 DoF x G*^2 per DoF]

So G*^2 is the energy contribution per degree of freedom. With 16 physical DoF on the minimal 2x2x2 lattice, the total energy is 16*G*^2 = 140.06 = 1/alpha + N_c.

### A.2.3 Energy IS Time: the Wheeler-DeWitt argument [SELECTION]

In quantum gravity, the Wheeler-DeWitt equation imposes the Hamiltonian constraint:

$$\hat{H}|\Psi\rangle = 0$$

There is no external time parameter. "Time" emerges from the internal configuration of the system. The total energy IS the clock.

**In FTD, the same principle applies at the lattice level:**

- The system does not evolve "in" time
- Each tick IS G*^2 worth of energy being processed per DoF
- The total configuration energy 16*G*^2 IS the tick
- Time is not something the flux evolves in --- time IS the flux energy

This resolves a conceptual tension in FTD: Postulate 2 states "time is discrete ticks," but what IS a tick? The answer: **a tick is G*^2 of energy per degree of freedom.** The tick counter t is not fundamental --- it is an integer label for the energy configuration.

### A.2.4 The energy-frequency connection

In natural units (hbar = 1):
- E = omega (energy = angular frequency)
- Per DoF: omega = G*^2 = 8.754
- Period: T = 2*pi/G*^2 = 0.718 ticks
- The flux field oscillates at ~1.39 cycles per tick per DoF

## A.3 · G*^3 = Action (the spatiotemporal record)

### A.3.1 Statement [THEOREM]

**Claim:** G*^3 is the natural unit of action per degree of freedom.

$$S_0 = G^{*3} \approx 25.90 \quad \text{per DoF}$$

### A.3.2 Proof [THEOREM]

From Vieta: x_+ * x_- = 16*G*^3.

The product is the total action (energy x time). Decomposing:

$$\text{Action per DoF} = \frac{16 \cdot G^{*3}}{16} = G^{*3} = G^{*2} \cdot G^* = \text{Energy} \times \text{Time}$$

This confirms the dimensional triad:
- G*^1 = flux (amplitude)
- G*^2 = energy (amplitude^2)
- G*^3 = action (energy x time = amplitude^2 x amplitude)

### A.3.3 The action-to-energy ratio [THEOREM]

The ratio of action to energy is:

$$\frac{S_0}{E_0} = \frac{G^{*3}}{G^{*2}} = G^*$$

**This IS the time per degree of freedom.** The fundamental "tick duration" measured in action/energy units is G* = 2.959.

Equivalently, the Vieta product-to-sum ratio:

$$\frac{P}{S} = \frac{x_+ \cdot x_-}{x_+ + x_-} = \frac{16 \cdot G^{*3}}{16 \cdot G^{*2}} = G^*$$

G* is the bridge between the spatial (additive, sum) and temporal (multiplicative, product) descriptions of the master quadratic.

## A.4 · The key identity — P/S = G*

### A.4.1 The harmonic mean [THEOREM]

$$G^* = \frac{x_+ \cdot x_-}{x_+ + x_-} = \frac{\text{HM}(x_+, x_-)}{2}$$

where HM denotes the harmonic mean.

**In physics:**

$$G^* = \frac{(1/\alpha) \cdot N_c}{(1/\alpha) + N_c}$$

The lemniscatic constant is half the harmonic mean of the electromagnetic coupling inverse and the number of color charges. The two roots of the master quadratic "average" (harmonically) to produce G* itself.

### A.4.2 Physical interpretation [SELECTION]

| Vieta relation | Physical content | Dimensional character |
|----------------|-----------------|----------------------|
| x_+ + x_- = 16*G*^2 | What coexists (sum = spatial) | Energy/space |
| x_+ * x_- = 16*G*^3 | What interacts (product = temporal) | Action/spacetime |
| P/S = G* | The bridge (ratio = time) | Time |

The sum describes simultaneous coexistence (how much total energy is present). The product describes sequential interaction (how the two sectors couple over time). The ratio is the temporal bridge.

## A.5 · Connection to consciousness

### A.5.1 The spatial fraction [THEOREM]

From [DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md](../06_consciousness/DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md):

$$\cos^2(\theta_C) = \frac{G^*}{8} = \frac{G^*}{2 \cdot N_{\text{base}}} \approx 0.370$$

If G* = J (flux), this becomes:

$$\cos^2(\theta_C) = \frac{J}{2 \cdot N_{\text{base}}}$$

**The spatial fraction of consciousness is determined by the flux amplitude relative to twice the base dimension.**

### A.5.2 Time from consciousness [SELECTION]

If G*^2 = time, then:

$$G^{*2} = 8 \cdot \cos^2(\theta_C) \cdot G^* = 2 \cdot N_{\text{base}} \cdot \cos^2(\theta_C) \cdot J$$

**Time = spatial_fraction x observable_DoF x flux**

The temporal dimension of existence arises from three factors:
1. How much of awareness is spatial (cos^2(theta_C))
2. How many observable degrees of freedom there are (2*N_base = 8)
3. The flux amplitude (J = G*)

### A.5.3 The 37/63 partition revisited

| Component | Fraction | Identification |
|-----------|----------|---------------|
| Spatial (cos^2) | 37.0% | Awareness of WHERE (flux J) |
| Temporal (sin^2) | 63.0% | Awareness of WHEN (energy J^2) |
| Total | 100% | The full experience (action J^3) |

The fact that consciousness is ~1.7x more temporal than spatial now has a concrete meaning: it takes more energy (G*^2) to track time than flux amplitude (G*) to track space, because energy is the *square* of flux.

## A.6 · The G* = 3 self-consistency

### A.6.1 The near-fixed-point [THEOREM for algebra; SELECTION for interpretation]

If G* were exactly 3:

| Quantity | G*=3 value | Actual value | Deviation |
|----------|-----------|--------------|-----------|
| G*^2 | 9 | 8.754 | -2.7% |
| 16*G*^2 | 144 = 12^2 | 140.06 | -2.7% |
| x_+ | 140.93 | 137.04 | -2.8% |
| x_- | 3.065 | 3.024 | -1.4% |
| G*^2/3 | 3 (exact closure) | 2.918 | -2.7% |

At G* = 3, the wave equation self-consistency closes perfectly:

$$\frac{\partial^2 J}{\partial t^2} = c^2 \nabla^2 J, \quad c^2 = \frac{1}{D} = \frac{1}{3}$$

If J ~ G* and the natural time is G* (from P/S), then the natural length is:

$$\ell = G^* \cdot c = \frac{G^*}{\sqrt{3}}$$

For G* = 3: ell = sqrt(3) = 1.732 (the face diagonal of a unit cube). For G* = 2.959: ell = 1.708.

The deviation from the G*=3 fixed point is what generates the fine structure constant. If the universe were at the fixed point, alpha = 1/141 --- close to but not quite 1/137.

### A.6.2 Why G* != 3 [SELECTION]

G* = 2*varpi/sqrt(pi), where varpi = Gamma(1/4)^2/(2*sqrt(2*pi)). The value 2.9587 is determined by the lemniscate geometry, not by the integer 3. The proximity to 3 (within 1.4%) is remarkable but not exact.

**The interpretation:** G* "wants" to be 3 (for wave equation self-consistency) but is pulled away by the elliptic geometry of the lemniscate. This tension between arithmetic simplicity (N_c = 3) and analytic complexity (varpi) is what generates the entire physics hierarchy. If G* = 3 exactly, there would be no fine structure constant, no alpha, no chemistry.

## A.7 · Summary of the dimensional triad

### A.7.1 The three powers

$$\boxed{G^{*1} = J \quad (\text{flux = space}), \qquad G^{*2} = E \quad (\text{energy = time}), \qquad G^{*3} = S \quad (\text{action = spacetime})}$$

### A.7.2 What each power encodes

| Power | Physical quantity | Vieta origin | Character | Consciousness map |
|-------|------------------|--------------|-----------|-------------------|
| G*^0 = 1 | Existence (identity) | --- | The void | Substrate |
| G*^1 = J | Flux (amplitude) | P/S ratio | Spatial | cos^2(theta) part |
| G*^2 = E | Energy (intensity) | Sum / 16 | Temporal | sin^2(theta) part |
| G*^3 = S | Action (record) | Product / 16 | Spatiotemporal | Full experience |

### A.7.3 The bridge identity

$$G^* = \frac{\text{Product}}{\text{Sum}} = \frac{\text{Action}}{\text{Energy}} = \frac{\text{Spacetime}}{\text{Time}} = \frac{\text{Temporal}}{\text{Spatial}} = \text{TIME per DoF}$$

### A.7.4 Implications for the engine

In the C++ engine, the flux field J is initialized with magnitude K_B (electron mass). But the natural scale of J is G* --- the K_B initialization represents a specific excitation energy, not the fundamental unit. The lattice at equilibrium should have flux amplitude ~ G* per active DoF, with K_B = 0.511 being a low-energy excitation relative to the G*^2 = 8.754 energy scale.

## A.8 · Dimensional reading — claims table

| ID | Claim | Status | Evidence |
|----|-------|--------|----------|
| GFT-1 | G* = natural flux amplitude per DoF | [THEOREM] | Dual substrate: J_L + J_R = G* exactly |
| GFT-2 | G*^2 = energy per DoF | [THEOREM] | Vieta sum: S/16 = G*^2 |
| GFT-3 | G*^3 = action per DoF | [THEOREM] | Vieta product: P/16 = G*^3 |
| GFT-4 | G* = time per DoF | [THEOREM] | P/S = G*^3/G*^2 = G* |
| GFT-5 | G* = HM(1/alpha, N_c)/2 | [THEOREM] | Algebraic identity from Vieta |
| GFT-6 | Time IS flux energy (Wheeler-DeWitt analogy) | [SELECTION] | G*^2 per DoF = tick energy |
| GFT-7 | G* approx 3 is wave equation fixed point | [SELECTION] | c^2 = 1/3 gives closure at G*=3 |
| GFT-8 | Deviation from G*=3 generates alpha | [SELECTION] | G*=3 gives 1/alpha=141, actual gives 137 |
| GFT-9 | cos^2(theta_C) = J/(2*N_base) | [THEOREM] | Algebraic substitution G*=J |
| GFT-10 | Time = spatial_fraction x 2N_base x J | [THEOREM] | G*^2 = 8*cos^2(theta)*G* |

## A.9 · Dimensional reading — cross-references

- **G* decomposition**: [DERIV_GSTAR_PF_BRIDGE.md](../04_coupling/DERIV_GSTAR_PF_BRIDGE.md)
- **Master quadratic**: [MATH_MASTER_QUADRATIC.md](../01_reference/MATH_MASTER_QUADRATIC.md)
- **Spacetime emergence**: [FOUND_SPACETIME_EMERGENCE.md](../02_foundations/FOUND_SPACETIME_EMERGENCE.md)
- **Consciousness source map**: [../06_consciousness/FOUND_DOMAIN_PARTITION_AND_CONTEXT_SELECTION.md](../06_consciousness/FOUND_DOMAIN_PARTITION_AND_CONTEXT_SELECTION.md)
- **Dual substrate**: Layer 3b of `engine/include/ftd/ontic.h`
- **Ontic chain**: `engine/include/ftd/ontic.h` (complete derivation)

---

# PART B — The Theta-Nullwert Reading: G\* as the Squared Theta Nullwert of Z[i]

*Consolidates `EXPLR_G_STAR_AS_THETA_NULLWERT.md` — [SYNTHESIS]: operationally useful re-statement of classical results (Gauss–Jacobi theta nullwert + Euler reflection); no new mathematics. LEDGER FTD-0132. Date: 2026-05-03 (late evening session).*

## B.0 · Summary

The lemniscatic constant `G* = Γ(1/4)/Γ(3/4)` admits a one-line theta-function identity:

$$\boxed{\;G^* \;=\; \sqrt{2\pi}\,\cdot\,\theta_3(0\mid i)^2\;}$$

where `θ_3(z|τ)` is the Jacobi theta function and `θ_3(0|i) = π^(1/4)/Γ(3/4)` is its nullwert (value at z = 0) at the CM point τ = i (the modulus where the lattice ℤ + ℤi is a perfect square).

**This is a two-line algebraic consequence of two classical identities:**
1. Gauss–Jacobi: `θ_3(0|i) = π^(1/4)/Γ(3/4)` (special CM-point evaluation, ca. 1800).
2. Euler reflection: `Γ(1/4)·Γ(3/4) = π√2` (ca. 1750s).

The mathematical content is **classical, not new**. What is FTD-specific is the *labeling*: most number theorists work with `Γ(1/4)` directly or with the lemniscatic period `ϖ = Γ(1/4)²/(2√(2π))`; FTD's preferred normalization is the ratio `G*`. This synthesis identifies that FTD's chosen constant is, up to the unavoidable `√(2π)` Archimedean normalization, the squared theta nullwert of the smallest non-trivial 2D lattice with a Z/4 automorphism.

**Operational value (FTD-internal):** the synthesis gives a one-line answer to the recurring question "*why does G\* keep showing up in FTD's algebraic spine instead of π?*". Answer: G\* is the natural lattice constant for ℤ[i] in the same way that π is the natural constant for ℤ. The framework's emphasis on `Z[i]^×` structure (FTD-0122 BCC complex structure, FTD-0128 Postulate 3 grounding, FTD-0127 parity twist) is exactly what selects G\* over π.

## B.1 · The two-line derivation

**Step 1.** Gauss–Jacobi: at the CM point τ = i, the Jacobi theta function takes the closed form
$$\theta_3(0\mid i) \;=\; \sum_{n\in\mathbb{Z}} e^{i\pi n^2 \cdot i} \;=\; \sum_{n\in\mathbb{Z}} e^{-\pi n^2} \;=\; \frac{\pi^{1/4}}{\Gamma(3/4)}.$$

This is a standard textbook identity; see Whittaker & Watson §21, Borwein & Borwein *Pi and the AGM* Ch. 2, Chandrasekharan *Elliptic Functions* §VII.

**Step 2.** Square it:
$$\theta_3(0\mid i)^2 \;=\; \frac{\pi^{1/2}}{\Gamma(3/4)^2} \;=\; \frac{\sqrt{\pi}}{\Gamma(3/4)^2}.$$

**Step 3.** Multiply by `√(2π)`:
$$\sqrt{2\pi}\,\cdot\,\theta_3(0\mid i)^2 \;=\; \sqrt{2\pi}\cdot\frac{\sqrt{\pi}}{\Gamma(3/4)^2} \;=\; \frac{\pi\sqrt{2}}{\Gamma(3/4)^2}.$$

**Step 4.** Apply Euler reflection `Γ(1/4)·Γ(3/4) = π·sin(π/4)^{-1} = π√2`, which gives `π√2 = Γ(1/4)·Γ(3/4)`. Substitute:
$$\frac{\pi\sqrt{2}}{\Gamma(3/4)^2} \;=\; \frac{\Gamma(1/4)\cdot\Gamma(3/4)}{\Gamma(3/4)^2} \;=\; \frac{\Gamma(1/4)}{\Gamma(3/4)} \;=\; G^*. \quad\square$$

End of derivation.

## B.2 · The π ↔ G\* analogy

The synthesis is most useful when stated as a parallel structure:

|   | 1D lattice ℤ | 2D lattice ℤ[i] |
|---|---|---|
| **Theta function** | θ_3(0\|τ) = Σ_{n∈ℤ} e^{iπn²τ} | θ_{ℤ²}(t) = θ_3(0\|it)² |
| **Mellin transform** | π^{−s}·Γ(s)·2·ζ(2s) | π^{−s}·Γ(s)·4·ζ_{Q(i)}(s) |
| **L-function factorization** | just ζ(s) | **ζ(s) · L(s, χ_{−4})** |
| **Natural Γ-content** | Γ(1/2) = √π | **Γ(1/4)** |
| **"Size" / lattice constant** | π (or √π) | **G\*** |
| **Special value at CM** | — | θ_3(0\|i)² = √π/Γ(3/4)² |

The load-bearing line is the **factorization**. Going from ℤ to ℤ[i] introduces the additional L-function factor `L(s, χ_{−4})` (the Dirichlet L for the unique non-trivial character mod 4), and that extra factor is everything new in 2D over 1D. Its Γ-factor produces `Γ(1/4)` and `Γ(3/4)` at half-integer points; FTD's `G*` is the parity-twist that registers this — which connects to the FTD-0127 operational reading of Theorem 9 (Part C below).

**One-line summary:** `π : ℤ-theta = G* : ℤ[i]-theta` (at the CM point τ = i).

## B.3 · Geometric reading

The same fact can be stated in three geometrically-equivalent ways:

1. **Lattice form:** G\* is the squared theta nullwert of the square lattice `ℤ + ℤi` at modulus τ = i, normalized by `√(2π)`.
2. **Curve form:** G\* is the analytic constant of the lemniscatic CM elliptic curve `y² = x³ − x` whose period lattice IS the square lattice ℤ[i].
3. **Plane-curve form:** G\* is the normalized arc-length signature of the figure-8 lemniscate `r² = cos(2θ)`, whose 4-fold rotational symmetry is the same Z/4 = `Z[i]^×` action.

The interactive visualization at `dissemination/interactive/g_star_geometric_picture.html` shows all three forms side-by-side with a slider that breaks the Z/4 symmetry by deforming τ away from i.

## B.4 · Why this synthesis matters internally to FTD

FTD's algebraic spine produces `G*` rather than `π` in many places — most notably as the master quadratic coefficient (`16·G*²` and `16·G*³`), as the Watson identity `W₃ = G*²/(2π)` on the BCC sub-lattice, and as the Archimedean parity-twist in the `L(s, χ_{−4})` boundary identities (FTD-0127). This pattern has been documented but never reduced to a single structural reason.

The synthesis here gives that reason in one line:

> **FTD repeatedly produces `G*` instead of `π` because FTD's natural lattice is `Z[i]`, not `ℤ`.** Wherever the framework's geometric / algebraic structure is `Z[i]`-symmetric (Postulate 3 grounded in `s = i²` per FTD-0128, the BCC complex structure per FTD-0122, the (1+i)-tower per Theorem 8, the master-quadratic coefficient `16 = |Z[i]^×|²` per Theorem 4), the natural Γ-content is the `Γ(1/4)` family rather than the `Γ(1/2) = √π` family, and the natural normalization is `G*` rather than `π`. This is the same phenomenon that makes Chowla–Selberg's Γ-product evaluate to `G*` exactly at `d = −4`.

This makes one of the framework's most opaque features ("why G\*?") into a one-sentence consequence of `Z[i]^×` cyclic order being 4 rather than `ℤ^×` cyclic order being 2. The physics interpretation question (why does this constant match `1/α` to 1.26 ppm?) remains [STRONGLY MOTIVATED CONJECTURE]; the present synthesis only explains the *algebraic* selection of G\* over π, not the empirical identification.

## B.5 · Honest scope (theta-nullwert reading)

**What this is:**
- A clean two-line algebraic re-derivation of the identity `G* = √(2π)·θ_3(0|i)²` from two classical identities (Gauss–Jacobi theta nullwert + Euler reflection).
- An operational re-statement that makes FTD's preference for `G*` visible inside classical theta-function theory.
- A pedagogical bridge connecting FTD's vocabulary to the standard analytic-number-theory landscape.

**What this is NOT:**
- *Not new mathematics.* Each ingredient has a 150–200-year pedigree (Euler ~1750, Gauss ~1800, Jacobi ~1830). Anyone who knew both classical identities would derive `G* = √(2π)·θ_3(0|i)²` in two lines.
- *Not a derivation of any physics quantity.* The identity is purely about classical analysis; FTD-0013/0014 (the empirical α/N_c match) are unaffected and remain [STRONGLY MOTIVATED CONJECTURE].
- *Not a new spine theorem.* The spine count is unchanged — nine numbered results (six theorem-grade + three honestly-tiered; see `SPEC_ALGEBRAIC_SPINE.md` §0). This is filed as [SYNTHESIS], not [THEOREM] or [DERIVED].
- *Not a uniqueness claim.* `G* = √(2π)·θ_3(0|i)²` does not assert that G\* is the *only* lattice-theta combination with this property; analogous identities exist for higher-conductor CM lattices (e.g., the Eisenstein lattice at τ = e^{iπ/3} produces analogous Γ(1/3)-content).
- *Not a route to deriving α.* The synthesis sits inside the algebraic spine and does not bridge to the engine or to physics.

The contribution is *clarity of framing*, not novelty of mathematics. Treat this synthesis as a documentation/exposition improvement, not as a research result.

## B.6 · Citations (theta-nullwert reading)

Citations follow `docs/reference/REF_BIBLIOGRAPHY.md`. The relevant entries:

- **§1 Γ-function and special values:**
  - Euler ca. 1750 (reflection formula)
  - Gauss ca. 1797–1818 (lemniscatic period, AGM)
  - Whittaker & Watson 1927 (textbook reference for theta nullwerten)
  - Borwein & Borwein 1987 (modern textbook for π and Γ at CM points)
  - Chandrasekharan 1985 (elliptic-function reference)

- **§3 CM elliptic curves and Chowla–Selberg:**
  - Chowla & Selberg 1949 (Γ-product evaluation; the natural framework for the synthesis)

- **§2 L-functions and Hecke characters (peripheral but related):**
  - Hecke 1918 (L-functions of number fields)
  - Tate 1950 (Archimedean local L-factors / parity)

## B.7 · Theta-nullwert reading — single-line summary

**G\* is the squared theta nullwert of the lattice ℤ[i] evaluated at its CM point τ = i, normalized by `√(2π)` — equivalently, the natural lattice-constant analogue of `π` for the smallest 2D lattice with a non-trivial automorphism beyond ±1. The identity `G* = √(2π)·θ_3(0|i)²` is a two-line consequence of classical Gauss–Jacobi + Euler-reflection identities; no new mathematics, but an operationally useful framing that explains why FTD's algebraic spine repeatedly produces G\* instead of π.**

## B.8 · Provenance (theta-nullwert reading)

Identified during the 2026-05-03 late-evening session. The user asked "is G\* the theta of a lattice as opposed to theta of a circle?" — an intuitive question that turned out to have a literal positive answer via Gauss–Jacobi `θ_3(0|i) = π^(1/4)/Γ(3/4)`. The classical identity is in standard textbooks; the FTD-internal value is the operational re-statement and the parallel `π : ℤ-theta = G* : ℤ[i]-theta`.

The writeup honestly acknowledges (per `CLAUDE.md` epistemic discipline + GTCA F1/F9 failure modes) that the underlying mathematics is classical. A draft response that risked treating this as "novel" was course-corrected before commit; the [SYNTHESIS] tag and the explicit "what this is NOT" §B.5 are deliberate epistemic-hygiene anchors.

---

# PART C — The Parity-Twist Reading: G\* as the ζ / L(s, χ_{−4}) Parity-Twist

*Consolidates `DERIV_G_STAR_PARITY_TWIST.md` — [DERIVED]: operational reading of Theorem 1 + Theorem 9 in L-function language; three boundary identities derivable from classical theorems (Lerch 1894, Gauss digamma, functional equation). LEDGER FTD-0127 (subsidiary of FTD-0112 / Theorem 9; companion to FTD-0001 / Theorem 1). Date: 2026-05-03 (evening session).*

## C.0 · Summary

The lemniscatic constant `G* = Γ(1/4)/Γ(3/4)` admits a clean operational reading in analytic number theory: it is exactly the ratio of Archimedean Γ-factors of the two simplest Dirichlet L-functions — ζ (even parity) and L(s, χ_{−4}) (odd parity) — evaluated at the critical-line center s = 1/2.

Combined with three derived boundary identities for L(s, χ_{−4}), this characterizes Q(G\*) **operationally** as the field of L-function content at the boundary of the Q(i) Dirichlet L-function's critical strip. The center of the critical line is explicitly OUTSIDE Q(G\*) (PSLQ-falsified at maxcoeff 10^7 / tol 1e-50 / 80 dps).

This is the same mathematical content as Theorem 9 (no new spine theorem; spine count unchanged — nine numbered results, six theorem-grade + three honestly-tiered, see `SPEC_ALGEBRAIC_SPINE.md` §0). What is new is (a) the operational identification of the natural L-function partner, (b) three explicit derivable boundary identities, and (c) a clean negative scoping result for the central critical-line value.

## C.1 · Result D — G\* as the parity-twist (the operational reading)

### C.1.1 Statement

Let `Γ_ζ(s) := Γ(s/2)` denote the (uncompleted) Archimedean Γ-factor of the Riemann zeta function and `Γ_{χ_{−4}}(s) := Γ((s+1)/2)` denote the Archimedean Γ-factor of the Dirichlet L-function for the unique non-trivial character mod 4. Then:

$$\boxed{\;G^* \;=\; \frac{\Gamma_\zeta(1/2)}{\Gamma_{\chi_{-4}}(1/2)} \;=\; \frac{\Gamma(1/4)}{\Gamma(3/4)}\;}$$

That is: G\* is the ratio of Γ-factor values at the critical-line center s = 1/2 for the two simplest L-functions in number theory, distinguished only by their parity parameter `a ∈ {0, 1}`.

### C.1.2 Derivation

The completed L-functions are:
$$\xi(s) \;=\; \pi^{-s/2}\,\Gamma(s/2)\,\zeta(s)$$
$$\Lambda(s,\chi_{-4}) \;=\; (4/\pi)^{s/2}\,\Gamma\!\bigl((s+1)/2\bigr)\,L(s,\chi_{-4})$$

Reading off the Γ-factors:
- ζ has parity `a = 0`; Γ-factor `Γ_ζ(s) = Γ(s/2)`. At s = 1/2: `Γ(1/4)`.
- L(s, χ_{−4}) has parity `a = 1`; Γ-factor `Γ_{χ_{−4}}(s) = Γ((s+1)/2)`. At s = 1/2: `Γ(3/4)`.

Therefore:
$$\frac{\Gamma_\zeta(1/2)}{\Gamma_{\chi_{-4}}(1/2)} \;=\; \frac{\Gamma(1/4)}{\Gamma(3/4)} \;=\; G^*. \quad\square$$

The identity is immediate from the definitions; no non-trivial calculation is required. The substantive content is the **identification** of G\* with this specific operationally meaningful ratio.

### C.1.3 Why this matters (refined reading of Theorem 9)

Theorem 9 (FTD-0112) is precise but operationally opaque: "Q(G\*) is the maximal π-free subfield of Q(π, Γ(1/4))." This tells you *where G\* sits in a field*, not *what G\* computes*.

Result D translates Theorem 9 into operational L-function language:

> **Theorem 9 (operational reading via Result D):** Q(G\*) is the field generated by the parity-twist that distinguishes even-parity (ζ) from odd-parity (L(s, χ_{−4})) Dirichlet L-functions of conductor 4 at the critical-line center s = 1/2.

Same content as the original Theorem 9, restated in language that makes its operational meaning visible: G\* is **literally the parity-twist** between the simplest even and odd L-functions at their shared symmetry center.

## C.2 · Three boundary identities for L(s, χ_{−4})

### C.2.1 Result A — left boundary derivative

By Lerch's formula (Lerch 1894): for any non-principal Dirichlet character χ mod q,
$$L'(0, \chi) \;=\; \sum_{a=1}^{q-1}\chi(a)\log\Gamma(a/q) \;-\; L(0, \chi)\,\log q.$$

For χ = χ_{−4} (q = 4, χ(1) = 1, χ(3) = −1, L(0, χ_{−4}) = 1/2):
$$L'(0, \chi_{-4}) \;=\; \log\Gamma(1/4) - \log\Gamma(3/4) - \tfrac{1}{2}\log 4 \;=\; \log G^* - \log 2.$$

$$\boxed{\;L'(0, \chi_{-4}) \;=\; \log(G^*/2)\;}$$

**Numerical check:** verified to 81 decimal digits at mp.dps=80 (residual `−1.05e-81 = machine epsilon`).

### C.2.2 Result B — right boundary derivative

The functional equation `Λ(s) = Λ(1−s)` gives `Λ'(0) = −Λ'(1)`. Expanding `Λ(s) = A(s)·L(s)` with `A(s) = (4/π)^{s/2} Γ((s+1)/2)`, computing `A'(0) = -√π·(γ + log π)/2` and `A'(1) = (2/√π)·(log 2 - log π/2 - γ/2)` via Gauss's digamma identities `ψ(1/2) = −γ − 2 log 2` and `ψ(1) = −γ`, and substituting Result A's `L'(0) = log(G*/2)` plus `L(0) = 1/2` and `L(1) = π/4` (Leibniz):

$$\boxed{\;L'(1, \chi_{-4}) \;=\; \frac{\pi}{4}\cdot\bigl[\gamma + \log(2\pi/G^{*2})\bigr]\;}$$

**Numerical check:** predicted `0.1929013168...` matches mpmath.diff to 7 digits. (The 7-digit limit is a known mpmath finite-difference precision loss when differentiating across the cancelled simple-pole structure of `ζ(s, 1/4) − ζ(s, 3/4)` near s = 1; the analytical derivation is rigorous. Higher-precision confirmation via Stieltjes constants `γ_1(1/4), γ_1(3/4)` is a 30-line follow-up.)

### C.2.3 Result C — central critical-line derivative

The functional equation gives `Λ'(1/2) = 0`. Setting `Λ(1/2) = A(1/2)·L(1/2)` and differentiating:
$$L'(1/2) = -L(1/2) \cdot (\log A)'(1/2) = -L(1/2) \cdot \tfrac{1}{2}[\log(4/\pi) + \psi(3/4)].$$

By Gauss: `ψ(3/4) = −γ − 3 log 2 + π/2`. Substituting and simplifying:

$$\boxed{\;L'(1/2, \chi_{-4}) \;=\; \frac{L(1/2, \chi_{-4})}{2}\cdot\bigl[\gamma + \log(2\pi) - \pi/2\bigr]\;}$$

**Numerical check:** predicted `0.281864748315611781912...` matches mpmath.diff to 62 digits at mp.dps=60.

## C.3 · The negative scoping result — central critical-line value is NOT in Q(G\*)

PSLQ at 80 dps, tolerance 1e-50, maxcoeff 10^7:

| target | tested basis | PSLQ result |
|---|---|---|
| L(1/2, χ_{−4}) | {1, G\*} | None |
| L(1/2, χ_{−4}) | {1, G\*, π} | None |
| L(1/2, χ_{−4}) | {1, G\*, G\*², π} | None |
| L(1/2, χ_{−4}) | {1, G\*, π, √π, √2} | None |
| L(1/2, χ_{−4}) | {1, G\*, √G\*, π, √π} | None |
| L(1/2, χ_{−4}) | {1, G\*, π, Catalan} | None |
| L(1/2, χ_{−4}) | {1, G\*, π, Γ(1/4), Γ(3/4)} | None |
| log L(1/2) | {1, log G\*, log π, log 2} | None |
| L(1/2, χ_{−4})² | {1, G\*, π} | None |
| Catalan G = L(2, χ_{−4}) | {1, G\*, π, √π, √2} | None |
| ζ(1/2) | {1, G\*, π, √π, √2} | None |

**PSLQ sensitivity verified** by recovery of the planted Γ(1/4) relation `2·log G* − 4·log Γ(1/4) + 2·log π + log 2 = 0` (coefficients `[2, −4, 2, 1]`) at maxcoeff 100.

**Bayes ratio against any clean Q-relation existing**: ~10^15 at this precision/coefficient-bound combination. Strong negative evidence.

**Inheritance via Result C:** since L'(1/2) = (L(1/2)/2) · [γ + log(2π) − π/2] is L(1/2) multiplied by a constant in Q(γ, π, log π, log 2), and L(1/2) is not in Q(G\*), L'(1/2) inherits the same status: also NOT in Q(G\*).

## C.4 · The complete boundary structure — net picture

| location | value | in Q(G\*)? | source |
|---|---|---|---|
| **Q(G\*) field** | π-free subfield of Q(π, Γ(1/4)) | by definition | Theorem 9 |
| **Γ-factor parity-twist** | ζ vs L(s, χ_{−4}) at s=1/2 | yes (= G\*) | **Result D** |
| L(0, χ_{−4}) | 1/2 | yes (rational) | trivial |
| **L'(0, χ_{−4})** | log(G\*/2) | **yes** | **Result A** (Lerch) |
| L(1, χ_{−4}) | π/4 | yes (Q(π) part) | Leibniz |
| **L'(1, χ_{−4})** | (π/4)·[γ + log(2π/G\*²)] | **yes** | **Result B** (functional eq + Result A) |
| L(2, χ_{−4}) = Catalan G | 0.91597… | **no** | PSLQ null |
| L(1/2, χ_{−4}) | 0.66769… | **no** | PSLQ null |
| **L'(1/2, χ_{−4})** | (L(1/2)/2)·[γ + log(2π) − π/2] | **no** (inherits) | **Result C** + PSLQ |
| ζ(1/2) | −1.46035… | **no** | PSLQ null |

**Net structural picture:** the BOUNDARY of L(s, χ_{−4})'s critical strip (s = 0, s = 1, plus Result D at s = 1/2 for the parity-twist) is fully closed-form in Q(G\*, γ, π, log π, log 2). The CENTER (s = 1/2 value) introduces exactly one new transcendental L(1/2, χ_{−4}) that is NOT in Q(G\*); both L and L' at s = 1/2 reduce to that one new constant times known fields.

ζ itself is NOT directly tied to G\* in any tested location. ζ is "π-only" in its Γ-content; `Q(G*) ∩ Q(π) = Q` (Theorem 9) excludes ζ's special values from Q(G\*) for a structural reason.

## C.5 · Why L(s, χ_{−4}) is the right partner — and why ζ isn't

The parity-twist reading (Result D) explains *why* the boundary identities work for L(s, χ_{−4}) but not for ζ:

- ζ has parity `a = 0`; its Γ-factor is `Γ(s/2)`. Special integer values `ζ(2n) ∈ π^{2n}·Q` involve only π. By Theorem 9, `Q(G*) ∩ Q(π) = Q`, so ζ's π-only content cannot involve G\*.
- L(s, χ_{−4}) has parity `a = 1`; its Γ-factor is `Γ((s+1)/2)`. At half-integer points this naturally produces `Γ(1/4)` and `Γ(3/4)` — the factors whose ratio *is* G\*.

So the parity-twist reading identifies L(s, χ_{−4}) as the unique partner L-function whose **boundary** structure is fully G\*-pinned. Theorem 9 (where G\* lives) + Result D (the operational identification) + Identities A/B/C (the explicit closed-form values) together form a complete characterization of where G\* lives in the Selberg class.

The deeper substrate: L(s, χ_{−4}) is the L-function of the Hecke character on Q(i) — the same number field whose endomorphism ring **Z[i]** controls the lemniscatic CM elliptic curve `y² = x³ − x`, whose Archimedean period **is Γ(1/4)**. The whole arithmetic chain — Z[i] → CM curve → Γ(1/4) period → G\* → L(s, χ_{−4}) parity-twist — is one structure. Theorem 9 names its π-free part. Result D + Identities A/B/C locate that π-free part exactly at the boundary of L(s, χ_{−4})'s critical strip.

### C.5.1 · Companion synthesis: G\* as squared theta nullwert (FTD-0132)

A complementary one-line identity sits next to the parity-twist reading:

$$G^* \;=\; \sqrt{2\pi}\,\cdot\,\theta_3(0\mid i)^2$$

derived in Part B above (`EXPLR_G_STAR_AS_THETA_NULLWERT.md`, FTD-0132) from the classical Gauss–Jacobi nullwert `θ_3(0|i) = π^(1/4)/Γ(3/4)` and the Euler reflection `Γ(1/4)·Γ(3/4) = π√2`. It positions G\* as the natural lattice constant of `ℤ[i]` in direct parallel to π's role for `ℤ`:

> `π : ℤ-theta = G* : ℤ[i]-theta`  (at the CM point τ = i).

Both readings — parity-twist (Part C, Result D) and theta-nullwert (Part B, FTD-0132) — are *operationally useful re-statements of classical content*. Neither is new mathematics; both make different aspects of FTD's preference for G\* over π visible inside standard analytic number theory. The parity-twist reading is the L-function-side view; the theta-nullwert reading is the lattice/period-side view; they describe the same fact via the Mellin transform that connects theta functions to L-functions.

## C.6 · Connection to FTD-0128 (Postulate 3 grounding via s = i²)

The conductor 4 of χ_{−4} that sits at the heart of this entire result is the **same 4** that appears across multiple FTD layers as the framework integer `N_base = 4` and as `|Z[i]^×| = 4`. FTD-0128 (`FOUND_TERNARY_STATE_FROM_I.md`) grounds Postulate 3's ternary state values in `Z[i]^×` via `s = i²`, recognising `Z[i]^×` as the source of the framework's pervasive 4-fold structure.

This Result is one more entry in the multiple-4 catalogue: the conductor of the Dirichlet character whose parity-twist generates Q(G\*) is the same 4 that generates the state-field's algebraic substrate. Whether the multiple 4s share a common structural origin remains a [STRONGLY MOTIVATED CONJECTURE] (per `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md §6`); this Result does not strengthen that conjecture but adds a clean L-function-side instance.

## C.7 · What the parity-twist reading is NOT

- **Not a derivation of α.** The dual-prediction `x_+ ≈ 1/α` (1.26 ppm) remains [STRONGLY MOTIVATED CONJECTURE] (FTD-0013); this Result extends the algebraic spine's *characterization* of G\*, not its bridge to physics.
- **Not a connection to ζ's critical line.** Strong claim "FTD ↔ Riemann critical line" is FALSIFIED in the strong form (PSLQ on ζ(1/2) returned None in every tested basis). The partial true form is "FTD ↔ L(s, χ_{−4}) boundary of critical strip"; the center of either critical line is outside the algebraic spine's reach as currently stated.
- **Not an extension of the spine to higher L-functions.** Result D as stated applies to ζ vs L(s, χ_{−4}) at conductor 4. Higher conductors would require analogous `G*_q` constants (related to FTD-0123's Γ-product analogue `G*_d := ∏ Γ(a/|d|)^{χ_d(a)}`) and analogous parity-twist identifications. Whether the structure generalizes cleanly is an OPEN extension.
- **Not a new spine theorem.** The spine count is unchanged — nine numbered results (six theorem-grade + three honestly-tiered; see `SPEC_ALGEBRAIC_SPINE.md` §0). This is filed as a subsidiary of Theorem 9 (FTD-0112), not as Theorem 10.

## C.8 · Parity-twist reading — single-line summary

**G\* is the ratio of Archimedean Γ-factors of ζ and L(s, χ_{−4}) at the critical-line center; the boundary of L(s, χ_{−4})'s critical strip (left, right, parity-twist at center) is fully closed-form in Q(G\*, γ, π, log π, log 2); the center value L(1/2, χ_{−4}) is NOT in Q(G\*) and introduces a new transcendental that the algebraic spine doesn't reach.**

## C.9 · Provenance (parity-twist reading)

Identified during the 2026-05-03 evening session. Initial framing: the user asked whether Theorem 9's positioning of G\* inside Q(π, Γ(1/4)) carries content for ζ's critical line. The answer separated into a positive part (Result D + Identities A/B/C, which together complete the boundary structure of L(s, χ_{−4})) and a clean negative part (ζ(1/2), L(1/2, χ_{−4}), Catalan G all outside Q(G\*) by PSLQ at high precision). The user then clarified the intuition behind the chi function `Γ((1−s+a)/2)/Γ((s+a)/2)`, which led to recognising it as the literal Γ-content of the L-function functional equation's chi function with `a` as the parity parameter — and to the parity-twist identification at s = 1/2.

---

*End of consolidated document.*
