# FTD-0997 — Preregistration: common/relative catalytic clock-growth transducer v1

**Identifier:** `FTD-0997`  
**Date locked:** 2026-08-12  
**Status before execution:** `[PREREGISTERED — NOT YET EVIDENCE]`  
**Expected classifier:** **Outcome B — existing-pair catalytic transducer / native compliance open**

## 1. Question

FTD-0995/0996 proves the exact crossing compliance
`2mU_y=p_x^2` but does not explain how a new site receives the complete
phase-bearing state without free canonical cloning. The FTD-0990 dual split
already contains a protected common clock pair and an open relative pair.
This discriminator asks whether the relative pair can serve as the missing
local catalytic port.

Specifically:

1. can a complete relative pair swap its state into a blank common receiver
   through an exact local symplectic map;
2. can the FTD-0994 formation-work shear refill the emptied relative pair;
3. is the composition energy exact and invertible for arbitrary positive
   formation work;
4. is catalytic recursion equivalent to the FTD-0996 compliance condition;
5. does the unchanged FTD-0990 separable membrane Hamiltonian force that
   condition or a phase-matched relative-port preparation; and
6. what happens on the natural quiescent kinetic-crossing seam?

No engine or production mutation is authorized.

## 2. Frozen sources

| source | SHA-256 |
|---|---|
| `THEOREM_CROSSING_MATCHED_FORMATION_ENERGY_AND_CAUSAL_QUARTIC_CLOCK_GROWTH_v1.md` | `68087ED4B410AF54571D61E6F8C7ABEFA694E29E0889ADC2286CC45BFEB70C0F` |
| `THEOREM_TERNARY_OCCUPANCY_MEMBRANE_AND_SELF_DUAL_BODY_CLOCK_SPLIT_v1.md` | `A19593DACD2CE97A6B785F235AE5048EADC228680E07D2F90F4C4DB7BD15333C` |
| `THEOREM_ZERO_ACTION_CANONICAL_SEED_AND_CAUSAL_CLOCK_GROWTH_BOUNDARY_v1.md` | `897367658B339F074A78FEA017994EEA63AD7921BA4C597663EA123088E76306` |
| `THEOREM_CATALYTIC_PHASE_REFERENCE_TRANSDUCER_v1.md` | `8BD6BB16999E91A72CADBA991A215F56A3E3E13816073E39B36F9EB51FD5FE33` |
| `THEOREM_GLOBAL_AGGREGATE_WORK_AND_LOCAL_CONCURRENCY_OWNERSHIP_BOUNDARY_v1.md` | `1CF020D3AA4EB78746C8CF7B932B3AB27E265E173E7F81524CF2A4547A38FA91` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/transmutation_phases.cpp` | `4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043` |

Any mismatch invalidates execution. A repair must preserve this protocol and
the first certificate byte-for-byte.

## 3. Registered local state and transaction

Use a mass-normalized chart first. Let the occupied donor common clock, its
local relative port, and the prospective receiver common clock be complete
canonical pairs

\[
 C=(q_C,p_C),\qquad R=(q_R,p_R),\qquad Y=(q_Y,p_Y).
                                                               \tag{1}
\]

On the registered kinetic crossing, prepare

\[
 C=R=z=(0,\sigma\sqrt{2e}),
 \qquad Y=0,
 \qquad e>0,quad\sigma\in\{-1,+1\}.             \tag{2}
\]

The equality `C=R` is a phase-complete **preparation condition**. It must not
be called a consequence of the FTD-0990 diagonal common/relative Hamiltonian.

Apply two local stages:

1. **complete-pair swap**

\[
 (R,Y)\longmapsto(Y,R);                               \tag{3}
\]

2. **formation refill** of the now-zero relative pair using positive released
   work `U` and retained orientation,

\[
 R=0\longmapsto z_U=(0,\sigma\sqrt{2U}).               \tag{4}
\]

The registered composite output is therefore

\[
 \boxed{C'=z,\qquad Y'=z,\qquad R'=z_U.}               \tag{5}
\]

For kinetic mass `m`, replace `sqrt(2e)` and `sqrt(2U)` by
`sqrt(2me)` and `sqrt(2mU)`.

## 4. Exact energy, inverse, and catalytic condition

Let the membrane/source lose `U` during refill. Complete-pair swap preserves
the oscillator sum. The final energy ledger must be

\[
 \begin{array}{c|ccc|c}
 &H_C&H_R&H_Y&H_{\rm source}\\ \hline
 \text{before}&e&e&0&E_s\\
 \text{after}&e&U&e&E_s-U
 \end{array}                                           \tag{6}
\]

and hence conserve total energy for every `U>0`.

The exact inverse must act in reverse order: first apply the inverse refill
`z_U -> 0`, restoring `U` to the source; then swap `Y` back into `R`. It must
recover equation (2) exactly when `sigma`, `U`, occupancy history, and the
source variables required by the shear are retained.

The relative port is catalytic exactly when

\[
 R'=R
 \quad\Longleftrightarrow\quad
 z_U=z
 \quad\Longleftrightarrow\quad
 \boxed{U=e}.                                         \tag{7}
\]

For mass `m`, equation (7) is exactly `2mU-p_C^2=0`, the FTD-0996 compliance
surface. On mismatch, the port is not erased: its energy changes by

\[
 \boxed{\Delta H_R=U-e.}                              \tag{8}
\]

It therefore retains the energetic failure of catalytic closure.

## 5. Symplectic and no-cloning tests

The complete-pair swap in equation (3) must satisfy

\[
 S^T\Omega_4S=\Omega_4,
 \qquad\det S=1,
 \qquad S^{-1}=S.                                     \tag{9}
\]

The refill is the FTD-0994 coordinate-gradient shear on the relative pair and
source variables. Its composition with (3) is symplectic and inverted by the
reverse-order inverse on the registered smooth positive-work branch.

Equation (5) must not be called unrestricted cloning. On the full phase
space, `R'=z_U` is generally not `z`; equality occurs only on the correlated
codimension-one compliance surface, and the source/membrane state changes by
`-U`. The complete source-plus-port map retains an inverse.

