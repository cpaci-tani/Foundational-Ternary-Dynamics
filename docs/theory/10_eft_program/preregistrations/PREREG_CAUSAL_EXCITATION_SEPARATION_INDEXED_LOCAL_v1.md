# FTD-0694 — Indexed-local causal excitation separation v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production impact:** none

## Question

Can FTD-0693's unchanged local-residual equations complete the `L=113`
causal-separation discriminator when the sparse midpoint field is indexed once
per nonlinear probe rather than searched repeatedly for every orbit sample?

## Frozen design

Inherit every physical setting, observer, tolerance, output field, and verdict
gate from FTD-0693. The sole licensed change is an exact lookup representation:

- for each root probe, construct a map from oriented face index to the ordered
  list of deposited-current contributions touching that face;
- retain the original constituent/segment deposition order in every list;
- reconstruct the local midpoint coefficient by starting from the fixed
  pre-current coefficient and applying those contributions sequentially;
- reuse that map for all 16 constituent gathers; and
- materialize and recheck the accepted complete state exactly once.

The FTD-0692 equivalence gate must remain unchanged: zero forward-state
difference at the locked arm, reverse difference below `1e-10`, identical root
iterations/evaluations, and local-to-materialized residual difference at most
`1e-14`.

## Verdict logic

The causal campaign retains FTD-0693's complete-run, physical-negative, and
execution-invalid branches verbatim. A runtime improvement is engineering
evidence only. The physical verdict comes solely from the previously locked
pre-contact spatial and late-time discriminators.

