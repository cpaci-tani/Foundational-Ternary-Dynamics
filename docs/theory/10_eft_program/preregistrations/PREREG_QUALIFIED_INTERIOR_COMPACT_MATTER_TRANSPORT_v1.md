# FTD-0608 — Qualified-interior compact matter transport v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`
**Scope:** observer-only autonomous transport from an independently qualified
FTD-0607 compact core using the unchanged FTD-0601 common-action transaction
**Production change:** forbidden
**Protocol lock:** `protocol_sha256=B64BB90EF082EC8E47BE83BA1F9951D7B30C3C5904AE8E4C639B33543020C5E0`

## 1. Ontological question

Can a stable compact constituent/field pattern that already satisfies the
one-label-per-site capacity move from one site chart to the next under the
same atomic transaction that determines its current, field update, recoil,
energy exchange, and state-only inverse?

This protocol adds no constituent, occupancy channel, force, field component,
history variable, connection, phase, or production state.

## 2. Launch state fixed before motion

Use the unchanged FTD-0607 `L=17` neutral pair, physical energy, global
`SO(3) x Sym(2)` coordinates, minimum-energy direct field, and hard unique-
anchor domain. Select fractional phase `15/32` because the completed FTD-0607
record identifies it as the lowest-energy member of the five qualified static
cores. No motion result was available when this rule was chosen.

Repeat the same 24 proper-cubic starts, six-dimensional Nelder-Mead settings,
hard objective domain, and differential checks at phase `15/32`. The seed is
reproduced only if:

- all 24 starts are admissible, at least 18 terminate, and at least two lie
  within `1e-10` energy of the best;
- the best energy differs from the FTD-0607 record
  `0.0031781023845096961` by at most `5e-10`;
- its chart margin is at least `5e-3`, all anchors are distinct, and internal
  distances and strain satisfy the unchanged FTD-0607 gates;
- the gradient infinity norm is at most `5e-7`;
- all six tangent eigenvalues exceed `1e-6` with none below `-5e-6`;
- the direct Gauss/curl/Green-energy gate is at most `1e-11`.

The lowest-energy reproduced candidate is frozen before either motion arm.

## 3. Autonomous transport arms

Create two independent state copies. Assign every constituent the unchanged
production-dispersion momentum corresponding to collective velocities
`v=(1/64,0,0)` and `v=(1/32,0,0)`. Initialize the electric field to the
independently rebuilt minimum-energy Gauss field and magnetic half-field to
zero. This dressing is selected, not derived.

Run the unchanged FTD-0601 forward transaction for 128 and 64 ticks,
respectively, then run the unchanged state-only inverse from each final state
for the same number of ticks. No relaxation, re-minimization, force
amplification, candidate substitution, or field reset is allowed after the
first forward step.

For each arm require:

- every forward and reverse solve is valid and common-action qualified;
- continuity, Gauss, work, energy, and causal residuals remain at most
  `1e-12` per tick;
- all six anchors remain pairwise distinct;
- each trimer's three distances remain in `[0.5,2.0]`;
- trimer-centre separation changes by at most `0.25`;
- mean longitudinal displacement is at least 75% of the nominal two-cell
  ballistic displacement and transverse drift is at most `0.25`;
- at least six legitimate constituent anchor changes occur;
- cumulative total-energy drift is at most `1e-10`;
- state-only reverse recovery is at most `1e-9`.

Repeat the first forward step after translating the entire launch state by one
integer site in `x`; require translated-state covariance at `1e-12`.

## 4. Verdicts

- `QUALIFIED_INTERIOR_COMPACT_MATTER_MOBILE_CONSTRUCTIVE`: the static seed,
  both motion arms, and integer covariance pass;
- `QUALIFIED_INTERIOR_COMPACT_TRANSPORT_CLOSED_NEGATIVE`: the static seed
  reproduces, but at least one registered motion or inverse gate fails;
- `QUALIFIED_INTERIOR_STATIC_SEED_NOT_REPRODUCED`: registered static search
  coverage passes but the fixed phase-15 core fails a static gate;
- `QUALIFIED_INTERIOR_COMPACT_MATTER_NUMERICALLY_UNRESOLVED`: execution,
  solver, or record coverage is insufficient to apply an earlier verdict.

A constructive result licenses only this selected compact family and two
velocity arms. A negative result closes only this fixed phase, initial
dressing, velocity pair, and transaction. Neither result licenses a physical
particle, production toggle, scenario, electromagnetic ontology, pole,
Lorentz recovery, or unitarity claim.
