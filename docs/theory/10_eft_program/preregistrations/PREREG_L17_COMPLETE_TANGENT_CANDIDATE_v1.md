# FTD-0774 — L=17 complete tangent candidate v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Scope:** first-internal-doublet-seeded complete matter--field tangent
candidate of the unchanged selected connected common-action map at the
registered numerical FTD-0638/0639 `L=17` fixed-point representative  
**Production impact:** none  
**Date:** 2026-08-02

## 1. Question

Can the complete selected one-tick matter--field endpoint be differentiated
consistently, to the locked tolerances, along the registered probe and
`K_16(F,B_0)` directions in the `L=17` chart? If so, does that finite seeded
construction resolve and qualify a four-dimensional positive-energy candidate
on which the measured one-tick restriction acts approximately as two equal
rotations to the locked tolerances?

This is the smallest complete-state gate between the failed bare matter clock
and an autonomous coupled recurrence. It does **not** test spatial
localization, volume stability, nonlinear continuation, finite-period return,
quartic occupancy, or a clock--rod ratio. Those require separately locked
successors and are licensed only by the constructive verdict below.

"Native" here means native to the frozen **selected** FTD-0622 connected
common-action branch. It does not mean forced by P1--P5 or used by the
production `RenderBridge` tick.

## 2. Frozen provenance, representative, and options

Before evaluating any new endpoint, the driver must verify the following
byte-level SHA-256 fingerprints:

| parent | artifact | SHA-256 |
|---|---|---|
| FTD-0638 analytic center | `engine/results/ftd_0638/ftd_0638_connected_block_analytic_static_refinement_v1.json` | `435493EDC8E5DA5B34CF416EB6445C537A1F6ED9ABFCE02BB032DE2486C1B18C` |
| FTD-0638 state record | `engine/results/ftd_0638/ftd_0638_connected_block_analytic_static_refinement_states_v1.csv` | `8A717BC9DFE3A43FB21A6B46EF723BD2649D5F1F5BC2174BBA6027D25550214F` |
| FTD-0639 fixed-point run | `engine/results/ftd_0639/ftd_0639_connected_block_analytic_dynamical_rest_v1.json` | `DFA39E27F0317165D2A85E7778BBC7DA5691D1449DEEF20B4990C2AB9A1E7BD6` |
| FTD-0640 matter modes | `engine/results/ftd_0640/ftd_0640_connected_block_analytic_matter_modes_v1.json` | `AB43D342CFE48BEF452955E56B1EDC34F9EE51911F7D899932E7E542877E6B9A` |
| FTD-0640 mode basis | `engine/results/ftd_0640/ftd_0640_connected_block_analytic_matter_modes_modes_v1.csv` | `FE9F916443F8A8BF8F04B53067741919B203AF4C726D9DD67134B0BB43ECEFFD` |
| FTD-0641 field control | `engine/results/ftd_0641/ftd_0641_connected_block_independent_field_modes_v1.json` | `EA24EF12476533DB8395C0E64C1E381A6605662EAA9ED35C1E38D66D560189E6` |

The selected-map source revision is Git commit
`93748ac2021e4db5a9b8583cc28493332c716ac0`. The pre-execution driver must
also require no tracked working-tree difference from that revision under
`engine/include/ftd/` or `engine/src/eft/`. This freezes the full tracked
constant/include closure as well as the selected-map implementation. New
test-only files do not alter this gate.

Load exactly orientation `0`, initializer
`initialize_connected_moore_block(17,2,0,0,0.5,1e-13,4096)`, and the refined
`x1,y1,z1` columns of the orientation-`0` FTD-0638 state rows. Rebuild its
minimum longitudinal dressing with fibre limit `8`. Load only orientation-`0`
FTD-0640 mode rows. No orientation or state-column substitution is allowed.

Freeze the primary endpoint options as:

```text
wave_speed                         C_SPEED
dt                                 1
binding_stiffness                  1
binding_law                        FixedEdgeQuartic
compact_pair_well_depth            0.01
compact_pair_cutoff_distance_sq    1.5
constituent_mass_scale             1
polarity_scale                     1
field_energy_scale                 1
gate_tolerance                     1e-10
solve_tolerance                    2e-13
finite_difference_scale            2e-7
max_iterations                     64
allow_shared_anchor_chart          true
use_sparse_local_current           true
use_local_residual_evaluation      true
use_low_rank_identity_broyden      false
use_matrix_free_newton_krylov      false
defer_volume_diagnostics           false
measure_final_root_regularity      false except locked preflight probes
root_momentum_seed                 empty
```

