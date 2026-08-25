# C18/FCC Gaussian-current collision and Maxwell mode price v1

**Date:** 2026-08-23  
**Status:** **[THEOREM — EXACT TARGET-FREE REVERSIBLE FCC COLLISION]** +
**[THEOREM — COMPLETE ADDITIVE-INVARIANT CLASSIFICATION]** +
**[THEOREM — EXACT PRODUCT-REFERENCE ZERO-WAVEVECTOR KERNEL]** +
**[THEOREM, CONDITIONAL — MAXWELL PHASE-SPACE MODE COUNT]** +
**[BOUNDARY — NO FINITE-$k$ POLE, NATIVE GAUSS/GAUGE LAW, TENSOR PROTECTION,
SC EXCHANGE, CONSERVATIVE SOURCE WORK, OR ALPHA MEASUREMENT]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_c18_fcc_gaussian_current_collision.py](../../../../../scripts/proofs/proof_c18_fcc_gaussian_current_collision.py)
performs 14,650 exact checks. It enumerates the complete registered collision
domain, verifies the local permutation, inverse period, cubic and C4
covariance, classifies every additive invariant by exact rank, derives the
integer product-reference tangent matrix and its exact characteristic
polynomial, checks the actualization-source normalization, and verifies the
conditional transverse mode count. No physical constant, fit, numerical
eigensolver, or target spectrum is used.

---

## 1. Why the previous collision failed

The earlier
[two-record collision kernel](../gravity_cosmology/THEOREM_C18_TWO_RECORD_LINEARIZED_KERNEL_AND_TENSOR_BOUNDARY_v1.md)
preserved four separate C4 phase counts and phase-blind directed momentum. Its
seven collision invariants were therefore

\[
 (N_0,N_1,N_2,N_3;P_x,P_y,P_z).                    \tag{1}
\]

That is the wrong seven-dimensional space for a phase-complete vector field.
The three nontrivial C4-weighted vector characters all relaxed. Conversely,
requiring all four phase-resolved momenta

\[
 P_{p,j}=\sum_{d}n_{d,p}d_j                         \tag{2}
\]

would protect twelve vector components: the blind, alternating, real, and
imaginary C4 triplets. That overprotects two unwanted vector species.

The exact middle target is not four phase momenta. It is one
Gaussian-integer vector current.

---

## 2. Native Gaussian current

On the twelve directed FCC channels, attach a C4 phase $p\in\mathbb Z_4$ and
define the one-record current

\[
 g(d,p)=i^p d\in\mathbb Z[i]^3.                    \tag{3}
\]

For a local two-record state $z=\{(d,p),(e,q)\}$, define

\[
 \mathcal C(z)=g(d,p)+g(e,q)=U(z)+iV(z),            \tag{4}
\]

with $U,V\in\mathbb Z^3$. A global C4 advance acts exactly as

\[
 (U,V)\longmapsto(-V,U).                            \tag{5}
\]

Thus $(U,V)$ is a native six-real-dimensional vector doublet with complex
structure $J^2=-I$; no continuous phase has been added.

Two elementary maps preserve equation (3):

\[
 A(d,p)=(-d,p+2),                                   \tag{6}
\]

which preserves each record's $i^p d$ separately, and, on a zero-current
equal-phase antipodal pair,

\[
 B:\{(d,p),(-d,p)\}\mapsto
 \{(d,p+1),(-d,p+1)\}.                              \tag{7}
\]

Equation (7) preserves the zero sum while advancing its internal clock.

---

## 3. One finite reversible collision

Let $\mathcal C_2$ be the previously certified phase-complete FCC
momentum-doubleton scatter. Restrict the active domain to local states with:

1. exactly two occupied, distinct FCC channels; and
2. the same C4 phase on both records.

SC records, if present in the larger C18 carrier, are spectators and do not
enter the gate. Define

\[
 \boxed{
 F(z)=
 \begin{cases}
 Bz,&\mathcal C(z)=0,\\
 (A\otimes A)\,\mathcal C_2z,&\mathcal C(z)\ne0.
 \end{cases}}                                      \tag{8}
\]

All local states outside the registered domain are fixed. The branch test in
equation (8) reads only exact local current, occupancy, and phase equality. It
does not read a physical target, probability, or coupling.

There are exactly 264 active states. Their 85 current sectors have histogram

\[
 \{2^{48},4^{36},24^1\},                            \tag{9}
\]

where the exponent counts sectors, not powers. The zero-current sector is the
unique 24-state sector. Equation (8) is a permutation with cycle census

\[
 120\text{ two-cycles}+6\text{ four-cycles}.        \tag{10}
\]

