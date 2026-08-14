# Theorem — Common/relative connection and momentum-gearbox boundary v1

**Identifiers:** `FTD-0899` parent; `FTD-0901` repaired proof of record  
**Date:** 2026-08-11  
**Status:** `[THEOREM — EXACT CONDITIONAL CONNECTION-ACTION GEARBOX]` +
`[THEOREM — EXACT CANONICAL ENERGY/MOMENTUM/ANGULAR-MOMENTUM LEDGER]` +
`[THEOREM — CONDITIONAL SIGNED-STEP AND CHANNEL-EXCHANGE REVERSAL]` +
`[BOUNDARY — GAMMA/SCALE/IDENTIFICATION OPEN]` +
`[BOUNDARY — CONTINUOUS NONZERO CONNECTION DETUNES CRITICAL QUARTIC]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]`

## 1. Result

The minimum velocity-linear common/relative connection is a coherent exact
reference gearbox, but not yet a native substrate coupling. Adopt

\[
L=\frac M2|\dot C|^2+\frac m2|\dot D|^2
  +\gamma D\mathbin{\cdot}\dot C-\lambda|D|^4,
\qquad M,m,\lambda>0.                                    \tag{1}
\]

Equation (1) is an **[IMPOSED reference coupling law]** on the already
selected common/relative variables. Its canonical momenta and Hamiltonian are

\[
P=M\dot C+\gamma D,
\qquad \Pi=m\dot D,                                      \tag{2}
\]

\[
\boxed{H=\frac{|P-\gamma D|^2}{2M}
  +\frac{|\Pi|^2}{2m}+\lambda|D|^4}.                     \tag{3}
\]

The energy is positive. Because `C` is cyclic, `P` is exactly conserved.
The connection one-form and curvature are

\[
A_\gamma=\gamma\sum_a D_a\,dC_a,
\qquad F_\gamma=dA_\gamma
=\gamma\sum_a dD_a\wedge dC_a.                          \tag{4}
\]

For `gamma != 0`, the curvature is nonzero. The interaction is therefore not
a removable total derivative or a constant coordinate shear.

## 2. Canonical momentum versus mechanical impulse

Define the mechanical common momentum

\[
K=M\dot C=P-\gamma D.                                    \tag{5}
\]

Since `P` is conserved,

\[
\boxed{\Delta K=-\gamma\Delta D}.                        \tag{6}
\]

This is the exact gearbox law. A relative displacement produces a mechanical
common impulse while the additive canonical Noether momentum remains fixed.
It breaks FTD-0898's common/relative mechanical decoupling without violating
the full canonical ledger.

In the canonical channel chart

\[
P_L=\frac{P+\Pi}{\sqrt2},
\qquad P_R=\frac{P-\Pi}{\sqrt2},                         \tag{7}
\]

the relative endpoint still gives equal-and-opposite channel impulses,

\[
\Delta P_L=+\frac{\Delta\Pi}{\sqrt2},
\qquad
\Delta P_R=-\frac{\Delta\Pi}{\sqrt2}.                  \tag{8}
\]

Equations (6) and (8) are different ledgers: (6) is the exchange between
mechanical common momentum and relative coordinate; (8) is the canonical
two-channel split at fixed `P`. Neither equation identifies a channel as
physical matter, field, brain hemisphere, or production substrate hardware.

## 3. Exact registered discrete map

For signed nonzero step `h`, set `P_1=P_0=P` and define endpoint averages
`Dbar=(D_1+D_0)/2` and `Pibar=(Pi_1+Pi_0)/2`. The registered endpoint map is

\[
C_1-C_0=\frac hM(P-\gamma\bar D),                        \tag{9}
\]

\[
D_1-D_0=\frac hm\bar\Pi,                                 \tag{10}
\]

\[
\Pi_1-\Pi_0
=\frac{h\gamma}{M}(P-\gamma\bar D)
-h\lambda(|D_1|^2+|D_0|^2)(D_1+D_0).                   \tag{11}
\]

The frozen exact certificate proves that (9)--(11):

- preserve (3) exactly;
- preserve `P` and (6) exactly;
- generate (8) exactly;
- conserve the three-vector canonical angular momentum
  \[
  J_{\rm tot}=C\times P+D\times\Pi;                       \tag{12}
  \]