The sparse/local-residual route changes storage and evaluation route only; the
accepted root is materialized and rechecked by the unchanged complete
transaction. No Newton/Broyden/Krylov route substitution is allowed.
Every adjudicative endpoint passes a null nonlinear-solve cache and therefore
uses the frozen direct route. Populated caches are exercised only by the
non-adjudicative method control in section 8.

FTD-0659, FTD-0676, and FTD-0699 are contextual rather than execution
parents. They establish that the bare matter doublet has a coherent projected
phase, loses matter-only action, and transfers energy resonantly into the
propagating field band. Their stored artifacts omit the complete per-tick
state and cannot answer the present question retroactively.

The registered first-doublet phase and target cosine are

\[
 \phi_{\rm int}=1.0911648733663635,
 \qquad \mu_0=\cos\phi_{\rm int}.
\]

They set the search window but do not force the coupled output phase:

\[
 |\Omega-\phi_{\rm int}|\le0.08.                 \tag{1}
\]

## 3. Fixed constraint chart and codec

Freeze charges, Moore binding graph, width, orientation, anchor-fibre limit,
B-spline knot cells, and every discrete chart sector. With `N_m=16`, the raw
storage count is

\[
 D_{\rm raw}=6(17)^3+6N_m=29{,}574.
\]

Gauss removes `17^3-1` independent electric coordinates. Because the engine
admits every finite edge-magnetic field, the independent chart dimension is

\[
 D_{\mathcal C}=5(17)^3+6N_m+1=24{,}662.         \tag{2}
\]

Discrete anchors and topology are metadata, not tangent coordinates. Any site
hop, knot-cell crossing, graph change, width/orientation change, or
anchor-sector change invalidates the derivative evaluation.

Let `z_hat` denote the registered numerical representative. Use the local
chart

\[
 \xi=(\delta x,\delta p,e_T,b),                  \tag{3}
\]

where `e_T` is a divergence-free face field and
`b in R^(3*17^3)` is unrestricted. Thus all three uniform electric means and
all magnetic curl-kernel components are retained; no harmonic is silently
fixed or projected out.

At `z_hat`, compute the analytic fractional-density tangent
`D rho[delta x]` from the frozen quadratic-coat basis. Define
`L delta x` to be its zero-mean-gauge minimum longitudinal face field, using a
source-normalized Poisson solve with relative residual `<=1e-13`. This is the
linear longitudinal chart map.

The physical retraction `chi(h xi)` is:

1. displace the effective constituent positions in the locked unwrapped chart
   by `h delta x`;
2. rebuild the **nonlinear** minimum longitudinal dressing with fibre limit
   `8`, Poisson tolerance `1e-13`, and at most `4096` iterations;
3. set momenta to `p_hat+h delta p`, add `h e_T` to the redressed electric
   field, and add `h b` to `B_hat` without another projection;
4. require finite values, fractional Gauss residual `<=1e-10`, unchanged
   metadata and knot/anchor sectors, and zero hops.

To encode the centered output pair `y_+=Phi(chi(h xi))` and
`y_-=Phi(chi(-h xi))`:

1. subtract positions in the locked unwrapped chart and divide positions,
   momenta, face fields, and edge fields by `2h`, producing
   `(delta x_o,delta p_o,delta E_o,delta B_o)`;
2. compute `L delta x_o` by the analytic tangent-source solve above;
3. set `r=delta E_o-L delta x_o`, Hodge-project only the numerical
   longitudinal residue out of `r`, and call the result `e_T,o`;
4. retain `b_o=delta B_o` without projection.

The pre-clean divergence must be `<=2e-7`, the cleaned divergence `<=1e-10`,
and the Hodge correction and reconstruction residual must each be `<=2e-4`
relative to `max(||delta E_o||_2,1e-30)`. The three face and three edge mean
coefficients must reconstruct to `<=1e-12` relative. Failure is
execution-invalid. Hodge cleaning is a numerical tangent-chart codec, not a
new physical step.

