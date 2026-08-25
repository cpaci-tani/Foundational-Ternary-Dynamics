# Cotangent handed directional radiation port and momentum boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — ORDERED-PLANE POLAR-NORMAL OBSTRUCTION]** +
**[THEOREM — PSEUDOSCALAR COMPLETION AND CUBIC-COVARIANT DIRECTIONAL PORT]** +
**[THEOREM — EXACT CANONICAL NORM/POYNTING/STREAMING CERTIFICATE]** +
**[SELECTION, CONDITIONAL — CAPACITY/FIELD ACTION-SCALE MATCH]** +
**[BOUNDARY — PSEUDOSCALAR OWNERSHIP, COARSE ENERGY, RECOIL, AND FORCE OPEN]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_cotangent_handed_directional_radiation_port.py](../../../../../scripts/proofs/proof_cotangent_handed_directional_radiation_port.py)
performs **177,939 exact checks**. It exhausts all 9,216 matched port states,
all 24 ordered SC planes, both spatial-pseudoscalar branches, both charge
orientations, all four C4 phases, all twelve cotangent stages, both field
modes, and the complete 48-element signed cubic group. No measured target,
master root, numerical eigensolver, or fitted coefficient enters.

---

## 1. Why the planar material turn cannot point outward

Let \((d,v)\) be the ordered perpendicular polar frame supplied by the square
material clock. Its geometric normal

\[
 n_A=d\times v                                             \tag{1}
\]

is **axial**, not polar. The signed-cubic stabilizer of \((d,v)\) has two
elements: the identity and the reflection through the plane. That reflection
fixes both polar frame legs and reverses every polar vector perpendicular to
the plane. Consequently there is no nonzero equivariant map

\[
 (d,v)\longmapsto r_{\perp}\quad\text{with}\quad
 r_{\perp}\cdot d=r_{\perp}\cdot v=0                       \tag{2}
\]

when \(r_{\perp}\) is required to be polar.

This is the concrete radiation version of the earlier source-frame
handedness obstruction: an ordered planar turn supplies a polarization plane,
but it does not distinguish the two physical propagation directions normal to
that plane.

Introduce one spatial pseudoscalar

\[
 \chi\in\{-1,+1\},\qquad \chi\mapsto\det(R)\chi.             \tag{3}
\]

Then

\[
 \boxed{r=\chi(d\times v)}                                  \tag{4}
\]

is polar and obeys \(r\mapsto Rr\) under every \(R\in O_h\). Equation (4)
is sufficient and exact. It does not prove that the planar proto-matter loop
owns \(\chi\).

---

## 2. Eight-record directional ray bank

For each oriented plaquette edge \(e\), define the axial magnetic direction

\[
 b_e=r\times e.                                             \tag{5}
\]

Retain both internal cotangent handedness records

\[
 f_{e,h}^{(\epsilon)}=(\epsilon e,\epsilon b_e,h),
 \qquad h=\pm1,                                             \tag{6}
\]

at one common C4 phase. The four edges therefore carry eight records. At the
clock-matched cotangent layer their local readout is

\[
 E_e=2\epsilon e,qquad B_e=2\epsilon b_e,                  \tag{7}
\]

and

\[
 E_e\times B_e=4r.                                         \tag{8}
\]

The electric edges form the primal plaquette boundary. The magnetic vectors
form the corresponding staggered dual plaquette boundary. The certificate
checks both incidence divergences exactly:

\[
 \partial E=0,qquad \partial_*B=0.                         \tag{9}
\]

Charge conjugation \(\epsilon\mapsto-\epsilon\) reverses both \(E\) and
\(B\), while fixing energy and the Poynting direction.

The extra pseudoscalar reduces the earlier context-free stabilizer price. The
64-record plane-only seed had to average the complete normal/handed orbit and
therefore began with \(B=0\). The present directional bank may retain the
magnetic partner because equation (4) supplies the missing normal branch.
No record-minimality claim outside this declared cotangent alphabet is made.

