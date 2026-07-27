# AUDIT — Symmetric diagonal coupled endpoint

**Date:** 2026-07-25  
**Identifier:** `FTD-0531`  
**Status:** `[CONSTRUCTIVE — ENERGY-COUPLED SYMMETRY REDUCTION]` +
`[MEASURED — UNIQUE LOCKED ROOT FAMILY]` +
`[GLOBAL ACTION DOMAIN RESOLVED BY FTD-0533]` +
`[SINGLE-SLAB COMPOSITION CLOSED NEGATIVE BY FTD-0534]` +
`[CLOSED NEGATIVE BY FTD-0536 — ATOMIC-ACTION STATIONARITY]` +
`[OPEN — NEW NONLINEAR ATOMIC ROOT/ARBITRARY FIELDS]`  
**Verdict:** `SYMMETRIC_DIAGONAL_ENERGY_COUPLED_ENDPOINT_CONSTRUCTIVE`  
**Pre-registration:**
[`PREREG_SYMMETRIC_DIAGONAL_COUPLED_ENDPOINT_v1.md`](../10_eft_program/preregistrations/PREREG_SYMMETRIC_DIAGONAL_COUPLED_ENDPOINT_v1.md)  
**Run of record:** `engine/results/ftd_0531/windows_msvc_cpu.json`

## 1. What was solved

FTD-0529 proves that an unchanged edge/corner elastic output cannot pay exact
matched-field work. FTD-0531 lets the outgoing equal-and-opposite momentum
magnitude determine the endpoint and current self-consistently.

For the production dispersion,

```text
H(p)=sqrt(E_REST^2+C_SPEED^2 p^2),
d(p_1)=C_SPEED^2(p_0+p_1)/(H_0+H_1).
```

The two momentum-dependent straight histories generate the exact aggregate
current `K(p_1)`. The field update is `E_1=E_0-K(p_1)`. The scalar zero-COM
energy equation is

```text
R(p_1)=2(H_1-H_0)
       +beta/2 (||E_0-K(p_1)||^2-||E_0||^2)=0.
```

The locked field is

```text
E_0=K_0/2+(1/8)C C^T K_0,
```

where `K_0=K(p_0)`. A stationary compensating density is defined once from
the initial Gauss law and remains fixed. The full staggered embedding uses
`B_before=C_SPEED C^T E_0` and lands on `B_half=0` before current deposition.

## 2. Root and endpoint result

Every one of the 240 edge/corner arms has a sign-changing bracket between the
incoming momentum and the momentum at `0.95 C_SPEED`. `R` is strictly
increasing on the locked 65-point grid, with minimum increment
`0.0035459419001128132`. Bisection converges in at most 38 iterations.

The field does not merely relabel the FTD-0527 path:

```text
momentum increase      0.0006009185423793556 .. 0.0012450839865014163
endpoint shift         0.0005468927379653765 .. 0.0008907723858042060
```

Thus existing relativistic momentum has sufficient local capacity to absorb
the exact diagonal field work while the endpoint and current move with it.

## 3. Exact transaction identities

Registered maxima are:

```text
root/total-energy residual          6.2533507018402990e-13
continuity                          6.9388939039072284e-15
absolute Gauss                      6.9388939039072284e-15
staggered embedding                 0
field midpoint-work                 1.4094628242311558e-18
matter-work                         6.2533431124250916e-13
discrete-gradient displacement      7.7715611723760958e-16
causal excess                       0
explicit inverse                    8.8817841970012523e-16
translation/polarity/cubic orbit    0
```

The reversed endpoint histories deposit `-K`, restore the field and endpoint
density, and undo the equal matter-energy change. No energy projection,
velocity clipping, fitted amplification, or tolerance change is used.

## 4. Epistemic ceiling

This is an existence construction in a one-scalar symmetry sector. It does not
derive a three-component impulse from the FTD-0484 interaction action. The
field family and its compensating stationary density are selected test inputs,
not an emergent isolated-pair dressing. Strict increase on 65 grid points is a
measured uniqueness gate, not a global monotonicity theorem.

Consequently FTD-0531 proves neither arbitrary-field contact nor general
scattering. The next load-bearing question is whether variation of the complete
matched worldline interaction yields this momentum change, including every
transverse component, without restoring the FTD-0485 threshold jump or the
FTD-0480 zero-displacement underdetermination.

FTD-0532 resolves the first direct composition attempt negatively: every one
of these constructive edge/corner endpoints crosses two or three coordinate
planes simultaneously, so the frozen one-cell FTD-0485 evaluator rejects it
even on a zero connection. The energy endpoint remains valid, but its force now
requires a registered multi-cell simultaneous-knot variation.

FTD-0533 supplies that variation from the complete FTD-0484 deposited action:
internal-knot gradients converge and all 240 geometries enter its domain. It
does not yet establish that this scalar energy endpoint is stationary under the
full vector matter-field action.

FTD-0534 then proves the FTD-0531 midpoint work field and staggered magnetic
history cannot occupy one connection slab unless `C^T K=0`; every diagonal
root violates that condition. Only a registered multistage/phase-space action
can now test full-vector stationarity.

FTD-0536 constructs that minimal atomic action and tests this root family. All
240 roots fail its particle Legendre equations and ordinary energy gate by
resolved margins. FTD-0531 remains a valid scalar energy construction, but it
is closed negative as a common-action stationary solution.

No production code, default, toggle, scenario, force, collision law, phase
order, field ontology, normalization, or tolerance changed.

## 5. Reproducibility

- checks: `6/6 PASS` over `240` arms;
- test SHA256:
  `EB402CD63EB34CF2AFF40C27AAB6BE0C2EB46F10158BC32239A01F1A3450787E`;
- header SHA256:
  `EE829FAF001A9A4D591D13223D202B6E989CDE6C5ADC26C2061115A9E856D815`;
- implementation SHA256:
  `FEFFBE03117F1E1E30999EFC80DEC1F025026DC2383E34A8DAB08A6B40CEEF2E`;
- locked preregistration SHA256:
  `BC1A8905A01759D0BFFF6D9371E7F4CE77108FCDCF57D766B1752A74A307DC2F`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
