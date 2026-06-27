# DERIV — The Cosmological Constant as a Scale-Covariant Holographic Ratio

**Tag:** `[SELECTION]` + `[BOUNDARY]` (a *mechanism* for the smallness of Λ; **not** a derivation of its value)
**Date:** 2026-06-26 · **LEDGER:** FTD-0331
**Supersedes the rationale of:** `DERIV_COSMOLOGICAL_CONSTANT.md` (the `m_e⁴·α¹⁶·G*²` / `α⁵⁷` numerology) and reconciles its zero-point basis against FC-1.
**Depends on:** FC-1 (FTD-0255), FC-3 (FTD-0304), the undefined-boundary ontology (`AUDIT_INFINITY_REFRAME.md`), the no-native-length no-go (FTD-0059), the area-law / holographic-principle results (`DERIV_BLACK_HOLE_PHYSICS.md`).

> **Thesis.** FTD does not inherit the quantum-field-theory cosmological-constant *catastrophe*: that `M_Planck⁴` disaster is built from quantum zero-point energy, and FTD's classical substrate declines the apparatus that produces it. The small nonzero Λ that dark energy still requires is forced by FC-3 to be a **scale-ratio**, not a fundamental constant; a holographic saturation fixes the ratio to `Λ ~ (ℓ_P/L)²`. The smallness of Λ is then the largeness of the universe — a mechanism, with no tuned exponent. The numerical *value* remains a boundary: it requires the horizon length `L_H`, which FTD cannot supply natively (FTD-0059). This replaces the `α⁵⁷` constant-match (W-COSMO-4) with an honest, L-dependent account, and lands at `[SELECTION]` + `[BOUNDARY]` — nothing is promoted.

---

## 0 · Why this replaces the existing derivation

The canonical formula in `DERIV_COSMOLOGICAL_CONSTANT.md`, `ρ_Λ = m_e⁴·α¹⁶·G*²` (1.0 % match; the `α⁵⁷` form is its logarithmic mnemonic), is a *value-match*: a fixed mass scale `m_e` multiplied by α-suppression factors. It carries no dependence on any cosmological length, so it cannot express "Λ is small because the universe is large," and the canonical boundary doc already tags it `[PARAMETRIC]` numerology (`SPEC_COSMOLOGY_FRAMEWORK_BOUNDARY.md`, W-COSMO-4). It also rests on a vacuum expectation value `⟨T₀₀⟩ = ½Σ∫ω(k)` — a quantum zero-point sum the classical FTD substrate has no warrant for (§1). This document supplies the *mechanism* the numerology lacks and reconciles the zero-point inconsistency.

---

## 1 · The catastrophe is dissolved, not solved `[DERIVED]` + `[SELECTION]`

The standard cosmological-constant problem is the statement that the QFT vacuum energy,
$$\rho_\text{QFT}=\tfrac{1}{2}\sum_\text{modes}\int\!\frac{d^3k}{(2\pi)^3}\,\sqrt{k^2+m^2}\ \sim\ M_P^4,$$
exceeds the observed value by ~123 orders of magnitude. **This object is entirely quantum:** the `½ℏω` per mode is the ground-state energy of a quantum harmonic oscillator. It exists only given canonical quantization — exactly the apparatus FC-1 (FTD-0255) declines. FTD's substrate is deterministic (Postulate P5) and carries no `ℏ`, no canonical commutator, and no mode quantization.

**The classical empty void is identically zero-energy `[DERIVED]`.** Every term of the FTD field energy density (`SPEC_FTD_LAGRANGIAN.md` §3.6 — field kinetic `½‖Δ_tJ‖²` and gradient `½c²‖∇J‖²`) vanishes at `J=0`; the Born-Infeld rest term `−K_B` applies only to manifested sites (`s≠0`) and is absent in the void (`s=0`). So the vacuum `(J=0,\,s=0)` has exactly zero field energy, with no zero-point floor.

**Consequence:** FTD does not have the `M_Planck⁴` catastrophe at all — it is dissolved by construction, not solved by a suppression chain. A second, independent reason the catastrophe is absent even if one re-imported a mode sum: the discrete lattice has a compact Brillouin zone (`k_max=π/a`), so any mode integral is automatically finite and O(1) in lattice units (the native `[THEOREM]`-grade fact behind `DERIV_VACUUM_ENERGY_CUTOFF.md`; "no trans-Planckian problem", `DERIV_BLACK_HOLE_PHYSICS.md` BH-3).

