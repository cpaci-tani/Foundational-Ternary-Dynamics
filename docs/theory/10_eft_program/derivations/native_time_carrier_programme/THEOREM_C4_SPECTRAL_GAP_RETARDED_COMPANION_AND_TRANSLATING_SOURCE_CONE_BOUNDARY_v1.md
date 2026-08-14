# FTD-0932 — C4 spectral-gap retarded companion and translating-source cone boundary v1

**Identifier:** `FTD-0932`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — UNIQUE GAPPED C4 QUASILOCAL COMPANION]` +
`[THEOREM — CAUSAL LOCAL C4 PHASE TRACKING]` +
`[THEOREM — POSITIVE MISMATCH ENERGY / ZERO STEADY FOUR-CYCLE WORK]` +
`[THEOREM — PRIMITIVE MOORE-TRANSLATION RESONANCE CONE]` +
`[SCOPED NO-GO — GENERIC FINITE-ENERGY RIGID CO-MOVING HALO]` +
`[OPEN — SOURCE FORMATION / RECIPROCAL TRANSLATIONAL RECOIL / PRODUCTION]`  
**Protocol:**
[`PREREG_C4_SPECTRAL_GAP_RETARDED_COMPANION_AND_TRANSLATING_SOURCE_CONE_BOUNDARY_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_C4_SPECTRAL_GAP_RETARDED_COMPANION_AND_TRANSLATING_SOURCE_CONE_BOUNDARY_v1.md),
SHA-256 `0F25E339C6C8AC0BAA122E78FA985BDD4B42FA39098EEC13BF2489AB1240FCFD`  
**Certificate:**
[`proof_c4_spectral_gap_retarded_companion_translating_source_cone_boundary.py`](../../../../../scripts/proofs/proof_c4_spectral_gap_retarded_companion_translating_source_cone_boundary.py),
SHA-256 `3E5D4D606DE828F63478CA6E5DA3181FDFA5F30DB5208F15B833FBBF2A972049`,
`259/259` exact checks  
**Registered outcome:** `A — C4 RETARDED TRACKING / TRANSLATING-SOURCE CONE BOUNDARY`

---

## 1. Result

The fixed-center `C4` companion left open by FTD-0929 and FTD-0931 has a
natural causal formation mechanism. It is again the existing native
`(flux,wave_vel)` pair, but now driven at the exact quarter-turn temporal
phase.

Let

\[
 W_{n+1}=W_n-KJ_n+f_n,
 \qquad
 J_{n+1}=J_n+W_{n+1},                                   \tag{1}
\]

where `K` is the production-normalized C18 stiffness and the compact source
obeys

\[
 \boxed{f_{n+2}=-f_n.}                                  \tag{2}
\]

Equation (2) is the real two-arm expression of multiplication by `i`. The
unique four-periodic particular field is

\[
 \boxed{
 q_n=-(2I-K)^{-1}f_n,
 \qquad
 p_n=q_n-q_{n-1}.}                                      \tag{3}
\]

Starting from zero field, equation (1) forms equation (3) retardedly: every
finite tick is local and finitely supported, while at every fixed site

\[
 \boxed{
 J_n(x)-q_n(x)\longrightarrow0,
 \qquad
 W_n(x)-p_n(x)\longrightarrow0.}                        \tag{4}
\]

The overwritten mismatch is not destroyed. It remains in an exact positive
homogeneous-field invariant and disperses outside every fixed finite region.

This does **not** extend to primitive rigid translation. If

\[
 f_n(x)=f_0(x-nu),
 \qquad u\in\{-1,0,1\}^3\setminus\{0\},                \tag{5}
\]

then every one of the 26 Moore steps intersects the native wave band along a
codimension-one infrared resonance cone. A generic compact source therefore
has no finite-energy exact co-moving halo. Internal quarter-turn recurrence
and translational mobility are mathematically different operations.

---

## 2. Why `i` is special in the native field map

The exact C18 symbol is

\[
 \kappa(k)={4\over3}-{2\over9}
 (u+v+w+uv+uw+vw),
 \qquad
 0\le\kappa\le{16\over9}.                              \tag{6}
\]

For a temporal drive `z=exp(-i Omega)`, the driven pole of the native
kick-drift map is

