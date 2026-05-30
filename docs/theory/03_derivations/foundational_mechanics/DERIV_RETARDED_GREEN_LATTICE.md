# DERIV — Retarded Lattice Green's Function Identity (Subsidiary of Phase G)

**Document type:** Derivation (subsidiary of Theorem 6 / FTD-0004)
**Status:** [DERIVED] — clean three-line lattice Fourier argument; the underlying d'Alembert relation is standard, the FTD-stencil-specific statement is new
**Created:** 2026-04-30
**Provenance:** opens the "exploit Maxwell's wave equation" research thread; identifies the radiation-zone extension of Phase G's static Coulomb identity
**Related:** `SPEC_ALGEBRAIC_SPINE.md §6` (Phase G static identity);
`DERIV_HEAT_EQUATION_FROM_RATIO.md` (heat-equation-side analog, where G* is the half-derivative eigenvalue);
`scripts/proofs/proof_retarded_green_identity.py` (numerical sanity check at L = 8)

---

## 0 · Statement

**Theorem (retarded Phase G).** Let `G^ret_L(r, t)` denote the retarded Green's
function of the lattice wave equation

```
(D²_t − c² Δ_L) G^ret_L(r, t) = δ_{r, 0} · δ(t),     G^ret_L(r, t) = 0  for  t < 0,
```

on the periodic L³ cubic lattice with lattice speed of light `c = c_lat = 1/√3`.
Define the **time-resolved geometric coupling**

```
α_r(r, t, L) := 2 · r · G^ret_L(r, t).
```

Then at every finite L,

```
∫_0^∞ α_r(r, t, L) dt = 2 · r · G_L(r) = α_r(r, L),     (★)
```

where `G_L(r)` is the periodic lattice Poisson Green's function and the
right-hand side is exactly Phase G's static identity (Theorem 6).

**Continuum limit.** As `L → ∞`,

```
α_r(r, t, ∞) = δ(t − r/c) / (2π),
```

a delta function on the forward light cone with **universal amplitude
`1/(2π)` independent of r**. The time integral evaluates to `1/(2π)`,
matching the continuum static `2r · 1/(4π r) = 1/(2π)` ✓.

---

## 1 · Three-line proof

In lattice Fourier (per-mode propagator on the periodic L³ lattice with
crystal momentum `k`):

```
G^ret_L(k, t) = sin(c · |k̂| · t) / (c · |k̂|),     t > 0,
```

where `k̂_i = 2 sin(k_i/2)` and `|k̂|² = Σ_i k̂_i²` is the lattice
Laplacian eigenvalue. Time-integrate with damping regulator `ε → 0⁺`:

```
∫_0^∞ e^{−εt} sin(c|k̂|t) / (c|k̂|) dt
    = 1 / [(c|k̂|)² + ε²]
    →  1 / (c² · k̂²)
    = G_L(k)     (the per-mode lattice Poisson Green's function)
```

Multiply by `2r` and inverse Fourier transform: identity (★) follows. ∎

The argument is the lattice version of the standard d'Alembert relation
`∫₀^∞ G^ret = G_static` for any wave equation; on a finite periodic lattice
the relation is exact (no IR regularization needed beyond the zero-mode
subtraction inherited by `G_L`).

---

## 2 · What this means

### 2.1 · Phase G is a static slice of a parent retarded identity

The static identity `α_r(r, L) = 2r · G_L(r)` (Theorem 6) is the
time-integrated form of a **time-resolved** identity at every finite L.
The instantaneous geometric coupling `α_r(r, t, L) = 2r · G^ret_L(r, t)`
carries the same prefactor `2r` as the static version — no new
calibration.

### 2.2 · The amplitude `1/(2π)` is a clean continuum number

In the continuum limit, the retarded coupling is a delta on the light
cone with amplitude exactly `1/(2π)`:

```
α_r(r, t, ∞) = δ(t − r/c) / (2π).
```

This is a **dimensionless, calibration-independent constant** that drops
out of the radiation-zone extension. It is a structural feature of the
geometric coupling, not a fitted parameter. Whether `1/(2π)` connects
structurally to `G* = Γ(1/4)² / (2√(2π) Γ(1/2))` via the `√(2π)` factor
is an open question worth scanning.

### 2.3 · Zero fine-structure content (preserved from Phase G)

Identity (★) carries no QED `α` content. The retarded coupling is
purely lattice geometry plus `c_lat = 1/√3`. Same caveat as Phase G:
this does **not** derive `α` at any scale; it states what the lattice
geometry produces with zero free parameters.

### 2.4 · Engine connection (radiation zone)

