# Audit — FTD-0621 ternary block-bipole Peierls scaling

**Status:** `[AUDIT — EXACT INTEGER REPRESENTABILITY CLOSED POSITIVE;
DYNAMICAL MATTER OPEN]`  
**Verdict:** `INTEGER_TERNARY_EXTENSION_SUPPRESSES_PEIERLS`

- protocol SHA-256: `905819BD...8CB31`;
- observer header SHA-256: `DE06244A...08B6A`;
- observer source SHA-256: `B8BF877D...DD89C`;
- runner SHA-256: `7A772DAE...F7513`;
- result JSON SHA-256: `D6ED6A0B...26383`;
- CSV SHA-256: `693AB224...28B2`;
- certificate SHA-256: `D6489E11...95670`;
- independent certificate: 29/29 checks pass;
- run of record: `engine/results/ftd_0621/`;
- production dynamics: unchanged.

All 90 held-out arms pass exact ternary counts, neutrality, spectral identity,
cubic covariance, finite-volume, monotonicity, scaling, and endpoint gates.
The least `w=5` to `w=35` suppression factor is `220.231`; the largest final
relative pinning index is `1.39720e-5`. Measured slopes remain within the
registered bands around `E~w^5`, `B~w^2`, and `B/E~w^-3`.

Two invalid executions are retained. They exceeded the locked structure-factor
and then structure-factor-only numerical gates because of accumulated ordinary
double summation. Compensated accumulation and an independent closed-form
versus finite-sum comparison close the same unchanged identities. No tolerance
or physical gate was relaxed.

The result is not a mobile carrier. Its absolute barrier increases, constituent
count is `2w^3`, and total field energy increases as `~w^5`. The valid claim is
that exact primitive ternary extension can suppress *relative* lattice pinning.
A connected local action, stable shape, coherent translation, fixed physical
normalization, and joint translation-defect scaling remain open.

