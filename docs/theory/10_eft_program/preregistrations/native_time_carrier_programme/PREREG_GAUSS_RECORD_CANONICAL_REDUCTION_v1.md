# FTD-0877 — Gauss-record canonical reduction v1

**Identifier:** `FTD-0877`  
**Date frozen:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Search policy:** exact incidence algebra, exact Fourier-symbol evaluation,
and source inspection only. No near-miss, parameter, formula-substitution, or
coincidence search is permitted.

## 1. Question frozen before execution

Can the native `(flux,wave_vel)` carrier be reduced on a matched Gauss
constraint so that a ternary divergence record has a mathematically exact
canonical conjugate and stable transverse recursion? Does the live
cell-centred central-difference/18-point-SOR production pass realize that exact
reduction? What information must be retained for reversible preparation?

## 2. Frozen sources

| source | SHA256 |
|---|---|
| `THEOREM_FLUX_WAVE_VELOCITY_MARKOV_CANONICAL_CARRIER_AND_PRODUCTION_BOUNDARY_v1.md` | `656F51A4E5A533C0436E932B452A33810CD851D63E571621DF81ECB0C9BED622` |
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |
| `engine/include/ftd/poisson_solvers.h` | `07F2E7DD85A1E476DAE6BE7F4FE371E664A2B965B9B542EC4162BDEEC5A9DBC4` |
| `engine/src/poisson_solvers.cpp` | `59DC42FB8D0160373F02301C5B7AB09B2C9692242FC0D852C0404ECCA371362B` |
| `engine/include/ftd/eft/matched_gauss_transport.h` | `1E07F87A0EBD0D1830D0632B82C2BD65497EBEAE7BB152EA02C5AAE19328B033` |
| `engine/src/eft/matched_gauss_transport.cpp` | `12BF98040BB45AD6CD9A409A93C842101C400CEEE6242E9B9352158A33A9D028` |
| `engine/tests/test_native_source_core_fork.cpp` | `81BE123F7EC73D78B2D233CAD733D438DED6A4E683FB542F9F1EC200FD6C68B1` |

## 3. Frozen mathematical class

For a connected finite periodic oriented-face incidence complex, freeze

```text
D : R^(3V) -> Q,                 Q = 1-perp
L = D D^T,                      Pi_Q = I - 11^T/V
q = D J,                        p = L+ D P
J_T = J - D^T L+ q,             P_T = P - D^T p.
```

Test the canonical two-form on `(J,P)` against the direct sum of its
restriction to `ker D x ker D` and the `(q,p)` form on `Q`. The finite periodic
probe is a computational quotient, not the uncontained ontology.

The production audit is source-locked to the actual central divergence,
central gradient, 18-point SOR stencil, finite iteration count, and manifested-
site skip. It may not infer exactness from a low residual.

## 4. Frozen certificate gates

The certificate must pass every gate without source or tolerance repair:

1. seven frozen source hashes;
2. protocol self-hash;
3. exact connected incidence rank `V-1` on a fixed finite witness;
4. exact `L L+ = L+ L = Pi_Q`;
5. exact charge bracket `{q,p}=Pi_Q`;
6. exact longitudinal/transverse reconstruction of generic rational `(J,P)`;
7. exact vanishing of transverse divergences and longitudinal/transverse
   cross pairings;
8. exact symplectic-form split on two fixed rational variations;
9. exact neutral ternary minimum-energy/static-section witness;
10. exact polarity covariance;
11. exact `D C=0` for a matched curl witness;
12. exact preservation of charge by transverse kick/drift;
13. general fixed-range right-inverse contradiction via the polynomial degree
    bound `deg f <= 2R+1`, `L-1>2R+1`, and `f(1)=-1`;
14. exact central-composition versus 18-point symbols at `(pi/2,0,0)`;
15. exact Nyquist blindness versus 18-point response at `(pi,0,0)`;
16. source markers for the live stencil mismatch, finite SOR, and manifested-
    site skip;
17. exact nonidentity/idempotent/noninjective matched affine preparation map;
18. an explicit collision of two longitudinally distinct inputs at the same
    prepared record;
19. exact recovery when the discarded discrepancy is retained;
20. scope markers excluding Hilbert, Born, Bell, `G*`, production promotion,
    and completeness; and
21. terminal gate reached only if all preceding checks pass.

## 5. Frozen outcomes

- **Outcome A — exact reduction / production boundary:** the matched complex
  passes the canonical and record gates, while the live production symbols do
  not match. Book the exact constrained reduction, close the exact-production-
  projector claim negative, and keep physical preparation/actuation open.
- **Outcome B — abstract reduction only:** the incidence algebra passes but
  the declared matched-engine/source facts fail. Book no engine witness.
- **Outcome C — closed negative:** an exact algebraic gate fails. Preserve the
  attempt and do not repair it under v1.

## 6. Banned promotions

The following are forbidden:

- a static minimum-energy section is a dynamically derived particle;
- a global inverse-Laplacian observer is an onsite local actuator;
- mean subtraction creates or explains net charge;
- approximate production residual reduction is exact projection;
- matched-sidecar success silently changes production storage;
- an information ledger proves thermodynamic reversibility without an
  environment dynamics;
- a Gauss record derives Hilbert space, quantization, Born, or Bell; or
- a constraint-preserving recursion derives the quartic `G*` calendar.

## 7. Execution rule

The protocol SHA256 and frozen certificate SHA256 must be entered in
`REF_PREREGISTER_MANIFEST.md` as `LOCKED/PRE-RUN` before first execution. Any
failed gate requires a separately preregistered repair; v1 is not edited to fit
the result.
