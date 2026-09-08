# Registered mixed field/relation response, version 1

**[SELECTION] finite deterministic preparation; [OPEN] material recovery.**
This protocol interrogates `phi-v2-staged-candidate-1` without changing its law,
tables, schedule or accepted CUDA kernels. Both a positive response and a
negative result are retained; cases and horizon will not be tuned to outcomes.

## Fixed matrix

Ten cases use periodic `L=17`, initial microtick zero, uniform `ell=0` and
explicitly lagged `s=0`. Every SC and FCC relation anchor initially contains
one positive phase-zero token in its primary slot; every reserve is blank.
Five relation preparations are crossed with two field preparations:

1. Homogeneous reference.
2. At owner `(8,8,8)`, SC axis 0: primary phase zero becomes phase one.
3. At the same SC anchor: move the phase-zero token to the reserve slot.
4. At owner `(8,8,8)`, FCC plane 0, diagonal 1: primary phase zero becomes
   phase one. This diagonal's endpoints are `(8,9,8)` and `(8,8,9)`;
   neither endpoint is its stored owner.
5. At that FCC anchor: move the phase-zero token to the reserve slot.

Each preparation has either no fields, or one positive phase-zero field token
at every site of the fixed cube `{7,8,9}^3`. All 27 field tokens use the first
canonical channel with tangent `(+1,0,0)`, phase zero and positive polarity.
No random seed or channel selection by measured outcome is used. These are
27 exactly specified tokens, not a continuum density or a thermal ensemble.

The horizon is **16 physical microticks** (four complete stage cycles), for
160 measured microticks total. The resource cap is 1 GiB of campaign artifacts
and 30 minutes for GPU execution; resource failure makes the campaign
incomplete. The CPU/Python audit is separate from the GPU runtime limit.
The ten complete initial checkpoints and the registration, source closure and
executable SHA256 identities are locked before the first measured GPU step.

## Paired questions and falsifiers

- **Field to relation:** for each fixed relation preparation, compare its
  27-field and field-free cases. Does any SC/FCC record differ after the first
  local-relation stage? A difference in saved gates alone is a control-bit
  influence, not yet a change to relation payload.
- **Relation to field:** compare each defect with its homogeneous relation
  reference using the same 27-field patch. Does any field-bank slot differ
  after evolution? Initial field banks must be byte-identical in these pairs.
- **Propagation:** does a defect cause relation-payload differences at stored
  owners beyond its original owner, or field differences beyond that owner?
  Record the full support and time of first occurrence, including null results.
- **Field-free control:** compare each defect with the homogeneous field-free
  reference. This distinguishes local relation dynamics and manifestation
  endpoint effects from field-mediated changes at other relation anchors.

Every pair has equal global clock and schedule. Differences cover **all eight
complete arrays**, including lagged manifestation, collision clock and pending
controls. Array-entry Hamming counts, exact owner supports and maximum periodic
Moore distance from the original difference set are retained at every tick.
Changed FCC records are located at stored owners for the radius-one support
test; their physical endpoints are separately reported for interpretation.

The transition support condition is
`D(t+1) subset Moore1_periodic(D(t))` for the union of all complete-record
owner differences. It is checked at every microtick, not just cycle boundaries.
This is a finite regression check of the established stage support argument,
not a universal causality proof inferred from ten trajectories.

## Periodic boundary and interpretation limits

A radius-16 Moore cone covers this finite torus. Therefore the generic cone
bound does **not** certify absence of periodic contamination. Record whether
actual differences reach the coordinate seam, but treat this as an observed
support diagnostic only. All conclusions are explicitly about the periodic
candidate. This protocol supplies neither an undefined-boundary equivalence
proof nor a translated-domain or growing-domain convergence test. Such a test
requires a new preregistration; no finite-torus recurrence establishes binding.

Bidirectional record influence is a mechanism result. It is not evidence that
a persistent local structure transports as a body, possesses mass, binds,
generates gravity or has a closed continuum description. M1/M2 and continuum
recovery flags remain false regardless of the measured signal amplitudes.

## Instrument and acceptance

The standalone `engine/strict/recovery_mixed` runner calls the existing real
CUDA backend for every physical tick through WSL2 Ubuntu-22.04. It records all
complete checkpoints, per-tick SHA256 digests and complete event logs. It also
advances a separate accepted native CPU reference and requires exact complete
state bytes and event equality at every tick. CPU states never replace GPU
states or constitute a GPU fallback.

The Python launcher checks the mandatory source-closure key set, all frozen
source and binary hashes, registration, case inventory and every input before
and after execution. Preflight, lock, execution and trace identities are bound
by SHA256. The auditor independently regenerates every preparation and every
microtick using the Python staged law, compares all complete-state bytes and
events, then computes paired differences from retained actual GPU snapshots.
Missing events, missing states, wrong clocks, invalid records, incomplete
locks, hash mismatches or failed parity reject acceptance. No receipt flag
alone substitutes for actual transition comparison.

Independent review of the written protocol and instrument precedes campaign
execution. Frozen carrier v2 files and all accepted law/kernel sources remain
unchanged. A repair after lock requires a new campaign directory and honest
instrument provenance; scientific criteria cannot silently change.
