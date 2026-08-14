# FTD-0942 — Preregistration: existing L/R occupancy-history carrier classifier v1

**Identifier:** `FTD-0942`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact audit of whether the current production `flux_L`, `flux_R`,
`wave_vel_L`, and `wave_vel_R` storage and update laws realize the reversible,
collision-separated occupancy-history export class of FTD-0941; common/relative
canonical reduction; bare-wave aggregate reversibility; equal-source
cancellation; source-local one-hop fanout; covariant collision injectivity;
current telemetry/energy boundary; type-price trilemma; no numerical search,
fit, production change, new ontology adoption, physical scale, `G*`, Born,
Bell, measurement context, or outcome read

## 1. Question

FTD-0941 proves that exact unbounded winding cannot remain in fixed bounded
finite-alphabet hardware. Its two-lane Moore-token reference carrier survives
only by exporting distinguishable tokens into expanding support. The next
question is deliberately narrower:

> Do the **existing production L/R real fields and their present dynamics**
> already implement that collision-safe occupancy-history carrier?

The audit must distinguish three propositions which must not be conflated:

1. the L/R storage has enough coordinates for an aggregate canonical field;
2. the isolated undamped wave update is invertible on that aggregate field;
3. the production system preserves a recoverable factorization into separate
   occupancy-event tokens with direction, multiplicity, backpressure, and an
   exact source-energy transaction.

Only proposition 3 would realize FTD-0941's carrier.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/include/ftd/term_toggles.h` | `2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/render_bridge_phases/phase_movement.cpp` | `6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB` |
| `engine/src/injection.cpp` | `228A1AE44532DB7D80A0EC10ABF5639B2811849189EF2F71A6343EE59C253DC5` |
| `engine/src/constructors/constructors_molecules.cpp` | `568C896020392F448E6F2484547C60B502E9701D2F3B9FBF2FDC11B8706D06D8` |
| `engine/src/transmutation_phases.cpp` | `4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043` |
| `engine/src/diagnostics_compute.cpp` | `C3703292F8474EBC119F70024B0F3E4A23921C26EA58F8F6AB5E7581FB654AA6` |
| `THEOREM_FINITE_CAPACITY_LOCAL_REVERSIBLE_OCCUPANCY_CARRY_TRILEMMA_v1.md` | `A89DE2964B7D48100EC850547D00BB540D05F1166CF18CABE654EB9D26917548` |
| `proof_finite_capacity_local_reversible_occupancy_carry_trilemma.py` | `0256BF01710F8D6B9FFCE717FA8CB6A0E0E0B0715F2BC2F004380B9A5374FBC7` |
| `THEOREM_FLUX_WAVE_VELOCITY_MARKOV_CANONICAL_CARRIER_AND_PRODUCTION_BOUNDARY_v1.md` | `656F51A4E5A533C0436E932B452A33810CD851D63E571621DF81ECB0C9BED622` |
| `THEOREM_NATIVE_BILATERAL_QUARTIC_DYNAMICS_OBSTRUCTION_v1.md` | `2888C64166BC1E8B95807B6A8938A83971BDDF84718464B60D331B42C319C1DD` |
| `THEOREM_CUBIC_ODD_EVENT_DEPOSIT_v1.md` | `08FBF3361C453DC9E0A99184920883DBC6DE15B5043F7EFC140B0EB740A26474` |
| `THEOREM_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md` | `4A498C6D7C7E65FA685D9F0879157D76713F310A6D025CCAA8756C3F1E0322E6` |
| `THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md` | `5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA` |

The certificate must fail closed on any source drift.

## 3. Registered common/relative reduction

At every site define

\[
 F=J_L+J_R,\qquad D=J_L-J_R,
 \qquad P_F=P_L+P_R,\qquad P_D=P_L-P_R.       \tag{1}
\]

The inverse is

\[
 J_L=(F+D)/2,\quad J_R=(F-D)/2,
 \quad P_L=(P_F+P_D)/2,\quad P_R=(P_F-P_D)/2. \tag{2}
\]

This proves a storage isomorphism, not a token decomposition. If the two
halves receive the same linear operator `K` and the same source `b`,

\[
 P'_L=P_L+hKJ_L+b,\qquad P'_R=P_R+hKJ_R+b,   \tag{3}
\]

then

\[
 P'_F=P_F+hKF+2b,\qquad P'_D=P_D+hKD.        \tag{4}
\]

Hence an equally split source cannot deposit any relative record.

For the isolated undamped kick--drift step

\[
 P'_D=P_D+hKD,\qquad D'=D+hP'_D,              \tag{5}
\]

the registered inverse is

\[
 D=D'-hP'_D,\qquad P_D=P'_D-hKD.              \tag{6}
\]

The certificate must verify exact inversion and symplectic determinant one on
finite rational test operators. This gate establishes only reversible
aggregate field transport.

## 4. Registered one-hop direction gate

The production 18-point Laplacian has face weight `1/3`, edge weight `1/6`,
corner weight zero, and center weight `-4`. For a source-local initial
relative field `D_0=v!=0` and source-local conjugate value `P_0`, one
kick--drift step gives every one of the six face neighbors the nonzero value
`h^2 c_w^2 v/3` and every one of the twelve edge neighbors the nonzero value
`h^2 c_w^2 v/6`. `P_0` can alter only the source value at this step.

