# Even-STF second-order action spin-2 escape and constraint price v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EVEN/EVEN FIRST-DERIVATIVE NO-GO HAS A JORDAN/SECOND-ORDER ESCAPE]** +
**[THEOREM, CONDITIONAL — EXACT LOCAL REVERSIBLE TWO-TT-MODE ACTION]** +
**[THEOREM — POSITIVE NONZERO-MODE INVARIANT AND FULL-BAND \(c_T=1/6\) STABILITY]** +
**[SCOPED NO-GO — LOCAL MANIFESTATION STF STRESS CANNOT SOURCE A TT-ONLY ACTION]** +
**[SELECTION PRICE — LOCAL DIVERGENCE CONSTRAINTS AND ZERO-MODE DEGENERACY]** +
**[OPEN — NATIVE MULTIPLIERS, STATIC CONSTRAINT SECTOR, UNIVERSAL COUPLING, LENSING]**  
**Production status:** unchanged  
**Ledger status:** no row minted; this is an alternate gravity-action route,
not a native gravity closure

**Exact certificate:**
[proof_even_tensor_second_order_action_spin2_escape.py](../../../../../scripts/proofs/proof_even_tensor_second_order_action_spin2_escape.py)
performs **29,443 exact checks**. It exhausts all 98 primitive nonzero integer
wavevectors in \([-2,2]^3\), every signed-cubic transformation, the complete
STF divergence kernel, the discrete variational transfer, its symplectic and
energy invariants, and every oriented manifestation STF source. No gravity
coefficient, deflection target, master root, continuum metric fit, or measured
constant enters.

---

## 1. Result

The parity theorem proved that an inversion-even STF tensor cannot acquire a
massless cone from an inversion-preserving **first-spatial-derivative
even-to-even block**. That result is correct in its registered class.

There is a distinct local variational route. Let

\[
 h_n(x)\in\operatorname{STF}(3)                            \tag{1}
\]

be an inversion-even symmetric trace-free potential on the cubic lattice.
Choose the nearest-neighbor quadratic action

\[
 \boxed{
 {\cal S}_T
 =\sum_{n,x}\left[
 {1\over2}\|\Delta_t h_n\|_F^2
 -{c_T^2\over2}\sum_{a=1}^{3}
 \|\Delta_a h_n\|_F^2
 +\xi_{n,i}\Delta^-_j h_{n,ij}
 \right].}                                                \tag{2}
\]

The local multiplier \(\xi_i\) imposes

\[
 \boxed{\Delta^-_j h_{ij}=0.}                             \tag{3}
\]

Trace freedom is already built into equation (1). At every nonzero
wavevector, the divergence map from the five-dimensional STF space has exact
rank three. Therefore

\[
 \boxed{\dim\ker(\operatorname{div}|_{\rm STF})=2.}        \tag{4}
\]

Equation (2), restricted by equation (3), is an exact local two-tensor-mode
action. The constraints and multiplier ownership are **selected reference
structure**; the current finite cotangent collision has not generated them.

---

## 2. Exact discrete transfer

For either of the two TT coordinates, let the positive cubic lattice
Laplacian symbol be

\[
 \Lambda(k)=\sum_{i=1}^{3}\left(2-z_i-z_i^{-1}\right)
 =4\sum_{i=1}^{3}\sin^2{k_i\over2}.                       \tag{5}
\]

Set

\[
 a(k)=c_T^2\Lambda(k).                                     \tag{6}
\]

The discrete Euler--Lagrange equation is

\[
 h_{n+1}-(2-a)h_n+h_{n-1}=0.                              \tag{7}
\]

With half-step momentum

\[
 p_{n-1/2}=h_n-h_{n-1},
\]

equation (7) becomes the local kick--drift map

\[
 \begin{pmatrix}h_{n+1}\\p_{n+1/2}\end{pmatrix}
 =
 \boxed{
 M(a)
 \begin{pmatrix}h_n\\p_{n-1/2}\end{pmatrix}},
\qquad
 M(a)=
 \begin{pmatrix}
 1-a&1\\
 -a&1
 \end{pmatrix}.                                           \tag{8}
\]

It obeys

\[
 \det M=1,\qquad M^{\mathsf T}JM=J,                        \tag{9}
\]

so the transfer is exactly reversible and symplectic.

---

## 3. Positive invariant and lattice cone

For

\[
 0<a<4,                                                    \tag{10}
\]

the matrix

\[
 G(a)=
 \begin{pmatrix}
 a&-a/2\\
 -a/2&1
 \end{pmatrix}                                            \tag{11}
\]

is positive definite because

\[
 \det G=a\left(1-{a\over4}\right)>0.                      \tag{12}
\]

Direct multiplication gives

\[
 \boxed{M^{\mathsf T}GM=G.}                               \tag{13}
\]

Thus every nonzero TT lattice mode has an exact positive conserved
quadratic.

The transfer polynomial is

\[
 z^2-(2-a)z+1=0,                                          \tag{14}
\]

or

