# C4 symmetric-stress packet momentum and source-handoff boundary v1

**Date:** 2026-08-24

**Status:** **[THEOREM — FINITE CARRIER DOES NOT SELECT REAL MOMENTUM SCALE]** +
**[THEOREM, CONDITIONAL — SYMMETRIC STRESS UNIQUELY FIXES PACKET MOMENTUM]** +
**[THEOREM, CONDITIONAL — ONE STRESS HAS EXACT SCALAR/STF EVENT PROJECTIONS]** +
**[THEOREM, CONDITIONAL — RECOIL/MASS ADMISSION BOUNDARY]** +
**[OUTCOME B — COMMON STRESS COMPLETION EXACT, NATIVE ORIGIN OPEN]** +
**[OPEN — FINITE STRESS SYMMETRY, TENSOR DYNAMICS, LENSING, AND SCALE]**

**Production status:** unchanged

**Ledger status:** no FTD claim row minted

**Locked preregistration:**
[PREREG_C4_SYMMETRIC_STRESS_PACKET_MOMENTUM_AND_SOURCE_HANDOFF_v1.md](../../preregistrations/common_action_mechanics_reciprocity/PREREG_C4_SYMMETRIC_STRESS_PACKET_MOMENTUM_AND_SOURCE_HANDOFF_v1.md),
pre-execution SHA-256
`43219F025D3AE29D2454E25FBEFB56A25018510875938C81A399C22C16519267`.

**Exact certificate:**
[proof_c4_symmetric_stress_packet_momentum_and_source_handoff.py](../../../../../scripts/proofs/proof_c4_symmetric_stress_packet_momentum_and_source_handoff.py),
SHA-256
`312FA1071D09FEBE61225A8BAFBA2C6D7994E80A584DE4B9220EE5274ACCB938`,
performs 4,751 exact symbolic, cubic-group, and rational checks. It tests all
six SC rays, all 48 signed-permutation cubic transformations, arbitrary
positive symbolic packet count/energy, the scale-underdetermination control,
the complete scalar/STF decomposition, exact absorption energy/inverse, and
rational fixtures on both sides of the receiver-mass boundary. No floating
point, target coupling, master root, or empirical normalization enters.

---

## 1. What the finite carrier fixes—and what it does not

Let (N\in\mathbb N_{>0}) co-directed complete packets each have field energy
(Gamma>0). Put

\[
 E=N\Gamma.                                             \tag{1}
\]

For an SC ray (r\in\{\pm e_1,\pm e_2,\pm e_3\}), the exact half-admitted
carrier theorem fixes

\[
 c={1\over6},\qquad v=cr,qquad J_E=Ev={E\over6}r.       \tag{2}
\]

These are native transport facts within the selected C4 carrier. They do not
determine a real canonical momentum. For every (lambda>0),

\[
 p_\lambda=\lambda E r                                \tag{3}
\]

leaves the packet positions, energy density, inverse transport, and equation
(2) unchanged. Different (lambda) nevertheless gives different material
recoil when inserted into the absorption generator.

This is the local packet instance of the already proved Bloch-momentum
boundary: lattice translation gives a torus-valued quasimomentum, while a
real additive lift and its physical scale require retained winding or an
independent action/stress law. Neither (r), (Er), nor the raw
(E\times B) readout is thereby a Noether charge.

---

## 2. Conditional symmetric-stress gate

Now add one explicitly selected requirement: the blocked packet belongs to a
symmetric rank-one stress-energy completion at the same limiting speed. In
lattice tick/node units this requires

\[
 J_E=c^2p_F,qquad \Sigma_F=p_Fv^{\mathsf T}.             \tag{4}
\]

The first relation has the unique solution

\[
 \boxed{p_F={J_E\over c^2}={E\over c}r=6Er.}             \tag{5}
\]

The spatial stress is then

\[
 \boxed{\Sigma_F=p_Fv^{\mathsf T}=E,rr^{\mathsf T}.}    \tag{6}
\]

Equations (5)--(6) are exact consequences of equation (4). Equation (4) is
not yet derived by the finite carrier. The theorem therefore closes a unique
conditional completion, not native field momentum.

This distinction is physically important. The momentum normalization is not
an independent number once stress-energy symmetry and the propagation speed
are fixed; but neither can be inferred from configuration transport alone.

---

## 3. The manifestation event already has the required stress type

The phase-neutral shared-source vertex emits the charge-even event dyad

