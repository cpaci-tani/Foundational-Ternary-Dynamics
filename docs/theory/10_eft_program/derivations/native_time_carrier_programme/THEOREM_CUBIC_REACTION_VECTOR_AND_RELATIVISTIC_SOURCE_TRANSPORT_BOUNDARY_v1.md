# Theorem — Cubic reaction vector and relativistic source-transport boundary v1

**Identifier:** `FTD-0889` / repaired execution `FTD-0890`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — SCALAR-TO-VECTOR RECOIL FORBIDDEN BY CUBIC SYMMETRY]` +
`[THEOREM — THREE CANONICAL PAIRS MINIMUM IN THE REGISTERED ORIENTATION-FREE CLASS]` +
`[CONDITIONAL THEOREM — EXACT RELATIVISTIC COTANGENT CHART AND FREE TRANSPORT]` +
`[THEOREM — LOCAL ENERGY/MOMENTUM CONSERVATION FIXES THE SPLIT ANGLE]` +
`[CLOSED NEGATIVE — UNIVERSAL EQUAL SPLIT AND SCALAR-ONLY SPATIAL RECOIL]` +
`[SELECTION — RELATIVISTIC DISPERSION AND VECTOR REACTION ROLE]` +
`[IMPOSED — E0, c, SOURCE CHARGE, AND SOURCE INITIAL DATA]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]` +
`[OPEN — NATIVE VECTOR COMMON ACTION, MASS SCALE, STABLE SOURCE, PRODUCTION]`

## 1. Verdict

The scalar reaction pair of FTD-0888 cannot choose a spatial recoil direction.
That is not a missing coefficient: cubic symmetry forbids any nonzero
equivariant map from a scalar to the spatial vector representation. In the
registered onsite-direct-sum symplectic class, an orientation-free spatial
reaction carrier therefore requires three canonical pairs
`(R,Pi) in R^3 + R^3`. One scalar pair remains sufficient only after an
independent local field/current context supplies and holds a direction.

Conditional on the already selected relativistic dispersion, the resulting
vector reaction energy has an exact symplectic cotangent chart to physical
position and momentum. Its Hamiltonian drift is reversible, causal, and
energy preserving, and the existing face-current construction gives exact
endpoint continuity. This closes a reference source-transport gearbox; it
does not derive stable matter, the rest-energy scale, or a native common
action that creates the vector triplet.

The FTD-0888 equal split is also narrowed. Once the matched field transaction
supplies a required recoil impulse, local energy and momentum conservation
fix the split angle. Equal splitting occurs only on the special surface where
the requested kinetic energy is half the residual energy. Here “context” is
the local physical energy/momentum data, not a Bell measurement setting.

## 2. Cubic representation boundary

Let `O_h` act on a scalar `z` trivially and on `R^3` by signed permutation
matrices. If `F : R -> R^3` is equivariant, then for every `R in O_h`,

\[
F(z)=R F(z).                                                \tag{1}
\]

The common fixed subspace of the vector representation is `{0}`: independent
coordinate sign flips force every component to vanish. Hence

\[
\boxed{F(z)=0}                                             \tag{2}
\]

for every equivariant scalar-only map, linear or nonlinear. A scalar reaction
magnitude cannot supply its own spatial direction.

One vector copy `T_{1u}` is three-dimensional. Every alternating form on an
odd-dimensional real vector space is singular, so one vector copy cannot be
a nondegenerate canonical carrier. Two copies admit

\[
(R,\Pi)\in\mathbb R^3\oplus\mathbb R^3,\qquad
\omega=\sum_{i=1}^3 dR_i\wedge d\Pi_i.                    \tag{3}
\]

The diagonal `O_h` action preserves (3), and the form is nondegenerate.
Therefore three canonical pairs are minimum and sufficient in the registered
orientation-free onsite-direct-sum class.

This does not contradict FTD-0888. One pair remains minimum for a scalar
internal reaction channel. It also remains sufficient for a spatial reaction
restricted to a fixed one-dimensional submanifold when a unit direction is
supplied independently and held fixed during the gate.

## 3. Exact relativistic cotangent chart

Let the vector reaction carrier have ready energy

\[
E_{\rm react}=\frac12\lVert\Pi\rVert^2,                   \tag{4}
\]

and adopt the selected physical kinetic energy

\[
K(p)=\sqrt{E_0^2+c^2\lVert p\rVert^2}-E_0,
\qquad E_0>0,\quad c>0.                                   \tag{5}
\]

For `rho=|Pi|`, define

\[
a(\rho)=\frac{\sqrt{E_0+\rho^2/4}}{c},\qquad
p=g(\Pi)=a(\rho)\Pi,                                     \tag{6}
\]

\[
A(\Pi)=Dg(\Pi),\qquad x=A(\Pi)^{-T}R.                    \tag{7}
\]

The two tangential and one radial eigenvalues of `A` are

\[
\lambda_t=\frac{\sqrt{E_0+\rho^2/4}}{c},\qquad
\lambda_r=\frac{E_0+\rho^2/2}
 {c\sqrt{E_0+\rho^2/4}},                                  \tag{8}
\]

and are strictly positive. The map is therefore invertible, including at
`rho=0`. Since

\[
x\cdot dp=R\cdot d\Pi,                                    \tag{9}
\]

it preserves the canonical one-form and hence the symplectic form. Direct
substitution gives the exact energy identity

\[
\boxed{K(g(\Pi))=\frac12\lVert\Pi\rVert^2}.               \tag{10}
\]

At low momentum,

\[
K(p)=\frac{\lVert p\rVert^2}{2m}+O(\lVert p\rVert^4),
\qquad m=\frac{E_0}{c^2}.                                 \tag{11}
\]

Equation (11) conditionally recovers the existing production relation
`E_REST=M_INERTIAL*C_SPEED^2`. It does not determine `E0`, `c`, or the
numerical inertial mass. Canonical rescaling changes the mass normalization
while preserving the symplectic form, so the scale remains imported.

## 4. Reversible source transport and exact current

For the physical Hamiltonian

\[
E(p)=\sqrt{E_0^2+c^2\lVert p\rVert^2},                    \tag{12}
\]

the free drift over one admitted interval is

\[
q'=q+\Delta t\,\frac{c^2p}{E(p)},\qquad p'=p.             \tag{13}
\]

This is the exact Hamiltonian flow: energy is constant, replacing
`Delta t` by `-Delta t` gives the inverse, and

\[
\left\lVert\frac{c^2p}{E(p)}\right\rVert<c               \tag{14}
\]

for finite momentum. The integer-site plus centered-remainder description is
a quotient chart for `q`; it does not remove the known centered half-cell
section obstruction. The existing `FaceCurrentSegment` construction then
supplies an exact discrete continuity ledger between the two endpoints.

## 5. Conservation-fixed reaction split

The matched field momentum observer supplies the required matter impulse

\[
\Delta p_{\rm matter}=-\Delta P_{\rm field}.               \tag{15}
\]

This local vector can orient the conditional one-dimensional reaction slice
without inserting an arbitrary axis. A zero field impulse supplies no
orientation. For an initially stationary source, define

\[
K_{\rm req}=K(\Delta p_{\rm matter}),\qquad
E_{\rm res}=\frac12u^2.                                   \tag{16}
\]

The FTD-0888 history/reaction splitter is compatible exactly when

\[
0\le K_{\rm req}\le E_{\rm res}.                          \tag{17}
\]

For `u != 0`, conservation fixes the unique angle in `[0,pi/2]`:

\[
\boxed{\sin^2\eta=\frac{K_{\rm req}}{E_{\rm res}}
 =\frac{2K_{\rm req}}{u^2}}.                              \tag{18}
\]

The outgoing ledger is

\[
E_{\rm reaction}=K_{\rm req},\qquad
E_{\rm history}=E_{\rm res}-K_{\rm req}.                 \tag{19}
\]

Thus `eta=pi/4` if and only if

\[
K_{\rm req}=\frac12E_{\rm res}.                           \tag{20}
\]

Equal splitting is not a universal law. If `K_req>E_res`, the event is
inadmissible at that gate and must fail closed; clipping the request or
reading a target probability would violate the frozen programme rules.

## 6. Boundary

### Closed at reference level

- cubic symmetry forbids scalar-only spatial recoil;
- three canonical pairs are minimum in the registered orientation-free
  onsite class;
- an independently supplied fixed direction conditionally reduces the
  reaction to one pair;
- the selected relativistic dispersion admits the exact cotangent chart
  (6)--(10);
- free transport is Hamiltonian, reversible, causal, and energy preserving;
- the inherited face-current construction gives exact continuity; and
- the matched local field impulse and available residual energy uniquely fix
  the reaction split angle.

### Closed negative

- a scalar reaction magnitude selecting a nonzero cubic vector;
- a universal `eta=pi/4` recoil law; and
- deriving the inertial mass scale from the cotangent chart alone.

### Still open

- native formation and maintenance of the vector common-action triplet;
- dynamical coupling from the matched field impulse into that triplet;
- derivation of `E0` or inertial mass from a stable localized family, energy
  curvature, or pole rather than the selected production constant;
- stable ternary-source formation and interacting source dynamics;
- physical open history routing and complete energy bookkeeping;
- production migration and the centered-half-cell boundary;
- synchronization with the distinct quartic-`G*` calendar;
- Born recovery, Bell laboratory recovery, operational Lorentz hiding; and
- whole-framework completeness.

## 7. Epistemic accounting

The cubic no-go, registered-class minimum, chart identities, transport
properties, compatibility inequality, and conditional uniqueness of `eta`
are theorem-grade within their stated assumptions. The relativistic
dispersion and the interpretation of the vector pair as a source reaction are
selected. `E0`, `c`, charge, source initial data, and the fixed-context use of
the field impulse are inputs. The isolated implementation is a consistency
witness, not substrate evidence.

No new v2 selected type is introduced: the vector carrier is three copies of
the already registered canonical-pair type. Neither Hilbert-space recovery,
Born-frequency recovery, `G*` clock hardware, nor production dynamics is read
or altered by this theorem.

## 8. Verification and provenance

The frozen FTD-0889 protocol SHA-256 is
`A92F0BFB95993971AB80661B39296E948BA68E52ADED6D4A3DAF92804DB37F66`.
The frozen parent certificate SHA-256 is
`D8A8D80E1E6E497C08E7011ED7731E27C2B0B221EB894D3E9C8A61C89CF1EA0F`.
Its first locked execution returned `64/68`: every substantive theorem gate
passed; C30 retained an unsimplified positive square root, C54 lacked an
explicit frozen interval fact, C66 read a line-wrapped protocol marker, and
C68 failed dependently. No verdict is booked from that parent run.

FTD-0890 froze only those representation normalizations. Its repair protocol
SHA-256 is
`F4D8416C0AD1196070EFAEFF0DDEE4A2BA626252309142E9E44568EC15E7CF82`;
the in-memory wrapper SHA-256 is
`835EB6395B50A492DDD561691FBA23A8A565FB4F666BDE0BD291A8D6B9532445`.
The inherited certificate passes `68/68` with markers:

```text
SCALAR_REACTION_TO_SPATIAL_VECTOR=FORBIDDEN_BY_CUBIC_SYMMETRY
ORIENTATION_FREE_SPATIAL_REACTION=THREE_CANONICAL_PAIRS_MINIMUM
RELATIVISTIC_REACTION_TO_MOMENTUM_CHART=EXACT_SYMPLECTIC
SOURCE_TRANSPORT=EXACT_REVERSIBLE_REFERENCE_CONTINUATION
SPLIT_ANGLE=FIXED_BY_LOCAL_ENERGY_MOMENTUM_COMPATIBILITY
EQUAL_SPLIT=CONDITIONAL_NOT_UNIVERSAL
INERTIAL_MASS_SCALE=IMPORTED_THROUGH_E0_AND_C
NATIVE_VECTOR_COMMON_ACTION=OPEN
PRODUCTION_COUPLING=NONE
GSTAR_BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```

The isolated implementation is:

- `engine/include/ftd/eft/cubic_reaction_vector_source_transport.h`;
- `engine/src/eft/cubic_reaction_vector_source_transport.cpp`; and
- `engine/tests/test_cubic_reaction_vector_source_transport.cpp`.

The focused CTest passes `1/1`; the isolated actualization-labelled chain
passes `21/21`. No production `Voxel`, field, toggle, default, boundary mode,
or tick phase is changed.
