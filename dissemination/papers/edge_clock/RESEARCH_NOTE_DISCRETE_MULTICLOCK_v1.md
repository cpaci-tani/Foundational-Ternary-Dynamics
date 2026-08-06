# Research Note — The Discreteness Edge and the Two-Clock Edge (Paper-II material)

**Status:** [EXPLORATORY — measured results verified in-session; NO independent audit yet; nothing enters the main paper or any ledger until audited]
**Date:** 2026-08-06 · **Code:** `figures/discrete_edge.py` (+ refinement sweep in session record) · **Figure:** `figures/fig6_discrete.pdf`

## Scope

Three new "edges" beyond the main paper's crossover, all in the same normal-form family: (1) the exact boundary between discrete-time and continuous dynamics for the quartic clock; (2) that boundary across the whole pitchfork crossover; (3) the localization edge of a two-clock system. Everything here uses only the leapfrog (Störmer–Verlet) map and direct simulation; the two conjectures are flagged as such.

## Results

**R1 — Scaling theorem [EXACT, machine-verified].** The leapfrog map for ẍ = −(4λ/m)x³ is exactly equivariant under (x, p, Δt) → (sx, s²p, Δt/s); verified to 0.0e0 over 2000 steps. Consequence: every discreteness phenomenon of the pure quartic clock depends on the single invariant **ρ = Δt/T(A)** (timestep per period). "How discrete can time be before the clock notices" is a one-parameter question.

**R2 — The discreteness edge [MEASURED].** Instability onset of the discrete pure-quartic clock at **ρ_c = 0.1222** (60k steps, blowup criterion), against the hand-derived turning-point-stiffness bound ρ ≤ 2/(√6·√π·G*) = 0.15570. The naive bound is an upper bound only: parametric (Mathieu-type) resonance from the orbit's own frequency modulation erodes it by ~22%, and the region above onset is an instability *band* with interleaved stable windows (tongue structure, fig 6a). A harmonic clock survives to ρ = 1/π = 0.3183; the quartic clock tolerates only ~38% of that discreteness.

**R3 — The edge across the crossover [MEASURED; method validated].** ρ_c(k²) is a monotone-decreasing universal curve: 0.3163 at k² = 0.05 (vs the exact harmonic bound 1/π = 0.3183 — 0.6% validation of the method), 0.123 at the quartic point, 0.059 deep over-barrier (k² = 0.95), with plateau/staircase structure suggesting resonance locking (fig 6b). Reading: the closer a clock runs to (and past) its edge, the finer the time resolution required to simulate — or *be* — it faithfully.

**R4 — The two-clock self-trapping edge [MEASURED, sharp; value CONJECTURED].** Two identical quartic clocks coupled by ½κ(x₁−x₂)², all energy initially in clock 1. The same scaling symmetry forces a single coupling parameter **χ = κ/(2λA²)**. Measured: energy transfer to clock 2 is blocked (self-detuning localization — the receiving clock changes frequency as it receives, spoiling its own resonance) below a razor-sharp threshold:

> **χ_c = 0.33325 ± 0.00025** — transition from 28% to 100% transfer within one 5×10⁻⁴ grid step; independent of transfer cutoff (0.30/0.45/0.60) and converged in duration (600/1200/1800 periods).

**Conjecture C1: χ_c = 1/3 exactly.** The competing pretty value 1/G* = 0.33799 is *excluded* by 9σ of the bracket — recorded deliberately, since the first coarse sweep landed at 0.337 and invited exactly that misidentification; the fine sweep killed it. (Base-rate lesson applied to our own numbers.)
**Also observed:** re-entrant localization windows just above χ_c (transfer collapses again near χ ≈ 0.36 and 0.39, fig 6c) — unexplained.

## Derivation status for C1 (χ_c = 1/3) — 2026-08-06 session

**Mechanism: ESTABLISHED [MEASURED + MODEL].** Three independent probes agree:
1. Floquet analysis of the localized (breather) orbit: max|multiplier| = 1.000000
   for all χ ∈ [0.25, 0.40] — the transition is **not** a linear instability;
   the Hill/Lamé route is closed (checked before climbing it).
