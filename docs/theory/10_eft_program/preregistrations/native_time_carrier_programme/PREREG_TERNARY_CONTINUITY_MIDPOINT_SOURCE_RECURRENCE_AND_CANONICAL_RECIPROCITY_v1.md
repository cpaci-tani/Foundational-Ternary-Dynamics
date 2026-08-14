# FTD-0927 — Ternary continuity, midpoint-source recurrence, and canonical reciprocity v1

**Identifier:** `FTD-0927`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** target-blind local generation of the FTD-0925/0926 ternary record,
live current, midpoint Hodge field source, and affine field recurrence;
positive total reference ledger and a frozen mixed-partial test of whether the
same minimum canonical interaction can supply reciprocal velocity recoil; no
numerical search, fitted coefficient, engine mutation, new ontology type, or
`G*`/Born/Bell read

## 1. Question

FTD-0926 closes the local remainder–velocity gearbox but still supplies the
four ternary record snapshots by hand. Can central continuity itself generate
the next record and its exact midpoint field source from the current state?
If so, does the same local differentiable Hamiltonian coupling act
reciprocally on the canonical velocity, or does it act on the remainder
equation instead?

The certificate must separate:

1. an autonomous compositional reference recurrence;
2. exact scalar energy closure; and
3. derivation from one reciprocal common action.

The first two do not imply the third.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `THEOREM_LOCAL_REMAINDER_VELOCITY_C4_HAMILTONIAN_AND_FORMATION_BOUNDARY_v1.md` | `60DFDF4F3FDB13151D66E2128AA14FB92318D619ABD5506D98A22B75EDCC39F3` |
| `proof_local_remainder_velocity_c4_hamiltonian_formation_ledger.py` | `F2E53AA3180816AE0732663E6DC5180EFFE419C864B5310E0E400DFC6B81007E` |
| `THEOREM_RADIUS_TWO_CAUSAL_TERNARY_BRIDGE_SCAFFOLD_AFFINE_C4_FIELD_AND_AUTONOMY_BOUNDARY_v1.md` | `581D41914A0E60D1E2AAB5CC6D212FE8395F2AA20D52C91C9E6A01DB059CED39` |
| `proof_radius_two_causal_ternary_bridge_scaffold_affine_c4_field.py` | `62F7E3B5EA37FD8B00CC736CF2A507260313D8F5724E1A0562CEB4B870F9E1DC` |
| `THEOREM_NATIVE_HODGE_ENERGY_CONTINUITY.md` | `7849BFF214225723BFA52EA9034C34B22B94D204A78BE1D6DC6F97D065222868` |
| `AUDIT_NATIVE_FIELD_DISCRETE_ACTION.md` | `5EDC7F8C81456BEE4EEB061168154E8EF4D8347B8948C429BB40B8306FFC8AD8` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |

The certificate fails closed on source drift.

## 3. Frozen present-state record law

Let `rho_n=h+s_n` be the complete ternary record and let the live current be

\[
 Q_n=\rho_n v_n.                                           \tag{1}
\]

The candidate update is exactly central continuity:

\[
 \boxed{\rho_{n+1}
 =\rho_n-\operatorname{div}_c Q_n}.                        \tag{2}
\]

Equation (2) may read only `rho_n` and `v_n` through (1). It may not read the
registered next record, the desired arm, site coordinates, `G*`, context,
outcome, or Born weight.

The certificate must prove on all four registered arms:

- equation (2) equals the FTD-0925 next record exactly;
- every output remains in `{-1,0,+1}`;
- neutrality and 22-site support are retained;
- the record and remainder–velocity fields return together after four steps;
- the update is radius-one local under the frozen central divergence; and
- the resulting current remains the exact 19-site causal current.

The scope ceiling is mandatory: equation (2) need not preserve ternarity on
arbitrary inputs. A fixed rational-current counterexample must demonstrate
whether the registered ternary orbit is an invariant section rather than a
universal production law.

## 4. Frozen target-blind midpoint source

Define

\[
 \bar\rho_n={\rho_n+\rho_{n+1}\over2}
 =\rho_n-\frac12\operatorname{div}_c Q_n.                  \tag{3}
\]

The exact Hodge source is

\[
 U_n=-G_C\nabla_c\bar\rho_n+G_C\operatorname{curl}_c Q_n.
                                                                    \tag{4}
\]

Substituting (3) gives the present-state-only form

\[
 \boxed{
 U_n=-G_C\nabla_c\rho_n
 +{G_C\over2}\nabla_c\operatorname{div}_cQ_n
 +G_C\operatorname{curl}_cQ_n}.                            \tag{5}
\]

