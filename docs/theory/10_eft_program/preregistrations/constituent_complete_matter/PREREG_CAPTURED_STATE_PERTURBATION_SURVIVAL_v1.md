# FTD-0732 — Captured-state perturbation survival v1

**Status:** `[PRE-REGISTRATION — LOCKED / NOT YET RUN]`  
**Identifier:** `FTD-0732`  
**Date:** 2026-07-29  
**Parent:** `FTD-0731`  
**Scope:** test finite captured-state robustness under a fixed local
perturbation cross; no action, coefficient, state type, production default,
toggle, scenario, physical-constant target, or post-output perturbation change.

## 1. Questions locked before execution

1. Does the FTD-0731 captured core survive finite perturbations of relative
   separation, radial momentum, both transverse momentum axes, and dynamic
   field amplitude?
2. Does the unperturbed center survive from parent tick 128 through tick 384?
3. Do the most graph-stressed and energy-stressed perturbations reproduce on
   a held-out larger volume?

This is a finite survival-cross test, not a proof of an open basin or an
attractor. Exact reversible dynamics is not expected to contract onto a
dissipative attractor.

## 2. Parent lock

- FTD-0731 protocol SHA-256:
  `F319B4CA5C0A8F9A777578507828FC0881E996023FD09AA83033D797B47C01EE`;
- FTD-0731 JSON SHA-256:
  `0D4F8519F44F15BF941A410D055947EE4E079A115AF41C476866DF413D45F03D`;
- FTD-0731 CSV SHA-256:
  `BC060706C00E5A15A0C8FF34960EB521301BA73DDC9F33015E1930D65DE5F163`;
- FTD-0731 verdict:
  `MULTIPASS_RADIATIVE_CAPTURE_VOLUME_STABLE`;
- reconstruct each `p=0.0120` parent through tick 128 with the same
  `dt=1/4`, selected compact well, normalization, sparse face current,
  local residual evaluation, root tolerance `2e-14`, and at most 384 root
  iterations.

## 3. Captured centers

Use the three registered representatives `0_0_1`, `0_1_-1`, and `1_1_1`,
both polarity orders, periodic fields, and the exact FTD-0731 initial dress.
At tick 128 require each reconstructed center to match the stored FTD-0731
separation and pair-energy records within `1e-12`, to be graph-inside, and to
have pair internal energy `<-1e-6`.

Let the instantaneous constituent midpoint be `c`, relative vector be
`r=x_1-x_0`, radial unit be `u=r/|r|`, and probe fraction be fixed once as

```text
epsilon = 1/20.
```

Use the following locked transverse frames:

| direction | `t_1` | `t_2` |
|---|---|---|
| `0_0_1` | `(1,0,0)` | `(0,1,0)` |
| `0_1_-1` | `(1,0,0)` | `(0,1,1)/sqrt(2)` |
| `1_1_1` | `(1,-1,0)/sqrt(2)` | `(1,1,-2)/sqrt(6)` |

The frame vectors are orthonormal to the registered initial ray. No claim of
continuous rotational covariance follows from these representatives.

## 4. Locked perturbation cross

For every `L=33` captured center run exactly 11 variants:

1. `center` — no perturbation;
2. `separation_minus/plus` — replace `r` by `(1-epsilon)r` or
   `(1+epsilon)r` about fixed midpoint `c`;
3. `radial_impulse_minus/plus` — add equal-and-opposite constituent impulses
   of magnitude `epsilon * 0.0120 = 0.0006` along `u`;
4. `tangent1_impulse_minus/plus` — the same equal-and-opposite impulse along
   `t_1`;
5. `tangent2_impulse_minus/plus` — the same equal-and-opposite impulse along
   `t_2`;
6. `dynamic_field_minus/plus` — multiply the divergence-free dynamic field
   residual by `1-epsilon` or `1+epsilon`.

For position and field variants, decompose the tick-128 field as the freshly
derived static dress for the same matter configuration plus a dynamic
residual. A separation variant receives the freshly derived static dress for
its perturbed matter configuration plus the **unchanged** dynamic residual.
A field variant receives the unchanged static dress plus the scaled dynamic
residual. Apply the same construction to electric and magnetic fields.

Require initial Gauss residual `<=1e-12`, exact total constituent momentum
preservation for every relative perturbation, graph membership, negative pair
energy, and causal constituent speeds. An invalid initial variant fails the
campaign; it is not replaced.

Stage A therefore contains 66 fixed `L=33` histories.

## 5. Held-out volume selectors

After Stage A, select exactly three variant names per direction/polarity for a
held-out `L=65` replay:

1. `center`;
2. the non-center variant with the smallest energy margin
   `min_t(-E_pair(t)/0.01)` over its 256-step continuation;
3. the distinct non-center variant with the smallest graph margin
   `min_t(sqrt(1.5)-|r(t)|)`; if the energy selector is also the smallest
   graph-margin variant, take the next graph-margin variant, with variant name
   as the deterministic final tie-break.

The selectors are locked before Stage A output. They choose the least-stable
observed arms for confirmation, not the arms closest to a desired target.
Stage B contains 18 `L=65` histories. Total campaign size: 84 histories.

## 6. Continuation and observables

From each perturbed tick-128 state run 256 forward steps to parent tick 384,
then 256 state-only reverse steps back to the exact perturbed state. Record at
every continuation tick:

- separation, graph membership, pair internal energy, and field energy;
- every graph-transition tick;
- maximum action, Gauss, energy, causal-speed, and recoil residual;
- state-only inverse recovery;
- minimum energy and graph margins;
- final physical class.

`survives` requires graph membership and `E_pair<-1e-6` at the initial
perturbed state and every one of the 256 later states, no graph transition,
positive-semidefinite field energy, common-action residual `<=1e-10`, recoil
defect `<=1e-9`, pair-plus-field energy defect `<=1e-8`, and inverse recovery
`<=1e-8`.

Require both polarity orders to have the same survival class for each matched
direction/variant. Require every selected Stage-B arm to match its Stage-A
survival class; transition counts must match, and corresponding transition
ticks must differ by at most two.

## 7. Locked verdict map

- Any execution, initial-constraint, action, Gauss, energy, recoil, causal, or
  inverse gate fails:
  `CAPTURE_PERTURBATION_TRANSACTION_UNRESOLVED`.
- Any unperturbed center fails survival:
  `CAPTURE_LONG_HORIZON_UNSTABLE`.
- Any matched polarity pair differs:
  `CAPTURE_PERTURBATION_POLARITY_SENSITIVE`.
- Any selected `L=33/65` survival or transition class differs:
  `CAPTURE_PERTURBATION_VOLUME_SENSITIVE`.
- Every Stage-A perturbation and every Stage-B confirmation survives:
  `CAPTURE_FINITE_PERTURBATION_CROSS_SURVIVES`.
- Centers survive, volume/polarity controls agree, but at least one non-center
  perturbation fails:
  `CAPTURE_FINITE_PERTURBATION_BOUNDARY_WITNESSED`.

Even a full cross survival is only finite-radius, finite-direction,
finite-horizon evidence. It licenses a preregistered corner/hyperrectangle or
linearized stability test; it does not establish an open invariant basin,
asymptotic stability, matter mass, or a physical particle.

