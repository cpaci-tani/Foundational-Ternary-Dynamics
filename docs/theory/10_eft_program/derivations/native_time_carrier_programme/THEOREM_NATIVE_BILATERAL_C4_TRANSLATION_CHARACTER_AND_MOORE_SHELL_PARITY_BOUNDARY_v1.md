# FTD-0935 — Native bilateral C4 translation character and Moore-shell parity boundary v1

**Identifier:** `FTD-0935`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — ORDERED-PRESENTATION-INVARIANT TIME-ODD POLAR INTEGER DATUM]` +
`[THEOREM — EXACT EXISTING-DATA C4 TRANSLATION CHARACTER]` +
`[CONDITIONAL CLASSIFICATION — MINIMUM CUBIC INTEGER-LINEAR GEARBOX UNIQUE UP TO CONJUGATION]` +
`[THEOREM — MOORE-SHELL CHIRALITY PARITY]` +
`[BOUNDARY — PROTECTED PRODUCTION MEMORY/PHYSICAL MOMENTUM/COMMON ACTION OPEN]`  
**Production status:** unchanged

## 1. Result

The directional character required by FTD-0934 is representable in existing
native data without adding a state type.

For a distinct neutral ternary pair, let

\[
 a=x_+-x_-\in\mathbb Z^3\setminus\{0\}               \tag{1}
\]

be the displacement from the negative endpoint to the positive endpoint. Use
the FTD-0907 bilateral phase wedge

\[
 \ell=q_+p_--q_-p_+,
 \qquad \chi=\operatorname{sgn}(\ell),
 \qquad \ell\ne0.                                    \tag{2}
\]

Then

\[
 \boxed{p_4=\chi a\in\mathbb Z^3}                    \tag{3}
\]

is invariant under reversal of the ordered endpoint presentation, transforms
as a polar vector under the signed cubic group, and reverses under canonical
time reversal. It supplies the compact translation character

\[
 \boxed{
 \Xi_{p_4}(d)=i^{p_4\cdot d}
 =\exp\!\left({i\pi\over2}p_4\cdot d\right),
 \qquad d\in\mathbb Z^3.}                            \tag{4}
\]

Equation (4) is the first exact representation-level gearbox in this branch
between the internal clockwise/counterclockwise `C4` sheet and integer spatial
translation. It is not yet a dynamical or energetic gearbox. In particular,
it does not protect `p_4`, produce motion, exchange recoil, unwrap Bloch
momentum, or identify the quartic `G*` period with global tick cadence.

## 2. Exact transformation law

Define the native polar axis `e=a/|a|` and endpoint projections

\[
 q_\pm=e\cdot J_\pm,
 \qquad p_\pm=e\cdot W_\pm.                           \tag{5}
\]

### 2.1 Signed cubic covariance

For any signed permutation matrix `Q`, transform positions and native vector
fields together. Then

\[
 a\mapsto Qa,
 \qquad e\mapsto Qe,
\]

while each projected `q_+`, `q_-`, `p_+`, and `p_-` remains a spatial scalar.
Consequently `ell` and `chi` are unchanged and

\[
 \boxed{p_4\mapsto Qp_4.}                             \tag{6}
\]

No global preferred direction is introduced.

### 2.2 Canonical time reversal

Under the FTD-0907 canonical reversal, the projected fluxes are unchanged and
the projected wave velocities reverse:

\[
 (q_+,q_-,p_+,p_-)
 \mapsto(q_+,q_-,-p_+,-p_-).                          \tag{7}
\]

Therefore

\[
 \ell\mapsto-\ell,
 \qquad \chi\mapsto-\chi,
 \qquad \boxed{p_4\mapsto-p_4}.                      \tag{8}
\]

This is the correct time parity for a momentum-like directed label.

### 2.3 Ordered-presentation reversal

Reverse the ordered presentation of the two endpoints and recompute every
projection against the reversed axis. Exactly,

\[
 a\mapsto-a,
\]

\[
 (q_+,p_+;q_-,p_-)
 \mapsto(-q_-,-p_-;-q_+,-p_+).                        \tag{9}
\]

Substitution into (2) gives

\[
 \ell\mapsto-\ell,
 \qquad\chi\mapsto-\chi.
\]

Both signs reverse, so

\[
 \boxed{(-\chi)(-a)=\chi a=p_4.}                     \tag{10}
\]

The directed datum is therefore carried by the relation between the polar
axis and the phase wedge, not by an arbitrary endpoint ordering.

## 3. Exact compact translation representation

Because `p_4` and `d` are integer vectors, (4) lies in

\[
 C_4=\{1,i,-1,-i\}.
\]

It obeys the character law

\[
 \Xi_{p_4}(d+f)
 =i^{p_4\cdot(d+f)}
 =\Xi_{p_4}(d)\Xi_{p_4}(f),                           \tag{11}
\]

the reversal law

\[
 \Xi_{-p_4}(d)
 =\Xi_{p_4}(-d)
 =\overline{\Xi_{p_4}(d)},                            \tag{12}
\]

and signed-cubic covariance

\[
 \boxed{
 \Xi_{Qp_4}(Qd)
 =i^{(Qp_4)\cdot(Qd)}
 =i^{p_4\cdot d}
 =\Xi_{p_4}(d).}                                     \tag{13}
\]

Its Bloch-torus label is

\[
 \boxed{[k]={\pi\over2}p_4
 \pmod{2\pi\mathbb Z^3}.}                           \tag{14}
\]

Thus the quarter-turn scale is fixed by the adopted internal `C4` generator,
not fitted to a trajectory. But only `p_4 mod 4` is visible in (4). The map is
compact and cannot supply the globally real additive momentum excluded by
FTD-0896 without a winding history, carry law, and physical impulse scale.

## 4. Minimum-class classification

To determine how arbitrary (4) is, restrict to the minimum registered class:
integer-linear maps

\[
 F:\mathbb Z^3\to\mathbb Z^3                         \tag{15}
\]

that commute with every signed permutation matrix of the cubic group.

Commutation with each independent coordinate sign flip forces every
off-diagonal entry of `F` to vanish. Commutation with coordinate permutations
forces the three diagonal entries to coincide. Hence

\[
 \boxed{F=mI_3,\qquad m\in\mathbb Z.}                 \tag{16}
\]

The four residue classes of `m` have exact meanings:

| `m mod 4` | character class | directional status |
|---:|---|---|
| 0 | `1` | trivial |
| 1 | primitive `C4` | orientation sensitive |
| 2 | real `C2` | `p_4` and `-p_4` indistinguishable |
| 3 | conjugate primitive `C4` | orientation sensitive |

The two odd cases differ only by global complex conjugation, which is the
choice of which quarter-turn is named clockwise. Therefore (4) is unique up
to conjugation inside the registered integer-linear signed-cubic-equivariant,
chirality-sensitive class.

This is a conditional classification of possible gearboxes. It does not prove
that the production tick forms, selects, or protects one.

## 5. Moore-shell parity theorem

For a one-step Moore displacement

\[
 a\in\{-1,0,+1\}^3\setminus\{0\},
 \qquad w=|a|^2\in\{1,2,3\},                          \tag{17}
\]

evaluate the character on the dipole displacement itself:

\[
 \boxed{\Xi_{\chi a}(a)=i^{\chi|a|^2}=i^{\chi w}.}  \tag{18}
\]

The exact result is

| Moore shell | directions | `w mod 4` | positive/negative chirality | verdict |
|---|---:|---:|---|---|
| face / SC | 6 | 1 | `+i / -i` | chirality visible |
| edge / FCC | 12 | 2 | `-1 / -1` | chirality lost |
| corner / BCC | 8 | 3 | `-i / +i` | chirality visible with reversed quarter-turn |

The self-translation phase distinguishes `chi` exactly when `|a|^2` is odd.
The FCC edge shell lands on the unique nontrivial self-conjugate `C4` element
`-1`; its self-probe cannot distinguish clockwise from counterclockwise. The
BCC corner shell has `|a|^2=3 mod 4` and lands on the conjugate quarter-turn,
so it retains the sign that a symmetric square loses.

This is a precise answer to what is special about the orientation-blind
directions in this gearbox: they occupy the even quadratic class. It is not a
claim that the FCC shell is physically inert or unable to transmit other
directional observables.

## 6. Reconciliation with the dressing square

For every value of the character,

\[
 \boxed{
 |1-\Xi_{p_4}(d)|^2
 =2[1-\operatorname{Re}\Xi_{p_4}(d)].}                \tag{19}
\]

Under time reversal or directed-label reversal,
`Xi -> conjugate(Xi)`. Equation (19) is unchanged, whereas

\[
 \operatorname{Im}\Xi_{-p_4}(d)
 =-\operatorname{Im}\Xi_{p_4}(d).                    \tag{20}
\]

This is exactly the finite `C4` realization of the FTD-0934 distinction:

- the positive dressing wake is the symmetric square and records cost;
- the unsquared character carries the missing forward/backward sheet.

Neither determines the other. A common action must couple them if the sheet
is to generate a reciprocal physical recoil.

## 7. Relation to the other candidate carriers

The present construction is minimal because the ternary pair supplies an
integer polar displacement directly and the bilateral wedge supplies its
time-odd sheet. Other existing observables remain useful but need additional
structure for this exact role:

- a real remainder or velocity is polar and may be time odd, but needs a
  justified integer/mod-four quantizer before it can be the exponent in (4);
- a plaquette construction supplies reflection-odd circulation and an axial
  normal, whose product can be polar, but it still requires a maintained
  oriented plaquette body; and
- a Fourier phase supplies `k` spectrally, but is not thereby a protected
  local source label.

This comparison does not exclude any of those routes. In particular the
later compact `C4` scaffold may be the better dynamic carrier even though the
two-endpoint pair is the minimum representation witness.

## 8. Production and Gaussian/CM boundaries

FTD-0908 observed finite sign-stable pair intervals, but FTD-0911/0913 closed
pair-specific protected bilateral memory and the exact central one-tick law
negative in the frozen production class. The present theorem does not reverse
that result. Equation (3) is a well-defined snapshot observable on `ell != 0`,
not a demonstrated persistent production state.

The mod-four shell table also must not be conflated with Gaussian-prime
splitting. The statement

\[
 |a|^2\pmod4\longmapsto i^{\chi|a|^2}
\]

is a finite lattice-pairing identity. The split/inert theorem for rational
primes in `Z[i]`, the CM Euler product, and the archimedean lemniscatic period
are different statements. No map from prime ideals or Frobenius data into the
local substrate update is derived here.

The honest gearbox status is therefore:

1. **internal orientation to compact spatial character:** closed at
   representation level by (3)--(4);
2. **compact character to protected local source state:** open, and negative
   for the frozen bilateral-pair production class;
3. **compact character to unwrapped physical momentum/recoil:** open;
4. **quartic `G*` period to integer global tick cadence:** open; and
5. **CM prime calendar to local substrate dynamics:** open.

## 9. Epistemic accounting

Theorem-grade on the registered nonzero-wedge domain:

- ordered-presentation invariance and exact time/cubic parities of `p_4`;
- the `C4` character, homomorphism, reversal, and covariance identities;
- classification `F=mI_3` for the registered cubic integer-linear class;
- uniqueness of the chirality-sensitive class up to conjugation;
- exact face/edge/corner counts and self-phase table; and
- reconciliation of the unsquared character with symmetric-square sign loss.

Still open:

- production-native protection or recovery of `p_4`;
- a compact scaffold/current realization of the same character;
- winding history, reciprocal carry ownership, and physical scale `p_*`;
- a common local action, equal-and-opposite field/source impulse, and vector
  recoil;
- autonomous source motion, formation, collision composition, and recovery;
- coupling to the critical quartic clock and finite-tick `G*` cadence;
- the CM-prime/substrate operator bridge;
- Born recovery, Bell-laboratory recovery, operational Lorentz hiding, and
  framework completeness.

No selected type, import currency, fitted constant, production law, or engine
path is added.

## 10. Certificate provenance

The frozen preregistration is
[`PREREG_NATIVE_BILATERAL_C4_TRANSLATION_CHARACTER_AND_MOORE_SHELL_PARITY_BOUNDARY_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_NATIVE_BILATERAL_C4_TRANSLATION_CHARACTER_AND_MOORE_SHELL_PARITY_BOUNDARY_v1.md),
SHA-256
`19512CF3431EF65DD65E88A53C14BA835681D2A29099B9DEAB81DB03D67B0CCA`.

