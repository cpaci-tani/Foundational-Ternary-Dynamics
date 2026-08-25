# Moore-bond capacity type census and unified-action boundary v1

**Date:** 2026-08-23  
**Status:** `[THEOREM — EXACT O_h PERMUTATION AND MOMENT-RANK CENSUS]` +
`[FOUNDATIONAL BOUNDARY — A TENSOR READOUT IS NOT A TENSOR DEGREE OF FREEDOM]` +
`[CONJECTURE — PHASE-RESOLVED BOND CAPACITY AS A COMMON CARRIER]` +
`[OPEN — NONLINEAR COLLECTIVE POLE OR EXPLICIT NEW LINK TYPE]`  
**Production status:** unchanged  
**Ledger status:** no new LEDGER row minted

## 1. Result

One scalar capacity on each antipodal pair of the 26 Moore directions gives a
13-dimensional inversion-even bond space.  Its exact octahedral decomposition
is

\[
 \boxed{V_{\rm even}
 =3A_{1g}\oplus2E_g\oplus2T_{2g}.}                    \tag{1}
\]

For line representatives \([d]=\{d,-d\}\), define the symmetric second
moment

\[
 \mathcal K(w)=\sum_{[d]} w_{[d]}\,d\otimes d.         \tag{2}
\]

The map in equation (2) is exactly rank six and is equivariant under
\(O_h\).  Its image is the complete symmetric-tensor space

\[
 \operatorname{Sym}^2(T_{1u})
 =A_{1g}\oplus E_g\oplus T_{2g},                       \tag{3}
\]

while

\[
 \boxed{\ker\mathcal K
 =2A_{1g}\oplus E_g\oplus T_{2g}}                      \tag{4}
\]

has dimension seven.

Thus independent Moore-bond capacities can represent one scalar trace and all
five local shear components.  This is a **kinematic type statement**.  It does
not establish an independent propagating tensor mode, a gravitational action,
two transverse polarizations, lensing, or General Relativity.

The exact certificate is
[`proof_moore_bond_capacity_type_census.py`](../../../../../scripts/proofs/proof_moore_bond_capacity_type_census.py).
It uses rational ranks and exact character sums; it performs no parameter
search and no comparison with measured constants.

## 2. Exact shell census

Choose one representative of every antipodal line.  The 13 lines decompose as

| Shell | Unoriented lines | Even line module | Rank under \(\mathcal K\) |
|---|---:|---|---:|
| SC | 3 | \(A_{1g}\oplus E_g\) | 3 |
| FCC | 6 | \(A_{1g}\oplus E_g\oplus T_{2g}\) | 6 |
| BCC | 4 | \(A_{1g}\oplus T_{2g}\) | 4 |
| Moore | 13 | \(3A_{1g}\oplus2E_g\oplus2T_{2g}\) | 6 |

The six FCC lines alone give an invertible coordinate system for a symmetric
\(3\times3\) tensor.  Writing their capacities as

\[
 (a,b,c,d,e,f)
 =\bigl(w_{(1,1,0)},w_{(1,-1,0)},w_{(1,0,1)},
 w_{(1,0,-1)},w_{(0,1,1)},w_{(0,1,-1)}\bigr),          \tag{5}
\]

their moment is

\[
 \begin{aligned}
 K_{xx}&=a+b+c+d,&K_{xy}&=a-b,\\
 K_{yy}&=a+b+e+f,&K_{xz}&=c-d,\\
 K_{zz}&=c+d+e+f,&K_{yz}&=e-f.
 \end{aligned}                                        \tag{6}
\]

Equation (6) has a unique inverse over \(\mathbb Q\).  Consequently the
production C18 geometry, which contains SC plus FCC directions, already has
enough **direction types** to parameterize a local symmetric tensor.  BCC
corners are not required for the rank-six kinematic statement.

The inversion-odd partner of the 13-line space is

\[
 V_{\rm odd}=A_{2u}\oplus3T_{1u}\oplus T_{2u}.         \tag{7}
\]

The first-moment map

\[
 \mathcal J(v)=\sum_{[d]}v_{[d]}d                     \tag{8}
\]

has rank three, selects one \(T_{1u}\) vector, and has kernel

\[
 A_{2u}\oplus2T_{1u}\oplus T_{2u}.                   \tag{9}
\]

Equations (2) and (8) make the unification opportunity precise: an oriented
bond state can have an odd vector moment and an even capacity moment.  They do
not prove that the current FTD state supplies such an independently evolving
bond state.

## 3. Collateral representation correction

The same exact character calculation gives the 27-site permutation module

