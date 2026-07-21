# Rest Mass as a Topological Invariant of the Manifested State-Field Pattern

**Status:** [TERMINAL UNDERDETERMINATION — the octahedral-shell hedgehog charge is robustly ZERO at freeze, not the nonzero ±1 needed to anchor a mass floor. FTD-0398 tracked the unchanged convention over R=1..6 and t=0..8 but could distinguish neither persistent transport nor a definition-boundary destruction event. The campaign supplies no mass evidence and licenses no further shell geometry.]
**Supersedes nothing; runs alongside** `DERIV_REST_MASS_FROM_CONSTRAINT_ENERGY.md` (CLOSED — the constraint-locked-*energy* route). This document proposes a structurally different kind of quantity for the same question, motivated directly by why that route closed.

---

## 1 · Why an energy functional cannot be the answer, and what can

`DERIV_REST_MASS_FROM_CONSTRAINT_ENERGY.md` tested five distinct energy-based candidates for a substrate-native rest-mass scale (the Gauss-constrained self-energy W_SC, a kinetic-drain mechanism, single-site curl-response, forced-delay timing, and independently-constructed birth seeds). All five measured the *energy* content of a manifestation event under some protocol. All five were found circumstance-dependent: the locked energy varies by up to 9.2× depending on the birth amplitude alone (seed-diversity campaign, CV = 0.878).

This is not five unlucky guesses at the wrong energy functional. It is a structural property of energy: an energy is a *continuous, deformable* functional of a field configuration, and generically depends on how that configuration arose. Nothing in FTD's dynamics (wave propagation, Gauss projection, genesis) forces two differently-born charges to arrive at the same field energy, so no energy-based candidate should have been expected to survive circumstance variation. A seventh energy-based attempt would predictably fail the same way.

**What would not fail this way is a topological invariant** — an integer classifying a field configuration up to continuous deformation, by construction insensitive to exactly the kind of birth-circumstance variation (amplitude, position, timing) that broke every energy-based candidate. This is the standard move in soliton/kink physics: a sine-Gordon kink's topological charge (winding number between vacua) is exactly conserved under arbitrary continuous deformation of the field, even while its energy is not fixed in the same trivial sense — the topological sector fixes a *minimum* possible energy (a Bogomolny-type bound), not the energy of any one realization.

## 2 · The candidate: hedgehog charge of the flux field's direction map

FTD's flux field J is a continuous ℝ³-valued field on the lattice — the natural place to look for a genuine topological invariant, exactly as in continuum vector-field theories (magnetic monopoles, Skyrmions, hedgehog defects in Heisenberg magnets and liquid crystals).

**Construction.** Around any lattice site, normalize Ĵ = J/|J| wherever J ≠ 0. On a small closed surface enclosing the site, Ĵ defines a map to the unit sphere S². The **degree of this map** — how many times it wraps S² — is an integer, invariant under any continuous deformation of J that never lets |J| vanish on the surface. This is computed by the standard **Berg–Lüscher discretization** (Berg & Lüscher, *Nucl. Phys.* B190 (1981) 412): for a triangulated closed surface with unit vectors (n_i, n_j, n_k) at each triangle's vertices,

```
tan(Ω_ijk / 2) = [n_i · (n_j × n_k)] / [1 + n_i·n_j + n_j·n_k + n_k·n_i]
Q = (1 / 4π) · Σ Ω_ijk   over all triangles of the closed surface
```

**The enclosing surface is FTD's own octahedral Moore-shell** (the k=1 layer of the Moore Layer Theorem: the 6 face-neighbors at distance 1, decomposing naturally into 8 triangular faces). This is not a surface chosen to produce a favorable answer — it is the innermost closed shell FTD's own established polyhedral decomposition already provides.

