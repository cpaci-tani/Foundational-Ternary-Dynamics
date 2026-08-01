# FTD-0632 — Connected-block cubic eight-fibre v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Parent:** FTD-0631 verdict
`CONNECTED_BLOCK_FULL_HALF_STATIC_REFINEMENT_CLOSED_NEGATIVE`  
**Scope:** derive and test the minimum nearest-site chart multiplicity required
to represent the fully-half connected-block Hessian neighbourhood  
**Date:** 2026-07-27

## 1. Question

FTD-0631 could evaluate the central fully-half state and its gradient but could
not form the registered four-coordinate Hessian under the inherited maximum
shared-anchor multiplicity of two. Is that failure caused by a finite,
geometrically determined chart fibre, or does the candidate require
unbounded/ad-hoc occupancy?

## 2. Candidate derivation

`anchor + remainder` is the effective constituent position. At a nearest-site
chart boundary, two distinct subcell signs can map to the same anchor in each
Cartesian coordinate. For a three-dimensional cubic chart, the tensor-product
upper bound for distinct orthant representatives at one anchor is therefore

`N_fibre <= 2^D = 8`.

This is a chart bound for the locked connected-block family, not a new gauge
group, generation count, particle count, or universal occupancy theorem.
Fibre records remain distinguished by effective position, polarity, momentum,
and graph incidence. No energy, current, or force may depend on the fibre
index or on anchor multiplicity itself.

## 3. Frozen stencil

Use the FTD-0631 x- and cyclic y-oriented starting geometries at `L=17` and
the exact registered four-coordinate Hessian stencil:

- central point;
- `+/-2e-4` along each of four coordinates;
- all four sign combinations for each of the six coordinate pairs.

This gives 33 geometries per orientation and 66 total. No optimizer, alternate
step, random sample, or enlarged shape box is allowed.

For every geometry, compute the exact anchor multiplicity and independently
attempt longitudinal Gauss redressing with locked fibre caps `2`, `4`, and
`8`.

## 4. Gates and verdicts

Common gates:

- 66/66 geometries are enumerated with the inherited 16 constituents and
  72-edge graph;
- effective constituent positions are finite and pairwise distinct;
- all same-anchor effective separations are `>=0.9`;
- all cap-8 redressings pass with Gauss residual `<=1e-11`;
- x/y multiplicity histograms and validity counts agree exactly.

Discriminator:

- the observed maximum multiplicity is exactly `8`;
- at least one cap-2 and at least one cap-4 redressing fail;
- a redressing passes exactly when its cap is at least the measured
  multiplicity of that geometry.

Verdicts:

- `CUBIC_EIGHT_FIBRE_NECESSARY_AND_SUFFICIENT_FOR_LOCKED_CHART` if every gate
  and discriminator passes;
- `FINITE_FIBRE_BOUND_DIFFERENT_FROM_EIGHT` if all geometries have a finite
  sufficient bound but the observed minimum is not eight;
- `CUBIC_EIGHT_FIBRE_CLOSED_NEGATIVE` if cap eight is insufficient or
  pass/fail depends on something besides the measured chart multiplicity;
- `CONNECTED_BLOCK_CUBIC_EIGHT_FIBRE_EXECUTION_INVALID` for provenance,
  coverage, covariance, or output failure.

A constructive verdict licenses a cap-eight observer chart for this composite
family. It does not license multiple production `s` values per site, promote
the constituent list to a primitive ontology, or establish static/dynamic
stability.

## 5. Artifacts

Add a named observer redress API with an explicit finite multiplicity cap. The
existing Boolean API must preserve cap-one/cap-two behavior exactly. Produce
one CTest, JSON and CSV records, an independent combinatorial certificate,
analysis/audit, and synchronized canonical records. Production remains
unchanged.
