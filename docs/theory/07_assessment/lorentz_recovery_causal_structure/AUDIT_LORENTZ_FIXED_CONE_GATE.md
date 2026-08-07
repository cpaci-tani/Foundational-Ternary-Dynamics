# Audit — Fixed-Cone Lorentz Recovery Gate

**Registry:** FTD-0409  
**Status:** `[SCOPED NO-GO — scalar periods 2–3 and minimal positive-Hermitian auxiliary]` + `[CONSTRUCTIVE TARGET — stable period-four trace]` + `[OPEN — realizable period-four/multi-state transfer]`  
**Verdict:** `MINIMAL-FIXED-CONE-CLASSES-CLOSED`  
**Exact verifier:** [`proof_lorentz_fixed_cone_gate.py`](../../../../scripts/proofs/proof_lorentz_fixed_cone_gate.py) (25/25)

---

## 0. Result

Freeze the common-cone target before choosing an update:

\[
c_{\rm target}^2=\frac13,
\qquad
\theta^2=\frac13S_2+O(|q|^6),
\]

with the production spatial symbol `M18`, one nearest-Moore read per tick,
deterministic reversibility, and spectral stability for
`0 <= M18 <= 16/3`.

Three minimal architecture classes fail exactly:

1. scalar period-two kick sequences;
2. scalar period-three kick sequences;
3. one real positive-gap Hermitian auxiliary oscillator coupled through a
   stiffness matrix linear in `M18`.

A fourth class, scalar positive-weight one-step link averaging on Moore
displacements, also fails the required fourth-moment identity at `c²=1/3`.

The result does **not** prove that P4-compatible fixed-cone improvement is
impossible. An exact stable degree-four Floquet trace with the required
infrared germ exists. The specific endpoint-saturating `c3=0` witness cannot be
factored into four real scalar kicks, but general period-four scalar traces and
multi-state paraunitary transfers remain open.

No engine coefficient changes under FTD-0409. The FTD-0408 `1/sqrt(13)`
prototype remains a default-off LR-1 witness, not the common-cone solution.

---

## 1. Period two fails at the fixed cone

For alternating kicks `kappa0,kappa1`, define

\[
A=\frac{\kappa_0+\kappa_1}{2},\qquad
B=\frac{\kappa_0\kappa_1}{4}.
\]

The exact two-tick pole is

\[
\sin^2\theta=AM-BM^2.
\]

The `q4` cancellation condition from FTD-0408 is

\[
B=\frac{A(4A-1)}{12}.
\]

At the fixed cone `A=1/3`, this gives `B=1/108`, hence

\[
X(M)=\frac{M}{3}-\frac{M^2}{108}.
\]

At the exact production endpoint,

\[
X(16/3)=\frac{368}{243}>1.
\]

The Floquet multipliers therefore leave the unit circle. No scalar period-two
sequence can retain the live `1/sqrt(3)` cone and pass LR-1. FTD-0408 becomes
stable only by reducing `A` to `1/13` and using an anti-kick.

---

## 2. Period three fails independently of kick reality

For three kicks `k1,k2,k3`, let

\[
s_1=\sum_i k_i,\qquad
s_2=\sum_{i<j}k_ik_j,\qquad
p=k_1k_2k_3.
\]

The exact three-tick discriminant is

\[
C_3(M)=\cos(3\theta)
=1-\frac32s_1M+s_2M^2-\frac12pM^3.
\]

Matching `c²=1/3` gives `s1=1`. Expanding the phase shows that exact `q4`
cancellation requires

\[
\sum_i k_i^2=\frac12,
\qquad
s_2=\frac14.
\]

Thus every candidate has

\[
C_3(M)=1-\frac32M+\frac14M^2-\frac12pM^3.
\]

Full-band stability requires `-1 <= C3(M) <= 1`. Two points suffice to
contradict it:

\[
C_3(3)=-\frac54-\frac{27}{2}p\ge-1
\quad\Longrightarrow\quad
p\le-\frac1{54},
\]

while

\[
C_3(16/3)=\frac19-\frac{2048}{27}p\le1
\quad\Longrightarrow\quad
p\ge-\frac3{256}.
\]

But `-3/256 > -1/54`. The intervals are disjoint. This no-go does not assume
positive kicks and arises before imposing the separate real-kick discriminant
conditions.

---

## 3. Minimal positive-Hermitian auxiliary has the wrong sign

Consider one gapped auxiliary mode with the most direct real Hermitian
stiffness pencil linear in the local spatial symbol:

\[
K(M)=
\begin{pmatrix}
aM&gM\\
gM&\mu+cM
\end{pmatrix},\qquad \mu>0.
\]

The acoustic eigenvalue has expansion

\[
\lambda_{\rm ac}(M)=aM-\frac{g^2}{\mu}M^2+O(M^3).
\]

For the centered-time pole
`4 sin²(theta/2)=lambda_ac`, cancellation at `a=1/3` requires instead

\[
\lambda_{\rm ac}(M)=\frac13M+\frac1{54}M^2+O(M^3).
\]

