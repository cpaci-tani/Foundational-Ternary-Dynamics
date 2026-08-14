# THEOREM — Signal-acknowledged two-stroke reset and smooth boundary v1

**Identifier:** `FTD-0869`  
**Status:** `[THEOREM — CONDITIONAL ON AN IMPOSED TWO-STROKE WAVEFORM AND
SELECTED NONSMOOTH RESET LAW]`  
**Certificate:** repaired FTD-0869 `44/44`; FTD-0868 remains
execution-invalid and aborted after C26  
**Date:** 2026-08-11

## 1. Result

The FTD-0867 one-shot handshake admits an exact recursively-ready reference
controller without an externally issued acknowledgement command. The
completed outgoing signal itself supplies the acknowledgement during a second
clock stroke. Exact finite-time reset cannot be supplied by a locally
Lipschitz autonomous attraction, but a selected cusp/dry-friction reset closes
in finite time with an explicit controller/bath ledger.

This is reference-controller closure. It is not native cubic production,
protected transport, a microscopic bath, or a `G*` gearbox.

## 2. Exchange stroke

Let `dot(theta)=omega>0` and partition one harmonic supercycle into exchange
and reset halves. Select

\[
 g_E(\theta)=
 \begin{cases}
 1-\cos(2\theta),&0\le\theta\le\pi,\\
 0,&\pi\le\theta\le2\pi.
 \end{cases}                                      \tag{1}
\]

Then `g_E(0)=g_E(pi)=g_E(2pi)=0` and

\[
 \int_0^\pi g_E(\theta)d\theta=\pi.              \tag{2}
\]

Use the **[IMPOSED reference controller Hamiltonian]**

\[
 H_E=\omega I+2\omega(I_c+I_r)
     +s^2\omega g_E(\theta)I_r.                  \tag{3}
\]

During the first half, the common mode winds by `2pi` and the relative mode by
one additional `pi`. Therefore

\[
 s=0:\ (M,D)\mapsto(M,D),
\]

\[
 s=\pm1:\ (M,D)\mapsto(D,M)                     \tag{4}
\]

exactly at `theta=pi`. During the second half both modes receive the same
additional `2pi`, so (4)'s midpoint values are unchanged at `theta=2pi`.

The clock action on the active stroke is

\[
 I(\theta)=I_0-s^2g_E(\theta)I_r.               \tag{5}
\]

For empty-signal emission `I_r=B/2`, hence

\[
 I_{\min}=I_0-B,
 \qquad I(\pi)=I(2\pi)=I_0.                     \tag{6}
\]

The compressed half-cycle exchange has a real cost: it requires

\[
 \boxed{I_0>B},                                  \tag{7}
\]

twice the active reserve decrement of FTD-0865's full-cycle exchange. Maximum
interaction energy and maximum reference-energy loan both equal `omega B`.

## 3. Signal completion is the acknowledgement

At `theta=pi`, define

\[
 a=s^2\,
   \mathbf1[|M|^2/2=0]\,
   \mathbf1[|D|^2/2>0].                          \tag{8}
\]

For the registered event, (4) gives `M=0`, `|D|^2/2=B>0`; thus `a=1`. For the
no-event state, `s=0,M=D=0`; thus `a=0`. A partial transfer with nonzero matter
does not acknowledge.

Equation (8) is local, deterministic, even under sign reversal, and target
blind. It reads neither the magnitude expected for `B`, a Born weight, a
measurement setting, a remote context, nor `G*`. The signal remains local
during the reset half and therefore acts as both event record and temporary
acknowledgement token. No extra acknowledgement bit is selected.

## 4. Smooth finite-time reset obstruction

Consider an autonomous ODE

\[
 \dot x=F(x),\qquad F(0)=0,                      \tag{9}
\]

with `F` locally Lipschitz. Suppose a solution beginning at `x_0!=0` reached
`x(T)=0` at finite time. Local existence and uniqueness for the initial-value
problem through `(T,0)` makes the identically-zero solution the unique solution
both immediately forward and backward from `T`. The incoming nonzero solution
would have to coincide with it, a contradiction.

Therefore:

\[
 \boxed{\text{locally Lipschitz autonomous attraction cannot reach an
 exact fixed-point reset in finite time}.}       \tag{10}
\]

A finite-time exact reset must use a nonsmooth/non-Lipschitz law, a timed
forcing switch, an explicit many-to-one quotient, or additional retained
controller state. Ordinary exponential damping is asymptotic only.

## 5. Selected finite-time reset