The engine's wave-propagation phase already evolves the flux field via
the lattice wave equation. Identity (★) is a **diagnostic** for the
wave-propagation toggle: emit a delta-pulse at the origin, read out
`G^ret_L(r, t)` directly from the engine, time-integrate, and verify
the result equals `2r · G_L(r) / r` from the static run. This is a
clean cross-check between the wave-propagation and gauss-projection
toggles, with zero free parameters.

---

## 3 · Lattice dispersion structure of the time-resolved coupling

At finite L, `α_r(r, t, L)` is **not** a delta on the light cone — it
is smeared by lattice dispersion. The smearing structure is governed by
the discrete dispersion relation:

```
ω_lattice(k) = c · |k̂| = c · √[ Σ_i 4 sin²(k_i/2) ]
```

For `|k| ≪ π`, `ω_lattice ≈ c·|k| · (1 − k²/24 + …)`, so wavefronts
propagate slightly slower than `c` for high-k modes; the front of the
retarded pulse trails the continuum light cone by `O(k²)`. The time
spread of `α_r(r, t, L)` at fixed `r` is `O(r/L²)` — vanishing in the
continuum limit, present at finite L.

This finite-L dispersion smearing is the lattice analog of regulator
artifacts in standard QED loop calculations. It is a structural
feature, not a defect.

---

## 4 · What this is NOT

- **NOT a derivation of `α` from the lattice.** Identity (★) preserves
  Phase G's "no fine-structure content" caveat. The geometric coupling
  is `2r · G^ret_L(r, t)` — no fine-structure constant appears anywhere.

- **NOT new mathematics in the d'Alembert sense.** The relation
  `∫G^ret = G_static` is standard. What's new is its application to
  FTD's specific lattice stencil with the Phase G prefactor, and the
  observation of the universal `1/(2π)` continuum amplitude.

- **NOT a Liénard-Wiechert formula derivation.** Liénard-Wiechert
  describes the fields of a moving point charge in continuum
  electrodynamics; identity (★) is the lattice retarded Green's
  function for the wave operator with a stationary source. Extending
  to moving sources (and verifying the lattice Liénard-Wiechert form)
  is a separate [OPEN] follow-up.

- **NOT a resolution of the FTD-0096 mass-unit no-go or the FTD-0059
  length-unit no-go.** Identity (★) lives in dimensionless lattice
  geometry; calibration to physical units is unaffected.

- **NOT a new spine theorem.** The spine count is unchanged — nine
  numbered results, six theorem-grade + three honestly-tiered (see
  `SPEC_ALGEBRAIC_SPINE.md` §0). Identity (★) is filed as a
  subsidiary derivation under Theorem 6 / FTD-0004.

---

## 5 · Open follow-ups

1. **Does `1/(2π)` connect structurally to `G*`?** `G* =
   Γ(1/4)² / (2√(2π) Γ(1/2))` carries a `1/√(2π)` factor; the
   continuum retarded amplitude is `1/(2π) = 1/(√(2π))²`. Whether this
   is a coincidence or a structural connection is open. Anti-target
   reminder: a numerical match of `1/(2π)` to FTD constants without a
   structural mechanism is a Koide-style fishing exercise. A fishing
   answer is forbidden by CLAUDE.md.

2. **Lattice Liénard-Wiechert.** Replace the stationary delta source
   in identity (★) with a moving point source on the lattice; ask
   whether the radiation-zone field is captured by a Liénard-Wiechert
   analog. Extension to moving charges is the natural radiation-physics
   continuation.

3. **Engine cross-check.** Implement an engine measurement of
   `G^ret_L(r, t)` from a delta-pulse source; verify identity (★)
   numerically end-to-end at L ∈ {16, 32, 64}. This is a one-day engine
   campaign that bridges the static and wave-propagation toggles.

4. **Hodge duality on the lattice.** Independent of (★), the question
   "does FTD's stencil preserve the Hodge duality `F ↔ ⋆F` of Maxwell?"
   — and if it doesn't, what is the lattice-scale duality-breaking
   parameter? — is a separate open direction in the same Maxwell-exploit
   thread.

---

## 6 · LEDGER status

This document files **FTD-0113** at the [DERIVED] tag, subsidiary to
FTD-0004 (Phase G). It does not modify FTD-0001/0002/0013/0014.

---

## 7 · Verification

Numerical sanity check at L = 8 in `scripts/proofs/proof_retarded_green_identity.py`:

- Builds the lattice Laplacian eigenvalues directly
- Computes `G^ret_L(r, t)` via the per-mode propagator
- Time-integrates (truncated at finite T_max with damping regulator)
- Compares to `G_L(r)` computed independently via the lattice Poisson
  solver
- Asserts equality to `≥ 10` digits at `r ∈ {1, 2, 3, 4}` along the
  axis direction

---

*End of derivation.*
