# PRE-REGISTRATION — Axial face-hop reciprocity v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0497`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0478`, `FTD-0490`, `FTD-0495`, `FTD-0496`

## Question

Can the exact face-current/common-work transaction be continued through one
genuine production-style `|remainder| >= 1` anchor change while preserving
both the physical state and the raw manifested `(site,remainder)` state under
the same reverse update?

This is an observer-only axial theorem/prototype. It does not modify the
production tick, add a toggle, or authorize a scenario.

## Locked kinematics

Restrict motion to one principal axis with zero transverse remainder and
momentum. Use

```text
H(p)=sqrt(E_REST^2+c^2 p^2),
d=dt c^2(p_0+p_1)/(H_0+H_1).
```

The endpoint is represented by the existing production threshold map

```text
r_raw=r_0+d,
(n_1,r_1)=
  (n_0+1,r_raw-1), r_raw >= 1,
  (n_0-1,r_raw+1), r_raw <= -1,
  (n_0,r_raw),     otherwise.
```

No canonical-cell replacement, route label, hop bit, or stored previous
anchor is allowed.

## Locked matched transaction

For each trial endpoint, deposit the exact FTD-0478 current `K(d)` and set

```text
E_1=E_0-gK,
E_mid=(E_0+E_1)/2,
E_path=<K,E_mid>/(q d),
p_1-p_0=dt g q E_path.
```

At `d=0`, use the occupied face in a signed subcell and the arithmetic trace
of the two adjacent faces at an exact knot. The discrete-gradient identity
must give

```text
H_1-H_0=g<K,E_mid>,
Delta(1/2 ||E||^2)= -g<K,E_mid>.
```

For a uniform pre-field and a segment crossing at most one integer plane,
write the signed segment lengths as `ell_i`. Then

```text
E_path=E_0-(gq/2) sum_i ell_i^2/d,
|d/dd [sum_i ell_i^2/d]| <= 1,
Lip(T) <= g^2 dt^2 c^2/(2 E_REST).
```

At `g=0.73`, `dt=1`, `c=1/sqrt(3)`, and `E_REST=0.511`, require this bound
below one and convergence from three initial guesses to the same root.

## Locked reversibility discriminator

For a positive hop with `0<d<1` and `1-d<r_0<1`, the threshold map gives

```text
M_d(n,r_0)=(n+1,r_0+d-1),
M_-d M_d(n,r_0)=(n+1,r_0-1) != (n,r_0).
```

The two final representations have the same effective position and the same
trilinear polarity shape. They are nevertheless different frozen engine
states because ternary manifestation and collision occupancy live on the
anchor site.

Also test the explicit preimage collision

```text
M_d(n,r_0)=M_d(n+1,r_0-1).
```

Thus the raw threshold map is non-injective if this identity holds. An exact
inverse cannot then be constructed from the final frozen state alone.

## Locked arms and gates

1. zero field and zero momentum at an interior remainder remain static;
2. a uniform axial field drives a positive-polarity positive hop from
   `r_0=+0.85` and a negative-polarity negative hop from `r_0=-0.85`;
3. all three principal axes and integer translations agree;
4. current continuity, relative Gauss transport, matter work, total energy,
   locality, causal speed, and fixed-point residual are below `1e-12`;
5. physical position, polarity shape, current, field, and momentum reverse
   below `1e-10`;
6. raw site and remainder must also reverse exactly for the positive verdict;
7. at least one explicit pair of distinct raw preimages must be tested for a
   common output.

## Frozen verdicts

- `REVERSIBLE_AXIAL_FACE_HOP` only if every algebraic gate and the raw-state
  inverse gate pass.
- `AXIAL_HOP_PHYSICAL_QUOTIENT_ONLY` if exact face work, energy, Gauss,
  continuity, causal motion, and physical reversal pass but the raw
  `(site,remainder)` inverse or injectivity gate fails. This closes the
  candidate for the frozen ontology.
- `AXIAL_FACE_HOP_CLOSED_NEGATIVE` if any matched transaction identity fails.

## Scope ceiling

A negative raw-state verdict does not authorize changing the ±1 threshold,
declaring anchor location a gauge redundancy, or adding a hidden hop-history
variable. A positive verdict would still establish only isolated axial motion,
not multi-axis motion, magnetic exchange, reactions, a production toggle, a
scenario, or infrared recovery.

Run-of-record SHA256 values:

- test: `33DB9450D23F35C6D9D76670997E6EA3DA674CC2601E4E5CA3C60447B6EC83A0`;
- header: `A68DD03779B72B77A1C43077F80B9AB5A71946F44396836E904C1803F623792E`;
- implementation:
  `33392108DB3DFA8CD83864B44A346E85CBDF0F85C75FD4ACC1DE2F3072E49644`.
