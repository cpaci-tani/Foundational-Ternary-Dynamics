# Directional-port recoil energy partition and coupling-measure boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — UNIQUE SIGNED-CUBIC QUADRATIC RECOIL FORM]** +
**[SCOPED NO-GO — UNIT FIELD/CAPACITY MATCH WITH POSITIVE RECOIL]** +
**[THEOREM, CONDITIONAL — COMMON WORK-PARTITION COUPLING RELATION]** +
**[CORRECTED BY SUCCESSOR — SELECTED FIELD INCREMENT IS ONE HALF]** +
**[CORRECTED BY SUCCESSOR — STABLE RECOIL SPEED IS ONE OVER \(L\)]** +
**[OPEN — DERIVE CADENCE/INERTIA/FIELD ACTION AND PERFORM NATIVE MEASUREMENT]**  
**Production status:** unchanged  
**Ledger status:** no row minted; no alpha comparison

**Exact certificate:**
[proof_directional_port_recoil_energy_partition_obstruction.py](../../../../../scripts/proofs/proof_directional_port_recoil_energy_partition_obstruction.py)
performs **81 exact symbolic checks**. It classifies the full signed-cubic
invariant symmetric quadratic-form space, checks every SC recoil direction,
and proves the energy/momentum partition identities without a target value,
master root, or experimental constant.

---

## 1. Why the previous fully coarse unit ledger cannot be physical energy

The directional port has canonical local field norms

\[
 h_F({\cal S})=1,qquad h_F({\cal O})=2,                    \tag{1}
\]

while the complementary source capacity changes from \(g=1\) to \(g=0\).
Ignoring translational recoil gives the exact dimensionless ledger

\[
 g+h_F=2.                                                    \tag{2}
\]

The reciprocal-recoil theorem then derives a one-SC-step material
displacement. If that displacement carries any positive kinetic energy,
equation (2) has already spent the entire capacity unit on the field and has
nothing left to pay the recoil. Thus equation (2) is not yet a physical total
Hamiltonian.

---

## 2. Unique quadratic recoil type

Let the local quadratic translational energy be

\[
 K(\delta)={1\over2}\delta^{\mathsf T}A\delta,               \tag{3}
\]

with \(A=A^{\mathsf T}\). Full signed-cubic invariance requires

\[
 R^{\mathsf T}AR=A\qquad\text{for all }R\in O_h.             \tag{4}
\]

The exact constraint matrix on the six independent entries of \(A\) has rank
five. Its nullspace is one-dimensional:

\[
 \boxed{A=\mu I_3.}                                         \tag{5}
\]

For every unit SC recoil \(r\),

\[
 \boxed{K(r)={\mu\over2},qquad p_M=\mu r.}                 \tag{6}
\]

Positive recoil inertia means \(\mu>0\). Equation (5) is forced within the
declared quadratic class; the value of \(\mu\) is not.

---

## 3. Provisional fully coarse work partition

Write the dimensional capacity and field contributions as

\[
 H_C=I_*g,qquad H_F=\Gamma h_F.                             \tag{7}
\]

The standing and outgoing energies are then

\[
 H_{\cal S}=I_*+\Gamma,qquad
 H_{\cal O}=2\Gamma+{\mu\over2}.                            \tag{8}
\]

Exact emission energy conservation is equivalent to

\[
 \boxed{I_*=\Gamma+{\mu\over2}.}                            \tag{9}
\]

Therefore the common-action field-work fraction is

\[
 \boxed{
 \chi_{\rm work}:={\Gamma\over I_*}
 =1-{\mu\over2I_*}.}                                       \tag{10}
\]

The no-recoil unit match \(\Gamma=I_*\) leaves the positive uncancelled
defect \(\mu/2\). It is incompatible with any \(\mu>0\) in this class.

If the normalized outgoing field momentum \(r\) carries dimensional scale
\(P_*\), reciprocal momentum conservation with equation (6) gives

\[
 \boxed{P_*=\mu,\qquad
 \chi_{\rm work}=1-{P_*\over2I_*}.}                         \tag{11}
\]

Equations (8)--(11) are exact for the provisional fully coarse field norm used
by this certificate. The later field-type theorem shows that this is not the
handoff-conserving physical candidate metric, so equations (9)--(11) are not
the current coupling normalization.

### 3.1 Successor correction

The exact
[C4-trivial field-sector theorem](THEOREM_C4_TRIVIAL_FIELD_SECTOR_UNIQUE_DIRECTIONAL_PORT_HANDOFF_METRIC_v1.md)
uses the actual phase-blind field type and exact port/free conservation to
select \((a,b,c)=(0,1,0)\). It gives

\[
 H_{\cal S}={1\over2},\qquad H_{\cal O}=H_{\rm free}=1,
 \qquad \Delta H_F={1\over2}.                              \tag{11a}
\]

The corrected source-work partition is therefore

