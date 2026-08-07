# AUDIT — Matched-face momentum transaction

**Identifier:** `FTD-0473`  
**Date run:** 2026-07-25  
**Status:** `[THEOREM — SELECTED LOCAL STAGGERED PSEUDOMOMENTUM ON PERIODIC WINDOWS]` +
`[SCOPE CORRECTION — FINITE-CYCLIC TRANSLATION IS COMPUTATIONAL ONLY]` +
`[MEASURED — EXACT CURRENT EXCHANGE]` +
`[CLOSED NEGATIVE — QUIET ELECTROSTATIC RECOIL IN THIS CHANNEL]` +
`[OPEN — AUTONOMOUS MATTER CURRENT/INERTIA]`  
**Pre-registration:** [`PREREG_MATCHED_FACE_MOMENTUM_TRANSACTION_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_MATCHED_FACE_MOMENTUM_TRANSACTION_v1.md)  
**Run of record:** `engine/results/ftd_0473/windows_msvc_cpu.csv`

## Verdict

`LOCAL_PSEUDOMOMENTUM_EXACT_ELECTROSTATIC_HOP_RECOIL_ABSENT`

The selected matched field owns an exact local translation pseudomomentum,
and its change under a face current is known exactly. The positive algebraic
result exposes a negative mechanics result: the quiet electrostatic hop that
exchanges substantial energy has essentially zero recoil in this field
channel. FTD-0472 is therefore an exact driven energy ledger, not yet an
autonomous matter-field hop.

## Exact local invariant

For periodic central translation `D_i`, matched curl `C`, and the staggered
source-free step

```text
B_1 = B_0-lambda C^T E_0,
E_* = E_0+lambda C B_1,
```

define

```text
P_i(E,B)=<E,D_i C B>.
```

Because `D_i` is skew-adjoint and commutes with the translation-invariant curl
products,

```text
P_i(E_*,B_1)-P_i(E_0,B_0)
 = -lambda<E_0,D_i C C^T E_0>
   +lambda<C B_1,D_i C B_1>
 = 0.
```

After the conservative event current `E_1=E_*-K`,

```text
Delta P_i^field=-<K,D_i C B_1>.
```

Both statements are exact periodic-window identities for this selected local
candidate. Six nonzero directed-wave controls over 256 ticks conserved it
with worst absolute drift `5.1903e-14` and relative drift `1.1708e-13`.
Across all 624 current routes, the source-free residual was at most
`6.3335e-17` and the current-exchange formula residual at most `8.1765e-17`.

## Electrostatic result

The electrostatic arm began from the minimum-energy neutral dressing and zero
magnetic half-field. FTD-0472's midpoint work magnitude was never below

```text
0.333155140642,
```

but the largest field-pseudomomentum change was only

```text
3.69893597042e-17.
```

The ratio is below `1.12e-16`. Subsequent source-free staggered evolution
cannot create delayed momentum in this candidate because the source-free map
conserves it exactly. The recoil must enter during the current/matter event or
through a different, explicitly justified momentum type.

This does not violate energy conservation: FTD-0472 says how much work the
prescribed current exchanges with the field. A prescribed current can be an
external agent. The missing step is making that current a dynamical matter
history whose energy and momentum changes are both included.

## Transverse route dependence

With nonzero transverse electric and magnetic dressing, field recoil is
nonzero but inherits the FTD-0472 route ambiguity. Seventy-eight transverse
multi-route endpoint groups had required-impulse span above `1e-6`; the maximum
was

```text
0.00102318226.
```

Thus even this exact pseudomomentum does not cause Gauss-equivalent diagonal
routes to become mechanically equivalent. A route type or face-only movement
restriction remains necessary before a unique transaction exists.

## Scope correction: the ontology is not a finite torus

The campaign uses periodic `L={16,17}` computational windows. Those quotients
have exact translation group `Z_L^3`; their characters give crystal momentum
modulo the reciprocal lattice, and `Hom(Z_L,R)=0`. That statement is correct
for the test topology only.

It does **not** describe FTD's ontology. The substrate is uncontained and has
undefined boundary: it is not a box, a torus, or a completed infinite set.
Finite realized structures are cloud/star-like configurations of finite
extent or support inside that uncontained adjacency, not contents bounded by
a container. There is no ontological relation `T^L=1`, so the finite-cyclic
no-homomorphism argument cannot prohibit a real translation generator.

For an uncontained axial translation algebra modeled locally by `Z`,
`Hom(Z,R)` is nontrivial; the remaining ambiguity is its unit/normalization,
not forced triviality. Extending the periodic summation-by-parts proof to the
uncontained ontology requires a finite-support or decay condition that kills
boundary-at-extent terms. That extension was not run here and remains open.

The local quadratic candidate is distinguished on the periodic probes by
locality and exact conservation, but its overall normalization can be rescaled
and other spectral weightings can produce other conserved pseudomomenta. It is
therefore not promoted to the unique ontological physical momentum.

## Consequence for the ontology

The current state of the matched construction is now precise:

1. Oriented current transports polarity/Gauss source exactly.
2. Endpoint-midpoint electric work closes exact field energy.
3. A local field pseudomomentum and its current exchange are exact.
4. A quiet electrostatic hop transfers energy but no momentum in that channel.
5. The code does not yet generate the opposite matter update from the same
   discrete action.

The flux layer is therefore a valid carrier of propagation, dressing, energy,
and conditional transverse recoil. It is not yet a self-contained pilot wave
that moves manifested matter. Calling the current "charge motion" is only a
kinematic bookkeeping statement until the matter-side trajectory is native.

## What is closed and what remains open

**Closed:** the claim that the FTD-0472 matched current by itself completes a
quiet electrostatic matter-field hop under the minimal local staggered
pseudomomentum.

**Open:**

- a dynamical matter coordinate and inertia;
- a single-action current/force law that pays both midpoint work and recoil;
- a unique route type for edge/corner Moore motion;
- the uncontained/finite-support translation generator or a justified
  Poynting momentum;
- coupling the selected matched sidecar to production cell-centered `J`;
- empirical Lorentz/common-cone recovery after mechanics exists.

## Next gate

The next campaign must promote the current from a prescribed history to an
observed consequence of a multi-tick matter trajectory. Starting from a
stationary dressed manifestation, it must derive current, matter impulse, and
field update from one registered discrete interaction, then test energy,
momentum, Gauss, locality, and inverse together. An independently appended
magnetic kick or fitted momentum normalization is banned; either would hide
the missing common action rather than supply it.

## Reproducibility

- campaign SHA-256:
  `D8128170B305D8BFFD5923040C5EE12DE69D4EF81ADB648DBD7F7EC95BD37723`
- helper SHA-256:
  `BA7B0CA7895D4DC5259527CCDCB06EC9B08DF7C4CB38AC8CDEDC31EFCD3FA62B`
- run CSV SHA-256:
  `51749C3064C07B932CE49E6A2C8AC321337F9A651C2F38812F4FEA1BB4129FB0`
- toolchain: MSVC `14.44.35207`, Release, CPU observer
- production dynamics: unchanged
