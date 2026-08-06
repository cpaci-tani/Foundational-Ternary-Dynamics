# Audit — FTD-0612 uniform single-core stationary refinement v1

**Status:** `[AUDIT — CONSTRUCTIVE SELECTED STATIC CORE]`
**Verdict:** `REFINED_UNIFORM_SINGLE_CORE_STATIC_CONSTRUCTIVE`

- protocol SHA: `B0C93907...2002`;
- runner: `engine/tests/test_uniform_single_core_stationary_refinement.cpp`;
- certificate:
  `scripts/proofs/proof_uniform_single_core_stationary_refinement.py`;
- independent checks: 16/16 pass;
- result: `engine/results/ftd_0612/`.

The result reproduces the FTD-0611 fingerprint, uses one accepted undamped
Newton step, preserves every nine-mode and direct-field gate, and produces an
exact 64-tick fixed point under the observer transaction. No dynamics or
ontology changed. The constructive scope is a compact rest state in a
uniformly neutralized periodic laboratory; mobility and isolated momentum are
not included.
