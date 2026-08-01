# FTD-0745 — Finite-support environmental closure v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE VALIDATION EXECUTION]`  
**Date:** 2026-07-29  
**Parents:** FTD-0739--0743  
**Scope:** observer-only validation of the selected reciprocal `(s,C,F)`
dynamics; no production rule, primitive, potential, field equation, force,
coefficient, scenario, or default changes

## 1. Question

FTD-0739 establishes one compact-support formation witness on `L=145` through
the last precontact tick. Does the same local initial state and unchanged
selected action remain causally identical in a larger quotient, retain a
localized negative core and nonvanishing near field for a longer horizon, and
transport an operationally source-free outgoing component through a ladder of
shells without inward return?

This is FTD-0740 gate M2. It tests finite-ladder environmental closure, not an
infinite-volume limit, invariant basin, particle, bound-state pole, or native
reduction to `(s,J)`.

## 2. Discovery/validation split

The complete FTD-0739 `L=145`, 136-tick record is the **discovery and frozen
prefix record**. Its CSV is fixed at SHA-256

```text
E9B9B2FCE0FDA1350DBD6195AE039E99004141C86CB8A3F195ACE5CF24ADC622.
```

It may be used only to fix the M2 observer and margins. It is not rerun and it
does not count as held-out validation. The only new candidate data are one
fresh `L=193` execution after this protocol and its implementation are frozen.

The discovery record has minimum radius-eight field energy over its last 16
ticks of `1.8895e-3` (face), `1.3779e-3` (edge), and `9.1989e-4` (body). The
held-out near-field floor is deliberately frozen lower at `5e-4`; this is a
prior-data-derived validation margin, not an independently predicted number.
The late-window dynamic-range ceiling `4` is likewise a coarse stability gate,
not a physical constant.

## 3. Locked dynamics and history matrix

Use the exact FTD-0739 compact-support preparation and selected action with:

- periodic quotient `L=193`;
- initial support radius `R0=4`;
- horizon `T=184`, followed by 184 state-only inverse ticks;
- first possible periodic self-contact `T_contact=L-2R0=185`;
- `dt=1/4`, live `C_SPEED`, compact-pair depth `0.01`, cutoff squared
  `3/2`, solve tolerance `2e-14`, action/observer gate `1e-10`, and 384
  nonlinear iterations;
- exact sparse local current and local residual evaluation;
- unbound separation `1.30` and opposing momentum `0.0120`;
- bound-control separation `1.00` and opposing momentum `0.0150`;
- no damping, reaction, collision, absorption, external support, global
  redress, legacy force, field rescaling, post-hoc correction, or retuning.

Run the same five histories:

1. unbound `plus_minus` face `<001>`;
2. unbound `plus_minus` edge `<01-1>`;
3. unbound `plus_minus` body `<111>`;
4. unbound `minus_plus` body `<111>`;
5. initially bound `plus_minus` face control.

Persist tick zero and every forward state. Reverse states need not repeat the
regional observer, but every reverse root participates in exact action,
recoil, speed, energy, and final state-recovery gates. The expected forward
row count is `5*(184+1)=925`.

## 4. Locked regional observer

Use the already qualified FTD-0686 batched observer at centered Chebyshev
radii

```text
R = {8,12,16,24,32,48}.
```

It is algebraically equivalent to the FTD-0671 scalar regional-energy ledger
and changes no state. At every forward step record inside/outside modified
field energy, signed boundary transport into each region, current/source
exchange inside each region, and cumulative outward transport. Reconstruct the
pre-current electric field from the accepted before-field and after magnetic
half-step.

For each shell, define the source exchange outside it by

```text
Delta U_source,out = Delta U_source,total - Delta U_source,inside.
```

This quantity must remain below `1e-10` in absolute value. Deposited current
support must remain at radius at most three.

## 5. Registered gates

### E0 — execution and exact transaction

Every preparation must satisfy the FTD-0739 compact-support, Gauss, Poisson,
and zero-crossing gates. Every forward and reverse transaction must be valid.
Require:

- maximum common-action and regional residual `<=1e-10`;
- per-step total-energy residual `<=1e-8`;
- recoil defect `<=1e-9`;
- causal-speed excess `<=1e-12`;
- total pair-plus-field drift `<=1e-8`;
- state-only inverse recovery `<=1e-8`;
- current-source radius `<=3` and `T<T_contact`.