- are covariant under signed cubic permutations;
- reverse exactly under endpoint exchange and `h -> -h`; and
- have one globally unique endpoint because the reduced endpoint residual is
  strongly monotone and coercive for `M,m,lambda>0`.

At fixed finite `P` and energy, `(D,Pi,K)` are bounded. The cyclic coordinate
`C` is absent from (3) and may translate without bound. This is stable internal
recursion plus common transport, not spatial localization or constituent
formation by itself.

The map is an exact energy-preserving discrete-gradient realization. No claim
is made that it is exact continuum flow or that a conventional discrete
variational action has been derived.

## 4. Reciprocal-carry composition

After an imposed positive momentum unit `p_*` is supplied, define the
componentwise dimensionless increment

\[
q=\frac{\Delta\Pi}{\sqrt2\,p_*}.                         \tag{13}
\]

The FTD-0897 reciprocal-carry transaction composes exactly with the endpoint
momenta (7), including multiple-zone crossings. The forward transaction and
the `-h` inverse restore both the principal labels and the aggregate integer
carry.

This proves conditional chart completeness. It does not derive `p_*`, give
the carry reservoir an energy law, or identify the lifted canonical momentum
with the complete production matter--field momentum.

## 5. Orientation and reversal

On every `(C_a,D_a)` plane,

\[
J=\begin{pmatrix}0&-1\\1&0\end{pmatrix},
\qquad J^2=-I,                                            \tag{14}
\]

supplies an orientation. It does not determine the real curvature magnitude.
Every real `gamma` is compatible with (14), and coordinate normalization can
rescale `gamma` while leaving `J^2=-I` unchanged. Therefore

```text
I_SUPPLIES_ORIENTATION=TRUE
GAMMA_MAGNITUDE_DERIVED_FROM_I=FALSE
```

The channel swap `(D,Pi)->(-D,-Pi)` exchanges `gamma` and `-gamma` branches.
The map

\[
\Theta(C,D,P,\Pi)=(C,-D,-P,\Pi)                          \tag{15}
\]

is anti-symplectic and leaves (3) invariant. Time-reversal covariance of one
fixed connection branch is therefore conditional on the declared channel
exchange; it is not an unconditional handedness theorem for the substrate.

## 6. Critical-clock obstruction

At fixed `P`, the relative effective potential is

\[
V_P(D)=\lambda|D|^4+\frac{|P-\gamma D|^2}{2M}.           \tag{16}
\]

At rest,

\[
\nabla^2V_0(0)=\frac{\gamma^2}{M}I.                      \tag{17}
\]

For nonzero `P`, the origin is tilted by

\[
\nabla V_P(0)=-\frac\gamma M P.                          \tag{18}
\]

Consequently, in this minimum continuous-connection class the exact
zero-Hessian critical quartic survives if and only if `gamma=0`. But
`gamma=0` also turns off (6). The pure-quartic continuum `G*` period factor
therefore cannot be promoted unchanged through a continuously active nonzero
connection of the form (1).

This is a constructive no-go boundary for the registered class, not a no-go
theorem for every possible gearbox. A context-blind pulsed clutch, a derived
positive counterterm, or a separate compensated clock remains admissible.
`G*` is absent from (9)--(11), so no finite integer-tick cadence has been
derived.

## 7. Epistemic accounting

Theorem-grade inside the imposed reference law:

- the Legendre transform, positive Hamiltonian, and nonzero curvature;
- exact canonical energy, momentum, mechanical impulse, channel impulse, and
  angular-momentum identities;
- exact reciprocal-carry composition conditional on supplied `p_*`;
- endpoint uniqueness, internal boundedness, cubic covariance, and
  signed-step reversal;
- orientation from `J`; and
- the continuous-coupling critical-quartic obstruction (17).

Still open:

- a substrate derivation and normalization of `gamma`;
- physical identification of `C`, `D`, `P`, and the two channels;
- the physical momentum unit `p_*`, complete total-momentum partition, and
  carry energy law;
- absolute inertial mass and a stable constituent-production mechanism;
- a context-blind critical-clock-preserving clutch or compensation law;
- integer-tick `G*` cadence and operational hiding; and
- Born recovery, Bell laboratory recovery, and Lorentz recovery.

No new selected type, adoption currency, target-coded weight, fitted
coupling, or production integration is added.

