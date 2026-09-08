# Phase-directed alignment successor proposal, version 1

Date: 2026-09-07. Proposal identity: `phi-alignment-successor-proposal-v1`.
Status: **[PROPOSAL -- SELECTED FINITE MAP; NOT IMPLEMENTED OR ADOPTED]**.

This document specifies a possible research successor to
`phi-v2-staged-candidate-1`. It changes no existing law, table, checkpoint,
backend, dashboard capability or canonical claim. The
[current candidate](SPEC_PHI_STAGED_CANDIDATE.md), its frozen evidence and the
[formation obstruction](AUDIT_STRICT_PAIR_FORMATION_OBSTRUCTION.md) retain
their identities and dispositions. Canonical adoption remains a separate
decision governed by the project LEDGER and active constitution.

The proposal selects a local alignment transaction that permits entry into
the known co-located same-flag pair sector. It adds no record or physical
stage, but introduces a new, explicitly non-injective collision expiry.
Formation is designed into this microscopic rule; it is not claimed to be
forced by the postulates or independently recovered physical binding.
The existing displacement counterexample survives. Atomic, molecular,
continuum, inertial and gravitational recovery remain open.

No successor runtime identifier, generated table hash, checkpoint schema or
backend artifact exists yet. The identity above names this proposal only.

## 1. Chosen route and exact local map

This is a field-record proposal under Route A of the
[successor requirements](SPEC_STRICT_RECOVERY_SUCCESSOR_REQUIREMENTS_V1.md).
It leaves relation support, absorption and emission behavior unchanged. It
does not satisfy Route B's mobile, emitting relation-constituent requirements.

A channel of one fixed polarity has internal state

```text
c = ((d,n,h),k),
d,n = perpendicular signed SC unit directions,
h in {-1,+1},
k in Z/4Z.
```

There are 48 flags and four phases, hence 192 internal channels per polarity.
The tangent, axial normal and pseudoscalar handedness conventions are those
of [the flag source](../../scripts/proofs/proof_shared_edge_hodge_flag_bcc_propagation.py).
Let `C_q` denote the frozen collision table at input layer `q`, and `U` the
existing complete internal channel permutation. Maps below act on unordered
pairs of distinct channels of one polarity.

Define the base eligible domain

```text
D_0 = {
  {((d,n,h),k), ((d,n,-h),k+1)}
  : d perpendicular n, h in {-1,+1}, k in Z/4Z
}.
```

An unordered adjacent phase pair has a unique predecessor `k` under the
chosen `+1` orientation of the four-cycle. Define the pre-map `A_0` by

```text
A_0({((d,n,h),k), ((d,n,-h),k+1)})
    = {((d,n,h),k), ((d,n,h),k+1)}.

A_0(a) = a for a outside D_0.
```

The output retains both phase values and two distinct occupied channels.
The predecessor's handedness is copied to the successor before the existing
collision acts. This predecessor choice is a **[SELECTION -- NEW RULE]**;
no claim of its uniqueness follows from the postulates.

The complete replacement collision family is

```text
C_0' = C_0 composed with A_0,
C_q' = U^r C_0' U^(-r),       r = (-q) mod 3.
```

Thus `r=0,2,1` for layers `q=0,1,2`, respectively. This sign and ordering
match the existing law's decrement of the collision layer. The conjugation
acts on both channels in the unordered pair.

At each site, apply `C_q'` independently to a polarity bank if and only if
that bank contains exactly two occupied channels. Zero, one, or at least
three occupied channels retain the current no-collision behavior. Two
opposite-polarity tokens do not constitute an eligible pair. If both polarity
banks qualify, each receives its own map into its disjoint channel range.

These rules specify every local bank occupancy. The pre-map and composition
define a single finite collision lookup, not sequential physical substeps.
Only the pre-state bank and its input layer are collision inputs; A9 values,
saved gates, admission controls, diagnostics and external histories are not.

## 2. Complete records and unchanged physical schedule

Retain the full state and validation contract of
[`StagedState`](../../scripts/phi_v2_lattice/staged.py), including all eight
arrays, the global ordinal, law identity, encoding and boundary convention.
The finite inventory per stored site/owner is:

