# Exact singleton transport sector: preregistered recovery contract

Date: 2026-09-05. Law: `phi-v2-staged-candidate-1`.
Initial status: **[PREREGISTERED — RESTRICTED SECTOR]**. This contract was
written before running its new algebraic/runtime validation. Results will be
appended below without rewriting these acceptance conditions.

## Frozen question and preparation

Does the existing staged law admit a noninteracting, exactly invariant finite
sector whose individual trajectories admit a uniform finite-time transport
comparison? This is a positive attack on a restricted recovery target, not
collisional Maxwell, gravity, stable matter, or a new primitive probability.

Use finite periodic L>=3 domains. Initially every SC/FCC slot contains the
same nonblank A9 token; s=0; ell is spatially uniform; exactly one field channel
is occupied globally; pending controls are clear. Every allowed channel,
polarity, initial uniform layer, and nonblank homogeneous background is admitted.
This is an external preparation restriction, not a dynamically formed vacuum.

Because both slots are occupied, absorption and relation crossing should be
inert. Relation phases must still rotate in the actual collision/relation
stage. The single field token cannot trigger the exactly-two collision.
Uniform relation incidence must keep manifestation zero. Saved crossing gates
remain complete finite state and must not be erased or bypassed in execution.

## Locked tests and quantities

1. Enumerate the complete existing 384-channel permutation U into disjoint
   cycles. Derive their SC displacements from the actual channel tangents.
   Derive the uniform ell and A9-background cycles from their finite maps.
   Do not inherit the old projected wave speed or fit a target coefficient.
2. One channel update/hop is charged four actual physical microticks. Include
   the within-cycle timing: the hop occurs on input phase 2, reaching phase 3.
   Compute a complete internal period, its displacement and exact drift by
   dividing displacement by the charged period. Enumerate every microtick in
   that period to bound ripple in the L-infinity spatial norm.
3. Compare a finite lifted microscopic path with uniform translation. The lift
   records actual one-hop seam crossings over the declared finite horizon;
   it does not postulate an infinite microscopic state. For a spacing a and
   tick duration tau, report the exact integer-tick bound and the supremum
   bound for a position held constant between ticks. State the scaling a/tau
   explicitly and include the within-tick remainder.
4. Check actual staged runtime transitions for all channel-orbit representatives
   across one complete internal period, inspecting channel, position, layer,
   homogeneous relation background, manifestation, token count, pending state,
   and absence of absorption/collision/crossing. Include periodic seam paths.
   Additional L=4 and L=7 fixtures check the coordinate lift independently.
5. A finite counting ensemble means separate singleton experiments, optionally
   repeated by positive integer multiplicity. Derive an explicit deterministic
   pairing bound in W1 with L-infinity cost, hence for unit-Lipschitz observables.
   Do not evolve multiple occupied tokens in one lattice and call them independent.
6. All acceptance is exact integer/Fraction equality or a rigorously derived
   inequality. No numerical coincidence search, sampled eigenvalue fit, random
   campaign, adjustable threshold, or continuum-speed target is permitted.

## Continuum meaning and rejection conditions

The intended comparison is measure-valued characteristic advection, with one
velocity species for each derived orbit displacement class. The microscopic
object is a deterministic finite trajectory. The ensemble is external finite
counting. The continuous space/time chart and any smooth reference measure are
explicit comparison objects, not primitive records or recovered Born weights.

The desired finite formulation is: for every stated finite T and tolerance,
choose finite spacing/tick duration and a finite probe region (or a finite path
lift) such that the displayed bound holds uniformly through T. A refinement
family is a comparison of finite preparations, not a completed infinity.

Reject this sector certificate if the actual law leaves the sector, complete
clock/background return is missing, the displacement accounting omits a physical
stage, or any tested microscopic path exceeds the derived bound. A theorem
about this sector cannot be promoted to collisional/hydrodynamic closure or
matter recovery. A singleton has conserved transport; persistence of this
prepared token is not a constituent-complete particle identification.

## Results

**Outcome: [DERIVED — EXACT PREPARED TRANSPORT SECTOR; independent review
pending].** All 48 focused tests passed in 9.76 s on the first full run.
The preregistered requirements above were not changed after validation.

