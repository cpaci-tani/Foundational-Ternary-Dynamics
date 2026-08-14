# FTD-0931 — Native retarded static-halo radiative formation and finite-recurrence boundary v1

**Identifier:** `FTD-0931`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — NATIVE FIXED-SOURCE RADIATIVE STATIC FORMATION]` +
`[THEOREM — POSITIVE SOURCE-CENTERED TICK ENERGY]` +
`[THEOREM — THREE-DIMENSIONAL MINIMUM GENERIC MONOPOLE IR THRESHOLD]` +
`[THEOREM — FINITE-GROUNDED NONCONVERGENCE / CESARO CONVERGENCE]` +
`[OPEN — SOURCE FORMATION/RECOIL / TIME-DEPENDENT TRACKING / PRODUCTION COUPLING]`  
**Protocol:**
[`PREREG_NATIVE_RETARDED_STATIC_HALO_RADIATIVE_FORMATION_BOUNDARY_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_NATIVE_RETARDED_STATIC_HALO_RADIATIVE_FORMATION_BOUNDARY_v1.md),
SHA-256 `B32E91E59C21366309C0BBA654C94DF312A7267B4F71B26AED1AD804A9973CED`  
**Certificate:**
[`proof_native_retarded_static_halo_radiative_formation_boundary.py`](../../../../../scripts/proofs/proof_native_retarded_static_halo_radiative_formation_boundary.py),
SHA-256 `C37A08BE45533F7E9415076AD779FA7EC14CAFF82A38A084753D81A5475EF028`,
`154/154` exact checks  
**Registered outcome:** `A — NATIVE RADIATIVE STATIC FORMATION / FINITE-RECURRENCE BOUNDARY`

---

## 1. Result

The massless static halo does have a natural causal formation mechanism. It
does not require damping, repeated fresh ports, or instantaneous solution of
the Poisson equation. It is the native retarded wave pair itself.

Let `J` be the flux coordinate, `W=wave_vel` its native canonical momentum,
`K` the production-normalized C18 stiffness, and `f` a fixed compact source.
In the frozen undamped unit-tick sector,

\[
 W_{n+1}=W_n-KJ_n+f,
 \qquad
 J_{n+1}=J_n+W_{n+1}.                                    \tag{1}
\]

Starting from `J_{-1}=J_0=W_0=0`, equation (1) has four simultaneous
properties:

1. every finite tick is radius-one causal and finitely supported;
2. the source-centered field has an exact positive conserved tick energy;
3. on the uncontained three-dimensional translation-invariant scaffold,
   `J_n(x)` converges at every fixed site to the static Green profile; and
4. the mismatch energy does not disappear—it disperses into the outgoing
   retarded field outside every fixed region.

Thus the static field is not created everywhere at once. Its causal front
expands, and each already-reached finite region settles locally while the
history continues outward.

This result uses the existing `(flux,wave_vel)` type proved canonical in
FTD-0574. It does not add a field, bath, hidden port, or ontology type.

---

## 2. Exact native recursion and positive energy

The C18 symbol is

\[
 \kappa(k)={4\over3}-{2\over9}
 (u+v+w+uv+uw+vw),                                      \tag{2}
\]

where `u=cos(k_x)`, `v=cos(k_y)`, and `w=cos(k_z)`. It has exact band

\[
 0\le\kappa\le{16\over9}.                               \tag{3}
\]

For the fixed static response `J_*`,

\[
 KJ_*=f.                                                  \tag{4}
\]

Set `e_n=J_n-J_*`. Equation (1) becomes

\[
 \binom{e_{n+1}}{W_{n+1}}
 =\begin{pmatrix}I-K&I\\-K&I\end{pmatrix}
 \binom{e_n}{W_n}.                                        \tag{5}
\]

The exact invariant is

\[
 \boxed{
 H_{\rm rad}(e,W)
 ={1\over2}\langle W,W\rangle
 +{1\over2}\langle e,Ke\rangle
 -{1\over2}\langle W,Ke\rangle.}                        \tag{6}
\]

For one stiffness mode `a`, equation (5) and its metric are

\[
 U_a=\begin{pmatrix}1-a&1\\-a&1\end{pmatrix},
 \qquad
 G_a=\begin{pmatrix}a&-a/2\\-a/2&1\end{pmatrix}.         \tag{7}
\]

Direct algebra gives

\[
 U_a^TG_aU_a=G_a,
 \qquad
 \det U_a=1,
 \qquad
 \det G_a=a(1-a/4).                                      \tag{8}
\]

Every nonzero C18 mode lies strictly in `0<a<4`. Equivalently,

\[
 H_{\rm rad}
 ={1\over2}\|W-Ke/2\|^2
 +{1\over2}\langle e,K(I-K/4)e\rangle\ge0.              \tag{9}
\]

The abrupt source switch places the zero field above the new static minimum
by

