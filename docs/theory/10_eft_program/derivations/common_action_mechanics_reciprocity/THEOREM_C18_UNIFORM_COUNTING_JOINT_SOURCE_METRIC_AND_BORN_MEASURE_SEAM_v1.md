# C18 uniform-counting joint source metric and Born-measure seam v1

**Date:** 2026-08-24

**Status:** **[THEOREM — EXACT RANK-24 JOINT COUNTING-MEASURE METRIC]** +
**[THEOREM — COORDINATE-INVARIANT COMMON-EVENT COST]** +
**[THEOREM — PREPARED ORDERED-PAIR BORN COUNTING PUSHFORWARD]** +
**[CONDITIONAL — ONE BARE ACTION/MEASURE SEAM]** +
**[BOUNDARY — SC/FCC SHEAR ANISOTROPY]** +
**[OPEN — DYNAMICAL HESSIAN, NATIVE PREPARATION, POLES, LENSING, AND COUPLING]**

**Production status:** unchanged

**Ledger status:** no FTD claim row minted

**Locked preregistration:**
[PREREG_C18_UNIFORM_COUNTING_JOINT_SOURCE_METRIC_AND_BORN_MEASURE_SEAM_v1.md](../../preregistrations/common_action_mechanics_reciprocity/PREREG_C18_UNIFORM_COUNTING_JOINT_SOURCE_METRIC_AND_BORN_MEASURE_SEAM_v1.md),
pre-execution SHA-256
`775EB8F05FF495C2A7CA6D652A686323527A31E70E350BCB67C0B28F36BF26DB`.

**Exact certificate:**
[proof_c18_uniform_counting_joint_source_metric_born_measure_seam.py](../../../../../scripts/proofs/proof_c18_uniform_counting_joint_source_metric_born_measure_seam.py),
SHA-256
`16B488596270EED63A71D49B09391C7E6E5C336EF2D7681C4C35032300FAF1F6`,
performs 5,504 exact rational, symbolic, finite-orbit, signed-cubic, source,
and reparameterization checks. No target coupling, gravity coefficient,
deflection value, master root, experimental constant, fitted probability, or
numerical search enters.

---

## 1. One finite counting measure sees all source blocks

Use the existing C18 five-state record alphabet

\[
 \mathcal A_5=\{0,1,i,-1,-i\}                         \tag{1}
\]

with uniform counting measure independently on the two directions of every
antipodal line. The exact block readouts are:

\[
 (R_u,R_v)\in\mathbb R^3\oplus\mathbb R^3,             \tag{2}
\]

\[
 (Q,P)\in\operatorname{Sym}(3)\oplus\operatorname{Sym}(3),
 \qquad K\in\operatorname{Sym}(3).                     \tag{3}
\]

The first pair is the relative phase-current doublet, the second is the
common tensor quadrature doublet, and the final tensor is blank capacity.
They are not three independently chosen probability models. They are three
block readings of the same uniform finite record measure.

For one antipodal line, direct enumeration of all 25 record pairs gives the
scalar covariance

\[
 \operatorname{Cov}(r_u,r_v,q,p,k)
 =\operatorname{diag}\left(
 {4\over405},{4\over405},{1\over405},{1\over405},
 {2\over2025}\right).                                  \tag{4}
\]

Every cross covariance vanishes as an output of the enumeration. Relative
and common phase, the two C4 quadratures, and phase versus capacity are
orthogonal at this bare quadratic level.

---

## 2. Exact joint covariance

In symmetric coordinates \((xx,yy,zz,xy,xz,yz)\), define

\[
 A_6=
 \begin{pmatrix}
 4&1&1&0&0&0\\
 1&4&1&0&0&0\\
 1&1&4&0&0&0\\
 0&0&0&1&0&0\\
 0&0&0&0&1&0\\
 0&0&0&0&0&1
 \end{pmatrix}.                                           \tag{5}
\]

Summing the nine independent line contributions gives

\[
 \boxed{\Sigma_R={4\over135}I_6,}                       \tag{6}
\]

\[
 \boxed{
 \Sigma_T=\operatorname{diag}(A_6/810,A_6/810),
 \qquad \Sigma_K=A_6/2025.}                             \tag{7}
\]

Consequently

\[
 \boxed{
 \Sigma_{\rm joint}
 =\operatorname{diag}(\Sigma_R,\Sigma_T,\Sigma_K),
 \qquad \operatorname{rank}\Sigma_{\rm joint}=24.}     \tag{8}
\]

All 24 eigenvalues are positive. Thus the finite alphabet supplies a
nondegenerate local quadratic metric simultaneously on the electromagnetic
source doublet, tensor phase space, and capacity sector.

