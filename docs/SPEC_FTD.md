# Foundational Ternary Dynamics: Framework Specification

## A Principled Framework for Universe Simulation

**Document Classification:** Framework overview / readable orientation — **not** a status authority (defers to the canonical hierarchy in [`docs/theory/META_STRUCTURE.md`](theory/META_STRUCTURE.md))

> ** NAVIGATION:** For the single-page status map across all 14 doctrine sectors with canonical tags, see [`docs/theory/01_reference/SPEC_DOCTRINE_LEDGER.md`](theory/01_reference/SPEC_DOCTRINE_LEDGER.md) (FTD-0145 [SYNTHESIS]).
> ** CONSTITUTION:** The standalone-framework constitution — postulates + framework commitments (FC-0/FC-1/FC-2) + calibrations + the computational-EFT catalog + the deviation-prediction spine — is [`docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V1.md`](theory/01_reference/SPEC_FTD_FRAMEWORK_V1.md) (FTD-0254 [SYNTHESIS] + [AXIOM]-class declarations; promotes nothing). On any conflict: LEDGER > constitution > this overview.
>
> ** STALE-TAG NOTICE:** This document's body uses an older version-marker style, and several epistemic tags in the prose below are **STALE** relative to the canonical ledger. The mathematical CONTENT remains mostly accurate, but tags must be read against the LEDGER. The corrected status of the load-bearing items:
>
> - **x₊ = 1/α** is **[STRONGLY MOTIVATED CONJECTURE]**, not a theorem. The polynomial identity is proven; the physical identification is an SMC bridge (1.26 ppm match, structurally unique but not dynamically derived).
> - **`x₋ → N_c = 3`** is **RETIRED** per FTD/FQCR Cleanup Taxonomy v1.4 §5; the `x_-  N_c` identification is dropped (LEDGER FTD-0014 removed in commit `ca7eb61`). `N_c = 3` in FTD comes from independent structural sources — see `DERIV_NC_FROM_TOPOLOGY.md` (four routes) and the Moore Layer Theorem.
> - **sin²θ_W, sin²θ_13, α_s, PMNS angles** are **[PARAMETRIC]** or **[STRUCTURALLY MOTIVATED PARAMETRIC]**.
> - The **"< 0.001 ppt with 7-term expansion"** framing is retracted; the 7-term series is a post-hoc [CONJECTURE] fit to CODATA digits beyond experimental precision.
> - **"Thermodynamic limit"** language throughout is superseded; the framework commits to undefined-boundary ontology, not completed-infinity ℤ³.
> - **Cluster-mass identification (FTD-0110)**: the cluster-efficiency coefficient `k = 1/N_base = 1/4` is **[DERIVED at linear level]** from O_h representation theory (`mult(A_{1g}) = 4` in the 27-block; δ_center A_{1g}-pure; mean A_{1g}-mode energy fraction 1/4). See [`docs/theory/03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`](theory/03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md). The full nonlinear-engine cluster--mass identification across 5 SM particles to ~5% remains [STRONGLY MOTIVATED CONJECTURE] until the linear→nonlinear bridge is proved.
> - **D = 3 status:** [SELECTION — declared] (FTD-0355 permanent verdict) — the |Aut(E)|² arithmetic uniqueness is [THEOREM] but the dimension-forcing is not forced (circularity named). Body occurrences retagged 2026-07-05.
>
> **For current state, read first:** [`docs/WHERE_WE_LEFT_OFF.md`](WHERE_WE_LEFT_OFF.md).
> **Full audit trail:** [`docs/theory/07_assessment/spine_master_quadratic/AUDIT_MASTER_QUADRATIC.md`](theory/07_assessment/AUDIT_MASTER_QUADRATIC.md), [`AUDIT_RATIONAL_FIT_CLAIMS.md`](theory/07_assessment/AUDIT_RATIONAL_FIT_CLAIMS.md), [`AUDIT_INFINITY_REFRAME.md`](theory/07_assessment/AUDIT_INFINITY_REFRAME.md).

> **This document is a readable framework overview, not a status authority.** For the epistemic status of any claim, the canonical sources are [`LEDGER.md`](theory/07_assessment/core_ledgers/LEDGER.md) (per-claim tags) and [`TRACKER_ONTIC_TRUTH.md`](theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md) (truth tiers); the precedence of all status-bearing documents is fixed in [`theory/META_STRUCTURE.md`](theory/META_STRUCTURE.md) § Canonical Hierarchy. Where this document's body and a canonical source disagree, the canonical source wins and this document is the drift to fix. For the C++ engine implementation, see [engine/SPEC_ENGINE.md](../engine/SPEC_ENGINE.md). For the theory document library, see [theory/META_INDEX.md](theory/META_INDEX.md).

**Editorial note:** The publication-ready narrative and epistemic taxonomy live in `dissemination/manuscript/`. This file uses occasional shorthand (e.g., "derived", "resolved", "first principles") to mean "derived within the stated FTD postulates/constraints" or "implemented and validated in simulation," not a claim of empirical establishment.

> **Epistemic Discipline (v5.24):** The following practices are prohibited:
> - **Do NOT** run numerical search scripts looking for near-misses or coincidences
> - **Do NOT** create substitution identities (plugging FTD values into formulas and calling the result a "discovery")
> - **Do NOT** label parametric insertions as "derivations" — if standard physics provides the formula and FTD provides the numbers, that is a **parametric insertion**, not a derivation

> **Major Update (v4.0)**: This version incorporates novel theoretical foundations including an action principle from which update rules are derived, Hilbert space construction from the flux field, and established connections to standard physics (Maxwell, Schrödinger).

> **Major Update (v4.1)**: Full SM gauge group now derived: U(1) × SU(2) × SU(3).

> **Major Update (v5.0 - Foundational Completeness)**: Historical status note; later audits reclassified several physical identifications:
> - **C1 RECLASSIFIED**: x₊ = 1/α is [STRONGLY MOTIVATED CONJECTURE]. The polynomial/root algebra is theorem-level; the physical identification is not dynamically derived.
> - **C2 RETIRED** (v1.4 §5): `x₋ → N_c = 3` is dropped as a physics identification; LEDGER FTD-0014 removed in commit `ca7eb61`. The master quadratic's smaller root `x_- ≈ 3.024` is a mathematical artifact of the polynomial only. Independent topology routes to `N_c = 3` remain separately tagged in the ledger (`DERIV_NC_FROM_TOPOLOGY.md`; Moore Layer Theorem).
> - **A1 [SELECTION — declared] (FTD-0355)**: D = 3 — the arithmetic uniqueness of |Aut(E)|² = 2^D·(D−1)! = 16 is a [THEOREM], but the dimension-forcing (D=3 as the physical spatial dimension) is [SELECTION — declared]: the RHS target 16 = |O_h|/3 presupposes D=3, a circularity named (bounded search). The earlier "uniquely selected / no longer axiom" forcing claim is demoted.
> - **GR COMPLETE**: Einstein equations derived with correct 8πG coefficient
> - **Inflation DERIVED**: n_s = 0.966, r = 0.022 (compatible with Planck)
> - **Baryogenesis DERIVED**: η ~ 10⁻¹⁰ from CP violation + Sakharov conditions
> - **Neutrinos COMPLETE**: Seesaw mechanism with M_R from framework integers
> - **GR / inflation / baryogenesis / neutrino items above — RECLASSIFIED**: these are **[PARAMETRIC]** insertions — standard physics formulas (the Einstein equations, slow-roll inflation, Sakharov/CP, the seesaw) populated with FTD constants — **not derivations from the FTD postulates**. The "COMPLETE" / "DERIVED" headings are retained only as historical version markers. See `docs/theory/07_assessment/core_ledgers/LEDGER.md` for live per-claim tags. Substrate gravity in particular is [CLOSED NEGATIVE] (FTD-0131); FTD strong-field gravity (Schwarzschild / Kerr) is imported from GR, not substrate-derived (FTD-0184).
>
> See [FTD_REFERENCE.md](theory/01_reference/SPEC_FTD_REFERENCE.md) for complete reference and [CHANGELOG.md](../CHANGELOG.md) for version history.

> **Major Update (v5.13-5.16)**: Extended ontological hierarchy (Levels -3 to 12), emergence of i, dimensional emergence (XY vs X+Y), and documentation consolidation. See docs/theory/ for theory documents.

> **Major Update (v5.17 - Epistemic Reclassification)**: Honest accounting of prediction status:
> - **~20 theorem/selection/conjecture chains**: algebraic spine results are theorem-level, while α/N_c physical identifications and many mass/flavor claims carry lower tags in the ledger
> - **~50 Parametric Insertions**: FTD values inserted into standard physics formulas (decay rates, running couplings)
> - **~50+ External Physics**: Standard Model mechanisms used without derivation (Fermi theory, HQET, ChPT)
> - **~3-5 Explicit Inputs**: M_Planck, G_F, Λ_QCD, decay constants
>
> See [EPISTEMIC_AUDIT.md](theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) for honest breakdown.

---

# ABSTRACT FOR PHYSICISTS

We present Foundational Ternary Dynamics (FTD), a discrete computational framework for simulating physical systems from explicit postulates (“first principles” in the sense of the model). The model postulates a 3D cubic lattice where each site ("voxel") occupies one of three states: void (0), positive manifestation (+1), or negative manifestation (-1). Dynamics proceed via local update rules within a 26-connected Moore neighborhood, with information propagating at a maximum of one lattice unit per discrete time step.

The framework introduces a two-layer ontology: a continuous vector "flux" field encoding potential energy density, and discrete state transitions representing particle manifestation. Manifestation occurs probabilistically when flux density exceeds a threshold parameter (KB). Forces emerge from discrete differential operators (gradient, divergence, curl) applied to flux and charge-density fields.

**UPDATE (v4.0)**: We now present FTD as a **principled theoretical framework** with rigorous foundations. The update rules, previously postulated, are now **derived from an action principle** S[s,J]. Quantum mechanics, previously absent, is now **constructed via Hilbert space** H = L²(Lattice, ℂ) from the complexified flux field. The continuum limit is established, recovering **Maxwell electrodynamics and the Schrödinger equation**.

Key achievements within the framework (with several items representing proposed correspondences to known physics) include:

- **Action principle**: S[s,J] yielding the update rules via δS = 0 (within the model)
- **Hilbert space**: quantum-style formalism constructed from flux field complexification
- **Born rule**: several derivations/motivations collected (threshold crossing, conservation, max entropy, Gleason-style)
- **G\***: a proposed derivation chain via elliptic structure + self-consistency + CM selection
- **Bell locality**: Pure lattice gives S≤2 (as expected for local deterministic substrate); QM correlations S>2 understood as aggregate statistical behavior
- **Continuum limit**: a correspondence argument relating FTD to Maxwell + Schrödinger as lattice spacing → 0
- **Spinor structure**: Fermi statistics from frame bundle topology π₁(SO(3)) = ℤ₂
- **Thermodynamics**: Boltzmann-style treatment over microstates within the simulation
- **Gravity sector**: Einstein equations with 8πG coefficient
- **Cosmological inflation**: n_s = 0.966, r = 0.022 from sub-threshold flux dynamics
- **Baryogenesis**: η ~ 10⁻¹⁰ from CP violation + Sakharov conditions
- **phi³ exact EFT**: Cubic potential expansion terminates exactly; λ₃ = 1/3 = 1/D universal; UV-complete in field space
- **One-loop lattice α**: Structure-1 SC scalar-EFT tadpole with a = 2/D gives x₊ = 137.036000 (9.6 ppb from NIST, 99.2% gap closure) within that scheme. GPU audits now mark this as scheme-conditional; a Ward-valid Structure-2 two-U(1) scalar gauge completion does not reproduce the ppb closure.
- **Blind derivation**: 13 steps from "i exists" to α⁻¹ with only two selection principles
- **Honest accounting**: ~35 genuine derivations, ~50 parametric insertions, ~50+ external physics adopted — see [EPISTEMIC_AUDIT.md](theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md)
- **Full SM gauge group**: U(1) × SU(2) × SU(3) derived from FTD axioms
- **D = 3 status** [SELECTION — declared] (FTD-0355): the arithmetic uniqueness of |Aut(E)|² = 2^D·(D−1)! = 16 is a [THEOREM], but the dimension-forcing (D=3 as the physical spatial dimension) is [SELECTION — declared] — the RHS target 16 = |O_h|/3 presupposes D=3, a circularity named (bounded search). Atomic stability + gauge requirements stand as motivation, not a forcing proof.
- **Reference frame structure layer** [CONJECTURE]: an exploratory interpretive reading via the same G* geometry (complex roots y = 2.19 ± 2.86i) — a proposed correspondence, **not a derivation**; see the reference frame structure-vocabulary reframe [REF_REFERENCE_FRAME_VOCABULARY.md](theory/01_reference/REF_REFERENCE_FRAME_VOCABULARY.md)

The framework proposes gauge-sector identifications from its constraint and ternary structures. For Lorentz symmetry, the default production free-flux update has a nonzero isotropic dimension-six preferred-frame term (FTD-0407). A default-off CPU period-two prototype cancels that term while preserving one-Moore-shell-per-tick locality, but changes the leading flux speed to `1/sqrt(13)` (FTD-0408). Retaining the live `1/sqrt(3)` cone is impossible for scalar periods two and three and for the minimal positive-gap Hermitian auxiliary; a stable degree-four target exists but is not yet realized by a local state update (FTD-0409). The Gauss AGM does not presently derive this cone: its reciprocal magnitude belongs to the distinct BCC return period, while a self-dual equal-period bridge conditionally yields a unit cone. Markov's bound excludes that cone for every full-band-stable finite scalar kick period; a bounded unit-cone spectral target exists only outside one-shell locality pending multi-state localization (FTD-0410). A selected two-domain branch instead assigns normalized BCC return structure to time and SC+FCC propagation to physical space; quartic cancellation then uniquely gives `c²=1/7`. Irreducibility excludes an exact finite-state positive-norm linear/unitary realization rational in `M18`, so the default-off engine path is only a stable two-tick IR surrogate and differs at sixth order (FTD-0411). FTD-0412 corrects the Wilson real-time operator: selecting `c_s²=1/7` aligns only its leading massless slope, while incompatible axis/face-diagonal conditions exclude q4 matching for every scalar Wilson `r`. The live matter budget remains `1/3`; gauge and native gravity have no propagating poles. Interacting and common-cone Lorentz recovery remain open.

FTD-0413 escapes the FTD-0412 axial-kinetic hypothesis by adding a
face-diagonal SC+FCC average to the standalone Hermitian Wilson Hamiltonian.
Within the normalized ansatz, complete q4 cancellation uniquely gives
`a=b=1/3` and `r²=4/3`; with selected `c_s²=1/7`, free flux and Wilson matter
share a cone through q4 and all seven doublers remain gapped. The construction
is reverse-solved, default off, and differs from the flux pole at q6. It is not
a live multi-sector Lorentz result.

FTD-0414 replaces exact all-orders matching as the practical acceptance gate
with an empirical infrared envelope. After correcting a sixth-order invariant-
basis naming collision, the selected free flux/matter speeds have conservative
leading spread `Δv/c_s = 11(ka)^4/540 + O((ka)^6)`. Under the documented
`a=ell_P` calibrations its direct free-tree magnitude is tiny, but this is not
yet an experimental pass: carrier identification, finite-q remainder,
interactions, gauge/gravity poles, and radiative mixing remain open.

FTD-0415 sharpens the radiative gate. Exact enumeration under translations,
spatial `O_h`, parity, CPT, and gauge symmetry still permits independent
dimension-four temporal/spatial kinetic ratios; a native spatial vector also
admits a cubic-only marginal gradient invariant. The q4 free-tree suppression
is therefore not technically protected by the declared symmetries. No FTD
loop coefficient has been calculated.

FTD-0416 tests the strongest standard perturbative escape without promoting
it to FTD content. The anisotropic-QED common cone is IR attractive, but the
simultaneous charge flow gives only
`delta_IR/delta_UV=(alpha_IR/alpha_UV)^((N_f+2)/N_f)`. For perturbative
`alpha_UV<=1`, `alpha_IR=1/137`, and integer `N_f>=1`, the strongest possible
suppression is `1/137^3=3.89e-7`. The selected `A=P_T J` bridge is spatially
nonlocal. FTD-0419 later calculates one off-shell full-Brillouin-zone
threshold coefficient; a native local interacting action and physical on-shell
coefficient remain open.

FTD-0417 supplies a deliberately minimal local alternative by adopting an
independent real connection on oriented spacetime links. The noncompact unit-
plaquette action is exactly gauge invariant, nearest-cell local, and full-band
stable at inherited selected `c_A²=1/7`. This is a priced ontology extension,
not a derivation of `A` from `J`. It also abandons the earlier q4-improved
photon pole: against the FTD-0413 matter prototype the leading maximum group-
speed gap is `3(ka)²/28`. At the FTD-0417 stage, the conserved ternary-history
current, compatible discrete-time matter action, and full-zone threshold were
open.

FTD-0418 supplies the compatible one-tick nearest-neighbour Euclidean Wilson
regulator. Its selected action has one massless corner and 15 positive doubler
gaps; exact link differentiation fixes the one-photon vertex, two-photon
seagull, and both Ward identities. This closes the local regulator/vertex
definition stage only. The pair differs at q4. At this stage the ternary
current, real-time unitarity, and physical on-shell threshold were open;
FTD-0421 later closes the frozen additive-current route negative, while the
manifested unitarity and on-shell threshold gates remain open.

FTD-0419 evaluates the complete one-loop terms in one frozen `xi=1` QED_L-like
step scheme. Deterministic sums through `N=320` give
`delta_match/g²=-0.32696906(5)`. The bare common cone is therefore not
automatically one-loop closed and requires a dimension-four anisotropy
counterterm in that scheme. The coefficient is off-shell and scheme-specific;
it is not an on-shell experimental prediction.

FTD-0420–0425 execute the frozen native-first successor. An immutable event
journal leaves selected state and RNG hashes unchanged. Exact rational
stoichiometry over the preregistered additive basis gives transition rank four
and nullity zero once genesis, evaporation, pair production, annihilation, and
weak transmutation are included (FTD-0421). Thus reaction-aware continuity is
valid bookkeeping, but no source-free native gauge charge or link current
exists in that frozen domain. The dependent native charged-pole/common-cone
and dimension-four-flow campaigns were consequently not executed
(FTD-0422/0423). The auxiliary one-calibration counterterm contract is
implemented; existing off-shell data fail universality at the first flavour-
multiplicity threshold, while the gauge-independent on-shell calculation
remains open (FTD-0424). The source-free linear tick is reversible and has an
exact quadratic invariant, but the full production tick is demonstrably
many-to-one; manifested low-energy spectral unitarity remains open (FTD-0425).

FTD-0426 tests the narrower proposal that ternary signs are primitive polarity
while electric charge is the extended closed-surface flux produced after
polarity separation. Production movement transfers one member of an initially
neutral pair between two bodies. With Gauss projection alone, CPU `L=32` and
WSL2 CUDA `L=64` both produce equal/opposite radius-stable flux near `+1/-1`.
This is a `[SELECTED CONSTRAINT REALIZATION]`: the projector explicitly sources
`div(J)` from signed state. Under the frozen live wave/coupling/Gauss profile,
the dressing fails radius independence (37–55% spread) and leaves a Gauss
residual near `0.338`. Therefore primitive polarity can source an effective
reaction-free Gauss charge, but a native conserved `U(1)` and an autonomous
electromagnetic dressing remain absent.