Let the acknowledged FTD-0848 latch begin at `x=sA`, `s=+/-1`. At the
midpoint gate zero, switch from

\[
 V_T(x)=\beta x^2(x^2-A^2)^2
\]

to the **[SELECTED nonsmooth reset potential]**

\[
 V_R(x)=\kappa|x|.                               \tag{11}
\]

Use the overdamped maximal-monotone inclusion

\[
 \gamma\dot x\in-\kappa\partial|x|.             \tag{12}
\]

With sticking at zero,

\[
 x(t)=s\max\left(A-\frac{\kappa t}{\gamma},0\right),
 \qquad T_R=\frac{\gamma A}{\kappa}.            \tag{13}
\]

The reset half lasts `pi/omega`, so exact compliance is

\[
 \boxed{\kappa\ge\frac{\gamma A\omega}{\pi}}.   \tag{14}
\]

Equality is the minimum force in this selected cusp class. At the endpoint,
`x=0`, the ternary basin record is exactly zero, and switching back to `V_T`
costs zero.

Switch-on raises system potential by `kappa A`; the controller reservoir loses
that amount. Reset exports

\[
 \Delta B_R=\int_0^{T_R}\frac{\kappa^2}{\gamma}dt
            =\kappa A                           \tag{15}
\]

to the scalar bath. Thus controller loss, reset-system return, and bath gain
close exactly. Equation (15) is not a microscopic thermal bath state and does
not imply a Landauer bound.

## 6. Output handoff and recursion

After reset, exchange the complete local signal with an initially empty output
port. The full-mode swap is symplectic, involutive, and action preserving:

\[
 (s,x,M,D,E)=(\pm1,\pm A,M_s,0,0)
 \longmapsto(0,0,0,0,M_s).                      \tag{16}
\]

The local latch, matter mode, and signal port are ready. The output retains

\[
 B=|E|^2/2,
 \qquad s=\operatorname{sign}(\beta\wedge E).   \tag{17}
\]

The next local cycle may begin only after a consumer accepts the output port.
Equation (16) is an exact reference handoff, not a derivation of protected
cubic propagation or arbitrary-load queuing.

## 7. Certificate and implementation boundary

The first FTD-0868 certificate passed 25 of the 26 checks it recorded, then
aborted constructing C27. Its two defects were verifier representations: a
relational/set comparison at C21 and direct integer conversion of a SymPy
Boolean at C27. FTD-0869 froze exactly those two repairs and passed inherited
`44/44`. The parent remains invalid.

The isolated public witness is implemented at:

- `engine/include/ftd/eft/signal_acknowledged_two_stroke_reset.h`
- `engine/src/eft/signal_acknowledged_two_stroke_reset.cpp`
- `engine/tests/test_signal_acknowledged_two_stroke_reset.cpp`

It evaluates exact endpoint maps and ledgers; it is not a numerical
subgradient integrator.

## 8. Epistemic accounting

- **[THEOREM, CONDITIONAL]** Equations (4)--(8) follow from the imposed
  two-stroke waveform and the prior selected latch/reference modes.
- **[THEOREM]** The locally Lipschitz autonomous finite-time reset obstruction
  (10).
- **[THEOREM, CONDITIONAL]** Equations (13)--(17) for the selected cusp reset
  and empty output port.
- **[IMPOSED]** The half-cycle waveform, `2omega` common winding, `omega`
  interaction coefficient, and phase partition.
- **[SELECTED]** The cusp reset potential, overdamped subgradient law, scalar
  controller reservoir, and scalar bath account.
- **[OPEN]** Native formation of the continuous latch/reference/modes.
- **[OPEN]** A microscopic bath and thermal reset analysis.
- **[OPEN]** Robustness under detuning, noise, non-minimum initial latch
  coordinate, overlapping events, and nonempty output backpressure.
- **[OPEN]** Protected cubic transport and production energy-current coupling.
- **[OPEN]** Dynamic compensation for the quartic-clock load dependence.
- **[OPEN]** Native `G*` synchronization, Born/Bell recovery, operational
  Lorentz hiding, and framework completeness.

## 9. Conclusion

The missing acknowledgement need not be a new hidden bit: the outgoing signal
already is the local evidence that the exchange completed. A second clock
stroke can use that signal to reset the latch and hand the event to an output
port. The irreducible cost is not logical acknowledgement; it is the physical
reset law and its energy/environment account. Exact finite-time reset exposes
the sharp choice between smooth asymptotic dynamics and a declared nonsmooth,
switched, or lossy mechanism.
