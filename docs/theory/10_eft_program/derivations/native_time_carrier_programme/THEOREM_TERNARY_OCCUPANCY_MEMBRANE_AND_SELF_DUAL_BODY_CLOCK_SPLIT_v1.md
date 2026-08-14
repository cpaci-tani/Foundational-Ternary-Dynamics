# Theorem — Ternary occupancy membrane and self-dual body-clock split v1

**Identifier:** `FTD-0990`  
**Date:** 2026-08-12  
**Status:** `[THEOREM — UNIQUE STATIC OCCUPANCY MEMBRANE]` +
`[THEOREM, CONDITIONAL — UNIQUE COMMON-STORAGE / RELATIVE-INTERACTION SPLIT]` +
`[THEOREM, CONDITIONAL — CONNECTED-BODY UNIFORM CLOCK MODE]` +
`[TYPE PRICE RETIRED — NO INDEPENDENT STATIC BOND MEMORY]` +
`[SELECTION — OCCUPANCY-CONTROLLED DUAL-STIFFNESS LAW]` +
`[OPEN — DYNAMIC APERTURE / FORMATION / PRODUCTION]`

## Result

The actual ternary state already contains the minimum **static** information
needed to define the regional membrane of FTD-0988/0989. No independent
ternary latch must be stored on every fixed matter--void bond.

For `s_x in {-1,0,+1}`, define

\[
 \boxed{m_x=s_x^2\in\{0,1\}.}                            \tag{1}
\]

This is the unique charge-blind occupancy function with `m(0)=0` and
`m(+/-1)=1`. For an oriented C18 bond `(x,y)`, set

\[
 \eta_{xy}=m_x-m_y,
 \qquad
 \boxed{g_{xy}=1-\eta_{xy}^2
 =1-m_x-m_y+2m_xm_y.}                                   \tag{2}
\]

Equation (2) is the unique symmetric Boolean gate that transmits equal
occupancy and cuts unequal occupancy. It is therefore one on matter--matter
and void--void bonds and zero exactly on the matter--void boundary.

Using the exact C18 incidence factor,

\[
 K_m=B^TG_mB,
 \qquad G_m=\operatorname{diag}(g_b),                    \tag{3}
\]

is positive, Moore-local, and separates every connected occupied component
from the surrounding void.

The dual substrate then admits a minimum conditional division of labor. In
canonical common/relative coordinates,

\[
 q_\pm={q_L\pm q_R\over\sqrt2},
 \qquad p_\pm={p_L\pm p_R\over\sqrt2},                  \tag{4}
\]

choose the common sector to use the occupancy membrane `K_m` and retain the
full C18 stiffness `K` on the relative sector:

\[
 H_0={1\over2}p_+^Tp_+ + {1\over2}q_+^TK_mq_+
    +{1\over2}p_-^Tp_- + {1\over2}q_-^TKq_-.             \tag{5}
\]

The common mode is the body's protected recursive storage/clock channel. The
relative mode remains the open interaction channel. This is not a verbal
analogy: it is an exact positive quadratic decomposition with different
boundary currents.

Conditional on requiring (4), common isolation, unchanged relative C18
propagation, real quadratic dynamics, and `L/R` exchange symmetry, the `L/R`
stiffness is uniquely

\[
 \boxed{{1\over2}
 \begin{pmatrix}
 K_m+K&K_m-K\\
 K_m-K&K_m+K
 \end{pmatrix}.}                                        \tag{6}
\]

Thus the required `L/R` cross coupling is supported only on the matter--void
boundary. It has no fitted coefficient.

For a fixed connected body, the existing imposed matter-site de Broglie clock
then selects the normalized uniform common mode as the unique lowest body
mode. If its imposed frequency is `omega_0>0`, that mode obeys

\[
 \boxed{H_u=\omega_0 I_u.}                               \tag{7}
\]

This is the first exact conditional gearbox identifying the same actual
occupancy mask with both a body membrane and the support of a local clock. It
does not derive the coupling law, `omega_0`, a `G*` cadence, body formation,
or production dynamics.

## Certificate of record

