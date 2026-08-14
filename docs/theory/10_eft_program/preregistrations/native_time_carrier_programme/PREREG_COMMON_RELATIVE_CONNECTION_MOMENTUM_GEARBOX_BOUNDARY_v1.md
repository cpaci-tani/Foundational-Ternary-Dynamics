# FTD-0899 — common/relative connection and momentum-gearbox boundary v1

**Identifier:** `FTD-0899`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Date:** 2026-08-11  
**Production status:** unchanged

## 1. Question

Can the already selected common/relative canonical variables support one
local action that transfers mechanical common momentum to the relative
quartic sector while retaining a positive energy, an additive conserved total
momentum, exact reversal, and the FTD-0898 reciprocal-carry ledger? Does the
complex orientation `i` fix the coupling magnitude `gamma`, and can the
coupling remain continuously active without detuning the exact critical
quartic `G*` clock?

The registered candidate is the minimum velocity-linear connection

\[
L=\frac M2|\dot C|^2+\frac m2|\dot D|^2
  +\gamma D\cdot\dot C-\lambda|D|^4,
\qquad M,m,\lambda>0.                                    \tag{1}
\]

It is an **[IMPOSED reference coupling law]** on existing selected variables,
not a derived production interaction and not a new selected type.

## 2. Frozen sources

| source | SHA256 |
|---|---|
| `THEOREM_QUARTIC_RELATIVE_IMPULSE_RECIPROCAL_CARRY_GEARBOX_BOUNDARY_v1.md` | `E044129DB0E28DCCE3723D77027E5A652EC7A668C0DD73AD17C77E74FA7F4F6C` |
| `THEOREM_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_v1.md` | `64241D7AB18AD2079ECADF9EA25448F53F42696AB3FF439637970D4284497FD0` |
| `THEOREM_SWAP_PARITY_PHASE_READOUT_AND_ODD_POINTER_MINIMUM_v1.md` | `D73693F364A83D468AC76F3165411784610965A66ACC7BD1E7CE3766A3D267AB` |
| `THEOREM_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_BOUNDARY_v1.md` | `8269A241928681A6126B4D1F189FDEC3C5869916AF90E8825216844048D5A4C8` |
| `THEOREM_DRESSED_BOOST_MOMENTUM_MAP_AND_INERTIAL_IDENTIFIABILITY_BOUNDARY_v1.md` | `378E38227422336BF9956EA6668CA7C09006B3A1D226370577126944654F833C` |
| `THEOREM_CUBIC_REACTION_VECTOR_AND_RELATIVISTIC_SOURCE_TRANSPORT_BOUNDARY_v1.md` | `56F3DF2B830A5C52320757DAF368EAA72F3E4A4B1DA388090A2E1EB7F30C2D27` |
| `THEOREM_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_BOUNDARY_v1.md` | `0D5A093597CE7BFFF7F593C0A1AF2B65E6CDE99DB0FFEDA1183D9849BC58624F` |
| `quartic_relative_carry_gearbox.h` | `9C47BFEBE75FE61070720E53BC583CF7B9CD118C6E9E59435D4FB95B7A4BF83E` |
| `native_pair_energy_recursion.h` | `81B4941B951BC9D680A862188310706B86CDDA9DF9550204FC3F3DD567371E5A` |

Any source-hash mismatch invalidates the certificate.

## 3. Continuous common action

The canonical momenta of (1) are

\[
P=M\dot C+\gamma D,
\qquad \Pi=m\dot D.                                      \tag{2}
\]

The Legendre transform must give

\[
H=\frac{|P-\gamma D|^2}{2M}
  +\frac{|\Pi|^2}{2m}+\lambda|D|^4.                      \tag{3}
\]

Because `C` is cyclic, `P` is exactly conserved. Define the mechanical common
momentum

\[
K=M\dot C=P-\gamma D.                                    \tag{4}
\]

Then

\[
\Delta K=-\gamma\Delta D.                                \tag{5}
\]

