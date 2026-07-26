# DERIV — The dispersion boundary formalized: the frequency-branch torsor and the two-gap split of FTD-0270

**Tag:** `[THEOREM]` (branch blindness, ω²-ownership, one-slice underdetermination — elementary, machine-verified, scoped to the linear free flux-wave sector) + `[DERIVED]` (the two-import anatomy of quadratic first-order evolution, same scope) + `[SYNTHESIS]` (the split of the FTD-0270 boundary; the IMP-S2 identification) + `[OPEN]` (native nonlinear gapping — untouched).
**LEDGER id:** FTD-0520 · **Date:** 2026-07-25
**Formalizes:** the one schema instance with no exhibited fiber — the frontier's ATTEMPTED row (FTD-0336 §4, "cavity-not-Schrödinger dispersion", FTD-0270 `[MEASURED — BOUNDARY]`) — as flagged next attack point in [`DERIV_SECTION_SCHEMA_TORSOR_CORRECTION.md`](DERIV_SECTION_SCHEMA_TORSOR_CORRECTION.md) (FTD-0518). Dispersion of record: [`ANALYSIS_LATTICE_WAVE_SECTORS_v1.md`](../03_derivations/foundational_mechanics/ANALYSIS_LATTICE_WAVE_SECTORS_v1.md) — axis modes `sin²(θ/2) = c²·sin²(k/2)`, `c = 1/√3`.
**Verification:** `scripts/proofs/proof_dispersion_branch_torsor.py` (6/6 PASS, 2026-07-25).

---

## 0 · Verdict up front

The dispersion boundary is not one gap but two, and they live on opposite sides of the modulus/argument frontier:

1. **The mass gap is owned-side.** The linear sector's dynamics determines only the even, squared object `sin²(θ/2)` — and determines it to be exactly gapless (`θ(0) = 0`, machine-exact). A gap requires modifying the *owned symmetric invariant* with a restoring term the six native rules do not contain: an explicitly imported `−m²J` produces `θ₀ = 2·asin(m/2)`, simulated to 10⁻⁹. No branch choice can gap a spectrum — sections preserve ω².
2. **The first-order complex form is section-side.** For each spatial mode the real dynamics presents a two-element conjugation orbit `{(k, ω), (−k, −ω)}` — one real solution, two complex labels. The real update is branch-blind (it commutes with conjugation, machine-exact), and the real field is one-slice underdetermined (two futures share a snapshot). Selecting the positive-frequency point of each orbit — the section of this ℤ/2 torsor — is what packs `(J, J̇)` into a single complex slice and yields exact first-order evolution `ψ(t+1) = e^{−iθ}ψ(t)`. That section *is* a complex structure on the solution space — the same carrier the import ledger already prices as **IMP-S2** (the J→ψ complexification, `[SELECTION]`).

The torsor form of FTD-0518 therefore **extends** to the dispersion row rather than breaking there — and for the second time (after I3/IMP-S4), the schema's formalization of an engine-side boundary has landed on an import line the ledger had already priced independently. Evidence that the schema tracks real structure; evidence, not proof.

## 1 · The owned half `[THEOREM — linear sector]`

The free flux-wave recurrence of record, `J(t+1) = 2J(t) − J(t−1) + c²ΔJ`, reproduces the axis dispersion `sin²(θ/2) = c²·sin²(k/2)` to 10⁻⁹ over 200 ticks (G1), and its `k = 0` mode has frequency exactly zero — the uniform slice is a fixed point, not an oscillator. Two structural facts make the *squared* object the owned one. First, the recurrence is second-order with real coefficients, so it constrains frequencies only through even functions of θ. Second, evenness is stencil-class-wide: any ±-symmetric symbol — including the 18-point production class — satisfies `symbol(k) = symbol(−k)` (G6, 50 random wavevectors), so ω²-only ownership is not an artifact of the axis-mode toy. This is the frontier's even/odd split appearing verbatim in the wave sector: the dynamics owns `ω²`; nothing in it addresses `ω`.

