# Pre-Registration — Route B / B1: von Neumann Type of the FTD Substrate Algebra (v1)

**Tag:** `[PRE-REGISTRATION]` — locks the design of the **gating** attempt for Route B (time as modular flow). **Contains no result.** Three outcomes — FOUND (type III₁) / UNDERDETERMINED / CLOSED-NEGATIVE — all pre-blessed; verdict genuinely open. Prior-favoured: III₁ (driven by flux masslessness), but the "derived-state" obligation and the lattice's discrete spectrum keep it open.
**Date:** 2026-05-29
**Hash-lock target tag:** `preregister-modular-time-algebra-type-v1`
**LEDGER row reservation:** FTD-0225 (provisional; confirm next-free at lock — note the 0210–0216 alpha-readout tangle flagged in FTD-0224).
**Companion:** `SCOPE_ROUTE_B_MODULAR_TIME.md`. **Cross-refs:** Phase-G geometric Coulomb (massless flux, `[THEOREM]`); FTD-0051 (Langevin equilibrium); FTD-0208 (the L²-law obstruction Route B targets); FTD-0214 (Connes-λ foothold).

> Discipline: §§1–7 hash-stamped before the attempt; commit deferred per owner pattern (provenance via in-session SHA). Result lands in a separate doc. Defective design → v2, not an edit.

---

## §1 — Context

Route B (per the scope memo) needs the substrate's local algebra to be **type III₁**, because only then is the Tomita–Takesaki modular flow **canonical (state-independent up to inner automorphisms)** — hence a candidate for *objective, derived* time rather than a circular inserted-Hamiltonian flow. B1 is the gating question: **is it III₁?** Everything downstream (B2: is the flow a boost → L²?) is moot if B1 fails.

The viable route to III₁ is the IR one (§2 of the scope): the unbounded ternary+flux lattice is an Araki–Woods (ITPFI) factor whose type is the **Connes S-invariant = asymptotic ratio set `r∞`** of the natural state's local spectra; a **continuous (gapless)** spectrum gives `r∞ = [0,∞) =` III₁. FTD's flux sector is gapless (Phase-G geometric Coulomb), so the prior favours III₁ — but this must be *computed* from FTD's derived equilibrium state, not assumed.

---

## §2 — The question (LOCKED)

**Q-B1.** For the **linear massless flux sector** of the FTD substrate (the Phase-G wave/flux field on the unbounded cubic lattice, fixed spacing `≡ ℓ_P`), in its **derived Langevin/KMS equilibrium state** (FTD-0051), what is the **Connes S-invariant** (equivalently the Araki–Woods asymptotic ratio set `r∞`) of the quasi-local von Neumann algebra — i.e. is it **type III₁** (`r∞ = [0,∞)`), or type I / II / III_λ?

The verdict is genuinely open; all §6 outcomes are pre-blessed.

---

## §3 — Admissible ingredients (LOCKED)

- **The FTD linear flux sector:** the discrete wave/flux field, its lattice **dispersion relation `ω(k)`** (from the Phase-G/wave-equation stencil), and the gapless spectrum (Phase-G geometric Coulomb, `[THEOREM]`).
- **The natural equilibrium state:** the **Langevin/Ornstein–Uhlenbeck KMS state** of that linear sector (FTD-0051), with its local two-point function / occupation spectrum — **derived from FTD dynamics, not chosen.**
- **The Araki–Woods / Connes machinery:** ITPFI factor of the infinite tensor product of finite site-algebras; the asymptotic ratio set `r∞`; the Connes S-invariant classification; Tomita–Takesaki modular theory.

