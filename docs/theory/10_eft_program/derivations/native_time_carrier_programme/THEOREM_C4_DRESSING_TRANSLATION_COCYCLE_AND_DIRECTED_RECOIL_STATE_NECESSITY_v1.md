# FTD-0934 — C4 dressing translation cocycle and directed-recoil state necessity v1

**Identifier:** `FTD-0934`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — NEGATIVE-TYPE C4 DRESSING TRANSLATION GEOMETRY]` +
`[SCOPED NO-GO — EVEN DRESSING ENERGY CANNOT SELECT A DIRECTED HOP]` +
`[THEOREM — TRANSLATION-CHARACTER ORIENTATION CLASSIFIER]` +
`[BOUNDARY — BLOCH TORUS IS NOT UNWRAPPED PHYSICAL MOMENTUM]` +
`[OPEN — DYNAMIC COMMON ACTION / IMPULSE ORIGIN / VECTOR RECOIL]`  
**Protocol:**
[`PREREG_C4_DRESSING_TRANSLATION_COCYCLE_AND_DIRECTED_RECOIL_STATE_NECESSITY_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_C4_DRESSING_TRANSLATION_COCYCLE_AND_DIRECTED_RECOIL_STATE_NECESSITY_v1.md),
SHA-256 `5252D61FFABB0BBA9E61524B5345943F627F9C032A502C350AECC0EDEC34922A`  
**Certificate:**
[`proof_c4_dressing_translation_cocycle_directed_recoil_state_necessity.py`](../../../../../scripts/proofs/proof_c4_dressing_translation_cocycle_directed_recoil_state_necessity.py),
SHA-256 `52C776DE265D8535C7CF0ABF531EC468802CA06FE71B40BC3D61EC963CAD3DD3`,
`279/279` exact checks  
**Registered outcome:** `A — NEGATIVE-TYPE DRESSING GEOMETRY / DIRECTED-STATE NECESSITY`

---

## 1. Result

FTD-0933's positive hop wake is not just an isolated energy formula. It is
the square norm of an exact translation-group cocycle. Consequently the
formed C4 dressing embeds integer source positions into a Hilbert geometry:

\[
 d\longmapsto b_Y(d)=\pi(d)Y-Y,                        \tag{1}
\]

\[
 \overline{\mathcal D}(d)=\|b_Y(d)\|^2.               \tag{2}
\]

The square root of equation (2) is a genuine translation-orbit metric for
nonzero compact source data with no translation stabilizer, and equation (2)
is conditionally negative definite on `Z^3`.

This positive geometry still cannot move the source. Its spectral factor is

\[
 \boxed{
 |1-e^{-ik\cdot d}|^2=2[1-\cos(k\cdot d)],}            \tag{3}
\]

which is exactly invariant under `d -> -d`. The energy remembers separation
but loses orientation. An isotropic scalar action built only from equation
(2) therefore prefers rest and cannot choose a nonzero directed hop or
vector recoil.

The missing orientation datum is the unsquared translation character

\[
 \boxed{\chi_k(d)=e^{ik\cdot d},\qquad k\in\mathbb T^3.} \tag{4}
\]

Its conjugation distinguishes reverse displacements:

\[
 \chi_k(-d)=\overline{\chi_k(d)},
 \qquad
 \sin(k\cdot[-d])=-\sin(k\cdot d).                    \tag{5}
\]

Equation (4) classifies the minimum one-dimensional unitary representation
of directed integer translation. It is compact Bloch data, not yet physical
momentum. A physical recoil law must dynamically generate and update this
directed state, choose or derive its lift and scale, and couple it
reciprocally to the field through one common action.

---

## 2. Four-phase energy space

Let the exact C4 companion phases be

\[
 Y_r=(q_r,p_r),\qquad r=0,1,2,3.                       \tag{6}
\]

Polarization of the FTD-0932 positive energy gives

\[
 \begin{aligned}
 \langle(e,z),(f,w)\rangle_{C4}
 ={}&{1\over2}\langle z,w\rangle
   +{1\over2}\langle e,Kf\rangle\\
  &-{1\over4}
   [\langle z,Kf\rangle+\langle w,Ke\rangle].
 \end{aligned}                                        \tag{7}
\]

