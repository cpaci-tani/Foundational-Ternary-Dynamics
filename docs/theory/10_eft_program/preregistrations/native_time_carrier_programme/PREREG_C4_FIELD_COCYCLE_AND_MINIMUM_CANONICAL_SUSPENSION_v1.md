# Pre-registration — C4 field cocycle and minimum canonical suspension v1

**Identifier:** `FTD-0973`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Question

What is the minimum honest coupling between the retained FTD-0972 `C4`
carrier and one complete continuous field pair?

The test must separate:

1. a reversible fiber rotation synchronized with the carrier;
2. a state-dependent discrete connection with gauge-invariant holonomy;
3. a genuine canonical suspension with controller reaction; and
4. a production-derived interaction.

The carrier may not be credited with choosing a cocycle, controller action,
mass scale, background frequency, connection profile, `G*` cadence, Born
weight, or outcome. No numerical search, fitted tolerance, engine change, or
production promotion is permitted.

## 2. Frozen sources

| Source | Frozen SHA-256 |
|---|---|
| `THEOREM_ORIENTED_PHASE_CONNECTION_TOKEN_LOADING_AND_SELF_DUAL_GEARBOX_v1.md` | `56711EE1A215F4418A9B8FA5E4EF6C46BD0B2767D407F70E04C7C6A0FD6345B1` |
| `THEOREM_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_AND_CUBIC_CHART_BOUNDARY_v1.md` | `FF80023FA73326B439405C8A07F08A72A5EBD8CC845AC145224B5BE4D647F07C` |
| `THEOREM_MOVING_REGIONAL_FRAME_COTANGENT_CONNECTION_AND_PURE_GAUGE_BOUNDARY_v1.md` | `C5C28405CA439BF2341D545F99E9BDFC985BF65155B1CD49075541CD5C258462` |
| `THEOREM_KRYLOV_DEGENERACY_TERNARY_LATCH_AND_ORIENTED_C4_TRANSITION_v1.md` | `7DA2366C75D38E0EA1F8012632D71C676C4E6F8D1A7F8D1467EAF4185AE77194` |

No public type, ontology rule, selector, clock law, engine source, tick phase,
or production action may change under this protocol.

## 3. Frozen discrete fiber-cocycle class

Let the carrier phase be `k in Z_4`, with forward base update `k -> k+1`.
Let one normalized complete field pair be `z=(Q,P)^T`, with

\[
 \Omega=\begin{pmatrix}0&1\\-1&0\end{pmatrix},qquad
 J=\begin{pmatrix}0&-1\\1&0\end{pmatrix},qquad J^2=-I.      \tag{1}
\]

For an edge cocycle `a=(a_0,a_1,a_2,a_3) in Z_4^4`, define

\[
 U_a(k,z)=(k+1,J^{a_k}z).                                  \tag{2}
\]

The certificate must prove:

- every `U_a` is bijective and fiber symplectic;
- `I_f=(Q^2+P^2)/2` is invariant;
- the exact inverse is
  `U_a^{-1}(k,z)=(k-1,J^{-a_{k-1}}z)`; and
- after one carrier cycle,

\[
 U_a^4(k,z)=(k,J^m z),qquad
 m=\sum_{r=0}^3a_r\pmod4.                                  \tag{3}
\]

Under a vertex-dependent fiber relabelling `z_k -> J^{b_k}z_k`, the cocycle
changes as

\[
 a'_k=a_k+b_{k+1}-b_k\pmod4.                               \tag{4}
\]

The total `m` is invariant. Every cocycle is gauge equivalent to
`(0,0,0,m)`. Thus the carrier does not erase the discrete connection debt;
it exposes it as a `Z_4` holonomy class.

## 4. Symmetry and underdetermination gates

Let `R(k,z)=(k+1,z)` be carrier-phase translation. Prove

\[
 RU_a=U_aR\quad\Longleftrightarrow\quad
 a_0=a_1=a_2=a_3.                                          \tag{5}
\]

Every such homogeneous cocycle is a direct product `T x J^a`, and its
four-step holonomy is identity. The faithful real `C4` representations have
`a=+1` and `a=-1`; carrier orientation exchanges them, but the carrier alone
does not choose either physical identification.

Use the minimum fixed time reversal

\[
 \Theta(k,z)=(-k,Cz),qquad C=\operatorname{diag}(1,-1),
 qquad CJC=J^{-1}.                                         \tag{6}
\]

The covariance condition `Theta U_a Theta=U_a^{-1}` must reduce to

