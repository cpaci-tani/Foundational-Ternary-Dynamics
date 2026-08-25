# Cotangent framed-plaquette number-neutral radiation release v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — MINIMAL STABILIZER-PACKET NUMBER-NEUTRAL TERNARY EDGE]** +
**[THEOREM — EXACT REVERSIBLE FRAMED PLAQUETTE RELEASE SEED]** +
**[THEOREM — FIRST-ORDER INJECTION INTO THE CONSTRAINED VACUUM MAXWELL SECTOR]** +
**[THEOREM — ORDERED MATERIAL-TURN FRAME QUOTIENT]** +
**[BOUNDARY — ZERO-WORK TOKEN LEDGER]** +
**[OPEN — FINITE-AMPLITUDE COLLISION, RECOIL/LORENTZ FORCE, CHARGED POLE, ALPHA]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_cotangent_framed_plaquette_radiation_release.py](../../../../../scripts/proofs/proof_cotangent_framed_plaquette_radiation_release.py)
exhausts all 24 ordered perpendicular SC frames, four C4 origins, twelve
internal clock stages, two charge orientations, both release states, and the
complete 48-element signed cubic group in **85,395 exact checks**. No measured
coefficient or target spectrum enters the construction.

---

## 1. Why the Gauss packet cannot simply be released

The exact stabilizer-complete source packet on a directed edge \(d\) has

\[
 {\cal P}_d^{(p)}:\qquad (N,E,B)=(8,8d,0).          \tag{1}
\]

It is the correct local Gauss dressing because its boundary equals the two
endpoint charges. Those same facts prevent equation (1) from being a free
radiative excitation: it changes public carrier number and has nonzero
boundary. Releasing it unchanged would excite the scalar--longitudinal
acoustic pair in addition to the transverse vacuum sector.

A radiation record must instead be a signed occupation change about the
vacuum reference, with

\[
 \Delta N=0,\qquad \partial\Delta E=0.              \tag{2}
\]

This separates **bound Gauss dressing** from **released transverse response**
inside the same cotangent carrier.

---

## 2. Minimum fixed-number ternary edge in the packet alphabet

One complete \(D_4\)-stabilizer orbit has eight records and can read only
\(+8d\) or \(-8d\); it has no zero-field state. Therefore a fixed-public-number
edge alphabet with values \(\{-1,0,+1\}\) needs at least two complete packet
orbits. Two phase-distinct bands attain the bound.

For carried charge orientation \(\epsilon=\pm1\), define

\[
 Z_{\epsilon,d}^{(p)}
 ={\cal P}_{\epsilon d}^{(p)}
  \sqcup{\cal P}_{-\epsilon d}^{(p+2)},             \tag{3}
\]

\[
 A_{\epsilon,d}^{(p)}
 ={\cal P}_{\epsilon d}^{(p)}
  \sqcup{\cal P}_{\epsilon d}^{(p+2)}.              \tag{4}
\]

Both contain exactly sixteen distinct public records. The local switch

\[
 Z_{\epsilon,d}^{(p)}\longleftrightarrow
 A_{\epsilon,d}^{(p)}                               \tag{5}
\]

replaces eight occupied channels by eight previously unoccupied channels and
is an involution. Its signed slow-space increment is

\[
 \boxed{(\Delta N,\Delta E,\Delta B)
       =(0,16\epsilon d,0).}                        \tag{6}
\]

Thus sixteen records—not eight—are the exact minimum within the
stabilizer-complete packet alphabet for a number-neutral ternary edge
response. This is a scoped minimum, not a proof against every possible larger
microscopic alphabet.

---

## 3. The four-way spatial-context quotient

A directed SC edge has a \(D_4\) stabilizer acting without a fixed member on
its four perpendicular SC directions. Consequently no deterministic
\(O_h\)-equivariant function of \(d\) alone can choose a plaquette beside that
edge.

The release seed needs only an ordered polar frame

\[
 f=(d,v),\qquad d\cdot v=0,                         \tag{7}
\]

not the complete eight-way \((n,h)\) flag. It is exactly the quotient

\[
 v=hn,\qquad
 (n,h)=(v,+1)\sim(-v,-1).                           \tag{8}
\]

The two cotangent presentations in equation (8) give the same polar
perpendicular direction and the same radiation seed. The 48 flag frames
therefore reduce to 24 ordered polar planes, four choices for each directed
edge.

This quotient has an intrinsic material interpretation. An ordered
right-angle material turn

\[
 d_{\rm in}=d,\qquad d_{\rm out}=v                 \tag{9}
\]

supplies equation (7) equivariantly under every signed cubic transformation.
The minimal release channel is thus acceleration/history framed: one
instantaneous directed source is insufficient, while two consecutive
perpendicular route directions require no independent handedness bit. The
present one-bond proto-matter clock does not yet retain that turn history.

The selected frame defines the elementary plaquette boundary

\[
 (0,d),\quad(d,v),\quad(d+v,-d),\quad(v,-v).        \tag{10}
\]

---

## 4. Reversible transverse release seed

Place one copy of equation (3) or (4) on each of the four edges in equation
(10), using the same \((p,\epsilon)\). The inactive seed contains
\(4\times16=64\) public records and has zero field. The active seed has
normalized edge cochain

\[
 \Delta E_{\square}
 =\epsilon\bigl[d@0+v@d-d@(d+v)-v@v\bigr].         \tag{11}
\]

