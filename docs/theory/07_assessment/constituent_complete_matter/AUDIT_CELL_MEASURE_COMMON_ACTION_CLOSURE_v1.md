# Audit — cell-measure common-action closure v1

**Ledger ID:** FTD-0649  
**Verdict:** `CELL_MEASURE_RECIPROCAL_COMMON_ACTION_CONSTRUCTIVE`

The preregistered 45-arm matrix is complete. All widths, orientations, launch
axes, sign mirrors, and zero controls execute forward and reverse. The same
polarity factor appears in source, current, Gauss density, and gather; the
same field coefficient governs energy, work, and recoil; scaled dispersion
governs both energy and velocity. All action and inverse gates pass.

The dense solver's wall-clock failure and the low-rank solver's nonconvergence
are retained as implementation provenance. They do not close the physical
candidate because the matrix-free Newton--Krylov implementation solves the
unchanged locked equations and satisfies the original tolerances without
altering any registered arm or coefficient.
The separate `connected_moore_block_matrix_free_solve` CTest agrees with the
dense root at `1e-9` state tolerance and recovers the initial state by the
matrix-free inverse.

Run-of-record hashes:

- protocol: `612172E79EF58526FC4EE02DE84EDEA0AC6EEF6EDF1F52160EF8F35363AA7C5A`;
- runner: `256DD29974620F0C1F0B52A09D362CB9F8B03DA1CC2E7A24965C5134E8DEC53D`;
- JSON: `7549A16D786279EE9E2AAC21BEC3B6C1CAD6712DA36100068E0126D1A5FBD9D3`;
- arm CSV: `301C188C4A4F14D96A2E9E197BDF1FF0F5FF5649E908906FA32AD94E5D2B88A1`.
- independent certificate: `2A844C1E85A0BD8D821F2765677A58194646C4FB0EBE0E4D1D4360502E4BBAE4`.

The constructive result licenses long-horizon cell-measure dynamics. It does
not establish a native particle, gapless pole, conserved reaction-complete
charge, gauge ontology, common cone, Lorentz recovery, or production adoption.
