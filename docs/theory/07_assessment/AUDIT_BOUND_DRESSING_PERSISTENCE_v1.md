# Audit — FTD-0727 bound-dressing persistence v1

**Status:** `[AUDIT PASS — EXECUTION UNRESOLVED / RAW PERSISTENCE
NON-PROMOTABLE]`  
**Date:** 2026-07-29

## Findings

1. **Every registered history executes.** All 208 arms pass rowwise action,
   energy, recoil, inverse, and bound-control gates over 96 forward and 96
   reverse steps.

2. **The locked global covariance gate fails.** Maximum scalar-history spread
   is `1.1065308669344631e-9`, above `1e-9`. The campaign verdict is therefore
   unresolved regardless of the raw persistence classes.

3. **Raw lower-energy trapping persists.** All 104 parent arms at `p=0.0060`
   and `0.0095` remain inside and negative for every tick 49--96.

4. **The compact-dressing classifier fails.** Parent dynamic-field radius
   grows from three at tick 48 to five or six at tick 96. Localized-dressing
   count is `0/104`.

5. **The escape control is contaminated.** Every `p=0.0120` arm re-enters the
   graph, giving three transitions per arm; 12/52 finish negative. No arm
   satisfies the locked positive-outside tail.

6. **The campaign is not pre-wrap.** Exact local support and the periodic
   initial dress permit finite-volume recurrence within 96 ticks on `L=33`.

7. **No new primitive is indicated.** Determination and inversion succeed
   with the current complete state. The open defects concern numerical
   covariance and finite-volume/dynamical recurrence.

## Correct statement

The selected `L=33` action produces raw 96-tick negative-core persistence
with an expanding field component, but the registered campaign is unresolved
because covariance narrowly fails and the escaping control universally
re-enters. Neither stable matter nor compact bound dressing is qualified.

## Verification

- protocol `49941B34…7312F`;
- runner `5640D53F…5B68`;
- JSON `52C9537C…3F95`;
- CSV `007E3FC1…6668`;
- independent certificate `7D25D322…1243`, `93/93 PASS`;
- focused CTest `1/1 PASS` in `919.65 s`;
- production defaults, tick, toggles, and scenarios unchanged.

