# C18 two-record linearized kernel and tensor boundary v1

**Date:** 2026-08-23  
**Status:** **[THEOREM — EXACT PRODUCT-REFERENCE COLLISION JACOBIAN AND SPECTRUM]** +
**[THEOREM — EXACT CONSERVED-MODE AND CUBIC-SHEAR DECOMPOSITION]** +
**[CLOSED NEGATIVE, SCOPED — SELECTED TWO-RECORD COLLISION AS A GAPLESS PHASE-VECTOR OR TENSOR CARRIER]** +
**[BOUNDARY — SC SPECTATORS, RARE UNIFORM INTERACTION, NO PHYSICAL EM/CLOCK/BORN IDENTIFICATION]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_c18_two_record_linearized_collision_kernel.py](../../../../../scripts/proofs/proof_c18_two_record_linearized_collision_kernel.py)
constructs the exact integer tangent matrix, verifies a square-free
annihilating polynomial and seven trace moments, proves the complete
eigenvalue multiplicities, checks all conserved rows, and evaluates the cubic
capacity-shear modes. It performs no fit and uses no numerical eigensolver.

---

## 1. Registered local collision

Use the selected reference involution $\mathcal C_2$ from the
[two-record scattering theorem](THEOREM_C18_TWO_RECORD_PHASE_COMPLETE_SCATTERING_AND_AXIAL_ROUTING_BOUNDARY_v1.md).
It acts nontrivially only when exactly two local channels are occupied and the
phase-complete FCC pair lies in an eligible momentum doubleton. All other
local states fail closed to identity.

For each directed channel, take the same five-state alphabet as the bare
blocking theorem:

\[
 a\in\{0,1,i,-1,-i\}.                               \tag{1}
\]

The target-blind reference measure is the independent product distribution

\[
 p_{d,a}^{*}={1\over5}.                             \tag{2}
\]

This is the precise ensemble registered here. The result below is not a claim
about every correlated invariant measure or nonlinear coherent orbit.

---

## 2. Exact marginal map and tangent chart

Let $F_{d,k}(p)$ be the post-collision marginal probability for occupied C4
phase $k$ on FCC direction $d$. Use the normalized tangent coordinates

\[
 p_{d,0}=1-\sum_{k=0}^{3}p_{d,k}.                   \tag{3}
\]

There are $12\times4=48$ independent FCC tangent coordinates. Because the
collision differs from identity on only the 432 registered phase-complete
two-record configurations,

\[
 F_i(p)=p_i+
 \sum_{z\in M}\Delta_i(z)\prod_{d\in D_{18}}p_{d,z_d},
 \qquad |M|=432,                                    \tag{4}
\]

where $\Delta_i(z)$ is the output-minus-input occupied-phase indicator.

Differentiating equation (4) at equation (2) gives exactly

\[
 \boxed{DF(p^*)=I_{48}+{1\over5^{17}}N,}             \tag{5}
\]

with $N$ an integer symmetric matrix. The factor $5^{-17}$ is a consequence
of differentiating one factor from an eighteen-channel product state; it is
not an adjustable collision strength.

---

## 3. Complete exact spectrum

The correction matrix has spectrum

| Eigenvalue of $N$ | Multiplicity |
|---:|---:|
| $-180$ | 3 |
| $-160$ | 3 |
| $-30$ | 8 |
| $-28$ | 9 |
| $-16$ | 9 |
| $-8$ | 9 |
| $0$ | 7 |

The certificate proves this without floating-point diagonalization. It checks

\[
 \prod_{\lambda\in\{-180,-160,-30,-28,-16,-8,0\}}
 (N-\lambda I)=0                                   \tag{6}
\]

exactly and verifies the first seven trace moments against the displayed
multiplicities. The roots in equation (6) are distinct, so the annihilator is
square-free and $N$ is diagonalizable; the Vandermonde system fixes the seven
multiplicities uniquely.

Thus

\[
 \operatorname{rank}N=41,\qquad \operatorname{nullity}N=7. \tag{7}
\]

Every eigenvalue of equation (5) lies in $(0,1]$. The selected mean-field
collision is linearly stable and relaxational on every nonconserved FCC mode.

---

## 4. The seven and only seven FCC collision invariants

Seven explicit left null vectors are:

1. the total number of records carrying each of the four C4 phases; and
2. the three components of total directed momentum, summed over phases.

They are linearly independent. Equation (7) proves that they span the entire
FCC tangent nullspace. In cubic representation language, the protected modes
are four scalar counts plus one three-component polar vector.

There is no protected FCC tensor mode. The collision also leaves every SC
channel untouched, adding 24 normalized SC tangent spectators in the full
C18 marginal chart. Therefore this minimum collision does not yet unify the
SC and FCC carriers.

---

## 5. Exact capacity-shear result

Sum over all four phases and form the FCC quadratic direction moments. A basis
for the diagonal traceless sector is

\[
 E_1(d)=d_x^2-d_y^2,
 \qquad
 E_2(d)=2d_z^2-d_x^2-d_y^2,                         \tag{8}
\]

and a basis for the off-diagonal shear sector is