| Record | Finite cardinality or storage count |
|---|---|
| Lagged manifestation `s` | 3 symbols |
| Collision layer `ell` | 3 symbols |
| Field bank | 384 Boolean exclusion slots |
| SC payload | 3 anchors, 2 A9 slots each: 6 slots of 9 symbols |
| FCC payload | 3 planes, 2 diagonals, 2 A9 slots: 12 slots of 9 symbols |
| `admitted_sc` | 3 Boolean controls |
| `gate_sc` | 3 Boolean controls |
| `gate_fcc` | 6 Boolean controls |

No new ontic identifier, inverse tape, reservoir, pending payload, random
source or controller is added. Before schedule-admissibility restrictions,
these local factors have cardinality `9^19 * 2^396`. At phase zero the twelve
pending controls must be clear; existing admitted-mask/payload consistency
restrictions remain in force at intermediate phases. This factor count is
an alphabet price, not an assertion that every formal tuple is reachable.

The global nonnegative ordinal remains explicit; it is not claimed to be a
finite local alphabet. Initialization retains the current valid finite
payload domain and clears pending records. The preparation convention permits
an initially lagged `s`; subsequent values follow actual evolution.
The initial implementation domain remains finite periodic integer `L>=3`.
No undefined-boundary execution result is claimed.

| Input phase | One physical microtick |
|---|---|
| 0 | Save endpoint parity gates and perform current unique SC admission |
| 1 | Apply `C_ell'`; perform current local relation updates using saved controls; decrement `ell` |
| 2 | Apply current `U`/manifestation half-turn and one-hop streaming |
| 3 | Compute current incidence manifestation and clear pending controls |

Every pending record remains part of the checkpoint. Additional physical
microticks: zero. Existing wave-speed or continuum calibrations are not
inherited merely because the number of schedule phases is unchanged.

## 3. Local support, accounting and symmetries

The replacement collision reads and writes only its owner's bank and input
layer. Its support is radius zero. Distinct owner/polarity banks have
disjoint writes, and each qualifying bank is replaced by exactly two distinct
channels. Local arithmetic used to generate a lookup table is not a hidden
neighbor-dependent transaction.

All other stages retain the source dependencies established in the
[staged-candidate specification](SPEC_PHI_STAGED_CANDIDATE.md): admission and
saved gates read their declared endpoints, streaming crosses one SC hop, and
manifestation reads incident owners within the Moore neighborhood. These
source arguments must still be independently checked against any future
implementation, including every intermediate control array.

Token count and each polarity's combined field/relation inventory remain
exact at every physical stage. The collision adds no transfer between sites
or between field and relation storage. The existing inventory continuity
transactions and their boundary ownership remain applicable to this change
in channel labels. Their effective physical interpretation is not upgraded.
In particular, token count is not identified with energy, mass or heat.

At layer zero the canonical pair moment uses `v_0(c)=(d,n)` and is independent
of phase and handedness. Therefore `A_0` preserves its six-component sum.
The old `C_0` preserves the same sum, and the existing layer covariance
transports the result to each input layer `q`:

```text
sum_(c in a) v_q(c) = sum_(c in C_q'(a)) v_q(c).
```

This is **input-layer collision-moment preservation only**. In the actual
staged runtime, `ell` is decremented during the collision microtick, before
`U` occurs in the streaming microtick. It does not follow that moments
re-evaluated at the committed `ell` are conserved at every microtick. The
original candidate has the same staging distinction. New fixed additive
invariant ranks or full-state physical conservation laws require their own
proofs; none is inherited by this six-component identity.

For the collision map, `A_0` commutes with the signed cubic group `O_h`:
transformations preserve the common tangent/normal and transform the selected
handedness as a pseudoscalar. It also commutes with uniform C4 phase shifts,
which preserve the unique adjacent-phase predecessor. Since `U^3` is a uniform
phase shift, the conjugated family closes consistently and obeys

```text
U C_q' = C_(q-1)' U.
```

