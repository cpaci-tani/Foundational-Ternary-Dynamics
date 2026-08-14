# One-clock C4 cotangent lift and connection underdetermination

**Identifiers:** `FTD-0976/0977`  
**Status:** `[THEOREM, CONDITIONAL — UNIQUE COTANGENT MOMENTUM/ONE KINETIC SQUARE]` +
`[THEOREM, CONDITIONAL — EXACT MECHANICAL REACTION CANCELLATION/PHASE HOLONOMY]` +
`[THEOREM — C4 ENDPOINT UNDERDETERMINES LOCAL PROFILE AND INTEGER LIFT]` +
`[CORRECTION — COMMON A(G-qI) PROFILE IS AN ADDITIONAL SELECTION]` +
`[BOUNDARY — REGULAR LOCAL CONNECTION IS PASSIVE PURE GAUGE]` +
`[OPEN — PHYSICAL BUNDLE GLUING/SWITCHING/FORMATION/PRODUCTION]`  
**Date:** 2026-08-12

## 1. Result

One physical clock with a specified clock-dependent action on two internal
phase fibers has exactly one covariant mechanical momentum and one kinetic
square. On a regular local chart the forced general form is

\[
 \boxed{K=\Pi+r_G{\cal A}_G(\delta)G
                  -q r_I{\cal A}_I(\delta)I}               \tag{1}
\]

and

\[
 \boxed{H={K^2\over2M}+V(\delta)+H_{\rm int}.}             \tag{2}
\]

Here `G` is the signed gearbox generator, `I` is a positive field action,
`r_G,r_I` are integer representation lifts, and
`q in {-1,0,+1}` is held fixed during one gate. The character `q` is not yet
identified with the production ternary state.

This sharpens and corrects the proposed compact law. The existing reference
mechanisms compose as

\[
 r_G=r_I=1,qquad
 {\cal A}_G={\cal A}(\delta),qquad {\cal A}_I=1,           \tag{3}
\]

so their direct one-clock candidate is

\[
 \boxed{K=\Pi+{\cal A}(\delta)G-qI,\qquad
 H={\left[\Pi+{\cal A}(\delta)G-qI\right]^2\over2M}
   +V+H_{\rm int}.}                                        \tag{4}
\]

For `q=+1`, equation (4) is the FTD-0975 merged square with
`X={\cal A}G`.

The more compact expression

\[
 K=\Pi+{\cal A}(\delta)(G-qI)                              \tag{5}
\]

requires the additional choice
`A_G=A_I=A`. It is a valid diagonal specialization, but it is not forced by
the `C4` carrier or by the existing FTD-0963/0974 pair of mechanisms.

## 2. Cotangent derivation

Use a regular fiber chart with clock coordinate `delta`, internal angles
`beta,alpha`, and laboratory fiber coordinates

\[
 \beta_{\rm lab}=\beta-r_GF_G(\delta),\qquad
 \alpha_{\rm lab}=\alpha+q r_IF_I(\delta),                 \tag{6}
\]

where `A_G=F_G'` and `A_I=F_I'`. Equality of canonical one-forms requires

\[
 \begin{aligned}
 K\,d\delta+G\,d\beta_{\rm lab}+I\,d\alpha_{\rm lab}
 &=\left(K-r_GA_GG+q r_IA_II\right)d\delta\\
 &\quad+G\,d\beta+I\,d\alpha\\
 &=\Pi\,d\delta+G\,d\beta+I\,d\alpha.
 \end{aligned}                                             \tag{7}
\]

The coefficient of `d delta` uniquely gives equation (1). The exact
six-dimensional Jacobian of equation (6) together with equation (1) obeys

\[
 J^T\Omega_6J=\Omega_6,qquad \det J=1.                    \tag{8}
\]

Thus this is a full cotangent lift, not a scalar coefficient analogy. If the
uncoupled physical clock has one kinetic term `K^2/(2M)`, substitution of the
unique canonical momentum yields equation (2). Adding a second kinetic square
for the same clock is neither required nor allowed.

## 3. Mechanical equation and reciprocal reaction

Assume `H_int` is independent of the two fiber angles and the representation
sector remains fixed. Then

\[
 \dot G=0,\qquad \dot I=0,
 \qquad \dot\delta={K\over M}.                             \tag{9}
\]

Canonical momentum carries the connection force,

\[
 \dot\Pi=-V'(\delta)
 -{K\over M}\left(r_GA_G'G-q r_IA_I'I\right).              \tag{10}
\]

Differentiating equation (1) and using equation (9) cancels the connection
terms exactly:

\[
 \boxed{\dot K=-V'(\delta).}                               \tag{11}
\]

The load therefore changes canonical bookkeeping without adding a second
bare clock inertia. The internal interaction-picture angles obey

\[
 \dot\beta_{\rm int}=r_GA_G\dot\delta,
 \qquad
 \dot\alpha_{\rm int}=-q r_IA_I\dot\delta,                \tag{12}
\]

and hence

\[
 \Delta\beta_{\rm int}=r_G\int A_G\,d\delta,
 \qquad
 \Delta\alpha_{\rm int}=-q r_I\int A_I\,d\delta.         \tag{13}
\]

The sign in the second equation follows from the coordinate convention in
equation (6); reversing that convention reverses both the character label and
the reported phase sign without changing the physics.

## 4. What C4 does and does not determine

The endpoint `C4` action fixes an integrated angle only modulo `2 pi`.
It does not fix a local connection.

On `delta in [0,1]`, both

\[
 A_0={\pi\over2},qquad
 A_\epsilon={\pi\over2}+\epsilon(2\delta-1)                \tag{14}
\]

have integral `pi/2`, although they are unequal for nonzero `epsilon`.
Likewise the integer weights `1` and `5` give the same endpoint matrix,

\[
 J^5=J,                                                     \tag{15}
\]

while producing different local momentum loads. The finite carrier therefore
determines weights only modulo four. Choosing the minimum faithful integer
lift `+1` or `-1`, a connection profile, and a common versus unequal profile
is selection work.

This explains why equations (4) and (5) must not be conflated. In equation
(4), the FTD-0974 identity suspension uses the clock phase itself,
`F_I(delta)=delta`, while the FTD-0963 gearbox uses its selected gate primitive
`F_G'=A(delta)`. Equal endpoint quadrants do not make the two local profiles
equal.

## 5. The cross term is conditionally mandatory

Write

\[
 X=r_GA_GG,qquad Y=q r_IA_II.                             \tag{16}
\]

Once a merged cotangent action is selected, its kinetic energy is

\[
 {\left(\Pi+X-Y\right)^2\over2M}
 ={\Pi^2+X^2+Y^2+2\Pi X-2\Pi Y-2XY\over2M}.               \tag{17}
\]

Therefore

\[
 \boxed{H_{GI}=-{q r_Gr_I A_GA_I GI\over M}}              \tag{18}
\]

cannot be deleted independently while retaining the selected square. For the
existing-mechanism composition (3), this is

\[
 H_{GI}=-{q{\cal A}(\delta)GI\over M},                     \tag{19}
\]

which is precisely the FTD-0975 `-XI/M` term for `q=1`. For the extra common
profile selection (5), it instead becomes

\[
 H_{GI}=-{q{\cal A}(\delta)^2GI\over M}.                   \tag{20}
\]

Thus the cross term is not arbitrary *after* the bundle action has been
chosen. The underdetermination lies in choosing that bundle action, its
weights, and its profiles.

## 6. Ternary sector and reversal boundary

For a fixed gate, `q=-1,0,+1` yields inverse, inert, and forward field
holonomies. Under the registered signed-generator reversal

\[
 (\Pi,G,I,q)\mapsto(-\Pi,-G,I,-q),                         \tag{21}
\]

equation (1) gives `K -> -K`; the full sector map is anti-symplectic. Holding
a nonzero `q` fixed instead leaves the exact reversal defect

\[
 -2q r_IA_II.                                              \tag{22}
\]

If the character changes from `q` to `q'` at fixed canonical coordinates,

\[
 \Delta K=-(q'-q)r_IA_II.                                 \tag{23}
\]

A dynamically changing ternary label therefore requires a reversible switch
that books its impulse, work, history, and inverse. This theorem does not
identify `q` with `s` or supply that switch.

## 7. Passive connection and global gluing

On a contractible regular gate, both connections are exact one-forms on a
one-dimensional base. Their curvature is zero, and equation (6) removes them
by a canonical change of chart. A single-valued periodic primitive obeys

\[
 \oint dF=0.                                                \tag{24}
\]

Consequently, the cotangent lift alone is passive local geometry. Nonzero
quarter holonomy requires non-single-valued endpoint data, such as the
mapping-torus identification

\[
 (1,z)\sim(0,Jz).                                          \tag{25}
\]

The retained ternary `C4` carrier supplies a candidate for that gluing, but
does not prove that a production field realizes it physically. This is the
same distinction exposed by the FTD-0970 pure-gauge moving-frame result:
canonical reaction bookkeeping is not by itself active token loading.

## 8. Certificate and immutable failure record

- FTD-0976 protocol SHA-256:
  `FD80A0524A8BB437210FC213B0DB071F8FCBB11E03D67594A23BCF4443B084F2`;
- immutable parent proof SHA-256:
  `436E54D2CF9A117CA17F53D054D7C51F670A156A81EA0F4F658F63C85BC6065A`;
- first immutable execution: `49/52`, Outcome D, solely on two Markdown
  line-wrap markers and one factored-versus-expanded structural comparison;
- FTD-0977 repair protocol SHA-256:
  `E08115800DEACDC8D9059D815BF87D408AED927BF33B3E74E368BE8DCDCC296F`;
- repaired wrapper SHA-256:
  `8D7424D69E76AF6D608BC267E853E5F064E62B73441FA21A640C454A18C31CEC`;
- repaired execution: inherited `52/52` plus repair integrity `16/16`,
  Outcome B;
- no parent mutation, numerical search, engine mutation, or production
  mutation.

## 9. Scope firewall

This theorem does not establish:

- the physical identity of `G`, `I`, or the unused sixth production pair;
- `q` as the actual ternary state, charge, or an autonomously changing field;
- either connection profile, integer lift, or mapping-torus gluing as native;
- active field exchange merely from a locally removable cotangent chart;
- formation, switching, maintenance, finite reserve, routing, backpressure,
  positive export, or repeated-cycle stability;
- finite-tick `G*` cadence, the CM-prime/substrate gearbox, Born/Bell recovery,
  preferred-order hiding, or whole-framework completeness; or
- production integration.

The next admissible physical test is no longer another algebraic rearrangement.
It is a source-locked production census for an actual clock-indexed twisted
fiber identification: a retained substrate record must undergo the quadrant
map while laboratory observables, work, reciprocal reaction, switching
history, and the inverse are all measured. Until such a witness exists,
equation (4) is the minimum coherent selected reference law, not an emergent
production law.
