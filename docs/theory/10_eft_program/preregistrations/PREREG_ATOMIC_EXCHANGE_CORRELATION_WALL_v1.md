# PRE-REGISTRATION -- Atomic exchange/correlation wall (FTD-0282)

**Status:** `[PRE-REGISTRATION -- LOCK PENDING]` -- design authored; commit/tag
must precede the wall record.  
**Date:** 2026-06-13  
**LEDGER id (reserved):** FTD-0282  
**Git tag (to be applied at lock):** `preregister-atomic-exchange-correlation-wall-v1`  
**Result class (declared):** `[NEGATIVE-BOUNDARY / FIXED-IMPORT TEST]`.

---

## 1. Question

Under exactly the FTD-0278/0279 imports I1+I2+I3, does the atomic sector stop at
restricted mean-field physics, leaving exchange, ortho/para splitting, and
correlation energy unrepresented unless a deeper statistics/configuration-space
import is declared?

The expected result is a boundary, not a failure to be patched.

## 2. Frozen import register

| # | Input | Status |
|---|---|---|
| I1 | clock scalar `omega0 proportional to M_REST` | `[IMPOSED]` |
| I2 | scalar-potential coupling `omega_eff^2 = omega0^2 + 2 omega0 V` | `[IMPOSED -- motivated]` |
| I3 | mode occupancy for two-electron mean-field atoms | `[IMPOSED -- motivated]`; no exchange, no correlation |

No new spin-statistics, antisymmetrization, configuration-space, or correlation
functional import is allowed in FTD-0282.

## 3. Frozen artifact

| Artifact | SHA256 |
|---|---|
| `scripts/exploration/atomic_next_three_campaigns.py` | `f7ef3f73427a90674d70695bbc875fb2de9984e77e691ea7160fd27c31925df8` |

## 4. Run of record

After this file and the artifact are committed and tagged:

```powershell
python scripts/exploration/atomic_next_three_campaigns.py --ftd-0282-wall-record `
  --out scripts/exploration/results/atomic_next_three_2026-06-13/exchange_correlation_wall.json
```

## 5. Frozen verdict logic

- **W-SCOPE:** PASS iff the record uses only I1+I2+I3 and explicitly declares no
  new import.
- **W-NEG:** PASS iff dynamic Pauli exchange, ortho/para exchange splitting, and
  correlation energy are reported as unrepresented rather than fitted.
- **W-LANG:** PASS iff the result states that this is a boundary of the
  conditional sector, not a derivation upgrade.

**Verdict:** `EXCHANGE-CORRELATION-WALL-CONFIRMED` iff W-SCOPE, W-NEG, and
W-LANG pass. `SCOPE-FAIL` if a new import is smuggled in. `LANGUAGE-FAIL` if
the boundary is promoted into an unconditional atomic claim.

## 6. Banned moves

1. No exchange, correlation, Pauli, or ortho/para term may be added.
2. No use of continuum exact helium energy to tune an FTD parameter.
3. No lab-line comparison.
4. No claim that FTD derives full helium or full QM from the five postulates.
5. No promotion of I1, I2, I3, FTD-0013, MC-T4.3, FC-1, or FTD-0270.

## 7. Hash-lock declaration

This document and `scripts/exploration/atomic_next_three_campaigns.py` must be
committed and tagged `preregister-atomic-exchange-correlation-wall-v1` before
the wall record. Any post-lock edit to Sections 2-6 or to the artifact
invalidates v1 and requires a v2.