## 4. Exact conditional theorem and numerical energy form

For the exact theorem, let `chi:U subset R^d -> C` be the fixed constraint
chart about an ideal exact representative `z_*`, and define

\[
 f=\chi^{-1}\Phi\chi,\qquad e=E\chi,\qquad
 T=Df(0),\qquad K=D^2e(0).                       \tag{4}
\]

Assume `f` and `e` are `C^2`, the selected implicit endpoint is locally
single-valued, and

\[
 f(0)=0,\qquad De(0)=0,\qquad e\circ f=e.        \tag{5}
\]

Differentiating twice gives

\[
 T^TKT=K.                                        \tag{6}
\]

The chart Hessian in (4), not an ambient Hessian restricted after the fact,
is load-bearing. Away from criticality a term
`De(0) D^2f(0)` remains. FTD-0638/0639 supplies only a highly resolved
**numerical** representative, so (6) is not assumed of the measured map; the
finite-dimensional adjoint and isometry gates below must measure it.

On chart vectors the registered bilinear form is

\[
\begin{aligned}
 \langle\xi,\eta\rangle_K={}&
 \delta p_\xi^TM^{-1}\delta p_\eta
 +\delta x_\xi^TH_{\rm red}\delta x_\eta\\
 &+\beta\Bigl(\langle e_{T,\xi},e_{T,\eta}\rangle
 +\langle b_\xi,b_\eta\rangle\\
 &\hspace{1.6cm}-\frac{\lambda}{2}\left[
 \langle b_\xi,C^Te_{T,\eta}\rangle+
 \langle b_\eta,C^Te_{T,\xi}\rangle\right]\Bigr), \tag{7}
\end{aligned}
\]

with `M=M_INERTIAL I`,
`beta=mapped_field_work_coefficient*field_energy_scale`,
`lambda=C_SPEED`, and the orientation-`0` FTD-0638 analytic chart Hessian
`H_red`, recomputed before use. Longitudinal dressing is already contained in
`H_red`; Hodge orthogonality prevents counting it again in the transverse
term.

FTD-0638 predicts `lambda_min(H_red)>0`. At odd `L=17`,

\[
 \lambda\sigma_{\max}=2\cos(\pi/34)<2,
\]

so the exact matched-field block is positive, including its uniform harmonic
block. The identity action of a uniform mode applies only to the isolated
source-free field operator; the complete map may couple it to matter.

Global positivity is an execution gate, not an inference from sampled norms.
The recomputed `H_red` must have antisymmetry `<=1e-12`, eigensystem residual
`<=1e-7`, orthogonality residual `<=1e-10`, and
`lambda_min(H_red)>1e-5`. Also require finite `beta>0` and independently
recompute the positive field lower bound
`beta*(1-cos(pi/34))>0`. Serialize `H_red`, its eigenvalues, and these
certificates for independent replay.

If `K>0`, (6) gives `T^sharp=T^-1`, and therefore

\[
 S=\frac{T+T^{-1}}2,\qquad S^\sharp=S.           \tag{8}
\]

Then `K^(1/2) T K^(-1/2)` is real orthogonal. A unit-circle pair
`exp(+-i Omega)` becomes the real `S` eigenvalue `cos(Omega)`. A real
four-dimensional `S` eigenspace away from `+-1` that is invariant under `T`
therefore decomposes into two equal-angle rotation planes.

The complete symplectic form has not been derived. The accepted object may be
called a **numerically preserved positive quadratic form** or an
**approximate positive quadratic tangent invariant at the locked tolerances**,
but not a Krein signature, canonical action, or action variable.

## 5. Seeds, energy probes, norms, and endpoint preflights

For orientation-`0` FTD-0640 modes `m=6,7`, let `v_m` be the stored
mass-normalized vector, so `v_i^T M v_j=delta_ij`. Define raw seeds

\[
 q_m=(v_m,0,0,0),\qquad
 p_m=(0,Mv_m,0,0),                               \tag{9}
\]

then perform two complete `K`-MGS passes in the exact order

\[
 B_0=(q_6,q_7,p_6,p_7).                          \tag{10}
\]

