# Theorem Boundary — Ten-Source Distance-Distribution LP

**FTD ID:** FTD-0596  
**Status:** `[THEOREM — FOURIER-POSITIVE DISTANCE-DISTRIBUTION GRAM BOUND]` +
`[NUMERICALLY CERTIFIED FACT — 32 PADDED DUAL CERTIFICATES]` +
`[INCONCLUSIVE — N=10]` +
`[CLOSED NEGATIVE — REGISTERED DELSARTE-LP DECIDER]`  
**Date:** 2026-07-26  
**Verdict:** `TEN_SOURCE_DISTANCE_DISTRIBUTION_LP_INCONCLUSIVE`

## 1. Complete distance distribution

Let `D_0={0},D_1,...,D_m` be the displacement orbits of
`Z_L^3` under signed coordinate permutations, with `v_j=|D_j|`. For an
`r`-site set `X`, define

\[
 a_j(X)=\frac1r\left|\{(x,y)\in X^2:y-x\in D_j\}\right|.
\]

Every realizable set satisfies

\[
 a_0=1,\qquad a_j\ge0,\qquad
 \sum_{j=1}^m a_j=r-1,\qquad
 a_j\le\min(v_j,r-1).
\]

The exact FTD-0595 axial capacity supplies the additional inequality

\[
 a_{\rm axial}\le\frac{2e_L(r)}r.
\]

This representation retains every cubic distance class. It is still a
relaxation: not every real vector satisfying the inequalities below must be
the distance distribution of an actual finite site set.

## 2. Fourier-positivity theorem

For a nonzero momentum-orbit representative `k_l`, define

\[
 P_{\ell j}=\frac1{v_j}\sum_{d\in D_j}
 \cos\!\left(\frac{2\pi}{L}k_\ell\!\cdot d\right).
\]

The autocorrelation of the indicator of `X` has nonnegative Fourier
transform. Averaging under the cubic group preserves both its distance
distribution and positivity. Therefore every realizable `X` obeys

\[
 \boxed{1+\sum_{j=1}^mP_{\ell j}a_j\ge0}
 \quad\hbox{for every momentum orbit }\ell.
\]

Consequently, if `kappa_j` is the exact FTD-0595 shared-`M` kernel on `D_j`,
then the linear program

\[
 q_L(r)=\max_a\sum_{j=1}^m\kappa_j a_j
\]

over normalization, nonnegativity, orbit capacities, the axial cap, and all
Fourier inequalities is an upper bound for every realizable `r`-site set.
The removed-source Gram factor and ten-source partition bound are therefore

\[
 G_L^{\rm DD}(r)=r[1+q_L(r)],\qquad
 H_L^{\rm DD}(10,r)=C_L\sqrt{10-r}+Q_L\sqrt{G_L^{\rm DD}(r)}.
\]

The theorem is the upper-bound implication. It does not assert that the LP
optimum is realizable or attained by a source history.

## 3. Certified optimization

For every `L in {9,17,33,65}` and `r=2,...,9`, a sparse dual certificate
`(lambda,y,z)` was recorded. With

\[
 \epsilon=\max_j\left[\kappa_j-\lambda-z_j
 +\sum_\ell y_\ell P_{\ell j}\right]_+
\]

and registered coefficient padding

\[
 \delta=5\times10^{-12}\left(1+\sum_\ell y_\ell\right)+10^{-12},
\]

the authoritative objective is

\[
 U_L(r)=(r-1)(\lambda+\epsilon+\delta)
 +\sum_\ell y_\ell+\sum_j u_jz_j.
\]

Independent 90-decimal-digit reconstruction verifies all primal Fourier
constraints and all padded dual inequalities. The largest primal/dual gap is
below `5.5e-11`, and the most negative reconstructed primal Fourier value is
`-1.9511e-11`, within the locked `-1e-10` feasibility gate.

## 4. Evaluation and boundary

| `L` | orbit classes | maximizing `r` | certified bound | `K_GENESIS-bound` |
|---:|---:|---:|---:|---:|
| 9 | 34 | 8 | 1.5218539833164362 | -0.005467924164458182 |
| 17 | 164 | 8 | 1.5741191331652207 | -0.057733074013242680 |
| 33 | 968 | 8 | 1.5852789946030676 | -0.068892935451089650 |
| 65 | 6,544 | 8 | 1.5932999259156457 | -0.076913866763667740 |

The LP improves every FTD-0595 bound and moves the maximizing partition from
`r=9` to `r=8`. Nevertheless every registered maximum remains above
`K_GENESIS=1.5163860591519780`. FTD-0596 therefore does not close `N=10` and
does not construct a ten-source genesis witness.

## 5. Exact scope

FTD-0596 proves a stronger uniform Gram upper bound for the frozen first-event
sector. At this stage it left the certified closure at `N<=9`. It does not prove LP
realizability, threshold crossing, descendant creation, persistence,
reciprocity, a particle, or infrared physics. The registered Delsarte-LP
decider is closed negative. FTD-0597 subsequently resolves the needed
same-observation-time pair-product projection and closes the frozen boundary
through `N=10`; that later theorem does not change the FTD-0596 verdict.

## 6. Verification

- preregistration SHA-256:
  `D69E9AFE8FCB2ECA487D285AC0B4A85D57FF1182B68FE613E32B0CADE7D3F2FA`;
- 32 sparse primal/dual certificates, covering four volumes and eight
  nontrivial removal partitions;
- 396/396 independent high-precision and cross-language checks;
- exact orbit coverage through all 6,544 nonzero cubic displacement classes
  at `L=65`;
- no configuration, polarity, schedule, history, extra-cut, scenario, toggle,
  or production search/change.
