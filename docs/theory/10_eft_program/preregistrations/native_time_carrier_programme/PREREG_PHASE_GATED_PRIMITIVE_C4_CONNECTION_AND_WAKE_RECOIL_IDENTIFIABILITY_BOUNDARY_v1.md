# FTD-0937 — Preregistration: phase-gated primitive C4 connection and wake-recoil identifiability boundary v1

**Identifier:** `FTD-0937`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact synthesis of the live FTD-0936 primitive current with the
lowest-degree critical-quartic-preserving common connection; exact source
energy, mechanical/canonical momentum, switching, and full-cycle ledgers;
direct-composition test against the positive FTD-0933 hop wake; identifiability
test for real impulse magnitude, reciprocal carry, and physical scale; no
numerical search, fit, post-hoc tolerance, production promotion, new type,
physical-momentum promotion, `G*` cadence, Born, Bell, context, outcome, or
hiding read

## 1. Question

FTD-0936 supplies a live, time-odd polar primitive current

\[
 u_n\in\{(-1,1,0),(-1,-1,0),(1,-1,0),(1,1,0)\},
 \qquad u_{n+1}=Su_n.                                  \tag{1}
\]

FTD-0904 proves that an even quadratic connection can preserve a pure
critical quartic while producing directed common displacement, but its polar
axis and branch were conditional inputs. FTD-0933 proves that every nonzero
integer relocation of a formed C4 source leaves a strictly positive field
wake. The present protocol asks:

1. does the native `u_n` uniquely fix the lowest-degree connection tensor in
   a declared cubic-equivariant class;
2. does the resulting source action preserve the exact critical quartic and
   canonical ledgers;
3. can that closed source cycle be composed directly with the FTD-0933 field
   hop without an additional debit or backreaction; and
4. do direction, wake energy, and the compact character identify a unique
   real reciprocal impulse?

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `THEOREM_C4_CHARACTER_PARITY_KERNEL_PRIMITIVE_DIRECTION_AND_COMPACT_BODY_ORBIT_v1.md` | `19BC23F55AB421E4F4D579DAE735000FDB29A7D45E1CB7AAE6B7A9366BDA71A8` |
| `proof_c4_character_parity_kernel_primitive_direction_compact_body_orbit.py` | `6FBBC402CCE5B26C3D79F7F57B1B78752420C9072EFA8FF5B58FEAF92066B3B2` |
| `THEOREM_C4_COMPANION_TRANSLATION_MISMATCH_DRESSING_METRIC_AND_RECOIL_BOUNDARY_v1.md` | `BE70433D871293C42FACD879FF4C8D5E3DCD23DAF83CAD7266806648DF17024F` |
| `proof_c4_companion_translation_mismatch_dressing_metric_recoil_boundary.py` | `5B56223709DA3957F852D889F4514D94F261F3819E3178E0E4FA43CEB74814FC` |
| `THEOREM_ORIENTED_EVEN_SELF_PAIR_RECTIFIER_AND_GSTAR_GEAR_RATIO_BOUNDARY_v1.md` | `E87EB15B482AFBBF1147726B3F07C4008B82BC07B06BD9786656BEA28AD3BDDA` |
| `proof_oriented_even_self_pair_rectifier_gstar_gear_ratio_boundary.py` | `4627E99F50AA011B5C1FBF439681FB68B60CB341E4E87C9840DB3FB84D6ED0A3` |
| `THEOREM_COMMON_RELATIVE_CONNECTION_AND_MOMENTUM_GEARBOX_BOUNDARY_v1.md` | `3E2895157741C19DC8603E92E31A71933BFDAAF5B35062DFCE2F92404F8B9542` |
| `proof_common_relative_connection_momentum_gearbox_boundary_v3.py` | `9F3988F6DB0996FC81F856FEAFEF4B50A2B49190877E8BC4AEE3D59D26BB0E43` |
| `THEOREM_BLOCH_QUASIMOMENTUM_LIFT_AND_LOCAL_MOMENTUM_MAP_TRILEMMA_v1.md` | `0C2F0C289C82D45457B5DF330F767C10AD5CA3966FB667B329391C283FD47973` |
| `proof_bloch_quasimomentum_lift_local_momentum_map_trilemma_v3.py` | `62CB476D4A5F545B03A286E6C29B7710870E90802606C5DA7561F32397AA59FC` |

The certificate fails closed on source drift.

## 3. Registered minimum connection class

At one declared local gate phase let `g in {0,1}` and let `u` be the live
nonzero primitive current, not a target direction. Register maps

\[
 A_g(q,u)=g B(q)u                                             \tag{2}
\]

with the following restrictions:

- `A_g` is linear in the polar vector `u`;
- it is equivariant under all 48 signed cubic transformations;
- it is polynomial of degree at most two in the scalar clock coordinate `q`;
- it is even under `q -> -q`;
- `A_g(0,u)=0`; and
- its first `q` derivative vanishes at the critical point.

The certificate must prove that every such nonzero map is

\[
 \boxed{A_g(q,u)=g\gamma q^2u}                            \tag{3}
\]

for one real coefficient `gamma`. This is uniqueness inside the registered
lowest-degree class, not a derivation of `gamma`.

## 4. Registered source action and exact ledgers

For frozen `g,u`, register the conditional source action

\[
 L_g={M\over2}|\dot C|^2+{m\over2}\dot q^2
     +g\gamma q^2u\cdot\dot C-\lambda q^4,
 \qquad M,m,\lambda>0.                                  \tag{4}
\]

