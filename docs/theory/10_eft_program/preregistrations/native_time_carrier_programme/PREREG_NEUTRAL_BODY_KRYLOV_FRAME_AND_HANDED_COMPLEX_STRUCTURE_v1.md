# Pre-registration — Neutral-body Krylov frame and handed complex structure v1

**Identifier:** `FTD-0966`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Question

Can a finite neutral actual ternary body derive, from its own existing spatial
record alone,

1. a polar body axis;
2. a nonparallel polar transverse ray;
3. the spatial pseudoscalar needed to turn an axial cross product back into a
   polar momentum direction under the full signed-cubic group; and
4. a real `i`-like complex structure that is covariant under improper as well
   as proper cubic transformations?

The result must not read an external Cartesian axis, target orientation,
future event, `G*`, context, outcome, or Born weight. A successful snapshot
frame is not automatically a canonical moving-frame production law.

## 2. Frozen sources

| Source | Frozen SHA-256 |
|---|---|
| `THEOREM_NATIVE_TERNARY_DIPOLE_AXIS_AND_BILATERAL_PHASE_WEDGE_MEMORY_BOUNDARY_v1.md` | `8B07C26475A76E79C37B825B91EA174C0D1D8C13F06422483EE60B236DC14340` |
| `THEOREM_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_AND_SOURCE_FRAME_BOUNDARY_v1.md` | `BD5B9DB5C9543F76241E6525B0CCD44787D16FE933D24E742C3982F9E6898981` |
| `THEOREM_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_AND_CUBIC_CHART_BOUNDARY_v1.md` | `FF80023FA73326B439405C8A07F08A72A5EBD8CC845AC145224B5BE4D647F07C` |

No engine source, ontology type, production phase, selector, or constant may
change under this protocol.

## 3. Frozen regional construction

Let `S` be a finite connected support of nonzero ternary actual states
`s_x in {-1,+1}` inside one unambiguous minimum-image chart, with

\[
 N=|S|,\qquad \sum_{x\in S}s_x=0.                              \tag{1}
\]

For unwrapped lattice coordinates `r_x`, define the occupancy centroid,
neutral dipole, and centered occupancy covariance

\[
 X={1\over N}\sum_xr_x,
 \qquad d=\sum_xs_x(r_x-X)=\sum_xs_xr_x,                       \tag{2}
\]

\[
 C={1\over N}\sum_x(r_x-X)(r_x-X)^T.                          \tag{3}
\]

Form the exact moment-Krylov pseudoscalar

\[
 \kappa=\det[d,Cd,C^2d].                                      \tag{4}
\]

On the regular chart `kappa != 0`, define

\[
 \chi=\operatorname{sign}(\kappa),\qquad
 e_1={d\over|d|},                                             \tag{5}
\]

\[
 t=(I-e_1e_1^T)Cd,\qquad e_2={t\over|t|},\qquad
 e_3=\chi(e_1\times e_2).                                    \tag{6}
\]

The ordered frame is a **polar triad**: all three axes must transform as
ordinary spatial vectors even under reflections. Its orientation is retained
as

\[
 \det[e_1,e_2,e_3]=\chi.                                     \tag{7}
\]

Define the handed transverse complex structure

\[
 \mathcal I_Fv=\chi(e_1\times v).                             \tag{8}
\]

It must obey

\[
 \mathcal I_F^T=-\mathcal I_F,\qquad
 \mathcal I_F^2=-(I-e_1e_1^T).                               \tag{9}
\]

## 4. Frozen symmetry claims

For a translation `a` and a signed-cubic matrix `Q in O_h`, the transformed
record is `r'_x=Qr_x+a`, `s'_x=s_x`. The certificate must prove

\[
 d'=Qd,\qquad C'=QCQ^T,\qquad
 \kappa'=\det(Q)\kappa,\qquad \chi'=\det(Q)\chi,              \tag{10}
\]

\[
 e'_j=Qe_j\quad(j=1,2,3),\qquad
 \mathcal I'_F=Q\mathcal I_FQ^T.                             \tag{11}
\]

Thus `chi` pays exactly the improper-covariance price identified by FTD-0948.
It is a derived snapshot observable, not a new stored type.

Under actual sign reversal `s_x -> -s_x`, `C` is unchanged while
`d,Cd,C^2d`, `chi`, and all three polar axes reverse. No result will identify
actual sign reversal with time reversal.

