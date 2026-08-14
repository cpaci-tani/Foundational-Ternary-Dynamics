# The Priced-Import Ledger — What FTD Must Borrow, Counted in a Common Currency

**Tag:** [SYNTHESIS] (prices existing commitments at their canonical tags; introduces no theorem, promotes nothing) · **LEDGER id:** FTD-0371
**Data:** `import_ledger.json` (machine-readable, the single source) · **Verifier:** `scripts/proofs/proof_import_ledger.py` (8/8 PASS) · **Date:** 2026-07-05 · **Rev:** v1.26 2026-08-12 (FTD-0998/0999 derives the cumulative finite-resource, atomic-backpressure, causal-delay, and exact-reversal laws; native phase-complete reserve density/current remains open, no v1 currency change; prior v1.25 existing-relative-pair growth machine)

> **⚠ Rev-stamp / data-file sync flag (added this pass, unresolved).** This header's rev stamp (v1.26, 2026-08-12) is ahead of `import_ledger.json`'s own `revision_log`, which stops at **v1.3 (2026-07-26)**. The §0 running-log narrative below (the FTD-0901/0903/0904/0905–0907/0988–0999 entries, none of which change v1 currency) appears to be real and already reconciled against the LEDGER, but no corresponding `revision_log` entries (v1.4 through v1.26) exist in the JSON, and this pass could not confidently reconstruct which narrative entry corresponds to which intermediate version number — the §0 prose does not itself carry per-entry `vX.Y` tags. **This is flagged, not fixed:** bringing `import_ledger.json`'s `revision_log` up through v1.26 is an owner reconciliation item, not resolved here. No tag, price, or currency total is affected by the gap — every §0 entry states explicitly that it changes no v1 currency.

**Serves:** the Number-One Goal's boundary face — *"rigorously mark and price which types the ontology sets for itself and which it must import"* (the 2026-07-05 wording of that face; the amended sentence carries it as "…either forced content or a rigorously marked and priced import") — and, since the 2026-07-12 amendment (FTD-0383), doubles as the **work queue of the goal's third face**: *drive every priced line to retirement, to a theorem-grade no-go, or to a sharper falsifier — never leaving a line merely booked* (program charter: `SCOPE_CONSUMPTION_PROGRAM.md`). The modulus/argument frontier (`FOUND_MODULUS_ARGUMENT_FRONTIER.md`, FTD-0336) states the boundary *qualitatively*; this ledger prices it *quantitatively*, with a falsifier on every line.

---

## 0 · Scope and reading guard

The goal has three faces (amended 2026-07-12, FTD-0383): **derive** (build content forward from the types the ontology sets), **mark and price the boundary** (name the types it cannot set and must import — and count each in a common currency with its falsifier), and **drive** (treat every priced line as a standing work item: run its falsifier, attack its retirement path under a fresh lock, prove its no-go, or convert it via a declared minimal adoption). The second face is a deliverable only if it is *auditable* — so this ledger enumerates every import in a common currency, tags each at its canonical LEDGER status, and attaches a falsifier to each. It is the dual of `CATALOG_PARAMETRIC_INSERTIONS.md`: that catalogs one currency (parametric insertions); this rolls up **all** of them and adds the self-set credits and the declined bets.

> **Reading guard (load-bearing).** The **adopted-bit total is 1** — the FC-W branch choice of the α-sector algebra (the sign of δ). This is **not** FTD's total cost of physics. The empirical identification `x₊ = 1/α` [SMC], the ~131 [PARAMETRIC] insertions, and the 3 dimensional calibrations are *separate, larger* debts, all listed below. Anyone who reads "1 bit" as "FTD imports only one bit of physics" has misread the ledger. The 1 bit is the *algebraic branch*; the *physics* is imported separately and abundantly, at its honest tags.

This document moves no tag. `x₊ = 1/α` stays [SMC]; MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION]; FC-W stays [AXIOM]; D=3 stays [SELECTION — declared].