\[
 \boxed{
 E_{\rm form}=H_{\rm rad}(-J_*,0)
 ={1\over2}\langle f,K^{-1}f\rangle.}                    \tag{10}
\]

Equation (10) is the exact positive formation debit in the registered fixed-
source field sector. It does not derive the matter reservoir that creates and
holds `f`.

---

## 3. Exact retarded step response

Define

\[
 \cos\omega(k)=1-\kappa(k)/2.                             \tag{11}
\]

Since `0<=kappa<=16/9`,

\[
 0\le\omega\le2\arcsin(2/3)<\pi,
 \qquad
 \cos[\omega/2]=\sqrt{1-\kappa/4}\ge{\sqrt5\over3}.     \tag{12}
\]

The exact source response is

\[
 \boxed{
 J_n(x)=J_*(x)-
 \int_{\mathbb T^3}{d^3k\over(2\pi)^3}
 e^{ik\cdot x}{\widehat f(k)\over\kappa(k)}
 {\cos[(n+1/2)\omega(k)]\over\cos[\omega(k)/2]}.}       \tag{13}
\]

The modal transient in equation (13) is fixed by

\[
 C_{-1}=C_0=1,
 \qquad
 C_{n+1}=(2-\kappa)C_n-C_{n-1},                           \tag{14}
\]

so

\[
 C_n={\cos[(n+1/2)\omega]\over\cos(\omega/2)}.           \tag{15}
\]

The characteristic polynomial is exactly the corrected FTD-0558 production
pole,

\[
 z^2-(2-\kappa)z+1.                                      \tag{16}
\]

The certificate verifies equations (13)--(16) through the exact polynomial
recurrence for nine ticks, including the field, momentum, and positive-energy
identity on every arm.

Equation (13) must not be read as an instantaneous nonlocal evaluation. In
position space, equation (1) proves inductively that after `n` ticks the field
depends only on the finite C18 causal cone of the compact source. The static
and oscillatory terms in equation (13) cancel exactly outside that cone at
every finite tick.

---

## 4. Why three dimensions are the threshold

The stiffness has a unique torus zero at `k=0`. Its Hessian is

\[
 D^2\kappa(0)={2\over3}I,                                 \tag{17}
\]

so

\[
 \kappa(k)={|k|^2\over3}+O(|k|^4).                        \tag{18}
\]

For a compact source with nonzero total, `fhat(0) != 0`. Both the static Green
profile and the source-switch formation energy have infrared measure

\[
 {d^dk\over\kappa(k)}\sim r^{d-3}dr.                     \tag{19}
\]

The integral in equation (19) is finite precisely when `d>2`. Therefore:

\[
 \boxed{
 d=3\text{ is the minimum spatial dimension in which a generic compact
 monopole source has this finite-energy local static response}.}           \tag{20}
\]

Equation (20) is a minimum threshold, not uniqueness against `d>3`. A neutral
source whose Fourier numerator also vanishes at zero has a different
infrared count and is not governed by the generic-monopole statement.

This threshold is structural. It is not a numerical match, fitted exponent,
or continuum-limit substitution.

---

## 5. Proof of instantaneous local convergence

Fix a site `x`. In three dimensions,

\[
 g_x(k)=e^{ik\cdot x}{\widehat f(k)\over
 \kappa(k)\cos[\omega(k)/2]}                              \tag{21}
\]

belongs to `L1(T^3)`:

- compact support makes `fhat` bounded;
- equation (18) gives only `1/|k|^2` at the unique zero;
- three-dimensional measure contributes `|k|^2 d|k|`; and
- equation (12) keeps the half-angle denominator away from zero.

The phase `omega` is Lipschitz and nonconstant. Away from `k=0`,

\[
 \nabla\omega={\nabla\kappa\over2\sin\omega}.            \tag{22}
\]

At least one analytic derivative of `kappa` is not identically zero—for
example

\[
 \partial_{k_x}\kappa(\pi/2,0,0)={2\over3}.              \tag{23}
\]

Hence the critical set of `omega` has measure zero. Applying the standard
coarea formula to equation (21) pushes `g_x(k)d^3k` to an `L1` density on the
frequency interval. The Riemann--Lebesgue lemma then sends both exponential
branches of the cosine in equation (13) to zero.

Therefore

\[
 \boxed{
 J_n(x)\longrightarrow J_*(x),
 \qquad
 W_n(x)=J_n(x)-J_{n-1}(x)\longrightarrow0}                \tag{24}
\]

for every fixed site. Equivalently, for every specified finite region `R`
and every `epsilon>0`, there is a finite tick `N(R,epsilon)` after which the
field and momentum on `R` lie within `epsilon` of static rest.

No convergence rate is claimed. Critical-level geometry can affect rates,
and the long-wave phase approaches zero.

Equation (24) is the project-compatible local statement. It neither treats
the uncontained substrate as a completed infinity nor invokes an `L to
infinity` limit. The Brillouin representation is the proof scaffold for a
finite-support source and an epsilon-local conclusion.

