# PREREGISTRATION — Component-aware radial field profile v1

**Identifier:** `FTD-0683`  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Date locked:** 2026-07-28  
**Production changes:** forbidden; observer only

## Purpose

Define the radial morphology observer needed to distinguish a localized
excitation-associated field component from an outward packet.  The observer
must use the actual geometric locations of face and edge components.  It may
not assign every component to the integer storage-cell origin or move its
origin with a fitted field centroid.

## Frozen definition

For reference and candidate matched fields on the same odd periodic volume
`L`, use a fixed integer origin `o`.  For every component form the positive
control-relative weight

```text
dH(E_a) = (beta/2) (delta E_a)^2,
dH(B_a) = (beta c^2/2) (delta B_a)^2.
```

Use the component locations

```text
E_x: (x+1/2,y,z)       B_x: (x,y+1/2,z+1/2)
E_y: (x,y+1/2,z)       B_y: (x+1/2,y,z+1/2)
E_z: (x,y,z+1/2)       B_z: (x+1/2,y+1/2,z).
```

Let `rho2` be twice the periodic Chebyshev distance from the component
location to `o`.  Because every coordinate is integer or half-integer,
`rho2` is an exact nonnegative integer.  Accumulate one bin `H[rho2]` for
every allowed `rho2=0..L` and its cumulative profile
`C[k]=sum_{rho2<=k} H[rho2]`.

Report total norm, mean radius, RMS radius, and the minimal doubled radii
containing 50%, 90%, and 99% of the total.  If the total is exactly zero, set
all quantile radii to zero and mark `zero_profile=true`; this is a valid zero
observation.

## Locked exact gates

1. Input volumes and every component extent match; `L` is positive and odd;
   `beta>0`, `c>0`, and all values are finite.
2. Every bin is nonnegative.
3. The sum of bins equals the direct six-component norm within `1e-12`.
4. The cumulative profile is monotone and its last value equals the total
   within `1e-12`.
5. Quantiles are the minimal bins satisfying their thresholds.
6. A simultaneous integer translation of both fields and the origin preserves
   every scalar output within `2e-12`.
7. Proper cyclic cubic rotations, including component locations and component
   labels, preserve every scalar output and bin within `2e-12`.
8. Invalid sizes, a noninteger origin, nonfinite values, even `L`, and
   nonpositive scales fail closed.

An exact-rational certificate will additionally check all 48 signed cubic maps
at the abstract component-position level.  The C++ test will use an asymmetric
integer origin and isolated face/edge witnesses so an x/z storage reversal or
component-location collapse cannot pass.

## Interpretation boundary

This profile is a positive morphology norm, not the complete incremental field
energy in a dressed background.  It contains no sign of energy flow and cannot
by itself identify radiation, dressing, a wake, a photon, or a particle.
Signed transport is supplied separately by the exact FTD-0671 regional
ledger.  No dynamics, default, toggle, force, scenario, or ontology changes.
