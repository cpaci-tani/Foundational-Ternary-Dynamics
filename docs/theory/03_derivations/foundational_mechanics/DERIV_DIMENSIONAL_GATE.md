# DERIV — The Dimensionless → Dimensionful Gate

**Tag:** [DERIVED — schema-level] for the gate map (rides on the grade-0 closure theorem, FTD-0368); [CORRECTION] for the `t_phys` value (fixes a factor-3 arithmetic error propagated from FTD-0041, 2026-07-08). Promotes no physics claim and moves no epistemic tag.
**LEDGER:** maintenance-log line; content rides on FTD-0059 (length no-go), FTD-0096 (mass no-go), FTD-0368 (grade-0 closure). The `t_phys` correction touches FTD-0041.
**Verification:** numeric checks reproduced inline (mpmath); the underlying closure is `scripts/proofs/proof_dimensional_grade_closure.py` (8/8).
**Audience:** anyone asking "how does FTD turn a pure number into a metre / second / kilogram, and how many things must it import to do so?"

---

## §0 — What this document is

FTD's native content is **dimensionless** (states, tick-counts, lattice-unit flux, and the constants `G*, α, N_c, c_lat = 1/√3` are all pure numbers — the grade-0 closure theorem, `FOUND_DIMENSIONAL_GRADE_CLOSURE.md`, FTD-0368). Yet the framework quotes electron masses in MeV and lengths in metres. The **gate** is the map that gets from the first to the second. This note states it operationally, classifies its three slots as *defined* or *derived*, and records a correction to the time slot's value.

This is the **"mark and price the imported types" face of the Number-One Goal, made concrete**: the gate is exactly the surface where the substrate must import what it cannot set — the absolute scales.

---

## §1 — The gate is exactly 3-dimensional (forced)

Physical quantities form a group **graded by ℤ³** — each carries a dimension exponent `(a, b, c)` for `(M, L, T)`. A unit system is a group homomorphism `φ : ℤ³ → ℝ₊` assigning a positive scale to each base dimension; a homomorphism out of ℤ³ is fixed by its values on the **3 generators**. Hence:

> **The gate requires exactly 3 dimensionful calibration constants — one per base dimension. No fewer** (an unfixed base dimension leaves a residual scaling freedom) **and no more** (a fourth is redundant, constrained by a consistency relation).

Electromagnetism adds **no** fourth slot: in FTD's convention `e = √(ℏcα)` with `α` dimensionless, so charge is mechanical (`[e²] = M L³ T⁻²`). This is the operational reading of grade-0 closure (FTD-0368 §2): every native rule is grade-homogeneous, so the whole substrate stays grade-`(0,0,0)`, and *any* dimensional prediction must factor as

$$Q_{\text{SI}} = \hat q \cdot \mu^{a}\,\lambda^{b}\,\tau^{c},$$

a **dimensionless native number `q̂`** times a **calibration monomial**. The exponents `(a,b,c)` are read off `Q`'s dimensions; the map `q̂ ↦ Q` is a graded homomorphism from grade-0 into grade-`(a,b,c)`.

---

## §2 — The three slots: two defined, one derived

| Slot | Symbol | Status | Basis |
|---|---|---|---|
| **Length** | `λ = a_phys ≡ ℓ_P` | **DEFINED** (imported) | `THEOREM_A_PHYS_NO_GO` (FTD-0059): *no length is derivable from Axiom Zero* — proven, not merely untried. |
| **Mass** | `μ = m_e/K_B = 1 MeV/c²` | **DEFINED** (imported) | `THEOREM_MU_NO_GO_FTD0096` (FTD-0096): *no mass is derivable*, including from the `K_GENESIS` thresholds. |
| **Time** | `τ = t_phys` | **DERIVED** | fixed by `λ` + the dimensionless CFL Courant speed `c_lat = 1/√3` ([THEOREM]) + measured `c`. Not an independent import. |

The intuitive picture: **length and mass are *defined* because the substrate provably cannot set any absolute scale (grade-0 closure); time is *derived* because once the length scale and the lattice's own dimensionless speed are fixed, the tick follows.** Two imports, one derivation.

**Ledger consequence.** The priced-import ledger's IMP-K2 (`t_phys`) is therefore **not an independent import** — it is `[DERIVED from IMP-K1 + c_lat]`. The genuine dimensional imports number **two** (length, mass), plus the physical value of `c` that bridges L and T. This tightens the boundary by one line in FTD's favour. And of those two, the *length* import can itself be dropped: anchoring the electron mass and deriving the Planck scale + `G` from the α-ladder leaves a single beyond-universal import — the **electron-primary gauge**, `FOUND_ELECTRON_PRIMARY_GAUGE.md`, the recommended proper entry.

---

## §3 — Deriving the time slot (and correcting its value)

The CFL Courant number on the cubic lattice is `c_lat = 1/√3` — the dimensionless number of voxels a signal advances per tick, a [THEOREM] from the leapfrog wave equation (`C_SPEED = 0.57735…` in `engine/.../gauge_couplings.h`). Converting to SI, the physical speed of light is

