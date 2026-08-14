# FTD-0936 — Preregistration: C4 character parity kernel, primitive direction, and compact-body orbit v1

**Identifier:** `FTD-0936`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** substantive parity-kernel correction to the universal-direction
reading of FTD-0935; canonical primitive-ray repair; exact application to the
FTD-0925/0926 formed reference scaffold's integrated current orbit; phase-
blind cancellation and gate boundary; no numerical search, fit, post-hoc
tolerance, production promotion, robust-memory claim, new type, physical
momentum, recoil, `G*`, Born, Bell, context, outcome, or hiding read

## 1. Question and correction trigger

FTD-0935 proves that

\[
 \Xi_{\chi a}(d)=i^{\chi a\cdot d}                    \tag{1}
\]

is always a `C4` character. Its Moore-shell test uses one-step vectors and
correctly detects orientation globally whenever a component of `a` is odd.

The formed FTD-0925 source dipole instead has endpoints `+e_x` and `-e_x`, so
its raw displacement is `a=2e_x`. Equation (1) then gives only `+/-1` and
cannot distinguish `chi` from `-chi`. The present protocol freezes the exact
parity kernel before any correction is issued.

It asks:

1. exactly when does the raw character distinguish `a` from `-a`;
2. is division by the coordinate gcd the canonical integer repair;
3. does the already-formed compact scaffold contain an independent primitive
   time-odd polar current label; and
4. can that label select net motion without a phase gate or common action?

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `PREREG_NATIVE_BILATERAL_C4_TRANSLATION_CHARACTER_AND_MOORE_SHELL_PARITY_BOUNDARY_v1.md` | `19512CF3431EF65DD65E88A53C14BA835681D2A29099B9DEAB81DB03D67B0CCA` |
| `THEOREM_NATIVE_BILATERAL_C4_TRANSLATION_CHARACTER_AND_MOORE_SHELL_PARITY_BOUNDARY_v1.md` | `EB639E6183E5972CDBF3FC7817CC8E8F4E51669119D988AD2C89200157A27D78` |
| `proof_native_bilateral_c4_translation_character_moore_shell_parity_boundary.py` | `D24F44FA80D34AC8F45A2C6330AF2E35CC86BEABF56AF028609AA154F4D86DE4` |
| `THEOREM_RADIUS_TWO_CAUSAL_TERNARY_BRIDGE_SCAFFOLD_AFFINE_C4_FIELD_AND_AUTONOMY_BOUNDARY_v1.md` | `581D41914A0E60D1E2AAB5CC6D212FE8395F2AA20D52C91C9E6A01DB059CED39` |
| `proof_radius_two_causal_ternary_bridge_scaffold_affine_c4_field.py` | `62F7E3B5EA37FD8B00CC736CF2A507260313D8F5724E1A0562CEB4B870F9E1DC` |
| `THEOREM_LOCAL_REMAINDER_VELOCITY_C4_HAMILTONIAN_AND_FORMATION_BOUNDARY_v1.md` | `60DFDF4F3FDB13151D66E2128AA14FB92318D619ABD5506D98A22B75EDCC39F3` |
| `proof_local_remainder_velocity_c4_hamiltonian_formation_ledger.py` | `F2E53AA3180816AE0732663E6DC5180EFFE419C864B5310E0E400DFC6B81007E` |

The certificate fails closed on source drift.

## 3. Frozen parity-kernel theorem

For `p in Z^3`, define

\[
 \Xi_p(d)=i^{p\cdot d}.
\]

Register orientation sensitivity as the existence of at least one integer
translation `d` for which

\[
 \Xi_p(d)\ne\Xi_{-p}(d).                               \tag{2}
\]

The certificate must prove the equivalences

\[
 \boxed{
 \Xi_p=\Xi_{-p}
 \Longleftrightarrow
 2p=0\pmod4
 \Longleftrightarrow
 p\in(2\mathbb Z)^3.}                                 \tag{3}
\]

Thus the raw FTD-0935 character is directional exactly when at least one
component of the displacement is odd. Its existence as a character is
unconditional; its ability to retain the conjugation sign is not.

## 4. Frozen primitive-direction repair

For every nonzero integer vector `a`, define

\[
 g(a)=\gcd(|a_1|,|a_2|,|a_3|)>0,
 \qquad
 \operatorname{prim}(a)={a\over g(a)}.                \tag{4}
\]

Then register

\[
 \boxed{p_4^{\rm prim}=\chi\operatorname{prim}(a),}
 \qquad
 \boxed{\Xi^{\rm prim}(d)=i^{p_4^{\rm prim}\cdot d}.} \tag{5}
\]

The certificate must prove:

- `prim(a)` is integer and has coordinate gcd one;
- at least one component is odd, so (5) is orientation sensitive;
- `prim(-a)=-prim(a)`;
- for every signed permutation `Q`, `prim(Qa)=Q prim(a)`;
- the FTD-0935 ordered-presentation and time-reversal laws therefore survive;
  and
- `prim(a)` is the unique primitive integer representative of the oriented
  rational ray through `a`.

The repair deliberately removes separation multiplicity. It supplies a
compact directed ray, not physical momentum magnitude.

## 5. Frozen compact-body current character

Use the exact FTD-0925 distributed current orbit. Its integrated currents are

\[
 P_n=\sum_xj_n(x),
\]

with

\[
 P_0=2(e_y-e_x),
 \qquad P_{n+1}=SP_n,
 \qquad P_{n+2}=-P_n,                                 \tag{6}
\]

where

\[
 S=\begin{pmatrix}0&-1&0\\1&0&0\\0&0&1\end{pmatrix},
 \qquad S^2=-I\ \text{on the }xy\text{ plane}.       \tag{7}
\]

Define the primitive body labels

\[
 \boxed{u_n=\operatorname{prim}(P_n)}                 \tag{8}
\]

and their characters

\[
 \boxed{\Xi_n(d)=i^{u_n\cdot d}.}                    \tag{9}
\]

The exact orbit must be

\[
 u_0=(-1,1,0),\quad
 u_1=(-1,-1,0),\quad
 u_2=(1,-1,0),\quad
 u_3=(1,1,0),                                         \tag{10}
\]

with

\[
 u_{n+1}=Su_n,
 \qquad u_{n+2}=-u_n,
 \qquad u_{n+4}=u_n.                                 \tag{11}
\]

Because current is a time-odd polar field, `P_n` and `u_n` are time-odd polar
vectors. The character covariance is

\[
 \boxed{\Xi_{n+1}(Sd)=\Xi_n(d).}                     \tag{12}
\]

FTD-0926's prepared remainder/velocity Hamiltonian generates the four current
arms exactly and returns after four steps. This licenses an exact prepared
reference character orbit. It does not license production formation,
attraction, or perturbation-robust protection.

## 6. Frozen phase-blind boundary

The current label has zero full-cycle mean:

\[
 \boxed{\sum_{n=0}^3u_n=0.}                           \tag{13}
\]

Therefore every phase-blind linear vector readout of the complete C4 cycle
vanishes. A nonzero directed export must use at least one of:

1. a preregistered phase gate selecting one `u_n`;
2. an additional retained polar bias/body axis;
3. an incoming time-odd polar current; or
4. a nonlinear common action with an independently audited directional state.

A selected gate can expose (9) stroboscopically but does not derive a physical
impulse. `G*` may control gate eligibility only after a separate finite-tick
clock law exists; it cannot choose the outcome direction or create recoil.

## 7. Registered outcomes

- **Outcome A — parity correction plus exact prepared body character:**
  equations (3)--(13) pass. FTD-0935 remains correct as a character theorem
  but its universal directed reading is narrowed by (3). Primitive
  normalization gives the canonical directed ray for every nonzero integer
  displacement. The formed reference scaffold independently carries the
  exact rotating primitive character orbit (10), but phase-blind export
  cancels and production/robustness/common action remain open.
- **Outcome B — parity correction only:** equation (3) passes but primitive
  uniqueness or the body orbit fails. No body-character claim is licensed.
- **Outcome C — FTD-0935 direction reading invalid without repair:** the
  parity kernel passes but no registered repair satisfies the declared
  covariance and orientation gates.
- **Invalid:** source drift, post-lock formula change, numerical search, fit,
  tolerance repair, concealment of the even-displacement counterexample,
  production promotion, robust-memory claim, physical momentum/recoil
  promotion, new type adoption, engine/CMake mutation, `G*`/Born/context read,
  or completed-infinity rhetoric.

## 8. Firewalls and next gate

No engine source, CMake target, `Voxel` field, toggle, default, production
law, ontology type, import, physical constant, phenomenological formula, Born
weight, Bell correlation, measurement context, or `G*` cadence is changed.

Even Outcome A does not derive a phase gate, unwrapped momentum, winding/carry
owner, `p_*`, `gamma`, common action, vector recoil, net displacement, source
formation, attraction, robust recovery, production behavior, Lorentz hiding,
or completeness.

The next admissible gate is an exact common-action classifier coupling one
stroboscopically selected `u_n` to the FTD-0933 dressing cocycle. It must
derive equal-and-opposite source/field impulse and prove whether the four
gate choices are dynamically equivalent or require an additional phase
selection.
