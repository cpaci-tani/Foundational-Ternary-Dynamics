# FTD-0760 — M3 relational-chart held-out validation pre-execution audit v1

**Status:** `[PRE-EXECUTION LOCK — QUALIFICATION AND REGISTERED MODES NOT RUN]`  
**Date:** 2026-07-30  
**Protocol:** `PREREG_M3_RELATIONAL_CHART_HELD_OUT_VALIDATION_v1.md`

## Lock statement

At this audit point `engine/results/ftd_0760/` does not exist. No FTD-0760
qualification, candidate, or causal-fibre mode has run. The protocol's first
draft repeated the already-inspected FTD-0758 remote plaquette. That defect was
caught before implementation or execution; the final lock instead freezes the
previously unrun amplitude `1/1024` and displacement 104. The superseded draft
never licensed a run.

The final frozen files are:

| artifact | SHA-256 |
|---|---|
| protocol | `681FA36CCE4479D268D37651E4CD58AA6C1D5A4809F989EA4FF2AA24B7B40722` |
| CUDA-linked runner source | `374DE8AE67DCC69D40FF1AF8B74300497728FFE825D393429B5BF3D54B453BA5` |
| independent certificate | `E8704CFB9CAEEED68AC604E448A936572FF4359BD536FAED257417AF10F35E4C` |
| matter-predicate header | `B11E087E2E7E16375C173185233AD001AB8B9F049E9B9B5A3156D8618CB4F104` |
| matter-predicate source | `752CE7C3B03A9944C1E7016A62CCA584FAC868EF191D8241ACEE7E6C9C550D21` |
| WSL2 executable | `4D42344CDA34E4146D1946222C8940B2E5E2BC6841633DA92B80EBA924150F93` |

The independent preflight passes `17/17` checks, including absence of the
result directory and exact hashes for the protocol, inherited protocol,
predicate, and runner.

## Locked distinction

The candidate CSV records three separate facts:

```text
site_projection_valid
allow_shared_anchor_chart
chart_admissible = allow_shared_anchor_chart || site_projection_valid
```

The certificate reconstructs this disjunction row by row. It cannot repeat
FTD-0758's unconditional unique-anchor conjunct.

Qualification may now run exactly once as a non-evidential interface check.
If it passes, the six registered modes may run exactly once each. No failed or
interrupted mode may be tuned or repeated under FTD-0760.
