# FTD-0737 — Precontact energetic-capture delay discriminator v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Date:** 2026-07-29  
**Parent:** FTD-0736 (`PRECONTACT_REENTRY_WITHOUT_PERSISTENT_CORE`)  
**Scope:** observer-only selected dynamics; no action, binding law, state type,
coefficient, force, production rule, or morphology threshold changes

## 1. Question and prediction

FTD-0736 validly failed its strict requirement that pair energy be negative
from the instant of graph re-entry. Its independently certified raw record
contains a new post-hoc pattern: each registered ray first enters a
continuously graph-inside, `E_pair<-1e-6` tail exactly 15 ticks after re-entry:

```text
<001>   re-entry 63  -> energetic onset 78
<01-1>  re-entry 79  -> energetic onset 94
<111>   re-entry 96  -> energetic onset 111
```

This 15-tick equality is a new prediction, not a reinterpretation of the
FTD-0736 verdict. Extend the same three plus/minus histories through tick 122.
FTD-0736 measured maximum conservative current-source radius 3. Locking that
same cap gives

```text
T_contact = 129 - 2(3) = 123,
```

so tick 122 remains causally prior to emitted-disturbance self-contact.

## 2. Frozen protocol

- `L=129`, horizon `T=122`, `dt=1/4`;
- selected `DerivedCompactPair` well with depth `0.01` and cutoff squared
  `3/2`;
- unbound separation `1.30`, opposing momentum magnitude `0.0120`;
- the `<001>`, `<01-1>`, and `<111>` rays, all `plus_minus` polarity;
- the same minimum-energy initial static redress, face-flux normalization,
  sparse exact current, local residual evaluation, solve tolerance `2e-14`,
  gate tolerance `1e-10`, and 384-iteration cap as FTD-0736;
- 122 forward steps followed by 122 state-only inverse steps;
- no new morphology solve: FTD-0736 already qualified the receiver on these
  exact prefixes. Field-energy gain remains a required ledger gate.

The opposite polarity, bound control, and three morphology times are not
repeated because FTD-0736 passed them in the shared 112-tick prefix. This
campaign tests only the newly predicted energetic delay and ten-tick tail.

## 3. Locked gates

For every forward and reverse step require:

- valid common-action root;
- maximum registered residual `<=1e-10`;
- recoil defect `<=1e-9`;
- causal-speed excess `<=1e-12`;
- maximum conservative sparse-current source radius `<=3`;
- complete-history pair-plus-field balance `<=1e-8`;
- state-only inverse recovery `<=1e-8`.

Persist all three tick-zero states, all 366 forward states, and all 366
reverse roots (`735` CSV rows total).

For each ray require:

- initial graph exclusion and `E_pair>1e-6`;
- exactly the frozen transition sequence `7;26;{63,79,96}`;
- the first tick beginning a continuous graph-inside,
  `E_pair<-1e-6` tail through tick 122 to equal the third transition plus 15;
- no fourth graph transition;
- field-energy gain from tick zero to 122 greater than `1e-6`.

## 4. Verdict map

- Any matrix, serialization, initialization, root, action, support, energy,
  recoil, speed, inverse, or parent-prefix failure:
  `PRECONTACT_ENERGETIC_CAPTURE_EXECUTION_INVALID`.
- Any transition sequence changes:
  `PRECONTACT_REENTRY_SEQUENCE_NOT_REPRODUCED`.
- A continuous negative tail exists but its onset is not exactly `+15` ticks:
  `PRECONTACT_ENERGETIC_DELAY_NOT_REPRODUCED`.
- The predicted onset occurs but a later state through tick 122 leaves the
  graph or becomes nonnegative:
  `PRECONTACT_ENERGETIC_CORE_RELEASES`.
- Every core gate passes but field-energy gain fails:
  `PRECONTACT_CAPTURE_WITHOUT_FIELD_ENERGY_RECEIVER`.
- Every gate passes:
  `PRECONTACT_DELAYED_ENERGETIC_CAPTURE_CONSTRUCTIVE`.

## 5. Interpretation boundary

A constructive result establishes a finite causal sequence in the selected
dynamics: geometric re-entry, a direction-independent 15-tick relaxation
delay, then a continuously negative relational core through the last tick
before possible periodic self-contact. It supports field-mediated formation
as a local process rather than an instantaneous graph-label event.

It does not establish persistence after tick 122, an uncontained solution,
an invariant basin, attraction, a production particle, a native binding law,
or physical mass/charge/spin. The 15-tick delay remains measured structure
until derived from the action's local equations.
