# Theorem — Local remainder–velocity `C4` Hamiltonian and formation boundary v1

**Identifier:** `FTD-0926`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — EXISTING-TYPE HOMOGENEOUS ONSITE C4 GENERATOR]` +
`[SELECTED REFERENCE HAMILTONIAN — POSITIVE QUADRATIC STORAGE]` +
`[THEOREM — EXACT PERIOD-FOUR REVERSIBLE STABILITY]` +
`[CONDITIONAL FORMATION LEDGER — POSITIVE CARRIER-PLUS-HALO DEBIT]` +
`[OPEN — TERNARY-RECORD GENERATOR/FIELD RECOIL/SPONTANEOUS FORMATION]`

## 1. Result

The FTD-0925 causal scaffold does not need a new current type, a hidden
site-phase table, or an additional conjugate variable to generate its four
registered velocities. Its existing subcell remainder and velocity already
form the required local phase-space doublet.

At each of the 20 scaffold sites, initialize

\[
 \boxed{r_0={v_0-v_1\over2}}.                               \tag{1}
\]

Then apply the same local rule to every site and Cartesian component:

\[
 \boxed{v'=v-2r},\qquad
 \boxed{r'=r+v'=v-r}.                                      \tag{2}
\]

Equation (2) reads only the onsite pair `(r,v)`. It does not read position,
scaffold sign, the desired arm, an external phase, `G*`, measurement context,
outcome, or Born weight. Starting from (1), it generates exactly

\[
 (v_0,v_1,-v_0,-v_1)
\]

and returns both remainder and velocity after four applications. Every
remainder component has magnitude at most `2/5`, while the velocity retains
the FTD-0925 peak speed squared `8/25<1/3`.

The rule is the exact quarter-period flow of a positive onsite quadratic
Hamiltonian. Therefore the missing velocity gearbox is closed at reference
level using existing types.

This does **not** yet make the full source–field body autonomous. The ternary
record snapshots remain prescribed, the production force law does not contain
equation (2), and no reciprocal common action has been shown to couple this
matter oscillator to the evanescent field orbit or to form the scaffold.

## 2. Why velocity alone was insufficient

At a fixed scaffold site the registered velocity sequence is

\[
 a_x,\ b_x,\ -a_x,\ -b_x.                                  \tag{3}
\]

Seven sites have linearly independent `a_x,b_x`. At the other twelve,
`b_x=\pm a_x`. The latter produce local sequences such as

\[
 (+,+,-,-),\qquad (+,-,-,+).
\]

Consequently identical onsite velocities occur with different successors.
No deterministic velocity-only update can generate the complete body.

FTD-0925 started its no-hop audit at `r=0`, but that was a reference initial
condition rather than an ontological requirement. Equation (1) stores the
missing phase in the already-existing subcell remainder. No additional
memory type is required.

The four remainder fields are

\[
\begin{array}{c|c}
n&r_n\\ \hline
0&(v_0-v_1)/2\\
1&(v_0+v_1)/2\\
2&(v_1-v_0)/2\\
3&-(v_0+v_1)/2.
\end{array}                                                 \tag{4}
\]

They distinguish every successor collision exposed by velocity alone.

## 3. Local symplectic map

For one scalar component write `z=(r,v)^T`. Equation (2) is

\[
 z'=\mathsf Mz,\qquad
 \mathsf M=
 \begin{pmatrix}-1&1\\-2&1\end{pmatrix}.                   \tag{5}
\]

With

\[
 \mathsf J=
 \begin{pmatrix}0&1\\-1&0\end{pmatrix},
\]

direct multiplication gives

\[
 \boxed{\mathsf M^2=-I},\qquad
 \boxed{\mathsf M^4=I},\qquad
 \det\mathsf M=1,                                          \tag{6}
\]

\[
 \boxed{\mathsf M^T\mathsf J\mathsf M=\mathsf J}.           \tag{7}
\]

The inverse is

\[
 \mathsf M^{-1}=-\mathsf M,
\]

so reversing the map reaches the prior registered arm exactly. This is an
orientation-preserving local canonical recurrence, not a lossy latch or
externally sequenced lookup table.

Equation (2) also has the force–movement form

\[
 v'=v-2r,\qquad r'=r+v'.                                   \tag{8}
\]

Thus its discrete ordering is compatible with reading the current from
`v_n`, updating the velocity, and then accumulating the new velocity into
the remainder. Production has that phase order, but not this force.

## 4. Positive Hamiltonian lift

Define

\[
 \mathsf G=
 \begin{pmatrix}2&-1\\-1&1\end{pmatrix}.                   \tag{9}
\]

Its exact eigenvalues are

\[
 {3-\sqrt5\over2},\qquad {3+\sqrt5\over2},
\]

so `\mathsf G` is positive definite. Moreover

\[
 \mathsf J\mathsf G=\mathsf M,\qquad
 \mathsf M^T\mathsf G\mathsf M=\mathsf G.                  \tag{10}
\]

The local positive quadratic is therefore

\[
 \mathcal E(r,v)
 =|r|^2-r\cdot v+\frac12|v|^2
 ={1\over2}z^T\mathsf Gz>0
\]

away from zero. Give every site the same selected Hamiltonian

\[
 \boxed{
 H_{\rm rv}
 =\omega\sum_x\left(
 |r_x|^2-r_x\cdot v_x+\frac12|v_x|^2\right)}.               \tag{11}
\]

Hamilton's equation is

\[
 \dot z_x=\omega\mathsf Mz_x.
\]

Because `\mathsf M^2=-I`,

\[
 \exp(\omega T\mathsf M)
 =\cos(\omega T)I+\sin(\omega T)\mathsf M.
\]

At

\[
 \omega T={\pi\over2},                                     \tag{12}
\]

the exact flow is equation (5). This is a time-independent positive
Hamiltonian lift of the discrete onsite map.

The canonical identification of physical remainder and velocity, the
quadratic (11), and the scale in (12) are selected reference structure. They
are not derived from the production action. The continuous interpolation is
stroboscopic hardware; it is not a claim that production obeys
`\dot r=v` between integer ticks.

## 5. Exact registered orbit

Let `a=v_0` and `b=v_1` as complete 20-site vector fields. FTD-0925 gives

\[
 \|a\|^2=\|b\|^2={104\over25},\qquad
 \langle a,b\rangle=0.                                    \tag{13}
\]

Starting from `((a-b)/2,a)`, equation (5) gives

\[
\begin{aligned}
 ((a-b)/2,a)&\mapsto((a+b)/2,b)\\
 &\mapsto((b-a)/2,-a)\\
 &\mapsto(-(a+b)/2,-b)\\
 &\mapsto((a-b)/2,a).
\end{aligned}                                              \tag{14}
\]

The certificate reconstructs all 20 sites independently and verifies (14)
component by component. At each source-read section the current remains

\[
 j_n=(h+s_n)v_n
\]

and obeys

\[
 (h+s_{n+1})-(h+s_n)+\operatorname{div}_c j_n=0.            \tag{15}
\]

Equation (15) proves that the generated velocity is exactly the
continuity-compatible FTD-0925 current. It does not cause `s_n` to change.
A local law that generates the ternary record transition remains required.

## 6. Exact energy and work

The initial phase remainder has

\[
 \|r_0\|^2={52\over25}.                                    \tag{16}
\]

On all four arms,

\[
 \boxed{\sum_x\mathcal E(r_n,v_n)={52\over25}}.             \tag{17}
\]

For unit-tick sampling, `T=1` and `\omega=\pi/2`, so

\[
 \boxed{H_{\rm rv}={26\pi\over25}}.                         \tag{18}
\]

For a general sampling time the invariant is `52\omega/25`. Equation (18)
is a selected reference normalization, not a measured or substrate-derived
energy.

The velocity norms are equal pointwise across every arm. Therefore

\[
 (v_{n+1}-v_n)\cdot{v_{n+1}+v_n\over2}
 ={1\over2}(|v_{n+1}|^2-|v_n|^2)=0                        \tag{19}
\]

at every site. Any isotropic speed-only matter dispersion has zero endpoint
kinetic change. This is exactly compatible with FTD-0925's zero scalar
matter-work ledger, including sites that reverse by `pi` and have zero
midpoint velocity.

Equation (19) is not a vector force derivation. It does not show that the
evanescent field supplies the impulse, that the matter oscillator supplies
the reciprocal field source, or that one common action generates both.

## 7. Formation ledger

The empty local carrier `(r,v)=(0,0)` has zero energy. Preparing the
registered carrier therefore requires the positive debit

\[
 \Delta E_{\rm rv}={26\pi\over25}
\]

at the unit-tick normalization. The static ternary scaffold also sources the
positive halo energy already booked by FTD-0925:

\[
 E_h={1\over2}\langle H,KH\rangle>0.
\]

Thus the modeled formation ledger has the lower subtotal

\[
 \boxed{\Delta E_{\rm modeled}=E_h+{26\pi\over25}>0}.       \tag{20}
\]

This prevents the recursive carrier from being treated as free. It is not a
formation mechanism. The energy of manifesting the 20 ternary sites, the
reservoir that supplies (20), localization work, and reciprocal recoil are
not yet defined.

## 8. Stability and recovery boundary

Equation (6) holds on the entire local phase space, not only on the
registered orbit. Hence every perturbation `delta z` satisfies

\[
 \mathsf M^4\delta z=\delta z,\qquad
 (\mathsf M\delta z)^T\mathsf G(\mathsf M\delta z)
 =\delta z^T\mathsf G\delta z.                             \tag{21}
\]

The carrier is neutrally stable: perturbations do not grow, and each
perturbed remainder–velocity state returns exactly after four maps.

Reversible neutral stability is not error correction. The map does not
attract a perturbed state back to the registered orbit, repair a changed
ternary record, re-form a lost scaffold site, or restore the field/source
phase relation after a coupled disturbance.

## 9. Production boundary

Production already contains both `Voxel::velocity` and `Voxel::remainder`.
Its tick reads `s v` before force integration and accumulates the updated
velocity into the remainder afterward. But its selected forces do not contain
`v'=v-2r`, and its relativistic momentum integrator is not the Hamiltonian
(11).

