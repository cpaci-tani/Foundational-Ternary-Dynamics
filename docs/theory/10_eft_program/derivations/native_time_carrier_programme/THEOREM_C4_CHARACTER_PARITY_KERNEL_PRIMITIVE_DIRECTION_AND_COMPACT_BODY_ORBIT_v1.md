# FTD-0936 — C4 character parity kernel, primitive direction, and compact-body orbit v1

**Identifier:** `FTD-0936`  
**Date:** 2026-08-11  
**Status:** `[CORRECTION — RAW C4 CHARACTER IS DIRECTIONAL IFF SOME LABEL COMPONENT IS ODD]` +
`[THEOREM — CANONICAL PRIMITIVE-DIRECTION REPAIR]` +
`[THEOREM — EXACT FORMED-REFERENCE-BODY PRIMITIVE CURRENT CHARACTER ORBIT]` +
`[SCOPED NO-GO — PHASE-BLIND LINEAR VECTOR EXPORT]` +
`[BOUNDARY — PHASE GATE/COMMON ACTION/PHYSICAL MOMENTUM OPEN]`  
**Production status:** unchanged

## 1. Result and correction

FTD-0935 correctly constructs a compact `C4` character from any integer
directed label:

\[
 \Xi_p(d)=i^{p\cdot d}.                                \tag{1}
\]

But existence of a character is weaker than retention of direction. The
exact parity kernel is

\[
 \boxed{
 \Xi_p=\Xi_{-p}
 \Longleftrightarrow
 p\in(2\mathbb Z)^3.}                                 \tag{2}
\]

Therefore the raw FTD-0935 label `p_4=chi a` distinguishes its time-reversed
or conjugate branch exactly when the displacement `a` has at least one odd
component. The one-step Moore-shell results remain correct. The universal
directed reading for arbitrary nonzero integer separation is corrected.

The canonical repair is primitive normalization:

\[
 \boxed{
 p_4^{\rm prim}
 =\chi\operatorname{prim}(a),
 \qquad
 \operatorname{prim}(a)
 ={a\over\gcd(|a_1|,|a_2|,|a_3|)}.}                  \tag{3}
\]

Every nonzero primitive integer vector has at least one odd component, so
its character always distinguishes reversal.

The formed FTD-0925/0926 reference body supplies an independent realization.
Its integrated live-current orbit has primitive labels

\[
 \boxed{
 u_0=(-1,1,0),\quad
 u_1=(-1,-1,0),\quad
 u_2=(1,-1,0),\quad
 u_3=(1,1,0).}                                        \tag{4}
\]

These rotate by the spatial quarter-turn, are time-odd polar integer vectors,
and define an exact prepared `C4` character orbit. Their full-cycle sum is
zero. Consequently the internal clock body can carry direction at each phase,
but it cannot export a phase-blind net vector. A phase gate, retained bias,
incoming current, or nonlinear directed common action remains necessary.

## 2. Exact parity kernel

Two characters are equal precisely when they agree on the three basis
translations. For the label and its reverse,

\[
 \Xi_p(e_j)=i^{p_j},
 \qquad
 \Xi_{-p}(e_j)=i^{-p_j}.                               \tag{5}
\]

Equality requires

\[
 i^{2p_j}=1
 \Longleftrightarrow
 2p_j=0\pmod4
 \Longleftrightarrow
 p_j=0\pmod2                                           \tag{6}
\]

for every coordinate. This proves (2).

Equivalently, among the `4^3=64` modulo-four labels, the kernel contains the
eight all-even labels and the other 56 distinguish reversal on at least one
basis translation.

The formed source dipole makes the correction unavoidable. Its endpoints
are `+e_x` and `-e_x`, so

\[
 a=2e_x,
 \qquad
 \Xi_{2e_x}(e_x)=\Xi_{-2e_x}(e_x)=-1.                 \tag{7}
\]

Equation (1) still defines a valid nontrivial `C2` character, but the
clockwise/counterclockwise sheets coincide.

## 3. Canonical primitive-ray repair

For every nonzero `a in Z^3`, let

\[
 g(a)=\gcd(|a_1|,|a_2|,|a_3|)>0.                     \tag{8}
\]

Then `a/g(a)` is integer and has coordinate gcd one. If all of its components
were even, the gcd would be at least two, a contradiction. Hence

\[
 \operatorname{prim}(a)\notin(2\mathbb Z)^3           \tag{9}
\]

and its `C4` character is direction sensitive by (2).

The construction has the exact covariance laws

\[
 \operatorname{prim}(-a)=-\operatorname{prim}(a),     \tag{10}
\]

\[
 \operatorname{prim}(Qa)=Q\operatorname{prim}(a)      \tag{11}
\]

for every signed permutation `Q`, because signed permutations preserve the
coordinate gcd. Under ordered-endpoint reversal both `chi` and
`prim(a)` reverse, so their product remains invariant. Under canonical time
reversal only `chi` reverses, so `p_4^prim` is time odd.

