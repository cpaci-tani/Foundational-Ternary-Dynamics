# C18 uniform-token bare blocking and shear Hessian v1

**Date:** 2026-08-23  
**Status:** **[THEOREM — EXACT FIVE-STATE AND C18 BLOCK COVARIANCES]** +
**[THEOREM — REVERSIBLE STREAMING PRESERVES UNIFORM COUNTING MEASURE]** +
**[THEOREM, CONDITIONAL — BARE LARGE-BLOCK QUADRATIC RESPONSE]** +
**[BOUNDARY — POSITIVE TRACE/SHEAR TYPE, CUBIC ANISOTROPY, NO TENSOR POLE]** +
**[OPEN — INTERACTING MICROSCOPIC ACTION, MATTER, LENSING, NATIVE COUPLING]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_c18_uniform_token_blocking.py](../../../../../scripts/proofs/proof_c18_uniform_token_blocking.py)
uses rational arithmetic to verify the alphabet covariance, antipodal
common/relative decomposition, one-hop streaming permutation, isotropic vector
moment, rank-six capacity covariance, exact inverse Hessian, and positive cubic
trace/shear sectors. It performs no fit and imports no physical constant.

---

## 1. Result

This is the first explicit strict-discrete bridge from a finite local carrier
to a real quadratic response in the one-action program.

Put one phase-complete five-state record on each directed C18 channel:

\[
 \lambda_{x,d}\in\mathcal C_4^0
 =\{0,1,i,-1,-i\}.                                  \tag{1}
\]

Write

\[
 \lambda=u+iv,\qquad
 c=1-u^2-v^2.                                       \tag{2}
\]

In the bare reference vacuum, use the uniform counting measure on the five
states. Then exactly

\[
 \mathbb E(u,v,c)=\left(0,0,{1\over5}\right),        \tag{3}
\]

\[
 \operatorname{Cov}(u,v,c)
 =
 \begin{pmatrix}
 2/5&0&0\\
 0&2/5&0\\
 0&0&4/25
 \end{pmatrix}.                                     \tag{4}
\]

The coefficients in equation (4) are forced by the alphabet and the uniform
counting measure. They are not physical electromagnetic or gravitational
couplings.

---

## 2. Minimum reversible bare dynamics

On a periodic finite lattice, stream every directed record one channel hop per
global tick:

\[
 \lambda_{x,d,n+1}=\lambda_{x-d,d,n}.                \tag{5}
\]

Equation (5) is a bijection of the complete finite configuration space. Every
local collision or manifestation gate constructed in the finite C4 program is
also a permutation on its payload-complete state space. Therefore any
composition

\[
 T=\mathcal S\circ\prod_x\mathcal C_x               \tag{6}
\]

of streaming with nonoverlapping reversible local collisions preserves the
uniform counting measure exactly.

For the theorem below, take the **bare vacuum arm**
\(\mathcal C_x=1\). Interacting collision statistics remain open.

Equation (5) is a concrete finite reversible microscopic generator. It is not
yet the whole requested native action because it produces no autonomous
manifestation, binding, or backreaction.

---

## 3. Antipodal common and relative channels

For one antipodal line \([d]\), let the two directed records have phase
coordinates \((u_+,v_+)\) and \((u_-,v_-)\). Define

\[
 u_{\rm com}={u_++u_-\over\sqrt2},\qquad
 u_{\rm rel}={u_+-u_-\over\sqrt2},                  \tag{7}
\]

and identically for \(v\). Independence and equation (4) give

\[
 \operatorname{Var}(u_{\rm com})
 =\operatorname{Var}(u_{\rm rel})
 =\operatorname{Var}(v_{\rm com})
 =\operatorname{Var}(v_{\rm rel})
 ={2\over5},                                        \tag{8}
\]

with every common/relative and quadrature cross-covariance zero.

Thus the finite alphabet does not have to import a real L/R diagonalization:
the common/relative chart is the exact block covariance chart of antipodal
directed channels.

