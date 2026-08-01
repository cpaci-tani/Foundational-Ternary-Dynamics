# FTD-0766 — Aged wake and residual-entrainment discriminator v1

**Status:** `[PREREGISTERED OBSERVER QUALIFICATION + FACE-RAY CUDA DISCOVERY]`  
**Scope:** observer-only; production dynamics and defaults frozen

## 1. Question

FTD-0765 shows that FTD-0764's raw trailing moment is exactly a centroid-lag
observable and that the measured residual window is mostly unentrained. This
protocol asks a stronger question:

```text
After the preparation field is aged and mirrored motion is applied, does the
dynamics create a rest-subtracted trailing energy excess that is aligned with
velocity, ordered with boost magnitude, and stable against further aging?
```

The protocol does not test radiation, particle poles, Lorentz recovery, or a
new ontology primitive.

## 2. Frozen parent and branches

- Use the unchanged FTD-0761/0763 connected two-constituent parent.
- Registered volume: `L=321`; registered direction: face `(0,0,1)` only.
- Formation checkpoint: tick `160`.
- Preparation ages after formation: `A={0,64,128}`, so boosts begin at global
  ticks `{160,224,288}`.
- At every age branch the exact same aged state into
  `q={0, +/-0.0075, +/-0.015, +/-0.030}`.
- Evolve each branch for 64 ticks and record local branch times
  `tau={0,16,32,48,64}`.
- A signed boost adds `q d_hat` to every constituent momentum exactly as in
  FTD-0761/0764. No field is boosted, regenerated, recentered, or corrected.
- The `q=0` arm is the matched formation-memory control at each age.
- No result from edge/body rays may be used to change this face discovery
  protocol. Held-out orientations require a later identifier.

## 3. Direct trailing/leading observer

Use the same actual-minus-selected-bound residual fields and staggered
component positions as FTD-0764. For each nonzero arm use its own direction of
motion

```text
d_q = sign(q) d_hat,
xi = x - c_q(tau).                                         (1)
```

Within the registered radius-48 Chebyshev window define residual energy
`u_R>=0`. Exclude the neutral longitudinal slab `|xi dot d_q|<=1/2` and record

```text
T_q = sum u_R 1[xi dot d_q < -1/2],
H_q = sum u_R 1[xi dot d_q > +1/2],
D_q = (T_q-H_q)/(T_q+H_q).                                 (2)
```

Record (2) separately for the near window `r_infinity<=8`, the outer annulus
`8<r_infinity<=48`, and their union. Also retain the FTD-0765 absolute
residual centroid and entrainment fraction.

For each magnitude `a=|q|`, align the plus and minus arms with their own
directions and define

```text
D_pair(a,A,tau) = [D_+(a,A,tau)+D_-(a,A,tau)]/2.            (3)
```

The aligned mirror discrepancy is the maximum normalized difference between
the plus and reflected-minus `(T,H,E_R)` triples. Static directional
anisotropy changes sign under the alignment and is not a wake signal.

## 4. Qualification before registered output

At `L=17,33`, both polarities and face/edge/body directions must pass:

- CPU/CUDA parity for every near/outer leading/trailing energy within `1e-11`;
- direct scalar recomputation within `1e-12`;
- actual/bound/residual/interference reconstruction within `1e-12`;
- integer translation and proper cubic covariance within `1e-11`;
- direction reversal swaps leading/trailing within `1e-12`;
- a symmetric fixture gives `|D|<=1e-12`;
- an independently constructed trailing fixture gives `D>0` and its reflected
  leading fixture gives `D<0`;
- no complete CUDA field download;
- all FTD-0763/0764 observer regressions pass.

No `engine/results/ftd_0766` artifact may be written before all qualification
gates pass.

## 5. Registered validity gates

Every age/boost/checkpoint must satisfy:

- common-action, continuity, Gauss, energy, causality, regularity, and
  one-step inverse residuals `<=1e-12`;
- fractional observer, boundary ledger, and support ladder valid;
- morphology reconstruction residuals `<=1e-12`;
- signed-pair core trajectories mirror within `1e-10` after reflection;
- signed-pair leading/trailing normalized triples mirror within `1e-10`;
- the `q=0` core displacement remains `<=1e-12` over 64 ticks;
- every nonzero arm has positive displacement along `d_q` at tick 64.

Any failure returns `AGED_WAKE_EXECUTION_INVALID`. No tolerance or arm may be
removed after execution.

## 6. Outcome components

Continuous metrics are primary. The following labels are locked.

### 6.1 Velocity-aligned trailing excess

`ALIGNED_TRAILING_EXCESS` requires, for the union window at `tau=64`,

```text
D_pair(a,A,64) >= 1e-5                                    (4)
```

for all three nonzero magnitudes and all three ages, with every registered
mirror gate passing. Otherwise report `NO_UNIVERSAL_ALIGNED_TRAILING_EXCESS`.

### 6.2 Amplitude ordering

`TRAILING_EXCESS_AMPLITUDE_ORDERED` requires at every age

```text
0 < D_pair(0.0075,A,64)
    < D_pair(0.015,A,64)
    < D_pair(0.030,A,64).                                  (5)
```

No power law is fitted in v1.

### 6.3 Preparation-age stability

`TRAILING_EXCESS_AGE_STABLE` requires for every magnitude

```text
|D_pair(a,128,64)-D_pair(a,64,64)|
 / max(|D_pair(a,128,64)|,|D_pair(a,64,64)|) <= 0.25.       (6)
```

Age zero is reported but excluded from (6) because it is the known
formation-transient condition.

### 6.4 Dynamical wake classification

`DYNAMICAL_WAKE_CANDIDATE` requires the conjunction of (4), (5), and (6).
Failure of any component returns `WAKE_CREATION_NOT_ESTABLISHED`. This label
still does not identify a material aura, radiation, drag law, or particle.

### 6.5 Entrainment classification

For `a=0.015`, compute the FTD-0765 final entrainment at each age.

- `AGE_IMPROVING_ENTRAINMENT` requires
  `epsilon_R(128)-epsilon_R(0)>=0.10` and `epsilon_R(128)>0.50`.
- `PERSISTENT_UNDER_ENTRAINMENT` requires `epsilon_R(A)<0.20` at all ages.
- all other outcomes are `MIXED_ENTRAINMENT`.

These thresholds are selected finite-resolution criteria, not particle
definitions.

## 7. Momentum and Peierls records

Record at every checkpoint:

- matter momentum;
- FTD-0473 local field pseudomomentum;
- FTD-0619 spline-Poynting momentum;
- both cumulative defects;
- support-chart phase and available Peierls energy diagnostics.

No momentum closure label is promoted in this face discovery. The data may
motivate a separately derived boundary-stress or connection generator, but the
measured defect may never be assigned as `P_substrate`.

## 8. Consequences

- A constructive wake result licenses only a face-ray, finite-horizon,
  motion-correlated trailing excitation after formation aging.
- A negative wake result means the FTD-0764/0765 trailing signal is adequately
  explained by under-entrained preparation memory at this scope.
- Improving entrainment motivates held-out orientations and wider native
  carriers.
- Persistent under-entrainment moves the matter mainline toward a recruited/
  shed environmental pattern rather than a rigid dressing.
- No outcome changes production, defaults, primitives, toggles, scenarios,
  constants, mass formulas, Lorentz claims, or `RenderBridge`.

