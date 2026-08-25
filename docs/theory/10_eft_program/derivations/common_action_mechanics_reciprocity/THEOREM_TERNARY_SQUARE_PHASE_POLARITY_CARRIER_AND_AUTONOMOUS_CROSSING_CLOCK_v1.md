# Ternary-square phase/polarity carrier and autonomous crossing clock v1

**Date:** 2026-08-24
**Status:** **[THEOREM — TWO TERNARY SLOTS ARE THE EXACT MINIMUM BLANK-PLUS-C4xZ2 CARRIER]** +
**[THEOREM — CHARGE-CONJUGATION-COVARIANT MANIFESTATION REQUIRES CARRIED POLARITY]** +
**[THEOREM — EXACT REVERSIBLE PHASE/POLARITY OWNERSHIP TRANSFER]** +
**[THEOREM, CONDITIONAL — CONTROLLER-FREE PERIOD-EIGHT CROSSING CLOCK]** +
**[SELECTION — ONE PHASE CROSSING IS THE LOCAL TRANSACTION SECTION]** +
**[OPEN — FORMATION, ROUTING, MICROSCOPIC ACTION SELECTION, BLOCKING, STABILITY, POLES, LENSING, GENERAL BORN, NATIVE ALPHA]**
**Production status:** unchanged
**Ledger status:** no row minted

**Exact certificate:**
[proof_ternary_square_phase_polarity_autonomous_clock.py](../../../../../scripts/proofs/proof_ternary_square_phase_polarity_autonomous_clock.py)
performs 9,804 exact finite-state and rational checks. It exhausts the complete
two-slot ternary alphabet, the full local endpoint/link/reserve state space,
all oriented C4 tokens, all C18 capacity dyads, and all capacity-permission
words through eight global ticks.

---

## 1. The missing orientation cannot be generated from a blank

The current phase-complete bond scope used the five-state one-hot alphabet

\[
 \mathcal C_4^0=\{(0,0),(1,0),(0,1),(-1,0),(0,-1)\},
\]

while the exact actualization vertex requires the complete token

\[
 \tau=(k,\epsilon),\qquad k\in\mathbb Z_4,
 \quad\epsilon\in\{-1,+1\}.                         \tag{1}
\]

The phase-only alphabet does not contain \(\epsilon\). This is not merely a
missing convention.

Let \(C\) be charge conjugation. If a deterministic local rule \(F\) is
charge-conjugation equivariant and its input \(X_0\) is fixed by \(C\), then

\[
 F(X_0)=F(CX_0)=CF(X_0).                             \tag{2}
\]

Thus the output is also fixed by \(C\). But the two oriented neutral
manifestations

\[
 (+1,-1),\qquad(-1,+1)                               \tag{3}
\]

are exchanged rather than fixed. Therefore a target-free equivariant rule
cannot choose either member of equation (3) from a completely charge-even
blank. The orientation must be supplied by a charge-odd incoming record,
boundary condition, or earlier history. Random choice would not remove this
type requirement; it would only hide the missing orientation in a stochastic
seed.

For the one-action program, the aligned repair is to **transport** polarity
with the same token that transports phase. The transaction does not create
the sign and later erase its origin.

---

## 2. Exact minimal carrier

One blank plus two polarities at four phases requires

\[
 1+2\cdot4=9                                         \tag{4}
\]

states. One ternary register has only three states; two have exactly nine.
Hence the exact minimum product of ternary registers is

\[
 \boxed{\mathcal A_9=\{-1,0,+1\}^2
 =\{0\}\sqcup(\mathbb Z_4\times\mathbb Z_2).}       \tag{5}
\]

Write a register state as \((u,v)\) and define

\[
 r=u^2+v^2,\qquad d=u^2v^2.                          \tag{6}
\]

The exact polynomial readouts are

\[
 \boxed{n=r-d,\qquad c=1-n,\qquad
 \epsilon=r-3d.}                                    \tag{7}
\]

Here \(n\) is token occupation, \(c\) is residual capacity, and
\(\epsilon\) is zero only on the blank. On the four axis states
\(\epsilon=+1\); on the four diagonal states \(\epsilon=-1\).

The common C4 phase coordinates, independent of polarity shell, are

\[
 a=r-2d,
\]