Accordingly FTD-0926 changes no engine source, CMake target, toggle, default,
force, current, import, or ontology type. It proves that an existing-type
local generator is mathematically available; it does not insert that
generator into production.

## 10. Next dynamics

The local velocity-generator debt is retired at reference level. The next
exact gate is a common local source–matter–field recurrence that:

1. generates `s_{n+1}` from `s_n` and the live current rather than prescribing
   the ternary snapshots;
2. derives the vector impulse in equation (2) from a local field or bond
   action;
3. gives the field the equal-and-opposite reciprocal reaction;
4. preserves the positive total energy including (20);
5. locks and recovers the relative phase of the matter and evanescent field
   doublets without reading the desired arm; and
6. identifies a reservoir and local transaction that forms the scaffold.

Only after that coupled source/body closes may a critical-quartic envelope
be attached and `G*` tested as its cadence.

## 11. Epistemic boundary

FTD-0926 derives the algebraic existing-type generator, its exact local
symplectic structure, positive invariant, reference orbit, no-hop bound,
zero endpoint kinetic work, and neutral stability. The Hamiltonian
identification and unit-tick scale are selected reference structure.

It does not derive production insertion, the ternary-record transition,
field-derived force, reciprocal recoil, common action, spontaneous formation,
asymptotic recovery, mobility, physical scale, critical quarticity, `G*`,
gamma, Born frequencies, Bell correlations, measurement context, or
preferred-tick hiding.

