# V3 uniform field-bank EM/Born measure seam v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT BARE FIELD-BANK COUNTING METRIC]** +
**[THEOREM — PREPARED BRIGHT-PAIR PUSHFORWARD]** +
**[CONDITIONAL — COMMON EM/BORN MEASURE SEAM]** +
**[OPEN — DYNAMICAL MEASURE, ACTION UNIT, STATIC POLE, AND PHYSICAL TRIALS]**  
**Scope:** the selected finite v3 field-channel bank under uniform product
counting  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Candidate law:**
[`SPEC_V3_CHARGED_COMMON_ACTION_PHI_v3_CANDIDATE.md`](../../../01_reference/SPEC_V3_CHARGED_COMMON_ACTION_PHI_v3_CANDIDATE.md)  
**Exact certificate:**
[`proof_v3_uniform_field_bank_em_born_measure_seam.py`](../../../../../scripts/proofs/proof_v3_uniform_field_bank_em_born_measure_seam.py)

---

## 1. One finite bank, not two probability assumptions

For one fixed polarity the v3 carrier has 192 independent binary field
channels. Let $v_a\in\mathbb Z^6$ be the additive electric-magnetic readout
of channel $a$. Exact enumeration gives

\[
 \sum_{a=1}^{192}v_a=0,
 \qquad
 \sum_{a=1}^{192}v_av_a^{\mathsf T}=64I_6.              \tag{1}
\]

Under the uniform binary product measure, every channel occupation has
variance $1/4$. Therefore

\[
 \boxed{\operatorname{Cov}(E,B)=16I_6.}                 \tag{2}
\]

The carrier-number variance is $48$, its covariance with all six field
coordinates vanishes, and the complete number-plus-field covariance has rank
seven. The bare field metric is thus nondegenerate and exactly isotropic in
the selected six-dimensional readout.

Equation (2) is a finite counting-measure theorem. It is not yet the Hessian
of the physical time-evolution action.

---

## 2. Canonical packet coordinates

A complete native field packet has additive magnitude eight. In packet
coordinates

\[
 f={1\over8}(E,B),                                      \tag{3}
\]

the exact covariance and inverse covariance are

\[
 \boxed{\Sigma_f={1\over4}I_6,\qquad
        \Sigma_f^{-1}=4I_6.}                            \tag{4}
\]

Every positive or negative unit electric packet consequently has the same
bare Mahalanobis insertion cost

\[
 {1\over2}\delta f^{\mathsf T}\Sigma_f^{-1}\delta f=2. \tag{5}
\]

For a block of $B$ independent sites, a coherent unit shift costs $2B$,
whereas inserting one packet into the block-average coordinate costs $2/B$.
Thus even after the exact local metric is fixed, a physical blocking and
action-unit convention remains part of the required dynamics.

---

## 3. The same bank supplies the prepared Born seam

One tangent/polarity outcome port has eight binary channels in each of four
C4 phase classes. Under the same uniform product measure each phase count is

\[
 n_k\sim\operatorname{Binomial}(8,1/2),
 \qquad
 \mathbb E n_k=4,
 \qquad
 \operatorname{Var}(n_k)=2.                            \tag{6}
\]

Define the Gaussian-integer readout

\[
 Z=(n_0-n_2)+i(n_1-n_3).                               \tag{7}
\]

Then

\[
 \operatorname{Cov}(\Re Z,\Im Z)=4I_2,
 \qquad
 \mathbb E|Z|^2=8                                     \tag{8}
\]

per site. Independently, exhaustive enumeration of all $5^4=625$ bounded
phase-count vectors verifies that the existing compatible ordered-pair event
count is exactly

\[
 \boxed{M=|Z|^2.}                                      \tag{9}
\]

Equations (2) and (9) therefore arise from one finite bank and one declared
uniform counting reference. The EM metric and prepared Born map are not being
assigned unrelated probability tables.

---

## 4. Conditional coupling statement

If a future dynamical theorem selects this uniform measure, identifies its
blocked large-deviation Hessian with the Maxwell--Gauss action, and fixes one
common positive action multiplier

\[
 \lambda_{\rm common}>0,
\]

then the packet-coordinate result would give

\[
 \chi_{\rm EM}=4\lambda_{\rm common}.                  \tag{10}
\]

Using the already registered conditional static/radiative measurement
protocol would then imply

\[
 \alpha_{\rm native}
 ={3\chi_{\rm EM}\over2\pi}
 ={6\lambda_{\rm common}\over\pi}.                    \tag{11}
\]

This is deliberately not a numerical prediction. The deterministic state
transition is unchanged for every positive value of
$\lambda_{\rm common}$. Neither the bank combinatorics nor the charged
candidate presently fixes that multiplier.

---

## 5. Exact boundary

Established:

1. one-polarity field covariance $16I_6$;
2. number-field orthogonality and rank-seven joint covariance;
3. canonical packet covariance $I_6/4$ and Hessian $4I_6$;
4. equal bare cost two for every unit electric packet;
5. exact extensive and block-average scaling;
6. exact isotropic Gaussian-integer second moments; and
7. the prepared compatible-pair pushforward $M=|Z|^2$.

Not established:

1. selection or attraction of the uniform product measure by Phi-v3;
2. equality of the counting Hessian and the interacting dynamical Hessian;
3. a common physical action unit or the value of
   $\lambda_{\rm common}$;
4. a charged static $1/\Lambda$ pole;
5. native preparation of the phase bank, address orbit, or detector;
6. independent physical trials or normalized Born frequencies;
7. stable localized matter; or
8. the scalar/vector/tensor gravity response of the same action.

The result is therefore a genuine common-measure seam, not the completion of
electromagnetism, the Born rule, or fine-structure normalization.