Its diagonal is `H_C4(e,z)`. On the finite-energy uncontained class the only
zero of equation (7) is the zero pair. Form the direct sum

\[
 \mathcal H_4=\bigoplus_{r=0}^3\mathcal H_{C4},
 \qquad
 \langle Y,Z\rangle_4={1\over4}\sum_{r=0}^3
 \langle Y_r,Z_r\rangle_{C4}.                          \tag{8}
\]

Integer translation acts diagonally:

\[
 \pi(d)=\bigoplus_{r=0}^3T_d.                          \tag{9}
\]

Because `T_d` is unitary and commutes with `K`, equation (9) is unitary in
the energy metric.

The FTD-0933 phase-averaged wake is precisely

\[
 \boxed{
 \overline{\mathcal D}(d)
 ={1\over4}\sum_{r=0}^3
 H_{C4}((T_d-I)q_r,(T_d-I)p_r)
 =\|\pi(d)Y-Y\|_4^2.}                                 \tag{10}
\]

No new energy or translation rule is inserted in equation (10).

---

## 3. Exact cocycle and metric

Define

\[
 b_Y(d)=\pi(d)Y-Y.                                     \tag{11}
\]

The representation law gives

\[
 \begin{aligned}
 b_Y(d+e)
 &=\pi(d)\pi(e)Y-Y\\
 &=[\pi(d)Y-Y]+\pi(d)[\pi(e)Y-Y].
 \end{aligned}
\]

Therefore

\[
 \boxed{b_Y(d+e)=b_Y(d)+\pi(d)b_Y(e).}                 \tag{12}
\]

This is the exact one-cocycle identity. Translation unitarity gives

\[
 \overline{\mathcal D}(0)=0,
 \qquad
 \overline{\mathcal D}(-d)=\overline{\mathcal D}(d). \tag{13}
\]

It also gives the triangle inequality:

\[
 \begin{aligned}
 \sqrt{\overline{\mathcal D}(d+e)}
 &=\|b_Y(d)+\pi(d)b_Y(e)\|_4\\
 &\le\|b_Y(d)\|_4+\|b_Y(e)\|_4.
 \end{aligned}                                        \tag{14}
\]

For a nonzero compact source, `b_Y(d)=0` at nonzero `d` would make every
companion phase periodic along an infinite translation orbit. A
square-summable periodic field must vanish, which would force the source to
vanish. Hence

\[
 \boxed{
 \rho_Y(d,e)=
 \sqrt{\overline{\mathcal D}(d-e)}}                    \tag{15}
\]

is a metric on the source's integer translation orbit.

The certificate verifies equations (10)--(15) exactly on a 27-site rational
C18 witness, including every nonzero translation, all five registered
cocycle/polarization representative pairs, and all face/edge/corner cubic
orbits.

---

## 4. Negative type

For positions `d_a` and real coefficients satisfying `sum_a c_a=0`, use

\[
 \overline{\mathcal D}(d_a-d_b)
 =\|\pi(d_a)Y-\pi(d_b)Y\|_4^2.                         \tag{16}
\]

The diagonal terms vanish after summing against the zero-sum coefficients,
leaving

\[
 \boxed{
 \sum_{a,b}c_ac_b\overline{\mathcal D}(d_a-d_b)
 =-2\left\|\sum_a c_a\pi(d_a)Y\right\|_4^2\le0.}      \tag{17}
\]

Thus `Dbar` is conditionally negative definite. Its exact polarization is

\[
 \boxed{
 \langle b_Y(d),b_Y(e)\rangle_4
 ={1\over2}
 [\overline{\mathcal D}(d)+\overline{\mathcal D}(e)
 -\overline{\mathcal D}(d-e)].}                       \tag{18}
\]

Equations (17)--(18) are useful because they show that the source-position
geometry is not fitted. It is forced by one already-derived positive field
energy and the exact translation representation.

