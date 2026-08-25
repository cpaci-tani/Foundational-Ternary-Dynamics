# C4 physical Born actualization tape v1

**Date:** 2026-08-23
**Status:** **[THEOREM, CONDITIONAL — PREPARED FINITE C4 RECORDS PUSH FORWARD TO PHYSICAL MANIFESTATION COUNTS]** +
**[THEOREM — REVERSIBLE DETECTOR TAPE, SELF-ADDRESS TERMS, PAYLOAD/DARK-RECORD RETENTION]** +
**[SELECTION — OUTCOME PORT AS PHYSICAL TOKEN-OWNERSHIP ROUTE]** +
**[OPEN — NATIVE PREPARATION/ROUTER, GENERAL AMPLITUDES, SINGLE-TRIAL COMPETITION, MULTIPARTITE NO-SIGNALLING]**
**Physical Born status:** finite equal-weight C4 detector actualization is exact
conditional on a prepared residual bank and finite detector tape; general
physical Born recovery remains open
**Production status:** unchanged
**Ledger status:** no row minted

**Exact certificate:**
[proof_c4_physical_born_actualization_tape.py](../../../../../scripts/proofs/proof_c4_physical_born_actualization_tape.py)
exhausts all one-outcome phase multiplicities from zero through four, checks
four multi-outcome cases, and verifies the local gate algebra over all phases,
orientations, and common C4 shifts. It performs 12,924 exact checks without a
random sampler or target probability.

---

## 1. The remaining gap after deterministic pair enumeration

The
[coprime-ring theorem](THEOREM_C4_COPRIME_RING_BORN_PUSHFORWARD_v1.md)
constructs a target-blind orbit through every ordered pair of a prepared
residual record bank. The
[reversible click theorem](THEOREM_REVERSIBLE_C4_CANCELLATION_AND_CLICK_CIRCUIT_v1.md)
then toggles a bit for phase-compatible pairs. Those results leave a legitimate
physical question:

> What actually changes in the detector, and how are self-address events
> realized without copying or consuming the signal record?

The answer registered here is a finite reversible detector tape. The signal
records are nondestructive controls. A separate owned detector token is what
manifests.

---

## 2. Prepared records and one shared bank

For each physical outcome port $o$, begin with finite C4 counts

\[
 (N_{o,0},N_{o,1},N_{o,2},N_{o,3}).                 \tag{1}
\]

Reversible opposite-phase transfer leaves active residual records with

\[
 Z_o=(N_{o,0}-N_{o,2})+i(N_{o,1}-N_{o,3}).          \tag{2}
\]

All canceled pairs remain in payload-complete dark records. Load the active
residuals once into a shared bank of capacity $L$. Two nondestructive address
heads of periods $L$ and $L+1$ traverse the orbit

\[
 (x_n,y_n)=\bigl(n\bmod L,n\bmod(L+1)\bigr),          \tag{3}
\]

of length

\[
 T=L(L+1).                                          \tag{4}
\]

The signal bank is never duplicated. Each ordered pair of occupied addresses,
including every diagonal pair $(a,a)$, appears exactly once.

---

## 3. One fresh finite detector cell per pointer state

Provide a tape of $T$ finite detector cells. Cell $n$ initially owns one
complete oriented C4 token in reserve:

\[
 D_n^{(0)}=(\text{feed/reserve};\tau_n),
 \qquad
 \tau_n=(n\bmod4,+1).                               \tag{5}
\]

The phase schedule in equation (5) is target-blind and is not used by the
compatibility test; any predeclared detector-token phases give the same event
counts.

At pointer state $n$, let $r_x,r_y$ be the two nondestructively read signal
records. Define the local bright predicate

\[
 \chi(r_x,r_y)=1                                   \tag{6}
\]

exactly when both records are nonblank and occupy the same physical outcome
port and surviving C4 phase. Opposite phases have already entered dark memory.

The detector gate is

\[
 \chi=1:\quad
 D_n^{(0)}\longleftrightarrow D_{n,o}^{(1)},       \tag{7}
\]

where $D_{n,o}^{(1)}$ is the same token in manifested bond ownership at
physical route $o$, with neutral ternary endpoints. For $\chi=0$, the cell is
fixed. Equation (7) is the exact controlled-actualization involution, with the
port label interpreted as the token's physical apparatus route rather than an
abstract answer supplied to a probability selector.

---

## 4. Finite physical pushforward theorem

Let $M_o$ be the number of manifested tape cells routed to outcome port $o$
after one complete orbit. Then

\[
 \boxed{
 M_o=(N_{o,0}-N_{o,2})^2+(N_{o,1}-N_{o,3})^2
 =|Z_o|^2.}                                         \tag{8}
\]

### Proof

After cancellation, all real-rail survivors at one outcome have the same
phase, and all imaginary-rail survivors have the same phase. If their counts
are $r_{o,R}$ and $r_{o,I}$, the complete ordered-pair orbit contains exactly
$r_{o,R}^2$ real bright pairs and $r_{o,I}^2$ imaginary bright pairs. Every
such pair controls one distinct detector cell by equation (7), giving

\[
 M_o=r_{o,R}^2+r_{o,I}^2=|Z_o|^2.                  \tag{9}
\]

No other cell manifests. $\square$

Conditioning the manifested tape on an event gives

\[
 \boxed{
 f_o={M_o\over\sum_rM_r}
 ={|Z_o|^2\over\sum_r|Z_r|^2}.}                    \tag{10}
\]

Equation (10) is now a count of physical token-ownership changes, not only a
software counter or abstract basin cardinality.

---

## 5. Self-address terms are physical without record duplication

The diagonal ordered pairs contribute

