# FTD-0651 — Repeated exact-root acceleration v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Parent:** FTD-0649; execution successor to FTD-0650 v1  
**Scope:** observer-only nonlinear-solver qualification; no physical change

## Question

Can the existing central-difference Jacobian plus repeated-root Broyden cache
solve the unchanged connected-block common-action residual with the same
accepted states as the qualified Jacobian-free Newton--Krylov path, while
requiring fewer exact residual evaluations over repeated histories?

## Frozen dynamics

Every FTD-0649 physical definition remains fixed: cell factors, dispersion,
polarity deposit, face current, edge magnetic field, binding graph, action,
field-energy coefficient, chart fibre, root tolerance, action gates, and
forward/reverse maps. Solver choice is instrumentation only.

The candidate solver is the already implemented dense central-difference
Jacobian path with one independent `ConnectedMooreBlockSolveCache` for the
forward direction and one for reverse. Accepted states must still satisfy the
exact residual. A stale cached Jacobian may be discarded only by the existing
deterministic fallback, which rebuilds the same central-difference Jacobian.
No physical coefficient or tolerance may depend on performance.

## Locked arms

Use widths `w={2,3,4}`, `a=2/w`, and the FTD-0649 scale factors. At each width
compare cached and matrix-free one-step forward/reverse results for four
launches:

1. `v=0.01` in `<100>`;
2. `v=0.04` in `<100>`;
3. `v=0.04` in `<110>`;
4. `v=0.04` in `<111>`.

This gives 12 paired one-step arms. In addition, at width two run three forward
ticks and three from-state reverse ticks for the four launches under both
solvers. Caches persist only within their assigned history and time direction.

## Locked measurements and gates

Record exact-residual evaluations, iterations, Jacobian refreshes/reuses,
Krylov matvecs, accepted-state differences, action residuals, and state-only
recovery.

The candidate passes only if:

- every cached and matrix-free root converges;
- every exact action residual is at most `1e-9`;
- paired one-step accepted states differ by at most `1e-8`;
- paired repeated states differ by at most `1e-8` after every tick;
- both three-tick reverse histories recover their initial state within `1e-8`;
- all cached histories contain at least one Jacobian reuse after their first
  refresh;
- over the repeated histories, the cached path uses fewer exact residual
  evaluations than the matrix-free path.

Wall time is recorded but is not an acceptance gate. No arm may be dropped
after execution.

## Verdicts

- `REPEATED_EXACT_ROOT_ACCELERATION_CONSTRUCTIVE` if all gates pass;
- `REPEATED_EXACT_ROOT_ACCELERATION_EQUIVALENT_BUT_NOT_CHEAPER` if exact-state
  gates pass but residual-evaluation cost does not decrease;
- `REPEATED_EXACT_ROOT_ACCELERATION_CLOSED` if any convergence, action,
  equivalence, cache-reuse, or inverse gate fails;
- `REPEATED_EXACT_ROOT_ACCELERATION_EXECUTION_INVALID` if coverage or records
  are incomplete.

A constructive result licenses FTD-0652 to rerun the unchanged FTD-0650
physical campaign using the qualified solver and per-arm atomic checkpoints.
It does not itself establish mobile matter or change production dynamics.