The certificate must derive (5), not merely compare sampled values. It must
then verify exact equality on the registered finite fields, `C4` covariance,
the invariant scaffold plus antipodal dynamic-doublet split, finite support,
and the absence of a next-record read.

Production uses the first and third terms of (5), not the midpoint correction.
No production-equivalence claim is permitted.

## 5. Frozen affine field recurrence

Let `K` be the symmetric free-field operator. Decompose the field into an
invariant static part `H` and a dynamic `C4` doublet `F_n`:

\[
 J_n=H+F_n,\qquad
 P_n=F_n+F_{n+1}.                                          \tag{6}
\]

The source generated by (5) decomposes as

\[
 U_n=KH+(K-2I)F_n.                                        \tag{7}
\]

The frozen kick–drift is

\[
 P_{n+1}=P_n-KJ_n+U_n,\qquad
 J_{n+1}=J_n+P_{n+1}.                                     \tag{8}
\]

The certificate must prove (8) advances (6) exactly and that all source
inputs in (7) are generated from the present record/current through (2) and
(5). The inherited outside-band resolvent and positive free-field band may
be used only under their frozen FTD-0923/0925 assumptions.

## 6. Frozen energy and formation ledger

The certificate must combine:

- the FTD-0926 remainder–velocity invariant `52/25`;
- the positive field invariant

  \[
  H_f(J,P)=\frac12\langle P,P\rangle
  +\frac12\langle J,KJ\rangle
  -\frac12\langle P,KJ\rangle;
  \]

- the exact Hodge interaction/matter-work cancellation; and
- the static halo.

It must prove the modeled total is constant and positive on the reference
orbit. The formation debit must include the rotating field energy as well as
the static halo and the unit-tick carrier debit `26 pi/25`. Ternary
manifestation energy and the paying reservoir remain open unless independently
derived.

## 7. Frozen canonical-reciprocity class

Use the FTD-0926 onsite canonical pair `(r,v)`. In the minimum registered
interaction class:

1. `H_int(r,v,R,rho)` is differentiable and local;
2. its field variation produces a prescribed source
   `S(rho,v)` independent of `r` and of the field coordinate `R`; and
3. no target-orbit-vanishing correction or hidden phase field is added.

Up to a matter-only term `C(r,v,rho)`, this fixes

\[
 H_{\rm int}=-\langle R,S(\rho,v)\rangle+C(r,v,\rho).       \tag{9}
\]

The certificate must test the mixed partials:

\[
 {\partial\over\partial R}
 \left(-{\partial H_{\rm int}\over\partial r}\right)
 ={\partial S\over\partial r}=0,                           \tag{10}
\]

\[
 {\partial\over\partial R}
 \left({\partial H_{\rm int}\over\partial v}\right)
 =-{\partial S\over\partial v}.                            \tag{11}
\]

Thus a source depending on `v` but not `r` reciprocates into the canonical
`dot r` equation, not the `dot v` equation. If (10)–(11) hold, the existing
Hodge source cannot be claimed to derive the FTD-0926 velocity impulse inside
this minimum common-action class.

This is a scoped theorem. It does not exclude a discrete generating function,
an `r`-dependent current, a changed canonical identification, a bond/link
current coordinate, or an enlarged source action.

## 8. Frozen outcomes

- **Outcome A — reciprocal common recurrence:** record, source, field,
  remainder, and velocity close exactly, and the same minimum local canonical
  interaction generates both field source and reciprocal velocity update.
- **Outcome B — autonomous compositional recurrence / reciprocity boundary:**
  equations (1)–(8) close target-blindly with positive scalar energy, but
  (10)–(11) prove that the frozen canonical source coupling acts back on
  `dot r` rather than `dot v`. Book the coupled reference recurrence and the
  precise common-action debt separately.
- **Outcome C — record/source closure fails:** equation (2) leaves the
  registered ternary orbit or equation (5) fails to reproduce the midpoint
  source.
- **Invalid:** source drift, altered scaffold/current, post-lock coefficient
  change, target-record read, fitted tolerance, failed combined gate, or
  production mutation.

## 9. Firewalls

No parameter sweep, near-miss search, formula-substitution discovery,
production execution, engine source, CMake target, toggle, default, import,
current type, or selected ontology type may change.

Even Outcome A would not establish spontaneous formation, a paying reservoir,
asymptotic recovery, mobility, physical scale, critical quarticity, `G*`
cadence, Born frequencies, Bell correlations, measurement context, or
operational hiding. Outcome B additionally leaves the reciprocal common
action and vector recoil open.