Because equation (11) is a boundary cycle,

\[
 \boxed{\partial\Delta E_{\square}=0}              \tag{12}
\]

exactly at every vertex. Carrier number is unchanged edge by edge, and no
magnetic source is inserted. The inactive/active swap is reversible, charge
conjugation sends \(\epsilon\mapsto-\epsilon\), and all twelve internal
cotangent/C4 stages commute with the release switch. Transforming both the
ordered plane and all sixty-four records by any signed cubic transformation
gives the same transformed seed.

Equation (11) is therefore the first finite public-field payload in this
strict-discrete chain that a manifestation event can release without changing
Gauss charge or carrier number.

---

## 5. Exact first-order Maxwell-sector membership

Use edge-midpoint phases for the Fourier transform of equation (11). Its
zeroth-order vector sum vanishes. The first nonzero term is

\[
 E_{\square}^{(1)}(k)
 =(k\cdot d)v-(k\cdot v)d,                          \tag{13}
\]

and therefore

\[
 k\cdot E_{\square}^{(1)}(k)=0,\qquad
 \|E_{\square}^{(1)}\|^2
 =(k\cdot d)^2+(k\cdot v)^2.                       \tag{14}
\]

The signed occupation increment also has \(\Delta N=0\) and
\(\Delta B=0\). It lies entirely in the constrained vacuum slow space of the
proved global-C3 cotangent collision. The existing exact Floquet derivative
therefore evolves equation (13) through the two transverse \(E/B\) pairs at

\[
 c_{\rm eff}=\frac16.                               \tag{15}
\]

This is a rigorous **first-order injection theorem**. It is not yet an exact
finite-amplitude wave packet: pair scheduling, nonlinear collisions, and
finite-\(k\) lattice incidence must still be composed on the sixty-four-record
state.

---

## 6. Composition with the common material transaction

The common material/stress/Gauss action already retains the material output
needed to invert its ownership event. Conditional on an ordered material turn
(9), that same event can control equation (5):

\[
 \text{material turn/ownership event}
 \quad\Longrightarrow\quad
 Z_{\epsilon,\square}\leftrightarrow
 A_{\epsilon,\square}.                              \tag{16}
\]

No external random sign is needed; the A9 material token supplies
\(\epsilon\). No Gauss packet is consumed; bound charge dressing and released
radiation are distinct public resources. The inverse material event reverses
the field switch while the seed remains locally present.

Equation (16) is not yet a complete global emission rule. Once the active
records stream away, the local action needs collision/backpressure ownership
that makes the inverse an arriving-field absorption event rather than a
nonlocal recall. A recurrent material loop must also supply its ordered turn
from its own retained route history.

The
[square-material turn successor](../common_action_mechanics_reciprocity/THEOREM_C4_SQUARE_MATERIAL_TURN_CLOCK_AND_ENDOGENOUS_RADIATION_FRAME_v1.md)
closes that last routing requirement on a prepared proto-matter orbit. Its
period-four neutral dipole recurrence supplies one ordered perpendicular turn
at every corner with exact charge continuity and positive mean stress.
Formation/binding and the field-energy exchange remain open.

The later
[reciprocal-work successor](../common_action_mechanics_reciprocity/THEOREM_C4_SQUARE_MATTER_STRESS_RADIATION_RECIPROCAL_WORK_EXCHANGE_v1.md)
places this seed, the square material clock, and one stress-capacity owner in
one exact matched permutation. It derives a complementary local energy ledger
from the seed's canonical norm, but the seed is reabsorbed locally and carries
no initial Poynting momentum. Finite propagation, recoil, and Lorentz force
therefore remain open.

---

## 7. Exact work and force boundary

The inactive and active seeds have the same sixty-four-token count. Hence the
positive one-unit-per-token ledger gives

\[
 \Delta H_{\rm token}=0.                            \tag{17}
\]

It cannot pay positive radiation energy, determine recoil, or normalize a
source--field coupling. A physical completion requires a nondegenerate
collision/action curvature on the signed particle--hole mode and an equal and
opposite matter increment. Only after that reciprocal invariant exists can a
lattice Lorentz response be tested.

The next local gate is therefore precise:

1. make a recurrent material route retain the ordered turn (9);
2. schedule the sixty-four-record excitation through the finite collision and
   streaming map without channel over-occupation;
3. derive a positive field energy distinguishing \(Z\) from \(A\);
4. close the same energy and momentum ledger with material recoil; and
5. recover the electric work and magnetic no-work identities from that finite
   exchange, not by inserting the continuum Lorentz formula.

Until those pass, FTD has a reversible transverse release **vertex**, not yet
a native electromagnetic force law or a measured fine-structure coupling.

The later
[handed directional-port theorem](THEOREM_COTANGENT_HANDED_DIRECTIONAL_RADIATION_PORT_AND_MOMENTUM_BOUNDARY_v1.md)
closes items 1 and 3 conditionally on the exact missing type. One spatial
pseudoscalar selects the polar normal; the reduced sixteen-record port then
has nonzero \(E\), \(B\), Poynting momentum, a unit canonical norm increment,
and an exact streaming centroid. The planar material loop has not yet
generated that pseudoscalar, absorbed the reciprocal recoil, or supplied the
finite collision preserving coarse Maxwell energy.
