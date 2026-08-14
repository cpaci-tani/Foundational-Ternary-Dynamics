# FTD-0933 — Preregistration: C4 companion translation mismatch, dressing metric, and recoil boundary v1

**Identifier:** `FTD-0933`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** abrupt integer relocation of an already formed fixed-center `C4`
source and its gapped companion; exact native mismatch/wake energy; causal local
re-dressing; phase-averaged spectral translation curvature; scalar
common-ledger necessity; no numerical search, fit, engine mutation, continuous
local-translation claim, inertial-mass identification, vector recoil law,
production promotion, `G*`, Born, Bell, context, outcome, or hiding read

## 1. Question

FTD-0932 proved that a compact fixed-center `C4` source has a unique gapped
quasilocal companion and that the native `(J,W)` pair tracks it by retarded
dispersal. It also proved that a rigid primitive source translating one site
per tick generically intersects the native wave band and has no finite-energy
co-moving halo.

The next smaller question is not yet uniform motion. It is one relocation
event:

1. if a formed compact source moves by an integer lattice vector while its
   extended field is not translated instantaneously, what exact mismatch is
   left between the old and new companions;
2. does the ordinary local field update radiate that mismatch and re-form the
   new fixed-center companion;
3. does the resulting translation-space curvature define a physical inertial
   mass or only an energy metric; and
4. what must a future reciprocal source law book before it can claim closed
   recoil?

The registered answer is allowed to close only the abrupt-hop field ledger.
It may not extrapolate to arbitrary slow paths, continuous local translation,
or autonomous matter motion.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `PREREG_C4_SPECTRAL_GAP_RETARDED_COMPANION_AND_TRANSLATING_SOURCE_CONE_BOUNDARY_v1.md` | `0F25E339C6C8AC0BAA122E78FA985BDD4B42FA39098EEC13BF2489AB1240FCFD` |
| `THEOREM_C4_SPECTRAL_GAP_RETARDED_COMPANION_AND_TRANSLATING_SOURCE_CONE_BOUNDARY_v1.md` | `411D292D9A1AEB28285A5DE0E0D6D6545FFDE2D658FF427172275B02BEA68997` |
| `proof_c4_spectral_gap_retarded_companion_translating_source_cone_boundary.py` | `3E5D4D606DE828F63478CA6E5DA3181FDFA5F30DB5208F15B833FBBF2A972049` |
| `THEOREM_NATIVE_FIELD_DISCRETE_ACTION.md` | `2CB4B2D49DED01D9B642416D3C20B89C41F5682FC52896446BEBFB3D1CA8B63C` |
| `THEOREM_NATIVE_HODGE_ENERGY_CONTINUITY.md` | `7849BFF214225723BFA52EA9034C34B22B94D204A78BE1D6DC6F97D065222868` |
| `THEOREM_CONTINUOUS_TRANSLATION_LOCALITY_TRILEMMA.md` | `527BDA49C213C1D58862A8A6254FC153416253EA3159BD7B958F8E43B69630EC` |
| `THEOREM_PASSIVE_DRESSING_DEPINNING_OBSTRUCTION.md` | `238AB6376EBC3FFE0A7324352C764D3BD5224EB89B91D05CF438067C6E6164CD` |
| `THEOREM_DRESSED_BOOST_MOMENTUM_MAP_AND_INERTIAL_IDENTIFIABILITY_BOUNDARY_v1.md` | `378E38227422336BF9956EA6668CA7C09006B3A1D226370577126944654F833C` |
| `THEOREM_BLOCH_QUASIMOMENTUM_LIFT_AND_LOCAL_MOMENTUM_MAP_TRILEMMA_v1.md` | `0C2F0C289C82D45457B5DF330F767C10AD5CA3966FB667B329391C283FD47973` |
| `THEOREM_RECIPROCAL_CARRY_RESERVOIR_AND_LOCAL_IMPULSE_LEDGER_BOUNDARY_v1.md` | `8696F6024CE6ED49120DF6A238F98C8C804AA7B8C441BCA83B5AFDCE111C6048` |

The certificate fails closed on source drift.

## 3. Frozen field, companion, and translation

Use the native driven map

\[
 W_{n+1}=W_n-KJ_n+f_n,
 \qquad
 J_{n+1}=J_n+W_{n+1},                                  \tag{1}
\]

