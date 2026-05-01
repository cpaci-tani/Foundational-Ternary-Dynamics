# DERIV — Lattice Liénard-Wiechert Potential and Lattice Cherenkov Pole

**Document type:** Derivation
**Status:** [DERIVED] — closed-form lattice boosted Coulomb at uniform velocity; general accelerating motion remains [OPEN]
**Created:** 2026-04-30
**Provenance:** Q1 of the Maxwell-exploit thread opened with FTD-0113
**Related:** `DERIV_RETARDED_GREEN_LATTICE.md` (FTD-0113, the static-source parent);
`DERIV_LATTICE_HODGE_DUALITY.md` (FTD-0114, the algebraic-backbone parallel);
`SPEC_ALGEBRAIC_SPINE.md §6` (Phase G, the static-Coulomb base);
`scripts/proofs/proof_lattice_lienard_wiechert.py` (numerical verification at L = 16)

---

## 0 · Statement

**Theorem (lattice boosted Coulomb at uniform velocity).** Let `q · δ(x − v·t)`
be a signed point source moving at constant velocity `v` on the
periodic L³ cubic lattice with lattice speed `c = c_lat = 1/√3`.
Defining `X := x − v·t` (the instantaneous relative position) and
working in the source-instantaneous frame, the retarded scalar potential is

```
A⁰(X, L, v) = q · (1/L³) · Σ_{k≠0}  e^{i k·X} / [(c|k̂|)² − (k·v)²]      (★)
```

where `|k̂|² = 4 Σ_i sin²(k_i/2)` is the lattice Laplacian eigenvalue.
Identity (★) is exact at every finite L, modulo the iε retarded
prescription in the time integral.

**Limits:**
- `v = 0`: identity (★) reduces to `q · G_L(X)` (Phase G recovery via FTD-0113).
- `L → ∞`: identity (★) reduces to the continuum Lorentz-boosted Coulomb
  potential (longitudinal contraction by `1/γ`).

**Structural feature (lattice Cherenkov pole).** The denominator in (★)
vanishes when

```
|k · v| = c · |k̂|
```

at any non-zero mode `k`. At small `|k|` this requires `v ≥ c` (forbidden
by CFL); at large `|k|` near the Brillouin-zone edge, lattice dispersion
makes `|k̂|/|k|` arbitrarily small, so the pole exists at *any* `v > 0`
for sufficiently high-k modes. This is a **lattice Cherenkov effect**:
moving sources excite radiation in lattice-wavelength modes regardless of
how slow `v` is — sourced by lattice dispersion, not by a refractive medium.

---

## 1 · Derivation

### 1.1 · The convolution integral

The retarded 4-potential for a 4-current `j^μ(x', t')` is

```
A^μ(x, t) = ∫ G^ret_L(x − x', t − t') · j^μ(x', t') d³x' dt'.
```

For a point charge at trajectory `x_s(t')` with velocity `v(t')`:

```
j⁰(x', t') = q · δ³(x' − x_s(t')),
jⁱ(x', t') = q · vⁱ(t') · δ³(x' − x_s(t')).
```

Substituting and integrating out the spatial delta:

```
A⁰(x, t) = q · ∫_{−∞}^t G^ret_L(x − x_s(t'), t − t') dt'.        (1)
Aⁱ(x, t) = q · ∫_{−∞}^t vⁱ(t') · G^ret_L(x − x_s(t'), t − t') dt'.  (2)
```

These are the **lattice Liénard-Wiechert potentials** — exact at every
finite L, valid for arbitrary trajectories.

### 1.2 · Static recovery (v = 0)

For `x_s(t') = 0` and `v = 0`:

```
A⁰(x, t)|_{v=0} = q · ∫_{−∞}^t G^ret_L(x, t − t') dt'
                 = q · ∫_0^∞ G^ret_L(x, τ) dτ
                 = q · G_L(x)                                       (3)
```

by FTD-0113. Multiplying by `2r/r` recovers Phase G: `α_r(r, L) = 2r ·
G_L(r)`. ✓

### 1.3 · Uniform-velocity solution in closed form

For `x_s(t') = v · t'` (constant velocity), substitute `τ = t − t'` and
`X = x − v · t` (the instantaneous separation in the lab frame):

```
x − x_s(t') = x − v · t' = x − v · (t − τ) = X + v · τ.
```

Therefore