Consequently $F^4=1$ on the complete active domain. The six four-cycles are
the six antipodal FCC lines carrying the four C4 phases.

The certificate verifies exactly that

\[
 \mathcal C(Fz)=\mathcal C(z),                      \tag{11}
\]

and that $F$ commutes with all 48 signed cubic transformations and every
global C4 phase shift. Equation (8) is therefore a local, reversible,
$O_h\times C_4$-covariant collision witness.

It is a **reference action candidate**, not a selected production law. In
particular, it does not conserve phase-blind mechanical momentum. A complete
field-plus-matter momentum ledger remains required.

---

## 4. Complete additive-invariant theorem

Let $h(d,p)$ be any real one-record quantity. Call it an additive invariant
of equation (8) when

\[
 h(d,p)+h(e,p)=h(d',p')+h(e',p')                   \tag{12}
\]

for every transition $\{(d,p),(e,p)\}\mapsto
\{(d',p'),(e',p')\}$ generated by $F$.

There are 48 one-record FCC phase states. The exact 264-by-48 transition
matrix has

\[
 \operatorname{rank}T=41,
 \qquad \dim\ker T=7.                              \tag{13}
\]

Seven explicit independent invariant rows are

\[
 1,qquad u_p d_x,u_p d_y,u_p d_z,qquad
 v_p d_x,v_p d_y,v_p d_z,                           \tag{14}
\]

where $i^p=u_p+iv_p$. Equations (13)--(14) prove the complete classification:

\[
 \boxed{
 \text{every additive invariant is a linear combination of record number,
 }U,\text{ and }V.}                                \tag{15}
\]

This is the key improvement over the earlier collision. The protected vector
space is exactly one complex triplet. The phase-blind and alternating
triplets are not protected, and the four individual phase counts are not
protected.

The comparison is exact:

| Conservation choice | Protected vector components |
|---|---:|
| phase-blind momentum only | 3 |
| all four phase-resolved momenta | 12 |
| Gaussian current $\mathcal C=U+iV$ | **6** |

---

## 5. Exact product-reference kernel

Use the independent uniform five-state measure

\[
 \{0,1,i,-1,-i\}
\]

on each of the twelve FCC channels. The active collision configurations have
exact weight

\[
 {264\over5^{12}}.                                  \tag{16}
\]

In the 48-dimensional normalized occupied-phase tangent chart, the marginal
Jacobian is

\[
 \boxed{DF(p^*)=I_{48}+{1\over5^{11}}N,}            \tag{17}
\]

where $N$ is an exact integer matrix of rank 41. Its characteristic
polynomial is

\[
\begin{aligned}
 \chi_N(x)={}&x^7(x+4)(x+8)^3(x+10)^2
 (x^2+20x+104)^3\\
 &\times(x+12)^9(x^2+24x+148)^2
 (x+20)^3(x+30)^2\\
 &\times(x+40)^6(x^2+84x+1768)(x+100)^3.           \tag{18}
\end{aligned}
\]

Equivalently, the three conjugate-pair families are

\[
 -10\pm2i,qquad -12\pm2i,qquad -42\pm2i,         \tag{19}
\]

with multiplicities three, two, and one respectively. Every nonzero root of
equation (18) gives a Jacobian eigenvalue strictly inside the unit disk. The
seven unit eigenmodes are exactly equation (15). The complex roots give
chiral damped transients, not a propagating photon claim.

This closes the zero-wavevector protection problem for the FCC vector
carrier: the desired doublet is no longer relaxing, and no other additive
vector character remains gapless.

It does **not** establish a finite-$k$ pole. Streaming plus collision must
still be linearized as a Bloch operator, and its transverse eigenvalues must
show a positive, gapless, isotropic dispersion rather than diffusion,
ballistic channel memory, or a gapped C4 recurrence.

---

## 6. Exact source-to-carrier alignment

All FCC directions have the same Euclidean length $\sqrt2$. Therefore the
normalized current is unambiguously

\[
 \widehat{\mathcal C}={\mathcal C\over\sqrt2}.      \tag{20}
\]

For an FCC line with raw direction $d$, the
[actualization source vertex](../common_action_mechanics_reciprocity/THEOREM_C18_ACTUALIZATION_SHARED_MOMENT_SOURCE_VERTEX_v1.md)
injects

\[
 \boxed{
 \Delta(R_u+iR_v)
 ={\epsilon i^p\over9}{d\over\sqrt2}
 ={1\over9}\,\epsilon\widehat g(d,p).}             \tag{21}
\]

The certificate checks equation (21) for all six FCC lines, four phases, and
two orientations through the actualization macro itself. This is the first
exact normalization match between the shared manifestation vertex and a
protected collision current.

