# $O_h$ Moore-local dual-capacity mixing and isotropic-factor pass v1

**Date:** 2026-08-24

**Status:** **[THEOREM — EXACT $\mathbb Z_L^3$ RELATIVE-TRANSLATION
CENSUS]** + **[THEOREM — EVERY UPDATE IS ONE 26-MOORE-NEIGHBOR
PERMUTATION]** + **[THEOREM — REVERSED SIGNED-CUBIC SCHEDULE HAS ZERO DRIFT
AND ISOTROPIC SECOND MOMENT]** + **[THEOREM — ARBITRARY 3D BINARY CAPACITY
PATTERNS FACTORIZE EXACTLY]** + **[REFERENCE CONSTRUCTION — GLOBAL FRAME
SCHEDULE]** + **[OPEN — NATIVE FRAME SELECTION, C18-ONLY LIFT IF REQUIRED,
SOURCE LEDGER, COTANGENT/C3 COMPOSITION, TT LIFT, STATIC POLE, LENSING]**

**Production status:** unchanged

**Ledger status:** no row minted

**Exact certificate:**
[proof_oh_moore_dual_capacity_mixing_schedule.py](../../../../../scripts/proofs/proof_oh_moore_dual_capacity_mixing_schedule.py)
performs 505,657 exact checks. It verifies all 48 signed-cubic frames in both
directions through $L=8$, exact first/second step moments through $L=12$,
every pair of $2\times2\times2$ binary capacity patterns, structured larger
3D controls, and the complete displacement multiplicity.

---

## 1. Question inherited from cyclic mixing

The
[dual-capacity cyclic-mixing theorem](THEOREM_DUAL_CAPACITY_CORRELATION_OBSTRUCTION_AND_CYCLIC_MIXING_RESPONSE_v1.md)
proved exact factorization by translating a dual capacity layer around one
periodic axial ring. That witness was local and reversible but selected one
spatial direction.

The present theorem asks whether the same decorrelation can be realized in
three dimensions with:

1. one Moore-local hop per global tick;
2. complete relative-translation coverage;
3. zero blocked drift;
4. exact signed-cubic isotropy; and
5. unchanged arbitrary-pattern factorization.

All five pass at the finite reference-schedule level.

---

## 2. Mixed-radix translation walk

Work on the periodic translation group

\[
 \Lambda_L=\mathbb Z_L^3.                          \tag{1}
\]

For pointer

\[
 m\in\mathbb Z_{L^3},                              \tag{2}
\]

define

\[
 r(m)=left(
 m\bmod L,
 \left\lfloor{m\over L}\right\rfloor\bmod L,
 \left\lfloor{m\over L^2}\right\rfloor\bmod L
 \right).                                         \tag{3}
\]

Equation (3) is a bijection from the pointer cycle to every relative
translation in $\Lambda_L$ exactly once.

The forward step from pointer $m$ is

\[
 \delta(m)=left(
 1,
 \mathbf1_{x=L-1},
 \mathbf1_{x=L-1}\mathbf1_{y=L-1}
 \right),                                         \tag{4}
\]

where $(x,y,z)=r(m)$. Modulo $L$,

\[
 r(m+1)=r(m)+\delta(m).                            \tag{5}
\]

Every step in equation (4) is one of:

\[
 (1,0,0),\qquad(1,1,0),\qquad(1,1,1),             \tag{6}
\]

namely an SC, FCC, or BCC member of the 26-connected Moore neighborhood.
Thus carry propagation in the mixed-radix pointer does not create a nonlocal
spatial jump.

After $L^3$ steps the pointer, translation, and dual layer return exactly.

---

## 3. Signed-cubic frame family

Let

\[
 R\in O_h                                             \tag{7}
\]

be any of the 48 signed permutation matrices. Transform both the positions
and steps:

\[
 r_R(m)=Rr(m)\pmod L,
 \qquad
 \delta_R(m)=R\delta(m).                           \tag{8}
\]

Because $R$ is a signed coordinate permutation, every component of
$\delta_R$ remains in $\{-1,0,1\}$ and every step remains Moore-local.
Equation (8) still visits all $L^3$ translations once.

For every frame, also include the reversed block

\[
 r_R(-m),qquad -R\delta(-m-1).                    \tag{9}
\]

The reversed block is the exact spatial inverse of the forward walk.

The complete reference supercycle contains

\[
 48\times2\times L^3=96L^3                         \tag{10}
\]

global ticks.

---

## 4. Exact zero drift and isotropic second moment

Forward/reverse pairing gives

\[
 \boxed{\sum_{\rm schedule}\delta=0.}             \tag{11}
\]

so the complete mixing cycle has no net preferred drift.

In one forward base block, the exact squared-step sum is

\[
 \sum_m\|\delta(m)\|^2
 =L^3+L^2+L.                                       \tag{12}
\]

The signed-cubic group average of any rank-two tensor is proportional to the
identity. Direct finite summation gives

\[
 \boxed{
 \sum_{\rm schedule}\delta\delta^T
 ={96(L^3+L^2+L)\over3}I_3.}                      \tag{13}
\]

Dividing by the step count,

\[
 \left\langle\delta_i\delta_j\right\rangle
 ={1\over3}left(1+{1\over L}+{1\over L^2}\right)
 \delta_{ij}.                                     \tag{14}
\]

