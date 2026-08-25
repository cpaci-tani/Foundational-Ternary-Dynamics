# Oriented bond--plaquette Hodge-Maxwell target and finite-lift boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT AXIAL-SIGN OBSTRUCTION FOR UNORIENTED PAIRS]** +
**[THEOREM — ONE-BIT ORIENTED-PLAQUETTE CARRIER]** +
**[THEOREM — EXACT DISCRETE-INCIDENCE MAXWELL GENERATOR AND TWO MODES]** +
**[SELECTION CANDIDATE — PARITY-TWISTED C4 EDGE--FACE TRANSACTION]** +
**[OPEN — FINITE LOCAL PERMUTATION LIFT, WORK, SOURCE CLOSURE, MATTER,
LENSING, BORN INTEGRATION, ALPHA]**  
**Production status:** unchanged  
**Ledger status:** no row minted; no new production type adopted

**Exact certificate:**
[proof_oriented_bond_plaquette_hodge_maxwell_target.py](../../../../../scripts/proofs/proof_oriented_bond_plaquette_hodge_maxwell_target.py)
performs 1,280 exact checks. It enumerates signed cubic stabilizers, the
complete oriented perpendicular-SC-pair carrier, all 48 cubic transformations,
and the symbolic centered-incidence curl generator. No continuum target
constant, fitted coefficient, or numerical eigensolver is used.

---

## 1. What the Bloch failure requires

The
[Gaussian-current Bloch boundary](THEOREM_C18_FCC_GAUSSIAN_CURRENT_BLOCH_DIFFUSION_BOUNDARY_v1.md)
proved that a protected polar current plus phase-independent streaming has

\[
 LK_aR=0                                             \tag{1}
\]

and therefore only $O(k^2)$ chiral diffusion. A Maxwell repair must add either
a nonzero first-order spatial intertwiner or a nonsemisimple cotangent carrier.

The minimum finite-state first-order candidate is not another onsite phase
label. It is the incidence relation between:

- a **polar bond/edge record**, and
- an **axial oriented plaquette/face record**.

This theorem prices that relation exactly.

---

## 2. Why an unordered pair cannot carry magnetic orientation

Take two perpendicular unoriented SC bond lines $[d]$ and $[e]$. Their plane
determines the unsigned normal line $[d\times e]$, but not its sign.

For every one of the three coordinate planes, the exact $O_h$ stabilizer of
the unordered line pair contains an improper cubic transformation $R$ such
that

\[
 R\{[d],[e]\}=\{[d],[e]\},                         \tag{2}
\]

while the axial transformation law gives

\[
 \det(R)R(d\times e)=-(d\times e).                 \tag{3}
\]

An equivariant signed normal would have to be both fixed by the stabilizer and
negated by equation (3), hence must vanish. Therefore:

\[
 \boxed{
 \text{an unordered perpendicular bond pair has no nonzero canonical axial
 normal under }O_h.}                                \tag{4}
\]

This is the same structural issue exposed by the earlier axial payload-routing
boundary: a plane without ordered presentation does not own a circulation
sign.

---

## 3. One orientation bit is sufficient

Adjoin one sign

\[
 \epsilon\in\{-1,+1\}                              \tag{5}
\]

to pair presentation and impose the equivalence

\[
 (d,e,\epsilon)\sim(e,d,-\epsilon).                \tag{6}
\]

Then

\[
 b[d,e,\epsilon]=\epsilon(d\times e)               \tag{7}
\]

is independent of presentation. Under every signed cubic transformation,

\[
 b[Rd,Re,\epsilon]=\det(R)R\,b[d,e,\epsilon],       \tag{8}
\]

so $b$ is an axial vector exactly.

The complete finite census contains 24 oriented plane states. Equation (7)
maps them onto the six directed axial SC normals with multiplicity four each.
One bit is therefore sufficient to repair the sign obstruction; no continuous
orientation variable is required.

FTD already carries a sign $\epsilon$ in the controlled-actualization token.
That makes shared ownership possible, but does **not** prove that the existing
orientation is dynamically transferred into a plaquette circulation. Such a
transfer is part of the open finite-lift gate.

---

## 4. Edge--face field types

Let $E$ be a polar edge 1-cochain and $B$ an axial face 2-cochain, represented
as three components after the cubic Hodge identification. This parity split is
the lattice-native counterpart of electric and magnetic field types:

\[
 E\mapsto RE,
 \qquad B\mapsto\det(R)RB.                          \tag{9}
\]

The proposed C4 reinterpretation is therefore parity-twisted:

\[
 \text{bond}\longrightarrow\text{oriented face}
 \longrightarrow-\text{bond}
 \longrightarrow-\text{oriented face}.             \tag{10}
\]

Equation (10) is a **[SELECTION CANDIDATE]** for how the finite C4 clock could
act geometrically. It is not the onsite scalar phase cycle used by the closed
streaming route. A physical quarter-turn would include the edge--face
incidence/Hodge transaction, not merely relabel $p$.

---

## 5. Exact centered-incidence curl

For the cubic cell complex, the centered edge-difference symbol is

\[
 q_a(k)=2\sin{k_a\over2}.                            \tag{11}
\]

Define the cross-product matrix

\[
 [q]_\times=
 \begin{pmatrix}
 0&-q_z&q_y\\
 q_z&0&-q_x\\
 -q_y&q_x&0
 \end{pmatrix},                                    \tag{12}
\]