Primitive normalization is canonical on the declared ray: if `b` is a
primitive integer vector on the same oriented rational ray as `a`, then
`b=prim(a)`. It removes only positive integer separation multiplicity. It
does not derive physical momentum magnitude or the conversion scale `p_*`.

For the even source displacement in (7),

\[
 \operatorname{prim}(2e_x)=e_x,
 \qquad
 \Xi_{e_x}(e_x)=i,
 \qquad
 \Xi_{-e_x}(e_x)=-i.                                  \tag{12}
\]

The conjugate sheets are restored.

## 4. Existing formed-body current label

The FTD-0925 radius-two scaffold compiles the live current through existing
ternary state and velocity:

\[
 j_n(x)=r_n(x)v_n(x).                                  \tag{13}
\]

Its exact integrated current is

\[
 P_n=\sum_xj_n(x).                                     \tag{14}
\]

The registered five-channel construction gives

\[
 P_0=2(e_y-e_x)=(-2,2,0).                              \tag{15}
\]

Let

\[
 S=\begin{pmatrix}
 0&-1&0\\
 1&0&0\\
 0&0&1
 \end{pmatrix}.                                       \tag{16}
\]

Because the complete current rotates as `j_(n+1)=S j_n`, summation commutes
with the rotation:

\[
 P_{n+1}=SP_n,
 \qquad P_{n+2}=-P_n.                                 \tag{17}
\]

The exact sequence is

\[
 P_n=2\bigl(
 (-1,1,0),
 (-1,-1,0),
 (1,-1,0),
 (1,1,0)
 \bigr).                                               \tag{18}
\]

Applying primitive normalization gives (4). No bilateral endpoint pairing or
phase-wedge memory is needed for this body-level label.

Since current is a polar vector field and reverses under canonical time
reversal, both `P_n` and `u_n` are time-odd polar observables. Their characters

\[
 \boxed{\Xi_n(d)=i^{u_n\cdot d}}                      \tag{19}
\]

obey

\[
 \boxed{\Xi_{n+1}(Sd)=\Xi_n(d)}                       \tag{20}
\]

and

\[
 \Xi_{n+2}(d)=\overline{\Xi_n(d)}.                   \tag{21}
\]

This is the compact spatial-character orbit that FTD-0934 required.

## 5. What the remainder–velocity Hamiltonian supplies

FTD-0926 uses the existing onsite remainder/velocity pair and the homogeneous
map

\[
 \begin{pmatrix}r'\\v'\end{pmatrix}
 =\mathsf M\begin{pmatrix}r\\v\end{pmatrix},
 \qquad
 \mathsf M=
 \begin{pmatrix}-1&1\\-2&1\end{pmatrix}.             \tag{22}
\]

It obeys

\[
 \mathsf M^2=-I,
 \qquad
 \mathsf M^4=I.                                       \tag{23}
\]

On the prepared FTD-0925 scaffold, this generates the exact four velocity and
current arms and therefore repeats (4) after four steps. The formed reference
body consequently has enough existing state to carry the compact character
recursively.

This is not a production or robustness result. Equation (22) remains a
selected reference force, the ternary scaffold remains prepared, and the
period-four map is neutrally stable rather than attracting. Generic real
perturbations also need not preserve an integer integrated-current ray.

The correct status is **exact prepared reference recurrence**, not protected
production memory.

## 6. Phase-blind export obstruction

Directly from (4),

\[
 \boxed{u_0+u_1+u_2+u_3=0.}                           \tag{24}
\]

Therefore a linear vector export that weights all four internal phases
equally is zero. This is the body-level version of the earlier unrectified
clock result: recurrence and orientation do not by themselves produce drift.

A nonzero directed event may use a preregistered phase gate:

\[
 n=n_g\pmod4
 \quad\Longrightarrow\quad
 u_{n_g}\ne0.                                         \tag{25}
\]

But (25) exposes a direction; it does not generate an impulse or pay the
field wake. The allowed logical separation is:

1. the internal C4 recurrence supplies `u_n`;
2. a clock/gate law determines when one phase is eligible;
3. a common action determines whether and how source and field exchange
   equal-and-opposite momentum; and
4. a physical scale maps the compact character to an impulse.

`G*` could enter step 2 only through a separately derived finite-tick critical
quartic clock. It cannot choose `u_n`, create step 3, or normalize step 4.

## 7. Reconciliation with FTD-0935 and the Moore shells

FTD-0935 remains theorem-grade for:

- the ordered-presentation/time/cubic transformation of `chi a`;
- existence and covariance of `Xi_(chi a)`;
- the signed-cubic integer-linear centralizer `m I_3`; and
- the exact one-step Moore-shell self-phase table.

FTD-0936 corrects only the stronger reading that every nonzero raw separation
produces a directed character. The corrected statement is:

\[
 \boxed{
 \text{raw }\Xi_{\chi a}\text{ is directed}
 \Longleftrightarrow a\notin(2\mathbb Z)^3.}          \tag{26}
\]

For arbitrary nonzero separation, use the primitive ray (3). For the formed
body, use the independently derived current label (4).

The FCC Moore edge self-probe remains `-1` for both chiralities even though
its full character is directed: for `a=(1,1,0)`, a basis translation sees
`+/-i`. Thus “self-probe loses chirality” and “the complete character loses
chirality” are different claims. FTD-0935 established the former; (2)
classifies the latter.

## 8. Epistemic accounting

Theorem-grade:

- the exact parity kernel (2);
- the all-even FTD-0925 raw-dipole counterexample;
- primitive normalization, covariance, reversal, and ray uniqueness;
- universal direction sensitivity of the primitive character;
- exact reconstruction of the 19-site current's integrated four-arm orbit;
- the primitive body-label and character covariance laws;
- recurrence under the prepared FTD-0926 map; and
- zero phase-blind full-cycle vector export.

Still open:

- a production-native force producing the remainder/velocity orbit;
- autonomous scaffold formation and perturbation-robust label protection;
- phase-gate origin, work, and recovery;
- common source-field action and equal-and-opposite vector recoil;
- winding/carry ownership and physical momentum scale `p_*`;
- net displacement, mobility, collision composition, and recovery;
- critical-quartic `G*` finite-tick cadence and CM/substrate operator bridge;
- Born recovery, Bell-laboratory recovery, Lorentz hiding, and completeness.

No selected type, import currency, fitted constant, engine law, or production
path is added.

## 9. Certificate provenance

The frozen preregistration is
[`PREREG_C4_CHARACTER_PARITY_KERNEL_PRIMITIVE_DIRECTION_AND_COMPACT_BODY_ORBIT_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_C4_CHARACTER_PARITY_KERNEL_PRIMITIVE_DIRECTION_AND_COMPACT_BODY_ORBIT_v1.md),
SHA-256
`AB3B368AFC8B04BFCC8319D3A5A4139F193D2D1C61FB8B55C22D326A70A7F4CC`.

The exact proof of record is
`scripts/proofs/proof_c4_character_parity_kernel_primitive_direction_compact_body_orbit.py`,
SHA-256
`6FBBC402CCE5B26C3D79F7F57B1B78752420C9072EFA8FF5B58FEAF92066B3B2`.
Its first immutable execution passed `132/132` checks and returned Outcome A.
No repair protocol was required.

## 10. Next acceptance gate

Pre-register the minimum common-action classifier for one stroboscopic body
character. It must:

1. use `u_n` from the live current, not a target direction;
2. open only at a declared local clock phase;
3. couple source and FTD-0933 dressing cocycle through one local action;
4. derive equal-and-opposite source/field impulse and exact energy exchange;
5. identify the local owner of any reciprocal-lattice carry;
6. keep physical normalization symbolic unless fixed independently; and
7. remain blind to `G*`, Born targets, measurement settings, and outcomes.

If every phase-symmetric local action retains (24), a theorem-grade
phase-gate/rectification necessity result is the correct next boundary.

```text
RAW_C4_CHARACTER_EXISTS_FOR_EVERY_INTEGER_LABEL=TRUE
RAW_C4_CHARACTER_DIRECTIONAL_IFF=SOME_COMPONENT_ODD
RAW_CHARACTER_PARITY_KERNEL=(2Z)^3_MOD_4
FTD0935_UNIVERSAL_RAW_DIRECTION_READING=CORRECTED
CANONICAL_DIRECTION_REPAIR=chi*primitive(a)
PRIMITIVE_REPAIR_CUBIC_COVARIANCE=EXACT
PRIMITIVE_REPAIR_DIRECTIONAL_FOR_NONZERO_A=TRUE
BODY_INTEGRATED_CURRENT_ORBIT=EXACT_C4
BODY_PRIMITIVE_CHARACTER_ORBIT=EXACT_C4
BODY_CHARACTER_SOURCE=EXISTING_TERNARY_STATE_AND_VELOCITY
PREPARED_REFERENCE_RECURRENCE=EXACT
PROTECTED_PRODUCTION_MEMORY=NOT_DERIVED
PHASE_BLIND_LINEAR_VECTOR_EXPORT=ZERO
PHASE_GATE_OR_ADDITIONAL_DIRECTIONAL_STATE=REQUIRED
UNWRAPPED_PHYSICAL_MOMENTUM=OPEN
RECIPROCAL_CARRY_OWNERSHIP=OPEN
DYNAMIC_COMMON_ACTION_VECTOR_RECOIL=OPEN
GSTAR_INTEGER_TICK_CADENCE=OPEN
PRODUCTION_CHANGED=FALSE
NO_NEW_SELECTED_TYPE=TRUE
BORN_BELL_CONTEXT_USED=FALSE
```
