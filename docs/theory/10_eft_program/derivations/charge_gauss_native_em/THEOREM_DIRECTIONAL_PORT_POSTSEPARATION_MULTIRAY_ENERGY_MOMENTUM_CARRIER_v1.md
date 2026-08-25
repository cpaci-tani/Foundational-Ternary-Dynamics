# Directional-port post-separation multi-ray energy/momentum carrier v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT ALL-FUTURE RAY NONINTERSECTION]** +
**[THEOREM — POST-SEPARATION CONSTANT COARSE NORM AND POYNTING]** +
**[THEOREM — FINITE REVERSIBLE DIRECTIONAL MULTI-RAY PROPAGATION]** +
**[BOUNDARY — RAW COARSE-MOMENT HANDOFF DEFECT]** +
**[CLOSED CONDITIONALLY BY SUCCESSOR — C4-TRIVIAL CHANNEL METRIC]** +
**[CLOSED CONDITIONALLY BY SUCCESSOR — RANK-TWO C4 HALF-ADMITTED MAXWELL CONE]** +
**[OPEN — ACTION SELECTION AND NONLINEAR PROTECTION]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_directional_port_postseparation_multiray_energy_momentum.py](../../../../../scripts/proofs/proof_directional_port_postseparation_multiray_energy_momentum.py)
performs **5,655 exact checks**. It solves every pairwise ray-intersection
equation on the uncontained integer lattice, then exhausts all 24 ordered SC
planes, both propagation signs, both charge orientations, fourteen displayed
ticks, exact inverse streaming, and every three-tick BCC displacement. No
finite-box recurrence, eigensolver, target speed, or measured constant enters.

---

## 1. The outgoing port contains eight spatial rays

The outgoing directional port has two C4 phase-distinct copies of one
eight-record ray bank. Each bank contains one record for every plaquette-edge
and internal-handedness pair. The two phase bands have identical spatial flags
and the current field readout is phase blind, so they follow the same eight
trajectories with identical clock-matched readout. This is not yet an
identification with complex-amplitude phase coherence.

For ray \(a\), write its position at tick \(t=3n+s\), \(s\in\{0,1,2\}\), as

\[
 x_a(t)=x_a(0)+n\Delta_a+u_{a,s},                            \tag{1}
\]

where \(\Delta_a\) is the exact three-tick BCC displacement and \(u_{a,s}\)
is the finite residue-prefix displacement.

For every pair \(a\ne b\), an intersection at fixed residue would require

\[
 (x_a(0)-x_b(0))+(u_{a,s}-u_{b,s})
 +n(\Delta_a-\Delta_b)=0.                                   \tag{2}
\]

The certificate solves all 28 ray pairs and three residues over exact
rationals. The complete collision census is

\[
 \boxed{
 4\text{ ray-pair coincidences at }t=0,
 \quad4\text{ at }t=1,
 \quad0\text{ for every }t\ge2.}                            \tag{3}
\]

Thus after two ticks the eight rays never intersect again on the uncontained
lattice. This statement concerns finite-support trajectories on the
undefined-boundary substrate; it does not use a periodic-box limit.

---

## 2. Exact post-separation field ledger

At ticks zero and one, internal-handed ray pairs remain co-located. Their
coarse local-moment aggregation gives

\[
 h_{m port}=2,qquad p_{m port}=r.                       \tag{4}
\]

At every tick \(t\ge2\), there are exactly eight occupied sites with two
phase-distinct records per site. The two records have identical
clock-matched \((E,B)\) readout. Summing the certified cotangent metric and
Poynting moment gives

\[
 \boxed{
 h_{\rm free}=1,qquad p_{\rm free}={r\over2}
 \quad\text{for all }t\ge2.}                                \tag{5}
\]

Equation (3) makes equation (5) an all-future theorem, not an inference from a
finite time display. Record number remains sixteen and the microscopic stream
has an exact local inverse.

---

## 3. Directional propagation

Every ray displacement satisfies

\[
 \Delta_a\cdot r=1.                                         \tag{6}
\]

