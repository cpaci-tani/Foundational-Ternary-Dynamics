# Audit — FTD-0721 derived interaction-graph transaction

**Status:** `[AUDIT PASS — SELECTED DYNAMICS / FORMATION RESERVOIR OPEN]`  
**Date:** 2026-07-28

## Findings

1. **A stored bond bit is unnecessary for the registered topology change.**
   The interaction graph is recomputed from current constituent separation.
   Every scattering arm gains and loses its edge while the complete inverse
   recovers the initial state below `1.38e-13`.

2. **The topology boundary is physically smooth.** The selected compact well
   and its first derivative vanish at squared separation `3/2`; graph
   membership changes where neither potential energy nor force jumps.

3. **The transaction identities close.** All 104 fresh arms pass. Worst root,
   energy, kinematic, and covariance/inverse diagnostics are respectively
   `9.98e-15`, `5.00e-16`, `3.13e-15`, and `2.53e-13`.

4. **Two-body formation does not follow.** The scattering family remains in
   the positive-energy continuum. The already-bound family remains negative.
   Exact conservation prevents an outside-support positive-energy state from
   becoming a negative-energy bound state without another energy channel.

5. **The potential remains an explicit model choice.** Its compactness,
   smooth cutoff, repulsive core, and unit-separation minimum are logical
   design constraints, not a derivation of a physical binding law.

6. **Count-changing reactions remain outside scope.** No constituent is
   created, destroyed, or merged. Annihilation and genesis still require a
   reversible reservoir or a declared open-system map.

## Correct statement

The current constituent variables can carry reversible relational topology
change without a persistent edge variable. In a closed conservative two-body
sector they cannot also create a negative-energy bound object from an unbound
encounter. FTD-0722 completes the common matter/current/field action but finds
the locked transfer insufficient for capture. The live successor is a fresh
incident-energy window, not an edge-state extension.

## Verification

- preregistration SHA-256:
  `FFCAC54E3368A3DE9FE466908A8BAFF2831D58B0F07AF83BA045BA4315AB6807`;
- focused CTest: `1/1 PASS`;
- independent run-record certificate: PASS;
- production defaults, tick, scenarios, and ontology: unchanged.
