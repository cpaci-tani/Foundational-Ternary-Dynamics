# Dual-capacity correlation obstruction and cyclic-mixing response v1

**Date:** 2026-08-24

**Status:** **[THEOREM — EXACT COLLOCATED PRIMAL/DUAL CAPACITY-CORRELATION
OBSTRUCTION]** + **[THEOREM — EXACT REVERSIBLE CYCLIC-MIXING
FACTORIZATION]** + **[THEOREM, CONDITIONAL — ARBITRARY EQUAL FINITE DEFICIT
COUNTS GIVE THE SQUARED WEAK OPTICAL RESPONSE]** + **[THEOREM — TWO-TOKEN
PRICE FOR SIMULTANEOUS INDEPENDENT PRIMAL/DUAL OCCUPANCY]** + **[OPEN — NATIVE
PAIRED SOURCE LEDGER, 3D ISOTROPIC ROUTING, $M(U)$, SLOW-BODY RESPONSE,
COTANGENT/TT LIFT, STATIC POLE, LENSING]**

**Production status:** unchanged

**Ledger status:** no row minted

**Exact certificate:**
[proof_dual_capacity_cyclic_mixing_source_response.py](../../../../../scripts/proofs/proof_dual_capacity_cyclic_mixing_source_response.py)
performs 1,146,900 exact checks. It exhausts collocated binary patterns through
length twelve, every pair of binary primal/dual patterns through length seven,
all finite deficit counts through block size 128, the complete reversible
shift cycle, and the source-token occupancy price.

---

## 1. Question inherited from the homogeneous skew generator

The
[dual-A9 skew theorem](THEOREM_DUAL_A9_SKEW_CAPACITY_CLOCK_GENERATOR_AND_HOMOGENEOUS_FACTOR_PASS_v1.md)
constructed an endogenous finite generator whose every occupied physical cell
has

\[
 (\nu_t,\nu_s,j)=\left({1\over2},{1\over2},{1\over4}\right), \tag{1}
\]

where $j$ is the joint permission fraction.

Gravity requires a **variable** response. A first attempt is to mix open
vacuum cells with source-occupied skew cells. This theorem asks whether that
mixture preserves

\[
 j=\nu_t\nu_s.                                     \tag{2}
\]

It does not if the source deficits remain collocated.

---

## 2. General collocation obstruction

Let

\[
 p_x,d_x\in\{0,1\},\qquad x\in\mathbb Z_L         \tag{3}
\]

be primal and dual open-capacity bits. If the two patterns are identical and
collocated,

\[
 d_x=p_x,                                          \tag{4}
\]

then binary idempotence gives

\[
 j_{\rm collocated}
 ={1\over L}\sum_xp_xd_x
 ={1\over L}\sum_xp_x
 =\nu.                                             \tag{5}
\]

But factorization requires $j=\nu^2$. For every nontrivial capacity fraction
$0<\nu<1$,

\[
 \boxed{
 j_{\rm collocated}-\nu^2
 =\nu(1-\nu)>0.}                                  \tag{6}
\]

Thus a source that marks the same primal and dual locations leaves a positive
common-cause covariance. Duplicating the geometric layer is insufficient;
the histories must also mix or otherwise decorrelate at the blocking scale.

---

## 3. Specific obstruction for vacuum plus skew cells

Let a block fraction $\rho$ contain occupied dual-A9 skew cells with equation
(1), while fraction $1-\rho$ is always-open vacuum. The block marginals are

\[
 \nu_t=\nu_s=1-{\rho\over2},                       \tag{7}
\]

but the joint fraction is

\[
 j=1-{3\rho\over4}.                                \tag{8}
\]

Their exact covariance is

\[
 \boxed{
 j-\nu_t\nu_s={\rho(1-\rho)\over4}.}              \tag{9}
\]

Define the marginal admission depth

\[
 U_c={\rho\over2},
 \qquad
 \nu_t=\nu_s=1-U_c.                                \tag{10}
\]

