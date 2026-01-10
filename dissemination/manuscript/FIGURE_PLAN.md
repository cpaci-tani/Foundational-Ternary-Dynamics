# FTD Manuscript Figure Plan (Quarto)

Scope: `dissemination/manuscript/` (the Quarto book used for `_book/` outputs).

## Current State (Audit)

- **Book pages**: 72 `.qmd` files in `_quarto.yml` order (index/preface + 68 chapters + back matter).
- **Figures currently embedded**: 116 images across **66/68** chapter files.
  - **ASCII placeholder diagrams** (`fig-ascii-*`): 0 embeds (chapters only)
  - **Generated figures** (`fig-*.png`): 116 embeds (includes `index.qmd`)
- **Chapters with zero figures**: **2/68** (`14.2-equations-reference.qmd`, `14.3-glossary.qmd`).
- **Figure generator**: `dissemination/manuscript/figures/generate_figures.py` writes **PNG + SVG** into `dissemination/manuscript/figures/chXX/`.

## Progress (Implemented)

- All chapter sources: replaced `fig-ascii-*` embeds with generated figures (0 remaining).
- `ch00/fig-intro-hierarchy-of-being` embedded in `index.qmd`
- `ch00/fig-epistemic-claim-types` embedded in `chapters/0.1-first-principles.qmd`
- `ch00/fig-discrete-operators` embedded in `chapters/0.2-mathematics.qmd`
- `ch00/fig-ontological-levels` embedded in `chapters/0.3-philosophy.qmd`

## Figure Standards

- **File location**: `dissemination/manuscript/figures/chXX/fig-<slug>.png` (+ matching `.svg`).
- **Quarto embed** (current pattern):
  - `![](../figures/chXX/fig-<slug>.png){#fig-<slug> width="80%"}`
- **IDs**: keep `#fig-...` globally unique; prefer chapter-scoped slugs like `fig-voxel-anatomy-...` over generic `fig-diagram`.
- **Output**:
  - PNG = stable for PDF builds
  - SVG = crisp for HTML; keep generating even if we reference PNG today

## Implementation Workflow

1. **Inventory each chapter**: identify (a) ASCII diagrams to replace and (b) “concept jumps” needing a visual.
2. **Design figures** as either:
   - **Schematic diagrams** (flowcharts, layer diagrams, field arrows)
   - **Canonical plots** (log-log spectra, phase diagrams, timelines)
   - **Multi-panel figures** when a chapter needs 2–3 related visuals but we want to keep figure count sane.
3. **Implement** figure functions in `dissemination/manuscript/figures/generate_figures.py` and register them in `FIGURE_REGISTRY` under the appropriate `chXX`.
4. **Embed** figures into `.qmd` near first introduction of the concept, add `@fig-...` references in the surrounding text.
5. **Regenerate + render**:
   - `python dissemination/manuscript/figures/generate_figures.py`
   - `cd dissemination/manuscript && quarto render`

## Priority Tiers

- **Tier 0 (core narrative + core derivations)**: Prolegomena, Book I, Book II, and the main Lemniscate-Alpha/Action machinery; plus the constants/self-consistency appendices.
- **Tier 1 (big remaining gaps)**: Book VII–XIV chapters with ASCII diagrams and no figures.
- **Tier 2 (polish)**: add second/third figures in chapters that already have one but still lean on ASCII or would benefit from a canonical plot.

## Chapter-by-Chapter Backlog (Minimal Viable Figure Pass)

Each line is “one figure pack per chapter”: prefer 1 multi-panel figure where appropriate.

### Front/Back Matter

- `index.qmd`: `ch00/fig-intro-hierarchy-of-being` (flow diagram replacing the ASCII hierarchy in the intro). (implemented)
- `about.qmd`: no figure planned.
- `symbols-glossary.qmd`: no figure planned.

### Prolegomena (ch00)

- `chapters/0.1-first-principles.qmd`: `ch00/fig-epistemic-claim-types` (taxonomy diagram: AXIOM/THEOREM/SELECTION/CONJECTURE/IMPOSED/EMERGENT/OPEN). (implemented)
- `chapters/0.2-mathematics.qmd`: `ch00/fig-discrete-operators` (finite-difference stencil panels for grad/div/curl + Moore neighborhood callout). (implemented)
- `chapters/0.3-philosophy.qmd`: `ch00/fig-ontological-levels` (Void → Flux → Manifestation + "graded monism" framing). (implemented)

### Book I: Foundations (ch01)

