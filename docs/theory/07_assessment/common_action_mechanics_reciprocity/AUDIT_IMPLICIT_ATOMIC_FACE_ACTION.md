# AUDIT — Implicit atomic face action

**Date:** 2026-07-25  
**Identifier:** `FTD-0536`  
**Status:** `[DERIVED — MINIMAL ACTION CONDITIONAL ON SELECTED FACE FIELD]` +
`[CONSTRUCTIVE — ATOMIC FIELD EQUATIONS]` +
`[CLOSED NEGATIVE — FTD-0531 SCALAR ROOT STATIONARITY]` +
`[CLOSED NEGATIVE BY FTD-0539 — EXACT-ENERGY/UNIQUE-INVERSION MOBILE LAW]`  
**Verdict:**
`ATOMIC_FACE_ACTION_CONSTRUCTIVE_SCALAR_ROOT_NOT_STATIONARY`  
**Pre-registration:**
[`PREREG_IMPLICIT_ATOMIC_FACE_ACTION_v1.md`](../10_eft_program/preregistrations/PREREG_IMPLICIT_ATOMIC_FACE_ACTION_v1.md)  
**Derivation:**
[`DERIV_IMPLICIT_ATOMIC_FACE_ACTION.md`](../10_eft_program/derivations/DERIV_IMPLICIT_ATOMIC_FACE_ACTION.md)  
**Run of record:** `engine/results/ftd_0536/windows_msvc_cpu.json`

## 1. Minimal atomic action

Once the selected face-field representation and FTD-0478 normalization are
fixed, the FTD-0484 endpoint split determines the minimal one-slab action:

```text
S_d=S_m
   +beta/(2 lambda^2)||A_1-A_0||^2
   -beta/2||C^T A_1||^2
   +(beta/lambda)S_int^(1).
```

Its connection variations are exactly

```text
E_0=E_slab+K^(0),
E_1=E_slab+lambda C B_1-K^(1),
E_1-E_0=lambda C B_1-K.
```

This resolves FTD-0535's phase-order obstruction at the action level: the
start current is included in the initial canonical relation rather than added
after a current-free Faraday step. It does not yet supply a solution of the
coupled particle equations.

## 2. Registered result

All 240 FTD-0531 edge/corner roots enter the reconstructed atomic slab. Exact
current and field identities close at:

```text
current split                    6.439293542825908e-15
continuity                       5.911937606128959e-15
start connection equation        1.387778780781446e-17
end connection equation          0
atomic field update               4.163336342344337e-17
Gauss evolution                   5.884182030513330e-15
```

Complete deposited-action endpoint derivatives converge between `h=2^-12`
and `h/2` to `6.40e-14`, far below the locked `1e-7` derivative gate.
Translation, polarity-mirror, and signed-cubic residuals are below `2.63e-15`.

## 3. Scalar-root failure

No registered scalar root is stationary. Gauge-invariant endpoint kinetic
residuals are

```text
start: 3.590020322841089e-05 .. 2.334460897594862e-04
end:   1.176566194936720e-05 .. 1.534289945201377e-04.
```

Every arm has a longitudinal defect; some arms also have transverse defects up
to `2.194536670001951e-04`. The ordinary matter-plus-quadratic-field energy
defect is nonzero on every arm:

```text
9.117660283101915e-05 .. 5.027261719732542e-04.
```

These gaps exceed their registered gates by orders of magnitude and cannot be
attributed to differentiation error. FTD-0531 solved a scalar energy equation
for a different staggered field history. It did not solve the discrete
Legendre equations of the atomic action.

## 4. Consequence

The action candidate survives as a selected variational object; the old root
family does not. FTD-0537 attempted
the simultaneous endpoint solve and found 240 provisional fine-stencil roots,
but its locked coarse/fine derivative comparison straddled a current chart in
144 arms and failed. The next valid step is a chart-contained or analytic
endpoint derivative. FTD-0538 then found smooth corner roots with nonclosing
energy and an exact edge reflection plane. FTD-0539 resolved the plane into a
genuine normal cusp: every edge root needs a set-valued subgradient selection
and also misses both energies. The unchanged action is therefore closed
negative as FTD-0479's exact reciprocal mobile law. Reversal is not licensed.
Retuning the FTD-0531 scalar equation, adding a magnetic kick, or projecting
energy afterward would not be a common-action solution.

No production code, default, toggle, scenario, force, collision law, phase
order, field ontology, normalization, or tolerance changed.

## 5. Reproducibility

- checks: `6/6 PASS` over `240` arms;
- test SHA256:
  `E22B21001157328351752F5C17F0BD98BAADAE03E3861C44B950CBCE7C1784E8`;
- header SHA256:
  `22418FA05A339D52872F871A19A8BF3E27DB7183D8A18EABB132E6A926176D5D`;
- implementation SHA256:
  `366BECAE202CCC6E78710FA42F83B83809D57EEE2EA517FBBCAF3E7CEAE945AC`;
- locked preregistration SHA256:
  `8CB27E8232FF65C43A74B2A6A24B407AA86AA969DE0042419E7B630424403CC7`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