\[
 t_{\rm evt}={rr^{\mathsf T}\over18}.                   \tag{7}
\]

Combining equations (6)--(7) gives the exact handoff

\[
 \boxed{\Sigma_F=18E,t_{\rm evt}.}                     \tag{8}
\]

Thus no new tensor **type** is required to express the packet stress. The
same manifestation dyad has the two forced irreducible projections

\[
 \boxed{\rho_F=\operatorname{tr}\Sigma_F=E,}             \tag{9}
\]

and

\[
 \boxed{
 \Pi_F=\operatorname{STF}(\Sigma_F)
 =E\left(rr^{\mathsf T}-{\mathbf1\over3}\right).}       \tag{10}
\]

Equivalently,

\[
 \Sigma_F={E\over3}\mathbf1+\Pi_F.                    \tag{11}
\]

The certificate proves equations (8)--(11) under all 48 signed-permutation
cubic transformations. Both (ho_F) and (Pi_F) are charge even. The
charge-odd electromagnetic current remains the distinct vector projection of
the same manifested token.

The coefficient (18E) is not a gravity coupling. The factor 18 reverses the
registered event-moment normalization, while (E=NGamma) is the packet's
physical energy coefficient. A canonically normalized tensor kinetic action
and its source-response residue are still absent.

---

## 4. Reciprocal absorption with the completed momentum

The reciprocal absorption generator accepts a declared incoming translation
charge (p). Substituting equation (5) removes that declaration within the
conditional symmetric-stress branch:

\[
 P'=P+6Er,                                             \tag{12}
\]

\[
 \omega\Delta I=E+K(P)-K(P+6Er).                      \tag{13}
\]

The complete map remains symplectic, energy conserving, history invertible,
and seam local because equations (12)--(13) are a substitution into the same
frozen generating function, not an appended recoil rule.

For (K(P)=|P|^2/(2m)) and a receiver initially at rest,

\[
 \boxed{
 E=\omega\Delta I+{18E^2\over m}.}                    \tag{14}
\]

Therefore

\[
 \boxed{Delta I\ge0\quad\Longleftrightarrow\quad m\ge18E.} \tag{15}
\]

At (m=18E), the complete packet energy becomes recoil kinetic energy and
the clock receives zero action. For (m<18E), the frozen nonnegative-action
admission gate must reject the rest absorption. This is an exact receiver
capacity boundary, not a mass prediction.

---

## 5. Recoil-corrected electromagnetic compliance

If one admitted batch produces one clock action quantum (I_*), then equation
(14) gives

\[
 I_*={N\Gamma-18N^2\Gamma^2/m\over\omega}.             \tag{16}
\]

Consequently the exact conditional compliance becomes

\[
 \boxed{
 \chi_{\rm EM}={\Gamma\over I_*}
 ={\omega\over N-18N^2\Gamma/m}.}                     \tag{17}
\]

Equation (17) is more restrictive than the momentum-neutral
(chi_{\rm EM}=\omega/N) branch because it prices the recoil required by the
symmetric stress completion. It still fixes no number: (m,omega,Gamma),
and the dynamically admitted batch (N) remain unforced. No comparison with
the algebraic fine-structure root is licensed.

---

## 6. Gravity-source significance and boundary

Equations (8)--(11) identify the exact common source object that the gravity
sector needs:

\[
 \text{one packet stress}
 \longrightarrow
 \begin{cases}
 \rho_F & \text{scalar energy owner},\\
 \Pi_F & \text{STF stress source}.
 \end{cases}                                           \tag{18}
\]

The absorption theorem already preserves complete local energy and hence
scalar (T_{00}) ownership through the field-to-body seam. The present result
shows that, on the symmetric-stress branch, the tensor source is not a second
ad hoc input: it is the STF projection of the very same stress whose flux
fixes recoil.

What remains missing is dynamical tensor ownership. The current absorption
generator contains no tensor canonical pair or constraint multiplier, so it
does not transfer (Pi_F) into the phase-complete tensor carrier. Nor does it
derive the scalar/vector constraints needed to connect a non-TT local source
to the two conditional radiative tensor modes.

Therefore equations (8)--(18) do **not** establish native spin-2 propagation,
lensing, Shapiro delay, nonlinear self-coupling, or Einstein-equivalent
dynamics.

---

## 7. Contextual-measurement consequence

A prepared heralded Gauss event may now be followed conditionally through