---

## 3. Fixed-number standing and outgoing modes

Use two phase-distinct eight-record banks. Let \({\cal R}_{\chi}^{p}\) denote
one bank with propagation branch \(\chi\) and phase \(p\). Define

\[
 \begin{aligned}
 {cal S}_{\chi}^{p}
   &= {\cal R}_{\chi}^{p}+{\cal R}_{-\chi}^{p+2},\\
 {\cal O}_{\chi}^{p}
   &= {\cal R}_{\chi}^{p}+{\cal R}_{\chi}^{p+2}.
 \end{aligned}                                             \tag{10}
\]

Both modes contain exactly sixteen public records. Their intersection has
eight records, so the reversible port event retains eight and swaps eight.
At the local release surface,

\[
 \begin{array}{c|cc}
  & E_e & B_e\\ \hline
  {cal S} & 4\epsilon e & 0\\
  {cal O} & 4\epsilon e & 4\epsilon b_e .
 \end{array}                                                \tag{11}
\]

Thus \({\cal S}\) is the counterpropagating standing port and \({\cal O}\)
is the selected outgoing port. This is a conversion of a retained field
resource, not creation from a blank.

---

## 4. Canonical local energy and Poynting units

The certified cotangent slow-space Gram is \(64I_6\). Define the local
configuration norm and Poynting readout

\[
 h_F={1\over64}\sum_{e\subset\partial\square}
       (|E_e|^2+|B_e|^2),
 \qquad
 p_F={1\over64}\sum_{e\subset\partial\square}E_e\times B_e.
                                                               \tag{12}
\]

Equations (10)--(12) give exactly

\[
 \boxed{
 h_F({\cal S})=1,\quad h_F({\cal O})=2,
 \qquad p_F({\cal S})=0,\quad p_F({\cal O})=r.}             \tag{13}
\]

Let the already-retained response capacity be \(g=1\) in the standing mode
and \(g=0\) in the outgoing mode. The mode toggle, capacity toggle, and one
cotangent-stage advance form a finite permutation with explicit inverse and

\[
 \boxed{g+h_F=2,qquad \Delta h_F=-\Delta g.}                \tag{14}
\]

Unlike the plane-only 64-record seed, equation (14) needs no inserted
division by sixteen inside the canonical field metric: the standing-to-
outgoing norm increment is already one.

This does **not** derive a physical electromagnetic coupling. If

\[
 H_C=I_*g,qquad H_F=\Gamma h_F,                              \tag{15}
\]

then treating equation (14) as dimensional action conservation still selects
\(\Gamma=I_*\). The finite port proves a unit-matched section exists; it does
not force the relative sector scale or measure alpha.

---

## 5. Exact microscopic propagation

Every record streams one SC step along its current tangent and then applies
the already-certified cotangent internal tick. This is a strict local
permutation with an explicit inverse. For every record in one outgoing ray
bank, its three-tick BCC displacement \(\Delta_f\) obeys

\[
 \Delta_f\cdot r=1.                                         \tag{16}
\]

The two internal-handed records cancel their lateral components, and the four
plaquette edges cancel theirs, so

\[
 {1\over8}\sum_{f\in{\cal R}_{\chi}^{p}}\Delta_f=r.         \tag{17}
\]

The standing port combines the \(r\) and \(-r\) centroids and has zero net
streaming direction; the outgoing port doubles the \(r\) branch. This is an
exact finite carrier propagation statement, not merely a Bloch derivative.

The streamed records spread laterally. The certificate does not show that the
coarse quadratic norm in equation (12) is preserved after that spreading, nor
that collision reduces the microscopic ray family to the two protected
Maxwell polarizations. Therefore the port is not yet a finite-amplitude photon
or Maxwell soliton.

---

## 6. Exact momentum debt

The release event changes field momentum by

\[
 \Delta p_F=r.                                               \tag{18}
\]

Total momentum would require the simultaneous material impulse

