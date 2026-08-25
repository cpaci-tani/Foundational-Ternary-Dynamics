# C4 paired-history phase-neutral actualization/source vertex v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — UNIQUE NORMALIZED SYMMETRIC C4 QUADRATURE CONTRACTION]** +
**[THEOREM — BRIGHT/CROSS-RAIL/DARK RELATION IS THE (+1/0/-1) CONTRACTION]** +
**[THEOREM, CONDITIONAL — PREPARED POSITIVE PAIR DRIVES REVERSIBLE MANIFESTATION AND COMMON SOURCE]** +
**[OPEN — AUTONOMOUS HISTORY/CONTEXT PREPARATION, ACTION SELECTION, RECIPROCAL WORK, AND GENERAL BORN]**  
**Production status:** unchanged  
**Ledger status:** no row minted; no Born, gravity, coupling, or alpha claim promoted

**Exact certificate:**
[proof_c4_paired_history_phase_neutral_actualization_vertex.py](../../../../../scripts/proofs/proof_c4_paired_history_phase_neutral_actualization_vertex.py)
performs **14,957 exact checks**. It proves the invariant-form uniqueness,
exhausts the complete two-outcome/four-phase relation table and global C4
orbit, checks all phase multiplicities from zero through four after reversible
dark cancellation, and composes every compatible pair with every detector
phase, both charge orientations, and all nine C18 lines. No target probability,
coupling, master root, or measured value enters.

---

## 1. Why a pair, rather than a linear mixer, is natural

The
[C4 Born/radiation kernel-separation theorem](THEOREM_C4_BORN_RADIATION_KERNEL_SEPARATION_AND_CONTEXTUAL_MIXER_BOUNDARY_v1.md)
proves that a linear C4-invariant quadratic action cannot mix the
phase-neutral field sector \(P_0\) with the history quadrature sector \(P_Q\).
It left two admissible routes: a context-carrying off-diagonal linear mixer or
a phase-neutral nonlinear interaction.

Born counting already supplies the second route. Actualization is controlled
by **two** histories, and the tensor product of two quadrature carriers contains
a C4 scalar.

Let

\[
 q_0=(1,0),\quad q_1=(0,1),\quad
 q_2=(-1,0),\quad q_3=(0,-1),                         \tag{1}
\]

with quarter-turn action

\[
 R=\begin{pmatrix}0&-1\\1&0\end{pmatrix}.             \tag{2}
\]

For a symmetric bilinear form (A=A^{\mathsf T}), C4 invariance requires

\[
 R^{\mathsf T}AR=A.                                     \tag{3}
\]

The exact solution is (A=\lambda I_2). Normalizing
(q_0^{\mathsf T}Aq_0=1) uniquely gives

\[
 \boxed{A=I_2.}                                        \tag{4}
\]

The independent antisymmetric wedge is excluded by ordered-history exchange
symmetry; it is not a positive event count.

---

## 2. The compatibility trit is an invariant contraction

Define

\[
 \kappa(p,q)=q_p\cdot q_q
 =\cos\!\left({\pi(p-q)\over2}\right).                 \tag{5}
\]

Its complete relative-phase table is

\[
 \boxed{
 \begin{array}{c|ccc}
 q-p\pmod4&0&\pm1&2\\ \hline
 \kappa&+1&0&-1\\
 \text{registered relation}&\text{bright}&
 \text{cross rail}&\text{dark}
 \end{array}}                                           \tag{6}
\]

The relation therefore has an exact ternary value. A common phase advance
leaves it invariant:

\[
 \kappa(p+1,q+1)=\kappa(p,q).                           \tag{7}
\]

The outcome-port equality test is separate: histories routed to different
physical outcome ports do not interact. Within one port, however, the existing
bright/dark predicate is exactly equation (5), not an independently chosen
phase-compatibility lookup table.

---

## 3. Dark cancellation and the exact positive-pair count

For one outcome, opposite-phase records have (kappa=-1). The reversible
cancellation circuit moves those pairs into payload-complete dark records.
The residual bank then has at most one phase sign on each quadrature rail.
Consequently every same-outcome residual ordered pair has

\[
 \kappa\in\{0,+1\}.                                      \tag{8}
\]

If the raw multiplicities are ((n_0,n_1,n_2,n_3)), the number of positive
residual contractions is exactly

\[
 \begin{aligned}
 N_+
 &=|n_0-n_2|^2+|n_1-n_3|^2\\
 &=\left|(n_0-n_2)+i(n_1-n_3)\right|^2\\
 &=|Z|^2.                                                 \tag{9}
 \end{aligned}
\]

Thus the square is the cardinality of positive values of the unique
normalized symmetric C4 contraction after reversible negative-pair removal.
Equation (9) remains a prepared finite counting theorem; it does not by itself
prepare or sample the histories.

---

