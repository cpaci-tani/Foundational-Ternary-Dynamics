# Preregistration — Autonomous phase parity and source-reaction splitter v1

**Identifier:** `FTD-0887`  
**Date frozen:** 2026-08-11  
**Status:** `[LOCKED/PRE-RUN]`  
**Programme:** native time carrier / contextual actualization  
**Method guard:** exact symbolic algebra, fixed rational witnesses, and
exhaustive finite-state controls only; no numerical search, fitting, near-miss
scan, or formula substitution is permitted.

## 1. Question

FTD-0886 leaves two coupled reference debts:

1. the checkerboard color is still selected by an external integer-parity
   schedule; and
2. the fixed-source gate exports the entire residual energy into history, so
   no positive source-reaction channel can receive a persistent impulse.

This lock asks for the smallest exact canonical refinement that removes the
external parity switch and makes room for a source reaction without changing
the Gauss-clearing endpoint. It tests:

- whether one autonomous, time-independent Hamiltonian on an extended phase
  space can compile the ordered color-0/color-1 layers;
- whether the FTD-0886 history-only endpoint is energy-saturated;
- whether one additional instance of the already selected canonical-pair type
  is minimum and sufficient for a positive reaction channel; and
- what, if anything, selects an equal history/reaction energy split.

The certificate may close an autonomous **reference phase controller** and a
canonical **source-reaction impulse**. It may not identify that reaction mode
with native spatial matter momentum, derive source formation or transport,
claim a production Hamiltonian, derive the clock scale or phase origin, couple
to `G*`, recover Born/Bell/Lorentz physics, or claim framework completeness.

## 2. Frozen sources

| source | SHA-256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_ALTERNATING_ORIENTED_TERNARY_PARITY_RAIL_AND_ONE_SHOT_BOUNDARY_v1.md` | `E70F2AD61BFA1C8BBFF4EA03DCF0312B8F96224ECF2453FDF4B81B0FEA845CA1` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_LOCAL_CANONICAL_HAMILTONIAN_PARITY_RAIL_AND_SCALAR_LOCALITY_BOUNDARY_v1.md` | `982C3B9D00798920A1BDAB96C75EBC9DB3A08111E8900F1D630382B0249B25F6` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_REVERSIBLE_CHECKERBOARD_GAUSS_RECORD_PREPARATION_AND_SELF_DUAL_ENERGY_SPLIT_v1.md` | `143D897A69B5C6FED8C00402C1840EA9FAEE5BD4BC259C9BDD065DFDC616A814` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_BOUNDARY_v1.md` | `0D5A093597CE7BFFF7F593C0A1AF2B65E6CDE99DB0FFEDA1183D9849BC58624F` |
| `engine/include/ftd/eft/local_canonical_hamiltonian_parity_rail.h` | `28A76212958450A836CD8D522BDCC7C3C19D848E1ECDDCBCA3D235AF84B3AED5` |
| `engine/include/ftd/eft/canonical_source_centered_gauss_gate.h` | `C65E562B4B3855076748B1A73EF742DD20D106191120D2631864C6D16FFE8C2D` |

## 3. Frozen common phase space

Let `(Q,P)` be the full source-centered matched-face canonical field pair. For
cell `x`, let `v_x=d_x/sqrt(6)` and define

\[
u_x=v_x\mathbin{\cdot}Q,\qquad
\pi_{u,x}=v_x\mathbin{\cdot}P.                              \tag{P1}
\]

Rows belonging to one checkerboard color are orthonormal. Give every cell a
history pair `(a_x,pi_{a,x})` and a source-reaction pair `(r_x,pi_{r,x})`.
The latter is another instance of the already selected local canonical-pair
type, not a newly claimed substrate degree of freedom. Define the common norm

\[
N=\frac12\left(\lVert Q\rVert^2+\lVert P\rVert^2
 +\sum_x(a_x^2+\pi_{a,x}^2+r_x^2+\pi_{r,x}^2)\right).       \tag{P2}
\]

For color `m in {0,1}`, freeze

\[
L_{ua}^{(m)}=\sum_{x\in C_m}
 (a_x\pi_{u,x}-u_x\pi_{a,x}),                               \tag{P3}
\]

\[
L_{ar}^{(m)}=\sum_{x\in C_m}
 (a_x\pi_{r,x}-r_x\pi_{a,x}),\qquad
N_r^{(m)}=\frac12\sum_{x\in C_m}(r_x^2+\pi_{r,x}^2).       \tag{P4}
\]

The frozen bounds are

\[
\{N,G\}=0,\qquad |L_{ua}^{(m)}|\le N,\qquad
|L_{ar}^{(m)}|\le N,\qquad 0\le N_r^{(m)}\le N.             \tag{P5}
\]

## 4. Frozen autonomous six-window controller

Let `(theta,I)` be one common phase pair, `Omega>0`, and partition its circle
into six intervals

\[
W_j=[j\pi/3,(j+1)\pi/3],\qquad j=0,\ldots,5.                \tag{A1}
\]

Define the periodic `C^1` windows

