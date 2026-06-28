# SPEC — The cosmology-sector boundary (canonical verdict)

**Tag:** `[SYNTHESIS / BOUNDARY]` — consolidation of established status; introduces NO new claim and promotes nothing.
**Scope:** the single canonical statement of what FTD's cosmology sector (§8) is and is not. It consolidates the already-honest tags in `SPEC_OPEN_MATH_BY_SECTOR.md` §8 + `evaluation/AUDIT_WEAKNESSES_MASTER.md` (W-COSMO 1–7) into one citable boundary. Precedence: **LEDGER > this doc > other prose.**

---

## 0 · The verdict

**Cosmology is FTD's most-imported, least-substrate-derived sector.** Every §8 entry is **standard ΛCDM apparatus filled with FTD constants** (numerology), or an **ad-hoc identification** ([SELECTION]), or **[OPEN]**. There is **no substrate derivation of a cosmological observable** — no inflaton dynamics, no power spectrum / BAO, no halo profile, no Λ mechanism, no first-principles expansion history. The canonical theory docs already carry these honest tags; this doc makes the boundary first-class so that **no prose (especially the manuscript / dissemination layer) reads "FTD derives cosmology."**

This is a Number-One-Goal **boundary**: the discrete substrate, as currently developed, does not reach cosmological-scale physics. The honest framing for any cosmology claim is **"ΛCDM apparatus + FTD numerology,"** never "derived."

---

## 1 · The honest decomposition (the six W-COSMO weaknesses)

| Item | Claim | Honest tag | Note |
|---|---|---|---|
| **W-COSMO-1** | Inflaton ≡ mean flux | `[SELECTION]` / ad hoc | no inflaton dynamics derived |
| **W-COSMO-2** | Dark-matter mechanism | `[OPEN]` (internally inconsistent) | see §2 — the dark-state *count* is structural; the *dynamics* are not |
| **W-COSMO-3** | First-order electroweak transition | `[CONJECTURE]` (assumed, not derived) | the one forward prediction (FP-1) rides on it; see §3 |
| **W-COSMO-4** | `Λ = α⁵⁷` | `[PARAMETRIC]` numerology | the `α⁵⁷`/`α¹⁶` value-match stays `[PARAMETRIC]`. **FC-1 dissolves the *old* catastrophe** (classical, no `½ℏω` → `Λ = 0`); FC-3 fixes the *form* (scale-ratio) and the holographic bound the *ceiling* (`Λ ≲ (ℓ_P/L_H)²`). **But the nonzero *source* is `[OPEN]` — FTD predicts `Λ = 0`** (the per-mode energy the ceiling needs is the declined zero-point; the condensate leaks `L⁻⁵`), and the value a `[BOUNDARY]` (FTD-0059). See FTD-0331 / `DERIV_LAMBDA_SCALE_COVARIANT.md` |
| **W-COSMO-4b** | Equation of state `w` | `[OPEN]` (per FTD-0331) | `w` is NOT an FTD prediction. The static "`w=−1` exactly / no time variation / DESI-Euclid sees `w=−1` at all `z`" reading in `DERIV_COSMOLOGICAL_CONSTANT` §6.1/§6.2 + CC-8/CC-9 is RETIRED. Static reading = coincidence; dynamical = holographic dark energy whose Hubble-cutoff version fails (`w≈0`, Hsu 2004). A measured `w≠−1` does NOT falsify FTD. |
| **W-COSMO-5** | Power spectrum + BAO | `[OPEN]` (missing) | no FTD prediction exists |
| **W-COSMO-6** | NFW halo profile | `[OPEN]` (not derived) | reinforced by FTD-0300 (§2) |

The ΛCDM expansion history (Hubble, Friedmann), structure formation, and recombination are **adopted external physics**, not FTD outputs.

## 2 · Dark matter — the one structural residue, and its limit

The Moore Layer Theorem gives **17 dark states** out of 27 (the polyhedral decomposition: octahedron + cuboctahedron + stella octangula), so a dark fraction `17/27 ≈ 0.63` is a **structural integer count** — the *most* defensible cosmology-adjacent number. **But the dark-matter DYNAMICS are not derived:** the halo profile / SPARC rotation curves were attacked directly and returned **`INDETERMINATE`** — **FTD-0300** (halo forcedness) found SPARC **NOT founded**: the lossless dark-matter halo **box-fills the periodic lattice** (`r_eff ≈ L/2`, not localized), and the §4.1 −0.69 halo exponent was **FALSIFIED** (an L=64 transient → −1.25 at L≥128). So: the dark-state *count* is structural; the dark-matter *object/dynamics* is an `[OPEN]`/mapped-negative boundary.

