# C4 half-admitted energy current and mechanical-momentum boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT NATIVE ENERGY-CENTROID CURRENT]** +
**[THEOREM — RAW \(E\times B\) READOUT IS THREE TIMES THE HYDRODYNAMIC ENERGY CURRENT]** +
**[SCOPED NO-GO — ONE-TICK MATERIAL HOP IS NOT A STABLE \(c_{\rm eff}=1/6\) RECOIL WORLDLINE]** +
**[THEOREM, CONDITIONAL — SYMMETRIC-STRESS MOMENTUM/WORK IMPLICATION]** +
**[SUCCESSOR — SLOW MATERIAL WORLDLINE AND QUADRATIC NOETHER/LEGENDRE MAP CLOSED CONDITIONALLY]** +
**[OPEN — FIELD NOETHER CHARGE AND COMPLETE COMMON ACTION]**  
**Production status:** unchanged  
**Ledger status:** no row minted; the conditional (1/7) is not a coupling prediction

**Exact certificate:**
[proof_c4_half_admitted_energy_current_momentum_boundary.py](../../../../../scripts/proofs/proof_c4_half_admitted_energy_current_momentum_boundary.py)
performs **10,952 exact checks**. It exhausts all 24 ordered SC frames, both
propagation branches and both charge orientations, then exhausts every C4
phase origin, all twelve internal stages, both charges, and both time-shifted
parity schedules on a representative spatial orbit. It uses no fitted
coefficient, experimental momentum, master root, or target coupling.

---

## 1. The selected field energy has eight equal transported groups

The
[C4-trivial field metric](../charge_gauss_native_em/THEOREM_C4_TRIVIAL_FIELD_SECTOR_UNIQUE_DIRECTIONAL_PORT_HANDOFF_METRIC_v1.md)
merges the two C4 phase bands within each internal-handedness ray but retains
different handedness flags as orthogonal energy channels. The outgoing carrier
therefore has eight phase-paired groups, each with exact energy

\[
 h_a={1\over8},\qquad \sum_{a=1}^{8}h_a=1.                \tag{1}
\]

This energy assignment is unchanged by every movement and hold tick of the
[half-admitted carrier](../charge_gauss_native_em/THEOREM_C4_PHASE_PARITY_HALF_ADMITTED_TWO_POLARIZATION_MAXWELL_CARRIER_v1.md).

Define the uncontained-lattice energy centroid

\[
 X_E(t)=\sum_{a=1}^{8}h_a x_a(t).                          \tag{2}
\]

Because total field energy is one, no further denominator is required.

---

## 2. Exact energy-current pulse train

On every six-tick interval, five centroid increments vanish and one equals the
outgoing direction:

\[
 \boxed{
 \left\{X_E(t+j+1)-X_E(t+j):j=0,\ldots,5\right\}
 =\{0,0,0,0,0,r\}}                                       \tag{3}
\]

as a multiset. The nonzero pulse location depends on the C4 time origin, but
the six-tick total does not:

\[
 \boxed{X_E(t+6)-X_E(t)=r.}                               \tag{4}
\]

The native hydrodynamic energy current is therefore

\[
 \boxed{J_E={r\over6}.}                                   \tag{5}
\]

Equation (5) follows from the conserved microscopic energy and its actual
transport. It is not inferred from continuum electromagnetism.

---

## 3. The raw Poynting readout is not yet momentum

At every movement **and hold** tick, the clock-matched bilinear field readout
is

\[
 P_{EB}={r\over2}.                                        \tag{6}
\]

Combining equations (5)--(6) gives

\[
 \boxed{P_{EB}=3J_E.}                                     \tag{7}
\]

This proves that the previously named “Poynting momentum” cannot literally be
the transported energy current or canonical momentum. It remains nonzero on
ticks when no energy group moves. The factor three is the exact
clock/velocity-quadrature normalization of this carrier.

Accordingly, the older displacement identity

\[
 \Delta x_M+\Delta P_{EB}=0                               \tag{8}
\]

is a reciprocal **readout** ledger, not a physical momentum-conservation law.
This strengthens rather than contradicts its original boundary tag.

---

## 4. Conditional symmetric-stress implication

The native theorem ends with equation (7). To display the remaining price,
add the explicitly conditional continuum requirements:

1. the blocked stress tensor is symmetric;
2. its limiting speed is (c_{\rm eff}=1/6); and
3. the free packet has physical energy (E_F=\Gamma).

Then the integrated stress relation would give

