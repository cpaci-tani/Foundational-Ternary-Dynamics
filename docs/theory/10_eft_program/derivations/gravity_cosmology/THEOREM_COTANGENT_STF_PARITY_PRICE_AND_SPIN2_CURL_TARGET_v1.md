# Cotangent STF parity price and spin-2 curl target v1

**Date:** 2026-08-24

**Status:** **[THEOREM — INVERSION FORBIDS AN EVEN/EVEN FIRST-DERIVATIVE
TENSOR CONE]** + **[THEOREM — EXISTING FLAG SPANS EVEN/ODD STF TYPES AND A
MINIMAL RANK-20 PARITY-COMPLETE C4 QUARTET]** + **[THEOREM — UNIQUE ISOTROPIC
SYMMETRIC-CURL TARGET HAS TWO TT HELICITY-TWO MODES]** + **[THEOREM,
CONDITIONAL — COMMON COTANGENT INCIDENCE GIVES $c_T=c_{\rm EM}=1/6$]** +
**[OPEN — FINITE STAGGERED LIFT, ACTION-DERIVED CONSTRAINTS, STATIC GRAVITY,
UNIVERSAL COUPLING, LENSING]**

**Production status:** unchanged

**Ledger status:** no row minted

**Exact certificate:**
[proof_cotangent_stf_parity_spin2_curl_target.py](../../../../../scripts/proofs/proof_cotangent_stf_parity_spin2_curl_target.py)
performs 59,425 exact symbolic and rational checks. It exhausts all 192
cotangent flag/phase records at all three clock layers, all 48 signed cubic
transformations, the complete rank and C4 ledgers, both packet orientations,
and all 98 primitive nonzero integer wavevectors in $[-2,2]^3$.

No gravitational coefficient, continuum pole, deflection angle, or observed
target enters the certificate.

---

## 1. Why the registered phase tensor could not propagate at first order

For one cotangent flag, the existing normalized FCC dyad is

\[
 D={1\over2}rr^T,
 \qquad r=E+hB,
 \qquad \operatorname{tr}D=1.                     \tag{1}
\]

Its trace-free part is

\[
 S=D-{1\over3}I.                                  \tag{2}
\]

Under every signed cubic transformation $R\in O_h$,

\[
 S\longmapsto RSR^T.                              \tag{3}
\]

In particular, $S$ is inversion even. The previously registered C4 tensor
doublet

\[
 Q=u_pS,\qquad P=v_pS                            \tag{4}
\]

therefore consists of two inversion-even tensors. The C4 quadrature is a time
phase and does not change spatial parity.

Let $L(k)$ be any translation-invariant first-derivative linear map from an
even tensor to another even tensor. Linearity in the wavevector gives

\[
 L(-k)=-L(k).                                      \tag{5}
\]

Inversion equivariance of an even-to-even map instead requires

\[
 L(-k)=L(k).                                       \tag{6}
\]

Therefore

\[
 \boxed{L(k)=0.}                                  \tag{7}
\]

This parity theorem explains the exact vanishing $O(k)$ tensor block in the
[same-site two-record collision](THEOREM_COTANGENT_EM_TENSOR_EQUIVARIANT_COLLISION_AND_SPIN2_BOUNDARY_v1.md).
It is stronger than a statement about that selected collision: every
inversion-preserving even/even first-derivative realization is excluded.

**Successor correction.** An analytic even/even transfer whose zero-mode slow
block is semisimple cannot obtain a linear cone from higher even spatial
derivatives. A non-semisimple Jordan or gauge-degenerate zero mode is outside
that assumption. The
[second-order STF action successor](THEOREM_EVEN_STF_SECOND_ORDER_ACTION_SPIN2_ESCAPE_AND_CONSTRAINT_PRICE_v1.md)
constructs exactly that escape: a nearest-neighbor \(k^2\) action has
eigenphase proportional to \(|k|\) because its zero-mode transfer is a Jordan
block. Thus equation (7) remains the no-go for a direct even/even \(O(k)\)
block; it is not a no-go against every second-order constrained tensor action.

---

## 2. The missing odd tensor is already in the flag alphabet

Flag handedness transforms as

\[
 h\longmapsto(\det R)h.                            \tag{8}
\]

Define

\[
 \boxed{H=hS.}                                    \tag{9}
\]

Then

