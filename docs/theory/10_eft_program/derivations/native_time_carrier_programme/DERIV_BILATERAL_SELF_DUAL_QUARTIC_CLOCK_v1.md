# Derivation — Bilateral Self-Dual Quartic Clock v1

**Identifier:** `FTD-0836`  
**Status:** `[CONDITIONAL THEOREM — SELECTED CRITICAL QUARTIC HAMILTONIAN]` +
`[THEOREM — ORIENTED SELF-DUAL ENERGY COORDINATE AND G* TRAVERSAL]` +
`[IMPOSED — LOCAL RADIAL STABILIZER WITH EXACT ENERGY LEDGER]` +
`[OPEN — NATIVE LOCAL REALIZATION, AUTONOMOUS STABILIZATION, AND BIOLOGICAL APPLICATION]`  
**Date:** 2026-08-10  
**Production status:** unchanged; exact standalone mathematics only  
**Certificate:** `scripts/proofs/proof_bilateral_self_dual_quartic_clock_v2.py`,
`17/17`

## 0. Result in one sentence

Conditional on the selected critical quartic Hamiltonian, the signed
potential-energy coordinate `u=x|x|` turns the clock shell
`x^4+y^2=1` into the self-dual circle `u^2+y^2=1`; quartic Hamiltonian flow
is an oriented rotation of those two energy channels with angular speed
`-2 sqrt(|u|)`, and the nonuniform time required for one traversal is exactly

\[
 \boxed{\sqrt\pi\,G^*.}
\]

Thus `G*` is the traversal-time cost of lifting a self-dual two-channel energy
circle back to the physical quartic coordinate. This does not derive the
quartic law or a native stabilizer from the substrate.

## 1. Epistemic firewall

The dynamical input remains

\[
 H(q,p)=\frac{p^2}{2m}+\lambda q^4,
 \qquad m>0,\quad\lambda>0.                    \tag{1}
\]

FTD-0821/0827 state the status of (1): it is the critical-quartic branch, not
a P1--P5-derived production clock. FTD-0836 changes none of those debts.

The words *bilateral*, *left*, and *right* refer below to two mathematical
energy channels. The human cerebral hemispheres motivate an architectural
analogy only. No anatomical, cognitive, consciousness, or clinical claim is
derived or tested here.

## 2. Minimal oriented bilateral kernel

Take two real channels `L,R` and the quarter-turn

\[
 X=\begin{pmatrix}L\\R\end{pmatrix},
 \qquad
 \mathcal J=\begin{pmatrix}0&1\\-1&0\end{pmatrix},
 \qquad
 X_{n+1}=\mathcal JX_n.                         \tag{2}
\]

Exactly,

\[
 \mathcal J^T\mathcal J=I,
 \qquad
 \mathcal J^2=-I,
 \qquad
 \mathcal J^4=I.                               \tag{3}
\]

The ternary unit-shell orbit is

\[
 (1,0)\to(0,-1)\to(-1,0)\to(0,1)\to(1,0).    \tag{4}
\]

Its energy

\[
 \mathcal E=L^2+R^2                             \tag{5}
\]

is invariant. Its signed phase current is

\[
 \chi=L_nR_{n+1}-R_nL_{n+1}=-\mathcal E.       \tag{6}
\]

The reverse lift `-J=J^{-1}` gives `chi=+E`. Hence the kernel remembers
clockwise versus counterclockwise orientation.

But every degree-two observable is blind to the central sign:

\[
 \operatorname{Sym}^2(\mathcal J)
 =\operatorname{Sym}^2(-\mathcal J).            \tag{7}
\]

This is the smallest exact version of the orientation datum lost by the BCC
symmetric square.

## 3. The quartic shell is a nonlinear lift of the self-dual circle

Let `A` be the positive turning amplitude and normalize

\[
 x=\frac qA,
 \qquad
 y=\frac{p}{\sqrt{2m\lambda}\,A^2},
 \qquad
 s=A\sqrt{\frac{2\lambda}{m}}\,t.              \tag{8}
\]

