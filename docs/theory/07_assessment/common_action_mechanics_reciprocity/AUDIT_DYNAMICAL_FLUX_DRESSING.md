# FTD-0476 — Dynamical Flux Dressing / Wake / Release Audit

**Date:** 2026-07-25  
**Status:** `[MEASURED — POLARITY-GENERATED DYNAMICAL FIELD]` +
`[CLOSED NEGATIVE — LOCKED RADIAL-DRESSING, ATTACHMENT, WAKE, AND RELEASE LABELS]`  
**Scope:** deterministic CPU production tick, restricted native
`wave_propagation + coupling` sector, periodic computational windows
`L={49,65}`

## 1. Result

A manifested polarity placed into an exactly zero `J,W` field dynamically
creates a finite, polarity-odd flux response through the existing native source
term

\[
\Delta_t^2J=C_{\rm wave}^2\Delta_{18}J-G_C\nabla_c s.
\]

That statement survives.  The stronger preregistered labels do not.  At both
volumes the run-of-record verdicts are:

```text
source_verdict=NO_QUALIFIED_SOURCE_BUILT_DRESSING
movement_verdict=NO_QUALIFIED_ATTACHMENT_OR_WAKE
release_verdict=NO_QUALIFIED_RELEASED_OUTGOING_FIELD
```

The honest current description is **a polarity-generated, retarded dynamical
flux field with a large trailing response under prescribed movement**.  It is
not yet a qualified co-moving aura, wake, emitted radiation field, photon,
electromagnetic field, or pilot-wave mechanism.

## 2. Scenario admission

The new dashboard scenario is
`s0-seed-dynamical-flux-dressing` (*Dynamical Flux Dressing — Native Source
Probe*).  It starts with one locked central `s=+1`, exactly zero `J` and
`wave_vel`, periodic flux boundary, and only the native wave/coupling terms on.

The behavioral admission test proves that the first production tick creates
exactly the six face-neighbour vectors required by the centered-gradient
source.  Each vector has outward magnitude `G_C/2`, the source remains fixed,
and no matter is created or moved.  Later support stays inside the tick
dependency cone.  The browser mirror loads the same zero-field source and
automatically exposes state, divergence, volume, slice, and flux-line views.
The rendered lines are integral curves of `J`; they are not additional
substrate objects. Any user change to the registered physics toggles now
visibly suspends the dashboard qualification until the scenario profile is
restored; the broad engine defaults are not silently substituted.

## 3. Locked measurements

All primary numbers are identical at `L=49` and `L=65` through tick 24, so the
registered window has no detected boundary contamination.

| Arm / metric | Observed | Locked gate | Result |
|---|---:|---:|---|
| empty activity, tick 12 | `0` | `<=1e-15` | pass |
| source activity, tick 12 | `0.0138019851` | `>1e-8` | pass |
| polarity mirror residual | `0` | `<=1e-12` | pass |
| source radial alignment, tick 12 | `0.737595227` | `>=0.75` | **fail** |
| signed source divergence, tick 12 | `0.148643302` | `>0` | pass |
| production movements / reactions | `6 / 0` | `>=4 / 0` | pass |
| moving final near activity | `0.112280366` | `>=0.005925479` | pass |
| moving final radial alignment | `0.431193847` | `>=0.50` | **fail** |
| moving trailing fraction | `0.149976917` | `>=0.15` | **fail** |
| trailing / leading activity | `483.18` | `>=2` | pass |
| release mean-radius growth | `1.86443585` | `>=2.0` | **fail** |
| release near-fraction drop | `0.193743399` | `>=0.20` | **fail** |
| release exact-energy drift, worst | `1.35023e-13` | `<=1e-10` | pass |

The misses are numerically narrow but remain failures because the cutoffs were
locked before execution.  No post-hoc threshold relaxation is admissible.

## 4. What the dynamics actually show

### 4.1 Stationary polarity

The field is generated from zero rather than supplied by an initializer.  The
empty control stays exactly zero, `s=+1` and `s=-1` histories are exact odd
mirrors, and sign-corrected source divergence is positive.  This establishes a
native dynamical source response.  Its tick-12 morphology is not radial enough
for the registered *radial dressing* label.

### 4.2 Moving polarity

The production source executes six positive-x face hops.  The repaired journal
records those same six movements and no reactions, exactly matching the
primitive displacement.  Final activity rises to `0.158745165`, about `11.50`
times the stationary tick-12 activity.  `70.73%` remains within radius four of
the current source, but the field is not sufficiently radial there.  Activity
behind the source is about `483` times activity ahead, yet its fraction
`0.149976917` misses the locked `0.15` wake gate.

The strong amplification prevents a conservative interpretation.  The source
velocity is prescribed and the frozen arm contains no reciprocal force/work
closure, so the movement can pump the flux field.  The result is evidence for
retarded source-history dependence, not evidence for radiation or a stable
co-moving material coat.

### 4.3 Source removal

After the declared source-off intervention, manifested count remains zero and
the exact source-free tick invariant is conserved to `1.36e-13`.  Mean radius
increases from `2.12616` to `3.99060`, while the near fraction falls from
`0.858642` to `0.664898`.  This is conservative outward redistribution of the
field, but both registered morphology gates fail narrowly.  The run therefore
does not license the label *released outgoing field*.

## 5. Instrumentation correction

Version 1 read the immutable event journal only after the complete moving run.
The bridge clears that observer journal at the beginning of each tick, so v1
reported zero movements even though the primitive source position advanced by
six sites.  Its morphology and field results are preserved, but its movement
classification is invalid.

Version 2 changes only the sampling time: it accumulates events immediately
after each completed tick.  It also requires the accumulated count to equal the
measured displacement.  All thresholds, engine rules, scenarios, fields, and
estimators are unchanged.  The corrected record contains six movements, zero
reactions, and `structural_valid=1`.

## 6. Ontological boundary and next discriminator

The engine presently supports this narrow story:

> Manifested polarity acts as a local source for a continuous dispositional
> flux field.  That field propagates with memory, remains partly concentrated
> near a moving source, and redistributes after source removal.

It does not yet support the stronger story that an object owns a stable aura or
that motion sheds a physical wake.  Those claims require one reciprocal
multi-tick matter-field transaction in which source motion, field growth,
particle work, field energy, momentum, and any detached outgoing component are
closed together.  A prescribed moving source is insufficient because its
external work is not measured.

The next legitimate probe is therefore not a threshold tweak.  It is the same
source/wake morphology observer applied to a common-action mobile history whose
motion is dynamically caused and whose matter-field energy transaction is
closed.  Only the baseline-subtracted detached component may be called a wake
or radiation candidate.

## 7. Reproducibility

- preregistrations:
  `PREREG_DYNAMICAL_FLUX_DRESSING_v1.md` and
  `PREREG_DYNAMICAL_FLUX_DRESSING_v2.md`;
- campaign: `campaign_dynamical_flux_dressing`;
- scenario test: `dynamic_flux_dressing_scenario`;
- browser test: `scale0-dynamical-flux-dressing.spec.js`;
- records: `engine/results/ftd_0476/dynamical_flux_dressing_v1.csv`,
  `verdict_v1.txt`, `dynamical_flux_dressing_v2.csv`, `verdict_v2.txt`, and
  `manifest.json`;
- v2 source lock:
  `D24A9EC9051B98313E1D5BC3645A5635151DAFB1A365665A04D6D89F28A33BCE`.

No production tick rule, constant, event order, RNG stream, or default toggle
was changed.
