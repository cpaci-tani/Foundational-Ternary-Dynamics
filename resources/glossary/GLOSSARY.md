# FTD Glossary

FTD-specific vocabulary with one-line definitions. Canonical symbol list: `docs/reference/REF_SYMBOL_GLOSSARY.md`.

## Ontology

- **Voxel** — a single lattice site at integer coordinates, holding a ternary state `s ∈ {−1, 0, +1}` and a flux vector `J ∈ ℝ³`.
- **Flux field `J(x)`** — the continuous vector field on the lattice; the *dispositional* layer of the two-layer ontology.
- **State field `s(x)`** — the discrete ternary field; the *actual* (manifested) layer.
- **Manifestation** — the process by which `|J| ≥ K_B` at a voxel flips `s` from 0 to ±1, creating a particle.
- **Genesis** — the complete fill of all `N_c = 3` color channels at a voxel, crossing `K_GENESIS = K_B · N_c`.
- **Moore neighborhood** — the 26 + self = 27 cells within one lattice unit of a voxel. See `cheatsheets/MOORE_NEIGHBORHOOD.md`.
- **Existential unit** — the 3³ cube of 27 voxels around a central voxel; the smallest self-contained FTD "region".
- **Bandwidth budget** — `v² + L² < 1` where `v` is lattice velocity and `L` is topological latency. Exceeding it stalls the voxel.
- **Latency `L`** — the gravitational-field analog; grows where mass/energy concentrates, slowing clocks.

## Math primitives

- **G\*** — the lemniscatic bridge constant `Γ(1/4)/Γ(3/4) ≈ 2.9587`. Single primitive alongside the axioms.
- **ϖ** (varpi) — the classical lemniscate constant `Γ(1/4)²/(2√2 Γ(1/2))`. Related to G* by `G* = 2ϖ/√π`.
- **Master quadratic** — `x² − 16 G*² x + 16 G*³ = 0`. Roots `x₊ ≈ 137.030` and `x₋ ≈ 3.024`.
- **Watson identity** — `W₃ = G*²/(2π)`, the Brillouin-zone integral on BCC. See `cheatsheets/MOORE_NEIGHBORHOOD.md`.

## Framework integers

- **N_c = 3** — number of colors (SU(3) triplet dimension).
- **N_base = 4** — smallest FLT-forbidden exponent, base dimensionality.
- **b_3 = 7** — QCD β-coefficient at `N_f = 6`.
- **N_eff = 13** — effective degrees of freedom; Fibonacci `F_7`.
- **D_constraint = 47** — `N_c · N_base² − 1`.

## Physics quantities

- **K_B** — manifestation threshold, `0.511 MeV` (= electron mass).
- **K_GENESIS** — genesis threshold, `K_B · N_c = 1.533 MeV`.
- **C_SPEED** — lattice light speed, `1/√3 ≈ 0.577` in lattice units (CFL stability).
- **DAMPING** — dissipation rate, `α ≈ 7.297 × 10⁻³`.
- **G_N** — gravitational coupling on lattice, `1/(b_3 + N_c)² = 0.01`.
- **α** — fine-structure constant, `1/x₊ ≈ 1/137.036`.
- **α_s** — strong coupling at M_Z, `b_3 / (b_3 + 4 N_eff) = 7/59 ≈ 0.119`.
- **sin²θ_W** — Weinberg angle, `N_c / N_eff = 3/13 ≈ 0.231`.

## Engine / simulation

- **Tick** — one discrete time step. See `cheatsheets/ENGINE_TICK_CYCLE.md`.
- **Phase (of a tick)** — one of six sub-steps: `phase_read → phase_write → gauss_project → phase_forces → phase_movement → tick++`.
- **Toggle** — a boolean flag enabling/disabling a physics mechanism (damping, gravity, confinement, etc.). See `engine/web/js/config/toggles.js`.
- **Scenario** — a named initial condition + parameter set. Loaded on scale switch or scenario-dropdown change.
- **Capability** — a per-scale interface object on the bridge exposing that scale's read/write/tick methods (`bridge.capabilities.scale0.tickScale0()` etc.).
- **Bridge** — the abstraction over the simulation backend (WasmBridge ← C++ engine, MockBridge ← JS-only).
- **Scale (0–11)** — the conceptual level the simulation is operating at. Scale 0 = substrate lattice; Scale 11 = consciousness.

## Timeline / playback

- **Memory recorder** — rolling ring buffer of sim snapshots; LOD-tiered age decay.
- **Render controller** — offline fast-forward that captures a dense clip into a separate buffer.
- **Snapshot LOD** — level of detail: 0 (full N³), 1 (N/2)³, 2 (N/4)³, 3 (telemetry-only).
- **Scrubbing** — dragging the scrub-bar thumb to hydrate the engine from a timeline snapshot.
- **Hydrate** — load a snapshot back into the engine buffers for display.

## Epistemic

- **[AXIOM]** — structural postulate. Not derivable.
- **[THEOREM]** — rigorously proven from axioms.
- **[SELECTION]** — argued from consistency; not uniquely proven.
- **[CONJECTURE]** — hypothesis awaiting validation.
- **[IMPOSED]** — parameter input, not output.
- **[EMERGENT]** — arose from dynamics; not designed in.
- **[OPEN]** — unresolved.

## Relationships to standard physics

- **Flux J** ↔ vector potential A (gauge-fixed, Coulomb gauge)
- **∇·J** ↔ charge density ρ (Gauss's law is a constraint, not a consequence, in FTD)
- **|J|² = |J_L|² + |J_R|²** ↔ |ψ|² (Born rule at Tier 1)
- **arg(J_L + i J_R)** ↔ quantum phase φ
- **Ternary state s** ↔ particle presence + sign of charge
- **Moore neighborhood** ↔ gauge-group origin (see above)
- **Lattice spacing 2/D** ↔ UV cutoff in φ³ EFT

## Cross-references

- Full symbol list with LaTeX forms: `docs/reference/REF_SYMBOL_GLOSSARY.md`
- Epistemic labels: `docs/reference/REF_EPISTEMIC_LABELS.md`
- Naming conventions: `docs/reference/REF_NAMING_CONVENTIONS.md`
- Scope limitations: `docs/reference/REF_SCOPE_LIMITATIONS.md`
