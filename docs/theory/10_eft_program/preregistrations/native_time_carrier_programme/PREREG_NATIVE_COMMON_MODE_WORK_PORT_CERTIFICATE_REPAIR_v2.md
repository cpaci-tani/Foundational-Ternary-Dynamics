# Pre-registration — Native common-mode work-port certificate repair v2

**Identifier:** `FTD-0987`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE REPAIR EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Parent disposition

The immutable FTD-0986 execution passed every hash and every reached
common/relative, covariance, action-angle, seam-symplectic, energy, inverse,
and production-capacity gate. It then exposed two verifier-only defects:

1. the ownership source check requested the nonexistent contiguous phrase
   `prepositioned complete local work pairs`, while the theorem states both
   `one complete local pair` and `prepositioned local reserves`; and
2. SymPy's `Expr.coeff` API requires a concrete integer exponent and rejects
   the symbolic exponent `m+1`, even though multiplication by the Laurent
   unit `z^m` is irrelevant to the extremal coefficient.

No physical identity or classifier failed. The parent protocol and proof are
preserved unchanged.

## 2. Frozen parents

| Source | Frozen SHA-256 |
|---|---|
| `PREREG_NATIVE_COMMON_MODE_WORK_PORT_OWNERSHIP_DISCRIMINATOR_v1.md` | `7E5E00C9262D3E6AF5D2BBD41D7F2845D4744D902157C32BADA7F6787D86AECF` |
| `proof_native_common_mode_work_port_ownership.py` | `88B3296231CAFA4F98E7778B82BE00538D9E66BD3EC54BE1E628F0E1EFBD5DD3` |

All twelve frozen source hashes inherited from FTD-0986 remain binding.

## 3. Exactly permitted repairs

The wrapper may make exactly two in-memory full-line substitutions:

1. change the ownership marker from `prepositioned complete local work pairs`
   to the exact theorem phrase `one complete local pair`;
2. replace the symbolic-exponent coefficient request with the normalized
   expression

```text
sp.expand(sp.cancel(extremal_term / z**m)).coeff(z, 1) == -b * u_max
```

Dividing by `z^m` is exact because `z` is nonzero and `z^m` is a Laurent
unit. It shifts the upper extremal exponent from `m+1` to `1` without changing
its coefficient.

## 4. Integrity gates

- the two parent hashes and this repair-protocol hash must match;
- each old full-line anchor must occur exactly once and each repaired line
  must be absent before repair;
- exactly two in-memory substitutions are permitted;
- every inherited check must pass and the inherited result must remain
  Outcome B;
- parent bytes must be unchanged afterward.

Any failure gives Outcome D. Passing licenses only the FTD-0986 conditional
native-chart/priced-ownership conclusion. No engine, production, `G*`,
Born/Bell, Hilbert, mass, selector-energy, or completeness promotion follows.