This is a counting-measure/Fisher or large-deviation metric. Equation (8) is
not yet a spacetime kinetic operator.

---

## 3. The common event has one invariant metric cost

For the existing one-token manifestation event, let \(d\) be a normalized
C18 line, \(M=dd^{\mathsf T}\), \(i^k=u+iv\), and
\(\epsilon=\pm1\). Its source increment is

\[
 \delta R_u={\epsilon u\over9}d,qquad
 \delta R_v={\epsilon v\over9}d,                       \tag{9}
\]

\[
 \delta Q={u\over18}M,qquad
 \delta P={v\over18}M,qquad
 \delta K=-{1\over18}M.                                \tag{10}
\]

Define

\[
 C_X={1\over2}\delta X^{\mathsf T}\Sigma_X^{-1}\delta X,
 \qquad X\in\{R,T,K\}.                                \tag{11}
\]

For every C4 phase and both charge orientations on a signed-SC line,

\[
 \boxed{
 C_R={5\over24},\qquad
 C_T={25\over72},\qquad
 C_K={125\over144}.}                                    \tag{12}
\]

Therefore

\[
 \boxed{C_{\rm joint}^{\rm SC}={205\over144}.}          \tag{13}
\]

The tensor and capacity trace/STF split is

\[
 C_T^{\rm tr}={5\over72},\qquad
 C_T^{\rm STF}={5\over18},                              \tag{14}
\]

\[
 C_K^{\rm tr}={25\over144},\qquad
 C_K^{\rm STF}={25\over36}.                             \tag{15}
\]

In particular, the source-block ratios

\[
 {C_T\over C_R}={5\over3},qquad
 {C_K\over C_R}={25\over6}                              \tag{16}
\]

are exact properties of the joint bare metric and the one-token event. They
are not continuum constants.

---

## 4. Why this is stronger than the raw norm ledger

Let the three block coordinates be changed independently:

\[
 R'=L_RR,qquad T'=L_TT,qquad K'=L_KK,                  \tag{17}
\]

with each \(L_X\) invertible. Then

\[
 \delta X'=L_X\delta X,qquad
 \Sigma_X'=L_X\Sigma_XL_X^{\mathsf T}.                 \tag{18}
\]

Exactly,

\[
 \boxed{
 {1\over2}\delta X'^{\mathsf T}\Sigma_X'^{-1}\delta X'
 ={1\over2}\delta X^{\mathsf T}\Sigma_X^{-1}\delta X.} \tag{19}
\]

The certificate verifies equation (19) symbolically for independent block
scales and exactly for nonsingular rational mixing charts, including an FCC
source with live off-diagonal tensor components.

The predecessor correctly proved that raw Euclidean source norms and the
displayed generator coefficients change under canonical rescaling. Equation
(19) closes that objection **for the counting metric itself**. It does not
show that physical propagation uses this metric. The remaining question is
dynamical rather than notational:

\[
 \boxed{
 G_{\rm dyn}\stackrel{?}{=}\lambda\Sigma_{\rm joint}^{-1}}
                                                               \tag{20}
\]

for one common action scale \(\lambda\), after interacting blocking.

---

## 5. The bare metric exposes rather than hides cubic anisotropy

For an FCC line, the same exact calculation gives

\[
 C_R={5\over24},\qquad
 C_T={65\over144},\qquad
 C_K={325\over288},                                     \tag{21}
\]

and

\[
 \boxed{C_{\rm joint}^{\rm FCC}={515\over288}.}         \tag{22}
\]

The trace costs are identical to equations (14)--(15), but

\[
 C_T^{\rm STF}={55\over144},qquad
 C_K^{\rm STF}={275\over288}.                            \tag{23}
\]

All 48 signed-cubic transformations preserve the appropriate SC or FCC value.
Nevertheless,

\[
 C_{\rm joint}^{\rm SC}\ne C_{\rm joint}^{\rm FCC}.     \tag{24}
\]

Thus the bare measure is exactly \(O_h\)-covariant but not accidentally
\(SO(3)\)-isotropic in the shear sector. A native spin-2-equivalent action
must derive an interacting infrared flow or constraint reduction that removes
this shell distinction. Averaging equations (13) and (22) by hand would be an
imposed normalization.

---

## 6. The same counting principle reaches the prepared Born map

For one residual C4 bank with phase counts

\[
 (n_0,n_1,n_2,n_3),qquad
 Z=(n_0-n_2)+i(n_1-n_3),                                \tag{25}
\]

the two consecutive address rings traverse every ordered bank pair exactly
once. Pushing this uniform orbit count through the same-route, same-rail
compatibility event gives

