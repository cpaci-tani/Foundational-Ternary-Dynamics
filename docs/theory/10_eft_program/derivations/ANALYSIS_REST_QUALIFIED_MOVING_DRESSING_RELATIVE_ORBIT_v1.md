# FTD-0709 — Rest-qualified moving-dressing relative orbit v1

**Status:** `[SELECTED DYNAMICS — STATIC BOOST CLOSED NEGATIVE]`  
**Verdict:** `REST_QUALIFIED_CORE_TRANSLATES_WITHOUT_COMPLETE_MOVING_DRESSING`  
**Production status:** unchanged

The FTD-0708 state reconstructs and passes the locked two-tick rest control at
`5.57e-11`. After every constituent is assigned `v=1/2`, two complete ticks
produce 12 legitimate hops and reverse within `1.14e-11`; maximum energy drift
is `8.64e-12`, common residual `1.96e-11`, and translation covariance
`1.91e-13`.

The relative-orbit residuals are:

| component | `F^2(X)-T_1X` maximum |
|---|---:|
| constituent position | `8.56089e-4` |
| constituent momentum | `3.52459e-3` |
| electric face field | `2.98075e-1` |
| magnetic edge field | `1.44323e-1` |
| complete state | `2.98075e-1` |

The rest-qualified constituent core approximately translates, but the complete
field dressing does not. Therefore instantaneous momentum assignment to a
static dressed state is not a uniformly moving object under the selected
action. The result does not show that no moving dressing exists. It selects a
co-moving field shooting equation or causal formation history as the next
candidate, before any radiation or wake classification.

Record: protocol `14AE617C...7A74E`; JSON `86562D8A...0FF7C`; runner
`03387420...C7717`; proof `67DFAEEE...DA564`.