Equation (8) becomes

\[
 j=1-{3\over2}U_c,                                 \tag{11}
\]

so its weak optical index is

\[
 {1\over j}=1+{3\over2}U_c+O(U_c^2).              \tag{12}
\]

> The naive collocated mixture gives coefficient $3/2$, not the factorized
> temporal-plus-spatial coefficient $2$.

This is an exact rejection of a tempting shortcut, not a comparison with an
experimental target.

---

## 4. Reversible local cyclic mixing

Hold the primal pattern fixed during one mixing epoch and translate the dual
pattern by one site per tick:

\[
 (Sd)_x=d_{x-1}.                                    \tag{13}
\]

On a periodic ring, $S$ is a one-hop local permutation with inverse

\[
 (S^{-1}d)_x=d_{x+1}.                              \tag{14}
\]

It preserves the exact dual token/open count. After $L$ ticks, $S^Ld=d$.

Let

\[
 P=\sum_xp_x,
 \qquad
 D=\sum_xd_x.                                      \tag{15}
\]

The complete space--time joint count is

\[
 \begin{aligned}
 \sum_{t=0}^{L-1}\sum_{x=0}^{L-1}p_x(S^td)_x
 &=\sum_{x,t}p_xd_{x-t}\\
 &=\left(\sum_xp_x\right)
   \left(\sum_yd_y\right)\\
 &=PD.
 \end{aligned}                                     \tag{16}
\]

Every primal slot meets every dual slot exactly once. Therefore

\[
 \boxed{
 \bar j={PD\over L^2}
 ={P\over L}{D\over L}
 =\nu_t\nu_s.}                                    \tag{17}
\]

Equation (17) holds for every finite binary pattern, including initially
collocated patterns. It uses deterministic local streaming, not stochastic
independence or a target probability table.

---

## 5. Arbitrary finite source-count response

Let $M_P$ and $M_D$ be the numbers of blocked primal and dual slots in a block
of size $L$. Then

\[
 \nu_t=1-{M_P\over L},
 \qquad
 \nu_s=1-{M_D\over L}.                             \tag{18}
\]

After one complete mixing orbit,

\[
 \boxed{
 \bar j=left(1-{M_P\over L}\right)
             \left(1-{M_D\over L}\right).}        \tag{19}
\]

If an exchange-symmetric source ledger gives

\[
 M_P=M_D=M,                                        \tag{20}
\]

then

\[
 \boxed{
 \nu_t=\nu_s=1-{M\over L},
 \qquad
 \bar j=\left(1-{M\over L}\right)^2.}            \tag{21}
\]

Thus arbitrary rational capacity depths $M/L$ have an exact deterministic
squared response at finite block size. The allowed rational family becomes
dense as the block size grows, but equation (21) itself is exact for every
finite $L$ and $M$.

The theorem does not derive $M$ from a material source or identify $M/L$ with
the measured gravitational potential. It derives the response *conditional
on the blocked source counts*.

---

## 6. Conditional weak optical coefficient

Suppose the native source dynamics later derives

\[
 \nu_t(U)=1-a_tU+O(U^2),
 \qquad
 \nu_s(U)=1-a_sU+O(U^2).                           \tag{22}
\]

Equation (17) gives

\[
 c_{\rm ray}(U)={1\over6}\nu_t(U)\nu_s(U),         \tag{23}
\]

and hence

\[
 {c_*/c_{\rm ray}(U)}
 =1+(a_t+a_s)U+O(U^2).                             \tag{24}
\]

If source exchange proves $a_s=a_t$ and slow-body response independently
gives $a_t=a_m$, then

\[
 \mathscr D=\mathscr S={a_t+a_s\over a_m}=2.      \tag{25}
\]

Equation (25) remains conditional because neither $M(U)$ nor $a_m$ has been
derived.

---

## 7. Exact source-token price