\[
 \boxed{
 M=|Z|^2=(n_0-n_2)^2+(n_1-n_3)^2.}                      \tag{26}
\]

The certificate exhausts all \(5^4\) count vectors with
\(0\le n_p\le4\) and four multi-outcome banks. No probability table is read.

Equations (8) and (26) therefore share one finite combinatorial principle:

\[
 \boxed{
 \begin{array}{c}
 \text{uniform finite counting measure}\\[2pt]
 \swarrow\qquad\searrow\\[-2pt]
 \text{block-multiplicity Hessian}
 \qquad
 \text{ordered-history-pair pushforward}.
 \end{array}}                                             \tag{27}
\]

This is the first exact common **action/measure candidate seam** in the
current chain. It is not yet a physical Born derivation. The blocked metric
uses the invariant uniform product measure, while the Born map still assumes
a prepared residual bank and an equidistributing address orbit. No theorem
yet proves that the native source dynamics prepares that bank or samples the
required invariant sector.

---

## 7. Consequences for the one-action objective

### Manifestation, matter, and clocks

The same owned token whose finite move manifests the ternary pair now has one
coordinate-invariant insertion cost in the joint bare chart. The signed event
generator still supplies recoil and clock action exactly. Stable localized
matter, reserve formation, and the physical identification of this cost with
clock work remain open.

### Electromagnetism and native alpha

Equation (12) supplies a non-arbitrary bare normalization candidate for the
electromagnetic source block. It does **not** imply

\[
 C_R=\chi_{\rm EM}.                                      \tag{28}
\]

The left side is a static one-cell insertion cost. The right side is the
curvature of the common propagating Maxwell--Gauss action, measurable from
both a free-field Hessian and a charged static residue. Until equation (20)
is derived for the Maxwell block and both measurements agree, no native
fine-structure coupling follows and the master root remains an external
mathematical correspondence.

### Gravity, spin-2, and lensing

The trace and STF source costs are now fixed inside the same bare metric as
the electromagnetic source. This does not create spatial derivatives,
constraints, a static pole, or tensor radiation. Equation (24) is instead an
honest obstruction to reading the bare Hessian as the completed gravity
action. Native lensing still requires one interacting action to produce the
self-dual static capacity mode, equal temporal/spatial Maxwell response, the
vector constraint, and two isotropic tensor modes.

### Contextual measurement

Equation (26) is a real physical-event count once the prepared renewal
detector owns the event resource. What remains missing is the endogenous map

\[
 \text{native source orbit}
 \longrightarrow
 \text{residual bank and address measure}.               \tag{29}
\]

Uniform invariance of a reversible permutation does not by itself prove
ergodicity, preparation, or operational no-signalling.

---

## 8. Epistemic disposition

### Established exactly

1. the full joint covariance has rank 24 and is positive;
2. every cross block vanishes under the uniform finite measure;
3. one SC event has the exact source costs (12)--(15);
4. the complete cost is invariant under invertible coordinate changes;
5. signed-cubic covariance holds on both line shells;
6. the bare SC/FCC shear anisotropy is nonzero;
7. the prepared uniform ordered-pair pushforward gives equation (26); and
8. all 5,504 locked checks pass, **Outcome A at bare-measure level**.

### Not established

1. equality of the Fisher and dynamical action Hessians;
2. interacting stationary-measure selection or ergodicity;
3. autonomous bank, owner, Hodge-frame, or detector formation;
4. stable composite matter;
5. charged Maxwell continuity and a common static/radiative curvature;
6. a native spin-2-equivalent pole or scalar/vector constraint algebra;
7. native lensing or Shapiro delay;
8. a physical general Born pushforward or multipartite no-signalling; or
9. any value of \(\alpha\), \(G_N\), mass, or another empirical constant.

---

## 9. Next locked discriminator

The next pass must be dynamical. Starting from a payload-complete local
collision and streaming rule, it must derive its interacting two-point block
kernel and test, before examining any target constant, whether:

1. the stationary sector is the finite counting measure used above or a
   uniquely derived alternative;
2. its low-energy kinetic metric obeys equation (20) with at most one common
   action scale;
3. the relative-vector block retains the exact two-polarization Maxwell cone
   and gains local charged Gauss continuity;
4. the capacity/tensor block removes the SC/FCC shear split, produces the
   self-dual static pole, and retains two tensor modes;
5. the same event and inverse source every block with positive reciprocal
   work;
6. the source dynamics forms the residual C4 bank and renewal orbit; and
7. blind static/free-field curvature, fall/clock/lensing, and fixed-window
   event-frequency observables can then be evaluated without target data.

Only such a pass can promote the common counting seam into the requested
native action.