## 4. Reversible manifestation from the derived predicate

Let (g=mathbf1_{\kappa=+1}) after same-port routing. The existing complete
actualization macro uses this derived bit to exchange one payload-complete
token between reserve and manifestation ownership:

\[
 ({\cal R},g=1)\longleftrightarrow({\cal M},g=1).          \tag{10}
\]

For (g=0), the state is unchanged. For every token phase and charge
orientation, equation (10):

- is an involution;
- preserves the token and its payload;
- preserves total token number and neutral endpoint charge; and
- is equivariant under a common C4 advance.

The Boolean controlling equation (10) is therefore no longer an unexplained
phase relation on the prepared bank. It is the positive value of equation
(5). The remaining external input is the physical preparation and contextual
routing of that bank.

---

## 5. The same event produces phase-neutral field and stress sources

On a C18 line (d), let (M=dd^{\mathsf T}) and token orientation
(\epsilon=\pm1). Composing equation (10) with the certified common-moment
readout gives

\[
 \boxed{j_{\rm evt}={\epsilon\over9}d,}                   \tag{11}
\]

\[
 \boxed{t_{\rm evt}={1\over18}M=-\Delta K.}              \tag{12}
\]

The orthogonal phase contractions vanish. Hence equations (11)--(12) are
independent of the token's absolute C4 phase. Charge conjugation reverses
(j_{\rm evt}) and preserves (t_{\rm evt}) and the capacity debit.

The exact conditional chain is now

\[
 \boxed{
 \begin{gathered}
 \text{two routed C4 histories}
 \xrightarrow{\ \kappa=+1\ }
 \text{one reversible manifested token}\\
 \xrightarrow{\ \text{same ownership change}\ }
 \left(\Delta s,\;j_{\rm evt},\;t_{\rm evt},\;\Delta K\right).
 \end{gathered}}                                         \tag{13}
\]

This is one local nonlinear **source vertex** joining contextual history
compatibility, manifestation, the charge-odd electromagnetic source type, and
the charge-even stress/gravity source type. It does not yet provide their
propagating response or reciprocal action.

---

## 6. Minimum common-action term suggested by the theorem

Let (m_o) be a phase-neutral manifestation coordinate at outcome port (o),
and let (q_{L,o},q_{R,o}) be the two routed quadrature-history coordinates.
The representation-theoretically minimum symmetric interaction has the form

\[
 \boxed{
 \mathcal A_{\rm pair}
 =g_A\sum_o m_o\,q_{L,o}\cdot q_{R,o}.}                  \tag{14}
\]

Equation (14) is C4 invariant without selecting a global phase. It is a
**selection candidate**, not an adopted action: the finite gate proves the
allowed contraction and reversible ownership response, but it does not derive
the coefficient (g_A), the dark-binding energy, the detector route (o(C)),
or the source/field backreaction.

Context should therefore enter through the physical routing map

\[
 (q_L,q_R)\longmapsto(q_{L,o(C)},q_{R,o(C)}),             \tag{15}
\]

not through an absolute C4 phase choice. This preserves global phase
covariance while allowing different apparatus contexts to define different
localized outcome ports.

---

## 7. What this closes and what remains

### Closed on the prepared finite domain

1. The symmetric C4 pair contraction is unique after normalization.
2. Bright, cross-rail, and dark relations are its (+1,0,-1) values.
3. Reversible dark cancellation leaves exactly (|Z_o|^2) positive ordered
   contractions.
4. A positive pair drives a reversible physical manifestation transfer.
5. The same transfer produces the registered phase-neutral charge and stress
   sources.

### Still open

1. autonomous history generation and local reversible routing into outcome
   ports for arbitrary apparatus contexts;
2. formation and work cost of dark memory, address clocks, and detector
   readiness from the common substrate action;
3. single-trial competition, incomplete-window robustness, amplification, and
   multipartite operational no-signalling;
4. reciprocal electromagnetic and tensor-field work on the manifested token;
5. stable matter, physical inertia, constrained Maxwell and spin-2/equivalent
   poles, and lensing; and
6. all native coupling normalizations, including the fine-structure constant.

The physical Born problem is consequently narrower but not closed: the local
pair-to-event-to-source map is exact, while the action-generated ensemble and
its autonomous contextual routing remain unproved.

The subsequent
[Hodge-framed all-axis signed-event theorem](THEOREM_HODGE_FRAMED_ALL_AXIS_CONSTRAINT_LIFT_AND_ONE_SIGNED_EVENT_GENERATOR_BOUNDARY_v1.md)
places this prepared pair gate inside one exact reciprocal event generator.
The same ownership sign now controls manifestation, charge current,
trace/STF/longitudinal source loading, material recoil, clock action, event
energy, and the required port reaction. This closes the prepared event-action
composition, not the autonomous history ensemble, apparatus routing, or
physical Born pushforward.
