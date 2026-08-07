# FTD-0773 — Quartic Waveform Nonlinear Edge Signature v1

**Status:** `[THEOREM]` + `[IMPOSED]` + `[OPEN]`  
**Conditions:** the theorems use a continuous, branch-reversible, fixed
unit-mass natural coordinate; the quadratic coordinate edge is imposed; the
native recurrence and native coupling remain open.  
**Exact verdicts:**
`QUARTIC_CONTINUOUS_INVERSE_CHAIN_CONDITIONAL_THEOREMS_PASS` +
`QUARTIC_NONLINEAR_EDGE_SHAPE_FUNCTIONAL_GSTAR_PRESENT`  
**Native verdicts:** `NATIVE_QUARTIC_TIME_DERIVATION_NOT_ESTABLISHED` +
`NATIVE_NONLINEAR_EDGE_TEST_BLOCKED`  
**Protocol SHA256:**
`33E126673B8F072CAEBAD490B74F810818373D8014CC2D6F73CEF9592ED88DAA`  
**Production impact:** none; no engine or selected-model execution was run

## 1. Result in one sentence

The proposed occupancy-to-clock chain and the nonlinear ratio

\[
\boxed{
\mathcal B_4=
\frac{\overline V(\pi)-\overline V(0)}{\overline V''(0)}
=\frac{48\pi}{G^{*4}}
=1.9678953151426559559\ldots
}
\]

are exact for an already-selected continuous quartic natural coordinate with
the separately selected quadratic edge energy
`epsilon(q_v-q_w)^2/2`; they do not derive that clock or coupling from the
native FTD tick map, and the existing native candidate is blocked by
FTD-0772 before this signature can be measured.

The ratio is a useful **conditional waveform-shape invariant**. It is not an
independent determination of `G*`: its barrier and curvature are two
functionals of the same assumed quartic autocorrelation.

## 2. Occupancy determines speed only with a branch hypothesis

Let a continuous periodic orbit have period `T(A)`, fix the signed scalar
`q=Q(X)` before inspecting its waveform, and write `x=q/A`. The pushforward
of uniform elapsed time gives, at a regular interior value,

\[
\rho_A(x)=\frac1{T(A)}
\sum_{t_j:x(t_j)=x}\frac1{|\dot x(t_j)|}.
\]

If every interior value is crossed exactly twice per cycle and the two
crossings have equal speed magnitude, then

\[
\boxed{
\rho_A(x)=\frac{2}{T(A)|\dot x(x)|},
\qquad
|\dot q|=\frac{2A}{T(A)\rho_A(x)}.
}
\]

Without equal branch speeds `v_+(x)` and `v_-(x)`, occupancy yields only

\[
\rho_A(x)=\frac1{T(A)}
\left(\frac1{v_+(x)}+\frac1{v_-(x)}\right),
\]

so `2/[T(A)rho_A(x)]` is their harmonic mean, not either branch speed. This
is the first boundary on the inverse construction.

## 3. Speed determines a potential only after natural-coordinate closure

Now impose a fixed unit-mass natural coordinate with one conservative
potential,

\[
\frac12\dot q^2+V(q)=V(A),
\qquad V(0)=0.
\]

Substituting the equal-branch speed gives

\[
\boxed{
V(A)-V(Ax)=
\frac{2A^2}{T(A)^2\rho_A(x)^2}
}
\]

and, at the central crossing,

\[
V(A)=\frac{2A^2}{T(A)^2\rho_A(0)^2}.
\]

Therefore

\[
\boxed{
V(Ax)=\frac{2A^2}{T(A)^2}
\left[\rho_A(0)^{-2}-\rho_A(x)^{-2}\right].
}
\]

If branch reversibility, constant-mass closure, or conservativity has not
been established, the right-hand side is only a **pseudo-potential
diagnostic**. A scalar waveform by itself does not establish a canonical
momentum or exclude hidden-state, velocity, or memory dependence.