\[
 \boxed{\Delta p_M=-r.}                                      \tag{19}
\]

Equation (19) is uniquely fixed as a ledger entry and is polar/cubic
covariant. It has **not** been written into the square material carrier. That
carrier currently has position, internal phase, charge orientation, current,
and stress, but no derived translational momentum coordinate or handed
source. Defining a new momentum label and toggling it by hand would merely
install recoil.

Consequently this theorem closes a directional field port and states the
reciprocal impulse exactly, but it does not yet derive a Lorentz force.

---

## 7. Updated one-action boundary

The strict chain now reaches

\[
 \text{material turn/plane}
 \xrightarrow{\ +\chi\ }
 \text{standing field port}
 \longleftrightarrow
 \text{outgoing }(E,B)\text{ port}
 \xrightarrow{\rm stream}
 \text{directional finite carrier}.                         \tag{20}
\]

The next gate is no longer “find any nonzero field momentum.” It is:

1. derive \(\chi\) from a formed three-dimensional material recurrence or
   another already-owned spatial-parity datum;
2. derive a material translational momentum coordinate and make the same port
   event write equation (19) through its exact inverse;
3. construct the collision/streaming lift that preserves the coarse Maxwell
   energy while protecting only two transverse polarizations;
4. make incoming absorption the literal inverse of outgoing emission; and
5. derive electric work and magnetic no-work from that transaction before
   naming a Lorentz force or measuring a coupling.

Gravity/lensing, physical Born preparation, a charged pole, and native alpha
remain open.

The
[C6 cubic-Petrie material successor](../common_action_mechanics_reciprocity/THEOREM_C6_CUBIC_PETRIE_MATERIAL_CLOCK_AND_ENDOGENOUS_DIRECTIONAL_PORT_v1.md)
closes item 1 on a prepared nonplanar recurrence. Its retained third route
edge supplies both the pseudoscalar determinant and the polar port direction,
so no independent \(\chi\) label is needed there. Items 2--5 remain open:
formation, translational recoil, incoming absorption, and coarse
Maxwell-energy-preserving collision have not been derived.

The subsequent
[reciprocal-recoil vertex](../common_action_mechanics_reciprocity/THEOREM_C6_PETRIE_DIRECTIONAL_PORT_RECIPROCAL_RECOIL_CURRENT_v1.md)
adds a literal local inverse absorption event and the exact manifested
displacement identity \(\Delta x_M=-\Delta p_F\). It remains a recoil current,
not an action-derived momentum, until a translational Legendre/dispersion map
and its kinetic-energy ledger are derived.

The later
[recoil energy-partition theorem](THEOREM_DIRECTIONAL_PORT_RECOIL_ENERGY_PARTITION_AND_COUPLING_MEASURE_BOUNDARY_v1.md)
shows that positive quadratic recoil changes the dimensional scale condition:
conservation requires \(I_*=\Gamma+\mu/2\), not \(\Gamma=I_*\). The unit norm
increment remains exact, while its physical work coefficient must be measured
from the common source/field/recoil partition.

The C4-trivial field metric and clocked-remainder successors subsequently
correct both terms of that historical partition: emitted field work is
\(\Gamma/2\), while a stable speed-\(1/L\) material orbit with impulse
\(\mu=m/L\) costs \(\mu/(2L)\). The current conditional debit is therefore
\(I_*=\Gamma/2+\mu/(2L)\); \(L,m\), and the field translation charge remain
open.

The
[rigid-propagation classifier](THEOREM_DIRECTIONAL_PORT_LOCAL_COLLISION_RIGID_PROPAGATION_BOUNDARY_v1.md)
proves that this sixteen-record port cannot translate outward as one rigid
shape in a single local number-plus-\(E/B\)-preserving collision/streaming
tick. Its microscopic rays remain directional, but the collective Maxwell
carrier must be multi-tick/dispersive, larger, or represented by a different
energy-preserving amplitude structure.