> **Successor-branch boundary (FTD-0825, 2026-08-09).** This ledger and
> `import_ledger.json` remain the canonical account for FTD v1/FC-1. They are
> intentionally frozen rather than retrofitted. The ratified contextual-
> actualization successor carries a branch-local overlay at
> [`contextual_actualization_register_v2.json`](contextual_actualization_register_v2.json).
> That overlay separately books the adopted potentiality architecture, selected
> qutrit representation, preparation map, contextual selector, G* clock role,
> the FTD-0860 signed-quarter-turn action-pump reference, and the FTD-0862
> prepared phase-calendar/directed export-rail reference. Its currencies are
> not collapsed into the v1 "one adopted bit" total. FTD-0863 refines that
> existing phase-rail selection into a separate conserved phase reference and
> zero-baseline signal lane; it adds no selected-type currency. Native reference
> formation, robust maintenance/backreaction, rail protection, production
> event coupling and energy-current accounting, cubic embedding, and controller
> work remain a separately named open debt.
> FTD-0865 adds an explicitly **imposed reference Hamiltonian law** on those
> already-booked types: a harmonic phase waveform and commensurate coupling
> ratios realize the exact stroboscopic hold/swap. This is not a new ontological
> type and does not change the five-type count, but the functional law is an
> input, not a derivation. Its convex-clock theorem closes the uncompensated
> universal load-blind quartic controller only in the registered minimal class.
> Dynamic eligibility, compensation, quartic action-angle realization, native
> mode formation, cubic/production coupling, and controller-state accounting
> remain open.
> FTD-0867 further refines the same selected phase-rail transaction without a
> new selected type: the already-booked ternary latch supplies the unique even
> degree-at-most-two eligibility map `epsilon=s^2`, and the outgoing oriented
> signal retains `(s,B)` across a gate-zero release request. The remaining
> controller debt is narrower but physical: native latch formation,
> autonomous acknowledgement/reset, microscopic bath and reset-work closure,
> clock synchronization, native mode preparation, and cubic transport. The
> zero clutch work at the gate endpoint is not zero latch/reset work.
> FTD-0869 closes the abstract acknowledgement/reset sub-debt without adding a
> selected type. The completed local signal is the acknowledgement token, so
> no extra acknowledgement bit is booked. The half-cycle waveform and
> coefficients remain **imposed functional choices**, while the cusp potential,
> overdamped subgradient law, controller reservoir, and scalar bath account are
> **selected realizations** on the existing type surface. Their reset cost is
> `kappa*A`, distinct from FTD-0867's zero gate-endpoint clutch-switch work.
> A locally Lipschitz autonomous exact finite-time reset is closed negatively.
> Microscopic bath dynamics, native latch/reference/mode formation, robustness,
> protected cubic transport, production coupling, quartic compensation, and
> native `G*` synchronization remain priced open debts.
> FTD-0871 closes a still narrower logical debt: on the actual ternary layer,
> the completed oriented signal reversibly uncomputes the matching latch by
> controlled subtraction in `Z_3`. Therefore no extra acknowledgement bit,
> reset-history trit, or logical bath is booked. FTD-0869's cusp potential,
> subgradient law, reservoir, and scalar bath remain an optional **selected
> continuous realization** only if the `x` latch is retained physically; they
> are not mandatory v2 actual-layer types. The physical controlled permutation,
> controller work, native formation, robust output clearing, protected cubic
> transport, production coupling, and `G*` synchronization remain open at this
> historical FTD-0871 boundary.
> FTD-0872 closes the **logical** controlled-permutation sub-debt without
> adding selected-type currency. The unique oriented ternary isometry is
> `R(s,o)=(-o,s)`, with ready emission `(s,0)->(0,s)` and inverse absorption.
> A naive empty-port/otherwise-identity wrapper is noninjective, so physical
> readiness must be scheduled or a reciprocal/reflected output retained. The
> label-norm identity is not a physical energy/work derivation.
> FTD-0873 closes the **imposed harmonic reference-actuator** sub-debt without
> adding selected-type currency. The four-dimensional lift supplies exact
> hold/forward/reverse windings, the declared record-energy scale
> `nu a^2/2`, maximum transient clock/interaction exchange `Omega A/2`, and
> zero complete-cycle endpoint residual. The scale and Hamiltonian remain
> imposed; zero net endpoint work is not zero transient work. Repeated active
> cycles and invariant-norm gating are closed negatively as autonomous
> one-shot schedulers. Native scale/formation, gate-zero acquisition/release,
> reciprocal backpressure-safe handoff, protected cubic transport, production
> coupling, robustness, and synchronization to the separate quartic `G*`
> calendar remain priced open debts.
> FTD-0874 closes the **selected finite-horizon scheduling and reference-rail**
> sub-debt without adding selected-type currency. Alternating existing global
> tick parity with rail-coordinate parity schedules the oriented bond map on
> disjoint nearest-neighbour bonds. A prepared isolated record moves exactly
> one cell per tick and is exactly recoverable. Occupied bonds retain both
> labels by reciprocal exchange, but do not guarantee readiness or progress.
> Native intersite Hamiltonian formation, multidimensional routing,
> congestion/backpressure resolution, finite-boundary completion, production
> coupling, robustness, and synchronization to the separate quartic `G*`
> calendar remain priced open debts.
> FTD-0875 closes the **local canonical Hamiltonian reference-lift and bond-
> current** sub-debt without adding selected-type currency. The scalar finite
> rail has a common symplectic form only by pairing boundary-mirror sites; in
> the registered onsite-direct-sum local class, one canonical pair per site is
> minimum and sufficient. The imposed positive Hamiltonian reproduces the
> forward and inverse parity layers exactly after one clock cycle and supplies
> local antisymmetric energy currents, an exact prepared-record transfer, and
> a closed clock/action ledger. Native formation of that canonical doublet and
> its scale, multidimensional routing, congestion and finite-boundary handling,
> production coupling, robustness, synchronization to the separate quartic
> `G*` calendar, and operational hiding remain priced open debts.
> FTD-0876 closes the **native carrier-coordinate availability** sub-debt
> without adding selected-type currency. Two consecutive flux slices are
> exactly equivalent to the phase-complete Markov pair `(J,wave_vel)`, and the
> production `Voxel` already stores three such canonical pairs. The symmetric-
> stiffness free kick/drift is exactly symplectic and invertible. This does not
> supply record preparation or the FTD-0875 bond actuator: damping, Langevin,
> Gauss projection, manifestation/loss, and boundary maps prevent promotion to
> a symplectic complete-tick claim. Preparation/persistence, scale recovery,
> production actuation, constrained dynamics, environment-complete loss,
> routing, robustness, quartic-`G*` synchronization, and hiding remain priced
> open debts.
> FTD-0880 closes the **matched constrained-coordinate and static ternary-
> record representation** sub-debt without adding currency. On the selected
> oriented-face complex, divergence and its pseudoinverse give an exact
> transverse/charge canonical split and a unique minimum-energy static neutral
> record section. The charge coordinate is local, but a uniformly finite-range
> translation-invariant conjugate is impossible; the inverse solve is
> relational. Retaining the discarded longitudinal discrepancy makes abstract
> affine preparation exactly reversible. The live cell-centred central-
> difference/18-point-SOR pass is not the matched exact projector. At the
> FTD-0880 boundary, dynamic native preparation, a physical environment carrier and reversible dynamics,
> matched production actuation, scale/routing, finite-support uncontained
> completion, quartic-`G*` synchronization, and hiding remain priced open debts.
> FTD-0882 closes the **conditional dynamic reference-preparation and abstract
> reversible history-carrier** sub-debt without adding currency. Local six-face
> residual/environment quarter-turns, alternated by existing checkerboard tick
> parity and supplied with fresh zero ports, converge from empty flux to the
> FTD-0880 minimum-energy record without evaluating the inverse Laplacian in
> any local gate. Retaining every outgoing signed port makes each finite
> history exactly reversible. The exact local work ledger gives, at this
> declared boundary, equal limiting field and exported-history energies, each
> one half of supplied source work. This does not supply autonomous fresh-port
> generation or recycling, a positive canonical source reservoir, local
> stopping, moving-source continuity, nonperiodic/uncontained boundaries,
> finite-capacity backpressure, production migration, physical scale,
> quartic-`G*` synchronization, or hiding; those remain priced open debts.
> FTD-0884 closes the **finite-horizon explicit ready-bank and positive source-
> work register** sub-debts without adding currency. A capacity-`C` cyclic bank
> of initially zero signed-port vectors supplies exactly `C` fresh reversible
> reference layers; a generic nonzero stored vector returns on layer `C+1`, so
> indefinite finite cyclic freshness closes negative only in that explicit-
> port class. This is not a universal finite-dimensional memory no-go and does
> not exclude exact-real compression. On the already available continuous
> carrier type, the sign-preserving quadratic battery law is uniquely fixed by
> exact energy conservation once `E_b=b^2/2` is imposed. The law and reserve
> scale remain inputs, not recovered physics. A canonical Hamiltonian battery,
> native formation/recharge, unbounded/open or justified compressed signed
> history, 3D routing and backpressure, moving-source continuity, production
> migration, physical scale, quartic-`G*` synchronization, and hiding remain
> priced open debts.
> FTD-0886 closes the **positive source-centered local canonical layer and raw-
> work/interaction-energy accounting** sub-debts without adding currency. With
> `u=(d_xJ-q_x)/sqrt(6)`, `a=e_x/sqrt(6)`, canonical conjugates, and
> `N=(u^2+a^2+pi_u^2+pi_a^2)/2`,
> `L=a*pi_u-u*pi_a`, the imposed positive clocked Hamiltonian
> `H=omega I+omega N+sigma*omega(1-cos(theta))L/4` generates the exact
> source-centered quarter-turn over one clock cycle while satisfying
> `|L|<=N`. The source-work identity is instead the exact interaction-energy
> exchange `E_raw+U_int=(u^2+a^2)/2`, with
> `U_int=-s*y+s^2/2`; no separate post-hoc battery is required at this fixed-
> source local scope. FTD-0884's square-root amplitude law remains exact on
> the zero-conjugate Lagrangian section, but its cotangent lift changes the
> full quadratic energy by `-w(1+p_b^2/b^2)`, and a phase-blind state-dependent
> action drain is not symplectic. A complete-pair open history shift is
> kinematically canonical and reversible; finite cyclic capacity remains a
> boundary. The autonomous common parity controller, dynamical source
> formation/motion/recoil, a physical native open or bilateral complete-pair
> history (or justified compression), 3D routing, production migration,
> physical scale, quartic-`G*` synchronization, and hiding remain priced open
> debts.
> FTD-0888 closes the **external integer-parity switch and positive canonical
> reaction-channel existence** sub-debts at reference level without adding
> type currency. Six disjoint `C^1` windows of one already selected phase
> coordinate compile the exact color-0/color-1 order under the autonomous
> positive Hamiltonian
> `H=Omega I+6 Omega N+Omega sum_j kappa_j rho_j G_j`; clock action returns at
> every boundary. A zero-initialized positive reaction cannot coexist with the
> unchanged history-saturated FTD-0886 endpoint. Reducing the history amplitude
> makes one additional instance of the existing canonical-pair type minimum
> and sufficient: `E_hist=cos^2(eta) E_res` and
> `E_react=sin^2(eta) E_res`. The equal split `eta=pi/4` is a selected parameter
> conditional on imposed output-channel exchange symmetry, not a new selected
> type and not a derivation from P1--P5. Native formation of the phase windows
> and their origin/scale, physical identification of the reaction pair with
> spatial ternary-source momentum, source mass/inertia and intercell motion, a
> physical open complete-pair history, 3D routing, production migration,
> quartic-`G*` synchronization, and hiding remain priced open debts.
> FTD-0890 closes the **scalar-to-spatial-vector representation question** and
> an exact conditional source-transport gearbox without adding type currency.
> Cubic symmetry forces every scalar-only equivariant recoil map to zero, and
> one spatial vector copy cannot carry a nondegenerate alternating form. The
> minimum orientation-free registered carrier is therefore three instances of
> the existing canonical-pair type, `T1u+T1u`. Conditional on the already
> selected relativistic dispersion, an exact cotangent map converts quadratic
> reaction energy to physical momentum, and the inherited free drift/current
> closes reversibility and continuity. A matched local field impulse supplies
> direction and fixes `sin^2(eta)=K_req/E_res`; equal split is conditional, not
> universal. The dispersion, `E0`, `c`, vector reaction interpretation, and
> source initial data remain selected/imposed. Native vector common-action
> formation, dynamical field-to-triplet coupling, inertial-mass derivation,
> stable matter, production migration, quartic-`G*` synchronization, and
> operational hiding remain priced open debts.
> FTD-0892 closes the **selected common-action origin of the reaction
> triplet** without adding type currency. Once the already selected
> constituent canonical phase space exists, its Helmert reduction splits the
> canonical one-form exactly into a three-pair center/summed-momentum sector
> plus internal modes. The orientation-free carrier required by FTD-0890 is
> therefore not another selected type. Conditional on the selected
> constituent relativistic dispersions, strict convexity also gives the exact
> minimum composite dispersion and additive inertia
> `M_coll=sum(epsilon_a)/c^2`. This does not retire the mass-scale debt:
> constituent rest energies and `c` remain inputs, static stability and its
> Hessian do not identify kinetic curvature, and a static binding offset that
> does not participate in the boosted family produces an exact rest/inertia
> mismatch. Exact `Z^3` translations and Bloch transport likewise do not make
> the observed matter-plus-field momentum an additive continuous Noether
> charge. Constituent formation, dynamically dressed boost closure, exact
> total physical momentum, absolute mass, stable matter, production,
> quartic-`G*` synchronization, and hiding remain priced open debts.
> FTD-0893 sharpens the **dynamically dressed mass debt** without adding type
> currency. For a complete time-odd tangent energy Hessian `A` and an
> independently defined additive physical total-momentum map `B`, exact
> constrained minimization gives `M=B A^-1 B^T`. A moving field coat can
> contribute through its odd sector, but a static energy offset cannot. The
> same `A` under `B->sB` gives `M->s^2M`, closing the energy-only route to
> absolute mass negative. The current common action has exact energy and
> measured co-moving dressing but no exact total `B`; the natural
> spline-Poynting candidate remains closed negative for coupled recoil.
> Retirement now requires an independent local stress/momentum state or exact
> additive operational quasimomentum ledger whose mass agrees across
> constrained energy curvature, impulse/center velocity, and matter--field
> momentum partition. Exact total momentum, absolute mass, stable matter,
> production, quartic-`G*` synchronization, and hiding remain priced open.
> FTD-0896 then prices the **translation-spectral momentum route** without
> changing currency. `Z^3` characters supply exact `T^3`-valued
> quasimomentum, but no continuous homomorphic section `T^3 -> R^3` exists,
> and no finite-range periodic spectral weight equals the global unwrapped
> coordinate. A real lift therefore needs a nonlocal branch or an integer
> winding/history triplet, and physical momentum still needs an independently
> fixed scale. Neither winding dynamics nor that scale is adopted here. A new
> local stress/bond-impulse state remains unexcluded. Retirement requires one
> route to produce an exact matter--field exchange law and the same FTD-0893
> inertia from energy curvature, impulse/velocity, and momentum partition.
> FTD-0897 closes the minimum **conditional carry update** without changing
> currency. For a supplied opposite pair increment, the unique integer update
> `W'=W+c_1+c_2` restores exact lifted additivity and reverses exactly when the
> increment is reversibly available. This does not derive the increment,
> choose a particle/bond/substrate/stress partition, fix the physical unit, or
> give the reservoir an energy law. Periodic band energy is blind to winding,
> and opposite quasimomentum transfer need not conserve band energy.
> Retirement now requires one local matter--field action that supplies the
> impulse, carry ownership/transport, exact work/backreaction, and physical
> normalization before insertion into the FTD-0893 tensor.
> FTD-0898 closes the **increment origin and positive-energy ledger only
> inside the existing selected relative-quartic reference model**, again
> without changing currency. Its discrete-gradient step generates exact
> equal-and-opposite channel impulses, conserves the relative quartic energy,
> composes with every FTD-0897 reciprocal carry, and shares the exact continuum
> law `T A=sqrt(pi) G* sqrt(m/(2 lambda))`. The common mode is exactly
> invariant, `p_*` remains imposed, the integer carry has no derived energy,
> and the finite-step orbit has no derived integer-tick `G*` cadence.
> Retirement now requires a substrate-derived common coupling that transfers
> impulse and energy between sectors while conserving full momentum and
> retaining reversible history, then fixes or honestly prices `p_*`.
> FTD-0901 closes the **existence of one exact imposed common-action gearbox**
> without changing currency. The velocity-linear connection has nonzero
> curvature, conserves its positive Hamiltonian, canonical momentum, and
> canonical angular momentum, and gives `Delta K=-gamma Delta D` while
> composing with reciprocal carry conditional on `p_*`. This does not derive
> `gamma`, `p_*`, the physical variables, the total production momentum map,
> or absolute mass. Moreover, continuous nonzero `gamma` adds rest-sector
> Hessian `gamma^2/M`, so the exact critical quartic survives only at
> `gamma=0`, which turns off the gearbox. Retirement now requires a
> context-blind phase clutch or positive compensated action with switching
> work, clock compliance, physical normalization, and production partition
> all closed independently of `G*`, context, outcome, or Born targets.
> FTD-0903 closes the **positive-connection order boundary and one exact
> rest-sector self-pair escape** without changing currency. For `B=DA(0)`, the
> positive rest clock Hessian is `B^T M^-1 B`, so nonzero linearized engagement
> cannot preserve critical quarticity. The imposed existing self-pair law
> `A(D)=gamma|D|D` has zero origin derivative and folds exactly into
> `Lambda=lambda+gamma^2/(2M)`, preserving the rest-sector quartic and its
> continuum `G*` factor while giving `Delta K=-gamma Delta(|D|D)`. This does
> not derive the connection law, `gamma`, `p_*`, mass, or production. The law
> is `C1` but not `C2`, generic moving sectors regain a quadratic term, and a
> symmetric rest cycle has zero net common drift. Retirement now requires a
> context-blind rectifier with controller state and switching work closed or a
> separate clock/gearbox architecture, plus physical normalization and
> production partition, all independently of `G*`, context, outcome, or Born
> targets.
> FTD-0904 closes the **rest-sector rectification existence question inside
> one imposed oriented reference law** without changing currency. Retaining a
> local unit polar axis `e` and time-odd chirality `chi`, the even connection
> `A=chi gamma q^2 e` preserves the exact rest quartic and produces exact
> directed transport with displacement per cycle proportional to `1/G*` and
> mean speed per squared amplitude proportional to `1/(G*)^2`. A nonzero even
> polar rectifier cannot be inversion-equivariant if it depends on `D` alone,
> so the retained orientation memory is mathematically necessary in the
> registered class. This does not derive the native formation or maintenance
> of `e` or `chi`, `gamma`, `p_*`, mass, production, or finite-tick cadence.
> Retirement now requires a target-blind native current/history construction
> of that orientation memory with work, retention, erasure, reversal, and
> symmetry controls closed.
> FTD-0905--0907 close the narrower **native-type representability** question
> without changing currency. A neutral ternary dipole supplies an origin-
> independent polar axis, while the antisymmetric wedge of bilateral
> flux/wave-velocity projections supplies a time-odd chirality. Symmetric
> square/Gram data loses their signs. Under one imposed central quartic memory
> law the wedge is conserved and bounded, but its centrifugal term proves
> that this same mode cannot remain the exact pure radial `G*` clock.
> Retirement now requires production-native, target-blind formation and
> persistence of the neutral dipole and nonzero wedge, their work/erasure
> ledger, derivation of the memory law and coupling, and a separate clock-
> memory architecture synchronized without reading `G*`, context, outcome,
> or Born targets.
> FTD-0963 closes the **positive complete-square phase-connection** sub-debt
> without changing the frozen v1 currency. Its own source document states
> explicitly that no new public ontology type is introduced: the connection
> `𝒜(δ)` in `H_conn` is a selected functional coupling assembled entirely from
> four already-priced reference-mode canonical pairs (token battery, outgoing
> record, target controller, aligned reserve) from the native-time-carrier
> programme, not a variant of IMP-S4's charged-matter gauge-connection carrier.
> Native formation, production realization, and replenishment of those modes
> remain open debts.
> FTD-0974 closes the **minimum faithful positive canonical suspension**
> sub-debt without changing the frozen v1 currency. Given the already-
> classified C4-carrier gauge underdetermination, the adopted coupling law
> `H_susp` realizes the faithful identity representation on one further
> instance of the existing canonical-pair type; the companion v2 register
> (`contextual_actualization_register_v2.json`) already carries this row as an
> open-debt refinement (`OPEN-CA-TRANSDUCER`) rather than as one of its five
> selected reference types. Substrate identity, formation, switching, `G*`
> cadence, Born/Bell, and production remain open.
> FTD-0982 closes the **local canonical work-port and phase-dependent seam
> family** sub-debt without changing the frozen v1 currency. The identified
> work-port pair `(theta,I_R)` is one further instance of the existing
> canonical-pair type, and the selected seam family/instantaneous local root
> extends FTD-0980's own seam handshake on already-booked type surface rather
> than opening a new selection axis. The physical work reservoir and
> multicomponent factor realization remain open.
> FTD-0987 closes the **regional common-mode work-pair ownership designation**
> sub-debt without changing the frozen v1 currency. The ownership designation,
> projector-based isolation clutch, and switching-work law are mechanism/law
> choices built entirely on the pre-existing dual canonical-pair fields and the
> pre-existing ternary latch, matching the disposition its own direct
> successor FTD-0988/0989 receives below (booked in the v2 overlay as a
> selected law/open realization debt, not a new v1 import row). Native
> ownership formation and production coupling remain open.
> FTD-0988/0989 closes the successor's **local work-ownership/current law at
> reference scope** without changing the frozen v1 currency. The exact C18
> incidence factor permits the Moore-local gate
> `K_ell=B^T diag(1-ell_b^2)B`; a regional boundary cut remains positive,
> carries an exact antisymmetric open-bond current, has explicit reciprocal
> switching work, and admits a zero-work/zero-impulse seam at zero bond
> strain. Fixed-gate kick--drift has an exact local shadow Hamiltonian and
> inverse. The physical modal action is frequency-normalized,
> `H_u=omega I_u`; the earlier `H+2I` identity is retained only as an
> observable-amplitude audit. No continuous type is added. The candidate does
> require distributed reversible ternary boundary ownership—one latch per
> controlled bond or a proved equivalent site encoding. Because neither that
> encoding nor its autonomous formation exists in production, this is booked
> in the v2 overlay as a selected law/open realization debt rather than minted
> as a new v1 import row. A future adoption of irreducible bond memory must be
> priced explicitly; a native site-local encoding would retire that price.
> FTD-0990 supplies exactly that **native static encoding** and therefore
> retires the contemplated per-bond memory price before adoption. The unique
> charge-blind occupancy `m=s^2` gives
> `g_xy=1-(m_x-m_y)^2`, so every static matter--void gate is recomputed from
> its endpoint states. Conditional on isolating the common mode while leaving
> the relative C18 channel open, L/R symmetry uniquely fixes the boundary-
> supported quadratic block. The same occupancy mask supports the imposed
> onsite clock and makes the connected body's uniform common mode uniquely
> lowest. This does not make the functional coupling native production: an
> active-aperture controller, temporal switch history, physical formation
> actuator/inverse, and mode preparation remain selected/open. If that coupling is eventually
> adopted rather than derived, price the **law**, not a redundant bond-memory
> type. `omega0` remains imposed and is not a `G*` derivation.
> FTD-0991/0992 now closes the **conditional transition ledger** without
> changing currency. A simultaneous occupancy flip changes only its cut-set,
> with exact work
> `W_S=sum_(b in boundary S)(1-2g_b)a_b d_b^2/2`; one-site growth costs
> `E_join-E_cut`, and same-point reversal returns the opposite work. A prepared
> local action books it through `I'=I-W/Omega`. Static boundaries still need
> no auxiliary type. A genuinely active fail-closed aperture, however, must
> distinguish blank, closed `+/-`, and open `+/-`; within the registered
> ternary interface this requires two ternary slots and the reversible transfer
> `(sigma,0)<->(0,sigma)`. Those slots are a selected **controller realization
> debt**, not a new frozen v1 import row, because no production adoption has
> occurred. The selected dual-stiffness law, positive reserve, and first phase
> remain open; any future irreducible adoption must price those functions
> explicitly.
> FTD-0993/0994 retires the **abstract zero-action phase-seed price** at
> bounded conditional reference scope. The action--angle chart is singular at
> `I=0`, but the underlying Cartesian pair is not. A coordinate-gradient
> momentum shear maps local positive work `U` and retained orientation `sigma`
> to `P'=sigma sqrt(2U)`, hence `I'=U/Omega` and
> `theta'=-sigma*pi/2`, with exact symplectic inverse and no target read. This
> does not mint a new type: it uses the existing receiver pair and retained
> sign. The remaining price moves to the **law/identification layer**—derive
> the physical `U`, receiver/frame formation, and a nearest-neighbor causal
> growth/locking dynamics. Instantaneous extended uniform preparation is
> excluded by locality and free phase copying is noncanonical; a new site must
> arrive phase matched with its energy share or retain a mismatch mode.
> FTD-0995/0996 now retires the **abstract local coherent-growth law** at an
> exact conditional compliance surface. At a donor kinetic crossing, the
> FTD-0994 seed equals the donor state iff released formation work satisfies
> `2mU_y=p_x^2`. This condition is necessary and sufficient, not a fitted
> coefficient. It gives exact local energy/inverse closure and, conditional
> on selected quartic hardware, inherits the donor's amplitude, orientation,
> `G*` period, and CM normalization without a target read. No new type or v1
> currency is minted. The remaining price sharpens to the **law/controller
> layer**: derive why the common/relative membrane dynamics lands on this
> surface or provide a positive local mismatch port, backreaction,
> tolerance/robustness, scheduling, and production inverse.
> FTD-0997 retires the **new-pair capacity price** for that mismatch machine.
> The existing relative `L-R` pair can swap its complete phase-bearing state
> into a blank common receiver and be refilled by the FTD-0994 work shear. The
> map is symplectic, energy exact, and invertible; the port is catalytic iff
> `U=e` and otherwise retains `U-e`. No seventh continuous pair is adopted.
> The same result closes the **static quiescent membrane refill route**
> negative: zero changed-bond strain and no onsite load give `U=0`, so a
> positive port can copy once but cannot restore itself. The remaining price
> is physical power and control—preparation/ownership, stored strain or latent
> energy, local relative/environmental inflow or reserve, capacity,
> backpressure, replenishment, and production inverse.
> FTD-0998/0999 now closes that **abstract cumulative resource law** without
> changing currency. For a batch with receiver demand `D`, exact local
> formation release `U(F)`, causal boundary inflow `Phi`, and usable reserve
> `B`, conservation uniquely gives `B'=B+Phi+U(F)-D`; positivity requires
> atomic admission. Summation yields the finite-reserve and average-power
> bounds, and complete signed history closes the inverse. A restored catalyst
> contributes no net fuel. One-site work is additive only for independent
> supports, and remote reserve obeys the Moore delay. This retires the open
> **accounting-form price**, not the physical reservoir. The remaining debt is
> a native nonnegative reserve density and signed local current with
> phase-complete ownership, charging, routing, joint debit, replenishment,
> backpressure, refill coupling, reverse transport, and production realization.
> No new type or v1 currency is minted.
> In particular, v2 adopts measurement independence and supersedes, on that
> branch only, v1's FTD-0329 measurement-dependence commitment.

