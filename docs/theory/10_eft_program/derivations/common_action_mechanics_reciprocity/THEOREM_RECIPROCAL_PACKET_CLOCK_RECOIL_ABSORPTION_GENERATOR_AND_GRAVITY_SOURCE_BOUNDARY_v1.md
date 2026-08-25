# Reciprocal packet/clock/recoil absorption generator and gravity-source boundary v1

**Date:** 2026-08-24

**Status:** **[THEOREM — ONE TYPE-2 GENERATOR PRODUCES CLOCK-ACTION/RECOIL/SOURCE-REACTION MAP]** +
**[THEOREM — EXACT SYMPLECTICITY, ENERGY, TRANSLATION-CHARGE EXCHANGE, AND INVERSE]** +
**[THEOREM, CONDITIONAL — SCALAR GRAVITY-SOURCE CONTINUITY]** +
**[THEOREM, CONDITIONAL — RECOIL-CORRECTED FIELD/CLOCK COUPLING COMPLIANCE]** +
**[OUTCOME B — EXACT SELECTED COMMON-ACTION VERTEX]** +
**[OPEN — NATIVE TRIGGER, FIELD MOMENTUM, INERTIA, TENSOR STRESS, AND SCALE]**

**Production status:** unchanged

**Ledger status:** no FTD claim row minted

**Locked preregistration:**
[PREREG_RECIPROCAL_PACKET_CLOCK_RECOIL_ABSORPTION_GENERATOR_v1.md](../../preregistrations/common_action_mechanics_reciprocity/PREREG_RECIPROCAL_PACKET_CLOCK_RECOIL_ABSORPTION_GENERATOR_v1.md),
pre-execution SHA-256
`C78C5C887367852AEEAD19F5DDEF7D71F3E85BEBCA240800D427399A842C2156`.

**Exact certificate:**
[proof_reciprocal_packet_clock_recoil_absorption_generator.py](../../../../../scripts/proofs/proof_reciprocal_packet_clock_recoil_absorption_generator.py),
SHA-256
`4B824C3B37A8BADEC9F50ED1785602734B75D6CCF03234D65826E0541CDC2576`,
performs 14,777 exact symbolic and rational checks. It verifies the generator
derivatives, the complete one- and three-dimensional symplectic Jacobians,
energy and translation-charge exchange, seam locality, exact inverse,
quadratic recoil specialization, fail-closed rational fixtures, scalar source
continuity, and the recoil-corrected scale identity. No floating point,
parameter fit, target coupling, or master root enters.

---

## 1. Why ownership transfer alone was insufficient

The
[field-packet reserve-current theorem](THEOREM_C4_FIELD_PACKET_RESERVE_DENSITY_CURRENT_AND_ATOMIC_CLOCK_DEBIT_BOUNDARY_v1.md)
gives a positive, causal, phase-complete energy carrier and an atomic
whole-packet ownership debit. That permutation answers which finite object
owns the reserve, but it does not determine what the receiving clock and body
do when ownership changes.

A physical absorption event must simultaneously:

1. remove field energy from active packet ownership;
2. increase local clock action;
3. deliver field translation charge to material recoil;
4. preserve exact energy and the canonical two-form;
5. retain the off-seam source reaction required by symplecticity; and
6. reverse into the same packet histories.

This theorem supplies one generating function that does all six. The
generator is selected; its consequences are exact.

---

## 2. Declared local variables

Let an admitted batch contain $d\in\mathbb N_{>0}$ complete C4 packets, each
with physical energy coefficient $\Gamma>0$. The active field energy is

\[
 E_F=d\Gamma.                                           \tag{1}
\]

Let $p\in\mathbb R^3$ be the batch's **declared canonical translation
charge**. This theorem does not identify $p$ with the raw $E\times B$ readout;
that identification remains open.

The receiving body owns:

- one regular clock pair $(\theta,I)$ with Hamiltonian $H_C=\omega I$,
  $\omega>0$; and
- one material translation pair $(X,P)$ with differentiable positive kinetic
  energy $K(P)$.

The complete discrete packet identities, C4 phases, route, orientation,
$d$, and $p$ move from active-field ownership to retained absorbed history.
They are not erased.

---

## 3. One type-2 generating function

Freeze

\[
 \boxed{
 F_2(\theta,X;I',P')
 =\theta I'+X\cdot(P'-p)
 -{\theta\over\omega}
 \left[d\Gamma+K(P'-p)-K(P')\right].}                  \tag{2}
\]

The canonical derivative rules give

