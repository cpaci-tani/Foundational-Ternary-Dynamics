# PRE-REGISTRATION — Constituent-complete charged-trimer transaction v1

**Date locked:** 2026-07-27  
**Identifier:** `FTD-0600`  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Parents:** `FTD-0501`, `FTD-0512`–`FTD-0514`, `FTD-0541`,
`FTD-0544`, `FTD-0550`–`FTD-0553`, `FTD-0598`, `FTD-0599`  
**Scope:** observer-only selected constituent dynamics. Production state,
production tick, defaults, toggles, scenarios, CUDA, and WASM remain unchanged.

## 1. Ontology and scope lock

The one-voxel R0 carrier is not treated as physical matter. The candidate
matter state is the lossless three-constituent phase-space record

```text
C3={(q_a,X_a,p_a)}_(a=1)^3 / permutations preserving q,
q=(-1,-1,+1) or its charge conjugate,
X_a=anchor_a+remainder_a.
```

The ordered array used by the implementation is not an observable label.
Swapping the two equal-polarity constituents must leave every aggregate and
verdict quantity invariant after applying the same swap to the returned
constituent records.

Every constituent polarity remains exactly ternary. Fractional quadratic
coats are derived coupling representations, not fractional primitive state.
The three constituent anchors must remain distinct and their site projection
must remain in `{-1,0,+1}` in every admitted arm.

This candidate tests one finite charged composite. It does not identify an
electron, derive a generation, establish binding in nature, or adopt a
connection/gauge ontology.

## 2. Locked internal action

Use the smallest equilateral triangle whose vertices lie on the cubic lattice,
with all squared rest lengths equal to two. The selected translation- and
cubic-covariant binding potential is

\[
 V_b(X)=\frac{\kappa}{4}\sum_{a<b}
       \left(\lvert X_a-X_b\rvert^2-2\right)^2,
 \qquad \kappa=1.
\]

The value `kappa=1` is an imposed unit-stiffness candidate, not a derived
constant and not fitted after execution. No other binding, damping, hard
contact, force amplification, or constraint projection is admitted.

For pair `a,b`, define

```text
d0=X_a0-X_b0,  d1=X_a1-X_b1,
u0=|d0|^2-2,   u1=|d1|^2-2,
G_ab=(kappa/4)(u0+u1)(d0+d1).
```

The atomic binding impulses are

```text
I_bind,a += -h G_ab,
I_bind,b += +h G_ab.
```

They must satisfy exactly

\[
 \sum_a \bar v_a\cdot I_{b,a}=-(V_b(X_1)-V_b(X_0)),
 \qquad \sum_a I_{b,a}=0.
\]

## 3. Locked constituent–field transaction

Use the production dispersion independently for every constituent,

\[
 H(p)=\sqrt{E_{\rm REST}^2+C_{\rm SPEED}^2|p|^2},
 \qquad
 \bar v_a=C_{\rm SPEED}^2\frac{p_{a0}+p_{a1}}
 {H(p_{a0})+H(p_{a1})},
 \qquad X_{a1}=X_{a0}+h\bar v_a.
\]

For each straight constituent chord, deposit the exact FTD-0541 quadratic
face current `K_a` and endpoint densities. Sum them only after retaining every
constituent record:

```text
K=sum_a K_a,  rho_n=sum_a rho_(a,n).
```

Advance the matched face-electric/edge-magnetic field once:

```text
B1=B0-lambda C^T E0,
E*=E0+lambda C B1,
E1=E*-K,                    lambda=C_SPEED h.
```

Gather the same midpoint electric field and updated magnetic field on each
constituent orbit using FTD-0550. With the already fixed face normalization
`g`, solve all nine endpoint momentum components simultaneously:

\[
 p_{a1}-p_{a0}=I_{E,a}+I_{B,a}+I_{b,a}.
\]

No native-recoil substitution, `grad|J|`, Poisson force, legacy Lorentz force,
self-field subtraction, fitted multiplier, energy projection, endpoint reset,
or post-hoc residual deposit is permitted.

A distant static opposite unit coat may neutralize the periodic computational
window. It is fixed in both time directions and is explicitly an external
computational compensator; global momentum including that compensator is not
claimed.

## 4. Independently declared identities

For every converged candidate require:

```text
rho1-rho0+div K=0,
div E0=rho0+rho_comp,
div E1=rho1+rho_comp,
Delta H_a=vbar_a dot Delta p_a,
sum_a vbar_a dot I_B,a=0,
Delta V_b+sum_a vbar_a dot I_bind,a=0,
Delta(sum H_a+V_b)=g<Ebar,K>,
Delta U_field=-g<Ebar,K>,
Delta(sum H_a+V_b+U_field)=0,
sum_a I_bind,a=0.
```

