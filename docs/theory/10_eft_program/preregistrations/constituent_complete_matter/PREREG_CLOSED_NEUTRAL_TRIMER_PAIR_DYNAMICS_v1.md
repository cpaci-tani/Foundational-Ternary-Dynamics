# PRE-REGISTRATION — Closed neutral charged-trimer pair dynamics v1

**Date locked:** 2026-07-27  
**Identifier:** `FTD-0601`  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Parent:** `FTD-0600`  
**Scope:** observer-only selected constituent dynamics. Production state,
production tick, defaults, RNG, toggles, scenarios, CUDA, and WASM remain
unchanged.

## 1. Question and ontology lock

FTD-0600 constructed a reversible, energy-closing charged trimer but used a
fixed distant compensating coat and therefore could not license isolated
momentum closure. This protocol removes that external reservoir.

The persistent state contains six explicit ternary constituents grouped into
two dynamical composites,

```text
A={(-1,X0,p0),(-1,X1,p1),(+1,X2,p2)}/S2,
B={(+1,X3,p3),(+1,X4,p4),(-1,X5,p5)}/S2.
```

The total primitive polarity is exactly zero. No stationary density,
background charge, fixed compensator, external force, damping, or hidden
history is admitted. Aggregate density/current are derived field-coupling
observables, not the matter state.

The grouping and three-constituent composition are selected research content,
not a claim that an electron or physical matter has this composition.

## 2. Locked dynamics

Each trimer independently uses the unchanged FTD-0600 binding potential

\[
V_b=\frac14\sum_{g\in\{A,B\}}\sum_{a<b\in g}
 (|X_a-X_b|^2-2)^2.
\]

There is no direct cross-trimer potential. All cross-composite response must
come through the shared matched face-electric/edge-magnetic field.

For every constituent use the production dispersion and chord velocity,

\[
H(p)=\sqrt{E_{\rm REST}^2+C_{\rm SPEED}^2|p|^2},\qquad
\bar v_a=C_{\rm SPEED}^2\frac{p_{a0}+p_{a1}}{H(p_{a0})+H(p_{a1})},
\]

and the exact quadratic-coat face current on the continuous chord. Advance
the shared field once with the sum of all six currents,

```text
B1=B0-lambda C^T E0,
E*=E0+lambda C B1,
E1=E*-sum_a K_a.
```

Gather the same midpoint electric and updated magnetic fields on all six
orbits. Solve all 18 endpoint momentum components simultaneously from

\[
p_{a1}-p_{a0}=I_{E,a}+I_{B,a}+I_{b,a}.
\]

The reverse solve receives only the final persistent state and immutable
parameters. It may not receive forward paths, currents, roots, impulses, or
history.

## 3. Exact common-action gates

Every converged step must satisfy at `1e-12`:

```text
rho1-rho0+div(sum K_a)=0,
div E0=rho0,
div E1=rho1,
Delta H_a=vbar_a dot Delta p_a,
sum_a vbar_a dot I_B,a=0,
Delta V_b+sum_a vbar_a dot I_bind,a=0,
Delta(sum H_a+V_b)=g<Ebar,sum K_a>,
Delta U_field=-g<Ebar,sum K_a>,
Delta(sum H_a+V_b+U_field)=0,
sum_(a in A) I_bind,a=sum_(a in B) I_bind,a=0.
```

Continuity, Gauss, root, force, kinematics, causality, cubic covariance,
integer-translation covariance, independent equal-charge permutations, and
global charge conjugation must each remain below `1e-12`. One-step state-only
recovery must be below `1e-10`.

## 4. Momentum and interaction gates

With no external compensator, measure

\[
P_{\rm tot}=\sum_{a=0}^{5}p_a
 +gP_{\rm matched}(E,B),
\qquad
R_P=|P_{{\rm tot},1}-P_{{\rm tot},0}|.
\]

Unlike FTD-0600, `R_P<=1e-12` is the isolated-momentum gate. A larger value
does not erase a valid energy/common-action result, but it closes the claim
that the field-strength state is a complete isolated momentum carrier.

For the initially resting base pair, let `n` point from the center of A to the
center of B along the shortest periodic displacement. Define