\[
 \sum_k n_{o,k}                                    \tag{11}
\]

terms inside the corresponding squares. At a diagonal pointer state, both
heads read the same ontic bank record. That record is neither copied nor moved.
It acts twice as a nondestructive control on one separate detector cell.

The certificate verifies exactly one manifested diagonal detector cell per
active residual record. Thus the mathematical self-pair is realized as two
read contexts sharing one record, with a distinct apparatus token carrying the
physical consequence.

---

## 6. Reversibility, records, and finite work ledger

Every local detector gate is an involution because the bright predicate
depends on unchanged signal records. After $T$ ticks, both address heads return
to their initial positions. Applying the same complete orbit again gives

\[
 \mathcal U_T^2=1                                  \tag{12}
\]

and restores the entire tape to reserve ownership.

Throughout the forward pass:

- the shared residual bank is unchanged;
- every canceled pair remains in dark memory;
- each detector cell owns exactly one complete C4 token;
- every manifested detector bond carries neutral ternary endpoints;
- all record identities and detector payloads survive; and
- no information is erased to obtain equation (10).

This closes the finite token-energy and inverse ledger for the detector tape.
It does not derive the blocked continuous work required by a macroscopic
detector; that action must emerge from the finite transaction ensemble.

---

## 7. Connection to the common source vertex

Each firing of equation (7) is the
[actualization shared-moment source vertex](../common_action_mechanics_reciprocity/THEOREM_C18_ACTUALIZATION_SHARED_MOMENT_SOURCE_VERTEX_v1.md).
Therefore every counted Born event has simultaneous finite consequences:

\[
 \text{detector manifestation}
 +\text{relative-vector increment}
 +\text{common-tensor increment}
 +\text{capacity debit}.                            \tag{13}
\]

The Born count and the candidate field source are no longer separate
primitives. They are different readings of the same manifested detector
token.

---

## 8. Contextuality

The mixer law, address periods, and local bright predicate do not read
$Z_o$, $|Z_o|^2$, or a desired frequency. Context enters through physical
pre-measurement routing: it determines which record arrives at which localized
outcome port. Once the routed bank is prepared, equations (3), (6), and (7)
are unchanged for every context.

This is a concrete contextual pushforward architecture. It does not yet prove
that arbitrary context transformations are generated locally or that
multipartite implementations obey operational no-signalling.

The exact
[C4 Born/radiation kernel-separation theorem](../common_action_mechanics_reciprocity/THEOREM_C4_BORN_RADIATION_KERNEL_SEPARATION_AND_CONTEXTUAL_MIXER_BOUNDARY_v1.md)
clarifies the type of the remaining bridge. The pre-cancellation history
amplitude occupies the real two-dimensional quadrature sector (P_Q), while
the phase-neutral manifested detector/Gauss readout occupies the trivial
sector (P_0). The controlled bright predicate in equation (7) conditionally
performs that sector conversion. The exact
[paired-history phase-neutral successor](../common_action_mechanics_reciprocity/THEOREM_C4_PAIRED_HISTORY_PHASE_NEUTRAL_ACTUALIZATION_SOURCE_VERTEX_v1.md)
further proves that the bright/cross-rail/dark relation is the (+1/0/-1)
value of the unique normalized symmetric C4 pair contraction, and that its
positive value drives the same phase-neutral charge/stress source. This tape
theorem still does not derive the history preparation or the physical
contextual routing into outcome ports. Thus the registered pushforward is
physical after its prepared routed-bank input, not yet an autonomous
derivation of that input.

---

## 9. What remains open

This theorem closes only the prepared finite equal-weight C4 detector step.
The native action must still derive:

1. production of the complete history records from source dynamics;
2. local routing, the (P_Q) cancellation, and native nonlinear pair coupling
   into the phase-neutral manifestation sector;
3. formation and transport of the two address heads and finite detector tape;
4. robustness away from a complete $L(L+1)$ orbit;
5. native production of the finite Gaussian-integer block sequence used by
   the later controlled general complex-amplitude limit;
6. why one experimental trial yields the registered macroscopic outcome rather
   than interpreting a complete tape pass as one trial;
7. sequential measurement and detector refractory/reuse dynamics; and
8. multipartite context composition with operational no-signalling.

The physical Born pushforward is therefore **exact on the registered finite
prepared domain** and still **open in general**. No claim about Bell violation
or Tsirelson recovery is made.

The subsequent
[Gaussian-integer general-amplitude physical limit](THEOREM_C4_GAUSSIAN_INTEGER_GENERAL_AMPLITUDE_PHYSICAL_LIMIT_v1.md)
extends this prepared domain densely to every finite normalized complex
response with an explicit finite-resolution error and detector-resource
price. That theorem does not generate its approximating banks or close
single-trial competition and multipartite no-signalling.

---

## 10. Next locked gate

Embed the bank, dark memory, two heads, and detector tape into the same local
C18 transaction state as scattering and the shared source vertex. The pass
criterion is one reversible finite update law whose autonomous preparation
and finite-time statistics retain equation (10), while its material detector
forms and resets without an external compiler or erasure step.

The later
[autonomous reversible renewal detector](THEOREM_C4_AUTONOMOUS_REVERSIBLE_BORN_RENEWAL_DETECTOR_v1.md)
closes the prepared detector-reset portion without retaining the full tape.
It replaces $L(L+1)$ separately owned detector/source cells by one reusable
nine-token working payload and a total balanced-ternary renewal permutation.
Every bright pair produces one exclusive physical Gauss event before the
address clock advances. The residual bank and rings are still prepared, and
externally heralded one-click trials, finite-window robustness, and
multipartite no-signalling remain open.
