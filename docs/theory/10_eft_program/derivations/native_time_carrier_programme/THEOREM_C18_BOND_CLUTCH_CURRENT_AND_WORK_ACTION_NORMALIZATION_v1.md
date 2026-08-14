# Theorem — C18 bond clutch, current, and work-action normalization v1

**Identifier:** `FTD-0989`  
**Date:** 2026-08-12  
**Status:** `[THEOREM — MOORE-LOCAL POSITIVE BOND CLUTCH]` +
`[THEOREM — ANTISYMMETRIC C18 ENERGY CURRENT]` +
`[THEOREM — RECIPROCAL SWITCHING WORK / ZERO-STRAIN SEAM]` +
`[THEOREM — FIXED-CLUTCH SHADOW ENERGY AND INVERSE]` +
`[CORRECTION — PHYSICAL WORK ACTION IS FREQUENCY-NORMALIZED]` +
`[SELECTION — DISTRIBUTED TERNARY BOUNDARY OWNERSHIP]` +
`[OPEN — FORMATION / MODE SELECTION / PRODUCTION]`

## Result

The dense one-mode projector of FTD-0987 is not needed to isolate a finite
region of the existing common field. The exact `C18` incidence factor gives a
strictly Moore-local alternative: gate the boundary bonds themselves.

For an oriented representative of every undirected `C18` bond, let

\[
 (Bq)_b=\sqrt{a_b}(q_y-q_x),
 \qquad a_b=\begin{cases}1/9,&b\text{ face},\\
                          1/18,&b\text{ edge},\end{cases}          \tag{1}
\]

so `K=B^TB`. Give each controlled bond a ternary latch
`ell_b in {-1,0,+1}` and define

\[
 g_b=1-\ell_b^2,qquad
 G_\ell=\operatorname{diag}(g_b),qquad
 \boxed{K_\ell=B^TG_\ell B}.                                    \tag{2}
\]

Here `ell_b=0` transmits the ordinary C18 interaction, while either nonzero
sign cuts it. The square controls isolation; the sign retains the oriented
crossing record.

For fixed latches,

\[
 H_\ell(q,p)=\frac12p^Tp+rac12q^TK_\ell q                    \tag{3}
\]

is nonnegative and local. Cutting exactly the bonds crossing a finite
regional boundary gives

\[
 K_\ell=K_\Lambda\oplus K_{\Lambda^c}.                           \tag{4}
\]

Thus the region owns a conservative finite common-field subsystem. It can
contain compact normal modes because the boundary clutch has changed the
operator; this does not contradict the unchanged-infinite-C18 compact-mode
no-go of FTD-0943/0987.

The natural switching law is an **ideal zero-strain clutch**:

\[
 \boxed{q_y-q_x=0\quad\text{on every switching bond}.}            \tag{5}
\]

At (5), changing the latch costs zero work and produces no force impulse.
Away from (5), switching has a calculable nonzero cost which must be booked
reciprocally or the switch fails closed.

This closes the mathematical ownership/current law at reference scope. It
does not show that production creates the regional membrane, selects one
normal mode, stores one latch per boundary bond, or performs the reciprocal
transaction.

## Certificate of record

