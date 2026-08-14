# Neutral-body Krylov frame and handed complex structure

**Identifiers:** `FTD-0966`, `FTD-0967`, `FTD-0968`, `FTD-0969`  
**Status:** `[THEOREM — CONDITIONAL REGIONAL SNAPSHOT FRAME]` +
`[THEOREM — MINIMUM NEUTRAL SUPPORT CARDINALITY FOUR]` +
`[OPEN — FORMATION/MOVING-FRAME CONNECTION/PRODUCTION]`  
**Date:** 2026-08-11

## 1. Result

A finite neutral actual ternary body can supply the spatial orientation datum
that a site-local linear cubic chart cannot.

For a regular body, its neutral dipole and centered occupancy covariance
define an exact ordered polar frame and a handed transverse complex
structure. The construction is translation invariant and covariant under all
48 proper and improper signed-cubic transformations. Four occupied ternary
sites are both necessary and sufficient; an exact witness fits within one
Moore neighborhood.

This closes only the fixed-snapshot chart problem left by FTD-0965. It does
not derive a dynamically maintained frame, its canonical connection and
reaction, a time arrow, autonomous formation, or a production gearbox.

## 2. Exact construction

Let a finite support `S` contain only nonzero actual states
`s_x in {-1,+1}` and obey neutrality `sum_x s_x=0`. In an unambiguous
minimum-image chart define

\[
 X={1\over N}\sum_x r_x,qquad
 d=\sum_xs_x(r_x-X),qquad
 C={1\over N}\sum_x(r_x-X)(r_x-X)^T.                 \tag{1}
\]

The moment-Krylov determinant

\[
 \kappa=\det[d,Cd,C^2d]                              \tag{2}
\]

is a spatial pseudoscalar. On the regular stratum `kappa != 0`, put

\[
 \chi=\operatorname{sign}(\kappa),\qquad e_1={d\over|d|},
                                                               \tag{3}
\]

\[
 t=(I-e_1e_1^T)Cd,qquad e_2={t\over|t|},qquad
 e_3=\chi(e_1\times e_2).                           \tag{4}
\]

Then `(e1,e2,e3)` is an orthonormal polar triad with

\[
 \det[e_1,e_2,e_3]=\chi.                            \tag{5}
\]

The corresponding transverse complex structure is

\[
 \mathcal I_Fv=\chi(e_1\times v),qquad
 \mathcal I_F^T=-\mathcal I_F,qquad
 \mathcal I_F^2=-(I-e_1e_1^T).                      \tag{6}
\]

Thus a real, orientation-sensitive `i` is an antisymmetric quarter-turn on
the two-dimensional plane transverse to the body's dipole. The longitudinal
axis remains its kernel; this is a transverse complex structure, not a scalar
complex number acting on all three spatial dimensions.

## 3. Full signed-cubic covariance

For `r'_x=Qr_x+a`, where `Q` is any signed permutation matrix,

\[
 d'=Qd,qquad C'=QCQ^T,qquad
 \kappa'=\det(Q)\kappa,qquad \chi'=\det(Q)\chi.      \tag{7}
\]

The cross-product pseudovector factor is exactly cancelled by the
pseudoscalar `chi`, giving

\[
 e'_j=Qe_j,qquad
 \mathcal I'_F=Q\mathcal I_FQ^T.                    \tag{8}
\]

This is why the construction succeeds where a site-local linear scalar does
not: the regional body's higher moments provide both a second nonparallel ray
and the missing handedness datum.

## 4. Minimum theorem and exact Moore-local witness

Neutrality of nonzero `+/-1` states forces `N` to be even. For `N=2`, the
centered covariance is `C=dd^T/4`; consequently `d`, `Cd`, and `C^2d` are
collinear and `kappa=0`. One- and three-site neutral supports are impossible,
so `N>=4` is necessary.

Four sites suffice. For

\[
\begin{array}{c|cccc}
x&(0,0,0)&(1,0,0)&(0,1,0)&(1,1,1)\\ \hline
s_x&+1&+1&-1&-1,
\end{array}                                                   \tag{9}
\]

the exact moments are

\[
 X=(1/2,1/2,1/4),\qquad d=(0,-2,-1),                         \tag{10}
\]

\[
 C=\begin{pmatrix}
 1/4&0&1/8\\
 0&1/4&1/8\\
 1/8&1/8&3/16
 \end{pmatrix},\qquad \kappa=-1/256.                        \tag{11}
\]

The frame is

\[
 e_1={1\over\sqrt5}(0,-2,-1),\quad
 e_2={1\over3\sqrt5}(-5,2,-4),\quad
 e_3={1\over3}(-2,-1,2),\quad\chi=-1.                      \tag{12}
\]