## 2.1 · The Ω_Λ collision (one canonical position)

FTD has **NO derived Ω_Λ.** Three uncoordinated numbers sit near observed `0.685`, mutually inconsistent, none a derivation:

- **`0.683`** — the `α¹⁶` value-match (`DERIV_COSMOLOGICAL_CONSTANT` CC-7 / `DERIV_DARK_SECTOR` DSD-10 / `SPEC_QUADRATIC_PHYSICS_BRIDGE` §15.2). `[PARAMETRIC]` (FTD-0331, no L-dependence).
- **`2/3 = 0.667`** — `OMEGA_LAMBDA_CONJ` dual-substrate (engine constant). `[CONJECTURE]` round-number.
- **`17/27 ≈ 0.63`** — the Moore dark-**STATE** count (Moore Layer Theorem §7, `[THEOREM]`). A Hilbert-space STATE count, category-distinct from an energy-density Ω_Λ; it must **NOT** be cited as a dark-energy fraction.

Per FTD-0331 the Ω_Λ **VALUE** is a `[BOUNDARY]` (needs `L_H`; FTD-0059 no native length). The proximity to `0.685` is coincidental, not a coordinated FTD output.

## 3 · The one forward prediction (FP-1), at its honest tag

The first-order electroweak phase transition (from `K_GENESIS = 3·K_B`, a genuine FTD hysteresis — FTD-0272) ⇒ a relic stochastic GW background in the LISA band. **Existence is `[CONJECTURE]`** (the cosmological identification is not derived); **the spectrum is `[PARAMETRIC]`** (rides on (α_GW, β/H)). Registered in `SPEC_PREDICTIONS_FORWARD_2026.md` (FP-1) with its kill condition (LISA-class reach with no EWPT-compatible background). This is the sector's only falsifiable forward datum, and it is honestly conjectural.

## 3.1 · The one internally-producible cosmology datum (OPEN, pre-register before running)

Distinct from FP-1 (the EWPT/LISA forward GW prediction), FTD can produce **one** decisive internal cosmology datum: a **pre-registered GPU L-scan** over `L ∈ {64, 96, 128, 160, 256}` (WSL2/RTX 5090, golden-neutral, observation-only counters) measuring steady-state manifested-condensate **vacuum-energy density `ρ_vac` vs L**. Three outcomes:

- **(A) AREA-LAW SOURCE** — `ρ_vac·L² → const` (exponent `−2`): FTD gains a native nonzero `Λ ~ (ℓ_P/L)²`, the `[OPEN]` source gap closes **POSITIVE** (value still a `[BOUNDARY]` needing `L_H`).
- **(B) LEAK** — `ρ_vac ∝ L⁻⁵` persists (the current FTD-0273 evidence, **prior-favoured**): no native dark-energy source, the dissolution `Λ = 0` is the final word, route closes **NEGATIVE**.
- **(C) VOLUME-LAW / other exponent** — likewise no area-law source ⇒ closes negative on the source gap.

Prior-favoured outcome: **B** (FTD-0273's `L⁻⁵` leak disfavours the area-law source). Pre-register (e.g. tag `preregister-vacuum-energy-arealaw-v1`, SHA256) **before** measurement.

## 4 · Scope of the credibility fix

- **Canonical theory layer (already honest):** `SPEC_OPEN_MATH_BY_SECTOR.md` §8 + `AUDIT_WEAKNESSES_MASTER.md` (W-COSMO) already tag the sector `[SELECTION]`/`[PARAMETRIC]`/`[OPEN]`. **No retag needed in the canonical theory docs.**
- **Dissemination / manuscript layer (the actual risk):** any chapter or outreach asset that frames cosmology as *derived* must be corrected to the "ΛCDM apparatus + FTD numerology" reading, governed by `dissemination/manuscript_v2/PROPAGATION_RULE.md`. This is the remaining propagation check (a dissemination-layer task, not a theory-doc task).

## 5 · Non-promotion

**Nothing promoted.** All §8 entries keep their existing `[SELECTION]`/`[PARAMETRIC]`/`[OPEN]` tags. The dark-state count `17/27` is structural (Moore Layer Theorem) but the dark-matter dynamics are `[OPEN]`/mapped-negative (FTD-0300). No cosmological observable is derived; FTD-0013 `[SMC]`, MC-T4.3, the spine — all unchanged. Companion accepted-boundary docs: [`SPEC_ALPHA_DYNAMICAL_BOUNDARY.md`](SPEC_ALPHA_DYNAMICAL_BOUNDARY.md), [`SPEC_FTD0110_BRIDGE_BOUNDARY.md`](../03_derivations/foundational_mechanics/SPEC_FTD0110_BRIDGE_BOUNDARY.md).
