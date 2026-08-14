# FTD-0929 — Preregistration: quasilocal companion preparation and reversible-history formation boundary v1

**Identifier:** `FTD-0929`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** unique source-to-companion map forced by FTD-0928 self-duality;
finite-support and finite-causal-depth obstruction; gapped Neumann preparation
with exact locality/covariance/error bounds; minimum fresh-pair cotangent
dilation of one local contraction layer; loss/history and positive-energy
boundary; static-halo gap control; existing dual-field capacity audit; no
numerical search, fit, engine mutation, ontology adoption, `G*`, Born, Bell,
context, outcome, or hiding read

## 1. Question

FTD-0928 proves that the frozen one-way source is not canonical and constructs
the minimum reciprocal action with a field-shaped companion `Q`. On the
self-dual section its source is

\[
 U=(K-2I)Q.                                               \tag{1}
\]

Does the present matter record/current determine `Q` through a strictly local
finite-time PreparationMap using existing types, and can that preparation be
made reversible and positive-energy without preloading the completed target
profile?

The certificate must distinguish:

1. mathematical uniqueness of the companion;
2. strict finite-range locality versus exponential quasilocality;
3. causal approximation versus exact finite-time formation;
4. reversibility versus positive-energy conservation; and
5. representation capacity versus physical identification of the production
   left/right fields.

None may be inferred from another.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `PREREG_SELF_DUAL_RECIPROCAL_DISCRETE_ACTION_AND_FORMATION_RESERVOIR_BOUNDARY_v1.md` | `27BD89002B2B432FB58950B639B56E0FD22C5511E48550AD026DB462BEE2E076` |
| `THEOREM_SELF_DUAL_RECIPROCAL_DISCRETE_ACTION_AND_FORMATION_RESERVOIR_BOUNDARY_v1.md` | `A7DC30C90C491976F58CDEAF71FB5ABFCE04952ECE971CA7FF72C65A7B9B90BF` |
| `proof_self_dual_reciprocal_discrete_action_formation_reservoir_boundary.py` | `E41455B589705E1B3B2F4ECCFABD5F0AF28DE303AD216DE700B241EBFB113AE0` |
| `THEOREM_TERNARY_CONTINUITY_MIDPOINT_SOURCE_RECURRENCE_AND_CANONICAL_RECIPROCITY_BOUNDARY_v1.md` | `B3140D967A3593846B7A8FB0D9682C403E379540F3314AF9CFFF25A649EF20EF` |
| `proof_ternary_continuity_midpoint_source_recurrence_canonical_reciprocity.py` | `E0A03721A089B43137EC986E1EB2024D9AF93B43062603B4C23FF5CA32E806B9` |
| `THEOREM_TERNARY_DIPOLE_CORE_EVANESCENT_C4_REFERENCE_ORBIT_AND_AUTONOMY_BOUNDARY_v1.md` | `DB9894C1554422B0BA0C97A991FFF7F714B83EF673DDF5FEDA026B45C55B88AF` |
| `THEOREM_NATIVE_C4_MODAL_CIRCULATION_AND_COMPACT_SUPPORT_OBSTRUCTION_v1.md` | `CA05D786A73775B398F90EE33E207E2A4D3522D49ECA86B9BF5774E2D6B1A285` |
| `THEOREM_REVERSIBLE_CHECKERBOARD_GAUSS_RECORD_PREPARATION_AND_SELF_DUAL_ENERGY_SPLIT_v1.md` | `143D897A69B5C6FED8C00402C1840EA9FAEE5BD4BC259C9BDD065DFDC616A814` |
| `THEOREM_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_BOUNDARY_v1.md` | `0D5A093597CE7BFFF7F593C0A1AF2B65E6CDE99DB0FFEDA1183D9849BC58624F` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |

The certificate fails closed on source drift.

## 3. Frozen dynamic companion

Let `K` be the symmetric C18 production stiffness. Its Fourier symbol has
band

\[
 0\le\kappa\le {16\over9}.                              \tag{2}
\]

For the exact FTD-0927 dynamic source `U_n`, equation (1) forces

\[
 \boxed{Q_n=-(2I-K)^{-1}U_n.}                            \tag{3}
\]

Because `2I-K >= (2/9)I`, this solution is unique in `ell^2`. This is a
mathematical PreparationMap. It is not yet a physical formation law.

The certificate must reconstruct the registered arm-zero source from the
frozen record/current law and verify its support, norm, and `C4` covariance.

## 4. Frozen finite-support discriminator

Use Laurent variables `(x,y,z)`. With face sum `F` and edge sum `E`, define

\[
 18(2-\kappa)=12+2F+E.                                  \tag{4}
\]

On the slice `y=z=1`, equation (4) is

\[
 18(2-\kappa(x,1,1))=6(x+x^{-1}+4).                     \tag{5}
\]

The reconstructed arm-zero dynamic-source x-component must be

\[
 U_x(x,1,1)={(x-1)^2(x+1)^2\over4x^2}.                  \tag{6}
\]

At `x=-2+sqrt(3)`, equation (5) vanishes while equation (6) does not.
Therefore the denominator does not divide the source in the Laurent ring,
and the unique companion cannot have finite support. The certificate must
prove this exact witness and a multivariate polynomial-division remainder
control.

Consequently no fixed finite number of radius-one causal ticks can form the
exact uncontained companion from compact data. This no-go is only for exact
finite-time/finitely supported preparation; it does not exclude causal
convergence.

