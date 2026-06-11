# Mechanism δ — Attempt to Fix `a_phys` from Information-Density / CFL Primitives

> **[CLOSED NEGATIVE 2026-04-23]** — Mechanism δ does not deliver a first-principles `a_phys`. Every candidate route either stays dimensionless (no length produced) or requires introducing an external dimensional input (at which point the result is a calibration, not a derivation). This closure is structural, not computational, and motivates the no-go theorem in `THEOREM_A_PHYS_NO_GO.md`.

**Tag:** [CLOSED NEGATIVE]
**Status:** Closed. Authoritative disposition: `a_phys ≡ ℓ_P` as CALIBRATION (`docs/SPEC_FTD.md`, LEDGER FTD-0030 / FTD-0041).
**Predecessors:** Mechanism α (algebraic invariants, closed), Mechanism β (EFT matching, not a derivation by construction), Mechanism γ (gravitational chain, closed negative — `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md`; later "SUCCESS" doc retracted 2026-04-23).

---

## 1 · Calibration Hygiene Rules (binding for this document)

Prior failed attempts all shared one failure mode: a dimensional constant (ℏ, c_phys, G_N, m_e, ℓ_P) was silently introduced mid-derivation. This document enforces the following rules:

1. **Input declaration.** Every quantity used as a starting ingredient is listed in §2 with its SI units and its source (Axiom-Zero invariant, pure mathematical constant, or external dimensional input).
2. **No silent dimensional insertion.** No step may introduce a dimensional constant that is not in the §2 list. If any step writes a quantity whose SI units cannot be derived from units of §2 inputs, that step is flagged and the attempt terminates.
3. **Unit trace at output.** Each SI-unit factor in the final expression must trace back to a specific §2 input that carried that unit. Meters cannot be manufactured from dimensionless inputs.
4. **Axiom-Zero inputs are dimensionless.** `{D=3, ternary state, 26-Moore, determinism, discrete time}` plus derivable dimensionless constants `{N_base=4, N_eff=13, b_3=7, N_c=3, G*, ϖ, π, x_+ = 137.036…, x_− = 3.024…, c_lat = 1/√3, …}` — none of these carry SI units.
5. **Flag on dimensional input.** If a step needs ℏ, c_phys, G_N, m_e, k_B, ℓ_P, or any measured physical quantity to proceed, the attempt records "dimensional input introduced at step N" and treats the resulting chain as calibration, not derivation.

---

## 2 · Declared inputs

### 2.1 · Axiom-Zero invariants (all dimensionless)

| Name | Value | Source | Units |
|---|---|---|---|
| D | 3 | Postulate 1 | — |
| ternary alphabet size | 3 (states ∈ {−1, 0, +1}) | Postulate 3 | — |
| Moore degree | 26 | from D=3 + 3×3×3 neighborhood minus self | — |
| `c_lat` | 1/√3 | CFL bound on 3D cubic stencil | voxels / tick (ratio of Axiom-Zero units) |
| `G*` | √2 · Γ(1/4)² / (2π) ≈ 2.9587 | lemniscatic identity (`FTD-0002`) | — |
| `ϖ` | lemniscate constant ≈ 2.6221 | from Γ(1/4) | — |
| `π` | circle constant | pure math | — |
| `N_base, N_eff, b_3, N_c` | 4, 13, 7, 3 | Moore polyhedral decomposition (`FTD-0008`) | — |
| `x_+, x_−` | 137.036…, 3.024… | roots of master quadratic (`FTD-0001`) | — |
| information per voxel | log₂(3) bits | Postulate 3 | bits |
| tick interval | 1 tick | Postulate 2 | tick |
| voxel spacing | 1 voxel | Postulate 1 | voxel |

**Crucial observation.** "voxel" and "tick" are Axiom-Zero unit-labels, not SI units. Nothing in Axiom Zero specifies how many meters a voxel is, or how many seconds a tick is. They are abstract lattice coordinates.

### 2.2 · External dimensional constants (NOT in the Axiom-Zero basis)

These are listed only to be explicitly excluded from §3–§6. Any attempted route that requires them is flagged per Rule 5.

| Name | SI value | SI units |
|---|---|---|
| `c_phys` | 2.998 × 10⁸ | m · s⁻¹ |
| `ℏ` | 1.055 × 10⁻³⁴ | J · s = kg · m² · s⁻¹ |
| `G_N` | 6.674 × 10⁻¹¹ | m³ · kg⁻¹ · s⁻² |
| `m_e` | 9.109 × 10⁻³¹ | kg |
| `ℓ_P` | 1.616 × 10⁻³⁵ | m |

---

## 3 · Route δ-1 — Information-density velocity

**Attempt.** Postulate 3 fixes information content per voxel at log₂(3) bits. Postulate 4 (26-Moore) plus Postulate 2 (discrete time) fix a maximum information-propagation rate of at most 26 outgoing bit-influences per voxel per tick (or tighter per CFL). Define an "information velocity":

v_info := (log₂(3) bits × c_lat voxels/tick) / (1 tick) = log₂(3) · (1/√3) bits · voxel · tick⁻².

**Unit trace.** Output units are `bits · voxel · tick⁻²`. These are not length per time in SI. To convert "voxel" to meters or "tick" to seconds, a dimensional input from §2.2 is required.

**Dimensional input introduced at step (§3 final):** none yet, but the output is not a length. To coerce it to a length we would have to introduce `c_phys` (m/s) or `ℓ_P` (m) as a separate calibration.

**Verdict.** Produces a dimensionless/Axiom-Zero-unit velocity. Does not produce a length. **Route dies at the unit-trace check.**

---

## 4 · Route δ-2 — CFL + discrete time, combined with lattice invariants

