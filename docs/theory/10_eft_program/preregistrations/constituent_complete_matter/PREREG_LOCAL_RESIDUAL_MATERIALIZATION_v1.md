# FTD-0692 — Local residual / accepted-state materialization v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production impact:** none  
**Question:** can the connected common-action root be evaluated from the
registered local current/gather data and materialize the complete lattice
state only once, without changing the accepted transaction?

## Frozen comparison

Use the existing connected width-two neutral block at `L=17`, orientation
axis zero, dressing displacement `0.125`, and the deterministic launch
velocity `(0.02,-0.01,0.015)`. Compare one forward and one from-state reverse
transaction under:

1. the established full-field residual evaluator; and
2. `use_local_residual_evaluation=true` with the same central-difference root,
   tolerances, equations, sparse current, and physical options.

The local evaluator may retain only constituent endpoints, deposited sparse
face currents, gathers, impulses, and the `3N` residual during nonlinear
probes. After convergence it must construct the complete candidate using the
established evaluator and replace the probe result before `finalize()`.

## Acceptance gates

- both forward and reverse transactions are valid and pass every established
  common-action gate;
- the full and local routes use the same number of residual evaluations and
  nonlinear iterations;
- exactly one full candidate is materialized per accepted local solve;
- the local-versus-materialized residual difference is at most `1e-14`;
- complete forward-state difference is at most `1e-10`;
- complete reverse-state difference is at most `1e-10`;
- state-only recovery is at most `1e-9` in both routes; and
- no production default or interaction coefficient changes.

Failure is an engineering negative for the local evaluator, not a matter-
dynamics verdict. Success licenses a separately locked large-volume timing
and causal-separation rerun; it does not itself establish dressing, radiation,
a particle pole, or an ontological primitive.