## 1 · The currency

Every type FTD touches falls into one column. The organizing fact (FTD-0336): **the entire import surface is the *argument* half of the modulus/argument frontier** — the odd / asymmetric / branch-selecting / chosen-adjoint half that a finite, discrete, deterministic, forward-only substrate provably cannot self-supply. The **self-set** column is the *modulus* half — the even / self-adjoint / tracial / forced-magnitude half the substrate owns.

| column | meaning | unit |
|---|---|---|
| **self-set** | the ontology grounds it (postulates + forced results) | — (credit) |
| **adopted-bit** | a binary structural choice the substrate cannot force, but FTD *adopts* | bit |
| **selected-type** | a declared selection among discrete alternatives | selected type |
| **named-result** | an external mathematical result the spine leans on (proven-elsewhere or open) | external theorem / open conjecture |
| **calibration** | a dimensional scale anchor (A2: N is grade-0, so *all* dimension is imported here) | dimensional anchor |
| **empirical-identification** | a "this math *is* this physics" bridge | identification |
| **declined** | an import FTD *refuses*, staking the substrate's prediction instead — a bet, not a debt | — (risk) |

## 2 · Self-set — the credits (the modulus half the ontology owns)

| ref | type | status | why it is self-set |
|---|---|---|---|
| SS-1 | the five postulates P1–P5 | [AXIOM] | the ontology's own smallest-honest type set — the context, chosen first |
| SS-2 | FC-0 / native complex structure, `i = √−1` | [AXIOM] | `i` is a native generator (FTD-0244) — the *self-set counterpart* of the imported δ |
| SS-3 | abstract constant `G* := Γ(1/4)/Γ(3/4)` and its equivalent identities | [DEFINITION + THEOREM IDENTITIES; PHYSICAL REALIZATIONS CONDITIONAL] | The mathematical identities stand. FTD-0839 corrects the FQCR overreach: FC-0/`i` alone does not force the twisted circle, chiral half-line, origin, scale, operator order, or multiplicity. The v2 clock/gate role is separately selected in its branch overlay. |
| SS-4 | the master quadratic (pure algebra) | [THEOREM] | `x² − 16G*²x + 16G*³ = 0` follows from the substrate's own structure |
| SS-5 | `N_c = 3` | [THEOREM] | RG flow + topological quantization, four routes |
| SS-6 | the theorem-grade algebraic spine (7 of 9) | [THEOREM] | the even/self-adjoint/tracial half of FTD-0336 |