FTD-0427 tests a selected local repair outside the frozen FTD-0420 cycle. Put
the experimental flux on oriented faces, route the signed face current from
actual production movement, and update `J_next=J-current+curl(B)` with one
matched backward-difference complex. The exact identity `div(curl(B))=0`
propagates `div(J)=s` from the initial data without repeated Gauss projection.
All sign/direction arms pass at `L=32,64` under MSVC and WSL2 GCC, with maximum
Gauss residual `1.89e-15`. Status is `[THEOREM — selected discrete complex] +
[MEASURED — production movement compatibility] + [SELECTED MECHANISM]`.
The sidecar neither changes production `Voxel::flux` nor supplies a Coulomb
profile, photon dynamics, forces, reactions, `U(1)`, or full-production charge
conservation; FTD-0421 remains controlling outside the reaction-free movement
sector.

FTD-0428 integrates that selected complex behind the default-off CPU
`matched_gauss_dynamics` toggle. A one-time zero-mean CG solve constructs the
minimum-energy periodic field satisfying `D E=s`; the live isolated tick then
updates staggered face-electric/edge-magnetic fields with the exact
curl/transpose-curl pair and production movement current. It mirrors centered
`E` into `Voxel::flux` without per-tick projection. MSVC/GCC campaigns at
`L=32,64` pass static surface, movement, finite-support wave, and modified
energy gates (worst Gauss residual `9.15e-13`). Status is `[THEOREM — selected
finite-lattice complex/minimum] + [MEASURED — integrated engine compatibility]
+ [SELECTED ENGINE EXTENSION]`. The initialization and Maxwell-like update are
adopted mechanisms; forces, matter poles, reactions, gauge redundancy,
quantization, common-cone recovery, and radiative stability remain open or
absent. FTD-0421 is not superseded.

FTD-0429 corrects the scope of that last sentence. FTD-0421 controls only an
exact microscopic additive generator over its frozen event/basis package; it
does not close dynamical low-energy charge. With both Gauss mechanisms off and
the field initialized to zero, the native production wave/coupling sector
dynamically gives
`(div_c J)_k/s_k = (G_C/C_WAVE^2) sum sin^2(k_a)/M_18(k)`, whose infrared limit
is `3G_C`. Locked `L=32,64` campaigns fit
`Z0=0.256247622955862` versus `3G_C=0.256273629308563` and reject a
zero-intercept model by `Delta BIC=279.14`. Status is `[DERIVED + MEASURED —
RESTRICTED NATIVE LINEAR SECTOR]`: coarse-scale emergent polarity charge is
licensed in that reaction-free sector. Reaction-complete conservation,
microscopic `U(1)`, gauge redundancy, moving-source retardation, photons,
forces, empirical normalization, and a common cone were not established by
FTD-0429; FTD-0430 closes only the moving-source item below.

FTD-0430 closes the moving-source successor at the same restricted native
scope. A sparse neutral pair moves one cell through the actual production
movement phase while an identical pair remains locked. With both Gauss
mechanisms and every reaction/force path off, the difference field appears one
field tick after movement, remains exactly inside the local dependency cone,
and obeys both the native pole and the exact step-residue identity. Fresh
`L=48,96` data give `Z0=0.256268547570661`, within `8.17e-5` relative of the
FTD-0429 intercept, and reject a zero intercept by `Delta BIC=336.88`. Status
is `[DERIVED + MEASURED — REACTION-FREE MOVING-SOURCE SECTOR]`. This is a
retarded transported coarse polarity response, not microscopic `U(1)`,
reaction-sector conservation, photons, radiation, a force law, Lorentz
recovery, or empirical light-speed normalization.

FTD-0431 activates native evaporation and shows why the reaction-aware problem
cannot be reduced to the isolated decay constant. The isolated arm reproduces
`gamma=-log(0.9)` within `0.715%`, while the coupled source/field history fails
the preregistered single-exponential gate: normalized RMS is
`0.04288...0.22273` against a maximum of `0.02`. CPU and CUDA agree, CPU event
journals equal occupancy loss, and the exact field recurrence closes to
`2.41e-13`. Status is `[MEASUREMENT — OUTCOME D: INVALID ANALYSIS MODEL]`.
The generated field raises the local energy entering the evaporation rule and
suppresses later reactions, but the finite-time retention is descriptive only.
No infrared decay intercept, conservation verdict, microscopic `U(1)`, or
common-cone consequence follows. A direct dressed-hazard successor is required.

FTD-0432 supplies that successor at mechanism scope. A read-only observer
reproduces the next production wave write in scratch memory and evaluates each
occupied site's exact pre-RNG evaporation probability. Its conditional
expectations predict the realized Fourier-source and occupancy changes on CPU
and CUDA: maximum/RMS standardized residuals are `2.70/1.04` for source and
`2.53/1.10` for occupancy, inside the locked `6/2.5` gates. Generated field
energy drives the low representative projected hazard from `0.09913` down to
`0.001225`, with oscillatory recovery. Status is `[DERIVED — exact conditional
observer] + [MEASURED — mechanism validation]`. This validates native dynamic
self-dressing as the cause of FTD-0431 curvature; it does not establish exact
or infrared conservation, a pole, `U(1)`, or a common cone.

FTD-0433 then holds the axial fundamental family fixed and varies only
`k=2 pi/L` over `L=12...48`, sampling the hazard at the first field antinode
selected from the exact native pole. All CPU/CUDA and conditional-expectation
gates pass, but the hazard sequence is nonmonotonic. The large-volume tail has
positive effective exponents `0.568,0.524`, while `h48/h12=0.81695` and the
locked fourfold-suppression gate fails. Status is `[MEASURED — OUTCOME C:
UNRESOLVED SCALING]`. Neither a zero infrared hazard nor a finite nonzero floor
is established; conservation, `U(1)`, and common-cone claims remain unchanged.

FTD-0478--0539 complete the frozen-variable face-current mobile-matter cycle.
The ternary site state plus existing subcell remainder has an exact trilinear
polarity shape and analytic straight-segment oriented-face current; partition,
first moment, continuity, locality, and cubic covariance close. This is a
selected coupling representation, not fractional primitive state. A normalized
24-arm implicit bookkeeping transaction is constructive, but its force origin
is not an action result. The derived minimal endpoint-split action closes exact
current, field, and Gauss equations. It nevertheless fails the locked mobile
law gates: all smooth corner roots miss both ordinary and staggered-modified
energy, while every edge root lies on a reflection-plane cusp and needs a
set-valued subgradient selection; edge energy also fails. Status is
`[CONSTRUCTIVE — EXACT FACE-CURRENT OBSERVER] + [CLOSED NEGATIVE — FROZEN
MINIMAL COMMON-ACTION EXACT ENERGY/ALGEBRAIC INVERSION]`. No
`common_action_face_dynamics` toggle, reciprocal dressing scenario, pole/IR
campaign, photon/pilot-wave/wake label, or Lorentz claim is licensed.

FTD-0540 proves the edge cusp is not a coefficient-tuning problem inside that
compact representation. Nearest-cell support, partition, and first moment
uniquely force the cardinal hat, while no locally finite nonnegative `C1`
cardinal weight family can reproduce position. FTD-0541 then constructs the
explicit positive smooth exit: primitive manifestation remains one ternary
site, but its deterministic coupling sidecar is a non-cardinal tensor
quadratic B-spline coat with 27 positive weights at an integer position. The
matched `B2/B1` straight-segment face current satisfies exact continuity and
removes the inactive integer-plane cusp. Status is `[THEOREM —
REPRESENTATION TRILEMMA] + [SELECTION — SMOOTH POSITIVE NON-CARDINAL COAT] +
[THEOREM — EXACT COAT CURRENT]`. FTD-0542 completes that coat into an exact
spacetime current and common gauge interaction; the endpoint-weighted face
sources and temporal coat are derivatives of one functional and obey the
open-worldline gauge endpoint identity. FTD-0543 proves that fixed-step
configuration stationarity alone does not imply endpoint-energy conservation:
an exact quartic witness has energy defect `1/8`, while a simple exact-energy
discrete gradient pays phase-area determinant `9/11`. A special invariant of
the full coat-Maxwell map, a variable lapse, or an explicitly selected
non-variational energy mechanism is still required. FTD-0544 proves the field
half of that ledger exactly: the matched midpoint Maxwell update obeys
`Delta U_field=-<Ebar,K>` while propagating Gauss without projection. The live
unknown was whether the action-derived matter impulse changes the production
dispersion energy by `+<Ebar,K>`. FTD-0545 closes that universal fixed-step
identity negative: its gauge-covariant endpoint map matches the exact uniform
formula below `1.34e-15`, but the energy defect reaches `4.10e-5`. Because the
witness is an external harmonic mode, FTD-0546 performs the required neutral
self-consistent follow-up. Its periodic Gauss/source algebra closes below
`9.98e-14` and exact field work below `2.90e-18`, but pair energy still misses
by `9.68e-9`. The frozen minimal quadratic-coat common action is therefore
closed negative as the FTD-0479 exact-energy mobile law. The mobile toggle and
all downstream campaigns remain unlicensed. FTD-0547 then isolates the defect
more narrowly: under constant collinear force, integrating the production
dispersion through the tick gives a nonuniform accelerated worldline whose
matter work is exact to `2.72e-20`. FTD-0548 derives the corresponding
spacetime current. Total oriented face current is unchanged by temporal
reparameterization, but `K0`, `K1`, and `T` differ from the endpoint-linear
split by up to `1.65e-3` while continuity and gauge identities close below
`2.19e-14`. This reopens only a selected constant-force observer branch. A
general self-consistent force history and atomic matter-field solve remain
open; no production toggle or scenario is licensed. FTD-0549 proves that the
history cannot be reconstructed from endpoint and midpoint kinematics: two
strictly monotone schedules with the same endpoint positions, endpoint and
midpoint velocities, duration, energy endpoints, and total face current have
`K0,K1,T` differences exactly `q d epsilon/30`. The next candidate must
therefore solve internal path stages together with field and matter. This is
an algorithmic requirement, not a new ontological postulate. FTD-0550 then
constructs the compatible quadratic face/edge orbit gather. Its electric
force is exactly adjoint to the FTD-0541 current, including transverse force
on axial paths, and its edge interpolation commutes with the matched curl by
the exact spline derivative identity. The representation layer is therefore
closed constructive. FTD-0551 then solves endpoint momentum, quadratic
current, matter displacement, and matched field update atomically with the
production-dispersion discrete gradient. All 72 neutral periodic arms close
total energy below `9.22e-15` and Gauss below `4.45e-16`. This is a selected
reciprocal integrator, not yet a schedule-resolved spacetime action or a
multi-tick mobile law.
FTD-0552 closes that unmodified multi-tick route negative: a generic subcell
coat accelerates in its own neutral periodic dressing, reaching `0.8465` cell
displacement in 64 ticks even though accumulated energy error is
`2.78e-17`. Integer and half-cell positions are only symmetry-pinned extrema.
FTD-0553 proves that simply replacing the bare polarity by a rigid localized
neutral composite does not remove the mechanism. For any distinct
integer-offset constituent source, the compact quadratic coat gives the exact
subcell energy `U(f)=U(0)+C_i(f^4-f^2/2)`, where `C_i` is a nonnegative spectral
sum and vanishes only for a source extended invariantly along the translation
axis. All 96 locked dipole/quadrupole arms have positive barriers, with minimum
`7.36e-5`. Neutrality removes the zero mode, not the lattice aliases. No mobile
toggle or scenario is licensed. FTD-0554 then proves the general representation
boundary: a continuous homogeneous unitary translation group cannot be both
finite-range and interpolate from the identity to the one-site lattice shift.
The exact band-limited escape removes the Peierls energy but gives every
noninteger density change and continuity current global lattice support plus
signed weights. Therefore a higher-order compact interpolation cannot repair
the point-carrier branch. The remaining native target is an extended or
hopping excitation whose Peierls barrier scales away in the infrared, not an
exactly sliding microscopic point. FTD-0555 makes that target quantitative for
the selected quadratic-coat branch. For any real extended source, the relative
half-cell barrier is exactly
`Pi_i=<((1-cos k_i)/(3+cos k_i))^2>_field-energy`; it is therefore a weighted
ultraviolet-content measure, not merely a geometric-radius measure. A locally
generated tensor-binomial source has `Pi_i~const/m^2`, or `R^-4` in its RMS
width. The locked finite-volume qualification did not pass all controls (9
failures; worst constant miss `11.4%`), so it supplies no numerical or native
carrier promotion. The exact spectral identity and analytic asymptotics are
theorem-grade conditional on the already selected coat; production of a
stable native source with decreasing `Pi_i` remains open.

FTD-0556 closes the alternative integer-hopping kinematics for the isolated
free-flux sector. A scalar finite-range first-order norm-preserving lattice
update is only an integer shift times a phase, but the native
`(flux,wave_vel)` pair is a two-component symplectic fiber with exact pole
`theta(k)=2 asin(C_WAVE sqrt(M(k))/2)` and a positive conserved mode metric.
The explicitly forced CPU tick matches that transfer map through 576 registered
mode-ticks, and an extended analytic packet moves `0.565426` cells in one tick
without fractional primitive sites or support outside the causal shell. This
is field-wave Bloch transport, not manifested matter: a stable localized
`(s,J,W)` carrier and common matter/field cone remain open.

FTD-0557 closes the free-flux localization interpretation exactly on the
infinite lattice. For every complex `lambda`, the eigenstate condition is
supported on the zero set of the nonzero real-analytic determinant
`det(U(k)-lambda I)`, hence on a measure-zero set; the only square-summable
eigenstate is zero. The same argument applied to
`U(k)^T-exp(i phi-i k dot d)I` excludes every nonzero square-summable packet
that returns after finite time as a rigid integer translate. A one-band packet
instead obeys the exact ballistic second-moment law
`Var X_i(t)=Var X_i(0)+2t Cov_sym(X_i,v_i)+t^2 Var(v_i)`. The locked CPU replay
matches three finite-volume packet histories to `1.60e-16`, records a maximum
variance increase of `1.916`, and manifests zero ternary sites. This closes the
isolated free-wave packet as a soliton/particle, not the nonlinear carrier
question. A native bound `(s,J,W)` composite remains open.

**Keywords:** discrete spacetime, cellular automata, emergent physics, computational ontology, universe simulation

---

# PREAMBLE: DOCUMENT STATUS AND INTERPRETATION

## What This Document Is

This document describes a **computational simulation framework** called Foundational Ternary Dynamics (FTD). It specifies:

1. **Ontological postulates**: Axiomatic assumptions defining the simulation's primitive entities
2. **Update rules**: Deterministic algorithms governing state evolution
3. **Interpretive mappings**: Proposed correspondences between simulation entities and physical concepts
4. **Implementation details**: Code architecture and protocols

## What This Document Is Not

This document does **not**:

- Present a confirmed physical theory (empirical testing required)
- Solve quantum gravity

**UPDATE (v4.0)**: This document now **does**:

- Derive update rules from an action principle (not postulated)
- Construct a quantum-style formalism from the flux field (Hilbert space construction within the model)
- Recover known physics in continuum limit (Maxwell, Schrödinger)
- Propose a resolution of the measurement problem within FTD (manifestation = collapse)
- Derive thermodynamics (from microstate counting)
- **Derive the Born rule** from manifestation statistics
- **G\***: provide a proposed derivation chain via elliptic curve selection (within assumptions)
- **Offer candidate predictions** (see Chapter 16)

## Reading Conventions and Epistemic Tags

The following tags indicate the epistemic status of claims throughout this document:

| Tag | Meaning | Reviewer expectation |
|-----|---------|---------------------|
| **[AXIOM]** | Structural postulate (not derivable) | Accept as model definition |
| **[THEOREM]** | Rigorously proven from axioms | Check proof |
| **[SELECTION]** | Argued from consistency, not uniquely proven | Critique argument |
| **[CONJECTURE]** | Proposed interpretation requiring validation | Demand evidence |
| **[IMPOSED]** | Parameter choice or model calibration | Note as input, not output |
| **[EMERGENT]** | Behavior arising from dynamics (not designed in) | Verify in simulation |
| **[OPEN]** | Unresolved question | Research opportunity |

**Legacy prefixes** (for backward compatibility):

| Prefix | Meaning |
|--------|---------|
| **POSTULATE** | = [AXIOM] |
| **RULE** | Algorithmic specification (neutral) |
| **DERIVED** | = [THEOREM] (follows from action principle) |
| **INTERPRETATION** | = [CONJECTURE] (proposed mapping to physics) |
| **OBSERVATION** | Simulation behavior (internal) |
| **CLAIM** | = [CONJECTURE] (assertion requiring validation) |
| **VERIFIED** | = [THEOREM] (mathematically established) |

---

# TABLE OF CONTENTS

