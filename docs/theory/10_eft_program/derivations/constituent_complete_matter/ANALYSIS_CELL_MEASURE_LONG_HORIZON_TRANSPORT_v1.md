# Cell-measure long-horizon transport v1

**Campaign:** FTD-0650  
**Status:** `[EXECUTION INVALID — NO PHYSICAL VERDICT]`  
**Verdict:** `CELL_MEASURE_LONG_HORIZON_EXECUTION_INVALID`  
**Production impact:** none

## Result

The locked v1 runner compiled and began the 30-history campaign. Its first
six-history width-two batch completed after approximately 71 minutes and the
second batch began. The runner wrote results only after all five batches, so
no complete per-arm checkpoint survived termination. At 72.3 minutes the
process had accumulated `14750.34375` CPU-seconds and reached a measured peak
working set of approximately `2.39 GB`.

The remaining batches include the larger width-three and width-four systems.
The observed first-batch cost projected beyond the registered six-hour CTest
window. The run was terminated rather than silently changing the locked
Jacobian-free Newton--Krylov solver, arm matrix, horizon, or tolerances.

## Correct interpretation

This is not a negative result for cell-measure matter. Coverage and record
completeness prevent evaluation of every registered action, coherence,
mobility, resolution, covariance, and inverse gate. The preregistered
execution-invalid rule therefore controls.

What failed is the v1 numerical instrument:

1. every tick rebuilds a fresh matrix-free Krylov space for the same slowly
   varying exact residual;
2. each history solves both forward and from-state reverse evolution;
3. records are committed only after the entire campaign rather than after
   each completed arm.

No equation of motion failed, and no long-horizon physical observable is
qualified from the partial run.

## Consequence

The next candidate must qualify an exact-root repeated-solve acceleration
against the established matrix-free roots before the unchanged physical arm
matrix is rerun. This is an implementation successor, not permission to alter
cell factors, interactions, tolerances, or verdict gates. Per-arm atomic
checkpointing is also required so a later execution interruption preserves
completed evidence without changing dynamics.