## 8. Certificate and repair provenance

The frozen FTD-0899 protocol SHA-256 is
`38B7B6C929CC10F3F296FBA56A36478790D5AD648F8F9D2603058EE58F245AA0`.
Its frozen parent certificate SHA-256 is
`75426CCCE016C6471583BB65FD2D9C608D27AE871C44643F1A224F2C867176AB`.

The first FTD-0899 execution passed `C01--C41` and then raised a SymPy
`ShapeError` from a malformed generated symbol range. FTD-0900 repaired only
that separator, then passed `82/87`; its five remaining failures were frozen
Markdown/source-marker representation mismatches. Neither invalid execution
booked a theorem.

FTD-0901 froze exactly six in-memory representation substitutions:

- repair protocol SHA-256
  `6A4B56BCBC4F9552564A27FAF07EE512B6C5BF682420A2D34F628EB7BC350177`;
- repair wrapper SHA-256
  `9F3988F6DB0996FC81F856FEAFEF4B50A2B49190877E8BC4AEE3D59D26BB0E43`.

All repair-integrity gates passed, and the inherited exact certificate passed
`87/87`. The mathematics, sources, thresholds, outcomes, and scope ceilings
were unchanged. FTD-0901 is the repaired proof of record for this theorem.

## 9. Isolated reference implementation

The fail-closed witness is isolated under `ftd::eft`:

- `engine/include/ftd/eft/common_relative_connection_gearbox.h`, SHA-256
  `3FBAF00E36D8231B8B1227D6DDCC0460FED29E569F47CF1C1B0146348C9B2329`;
- `engine/src/eft/common_relative_connection_gearbox.cpp`, SHA-256
  `984EB62FB4834F26D0D1A9BED09E5FB715CFCE718727FD1D67026C4C5FB19150`;
- `engine/tests/test_common_relative_connection_gearbox.cpp`, SHA-256
  `BC59114DF3D4F25D311F1156568BC614AADBA6D759881EBF4A4CA831DF5D19B1`.

The pinned MSVC 14.44 build succeeds, the focused CTest passes `1/1`, and the
isolated actualization chain passes `27/27`. The implementation changes no
production `Voxel`, renderer, boundary, default toggle, or tick phase.

## 10. Next acceptance gate

Pre-register the smallest context-blind mechanism that preserves the critical
quartic while enabling net connection transfer. The two honest candidates are:

1. a phase clutch that sets `gamma(n)` nonzero only at a locally detectable,
   preregistered crossing and books switching work; or
2. a positive compensated action whose counterterm cancels (17) without
   cancelling (6).

The gate may read only local clock state and fixed compliance tolerances. It
may not read `G*`, a measurement context, an outcome, or a Born weight. It must
audit switching/controller energy and show whether `gamma` and `p_*` are
derived or remain calibrations.

```text
COMMON_RELATIVE_CONNECTION_ACTION=IMPOSED_REFERENCE_LAW
CONNECTION_CURVATURE=NONZERO_FOR_GAMMA_NONZERO
CANONICAL_TOTAL_MOMENTUM=EXACTLY_CONSERVED
MECHANICAL_COMMON_IMPULSE=EXACTLY_EXCHANGED_WITH_RELATIVE_COORDINATE
DISCRETE_COMMON_ENERGY=EXACTLY_CONSERVED
CANONICAL_ANGULAR_MOMENTUM=EXACTLY_CONSERVED
RECIPROCAL_CARRY_COMPATIBILITY=EXACT_CONDITIONAL_ON_PSTAR
I_SUPPLIES_ORIENTATION=TRUE
GAMMA_MAGNITUDE_DERIVED_FROM_I=FALSE
TIME_REVERSAL=CONDITIONAL_ON_CHANNEL_EXCHANGE
CONTINUOUS_NONZERO_CONNECTION_PRESERVES_CRITICAL_QUARTIC=FALSE_IN_REGISTERED_CLASS
PHYSICAL_COMMON_COORDINATE_IDENTIFICATION=OPEN
PHYSICAL_MOMENTUM_SCALE=OPEN
ABSOLUTE_MASS=NOT_DERIVED
INTEGER_TICK_GSTAR_CADENCE=OPEN
EXACT_DISCRETE_VARIATIONAL_ACTION=OPEN
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_TYPE=TRUE
BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```
