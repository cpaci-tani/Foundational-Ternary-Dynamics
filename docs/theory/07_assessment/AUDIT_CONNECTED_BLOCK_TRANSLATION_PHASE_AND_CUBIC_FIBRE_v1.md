# Audit — connected-block translation phase and cubic fibre

**Campaigns:** FTD-0630 through FTD-0633  
**Verdict:** `[MIXED: INVALID / CLOSED NEGATIVE / CONSTRUCTIVE / CONSTRUCTIVE]`

## Audit findings

1. FTD-0630 reproduces every stored finite difference. The body-half state has
   four negative transverse curvature instances across two orientations; the
   fully shifted control has six positive instances. The locked covariance
   residual is `2.740863e-8 > 1e-8`, so the execution-invalid verdict is
   mandatory.
2. FTD-0631 initializes both cap-two arms but records no valid Hessian
   neighbourhood. Its closed-negative verdict is the correct frozen-chart
   disposition, not an infrastructure excuse.
3. FTD-0632 enumerates 66/66 registered geometries. Independent CSV analysis
   reproduces the exact validity law `pass iff multiplicity <= cap`, the
   failure counts `(26,14,0)`, maximum multiplicity eight, cyclic histogram
   equality, and separation/Gauss gates.
4. FTD-0633 passes every registered reduced stationarity, translation,
   action, energy, fibre, covariance, repeated-rest, and inverse gate. Its
   covariance residual `7.78e-10` has only 1.29 times margin and must remain an
   engine-resolution result.
5. The cap-eight result is observer-only. No production-site multiplicity or
   new force was introduced.

Independent certificates:

- `proof_connected_block_translation_curvature.py`;
- `proof_connected_block_full_half_static_refinement.py`;
- `proof_connected_block_cubic_eight_fibre.py`;
- `proof_connected_block_eight_fibre_static_basin.py`.
