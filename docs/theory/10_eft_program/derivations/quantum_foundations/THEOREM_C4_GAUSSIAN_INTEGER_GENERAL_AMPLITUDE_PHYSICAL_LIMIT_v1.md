# C4 Gaussian-integer general-amplitude physical limit v1

**Date:** 2026-08-24
**Status:** **[THEOREM — EXACT C4 REPRESENTATION OF EVERY GAUSSIAN-INTEGER RESPONSE]** +
**[THEOREM, CONDITIONAL — PREPARED PHYSICAL GAUSS-EVENT FREQUENCIES ARE DENSE IN EVERY FINITE COMPLEX BORN SIMPLEX]** +
**[THEOREM — EXPLICIT FINITE-RESOLUTION ERROR AND RESOURCE PRICE]** +
**[BOUNDARY — APPROXIMATING BANK IS PREPARED, NOT ACTION-GENERATED]** +
**[OPEN — NATIVE PREPARATION, INCOMPLETE WINDOWS, EXTERNALLY HERALDED TRIALS, SEQUENTIAL/MULTIPARTITE COMPOSITION AND NO-SIGNALLING]**
**Physical Born status:** the prepared finite C4/cotangent construction now has
a controlled general-complex-amplitude limit for every finite outcome set and
the later renewal successor supplies one reusable exclusive-event detector;
the general native physical Born rule remains open.
**Production status:** unchanged
**Ledger status:** no row minted

**Exact certificate:**
[proof_c4_gaussian_integer_general_amplitude_limit.py](../../../../../scripts/proofs/proof_c4_gaussian_integer_general_amplitude_limit.py)
performs 10,232 exact integer and rational checks. It exhausts bounded
Gaussian-integer responses, multi-outcome normalized event counts, the full
phase/polarity projection of the two-ternary-slot carrier, five normalized
complex rational preparation families through resolution 64, and the
charge-conjugation-compatible rounding rule. It performs no fit, stochastic
sampling, or numerical near-match search.

---

## 1. Input from the physical finite pushforward

The
[Born-to-cotangent physical pushforward](THEOREM_C4_BORN_TO_COTANGENT_PHYSICAL_GAUSS_EVENT_PUSHFORWARD_v1.md)
already proves the following finite statement. For a prepared response vector

\[
 Z=(Z_1,\ldots,Z_m)\in\mathbb Z[i]^m\setminus\{0\}, \tag{1}
\]

the reversible coprime construction produces exactly

\[
 M_o=|Z_o|^2                                       \tag{2}
\]

physical ternary manifestations carrying canonical cotangent Gauss-source
packets. Conditioning on a manifestation gives

\[
 f_o(Z)={|Z_o|^2\over\sum_r|Z_r|^2}.               \tag{3}
\]

The mixer does not calculate equation (3). It traverses every ordered pair of
the finite residual bank exactly once, and equation (2) is the resulting
physical event count.

What was previously missing was a controlled statement connecting the
Gaussian-integer domain of equation (1) to a general finite complex
preparation.

---

## 2. Every Gaussian integer is a finite C4 record multiset

Write

\[
 Z_o=A_o+iB_o,\qquad A_o,B_o\in\mathbb Z.           \tag{4}
\]

The minimum residual C4 counts realizing it are

\[
 \boxed{
 (N_{o,0},N_{o,1},N_{o,2},N_{o,3})
 =\bigl(A_o^+,B_o^+,(-A_o)^+,(-B_o)^+\bigr),}       \tag{5}
\]

where \(x^+=\max(x,0)\). Their coherent sum is exactly equation (4), their
residual-bank size is

\[
 \ell_o=|A_o|+|B_o|,                                \tag{6}
\]

and the phase-compatible ordered-pair count is exactly

\[
 A_o^2+B_o^2=|Z_o|^2.                               \tag{7}
\]

Thus no phase outside \(C_4=\{1,i,-1,-i\}\) is needed to represent a finite
Gaussian-integer response.