Equation (5) is the registered gearbox law: mechanical common impulse is
exchanged with the relative coordinate while the canonical Noether momentum
`P` remains additive and conserved. Interpreting `C` as physical position or
`P` as the production total field--matter momentum remains open.

The connection one-form and curvature are

\[
A_\gamma=\gamma\sum_a D_a,dC_a,
\qquad
F_\gamma=dA_\gamma
=\gamma\sum_a dD_a\wedge dC_a.                            \tag{6}
\]

For `gamma != 0`, (6) is nonzero, so the coupling is not a total derivative
and cannot be removed as a mere constant coordinate shear.

## 4. Orientation and the role of `i`

On each `(C_a,D_a)` plane, the complex structure

\[
J=\begin{pmatrix}0&-1\\1&0\end{pmatrix},
\qquad J^2=-I,                                             \tag{7}
\]

supplies an orientation. It does not fix the real scalar multiplying the
oriented curvature. Every real `gamma` is compatible with (7); continuous
canonical/coordinate normalization changes the numerical coefficient while
leaving `J^2=-I` unchanged. The certificate must therefore keep

```text
I_SUPPLIES_ORIENTATION=TRUE
GAMMA_MAGNITUDE_DERIVED_FROM_I=FALSE
```

The canonical channel swap sends `(D,Pi)->(-D,-Pi)` and exchanges the two
connection branches:

\[
H_\gamma\circ S=H_{-\gamma}.                              \tag{8}
\]

If physical time reversal also exchanges the left/right channels, then

\[
\Theta(C,D,P,\Pi)=(C,-D,-P,\Pi)                           \tag{9}
\]

is anti-symplectic and leaves (3) invariant. Thus a fixed branch is
time-reversal covariant only conditional on the declared channel-exchange
action; otherwise `gamma` is an external time-odd orientation choice.

## 5. Registered exact discrete recursion

For signed nonzero step `h`, hold `P_1=P_0=P` and define endpoint averages
`Dbar=(D_1+D_0)/2` and `Pibar=(Pi_1+Pi_0)/2`. The registered discrete-gradient
map is

\[
C_1-C_0=\frac hM(P-\gamma\bar D),                         \tag{10}
\]

\[
D_1-D_0=\frac hm\bar\Pi,                                  \tag{11}
\]

\[
\Pi_1-\Pi_0
=\frac{h\gamma}{M}(P-\gamma\bar D)
-h\lambda(|D_1|^2+|D_0|^2)(D_1+D_0).                    \tag{12}
\]

The source-locked certificate must prove:

- exact preservation of (3);
- exact `P` conservation and (5) at the endpoints;
- exact equal-and-opposite canonical channel impulses under
  `P_L=(P+Pi)/sqrt(2)`, `P_R=(P-Pi)/sqrt(2)`;
- compatibility with the FTD-0897/0898 reciprocal-carry update after an
  imposed `p_*` is supplied;
- exact endpoint-exchange/signed-step reversal;
- covariance under signed cubic permutations;
- exact conservation of
  `J_tot=C cross P+D cross Pi` in the three-vector realization;
- global uniqueness of the endpoint from strong monotonicity and coercivity;
  and
- bounded `(D,Pi,K)` recurrence on every fixed-`P` finite-energy shell, while
  the cyclic coordinate `C` may translate without bound.

The map is an exact energy-preserving discrete-gradient realization of the
common Hamiltonian. It is not to be represented as a proved conventional
discrete variational integrator or as exact continuum flow.

## 6. Critical-clock detuning boundary

At fixed conserved `P`, the relative effective potential is

\[
V_P(D)=\lambda|D|^4+\frac{|P-\gamma D|^2}{2M}.            \tag{13}
\]

For `P=0`,

\[
\nabla^2V_0(0)=\frac{\gamma^2}{M}I.                       \tag{14}
\]

Thus the zero-Hessian critical quartic is retained in this minimum connection
class if and only if `gamma=0`, which also turns off (5). For `P != 0` and
`gamma != 0`, the origin is not an equilibrium because