Its canonical momentum, mechanical momentum, and Hamiltonian are

\[
 P=M\dot C+g\gamma q^2u,
 \qquad K=P-g\gamma q^2u,                               \tag{5}
\]

\[
 H_g={|P-g\gamma q^2u|^2\over2M}
     +{\pi^2\over2m}+\lambda q^4.                       \tag{6}
\]

The certificate must establish positivity, conservation of `P`, and

\[
 \boxed{\Delta K=-g\gamma\Delta(q^2)u}.                \tag{7}
\]

At `P=0`, define

\[
 \Lambda_u=\lambda+{g\gamma^2|u|^2\over2M}>0.          \tag{8}
\]

Then the exact rest-sector energy is

\[
 \boxed{H_{g,P=0}={\pi^2\over2m}+\Lambda_u q^4}.       \tag{9}
\]

The pure critical quartic and its conditional continuum traversal therefore
survive. For turning amplitude `a`, the full-cycle common displacement is

\[
 \boxed{
 \Delta C_T=-{4\sqrt\pi\,g\gamma a\over M G^*}
 \sqrt{m\over2\Lambda_u}\,u.}                         \tag{10}
\]

Equation (10) must be treated as an exact identity inside (4), not as a
derived integer hop or a normalization of `gamma`.

Switching `g` at `q=0` must have zero instantaneous Hamiltonian cost. Away
from `q=0`, the exact switching difference

\[
 \Delta H_{g\to g'}=
 {|P-g'\gamma q^2u|^2-|P-g\gamma q^2u|^2\over2M}       \tag{11}
\]

is generally nonzero and must remain booked.

## 5. Frozen direct-composition test

Suppose one full source cycle produces a nonzero integer displacement `d`
while returning `(q,pi,P,K)` and every source/internal energy store to its
initial value. Locality forbids simultaneous translation of the already
formed extended companion. FTD-0933 then gives

\[
 \overline{\mathcal D}(d)
 =\|\pi(d)Y-Y\|_4^2>0.                                 \tag{12}
\]

The certificate must test the direct sum of the closed source cycle and the
unchanged field transaction. If no incoming/environmental store changes,

\[
 \boxed{\Delta E_{\rm total}=\overline{\mathcal D}(d)>0.} \tag{13}
\]

Thus simple juxtaposition is not a common action. Exact closure requires a
backreacting source-field coupling that reduces source/internal/incoming
energy by (12), changes the source cycle, or prevents the hop. A post-hoc
scalar debit is bookkeeping, not a derivation of that coupling.

## 6. Frozen recoil-identifiability test

Even if all wake energy is assigned to a reciprocal impulse `+I,-I` along
the live direction, positive quadratic source and field inertias give

\[
 \overline{\mathcal D}(d)
 ={I^2\over2M_s}+{I^2\over2M_f}
 ={I^2\over2\mu},
 \qquad {1\over\mu}={1\over M_s}+{1\over M_f}.          \tag{14}
\]

Hence

\[
 |I|=\sqrt{2\mu\,\overline{\mathcal D}(d)}.            \tag{15}
\]

The direction and wake fix the sign axis and a scalar energy, but `mu`
remains independent. Distinct positive `mu` give distinct impulses while
preserving the same `u`, character, wake, cubic covariance, energy balance,
and total-momentum cancellation.

Independently, the compact character admits

\[
 P_{\rm candidate}=p_*[k+2\pi W]                       \tag{16}
\]

for every `p_*>0`. Therefore neither `gamma`, `mu`, `p_*`, nor the local owner
of `W` is determined by the registered data. `i` and `u` orient the gearbox;
they do not normalize it.

## 7. Registered outcomes

- **Outcome A — minimum connection plus exact composition obstruction:**
  equations (3)--(16) pass. The live primitive current closes the conditional
  orientation input of the lowest-degree source connection, but the naive
  closed source cycle cannot be composed with the positive field wake. Real
  reciprocal impulse remains non-identifiable without a backreacting common
  source-field action, inertial/impulse scale, and carry owner.
- **Outcome B — conditional connection only:** the connection and source
  ledgers pass, but the strict wake or scale-family discriminator fails. No
  composition obstruction or recoil-identifiability theorem is licensed.
- **Outcome C — registered synthesis fails:** cubic uniqueness, critical-
  quartic preservation, or source ledger fails. The candidate is archived as
  closed negative.
- **Invalid:** source drift, post-lock formula change, numerical search, fit,
  tolerance repair, target direction, hidden environment debit, promotion of
  `gamma`, `mu`, or `p_*`, production mutation, new type adoption,
  `G*`/Born/context/outcome read, or completed-infinity rhetoric.

## 8. Firewalls and next gate

No engine source, CMake target, `Voxel` field, toggle, default, production
law, ontology type, import, physical constant, phenomenological formula, Born
weight, Bell correlation, measurement context, outcome, or `G*` cadence is
changed.

Even Outcome A does not derive an actual hop, backreacting field coupling,
physical impulse, total momentum map, carry hardware, `gamma`, `p_*`, mass,
source formation, robust recovery, production behavior, Lorentz hiding, or
completeness.

The next admissible gate is a local source-centered field action. It must use
the live gated `u_n`, contain the compact source-field coupling before the
hop, generate the field wake by its own equations, debit a named source or
incoming store without reading (12) as a target, derive equal/opposite torus-
momentum transfer with explicit carry ownership, and keep every dimensional
normalization symbolic unless independently fixed.
