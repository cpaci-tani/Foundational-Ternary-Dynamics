# FTD-0928 — Self-dual reciprocal discrete action and formation-reservoir boundary v1

**Identifier:** `FTD-0928`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact symplectic test of the frozen FTD-0927 one-way
record–matter-to-field recurrence; minimum swap-symmetric two-coordinate
reciprocal completion; positive discrete tick invariant and stability band;
canonical rechart of the existing remainder–velocity pair; phase-complete
formation-reservoir lower bound and conditional complete-pair transfer; no
numerical search, fit, engine mutation, new production law, or `G*`/Born/Bell
read

## 1. Question

FTD-0927 proves that its minimum continuous canonical interaction reacts in
the remainder equation rather than supplying the registered velocity impulse.
Can a discrete generating function evade that result while retaining the
one-way compositional map? If not, what is the minimum reciprocal
exchange-symmetric discrete action that preserves the registered `C4` orbit,
and what additional phase-complete reservoir is required to form rather than
merely propagate that orbit?

The certificate must distinguish:

1. a generating function for the frozen triangular map;
2. a reciprocal common action on an enlarged or reidentified phase space;
3. a positive stable recursive reference body; and
4. a physical formation mechanism.

None may be inferred from the others.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `THEOREM_TERNARY_CONTINUITY_MIDPOINT_SOURCE_RECURRENCE_AND_CANONICAL_RECIPROCITY_BOUNDARY_v1.md` | `B3140D967A3593846B7A8FB0D9682C403E379540F3314AF9CFFF25A649EF20EF` |
| `proof_ternary_continuity_midpoint_source_recurrence_canonical_reciprocity.py` | `E0A03721A089B43137EC986E1EB2024D9AF93B43062603B4C23FF5CA32E806B9` |
| `THEOREM_LOCAL_REMAINDER_VELOCITY_C4_HAMILTONIAN_AND_FORMATION_BOUNDARY_v1.md` | `60DFDF4F3FDB13151D66E2128AA14FB92318D619ABD5506D98A22B75EDCC39F3` |
| `proof_local_remainder_velocity_c4_hamiltonian_formation_ledger.py` | `F2E53AA3180816AE0732663E6DC5180EFFE419C864B5310E0E400DFC6B81007E` |
| `THEOREM_NATIVE_FIELD_DISCRETE_ACTION.md` | `2CB4B2D49DED01D9B642416D3C20B89C41F5682FC52896446BEBFB3D1CA8B63C` |
| `THEOREM_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_BOUNDARY_v1.md` | `0D5A093597CE7BFFF7F593C0A1AF2B65E6CDE99DB0FFEDA1183D9849BC58624F` |
| `THEOREM_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_v1.md` | `64241D7AB18AD2079ECADF9EA25448F53F42696AB3FF439637970D4284497FD0` |
| `THEOREM_NATIVE_BILATERAL_QUARTIC_DYNAMICS_OBSTRUCTION_v1.md` | `2888C64166BC1E8B95807B6A8938A83971BDDF84718464B60D331B42C319C1DD` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |

The certificate fails closed on source drift.

## 3. Frozen triangular-map symplectic test

Let `z` denote the FTD-0926 matter canonical state and `y=(J,P)` the
FTD-0574 field canonical state. The frozen compositional map has block form

\[
 z'=Az,
 \qquad
 y'=By+c(z),                                               \tag{1}
\]

where `A` and `B` are individually symplectic and the Hodge source has
nonzero derivative `C=Dc/Dz`.

The full Jacobian is

\[
 T=\begin{pmatrix}A&0\\C&B\end{pmatrix}.                 \tag{2}
\]

For the direct-sum symplectic form, the upper-right symplectic condition is

