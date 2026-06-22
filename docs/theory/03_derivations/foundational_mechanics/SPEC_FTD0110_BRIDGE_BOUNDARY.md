# SPEC — The FTD-0110 nonlinear-bridge boundary (canonical status map)

**Tag:** `[SYNTHESIS / BOUNDARY HARDENED]` — consolidation of established results; introduces NO new claim and promotes nothing.
**Date:** 2026-06-22
**Scope:** the single canonical statement of where the FTD-0110 cluster-mass bridge stands. It re-states FTD-0269/0276/0307/0309 at their existing tags; if any prose elsewhere disagrees, **LEDGER > this doc > other prose**.
**Read-with:** `SPEC_OPEN_MATH_BY_SECTOR.md` §9 (engine–algebra bridge), `TRACKER_OPEN_ITEMS.md` (FTD-0110 entries), and the three run-of-record analyses cited below.

---

## 0 · The claim and what is settled

**FTD-0110 (the cluster-mass bridge):** the mass of an emergent cluster equals its voxel count `N`, and the cluster-size law `N(A)` (count vs injection amplitude `A`) is what the SM-particle mass identification rests on.

- **Linear level — `[DERIVED]` (untouched mathematics).** The efficiency coefficient `k = 1/N_base = ¼` is derived from the O_h representation theory of the 27-block: the character-table formula gives `mult(A_{1g}) = 4`; `δ_center` is A_{1g}-pure; the 4×4 Laplacian projection gives mean energy `¼` across the A_{1g} eigenmodes. Direction-invariant. Source: `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`, FTD-0110. **This boundary doc does not touch it.**
- **Nonlinear level — `[OPEN]`, boundary now HARDENED on three independent axes (below).** Whether the full nonlinear engine (genesis + coupling + Gauss projection + Langevin) reproduces `N(A)` from framework-only inputs is the open bridge. The SM-particle mass identification (cluster size `N` = particle mass) stays **`[STRONGLY MOTIVATED CONJECTURE]`**.

