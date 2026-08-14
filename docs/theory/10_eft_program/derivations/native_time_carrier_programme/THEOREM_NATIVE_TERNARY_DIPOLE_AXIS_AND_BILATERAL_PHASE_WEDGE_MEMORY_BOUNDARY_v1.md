# Theorem — Native ternary dipole axis and bilateral phase-wedge memory boundary v1

**Identifiers:** `FTD-0905`, `FTD-0906`, `FTD-0907`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — NATIVE-TYPE POLAR AXIS REPRESENTABILITY]` +
`[THEOREM — NATIVE-TYPE TIME-ODD PHASE-WEDGE REPRESENTABILITY]` +
`[THEOREM — CONDITIONAL CENTRAL QUARTIC ORIENTATION MEMORY]` +
`[BOUNDARY — SEPARATE G* CLOCK AND CHIRALITY-MEMORY MODES REQUIRED]` +
`[OPEN — PRODUCTION FORMATION, MAINTENANCE, AND ERASURE]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]`

## 1. Native polar axis from a neutral ternary dipole

For a finite region `Lambda` of the existing cubic substrate, define

\[
S_\Lambda=\sum_{x\in\Lambda}s_x,
\qquad
d_\Lambda(r)=\sum_{x\in\Lambda}s_x(x-r).              \tag{1}
\]

If `S_Lambda=0`, then

\[
d_\Lambda(r')-d_\Lambda(r)=-(r'-r)S_\Lambda=0.        \tag{2}
\]

Thus the neutral ternary dipole is origin independent. Under a signed cubic
transformation `Q` followed by a translation `a`,

\[
d_\Lambda\longmapsto
\sum_xs_x(Qx+a-r')=Qd_\Lambda                         \tag{3}
\]

when the reference origin is transformed with the configuration. Whenever
`d_Lambda != 0`, the unit vector

\[
e=\frac{d_\Lambda}{|d_\Lambda|}                       \tag{4}
\]

is therefore a native polar axis. It uses only the already-registered
ternary states and site positions. The minimum nonzero neutral support is
one `+1` site and one `-1` site, for which

\[
d_\Lambda=x_+-x_-.                                    \tag{5}
\]

Spatial inversion reverses `e`. The symmetric square `d_Lambda tensor
d_Lambda` is unchanged and therefore loses precisely this sign.

This proves native-type representability, not spontaneous production or
stable persistence of a nonzero dipole.

## 2. Bilateral phase wedge

At the two ternary endpoints, use the existing flux `J` and wave-velocity
`W` fields. Project them onto (4):

\[
q_\pm=e\cdot J_\pm,
\qquad
p_\pm=e\cdot W_\pm.                                   \tag{6}
\]

The projected quantities are spatial scalars when the fields and axis are
transformed together. Define the bilateral antisymmetric phase wedge

\[
\ell=q_+p_- - q_-p_+,
\qquad
\chi=\operatorname{sgn}(\ell)\quad(\ell\ne0).         \tag{7}
\]

Under a signed cubic transformation, including inversion, `ell` is a
scalar. Under canonical time reversal,

\[
(q_+,q_-,p_+,p_-)
\longmapsto(q_+,q_-,-p_+,-p_-),
\qquad \ell\longmapsto-\ell.                          \tag{8}
\]

Hence `chi` is the time-odd clockwise/counterclockwise bit required by the
FTD-0904 branch-paired rectifier. It is carried by existing native field
types; no new selected state type is introduced.

For `z_+=(q_+,p_+)` and `z_-=(q_-,p_-)`, their Gram matrix obeys

\[
\det\operatorname{Gram}(z_+,z_-)=\ell^2.              \tag{9}
\]

Symmetric Gram or square data therefore retains the magnitude but loses the
sign of `ell`, exactly as the BCC symmetric square loses orientation.

## 3. Correction to the one-step swept-area interpretation

The FTD-0840 one-step quantity constructed from one canonical pair changes
sign under endpoint exchange alone. It does **not** change sign under full
canonical time reversal, because time reversal exchanges the endpoints and
reverses the momentum simultaneously. The compiled witness verifies

\[
\mathcal A(q_1,-p_1;q_0,-p_0)
=\mathcal A(q_0,p_0;q_1,p_1).                          \tag{10}
\]

That swept area records orientation relative to an ordered update, but it
is not by itself a stored time-odd branch. Equation (7) is the minimum
bilateral replacement in the registered native field class.

## 4. Conditional stable recursive memory

To test whether nonzero `ell` can be retained by a bounded recursive system,
adopt the **[IMPOSED reference memory law]**

\[
H_\chi=\frac{p_+^2+p_-^2}{2\mu}
       +\kappa(q_+^2+q_-^2)^2,
\qquad \mu,\kappa>0.                                  \tag{11}
\]

Rotational symmetry in the internal `(q_+,q_-)` plane gives

\[
\dot\ell=0.                                           \tag{12}
\]

Writing `rho^2=q_+^2+q_-^2`, the radial effective potential is

\[
V_{\rm eff}(\rho)
=\frac{\ell^2}{2\mu\rho^2}+\kappa\rho^4.             \tag{13}
\]

For every `ell != 0`, it has a strict positive minimum

\[
\rho_0^6=\frac{\ell^2}{4\mu\kappa},
\qquad
V_{\rm eff}''(\rho_0)=24\kappa\rho_0^2>0.            \tag{14}
\]

Thus a nonzero bilateral wedge can be a bounded, stable, recursive
orientation memory conditional on (11). The theorem does not claim that
the production tick creates (11), prepares `ell != 0`, compensates losses,
or books maintenance and erasure work.

## 5. Exact clock-memory separation

The critical G* clock is the one-dimensional radial quartic at zero angular
momentum. A nonzero stored wedge necessarily introduces the centrifugal
term in (13). Therefore the same central mode cannot simultaneously carry
`ell != 0` and retain the exact pure-quartic clock law

\[
TA=\sqrt\pi G^*\sqrt{\frac{m}{2\lambda}}.             \tag{15}
\]

Within this registered central class the minimum honest architecture has
two roles:

1. a critical radial quartic mode supplying the exact G* period factor; and
2. a bilateral phase-wedge mode supplying the persistent time-odd branch.

Both roles are representable with existing native field types, but their
dynamics and coupling are separate. G* sets clock traversal; `chi` selects
orientation. Neither derives the magnitude of the FTD-0904 coupling
`gamma`.

## 6. Epistemic accounting

Theorem-grade in the registered finite-region and central-memory classes:

- neutral-dipole origin independence and signed-cubic covariance;
- minimum `+/-` support for a nonzero neutral dipole;
- native polar-axis representability and symmetric-square sign loss;
- spatial-scalar and time-odd transformation of the phase wedge;
- Gram-determinant loss of the wedge sign;
- the correction that one-step swept area is time-reversal even under the
  complete canonical reversal;
- exact conservation of `ell` under the imposed central law (11);
- the strict bounded radial minimum (14); and
- the obstruction to using one nonzero-wedge central mode as the exact pure
  G* radial clock.

Still open:

- production-native formation of a neutral nonzero ternary dipole;
- production-native formation of `ell != 0` without target reading;
- persistence under the actual lossy tick, perturbations, and transport;
- work, dissipation, and information accounting for maintenance and erasure;
- derivation of the central memory law and its coupling to the FTD-0904
  rectifier;
- derivation and normalization of `gamma`;
- physical momentum scale, absolute mass, and finite-tick G* cadence;
- operational hiding, Born recovery, Bell closure, and Lorentz recovery; and
- any production integration.

This result supersedes the narrower FTD-0904 concern that existing native
types might be unable to represent `(e,chi)`. It does not supersede
FTD-0904's formation, maintenance, or erasure debt.

## 7. Certificate provenance

The FTD-0905 parent protocol SHA-256 is
`6FC0C2BAB8A84378F3B88618BA41E16B4C328AFF497446A2A4542990AA20CA4E`.
The parent certificate SHA-256 is
`FAA3CD3635C048AAD95E312AE59D6B725444C7C55571A0913A864F8AC8E038F0`.
Its first immutable execution passed `74/75`; only one exact prose-source
marker failed. All mathematical, type, symmetry, parity, memory, clock, and
scope gates passed. No theorem was issued from that execution.

The FTD-0906 repair protocol SHA-256 is
`F3758EECECACFD92CB35DFD501868F0C72CE3AAA7ADB77AA8826029B2C1F1340`.
The FTD-0906 wrapper SHA-256 is
`4608E92745BCB047AA18BBB8B5EE8DDB7C825E9D2B4DCD0A2148F7B0EBD53E8B`.
It passed repair-integrity checks but inherited the same false source
marker, so FTD-0906 remained invalid and issued no theorem.

The proof of record is the FTD-0907 exact source-marker repair. Its protocol
SHA-256 is
`E6B1B158B525D83036D1C78AB68AC5435542C10E60999EF399AF580A3376EE96`
and its wrapper SHA-256 is
`53F95DCB14F53A10E50940EB6EFC1A06D51B8CE5BBBC155294CA323BCFAFC8D9`.
It changes exactly the failed in-memory source marker to the normalized
phrase actually present in FTD-0840. Repair integrity, inherited integrity,
and the full exact certificate pass `75/75`.

## 8. Isolated reference implementation

The fail-closed analyzer is isolated under `ftd::eft`:

- `engine/include/ftd/eft/native_ternary_dipole_phase_wedge_memory.h`,
  SHA-256
  `BADAE9D26E5FED6FCD4317A7534648256AFF051E2CAADB7E6BEEA00603AEDF46`;
- `engine/src/eft/native_ternary_dipole_phase_wedge_memory.cpp`, SHA-256
  `AA021926D1DE32AE9D04FB72682379DBB7F6CD3A1BB150AADBA6A957DFBF20B5`;
- `engine/tests/test_native_ternary_dipole_phase_wedge_memory.cpp`, SHA-256
  `C4DC5CC9E52180F26CB92093F37D1D4AB619975F6E9191FF943383A4362D4BF6`.

The pinned MSVC 14.44 build succeeds. The focused Release CTest passes `1/1`
and the actualization/EFT chain passes `30/30`. No production `Voxel`,
renderer, boundary, default toggle, or tick phase was changed.

## 9. Next acceptance gate

Pre-register a production-observational formation and maintenance campaign.
It must not alter the production tick before the observation pass. At
minimum it must:

1. specify local neutral-region detection without outcome or context
   conditioning;
2. measure the frequency and lifetime of `d_Lambda != 0` and `ell != 0` in
   preregistered production ensembles;
3. test signed-cubic, inversion-paired, time-reversed, randomized, and
   harmonic controls;
4. distinguish transient kinematics from persistent recursive memory;
5. book production energy flow, dissipation, transport, and erasure; and
6. remain blind to G*, Born targets, measurement settings, and outcomes.

If the production tick never forms and retains the two observables without
target-coded intervention, the correct result is native-type
representability without a native formation mechanism.

```text
NATIVE_NEUTRAL_TERNARY_DIPOLE_AXIS=REPRESENTABLE_CONDITIONALLY
DIPOLE_ORIGIN_INDEPENDENCE=EXACT_WHEN_TOTAL_STATE_ZERO
MINIMUM_NONZERO_NEUTRAL_SUPPORT=ONE_PLUS_ONE_MINUS
DIPOLE_SYMMETRIC_SQUARE_RETAINS_AXIS_SIGN=FALSE
BILATERAL_PROJECTED_PHASE_WEDGE=SPATIAL_SCALAR
BILATERAL_PROJECTED_PHASE_WEDGE=TIME_ODD
GRAM_DATA_RETAINS_WEDGE_SIGN=FALSE
ONE_STEP_SWEPT_AREA_IS_TIME_ODD_MEMORY=FALSE
CENTRAL_QUARTIC_WEDGE_CONSERVATION=EXACT_CONDITIONAL
NONZERO_WEDGE_BOUNDED_RECURSIVE_MEMORY=EXACT_CONDITIONAL
SAME_CENTRAL_MODE_IS_PURE_GSTAR_CLOCK_AND_NONZERO_WEDGE_MEMORY=FALSE
SEPARATE_GSTAR_CLOCK_AND_CHIRALITY_MEMORY=REQUIRED_IN_REGISTERED_CLASS
EXISTING_NATIVE_FIELD_TYPES_SUFFICE_FOR_REPRESENTATION=TRUE
PRODUCTION_DIPOLE_FORMATION=OPEN
PRODUCTION_PHASE_WEDGE_FORMATION=OPEN
MAINTENANCE_ERASURE_WORK=OPEN
CENTRAL_MEMORY_LAW=IMPOSED_REFERENCE
GAMMA_MAGNITUDE_DERIVED=FALSE
INTEGER_TICK_GSTAR_CADENCE=OPEN
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_TYPE=TRUE
BORN_TARGET_USED=FALSE
BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```
