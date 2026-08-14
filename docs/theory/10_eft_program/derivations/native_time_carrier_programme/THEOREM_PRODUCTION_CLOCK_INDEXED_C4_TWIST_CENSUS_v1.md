# Theorem — Production clock-indexed C4 twist census v1

**Identifier:** `FTD-0978`  
**Date:** 2026-08-12  
**Status:** `[THEOREM — EXACT SOURCE-LOCKED PRODUCTION-PATH CENSUS]` +
`[CORRECTION — FULL PHASE-COMPLETE SWAP IS SYMPLECTIC; FLUX-ONLY DETERMINANT IS NOT THE CANONICAL TEST]` +
`[CLOSED NEGATIVE — PRESENT DE BROGLIE-CLOCK/WEAK-TRANSMUTATION ROUTE AS A C4 WITNESS]` +
`[BOUNDARY — ORDER-TWO EXCHANGE LOSES THE ORIENTED SQUARE ROOT]` +
`[OPEN — EXPLICIT CLOCK-INDEXED TWIST, REACTION, WORK, HISTORY, AND FORMATION]`

## Result

The unchanged production engine does not implement the physical twisted
endpoint identification left open by FTD-0977 through its present
de Broglie-clock and weak-transmutation path.

Both ingredients exist, but they are dynamically separate:

- the imposed clock advances a read-only host diagnostic
  `phase += omega0 * delta_tau` and applies the same Klein--Gordon coefficient
  to the left and right substrates;
- weak transmutation is triggered by state, left-substrate stress, a fixed
  threshold, seeded voxel RNG, and global tick; it reads neither `phase` nor
  `tau`; and
- after the predicate fires, it flips ternary polarity and swaps the complete
  left/right flux and wave-velocity pairs.

That swap is a valid symplectic half-turn. It is not an oriented quarter-turn.
The current route therefore cannot distinguish clockwise from
counterclockwise in the precise sense required by the retained `C4` carrier.

This is a closed negative for the named production route only. It is not a
whole-engine impossibility theorem and does not prohibit a new explicit
clock-indexed mapping-torus mechanism.

## Certificate of record