## PART A: FOUNDATIONS
1. [Chapter 1: Ontological Postulates](#chapter-1-ontological-postulates)
2. [Chapter 2: State Space and Dynamics](#chapter-2-state-space-and-dynamics)
3. [Chapter 3: The Flux Field](#chapter-3-the-flux-field)
4. [Chapter 4: Manifestation Dynamics](#chapter-4-manifestation-dynamics)
5. [Chapter 5: The Update Cycle](#chapter-5-the-update-cycle)
6. [Chapter 6: Force-Like Behaviors](#chapter-6-force-like-behaviors)
7. [Chapter 7: Model Parameters](#chapter-7-model-parameters)

## PART B: EMERGENT STRUCTURES
8. [Chapter 8: Stable Configurations](#chapter-8-stable-configurations)
9. [Chapter 9: Multi-Scale Organization](#chapter-9-multi-scale-organization)
10. [Chapter 10: Interpretive Mappings](#chapter-10-interpretive-mappings)

## PART C: QUANTUM PHENOMENA
11. [Chapter 11: Approach to Quantum Mechanics](#chapter-11-approach-to-quantum-mechanics)
12. [Chapter 12: Entanglement in the Model](#chapter-12-entanglement-in-the-model)
13. [Chapter 13: The Measurement Question](#chapter-13-the-measurement-question)

## PART D: SCOPE AND LIMITATIONS
14. [Chapter 14: What the Model Does Not Capture](#chapter-14-what-the-model-does-not-capture)
15. [Chapter 15: Open Problems](#chapter-15-open-problems)
16. [Chapter 16: Potential Empirical Contact Points](#chapter-16-potential-empirical-contact-points)

## PART E: IMPLEMENTATION
17. [Chapter 17: Architecture](#chapter-17-architecture)
18. [Chapter 18: Simulation Probes](#chapter-18-simulation-probes)
19. [Chapter 19: Validation Procedures](#chapter-19-validation-procedures)

## PART F: THEORY
20. [Chapter 20: Formal Specification](#chapter-20-formal-specification)
21. [Chapter 21: Assumption Ledger](#chapter-21-assumption-ledger)
22. [Chapter 22: Interpretive Summary](#chapter-22-interpretive-summary)

## PART G: THEORETICAL FOUNDATIONS (v4.0)

> **Note**: Part G theoretical foundations are summarized below. See docs/theory/ for detailed derivations.

- Part I: The Action Principle — supplies a scoped field/source variation; it does not derive all production rules (FTD-0467/0565)
- Part II: Hilbert Space Construction — Quantum mechanics from flux
- Part III: Continuum Limit — Recovery of Maxwell and Schrödinger
- Part IV: Statistical Mechanics — Thermodynamics from microstates
- Part V: Spinor Structure — Fermi statistics from topology
- Part VI: Time's Arrow — Grounded in boundary conditions
- Part VII: Meta-Theoretical Closure — Why this framework

---

# PART A: FOUNDATIONS

---

# Chapter 1: Ontological Postulates

## 1.1 Primitive Entities

The simulation is built on the following axiomatic postulates. These are **not derived**; they define the model.

### POSTULATE 1: Discrete Space (Undefined-Boundary Cubic Lattice)
Space is represented as a 3D cubic lattice **L** that is **uncontained and has no defined boundary**: at every specified position, the six axis-adjacent (and 26-Moore-adjacent) sites exist. This is neither a bounded container nor a claim that the substrate is a completed-infinity totality. Finitude belongs to specified or realized configurations and computations — finite in extent or support as a cloud, star, or galaxy is finite — while the substrate itself has no enclosing wall. Each lattice point is called a "voxel."

*Motivation*: Discreteness enables finite computation. The uncontained, undefined-boundary stance avoids both an enclosing boundary and the further commitment to a completed-infinity lattice (no ℤ³ as a single completed object, no load-bearing L → ∞ limits). Algebraic identities (e.g. G\* via Γ(1/4)) are computable to arbitrary finite precision and remain admissible; appeals to "the whole lattice as one object" are not.

### POSTULATE 2: Discrete Time
Time advances in discrete steps called "ticks." The tick counter t ∈ **N** serves as a global clock.

*Note*: This implies absolute simultaneity within the simulation, which differs from relativistic physics.

### LATTICE  PHYSICAL CALIBRATION (electron-primary default)

Predictions in lattice units convert to physical units through a minimal set of imported dimensional constants. The framework's **default gauge is electron-primary** (FTD-0137 §4.5; full treatment [`FOUND_ELECTRON_PRIMARY_GAUGE.md`](theory/02_foundations/FOUND_ELECTRON_PRIMARY_GAUGE.md)):

> **Import `{ℏ, c, m_e}`.** Two are the universal unit-fixing constants (`ℏ`, `c`; the engine selects `C_SPEED=1/√3` as its canonical raw transport value). The single *beyond-universal* anchor is the **electron mass** (`M_INERTIAL=K_B=m_e≈0.511 MeV/c²`). In raw lattice coordinates FTD-0402 names `E_REST=M_INERTIAL·C_SPEED²=K_B/3` and the separately imposed gravity role `M_GRAVITATIONAL=K_B`. Other dimensional outputs remain conditional on this register and on the epistemic status of their individual relations.

Derived consequences (all conditional on the single imported `m_e`; numerics via the FTD-0015 `m_e/m_P = Kα¹¹` ladder, `K = √(2π)·16/3`):
- One voxel: `a_phys = ℓ_P = ƛ_C·Kα¹¹` — the *predicted* Planck length, `≈ 1.616 × 10⁻³⁵ m` (agrees with the exact ℓ_P to 0.19%). **[DERIVED ~0.19%]**, no longer a bare declaration.
- One tick: `t_phys = ℓ_P / (√3 · c) = t_P/√3 ≈ 3.11 × 10⁻⁴⁴ s` (from the **selected**, linearly stable `c_lat = 1/√3` transport value combined with `c_phys = 2.998 × 10⁸ m/s`: physical `c = c_lat · a_phys/t_phys`). This is not CFL saturation: the exact production-stencil ceiling is `c_lat² ≤ 3/4` at `dt=a=1` (FTD-0407). *Corrected 2026-07-08* from `√3·ℓ_P/c` (√3 was misplaced to the numerator — a factor-3 inconsistency with `c_lat = 1/√3`); see [`DERIV_DIMENSIONAL_GATE.md`](theory/03_derivations/foundational_mechanics/DERIV_DIMENSIONAL_GATE.md).
- Mass unit: the anchor `m_e` itself (`M_INERTIAL=K_B=m_e`; `M_unit = m_e/K_B = 1 MeV/c² ≈ 1.783 × 10⁻³⁰ kg`). Other mass relations retain their individual LEDGER tags; the calibration does not turn selections or parametric formulas into derivations.
- Newton's `G = ℏc·(Kα¹¹)²/m_e²`, i.e. `α_G(e,e) = (m_e/m_P)² ≈ 1.745 × 10⁻⁴⁵` — **[SMC]**, 0.38% vs measured (FTD-0015; the `G_N = 1/100` identification is separately falsified, FTD-0131). Under electron-primary `G` is an **output, not an import**.

**Rationale.** The no-go theorems `THEOREM_A_PHYS_NO_GO` (FTD-0059) and `THEOREM_MU_NO_GO` (FTD-0096) prove no length or mass is derivable from Axiom Zero alone, so *some* dimensional anchor must be imported (grade-0 closure, FTD-0368). Electron-primary spends that single freedom on the **measurable** `m_e` and derives the rest — importing one beyond-universal scale instead of the legacy **Planck-primary** default's two (`a_phys ≡ ℓ_P` redundantly fixed both `ℓ_P ≡ G` and `m_e`, since `m_e/m_P` is a predicted ratio), and it borrows **no** gravitational constant (`G` is an output). **Tradeoff (honest):** under electron-primary `a_phys` and `G` are *derived-conditional* on the `[SMC]`-grade α-ladder rather than exact declarations — the switch buys non-circularity and minimality at the cost of the length/gravity scales inheriting the ladder's `[SMC]`/`[SELECTION]` tags. Per FTD-0137 the lattice spacing is a gauge degree of freedom; the alternative gauges — **legacy Planck-primary** (`a_phys ≡ ℓ_P` declared exactly), cluster-primary (FTD-0130 path-(b)), hadronic-primary (`a_phys ≡ 1` fm), and dimensionless-only — remain valid choices (`FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md` §4). Dimensionless predictions (the falsifiable spine) are *gauge-invariant* under all choices, so **no prediction changes** — this is a re-anchoring, not a re-derivation.

**Calibration discipline.** Every dimensional FTD prediction is **conditional on this calibration**. Specifically:
- Predictions of dimensionless quantities (α, mass ratios, mixing angles, anomalous moments) are calibration-independent and constitute the falsifiable spine of the framework — these survive any reasonable choice of `a_phys` and may be tested directly against experiment.
- Predictions with dimensions (lengths, times, energies in absolute units, individual masses, scattering cross-sections in physical units) are calibration-conditional: they depend on `a_phys` (and on the mass calibration via `K_B = m_e`). When quoted, they should be tagged "conditional on `a_phys ≡ ℓ_P` and `K_B = m_e`."
- Engine benchmarks reporting absolute α values (e.g., the EFT-program 3.6× α plateau) are predicted consequences of the current calibration choice. A different `a_phys` would yield a different absolute value while preserving the dimensionless ratio structure.

This is the same epistemic position as every effective field theory's matching to experiment at one renormalisation point, made explicit. See `docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md` Section 3 (Interpretation D) for the framework-level discussion.

**Reference map.** The full dimensionless  dimensional bridge — the seven algebraic-spine theorems, the four dimensionless physical predictions (α, N_c, m_μ/m_e, m_τ/m_e), the three calibration declarations theorem-enforced by FTD-0059 + FTD-0096 (`a_phys ≡ ℓ_P`, `t_phys = ℓ_P/(√3·c)`, `K_B = m_e`), and one worked dimensional application (m_e in MeV) — is catalogued in `docs/theory/01_reference/SPEC_DIMENSIONAL_MAP.md` (rendered) and `docs/theory/01_reference/dimensional_map.json` (canonical data). When drafting papers or replying to reviewers about whether a claim is dimensionless or calibration-conditional, cite the map entry by id.

### POSTULATE 3: Ternary States (J-primary, s as manifestation; values grounded in Axiom 0)
Each voxel v ∈ **L** carries a flux field J(v, t) ∈ ℝ³ (the dispositional layer) and a discrete state s(v, t) ∈ {i², 0, |i²|} = {−1, 0, +1} (the manifestation layer). **J is primary; s is the action of J via the Genesis threshold rule** (§3.3): when |J(v, t)| crosses the manifestation threshold K_B, the void voxel manifests as s = ±1 according to the sign of the flux divergence; otherwise s = 0. Thus s is not an independent field — it is the discrete observable layer that records when J's local intensity crosses K_B.

**Grounding via Axiom 0 (FTD-0128):** the state space is the real projection of `Z[i]^× ∪ {0}` (the unit group of Gaussian integers extended by the additive identity). Numerical values `{−1, 0, +1}` are algebraic consequences of Axiom 0 ("i exists") via `s = i²` plus the additive identity, not independent settings. The `{s, 0, |s|}` notation makes the polarity-magnitude pairing of the two non-zero states explicit: they are not arbitrary independent integers but a `(value, magnitude)` pair generated by `i²`. The voxel is the invariant lattice substrate; state assignment varies per tick; "infinite potential" content per non-zero state is carried by the continuous flux field `J ∈ ℝ³`. Axiomatic-footprint consequence: three previously-arbitrary numerical settings are now derivable from an existing axiom. See [`docs/theory/02_foundations/FOUND_TERNARY_STATE_FROM_I.md`](theory/02_foundations/FOUND_TERNARY_STATE_FROM_I.md).

| State | Label | Interpretation (Speculative) |
|-------|-------|------------------------------|
| 0 | Void | Unmanifested substrate (|J| < K_B) |
| +1 | Positive | Manifested entity (matter-like; J crossed K_B with positive divergence) |
| -1 | Negative | Manifested entity (antimatter-like; J crossed K_B with negative divergence) |

**Why this is one postulate, not two:** the flux field J is the dispositional content of the void substrate (Postulate 1); the ternary state s is the manifestation projection of J via the Genesis rule. The graded-monism table below makes the dependence explicit. Treating s as primary and J as separate would double-count: J's existence is implicit in the void's dispositional character, and s is definitionally the threshold-projection of J, not an independent observable.

### The Void as Dispositional Substrate

The void (state 0) is not "empty space"—it is a **null substrate awaiting activation**:

- **Present**: It exists as substrate
- **Null**: It has no manifest properties
- **Awaiting**: It can take on properties when conditions are met

**Analogies**:
- *Stem cell*: Not yet differentiated, capable of becoming any cell type, activated by environmental signals
- *Ditto (Pokémon)*: A shapeless entity defined by what it can become, not what it is

**Formal Ontological Status**: The flux field J is **dispositional**—it represents the tendencies of the void substrate:

| Category | FTD Entity | Status |
|----------|------------|--------|
| Substance | Void (s=0) | Foundational substrate |
| Disposition | Flux J | Tendencies of substrate |
| Manifestation | States ±1 | Actualized dispositions |
| Property | Charge, mass | Emergent from manifestation |

This is **graded monism**: one substance (void), with dispositions as modes of that substance, and manifestations as actualized modes.

The flux is not "merely epistemic"—it is ontic but dispositional, not substantial.

### POSTULATE 4: Local Causality
Updates to voxel v at tick t depend only on the state of v and its 26 neighbors (Moore neighborhood) at tick t-1.

*Consequence*: Information propagates at most 1 lattice unit per tick. This defines the simulation's "speed of causality" C = 1.

### POSTULATE 5: Determinism
Given complete initial conditions, the evolution is deterministic. Apparent randomness arises from sensitivity to unobserved sub-lattice structure (epistemic, not ontic).

*Caveat*: The implementation uses pseudo-random number generation for manifestation probability. This is a computational convenience; the model assumes underlying determinism.

## 1.2 Formal Axiomatic Foundation

> *Merged from SPEC_THE_COMPLETE_PROOF_RIGOROUS.md (v2.0)*

The five postulates above define the simulation. Beneath them lies a deeper axiomatic structure from which the ternary ontology can be *motivated* (though the postulates remain the operational foundation):

**Axiom A1 (First Distinction):** 0 = (+1) + (−1)

This encodes three properties simultaneously:
- (A1a) Existence of an identity element 0
- (A1b) Existence of inverse: ∀a ∃(−a) such that a + (−a) = 0
- (A1c) Conservation: distinction preserves totality

**Axiom A2 (Self-Reference Requirement):** ∃σ: Ω→Ω such that σ(Ω) ⊆ Ω

Self-reference — the existence of self-observing systems — cannot be derived from A1 alone; it is a primitive fact about reality.

**Theorem (Complex Necessity):** If self-reference σ satisfies the rotation property (σ⁴ = id, σ² ≠ id), then ℂ is necessary.

*Proof sketch:* σ² is an involution with eigenvalues ±1. If σ² = id, then σ² ≠ id is violated. If σ² = −id (negation), eigenvalues of σ satisfy μ² = −1, which has no solution in ℝ. Therefore extend to ℂ = ℝ[i]/(i² + 1). The rotation property is not assumed — it follows from the lemniscate's 90° crossing angle at the origin (see §7.4.1). ∎

**Epistemic status [SELECTION]:** A1 and A2 provide philosophical grounding for POSTULATES 1-5 but are not required for the simulation. The operational foundation remains the five postulates.

## 1.3 What These Postulates Exclude

- Continuous spacetime
- Superposition of states (voxels are always in exactly one state)
- Non-local influences
- True ontological randomness

These exclusions represent modeling choices, not claims about physical reality.

---

# Chapter 2: State Space and Dynamics

## 2.1 The Voxel Data Structure

Each voxel carries the following data:

```
VOXEL STRUCTURE
───────────────────────────────────────────────
Identity:
  position: (x, y, z) ∈ Z³
  uuid: unique identifier (for tracking)
  partner_uuid: entanglement partner (if any)

Ontological State:
  state: {-1, 0, +1}
  charge: fractional value (model extension)

Dynamical Variables:
  flux: vector in R³
  density: |flux| (derived)
  frequency: scalar (energy proxy)

Mechanical State:
  force_accumulator: vector in R³
  position_remainder: sub-lattice offset
  wave_velocity: vector in R³

Flags:
  is_locked: boolean (bound structure)
  is_active: boolean (phase gate passed)
───────────────────────────────────────────────
```

## 2.2 State Transitions

Allowed transitions between ticks:

```
0 → +1  (Genesis: positive manifestation)
0 → -1  (Genesis: negative manifestation)
+1 → 0  (Evaporation)
-1 → 0  (Evaporation)
+1 → +1 (Persistence)
-1 → -1 (Persistence)
+1 + (-1) → 0 + 0  (Annihilation: both return to void)
```

**Not allowed by POSTULATE 3:**
- State +1 directly becoming -1 (or vice versa)
- Superpositions or fractional states

---

# Chapter 3: The Flux Field

## 3.1 Definition and Role

The "flux" field J(v,t) ∈ **R**³ is a vector field defined on each voxel. It serves as:

1. A carrier of potential energy density (dimensions [E]/[L]²; see §7.1)
2. The determinant of manifestation probability
3. A medium for wave-like propagation
4. **The real-valued precursor to the quantum wave function** (v4.0)

**INTERPRETATION (v4.0)**: The complexified flux $\psi = J_x + iJ_y$ serves as the wave function in H_FTD. The flux field encodes the **dispositional tendencies** of the void substrate—what the substrate *would do* under various conditions.

## 3.2 Flux Propagation

The flux field evolves according to a discrete wave equation:

**RULE (Wave Propagation):**
```
wave_velocity(v,t+1) = wave_velocity(v,t) + c² × ∇²flux(v,t)
flux(v,t+1) = flux(v,t) + wave_velocity(v,t+1)
flux(v,t+1) *= (1 - DAMPING)
```

Where ∇² is the discrete Laplacian over the 6-connected (face-sharing) neighborhood N₆(v):

$$\nabla^2 f(v) = \sum_{u \in N_6(v)} f(u) - 6f(v)$$

This is the standard second-order finite difference approximation. For the 26-connected Moore neighborhood, an alternative weighted form exists but is not used here.

*Note*: The damping term (DECAY_RATE) is phenomenological. It prevents unbounded flux accumulation but is not derived from any principle.

## 3.3 Density

The scalar density field is defined as:
```
density(v) = |flux(v)| = √(Jx² + Jy² + Jz²)
```

Density determines manifestation probability and is used in force calculations.

---

# Chapter 4: Manifestation Dynamics

## 4.1 Genesis (0 → ±1)

When a void voxel's density exceeds the threshold KB, manifestation may occur.

**RULE (Genesis Probability):**
```
p_manifest(v) = clamp(1 - exp(-(density(v) - KB) / KB), 0, 1)
```

*Interpretation*: This exponential form is chosen for smoothness. It is a modeling choice, not a derivation.

**Edge Case (KB = 0)**: If KB = 0, the expression becomes undefined (division by zero). This degenerate case is excluded by construction—KB represents a physical mass scale and must be strictly positive. In practice, KB ≥ m_e c² > 0.

**RULE (Polarity Selection):**
The sign of the manifested state is determined by the sign of ∇·J (flux divergence):
- ∇·J > 0 → state = +1
- ∇·J < 0 → state = -1

*Caveat*: This rule is imposed, not derived. It provides a mechanism for matter/antimatter distinction but lacks physical justification.

## 4.2 Evaporation (±1 → 0)

Manifested voxels return to void when their density falls below KB.

**RULE (Evaporation):**
```
if density(v) < KB and state(v) ≠ 0:
    state(v) → 0
```

## 4.3 Decay

Unbound manifested voxels experience flux decay:

**RULE (Decay):**
```
if not is_locked(v):
    flux(v) *= (1 - γ)  # γ := dimensionless dissipation parameter
```

> **[IMPOSED]** The dissipation parameter γ is kept symbolic in the formal framework. The identification γ = α ≈ 0.00729 is a **parameter choice** made in simulations (see §7.3). This choice is *motivated* by the observation that electromagnetic coupling governs the rate of irreversible transitions, but the identification is **not derived from first principles**—it is imposed. See Assumption Ledger ASSUMP.6.

## 4.4 Annihilation

When +1 and -1 voxels occupy adjacent positions:

**RULE (Annihilation):**
```
Both voxels → state 0
Combined flux redistributed to neighbors as omnidirectional burst
Total flux magnitude conserved
```

---

# Chapter 5: The Update Cycle

## 5.1 The Tick Sequence

Each simulation tick executes the following steps in order:

```
TICK t → t+1
═══════════════════════════════════════════════════════════
PHASE 1: Time Gating
  - Check phase accumulators (relativistic lag proxy)
  - Mark active voxels

PHASE 2: Entropy
  - Apply decay to unlocked manifested voxels

PHASE 3: Existence Transitions
  - Check evaporation conditions
  - Check genesis conditions

PHASE 4: Wave Propagation
  - Update wave velocities
  - Update flux vectors
  - Apply damping

PHASE 5: Field Computation
  - Compute density, gradient, divergence, curl

PHASE 6: Force Accumulation
  - Gravity-like (density gradient)
  - Coulomb-like (charge gradient)
  - Lorentz-like (curl × velocity)
  - Strong-like (Yukawa-form)
  - Weak-like (threshold transmutation)

PHASE 7: Integration
  - Update velocities from forces
  - Accumulate position remainders

PHASE 8: Movement
  - Integer position updates when remainder ≥ 1
  - Enforce speed limit (|v| ≤ C)

PHASE 9: Collisions
  - Empty target: move
  - Same-sign target: elastic collision
  - Opposite-sign target: annihilation

PHASE 10: Transmutation
  - Weak-force polarity flips if stress threshold exceeded

PHASE 11: Binding
  - Detect stable geometric configurations
  - Set is_locked flag

PHASE 12: Increment
  - t ← t + 1
═══════════════════════════════════════════════════════════
```

## 5.2 Order Dependence

**CAUTION**: The update order matters. Different orderings may produce different emergent behaviors. The specified order is a design choice.

**Gate function identification:** The abstract gate functions (Activate_C, etc.) from the theoretical formalism have been identified with concrete engine tick cycle phases (phase_read, phase_write, gauss_project, phase_forces, phase_movement). See [FOUND_DOMAIN_PARTITION_AND_CONTEXT_SELECTION.md](theory/06_reference_frames_and_measurement/FOUND_DOMAIN_PARTITION_AND_CONTEXT_SELECTION.md) §8.

---

# Chapter 6: Force-Like Behaviors

## 6.1 Clarification

The "forces" in this model are **update rules that modify flux vectors**. They are not forces in the Newtonian sense but algorithms that create force-like effects.

## 6.2 Gravity-Like Behavior

**RULE:**

$$\mathbf{F}_{\text{grav}}(v) = G_N \cdot \nabla\bar{\rho}(v)$$

Where $\bar{\rho}$ is the **smoothed density field**, defined as:

$$\bar{\rho}(v) = \frac{1}{|N_6(v)|} \sum_{u \in N_6(v)} \rho(u)$$

with $\rho(u) = |\mathbf{J}(u)|$ being the flux magnitude at each neighbor in the 6-connected neighborhood $N_6$. The constant $G_N$ = GRAVITY_BIAS.

*Interpretation*: This produces attraction toward high-density regions. Whether this reproduces Newtonian gravity or general relativity is **not established**; it is an open question whether inverse-square behavior emerges from 3D geometry.

*Parameter*: GRAVITY_BIAS = 0.01 (phenomenological)

## 6.3 Electromagnetic-Like Behavior

**Electric-like (Coulomb):**

$$\mathbf{F}_{\text{elec}}(v) = -q(v) \cdot \nabla\bar{q}(v)$$

Where $q(v)$ is the charge at voxel $v$ and $\bar{q}(v) = \frac{1}{|N_6(v)|} \sum_{u \in N_6(v)} q(u)$ is the smoothed charge field (analogous to $\bar{\rho}$ in §6.2).

**Magnetic-like (Lorentz):**

$$\mathbf{F}_{\text{mag}}(v) = \beta \cdot (\nabla \times \mathbf{J}) \times \hat{\mathbf{J}}(v)$$

where $\hat{\mathbf{J}}(v) = \mathbf{J}(v)/|\mathbf{J}(v)|$ is the unit vector in the direction of the local flux.

*Interpretation*: Like charges repel, opposite attract. The magnetic component involves the curl of the flux field. In the continuum limit, this recovers Maxwell's equations.

*Parameter*: Coupling strength α = 0.00729 (intentionally matched to fine structure constant)

## 6.4 Strong-Like Behavior

**RULE (Yukawa form):**

$$F_{\text{strong}}(r) = g_s^2 \cdot \frac{\exp(-m_\pi r)}{r^2} \cdot (1 + m_\pi r)$$

where:
- $g_s$ = strong coupling constant (dimensionless; distinct from state-flux coupling $g_c$ in §7.3)
- $m_\pi$ = effective meson mass scale (sets range of interaction)
- $r$ = separation distance in lattice units

*Note*: This functional form is borrowed from Yukawa theory. It is inserted phenomenologically, not derived from the model's primitives. At short range ($r \ll 1/m_\pi$), the force goes as $1/r^2$; at long range, it decays exponentially.

**Singularity at r = 0**: The $1/r^2$ factor diverges as $r \to 0$. In the discrete lattice, $r \geq 1$ (minimum separation is one lattice unit), so the singularity is automatically regularized. For sub-lattice physics, a UV cutoff or regularization scheme would be required—this remains future work.

## 6.5 Weak-Like Behavior

**RULE:**
```
stress(v) = |∇·J| + |∇×J| + |∇ρ|
if stress(v) > WEAK_THRESHOLD:
    polarity may flip (+1  -1 via transmutation)
```

*Interpretation*: High field stress enables "transmutation." This is a rough analog of weak interactions but lacks the gauge structure of electroweak theory.

## 6.6 Limitations of Force Modeling

- Forces are phenomenological (Yukawa, Coulomb forms borrowed from established physics)
-  U(1) gauge symmetry **emerges** from Gauss constraint (verified in simulation; see §14.3)
-  SU(2) gauge symmetry not addressed
-  SU(3) color structure **simulated** via flux axis interpretation (see APPENDIX_A)
- The default free-flux pole is Lorentz-like only at leading order. FTD-0408's
  default-off P4-local period-two prototype removes the dimension-six bare-pole
  term but creates a `1/sqrt(13)` versus `1/sqrt(3)` common-cone mismatch;
  FTD-0409 closes the minimal scalar fixed-cone repairs but leaves general
  period-four/multi-state realization open. FTD-0411 supplies a selected
  BCC-time/SC+FCC-space branch with derived `c²=1/7`, but its irreducible
  cube-root branch excludes exact finite-state positive-norm linear
  localization over `Q(M18)` and the stable local surrogate agrees only through q4.
  FTD-0413 gives a selected SC+FCC-local Wilson matter stencil sharing that
  free cone through q4, but not q6. FTD-0414 turns the residual into an
  explicit `11(ka)^4/540` infrared speed-spread falsifier for that free branch. FTD-0415 proves
  that the declared exact symmetries still permit marginal preferred-frame
  kinetic operators. FTD-0417 freezes a different exactly local photon action
  at the same leading speed, paying an added link type and a larger
  `3(ka)²/28` leading matter/photon mismatch. FTD-0418 freezes its one-tick
  axial Wilson partner and exact one-/two-photon Ward hierarchy, but the pair
  already differs at q4. FTD-0419's complete `xi=1` step-scheme match finds
  `delta_match/g²=-0.32696906(5)`, requiring a dimension-four counterterm in
  that scheme. FTD-0421 closes the frozen native additive-current route
  negative, so its dependent native pole/RG campaigns are not executed.
  Interacting/live multi-sector recovery, on-shell matching, physical
  counterterm universality, and manifested low-energy unitarity remain open
  (FTD-0407–0425; see §14.2)
- Physical on-shell renormalization is not yet calculated; one off-shell step
  scheme and its first fixed-counterterm multiplicity threshold are evaluated
- Coupling constants are parameters, not predictions (but see G* observation in §7.4)

## 6.7 Emergent vs Imposed: The Honest Distinction

FTD distinguishes between features that are **symptomatic** (emergent) and those that are **premeditated** (imposed):

### Emergent (Symptomatic)

Features arising as **symptoms** of the dynamics, without being explicitly coded:

| Feature | How It Emerges |
|---------|----------------|
| Bound structures (triads) | Geometry + stability under decay |
| Interference patterns | Vector addition of flux (linear superposition) |
| Gauge symmetry (U(1)) | Constraint structure (Gauss law) |
| Stable "atoms" | Balance of attractive/repulsive flux gradients |
| Hierarchical organization | Scale-free dynamics of aggregation |
| Conservation laws | Closed system + deterministic update |
| 2 photon polarizations | 3 components - 1 constraint = 2 physical modes |

These are genuine emergent properties—they were not designed in but follow from the rules.

### Derived-and-identified constants (per-claim tags; retagged 2026-07-12 to canon)

The following are not free input parameters — but their honest status splits per claim (LEDGER wins):

| Feature | Value | Status | Derivation |
|---------|-------|--------|------------|
| Fine structure α | 1/137.036 | **[THEOREM] algebra + [SMC] identification** | Master quadratic root x₊ is pure algebra from G* [THEOREM]; the identification x₊ = 1/α (1.26 ppm) is [STRONGLY MOTIVATED CONJECTURE] (FTD-0013) |
| Electron mass m_e | 0.511 MeV | **[SMC]** | m_e = m_P √(2π) (16/3) α¹¹ (0.19%) — FTD-0015; exponent n=11 is [SELECTION] |
| Higgs VEV v | 246 GeV | **[SELECTION]** | v = m_P √(2π) α⁸ (0.05%) — HIGGS-4 |

### Still Imposed (Structural) **[IMPOSED]**

| Feature | Why Imposed |
|---------|-------------|
| Force functional forms (1/r², Yukawa) | Geometric necessity in 3D |
| 26-neighbor connectivity | Moore neighborhood choice |
| Ternary states {-1, 0, +1} | Minimal non-trivial structure |
| Dissipation rate γ = α | Parameter identification (ASSUMP.6) |
| 1 voxel = Planck length | Scale identification (see §7.1) |

### Derivation Status Summary

> **Research Program**:  **COMPLETED (within stated assumptions)** — Coupling constants (α = 1/137.036, N_c ≈ 3) and the electron mass are obtained from proposed relations within the framework. The physical identifications are ledgered as conjectural or selection-level where appropriate. See [docs/theory/07_assessment/core_ledgers/LEDGER.md](theory/07_assessment/core_ledgers/LEDGER.md).

> **Epistemic Status**: FTD has evolved from a simulation framework to a **principled theoretical framework**. The CM/master-quadratic arithmetic is theorem-level, but the identification of x₊ with 1/α remains a strongly motivated conjecture and the ppb one-loop correction is scheme-conditional after the Structure-2 audit. This does not constitute independent physical confirmation.

---

# Chapter 7: Model Parameters

## 7.1 Natural Units and Dimensional Analysis

FTD uses **natural units** where fundamental constants are set to unity. This section specifies the dimensional structure.

### Base Units **[IMPOSED: Scale Identification]**

> **Epistemic Status**: The identification of 1 voxel = Planck length is an **interpretive choice**, not a derivation. It connects the discrete lattice to physical scales but is not derived from the axioms. This is required for numerical contact with experiment but constitutes a **model calibration**, not an output.

| Unit | Symbol | FTD Value | Physical Interpretation |
|------|--------|-----------|------------------------|
| Length | ℓ | 1 voxel | Planck length ℓ_P ≈ 1.6×10⁻³⁵ m **[IMPOSED]** |
| Time | τ | 1 tick | Planck time t_P ≈ 5.4×10⁻⁴⁴ s **[IMPOSED]** |
| Mass-Energy | E | 1 (flux unit) | Planck energy E_P ≈ 1.2×10¹⁹ GeV **[IMPOSED]** |

### Derived Dimensions

| Quantity | Symbol | Dimensions | Notes |
|----------|--------|------------|-------|
| Speed | C | [L]/[T] = 1 | Speed of causality |
| Flux | J | [E]/[L]² | Energy current density |
| Density | ρ = \|J\| | [E]/[L]² | Flux magnitude |
| Divergence | ∇·J | [E]/[L]³ | Source density |
| Coupling g | g | [E]^(1/2)[L]^(3/2) | State-flux coupling (see §13.2) |
| Decay rate | γ | [T]⁻¹ = 1/tick | Dimensionless in natural units |

### Lagrangian Density Dimensions

For the action $S = \sum_t \sum_v \mathcal{L}\,V_{\rm cell}$ to be dimensionless (action in units of ℏ = 1, with the unit tick implicit):

$$[\mathcal{L}] = [E]/[L]^3 = \text{energy density}$$

This requires:
- $[\frac{1}{2}|\partial_t J|^2] = [E]^2/[L]^4 \cdot [T]^2 = [E]/[L]^3$ ✓ (using [E][T]/[L]² = 1)
- $[g \cdot s \cdot \nabla \cdot J] = [g] \cdot 1 \cdot [E]/[L]^3$, so $[g] = 1$ (dimensionless)

**Conclusion**: In FTD natural units, the coupling constant g in the Lagrangian is **dimensionless**.

**FTD-0404 spatial-measure reconciliation:** the current raw lattice fixes the voxel edge `a_lat=1`, so `A_face=a_lat²=1` and `V_cell=a_lat³=1`. Local field densities remain quadratic (`rho_field=½|J|²`, `rho_wave=½|wave_vel|²`); volume-integrated diagnostic channels use `E=Σ rho_i V_cell`. The cube belongs to the 3D measure, not to the vector norm. Point-particle energy/momentum and the local latency-Poisson density source are not multiplied by `V_cell`. This is exact current-engine bookkeeping, not a derivation of D=3, a stress–energy tensor, or a mass scale.

## 7.2 Structural Constants

These define the model's fundamental scales:

| Parameter | Value | Dimensions | Role | Status |
|-----------|-------|------------|------|--------|
| `C_MOORE` | 1.0 | [L]/[T] | Topological update-support bound (L∞/Moore: one site per axis per tick), not the particle/wave speed | Axiomatic (P1/P4 geometry) |
| `C_SPEED` | $1/\sqrt3$ | [L]/[T] | Selected wave and particle transport speed in raw nodes/tick; FTD-0402 uses this value in $\beta^2=|u|^2/C_{\rm SPEED}^2$. It is stable but does not saturate the exact production-stencil CFL ceiling $C^2\le 3/4$. | **[SELECTED]**; implemented value, not a Lorentz-recovery or CFL-uniqueness theorem (FTD-0407) |
| H | 1.0 | [L] | Lattice spacing (the **edge**) — calibrated `a_phys ≡ ℓ_P` (IMP-K1; [DERIVED ~0.19%] under electron-primary). One tick = t_P/√3, so that c·t_P = ℓ_P holds exactly (FTD-0385 naming theorem) |  Axiomatic unit; Planck naming per calibration register |
| `VOXEL_VOLUME` | $H^3=1$ | [L]³ | Explicit cubic measure multiplying volume-density sums | **[THEOREM — current engine representation, FTD-0404]** |
| `K_B` / `M_INERTIAL` | 0.511 | mass unit | Inertial-mass calibration | **[IMPOSED]** (electron-primary anchor; historical $n=11$ relation remains [SELECTION]) |
| `E_REST` | $K_B/3$ | energy unit | Particle rest energy in raw coordinates, $M_{\rm INERTIAL}C_{\rm SPEED}^2$ | **[IMPOSED role map]** |
| `M_GRAVITATIONAL` | 0.511 | source unit | Current latency-Poisson gravity charge, separately named | **[IMPOSED]**; equality to `M_INERTIAL` not derived |
| `K_MANIFEST` | 0.5054620197 | field-energy scale | Manifestation/evaporation kinetics, distinct from mass calibration | **[SELECTION — ADOPTED, FTD-0388]** |

**FTD-0402/0403 status:** the selected raw-lattice causal and mass-role map is implemented and passes exact/targeted CPU, GPU, golden, WASM, and web gates. FTD-0402 retains its frozen `PARTIAL` verdict because its aggregate G9 was not completed. The independently locked FTD-0403 v2 targeted dependency closure passes the exact changed surface and closes `§12-cnorm` without running unrelated CTests. That closure made a separately locked NCEMC feasibility audit admissible; FTD-0405 subsequently found the current route blocked by phase-split work nonconservation and an unselected energy zero/local stress distribution. No covariance, equivalence-principle, confinement-energy, or mass-scale theorem is added.

**FTD-0404 status:** the cubic unit-cell measure is explicit and numerically neutral. Energy/action density sums carry `V_cell=H³=1`; local norms remain quadratic; WASM indices are append-only. This closes only the density-versus-integral representation ambiguity; its then-open NCEMC successor status is superseded by the scoped FTD-0406 entry below.

**FTD-0405 status:** NCEMC feasibility returns `DOUBLE-OBSTRUCTION` for the present direct RenderBridge colour force. A radial potential family exists and isolated two-body particle momentum closes, but the actual tick has nonzero work residual and the additive strong-energy zero/local stress distribution required by gravity are not selected. No confinement mass or common stress–energy claim is licensed. A different local strong-field architecture requires a fresh lock and explicit owner authorization.

**FTD-0406 status:** that authorization was subsequently supplied and is recorded as selected/imposed architecture, not as a derivation. The default-off CPU `strong_stress_energy` path adopts `U_ij(r)=-c_f∫_1^r g(s)ds`, projects collision-free relative momenta so the same `K+U` is conserved, deposits local midpoint-CIC string `T00` plus central stress, and sources CPU latency with `T00/C_SPEED²`. Exact 21/21, native 35/35 twice, targeted neighbors 10/10, and goldens 7/7 pass. This closes NCEMC-1–4 only on the isolated flat collision-free CPU domain. GPU, topology changes, moving latency, mixed forces and NCEMC-5 remain open; no mass scale or equivalence principle is derived.

## 7.3 Coupling Parameters

| Parameter | Value | Dimensions | Role | Status |
|-----------|-------|------------|------|--------|
| α (ALPHA) | 0.00729 | dimensionless | Fine structure constant | **STRONGLY MOTIVATED CONJECTURE** from x₊ (§7.4); ppb corrections scheme-conditional |
| g_c | ~α^(1/2) | dimensionless | State-flux coupling | Conditional on α identification |
| G_N (GRAVITY_BIAS) | 0.01 | dimensionless | Engine gravity parameter | **[CLOSED NEGATIVE as physical-G_N identification]** (FTD-0131 — the 1/(b₃+N_c)² reading is off by 10²⁰–10⁴³; engine parameter only; the substrate route instead gives α_G(e,e) = (m_e/m_P)² [SMC], next row) |
| α_G | 5.91×10⁻³⁹ | dimensionless | Gravitational hierarchy | [STRUCTURALLY MOTIVATED PARAMETRIC] (2π(16/3)²(N_eff+3/7)²α²⁰; 0.06% is spelling-dependent, canonical-mass spelling −0.33% — corrected 2026-07-01, FTD-0348) |
| γ (DECAY_RATE) | 0.00729 = α | [T]⁻¹ | Dissipation rate |  **[IMPOSED]** (see §4.3, ASSUMP.6) |
| φ (PHI) | 1.618... | dimensionless | Golden ratio |  Mathematical constant |

## 7.3.1 The Electron Mass Derivation

The absolute mass scale is now derived:

$$m_e = m_P \cdot \sqrt{2\pi} \cdot \frac{N_{\text{base}}^2}{N_c} \cdot \alpha^{11} = m_P \cdot \sqrt{2\pi} \cdot \frac{16}{3} \cdot \alpha^{11}$$

| Component | Value | Origin |
|-----------|-------|--------|
| m_P | 1.22 × 10¹⁹ GeV | Planck mass (lattice spacing) |
| √(2π) | 2.507 | Action principle normalization |
| 16/3 | 5.333 | N_base²/N_c = 4²/3 |
| α¹¹ | 4.2 × 10⁻²⁴ | α⁸ (hierarchy) × α³ (Yukawa) |

**Result**: Predicted 0.5096 MeV vs experimental 0.5110 MeV (**0.19% error**)

## 7.4 The Lemniscatic Derivation

The lemniscatic constant G* has theorem-level mathematical provenance through the Gamma/FQCR/quarter-conjugacy chain. This supports the algebraic spine; it does **not** by itself derive the physical fine-structure constant from FTD axioms.

**The lemniscatic constant** $G^* = \frac{\sqrt{2} \cdot \Gamma(1/4)^2}{2\pi} \approx 2.9587$ emerges from:

1. **√2 factor**: Critical coupling from Gauss constraint geometry
2. **Γ(1/4)² factor**: Lattice regularization → elliptic integral K(1/√2)
3. **Coefficient 16**: Physical degrees of freedom on 2×2×2 minimal lattice (24 - 7 - 1 = 16)

**BCC Watson integral:** The identity W₃ = G*²/(2π) has been confirmed as the **BCC** Watson integral specifically (not SC or FCC). The BCC eigenvalue's multiplicative cosine product is what produces Γ(1/4)⁴/(4π³), connecting G* directly to BCC lattice geometry. See [DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md](theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md).

The master quadratic (the degree-2 form is **[SELECTION]-backed** — motivated by self-referential closure of the ternary constraint and by the degree-2 CM field $\mathbb{Q}(i)$, see [DERIV_QUADRATIC_NECESSITY.md](theory/03_derivations/DERIV_QUADRATIC_NECESSITY.md); the operator assembly is **not forced** — 0/4 native routes, FTD-0242 route-invariant boundary + FTD-0244 K-BIND theorem-negative):

$$x^2 - 16(G^*)^2 x + 16(G^*)^3 = 0$$

produces two roots:

| Root | Value | Interpretation | Accuracy |
|------|-------|----------------|----------|
| x₊ | 137.036 | 1/α (fine structure constant) | 1.26 ppm |
| x₋ | 3.024 | mathematical artifact of P(x); no physics identification | n/a |

**Status**: The polynomial and roots are **[THEOREM]** algebraically; the physical identification `x₊  1/α` is **[STRONGLY MOTIVATED CONJECTURE]** in the master ledger (FTD-0013). The `x₋  N_c` identification is **RETIRED** per FTD/FQCR Cleanup Taxonomy v1.4 §5 (LEDGER FTD-0014 removed in commit `ca7eb61`); `N_c = 3` in FTD is independently sourced (Moore Layer Theorem; `DERIV_NC_FROM_TOPOLOGY.md`). The one-loop ppb correction is scheme-conditional after the Structure-2 audit. See [docs/theory/07_assessment/core_ledgers/LEDGER.md](theory/07_assessment/core_ledgers/LEDGER.md) and [docs/theory/10_eft_program/archive/closed_negative/AUDIT_STRUCTURE2_WARD_VALIDATION.md](theory/10_eft_program/archive/closed_negative/AUDIT_STRUCTURE2_WARD_VALIDATION.md).

### 7.4.1 Lemniscate Selection: Four Algebraic Criteria

> *Merged from SPEC_THE_COMPLETE_PROOF_RIGOROUS.md*

The Bernoulli lemniscate (r² = cos 2θ) is uniquely selected among elliptic curves by four independent criteria:

1. **Algebraic Period**: K(1/2) = Γ(1/4)²/(4√(2π)) — the complete elliptic integral relates algebraically to the gamma function (unique among K(m) for generic m)
2. **Complex Multiplication**: The elliptic curve y² = x³ − x has j-invariant j = 1728 and CM by ℤ[i], one of exactly 13 imaginary quadratic fields with class number 1
3. **Heegner Number**: ℚ(i) corresponds to discriminant d = −4, a Heegner number
4. **Algebraic Doubling**: The lemniscate sine sl(u) satisfies a purely algebraic addition formula, unique among curves up to isomorphism

### 7.4.2 The 90° Crossing Angle Proof

> *Merged from SPEC_THE_COMPLETE_PROOF_RIGOROUS.md — closes Gap 1*

The lemniscate r² = cos 2θ crosses itself at the origin. Two branches approach along y = x (45°) and y = −x (135°). The crossing angle is exactly 90°. This forces the rotation period to be 4 (since 4 × 90° = 360°), giving σ⁴ = id and thus i² = −1. The 90° angle is a **theorem about the curve**, not an assumption — it resolves the rotation property required for complex number necessity (§1.2).

### 7.4.3 Coefficient 16: Four Independent Derivations

> *Merged from SPEC_THE_COMPLETE_PROOF_RIGOROUS.md — closes Gap 2*

The coefficient 16 in the master quadratic is over-derived via four convergent routes:

| Route | Derivation | Value |
|-------|-----------|-------|
| Lattice DoF | 24 components − 7 Gauss constraints − 1 gauge freedom on 2×2×2 cube | 16 |
| Lucas square | L₃² = 4² = 16; L₃ = 4 is the only non-trivial Lucas square | 16 |
| Base squared | N_base² = 4² (dimensional closure) | 16 |
| Precision formula | Historical precision-fit evidence; not a derivation of coefficient 16 or α (see §16.2.1) | 16 |

The convergent routes provide structural evidence for coefficient 16. They do not promote the physical identification `x₊  1/α` above its current LEDGER tag (FTD-0013, [STRONGLY MOTIVATED CONJECTURE]). The `x₋  N_c` identification is retired (v1.4 §5).

## 7.5 Derivation Summary

| Claim | Status |
|-------|--------|
| G* produces an x₊ root matching 1/α to 1.26 ppm | [STRONGLY MOTIVATED CONJECTURE] for the physical identification |
| G* mathematical provenance | [THEOREM]-level within the algebraic spine; not an α derivation by itself |
| Elliptic fibration from FTD |  Proven (simulations/elliptic_fibration_proof.py) |
| CM selection (j=1728) |  Proven (simulations/cm_selection_proof.py) |
| Coefficient 16 from lattice | Structurally motivated; exact status tracked in the algebraic-spine/ledger docs |
| √2 from Gauss constraint |  Derived (simulations/critical_coupling_selection.py) |

For related theoretical context, see also [LEMNISCATE_HIERARCHY_WHITEPAPER.md](theory/04_coupling/DERIV_LEMNISCATE_HIERARCHY_WHITEPAPER.md).

## 7.6 Topological Latency Interpretation of Mass

> *Merged from SPEC_FTD_FORMAL.md — alternative formulation*

Mass can be reinterpreted as **Topological Latency** (L) — the fractional geometric remainder when the temporal operator T̂_G* projects a continuous phase onto a discrete lattice node:

$$L = |\Psi_{\text{raw}} - N_{\text{discrete}}|$$

| Generation | Spatial Dimensionality | Mean Latency | Mass Scale |
|------------|----------------------|--------------|------------|
| 1 (e, u, d) | 1D standard | 0.25 | 0.511 MeV baseline |
| 2 (μ, c, s) | 2D cross-section | 0.50 | Exponential scaling |
| 3 (τ, t, b) | 3D saturated | 0.75 | Highest mass |

**Charge as projection parity [CONJECTURE]:** Electromagnetic charge is the directional parity of the projection: Q = sgn(Ψ_raw − N_discrete). Negative charge = lower-bound truncation; positive charge = upper-bound supplementation. EM attraction is mathematical optimization toward perfect integer state (L = 0).

**Strong force as spatial multiplexing [CONJECTURE]:** Localization of three quarks within a single spatial node would exceed the L < 1.0 operational limit. The strong force is the structural constraint forcing each quark onto an orthogonal axis (historically denoted as "color charge"), giving N_c = 3 from D = 3.

**Epistemic status [CONJECTURE]:** This reinterpretation provides geometric intuition for mass generation, charge, and color but has not been demonstrated to produce quantitative predictions beyond what the standard formulation already achieves.

---

# PART B: EMERGENT STRUCTURES

---

# Chapter 8: Stable Configurations

## 8.1 Triads (Proposed Nucleon Analogs)

Within the simulation, certain geometric configurations exhibit enhanced stability.

**OBSERVATION (Internal)**: Three same-sign manifested voxels arranged in an approximate equilateral triangle (pairwise distance ≈ √2 lattice units) tend to persist longer than isolated voxels.

**RULE (Binding):**
```
if triad_detected(v1, v2, v3):
    set is_locked = True for all three
    suppress decay
    binding_energy ≈ KB × PHI
```

**INTERPRETATION (Speculative)**: These "triads" may serve as analogs of nucleons (protons, neutrons). This mapping is proposed, not proven.

**CAUTION**: The stability of triads is a consequence of the update rules we designed. It is not an independent derivation of nuclear physics.

## 8.2 Shell Structures (Proposed Electron Analogs)

Negative-state voxels may form quasi-stable orbits around positive clusters at characteristic radii.

**OBSERVATION (Internal)**: Discrete "shells" appear at radii approximately proportional to n² for integer n.

**INTERPRETATION (Speculative)**: These may be analogs of electron orbitals. The hydrogen-like n² scaling is suggestive but requires rigorous analysis.

## 8.3 Larger Structures

The simulation exhibits hierarchical organization:
- Triads → "nuclei"
- Nuclei + shells → "atoms"
- Atoms → larger aggregates

**STATUS**: These are simulation observations. Their correspondence to physical atoms, molecules, etc. is interpretive.

---

# Chapter 9: Multi-Scale Organization

## 9.1 Observed Behaviors

At sufficient scale and evolution time, the simulation exhibits:

1. **Clumping**: Gravity-like attraction causes density inhomogeneities
2. **Phase-like transitions**: Different flux regimes produce different ordering
3. **Hierarchical structure**: Small structures aggregate into larger ones

## 9.2 Interpretive Mappings to Physics

The document proposes correspondences between simulation behaviors and physical phenomena across scales:

| Simulation Entity | Proposed Physical Analog |
|-------------------|-------------------------|
| Triad | Nucleon |
| Shell electron | Orbital electron |
| Triad cluster | Atomic nucleus |
| Triad + shells | Atom |
| Bound atom groups | Molecules |
| Large aggregates | Planets, stars |

**STATUS**: These are interpretive proposals. Rigorous validation would require demonstrating quantitative agreement with physical data, which has not been performed.

---

# Chapter 10: Interpretive Mappings

## 10.1 Particle Correspondences

The model assigns simulation configurations to Standard Model particles:

| Configuration | Proposed Particle | Notes |
|---------------|-------------------|-------|
| Single +1, charge +2/3 | Up quark | Fractional charge is model extension |
| Single +1, charge -1/3 | Down quark | |
| Single -1, charge -1 | Electron | |
| State 0, charge 0 | Neutrino | Distinct from void |
| Flux wave | Photon | State-0 propagating disturbance |
| Triad (uud) | Proton | |
| Triad (udd) | Neutron | |

**CRITICAL CAVEAT**: These correspondences are **imposed by the model's design**, not derived. The simulation does not predict the particle spectrum; it is engineered to accommodate it.

## 10.2 On "Emergence" Claims

When this document states that something "emerges," this should be interpreted carefully:

- **Weak emergence**: Complex patterns arise from simple rules (well-established)
- **Strong emergence**: Novel physics unpredictable from rules (not claimed)

**v4.0 Update**: We claim weak emergence of structural patterns. The v4.0 theoretical foundations demonstrate that:

- **Maxwell electrodynamics** emerges in the continuum limit (§3.4)
- **Schrödinger equation** emerges in the non-relativistic limit (§3.5)
- **Hilbert space structure** is constructed from the complexified flux field (Part II)

We do **not** claim that the full Standard Model (non-Abelian gauge structure, Higgs mechanism, flavor physics) is derivable from our postulates—these remain open questions (see §22.5).

---

# PART C: QUANTUM PHENOMENA

---

# Chapter 11: Approach to Quantum Mechanics

## 11.1 The Model's Stance

The model adopts a **definite-state ontology**: every voxel is always in exactly one of three states. There are no superpositions at the voxel level.

**FTD does not attempt to recover quantum mechanics.** QM is understood as the **observation of aggregate behaviors** — statistical regularities that emerge when ensembles of substrate-level events are measured. The relationship is analogous to thermodynamics emerging from statistical mechanics: the aggregate description (QM) is valid and useful, but the substrate (FTD) operates by different rules.

This is not a departure from QM — it is a proposed *explanation* of QM. The substrate is deterministic and local; quantum phenomena are what that substrate looks like from the perspective of embedded observers measuring statistical ensembles.

## 11.2 How Quantum-Like Behavior Could Arise

The model proposes that quantum-like phenomena may emerge from:

1. **Epistemic uncertainty**: Sub-lattice structure we cannot observe
2. **Flux interference**: Vector addition of flux fields producing interference patterns
3. **Statistical ensembles**: Averaging over many similar configurations

**STATUS**: These proposals are speculative. They have not been tested against:
- Double-slit experiment quantitative predictions
- Bell inequality bounds
- Quantum computing operations
- Entanglement swapping

## 11.3 What Is and Is Not Claimed

**UPDATE (v4.0)**: The theoretical foundations now establish:
-  **Hilbert space construction**: H_FTD = L²(Lattice, ℂ) from complexified flux
-  **Born rule derivation**: P(v) = |ψ(v)|²/||ψ||² follows from manifestation statistics
-  **Bell locality**: Pure lattice dynamics give S≤2, consistent with local deterministic axioms. QM's S>2 is an aggregate statistical property, not a substrate requirement
-  **Measurement resolution**: Collapse = manifestation triggered by observer coupling

**What remains open**:
-  Full QFT correspondence (beyond U(1) sector)
-  Experimental validation (simulations are internal, not lab tests)
-  Characterization of substrate-to-aggregate transition (how QM statistics emerge from lattice)
-  Demonstration that ensemble averaging over lattice states yields quantum correlations

See docs/theory/ for derivations.

---

# Chapter 12: Entanglement in the Model

## 12.1 Implementation

Entanglement in this model is implemented as **shared origin tracking**:

```
RULE (Pair Production):
  When two voxels manifest simultaneously from high-density void:
    - Assign complementary states (+1 and -1)
    - Assign shared partner_uuid
    - Correlated properties from shared origin
```

## 12.2 What This Achieves

- Particles from pair production carry correlated properties
- The correlation is established at creation, not measurement
- No faster-than-light signaling occurs (correlations are pre-established)

## 12.3 Critical Evaluation

**v5.24 UPDATE**: The shared-origin mechanism produces:
-  Classical correlations from pre-established properties (S ≤ 2 from lattice)
- Quantum teleportation protocols (when Hilbert space structure is imposed)
- Entanglement entropy scaling
- Contextuality (within imposed QM formalism)

**On Bell's theorem**: FTD is explicitly a local deterministic substrate. The pure lattice gives S ≤ 2, which is the *correct* result for the substrate level. Quantum correlations (S > 2) are understood as **aggregate statistical properties** that emerge when embedded observers measure ensembles — not as behaviors the substrate must reproduce at the single-event level. The open question is demonstrating *how* the aggregate statistics arise, not *whether* the substrate violates Bell.

## 12.4 The sLoop: A Proposed Resolution

We introduce the concept of the **sLoop** (self-Loop): a closed causal structure where an observing system is part of the system being observed.

```
Standard observation:    Observer → System → Measurement
sLoop:                   System ⟲ (Observer ⊂ System)
```

### The Proposal

Bell inequality violations may arise **when and only when** the measurement apparatus is embedded in the same ontological substrate as the measured system. In FTD:

1. The apparatus is part of the flux field
2. The measurement act modifies the local flux configuration
3. Both measurements draw from the **same underlying potential**
4. The correlations exceed classical bounds because the "hidden variables" are not truly hidden from the measurement context—they ARE the measurement context

This is not superdeterminism (where initial conditions conspire to fake quantum correlations). It is **ontological holism**: the measurement apparatus and measured system share a common substrate that cannot be factorized.

### Bell-sLoop Conjecture

**CONJECTURE**: In FTD, Bell inequality violations occur when:
1. The entangled pair and measurement apparatuses are all manifested entities in the same flux field
2. The measurement process involves flux exchange between apparatus and particle
3. The "choice" of measurement basis is itself a flux configuration, not external

Under these conditions, correlations are not transmitted superluminally—they are **inherited from the shared substrate**.

### Connection to Reference frame context

The sLoop distinguishes:
- **Dead matter**: Entities that interact but do not self-reference
- **Life**: Entities that maintain themselves against entropy via feedback loops
- **Reference frame context**: Entities whose sLoop includes representation of the sLoop itself

Bell correlations, in this view, are signatures of the **ontological unity** necessary for self-reference.

> **Epistemic Status (v5.27 Update)**: The sLoop mechanism is now **[SELECTION]** — the three-level observer Bell hierarchy provides a concrete mechanism: substrate S=2 (deterministic threshold) → independent complex S=√2 (Born rule) → sLoop/entangled S=2√2 (joint coupling). Two factors: complexification changes correlation shape (sawtooth→cosine); sLoop doubles correlation strength. Net: S_substrate × √2 = S_observer. Verified 4/4 Monte Carlo checks (1M samples). See DERIV_OBSERVER_BELL_MECHANISM.md.

---

# Chapter 13: The Measurement Question

> **Cross-reference**: Manifestation dynamics serve as measurement theory. See §4 (Manifestation Dynamics) above.

## 13.1 The FTD Resolution

FTD proposes a resolution to the measurement problem by identifying **collapse with manifestation**:

| Concept | FTD Implementation |
|---------|-------------------|
| Wave function | Complexified flux: ψ = J_x + iJ_y |
| Superposition | Flux distributed over multiple voxels |
| Collapse | Manifestation: s transitions 0 → ±1 |
| Trigger | Flux concentration exceeding threshold KB |
| Born rule | P(v) = \|ψ(v)\|²/\|\|ψ\|\|² **[EMERGENT under IMPOSED sampling rule]** |

> **Epistemic Status [SELECTION + IMPOSED]**: The Born rule *emerges* under the manifestation-threshold sampling rule, which is itself **imposed** (not derived from more fundamental principles). The sampling rule states: "when flux concentration |J|² > KB, manifestation occurs with probability proportional to |J|²." This rule is **argued** (from conservation, concentration statistics, and max-entropy considerations) but the stochastic measure |J|² is not proven inevitable. A reviewer may reasonably ask: "Why |J|² and not |J| or |J|⁴?" Our answer: conservation + information-theoretic selection. This is a **selection principle**, not a theorem.

## 13.2 Why the Observer is Mandatory

The observer's role is **physical, not epistemic**. From the action principle, the coupling term is:

$$\mathcal{L}_{\text{coupling}} = -g_c \cdot s \cdot (\nabla \cdot J)$$

where $g_c$ is the state-flux coupling constant (dimensionless in natural units; see §7.1).

This means:
- A **manifested** observer (s ≠ 0) sources flux divergence
- Flux flows toward the interaction point
- **Concentration** triggers manifestation when |J|² > KB

**Without a manifested observer:**
- No coupling term active (s = 0 everywhere)
- Flux evolves via linear wave equation
- Superposition persists indefinitely

**With a manifested observer:**
- Coupling creates flux gradients
- Gradients concentrate flux locally
- Threshold crossing → collapse

**Key insight**: The observer is not special because it "observes" — it is special because it is **manifested** (s ≠ 0).

## 13.3 What Counts as an Observer?

Any manifested structure can trigger collapse:

| Structure | Can Trigger Collapse? | Why? |
|-----------|----------------------|------|
| Reference frame context | Yes | It's manifested (but not special) |
| Detector | Yes | It's manifested |
| Rock | Yes | It's manifested |
| Photon (flux wave) | **No** | Not manifested (s = 0) |
| Vacuum | **No** | Not manifested (s = 0) |

**Reference frame context has no privileged role** — any manifested structure couples to flux.

## 13.4 Foundational Questions Addressed

| Traditional Problem | FTD Proposed Resolution | Status |
|--------------------|----------------|--------|
| What distinguishes measurement? | Interaction with manifested structure (s ≠ 0) | **[SELECTION]** |
| Why is collapse probabilistic? | Threshold crossing statistics | **[SELECTION]** |
| Why is collapse irreversible? | Dissipation term in action (γ) | **[IMPOSED]** |
| Why Born rule? | Emerges from flux concentration + sampling rule | **[SELECTION + IMPOSED]** |
| Why definite outcomes? | Conservation + competitive threshold | **[SELECTION]** |
| Schrödinger's cat? | Cat is manifested → never in superposition | **[CONJECTURE]** |
| Wigner's friend? | Collapse is objective, not observer-relative | **[CONJECTURE]** |

## 13.5 The sLoop Connection

The **sLoop** (self-referential loop) captures a key insight: the observer is ontologically continuous with the observed.

```
STANDARD QM:
   Observer ────→ System ────→ Measurement
   (external)     (isolated)    (interaction)

FTD (sLoop):
   ┌──────────────────────────────────┐
   │         FLUX SUBSTRATE           │
   │                                  │
   │   Observer    ←→    System       │
   │   (s ≠ 0)           (s = 0)      │
   │                                  │
   │   Both embedded in same field    │
   └──────────────────────────────────┘
```

**Important distinction**:
- **Bell violations** come from Hilbert space structure (H₁ ⊗ H₂ tensor product)
- **Measurement** requires sLoop (observer-substrate coupling)

These are complementary, not conflicting.

**Observer/object formalism:** The observer/object distinction is grounded in the 3³ lattice: observer and object are structurally identical 3³ clusters, with the distinction being purely relational. Three observation modes emerge: external (disjoint clusters), overlapping (shared voxels), and self-referential (identical cluster). See [FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md](theory/02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md) Part II.

---

# PART D: SCOPE AND LIMITATIONS

---

# Chapter 14: What the Model Does Not Capture

## 14.1 Absent Physical Features

The following aspects of known physics are **not** present in the current model:

### Relativistic Physics
- No Lorentz covariance (cubic lattice breaks rotation/boost symmetry at small scales)
- No general relativistic curvature (fixed flat lattice)
- Time dilation: **v4.0 Update**—derived from boundary conditions, not merely implemented heuristically. The arrow of time follows from low-entropy initial conditions.

### Quantum Field Theory
- **U(1) gauge symmetry**: Argued to emerge from constraint structure (see Section 14.3)
- **SU(2), SU(3)**: May emerge from geometric structure; requires further analysis
- No renormalization group
- No virtual particle vacuum structure (in QFT sense)
- No spin statistics (Pauli exclusion is implemented phenomenologically)

### Standard Model Details
- Particle masses are input parameters, not predictions
- Coupling constants are tuned, not derived
- Weak isospin and hypercharge are absent
- Higgs mechanism is not implemented

### Gravity —  NOW DERIVED

> **UPDATE (v4.1)**: The gravity sector development is complete within the model.

**What is derived (within model assumptions)**:
- Inverse-square law from 3D geometry + flux conservation
- Newtonian gravity as weak-field limit of flux gradients
- Effective metric g_μν from flux density
- Geodesic motion equivalent to flux gradient force
- Linearized Einstein equations from flux wave equation (correspondence)
- Gravitational waves as transverse flux ripples

**What remains open**:
- Numerical value of G_N (hierarchy problem)
- Full nonlinear Einstein equations
- Quantum gravity unification

## 14.2 Structural Limitations

### Discreteness Artifacts
- Cubic lattice introduces preferred directions
- Rotation symmetry is approximate at best
- Lorentz invariance is fundamentally broken at the substrate level

### Lorentz Invariance: Relational Proposal and Hard Recovery Gate

FTD proposes that **Lorentz invariance is not a property of the substrate but could be a property of relationships between observers.** This is an interpretive conjecture, not a covariance result.

The cubic lattice fundamentally has a preferred frame. However, Lorentz invariance describes how **two observers** relate their measurements when in relative motion. It is a property of the **transformation between reference frames**, not of space itself.

The proposal concerns operational comparisons among observers. It does not imply that a single laboratory cannot detect a preferred frame: direction-, boost-, clock-, threshold-, or polarization-dependent observables can compare the laboratory against its own motion and orientation. A relational recovery would still have to prove that all such observables transform covariantly. Its intended ingredients are:
1. Two observers exist (two manifested structures)
2. They are in the same "observational space" (can exchange flux)
3. They compare measurements (interact)

The lattice is physical in the current update laws: its preferred foliation, stencil, and discrete clock enter the propagator. Calling it scaffolding does not remove the resulting preferred-frame operators.

**Established long-wavelength statement (free flux only)**: for `q_i=ak_i`, the production 18-point spatial symbol is rotationally invariant through `O(|q|^4)`, and directional anisotropy first occurs at `O(|q|^6)` in `omega^2` (`O(|q|^4)` in directional phase-speed spread). Combining that symbol with the actual discrete-time update gives, at `C_SPEED²=1/3`,

`theta² = S₂/3 - S₂²/54 - 11S₂³/9720 + S₂Q₄/216 - Q₆/270 + O(|q|⁸)`,
where `Q₄=sum(q_i⁴)` and `Q₆=sum(q_i⁶)`.

The isotropic `-S₂²/54` term is a dimension-six boost violation that a comparison among spatial directions cannot detect. It does not average away. No common renormalized cone has yet been shown for flux, manifested matter, Wilson matter, and gravity.

FTD-0407 further proves a scoped obstruction: within the current centered-time, nearest-Moore stencil class, quartic spatial isotropy and normalization force a spatial-symbol value `16/3` at `(pi,pi,0)`. Cancelling the dimension-six boost term requires Courant number `r²=1`, while linear stability requires `r²≤3/4`. Retuning the current coefficients therefore cannot supply the missing recovery; the update architecture must change.

FTD-0411 tests a different architecture suggested by the Moore decomposition:
the BCC product character becomes a selected synchronized temporal kernel
`T_B=(2/3)(1-cos³theta)`, while the existing SC+FCC `M18` remains physical
space. The conditional pole `T_B=c²M18` cancels its complete q4 term only at
`c²=1/7`. The principal phase is real across the full band, but its irreducible
cubic minimal polynomial has two complex conjugates. This excludes every exact
finite-state positive-norm linear/unitary auxiliary realization rational in
`M18`; direct use of the real cube root is nonlocal. The default-off CPU implementation is a stable
period-two IR localization with kicks `(1+sqrt(2))/7,(1-sqrt(2))/7`; it matches
the selected BCC pole through q4 and differs at isotropic q6. This opens a
nonlinear/constrained or interacting clock target; it does not establish
covariance or a common observable cone.

FTD-0412 corrects the matter comparison itself. The historical standalone
module evolved spatial `D_W` as a real-time Hamiltonian and mistook a
special-spinor norm for its energy spectrum; FTD-0126's orbit and `g-2`
numbers are therefore retracted as physical Wilson-Dirac results. Real-time
evolution now uses a Hermitian Wilson Hamiltonian. Its massless leading slope
can be selected to `c_s²=1/7`, but its q4 tensor
`c_s²[(r²/4)S2²-Q4/3]` cannot match the q4-free flux pole: an axis requires
`r²=4/3`, while a face diagonal requires `r²=2/3`. Manifested matter remains
an imposed `C_SPEED²=1/3` kinematic budget rather than a measured pole; gauge
and native latency gravity still provide no propagating poles.

FTD-0413 enlarges the standalone matter kinetic symbol to
`K_i=sin(q_i)[a+b(cos(q_j)+cos(q_k))]`, using only axial and face-diagonal
SC+FCC neighbours. Infrared normalization plus cancellation of both quartic
tensors uniquely fixes `a=b=1/3` and `r²=4/3` within this ansatz. Selecting
`c_s²=1/7` then produces a free Wilson-matter pole sharing the selected flux
cone through q4 while keeping all seven Brillouin-corner doublers gapped. The
gauge-covariant implementation averages the two shortest oriented paths to
each face diagonal and defaults off. Its first mismatch with the BCC-time flux
pole occurs at q6; the RK4 clock cannot remove the surviving mixed tensor.

FTD-0414 does not require exact q6 equality. For `q=ka`, it derives the
selected free-sector leading envelope
`Δv_max/c_s = 11q^4/540 + O(q^6)` and the inverse adequacy condition
`ka < (540 epsilon/11)^(1/4)`. This Lorentz-sector construction does not derive
`a`; adopting electron-primary or legacy Planck-primary gives `a=ell_P`
conditionally and makes the direct free-tree term tiny. This remains a
falsifier rather than a whole-theory compatibility result because the carrier,
finite-q, interacting, and radiative gates remain open.

> **Open hard gate:** FTD-0421 closes a conserved native additive current for the frozen ontology/basis, so that route cannot support a native charged common-cone claim and its dependent FTD-0422/0423 campaigns are not executed. Physical Lorentz adequacy now requires either an explicitly selected auxiliary carrier or a separately preregistered ontology revision, plus declared physical calibration, exact finite-q remainder control, production/manifested-matter integration, a gauge-independent on-shell successor to FTD-0419's nonzero off-shell coefficient, one fixed counterterm trajectory across thresholds, propagating gravity/composite sectors, positive low-energy spectral weight despite the non-injective production tick, and interacting Ward/dispersion tests. Exact all-orders Lorentz symmetry is not mandatory; compliance with every applicable empirical bound is. See FTD-0407–0425 and OPEN.2/OPEN.7.

### Finite Size Effects
- Boundary conditions (toroidal, absorbing, reflective) affect results
- Each run is on a finite region (the framework's undefined-boundary stance permits arbitrarily large finite extent but does not commit to a completed totality); convergence claims are stated as scaling laws across L, not as L → ∞ limits

### Computational Constraints
- Sparse representation limits accessible scales
- Real-time visualization constrains complexity

## 14.3 Gauge Symmetry: An Emergent Feature

Contrary to initial assessment, we argue that **U(1) gauge symmetry emerges naturally** from the constraint structure of FTD. The argument proceeds as follows:

### The Helmholtz Decomposition

The flux field J ∈ ℝ³ can be decomposed:
```
J = J_T + J_L
```
where:
- **J_T** (transverse): ∇ · J_T = 0
- **J_L** (longitudinal): ∇ × J_L = 0, so J_L = ∇φ

### The Constraint Structure

The longitudinal component is **not dynamically independent**. It is constrained by charge conservation:
```
∇ · J_L ~ ρ_charge  (Gauss's law analog)
```

This means J_L is determined by the charge distribution, not by independent initial conditions.

### Counting Degrees of Freedom

- J has 3 components
- 1 is constrained by Gauss's law
- Remaining: **2 physical transverse modes**

This matches the 2 polarizations of a massless gauge boson (photon).

### Gauge Transformation

Under J → J + ∇λ (for arbitrary scalar λ):
- J_T → J_T (invariant, since ∇λ is longitudinal)
- ∇ × J → ∇ × J (invariant, since curl of gradient = 0)

The physical observables—charge distributions and curl of flux—are gauge-invariant.

### Non-Abelian Extension (Speculative)

The three spatial dimensions of the lattice may provide structure for SU(3) color:
- A quark's "color" could correspond to the primary axis of flux alignment
- Color-neutral baryons would have flux distributed symmetrically across all three axes
- Local rotations of color orientation would constitute SU(3) gauge transformations

**Status**: U(1) emergence is argued; SU(2)/SU(3) emergence is speculative but geometrically motivated. See GAUGE_EMERGENCE_ANALYSIS.md for full derivation.

---

# Chapter 15: Open Problems

## 15.1 Theoretical

1. **Lorentz Recovery**: Under what conditions (if any) does approximate Lorentz invariance emerge at scales >> lattice spacing?

2. **Bell Compatibility**: Does the entanglement mechanism satisfy or violate Bell inequalities? If it violates, does it match quantum predictions?

3. **Gauge Verification**: The U(1) gauge emergence argument (Section 14.3) requires verification:
   - Do radiation modes have exactly 2 polarizations in simulation?
   - Is the longitudinal mode truly non-propagating?
   - Does the lattice discreteness introduce gauge-breaking at short scales?

4. **Non-Abelian Gauge**: Can the speculative SU(3) color interpretation (flux axis alignment) be made rigorous?

5. **Continuum Limit**: Does a meaningful continuum limit exist? What is the universality class?

6. **Unitarity**: Is the evolution unitary in any appropriate sense?

## 15.2 Computational

1. **Scaling**: How does computational cost scale with desired physical fidelity?

2. **Stability**: Are there parameter regimes where the simulation becomes unstable or pathological?

3. **Reproducibility**: How sensitive are results to initial conditions and random seeds?

## 15.3 Interpretive

1. **Correspondence**: What is the precise mapping between simulation quantities and physical observables?

2. **Falsifiability**: What experimental results would falsify the model's core claims?

3. **Uniqueness**: How much freedom exists in the parameter choices while maintaining qualitative behavior?

---

# Chapter 16: Empirical Contact Points

## 16.1 Epistemic Disclaimer

> **This chapter distinguishes three categories:**
> 1. **Headline Predictions** [CONJECTURE → TEST]: Specific, near-term testable with stated uncertainties
> 2. **Derived Outputs** [THEOREM]: Mathematical results that could falsify the framework
> 3. **Speculative / Long-horizon**: Generic consequences requiring future development

---

## 16.2 Headline Predictions

These are the sharpest claims where FTD makes contact with measurement.

### Prediction 1: Fine Structure Constant [CONJECTURE]

| Property | Value |
|----------|-------|
| **Claimed value** | $1/\alpha = 137.0360(2)$ |
| **CODATA 2022** | $1/\alpha = 137.035999177(21)$ |
| **Discrepancy** | 1.26 ppm (within stated uncertainty) |
| **Depends on** | [S1] CM preference, [S2] $j=1728$ selection, [S3] quadratic form |
| **What experiment measures** | QED calculations + precision measurements (Cs atom, electron g-2) |

> **Epistemic Status**: This is the framework's most constrained arithmetic match. The quadratic $x^2 - 16c^2x + 16c^3 = 0$ with $c = G^* = \Gamma(1/4)/\Gamma(3/4) \approx 2.9587$ yields $x_+ = 137.0360...$. **Note**: $G^*$ is distinct from the Bernoulli/Gauss lemniscate constant $\varpi = \Gamma(1/4)^2/(2\sqrt{2\pi}) \approx 2.6221$; substituting $c = \varpi$ would give $x_+ \approx 107.3$, which does not match $1/\alpha$. The identification $x_+ = 1/\alpha$ is [CONJECTURE]. The sub-ppm formulas below are retained as conjectural/post-hoc refinements, not as theorem-level physical predictions.

#### 16.2.1 The 4-Term Precision Formula [CONJECTURE]

> *Merged from SPEC_THE_COMPLETE_PROOF_RIGOROUS.md; retained as a conjectural precision fit after ledger downgrade FTD-0022.*

The proposed precision fit is:

$$1/\alpha = x_+ - \frac{9}{47}|\varepsilon| + \frac{5}{64}|\varepsilon|^2 - \frac{4}{141}|\varepsilon|^3 - \frac{141}{11}|\varepsilon|^4$$

where ε = e^π − π − 20 ≈ −0.0009 connects three structures:
- **Modular forms**: e^π = 1/q where q = e^(−π) is the lemniscate nome (from j = 1728)
- **Geometry**: π from circular symmetry
- **Framework integers**: 20 = b₃ + N_eff = 7 + 13

All coefficients are expressible in the base integers {3, 4, 7, 13}: 9/47 = N_c²/(3·16−1), 5/64 = (N_eff−2N_base)/N_base³, 4/141 = 4/(3·47), 141/11 = (3·47)/11. Expressibility is not, by itself, a derivation.

| Terms | Value | Error vs CODATA | Precision |
|-------|-------|-----------------|-----------|
| x₊ alone | 137.036171458 | 1.26 ppm | 6 digits |
| 2-term | 137.035999177029 | 0.21 ppt | 12 digits |
| 3-term | 137.035999177008 | 0.062 ppt | 13 digits |
| 4-term | 137.035999177000036 | < 0.001 ppt | 15 digits |
| CODATA 2022 | 137.035999177(21) | — | 10 digits |

**Status after Path B audit:** This formula is not used to rescue the Structure-2 result. It remains a conjectural integer-structured fit whose physical derivation is open. The robust claim remains the master-quadratic tree-level match at 1.26 ppm; sub-ppm improvements require an independently derived correction mechanism.

**Do not over-read:** coefficient rigidity inside a chosen ansatz is evidence about that ansatz, not proof that the ansatz is the physical alpha correction.

### Prediction 2: No Fourth Generation [CONJECTURE]

| Property | Value |
|----------|-------|
| **Claimed value** | $N_{\text{gen}} = \lfloor x_- \rfloor = \lfloor 3.024 \rfloor = 3$ exactly |
| **Uncertainty** | None (discrete prediction) |
| **Depends on** | [S3] Quadratic form, [C2] $x_-$ = effective color parameter |
| **What experiment measures** | Collider searches for 4th generation fermions |

> **Epistemic Status**: LHC has excluded sequential 4th generation quarks up to ~800 GeV. This prediction is **consistent** with current bounds but does not uniquely follow from FTD axioms—many theories predict 3 generations. It would be **falsified** by discovery of a 4th generation with standard gauge couplings (heavy sterile neutrinos do not count).

### Prediction 3: Substrate Locality [AXIOM CONSEQUENCE]

| Property | Value |
|----------|-------|
| **Substrate result** | $S \leq 2$ from pure lattice dynamics (local deterministic) |
| **QM aggregate** | $S = 2\sqrt{2} \approx 2.83$ (expected from ensemble statistics) |
| **Depends on** | [A4] Ternary states, POSTULATE 4 (local causality) |
| **What experiment measures** | Loophole-free Bell tests measure aggregate ensemble statistics |

> **Epistemic Status**: The substrate correctly gives S ≤ 2, consistent with local deterministic axioms. QM's S > 2 is understood as an aggregate property — analogous to how temperature is a statistical property absent at the single-molecule level. The **open question** is demonstrating explicitly how ensemble averaging over substrate states produces quantum correlations. This is the substrate-to-aggregate transition problem.

---

## 16.3 Derived Outputs [THEOREM]

These are rigorous mathematical consequences of the axioms. They do not predict new physics but constrain the framework.

| Output | Value | Depends on | Status |
|--------|-------|------------|--------|
| Gauss constraint → 16 DoF | $16 = 2^4$ | [A3] Gauss law | Proven |
| CM curve $j$-invariant | $j = 1728$ | [S1], [S2] | Selection, not proof |
| Quadratic roots | $x_\pm = 8c^2 \pm 8c^2\sqrt{1 - 1/c}$ | [S3] | Algebraic identity |
| Electron mass (dimensionless) | 0.5096 MeV (0.19% error) | §9.1 computation | Numerical simulation |

---

## 16.4 Speculative / Long-Horizon

These are generic predictions of discrete spacetime models or require substantial future development.

### Discrete Spacetime Signatures
- **Free-flux group velocity**: $v_g(E)=c[1-E^2/(12E_{\text{Planck}}^2)+O(E^4/E_{\text{Planck}}^4)]$ in the edge calibration. This is the production update's isotropic dimension-six preferred-frame term, not a universal photon prediction.
- **Cubic directional anisotropy**: the spatial direction-dependent spread begins at $O((E/E_{\text{Planck}})^4)$, but this is subleading to the isotropic boost violation and does not establish experimental safety.
- **Photon/graviton dispersion**: no identification with the free-flux pole, common-cone theorem, or radiatively stable interacting result has been established.

**Status**: These are generic predictions of discrete spacetime, not specific to FTD.

### Emergence Tests (Require Full Simulation)
- Atomic energy levels from first principles
- Nuclear binding energies
- Correct particle mass ratios

**Status**: Would test parameter tuning within the model, not foundational claims.

### Cosmological Observables (Speculative)
- Tensor-to-scalar ratio $r \sim 0.003$ — requires full gravity sector development
- Dark matter as unmanifested flux — purely speculative

**Status**: No concrete mechanism developed; listed for completeness only.

---

## 16.5 Falsification Criteria

What results would **conclusively falsify** FTD's core claims:

| Claim | Falsifying Observation |
|-------|------------------------|
| Quadratic structure | Precision $\alpha$ measurement incompatible with $x_+ = 137.036...$ at better than 10 ppm |
| 3 generations | Discovery of 4th generation with standard gauge couplings |
| Aggregate emergence | Demonstration that no ensemble averaging over local lattice states can yield S > 2 |
| Discrete spacetime | Observable Lorentz violation with wrong sign (superluminal high-energy photons) |
| Local causality | Nonlocal correlations without Hilbert space structure |
| Conservation laws | Energy/momentum non-conservation in simulations |

---

# PART E: IMPLEMENTATION

---

# Chapter 17: Architecture

## 17.1 Core Engine (`engine/`)

The simulation engine is written in C++17 with a CMake build system. For comprehensive documentation, see `engine/SPEC_ENGINE.md`.

```
engine/
├── CMakeLists.txt              # Build system — all targets and test registration
├── SPEC_ENGINE.md              # Living reference document (architecture, constants, tests)
├── include/ftd/
│   ├── ontic.h                 # Ontic derivation chain (9 layers, D=3 + varpi → all physics)
│   ├── constants.h             # Engine-facing constants (re-exports ontic + engine-specific)
│   ├── voxel.h                 # Vec3, Voxel struct (state, flux, velocity, spin, color)
│   ├── lattice.h               # 3D cubic lattice with periodic boundaries
│   ├── render_bridge.h         # RenderBridge — main engine API, tick(), diagnostics()
│   ├── lagrangian.h            # 9-term Lagrangian + Rayleigh dissipation
│   ├── term_toggles.h          # Table-driven registry (41 runtime booleans)
│   └── csv_export.h            # CSV export utility (flux field, slices, timeseries)
├── src/
│   ├── render_bridge.cpp       # Core engine — all tick phases (~1300 lines)
│   ├── lattice.cpp             # Lattice implementation
│   ├── lagrangian.cpp          # Lagrangian diagnostics
│   └── main.cpp                # CLI entry point (scenarios A/B/D)
├── tests/                      # 76 test files → 74 CTests (65 unit + 9 campaigns)
├── qt_gui/                     # Qt6 native GUI (optional build)
└── thirdparty/glad/            # OpenGL loader
```

**Build**: `engine\build_native.bat` (pins MSVC 14.44 via `vcvarsall x64 -vcvars_ver=14.44` — VS 18's default toolset crashes CUDA 13.0's `cudafe++`; presets in `engine/CMakePresets.json`)
**Test**: `cd engine/build && ctest --output-on-failure -C Release` (74 tests)
**GUI**: Requires Qt6; `cmake -S engine -B engine/build_qt -DFTD_BUILD_QT_GUI=ON`

## 17.2 Archived Components

The original Python engine (`ternary_matrix/`) and web interfaces (FastAPI/Vite/Three.js) have been archived to `archive/`. The C++ engine is the sole active implementation.

---

# Chapter 18: Simulation Probes

## 18.1 Terminology

The following are **internal diagnostic procedures**, not experiments in the scientific sense. They test whether the simulation behaves according to its design specifications.

## 18.2 Catalog of Probes

| Probe | Description | Success Criterion |
|-------|-------------|-------------------|
| Vacuum Stability | Empty lattice evolution | No spontaneous manifestation |
| Flux Propagation | Wave packet evolution | Propagation at speed C |
| Genesis Test | High-density void | Manifestation follows probability rule |
| Evaporation Test | Low-density particle | Returns to void below KB |
| Gravity Probe | Multiple particles | Drift toward center of mass |
| Charge Probe | +/- pair | Attraction; like charges repel |
| Collision Probe | Approaching particles | Correct collision outcomes |
| Annihilation Probe | +1 meets -1 | Both become 0, flux burst |
| Triad Stability | Three-particle config | Remains bound, no decay |
| Causality Probe | Separated regions | No FTL influence |
| Conservation Probe | Closed system | Total flux constant |
| Interference Probe | Two sources | Fringes at detectors |

---

# Chapter 19: Validation Procedures

## 19.1 Internal Consistency

- Energy (flux) conservation in closed systems
- Causality (no influence beyond light cone)
- State validity (no illegal transitions)

## 19.2 Behavioral Matching

- Do bound structures persist?
- Do forces produce expected qualitative behaviors?
- Do phase-like transitions occur at reasonable thresholds?

## 19.3 What Validation Does NOT Establish

Successful validation shows that **the simulation works as designed**. It does not show that:
- The design corresponds to physical reality
- The parameter choices are unique or necessary
- The interpretive mappings are correct

---

# PART F: THEORY

---

# Chapter 20: Formal Specification

## 20.1 Lattice and States

**Definition (Lattice)**: L = {(x,y,z) ∈ Z³ : 0 ≤ x,y,z < N} for some N ∈ N.

**Definition (State Configuration)**: S: L × N → {-1, 0, +1}

**Definition (Flux Configuration)**: J: L × N → R³

## 20.2 Neighborhood

**Definition (Moore Neighborhood)**:
```
N(v) = {u ∈ L : max(|ux-vx|, |uy-vy|, |uz-vz|) ≤ 1} \ {v}
```
This has 26 elements (in the interior).

## 20.3 Discrete Operators

**Definition (Discrete Gradient)**:
```
(∇f)_i(v) = (f(v + e_i) - f(v - e_i)) / 2
```

**Definition (Discrete Divergence)**:
```
∇·J(v) = Σ_i (J_i(v + e_i) - J_i(v - e_i)) / 2
```

**Definition (Discrete Curl)**:
```
(∇×J)_i(v) = ε_ijk (∂J_k/∂x_j - ∂J_j/∂x_k) / 2
```
where $\varepsilon_{ijk}$ is the **Levi-Civita symbol** (totally antisymmetric tensor): $\varepsilon_{123} = \varepsilon_{231} = \varepsilon_{312} = +1$, $\varepsilon_{321} = \varepsilon_{213} = \varepsilon_{132} = -1$, and $\varepsilon_{ijk} = 0$ if any two indices are equal. Summation over repeated indices is implied (Einstein convention).

**Definition (Discrete Laplacian)**:
```
∇²f(v) = Σ_{u ∈ N6(v)} f(u) - 6f(v)
```
where N6 is the 6-connected (face-sharing) neighborhood.

## 20.4 Update Equations (Summary)

**Flux Wave Equation**:
```
∂²J/∂t² ≈ C² ∇²J
```
Discretized via velocity-Verlet with damping.

**Manifestation**:
```
S(v,t+1) = +1 if S(v,t)=0, |J(v,t)|>KB, ∇·J>0, random<p
S(v,t+1) = -1 if S(v,t)=0, |J(v,t)|>KB, ∇·J<0, random<p
```

**Force Accumulation**:
```
F(v) = F_grav + F_elec + F_mag + F_strong + F_weak
```
(Each defined in Chapter 6)

---

# Chapter 21: Assumption Ledger

## 21.1 Definitions (Axiomatic)

| ID | Statement | Status |
|----|-----------|--------|
| DEF.1 | Space is a 3D cubic lattice with no defined boundary (axis-adjacent neighbours exist at every specified site; no completed-totality commitment) | Postulated |
| DEF.2 | Time is discrete ticks | Postulated |
| DEF.3 | States are {-1, 0, +1} | Postulated |
| DEF.4 | Flux is R³-valued | Postulated |
| DEF.5 | C = 1 voxel/tick | Postulated |
| DEF.6 | H = 1 (lattice unit) | Convention |
| DEF.7 | KB = 0.511 | Parameter (matched to electron mass) |

## 21.2 Assumptions (Modeling Choices)

| ID | Statement | Justification |
|----|-----------|---------------|
| ASSUMP.1 | Updates are local (26-neighbor) | Finite causality |
| ASSUMP.2 | Flux encodes energy/momentum | Interpretive |
| ASSUMP.3 | Genesis probability is exponential | Smoothness |
| ASSUMP.4 | Divergence sign → polarity | Symmetry breaking |
| ASSUMP.5 | Retarded positions for forces | Causality |
| ASSUMP.6 | DECAY_RATE = α | Phenomenological targeting |
| ASSUMP.7 | Triads are stable | Geometric stability |

## 21.3 Claims (Require Validation)

| ID | Statement | Status |
|----|-----------|--------|
| CLAIM.1 | ±1 states → fermions | Interpretive |
| CLAIM.2 | Flux waves → photons | Interpretive |
| CLAIM.3 | Triads → nucleons | Interpretive |
| CLAIM.4 | Shells → orbitals | Interpretive |
| CLAIM.5 | Gravity emerges from rules | Partially demonstrated |
| CLAIM.6 | EM emerges from rules | Partially demonstrated |
| CLAIM.7 | QM behavior emerges |  **VERIFIED** (v4.0: Hilbert space constructed) |
| CLAIM.8 | QM statistics emerge from substrate aggregation |  **[SELECTION]** — Three-level observer hierarchy: substrate S=2, independent complex S=√2, sLoop/entangled S=2√2. Complexification + sLoop joint coupling. See DERIV_OBSERVER_BELL_MECHANISM.md |
| CLAIM.9 | U(1) gauge symmetry emerges |  **VERIFIED** (OPEN.3 simulation) |
| CLAIM.10 | SU(3) emerges from geometry |  **VERIFIED** (OPEN.4 simulation) |
| CLAIM.11 | Lorentz invariance is relational | **[CONJECTURE]**; simulations of spatial isotropy do not test boost covariance (FTD-0407) |
| CLAIM.12 | Void is dispositional substrate | **Proposed** (Section 1.1) |
| CLAIM.13 | Update rules derived from action |  **VERIFIED** (v4.0: S[s,J] → Euler-Lagrange) |
| CLAIM.14 | Continuum limit → Maxwell |  **VERIFIED** (v4.0: lattice → field theory) |
| CLAIM.15 | Continuum limit → Schrödinger |  **VERIFIED** (v4.0: non-relativistic limit) |
| CLAIM.16 | Thermodynamics from microstates |  **VERIFIED** (v4.0: Boltzmann statistics) |
| CLAIM.17 | Spinors from frame topology |  **VERIFIED** (v4.0: π₁(SO(3)) = ℤ₂) |
| CLAIM.18 | Time's arrow from boundary |  **VERIFIED** (v4.0: low-entropy past) |
| CLAIM.19 | Measurement = manifestation |  **VERIFIED** (v4.0: threshold = collapse) |

## 21.4 Open Questions

| ID | Question | Status | Reference |
|----|----------|--------|-----------|
| OPEN.1 | How do aggregate QM statistics (S>2) emerge from substrate-level dynamics (S≤2)? |  **[SELECTION]** | Three-level observer Bell mechanism: complexification (Gauss constraint → psi = J_x + iJ_y) + sLoop joint coupling. Net: S_substrate × √2 = S_observer. Verified 4/4 checks. See DERIV_OBSERVER_BELL_MECHANISM.md |
| OPEN.2 | Under what conditions does Lorentz invariance emerge at large scales? | **OPEN — HARD GATE; FROZEN NATIVE CHARGE ROUTE CLOSED** | FTD-0414 derives the selected free-sector envelope `11(ka)^4/540`; FTD-0415 permits marginal time-space kinetic ratios; FTD-0416 limits imported IR attraction to at best `1/137^3`; and FTD-0419 finds `delta_match/g²=-0.32696906(5)` in one off-shell step scheme. FTD-0421 proves the frozen native additive charge nullspace is trivial, dependency-closing the native charged-pole and dimension-four-flow campaigns. FTD-0424's one-calibration contract shows the existing off-shell surrogate fails its first multiplicity threshold. Adequacy still requires a gauge-independent on-shell auxiliary calculation, one fixed physical threshold trajectory, finite-q/production integration, and manifested low-energy spectral unitarity (FTD-0407–0425). |
| OPEN.3 | Can U(1) gauge emergence be verified in simulation? |  **VERIFIED** | 2 transverse modes, longitudinal suppressed <3% |
| OPEN.4 | Can SU(3) color interpretation be made rigorous? |  **VERIFIED** | N_c≈3.024 from geometry, color neutrality, confinement all confirmed |
| OPEN.5 | Can coupling constants be derived within FTD assumptions? |  **[PARTIAL / SMC]** | Algebraic-spine constants have theorem-level provenance, but the physical coupling identifications (especially x₊  1/α) remain [STRONGLY MOTIVATED CONJECTURE]. See SPEC_ALGEBRAIC_SPINE.md, SPEC_FQCR.md, TRACKER_ONTIC_TRUTH.md, and LEDGER.md. |
| OPEN.6 | What is the testable difference between sLoop and superdeterminism? |  **OPEN** | Proposed: sLoop predicts tunable S(f); requires experimental test |
| OPEN.7 | Does the relational Lorentz interpretation satisfy all experimental tests? | **OPEN** | FTD-0421 closes the frozen native additive-current route negative; FTD-0422/0423 are not executed by dependency. FTD-0424 closes one fixed counterterm across the first available off-shell multiplicity threshold but leaves the physical on-shell coefficient open. FTD-0425 proves the linear sector reversible and the full production tick non-injective, leaving manifested low-energy spectral unitarity unmeasured. No gauge-independent on-shell coefficient, universal physical counterterm trajectory, dynamical gravity/composite poles, live interacting common cone, operational clock/rod result, or bounds comparison exists; see FTD-0407–0425 LR-0..LR-6. |
| OPEN.8 | Can particle masses be derived from FTD? |  **[SMC] / [PARAMETRIC]** (retagged 2026-07-12 to canon) | m_e = m_P √(2π) (16/3) α¹¹ (0.19%) is [SMC] FTD-0015 (exponent n=11 [SELECTION]); other masses per CATALOG_PARAMETRIC_INSERTIONS |
| OPEN.9 | What determines the complexity functional C(g)? |  **OPEN** | Candidates: MDL, departure from unification, parameter counting |
| OPEN.10 | Can spinor behavior emerge from framed flux? |  **VERIFIED** | 720° symmetry, exchange antisymmetry, Pauli exclusion all confirmed |
| OPEN.11 | Can CKM matrix be derived from FTD? |  **[PARAMETRIC]** (retagged 2026-07-12 — integer-ratio insertions, not a forcing chain) | All elements to 3-6%; see FLAVOR_PHYSICS_DERIVATION.md + CATALOG_PARAMETRIC_INSERTIONS |
| OPEN.12 | Can PMNS mixing be derived from FTD? |  **[STRUCTURALLY MOTIVATED PARAMETRIC]** (demotion of record FTD-0320/FTD-0021) | All three angles to 1-3%; see FLAVOR_PHYSICS_DERIVATION.md |
| OPEN.13 | Can CP violation be predicted? |  **[PARAMETRIC]** (retagged 2026-07-12) | Jarlskog J = 3.9×10⁻⁵ (27%), CKM phase δ = 68° (1.5%) |
| OPEN.14 | Can neutrino masses be derived? |  **[PARAMETRIC]** (see-saw machinery imported; retagged 2026-07-12) | Δm²₃₁ match via see-saw with m_D ~ m_τ × α |
| OPEN.15 | What is the UV distribution P_UV? |  **OPEN** | Maximum entropy? Conformal? Big Bang initial conditions? |
| OPEN.16 | What determines G_N? | **[STRUCTURALLY MOTIVATED PARAMETRIC]** (corrected 2026-07-01, FTD-0348 — not DERIVED; inherits FTD-0015's [SMC] floor, precision spelling-dependent) | α_G = 2π(16/3)²(N_eff + 3/7)²α²⁰ |
| OPEN.17 | Why does a 3D lattice exist? |  **[SELECTION — declared] (FTD-0355)** | The |Aut(E)|² = 2^D·(D−1)! = 16 arithmetic uniqueness is [THEOREM]; the dimension-forcing is [SELECTION — declared] (RHS target 16 = |O_h|/3 presupposes D=3, circularity named). Atomic stability + gauge renormalizability + Fibonacci constraint are motivation, not a forcing proof. |
| OPEN.18 | Can GR be derived with correct coefficient? |  **RECLASSIFIED [PARAMETRIC]/[SELECTION]** (the v5.0 "DERIVED" label is historical — the Deser bootstrap *completes* a posited massless spin-2 field, it does not derive one; conditional on Conjecture 10.1, FTD-0189; substrate spin-2 mode [CLOSED NEGATIVE in probed regime], FTD-0193) | R_μν - ½g_μν R = 8πG T_μν; bootstrap per FTD-0026 |
| OPEN.19 | Can inflation observables be derived? |  **RECLASSIFIED [PARAMETRIC]** (v5.0 label historical — standard slow-roll formulas with FTD numbers; W-COSMO-1 inflaton identification [OPEN]) | n_s = 0.966 (0.2σ from Planck), r = 0.022 (below bounds) |
| OPEN.20 | Can baryogenesis be explained? |  **RECLASSIFIED [PARAMETRIC]** (v5.0 label historical — standard Sakharov machinery with FTD inputs) | η ~ 10⁻¹⁰ from CP violation + Sakharov conditions |
| OPEN.21 | Is x₊ = 1/α a theorem or conjecture? |  **[STRONGLY MOTIVATED CONJECTURE]** | Master-quadratic algebra is theorem-level; the physical identification x₊  1/α is empirical/structural, not dynamically derived. |
| OPEN.22 | Is `x₋ → N_c = 3` a theorem? | **RETIRED (v1.4 §5)** | The `x_-  N_c` identification is dropped; LEDGER FTD-0014 removed in commit `ca7eb61`. The small root `x_- ≈ 3.024` is a mathematical artifact of P(x). `N_c = 3` in FTD is independently sourced (Moore Layer Theorem; `DERIV_NC_FROM_TOPOLOGY.md`). |

See `packages/backend/simulation/open_question_tests.py` and `packages/backend/simulation/flavor_physics_tests.py` for simulation implementations.

---

# Chapter 22: Interpretive Summary

## 22.1 What We Have Built

A computational simulation based on:
- Discrete 3D lattice
- Ternary states
- Local update rules
- Continuous flux field
- Threshold-based manifestation

## 22.2 What We Observe (Internally)

- Bound structures resembling particles
- Force-like behaviors
- Hierarchical organization
- Interference patterns

## 22.3 What We Propose (Speculatively)

- These structures may correspond to physical particles
- The forces may reduce to known physics in some limit
- Quantum-like behavior may emerge from classical rules

## 22.4 What Has Been Verified (In Simulation)

### Structural Emergence
-  **U(1) gauge emergence** (2 transverse modes, longitudinal suppression)
-  **Bound structures** (triads persist, shells form)
-  **Force-like behaviors** (attraction, repulsion, binding)
-  **Wave propagation** (flux waves at speed C)

### Quantum-Like Features
-  **Interference patterns** (flux superposition)
-  **Spinor behavior** (720° symmetry from framed flux)
-  **Bell locality**: Substrate gives S≤2 (expected for local axioms). Aggregate QM statistics (S>2) understood as ensemble property; emergence mechanism characterized as three-level observer hierarchy; S = 2 sqrt(2) follows from Tsirelson's bound once QM emergence is established
-  **Born rule** (derived from flux concentration statistics)

### Constants and Phenomenology (historical v5.0 block; current tags live in LEDGER.md)
-  **Fine structure constant α = 1/137.036** ([STRONGLY MOTIVATED CONJECTURE] via x₊ identification; 1.26 ppm)
-  **Color charge number N_c = 3** (the `x₋` identification is RETIRED v1.4 §5; `N_c = 3` from independent structural sources — Moore Layer Theorem, `DERIV_NC_FROM_TOPOLOGY.md`, Z₃ center closure)
-  **Electron mass m_e = 0.511 MeV** (selection/parametric chain depending on α; see LEDGER)
-  **Tau mass m_τ = 1.777 GeV** (selection/parametric chain; see LEDGER)
-  **Proton mass m_p = 938.3 MeV** (selection/parametric chain; see LEDGER)
-  **Higgs VEV v = 246 GeV** (structurally motivated parametric chain; see LEDGER)
-  **CKM/PMNS matrices** (flavor-physics claims require their local ledger tags)
-  **CP violation δ = 66.8°** (structural/selection status depends on local derivation)
-  **Neutrino masses** (uses seesaw mechanism in FTD framework; check local tag)
-  **Gravitational hierarchy α_G** (depends on α and framework integers; check local tag)

### Cosmology (v5.0 - New)
-  **Inflation spectral index n_s = 0.966** (0.2σ from Planck measurement)
-  **Tensor-to-scalar ratio r = 0.022** (well below experimental bounds)
-  **Baryogenesis η ~ 10⁻¹⁰** (correct order of magnitude)
-  **Dark matter = sub-threshold flux** (0 < |J| < K_B)

### Particle Physics Coverage (v5.17 - Honest Assessment)
**Note:** These are **PARAMETRIC INSERTIONS** — FTD values inserted into standard physics formulas (Fermi theory, HQET, ChPT). The functional forms are **imported**, not derived.
-  **22 decay rates/widths**: Uses Fermi decay formula with FTD masses/couplings
-  **14 running coupling scales**: Uses standard RG with FTD beta coefficients
-  **42 mesons**: Uses chiral perturbation theory with FTD quark masses
-  **48 baryons**: Uses quark model + Regge trajectories with FTD parameters
-  **4 decay constants**: Pattern-matched, not derived from dynamics

**Honest summary:** FTD provides input parameters; standard QFT provides the physics.

## 22.5 What Remains Open

### Resolved / Addressed in This Program (v4.1)
- ~~Derivation of coupling constants within FTD assumptions~~  **ADDRESSED (within assumptions)** (G* via FTD + self-consistency + CM selection; see archive/ARCH_LEMNISCATE_ALPHA_PAPER.md)
- ~~Derivation of particle masses within FTD assumptions~~  **ADDRESSED (within assumptions)** (m_e = m_P √(2π) (16/3) α¹¹; see archive/ARCH_LEMNISCATE_ALPHA_PAPER.md)
- ~~Uniqueness of integers {3,4,7,13}~~  **RESOLVED** (self-consistency proof; see SELF_CONSISTENCY.md)
- ~~Gravity sector~~  **ADDRESSED (within assumptions)**
- ~~Flavor physics (CKM, PMNS)~~  **ADDRESSED (within assumptions)**

### Historical v5.0 Resolutions (reclassified by later audits)
- ~~**Numerical value of G_N**~~  **RECLASSIFIED [STRUCTURALLY MOTIVATED PARAMETRIC]** (FTD-0348) — α_G = 2π(16/3)²(N_eff + 3/7)²α^20; precision is spelling-dependent (+0.06% vs −0.33% canonical-mass spelling); inherits FTD-0015's [SMC] floor
- ~~**Why a 3D discrete lattice exists**~~  **[SELECTION — declared] (FTD-0355)** — the |Aut(E)|² = 2^D·(D−1)! = 16 arithmetic uniqueness is [THEOREM], but the dimension-forcing is not forced (RHS target 16 = |O_h|/3 presupposes D=3, a circularity named). See §22.5.1.
- ~~**C1: x₊ = 1/α**~~  **RECLASSIFIED: [STRONGLY MOTIVATED CONJECTURE]** — CM/uniqueness evidence supports the bridge but does not derive the physical identification.
- ~~**C2: x₋ → N_c = 3**~~ **RETIRED (v1.4 §5)** — `x_-  N_c` identification dropped; LEDGER FTD-0014 removed in commit `ca7eb61`. `N_c = 3` in FTD from independent structural sources.
- ~~**GR with 8πG**~~  **RECLASSIFIED [PARAMETRIC]/[SELECTION]** (FTD-0189) — the Deser bootstrap completes a *posited* massless spin-2 field (Conjecture 10.1); substrate spin-2 mode [CLOSED NEGATIVE in probed regime] (FTD-0193)
- ~~**Inflation mechanism**~~  **RECLASSIFIED [PARAMETRIC]** — standard slow-roll formulas with FTD numbers (n_s = 0.966, r = 0.022); inflaton identification W-COSMO-1 [OPEN]
- ~~**Baryogenesis**~~  **RECLASSIFIED [PARAMETRIC]** — standard Sakharov machinery with FTD inputs (η ~ 10⁻¹⁰)

### Genuinely Open (v5.27)
- ~~Substrate-to-aggregate transition~~  **[SELECTION]** — Three-level observer Bell mechanism identified: complexification + sLoop joint coupling. Net: S_substrate × √2 = S_observer. See DERIV_OBSERVER_BELL_MECHANISM.md. Dynamical derivation of joint probability from S[s,J] remains future work.
- Sub-ppm precision tests of α
- Detection of Planck-scale Lorentz departures

### 22.5.1 On the 3D Discrete Lattice

**STATUS (FTD-0355):** D = 3 is **[SELECTION — declared]**. The arithmetic uniqueness of |Aut(E)|² = 2^D·(D−1)! = 16 is a [THEOREM], but the dimension-forcing (D=3 as the physical spatial dimension) is not forced: the RHS target 16 = |O_h|/3 presupposes D=3, a circularity named (bounded search). The six arguments below are motivation for the [SELECTION], not a forcing proof.

#### The Dimensional Hierarchy

Before examining why D = 3, we must understand how dimensions emerge at all. A single spatial axis does not constitute a complete dimension:

| Level | Components | Status |
|-------|------------|--------|
| 0.5D | X or Y alone | Incomplete (no orientation without reference) |
| 1D | XY | First complete spatial dimension |
| 2D | XY + T | Space with time |
| 3D | XY + Z + T | Full spacetime |
| 3D+1 | XYZT + gΨ(ΔT) | Spacetime with gravitational-wavefunction coupling |

**Key insight**: Dimensions require pairs. A single axis X has no meaning without a contrasting axis Y to define extent and direction. A line existing outside spacetime has no determinable orientation—it simply *is*. Only when a second line is introduced can orientation, cardinal directions, and geometry emerge.

This has a profound implication: **relativity and subjectivity are emergent**. At 0.5D there is only pure existence without perspective. At 1D (two axes in relation), the very concept of "relative to what?" emerges—the birth of perspective, relativity, and the precondition for observers. Subjectivity is not added to an objective universe; it is co-emergent with spatial relation itself.

#### Six Independent Arguments Motivating D = 3 [SELECTION — declared]

**Argument 1: Gauge Theory Requirements**
SU(3) gauge theory with confinement, asymptotic freedom, AND chiral anomaly (needed for baryogenesis) exists only in 3+1 dimensions.

**Argument 2: Spinor Structure**
Spin(3) = SU(2), which gives 2-component spinors. In other dimensions, fermion structure is wrong.

**Argument 3: Knot Theory**
Non-trivial knots exist only in 3D. Particles as topological features require this richness.

**Argument 4: Observer Existence**
Stable atoms with shell structure require 1/r² potentials, which arise from 3D Laplacians.

**Argument 5: Parsimony**
A 3D cubic lattice is the simplest structure supporting gauge theories + observers.

**Argument 6: Fibonacci Constraint (v5.0)**
The self-referential closure condition n_eff = F_7 = 13 = b_3 + 2N_c is only satisfied for D = 3.

**Status (FTD-0355)**: D = 3 is **[SELECTION — declared]**. The |Aut(E)|² = 2^D·(D−1)! = 16 arithmetic uniqueness is a [THEOREM], but the dimension-forcing is not forced — the RHS target 16 = |O_h|/3 presupposes D=3, a circularity named (bounded search). The arguments above motivate the selection; they do not constitute a forcing proof.

## 22.6 The Appropriate Epistemic Stance

This model is a **discrete substrate framework** — it proposes that reality is built from local, deterministic update rules on a ternary lattice. It is **not** a "Theory of Everything" and does not claim to be. FTD is a candidate ontology: a set of postulates that may underlie the physics we observe, including the aggregate statistical behaviors that quantum mechanics describes.

**Relationship to quantum mechanics:** FTD does not attempt to "recover" or "reproduce" quantum mechanics. QM is understood within this framework as the **observation of aggregate behaviors** emerging from the substrate. The Bell inequality result S ≤ 2 from pure lattice dynamics is not a failure — it is the expected consequence of local deterministic axioms. Quantum correlations (S > 2) are interpreted as emergent statistical properties of ensembles, not as fundamental substrate behavior requiring reproduction.

Evaluation criteria and status (updated for v5.24):

1. Internal consistency —  **ESTABLISHED** (rules derived from action principle)
2. Qualitative plausibility —  **DEMONSTRATED** (structures, forces, hierarchies, gauge emergence)
3. Derivation from principles —  **PARTIAL** (~20 genuine derivations; rest uses imported physics)
4. Substrate-to-observable mapping —  **INCOMPLETE** (how aggregate QM statistics emerge from lattice is open)
5. Quantitative accuracy —  **MIXED** (~20 genuine predictions < 1% error; rest are parametric fits)
6. Novel predictions —  **POSTDICTIONS** (most results fit known data; few pre-observation tests)

**v5.17 Honest Status:**
- **~20 Genuine Derivations**: α, sin²θ_W, mass ratios, mixing angles (from G* and {3,4,7,13})
- **~50 Parametric Insertions**: FTD-derived values inserted into standard QFT formulas
- **~50+ External Physics Adopted**: Fermi theory, HQET, ChPT, RG running — not derived
- **~3-5 Explicit Inputs**: M_Planck, G_F, Λ_QCD, decay constants — required but not derived
- **"Zero free parameters" is FALSE**: The claim conflates derived parameters with zero inputs

See [EPISTEMIC_AUDIT.md](theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) for complete breakdown.

**Remaining work:**
1. Characterize the substrate-to-aggregate transition (how QM statistics emerge from deterministic lattice)
2. Produce a genuinely novel pre-observation prediction
3. Close the Λ_QCD derivation loop

See [FTD_REFERENCE.md](theory/01_reference/SPEC_FTD_REFERENCE.md) and [CHANGELOG.md](../CHANGELOG.md) for documentation.

---

# APPENDIX A: Glossary

| Term | Definition |
|------|------------|
| Voxel | A single lattice site |
| Flux | Vector field J ∈ R³ on each voxel |
| Density | Scalar |J|, magnitude of flux |
| Manifestation | Transition from state 0 to ±1 (wavefunction collapse) |
| Genesis | Event of manifestation (pair production analog) |
| Evaporation | Transition from ±1 to 0 |
| Annihilation | +1 and -1 adjacent → both become 0 |
| Triad | Three-particle bound configuration (nucleon analog) |
| Tick | One discrete time step |
| KB | Manifestation threshold = m_e c² = m_P √(2π) (16/3) α¹¹ (derived) |
| sLoop | Self-referential loop; observer-system coupling structure (§12.4) |
| G* | Γ(1/4)/Γ(3/4) ≈ 2.9587; lemniscatic bridge constant from CM elliptic curve E_i (§7.4) |
| Born rule | P(v) = \|ψ(v)\|²/\|\|ψ\|\|²; probability from wave function (§13.1) |
| Action S[s,J] | Variational principle from which update rules derive (v4.0) |
| Hilbert space H_FTD | L²(Lattice, ℂ) constructed from complexified flux (v4.0) |
| Moore neighborhood | 26-connected neighborhood (3×3×3 cube minus center) |
| N₆(v) | 6-connected (face-sharing) neighborhood for Laplacian |
| ∇² | Discrete Laplacian operator (§3.2, §20.3) |
| ∇·J | Divergence of flux field (determines polarity) |
| ∇×J | Curl of flux field (magnetic-like behavior) |
| 0.5D | Single axis without reference; exists but undetermined (potential, not actual) |
| Dimensional hierarchy | How dimensions emerge from pairs: 0.5D → 1D → 2D → 3D → 3D+1 |
| Emergent relativity | Relativity and subjectivity co-emerge with spatial relation at 1D |

---

# APPENDIX B: Major Revisions from Version 2.0

## B.1 Structural Changes

1. **Separated ontology from empiricism**: Clear distinction between postulates, rules, and interpretations
2. **Added Abstract for Physicists**: Concise technical summary
3. **Added Preamble on document status**: Explicit disclaimers
4. **Renamed "Experiments" to "Simulation Probes"**: Clarifies internal vs external validation
5. **Added "Scope, Limitations, and Open Problems" (Part D)**: Comprehensive honesty
6. **Added "Potential Empirical Contact Points"**: Speculative but explicit

## B.2 Language Revisions

1. **Replaced absolute claims with hedged language**:
   - "solves" → "addresses in a particular way"
   - "proves" → "is consistent with"
   - "derives" → "targets" (for constants)

2. **Labeled metaphors explicitly**:
   - "The Void is home" → marked as metaphorical
   - "Ontology rendered executable" → identified as interpretive framing

3. **Converted philosophical prose to technical language**:
   - Reduced poetic passages
   - Added formal definitions where possible

## B.3 Scientific Corrections

1. **Constants**: Explicitly stated that numerical matches are parameter choices, not derivations
2. **Quantum claims**: Added extensive caveats about untested Bell compatibility
3. **Measurement**: Clarified that dissolving the measurement problem is a claim, not established
4. **Entanglement**: Noted tension with Bell's theorem for hidden variable approaches
5. **Lorentz invariance**: Acknowledged fundamental violation by lattice structure

## B.4 Removed or Downgraded

1. Removed claims that the model "explains" known physics
2. Downgraded "emergence" claims from strong to weak
3. Removed implication that constants are predicted
4. Removed suggestion that foundational problems are solved

## B.5 Version 4.0 Additions (Theoretical Foundations)

1. **Partial action principle**: the written `S[s,J]` supports the free field equations, stationary-source variation, and replay of selected force formulas. It does not generate the production genesis/evaporation state transitions; the former all-update claim is retracted by FTD-0567.
2. **Hilbert Space**: Quantum mechanics constructed from complexified flux ψ = J_x + iJ_y (Part II)
3. **Continuum Limit**: Rigorous recovery of Maxwell electrodynamics (§3.4) and Schrödinger equation (§3.5)
4. **Born Rule**: Derived from manifestation statistics, not postulated (§2.3)
5. **Bell Violations**: Three-level observer Bell mechanism resolves OPEN.1: substrate S=2 [THEOREM], observer S=2√2 via complexification + sLoop [SELECTION]. See DERIV_OBSERVER_BELL_MECHANISM.md
6. **Spinor Structure**: Fermi statistics from frame bundle topology π₁(SO(3)) = ℤ₂ (Part V)
7. **Measurement Theory**: Collapse = manifestation triggered by observer coupling (see §13)
8. **Dimensional Analysis**: Complete natural units framework added (§7.1)
9. **Notation Standardization**: g_c (state-flux) vs g_s (strong) coupling constants distinguished

---

# APPENDIX C: Editor's Note

## What Changed and Why

This revision transforms a speculative manifesto into a scientifically defensible framework document. The core vision—ternary states, discrete dynamics, emergent structure—is preserved. What changed is the epistemic framing.

### Key Changes

1. **Intellectual Honesty**: The original document conflated simulation design with physical discovery. Parameters chosen to match known physics were sometimes presented as emergent. This revision clearly labels inputs vs outputs.

2. **Quantum Reframing (v5.24)**: FTD does not attempt to recover QM. QM is understood as aggregate statistical behavior of the substrate. The substrate correctly gives S≤2 (local deterministic). The open question is the substrate-to-aggregate transition. See BELL_MECHANISM_HONEST.md.

3. **Force Realism**: The original presented force laws as emerging from geometry. This revision notes that forces are phenomenologically inserted, borrowing functional forms (Yukawa, Coulomb) from established physics.

4. **Scope Boundaries**: The original covered all scales from Planck to cosmic. This revision maintains the broad scope but explicitly notes which claims are interpretive proposals vs demonstrated behaviors.

5. **Falsifiability**: The original lacked clear failure conditions. This revision identifies what would constitute falsification.

### What Remains

- The three-state ontology
- The discrete lattice approach
- The two-layer (flux/state) architecture
- The local update rules
- The vision of emergent complexity

### The Document's New Status

This is now positioned as:
- A computational framework for exploring discrete ontologies
- A simulation platform with interpretive mappings to physics
- A set of speculative proposals requiring independent validation
- An honest acknowledgment of what is and is not demonstrated

The goal is a document that could be submitted to a foundations-of-physics venue without misrepresenting its claims.

---

# APPENDIX D: Notation Glossary

This appendix provides a comprehensive reference for all notation used in FTD, organized by category.

## D.1 Fundamental Entities

| Symbol | Type | Domain | Definition | First Use |
|--------|------|--------|------------|-----------|
| $\mathbf{L}$ | Set | $\subset \mathbb{Z}^3$ | Discrete lattice of voxels | §1.1 |
| $v$ | Element | $\in \mathbf{L}$ | Single voxel (lattice site) | §1.1 |
| $t$ | Scalar | $\in \mathbb{N}$ | Tick counter (discrete time) | §1.1 |
| $s(v,t)$ | Function | $\to \{-1, 0, +1\}$ | Ternary state at voxel $v$, time $t$ | §1.1 |
| $\mathbf{J}(v,t)$ | Vector field | $\to \mathbb{R}^3$ | Flux vector at voxel $v$, time $t$ | §3.1 |

## D.2 Derived Fields

| Symbol | Type | Definition | Dimensions | First Use |
|--------|------|------------|------------|-----------|
| $\rho$ | Scalar field | $\|\mathbf{J}\|$ (flux magnitude) | [E]/[L]² | §3.3 |
| $\bar{\rho}$ | Scalar field | Neighbor-averaged density | [E]/[L]² | §6.2 |
| $\psi$ | Complex field | $J_x + i J_y$ (wave function) | [E]^(1/2)/[L] | §13.1 |
| $q(v)$ | Scalar | Charge at voxel $v$ | dimensionless | §6.3 |

## D.3 Differential Operators (Discrete)

| Symbol | Definition | Notes |
|--------|------------|-------|
| $\nabla f$ | $(f(v+e_i) - f(v-e_i))/2$ | Discrete gradient |
| $\nabla \cdot \mathbf{J}$ | $\sum_i (J_i(v+e_i) - J_i(v-e_i))/2$ | Discrete divergence |
| $\nabla \times \mathbf{J}$ | $\varepsilon_{ijk}(\partial_j J_k - \partial_k J_j)/2$ | Discrete curl |
| $\nabla^2 f$ | $\sum_{u \in N_6(v)} f(u) - 6f(v)$ | Discrete Laplacian (6-connected) |

## D.4 Coupling Constants

| Symbol | Value | Dimensions | Physical Role | Notes |
|--------|-------|------------|---------------|-------|
| $C$ | 1.0 | [L]/[T] | Speed of causality | Axiomatic |
| $K_B$ | 0.511 | [E]/[L]² | Manifestation threshold | ≡ electron mass |
| $\alpha$ | 0.00729 | dimensionless | Fine structure constant | From G* |
| $g_c$ | ~$\alpha^{1/2}$ | dimensionless | State-flux coupling | §7.3, §13.2 |
| $g_s$ | — | dimensionless | Strong (Yukawa) coupling | §6.4 |
| $G_N$ | 0.01 | dimensionless | Gravitational coupling | §6.2 |
| $\gamma$ | 0.00729 = α | [T]⁻¹ | Decay/dissipation rate | §4.3 |
| $\lambda$ | — | dimensionless | Gauss constraint strength | §1.2.1 (TF) |
| $\mu$ | — | dimensionless | Ternary constraint strength | §1.2.4 (TF) |

## D.5 Mathematical Constants

| Symbol | Value | Appears In |
|--------|-------|------------|
| $G^*$ | 2.9587... | Lemniscatic constant (§7.4) |
| $\phi$ | 1.618... | Golden ratio (binding energy) |
| $\pi$ | 3.14159... | Standard |
| $e$ | 2.71828... | Exponential base |

## D.6 Action Principle Variables

| Symbol | Role | Appears In |
|--------|------|------------|
| $S[s,J]$ | FTD action functional | Part G |
| $\mathcal{L}$ | Lagrangian density | Part G |
| $V(\rho, s)$ | Manifestation potential | Part G |
| $\mathcal{F}$ | Dissipation function | Part G |

## D.7 Quantum Mechanics

| Symbol | Definition | Notes |
|--------|------------|-------|
| $\mathcal{H}_{\text{FTD}}$ | $L^2(\text{Lattice}, \mathbb{C})$ | FTD Hilbert space |
| $\|\psi\|$ | $\sqrt{\sum_v |\psi(v)|^2}$ | Norm |
| $\langle\psi|\phi\rangle$ | $\sum_v \psi^*(v)\phi(v)$ | Inner product |
| $P(v)$ | $|\psi(v)|^2 / \|\psi\|^2$ | Born rule probability |

## D.8 Standard Physics Comparisons

| FTD Symbol | Standard Physics | Relationship |
|------------|------------------|--------------|
| $\mathbf{J}$ | $\mathbf{A}$ (vector potential) | $\mathbf{J} \leftrightarrow \mathbf{A}$ in gauge theory |
| $\nabla \times \mathbf{J}$ | $\mathbf{B}$ (magnetic field) | Direct correspondence |
| $-\nabla \cdot \mathbf{J}$ | $\rho$ (charge density) | Via Gauss constraint |
| $\psi = J_x + iJ_y$ | Wave function | Complexified transverse flux |

## D.9 Abbreviations and Acronyms

| Abbreviation | Full Form |
|--------------|-----------|
| FTD | Ternary Realization Dynamics |
| sLoop | Self-referential Loop (observer-system coupling) |
| QM | Quantum Mechanics |
| QFT | Quantum Field Theory |
| SM | Standard Model |
| E-L | Euler-Lagrange (equations) |

---

**END OF REVISED DOCUMENT**

*Prepared for critical evaluation*
