# FTD-0661 — Internal-mode action-transfer ledger v2

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Production status:** unchanged; observer-only correction campaign  
**Parent v1 JSON:**
`08CA4F43FD8E35C5ED596D7379A8EF0EFA931D32BF7A4A7F2E7FB8F3F4CA2D15`

## 1. Corrections and nothing else

FTD-0660 passes exact execution, transfer detection, amplitude/sign controls,
and the full dynamic-field morphology conjunction. Its final verdict is mixed
because two observer controls were incorrectly specified:

1. the zero threshold `1e-20` lies below the locked Poisson-redressing
   observer's measured numerical floor `2.78e-17`, despite complete energy
   remaining exact to `2.67e-15`;
2. cyclic covariance compared the same coordinate vectors inside independently
   diagonalized degenerate doublets. An eigenbasis inside a degenerate
   eigenspace is arbitrary, so that comparison is not covariant.

V2 changes only those controls. The field decomposition, shell boundaries,
tick count, amplitudes, momentum quadratures, transfer thresholds, morphology
thresholds, exact gates, and verdict names remain unchanged.

## 2. Covariant polarization control

Replace the v1 two-vector sample by the four-vector tight frame

\[
\mathcal U=\left\{(1,0),(0,1),{(1,1)\over\sqrt2},
{(1,-1)\over\sqrt2}\right\},
\qquad \sum_{u\in\mathcal U}u u^T=2I_2.
\]

Run all four polarizations in each independently diagonalized cyclic
orientation. For quadratic ledger observables, compare the polarization-summed
history between orientations. The tight-frame sum is invariant under any
orthogonal basis change in the degenerate doublet.

Keep amplitudes `4e-6,8e-6`, quadratures `pi/2,3pi/2`, two orientations, and
`128` forward plus `128` inverse ticks. V2 therefore has
`2*4*2*2=32` nonzero arms plus two zero controls: `34` total.

## 3. Corrected zero control

The exact state remains the zero physical perturbation. The independent
redressing observer solves to `1e-13` field residual and is not bit-identical
to the stored dressing. Set the observer zero gate to `1e-14`, still two orders
tighter than the ledger's `1e-12` energy gate and more than two orders above
the measured v1 floor. No dynamics or physical tolerance changes.

## 4. Locked gates and verdicts

Retain every FTD-0660 gate except:

- zero perturbation-energy/norm maximum `<=1e-14`;
- amplitude and sign controls remain per polarization;
- cyclic covariance is the normalized residual between tight-frame-summed
  quadratic ledger histories and must be `<=5%`.

The same classifications apply:

- `INTERNAL_MODE_DYNAMIC_FIELD_TRANSFER_CONSTRUCTIVE`;
- `INTERNAL_MODE_LOCAL_HYBRID_TRANSFER_CONSTRUCTIVE`;
- `INTERNAL_MODE_ACTION_TRANSFER_MIXED`;
- `INTERNAL_MODE_ACTION_TRANSFER_CLOSED_NEGATIVE`;
- `INTERNAL_MODE_ACTION_TRANSFER_EXECUTION_INVALID`.

No v1 verdict is changed. V2 is a fresh execution. Even a constructive dynamic
field result establishes only outward redistribution of the instantaneous-
dressing residual on this finite periodic volume. It does not establish an
asymptotic photon, irreversible decay, or infinite-volume resonance.
