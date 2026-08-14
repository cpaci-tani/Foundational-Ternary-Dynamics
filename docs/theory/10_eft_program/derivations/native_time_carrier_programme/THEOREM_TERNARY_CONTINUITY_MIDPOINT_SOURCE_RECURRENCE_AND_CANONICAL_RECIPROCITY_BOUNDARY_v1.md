# FTD-0927 — Ternary continuity, midpoint-source recurrence, and canonical reciprocity boundary v1

**Identifier:** `FTD-0927`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — TARGET-BLIND LOCAL RECORD/MIDPOINT-SOURCE RECURRENCE]` +
`[REFERENCE CONSTRUCTION — AUTONOMOUS COMPOSITIONAL RECORD–MATTER–FIELD C4 BODY]` +
`[THEOREM — POSITIVE CONSTANT SCALAR LEDGER]` +
`[SCOPED NO-GO — MINIMUM CANONICAL FIELD-TO-VELOCITY RECIPROCITY]` +
`[OPEN — COMMON ACTION/FORMATION RESERVOIR/PRODUCTION]`  
**Protocol:**
[`PREREG_TERNARY_CONTINUITY_MIDPOINT_SOURCE_RECURRENCE_AND_CANONICAL_RECIPROCITY_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_TERNARY_CONTINUITY_MIDPOINT_SOURCE_RECURRENCE_AND_CANONICAL_RECIPROCITY_v1.md),
SHA-256 `A48B11D59D2EEE49FFCA7E9CF7116A8D49E1175B6FEAC50C781201CACA5BE19C`  
**Certificate:**
[`proof_ternary_continuity_midpoint_source_recurrence_canonical_reciprocity.py`](../../../../../scripts/proofs/proof_ternary_continuity_midpoint_source_recurrence_canonical_reciprocity.py),
SHA-256 `E0A03721A089B43137EC986E1EB2024D9AF93B43062603B4C23FF5CA32E806B9`,
`144/144` exact checks  
**Registered outcome:** `B — AUTONOMOUS COMPOSITIONAL RECURRENCE / CANONICAL RECIPROCITY BOUNDARY`

---

## 1. Result

The registered FTD-0925/0926 four-arm body no longer requires its next
ternary record or its midpoint field source to be supplied by a target table.
With complete record `rho_n`, velocity `v_n`, and live current

\[
 Q_n=\rho_n v_n,
\]

the radius-one central-continuity update

\[
 \boxed{\rho_{n+1}=\rho_n-\operatorname{div}_c Q_n}       \tag{1}
\]

generates the exact next registered record on all four arms. Each generated
record is neutral, ternary, and supported on 22 sites. The current remains the
registered 19-site causal current, and record, remainder, and velocity return
together after four updates.

The midpoint source is also a present-state law. Since

\[
 \bar\rho_n=\frac{\rho_n+\rho_{n+1}}2
 =\rho_n-\frac12\operatorname{div}_cQ_n,
\]

the direct midpoint Hodge source is identically

\[
 \boxed{
 U_n=-G_C\nabla_c\rho_n
 +\frac{G_C}{2}\nabla_c\operatorname{div}_c Q_n
 +G_C\operatorname{curl}_c Q_n.}                         \tag{2}
\]

Equation (2) exactly reproduces the direct midpoint expression on every arm,
using only the current record and current. Its static component is `C4`
invariant; its dynamic component is a rotating, antipodal `C4` doublet.

This closes an autonomous **compositional reference recurrence**. It does not
close one reciprocal common action. In the minimum differentiable canonical
class frozen by the preregistration, the field coupling that produces a
velocity-dependent source acts back on `dot r`, not on `dot v`. That precise
obstruction is the new mechanics boundary.

---

## 2. Exact record recurrence

### 2.1 Registered invariant section

Write the complete record as

\[
 \rho_n=h+s_n,
\]

where `h` is the neutral 20-site `C4`-invariant scaffold and `s_n` is the
rotating two-site ternary dipole. The FTD-0925 current obeys

\[
 s_{n+1}-s_n+\operatorname{div}_cQ_n=0.
\]

Because `h` is invariant,

\[
 \rho_{n+1}-\rho_n=s_{n+1}-s_n,
\]

and equation (1) follows. The certificate reconstructs `Q_n` from the live
product `rho_n v_n`; it does not read the next arm.

The simultaneous FTD-0926 onsite update is

\[
 v_{n+1}=v_n-2r_n,
 \qquad
 r_{n+1}=r_n+v_{n+1}=v_n-r_n.                            \tag{3}
\]

Starting from

\[
 r_0=\frac{v_0-v_1}{2},
\]

equations (1) and (3) generate the same four-arm phase without a site label,
target arm, hidden phase field, or outcome/context read.

### 2.2 Locality

At site `x`, equation (1) reads `rho_n(x)` and the six face-neighbor currents
`Q_n(x±e_i)`. Each current is the colocated product `rho_n v_n`. The update is
therefore radius-one local under the registered central stencil.