```
A⁰(X, L, v) = q · ∫_0^∞ G^ret_L(X + v · τ, τ) dτ.                  (4)
```

Lattice Fourier expansion of `G^ret_L`:

```
G^ret_L(r, τ) = (1/L³) Σ_{k≠0} (sin(c|k̂|·τ) / (c|k̂|)) · e^{i k·r}.  (5)
```

Substitute into (4) and exchange the (finite) sum with the integral:

```
A⁰(X, L, v)
  = q · (1/L³) Σ_{k≠0} (1/(c|k̂|)) · e^{i k·X} ·
        ∫_0^∞ sin(c|k̂|·τ) · e^{i (k·v) τ} dτ.                       (6)
```

The per-mode time integral, with damping regulator `ε → 0⁺`:

```
∫_0^∞ e^{−ετ} · sin(ω₀ τ) · e^{i Ω τ} dτ
   = ω₀ / [(ω₀ + Ω + iε)(ω₀ − Ω − iε)],   ω₀ := c|k̂|,  Ω := k·v.
   →  ω₀ / (ω₀² − Ω²)        (ε → 0⁺, principal value).
```

Divide by `ω₀ = c|k̂|` and substitute back:

```
A⁰(X, L, v) = q · (1/L³) Σ_{k≠0}  e^{i k·X} / [(c|k̂|)² − (k·v)²].   (★)
```

This is identity (★) of the theorem statement. ∎

### 1.4 · Continuum limit recovery

As `L → ∞`: lattice momenta `k` densify, `|k̂|² → |k|²`, the sum becomes
an integral:

```
A⁰_cont(X, v) = q ∫ d³k/(2π)³ · e^{i k·X} / [c²|k|² − (k·v)²].
```

Decompose into components parallel and perpendicular to `v`:
`k = k_∥ v̂ + k_⊥`, `k·v = k_∥ v`. The denominator becomes

```
c²|k|² − (k·v)² = c²(k_∥² + |k_⊥|²) − k_∥² v²
                = c²|k_⊥|² + k_∥² (c² − v²)
                = c² · [|k_⊥|² + k_∥² / γ²]      with γ := 1/√(1−β²).
```

Inverse Fourier transform over `(k_∥, k_⊥)` gives the continuum
Lorentz-contracted Coulomb potential:

```
A⁰_cont(X, v) = q · γ / (4π c² · √(|X_⊥|² + γ² · X_∥²))
              = q / (4π c² · √(|X_⊥|² + γ² · X_∥²) / γ)
              = q · γ / (4π c² R*),    R* := √(|X_⊥|² + γ² X_∥²)/γ.
```

(Standard result; longitudinal Lorentz contraction visible in the `γ²`
in front of `X_∥²`.) ✓

### 1.4.1 · L → ∞ subtlety: Cherenkov modes shift to UV

The "continuum recovery as L → ∞" claim in §1.4 is correct but with a
subtle caveat: at every finite L, identity (★) includes contributions
from high-k modes near their lattice Cherenkov pole (§1.5 below). As
`L → ∞`, those modes shift toward `k → ∞` (the UV cutoff), where they
are absent from the continuum spectrum. So the continuum recovery is
clean **when one takes L → ∞ at fixed physical k** — but the residual
between lattice (★) and continuum at any finite L includes a
lattice-Cherenkov tail that does not vanish smoothly with `1/L`.

Numerical illustration at L = 16, v = 5% c_lat (just below the
first-pole threshold v_th ≈ 6.6% c_lat from §1.5): the ratio
`A⁰_lattice(★) / A⁰_static` measured at perpendicular `X` ranges
1.14–1.40, while the continuum γ ≈ 1.001. The lattice deviation is
**dominated by the pre-pole lattice-Cherenkov tail at high k**, NOT
by Lorentz contraction. This is the cleanest structural finding of
the lattice-LW analysis: at finite L, the lattice boosted Coulomb is
qualitatively different from continuum boosted Coulomb at any
non-zero `v`, due to dispersion-driven mode tails near the BZ edge.

### 1.5 · The lattice Cherenkov pole

The denominator in (★) is `D_k(v) := (c|k̂|)² − (k·v)²`. Define the
mode phase speed

```
v_phase(k) := c · |k̂| / |k|.
```

Then `D_k(v) = |k|² · (v_phase² − (k̂·v̂)² · v²)` where `k̂ = k/|k|`.
The pole condition `D_k(v) = 0` is

