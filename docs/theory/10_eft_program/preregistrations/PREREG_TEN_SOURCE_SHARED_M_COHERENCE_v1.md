# PRE-REGISTRATION — Ten-source exact shared-M coherence v1

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0594`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-EVALUATION]`  
**Parent:** `FTD-0593`  

## 1. Question

Does retaining the exact equality of the temporal pulse factor on every full
stencil-eigenvalue shell tighten the FTD-0590 bound enough to close the
preregistered ten-source first-event question?

FTD-0593 is inconclusive because it allows a separate bounded temporal
coefficient on every cubic momentum orbit. The exact pulse law is stronger:
`u_j` depends on momentum only through `M(k)`. Orbits with the same exact `M`
must therefore share one coefficient. This protocol evaluates that single
registered refinement at `N=10`.

## 2. Frozen sector

Retain the FTD-0593 sector exactly:

- production 18-point wave stencil and native state-gradient source;
- zero initial `J`, wave velocity, and manifested velocity;
- one periodic odd quotient with `L in {9,17,33,65}`;
- ten distinct stationary ternary sources at arbitrary sites and signs;
- every source begins present and may be removed once at an arbitrary integer
  tick;
- no Gauss projection, damping, force, movement, transmutation, collision,
  clock, bath, toggle, scenario, or production modification.

The proof domain ends immediately before a hypothetical first descendant
genesis event.

## 3. Exact stencil-eigenvalue key

Let `zeta` be the class of `x` in the cyclotomic field
`Q[x]/(Phi_L(x))`. For a momentum representative `k=(a,b,c)`, define the
integer cyclotomic element

\[
\begin{aligned}
 R_L(k)=6M(k)={}&24
 -2\sum_{p\in\{a,b,c\}}(\zeta^p+\zeta^{-p})\\
 &-\sum_{p<q}
 (\zeta^{p+q}+\zeta^{p-q}+\zeta^{-p+q}+\zeta^{-p-q}).
\end{aligned}
\]

Reduce this Laurent polynomial exactly modulo `Phi_L` to its unique integer
coefficient vector of length `phi(L)`. Two cubic orbits belong to the same
shell if and only if these vectors are exactly equal. Approximate
floating-point clustering is prohibited.

The C++ implementation uses arbitrary-precision integers for the reduced key.
The Python verifier independently constructs `Phi_L`, reduces the same
integer Laurent polynomial, and compares the full shell census and every key
multiplicity.

## 4. Shared-M Gram bound

For an exact eigenvalue shell `S` and nonzero displacement `d`, define

\[
 K_S(d)=\sum_{O\subset S}w_O\chi_O(d),
 \qquad
 \mu_L^{(M)}=\max_{d\ne0}
 \frac{1}{W_L}\sum_S|K_S(d)|.
\]

Because `u_i(M)u_j(M)` is common to every orbit in `S`, each removed-source
cross term obeys

\[
 \left|\sum_{k\ne0}w(k)u_i(M(k))u_j(M(k))e^{-ik\cdot d}\right|
 \le \sum_S|K_S(d)|
 \le \mu_L^{(M)}W_L.
\]

Therefore evaluate

\[
 H_L^{(M)}(10,r)=C_L\sqrt{10-r}
 +Q_L\sqrt{r+\mu_L^{(M)}r(r-1)},
 \qquad r=0,1,\ldots,10,
\]

using the unchanged FTD-0590 `Q_L`, unchanged FTD-0588 `C_L`, and an
exhaustive maximum over the eleven partitions.

## 5. Registered validity gates

- exact cyclotomic polynomial identity
  `x^L-1 = product_{d|L} Phi_d(x)` for every registered `L`;
- exact integer key equality and exact shell membership;
- exact mode-orbit coverage and exact mode count `L^3-1`;
- exact shell coverage with no duplicated or omitted orbit;
- orbit-invariance residual `<=5e-14`;
- direct-character residual `<=5e-13`;
- shell-regrouping residual `<=5e-13` after normalization by `W_L`;
- independent C++/Python scalar agreement `<=5e-12` absolute;
- `mu_L^(M) <= mu_L + 5e-13` on every volume;
- finite bounds and unchanged registered threshold
  `K_GENESIS=1.5163860591519780`.

All floating sums use compensated summation. Ties use the lexicographically
smallest canonical displacement and smallest `r` only for reporting.

## 6. Prohibited searches

No source geometry, polarity, removal tick, observation tick, observation
site, approximate eigenvalue tolerance, shell split/merge choice, or further
norm relaxation is selected or searched. The protocol computes one exact
shell partition fixed by the production stencil.

## 7. Required artifacts

- `engine/include/ftd/eft/ten_source_shared_m_coherence.h`;
- `engine/src/eft/ten_source_shared_m_coherence.cpp`;
- `engine/tests/test_ten_source_shared_m_coherence.cpp`;
- `scripts/proofs/proof_ten_source_shared_m_coherence.py`;
- `engine/results/ftd_0594/windows_msvc_cpu.json`;
- `engine/results/ftd_0594/windows_msvc_cpu.csv`;
- theorem, analysis, and adversarial audit records.

## 8. Outcome map

- If every validity gate passes and
  `max_r H_L^(M)(10,r) < K_GENESIS` on all volumes, verdict:
  `ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_SHARED_M_COHERENCE`.
- If every validity gate passes but any registered maximum is not strictly
  subcritical, verdict:
  `TEN_SOURCE_SHARED_M_BOUND_INCONCLUSIVE`.
- If any exact grouping, coverage, character, tolerance, or cross-language
  gate fails, verdict: `PROTOCOL_INVALID`.

An inconclusive bound is not a positive genesis mechanism. Closure is only a
first-event impossibility theorem in the frozen sector.

## 9. Failure consequence

No grouping rule, tolerance, volume, coefficient, or relaxation is changed
after evaluation. If exact shared-`M` grouping remains inconclusive, FTD-0594
closes this registered norm-refinement branch without authorizing a geometry,
schedule, or phase-feasible-set search under the same identifier.