## 3 · The import ledger — the debts, priced

### 3.1 Adopted bit (𝔹) — total: **1**

| ref | imported type | price | tag | falsifier |
|---|---|---|---|---|
| IMP-B1 | the **δ branch**: a ℤ/2 twist realizing `√(G*(4G*−1))`, breaking the root-swap `x₊ ↔ x₋` | **1 bit** | [AXIOM] (FC-W) | a native **forced** ℤ/2 carrier exhibited ⇒ FC-W superseded by a derivation (constitution FC-W kill 1); or δ shown ∈ N (FTD-0369 REFUTED) |

This is the single most important line in the ledger: FTD's one *adopted* structural import (constitution §1: "the framework's one adopted import, vs the declined M / reversibility"). The substrate provably cannot reach δ (FTD-0369/0370); FC-W adopts the one bit that supplies it.

### 3.2 Selected types (𝕊) — total: **4**

| ref | imported type | tag | falsifier |
|---|---|---|---|
| IMP-S1 | spatial dimension **D = 3** | [SELECTION — declared] (FTD-0355) | a forcing proof (→ self-set); or an equally-consistent alternate D (confirms free choice). **✓ constitution reconciled 2026-07-05** (§1.4 + §3.2 now read [SELECTION — declared]; the RF-1 drift is closed — see §6) |
| IMP-S2 | the **singlet** (J→ψ complexification for Bell/Tsirelson) | [SELECTION] | a native forced singlet; or a native S>2 on the engine (an FC-1 falsifier) |
| IMP-S3 | the **ℭ generator-set** representativeness (N_calc) | [SELECTION] (FTD-0347 flag) | a canonically forced generating set retires the flag |
| IMP-S4 | the **gauge-connection carrier choice** for imported charged matter: global projected `A_μ=𝒫_TJ_μ` or an independently adopted local link `A_μ` with plaquette dynamics | [SELECTION] (`SPEC_WILSON_DIRAC_FTD.md` §6; FTD-0416 locality ceiling; FTD-0417 local branch; minted 2026-07-12, broadened 2026-07-22) | a forcing theorem uniquely derives the carrier from P1–P5 (retires the row), or matched projected-link versus independent-link protocols show inequivalent vertex phenomenology (falsifies at least one branch); every vertex result carries its branch conditional |

