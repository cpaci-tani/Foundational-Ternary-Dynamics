# FTD-0665 — Volume-scaled internal-mode transfer v2

**Status:** `[SELECTED DYNAMICS — CONSTRUCTIVE PRE-RETURN FIELD TRANSFER]`  
**Verdict:** `VOLUME_SCALED_PRE_RETURN_TRANSFER_V2_CONSTRUCTIVE`  
**Production impact:** none

> **FTD-0675 correction:** this campaign's constructive pre-return dynamic-
> field generation and radial-spreading result survives. Its paired-mode
> displacement history used an unweighted rather than mass-weighted
> projection, so statements about doublet decay, exchange, or return do not.

## Result

V2 measures the tick-zero paired projection, grades direct dynamic-field
generation instead of assuming monotone doublet loss, and uses the locked
accumulated inverse bound. All six fresh histories pass.

| diagnostic | result | gate |
|---|---:|---:|
| pre-return history locality RMS | `1.952806e-4` | `<=0.05` |
| tick-16 dynamic-field energy / initial doublet | `0.111212..0.111216` | `>0` |
| tick-16 positive dynamic norm / initial doublet | `0.169215..0.169221` | `>0` |
| radial second moment, tick 4 | `4.33310..4.33312` | reference |
| radial second moment, tick 16 | `17.09335..17.11449` | increase `>=4` |
| complete-energy drift | `<=1.377e-14` | `<=1e-10` |
| common residual | `<=1.996e-11` | `<=1e-10` |
| decomposition residual | `<=3.452e-15` | `<=1e-10` |
| state-only recovery | `1.710e-10..6.243e-10` | `<=1e-8` |

The tick-16 window precedes any newly generated disturbance's possible causal
wraparound on the smallest periodic circumference. The outward dynamic field
therefore is not created by a periodic echo. The legacy paired-mode diagnostic
is nonmonotonic, but FTD-0675 prevents that fact from being interpreted as
canonical matter-mode energy exchange.

The descriptive late-return classifier is mixed. Both signs return at tick 76
for `L=25,33`; the registered `L=17` horizon ends at tick 68. FTD-0666 tests
the missing out-of-sample segment.

## Ontological consequence

The prepared internal deformation launches a real dynamic substrate
disturbance before the environment can feed anything back. Whether the
canonical matter mode later reabsorbs that energy, or whether an effectively
uncontained environment produces a durable decay envelope, remains open.
