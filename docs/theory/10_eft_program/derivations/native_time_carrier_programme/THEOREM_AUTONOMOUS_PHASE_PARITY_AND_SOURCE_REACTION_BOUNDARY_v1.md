# Theorem — Autonomous phase parity and source-reaction boundary v1

**Identifier:** `FTD-0887` / repaired execution `FTD-0888`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — POSITIVE AUTONOMOUS PHASE-WINDOW PARITY CONTROLLER]` +
`[THEOREM — MINIMUM CANONICAL SOURCE-REACTION PAIR IN THE REGISTERED CLASS]` +
`[THEOREM — EXACT HISTORY/REACTION ENERGY SPLITTER]` +
`[CLOSED NEGATIVE — NONZERO POSITIVE RECOIL AT THE HISTORY-SATURATED FTD-0886 ENDPOINT]` +
`[SELECTION — EQUAL SPLIT GIVEN OUTPUT-CHANNEL EXCHANGE SYMMETRY]` +
`[IMPOSED — PHASE WINDOWS, ORIGIN, CLOCK SCALE, AND REACTION ROLE]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]` +
`[OPEN — NATIVE SPATIAL SOURCE IDENTIFICATION/MOTION, PRODUCTION, G*]`

## 1. Verdict

The checkerboard alternation no longer needs an external integer-parity switch
at reference level. One periodic phase coordinate can compile the complete
ordered color-0/color-1 sequence under a single autonomous Hamiltonian. The
price is explicit: the phase origin, six window shapes, clock frequency, and
generator order remain imposed reference structure. This is an autonomous
Hamiltonian controller, not a native derivation of clock hardware.

The source-reaction question has a similarly sharp answer. The FTD-0886
history-only gate sends all positive residual energy to the outgoing history
pair. That endpoint is energy-saturated and cannot also give a zero-initialized
positive source mode nonzero recoil. One additional canonical pair is minimum
and sufficient in the registered local symplectic class. Reducing the history
amplitude then pays an exact source-reaction impulse.

The equal history/reaction split is unique only after imposing exchange
symmetry between those two output channels. It is a selection, not a
consequence of P1--P5. The reaction pair is not yet identified with spatial
momentum of native ternary matter.

## 2. Common canonical phase space

Let `(Q,P)` be the complete source-centered matched-face field pair and
`v_x=d_x/sqrt(6)` a normalized active-cell incidence row. Define

\[
u_x=v_x\cdot Q,\qquad \pi_{u,x}=v_x\cdot P.                 \tag{1}
\]

Give every cell a history pair `(a_x,pi_{a,x})` and reaction pair
`(r_x,pi_{r,x})`. With

\[
N=\frac12\left(\lVert Q\rVert^2+\lVert P\rVert^2
 +\sum_x(a_x^2+\pi_{a,x}^2+r_x^2+\pi_{r,x}^2)\right),       \tag{2}
\]

define for checkerboard color `m`

\[
L_{ua}^{(m)}=\sum_{x\in C_m}
 (a_x\pi_{u,x}-u_x\pi_{a,x}),                              \tag{3}
\]

\[
L_{ar}^{(m)}=\sum_{x\in C_m}
 (a_x\pi_{r,x}-r_x\pi_{a,x}),\qquad
N_r^{(m)}=\frac12\sum_{x\in C_m}(r_x^2+\pi_{r,x}^2).       \tag{4}
\]

Same-color incidence rows are orthonormal. Consequently their local
generators commute, every generator commutes with `N`, and

\[
|L_{ua}^{(m)}|\le N,\qquad |L_{ar}^{(m)}|\le N,qquad
0\le N_r^{(m)}\le N.                                      \tag{5}
\]

## 3. Autonomous parity compiler

Partition one phase circle into six intervals

\[
W_j=[j\pi/3,(j+1)\pi/3]
\]

and define periodic `C^1` windows

\[
\rho_j(\theta)=
\begin{cases}
\sin^2(3\theta-j\pi),&\theta\in W_j,\\
0,&\text{otherwise}.
\end{cases}                                                 \tag{6}
\]

Each window and its first derivative vanish at both endpoints, distinct
interiors are disjoint, and `int rho_j dtheta=pi/6`. Order the generators as

\[
(G_0,\ldots,G_5)=
(L_{ua}^{(0)},L_{ar}^{(0)},N_r^{(0)},
 L_{ua}^{(1)},L_{ar}^{(1)},N_r^{(1)})                       \tag{7}
\]

with target angles

\[
(\alpha_0,\ldots,\alpha_5)=
(\pi/2,\eta,\pi/2,\pi/2,\eta,\pi/2),qquad
0\le\eta\le\pi/2.                                        \tag{8}
\]

For `kappa_j=6 alpha_j/pi`, the autonomous Hamiltonian is

\[
H=\Omega I+6\Omega N
 +\Omega\sum_{j=0}^5\kappa_j\rho_j(\theta)G_j.             \tag{9}
\]

It contains no external time or integer tick. Since `theta_dot=Omega`, each
window lasts `pi/(3 Omega)`. The base `6 Omega N` flow makes one `2*pi`
identity winding per window, while the active pulse integrates exactly to its
target angle. Different-color generators need not commute because their
supports are temporally disjoint.

Only one pulse is active and `0<=kappa_j rho_j<=3`, so

\[
H-\Omega I\ge3\Omega N\ge0.                               \tag{10}
\]

During a window, `G_j` is conserved and

\[
I(\theta)=I_{j,0}-\kappa_j\rho_j(\theta)G_j.                \tag{11}
\]

Clock action therefore returns at every boundary, has excursion at most
`3N`, and is positive under the sufficient reserve `I_0>3N`. Reversing the
Hamiltonian trajectory reverses the entire six-pulse sequence exactly.

## 4. Minimum reaction splitter

For one active cell, apply in order:

1. the FTD-0886 residual/history quarter-turn;
2. a history/reaction rotation by `eta`; and
3. a reaction-mode quarter-turn.

Writing `c=cos(eta)` and `s_eta=sin(eta)`, the exact endpoint is

\[
\begin{aligned}
u'&=a,& \pi_u'&=\pi_a,\\
a'&=-c u-s_\eta r,& \pi_a'&=-c\pi_u-s_\eta\pi_r,\\
r'&=-s_\eta\pi_u+c\pi_r,&
\pi_r'&=s_\eta u-c r.
\end{aligned}                                                \tag{12}
\]

This matrix is orthogonal, symplectic, orientation preserving, norm
preserving, and exactly invertible. On the ready slice

\[
a=\pi_a=r=\pi_r=\pi_u=0,
\]

it gives

\[
u'=\pi_u'=r'=\pi_a'=0,qquad
a'=-\cos\eta\,u,qquad
\pi_r'=\sin\eta\,u.                                       \tag{13}
\]

Thus the Gauss residual clears exactly and the reaction coordinate returns to
its equilibrium position while its conjugate receives a persistent impulse.
The energy partition is

\[
E_{\rm hist}'=\cos^2\eta\,E_{\rm res},\qquad
E_{\rm react}'=\sin^2\eta\,E_{\rm res},\qquad
E_{\rm res}=u^2/2.                                         \tag{14}
\]

At `eta=0`, equation (13) is the FTD-0886 history-only endpoint. Since its
history energy already equals `E_res`, a zero-initialized nonnegative reaction
mode must remain at zero if that endpoint is held fixed. Nonzero recoil must
reduce the outgoing history share or consume prior energy.

A one-dimensional real local fiber has only the zero skew form. Therefore one
new scalar cannot carry reversible recoil; one canonical pair is minimum and
sufficient in the registered onsite-direct-sum class. This is another
instance of the existing selected canonical-pair type, not a sixth v2 type.

## 5. Self-dual selection and energy ledger

Demanding output-channel exchange symmetry means

\[
E_{\rm hist}'=E_{\rm react}'.                               \tag{15}
\]

On `0<=eta<=pi/2`, equation (15) has the unique solution

\[
\eta=\pi/4,
\]

giving one half of the residual energy to each channel. The uniqueness is
conditional on the imposed symmetry. The symmetry itself is not derived.

For fixed equilibrium source offset `s_0`, write `y=s_0+u` and

\[
E_{\rm raw}=\frac12(y^2+a^2),\qquad
U_{\rm int}=-s_0y+\frac12s_0^2.                            \tag{16}
\]

On the ready slice the old fresh-port work is `w=-s_0u`, and

\[
\Delta E_{\rm raw}=w-E_{\rm react}',\qquad
\Delta U_{\rm int}=-w.                                    \tag{17}
\]

Therefore

\[
\boxed{\Delta(E_{\rm raw}+U_{\rm int}+E_{\rm react})=0}.  \tag{18}
\]

The reaction impulse is paid exactly by reducing the history energy. It is not
free controller work.

## 6. Boundary

### Closed at reference level

- one autonomous positive Hamiltonian compiles both checkerboard colors;
- no external integer-parity switch is mathematically required;
- one canonical reaction pair is minimum and sufficient in the registered
  energy-splitting class;
- the FTD-0886 history-only endpoint is positive-energy saturated;
- the reaction impulse and complete inverse ledger are exact; and
- equal splitting is unique conditional on channel-exchange symmetry.

### Still open

- native formation and maintenance of the common phase controller;
- derivation of its phase origin, six-window law, frequency, and scale;
- physical identification of `(r,pi_r)` with spatial source displacement and
  momentum;
- ternary-source formation, mass/inertia, intercell motion, and recoil;
- a physical open complete-pair history rail, routing, boundaries, and
  congestion;
- production migration and the full constrained energy ledger;
- synchronization with the distinct quartic-`G*` calendar;
- Born recovery, Bell laboratory recovery, operational Lorentz hiding; and
- whole-framework completeness.

The advance is therefore real but conditional: external parity scheduling is
removed, and an exact positive reaction channel exists. Native matter recoil
is not yet derived.

## 7. Verification and provenance

The frozen FTD-0887 protocol SHA-256 is
`484EC4ED25C322D93B44F88267259B81AE510AE659AE22C4366A5DE69635146A`.
The frozen parent certificate SHA-256 is
`814B0AA2E8A555C9F48D9BCAD27C970B07862D1868888A6E0B8C321FEBA97399`.
Its first locked execution returned `68/72`: every substantive gate passed;
C8 and C27 used raw line-wrapped prose, C65 compared an unsimplified symbolic
identity, and C72 failed dependently. No theorem is booked from that parent
run.

FTD-0888 froze only those three representation normalizations. Its repair
protocol SHA-256 is
`F2AA1B0239B4BAC4EBBB48DB4976097185EC006CC4AF13B8A8A9602533E61CC1`;
the in-memory wrapper SHA-256 is
`4C19F1A8197ED7C2198B59E56F288A707C3BC784CA4DE586B99A601C762AFC17`.
The inherited certificate passes `72/72` with markers:

```text
AUTONOMOUS_PHASE_PARITY_CONTROLLER=POSITIVE_EXACT_REFERENCE
EXTERNAL_INTEGER_PARITY_SWITCH=NOT_REQUIRED_AT_REFERENCE_LEVEL
SOURCE_REACTION_CHANNEL=ONE_CANONICAL_PAIR_MINIMUM_IN_REGISTERED_CLASS
HISTORY_ONLY_ENDPOINT=POSITIVE_ENERGY_SATURATED
REACTION_IMPULSE=PAID_BY_REDUCED_HISTORY_ENERGY
SELF_DUAL_HISTORY_REACTION_SPLIT=SELECTED_CHANNEL_SYMMETRY
SPATIAL_TERNARY_SOURCE_RECOIL=OPEN
FINITE_CYCLIC_FRESHNESS_BOUNDARY=UNCHANGED
PRODUCTION_COUPLING=NONE
GSTAR_ROLE=SEPARATE_CALENDAR
BORN_BELL_STATUS=UNTOUCHED
```

The isolated implementation is:

- `engine/include/ftd/eft/autonomous_phase_parity_source_reaction.h`;
- `engine/src/eft/autonomous_phase_parity_source_reaction.cpp`; and
- `engine/tests/test_autonomous_phase_parity_source_reaction.cpp`.

No production `Voxel`, field, toggle, default, boundary mode, or tick phase is
changed.