\[
 p_F={J_E\Gamma\over c_{\rm eff}^2}
 ={\Gamma/6\over(1/6)^2}r
 =\boxed{6\Gamma r}.                                      \tag{9}
\]

If the provisional one-hop matter term were simultaneously assigned
(p_M=-\mu r), momentum conservation would force

\[
 \boxed{\mu=6\Gamma.}                                     \tag{10}
\]

Combined with the selected energy partition, equation (10) would imply

\[
 I_*={\Gamma+\mu\over2}={7\Gamma\over2},                  \tag{11}
\]

\[
 \boxed{\chi_{\rm work}={1\over7},\qquad
 \chi_{\rm EM}={2\over7}.}                               \tag{12}
\]

Equations (9)--(12) are exact implications of the added assumptions, **not
native coupling results**. In particular, they are not compared with alpha or
the master root. Their purpose is diagnostic: any future common action that
claims a symmetric stress tensor and the present unit-hop momentum must pass
or explain this relation.

The later clocked-remainder successor removes the present unit-hop momentum
assumption. Its stable material velocity is \(d/L\), its quadratic canonical
momentum is \(p_M=(m/L)d\), and its recoil energy is \(m/(2L^2)\). Therefore
equations (10)--(12) remain only the diagnostic of the obsolete immediate-hop
model; they are not the current slow-worldline energy partition.

---

## 5. Why the present one-hop recoil is not a worldline velocity

The reciprocal port vertex moves the material origin by one SC node in one
global tick. If interpreted directly as coarse velocity,

\[
 |v_M|=1=6c_{\rm eff}.                                    \tag{13}
\]

Thus the one-hop change cannot be the stable observable velocity of massive
matter in the same emergent causal sector. It can still be:

- a local ownership/coordinate re-anchoring;
- an impulse written to a momentum reservoir; or
- one stroboscopic hop distributed over a longer material-clock interval.

The last interpretation is structurally natural because the prepared Petrie
matter clock and the half-admitted radiation carrier both already have a
six-tick macro cadence. A future action may book one center-of-energy recoil
hop per six or more ticks while retaining one-node microscopic locality.

---

## 6. Clocked-worldline successor

The
[clocked-remainder recoil theorem](THEOREM_CLOCKED_REMAINDER_RECOIL_AND_DISCRETE_TRANSLATION_CHARGE_BOUNDARY_v1.md)
constructs the missing finite cadence without adding continuous position.
With \(a\in\mathbb Z_L^3\), the lifted coordinate \(Y=Lx+a\) obeys
\(Y'=Y+d\). An SC impulse gives one visible local hop per \(L\) ticks, hence
stable speed \(1/L\), and \(L\ge6\) stays inside the selected field cone.

Its conditional quadratic translation action derives

\[
 p_M={m\over L}d,\qquad K_M={m\over2L^2}.                 \tag{14}
\]

Signed-cubic symmetry fixes the isotropic tensor form but not \(m\), while
the collision coefficient is \(\kappa=m/L\). Thus the slow material
worldline and leading Legendre map are now closed at reference level. The
field action must still derive its own translation Noether charge and the
same \(\kappa\).

## 7. Updated mechanical gate

The next common-action target must distinguish three objects that were
previously conflated:

1. \(P_{EB}\): the clock-matched local field bilinear;
2. \(J_E\): the exact transported energy current from equation (5); and
3. \(p_{\rm Noether}\): the translation charge derived from the action.

It must then:

1. derive \(L\) and \(m\) from the formed material orbit rather than append
   them;
2. derive the field stress tensor and its relation between \(J_E\) and
   \(p_{\rm Noether}\);
3. realize the canonical impulse exchange in the same action as the
   standing/outgoing conversion;
4. conserve total energy and translation charge through emission and
   absorption; and
5. only afterward evaluate the electromagnetic work/static
   response.

The native energy current and a conditional material momentum map are now
exact. Field momentum and the complete exchange remain action questions, not
relabelings of \(E\times B\) or one lattice displacement.

The subsequent
[symmetric-stress packet theorem](THEOREM_C4_SYMMETRIC_STRESS_PACKET_MOMENTUM_AND_SOURCE_HANDOFF_BOUNDARY_v1.md)
closes the conditional branch anticipated by equation (9). For batch energy
$E=N\Gamma$, the selected relation $J_E=c^2p_F$ uniquely yields
$p_F=6Er$ and $\Sigma_F=Err^{\mathsf T}$. It also identifies
$\Sigma_F=18E t_{\rm evt}$, so recoil and the scalar/STF gravity source are
projections of one stress. The carrier alone still does not derive that
symmetric stress or its real momentum scale.
