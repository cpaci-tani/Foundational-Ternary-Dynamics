# FTD-0939 — Preregistration: phase-gated neutral-C4 Hodge chord and occupancy-carry boundary v1

**Identifier:** `FTD-0939`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact classification of the minimum charge-even ternary transport
observable; exact signed and occupancy face currents for a live-current-selected
Moore-edge relocation of the neutral C4 dipole; exact central-current bridge;
conditional local Hodge action and scalar-energy transaction; signed-current,
history-carry, wake-identification, momentum-scale, and production boundaries;
no numerical search, fit, post-hoc tolerance, target direction, target wake,
new ontology type, production promotion, `G*` cadence, Born, Bell, measurement
context, outcome, or hiding read

## 1. Question

FTD-0938 proves that the live body current fixes a direction but that direction
plus the positive FTD-0933 wake does not identify a reciprocal impulse or its
carry owner. Earlier common-action work supplies two exact ingredients:

1. the FTD-0576 Hodge source/work identity, conditional on a source density and
   current satisfying the same central continuity law; and
2. the FTD-0580 positive chord plus democratic shortest-path face routing,
   which makes one Moore relocation local and energy centered.

The present protocol asks whether the existing ternary record itself contains
a minimum transport observable that can own the **directed neutral-body
crossing**, while keeping that transport role distinct from signed field charge,
Bloch carry, and real momentum.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `THEOREM_NATIVE_FIELD_DISCRETE_ACTION.md` | `2CB4B2D49DED01D9B642416D3C20B89C41F5682FC52896446BEBFB3D1CA8B63C` |
| `THEOREM_NATIVE_HODGE_ENERGY_CONTINUITY.md` | `7849BFF214225723BFA52EA9034C34B22B94D204A78BE1D6DC6F97D065222868` |
| `THEOREM_SYMMETRIC_CHORD_MOORE_ACTION.md` | `B80E574B8C421B28DC0AFFC35F5B898DF6FF79A1CEBA06588B22862FDCF1468D` |
| `THEOREM_TERNARY_CONTINUITY_MIDPOINT_SOURCE_RECURRENCE_AND_CANONICAL_RECIPROCITY_BOUNDARY_v1.md` | `B3140D967A3593846B7A8FB0D9682C403E379540F3314AF9CFFF25A649EF20EF` |
| `proof_ternary_continuity_midpoint_source_recurrence_canonical_reciprocity.py` | `E0A03721A089B43137EC986E1EB2024D9AF93B43062603B4C23FF5CA32E806B9` |
| `THEOREM_C4_COMPANION_TRANSLATION_MISMATCH_DRESSING_METRIC_AND_RECOIL_BOUNDARY_v1.md` | `BE70433D871293C42FACD879FF4C8D5E3DCD23DAF83CAD7266806648DF17024F` |
| `proof_c4_companion_translation_mismatch_dressing_metric_recoil_boundary.py` | `5B56223709DA3957F852D889F4514D94F261F3819E3178E0E4FA43CEB74814FC` |
| `THEOREM_C4_CHARACTER_PARITY_KERNEL_PRIMITIVE_DIRECTION_AND_COMPACT_BODY_ORBIT_v1.md` | `19BC23F55AB421E4F4D579DAE735000FDB29A7D45E1CB7AAE6B7A9366BDA71A8` |
| `proof_c4_character_parity_kernel_primitive_direction_compact_body_orbit.py` | `6FBBC402CCE5B26C3D79F7F57B1B78752420C9072EFA8FF5B58FEAF92066B3B2` |
| `THEOREM_PHASE_GATED_PRIMITIVE_C4_CONNECTION_AND_WAKE_RECOIL_IDENTIFIABILITY_BOUNDARY_v1.md` | `B75012F31DDFEBDA7ADFAE5C990AF8FEC3A34C384F840935DEDDB74398030646` |
| `proof_phase_gated_primitive_c4_connection_wake_recoil_identifiability_boundary_v2.py` | `8F3F063A4EF96D99F2797E04E10C9D08A0882F558F319E6ED836167C2A596C84` |

The certificate fails closed on source drift.

## 3. Minimum even ternary transport observable

Let `a(s)` be a real onsite polynomial of degree at most two on
`s in {-1,0,+1}`. Register the conditions

\[
 a(0)=0,\qquad a(-s)=a(s),\qquad a(1)=1.             \tag{1}
\]

The certificate must prove the unique solution

\[
 \boxed{a(s)=s^2.}                                   \tag{2}
\]

Write

\[
 \rho_s(x)=s(x),\qquad \nu(x)=s(x)^2.                \tag{3}
\]

`rho_s` is signed field charge. `nu` is an existing-record occupancy
observable: it is not a new primitive state or an assigned mass.

## 4. Registered neutral body and live direction

Use the neutral two-site C4 source arm

\[
 s_0=\delta_{e_x}-\delta_{-e_x},\qquad
 \sum_xs_0(x)=0,\qquad \sum_x\nu_0(x)=2.             \tag{4}
\]

At the declared local gate phase, take the direction from the live FTD-0936
body current,

\[
 d=u_n\in\{(-1,1,0),(-1,-1,0),(1,-1,0),(1,1,0)\}.   \tag{5}
\]

The endpoint record is the integer translate `T_d s_0`. The certificate may
not read a target direction or choose a direction from the field wake.

For each active axis of `d`, average all monotone shortest face paths
uniformly. Deposit the signed face current `K_s` with source weights `s_0(x)`
and the occupancy face current `K_nu` with weights `nu_0(x)`. With oriented
face divergence `d_f`, the registered identities are

