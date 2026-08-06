# FTD-0580 — Symmetric Chord Moore Action

**Status:** `[THEOREM — POSITIVE ENERGY-CENTERED SHAPE UNIQUENESS]` +
`[SELECTED — DEMOCRATIC SHORTEST-PATH FACE ROUTING]` +
`[THEOREM — EXACT COATED CONTINUITY AND ENERGY CENTERING]` +
`[THEOREM — POSITIVE CHORD PEIERLS BARRIER]` +
`[CLOSED NEGATIVE — UNMODIFIED CHORD AS GAPLESS MOBILE LAW]`  
**Date:** 2026-07-26  
**Verdict:**
`SYMMETRIC_CHORD_CLOSES_MOORE_CENTERING_PEIERLS_PINNING_REMAINS`

## 1. Scope

FTD-0578 found two obstructions for the straight trilinear Moore worldline:
diagonal endpoint-energy centering and Peierls pinning. FTD-0579 proved that
finite rigid smearing removes neither. FTD-0580 changes the subcell coupling
representation while keeping primitive manifestation site-valued and ternary.

## 2. Positive energy-centered shape

For a unit hop from site `0` to nonzero Moore neighbor `d`, require a
nonnegative coupling distribution `p_t` with

\[
 p_0=\delta_0,\quad p_1=\delta_d,\quad
 \sum_n p_t(n)=1,\quad \sum_n n p_t(n)=td,
\]

and require the exact time average to equal the endpoint midpoint needed by
the FTD-0576 energy identity:

\[
 \int_0^1p_tdt=\frac{\delta_0+\delta_d}{2}.
\]

For every site other than `0,d`, the integral of the nonnegative function
`p_t(n)` is zero, so that weight vanishes almost everywhere. With only the two
endpoints left, the first moment fixes their weights. The unique continuous
representative is

\[
 \boxed{p_t=(1-t)\delta_0+t\delta_d.}
\]

The FTD-0577 coat is applied afterward: `rho_t=B_Mp_t`. This is a derived
coupling sidecar under the stated positivity and energy-centering conditions,
not a new primitive state and not the FTD-0478 tensor-product trilinear shape.

## 3. Symmetric face current

Let `A` be the active axes and `D=|A|`. Average the `D!` monotone shortest
face paths uniformly. If `S` is the set of axes already traversed, the edge in
the next active direction `i` carries unsigned weight

\[
 \boxed{w(S,i)=\frac{|S|!(D-|S|-1)!}{D!}.}
\]

At an intermediate subset vertex, total incoming and outgoing weights both
equal

\[
 \frac{|S|!(D-|S|)!}{D!}.
\]

The start has unit outflow and the endpoint unit inflow, hence

\[
 \boxed{d_fK_d=\delta_0-\delta_d.}
\]

This construction is invariant under permutations and sign changes of the
active axes. It is canonical within the selected monotone-shortest-path class;
the theorem does not forbid adding a divergence-free curl or using longer
paths.

With the FTD-0577 bridge

\[
 q_i=A_i\prod_{j\ne i}B_jK_i,
\]

the exact identity `d_cA_i=B_id_f` gives

\[
 \boxed{D_cq=\rho_0-\rho_1.}
\]

All 104 registered raw and coated path arms close: raw residual is zero and
the maximum central residual is `1.39e-17`. All 24 proper cubic rotations
agree within `5.43e-19`.

## 4. Time-exact energy-centered action

For temporal hats `w_0=1-t`, `w_1=t`, direct integration gives

\[
 T_0=\frac{\rho_0}{3}+\frac{\rho_1}{6},\qquad
 T_1=\frac{\rho_0}{6}+\frac{\rho_1}{3},
\]

\[
 \boxed{T_0+T_1=\frac{\rho_0+\rho_1}{2}},\qquad
 Q_0=Q_1=\frac q2.
\]

Therefore

\[
 D_cQ_0=\rho_0-T,\qquad D_cQ_1=T-\rho_1.
\]

In the exact FTD-0576 work coordinate `R=J-W/2`, the selected interaction

\[
 \boxed{I_{\rm ch}=G_C\sum_{a=0}^1
 [\langle T_a,DR_a\rangle+\langle Q_a,CR_a\rangle]}
\]

is both the exact time integral for the chord history and the exact
endpoint-energy-centered source for axial, edge, and body hops. The compiled
centering residual is zero; split continuity closes within `6.94e-18`.

This proves the FTD-0578 diagonal centering defect was a defect of the
trilinear path representation, not an unavoidable consequence of Moore
motion.

## 5. Remaining Peierls barrier

At fractional coordinate `r`,

\[
 \widehat\rho_r=B_M[(1-r)+re^{-ik\cdot d}].
\]

Eliminating the FTD-0575 Hodge field yields

\[
 V_{\rm self}(r)=V_{\rm self}(0)+C_dr(1-r),
\]

\[
 \boxed{C_d=\frac{G_C^2}{L^3}\sum_kR_H(k)B_M(k)^2
 [1-\cos(k\cdot d)]>0.}
\]

The integrand is nonnegative and strictly positive on an open set for every
nonzero integer `d`. The observer finds all 104 registered coefficients
positive. The minimum coefficient is `2.6961904613504844e-4`, the minimum
half-cell barrier is `6.740476153376211e-5`, and 936 potential samples obey
the quadratic law within `2.59e-17`.

## 6. Consequence

The symmetric chord construction solves exact diagonal centering, local face
continuity, central-current compatibility, and cubic routing without a hidden
axis order. It does not solve gapless mobility. An unmodified finite chord has
a finite conservative depinning barrier, so arbitrarily small momentum does
not generate a translational pole.

The remaining live mechanisms are a genuinely deforming nonlinear dressing
whose internal energy cancels or dynamically traverses the barrier, a
noncompact/band-limited excitation, or integer hopping above a derived
threshold. None is established here. No production rule, toggle, default,
scenario, carrier ontology, particle, or Lorentz claim changes.

FTD-0581 closes the stable passive-dressing part of that list: the relaxed
Hodge solution already minimizes the field-plus-source energy and a Lipschitz
stable deformation cannot cancel the integer-site cusp. The narrower active,
finite-excitation phase-locking route remains open.

The locked preregistration SHA-256 is
`E3B651CA2E4D05395DA876DA61B873A11E6E5BD17220CDC70EB055F944527DF3`.
