# FTD-0930 — Preregistration: eight-color source-centered positive-port relaxation and massless-halo boundary v1

**Identifier:** `FTD-0930`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** minimum complete-port positive canonical dilation of one C18
source-centered coordinate-relaxation layer; exact eight-color locality;
finite-grounded convergence for the gapped dynamic companion and massless
static equation; uncontained static infrared and fresh-port boundary; no
numerical search, fit, engine mutation, ontology adoption, target-profile,
`G*`, Born, Bell, context, outcome, or hiding read

## 1. Question

FTD-0929 gives a target-blind causal construction of the unique dynamic
companion, but its registered overwrite-history lift is hyperbolic and has no
positive conserved quadratic energy. FTD-0886 separately proves that one
source-centered residual and one complete fresh port admit a positive
canonical quarter-turn.

Can that quarter-turn be composed across the actual C18 operator so that it:

1. computes rather than preloads the dynamic companion;
2. has a positive source-centered energy and exact local Hamiltonian lift;
3. converges under a finite local color schedule;
4. also supplies an honest causal candidate for a massless static halo; and
5. states exactly which reservoir, clock, source, and uncontained-limit costs
   remain open?

The certificate must not infer autonomous formation from a stroboscopic
reference gate, indefinite operation from a finite port bank, or an
uncontained halo from a finite grounded solve.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `THEOREM_QUASILOCAL_COMPANION_PREPARATION_AND_REVERSIBLE_HISTORY_FORMATION_BOUNDARY_v1.md` | `4E00155889BAD84D3ED4A7B907BFBC86589DEA6873A24529519ADE310DC9CEFB` |
| `proof_quasilocal_companion_preparation_reversible_history_formation_boundary.py` | `AE6B5A068C9F1A0F0F81A73DB2EB037EF13F49F31845070B833602558B4AF0A7` |
| `THEOREM_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_BOUNDARY_v1.md` | `0D5A093597CE7BFFF7F593C0A1AF2B65E6CDE99DB0FFEDA1183D9849BC58624F` |
| `proof_canonical_source_centered_gauss_gate_v2.py` | `6C35135A3B5B9345E6EA9A6EBFB61B32951EE07DDDB17188362B8B38A10F1816` |
| `THEOREM_FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_AND_RECYCLING_BOUNDARY_v1.md` | `AF810B73322DE8521C8509792E09D549A10E1D8417C1B283A3630EB8B16D7BFC` |
| `proof_finite_port_rail_positive_source_battery_boundary_v2.py` | `E2129A5284AB5C664C5A257B0D861D2A5C4329776CC0E684365845B120379D87` |
| `THEOREM_LOCAL_CANONICAL_HAMILTONIAN_PARITY_RAIL_AND_SCALAR_LOCALITY_BOUNDARY_v1.md` | `982C3B9D00798920A1BDAB96C75EBC9DB3A08111E8900F1D630382B0249B25F6` |
| `proof_local_canonical_hamiltonian_parity_rail.py` | `B971DDA9A79AD53C340B00A4268EF9DA5BF089AF62DC37DE3D04757FAE03E326` |
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |

The certificate fails closed on source drift.

## 3. Frozen operators and finite-region class

Let `K` be the scalar C18 stiffness

\[
 (Kq)_x={4\over3}q_x
 -{1\over9}\sum_{y\sim_f x}q_y
 -{1\over18}\sum_{y\sim_e x}q_y,                         \tag{1}
\]

where the first sum contains the six face neighbors and the second the twelve
edge neighbors. Register

\[
 M_d=2I-K,\qquad d_d=(M_d)_{xx}={2\over3},               \tag{2}
\]

for the FTD-0929 dynamic companion and

\[
 M_s=K,\qquad d_s=(M_s)_{xx}={4\over3},                  \tag{3}
\]

for the static massless equation.

All finite-matrix assertions use a finite connected region `Lambda` with
zero exterior extension. This is a grounded finite computation, not a wall
or a completed-infinity ontology. `M_d` is positive definite by the exact
band `2/9<=M_d<=2`. `M_s` is positive definite because

\[
 q^T Kq=
 \sum_{\{x,y\}_f}{1\over9}(q_x-q_y)^2
 +\sum_{\{x,y\}_e}{1\over18}(q_x-q_y)^2,                 \tag{4}
\]

with the zero extension included; equality would force a finitely supported
constant field and hence `q=0`.