### 3.3 Named results (ℂ) — total: **5** (2 proven-external, 3 open)

| ref | imported result | status | tag | falsifier |
|---|---|---|---|---|
| IMP-C1 | **Chudnovsky 1976** → the model ℚ(G*,π) ≅ ℚ(t,u) | proven (external) | [EXTERNAL — proven] | n/a (proven); the *import* is the spine's total dependence on it |
| IMP-C2 | **CM class-number h=1** uniqueness | open beyond h=1 | [NUMERICAL FACT, h=1 only] | an h≥2 curve reproducing the identity; or a structural all-h proof |
| IMP-C3 | **E1** (SC/FCC Watson-class independence) | open — **a case of multi-curve Chudnovsky** (strictly weaker than Rohrlich–Lang; priced FTD-0376, sharpened FTD-0377: per-constant floor CLOSED — {π, W} independent for each Watson constant individually via the disc −24 reduction; only the cross-disc joint independence is open; canonical marker Bertrand 1997) | [OPEN — exported P1] | a proof either way re-adjudicates FTD-0369 |
| IMP-C4 | **E\*/E\*\*** (no admissible period has (4G*−1) in its square class; subsumes E2) | open — E2 leg **a case of the exponential period conjecture** (Fresán–Jossen 8.2.6; priced FTD-0376; sharpened FTD-0378: individual transcendence of H_σ(τ) CLOSED unconditionally for SC/BCC symbols via Siegel–Shidlovskii — only independence-from-{π,Γ(1/4)} remains, behind the (e,π) wall) | [OPEN] | FTD-0353 §8 shared falsifier (forced native output, odd (4t−1)-valuation → REFUTED) |
| IMP-C5 | **the B-spline de Rham complex** (isogeometric compatible discretization: Buffa–Sangalli–Vázquez 2010, Buffa–Rivas–Sangalli–Vázquez 2011; FEEC framework: Arnold–Falk–Winther 2006; charge-conserving spline deposition: Villasenor–Buneman 1992 / Esirkepov 2001; coefficient-level assembly: GEMPIC 2017) — the quadratic-coat representation layer (FTD-0541–0551) is its `p=2` uniform-periodic instance, independently re-derived (FTD-0568) | proven (external) — **reconciliation anchor**: the coat's in-house identities keep their own proofs; the row prices the machinery's *provenance* (a selected standard framework), not a conditionality | [EXTERNAL — proven; anchors the FTD-0541/0550 [SELECTION] rows] | a coat identity with no counterpart in the cited complex, or a structural mismatch beyond relabeling in the space/degree correspondence (retracts the anchor; coat tags unaffected) |