- Parent protocol:
  [`PREREG_C18_BOND_CLUTCH_CURRENT_AND_RECIPROCAL_SWITCHING_DISCRIMINATOR_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_C18_BOND_CLUTCH_CURRENT_AND_RECIPROCAL_SWITCHING_DISCRIMINATOR_v1.md),
  SHA-256 `B85BAAA418F0BFF2AE67678BDB1FBD25532EB1CEC9FF596F2325F8D00AE169DD`.
- Immutable parent proof:
  [`proof_c18_bond_clutch_current_and_reciprocal_switching.py`](../../../../../scripts/proofs/proof_c18_bond_clutch_current_and_reciprocal_switching.py),
  SHA-256 `FA0A0A5885612959D5AC782F8AF396A73A275840F08AC3047F1DF6A69859FAD1`.
- First execution: `72/73`; every mathematical and source gate passed. One
  verifier phrase omitted the frozen protocol's `FTD-0987's` provenance and
  Markdown delimiter.
- Repair protocol:
  [`PREREG_C18_BOND_CLUTCH_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_C18_BOND_CLUTCH_CERTIFICATE_REPAIR_v2.md),
  SHA-256 `BD43A0633CBF0BF02651D38D6FBC71E8868F0777E5E9D488BCCB0DABA56CF7F8`.
- Repair wrapper:
  [`proof_c18_bond_clutch_current_and_reciprocal_switching_v2.py`](../../../../../scripts/proofs/proof_c18_bond_clutch_current_and_reciprocal_switching_v2.py),
  SHA-256 `CE3743EA954C5DE72441841652A1BB0BF209767E8ED511D6B21A70272BB08B01`.
- Final execution: inherited `73/73`, repair integrity `11/11`,
  **Outcome B**.
- Production mutation: none.

## 1. Locality, positivity, and regional ownership

Equation (2) is symmetric and has exact quadratic form

\[
 q^TK_\ell q
 =\sum_bg_ba_b(q_y-q_x)^2
 =\|G_\ell^{1/2}Bq\|^2\ge0.                              \tag{6}
\]

Each row of `B` touches the two endpoints of one face or edge bond. Hence
`K_ell` has the same one-Moore-shell range as `K`. If a bond has one endpoint
in `Lambda` and one outside, setting its `g_b` to zero removes precisely that
cross-block rank-one incidence contribution. Cutting all and only the
boundary bonds proves equation (4).

This distinction matters:

- FTD-0987's dense projector isolates one selected vector `u` from its
  orthogonal complement but is not generically finite range.
- Equation (2) isolates a whole finite region using strictly local bonds.
- Selecting a particular eigenmode inside that region still requires its
  formation/preparation dynamics. The bond clutch does not perform a hidden
  global eigenvector projection.

The price is not another continuous pair. It is a distributed discrete
boundary ownership structure: one reversible ternary latch per controlled
boundary bond, or an explicitly demonstrated equivalent site-local encoding.
That structure is **[SELECTED]**, not present in unchanged production.

## 2. Exact continuous-time local current

Assign the local energy

\[
 h_x=\frac12p_x^2+\frac14\sum_{y\sim x}
 g_{xy}a_{xy}(q_y-q_x)^2.                                  \tag{7}
\]

Hamilton's equations from (3) give

\[
 \dot q_x=p_x,qquad
 \dot p_x=\sum_{y\sim x}g_{xy}a_{xy}(q_y-q_x).            \tag{8}
\]

Define the oriented bond current

\[
 \boxed{{\cal J}_{x\to y}=
 \frac12g_{xy}a_{xy}(q_x-q_y)(p_x+p_y).}                  \tag{9}
\]

It obeys

\[
 {\cal J}_{y\to x}=-{\cal J}_{x\to y},
 \qquad
 \dot h_x+\sum_{y\sim x}{\cal J}_{x\to y}=0.             \tag{10}
\]

Summing (10) over a region cancels every internal bond pairwise:

\[
 \boxed{\frac{dH_\Lambda}{dt}
 =-\sum_{x\in\Lambda,y\notin\Lambda}{\cal J}_{x\to y}.}  \tag{11}
\]

An engaged cut has `g_xy=0`, so its boundary current is exactly zero. An open
clutch has a completely explicit charging/replenishment current. This is the
missing local conservation statement left open by FTD-0987.

## 3. Reciprocal switching transaction

At fixed canonical state, changing `ell` to `ell'` changes the Hamiltonian by

\[
 \boxed{W_{\ell\to\ell'}=
 \frac12q^T(K_{\ell'}-K_\ell)q
 =\frac12\sum_b(g_b'-g_b)a_b(q_y-q_x)^2.}               \tag{12}
\]

Equation (12) is controller work. A reversible implementation must transfer
it to or from a local controller reserve and retain the inverse history. It
cannot be discarded as a mere change of description.

There is, however, a distinguished local seam. If every switched bond has
zero strain, then

\[
 W_{\ell\to\ell'}=0,
 \qquad (K_{\ell'}-K_\ell)q=0.                            \tag{13}
\]

The clutch can switch there without an energy jump or force impulse. When
`p_y-p_x` is nonzero, its sign supplies the directed crossing token

\[
 \sigma_b=\operatorname{sgn}(p_y-p_x),qquad
 \Theta:\sigma_b\mapsto-\sigma_b.                        \tag{14}
\]

The existing FTD-0971/0972 ternary interface can retain that orientation via
the reversible two-slot transfer

\[
 (\sigma,0)\longleftrightarrow(0,\sigma).                 \tag{15}
\]

Resetting a nonzero sign to blank without a receiving record remains
noninjective. Thus zero work does not mean zero information cost.

## 4. Exact fixed-clutch finite-tick law

For a fixed latch configuration, the production-form kick--drift map is

\[
 p'=p-hK_\ell q,
 \qquad q'=q+hp'.                                         \tag{16}
\]

It is exactly symplectic and has local inverse

\[
 q=q'-hp',qquad p=p'+hK_\ell q.                          \tag{17}
\]

Although (16) does not exactly conserve the naive continuous Hamiltonian
(3), it preserves the exact shadow Hamiltonian

\[
 \boxed{\widetilde H_{h,\ell}
 =\frac12p^Tp+rac12q^TK_\ell q
  -\frac h2p^TK_\ell q.}                                 \tag{18}
\]

Because `K_ell=B^TG_ell B`, equation (18) is itself local:

\[
 \widetilde H_{h,\ell}
 =\frac12\sum_xp_x^2+
 \sum_bg_ba_b\left[
 \frac12(q_y-q_x)^2
 -\frac h2(p_y-p_x)(q_y-q_x)\right].                     \tag{19}
\]

For an eigenvalue `lambda` of `K_ell`, its modal contribution completes the
square as

\[
 \frac12\left(p-\frac{h\lambda q}{2}\right)^2
 +\frac{\lambda}{2}\left(1-\frac{h^2\lambda}{4}\right)q^2. \tag{20}
\]

It is nonnegative for `h^2 lambda_max(K_ell)<4`, positive on the quotient by
constant-coordinate null modes. Since cutting bonds subtracts a positive
incidence square, it cannot increase `lambda_max` or worsen this bound.

The exact finite-tick switching cost is the difference of (18), not merely
equation (12). At the zero-strain seam both the coordinate and cross-term
differences vanish. This supplies a fixed-clutch inverse and exact energy
ledger—not exact conservation for the complete damped/sourced/Gauss/genesis
production tick.

## 5. Correction: the physical action normalization

FTD-0987 proved that the existing common pair has observable-amplitude norm
`Q^2+P^2=2I`. The frozen production diagnostic explicitly says that norm is
not the gradient-plus-cross wave Hamiltonian. It follows that

\[
 H+2I=\text{constant}                                    \tag{21}
\]

is an exact **amplitude-audit reference identity only**. It is not the
physical C18 work law.

After a region is cut, let a normalized nonzero mode obey

\[
 K_\Lambda u=\lambda u,qquad\lambda>0,qquad
 \omega=\sqrt\lambda,qquad Q=u^Tq,quad P=u^Tp.         \tag{22}
\]

Its correct canonical action-angle variables are

\[
 Q=\sqrt{\frac{2I}{\omega}}\cos\theta,qquad
 P=-\sqrt{2\omega I}\sin\theta,                         \tag{23}
\]

with

\[
 dQ\wedge dP=d\theta\wedge dI,qquad
 \boxed{H_u=\frac12(P^2+\omega^2Q^2)=\omega I.}          \tag{24}
\]

Therefore the physical seam debit is

\[
 \boxed{I'=I+\frac{H(z)-H(z')}{\omega}},
 \qquad H(z')+\omega I'=H(z)+\omega I.                  \tag{25}
\]

This is the successor normalization of the work law. A zero mode has no
regular oscillator action-angle chart. Neither the finite region nor the
clutch uniquely selects `u`, `omega`, an absolute energy scale, or `G*`.

## 6. Epistemic disposition

Established:

- **[THEOREM]** the exact C18 incidence channels give a positive Moore-local
  bond clutch and exact regional block isolation;
- **[THEOREM]** the open bonds carry the antisymmetric current (9), and cut
  bonds carry zero current;
- **[THEOREM]** switching work is equation (12), with a zero-work,
  zero-impulse local seam at zero strain;
- **[THEOREM]** fixed-gate kick--drift is symplectic, invertible, and exactly
  conserves the local shadow Hamiltonian (18); and
- **[CORRECTION]** a true isolated oscillator obeys `H_u=omega I`, not the
  diagnostic `H=2I` unless `omega=2` is separately established.

Selected but not derived in production:

- deployment and autonomous control of a ternary latch on every active
  regional boundary bond;
- identification of those latches with an existing substrate record rather
  than adopted bond memory;
- the particular regional membrane and its owned positive-frequency mode;
  and
- the controller reserve/history implementation for off-seam switches.

Still open:

- autonomous formation, persistence, deformation, and motion of the
  boundary clutch;
- collision, branching, backpressure, routing, charging, and replenishment;
- selection and phase-complete preparation of one stable regional mode;
- exact coupled switching substeps and complete environment-inclusive energy
  closure;
- repeated finite-tick perturbation recovery and CPU/CUDA parity; and
- any physical `G*`, Born, Bell, mass, selector-energy, Lorentz-hiding, or
  completeness claim.

No production integration follows.