\[
 T_{xy}(d)=d_xd_y,\quad
 T_{xz}(d)=d_xd_z,\quad
 T_{yz}(d)=d_yd_z.                                  \tag{9}
\]

The exact collision action is

\[
 NE_a=-30E_a,\qquad a=1,2,                          \tag{10}
\]

\[
 NT_{ab}=-160T_{ab},\qquad ab=xy,xz,yz.             \tag{11}
\]

Consequently the one-tick marginal factors at zero wave number are

\[
 \mu_{E_g}=1-{30\over5^{17}},
 \qquad
 \mu_{T_{2g}}=1-{160\over5^{17}}.                  \tag{12}
\]

Both shear types relax and their rates are unequal. The interaction therefore
retains, rather than removes, a sharp cubic split.

---

## 6. Scoped gravity conclusion

A native massless spin-2 or equivalent carrier needs two protected transverse
tensor modes after constraints. In the registered product-reference
linearization, the only unit collision eigenmodes are scalar phase counts and
the momentum vector. Equations (10)--(12) place both candidate capacity-shear
types strictly outside the nullspace.

Therefore:

\[
 \boxed{\mathcal C_2\text{ alone is closed negative as the native gapless
 tensor carrier in this reference linearization.}}  \tag{13}
\]

This does not close every nonlinear or correlated C18 route. It proves that a
gravity completion must add a conservation/constraint mechanism that protects
a tensor sector, or use an equivalent non-shear object whose blocked response
produces lensing and two physical modes. Merely iterating this collision does
not do so.

---

## 7. Electromagnetic, clock, and actualization boundary

The four phase counts are protected, but the collision transports each phase
payload unchanged. It contains no local phase conversion, charge exchange,
C4 clock advance, reserve debit, dark cancellation, or manifestation gate.

This boundary is now exact. For spatial vector moment $d_i$ and the four real
C4 phase-character charts

\[
 w_{\rm blind}=(1,1,1,1),\quad
 w_R=(1,0,-1,0),\quad
 w_I=(0,1,0,-1),\quad
 w_A=(1,-1,1,-1),                                  \tag{14}
\]

the correction matrix obeys

\[
 N(d_iw_{\rm blind})=0,                             \tag{15}
\]

but

\[
 \boxed{
 N(d_iw_R)=N(d_iw_I)=N(d_iw_A)=-8(d_iw),}           \tag{16}
\]

for all three spatial components. Thus the phase-blind momentum vector is
protected, while all nine nontrivial phase-weighted vector modes relax by the
one-tick factor

\[
 \mu_{\rm phase\ vector}=1-{8\over5^{17}}<1.        \tag{17}
\]

The selected collision is therefore also scoped closed-negative by itself as
a gapless electromagnetic phase-vector carrier in this linearization. A
larger transaction must protect a relative-vector current through a derived
continuity/gauge structure rather than tune equation (17) toward unity.

Accordingly, the exact spectrum does not yet establish Maxwell propagation or
an operational charge response. Without that response there is no native
observable from which to read

\[
 \alpha_{\rm native}={g_{\rm eff}^2\over
 4\pi\hbar_{\rm eff}c_{\rm eff}}.                  \tag{18}
\]

The collision likewise does not generate detector routing or a physical Born
pushforward. Those finite circuits exist separately, but their shared state
ownership and dynamical activation have not been derived from $\mathcal C_2$.

---

## 8. Uniform-reference sparsity boundary

Under equation (2), the exact weight of configurations changed by the
collision is

\[
 \Pr(M)={432\over5^{18}}.                           \tag{19}
\]

Thus the exact-two-occupancy collision is extremely sparse in the same dense
uniform product ensemble that generated the bare Hessian. This is not a free
small coupling; it is a structural mismatch between a minimal binary gate and
the registered independent five-state reference measure.

A viable next action must either:

- extend the collision consistently to higher occupancies;
- derive a sparse correlated invariant ensemble in which binary encounters
  have finite blocked weight; or
- derive a different joint carrier whose interaction is not conditioned on
  sixteen simultaneous blanks.

Choosing an occupancy density to match a desired physical coupling is
forbidden.

---

## 9. Next locked gate

The selected collision is useful as a reversible scattering primitive, but it
cannot be the whole action. The next construction must add one
payload-complete local transaction that:

1. couples phase relation to capacity and ternary manifestation while
   preserving an exact inverse and work ledger;
2. removes the 24 SC spectator modes through a derived shell-exchange or
   shared-capacity rule;
3. protects the required vector/tensor modes rather than tuning their
   relaxation rates; and
4. generates the preparation, routing, work, and reset used by the physical
   Born pushforward.

Only its derived response kernel may then be tested for Maxwell propagation,
lensing, material clocks, and a blind native-alpha measurement.

The first kinematic repair is now exact in the
[common-phase tensor-doublet theorem](THEOREM_C18_COMMON_PHASE_TENSOR_DOUBLET_AND_CONSTRAINT_PRICE_v1.md).
Blocking the two common C4 quadratures separately yields a rank-twelve pair of
symmetric tensors with native quarter-turn complex structure. This pays the
candidate coordinate/momentum type price without a new continuum primitive,
but the native bracket, four required constraints, tensor pole, sourcing, and
lensing remain open.
