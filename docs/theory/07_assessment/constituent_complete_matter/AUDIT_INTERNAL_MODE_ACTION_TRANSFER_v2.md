# Audit — internal-mode action-transfer ledger v2

**Ledger ID:** FTD-0661  
**Verdict:** `[SELECTED DYNAMICS — MIXED]`

Protocol SHA-256:
`8496808C086B0DA6811A1908EEAE72DBBD9F70BFE84329671E6F75404E4F4814`.
V2 correctly repairs the Poisson-observer zero floor, but its unweighted
tight-frame sum ignores unequal generalized amplitudes and misses covariance
at `0.083494`. Every physical transfer/morphology arm remains favorable; the
overall verdict remains mixed.

Runner SHA-256:
`DFA431DA803A6EB4A253E2EC7E6F5217775C2FAA7B07AC022B7FD69D11F3E67D`.
JSON SHA-256:
`45E0AFAB3E986C72A06252087DDB06F662754964013109E92A205AD37C22C421`.
The joint v2/v3 independent certificate is recorded in the v3 audit.