- Pre-registration:
  [`PREREG_PRODUCTION_CLOCK_INDEXED_C4_TWIST_CENSUS_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_PRODUCTION_CLOCK_INDEXED_C4_TWIST_CENSUS_v1.md),
  SHA-256 `F194A9148909D4C8DDC0057266DC56CA93A1316335180C47541FADA3CE9F4A83`.
- Immutable proof:
  [`proof_production_clock_indexed_c4_twist_census.py`](../../../../../scripts/proofs/proof_production_clock_indexed_c4_twist_census.py),
  SHA-256 `701E3207A366DCF98E3975C3CF2B8A5C05E1A47DC978C9F7E8854CB2B1717070`.
- First execution: `51/51`, Outcome B, no repair.
- Production mutation: none.

Thirteen production sources were hash locked in the protocol. They cover the
voxel clock state, toggles, CPU clock and weak phases, tick ordering, dual
read/write dynamics, CUDA buffers and kernels, and the production energy
ledger.

## 1. Exact exchange algebra

For one Cartesian component, write the dual flux and wave-velocity variables
as the canonical-looking vector

\[
 z=(q_L,q_R,p_L,p_R)^T.
\]

The implemented simultaneous exchange is

\[
 S=
 \begin{pmatrix}
 0&1&0&0\\
 1&0&0&0\\
 0&0&0&1\\
 0&0&1&0
 \end{pmatrix}.                                           \tag{1}
\]

For the standard symplectic form

\[
 \Omega=
 \begin{pmatrix}0&I_2\\-I_2&0\end{pmatrix},
\]

the certificate verifies exactly

\[
 S^T\Omega S=\Omega,
 \qquad
 S^TS=I_4,
 \qquad
 S^2=I_4.                                                  \tag{2}
\]

Thus the conditional exchange is symplectic, preserves the equal quadratic
norm, and is its own inverse.

This corrects the limited determinant statement in FTD-0965. Its `2 x 2`
matrix exchanged only the two flux labels and indeed has determinant `-1`.
Production simultaneously exchanges their conjugate wave velocities. The
complete `4 x 4` canonical map is `diag(S_2,S_2)`, has determinant `+1`, and
is symplectic. FTD-0965's conclusion that the exchange is not the oriented
quarter-turn survives, but determinant parity is not the reason.

## 2. What the symmetric square loses

Introduce common and relative variables

\[
 q_C={q_L+q_R\over\sqrt2},\quad
 q_D={q_L-q_R\over\sqrt2},\qquad
 p_C={p_L+p_R\over\sqrt2},\quad
 p_D={p_L-p_R\over\sqrt2}.                                \tag{3}
\]

Then equation (1) becomes

\[
 (q_C,q_D,p_C,p_D)
 \longmapsto
 (q_C,-q_D,p_C,-p_D).                                     \tag{4}
\]

The common pair is unchanged and the relative pair sees `-I_2`. By contrast,
the two oriented quarter-turns are

\[
 J=\begin{pmatrix}0&-1\\1&0\end{pmatrix},
 \qquad -J,
 \qquad J^2=(-J)^2=-I_2.                                  \tag{5}
\]

Therefore the implemented relative swap is the common square of both
orientations:

\[
 S_{\rm rel}=-I_2=J^2=(-J)^2.                              \tag{6}
\]

Equation (6) is the exact information boundary. Squaring erases whether the
preceding quarter step was clockwise or counterclockwise. The left/right
doublet supplies the half-turn hardware, but it does not supply the missing
oriented square root.

## 3. Why the present clock does not restore orientation

The CPU and CUDA Klein--Gordon terms apply the same scalar coefficient to
both substrates. Algebraically their dual-channel operator is proportional
to the identity, so

\[
 [C_{\rm KG},S]=0.                                         \tag{7}
\]

This is a useful symmetry: the clock oscillator is compatible with left/right
exchange. It is not a gearbox. A commuting oscillator does not choose `+J`
versus `-J`, and the weak predicate never reads the accumulated phase.

The production ordering makes the separation explicit. Weak transmutation is
Rule 6; proper-time and diagnostic-phase accumulation occurs later at Rule 8.
On CUDA the weak kernel receives the RNG seed and global tick but no phase or
proper-time argument. Device storage contains `d_tau` and no `d_phase`.

Hence sharing a global update does not create the required fiber action.
Clock indexing needs a phase-crossing or equivalent retained latch in the
transition predicate itself.

## 4. Conditional inverse is not an autonomous inverse

For a frozen firing decision, `S^{-1}=S`. But the production update has the
form

\[
 F(x)=
 \begin{cases}
 Sx,&P(x)=1,\\
 x,&P(x)=0,
 \end{cases}                                               \tag{8}
\]

where the stress predicate is evaluated on the incoming left substrate. If
an admissible pair satisfies

\[
 P(x)=1,\qquad P(Sx)=0,
\]

then

\[
 F(x)=Sx=F(Sx).                                            \tag{9}
\]

The gate is then non-injective even though its fired branch is invertible.
The certificate constructs this exact predicate-level witness; it does not
claim every runtime state realizes it.

The optional CPU event journal records before/after states but is explicitly
observation-only. It is not an always-present production memory or a reverse
transaction.

## 5. Missing reaction and work ledger

The production energy ledger tracks observable field norm, wave norm,
particle kinetic energy, an optional strong potential, damping residual, and
cumulative injection/dissipation. It has no channel for:

- clock potential energy associated with the imposed `omega0` term;
- a conjugate clock momentum;
- the FTD-0977 covariant connection reaction;
- switching work or reserve;
- an oriented crossing latch; or
- retained inverse history.

Swapping left and right leaves their observable sum unchanged, so the current
operation is invisible to the ledger's observable-sum norm. That invariance is
consistent with the half-turn interpretation. It is not evidence of a
physical zero-cost quarter-turn.

## 6. The minimum missing production mechanism

The result isolates four requirements for a future production candidate:

1. **Oriented root:** act by `+J` or `-J`, not only by their shared square
   `-I`.
2. **Clock clutch:** fire at a local phase crossing or equivalent
   context-blind clock latch, not merely somewhere inside the same global
   tick.
3. **One-clock reaction:** use the single covariant momentum forced by
   FTD-0977 and book the equal-and-opposite reaction/work transaction.
4. **Retained inverse:** store enough ternary orientation/history to reverse
   the predicate-gated event without consulting an observation-only journal.

These are necessary criteria for this programme. They do not yet select a
unique field representation, local connection profile, integer lift, energy
scale, formation dynamics, or `G*` cadence.

## 7. Epistemic disposition

What is established:

- **[THEOREM]** the implemented simultaneous left/right exchange is
  symplectic, norm preserving, and order two;
- **[CORRECTION]** the FTD-0965 flux-only `det=-1` observation does not
  classify the phase-complete production map, whose determinant is `+1`;
- **[THEOREM]** its common/relative form is `diag(+1,-1,+1,-1)`;
- **[BOUNDARY]** its relative half-turn is the common square of both oriented
  quarter-turns and therefore cannot distinguish their sign;
- **[CLOSED NEGATIVE]** the present de Broglie-clock plus weak-transmutation
  route is not the FTD-0977 physical `C4` witness; and
- **[OPEN]** an explicit clock-indexed twist with reaction, work, retained
  inverse, formation, stability, `G*` cadence, and operational tests.

No Hilbert-space recovery, Born rule, Bell correlation, physical mass,
`G*` clock identification, or framework-completeness claim follows.