- Protocol:
  [`PREREG_TERNARY_OCCUPANCY_MEMBRANE_AND_SELF_DUAL_CLOCK_SPLIT_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_TERNARY_OCCUPANCY_MEMBRANE_AND_SELF_DUAL_CLOCK_SPLIT_v1.md),
  pre-execution SHA-256
  `461F6D68F2C28964D01A9AD21DA142CF0A446364FB17524E8A6F9246CBDFA904`.
- Immutable exact certificate:
  [`proof_ternary_occupancy_membrane_and_self_dual_clock_split.py`](../../../../../scripts/proofs/proof_ternary_occupancy_membrane_and_self_dual_clock_split.py),
  SHA-256
  `2EBC5EA2158B26218381C0AEBCF176B6274606C16F0068410BB96413C2857941`.
- First locked execution: `81/81`, **Outcome B**, no repair.
- Frozen sources: three theorem and four CPU production sources.
- Production mutation: none.

## 1. Uniqueness of the occupancy bit

A charge-blind function on the ternary alphabet satisfies `f(-1)=f(+1)`.
The endpoint requirements

\[
 f(0)=0,qquad f(-1)=f(+1)=1                           \tag{8}
\]

specify all three values, so equation (1) is unique as a function. In the
class of even real polynomials of degree at most two, write

\[
 f(s)=a s^2+b.                                           \tag{9}
\]

Equation (8) gives `b=0` and `a=1`. No coefficient has been fitted.

For two Boolean occupancies there are only four input pairs. Requiring a
symmetric gate with

\[
 g(0,0)=g(1,1)=1,qquad g(0,1)=g(1,0)=0                 \tag{10}
\]

specifies its complete truth table. Reducing Boolean powers `m^2=m` gives
exactly equation (2).

The gate is invariant under global charge conjugation because `(-s)^2=s^2`.
It is signed-cubic covariant because a spatial symmetry only permutes its
endpoint sites. Edge reversal gives

\[
 \eta_{yx}=-\eta_{xy},qquad g_{yx}=g_{xy}.              \tag{11}
\]

The sign `eta` is a spatial boundary-normal orientation. It is unchanged by
time reversal and cannot replace a clockwise/counterclockwise event sign.

## 2. Static regional isolation without bond memory

Equation (3) has quadratic form

\[
 q^TK_mq=\sum_{(x,y)}g_{xy}a_{xy}(q_y-q_x)^2\ge0.       \tag{12}
\]

Every term touches one C18 face or edge bond, so the operator remains within
one Moore shell. If `m_x` and `m_y` differ, its bond term vanishes. Hence no
matrix element of `K_m` connects an occupied component to a void component.
For a connected occupied region `Lambda`,

\[
 K_m=K_\Lambda\oplus K_{\Lambda^c}.                     \tag{13}
\]

At fixed `s`, every `g_xy` can be recomputed from the two endpoint states. A
separate stored boundary latch is redundant. This retires the **static type
price** contemplated by FTD-0989.

It does not retire the dynamical law. If occupancy changes, `K_m` changes.
That switching event still has the exact work

\[
 W={1\over2}q^T(K_{m'}-K_m)q.                            \tag{14}
\]

It is work-free and impulse-free only when every affected bond is at the
FTD-0989 zero-strain seam.

## 3. The self-dual common/relative split

The transformation (4) is orthogonal and symplectic. In the `+/-` chart the
stiffness is `diag(K_m,K)`. Conjugating back to `L/R` gives equation (6).
Conversely, every real `L/R`-swap-invariant quadratic block has the form

\[
 \begin{pmatrix}A&C\\C&A\end{pmatrix}.                  \tag{15}
\]

Its common and relative stiffnesses are `A+C` and `A-C`. Requiring them to be
`K_m` and `K` uniquely gives

\[
 A={K_m+K\over2},qquad C={K_m-K\over2}.                \tag{16}
\]

This proves conditional uniqueness. Positivity follows because equation (6)
is orthogonally equivalent to the positive block `diag(K_m,K)`.

Inside any equal-occupancy bulk, `K_m=K` bond by bond, so `C=0`. On a boundary
bond, `K_m-K` is minus its positive incidence square. The `L/R` cross term is
therefore local and boundary-supported.

The current statement is exact:

- the common-sector current carries the factor `g_xy` and vanishes through
  the membrane;
- the relative-sector current retains `g=1` and crosses the same boundary.

The two dual combinations are not disconnected worlds. One stores the body's
recursive phase while the other continues to exchange with its environment.
An active transfer between them still requires the separately retained C4
orientation/controller law.

## 4. The uniform body clock

On a connected occupied component, the internal weighted Laplacian obeys

\[
 q^TK_\Lambda q=sum_{(x,y)\subset\Lambda}
 a_{xy}(q_y-q_x)^2.                                     \tag{17}
\]

It vanishes exactly when every adjacent value agrees. Connectivity therefore
proves

\[
 \ker K_\Lambda=\operatorname{span}\{\mathbf1_\Lambda\}. \tag{18}
\]

Let

\[
 u_\Lambda={\mathbf1_\Lambda\over\sqrt{|\Lambda|}},
 \qquad Q=u_\Lambda^Tq_+,quad P=u_\Lambda^Tp_+.         \tag{19}
\]

The production de Broglie term is imposed exactly at `state!=0` sites and
acts identically on `L` and `R`. Since `state!=0` is equivalent to `s^2=1`,
the same mask in equation (1) supports the onsite potential

\[
 {\omega_0^2\over2}\sum_xm_x(q_{+,x}^2+q_{-,x}^2).      \tag{20}
\]

For the isolated common body sector,

\[
 (K_\Lambda+\omega_0^2I)u_\Lambda
 =\omega_0^2u_\Lambda.                                  \tag{21}
\]

When `omega_0>0`, every other connected-body eigenvalue is strictly larger,
so (19) is the unique lowest common mode. Its action-angle chart is

\[
 Q=\sqrt{\frac{2I_u}{\omega_0}}\cos\theta,qquad
 P=-\sqrt{2\omega_0 I_u}\sin\theta,                     \tag{22}
\]

and equation (7) follows. A work transaction uses

\[
 \boxed{I_u'=I_u+{H-H'\over\omega_0}}.                  \tag{23}
\]

At `omega_0=0`, the uniform coordinate is a zero mode and not a regular clock
action. Production currently imposes `omega_0`; neither its value nor a link
to the critical-quartic `G*` calendar follows from this theorem.

## 5. What remains dynamically unpaid

The fixed occupancy mask eliminates a static memory import, but it is not an
active aperture:

- equation (2) always cuts a matter--void bond and cannot temporarily open
  one while preserving the same body state;
- the boundary-normal sign `eta` is time-even, while a reversible crossing
  needs the time-odd `sigma=sgn(p_y-p_x)` and a receiving history record;
- changing `s` changes the membrane and incurs equation (14) away from the
  zero-strain seam;
- production genesis uses a random acceptance draw and its selected flux/
  kinetic drain is explicitly not an exact common-action latent-heat law;
- evaporation sets `state` to zero, and the optional event journal is
  observation-only; and
- unchanged CPU production applies the same full C18 stencil to `L` and `R`,
  not equation (6).

Consequently the physical law in equation (6) remains **[SELECTED]**. An
active charging aperture still needs controller state or another
substrate-derived mechanism. Autonomous body/membrane formation, exact
formation work, mode preparation, deformation, motion, collision,
backpressure, and environment-complete reversal remain open.

## 6. Epistemic disposition

Established:

- **[THEOREM]** `s^2` is the unique charge-blind ternary occupancy bit;
- **[THEOREM]** equation (2) is the unique static equality membrane;
- **[THEOREM]** its C18 incidence stiffness is local, positive, and region
  separating;
- **[THEOREM, CONDITIONAL]** equation (6) is the unique real quadratic
  `L/R`-symmetric realization of common isolation plus relative interaction;
- **[THEOREM, CONDITIONAL]** a connected body with `omega_0>0` has the unique
  uniform common clock mode (7); and
- **[TYPE PRICE RETIRED]** no independent static bond-memory variable is
  needed.

Selected/open:

- the occupancy-controlled dual-stiffness law itself is absent from
  production;
- an active aperture, temporal orientation transfer, and reversible switching
  controller remain physical debts;
- `omega_0` is imposed and the mode amplitude/phase is unprepared;
- genesis/evaporation do not pay exact formation work or provide an inverse;
  and
- `G*`, Born/Bell, mass, Lorentz hiding, selector energy, and completeness
  remain open.

No production integration follows.