The energy shell and flow are

\[
 x^4+y^2=1,
 \qquad
 \frac{dx}{ds}=y,
 \qquad
 \frac{dy}{ds}=-2x^3.                          \tag{9}
\]

Define the odd, invertible signed-energy coordinate

\[
 u=f(x)=x|x|,
 \qquad
 x=f^{-1}(u)=\operatorname{sgn}(u)\sqrt{|u|}.  \tag{10}
\]

Then

\[
 \boxed{x^4+y^2=u^2+y^2.}                      \tag{11}
\]

The quartic energy shell is therefore exactly the nonlinear lift of the
ordinary self-dual circle

\[
 \boxed{u^2+y^2=1.}                            \tag{12}
\]

The corresponding nonlinear quarter-turn is

\[
 \mathcal D_4=\Phi^{-1}\mathcal J\Phi,
 \qquad
 \Phi(x,y)=(f(x),y),                           \tag{13}
\]

or explicitly

\[
 \mathcal D_4(x,y)
 =\left(f^{-1}(y),-f(x)\right).                \tag{14}
\]

Because `Phi` is bijective,

\[
 \mathcal D_4^2(x,y)=(-x,-y),
 \qquad
 \mathcal D_4^4=I.                             \tag{15}
\]

This establishes an exact order-four oriented lift on the quartic shell. It
does not assert that one primitive substrate tick equals `D_4`.

The symbol `u` here is the signed self-dual energy coordinate. It is not the
CM-curve coordinate `u=x^{-2}` used in FTD-0827.

## 4. Physical time is the nonuniform traversal measure

Differentiate (10) along (9). On every open quadrant, with continuous
extension at `u=0`,

\[
 \frac{du}{ds}=2\sqrt{|u|}\,y,
 \qquad
 \frac{dy}{ds}=-2\sqrt{|u|}\,u.                \tag{16}
\]

Equivalently,

\[
 \frac{d}{ds}\begin{pmatrix}u\\y\end{pmatrix}
 =2\sqrt{|u|}\,
 \mathcal J\begin{pmatrix}u\\y\end{pmatrix}.\tag{17}
\]

Write `u=cos(theta)`, `y=sin(theta)` on the unit circle. The oriented angular
velocity is

\[
 \boxed{\frac{d\theta}{ds}=-2\sqrt{|u|}
 =-2\sqrt{|\cos\theta|}.}                      \tag{18}
\]

The energy geometry is self-dual, but the physical time measure is not
uniform around it. The flow slows at the two pure-kinetic crossings `u=0`.
The slowdown is integrable:

\[
 \begin{aligned}
 s_T
 &=\frac12\int_0^{2\pi}
   \frac{d\theta}{\sqrt{|\cos\theta|}}\\
 &=2\int_0^{\pi/2}\cos^{-1/2}\theta\,d\theta\\
 &=B\!\left(\frac14,\frac12\right)\\
 &=\sqrt\pi\,
   \frac{\Gamma(1/4)}{\Gamma(3/4)}\\
 &=\boxed{\sqrt\pi\,G^*.}                     \tag{19}
 \end{aligned}
\]

Restoring dimensions with (8) gives

\[
 \boxed{
 TA=\sqrt\pi\,G^*\sqrt{\frac{m}{2\lambda}}}.
                                                            \tag{20}
\]

Equation (19) supplies a minimal interpretation of `G*`: it is not the radius
or energy of the self-dual circle. It is the total physical-time weight of
one oriented traversal after the nonlinear quartic lift.

## 5. Important regularity boundary

The map `x -> x|x|` is a homeomorphism but not a local diffeomorphism at
`x=0`. Its derivative vanishes there. Consequently the induced vector field
(17) is non-Lipschitz at the pure-kinetic crossings and appears instantaneously
stationary in `(u,y)` coordinates even though the physical `(x,y)` trajectory
passes through.

Therefore:

- the self-dual circle is an exact energy-coordinate representation;
- it is not a globally regular canonical coordinate chart;
- the singular Jacobian is precisely what produces the integrable `G*`
  traversal weight; and