\[
 \cos\omega(k)=1-{c_T^2\Lambda(k)\over2}.                 \tag{15}
\]

Since \(0\leq\Lambda\leq12\), the common-cone candidate

\[
 c_T={1\over6}                                             \tag{16}
\]

gives

\[
 0\leq a\leq{1\over3}<4.                                 \tag{17}
\]

The complete cubic Brillouin band is therefore stable. At long wavelength,

\[
 \Lambda(k)=|k|^2+O(|k|^4),
\]

and equation (15) gives

\[
 \boxed{\omega(k)={|k|\over6}+O(|k|^3).}                  \tag{18}
\]

The two TT coordinates therefore produce two degenerate massless
long-wavelength tensor modes at the already selected Maxwell speed.

This common speed is conditional on choosing equation (16). The finite
cotangent transaction has not yet derived equation (2) or its coefficient
from the same incidence ledger as Maxwell.

---

## 4. How this escapes the parity obstruction

At \(k=0\), equation (8) becomes

\[
 M(0)=
 \begin{pmatrix}1&1\\0&1\end{pmatrix}.                     \tag{19}
\]

It has only one eigenvector and obeys

\[
 (M-I)^2=0,\qquad M\ne I.                                 \tag{20}
\]

The zero mode is a Jordan block. At the same point the invariant
\(G(0)\) is positive semidefinite rather than positive definite.

This is the exact mechanism producing the linear cone. The transfer depends
on the spatial wavevector only through the even quantity \(a\sim|k|^2\), but
the eigenvalue discriminant is

\[
 \boxed{a(a-4),}                                          \tag{21}
\]

whose square root is linear in \(|k|\).

By contrast, if the slow block at \(k=0\) is semisimple identity and its
analytic inversion-even perturbation begins at \(k^2\), its eigenvalues move
only by \(O(k^2)\). The linear cone is unavailable.

Therefore:

> An even/even second-order tensor action can evade the first-derivative
> parity no-go only by paying a non-semisimple or gauge-degenerate zero-mode
> price.

The earlier rank-twenty parity-complete theorem remains valid for a
first-order even/odd symmetric-curl carrier with positive onsite norm. The
present theorem is outside that class. It offers a second route:

1. retain the inversion-even STF potential/momentum pair;
2. derive the cone through a second-order local action; and
3. pay local constraints plus a degenerate uniform mode instead of an
   on-site even/odd rank-twenty carrier.

---

## 5. Relation to the existing C4 tensor doublet

The current cotangent alphabet already spans two inversion-even STF
quadratures

\[
 (Q,P)\in V_2\oplus V_2,                                  \tag{22}
\]

of total rank ten. That is the correct capacity for a five-component
potential and its conjugate momentum. Equation (4) would reduce its nonzero
radiative phase space to

\[
 2\ \text{TT configurations}
 +2\ \text{TT momenta}
 =4,                                                       \tag{23}
\]

equivalent to two propagating polarizations.

Capacity is not realization. The existing C4 quarter-turn is an internal
clock permutation, whereas equation (8) is a wavevector-dependent
kick--drift transfer with a Jordan uniform mode. A native theorem must derive
the map between those presentations and provide finite local ownership for
\(\xi_i\).

Calling \((Q,P)\) spin-2 without equations (2)--(4) would remain an
overstatement. The result here is that the existing rank-ten type is
sufficient for an alternate constrained second-order action, not that the
present collision already implements it.

---

## 6. The local manifestation source is not TT

The phase-neutral manifestation vertex produces the oriented STF stress

\[
 T_d=dd^{\mathsf T}-{1\over3}I.                            \tag{24}
\]

For every SC direction,

\[
 \boxed{\det T_d={2\over27}\ne0.}                          \tag{25}
\]

Hence \(T_d\) is invertible. For every nonzero wavevector,

\[
 \boxed{T_d k\ne0.}                                       \tag{26}
\]

The local event stress is never a TT plane-wave source. A theory containing
only the two radiative TT modes cannot couple locally to manifestation
without either:

1. applying a nonlocal TT projection;
2. violating the constraints; or
3. adding local scalar/vector constraint sectors that absorb the
   longitudinal source.

The first option is incompatible with the native-locality objective.
Therefore equation (26) makes the scalar/vector sector mandatory.

This is the central gravity result of the theorem:

> The missing static constraint sector is simultaneously what permits a
> local manifestation source, what generates a Newtonian-like exterior, and
> what can modify the temporal/spatial Maxwell operator for lensing.

The radiative tensor capacity is no longer the only bottleneck.

---

## 7. Lensing consequence

Equation (2) alone describes free radiative tensor modes. It does not provide:

- a static scalar pole;
- slow-body acceleration;
- material-clock response;
- temporal Maxwell admission;
- spatial Hodge response;
- light deflection; or
- Shapiro delay.

The current lensing discriminator requires the four blind derivatives

\[
 (a_m,a_t,a_0,a_s).                                       \tag{27}
\]

