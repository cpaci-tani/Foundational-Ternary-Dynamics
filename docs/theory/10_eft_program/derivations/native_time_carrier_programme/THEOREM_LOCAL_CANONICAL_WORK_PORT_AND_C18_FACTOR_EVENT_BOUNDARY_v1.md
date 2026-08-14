# Theorem — Local canonical work port and C18 factor/event boundary v1

**Identifier:** `FTD-0982`  
**Date:** 2026-08-12  
**Status:** `[THEOREM — EXACT C18 INCIDENCE/DIRAC FACTOR]` +
`[THEOREM — FACTOR DOES NOT LOCALIZE THE ONE-EVENT INVERSE]` +
`[THEOREM — MINIMUM LOCAL CANONICAL WORK-PORT LIFT]` +
`[THEOREM — EXACT FOUR-STROKE FIELD/RESERVE RECOVERY]` +
`[BOUNDARY — FINITE READY-DOMAIN RESERVE]` +
`[SELECTION — REFERENCE SEAM FAMILY]` +
`[OPEN — NATIVE FORMATION / PRODUCTION / HIDING]`

## Result

The FTD-0980 implementation fork is now discriminated.

1. The production `C18` scalar stiffness has an exact finite-range
   nine-channel incidence factor and hence a finite-range self-adjoint
   multicomponent Dirac-type block factor.
2. That factor does **not** make the energy-compatible one-event quarter turn
   finite-range. The event still requires the inverse factor; the massless
   factor is singular and the massive factor has no finite-range inverse.
3. One complete canonical work pair `(theta,I_R)` per independently gated
   batch is both necessary, by FTD-0928's phase-completeness lower bound, and
   sufficient for an exact finite-range symplectic lift of the site-local
   root.
4. At the registered crossing the work action changes by exactly the negative
   field-energy change. After four same-orientation strokes, both the field
   and the work reserve return exactly.

Thus the minimum **reference** implementation of the instantaneous local
clutch is the work-port branch, not factor hardware alone:

\[
 \boxed{
 z'=R_\sigma z,\qquad
 I_R'=I_R+H(z)-H(R_\sigma z),
 \qquad H(z')+I_R'=H(z)+I_R.}
                                                               \tag{1}
\]

Equation (1) is canonical only as the registered phase-dependent lift derived
below. Written without the phase reaction, its last line would be a passive
ledger and would fail symplecticity.

This theorem selects no production field, adds no engine type, and derives no
fermion, spinor, Hilbert space, `G*` mechanism, Born law, or physical mass.

## Certificate of record

- Parent protocol:
  [`PREREG_LOCAL_WORK_PORT_VERSUS_MULTICOMPONENT_FACTOR_DISCRIMINATOR_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_LOCAL_WORK_PORT_VERSUS_MULTICOMPONENT_FACTOR_DISCRIMINATOR_v1.md),
  SHA-256 `7CF3DC6239200CF1B773ADEC0633F0B30CD5735C7FF8BDA1360F730888C5EDE3`.
- Immutable parent proof:
  [`proof_local_work_port_multicomponent_factor_discriminator.py`](../../../../../scripts/proofs/proof_local_work_port_multicomponent_factor_discriminator.py),
  SHA-256 `BDD16E3D4AB8BF0E0D4C72E5520638AB712D64E113725145B27F919B620F0C69`.
- First execution: `76/79`; three verifier-only predicates failed while all
  substantive identities passed.
- v2 repair protocol:
  [`PREREG_LOCAL_WORK_PORT_FACTOR_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_LOCAL_WORK_PORT_FACTOR_CERTIFICATE_REPAIR_v2.md),
  SHA-256 `4FD4AAE506BF96B890C020FEB3E798F12558AC271A8EEACE5D26722FDA8BCD9E`.
- v2 wrapper:
  [`proof_local_work_port_multicomponent_factor_discriminator_repaired.py`](../../../../../scripts/proofs/proof_local_work_port_multicomponent_factor_discriminator_repaired.py),
  SHA-256 `39F0287B56EB4FC62BF04CEB0A40FFFCD8B3B06455229068321812C7CA984B09`;
  execution `78/79`, with one Markdown line-wrap marker still false.
- Final repair protocol:
  [`PREREG_LOCAL_WORK_PORT_FACTOR_CERTIFICATE_REPAIR_v3.md`](../../preregistrations/native_time_carrier_programme/PREREG_LOCAL_WORK_PORT_FACTOR_CERTIFICATE_REPAIR_v3.md),
  SHA-256 `6BE59B135CEA66F04A2F659E5A177AF8A4BD53AD0DDF592A2C8A173ACE946FB2`.
- Final wrapper:
  [`proof_local_work_port_multicomponent_factor_discriminator_repaired_v3.py`](../../../../../scripts/proofs/proof_local_work_port_multicomponent_factor_discriminator_repaired_v3.py),
  SHA-256 `CB4B7F076779C7870EE48DFBC69F6918C0136906CF14F3DF268B7084BB4F1353`.
- Final execution: inherited `79/79` plus all repair-integrity gates,
  **Outcome B**.
- Production mutation: none.

## 1. Exact local factor of the C18 stiffness

Choose one representative from each undirected `C18` bond:

\[
 F_+=\{(1,0,0),(0,1,0),(0,0,1)\},                     \tag{2}
\]

\[
 E_+=\{(1,1,0),(1,-1,0),(1,0,1),(1,0,-1),
       (0,1,1),(0,1,-1)\}.                              \tag{3}
\]

Set `a_r=1/9` on `F_+` and `a_r=1/18` on `E_+`, and define

\[
 (Bq)_{x,r}=\sqrt{a_r}(q_{x+r}-q_x).                    \tag{4}
\]

Then

\[
 B^*B=K,
\qquad
 K(z)=\sum_ra_r(2-z^r-z^{-r}).                          \tag{5}
\]

The exact coefficients are

\[
 K(0\hbox{-shift})={4\over3},\qquad
 K(\pm F)=-{1\over9},\qquad
 K(\pm E)=-{1\over18},                                 \tag{6}
\]

which reproduce FTD-0943's frozen positive production stiffness
coefficient by coefficient. This is a genuine local sum-of-squares factor,
not a numerical decomposition.

The vacuum Hessian is

\[
 \operatorname{Hess}K(0)={2\over3}I_3.                 \tag{7}
\]

If an analytic real factor used `m` channels, its Hessian would have the
form `2GG^T`, with rank at most `m`. Equation (7) therefore proves only

\[
 \boxed{m\ge3.}                                         \tag{8}
\]

The nine-channel incidence witness is convenient and cubic, but it is not
proved channel-minimal.

## 2. The exact block factor

For `K_mu=K+mu^2I`, define

\[
 {\cal D}_\mu=
 \begin{pmatrix}\mu I&B^*\\B&-\mu I_9\end{pmatrix}.   \tag{9}
\]

It is finite-range and self-adjoint, and direct multiplication gives

\[
 {\cal D}_\mu^2=
 \begin{pmatrix}
 K_\mu&0\\
 0&\mu^2I_9+BB^*
 \end{pmatrix}.                                        \tag{10}
\]

The off-diagonal blocks cancel because `mu` is scalar. Equation (10) proves
that the scalar stiffness can be embedded in a local first-order factor
representation. It does not identify the nine bond channels with new
physical matter or derive a Dirac equation.

## 3. Why a factor is not yet the event

For a self-adjoint invertible factor `D`, the energy-compatible oriented root
on coordinate/momentum phase space is

\[
 J_{D,\sigma}=
 \begin{pmatrix}0&-\sigma D^{-1}\\
                 \sigma D&0\end{pmatrix}.              \tag{11}
\]

The local factor in equation (9) does not remove `D^{-1}`.

For `mu=0`, the vacuum mode makes `K` and hence the scalar factor sector
singular. For `mu>0`, restrict to one axis:

\[
 k_\mu(z)=\mu^2+c^2(2-z-z^{-1}),\qquad c\ne0.          \tag{12}
\]

The units of the Laurent ring are exactly monomials `c z^n`. Equation (12)
has two nonzero extremal monomials and is not a unit, so `k_mu^{-1}` is not a
Laurent polynomial. If `D_mu^{-1}` were finite-range, then
`D_mu^{-2}` would be finite-range; equation (10) would make its scalar block
`K_mu^{-1}`, a contradiction. Therefore

\[
 \boxed{{\cal D}_\mu\text{ finite-range}
 \not\Rightarrow {\cal D}_\mu^{-1}\text{ finite-range}.} \tag{13}
\]

A local factor can be a first-order generator. It cannot, without more
structure, be the exact instantaneous root (11). Declaring the factor
variables primitive, selecting a constraint surface, or replacing one event
by multi-tick first-order propagation remains a separately priced branch.

## 4. Minimum local canonical work port

Write

\[
 z=(q,p)^T,qquad
 \Omega=\begin{pmatrix}0&I\\-I&0\end{pmatrix},qquad
 G=\begin{pmatrix}K&0\\0&I\end{pmatrix},               \tag{14}
\]

so `H(z)=z^TGz/2`. The site-local root is

\[
 R_\sigma=
 \begin{pmatrix}0&-\sigma I/\kappa\\
                 \sigma\kappa I&0\end{pmatrix}.        \tag{15}
\]

Its exact before-minus-after energy matrix is

\[
 B_0=G-R_\sigma^TGR_\sigma
 =\begin{pmatrix}
 K-\kappa^2I&0\\0&I-K/\kappa^2
 \end{pmatrix}.                                        \tag{16}
\]

To turn that debit into a canonical transaction, add one work pair
`(theta,I_R)` and set

\[
 B_q=K-\kappa^2I,qquad B_p=I-K/\kappa^2,               \tag{17}
\]

\[
 Q_s=\begin{pmatrix}I&-sB_p\\0&I\end{pmatrix},qquad
 P_s=\begin{pmatrix}I&0\\sB_q&I\end{pmatrix},
\quad S_s=Q_sP_s,
\quad R_s=R_\sigma S_s,                                \tag{18}
\]

where `s=theta-theta_*`. Both shears and their inverse are finite-range and
symplectic. At the crossing,

\[
 S_0=I,qquad R_0=R_\sigma,qquad
 R_s^T\Omega\partial_sR_s\big|_0=B_0.                 \tag{19}
\]

For general `s`, define the symmetric local matrix

\[
 B_s=R_s^T\Omega\partial_sR_s.                          \tag{20}
\]

The complete seam event is

\[
 \boxed{
 z'=R_sz,qquad
 \theta'=\theta,qquad
 I_R'=I_R+{1\over2}z^TB_sz.}                            \tag{21}
\]

Its Jacobian preserves

\[
 \Omega+d\theta\wedge dI_R.                            \tag{22}
\]

The cross term created by the phase dependence of `R_s` is exactly cancelled
by the action reaction in equation (21). This is what a passive scalar debit
lacks.

At `s=0`, equation (21) reduces to equation (1), because

\[
 {1\over2}z^TB_0z=H(z)-H(R_\sigma z).                  \tag{23}
\]

The inverse is finite-range: first apply
`S_s^{-1}=P_s^{-1}Q_s^{-1}`, then `R_sigma^{-1}`, and subtract the same
action generating function evaluated on the recovered state.

FTD-0928 proved that a scalar energy account is not phase-complete and that
at least one canonical pair is necessary per independently formed phase
plane. Equation (21) uses exactly one pair. It is therefore minimum in that
registered phase-completeness sense.

## 5. Four-stroke recovery

At repeated seam crossings with the same retained orientation,

\[
 z_m=R_\sigma^m z_0,qquad
 I_{R,m}=I_{R,0}+H(z_0)-H(z_m).                         \tag{24}
\]

The action increments telescope. Since `R_sigma^4=I`,

\[
 \boxed{z_4=z_0,qquad I_{R,4}=I_{R,0}.}                \tag{25}
\]

This is the stable recursive feature: the work port can be catalytic over a
complete four-stroke cycle. It exchanges energy on intermediate strokes but
has no net debit after exact return.

The work action does not record handedness. The ternary sign/history latch
from FTD-0980 remains separately necessary so that `R_+` and `R_-` are not
identified.

## 6. Positivity and the ready-domain boundary

The work action must stay inside its positive reserve domain. The exact
eligibility condition is

\[
 I_{R,0}\ge
 \max_{m=1,2,3,4}\{H(z_m)-H(z_0),0\}.                  \tag{26}
\]

For `K` not equal to `kappa^2I`, the positive work requirement grows
quadratically with field amplitude in at least one of the `q` or `p`
directions. Consequently no fixed finite reserve covers an unbounded state
space. A physical implementation must:

1. preregister a bounded compliance shell;
2. measure only local state needed by equation (26);
3. fail closed when reserve is insufficient; and
4. retain the work and orientation state needed by the inverse.

This is a capacity boundary, not an energy-conservation failure.

## 7. Epistemic disposition

Established:

- **[THEOREM]** the exact production `C18` stiffness is a nine-channel local
  incidence norm `B^*B`;
- **[THEOREM]** every analytic sum-of-squares factor needs at least three
  real channels near the vacuum, while nine-channel minimality is not proved;
- **[THEOREM]** the block operator (9) is a finite-range self-adjoint factor;
- **[THEOREM]** that factor has no finite-range inverse on either the massless
  or massive registered branch and therefore does not by itself localize the
  exact one-event root;
- **[THEOREM]** equation (21) is an exact finite-range extended symplectic
  event;
- **[THEOREM]** one complete work pair meets the inherited minimum and books
  the exact field-energy change;
- **[THEOREM]** field and reserve recover after four strokes; and
- **[BOUNDARY]** finite positive reserve requires a bounded ready domain.

Selected but not derived physically:

- the seam family (18);
- the identification of `(theta,I_R)` as the relevant local work port; and
- the use of the instantaneous local root rather than multistep factor
  propagation.

Still open:

- derive the work pair from existing substrate variables or price it as an
  adopted type;
- determine whether one global clock pair or many local batch pairs can obey
  Moore causality without double booking;
- form, charge, replenish, route, and recycle the reserve locally;
- combine the work port with the retained ternary orientation latch and the
  FTD-0977 single physical clock momentum;
- prove finite-tick stability, concurrency, perturbation recovery, causal
  CPU/GPU parity, and operational hiding; and
- establish any physical `G*`, Born, Bell, mass, or clock identification.

No whole-framework completeness claim follows.