\[
 \boxed{
 U=au+{d\over2}(u+v),\qquad
 V=av+{d\over2}(v-u).}                              \tag{8}
\]

For every nonblank state, \((U,V)\) is exactly one of

\[
 (1,0),(0,1),(-1,0),(0,-1).                         \tag{9}
\]

Although equation (8) contains halves, it is integer-valued on the ternary
square. Equations (7)--(8) are readouts of a finite alphabet, not continuous
microscopic amplitudes.

---

## 3. Clock and charge-conjugation actions

Choose one clock orientation

\[
 R(u,v)=(-v,u),\qquad R^4=1.                         \tag{10}
\]

Its inverse is the orientation used in equation (6) of the phase-complete
scope; the choice only reverses the phase-index convention. The action
preserves \(n,c,\epsilon\) and advances \((U,V)\) by one C4 step.

Define charge conjugation by exchanging the axis and diagonal state having the
same phase. In polynomial form,

\[
 \boxed{
 C(u,v)=a(u-v,u+v)
 +{d\over2}(u+v,v-u).}                              \tag{11}
\]

On \(\mathcal A_9\),

\[
 C^2=1,\qquad CR=RC,\qquad
 n(Cz)=n(z),\quad (U,V)(Cz)=(U,V)(z),
 \quad\epsilon(Cz)=-\epsilon(z).                    \tag{12}
\]

Thus phase and manifestation polarity are independent finite readouts of the
same two ternary slots.

---

## 4. Endogenous ownership transaction

Let \(\lambda\in\mathcal A_9\) be a bond record and
\(\rho\in\mathcal A_9\) a locally adjacent owned reserve record. The exact
transfer is

\[
 \boxed{
 (0,0;\lambda=0,\rho=z)
 \longleftrightarrow
 (\epsilon(z),-\epsilon(z);\lambda=z,\rho=0),
 \qquad z\ne0.}                                     \tag{13}
\]

All other local states are fixed. Equation (13):

1. is an involution;
2. preserves net ternary charge;
3. preserves exactly one complete phase/polarity token;
4. debits bond capacity when the token becomes manifest;
5. commutes with both \(R\) and \(C\); and
6. fails closed under missing reserve, occupied link, or inconsistent endpoint
   states.

The certificate checks equation (13) on all
\(3^2\times9\times9=729\) endpoint/link/reserve configurations. No separate
orientation register or event bit is required once local token ownership and
availability are part of the state. A contextual apparatus still has to route
the correct reserve token to the correct bond; equation (13) does not build
that router.

---

## 5. Controller-free crossing clock

Choose one C4 phase as the local crossing section, say \(k=0\). Let
\(A_0\) apply equation (13) only when the owned token is at that phase and let

\[
 \boxed{F=R\,A_0.}                                  \tag{14}
\]

Both factors are finite local permutations, so \(F\) is reversible. On every
valid one-token ownership state,

\[
 \boxed{F^4=A,\qquad F^8=1,}                        \tag{15}
\]

where \(A\) is the ungated ownership involution. No positive power below
eight returns the complete state. During one orbit, the token is reserve-owned
for four consecutive phases and bond-owned for four. The previously required
persistent bright controller is absent: after preparation, the token's own
phase and local availability generate the recurrence.

The selected phase is a clock-section choice, not a target outcome. The four
possible sections form a C4-covariant family,

\[
 R A_k R^{-1}=A_{k+1}.                               \tag{16}
\]

Therefore choosing \(k=0\) fixes a local time origin; it does not select a
manifestation polarity or measurement outcome.

---

## 6. Exact proto-matter, clock, and capacity ledger

Across the eight states of equation (15):

\[
 \sum_{\rm manifest}U=
 \sum_{\rm manifest}V=
 \sum_{\rm manifest}\epsilon U=
 \sum_{\rm manifest}\epsilon V=0,                  \tag{17}
\]

\[
 \left\langle s_L^2+s_R^2\right\rangle=1.           \tag{18}
\]

For every normalized C18 line dyad \(M=dd^T\), half the orbit carries blank
capacity \(M/9\) and half carries occupied capacity \(M/18\). Hence

\[
 \boxed{\langle K\rangle={M\over12},\qquad
 \langle K\rangle-K_{\rm blank}=-{M\over36}.}       \tag{19}
\]