---

## 6. Why finite boxes do not settle

On every specified finite grounded region, `K` is positive definite and has
a discrete spectrum. For each mode,

\[
 \lambda_\pm=e^{\pm i\omega}.                             \tag{25}
\]

The zero-field source switch has nonzero invariant (10). If the
instantaneous field converged to `(J_*,0)`, the invariant would converge to
zero, contradicting exact conservation. A finite undamped grounded system is
therefore recurrent or quasiperiodic; it does not form an instantaneous
attractor.

Its Cesaro average does converge:

\[
 {1\over N}\sum_{n=0}^{N-1}e^{in\omega}
 ={1-e^{iN\omega}\over N(1-e^{i\omega})}\longrightarrow0 \tag{26}
\]

for every fixed nonzero finite mode.

There is no region-independent rate in equation (26), because larger
grounded regions admit smaller frequencies. This is the same massless
infrared fact seen from the finite side.

The contrast is essential:

- finite closed field: positive, stable, recurrent, static only after time
  averaging;
- uncontained three-dimensional field: positive and globally conservative,
  but locally convergent because the mismatch disperses outside every fixed
  region.

The uncontained environment is doing real mathematical work. It is not an
infinite box used as rhetoric.

---

## 7. The reservoir interpretation

FTD-0930's coordinate-relaxation route exports one residual into one fresh
port at each active site. FTD-0931 supplies a different mechanism for a
fixed static source. The complete existing field pair `(J,W)` stores the
residual as a propagating wave, so no stream of newly blank field-shaped
ports is required.

Global `H_rad` remains constant, while equation (24) makes the mismatch and
momentum vanish on every fixed finite region. The energy and phase history
have moved outward. They have not been deleted. This is a direct dynamical
realization of reduced unactualization:

\[
 \text{locally irrelevant mismatch}
 \quad\longrightarrow\quad
 \text{inaccessible outgoing field history}.             \tag{27}
\]

This closes the **fixed-source static-halo formation** line at reference
level. It does not close:

- formation, motion, or reciprocal recoil of the matter source;
- a time-dependent source that tracks the rotating FTD-0929 companion;
- nonlinear recovery after source perturbations;
- finite-domain absorbing hardware;
- coupled production behavior with projection, genesis, damping, movement,
  and forces;
- a physical identification as gravity, dark matter, or photons; or
- the `(X,Q)` to `(L,R)` identity.

---

## 8. Time and `G*`

Equation (1) uses the ontic integer tick `n`. No continuous subcycle and no
`G*` cadence enters the proof. The static field settles locally because its
native frequency continuum dephases spatially, not because a critical
quartic clock selects an outcome.

This sharpens the gearbox boundary:

- native integer time already drives causal radiative formation;
- `G*` is not needed to form the fixed static halo;
- a future `G*` role must therefore concern eligibility, synchronization, or
  another independently demonstrated clock function, not ordinary static
  field formation.

---

## 9. Verification and boundary statement

The byte-frozen protocol has SHA-256
`B32E91E59C21366309C0BBA654C94DF312A7267B4F71B26AED1AD804A9973CED`.
The exact certificate has SHA-256
`C37A08BE45533F7E9415076AD779FA7EC14CAFF82A38A084753D81A5475EF028`
and passes `154/154` checks.

The registered verdict is Outcome A:

```text
OUTCOME=A_NATIVE_RADIATIVE_STATIC_FORMATION_FINITE_RECURRENCE_BOUNDARY
NATIVE_PAIR=FLUX_WAVE_VELOCITY
FIXED_SOURCE_TICK_MAP=EXACT_AFFINE_SYMPLECTIC
SOURCE_CENTERED_TICK_ENERGY=POSITIVE_AND_EXACTLY_CONSERVED
FORMATION_DEBIT=(1/2)<f,K^-1 f>
FINITE_TICK_SUPPORT=CAUSAL
UNCONTAINED_D3_INSTANTANEOUS_LOCAL_STATIC_FORMATION=YES
MINIMUM_GENERIC_MONOPOLE_DIMENSION=3
FINITE_GROUNDED_INSTANTANEOUS_CONVERGENCE=NO
FINITE_GROUNDED_CESARO_CONVERGENCE=YES
OUTGOING_MISMATCH_HISTORY=RADIATIVE_FIELD
FRESH_PORT_STREAM_FOR_STATIC_FORMATION=NOT_REQUIRED
MOVING_SOURCE_RECOIL=OPEN
TIME_DEPENDENT_COMPANION_TRACKING=OPEN
PRODUCTION_CHANGED=FALSE
GSTAR_USED=FALSE
BORN_BELL_CONTEXT_USED=FALSE
```

No engine source, CMake target, `Voxel` field, toggle, default, production
law, type, import, paper, physical constant, or phenomenological formula was
changed. No damping, numerical search, fitted decay, near-miss,
formula-substitution discovery, completed-infinity claim, or `L to infinity`
argument was used.