The implementation is
[`recovery_transport.py`](../../scripts/phi_v2_lattice/recovery_transport.py),
with [`test_recovery_transport.py`](../../scripts/tests/phi_v2_lattice/test_recovery_transport.py).
Reproduce from the repository root:

```text
python -m pytest scripts/tests/phi_v2_lattice/test_recovery_transport.py -q
```

The result is a controlled **eight-velocity ballistic/advection sector**. It
does not close the full collisional continuum program. Broad runtime flags for
continuum, Maxwell, matter and gravity recovery must remain false.

### 1. Exact invariant prepared sector

Fix a channel c, a uniform ell, and one nonblank A9 background z in every SC/FCC
slot. There is exactly one global field token. Both relation slots remain
occupied: absorption cannot write into them, and the crossing condition
requires exactly one occupied slot. The phase-1 transaction therefore rotates
each relation token, preserving its polarity and spatial uniformity. All
oriented incoming/outgoing uniform primary contributions cancel site by site,
including both FCC diagonal conventions, so s remains zero.

A global singleton produces at most one field token in either polarity bank
at any site. It cannot trigger the exactly-two collision. Its finite flag/phase
therefore advances by U and streams one hop in the actual phase-2 transaction.
There is no manifested-departure half-turn because s=0. Its destination is
unique and its global singleton count is preserved. These statements prove
sector invariance by induction over all four physical stages.

Pending controls are still present. During phases 1–3 the saved gate is false
precisely on relations incident to the field site at the preceding phase-zero
boundary, and true elsewhere. Admitted masks are always zero. At the next
phase-zero boundary every pending control is cleared. The runtime audit checks
these fields as well as the payload, including the gate's deliberately retained
old position after streaming. The exact conserved token count is `18 L^3 + 1`;
this includes the prepared relation background and is not physical energy.

### 2. Complete finite orbit and charged drift

For a channel flag `(d,n,h)`, define the signed coordinate triad
`P0=d`, `P1=h n`, `P2=d cross n`. The actual internal map cycles this triad:
its successive pre-update streaming tangents are `P0,P1,P2,P0,...`.
Its C4 phase advances once per hop. Thus the complete channel repeats after
12 hops. The exact 384-channel census yields 32 disjoint cycles of length 12.
The uniform layer has period 3 relation/collision transactions; every nonblank
A9 background has period 4 rotations. Consequently the complete internal
record, including global schedule phase and cleared pending controls, returns
after **48 actual microticks**, up to the field site's translation by

```text
D = 4 (P0 + P1 + P2) = (plus-or-minus 4, plus-or-minus 4, plus-or-minus 4).
```

The derived velocity in lattice nodes per candidate microtick is

```text
v = D/48 = (P0 + P1 + P2)/12.
```

There are exactly eight body-diagonal velocity species, with components
independently `+1/12` or `-1/12`. Their L-infinity speed is `1/12`; their
Euclidean speed is `sqrt(3)/12`. These are observer-coordinate statements, not
a calibration to physical light speed. They do not inherit the former
projected transverse speed `1/6`.

This sector is **anisotropic**: it supplies eight fixed directions and no
rotationally invariant collection of arbitrary transport velocities. Finite
cubic symmetry and a common Euclidean norm for the eight velocities do not
establish continuum rotational or Lorentz invariance.

### 3. Exact finite-horizon microscopic comparison

Start at a phase-zero boundary and let m be the elapsed physical microticks.
The number of completed hops is `H(m)=floor((m+1)/4)`: hops complete at
`m=3,7,11,...`. If `d_r` is the actual channel tangent after r applications of U,
the finite lifted position is exactly

```text
X(m) = X(0) + sum over 0 <= r < H(m) of d_r.
```

The runtime lift is independently reconstructed from its observed neighboring
site indices, including seams; it does not merely assign this formula to an
unexamined trajectory. L>=3 makes every actual SC-hop seam displacement
unambiguous. Every horizon uses only a finite sequence of lifted coordinates.

The drift-subtracted displacement repeats after the complete internal period:
`X(m+48)-X(0)-(m+48)v = X(m)-X(0)-m v`. Exhaustive exact Fraction evaluation of
all 48 residue times for every channel gives

```text
||X(m)-X(0)-m v||_infinity <= 5/6       for every integer m >= 0.
```

