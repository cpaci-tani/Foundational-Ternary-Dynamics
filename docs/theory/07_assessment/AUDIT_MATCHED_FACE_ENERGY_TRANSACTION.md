# AUDIT — Matched-face energy transaction

**Identifier:** `FTD-0472`  
**Date run:** 2026-07-25  
**Status:** `[THEOREM — FINITE-CURRENT MIDPOINT-WORK IDENTITY]` +
`[MEASURED — EXACT INVERSE/GAUSS CLOSURE]` +
`[MEASURED — TRANSVERSE MOORE-ROUTE DEPENDENCE]` +
`[SELECTION REQUIRED — EDGE/CORNER ROUTE TYPE]` +
`[OPEN — FIELD MOMENTUM/PARTICLE INERTIA/HOP SELECTION]`  
**Pre-registration:** [`PREREG_MATCHED_FACE_ENERGY_TRANSACTION_v1.md`](../10_eft_program/preregistrations/PREREG_MATCHED_FACE_ENERGY_TRANSACTION_v1.md)  
**Run of record:** `engine/results/ftd_0472/windows_msvc_cpu.csv`

## Verdict

`MIDPOINT_WORK_EXACT_MOORE_ROUTE_TYPE_STILL_REQUIRED`

The selected matched face layer has an exact finite-step energy transaction.
For the complete staggered wave/current step, particle work is the integrated
face current dotted into the average electric field between the tick
endpoints. All 624 registered transactions close this identity, Gauss, and
the explicit inverse.

That positive result does not yet complete a native hop. A face hop has one
face-current route, but an edge hop has two and a corner hop has six. In a
transverse field those routes reach the same manifested endpoint with the same
charge transport but different field states and different work. The primitive
Moore move does not currently carry the route type needed to select one.

## Exact finite-step identity

The FTD-0428 selected staggered state uses

```text
H~(E,B) = 1/2||E||^2 + 1/2||B||^2
          - (lambda/2)<B,C^T E>,  lambda=c dt.
```

For

```text
B_1 = B_0-lambda C^T E_0,
E_* = E_0+lambda C B_1,
E_1 = E_*-K,
```

direct finite-dimensional expansion gives

```text
Delta H~ = -<K,E_*-K/2-(lambda/2)C B_1>
          = -<K,(E_0+E_1)/2>.
```

Therefore

```text
W_matter = <K,(E_0+E_1)/2>,
Delta H~ + W_matter = 0.
```

No small-step or continuum limit is used. The result is exact within the
selected face/edge complex and its modified-energy convention.

Using the field immediately before the current update is not exact:

```text
Delta H~ + <K,E_*>
  = 1/2||K||^2 + (lambda/2)<K,C B_1>.
```

The registered maximum naive imbalance was `1.5028676374`, while the formula
for that imbalance closed to `8.49e-14`. The midpoint is not cosmetic; it
removes the finite current's self-step and magnetic-centering terms.

## Measurement

The locked campaign covered `L={16,17}`, both charge signs, all 26 Moore
displacements, every distinct Cartesian face ordering, and electrostatic and
transverse field arms.

| quantity | result |
|---|---:|
| transaction rows | 624 |
| endpoint/field groups | 208 |
| multi-route groups | 160 |
| worst full midpoint balance | `8.4821e-14` |
| worst current-only balance | `1.6876e-14` |
| worst source-free invariant residual | `7.0167e-14` |
| worst explicit inverse residual | `7.3726e-17` |
| worst continuity/Gauss residual | `9.9997e-13` |
| maximum naive pre-current imbalance | `1.50287` |
| maximum transverse route-work span | `0.0122917` |
| maximum same-endpoint field difference | `1` face unit |

All 80 electrostatic multi-route groups were path-independent within the
recorded precision: their minimum-energy electric field is curl-free, so its
discrete line integral depends only on endpoints. After adding a
divergence-free transverse dressing, 78 of 80 multi-route groups had work
span above `1e-6`. The two zero-span controls were the local zero-circulation
orientation of the frozen challenge at `L=16`; they do not weaken the
registered existence gate. The maximum span was `0.0122917`.

The different route fields all satisfy the same Gauss source because their
difference is a closed divergence-free face loop. Gauss law therefore cannot
choose among them.

## Physical reading

This supplies one missing piece from FTD-0469 through FTD-0471:

1. `K` is a literal event-current ledger on oriented faces.
2. `E<-E-K` transports the Gauss source exactly and locally.
3. The endpoint-midpoint field gives the exact amount of field energy made
   available to matter.
4. Reversing current, electric drift, and magnetic kick in reverse order
   reconstructs the initial state.

For axis-adjacent motion, those statements define a unique face-layer energy
transaction. For diagonal Moore motion they define a family of exact
transactions, one per face ordering. This is a type gap, not a numerical
error: the current production event says only "source and target are Moore
neighbours" and does not say which oriented-face path occurred within the
tick.

## What is not established

- The midpoint work is not yet assigned to a particle kinetic/inertial state.
- No matched-face field momentum or Poynting recoil has been derived.
- Exact inversion of the selected affine sidecar does not make the production
  tick globally injective.
- The route ordering for edge/corner hops is not native.
- The sidecar remains default-off and does not replace cell-centered `J`.
- Energy closure does not prove emergent `U(1)`, Lorentz recovery, or a photon
  ontology.

## Next gate

The next test is momentum, not another energy ansatz. Derive the translation
generator of the staggered finite lattice, measure its change for the exact
current transaction, and compare it with the particle impulse. The gate must
also test whether a route-independent total momentum exists for diagonal
Moore events. If it does not, FTD must either restrict physical manifestation
motion to face hops or adopt an explicit sub-tick route variable; neither may
be presented as forced by the current five postulates.

## Reproducibility

- campaign SHA-256:
  `65EE0F5F11B8B69A58AFDCE16E02A89E546103BA15D89DE9F539B024A5F80FAB`
- helper SHA-256:
  `787EA84CE298063E034F4A5ABF70EE5452AE9038604DEF1A4097D9B5B57F5B4D`
- run CSV SHA-256:
  `D19FC34CEC9FCAE0C54F68AFCBFF7E69CBC23E7C8580D0533301DFBCB60D1B73`
- toolchain: MSVC `14.44.35207`, Release, CPU observer
- production dynamics: unchanged
