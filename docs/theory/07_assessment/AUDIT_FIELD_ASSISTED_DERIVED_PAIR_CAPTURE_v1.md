# Audit — FTD-0722 field-assisted derived-pair capture v1

**Status:** `[AUDIT PASS — EXECUTION CONSTRUCTIVE / LOCKED CAPTURE CLOSED
NEGATIVE]`  
**Date:** 2026-07-28

## Findings

1. **The action executes without a new primitive.** All 104 complete
   matter/current/field histories satisfy the established atomic identities,
   inverse, covariance, and symmetric-recoil gates.

2. **The field is a genuine energy receiver.** Every unbound encounter loses
   pair internal energy while field energy rises by the same amount within
   `5.19e-12`. The dynamic field and magnetic energy are nonzero.

3. **The locked formation claim fails.** Every unbound arm enters and exits
   the interaction graph, with exactly two graph transitions and positive
   final pair internal energy. Capture count is `0/52`.

4. **The negative sector itself remains viable.** All 52 already-bound
   controls remain connected and negative for the full horizon.

5. **Detached radiation is not qualified.** The unbound dynamic-field median
   doubled radius is three, below the preregistered threshold four. The proper
   statement is dynamic field excitation and energy export, not photon or
   outgoing-radiation detection.

6. **The negative result is narrow.** One incident momentum, finite volume,
   finite time, and one selected compact well were tested. The result neither
   proves that field-assisted capture is impossible nor licenses changing this
   locked candidate after inspection.

## Correct statement

Existing variables support an exact reciprocal encounter that transfers
`12.70%--14.54%` of the registered incoming pair energy into the matched
field, but the transfer is insufficient to form a negative-energy bound pair
at `p=0.07`. The next test is a separately preregistered incident-energy
window, not a post-hoc reduction of this run's momentum.

## Verification

- preregistration SHA-256:
  `19594ECA39EC9489A3D07BC1AC04021BC1D4FC3597B0E8AFEE55312A51E09C68`;
- result JSON SHA-256:
  `1AAE192D20C5B745D079307B7A3C64B394C9C15ED5E168FF3B1DD2DBFC85E582`;
- result CSV SHA-256:
  `546A36472E79698D4554AB942EBD8EE13820AFE616E4799D00E9E9AE1DA1B9C5`;
- independent certificate: `57/57 PASS`;
- focused CTest: `1/1 PASS`;
- production defaults, tick, toggles, and scenarios: unchanged.