## 6. Existing-pair capacity and native forcing test

FTD-0990 supplies complete common and relative pairs through

\[
 q_\pm={q_L\pm q_R\over\sqrt2},
 \qquad
 p_\pm={p_L\pm p_R\over\sqrt2}.                       \tag{10}
\]

Thus equations (1)--(5) add no continuous pair type. They select ownership
and a swap/refill use of the existing relative pair.

But the fixed FTD-0990 Hamiltonian is block diagonal,

\[
 H_0={1\over2}p_+^2+V_m(q_+)
    +{1\over2}p_-^2+V(q_-),                            \tag{11}
\]

so it contains no term forcing `C=R`. Common and relative initial data are
independent.

Likewise, the occupancy-transition work is a configuration function
`U=U(q,m)` at fixed field coordinates, while crossing clock energy is
`e=p_C^2/(2m)`. Define

\[
 F(q,p_C,m)=2mU(q,m)-p_C^2.                            \tag{12}
\]

At every nonzero kinetic crossing,

\[
 {\partial F\over\partial p_C}=-2p_C\ne0.             \tag{13}
\]

Therefore the unchanged separable Hamiltonian cannot make `F=0` an identity
on an open local phase-space set. Its zero set is a regular codimension-one
admission surface. A coupling, constraint, selected preparation, or feedback
is required to land on it.

## 7. Quiescent-seam test

At the exact matching crossing take the prospective receiver and every
affected common-field endpoint to have zero coordinate, with no extra onsite
load. Every changed membrane bond then has zero strain. The FTD-0992 work law
must give

\[
 W_y=0,
 \qquad U=0.                                          \tag{14}
\]

For a positive donor clock `e>0`, equation (7) fails. The relative port can
perform the swap once, but the quiescent membrane cannot refill it. Hence the
static matter membrane is not by itself a recursively powered clock-growth
engine.

Positive refill can come only from pre-existing void/boundary strain, an
onsite latent term, the relative/environmental channel, or another local
reserve. None is forced to equal `e` by equation (11).

## 8. Exact gates

### G1 — source and production lock

- all nine frozen hashes match;
- sources contain common/relative pairs, the Cartesian seed, exact
  compliance law, catalytic reference precedent, and local ownership lower
  bound;
- production lacks the registered complete-pair swap/refill transaction,
  relative-port ownership, compliance gate, and inverse.

### G2 — complete-pair swap

Prove equation (9), preservation of both canonical brackets, zero cross
brackets, oscillator energy, and exact involution. Reject scalar-coordinate
swap as anti-symplectic/incomplete.

### G3 — composite transaction

Prove equations (5)--(8) for both orientations, arbitrary positive `e,U`,
mass-normalized and general-mass charts. Prove exact reverse-order recovery
and explicit mismatch retention.

### G4 — no-cloning scope

Prove that apparent donor/receiver equality occurs only on the constrained
prepared/compliant submanifold, that the source changes, and that the full map
has an inverse. Reject unrestricted phase-space cloning.

### G5 — existing-pair capacity and native non-forcing

Prove equation (10) is orthogonal/symplectic, equation (11) leaves common and
relative data independent, and equations (12)--(13) make compliance a regular
codimension-one surface rather than a Hamiltonian identity.

### G6 — quiescent seam

Prove equation (14) from the exact changed-bond work law. Show that positive
clock energy cannot be restored from zero strain and identify the allowed
but unforced positive-energy sources without promoting them.

### G7 — interpretation firewalls

Explicitly reject promotion to:

- a derived preparation `C=R`, swap controller, aperture scheduler, or
  relative-port ownership law;
- autonomous attraction to the compliance surface;
- a new continuous pair type or a production implementation;
- derived critical quarticity, scale, amplitude, `G*`, mass, or reserve;
- Born/Bell, probability, measurement, Lorentz hiding, biology,
  consciousness, or completeness.

No fit, numerical near-miss search, parameter scan, formula substitution, or
engine mutation is permitted.

## 9. Classifier

- **Outcome A — native catalytic growth:** all gates pass and frozen
  production already owns/prepares the relative port, performs the swap and
  refill, forces compliance, and retains the inverse.
- **Outcome B — existing-pair catalytic transducer / native compliance open:**
  G2--G6 pass, the existing relative pair supplies exact capacity, but its
  preparation/ownership, positive refill, compliance, or production remains
  selected/absent.
- **Outcome C — energy ledger only:** the bookkeeping closes but the complete
  pair map, symplecticity, inverse, or mismatch retention fails.
- **Outcome D — invalid:** a source hash or exact gate fails.

Outcome B is expected. Outcome A is forbidden unless the frozen production
sources contain the complete mechanism.