The quadratic large-deviation Hessian for each normalized scalar channel is

\[
 I_{\rm phase}^{(2)}
 ={5\over4}
 \left(u_{\rm com}^2+u_{\rm rel}^2+
       v_{\rm com}^2+v_{\rm rel}^2\right)            \tag{9}
\]

per independent line sample, before field normalization.

---

## 4. Isotropic relative-current response

Use the nine normalized C18 antipodal line representatives. Their exact
second moment is

\[
 \sum_{\ell=1}^{9}d_\ell d_\ell^T=3I_3.             \tag{10}
\]

Define the cell-averaged signed relative phase current

\[
 J={1\over9}\sum_{\ell=1}^{9}
 (u_{\ell,+}-u_{\ell,-})d_\ell.                     \tag{11}
\]

Equations (4) and (10) give the exact isotropic covariance

\[
 \boxed{
 \operatorname{Cov}(J)={4\over135}I_3.}             \tag{12}
\]

The bare quadratic response is therefore

\[
 I_J^{(2)}={135\over8}|J|^2.                         \tag{13}
\]

Equation (12) supplies a finite-alphabet origin for an isotropic vector
channel. It does not identify that channel with electromagnetism, derive
Gauss dynamics, or determine a physical charge normalization.

---

## 5. Capacity tensor and its exact Hessian

Average the two directed capacities on each antipodal line:

\[
 c_\ell={c_{\ell,+}+c_{\ell,-}\over2},\qquad
 \operatorname{Var}(c_\ell)={2\over25}.             \tag{14}
\]

Define

\[
 \mathcal K={1\over9}\sum_{\ell=1}^{9}
 c_\ell\,d_\ell d_\ell^T.                           \tag{15}
\]

Its isotropic mean is

\[
 \mathbb E\mathcal K={1\over15}I_3.                 \tag{16}
\]

In symmetric coordinates
\((xx,yy,zz,xy,xz,yz)\), the exact covariance is

\[
 \Sigma_{\mathcal K}
 ={1\over2025}
 \begin{pmatrix}
 4&1&1&0&0&0\\
 1&4&1&0&0&0\\
 1&1&4&0&0&0\\
 0&0&0&1&0&0\\
 0&0&0&0&1&0\\
 0&0&0&0&0&1
 \end{pmatrix}.                                     \tag{17}
\]

Equation (17) has exact rank six. Let

\[
 t=\operatorname{tr}\delta\mathcal K,\qquad
 e_i=\delta\mathcal K_{ii}-{t\over3},
 \qquad\sum_i e_i=0.                                \tag{18}
\]

The half-Hessian is

\[
 \boxed{
 \begin{aligned}
 I_{\mathcal K}^{(2)}
 ={}&{225\over4}t^2
 +{675\over2}\sum_i e_i^2\\
 &+{2025\over2}
 \left(\delta K_{xy}^2+\delta K_{xz}^2+\delta K_{yz}^2\right).
 \end{aligned}}                                     \tag{19}
\]

All trace and shear directions have positive cost. This is the statistical
counterpart of the exact
[Moore-bond type census](FOUND_MOORE_BOND_CAPACITY_TYPE_CENSUS_v1.md): the
finite directed carrier not only has the
\(A_{1g}\oplus E_g\oplus T_{2g}\) type but gives it a full positive bare
quadratic Hessian.

---

## 6. The gravity boundary becomes sharper

Equation (19) does **not** derive gravity.

First, it is a local entropy/large-deviation cost, not a spacetime kinetic
operator. No pole, propagation speed, gauge redundancy, constraint algebra,
or source law follows from a one-cell covariance.

Second, the two shear irreducible sectors are cubically anisotropic. In the
Frobenius norm, the \(T_{2g}\) shear coefficient is \(3/2\) times the
\(E_g\) coefficient. Thus the bare C18 uniform measure has exact \(O_h\)
symmetry but not accidental \(SO(3)\) shear isotropy.