## 4. Exact quartic characterization

Take the normalized quartic occupancy

\[
\rho_4(x)=\frac{C_4}{\sqrt{1-x^4}},
\qquad
C_4=\frac2{\sqrt\pi G^*},
\qquad
G^*=\frac{\Gamma(1/4)}{\Gamma(3/4)}.
\]

Since

\[
\rho_4(0)^{-2}-\rho_4(x)^{-2}
=\frac{x^4}{C_4^2},
\]

the inverse formula gives

\[
V(Ax)=V(A)x^4.
\]

If the same fixed coordinate and potential support this exact law over a
nontrivial interval of amplitudes, overlapping swept regions force

\[
\frac{V(A)}{A^4}=\lambda,
\qquad
\boxed{V(q)=\lambda q^4}
\]

on that swept region. The converse follows by direct substitution. This is
the FTD-0772 fixed-coordinate characterization theorem; it does not select a
native observable, prove a continuum bridge, or extend the potential beyond
the tested region.

## 5. Period, phase, action, and curvature

For the selected continuous Hamiltonian

\[
H_0(q,p)=\frac{p^2}{2}+\lambda q^4,
\qquad E=\lambda A^4,
\]

the quarter-period integral is

\[
\frac{T(A)}4
=\frac1{A\sqrt{2\lambda}}
\int_0^1\frac{dx}{\sqrt{1-x^4}}
=\frac{\sqrt\pi G^*}{4A\sqrt{2\lambda}}.
\]

Hence

\[
\boxed{
T(A)=\frac{\sqrt\pi G^*}{A\sqrt{2\lambda}},
\qquad
\Omega(A)=\frac{2\sqrt\pi A\sqrt{2\lambda}}{G^*}.
}
\]

Uniform phase satisfies `dtheta/dx=pi rho_4(x)`. On the increasing branch
from the central crossing,

\[
\theta_\uparrow(x)=
\frac{\pi}{2\sqrt\pi G^*}
\operatorname{sgn}(x)
B_{|x|^4}\!\left(\frac14,\frac12\right),
\qquad -1\le x\le1,
\]

where `theta(1)=pi/2`; the other branches require reflection and continuous
phase lifting. Position alone does not define a global phase because it does
not identify the branch.

If `(q,p)` is additionally a canonical pair with `p=q_dot`, the normalized
action

\[
I=\frac1{2\pi}\oint p\,dq
\]

is

\[
\boxed{
I(A)=\frac{A^3\sqrt{2\lambda}G^*}{3\sqrt\pi}.
}
\]

Direct differentiation then verifies

\[
\boxed{
\Omega=\frac{dE}{dI},
\qquad
H_0''(I)=\frac{d\Omega}{dI}
=\frac{2\pi}{A^2G^{*2}}.
}
\]

The action statement is conditional on the symplectic structure; periodic
motion of one scalar is not enough to derive it.

## 6. Moments and the local exponent identity

For `r>-1`, quartic occupancy gives

\[
\mu_r=\langle|x|^r\rangle
=\frac{B((r+1)/4,1/2)}{B(1/4,1/2)},
\qquad
\mu_{r+4}=\frac{r+1}{r+3}\mu_r.
\]

In particular,

\[
\mu_1=\frac{\sqrt\pi}{G^*},
\qquad
\mu_2=\frac4{G^{*2}},
\qquad
\mu_4=\frac13.
\]

Thus

\[
G_{\rm rms}=\frac2{\sqrt{\mu_2}},
\qquad
G_{\rm abs}=\frac{\sqrt\pi}{\mu_1},
\qquad
G_{\rm kurt}=
\left(48\frac{\mu_4}{\mu_2^2}\right)^{1/4}
\]

all return `G*` for the exact law. They are correlated diagnostics of one
distribution, not three independent measurements, and finitely many moments
do not identify the full law.

For the exact homogeneous family