The resulting columns are `K`-orthonormal. The four matter-mixed probes are
exactly one half times the signed sums given by the rows

```text
(1, 1, 1, 1), (1, -1, 1, -1), (1, 1, -1, -1), (1, -1, -1, 1).
```

Define the locked FTD-0641 cosine shape from
`family=100`, `wave=(1,0,0)`, `permutation=0`, `polarization=0`. Let `f_e` be
the corresponding pure transverse-electric tangent normalized in `K`, and
let
`f_b=+C^T f_e/||C^T f_e||_K` be the pure magnetic tangent. Let `h_E^x` and
`h_B^x` be the `K`-unit uniform x-directed
face and edge harmonics. The eight field/cross probes are

\[
 f_e,\ f_b,\ \widehat{f_e+f_b},\ \widehat{f_e-f_b},\
 h_E^x,\ h_B^x,\ \widehat{q_6+f_e},\ \widehat{p_6+f_b}, \tag{11}
\]

where each hat means `K` normalization. Equations (10), the four matter-mixed
rows, and (11) give sixteen locked probes spanning every term of (7).

Use a separate energy step `h_E=2e-4`. For every probe compute the centered
slope and second variation from **termwise cancellation-free energy
increments**, never by subtracting two complete energy totals. The increment
must use:

- the rationalized relativistic kinetic excess;
- the quartic binding product difference;
- `delta rho dot phi_0 + (delta rho dot delta phi)/2` for the redressed
  longitudinal field; and
- the exactly expanded quadratic transverse/magnetic/curl-cross polynomial.

Require analytic constrained-gradient infinity norm `<=1e-10`, absolute
centered slope `<=1e-8`, and

\[
 \frac{|\Delta_E^2(\xi;h_E)-\langle\xi,\xi\rangle_K|}
 {\max(\langle\xi,\xi\rangle_K,10^{-30})}\le10^{-6}               \tag{12}
\]

on all sixteen probes. Every basis and Krylov norm must be finite and
strictly positive. Absolute values, clipping, Euclidean replacement, or a
fitted metric are forbidden.

Explicitly, with every increment evaluated by the cancellation-free terms
above,

\[
 \delta E_\pm=E(\chi(\pm h_E\xi))-E(\chi(0)),\quad
 g_E=\frac{\delta E_+-\delta E_-}{2h_E},\quad
 \Delta_E^2=\frac{\delta E_++\delta E_-}{h_E^2}.                 \tag{13}
\]

For vectors and vector blocks define

\[
\|u\|_K=\sqrt{\langle u,u\rangle_K},\quad
 \|X\|_{K,F}=\sqrt{\operatorname{tr}(X^TKX)},\quad
 r_K(a,b)=\frac{\|a-b\|_K}{\max(\|b\|_K,10^{-30})}.               \tag{14}
\]

All ordinary small-matrix residuals use the Frobenius norm divided by
`max(reference Frobenius norm,1e-30)`. `max` means the maximum over the named
set, never an unregistered quantile.

Set `h_0=2e-6` and `h_1=1e-6=h_0/2`; there is no third derivative scale.
For `h in {h_0,h_1}`,

evaluate the centered forward and reverse derivatives

\[
 T_h\xi=\operatorname{Codec}_h
   (\Phi(\chi(h\xi)),\Phi(\chi(-h\xi))),\qquad
 T_h^{-1}\xi=\operatorname{Codec}_h
   (\Phi^{-1}(\chi(h\xi)),\Phi^{-1}(\chi(-h\xi))),                \tag{15}
\]

where `Codec_h` is exactly the centered output codec of section 3, including
its division by `2h`, and the reverse route takes
`solve_connected_moore_block_reverse(...).earlier`. Adjudicative paths pass
`nullptr`; no mutable cache is shared between signs, directions, or parallel
tasks.

On all sixteen probes require:

- fixed-point one-step excursion `<=1e-10`;
- endpoint common-action residual `<=1e-10` and complete-energy drift
  `<=1e-12`;
- one-step forward/reverse raw-state recovery `<=1e-10`;
- `r_K(T_(h_0) xi,T_(h_1) xi)<=1e-3`, and the reverse analogue;
- `T_(h_0)^-1 T_(h_0)` and `T_(h_0) T_(h_0)^-1` residuals `<=1e-4`;
- every retraction, codec, Gauss, harmonic, metadata, and chart gate above.