Equation (21) is not yet a conservative coupling. The reserve-side token is
not presently assigned the compensating directed current or common work
debit. Thus actualization can source the field representation, but the one
action still has to close the source-plus-field ledger and derive the response
to that source.

---

## 7. Conditional Maxwell mode price

At a nonzero infrared wavevector $k$, the protected pair $(U,V)$ has six real
components. If the finite action derives either

\[
 k\cdot U=0,qquad k\cdot V=0,                      \tag{22}
\]

or an equivalent one-constraint/one-gauge canonical reduction, the exact
rank count is

\[
 6-2=4.                                             \tag{23}
\]

Four real phase-space dimensions are two transverse polarizations and their
conjugate partners. The certificate verifies rank two for equation (22) on
every nonzero integer wavevector in the symmetric box $[-2,2]^3$; the general
rank proof follows immediately because $k\ne0$.

This is a **conditional kinematic count**, not Maxwell dynamics. Equation
(8) has not generated equation (22), a Gauss source, a gauge redundancy, a
curl/cotangent kinetic operator, a light cone, or a $1/k^2$ static Green
function.

---

## 8. Gravity and shell boundary

The six unoriented FCC line dyads have exact symmetric-tensor rank six, so the
same FCC carrier retains enough local type for the common tensor doublet.
However the complete invariant theorem also proves that no nonzero
C4-weighted FCC tensor moment lies in equation (15): the twelve tensor rows
have rank twelve and zero intersection with the seven invariant rows.

Therefore equation (8) repairs the vector sector only:

\[
 \boxed{
 \text{Gaussian vector current protected; tensor doublet not protected.}}
                                                               \tag{24}
\]

Gravity still requires an action-derived constraint symmetry or a different
tensor transport mechanism. The exact TT reduction remains only conditional,
and no lensing follows from this collision.

The SC shell is also a spectator. Extending equation (8) by conserving a
Euclidean-normalized mixed SC/FCC current is not automatic: the relative
$1:\sqrt2$ shell normalization can split exact finite-count conservation into
independent shell currents. Shell exchange and the physical spatial metric
must be derived, not chosen to improve a spectrum.

---

## 9. Consequence for the one-action program

The exact chain now contains a nontrivial common seam:

\[
 \begin{array}{c}
 \text{same C4 phase compatibility}\\
 \downarrow\\
 \text{reversible FCC collision conserving }U+iV\\
 \updownarrow\\
 \text{actualization injects one normalized current quantum}/9.
 \end{array}                                        \tag{25}
\]

This is materially closer to one action than juxtaposing an electromagnetic
term with a manifestation term. The same finite phase record and orientation
now define both the actualization source and the protected field current.

The result does not complete the objective. The collision is still separate
from:

- autonomous reserve preparation and conservative manifestation work;
- formation and stability of the recurrent proto-matter clock;
- the physical Born tape's preparation, routing, and general-amplitude limit;
- tensor constraints, a tensor pole, static gravity, and lensing; and
- a blind source-response measurement of
  $\alpha_{\rm native}=g_{\rm eff}^2/(4\pi\hbar_{\rm eff}c_{\rm eff})$.

---

## 10. Next locked gate

Construct the exact finite-wavevector FCC streaming-collision Bloch operator
for equation (8), before adding any coupling coefficient. The registered pass
conditions are:

1. the four conditionally transverse current modes are gapless and
   propagating, not merely diffusive or ballistic channel remnants;
2. their small-$k$ cone is cubically isotropic at leading order;
3. a local constraint/quotient removes exactly the two longitudinal modes;
4. equation (21) enters an exact reserve-plus-field continuity/work ledger;
5. the response produces both a static Green kernel and transverse radiation;
   and
6. no parameter is selected using the fine-structure root.

A failure of items 1--3 closes equation (8) as a Maxwell carrier despite its
exact zero-mode conservation. A pass would authorize the first blind native
source-response coupling measurement; it would not authorize identifying the
result with the master root until after measurement.

**Executed successor:** the
[Gaussian-current Bloch boundary](THEOREM_C18_FCC_GAUSSIAN_CURRENT_BLOCH_DIFFUSION_BOUNDARY_v1.md)
closes items 1--3 negative for phase-independent one-hop FCC streaming. The
exact first-order reduced generator vanishes and the transverse modes begin
with damped chiral $O(k^2)$ dispersion. The zero-mode conservation and
source-alignment theorems remain valid, but this collision/streaming
composition is not a Maxwell carrier. The next admissible repair is an
oriented Hodge/curl pair or a genuine cotangent/Jordan structure, not a tuned
relaxation coefficient.
