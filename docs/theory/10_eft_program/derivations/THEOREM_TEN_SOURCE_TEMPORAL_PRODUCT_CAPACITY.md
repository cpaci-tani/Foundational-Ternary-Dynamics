# Theorem — Ten-Source Temporal Product Capacity

**FTD ID:** FTD-0597  
**Status:** `[THEOREM — EXACT TEMPORAL PAIR-PRODUCT BOUND]` +
`[THEOREM — TEN-SOURCE FIRST-EVENT COROLLARY]` +
`[NUMERICALLY CERTIFIED FACT — 32 PADDED DUAL CERTIFICATES]` +
`[CLOSED NEGATIVE — ENDOGENOUS AUTOCATALYSIS FOR N<=10 IN FROZEN SECTOR]`  
**Date:** 2026-07-26  
**Verdict:**
`ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_TEMPORAL_PRODUCT_CAPACITY`

## 1. Scope

Retain the frozen FTD-0596 first-event sector: the production 18-point wave
operator, native state-gradient source, zero initial field/velocity, periodic
odd `L in {9,17,33,65}`, and at most ten distinct stationary ternary sources
of arbitrary position and sign. Every original source begins present and may
be removed once at an arbitrary integer tick.

Gauss projection, damping, forces, movement, reactions, collisions, clocks,
baths, toggles, scenarios, and production modifications are absent. The proof
ends immediately before a hypothetical first descendant genesis event.

## 2. Exact same-time pulse interval

For one exact stencil-eigenvalue shell, FTD-0589 gives the normalized finite
removal pulse

\[
 u_{n,T}(\theta)=
 \sin\frac{T\theta}{2}
 \sin\left(n-\frac{T-1}{2}\right)\theta.
\]

The product-to-sum identity gives

\[
 u_{n,T}(\theta)=\frac12\left[
 \cos\left(n-T+\frac12\right)\theta
 -\cos\left(n+\frac12\right)\theta\right].
\]

Put `b=(n+1/2)theta` and `t=T theta`. At fixed observation time `n`, every
source-removal factor on this shell has the form

\[
 u_j=\frac12[\cos(b-t_j)-\cos b]
\]

and therefore lies in

\[
 I_b=\left[-\frac{1+\cos b}{2},
             \frac{1-\cos b}{2}\right].
\]

The interval has width exactly one. Hence `|u_i-u_j|<=1`. If `u_i,u_j` have
opposite signs, write them as `a,-c` with `a,c>=0`; then `a+c<=1`, so
`ac<=1/4`. Same-sign products are at most one. Thus

\[
 \boxed{-\frac14\le u_i(M)u_j(M)\le1.}
\]

Both endpoints are attained in the continuous phase projection. The theorem
uses no density, irrationality, or rational-independence assumption.

## 3. Signed exact-shell kernel

For each exact cyclotomic `M` shell `S`, retain the FTD-0594 character

\[
 K_S(d)=\sum_{O\subset S}w_O\chi_O(d),
 \qquad W_L=\sum_{k\ne0}w(k).
\]

Define its positive and negative shell masses

\[
 P_L(d)=\sum_S[K_S(d)]_+,
 \qquad N_L(d)=\sum_S[-K_S(d)]_+.
\]

The temporal product interval implies, for either fixed polarity product,

\[
 \left|\sum_SK_S(d)u_i(S)u_j(S)\right|
 \le\max\left(P_L(d)+\frac14N_L(d),
              N_L(d)+\frac14P_L(d)\right).
\]

Therefore the exact registered pair kernel is

\[
 \boxed{\tau_L(d)=\frac1{W_L}
 \max\left(P_L(d)+\frac14N_L(d),
           N_L(d)+\frac14P_L(d)\right).}
\]

Equivalently,

\[
 \tau_L(d)=\frac58\kappa_L(d)
 +\frac{3}{8W_L}\left|\sum_SK_S(d)\right|
 \le\kappa_L(d),
\]

where `kappa_L` is the FTD-0596 absolute-shell kernel. The largest temporal
kernel on every registered quotient is axial:

| `L` | exact `M` shells | `max tau_L` | FTD-0596 `max kappa_L` |
|---:|---:|---:|---:|
| 9 | 29 | 0.27004525062779755 | 0.35925757426614319 |
| 17 | 164 | 0.27307963562353821 | 0.36250597734262191 |
| 33 | 947 | 0.27332801507249327 | 0.36267617904631827 |
| 65 | 6,544 | 0.27338500735364296 | 0.36273662797281120 |

## 4. Distance-distribution Gram theorem

Use the complete FTD-0596 distance distribution `a_j`, including all orbit
capacities, the exact axial cap, and every autocorrelation Fourier-positivity
constraint. The certified linear program is

\[
 q_L^{\rm time}(r)=\max_a\sum_j\tau_j a_j.
\]

For every realizable removed-source set,

\[
 G_L^{\rm time}(r)=r[1+q_L^{\rm time}(r)]
\]

is a Gram upper bound. Combining it with the unchanged present-source term
gives

\[
 \boxed{H_L^{\rm time}(10,r)=C_L\sqrt{10-r}
 +Q_L\sqrt{G_L^{\rm time}(r)}.}
\]

The `r=0,1,10` partitions retain their registered parent values. For
`r=2,...,9`, 32 padded sparse dual certificates bound the LP objectives
independently of the numerical solver.

## 5. Certified evaluation

| `L` | maximizing `r` | FTD-0597 maximum | FTD-0596 maximum | margin to `K_GENESIS` |
|---:|---:|---:|---:|---:|
| 9 | 8 | 1.4041876553015755 | 1.5218539833164362 | 0.11219840385040247 |
| 17 | 8 | 1.4402836373493018 | 1.5741191331652207 | 0.07610242180267623 |
| 33 | 8 | 1.4510212847705661 | 1.5852789946030676 | 0.06536477438141186 |
| 65 | 8 | 1.4577559407727352 | 1.5932999259156457 | 0.05863011837924281 |

Every registered maximum is strictly below
`K_GENESIS=1.5163860591519780`.

## 6. First-event corollary through ten sources

Assume `N<=10` and suppose a first descendant genesis event exists. Just
before that event, every field source is still one of the original stationary
sites and has either remained present or undergone its single removal. For
the corresponding removed count `r`, Sections 2--5 give

\[
 |J|\le H_L^{\rm time}(N,r)
 \le H_L^{\rm time}(10,r)<K_{\rm GENESIS}.
\]

The genesis predicate is false, contradicting the assumed first event. Hence
no first descendant can occur from at most ten original sources in the frozen
sector.

This is a first-event impossibility theorem, not a statement about the full
production engine. It supplies no mobile identity, reciprocal action,
particle pole, scenario, or infrared recovery.

## 7. Verification

- pre-evaluation protocol SHA-256:
  `7FF1D85959CE80932C3F60FBC0E39BEBC09E7567EF39724B166879F41843801D`;
- exact rational temporal interval/product proof;
- complete signed-shell reconstruction through all 6,544 `L=65` classes;
- 32 padded sparse dual certificates;
- 413/413 independent 90-decimal-digit, hash, and cross-language checks;
- no configuration, polarity, history, or observation-time search;
- no extra cut, tolerance change, toggle, scenario, or production change.
