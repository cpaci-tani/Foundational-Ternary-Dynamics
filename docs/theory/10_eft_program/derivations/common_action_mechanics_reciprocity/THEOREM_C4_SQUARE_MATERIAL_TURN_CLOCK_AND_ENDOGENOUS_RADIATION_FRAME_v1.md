# C4 square-material turn clock and endogenous radiation frame v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT SPATIALLY RECURRENT NEUTRAL PROTO-MATTER CLOCK]** +
**[THEOREM — DISCRETE CHARGE CONTINUITY, CURRENT, AND POSITIVE MEAN STRESS]** +
**[THEOREM — ORDERED MATERIAL TURN SUPPLIES THE RADIATION-PLANE QUOTIENT]** +
**[BOUNDARY — PREPARED LOOP, NOT FORMED OR ENERGY-BOUND MATTER]** +
**[OPEN — COMMON FEEDBACK, RADIATION WORK/RECOIL, LORENTZ FORCE, GRAVITY/LENSING, BORN, ALPHA]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_c4_square_material_turn_clock_radiation_frame.py](../../../../../scripts/proofs/proof_c4_square_material_turn_clock_radiation_frame.py)
performs **88,081 exact checks** over all 24 ordered perpendicular SC planes,
four material phases, two charge orientations, the complete signed cubic group,
and every capacity-permission word through eight global ticks. No measured
coefficient or target value enters.

---

## 1. From a bond recurrence to a spatial recurrence

The earlier material-clock theorem recurrently transfers one A9 token on a
single bond. It proves a local clock but has no spatial route history. The
radiation-release theorem independently proves that a directed source \(d\)
cannot choose a transverse plaquette; it needs an ordered perpendicular
direction \(v\).

The smallest common repair is an ordered SC plane

\[
 f=(d,v),\qquad d\cdot v=0.                         \tag{1}
\]

It has four directed boundary edges

\[
 e_0=(0,d),\quad e_1=(d,v),\quad
 e_2=(d+v,-d),\quad e_3=(v,-v).                    \tag{2}
\]

There are \(6\times4=24\) such ordered frames. They form a closed
\(O_h\)-covariant family.

---

## 2. Persistent neutral manifestation

At material phase \(p\in\mathbb Z_4\), place a neutral ternary dipole on edge
\(e_p=(x_p,u_p)\):

\[
 \rho_p(x_p)=-\epsilon,\qquad
 \rho_p(x_p+u_p)=+\epsilon,
 \qquad \epsilon=\pm1.                              \tag{3}
\]

The material step is simply

\[
 \boxed{F_{\square}(f,p,\epsilon)
       =(f,p+1\!\!\pmod4,\epsilon).}                \tag{4}
\]

It is a permutation, its explicit inverse decrements \(p\), and

\[
 F_{\square}^4=1                                   \tag{5}
\]

with no smaller positive return on the phase-labelled state. Manifestation
never disappears during the orbit: one negative and one positive endpoint
remain actual on every admitted tick.

This is a prepared **proto-matter** recurrence. Equation (4) does not derive
formation, binding, or a ground-state energy.

---

## 3. Exact continuity current

During \(e_p\to e_{p+1}\), move the negative endpoint along the old edge and
the positive endpoint along the next edge:

\[
 j_p=-\epsilon\,e_p+\epsilon\,e_{p+1}.              \tag{6}
\]

The two edges share the turning vertex. Direct incidence gives

\[
 \boxed{\rho_{p+1}-\rho_p=\partial j_p}             \tag{7}
\]

at all three affected sites. Thus net charge remains zero and the complete
four-step current telescopes:

\[
 \sum_{p=0}^{3}j_p=0.                               \tag{8}
\]

Charge conjugation sends

\[
 \epsilon\mapsto-\epsilon,\qquad
 \rho\mapsto-\rho,\qquad j\mapsto-j,                \tag{9}
\]

while leaving the route and its stress unchanged. Equations (3)--(9) commute
with every signed cubic transformation.