\[
\nabla V_P(0)=-\frac\gamma M P.                            \tag{15}
\]

The pure-quartic continuum `G*` identity remains exact only in the uncoupled
limit of this registered class. A pulsed clutch, derived counterterm, or
separate compensated clock may evade the continuous-coupling detuning, but no
such mechanism is included here.

## 7. Certificate gates

The frozen exact certificate must test:

- all nine source hashes and their scope markers;
- the Legendre transform (2)--(3), Hamilton equations, and positive energy;
- nonzero connection curvature and non-removability as a total derivative;
- channel-swap branch exchange and conditional time-reversal covariance;
- `J^2=-I` for all `gamma` plus gamma rescaling/non-identifiability;
- exact discrete energy, momentum, channel-impulse, carry, reversal, cubic,
  angular-momentum, uniqueness, and boundedness identities;
- the detuning Hessian (14), tilt (15), and `gamma=0` decoupling control;
- FTD-0893 momentum-map compatibility only at conditional reference scope;
- physical identification, absolute scale/mass, production, Born, and
  finite-tick `G*` firewalls; and
- one fail-closed aggregate verdict.

No numerical near-miss search, fitted coupling, target-coded phase, or
formula-substitution discovery is permitted.

## 8. Outcome map

- **Outcome A:** the connection action and discrete recursion close exact
  energy, canonical total momentum, mechanical impulse exchange, reversal,
  carry compatibility, and orientation conditionally, while `gamma`, physical
  identification, mass scale, and continuous-coupling cadence remain open.
- **Outcome B:** a frozen source already derives the same connection and its
  magnitude from production substrate dynamics while retaining exact
  critical cadence. Identify it explicitly before promotion.
- **Outcome C:** any common-action, conservation, uniqueness, orientation, or
  scope gate fails. Book no theorem.
- **Execution invalid:** any hash, protocol, or terminal gate fails.

## 9. Post-certificate implementation

Only after a passing locked certificate, add an isolated EFT analyzer for
(10)--(12). It must derive the endpoint rather than consume an external
impulse; report energy, canonical/mechanical momentum, reciprocal carry,
angular momentum, reversal, connection curvature, and clock-Hessian audits;
and fail closed on nonfinite data, nonpositive masses/coupling/tolerance,
zero step, solver failure, chart overflow, child-carry failure, or invariant
failure.

It must expose these negative flags explicitly:

```text
GAMMA_DERIVED_FROM_I=FALSE
PHYSICAL_COMMON_COORDINATE_IDENTIFIED=FALSE
PHYSICAL_MOMENTUM_SCALE_DERIVED=FALSE
ABSOLUTE_MASS_DERIVED=FALSE
INTEGER_TICK_GSTAR_CADENCE_DERIVED=FALSE
EXACT_DISCRETE_VARIATIONAL_ACTION_DERIVED=FALSE
PRODUCTION_COUPLING=FALSE
BORN_TARGET_USED=FALSE
```

## 10. Next acceptance gate

Derive either a context-blind phase clutch that turns the connection on only
at compliant crossings, or a positive compensated common action that retains
the critical quartic while coupling continuously. The gate must not read
`G*`, an outcome, a context, or a Born weight. It must fix `gamma` and `p_*`
from substrate normalization or book them as calibrations, then recover the
same FTD-0893 inertia from constrained energy curvature, impulse/velocity,
and complete matter--field momentum partition.

## 11. Scope firewall

```text
COMMON_RELATIVE_CONNECTION_ACTION=IMPOSED_REFERENCE_LAW
CONNECTION_CURVATURE=NONZERO_FOR_GAMMA_NONZERO
CANONICAL_TOTAL_MOMENTUM=EXACTLY_CONSERVED
MECHANICAL_COMMON_IMPULSE=EXACTLY_EXCHANGED_WITH_RELATIVE_COORDINATE
DISCRETE_COMMON_ENERGY=EXACTLY_CONSERVED
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

The exact SHA256 of this protocol and its certificate must be entered in the
preregistration manifest before first execution.
