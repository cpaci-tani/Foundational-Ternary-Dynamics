# C18 phase-neutral shared charge/stress vertex v1

**Date:** 2026-08-24

**Status:** **[THEOREM — EXACT C4-INVARIANT EVENT CURRENT AND TENSOR SOURCE]**
+ **[THEOREM — CHARGE-ODD / STRESS-EVEN SOURCE PARITY]** +
**[THEOREM — REVERSIBLE COMMON SOURCE LEDGER]** + **[BOUNDARY — KINEMATIC
MATTER-MEDIATED VERTEX, NOT RECIPROCAL FIELD COUPLING]**

**Production status:** unchanged

**Ledger status:** no row minted

**Exact certificate:**
[proof_c18_phase_neutral_shared_charge_stress_vertex.py](../../../../../scripts/proofs/proof_c18_phase_neutral_shared_charge_stress_vertex.py)
performs 1,656 exact checks across all nine C18 lines, four C4 phases, two
orientations, charge conjugates, phase rotations, and inverse transfers.

---

## 1. Why this vertex is required

The
[phase-complete C4 selection theorem](../gravity_cosmology/THEOREM_COTANGENT_PHASE_COMPLETE_COMMON_CLOSURE_AND_C4_SELECTION_v1.md)
proves that a C4-equivariant vacuum linear operator cannot mix the
phase-independent Maxwell carrier with the tensor quadratures. Therefore a
common physical coupling must be phase neutral and nonlinear, or mediated by
a manifested token/matter record.

The existing
[actualization shared-moment vertex](THEOREM_C18_ACTUALIZATION_SHARED_MOMENT_SOURCE_VERTEX_v1.md)
already carries the required data. A manifested token owns:

\[
 (u,v)\in\{(1,0),(0,1),(-1,0),(0,-1)\},\qquad
 \epsilon\in\{-1,+1\}.                       \tag{1}
\]

On a normalized C18 line \(d\), put \(M=dd^{\mathsf T}\). One event gives

\[
 \Delta R_u={\epsilon u\over9}d,\qquad
 \Delta R_v={\epsilon v\over9}d,              \tag{2}
\]

\[
 \Delta Q={u\over18}M,\qquad
 \Delta P={v\over18}M,\qquad
 \Delta K=-{1\over18}M.                       \tag{3}
\]

The new question is whether the token's own phase supplies the compensating
C4 covector required to form invariant physical sources.

---

## 2. Exact phase-neutral contractions

Contract each source doublet with the same token phase:

\[
 j_{\rm evt}=u\,\Delta R_u+v\,\Delta R_v,      \tag{4}
\]

\[
 t_{\rm evt}=u\,\Delta Q+v\,\Delta P.          \tag{5}
\]

Since \(u^2+v^2=1\),

\[
 \boxed{j_{\rm evt}={\epsilon\over9}d,}        \tag{6}
\]

\[
 \boxed{t_{\rm evt}={1\over18}M=-\Delta K.}   \tag{7}
\]

The orthogonal C4 contractions vanish:

\[
 u\,\Delta R_v-v\,\Delta R_u=0,\qquad
 u\,\Delta P-v\,\Delta Q=0.                   \tag{8}
\]

Thus the event contains one nonzero invariant source in each channel, not an
unpriced second coefficient.

---

## 3. Charge parity

Charge conjugation reverses orientation,

\[
 \epsilon\mapsto-\epsilon.                    \tag{9}
\]

Equations (6)--(7) give

\[
 j_{\rm evt}\mapsto-j_{\rm evt},\qquad
 t_{\rm evt}\mapsto t_{\rm evt},\qquad
 \Delta K\mapsto\Delta K.                    \tag{10}
\]

The same transaction therefore separates the two source parities naturally:

\[
 \boxed{\text{directed electromagnetic candidate source is charge odd},}
\]

\[
 \boxed{\text{tensor/capacity candidate source is charge even}.}
\]

This is the correct qualitative relation for electromagnetic charge versus
universal stress/energy sourcing. It is a structural source theorem, not yet a
proof that the long-distance fields are electromagnetism and gravity.

---

## 4. Exact common normalization ledger

Because \(M^2=M\),

\[
 j_{\rm evt}j_{\rm evt}^{\mathsf T}={1\over81}M, \tag{11}
\]

\[
 t_{\rm evt}^2={1\over324}M,                  \tag{12}
\]

and therefore

\[
 \boxed{
 j_{\rm evt}j_{\rm evt}^{\mathsf T}
 =4t_{\rm evt}^2.}                            \tag{13}
\]

Equations (7) and (13) fix the relative chart normalization of current,
tensor source, and capacity debit for one token event. No measured coupling,
master root, continuum action, or target force enters.

These ratios are not alpha or \(G_N\). Canonically normalized propagators and
reciprocal source response remain absent.

---

## 5. C4 invariance and reversibility

One global phase advance rotates both the payload and source doublets:

\[
 (u,v)\mapsto(-v,u),
\]

