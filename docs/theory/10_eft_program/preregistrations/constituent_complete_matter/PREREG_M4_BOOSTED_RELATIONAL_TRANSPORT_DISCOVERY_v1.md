# FTD-0761 — M4 boosted relational transport discovery v1

**Status:** `[PRE-REGISTRATION — FROZEN BEFORE IMPLEMENTATION; NOT RUN]`  
**Date:** 2026-07-31  
**Parent:** FTD-0760 `M3_FINITE_TIME_SELECTED_MATTER_FAMILY`  
**Scope:** existence of coherent translated histories in the certified
relational family; no native acceleration, field-driven recoil, wake,
particle, mass, pole, Lorentz, or production claim

## 1. Locked question

Does the unchanged reciprocal two-polarity common action admit a coherent
secularly translating history when a known collective momentum is supplied as
an explicit initial-state intervention?

FTD-0761 is a discovery campaign. It does not qualify a boost mechanism and
does not replace the boost with a field packet. A constructive result licenses
a separately frozen field-drive campaign; a negative result applies only to
the registered amplitude, directions, parent phase, and horizon.

## 2. Frozen parent and intervention

For each ray `<100>`, `<110>`, and `<111>`, reconstruct the unchanged
FTD-0760 center parent at `L=321` through tick 160 using the final
FTD-0760 protocol and common-action options. Require the reconstructed parent
to satisfy the FTD-0760 core and exact-transaction gates before applying any
intervention.

Let `d` be the unit ray and let `q=0.015`. Freeze three arms per ray:

```text
rest:   p_a -> p_a
plus:   p_a -> p_a + q d
minus:  p_a -> p_a - q d
```

for both constituent records `a`. The intervention preserves relative
momentum. Record its exact change to the production-dispersion kinetic energy
and the total matter momentum. Do not redress, translate, replace, or refit the
field after the intervention.

The value `q=0.015` is inherited as a finite coherent-transport scale from
FTD-0646, not fitted to the FTD-0760 parent. Its success in a different
selected object is background motivation only and is not evidence for this
campaign.

## 3. Frozen evolution and records

Evolve all nine arms for 256 forward transactions, from tick 160 through tick
416, using the unchanged explicit-rounding WSL2 CUDA matched-field backend.
The fixed integer center of the original parent remains the regional GPU
diagnostic chart; it does not define or track object identity.

At every tick record:

- effective constituent positions, anchors, remainders, and momenta;
- relational center `C`, relative position, and support-independent core
  margins;
- site hops and shared-anchor status;
- matter, local-field, and spline-Poynting momenta and their step defects;
- matter, binding, field, and total energy changes;
- every common-action residual and causal-speed excess.

At ticks `{160,224,288,352,416}`, also record the state-only matter/field
observer, support ladder `{4,6,8}`, root singular values, and condition number.
Run a state-only reverse check of the immediately preceding accepted
transaction at each post-initial checkpoint. The reverse check acts on a copy
and cannot alter the forward history.

Write one CSV/JSON pair per ray and one aggregate JSON under
`engine/results/ftd_0761/`. No FTD-0761 result artifact may exist before the
pre-execution audit is frozen.

## 4. Frozen gates

### Exact/coherence gates

Every arm must satisfy:

```text
P_core member at every tick
graph margin >= 1e-6
energy margin >= 1e-6
maximum common-action residual <= 1e-10
total-energy residual <= 1e-8 per transaction
causal speed excess <= 1e-12
minimum root singular value >= 1e-3 at checkpoints
root condition number <= 1e4 at checkpoints
reverse checkpoint recovery <= 1e-10
```

The rest arm for each ray must have final center displacement at most `1e-9`.
Internal anchor changes do not fail the rest arm.

### Transport gates

For each plus/minus pair define signed projected displacement relative to its
rest control. A direction passes transport only if:

```text
signed final displacement >= 1.0 cell for both signs
each 64-tick signed block increment after tick 160 is > 0
maximum transverse center displacement <= 0.10 cell
at least two constituent anchor changes occur in each sign arm
plus/minus signed displacement histories agree within 1e-7
plus/minus core-margin histories agree within 1e-7
```

This is a secular center-motion gate. `site_hops` alone cannot pass it.

### Momentum classification

Momentum does not determine the transport verdict. Separately classify a
passing direction as `FIELD_BALANCED` only if the maximum per-step and
cumulative spline matter--field momentum defects are at most `1e-9` and
`1e-8`, respectively. Otherwise classify it
`SUBSTRATE_REACTION_UNRESOLVED`. No fitted momentum normalization is allowed.

## 5. Verdict map

Apply the first matching verdict:

1. hash, parent, artifact, CUDA, reverse-check, or row-shape failure:
   `M4_BOOST_DISCOVERY_INFRASTRUCTURE_UNRESOLVED`;
2. any rest arm fails exact/coherence or stationary-center gates:
   `M4_BOOST_DISCOVERY_BASELINE_INVALID`;
3. a boost arm exits the core or fails exact/coherence:
   `M4_BOOSTED_RELATIONAL_COHERENCE_CLOSED_AT_REGISTERED_SCALE`;
4. no direction passes the transport gates:
   `M4_BOOSTED_RELATIONAL_TRANSPORT_CLOSED_AT_REGISTERED_SCALE`;
5. one or two directions pass:
   `M4_BOOSTED_RELATIONAL_TRANSPORT_ANISOTROPIC_WITNESS`;
6. all three directions pass:
   `M4_BOOSTED_RELATIONAL_TRANSPORT_WITNESS`.

Append the independent momentum classification to verdicts 5 and 6. A
constructive verdict establishes only a selected boosted moving-history
witness. It does not establish a native force, generic mobile family, field
acceleration, physical momentum, wake, mass, or particle pole.

## 6. Firewall

One non-evidential `L=33`, `<100>`, two-tick qualification may verify the
runner schema, CUDA calls, reverse interface, and absence of result writes.
It may not change `q`, the horizon, gates, directions, parent phase, or
verdict map. After the implementation and independent certificate are frozen
and hashed, every registered ray mode runs once. Interrupted or failed modes
are not tuned or rerun under FTD-0761.

Production, `RenderBridge`, scenarios, the common action, binding law,
predicate, constants, and ontology primitives remain unchanged.