\[
 d_fK_s=s_0-T_ds_0,\qquad
 d_fK_\nu=\nu_0-T_d\nu_0.                             \tag{6}
\]

The integrated currents must satisfy

\[
 \boxed{\sum_xK_s(x)=d\sum_xs_0(x)=0,}               \tag{7}
\]

\[
 \boxed{\sum_xK_\nu(x)=d\sum_x\nu_0(x)=2d.}         \tag{8}
\]

Thus signed current cannot own the center translation of this neutral body,
whereas the normalized occupancy crossing recovers `d` exactly. Equation (8)
is a transport identity, not a momentum normalization.

## 5. Central bridge and locality

Freeze the FTD-0577/0580 operators

\[
 B_i={T_i^{-1}+2+T_i\over4},\qquad
 A_i={1+T_i^{-1}\over2},\qquad B_M=B_xB_yB_z.        \tag{9}
\]

For either face current `K`, define

\[
 q_i=A_i\prod_{j\ne i}B_jK_i.                        \tag{10}
\]

The certificate must prove the Laurent identity

\[
 d_{c,i}A_i=B_id_{f,i}                                \tag{11}
\]

and therefore

\[
 \boxed{D_cq=B_M(\rho_0-\rho_1)}                     \tag{12}
\]

for both signed and occupancy densities. Support must remain finite and grow
only by the locked one-cell coat around the finite shortest-path corridor.

## 6. Conditional local Hodge action and scalar ledger

For a signed source transition obeying

\[
 \rho_1-\rho_0+D_cQ=0,                               \tag{13}
\]

use the unique native work coordinate

\[
 R=J-W/2                                              \tag{14}
\]

and the midpoint Hodge source

\[
 S=-G_CG\bar\rho+G_CCQ.                              \tag{15}
\]

The certificate must rederive, rather than merely assert,

\[
 \Delta H_f=\langle S,\Delta R\rangle,               \tag{16}
\]

\[
 \Delta U_{\rm int}
 =-G_C\langle\bar\rho,D\Delta R\rangle
  -G_C\langle Q,GD\bar R\rangle,                    \tag{17}
\]

\[
 \Delta H_m
 =G_C\langle Q,GD\bar R-C\Delta R\rangle,          \tag{18}
\]

and

\[
 \boxed{\Delta H_f+\Delta U_{\rm int}+\Delta H_m=0.}\tag{19}
\]

Equations (15)--(19) are a conditional common-action scalar transaction. They
use the pre-hop source record, selected path current, and endpoint fields;
they may not read the post-event FTD-0933 wake as a target debit.

## 7. Carry and wake firewalls

The certificate must keep four objects distinct:

1. signed Hodge current `K_s`, which sources the field but has zero aggregate
   on the neutral rigid translation;
2. occupancy current `K_nu`, which owns the directed crossing for one hop;
3. a cumulative unwrapped torus winding/carry, which is history dependent and
   is not present in the instantaneous ternary record; and
4. real physical momentum, which still requires an independent scale and
   inertial law.

For any scalar `p_*>0`, the same integer occupancy crossing admits the
candidate conversion `P=p_* W_nu`. Therefore neither equation (8) nor the
Hodge energy identity fixes `p_*`, mass, `gamma`, or impulse magnitude.

The local chord action is not automatically identical to the abrupt-source,
field-frozen transaction used to define the exact FTD-0933 wake
`Dbar(d)`. The certificate may establish a target-blind local debit route but
must not claim

\[
 \Delta H_m=-\overline{\mathcal D}(d)                 \tag{20}
\]

without an independent derivation for the same source history and temporal
ordering. Equality (20) remains open unless proved by the locked gates.

## 8. Registered outcomes

- **Outcome A — local Hodge transaction plus occupancy-carry boundary:**
  equations (2), (6)--(19) pass for all four live directions. Signed current
  is proved insufficient for neutral center transport; `s^2` supplies the
  unique registered even onsite occupancy and its face current owns one
  directed crossing. The local scalar action closes conditionally. Persistent
  torus carry, real momentum, scale, and identification with the exact
  FTD-0933 abrupt wake remain open.
- **Outcome B — local scalar transaction only:** the Hodge action and central
  continuity pass but the occupancy uniqueness, directional-current, or
  signed-current obstruction fails. No carry-owner statement is licensed.
- **Outcome C — registered route fails:** the local continuity or scalar
  energy identity fails. The candidate is archived as closed negative.
- **Invalid:** source drift, formula change after lock, numerical search, fit,
  tolerance repair, target direction, target wake subtraction, nonlocal
  simultaneous companion translation, hidden environment debit, promotion of
  `s^2` to assigned mass, promotion of `p_*`, `gamma`, mass, or impulse,
  production mutation, new type adoption, `G*`/Born/context/outcome read, or
  completed-infinity rhetoric.

## 9. Firewalls and next gate

No engine source, CMake target, `Voxel` field, toggle, default, production
law, ontology type, import, physical constant, phenomenological formula, Born
weight, Bell correlation, measurement context, outcome, or `G*` cadence is
changed.

Even Outcome A does not derive an autonomous hop, a persistent Markovian
carry state, real reciprocal impulse, physical mass, `p_*`, `gamma`, exact
FTD-0933 wake payment, source formation, perturbation recovery, production
behavior, Lorentz hiding, or completeness.

The next admissible gate is to decide whether cumulative occupancy flux can be
compiled into a finite-capacity local reversible carry without body identity,
or whether a separately priced link/worldline state is required. Only after
that gate may the carried integer be coupled to the phase-gated quartic source
action and tested against the exact FTD-0933 wake under one common temporal
ordering.
