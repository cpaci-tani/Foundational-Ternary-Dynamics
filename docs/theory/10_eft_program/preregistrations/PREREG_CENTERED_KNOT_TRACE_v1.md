# PRE-REGISTRATION — Centered knot trace v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0492`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0491`

## Question

Can the unique local linear cubic-invariant average of incident-cell field
traces remove the symmetric knot branch/self-force without ceasing to be the
ordinary derivative of the fixed-history common action?

## Exact centered trace

At a site, the eight incident cells form one transitive orbit under the cubic
group. For a field-independent linear trace rule

```text
T(F)=sum_sigma w_sigma F_sigma,
```

cubic invariance forces every `w_sigma` equal. Constant reproduction forces
their sum to one. Therefore uniquely

```text
w_sigma=1/8.
```

For a matched face electric field, four incident cells use the outgoing
`a`-face and four use the incoming `a`-face, so

```text
E_center,a(i) = [E_a(i)+E_a(i-e_a)]/2,
rho(i)         = sum_a [E_a(i)-E_a(i-e_a)].
```

The centered value and Gauss source are the sum and difference channels. The
reflection-symmetric self-field of FTD-0491 has `E_center=0` while a uniform
external bias is reproduced exactly. This is local and needs no per-source
provenance.

## Common-action discriminator

Freeze the FTD-0491 source plus `E_bias=(0.4,0.5,0.6)`, with
`A0=0`, `A1=-lambda E`. The centered rule would assign

```text
p_center=(g q lambda/2) E_bias.
```

and the production dispersion fixes its endpoint. In the selected incident
cell, however, the ordinary branch action sees

```text
E_branch=E_bias+q sigma/6,
P0_branch=p_center-(g q lambda/2)E_branch
         =-(g lambda/12)sigma.
```

Thus the centered trace is a generalized/weak derivative selection, not the
ordinary discrete Legendre derivative of the outgoing fixed-history action.

## Locked tests

Use `L=17`, knot `(8,8,8)`, `lambda=c=C_SPEED`, `E_REST=0.511`,
`g=0.73`, both polarities, and tolerance `1e-12`.

1. Enumerate the eight octants and prove the invariant normalized weight is
   `1/8`; require exact constant reproduction.
2. Verify the centered face formula on deterministic affine and general face
   fields under polarity, translation, coordinate reflection, and cyclic
   rotation.
3. Verify exact cancellation of the symmetric self trace, exact reproduction
   of the uniform bias, and unchanged `D E=q`.
4. Evaluate the centered-bias endpoint with the ordinary FTD-0490 action in
   its selected incident cell. Require the initial canonical residual to equal
   `g lambda/12` componentwise (with the appropriate signs), not zero.
5. Apply a nonzero gauge and require the mismatch in kinetic variables to be
   unchanged below `1e-12`.

## Frozen verdicts

- `CENTERED_TRACE_IS_COMMON_ACTION_STEP` only if the branch Legendre residual
  closes below `1e-12`.
- `CENTERED_TRACE_UNIQUE_LOCAL_BUT_NOT_BRANCH_ACTION_DERIVATIVE` if self-force
  cancellation/covariance pass and the exact nonzero residual is recovered.
- `IMPLEMENTATION_INVALID` if algebraic, cubic, Gauss, or gauge controls fail.

## Consequence

The negative common-action verdict does not make the centered trace useless.
It identifies the unique local linear cubic-covariant weak gather. But adopting
it changes the variational rule at knots and requires a new proof of finite
work, energy, reversal, and symplectic behavior. It cannot be inserted as an
unannounced repair of the frozen action.

No production toggle or scenario is authorized.

Run-of-record test-source SHA256:
`2302E4A48E7755173DC172477E7AF10DB7326AD2EA89CB9F45D8FB66D1BBEEF3`.
