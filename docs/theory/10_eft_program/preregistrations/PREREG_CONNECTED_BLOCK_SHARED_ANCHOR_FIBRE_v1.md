# FTD-0626 — Connected-block shared-anchor fibre v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Parents:** FTD-0609 shared-anchor fibre and FTD-0624/0625 connected-block
records  
**Scope:** apply an already priced local chart fibre before introducing a
reaction law  
**Date:** 2026-07-27

## 1. Correction and question

FTD-0624 and FTD-0625 called every repeated integer anchor an
opposite-polarity collision. Their diagnostic endpoints do not contain
coincident effective positions. A representative pair lies at approximately
`7.5007` and `8.4993`, nearly one cell apart, while both positions round to
the same integer chart anchor. The failed gate is therefore a representation
capacity gate, not by itself an annihilation event.

FTD-0609 already established a default-off multiplicity-two local chart fibre:
distinct constituent records may share an integer anchor while their compact
polarity coats and face currents remain functions of effective position. This
campaign asks:

> Does that already priced fibre make the connected block's exact half-cell
> minimum and registered circulating histories executable, reversible, and
> dynamically regular without a reaction law or another primitive?

## 2. Frozen action and sole change

Use the unchanged FTD-0622--0625 selected action:

- `L=17`, `w=2`, 16 exact `+1/-1` constituents and 72 reference-Moore bonds;
- `kappa=1`, `dt=1`, `C_SPEED=1/sqrt(3)`;
- minimum-energy initial Gauss field and zero magnetic half-field;
- production dispersion, quadratic coat, straight-segment face current,
  matched face/edge update, interaction normalization, binding action, common-
  action tolerance `1e-10`, solve tolerance `2e-11`, and 48 iterations;
- the FTD-0625 circulation amplitudes determined only from `K(A)=B_x` and
  `K(A)=4B_x`.

Add one observer option, `allow_shared_anchor_chart`, default `false`. When it
is true, accepted states may contain at most two distinct constituent records
with the same integer anchor. All equations, unknowns, current deposition,
field update, energies, impulses, graph edges, charges, and inverse equations
remain unchanged. The aggregate ternary site field remains a derived lossy
projection and is not used as the complete matter state.

No reaction, annihilation, graph rewiring, contact impulse, damping, legacy
force, fitted coefficient, tolerance change, hidden history, production
toggle, or new persistent variable is admitted.

## 3. Locked arms

Run 16 forward steps followed by 16 state-only reverse steps for:

1. exact half-cell zero-momentum rest, base `x` orientation/phase;
2. its cyclic `y` orientation/phase copy;
3. the FTD-0625 near-half zero-circulation control;
4. near-half `+A_1` and `-A_1` circulation;
5. near-half `+A_4` and `-A_4` circulation;
6. cyclic copies of `+A_1` and `+A_4`.

Total: nine arms and 288 registered transactions. No failed arm may be
replaced. In addition, run the unchanged default-false exact-half and
circulation cases and require the FTD-0624/0625 failure class to reproduce.

## 4. Fibre and exactness observers

At every accepted endpoint record:

- anchor multiplicity and the identities/charges of every shared pair;
- minimum effective-position separation within each shared anchor;
- centre, total matter momentum, internal angular momentum, shape RMS,
  maximum bond strain, and half-cell phase distance;
- constituent count, charge list, and graph fingerprint;
- matter, binding, field, and total energy;
- continuity, Gauss, work, energy, causal-speed, root, and state-only inverse
  residuals;
- local and spline translation-reaction defects.

Every shared pair must have multiplicity exactly two, distinct effective
positions separated by at least `1e-3`, and a deterministic constituent order.
The maximum anchor multiplicity may never exceed two.

## 5. Gates

Every fibre-enabled forward and reverse step must:

- converge and pass the unchanged common-action gates at `1e-10`;
- preserve constituent count, charge order, graph connectivity/locality, and
  graph fingerprint exactly;
- preserve total energy to `1e-9` over each history;
- recover the complete constituent-plus-field state to `1e-8` after reversal;
- remain causal and finite;
- satisfy the fibre regularity conditions in section 4.

The two exact-half rest arms additionally require centre displacement,
centre momentum, shape RMS, and maximum edge strain at most `1e-8` after every
tick. At least one accepted endpoint must exercise multiplicity two; otherwise
the extension has not tested the parent obstruction.

Signed circulation partners must agree in scalar histories and reverse
internal angular momentum within `1e-8`. Cyclic copies must rotate vector
histories and agree in scalar histories within `1e-8`.

The default-false controls must still reject the first repeated-anchor
endpoint, with a converged root, unchanged graph, zero same-polarity conflicts,
and only opposite-polarity repeated anchors. This is a regression gate, not
evidence that the fibre-enabled branch is a reaction.

## 6. Verdicts

- `CONNECTED_BLOCK_FIBRE_REST_AND_MOTION_CONSTRUCTIVE`: both exact-half rest
  arms and all registered circulation arms pass every exactness, fibre,
  symmetry, covariance, and inverse gate.
- `CONNECTED_BLOCK_FIBRE_REST_CONSTRUCTIVE_MOTION_OPEN`: both exact-half rest
  arms pass, but at least one circulation family fails a dynamical gate after
  all initialization and record-integrity gates pass.
- `CONNECTED_BLOCK_FIBRE_CLOSED_NEGATIVE`: the fibre is exercised and the
  campaign is executable, but an exact-half rest arm fails exactness,
  reversibility, fibre regularity, or stationarity.
- `CONNECTED_BLOCK_FIBRE_EXECUTION_INVALID`: parent regression, initialization,
  amplitude normalization, required coverage, or record integrity fails.

A constructive rest verdict establishes a stable selected connected pattern
only under the FTD-0609 chart fibre and the selected binding/common action. It
does not derive physical matter, annihilation, photons, spin/statistics,
charge conservation, a production state type, a pole, Lorentz recovery, or
unitarity.

## 7. Consequence rule

If the fibre rest is constructive, withdraw the inference that FTD-0624/0625
identified a physical reaction surface. Retain their narrower result: the
independent one-record-per-anchor projection is insufficient. Proceed to a
long periodic/Floquet and depinning campaign on the fibre-enabled state.

Only if the fibre campaign closes negative for a physical reason rather than
mere implementation failure may FTD-0627 preregister an atomic
opposite-polarity reaction transaction. That later transaction must compare a
count-preserving constrained scattering branch against count-changing field
emission; neither branch may be selected post hoc from its result.
