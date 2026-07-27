# FTD-0562 Audit — Full-Surface Finite-Source Obstruction

**Status:** [THEOREM — FIXED FINITE RIGID LINEAR SOURCE CLOSED NEGATIVE; NONLINEAR/DEFORMING CARRIER OPEN]
**Date:** 2026-07-26
**Production changes:** none

## Verdict

The neutral finite-profile escape left by FTD-0561 is closed in the frozen
linear rigid-source sector.  Every nonzero finite profile has a first nonzero
three-dimensional homogeneous moment.  The production operator supplies a
regular `l=1` slow-hop resonance in every direction, and at least one direction
has a nonzero moment polynomial and hop-axis component.  Its native gradient
source is therefore nonzero for all sufficiently large hop periods.

The result is an analytic source/dressing obstruction.  It is not a physical
radiation-power law and does not close nonlinear or deforming matter.

## Locked campaign

The preregistration was locked before observer implementation at SHA-256
`D9F9B23232AB1A67A1829090C216207BAF58873E3EC9CE75CC809E395E0531D5`.

| Diagnostic | Locked gate | Result |
|---|---:|---:|
| arms | 768 | 768 |
| witness groups | 96 | 96 |
| maximum denominator residual | `1e-12` | `2.855246421240576e-16` |
| minimum `T^2|dD/dr|` | `>1` | `7.224481330653771` |
| maximum polarity-mirror residual | `1e-12` | `0` |
| maximum cyclic-covariance residual | `1e-12` | `3.481659405224491e-13` |
| `T=512` radius-correction residual | `<0.25` | `0.13254411344298234` |
| `T=512` forcing-asymptotic error | `<0.20` | `0.010924707775298748` |
| minimum scaled witness forcing | `>0` | `9.925385341507626` |

The independent Python proof reproduced the verdict without reading the C++
record.  Both global polarity mirrors and all cyclic axis rotations passed.

## Scope ledger

| Claim | Status after FTD-0562 |
|---|---|
| A fixed nonzero finite rigid source cancels every sufficiently slow `l=1` resonance | [CLOSED NEGATIVE] |
| A finite neutral multipole can suppress the source by arbitrarily high fixed algebraic order | [THEOREM — if constructed with that first moment order] |
| A fixed finite neutral profile eliminates the source exactly | [CLOSED NEGATIVE — FULL SURFACE] |
| Physical matter radiation power is derived | [OPEN — NOT PROVED] |
| A deforming, nonlinear, defect, topological, or period-growing carrier is impossible | [OPEN — NOT PROVED] |

No toggle, scenario, force, damping rule, production ordering change, or
ontological promotion is licensed.
