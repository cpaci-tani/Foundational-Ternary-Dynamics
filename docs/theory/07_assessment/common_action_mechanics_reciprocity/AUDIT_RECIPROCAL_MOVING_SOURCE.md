# FTD-0477 — Reciprocal Moving-Source Discriminator

**Date:** 2026-07-25  
**Status:** `[MEASURED — DETERMINISTIC SELECTED-FORCE SUB-VOXEL RESPONSE]` +
`[CLOSED NEGATIVE — DYNAMIC MANIFESTED MOTION, DRESSING, WAKE, DETACHED FIELD, AND REGISTERED RECIPROCITY]`  
**Scope:** Windows/MSVC CPU production tick, `L=65`, periodic computational
boundary, 72 ticks, selected `G_C s grad|J|` force extension

## 1. Result

A spatially separate finite transverse flux packet gives an initially resting
polarity a deterministic, polarity-related mechanical response.  The two
mobile arms end with equal response magnitude

\[
|\Delta r_+|=0.20359848011794163,\qquad
|\Delta r_-|=0.20359848011794177.
\]

The source-only controls remain at rest to `1.65e-17`, the locked combined
control remains exactly fixed, and the exact repeat residual is zero.  The
response is therefore caused by the packet within the selected force branch;
it is not an initialized trajectory.

It does **not** become manifested lattice motion.  Neither polarity reaches
the locked `0.5`-cell displacement threshold and neither generates a
production movement event.  The run-of-record verdict is

```text
NO_DYNAMICAL_MOVING_SOURCE_IN_REGISTERED_PROTOCOL
```

This closes the moving-source interpretation for the registered protocol.  It
does not erase the measured continuous mechanical remainder or authorize a
stronger driver after seeing the result.

## 2. Scope restriction

The active accelerating channel is the toggle-gated production extension

\[
F_s=G_C s\,\nabla|J|_{r=2}.
\]

FTD-0435 already rejected this expression as ordinary electric `qE`: it is a
mixed-polarity self-field/interference response.  It is selected rather than
forced by the five postulates.  FTD-0477 therefore establishes a response
inside that extension only.  It does not establish native electromagnetism,
charge, a photon, a pilot wave, an aura, or radiation.

## 3. Locked controls and causation

| Observable | Observed | Locked gate | Result |
|---|---:|---:|---|
| driver activity near initial source | `2.2482263e-3` | `>1e-4` | pass |
| positive caused displacement | `0.2035984801` | `>=0.5` | **fail** |
| negative caused displacement | `0.2035984801` | `>=0.5` | **fail** |
| positive / negative movement events | `0 / 0` | `>=1 / >=1` | **fail** |
| locked-source displacement | `0` | `<=1e-12` | pass |
| maximum source-only displacement | `1.64063e-17` | `<=1e-9` | pass |
| exact-repeat residual | `0` | `<=1e-12` | pass |
| reactions in source arms | `0` | `0` | pass |

The positive and negative responses reverse their `x` and `y` components and
share the same `z` component.  This is a measured symmetry of the registered
selected-force history, not evidence that `s` has already become electric
charge.

The final speeds are about `5.10e-3`, and late interaction-force RMS is only
`0.003823` of the earlier peak for both polarities.  Those facts would be
consistent with a coast-like interval, but the preregistered coast label also
requires manifested motion.  The coast gate therefore fails and is not
promoted post hoc.

## 4. Field morphology

The matter-conditioned field is the observer subtraction

\[
(J_s,W_s)=(J,W)_{s+d}-(J,W)_d.
\]

At ticks 56 and 72 its translated correlation with the source-only field is
`0.999995` for both polarities.  That high correlation alone is insufficient:
the registered dressing gate also requires at least `75%` of activity within
radius four.

| Metric | Tick 56 | Tick 72 | Locked requirement | Result |
|---|---:|---:|---:|---|
| near fraction, positive | `0.437730` | `0.465416` | `>=0.75` at both | **fail** |
| near fraction, negative | `0.437730` | `0.465416` | `>=0.75` at both | **fail** |
| trailing fraction | `0.162015` | `0.170243` | `>=0.15` at tick 72 | pass |
| trailing / leading | `1.000000` | `1.000000` | `>=2` at tick 72 | **fail** |
| mean radius | `8.71784` | `9.81953` | growth `>=2.0` | **fail** (`1.10169`) |
| near-fraction change | — | `+0.027685` | drop `>=0.20` | **fail** |

The field is highly correlated with a translated source-only response, but it
is broad, front/back symmetric under the registered classification, and does
not detach quickly enough.  The dressing, wake, and detached-outgoing-field
labels all fail.

## 5. Registered reciprocity

The inclusion-exclusion audit gives worst normalized residuals

| Ledger | Positive | Negative | Locked gate | Result |
|---|---:|---:|---:|---|
| dynamic energy | `1.9606222e-6` | `1.9606222e-6` | `<=1e-6` | **fail** |
| particle + selected central field momentum | `2.4168494e-3` | `2.4165877e-3` | `<=1e-6` | **fail** |

The energy miss is about a factor of `1.96`; the momentum miss is about a
factor of `2.42e3`.  Thus even the continuous response does not close the
registered matter-field transaction.  Because the production audit omits
some candidate interaction energies, the correct statement is narrow:
**reciprocity is closed negative for the registered audit and selected
extension**.  The result is not a theorem that no enlarged common action could
close.

## 6. Dashboard qualification

`s0-seed-moving-source-reciprocity` is retained as *Driven Polarity —
Sub-voxel Response* under **Qualified Selected Extensions**.  It is a
qualified-negative research instrument, not a successful moving-source
scenario.  The dashboard enables distinct views of `J`, ternary state,
`-wave_vel`, and Poynting-like flow, while the metadata records the failed
labels.

The WASM particle sampler now renders `cell coordinate + remainder`, matching
the engine's continuous mechanical position.  Previously it rendered only the
cell centre, hiding every sub-voxel response until an integer hop occurred.
This is a visualization correction only; it does not change the production
tick, source trajectory, force, or campaign result.

## 7. Consequence

FTD-0476 asked for dynamically caused motion rather than a prescribed source.
FTD-0477 supplies a genuinely caused continuous response but does not cross
the manifestation threshold.  The frozen protocol therefore cannot be used
to argue for a co-moving aura or wake.

A new campaign may investigate response scaling with driver amplitude,
duration, and source separation only if those variations are preregistered as
a new protocol.  Such a campaign would characterize the selected force law;
it would still not make that law native or electromagnetic.  The deeper open
problem remains a common-action matter-field rule with closed energy and
momentum exchange.

## 8. Reproducibility

- preregistration:
  `PREREG_RECIPROCAL_MOVING_SOURCE_DISCRIMINATOR_v1.md`;
- preregistration SHA-256:
  `88835A61876F023F3FE76D841863D8FDBF087E201B35BAAEE22C999ACE5D2573`;
- campaign: `campaign_reciprocal_moving_source`;
- scenario gate: `reciprocal_moving_source_scenario`;
- browser contract: `scale0-reciprocal-moving-source.spec.js`;
- record: `engine/results/ftd_0477/windows_msvc_cpu_L65.csv`;
- verdict: `engine/results/ftd_0477/verdict.txt`.

No production tick rule, constant, event order, RNG stream, or default toggle
was changed.