## 5. Minimum-cardinality gate

For a nonempty neutral support with only `+/-1` states, `N` is even. The only
case below four is a two-site dipole. Its centered covariance has rank one and
`d,Cd,C^2d` are collinear, so `kappa=0`. Hence

\[
 N\ge4                                                        \tag{12}
\]

is necessary.

Freeze the exact one-cube witness

\[
\begin{array}{c|cccc}
x&(0,0,0)&(1,0,0)&(0,1,0)&(1,1,1)\\ \hline
s_x&+1&+1&-1&-1.
\end{array}                                                   \tag{13}
\]

For this body the certificate must recover

\[
 X=(1/2,1/2,1/4),\qquad d=(0,-2,-1),                          \tag{14}
\]

\[
 C=\begin{pmatrix}
 1/4&0&1/8\\
 0&1/4&1/8\\
 1/8&1/8&3/16
 \end{pmatrix},\qquad \kappa=-1/256,                          \tag{15}
\]

and

\[
 e_1={1\over\sqrt5}(0,-2,-1),\quad
 e_2={1\over3\sqrt5}(-5,2,-4),\quad
 e_3={1\over3}(-2,-1,2),\quad\chi=-1.                       \tag{16}
\]

This proves cardinality four is sufficient and Moore-local at snapshot level.

## 6. Canonical chart and temporal orientation boundary

On a fixed actual-record stratum, the orthogonal frame rotates each existing
polar field coordinate and its conjugate momentum by the same matrix. That
block-diagonal transformation is symplectic. It therefore repairs the
selected-frame debt in FTD-0965 conditionally on a formed regular body.

The spatial handedness `chi` is time-even. Clockwise/counterclockwise traversal
still requires the separate time-odd crossing sign `eta`. The two oriented
complex structures are

\[
 \mathcal I_{\pm}=\pm\mathcal I_F,\qquad
 \mathcal I_+^2=\mathcal I_-^2=-\Pi_{e_1}.                    \tag{17}
\]

Thus the symmetric square again loses temporal direction; the spatial frame
does not replace the oriented clock current.

## 7. Moving-frame and formation firewall

The map is undefined at `kappa=0`, and `sign(kappa)` is discontinuous across
that degeneracy. If the actual support changes, a naive instantaneous
reprojection is not thereby a canonical autonomous update. A moving frame
adds its own angular connection, reaction, switching work, and history.

The witness does not prove:

- autonomous formation or persistence of the four-site body;
- robustness away from the regular chart;
- a continuous global frame across `kappa=0`;
- that the body frame realizes the FTD-0963 connection profile or complete
  square;
- moving-frame reaction, energy/current closure, reserve, inverse, routing,
  or recycling;
- one-way phase-error export;
- critical-quartic `G*` synchronization;
- Born/Bell recovery, operational hiding, or completeness; or
- production integration.

## 8. Frozen checks

- **G1:** all source and protocol hashes plus scope markers;
- **G2:** translation invariance and origin independence from neutrality;
- **G3:** exact covariance under all 48 signed permutation matrices;
- **G4:** minimum-cardinality obstruction below four;
- **G5:** exact four-site witness, including `kappa=-1/256`;
- **G6:** orthonormal polar triad and pseudoscalar orientation;
- **G7:** handed complex-structure algebra and full improper covariance;
- **G8:** fixed-stratum symplectic field/momentum projection;
- **G9:** separate time-odd orientation and symmetric-square loss;
- **G10:** degeneracy, moving-frame, formation, production, `G*`, and Born
  firewalls.

No fitted tolerance, floating comparison, numerical search, near-miss scan,
or target-coded probability is permitted.

## 9. Frozen classifier

- **Outcome A — native global frame:** G1--G10 pass and the construction is a
  globally defined dynamically maintained canonical production frame.
- **Outcome B — exact conditional regional frame:** G1--G10 pass, equations
  (2)--(16) close exactly on `kappa!=0`, and formation/moving-frame production
  remain open.
- **Outcome C — obstruction:** the existing ternary geometry cannot supply
  the nonparallel ray or pseudoscalar even conditionally.
- **Outcome D — invalid:** any lock, exact identity, or scope gate fails.

The expected result is Outcome B. It licenses a minimum exact snapshot frame,
not a production connection.