\[
 H\longmapsto(\det R)RHR^T,                       \tag{10}
\]

so $H$ is an inversion-odd STF pseudotensor. Across the existing 192-state
alphabet at every cotangent layer,

\[
 \operatorname{rank}\{S\}=5,qquad
 \operatorname{rank}\{H\}=5,qquad
 \boxed{\operatorname{rank}\{S,H\}=10.}          \tag{11}
\]

No new record label is required to expose the parity-correct gravitoelectric
and gravitomagnetic tensor types. The required information was present in the
handed flag but absent from equation (4)'s readout.

The eight-record manifestation/Gauss packet oriented along $d$ has

\[
 \boxed{
 \sum_{z\in\mathcal P(d,p)}S(z)
 =2\left(dd^T-{1\over3}I\right),
 \qquad
 \sum_{z\in\mathcal P(d,p)}H(z)=0.}               \tag{12}
\]

Thus the common manifestation vertex sources an even tidal shear while
carrying no net odd tensor source, exactly as a static oriented matter event
should at the kinematic level. This is still not a universal stress-energy
law.

---

## 3. Exact parity/C4 type price

The pair $(S,H)$ has the correct spatial parities but does not by itself carry
the existing phase doublet. Retaining both C4 quadratures in both parity
sectors gives

\[
 Q_e=u_pS,\quad P_e=v_pS,
 \qquad Q_o=u_phS,\quad P_o=v_phS.                 \tag{13}
\]

The exact joint rank is

\[
 \boxed{5+5+5+5=20.}                              \tag{14}
\]

Under one internal C4 tick and the compensating cotangent-layer shift,

\[
 \boxed{
 (Q_e,P_e,Q_o,P_o)
 \longmapsto(-P_e,Q_e,-P_o,Q_o).}                 \tag{15}
\]

Equation (14) is the minimum on-site parity-preserving isotropic C4 price when
the carrier is assembled from complete STF $V_2$ copies. To see this, let
spatial inversion split a real tensor carrier into even and odd
five-dimensional sectors. Any isotropic time-phase complex structure $J$
commuting with inversion restricts separately to those sectors. But a real
$5\times5$ matrix cannot obey $J^2=-I_5$, because

\[
 (\det J)^2=\det(-I_5)=-1.                         \tag{16}
\]

Each complete isotropic parity type therefore requires an even multiplicity.
Two copies of each five-component STF representation cost twenty real block
rows. Adding inequivalent scalar/vector components cannot repair the odd STF
block because isotropy forbids $J$ from mixing inequivalent irreducible types.

A spatially staggered primal/dual realization may distribute this price over
different cells or half-ticks rather than store all twenty rows on site. It
must show that ownership and inversion action explicitly; calling the old
rank-ten even/even phase doublet “spin-2” cannot evade equation (7).

---

## 4. Unique isotropic first-derivative target

Let $K(k)$ be the cross-product matrix,

\[
 K(k)x=k\times x.                                  \tag{17}
\]

For a symmetric tensor $X$, define the symmetric tensor curl symbol

\[
 \boxed{
 \mathcal C_kX={1\over2}\bigl(K(k)X-XK(k)\bigr).} \tag{18}
\]

It is symmetric, trace-free when $X$ is trace-free, and skew-adjoint under the
Frobenius pairing. Its full orthogonal covariance is

\[
 \mathcal C_{Rk}(RXR^T)
 =(\det R)R(\mathcal C_kX)R^T.                    \tag{19}
\]

It therefore maps an even STF tensor to an odd one and an odd STF tensor to an
even one, exactly paying the parity requirement of equation (7).

Up to an overall coefficient, equation (18) is the unique isotropic
first-derivative map $V_2\to V_2$. Representation-theoretically,

\[
 V_1\otimes V_2=V_1\oplus V_2\oplus V_3,          \tag{20}
\]

and the output $V_2$ occurs with multiplicity one. The coefficient must be
derived from the finite incidence rule; uniqueness does not fix it.

For $k$ on a symmetry axis, the exact STF characteristic polynomial is

\[
 \boxed{
 \chi_{\mathcal C}(\lambda)
 =\lambda
 \left(\lambda^2+{|k|^2\over4}\right)
 \left(\lambda^2+|k|^2\right).}                  \tag{21}
\]

These are the helicity-zero, helicity-one, and helicity-two rotation weights.

---

