# FTD-0767 — Dynamic field response and the wake-clearing bound

**Status:** `[DERIVED OBSERVER ALGEBRA + POST-HOC NUMERICAL FACT FROM LOCKED FTD-0766 ARTIFACT; MATTER INTERPRETATION OPEN]`

**Production status:** unchanged

**New engine run:** none

## 1. The distinction FTD-0766 did not close

FTD-0766 found a velocity-aligned trailing/leading imbalance around a moving
relational core.  It correctly refused to promote a wake because the registered
execution was invalid and the signal had the wrong amplitude and age ordering.
There is a more basic distinction:

```text
local deformation: the field around the core becomes fore--aft asymmetric;
spatial wake:       a motion-created disturbance remains in a region the core
                    and its constitutive near field have already vacated.
```

A boat wake, flame trail, or discarded environmental memory is a spatial
claim.  A half-space imbalance measured while the initial and final support
windows still overlap cannot establish it.

## 2. Rest-subtracted directed response

Let `R_q(x,t)` be the FTD-0763 selected residual field for boost `q`, expressed
in the chart aligned with that arm's direction of motion.  In a registered
window define

```text
w_q(t) = U_T[R_q(t)] - U_H[R_q(t)],                       (1)
```

where `T` and `H` are trailing and leading residual-field energies.  The rest
arm is stored in the `+d` chart.  Reversing the chart swaps its halves, so the
matched dynamic directed responses are

```text
delta w_+(t) = w_+(t) - w_0(t),
delta w_-(t) = w_-(t) + w_0(t).                          (2)
```

The signed-pair response is

```text
delta W_pair(t) = [delta w_+(t) + delta w_-(t)]/2.        (3)
```

Equation (3) cancels the static rest-arm directional numerator algebraically:

```text
delta W_pair = [w_+ + w_-]/2.                            (4)
```

Thus the positive FTD-0766 pair numerator is not solely the initial static
anisotropy that broke its raw-triple mirror gate.  It is evidence for a local,
velocity-aligned redistribution of the selected residual-field energy.  It is
not yet evidence for a spatially deposited wake.

The certificate normalizes (3) by the average moving-arm directed energy,

```text
D_dynamic = delta W_pair
            / {[(T_+ + H_+) + (T_- + H_-)]/2}.           (5)
```

On the locked artifact, (5) reproduces the reported final `D_pair` within
`5e-12`.  This agreement is a fact about the registered signed pairs, not a
general identity between an average of ratios and a ratio of averages.

## 3. Energy subtraction is not field subtraction

Let the positive quadratic field-energy form be

```text
U(R) = (1/2) <R,R>_H,
delta R_q = R_q - R_0.                                   (6)
```

Then

```text
U(R_q) - U(R_0)
  = <R_0,delta R_q>_H + (1/2)<delta R_q,delta R_q>_H.     (7)
```

The first term is interference with the preparation field.  The second is the
nonnegative norm of the motion-created field difference.  Therefore a
rest-subtracted regional energy can be negative and cannot be called “created
field energy” by itself.  A complete successor must record both terms in (7),
plus the actual regional energy flux and matter work.  This is required to
distinguish deformation, destructive interference, radiation, and deposition.

## 4. Spatial-clearing theorem for the registered observer

FTD-0764/0766 froze a selected-bound support half-width `h=4` and a near-field
radius `R_N=8`.  Along a face ray, two centered intervals of half-width `h`
are disjoint only if their centers are separated by more than `2h`; two near
windows are disjoint only after more than `2R_N`:

```text
bound-support clearing: d > 8,
near-field clearing:    d > 16.                           (8)
```

This is elementary interval geometry conditional on the selected observer.
It does not assert that the observer's support is an ontological particle
boundary.

The 16 valid moving FTD-0766 arms travel only

```text
0.1908219969 <= d <= 0.8756523021.                        (9)
```

The maximum reaches only `10.95%` of the bound-support clearing distance and
`5.47%` of the near-window clearing distance.  No arm moves even one lattice
site.  Consequently FTD-0766 never creates a region that is simultaneously
behind the core and separated from both its initial and final selected near
windows.

The correct post-hoc classification is therefore:

```text
local velocity-aligned field deformation: observed descriptively;
spatially detached wake:                    not tested;
rigid co-moving dressing:                   already rejected at its scope.
```

## 5. Locked numerical record

The independent certificate reports:

| age | `|q|` | pair travel | `delta W_pair` | `D_dynamic` |
|---:|---:|---:|---:|---:|
| 0 | 0.0075 | 0.191541 | 0.00957032 | 0.0842306 |
| 0 | 0.0150 | 0.404247 | 0.00825854 | 0.0744118 |
| 64 | 0.0075 | 0.190969 | 0.00767051 | 0.0552359 |
| 64 | 0.0150 | 0.403566 | 0.00655950 | 0.0480590 |
| 64 | 0.0300 | 0.873776 | 0.00470929 | 0.0349433 |
| 128 | 0.0075 | 0.190822 | 0.00637704 | 0.0402139 |
| 128 | 0.0150 | 0.403717 | 0.00522792 | 0.0333536 |
| 128 | 0.0300 | 0.875652 | 0.00252424 | 0.0161127 |

At ages 64 and 128, both the dimensional directed response and its normalized
asymmetry decrease as the boost and travel increase.  The registered signal is
therefore more naturally described as a short-horizon local deformation or
preparation-memory response than as accumulated trail deposition.

Certificate:

```text
python scripts/proofs/proof_dynamic_response_spatial_clearing.py
FTD-0767 dynamic-response/clearing certificate: 113/113 checks
```

## 6. Matter ontology forced by the correction

The evidence supports a narrower but more coherent three-part description:

1. **Manifested relational kernel.** Two opposite-polarity constituents and
   their derived relation form the presently mobile state-selected core.  The
   object is not either occupied site and is not the sum of two permanent
   voxel identities.
2. **Constitutive constraint field.** The instantaneous state-selected Gauss
   component is needed for the common-action transaction, but its observer
   representation follows the fractional core chart and is not an independent
   material shell.
3. **Environmental dynamical field.** The residual field carries preparation
   memory and propagating degrees of freedom.  Most of its measured centroid
   is not entrained by the kernel over the tested interval.

This makes the best current matter candidate a **moving relational process
through an environment**, not a bead with permanently attached field lines.
Whether the process is flame-like—continually recruiting field ahead and
shedding field behind—cannot be decided until it travels beyond its own
selected support and closes an energy-flow ledger there.

## 7. Recursive questions now controlling the derivation

1. If the kernel crosses nine sites while remaining in the same state-only
   relational sector, what exactly has persisted: constituents, polarity
   ordering, a recurrence phase, or only a classifier label?
2. If no field remains after the kernel clears its old support, is the field
   environmental rather than constitutive, or is the observer subtracting the
   constitutive component by construction?
3. If positive excess remains behind, does its integrated energy equal the
   kernel's kinetic-energy loss plus boundary flux, without an unexplained
   reservoir?
4. If the kernel deposits energy but does not slow, which recorded internal
   degree of freedom supplies that energy?
5. If the kernel slows without a detached trail, is energy radiated broadly,
   stored in local deformation, or exchanged with a Peierls lattice mode?
6. Does the same long-horizon state-only identity survive an integer site hop,
   or does the current classifier merely recognize subcell motion inside one
   anchor chart?
7. Does preparation age change an inertial property, or only move the state
   farther from the selected core's energy-margin boundary?
8. Can two differently prepared histories converge to the same transported
   local state, or does reversible dynamics retain preparation memory forever
   in the environment?
9. Is charge the signed constituent sum, the asymptotic Gauss class of the
   complete process, or a conserved sector label that survives constituent
   replacement?
10. Is inertial mass the curvature of complete energy along a family of
    translated recurrent states, and does that curvature agree with impulse
    response and a pole residue?
11. Does the unresolved momentum defect represent ordinary lattice crystal
    momentum modulo a reciprocal vector, boundary stress, or a missing
    connection variable?
12. Can a finite moving world tube be Markov-complete when supplied only its
    incoming boundary data, making the object's boundary a causal interface
    rather than a permanent membrane?
13. Does a second localized family have a distinct recurrence spectrum and
    scattering response, which would justify calling the two families
    different matter species?
14. If every long-lived object requires an ever-growing environmental record,
    is “particle” only an effective name for a kernel plus its causal history?

## 8. Next gate

The next CUDA campaign must not ask whether a sub-site displacement has a
wake.  It must evolve a qualified aged mobile core until either:

1. the center travels beyond the selected bound-support clearing distance;
2. the core loses its state-only identity before clearing; or
3. the locked causal horizon is reached.

At fixed lab sites and in the moving chart it must record `delta R`, the two
terms in (7), signed actual energy excess, boundary energy flux, matter work,
core energy, and state-only inversion.  Only a response remaining behind in a
cleared region with a closed energy ledger may be called a spatial wake.
