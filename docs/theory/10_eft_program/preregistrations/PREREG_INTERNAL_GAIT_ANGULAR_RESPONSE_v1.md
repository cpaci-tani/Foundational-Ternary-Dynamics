# FTD-0617 — Internal-gait angular response v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`
**Scope:** observer-only response map of the FTD-0615 rotational subspace
**Production change:** forbidden

## 1. Frozen parent and state

Require the corrected FTD-0616 run-of-record SHA-256
`9EB7E10D912FE290795BB78E150744EC508C360F50E3BC209AF20091156A6B40`
and reproduce the exact FTD-0612 rest state, uniform neutralizer, common-action
solver, and complete FTD-0615 internal basis.  Use only rotation patterns
`u0,u1`.  No strain mode or new interaction is admitted.

## 2. Complete angular sampling

For `n=0,...,7`, set `theta_n=n*pi/4` and construct

```text
u(theta_n) = normalize(cos(theta_n) u0 + sin(theta_n) u1).
```

Solve the momentum amplitude independently for the unchanged excitation
energy `4 Delta_ref`, require zero centre momentum at `1e-12`, and run 256
forward plus 256 state-only inverse ticks.  The antipodal samples
`theta_(n+4)` are the sign-reversal controls; no separate sign arms are added.

Add four whole-state covariance controls: cyclically rotate the `n=0` and
`n=2` states, fields, and patterns by one and two turns.  This gives twelve
arms and 6,144 atomic transactions.

Record the full centre displacement, centre momentum, and internal `(q_u,p_u)`
at every tick, together with common-action, energy, geometry, anchor, and
pseudomomentum diagnostics.

## 3. Discrete response decomposition

At tick 256, let `D_n` be the displacement vector.  Define for `n=0,...,3`

```text
E_n = (D_n + D_(n+4))/2,
O_n = (D_n - D_(n+4))/2.
```

Compute the exact eight-point real DFT

```text
C_k = (1/8) sum_n D_n cos(k theta_n),
S_k = (1/8) sum_n D_n sin(k theta_n),  k=0,...,4,
```

with the standard endpoint conventions `S_0=S_4=0`. Reconstruct every
`D_n`; maximum residual must be at most `1e-12`. Report every vector
coefficient. No harmonic may be omitted after execution.

Define

```text
R_even = sqrt((1/4) sum_n |E_n|^2),
R_odd  = sqrt((1/4) sum_n |O_n|^2).
```

This is a finite-amplitude discrete angular response, not a continuum Taylor
coefficient or physical mobility tensor.

## 4. Algebraic and covariance gates

Every arm must complete all 512 transactions, retain pair distances in
`[0.5,2.0]`, maximum anchor multiplicity at two, common-action residual at
most `1e-12`, energy drift at most `1e-10`, and state-only recovery at most
`1e-8`.  Cyclic controls must reproduce every recorded centre and the complete
final state within `1e-8`.

## 5. Verdicts

- `MIXED_PARITY_INTERNAL_GAIT_RESPONSE_RESOLVED`: all algebraic, record, DFT,
  and covariance gates pass and both `R_even` and `R_odd` exceed `0.25` cell;
- `SINGLE_PARITY_INTERNAL_RESPONSE_RESOLVED`: all gates pass but at least one
  parity RMS is at most `0.25` cell;
- `INTERNAL_GAIT_ANGULAR_RESPONSE_NUMERICALLY_UNRESOLVED`: any parent, rest,
  excitation, arm, inverse, DFT, record, or covariance gate fails.

The result may identify a symmetry-constrained response law for a later
balanced-gait construction.  It does not authorize selecting an angle after
inspection, adding mode amplitudes as free fits, claiming a physical particle,
or calling the externally neutralized motion isolated self-propulsion.

**Protocol lock:** `protocol_sha256=3BBD327679EB34D2F4196D897EEEF3040E6A90899C489589612D707B833E1065`
