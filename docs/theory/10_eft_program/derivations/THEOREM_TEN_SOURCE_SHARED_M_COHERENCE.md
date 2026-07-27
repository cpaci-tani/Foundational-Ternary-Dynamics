# Theorem Boundary — Ten-Source Exact Shared-M Coherence

**FTD ID:** FTD-0594  
**Status:** `[THEOREM — EXACT CYCLOTOMIC EIGENSHELL GRAM BOUND]` +
`[NUMERICAL FACT — FOUR EXHAUSTIVE SHELL/DISPLACEMENT NORMS]` +
`[INCONCLUSIVE — N=10]` +
`[CLOSED NEGATIVE — SHARED-M NORM-REFINEMENT BRANCH]`  
**Date:** 2026-07-26  
**Verdict:** `TEN_SOURCE_SHARED_M_BOUND_INCONCLUSIVE`

## 1. Exact eigenshell partition

For `zeta=exp(2 pi i/L)`, the production stencil eigenvalue has the exact
cyclotomic representative

\[
\begin{aligned}
 R_L(k)=6M(k)={}&24
 -2\sum_p(\zeta^p+\zeta^{-p})\\
 &-\sum_{p<q}
 (\zeta^{p+q}+\zeta^{p-q}+\zeta^{-p+q}+\zeta^{-p-q}).
\end{aligned}
\]

Reduction modulo `Phi_L` gives a unique integer coefficient vector in the
basis `1,zeta,...,zeta^(phi(L)-1)`. Equality of these vectors is therefore
equivalent to exact equality of `M`; no floating tolerance enters the shell
partition.

## 2. Shared-M Gram theorem

Let `S` range over exact `M` shells and define

\[
 K_S(d)=\sum_{O\subset S}w_O\chi_O(d),
 \qquad
 \mu_L^{(M)}=\max_{d\ne0}
 \frac{1}{W_L}\sum_S|K_S(d)|.
\]

The normalized pulse factor `u_j(M)` is common to all orbits in `S`. Thus for
each removed-source pair,

\[
 \left|\sum_{k\ne0}w(k)u_i(M(k))u_j(M(k))e^{-ik\cdot d}\right|
 \le \mu_L^{(M)}W_L.
\]

Consequently

\[
 \boxed{
 |J|\le C_L\sqrt{N-r}
 +Q_L\sqrt{r+\mu_L^{(M)}r(r-1)}.}
\]

This inequality is uniform in distinct source sites, polarities, one-time
removal ticks, observation time, and observation site in the frozen sector.
Because a shell is a union of cubic orbits,
`mu_L^(M)<=mu_L` follows directly from the triangle inequality.

## 3. Exact shell census and evaluation

| `L` | `deg Phi_L` | cubic orbits | exact `M` shells | multi-orbit shells | max orbits/shell | `mu_L` | `mu_L^(M)` |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 9 | 6 | 34 | 29 | 1 | 6 | 0.36102817687951227 | 0.35925757426614324 |
| 17 | 16 | 164 | 164 | 0 | 1 | 0.36250597734262191 | 0.36250597734262191 |
| 33 | 20 | 968 | 947 | 1 | 22 | 0.36267617904631827 | 0.36267617904631827 |
| 65 | 48 | 6544 | 6544 | 0 | 1 | 0.36273662797281120 | 0.36273662797281120 |

The nontrivial `L=33` shell does not reduce the maximizing displacement norm.
At `L=65`, every cubic orbit has a distinct exact stencil eigenvalue, so the
shared-`M` refinement is identically the FTD-0590 orbit bound.

| `L` | maximizing `r` | shared-`M` bound | `K_GENESIS-bound` |
|---:|---:|---:|---:|
| 9 | 9 | 1.5640918360159304 | -0.047705776863952609 |
| 17 | 9 | 1.5933956191964316 | -0.077009560044453806 |
| 33 | 9 | 1.6062513990021401 | -0.089865339850162318 |
| 65 | 9 | 1.6127738812210539 | -0.096387822069076146 |

The refined upper bound remains super-threshold on every registered quotient.

## 4. Consequence

FTD-0594 proves the exact shared-eigenvalue refinement and proves that it does
not decide `N=10`. It supplies no source history and no genesis event. The
first-event closure remains `N<=9`.

Any stronger uniform norm argument must retain constraints other than equality
of `M`, such as the simultaneous distance spectrum of distinct sources or the
joint integer-time feasible set of the pulse factors. Those are new theorem
branches, not adjustments to FTD-0594.

## 5. Verification

- preregistration SHA-256:
  `F7E04AA0E1B417CC856C58C2B60A4AEABF8D81CA0B766DF5756AC4CEF8A83E25`;
- exact cyclotomic identities and integer shell keys on all volumes;
- every C++ shell key and multiplicity independently reproduced in Python;
- exact mode coverage through 274,624 nonzero modes at `L=65`;
- independent reconstruction: 172/172 PASS;
- no approximate clustering, geometry/history search, or production change.