For all `16x16` ordered probe pairs `(u,v)`, require

\[
 \frac{|\langle u,T_hv\rangle_K-
 \langle T_h^{-1}u,v\rangle_K|}
 {\max(\|u\|_K\|v\|_K,10^{-30})}\le10^{-4}.       \tag{16}
\]

On each signed preflight endpoint only, enable the observer-only final-root
regularity measurement with `h_root=2e-7`, and require `sigma_min>=1e-3`,
condition number `<=1e4`, and the `h_root` versus `h_root/2` sigma-min relative
difference `<=1e-5`. An observer-on/off base regression must reproduce the
endpoint to `1e-12`. These gates numerically support a locally single-valued
branch; they do not prove an analytic determinant bound.

Preflight failure is execution-invalid, not evidence against a coupled
candidate.

## 6. Deterministic matrix-free construction

Use primary step `h_0=2e-6` and

\[
 F=I-\frac14(S-\mu_0I)^2.                         \tag{17}
\]

On an ideal positive-energy spectrum, (17) ranks **cosines** by distance from
`mu_0`; it does not rank phase distance directly. It also has the exact
reflection degeneracy `F(mu_0+d)=F(mu_0-d)` and is a weak filter. These facts
may produce an unresolved result but cannot be repaired after inspection.

Construct

\[
 \mathcal K_{16}(F,B_0)=
 \operatorname{span}\{B_0,FB_0,\ldots,F^{15}B_0\},               \tag{18}
\]

in block order, preserving the within-block order (10), with two complete
`K`-MGS passes. Cache `Tv` and `T^-1v` when `Sv` is first evaluated, and use

\[
 w=Sv-\mu_0v,\qquad Fv=v-\tfrac14(Sw-\mu_0w).                    \tag{19}
\]

Maximum accepted dimension is `64`; `V_48` is the first 48 accepted columns
of the same run when they exist. Process every generated four-column block in
the fixed column order. A post-MGS norm below `1e-12` deflates that column, but
the remaining columns in the block are still processed and the four raw
recurrences `F^k B_0` continue. A happy breakdown occurs only when an entire
four-column block contributes zero accepted columns; then the run stops. The
terminal space remains eligible only if it is `T/T^-1` invariant to `2e-4`.
For a terminal dimension below 64, compare against the basis dimension just
before the last nonempty generated block. An invariant terminal dimension `4`
waives only that prior/final comparison and must still pass the independent
`h_1` construction. A terminal dimension below `4`, a non-invariant happy
breakdown, or exhaustion of 16 powers without a usable comparison dimension
is unresolved. Nonfinite norms or inconsistent bookkeeping are
execution-invalid.

Require

\[
 \|V^TKV-I\|_F\le10^{-10}.                         \tag{20}
\]

At every adjudicated dimension form the raw matrices

\[
 A_S=V^TKSV,\quad A_T=V^TKTV,\quad A_{T^{-1}}=V^TKT^{-1}V.       \tag{21}
\]

Require

\[
 \frac{\|A_S-A_S^T\|_F}{\max(\|A_S\|_F,10^{-30})}\le10^{-4}.   \tag{22}
\]

Only after (22) passes may the symmetric eigensolver receive
`Abar_S=(A_S+A_S^T)/2`. Its spectrum must lie in
`[-1-2e-4,1+2e-4]`. If any accepted eigenvalue lies outside the exact phase
domain `[-1,1]`, the solve is unresolved; no value is clipped or excluded.
No failing matrix is silently repaired.

Map each eigenvalue in `[-1,1]` to `Omega=acos(mu)`, sort by phase, and form
maximal spectral clusters whose adjacent phase gaps are `<=5e-4`. A cluster
is eligible only if all its phases satisfy (1). For a `K`-orthonormal cluster
basis `W`, define it as seed-linked exactly when
`tr(W^T K B_0 B_0^T K W)>=0.10`. A seed-linked cluster of rank greater than
four makes the solve unresolved. Rank below four is not a candidate. Every
eligible isolated rank-four cluster is reconstructed and qualified; no single
maximum-overlap quartet is allowed to stand for all clusters.