## 5. TT reduction and common-cone target

Let $\mathcal E$ be even STF and $\mathcal B$ odd STF. Impose the two
divergence constraints

\[
 k_j\mathcal E_{ij}=0,
 \qquad k_j\mathcal B_{ij}=0.                     \tag{22}
\]

Together with trace freedom, each field lies in the rank-two TT subspace. The
certificate proves for every nonzero tested wavevector that

\[
 [\mathcal C_k,\mathcal P_{\rm TT}]=0,
 \qquad
 \boxed{\mathcal C_k^2=-|k|^2I_{\rm TT}.}          \tag{23}
\]

The minimum parity-correct first-order target is

\[
 \partial_t\mathcal E
 =c_T\operatorname{curl}_s\mathcal B,
 \qquad
 \partial_t\mathcal B
 =-c_T\operatorname{curl}_s\mathcal E.            \tag{24}
\]

Its TT characteristic polynomial is

\[
 \boxed{
 \chi_{\rm TT}(\lambda,k)
 =\left(\lambda^2+c_T^2|k|^2\right)^2.}           \tag{25}
\]

It has two tensor polarizations, positive conserved quadratic norm

\[
 H_T={1\over2}
 \left(\|\mathcal E\|_F^2+\|\mathcal B\|_F^2\right), \tag{26}
\]

and no scalar or helicity-one mode after equation (22).

`[THEOREM, CONDITIONAL]` If the finite staggered lift uses the same normalized
cotangent edge--face incidence rate that produced the exact transverse Maxwell
sector, then

\[
 \boxed{c_T=c_{\rm EM}={1\over6}.}                 \tag{27}
\]

Equation (27) is not yet derived: the electromagnetic collision exists, while
the tensor staggered lift does not. It is a sharp common-cone pass criterion,
not permission to insert the light speed into an unrelated tensor action.

---

## 6. Relation to the closed two-record route

This theorem does not reopen the closed same-site collision. That route:

1. preserved the phase-even pair $(Q,P)$ of equation (4);
2. was therefore first-derivative blocked by equation (7);
3. forced two additional phase-blind $E_g$ invariants; and
4. produced generic cubic birefringence without a TT cone.

The surviving route must instead transport an even/odd pair across a genuine
primal/dual stagger or pay the rank-twenty on-site phase/parity price. Its
finite collision/streaming map must reproduce equation (18), preserve the
already-passed seven-invariant Maxwell sector, and generate equation (22)
rather than impose it.

---

## 7. Gravity and lensing boundary

Equations (18)--(27) close the previously ambiguous **linear spin-2-equivalent
target type**. They do not close gravity. A radiative parity-correct Weyl pair
does not by itself provide:

- the constrained static scalar/vector sector;
- universal coupling to the complete work/energy of every material clock;
- an inverse-distance static response;
- nonlinear self-coupling or an Einstein-equivalent completion;
- modification of the electromagnetic principal symbol by the sourced
  capacity background; or
- light deflection and Shapiro delay.

Lensing requires the same solved capacity background to enter both material
clock transport and the cotangent Maxwell incidence operator. Merely adding a
free tensor wave leaves the existing class-0 lensing null unchanged.

The later
[primal/dual permission theorem](../common_action_mechanics_reciprocity/THEOREM_PRIMAL_DUAL_PERMISSION_IDEMPOTENCE_AND_LENSING_FACTOR_PRICE_v1.md)
reaches the same ownership price independently from weak lensing. One binary
permission is idempotent and cannot supply separate temporal and spatial
responses; a retained primal/dual pair admits exact factorized blocking and
conditionally reaches class 2 under equal marginals. This convergence makes
the even/odd stagger the smallest shared spin-2/lensing candidate, but does
not construct its finite cotangent lift.

The successor
[A9/cotangent no-spare-scalar theorem](../common_action_mechanics_reciprocity/THEOREM_A9_COTANGENT_NO_SPARE_SCALAR_PERMISSION_AND_DUAL_COPY_PRICE_v1.md)
fixes the ownership type more narrowly. The existing A9 link/reserve
capacities are complements, and the transitive cotangent flag has no
nonconstant invariant scalar permission. The smallest existing-alphabet
candidate is therefore a second independently owned A9 record on the dual
face, where it can carry both scalar Hodge admission and the already-required
odd tensor placement. Its local generator is still open.

