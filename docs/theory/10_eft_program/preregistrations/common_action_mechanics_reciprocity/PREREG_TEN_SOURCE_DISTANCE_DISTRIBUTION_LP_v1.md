# PRE-REGISTRATION — Ten-source distance-distribution LP v1

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0596`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-EVALUATION]`  
**Parent:** `FTD-0595`

## 1. Question

Does enforcing every Fourier-positivity constraint on the complete cubic-orbit
pair-distance distribution tighten the exact shared-`M` Gram bound enough to
close the ten-source first-event question?

FTD-0595 distinguishes axial nearest-neighbor pairs from one complementary
class. It does not require that the remaining pair coefficients arise from one
simultaneously realizable finite set. This protocol replaces that two-class
relaxation by the Delsarte linear-programming relaxation of the complete
translation association scheme of `Z_L^3`.

## 2. Frozen physical sector

Retain FTD-0595 exactly:

- production 18-point wave stencil and native state-gradient source;
- zero initial `J`, wave velocity, and manifested velocity;
- periodic odd `L in {9,17,33,65}`;
- ten distinct stationary ternary sources at arbitrary sites and signs;
- every source begins present and may be removed once at an arbitrary integer
  tick;
- no Gauss projection, damping, force, movement, transmutation, collision,
  clock, bath, toggle, scenario, or production modification.

The proof domain ends immediately before a hypothetical first descendant
genesis event. The FTD-0594 `r=10` partition remains unchanged because it is
already subcritical on all four volumes.

## 3. Complete cubic distance distribution

Let `D_0={0},D_1,...,D_m` be all displacement orbits of `Z_L^3` under signed
coordinate permutations, and let `v_j=|D_j|`. For an `r`-site set `X`, define

\[
 a_j(X)=\frac1r\left|\{(x,y)\in X^2:y-x\in D_j\}\right|.
\]

For every realizable `X`,

\[
 a_0=1,\qquad a_j\ge0,\qquad
 \sum_{j=1}^m a_j=r-1,\qquad
 a_j\le\min(v_j,r-1).
\]

Retain the exact FTD-0595 axial capacity as the additional registered cut

\[
 a_{\rm axial}\le \frac{2e_L(r)}r.
\]

No further graph-capacity, integrality, triangle, semidefinite, or
configuration-specific cut may be added after evaluation.

## 4. Fourier-positivity constraints

For each nonzero momentum orbit `K_ell`, choose its lexicographically smallest
representative `k_ell` and define the real orbit character

\[
 P_{\ell j}=\frac1{v_j}\sum_{d\in D_j}
 \cos\!\left(\frac{2\pi}{L}k_\ell\cdot d\right).
\]

Rotationally symmetrizing the autocorrelation of `X` preserves `a_j`. Its
Fourier transform is nonnegative, hence every realizable distance distribution
satisfies

\[
 1+\sum_{j=1}^mP_{\ell j}a_j\ge0
 \quad\text{for every }\ell=1,\ldots,m.
\]

These constraints, the normalization, and the registered upper bounds define
the sole FTD-0596 feasible polytope. All displacement and momentum orbits must
be covered exactly. Character values are evaluated independently by an orbit
formula and a direct Fourier sum.

## 5. Registered Delsarte bound

Let `kappa_j` be the exact shared-`M` kernel of FTD-0595 on `D_j`. For each
`L` and `r=2,...,9`, solve

\[
 q_L(r)=\max_a\sum_{j=1}^m\kappa_j a_j
\]

over the polytope in Sections 3--4. Then

\[
 \sum_{x<y}\kappa_L(x-y)\le\frac r2q_L(r),
 \qquad
 G_L^{\rm DD}(r)=r\,[1+q_L(r)],
\]

and the registered ten-source partition bound is

\[
 H_L^{\rm DD}(10,r)=C_L\sqrt{10-r}
 +Q_L\sqrt{G_L^{\rm DD}(r)}.
\]

Use the exact trivial values at `r=0,1` and the unchanged FTD-0594 value at
`r=10`. The reported ten-source bound is the maximum over `r=0,...,10`.

## 6. Deterministic solve and dual certificate