with production-normalized C18 stiffness `K`, compact source arms satisfying

\[
 f_{n+2}=-f_n,                                          \tag{2}
\]

and the FTD-0932 companion

\[
 A=2I-K,
 \qquad
 q_n=-A^{-1}f_n,
 \qquad
 p_n=q_n-q_{n-1}.                                      \tag{3}
\]

The exact gap is

\[
 {2\over9}I\le A\le2I.                                \tag{4}
\]

For `d in Z^3`, define the unitary lattice translation

\[
 (T_dg)(x)=g(x-d).                                     \tag{5}
\]

It commutes with `K` and `A`. At a fixed `C4` phase, abruptly replace the
source sequence by `T_d f_n` but do not perform a nonlocal translation of the
already formed field. The old field at the switching phase is `(q_n,p_n)`;
the new exact companion is `(T_dq_n,T_dp_n)`.

## 4. Frozen translation-mismatch energy

Register the immediate source-centered mismatch

\[
 e_n^{(d)}=(I-T_d)q_n,
 \qquad
 z_n^{(d)}=(I-T_d)p_n.                                 \tag{6}
\]

Its native positive energy is

\[
 \boxed{
 \mathcal D_n(d)=H_{C4}(e_n^{(d)},z_n^{(d)})}
                                                               \tag{7}
\]

with

\[
 H_{C4}(e,z)
 ={1\over2}\|z\|^2+{1\over2}\langle e,Ke\rangle
 -{1\over2}\langle z,Ke\rangle.                      \tag{8}
\]

In the Brillouin representation, freeze

\[
 h_n(k)={1\over2}
 \left(
 |\widehat p_n|^2+\kappa|\widehat q_n|^2
 -\kappa\operatorname{Re}(\overline{\widehat p_n}\widehat q_n)
 \right),                                             \tag{9}
\]

so that

\[
 \boxed{
 \mathcal D_n(d)=
 \int_{\mathbb T^3}2[1-\cos(k\cdot d)]h_n(k)
 {d^3k\over(2\pi)^3}.}                               \tag{10}
\]

The certificate must prove

\[
 \mathcal D_n(0)=0,
 \qquad
 \mathcal D_n(-d)=\mathcal D_n(d),
 \qquad
 0\le\mathcal D_n(d)\le4H_{C4}(q_n,p_n).             \tag{11}
\]

For a nonzero compact source arm and nonzero integer `d`, translation
invariance would force a compact function to repeat along an infinite orbit.
Therefore `(I-T_d)q_n` cannot vanish, and positivity of equation (8) must give

\[
 \boxed{0<\mathcal D_n(d)<\infty.}                    \tag{12}
\]

Equation (12) is a state-space distance between two exactly degenerate
integer-translated companions. It is not the static Peierls curve of
FTD-0581 and is not a universal lower bound for every multi-tick path.

## 5. Frozen retarded re-dressing statement

After the hop, continue the translated fixed-center `C4` source through the
same local map (1). Relative to the new companion, the initial error is
exactly equation (6), and it obeys the free native recursion. Hence

\[
 H_{C4}(e_m,z_m)=\mathcal D_n(d)                       \tag{13}
\]

for every later tick.

The Fourier translation difference contributes

\[
 1-e^{-ik\cdot d}=O(|k|)                              \tag{14}
\]

at the massless point. It cancels the worst `1/sin(omega)=O(1/|k|)` factor
in the exact free-wave representation. Coarea plus the
Riemann--Lebesgue lemma must therefore imply, on the uncontained
three-dimensional scaffold,

\[
 \boxed{
 J_m(x)-T_dq_m(x)\longrightarrow0,
 \qquad
 W_m(x)-T_dp_m(x)\longrightarrow0}                    \tag{15}
\]

at every fixed site after the source is held at its new center. The mismatch
energy is not erased; it leaves every fixed finite region as a radiative
wake. On a finite grounded region it remains recurrent and only its Cesaro
average converges. No uniform decay rate or radiated-power formula is
registered.

## 6. Frozen phase-averaged dressing curvature

Average the exact spectral density over the four `C4` phases:

\[
 \bar h(k)={1\over4}\sum_{r=0}^3 h_r(k).               \tag{16}
\]

For analysis only, choose the principal Brillouin chart and define the
nonlocal spectral interpolation