The lateral components cancel over the eight-ray bank:

\[
 \sum_{a=1}^{8}\Delta_a=8r.                                 \tag{7}
\]

Therefore its centroid obeys

\[
 \boxed{X_{\rm centroid}(t+3)-X_{\rm centroid}(t)=r.}       \tag{8}
\]

The carrier is finite, reversible, polar-directional, and has nonzero stable
post-separation Poynting momentum. This is stronger than the earlier
first-order Bloch cone and stronger than a locally reabsorbed seed.

---

## 4. The handoff defect

Raw streaming changes the coarse quadratic readout during separation:

\[
 (h,p):(2,r)\longrightarrow(1,r/2).                         \tag{9}
\]

Microscopic record number and permutation norm do not change. Equation (9)
comes from loss of local cross terms when initially co-located ray records
separate. Unless another owned coordinate receives the missing unit, the
instantaneous squared field moment is not a conserved Hamiltonian through the
port-to-free handoff.

Consequently the source work partition cannot use equation (4) at emission
and equation (5) in free flight without accounting for their difference. A
physical common action must either:

1. transfer the cross-term work to material/capacity during the two-tick
   handoff;
2. define an energy on the full record amplitudes whose blocked readout is
   equation (5); or
3. add a finite collision that preserves the physical quadratic form while
   the bank separates.

The exact
[C4-trivial field-sector successor](THEOREM_C4_TRIVIAL_FIELD_SECTOR_UNIQUE_DIRECTIONAL_PORT_HANDOFF_METRIC_v1.md)
selects option 2 within the registered quadratic class. The C4 phase-blind
field type fixes the opposite-phase cross weight, and exact handoff
conservation uniquely resolves the internal-handedness channels. The resulting
physical candidate metric gives

\[
 (H,P):(1,r/2)\longrightarrow(1,r/2),                     \tag{9a}
\]

so the raw coarse-moment defect in equation (9) is absent. A common action
still has to realize that selected metric; it may not be adopted merely by
renaming the coarse readout.

---

## 5. Maxwell boundary

The free carrier contains eight ballistic BCC rays. It has a stable aggregate
direction, norm, and Poynting momentum, but it has not been reduced to the two
transverse hydrodynamic polarization pairs of the certified Maxwell slow
sector. Nor has a finite collision proved exact Gauss propagation and the
Maxwell energy simultaneously at finite amplitude.

FTD therefore now has an exact finite directional field carrier after a
two-tick separation transient, but not yet a photon or native Maxwell action.
The next field gate is a finite common action realizing the selected
energy-preserving metric plus two-mode hydrodynamic reduction, composed with
the recoil-energy partition and its inverse absorption history. Lorentz force,
charged poles, gravity/lensing, Born, and alpha remain open.

The
[coherence-metric successor](THEOREM_DIRECTIONAL_PORT_COHERENCE_METRIC_HANDOFF_AND_PHASE_COMPATIBILITY_BOUNDARY_v1.md)
proves that the apparent missing unit depends on the physical channel Gram.
Handoff-conserving positive metrics exist, but form a continuum with different
emission work. The action must derive the phase/handedness cross-term rule;
choosing the coarse moment square or a resolved metric by hand would merely
move the normalization ambiguity.

The C4-trivial successor supplies the missing field-type datum and uniquely
selects the resolved metric. The remaining ambiguity is no longer the handoff
Gram; it is whether the common finite action dynamically owns that metric and
reduces the eight rays to the protected Maxwell sector.

The exact
[C4 phase-parity half-admission successor](THEOREM_C4_PHASE_PARITY_HALF_ADMITTED_TWO_POLARIZATION_MAXWELL_CARRIER_v1.md)
closes the latter kinematic question. The eight-ray (E/B) readout has rank
two and satisfies (B=r\times E) ray by ray. Moving only on one C4 phase parity
preserves the selected energy/Poynting ledger and gives a twofold-degenerate
first-order cone with speed (1/6). The remaining debt is action selection
and nonlinear protection, not an eight-versus-two polarization count.