**Formula validated before any engine use** (`scripts/exploration/validate_hedgehog_charge.py` pattern, run inline this session): the octahedral Berg–Lüscher formula correctly returns Q=+1 for a pure radial (hedgehog) field, Q=−1 for anti-radial, Q=0 for a uniform or near-uniform field, and is confirmed rotation- and magnitude-invariant (scaling each vertex's field by an arbitrary positive factor leaves Q unchanged) — 6/6 hand-constructed test cases with known answers passed.

## 3 · What distinguishes this from electric charge (and why it isn't vacuous)

Every seed in the closed energy-based arc injects flux via a purely radial pulse (`amp·dx/r, amp·dy/r, amp·dz/r`), so the enclosed *electric* charge (∮J·dA, a linear functional of J) is trivially +1 by construction in every case — that is not new information. The hedgehog charge Q is a genuinely different, **nonlinear, purely angular** functional: it depends only on the *direction* of J at each point of the shell, not on magnitude, and is provably insensitive to the same magnitude/energy variation that made the closed arc's candidates circumstance-dependent (§2's magnitude-distortion test case). A field can have any nonzero electric charge with Q=0 (direction doesn't wind), or vice versa in principle. They coincide for the canonical radial configuration but are not the same invariant, and only direct measurement can say whether Q stays pinned once real dynamics (not just the initial injection) has acted on the field.

## 4 · The falsifiable question

**Does Q, measured on the octahedral shell around each manifested voxel at the moment of freeze, stay pinned at the same value across the three valid seed-diversity seeds (A_baseline, C_hot, E_cold — e_half 1.71 / 7.22 / 0.78, a 9.2× spread), despite that spread — and does it match the value on the idealized synthetic (W_SC) charge?**

- **If Q is pinned across all seeds and matches the synthetic reference:** genuine support for "topological charge, not energy, is the substrate-invariant quantity" — the excess energy over W_SC would then be reframed as the *cost* of supporting a fixed topological charge, not itself the invariant, and would open the question of a Bogomolny-type lower-bound relation between Q and energy as the next target.
- **If Q varies with circumstance the same way energy did, or is ill-defined (field vanishes on the shell) for some seeds:** the topological-invariant route closes negative at its first test, on the same footing as the five energy-based closures, and should not be re-attempted with this specific construction without a new, independently-motivated surface or field choice.

See `docs/theory/10_eft_program/preregistrations/PREREG_HEDGEHOG_CHARGE_ROBUSTNESS_v1.md` for the locked falsifier, frozen reading bands, and full outcome.

**Freeze outcome (FTD-0392, 2026-07-20):** ROBUST, per the frozen bands as literally written — Q_A = Q_C = Q_E = 0.0000000000 (spread ≈1e-10) across the three seeds despite their 9.2× energy spread. But the pinned value is the *trivial* sector (Q=0), not the ±1 hedgehog value both the initial radial injection and the idealized synthetic-charge reference (Q_S=+1, exact) display. Large RMS angular deviation (135–153°) confirms the field's direction genuinely changed by freeze time. A trivial topological sector carries no Bogomolny-type energy floor above zero in the standard construction, so it cannot anchor a nonzero rest mass the way this document hoped.

**Terminal transport outcome (FTD-0398, 2026-07-20): UNDERDETERMINED.** The unchanged octahedral convention was evaluated on scaled shells R=1..6 for post-injection ticks 0..8. Unit charges appeared transiently on several radii and with both signs, but no fixed inner shell remained charged, outward motion was not non-returning, and no previously charged shell crossed through the registered `|J|=0` definition boundary followed only by trivial enclosing shells. The data decide neither transport nor destruction. Under the terminal lock this supplies no mass evidence and ends shell redesign for this route. See `ANALYSIS_TOPOLOGICAL_CHARGE_TRANSPORT_v1.md`.

## 5 · Claim ledger

| Claim | Tag | Note |
|---|---|---|
| Berg–Lüscher discretization on a closed triangulated surface computes an integer topological degree | [THEOREM — imported, Berg & Lüscher 1981] | Standard result; not an FTD-native derivation, an imported mathematical tool, same status as the Watson/Ewald machinery used for W_SC |
| FTD's octahedral Moore-shell provides a natural, non-arbitrary closed surface for this construction | [SELECTION — motivated by the pre-existing Moore Layer Theorem, not chosen post-hoc] | The k=1 layer is FTD's own innermost established shell |
| Q is well-defined and behaves correctly (rotation/magnitude invariant, correct sign) on hand-constructed test fields | [VERIFIED] | 6/6 synthetic checks, this session |
| Q is robust across circumstance-dependent genesis-born configurations, where energy is not | [CONFIRMED — ROBUST, 2026-07-20] | Q pinned at 0.0000000000 (spread ≈1e-10) across a 9.2× e_half spread; the robustness claim holds, just not at the hoped-for nonzero value |
| The pinned value is nonzero (±1), matching injection and the synthetic reference | [REFUTED — 2026-07-20] | Q_A=Q_C=Q_E=0 exactly; Q_S=+1. Real dynamics drives the shell charge to the trivial sector within 2 ticks, independent of amplitude |
| M_REST ≡ (some function of Q) on this construction | [CLOSED NEGATIVE] | A trivial (Q=0) topological sector carries no energy floor above zero in the standard construction; this specific shell/timing cannot anchor a nonzero rest mass |
| Charge migrates outward rather than being destroyed | [UNDERDETERMINED — FTD-0398] | Scaled octahedral shells R=1..6 show transient integer charges but fail both the non-returning transport predicate and the registered destruction predicate |
| Another shell geometry should be tested | [CLOSED BY TERMINAL PROTOCOL] | FTD-0398 was the final registered geometry campaign; its UNDERDETERMINED outcome licenses no redesign and supplies no mass evidence |

---

*Registered 2026-07-20. Author: session 8294fddb. Parent: `DERIV_REST_MASS_FROM_CONSTRAINT_ENERGY.md` (CLOSED, energy-route). Companion: `PREREG_HEDGEHOG_CHARGE_ROBUSTNESS_v1.md`.*
