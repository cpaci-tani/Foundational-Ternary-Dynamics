# Analysis — Collective Source-History Bound

**FTD ID:** FTD-0588  
**Date:** 2026-07-26  
**Status:** `[THEOREM + NUMERICAL FACT + MEASURED; BOUNDARY SUPERSEDED BY FTD-0589]`

## Locked result

The FTD-0586 linear `N B_L` envelope overcounted synchronized sources. For
distinct source sites, finite-group Parseval fixes the total spatial Fourier
power to `L^3 N`. Together with the newly derived matched-stencil inequality

\[
 \sum_a\sin^2k_a\le M(k),
\]

this changes the common-history envelope from order `N` to order `sqrt(N)`:

\[
 |J|\le2C_L\sqrt N
\]

for a synchronized rectangular pulse.

At the largest registered volume,

\[
 2C_{65}\sqrt5=1.4008650358896921
 <K_{\rm GENESIS}=1.5163860591519780.
\]

Thus five arbitrarily placed and signed sources cannot reach genesis if their
on/off histories are common.

For independent one-time removals, the exact decomposition into the common
initial step plus delayed single-source off-steps gives

\[
 |J|\le C_L\sqrt N+rB_L^{\rm step}.
\]

At `N=4` this is uniformly subcritical. At `N=5` it is subcritical for
`r<=4`; only the field remaining after the fifth and final source disappears
is not excluded.

## Run of record

| class | arms | genesis | evaporations | maximum closed-scope `|J|` |
|---|---:|---:|---:|---:|
| locked step, `N=4` | 16 | 0 | 0 | 0.071266172157639224 |
| synchronized pulse, `N=4` | 16 | 0 | 0 | 0.071266172157639224 |
| locked step, `N=5` | 16 | 0 | 0 | 0.071895466243716816 |
| synchronized pulse, `N=5` | 16 | 0 | 0 | 0.071895466243716816 |
| native unlocked, `N=4` | 32 | 0 | 128 | 0.071266172157639224 |
| native unlocked, `N=5` | 32 | 0 | 160 | 0.071895466243716816 |

Every unlocked arm removed all of its original sources. The registered
five-source residual tails therefore existed and still produced no genesis.
The result is not promoted to a universal negative because the exact theorem
does not cover every possible five-tick removal schedule.

FTD-0589 subsequently closes that schedule class universally by deriving the
exact finite-pulse cancellation. The FTD-0588 spatial `sqrt(N)` result remains
load-bearing; only its final sourcewise off-step envelope is superseded.

## What changed

The honest native boundary is no longer “four sources might ignite.” It is:

1. six or more sources with a common history are not excluded by this norm;
2. five or more asynchronously removed sources are not excluded;
3. at exactly five, the only open interval begins after the last original
   source vanishes.

This is a capacity theorem for a linear prescribed-source field. It is not a
particle or self-maintenance result.

The current boundary after FTD-0589 is arbitrary `N>=7`, not either item 1 or
2 above. The numbered list is retained as the historical boundary produced by
this run.

## Reproducibility

- protocol:
  `preregistrations/PREREG_COLLECTIVE_SOURCE_HISTORY_BOUND_v1.md`;
- theorem:
  `derivations/THEOREM_COLLECTIVE_SOURCE_HISTORY_BOUND.md`;
- native observer: `collective_source_history_bound` PASS;
- independent verifier: 127/127 PASS;
- run record:
  `engine/results/ftd_0588/windows_msvc_cpu.{json,csv}`.
