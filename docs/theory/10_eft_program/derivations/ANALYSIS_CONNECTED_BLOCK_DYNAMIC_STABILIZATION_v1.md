# FTD-0625 — Rigid circulation does not stabilize the connected block

> **FTD-0626 successor correction:** circulation reaches the strict chart
> boundary sooner, but it does not reach a demonstrated reaction surface.
> Under the existing multiplicity-two fibre every registered circulation arm
> executes and reverses while the aliased constituents remain spatially
> distinct.

**Status:** `[SELECTED DYNAMICS] + [MEASURED — EXACT SIGN/CUBIC FAILURE
CLASS] + [CLOSED NEGATIVE — REGISTERED RIGID-CIRCULATION STABILIZER] +
[OPEN — ATOMIC REACTION OR NEW NONANNIHILATING STRUCTURE]`  
**Protocol SHA-256:**
`E95F2EB5A91C599AEFF790F55A34E548628D95FE247E95C843446A6940E751CA`  
**Parent:** FTD-0624 JSON SHA-256
`55D34381B4968653740DF57A0F2330A3D175CC2CFD52012A2C4657D601825653`  
**Verdict:** `RIGID_CIRCULATION_DYNAMIC_STABILIZATION_CLOSED_NEGATIVE`  
**Production status:** unchanged

## 1. Result

Zero-total-momentum rigid circulation does not keep the connected neutral block
away from the FTD-0624 occupancy surface. At circulation energy equal to the
parallel Peierls barrier, both signs and the cyclic control fail unique ternary
site projection at tick two. At four times that energy they fail at tick one.
Every failed nonlinear root converges between `6.93e-15` and `1.76e-14`; the
registered graph remains valid. The failed gate is the accepted endpoint's
site projection.

Every failure contains exactly two anchor conflicts. All are opposite-polarity
pairs; none is same-polarity packing. Increasing circulation energy brings the
conflict earlier rather than delaying it.

## 2. Model-internal launch

The two amplitudes are fixed by the production dispersion and the measured
parallel Peierls barrier, not fitted:

| excess circulation energy | amplitude `A` | initial `|L_int|` | failure tick |
|---|---:|---:|---:|
| `B_x` | `0.008084307776` | `0.1940233866` | 2 |
| `4 B_x` | `0.01617846399` | `0.3882831358` | 1 |

Every launch has total matter momentum below `1e-14`. Reversing circulation
reverses internal angular momentum but leaves the scalar failure history
unchanged to `3.67e-15`. Cyclic covariance closes at `6.25e-13`.

## 3. Control

The zero-circulation near-half control completes all 16 forward and 16 state-
only reverse steps. It remains conflict free, reaches minimum chart margin
`0.0047332820`, drifts total energy below `7.87e-14`, and recovers the complete
state to `4.62e-14`.

The one-barrier circulating arms briefly have a larger margin
`0.0068919568` after tick one, but fail at the next endpoint. They therefore do
not satisfy the locked improvement criterion; instantaneous separation along
one coordinate is not stabilization of the complete ternary projection.

## 4. Ontological consequence

The present block cannot be understood as a classical neutral lump stabilized
by ordinary rigid rotation. Its failure is not missing total angular momentum,
insufficient rotational energy, or a preferred circulation sign. The failure
is sign-even, cubic, and becomes faster at higher internal kinetic energy.

The measured conflicts strengthen the FTD-0624 reading: the connected block's
local `+/-` interface is a reaction channel. Circulation stirs opposite
polarities into common manifestation sites; it does not supply a centrifugal
barrier on the full constrained lattice configuration.

This is compatible with the dynamical-pattern ontology in a narrow sense. A
pattern may propagate coherently under finite boost yet decay when its internal
trajectory reaches a local reaction surface. It does not yet demonstrate
decay, because the selected common action currently rejects the conflicting
endpoint rather than transforming it into outgoing field state.

## 5. What is now ruled out and what is not

Closed for the frozen connected block:

- static half-cell reversible rest under independent nearest-site projection;
- zero-total-momentum rigid circulation at `B_x` or `4B_x` as a 16-tick
  stabilizer;
- rescue by circulation sign or cubic orientation;
- interpreting the failure as numerical root nonconvergence.

Still open:

- a non-rigid, topologically organized internal current distinct from solid-
  body rotation;
- an atomic `(+1,-1)->0+field` reaction transaction;
- a site-capacity constraint impulse derived from an additional conserved
  species/topological label;
- an explicit occupancy/temporal-phase fibre;
- native formation, physical mass, spin/statistics, a particle pole, and
  infrared recovery.

## 6. Next gate

The immediate next candidate should be the reaction branch, because every
registered conflict is opposite polarity. At the conflict tick, the atomic
solve must determine simultaneously:

1. which `+/-` pairs disappear from ternary manifestation;
2. their deposited face current and the face/edge field endpoint;
3. outgoing field energy and momentum;
4. any surviving constituent/bond state;
5. whether the complete map is state-only invertible.

If the outgoing field cannot encode the incoming reaction state uniquely, the
many-to-one update is an explicit low-energy unitarity obstruction. If stable
matter rather than decay is desired, a nonannihilating topological/species
label must be independently motivated and priced; it cannot be inferred from
the existing `J_L/J_R` field register.

## 7. Reproducibility

- test SHA-256:
  `22691ECABFD6C226CE03A9D2D3CE3232141897C421B35B0F46D70CCE5DA405EA`
- JSON SHA-256:
  `99F6337B6B210DFBDF08C175DA62F6777FCD20E376D46D57B1A25A94229A4C02`
- arm CSV SHA-256:
  `3BC30ADA092A0317EE8CA865949034150E0862E83208D909C19B08990F7C5DF4`
- tick CSV SHA-256:
  `DD77EC9CC2C9CD30A34C5C4A94739AF530B6B2C095908D23EBC9693579C981DB`
- independent certificate SHA-256:
  `E6AF6BA22A732B8E45B1387A51E9355E53A1E8EBF3232EBFB15DDE926D232816`
- independent certificate: `37/37` checks pass
