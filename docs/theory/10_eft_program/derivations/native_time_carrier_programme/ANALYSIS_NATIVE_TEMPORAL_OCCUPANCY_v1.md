# FTD-0772 — Native Temporal Occupancy v1

**Status:** `[THEOREM — CONDITIONAL, FIXED NATURAL COORDINATE]` +
`[LOCKED RETROSPECTIVE REANALYSIS — RECURRENCE UNQUALIFIED]` +
`[OPEN — AUTONOMOUS NATIVE CLOCK]`  
**Verdict:** `NATIVE_TEMPORAL_OCCUPANCY_RECURRENCE_UNQUALIFIED`  
**Protocol SHA256:**
`3E779CDFFDE5D17299921750A06E26B075000572CAB60E9DA8FBF154239CC41C`  
**Production impact:** none; FTD-0659 was not rerun or overwritten

## 1. Result in one sentence

The quartic temporal-occupancy formula and its fixed-coordinate converse are
exact, but the registered FTD-0659 native doublet does not supply a stationary
orbit to which that theorem can be applied: its normalized envelope falls
from the prepared turning amplitude to about `0.538` by the final locked
window, and every one of the `18` primary cells fails the pre-registered
stationarity and return-amplitude gates.

The immediate claim is therefore not “native dynamics prefer another power.”
It is:

> the current strongest native phase signal is a transient projected signal,
> not a qualified invariant-measure clock.

## 2. Exact temporal occupancy

Consider the selected mathematical comparison family

\[
H_m(q,p)=\frac{p^2}{2}+\frac{|q|^m}{2}=E,
\qquad m=2,4,6,\ldots,
\]

and define the positive turning amplitude and normalized coordinate by

\[
A=(2E)^{1/m},\qquad x=\frac qA.
\]

On either monotone branch,

\[
\dot q=\pm\sqrt{A^m-|q|^m}
=\pm A^{m/2}\sqrt{1-|x|^m},
\]

so

\[
\left|\frac{dt}{dx}\right|
=A^{1-m/2}\frac1{\sqrt{1-|x|^m}}.
\]

The full period is

\[
\begin{aligned}
T_m(E)
&=4A^{1-m/2}\int_0^1\frac{dx}{\sqrt{1-x^m}}\\
&=\frac{4}{m}A^{1-m/2}
B\!\left(\frac1m,\frac12\right).
\end{aligned}
\]

Each interior position is traversed twice per period. Dividing the two branch
contributions by `T_m` gives the normalized continuous-time coordinate law

\[
\boxed{
\rho_m(x)=
\frac{m}{2B(1/m,1/2)}
\frac1{\sqrt{1-|x|^m}},
\qquad -1<x<1.
}
\]

Amplitude and a constant rescaling of the time parameter cancel. For every
`r>-1`, the absolute moment is

\[
\boxed{
\left\langle |x|^r\right\rangle_m
=\frac{B((r+1)/m,1/2)}{B(1/m,1/2)}.
}
\]

For `m=4`,

\[
B\!\left(\frac14,\frac12\right)
=\sqrt\pi\,\frac{\Gamma(1/4)}{\Gamma(3/4)}
=\sqrt\pi G^*,
\]

and hence

\[
\boxed{
\rho_4(x)=\frac{2}{\sqrt\pi G^*\sqrt{1-x^4}}.
}
\]

The registered quartic checks are

\[
\boxed{
\mu_1=\frac{\sqrt\pi}{G^*},\qquad
\mu_2=\frac4{G^{*2}},\qquad
\mu_4=\frac13,
}
\]

with

\[
G_{\rm rms}=\frac2{\sqrt{\mu_2}},\qquad
G_{\rm abs}=\frac{\sqrt\pi}{\mu_1}.
\]

These are correlated consistency functionals of the same trajectory. Three
moments do not identify a distribution, so FTD-0772 also locks the full CDF

\[
F_m(x)=\frac12+\frac{\operatorname{sgn}(x)}2
I_{|x|^m}\!\left(\frac1m,\frac12\right)
\]

and quadratic/quartic/sextic controls.

## 3. Fixed-coordinate quartic occupancy characterization

### Theorem `[THEOREM — CONDITIONAL]`

Let `I` be a nonempty interval of positive turning amplitudes, and let `V` be
an even `C^2` potential with `V(0)=0` on the region swept by those amplitudes.
Assume a fixed unit-mass natural coordinate `q`, regular finite-period
oscillations traversing `[-A,A]`, and

\[
V(A)>V(q)\qquad (|q|<A).
\]

The amplitude-normalized continuous-time occupancy is `rho_4` for every
`A in I` if and only if there is one `lambda>0` such that

\[
V(q)=\lambda q^4
\]

throughout the swept region.

### Proof

For a unit-mass natural system, the normalized density has the form

\[
\rho_A(x)=
\frac{\sqrt2 A}{T_A\sqrt{V(A)-V(Ax)}}.
\]

Equality with a constant multiple of `(1-x^4)^(-1/2)` implies, for each
registered amplitude,

\[
V(A)-V(Ax)=c(A)(1-x^4).
\]

