# FTD-0618 — Closed symmetry-balanced gait v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`
**Scope:** observer-only six-constituent test of the FTD-0617 parity law
**Production change:** forbidden

## 1. Frozen parent and action

Require the FTD-0617 run-of-record SHA-256
`DABFBE348F9714E8B1F5EAF78D1EB06744A3BAE22D2BA4C9FBB2D2C5099995C0`
and reproduce the exact FTD-0612 rest core, FTD-0615 mode-zero tangent,
excitation energy `4 Delta_ref`, multiplicity-two chart, and existing FTD-0601
six-constituent common-action solver. No force, binding, field normalization,
solver tolerance, or interaction is changed.

The uniform neutralizer is forbidden. Construct one net-neutral state in the
unchanged `L=17` periodic volume from a charge `-1` core and its
charge-conjugate `+1` copy. Initialize the one shared electric field as the
minimum-energy periodic Gauss solution of the complete six-constituent
density; initialize the magnetic field at zero.

## 2. Locked half-turn construction

Let `R=diag(-1,-1,+1)`, the proper half-turn about the lattice `z` axis through
`(x,y)=(15,9)`. Keep core A at the exact FTD-0612 position and set every core-B
effective position and momentum to the `R` image about that axis. Reverse all
three B charges. This gives a charge-conjugate, half-turn-related pair at a
near-maximal periodic separation without changing A's registered subcell
phase.

For active sign `sigma=+1,-1`, assign

```text
p_A,a = sigma A u0,a,
p_B,a = R p_A,a,
```

where `A` is independently solved for the unchanged excitation energy
`4 Delta_ref`. Each core and the pair must have zero initial centre momentum
within `1e-12`. Add one zero-momentum rest arm. No angle, phase, separation,
or amplitude may be varied after execution.

## 3. Histories and observables

Run each of the three arms for 128 forward ticks followed by 128 state-only
inverse ticks: 768 common-action transactions. At every forward tick record:

- both core centres and the pair centre;
- both core momenta and total matter momentum;
- pair-centre displacement and the half-turn centre residual;
- internal pair distances, anchor multiplicity, common-action residual, and
  total-energy drift;
- one-step pseudomomentum defect and cumulative total pseudomomentum drift.

The half-turn centre residual compares core B with the periodic `R` image of
core A after removing their common axial displacement. It is a constituent
symmetry check, not a field-symmetry theorem.

## 4. Algebraic and balance gates

Every arm must complete all 256 transactions, keep each trimer's internal
pair distances in `[0.5,2.0]`, maximum anchor multiplicity at two,
common-action residual at most `1e-12`, total-energy drift at most `1e-10`, and
state-only recovery at most `1e-8`. The initial net charge must be exactly zero
in integer arithmetic and no stationary density may be supplied.

For the active arms require:

```text
max transverse pair-centre displacement <= 1e-8,
min |axial pair-centre displacement|      >= 0.5 cell,
|d_z(+) + d_z(-)|                         <= 1e-8,
max half-turn centre residual             <= 1e-8.
```

The rest-arm pair-centre displacement must remain below `1e-8`. These are
absolute locked gates; no rest subtraction or post-hoc vector addition is
allowed.

The isolated momentum gate is a separate conjunction: maximum cumulative
change in the solver's declared field-plus-matter pseudomomentum must be at
most `1e-10`. It may not be repaired by redefining momentum after execution.

## 5. Verdicts

- `CLOSED_SYMMETRY_BALANCED_GAIT_CONSTRUCTIVE`: all algebraic, rest, balance,
  sign, symmetry, and isolated-momentum gates pass;
- `SYMMETRY_BALANCED_GAIT_KINEMATIC_MOMENTUM_OPEN`: every gate except the
  isolated-momentum gate passes;
- `SYMMETRY_BALANCED_GAIT_NOT_CONSTRUCTIVE`: all arms and algebraic gates pass
  but rest, transverse cancellation, axial transport, sign reversal, or
  half-turn symmetry fails;
- `CLOSED_SYMMETRY_BALANCED_GAIT_NUMERICALLY_UNRESOLVED`: any parent,
  reconstruction, initialization, solve, inverse, record, or algebraic gate
  fails.

Even the strongest verdict establishes only a selected closed lattice-matter
mechanism. It does not establish a physical particle, inertial motion, a mass
shell, microscopic electromagnetism, or derivation from the five postulates.

**Protocol lock:** `protocol_sha256=C8D6D2550A38BA01FAA52CDDB37A152AA0EB6D258BFBA8C1AA092B1973387A73`

**Clerical correction after first execution:** the initially displayed hash
`166BFBCB...DBB9A` omitted the literal `**Protocol lock:** ` label from the
hashed prefix. The protocol text, parent, arms, tolerances, and verdict rules
above are unchanged. `C8D6D255...7A73` is the correct prefix hash under the
project convention. The complete campaign must be rerun with the corrected
executable metadata before certification.