**The settled decomposition (what the law IS).** The current-stack `N(A)` is a broken power law (FTD-0261): knee `A ≈ 16`, sub-knee exponent `p_lo ≈ 3.7`, super-knee exponent `p_hi ≈ 1.9`, `k_eff ≈ 0.05`. Its structure decomposes into:
- **super-knee = the energy-budget/equipartition regime** — `N ≈ capture·½(A·K_GEN)²/(drain·½K_GEN²)` ⇒ **exponent `p_hi = 2` DERIVES** (this is the linear k=¼ theorem's regime; FTD-0309 adjudicator measures 2.000);
- **sub-knee = the 27-block threshold-filling** — Moore shells cross `K_GENESIS` sequentially as `J_s ≈ c_s·A` (SC at `A≈9`, then BCC/FCC/SC2): a **DERIVED mechanism**;
- **the calibration COEFFICIENT** (`capture ≈ 0.024–0.04`, the exact knee) is the engine-emergent nonlinear-suppression factor (the linear theory would give `drain/4 = 0.125`; the measured value is the FTD-0267 genesis throttling).

So the **shape structure is derivable; the absolute calibration is engine-emergent.** The three boundary axes below establish that this split is irreducible.

---

## 1 · The three boundary axes (each closed-negative)

| Axis | Question | Verdict | Run of record |
|---|---|---|---|
| **Exit-i — derive the calibration** | Can the calibration constants (`drain=0.5`, `γ=0.02`) be derived from the action / simplest framework sources? | **`[CLOSED NEGATIVE]`** (FTD-0276) | `10_eft_program/preregistrations/PREREG_DRAIN_SCALING_v1.md` + `DERIV_KINETIC_DRAIN_FROM_QUADRATURE.md` |
| **Exit-ii — is it convention?** | Is the calibration pure CONVENTION (only the dimensionless shape physical; removable by an affine `(A,N)` rescaling)? | **`[CLOSED NEGATIVE]` — PHYSICAL** (FTD-0307) | `ANALYSIS_FTD0110_CONVENTION_AUDIT_v1.md` |
| **The reduction axis** | Does a faithful low-dimensional (scalar collective-coordinate) reduction reproduce the law? | **`[CLOSED NEGATIVE]` — obstructed** (FTD-0309) | `ANALYSIS_GENESIS_COUNTING_V2.md` |

### 1.1 Exit-i — the calibration is not derivable by the simplest routes (FTD-0276)
The kinetic-drain origin is **not** a squared structural quantity: the sub-knee `k_eff ∝ drain^{−0.93}` (≈ 1/drain), so the drain is a **linear calibration prefactor**, not a derivable `¼`-type structural constant; the `drain²` origin is closed-negative. The Langevin friction `γ` **calibrates** the super-knee (γ=0 over-predicts ×1.53; the engine's γ=0.02 lands the FTD-0261 knee 16 / `p_hi` 1.81) but is itself imposed. No framework source for either constant was found. *Quadrature equipartition (½) cannot give a drain-DEPENDENT `k_eff`.*

### 1.2 Exit-ii — the calibration is PHYSICAL, not convention (FTD-0307)
**Method correction first:** a broken power law's exponents are INVARIANT under any affine `(A,N)` rescaling, so the FTD-0269 knee-shift criterion does NOT distinguish convention from physics — **exponent-invariance** is the right discriminator. Run of record (`campaign_drain_scan`, L=32, 8 seeds, bit-identical parallel): the clean super-knee single-power exponent **DECREASES monotonically `1.91→1.59`** across drain (~6σ) — a pure rescaling would hold it exactly constant. Both knobs (drain + γ) are PHYSICAL ⇒ the calibration is **irreducibly engine-emergent**, neither derivable (exit-i) nor removable as convention (exit-ii).

### 1.3 The reduction axis — no scalar collective-coordinate reduction (FTD-0309)
A faithful **scalar (O_h-radial)** collective-coordinate reduction is structurally obstructed: it reproduces the derivable structure (super-knee `p_hi=2.000`, knee 15.75, A=10 count 4.33) but fails the A=14 Moore-shell **geometry** in both boost modes (L1 = 0.42 monopole runaway / 1.16 local under-fill, vs gate 0.30). The intermediate-shell (FCC/SC2) filling is carried by the **irreducibly-angular dipole Gauss field** (the fired state-field is an x-dipole, net charge ≈ 0); a radial coordinate cannot represent a dipole. The minimal faithful carrier is the angular-resolved field — i.e. the FTD-0269 forward model itself. **This sharpens FTD-0250** (the cluster collective-coordinate reduction `[OPEN]`): no scalar reduction exists; an angular DOF is mandatory.

---

## 2 · Net boundary statement

The FTD-0110 nonlinear bridge is **`[OPEN]` with its boundary HARDENED**:
- the cluster-mass law's **shape structure is DERIVED** given the imposed register (super-knee energy-budget exponent `p_hi=2`; sub-knee 27-block-filling mechanism);
- the **calibration is irreducibly engine-emergent** — not derivable by the simplest routes (exit-i) and not removable as convention (exit-ii);
- **no scalar collective-coordinate reduction** reproduces the law (the reduction axis); the angular dipole structure is load-bearing.

This is a Number-One-Goal **boundary result**: the discrete substrate + O_h Laplacian + Gauss Green's function fix the geometric shape; the absolute calibration is set by non-framework constants, and the law has no low-dimensional radial reduction.

**The one registered open next-step (OUT of scope for the consolidation):** an **angular-resolved** (2-stream on-axis/off-axis, or C4v-reduced) collective-coordinate reduction. Prior-favoured outcome: PARTIAL_BOUNDARY (the calibration stays engine-emergent per §1.2). Pursuing it would either land the shape as `[CONDITIONAL — DERIVED-GIVEN-IMPOSED]` or close the reduction axis fully.

---

## 3 · Provenance and non-promotion

| FTD-id | Role | Tag |
|---|---|---|
| FTD-0110 | linear k=¼ O_h theorem; the bridge it anchors | `[DERIVED]` (linear) / `[SMC]` (identification) |
| FTD-0261 | the current-stack `N(A)` target law | `[MEASURED]` |
| FTD-0267 | one-shot genesis throttling = the emergent `capture` | `[MEASURED]` |
| FTD-0269 | the forward model = framework dynamics; the original boundary map | `[MEASURED — BOUNDARY]` |
| FTD-0276 | exit-i (derive the calibration) closed-negative | `[CLOSED NEGATIVE]` |
| FTD-0307 | exit-ii (convention audit) closed-negative; calibration PHYSICAL | `[MEASURED — BOUNDARY]` |
| FTD-0309 | the reduction axis closed-negative; scalar reduction obstructed | `[MEASURED — BOUNDARY]` |
| FTD-0250 | the cluster collective-coordinate reduction, sharpened by FTD-0309 | `[OPEN]` |

**Nothing promoted.** FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`, the SM cluster-mass identification `[SMC]`, the linear k=¼ O_h theorem — all unchanged. No α derived anywhere; golden gate untouched (documentation-only consolidation).
