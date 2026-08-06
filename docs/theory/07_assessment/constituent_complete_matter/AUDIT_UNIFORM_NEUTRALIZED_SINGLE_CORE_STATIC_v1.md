# Audit — FTD-0611 uniform-neutralized single-core static state v1

**Status:** `[AUDIT — POSITIVE BASIN; LOCKED PRECISION GATE CLOSED]`
**Verdict:** `UNIFORM_NEUTRALIZED_COMPACT_STATIC_CLOSED_NEGATIVE`

- protocol SHA: `45FC3250...9B69`;
- runner: `engine/tests/test_uniform_neutralized_single_core_static.cpp`;
- certificate: `scripts/proofs/proof_uniform_neutralized_single_core_static.py`;
- independent checks: 18/18 pass;
- result: `engine/results/ftd_0611/`.

The record has complete search, differential, field, covariance, and
transaction coverage. It resolves a stable nine-dimensional basin, but misses
two numerical cutoffs by factors of `4.4` and `1.49`. Treating this as a
compact-matter no-go would be false: every signed perturbation rises and the
minimum Hessian eigenvalue is positive by three orders of magnitude over its
gate. The correct disposition is a closed locked protocol with a separately
registered precision refinement.
