# AUDIT — Recovery-reservoir donor discriminator v1

**Date:** 2026-07-28  
**Identifier:** `FTD-0674`  
**Status:** `[EXECUTION INVALID — NO LOCKED RECOVERY]`  
**Verdict:** `RECOVERY_RESERVOIR_DONOR_EXECUTION_INVALID`

## Result

The campaign does not identify an energy donor. The preregistered tick-72 to
tick-78 target change is negative (`~-0.02518116`) for both signs, rather than
the required recovery of at least `+0.05`. Its normalized interval closure
also misses the locked `1e-8` threshold. The printed provisional donor class
is void under the protocol.

The invalid result is scientifically consequential because the corrected
canonical mode energy removes the old recovery pattern. FTD-0675 proves that
the old paired-mode observer omitted the mass metric and can manufacture
trough/recovery sequences even for a constant-energy harmonic mode.

## Valid execution facts

```text
maximum constituent momentum      9.9999999999999995e-7
initial canonical target energy   1.1194521889876922e-11
maximum observer residual         2.8132771982229655e-12 normalized
state-only inverse recovery       5.5422333389287814e-13
polarity target-change mismatch   1.992929044e-9
production changed                false
```

These facts do not override the locked invalid verdict.

The independent certificate verifies the protocol, runner, JSON, and CSV
hashes; all 162 sign/tick rows; per-tick observer validity; both failed locked
gates; and polarity agreement. Certificate SHA256:
`C66F60018BAAEA87D14D1194DBE78774414AB4A07672FC756B58788C2270AE0F`.

## Required next gate

Replay the earlier modal campaigns with canonical coordinates and express
future recovery windows in intrinsic mode phase or event-defined extrema fixed
from an independent training amplitude. Absolute tick transfer across a
changed diagnostic is not admissible.
