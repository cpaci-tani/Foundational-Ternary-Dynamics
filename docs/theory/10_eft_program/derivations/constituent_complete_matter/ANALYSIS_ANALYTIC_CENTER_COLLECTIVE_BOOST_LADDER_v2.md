# Analytic-center collective boost ladder v2

**Campaign:** FTD-0644  
**Status:** `[MEASURED — COHERENT TRANSPORT] + [CLOSED NEGATIVE — LOCKED
CUBIC CONJUNCTION]`  
**Production impact:** none

FTD-0644 repairs the FTD-0643 arm count and rotates the complete dressed state
in its cubic controls. All 32 arms execute, remain coherent, conserve energy,
and invert. The rest control remains fixed; every one of the 21 nonzero
canonical boosts is classified mobile, including `p=0.001875`. High boosts
cross multiple site layers in every direction with mobility `0.9805..0.9860`,
soft fraction above `0.9999995`, shape RMS below `0.00791`, and longitudinal
dressing residual below `0.292`.

The locked verdict is nevertheless
`ANALYTIC_CENTER_V2_DIRECTIONAL_TRANSPORT_CLOSED`: cubic residual is
`5.1404e-6` against `1e-7`.

The independent certificate decomposes that residual. Center, momentum,
shape, field energy, dressing residual, hops, and inversion all satisfy the
cubic tolerance. The sole failure is soft fraction. The runner projects a
rotated state onto the unrotated FTD-0640 eigenvectors; the six-dimensional
soft subspace is covariant, but that observer basis was not rotated with the
state. This is an observer-coordinate defect, not a measured dynamical
anisotropy. Because soft fraction was part of the locked conjunction, the v2
verdict remains closed.

The admissible successor changes only the covariance observer: rotate the
complete analytic modal basis with the state. No amplitude, physical gate, or
tolerance may change.

