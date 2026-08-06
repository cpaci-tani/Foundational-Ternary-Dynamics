# FTD-0581 — Passive Dressing Depinning Obstruction

**Status:** `[THEOREM — EXACT PRODUCTION-DISPERSION DEPINNING THRESHOLD]` +
`[THEOREM — PASSIVE COMPLETED-SQUARE/CUSP OBSTRUCTION]` +
`[DERIVED — NECESSARY FINITE ACTIVE-EXCITATION BUDGET]` +
`[OPEN — NATIVE PHASE-LOCKED ACTIVE TRAVERSAL]`  
**Date:** 2026-07-26  
**Verdict:**
`PASSIVE_DRESSING_CANNOT_DEPIN_ACTIVE_TRAVERSAL_COSTS_FINITE_EXCITATION`

## 1. Scope

FTD-0580 repaired the diagonal energy-centering defect by selecting the
positive endpoint chord, but derived a finite Peierls curve

\[
 V_d(r)=V_d(0)+C_d r(1-r),\qquad C_d>0,
 \quad 0\le r\le1.
\]

FTD-0581 determines the exact kinetic threshold and asks whether deformation
of the already existing native `(J,W)` field can remove that curve. It does
not implement motion or infer a particle from an energy budget.

## 2. Exact relativistic depinning threshold

The production momentum convention is

\[
 H(p)=\sqrt{E_{\rm REST}^2+C_{\rm SPEED}^2|p|^2},
 \qquad K(p)=H(p)-E_{\rm REST}.
\]

The half-cell saddle has height

\[
 \Delta_d=\frac{C_d}{4}.
\]

A ground-dressed carrier starting at an integer minimum can reach that saddle
only if `K(p_0)>=Delta_d`. Equality gives

\[
 \boxed{p_{\rm dep}(d)=\frac{
 \sqrt{2E_{\rm REST}\Delta_d+\Delta_d^2}}
 {C_{\rm SPEED}}},
\]

\[
 \boxed{v_{\rm dep}(d)=
 \frac{C_{\rm SPEED}^2p_{\rm dep}(d)}
 {E_{\rm REST}+\Delta_d}}.
\]

Indeed,

\[
 \frac{v_{\rm dep}^2}{C_{\rm SPEED}^2}
 =1-\frac{E_{\rm REST}^2}
 {(E_{\rm REST}+\Delta_d)^2},
\]

so every positive finite barrier gives `0<v_dep<C_SPEED`.

Across `L=17,33`, both polarities, and all 26 signed Moore directions, the
observer obtains

\[
 2.6961904613504844\times10^{-4}
 \le C_d\le
 5.7637162071441487\times10^{-4},
\]

\[
 6.740476153376211\times10^{-5}
 \le\Delta_d\le
 1.4409290517860372\times10^{-4},
\]

\[
 0.008300680483739701\le p_{\rm dep}\le
 0.012137760806199034,
\]

\[
 0.016237567545273713\le v_{\rm dep}\le
 0.023732879818776923.
\]

Thus `v_dep/C_SPEED` lies between `0.0281243` and `0.0411066`. These are
internal lattice units, not a comparison to an experimental particle.

## 3. Why passive dressing cannot remove the barrier

For fixed `r`, the FTD-0574/0575 native linear Hodge field has a positive
quadratic field energy and a linear source term. On the gauge-fixed physical
subspace, completing the square about its stationary solution `z_*(r)` gives

\[
 E_{\rm field+source}(r,z)
 =V_d(r)+\frac12\langle z-z_*(r),
 K[z-z_*(r)]\rangle,
 \qquad K\succeq0.
\]

Therefore

\[
 \boxed{E_{\rm field+source}(r,z)\ge V_d(r).}
\]

The Peierls curve is already the relaxed, pointwise minimum field energy for
the registered source history. A passive deformation or dynamical lag cannot
lower it; both add nonnegative energy above it. A zero mode can leave the
energy unchanged but cannot supply the missing negative barrier.

## 4. The independent cusp obstruction

Periodically continue the one-cell curve. For small displacement from an
integer anchor,

\[
 V_d(r)-V_d(0)=C_d|r|-C_dr^2.
\]

The one-sided slopes are `-C_d,+C_d`. In contrast, let `z_0` be a stable
passive equilibrium and suppose its response is locally Lipschitz:
`z(r)-z_0=O(|r|)`. Positive quadratic stability gives

\[
 U(z(r))-U(z_0)=O(r^2).
\]

It has zero linear coefficient and cannot cancel `C_d|r|`. This local theorem
does not rely on the numerical size of the barrier.

Cancellation therefore requires at least one of the following departures:

- a nonstationary excited internal state;
- a non-Lipschitz field response;
- a zero/negative-energy direction that invalidates strict passive stability;
- a noncompact limit in which `C_d` tends to zero; or
- an explicit counterterm.

The last option is outside the frozen native program. The finite rigid and
finite chord cases already exclude the noncompact escape.

## 5. Price of an active internal mode

Let `epsilon_0` be internal excitation above the relaxed integer-site
dressing. Conservation of total energy supplies the necessary condition

\[
 \boxed{K(p_0)+\epsilon_0\ge\Delta_d.}
\]

At zero external momentum,

\[
 \boxed{\epsilon_0\ge\Delta_d>0.}
\]

This does not rule out an active breather or phase-locked field cycle. It
proves that such an object would not be a passively dressed translational
ground state: it must retain a finite excitation/phase as `p_0` tends to zero.

For a positive oscillator `U=(P^2+omega^2Q^2)/2`, the energy-only witness

\[
 U(r)=\epsilon_0-C_dr(1-r)
\]

is real only if `epsilon_0>=Delta_d`. At equality,

\[
 U(r)=C_d(r-1/2)^2,
 \qquad Q(r)=\frac{\sqrt{2C_d}}{\omega}|r-1/2|,
\]

which is continuous and Lipschitz but nondifferentiable at the half cell. For
strictly larger excitation it is smooth but begins and ends excited. This is
an energy-budget construction only. It does not derive the coupling, phase
recurrence, or equation of motion from the frozen action.

## 6. Consequence

Stable passive `(J,W)` dressing is closed as the cure for the FTD-0580 chord
barrier. The remaining native question is narrower and dynamical:

> Does the frozen nonlinear engine contain a localized, phase-carrying field
> mode with at least `Delta_d` excitation that transfers this energy
> reversibly across every saddle and restores it over repeated integer hops?

That candidate must demonstrate phase-dependent energy flow, repeated
barrier traversal, locality, continuity, Gauss closure, total-energy closure,
and inverse recovery. A one-hop budget or an externally prepared packet is
insufficient.

FTD-0582 closes that candidate for the frozen production tick: native fields
have no momentum-write path when selected forces are disabled. A future
common-action implementation is a new selected extension and must be assessed
under a new authorization, not promoted as latent production behavior.

No production state, action, force, movement phase, toggle, default, scenario,
particle claim, or Lorentz claim changes.

The locked preregistration SHA-256 is
`CB525DEF5A5E6B92127C4DFD9C72DCF1F7799E7D97113519EDF2C732E56B0DDC`.
