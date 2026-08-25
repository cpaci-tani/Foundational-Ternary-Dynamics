# Global-C3 cotangent-layer collision and vacuum-Maxwell pass v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT REVERSIBLE LAYER-COVARIANT COLLISION]** +
**[THEOREM — FIRST-ORDER FINITE-ACTION VACUUM MAXWELL SECTOR]** +
**[OPEN — LOCAL CHARGED GAUSS/CONTINUITY, SOURCE WORK, PHYSICAL COUPLING]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificates:**

- [proof_global_c3_cotangent_layer_equivariant_collision.py](../../../../../scripts/proofs/proof_global_c3_cotangent_layer_equivariant_collision.py):
  128,499 exact checks; and
- [proof_global_c3_cotangent_layer_full_tick_maxwell_vacuum.py](../../../../../scripts/proofs/proof_global_c3_cotangent_layer_full_tick_maxwell_vacuum.py):
  exact independent-binary product-reference and three-tick Floquet derivative.

No fitted coefficient, target-spectrum search, or measured quantity enters the
construction.

**Certificate correction (2026-08-24):** the original final two-row witness
used the constant graph $E_\parallel=\kappa\rho$ while describing it as the
derivative graph $kE_\parallel=\kappa\rho$.  The corrected certificate tests
both graphs separately.  The former is precisely the acoustic characteristic
pair; the latter gives equation (11).  The theorem conclusion is unchanged,
but the earlier witness did not by itself establish it.

---

## 1. Base collision

At cotangent layer zero, one flag reads

\[
 E=d,
 \qquad B=n,                                       \tag{1}
\]

where $d$ is a polar SC direction and $n$ is a perpendicular axial SC
direction. The 18,336 unordered pair states form 253 exact $(E,B)$ sectors,
with histogram

\[
 \{28^{24},64^{192},128^{36},768^1\}.              \tag{2}
\]

The complete relation has transition rank 185 and exactly seven additive
invariants: record number and the six components of $(E,B)$.

Under $O_h\times C_4$, the pair space decomposes into 115 orbits,

\[
 \{96^{39},192^{76}\}.                             \tag{3}
\]

Every orbit admits a field-preserving fixed-point-free self-involution. A
deterministic exact rank-greedy selection constructs one base collision $C_0$
that is reversible, $O_h\times C_4$ equivariant, and has rank 185.

## 2. One clock-indexed collision family

Let $U$ be the shared-edge flag update plus C4 advance. Define the remaining
layer collisions by conjugation:

\[
 C_2=UC_0U^{-1},
 \qquad
 C_1=U^2C_0U^{-2}.                                 \tag{4}
\]

Because $C_0$ commutes with the three-tick return $U^3$, equation (4) closes
as one period-three rule and obeys

\[
 \boxed{UC_q=C_{q-1}U}.                            \tag{5}
\]

All three collisions are fixed-point-free involutions, preserve the field
readout appropriate to their clock layer, and have exact rank 185/nullity
seven.

Their independent-binary product-reference corrections are symmetric
negative-semidefinite matrices $N_q$ with

\[
 J_q=I+\frac{N_q}{2^{191}},
 \qquad
 N_{q-1}=UN_qU^T.                                  \tag{6}
\]

The extremely small coefficient in equation (6) is the exact tangent weight
of an event containing exactly two occupied channels among 192 independent
half-occupied binary channels. It is a reference-distribution property, not a
physical fine-structure coupling.

## 3. Direct three-tick Floquet derivative

Starting at layer zero, the collision--streaming sequence is $(0,2,1)$. The
certificate differentiates the complete three-tick product, not merely a
continuum target. Covariance and the exact left/right slow spaces give

\[
 \frac{1}{3}D_kF_3(0)\big|_{\rm slow}
 =-\frac{i}{3}\sum_{q=0}^2 A^{(q)}(k).             \tag{7}
\]

Its characteristic polynomial is

\[
 \boxed{
 \lambda
 \left(\lambda^2+\frac{|k|^2}{27}\right)
 \left(\lambda^2+\frac{|k|^2}{36}\right)^2.}      \tag{8}
\]

