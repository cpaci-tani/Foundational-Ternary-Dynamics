# PREREGISTRATION — Causal excitation separation v4

**Identifier:** `FTD-0689`  
**Status:** `[LOCKED BEFORE IMPLEMENTATION AND EXECUTION]`  
**Date:** 2026-07-28

FTD-0687 passed initialization but emitted no tick-8 checkpoint or result file
before resource termination.  V4 inherits every FTD-0687 physical parameter,
gate, output, and classifier unchanged and substitutes only the FTD-0688
prefix-sum implementation after scalar-equivalence qualification.

Use `L=129`, ticks `0..112`, fixed origin `(64,64,64)`, both signs plus
control, `p_max=1.25e-7`, the same six radii, arrival threshold, late window,
exact gates, polarity gates, and factorized classes.  Lock wrapper, embedded
runner, observer, and binary hashes before invocation.  Output goes to
`engine/results/ftd_0689/`.