Set `x=0`. Since `V(0)=0`, `c(A)=V(A)`, so

\[
V(Ax)=V(A)x^4.
\]

For any two amplitudes in the interval, choose a nonzero `q` lying inside
both orbits and substitute `x=q/A`. This gives

\[
\frac{V(A_1)}{A_1^4}
=\frac{V(q)}{q^4}
=\frac{V(A_2)}{A_2^4}.
\]

The ratio is therefore a single positive constant `lambda`, proving
quarticity on the union of the sampled orbit interiors. The converse follows
by direct substitution into the energy quadrature, whose normalization is
`2/(sqrt(pi)G*)`. QED.

### Exact scope

This is a characterization in a **fixed natural coordinate**, not a
coordinate-free invariant of a periodic orbit. Under an odd increasing
nonlinear relabeling `y=f(x)`,

\[
\rho_y(y)=\frac{\rho_x(f^{-1}(y))}{f'(f^{-1}(y))},
\]

so a nonlinear observable can manufacture or erase the quartic profile. The
result survives constant time-unit changes and affine coordinate-unit
changes only. It also proves quarticity only where the registered amplitude
family travels; it says nothing about the potential outside that region.

Finally, a truly periodic `P`-tick native orbit has the atomic measure

\[
\mu_P=\frac1P\sum_{n=0}^{P-1}\delta_{x_n},
\]

not an absolutely continuous density. A native exact-density theorem would
require a separately derived within-tick flow, invariant-circle limit, or
equidistribution/refinement theorem. FTD-0772 is therefore a finite-tick
distributional pilot even before recurrence is considered.

## 4. Why the signed observer was fixed

FTD-0659 already records the doublet coordinates

\[
q=(q_6,q_7)
\]

for three polarizations prepared before evolution. FTD-0772 freezes

\[
Q_u(t)=u^Tq(t),\qquad
x(t)=\frac{Q_u(t)}{A_{\rm parent}},
\]

where `u` is the parent polarization and `A_parent` is the parent
`modal_amplitude` field. Under a basis rotation inside the degenerate
eigenspace, `q` and `u` co-transform, leaving `Q_u` invariant. Neither the
direction nor the amplitude is inferred from the waveform.

The radial alternative `sqrt(q_6^2+q_7^2)` is excluded. It folds the two
signed turning directions and can turn a circular or rotating doublet into a
constant envelope, violating the one-dimensional hypothesis of the theorem.

This choice was locked before occupancy inspection in
[`PREREG_NATIVE_TEMPORAL_OCCUPANCY_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_NATIVE_TEMPORAL_OCCUPANCY_v1.md).

## 5. Immutable corpus and execution

The engine was not rerun. The exact FTD-0659 parent artifacts were accepted
only after all three locked hashes matched:

| parent artifact | SHA256 |
|---|---|
| result JSON | `DB6CA66770812E4C8FC94411B109F23E424FFF1CE3173A5D16AB43B5949ACEEE` |
| arm CSV | `4F7D2E38B0FE4D6EF33F137E2AA753E4143B3AD541F6934CF39FD11772844941` |
| tick CSV | `4EF51456F161E6CD836518B72EBAACE4A5007F5EF5525E07CD097B343566634A` |

Schema, `74`-arm coverage, ticks `0..256`, finite nonzero records, two exact
zero controls, total-energy, inverse-recovery, common-action, support,
leakage, and phase gates all pass. The campaign therefore executed validly.

## 6. Locked result

There are `18` primary histories: two stored orientations, three prepared
polarizations, and three amplitudes, all at parent quadrature zero. The other
three quadratures are locked sign/time-origin controls.

| registered gate | passing cells | worst/representative result |
|---|---:|---:|
| at least 8 cycles | `18/18` | `44.5044` cycles |
| fixed-ray transverse fraction | `18/18` | `1.23e-13` maximum |
| normalized global bound | `18/18` | `1.00000000008` maximum `|x|` |
| parent execution/phase/support | `18/18` | pass |
| control-quadrature CDF | `18/18` | `0.03113` maximum |
| control-quadrature moments | `18/18` | `0.00230` maximum delta |
| amplitude invariance | `6/6` control sets | `0.00389` maximum CDF distance |
| orientation/polarization covariance | `3/3` control sets | `0.00389` maximum CDF distance |
| every window reaches `|x|>=0.85` | **`0/18`** | **`0.537876` final-window maximum** |
| window CDF stationarity | **`0/18`** | **`0.313953` maximum distance** |
| window moment stationarity | **`0/18`** | **`0.290896` maximum spread** |
| full quartic gate | **`0/18`** | fail |

One representative primary history shows the envelope loss directly:

| locked window | `max|x|` | `mu_1` | `mu_2` | `mu_4` |
|---|---:|---:|---:|---:|
| ticks `0..85` | `1.000000` | `0.557900` | `0.388882` | `0.234360` |
| ticks `86..171` | `0.721674` | `0.417895` | `0.215228` | `0.071090` |
| ticks `172..256` | `0.537876` | `0.277549` | `0.097987` | `0.015880` |

The pooled primary values are descriptive only:

\[
\mu_1=0.4183273,\qquad
\mu_2=0.2345617,\qquad
\mu_4=0.1074649.
\]

Their locked CDF distances are

\[
D_2=0.1892171,\qquad
D_4=0.1617787,\qquad
D_6=0.1451857.
\]

All `18` cells are nominally closest to `m=6`, but this has no power-law
interpretation. Every target family assumes a stationary orbit normalized by
its fixed turning amplitude; the measured envelope instead contracts through
the registered windows. Concentrating progressively near zero mechanically
moves the empirical distribution toward the more center-weighted controls.
It does not derive a sextic potential.

The parent harmonic-action diagnostic, `0.898691` maximum relative drift,
points in the same direction but was deliberately not reused as a quartic
gate. FTD-0772 measures the amplitude/stationarity failure directly.

The ignored versioned result artifacts have hashes:

| result artifact | SHA256 |
|---|---|
| `ftd_0772_native_temporal_occupancy_v1.json` | `FAD820D59EE5A0E7ED8FE22BB187FDD97544F085208219A56990E7ADDE86AD9B` |
| `ftd_0772_native_temporal_occupancy_cells_v1.csv` | `600A00611D81AC329612617F0E60206820C1670791AE4A20862221E34D6173A3` |

The implementation and certificates are independently hash-pinned:

| tracked artifact | SHA256 | result |
|---|---|---|
| `analyze_native_temporal_occupancy.py` | `01CFDA65A64F18E3FDED577BA2832F29A8F668521D422C3675C80E6328764C90` | locked analyzer executed |
| `proof_temporal_occupancy_characterization.py` | `98F2D9EC874F4981B901A1E6054B14CC6E017CFCBCEBCA7BE500ED2A18E6A4B1` | exact mathematics `45/45` |
| `proof_native_temporal_occupancy.py` | `8757D0950E414F9D5B11115960444B51DA74D7F787A718F8073F016BE2E77739` | independent corpus/result reconstruction `49/49` |

## 7. Structural cross-check near rest

The selected doublet has positive Hessian eigenvalue

\[
\lambda_1=0.75321764\ldots
\]

and the parent amplitudes are only `2e-6`, `4e-6`, and `8e-6` in maximum
constituent displacement. If a smooth one-coordinate natural closure exists
near the dressed rest state, its local normal form begins

\[
V(Q)=\frac12\lambda_1Q^2+O(Q^3),
\]

whereas an exact quartic family requires `V''(0)=0`. Thus an exact
amplitude-invariant quartic law extending toward this rest state is already
structurally incompatible with the registered positive stiffness. This is a
conditional normal-form statement, not a claim that the incomplete projected
coordinate is autonomous.

## 8. What is and is not learned

### Established

1. The beta-normalized occupancy and all registered quartic moments are exact
   consequences of the selected power-law Hamiltonian.
2. In a fixed natural coordinate, exact amplitude-invariant quartic occupancy
   characterizes a quartic potential on the swept region.
3. `G*` survives affine amplitude and constant time-rate normalization in
   that selected coordinate law.
4. The FTD-0659 signed fixed-ray projection is basis-covariant and remains
   exceptionally one-dimensional and phase-coherent.
5. It is not stationary at the prepared amplitude and therefore supplies no
   qualified invariant occupancy.

### Not established

1. `G*` is not a coordinate-free modulus of a periodic orbit.
2. The parent tick corpus does not produce an exact continuous density.
3. The FTD-0659 projection does not close as a one-dimensional natural
   Hamiltonian in the recorded data.
4. No native quartic potential, clock action, phase response, or neighbor
   coupling has been derived.
5. The quartic FTD-0770 clock remains an imposed/selected extension.

## 9. Consequence for phase and coupling

A native phase-response covector and reduced coupling law require a recurrent
autonomous object first. The current projection loses its orbit amplitude
while retaining a clean phase, so an infinitesimal perturbation cannot yet be
separated from ongoing internal matter--field transfer. Computing

\[
Z(\theta)=\nabla_X\theta,
\qquad
H_{vw}(\phi)=\frac1{2\pi}\int Z(\theta)\cdot
G_{vw}(X_0(\theta),X_0(\theta+\phi))\,d\theta
\]

at this stage would import the missing recurrent orbit and phase geometry.
The coupling derivation is therefore blocked honestly, not merely deferred
for more precision.

## 10. Next falsifier

Do not repeat the same deterministic `256`-tick FTD-0659 histories. The next
candidate must include the field/binding degrees of freedom that receive the
bare doublet action and must first pass:

1. complete-state recurrence and localization;
2. stable return amplitude over longer histories and multiple volumes;
3. a preregistered, state-functional signed scalar;
4. a natural-coordinate closure test for branch-, velocity-, history-, and
   hidden-state dependence; and only then
5. the same locked full-CDF/moment occupancy test.

Only after those gates pass is a native phase-response/coupling campaign
licensed. FTD-0772 therefore sharpens the goal: the missing object is not a
better statistic of the bare doublet, but an autonomous **coupled
matter--field recurrence**.