\[
\rho_j(\theta)=
\begin{cases}
\sin^2(3\theta-j\pi),&\theta\in W_j,\\
0,&\text{otherwise},
\end{cases}                                                  \tag{A2}
\]

with endpoints identified. Their supports do not overlap and

\[
\int_{W_j}\rho_j(\theta)\,d\theta=\frac\pi6.               \tag{A3}
\]

For a frozen split angle `0<=eta<=pi/2`, order the generators and target
angles as

\[
(G_0,\ldots,G_5)=
(L_{ua}^{(0)},L_{ar}^{(0)},N_r^{(0)},
 L_{ua}^{(1)},L_{ar}^{(1)},N_r^{(1)}),                       \tag{A4}
\]

\[
(\alpha_0,\ldots,\alpha_5)=
(\pi/2,\eta,\pi/2,\pi/2,\eta,\pi/2),\qquad
\kappa_j=\frac{6\alpha_j}{\pi}.                            \tag{A5}
\]

Freeze the autonomous Hamiltonian

\[
H=\Omega I+6\Omega N
 +\Omega\sum_{j=0}^5\kappa_j\rho_j(\theta)G_j.             \tag{A6}
\]

This is autonomous because it has no external time or tick argument;
`theta_dot=Omega` carries the system through the frozen phase order. On every
window the base `6*Omega*N` flow completes one `2*pi` identity winding, while
the active generator integrates to `alpha_j`. Equation (P5) and
`0<=kappa_j*rho_j<=3` give

\[
H-\Omega I\ge3\Omega N\ge0.                                \tag{A7}
\]

Within window `j`, `G_j` is conserved and

\[
I(\theta)=I_{j,0}-\kappa_j\rho_j(\theta)G_j.                \tag{A8}
\]

Thus clock action returns at all six boundaries and its excursion is at most
`3*N`; `I_0>3*N` is a sufficient cycle-wide reserve. No commutation of
different-color generators is assumed: disjoint phase support supplies their
exact ordered composition.

## 5. Frozen local reaction splitter

For one active cell, the first pulse is the FTD-0886 residual/history
quarter-turn. The second rotates history into the reaction mode by `eta`; the
third rotates the reaction mode by a quarter cycle. Writing
`c=cos(eta)` and `s_eta=sin(eta)`, the resulting endpoint is

\[
\begin{aligned}
u'&=a,&\quad \pi_u'&=\pi_a,\\
a'&=-c\,u-s_\eta r,& \pi_a'&=-c\,\pi_u-s_\eta\pi_r,\\
r'&=-s_\eta\pi_u+c\,\pi_r,&
\pi_r'&=s_\eta u-c\,r.
\end{aligned}                                                \tag{R1}
\]

Equation (R1) is the product of three orthogonal symplectic rotations, has
determinant `+1`, preserves `N`, and is inverted by the reverse pulse order
with opposite angles.

On the ready reaction slice

\[
a=\pi_a=r=\pi_r=\pi_u=0,                                   \tag{R2}
\]

the endpoint is

\[
u'=\pi_u'=r'=\pi_a'=0,\qquad
a'=-\cos\eta\,u,qquad
\pi_r'=\sin\eta\,u.                                       \tag{R3}
\]

The Gauss residual is therefore cleared exactly, while

\[
E_{\rm hist}'=\cos^2\eta\,E_{\rm res},\qquad
E_{\rm react}'=\sin^2\eta\,E_{\rm res},\qquad
E_{\rm res}=\frac12u^2.                                   \tag{R4}
\]

The FTD-0886 gate is the `eta=0` history-only endpoint. It saturates the
positive residual energy: if `u'=0` and `E_hist'=E_res`, every additional
nonnegative zero-initialized reaction energy must remain zero. Nonzero
reaction therefore requires reducing the outgoing history amplitude or adding
pre-existing energy. A nondegenerate local symplectic reaction channel needs
at least two real coordinates, so one canonical pair is minimum in the
registered onsite-direct-sum class.

The exchange-symmetric condition `E_hist'=E_react'` uniquely fixes
`eta=pi/4` on `[0,pi/2]`. This is a **[SELECTION — imposed output-channel
exchange symmetry]**, not a theorem that P1--P5 force equal splitting.

## 6. Frozen source/inter-action ledger

On (R2), write `y=s_0+u`, retain the fixed equilibrium source offset `s_0`,
and define

\[
E_{\rm raw}=\frac12(y^2+a^2),\qquad
U_{\rm int}=-s_0y+\frac12s_0^2.                             \tag{E1}
\]

With the old fresh-port work `w=-s_0u`, the splitter obeys

\[
\Delta E_{\rm raw}=w-E_{\rm react}',\qquad
\Delta U_{\rm int}=-w,                                     \tag{E2}
\]

and hence

\[
\Delta(E_{\rm raw}+U_{\rm int}+E_{\rm react})=0.           \tag{E3}
\]