\[
 \text{manifestation dyad}
 \longrightarrow
 \text{field energy/current/stress}
 \longrightarrow
 \text{clock action + recoil + scalar/STF record}.       \tag{19}
\]

This makes the physical record richer but does not alter its prepared Born
status. Native formation of the history bank, generic basin measure,
multipartite no-signalling, and macroscopic apparatus recurrence remain open.

---

## 8. Contribution to the one-action programme

The result exposes a useful unification:

> the condition that makes the field stress compatible with a symmetric
> spacetime source is the same condition that fixes the translation charge
> required by material recoil.

Momentum normalization and gravity-source completion are therefore not two
independent arbitrary coefficients on this branch. They are two projections
of one still-selected stress-energy symmetry.

This narrows the next native-action gate. A successful finite action must now
derive equation (4), not separately install packet momentum and tensor source
normalizations. Its variations must generate:

1. the C4 packet continuity/current;
2. symmetric stress and equation (5);
3. reciprocal absorption equations (12)--(13);
4. scalar and STF source transfer equations (9)--(10);
5. the tensor constraint/radiative evolution; and
6. the same exact inverse and capacity ledger.

---

## 9. Epistemic disposition

### Established exactly

- finite carrier energy/current does not select a real momentum scale;
- the symmetric-stress gate uniquely gives (p_F=6Er);
- the corresponding stress is (Sigma_F=Err^{\mathsf T});
- the manifestation dyad supplies its exact scalar and STF projections;
- the construction is cubic covariant and charge even;
- reciprocal absorption remains energy conserving and invertible after the
  momentum substitution;
- rest absorption has the exact mass boundary (m\ge18E); and
- equation (17) is the resulting target-blind compliance identity.

### Still selected or open

1. derivation of stress-energy symmetry from the finite C4 action;
2. a real translation charge/lift and physical scale before blocking;
3. native derivation of (Gamma,m,omega,N), and the absorption trigger;
4. finite tensor ownership and reciprocal STF work transfer;
5. scalar/vector constraints and a protected two-mode tensor pole;
6. inhomogeneous Maxwell propagation, Lorentz force, and scattering;
7. native lensing and nonlinear gravity;
8. native Born preparation and general contextual measurement; and
9. a block-stable native value of (chi_{\rm EM}) or (alpha).

The preregistered disposition is therefore **Outcome B**: an exact common
stress completion and receiver boundary, while the microscopic action that
would force that completion remains open.

---

## 10. Next locked discriminator

Add one phase-complete tensor canonical pair and the minimum scalar/vector
constraint ownership to the absorption seam. The same finite stationary map
must then:

1. derive rather than assume equation (4);
2. transfer (Pi_F) into the tensor owner while preserving total energy and
   the inverse;
3. reduce the sourced tensor state to exactly two radiative modes away from
   the seam;
4. retain the C4 Maxwell packet and its Gauss constraint;
5. reproduce equation (14) as the body-clock/recoil projection; and
6. permit the frozen blind lensing fixtures only after the source/readout
   coefficients are action-derived.

A pass would make recoil and linearized gravity two Noether/source projections
of one finite absorption transaction. Failure would identify whether the
missing price is tensor ownership, constraint memory, or stress symmetry
itself.

### Subsequent existing-type constraint seam (2026-08-24)

The preregistered
[C18 scalar/STF/vector-constraint successor](THEOREM_C18_EXISTING_TYPE_SCALAR_STF_VECTOR_CONSTRAINT_ABSORPTION_SEAM_AND_EQUAL_COUPLING_BOUNDARY_v1.md)
proves that C18 already contains two independent \(T_{1u}\) vector copies.
One can retain the electromagnetic vector while the other conditionally owns
the longitudinal gravity record. For STF source \(S\) and nonzero local
derivative symbol \(q\), preservation of

\[
 {\cal C}_q(\Pi,\kappa)=\Pi q-\kappa
\]

uniquely forces

\[
 \Pi'=\Pi+g_TS,\qquad
 \kappa'=\kappa+g_TS q.
\]

One extended type-2 generator then carries recoil, scalar trace, STF stress,
and vector constraint ownership with exact symplecticity, energy, and inverse;
the homogeneous constrained STF phase space has two canonical tensor modes.
This closes the blocked source-seam/type price, not the finite action. The
charge-even ownership rule, constraint dynamics, static pole, and
\(g_0=g_T\) remain open, so native lensing is not promoted.