This reproduces the mean capacity deficit of the earlier prepared four-tick
clock while removing its external bright controller. The orbit is still only
a localized **proto-matter clock**: its initial reserve token and bond are
prepared, and no formation basin, spatial binding, perturbative stability,
translation, or physical mass has been derived.

If an external capacity/backpressure permission \(g_n\in\{0,1\}\) admits or
stalls the whole local tick, then for every finite permission word

\[
 X_N=F^{\tau_N}X_0,
 \qquad \tau_N=\sum_{n<N}g_n.                       \tag{20}
\]

The certificate exhausts all permission words through eight global ticks.
Equation (20) gives a global-tick/local-clock split, but a gravitational
time-dilation claim still requires the common capacity dynamics to generate
\(g_n\) and to modify electromagnetic propagation through the same carrier.

---

## 7. Consequence for the one-action program

The minimum candidate transaction state should no longer be described as a
five-state C4-plus-blank record plus an unexplained \(\epsilon\). The exact
finite carrier is the full two-slot ternary square:

\[
 \text{two ternary slots}
 \longrightarrow
 \begin{cases}
 \text{blank capacity},\\
 \text{C4 clock phase},\\
 \text{manifestation polarity},\\
 \text{charge-conjugate partner},\\
 \text{reversible ownership history}.
 \end{cases}                                        \tag{21}
\]

Together with the shared-moment source theorem, one transfer of equation (13)
still has the exact block readings

\[
 \text{manifestation}
 \leftrightarrow
 \text{relative-vector source}
 \leftrightarrow
 \text{common tensor/capacity source}.              \tag{22}
\]

Repeated under equation (14), it also gives a localized clock and persistent
mean capacity deficit. This closes a state-type mismatch between the
manifestation, clock, and source constructions without adding a new register.

It does **not** yet close the unified-action objective. The map is a selected
finite transfer rule, not a non-tautological variational derivation; the
reserve must still be formed and routed; and the recent exact collision result
shows that the current two-record EM-plus-tensor lift has neither generic
Maxwell isotropy nor a propagating tensor pole.

---

## 8. Next locked gate

Use \(\mathcal A_9\), not \(\mathcal C_4^0\), in the next microscopic
blocking attempt. The collision/action must:

1. transport both phase and polarity without changing either payload;
2. make local reserve arrival and capacity/backpressure the physical event
   condition rather than reading an abstract compatibility bit;
3. produce equation (13) and its inverse on the registered crossing section;
4. preserve the autonomous recurrence (15) inside a finite bound complex;
5. derive the common/relative quadratic response from finite histories; and
6. retain an electromagnetic pole while generating tensor-capacity transport
   beyond the closed two-record dyad route.

Only after those gates pass are a physical Born basin, lensing fixture, or
native-alpha source-response measurement authorized.

The later
[common-admission clock/Maxwell theorem](THEOREM_COMMON_ADMISSION_CLOCK_MAXWELL_AND_SPATIAL_LENSING_PRICE_v1.md)
extends equation (20) to the cotangent light cone. If the same retained
permission history gates both the material clock and the complete Maxwell
advance, their rates scale by the same admission fraction and the weak wave
coefficient obeys $a_0=a_t$. This supplies only the temporal/1911 lensing
class when $a_t=a_m$; an independently derived spatial Hodge response is
still required.

The subsequent
[A9/cotangent no-spare-scalar theorem](THEOREM_A9_COTANGENT_NO_SPARE_SCALAR_PERMISSION_AND_DUAL_COPY_PRICE_v1.md)
shows that this clock cannot generate that second permission internally. Link
and reserve capacities are complementary on its one-token orbit, and a
state-only partial successor is reversible only for constant all/none
admission. A dual-complex A9 ownership copy is the minimum repair that reuses
the same alphabet while providing an independent scalar capacity coordinate.

The next
[dual-A9 skew generator](THEOREM_DUAL_A9_SKEW_CAPACITY_CLOCK_GENERATOR_AND_HOMOGENEOUS_FACTOR_PASS_v1.md)
uses that second copy as a retained controller. Its capacity conditionally
advances the receiving A9 recurrence, giving one exact local period-eight
clock per sixteen global ticks on every orbit without an external permission
word. The fixed half-rate reference is not yet a source-dependent dilation
law or stable material body.
