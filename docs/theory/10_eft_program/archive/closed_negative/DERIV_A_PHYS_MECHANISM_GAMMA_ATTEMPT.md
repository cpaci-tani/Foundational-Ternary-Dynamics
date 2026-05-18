# Mechanism γ — Attempt to Fix `a_phys` from the Gravitational Coupling

> **[CLOSED NEGATIVE 2026-04-19]** — Mechanism γ does not deliver a first-principles `a_phys`. The framework's disposition is the fallback in §4: declare `a_phys ≡ ℓ_P` as a calibration. This declaration is now authoritative in `docs/SPEC_FTD.md` ("LATTICE ↔ PHYSICAL CALIBRATION") and recorded in `docs/theory/07_assessment/LEDGER.md` as rows **FTD-0030 (RESOLVED-BY-CALIBRATION)** and **FTD-0041 (CALIBRATION)**. See also `docs/WHERE_WE_LEFT_OFF.md` §4.
>
> **Note on a later document.** A file named `../retracted/DERIV_A_PHYS_MECHANISM_GAMMA_SUCCESS.md` claims to supersede this one with a `[THEOREM]` tag and `a_phys ≈ 4.39 ℓ_P`. That claim was **retracted on 2026-04-23** (see preamble in that file): its "derivation" silently swaps the `K_B = m_e` mass calibration for `ℏ_lat = 1`, which is the same class of calibration-shuffle this ATTEMPT document already identified (§2.3). The authoritative closure is the one in this document and in FTD-0030/0041, not the retracted SUCCESS claim.

**Tag:** [CLOSED NEGATIVE] — no first-principles `a_phys`; reverts to calibration.
**Status:** **Closed.** Fallback declared in `docs/SPEC_FTD.md` (2026-04-19).

---

## 1 · The dimensional chain

We want a single equation that fixes `a_phys` (one lattice-distance unit, in metres) from invariants the framework already produces.

The framework supplies:

| Quantity | Lattice value | Source |
|---|---|---|
| `c_lat` (lattice CFL speed) | `1/√3` voxels/tick | CFL stability, [THEOREM] |
| `K_B` (manifestation threshold) | `0.511` lattice mass-units | parameter set to match `m_e` (parametric) |
| `G_N(lattice)` | `1/(b_3+N_c)² = 0.01` lattice gravitational-coupling units | [SELECTION], `engine/include/ftd/ontic/gauge_couplings.h` |

Two physical anchors must be read in:

- **`c`-anchor:** `c_lat × (a_phys / t_phys) = c_phys = 2.998 × 10⁸ m/s`. Hence `t_phys = a_phys × √3 / c_phys`.
- **`K_B`-anchor:** the framework already calibrates `K_B = 0.511 lattice` ↔ `m_e = 0.511 MeV/c²`. So one lattice mass-unit corresponds to `M_unit = m_e / K_B = 0.511 MeV / 0.511 = 1 MeV/c² ≈ 1.783 × 10⁻³⁰ kg`.

These two anchors are the only ingredients besides `G_N(lattice)`. Dimensional analysis on Newton's constant:

$$[G_N] \;=\; \frac{L^3}{M\,T^2}$$

gives one equation:

$$G_N(\text{phys}) \;=\; G_N(\text{lattice}) \cdot \frac{a_\text{phys}^3}{M_\text{unit} \cdot t_\text{phys}^2}.$$

Substituting `t_phys = a_phys × √3 / c_phys`:

$$G_N(\text{phys}) \;=\; G_N(\text{lattice}) \cdot \frac{a_\text{phys}^3 \cdot c_\text{phys}^2}{3\, a_\text{phys}^2 \cdot M_\text{unit}} \;=\; \frac{G_N(\text{lattice})\, c_\text{phys}^2\, a_\text{phys}}{3\, M_\text{unit}}.$$

Solving for `a_phys`:

$$\boxed{\;a_\text{phys} \;=\; \frac{3\, M_\text{unit}\, G_N(\text{phys})}{G_N(\text{lattice})\, c_\text{phys}^2}\;}$$

This is **one equation in one unknown**, given the framework's existing calibrations. Plugging in:

- `M_unit = 1.783 × 10⁻³⁰ kg`
- `G_N(phys) = 6.674 × 10⁻¹¹ m³ kg⁻¹ s⁻²`
- `G_N(lattice) = 0.01`
- `c_phys² = 8.988 × 10¹⁶ m²/s²`

gives

$$a_\text{phys} \;\approx\; \frac{3 \cdot (1.783\!\times\!10^{-30}) \cdot (6.674\!\times\!10^{-11})}{0.01 \cdot (8.988\!\times\!10^{16})} \;\approx\; 4.0 \times 10^{-55}\;\text{m}.$$

---

## 2 · What this means

`4 × 10⁻⁵⁵ m` is **20 orders of magnitude smaller than the Planck length** `ℓ_P ≈ 1.616 × 10⁻³⁵ m`. It is also smaller than any length scale physics has reason to invoke, by a factor of `~10²⁰`.

This result has three possible readings:

1. **`G_N(lattice) = 0.01` is the wrong number to plug in.** The engine's own header explicitly flags this: "the engine runs in a TOY-GRAVITY REGIME where `G_N ≈ 0.01` — roughly 37 orders of magnitude stronger than physical gravity." If `G_N(lattice)` were the framework's actual prediction for the dimensionless gravitational coupling at electron-mass scale, it would be `α_G ≈ 5.91 × 10⁻³⁹` (the value that replaces `0.01` in the formula above). Re-running with `G_N(lattice) = α_G`:

$$a_\text{phys}^{\text{(α_G route)}} \;=\; \frac{3 \cdot (1.783\!\times\!10^{-30}) \cdot (6.674\!\times\!10^{-11})}{(5.91\!\times\!10^{-39}) \cdot (8.988\!\times\!10^{16})} \;\approx\; 6.7\times 10^{-19}\;\text{m},$$

   which is roughly attometre-scale — sixteen orders of magnitude smaller than the Planck length, and not an atomic length either. **Equally implausible** as a fundamental lattice spacing. (Caveat: this calculation conflates mass scales — α_G ≈ 5.91 × 10⁻³⁹ in the framework's derivation is keyed to the proton-mass scale via the cross-domain α²⁰ factor, while `M_unit = m_e` here is electron-scale; the mixing further flags the chain as a calibration shuffle rather than a derivation.)

2. **The `K_B = m_e` calibration is doing too much work.** `M_unit` is set by demanding `K_B = m_e`, which is a parametric calibration to the electron mass. If we instead anchored mass by setting `K_B` to something else (e.g. the QCD scale Λ_QCD, or the Planck mass), `M_unit` and hence `a_phys` would shift by orders of magnitude. The result is therefore not a derivation of `a_phys` — it is an output that depends on which mass observable was used to calibrate `K_B`.

3. **The dimensional chain is not closed by Axiom-Zero ingredients alone.** What we have is `c` (the dimensionless lattice CFL value), `K_B` (a dimensionless manifestation-threshold value calibrated to a physical mass), and `G_N(lattice)` (a dimensionless number whose physical-units interpretation is **not** fixed by Axiom Zero). The "derivation" of `a_phys` is in fact a derivation of *the conversion ratio that makes the chosen calibrations mutually consistent*. That is calibration, not derivation.

---

## 3 · Why this does not constitute a derivation

A genuine Mechanism-γ derivation would require:

- a value of `G_N(lattice)` that is **dimensionally meaningful at the lattice level** without first fixing `M_unit` from a physical observable, AND
- a definition of `M_unit` (one lattice mass-unit) that is **forced by the framework's own combinatorics**, not chosen so that `K_B = m_e`.

Neither is currently in hand:

- `G_N(lattice) = 1/(b_3+N_c)²` is a finite-combinatorial number, but its **dimensional interpretation** (as a coefficient in Newton's law on the lattice) requires choosing units. The engine declares the choice that makes gravity simulable on `~100³` lattices; that is an explicit modelling convenience, not a forced consequence.
- `M_unit` is currently **defined** by the calibration `K_B = m_e`. Without that calibration, the framework has no mass-scale. Hence `M_unit` is empirical, and any subsequent "derived" quantity that depends on it inherits empirical status.

The dimensional chain in §1 is therefore best read as

> *given* the engine's choice for `G_N(lattice)` and *given* the calibration `K_B = m_e`, the conversion that reproduces `G_phys` is a definite number.

Both "given"s are external to Axiom Zero. The chain converts one calibration into another; it does not derive a calibration from nothing.

---

## 4 · What this leaves

Mechanism γ is **closed as a candidate first-principles derivation**. The honest disposition for `a_phys` therefore reverts to the fallback in [OPEN_A_PHYS_DERIVATION.md §4](../resolved/OPEN_A_PHYS_DERIVATION.md): declare `a_phys` as a calibrated empirical parameter, name the matching observable used to fix it, and quote all dimensional predictions as conditional on the calibration.

Three reasonable choices for the matching observable:

| Calibration choice | Result | Property |
|---|---|---|
| `K_B = m_e` and `G_N(lattice) = 0.01` self-consistent | `a_phys ≈ 4 × 10⁻⁵⁵ m` | unphysically small; signals `G_N(lattice) = 0.01` is toy-regime, not physical |
| `K_B = m_e` and `G_N(lattice) = α_G` | `a_phys ≈ 6.7 × 10⁻¹⁹ m` | attometre-scale; signals `α_G` derivation + cross-mass-scale conflation needs revisiting |
| `K_B = m_e` and `a_phys ≡ ℓ_P` (declared) | `G_N(lattice)` forced to ≈ `1.6 × 10⁻²⁵` | preserves Planck-scale ontology; `G_N(lattice) = 0.01` becomes a separate engine-toy parameter |

**Recommended position:** declare `a_phys ≡ ℓ_P` (Planck-length anchor) in `SPEC_FTD.md` as the framework's working calibration, treat `G_N(lattice) = 0.01` as the engine's deliberate toy-coupling for visibility, and quote all engine results in lattice units with the conversion `ℓ_P = a_phys` available for any reader who wants physical units. This matches the engine's existing TOY-regime banner and makes the calibration choice explicit instead of implicit.

---

## 5 · One-paragraph summary

Working through the dimensional chain `G_N(phys) = G_N(lattice) · a_phys³ / (M_unit · t_phys²)` with the framework's existing calibrations (`c_lat = 1/√3` and `K_B = m_e`) produces a definite value for `a_phys` — but the value depends on which `G_N(lattice)` one substitutes. Using the engine's toy-regime `0.01` gives `~4 × 10⁻⁵⁵ m` (clearly unphysical); using the physical-coupling `α_G ≈ 5.91 × 10⁻³⁹` gives `~7 × 10⁻⁷ m` (also implausible). Neither result is forced by Axiom Zero, because both `G_N(lattice)` and `M_unit` are themselves calibrations rather than derived invariants. Mechanism γ therefore **does not deliver a first-principles `a_phys`**. The honest disposition (per `OPEN_A_PHYS_DERIVATION.md §4`) is to declare `a_phys` as a calibrated parameter — the recommendation is to anchor on `a_phys ≡ ℓ_P` — and to flag every dimensional prediction as conditional on that calibration.

---

## 6 · Reproducibility

```
docs/theory/10_eft_program/archive/resolved/OPEN_A_PHYS_DERIVATION.md           # the open problem
docs/theory/10_eft_program/archive/closed_negative/DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md   # this attempt
engine/include/ftd/ontic/gauge_couplings.h                     # G_N(lattice) = 0.01 with TOY banner
engine/include/ftd/constants.h                                 # K_B, ALPHA_G_APPROX
```