**Attempt.** `c_lat = 1/√3` is a pure number. `G*`, `π`, `x_+`, `x_−` are pure numbers. Any arithmetic combination `f(G*, π, x_+, x_−, c_lat, N_eff, …)` is pure number. Multiplying by the unit-label "voxel" gives a quantity of dimension `voxel`, not meters.

**Formal claim.** The ring generated by §2.1 under +, ×, exp, log, Γ, and Watson integrals is `ℝ` (or a sub-ring thereof). Every element is a dimensionless real. A length has SI dimension `L`; no element of ℝ has SI dimension `L` unless a dimensional generator is adjoined.

**Dimensional input introduced:** would be required (specifically a length `ℓ` with SI units `m`) to produce `a_phys` in meters. None available in §2.1.

**Verdict.** Structural dead end. This is the same wall Mechanism α hit. **Route closed.**

---

## 5 · Route δ-3 — Candidates in the engine's `ontic.h` chain

Searching `engine/include/ftd/ontic/*.h` for any quantity that carries SI units natively (as opposed to being flagged "lattice mass-units" or "lattice gravitational-coupling units"):

- **Layer −1 to Layer 4** (transcendental seeds → framework integers): all dimensionless (e, γ, Γ(1/4), θ₃, ϖ, M, G*, π, x_+, x_−, N_c, b_3, N_eff, D). No SI units anywhere.
- **Layer 5** couplings (α, g_c, G_N, α_G): all dimensionless. `G_N = 1/(b_3+N_c)² = 0.01` is labelled in `ontic/gauge_couplings.h` as "lattice gravitational-coupling units" — explicitly not SI m³/(kg·s²).
- **Layer 6** mass scale (`K_B`): labelled "lattice mass-units"; set to `0.511`. Carries no SI dimension at declaration; a mass calibration is needed to assign kilograms.
- **Layer 7+**: built from the above; inherit dimensionless status.

**Verdict.** The entire ontic chain is dimensionless. No layer introduces SI units natively. Every mention of SI units in the engine or specs (MeV, kg, m, s) traces to an **external** anchor (`K_B = m_e` for mass, `a_phys ≡ ℓ_P` for length, `c_phys` for time). **Route confirms Mechanism α's verdict at the engine level.**

---

## 6 · Route δ-4 — Two-anchor elimination (an attempted "escape clause")

**Attempt.** Suppose we have two independent dimensional anchors (one for mass, one for something else). Can we derive `a_phys` without introducing length directly?

Mechanism γ tried exactly this with (mass anchor `K_B = m_e`, coupling anchor `G_N(lat) = 0.01`). The result was a ratio `a_phys = 3 · M_unit · G_N(phys) / (G_N(lat) · c_phys²)`. Every SI factor in that output traces back to a §2.2 dimensional input (`m_e` provides kg, `c_phys` provides m/s, `G_N(phys)` provides m³/(kg·s²)). The §2.1 Axiom-Zero factors (`G_N(lat) = 0.01`, the numerical `3`) contribute only dimensionless shaping. **Every meter in the output comes from a meter already present in a §2.2 input.** The "derivation" is an algebraic rearrangement of known dimensional inputs — precisely the failure Mechanism γ documented.

The retracted SUCCESS variant (2026-04-23) does the same thing one level deeper: replaces `K_B = m_e` with `ℏ_lat = 1` (which, when matched to physical `ℏ`, supplies the kg·m²/s unit). The calibration is shuffled, not eliminated.

**General claim.** If the set of dimensional anchors spans SI dimensions {M, L, T} (or any two of the three, with the third fixed via `c_phys`), then `a_phys` is algebraically determined **by the anchors**, not by Axiom Zero. If the set spans fewer dimensions, `a_phys` is not fixed at all. Either way, the Axiom-Zero side is doing no dimensional work.

**Verdict.** No escape. **Route closed.**

---

## 7 · Why the attempt fails, precisely

At the end of every route, the same structural wall appears:

1. Axiom Zero generates only dimensionless quantities (the ring `R` of reals built from `{G*, π, ϖ, x_+, x_−, N_k, c_lat, …}`).
2. Length is a non-trivial SI dimension (`L¹`).
3. No element of `R` has SI dimension `L¹`.
4. Therefore no length is expressible from Axiom Zero alone.
5. Any expression that outputs a length must contain at least one factor of a dimensional input not in `R`. That factor is the calibration.

The failure is at step 3–4. It is not a computational limitation; it is a statement about the dimension of the Axiom-Zero-generated ring.

**Corollary.** The same argument applies to mass, time, energy, temperature — any SI dimension. Axiom Zero cannot set a physical scale. Every scale is a calibration.

This observation is elevated to a theorem in `THEOREM_A_PHYS_NO_GO.md`.

---

## 8 · Disposition

- Mechanism δ joins α, β, γ as [CLOSED NEGATIVE].
- The authoritative calibration `a_phys ≡ ℓ_P` in `docs/SPEC_FTD.md` stands.
- The no-go theorem in the sibling document formalizes why this closure is structural.
- No further derivation attempt of `a_phys` from Axiom-Zero invariants should be undertaken without first identifying a **new dimensional generator** that legitimately belongs to Axiom Zero. None has been identified; none is likely to exist.

---

## 9 · Pointers

```
docs/theory/10_eft_program/THEOREM_A_PHYS_NO_GO.md          # formal no-go theorem
docs/theory/10_eft_program/archive/resolved/OPEN_A_PHYS_DERIVATION.md        # closed open problem
docs/theory/10_eft_program/archive/closed_negative/DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md  # Mechanism γ closure
docs/theory/07_assessment/core_ledgers/LEDGER.md                         # FTD-0030, FTD-0041, FTD-0059
docs/SPEC_FTD.md                                            # LATTICE  PHYSICAL CALIBRATION
```
