# DERIV — The Cosmological Constant as a Scale-Covariant Holographic Ratio

**Tag:** `[SELECTION]` + `[BOUNDARY]` (a *mechanism* for the smallness of Λ; **not** a derivation of its value)
**LEDGER:** FTD-0331
**Supersedes the rationale of:** `DERIV_COSMOLOGICAL_CONSTANT.md` (the `m_e⁴·α¹⁶·G*²` / `α⁵⁷` numerology) and reconciles its zero-point basis against FC-1.
**Depends on:** FC-1 (FTD-0255), FC-3 (FTD-0304), the undefined-boundary ontology (`AUDIT_INFINITY_REFRAME.md`), the no-native-length no-go (FTD-0059), the area-law / holographic-principle results (`DERIV_BLACK_HOLE_PHYSICS.md`).

> **Thesis.** FTD does not inherit the quantum-field-theory cosmological-constant *catastrophe*: that `M_Planck⁴` disaster is built from quantum zero-point energy, and FTD's classical substrate declines the apparatus that produces it — so FTD dissolves the *old* problem, and its classical vacuum is exactly zero-energy. For any *nonzero* Λ, FC-3 forces the **form** (a scale-ratio, not a fundamental constant) and the holographic bound fixes the **ceiling** (`Λ ≲ (ℓ_P/L)²` — smallness = largeness, no tuned exponent). But FTD supplies no **source** to fill that ceiling: its own dynamics predicts `Λ = 0`, and the candidate sources fail (zero-point declined by FC-1; the manifested condensate leaks `~L⁻⁵`). So the small nonzero dark energy is an `[OPEN]` source gap, and its numerical *value* is a `[BOUNDARY]` requiring the horizon `L_H`, which FTD cannot supply natively (FTD-0059). This replaces the `α⁵⁷` constant-match (W-COSMO-4) with an honest account — dissolution `[DERIVED]`, form `[DERIVED-from-FC-3]`, source `[OPEN]`, value `[BOUNDARY]` — and promotes nothing.

---

## 0 · Why this replaces the existing derivation

The canonical formula in `DERIV_COSMOLOGICAL_CONSTANT.md`, `ρ_Λ = m_e⁴·α¹⁶·G*²` (1.0 % match; the `α⁵⁷` form is its logarithmic mnemonic), is a *value-match*: a fixed mass scale `m_e` multiplied by α-suppression factors. It carries no dependence on any cosmological length, so it cannot express "Λ is small because the universe is large," and the canonical boundary doc already tags it `[PARAMETRIC]` numerology (`SPEC_COSMOLOGY_FRAMEWORK_BOUNDARY.md`, W-COSMO-4). It also rests on a vacuum expectation value `⟨T₀₀⟩ = ½Σ∫ω(k)` — a quantum zero-point sum the classical FTD substrate has no warrant for (§1). This document supplies the *mechanism* the numerology lacks and reconciles the zero-point inconsistency.

---

## 1 · The catastrophe is dissolved, not solved `[DERIVED]` + `[SELECTION]`

The standard cosmological-constant problem is the statement that the QFT vacuum energy,
$$\rho_\text{QFT}=\tfrac{1}{2}\sum_\text{modes}\int\!\frac{d^3k}{(2\pi)^3}\,\sqrt{k^2+m^2}\ \sim\ M_P^4,$$
exceeds the observed value by ~123 orders of magnitude. **This object is entirely quantum:** the `½ℏω` per mode is the ground-state energy of a quantum harmonic oscillator. It exists only given canonical quantization — exactly the apparatus FC-1 (FTD-0255) declines. FTD's substrate is deterministic (Postulate P5) and carries no `ℏ`, no canonical commutator, and no mode quantization.

**The classical empty void is identically zero-energy `[DERIVED]`.** Every term of the FTD field energy density (`SPEC_FTD_LAGRANGIAN.md` §3.6 — field kinetic `½‖Δ_tJ‖²` and gradient `½c²‖∇J‖²`) vanishes at `J=0`. The Born-Infeld rest term is excluded from the canonical gravity-sourcing tensor: `T^{\mu\nu}` (EFE-4, `[THEOREM]`, `DERIV_EINSTEIN_FIELD_EQUATIONS.md` §2.2) is derived from the free-field Lagrangian alone, and its Born-Infeld upgrade is a separate, undeployed `[SELECTION]` (`DERIV_EINSTEIN_FIELD_EQUATIONS.md` §2.4). So the gravitating vacuum energy is exactly zero, with no zero-point floor.