The revised
[ternary-square phase/polarity carrier](../common_action_mechanics_reciprocity/THEOREM_TERNARY_SQUARE_PHASE_POLARITY_CARRIER_AND_AUTONOMOUS_CROSSING_CLOCK_v1.md)
contains two retained polarity states above every C4 phase. The Born rail reads
the common phase coordinate while the polarity record remains in the complete
payload. Therefore equation (5) is compatible with charge-conjugate
manifestation without erasing or identifying the polarity histories.

---

## 3. Canonical finite-resolution approximants

Let

\[
 \psi=(\psi_1,\ldots,\psi_m)\in\mathbb C^m,
 \qquad\sum_o|\psi_o|^2=1.                          \tag{8}
\]

For a positive integer resolution \(N\), define the canonical
nearest-Gaussian-integer vector

\[
 \boxed{
 Z_o^{(N)}=
 \operatorname{nint}\!\bigl(N\operatorname{Re}\psi_o\bigr)
 +i\operatorname{nint}\!\bigl(N\operatorname{Im}\psi_o\bigr),} \tag{9}
\]

using nearest integer with half ties away from zero. This tie rule is fixed
before any outcome comparison and obeys

\[
 \operatorname{nint}(-x)=-\operatorname{nint}(x).  \tag{10}
\]

Equation (9) is an existence/certification map. It is **not** licensed as a
physical amplitude compiler. A native action must generate finite block sums
\(Z^{(N)}\) from its own source and apparatus history; it may not read a target
\(\psi\) and manufacture the required records.

Define the componentwise blocking error

\[
 e_o^{(N)}={Z_o^{(N)}\over N}-\psi_o.               \tag{11}
\]

Both real-coordinate rounding errors have magnitude at most \(1/(2N)\), so

\[
 |e_o^{(N)}|^2\le {1\over2N^2},
 \qquad
 \boxed{\|e^{(N)}\|_2^2\le {m\over2N^2}.}          \tag{12}
\]

Consequently \(Z^{(N)}\ne0\) whenever
\(N>\sqrt{m/2}\).

---

## 4. Physical general-amplitude limit

Let

\[
 q^{(N)}={Z^{(N)}\over\|Z^{(N)}\|_2}.              \tag{13}
\]

The exact physical event frequency from equation (3) is

\[
 \boxed{f_o^{(N)}=|q_o^{(N)}|^2.}                  \tag{14}
\]

Set \(\eta_N=\|e^{(N)}\|_2<1\). By the reverse triangle inequality,

\[
 \|\psi+e^{(N)}\|_2\ge1-\eta_N.                   \tag{15}
\]

Normalizing equation (11) therefore gives

\[
 \|q^{(N)}-\psi\|_2
 \le {2\eta_N\over1-\eta_N}.                       \tag{16}
\]

For normalized complex vectors \(q,\psi\), Cauchy--Schwarz gives

\[
 {1\over2}\sum_o\left||q_o|^2-|\psi_o|^2\right|
 \le\|q-\psi\|_2.                                  \tag{17}
\]

Combining equations (12), (16), and (17),

\[
 \boxed{
 d_{\rm TV}\!\left(f^{(N)},|\psi|^2\right)
 \le {2\eta_N\over1-\eta_N},
 \qquad
 \eta_N\le{\sqrt{m/2}\over N}.}                  \tag{18}
\]

Hence

\[
 \boxed{
 \lim_{N\to\infty}f_o^{(N)}=|\psi_o|^2
 \quad\text{for every }o.}                         \tag{19}
\]

More operationally, for any \(0<\varepsilon<1\), it suffices to choose a
finite integer resolution satisfying

\[
 N\ge
 \left\lceil
 \sqrt{m/2}\,{2+\varepsilon\over\varepsilon}
 \right\rceil                                      \tag{20}
\]

to guarantee total-variation error at most \(\varepsilon\).

Equation (19) is a conditional physical limit because every
\(f^{(N)}\) is already a count of finite manifested Gauss events. It does not
make a continuum wavefunction ontically primitive.

---

## 5. Exact finite resource price

Let

\[
 L_N=\sum_o\bigl(|\operatorname{Re}Z_o^{(N)}|
                 +|\operatorname{Im}Z_o^{(N)}|\bigr) \tag{21}
\]

be the minimum residual-bank size. Nearest rounding and Cauchy--Schwarz give

\[
 \boxed{L_N\le N\sqrt{2m}+m.}                      \tag{22}
\]

