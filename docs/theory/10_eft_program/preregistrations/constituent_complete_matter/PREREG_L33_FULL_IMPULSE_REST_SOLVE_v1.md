# FTD-0708 — L=33 full-impulse rest solve v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**Parent:** FTD-0707

## Question

Does the existing 48-coordinate constituent state contain a nearby exact
`L=33` common-action rest fixed point that the four-coordinate symmetry
refinement of FTD-0707 could not resolve?

## Frozen residual and solve

Start from the unchanged FTD-0707 accepted state (identical to the FTD-0706
rest preparation because FTD-0707 accepted no step). For each trial:

1. displace all 48 constituent position coordinates;
2. keep charges, graph, rest lengths, coat, volume, and zero momentum fixed;
3. rebuild the minimum-energy longitudinal matched field with fibre cap 8;
4. execute one unchanged complete common-action tick;
5. use the 48 components of `total_impulses` as the root residual.

Compute the full `48x48` centered-difference Jacobian with `h=2e-5`. Use
pivoted Gaussian elimination on `J delta=-R`, at most six Newton iterations,
and backtracking scales `1,1/2,...,1/1024`. Accept only a trial with strictly
smaller impulse infinity norm. Reject a trial if any constituent changes its
nearest-site anchor or if cumulative coordinate displacement exceeds `0.05`.
No damping is applied to dynamics; backtracking selects a static initial state
before qualification.

## Frozen qualification

Require:

- impulse infinity norm `<=1e-9`;
- one-tick complete-state change and total momentum each `<=1e-9`;
- common residual `<=1e-10` and energy drift `<=1e-10` in every solve;
- eight forward ticks with maximum complete-state excursion `<=1e-8`, center
  displacement `<=1e-10`, and no site hops;
- eight state-only reverse ticks with recovery `<=1e-9`;
- shift-`(3,0,0)` state/first-tick covariance `<=1e-9`.

Verdicts:

- `L33_FULL_IMPULSE_REST_FIXED_POINT_CONSTRUCTIVE` if every gate passes;
- `L33_EXISTING_STATE_HAS_NO_NEARBY_FULL_IMPULSE_REST_ROOT` if all algebra,
  field, action, and covariance evaluations are valid but the root or fixed-
  point gates fail;
- `L33_FULL_IMPULSE_REST_SOLVE_EXECUTION_INVALID` for failed provenance,
  Jacobian/linear algebra, redressing, action, inverse, or covariance
  evaluation.

The negative verdict closes only the `0.05` neighborhood of this selected
state under the locked solver. It does not force a new primitive. A
formation-from-vacuum or broader configuration-space construction remains a
separate candidate.

