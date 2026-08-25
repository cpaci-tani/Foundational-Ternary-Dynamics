# C6 cubic-Petrie material clock and endogenous directional port v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT NONPLANAR NEUTRAL PROTO-MATTER CLOCK]** +
**[THEOREM — DISCRETE CONTINUITY AND ISOTROPIC MEAN STRESS]** +
**[THEOREM — ROUTE-DERIVED SPATIAL PSEUDOSCALAR]** +
**[THEOREM, CONDITIONAL — ENDOGENOUS DIRECTIONAL RADIATION PORT]** +
**[BOUNDARY — PREPARED LOOP, NO FORMATION/BINDING/TRANSLATIONAL RECOIL]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_c6_petrie_material_clock_endogenous_directional_port.py](../../../../../scripts/proofs/proof_c6_petrie_material_clock_endogenous_directional_port.py)
performs **170,979 exact checks**. It exhausts all 48 ordered orthonormal SC
triads, six material phases, twelve cotangent stages, both charge
orientations, arbitrary permission words through length eight, and the full
48-element signed cubic group. No target constant, numerical eigensolver, or
measured input enters.

---

## 1. The smallest useful change from the planar clock

The square material recurrence supplies an ordered polar plane but cannot
select a polar normal under the reflection fixing that plane. The handed-port
theorem therefore priced one spatial pseudoscalar \(\chi\).

That price need not be paid by adding a free label. Let

\[
 (d,v,w)                                                     \tag{1}
\]

be an ordered orthonormal triad of signed SC directions. Consider the
six-edge cubic Petrie route

\[
 \boxed{d,\ v,\ w,\ -d,\ -v,\ -w.}                          \tag{2}
\]

Its vector sum vanishes. Starting at one cube vertex, its first six vertices
are distinct and the sixth edge returns exactly to the origin. Thus equation
(2) is a finite nonplanar closed route rather than an imported spatial axis.

There are exactly

\[
 3!\,2^3=48                                                 \tag{3}
\]

ordered signed-coordinate triads, and the complete signed cubic group acts
transitively on them.

---

## 2. Neutral material recurrence and clock

At material phase \(q\in\mathbb Z_6\), place a neutral ternary dipole on the
current route edge:

\[
 s(x_q)=-\epsilon,qquad s(x_{q+1})=+\epsilon.               \tag{4}
\]

One admitted material tick advances the dipole to the next edge. The signed
current consists of the negative endpoint moving along the old edge and the
positive endpoint moving along the next edge. Exact incidence gives

\[
 \boxed{\rho_{q+1}-\rho_q=\partial j_q.}                    \tag{5}
\]

The route returns after six admitted ticks. The cotangent field stage advances
on every global tick and returns after twelve. With every material tick
admitted, the common state therefore has period

\[
 \operatorname{lcm}(6,12)=12.                               \tag{6}
\]

For an arbitrary retained permission history \(g_n\in\{0,1\}\), the exact
state variables are

\[
 q_N=q_0+\sum_{n<N}g_n\pmod6,
 \qquad
 t_N=t_0+N\pmod{12}.                                        \tag{7}
\]

The inverse uses the same retained permission. Equation (7) is the exact
global-clock/local-material-clock split on this nonplanar recurrence.

---

## 3. Isotropic mean stress

The instantaneous route stress is the dyad

\[
 T_q=e_qe_q^{\mathsf T}.                                    \tag{8}
\]

Each coordinate axis occurs once with each sign around equation (2), so

\[
 \boxed{{1\over6}\sum_{q=0}^{5}T_q={I_3\over3}.}            \tag{9}
\]

The cycle-averaged signed transport current vanishes. Charge conjugation
reverses charge/current while preserving equation (9). Compared with the
square clock's plane projector, equation (9) is an isotropic prepared
stress source.

This does not make the loop a massive particle. Its route, reserve, and
initial phase are prepared; no formation basin, binding response,
perturbative stability, or physical mass has been derived.

---

## 4. Handedness is route history

At any route phase, let the current, next, and following directions be

\[
 (e_q,e_{q+1},e_{q+2}).                                     \tag{10}
\]

Define

\[
 \boxed{\chi_q
 =\det[e_q,e_{q+1},e_{q+2}]
 =(e_q\times e_{q+1})\cdot e_{q+2}\in\{\pm1\}.}            \tag{11}
\]

Under \(R\in O_h\),

\[
 \chi_q\mapsto\det(R)\chi_q.                               \tag{12}
\]

Thus \(\chi_q\) is exactly the spatial pseudoscalar required by the handed
radiation port. More strongly, the route identity is

\[
 \boxed{e_{q+2}=\chi_q(e_q\times e_{q+1}).}                 \tag{13}
\]

The outgoing polar direction is not reconstructed from an instantaneous
plane plus a free bit. It is the third direction already retained in the
nonplanar transaction history.

---

## 5. Endogenous directional radiation port

Feed the local data

\[
 (d_{\rm port},v_{\rm port},\chi_{\rm port})
 =(e_q,e_{q+1},\chi_q)                                      \tag{14}
\]

into the certified cotangent handed port. Equation (13) gives

\[
 r_{\rm port}=e_{q+2}.                                      \tag{15}
\]

The standing and outgoing sixteen-record modes then obey

\[
 \boxed{
 h_F({\cal S})=1,\quad p_F({\cal S})=0,
 \qquad
 h_F({\cal O})=2,\quad p_F({\cal O})=e_{q+2}.}              \tag{16}
\]

All 48 spatial transformations commute with the material step and with this
port construction. The C4 phase is read from the common cotangent stage, so
no independent radiation clock is added.

This closes the handedness-**ownership** debt on a prepared nonplanar matter
recurrence. It does not show that ordinary manifestation dynamics forms that
recurrence.

---

## 6. What remains before recoil is physical

Standing-to-outgoing conversion gives

\[
 \Delta p_F=e_{q+2}.                                        \tag{17}
\]

Momentum conservation therefore requires

\[
 \Delta p_M=-e_{q+2}.                                       \tag{18}
\]

The present material state has an owned three-dimensional route but not a
derived translational momentum coordinate. Changing its route phase is
internal recurrence; it is not center-of-mass recoil. Appending a momentum
label and toggling it by equation (18) would install the desired answer.

The next gate must derive a reversible displacement/momentum pair for the
localized loop and make the same standing/outgoing port event update that
pair. Its inverse must be an incoming packet absorbed by the loop. The finite
collision/streaming action must also preserve the coarse Maxwell norm after
the ray bank leaves the source.

Only after those pass may the transaction be interpreted as a native Lorentz
response. Autonomous formation/binding, a charged static pole, gravity and
lensing, physical Born preparation, and native alpha remain open.

The
[reciprocal-recoil successor](THEOREM_C6_PETRIE_DIRECTIONAL_PORT_RECIPROCAL_RECOIL_CURRENT_v1.md)
closes the equal-and-opposite manifested displacement at one local port. It
does not promote that displacement to physical momentum: the translational
Legendre map, kinetic-energy law, and emitted-field handoff remain open.