For an external length spacing a>0 and tick duration tau>0, let the microscopic
comparison path be held constant on each interval `[m tau,(m+1) tau)`. Compare
it with `x(0)+(a/tau) v t`, where `x(0)=a X(0)`. On each held interval the error
is affine before taking absolute values, so its supremum occurs at one of
the two endpoints, with the right endpoint understood as a one-sided limit.
Exact endpoint enumeration, including within-stage ripple, gives

```text
sup over 0 <= t <= T of
||a X(floor(t/tau))-a X(0)-(a/tau) v t||_infinity <= (11/12) a
```

for every stated finite T. At the integer tick times the stronger bound is
`(5/6) a`. These are uniform in T, not merely fixed-step small-wavelength
statements. Holding `a/tau=u` fixed yields an explicit O(a) ballistic
comparison with physical-chart velocity `u v`; u remains an external unit
choice. For any tolerance epsilon>0, taking finite `a <= 12 epsilon/11`
gives the claimed finite-horizon error. The test of a large symbolic horizon
uses periodicity as an exact algebraic identity, not a claim that those ticks
were simulated.

A finite torus is a probe. The displayed Euclidean bound uses the declared
finite path lift. Without that lift, ordinary coordinate subtraction across
a periodic seam is inappropriate. Alternatively the same local statement can
be restricted to a finite no-seam observation region and horizon. This result
does not establish an undefined-boundary implementation of the constitution.

### 4. Finite counting and the transport equation

Take a finite list of **separate singleton universes**, with positive integer
multiplicities w_i and total N=sum w_i. This is an externally selected counting
experiment. Multiple simultaneously interacting tokens in one lattice are
outside the proof; superposing singleton solutions into such a state is not
licensed by this result.

Define the empirical measure of their lifted positions and its characteristic
comparison by

```text
mu_a(t) = (1/N) sum_i w_i delta_[a X_i(floor(t/tau))],
nu_a(t) = (1/N) sum_i w_i delta_[a X_i(0)+(a/tau) v_i t].
```

Pairing each microscopic trajectory with its own characteristic is an explicit
coupling. Therefore, for transport cost given by L-infinity distance,

```text
W1_infinity(mu_a(t),nu_a(t)) <= (1/N) sum_i w_i ||error_i(t)||_infinity
                            <= (11/12) a.
```

Equivalently, every scalar observable with Lipschitz constant at most one in
that spatial norm has absolute empirical-expectation error at most `11a/12`.
The implementation reports both this uniform held-time bound and the weighted
maximum error actually observed at integer ticks. Neither expression introduces
a microscopic probability measure or a Born interpretation.

For each fixed derived species v, the comparison measure exactly satisfies
the weak constant-velocity advection equation
`partial_t nu_v + (u v) dot grad nu_v = 0`: differentiation of a smooth test
function along each characteristic gives the weak equation directly. A finite
empirical measure is not a smooth fluid density. If an independently specified
initial measure of the **same velocity species masses** is approximated by
the initial empirical measures with summed weighted W1 error epsilon_0,
constant-velocity translation preserves each species' W1 error. The total
comparison bound is then `epsilon_0 + 11a/12`. Approximating that reference
measure is an additional declared preparation task, not an output inferred
from one singleton.

### 5. Executed validation and limits

The algebraic census covers all 384 channels and all 24 uniform-layer/nonblank-
background combinations. Actual runtime checks cover all 32 channel-orbit
representatives for 48 stages each (1,536 transitions), varying the layer and
background and checking every live/pending array. Four additional L=4/7 seam
fixtures run 97 stages each, including exact large Python global-clock values.
These fixture runs validate the implementation; the uniform all-horizon result
uses the structural invariant-sector and periodicity arguments above.

The tests also check two distinct velocity species with unequal integer
multiplicities, held-time ripple, invalid preparation/scaling rejection and
an injected extra-token defect. They do not test a random ensemble or estimate
a rare collision rate. The original selected law and staged candidate remain
unchanged.

This closes a restricted microscopic-trajectory to characteristic-transport
comparison at the stated conditional scope. It does not close collisional
correlation feedback, general projected return, hydrodynamic relaxation,
Maxwell recovery, Born weights, interacting matter, common matter/light cones,
or gravity. Those gates retain their previous status.
