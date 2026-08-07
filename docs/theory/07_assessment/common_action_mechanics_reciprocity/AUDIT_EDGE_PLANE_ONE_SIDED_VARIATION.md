# AUDIT — Edge-plane one-sided variation

**Date:** 2026-07-25  
**Identifier:** `FTD-0539`  
**Status:** `[CONSTRUCTIVE — IN-PLANE LEGENDRE ROOTS]` +
`[NUMERICAL FACT — CONVERGED NORMAL CUSP]` +
`[CLOSED NEGATIVE — UNIQUE ALGEBRAIC INVERSION]` +
`[CLOSED NEGATIVE — BOTH REGISTERED ENERGIES]`  
**Verdict:** `EDGE_PLANE_NONSMOOTH_STATIONARY_REQUIRES_SUBGRADIENT_SELECTION`  
**Pre-registration:**
[`PREREG_EDGE_PLANE_ONE_SIDED_VARIATION_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_EDGE_PLANE_ONE_SIDED_VARIATION_v1.md)  
**Run of record:** `engine/results/ftd_0539/windows_msvc_cpu.json`

## 1. In-plane roots exist

Keeping the shell-2 inactive coordinate exactly on its integer reflection
plane, the four active Legendre equations converge in two Newton iterations at
both registered speeds. All 144 signed-cubic, polarity, and translation arms
reevaluate successfully:

```text
worst active root residual           4.685418719674317e-13
worst active derivative convergence  4.510281037539698e-15
```

Current, field, Gauss, and causal identities remain below `1.78e-15` except
the field equation maximum `4.16e-17`. The edge plane is therefore not an
algebraic dead end.

## 2. The normal derivative is a genuine cusp

Fourth-order left and right derivatives were evaluated entirely in their
respective endpoint charts and compared at `h=2^-12` and `h/2`. Same-side
convergence is `1.37e-14`, while the incoming normal residual jumps are

```text
speed 0.125: left -2.2988166983e-4, right +5.3414982481e-5,
             jump 2.8329665231e-4;
speed 0.250: left -7.2268155687e-4, right +2.8377422288e-4,
             jump 1.0064557798e-3.
```

The two carriers agree within numerical covariance. Every one of the 144 arms
has the same scalar classification: the one-sided residual interval contains
zero, but the two derivatives disagree by more than `1e-7`. Thus the edge
plane is stationary only in a nonsmooth subgradient sense.

This is not a unique discrete Legendre map. Selecting one element of the
interval requires an additional rule or variable. The ordinary FTD-0536 action
does not provide that selection, so the FTD-0479 algebraic-inversion gate is
closed negative for the frozen variables.

## 3. Energy also fails

At the in-plane roots, both registered total-energy defects remain nonzero in
every edge arm:

```text
ordinary quadratic total: 1.087995886511095e-4 .. 5.566094274269485e-4
staggered-modified total:  8.588376658098333e-5 .. 3.307947937119817e-4
```

FTD-0538 independently found the same qualitative failure at every smooth
corner root. Therefore the frozen minimal action fails FTD-0479 in both Moore
sectors: the corner map is smooth but not energy conserving, while the edge
map is set-valued and also not energy conserving.

## 4. Program consequence

The FTD-0536 minimal atomic action remains a coherent selected variational
model, but it is closed negative as the exact reciprocal mobile-matter law
requested by FTD-0479. This is the plan's explicit failure condition: exact
continuity, Gauss, locality, cubic covariance, and causality coexist, but exact
energy and unique algebraic inversion do not coexist without an additional
primitive selection mechanism.

No `common_action_face_dynamics` toggle, dashboard scenario, morphology claim,
or infrared/Lorentz campaign is licensed. The exact face-current observer and
its centered rendering projection remain valid.

No production code, default, toggle, scenario, force, collision law, phase
order, field ontology, normalization, or tolerance changed.

## 5. Reproducibility

- checks: `6/6 PASS` over two canonical roots and 144 edge arms;
- test SHA256:
  `81004771EB86A538BA175FB0D8AE90B2872BA0575C27F8898EE5ECD5393BE1F5`;
- observer header SHA256:
  `754429169221B32EA900F28F77ECACA70BC2C02E2AF81FABE32D44BF676C8572`;
- observer implementation SHA256:
  `63CF547F9D9975B5AA3E4AFBAD9C79F2247A8798FE0D50EEFB44642CD88F3894`;
- shared face-action header SHA256:
  `22418FA05A339D52872F871A19A8BF3E27DB7183D8A18EABB132E6A926176D5D`;
- shared face-action implementation SHA256:
  `366BECAE202CCC6E78710FA42F83B83809D57EEE2EA517FBBCAF3E7CEAE945AC`;
- locked preregistration SHA256:
  `F8637E0D4474BAF0C985FAF5D16A441A065620003B86E5E4133A1D270702F564`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
