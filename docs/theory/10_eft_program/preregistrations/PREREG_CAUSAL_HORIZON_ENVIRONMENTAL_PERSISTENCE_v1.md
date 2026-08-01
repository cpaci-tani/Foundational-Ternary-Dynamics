# FTD-0746 — Causal-horizon environmental persistence v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE HELD-OUT EXECUTION]`  
**Date:** 2026-07-29  
**Parents:** FTD-0739, FTD-0740, FTD-0745  
**Scope:** observer-only forward validation of the selected reciprocal `(s,C,F)`
dynamics; no production rule, primitive, potential, field equation, force,
coefficient, threshold, scenario, or default changes

## 1. Question

FTD-0745 preserved all four negative cores and late near fields but failed its
outer arrival conjunction because the source-free threshold front had not
reached radii 32 and 48 by the last precontact tick. Does the same local initial
state and unchanged action retain its localized core and near field until the
frozen radius-48 arrival prediction, and does the radius-48 exterior component
then remain present and outward through a preregistered post-arrival window?

This is a fresh M2 spatial-reach/persistence candidate. It is not a rerun,
threshold relaxation, asymptotic limit, invariant basin, particle, radiation,
or native-reduction claim.

## 2. Frozen discovery record and horizon construction

The complete FTD-0745 CSV is discovery data fixed at SHA-256

```text
58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C.
```

For each unbound cubic ray its registered `1e-8` first-passage ticks were

```text
(R,t) = (8,22), (12,48), (16,75), (24,130).
```

The ordinary least-squares line through those four frozen points is

```text
t_threshold(R) = -32.7142857142857 + 6.76428571428571 R.
```

It predicts radius-48 passage at tick `291.9714`. This construction is used
only to set the held-out horizon. It is not called a signal speed or physical
dispersion relation.

Freeze:

- radius-48 first passage no later than tick `300`;
- final horizon `T=312`, giving at least 12 ticks after the latest allowed
  first passage;
- periodic quotient `L=321` and compact support radius `R0=4`;
- earliest possible periodic self-contact `T_contact=L-2R0=313`, so
  `T<T_contact`.

The threshold remains exactly `1e-8`; no FTD-0745 margin is relaxed.

## 3. Locked dynamics and ray matrix

Use the exact FTD-0745 compact-support preparation and selected action with:

- `dt=1/4`, live `C_SPEED`;
- derived compact-pair depth `0.01`, cutoff squared `3/2`;
- solve tolerance `2e-14`, action/observer gate `1e-10`, and 384 nonlinear
  iterations;
- exact sparse local current and local residual evaluation;
- unbound separation `1.30` and opposing momentum `0.0120`;
- no damping, reaction, collision, absorption, external support, global
  redress, legacy force, field rescaling, post-hoc correction, or retuning.

Run exactly one fresh forward history on each inequivalent cubic ray:

1. `plus_minus` face `<001>`;
2. `plus_minus` edge `<01-1>`;
3. `plus_minus` body `<111>`.

The body polarity conjugate and bound face control are not new validation arms.
FTD-0745 already found exact conjugacy and a stable control through tick 184;
this protocol does not claim to extend either result to tick 312. Omitting them
reduces memory and compute while retaining every inequivalent spatial ray. No
ray may be stopped after another ray's result is known.

The runner is invoked once per arm. Persist tick zero and every forward state;
expected rows are `3*(312+1)=939`. No state-only inverse is run. Therefore no
new long-horizon reversibility claim is licensed even if every forward gate
passes.

## 4. Locked regional observer

Use the qualified FTD-0686 batched observer at centered Chebyshev radii

```text
R = {8,12,16,24,32,48}.
```

At every forward step record inside/outside modified field energy, signed
boundary transport into each region, current/source exchange, and cumulative
outward transport. Reconstruct the pre-current electric field from the accepted
before-field and after magnetic half-step.

At every shell require absolute source exchange outside the registered support
below `1e-10`. Deposited-current support must remain at radius at most three.

## 5. Registered gates

### H0 — execution and exact forward transaction