The C4 claim concerns the collision family. It is not a new C4 symmetry claim
for the full runtime: existing absorption eligibility, for example, selects
absolute phase two.

Phase reversal is an explicit price. With `R:k -> -k mod 4`, the orientation
of an adjacent pair reverses and `A_0` selects the other handedness. The full
base replacement also fails this symmetry in a concrete example:

```text
C_0'({0,5}) = {4,5},
R C_0'({0,5}) = {4,7},
C_0' R({0,5}) = C_0'({0,7}) = C_0({4,7}) = {0,3}.
```

Here `C_0({0,3})={4,7}` and the frozen `C_0` is involutive. No phase-reflection,
time-reversal or full-law reversibility claim is made for the proposal.

## 4. Exact changed support and named expiry

Let `P_0` be the same-flag adjacent-phase pair orbit. Both `D_0` and `P_0`
contain `24 * 2 * 4 = 192` unordered pairs. The restriction
`A_0:D_0 -> P_0` is bijective. The noninjectivity occurs because `A_0` also
leaves `P_0` fixed: one `D_0` input and one already aligned `P_0` input share
each output. It is not a merger of two distinct eligible `D_0` inputs.

The frozen collision generator installs self-orbit maps under `O_h x C4`.
Consequently `C_0` permutes each of these two orbits separately. It follows
that `C_0'` has exactly two preimages for every `P_0` output, none for every
`D_0` output, and one for every other pair output. Conjugation gives the same
fiber statement at every layer. The rule changes precisely 192 rows per
layer, hence 576 internal table entries, used by both polarities.

Name the new loss **relative-handedness expiry at collision**. Across
`D_0 union P_0`, the erased distinction is whether the successor's handedness
was originally equal or opposite to the predecessor's. It is one binary
preimage distinction per merged output. This is an exact finite information
price, not thermodynamic entropy production or a heat account. Existing
absorption and control-clear expiry remain separate named losses.

Before/after diagnostic events may retain the distinction for an observer.
Such external history never feeds back into the law. Forward continuation
from a complete successor checkpoint remains deterministic; reconstructing
an expired past is not promised. A proposal that later retains an inverse
bit would be a different complete-state law with a new ownership/transport
and expiry contract.

## 5. Restricted minimum changed-row support

The 576-row count is minimal **only** within the following comparison class:

- A collision lookup depending on the unordered same-polarity pair and `q`
  only; no A9 payload or pending-control input.
- The same alphabet, exactly-two eligibility and other physical stages.
- Unchanged same-flag collision transitions.
- `O_h x C4` collision equivariance and the stated three-layer conjugation.
- Preservation of the six pair moments evaluated at input layer `q`.
- At least one unequal-flag input mapped to a same-flag output.

At layer zero a same-flag output has moment `(2d,2n)`. Since the incoming
tangents and normals are signed SC unit vectors, equality of these sums
forces both input tangents to be `d` and both input normals to be `n`.
An unequal-flag input can then differ only in handedness.

Reflection normal to `n` fixes polar `d` and axial `n` while reversing `h`.
For equal input phases this reflection stabilizes the unordered input; for
opposite phases, its composition with phase shift two stabilizes the input.
Neither operation can fix a same-handed output, because both output flags
have their common handedness reversed. Equivariance therefore forbids both
phase cases from entering the same-flag sector.

Adjacent phases remain. They comprise the single 192-pair orbit `D_0`.
If one of its rows enters the same-flag sector, equivariance requires all
192 rows to do so; all differ from the original self-orbit map. Layer
conjugation then requires 576 changed rows. The proposed map attains that
bound.

This is minimum changed-row support within a restricted table class, not a
globally minimal ontology, a unique successor, or minimum physical cost.
Allowing other existing records as collision inputs could break the stated
stabilizers and invalidates this minimality argument.

## 6. Analytic formation witnesses -- not executed

Both witnesses use the registered finite periodic background: zero initial
manifestation, a uniform collision layer, and both slots of every SC/FCC
relation set to one common nonblank A9 code. Pending arrays initially vanish.
Both relation slots remain occupied, rotate independently of saved gates,
and cancel manifestation. Admission is disabled. Gates retain their actual
values; no independent replacement of these controls is assumed.