\[
I_{\rm inward}=\tfrac12[(\Delta P_A)\cdot n
                         -(\Delta P_B)\cdot n].
\]

The selected dynamics is electromagnetically attractive at this fixture only
if `I_inward>1e-10`. This sign gate is declared before execution. It is not a
claim of a continuum inverse-square law.

## 5. Fixtures

Use `L=17`, `h=1`, unit binding stiffness, zero magnetic field, and two
oppositely oriented rest triangles translated away from the boundary:

```text
A anchors: (4,7,7), (5,8,7), (5,7,8)
B anchors: (12,9,9), (11,8,9), (11,9,8)
A remainder: (0.173,-0.219,0.287)
B remainder: (-0.137,0.191,-0.233)
```

Initialize the shared electric field by an exact deterministic dipole-path
construction from the full neutral quadratic-coat density. No minimum-energy
or fitted initialization is substituted after inspection.

Test four center-velocity pairs:

```text
(A,B)=(0,0),
((0.04,0,0),(0.04,0,0)),
((0.04,0,0),(-0.04,0,0)),
((0.04,0.03,0),(-0.03,0.02,-0.01)).
```

For each pair run base, integer-translated, cyclically rotated, independently
equal-charge-swapped, and globally charge-conjugated/composite-exchanged
copies: 20 forward and 20 reverse one-step arms. Add one colliding-anchor and
one impossible-solver control; both must fail closed.

## 6. Repeated campaign

If every one-step common-action and inverse gate passes, run the rest,
co-moving, and counter-moving base arms for 16 forward steps followed by 16
state-only reverse steps: 48 forward plus 48 reverse transactions.

Acceptance requires:

- every root converges;
- per-step common-action residuals remain below `1e-10`;
- complete state recovery remains below `1e-8`;
- total-energy drift remains below `1e-9`;
- both trimers remain bound with every internal pair distance in `[1.35,1.48]`;
- no constituent anchors collide;
- at least one legitimate site hop occurs.

Record, but do not fit, per-step `R_P`, cumulative total-pseudomomentum drift,
center separation, inward impulse, internal deformation, and hop count.

## 7. Locked verdicts

- common-action, repeated, inward-response, and isolated-momentum gates pass:
  `NEUTRAL_TRIMER_PAIR_CLOSED_DYNAMICS_CONSTRUCTIVE`;
- common-action, repeated, and inward-response gates pass but momentum fails:
  `NEUTRAL_TRIMER_PAIR_COMMON_ACTION_CONSTRUCTIVE_MOMENTUM_CHANNEL_MISSING`;
- common-action/repeated gates pass but the inward sign gate fails:
  `NEUTRAL_TRIMER_PAIR_NONATTRACTIVE_SELECTED_DYNAMICS`;
- one-step identities pass but repeated recovery/localization fails:
  `NEUTRAL_TRIMER_PAIR_ONE_STEP_ONLY`;
- a one-step physical identity or inverse gate fails:
  `NEUTRAL_TRIMER_PAIR_ATOMIC_TRANSACTION_CLOSED_NEGATIVE`;
- conditioning prevents certification without a physical failure:
  `NEUTRAL_TRIMER_PAIR_UNRESOLVED`.

Any repair requires a new `v2` or identifier locked before execution. A
constructive result licenses only this selected observer dynamics. It does not
derive a particle, local `U(1)`, a connection, a continuum force law, a pole,
Lorentz recovery, or production adoption.

## 8. Required artifacts

- `engine/include/ftd/eft/closed_neutral_trimer_pair.h`
- `engine/src/eft/closed_neutral_trimer_pair.cpp`
- `engine/tests/test_closed_neutral_trimer_pair.cpp`
- `scripts/proofs/proof_closed_neutral_trimer_pair.py`
- `engine/results/ftd_0601/ftd_0601_closed_neutral_pair_v1.json`
- `engine/results/ftd_0601/ftd_0601_closed_neutral_pair_v1.csv`

The protocol SHA-256 is computed over this file through the newline
immediately before the `protocol_sha256` line.
`protocol_sha256=89979BF190B8A5FD36DF6642356E455F13ED01C9A2C42E20777B150996C1C1F3`