Use SciPy/HiGHS dual-simplex with feasibility and optimality tolerances
`1e-10`. A deterministic cutting-plane implementation may begin from the
normalization and upper bounds, but must add every Fourier orbit whose current
symmetrized transform is below `-1e-12`; if none meet that threshold while the
minimum is negative, add the lexicographically first most-negative orbit.
Terminate only when the global minimum is at least `-1e-10`.

The solver objective is not authoritative. For constraints written as

\[
 -P_\ell a\le1,\qquad a_j\le u_j,
 \qquad \sum_ja_j=r-1,\qquad a_j\ge0,
\]

record nonnegative dual vectors `y,z` and free scalar `lambda`. Independently
verify every dual inequality

\[
 \lambda+z_j-\sum_\ell y_\ell P_{\ell j}\ge\kappa_j.
\]

Let

\[
 \epsilon=\max_j\left[\kappa_j-lambda-z_j
 +\sum_\ell y_\ell P_{\ell j}\right]_+,
\]

and register the conservative coefficient-error padding

\[
 \delta=5\times10^{-12}\left(1+\sum_\ell y_\ell\right)+10^{-12}.
\]

The certified value is

\[
 U_L(r)=(r-1)(\lambda+\epsilon+\delta)
 +\sum_\ell y_\ell+\sum_j u_jz_j.
\]

Use `U_L(r)`, never the raw primal objective, in the genesis bound. Require
`U_L(r)>=q_L(r)-1e-10`, primal/dual gap at most `1e-8`, nonnegative duals to
`1e-12`, and independent C++/Python agreement of all active character rows,
kernel values, certificate objectives, and final bounds within `5e-12`.

## 7. Registered validity gates

- exact FTD-0594 cyclotomic shell partition and mode coverage;
- exhaustive displacement and momentum cubic-orbit coverage;
- direct/orbit character residual `<=5e-13` on every active dual row;
- FTD-0595 kernel reproduction `<=5e-12` and axial-cap reproduction exactly;
- global primal Fourier minimum `>=-1e-10`;
- all dual sign, feasibility, padding, gap, and cross-language gates in
  Section 6;
- Delsarte bound no weaker than FTD-0595 by more than `1e-8`;
- finite bounds and unchanged `K_GENESIS=1.5163860591519780`.

All orbit ordering is lexicographic. All floating reductions use compensated
summation. Python coefficient evaluation uses at least 80 decimal digits for
the independent certificate reconstruction.

## 8. Prohibited adaptation

No source set, polarity, removal schedule, observation tick/site, integral
rounding, extra distance cut, graph-capacity cut beyond the inherited axial
cap, semidefinite refinement, threshold-dependent solver change, or temporal
phase constraint is permitted in FTD-0596.

## 9. Required artifacts

- `engine/include/ftd/eft/ten_source_distance_distribution_lp.h`;
- `engine/src/eft/ten_source_distance_distribution_lp.cpp`;
- `engine/tests/test_ten_source_distance_distribution_lp.cpp`;
- `scripts/proofs/proof_ten_source_distance_distribution_lp.py`;
- `engine/results/ftd_0596/windows_msvc_cpu.json`;
- `engine/results/ftd_0596/windows_msvc_cpu.csv`;
- theorem, analysis, and adversarial audit records.

## 10. Outcome map

- If all gates pass and every certified partition is strictly subcritical,
  verdict:
  `ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_DISTANCE_DISTRIBUTION_LP`.
- If all gates pass but any certified partition is not strictly subcritical,
  verdict: `TEN_SOURCE_DISTANCE_DISTRIBUTION_LP_INCONCLUSIVE`.
- If any orbit, primal, dual, certificate, or cross-language gate fails,
  verdict: `PROTOCOL_INVALID`.

An inconclusive upper bound is not a genesis witness. Closure is only a
first-event impossibility theorem in the frozen sector.

## 11. Failure consequence

No extra cut is added after evaluation. An inconclusive result closes the
Fourier-positive Delsarte-LP branch at this registered strength. The remaining
uniform route is the exact integer-time phase-feasible set, under a new lock.