> **Correction (FTD-0835, this review).** This paragraph previously justified the zero-energy claim by asserting the Born-Infeld rest term `−K_B` is gated by `s` and absent in the void. That is false: `SPEC_FTD_LAGRANGIAN.md` §3.3 (FTD-0567) and the engine's own `test_born_infeld.cpp` ("H at rest = E_REST") confirm the term is evaluated at every voxel independent of `s` — a literal vacuum voxel carries `H_BI = E_REST ≠ 0`. The zero-*gravitating*-vacuum conclusion above survives regardless, because the canonical `T^{\mu\nu}` this document's Λ argument actually depends on (EFE-4) never included the Born-Infeld sector in the first place. The correction is to the stated justification, not the claim it supported. Flagged rather than silently fixed, per this repo's correction policy.

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

If the area-many degrees of freedom each carry the infrared energy `~1/L`, the vacuum energy reaches `M_P²/L²` rather than the naive `M_P⁴` — so FTD's determinism-holography *motivates* the exponent 2. But this fixes only the **ceiling**. The CKN relation is the *maximum* vacuum energy a region of size `L` can hold without collapsing to a black hole (`E_vac ≤ M_BH(L) ~ M_P²·L`); it is a bound, not a source — it states what Λ could be *at most*, not that it is nonzero.