```
|cos θ_{kv}| · v = v_phase(k),
```

where `θ_{kv}` is the angle between `k` and `v`. For modes with
`|k| ≪ π`, `v_phase ≈ c`, so the pole condition requires `v ≥ c`
(forbidden by CFL). For modes with `|k|` near the Brillouin-zone edge,
`|k̂|/|k|` becomes small (lattice dispersion slows high-k modes), so
`v_phase < c`, and the pole condition is met for `v` strictly less
than the CFL maximum.

**Physical reading:** any moving source on the lattice excites
Cherenkov-like radiation in lattice-wavelength modes, regardless of how
slow `v` is. The radiation is concentrated at high-k (~ lattice
spacing) and is dynamically suppressed for `v ≪ c`, but it is
structurally present.

This is the lattice analog of the standard Cherenkov effect (charge
moving faster than light in a refractive medium). On the lattice the
"medium" is the lattice itself; the dispersion makes high-k modes
slow enough that any motion produces Cherenkov radiation in those
modes.

### 1.6 · General motion (accelerating sources)

For arbitrary `x_s(t')` and `v(t')`, equations (1)/(2) remain valid
but the closed form (★) does not factor. The natural representation
is the frequency-domain transform of the source worldline:

```
A⁰(x, ω) = q · (1/L³) Σ_{k≠0}  J⁰(k, ω) / [ω² − (c|k̂|)² + iε·sgn(ω)]
```

where `J⁰(k, ω) := ∫ e^{−i k·x_s(t') + iωt'} dt'` is the source's
spacetime Fourier transform. This is the formal solution; it does not
simplify further at finite L without specifying the trajectory.

In the continuum limit (`L → ∞`), the standard Liénard-Wiechert
result emerges from the saddle-point evaluation at the retarded time
`t_ret(x, t)` defined by the light-cone condition. On the lattice, the
"retarded time" becomes a smeared interval rather than a single point,
governed by lattice dispersion of `G^ret_L`.

This is the natural [OPEN] follow-up: characterize the lattice
LW radiation field for accelerating sources at finite L, including
the lattice analog of the radiation-zone `1/r` field and the
acceleration-radiation rate.

---

## 2 · What this means

### 2.1 · The substitution rule for moving sources

The structural content of (★), compared to FTD-0113's static
identity, is captured by **one substitution** in the per-mode
denominator:

```
Static Phase G:        denominator = (c|k̂|)²
Boosted Coulomb (★):   denominator = (c|k̂|)² − (k·v)²
```

This substitution captures the *entire* Lorentz-boost content of
moving point charges on the lattice. There are no additional lattice
correction terms beyond the standard lattice dispersion (`|k̂|` rather
than `|k|`).

### 2.2 · The lattice Cherenkov radiation tail

For any moving source `v ≠ 0`, the closed form (★) develops poles in
high-k modes near the Brillouin-zone edge. The poles are *real* —
they correspond to genuine radiation modes that are excited by the
moving source. This radiation has wavelength `~ a` (lattice spacing)
and is the lattice analog of medium-Cherenkov radiation.

In SI-calibrated units (`a_phys ≡ ℓ_P` per FTD-0059), the lattice
Cherenkov radiation has wavelength `~ ℓ_P` and is suppressed by
`(v/c)^{∞}` at typical accelerator energies. It is far below any
detectable scale. But it is a structural feature of any FTD-style
lattice ED.

### 2.3 · The radiation-zone analog of (★)

For accelerating sources, the radiation field (the `~ 1/r` part of
the LW potential) has a clean continuum form involving the proper
acceleration of the charge. The lattice version is computable from
the frequency-domain formula (§1.6) but does not have a single closed
form. Whether the radiation rate has a clean lattice expression is
an open question worth pursuing.

### 2.4 · Engine connection (radiation from moving charges)

The engine's wave-propagation phase + state-transport rules are
sufficient in principle to evolve a moving signed source and
simulate its retarded field. Identity (★) provides a closed-form
benchmark for this simulation in the uniform-velocity case: emit a
moving point source, measure `A⁰(X, L, v)` at various `(X, v)`,
verify it matches the Fourier sum (★). This is a clean cross-check
between the engine's dynamics and the analytical lattice LW result.

---

## 3 · What this is NOT

- **NOT a derivation of `α` from the lattice.** Identity (★)
  preserves Phase G's "no fine-structure content" caveat. The
  geometric coupling for moving sources is `2r · A⁰(X, L, v)/q` —
  pure lattice geometry plus `c_lat = 1/√3`.

