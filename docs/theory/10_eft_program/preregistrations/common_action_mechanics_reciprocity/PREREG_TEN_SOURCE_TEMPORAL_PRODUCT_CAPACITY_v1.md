# PRE-REGISTRATION — Ten-source temporal product capacity v1

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0597`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-EVALUATION]`  
**Parent:** `FTD-0596`

## 1. Question

Does the exact same-observation-time range of every pair of finite-removal
pulse factors, combined with the complete FTD-0596 distance-distribution LP,
tighten the ten-source first-event bound enough to close `N=10`?

FTD-0596 used `|u_i(M)u_j(M)|<=1` independently on every exact `M` shell.
The pulse identity forces a strictly smaller negative product:

\[
 -\frac14\le u_i(M)u_j(M)\le1.
\]

This protocol replaces only that product envelope. It does not select a
source configuration, removal schedule, observation time, or polarity.

## 2. Frozen physical sector

Retain FTD-0596 exactly:

- production 18-point wave stencil and native state-gradient source;
- zero initial `J`, wave velocity, and manifested velocity;
- periodic odd `L in {9,17,33,65}`;
- ten distinct stationary ternary sources at arbitrary sites and signs;
- every source begins present and may be removed once at an arbitrary integer
  tick;
- no Gauss projection, damping, force, movement, transmutation, collision,
  clock, bath, toggle, scenario, or production modification.

The proof domain ends immediately before a hypothetical first descendant
genesis event. The `r=0,1,10` values remain the exact/trivial parent values;
only `r=2,...,9` receive the new pair-product refinement.

## 3. Exact temporal pair-product lemma

For one exact stencil-eigenvalue shell, put

\[
 b=(n+\tfrac12)\theta,
 \qquad t_j=T_j\theta,
 \qquad
 u_j=\frac12[\cos(b-t_j)-\cos b].
\]

At fixed `b`, every `u_j` belongs to the interval

\[
 I_b=\left[-\frac{1+\cos b}{2},
             \frac{1-\cos b}{2}\right],
\]

whose width is exactly one. Therefore `|u_i-u_j|<=1`. Together with
`u_i,u_j in [-1,1]`, this implies

\[
 \boxed{-\frac14\le u_i u_j\le1}.
\]

The lower endpoint follows by writing opposite-sign values as `a,-c` with
`a,c>=0` and `a+c<=1`, hence `ac<=1/4`. The bound is exact as a two-coordinate
projection of the continuous phase set. No rational-independence or density
assumption is used.

## 4. Signed exact-shell kernel

Retain the exact cyclotomic `M` shells `S` and shell displacement character

\[
 K_S(d)=\sum_{O\subset S}w_O\chi_O(d),
 \qquad W_L=\sum_{k\ne0}w(k).
\]

For each nonzero displacement define

\[
 P_L(d)=\sum_S[K_S(d)]_+,
 \qquad
 N_L(d)=\sum_S[-K_S(d)]_+.
\]

For arbitrary removal ticks at one common observation time and either value
of the fixed polarity product `q_iq_j`, Section 3 gives

\[
 \left|\sum_SK_S(d)u_i(S)u_j(S)\right|
 \le\max\left(P_L(d)+\frac14N_L(d),
              N_L(d)+\frac14P_L(d)\right).
\]

Register the normalized temporal-product kernel

\[
 \boxed{\tau_L(d)=\frac1{W_L}
 \max\left(P_L(d)+\frac14N_L(d),
           N_L(d)+\frac14P_L(d)\right).}
\]

The identity

\[
 \tau_L(d)=\frac{5}{8}\kappa_L(d)
 +\frac{3}{8W_L}\left|\sum_SK_S(d)\right|
\le\kappa_L(d)
\]

must be verified independently. Here `kappa_L` is the FTD-0596 absolute-shell
kernel. Equality is permitted when all shell characters have one sign.

## 5. Registered distance-distribution bound

Use exactly the FTD-0596 feasible polytope:

- complete cubic displacement-orbit distribution `a_j`;
- nonnegativity and `sum a_j=r-1`;
- orbit capacities `a_j<=min(v_j,r-1)`;
- exact inherited axial cap `a_axial<=2e_L(r)/r`;
- every Fourier-positivity inequality
  `1+sum_j P_{ell j}a_j>=0`.

For each `L` and `r=2,...,9`, solve

\[
 q_L^{\rm time}(r)=\max_a\sum_j\tau_j a_j.
\]

Then