For either `M` and a fixed source `b`, define

\[
 \Phi(q)={1\over2}q^TMq+b^Tq,
 \qquad q_*=-M^{-1}b,
 \qquad \mathcal E(q)={1\over2}(q-q_*)^TM(q-q_*).        \tag{5}
\]

The update may read only the local residual `Mq+b`. The nonlocal notation
`q_*` is permitted only to prove positivity and convergence; reading `q_*`
or any target profile invalidates the result.

## 4. Frozen eight-color source-centered gate

Color the cubic lattice by

\[
 \chi(x)=(x_1\bmod2,x_2\bmod2,x_3\bmod2)\in\{0,1\}^3. \tag{6}
\]

No face or edge neighbor has the same color, so for one active color `A`,

\[
 M_{AA}=dI.                                               \tag{7}
\]

Let `E_A` embed the active coordinates and set

\[
 g_A=E_A^T(Mq+b),\qquad u_A={g_A\over\sqrt d}.           \tag{8}
\]

Give every active cell one fresh complete port `(a,pi_a)` and use the
configuration endpoint

\[
 u_A'=a_A,\qquad a_A'=-u_A,                              \tag{9}
\]

equivalently

\[
 q'=q+E_A{a_A-u_A\over\sqrt d}.                          \tag{10}
\]

On the fresh section `a_A=0`, equation (10) is exact block coordinate
relaxation:

\[
 q'=q-E_A{E_A^T(Mq+b)\over d}.                           \tag{11}
\]

The certificate must prove, exactly,

