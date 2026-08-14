# Theorem — Local Flux Self-Pair Tensor Recursion v1

**Identifier:** `FTD-0841`  
**Date:** 2026-08-10  
**Status:** `[THEOREM — LOCAL VECTOR SELF-PAIR CONSTRUCTION]` +
`[THEOREM — EXACT CONDITIONAL VECTOR RECURSION]` +
`[THEOREM — POLARIZED REDUCTION TO CONTINUUM G* CLOCK]` +
`[BOUNDARY — CUBIC SYMMETRY DOES NOT SELECT THE RADIAL QUARTIC]` +
`[SELECTION/OPEN — PRODUCTION COUPLING, POLARIZATION, SUPPORT, AND CADENCE]`  
**Certificate:**
[`proof_local_flux_self_pair_tensor.py`](../../../../../scripts/proofs/proof_local_flux_self_pair_tensor.py)
(`26/26` exact checks)  
**Pre-registration:**
[`PREREG_LOCAL_FLUX_SELF_PAIR_TENSOR_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_LOCAL_FLUX_SELF_PAIR_TENSOR_v1.md)

## 1. Result

FTD-0840's scalar pair-energy mechanism has an axis-free, voxel-local vector
extension.

Every production voxel already contains

\[
J_x=\texttt{flux},
\qquad
W_x=\texttt{wave\_vel},
\]

and the canonical field Lagrangian identifies `wave_vel` as the conjugate
momentum of flux. Form the local rank-one self-pair tensor

\[
\boxed{U_x=J_x\otimes J_x}.                    \tag{1}
\]

Its induced Frobenius energy obeys

\[
\boxed{\|U_x\|_F^2=|J_x|^4}.                  \tag{2}
\]

Thus the selected Hamiltonian

\[
\boxed{
H_x(J,W)=\frac{|W|^2}{2m}
+\lambda\|J\otimes J\|_F^2
=\frac{|W|^2}{2m}+\lambda|J|^4}               \tag{3}
\]

is a natural tensor version of “ordinary quadratic energy on a self-pair.” It
does not choose a Cartesian component and retains the unsquared vector `J`, so
the square's sign/sheet loss is not fatal.

An exact vector discrete-gradient update exists for (3). It has one global
next state for every finite input and nonzero step, conserves energy and
angular momentum exactly, is time-reversible, has a strict swept-area
orientation on every nonzero step, and remains on a compact energy shell.

This closes the **local mathematical phase-space type and recursion**. It does
not promote the coupling to production. The frozen engine contains `J` and
`W`, but not `lambda|J|^4` or its cubic force.

## 2. Why the tensor is the natural local square

For `J=(J_1,J_2,J_3)`, define

\[
U_{ab}=J_aJ_b.
\]

Then

\[
\begin{aligned}
\|U\|_F^2
&=\sum_{a,b}U_{ab}^2\\
&=\sum_{a,b}J_a^2J_b^2\\
&=\left(\sum_aJ_a^2\right)^2\\
&=|J|^4.                                        \tag{4}
\end{aligned}
\]

The construction uses only the Euclidean inner product already exposed by
`Vec3::mag2()`. It is rank one whenever `J != 0`, is positive, and is invariant
under every orthogonal change of spatial frame.

The tensor alone is two-to-one under `J -> -J`. The physical state is not
`U` alone; it retains `(J,W)`. This is the vector realization of the FTD-0839
rule that a squared energy carrier must be accompanied by its unsquared lift
or an equivalent orientation witness.

## 3. Continuous dynamics

Hamilton's equations are

\[
\dot J=\frac Wm,
\qquad
\dot W=-4\lambda|J|^2J.                        \tag{5}
\]

The force is radial. Consequently the local angular momentum

\[
L=J\times W                                     \tag{6}
\]

is conserved:

\[
\dot L
=\frac Wm\times W
+J\times(-4\lambda|J|^2J)=0.                  \tag{7}
\]

This supplies a stable recursive system with more structure than the scalar
clock: the magnitude and direction of `L` label invariant sectors.

## 4. Exact scalar-clock sector

Choose any fixed unit vector `e` and initial data

\[
J=qe,
\qquad
W=pe.                                           \tag{8}
\]

Equation (5) preserves this line. On it,

\[
H=\frac{p^2}{2m}+\lambda q^4,                  \tag{9}
\]

exactly the FTD-0840 Hamiltonian. Therefore the continuum period is

\[
\boxed{
TA=\sqrt\pi G^*\sqrt{\frac{m}{2\lambda}}.}     \tag{10}
\]

No spatial axis is preferred: every unit `e` defines an equivalent invariant
sector in the selected radial model.

The condition for this pure scalar reduction is

\[
L=J\times W=0.                                  \tag{11}
\]

Because `L` is conserved, the condition persists. It is not dynamically
attractive. Generic nonzero-`L` motion has the radial effective Hamiltonian

\[
H_{\rm radial}
=\frac{p_r^2}{2m}+\frac{|L|^2}{2mr^2}+\lambda r^4,  \tag{12}
\]

so its period is not the pure-quartic `G*` law. A native `G*` clock therefore
still needs a formation or selection mechanism for the zero-angular-momentum
polarized sector.

## 5. What cubic symmetry does and does not select

For a cubic vector, the general homogeneous even quartic invariant under
coordinate sign flips and permutations is

\[
V_4=aI_1+bI_2,                                  \tag{13}
\]

with

\[
I_1=J_x^4+J_y^4+J_z^4,
\qquad
I_2=J_x^2J_y^2+J_y^2J_z^2+J_z^2J_x^2.          \tag{14}
\]

Both survive the full octahedral lattice symmetry. The cubic substrate
therefore permits two independent quartic coefficients.

A 45-degree rotation in the `xy` plane compares `(1,0,0)` with
`(1/sqrt(2),1/sqrt(2),0)` and requires

\[
a=\frac a2+\frac b4
\quad\Longleftrightarrow\quad
\boxed{b=2a}.                                   \tag{15}
\]

Only at this ratio

\[
V_4=a(I_1+2I_2)=a|J|^4.                        \tag{16}
\]

Thus the self-pair/Frobenius construction supplies a clean reason to choose
the radial ratio, and full `O(3)` invariance forces it. But P1--P5's finite
cubic symmetry alone does not. The radial tensor norm is a selected local
interaction type, not a theorem forced by the lattice point group.

## 6. Exact vector discrete recursion

For a signed nonzero step `h`, define

\[
\frac{J_1-J_0}{h}=\frac{W_1+W_0}{2m},          \tag{17}
\]

\[
\frac{W_1-W_0}{h}
=-\lambda(|J_1|^2+|J_0|^2)(J_1+J_0).           \tag{18}
\]

### 6.1 Exact discrete gradient and energy closure

The force is the exact secant because

\[
\begin{aligned}
&(J_1-J_0)\cdot
\lambda(|J_1|^2+|J_0|^2)(J_1+J_0)\\
&\qquad
=\lambda(|J_1|^2+|J_0|^2)(|J_1|^2-|J_0|^2)\\
&\qquad
=\lambda(|J_1|^4-|J_0|^4).                    \tag{19}
\end{aligned}
\]

It approaches `4 lambda |J|^2 J` on the diagonal. Combining (17)--(19)
gives

\[
H_1-H_0=0.                                      \tag{20}
\]

### 6.2 Global determinism

Eliminating `W_1` gives

\[
F(X)=\frac{2m}{h}(X-J_0)-2W_0
+h\lambda(|X|^2+|J_0|^2)(X+J_0)=0.             \tag{21}
\]

After multiplication by `sign(h)`, its linear part contributes
`(2m/|h|)|v|^2` to every directional derivative. For the nonlinear part,
write

\[
A=v\cdot X,qquad B=v\cdot J_0,qquad V^2=|v|^2.
\]

Then

\[
\begin{aligned}
v\cdot Dg_Xv
={}&2A^2+(A+B)^2\\
&+(|X|^2V^2-A^2)
+(|J_0|^2V^2-B^2).                              \tag{22}
\end{aligned}
\]

Every term is nonnegative by Cauchy--Schwarz. Hence the full map is strongly
monotone. Its leading radial term is coercive. The standard
finite-dimensional monotone-map theorem gives existence and uniqueness of
the next state. No contextual selector or branch bit is consumed by the
implicit solve.

### 6.3 Reversibility

Endpoint exchange with `h -> -h` changes both residual equations by an
overall minus sign. Physical momentum reversal likewise maps a forward step
to its inverse. Thus

\[
\Phi_{-h}=\Phi_h^{-1},
\qquad
R\Phi_hR=\Phi_h^{-1},
\quad R(J,W)=(J,-W).                            \tag{23}
\]

### 6.4 Exact angular momentum

With midpoint variables `Jbar` and `Wbar`,

\[
\begin{aligned}
J_1\times W_1-J_0\times W_0
={}&Jbar\times(W_1-W_0)\\
&+(J_1-J_0)\times Wbar=0,                       \tag{24}
\end{aligned}
\]

because the discrete force is parallel to `Jbar` and the discrete velocity
is parallel to `Wbar`.

### 6.5 Strict orientation

The midpoint swept-area scalar is

\[
\begin{aligned}
\chi_h
={}&Jbar\cdot(W_1-W_0)-Wbar\cdot(J_1-J_0)\\
=-h\bigg[&\frac\lambda2(|J_1|^2+|J_0|^2)|J_1+J_0|^2
+\frac{|W_1+W_0|^2}{4m}\bigg].                 \tag{25}
\end{aligned}
\]

For `h>0`, it is strictly negative on every nonzero step. Equality would make
both endpoints antipodal; equations (17)--(18) then force the origin.

### 6.6 Bounded recurrence

For conserved energy `E>0`,

\[
|J_n|\leq(E/\lambda)^{1/4},
\qquad
|W_n|\leq\sqrt{2mE}.                            \tag{26}
\]

The local six-dimensional energy shell is compact. As in FTD-0840, this
proves bounded recurrence and stability of energy sublevels, not pointwise
Lyapunov stability of every phase point or attraction to a selected orbit.

## 7. Exact scope of “local”

FTD-0841 closes locality only at the **field-type and onsite-interaction**
level:

- `J_x` and `W_x` are stored at one voxel;
- `U_x`, `V_x`, and the cubic force require no distant data; and
- an explicit cubic kick would respect the existing one-Moore-shell causal
  read dependency.

It does not yet construct a spatially localized clock body. Production's
gradient energy couples neighboring voxels. An exact update for the combined
gradient plus onsite quartic energy would either require a simultaneous
implicit lattice solve or a new sequence of local energy transactions.
Installing the onsite map independently and operator-splitting it with the
wave step would preserve locality and reversibility in suitable schemes, but
would not automatically conserve the combined Hamiltonian exactly.

That coupled local-field energy closure is the next dynamical obligation.

## 8. Revised interface debt

| Interface | FTD-0841 result | Remaining debt |
|---|---|---|
| local canonical type | `Voxel::flux` plus conjugate `wave_vel` | variables are spatially coupled, not an independent clock body |
| self-pair map | `U=J tensor J` axis-free and local | adoption as physical interaction |
| quartic energy | induced Frobenius norm gives `lambda|J|^4` | absent from production; `lambda` and radial ratio not forced by cubic symmetry |
| recursion | unique, reversible, energy/angular-momentum closed onsite map | combined edge-gradient/onsite local transaction |
| scalar clock sector | every `J cross W=0` polarization reduces to FTD-0840 | autonomous formation, persistence, and readout |
| maintenance | conservative shell needs no bath | damping/recovery/selected-shell control still needs work ledger |
| cadence | continuum polarized period has `G*` | finite-tick gate and preferred-order hiding open |

## 9. Bell/Born firewall

The self-pair tensor is deterministic local field mechanics. It contains no
measurement context, equilibrium ensemble, probability weight, effect,
selector, or outcome. Its local phase could later gate an actualization
batch, but cannot determine Born frequencies without the separate
non-target-coded pushforward programme.

## 10. Epistemic disposition

| Claim | Status |
|---|---|
| voxel stores local flux and conjugate wave register | `[PRODUCTION FACT]` |
| `||J tensor J||_F^2=|J|^4` | `[THEOREM]` |
| selected pair energy gives radial cubic force | `[THEOREM — CONDITIONAL COUPLING]` |
| vector discrete next state exists uniquely | `[THEOREM]` |
| energy and angular momentum are conserved exactly | `[THEOREM]` |
| every nonzero step has a signed orientation | `[THEOREM]` |
| linearly polarized sector has the scalar `G*` period | `[THEOREM — CONDITIONAL SECTOR]` |
| generic vector sector has the pure scalar `G*` period | `[REJECTED]` |
| cubic symmetry uniquely forces `|J|^4` | `[REJECTED BY EXACT CONTROL]` |
| production already contains `lambda|J|^4` | `[REJECTED BY SOURCE AUDIT]` |
| localized maintained clock body now exists | `[OPEN]` |
| exact finite-tick `G*` gate now exists | `[OPEN]` |

## 11. Certificate outcome

The first locked execution returned:

```text
FTD-0841 local flux self-pair tensor: 26/26 PASS
VOXEL_FLUX_AND_WAVE_VELOCITY_SUPPLY_LOCAL_CANONICAL_TYPE
SELF_PAIR_TENSOR_FROBENIUS_ENERGY_GIVES_AXIS_FREE_QUARTIC
VECTOR_RECURSION_UNIQUE_REVERSIBLE_ENERGY_AND_ANGULAR_MOMENTUM_CLOSED
POLARIZED_CONTINUUM_SECTOR_HAS_GSTAR_PERIOD
PRODUCTION_COUPLING_ISOTROPY_POLARIZATION_SUPPORT_AND_TICK_CADENCE_OPEN
```

Registered Outcome B passes. No production code was changed.
