# AUDIT — Chart-contained atomic endpoint solve

**Date:** 2026-07-25  
**Identifier:** `FTD-0538`  
**Status:** `[THEOREM + MEASURED — SHELL-2 ENDPOINT ON REFLECTION PLANE]` +
`[CONSTRUCTIVE — SHELL-3 CHART-CONTAINED ROOTS]` +
`[RESOLVED BY FTD-0539 — SHELL-2 NONSMOOTH SUBGRADIENT]` +
`[CONDITIONAL CLOSED NEGATIVE — SHELL-3 ENERGY]`  
**Verdict:** `CHART_CONTAINED_ENDPOINT_SOLVE_UNRESOLVED`  
**Pre-registration:**
[`PREREG_CHART_CONTAINED_ATOMIC_ENDPOINT_SOLVE_v1.md`](../10_eft_program/preregistrations/PREREG_CHART_CONTAINED_ATOMIC_ENDPOINT_SOLVE_v1.md)  
**Run of record:** `engine/results/ftd_0538/windows_msvc_cpu.json`

## 1. Result

The FTD-0537 derivative failure was not merely an overlarge step near a
generic endpoint. For every shell-2 edge arm, the free endpoint lies exactly
on the integer plane of the inactive coordinate. This follows directly from
the edge direction `(+-1,+-1,0)`: the selected free displacement has no normal
component, so its normal coordinate remains integer.

The locked chart-contained five-point rule therefore cannot initialize the
two shell-2 canonical roots. Its clearance is exactly zero, below the
registered `2^-30` floor. Signed-cubic, polarity, and translation transport
produces the same obstruction in all 144 shell-2 arms.

The 96 shell-3 corner arms have no inactive coordinate. Both canonical roots
converge in two iterations and transport through the full corner orbit:

```text
worst root residual             2.280953204092384e-13
worst derivative convergence   6.397660179402465e-14
minimum chart clearance        5.459777756451345e-3
minimum Jacobian pivot          0.5358317529897854
```

Their endpoints and action values agree exactly with the ordinary FTD-0537
evaluation. Thus chart containment changes no action; it only refuses to
differentiate through an endpoint chart boundary.

## 2. Algebraic transaction

Across all 240 classified arms, including the shell-2 clearance failures, the
already-computed non-derivative transaction diagnostics remain:

```text
current split             6.439293542825908e-15
continuity                6.938893903907228e-15
field equations/update    3.816391647148976e-17
Gauss evolution           6.994405055138486e-15
causal excess             0
```

The obstruction is therefore localized to the variational derivative, not
continuity, Gauss transport, field update, or causality.

## 3. What FTD-0537's shell-2 roots mean

FTD-0537's coarse centered stencil crossed the inactive-coordinate chart. Its
Newton solve then moved the edge endpoint `1.30e-4..1.44e-4` off the exact
reflection plane and reported an estimator-dependent transverse root. FTD-0538
does not accept that symmetry-breaking displacement as a differentiably
verified solution. It also does not prove that the plane is nonstationary:
the action may be smooth across the plane, have a cusp with a valid nonsmooth
extremum, or have incompatible one-sided slopes.

FTD-0539 performs the decisive one-sided calculation: all in-plane roots exist,
but the normal derivatives disagree while bracketing zero. The edge action is
nonsmooth stationary only after a set-valued subgradient selection. No
transverse perturbation or fitted smoothing is licensed.

## 4. Conditional corner energy result

At the 96 differentiably verified shell-3 roots, neither registered energy
closes:

```text
ordinary quadratic total: 1.451598499398153e-4 .. 4.600247519254358e-4
staggered-modified total:  6.030574702943766e-5 .. 3.144434937908146e-4
```

This closes the FTD-0536 action negative as an exact-energy transaction for the
corner sector only. It does not yet close the full edge-plus-corner candidate,
because the shell-2 derivative gate has priority under the locked verdicts.

## 5. Consequence

FTD-0538 resolves the numerical ambiguity from FTD-0537 and replaces it with a
precise mathematical boundary problem. The face-current representation remains
constructive. The minimal FTD-0536 action has differentiable roots but fails
energy in the corner sector; its edge-sector stationarity requires a one-sided
or nonsmooth variational audit.

No production code, default, toggle, scenario, force, collision law, phase
order, field ontology, normalization, or tolerance changed.

## 6. Reproducibility

- checks: `7/7 PASS` over four canonical classifications and 240 arms;
- test SHA256:
  `0EDE89A454B08CAFE55D58FC78F356D174A212000AB9156D9FFDA16774B0F5C3`;
- face-action header SHA256:
  `22418FA05A339D52872F871A19A8BF3E27DB7183D8A18EABB132E6A926176D5D`;
- face-action implementation SHA256:
  `366BECAE202CCC6E78710FA42F83B83809D57EEE2EA517FBBCAF3E7CEAE945AC`;
- endpoint-solver header SHA256:
  `CBA5799689F42EC3F758AB622584735D6D7B8F6CD0EF39FBA550BEC702C1F502`;
- endpoint-solver implementation SHA256:
  `EBDFE4D5D724A7A04002ED43C2FE4E0C9221F97C52FA31BAC8581040099452F4`;
- locked preregistration SHA256:
  `CA3C609281B116B25EB02116399DA938625B572DBFE60CB6C45402D3B02A37E2`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