**Reconciliation (honesty correction).** `DERIV_COSMOLOGICAL_CONSTANT.md` (§2.2, §3.1) and `DERIV_VACUUM_ENERGY_CUTOFF.md` both carry a `½ℏω` zero-point weighting. On the FC-1 reading that weighting has no substrate basis; the finiteness of a Brillouin-zone integral (genuine, `[NUMERICAL FACT]`) is not the same as the substrate possessing a zero-point vacuum energy. Those documents' zero-point starting point is therefore reconciled here as FC-1-inconsistent; per the precedence rule (LEDGER > doc > prose) this account governs.

**Falsifier.** If a defensible classical reading assigns the empty lattice a nonzero *intensive* energy density, the catastrophe is not dissolved and the program reframes from "dissolve" to "solve."

---

## 2 · FC-3 forces Λ to be a scale-ratio, not a constant `[DERIVED-from-FC-3]`

Dark energy is observed (`Ω_Λ≈0.68`, equation of state `w≈−1`), so a *small nonzero* Λ is still owed. Λ has dimension `1/length²`. FC-3 (scale-ratio-covariance, FTD-0304) states that only dimensionless internal ratios are physical and that the lattice spacing `a` (UV scale) and the box length `L` (IR scale) are properties of the observation, not of the substrate. Under FC-3 a dimensionful Λ therefore **cannot be a fundamental constant** — it must reduce to a dimensionless function of the available scales `(ℓ_P, L)`.

The calibration-free object is the pure ratio
$$\Lambda\,\ell_P^2 = f\!\left(\frac{\ell_P}{L}\right),$$
which is the falsifiable spine (`SPEC_DIMENSIONAL_MAP.md` §1, dimensionless layer: "no calibration enters; comparison to lab is direct"). An *absolute* Λ in SI units is calibration-conditional on the declared gauge `a_phys≡ℓ_P` (FTD-0041/0137).

**Boundary of this step.** FC-3 forces "ratio, not constant" and supplies exactly the two scales the holographic argument needs, but it does **not** by itself pick the exponent of `ℓ_P/L`. That requires the saturation law of §3.

---

## 3 · The holographic saturation fixes the exponent to 2 `[SELECTION]`

The Cohen–Kaplan–Nelson UV–IR relation states that in a region of size `L` an effective field theory with UV cutoff `M_P` cannot store more vacuum energy than its largest non-collapsing configuration permits, giving
$$\rho_\text{vac}\ \sim\ \frac{M_P^2}{L^2}\qquad\Longrightarrow\qquad \Lambda\ \sim\ \left(\frac{\ell_P}{L}\right)^{2}\frac{1}{\ell_P^2}=\frac{1}{L^2}.$$
The exponent 2 is the holographic *degree-of-freedom bound* (entropy/information scaling as area, not volume), not a free choice. Adopted here as `[IMPOSED — motivated]` in the project's endorsed impose-with-motivation discipline.

**Can FTD force the exponent natively?** FTD already carries the ingredients:
- a native UV cutoff (the compact Brillouin zone), §1;
- the **holographic principle from determinism** (`DERIV_BLACK_HOLE_PHYSICS.md` BH-5, `[THEOREM]`): interior configurations are fixed by boundary data, so the independent degrees of freedom in a region scale with its boundary area;
- an **area law** `S = A/(4ℓ_P²)` (BH-4, `[SELECTION]`; the area-scaling is `[THEOREM]` from determinism, the coefficient `1/4` is a `[SELECTION]` argument and is alphabet-invariant).

If the independent degrees of freedom in a region scale as the area, the vacuum energy saturates at the CKN value `M_P²/L²` rather than the naive `M_P⁴` — i.e. FTD's own determinism-holography *motivates* the exponent 2. Because the area-law coefficient is itself `[SELECTION]`, this native motivation is `[SELECTION]`-grade, not a theorem; it is a genuine reduction of the import, not its elimination.

**Falsifier (a real, in-corpus tension).** The exponent is load-bearing, and FTD's measured energetics are not yet known to respect it. `DERIV_LATTICE_BLACK_HOLES.md` / FTD-0273 found that the manifested-condensate energy does **not** settle to a clean intensive density — it leaks as `~L⁻⁵` on larger boxes. If the substrate's vacuum energetics followed a volume-law or a leak-dominated scaling rather than the area law, the exponent would not be 2 (e.g. `(ℓ_P/L)⁴` or steeper), and the holographic reading would be disfavored. Reconciling the area-law motivation against the FTD-0273 leak scaling is the decisive open check; a measured volume-law would close this route negative.

---

## 4 · The value is a boundary `[BOUNDARY]`