- `chapters/1.1-the-void.qmd`: `ch01/fig-void-three-states` (triangle/graph of {0,+1,-1} + transitions; complements the “Three States” table). (implemented)
- `chapters/1.2-the-first-division.qmd`: `ch01/fig-genesis-pair-production` (0 → (+1,−1) with conservation “balance sheet”; optional inset: `p_create(E)` curve). (implemented)
- `chapters/1.3-the-two-layers.qmd`: `ch01/fig-two-layers-split` (potential/flux field vs manifestation/particles, “ocean metaphor” in one diagram). (implemented)
- `chapters/1.7-time-and-causality.qmd`: `ch01/fig-time-gate-and-lightcone` (phase accumulator/time dilation + discrete lightcone / speed limit). (implemented)
- `chapters/1.9-constants.qmd`: `ch01/fig-constants-dependency` (what’s axiomatic vs derived vs imposed; arrows from {b3,Nc,neff,Nbase} where applicable). (implemented)
- `chapters/1.10-lemniscate-alpha.qmd`: **3-figure set** (implemented):
  - `ch01/fig-lemniscate-alpha-curve` (the parametric curve, harmonic components or color-coded modes)
  - `ch01/fig-master-quadratic-roots` (quadratic with x± = 137.036 and 3.024 highlighted)
  - `ch01/fig-derivation-chain-alpha` (constraint chain → G* → quadratic → roots)
- `chapters/1.11-the-action-principle.qmd`: `ch01/fig-action-terms` (block diagram of action terms: flux dynamics, coupling, manifestation potential, constraint; optional: Noether → conservation mini-panel). (implemented)

### Book II: The Subatomic Realm (ch02)

- `chapters/2.2-voxel-anatomy.qmd`: `ch02/fig-voxel-structure` (voxel data structure schematic) + optional small inset for Moore neighborhood. (implemented)
- `chapters/2.3-the-particle-zoo.qmd`: `ch02/fig-standard-model-overview` (compact SM chart: fermions by generation + bosons; highlight FTD “state” mapping) + replace proton/neutron ASCII triangles with `ch03` images or a dedicated `ch02` baryon schematic. (implemented)
- `chapters/2.5-the-higgs-mechanism.qmd`: `ch02/fig-mexican-hat-and-mass` (Mexican hat potential + symmetry breaking → W/Z masses; one multi-panel figure). (implemented)

### Book III: The Atomic Realm (ch03)

- `chapters/3.3-electron-dynamics.qmd`: `ch03/fig-electron-shells-and-spectra` (Aufbau filling order + energy level diagram/spectral lines). (implemented)
- `chapters/3.4-nuclear-physics.qmd`: `ch03/fig-nuclear-binding-and-decay` (binding energy per nucleon curve replacing ASCII + decay modes panel). (implemented)

### Book IV: The Molecular Realm (ch04)

- `chapters/4.4-macromolecules.qmd`: `ch04/fig-macromolecules-overview` (amino acid + peptide bond + DNA base pairing as small panels; optionally reference `ch06` DNA/lipid figures rather than duplicating). (implemented)

### Book V: States of Matter (ch05)

- `chapters/5.1-states-of-matter.qmd`: `ch05/fig-states-of-matter-panels` (solid/liquid/gas/plasma particle cartoons + motion cues). (implemented)
- `chapters/5.3-exotic-states.qmd`: `ch05/fig-exotic-phase-map` (temperature vs density schematic locating BEC/superfluid/superconductor/QGP/neutron-star matter). (implemented)

### Book VI: Structures and Materials (ch06)

- No new mandatory figures (chapters 6.1–6.4 are already well illustrated); revisit in Tier 2 for any remaining ASCII.

### Book VII: The Planetary Realm (ch07)

- `chapters/7.1-gravity-wells.qmd`: `ch07/fig-gravity-well-and-formation` (planet formation stages + gravitational potential well panel). (implemented)
- `chapters/7.2-atmospheres.qmd`: `ch07/fig-atmospheres-core` (temperature-vs-altitude profile + greenhouse energy-balance panel). (implemented)
- `chapters/7.3-geology.qmd`: `ch07/fig-geology-core` (planet interior layers + plate boundary types or rock cycle). (implemented)
- `chapters/7.4-magnetospheres.qmd`: `ch07/fig-magnetosphere-structure` (magnetopause/bow shock/tail + aurora inset). (implemented)

### Book VIII: The Stellar Realm (ch08)

- `chapters/8.1-stellar-formation.qmd`: `ch08/fig-star-formation` (cloud collapse → fragmentation → protostar classes; optional IMF inset). (implemented)
- `chapters/8.3-stellar-nucleosynthesis.qmd`: `ch08/fig-nucleosynthesis-onion` (onion-shell burning schematic + burn-stage timescales). (implemented)
- `chapters/8.4-stellar-death.qmd`: `ch08/fig-stellar-fates` (mass → outcome chart + supernova type comparison). (implemented)
- `chapters/8.5-compact-objects.qmd`: `ch08/fig-compact-objects` (mass-radius + WD/NS structure + pulsar beam inset). (implemented)

