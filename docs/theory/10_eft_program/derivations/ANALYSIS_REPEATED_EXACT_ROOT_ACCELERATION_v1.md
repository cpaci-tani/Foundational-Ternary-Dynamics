# Repeated exact-root acceleration

**Campaign:** FTD-0651  
**Status:** `[OBSERVER SOLVER QUALIFICATION — CONSTRUCTIVE AT LOCKED SCOPE]`  
**Verdict:** `REPEATED_EXACT_ROOT_ACCELERATION_CONSTRUCTIVE`  
**Production impact:** none

## Result

The existing central-difference Jacobian with deterministic repeated-root
Broyden updates was compared directly with the FTD-0649 Jacobian-free
Newton--Krylov solver. All 12 locked arms converged and inverted.

| diagnostic | result | gate |
|---|---:|---:|
| worst complete-state difference | `2.16538e-11` | `1e-8` |
| worst exact action residual | `1.36431e-11` | `1e-9` |
| worst state-only recovery | `1.55467e-11` | `1e-8` |
| width-two repeated matrix-free evaluations | `955` | reference |
| width-two repeated cached evaluations | `884` | `<955` |

Every cached history refreshed its central-difference Jacobian and then reused
it. Accepted states were always rechecked against the unchanged exact action.
No physical coefficient, tolerance, field, constituent variable, or update
map changed.

## Performance boundary

The constructive verdict is deliberately amortized and narrow. At width two,
three forward plus three reverse ticks used fewer exact residual evaluations
under caching. A one-step dense initialization is substantially more expensive
at larger width:

| width | matrix-free seconds per paired arm | cached seconds per paired arm |
|---:|---:|---:|
| 2, six solves | `1.52–1.98` | `1.59–1.65` |
| 3, two solves | `10.02–10.58` | `70.47–71.39` |
| 4, two solves | `49.36–50.78` | `764.06–764.73` |

The larger-width cost is the two initial dense Jacobian builds, one for each
time direction. A long history may amortize them because subsequent roots use
secant reuse, but FTD-0651 did not measure that crossover. Wall time was not a
registered acceptance gate. Therefore this result does not justify calling
the cached method universally faster.

## Consequence

FTD-0651 licenses one v2 attempt at the unchanged FTD-0650 physical campaign,
using independent forward/reverse caches and per-arm atomic checkpoints. That
attempt must record the actual evaluation and wall-time crossover. This solver
qualification is not evidence for mobile matter, a pole, charge, Lorentz
recovery, or production adoption.
