# FTD-0842 — Coupled self-pair field-energy closure and local-clock obstruction

**Status:** `[THEOREM — EXACT CONDITIONAL COUPLED DISCRETE GRADIENT]` +
`[THEOREM — GLOBAL ALGEBRAIC DEPENDENCE OF THE EXACT SOLVE]` +
`[THEOREM — POSITIVE-EDGE CRITICAL-LOCALIZATION OBSTRUCTION]` +
`[BOUNDARY — PRODUCTION MAP AND NATIVE LOCAL CLOCK REMAIN OPEN]`  
**Date:** 2026-08-10  
**Programme row:** `FTD-0842`  
**Protocol:**
[`PREREG_COUPLED_SELF_PAIR_FIELD_ENERGY_CLOSURE_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_COUPLED_SELF_PAIR_FIELD_ENERGY_CLOSURE_v1.md),
pre-run SHA-256
`16B96F59DB44F77B30A71417D740355C35D70BEA48E76B9108AE0B08062E91E4`  
**Certificate:**
[`proof_coupled_self_pair_field_energy_closure.py`](../../../../../scripts/proofs/proof_coupled_self_pair_field_energy_closure.py),
SHA-256
`A963EDBA1B9F698EB66C3E6AD1A3A296DE0E03DA18E824BD3D6C156542C7EB8A`,
`26/26 PASS`  
**Production impact:** none

## 0. Result

The FTD-0841 onsite self-pair energy can be combined exactly with the
production 18-point spatial-gradient energy. The symmetric simultaneous
discrete-gradient map is globally unique, reversible, exactly conserves the
combined energy, and preserves the global internal-vector angular momentum.

That mathematical closure does **not** produce the required physical clock:

1. the exact next state is the solution of a globally coupled implicit system;
   already in the linear control, its inverse is dense on a connected finite
   quotient, so it is not a one-Moore-shell ontic update;
2. the positive spatial edge energy vanishes only on the spatially constant
   mode; every nonzero bounded profile has a positive quadratic stiffness and
   therefore is not an exact critical-quartic oscillator; and
3. at zero onsite coupling the selected map is implicit midpoint, not the
   production kick--drift tick or its exact cross-term tick invariant.

The simplest positive architecture

```text
one flux field + positive edge energy + positive radial onsite quartic
```

therefore closes neither strict local causality nor a bounded exact `G*`
clock. Additional dynamical structure is required.

## 1. Frozen production field sector

On a finite connected periodic computational quotient, collect the local
fluxes and conjugate wave registers into

\[
Q=(J_i)_{i\in V},\qquad P=(W_i)_{i\in V}.
\]

FTD-0574 proves that `wave_vel` is the discrete Legendre momentum of flux in
the source-free production field sector. Let

\[
K=-C_{\rm WAVE}^2L_{18}.
\]

The production 18-point Laplacian is symmetric, so `K` is positive
semidefinite. With positive face/edge weights,

\[
\frac12\langle Q,KQ\rangle
=\frac{C_{\rm WAVE}^2}{2}
 \sum_{\{i,j\}\in E}w_{ij}|J_i-J_j|^2.          \tag{1}
\]

FTD-0841 supplies the exact local tensor identity

\[
U_i=J_i\otimes J_i,
\qquad
\|U_i\|_F^2=|J_i|^4.                            \tag{2}
\]

Adopt, explicitly and conditionally,

\[
\boxed{
H(Q,P)=\frac1{2m}\sum_i|P_i|^2
+\frac12\langle Q,KQ\rangle
+\lambda\sum_i|Q_i|^4,
\qquad m,\lambda>0.}                            \tag{3}
\]

The fields and edge term are source-native. The radial onsite coupling and
its coefficient remain `[SELECTION]`.

## 2. Exact coupled discrete gradient

For one signed step `h!=0`, define

\[
\frac{Q_1-Q_0}{h}=\frac{P_1+P_0}{2m},           \tag{4}
\]

\[
\frac{P_1-P_0}{h}
=-\frac12K(Q_1+Q_0)-\lambda G(Q_0,Q_1),         \tag{5}
\]

where

\[
G_i(Q_0,Q_1)
=(|Q_{1i}|^2+|Q_{0i}|^2)(Q_{1i}+Q_{0i}).        \tag{6}
\]

Because `K` is symmetric,

\[
(Q_1-Q_0)\cdot\frac12K(Q_1+Q_0)
=\frac12\langle Q_1,KQ_1\rangle
-\frac12\langle Q_0,KQ_0\rangle.               \tag{7}
\]

Sitewise,

\[
(Q_{1i}-Q_{0i})\cdot G_i
=|Q_{1i}|^4-|Q_{0i}|^4.                         \tag{8}
\]

The kinetic secant is

\[
\Delta H_{\rm kin}
=\frac{P_1+P_0}{2m}\cdot(P_1-P_0).              \tag{9}
\]

Insert (4)--(8) into (9). The kinetic difference cancels the two potential
differences exactly:

\[
\boxed{H(Q_1,P_1)=H(Q_0,P_0).}                  \tag{10}
\]

No floating tolerance, controller, bath, or target period enters (10).

Endpoint exchange together with `h -> -h` leaves (4)--(6) unchanged. The map
is therefore self-adjoint. Combined with `P -> -P`, this gives the usual
physical time reversal.

On the diagonal `Q_1=Q_0=Q`, (5) tends to

\[
\dot P=-KQ-4\lambda(|J_i|^2J_i)_i,              \tag{11}
\]

the Hamiltonian force of (3).

## 3. One global next state

Eliminate `P_1` using (4). The next coordinate `X=Q_1` solves

\[
F(X)=2m(X-Q_0)-2hP_0
+\frac{h^2}{2}K(X+Q_0)
+h^2\lambda G(Q_0,X)=0.                         \tag{12}
\]

For a direction `v`, the linear part contributes

\[
2m|v|^2+\frac{h^2}{2}\langle v,Kv\rangle
\ge 2m|v|^2.                                    \tag{13}
\]

At each site the derivative of the nonlinear secant has the FTD-0841
decomposition

\[
2A^2+(A+B)^2
+(|X|^2|v|^2-A^2)+(|Q_0|^2|v|^2-B^2)\ge0,      \tag{14}
\]

where `A=v dot X` and `B=v dot Q_0`. Hence `F` is strongly monotone. Its
radial quartic leading term is coercive. The finite-dimensional
strong-monotonicity/coercivity theorem gives exactly one solution for every
input state and nonzero signed step.

This proves deterministic mathematical solvability. It does not prove local
ontic computability in one tick; Section 6 separates those notions.

## 4. Exact global internal angular momentum

Define

\[
L_{\rm int}=\sum_iJ_i\times W_i.                \tag{15}
\]

Let `Qbar=(Q_1+Q_0)/2` and `Pbar=(P_1+P_0)/2`. The increment of (15) is

\[
\Delta L_{\rm int}
=\sum_i\left[
 \bar J_i\times(P_{1i}-P_{0i})
 +(J_{1i}-J_{0i})\times\bar P_i
\right].                                       \tag{16}
\]

The second term vanishes by (4). The onsite force in (5) is parallel to
`J_1+J_0` and has zero torque. For every spatial edge, symmetry gives

\[
J_i\times w_{ij}J_j+J_j\times w_{ji}J_i=0.      \tag{17}
\]

Therefore

\[
\boxed{L_{{\rm int},1}=L_{{\rm int},0}.}        \tag{18}
\]

This is a global internal-vector invariant. Spatial coupling can exchange
local angular momentum between sites, so (18) is not a local clock-gate
orientation theorem.

## 5. Positive-edge critical-localization obstruction

Every term on the right of (1) is nonnegative. On a connected graph,

\[
\langle Q,KQ\rangle=0
\quad\Longleftrightarrow\quad
J_i=J_j\text{ on every edge}
\quad\Longleftrightarrow\quad
Q\text{ is spatially constant}.                \tag{19}
\]

On the uncontained connected substrate, a spatially constant field with
finite support is necessarily zero. Thus

\[
Q\ne0\text{ and bounded/finite support}
\quad\Longrightarrow\quad
\langle Q,KQ\rangle>0.                          \tag{20}
\]

To expose the clock consequence, choose a normalized scalar spatial profile
`phi` and a fixed unit polarization `e`, and restrict to the ray

\[
J_i=q\phi_i e,
\qquad
W_i=p\phi_i e.                                  \tag{21}
\]

Then

\[
H_\phi(q,p)=\frac{p^2}{2m}
+\frac{\kappa_\phi}{2}q^2
+\lambda c_4(\phi)q^4,                         \tag{22}
\]

with

\[
\kappa_\phi=\langle\phi,K\phi\rangle,
\qquad
c_4(\phi)=\sum_i\phi_i^4>0.                    \tag{23}
\]

An exact critical-quartic oscillator requires

\[
\frac{\partial^2H_\phi}{\partial q^2}(0,0)
=\kappa_\phi=0.                                 \tag{24}
\]

By (19), this selects only the spatially constant profile on a connected
finite quotient. That profile is box-wide rather than a bounded body. On the
uncontained substrate, the only finite-support case is zero.

Hence:

> **Positive-edge obstruction.** A single flux field with positive
> nearest-neighbor gradient energy and positive onsite radial quartic has no
> nonzero bounded exact zero-stiffness sector. Its localized modes are
> quadratic-plus-quartic, not exact critical-quartic `G*` clocks.

This is scoped to the registered positive same-field architecture. It does
not rule out a nonlinear breather, a metastable clock, a tuned defect zero
mode, a topological carrier, a multi-field cancellation, or a sign-indefinite
interaction. Those are different dynamical types and must be priced/tested.

## 6. Exact simultaneous solve is globally dependent

At `lambda=0`, (12) contains the linear solve

\[
A X=b,
\qquad
A=2mI+\frac{h^2}{2}K.                           \tag{25}
\]

For the regular connected production quotient, write

\[
K=C_{\rm WAVE}^2(dI-A_w),                       \tag{26}
\]

where `A_w` is the nonnegative weighted adjacency and every row sums to `d`.
Then

\[
A=\alpha I-\beta A_w,
\quad
\alpha=2m+\frac{h^2C_{\rm WAVE}^2d}{2},
\quad
\beta=\frac{h^2C_{\rm WAVE}^2}{2}.              \tag{27}
\]

Because

\[
\frac{\beta d}{\alpha}<1,                       \tag{28}
\]

the inverse has the convergent positive expansion

\[
A^{-1}=\frac1\alpha\sum_{r=0}^{\infty}
\left(\frac\beta\alpha A_w\right)^r.           \tag{29}
\]

For any two sites `i,j`, connectedness supplies a path of some length `r`, so
`(A_w^r)_{ij}>0`. Therefore

\[
\boxed{(A^{-1})_{ij}>0\quad\text{for all }i,j.} \tag{30}
\]

Equation (12) has a finite-range residual: each row reads only the site and
its 18 neighbors. But its exact solution has global algebraic dependence.
A perturbation of the right-hand side at a remote site changes `X_i` in the
same simultaneous solve. Treating that solve as one primitive tick would
violate the P4 one-Moore-shell dependency contract.

This is not a no-go for implicit mathematics. It is a type/cadence boundary:

- a local iterative solver can respect P4 if every iteration consumes an
  ontic tick, but then the solve has explicit latency and is not one tick;
- a local energy-transaction system may avoid global inversion, but it needs
  a separately declared transaction/state architecture; and
- a global solve may remain a reference calculation, but not an ontic update.

## 7. Production-map control

With `lambda=0`, (4)--(5) are implicit midpoint. The frozen production free
field instead uses

\[
P_{n+1}=P_n-KQ_n,
\qquad
Q_{n+1}=Q_n+P_{n+1},                            \tag{31}
\]

and preserves the exact normalized tick invariant

\[
H_{\rm tick}
=\frac12\langle P,P\rangle
+\frac12\langle Q,KQ\rangle
-\frac12\langle P,KQ\rangle.                   \tag{32}
\]

The cross term is not the endpoint Hamiltonian (3). Consequently FTD-0842 is
not an additive production patch. Installing its onsite force inside the
kick--drift phase would require a fresh invariant or an explicit energy error
ledger; installing (4)--(5) would replace the free integrator and introduce a
global solve.

## 8. What dynamics are now missing

FTD-0842 rules out only the simplest single-field positive architecture. The
next physical construction must supply at least one new mechanism from each
column:

| need | currently absent minimal classes |
|---|---|
| strictly local exact accounting | edge/bond energy-current registers; a finite local transaction rule; or an explicitly multi-tick local solve with latency |
| bounded zero/soft mode | defect/topological constraint; competing or sign-indefinite coupling; multi-field cancellation; or a separately bounded constituent scaffold |
| stable oriented clock sector | invariant/attracting polarization with local antisymmetric readout and an energy/work audit |
| `G*` cadence | native critical quarticity plus a finite-tick rotation/gate theorem; the continuum period alone is insufficient |

The left/right or “two hemispheres” intuition is therefore mathematically
relevant only if two channels do more than duplicate the same positive field:
they must create a relative mode whose common spatial stiffness cancels or is
topologically protected **while** a positive total energy and local causal
transactions remain. That is a concrete next discriminator, not yet a result.

## 9. Certificate verdict and non-claims

The first locked execution returned

```text
FTD-0842 coupled self-pair field energy: 26/26 PASS
COMBINED_DISCRETE_GRADIENT_UNIQUE_REVERSIBLE_AND_ENERGY_CLOSED
EXACT_SIMULTANEOUS_SOLVE_HAS_GLOBAL_ALGEBRAIC_DEPENDENCE
POSITIVE_EDGE_ENERGY_EXCLUDES_NONZERO_BOUNDED_ZERO_STIFFNESS_MODE
LOCAL_CRITICAL_GSTAR_CLOCK_REQUIRES_ADDITIONAL_DYNAMICAL_STRUCTURE
```

This is registered Outcome B. It proves exact conditional mathematical
closure and two scoped obstructions. It does not derive the selected onsite
coupling, a production integrator, a local clock body, a Born pushforward, an
actualization event, a biological mechanism, or a finite-tick `G*` calendar.

