# V3 triplet-clock Legendre inertia Phi-v11 candidate and gravity boundary v1

**Date:** 2026-08-24  
**Status:** **[SELECTION — TRIPLET CLOCK AS INERTIAL DENOMINATOR]** +
**[THEOREM, CONDITIONAL — EXACT CLOCKED MOMENTUM-TO-HOP CADENCE]** +
**[THEOREM — EXACT INERTIAL CONTINUATION AND INVERSE]** +
**[THEOREM, CONDITIONAL — DISCRETE LEGENDRE MATCH AND BLOCK INERTIA]** +
**[THEOREM, CONDITIONAL — INHERITED INITIAL-PHASE PROTECTION]** +
**[BOUNDARY — CANONICAL PHI, PHYSICAL UNITS, UNIVERSALITY, AND GRAVITY OPEN]**  
**Additional carrier price:** one existing fixed-occupancy A2 inertial-phase
owner and a prepared finite clear corridor; no new primitive type  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Mechanical-drift parent:**
[`THEOREM_V3_GREEN_PULSE_TRIPLET_MECHANICAL_DRIFT_PHI_v10_CANDIDATE_AND_INERTIA_BOUNDARY_v1.md`](THEOREM_V3_GREEN_PULSE_TRIPLET_MECHANICAL_DRIFT_PHI_v10_CANDIDATE_AND_INERTIA_BOUNDARY_v1.md)  
**Exact certificate:**
[`proof_v3_triplet_clock_legendre_inertia_phi_v11_candidate.py`](../../../../../scripts/proofs/proof_v3_triplet_clock_legendre_inertia_phi_v11_candidate.py)

---

## 1. The exact boundary inherited from Phi-v10

Phi-v10 proves that a finite Green-response pulse moves a prepared physical
triplet. It also proves that the body stops when the response pulses stop,
even though Phi-v9 retains nonzero probe momentum `p_P`. Thus the missing map
is precise:

\[
 p_P\quad\not\longmapsto\quad\text{persistent body motion}              \tag{1}
\]

under Phi-v10 alone.

The clean self-correcting triplet has an independently exact internal period

\[
 \boxed{T_M=16\text{ global ticks}.}                  \tag{2}
\]

Phi-v11 selects this physical matter-clock period as the denominator of the
minimum finite momentum-to-motion cadence. This identification is motivated
by the ontology—matter carries its own recurrence—but it is a **selection**,
not a theorem forced by the five postulates.

---

## 2. Finite inertial cadence

Add one fixed-occupancy A2 phase owner `r in Z_16`. For admitted momentum

\[
 |p|\le T_M,                                           \tag{3}
\]

advance

\[
\begin{aligned}
 u_t&=r_t+|p|,\\
 b_t&=\mathbf1[u_t\ge T_M],\\
 r_{t+1}&=u_t-T_Mb_t.
\end{aligned}                                         \tag{4}
\]

On a carry, the force-aligned triplet applies its existing clean Phi-v6 hop;
otherwise only its Phi-v5 internal clock advances. Over one material-clock
period,

\[
 \sum_{t=0}^{15}b_t=|p|,
 \qquad r_{16}=r_0.                                   \tag{5}
\]

Hence the exact signed displacement and velocity are

\[
 \boxed{\Delta X=p,d,
 \qquad v={p\over16}d.}                               \tag{6}
\]

Each microtick is zero or one SC hop. Addition by `|p|` on `Z_16`, the clean
triplet predecessor, and the retained momentum sign give one exact inverse.
The certificate exhausts every integer `-16<=p<=16`.

---

## 3. Inertial continuation is now an actual position history

For a completed Phi-v9 response with edge current `C`,

\[
 p_P=-C.                                               \tag{7}
\]

Phi-v10's field-controlled window ends after `12N` ticks. Phi-v11 then runs
with no new response pulse. Equations (6)--(7) give the next dark-window
displacement

\[
 \boxed{\Delta X_{\rm dark}=-Cd.}                     \tag{8}
\]

The certificate verifies equation (8) for
`C in {-16,-7,-1,1,7,16}`. Unlike Phi-v10, motion now continues because the
momentum record survives, not because the external response keeps pulsing.

This closes **inertial continuation on the prepared finite component**. It
does not yet prove that equation (6) is the unique or canonical matter law.

---