---

## 4. One orbit is both matter and a clock

Let the instantaneous route stress be the unit dyad

\[
 T_p=u_pu_p^{\mathsf T}.                            \tag{10}
\]

Although the cycle current vanishes, the cycle-mean stress is positive:

\[
 \boxed{
 \frac14\sum_{p=0}^{3}T_p
 =\frac12\left(dd^{\mathsf T}+vv^{\mathsf T}\right),
 \qquad \operatorname{tr}\langle T\rangle=1.}       \tag{11}
\]

The orbit therefore separates a phase-cancelling transport current from a
persistent charge-even stress source. That is precisely the parity split
required by the common electromagnetic/tensor source theorem.

If a retained capacity permission \(g_t\in\{0,1\}\) admits or stalls the whole
step, then for every finite history

\[
 X_N=F_{\square}^{\,\tau_N}X_0,\qquad
 \tau_N=\sum_{t<N}g_t.                              \tag{12}
\]

The certificate exhausts all permission words through length eight. The global
substrate tick always advances; the material clock counts admitted spatial
turns.

---

## 5. The turn history supplies the radiation frame

At phase \(p\), the ordered pair

\[
 f_p=(u_p,u_{p+1})                                  \tag{13}
\]

is perpendicular and transforms equivariantly under \(O_h\). It is exactly
the four-way plane quotient required by the cotangent radiation theorem:

\[
 v_{\rm rad}=h n,\qquad
 (n,h)\sim(-n,-h).                                  \tag{14}
\]

Therefore every material corner supplies a radiation frame without an
external normal selector, handedness bit, or target-dependent router. The
same carried polarity \(\epsilon\) sets the sign of the charge-odd released
field, while equation (11) remains charge even.

Composing the material turn with the proven radiation toggle gives the local
type-correct chain

\[
 \boxed{
 \text{persistent material turn}
 \longrightarrow
 \text{ordered transverse frame}
 \longrightarrow
 \text{number-neutral closed field seed}.}         \tag{15}
\]

This is a structural reason for radiation to depend on transaction history:
one instantaneous velocity direction cannot frame a transverse release, but
a change of route can.

---

## 6. Progress toward one action

Equations (3)--(15) now put five requested ingredients on one finite recurrent
state:

1. nonzero ternary manifestation;
2. persistent localized proto-matter;
3. a material C4 clock with capacity-controlled local rate;
4. exact charge current and positive charge-even stress; and
5. an endogenous plane for the transverse electromagnetic release vertex.

The construction still does not emit physical energy. The inactive and active
radiation seeds have equal token number, so the token ledger cannot distinguish
them. Nor has the square material route been composed with the A9 stress
feedback map on one exhaustive product state.

The next exact gate is to add a nondegenerate field-action coordinate and make
one local corner event exchange it with material momentum while preserving:

\[
 \Delta H_{\rm matter}+\Delta H_{\rm field}=0,
 \qquad
 \Delta P_{\rm matter}+\Delta P_{\rm field}=0.      \tag{16}
\]

Only that reciprocal exchange can promote the present release vertex to a
force law. A lattice Lorentz form must then emerge from the exchange and its
symmetries; it may not be inserted as an independent continuum force.

Stable formation, finite-amplitude Maxwell propagation, a charged static pole,
tensor constraints, spin-2 dynamics, lensing, the physical Born pushforward,
and native alpha remain open.

The
[reciprocal-work successor](THEOREM_C4_SQUARE_MATTER_STRESS_RADIATION_RECIPROCAL_WORK_EXCHANGE_v1.md)
closes the local field-energy half of equation (16): the square clock, A9
stress-capacity owner, and transverse seed form an exact matched permutation
with \(\Delta h_F=-\Delta h_C\). Its one-plaquette/one-capacity normalization is
conditional, and its locally reabsorbed electric seed has no directional
Poynting momentum, so the momentum and force half of (16) remains open.
