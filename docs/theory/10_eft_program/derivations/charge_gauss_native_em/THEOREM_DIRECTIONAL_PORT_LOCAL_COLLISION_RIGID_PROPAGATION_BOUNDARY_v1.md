# Directional-port local-collision rigid-propagation boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT COMPLETE TARGET/TRANSLATION CLASSIFIER]** +
**[CLOSED NEGATIVE, SCOPED — ONE-TICK RIGID OUTWARD PORT PROPAGATION]** +
**[REFERENCE — TWO-RECORD RELATION PERMITS IN-PLANE REFOCUSING]** +
**[OPEN — MULTI-TICK DISPERSIVE OR LARGER COLLECTIVE MAXWELL CARRIER]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_directional_port_local_collision_rigid_propagation_boundary.py](../../../../../scripts/proofs/proof_directional_port_local_collision_rigid_propagation_boundary.py)
performs **248 exact checks**. It exhausts all 768 stage-one port targets
(24 ordered planes, two pseudoscalar signs, four C4 phases, two charge signs,
and two field modes), every translation aligning their four inverse-streamed
collision sites, all local number-plus-six-field totals, and all three
partitions of each four-record site into two unordered pairs. The registered
source represents the full alphabet by signed-cubic, C4,
charge-conjugation, and translation covariance.

---

## 1. The rigid one-tick question

Fix one outgoing port \({\cal O}_0\) with Poynting direction

\[
 r=(0,0,1).                                                  \tag{1}
\]

It occupies four plaquette sites with four cotangent records per site. A
one-tick rigid handoff through a local collision followed by microscopic
streaming would have to satisfy

\[
 \boxed{{\cal U}{\cal C}{\cal O}_0
 =\tau_a{cal O}_1}                                         \tag{2}
\]

for some stage-one port state \({\cal O}_1\) and lattice translation \(a\).
Here \({\cal C}\) is local and preserves record number plus the six \(E/B\)
totals at every collision site, and \({\cal U}\) is the certified record
stream/internal tick.

Instead of guessing \({\cal C}\), apply \({\cal U}^{-1}\) to every possible
target. Equality (2) can hold only if source and inverse-streamed target have:

1. the same four collision sites up to one translation;
2. four records at every site; and
3. identical local \((N,E,B)\) totals.

These are necessary conditions for every collision in the declared class.

---

## 2. Exact classifier result

The 768 target states yield

\[
 128                                                        \tag{3}
\]

position-compatible target/translation pairs. Imposing all six local field
totals leaves exactly

\[
 16.                                                        \tag{4}
\]

Every survivor is outgoing and has the same Poynting direction \(r\). Their
translation set is

\[
 \boxed{
 a\in\{(0,0,0),(1,0,0),(0,1,0),(1,1,0)\},
 \qquad a\cdot r=0.}                                        \tag{5}
\]

Thus the only compatible targets are in-plane re-anchoring presentations of
the same plaquette. None advances the port along its Poynting direction:

\[
 \boxed{
 a\cdot r\ne0\quad\Longrightarrow\quad
 \text{no local }(N,E,B)\text{-preserving solution of (2)}.} \tag{6}
\]

Equation (6) is the scoped closed-negative result.

---

## 3. Pair-collision result

Each source and target collision site contains four distinct records. There
are three pair partitions. For every one of the sixteen survivors, at least
one source partition and one target partition have the same multiset of pair
\((E,B)\) totals.

Therefore the **complete relation** of two-record field-preserving collisions
can refocus all sixteen in-plane presentations. This is not yet one selected
global \(O_h\times C_4\)-equivariant involution, and it does not change
equation (5): pair collisions can reparameterize the local port but cannot
make it move outward in one tick.

---

## 4. Consequence for the common action

The directional port has real nonzero Poynting momentum, and each microscopic
ray record streams with positive three-tick projection along \(r\). But those
facts do not make the sixteen-record collective configuration a rigid Maxwell
packet. Local conservation prevents its one-tick port-shape translation in
the Poynting direction.

The next construction must therefore choose one of three honest routes:

1. a multi-tick dispersive wavepacket whose **global** coarse energy and
   centroid propagate even though its local port shape changes;
2. a larger collective carrier with enough local records to close a rigid
   collision/streaming orbit; or
3. a different energy-preserving amplitude/cochain realization in which
   field energy is not the instantaneous squared moment of a classical
   record pile.

This theorem does not close any of those routes. It only excludes one-tick
rigid outward propagation for the present port alphabet and local
number-plus-\(E/B\)-preserving collision class. Recoil energy, Lorentz force,
charged poles, gravity/lensing, Born, and alpha remain open.

The
[post-separation multi-ray successor](THEOREM_DIRECTIONAL_PORT_POSTSEPARATION_MULTIRAY_ENERGY_MOMENTUM_CARRIER_v1.md)
closes route 1 at the carrier level: after two ticks the eight spatial rays
never meet again and have constant aggregate norm/Poynting with directional
centroid transport. It does not close the port-to-free energy handoff or the
eight-ray-to-two-mode Maxwell reduction.