The exact proof of record is
`scripts/proofs/proof_native_bilateral_c4_translation_character_moore_shell_parity_boundary.py`,
SHA-256
`D24F44FA80D34AC8F45A2C6330AF2E35CC86BEABF56AF028609AA154F4D86DE4`.
Its first immutable execution passed `98/98` checks and returned Outcome A.
No repair protocol was required.

## 11. Next acceptance gate

Use the formed compact C4 body/current rather than reviving the closed
bilateral-memory candidate. Construct from its live ternary/remainder/velocity
history an integer time-odd polar label `p_4^body` and demand:

1. equality with (3) whenever both observables coexist;
2. exact signed-cubic covariance and time reversal;
3. protection under held-out perturbations without target phase reading;
4. a local winding/carry owner if the character crosses a reciprocal boundary;
5. a common action that books the positive FTD-0933 wake and produces an
   equal-and-opposite source/field impulse; and
6. no read of `G*`, physical momentum targets, Born weights, settings, or
   outcomes.

If no protected body label exists, equation (4) remains an exact kinematic
representation witness rather than a physical momentum state.

```text
NATIVE_SNAPSHOT_DIRECTED_DATUM=p4=chi*(x_plus-x_minus)
P4_ORDERED_PRESENTATION_INVARIANCE=EXACT
P4_SIGNED_CUBIC_PARITY=POLAR
P4_TIME_PARITY=ODD
COMPACT_C4_TRANSLATION_CHARACTER=Xi_p4(d)=i^(p4.dot.d)
CHARACTER_HOMOMORPHISM=EXACT
MINIMUM_CUBIC_INTEGER_LINEAR_GEARBOX=m*I3
CHIRALITY_SENSITIVE_MULTIPLIERS=ODD_MOD_4
MINIMUM_GEARBOX_UNIQUENESS=UP_TO_COMPLEX_CONJUGATION
FACE_SELF_PROBE_CHIRALITY=VISIBLE
FCC_EDGE_SELF_PROBE_CHIRALITY=LOST
BCC_CORNER_SELF_PROBE_CHIRALITY=VISIBLE
DRESSING_SYMMETRIC_SQUARE_RETAINS_CHARACTER_SIGN=FALSE
PROTECTED_BILATERAL_PRODUCTION_MEMORY=FALSE_IN_FROZEN_TESTED_CLASS
COMPACT_BODY_CHARACTER_PROTECTION=OPEN
UNWRAPPED_PHYSICAL_MOMENTUM=OPEN
RECIPROCAL_CARRY_OWNERSHIP=OPEN
PHYSICAL_MOMENTUM_SCALE=OPEN
DYNAMIC_COMMON_ACTION_VECTOR_RECOIL=OPEN
GSTAR_INTEGER_TICK_CADENCE=OPEN
CM_PRIME_SUBSTRATE_OPERATOR_BRIDGE=OPEN
GAUSSIAN_PRIME_SPLIT_DERIVED_FROM_MOORE_PARITY=FALSE
PRODUCTION_CHANGED=FALSE
NO_NEW_SELECTED_TYPE=TRUE
BORN_BELL_CONTEXT_USED=FALSE
```
