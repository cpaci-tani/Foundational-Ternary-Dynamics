# FTD-0668 — Causally isolated internal-recurrence discriminator v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Production status:** unchanged; observer-only large-buffer execution  
**Parent FTD-0665 JSON:**
`3D9C7F4601C4932458F351A1DE412A6E6E849E2514691C2C21093944BEE9B5B2`  
**Parent FTD-0666 JSON:**
`E89871BA5CE26D098AFB1063BD74084E6971D4E3426CCB4907009565AA9A0749`

## 1. Question and prediction

FTD-0665/0666 observe the first corrected internal-doublet return at ticks
`{73,76,76}` on periodic quotients `L={17,25,33}`. Their weak volume
dependence disfavors direct circumference scaling but does not exclude a
periodic-return explanation because every recorded return occurs after at
least one numerical circumference is causally accessible.

Run the same excitation in a quotient whose unused region is a causal buffer.
The prior-favoured prediction is that both polarity signs again cross the
unchanged return threshold during ticks `68..80`, with their first-return
times differing by at most two ticks. A return before the locked causal
self-contact time establishes a recurrence of the local coupled matter--field
system, not a signal returning through the periodic identification.

## 2. Frozen protocol

- Use `L=97`, horizon `T=80`, both signs of the FTD-0640 mode-6 momentum kick,
  and one same-volume unexcited control.
- Use the recentered FTD-0638 geometry, FTD-0665 actual paired tick-zero modal
  normalization, `8e-6` maximum constituent-momentum amplitude, selected
  connected-block common action, and no reactions or legacy forces.
- Use a default-off sparse storage of the unchanged quadratic face current so
  the empty causal buffer is not materialized once per constituent. Before the
  large run, compare one dense and one sparse `L=17` excited-state step;
  require both exact common-action gates and complete later-state difference
  `<=1e-10`. Sparse storage changes no current value or accepted root.
- The excited and control face/edge fields must be bitwise equal at tick zero;
  the only initial difference is the localized constituent momentum kick.
- At every forward step, the support of every deposited face-current segment
  in all three paths must remain inside periodic Chebyshev radius `R_s=8` of
  the fixed initial matter center. Segment support means a face-array entry
  that is exactly nonzero; its cell index is assigned the conservative radius
  `1+max(|dx|,|dy|,|dz|)` to cover the face offset. The matched curl and
  adjoint-curl stencils expand support by at most one site per tick.
- The locked earliest periodic self-contact time is
  `T_contact = L - 2 R_s = 81`. The observation horizon is strictly smaller.
- Define the first return exactly as in FTD-0665/0666: after the normalized
  doublet energy first falls below `0.60`, it later exceeds `0.80`.
- Record at every tick the normalized doublet energy, complete excited-minus-
  control face/edge energy and positive norm, near-field fraction inside
  radius 8, radial second moment, dynamic support radius, complete-energy
  drift, and common-action residual. Dynamic support uses positive squared
  face/edge difference greater than `1e-28` (amplitude threshold `1e-14`);
  its reported radius is the ceiling of periodic Chebyshev distance from the
  fixed initial matter center.
- Invert all three complete histories through 80 ticks. Require sector/fibre
  preservation, common residual and complete-energy drift `<=1e-10`, and
  accumulated state recovery `<=1e-8`.

No absorbing boundary, damping coefficient, source amplitude, return
threshold, time window, field normalization, or action parameter may be fitted
after looking at the `L=97` history.

## 3. Locked verdicts

- Any initialization, equality, locality, source-support, action, energy,
  dense/sparse equivalence, sector, inverse, schema, or horizon failure:
  `CAUSALLY_ISOLATED_RECURRENCE_EXECUTION_INVALID`.
- Execution passes; both signs return in ticks `68..80`, before tick 81, and
  differ by at most two ticks:
  `CAUSALLY_ISOLATED_INTERNAL_RECURRENCE_CONSTRUCTIVE`.
- Execution passes; both signs fall below `0.60` but neither returns by tick
  80: `NO_PRECONTACT_INTERNAL_RECURRENCE_IN_WINDOW`.
- Every other executable result:
  `CAUSALLY_ISOLATED_INTERNAL_RECURRENCE_MIXED`.

## 4. Interpretation boundary

A constructive verdict removes periodic self-return as the cause of the first
73--76-tick revival and establishes a local coupled-system recurrence in the
selected finite-support dynamics. It does not establish an infinite-support
normal mode, asymptotic stability, a quantum stationary state, a photon,
exponential decay, a resonance width, or the fate of energy at arbitrarily
late ticks. A no-recurrence verdict through tick 80 would support continued
outward transfer only over that finite causal window, not irreversible decay.