This is locality of the update operator. The particular compact body remains
the selected radius-two scaffold/current construction inherited from
FTD-0925.

### 2.3 Mandatory scope ceiling

Equation (1) is **not** a universal map from arbitrary ternary records and
rational currents back to ternary records. The locked counterexample

\[
 \rho(0)=1,
 \qquad
 Q(0)=\left(\frac15,0,0\right)
\]

produces values `-1/10` and `+1/10` at the two neighboring x-sites.

Therefore:

- `[THEOREM]` the registered four-arm body is an exact ternary invariant
  section of equation (1);
- `[OPEN]` a universal production law selecting or attracting such sections
  has not been derived.

No universal ternary-closure claim is permitted.

---

## 3. Present-state midpoint source

Substitution of equation (1) into the midpoint is algebraic:

\[
 \begin{aligned}
 U_n
 &=-G_C\nabla_c\left(\rho_n-\frac12\operatorname{div}_cQ_n\right)
   +G_C\operatorname{curl}_cQ_n\\
 &=-G_C\nabla_c\rho_n
   +\frac{G_C}{2}\nabla_c\operatorname{div}_cQ_n
   +G_C\operatorname{curl}_cQ_n.
 \end{aligned}
\]

This is an operator identity, not a numerical match. The exact finite-field
certificate separately confirms it on all four registered arms.

For `G_C=1`, the reconstructed source has:

| Quantity | Exact result |
|---|---:|
| static-source support | 45 sites |
| dynamic-source support on arm 0 | 53 sites |
| dynamic-source squared norm | `463/100` |
| dynamic covariance | exact `C4` rotation |
| dynamic antipodality | `D_2=-D_0`, `D_3=-D_1` |
| static–dynamic inner product | zero on every arm |

Production currently contains the first and third terms of equation (2),
`-G_C grad(state)` and `+G_C curl(state*velocity)`. It does **not** contain
the midpoint correction

\[
 \frac{G_C}{2}\nabla_c\operatorname{div}_c(\rho v).
\]

Hence this theorem does not identify the reference recurrence with the live
engine.

---

## 4. Affine field recurrence

Use the inherited reference decomposition

\[
 J_n=H+F_n,
 \qquad
 P_n=F_n+F_{n+1},
\]

with invariant `H` and rotating doublet `F_{n+2}=-F_n`. Let `K` be the
symmetric free-field operator and encode the generated source sectors as

\[
 U_n=KH+(K-2I)F_n.                                       \tag{4}
\]

Then the kick–drift recurrence

\[
 P_{n+1}=P_n-KJ_n+U_n,
 \qquad
 J_{n+1}=J_n+P_{n+1}                                    \tag{5}
\]

is exact:

\[
 P_n-K(H+F_n)+KH+(K-2I)F_n
 =F_{n+1}-F_n=P_{n+1},
\]

and

\[
 J_n+P_{n+1}=H+F_{n+1}=J_{n+1}.
\]

`[REFERENCE CONSTRUCTION]` Equation (4) is the inherited abstract
invariant-plus-doublet representation. The concrete source sectors that feed
it are now generated by equation (2), rather than read from a target table.
This does not derive the outside-band resolvent, a physical normalization, or
production coupling.

---

## 5. Positive constant scalar ledger

The exact field invariant is

\[
 H_f(J,P)
 =\frac12\langle P,P\rangle
 +\frac12\langle J,KJ\rangle
 -\frac12\langle P,KJ\rangle.                            \tag{6}
\]

Its positive-band form is

\[
 H_f
 =\frac12\left\|P-\frac12KJ\right\|^2
 +\frac18\langle J,K(4I-K)J\rangle.                     \tag{7}
\]

For `0<K<4I`, equation (7) is positive. In the registered three-coordinate
reference witness, with

\[
 K=\operatorname{diag}(k_h,k_d,k_d),
\]

the exact value on every arm is

\[
 H_f=1+\frac{k_h}{2}.                                    \tag{8}
\]

Here `1` is the rotating-field component and `k_h/2` is the static-halo
component. Adding the FTD-0926 matter-carrier invariant gives

\[
 H_{\rm scalar}
 =\frac{52}{25}+1+\frac{k_h}{2}
 =\frac{77}{25}+\frac{k_h}{2}>0.                         \tag{9}
\]

At the selected unit-tick carrier normalization, the modeled formation debit
must therefore include

\[
 \boxed{
 E_{\rm form}^{\rm modeled}
 =\frac{26\pi}{25}+1+\frac{k_h}{2}.}                     \tag{10}
\]

This improves the previous ledger: the rotating field cannot be omitted from
formation bookkeeping.

`[THEOREM]` Equations (6)–(10) give a positive, constant scalar ledger on the
reference orbit.  
`[OPEN]` They do not identify the paying substrate reservoir, assign a
derived energy to ternary manifestation, or prove that one reciprocal action
generates all sectors.

---

## 6. Minimum canonical reciprocity obstruction

Use the FTD-0926 canonical pair `(r,v)` and a field coordinate `R`. Freeze the
minimum local differentiable interaction class in which field variation gives
a prescribed source `S(rho,v)` that is independent of `r` and `R`. Up to a
matter-only term,