For a `K`-orthonormal candidate basis `U` and seed basis `B_0`, define its
total squared seed overlap by the basis-invariant projector trace

\[
 w_B(U)=\operatorname{tr}
 (U^TKB_0B_0^TKU).                                \tag{23}
\]

Require `w_B>=0.10`. Match candidates across the prior/final and `h_0/h_1`
constructions by the maximum projector overlap
`m(U,W)=||U^T K W||_F^2`. The maximum must exceed `3.9`; if the largest and
second-largest available overlaps differ by `<=1e-8`, matching is not unique
and the solve is unresolved. For `K`-orthonormal `U,W`, report

\[
 \sin\theta_{\max}=
 \sqrt{\max(0,1-\sigma_{\min}(U^TKW)^2)}.         \tag{24}
\]

If no unique matching candidate exists, the solve is unresolved. If several
rank-four candidates pass every held-out gate, select the one with largest
`w_B`, then smaller full-state `S`-Ritz residual, then lower mean phase.

Repeat the complete construction independently at `h_1`. At the primary
`h_0`, also repeat it with globally sign-reversed seeds and with

\[
 (a',b')=((a+b)/\sqrt2,(-a+b)/\sqrt2)              \tag{25}
\]

applied separately and in order to `(q6,q7)` and `(p6,p7)`. No mutable
endpoint cache is shared between constructions.

## 7. Held-out complete-state qualification

For each eligible candidate `U`, use full chart vectors, not only projected
coordinates. Define

\[
 R=U^TKTU,\qquad R_-=U^TKT^{-1}U.                 \tag{26}
\]

The full-state `S`-Ritz residual is

\[
 r_{\rm Ritz}=
 \sqrt{\frac14\sum_{j=1}^4
 \|Su_j-\mu_j u_j\|_K^2}.                        \tag{27}
\]

Require:

- `r_Ritz<=2e-4`;
- prior/final `sin(theta_max)<=1e-3`, except for the explicitly waived
  invariant terminal-dimension-4 case, and independent `h_0/h_1`
  `sin(theta_max)<=1e-2`;
- sign-reversed and 45-degree-rotated seed constructions preserve the
  projector with `sin(theta_max)<=1e-6`;
- `||(I-UU^TK)TU||_(K,F)/||TU||_(K,F)<=2e-4`, and the reverse analogue;
- candidate `T^-1T` and `TT^-1` residuals `<=1e-4`;
- `||R_- - R^T||_F/max(||R||_F,1e-30)<=2e-4`;
- `||R^TR-I||_F<=2e-4`;
- the four eigenvalues of real `R` have `abs(Im lambda)>=1e-6` and form exactly
  two numerically conjugate pairs. Enumerate the three possible pairings,
  minimize the maximum normalized conjugacy residual
  `|lambda_i-conj(lambda_j)|/max(|lambda_i|,|lambda_j|,1e-30)`, require the
  minimum `<=1e-8`, and require its separation from the second-best pairing
  to exceed `1e-10`. Each selected pair product must be positive and every
  modulus must differ from one by `<=2e-4`;
- phases `Omega_j=atan2(abs(Im lambda_j),Re lambda_j)` satisfy (1), their
  split is `<=1e-4`, and their mean agrees with the `h_1` result to `<=1e-3`;
- with `W` the matched `h_1` basis and `C=U^TKW`, the basis-independent
  intertwining residual
  `||RC-C R_(h_1)||_F/max(||R||_F,1e-30)<=1e-3`;
- `w_B(U)>=0.10`;
- the unnormalized candidate Gram matrix is positive with
  `lambda_min/lambda_max>=1e-6`.

No polar projection, forced orthogonalization of `R`, clipped `K`, or fitted
phase is allowed. Passing `R^T R` means the positive quadratic tangent form
`I_K(c)=c^Tc/2` is numerically preserved to the stated residual; it does not
establish an exact invariant or a symplectic action.

## 8. Controls

The same runner must complete three non-adjudicative method controls:

1. identical zero signed inputs produce identical endpoints and zero centered
   tangent, while the common numerical representative's one-tick excursion
   remains `<=1e-10`;