Failure is `ENVIRONMENTAL_CLOSURE_EXECUTION_INVALID`; no physical verdict may
be extracted.

### E1 — causal prefix embedding

For every history and tick `0..136`, compare the `L=193` record against the
frozen `L=145` CSV after centering translations are removed. The discrete
fields `valid`, `common`, `regional_valid`, source radius/count, and graph
membership must agree exactly. The maximum scalar difference over action,
energy, recoil, speed, regional residual, separation, pair energy, total field
energy, and all radius-eight/radius-twelve inside, outside, transport,
source-exchange, and cumulative-outward records must be `<=1e-10`.

Failure is `ENVIRONMENTAL_CLOSURE_CAUSAL_PREFIX_DRIFT`.

### E2 — controls and polarity

The initially bound face control must remain graph-inside and below pair energy
`-1e-6` with no graph transition through tick 184. The two body-diagonal
polarity histories must have identical transitions and first-passage ticks,
with maximum persisted scalar difference `<=1e-9`.

Failures are, in order,
`ENVIRONMENTAL_CLOSURE_BOUND_CONTROL_UNSTABLE` and
`ENVIRONMENTAL_CLOSURE_POLARITY_SENSITIVE`.

### E3 — longer-horizon core

Each unbound history must remain continuously graph-inside with pair energy
below `-1e-6` for at least 64 consecutive ticks ending at tick 184. This is a
longer held-out persistence window than the latest-forming FTD-0739 body arm.

Failure is `ENVIRONMENTAL_CLOSURE_CORE_NOT_PERSISTENT`.

### E4 — operational near field

Over ticks `153..184`, each unbound history must have radius-eight inside field
energy with

```text
minimum >= 5e-4,
maximum/minimum <= 4.
```

This defines only a noncollapsing localized near-field component over the
registered window. It is not called a bound-state wavefunction or a uniquely
projected dressing.

Failure is `ENVIRONMENTAL_CLOSURE_NEAR_FIELD_NOT_STABLE`.

### E5 — ordered source-free shell arrival

At each shell `R in {12,16,24,32,48}`, require:

- initial outside energy `<=1e-12`;
- maximum outside energy `>1e-8`;
- final outside energy `>1e-9`;
- a first threshold-passage tick exists;
- first-passage ticks are nondecreasing with radius;
- outside source-exchange residual is `<=1e-10`.

The ordering is the registered causal shell-arrival law. No affine speed fit is
permitted in this campaign.

Failure is `ENVIRONMENTAL_CLOSURE_ARRIVAL_LAW_FAIL`.

### E6 — no registered inward return

From first passage onward at every shell `R in {12,16,24,32,48}`, each tick's
increment of cumulative outward transport must be at least `-1e-10`.

Failure is `ENVIRONMENTAL_CLOSURE_OUTGOING_COMPONENT_RETURNS`.

## 6. Verdict order

The first failed branch in Sections 5.E0--5.E6 is the verdict. If every branch
passes, the only constructive token is

```text
FINITE_LADDER_ENVIRONMENTAL_CLOSURE_CONSTRUCTIVE.
```

A constructive verdict closes M2 only over the registered two-volume,
precontact ladder and licenses implementation of the already frozen FTD-0743
M3 family classifier. It does not license particle names, asymptotic stability,
charge, mass, spin, statistics, Lorentz recovery, or production adoption.

## 7. Failure consequences

- Prefix drift closes the claimed domain-of-dependence embedding for this
  preparation/implementation.
- Core or near-field failure classifies the FTD-0739 witness as a finite
  environmental-reservoir history at this longer horizon.
- Arrival failure withdraws the claimed operational dressing/tail separation.
- Inward return prevents any persistence claim beyond the first failing shell
  and tick.
- No failed gate authorizes tolerance relaxation, force amplification,
  favorable-ray selection, new primitive state, or an altered potential.

## 8. Run discipline

Before validation execution:

1. compile the runner only;
2. statically certify the protocol/runner constants, matrix, gates, verdict
   grammar, output schema, and frozen FTD-0739 baseline hash;
3. freeze the runner and Release executable hashes;
4. replace the runner's `UNLOCKED` protocol token with this document's SHA-256,
   rebuild without execution, and certify again;
5. authorize exactly one fresh five-history execution.

After execution, an independent proof must recompute every gate from the
persisted CSV/JSON rather than trust the runner summary. Production and golden
tests remain unchanged.