## 2 · The fiber and its section `[THEOREM — linear sector]`

Over each owned `(k, ω²)` datum sits the two-element orbit `{(k, ω), (−k, −ω)}` under simultaneous conjugation. The machine checks establish the torsor reading exactly (G2–G3):

- the two labels name the *same* real solution — `cos(kj − θt + φ) ≡ cos(−kj + θt − φ)` pointwise; the real sector is the quotient;
- the update commutes with complex conjugation (real coefficients), so no update-derived functional selects a branch: **branch blindness**, the analogue of FTD-0243's commutativity for this wall;
- the real field is one-slice underdetermined — a snapshot admits two distinct futures (velocity sign) — while the branch-selected complex mode is one-slice deterministic with exact first-order evolution.

The section is precisely a complex structure on the real solution space (positive-frequency projection; equivalently the analytic-signal choice). Structure group ℤ/2; owned invariants = the even algebra; import = the branch. This is instance **I7** of the corrected (torsor-form) schema, with the same shape as the δ wall: a quadratic fiber `X² = ω²` whose symmetric data is owned and whose root order is imported. Scope guard: branch blindness is proven for the linear sector's update class, not for every conceivable substrate mechanism — the same scoping discipline as MC-T4.3.

## 3 · The anatomy of "not Schrödinger" `[DERIVED — linear sector]`

FTD-0270 measured that the native lattice is not a quantum-dynamics engine. The split above says *why*, and says it quantitatively (G5):

- **Native (no imports):** linear and gapless — `θ(k)/|k| → c`.
- **Mass import alone:** gapped but still second-order real — quadratic *spectrum* `(θ(k) − θ₀)/k² →` the predicted coefficient `c²/(2·sinθ₀)` (verified to 10⁻³), yet the real field remains one-slice underdetermined: no first-order evolution.
- **Branch import alone:** first-order complex evolution, but massless — no gap, no quadratic regime.
- **Both imports:** first-order complex evolution with quadratic-above-gap phase — the nonrelativistic *form*.

So the "cavity-not-Schrödinger" boundary decomposes exactly: **quadratic first-order evolution = one owned-side purchase (the restoring term) + one section-side purchase (the IMP-S2 branch), jointly necessary, severally insufficient.** What this does not do: it derives no Schrödinger equation for matter — there is no ℏ (θ is a geometric phase per tick), no interaction, no Pauli structure, no Born rule, and no claim that any native nonlinear mechanism cannot gap the spectrum (FTD-0333 returned `[INVALID per pre-registration]`, not a clean negative; that door stays `[OPEN]`). FTD-0270's tag is untouched.

## 4 · Proposed row split (owner-facing; not applied)

The frontier's §4 row 5 currently carries the whole boundary as one ATTEMPTED entry. This document's decomposition supports splitting it: **5a (mass gap)** — owned-side, remains ATTEMPTED/`[OPEN]` with the FTD-0333-v2 falsifier (a native regime showing `ω²(0) > 0`); **5b (first-order complex form)** — section-side, identified with the already-priced IMP-S2 line and inheriting its `[SELECTION]` status and its S2-a sharpening target (the ±J equivariance theorem). The split moves no tag by itself; adopting it in `FOUND_MODULUS_ARGUMENT_FRONTIER.md` is an owner edit, proposed here and recorded in the LEDGER row.

## 5 · Status line

Branch blindness, ω²-ownership, one-slice underdetermination: `[THEOREM — elementary, machine-verified 6/6, linear free flux-wave sector]`. Two-import anatomy: `[DERIVED — same scope]`. IMP-S2 identification and the FTD-0270 split: `[SYNTHESIS]` — the second schema rediscovery of a priced import line. Native nonlinear gapping: `[OPEN]`. Nothing promoted: FTD-0270 stays `[MEASURED — BOUNDARY]`; IMP-S2 stays `[SELECTION]`-priced; FC-1/FC-2 declined; FC-W adopted; `x₊ = 1/α` `[SMC]`; the four-walls forcing theorem `[OPEN]`, its instance count now seven.
