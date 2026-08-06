# PRE-REGISTRATION — Matched-face energy transaction v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0472`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0427`, `FTD-0428`, `FTD-0470`, `FTD-0471`  
**Engine artifact:** `engine/tests/campaign_matched_face_energy_transaction.cpp`  
**Campaign SHA256:** `65EE0F5F11B8B69A58AFDCE16E02A89E546103BA15D89DE9F539B024A5F80FAB`  
**Helper SHA256:** `787EA84CE298063E034F4A5ABF70EE5452AE9038604DEF1A4097D9B5B57F5B4D`

## 1. Question

FTD-0471 proved that an oriented face layer transports a one-site source
change locally. This campaign asks whether the same selected layer owns an
exact finite-step energy transaction, and whether the primitive Moore hop
uniquely determines that transaction for face, edge, and corner moves.

The campaign is observer-only. It does not change the production tick, select
a new movement rule, or claim a field-momentum law.

## 2. Pre-derived energy identity

Write `lambda=c*dt`, let `C` be the matched curl, and use the FTD-0428
half-tick invariant

```text
H~(E,B) = 1/2 ||E||^2 + 1/2 ||B||^2
          - (lambda/2) <B,C^T E>.
```

The source-free staggered step is

```text
B_1 = B_0 - lambda C^T E_0,
E_* = E_0 + lambda C B_1,
```

and the conservative event current `K` then gives `E_1=E_*-K`. Direct
expansion yields

```text
H~(E_1,B_1)-H~(E_0,B_0)
  = -<K,E_*-K/2-(lambda/2) C B_1>
  = -<K,(E_0+E_1)/2>.
```

Thus the exact matter-work currency is the endpoint-midpoint electric field:

```text
W_matter = <K,(E_0+E_1)/2>,
Delta H~ + W_matter = 0.
```

Using the pre-current field instead gives the registered defect

```text
Delta H~ + <K,E_*>
  = 1/2 ||K||^2 + (lambda/2)<K,C B_1>.
```

The complete affine map has the explicit inverse

```text
E_* = E_1 + K,
E_0 = E_* - lambda C B_1,
B_0 = B_1 + lambda C^T E_0.
```

These are finite-dimensional algebraic identities, not continuum imports.

## 3. Route statement under test

An axis-adjacent hop has one face route. An edge Moore hop has two ordered
face routes and a corner hop has six. Routes with the same endpoint have the
same divergence but can differ by a closed face-current loop.

For a curl-free minimum-energy electric dressing, the discrete line integral
should be path-independent up to solver tolerance. After adding a registered
divergence-free transverse dressing, at least one edge/corner endpoint group
must acquire distinct work values. That result would mean energy closes for
every chosen route while the primitive Moore event still lacks the route type
needed to choose a unique microtransaction.

## 4. Frozen fixtures

- periodic `L={16,17}`;
- both charge signs;
- all 26 Moore displacements;
- all distinct Cartesian face-orderings: 1 face, 2 edge, 6 corner;
- two field arms:
  - minimum-energy neutral dipole, zero initial magnetic field;
  - the same dressing plus `make_transverse_challenge(L,0.037)` through the
    matched curl and initial magnetic challenge amplitude `0.019`;
- `wave_speed=C_SPEED`, `dt=1`;
- exact stationary opposite charge included in both history endpoints;
- 624 transaction rows in 208 endpoint groups, 160 of them multi-route.

## 5. Gates

- all fixtures initialize and remain finite;
- source-free, full midpoint, current-only midpoint, and naive-defect formula
  residuals each `<=1e-10`;
- exact inverse residual `<=1e-12`;
- continuity residual `<=1e-10`, Gauss residual before/after `<=1e-9`;
- distinct route counts exactly `1!`, `2!`, `3!` for face/edge/corner;
- face groups have zero work span and zero endpoint-field difference within
  `1e-10`;
- electrostatic route work spans `<=1e-9`;
- at least one transverse multi-route group has work span `>1e-6`, and at
  least one multi-route endpoint field differs by `>1e-6`;
- the pre-current work rule fails by more than `1e-3` somewhere while its
  derived defect formula still closes.

## 6. Outcome map

- all gates pass:
  `MIDPOINT_WORK_EXACT_MOORE_ROUTE_TYPE_STILL_REQUIRED`;
- algebraic, inverse, Gauss, route-count, or registered-control gate fails:
  `MATCHED_FACE_ENERGY_TRANSACTION_CLAIM_FAILS`;
- invalid initialization/history:
  `PROTOCOL_INVALID`.

A positive verdict establishes an exact selected face-layer energy ledger. It
does not derive particle inertia, a hop acceptance rule, field momentum,
Poynting recoil, a unique Moore routing convention, or a production
replacement. Momentum remains the next independent mechanics gate.

## 7. Run of record

Pinned MSVC `14.44.35207`, Release, CPU observer, focused target
`campaign_matched_face_energy_transaction`, stdout captured as
`engine/results/ftd_0472/windows_msvc_cpu.csv`.

**Recorded outcome:**
`MIDPOINT_WORK_EXACT_MOORE_ROUTE_TYPE_STILL_REQUIRED`. All 624 transactions
closed the endpoint-midpoint work identity, Gauss transport, and explicit
inverse. Electrostatic route work was path-independent; 78/80 transverse
multi-route groups were path-dependent, with maximum span `0.0122917`. See
`AUDIT_MATCHED_FACE_ENERGY_TRANSACTION.md`.
