# FTD-0704 — Connected dressed-matter high-speed preflight v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**State source:** FTD-0638 orientation-0 refined connected-bipole state  
**Dynamics:** FTD-0622 complete connected-Moore-block common action with the
FTD-0692 exact local-residual representation

## 1. Question

Can the selected 16-constituent dressed matter candidate sustain a short,
coherent, reversible axial history on both sides of the FTD-0700 radiation
threshold before a time-resolved radiation discriminator is attempted?

This campaign does not measure radiation. It is an execution and source-quality
gate for that later campaign.

## 2. Frozen initialization

- remap the FTD-0638 orientation-0 refined geometry to a periodic `L=33`
  lattice, centred at `(16,16,16)`;
- redress the remapped geometry with the established minimum-energy matched
  face field, fibre limit `8`, tolerance `1e-13`, and at most `4096` Poisson
  iterations;
- retain all 16 polarities, Moore binding edges, rest lengths, and the complete
  matched face/edge field;
- assign every constituent the production momentum
  `production_flat_momentum(sign * v * e_x)` for
  `v={0.35,0.45,0.50}` and `sign={-1,+1}`;
- execute exactly eight forward ticks and eight reverse ticks per arm;
- use `allow_shared_anchor_chart`, `use_sparse_local_current`, and
  `use_local_residual_evaluation`; these are exact observer/research
  representations of the already qualified transaction.

No legacy force, Poisson-Coulomb force, post-hoc recoil, damping, reaction,
collision, pair-production, or production-tick branch is used.

At the selected lattice wave speed `c=1/sqrt(3)`, `v=0.35` lies below the exact
FTD-0700 axial threshold `0.3918265520...`; `v=0.45` and `v=0.50` lie above it.

## 3. Frozen observables

For every forward tick record:

- complete common-action residual and total-energy drift;
- centre displacement and per-tick axial increment;
- mean constituent production velocity and total matter momentum;
- site hops, maximum chart multiplicity, minimum same-anchor separation;
- RMS shape change after removing centre translation;
- maximum binding-edge strain.

For each arm record the complete forward/reverse state distance. Compare the
positive and negative arms after reflecting centre displacement and momentum.

## 4. Locked gates

Execution is valid only if every arm:

- initializes all 16 constituents with net polarity zero and a valid redressed
  matched field;
- completes all eight forward and eight reverse ticks with every common-action
  gate true and common residual `<=1e-10`;
- keeps total-energy drift `<=1e-10`;
- keeps maximum chart multiplicity `<=8`, any finite same-anchor separation
  `>=0.9`, RMS shape change `<=0.05`, and edge strain `<=0.05`;
- has no transverse centre displacement larger than `1e-8`;
- recovers the complete initial state within `1e-9` after reversal.

The source-quality gate additionally requires:

- at least 16 total site hops in each arm;
- positive displacement in the assigned direction on every tick;
- mean axial speed within `0.05` of the assigned speed;
- coefficient of variation of the eight axial increments `<=0.15`;
- positive/negative reflected histories agree within `1e-6` for displacement,
  shape, strain, field energy, and momentum.

No tolerance or speed may be changed after execution.

## 5. Verdicts

- `DRESSED_MATTER_HIGH_SPEED_PREFLIGHT_CONSTRUCTIVE`: every execution and
  source-quality gate passes;
- `DRESSED_MATTER_HIGH_SPEED_SOURCE_UNSTEADY`: the complete reversible
  transaction executes coherently, but a speed, increment, hop, or mirror gate
  fails;
- `DRESSED_MATTER_HIGH_SPEED_COHERENCE_CLOSED`: complete execution succeeds but
  shape, strain, fibre, transverse, energy, or inverse gates fail;
- `DRESSED_MATTER_HIGH_SPEED_EXECUTION_INVALID`: initialization, solver, or
  algebraic common-action execution fails.

A constructive result licenses a separately preregistered time-resolved field
test. It does not establish a particle, radiation, a photon, a wake, an aura,
or an infrared matter pole.