The channel ordering and existing table entries are read from the unchanged
[channel source](../../scripts/phi_v2_lattice/channels.py) and
[native frozen transcription](../strict/frozen_tables.h), whose packing is
defined by [its exporter](../strict/generate_tables.py). No successor table
was generated and no trajectory was executed to produce the statements below.

### 6.1 Contact-to-co-transport witness

Channels zero and five are

```text
0 = ((-e_x,-e_y,-1),0),
5 = ((-e_x,-e_y,+1),1).
```

Prepare them together at a site `x` at the initial cycle boundary, layer
zero. The fixed regression uses `L=7` and `x=(3,3,3)`; the local argument
also applies to other admitted periodic placements. The existing first table
entry is `773=4*192+5`, encoding
`C_0({0,1})={4,5}`. Thus

```text
A_0({0,5}) = {0,1},
C_0'({0,5}) = {4,5}.
```

Tick one leaves the fields unchanged. Tick two aligns them and produces the
same complete output as the original law's prepared `{0,1}` pair: all
payloads, layers and pending gates agree. Tick three uses `U(4)=93` and
`U(5)=94`, streaming both to `x-e_x`. The original preparation's subsequent
same-flag dynamics are unchanged by the successor.

For comparison, the original `C_0({0,5})={1,4}` retains unequal flags. The
new complete-state convergence at tick two is also a direct witness of the
named expiry, pending executable verification.

### 6.2 Initially separated witness

Use `L=7`, initial layer two and the same background. Prepare channel 113 at
`(3,2,3)` and channel 94 at `(3,4,3)`. The first collision stage has only one
token at either site and does nothing to either bank.

| Physical tick | Analytically specified field state |
|---|---|
| 0 | Channel 113 at `(3,2,3)`; channel 94 at `(3,4,3)` |
| 3 | Both at `(3,3,3)` with channels `{167,170}` |
| 4 | Same fields at a cycle boundary; uniform layer one |
| 6 | `C_1'({167,170})={166,167}` at `(3,3,3)` |
| 7 | Both at `(3,3,4)` with channels `{4,7}` |

The identities are `113=U(0)`, `94=U(5)`,
`{167,170}=U^2({0,5})` and

```text
C_1'(U^2({0,5})) = U^2(C_0'({0,5}))
                 = U^2({4,5}) = {166,167}.
```

The old collision instead gives
`U^2(C_0({0,5}))={166,171}`. Under the proposed map, the predicate

```text
P = co-located sites and equal complete flags
```

is false through tick five, becomes true at tick six and then persists.
Persistence follows from the unchanged same-flag maps and common streaming
flag update. These are analytic claims about the specified candidate, not
accepted runtime measurements or an undefined-boundary equivalence result.

## 7. Continuum reference obstruction and surviving material failure

The new collision does not preserve the previous full-support Bernoulli
bank reference. At one site and one polarity, let all 192 channel bits have
independent occupation probability `0<p<1`. Every exactly-two pair then has
probability

```text
w = p^2 (1-p)^190.
```

Under one collision, each of the 192 `D_q` bank states loses probability
`w`, and each of the 192 `P_q` states gains `w`. Other bank probabilities are
unchanged. Hence the exact bank-marginal total-variation departure is

```text
TV = (1/2) * (192 w + 192 w)
   = 192 p^2 (1-p)^190 > 0.
```

This statement concerns one site's one-polarity bank marginal at one
collision, not the TV distance between complete multi-site states at an
arbitrary horizon. Existing saved-gate correlations do not restore the bank
stationarity that this marginal already disproves.