- **NOT a closed-form general LW.** The closed form holds only for
  uniform velocity. Accelerating sources require the
  frequency-domain formula in §1.6, which does not simplify at
  finite L.

- **NOT a Lorentz-invariance result.** The lattice breaks Lorentz at
  finite L; identity (★) merely shows that the L → ∞ limit recovers
  the continuum Lorentz-contracted form. Lattice Lorentz anisotropy
  is being audited separately (`AUDIT_LORENTZ_ANISOTROPY.md`).

- **NOT a prediction of macroscopic Cherenkov radiation.** The
  lattice Cherenkov tail predicted by (★) has wavelength `~ a_phys ≡
  ℓ_P` and is far below any laboratory scale. The prediction is
  structural, not phenomenological.

- **NOT a new spine theorem.** Filed as FTD-0115 [DERIVED],
  subsidiary to FTD-0004/0113. Spine count remains 9.

---

## 4 · Open follow-ups

1. **Radiation-rate formula for accelerating sources.** Compute the
   lattice analog of the Larmor formula `P = q²a²/(6πε₀c³)` from the
   frequency-domain LW formula (§1.6). Whether the result has a
   clean closed form on the lattice is open.

2. **Engine cross-check for uniform-velocity LW.** Implement an
   engine benchmark: source moving at velocity `v` along the x-axis,
   measure scalar potential at points off-axis, compare to (★).
   Should pass at machine precision modulo discretization of the
   delta source.

3. **Lattice Cherenkov radiation rate.** The pole in (★) at
   `|k·v| = c|k̂|` indicates radiation. Compute the energy loss rate
   of a uniformly-moving source on the lattice (the lattice analog
   of "Cherenkov drag"). At what `v` does the rate become
   non-negligible?

4. **Generalization to bound-state dynamics.** The lattice LW
   formula describes the field of a moving point source; the
   engine's bound states (FTD-0107 cluster of 25 voxels) move
   collectively. Whether the LW formula generalizes to extended
   bound-state sources, with the cluster centroid replacing
   `x_s(t)`, is open.

---

## 5 · LEDGER status

This document files **FTD-0115** at the [DERIVED] tag, subsidiary to
FTD-0004 (Phase G) and FTD-0113 (retarded extension). Together
FTD-0004 + FTD-0113 + FTD-0114 + FTD-0115 constitute the lattice
electrodynamics kinematic-and-source-structure-and-Bianchi backbone:

| Result | Coverage | Source |
|---|---|---|
| FTD-0004 | Static Coulomb on lattice | Phase G |
| FTD-0113 | Retarded radiation Green's function | Q-A of Maxwell exploit |
| FTD-0114 | Bianchi identities (no monopoles, gauge invariance) | Q-2 of Maxwell exploit |
| FTD-0115 | Liénard-Wiechert for moving sources, lattice Cherenkov | Q-1 of Maxwell exploit |

This completes the kinematic + uniform-motion structure on the
lattice. What remains for a full Maxwell-on-FTD theory is the
dynamical source coupling (which is the EFT Recovery Program in
`docs/theory/10_eft_program/`) and the radiation-rate formulas for
accelerating sources (open follow-up #1 above).

---

## 6 · Verification

Numerical sanity check at L = 16 in
`scripts/proofs/proof_lattice_lienard_wiechert.py`:

- **Test A** (static recovery): set `v = 0`, evaluate (★), compare
  to direct lattice Poisson Green's function `G_L(X)`. Asserts
  `≥ 13` digits of agreement at `X ∈ {1·e_x, 2·e_x, 3·e_x}`.
- **Test B** (continuum-limit boosted recovery): set `v = 0.3 · c`
  along x-axis at L = 16, compute (★) at fixed `X = (0, 2, 0)`
  (perpendicular to motion), compare to continuum boosted-Coulomb
  prediction `q · γ / (4π c² · √(|X_⊥|² + γ² X_∥²))`. Expects
  agreement at the few-percent level (lattice corrections at
  finite L; not machine-precision).
- **Test C** (lattice Cherenkov pole): scan `v` from `0` to
  `0.5 · c` and locate the smallest `v` at which (★) develops a
  near-singular contribution from a high-k mode. Reports the
  `(k_pole, v_threshold)` pair.

---

*End of derivation.*