A TT-only action leaves all four undetermined. The required extension is a
single local constrained capacity action whose scalar and vector multipliers
are dynamical response fields, not externally chosen schedules. The same
action must:

1. solve the longitudinal part of equation (24);
2. couple the solution to the material recoil action and internal clock;
3. gate the temporal Maxwell advance;
4. modify the spatial primal/dual Hodge incidence; and
5. leave equation (18) as the radiative tensor pole.

If that common constraint action forces

\[
 a_m=a_t=a_0=a_s,                                         \tag{28}
\]

the existing blind discriminator would give equal temporal and spatial
lensing response. Equation (28) is a future pass condition, not a result of
this theorem.

---

## 8. Revised native spin-2 gate

There are now two honest construction branches.

### Branch A: first-order parity carrier

- retain even and odd STF fields;
- realize the symmetric tensor curl;
- pay rank twenty before the C4 phase-complete lift;
- derive constraints that remove TT leakage.

### Branch B: second-order constrained potential

- retain the existing rank-ten even STF potential/momentum type;
- derive equation (2) and its local multipliers;
- accept the Jordan/gauge-degenerate zero mode;
- generate scalar/vector constraints and static response in the same action.

Branch B is smaller in carrier rank and connects more directly to the static
lensing debt. Its harder price is variational: the native transaction must
generate the constraints rather than impose TT initial data.

Neither branch has yet been derived from the complete finite collision
action. This theorem makes Branch B a mathematically live alternative and
removes the false dichotomy that a native spin-2-equivalent cone must always
use an explicit inversion-odd tensor carrier.

---

## 9. Next acceptance gate

Construct the smallest finite capacity action extending equation (2) by one
scalar and one vector constraint sector. It must:

1. derive the divergence constraint rather than merely select it;
2. accept the local manifestation source in equation (24);
3. preserve exact reversal and a positive total nonzero-mode energy;
4. yield a nonzero static scalar Green response;
5. couple that response to material momentum and clock admission;
6. derive both temporal and spatial Maxwell response coefficients;
7. retain two radiative tensor modes with the common speed \(1/6\); and
8. evaluate the blind lensing/clock/fall ratios before any comparison with
   continuum GR.

Only that extension would unite the native spin-2-equivalent and lensing
gates in one action.

The subsequent
[C18 existing-type constraint-seam theorem](../common_action_mechanics_reciprocity/THEOREM_C18_EXISTING_TYPE_SCALAR_STF_VECTOR_CONSTRAINT_ABSORPTION_SEAM_AND_EQUAL_COUPLING_BOUNDARY_v1.md)
provides an exact blocked source interface for this branch. C18 has a spare
spatial-vector copy, and loading \(\Pi\mapsto\Pi+g_TS\) preserves the local
constraint only with \(\kappa\mapsto\kappa+g_TS q\). A single reciprocal
generator books that load with the packet recoil and scalar trace. The result
does not derive equation (2), the multiplier dynamics, the static pole, or
equal scalar/tensor normalization.

The subsequent
[transverse finite constraint-bundle theorem](../common_action_mechanics_reciprocity/THEOREM_C18_TRANSVERSE_CHARGE_EVEN_CONSTRAINT_BUNDLE_AND_AXIAL_TWO_OWNER_BOUNDARY_v1.md)
realizes the blocked longitudinal record by an exact charge-even,
EM-neutral retained-record map for every transverse nearest-neighbor chart.
The same census proves that an axial source needs both transverse plane
bundles, while the ordered axial \(D_4\) stabilizer forbids a scalar-phase
choice between them. This closes one finite source component but leaves the
second axial owner, native constraint algebra, static pole, and the coupling
to this conditional two-mode action open.

The later
[Hodge-framed all-axis signed-event theorem](../common_action_mechanics_reciprocity/THEOREM_HODGE_FRAMED_ALL_AXIS_CONSTRAINT_LIFT_AND_ONE_SIGNED_EVENT_GENERATOR_BOUNDARY_v1.md)
closes the remaining local source-chart context at blocked prepared-reference
level. The existing electromagnetic Hodge flag orients both axial plane
bundles, and one generator writes the tensor and forced longitudinal loads
with manifestation, recoil, and clock action. It still does not generate this
second-order tensor transfer, its multipliers, static sector, or kinetic
normalization.

---

## 10. Epistemic firewall

\[
\boxed{
\begin{array}{ll}
\text{STF TT configuration rank} & 2\ \text{exact},\\
\text{local discrete action transfer} & \text{exact conditional},\\
\text{nonzero-mode positive invariant} & \text{exact},\\
\text{full-band stability at }c_T=1/6 & \text{exact conditional},\\
\text{long-wave tensor cone} & \text{exact conditional},\\
\text{Jordan/degenerate escape price} & \text{exact},\\
\text{native constraint multipliers} & \text{open},\\
\text{finite cotangent realization} & \text{open},\\
\text{static gravity and universal source} & \text{open},\\
\text{lensing and Shapiro response} & \text{open},\\
\text{Einstein-equivalent completion} & \text{open}.
\end{array}}
\]