\[
 \boxed{I_*={\Gamma+\mu\over2},\qquad
 \chi_{\rm work}={\Gamma\over\Gamma+\mu}.}                \tag{11b}
\]

The selected Poynting increment is \(r/2\), but it is not yet a canonical
translation momentum. Accordingly the scale equality in equation (11) is not
retained as a physical result; a discrete Noether/Legendre map and carrier-
speed conversion are still required.

### 3.2 Clocked-worldline correction

The later
[clocked-remainder recoil theorem](../common_action_mechanics_reciprocity/THEOREM_CLOCKED_REMAINDER_RECOIL_AND_DISCRETE_TRANSLATION_CHARGE_BOUNDARY_v1.md)
supplies that material-side map. It replaces an immediate unit-velocity step
by the finite lifted law

\[
 Y=Lx+a,\qquad Y'=Y+d,\qquad v_M={d\over L}.              \tag{11c}
\]

Its conditional quadratic action gives

\[
 p_M={m\over L}d,\qquad K_M={m\over2L^2}.                 \tag{11d}
\]

If \(\mu\) denotes the material impulse magnitude, then

\[
 \mu={m\over L},\qquad K_M={\mu\over2L}.                  \tag{11e}
\]

Thus equation (11b) is itself superseded for a stable slow worldline. The
current conditional work ledger is

\[
 \boxed{
 I_*={\Gamma\over2}+{\mu\over2L},\qquad
 \chi_{\rm work}={\Gamma L\over\Gamma L+\mu}.}            \tag{11f}
\]

Equation (11f) still is not a coupling prediction: \(L\), \(m\), the field
Noether charge, and the common action remain open.

---

## 4. What a native coupling measurement now means

Equation (10) supplies an operational definition that was absent from the
master-root correspondence. A native coupling is not the norm of a packet or
an algebraic root assigned to a familiar symbol. It is the fraction of one
source transaction's work that remains in the propagating field after the
same transaction pays material recoil:

\[
 \boxed{
 \text{native EM work fraction}
 ={\text{outgoing field work}\over\text{released source work}}.} \tag{12}
\]

To evaluate equation (12), the action must independently derive or measure:

1. the source action debit \(I_*\);
2. the translational impulse \(\mu\), cadence \(L\), and inertia \(m=\mu L\);
3. the emitted field coefficient \(\Gamma\);
4. the energy-preserving outgoing Maxwell carrier; and
5. the same partition on absorption and scattering histories.

Only after those quantities arise from one common action may
\(\chi_{\rm work}\) be compared with a physical electromagnetic coupling.
No identification with \(\alpha\), \(\sqrt\alpha\), or the master quadratic
is made here.

The later
[charged-pole reciprocal-alpha protocol](THEOREM_COTANGENT_CHARGED_POLE_RECIPROCAL_ALPHA_MEASUREMENT_PROTOCOL_v1.md)
turns that operational boundary into two basis-fixed estimators. For a
selected common Maxwell--Gauss action, the canonical free-field Hessian and
the charged static Green residue both equal $\chi_{\rm EM}=\Gamma/I_*$, and
$\alpha_{\rm native}=3\chi_{\rm EM}/(2\pi)$. The protocol derives no number:
microscopic action selection, $L,m$, and local reversible realization remain
open.

---

## 5. Updated boundary

This theorem turns the coupling problem into a blind conservation test. It
also removes two tempting but invalid shortcuts:

- equating a canonical field norm to a capacity bit while omitting recoil;
- treating the earlier conditional \(1/16\) packet normalization as a
  measured coupling.

The next gate is to derive \(L\), \(m\), and the field translation charge from
formed material and the common transaction, then realize equation (11f) in a
finite action with the two-mode Maxwell sector. Until then, the field-work
fraction is a conditional relation, not native alpha.

The
[post-separation multi-ray theorem](THEOREM_DIRECTIONAL_PORT_POSTSEPARATION_MULTIRAY_ENERGY_MOMENTUM_CARRIER_v1.md)
now supplies a free-flight interval with exactly constant coarse norm and
Poynting after tick two. Its raw fully coarse readout changes from port norm
\(2\) to free norm \(1\), motivating the channel-metric classification.

The
[coherence-metric successor](THEOREM_DIRECTIONAL_PORT_COHERENCE_METRIC_HANDOFF_AND_PHASE_COMPATIBILITY_BOUNDARY_v1.md)
generalizes the unit field increment assumed here. On an energy-preserving
channel Gram, \(\Delta H_F=(b-a)/2\), so the complete partition is
\(I_*=\Gamma(b-a)/2+\mu/2\). The work fraction remains operational, but the
bare field coefficient could not be extracted at that stage. The C4-trivial
field-sector successor now fixes \(b-a=1\). The still later clocked-worldline
successor replaces the immediate-recoil term by \(\mu/(2L)\), giving equation
(11f). The field action, cadence, and inertia scale remain open.