The later
[dual-A9 skew generator](../common_action_mechanics_reciprocity/THEOREM_DUAL_A9_SKEW_CAPACITY_CLOCK_GENERATOR_AND_HOMOGENEOUS_FACTOR_PASS_v1.md)
closes one homogeneous reference generator on those placements. Every orbit
factorizes at half admission, and applying its joint gate to both first-moment
operators would preserve $c_T=c_{\rm EM}=1/24$. The actual odd STF finite lift,
TT constraints, and sourced variable response remain open.

The subsequent
[dual-capacity mixing theorem](../common_action_mechanics_reciprocity/THEOREM_DUAL_CAPACITY_CORRELATION_OBSTRUCTION_AND_CYCLIC_MIXING_RESPONSE_v1.md)
proves exact arbitrary-count factorization after reversible dual-layer
translation. Its
[3D successor](../common_action_mechanics_reciprocity/THEOREM_OH_MOORE_LOCAL_DUAL_CAPACITY_MIXING_AND_ISOTROPIC_FACTOR_PASS_v1.md)
supplies an exact $O_h$-balanced full-Moore reference schedule with zero drift
and isotropic second moment. Native schedule selection, a C18-only lift if
required, and proof that its transported odd STF payload preserves TT and
Maxwell incidence remain open.

The
[single-record STF streaming-lift obstruction](THEOREM_COTANGENT_SINGLE_RECORD_STF_STREAMING_LIFT_OBSTRUCTION_v1.md)
now closes the smallest implementation route negative. Every
$O_h$-equivariant phase-dependent C18 route has zero co-layer STF first
moment. Adjacent-layer staggers produce only a rank-three cubic operator
family, while adjoining the symmetric-curl target raises the exact span rank
to four. The surviving lift must therefore contain a genuine multi-record
parity-changing collision, a larger carrier, or a longer-range construction.
Its
[right-regular collision successor](THEOREM_COTANGENT_RIGHT_REGULAR_COLLISION_SPIN2_SLOW_CLOSURE_OBSTRUCTION_v1.md)
closes every one-record cubic-equivariant local permutation as well: the curl
appears only in projections that lose zero-momentum slow closure, while all
slow-preserving collisions have zero derivative span.
The
[rank-twenty closure theorem](THEOREM_COTANGENT_RANK20_COLLISION_CLOSURE_AND_TT_LEAKAGE_v1.md)
then retains the missing copies exactly. The carrier closes, but an embedded
four-dimensional TT seed grows to 8--18 dimensions and has
direction-dependent spectrum. Its
[constraint-count successor](THEOREM_COTANGENT_RANK20_CONSTRAINT_COUNT_OBSTRUCTION_v1.md)
proves that the enlarged carrier requires total phase-space reduction
$2F+S=16$; four optimistic first-class constraints would still leave twelve
dimensions. Its
[common-closure successor](THEOREM_COTANGENT_COMMON_MAXWELL_TENSOR_COLLISION_CLOSURE_PRICE_v1.md)
further raises the smallest target-containing Maxwell-plus-tensor invariant
carrier to dimension thirty.

The second-order STF action successor opens a separate construction branch.
It reuses an inversion-even STF potential/momentum pair, imposes local
divergence constraints, and derives two stable tensor modes at speed \(1/6\)
without an explicit odd first-derivative carrier. Its exact price is a
Jordan/gauge-degenerate uniform mode and selected constraint multipliers. The
local manifestation STF source is invertible and never TT, so this smaller
radiative carrier still requires scalar/vector constraint sectors. Those
sectors are also the missing static and lensing response.

---

## 8. Next locked gate

Construct one of the two now-live finite tensor-action branches:

1. a multi-record parity-staggered symmetric-curl lift using the existing
   flag; or
2. the second-order even-STF potential action with native local constraint
   multipliers and its Jordan/gauge zero-mode ownership.

Either branch must:

1. generate and preserve the TT constraints in equation (22);
2. retain the exact Maxwell cone and Gauss source transaction;
3. derive the common speed in equation (27) from one incidence ledger;
4. couple the even packet source in equation (12) reciprocally;
5. supply scalar/vector constraint sectors for the non-TT local source; and
6. expose the static capacity response needed for a subsequent blind lensing
   fixture.

Only that finite lift would turn this target theorem into a native propagating
spin-2-equivalent sector.
