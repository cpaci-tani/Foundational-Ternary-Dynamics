# C6 Petrie directional-port reciprocal recoil current v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT LOCAL STANDING/OUTGOING MATTER-FIELD INVOLUTION]** +
**[THEOREM — CHARGE CONTINUITY AND RECIPROCAL DISPLACEMENT CURRENT]** +
**[THEOREM — CANONICAL CAPACITY/FIELD LEDGER AND CUBIC COVARIANCE]** +
**[BOUNDARY — DISPLACEMENT CURRENT IS NOT YET PHYSICAL MOMENTUM OR LORENTZ FORCE]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_c6_petrie_directional_port_recoil_current.py](../../../../../scripts/proofs/proof_c6_petrie_directional_port_recoil_current.py)
performs **801,794 exact checks**. It exhausts 41,472 located matched states
across all 48 ordered SC triads, six route phases, twelve cotangent stages,
both charge signs, both port modes, three translated origins, and the complete
signed cubic group. No mass, dispersion, measured coupling, master root, or
continuum force law enters.

---

## 1. The exact local vertex

The cubic-Petrie material clock supplies the current ordered plane, its
route-history pseudoscalar, and the outgoing polar direction

\[
 r=e_{q+2}=\chi_q(e_q\times e_{q+1}).                       \tag{1}
\]

Let \(x\) denote the spatial origin of the currently manifested neutral
dipole. The directional cotangent port has a standing mode \({\cal S}\) with
field momentum zero and an outgoing mode \({\cal O}\) with field momentum
\(r\). Define the collision substep

\[
 \boxed{
 (x,{\cal S},g=1)
 \longleftrightarrow
 (x-r,{\cal O},g=0).}                                      \tag{2}
\]

The map is an involution. It is target blind: \(r\) is read from the retained
material route, not from a requested outcome or external direction.

---

## 2. The field interaction vertex stays fixed

For a standing state define the field-port anchor to be \(x\). For an
outgoing state define it to be \(x+r\). Equation (2) then gives

\[
 a_{\rm port}^{\rm after}=(x-r)+r=x
 =a_{\rm port}^{\rm before}.                               \tag{3}
\]

Thus the sixteen retained cotangent records change mode at one fixed local
interaction vertex while the manifested matter moves one SC step in the
opposite direction. Eight field records are retained and eight are swapped;
public field-record number remains sixteen.

The reverse event is literal incoming absorption at the same vertex:

\[
 (x-r,{\cal O},0)\longmapsto(x,{\cal S},1).                 \tag{4}
\]

No event log is needed because equation (4) is the same local involution.

---

## 3. Manifested continuity

Both charged endpoints of the neutral dipole translate by the same SC step
\(\Delta x_M=\mp r\). Depositing each endpoint charge on that oriented hop
gives

\[
 \boxed{\rho^{+}-\rho^{-}=\partial j_{\rm recoil}.}         \tag{5}
\]

Net ternary charge remains zero. Charge conjugation reverses the two endpoint
currents and both charge distributions, while leaving the spatial recoil and
field Poynting direction unchanged.

Equation (5) is an actual manifested-state transport statement, not a label
attached to the material clock.

---

## 4. Exact local energy and reciprocal vector ledger

The handed-port theorem gives

\[
 h_F({\cal S})=1,qquad h_F({\cal O})=2,
 \qquad
 p_F({\cal S})=0,qquad p_F({\cal O})=r.                    \tag{6}
\]

With the complementary capacity bit in equation (2), every collision state
obeys

\[
 \boxed{g+h_F=2.}                                           \tag{7}
\]

The material displacement current and field Poynting change satisfy

\[
 \boxed{\Delta x_M+\Delta p_F=0.}                           \tag{8}
\]

Equations (7) and (8) are preserved under every signed cubic transformation.
The collision commutes with charge conjugation and has the exact inverse (4).

---

## 5. Why equation (8) is not yet physical momentum conservation

The vector \(\Delta x_M\) is a one-tick manifested displacement relative to
the no-emission branch. It is therefore a native recoil **current**. A physical
momentum requires an action-derived Legendre map or dispersion relation,
schematically

\[
 p_M={\partial L_d\over\partial(\Delta x_M)},                \tag{9}
\]

and an associated translational kinetic-energy term. Neither follows from the
incidence identity (5) or the vector cancellation (8).

Assigning unit mass and declaring \(p_M=\Delta x_M\) would convert equation
(8) into momentum conservation by definition. This theorem does not make that
assignment. Likewise, equation (7) is a canonical dimensionless port ledger;
the relative dimensional field/capacity action scale remains selected.

---

## 6. Handoff boundary

Equation (2) is a collision vertex. The outgoing records are still represented
at the source port. The separate microscopic streaming theorem can move them
away reversibly, but collisionless spreading does not preserve the coarse
quadratic Maxwell norm.

A physical emission/absorption action must therefore compose, in one exact
ordered tick:

1. the local collision (2);
2. ownership transfer from the source port to the vacuum carrier;
3. an energy-preserving finite Maxwell collision/streaming map;
4. a material Legendre/dispersion map turning recoil current into momentum;
5. the reverse incoming handoff and absorption event; and
6. electric work and magnetic no-work identities on the resulting coupled
   trajectory.

Until those pass, FTD has exact reciprocal recoil kinematics, not a native
Lorentz force, photon, or coupling measurement. Formation/binding, charged
poles, gravity/lensing, physical Born preparation, and alpha remain open.

The
[recoil energy-partition successor](../charge_gauss_native_em/THEOREM_DIRECTIONAL_PORT_RECOIL_ENERGY_PARTITION_AND_COUPLING_MEASURE_BOUNDARY_v1.md)
proves that any positive cubic-invariant quadratic kinetic energy invalidates
the no-recoil unit field/capacity match. It replaces that shortcut by the
provisional fully coarse conservation equation. The later C4-trivial field
metric corrects emitted field work to \(\Gamma/2\). The still later
clocked-remainder successor fixes the stable material debit conditionally to
\(\mu/(2L)\), with \(\mu=m/L\). Deriving \(L,m\), and the field Noether
charge is therefore simultaneously the next momentum and native-coupling
gate.

The exact
[half-admitted energy-current successor](THEOREM_C4_HALF_ADMITTED_ENERGY_CURRENT_AND_MECHANICAL_MOMENTUM_BOUNDARY_v1.md)
sharpens the interpretation of this theorem's vector ledger. The selected
carrier transports energy at \(J_E=r/6\), while the clock-matched
\(E\times B\) readout is \(r/2=3J_E\). A one-tick material hop would also be
six times the emergent light speed if treated as a stable worldline velocity.
Thus \(\Delta x_M+\Delta p_F=0\) remains an exact displacement/readout
identity, but it is not the final mechanical momentum law. A slow material
recoil clock and discrete Noether/Legendre map are required.

The later
[clocked-remainder recoil successor](THEOREM_CLOCKED_REMAINDER_RECOIL_AND_DISCRETE_TRANSLATION_CHARGE_BOUNDARY_v1.md)
closes that material-side reference gate. It replaces the immediate
relocation by the impulse write

\[
 (d=0,{\cal S},g=1)\longleftrightarrow
 (d=-r,{\cal O},g=0),
\]

then transports the finite lift \(Y=Lx+a\) by \(Y'=Y+d\). The visible body
moves one SC node per \(L\) ticks, and a conditional quadratic action gives
\(p_M=(m/L)d\) and \(K_M=m/(2L^2)\). This does not retroactively turn
\(\Delta p_F\) above into field Noether momentum. The field action,
\(L,m\), and common coefficient remain open.