## 5. Frozen quasilocal causal preparation

Since `||K/2||<=8/9`, define

\[
 Q^{(0)}=0,
 \qquad
 Q^{(N+1)}={K\over2}Q^{(N)}-{U\over2}.                  \tag{7}
\]

Then

\[
 Q^{(N)}=-{1\over2}\sum_{m=0}^{N-1}\left({K\over2}\right)^mU, \tag{8}
\]

and

\[
 \boxed{
 \|Q-Q^{(N)}\|_2
 \le {9\over2}\left({8\over9}\right)^N\|U\|_2.}       \tag{9}
\]

Each layer is radius one, so a value after `N` layers depends only on the
`N-1` C18-neighborhood of the source. On nested finite regions, the maps must
agree at every point whose complete dependency cone lies in the smaller
region. Since `K` commutes with signed-cubic rotations, the preparation must
preserve the registered `C4` covariance exactly.

Equation (7) reads the present source, not a target field, target arm, final
profile, measurement context, or probability.

## 6. Frozen reversible-history lift

The reduced configuration contraction in equation (7) is not invertible on
the full field space because `K` has a zero mode. Register one fresh
field-shaped environment coordinate `e` and outgoing history coordinate `h`
per layer:

\[
 z'=z,
 \qquad q'=Aq+e+Bz,
 \qquad h'=q,                                            \tag{10}
\]

where `A=K/2` and `Bz=-U/2`. Its coordinate Jacobian is

\[
 J=\begin{pmatrix}
 I&0&0\\
 B&A&I\\
 0&I&0
 \end{pmatrix},                                         \tag{11}
\]

with local inverse

\[
 J^{-1}=\begin{pmatrix}
 I&0&0\\
 0&0&I\\
 -B&I&-A
 \end{pmatrix}.                                         \tag{12}
\]

The cotangent lift `diag(J,J^{-T})` must be proved symplectic. On the
fresh-coordinate section `e=0`, equation (10) implements one layer of
equation (7) and exports the overwritten `q` into `h`. Reusing that port as
fresh without reset is forbidden. Discarding `h` realizes a lossy reduced
map, not reversible fundamental dynamics.

This is a registered information/history witness, not a positive reservoir.
For any scalar mode `0<a<=8/9`, the local coordinate block

\[
 C_a=\begin{pmatrix}a&1\\1&0\end{pmatrix}               \tag{13}
\]

has an eigenvalue with modulus greater than one. Its cotangent lift therefore
cannot preserve a positive-definite quadratic metric. The certificate must
prove this. Canonical reversibility alone is not positive-energy formation.

The FTD-0928 equal-metric species quarter turn remains positive and
energy-preserving, but it requires the reservoir pair to contain the complete
prepared target phase. It may not be counted as deriving equation (3).

## 7. Static-halo control

The gapped dynamic inverse `2I-K` must not be conflated with static halo
preparation. The static equation uses the massless operator `K`. Along the
exact slice `(x,y,z)=(exp(i theta),1,1)`,

\[
 \kappa={2\over3}(1-\cos\theta)\longrightarrow0.        \tag{14}
\]

Hence a fixed local Richardson factor `1-eta K` has contraction supremum one
on the uncontained band. No volume-independent geometric rate analogous to
equation (9) is permitted for the static halo.

## 8. Existing-type audit

Production already stores the complete field-shaped pairs
`(flux_L,wave_vel_L)` and `(flux_R,wave_vel_R)`. This proves representation
capacity only. The certificate must verify that production:

- defines observable flux as `flux_L+flux_R` and chirality as their
  difference;
- propagates the two Laplacians separately;
- applies the same prescribed source to both; and
- contains neither equation (7), equation (10), nor the FTD-0928 reciprocal
  cross-operator.

No identification of `(X,Q)` with `(L,R)`, their common/relative modes, or a
normalization is adopted by this certificate.

## 9. Registered outcomes

- **Outcome A — existing-type local positive formation closure:** an exact
  finite-depth local PreparationMap forms both companion and static halo from
  unprepared existing fields, with reversible positive reservoir work and no
  target/profile read.
- **Outcome B — unique quasilocal companion / reversible-history boundary:**
  equation (3) is unique and equation (7) converges causally with equation
  (9), but exact finite-time support closes negative; equation (10) gives a
  local canonical history lift while positive-energy autonomous formation,
  port supply/recycling, dual-field identification, and static-halo formation
  remain open.
- **Outcome C — quasilocal preparation fails:** uniqueness, convergence,
  covariance, locality, or the registered reversible lift fails.
- **Invalid:** source drift, post-lock coefficient/tolerance change, numerical
  search, fitted decay, target/profile/context/Born read, engine/CMake
  mutation, or failed combined gate.

## 10. Firewalls

No engine source, CMake target, `Voxel` field, toggle, default, production
law, ontology type, or paper is changed. No dual field is silently promoted
to the physical companion. No numerical near-miss search, fit, sweep, or
formula-substitution discovery is permitted.

Even Outcome A would not establish a physical `G*` cadence, Born frequencies,
Bell correlations, measurement context, Lorentz hiding, mass/scale, or
framework completeness. Outcome B additionally leaves positive source work,
fresh-port origin/recycling, autonomous stopping, nonlinear full-profile
transfer, static-halo preparation, recovery, and production integration open.