\[
 (\Delta R_u,\Delta R_v)\mapsto
 (-\Delta R_v,\Delta R_u),
\]

\[
 (\Delta Q,\Delta P)\mapsto(-\Delta P,\Delta Q).
\]

Their dot contractions (4)--(5) remain exactly fixed. Hence
\(j_{\rm evt}\) and \(t_{\rm evt}\) are legitimate C4-neutral outputs even
though their constituent quadratures are nontrivial C4 types.

The inverse ownership transfer returns the same token to reserve and negates

\[
 (j_{\rm evt},t_{\rm evt},\Delta K,\Delta s_L,\Delta s_R). \tag{14}
\]

The common source is therefore reversible at the transaction level.

---

## 6. Progress toward one native action

The exact microscopic chain is now

\[
 \text{one phase/polarity token transfer}
 \longrightarrow
 \begin{cases}
 (\epsilon,-\epsilon)&\text{manifested endpoint pair},\\
 \epsilon d/9&\text{phase-neutral directed current},\\
 dd^{\mathsf T}/18&\text{phase-neutral tensor source},\\
 -dd^{\mathsf T}/18&\text{capacity debit}.
 \end{cases}                                  \tag{15}
\]

This closes the **source-type** part of the C4 selection boundary. Maxwell and
tensor vacuum sectors need not mix linearly; the manifested event supplies a
single matter-mediated vertex that can source both with the correct charge
parities.

Equation (15) also links actualization to physical consequences. A detector
event is not merely a pointer count: the same reversible ownership change
creates endpoint, current, tensor, and capacity records. Native Born
preparation and basin weights remain open, but the manifested output now has a
common phase-neutral field source.

The later exact
[paired-history phase-neutral actualization theorem](THEOREM_C4_PAIRED_HISTORY_PHASE_NEUTRAL_ACTUALIZATION_SOURCE_VERTEX_v1.md)
supplies a conditional provenance for the ownership control. On a prepared
routed residual bank, the bright predicate is the positive value of the unique
normalized symmetric C4 contraction of two histories. Its (-1) values are
reversibly dark-bound and its (0) values are cross-rail. Thus one nonlinear
pair transaction now connects the finite (|Z_o|^2) count to equation (15),
while autonomous preparation, contextual routing, and reciprocal work remain
open.

---

## 7. Exact boundary

This theorem does not derive:

1. reciprocal work done by either field on the token;
2. charge continuity or a static Coulomb pole;
3. a tensor constraint algebra or spin-2 pole;
4. universal sourcing by a stable composite rather than one event;
5. static gravity, redshift, Shapiro delay, or lensing;
6. a native physical Born pushforward; or
7. a native electromagnetic coupling measurement.

In particular, simultaneous source increments do not prove action
reciprocity. The fields must feed back through the same capacity/work ledger.

---

## 8. Next locked gate

Lift equation (15) into one payload-complete local action that:

1. maps \(j_{\rm evt}\) into the cotangent Gauss packet with exact continuity;
2. maps \(t_{\rm evt}\) into the phase-complete tensor carrier;
3. debits and returns \(\Delta K\) as actual field work;
4. makes field response alter the next admissible token transaction;
5. preserves the global inverse and C4 covariance; and
6. yields direction-stable constrained Maxwell and tensor poles.

Only after this reciprocal gate passes can the static response be used for
lensing and for the blind electromagnetic action-curvature measurement.

**Successor status (2026-08-24).** The
[C4 stress-capacity reciprocal-feedback theorem](THEOREM_C4_STRESS_CAPACITY_RECIPROCAL_FEEDBACK_AND_MAXWELL_PARITY_PRICE_v1.md)
passes the finite even-source part of item 4 and the local-inverse part of item
5: response capacity admits the material clock, persistent stress toggles that
response, and the next admission reads the changed capacity. It does not pass
item 3's physical-work requirement, and its exact parity no-go requires item
1's charge-odd current to enter the distinct Maxwell/cotangent carrier.

The later
[common material/stress/Gauss transaction](THEOREM_C4_COMMON_MATERIAL_STRESS_GAUSS_TRANSACTION_AND_FIELD_BOUNDARY_v1.md)
passes the matched SC source-packet realization of item 1 as well:
\(E/8=9j\) and \(\partial E=\rho\) on the same state that drives even
capacity feedback. The packet remains source dressing, so continuity under
release/propagation and the direction-stable Maxwell pole are still open.

The later
[Hodge-framed all-axis signed-event theorem](THEOREM_HODGE_FRAMED_ALL_AXIS_CONSTRAINT_LIFT_AND_ONE_SIGNED_EVENT_GENERATOR_BOUNDARY_v1.md)
adds the complete local STF-divergence record and one signed canonical
generator. The Hodge flag supplies the two axial plane axes, while the
constraint bundles leave \(J_{\rm EM}\) unchanged; the charge-odd current and
charge-even stress therefore coexist without an appended vector balance.
Native owner/flag formation, Maxwell and tensor poles, reciprocal field work,
coupling normalization, and lensing remain open.