Combining §1–§3:
$$\boxed{\ \Lambda\,\ell_P^2\ \sim\ \left(\frac{\ell_P}{L_H}\right)^{2}\ }$$
with `L_H` the undefined-boundary IR scale identified with the cosmological horizon. Numerically `L_H ≈ 8.4×10⁶⁰ ℓ_P`, so `(ℓ_P/L_H)² ≈ 1.4×10⁻¹²²` and `Λ·L_H² ≈ 2 = O(1)` — i.e. the observed `Λ·ℓ_P² ≈ 2.8×10⁻¹²²` is reproduced as a ratio, and the famous "123 orders of magnitude" is simply `(ℓ_P/L_H)²`: **the smallness of Λ is the largeness of the universe in Planck units.**

But this is a mechanism, not a prediction of the number. Obtaining the value requires `L_H/ℓ_P`, and FTD-0059 (`[THEOREM]`) proves that **no native length is expressible from Axiom-Zero invariants** (all four mechanism candidates α/β/γ/δ closed negative). So FTD cannot predict `L_H/ℓ_P`; the horizon is an external IR input, exactly parallel to `a_phys≡ℓ_P` being a declared UV gauge. Two further imports are explicit: the identification of the undefined-boundary box `L` with the physical horizon `L_H` (an ontological promotion of an apparatus scale), and the holographic saturation of §3.

**Net:** mechanism — yes (Λ is a scale-ratio, `Λ~1/L²`, no tuned exponent); value — no (`L_H` imported; FTD-0059 boundary). The equation of state `w=−1` (a pure cosmological constant, no time variation) is the modest bonus the scale-ratio reading gives.

---

## 5 · Epistemic status and what would upgrade it

| Claim | Tag |
|---|---|
| Classical void is zero-energy; FTD has no `M_Planck⁴` catastrophe | `[DERIVED]` (FC-1 + Lagrangian) |
| The catastrophe is dissolved-by-construction; existing `½ℏω` route is FC-1-inconsistent | `[SELECTION]` / reconciliation |
| Λ must be a scale-ratio `Λ·ℓ_P² = f(ℓ_P/L)`, not a fundamental constant | `[DERIVED-from-FC-3]` |
| The exponent is 2 (`Λ~(ℓ_P/L)²`), holographic saturation | `[SELECTION]` (motivated by determinism-holography; imported as CKN) |
| The numerical value `Ω_Λ≈0.68`, `~10⁻¹²²` | `[BOUNDARY]` (needs `L_H`; FTD-0059) |

**What would upgrade to `[DERIVED]`:** a rigorous derivation of the holographic degree-of-freedom saturation from FTD's determinism-holography (rather than importing the CKN bound), together with a resolution of the FTD-0273 leak-scaling tension in favour of the area law. Neither is in hand.

**Zero promotions.** `x₊=1/α` stays `[SMC]` (FTD-0013); MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`; no α is derived; the value of Λ is **not** derived. The deliverable is a mechanism for the *smallness* plus a clearly-marked value boundary — a Number-One-Goal second-clause result, replacing the rationale of the `α⁵⁷`/`α¹⁶` numerology.

---

## 6 · The numerology trap, explicitly avoided

Cosmology is the framework's most W-CRIT-1-vulnerable sector. This account stays honest by three commitments stated in advance: it targets only the dimensionless ratio `Λ·ℓ_P²`; it declares `L_H` an explicit external input rather than a prediction; and it never reverse-engineers `L_H` from the observed Λ and presents the round-trip as a derivation. No exponent is tuned — the `2` is the holographic dimension, not a fit.

---

## 7 · Cross-references

- `DERIV_COSMOLOGICAL_CONSTANT.md` — the `m_e⁴·α¹⁶·G*²` / `α⁵⁷` numerology whose rationale this supersedes and whose `½ℏω` basis this reconciles.
- `DERIV_VACUUM_ENERGY_CUTOFF.md` — the compact-BZ finiteness `[NUMERICAL FACT]` (UV side); its `½ℏω` weighting reconciled here.
- `SPEC_SCALE_RATIO_ONTOLOGY.md` (FC-3, FTD-0304) — only ratios are physical; the `(a, L)` scale pair.
- `AUDIT_INFINITY_REFRAME.md` — the undefined-boundary ontology and the IR cutoff; forbids the completed-infinity vacuum sum.
- `DERIV_BLACK_HOLE_PHYSICS.md` — holographic-principle-from-determinism (BH-5, `[THEOREM]`) and the area law (BH-4, `[SELECTION]`).
- `SPEC_DIMENSIONAL_MAP.md` §1 + FTD-0059 — the dimensionless-ratio layer and the no-native-length no-go (the value boundary).
- `SPEC_COSMOLOGY_FRAMEWORK_BOUNDARY.md` — W-COSMO-4, updated: a mechanism now exists; the value does not.