2. replay the isolated **source-free field operator**, not the coupled map,
   for the exact FTD-0641 mode named in section 5. Require its registered
   256-tick recurrence, forward/reverse recovery, and phase
   `2 asin(C_SPEED sin(pi/17))` to relative residual `<=1e-8`;
3. for every signed preflight endpoint, compare one `nullptr` solve with a
   dedicated cache-population solve and a second identical solve offered that
   cache. Require raw complete-state agreement `<=1e-10` and no cache
   fallback. If the population solve records a Jacobian refresh and leaves a
   valid cache, the reuse solve must record at least one reuse. If it converges
   at iteration zero without a valid cache, both cache counters must remain
   zero. At least one endpoint in the complete control set must populate and
   reuse a cache.

The field control holds matter fixed exactly as FTD-0641 did. It is not a
complete-map mode or matter candidate, and FTD-0642's known coupled phase
shift is not required to vanish.

## 9. Ordered verdict map

Apply this decision order:

1. `L17_COMPLETE_TANGENT_EXECUTION_INVALID`: any provenance, source/options,
   representative, gradient, energy-form, retraction, codec, chart, Gauss,
   endpoint, root-regularity, cache, control, nonfinite, or artifact-schema
   gate fails.
2. `L17_FIRST_DOUBLET_TANGENT_SOLVE_UNRESOLVED`: execution is valid, but
   Krylov breakdown, projected-matrix validity, absence of an isolated
   in-window rank-four cluster, seeded cluster rank greater than four,
   prior/final matching, or `h_0/h_1`/covariance convergence does not resolve a
   candidate.
3. `L17_FIRST_DOUBLET_LOCKED_CANDIDATES_NOT_QUALIFIED`: the solve is valid and
   resolves at least one eligible in-window rank-four cluster, every such
   cluster has been enumerated, and none passes all held-out gates.
4. `L17_FIRST_DOUBLET_POSITIVE_TANGENT_CANDIDATE_CONSTRUCTIVE`: at least one
   enumerated candidate passes every provenance, tangent, spectral, held-out,
   positivity, covariance, and control gate.

Every non-invalid verdict also carries `PRODUCTION_NATIVE_BRIDGE_OPEN`.
`NOT_QUALIFIED` concerns only the locked finite
`K_16(F,B_0)` construction at `L=17`; it is not a statement about the full
cyclic closure, all modes, all algorithms, or volume stability.

## 10. Interpretation and next license

A constructive result establishes an approximate finite-volume complete
matter--field tangent candidate at one numerical selected-map representative.
It does not establish spatial localization, a nonlinear orbit, finite-period
return, a particle pole, a quantum state, a P1--P5-native clock, continuous
within-tick flow, quartic occupancy, a minimum dimensionless `dt`, or the
FTD-0773 edge signature.

Only the constructive verdict licenses a fresh FTD-0775 protocol using the
separately qualified FTD-0708 `L=33` representative to test phase, projector,
and participation-volume stability. Only a constructive two-volume result may
license a finite-amplitude recurrence protocol.

## 11. Required artifacts and independent replay

Produce:

- one focused C++ matrix-free tangent/Krylov runner plus test-only codec, with
  no production change;
- a JSON execution record and primitive CSV/binary artifacts containing both
  finite-difference scales, every adjudicated projected `A_S/A_T/A_Tinv`,
  projected seed matrix, cluster membership, and the full chart vectors and
  images `U,SU,TU,TinvU,TinvTU,TTinvU` for every eligible candidate at the
  prior/final, `h_1`, sign, and 45-degree constructions;
- all K-Gram blocks needed to reconstruct seed overlap, principal angles,
  covariance, harmonic coefficients, and control residuals;
- one independent Python certificate that reconstructs candidate enumeration,
  every matrix/vector gate, and the ordered verdict without trusting the C++
  scalar verdicts;
- one symbolic/algebraic certificate for (4)--(8), filter (17), positivity,
  and the rotation reconstruction;
- an analysis document and synchronized ledger, tracker, EFT index,
  preregistration manifest, meta-index, documentation map, current-state page,
  and changelog entry.

The result corpus must carry hashes of the locked protocol, runner, support
header, and every primitive replay artifact. No FTD-0775 or nonlinear
trajectory may be executed before its own protocol is written and SHA-256
locked.
