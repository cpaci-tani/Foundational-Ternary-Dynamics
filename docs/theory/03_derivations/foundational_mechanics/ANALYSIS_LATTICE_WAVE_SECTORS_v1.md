# Analysis — FTD-0298: Lattice Wave Sectors — Light + Radio = One Flux-Wave Sector; No Acoustic Sector (BOUNDARY)

**Tag:** `[SYNTHESIS]` + `[BOUNDARY]`
**LEDGER row:** FTD-0298
**Verification:** `scripts/exploration/wave_sector_unit_map.py` (the unit-map numbers in §2/§4 are computed there, not asserted).

---

## 0 · Verdict

Three statements, at their honest tags:

1. **One dispersion curve organizes sound, light, and radio.** The flux field obeys a second-order wave equation `∂²J/∂t² = c²∇²J + sources` `[DERIVED]` with `c = C_SPEED = 1/√3` `[DERIVED]` (the CFL stability ceiling). Its dispersion `ω(k) = 2c·|sin(k/2)|` (per axis) is the single object behind all wave phenomena on the lattice: linear in the deep IR, flattening to a hard cutoff at the Brillouin-zone edge.

2. **Light and radio are the same thing** — both are excitations of the **flux wave sector**, differing only in wavenumber `k` `[DERIVED]` (Maxwell-exploit chain FTD-0113/0114/0115/0120; FC-2 makes this sector the carrier of the emergent Lorentzian metric). Because every laboratory frequency sits at `k/k_zone ≲ 10⁻²⁸`, FTD structurally predicts **no vacuum dispersion** at any observable scale: radio and light co-propagate at `c` to a precision (`~10⁻⁵⁷`) far beyond what astrophysical timing tests can probe `[DERIVED on the FTD side; the agreement with GRB/FRB null-dispersion bounds is an [OBSERVATION]]`.

3. **FTD has light but no sound.** There is **no acoustic / phonon sector** on the lattice, and the reason is structural: acoustic phonons are the Goldstone modes of *spontaneously broken continuous translation symmetry*, and FTD's lattice **is** space — there is no continuum for it to be a pattern *within*, so there is no such symmetry to break `[SELECTION]` + `[BOUNDARY]`. Three independent corpus facts close the gap (no displacement degree of freedom in the voxel model; the Gauss constraint projects out the longitudinal flux; FTD-0272's first-order genesis admits no Goldstone). One door stays open: a **condensate compression mode** in the manifested phase `[OPEN]`.

**Nothing is promoted.** This document is a synthesis of existing tags plus one mapped boundary. `x₊ = 1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`; FC-1/FC-2 stay `[AXIOM]`-class declarations; FTD-0270/0271/0272 are unchanged. No engine change; the golden gate is untouched.

---

## 1 · The organizing object — the lattice dispersion relation

The substrate carries one propagating wave: the flux `J`. From `render_bridge.cpp` (phase_read / phase_write), its dynamics is

```
∂²J/∂t² = c²·∇²J  +  g_c·∇(s)  +  g_c·∇×(s·v)          [DERIVED]
```