\[
 I={\partial F_2\over\partial\theta},\qquad
 P={\partial F_2\over\partial X},\qquad
 \theta'={\partial F_2\over\partial I'},\qquad
 X'={\partial F_2\over\partial P'}.                    \tag{3}
\]

Solving equations (3) yields

\[
 \boxed{P'=P+p,\qquad \theta'=\theta,}                  \tag{4}
\]

\[
 \boxed{
 I'=I+{d\Gamma+K(P)-K(P+p)\over\omega},}               \tag{5}
\]

and

\[
 \boxed{
 X'=X-{\theta\over\omega}
 [\nabla K(P)-\nabla K(P+p)].}                           \tag{6}
\]

Equation (6) is not optional bookkeeping. Removing it away from the clock
crossing breaks the canonical two-form. At the registered local absorption
seam

\[
 \theta=0,                                              \tag{7}
\]

it gives $X'=X$: absorption is an impulse/action update with no instantaneous
position hop.

---

## 4. Exact symplecticity and inverse

Let

\[
 z=(\theta,X;I,P),\qquad z'=(\theta',X';I',P').          \tag{8}
\]

The exact symbolic Jacobian $M=\partial z'/\partial z$ obeys

\[
 \boxed{M^{\mathsf T}JM=J}                              \tag{9}
\]

for the full eight-dimensional clock-plus-translation phase space. This also
follows structurally because equation (2) is a regular type-2 generator.

The inverse is explicit:

\[
 P=P'-p,qquad \theta=\theta',                           \tag{10}
\]

\[
 I=I'-{d\Gamma+K(P'-p)-K(P')\over\omega},              \tag{11}
\]

\[
 X=X'+{\theta'\over\omega}
 [\nabla K(P'-p)-\nabla K(P')].                          \tag{12}
\]

Inverse emission also moves the retained packet identities from absorbed
history back into active field ownership. The rational fixture census proves
that missing field ownership, an off-seam trigger, or a negative post-action
fails before mutation.

---

## 5. Exact energy and translation charge

Before absorption, the declared local energy is

\[
 H_{\rm before}=\omega I+K(P)+d\Gamma.                  \tag{13}
\]

After absorption,

\[
 H_{\rm after}=\omega I'+K(P').                         \tag{14}
\]

Substituting equations (4)--(5) proves

\[
 \boxed{H_{\rm after}=H_{\rm before}}                  \tag{15}
\]

identically for every differentiable $K$ appearing in equation (2).

If the incoming field owns translation charge $p$ and the absorbed field owns
zero, equation (4) gives