The old [stationary reference](DERIV_STRICT_KINETIC_REFERENCE.md), full-score
isometries and [response bounds](DERIV_STRICT_KINETIC_RESPONSE.md) therefore
cannot be reused for this successor. In particular, the previously accepted
multi-microtick tangent and finite-amplitude budgets apply only to their
frozen original law. Retaining its schedule or six input-layer moments does
not transfer those estimates. A successor campaign needs a newly specified
reference evolution, leakage/accounting bounds and fixed physical-time
horizon; it may not substitute the old horizon or reference after a failure.
The unchanged exactly-two gate also retains the half-occupation feasibility
problem. At the specified initial Bernoulli preparation, `192w` is the
alignment-eligible `D_q` probability; total exactly-two eligibility is
`18336w`. Neither one-step probability may be multiplied by an arbitrary
collision-stage horizon without a new ensemble-evolution argument. Neither
establishes mixing, equilibration or useful relaxation.

The [same-flag displacement failure](DERIV_STRICT_PAIR_SECTOR.md) remains an
exact negative control. Separate the aligned constituents by any nonzero
periodic offset while retaining their channels. Their flags evolve equally,
so they keep the same separation and never collide. The new local rule never
fires. Thus formation into this pair family is enabled, but restoring
binding is still absent. No equilibrium pair species, dissociation energy,
inertial response, radiation, spectrum, chemical bond or gravitational source
is established.

## 8. Locked next gates and review disposition

Before implementation acceptance, allocate fresh law, table, checkpoint and
backend identities. Reject original-law checkpoints as successor checkpoints;
an explicitly specified preparation conversion, if later needed, must retain
its own provenance. Preserve all original sources, frozen manifests and
obstruction evidence. No existing runtime capability is promoted by this
proposal.

The next finite certificate must check all 55,008 internal collision rows,
the 192/192 orbit domains and their exact fibers, the 576-row difference,
distinct output channels, both polarity copies, input-layer moment sums,
cubic/C4 covariance and the three-layer conjugation. Check the phase-reversal
failure as a declared property rather than suppressing it. Independently
review the restricted minimality proof and every stage's causal support.

The first successor runtime validation is fixed to the two witnesses above
and their translated, cubic-oriented, phase-shifted and negative-polarity
counterparts on the stated periodic domain. For this finite regression lock,
inspect every complete array and actual event through 56 charged microticks
from each initial preparation. Require the stated contact and initially
separated formation times, exact token accounting, no unregistered admissions,
and equality with original-law continuation from the corresponding aligned
complete state. Include seam placements and all eight nonblank homogeneous
background codes. The fixed horizon covers a full 48-microtick internal
recurrence after either formation event; recurrence is not a binding test.

Also check the local contact witness at every layer by conjugating its input
pair with the specified `U^r`. This is a collision-table transformation;
`U` is not asserted to be a spatial symmetry of complete streamed histories.
Do not keep the separated preparation's coordinates fixed while changing
its channels by `U` and assume the same approach trajectories or formation
time. Any additional layer-dependent spatial approach must be separately
specified before execution.

Additional controls must cover zero/one/three-or-more occupancy, opposite
polarities, simultaneous qualifying polarity banks, all schedule phases,
owned snapshots, nonmutation, corrupt-state rejection and deterministic
checkpoint restoration. An explicit expiry witness must show the two
different pre-states merging while forward replay stays exact. A complete
inverse must not be asserted. The separated same-flag non-restoring history
is compulsory: it must continue to fail the physical binding criterion.

Reject the design's declared claims if a finite row, stage dependency,
accounting identity or registered witness fails. Preserve that failure and
version any repair. Backend ports require complete-state/event parity and
their own lifecycle gates after the finite source law passes. No positive
material or continuum campaign is authorized by table correctness alone.

Independent mathematical agent `formation_alignment_redteam` performed
provisional read-only source/design scrutiny separately from the design
author. It agreed with the conjugation and collision covariance arguments,
sharpened the minimum-support class and two-preimage expiry statement, and
identified the exact TV drift and input-layer-only moment limitation. Those
limitations are incorporated above. This is an AI-agent design review, not
external human approval, an executed successor certificate or release
acceptance. The two formation witnesses remain analytic and unexecuted.

**Disposition:** a concrete, finitely specified route past the existing
formation preimage obstruction is available at a declared expiry price.
Its restoring-response failure and new continuum-reference obstruction are
retained. Any further recovery requires another explicit mechanism and a
separately reviewed gate; canonical adoption never occurs implicitly.