**Out of scope (NOT admissible):**
- Assuming relativistic-QFT type results (e.g. "QFT local algebras are III₁") rather than computing from the FTD state.
- **Inserting a Hamiltonian / choosing ρ** to obtain a desired flow (the state must be the derived equilibrium).
- Any `ε → 0` continuum limit (FTD's spacing is fundamental; the type must come from infinite extent + spectrum).
- Assuming Lorentz/Poincaré covariance (that is B2's forbidden-until-derived territory).

---

## §4 — Benchmark (LOCKED)

The deliverable is the **type classification** (the Connes S-invariant). The "target" is **III₁**, but the classification is reported as computed; no fitting of `r∞` to `[0,∞)`.

---

## §5 — Falsifiers (LOCKED, mechanical)

- **BF1 — assumed answer.** The type is asserted from analogy to relativistic QFT rather than computed from FTD's `ω(k)` + equilibrium spectrum.
- **BF2 — inserted dynamics.** A modular Hamiltonian / state ρ is chosen (not derived from the Langevin/KMS equilibrium) so the flow comes out as wanted — circular.
- **BF3 — continuum smuggling.** An `ε → 0` limit is used to manufacture III₁ (forbidden by the fixed-spacing ontology); the type must arise from the IR.
- **BF4 — Lorentz assumption.** Poincaré covariance is assumed (reserved for B2; barred here).
- **BF5 — ratio-set fitting.** `r∞` is fit/forced to `[0,∞)` by a free parameter rather than read off the derived spectrum.
- **BF6 — CODATA / constant insertion.**

---

## §6 — Three pre-blessed outcomes (LOCKED)

- **FOUND.** Computed `r∞ = [0,∞)` (Connes `S = ℝ₊`) → **type III₁** → the substrate's modular flow is canonical/objective → Route B's precondition is **met**. Proceed to **B2** (is the wedge modular flow a boost → L², non-circularly?). *No spine tag moves; no claim that time is yet derived — only the precondition.*
- **UNDERDETERMINED.** The type depends on an unforced choice (e.g. the equilibrium state isn't uniquely fixed by FTD dynamics), or the computation reaches only a partial classification, or it lands **III_λ** (a non-canonical flow — suggestive but not the objective III₁).
- **CLOSED-NEGATIVE.** Computed type is **I or II** (semifinite → tracial/state-dependent, non-canonical flow), with no path to III₁ from the linear sector → Route B **cannot derive objective time** from the substrate algebra; the program reverts to **Route A** (import the metric). A recognized boundary deliverable (CLAUDE.md goal-clause 2): it would sharpen *why* discreteness alone doesn't yield relativistic time.

---

## §7 — Method (LOCKED, ordered)

1. **Site algebra & quasi-local structure.** State the finite per-site algebra (ternary state ⊕ flux d.o.f.) and the unbounded-lattice infinite-tensor-product (ITPFI) structure.
2. **Derived equilibrium state.** Obtain the Langevin/KMS equilibrium two-point function of the linear flux sector from FTD dynamics (FTD-0051) — *derived, not chosen*. Record its mode-occupation spectrum as a function of `ω(k)`.
3. **Local spectra.** Compute the eigenvalue distribution of the local (per-mode / per-region) density matrices from step 2.
4. **Asymptotic ratio set.** Compute `r∞` (the Connes S-invariant of the ITPFI factor) from the asymptotic spectrum — paying attention to whether the gapless `ω(k) → 0` modes make `r∞` continuous (`[0,∞)`) or leave it discrete.
5. **Classify** `r∞` → type (I / II / III_λ / III₁).
6. **Falsifier checklist** (§5, mechanical) **then** verdict per §6. (No numerical-target comparison; the deliverable is the type itself.)

**Substrate.** Primarily desk computation (Gaussian-state / Araki–Woods spectral analysis), optionally checked against finite-lattice numerics for the occupation spectrum (instrument SHA recorded if so).

---

## §8 — Hash-lock

`sha256sum` this file; record in-session and (at canonization) in `REF_PREREGISTER_MANIFEST.md` + LEDGER (FTD-0225). Commit deferred per owner; canonize with B-9 (temporal separation) + B-10 (independent review). Defective §3 ingredient or §5 falsifier → **v2**, not a v1 edit.

*Authored 2026-05-29. **No result.** The gating type-classification is genuinely open; prior-favoured III₁ via flux masslessness, but the derived-state obligation (BF2) and the lattice's discrete spectrum are the live risks. Engineering toward III₁ invalidates the attempt.*
