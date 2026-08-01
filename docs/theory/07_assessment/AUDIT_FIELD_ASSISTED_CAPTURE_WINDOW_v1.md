# Audit — FTD-0723 field-assisted capture window v1

**Status:** `[AUDIT PASS — LOCKED WINDOW CLOSED NEGATIVE / LOWER-ENERGY
EXTRAPOLATION OPEN]`  
**Date:** 2026-07-29

## Findings

1. **The constant-export prediction is falsified.** The `p=0.0200` and
   `p=0.0225` families were preregistered to capture if the FTD-0722 absolute
   export persisted. Neither family reaches negative pair energy.

2. **The failure is not an algebra or stability failure.** All 312 histories
   execute; every common-action, inverse, translation/polarity, and
   symmetric-recoil gate passes; all 52 negative-energy controls remain bound.

3. **The energy receiver remains constructive.** Every unbound arm loses pair
   energy while the matched field gains the same amount within `7.44e-12`.
   Dynamic field norm and magnetic energy are nonzero.

4. **The locked window contains no formation event.** All 260 unbound arms
   enter and leave the graph, end with positive energy, and have capture and
   negative-sector counts zero.

5. **Detached radiation remains unqualified.** Every unbound arm has median
   doubled dynamic-field radius two, below the locked threshold four.

6. **The observed export is momentum-dependent.** Its five momentum means are
   described by an approximately linear trend (`R^2=0.99967`) over this narrow
   range. This is a post-run descriptor, not a derived law or uncertainty
   interval.

7. **A lower-momentum test remains logically live.** A zero-intercept
   continuation crosses the exact pair kinetic energy near `p=0.00838`, with
   min/max directional fit crossings `0.00776--0.00919`. Those numbers define
   a new candidate to preregister; they do not rescue FTD-0723.

## Correct statement

The frozen face/edge common action exchanges pair and field energy exactly but
does not capture an unbound derived compact pair anywhere in the registered
`p=0.0200--0.0300` window. The FTD-0722 absolute energy export decreases with
incident momentum, invalidating the constant-export threshold model. Existing
variables remain sufficient for the transaction, while formation remains
unestablished.

## Verification

- preregistration SHA-256:
  `EBAF990F2DF6121DDC4E0E7A79A492B2A30D6D59CD29DF3DE54CC2B266B84CC6`;
- runner SHA-256:
  `05AA224853D3CF4219002975102901C04E0C3E036EFCDA5BC80061E6DDA307E7`;
- result JSON SHA-256:
  `E785C1061CD715B64414DD4685F80DDF2BC4C9A047B1EA2FB124834F45D38895`;
- result CSV SHA-256:
  `6B8B3CE2EB93E6DC3AD7977B0CC388DB2218C026235EC3E2E681842E5C3F60F5`;
- independent certificate:
  `ADDC36647E68D04F1CB9E405AD224D76BA005A9A008FE1981F6CADDDEB2250BD`,
  `146/146 PASS`;
- focused CTest: `1/1 PASS`;
- production defaults, tick, toggles, and scenarios: unchanged.