The required curvature is positive; level repulsion from a real positive-gap
Hermitian auxiliary makes it nonpositive. This class can escape only by adding
an indefinite/tachyonic gap, an imaginary coupling, an explicit `M²` operator,
kinetic/time-derivative mixing, or more internal states. The first three merely
move the instability or P4 problem and are not accepted as recovery.

---

## 4. Scalar positive-link averaging also fails

For a scalar convex link average with Moore displacement
`d_i in {-1,0,1}`, the fourth and second coordinate moments are identical:

\[
\mathbb E[d_i^4]=\mathbb E[d_i^2].
\]

A pole with no quartic correction requires these moments to equal `c4` and
`c2`, respectively. At `c²=1/3` that demands simultaneously

\[
\mathbb E[d_i^2]=\frac13,
\qquad
\mathbb E[d_i^4]=\frac19,
\]

which is impossible. Internal phases or multiple components are therefore not
optional in a stream/collide construction at the fixed cone.

---

## 5. Stable period-four target exists

For a four-tick cell, the desired phase germ implies

\[
\cos(4\theta)
=1-\frac83M+\frac{26}{27}M^2+O(M^3).
\]

Set the cubic coefficient to zero and choose the quartic coefficient so the
upper production endpoint saturates `C4(16/3)=-1`:

\[
\boxed{
C_4^*(M)=1-\frac83M+\frac{26}{27}M^2
-\frac{1843}{98304}M^4
}.
\]

This is stable on the complete band. With `x=3M/16 in [0,1]`,

\[
1-C_4^*=\frac{2x}{243}
\left(1843x^3-3328x+1728\right),
\]

\[
1+C_4^*=\frac{2(1-x)}{243}
\left(1843x^3+1843x^2-1485x+243\right).
\]

Exact degree-three Bernstein certificates on eight rational subintervals give
minimum coefficients `167/384` and `2555/384`, respectively. Both factors are
strictly positive, so `-1 <= C4* <= 1`.

This matters: the fixed-cone germ itself is compatible with a stable
degree-four transfer polynomial. The obstruction is now realization by a
P4-local state update, not an abstract stability bound.

---

## 6. The natural `c3=0` target has no four-real-kick factorization

For four scalar kicks, group opposite temporal sites:

\[
a=k_1+k_3,\quad b=k_2+k_4,\quad
p=k_1k_3,\quad q=k_2k_4.
\]

The target's zero cubic coefficient requires `aq+bp=0`; its quartic
coefficient requires

\[
pq=-\frac{1843}{49152}=-K.
\]

Suppose `p<0<q`; the other sign choice is symmetric. The zero-cubic equation
forces `a,b>0`. Put `r=a/b=(-p)/q`. Reality of the positive-product quadratic
pair requires `q <= b²/4`. Since `a+b=4/3`,

\[
K=rq^2\le\frac{16r}{81(1+r)^4}le\frac1{48},
\]

where the last maximum occurs at `r=1/3`. But

\[
K=\frac{1843}{49152}>\frac1{48}
=\frac{1024}{49152}.
\]

Therefore this exact stable target cannot be produced by four real scalar
kicks. This is a result about the chosen `c3=0`, endpoint-saturating witness;
it is not a no-go for all period-four traces.

---

## 7. Next admissible construction

**FTD-0410 follow-on.** The Gauss AGM does not derive the frozen `1/sqrt(3)`
value from the live action. Its self-dual period-ratio reading instead points
conditionally to a unit cone, which improves the infrared pole. Markov's
endpoint bound excludes that cone for every full-band-stable finite scalar
kick period on `M18`; the fixed-cone work below therefore remains a
compatibility branch, while multi-state localization of a bounded unit-cone
spectral target is the primary AGM-motivated branch.

The fixed-cone search space is sharply bounded:

1. solve the full two-parameter period-four scalar realizability problem with
   an exact semialgebraic certificate, allowing nonzero cubic trace coefficient;
2. if empty, construct a minimum four-component paraunitary/real-symplectic
   transfer `U(q)=sum_d A_d exp(i q dot d)` with `d` in the Moore shell;
3. impose parity/time-reversal branchwise, not merely after averaging opposite
   chiralities;
4. require every physical gapless branch to satisfy
   `theta_a²=S2/3+O(q6)`, every other branch to remain gapped and stable, and
   the full determinant/metric form to be exactly preserved;
5. only then wire a new engine prototype.

No coefficient may be chosen from a low-momentum fit alone. Full-band
stability and branch positivity are simultaneous exact constraints.

---

## 8. Recovery status

| Gate | Status after FTD-0409 |
|---|---|
| Default production pole | `[FAIL at dimension six]` |
| FTD-0408 P4 tree improvement | `[PASS at speed 1/sqrt(13)]` |
| Fixed `1/sqrt(3)` scalar period two | `[CLOSED NO-GO]` |
| Fixed `1/sqrt(3)` scalar period three | `[CLOSED NO-GO]` |
| Minimal positive-Hermitian one-auxiliary stiffness | `[CLOSED NO-GO]` |
| Stable degree-four target polynomial | `[EXISTS — exact]` |
| Realizable general period-four or multi-state transfer | `[OPEN]` |
| LR-2 common cone | `[OPEN]` |
| LR-3 through LR-6 | `[OPEN]` |