- no production update rule may be inferred merely by iterating the rigid
  quarter-turn `J`.

## 6. Minimal imposed stabilizer and loss ledger

The exact bilateral kernel is neutrally stable. As a reference realization
only, let

\[
 X_{n+1}=g(\mathcal E_n)\mathcal JX_n,
 \qquad
 g(\mathcal E)=1+\eta(1-\mathcal E).            \tag{21}
\]

Then

\[
 \mathcal E_{n+1}
 =\mathcal E_n[1+\eta(1-\mathcal E_n)]^2,       \tag{22}
\]

and the linearized transverse multiplier at the unit shell is

\[
 \left.\frac{d\mathcal E_{n+1}}{d\mathcal E_n}
 \right|_{\mathcal E=1}
 =1-2\eta.                                      \tag{23}
\]

Thus `0<eta<1` gives local radial stability. This is not a global stability
theorem and `eta` is imposed.

Introduce an environmental energy account `B`:

\[
 B_{n+1}=B_n+\mathcal E_n-\mathcal E_{n+1}.     \tag{24}
\]

Then exactly

\[
 \mathcal E_{n+1}+B_{n+1}=\mathcal E_n+B_n.    \tag{25}
\]

This is the minimal energy ledger for a self-correcting recursive core. It
does not supply a positive reservoir, reversible information ledger, or
native controller. Those remain physical construction requirements.

## 7. What is established and what remains open

### Established conditionally

1. The two-channel quarter-turn is the minimal oriented order-four recursion.
2. Its quadratic energy is self-dual and its symmetric square loses direction.
3. The quartic shell is exactly its nonlinear signed-energy lift.
4. Quartic Hamiltonian time is a nonuniform circle traversal.
5. The traversal integrates exactly to `sqrt(pi) G*`.
6. A declared radial repair can be locally stable while closing an explicit
   environment energy account.

### Still open

1. Which production substrate degrees of freedom realize the two channels?
2. What native symmetry makes the quadratic detuning duality-odd and selects
   the quartic critical surface?
3. What local dynamics supply the radial repair and where is their positive
   energy reservoir?
4. Can the recursive core be bounded, autonomous, and robust under held-out
   perturbations?
5. Does any biological bilateral system instantiate this exact reduced
   architecture? That is an empirical neuroscience question, not answered by
   the mathematical analogy.
6. Does a substrate operation realize the conductor-32 Frobenius calendar?
   FTD-0836 does not identify primes with ticks.

### Successor adjudication — FTD-0838

FTD-0838 tests the first three native-dynamics gaps against the frozen
production core. It proves at that source scope that the current L/R fields do
not supply an oriented quarter-turn, their smooth fixed-state branches do not
contain a quartic restorer, and homogeneous damping does not select a positive
shell. It also proves that primitive ternarity does not itself realize
`u=x|x|`, because `s|s|=s` on every primitive state. The minimum conditional
repair requires an oriented phase pair, a contextual constituent-pair closure,
an energy bath, and a cadence map. Thus the coordinate theorem above survives
unchanged while its production realization is sharpened from a generic open
question to an explicit four-interface construction debt.

## 8. Certificate and provenance

FTD-0835 was the first locked certificate. It returned `16/17` because one
SymPy check compared equivalent factorizations by structural equality. Under
its frozen outcome rules it books no theorem.

FTD-0836 preregistered a tooling-only repair: exact simplification of that
one equality and a fail-closed terminal verdict. The locked successor returns

```text
FTD-0836 bilateral self-dual quartic clock: 17/17 PASS
BILATERAL_SELF_DUAL_QUARTIC_CLOCK_COORDINATE_THEOREM
QUARTIC_HAMILTONIAN_STATUS=SELECTED_INPUT
RADIAL_STABILIZER_STATUS=IMPOSED_REFERENCE_WITH_EXPLICIT_LEDGER
NATIVE_SUBSTRATE_REALIZATION=OPEN
```

No numerical search, fitted tolerance, production change, or neuroscience
data enters the certificate.
