# Theorem — Relative action/orientation transducer boundary (FTD-0860)

**Status:** `[THEOREM — EXACT NONZERO-CARRIER ACTION PUMP]` +
`[THEOREM — PUNCTURED-PAIR SYMPLECTICITY AND TIME-REVERSAL COVARIANCE]` +
`[THEOREM — EMPTY-CARRIER ROTATION-EQUIVARIANCE OBSTRUCTION]` +
`[THEOREM — ONE-PAIR SIGN-FAITHFULNESS OBSTRUCTION]` +
`[SELECTION — SIGNED QUARTER-TURN RESPONSE]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]` +
`[CLOSED NEGATIVE — COMPLETE FAITHFUL TRANSDUCER IN ONE UNLABELLED PAIR]` +
`[OPEN — LOSS LEDGER OR RESERVED SIGNED RAIL, LOCALIZATION, EXPORT, AND CONTROLLER]`  
**Date:** 2026-08-11  
**Repaired protocol:**
[`PREREG_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_CERTIFICATE_REPAIR_v2.md)  
**Invalid parent:**
[`PREREG_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_v1.md)  
**Parent pre-run SHA256:**
`FC558B798D556812A12FE239264B99855A7753EDD8DCAA66891B906BBB4DD351`  
**Repair pre-run SHA256:**
`9971EEE4B0E532F7732AAE35151DF284C55EF95DBD1C26D8B42CE7F7A9BEC11F`  
**Certificate:**
`scripts/proofs/proof_relative_action_orientation_transducer_v2.py`, SHA256
`E174EDE70863EC68FA27765EFAF61C8C849DD66EC7B91D21294ED4E23D6BC53B`,
`36/36 PASS`

## 1. Result

A locally accepted signed event can transfer its released energy `B` into an
already nonzero relative canonical carrier without first clearing that carrier
to zero. The exact reference map is a signed quarter-turn followed by the
unique radial gain that increments action by `B`.

That result does **not** close a faithful history transducer. On an arbitrary
background, one unlabelled pair cannot retain the old phase together with the
erased sign and separately identifiable event energy. This is not a defect if
unactualization is declared to be lossy; it is a theorem-level type boundary if
the framework instead requires microscopic event recovery.

## 2. The canonical pair and `i`

Use a real canonical pair and its complex structure

\[
 z=\binom q p,qquad
 I=\frac{q^2+p^2}{2},qquad
 J=\begin{pmatrix}0&-1\\1&0\end{pmatrix}.       \tag{1}
\]

Then

\[
 J^2=-\mathbf 1,qquad J^TJ=\mathbf 1,qquad \det J=1.       \tag{2}
\]

This is the exact real meaning of multiplication by `i`: `J` is a quarter-turn
operator on a pre-existing conjugate pair. It is not a second state value, a
Hilbert vector, or a source of energy. The two signed orientations are

\[
 J_s=sJ,qquad s\in\{-1,+1\}.                    \tag{3}
\]

Thus `i` supplies orientation. Energy conservation supplies the radial gain;
the two roles must not be conflated.

## 3. Exact nonzero-carrier action pump

For a released event energy `B>0` and `I>0`, define

\[
 \rho_B(I)=\sqrt{\frac{I+B}{I}},qquad
 z'=F_{s,B}(z)=\rho_B(I)sJz.                     \tag{4}
\]

Since `sJ` is orthogonal,

\[
 I' = \frac{\lVert z'\rVert^2}{2}
     = \rho_B(I)^2 I
     = I+B.                                      \tag{5}
\]

Conversely, any positive radial gain with the frozen action increment must
satisfy `rho^2 I=I+B`, so equation (4)'s gain is unique in that class. In
action-angle coordinates the map is

\[
 I'=I+B,qquad \theta'=\theta+s\frac{\pi}{2}.    \tag{6}
\]

The `+/- pi/2` choice is `[SELECTED]`. Once that response is selected, every
other factor in (4) is fixed by the local input and exact energy increment. The
gain `rho_B` is not a Lorentz factor and no relativistic `gamma` is derived.

### 3.1 Canonical structure

On the punctured plane `I>0`, direct differentiation gives

\[
 \det\frac{\partial(q',p')}{\partial(q,p)}=1,
 \qquad \{q',p'\}=1.                             \tag{7}
\]

For known `(s,B)`, the inverse on `I'>B` is

\[
 z=-sJ\sqrt{\frac{I'-B}{I'}}\,z'.               \tag{8}
\]

This known-event inverse proves that the pump itself does not destroy the old
pair when its control data are retained. It does not prove that those control
data can be inferred from the unlabelled output.

Let canonical time reversal be `K=diag(1,-1)`. Then

\[
 K(sJ)K=(-s)J.                                   \tag{9}
\]

The two signed quarter-turns therefore exchange under time reversal. Every
planar rotation commutes with `J`, and the gain depends only on rotationally
invariant action, so (4) is rotation equivariant on its declared domain.

## 4. Why the carrier cannot start at zero

The map has no rotation-equivariant positive-energy extension at `z=0`.
Because zero is fixed by every rotation, equivariance requires `F(0)` to be
fixed by every rotation. The only such vector is zero. But exact event-energy
transfer requires

\[
 \lVert F(0)\rVert^2=2B>0.                      \tag{10}
\]

Equivalently, approaching zero along different input phases gives different
points on the radius-`sqrt(2B)` output circle. No unique continuous limit
exists.

This is the clean answer to the “more than matter” question: an oriented event
cannot create an orientation-covariant phase from an exactly empty isotropic
pair. It needs one of:

1. a persistent nonzero carrier phase;
2. a local directional/clock anchor;
3. an explicitly selected symmetry-breaking direction; or
4. a separate signed receiver rail.

No claim that production vacuum already supplies item 1 is licensed.

## 5. Why the action pump is lossy

The two sign branches collide exactly:

\[
 F_{+,B}(z)=F_{-,B}(-z).                         \tag{11}
\]

Moreover, output action records only `I+B`; it does not separate prior carrier
action from event energy. For fixed `I>0`, a general continuous exact-shell
sign-faithful map would need to embed two disjoint input circles into the one
output circle at action `I+B`. By invariance of domain in one dimension, a
continuous injection of a circle into a circle has open and closed image and
is therefore onto. Both sign images would be the whole output circle and must
intersect.

Accordingly:

- **lossy branch:** equation (4) is a valid deterministic reduced map. Energy
  and an orientation response persist, while the event identity need not be
  recoverable from the pair alone;
- **faithful branch:** a separate signed coordinate is necessary in the
  registered continuous exact-shell class.

The minimal signed amplitude already proved by FTD-0852 is

\[
 a=s\sqrt{2B},qquad s=\operatorname{sign}(a),qquad B=a^2/2. \tag{12}
\]

Equation (12) carries both sign and energy. But if it is assigned its quadratic
energy, using it together with the action increment (5) would deposit `2B`.
The two constructions are alternatives, not cumulative corrections:

- action pump = lossy energy mixing into a nonzero carrier;
- signed rail = faithful event history with recursive port clearing.

This branch distinction implements the user's statement that some details may
be genuinely irrelevant and lost. The framework must still name which fields
are lossy; it cannot call an unbooked disappearance conservation.

## 6. Stability and recursion

Repeated pump events obey the immediate corollary

\[
 I_N=I_0+\sum_{n=0}^{N-1}B_n.                   \tag{13}
\]

For positive events, the isolated pump is therefore not a bounded clock or a
stable recursive system. A maintained carrier requires an outgoing transaction

\[
 I_{n+1}-I_n=B_n-E_{{\rm out},n}-W_{{\rm loss},n},            \tag{14}
\]

where every term is separately defined and signed. FTD-0852 supplies an exact
selected one-cell export rail. Production C18 is dispersive rather than that
rail, so it still needs either a protected-mode realization or a preregistered
finite-window clearing/export theorem.

## 7. Production boundary

FTD-0858 proves that production already has target-blind event acceptance but
that its common-field trigger cannot determine the relative port. FTD-0860
does not erase that kernel; it gives a lawful map **after** `(s,B)` and a
nonzero relative canonical carrier are supplied.

Production still lacks:

1. a local phase identifying a protected nonzero canonical receiver;
2. an event hook applying (4), or a reserved signed rail applying (12);
3. separate common/relative energy and face-current accounting;
4. a declaration of which erased labels are intentionally lossy;
5. a controller-work and maintenance account; and
6. a bounded export/clearing theorem for repeated events.

No `Voxel`, toggle, tick phase, default, or production energy term changed.

## 8. Isolated implementation

The reference API is
[`relative_action_transducer.h`](../../../../../engine/include/ftd/eft/relative_action_transducer.h),
SHA256
`E4E7C237D7AF7BB3B3000CFAC4D63C0E8126801422EF43809754BAF086400D42`.
Its implementation and focused test are:

- [`relative_action_transducer.cpp`](../../../../../engine/src/eft/relative_action_transducer.cpp),
  SHA256
  `B2A5EDD8F00931B5384D924DE5BAE3CC3F0A64AE5AD9B494127201D5E106AF67`;
- [`test_relative_action_transducer.cpp`](../../../../../engine/tests/test_relative_action_transducer.cpp),
  SHA256
  `9E202E5B36E5DCE23FFF117EC2F16E862C90D9133553A5799DAB9CB0AB24EF9F`.

The API fails closed on an empty carrier, invalid sign/energy, nonfinite input,
and points outside the strict known-event inverse image. The focused Release
CTest passes `1/1` and reports:

```text
FTD-0860 relative action transducer EFT: PASS
scope=NONZERO_CARRIER_LOSSY_ACTION_PUMP
empty_carrier=REJECTED
faithful_signed_history=SEPARATE_RAIL_REQUIRED
production_integration=NONE
```

## 9. Certificate record

The FTD-0859 parent passed all frozen source hashes but returned `31/36` due to
five verifier defects. It is preserved invalid and books no theorem. FTD-0860
applied exactly the five preregistered repairs in memory and returned:

```text
FTD-0859 relative action/orientation transducer: 36/36 PASS
NONZERO_RELATIVE_CARRIER_ADMITS_EXACT_TARGET_BLIND_ACTION_PUMP
SIGNED_QUARTER_TURN_PUMP_IS_SYMPLECTIC_AND_TIME_REVERSAL_COVARIANT
EMPTY_ISOTROPIC_PAIR_HAS_NO_POSITIVE_ROTATION_EQUIVARIANT_EXTENSION
ONE_UNLABELLED_PAIR_CANNOT_FAITHFULLY_RETAIN_EVENT_AND_BACKGROUND
LOSSY_ACTION_MIXER_AND_FAITHFUL_SIGNED_RAIL_ARE_DISTINCT_BRANCHES
VERDICT=OUTCOME_B_EXACT_REFERENCE_PUMP_FAITHFULNESS_BOUNDARY
```

No Born, Bell, Hilbert-recovery, biological, CM/substrate, `G*` cadence,
thermodynamic, local-vacuum, production-integration, or completeness claim is
made.

