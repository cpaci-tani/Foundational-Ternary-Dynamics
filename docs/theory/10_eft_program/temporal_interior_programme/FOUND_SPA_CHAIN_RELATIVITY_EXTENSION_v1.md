# FOUND — The SPA Chain under Relativity and toward Quantum Gravity v1

- **Document class:** foundation synthesis
- **Epistemic tag:** [SYNTHESIS]
- **Maturity:** draft; no LEDGER row has been minted
- **Version:** 1.2
- **Created:** 2026-08-07
- **Last revised:** 2026-08-07
- **Canonical precedence:** the
  [LEDGER](../../07_assessment/core_ledgers/LEDGER.md) and the
  [FTD constitution](../../01_reference/SPEC_FTD_FRAMEWORK_V1.md) govern every
  FTD status quoted here; this document moves no tag
- **Parents:** the
  [Selection–Potentiality–Actualization chain](FOUND_SELECTION_POTENTIALITY_ACTUALIZATION_CHAIN_v1.md),
  [Observer's Completion Map](SPEC_OBSERVERS_COMPLETION_MAP_v1.md), and
  [Temporal Interior Programme charter](SCOPE_TEMPORAL_INTERIOR_PROGRAM_v2.md)
- **Canonical companions:** the
  [Wigner's-friend resolution](../../06_reference_frames_and_measurement/FOUND_WIGNERS_FRIEND_RESOLUTION.md),
  [reference-frame vocabulary](../../01_reference/REF_REFERENCE_FRAME_VOCABULARY.md),
  [Lorentz-recovery hard gate](../../07_assessment/lorentz_recovery_causal_structure/AUDIT_LORENTZ_RECOVERY_HARD.md),
  and
  [native clock-carrier specification](../native_time_carrier_programme/SPEC_CARRIER_CONSTRAINTS_v1.md)
- **Scope:** translate the SPA architecture into relativistic language,
  identify the additional assumptions required for local measurements, and
  state the preferred-foliation price of any FTD realization
- **Production impact:** none; no engine, postulate, framework commitment, or
  empirical result is changed
- **External-literature cutoff:** 2026-08-07
- **Supersession note:** version 1.2 replaces the scientific claims of version
  1.1 in this file while retaining the filename for provenance

---

## 0. Verdict at the correct strength

The SPA recursion,

\[
(H_n,\mathcal C_n,\mathcal D)
\xrightarrow{\Gamma}\rho_n
\longrightarrow\{(e_i,w_i)\}
\xrightarrow{K\;\mathrm{or}\;(F,\mu)}e_{n+1}
\longrightarrow H_{n+1},
\]

where the parent document marks the closure map \(\Gamma\) as [IMPOSED] and
the physical Born pushforward for \((F,\mu)\) as [OPEN].

contains two separable relativistic questions.

1. **Operational probability layer.** Relativistic quantum field theory
   supplies covariant local algebras, algebraic states, and causal measurement
   schemes. Conditional on those imported structures, probabilities for
   causally disjoint local instruments are independent of which admissible
   ordering is used to compose them. This is established external physics, not
   an FTD derivation and not a theorem of the SPA vocabulary.
2. **Single-outcome selector.** A law for the realized history must satisfy a
   further covariance or empirical-equivalence condition. Instrument
   no-signalling does not provide that law. The SPA selector remains [OPEN],
   and FTD has not constructed a selector whose history distribution is
   foliation independent.

Several representative response families are surveyed below. They are not an
exhaustive classification: hybrid, relational-event, retrocausal,
superdeterministic, covariant-histories, and other proposals do not fit a
proved three-way partition. In particular, hypersurface-relative state update
is a rule for conditional descriptions; by itself it is not a complete
ontology of actualization.

The FTD conclusion is deliberately asymmetric:

> FTD postulates an ontically real global tick order [AXIOM]. It has not shown
> that this preferred order is empirically hidden. FC-1 constitutionally
> declines the measurement-map import $M$ [AXIOM — FRAMEWORK-COMMITMENT
> CLASS]: Hilbert states, operator effects/instruments, and a noncommutative
> observable algebra are not FTD structures, whether labelled fundamental or
> observer-effective. What remains [OPEN] inside canonical FTD is a
> **commutative frame-relative readout** from substrate/genesis histories and
> laboratory protocol labels to classical records, response functions, and
> empirical distributions, together with observable Lorentz recovery and a
> selector-history law. AQFT supplies only an externally imposed calculation
> and comparison language. Matching its probabilities would not derive its
> algebra. A physical noncommutative bridge would be $M$ and would require an
> explicit FC-1 amendment.

---

# Part A — Relativistic operational probabilities

## 1. AQFT uses algebraic states, not a local density-matrix trace

Let \(O\mapsto\mathcal A(O)\) be a net of local unital
\(C^*\)- or von Neumann algebras. For spacelike-separated regions
\(O_A\) and \(O_B\), Einstein causality gives

\[
[\mathcal A(O_A),\mathcal A(O_B)]=0. \tag{R1}
\]

A state is a positive normalized linear functional
\(\omega:\mathcal A\rightarrow\mathbb C\). For a local effect
\(E\in\mathcal A(O)\), \(0\leq E\leq\mathbf 1\), its probability is

\[
p_\omega(E)=\omega(E).
\]

This is the representation-independent replacement for
\(\operatorname{Tr}(\rho E)\). In a genuinely type-I model, or when the
relevant state–effect pair is represented by a trace-class density operator
on \(B(\mathcal H)\), the familiar trace notation is recovered. Merely choosing
a normal representation of a type-III local algebra does not turn that
algebra into type I. Generic local AQFT algebras are often type III and do not
carry an intrinsic density-matrix trace. Therefore the
finite-dimensional trace-rule reconstruction in the parent SPA document does
not pass into local AQFT “unchanged.” What survives is the state–effect
pairing; normality, representation, regulator, and limiting assumptions must
be declared when trace language is used [1–3].

Microcausality is necessary but not, by itself, a complete relativistic
measurement theory. Arbitrary maps labelled “local instruments” can still
create impossible-measurement pathologies. The instrument must arise from, or
satisfy the axioms of, a causal local measurement scheme [3].

## 2. Hypersurface evolution is conditional on integrability

In a representation admitting an interaction-picture hypersurface state, the
Tomonaga–Schwinger equation is

\[
i\hbar\frac{\delta\Psi[\sigma]}{\delta\sigma(x)}
=\mathcal H_{\mathrm{int}}(x)\Psi[\sigma]. \tag{R2}
\]

Foliation-independent evolution between two hypersurfaces requires the
integrability or causal-factorization condition associated with

\[
[\mathcal H_{\mathrm{int}}(x),\mathcal H_{\mathrm{int}}(y)]=0
\quad\text{for spacelike }x,y,
\]

with the usual qualifications needed for renormalized interacting fields.
Thus (R2) is not a free-standing proof that every proposed potentiality
dynamics is covariant. It is an established external construction when its
field-theoretic and integrability assumptions hold [4]. AQFT may instead
remain in the Heisenberg picture and formulate the same operational content
without assigning an ontic state to every hypersurface.

## 3. Causal-local instruments: the actual order-independence statement

Use the Heisenberg picture. An outcome map
\(\mathcal J^{A*}_{a|x}\) is a normal completely positive map on observables;
the nonselective map
\(\mathcal J^{A*}_{x}=\sum_a\mathcal J^{A*}_{a|x}\) is unital. Define
\(\mathcal J^{B*}_{b|y}\) similarly.

For causally disjoint coupling regions, a causal-factorizing measurement
scheme requires the outcome maps to compose in either order:

\[
\mathcal J^{A*}_{a|x}\circ\mathcal J^{B*}_{b|y}
=
\mathcal J^{B*}_{b|y}\circ\mathcal J^{A*}_{a|x}.
\]

The joint distribution is consequently order independent,

\[
\begin{aligned}
p_\omega^{A\prec B}(a,b|x,y)
&=\omega\!\left[
  \mathcal J^{A*}_{a|x}\circ
  \mathcal J^{B*}_{b|y}(\mathbf 1)\right]\\
&=\omega\!\left[
  \mathcal J^{B*}_{b|y}\circ
  \mathcal J^{A*}_{a|x}(\mathbf 1)\right]
=p_\omega^{B\prec A}(a,b|x,y).
\end{aligned}\tag{R3a}
\]

Unitality then yields both no-signalling marginals,

\[
\sum_b p_\omega(a,b|x,y)
=\omega\!\left[\mathcal J^{A*}_{a|x}(\mathbf 1)\right],
\qquad
\sum_a p_\omega(a,b|x,y)
=\omega\!\left[\mathcal J^{B*}_{b|y}(\mathbf 1)\right]. \tag{R3b}
\]

Equations (R3a–b), not trace preservation alone, are the required operational
statement. Trace preservation of a later nonselective operation would make
one earlier marginal invariant even for a timelike operation; it says nothing
by itself about equality of the two spacelike orderings. The load-bearing
input here is causal factorization of the local measurement schemes [3].

## 4. Selector-history equivariance is a separate requirement

Operational order independence does not prove covariance of a hidden
single-outcome process. Let \((\Omega_f,\mathcal F_f)\) and
\((\Omega_g,\mathcal F_g)\) be measurable spaces of complete realized
histories for two admissible foliations \(f\) and \(g\), and let \(\mu_f\) and
\(\mu_g\) be the probability measures generated by the selector law. Assume
that \(\tau_{f\to g}:\Omega_f\to\Omega_g\) is measurable and exchanges only
the birth order of spacelike-incomparable events while preserving their
spacetime labels and outcomes.

A strong selector-history equivariance condition would be

\[
\mu_g=(\tau_{f\to g})_*\mu_f. \tag{R4a}
\]

That condition is stronger than empirical adequacy. If \(f_0\) is a physically
fixed preferred foliation, one need not define a counterfactual \(\mu_g\) at
all. Its operational hiding instead requires

\[
(\pi_{\mathcal E})_*\mu_{f_0}=P^{\mathrm{cov}}_{\mathcal E}
\quad\text{for every operational experiment }\mathcal E, \tag{R4b}
\]

where \(\pi_{\mathcal E}\) forgets unobservable history detail and retains the
recorded data, and \(P^{\mathrm{cov}}_{\mathcal E}\) is the target covariant
operational distribution. If a candidate law is defined for every admissible
foliation, equality of these pushforwards for all \(f,g\) is a useful stronger
comparison test. Equation (R4a), together with a readout map compatible with
\(\tau\), gives that equality; neither operational agreement nor (R4b) implies
equality of hidden histories.

Neither equation follows from (R3). A deterministic substrate also does not
obtain \(\mu_f\) for free: the measure must come from a specified distribution
over preparations or initial conditions, or from an explicitly added
stochastic law. For SPA and FTD, construction and testing of that law are
[OPEN].

---

# Part B — Representative responses to relativistic actualization

## 5. Route 0 — no fundamental selector

Unitary relativistic QFT does not add a singular actualization map. Everettian
or other branching descriptions place the burden on an account of records and
Born weights rather than on a covariant collapse law [8,9]. This route removes
the specific selector-covariance problem; it does not constitute an FTD result,
and it does not make contested probability arguments into theorems.

## 6. Route I — hypersurface-relative conditional state assignment

One may assign different conditional states to different hypersurfaces,
according to which recorded events lie in their past. Aharonov–Albert and
later analyses show why a single instantaneous global collapse surface is not
required for consistent predictions [5–7].

This route is best classified as **update semantics**. It can be combined with
different ontologies and does not, on its own, specify:

- which event becomes actual;
- the probability measure over actual events;
- whether actual events are foliation independent; or
- whether a hidden foliation is empirically invisible.

It therefore cannot close the SPA selector problem by conditionalization
alone.

## 7. Route II — covariant stochastic flashes

Following Bell's proposal to treat GRW jumps as spacetime-local beables [10],
relativistic GRW-flash models show that stochastic spacetime events and
relativistic covariance can coexist. Tumulka's 2006 model treats a fixed
number of noninteracting distinguishable particles [11]. The status changed
in 2020: Tumulka constructed a relativistic flash process for a fixed number
of **interacting distinguishable particles**, starting from a given
interaction-local Tomonaga–Schwinger evolution [12].

The remaining scope gap is therefore not “all interacting relativistic GRW.”
The 2020 construction does not supply a model for indistinguishable
particles, variable particle number, or a full interacting quantum field
theory. It is an existence result in its stated fixed-\(N\) domain, not a
completed relativistic collapse theory for the Standard Model.

## 8. Route III — a real preferred foliation

A selector may act in a genuine hidden order, as in preferred-foliation
formulations of nonlocal hidden-variable theories. This makes a total
actualization order well defined. It does **not** make the order empirically
hidden by stipulation.

The price is two independent proofs:

1. the selector law must satisfy (R4b), or an experimentally adequate
   approximation to it; and
2. every observable sector must pass the ordinary Lorentz-recovery tests for
   dispersion, common limiting cones, interactions, radiative stability,
   unitarity/Ward identities, and composite clocks and rods.

Equation (R3) supplies neither proof. Preferred foliation is a logically
available ontology; empirical invisibility is a dynamical result that must be
earned.

These routes are representative families, not mutually exclusive and
exhaustive alternatives. For example, hypersurface-relative state assignment
can accompany either flashes or a preferred-foliation ontology.

---

# Part C — Causal-set growth as an optional diagnostic

## 9. From a sequence to a partial order

If, and only if, a candidate theory supplies a coarse-graining from its
histories to locally finite causal sets, one may replace a total event
sequence by

\[
C=(\{e_i\},\prec)
\]

and consider a growth rule

\[
C_n\longrightarrow
\{\text{admissible births }C_n\to C_{n+1},\,
t(C_n\to C_{n+1})\}
\xrightarrow{\Sigma}C_{n+1}.
\]

This is a possible formalization, not the unique relativistic form of the SPA
recursion. FTD presently has no canonical map from a cubic-lattice tick
history to an unlabeled causet of actualization events.

## 10. Rideout–Sorkin conditions and the correct gauge statement

Rideout–Sorkin classical sequential growth uses four load-bearing
requirements [13,14]:

1. **Internal temporality:** a new element is born maximal, never to the past
   of an existing element.
2. **Markov sum rule:** the transition probabilities from each finite causet
   are normalized.
3. **Discrete general covariance:** the probability of obtaining a given
   unlabeled finite causet is independent of the labeled growth path.
4. **Bell causality:** after deleting spectators, the ratio of two competing
   transition probabilities is unchanged,
   \[
   \frac{t(C\to C_1)}{t(C\to C_2)}
   =
   \frac{t(B\to B_1)}{t(B\to B_2)},
   \]
   where \(B\) contains the union of the relevant precursor sets. The basic
   ratio statement assumes nonzero denominators; forbidden transitions
   require the paper's zero-transition extension.

For two labeled paths \(\gamma,\gamma'\) leading to the same unlabeled causet,
discrete general covariance includes the path-independence condition

\[
\prod_{r\in\gamma}t_r
=
\prod_{r\in\gamma'}t_r. \tag{R5}
\]

The gauge distinction is precise:

- the **birth label assigned to a particular element**, or equivalently the
  ordering chosen among incomparable births, is gauge;
- the stage number \(n=|C_n|\), the cardinality of the finite causet, is
  label invariant and is not gauge, although it is not a Lorentz coordinate
  time.

Under the stated classical assumptions the generalized-percolation family is
classified. A quantum sequential-growth dynamics with an adequate quantum
analogue of these conditions remains an open research programme [15,30].

### What (R5) does not establish

Equation (R5) is an **optional causet diagnostic**. It is neither necessary nor
sufficient for hiding FTD's preferred cubic lattice. It does not constrain
lattice dispersion, cubic anisotropy, mismatched matter/flux/gravity cones,
lower-dimensional preferred-frame operators, radiative mixing, or operational
clock/rod boosts. Even a successful FTD-to-causet map satisfying (R5) would
leave those independent Lorentz gates untouched.

Accordingly, (R5) is not the target of FTD Lorentz recovery. It becomes
relevant only after a causal-set coarse-graining is proposed, at which point
its status is [OPEN] until the four classical conditions, or justified quantum
replacements, are checked.

---

# Part D — Indefinite order and relational clocks

## 11. Process matrices and the 2026 experimental status

For finite-dimensional operational laboratories, the process-matrix
framework assigns probabilities

\[
P(a,b|x,y)
=\operatorname{Tr}\!\left[
W\left(M^A_{a|x}\otimes M^B_{b|y}\right)\right]. \tag{R6}
\]

Some processes are causally nonseparable [16]. The quantum switch is a
realized coherently controlled order and is causally nonseparable, but the
standard switch alone does not violate the original causal inequalities
[17,29]. Causal nonseparability, a causal-inequality violation, and a
loophole-free device-independent certification are distinct claims.

The empirical status is no longer “device-independent indefinite order
unobserved.” In 2026 Richter et al. reported the first implementation of a
device-independent protocol and a violation of its definite-order bound
[18]. The experiment explicitly retains loopholes. It is therefore evidence
toward device-independent certification, not loophole-free confirmation that
fundamental spacetime order is indefinite.

The SPA gloss that \(\Sigma\) might actualize an ordering is [CONJECTURE]. No
selector law, history measure, or FTD implementation follows from the process
matrix formalism.

## 12. Page–Wootters clocks do not replace FTD's ontic tick

In canonical quantum gravity, the Wheeler–DeWitt constraint has the schematic
form

\[
\widehat H\Psi=0. \tag{R7}
\]

This is a problem of time in a particular canonical quantization setting, not
a theorem that every quantum-gravity approach lacks causal order. The
Page–Wootters construction recovers relational conditional states by using a
clock subsystem \(K\), for example

\[
\rho_S(t)=
\frac{\operatorname{Tr}_K[
(\Pi_t^K\otimes I)\rho_{\rm total}(\Pi_t^K\otimes I)]}
{\operatorname{Tr}[
(\Pi_t^K\otimes I)\rho_{\rm total}]}. \tag{R8}
\]

Kuchař raised localization and propagator objections to the original
conditional-probability proposal [21]. Höhn, Smith, and Lock resolve the
specific relativistic-particle localization objection, separately within
each frequency superselection sector, by conditioning on a covariant clock
POVM [27]. That scoped result is not a universal answer to every multi-time or
propagator objection. For measurement sequences, Hausmann, Schmidhuber, and
Castro-Ruiz show that two consistent Page–Wootters measurement schemes agree
for ideal clocks but can diverge for nonideal finite-resource clocks [28].
Sequential statistics are therefore formulation- and clock-assumption
dependent; interacting and gravitationally backreacting clocks require
further work [19–22,27,28].

FTD is logically different. P2 already postulates the global update index
\(n\) as ontic succession [AXIOM]. FTD does not need a clock carrier in order
for its update map to be indexed. What the
[Temporal Interior Programme](SCOPE_TEMPORAL_INTERIOR_PROGRAM_v2.md) and
[carrier specification](../native_time_carrier_programme/SPEC_CARRIER_CONSTRAINTS_v1.md)
seek is an internal physical subsystem that converts succession into
operational duration and supports comparisons among clocks and rods. A
Page–Wootters clock is an imported relational construction; it neither derives
nor removes the FTD tick.

---

# Part E — Gravity-related collapse and events-first analogies

## 13. Diósi–Penrose: hypothesis class and current bounds

Penrose's heuristic collapse-timescale estimate is commonly written

\[
\tau_{\rm DP}\sim\frac{\hbar}{E_G}, \tag{R9}
\]

where \(E_G\) is a gravitational self-energy difference. Diósi's stochastic
dynamics is a more specific model and requires a short-distance
regularization. The heuristic timescale and the Markovian white-noise DP
model must not be treated as one parameter-free theorem [23].

The experimental status is:

- Donadi et al. (2021) excluded the natural parameter-free DP version in the
  spontaneous-radiation model they tested [24].
- XENONnT found no significant excess and, for the Markovian white-noise DP
  model, reported \(R_0>4.9\times10^{-10}\,\mathrm m\) at 90% confidence,
  improving the previous lower limit by about a factor of five [25].
- The XENONnT result constrains that parameterized Markovian model; it does not
  exclude every colored, dissipative, or otherwise modified gravity-related
  collapse proposal.

The resemblance between a gravity-linked collapse timescale and a slow
actualization gate is [CONJECTURE — SPECULATIVE ALIGNMENT]. It gives no
evidence that FTD latency is a collapse noise, a Born selector, or a covariant
history law.

## 14. Events-first programmes are analogies, not inheritances

Causal sets, quantum measures over histories, and energetic-causal-set
programmes contain discrete events, partial orders, or real becoming
[13–15,26]. These are useful comparison classes for SPA terminology. They do
not thereby instantiate the SPA selector, and SPA does not solve their
dynamics. Any claimed bridge must specify:

1. the map from the source theory's configurations to SPA events;
2. the state/effect/instrument or quantum-measure structure;
3. the probability or selection law; and
4. the covariance and empirical tests preserved by the map.

Until such a bridge exists, “events first” is
[CONJECTURE — STRUCTURAL ANALOGY] only.

---

# Part F — The FTD boundary

## 15. Preferred-foliation honesty

The [FTD constitution](../../01_reference/SPEC_FTD_FRAMEWORK_V1.md) fixes:

- P2: global discrete ticks and absolute substrate simultaneity [AXIOM];
- P4: one-tick Moore-neighbour dependency [AXIOM];
- P5: a deterministic update map [AXIOM]; and
- FC-2: a native arrow with reversibility only sector scoped
  [AXIOM — FRAMEWORK-COMMITMENT CLASS].

Thus FTD is a preferred-tick ontology. The constitution does **not** establish
a quantum actualization selector. If a future FTD selector acts at the tick
level, it will be a preferred-order realization in the sense of Route III;
that conditional statement must not be shortened to “FTD has derived
relativistic collapse.”

Nor is the foliation presently known to be hidden. The
[hard Lorentz audit](../../07_assessment/lorentz_recovery_causal_structure/AUDIT_LORENTZ_RECOVERY_HARD.md)
records:

- a dimension-six preferred-frame term in the default free-flux pole;
- the selected production equality
  \(C_{\rm WAVE}=C_{\rm SPEED}=1/\sqrt3\), not a uniquely forced CFL
  saturation [SELECTION];
- only scoped free-sector improvements;
- no established live interacting common cone; and
- physical Lorentz invariance [OPEN].

Its LR-0 through LR-6 contract remains the recovery criterion: full poles,
stable improvement, a common cone across all sectors, interacting matching,
Ward/unitarity compatibility, SME phenomenology, and operational composite
boosts. Neither AQFT equation (R3) nor causal-set equation (R5) bypasses that
contract. Gravity also remains separately limited by the canonical status of
the \(g_{00}\) construction and the [OPEN] substrate spin-2 carrier; nothing
in this synthesis upgrades it.

## 16. Ontic ticks, internal clocks, and imposed latency

Three notions must remain separate:

| Notion | FTD status | What it supplies |
|---|---|---|
| global update index \(n\) | [AXIOM] via P2 | ontic succession and preferred order |
| internal clock carrier | [OPEN] at the programme level | operational duration, synchronization, and clock/rod comparisons |
| latency field \(L(x)\) | [IMPOSED] | an engine time-rate/gravity-potential sector |

The latency field is recorded as [IMPOSED] in the
[constitution](../../01_reference/SPEC_FTD_FRAMEWORK_V1.md) and
[Lagrangian specification](../../01_reference/SPEC_FTD_LAGRANGIAN.md).
Whether it can participate in an actualization mechanism is [OPEN]. At
minimum a proposal must state its stochastic or deterministic source, its
coupling to candidate events, its separation of timescales, its Born-law
falsifier, and its history-equivariance and Lorentz tests. Calling latency
“the native slow gate” before those items exist would promote an imposed
engine sector into an unearned mechanism.

## 17. What the Born campaigns actually establish

The two 2026-08-07 campaigns have narrow, non-transferable scopes.

1. The
   [Born-fraction saturation scan](../preregistrations/quantum_foundations/PREREG_BORN_DENSITY_SATURATION_v2.md)
   reports a **pre-registered mechanism-level measurement with no canonical
   tag move**, using an [IMPOSED] ensemble on a quick-check platform. Within
   that particular threshold model, the pre-registered Born-fraction statistic
   increased monotonically from \(0.049\) to \(0.836\) as the
   fast-mode/slow-noise control parameter increased. The largest arm did not
   reach exact Born weighting; whether the asymptote is \(1\) or approximately
   \(0.86\) remains unresolved. The run demonstrates a sufficient trend in
   the tested toy mechanism, not a universal theorem that Born weighting
   requires slow gating.
2. The
   [engine regime-map campaign](../preregistrations/quantum_foundations/PREREG_BORN_REGIME_MAP_ENGINE_v1.md)
   contains an **unbooked draft execution** under an [IMPOSED] Langevin
   thermostat. It reports Outcome N at its declared draft scope: none of the
   declared axes reached the transfer threshold. This draft result moves no
   tag and cannot be cited as a registered measurement claim. At most, it
   provisionally disfavors transfer of the toy regime law to the native
   thermal field in the sampled domain. The claim that flux-borne noise “can
   never” supply the regime is a post-hoc diagnosis, not the declared draft
   outcome and not a no-go theorem.

No campaign tested latency, a relativistic selector, (R4), or exact Born
weights in the production engine. Those remain [OPEN] and require fresh
pre-registrations.

## 18. Minimum acceptance contract for an FTD selector

FC-1 declines the map $M$ from the commuting substrate algebra to
noncommutative quantum measurement structure without a fundamental/effective
loophole. AQFT states, effects, and instruments may therefore be used here only
as **[IMPOSED — EXTERNAL REPRESENTATIONAL BOOKKEEPING]** and empirical
comparators; they are not physical FTD observables and are not claimed to
emerge by coarse-graining. Any proposal that assigns them that physical role
must declare an explicit revision or extension of FC-1 rather than presenting
it as canonical FTD. Under the canonical FC-1 branch, an FTD realization of
the SPA selector is not scientifically complete until it supplies all of the
following:

1. a commutative map from finite-support FTD configurations, observer access
   restrictions, and laboratory protocol labels to a Boolean record algebra,
   classical response functions, settings, and event probabilities;
2. a single-outcome rule compatible with P5, or an explicitly declared
   stochastic import;
3. normalized joint distributions for separated instruments;
4. the commutative record-level analogue of the operational
   causal-factorization test (R3);
5. the independent history-equivariance or empirical-pushforward test (R4);
6. a non-fitted empirical-frequency campaign outside the imposed toy
   ensemble, with the external Born/AQFT probabilities declared only as the
   comparison target;
7. all applicable LR-0 through LR-6 Lorentz gates; and
8. an empirical signature or falsifier distinguishing the realization from
   ordinary relativistic quantum theory.

A causet coarse-graining and (R5) may be added as an optional ninth diagnostic.
They cannot replace items 4–7.

# Part G — Status ledger

| Claim | Status at this revision |
|---|---|
| AQFT probabilities use the state–effect pairing \(\omega(E)\) | [IMPOSED — IMPORTED AQFT FORMALISM], not FTD-derived |
| AQFT states/effects/instruments as physical FTD structures | [AXIOM — FRAMEWORK-COMMITMENT CLASS]: declined by FC-1; importing them is $M$ and requires an explicit framework amendment |
| Canonical FTD observer readout | [OPEN] only as a commutative, frame-relative map to classical records and empirical distributions; matching AQFT probabilities would not derive AQFT's algebra |
| Generic local AQFT probabilities need not admit an intrinsic density-matrix trace | [THEOREM — EXTERNAL, CONDITIONAL] under the standard type-III local-algebra assumptions |
| Tomonaga–Schwinger evolution is foliation independent | [THEOREM — EXTERNAL, CONDITIONAL] on integrability/causal factorization |
| Spacelike instrument joint probabilities are order independent | [THEOREM — EXTERNAL, CONDITIONAL] on causal-factorizing local measurement schemes, (R3) |
| Operational no-signalling (R3) does not entail selector-history equivariance (R4) | [SYNTHESIS — LOGICAL SEPARATION]; R3 contains no selector law or history measure, and both remain [OPEN] |
| The route list | [SYNTHESIS — NON-EXHAUSTIVE TAXONOMY]; no classification theorem is claimed |
| Relativistic GRW with interaction exists | [THEOREM — EXTERNAL, CONDITIONAL] for fixed \(N\), distinguishable particles given interaction-local hypersurface evolution [12] |
| Relativistic GRW for indistinguishable particles, variable number, or full QFT is complete | [OPEN] — those external scopes remain unresolved |
| Rideout–Sorkin classical sequential growth | [THEOREM — EXTERNAL, CONDITIONAL] under internal temporality, Markov normalization, discrete general covariance, and Bell causality |
| Birth labels in causal-set growth | [THEOREM — EXTERNAL] gauge statement for incomparable events |
| Stage \(n=|C_n|\) in causal-set growth | [THEOREM — EXTERNAL] label-invariant cardinality; not gauge and not coordinate time |
| (R5) in FTD | [OPEN — OPTIONAL CAUSET DIAGNOSTIC] only; it does not replace the separately [OPEN] Lorentz-recovery contract |
| Device-independent indefinite-order evidence | external 2026 experimental result with stated loopholes; not loophole-free certification and no FTD tag move |
| Page–Wootters and the FTD tick | [SYNTHESIS — SCOPE SEPARATION]; the former is an external relational-clock construction, whereas P2 supplies an ontic index |
| FTD global tick | [AXIOM] |
| FTD observable Lorentz recovery | [OPEN] |
| FTD latency | [IMPOSED] |
| Latency as an actualization/Born mechanism | [OPEN] |
| Slow-gate toy trend | pre-registered mechanism-level result in an [IMPOSED] ensemble; no canonical tag move and exact asymptote unresolved |
| Native-engine transfer of that trend | **Unbooked draft evidence (no epistemic tag):** Outcome N at its declared draft scope; no tag moves |
| DP–latency correspondence | [CONJECTURE — SPECULATIVE ALIGNMENT] only |
| A covariant or empirically hidden FTD selector | [OPEN] |
| Any new theorem or numerical prediction from this document | none; [SYNTHESIS] only |

---

## References

[1] R. Haag and D. Kastler, “An algebraic approach to quantum field
theory,” *Journal of Mathematical Physics* **5**, 848–861 (1964),
[doi:10.1063/1.1704187](https://doi.org/10.1063/1.1704187).

[2] R. Haag, *Local Quantum Physics: Fields, Particles, Algebras*, 2nd
ed. (Springer, 1996),
[doi:10.1007/978-3-642-61458-3](https://doi.org/10.1007/978-3-642-61458-3).

[3] C. J. Fewster and R. Verch, “Quantum fields and local measurements,”
*Communications in Mathematical Physics* **378**, 851–889 (2020),
[doi:10.1007/s00220-020-03800-6](https://doi.org/10.1007/s00220-020-03800-6);
[arXiv:1810.06512](https://arxiv.org/abs/1810.06512).

[4] S. Tomonaga, “On a relativistically invariant formulation of the
quantum theory of wave fields,” *Progress of Theoretical Physics* **1**,
27–42 (1946),
[doi:10.1143/PTP.1.27](https://doi.org/10.1143/PTP.1.27);
J. Schwinger, “Quantum electrodynamics. I. A covariant formulation,”
*Physical Review* **74**, 1439–1461 (1948),
[doi:10.1103/PhysRev.74.1439](https://doi.org/10.1103/PhysRev.74.1439).

[5] K.-E. Hellwig and K. Kraus, “Formal description of measurements in
local quantum field theory,” *Physical Review D* **1**, 566–571 (1970),
[doi:10.1103/PhysRevD.1.566](https://doi.org/10.1103/PhysRevD.1.566).

[6] Y. Aharonov and D. Z. Albert, “Can we make sense out of the measurement
process in relativistic quantum mechanics?” *Physical Review D* **24**,
359–370 (1981),
[doi:10.1103/PhysRevD.24.359](https://doi.org/10.1103/PhysRevD.24.359);
“Is the usual notion of time evolution adequate for quantum-mechanical
systems? II. Relativistic considerations,” *Physical Review D* **29**,
228–234 (1984),
[doi:10.1103/PhysRevD.29.228](https://doi.org/10.1103/PhysRevD.29.228).

[7] W. C. Myrvold, “On peaceful coexistence: is the collapse postulate
incompatible with relativity?” *Studies in History and Philosophy of Modern
Physics* **33**, 435–466 (2002),
[doi:10.1016/S1355-2198(02)00004-3](https://doi.org/10.1016/S1355-2198(02)00004-3).

[8] H. Everett III, “‘Relative state’ formulation of quantum mechanics,”
*Reviews of Modern Physics* **29**, 454–462 (1957),
[doi:10.1103/RevModPhys.29.454](https://doi.org/10.1103/RevModPhys.29.454).

[9] D. Deutsch, “Quantum theory of probability and decisions,”
*Proceedings of the Royal Society A* **455**, 3129–3137 (1999),
[doi:10.1098/rspa.1999.0443](https://doi.org/10.1098/rspa.1999.0443);
W. H. Zurek, “Probabilities from entanglement, Born's rule from envariance,”
*Physical Review A* **71**, 052105 (2005),
[doi:10.1103/PhysRevA.71.052105](https://doi.org/10.1103/PhysRevA.71.052105).

[10] J. S. Bell, “Are there quantum jumps?” in *Speakable and Unspeakable
in Quantum Mechanics* (Cambridge University Press, 1987).

[11] R. Tumulka, “A relativistic version of the Ghirardi–Rimini–Weber
model,” *Journal of Statistical Physics* **125**, 821–840 (2006),
[doi:10.1007/s10955-006-9227-3](https://doi.org/10.1007/s10955-006-9227-3);
[arXiv:quant-ph/0406094](https://arxiv.org/abs/quant-ph/0406094).

[12] R. Tumulka, “A relativistic GRW flash process with interaction”
(2020), [arXiv:2002.00482](https://arxiv.org/abs/2002.00482).

[13] L. Bombelli, J. Lee, D. Meyer, and R. D. Sorkin, “Space-time as a
causal set,” *Physical Review Letters* **59**, 521–524 (1987),
[doi:10.1103/PhysRevLett.59.521](https://doi.org/10.1103/PhysRevLett.59.521).

[14] D. P. Rideout and R. D. Sorkin, “A classical sequential growth
dynamics for causal sets,” *Physical Review D* **61**, 024002 (2000),
[doi:10.1103/PhysRevD.61.024002](https://doi.org/10.1103/PhysRevD.61.024002);
[arXiv:gr-qc/9904062](https://arxiv.org/abs/gr-qc/9904062).

[15] R. D. Sorkin, “Quantum mechanics as quantum measure theory,”
*Modern Physics Letters A* **9**, 3119–3127 (1994),
[doi:10.1142/S021773239400294X](https://doi.org/10.1142/S021773239400294X);
[arXiv:gr-qc/9401003](https://arxiv.org/abs/gr-qc/9401003).

[16] O. Oreshkov, F. Costa, and Č. Brukner, “Quantum correlations with no
causal order,” *Nature Communications* **3**, 1092 (2012),
[doi:10.1038/ncomms2076](https://doi.org/10.1038/ncomms2076).

[17] G. Chiribella, G. M. D'Ariano, P. Perinotti, and B. Valiron,
“Quantum computations without definite causal structure,”
*Physical Review A* **88**, 022318 (2013),
[doi:10.1103/PhysRevA.88.022318](https://doi.org/10.1103/PhysRevA.88.022318).

[18] C. M. D. Richter, M. Antesberger, H. Cao, P. Walther, and
L. A. Rozema, “Toward an experimental device-independent verification of
indefinite causal order,” *PRX Quantum* **7**, 010354 (2026),
[doi:10.1103/5t2y-ddmt](https://doi.org/10.1103/5t2y-ddmt).

[19] B. S. DeWitt, “Quantum theory of gravity. I. The canonical theory,”
*Physical Review* **160**, 1113–1148 (1967),
[doi:10.1103/PhysRev.160.1113](https://doi.org/10.1103/PhysRev.160.1113).

[20] D. N. Page and W. K. Wootters, “Evolution without evolution:
dynamics described by stationary observables,” *Physical Review D* **27**,
2885–2892 (1983),
[doi:10.1103/PhysRevD.27.2885](https://doi.org/10.1103/PhysRevD.27.2885).

[21] K. V. Kuchař, “Time and interpretations of quantum gravity,” in
*Proceedings of the 4th Canadian Conference on General Relativity and
Relativistic Astrophysics* (World Scientific, 1992); reprinted in
*International Journal of Modern Physics D* **20**, 3–86 (2011),
[doi:10.1142/S0218271811019347](https://doi.org/10.1142/S0218271811019347).

[22] V. Giovannetti, S. Lloyd, and L. Maccone, “Quantum time,”
*Physical Review D* **92**, 045033 (2015),
[doi:10.1103/PhysRevD.92.045033](https://doi.org/10.1103/PhysRevD.92.045033).

[23] L. Diósi, “A universal master equation for the gravitational
violation of quantum mechanics,” *Physics Letters A* **120**, 377–381
(1987), [doi:10.1016/0375-9601(87)90681-5](https://doi.org/10.1016/0375-9601(87)90681-5);
R. Penrose, “On gravity's role in quantum state reduction,”
*General Relativity and Gravitation* **28**, 581–600 (1996),
[doi:10.1007/BF02105068](https://doi.org/10.1007/BF02105068).

[24] S. Donadi et al., “Underground test of gravity-related wave function
collapse,” *Nature Physics* **17**, 74–78 (2021),
[doi:10.1038/s41567-020-1008-4](https://doi.org/10.1038/s41567-020-1008-4).

[25] E. Aprile et al. (XENON Collaboration), “Challenging spontaneous
quantum collapse with XENONnT,” *Physical Review Letters* **136**, 120201
(2026), [doi:10.1103/2jm3-4976](https://doi.org/10.1103/2jm3-4976);
[arXiv:2506.05507](https://arxiv.org/abs/2506.05507).

[26] M. Cortês and L. Smolin, “The universe as a process of unique events,”
*Physical Review D* **90**, 084007 (2014),
[doi:10.1103/PhysRevD.90.084007](https://doi.org/10.1103/PhysRevD.90.084007).

[27] P. A. Höhn, A. R. H. Smith, and M. P. E. Lock, “Equivalence of
approaches to relational quantum dynamics in relativistic settings,”
*Frontiers in Physics* **9**, 587083 (2021),
[doi:10.3389/fphy.2021.587083](https://doi.org/10.3389/fphy.2021.587083);
[arXiv:2007.00580](https://arxiv.org/abs/2007.00580).

[28] L. Hausmann, A. Schmidhuber, and E. Castro-Ruiz, “Measurement events
relative to temporal quantum reference frames,” *Quantum* **9**, 1616
(2025), [doi:10.22331/q-2025-01-30-1616](https://doi.org/10.22331/q-2025-01-30-1616);
[arXiv:2308.10967](https://arxiv.org/abs/2308.10967).

[29] T. Purves and A. J. Short, “Quantum theory cannot violate a causal
inequality,” *Physical Review Letters* **127**, 110402 (2021),
[doi:10.1103/PhysRevLett.127.110402](https://doi.org/10.1103/PhysRevLett.127.110402);
[arXiv:2101.09107](https://arxiv.org/abs/2101.09107).

[30] R. Srivastava and S. Surya, “Implementing Bell causality in quantum
sequential growth” (2026),
[arXiv:2603.25503](https://arxiv.org/abs/2603.25503).

---

**Booking note.** This document is a [SYNTHESIS] of external relativistic
frameworks and canonical FTD boundaries. It introduces no theorem, promotes
no claim, and supplies no numerical prediction. Its decisive correction is
the separation of three different obligations: causal-local instrument
order independence (R3), selector-history equivariance or preferred-order
operational hiding (R4), and FTD's sector-by-sector Lorentz-recovery contract.