\[
 H_{\rm int}(r,v,R,\rho)
 =-\langle R,S(\rho,v)\rangle+C(r,v,\rho).                \tag{11}
\]

Hamilton's equations are

\[
 \dot r=\frac{\partial H_{\rm int}}{\partial v},
 \qquad
 \dot v=-\frac{\partial H_{\rm int}}{\partial r}.
\]

The exact mixed partials are

\[
 \frac{\partial\dot v}{\partial R}
 =\frac{\partial S}{\partial r}=0,                       \tag{12}
\]

but

\[
 \frac{\partial\dot r}{\partial R}
 =-\frac{\partial S}{\partial v}.                        \tag{13}
\]

The Hodge source in equation (2) is nontrivially linear in `v` at fixed
`rho`, through

\[
 \frac12\nabla_c\operatorname{div}_c(\rho v)
 +\operatorname{curl}_c(\rho v).
\]

Consequently its reciprocal term is nonzero in equation (13), while equation
(12) remains zero. The source coupling changes the canonical remainder rate;
it cannot generate the FTD-0926 velocity impulse in this minimum class.

### Scope of the no-go

This theorem excludes only the frozen minimum class. It does not exclude:

- a discrete generating function rather than a differentiable continuous
  Hamiltonian coupling;
- a current depending explicitly on the remainder `r`;
- exchanging or changing the canonical identification of `r` and `v`;
- using a bond/link current as a canonical coordinate;
- enlarging the source action or phase space.

Those are the honest next branches. Adding a term that vanishes only on the
desired orbit would not be explanatory and remains forbidden by the
preregistration.

---

## 7. What is now closed and what remains open

### Closed on the selected reference body

1. `[THEOREM]` target-blind radius-one generation of the next complete record;
2. `[THEOREM]` exact ternary, neutral 22-site closure on the registered
   invariant section;
3. `[THEOREM]` present-state generation of the exact midpoint Hodge source;
4. `[REFERENCE CONSTRUCTION]` exact record–matter–field `C4` recurrence;
5. `[THEOREM]` positive constant scalar total and corrected formation debit;
6. `[SCOPED NO-GO]` the minimum canonical source coupling cannot generate the
   required velocity recoil.

### Still open

1. a common reciprocal action, likely through a discrete generating function
   or bond-current canonical coordinate;
2. a universal ternary production/selection law beyond the registered
   invariant section;
3. ternary manifestation energy and a paying formation reservoir;
4. perturbation attraction or asymptotic recovery of the coupled body;
5. mobility, physical scale, production insertion, and operational hiding;
6. any identification with the critical quartic clock or `G*` cadence;
7. Born frequencies, Bell correlations, measurement context, or actualization.

---

## 8. Interpretation

The missing dynamics are now narrower than “a field plus matter.” The local
body already has:

- a ternary record phase;
- an existing-type remainder–velocity phase;
- a field phase;
- exact local propagation among them;
- a positive scalar energy ledger.

What it lacks is the **reciprocal gearbox** that makes those equations the
variations of one natural action and the **reservoir transition** that pays to
form the body from a preceding state.

That distinction is ledgerable because every statement above has one of four
separate currencies: exact theorem, selected reference representation, scoped
no-go, or open recovery debt. The successful recurrence cannot be silently
promoted into spontaneous formation or common-action closure.

---

## 9. Reproduction

```bash
python scripts/proofs/proof_ternary_continuity_midpoint_source_recurrence_canonical_reciprocity.py
```

Expected terminal summary:

```text
FTD-0927 exact certificate: 144/144 checks passed
OUTCOME=B_AUTONOMOUS_COMPOSITIONAL_RECURRENCE_CANONICAL_RECIPROCITY_BOUNDARY
TERNARY_RECORD_UPDATE=EXACT_ALL_FOUR_ARMS
TERNARY_CLOSURE_SCOPE=REGISTERED_INVARIANT_SECTION_ONLY
GENERIC_TERNARY_CLOSURE=FALSE
MIDPOINT_SOURCE=PRESENT_STATE_LOCAL_EXACT
STATIC_SOURCE_SUPPORT=45
DYNAMIC_SOURCE_SUPPORT=53
DYNAMIC_SOURCE_NORM_SQUARED=463/100
FULL_RECORD_MATTER_FIELD_RECURRENCE=TARGET_BLIND_REFERENCE
SCALAR_TOTAL_ENERGY=POSITIVE_CONSTANT
FORMATION_DEBIT=26*pi/25+1+k_h/2
MINIMUM_CANONICAL_FIELD_TO_VELOCITY_RECOIL=OBSTRUCTED
RECIPROCAL_COUPLING_ENTERS=DOT_R_NOT_DOT_V
COMMON_RECIPROCAL_ACTION=OPEN
FORMATION_RESERVOIR=OPEN
PRODUCTION_CHANGED=FALSE
GSTAR_USED=FALSE
BORN_BELL_CONTEXT_USED=FALSE
```
