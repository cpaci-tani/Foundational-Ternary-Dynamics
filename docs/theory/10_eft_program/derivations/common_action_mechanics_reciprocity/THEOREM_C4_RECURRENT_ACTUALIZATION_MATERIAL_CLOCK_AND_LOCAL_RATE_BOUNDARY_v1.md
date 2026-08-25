# C4 recurrent actualization material clock and local-rate boundary v1

**Date:** 2026-08-23
**Status:** **[THEOREM, CONDITIONAL — EXACT FOUR-ADMITTED-TICK RECURRENCE]** +
**[REFERENCE CONSTRUCTION — MINIMUM LOCALIZED NEUTRAL PROTO-MATTER CLOCK]** +
**[THEOREM — ZERO CYCLE PHASE MOMENTS AND NONZERO MEAN CAPACITY DEFICIT]** +
**[THEOREM, CONDITIONAL — GLOBAL TICK VERSUS CAPACITY-ADMITTED LOCAL TICK]** +
**[OPEN — AUTONOMOUS FORMATION/CONTROLLER, CAPACITY PERMISSION LAW, STABILITY, PHYSICAL TIME DILATION]**
**Production status:** unchanged
**Ledger status:** no row minted

**Exact certificate:**
[proof_c4_recurrent_actualization_material_clock.py](../../../../../scripts/proofs/proof_c4_recurrent_actualization_material_clock.py)
checks every C18 line, three apparatus ports, four initial phases, both
orientations, and every capacity-permission word through eight global ticks.
It performs 7,286 exact symbolic checks.

---

## 1. One transaction can recur

The
[physical detector gate](../quantum_foundations/THEOREM_C4_PHYSICAL_BORN_ACTUALIZATION_TAPE_v1.md)
uses a phase-compatible record pair as a nondestructive control on one owned
detector token. The
[shared source vertex](THEOREM_C18_ACTUALIZATION_SHARED_MOMENT_SOURCE_VERTEX_v1.md)
gives the exact manifestation and field-moment consequences of that ownership
transfer.

Hold one local controller pair in a bright relation and apply two operations
on every **admitted** local tick:

1. the reversible detector actualization involution $\mathfrak A$; and
2. one common C4 payload advance $R$.

Because actualization is C4-covariant,

\[
 R\mathfrak A=\mathfrak A R,
 \qquad
 \mathfrak A^2=1,
 \qquad
 R^4=1.                                             \tag{1}
\]

Define the local update

\[
 U=R\mathfrak A.                                    \tag{2}
\]

Then

\[
 \boxed{U^4=1.}                                     \tag{3}
\]

For the registered states, no smaller positive power returns the complete
phase and ownership state. The exact local period is four admitted ticks.

---

## 2. The four-state material-clock orbit

Starting with a reserve-owned token of phase $k$, the ownership sequence is

\[
 \begin{array}{c|c|c}
 \text{local tick}&\text{phase}&\text{ownership}\\ \hline
 0&k&\text{reserve/void endpoints}\\
 1&k+1&\text{manifested neutral endpoints}\\
 2&k+2&\text{reserve/void endpoints}\\
 3&k+3&\text{manifested neutral endpoints}\\
 4&k&\text{reserve/void endpoints}.
 \end{array}                                        \tag{4}
\]

The controller records rotate by the same C4 advance, so their equality and
bright compatibility persist. No signal record is consumed, and the detector
retains exactly one token throughout.

Equation (4) is a localized recurrent manifestation process. It is a
proto-matter clock rather than a derived particle: the bright controller and
initial detector cell are prepared inputs, and no formation basin or
perturbative stability has been proved.

---

## 3. Exact cycle moment ledger

Let $M=dd^T$ be the normalized dyad of the clock's C18 line. Evaluate the
relative vector doublet $(R_u,R_v)$, common tensor doublet $(Q,P)$, capacity
tensor $K$, and ternary endpoints at the four states after successive admitted
ticks.

The cycle sums obey

\[
 \sum_{n=1}^{4}R_u^{(n)}=
 \sum_{n=1}^{4}R_v^{(n)}=0,                         \tag{5}
\]

\[
 \sum_{n=1}^{4}Q^{(n)}=
 \sum_{n=1}^{4}P^{(n)}=0.                           \tag{6}
\]

The two manifested states have opposite C4 phases, so their oriented and
common phase moments cancel exactly. In contrast,