\[
 C^{\mathsf T}\Omega_f B=0.                              \tag{3}

Because `B` and `Omega_f` are invertible, equation (3) implies `C=0`.
Therefore any nonconstant one-way source makes (1) nonsymplectic. The
certificate must prove the general block statement and an exact scalar-mode
witness. If it holds, no discrete generating function of any type exists for
the frozen triangular map on the frozen direct-sum phase space.

This does not exclude a reciprocal completion whose matter output depends on
the field.

## 4. Frozen canonical rechart of the existing matter pair

The FTD-0926 map is

\[
 (r,v)\mapsto(v-r,v-2r).                                  \tag{4}

Define

\[
 Q={v\over\sqrt2},
 \qquad
 \Pi={v-2r\over\sqrt2}.                                  \tag{5}

The certificate must prove that (5) is symplectic and transforms (4) into

\[
 (Q,\Pi)\mapsto(\Pi,-Q),                                  \tag{6}

with

\[
 r^2-rv+{v^2\over2}={Q^2+\Pi^2\over2}.                   \tag{7}

Thus the source dependence on `v` can be represented as dependence on a
canonical configuration. This changes the chart, not the physics, and does
not by itself repair the triangular obstruction.

## 5. Frozen reciprocal self-dual action class

Shift the dynamic field coordinate by its static scaffold equilibrium,

\[
 X=J-H,
\]

and introduce a field-shaped canonical companion `Q`. Its physical
identification with the local record/current is **not** assumed.

For symmetric field stiffness `K`, freeze the nearest-time-slice action

\[
 L_d={1\over2}\|X_{n+1}-X_n\|^2
    +{1\over2}\|Q_{n+1}-Q_n\|^2
    -V_\gamma(X_n,Q_n),                                   \tag{8}
\]

where

\[
 V_\gamma
 ={1\over2}\langle X,KX\rangle
 +{1\over2}\langle Q,KQ\rangle
 -\langle X,(K-2I)Q\rangle
 +{\gamma\over2}\|X-Q\|^2.                              \tag{9}

The first three terms are the reciprocal completion that retains the frozen
source on the self-dual section. The last term is the registered
context-blind mismatch penalty. `gamma` may not be fitted to the orbit.

In self-dual and anti-self-dual coordinates

\[
 A={X+Q\over\sqrt2},
 \qquad
 B={X-Q\over\sqrt2},                                      \tag{10}

the potential eigenoperators must be

\[
 2I,
 \qquad
 2(K+(\gamma-1)I).                                        \tag{11}

For the full production band `0<=K<=16I/9`, positivity for every mode requires
`gamma>=1`. The frozen minimum is therefore

\[
 \boxed{\gamma=1}.                                        \tag{12}

At (12), the action factorizes exactly:

\[
 V=\langle A,A\rangle+\langle B,KB\rangle,                \tag{13}

\[
 \Delta_t^2A=-2A,
 \qquad
 \Delta_t^2B=-2KB.                                       \tag{14}

The self-dual sector is an exact period-four clock. The anti-self-dual sector
inherits twice the native field stiffness and remains inside the stable
kick–drift band because `2(16/9)<4`.

On the self-dual section `X=Q`, the field equation relative to the original
free term `-KX` has source

\[
 U=(K-I)Q-X=(K-2I)Q,                                     \tag{15}

which is the frozen FTD-0927 abstract dynamic source. Off that section, the
same action supplies the reciprocal mismatch feedback. The certificate must
prove (8)--(15), exact symplecticity, positive tick invariants for every
`0<k<=16/9`, and exact `C4` return.

## 6. Identification firewall

`Q` in (8) is a field-shaped canonical companion. The certificate must not
silently identify it with:

- the pointwise 19-site current;
- the complete ternary record;
- the production dual-substrate right field; or
- a new bond/link-current ontology type.

Those are separate candidate realizations. The local `(r,v)` rechart (5)
supplies an exact phase plane but not the spatial PreparationMap from the
20-site matter scaffold into the evanescent field-shaped coordinate. Outcome
B must therefore book the action as a selected reference completion and leave
this map open.

## 7. Frozen formation-reservoir tests

The action (8) is linear and (14) decouples its two sectors. Therefore an
initially empty self-dual sector remains empty. Stability is not formation.

The certificate must prove:

1. one scalar energy account cannot be a phase-complete canonical reservoir,
   because an odd-dimensional phase space has no nondegenerate antisymmetric
   form;
2. the phase-blind state-dependent drain
   `(z,I,phi)->(F(z),I-w(z),phi)` adds `-dw wedge dphi` and is nonsymplectic
   unless `dw=0`;
3. at least one complete canonical pair is therefore required for one
   registered phase plane; and
4. a complete-pair species quarter-turn

   \[
   (Z_b,Z_R)\mapsto(Z_R,-Z_b)                              \tag{16}
   \]

   is symplectic, exactly energy preserving for identical positive metrics,
   reversible, and transfers a prepared reservoir mode into an empty body.

Equation (16) is a conditional sufficiency witness only. It presupposes the
body profile, orientation, phase, full positive reserve, and eligibility
event. It does not form the static halo `H`, derive the physical debit
`26 pi/25+1+k_h/2`, or provide an autonomous local reservoir in production.

## 8. Frozen outcomes

- **Outcome A — existing-type reciprocal formation closure:** the frozen
  triangular map has a generating function or the reciprocal completion is
  realized entirely by existing local types, and a phase-complete positive
  reservoir forms the full body without target/profile reads.
- **Outcome B — reciprocal self-dual reference action / formation boundary:**
  the triangular map is nonsymplectic; equations (8)--(15) give the minimum
  positive exchange-symmetric reciprocal reference action with exact `C4`
  and stable anti sector; the existing matter pair has the exact chart (5),
  but its spatial identification with `Q` and autonomous full-profile
  formation remain open. Equation (16) establishes only the minimum
  phase-complete reservoir witness.
- **Outcome C — positive reciprocal completion fails:** the proposed action
  is not symplectic, does not retain the frozen source/orbit, or has an
  unstable production-band sector at the frozen minimum.
- **Invalid:** source drift, post-lock coefficient change, numerical search,
  target-arm/profile read, fitted tolerance, engine/CMake mutation, failed
  combined gate, or claim promotion beyond the locked scope.

## 9. Firewalls

No engine source, CMake target, `Voxel` field, toggle, default, import,
selected ontology type, production law, or paper is changed. No period search,
near-miss scan, formula-substitution discovery, or physical `G*` cadence is
permitted.

Even Outcome A would not establish Born frequencies, Bell correlations,
measurement context, Lorentz hiding, physical mass/scale, or framework
completeness. Outcome B additionally leaves the `PreparationMap`, local
bond-current/dual-field realization, static-halo formation, nonlinear energy
transfer, and production recovery open.