**The decisive gap (gap #1, examined): FTD has no source to fill the ceiling, and saturating it reintroduces what §1 declined.** The per-mode infrared energy `~1/L` the ceiling requires *is* a zero-point (`½ℏω_min`) contribution — exactly what §1 sets to zero. So FTD's classical vacuum does not saturate the holographic ceiling; it sits at the floor, `ρ_vac = 0`. **FTD's own dynamics therefore predicts `Λ = 0`:** it dissolves the *old* cosmological-constant problem (why is Λ not huge) but does not produce the *new* one (a small nonzero dark energy). Filling the vacuum up to the ceiling — the saturation that holographic-dark-energy models simply *assume* — needs a nonzero source, and none of FTD's candidates survives:
- a zero-point mode energy is forbidden by §1 (FC-1);
- the manifested condensate (FTD-0272) does not settle to a clean intensive density — its energy leaks as `~L⁻⁵` (FTD-0273);
- the matter-driven flux injection of `DERIV_DARK_SECTOR_DYNAMICS.md` carries no `L`-dependence (it reproduces the `α¹⁶` number, not a holographic ratio).

**Verdict on §3.** FC-3 fixes the *form* (a scale-ratio) and the area law fixes the *ceiling* (the exponent 2), but the **source — and hence the actual nonzero value — is `[OPEN]`**: FTD predicts `Λ = 0`, and the observed `Λ > 0` saturating the ceiling is, at present, a coincidence-match rather than an FTD output. The clean closing move would be a substrate vacuum-energy density that is intensive and obeys the area law instead of leaking; the FTD-0273 `L⁻⁵` result currently disfavours it.

---

## 4 · The value is a boundary `[BOUNDARY]`

Combining §1–§3:
$$\boxed{\ \Lambda\,\ell_P^2\ \sim\ \left(\frac{\ell_P}{L_H}\right)^{2}\ }$$
with `L_H` the undefined-boundary IR scale identified with the cosmological horizon. Numerically `L_H ≈ 8.4×10⁶⁰ ℓ_P`, so `(ℓ_P/L_H)² ≈ 1.4×10⁻¹²²` and `Λ·L_H² ≈ 2 = O(1)` — i.e. the observed `Λ·ℓ_P² ≈ 2.8×10⁻¹²²` is reproduced as a ratio, and the famous "123 orders of magnitude" is simply `(ℓ_P/L_H)²`: **the smallness of Λ is the largeness of the universe in Planck units.**

But this is a mechanism, not a prediction of the number. Obtaining the value requires `L_H/ℓ_P`, and FTD-0059 (`[THEOREM]`) proves that **no native length is expressible from Axiom-Zero invariants** (all four mechanism candidates α/β/γ/δ closed negative). So FTD cannot predict `L_H/ℓ_P`; the horizon is an external IR input, exactly parallel to `a_phys≡ℓ_P` being a declared UV gauge. Two further imports are explicit: the identification of the undefined-boundary box `L` with the physical horizon `L_H` (an ontological promotion of an apparatus scale), and the holographic saturation of §3.

**Net:** the *form* is fixed (Λ is a scale-ratio, `Λ~1/L²`), but FTD supplies neither the *source* (§3 — it predicts `Λ = 0`) nor the *value* (`L_H` imported; FTD-0059 boundary). The equation of state is **not** a free bonus (an earlier draft over-stated this): read as a static magnitude the result is only the coincidence "Λ today ≈ 1/horizon² today"; read as a *dynamical* law with `L_H` growing it is holographic dark energy, whose Hubble-horizon version is known to give **no** acceleration (`w ≈ 0`, the Hsu 2004 problem) and yields `w ≈ −1` only under a different (future-event-horizon) cutoff. So the dynamics / equation of state is itself `[OPEN]`, not a prediction.

**Cross-reference (FTD-0344, `[CONJECTURE]`).** `FOUND_MODULUS_ARGUMENT_FRONTIER.md` §7.1 records that the *same* renunciation driving §1's dissolution here — no imported continuum, no zero-point, no chosen adjoint — is also exactly what forbids the argument-half of that document's modulus/argument frontier: `Λ = 0`'s dissolution and δ's unreachability (MC-T4.3) share one structural root, not two independent facts. This does **not** change any tag in this document; §3's source stays `[OPEN]` and §4's value stays `[BOUNDARY]`.

---

## 5 · Epistemic status and what would upgrade it

| Claim | Tag |
|---|---|
| Classical void is zero-energy; FTD has no `M_Planck⁴` catastrophe | `[DERIVED]` (FC-1 + Lagrangian) |
| The catastrophe is dissolved-by-construction; existing `½ℏω` route is FC-1-inconsistent | `[SELECTION]` / reconciliation |
| Λ must be a scale-ratio `Λ·ℓ_P² = f(ℓ_P/L)`, not a fundamental constant | `[DERIVED-from-FC-3]` |
| The holographic *exponent / ceiling* `ρ_vac ≲ M_P²/L²` (`Λ ≲ (ℓ_P/L)²`) | `[SELECTION]` (area-law motivated; imported as CKN) — a *ceiling*, not a source |
| The nonzero *source* that would saturate the ceiling | `[OPEN]` — FTD's classical vacuum predicts `Λ = 0`; no native source (zero-point forbidden by §1; condensate leaks `L⁻⁵`, FTD-0273) |
| The dynamics / equation of state `w` | `[OPEN]` (static reading = coincidence; dynamical reading = holographic dark energy, Hubble-cutoff fails, Hsu 2004) |
| The numerical value `Ω_Λ≈0.68`, `~10⁻¹²²` | `[BOUNDARY]` (needs `L_H`; FTD-0059) |

**The decisive open check (gap #1):** whether the substrate has a vacuum-energy density that is *intensive* and obeys the area law rather than leaking (`L⁻⁵`, FTD-0273). If it does, FTD gains a native nonzero `Λ ~ (ℓ_P/L)²` and the source gap closes; if the leak / a volume-law wins, the route closes negative — FTD has no dark-energy source and the dissolution `Λ = 0` is the final word. Either way the *value* stays a `[BOUNDARY]` (FTD-0059), and the current evidence (the `L⁻⁵` leak) **disfavours** the area-law source.

**But the engine L-scan route to this check is itself boundary-limited (FTD-0364, `ANALYSIS_LAMBDA_LSCAN_FEASIBILITY_v1.md`).** An `ρ_vac(L)` engine scan was scoped and found not cleanly decisive: measuring the *native* vacuum is circular (`ρ=0` by the §1 `[DERIVED]`), while a *sourced* condensate measures fixed-energy dilution (`ρ ∝ L⁻³`) by conservation arithmetic, not a vacuum source; the whole-lattice observable is ill-posed under the determinism-required `langevin`-OFF lossless box (no steady state — the exponent is measurement-window-dependent); and a clean `L⁻²` would have to survive a Green's-function deflation check (the Phase-F/Phase-G precedent, where a lattice L-scan "plateau" was pure periodic-lattice geometry). So the source gap stays `[OPEN]` and is now annotated with *why* it resists engine closure; the wrong-tool `campaign_vacuum_energy.cpp` (an FC-1-declined `½ℏω` k-integral with no lattice and no `L`) is marked superseded.

**Zero promotions.** `x₊=1/α` stays `[SMC]` (FTD-0013); MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`; no α is derived; the value of Λ is **not** derived. The deliverable is honest and mixed: a clean **dissolution** of the old catastrophe (`Λ = 0`, `[DERIVED]` given FC-1), a **form** constraint (FC-3) and a **ceiling** (holographic) for any nonzero Λ, an `[OPEN]` **source** gap (FTD currently predicts zero), and a `[BOUNDARY]` on the value — a Number-One-Goal second-clause result that replaces the rationale of the `α⁵⁷`/`α¹⁶` numerology without claiming the value.

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
