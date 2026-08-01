# FTD-0723 — Field-assisted capture-window v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE VALIDATION RUN]`  
**Identifier:** `FTD-0723`  
**Date:** 2026-07-29  
**Parents:** `FTD-0721`, `FTD-0722`  
**Scope:** observer-only incident-energy discriminator for the unchanged
derived compact-pair/matched-field action; no production state, default,
toggle, scenario, interaction, tolerance, particle identity, or ontology
promotion.

## 1. Question locked before validation

Does the exact encounter-to-field energy transfer measured by FTD-0722 create
a reproducible incident-momentum capture window, or does the transferred
energy shrink or remain insufficient when the incoming pair energy is lowered?

The action is frozen. In particular, this campaign may not alter the compact
well, face-current coat, field normalization, field update, orbit gather,
solver, timestep, volume, horizon, or capture/radiation classifiers after
inspection.

## 2. Parent lock and threshold prediction

The parent FTD-0722 action is identified by:

- connected-action header SHA-256
  `DAC2DC83A7366EB5856B008613079E2FB8100A05D4C38ACC23B8C145DD03D65E`;
- connected-action source SHA-256
  `0B64BB431DCA847AE03321BF983D1023AD40CDE33D5B49DED0E2A14B6664337C`;
- FTD-0722 runner SHA-256
  `C694C32D5428F0A09B6F12A58FD91EDD7940A27ED53F6A3BDD35036BDCB58537`;
- FTD-0722 JSON SHA-256
  `1AAE192D20C5B745D079307B7A3C64B394C9C15ED5E168FF3B1DD2DBFC85E582`.

FTD-0722 measured one-pass field-energy gains

\[
\Delta E_f\in[0.0012012704657176076,
               0.001374812629945682]
\]

at incoming momentum `p=0.07`. Outside the compact support, the pair internal
energy is exactly

\[
K_{\rm pair}(p)=2\left(\sqrt{E_{\rm REST}^2+C_{\rm SPEED}^2p^2}
-E_{\rm REST}\right),
\qquad E_{\rm REST}=0.511/3.
\]

The zero-order constant-export model therefore places the threshold bracket at

\[
p_-=0.024797812323480179,
\qquad p_+=0.026531996461401596.
\]

This is a preregistered model prediction, not an assumption that field export
is momentum-independent. Its failure is an informative result.

## 3. Frozen momenta and predictions

Use five unbound incoming momenta:

| momentum | exact initial pair energy (numeric) | zero-order prediction |
|---:|---:|---|
| `0.0200` | `0.000781881596627454` | capture |
| `0.0225` | `0.000989268124136855` | capture; held-out interior point |
| `0.0250` | `0.001220904197076655` | transition band; no sign prediction |
| `0.0275` | `0.001476740563563506` | escape; held-out exterior point |
| `0.0300` | `0.001756722980970848` | escape |

The held-out labels mean that `0.0225` and `0.0275` are not threshold endpoints
and cannot be used to redefine the window after output. No momentum sweep,
adaptive bisection, or post-hoc replacement is allowed.

## 4. Locked campaign

Retain FTD-0722 exactly:

- `L=33`, `dt=1/4`, 24 forward and 24 state-only reverse steps;
- separation `1.30` for every unbound arm;
- all 13 unoriented Moore rays;
- both polarity orders;
- lattice-center and translated `(4,-3,2)` copies;
- minimum-energy periodic longitudinal initial face field, `B=0`, CG
  tolerance `1e-13`, at most 4096 iterations;
- canonical FTD-0468/0479 interaction normalization;
- exact quadratic-coat currents and matched face/edge field update;
- selected FTD-0721 compact well with depth `0.01` and squared cutoff `1.5`;
- common-action gate `1e-10`, reverse recovery `1e-8`, scalar-history
  translation/polarity gate `1e-9`, symmetric recoil gate `1e-9`.

The five unbound momenta give `5 x 13 x 2 x 2 = 260` histories. Repeat the
52 FTD-0722 already-bound controls at separation `1.00`, momentum `0.015`.
Total: 312 complete forward/reverse histories.

## 5. Unchanged classifiers

An unbound arm is captured only if it starts outside the graph with positive
pair energy, enters once and never exits, remains inside and below `-1e-6` for
the final eight steps, balances field gain against pair loss within `1e-8`,
has dynamic-field norm `>1e-8`, magnetic energy `>1e-10`, and dynamic-field
median doubled radius at least four.

Also record `negative_sector` before applying the outgoing-field morphology
gate. This distinguishes energetic capture from qualified separation of the
receiver field. Already-bound controls must remain inside and negative for the
final eight steps.

For each momentum report capture fraction, negative-sector fraction, graph
transitions, energy export, final pair energy, dynamic-field norm, magnetic
energy, and median radius. Capture ordering is monotone only if no higher
momentum has a larger capture fraction than a lower momentum.

## 6. Locked verdict map

- All atomic gates and bound controls pass; `p=0.0200` and held-out `0.0225`
  capture in 52/52 arms; `p=0.0275` and `0.0300` escape in 52/52 arms:
  `PREDICTED_CAPTURE_WINDOW_CONFIRMED`. The `0.0250` transition-band outcome
  is reported but does not choose this verdict.
- Atomic gates and controls pass; at least one momentum has qualified capture;
  capture fractions are monotone, but the locked low/high predictions are not
  all satisfied: `CAPTURE_WINDOW_SHIFTED_OR_DIRECTIONAL`.
- At least one arm reaches the negative sector but no arm passes the unchanged
  outgoing-field gate: `NEGATIVE_SECTOR_WITHOUT_DETACHED_FIELD`.
- Atomic gates and controls pass; no unbound arm reaches the negative sector:
  `NO_CAPTURE_WINDOW_OBSERVED_LOCKED_V1`.
- Capture fractions are nonmonotone in incident momentum:
  `NONMONOTONE_CAPTURE_RESPONSE`.
- Any already-bound control fails:
  `DERIVED_PAIR_BOUND_STATE_UNSTABLE_IN_WINDOW_CAMPAIGN`.
- Any root, current, Gauss, common-action, energy, inverse, symmetry, or recoil
  gate fails: `CAPTURE_WINDOW_TRANSACTION_UNRESOLVED`.

Only qualified capture supports a formation window. Even the strongest
positive verdict would remain a selected classical formation witness, not a
physical particle, quantum bound state, derived electromagnetic law, or
production adoption.