Thus the complete finite step census is exactly isotropic at second order,
not merely asymptotically cubic.

Equation (14) describes the auxiliary capacity-mixing walk. It is not the
physical Maxwell or tensor propagation speed.

---

## 5. Exact 3D factorization

Let

\[
 p_x,d_x\in\{0,1\},\qquad x\in\Lambda_L           \tag{15}
\]

be arbitrary primal and dual open-capacity patterns, with counts

\[
 P=\sum_xp_x,qquad D=\sum_xd_x.                  \tag{16}
\]

At relative translation $r$, the joint open count is

\[
 J(r)=\sum_xp_xd_{x-r}.                            \tag{17}
\]

Because equation (3) visits every $r\in\Lambda_L$ once,

\[
 \begin{aligned}
 \sum_{r\in\Lambda_L}J(r)
 &=\sum_{r,x}p_xd_{x-r}\\
 &=\left(\sum_xp_x\right)
   \left(\sum_yd_y\right)\\
 &=PD.
 \end{aligned}                                     \tag{18}
\]

Therefore one frame block already gives

\[
 \boxed{
 \bar j={PD\over L^6}
 ={P\over L^3}{D\over L^3}
 =\nu_t\nu_s.}                                    \tag{19}
\]

Each of the 96 forward/reverse signed-cubic blocks contains the same complete
translation set, so every relative displacement occurs exactly 96 times and
equation (19) remains unchanged over the isotropic supercycle.

The certificate exhausts all 65,536 ordered pairs of binary patterns on the
$2^3$ cell and verifies structured controls through $L=8$.

---

## 6. Local reversibility

At each tick the entire dual capacity layer is translated by one Moore vector
$\delta$. This is a site permutation:

\[
 d'_x=d_{x-\delta}.                                \tag{20}
\]

Its inverse is

\[
 d_x=d'_{x+\delta}.                                \tag{21}
\]

All binary counts and any payload transported with each dual record are
preserved. The global schedule phase identifies which inverse step applies;
no local capacity bit is erased.

This is a synchronous local permutation of a complete layer. The theorem does
not derive its scheduling from collisions among individual records.

---

## 7. Relation to the global FTD clock

The schedule can be indexed entirely by the global substrate tick:

\[
 (R,\text{direction},m)
 \in O_h\times\{+,-\}\times\mathbb Z_{L^3}.       \tag{22}
\]

No material clock must carry this pointer. In that sense the construction is
compatible with one global ordering and locally dilated material recurrences.

However, equation (22) is presently an imposed periodic frame program. The
native action has not generated:

1. the block size $L$;
2. the frame ordering;
3. the synchronization with the global-C3 cotangent layers; or
4. the ownership boundary at which one mixing epoch begins and ends.

These are physical selection debts, not algebraic failures of the mixing
identity.

---

## 8. C18 versus full-Moore price

The BCC carry step $(1,1,1)$ appears $L$ times per forward block. It is legal
under the foundational 26-connected Moore neighborhood. The current
phase-complete action candidate uses only the C18 SC+FCC bond set for its field
and tensor kinematics.

Therefore one of two choices must be made explicitly:

1. allow the auxiliary dual-capacity controller to use the full Moore
   neighborhood while the physical field carrier remains C18; or
2. construct a longer C18-only schedule that resolves each BCC carry into
   separately retained local steps while preserving uniform displacement
   multiplicity.

The present theorem proves the full-Moore route. It does not silently license
BCC field propagation or add BCC records to the cotangent Maxwell carrier.

---

## 9. Exact epistemic boundary

### Proved

1. Equation (3) visits every 3D relative translation exactly once.
2. Every update is a single 26-Moore-neighbor permutation with exact inverse.
3. The reversed 48-frame schedule has zero drift and isotropic second moment.
4. Every relative displacement occurs exactly 96 times.
5. Arbitrary finite 3D primal/dual binary patterns factorize exactly under the
   schedule.
6. The schedule preserves all transported dual counts and payloads.

### Not proved

1. native selection of the global frame schedule or block size;
2. a C18-only alternative if auxiliary BCC transport is disallowed;
3. the two-token/time-sharing source ledger;
4. composition with the global-C3 cotangent collision and Gauss source;
5. composition with the parity-staggered STF tensor lift;
6. a source law $M(U)$ or slow-body response;
7. a static inverse-distance capacity profile; or
8. lensing.

Production remains unchanged and class 0.

---

## 10. Next locked gate

Compose one complete finite macro:

1. actualization writes the paid primal/dual deficit records;
2. the dual-A9 skew generator supplies local clock admission;
3. the $O_h$ Moore schedule mixes the dual records;
4. the global-C3 cotangent map reads the joint capacity without losing its
   seven-dimensional vacuum/Gauss slow space;
5. a genuine multi-record odd/even STF collision reads the same dual
   placement and preserves TT, because diagonal single-record C18 streaming
   is excluded by its exact successor obstruction; and
6. every substep has one retained inverse and one common token/work ledger.

Only after this composition passes should the action derive $M(U)$ and a
static source response. The 3D decorrelation and isotropy problem is now an
exact reference construction rather than an unspecified emergence claim.