## 12. Verification

The locked preregistration has SHA-256
`BD98EA4CC0EF2B858BD2D8D504892468F5100DF462E5B169118BA6C39AFD6136`.

The exact certificate is
`scripts/proofs/proof_local_remainder_velocity_c4_hamiltonian_formation_ledger.py`,
SHA-256
`F2E53AA3180816AE0732663E6DC5180EFFE419C864B5310E0E400DFC6B81007E`.

It passes `106/106` gates and reports

```text
OUTCOME=A_EXACT_EXISTING_TYPE_LOCAL_REMAINDER_VELOCITY_GENERATOR
LOCAL_MAP=(r,v)->(v-r,v-2r)
LOCAL_MAP_SQUARED=-I
LOCAL_MAP_FOURTH_POWER=I
POSITIVE_QUADRATIC_STORAGE=52/25
UNIT_TICK_HAMILTONIAN=26*pi/25
REMAINDER_COMPONENT_MAX=2/5
VELOCITY_ORBIT=EXACT_ALL_FOUR_ARMS
LIVE_CONTINUITY=EXACT_ALL_FOUR_ARMS
ISOTROPIC_ENDPOINT_WORK=ZERO_POINTWISE
NEUTRAL_REVERSIBLE_STABILITY=EXACT_PERIOD_FOUR
NEW_ONTOLOGY_TYPE_ADOPTED=FALSE
PRODUCTION_FORCE_INSERTED=FALSE
FIELD_RECOIL_FORMATION_RECOVERY=OPEN
GSTAR_USED=FALSE
BORN_BELL_CONTEXT_USED=FALSE
```