Two simultaneous independently occupied primal/dual slots have binary
occupancies

\[
 (n_P,n_D)=(1,1).                                  \tag{26}
\]

A ledger containing only one token obeys

\[
 n_P+n_D=1                                         \tag{27}
\]

and permits only $(1,0)$ or $(0,1)$, for which $n_Pn_D=0$. Therefore one token
cannot occupy both independent placements.

\[
 \boxed{
 \text{simultaneous independent primal/dual occupancy costs at least two
 retained tokens, or an explicitly time-shared equivalent ledger}.}       \tag{28}
\]

The existing one-token actualization vertex cannot silently populate both A9
layers. A unified source macro must provide a second owned token, derive a
time-sharing construction with the same blocked counts, or close the dual-A9
route negative.

This is a token-count statement, not a physical energy or coupling
measurement.

---

## 8. Three-dimensional boundary

Equation (13) is Moore-local on a periodic axial ring, but one fixed shift
direction is anisotropic. A physical three-dimensional lift must:

1. route dual records through an $O_h$-covariant sequence of Moore directions;
2. retain an inverse at every step;
3. preserve the dual count and A9 phase/polarity payload;
4. mix the relevant local block without introducing superluminal transport;
5. avoid a net preferred drift in the blocked rest frame; and
6. commute appropriately with the global-C3 cotangent layers.

The successor
[3D $O_h$ Moore-local mixing theorem](THEOREM_OH_MOORE_LOCAL_DUAL_CAPACITY_MIXING_AND_ISOTROPIC_FACTOR_PASS_v1.md)
closes this reference-construction question. Its mixed-radix walk visits every
relative translation, every substep is one SC/FCC/BCC Moore hop, and the
forward/reverse 48-frame completion has zero drift and isotropic second
moment. It still does not derive the frame schedule from the local action or
show that the full-Moore auxiliary controller can be composed with the
current C18 field carrier without changing its cone.

---

## 9. Exact epistemic boundary

### Proved

1. Collocated identical primal/dual capacity patterns have $j=\nu$, not
   $\nu^2$.
2. Mixing vacuum with homogeneous half-admission skew cells gives exact
   covariance $\rho(1-\rho)/4$ and weak coefficient $3/2$.
3. One-hop cyclic translation is a reversible count-preserving local
   permutation.
4. Its complete mixing orbit gives exact factorization for arbitrary finite
   primal/dual binary patterns.
5. Equal finite deficit counts give the exact squared family in equation
   (21).
6. Simultaneous independent primal/dual occupancy cannot be paid by one token.

### Not proved

1. a native two-token or equivalent time-shared source macro;
2. native selection of the exact three-dimensional reference schedule, or a
   C18-only replacement if auxiliary BCC transport is disallowed;
3. the source-to-deficit law $M(U)$;
4. slow-body response $a_m$ or clock/fall coherence;
5. composition with the full finite cotangent Maxwell and tensor maps;
6. an inverse-distance static solution;
7. ray deflection, Shapiro delay, or lensing; or
8. any gravitational or electromagnetic coupling normalization.

Production remains unchanged and class 0.

---

## 10. Next locked gate

Construct a single payload-complete local source/mixing macro that:

1. starts from the existing actualization reserve and declares the exact
   second-token or time-sharing resource;
2. writes equal primal/dual deficit counts without copying a token;
3. selects or compiles the proved $O_h$-covariant Moore-local mixing cycle
   from retained local controller state;
4. composes with the dual-A9 skew clock generator and preserves its inverse;
5. derives $M(U)$ and the residence-count response from stored source work;
6. gates the actual cotangent Maxwell and odd-STF first moments; and
7. produces blind $a_t/a_m$, $a_s/a_m$, deflection, and delay observables.

The successor removes three-dimensional decorrelation and isotropic moment
balance as reference-construction gaps. The remaining problem is a concrete
source/resource law, native schedule selection, and composed field operator.