> **Period-conjecture frame (FTD-0375).** The transcendence content of IMP-C1 — and the general δ∉N that IMP-C3/C4 leave open — is restated period-conjecture-relative in [`MATH_PERIOD_IMPORT_FRONTIER.md`](../09_mathematical/number_theory/MATH_PERIOD_IMPORT_FRONTIER.md): IMP-C1's Chudnovsky input *is* the Grothendieck period conjecture proven for the single CM motive `h¹(E_lemn)` (trdeg = dim = 2); IMP-C3/C4 are exactly the open assumptions (E1/E2) above which δ∉N carries beyond Chudnovsky. That node **re-prices nothing here** — it imports nothing new, so this table is unchanged.

### 3.4 Calibrations (𝕂) — total: **3** (A2: N is grade-0, so *every* dimensional prediction pays here)

| ref | imported anchor | tag | falsifier |
|---|---|---|---|
| IMP-K1 | `a_phys ≡ ℓ_P` (length) | [IMPOSED — calibration] | a substrate derivation of `a_phys` (Mechanism-γ **closed negative** — currently genuinely imported) |
| IMP-K2 | `t_phys = ℓ_P/(√3·c)` (time) | [DERIVED-CONDITIONAL from IMP-K1 + selected c_lat] | not an independent dimensional import: fixed by `a_phys` (IMP-K1) + the selected transport value `c_lat = 1/√3` + physical `c`. The exact production-stencil CFL ceiling is `c_lat²≤3/4`, so CFL does not force this selection (FTD-0407). Corrected 2026-07-08 from `√3·ℓ_P/c`; see `DERIV_DIMENSIONAL_GATE.md` |
| IMP-K3 | `K_B = m_e` (mass) | [IMPOSED — calibration] | disentangling the FTD-0130 role-conflation so the anchor is forced |

