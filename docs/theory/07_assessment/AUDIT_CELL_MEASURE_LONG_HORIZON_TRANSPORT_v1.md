# Audit — cell-measure long-horizon transport v1

**Ledger ID:** FTD-0650  
**Verdict:** `CELL_MEASURE_LONG_HORIZON_EXECUTION_INVALID`

The preregistered runner and executable were built without changing the
FTD-0649 action. The process remained numerically active, bounded in memory,
and free of exceptions. Worker-thread turnover showed completion of the first
six width-two histories and entry into the second batch, but the v1 runner did
not checkpoint per-arm records. Consequently none of the registered physical
gate families has complete coverage.

The termination decision was made from runtime feasibility, not from inspected
physics residuals. The first batch required approximately 71 minutes; wider
systems remained, while the registered CTest window was six hours. The locked
protocol expressly assigns execution-invalid when coverage or record
completeness prevents evaluation.

Run provenance:

- protocol: `2670F4B0E1C67911D85FDC80DE64F5DFB15EC54F7B76E3C20882AD66F93CD131`;
- runner: `DF885AFA30D5C93712277741C01D675DD69B7C6D441E6FDE10D60FC62F24D3D9`;
- executable: `C9ADE1300835C715F89429FFAE4ACB69F389B0166C28D92BABAEB84D90E02D0C`;
- invalid-result JSON: `engine/results/ftd_0650/ftd_0650_cell_measure_long_horizon_transport_v1.json`.

No claim about long-horizon mobility, pinning, anisotropy, translation--field
momentum balance, or state-only reversibility follows. A fresh solver
qualification may repair execution, but FTD-0650 v1 remains execution-invalid.