All four sites lie within the unit cube, hence within one Moore-local region.
This is an exact existence witness, not a claim that the production dynamics
forms or stabilizes it.

## 5. Fixed-snapshot canonical chart

On a fixed actual-record stratum, rotate every polar field coordinate and
its conjugate momentum by the same orthogonal frame matrix. The resulting
block transform has full rank, determinant one, and preserves the symplectic
form. Applied independently to the left and right field pairs, it provides
the regional provenance missing from FTD-0965 without adding a new snapshot
storage pair.

This statement is conditional on holding the actual body, and therefore the
frame, fixed during the coordinate transformation. A state-dependent moving
frame adds connection terms. Ignoring them would omit reciprocal reaction and
work and would not be a canonical autonomous production update.

## 6. Spatial handedness is not temporal direction

The derived `chi` is a spatial pseudoscalar but is time-even. Clockwise and
counterclockwise traversal require a separate time-odd crossing current
`eta`. The alternatives

\[
 \mathcal I_+=+\mathcal I_F,qquad
 \mathcal I_-=-\mathcal I_F                              \tag{13}
\]

are distinct, while

\[
 \mathcal I_+^2=\mathcal I_-^2=-(I-e_1e_1^T).             \tag{14}
\]

Thus the symmetric square loses traversal orientation exactly as anticipated:
the body can distinguish spatial chirality, but it cannot infer the arrow of
time from a single record.

## 7. Degeneracy boundary

For the one-parameter family obtained by replacing the fourth witness point
with `(1,1,u)`, exact algebra gives

\[
 \kappa(u)=-{u^5\over256}.                              \tag{15}
\]

The frame becomes singular at the coplanar configuration `u=0`, and its
handedness reverses across that stratum. No continuous global frame follows
from equations (1)--(6); a physical moving-frame model must specify how it
crosses, avoids, or records this degeneracy.

## 8. Certificate and immutable repair history

- FTD-0966 protocol SHA-256:
  `F97713AE79015805D01E292E03FFF5EA18A85B515DC317251F83E9D17153B23C`;
- immutable parent proof SHA-256:
  `794B92828417A2264CAA3B75A6CF678E777D5D65D931DDAC8ECB24E8589F7C58`;
- first execution was terminated without a verdict after pathological generic
  symbolic expansion;
- FTD-0967 repair protocol SHA-256:
  `8D754A510DDF2399DF9243E0F9B8FAEDDF3EEAE8646D0EF5BC2A617DCBB7DA9F`;
- immutable FTD-0967 wrapper SHA-256:
  `BF416D09B3A89A6C93863D40DE5D2F8E364443673FC363EEDAA6284EF266734F`;
- first FTD-0967 execution: inherited `72/75`, repair `16/19`, Outcome D,
  solely on malformed two-site coordinate symbols;
- FTD-0968 repair protocol SHA-256:
  `55DB0E19370B743199E40ADF863DC4E9B90DB93A5FDC5196DB6BDCCC5B061122`;
- immutable FTD-0968 wrapper SHA-256:
  `555FB4C627D585E01D3F7BB9E5E4F4F5A13E4FA95E4EB309217746F4BF08D4CF`;
- first FTD-0968 execution: inherited `75/75`, nested repair `19/19`, own
  `19/20`, Outcome D solely on a verifier outcome-marker mismatch;
- FTD-0969 repair protocol SHA-256:
  `A44ADE36E7778BD1599895F86F08FE220321B7A5449EF73FC70BCCBA24BD077E`;
- FTD-0969 wrapper SHA-256:
  `4D5604F61D1DA7941A662A36B01DDF968DADB408F8798822EC44BF1E8CEBC286`;
- final chain: mathematical `75/75`, FTD-0967 integrity `19/19`, FTD-0968
  integrity `20/20`, FTD-0969 integrity `22/22`, Outcome B.

No engine or production file changed under the certificate chain.

## 9. Scope firewall

This theorem does not establish:

- autonomous formation or persistence of the witness body;
- a continuous frame through `kappa=0`;
- a state-dependent canonical moving-frame connection;
- reaction, switching work, energy/current closure, reserve, inverse, routing,
  or recycling;
- the FTD-0963 connection profile or repeated nonlinear stability;
- one-way phase-error export;
- a `G*` synchronization mechanism;
- Born/Bell recovery or operational hiding; or
- production integration or whole-framework completeness.

The next admissible step is to derive the connection and reciprocal reaction
induced by a moving regional frame, without changing production first.