### Book IX: The Galactic Realm (ch09)

- `chapters/9.3-the-milky-way.qmd`: `ch09/fig-milky-way-structure` (face-on + edge-on schematic; optional rotation curve inset). (implemented)
- `chapters/9.4-galaxy-interactions.qmd`: `ch09/fig-galaxy-merger-sequence` (interaction stages / tidal tails / ram-pressure stripping as panels). (implemented)

### Book X: The Cosmic Realm (ch10)

- `chapters/10.1-large-scale-structure.qmd`: `ch10/fig-cosmic-web-survey-slice` (survey slice scatter + void/filament annotations; complements `ch09/fig-cosmic-web` already used earlier). (implemented)
- `chapters/10.2-dark-matter.qmd`: `ch10/fig-dark-matter-evidence` (rotation curve + lensing/Bullet Cluster schematic as panels). (implemented)
- `chapters/10.3-dark-energy.qmd`: `ch10/fig-dark-energy-evidence` (Hubble diagram + `α^57` suppression vs observed Λ/Λ_P plot). (implemented)
- `chapters/10.4-cosmological-epochs.qmd`: `ch10/fig-cosmic-timeline` (log-time timeline replacing ASCII; optional bubble nucleation panel). (implemented)

### Book XI: Extreme Phenomena (ch11)

- `chapters/11.2-gravitational-waves.qmd`: `ch11/fig-gw-overview` (polarizations + chirp waveform + LIGO schematic). (implemented)
- `chapters/11.3-cosmic-rays.qmd`: `ch11/fig-cosmic-ray-spectrum` (log spectrum with knee/ankle/GZK + air-shower inset). (implemented)
- `chapters/11.4-vacuum-fluctuations.qmd`: `ch11/fig-vacuum-fluctuations` (virtual pair lifetime + Casimir plates + Hawking pair panel). (implemented)

### Book XII: Emergent Phenomena (ch12)

- `chapters/12.1-self-organization.qmd`: `ch12/fig-self-organization-examples` (Game of Life / reaction-diffusion / sandpile or edge-of-chaos phase diagram). (implemented)
- `chapters/12.3-complexity.qmd`: `ch12/fig-complexity-curve` (complexity vs time schematic + hierarchy ladder). (implemented)
- `chapters/12.4-the-anthropic-window.qmd`: `ch12/fig-anthropic-map` (α vs KB parameter sweep heatmap; placeholder schematic until simulation sweep exists). (implemented)

### Book XIII: The End (ch13)

- `chapters/13.1-heat-death.qmd`: `ch13/fig-entropy-timeline` (eras + entropy/temperature trend). (implemented)
- `chapters/13.2-alternative-endings.qmd`: `ch13/fig-cosmic-fates` (branching fate diagram keyed by w and stability; replace multiple ASCII blocks). (implemented)
- `chapters/13.3-return-to-void.qmd`: `ch13/fig-return-cycle` (the “Void → Manifestation → Structure → Return” loop; unify with earlier cycle figure by cross-referencing or reusing). (implemented)

### Book XIV: Appendices (ch14)

- `chapters/14.1-constants-reference.qmd`: `ch14/fig-constants-graph` (dependency graph from integers → constants → masses). (implemented)
- `chapters/14.4-particle-catalog.qmd`: `ch14/fig-mass-spectrum` (log-scale mass spectrum plot + charge/color legend). (implemented)
- `chapters/14.5-assumption-ledger.qmd`: `ch14/fig-epistemic-ledger-map` (core vs selection vs companion vs conjecture diagram). (implemented)
- `chapters/14.6-self-consistency.qmd`: `ch14/fig-self-reference-loop` (clean diagram replacing ASCII loop) + optional `ch14/fig-fibonacci-skeleton`. (implemented)
- `chapters/14.2-equations-reference.qmd`: no figure planned (pure reference).
- `chapters/14.3-glossary.qmd`: no figure planned (pure reference).

## Tier 2 (Optional Enhancements)

- `chapters/2.4-quantum-phenomena.qmd`: `ch02/fig-quantum-phenomena-panels` (tunneling / entanglement / sLoop panels). (implemented)
- `chapters/3.2-the-periodic-table.qmd`: `ch03/fig-periodic-table-overview` (block structure overview beyond H/He). (implemented)
- `chapters/8.2-main-sequence.qmd`: `ch08/fig-mass-luminosity-relation` (log–log L vs M plot). (implemented)
- `chapters/9.1-galaxy-formation.qmd` + `chapters/9.2-galaxy-types.qmd`: replace remaining ASCII diagrams with generated figures (`ch09/fig-galaxy-formation-pathways`, `ch09/fig-morphology-density-relation`). (implemented)
