# Theorem — Native ternary plaquette quarter-turn recursion v1

**Identifier:** `FTD-0914`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — MINIMUM CARDINAL SPATIAL REALIZATION OF J^2=-I]` +
`[THEOREM — EXACT ORIENTED BIVECTOR RECURSION]` +
`[THEOREM — EQUAL RADIAL/TANGENTIAL ENERGY CHANNELS]` +
`[BOUNDARY — INSTANTANEOUS STATE DOES NOT RETAIN DIRECTION]` +
`[CLOSED NEGATIVE — TOPOLOGICAL PROTECTION IN CURRENT NONCOMPACT LIFT]` +
`[OPEN — PRODUCTION ORBIT FORMATION, INVARIANT REGION, WORK, AND COUPLING]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]`

## 1. Result

The smallest closed cardinal loop of the cubic substrate is a four-site
plaquette. On it, the neutral ternary pattern

\[
(+1,0,-1,0)
\]

and one directed cardinal shift realize the real complex structure exactly:

\[
R^2=-I,\qquad R^4=I.
\]

The transition bivector

\[
L_n=d_n\times d_{n+1}
\]

retains clockwise versus counterclockwise information and reconstructs the
next state through

\[
\boxed{d_{n+1}=\frac{L_n\times d_n}{|d_n|^2}}.
\]

This supplies the simplest exact spatial recursion suggested by the
left/right, `i`, and symmetric-square discussion. It does **not** yet supply a
protected production memory: forward and reverse motion visit the same four
instantaneous words, and the ordinary real field lift contracts through
`L=0` at finite, monotonically decreasing energy.

## 2. Minimum cubic cycle and the emergence of the quarter-turn

Choose the square

\[
r_0=(1,0,0),\quad r_1=(0,1,0),\quad
r_2=(-1,0,0),\quad r_3=(0,-1,0).
\]

Let `S` move every site value one step around the directed square. Define

\[
a=(1,0,-1,0),\qquad b=(0,1,0,-1).
\]

Then

\[
Sa=b,\qquad Sb=-a,
\]

so in the ordered basis `(a,b)`,

\[
S\big|_{\operatorname{span}(a,b)}
=R=\begin{pmatrix}0&-1\\1&0\end{pmatrix}.
\]

Consequently `R^2=-I` and `R^4=I`. Reverse transport gives

\[
R^{-1}=-R.
\]

This is a spatial realification of multiplication by `i`; no complex-valued
voxel field was inserted. The cubic cardinal graph is bipartite, so it has no
odd cycle. A two-site neutral exchange has order two. The displayed square is
therefore the minimum cardinal cycle capable of an order-four orientation.

Each word is ternary, neutral, and contains exactly one positive and one
negative site. The construction uses the existing actual alphabet and site
positions only.

## 3. Dipole orbit and retained orientation

For `s^(n)=S^n a`, define the neutral dipole

\[
d_n=\sum_{j=0}^3s_j^{(n)}r_j.
\]

The exact orbit is

\[
(2,0,0)\to(0,2,0)\to(-2,0,0)
\to(0,-2,0)\to(2,0,0).
\]

Successive dipoles are orthogonal and have equal squared norm `4`. Forward
transport has

\[
L_n^+=d_n\times d_{n+1}=(0,0,4)
\]

at every step, while reverse transport has

\[
L_n^-=(0,0,-4)=-L_n^+.
\]

Exchanging the temporal order of one transition reverses `L`. Thus `L` is an
axial, time-odd orientation carrier. The vector triple-product identity gives

\[
(d_n\times d_{n+1})\times d_n
=|d_n|^2d_{n+1}
\]

because `d_n` and `d_(n+1)` are orthogonal. This proves the coordinate-free
recursion formula in section 1.

The result answers the earlier question “can the substrate distinguish
clockwise from counterclockwise?” at the representation level: yes, an
ordered plaquette transition retains the sign in an axial bivector. An
instantaneous plaquette word does not.

## 4. Full signed-cubic covariance

For any signed cubic matrix `Q`, including reflections,

\[
d\mapsto Qd,
\qquad
L=d\times d'\mapsto\det(Q)QL.
\]

The reconstructed tangent transforms as

\[
\frac{(\det Q\,QL)\times(Qd)}{|Qd|^2}
=Q\frac{L\times d}{|d|^2}.
\]

Thus `L` is correctly axial and the successor is correctly polar. The exact
certificate checks all 48 signed cubic transformations. No global preferred
direction is introduced by choosing the displayed `xy` representative; every
other cardinal plaquette belongs to its cubic orbit.

## 5. Equal-channel energy recursion

Let

\[
t_n=\frac{L_n\times d_n}{|d_n|^2}=d_{n+1}.
\]

Then

\[
d_n\cdot t_n=0,
\qquad |d_n|^2=|t_n|^2.
\]

The two quadratic channel energies are exactly equal:

\[
E_r=\frac12|d_n|^2
=\frac12|t_n|^2=E_t.
\]

The shift swaps radial and tangential roles every quarter-turn without
changing their common magnitude. This is an exact bilateral/equal-channel
energy recursion. It is not, by terminology alone, a derivation of Lorentzian
Hodge self-duality, electromagnetism, or a physical energy scale. A physical
energy interpretation still needs the action and normalization that make the
dimensionless quadratic label energetic.

## 6. What the symmetric square loses

On the modal plane,

\[
R^{-1}=-R.
\]

Every quadratic observable is blind to the central sign, so

\[
\operatorname{Sym}^2(R)
=\operatorname{Sym}^2(R^{-1}).
\]

By contrast,

\[
d_n\times Rd_n=+L,
\qquad
d_n\times R^{-1}d_n=-L.
\]

The ordered bivector is therefore precisely the datum lost by the BCC-like
symmetric square. This does not make the plaquette a native CM motive or
derive `G*`; it identifies the minimum spatial carrier of the missing
orientation sign.

## 7. State is not direction

Forward and reverse transport visit the same set

\[
\{a,b,-a,-b\}.
\]

At each member of that set, the forward and reverse successors differ. Hence
no function of one instantaneous word can distinguish the two directions. A
faithful local carrier must retain at least one of:

1. an ordered pair `(d_n,d_(n+1))`;
2. the bivector `L_n`;
3. an equivalent momentum/wave-velocity coordinate; or
4. an explicit branch variable.

This is the finite spatial analogue of the FTD-0839 result that the square
field loses the unsquared orientation lift. It also explains why a matter
snapshot alone is insufficient: recursion is a relation between successive
states.

## 8. Exact protection boundary

The ordinary real lift has the continuous homotopy

\[
d_n(t)=t d_n,
\qquad d_{n+1}(t)=t d_{n+1},
\qquad 0\le t\le1.
\]

Along it,

\[
L_n(t)=t^2L_n,
\qquad E_r(t)=t^2E_r.
\]

The path reaches the zero field without an energy floor. This is the local
witness of the broader FTD-0583/0584 theorem: current noncompact real field
fibres contain no localized protected topological sector. Ternary discreteness
does not repair the dynamics automatically, because production genesis,
evaporation, movement, and transmutation can leave the four-word orbit.

Therefore the exact result is:

- **recurrence:** established conditional on the directed shift;
- **orientation representability:** established by `L`;
- **topological protection in current fields:** closed negative;
- **production orbit invariance:** open.

Protection would require a dynamically derived invariant region such as

\[
|L|\ge L_{\min}>0,
\]

with a work/defect barrier, or a separately selected compact/singular branch
with its admissibility law. Merely naming the square a loop does not provide
that barrier.

## 9. Relation to the failed two-endpoint carrier

FTD-0913 found that the two-endpoint phase wedge was neither pair-specific
against all fixed endpoint derangements nor governed by the imposed exact
central quartic law in production. The plaquette construction changes the
mathematics rather than tuning that failed law:

- adjacency closes into a physical four-cycle;
- the orientation carrier is the transition bivector of that cycle;
- the successor is reconstructed from the same bivector and current dipole;
  and
- the failure surface is explicit: `L=0` or departure from the ternary orbit.

This does not overturn FTD-0913. It supplies the next structurally distinct
candidate requested by its Outcome-D boundary.

## 10. `G*`, `gamma`, and the next gearbox

The theorem derives a normalized direction, not a physical coupling
magnitude. If a later common-mode action uses the polar tangent

\[
t_n=\frac{L_n\times d_n}{|d_n|^2},
\]

then a coefficient such as `gamma` still sets the dimensional impulse or
curvature scale. The algebra `R^2=-I` fixes orientation and normalization on
the unit orbit; it cannot fix that independent physical coefficient.

Similarly, four shift steps close the reference orbit only conditional on the
shift being the physical update. This is not the quartic clock period and
does not derive the `G*` traversal factor. The honest possible architecture is
now:

\[
\text{quartic radial clock}
\quad\text{coupled to}\quad
\text{oriented plaquette recursion},
\]

with `G*` governing eligibility cadence and the plaquette tangent governing
direction. Deriving their coupling, normalization, finite-tick synchronization,
and work ledger remains open.

## 11. Verification and implementation

The locked protocol SHA-256 is
`659AFA6FE64905C848335052C91F4376A78F6B48E5C416028A8189A2C40951A8`.
The independent exact certificate
`scripts/proofs/proof_native_ternary_plaquette_quarter_turn.py` has SHA-256
`9F707930387AE7A3632694E8706074ED6834FFFBA27663C60CD0962B22297FD3`
and passed `48/48`.

The isolated reference implementation is:

- `engine/include/ftd/eft/native_ternary_plaquette_quarter_turn.h`, SHA-256
  `3A970B82EF0BDCCC457D5DDA049CAF971C2318429970E696E64DB84CEB7D1D09`;
- `engine/src/eft/native_ternary_plaquette_quarter_turn.cpp`, SHA-256
  `E7891C5099D2DCA1F20DF72E6B37F29A60FE63A7A9E7E645D8AC6E2DF73E1F4C`;
- `engine/tests/test_native_ternary_plaquette_quarter_turn.cpp`, SHA-256
  `3E5AE8D8518513150F24EE8FD6FE9104F9605C85DCD0FC865FEB68B0ABF7840D`.

The pinned MSVC 14.44 build passes. Focused Release CTest passes `1/1`. No
`Voxel`, `RenderBridge`, production phase, toggle, default, or result corpus
was changed.

## 12. Next acceptance gate

Before changing production dynamics, pre-register an observation-only census
over all cardinal plaquettes. It must measure:

1. spontaneous formation of the four neutral words and exact forward/reverse
   one-step transitions;
2. run length on the same transported or fixed support;
3. the transition bivector and the `L=0`/orbit-departure defect rate;
4. signed-cubic, reflection, time-reversed, empty, randomized, and noncyclic
   controls;
5. field/wave-velocity alignment with the ternary tangent; and
6. energy, dissipation, and production-event changes without reading `G*`,
   `gamma`, context, outcome, or Born targets.

Only a positive, held-out, support-specific recurrence result may license a
separate perturbation/barrier campaign. No coefficient tuning is licensed by
this theorem.

```text
MINIMUM_CARDINAL_SPATIAL_COMPLEX_STRUCTURE=FOUR_SITE_PLAQUETTE
TERNARY_NEUTRAL_QUARTER_TURN_ORBIT=EXACT
CLOCKWISE_COUNTERCLOCKWISE_TRANSITION_BIVECTOR=EXACT
COORDINATE_FREE_SUCCESSOR=EXACT
SIGNED_CUBIC_COVARIANCE=EXACT
EQUAL_RADIAL_TANGENTIAL_ENERGY_CHANNELS=EXACT_LABEL_GEOMETRY
SYMMETRIC_SQUARE_RETAINS_DIRECTION=FALSE
INSTANTANEOUS_WORD_RETAINS_DIRECTION=FALSE
ORDERED_TRANSITION_RETAINS_DIRECTION=TRUE
CURRENT_NONCOMPACT_LIFT_TOPOLOGICALLY_PROTECTED=FALSE
PRODUCTION_PLAQUETTE_ORBIT_INVARIANT=OPEN
GSTAR_USED=FALSE
GAMMA_MAGNITUDE_DERIVED=FALSE
BORN_BELL_TARGET_USED=FALSE
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_TYPE=TRUE
```
