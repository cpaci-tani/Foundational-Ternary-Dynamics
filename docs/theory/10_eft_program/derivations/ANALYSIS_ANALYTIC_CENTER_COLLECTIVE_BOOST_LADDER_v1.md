# Analytic-center collective boost ladder v1

**Campaign:** FTD-0643  
**Status:** `[EXECUTION INVALID — PREREGISTERED COVERAGE ARITHMETIC DEFECT]`  
**Production impact:** none

The locked arm list contains one rest arm, 21 positive ladder arms, six sign
mirrors, and four cyclic controls: 32 arms. Section 3 incorrectly states that
this totals 29, and the runner's locked coverage gate requires 29. The complete
32-arm execution therefore returns
`ANALYTIC_CENTER_BOOST_EXECUTION_INVALID`.

The raw trajectories are retained but cannot issue a boost verdict. Every arm
did execute coherently and invert; every nonzero canonical arm had mobility
above `0.78`; and the integrated soft fraction exceeded `0.9999995` in every
high-amplitude arm. Those facts may motivate a corrected protocol but are not
promoted from this invalid campaign.

The cyclic implementation also changed only the launch direction while keeping
the orientation-zero dressed object fixed. Its differences therefore mix
object orientation with lattice direction and are anisotropy probes, not
whole-state cubic covariance tests. A corrected version must rotate the entire
initial matter-plus-dressing state before applying the cyclic launch.