\[
 {\det(zI-U_\kappa)\over z}
 =\kappa-4\sin^2(\Omega/2).                             \tag{7}
\]

At the quarter turn,

\[
 z=i,
 \qquad z^2=-1,
 \qquad \Omega={\pi\over2},                             \tag{8}
\]

and equation (7) becomes

\[
 \kappa-2=-(2-\kappa).                                  \tag{9}
\]

The C18 band ends at `16/9`, so

\[
 \boxed{
 2-\kappa\ge{2\over9}.}                                \tag{10}
\]

Equivalently, the free wave frequencies satisfy

\[
 0\le\omega\le\arccos(1/9)<\pi/2.                    \tag{11}
\]

Thus `z=i` lies outside the entire propagating band. It selects an evanescent,
gapped spatial response rather than a resonant wave. This is the exact
dynamical content supplied by `i` in this sector:

\[
 \boxed{i\quad\longmapsto\quad 2I-K
 \quad\longmapsto\quad \text{gap }2/9.}                \tag{12}
\]

Equation (12) is not a derivation of a physical Lorentz factor `gamma`, a
`G*` clock, complex quantum amplitudes, or Born weights. It is a theorem about
the frozen discrete field pole.

---

## 3. Unique quasilocal companion

Because of equation (10), `A=2I-K` is positive and invertible with

\[
 {2\over9}I\le A\le2I,
 \qquad
 \|A^{-1}\|\le{9\over2}.                               \tag{13}
\]

Equation (2) implies `q_{n+1}+q_{n-1}=0`. Substitution into the second-order
form of equation (1) gives

\[
 q_{n+1}-(2I-K)q_n+q_{n-1}
 =-(2I-K)q_n=f_n,                                       \tag{14}
\]

which proves equation (3). Invertibility proves uniqueness within the
four-periodic finite-energy class.

The companion is quasilocal:

\[
 \boxed{
 q_n=-{1\over2}\sum_{m=0}^{\infty}(K/2)^m f_n.}         \tag{15}
\]

Each term expands support by at most one C18 radius. The `N`-term local
preparation has the exact norm bound

\[
 \boxed{
 \|q_n-q_n^{(N)}\|
 \le{9\over2}\left({8\over9}\right)^N\|f_n\|.}        \tag{16}
\]

Equation (15) reproduces FTD-0929's resolvent as an analytic identity. The
formation dynamics does not evaluate it. Equation (1) reads only the current
compact source and builds the profile through its causal history.

---

## 4. Positive mismatch energy and radiative formation

Set

\[
 e_n=J_n-q_n,
 \qquad
 z_n=W_n-p_n.                                           \tag{17}
\]

The source cancels exactly, leaving

\[
 z_{n+1}=z_n-Ke_n,
 \qquad
 e_{n+1}=e_n+z_{n+1}.                                   \tag{18}
\]

The native positive-band invariant is

\[
 \boxed{
 H_{C4}(e,z)
 ={1\over2}\langle z,z\rangle
 +{1\over2}\langle e,Ke\rangle
 -{1\over2}\langle z,Ke\rangle}                       \tag{19}
\]

with factorization

\[
 H_{C4}
 ={1\over2}\|z-Ke/2\|^2
 +{1\over2}\langle e,K(I-K/4)e\rangle.                \tag{20}
\]

Every nonzero C18 mode lies in `0<kappa<4`; the finite-energy uncontained
class has no nonzero square-summable constant mode. Hence the switch-on
mismatch from zero field has finite strictly positive energy for nonzero
nondegenerate compact source data. This is the formation-history debit, not
the source reservoir that pays it.

The exact free error is

\[
 \widehat e_n(k)=
 {\sin[(n+1)\omega]\widehat e_0(k)
  -\sin(n\omega)\widehat e_{-1}(k)\over\sin\omega}.     \tag{21}
\]

The companion amplitudes are bounded by the spectral gap. At the only
massless point,

\[
 \sin\omega(k)=O(|k|),                                  \tag{22}
\]

so equation (21) has at worst `1/|k|` amplitude. Three-dimensional measure
makes this locally integrable. Away from the origin the nonconstant analytic
dispersion has a measure-zero critical set. Coarea followed by the
Riemann--Lebesgue lemma removes both oscillatory branches at each fixed site,
proving equation (4).

