# AUDIT — Exact momentum face balance

**Date:** 2026-07-25  
**Identifier:** `FTD-0514`  
**Status:** `[THEOREM — COMPONENTWISE MOMENTUM-CONTINUITY LIFT]` +
`[THEOREM — INTEGRATED KINETIC-STRESS BRIDGE]` +
`[CONSTRUCTIVE — SELECTED CONTACT BALANCE]` +
`[CLOSED BY FTD-0516 — SELECTED CONTACT ACTION]` +
`[CORRECTED BY FTD-0526 — IDENTICAL CONTACT IMPULSE IS QUOTIENT DATA]` +
`[OPEN — DISTINGUISHABLE CONTACT/FIELD ORIGIN]`  
**Verdict:**
`EXACT_MOMENTUM_FACE_BALANCE_CLOSES_SELECTED_CONTACT_COMPATIBILITY_ONLY`  
**Pre-registration:**
[`PREREG_EXACT_MOMENTUM_FACE_BALANCE_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_EXACT_MOMENTUM_FACE_BALANCE_v1.md)  
**Run of record:** `engine/results/ftd_0514/windows_msvc_cpu.json`

## 1. The exact lift

Let one carrier's compact trilinear number density and oriented face current
obey the already-proved scalar identity

```text
Delta rho + div J = 0.
```

During a straight segment the carrier momentum `p` is constant. Define

```text
g_i = p_i rho,
Pi_ij = p_i J_j.
```

Multiplication of the scalar equation by each constant component `p_i` gives

```text
Delta g_i + div_j Pi_ij = 0
```

site by site. This is an algebraic theorem once exact scalar continuity is
available; it is not a fitted constitutive law. `rho` here is unit-carrier
density, so momentum density is polarity-even. Electric charge density remains
the signed polarity shape of FTD-0478.

For an instantaneous momentum jump at fixed position, the trilinear vertex
density gives the exact integrated impulse source

```text
I_i = (p_i^after-p_i^before) rho_vertex,
Delta g_i - I_i = 0.
```

Piecewise transport and impulses therefore compose as

```text
Delta g_i + div_j Pi_ij - I_i = 0.
```

No new spatial stencil, route choice, field, or production variable is needed
for this observer identity.

## 2. Why the tensor flux is the FTD-0513 stress

The analytic face-current segment has the first-moment identity

```text
sum_faces J_j = Delta x_j.
```

Therefore

```text
sum_faces Pi_ij = p_i Delta x_j.
```

For free motion under the existing dispersion,

```text
E^2 = m^2 + c^2 |p|^2,
v = c^2 p/E,
Delta x = v Delta t,
```

and hence

```text
sum_faces Pi_ij = Delta t p_i v_j.
```

Summing carriers recovers exactly the FTD-0513 kinetic-stress moment

```text
Sigma_ij = sum_a p_i^(a) v_j^(a).
```

Thus `Sigma` is the spacetime-integrated first moment of local constituent
momentum transport. This supplies the missing local-balance interpretation of
FTD-0513. It does not turn `Sigma` into an independent field or prove a stress
equation of motion.

## 3. Selected collision composition

The FTD-0512 restricted collision has two incoming segments, two equal and
opposite vertex impulses, and two outgoing segments. Each constituent obeys
the sourced equation above. Because

```text
Delta p_1 + Delta p_2 = 0,
```

the two vertex sources cancel site by site even though each source is nonzero.
The complete pair then obeys a source-free aggregate momentum balance.

This closes a compatibility question only. The face observer accepts the
FTD-0512 Householder impulse after central contact, elasticity, and outgoing
nonpenetration have selected it. The aggregate electromagnetic density/current
still cannot derive that impulse: the FTD-0512 axial kernel remains exact.

## 4. Exact campaign

The frozen campaign used `L=17`, both polarities, all 26 nonzero Moore
directions, three translations, and two speeds. It ran 312 free arms and 312
selected-collision arms.

```text
worst free local residual             2.7755575615628914e-17
worst free global residual            1.6653345369377348e-16
worst free first-moment residual      6.9388939039072284e-18
worst free stress-bridge residual     2.0816681711721685e-16
worst free causal residual            0
worst collision segment residual      1.3877787807814457e-17
worst constituent impulse residual    0
worst aggregate impulse-source L1     0
minimum individual impulse-source L1  0.78512214774851818
worst collision local residual        5.5511151231257827e-17
worst collision global residual       1.7347234759768071e-17
worst collision energy residual       2.2204460492503131e-16
worst collision tensor-moment residual 2.7755575615628914e-17
```

Under time reversal, momentum density negates and its endpoints swap. Both
`Pi=p tensor J` and the integrated impulse source are even because `p`, `J`,
and the event orientation all reverse together. The worst registered flux
parity residual was `6.94e-18`; endpoint and source parity residuals were zero.

## 5. What closed and what did not

Closed:

1. exact local momentum bookkeeping for a known constituent worldline;
2. exact identification of the integrated tensor face flux with kinetic stress;
3. exact local and global balance for the already selected restricted contact;
4. compatibility with causality, cubic/translational copies, polarity mirror,
   and one-step reversal at observer level.

Still open:

1. a physical interaction functional for distinguishable or nontrivial
   collision sectors;
2. a conjugate strain/connection/contact variable that exchanges momentum and
   energy with the stress channel;
3. generic unequal-mass, noncentral, multistream, and reaction events;
4. a production transaction, exact field recoil, and reciprocal mobile matter.

The result forbids one misleading upgrade: exact balance after an impulse is
not an origin for the impulse. No production code, default, toggle, scenario,
force, collision rule, field normalization, or tolerance changed.

**Successor FTD-0516 supplies a selected, not native, origin:** one unilateral
hard-contact matter action derives the restricted impulse and composes with
this balance. FTD-0526 proves that the impulse is only permutation-section data
for the registered identical-carrier class: pass-through and bounce have the
same aggregate phase/current history. Reciprocal face E/B force and a genuine
distinguishable-collision origin remain open.

- checks: `5/5 PASS`;
- test SHA256:
  `7716F71E15EEE6D2A89EA2F419F3E9D9BF1B691F8A72DB7D53360843369EF0B1`;
- header SHA256:
  `B9F435FF75E7EE133A9393294E45B1C316E026472A0C93FCF457077BDE6A6567`;
- implementation SHA256:
  `2F5BE00608D85CAF02D57E5813362BA4484EC6196154BFB8C88EF5D776B2AB86`;
- locked preregistration-body SHA256:
  `05C02C5075CD2DA1359094C13CBF40A2D101E18FE5651B7F17A19A71BF5A9419`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