\[
 \mathcal D(\xi)=
 \int_{[-\pi,\pi)^3}2[1-\cos(k\cdot\xi)]\bar h(k)
 {d^3k\over(2\pi)^3},
 \qquad \xi\in\mathbb R^3.                            \tag{17}
\]

Its Hessian at the identity is

\[
 \boxed{
 G_{ij}^{\rm dress}
 ={\partial^2\mathcal D\over\partial\xi_i\partial\xi_j}(0)
 =2\int k_i k_j\bar h(k){d^3k\over(2\pi)^3}.}         \tag{18}
\]

The certificate must prove that `G_dress` is real, symmetric, positive
semidefinite, and positive definite for nonzero compact source data whose
spectral energy is nonzero on an open set. If the phase-averaged source is
cubic-covariant, it must reduce to

\[
 G_{ij}^{\rm dress}=g_{\rm dress}\delta_{ij},
 \qquad
 g_{\rm dress}={2\over3}\int |k|^2\bar h(k)
 {d^3k\over(2\pi)^3}.                                 \tag{19}
\]

Equations (17)--(19) are a selected spectral chart and therefore a
`[REFERENCE — DRESSING CURVATURE]`. FTD-0554 and FTD-0896 forbid reading the
fractional interpolation as an exact finite-range local translation law.
FTD-0893 forbids calling `G_dress` inertial mass without an independently
defined total momentum linearization.

## 7. Frozen common-ledger and recoil discriminator

Within the already adopted source-centered field-energy interpretation, an
abrupt local hop leaves the field sector with the positive invariant
`mathcal D_n(d)` above its new companion. Therefore a closed reciprocal
source-field model cannot simultaneously assert all three of the following:

1. the source moves by nonzero `d` while its extended field is not translated
   instantaneously;
2. no source, internal, incoming-field, or environmental reservoir changes;
3. the total energy ledger is exact and the translated state has no wake.

At least one item must fail. If the wake is retained and the whole model has
an exact common action, its scalar ledger must contain an opposite balancing
term, schematically

\[
 \Delta E_{\rm source}+
 \Delta E_{\rm internal}+
 \Delta E_{\rm incoming/environment}
 =-\mathcal D_n(d),                                    \tag{20}
\]

with the allocation derived from that future action rather than assigned by
this certificate.

Equation (20) is a necessary scalar accounting condition, not a vector
recoil update. A physical recoil law still requires a source coordinate,
source kinetic/internal action, a local current, a total momentum map, and
its scale. `G_dress` alone does not provide any of those objects.

## 8. Registered outcomes

- **Outcome A — positive translation wake / dressing-metric boundary:**
  equations (6)--(19) pass. Every nonzero abrupt integer relocation of a
  nonzero compact formed companion leaves a finite positive mismatch wake;
  the native local field re-dresses the new fixed center while conserving
  the wake invariant; and the spectral interpolation has a positive dressing
  curvature. Equation (20) is booked only as a necessary future common-ledger
  condition. Physical mass and vector recoil remain open.
- **Outcome B — positive hop wake / no metric promotion:** equations
  (6)--(15) pass, but differentiability, positivity, or covariance of the
  registered spectral curvature fails. The abrupt-hop wake survives without
  a translation metric.
- **Outcome C — no exact hop discriminator:** the translated companion does
  not solve the translated drive, the mismatch is not invariant/positive, or
  local re-dressing fails.
- **Invalid:** source drift, post-lock formula change, numerical search,
  fitted decay, a continuous finite-range translation claim, identification
  of `G_dress` with inertial mass, assignment of vector recoil without a
  momentum map, engine/CMake mutation, production promotion, target/context/
  Born read, or completed-infinity rhetoric.

## 9. Firewalls

No engine source, CMake target, `Voxel` field, toggle, default, production
law, ontology type, paper, physical constant, phenomenological formula, Born
weight, Bell correlation, measurement context, or `G*` cadence is changed.
The proof uses exact algebra, finite grounded witnesses, and Fourier
representation only as mathematical scaffolds.

Even Outcome A does not derive autonomous source motion, a kinetic term,
inertial mass, a vector momentum or recoil law, an exceptional mobile
carrier, slow adiabatic transport, a common matter-field action, attraction,
recovery, collision composition, Lorentz hiding, or framework completeness.