$$c = c_{\text{lat}}\cdot\frac{a_{\text{phys}}}{t_{\text{phys}}} = \frac{1}{\sqrt3}\cdot\frac{\ell_P}{t_{\text{phys}}}
\;\;\Longrightarrow\;\;
\boxed{\,t_{\text{phys}} = \frac{\ell_P}{\sqrt3\,c} = \frac{t_P}{\sqrt3} \approx 3.11\times10^{-44}\ \text{s}\,}$$

where `t_P = ℓ_P/c = 5.391×10⁻⁴⁴ s` is the SI Planck time.

**Three independent derivations all give `t_P/√3`:** (i) the Courant-number relation above; (ii) a signal at the CFL max advances `1/√3` voxel per tick, so crossing one voxel takes `√3` ticks, i.e. `t_phys = t_P/√3`; (iii) von-Neumann stability `c·dt/dx ≤ 1/√D` at the boundary gives `dt = dx/(√3\,c)`.

### §3.1 — Correction of record (2026-07-08)

Prior to this note the corpus quoted `t_phys = √3·ℓ_P/c = √3·t_P ≈ 9.34×10⁻⁴⁴ s` — the reciprocal factor. Substituting it back gives a physical light speed of `c/3`, contradicting FTD's own headline result `c = c_lat = 1/√3`. The `√3·t_P` value corresponds to a *different, unstated* convention (tick ≡ body-diagonal light-crossing time, under which the per-axis speed is `c/√3`, not `c_lat = 1/√3`); it is inconsistent with how `C_SPEED = 1/√3` is used as the Courant number in the engine leapfrog. Resolution adopted: **match the engine and the Courant derivation-of-record**, giving `t_phys = t_P/√3`.

Corrected in: `engine/web/js/constants.js` (`FTD_TICK_S`), `units.js`, `dimensional_map.json` (+ rendered map), `SPEC_FTD.md`, the constitution `SPEC_FTD_FRAMEWORK_V1.md`, `LEDGER.md` (FTD-0041), `SPEC_IMPORT_LEDGER.md` (IMP-K2), `CLAUDE.md`, `FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md`, and the active secondary specs. **Historical/archived/changelog records (e.g. `CHANGELOG_REFRAME.md`, clock-hypothesis audits, superseded pre-registrations) are left intact as provenance** — they correctly record what was believed at their date. **No dimensionless prediction changes** (the falsifiable spine is gauge-invariant); only absolute tick↔second conversions, and any tick-count derived from them (e.g. the Class-B muon-lifetime tick count, `τ_μ/t_phys`, triples).

---

## §4 — The gate, verified

For `Q` with dimension `(a,b,c)`, `Q_SI = q̂ · μ^a λ^b τ^c`. Two worked passes (numerics reproduced with mpmath):

- **Length — reduced Compton wavelength `ƛ_C`** (grade `(0,1,0)`): native `q̂ = ƛ_C/ℓ_P = α⁻¹¹/K` with `K = √(2π)·16/3`. Gate: `q̂·ℓ_P = 3.87×10⁻¹³ m` vs experiment `3.86×10⁻¹³ m` — **0.19%**.
- **Mass — electron** (grade `(1,0,0)`): `q̂ = m_e/m_P = K·α¹¹`. Gate: `q̂·m_P = 0.510 MeV` vs `0.511 MeV` — **0.19%**.

Same 0.19% both times — it is the *same* dimensionless number (`K·α¹¹`) read once as a mass and once (inverted) as a length. That the two land consistently is the gate working; the residual is the electron-mass ladder's accuracy, not the gate's.

---

## §5 — Epistemic discipline

The gate is exact algebra on top of established theorems; it introduces no new physics claim. `q̂` for each observable inherits its own tag (`α`'s identification stays `[SMC]`; the `√(2π)` prefactor stays `[SELECTION]`; mass-ratio `q̂`s stay `[PARAMETRIC]`). Being dimensionless marks the *falsification surface*; it does not upgrade any claim to derived. The `t_phys` change is a **correction of an arithmetic error**, not a re-derivation — it makes "time is derived from `c_lat = 1/√3`" true, which it previously was not.

## §6 — Cross-references

`FOUND_DIMENSIONAL_GRADE_CLOSURE.md` (FTD-0368, the grade-0 conservation theorem this operationalizes); `THEOREM_A_PHYS_NO_GO.md` (FTD-0059); `THEOREM_MU_NO_GO_FTD0096.md` (FTD-0096); `SPEC_DIMENSIONAL_MAP.md` + `dimensional_map.json` (the three-layer catalog); `SPEC_IMPORT_LEDGER.md` (IMP-K1/K2/K3 pricing); `FOUND_MODULUS_ARGUMENT_FRONTIER.md` (the qualitative boundary this is the dimensional face of); `FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md` (FTD-0137, the four gauge choices); `FOUND_ELECTRON_PRIMARY_GAUGE.md` (the recommended G-free entry into dimensionful units).