The reaction impulse is therefore paid by reducing the outgoing history
energy. It is not free controller work. The equilibrium charge `s_0` remains
fixed; `(r,pi_r)` is only a canonical source-reaction carrier. Identifying it
with spatial matter momentum, moving a ternary source between cells, and
deriving its mass/inertia remain open.

## 7. Frozen certificate gates

The certificate contains exactly **72** checks.

### C1--C10 — provenance and scope

1. all six frozen source hashes match;
2. this protocol hash matches its recorded pre-run value;
3. the common field/history/reaction phase space is frozen;
4. the six phase windows and their order are frozen;
5. the source/inter-action ledger is frozen;
6. the split angle is bounded to `[0,pi/2]`;
7. the equal split is explicitly selected rather than derived;
8. the equilibrium source offset remains fixed;
9. production and `G*` are outside the result; and
10. Born, Bell, Lorentz hiding, biology, and completeness are outside the
    result.

### C11--C32 — autonomous phase compiler

11. every `rho_j` vanishes with zero first derivative at its endpoints;
12. the periodic windows are `C^1`;
13. distinct window interiors are disjoint;
14. every window integral is `pi/6`;
15. `theta_dot=Omega` is uniform;
16. every window lasts `pi/(3*Omega)`;
17. the base `N` angle is `2*pi` per window;
18. `kappa` is `3` for every quarter-turn pulse;
19. reaction-split `kappa` is `6*eta/pi` and at most `3`;
20. every active pulse integrates to its frozen target angle;
21. `N` commutes with every frozen generator;
22. the two angular generators satisfy the frozen `N` bounds;
23. the reaction norm satisfies `0<=N_r<=N`;
24. the carrier Hamiltonian has lower bound `3*Omega*N`;
25. the Hamiltonian contains no external tick or time argument;
26. phase order is color 0 then color 1;
27. no cross-color commutation is used;
28. the exact endpoint is the ordered six-pulse product;
29. clock action returns at every window boundary;
30. maximum action excursion is at most `3*N`;
31. `I_0>3*N` is a sufficient positive reserve; and
32. reversing the Hamiltonian trajectory gives the exact inverse.

### C33--C56 — local reaction channel

33. the residual/history pulse sends `(u,a)` to `(a,-u)`;
34. it applies the same rotation to the conjugates;
35. the history/reaction pulse has angle `eta`;
36. the reaction phase pulse has angle `pi/2`;
37. their product gives equation (R1);
38. the endpoint matrix is symplectic;
39. the endpoint matrix is orthogonal;
40. its determinant is `+1`;
41. the reverse pulse product is the exact inverse;
42. the full quadratic norm is preserved;
43. the ready slice gives `u'=0`;
44. it also gives `pi_u'=0`;
45. the reaction displacement returns zero on that slice;
46. the reaction momentum is `sin(eta)*u`;
47. the outgoing history coordinate is `-cos(eta)*u`;
48. history energy is `cos(eta)^2*E_res`;
49. reaction energy is `sin(eta)^2*E_res`;
50. history plus reaction energy equals residual energy;
51. `eta=0` reproduces the FTD-0886 endpoint;
52. a nonzero split gives nonzero reaction for nonzero residual;
53. the history-only endpoint leaves no positive recoil energy;
54. nonzero recoil therefore changes the history amplitude or consumes prior
    energy;
55. one real scalar cannot be a nondegenerate local symplectic source mode;
    and
56. one additional canonical pair is minimum and sufficient in the registered
    class.

### C57--C72 — symmetry, energy, and interpretation firewall

57. history/reaction exchange symmetry gives `cos(eta)^2=sin(eta)^2`;
58. its unique solution on `[0,pi/2]` is `eta=pi/4`;
59. the equal split gives one half of residual energy to each channel;
60. the equal split remains a selection;
61. raw energy changes by `w-E_react`;
62. interaction energy changes by `-w`;
63. raw plus interaction plus reaction energy is exact;
64. the reaction impulse is not free energy;
65. the complete reaction pair is retained for inversion;
66. finite cyclic history capacity is unchanged;
67. the construction reuses the existing canonical-pair type;
68. no sixth selected v2 type is added;
69. spatial ternary source motion and native inertia remain open;
70. production and quartic-`G*` synchronization remain separate;
71. Born, Bell, Lorentz hiding, and completeness remain untouched; and
72. the terminal gate executes only if C1--C71 pass.

## 8. Frozen outcomes

- **Outcome A — autonomous compiler plus minimal reaction channel:** all
  `72/72` pass. Book the autonomous six-window Hamiltonian controller, the
  positive minimal source-reaction splitter, the energy-saturation boundary of
  the history-only gate, and the conditional equal split under imposed channel
  exchange symmetry. Keep native source identification/motion, production,
  `G*`, Born/Bell/Lorentz recovery, and completeness open.
- **Outcome B — partial:** provenance passes but one or more algebraic gates
  fail. Book only independently passing exact statements; do not claim an
  autonomous controller or source-reaction completion that failed its gate.
- **Outcome C — execution invalid:** any source/protocol hash or terminal-count
  gate fails. Book no theorem from this run.

## 9. Frozen terminal markers

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
