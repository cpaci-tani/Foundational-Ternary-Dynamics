# FTD-0604 — Symmetric breathing matter core v1

**Status:** `[THEOREM — EXACT BREATHING BINDING POLYNOMIAL] + [MEASURED —
SINGLE-MODE RESPONSE IS TOO SMALL] + [CLOSED NEGATIVE — REGISTERED STATIC
BRANCH AND PHASE-ROBUST ATTRACTION]`  
**Protocol:**
[`PREREG_SYMMETRIC_BREATHING_MATTER_CORE_v1.md`](../preregistrations/PREREG_SYMMETRIC_BREATHING_MATTER_CORE_v1.md),
SHA-256 `CD8DB5F38A6E9F01BB8EDFAF63664EF940BF0D1F87C1CE8BF5B17789616FDACE`  
**Verdict:** `SYMMETRIC_BREATHING_CORE_STATIC_BRANCH_CLOSED_NEGATIVE`

## Exact binding identity

Each reference trimer has three pair differences of squared length `2`. Under
the registered scale `lambda`, every squared pair distance is therefore
`2 lambda^2`. The selected pair potential is

\[
 V_{ab}=\frac14\left(|X_a-X_b|^2-2\right)^2.
\]

There are three pairs in each of two trimers, hence

\[
 V_{\rm bind}(\lambda)=6(\lambda^2-1)^2,
\quad
 V'_{\rm bind}=24\lambda(\lambda^2-1),
\quad
 V''_{\rm bind}=72\lambda^2-24.
\]

In particular, `V''_bind(1)=48` exactly. The measured minimum curvature
`47.9892` is therefore dominated by the selected stiff binding potential. This
is a theorem about the registered selected composite, not a derivation of a
native binding law.

## Locked measurement

The static optimizer remains interior at all 32 principal-axis phases and
returns

\[
 0.9999533342 \le \lambda_\star \le 0.9999572863.
\]

The breathing response reduces the rigid static Peierls barrier only from
`1.6702306836e-4` to `1.6701460315e-4`, a relative reduction
`5.0683e-5`. It does not change the qualitative force result: only 18 of 32
phases are attractive, and the worst inward-impulse diagnostic remains
negative at `-4.1592453e-4`.

All common-action, inverse, Gauss, energy, and exact integer-translation gates
pass. The largest finite-difference stationarity residual is `4.6078e-8`,
above the locked `1e-8` threshold. Therefore the formal preregistered verdict
is the static-branch closed-negative verdict, even though all curvatures are
positive and the physical force-sign failure is independently unambiguous.
The threshold is not widened after inspection.

## Correct statement

The totally symmetric one-coordinate breathing mode is too stiff to average
away the compact carrier's fractional lattice-phase dependence. This closes
only that deformation family. It does not close shear, orientation, internal
circulation, a deforming field dressing, or an extended low-momentum carrier.
No production, particle, electromagnetic, pole, Lorentz, or unitarity claim
follows.