## 4. Clock-selected discrete Legendre match

Phi-v9 selected the minimum positive sign-even phase-action shape

\[
 K_0=p_P^2+p_R^2+W.                                   \tag{9}
\]

To make its Legendre velocity agree with equation (6), the relative
coefficient must be

\[
 \boxed{\gamma_M={1\over2T_M}={1\over32}.}            \tag{10}
\]

Define

\[
 H_M={p_P^2+p_R^2+W\over2T_M}.                        \tag{11}
\]

Then

\[
 \boxed{{\partial H_M\over\partial p_P}
 ={p_P\over T_M}=v.}                                 \tag{12}
\]

Equation (12) is an exact match between the finite hop cadence and the
selected quadratic action. It does not require a continuous primitive time;
the derivative is a blocked readout of the finite action, while equation (4)
is the actual microdynamics.

The overall conversion from dimensionless lattice action to physical energy
remains free. Multiplying equation (11) by a common unit changes no finite
transition. Thus `1/32` is a relative lattice coefficient, not a value in SI
units and not a measured inertial mass.

---

## 5. Exact block inertia relation

During one Phi-v9 response window,

\[
 \Delta p=-C,
 \qquad \Delta t=12N.                                 \tag{13}
\]

The average impulse rate is

\[
 \bar F={\Delta p\over12N}.                           \tag{14}
\]

From equation (6),

\[
 \Delta v={\Delta p\over T_M},
 \qquad
 \bar a={\Delta v\over12N}.                          \tag{15}
\]

Therefore

\[
 \boxed{\bar F=T_M\bar a=16\bar a.}                 \tag{16}
\]

Equation (16) is exact on the selected block variables. It is not yet a claim
that the physical triplet mass is 16 kilograms, 16 Planck masses, or any
other dimensional quantity. It says only that the chosen lattice momentum,
clock, and displacement coordinates have inertial ratio 16.

---

## 6. Inherited Green protection

The physical edge current obeys

\[
 \left|{C\over N}-g_e\right|\le B_e(N).               \tag{17}
\]

Combining the response duration `12N` with equation (6) gives the exact block
acceleration

\[
 \bar a=-{C/N\over12T_M}.                             \tag{18}
\]

Hence

\[
 \boxed{
 \left|\bar a+{g_e\over12T_M}\right|
 \le {B_e(N)\over12T_M}.}                            \tag{19}
\]

Two initial rotor phases differ by at most twice the right-hand side. All 192
native uniform phases on the certified radius-one source edge satisfy
equation (19) at `N=37`, and their momenta remain inside equation (3). The
largest exact acceleration error is `5/42624` on that fixture.

---

## 7. What is selected and what remains open

```text
triplet internal period T_M=16:             theorem
finite |p|-per-16-tick hop cadence:         theorem, conditional apparatus
post-force inertial continuation:           theorem, conditional corridor
Legendre coefficient 1/(2T_M)=1/32:        selection by clock matching
block relation F_bar=16 a_bar:              theorem under that selection
canonical microscopic action provenance:   open
dimensional inertial mass:                  open
```

The remaining gates are:

1. derive or reject the clock-to-inertia identification from canonical
   homogeneous `Phi`;
2. form the inertial-phase owner, response apparatus, work reserve, chart, and
   corridor natively;
3. steer under the vector sum of all incident edge currents without erasing
   the previous chart;
4. give the reaction momentum its own material continuation and return it to
   the source;
5. extend the finite cadence to general momentum, relativistic saturation,
   collisions, traffic, packet loss, and overflow;
6. derive the overall action/length/time units and universal coupling of
   stable matter and radiation;
7. recover the scalar constraint and tensor radiative pole from the same
   action; and
8. establish the common cone, clock response, lensing, Shapiro delay, and
   nonlinear completion.

Phi-v11 closes a logically missing inertial map on one prepared finite sector.
It does not close physical gravity.

---

## 8. Reproduction

From the repository root:

```bash
python scripts/proofs/proof_v3_triplet_clock_legendre_inertia_phi_v11_candidate.py
```

Expected result: `12/12` exact checks pass. The certificate reports matter
clock 16, velocity `p/16`, relative quadratic coefficient `1/32`, exact
post-force dark continuation, block relation `F_bar=16 a_bar`, inherited
all-phase Green protection, and a free overall physical unit.