The existing coprime construction uses address periods \(L_N\) and
\(L_N+1\), so its complete address period is

\[
 \boxed{T_N=L_N(L_N+1)}                             \tag{23}
\]

ticks. The original physical tape realization used the same number of
independent detector cells. The later
[autonomous renewal detector](THEOREM_C4_AUTONOMOUS_REVERSIBLE_BORN_RENEWAL_DETECTOR_v1.md)
replaces those cells by one reusable nine-token detector/source payload. If

\[
 B_N=\sum_o|Z_o^{(N)}|^2,
\]

its exact renewal period is

\[
 T_N^{\rm renewal}=T_N+2B_N.                       \tag{23a}
\]

Thus amplitude resolution \(N\) retains an explicit \(O(mN^2)\) worst-case
address/time price, but no longer requires an \(O(mN^2)\) detector/source
tape. The prepared residual bank and physical address rings still cost finite
space depending on \(L_N\); the quadratic traversal cost is not hidden in a
real-valued amplitude register.

The later
[heralded fixed-window Poincare pushforward](THEOREM_C4_HERALDED_FIXED_WINDOW_BORN_POINCARE_PUSHFORWARD_v1.md)
assigns one isolated source herald to one exclusive event at a common
$T_N$-tick endpoint. On any prepared bank, every complete $B_N$-trial section
has the exact finite distribution in equation (17), and an arbitrary
$K$-trial window differs from it in total variation by less than $B_N/K$.
This closes the prepared finite-window/entry-phase debt, not the formation of
the approximating bank, herald apparatus, or multipartite composition.

FTD's undefined-boundary ontology does not require an actually completed
\(N=\infty\) apparatus. Equations (20)--(23a) are an epsilon--finite-resource
statement: every declared finite accuracy uses a declared finite bank and
renewal time. Whether physical preparation can construct those resources
causally is still open.

---

## 6. Covariance and scope

For every finite \(N\), multiplication of all \(Z_o^{(N)}\) by a common C4
phase permutes the four record labels and leaves equations (2)--(3) exact.
The nearest-grid construction is not exactly covariant under an arbitrary
continuous global phase at finite \(N\); its error bound is uniform and the
continuous phase invariance is recovered only in equation (19). No native
continuous U(1) symmetry is claimed here.

The theorem covers arbitrary normalized pure response vectors with a finite
number of outcome ports. It does not by itself cover:

- action-generated preparation of the Gaussian-integer sequence;
- mixed-state decomposition independence;
- continuous outcome spaces;
- concurrent finite windows with overlapping source traffic;
- macroscopic amplification of the one field-bearing event per selected
  isolated trial;
- sequential measurement and detector refractory/reset behavior; or
- multipartite contextual composition and operational no-signalling.

---

## 7. What debt is closed

The previous statement that irrational or continuously varying amplitudes had
no controlled C4 limit is now retired on the **prepared finite-outcome
representation** branch. The exact chain is

\[
 \text{finite C4 histories}
 \to Z^{(N)}\in\mathbb Z[i]^m
 \to M_o^{(N)}=|Z_o^{(N)}|^2
 \to f^{(N)}
 \xrightarrow[\text{finite-resource bound}]{N\uparrow}
 |\psi|^2.                                          \tag{24}
\]

This is not yet the general physical Born rule. The most important remaining
debt has moved from **amplitude representability** to **native preparation,
externally heralded trial coupling, and causal composition**.

---

## 8. Next locked gate

The next transaction must generate, rather than receive, the finite residual
bank. Starting from a finite source/apparatus configuration, one common local
action must:

1. create and route the C4/polarity records whose block sums are
   \(Z^{(N)}\);
2. preserve the exact cancellation, coprime enumeration, detector
   actualization, and Gauss-source inverse already proved;
3. couple every externally heralded source preparation to exactly one renewal
   event without consulting equation (14) or postselecting on a click;
4. retain the finite-resolution law under incomplete mixer windows and reset;
   and
5. compose multipartite contexts with operational no-signalling.

Failure of native record preparation or heralded source--renewal coupling
leaves equation (24) a prepared physical approximation theorem, not a
Born-law derivation.
