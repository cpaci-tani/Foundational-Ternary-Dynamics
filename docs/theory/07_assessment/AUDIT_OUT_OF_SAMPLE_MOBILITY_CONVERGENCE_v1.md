# Audit — out-of-sample mobility convergence v1

**Ledger ID:** FTD-0654  
**Verdict:** `OUT_OF_SAMPLE_NORMALIZED_MOBILITY_CONSTRUCTIVE`

The campaign uses no FTD-0652 primary velocity and doubles the physical
horizon. All 30 arms and 5,760 forward/reverse records are present. Every
primary advances in all four physical-time windows and passes transverse drift
and mobility thresholds. Exact/coherence/control gates pass by wide margins.

For both locked speeds, maximum absolute mobility error from one and
directional span decrease strictly at each width; maximum translation defect
decreases from width two to four. The normalized verdict therefore follows
without invoking the fallback common-interval classification.

This validates the corrected FTD-0653 diagnostic on unseen data. It does not
retroactively alter FTD-0652's mixed verdict and does not prove an infinite-
width limit. The data license a pole campaign, not a particle claim.

## Reproducibility

- protocol SHA-256: `10C77F2DF5DADA77E583145498ED4D33EF1E2F0A3EF31938BA5A883D301CBEA2`
- runner SHA-256: `A549322D20601B444D9B6DA25C026DD4146260E301874BEFB4E16744E79F2B46`
- result JSON SHA-256: `4512F23E5FDF98186A1873D0A2C382D35D1C45A8AC3E0D8B1FAB15E94B051825`
- arms CSV SHA-256: `56BC5706BA53213A23FD74DDA755C5D0B19E9FE16E10A803A5353E488279826E`
- independent certificate SHA-256: `96EF13174CEDE840AEF48E76B413E07B6B457A36A487A70E3BCDE0DCFC6501BB`

`scripts/proofs/proof_out_of_sample_mobility_convergence.py` independently
recomputes the arm counts, exact/coherence gates, controls, persistence, and
the registered convergence inequalities from the run-of-record files.
