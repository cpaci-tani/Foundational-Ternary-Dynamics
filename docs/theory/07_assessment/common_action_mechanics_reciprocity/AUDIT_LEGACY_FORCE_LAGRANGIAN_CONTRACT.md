# AUDIT — Legacy force/Lagrangian contract

**Date:** 2026-07-24  
**Identifier:** `FTD-0442`  
**Status:** `[BLOCKING CODE/CLAIM INCONSISTENCY]`  
**Scope:** production divergence-gradient force, declared discrete action, and EL tests

## 1. Defect

The declared interaction, helper force, production force, and residual checker
do not implement one Euler–Lagrange contract.

| location | implemented or claimed relation |
|---|---|
| `engine/include/ftd/lagrangian.h`, `coupling_term` | `L_int=+G_C s div(J)` |
| same file, `coupling_force` | `F=+alpha s grad(div J)` |
| `engine/src/render_bridge_phases/phase_forces.cpp`, legacy branch | `F=-alpha s grad(div J)` |
| `engine/src/lagrangian.cpp`, `compute_particle_el_residual` | expects the same negative production formula |

The sign amendment dated 2026-07-18 was applied to the declared interaction and
helper but not to the production legacy branch. The residual checker retained
the old sign and therefore confirms the implementation against a duplicated
formula rather than against the declared action.

## 2. Independent variation

Under the smooth-position sampling assumption used by the prose, a point source
at `R` has

$$
L_{int}(R)=G_Cq\,(\nabla\cdot J)(R).
$$

Its coordinate variation gives

$$
\frac{\partial L_{int}}{\partial R}
=+G_Cq\,\nabla(\nabla\cdot J)(R).
$$

Thus the declared term implies a positive sign and a single vertex factor
`G_C`, not the production `-alpha`. The second `G_C` needed for an effective
two-body strength of order `alpha=G_C^2` comes from the other source vertex when
the field equation is solved; inserting `alpha` again in the probe force counts
an additional coupling unless `J` is explicitly renormalized. No such
normalization is present in the production field variable.

There is a deeper qualification: the actual state field is site-valued and a
voxel hop changes `s` discontinuously. The code does not define an interpolation
of `s(R)` or an exact discrete virtual-work rule for sub-voxel remainders.
Therefore even the smooth formula above is not yet an exact theorem about the
production movement phase.

## 3. Test blind spot

`compute_particle_el_residual()` labels its calculation independent but copies
the same negative `-alpha` branch used in `phase_forces.cpp`. Consequently
`test_action_stationarity` passes even when production contradicts
`coupling_term()` and `coupling_force()`.

`test_variational_coulomb` separately confirms that `coupling_force()` has the
positive sign, but it never compares that helper to the production branch.
Both tests passed on 2026-07-24. Their simultaneous success demonstrates missing
cross-contract coverage; it is not evidence that the action and tick agree.
The focused set including FTD-0441 passed `3/3`, and the unchanged golden
merge-gate battery passed `7/7`.

## 4. Correct current statement

The production legacy force is the selected rule

$$
F_{legacy}=-\alpha s\,\nabla(\nabla\cdot J).
$$

It passed the FTD-0439 net-pair reciprocity gate, but it is **not derived from
the currently declared discrete action**. The broader claim in `lagrangian.h`
that the complete tick cycle is the Euler–Lagrange evolution of that action is
false for this particle-force branch as written.

The action-consistent smooth candidate is

$$
F_{candidate}=+G_Cs\,\nabla(\nabla\cdot J),
$$

pending an exact finite-site virtual-work definition. This is a candidate to be
derived and tested, not a production fix licensed by this audit.

## 5. Consequence

FTD-0439's finite-protocol balance result for the legacy branch remains a
measurement of the code, but no positive variational interpretation survives.
The branch cannot be advanced as native electromagnetism until sign,
normalization, and discrete positional variation are reconciled in one source
of truth and checked by a cross-contract test.

No production dynamics were changed in this audit.

## 6. Successor result

FTD-0443 closes the exact algebra. All 52 Moore-hop action differences match
`G_C q[(div J)_b-(div J)_a]` to `1.73e-18`. Production is exactly antiparallel
to the symmetric action candidate and smaller by `alpha/G_C=G_C`. Remainder-
only motion leaves the coupling action unchanged. The legacy variational claim
is therefore closed negative, while an exact event-native hop-work route is now
available.