> **Gauge note.** The three rows above price the **legacy Planck-primary** gauge (FTD-0041). Under the **default electron-primary** gauge (`FOUND_ELECTRON_PRIMARY_GAUGE.md`, FTD-0137 §4.5; adopted 2026-07-08), IMP-K1 (length) is also **derived** — `ℓ_P` follows from `{ℏ, c, m_e}` via the predicted `m_e/m_P = Kα¹¹` ladder — so the beyond-universal import surface collapses to the **single** anchor `m_e` (IMP-K3), and `G` becomes a 0.38% *output* rather than a hidden import. Under the default, IMP-K1 reads `[DERIVED ~0.19%]` and IMP-K2 stays derived; only IMP-K3 (`m_e`) is a genuine import. Same falsifiable spine either way (gauge-invariant, FTD-0137).

### 3.5 Empirical identifications (𝔼) — the largest debt surface

| ref | imported bridge | price | tag | falsifier |
|---|---|---|---|---|
| IMP-E1 | **`x₊ = 1/α`** (the large root *is* 1/α) | 1 identification | [STRONGLY MOTIVATED CONJECTURE] | α measured to disagree beyond tree-level tolerance (FC-W kill 3) |
| IMP-E2 | the **parametric-insertion catalog** (mass formulas, gauge ratios, Higgs, proton, Lamb, g-2, …) | ~131 rows | [PARAMETRIC] | per-row: the borrowed standard formula is the falsifiable object; FTD supplies only the number |
| IMP-E3 | **adopted external physics** (QM's Born/Tsirelson *given* the singlet, multi-loop QED functionals, GR) | ~50+ | [IMPORTED — external] | each is the established physics's own test; FTD imports the framework, not the falsifier |

> **Pointer (2026-07-10, no new line):** the vertex program's imposed coupling — g²_vertex ≡ 1/x₊ wired into Branch-B Wilson–Dirac matter (`../10_eft_program/scopes_and_specs/SCOPE_VERTEX_PROGRAM.md` §2) — is the **composition IMP-E1 ∘ IMP-E3**, already priced above; a separate line would double-count. Its δ-branch content is inherited from IMP-B1 (no new ramification act). Retirement path: an ARC-3 closure of `SPEC_ALPHA_READOUT_CONTRACT.md` moves the coupling to the self-set column. The Branch-A alternative (native fermion emergence, which would have made the vertex partially self-set) is CLOSED NEGATIVE at the protocols tested per FTD-0379/0380.
> **Priced 2026-07-12 as IMP-S4 (§3.2):** the vertex's gauge-connection identification **A_μ = 𝒫_T J_μ** ([SELECTION], `SPEC_WILSON_DIRAC_FTD.md` §6) had been flagged 2026-07-10 (redteam finding) as a selected type with no priced row; it is now the ledger's 4th selected type with the drafted falsifier of record. The charter-§2 conditional-carry note is retired by the pricing — vertex-program results now cite IMP-S4 directly.
>
> **FTD-0417 reconciliation (2026-07-22):** the independent local-link
> candidate is the mutually exclusive second value of the same connection-
> carrier selection. IMP-S4 is broadened to cover both branches; no IMP-S5 is
> minted because that would double-count one carrier decision. The local branch
> adds a continuous link field and plaquette action, but does not add another
> simultaneously consumed connection type.

## 4 · Declined — the bets (imports FTD refuses, at falsification risk)

Not every argument-half type is paid for. Two, FTD **declines** — and stakes the substrate's prediction where the imported structure would differ. These are the opposite of debts: they are falsifiable wagers.

| ref | declined import | commitment | independence | the wager |
|---|---|---|---|---|
| DEC-1 | the **measurement map M** (non-commutative formalism, Born, operator observables) | FC-1 | [THEOREM] (FTD-0243: substrate cannot generate the formalism) | FTD predicts the substrate where M differs (Born statistics, S>2) — *more falsifiable, not more general* |
| DEC-2 | **global reversibility** (metric as fundamental) | FC-2 | arrow native; metric emergent-IR | the readout `R` is lossy (FTD-0394); the current full update `F` is non-injective via evaporation (FTD-0395); FC-2 remains the declaration that generalizes the arrow beyond that scoped witness |

## 5 · The import surface — the "ontology score," done honestly

There is **no single headline number**, and building one would be the abuse §0 warns against. The honest score is the *shape* of the ledger, and it has two clean readings:

**Reading 1 — the import surface is exactly the argument half.** Every debt above is an argument-half type of FTD-0336; the self-set column (§2) is the modulus half in full. The framework grounds the even/forced/tracial half of its mathematics and imports the odd/chosen/branch-selecting half — and of that half it **adopts exactly one bit** (FC-W), **declines two** structures (M, reversibility) at falsification risk, **calibrates three** dimensional scales, **leans on five** external results (two proven, three open — one of the proven, IMP-C5, a reconciliation *anchor* rather than a load-bearing conditionality), and **identifies its empirical physics** at [SMC]/[PARAMETRIC] tags. That distribution *is* the boundary.

**Reading 2 — the cost is stratified, and cheapest where the framework is strongest.** The import bill depends on how far into physics you go:

| what you want | additional imports paid | running tag ceiling |
|---|---|---|
| the **dimensionless algebraic spine** (G*, master quadratic, D=3-conditional results) | IMP-C1 (Chudnovsky) + IMP-C2 (CM h=1) + IMP-B1 (the δ bit) | [THEOREM] core + 1 bit |
| the **dimensionless physical predictions** (α, mass ratios) | + IMP-E1 (`x₊=1/α` [SMC]) + IMP-C3/C4 (E-family, for the boundary theorems) | [SMC] |
| **dimensional** physics (masses in MeV, lengths in m) | + IMP-K1/K2/K3 (the 3 calibrations) | [IMPOSED] |
| the **full empirical catalog** | + IMP-E2 (~131 [PARAMETRIC]) + IMP-E3 (~50+ external) | [PARAMETRIC] |

The dimensionless spine is the cheapest and the most self-set; the price rises monotonically as the claims become more physical — exactly the stratification `SPEC_DIMENSIONAL_MAP.md` draws, now expressed as a bill. This is the quantitative form of the project's honest altitude: *a philosophy-of-mathematics project with a rigorous algebraic core (cheap, self-set) and suggestive physics connections (progressively imported).*

## 6 · Reconciliation flags (drift the ledger surfaces)

Building the ledger against the canonical sources surfaced live drift. These are **flagged, not fixed** — editing the constitution is out of scope for this accounting (and deferred per the program charter); they are logged for a future owner-approved reconciliation.

- **RF-1 — D=3 status drift — RESOLVED 2026-07-05.** The constitution (`SPEC_FTD_FRAMEWORK_V1.md`) formerly listed `D = 3` as **"Forced [THEOREM]"** at **§1.4 (line 82) and §3.2 (line 159)** — the original flag misattributed the location as "§3.3"; the actual sites were §1.4 + §3.2. FTD-0355's permanent verdict demoted D=3 to **[SELECTION — declared]** (bounded search, circularity named), and per LEDGER > constitution the ledger always priced D=3 as a *selected type* (IMP-S1). **The drift is now reconciled:** both constitution rows read [SELECTION — declared] (FTD-0355), and the corpus-wide "D=3 forced" residue was swept in the same 2026-07-05 pass (see the LEDGER maintenance log). The verifier (`proof_import_ledger.py` C7) now asserts the *resolved* state — the constitution's D=3 row reads [SELECTION] with no surviving "Forced [THEOREM]" for D=3.
- **RF-2 — unpriced [SELECTION] rows in the FTD-0908–0987 range — PARTIALLY RESOLVED this pass; 3 remain OPEN for the owner.** Per `LEDGER_INDEX.md`, the native-time-carrier/clock-carrier programme range FTD-0908–0987 carried **7 rows independently tagged [SELECTION]**: FTD-0957, FTD-0959, FTD-0963, FTD-0974, FTD-0980, FTD-0982, FTD-0987. A dedicated pricing-investigation pass (2026-08-13) reviewed each against IMP-S4's scope and the §0 absorption pattern. **4 are now resolved as absorbed, no new currency**, and carry an explicit §0 running-log entry above matching the FTD-0901/0903/0904/0990 style: **FTD-0963** (positive complete-square phase-connection, assembled from already-priced canonical pairs), **FTD-0974** (minimum faithful positive canonical suspension; corroborated by the v2 register's own `OPEN-CA-TRANSDUCER` classification), **FTD-0982** (local canonical work-port/seam family, extending FTD-0980's seam handshake on already-booked type surface), and **FTD-0987** (regional common-mode ownership designation, matching the disposition of its own successor FTD-0988/0989). **3 remain an open reconciliation item for the owner**: **FTD-0957** (adopts a fresh two-scale synchronization Hamiltonian H_sync — reads as a genuine mint, not a continuation of prior selected content), **FTD-0959** (adopts a phase-lift-plus-integer-winding-history construction to buy exact isochrony — also reads as a fresh selected device, and its own [OPEN] tag on "native winding carrier" suggests the framework itself treats the winding object as import-candidate, not forced content), and **FTD-0980** (adopts a reversible ternary clock-seam clutch as "the minimum coherent reference law found here," explicitly framed as one choice among named un-derived alternatives — carries its own [SELECTION] tag rather than the pure-[THEOREM]-closure marker the absorbed rows carry). Each of these 3 needs an owner decision on whether it (a) mints new selected-type currency (its own IMP-S row, possibly one row bundling several of the native-time-carrier programme's synchronization/clock devices), or (b) is priced instead in the v2 branch overlay (`contextual_actualization_register_v2.json`) rather than as v1 currency. No pricing decision is made by this flag; §3.2's total of 4 selected types is unchanged pending that decision.

