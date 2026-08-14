# FTD-0898 — quartic-relative impulse and reciprocal-carry gearbox boundary v1

**Identifier:** `FTD-0898`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Date:** 2026-08-11  
**Production status:** unchanged

## 1. Question

Can the existing selected local relative-quartic recursion generate the
equal-and-opposite increment consumed by FTD-0897, conserve a positive energy
exactly, and place the `G*` clock factor and reciprocal carry in one coherent
reference gearbox? If so, does that composition already couple the clock to
the common/matter mode or derive a physical momentum scale?

The registered candidate is a conditional exact composition. The relative
quartic update generates an internal impulse from its own state and coupling;
an orthogonal two-channel chart sends that impulse equally and oppositely to
the two channel momenta; FTD-0897 retains every reciprocal-zone carry. The
same construction leaves the common mode exactly unchanged and keeps all
couplings, scale, production identification, and finite-tick `G*` cadence
open.

## 2. Frozen sources

| source | SHA256 |
|---|---|
| `THEOREM_RECIPROCAL_CARRY_RESERVOIR_AND_LOCAL_IMPULSE_LEDGER_BOUNDARY_v1.md` | `8696F6024CE6ED49120DF6A238F98C8C804AA7B8C441BCA83B5AFDCE111C6048` |
| `THEOREM_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_v1.md` | `64241D7AB18AD2079ECADF9EA25448F53F42696AB3FF439637970D4284497FD0` |
| `THEOREM_LOCAL_FLUX_SELF_PAIR_TENSOR_RECURSION_v1.md` | `62A95FF322C99773D03002444376B9244A93CC19D01CF4400230277288CADAEB` |
| `DERIV_BILATERAL_SELF_DUAL_QUARTIC_CLOCK_v1.md` | `779044879BB28CE0DB13BA8783EC7FF9AB5DFDFE10DF1C259D3D11998DEEDB9A` |
| `THEOREM_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_BOUNDARY_v1.md` | `0D5A093597CE7BFFF7F593C0A1AF2B65E6CDE99DB0FFEDA1183D9849BC58624F` |
| `THEOREM_CUBIC_REACTION_VECTOR_AND_RELATIVISTIC_SOURCE_TRANSPORT_BOUNDARY_v1.md` | `56F3DF2B830A5C52320757DAF368EAA72F3E4A4B1DA388090A2E1EB7F30C2D27` |
| `THEOREM_DRESSED_BOOST_MOMENTUM_MAP_AND_INERTIAL_IDENTIFIABILITY_BOUNDARY_v1.md` | `378E38227422336BF9956EA6668CA7C09006B3A1D226370577126944654F833C` |
| `native_pair_energy_recursion.h` | `81B4941B951BC9D680A862188310706B86CDDA9DF9550204FC3F3DD567371E5A` |
| `reciprocal_carry_reservoir.h` | `69D4D225DD0D94EBD3A13C424FB78CA51238495A3DB51625129514253293B6BE` |

Any source-hash mismatch invalidates the certificate.

## 3. Registered relative recursion

For `m>0`, `lambda>0`, and signed nonzero step `h`, let the relative state
`(D,Pi)` satisfy the exact discrete-gradient endpoint equations

```text
D_1-D_0 = h(Pi_1+Pi_0)/(2m),
Pi_1-Pi_0 = -h lambda
  (D_1^3+D_1^2 D_0+D_1 D_0^2+D_0^3).          (1)
```

They conserve

```text
H_D = Pi^2/(2m)+lambda D^4.                    (2)
```

Introduce the orthogonal channel momenta

```text
P_L=(P_C+Pi)/sqrt(2),
P_R=(P_C-Pi)/sqrt(2).                           (3)
```

Then the recursion generates the channel increment

```text
Delta P_L = +Delta Pi/sqrt(2),
Delta P_R = -Delta Pi/sqrt(2),                  (4)
```

and the common total `P_L+P_R=sqrt(2)P_C` is exactly constant. Equation (4)
is the increment origin inside this selected relative model; it is not an
external impulse input.

## 4. Composition with reciprocal carry

For an imposed positive momentum unit `p_*`, define the dimensionless pair
increment

```text
q=(Pi_1-Pi_0)/(sqrt(2)p_*).                     (5)
```

Decompose the two channel momenta into principal labels and windings,

```text
P_a/p_* = k_a+2 pi w_a,
k_a in [-pi,pi), w_a in Z.                     (6)
```

Applying (5) as `+q,-q` and updating the aggregate reservoir by the two zone
carries must reproduce the chart decomposition of the endpoint momenta and
conserve

```text
(P_L+P_R)/p_*=k_L+k_R+2 pi(w_L+w_R).            (7)
```

The composed update must reverse under `h -> -h` and telescope over repeated
admitted steps.

## 5. `G*` cadence and common-mode boundary

The continuum generator associated with (2) has turning amplitude `A` and