Therefore a source-local event cannot clear its source and translate to one
chosen Moore neighbor under the current stencil. It fans into all 18 coupled
neighbors. If `D_0=0`, a source-local `P_0` remains at the source on the first
drift and also fails the one-hop translation gate. Corners are not coupled in
one tick.

This is not a complaint that wave propagation is bidirectional. It is the
exact statement that the current operator is not the channel permutation
`c_{nu}(x)->c_{nu}(x+nu)` used by FTD-0941.

## 5. Registered collision-injectivity gate

The production relative sector is linear and has no `nu`-indexed token
ports. For any cubic-covariant odd vector-pulse encoding `e(nu)`,

\[
 e(-\nu)=-e(\nu).                              \tag{7}
\]

At one site the field superposition for the multiset `{nu,-nu}` is therefore

\[
 e(\nu)+e(-\nu)=0,                             \tag{8}
\]

identical to the vacuum aggregate. Thus the map from separate event multisets
to the aggregate linear field is noninjective, even though subsequent wave
evolution may be perfectly invertible on the aggregate state. No later
invertible evolution can recover a factorization that the initial field state
did not contain.

The certificate must not infer insufficiency merely from a count such as
`6 < 26`; real coordinates can encode finite labels. The obstruction is the
registered combination of linear superposition, odd cubic covariance,
co-location, and absence of protected channel/pulse labels. Spatially
separated pulses or a nonlinear protected-pulse invariant are outside this
no-go and remain legitimate future routes.

## 6. Production-source and energy gates

The certificate must source-audit that:

1. ordinary flux and wave-velocity injections split equally between L/R;
2. the phase-read coupling source is added equally to L/R;
3. selected particle/wavepacket/neutrino preparation can seed L/R asymmetry,
   but this is initial-condition construction, not an occupancy-hop deposit;
4. weak transmutation swaps L/R, so it reflects `D -> -D` rather than routing
   a direction-labelled token;
5. movement/annihilation and boundary handling contain clears, redistribution,
   or scaling and therefore are not the protected FTD-0941 permutation rail;
6. current diagnostics separately report L/R onsite quadratic quantities;
   the stale claim that only the common field is visible is superseded; and
7. those diagnostics remain read-only onsite telemetry. They do not define an
   exact stiffness-plus-kinetic conserved Hamiltonian, a face current, an
   occupancy-event debit, or the energy normalization `epsilon_*`.

## 7. Type-price trilemma

If the current production realization fails, the certificate must classify
the remaining routes without selecting one:

1. **derived field route:** obtain dynamically protected, collision-resolving
   pulses or characteristic sectors inside existing `(D,P_D)` fields; no new
   primitive type, but a new nonlinear invariant and source law are required;
2. **channelized port route:** adopt a finite oriented channel index
   `nu in M_26`, bounded occupancy lanes/multiplicity, reversible
   backpressure, and an energy normalization; this is a separately priced
   selected carrier type/dynamics; or
3. **external journal route:** store event history in the observation journal;
   this may diagnose the engine but is not ontic substrate dynamics and cannot
   close the physical carrier.

The audit must not report a bit count or prove that a new primitive type is
logically unavoidable. A protected nonlinear realization in existing real
fields would evade that stronger claim.

## 8. Frozen outcomes

| Outcome | Exact condition | Verdict |
|---|---|---|
| A | Current L/R storage and production dynamics pass aggregate inversion, event deposit, one-hop direction, collision separation, backpressure, and exact energy-transaction gates | existing production hardware realizes FTD-0941 |
| B | L/R storage and isolated bare-wave dynamics pass aggregate canonical inversion, but current production fails at least event deposit, direction routing, collision separation, or energy transaction | existing fields are an aggregate carrier only; occupancy-history realization remains open through derived-pulse or channelized-port routes |
| C | Even the isolated common/relative aggregate map fails exact storage or inversion gates | existing L/R fields fail as a canonical aggregate carrier |
| D | A frozen source drifts or an exact gate cannot be evaluated | execution invalid; no theorem |

No tolerance, fitting, numerical near-miss, or post-hoc branch change is
permitted.

## 9. Acceptance and stop conditions

The certificate must report separately:

- source hashes and source markers;
- common/relative storage inversion;
- exact bare-wave forward/inverse and determinant;
- equal-source cancellation from `D`;
- exact 18-neighbor fanout and corner exclusion;
- opposite-direction collision noninjectivity;
- current initializer/transmutation/movement classification;
- separate L/R telemetry and missing transaction law; and
- the derived-field/channelized-port/external-journal trilemma.

Stop immediately on source drift. Do not modify production `Voxel`, engine
tick phases, toggles, CMake, diagnostics, or physical parameters.

## 10. Promotion boundary

Outcome B would be a useful closure, not a failure of the L/R ontology. It
would prove that the existing fields can carry aggregate canonical data while
the present linear, equally sourced, isotropic dynamics do not yet retain the
factorized history required by FTD-0941. It would neither force a new type nor
derive a protected pulse. Any next implementation must be preregistered as
one of the two physical routes in section 7 and must include a source debit,
collision law, backpressure, and inverse before production integration.