\[
 G_L^{\rm time}(r)=r[1+q_L^{\rm time}(r)],
\qquad
 H_L^{\rm time}(10,r)=C_L\sqrt{10-r}
 +Q_L\sqrt{G_L^{\rm time}(r)}.
\]

Use the parent exact/trivial values at `r=0,1,10`. The reported ten-source
bound is the maximum over `r=0,...,10`.

## 6. Deterministic solve and certificate

Use the FTD-0596 SciPy/HiGHS dual-simplex cutting-plane protocol unchanged:

- primal/dual feasibility and optimality tolerance `1e-10`;
- add every Fourier orbit below `-1e-12`, or the lexicographically first
  most-negative orbit if the minimum is negative but above that threshold;
- terminate only when the global Fourier minimum is at least `-1e-10`;
- no extra graph, integrality, triangle, semidefinite, configuration, or
  temporal cut.

For nonnegative dual vectors `y,z` and free `lambda`, independently verify

\[
 \lambda+z_j-\sum_\ell y_\ell P_{\ell j}\ge\tau_j.
\]

Let

\[
 \epsilon=\max_j\left[\tau_j-\lambda-z_j
 +\sum_\ell y_\ell P_{\ell j}\right]_+,
\]

and retain the registered padding

\[
 \delta=5\times10^{-12}\left(1+\sum_\ell y_\ell\right)+10^{-12}.
\]

The authoritative objective is

\[
 U_L(r)=(r-1)(\lambda+\epsilon+\delta)
 +\sum_\ell y_\ell+\sum_j u_jz_j.
\]

Require `U_L(r)>=q_L(r)-1e-10`, primal/dual gap at most `1e-8`, dual
nonnegativity to `1e-12`, and independent C++/Python agreement within
`5e-12`.

## 7. Registered validity gates

- exact FTD-0594 cyclotomic shell keys, partitions, and multiplicities;
- exact FTD-0596 displacement and momentum orbit coverage;
- direct/orbit character residual `<=5e-13` on every active shell/dual row;
- reconstruction of `P_L(d)`, `N_L(d)`, `kappa_L(d)`, and `tau_L(d)` within
  `5e-12`;
- `0<=tau_L(d)<=kappa_L(d)+5e-12` for every displacement;
- exact verification of the alternate `5/8+3/8` formula within `5e-12`;
- global primal Fourier minimum `>=-1e-10`;
- all dual sign, feasibility, padding, gap, and cross-language gates in
  Section 6;
- final bound no weaker than FTD-0596 by more than `1e-8`;
- finite bounds and unchanged `K_GENESIS=1.5163860591519780`.

All orbit and shell ordering is lexicographic. Floating reductions use
compensated summation. The independent verifier uses at least 80 decimal
digits for active character rows and certificate reconstruction.

## 8. Prohibited adaptation

No source set, polarity, removal schedule, observation tick/site,
rational-independence assumption, time scan, integral rounding, new spatial
cut, semidefinite refinement, threshold-dependent solver change, or production
modification is permitted in FTD-0597.

## 9. Required artifacts

- `engine/include/ftd/eft/ten_source_temporal_product_capacity.h`;
- `engine/src/eft/ten_source_temporal_product_capacity.cpp`;
- `engine/tests/test_ten_source_temporal_product_capacity.cpp`;
- `scripts/proofs/generate_ten_source_temporal_product_capacity.py`;
- `scripts/proofs/proof_ten_source_temporal_product_capacity.py`;
- `engine/results/ftd_0597/windows_msvc_cpu.json`;
- `engine/results/ftd_0597/windows_msvc_cpu.csv`;
- theorem, analysis, and adversarial audit records.

## 10. Outcome map

- If all gates pass and every certified partition is strictly subcritical,
  verdict:
  `ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_TEMPORAL_PRODUCT_CAPACITY`.
- If all gates pass but any certified partition is not strictly subcritical,
  verdict: `TEN_SOURCE_TEMPORAL_PRODUCT_BOUND_INCONCLUSIVE`.
- If any shell, orbit, primal, dual, certificate, or cross-language gate
  fails, verdict: `PROTOCOL_INVALID`.

An inconclusive upper bound is not a genesis witness. Closure is only a
first-event impossibility theorem in the frozen sector.

## 11. Failure consequence

No additional temporal constraint is added after evaluation. An inconclusive
result closes this exact pair-product projection as a decider. The full
higher-order common-time feasible set would remain a separate theorem branch
requiring a new preregistration.
