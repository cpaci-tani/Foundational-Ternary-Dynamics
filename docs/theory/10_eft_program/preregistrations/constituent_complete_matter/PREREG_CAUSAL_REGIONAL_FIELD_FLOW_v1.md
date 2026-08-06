# FTD-0672 — Causal regional field-flow discriminator v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Production status:** unchanged; observer-only large-buffer execution  
**Parent FTD-0670 JSON:**
`631BFCD005E5B223641260F8D1A59442EAFDFCF88565B8EEDBEAC8E4F228DC10`  
**Parent FTD-0670 tick CSV:**
`8C3CBCDAC9137114B2A17202FA04FF77362465D13BE94636C19493A1A31F347A`  
**Observer theorem:** `FTD-0671`

## 1. Question and prediction

FTD-0665, FTD-0668, and FTD-0670 show a distributed dynamic difference field
coexisting with an amplitude-stable internal-envelope turn before periodic
self-contact. Radial growth and a falling positive-norm near fraction do not
determine the direction of energy transfer. FTD-0671 now supplies the exact
regional ledger needed to distinguish source exchange from boundary flow.

Run the same causal construction at one further held-out half amplitude. The
prior-favoured prediction is that substantial energy crosses outward through
radius 24 before tick 68 and a non-negligible inward transfer or negative
dynamic-field/current exchange occurs during the fixed tick-68--80 recovery
window. The result may instead be one-pass outgoing flow, near-bound dressing,
or mixed. None of those classes is called radiation or a photon.

## 2. Frozen protocol

- Use `L=97`, horizon `T=80`, fixed initial-center regions, source-radius
  limit eight, contact tick 81, the same-volume unexcited control, recentered
  FTD-0638 geometry, paired modal normalization, selected connected-block
  common action, and the default-off exact sparse-current path.
- Use both signs of the mode-6 momentum kick at maximum constituent momentum
  amplitude `2e-6`, one half of FTD-0670. Initial face/edge fields must be
  bitwise equal to the unexcited control.
- The control and two sign histories may run concurrently. Each path keeps an
  independent state and solve cache; within-path arithmetic and transaction
  order stay serial; verdict reduction is serial.
- Require the parent fingerprints above, valid normalization and mode basis,
  horizon below contact, sector/fibre preservation, source support at most
  eight, common residual and complete-energy drift `<=1e-10`, state-only
  recovery `<=1e-8`, and a valid FTD-0671 observer at every tick and radius.
- Form exact excited-minus-control face/edge fields before and after every
  step. Reconstruct the difference-field source-free intermediate state by

  ```text
  B1 = B0-lambda C^T E0,
  E* = E0+lambda C B1.
  ```

  Then pass `(E0,B0,E*,B1,E1)` to FTD-0671.
- Use fixed component-aware periodic Chebyshev radii `R={8,16,24}` about the
  initial control center. Do not move the regions with a fitted centroid.
- Normalize every regional energy, transport, and source term by the sign
  arm's tick-zero paired-doublet energy, including the mapped field-work
  coefficient `beta`.
- At every tick and radius record regional energy before, pre-current, and
  after; signed source-free boundary transport `T_R` into the region; current
  exchange `S_R` into the dynamic difference field; partition/ledger/update
  residuals; doublet ratio; positive-norm near fraction; radial second moment;
  source support; complete-energy drift; and common-action residual.

Define, for each sign and radius,

```text
O_R = sum_{t=1}^{67} max(-T_R(t),0),
I_R = sum_{t=68}^{80} max(+T_R(t),0),
N_R = -sum_{t=1}^{80} T_R(t),
X_R = sum_{t=68}^{80} S_R(t).
```

All quantities above are normalized. Because current support is required
inside radius eight with a gap to every registered boundary, require the three
`X_R` values to agree within `1e-10` for each sign.

Retain the FTD-0670 strict-trough classifier at this new amplitude. Require a
primary trough at ticks `71..73`, three descending earlier troughs, two
ascending later troughs, and second-post-trough recovery at least `0.05` for a
reproduced turn.

Between signs require every `O_R,I_R,N_R,X_R` to agree within `1e-4`, primary
tick within one, primary ratio and recovery increment within `1e-4`, and final
positive-norm near fraction within `1e-4`.

No radius, time window, sign convention, normalization, threshold, or outcome
definition may change after viewing the `2e-6` history.

## 3. Locked factorized classes

Transport class:

- `BIDIRECTIONAL_CAUSAL_FLOW` if `O_24>=0.05` and at least one registered
  radius has `I_R>=0.01` and `I_R/O_R>=0.05`.
- `ONE_PASS_OUTGOING_FLOW` if `O_24>=0.05` and every registered radius has
  `I_R<=0.001` and `I_R/O_R<=0.01` (with a zero denominator treated as a
  failed condition).
- `NEAR_BOUND_FLOW` if the tick-80 positive-norm radius-eight near fraction is
  at least `0.50` and `N_24<=0.01`.
- `REGIONAL_FLOW_MIXED` otherwise.

Recovery-window current-exchange class, using the common `X_R` value:

- `DYNAMIC_FIELD_TO_CURRENT` if `X_R<=-0.01`;
- `CURRENT_TO_DYNAMIC_FIELD` if `X_R>=+0.01`;
- `RECOVERY_EXCHANGE_BALANCED` otherwise.

## 4. Locked verdicts

- Any fingerprint, initialization, exact-observer, locality, source-exchange
  consistency, action, energy, sector, inverse, schema, polarity, or horizon
  failure:
  `CAUSAL_REGIONAL_FIELD_FLOW_EXECUTION_INVALID`.
- Execution and turning pass, transport is bidirectional, and recovery exchange
  is `DYNAMIC_FIELD_TO_CURRENT`:
  `CAUSAL_BIDIRECTIONAL_FIELD_MEMORY_CONSTRUCTIVE`.
- Execution and turning pass, transport is one-pass outgoing:
  `CAUSAL_ONE_PASS_OUTGOING_FIELD_CONSTRUCTIVE`.
- Execution and turning pass, transport is near-bound:
  `CAUSAL_NEAR_BOUND_DRESSING_CONSTRUCTIVE`.
- Execution passes but none of the three constructive conjunctions closes:
  `CAUSAL_REGIONAL_FIELD_FLOW_MIXED`.

## 5. Interpretation boundary

The observer measures exact energy transfer in the dynamic difference-field
equations under the FTD-0671 symmetric regional allocation. Bidirectional flow
plus negative recovery-window current exchange would support a causal field-
memory feedback mechanism for the tested classical hybrid. One-pass flow would
instead show that the internal-envelope turn is paid by local matter/binding
dynamics while a portion of the field departs. Near-bound flow would support a
co-moving dressing interpretation.

No outcome establishes asymptotic radiation, a photon, a quantum pilot wave,
an infinite-volume bound state, a positive-residue pole, charge, species, or
production ontology. A mixed verdict may not be repaired by changing this
classifier on the same data.