```text
T A = sqrt(pi) G* sqrt(m/(2 lambda)).           (8)
```

Thus one selected relative quartic mode can coherently supply both a local
internal impulse cycle and the exact lemniscatic continuum period factor.
`G*` is not inserted into (1), (4), or the carry rule.

This does not establish an exact integer-tick cadence. The discrete-gradient
step is energy preserving and reversible, but its finite-step orbit is not
the exact continuum Hamiltonian trajectory. Nor does the composition couple
the relative clock to the common mode: `P_C` is a strict invariant. Calling
`L` matter and `R` field/reaction is a physical identification still to be
derived or selected.

FTD-0886's phase-cylinder obstruction also remains active. The carry is chart
history; it is not an action battery. A nonzero constant action translation
is not a globally Hamiltonian time map on a periodic phase cylinder, and a
state-dependent phase-blind drain is not symplectic. The exact energy in this
candidate is (2), not an inferred energy of the integer carry.

## 6. Certificate gates

The source-locked exact certificate must test:

- all nine source hashes and scope markers;
- orthogonal common/relative coordinate and momentum transforms;
- exact canonical one-form and kinetic-norm splits;
- exact discrete-gradient energy conservation;
- deterministic equal-and-opposite channel increments;
- common-momentum invariance;
- exact reciprocal wrap/carry composition on no-crossing, one-zone, and
  multiple-zone arms;
- exact endpoint chart reconstruction and aggregate conservation;
- signed-step reversal equations and carry reversal;
- the continuum beta/gamma identity (8);
- scale non-identifiability under `p_* -> s p_*`;
- strict common-mode decoupling and reaction-partner identification boundary;
- phase-cylinder/action-battery firewall;
- finite-tick cadence, production, Born, `G*`, mass, and type-currency
  firewalls; and
- one fail-closed aggregate verdict.

No numerical near-miss search, fitted period, target-coded increment, or
formula-substitution discovery is permitted.

## 7. Outcome map

- **Outcome A:** the selected relative recursion generates the exact internal
  impulse, energy, carry, reversal, and continuum `G*` factor, while common
  coupling, scale, finite-tick cadence, and production remain open.
- **Outcome B:** a frozen source already supplies the same composition as a
  native production matter--field common action with physical scale and
  finite-tick cadence. Identify it explicitly before any promotion.
- **Outcome C:** any algebraic composition or scope gate fails. Book no
  theorem.
- **Execution invalid:** any hash, protocol, or terminal gate fails.

## 8. Post-certificate implementation

Only after a passing locked certificate, add an isolated EFT analyzer that
composes `advance_native_pair_energy` with the FTD-0897 reciprocal-carry API.
It must derive the increment from the relative endpoint, reconstruct both
channel charts, verify common momentum and relative energy, execute the
signed-step inverse, and fail closed on invalid parameters, chart overflow,
or either child transaction failure.

It must expose these negative flags explicitly:

```text
COMMON_MODE_COUPLING_DERIVED=FALSE
MATTER_FIELD_IDENTIFICATION_DERIVED=FALSE
MOMENTUM_SCALE_DERIVED=FALSE
INTEGER_TICK_GSTAR_CADENCE_DERIVED=FALSE
CARRY_ENERGY_LAW_DERIVED=FALSE
ABSOLUTE_MASS_DERIVED=FALSE
PRODUCTION_COUPLING=FALSE
BORN_TARGET_USED=FALSE
```

## 9. Next acceptance gate

Break the exact common/relative decoupling with one preregistered local
coupling derived from the substrate common action. It must transfer an impulse
between the relative clock/reaction sector and the actual matter/field common
sector while conserving the full energy and total momentum, preserving
orientation and causal locality, and retaining the carry/history needed for
reversal. It must then fix `p_*` or expose that unit as an irreducible
calibration and test integer-tick phase crossing against the separate global
update order.

## 10. Scope firewall

```text
RELATIVE_QUARTIC_INCREMENT_ORIGIN=EXACT_INSIDE_SELECTED_REFERENCE_RECURSION
CHANNEL_IMPULSES=EXACT_EQUAL_AND_OPPOSITE
RELATIVE_ENERGY=EXACTLY_CONSERVED
RECIPROCAL_CARRY_COMPOSITION=EXACT
CONTINUUM_GSTAR_PERIOD=EXACT_CONDITIONAL_ON_SELECTED_QUARTIC
COMMON_MODE_COUPLING=OPEN
MATTER_FIELD_IDENTIFICATION=OPEN
MOMENTUM_SCALE=OPEN
INTEGER_TICK_GSTAR_CADENCE=OPEN
CARRY_ENERGY_LAW=OPEN
ABSOLUTE_MASS=NOT_DERIVED
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_TYPE=TRUE
BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```

The exact SHA256 of this protocol and its certificate must be entered in the
preregistration manifest before first execution.