and the Fourier curl

\[
 C(q)=i[q]_\times.                                  \tag{13}
\]

The six-component edge--face generator is

\[
 \boxed{
 \mathcal G(q)=
 \begin{pmatrix}
 0&C(q)\\
 -C(q)&0
 \end{pmatrix}.}                                   \tag{14}
\]

Equation (14) is the exact symbol of a nearest-cell incidence operator. It is
not inserted from a continuum derivative and contains no alpha or physical
speed target.

The certificate proves

\[
 C^\dagger=C,
 \qquad \mathcal G^\dagger=-\mathcal G,             \tag{15}
\]

and the boundary-of-boundary identities

\[
 q^TC=0,
 \qquad Cq=0.                                       \tag{16}
\]

Thus the quadratic norm $E^\dagger E+B^\dagger B$ is conserved by the
continuous generator, and both divergence constraints are invariant.
It also verifies equation (14) under all 48 signed cubic transformations with
$E$ polar and $B$ axial, including every improper reflection.

---

## 6. Exact linear cone and mode count

The characteristic polynomial is

\[
 \boxed{
 \chi_{\mathcal G}(\lambda)
 =\lambda^2(\lambda^2+|q|^2)^2.}                   \tag{17}
\]

For $q\ne0$, two longitudinal zero modes are fixed by

\[
 q\cdot E=\rho,
 \qquad q\cdot B=0.                                \tag{18}
\]

On the four-dimensional transverse phase space,

\[
 \mathcal G^2=-|q|^2I,                             \tag{19}
\]

so the two polarization pairs have

\[
 \omega_\pm(q)=\pm|q|.                             \tag{20}
\]

Because $q(k)=k+O(k^3)$,

\[
 \omega_\pm(k)=\pm|k|+O(|k|^3)                    \tag{21}
\]

in lattice units. Equations (17)--(21) supply exactly the dynamical structure
missing from the protected-current collision: two constraints, two
polarizations, and a first-order cone.

This is an exact **target generator**, not yet the finite native tick. The
coefficient one in equation (14) is the unit incidence normalization; a
physical $c_{\rm eff}$ exists only after an exact finite-time transaction
realizes the generator and fixes the relation between cell spacing and the
global tick.

---

## 7. Why this is not yet the one action

Equation (14) acts on linear cochain amplitudes. The strict-discrete ontology
still requires a payload-complete local permutation or equivalent finite
transaction on C4-plus-blank records whose blocked tangent generator is
equation (14).

That lift must solve all of the following simultaneously:

1. route a bond token into oriented face circulation without copying it;
2. retain enough owned state for the exact inverse;
3. enforce occupancy/capacity backpressure;
4. preserve a positive finite work ledger;
5. derive, rather than impose, the two divergence constraints;
6. accept the manifestation current quantum and return an equal/opposite
   reserve debit; and
7. avoid extra gapless scalar, polar, or axial species.

A real-linear incidence equation written above a finite alphabet does not by
itself satisfy those requirements. Claiming Maxwell at this stage would
confuse a proved representation target with a microscopic derivation.

---

## 8. Relation to the other requested sectors

### Manifestation and matter clocks

The same orientation-bit type already occurs in the actualization payload,
and the same C4 cycle occurs in the recurrent proto-matter clock. A successful
lift can therefore make one token's phase advance simultaneously mean:

\[
 \text{local clock phase}
 \quad\text{and}\quad
 \text{edge--face field transaction}.              \tag{22}
\]

This is a structural opportunity, not yet a theorem of shared dynamics.

### Gravity and lensing

A capacity-weighted Hodge star would make the same local capacity tensor alter
both body transactions and wave incidence. That is the cleanest present route
to a shared matter/light geometry and hence lensing. No capacity-weighted
finite lift, tensor constraint algebra, or deflection observable is yet
derived.

### Contextual Born actualization

The physical Born tape already uses phase compatibility and an oriented
actualization token. A common edge--face transaction could transport those
records into detector ports. Native apparatus preparation, trial competition,
and the general-amplitude pushforward remain open.

### Fine-structure readout

Equation (14) contains no charge response and no stable source. It supplies a
candidate vacuum cone but not $g_{\rm eff}$, $\hbar_{\rm eff}$, or an
operational alpha. No comparison with $x_+$ is admissible.

---

## 9. Next locked gate

Construct the smallest local finite lift on one elementary cube using only:

- C4-plus-blank bond tokens;
- the existing orientation sign;
- oriented plaquette ownership as equation (6), or an exactly equivalent
  distributed record;
- ternary endpoint state; and
- capacity/backpressure.

The lift must be a permutation of its complete finite state space, commute
with $O_h$, expose an exact inverse, and reduce at the uniform reference to a
nonzero first-order edge--face block proportional to equation (14). The
coefficient and any substep ordering must be frozen from locality and
reversibility before inspecting a physical cone or alpha.

Only that pass turns the Hodge-Maxwell target into part of a native action.

The first naive lift is already closed negative by the
[oriented plaquette flat-band theorem](THEOREM_ORIENTED_PLAQUETTE_C4_FLAT_BAND_AND_SHARED_EDGE_PRICE_v1.md).
A token that merely cycles around one fixed face has $P(k)^4=I$ and
$k$-independent quasiphases because its net displacement is zero. The minimum
remaining lift must exchange plaquette ownership reversibly at a shared edge;
local circulation alone is a clock, not propagation.
