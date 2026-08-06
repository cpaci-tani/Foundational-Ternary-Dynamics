# Audit — FTD-0711 co-moving field Fourier solvability

**Verdict:** `[AUDIT PASS — FINITE-VOLUME NULLSPACE INCOMPATIBILITY]`

The script validates its Fourier convention against the independent C++
GMRES run before using the SVD. The residual equals the nullspace source
projection, is supported on eight exact body-diagonal modes, and survives
minimum-norm reconstruction. This is a source-compatibility failure rather
than an iterative convergence failure.

The status remains a finite-volume numerical fact. No volume limit or
infinite-lattice radiation rate was computed. The correct consequence is to
alter the candidate source history under a new registration, not to delete a
mode or regularize the inverse.