The constrained vacuum subspace

\[
 \rho=0,
 \qquad k\cdot E=0,
 \qquad k\cdot B=0                                \tag{9}
\]

is first-order invariant and contains exactly two transverse
electric--magnetic polarization pairs with speed $1/6$ lattice units per
tick. This is the first finite selected collision in the strict-discrete chain
to pass the native vacuum-Maxwell first-order pole test.

## 4. Charged Gauss boundary

The remaining scalar block couples the uniform carrier-number perturbation
$n$ and longitudinal electric field.  Writing that coordinate as $\rho$ in
this section is the candidate identification being tested; manifested charge
has not been derived from it.  Its two wavevector-independent characteristic
graphs are

\[
 E_\parallel=\eta\rho,
 \qquad \eta^2=\frac13,                            \tag{10}
\]

which are the two acoustic characteristics—not Gauss law.  A putative local
Fourier Gauss graph instead contains one spatial derivative,

\[
 k\cdot E=\kappa\rho,                             \tag{11}
\]

and its exact invariance residual is proportional to

\[
 |k|\left(\frac{|k|^2}{9}-\frac{\kappa^2}{3}\right).
                                                               \tag{12}
\]

It therefore vanishes only if

\[
 \kappa^2=\frac{|k|^2}{3}.                         \tag{13}
\]

Therefore no wavevector-independent local constant $\kappa$ turns this
acoustic pair into Gauss law. The vacuum pass does not yet supply charged
electromagnetism.

## 5. Next common-action gate

The next action must distinguish carrier number from manifested charge and
introduce a local current/source transaction satisfying

\[
 \Delta\rho+\nabla\cdot j=0,
 \qquad
 \Delta(\nabla\cdot E-\rho)=0.                    \tag{14}
\]

That transaction must use the existing actualization payload and capacity
ledger, select the perpendicular face context required by the cotangent flag,
and preserve the transverse coefficient in equation (8). Only its resulting
dimensionless source-to-field response could become a native electromagnetic
coupling observable.

The subsequent stabilizer-packet theorem closes the local canonical Gauss-edge
source, while the
[native-alpha action-scale obstruction](THEOREM_COTANGENT_NATIVE_ALPHA_ACTION_SCALE_OBSTRUCTION_v1.md)
shows that the packet norm and speed do not fix the action paid per field
quantum. A charged massless pole and the blocked action-curvature ratio remain
required before a coupling can be measured.

The later
[framed-plaquette radiation-release theorem](THEOREM_COTANGENT_FRAMED_PLAQUETTE_NUMBER_NEUTRAL_RADIATION_RELEASE_v1.md)
constructs the first finite source payload whose signed occupation increment
lies in the constrained vacuum sector of equation (9): it has
\(\Delta N=0\), closed electric boundary, and a transverse first Bloch moment.
Thus the present Floquet derivative propagates that seed at first order. A
finite-amplitude pair schedule, positive emission work, and reciprocal matter
force remain open.

The later
[common-admission clock/Maxwell theorem](../common_action_mechanics_reciprocity/THEOREM_COMMON_ADMISSION_CLOCK_MAXWELL_AND_SPATIAL_LENSING_PRICE_v1.md)
proves that gating the entire cotangent Floquet advance by the material
clock's retained capacity permission scales the transverse speed from $1/6$
to $\nu/6$. In weak capacity depth this gives $a_0=a_t$. It is conditional on
a finite common gate and does not alter this vacuum certificate or the
production wave stencil.

The subsequent
[primal/dual permission theorem](../common_action_mechanics_reciprocity/THEOREM_PRIMAL_DUAL_PERMISSION_IDEMPOTENCE_AND_LENSING_FACTOR_PRICE_v1.md)
shows that a duplicated read of that binary gate cannot produce a second
spatial coefficient. A separate retained dual permission can scale the
first-order Bloch displacement while the primal permission scales clock
advance; exact factorized blocking then gives $c_{\rm ray}=\nu_t\nu_s/6$.
This is a type-price result, not yet a finite modification of the cotangent
collision or streaming map.