## 7 · Falsifiers, invariants, cross-references

Every import row above carries its own falsifier (verifier C2 enforces non-empty falsifiers on all 16 imports + 2 declined items) — that is what makes the boundary a *deliverable* and not a disclaimer. The sharpest single falsifier of the whole α-sector import stack is the FC-W one: **a forward-derived substrate object realizing `√(G*(4G*−1))` with a forced ℤ/2** would convert IMP-B1 from an adopted bit into a derivation, retire the largest line in the ledger, and upgrade `x₊ = 1/α` toward [SELECTED/DERIVED]. It is, in the constitution's words, "the one refutation FTD would welcome."

**Standing invariants:** x₊ = 1/α [SMC]; MC-T4.3 [FOUNDATIONAL OBSTRUCTION]; FC-W [AXIOM]; D=3 [SELECTION — declared]; no tag moves — this ledger prices existing commitments and introduces none.

**Cross-references:** `FOUND_MODULUS_ARGUMENT_FRONTIER.md` (FTD-0336 — the qualitative boundary this prices) · `SPEC_FTD_FRAMEWORK_V1.md` (the FCs and their falsifiers) · `SPEC_DIMENSIONAL_MAP.md` + `FOUND_DIMENSIONAL_GRADE_CLOSURE.md` (the calibration layer / grade-0 closure) · `CATALOG_PARAMETRIC_INSERTIONS.md` (the IMP-E2 detail) · `ANALYSIS_DELTA_IND_CLOSURE_v1.md` + `THEOREM_RAMIFICATION_LOCUS.md` + `FOUND_NATIVE_CLOSURE_REALIZABILITY.md` (why δ is imported and the E-family conjectures) · `SPEC_DOCTRINE_LEDGER.md` (FTD-0145 — the sibling status roll-up) · `TRACKER_ONTIC_TRUTH.md` (the bedrock tiers behind the self-set column).