\[
 \boxed{\langle K\rangle_{m cycle}={M\over12}.}   \tag{7}
\]

The always-blank line has $K_{\rm blank}=M/9$, hence the recurrent clock has
the nonzero average capacity deficit

\[
 \boxed{
 \langle K\rangle_{m cycle}-K_{\rm blank}
 =-{M\over36}.}                                     \tag{8}
\]

At every tick the total ternary charge is zero, while the average manifested
activity is

\[
 \boxed{
 \left\langle s_L^2+s_R^2\right\rangle_{m cycle}=1.} \tag{9}
\]

Equations (5)--(9) give the minimum exact pattern expected of a rest-like
neutral recurrence: no secular oriented phase flux, no secular common phase
tensor, but persistent manifestation activity and common capacity occupancy.

Interpreting equation (8) as inertial or gravitational mass is a conjecture,
not a derivation. A force response, mobility, composite energy, and universal
tensor sourcing must still be recovered.

---

## 4. Global ticks and local admitted ticks

Let $g_n\in\{0,1\}$ be a local capacity/backpressure permission on global tick
$n$. Define

\[
 X_{n+1}=
 \begin{cases}
 UX_n,&g_n=1,\\
 X_n,&g_n=0.
 \end{cases}                                        \tag{10}
\]

For any finite permission word,

\[
 \boxed{
 X_N=U^{\tau_N}X_0,
 \qquad
 \tau_N=\sum_{n=0}^{N-1}g_n.}                      \tag{11}
\]

Global ticks continue whether or not the local transaction is admitted. The
material clock advances only $\tau_N$ times and displays

\[
 \tau_N\pmod4.                                      \tag{12}
\]

The certificate exhausts every permission word through length eight and
verifies that only the admitted-tick count matters, not the placement of
stalls.

Equation (11) is an exact architecture for local clock-rate variation on one
global update order. It is **not** yet gravitational time dilation because the
action has not derived $g_n$ from the common tensor/capacity field or shown the
required relativistic clock comparisons.

---

## 5. Unifying interpretation

The same finite detector token now participates in three registered roles:

\[
 \text{contextual detector event}
 \longleftrightarrow
 \text{ternary manifestation}
 \longleftrightarrow
 \text{localized C4 recurrence}.                   \tag{13}
\]

Its recurring ownership creates the mean capacity deficit in equation (8),
while the C4 payload supplies the clock hand. Thus matter, clock, measurement,
and the candidate common gravity source are no longer unrelated state types.
They are distinct regimes or block readings of the same token transaction.

This is the strongest current one-action synthesis, but it remains conditional
on a prepared persistent controller.

---

## 6. Remaining gates

A genuine material clock requires:

1. autonomous formation of the controller-plus-token complex;
2. perturbative and collision stability without a fixed external latch;
3. finite translation and recoil under the same scattering action;
4. a positive conserved composite energy and operational mass readout;
5. derivation of $g_n$ from local capacity/tensor state and reciprocal work;
6. equality of clock-rate effects for every internally constructed clock; and
7. a common causal response for material and relative-vector signals.

Only after these pass may equation (11) be compared with proper time or
equation (8) with gravitational mass.

---

## 7. Next locked gate

Replace the fixed bright controller by a finite self-maintaining local orbit of
the C18 scattering and actualization rules. Its formation basin, inverse,
energy, translation, and capacity-gated period must be derived before fitting
any particle mass, redshift, or gravitational observable.

The subsequent
[ternary-square phase/polarity carrier theorem](THEOREM_TERNARY_SQUARE_PHASE_POLARITY_CARRIER_AND_AUTONOMOUS_CROSSING_CLOCK_v1.md)
removes the fixed bright controller for the local recurrence itself: a selected
phase-crossing permutation on the complete nine-state token has exact period
eight and reproduces the same mean capacity deficit. Initial reserve formation,
routing into a bound complex, stability, translation, blocked field response,
and physical time dilation remain open.

The later
[C4 stress-capacity reciprocal-feedback theorem](THEOREM_C4_STRESS_CAPACITY_RECIPROCAL_FEEDBACK_AND_MAXWELL_PARITY_PRICE_v1.md)
replaces the arbitrary permission word by a second owned A9 response carrier
in a complete finite permutation. That closes a reference capacity-derived
permission and backpressure loop, but not variational selection, physical
cross-sector work, composite formation/stability, or physical time dilation.