a genuine **second-order** (d'Alembertian) wave equation, integrated by leapfrog over the 18-point isotropic Laplacian. Two structural facts fix its character:

**The speed `c = 1/√3` is derived, not chosen.** It is the Courant–Friedrichs–Lewy stability ceiling for this equation (`engine/include/ftd/ontic/gauge_couplings.h`):

```
c²·(2D/h²) ≤ 2/dt²   (von Neumann)  ⇒  c² ≤ 1/D  ⇒  c = 1/√D = 1/√3   (D=3, h=dt=1)
```

This is `[DERIVED]` — uniquely forced by {D = 3, cubic lattice, leapfrog}; it is the same constraint as `c` in FDTD electromagnetics. **Scope:** the *dimensionless* value `1/√3` (voxels/tick) is derived; its SI value `2.998×10⁸ m/s` is calibration-dependent (`a_phys ≡ ℓ_P`, `t_phys`), which is `[DECLARED]`, not derived (§4).

**The dispersion is linear in the IR and flat at the zone edge.** Plane waves `J ∝ e^{i(kx − ωt)}` give, per axis,

```
ω(k) = 2c·|sin(k/2)|         (spatial-symbol form, used by the engine light scenarios)
ω(k) = 2·arcsin[c·sin(k/2)]  (the exact fully-discrete leapfrog form — same IR/UV behavior)
```

with the 3D engine using the 18-point isotropic symbol `M(k)` (validated to `1.33×10⁻¹⁵` against the closed form, FTD-0270, `[MEASURED]`). Three regimes follow:

| Regime | `k` | Behavior | Physical meaning |
|---|---|---|---|
| **IR** | `k → 0` | `ω ≈ c·k`; group velocity `v_g = c·cos(k/2) → c`; phase velocity `→ c` | dispersionless light/radio — all frequencies travel at `c` |
| **the bend** | `0 < k < π` | `ω` falls below `c·k`; `v_g = c·cos(k/2)` decreasing | where the PL-4/PL-5 deviations live (§3) |
| **zone edge** | `k → π` | `v_g → 0`; `ω_max = 2c = 2/√3 ≈ 1.15` rad/tick (leapfrog: `2·arcsin(1/√3) ≈ 1.23`) | the UV cutoff: 2-voxel standing waves carry no energy (Nyquist) |

The IR linearity is exactly why the lattice reproduces a sharp, frequency-independent speed of light; the zone-edge flattening is the substrate's intrinsic ultraviolet cutoff. The linear IR exponent is itself a measured FTD result: `ω₁ ∝ L^−0.944` (`s ≈ 1`, cavity/EM-like) — FTD-0270, [`ANALYSIS_QUANTIZATION_LATTICE_MODES_v1.md`](ANALYSIS_QUANTIZATION_LATTICE_MODES_v1.md).

---

## 2 · Light and radio are one flux-wave sector

In FTD there is **no intrinsic distinction** between light and radio. Both are excitations of the same flux wave sector `[DERIVED]`; they differ only by their position `k` on the IR-linear part of the curve. The full classical electromagnetic structure on this sector is derived, not imported:

- retarded lattice Green's function → static geometric Coulomb (FTD-0113, [`DERIV_RETARDED_GREEN_LATTICE.md`](DERIV_RETARDED_GREEN_LATTICE.md); Phase G, FTD-0004) `[DERIVED]`;
- lattice Hodge duality / Bianchi identities (FTD-0114, [`DERIV_LATTICE_HODGE_DUALITY.md`](../electromagnetism/DERIV_LATTICE_HODGE_DUALITY.md)) `[DERIVED]`;
- lattice Liénard–Wiechert for uniform motion, including a genuine **lattice Cherenkov pole** (FTD-0115, [`DERIV_LATTICE_LIENARD_WIECHERT.md`](../electromagnetism/DERIV_LATTICE_LIENARD_WIECHERT.md)) `[DERIVED]`;
- Cherenkov power + sinusoidal Larmor (FTD-0120, [`DERIV_LATTICE_LW_EXTENSIONS.md`](../electromagnetism/DERIV_LATTICE_LW_EXTENSIONS.md)) `[DERIVED / PARTIAL]`;
- the regime map across these (radiation / retardation / static) — [`DERIV_EM_REGIMES_UNIFIED.md`](../electromagnetism/DERIV_EM_REGIMES_UNIFIED.md).

That this sector carries the relativistic structure is a framework commitment: **FC-2** declares the Lorentzian metric an *emergent IR property of the weakly-coupled flux wave dynamics only* (`[AXIOM]`-class declaration, FTD-0256; the two-field reading FTD-0257 makes the flux the primary wave pair). "Light = the flux wave sector" is the `[SELECTION]` that names this carrier.

**The deep-IR consequence (the testable structural statement).** Under `a_phys ≡ ℓ_P` the Brillouin zone extends to `k_zone = π/voxel`, i.e. a minimum wavelength of **2 voxels = `3.23×10⁻³⁵ m`**. Every accessible frequency is therefore astronomically deep in the linear IR (computed in `wave_sector_unit_map.py`):

| Band | frequency | `k/k_zone = 2ℓ_P/λ` | PL-4 size `~(k/k_zone)²` | PL-5 size `~(k/k_zone)⁴` |
|---|---|---|---|---|
| visible | `5×10¹⁴ Hz` | `5.4×10⁻²⁹` | `2.9×10⁻⁵⁷` | `8×10⁻¹¹⁴` |
| FM radio | `1×10⁸ Hz` | `1.1×10⁻³⁵` | `1.2×10⁻⁷⁰` | `1.4×10⁻¹⁴⁰` |

So FTD **structurally predicts no vacuum dispersion** between radio and light: the dispersive and anisotropic corrections (§3) at observable frequencies are suppressed by factors of `~10⁻⁵⁷` (PL-4) and `~10⁻¹¹⁴` (PL-5) `[DERIVED from the dispersion + calibration]`. This is consistent with — and far below — the strongest astrophysical null-dispersion bounds from gamma-ray-burst and fast-radio-burst arrival-time spreads `[OBSERVATION]`. The experimental differences between radio and light (antennas vs. eyes) are entirely matter-coupling differences, not substrate propagation differences.

---

## 3 · The deviation regime — where the lattice departs from the continuum

The bend between IR and zone edge is where FTD's two measured structural deviations live (both registered in [`SPEC_PREDICTION_LEDGER_DEVIATIONS.md`](../../01_reference/SPEC_PREDICTION_LEDGER_DEVIATIONS.md), FTD-0258):

- **PL-4 — moving-clock dilation has a calculable UV bend.** A moving flux wave-clock departs from exact `γ` by a residual `R ∝ L⁻¹·⁹⁸ ≈ L⁻²` (∝ `k²`) on the ⟨100⟩ axis at moderate `v` `[MEASURED, scoped]` (FTD-0252, [`ANALYSIS_DYNAMICAL_TIME_DILATION.md`](ANALYSIS_DYNAMICAL_TIME_DILATION.md)). At fixed `L` the clock over-dilates (bends below `γ`); ultra-relativistic diagonals are `[OBSERVATION/OPEN]`.
- **PL-5 — native UV anisotropy dying as `k⁴`.** The substrate is anisotropic at UV and isotropizes in the IR with exponent `p = 4.0008 ± 0.0006` (`R² = 1.000000`), a dimension-`(D+4)=7` rotation-breaking operator, *strongly irrelevant* by Wilsonian counting `[MEASURED]`.

Both are the same dispersion's fingerprints: the `cos(k/2)` curvature of `v_g` (PL-4, the `k²` bend) and the `O_h` (not rotational) symmetry of the 18-point operator (PL-5, the `k⁴` anisotropy). Under `a_phys ≡ ℓ_P` both are Planck-suppressed (§2 table); their *engine-native* versions are testable now, their physical versions are bounded by Lorentz-violation searches.

---

## 4 · Frequency → physical units

The lattice speaks in mode numbers; the map to SI is the declared calibration interface (`dimensional_map.json`, FTD-0059/0096):

| Quantity | lattice | physical |
|---|---|---|
| length | 1 voxel | `a_phys ≡ ℓ_P = 1.616×10⁻³⁵ m` `[CALIBRATION]` |
| time | 1 tick | `t_phys = ℓ_P/(√3·c) ≈ 3.11×10⁻⁴⁴ s` `[CALIBRATION]` |
| wavenumber | `k` (rad/voxel) | `k/a_phys` (rad/m); wavelength `λ_phys = λ_lattice·ℓ_P` |
| frequency | `ω` (rad/tick) | `ω/t_phys` (rad/s); `ν = ω/(2π·t_phys)` |

The zone-edge cutoff lands at **order the Planck frequency**: `ω_max ≈ 1.24×10⁴³ rad/s` vs the Planck angular frequency `1/t_P ≈ 1.85×10⁴³ rad/s` (equivalently `f_max ≈ 2.0×10⁴² Hz` vs `f_P ≈ 3.0×10⁴² Hz`) — computed in `wave_sector_unit_map.py`. The dimensionless ratios in §2 are calibration-invariant (they depend only on `a_phys`); the absolute Hz/Planck statement rides the declared `t_phys` and inherits its `[CALIBRATION]` status, not a derivation.

---

## 5 · Sound — the structural boundary

Reconnaissance across the corpus finds **no acoustic / phonon sector**: no acoustic dispersion `ω_s(k)`, no displacement field, no elastic moduli, no propagating pressure wave. The wave-lab UI ships a scenario labeled "sound," but the code itself disowns it — *"a slower longitudinal medium proxy; not a material acoustics derivation."* This absence is not an oversight; it is forced by what the lattice **is**.

**The Goldstone argument (the cross-domain reason).** In an ordinary crystal you get *both* light and sound because the crystal is a pattern sitting inside a pre-existing continuum: continuous translation symmetry of empty space is spontaneously broken down to discrete lattice translations, and the **acoustic phonons are the Goldstone modes of that breaking** (the speed of sound is the Goldstone velocity; a displacement field `u(x)` measures how far the atoms sit from their equilibria *in the background space*). FTD's lattice has **no background**. The voxels *are* the coordinate system, not a density modulation within a continuum. There is therefore no spontaneously broken translation symmetry → **no acoustic Goldstone mode → no phonon** `[SELECTION]` + `[BOUNDARY]`. There is nothing the lattice is displaced *relative to*.

Three independent corpus facts make the same statement operationally:

1. **No displacement degree of freedom.** `engine/include/ftd/voxel.h` carries `state`, `flux`, `wave_vel`, and `velocity` (rigid transport, nodes/tick) — but no strain / displacement / elastic field. There is no `u(x)` to obey an acoustic wave equation.
2. **The Gauss constraint projects out the longitudinal mode.** `∇·J = ρ` enslaves the longitudinal flux `J_L` to the source instantaneously, leaving only the **2 transverse** propagating modes (the photon's polarizations). The would-be "longitudinal sound of the flux" is non-dynamical by construction ([`DERIV_BELL_COSINE_FROM_GAUSS.md`](../quantum_mechanics/DERIV_BELL_COSINE_FROM_GAUSS.md); the Helmholtz split is standard FTD electrodynamics).
3. **Genesis is first-order, not critical.** FTD-0272 ([`ANALYSIS_GENESIS_CRITICALITY_v1.md`](../../10_eft_program/ANALYSIS_GENESIS_CRITICALITY_v1.md)) found the manifestation transition strongly first-order — no critical point, no diverging correlation length, no Goldstone mode. A continuous symmetry breaking that would seed a gapless density wave is absent.

The only scalar propagating mode FTD does carry is the Higgs (an oscillation in the flux **magnitude** `|J|` about its VEV) — a *gapped* mode coupled to manifestation, not a gapless displacement/compression wave, so not a sound analog.

**The one open door `[OPEN]`.** A *condensate* compression mode. FTD-0272's manifested phase is a genuine, self-sustaining condensate, and real condensates carry sound. The first-order character argues *against* a gapless density-wave Goldstone, but whether the bulk manifested phase supports any propagating compression mode (gapped or otherwise) is unexplored in the corpus. This is the only place a sound-analog could legitimately live, and it is the new research item this analysis registers (TRACKER_OPEN_ITEMS / SPEC_OPEN_MATH_BY_SECTOR).

---

## 6 · Epistemic summary

| Claim | Tag | Canonical source |
|---|---|---|
| Flux wave equation `∂²J/∂t² = c²∇²J + sources` | `[DERIVED]` | `render_bridge.cpp` |
| `c = C_SPEED = 1/√3` (CFL ceiling); SI value calibration-dependent | `[DERIVED]` / SI `[DECLARED]` | `ontic/gauge_couplings.h` |
| Dispersion `ω(k)=2c|sin(k/2)|`; 18-pt isotropic symbol `M(k)` validated `1.3e-15` | `[DERIVED]` / `[MEASURED]` | FTD-0270 |
| Light and radio = one flux wave sector (only `k` differs) | `[DERIVED]` | FTD-0113/0114/0115/0120 |
| Lorentzian metric = emergent IR property of the flux wave sector | `[AXIOM]`-class declaration | FC-2 (FTD-0256) |
| No vacuum dispersion at observable scales (`k/k_zone ≲ 10⁻²⁸`) | `[DERIVED]`; match to GRB/FRB bounds `[OBSERVATION]` | this doc §2 |
| PL-4 moving-clock UV bend (`∝ k²`, measured `L⁻²`) | `[MEASURED, scoped]` | FTD-0252 / PL-4 |
| PL-5 UV anisotropy (`∝ k⁴`, `p=4.0008`) | `[MEASURED]` | FTD-0258 / PL-5 |
| Frequency→Hz via `t_phys=√3ℓ_P/c`; zone edge ≈ Planck frequency | `[CALIBRATION]` | `dimensional_map.json` |
| No acoustic/phonon sector (lattice is space ⇒ no broken-translation Goldstone) | `[SELECTION]` + `[BOUNDARY]` | this doc §5 |
| Condensate compression mode (possible acoustic-like sector) | `[OPEN]` | this doc §5; FTD-0272 |

**Nothing promoted: FTD-0013 [SMC], MC-T4.3, FC-1/FC-2, FTD-0270/0271/0272 — all unchanged.**