They still do not define source inertia. The embedding lives in field-state
space; it contains no source velocity, physical momentum normalization, or
equation selecting the next `d`.

---

## 5. Why the positive geometry stays at rest

The most direct source-edge functional supplied by the field is

\[
 S_{\rm wake}[X]
 =\sum_n\overline{\mathcal D}(X_{n+1}-X_n).            \tag{19}
\]

It is nonnegative and translation invariant. It is also even:

\[
 \overline{\mathcal D}(d)
 =\overline{\mathcal D}(-d).                           \tag{20}
\]

For the compact registered class,

\[
 \overline{\mathcal D}(0)=0
 <\overline{\mathcal D}(d)qquad(d\ne0).               \tag{21}
\]

Therefore a local minimum of equation (19) remains at rest. If a nonzero
step is externally required, cubic covariance partitions the 26 Moore steps
only into face, edge, and corner orbits of sizes `6`, `12`, and `8`; all
directions within an orbit remain degenerate for a cubic source.

More generally, any scalar depending only on the dressing distance and
cubic-invariant rest data is inversion even. A scalar that were also odd
under `d -> -d` would obey

\[
 A(d)=A(-d)=-A(d),                                     \tag{22}
\]

and hence vanish. This proves the scoped no-go:

\[
 \boxed{
 \text{positive dressing energy alone cannot select a nonzero directed
 hop or vector recoil from isotropic rest}.}            \tag{23}
\]

The theorem does not forbid motion driven by an incoming field momentum,
external gradient, body axis, existing current, defect orientation, or an
additional time-odd vector state. It proves that one of those directional
data is necessary.

---

## 6. The information lost by the square

All one-dimensional unitary representations of `Z^3` are its characters.
A character is fixed freely by its values on the three basis translations:

\[
 \chi(e_j)=e^{ik_j}.
\]

Therefore

\[
 \boxed{
 \chi_k(d)=e^{i(k_xd_x+k_yd_y+k_zd_z)},
 \qquad k\in\mathbb T^3.}                             \tag{24}
\]

The group and reversal laws are

\[
 \chi_k(d+e)=\chi_k(d)\chi_k(e),
 \qquad
 \chi_k(-d)=\overline{\chi_k(d)}.                     \tag{25}
\]

Now compare the two pieces:

\[
 \operatorname{Re}\chi_k(d)=\cos(k\cdot d),
 \qquad
 \operatorname{Im}\chi_k(d)=\sin(k\cdot d).           \tag{26}
\]

The real part is even; the imaginary part is odd. Squaring the translation
difference gives

\[
 \boxed{
 |1-\chi_k(d)|^2=2[1-\operatorname{Re}\chi_k(d)],}     \tag{27}
\]

which removes `Im chi` and therefore removes the sign exchanged by
conjugation.

This is the exact translation analogue of the earlier symmetric-square
lesson:

- a symmetric square can preserve size, axis, and energy geometry;
- it cannot preserve clockwise/counterclockwise or forward/backward
  orientation;
- an unsquared phase/current is needed to retain that distinction.

Accordingly a nontrivial `Z^3` character is the minimum **one-dimensional
unitary representation class** able to distinguish directed translations.
Without a supplied axis, its label has three compact components. If a polar
axis is independently present, one compact phase along that axis suffices.

This is not a claim that a new production field must be added. Existing
current, remainder/velocity, flux/wave, or defect data might realize the
character. That identification must be derived and tested.

---

## 7. Why internal `i` is not yet spatial momentum

The internal C4 phase obeys `i^2=-1` and distinguishes temporal rotation
sense. But it is a spatial scalar. The translation label `k` in equation
(24) is a polar covector under the cubic group. No scalar-only equivariant
map selects a nonzero polar vector from isotropic rest.

To direct motion, the internal phase must therefore be paired with something
spatial, for example:

\[
 \text{internal C4 orientation}
 \quad+\quad
 \text{polar body axis/current}
 \quad\longrightarrow\quad
 \text{directed translation character}.              \tag{28}
\]