\[
 \boxed{
 4A_{1g}\oplus A_{2u}\oplus2E_g\oplus2T_{2g}
 \oplus3T_{1u}\oplus T_{2u}.}                         \tag{10}
\]

In particular, the cuboctahedral shell is

\[
 A_{1g}\oplus E_g\oplus T_{2g}\oplus T_{1u}\oplus T_{2u}, \tag{11}
\]

not the previously printed expression with \(T_{1g}\).  A direct witness is
a quarter-turn about a coordinate axis: the trace on the six inversion-even
FCC lines is zero.  The character of
\(A_{1g}\oplus E_g\oplus T_{2g}\) on that class is
\(1+0-1=0\), whereas replacing \(T_{2g}\) by \(T_{1g}\) gives
\(1+0+1=2\).

The corrected representation still has four \(A_{1g}\) copies, three
\(T_{1u}\) copies, six triplet copies in total, and a 13+13 inversion-parity
dimension split.  It has **six**, not seven, distinct irrep labels.  Therefore
the earlier identification “number of distinct irreps \(=b_3=7\)” is closed
negative; it was not used in the present capacity result.

## 4. The type boundary

The existence of the image (3) leaves four physically different cases.

### 4.1 Capacity derived from the vector field

If

\[
 w_{[d]}=F(J\cdot d)                                    \tag{12}
\]

for the existing site vector \(J\), then \(\mathcal K\) is a composite
operator.  In the quadratic case it belongs to the already tested
\(J\otimes J\) catalog.  The
[`spin-2 boundary theorem`](FOUND_SPIN2_BOUNDARY_THEOREM.md) and FTD-0193 then
apply: a five-component tensor **readout** exists, but it has no separable
gapless tensor pole in the probed regime.

### 4.2 Capacity derived from ternary occupancy

The existing membrane uses

\[
 m_x=s_x^2,
 \qquad
 g_{xy}=1-(m_x-m_y)^2.                                 \tag{13}
\]

This exactly supplies a static bond gate, as proved in
[`FTD-0990`](../native_time_carrier_programme/THEOREM_TERNARY_OCCUPANCY_MEMBRANE_AND_SELF_DUAL_BODY_CLOCK_SPLIT_v1.md).
It does not supply an infinitesimal vacuum shear: in a uniform occupancy
region every gate is fixed at one, and at a matter--void boundary it changes
discretely.  It is a membrane, not a free radiative geometry.

### 4.3 Capacity as an independent bond state

If the 13 capacities are independent dynamical coordinates, equation (3)
provides a genuine local tensor carrier.  Reversible evolution then requires
their conjugate response variables or an equivalent constrained phase-space
construction.  This is a new link ontology type.  It is not supplied by P1--P5
or by the present `Voxel` fields and must be adopted and priced explicitly.

The retirement of an independent **static membrane bit** in FTD-0990 does not
retire this price.  A dynamic phase-complete bond coordinate is not redundant
with the endpoint truth table (13).

### 4.4 Capacity as a nonlinear collective mode of existing L/R fields

This is the only currently open route that adds no primitive storage type.
FTD-0942 proves that the linear L/R fields are an aggregate reversible carrier
but do not supply collision-separated direction channels.  A nonlinear
protected pulse, condensate, or characteristic sector could still make a
bond-capacity tensor dynamically independent in the infrared.

The acceptance condition is not “a tensor can be calculated.”  It is a
separable pole or classical normal mode in the connected shear response that
is absent from the spin-1 control.

### 4.5 Linear-slaving no-go at the isotropic vacuum

There is a representation-theoretic obstruction stronger than the particular
quadratic ansatz \(J\otimes J\).

At a cubic-invariant vacuum, the differential of any differentiable
\(O_h\)-equivariant constitutive map must itself be an intertwiner.  The
existing continuous local field coordinates and their canonical velocities
are copies of the polar-vector type \(T_{1u}\); a spatial scalar is of type
\(A_{1g}\).  The capacity output is

\[
 A_{1g}\oplus E_g\oplus T_{2g}.                       \tag{14}
\]

Schur decomposition gives

\[
 \operatorname{Hom}_{O_h}
 \!\left(T_{1u},A_{1g}\oplus E_g\oplus T_{2g}\right)=0. \tag{15}
\]

Consequently a scalar perturbation can change only the trace, while no vector
perturbation can generate a linear \(E_g\) or \(T_{2g}\) shear.  For the
charge-even occupancy \(m=s^2\), even the scalar differential vanishes at
\(s=0\).