\[
\rho_m(x)=\rho_m(0)(1-|x|^m)^{-1/2},
\]

one may rearrange

\[
\boxed{
m=\frac{\log\!\left[1-(\rho_m(0)/\rho_m(x))^2\right]}
{\log|x|}
}
\]

only for `0<|x|<1`. It is singular or ill-conditioned at the center and
turning points and is not available from a finite atomic tick measure without
a separately locked density estimator. FTD-0773 therefore performs no
empirical exponent fit.

## 7. Selected quadratic edge and the general shape ratio

Now add the distinct `[IMPOSED]` model choice

\[
V_{vw}=\frac\varepsilon2(q_v-q_w)^2,
\qquad \varepsilon>0,
\]

and compare two equal-amplitude uncoupled waveforms using uniform phase. For
the even-power family

\[
H_m=\frac12\left(p^2+|q|^m\right),
\]

let

\[
C_m(\phi)=\frac1{2\pi}\int_0^{2\pi}
x_m(\theta)x_m(\theta+\phi)\,d\theta.
\]

Cycle averaging gives

\[
\overline V_m(\phi)
=\varepsilon A^2[\mu_{2,m}-C_m(\phi)].
\]

Write

\[
B_{m0}=B\!\left(\frac1m,\frac12\right),
\qquad
\mu_{2,m}=\frac{B(3/m,1/2)}{B_{m0}}.
\]

Half-wave antisymmetry gives `C_m(pi)=-mu_2,m`, while periodic integration by
parts gives `-C_m''(0)=<(x_m')^2>`. The normalized phase speed obeys

\[
\left(\frac{dx_m}{d\theta}\right)^2
=\frac{4B_{m0}^2}{m^2\pi^2}(1-|x_m|^m),
\]

and `mu_m=2/(m+2)`. Therefore

