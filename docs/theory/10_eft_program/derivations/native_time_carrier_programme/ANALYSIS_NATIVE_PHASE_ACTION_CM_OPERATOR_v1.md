# Analysis — Native Phase/Action Carrier and CM Realization Operator v1

**Identifier:** `FTD-0826`  
**Protocol:**
[`PREREG_NATIVE_PHASE_ACTION_CM_OPERATOR_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_NATIVE_PHASE_ACTION_CM_OPERATOR_v1.md),
SHA-256
`8BE09323F54424C51EA96B2589D532559CC54C4656DE39DEE0626DD6C5EC09F5`  
**Status:** `[THEOREM — TARGET-BLIND NATIVE MODAL PHASE/ACTION]` +
`[SOURCE-FIXED RATE — CONDITIONAL ON SELECTED C_WAVE]` +
`[THEOREM — FIXED-CURVE CM REALIZATION OPERATOR]` +
`[CLOSED NEGATIVE — ONE REGISTERED SUBSTRATE OPERATOR DOES BOTH]` +
`[SELECTION BOUNDARY — RANK-TWO ORIENTATION/TWIST LIFT]`  
**Date:** 2026-08-10  
**Production status:** unchanged; the carrier interface is isolated under
`engine/include/ftd/eft/`

## 0. Result in one sentence

The production source-free `(J,W)` map already contains an exact,
target-blind action--angle carrier whose coupling and radians-per-tick rate are
fixed once the frozen production stencil and selected `C_WAVE` are fixed, and
the fixed CM curve `E:y^2=x^3-x` supplies a genuine compatible
archimedean/Hecke/Frobenius realization of `G*`; however, exact local-system
tests prove that none of the registered C18, equal-Moore C26, or BCC operators
identifies these two structures without selecting an additional rank-two
orientation/quadratic-twist lift.

The locked verdicts are:

```text
NATIVE_CARRIER_CM_OPERATOR_SPLIT
BCC_CM_STRUCTURAL_SELECTION_REQUIRED
```

This is a positive answer to the decomposed programme and a closed negative
to the stronger single-operator hypothesis. It does not establish a localized
maintained clock, a matter clock, an actualization gate, seconds per tick, or a
substrate derivation of the CM curve.

## 1. Native carrier theorem

For one nonzero eigenmode of the production spatial operator, put
`a=a_18(k)`. The source-free engine update is

\[
 W'=W-aJ,\qquad J'=J+W',
\]

or

\[
 \binom{J'}{W'}=
 U_a\binom{J}{W},\qquad
 U_a=\begin{pmatrix}1-a&1\\-a&1\end{pmatrix}.
\]

The determinant is one, and the elliptic band is exactly `0<a<4`. Define

\[
 \cos\theta_a=1-\frac a2,\qquad
 \sin\theta_a=\sqrt{a\left(1-\frac a4\right)},
\]

\[
 Q_a=\sqrt{\sin\theta_a}\,J,
 \qquad
 P_a=\frac{W-aJ/2}{\sqrt{\sin\theta_a}},
 \qquad
 Z_a=Q_a+iP_a.
\]

Direct substitution gives

\[
 \binom{Q_a'}{P_a'}=
 \begin{pmatrix}
 \cos\theta_a&\sin\theta_a\\
 -\sin\theta_a&\cos\theta_a
 \end{pmatrix}
 \binom{Q_a}{P_a},
 \qquad Z_a'=e^{-i\theta_a}Z_a.
\]

The chart is canonical, `{Q_a,P_a}=1`, and therefore

\[
 I_a=\frac{|Z_a|^2}{2}
 =\frac{W^2-aJW+aJ^2}{2\sin\theta_a}
\]

is positive and exactly invariant. No occurrence of `G*`, a desired period,
or an empirical frequency enters this construction. The complex number is a
coordinate on an already-present real symplectic pair, not an inserted
potentiality or Hilbert-space state.

The zero mode is deliberately excluded: it has translational/shear rather
than elliptic dynamics and no finite action--angle chart of this form.

## 2. Coupling and tick rate are fixed independently of `G*`

The production face-plus-edge stencil has the exact Fourier symbol

\[
 L_{18}(k)=\frac23
 (c_x+c_y+c_z+c_xc_y+c_yc_z+c_zc_x)-4,
 \qquad c_\mu=\cos k_\mu.
\]

Consequently

\[
 a_{18}(k)=-C_{\rm WAVE}^2L_{18}(k),\qquad
 \omega_{18}(k)=
 \arccos\!\left(1-\frac{a_{18}(k)}2\right)
 \quad\hbox{radians per primitive tick}.
\]

The exact spatial band maximum is `-L_18=16/3`. With the pre-existing engine
selection `C_WAVE=1/sqrt(3)`,

\[
 0<a_{18}(k)\le\frac{16}{9}<4
\]

for every nonzero production mode. Thus all such modes are elliptic and their
rates contain no adjustable parameter after the source is frozen.

The status distinctions are load-bearing:

| quantity | what fixes it | status |
|---|---|---|
| spatial coupling | frozen C18 face/edge weights | production-native |
| wave coefficient | `C_WAVE=1/sqrt(3)` | `[SELECTED]`, not P1--P5-derived |
| tick interval | one kick--drift update, `Delta n=1` | ontic global update |
| modal rate | `theta_18(k)` above | derived from the preceding three |
| seconds per tick | no source in this construction | `[OPEN]` calibration |

This is the requested independent fixation: the source and tick are read
before the CM curve or `G*` is consulted. It is not a claim that the selected
wave coefficient has itself been derived.

## 3. Isolated engine witness

The public reference implementation is
[`native_modal_phase_action.h`](../../../../../engine/include/ftd/eft/native_modal_phase_action.h).
It provides the exact one-tick map, canonical chart, action, phase, C18 symbol,
and source-fixed modal eigenvalue. It deliberately exports no `G*` field and
does not integrate with `Voxel`, `RenderBridge`, actualization, or the default
tick phases.

The focused regression
[`test_native_modal_phase_action.cpp`](../../../../../engine/tests/test_native_modal_phase_action.cpp)
checks:

- the exact C18 zero and band-edge symbols;
- rejection of non-elliptic eigenvalues;
- conjugacy of the engine map to a rigid rotation;
- one-tick and 10,000-tick action stability;
- quadratic amplitude scaling; and
- the phase step and rate in radians per primitive tick.

The canonical MSVC 14.44 build and focused CTest pass `1/1`.

## 4. Genuine global CM realization operator

Fix, before any prime data are inspected,

\[
 E/\mathbb Q:y^2=x^3-x,
 \qquad \omega_E=\frac{dx}{2y},
 \qquad M_E=H^1(E).
\]

The fixed-curve certificate verifies:

```text
Cremona/LMFDB       32a2 / 32.a2
conductor           32
j                   1728
minimal discriminant 64
rank                0
torsion             Z/2 x Z/2
real components     2
Kodaira type at 2   III
c_2                 2
analytic |Sha|      1
```

### 4.1 Archimedean realization

With the Neron differential above, the least positive real-cycle period and
the BSD real volume are different:

\[
 \Omega_{\min}=\varpi
 =\frac{\Gamma(1/4)^2}{2\sqrt{2\pi}},
 \qquad
 \Omega_{\rm BSD}=2\varpi.
\]

Hence

\[
 G^*=\frac{2\varpi}{\sqrt\pi},
 \qquad
 L(E,1)=\frac{\Omega_{\rm BSD}c_2}
 {|E(\mathbb Q)_{\rm tors}|^2}
 =\frac{\varpi}{4},
\]

and therefore the exact archimedean-to-Euler bridge is

\[
 \boxed{G^*=\frac{8L(E,1)}{\sqrt\pi}}.
\]

This corrects the older compensating pair of normalization errors in the
L-function note: the real-volume factor is two and `c_2=2`, not four.

### 4.2 Finite-prime realization

For each odd prime, Frobenius on `H^1(E)` has characteristic polynomial

\[
 P_p(T)=T^2-a_pT+p,
 \qquad a_p=p+1-\#E(\mathbb F_p),
\]

with companion representative

\[
 F_p=\begin{pmatrix}0&-p\\1&a_p\end{pmatrix}.
\]

The same coefficients are those of the weight-two newform

\[
 f(\tau)=\eta(4\tau)^2\eta(8\tau)^2,
\]

and its Mellin transform/Euler product gives `L(E,s)`. Thus this is one
compatible global operator system, not a post-hoc list of prime identities.

For inert primes `p=3 mod 4`, CM forces

\[
 a_p=0,\qquad F_p^2=-pI.
\]

After removing the radial scale,

\[
 R_p=F_p/\sqrt p,\qquad R_p^2=-I,\qquad R_p^4=I.
\]

All inert primes therefore carry the same exact quarter-turn while their
radial scale changes with `sqrt(p)`. For split primes `p=1 mod 4`, the fixed
primary Gaussian factor `u+iv` determines `a_p=2u`; the sign is not chosen
after inspection.

This realizes the desired finite-prime Frobenius dynamics. It remains an
arithmetic realization on `H^1(E)`, not an engine-time update.

## 5. Why the registered stencil operators cannot be that same operator

The single-operator gate asks for local-system agreement, not equality of one
special value.

| candidate | native carrier | CM/local-system gate | exact reason |
|---|---:|---:|---|
| production C18 | pass | fail | its order-four period operator has local exponents `{0,1/2,1,2}` at `z=1,-2,-3`, which admit no self-duality center |
| equal-Moore C26 | conditional modal pass, not production | fail | its exact minimal order-four factor fails the same necessary self-duality pairing at `w=0` and the physical boundary `w=1/27` |
| body-diagonal BCC | mathematical modal pass, zero production weight | rank-three CM only | its Green period is `Sym^2 H^1`; it is twist- and orientation-blind |

### 5.1 C18 obstruction

The C18 symbol contains face and edge terms but exactly zero cubic
`c_xc_yc_z` coefficient. Its exact order-four lattice-period operator is not
self-dual: at each of the genuine finite singularities `z=1,-2,-3`,

\[
 0+2\ne\frac12+1.
\]

It is therefore not an elliptic motive, an elliptic symmetric power, a K3
transcendental piece, or a rigid-Calabi--Yau/newform local system. This closes
the registered C18 CM-realization route exactly.

### 5.2 C26 obstruction

For equal weighting of all 26 neighbours,

\[
 A_{26}=\frac1{13}(c_x+c_y+c_z)
 +\frac2{13}(c_xc_y+c_yc_z+c_zc_x)
 +\frac4{13}c_xc_yc_z
 =\frac{\prod_\mu(1+2c_\mu)-1}{26}.
\]

Its Green series is a rational transform of the cube of the central
trinomial generating function. Exact Ore factorization selects a minimal
order-four right factor, but its indicial roots include

\[
 \{0,0,0,1\}\quad(w=0),
 \qquad
 \{0,1/2,1,2\}\quad(w=1/27),
\]

both incompatible with the necessary self-duality pairing. The richer cubic
term therefore does not repair the CM bridge.

### 5.3 BCC is the square, not the oriented carrier

The BCC Green function is

\[
 P_{\rm BCC}(t)={}_3F_2(1/2,1/2,1/2;1,1;t),
\]

the symmetric square of a rank-two hypergeometric system. At its boundary,

\[
 P_{\rm BCC}(1)
 =\left(\frac{2K(1/\sqrt2)}{\pi}\right)^2
 =\frac{G^{*2}}{2\pi}.
\]

The underlying rational model is naturally the quadratic twist
`E^(2):y^2=x^3-4x` (64a1), and

\[
 \operatorname{Sym}^2(E\otimes\chi)
 \cong\operatorname{Sym}^2(E).
\]

Thus the BCC object cannot select between `E` and a quadratic twist. At an
inert prime, the rank-two normalized spectrum `{i,-i}` has order four, while
the symmetric-square spectrum `{-1,1,-1}` has only order two. The square has
forgotten the clockwise/counterclockwise orientation required for a clock.

The Watson identity is therefore exact and structurally important, but taking
its square root and choosing a rank-two twist is an additional type.

## 6. Epistemic price and remaining open bridge

The campaign establishes three different statements at three different
altitudes:

1. `[THEOREM]`: the production free field contains a target-blind modal
   phase/action carrier.
2. `[THEOREM — arithmetic, fixed selected curve]`: `H^1(E)` supplies a genuine
   operator connecting the archimedean quartic period to all finite-prime
   Frobenius actions.
3. `[CLOSED NEGATIVE — registered candidates]`: no C18/C26/CBCC operator is
   both of the preceding objects.

Identifying the native modal carrier with the oriented rank-two CM motive
would require a new declared map

\[
 \mathcal L_{\rm CM}:\text{native modal phase bundle}
 \longrightarrow H^1(E),
\]

together with a rule that fixes the quadratic twist/orientation and proves
compatibility with the native tick. Until such a map is independently forced,
`mathcal L_CM` is a `[SELECTION]`, not a derivation. Its falsifier is an exact,
target-blind construction from the production variables and source operator
that reproduces the rank-two local Frobenius polynomials, including their
twist signs, without consulting `G*` or prime outcome targets.

The local physical-clock problem also remains open: one must localize or
maintain a packet, book controller work/dissipation, and show a stable clock
readout. Modal phase alone does not meet that gate.

### 6.1 Successor clarification — FTD-0827

FTD-0827 subsequently constructs the oriented rank-two map **for the selected
critical quartic clock**, not for the production C18 modal carrier. Its
dimensionless energy shell `C:y^2=1-x^4` maps exactly to `E:v^2=u^3-u` by

\[
 u=x^{-2},\qquad v=-yx^{-3},\qquad
 \frac{du}{2v}=\frac{dx}{y}.
\]

That result closes the mathematical gearbox conditional on the quartic
Hamiltonian. It does not alter this campaign's locked C18/C26/BCC verdicts,
and it does not supply native critical-clock maintenance. The open
`mathcal L_CM` statement above therefore remains controlling for the free
production modal bundle, while
[`DERIV_QUARTIC_CLOCK_CM_GEARBOX_v1.md`](DERIV_QUARTIC_CLOCK_CM_GEARBOX_v1.md)
controls the selected quartic-clock branch.

## 7. Reproducibility record

The exact certificates were run after the protocol lock:

```text
python scripts/proofs/proof_native_phase_action_cm_operator.py
  PASS 47/47

python scripts/proofs/explr_stencil18_selfduality_derived.py
  W_18 is NOT self-dual [THEOREM, exact]

wsl.exe -d Ubuntu-22.04 -- bash -lc
  "cd /mnt/c/Users/cpaci/Desktop/ftd &&
   sage scripts/proofs/proof_cm_realization_operator.sage"
  PASS 36/36

wsl.exe -d Ubuntu-22.04 -- bash -lc
  "cd /mnt/c/Users/cpaci/Desktop/ftd &&
   sage -python scripts/proofs/proof_moore26_operator.sage"
  exact factorization PASS;
  verdict MOORE26_NON_SELF_DUAL_CLOSED_FOR_CM_REALIZATION

engine/build_native.bat shell cmake --build engine/build --config Release
  --target test_native_modal_phase_action --parallel 24
  build PASS (MSVC 14.44)

engine/build_native.bat shell ctest --test-dir engine/build -C Release
  -R "^native_modal_phase_action$" --output-on-failure
  1/1 PASS
```

No stencil, coupling, curve, prime class, sign, or period normalization was
selected from a numerical near miss. The exact output of this campaign does
not change the production engine.

## 8. Completion boundary

This analysis closes the stated programme objective in its honest decomposed
form:

- target-blind native phase/action carrier: **established**;
- coupling and tick rate fixed independently of `G*`: **established,
  conditional on the already-selected `C_WAVE`**;
- genuine operator connecting the archimedean quartic period to finite-prime
  Frobenius dynamics: **established as the compatible `H^1(E)` realization**;
- one already-defined substrate operator doing all three: **closed negative
  for C18/C26/CBCC**;
- physical identification of the free native carrier with `H^1(E)`:
  **selected type still open**, not silently assumed;
- oriented CM identification of the selected critical quartic energy shell:
  **conditionally established by FTD-0827**, with native maintenance still
  open.

Nothing here licenses the broader claims that `G*` is global time, that inert
primes literally trigger engine events, or that the framework is complete.