Therefore a capacity tensor algebraically slaved to any finite collection of
the present scalar/vector coordinates adds no linear vacuum shear mode.  Its
shear begins at nonlinear order and remains a composite observable unless the
interacting dynamics develops a separable collective pole.  A constitutive
formula \(\mathcal K=\mathcal K(s,J_L,J_R,\ldots)\) cannot by itself evade the
spin-2 boundary.

This no-go assumes a differentiable local constitutive map at a cubic-invariant
vacuum.  It does not exclude a nonperturbative bound-state pole, a phase with
an additional isotropic order parameter, temporal-memory dynamics that is not
algebraically slaved, or an explicitly adopted link coordinate.

## 5. Consequence for one native action

`[CONJECTURE]` A phase-resolved oriented bond state is still a viable common
carrier, but the exact census makes its price explicit.  Schematically write

\[
 z_{[d]}=\sqrt{w_{[d]}}\,i^{a_{[d]}},
 \qquad a_{[d]}\in\mathbb Z/4\mathbb Z,                \tag{16}
\]

or realize the same information through a finite token population.  Its
moments can then be assigned the following candidate roles:

| Observable | Candidate role | Present status |
|---|---|---|
| ternary endpoint \(s\) | actual manifestation | native record; autonomous reciprocal formation open |
| odd first moment \(\mathcal J\) | charge/current and electromagnetic response | kinematically available; native coupling measurement open |
| even trace of \(\mathcal K\) | local clock/transport capacity | candidate bridge to the latency well |
| even shear of \(\mathcal K\) | lensing and radiative geometry | no native dynamics or two-mode reduction |
| bond phase/winding \(a\) | recurrence and history interference | conditional C4 hardware exists; physical Born pushforward open |
| stable localized recurrence | matter and its internal clock | conditional membrane/clock mode exists; formation and mass open |

One action would have to make these roles different observables of the same
transaction, not a sum of independently selected phenomenological sectors.
At minimum its wave principal part must contain the same capacity tensor that
governs body clocks,

\[
 L_{\rm principal}
 ={1\over2}|D_t\phi|^2
 -{1\over2}\mathcal K^{ij}(w)D_i\phi\,D_j\phi,         \tag{17}
\]

with reciprocal dynamics for \(w\).  If \(\mathcal K\) changes clock rates
but does not enter equation (15), the FTD-1020 class-0 lensing result remains.

Equation (17) is a requirement template, not an adopted Lagrangian.

## 6. Decisive gates

The bond-capacity route survives only if all of the following are met.

1. **Type honesty.** Derive \(w\) from existing L/R fields, or explicitly
   adopt it as a new phase-complete link type.  Do not alternate between those
   readings.
2. **Non-tautological action.** Discrete variation must generate the transfer,
   manifestation, reserve, and reciprocal-reaction laws; the action cannot
   merely be \(-\log\) of an already specified tick kernel.
3. **Matter and clock.** The empty state must form a finite-energy localized
   recurrence from a target-blind reserve transaction, and its clock must be
   that recurrence's phase rather than an imposed onsite frequency.
4. **Lensing.** The same sourced capacity background must alter matter and
   wave characteristics and produce nonzero deflection and Shapiro delay.
5. **Tensor/equivalent gravity.** Linearized shear response must contain
   exactly two physical gapless transverse modes after constraints, with
   positive energy and a cone compatible with light.  The five-dimensional
   \(E_g\oplus T_{2g}\) local type is not yet this result.
6. **Born pushforward.** Apparatus context may change physical boundary
   couplings, but no outcome or target probability may enter the law.  Basin
   measure under the deterministic selector must independently equal the
   squared coherent history weight.
7. **Native \(\alpha\).** The long-range response between formed unit charges
   must determine \(g_{\rm eff}\); the same action must determine
   \(\hbar_{\rm eff}\) and \(c_{\rm eff}\).  Only then may
   \(g_{\rm eff}^2/(4\pi\hbar_{\rm eff}c_{\rm eff})\) be compared with the
   master-quadratic root.

## 7. Verdict and next executable question

The Moore neighborhood contains the correct **representation capacity** for
a unified vector-plus-tensor carrier.  Present FTD does not yet contain the
corresponding independent dynamics.

The next exact question is therefore:

> Does any Moore-local, cubic-covariant, target-blind nonlinear functional of
> the existing L/R canonical pairs and ternary occupancy have a linearized
> bond-capacity shear mode separable from the vector spectrum, while retaining
> exact reciprocal energy and manifestation work?

A negative theorem for a clearly registered functional class would price the
new link type.  A positive construction would be the first ontology-preserving
step toward the requested single native action.