\[
D_m:=\langle(x_m')^2\rangle
=\frac{4B_{m0}^2}{m(m+2)\pi^2}.
\]

The antiphase barrier, in-phase curvature, and their ratio are

\[
\boxed{
\begin{aligned}
\Delta_m
&:=\overline V_m(\pi)-\overline V_m(0)
=2\varepsilon A^2\mu_{2,m},\\
K_m
&:=\overline V_m''(0)
=\varepsilon A^2D_m,\\
\mathcal B_m
&:=\frac{\Delta_m}{K_m}
=\frac{m(m+2)\pi^2B(3/m,1/2)}{2B_{m0}^3}.
\end{aligned}
}
\]

The registered exact controls are

\[
\boxed{
\mathcal B_2=2,
\qquad
\mathcal B_4=\frac{48\pi}{G^{*4}},
\qquad
\mathcal B_6=
\frac{24\pi^3}{B(1/6,1/2)^3}.
}
\]

For the quartic member specifically,

\[
\left(\frac{dx}{d\theta}\right)^2
=\frac{G^{*2}}{4\pi}(1-x^4),
\qquad
D_4=\frac{G^{*2}}{6\pi},
\]

so

\[
\Delta_4=\frac{8\varepsilon A^2}{G^{*2}},
\qquad
K_4=\frac{\varepsilon A^2G^{*2}}{6\pi},
\]

and consequently

\[
\boxed{
\mathcal B_4=\frac{48\pi}{G^{*4}},
\qquad
H_0''K_4=\frac\varepsilon3.
}
\]

The second cancellation belongs to this selected, cycle-averaged quadratic
coordinate coupling. It does not by itself derive or retroactively identify
the selected cosine coupling used in FTD-0770.

## 8. What the ratio does and does not remove

`mathcal B_4` cancels the amplitude `A`, edge strength `epsilon`, constant
physical-time scaling, and `lambda` after uniform `2pi` phase is fixed. It is
therefore a nontrivial scale-free discriminator between locked waveform
families; for reference, the exact controls evaluate descriptively to

\[
\mathcal B_2=2,
\qquad
\mathcal B_4=1.9678953151426559559\ldots,
\qquad
\mathcal B_6=1.92398619936\ldots.
\]

It does **not** remove dependence on the observable coordinate or on the edge
functional. A nonlinear change `y=f(x)` changes both the autocorrelation and
the meaning of `(q_v-q_w)^2`; another interaction changes the averaged
potential. The rearrangement

\[
G_{\rm edge}=\left(48\pi\frac{K_4}{\Delta_4}\right)^{1/4}
\]

is thus a conditional consistency functional, not epistemically independent
evidence for `G*`.

## 9. Why this is not yet native FTD time

FTD evolves by primitive discrete ticks. A finite `P`-tick recurrence has the
atomic occupancy measure

\[
\mu_P=\frac1P\sum_{n=0}^{P-1}\delta_{x_n},
\]

not the continuous density used above. Derivatives, a continuous potential,
the incomplete-beta phase, canonical action, and a continuous adjoint require
a separately proved suspension, refinement, or equidistribution bridge. A
differentiable discrete recurrence would instead require a discrete-map
phase adjoint.

More decisively, FTD-0772 found that all `18/18` locked cells of the current
FTD-0659 native candidate fail the return-amplitude and stationarity gates.
There is no qualified recurrent orbit on which to measure invariant
occupancy. The current corpus also contains no native paired-edge interaction
record corresponding to the selected quadratic coordinate energy.

Running the engine now would insert both the quartic clock and its quadratic
edge and then recover an already-proved consequence. That would test an
implementation of the selected model, not derive native FTD time. No such run
is licensed by FTD-0773.

## 10. Claim accounting

| claim | result | status |
|---|---|---|
| occupancy -> branch speed | exact with two equal-speed crossings | `[THEOREM]` |
| occupancy -> potential | exact with fixed unit-mass conservative closure | `[THEOREM]` |
| quartic occupancy iff quartic potential | exact on the swept amplitude region for the fixed natural coordinate | `[THEOREM]` |
| period, phase, action, and moments | exact for the imposed continuous Hamiltonian; action needs canonical structure | `[THEOREM]` + `[IMPOSED]` |
| quadratic-edge `mathcal B_4` | exactly `48pi/G*^4` for the imposed quadratic edge | `[THEOREM]` + `[IMPOSED]` |
| independent `G*` determination | numerator and curvature share one quartic autocorrelation | `INDEPENDENCE_PROMOTION_INVALID` |
| native quartic-time derivation complete | continuous/native and recurrence/coupling prerequisites are missing | `[OPEN]`; `NATIVE_QUARTIC_TIME_DERIVATION_NOT_ESTABLISHED` |
| native nonlinear edge campaign now | FTD-0772 recurrence gate fails and no native edge record exists | `[OPEN]`; `NATIVE_NONLINEAR_EDGE_TEST_BLOCKED` |

## 11. Verification

The immutable protocol is
[`PREREG_QUARTIC_WAVEFORM_NONLINEAR_EDGE_SIGNATURE_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_QUARTIC_WAVEFORM_NONLINEAR_EDGE_SIGNATURE_v1.md).
Its SHA256 is the value recorded in the header.

The independent exact certificate is
`scripts/proofs/proof_quartic_waveform_nonlinear_edge_signature.py`. It passes
`95/95` exact and structural checks and has SHA256
`CA8876D7DCF8370C313C96C9016A81A15E7E183D8E8E9FC9F630658DF943CF7E`.

## 12. Next falsifier

The next native target remains the one identified by FTD-0772: construct a
complete coupled matter--field recurrence and first demonstrate localization,
complete-state return, stable amplitude over longer histories and multiple
volumes, and closure of a preregistered signed observable. Only after those
gates pass may the full occupancy law be retested; only after occupancy passes
may native phase response and edge coupling be measured with `mathcal B_4`
locked as a secondary waveform discriminator.
