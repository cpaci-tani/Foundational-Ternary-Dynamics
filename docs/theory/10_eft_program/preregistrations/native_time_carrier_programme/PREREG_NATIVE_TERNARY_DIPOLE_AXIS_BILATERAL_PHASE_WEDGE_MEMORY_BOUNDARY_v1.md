# FTD-0905 — native ternary-dipole axis and bilateral phase-wedge memory boundary v1

**Identifier:** `FTD-0905`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Date:** 2026-08-11  
**Production status:** unchanged

## 1. Question

Can the existing ternary state, spatial incidence, flux, wave-velocity, and
ordered canonical data distinguish the polar axis and time-odd
clockwise/counterclockwise branch required by FTD-0904 without adopting a new
state type?

The registered minimum is a neutral bilateral region containing distinct
actual sites `x_+` and `x_-` with states `+1` and `-1`. Its actual-layer
dipole supplies a polar axis. Projecting each site's existing flux and wave
velocity onto that axis supplies two scalar canonical modes. Their
antisymmetric phase wedge supplies a time-odd chirality candidate.

This protocol distinguishes three claims:

1. the native types can **represent and read** the two signs conditionally;
2. one imposed central memory law can **retain** the chirality in an isolated
   reference model; and
3. production dynamics do **not yet form or maintain** that bilateral body or
   central memory law.

## 2. Frozen sources

| source | SHA256 |
|---|---|
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `THEOREM_NATIVE_PAIR_ENERGY_RECURSION_v1.md` | `C352EC96A6513D5ED3AB8A7318F47FD1A695FBB0C4FBEB33E9DE43680A70DF93` |
| `THEOREM_LOCAL_FLUX_SELF_PAIR_TENSOR_RECURSION_v1.md` | `62A95FF322C99773D03002444376B9244A93CC19D01CF4400230277288CADAEB` |
| `THEOREM_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_v1.md` | `64241D7AB18AD2079ECADF9EA25448F53F42696AB3FF439637970D4284497FD0` |
| `THEOREM_ORIENTED_EVEN_SELF_PAIR_RECTIFIER_AND_GSTAR_GEAR_RATIO_BOUNDARY_v1.md` | `E87EB15B482AFBBF1147726B3F07C4008B82BC07B06BD9786656BEA28AD3BDDA` |
| `engine/include/ftd/eft/native_pair_energy_recursion.h` | `81B4941B951BC9D680A862188310706B86CDDA9DF9550204FC3F3DD567371E5A` |

Any source-hash mismatch invalidates the certificate.

## 3. Actual-layer polar axis

For a finite region `Lambda`, define total ternary charge and dipole

\[
S_\Lambda=\sum_{x\in\Lambda}s_x,
\qquad
d_\Lambda(r)=\sum_{x\in\Lambda}s_x(x-r).               \tag{1}
\]

If `S_Lambda=0`, then `d_Lambda(r)` is independent of the reference origin
`r`. Under a signed cubic transform and translation `x -> Qx+a`,

\[
d_\Lambda\mapsto Qd_\Lambda.                           \tag{2}
\]

For `Q=-I`, the dipole reverses. Whenever `d_Lambda != 0`, define

\[
e=\frac{d_\Lambda}{|d_\Lambda|}.                       \tag{3}
\]

The minimum neutral nonzero configuration is one distinct `+/-` pair, for
which

\[
d=x_+-x_-.
\]

Equation (3) uses only existing ternary states and relational lattice
positions. It is undefined for a zero dipole and origin-dependent for a
nonneutral region; the implementation must fail closed in either case.

The symmetric square `d tensor d` is identical for `d` and `-d`, so it cannot
recover the polar sign.

## 4. Bilateral phase wedge

At the `+/-` sites, use the existing local fields and project them along the
dipole axis:

\[
q_\pm=e\cdot J_\pm,
\qquad
p_\pm=e\cdot W_\pm.                                    \tag{4}
\]

Define the antisymmetric phase wedge

\[
\ell=q_+p_- - q_-p_+,
\qquad
\chi=\operatorname{sgn}(\ell)\in\{-1,+1\}             \tag{5}
\]

when `ell != 0`.

Under signed cubic spatial transformations, `e`, `J`, and `W` transform as
polar vectors, so all four projected scalars and `ell` are invariant. Under
spatial inversion, both `e` and the vector fields reverse and `ell` remains a
scalar. Under time reversal, `W -> -W`, so

\[
\ell\mapsto-\ell,
\qquad \chi\mapsto-\chi.                               \tag{6}
\]

Thus (5) has exactly the spatial and temporal parity required by FTD-0904.
It uses the existing two sites and their existing continuous fields; it does
not add a new public state type.

Let `z_+=(q_+,p_+)` and `z_-=(q_-,p_-)`. Their Gram matrix determines
`ell^2` but not the sign of `ell`:

\[
\det\operatorname{Gram}(z_+,z_-)=\ell^2.               \tag{7}
\]

This is the precise bilateral information lost by a symmetric square.

## 5. One-step swept-area control

FTD-0840 proves that every nonzero forward discrete quartic step has one
strict swept-area sign. The certificate must also prove that this sign is not
the required `chi`: under full canonical time reversal, which exchanges the
endpoints and flips both endpoint momenta, the swept-area expression is
unchanged. It records orientation relative to the chosen update order, not a
time-odd branch stored in one instantaneous bilateral state.

