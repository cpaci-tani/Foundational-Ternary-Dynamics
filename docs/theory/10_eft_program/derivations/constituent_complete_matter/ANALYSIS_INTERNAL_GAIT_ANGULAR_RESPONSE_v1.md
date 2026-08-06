# FTD-0617 — Internal-gait angular response

**Status:** `[MEASURED — MIXED-PARITY ANGULAR GAIT RESPONSE]` +
`[MEASURED — AXIS/DIAGONAL TRANSPORT SELECTION]` +
`[OPEN — SYMMETRY-BALANCED COMPOSITE / PHASE RECURRENCE / CLOSED MOMENTUM]`
**Verdict:** `MIXED_PARITY_INTERNAL_GAIT_RESPONSE_RESOLVED`
**Production status:** unchanged

## 1. Result

The complete preregistered eight-angle circle in the two-dimensional
rotational tangent plane passes. All twelve main/covariance arms complete 256
forward and 256 state-only inverse ticks, for 6,144 common-action
transactions. The maximum DFT reconstruction residual is `3.85e-15`, maximum
whole-history cyclic-covariance residual is `1.14e-13`, worst common-action
gate is `1.98e-13`, and maximum state recovery error is `7.43e-11`.

The measured parity RMS values are

```text
R_even = 0.859602098154457
R_odd  = 1.515800885988654
```

so both registered response sectors are nonzero. Across the full circle, the
antipodal-even displacement is planar to `2e-9` and the antipodal-odd
displacement is axial to `1e-8`. This extends the FTD-0616 parity observation
from two basis modes to every sampled direction in their span.

## 2. The angular law is not linear

The four axis samples move `2.4558566554...2.4558566556` cells. The four
diagonal samples move only `0.1008852454...0.2713498021` cell. Equal initial
excitation energy therefore does not produce an angle-independent speed or a
linear velocity coordinate.

The exact eight-point decomposition has vector-coefficient norms

```text
k=0: 0.2686722508
k=1: 0.7513474338
k=2: 0.5197423031
k=3: 0.7643972767
k=4: 0.3556218012
```

The sampled third odd harmonic is slightly larger than the sampled first
harmonic. The diagonal suppression is produced by interference among these
registered components, not by loss of the compact core. Because an eight-point
DFT aliases angular harmonics modulo eight, `k=3` here is a finite discrete
response component. It is not a continuum cubic Taylor coefficient and does
not establish a universal analytic mobility law.

## 3. Ontological consequence

The internal rotational plane is a nonlinear control manifold for the
selected compact excitation, not ordinary point-particle velocity space. Its
state already contains enough information to distinguish:

1. an antipodal-even, body/lattice-dependent planar drift;
2. an antipodal-odd axial transport channel;
3. strong angular interference between those response components; and
4. exact proper-cubic covariance when the complete body and field are rotated.

No new primitive is forced by this map. It instead strengthens the conclusion
that a centre-only effective law is incomplete: internal orientation, its
conjugate momentum, body orientation, and field dressing are dynamically
load-bearing. The closest mechanical analogy is a symmetry-constrained gait
on a discrete substrate, not a prescribed force and not a freely chosen
velocity vector.

## 4. Boundary of the claim

The campaign is a complete preselected angular map, not a post-hoc favorable-
angle search. It does not establish a self-propelled isolated particle. The
uniform neutralizer is external, the maximum field-plus-matter
pseudomomentum defect is `1.215e-3`, phase recurrence is unmeasured, and the
transport remains compact and Peierls-sensitive. A physical mass shell,
momentum conservation, inertial straight motion, electromagnetism, and an
infrared pole remain open.

## 5. Next discriminator

The measured parity law fixes the next construction before execution. A proper
half-turn about the odd-response axis reverses the planar even component while
preserving the axial odd component. The next test must therefore place two
symmetry-related compact cores in one common dynamical field and initialize
their internal gaits by that half-turn. It must test whether the combined
system cancels transverse drift while retaining axial transport and whether
the field/environment carries the balancing momentum.

Adding two independently simulated displacement vectors after execution is
not this test. The pair must transact as one six-constituent system with all
cross-interactions present, a state-only inverse, and a closed or explicitly
measured momentum ledger. Failure would show that the apparent balance is only
a one-body response symmetry, not a composite matter mechanism.