\[
 a_0=a_3,qquad a_1=a_2.                                   \tag{7}

Consequently `m=2(a_0+a_1)` is even. A net full-cycle `+/-J` holonomy cannot
be obtained with this minimum time-reversal action. It requires an additional
vertex-dependent reversal phase, a time-odd port, or explicit symmetry
breaking, each separately priced.

The trivial homogeneous cocycle and faithful homogeneous cocycle must both
pass all base reversibility and norm gates. Their different field actions
prove the FTD-0972 carrier alone underdetermines the coupling.

## 5. Minimum canonical suspension

A continuous Hamiltonian phase source needs a nondegenerate symplectic
controller. One phase variable alone cannot carry a symplectic form; the
minimum continuous controller is one complete pair `(theta,A)`.

For the field pair define

\[
 I={Q^2+P^2\over2}.                                        \tag{8}
\]

Freeze the minimum faithful positive suspension as a **[SELECTED REFERENCE
LAW]**:

\[
 H_{\rm susp}={K^2\over2M}+\nu I,qquad
 K=A-I,qquad M>0,quad\nu\ge0.                            \tag{9}
\]

The unit coefficient is the faithful `C4` representation choice: one
controller quadrant corresponds to one field quadrant in the interaction
picture. The identification of the carrier with this continuous suspension,
and the scales `M,nu`, are not derived from FTD-0972.

The certificate must prove the exact Hamilton equations

\[
 \dot\theta={K\over M},qquad \dot A=0,qquad \dot I=0,
 qquad \dot K=0,                                           \tag{10}
\]

\[
 {d\over dt}\binom QP=left({K\over M}-\nu\right)
 J\binom QP.                                                \tag{11}
\]

For `w=R(\nu t)z`, where `R(alpha)=exp(alpha J)`, prove

\[
 \dot w=\dot\theta Jw,qquad
 w(t_1)=R(\theta_1-\theta_0)w(t_0).                         \tag{12}
\]

Hence one controller quadrant gives `w_1=Jw_0`, four give identity, and
negative elapsed time supplies the exact inverse. Equation (9) is
nonnegative, autonomous, and conserves its full energy.

The canonical momentum contains the exact field-action dressing

\[
 A=K+I.                                                     \tag{13}
\]

At fixed canonical `A`, field load changes the controller rate; at fixed
mechanical `K`, the bare controller rate is unchanged. This is reciprocal
canonical bookkeeping, not free control.

## 6. Capacity, switching, and production firewall

The suspension permanently couples the modes. It transfers phase while
preserving `I`; therefore no energy or action token is exported and no
finite-reserve debit occurs during the ideal flow. Turning the coupling on or
off, changing its sign, selecting one edge, or resetting phase is a different
time-dependent transaction and requires controller work/history.

The finite FTD-0972 carrier is a stroboscopic section of (9) only after a
continuous phase/action lift is adopted. FTD-0965 establishes conditional
storage capacity for such pairs, not their formation or production law.

Neither the discrete cocycle nor the suspension derives:

- the physical identity of `(theta,A)` or `(Q,P)`;
- autonomous body/carrier formation or perturbative stability;
- the selected connection from the unchanged production tick;
- switching, replenishment, routing, positive export, or erasure;
- the dwell time per quadrant or the exact `G*` period factor;
- Born/Bell recovery or preferred-tick hiding; or
- production integration.

## 7. Frozen checks

- **G1:** hashes and all source/protocol scope markers;
- **G2:** all 256 cocycles: inverse, fiber symplecticity, and field-action
  preservation;
- **G3:** four-step holonomy, gauge transformation, invariant `m`, and
  canonical representative;
- **G4:** carrier-translation equivalence, direct-product boundary, and
  faithful-representation classification;
- **G5:** minimum time reversal, exact covariance class, and even-holonomy
  obstruction;
- **G6:** underdetermination by two symmetry-admissible couplings;
- **G7:** even-dimensional minimum controller and exact Hamilton equations;
- **G8:** closed-form interaction-picture flow, quadrant `J`, four-step
  identity, inverse, positivity, energy, and momentum dressing;
- **G9:** permanent-coupling, switching, formation, `G*`, Born, and production
  firewalls.

The enumeration is a finite exact classification, not a numerical or
near-miss search. No floating comparison is permitted.

## 8. Frozen classifier

- **Outcome A — native production coupling:** G1--G9 pass and the finite
  carrier uniquely fixes a production-formed, energy-closed field coupling.
- **Outcome B — exact cocycle classification / selected minimum suspension:**
  the discrete connection classes and minimum canonical suspension close
  exactly, but the physical identification, switching law, scales, formation,
  and production remain selected/open.
- **Outcome C — coupling obstruction:** no reversible fiber coupling or
  positive canonical suspension exists even conditionally.
- **Outcome D — invalid:** any lock, exact identity, or scope gate fails.

The expected result is Outcome B. Success licenses a mathematically complete
reference coupling, not its substrate derivation or production promotion.