Any theorem that identifies the FTD-0840 swept-area sign alone with (5) fails
this protocol.

## 6. Stable recursive reference memory

Let

\[
Q=(q_+,q_-),
\qquad P=(p_+,p_-),
\qquad \rho^2=|Q|^2,
\]

and adopt the isolated **[IMPOSED reference memory law]**

\[
H_\chi=\frac{|P|^2}{2\mu}+\kappa\rho^4,
\qquad \mu,\kappa>0.                                   \tag{8}
\]

Rotational invariance in the internal `(+,-)` plane gives

\[
\frac{d\ell}{dt}=0.                                    \tag{9}
\]

For `ell != 0`, the sign `chi` is therefore retained. The radial reduction is

\[
H_\chi=\frac{p_\rho^2}{2\mu}
 +\frac{\ell^2}{2\mu\rho^2}+\kappa\rho^4.             \tag{10}
\]

Its effective potential has one positive minimum satisfying

\[
\rho_0^6=\frac{\ell^2}{4\mu\kappa}.                   \tag{11}
\]

The nonzero wedge therefore supplies a bounded recursive orientation memory
in the imposed isolated model.

Equation (10) also freezes the separation boundary. The pure radial
critical-quartic `G*` traversal is the `ell=0` sector. A nonzero retained
chirality adds the centrifugal inverse-square term, so the same central mode
does not retain the exact one-dimensional FTD-0840/0904 `G*` period law.
The minimum honest architecture uses a critical quartic clock mode and a
distinct bilateral chirality-memory mode, even if both are built from the
same native field types.

## 7. Registered implementation witness

Only after a passing locked certificate, add an isolated `ftd::eft` analyzer
that accepts a finite set of ternary sites with positions, fluxes, and wave
velocities plus an explicitly selected `+/-` pair. It must report:

- neutrality and origin-independent dipole;
- the unit polar axis and signed-cubic/inversion covariance;
- the four projected canonical scalars;
- `ell`, `chi`, and their time-reversal parity;
- Gram determinant `ell^2` and the missing determinant sign;
- central-memory energy, angular-momentum conservation, and radial minimum;
- the one-step swept-area time-parity control;
- the same-mode `G*`/nonzero-`ell` incompatibility; and
- all production, formation, maintenance, controller-work, scale, mass,
  Born, and cadence firewalls.

It must fail closed on nonfinite data, nonternary state, nonneutral selected
region, missing or nonunique `+/-` endpoints, coincident endpoints, zero
dipole, zero phase wedge, nonpositive memory parameters/tolerance, or any
identity failure.

## 8. Outcome map

- **Outcome A:** the native-type axis and phase-wedge construction, symmetry
  parities, symmetric-square losses, central-memory conservation/stability,
  and separate clock/memory boundary all pass. Book representability and the
  conditional reference memory, while leaving physical formation and
  maintenance open.
- **Outcome B:** the axis is representable but no existing bilateral
  canonical data has the required time parity, or chirality retention cannot
  coexist with a positive bounded reference memory. Book the exact failed
  gate and prefer a separately adopted memory type.
- **Outcome C:** the native types cannot even represent the polar axis without
  an additional type. Book the exact obstruction and do not implement the
  FTD-0904 coupling.
- **Execution invalid:** any frozen hash, source marker, exact identity, or
  terminal firewall fails.

## 9. Scope firewall

```text
NEUTRAL_TERNARY_DIPOLE_SUPPLIES_POLAR_AXIS=CONDITIONAL_EXACT
NONZERO_DIPOLE_AND_PHASE_WEDGE_FORMATION=NOT_DERIVED
SIGNED_CUBIC_AND_INVERSION_COVARIANCE=EXACT
BILATERAL_PHASE_WEDGE_IS_SPATIAL_SCALAR=TRUE
BILATERAL_PHASE_WEDGE_IS_TIME_ODD=TRUE
SYMMETRIC_SQUARE_RETAINS_WEDGE_SIGN=FALSE
FTD0840_ONE_STEP_SWEPT_AREA_IS_TIME_ODD_MEMORY=FALSE
CENTRAL_QUARTIC_MEMORY_CONSERVES_PHASE_WEDGE=CONDITIONAL_EXACT
NONZERO_WEDGE_BOUNDED_RECURSIVE_MEMORY=CONDITIONAL_EXACT
SAME_MODE_NONZERO_WEDGE_RETAINS_PURE_GSTAR_RADIAL_CLOCK=FALSE
SEPARATE_CLOCK_AND_CHIRALITY_MEMORY_MINIMUM=TRUE_IN_REGISTERED_CLASS
PRODUCTION_BILATERAL_MEMORY_LAW=PRESENTLY_ABSENT
MEMORY_FORMATION_MAINTENANCE_ERASURE_WORK=OPEN
GAMMA_MAGNITUDE_DERIVED=FALSE
PHYSICAL_MOMENTUM_SCALE=OPEN
ABSOLUTE_MASS=NOT_DERIVED
INTEGER_TICK_GSTAR_CADENCE=OPEN
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_TYPE=TRUE
BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```

The exact SHA256 of this protocol and its certificate must be entered in the
preregistration manifest before first execution.
