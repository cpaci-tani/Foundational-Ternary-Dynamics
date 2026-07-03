# ANALYSIS — Λ source-gap L-scan feasibility: the engine route is boundary-limited

**Tag:** `[SYNTHESIS] + [BOUNDARY]` (a feasibility finding; closes nothing, sharpens why the source gap resists engine closure). **LEDGER id:** FTD-0364.
**Governs:** the FTD-0331 `[OPEN]` **source** gap (gap #1) and the FTD-0332-registered L-scan open check.
**Read first:** `DERIV_LAMBDA_SCALE_COVARIANT.md` §3–§5 (the source gap); `SPEC_COSMOLOGY_FRAMEWORK_BOUNDARY.md` (the pre-declared L-scan); `AUDIT_INFINITY_REFRAME.md` §66/§116 (undefined-boundary + the Phase-G precedent).

---

## §0 · Question and verdict

Can the FTD-0331 `[OPEN]` **source** gap — "does the substrate carry an *intensive, area-law* vacuum-energy density `ρ_vac ∝ L⁻²` (a native nonzero Λ), or does it *leak* (`~L⁻⁵`, FTD-0273) so that `Λ=0` is the final word?" — be **cleanly decided by an engine `ρ_vac(L)` L-scan?**

**No — the L-scan route is boundary-limited.** A pre-registered engine L-scan was scoped in full (config, observable, L-grid, bands) and found to be obstructed three independent ways, any one of which is disqualifying for a *clean* verdict. The source gap stays `[OPEN]`; it is **not** closable by the naive instrument the FTD-0332 note gestured at, and the honest deliverable is this boundary on the method itself. **Zero promotions:** x₊=1/α `[SMC]`; MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`; FTD-0331's source stays `[OPEN]`, value `[BOUNDARY]`, dissolution `[DERIVED]`, ceiling `[SELECTION]` — all unchanged; no α or Λ derived.

## §1 · Three obstructions

**Obstruction 1 — circular by construction.** FTD's native vacuum is `ρ_vac = 0` *by the §1 `[DERIVED]`* (every Lagrangian term vanishes at `(J=0,s=0)`; `DERIV_LAMBDA_SCALE_COVARIANT.md` §1). Scanning the *unperturbed* vacuum measures floating-point zero at every L — a tautology, and to report it as a "confirmed area-law-absence" would promote a definition to a measurement (a zero-promotion violation). The only non-trivial `ρ_vac(L)` requires *injecting* energy — but a fixed energy dump on a lossless periodic box gives `ρ = E_fixed/L³ ∝ L⁻³` **by conservation arithmetic**, not physics. FTD-0273's `~L⁻⁵` is exactly this: a fixed condensate energy smearing into a growing box, sampled in a transient near-field window (`ANALYSIS_CLUSTER_ENERGY_SPECTROSCOPY_v1.md`). Reporting either the `0` or the `L⁻³`/`L⁻⁵` as a "measured vacuum exponent" measures the box, not a source.

**Obstruction 2 — the observable is ill-posed.** Determinism (gate E3) requires `langevin` OFF, but then the lossless periodic box **never settles**: total energy is conserved and *recurs* (FPU-like), so the whole-lattice `ρ_field(t) = ½Σ|J|²/L³` sloshes between field and wave channels indefinitely and the time-averaged value is **measurement-window-dependent**. There is no well-defined steady-state `ρ_vac` to fit. Turning `langevin` ON supplies a genuine thermal steady state but **breaks determinism** — the two requirements are in direct tension, and the conflict is not resolvable within the current single-substrate engine. This is the same non-settling pathology `ANALYSIS_CLUSTER_ENERGY_SPECTROSCOPY_v1.md` documents for FTD-0273.

**Obstruction 3 — the Green's-function trap (Phase-F/Phase-G precedent).** Even a clean fitted exponent would be untrustworthy. `AUDIT_INFINITY_REFRAME.md` §116 records the decisive precedent: a lattice L-scan produced a crisp scaling "plateau" (the α extraction) that turned out to be the **periodic-lattice Poisson Green's function with zero fine-structure content** (`DERIV_EMERGENT_COULOMB_GEOMETRIC.md`). A sourced field's energy density on a periodic lattice is governed by that same `G_L(r)`; a clean `L⁻²` could be its finite-size boundary term, **not** an area-law vacuum source. Any positive result would first have to survive a zero-coupling Green's-function deflation check — which, on the precedent, more likely than not deflates it.

## §2 · Even the deflation-safe instrument cannot close the gap

The one non-circular design (fixed *condensate density* injection `∝ L³`, a whole-lattice-vs-dilution two-channel observable, and a mandatory Green's-function deflation gate) was worked out and remains buildable, but its ceiling is low: its **prior-favoured outcome is the leak/dilution negative** that merely re-confirms the already-predicted `Λ=0` (`SPEC_COSMOLOGY_FRAMEWORK_BOUNDARY.md` pre-declares outcome B prior-favoured), and even an improbable positive `L⁻²` would establish only that *a manifested condensate sources an intensive field-energy density* — a `[CONJECTURE]`-grade identification with cosmological Λ, leaving the **value** a `[BOUNDARY]` (needs `L_H`; FTD-0059) regardless. The measurement cannot reach the thing that is actually `[OPEN]` at a level that would move a tag.

## §3 · Honest ceiling — the measurement is under-posed for the claim

Under undefined-boundary ontology `lim_{L→∞} ρ_vac` is **not well-posed** (`AUDIT_INFINITY_REFRAME.md` §19/§66): "Λ→0 as the universe grows" cannot be stated as an FTD claim without an ε-L restatement. A finite L-grid reports an exponent *over a bounded window*; it can never establish an asymptotic Λ. Combined with Obstruction 2 (no steady state to fit) and Obstruction 1 (nothing but the box to measure), the engine L-scan is **under-posed for the cosmological question** it was meant to answer.

## §4 · Consequence for the instruments

`engine/tests/campaign_vacuum_energy.cpp` is marked **superseded**: it is doubly disqualified for this purpose — it computes the `½Σ½ℏω` Brillouin-zone zero-point sum FC-1 explicitly declines (`DERIV_LAMBDA_SCALE_COVARIANT.md` §1) and bakes in the retired `α¹⁶·G*²` numerology, and it is a standalone k-space integral with **no `RenderBridge`, no lattice state, and no `L`** — it cannot scan and its physics is the superseded rationale. No `campaign_lambda_lscan.cpp` is built: on the finding above it would, at best cost, return the prior-favoured negative or (more likely) UNDERDETERMINED, and cannot in principle move FTD-0331's source `[OPEN]` or value `[BOUNDARY]` tags.

## §5 · Disposition

The FTD-0331 source gap remains `[OPEN]` and is now annotated with *why* it resists engine-L-scan closure. The FTD-0332-registered open check is refined: the discriminator is real *in principle* but **not cleanly measurable** by the current engine (ill-posed observable, dilution/source confound, Green's-function artifact). This is a Number-One-Goal clause-2 boundary result — marking a check the ontology's current engine cannot perform, not force-closing it. Provenance: scoped by a dedicated research pass over `DERIV_LAMBDA_SCALE_COVARIANT.md`, `SPEC_COSMOLOGY_FRAMEWORK_BOUNDARY.md`, `ANALYSIS_CLUSTER_ENERGY_SPECTROSCOPY_v1.md`, `AUDIT_INFINITY_REFRAME.md`, and `campaign_vacuum_energy.cpp`; no engine run was performed (correctly — the instrument would not have been decisive).
