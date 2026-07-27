# AUDIT — Centered-fiber knot transaction

**Date:** 2026-07-25  
**Identifier:** `FTD-0496`  
**Status:** `[THEOREM — CENTERED CURRENT TRACE]` +
`[THEOREM — CONTRACTION/UNIQUE KNOT STEP]` +
`[CONSTRUCTIVE — NONHOLONOMIC FIBER TRANSACTION]`  
**Verdict:** `UNIQUE_CENTERED_FIBER_KNOT_STEP`  
**Pre-registration:**
[`PREREG_CENTERED_FIBER_KNOT_TRANSACTION_v1.md`](../10_eft_program/preregistrations/PREREG_CENTERED_FIBER_KNOT_TRANSACTION_v1.md)  
**Run of record:** `engine/results/ftd_0496/windows_msvc_cpu.json`

## 1. Exact centered trace of a straight deposited current

For a segment from a knot to `d` inside one signed octant, the face current in
direction `a` is weighted by the two transverse anchor hats. Exact integration
gives

```text
I_bc(d)=integral_0^1 (1-|d_b|t)(1-|d_c|t) dt
       =1-(|d_b|+|d_c|)/2+|d_b d_c|/3,

C(K)_a=(q d_a/2) I_bc(d).
```

The factor `1/2` is the centered average of the occupied one-sided face and
the zero opposite face. The implementation checks both polarities, three
magnitude triples, and all eight octants. The worst residual is `3.47e-16`.

## 2. Exact matter work and causal displacement

Using

```text
H(p)=sqrt(E_REST^2+c^2|p|^2),
d=dt c^2(p_0+p_1)/(H_0+H_1),
p_1-p_0=dt gq E_center^(1/2),
```

gives

```text
H_1-H_0
=c^2(p_1+p_0) dot (p_1-p_0)/(H_1+H_0)
=gq E_center^(1/2) dot d.
```

Moreover `H_i>=c|p_i|` and the triangle inequality give `|d|/|dt|<=c`.
No velocity clipping or post-step energy projection is used.

## 3. The pre-state determines a unique midpoint transaction

Since `E_mid=E_0-gK/2`, the exact current trace reduces the implicit problem
to

```text
p_1=T(p_1),
T_a=p_0,a+dt gq C(E_0)_a-(dt g^2/4)d_a I_bc(d).
```

For `R_a(d)=d_a I_bc`, its Jacobian has diagonal magnitude at most one and
off-diagonal row sum at most `|d_a|`, giving

```text
||D R||_2 <= 1+c|dt|.
```

The relativistic displacement obeys

```text
||D_p d||_2 <= |dt|c^2/E_REST.
```

Therefore

```text
Lip(T)<=g^2 dt^2 c^2(1+c|dt|)/(4E_REST).
```

At the locked values this is `0.13707924958433651`. Banach contraction gives
a unique root throughout the admitted knot-to-subcell domain. Three widely
separated initial guesses converge to the same root in at most 12 iterations;
the worst fixed-point residual is `2.78e-17`.

## 4. Branch selection without a route variable

The centered rule and existing momentum determine the displacement:

- zero field and zero momentum remain exactly at rest;
- a symmetric `D E=q` self source also remains exactly at rest;
- an external centered bias creates a nonzero signed displacement;
- nonzero initial momentum selects the corresponding signed octant;
- opposite polarity mirrors the driven trajectory;
- translations and all 48 signed cubic transformations agree exactly.

Thus the FTD-0491 ambiguity is removed for this selected knot rule without an
x/y/z route label. This does not contradict FTD-0491: the present map abandons
the ordinary one-sided branch action and uses the explicit FTD-0495
nonholonomic fiber.

## 5. Exact matched transaction

After solving the root, the observer deposits the full FTD-0478 current and
sets

```text
E_1=E_0-gK,
D_1-D_0=g<K,E_mid>-gq E_center^(1/2) dot d.
```

The registered maxima are:

```text
total energy residual       1.72e-14
relative Gauss residual     2.33e-16
current continuity residual 5.28e-16
constructed inverse         7.64e-17
causal excess               0
```

The largest dressing transfer is `0.01637445250631649`. Reversing the segment,
current, impulse, field update, and fiber returns the initial state.

## 6. Scope ceiling and next gate

The largest registered displacement is only `0.089472455266425346`. No
integer threshold is crossed. The result proves one unique knot-to-subcell
transaction, not arbitrary-remainder evolution or mobile matter.

The next derivation must extend the path-averaged centered rule to a general
starting remainder, preserve contraction or otherwise prove unique inversion,
and carry the state through an actual anchor change. Ballistic dressed motion,
packet-caused hopping, magnetic work, reactions, production, scenarios, and
infrared claims remain unlicensed.

## 7. Reproducibility

- checks: `13/13 PASS`;
- test SHA256:
  `634F5357403709C84587AC767828AB797673AE9565395C9003A6BF79E51B6A63`;
- header SHA256:
  `C656647420FAACC06E65AC275D2E3A8A5A0C9DE320ABF2A16EBA8F888669A073`;
- implementation SHA256:
  `0E2BEF9427E9DB131D2DB86988DEC3CC91E863BBD9B468A27D36885564884835`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