At every finite tick, direct iteration of equation (1) from compact source
arms has finite C18-cone support. Therefore the proof does not install a
completed profile or use an `L to infinity` extrapolation. The Fourier torus
is scaffolding for the epsilon-local statement on the uncontained substrate.

Finite grounded fields behave differently. Their nonzero mismatch invariant
prevents instantaneous convergence; the error is recurrent or
quasiperiodic. Its Cesaro average converges modewise. No region-independent
rate is claimed.

---

## 5. Steady C4 work

FTD-0576 proves the exact driven-field work identity

\[
 H_{n+1}-H_n
 =\left\langle f_n,{W_n+W_{n+1}\over2}\right\rangle.   \tag{23}
\]

On the exact companion,

\[
 {p_n+p_{n+1}\over2}=q_{n+1},                           \tag{24}
\]

and the step work is

\[
 w_n=\langle f_n,q_{n+1}\rangle.                       \tag{25}
\]

Since `(2I-K)^{-1}` is symmetric and the source is antipodal after two
ticks,

\[
 \boxed{w_0+w_1+w_2+w_3=0.}                            \tag{26}
\]

If the two arms obey `f_1=Rf_0` for a skew-adjoint orthogonal complex
structure with `[R,K]=0`, then

\[
 \langle f_0,(2I-K)^{-1}Rf_0\rangle=0,                 \tag{27}
\]

and every step work vanishes separately.

The formed fixed-center companion therefore needs no **secular** energy
input. Generic arms can exchange energy during the cycle, but return it after
four ticks. Equations (26)--(27) do not pay the initial formation debit or
derive an autonomous source oscillator.

---

## 6. Why primitive translation is different

For the rigidly translating source in equation (5), a co-moving profile would
have the same phase `z=exp(-ik dot u)` at each spatial momentum. Equation (7)
therefore becomes

\[
 \boxed{
 D_u(k)\widehat q_0(k)=\widehat f_0(k),
 \qquad
 D_u(k)=\kappa(k)-4\sin^2(k\cdot u/2).}                \tag{28}
\]

Near `k=0`,

\[
 \boxed{
 D_u(k)={|k|^2\over3}-(k\cdot u)^2+O(|k|^4).}          \tag{29}
\]

The quadratic matrix is

\[
 B_u={1\over3}I-uu^T,                                   \tag{30}
\]

with spectrum

\[
 \operatorname{spec}(B_u)
 =\left\{{1\over3},{1\over3},{1\over3}-|u|^2\right\}. \tag{31}
\]

Every nonzero Moore step has `|u|^2` equal to `1`, `2`, or `3`. Hence
equation (31) has signature `(+, +, -)` on all 26 arms. The denominator is
positive in directions perpendicular to motion and negative parallel to
motion. Its nondegenerate quadratic cone persists under the analytic
higher-order terms as smooth codimension-one zero sheets accumulating at the
origin.

For a compact source with nonzero total, `fhat_0(0) != 0`, so its numerator
remains nonzero on sufficiently small pieces of those sheets. In a normal
coordinate `s`, equation (28) has the local form

\[
 \widehat q_0\sim{c\over s}.                            \tag{32}
\]

The energy norm contains

\[
 \int_{-\epsilon}^{\epsilon}{ds\over s^2},             \tag{33}
\]

which diverges. Therefore

\[
 \boxed{
 \text{a generic compact primitive translating source has no exact
 finite-energy rigid co-moving companion.}}             \tag{34}
\]

The retarded field can carry a wake, but this theorem derives no radiated
power. A neutral source is not automatically exempt: its numerator must
vanish on the entire resonance sheet, not merely at its apex. Exceptional
cone-canceling profiles remain open.

This no-go is scoped. It does not cover a sufficiently slow smooth drive,
consistent with FTD-0558's positive no-resonance speed floor. It also does not
cover subvoxel interpolation, deforming matter, recoil-induced slowing,
nonrigid carriers, or nonlinear dispersion.

---

## 7. Mechanics consequence

FTD-0932 separates two notions that had been bundled under “moving source”:

1. **internal phase motion:** a fixed-center source recursively changes arm
   by `i`; because `pi/2` lies above the native wave band, it supports a
   stable evanescent companion and causal local tracking;
2. **spatial translation:** a one-site source motion Doppler-locks to the
   field spectrum; because its speed exceeds the infrared wave speed in some
   direction, it intersects the band and cannot generically retain a rigid
   finite-energy halo.

This explains why a source can possess stable recursive “left/right” or
quadrature dynamics without yet being a freely translating particle. The
companion is more than a static matter label: it is maintained field history.
But spatial motion now requires an actual reciprocal gearbox:

- radiation/wake energy must enter the exact FTD-0576 work ledger;
- the source must slow, recoil, deform, or draw on a reservoir;
- the current and matter update must follow from one common action or be
  declared as selected dynamics.

Freezing the source trajectory avoids that problem only by making the source
external. It is not isolated mechanics.

---

## 8. What is closed and what remains open

### Closed

1. `[THEOREM]` `z=i` lies in an exact `2/9` gap above the native C18 wave
   band.
2. `[THEOREM]` the fixed-center `C4` source has the unique companion
   `q_n=-(2I-K)^{-1}f_n`.
3. `[THEOREM]` the companion is quasilocal with tail bound (16).
4. `[THEOREM]` the existing native field pair forms and tracks it causally at
   every fixed site in three dimensions.
5. `[THEOREM]` the outgoing mismatch retains an exact positive invariant.
6. `[THEOREM]` the steady companion has zero net work per four-cycle; a skew
   commuting `C4` doublet has zero work every step.
7. `[THEOREM]` all 26 primitive Moore translations possess an infrared
   resonance cone.
8. `[SCOPED NO-GO]` a generic compact nonzero-total source has no exact
   finite-energy rigid co-moving halo at one lattice step per tick.

### Open

1. formation and autonomous maintenance of the compact source itself;
2. a reciprocal translational matter update and exact recoil;
3. exceptional neutral or cone-canceling moving profiles;
4. slow, subvoxel, deforming, or nonlinear mobile carriers;
5. a common source-field action including the source's kinetic/internal
   energy;
6. universal ternary closure, attraction, recovery, and collision
   composition;
7. production coupling and operational hiding;
8. any identification with `G*`, a physical clock cadence, photons, gravity,
   dark matter, Born frequencies, Bell correlations, or actualization.

---

## 9. Reproduction

```bash
python scripts/proofs/proof_c4_spectral_gap_retarded_companion_translating_source_cone_boundary.py
```

Expected terminal summary:

```text
FTD-0932 exact certificate: 259/259 checks passed
OUTCOME=A_C4_RETARDED_TRACKING_TRANSLATING_SOURCE_CONE_BOUNDARY
C4_TEMPORAL_PHASE=z=i
C4_SPECTRAL_GAP=2/9
C4_COMPANION=-(2I-K)^-1 f_n
C4_COMPANION_UNIQUE_AND_QUASILOCAL=TRUE
C4_FORMATION_UPDATE=TARGET_BLIND_CAUSAL_NATIVE_FIELD
C4_LOCAL_PHASE_TRACKING=YES
C4_MISMATCH_ENERGY=POSITIVE_AND_EXACTLY_CONSERVED
C4_STEADY_FOUR_CYCLE_WORK=ZERO
PRIMITIVE_TRANSLATION_DENOMINATOR=kappa-4sin^2(k.u/2)
PRIMITIVE_TRANSLATION_RESONANCE_CONE=ALL_26_MOORE_STEPS
GENERIC_FINITE_ENERGY_COMOVING_HALO=NO
TRANSLATIONAL_RECOIL_COMMON_ACTION=OPEN
SOURCE_FORMATION_RESERVOIR=OPEN
PRODUCTION_CHANGED=FALSE
GSTAR_USED=FALSE
BORN_BELL_CONTEXT_USED=FALSE
```

No engine source, CMake target, `Voxel` field, toggle, default, production
law, type, import, paper, physical constant, or phenomenological formula was
changed. No damping, numerical search, fit, near-miss, formula-substitution
discovery, completed-infinity claim, or `L to infinity` argument was used.