Equation (28) is a type requirement, not a derived gearbox. It explains why
the stable recursive internal clock can exist before translational mobility:
internal clockwise/counterclockwise memory and external forward/backward
momentum transform differently.

---

## 8. Bloch and recoil boundary

The native character parameter is

\[
 k\in\widehat{\mathbb Z^3}=\mathbb T^3.               \tag{29}
\]

It is exact only modulo reciprocal-lattice periods. FTD-0896 proves there is
no globally continuous additive section from `T^3` to `R^3`; a real-valued
lift requires a branch or integer winding history plus an independent scale.
FTD-0897 proves that a carry variable can maintain such a lift after an
opposite increment has been supplied. It does not derive that increment,
its energy, or the substrate sector that owns it.

The four distinct objects are therefore:

1. `Dbar(d)`: even field-dressing energy;
2. `chi_k(d)`: directed compact translation phase;
3. lifted `p_* (k+2 pi w)`: selected unwrapped momentum candidate;
4. a common action: the missing dynamics that must generate equal/opposite
   updates and pay their energy.

Neither item 1 nor item 2 implies items 3 or 4. In particular,

\[
 \boxed{
 \text{the C4 wake metric does not determine vector recoil}.}          \tag{30}
\]

---

## 9. What is now closed and open

### Closed

1. `[THEOREM]` the phase-averaged C4 translation wake is the square norm of
   an exact `Z^3` one-cocycle.
2. `[THEOREM]` its square root is a metric on the nondegenerate compact-source
   translation orbit.
3. `[THEOREM]` the wake is conditionally negative definite and has the exact
   polarization identity (18).
4. `[SCOPED NO-GO]` the positive even wake action cannot select a nonzero
   directed hop or recoil from isotropic rest.
5. `[THEOREM]` the lost orientation is precisely the conjugation sign of a
   `Z^3` character.
6. `[THEOREM]` every one-dimensional unitary directed translation phase is
   labeled by the Bloch torus `T^3`.

### Open

1. which existing substrate observable, if any, realizes the directed
   character for the formed C4 body;
2. a dynamic common action coupling that character, source worldline/current,
   and field wake;
3. the origin of equal/opposite impulse, carry ownership, and physical scale;
4. unwrapped total momentum and vector recoil;
5. autonomous hop selection and slow/exceptional mobility;
6. source formation, universal ternary closure, recovery, attraction, and
   collision composition;
7. production integration, physical L/R identity, critical-quartic `G*`
   synchronization, gamma, Lorentz hiding, Born, Bell, and actualization.

---

## 10. Reproduction

```bash
python scripts/proofs/proof_c4_dressing_translation_cocycle_directed_recoil_state_necessity.py
```

Expected terminal summary:

```text
FTD-0934 exact certificate: 279/279 checks passed
OUTCOME=A_NEGATIVE_TYPE_DRESSING_GEOMETRY_DIRECTED_STATE_NECESSITY
DRESSING_COCYCLE=b(d)=pi(d)Y-Y
PHASE_AVERAGED_WAKE=norm(b(d))^2
WAKE_NEGATIVE_TYPE=TRUE
SQUARE_ROOT_WAKE_TRANSLATION_METRIC=TRUE
EVEN_WAKE_SELECTS_DIRECTED_HOP=FALSE
LOST_INFORMATION=CHARACTER_CONJUGATION_SIGN
MINIMUM_DIRECTED_REPRESENTATION=NONTRIVIAL_Z3_CHARACTER
NATIVE_DIRECTED_STATE_DOMAIN=BLOCH_TORUS_T3
UNWRAPPED_PHYSICAL_MOMENTUM=OPEN
DYNAMIC_COMMON_ACTION_VECTOR_RECOIL=OPEN
PRODUCTION_CHANGED=FALSE
GSTAR_USED=FALSE
BORN_BELL_CONTEXT_USED=FALSE
```

No engine source, CMake target, `Voxel` field, toggle, default, production
law, type, import, paper, physical constant, or phenomenological formula was
changed. No numerical search, fit, near-miss, formula-substitution discovery,
completed-infinity claim, or `L to infinity` argument was used.
