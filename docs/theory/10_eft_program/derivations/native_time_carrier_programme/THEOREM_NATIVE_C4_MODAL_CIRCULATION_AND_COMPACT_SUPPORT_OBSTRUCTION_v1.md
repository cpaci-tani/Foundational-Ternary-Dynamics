# Theorem — Native `C4` modal circulation and compact-support obstruction v1

**Identifier:** `FTD-0919`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — EXACT COMMUTATOR CONSERVATION/TORQUE LAW]` +
`[THEOREM — GLOBAL PERIODIC C4 MODAL CIRCULATION]` +
`[CLOSED NEGATIVE — NONZERO FINITE-SUPPORT FINITE-DIMENSIONAL FREE C18 CARRIER]` +
`[CLOSED NEGATIVE — FREE ONE-TICK ORDER-FOUR MODE]` +
`[OPEN — MAINTAINED OR NONLINEAR LOCAL CLOCK BODY]`

## 1. Result

The unchanged source-free production field possesses exact conserved
clockwise/counterclockwise circulation on every degenerate modal plane. The
general charge is

\[
\boxed{\mathcal L_A=J^TAP},\qquad A^T=-A,
\]

and the exact conservation criterion is

\[
\boxed{[A,K]=0},
\]

where `K=-C_WAVE^2 Delta_18` is the real symmetric free-field stiffness.

The positive result is global: on a finite periodic `L=4` probe, the exact
modes

\[
a(x,y,z)=\sin(\pi x/2),\qquad
b(x,y,z)=\sin(\pi y/2)
\]

form a physical `C4` doublet, share stiffness `2/3`, and carry an exactly
conserved modal circulation.

The localization result is closed negative for the unchanged free operator.
The 18-point convolution has no nonzero finite-support eigenvector. Hence it
has no nonzero finite-dimensional invariant subspace with finite support and
no finite-support finite-rank circulation generator commuting with `K`.

This resolves the two phrases that had been running together:

- the substrate free field has a **coherent global modal calendar**;
- it does not thereby contain a **bounded local clock body**.

The missing gearbox is now a localization/confinement mechanism, not a
missing complex coordinate or orientation observable.

## 2. Exact discrete circulation balance

Let `J,P` be the finite-volume real flux and wave-velocity vectors and let

\[
K^T=K,
\qquad
A^T=-A.
\]

The production-order kick--drift with arbitrary impulse `U` is

\[
P^+=P-hKJ+U,
\qquad
J^+=J+hP^+.
\]

Because every skew quadratic self-pair vanishes,

\[
(P^+)^TAP^+=0.
\]

Therefore

\[
\begin{aligned}
\mathcal L_A^+
&=(J+hP^+)^TAP^+\\
&=J^TA(P-hKJ+U)\\
&=\mathcal L_A-hJ^TAKJ+J^TAU.
\end{aligned}
\]

Since

\[
(AK)^T=-KA,
\]

the quadratic form sees the symmetric part

\[
J^TAKJ={1\over2}J^T(AK-KA)J
={1\over2}J^T[A,K]J.
\]

Thus the exact ledger is

\[
\boxed{
\Delta\mathcal L_A
=-{h\over2}J^T[A,K]J+J^TAU.}
\]

The first term is the stiffness/boundary torque. The second is the source
torque.

The commutator `[A,K]` is symmetric. A symmetric quadratic form vanishes for
every `J` if and only if its matrix is zero. Consequently, in the source-free
sector,

\[
\boxed{
\mathcal L_A\text{ is conserved for every state}
\iff[A,K]=0.}
\]

This is an exact property of the discrete kick--drift map, not a continuum
approximation.

## 3. Damping and noise ledger

Apply common post-drift momentum damping `rho` followed by additive impulse
or noise `eta`:

\[
P_{\rm end}=\rho P^++\eta.
\]

Then

\[
\boxed{
\mathcal L_{A,\rm end}
=\rho\left(
\mathcal L_A-{h\over2}J^T[A,K]J+J^TAU
\right)
+(J^+)^TA\eta.}
\]

Common damping scales the post-torque charge. Additive forcing contributes
its own oriented impulse. Spatially varying damping, genesis drains, Gauss
projection, evaporation, and other non-free maps require their own defect
terms and are not covered by the common-scalar formula.

## 4. Degenerate modes are exactly the carrier planes

Let `a,b` be orthonormal eigenvectors of `K` with the same eigenvalue:

\[
Ka=\kappa a,
\qquad
Kb=\kappa b.
\]

Define

\[
A=ab^T-ba^T.
\]

Then `A^T=-A` and direct multiplication gives

\[
AK=KA=\kappa A.
\]

Writing the modal coordinates as

\[
Q_a=a^TJ,\quad Q_b=b^TJ,
\qquad
P_a=a^TP,\quad P_b=b^TP,
\]

the charge is

\[
\boxed{
\mathcal L_{ab}=Q_aP_b-Q_bP_a.}
\]

It is exactly conserved by the source-free tick.

Conversely diagonalize symmetric `K`. In its eigenbasis,

\[
[A,K]_{ij}=A_{ij}(\kappa_j-\kappa_i).
\]

Thus a nonzero entry `A_ij` can occur only when
`kappa_i=kappa_j`. Every conserved skew generator rotates within degenerate
eigenspaces and nowhere else. The continuous `O(2)` mixing symmetry of that
degenerate quadratic plane—not discrete `C4` by itself—is what produces the
conserved charge.

## 5. Exact periodic `C4` witness

Take the finite periodic quotient `(Z/4Z)^3` only as a computational witness.
The sine values are exactly

\[
(0,1,0,-1).
\]

Define

\[
a(x,y,z)=\sin(\pi x/2),
\qquad
b(x,y,z)=\sin(\pi y/2).
\]

Exact finite summation gives

\[
\langle a,b\rangle=0,
\qquad
\|a\|^2=\|b\|^2=32.
\]

For the quarter-turn pullback

\[
(Sf)(x,y,z)=f(y,-x,z),
\]

one has

\[
Sa=b,
\qquad
Sb=-a.
\]

Hence the modal plane carries

\[
S\big|_{\operatorname{span}(a,b)}
=\begin{pmatrix}0&-1\\1&0\end{pmatrix},
\qquad S^2=-I.
\]

The production stencil acts exactly as

\[
\Delta_{18}a=-2a,
\qquad
\Delta_{18}b=-2b.
\]

Since `C_WAVE^2=1/3`, both modes have

\[
\kappa={2\over3}.
\]

Their modal circulation

\[
\mathcal L_{ab}=Q_aP_b-Q_bP_a
\]

is therefore exactly invariant under the unit source-free production tick.

This is not a local clock. Each mode occupies 32 of the 64 sites and every
`z` slice. Its normalization and recurrence are defined on a periodic
computational quotient. The uncontained ontology supplies no periodic wall
that turns this global mode into a bounded object.

## 6. Compact-support eigenmode obstruction

The proof can be stated without treating a completed infinite lattice as an
ontic object. Use the algebra of finite formal translations. A scalar field
with finite support has a Laurent polynomial representative

\[
F(z)=\sum_{x\in S}f_xz_x^{x_1}z_y^{x_2}z_z^{x_3}
\]

in

\[
R=\mathbb R[z_x^{\pm1},z_y^{\pm1},z_z^{\pm1}].
\]

The production stiffness is convolution by the nonconstant Laurent
polynomial

\[
\begin{aligned}
\kappa_{18}(z)
=-{1\over3}\bigg[&{1\over3}
\sum_{\rm faces}z^e
+{1\over6}\sum_{\rm edges}z^e-4\bigg].
\end{aligned}
\]

If a nonzero finite-support eigenfield existed, it would obey

\[
(\kappa_{18}(z)-\lambda)F(z)=0.
\]

But `R` is an integral domain, and `kappa_18(z)-lambda` is nonzero because it
contains nonzero neighbor monomials. Hence

\[
F(z)=0.
\]

Therefore:

\[
\boxed{
\text{the free production stiffness has no nonzero finite-support
eigenvector}.}
\]

The vector field adds three independent components and does not change the
argument.

## 7. No compact finite-dimensional invariant doublet

Suppose a nonzero finite-dimensional subspace `V` consists of fields
supported in one finite region and satisfies

\[
KV\subseteq V.
\]

The restriction of symmetric `K` to `V` is a finite symmetric operator, so
the finite-dimensional spectral theorem supplies a nonzero eigenvector in
`V`. It would have finite support, contradicting section 6. Thus

\[
\boxed{
V=0.}
\]

Now suppose `A` is a nonzero finite-rank skew generator with finite spatial
support and `[A,K]=0`. Its range is a nonzero finite-dimensional,
finite-support subspace, and

\[
K\operatorname{Ran}(A)
=\operatorname{Ran}(KA)
=\operatorname{Ran}(AK)
\subseteq\operatorname{Ran}(A).
\]

That is impossible by the preceding result. Hence

\[
\boxed{
\text{no nonzero compact finite-rank circulation generator commutes with
the free }K.}
\]

FTD-0918's exterior value `q/3` is the smallest explicit witness of this
general theorem: its local plaquette generator has a nonzero commutator with
the full stiffness, so circulation leaves through the boundary.

## 8. Entire free band excludes a one-tick quarter-turn

With `c_i=cos(k_i)`, the exact stencil symbol is

\[
L_{18}(k)={2\over3}
(c_x+c_y+c_z+c_xc_y+c_yc_z+c_zc_x)-4.
\]

The bracket is multi-affine on `[-1,1]^3`; therefore its extrema occur at the
eight vertices. Exact evaluation gives

\[
-2\le
c_x+c_y+c_z+c_xc_y+c_yc_z+c_zc_x
\le6.
\]

Consequently

\[
-{16\over3}\le L_{18}\le0,
\qquad
0\le\kappa_{18}=-{1\over3}L_{18}\le{16\over9}<2.
\]

For the unit kick--drift mode

\[
M_\kappa=
\begin{pmatrix}1-\kappa&1\\-\kappa&1\end{pmatrix},
\]

direct multiplication shows

\[
M_\kappa^2=-I
\iff\kappa=2.
\]

No source-free production mode therefore performs an exact one-tick
quarter-turn. The theorem does not exclude other exact finite periods,
multiple-tick phase gates, or maintained clocks. It excludes only the claim
that the unchanged unit free tick already supplies an order-four local rotor.

## 9. What remains available

The closed-negative result is sharply scoped to the unchanged linear
source-free 18-point action and exact finite-support finite-dimensional
localization. It does not exclude:

1. a driven region whose boundary torque is returned by an explicit source;
2. a nonlinear localized mode or breather;
3. a manifested core coupled reciprocally to the field;
4. a compact or constrained field type adopted at a declared price;
5. an approximate long-lived dispersive packet; or
6. FTD-0841's selected onsite quartic confinement.

FTD-0841 demonstrates that an imposed local radial quartic can have compact
energy shells and exact angular-momentum recursion. The present theorem
explains why that term is doing real ontological/dynamical work: it is not
already hidden inside the production quadratic free wave. Its production
origin, support, reciprocal reaction, coefficient, and work ledger remain
open.

## 10. Consequence for `G*`

The free field supplies action-angle phase and global degenerate circulation,
but neither selects the critical quartic potential nor localizes a maintained
clock. Therefore `G*` cannot yet be identified with a production clock period.

The honest order is:

\[
\text{localize/maintain a carrier}
\to\text{close energy and boundary torque}
\to\text{establish a critical quartic law}
\to\text{test }G^*\text{ as its period factor}.
\]

Using `G*` before the first three steps would merely relabel a modal phase or
rescale a nonlocal oscillator. No gamma magnitude, Born frequency, Bell law,
or measurement selector follows from the circulation theorem.

## 11. Registered outcome and next route

The preregistered result is:

```text
OUTCOME=A_GLOBAL_MODAL_CIRCULATION_COMPACT_LOCAL_FREE_BODY_OBSTRUCTION
FREE_CONSERVATION_CRITERION=COMMUTATOR_ZERO
PERIODIC_GLOBAL_C4_WITNESS=EXACT
FINITE_SUPPORT_EIGENMODE=FORBIDDEN
FINITE_SUPPORT_INVARIANT_DOUBLET=FORBIDDEN
FREE_ONE_TICK_ORDER_FOUR=FORBIDDEN
MAINTAINED_OR_NONLINEAR_LOCAL_CLOCK=OPEN
PRODUCTION_CHANGED=FALSE
GSTAR_USED=FALSE
GAMMA_DERIVED=FALSE
BORN_BELL_CONTEXT_USED=FALSE
```

The next admissible attack is the first non-free route: derive an exact
source-balanced maintained region from an existing reciprocal action, or
price a minimal nonlinear confinement term and require it to close:

- field and matter energy;
- circulation inflow/outflow;
- source reaction;
- finite support or quantified tails;
- reversal and reset;
- controller work and dissipation; and
- context-blind clock compliance.

No new production term is licensed by this theorem alone.

## 12. Certificate

- Locked protocol:
  `PREREG_NATIVE_C4_MODAL_CIRCULATION_AND_COMPACT_SUPPORT_OBSTRUCTION_v1.md`
  (`SHA-256 BD097E6ACC011E11248221875086B3F4257367D459C70542188101B9192F214E`)
- Exact certificate:
  `scripts/proofs/proof_native_c4_modal_circulation_compact_support_obstruction.py`
  (`SHA-256 C1C312E1B5FA9F9EB90DFD1A2B71B38736BC7F8AEE93DFDBA56B88A5133031EA`)
- Result: `54/54` exact checks passed.

The certificate source-locks the production stencil and update, the native
discrete-action audit, the earlier modal phase/action theorem, the selected
quartic reference theorem, and FTD-0918. It performs no numerical search and
changes no engine source, CMake target, production default, or result corpus.
