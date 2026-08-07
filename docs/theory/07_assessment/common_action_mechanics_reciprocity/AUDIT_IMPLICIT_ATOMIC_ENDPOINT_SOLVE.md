# AUDIT — Implicit atomic endpoint solve

**Date:** 2026-07-25  
**Identifier:** `FTD-0537`  
**Status:** `[NUMERICAL FACT — PROVISIONAL SIX-COORDINATE ROOTS]` +
`[RESOLVED BY FTD-0538/0539 — EDGE REFLECTION-PLANE CUSP]` +
`[CONDITIONAL DIAGNOSTIC — BOTH REGISTERED ENERGIES NONCLOSING]`  
**Verdict:** `IMPLICIT_ATOMIC_ENDPOINT_SOLVE_UNRESOLVED`  
**Pre-registration:**
[`PREREG_IMPLICIT_ATOMIC_ENDPOINT_SOLVE_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_IMPLICIT_ATOMIC_ENDPOINT_SOLVE_v1.md)  
**Run of record:** `engine/results/ftd_0537/windows_msvc_cpu.json`

## 1. Locked initial-value solve

The FTD-0536 connection endpoint was eliminated exactly as

```text
A_1(X_1)=A_0-lambda(E_0-K^(0)(X_1)).
```

The six endpoint coordinates were then solved from the start Legendre map

```text
R(X_1)=P_kin,0(X_1)-P_kin,input
```

using the preregistered damped Newton method. Energy was not included in the
root equation.

All four canonical solves reached a fine-stencil residual below `1.31e-9` in
two or three iterations. Their signed-cubic, polarity, and integer-translation
images reevaluated to provisional roots in all 240 arms. The minimum accepted
Jacobian pivot was `0.4926`; no backtracking was required.

## 2. Algebraic identities survive

At those endpoints the non-derivative atomic identities remain closed:

```text
current split             6.439293542825908e-15
continuity                6.938893903907228e-15
field equations/update    5.551115123125783e-17
Gauss evolution           6.994405055138486e-15
causal excess             0
```

Translation, polarity-mirror, and signed-cubic scalar residuals remain below
`3.64e-15`. These are valid algebraic results independent of the derivative
verdict.

## 3. The preregistered derivative gate fails

The locked endpoint derivative comparison used deposited five-point
differences at `h=2^-12` and `h/2`. Its worst disagreement is
`5.398133168706594e-5`, exceeding the `1e-7` gate by a factor of about 540.

The failure is localized and explained geometrically. The two shell-2
canonical endpoints lie only

```text
speed 0.125: 1.303503676890472e-4
speed 0.250: 1.442841426015917e-4
```

from an integer face. Both distances are smaller than `h=2.44140625e-4` but
larger than `h/2=1.220703125e-4`. Consequently, the coarse five-point stencil
crosses a face-current chart whereas the fine stencil does not. Exactly 144 of
240 transported arms inherit this shell-2 chart straddling. The shell-3 roots
remain farther from a chart and converge at `6.40e-14` or better.

The roots are not on a cusp, so this is not a proof that the action lacks a
derivative. It is a failure of the locked cross-chart numerical estimator.
Under the preregistration, that failure forces the unresolved verdict and
prevents the fine-stencil roots from being called stationary solutions.

## 4. Conditional energy diagnostic

If the fine-stencil roots are provisionally retained, neither registered
energy closes:

```text
ordinary quadratic total: 1.086567797354246e-4 .. 5.563111083843823e-4
staggered-modified total:  6.030574702943766e-5 .. 3.309156135578049e-4
```

These values are recorded, not promoted to a no-go result. The locked verdict
order requires derivative diagnostics to pass before the energy branch can be
classified closed negative.

## 5. Consequence

FTD-0537 alone neither validated nor killed the FTD-0536 action as an
exact-energy transaction. It isolated the next mathematical task: differentiate the
piecewise-polynomial deposited action within its actual endpoint chart (or
use a chart-contained convergence sequence), then solve the same six equations
without changing the action, initial data, or energy gate. FTD-0538/0539 now
resolve that task: smooth corners fail energy, while edges have a genuine
set-valued cusp and also fail energy. Reversal is not licensed.

No production code, default, toggle, scenario, force, collision law, phase
order, field ontology, normalization, or tolerance changed.

## 6. Reproducibility

- checks: `6/6 PASS` over four canonical roots and 240 transported arms;
- test SHA256:
  `0504191704CE074F626206DB0D04B2731DE6EE13470FF102491072486C548237`;
- header SHA256:
  `CBA5799689F42EC3F758AB622584735D6D7B8F6CD0EF39FBA550BEC702C1F502`;
- implementation SHA256:
  `EBDFE4D5D724A7A04002ED43C2FE4E0C9221F97C52FA31BAC8581040099452F4`;
- locked preregistration SHA256:
  `5903A6EE88A2232017BAA99454EC74FF6134712F9692B2D7CD06907C3B42D215`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