Every one-step algebraic, continuity, Gauss, work, energy, kinematic,
causality, cubic-covariance, integer-translation, and equal-charge-permutation
residual must be at most `1e-12`.

The selected periodic matched-field pseudomomentum

\[
 (P_f)_i=g\langle E,D_i C B\rangle
\]

is measured independently. The defect

\[
 R_P=\left|\sum_a(p_{a1}-p_{a0})+(P_{f1}-P_{f0})\right|
\]

is a required diagnostic, not silently set equal to zero. Because a fixed
neutralizing coat is an external momentum reservoir and the cubic substrate
has no continuous microscopic translation symmetry, `R_P` is not a
one-transaction algebraic kill gate. A value above `1e-12` blocks any claim of
isolated total-momentum closure and must be carried into the infrared gate as
an explicit substrate/compensator recoil defect.

## 5. State-only inverse

The reverse solver receives only the final persistent state and immutable
charges/binding parameters. It must solve the nine unknown initial momenta,
reconstruct

```text
X0=X1-h vbar(p0,p1),
E0=E1+K[X0,X1]-lambda C B1,
B0=B1+lambda C^T E0,
```

and satisfy the same impulse equation. It may not receive the forward current,
endpoint, impulses, Newton root, iteration history, or branch record.

One-step state recovery must be at most `1e-10`. Nonunique reverse roots,
failure to recover the equal-charge permutation class, or use of a stored
forward record fails this candidate.

## 6. Locked fixtures and execution order

Use `L=17`, `h=1`, the rest triangle

```text
r1=(0,0,0), r2=(1,1,0), r3=(1,0,1),
```

translated away from the boundary and given the common subcell offset
`(0.173,-0.219,0.287)`. Test both charge-conjugate assignments and the locked
common velocities

```text
(0,0,0),
(0.06,0,0),
(0.05,0.04,0),
(0.04,0.05,-0.03).
```

For each, run the base, integer-translated, cyclically rotated, and
equal-charge-swapped copies. Add one invalid-colliding-anchor input and one
deliberately impossible solver budget; both must fail closed.

Execution order:

1. symbolic binding-work and impulse-sum identities;
2. stationary one-step arm;
3. remaining one-step fixtures and state-only inverse;
4. only if all one-step conjunctive gates pass, a 32-step common-velocity
   traversal for each polarity followed by 32 state-only reverse solves;
5. only if the repeated campaign passes, classify localization, site hops,
   and pseudomomentum scaling. No scenario or production toggle follows from
   this version.

Repeated acceptance requires:

- every forward and reverse root converges;
- per-step algebraic gates remain below `1e-10`;
- the complete initial persistent state is recovered below `1e-8`;
- total-energy drift is below `1e-9`;
- all constituent speeds remain causal;
- at least one legitimate constituent site hop occurs;
- pair distances remain finite and no site-projection collision occurs.

## 7. Locked verdicts and repair policy

- all one-step and repeated gates pass:
  `CHARGED_TRIMER_COMMON_ACTION_CONSTRUCTIVE`;
- one-step common-action identities pass but repeated/state-only recovery
  fails:
  `CHARGED_TRIMER_ONE_STEP_ONLY`;
- any one-step identity, uniqueness, or inverse gate fails:
  `CHARGED_TRIMER_ATOMIC_TRANSACTION_CLOSED_NEGATIVE`;
- numerical conditioning prevents certification without a physical gate
  failure:
  `CHARGED_TRIMER_TRANSACTION_UNRESOLVED`.

Any repair is allowed by the owner research policy only as a new `v2` or new
identifier locked before execution. The v1 record and verdict remain intact.
A constructive result licenses selected constituent dynamics only. A failed
field-pseudomomentum diagnostic blocks an isolated recoil claim even when the
common-action energy transaction is constructive.

## 8. Required artifacts

- `engine/include/ftd/eft/constituent_complete_charged_trimer.h`
- `engine/src/eft/constituent_complete_charged_trimer.cpp`
- `engine/tests/test_constituent_complete_charged_trimer.cpp`
- `scripts/proofs/proof_constituent_complete_charged_trimer.py`
- `engine/results/ftd_0600/ftd_0600_charged_trimer_v1.json`
- `engine/results/ftd_0600/ftd_0600_charged_trimer_v1.csv`

The protocol SHA-256 is computed over this file through the newline
immediately before the `protocol_sha256` line.
`protocol_sha256=F24CC0BFBF0741B0F1A07DCE3B719EA6452E3DC81BB0E9F76013F211D25F6328`