\[
 \boxed{P+p=P'.}                                        \tag{16}
\]

Thus the same selected generator transfers both field energy and declared
translation charge. It does not determine the physical normalization of $p$.

---

## 6. Quadratic clock/recoil partition

For the signed-cubic quadratic material sector,

\[
 K(P)={|P|^2\over2m},\qquad m>0,                        \tag{17}
\]

equation (5) becomes

\[
 \boxed{
 \omega(I'-I)
 =d\Gamma-{2P\cdot p+|p|^2\over2m}.}                   \tag{18}
\]

For a body initially at rest,

\[
 \boxed{
 d\Gamma
 =\omega\Delta I+{|p|^2\over2m}.}                     \tag{19}
\]

The field packet therefore pays both clock action and material recoil. A
momentum-neutral counterpropagating batch has $p=0$, in which case its entire
energy funds clock action:

\[
 \omega\Delta I=d\Gamma.                                \tag{20}
\]

Equations (18)--(20) correct any absorption ledger that books the whole packet
into the clock while also assigning uncompensated recoil.

---

## 7. Gravity-source consequence and boundary

If the scalar gravity constraint couples to complete local energy ownership,
rather than a sector label, equation (15) gives exact continuity of its local
$T_{00}$ source through absorption:

\[
 T_{00}^{\rm field}+T_{00}^{\rm clock/matter}
 \quad\text{is unchanged by the owner transfer}.         \tag{21}
\]

This is the scalar source property a common action requires: gravity cannot
see energy disappear when a photon-like packet becomes clock/recoil energy.

Equation (21) is conditional. The framework still lacks:

- a derived local stress tensor for the finite packet;
- the tensor handoff from radiation stress to matter stress;
- the scalar/vector constraints required by the STF source;
- nonlinear universal coupling; and
- a production realization of the selected gravity reference action.

Accordingly this theorem advances gravity-source consistency but does not
claim native lensing or a completed spin-2 interaction.

---

## 8. Recoil-corrected coupling compliance

Suppose one rest-frame absorption creates one clock action quantum $I_*$. Then
equation (19) requires

\[
 \omega I_*=d\Gamma-{|p|^2\over2m}.                     \tag{22}
\]

Using the native-alpha measurement definition

\[
 \chi_{\rm EM}={\Gamma\over I_*},                       \tag{23}
\]

gives

\[
 \boxed{
 \chi_{\rm EM}
 ={\omega\over d-|p|^2/(2m\Gamma)}.}                   \tag{24}
\]

For a momentum-neutral batch this reduces to

\[
 \boxed{\chi_{\rm EM}={\omega\over d}.}                \tag{25}
\]

Equations (24)--(25) define exact common-action compliance. They do not fix a
coupling because $\omega,d,p,m,$ and $\Gamma/I_*$ remain unforced. No master
root or experimental value is substituted.

---

## 9. Contextual measurement consequence

The heralded Poincare pushforward already routes one prepared compatible
history to one physical Gauss event at a fixed trial endpoint. If its released
field packet is absorbed by equation (2), the same event can produce a finite
apparatus-clock/action increment and recoil record without irreversible
erasure.

This supplies a reference physical amplification vertex:

\[
 \text{routed field event}
 \longrightarrow
 \text{retained packet history + clock/recoil record}.   \tag{26}
\]

The history bank, herald/counter formation, macroscopic amplification chain,
overlapping traffic, and multipartite no-signalling remain open. Equation
(26) does not promote the prepared Born pushforward to a general measurement
theory.

---

## 10. Contribution to the one-action programme

The common causal chain is now sharper:

\[
 \begin{aligned}
 \text{contextual histories}
 &\longrightarrow \text{manifestation + charge/stress source}\\
 &\longrightarrow \text{Maxwell packet/reserve current}\\
 &\xrightarrow{F_2}
 \text{body clock action + recoil + continuous scalar }T_{00}.
 \end{aligned}                                           \tag{27}
\]

Equation (27) uses one actual generating function at the absorption vertex,
not an energy balance appended after the fact. But the full chain still mixes
finite theorem-grade maps and selected effective canonical structures. It is
therefore a common-action reference construction, not the final native finite
action.

---

## 11. Epistemic disposition

### Established exactly

- one type-2 generator yields the complete clock/recoil/source-reaction map;
- full three-dimensional symplecticity;
- exact total energy;
- exact declared translation-charge exchange;
- seam-local impulse with the required off-seam position reaction;
- retained-history inverse emission;
- quadratic recoil/clock partition;
- conditional scalar gravity-source continuity; and
- recoil-corrected coupling compliance.

### Still selected or open

1. the absorption trigger and clock-crossing aperture;
2. canonical field momentum $p$ and its relation to transported energy;
3. material inertia $m$ and clock frequency $\omega$;
4. the field coefficient $\Gamma$ and action unit $I_*$;
5. tensor stress transfer and complete gravity constraints;
6. nonlinear field scattering and Lorentz force;
7. autonomous contextual-history preparation and no-signalling; and
8. a strictly finite microscopic realization of the real canonical pairs and
   equation (2).

The preregistered result is **Outcome B**: an exact selected common-action
vertex, not a native derivation of its types or scales.

---

## 12. Next locked discriminator

The next action gate is to derive the packet translation charge from the same
finite carrier action and couple the event's charge-even stress moment to the
scalar/STF constraint sector. A pass must make equations (16) and (21) two
Noether/source projections of one finite transaction, while retaining the
Born route and absorption inverse. Only after that can equation (24) become a
blind measurement of a dynamically fixed coefficient rather than a
compliance relation among selected types.

### Subsequent symmetric-stress discriminator (2026-08-24)

The preregistered
[C4 symmetric-stress successor](THEOREM_C4_SYMMETRIC_STRESS_PACKET_MOMENTUM_AND_SOURCE_HANDOFF_BOUNDARY_v1.md)
proves that the finite carrier alone leaves real momentum scale
underdetermined, but the additional symmetric-stress condition
$J_E=c^2p_F$ at $c=1/6$ uniquely gives

\[
 p_F=6Er,
 \qquad
 \Sigma_F=E\,rr^{\mathsf T}=18E\,t_{\rm evt}.
\]

Substitution into this theorem's generator is exact and yields the rest
admission boundary $m\ge18E$. The scalar and STF gravity sources are then
projections of the same stress that fixes recoil. This is Outcome B: stress
symmetry, its finite action, tensor ownership/constraints, and the physical
scale remain open, so the successor does not turn the declared $p$ into an
unconditional native Noether charge.