\[
 \Phi(q')-\Phi(q)={1\over2}(\|a_A\|^2-\|u_A\|^2),      \tag{12}
\]

and therefore

\[
 \boxed{\mathcal E(q')+{1\over2}\|a_A'\|^2
 =\mathcal E(q)+{1\over2}\|a_A\|^2}.                   \tag{13}
\]

The outgoing port carries the removed residual amplitude, sign, and energy.
It is history, not erased detail.

## 5. Frozen canonical and positive-Hamiltonian gates

For one active scalar coordinate and all inactive coordinates grouped as
`y`, write

\[
 M=\begin{pmatrix}d&c^T\\c&R\end{pmatrix}.
\]

On source-centered deviations and the port coordinate, the endpoint matrix
is

\[
 S=\begin{pmatrix}
 0&-c^T/d&1/\sqrt d\\
 0&I&0\\
 -\sqrt d&-c^T/\sqrt d&0
 \end{pmatrix},
 \qquad
 G=\operatorname{diag}(M,1).                             \tag{14}
\]

The certificate must verify

\[
 S^4=I,\qquad S^TGS=G,                                   \tag{15}
\]

and that the cotangent lift `diag(S,S^{-T})` is symplectic. For all mutually
nonadjacent sites in one color, the normalized modes

\[
 u_i={(Mq+b)_i\over\sqrt d},\qquad
 \pi_{u_i}={p_i\over\sqrt d}                            \tag{16}
\]

must obey `{u_i,pi_u_j}=delta_ij`.

The positive local Hamiltonian layer is the already frozen FTD-0886
construction, summed over the active color:

\[
 N={1\over2}\sum_i(u_i^2+a_i^2+\pi_{u_i}^2+\pi_{a_i}^2),
 \qquad
 L=\sum_i(a_i\pi_{u_i}-u_i\pi_{a_i}),                    \tag{17}
\]

\[
 H=\omega I+\omega N
 +\sigma{\omega\over4}(1-\cos\theta)L.                  \tag{18}
\]

The exact checks are `{N,L}=0`, `|L|<=N`, carrier lower bound
`H-omega I>=omega N/2`, and the one-cycle endpoint (9) on the
zero-conjugate section. This establishes a positive local stroboscopic gate,
not an autonomous eight-color clock or source dynamics.

No-port fresh relaxation is noninjective. Within the registered onsite
nondegenerate canonical class, at least one complete port pair is therefore
necessary, and equation (9) is sufficient. No universal minimum outside that
class is claimed.

## 6. Frozen finite-region convergence

On centered error `e=q-q_*`, one fresh color layer is

\[
 P_A=I-E_A d^{-1}E_A^TM.                                  \tag{19}
\]

The certificate must prove

\[
 P_A^2=P_A,qquad P_A^TM=MP_A,                            \tag{20}
\]

\[
 \mathcal E(e)-\mathcal E(P_Ae)
 ={1\over2d}\|E_A^TMe\|^2.                              \tag{21}
\]

Freeze the color order as binary lexicographic
`000,001,010,011,100,101,110,111` and define the sweep

\[
 P=P_{111}\cdots P_{001}P_{000}.                         \tag{22}
\]

For either positive-definite finite-region operator, every factor is
nonexpansive in the `M` norm. Equality through a complete sweep would make
the residual vanish on all eight colors, hence `Me=0` and `e=0`. Compactness
of the finite-dimensional unit `M` sphere then gives a region-dependent

\[
 \|Pe\|_M\le\rho_{\Lambda,M}\|e\|_M,
 \qquad 0\le\rho_{\Lambda,M}<1.                          \tag{23}
\]

The certificate must check the exact projection identities and fixed-point
intersection, and supply exact rational witnesses on the zero-extended
`3x3x3` region for both `M_d` and `M_s`. Numerical eigenvalue estimates are
not permitted or needed.

## 7. Frozen causal and massless-halo boundary

Each color layer reads only the C18 radius-one neighborhood. Starting from a
compact source and zero field, after `t` color layers the dependency cone has
radius at most `t`. Thus equations (10)--(23) give a target-blind causal
formation law for every specified finite grounded solve.

This does not yield a volume-independent massless rate. On the exact static
line `(e^{i theta},1,1)`,

\[
 \kappa(\theta)={2\over3}(1-\cos\theta)\longrightarrow0. \tag{24}
\]

The certificate must preserve the FTD-0929 conclusion: no strict uniform
geometric contraction factor may be promoted for the uncontained static
halo. It must also leave open local convergence to, boundary independence
of, and physical persistence of an uncontained static Green profile. Finite
grounded convergence is a construction witness, not an ontological wall or
an `L to infinity` proof.

## 8. Port, source, and scheduling boundary

One complete fresh port pair per active site is sufficient for one color
layer. After the gate, `a'=-u` is generically nonzero and is not fresh. A
complete sweep therefore consumes one fresh pair per site; `N` sweeps consume
`N` such pairs unless a separately derived reset, return, compression, or
open-rail mechanism is supplied.

FTD-0875 proves that a positive local rail can transport a prepared record,
and FTD-0884 proves that a finite cyclic bank cannot guarantee indefinite
freshness. This certificate may therefore identify an open/bilateral
prepared-blank rail as a representation-level environment, but may not claim
native three-dimensional routing, absence of congestion, finite recycling,
or free blank preparation.

The source `b` is fixed during every registered layer and sweep. Source
formation, motion, recoil, time-dependent switching work, tracking lag,
nonlinear saturation, stopping, recovery, and erasure-to-heat bookkeeping
remain open. Existing left/right production fields are not identified with
the field/port pair.

## 9. Registered outcomes

- **Outcome A — autonomous positive formation closure:** the exact gates and
  finite/uncontained convergence pass, and existing substrate types supply a
  closed local fresh-port recycler, autonomous eight-color schedule, moving
  source/recoil, and boundary-independent persistent static halo.
- **Outcome B — positive local port relaxation / massless-halo boundary:**
  the one-color gate is exact, positive, canonical, target-blind, and minimum
  within the registered local class; all finite grounded dynamic/static
  solves converge under eight colors; indefinite port supply, autonomous
  scheduling, source dynamics, and uncontained static-halo convergence and
  persistence remain open.
- **Outcome C — positive relaxation fails:** locality, energy conservation,
  canonicality, finite-region convergence, or target-blindness fails.
- **Invalid:** source drift, post-lock formula/tolerance change, numerical
  search or fitted rate, target/profile/context/Born read, engine/CMake
  mutation, silent L/R identification, uncontained-limit promotion, or
  failed combined gate.

## 10. Firewalls

No engine source, CMake target, `Voxel` field, toggle, default, production
law, ontology type, paper, physical constant, or phenomenological formula is
changed. No numerical near-miss search, fit, parameter sweep, formula
substitution, or continuum-limit rhetoric is permitted.

Even Outcome A would not establish a physical `G*` cadence, Born frequencies,
Bell correlations, measurement context, operational Lorentz hiding,
mass/scale, gravity, a native spin-2 carrier, or framework completeness.
Outcome B additionally leaves fresh-port origin/recycling, three-dimensional
routing, autonomous switching, source recoil, uncontained static-halo
formation, physical L/R identity, recovery, and production integration open.
