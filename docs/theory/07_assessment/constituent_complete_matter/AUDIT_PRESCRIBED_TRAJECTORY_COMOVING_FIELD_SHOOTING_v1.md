# Audit — FTD-0710 prescribed co-moving field shooting

**Verdict:** `[AUDIT PASS — NEGATIVE SCOPE PRESERVED]`

The run reconstructs both parents, deposits exact currents, preserves Gauss
and covariance, and executes the locked 480 GMRES applications. The residual
decreases by about `86x` but remains over the registered field gate. The JSON
therefore records an unresolved solve rather than a physical no-go and leaves
all reciprocal metrics null.

The distinction is essential: slow Krylov convergence can be caused by exact
or near resonances even when a compatible minimum-norm solution exists.
FTD-0710 alone licenses no claim about radiation or new primitives.