2. Frequency-resolved dynamics at the threshold: below, clock 1's frequency is
   constant forever and clock 2 stays slaved; above, clock 1 **chirps down**
   (1.098 → 0.898/T₀) while clock 2 rises to meet it (0.930/T₀) —
   **mutual capture into 1:1 resonance with runaway** (transfer lowers Ω₁ ∝ E₁^{1/4},
   raises ω₂ — positive feedback to full exchange).
3. Averaged-separatrix models reproduce a sharp pole-to-pole threshold of the
   right size. The criterion: the initial all-in-one-clock "pole" state can reach
   the saddle at equal action split, worst relative phase (carrier autocorrelation
   at −⟨cn²⟩, exact by half-wave antisymmetry).

**Value: NOT yet derived. χ_c = 1/3 stays [CONJECTURE].** Three refinement stages
of first-order averaging bracket but do not select it:
- single-harmonic carriers: χ_c = 0.288
- exact lemniscatic carriers, bare clocks: χ_c = 0.320
- exact carriers, coupling-dressed clocks (μ = κ crossover oscillators, exact
  actions by quadrature): χ_c = 0.361

Measured 0.33325(25) sits inside the bare/dressed bracket: the answer is
sensitive to carrier dressing at the ~10% level, i.e., first-order averaging is
insufficient and the clean value (if it is exactly 1/3) must come from the
**exact resonance Hamiltonian**.

**RESOLUTION (same session, later): C1 REFUTED.** Before more analytics, two
decisive numerical tests of whether an exact constant exists at all:
1. **Ultra-fine sweep (5×10⁻⁵ resolution, 2400 periods, turning protocol):**
   the "threshold" dissolves into fine structure — an isolated full-transfer
   WINDOW at χ = 0.33265–0.33270, with transfer blocked again (28%) at
   χ = 0.33275–0.33285. The coarser sweeps' sharp edge was aliasing the first
   window of an interleaved channel structure.
2. **Protocol test:** starting clock 1 at mid-swing (x₁ = 0, p₁ = √2E₀; same
   energy, quarter-period phase shift) gives full transfer at every χ in the
   scanned range down to 0.332 — the onset is **initial-phase-dependent**.

Conclusion: there is no protocol-independent constant χ_c. The two-clock
transfer edge is a **chaotic-transport boundary with resonance-channel fine
structure**, onset near χ ≈ 0.33 for the turning protocol — a KAM-type
object, for which no clean closed form should be expected. Both pretty
identifications died under escalating resolution: 1/G* (killed at 5×10⁻⁴)
and 1/3 (killed at 5×10⁻⁵). The base-rate lesson, twice in one object.

**What survives of C1's investigation [MEASURED + MODEL]:** the capture
mechanism (Floquet-stable localized state; frequency-convergence runaway);
the averaged-separatrix bracket [0.29, 0.36] correctly localizing the chaotic
zone; and a sharpened Paper-II scope — the right questions are transport
questions (window structure, phase-dependence of onset, channel widths via
resonance overlap), not constant-hunting.

## What needs doing (the honest queue)

1. **Derive χ_c = 1/3** (or refute it): rotating-wave reduction of the coupled pair with *lemniscatic* carriers → DNLS-dimer-type self-trapping condition; the cn-carrier overlap integrals are elliptic, so a closed form should exist. This is the note's headline open problem.
2. **Derive the tongue structure of R2**: the orbit's instantaneous stiffness 12λx²(t)/m is a known elliptic function of time, so the discrete-map stability problem is a Hill equation with cn² coefficient — the tongue boundaries should be computable exactly (Lamé equation territory). Genus-1 machinery once more.
3. Explain the R3 plateaus and the R4 re-entrant windows.
4. Independent refute-by-default audit of all of the above before anything is registered or added to any manuscript.
5. IC-robustness: thresholds measured from one protocol (clock 2 at rest, zero phase); sweep initial phases.

## Relation to the main paper and to FTD

Main paper: untouched — this is Paper-II material (the stochastic/discrete program promised in the "extrapolation" discussion). FTD: no claims; but the *language* built here — "how discrete can a clock's time be before its physics notices," answered by a universal ρ_c(k²) curve — is the mathematically clean form of a question the discrete-substrate program cares about, available for free once this is audited.