Third, a positive six-component Hessian does not isolate two transverse
gapless modes. The trace/capacity and divergence constraints needed for a
two-mode tensor sector must arise from interacting conservation laws.

The interacting microscopic action must therefore accomplish three things
without tuning to General Relativity:

1. turn the local capacity cost into a derivative transport action;
2. drive the \(E_g/T_{2g}\) anisotropy toward one infrared coefficient; and
3. derive trace and transversality constraints leaving two positive gapless
   modes.

Failure at any step closes this carrier as a native spin-2/equivalent route.

---

## 7. Blocking statement

For a block of \(N_B\) independent bare cells, the multivariate central limit
theorem and finite-alphabet large-deviation expansion give

\[
 -\log\Pr(\delta\bar X)
 =
 N_B\left[
 {1\over2}\delta\bar X^T\Sigma^{-1}\delta\bar X
 +O(\|\delta\bar X\|^3)
 \right].                                           \tag{20}
\]

Equations (9), (13), and (19) are the exact quadratic coefficient of that
expansion. This is the precise sense in which a real Gaussian field action can
emerge from finite records: the real variables are block-frequency
coordinates, and the quadratic functional is the Hessian of finite
combinatorial multiplicity.

The theorem is conditional on the independent uniform bare-vacuum ensemble.
It does not establish that the interacting deterministic dynamics is ergodic
on that ensemble or that physical preparations realize it.

---

## 8. Consequence for the one-action program

The
[finite-reserve work boundary](../quantum_foundations/THEOREM_C4_CONTROLLED_ACTUALIZATION_AND_CONTINUOUS_WORK_BOUNDARY_v1.md)
required the discrete-first route to derive, rather than assume, a real field
action. Equations (3)--(20) pass that requirement at the first noninteracting
quadratic rung:

\[
 \text{finite C4-plus-blank records}
 \longrightarrow
 \text{uniform reversible counting measure}
 \longrightarrow
 \text{real common/relative vector and capacity Hessians}. \tag{21}
\]

No physical scale or target coupling was used. Field normalization remains a
choice until a source and matter response are derived, so the coefficients
above may not be read as \(\alpha\), \(G_N\), or a graviton normalization.

---

## 9. Next locked gate

Freeze an interacting local collision permutation built only from:

- C4 phase relation;
- signed orientation;
- ternary endpoint charge;
- finite capacity/backpressure; and
- the exact reversible dark/bright/manifestation gates already proved.

Then derive its two-point block kernel before inspecting Maxwell, gravity, or
the master root. A pass requires:

1. a gapless relative vector response with local continuity;
2. a common capacity trace/shear transport response;
3. positive energy and a common causal cone;
4. measured reduction, rather than growth, of cubic shear anisotropy; and
5. no free coefficient selected using \(\alpha\), \(G_N\), lensing, or a
   desired tensor pole.

Only after this gate may the program test stable matter, lensing, tensor
polarizations, or a blind native electromagnetic coupling.

The first subcase is now closed negative. The
[single-record equivariant collision no-go](THEOREM_C18_EQUIVARIANT_SINGLE_RECORD_COLLISION_NO_GO_v1.md)
proves that any fixed cubic-covariant permutation acting on one C18 record is
only identity or antipodal reversal on each direction shell. After streaming,
these give independent ballistic rays or exact two-tick bounces. The next
candidate must therefore be state-dependent on a joint multi-record sector or
carry a dynamical local controller.

The
[two-record phase-complete scattering construction](THEOREM_C18_TWO_RECORD_PHASE_COMPLETE_SCATTERING_AND_AXIAL_ROUTING_BOUNDARY_v1.md)
now supplies the first such joint-state candidate. It is an exact reversible
FCC doubleton collision preserving total directed momentum and complete C4
payload. Twelve grazing sector types have a unique cubic-covariant payload
route; six axial types require equal phases or an additional handed/controller
record. Its interacting block kernel and coupling to manifestation remain the
next gate.