Every preparation must satisfy the FTD-0745 compact-support, Gauss, Poisson,
and zero-crossing gates. Every forward transaction must be valid. Require:

- maximum common-action and regional residual `<=1e-10`;
- per-step total-energy residual `<=1e-8`;
- recoil defect `<=1e-9`;
- causal-speed excess `<=1e-12`;
- total pair-plus-field drift `<=1e-8`;
- current-source radius `<=3` and `T<T_contact`;
- valid standard JSON output: every non-finite optional scalar is serialized as
  JSON `null`, never `inf` or `nan`.

Failure is `CAUSAL_HORIZON_EXECUTION_INVALID`; no physical verdict may be
extracted.

### H1 — exact FTD-0745 causal prefix

For each ray and tick `0..184`, compare the `L=321` record against the frozen
FTD-0745 CSV after centering translations are removed. The discrete fields
`valid`, `common`, `regional_valid`, source radius/count, and graph membership
must agree exactly. The maximum scalar difference over action, energy, recoil,
speed, regional residual, separation, pair energy, total field energy, and all
six-shell inside/outside/transport/source/cumulative records must be `<=1e-10`.

Failure is `CAUSAL_HORIZON_PREFIX_DRIFT`.

### H2 — long core persistence

Each ray must remain continuously graph-inside with pair energy below `-1e-6`
for at least 160 consecutive ticks ending at tick 312.

Failure is `CAUSAL_HORIZON_CORE_NOT_PERSISTENT`.

### H3 — late localized near field

Over ticks `281..312`, radius-eight inside field energy must satisfy

```text
minimum >= 5e-4,
maximum/minimum <= 4.
```

Failure is `CAUSAL_HORIZON_NEAR_FIELD_NOT_STABLE`.

### H4 — frozen radius-48 arrival prediction

At radius 48 require:

- tick-zero outside energy `<=1e-12`;
- maximum outside energy `>1e-8`;
- first threshold passage exists and is `<=300`;
- source exchange outside radius 48 remains `<=1e-10`.

Failure is `CAUSAL_HORIZON_R48_ARRIVAL_FAIL`.

### H5 — post-arrival outward persistence

From radius-48 first passage through tick 312 require:

- every cumulative-outward increment `>=-1e-10`;
- final outside energy `>1e-9`;
- every radius-48 outside energy over ticks `301..312` is `>1e-9`.

Failure is `CAUSAL_HORIZON_POST_ARRIVAL_NOT_PERSISTENT`.

## 6. Verdict order

The first failed branch H0--H5 is the result. If every ray passes every branch,
the only constructive token is

```text
CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE.
```

A constructive verdict establishes only three-ray forward core/near-field
persistence through a causally predicted radius-48 arrival and 12-tick
post-arrival window. It does not close long-horizon inverse recovery, M3 family
identity, asymptotic environmental closure, particle interpretation, or native
reduction.

## 7. Failure consequences

- Prefix drift closes the claimed domain-of-dependence embedding for this
  implementation.
- Core or near-field failure classifies the FTD-0745 witness as dependent on a
  larger finite environmental reservoir at this horizon.
- Radius-48 arrival failure falsifies the frozen threshold-front extrapolation
  at the registered margin; no later fit is substituted.
- Post-arrival failure prevents any environmental-persistence claim beyond the
  first failed tick.
- No failure authorizes threshold relaxation, ray removal, force amplification,
  altered potential, or a new primitive.

## 8. Execution discipline

Before any arm executes:

1. implement the command-selected three-ray runner with protocol token
   `UNLOCKED`;
2. statically certify constants, action, matrix, gates, standard-JSON
   serialization, output schema, and frozen FTD-0745 hash;
3. hash this protocol, embed it in the runner, rebuild without execution, and
   freeze runner plus WSL2 Release executable hashes;
4. verify every result file is absent;
5. authorize exactly one invocation of each arm.

Run under WSL2 Ubuntu-22.04. Resource scheduling may serialize or overlap arms
but may not change their inputs. After all arms finish, an independent proof
must reconstruct all 939 rows, the prefix comparison, every gate, and the
ordered verdict without calling the C++ verdict logic.

