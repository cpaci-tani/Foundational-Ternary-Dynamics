# FOUND — The Selection–Potentiality–Actualization Chain v1

**Status:** `[SYNTHESIS — OPERATIONAL RECONSTRUCTION SCAFFOLD ON ESTABLISHED
EXTERNAL RESULTS; NO NEW FTD THEOREM]` +
`[BOOKED — FTD-0806]`

**Version:** v1.3

**Date:** 2026-08-07

**Scope:** finite-dimensional operational quantum theory through §13;
§§14–16 are an explicitly imported infinite-dimensional extension;
Part H is an FTD-facing status map, not a derivation.

**Provenance:** draft composed from the owner's construction of
2026-08-07; editorial integration and Part H were added during draft
revision. Revision v1.2
separated the CDP reconstruction from Busch's independent trace-rule
characterization and made the selector's response map and ensemble measure
explicit; **revision v1.3 restores the Part H framing of v1.1 at owner
direction** (the genesis identification, the six-principle scoring, and the
Born-campaign readings), retaining v1.2's notational precision. The
superseded v1.2 text is preserved at
`ARCH_FOUND_SPA_CHAIN_v1.2_superseded.md`. Prior revisions reconciled the
FTD-facing claims with the canonical Bell and
Born records.

**Canonical precedence:** [`LEDGER.md`](../../07_assessment/core_ledgers/LEDGER.md)
and [`SPEC_FTD_FRAMEWORK_V1.md`](../../01_reference/SPEC_FTD_FRAMEWORK_V1.md)
govern every FTD status stated here.

**Programme parents and recorded evidence:**
[`SPEC_OBSERVERS_COMPLETION_MAP_v1.md`](./SPEC_OBSERVERS_COMPLETION_MAP_v1.md)
(front T5),
[`SCOPE_TEMPORAL_INTERIOR_PROGRAM_v2.md`](./SCOPE_TEMPORAL_INTERIOR_PROGRAM_v2.md),
[`ANALYSIS_FC1_BELL_TRAP_v1.md`](../derivations/quantum_foundations/ANALYSIS_FC1_BELL_TRAP_v1.md)
(FTD-0796), FTD-0187/FTD-0199/FTD-0200 in the canonical ledger,
[`PREREG_BORN_DENSITY_UPCROSSING_v1.md`](../preregistrations/quantum_foundations/PREREG_BORN_DENSITY_UPCROSSING_v1.md),
[`PREREG_BORN_DENSITY_SATURATION_v2.md`](../preregistrations/quantum_foundations/PREREG_BORN_DENSITY_SATURATION_v2.md).
The unbooked draft execution
[`PREREG_BORN_REGIME_MAP_ENGINE_v1.md`](../preregistrations/quantum_foundations/PREREG_BORN_REGIME_MAP_ENGINE_v1.md)
is consulted only at its stated draft scope and moves no tag.
Vocabulary follows
[`REF_REFERENCE_FRAME_VOCABULARY.md`](../../01_reference/REF_REFERENCE_FRAME_VOCABULARY.md).
**External literature relied upon:** references [1]–[15], plus the
Wigner and Stone sources [16]–[17], are itemized completely below. Their
results remain external and conditional; citation here moves no FTD tag.

---

## 0. The claim, at its exact strength

The result is not "derive quantum mechanics from a word choice." That is
impossible, and this document does not attempt it. The result is:

> **Start below Hilbert space** — with actual events, selectable
> interventions, composable processes, and potential weights. Import the
> operational-probabilistic framework and reconstruction principles of
> [1], which conditionally recover finite-dimensional complex quantum
> theory. Use [2] as an independent characterization of the trace rule
> once the quantum effect space has been obtained. Then isolate singular
> actualization as an additional ontological postulate.

The resulting structure is the **Selection–Potentiality–Actualization
(SPA) chain**. Its genre should be stated in its own first section: this
is an **architecture** — a semantic and structural organization of
reconstruction-grade results with one isolated postulate — and its
empirical claims are confined to the declared interfaces
([`FOUND_SPA_CHAIN_RELATIVITY_EXTENSION_v1.md`](./FOUND_SPA_CHAIN_RELATIVITY_EXTENSION_v1.md)
§§17–18 and Part H below);
readers should cite it as foundations, never as a physics result. The term
"potentiality" is **[CONJECTURE — INTERPRETIVE]** and makes no independent
prediction in this document. Section 10 shows only that an imported coherent
quantum state is not one specified incoherent mixture; the stronger PBR
conclusion remains conditional on preparation independence. The vocabulary
would acquire independent empirical content only from a specified SPA
implementation that differs observationally from competing descriptions.
Part H supplies research interfaces, not confirming evidence for the
vocabulary. Its centerpiece is a recursion schema,

$$
(H_n,\mathcal C_n,\mathcal D)
\xrightarrow{\;\Gamma\;} \rho_n
\longrightarrow \{(e_i,w_i)\}
\xrightarrow{\;K\;\text{or}\;(F,\mu)\;} e_{n+1}
\longrightarrow H_{n+1},
\tag{1}$$

and its central unresolved object is the actualization block $\Sigma$.
The left side organizes standard finite-dimensional quantum structure
*conditional on imported operational assumptions*; it does not derive a
Hamiltonian, $\hbar$, masses, couplings, or a preparation ensemble. Nor
does $\Sigma$ contain every open problem in quantum foundations. Its
narrower role is to expose what must be added if singular outcomes are
retained. For FTD, an additional constitutional boundary applies:
[FC-1](../../07_assessment/core_ledgers/LEDGER.md#ftd-0255-record)
declines the measurement-map import $M$, so the Hilbert/operator chain in
this document is an externally imposed comparator rather than a candidate
FTD observable ontology. Canonical FTD may still seek a commutative map
from substrate histories and laboratory protocol labels to frame-relative
records and empirical frequencies; it may not call that map a derivation
of noncommutative states, effects, or instruments. Adopting the latter
would require an explicit FC-1 revision. The accounting table of §32
states line by line what is imported, postulate-conditional, conjectural,
and open. Scope flags (§6, §10, §14) mark where assumptions or extensions
do load-bearing work.

---

# Part A — Primitive ontology and the operational layer

## 1. Singular actuality

At stage $n$ the **actual history** is the finite event sequence

$$H_n = (e_1, e_2, \ldots, e_n).$$

**Postulate A (singular actuality) — `[IMPOSED — SPA SCAFFOLD]`.** One realized
experiment produces one next event $e_{n+1}$, so that
$H_{n+1}=H_n\circ e_{n+1}$. This is an axiom of the present scaffold,
not a sixth FTD postulate and not a consequence of FTD P1–P5.

There may be several possible successors, the set of **potential
continuations** $\mathcal{P}(H_n) = \{e'_1, e'_2, \ldots\}$; they are not
thereby several actual histories. The founding distinction of the chain
is therefore

$$\text{actual history } H_n \;\neq\; \text{set of potential
continuations } \mathcal{P}(H_n). \tag{2}$$

## 2. An experiment requires selection

An experiment permits controlled inputs $\mathcal{U} = \{u_1, u_2,
\ldots\}$ and measurement contexts $\mathcal{M} = \{M_1, M_2, \ldots\}$.
A particular trial contains the **selections**

$$u = \Sigma_U(\mathcal{U}), \qquad M = \Sigma_M(\mathcal{M}).$$

The experiment does not merely encounter a probability distribution. It
first establishes *what is being done to the system* and *what
distinction will be read out*; only afterward is an outcome distribution
defined. $\Sigma_U$ and $\Sigma_M$ denote **operational** selection: a
knob, a computer, a clock, or a deterministic circuit can instantiate the
selected setting. No metaphysical commitment about freedom is made here.

## 3. States and effects, operationally

Two preparation procedures are equivalent if every possible subsequent
experiment gives the same statistics; an equivalence class is a **state**
$\omega$. Two detector outcomes are equivalent if they have the same
probability for every preparation; an equivalence class is an **effect**
$e$. The primitive pairing is

$$e(\omega) \in [0, 1], \tag{3}$$

read semantically as *the potential weight of actualizing effect $e$
given state $\omega$*. At this stage there is no Hilbert space, no
wavefunction, and no Born rule.

## 4. Convexity from controllable mixing

If $\omega_1$ is prepared with frequency $p$ and $\omega_2$ with
frequency $1-p$, operational consistency requires

$$e\!\left(p\,\omega_1 + (1-p)\,\omega_2\right)
= p\,e(\omega_1) + (1-p)\,e(\omega_2).$$

The state space is therefore convex and effects are affine functionals.
This is the general-probabilistic-theory level: linear/convex structure
follows from mixing alone, and is not yet specifically quantum.

---

# Part B — Reconstruction

## 5. What selects quantum theory

"Potential + selection" admits many theories besides quantum mechanics:
classical probability, real-Hilbert-space quantum theory, box-world, and
the wider generalized-probabilistic class. The reconstruction of [1]
recovers **finite-dimensional complex quantum theory**, including its
operational state/effect/probability structure, from the imported
operational-probabilistic framework plus six principles:

| Principle | Meaning in the present semantics |
|---|---|
| Causality | probabilities of established events do not depend on later measurement choices |
| Perfect distinguishability | sufficiently distinct states can produce perfectly distinct actual records |
| Ideal compression | only the effective potential degrees of freedom a state needs must be retained |
| Local distinguishability | composite potential states are distinguishable by suitable local experiments |
| Pure conditioning | a maximally resolved outcome on a pure joint potential state leaves the conditional partner pure |
| Purification | apparent mixedness is the marginal of a larger pure potential structure, unique up to reversible action on the purifier |

Purification is the principle that separates quantum theory from the
classical member of the same class [1]. Under these principles,

$$\mathcal{H}_A \simeq \mathbb{C}^{d_A}. \tag{4}$$

This is a **theorem external to FTD, conditional on its imported
premises**: complex Hilbert space is not assumed *within that
reconstruction*, but neither the
operational-probabilistic framework nor its six physical principles are
derived from FTD here. The scope is finite-dimensional; the consequences
of that scope are flagged where they bind (§14).

## 6. An independent trace-rule characterization

The full theorem in [1] already recovers quantum probabilities; [2] is
therefore **not an additional step needed to complete CDP**, and combining
the two does not constitute a second derivation. It gives a useful,
logically independent characterization: once the quantum effect space
has been supplied, let $0\le E\le I$ and define a potential-weight
functional $w : \mathcal{E}(\mathcal{H})\to[0,1]$ with $w(I)=1$.

**Postulate B (effect-noncontextual additivity) — `[IMPOSED — SPA
SCAFFOLD]`, FLAG 2, load-bearing.**
$w(E + F) = w(E) + w(F)$ whenever $E + F \le I$, for *all* effect
decompositions, including those whose members do not commute. This is a
substantive physical assumption — a noncontextuality of valuation over
generalized measurements — and it is the axiom that does the entire work
of Busch's trace-form characterization below. A contextual valuation escapes the conclusion
without contradiction.

From additivity, $w(E) = n\,w(E/n)$ and hence $w(qE) = q\,w(E)$ for
rational $q \in [0,1]$ with $qE$ an effect. The extension to real
coefficients is carried by **monotonicity**, which is itself derived:
for effects $E \le F$, the difference $F - E$ is an effect and

$$w(F) = w(E) + w(F - E) \;\ge\; w(E),$$

so for real $\lambda$ and rational $q_n \uparrow \lambda \le r_n
\downarrow \lambda$,

$$q_n\,w(E) = w(q_n E) \;\le\; w(\lambda E) \;\le\; w(r_n E) =
r_n\,w(E) \;\Longrightarrow\; w(\lambda E) = \lambda\,w(E).$$

Extending by differences to Hermitian operators yields a positive linear
functional, and in finite dimension every such functional has the form

$$w(E) = \operatorname{Tr}(\rho_w E) \tag{5}$$

for a unique density operator $\rho_w$. Compatibility with the state
representation recovered in §5 identifies $\rho_w$ with the density
operator representing the preparation. This is the generalized Gleason
theorem of Busch [2] — valid already in dimension 2, where the original
Gleason theorem (which assumes additivity only over *projective*
decompositions) requires dimension $\ge 3$ and fails; the strengthening
of the premise from projectors to effects is exactly Postulate B's
content. Busch notes that on an individual-system reading the values
admit a propensity interpretation. Equation (5) is an external theorem
conditional on the imported quantum effect space and `[IMPOSED]`
Postulate B: the normalized valuation is trace-form. It neither derives
Postulate B nor derives why a single outcome occurs.

## 7. Probability is state plus question

With (5),

$$\rho = \text{potential state}, \qquad
p(E \mid \rho) = \operatorname{Tr}(\rho E).$$

Probability appears only when an effect is supplied. For a measurement
$M = \{E_1, \ldots, E_n\}$ with $\sum_i E_i = I$,

$$p(i \mid \rho, M) = \operatorname{Tr}(\rho E_i), \qquad
\sum_i p_i = \operatorname{Tr}\rho = 1.$$

The logical order is $\rho + M \longrightarrow P(O \mid \rho, M)$ — the
state is not itself a probability distribution.

## 8. The Born square as a special case

For a pure state $\rho = |\psi\rangle\langle\psi|$ and a sharp outcome
$E_i = |i\rangle\langle i|$,

$$p_i = \operatorname{Tr}\big(|\psi\rangle\langle\psi|\,
|i\rangle\langle i|\big) = |\langle i|\psi\rangle|^2. \tag{6}$$

The semantic hierarchy is therefore $|\psi\rangle \to \rho \to (\rho, M)
\to p_i$: the wavefunction is not probability; it represents a pure
structured potential state.

## 9. The wavefunction as ray

Global phase changes nothing, $|e^{i\alpha}\psi\rangle\langle
e^{i\alpha}\psi| = |\psi\rangle\langle\psi|$, so physical pure
potentiality is a **ray**. Within the finite-dimensional scope of this part,
write the components in any fixed orthonormal basis as
$\psi_j=R_j e^{i\theta_j}$, with $j$ a finite discrete label and
$R_j^2=|\psi_j|^2$. A continuum position representation belongs to the
explicitly imported infinite-dimensional extension of §14:

$$\psi = \text{weight structure} + \text{phase structure},$$

and probability retains only the first part after squaring.

## 10. Interference: potentiality is richer than that mixture

For $|\psi\rangle = \alpha|A\rangle + \beta|B\rangle$ and effect $E$,

$$p(E) = |\alpha|^2\langle A|E|A\rangle + |\beta|^2\langle B|E|B\rangle
+ 2\,\mathrm{Re}\,\alpha^*\beta\,\langle A|E|B\rangle,$$

whereas the mixture $\rho_{\mathrm{mix}} = |\alpha|^2|A\rangle\langle A|
+ |\beta|^2|B\rangle\langle B|$ lacks the cross terms. The coherent state
is therefore not ignorance over the pre-existing alternatives $\{A,B\}$ in
that specified mixture: relative phase has predictive consequences within
the imported quantum formalism. This does not select the word
"potentiality" or establish a unique ontology.

**FLAG 3 — scope of the epistemic exclusion.** As stated, the argument
excludes only the *specific-mixture* reading. The
Pusey–Barrett–Rudolph theorem [6] excludes overlapping ontic supports for
distinct pure quantum states within its ontological-model framework when
preparation independence and its other modeling assumptions are adopted.
It is not an assumption-free exclusion of every theory called
"$\psi$-epistemic." The chain's claim is the narrow interference claim
unless the PBR assumptions are separately imposed.

---

# Part C — Dynamics

## 11. Physical transformations are CPTP

A transformation $\mathcal{T} : \rho \mapsto \rho'$ must be affine on
mixtures (convexity), positivity-preserving, and — because the system
may be entangled with an untouched ancilla $B$ — $\mathcal{T}_A \otimes
I_B$ must preserve positivity for every ancilla: complete positivity.
Outcome-conditioned operations may decrease trace; a complete
deterministic channel preserves it:

$$\text{physical channels are CPTP maps},$$

the process-level formulation of the instrument tradition of Davies and
Lewis [3].

## 12. Reversibility gives unitarity; continuity gives Schrödinger

A reversible transformation **assumed to preserve pure-state transition
probabilities** acts on *rays* and is implemented by a unitary or
antiunitary operator (Wigner's theorem). For a one-parameter group the
antiunitary branch is excluded without appeal to vague continuity: each
$U(t) = U(t/2)\,U(t/2)$ is the square of an implementer, and the square
of either a unitary or an antiunitary is unitary. The implementers are a
priori projective; for the group $\mathbb{R}$ the obstruction vanishes
(Bargmann [14]: $\mathbb{R}$ is simply connected with trivial second
cohomology), so the phases can be chosen to give a true representation
$U(t+s) = U(t)U(s)$, $U(0) = I$. **Stone's theorem then requires strong
continuity** — adopted here as the precise content of the continuity
postulate — and gives $U(t) = e^{-itG}$ for a unique self-adjoint
generator $G$. Defining the energy operator by the empirical conversion
$H:=\hbar G$ gives

$$i\hbar\,\partial_t|\psi\rangle = H|\psi\rangle, \qquad
i\hbar\,\dot\rho = [H, \rho]. \tag{7}$$

These are external theorems conditional on transition-probability
preservation, a projective one-parameter group that admits the stated
lift, and strong continuity. They are not FTD `[THEOREM]` claims. Under
the present ontology, Schrödinger evolution is the deterministic
evolution of structured potentiality — not of probability. The
density-operator form is the general closed-system law; the Schrödinger
equation is its rank-one specialization.

## 13. What $\hbar$ is and is not

The existence of a phase generator is structural: $U(t) = e^{-itG}$.
Physical energy enters through $H = \hbar G$. The numerical value of
$\hbar$ is empirical; no structure in this chain derives it. Any claim
to derive it requires additional physical structure.

## 14. Canonical structure — an extension beyond the reconstruction

**FLAG 1 — the finite-dimension scope break.** The reconstruction (§5)
delivers $\mathbb{C}^{d}$ with $d$ finite. The canonical relation below
has **no finite-dimensional representation** (the trace of a commutator
vanishes; the trace of $i\hbar I$ does not). Sections 14–16 are
therefore an *infinite-dimensional extension* — standard, empirically
compelled, but imported separately rather than licensed by the
reconstruction theorem. The chain is reconstruction-clean through §13.

With that flag: let translations act by $T(a) = e^{-iaP/\hbar}$ and
require $T(a)^\dagger X T(a) = X + aI$. The mathematically primary
statement is the corresponding Weyl relation between the unitary
translation groups. If there is a common dense invariant domain
$\mathcal D$ of differentiable vectors for $X$ and $P$, differentiating
the covariance relation at $a=0$ gives, for $\psi\in\mathcal D$,

$$[X, P]\psi = i\hbar\psi. \tag{8}$$

Equation (8) is not an equality of bounded operators on the whole Hilbert
space; unbounded-operator domains are part of its statement.

Canonical non-commutativity is transformation geometry — how translation
acts on position — not an epistemic slogan about simultaneous knowledge.

## 15. Joint contexts and commutation

For sharp observables with spectral measures $P^A$ and $P^B$, joint sharp
measurability is equivalent to **strong commutation**:

$$[P^A(\Delta),P^B(\Gamma)]=0
\quad\text{for every Borel }\Delta,\Gamma
\iff \text{one joint sharp PVM exists}. \tag{8a}$$

In the discrete case the joint effects are
$G_{ij}=\Pi_i\Lambda_j$. For bounded self-adjoint $A,B$, strong commutation
is equivalent to the operator identity $[A,B]=0$. For unbounded operators,
vanishing of a commutator on a common dense domain is not sufficient; their
spectral measures must commute. Thus incompatibility of the spectral
distinctions—not a merely formal domain calculation—is the failure of one
joint sharp context.

## 16. Uncertainty

For a normalized vector $\psi$ in the domains required for the centered
operators, Cauchy–Schwarz gives the Robertson form. Writing an operator
commutator expectation additionally requires
$\psi\in\operatorname{Dom}(AB)\cap\operatorname{Dom}(BA)$:

$$\Delta A\,\Delta B \ge \tfrac{1}{2}\,\big|\langle[A,B]\rangle\big|,
\qquad \Delta X\,\Delta P \ge \tfrac{\hbar}{2}. \tag{9}$$

For the canonical pair $X,P$, the state-independent lower bound
$\hbar/2$ prevents both distributions from becoming arbitrarily sharp in
one state. For a general noncommuting pair, Robertson supplies only the
displayed state-dependent constraint: $\langle[A,B]\rangle$ may vanish,
and noncommuting operators can share an eigenvector. The general sharp-
measurement incompatibility statement is therefore §15's no-joint-PVM
criterion, not a blanket positive uncertainty product. No simultaneous
measurement histories are assumed anywhere in the derivation.

---

# Part D — Instruments and actualization

## 17. Measurement requires an instrument

A measurement context $M$ with outcomes $o$ is represented by a family
$\{\mathcal{I}^M_o\}_o$ of CP trace-nonincreasing maps whose sum is
trace-preserving. The associated effect is $E^M_o =
(\mathcal{I}^M_o)^*(I)$, and

$$p(o \mid \rho, M) = \operatorname{Tr}\big[\mathcal{I}^M_o(\rho)\big]
= \operatorname{Tr}(\rho E^M_o).$$

An instrument therefore carries two things at once: the **weight of a
possible actualization** and the **successor potential** if that
actualization occurs.

## 18. The actualization selector

Quantum theory supplies, for measurement $M$, the weighted alternatives
$\mathcal{P}_M = \{(o_1, p_1), \ldots, (o_n, p_n)\}$. The experimental
history contains one outcome. There are two mathematically distinct
single-outcome implementations:

$$
\begin{aligned}
&\text{primitive stochastic:}
&&K_O(o\mid\rho,M,H),\\
&\text{context-complete deterministic:}
&&\lambda\sim\mu(d\lambda\mid\rho,M,H),\qquad
o=F_O(\rho,M,H,\lambda).
\end{aligned}
\tag{10}
$$

Here $K_O$ is a stochastic kernel, while $F_O$ is a response map and
$\mu$ is an ensemble measure over the additional physical context. The
symbol $\Sigma_O$ will mean the *whole implementation* — $K_O$, or the
pair $(F_O,\mu)$ — and must not be used to hide the measure inside a
deterministic map. Agreement with quantum statistics requires either
$K_O(o\mid\rho,M,H)=\operatorname{Tr}(\rho E_o^M)$ or the explicit Born
pushforward condition

$$
(F_O)_*\mu(\{o\})
:=\int \mathbf 1_{\{F_O(\rho,M,H,\lambda)=o\}}\,
\mu(d\lambda\mid\rho,M,H)
=\operatorname{Tr}(\rho E^M_o).
\tag{11}
$$

The right-hand side of (11) is characterized by §6 conditional on
Postulate B. Neither Busch nor this notation constructs $F_O$, $\mu$, or
$K_O$; proves the pushforward equality for a physical microdynamics; or
explains why trial $n$ has outcome $o_k$ rather than $o_j$. Those are the
actualization and equilibrium burdens isolated by the chain.

## 19. Operational state update is conditionalization

If $o_k$ occurs with $p_k = \operatorname{Tr}[\mathcal{I}^M_k(\rho)]$,
the successor potential is

$$\rho' = \frac{\mathcal{I}^M_k(\rho)}{p_k},$$

i.e. new actuality $\Rightarrow$ new conditional potentiality; for an
ideal projector, $\rho' = \Pi_k\rho\,\Pi_k / \operatorname{Tr}(\rho\Pi_k)$.
This is the conditional successor-state rule inside the imported
instrument formalism. It neither constructs the actual event nor proves
that every ontology may identify physical collapse with information
update alone.

## 20. Histories compose

For settings $M_1, \ldots, M_N$ and outcome history $h = (o_1, \ldots,
o_N)$,

$$P(h) = \operatorname{Tr}\big[\mathcal{I}^{M_N}_{o_N} \circ \cdots
\circ \mathcal{I}^{M_1}_{o_1}(\rho_0)\big]. \tag{12}$$

Quantum theory natively supplies a weighted structure over possible
histories; one experiment yields one member. Potential histories are
plural; the actual history is singular — now as instrument algebra
rather than metaphor.

## 21. Instrument non-commutativity is history order-dependence

For interventions with successor maps $\mathcal{I}_A, \mathcal{I}_B$:
if $\mathcal{I}_B \circ \mathcal{I}_A \neq \mathcal{I}_A \circ
\mathcal{I}_B$ the two orderings are distinguishable histories.
Non-commutativity measures order dependence among potential histories;
the actual $AB$ may be compared with the unactualized counterfactual
$BA$ — an epistemic comparison between one actuality and one
potentiality, never between two actualities.

---

# Part E — Composition and nonlocality

## 22. Tensor structure and entanglement

The reconstruction gives $\mathcal{H}_{AB} = \mathcal{H}_A \otimes
\mathcal{H}_B$. A state with no decomposition $\rho_{AB} = \sum_k p_k\,
\rho_A^{(k)} \otimes \rho_B^{(k)}$ is entangled:

$$\text{entanglement} = \text{irreducible joint potentiality},$$

not multiple simultaneous actualities.

## 23. Local statistics and the Bell constraint

With local effects $E^x_a, F^y_b$, one trial produces one record
$(x, y, a, b)$ from $P(a,b \mid x,y) = \operatorname{Tr}[\rho_{AB}(E^x_a
\otimes F^y_b)]$. If a deeper variable $\lambda$ governs outcomes
locally, $P(a,b \mid x,y,\lambda) = P(a \mid x,\lambda)P(b \mid
y,\lambda)$, with measurement independence $P(\lambda \mid x,y) =
P(\lambda)$, then for $A_x(\lambda), B_y(\lambda) \in [-1,1]$ the CHSH
combination obeys $|S| \le 2$ [4]. Any deeper starting-context
actualization law reproducing quantum correlations must therefore
surrender ordinary Bell locality or measurement independence.

## 24. The quantum bound

For $\mathcal{B} = A_0\otimes(B_0+B_1) + A_1\otimes(B_0-B_1)$ with
$A_i^2 = B_j^2 = I$,

$$\mathcal{B}^2 = 4I - [A_0,A_1]\otimes[B_0,B_1]
\;\Rightarrow\; \|\mathcal{B}\| \le 2\sqrt{2},$$

Tsirelson's bound [5]. Saturation additionally requires a suitable
entangled state and observables; local non-commutativity alone does not
produce the excess. The operator proof concerns *alternative local
contexts* and does not require all four contexts to be actual in one
trial.

## 25. Decoherence's exact contribution

Unitary system–environment coupling takes
$(\alpha|A\rangle+\beta|B\rangle)|E_0\rangle$ to
$\alpha|A\rangle|E_A\rangle + \beta|B\rangle|E_B\rangle$; tracing out
the environment suppresses interference in proportion to $\langle
E_B|E_A\rangle$. Decoherence therefore explains why alternative
potential histories stop visibly interfering. The full state retains
both branches: decoherence supplies no $\Sigma_O$ and does not, by
itself, explain singular actualization.

---

# Part F — The unified selector and the fork

## 26. Three selections, one type

$\Sigma_U : \mathcal{U} \to u$, $\Sigma_M : \mathcal{M} \to M$,
$\Sigma_O : \mathcal{O}_M \to o$ all instantiate

$$\Sigma : \text{available alternatives} \longrightarrow
\text{one realized member}.$$

This is only a common **set-theoretic signature**. It does not establish
that a human intervention, an apparatus configuration, and an outcome
share one physical mechanism or one probability measure. Any such
unification is **[CONJECTURE]** until a common dynamics is supplied.

In a restricted laboratory model, $u$ and $M$ are treated as
interventions — the prediction is conditioned on
$\operatorname{do}(u), \operatorname{do}(M)$ — and are not outcomes
*inside that model*.

## 27. Boundary relativity

If the setting choice is made by a physical device $C$, then $u$ has a
prior physical state and becomes an output; likewise the apparatus
selector for $M$. The complete record $r = (u, M, o)$ is then governed
by one larger instrument $\{\mathcal{J}_r\}$ with $P(r \mid
\rho_{\mathrm{total}}) = \operatorname{Tr}[\mathcal{J}_r
(\rho_{\mathrm{total}})]$, and one complete record becomes actual. The
distinction *setting choice vs. measurement outcome* is
boundary-relative: interventions from outside, physical process from
within a closed description. Section 33 states the resulting FTD burden;
it does not derive a substrate instrument.

**The Frauchiger–Renner confrontation.** Boundary relativity walks
directly into the territory of the Frauchiger–Renner theorem [11]:
nested agents applying quantum theory to one another cannot jointly
maintain (Q) universal applicability of the quantum weights, (S) single
outcomes, and (C) unrestricted consistency of cross-agent inference.
Postulate A commits the chain to (S), but **the architecture alone does
not select one universal FR escape**:

- A relational conditional-state implementation may retain (Q) and (S)
  while denying unrestricted (C): agents use only records available in
  their histories, and cross-perspective inferences require an explicit
  record-comparison rule. This is a proposed interpretive completion,
  not a theorem of the SPA schema.
- A primitive-collapse or context-complete objective-actualization model
  that forbids an external agent from evolving an already actualized lab
  unitarily has modified or restricted universal (Q), even if it also
  restricts (C). It must state that dynamical departure explicitly.
- A branching implementation denies (S) rather than solving the
  single-outcome problem.

Thus no route may claim to preserve (Q), (S), and unrestricted (C)
simultaneously. The relational reading owes priority to relational
quantum mechanics [12] and QBism [13], but differs by treating the
Postulate-B weights as agent-independent while making conditional state
assignments perspective-dependent.

## 28. The closed recursion

For a closed description, let $\mathcal C_n$ contain the preparation and
control record, let $\mathcal D$ be the stipulated dynamics, and let
$\Gamma$ be the state-assignment/update rule:

$$\boxed{\;(H_n,\mathcal C_n,\mathcal D)
\xrightarrow{\;\Gamma\;} \rho_n \;\to\; \mathcal{P}_n \;\to\;
w(\,\cdot \mid H_n) \;\xrightarrow{\;K\;\text{or}\;(F,\mu)\;}\;
e_{n+1} \;\to\; H_{n+1}
\xrightarrow{\;\mathcal C_{n+1},\mathcal D,\Gamma\;} \rho_{n+1}
\;\to\; \cdots\;} \tag{13}$$

Equation (13) is **`[IMPOSED — SPA CLOSURE SCHEMA]`**, not a theorem
that history alone determines a quantum state. History together with
preparation/control data, stipulated dynamics, and $\Gamma$ supplies the
structured potentiality; the new event extends history and the same
inputs determine the next assignment. The laboratory-level expansion
(§31) is its operational reading.

## 29. Three logical response classes to the actualization question

At the schema level there are three logical response classes: a
deterministic response after state completion, a primitive stochastic
kernel, or denial of singular selection. These are not claimed as an
exhaustive taxonomy of named interpretations; the mainstream branching
alternative appears in the third class because it denies that there is a
$\Sigma$ to solve.

**Context-complete selection.** There exists a physical variable
$\lambda_n\sim\mu_n(d\lambda\mid H_n,\rho_n,M_n)$ with
$e_{n+1} = F(H_n, \rho_n,M_n, \lambda_n)$ and
$H(E_{n+1} \mid H_n, \rho_n,M_n, \lambda_n) = 0$: potential plurality is
partly epistemic. Reproducing quantum probabilities then requires the
nontrivial Born pushforward (11); averaging inaccessible context does not
guarantee it.

**Primitive selection.** No augmentation of physically meaningful prior
state fixes the outcome: $H(E_{n+1} \mid C_{\mathrm{complete}}) > 0$.
The future is genuinely underdetermined by prior actuality, and a kernel
$K_O$ is a new actualization primitive whose Born form remains an imposed
law unless independently derived.

**No selection (branching actuality).** Deny Postulate A: all weighted
continuations are actual, in the relative-state sense of Everett [7].
This dissolves the stochastic-collapse version of the $\Sigma$ problem, but
it does not make relativization automatic: the unitary theory still owes
microcausality and hypersurface-integrability, while its branch/record
definition must be stable under admissible relativistic descriptions. It
also carries a symmetric price: **the Born weights must then be derived
without a selector**, since nothing in bare branching
explains why weight $|\alpha|^2$ rather than branch counting governs
expectation. The two principal strategies are decision-theoretic
derivation (Deutsch [8], developed by Wallace [9]) and Zurek's
envariance derivation [10]; both are serious and both remain contested
(see e.g. Kent [15]). The chain's accounting is therefore symmetric and
explicit: **Postulate A purchases singular actuality at the price of an
unsolved $\Sigma$; its denial purchases the dissolution of $\Sigma$ at
the price of an unsolved Born-weight derivation and a preferred-basis
story carried by decoherence.** Neither side of the ledger is free, and
the chain's choice of Postulate A is a priced ontological commitment,
not an oversight of the alternative.

## 30. The statistical no-go

Given Born weights $p_1, \ldots, p_n$, let $\lambda \sim U[0,1)$ and
$F(\lambda) = k$ iff $c_{k-1} \le \lambda < c_k$ with $c_k = \sum_{j\le
k} p_j$. Then $P(F = k) = p_k$. Every stochastic Born selector has a
deterministic *measure-theoretic representation* with an auxiliary seed.
That representation is not automatically a physically equivalent
ontology: it need not preserve locality, measurement independence,
Lorentz covariance, causal accessibility, or an independently specified
microdynamic measure. Therefore **Born statistics alone cannot decide
between primitive and context-complete representations**, but physical
constraints can. Branching theories require their own probability
account. Bell (§23), contextuality, and covariance constrain what any
physical hidden context may be; only an unrestricted mathematical seed
is always constructible.

---

# Part G — Accounting

## 31. The laboratory chain

$$\begin{aligned}
&(H_n,\mathcal C_n,\mathcal D)\\[-2pt]
&\downarrow\\[-2pt]
&\rho_n=\Gamma(H_n,\mathcal C_n,\mathcal D)
&&\text{structured potentiality under the imposed closure schema}\\[-2pt]
&\mathcal{U}_n \xrightarrow{\Sigma_U} u_n &&\text{selected intervention}\\[-2pt]
&\rho_n^- = \mathcal{E}_{u_n}(\rho_n) &&\text{potential evolution}\\[-2pt]
&\mathcal{M}_n \xrightarrow{\Sigma_M} M_n &&\text{selected question}\\[-2pt]
&p(o) = \operatorname{Tr}(\rho_n^- E^{M_n}_o) &&\text{actualization weights}\\[-2pt]
&\lambda_n\sim\mu_n,\quad
o_n=F_O(\rho_n^-,M_n,H_n,\lambda_n)
&&\text{context-complete realization; or use }K_O\\[-2pt]
&(F_O)_*\mu_n(\{o\})=p(o)
&&\text{Born pushforward obligation}\\[-2pt]
&H_{n+1} = H_n \circ (u_n, M_n, o_n) &&\text{new history}\\[-2pt]
&\rho_{n+1} &&\text{new potentiality}
\end{aligned}$$

## 32. The derivation ledger

| Result | Status |
|---|---|
| Singular actuality | `[IMPOSED — SPA SCAFFOLD]` Postulate A; not an FTD axiom and priced against branching (§29) |
| Closure map $\Gamma$ from history + preparation/control + dynamics to state | `[IMPOSED — SPA CLOSURE SCHEMA]`; history alone is insufficient |
| CDP operational framework and six principles | `[IMPOSED — IMPORTED PREMISES]`; not derived from FTD here |
| Complex finite-dimensional QM | `[THEOREM — EXTERNAL, CONDITIONAL]` on the CDP premises [1] |
| Convex state/effect structure | `[DERIVED — WITHIN IMPORTED OPERATIONAL FRAMEWORK]` from controllable probabilistic mixing |
| Postulate B | `[IMPOSED — SPA SCAFFOLD]` effect-noncontextual additivity |
| Trace-form valuation / pure-state Born square | `[THEOREM — EXTERNAL, CONDITIONAL]` on the quantum effect space + Postulate B [2]; CDP already contains its own probability representation |
| Physical Born pushforward $(F_O)_*\mu=\operatorname{Tr}(\rho E)$ | `[OPEN]` inside the SPA scaffold; neither $F_O$ nor $\mu$ is constructed here. Its FTD-facing use is restricted by FC-1 (§§33–35): only a commutative empirical-frequency map is a canonical target |
| Phase relevance | `[DERIVED — ALGEBRAIC, WITHIN IMPORTED QM]`; PBR extension conditional on preparation independence (§10) |
| CPTP channel form | `[THEOREM — EXTERNAL, CONDITIONAL]` on affine mixture preservation, ancilla extension and physical positivity |
| Unitary reversible evolution | `[THEOREM — EXTERNAL, CONDITIONAL]` on ray transition-probability preservation and the group/lift assumptions (Wigner/Bargmann) |
| Schrödinger form | `[THEOREM — EXTERNAL, CONDITIONAL]` on a strongly continuous unitary time-translation group (Stone); $\hbar$ and $H$ are not supplied |
| Weyl/CCR structure | `[THEOREM — EXTERNAL, CONDITIONAL]` on translation covariance, outside finite $d$; commutator only on a stated dense domain |
| Robertson uncertainty | `[THEOREM — EXTERNAL, CONDITIONAL]` with the required operator domains |
| Instruments and conditional update | `[IMPOSED — IMPORTED STANDARD QM FRAMEWORK]`; successor state follows conditionally once an outcome is given |
| Tensor composition and entanglement | `[THEOREM — EXTERNAL, CONDITIONAL]` within the CDP reconstruction |
| CHSH bound 2 | `[THEOREM — EXTERNAL, CONDITIONAL]` on a joint, setting-independent measure; locality is sufficient but not the FTD-0796 obstruction's essential premise |
| Tsirelson bound $2\sqrt{2}$ | `[THEOREM — EXTERNAL, CONDITIONAL]` on standard quantum operator structure [5] |
| FR consistency | `[OPEN] + [SELECTION — INTERPRETIVE]`; each route rejects or restricts at least one of Q, S, C (§27) |
| Branching alternative | `[CONJECTURE — LIVE EXTERNAL ALTERNATIVE]`, priced rather than refuted; Born-weight and preferred-basis accounts remain debated [7–10, 15] |
| Individual actual outcome law | `[OPEN]` after Postulate A |
| Numerical $\hbar$; specific Hamiltonian; masses/couplings | `[OPEN — NOT SUPPLIED BY SPA]`; concrete applications import or otherwise determine them |
| Determinism vs. primitiveness of $\Sigma$ | `[OPEN — TO QUANTUM STATISTICS ALONE]`; physical constraints distinguish implementations (§30) |

The chain's bounded summary claim is: conditional on the imported
finite-dimensional quantum effect structure and Postulate B, normalized
effect-noncontextual weights are trace-form; Postulate A then requires a
single event to enter history. The actualization-specific missing object
is not a bare deterministic arrow but either a primitive kernel or a
response map together with its physical ensemble measure:

$$
K_O(o\mid H_n,\rho_n,M_n)
\quad\text{or}\quad
\lambda_n\sim\mu_n,\;
e_{n+1}=F_O(H_n,\rho_n,M_n,\lambda_n),\;
(F_O)_*\mu_n(\{o\})=w_n(o)\quad\text{for every outcome }o.
$$

This sharpens one boundary; it does not remove the independent open
inputs and bridges listed in §32.

---

# Part H — The FTD boundary

*This part is the document's FTD-facing draft integration. It maps the
chain onto canonical framework commitments and results, proposes no tag
promotion, and marks every unclosed bridge as conjectural or open.*

## 33. $\Sigma$ is genesis, and P5 has already chosen the fork

FTD is not in the generic reader's position. The framework possesses an
explicit actualization selector: the genesis/manifestation rule of the
production engine. At the substrate-transition level P5 makes that rule
deterministic, so with the complete engine state — including the seed of
its deterministic pseudorandom draw — carried in $\lambda_n$, a genesis
transition has the response-map form

$$s_{n+1}=F_{\rm gen}(s_n,J_n,\lambda_n).$$

Genesis is not a bare threshold: eligible sites pass a probability ramp
and an index/seed/tick-keyed draw, and more than one site can manifest in
a tick (see [`SPEC_ENGINE.md`](../../../../engine/SPEC_ENGINE.md) and
[`phase_write.cpp`](../../../../engine/src/render_bridge_phases/phase_write.cpp)).

**The identification.** The chain's central FTD-facing proposal is that
this rule is the framework's instance of the actualization block:

$$\Sigma_O \;\longleftrightarrow\; F_{\rm gen},$$

which places FTD squarely on the **context-complete branch** of §29 by
constitution, with $\lambda_n$ the substrate microstate. The fork is not
open for this framework: P5 forecloses a primitive stochastic engine law
unless FTD is extended, and Postulate A is already FTD's posture on
singular outcomes.

The identification is *proposed and priced*, not proven. What it still
owes is explicit, and the chain states it as the price rather than hiding
it: a commutative association of laboratory preparation and setting
labels with substrate ensembles, response functions, and mutually
exclusive records $o$, whose empirical pushforward may then be compared
with the external benchmark
$p_{\rm QM}(o\mid P,M)=\operatorname{Tr}(\rho_P E_o^M)$. Neither $\rho_P$
nor $E_o^M$ thereby becomes an FTD observable; a *structure-preserving*
map onto the Hilbert/operator state–effect–instrument algebra would be
the measurement-map import $M$, which
[FC-1](../../07_assessment/core_ledgers/LEDGER.md#ftd-0255-record)
declines and whose adoption would require an explicit framework
amendment. Canonical constraints that bound the proposal are recorded
rather than suppressed:
[FTD-0226](../../07_assessment/core_ledgers/LEDGER.md#ftd-0226-record)
(the manifestation map has a distributive Boolean event lattice; its
tested non-commutativity route is **[CLOSED NEGATIVE]**),
[FTD-0187](../../07_assessment/core_ledgers/LEDGER.md#ftd-0187-record)
(probability-as-energy-density **[OPEN]**), and
[FTD-0199](../../07_assessment/core_ledgers/LEDGER.md#ftd-0199-record)/[FTD-0200](../../07_assessment/core_ledgers/LEDGER.md#ftd-0200-record)
(**[CLOSED NEGATIVE]** in their tested constructions). Status of the
identification: **[CONJECTURE — PROPOSED IDENTIFICATION, PRICED]**.

**The Bell price is already booked, and the chain explains it.**
[FTD-0796](../../07_assessment/core_ledgers/LEDGER.md#ftd-0796-record)
shows that if the four CHSH observables belong to the complete
commutative algebra $A_5$ **and one setting-independent ensemble measure
$\mu$ applies**, their pushforward is a joint distribution and Fine's
theorem forces $|S|\le2$; locality is not the operative extra premise.
This is precisely the §27 enlargement seen from the substrate side: a
theory whose weights collapse into a single setting-independent measure
over the closed recursion is native-$S\le2$ by theorem. Quantum theory
survives §27 because its weights remain instrument-indexed rather than
forming one joint measure. **The chain therefore explains FTD's declared
boundary rather than conflicting with it** — native $S\le2$, with
Tsirelson imported conditional on the **[SELECTION]** singlet.

The corresponding constitutional tension is recorded, not resolved: under
**Posture A** (measurement independence) the registered FTD triangle
correlator has $S\le2$ and is contradicted by laboratory Bell
correlations; under **Posture B**
([FTD-0329](../../07_assessment/core_ledgers/LEDGER.md#ftd-0329-record)'s
ledger-governing measurement dependence) the exact allowance is
$S\le\min(2+3M,4)$, but $M$ is not independently pinned. Choosing between
them is an owner-level constitutional act.

## 34. The reconstruction as an itemized import invoice

[`SPEC_OBSERVERS_COMPLETION_MAP_v1.md`](./SPEC_OBSERVERS_COMPLETION_MAP_v1.md)
currently imports "the completed Hilbert space" as a single
representation-side line. The reconstruction of §5 replaces that monolith
with six separately auditable premises — and therefore with **six
separately falsifiable questions** that can be put to a candidate
substrate one at a time. That is the priced-import ledger's methodology
applied to quantum theory itself: instead of one unpriced import, six
lines each with its own falsifier.

The scoring column below is **preliminary and non-binding**; it names
what would have to be shown, and moves no canonical tag. Under FC-1 the
six remain external premises of the SPA/quantum comparator, and a
physical realization of their joint complex-quantum conclusion would
import $M$ and require an explicit FC-1 amendment.

| Principle | Preliminary substrate scoring (non-binding; no tags moved) |
|---|---|
| Causality | plausibly native — P5 forward determinism and the FC-2 arrow supply a no-future-signalling structure at the record level; the commutative protocol test is a distinct **[OPEN]** item |
| Perfect distinguishability | plausibly native — ternary microstates are perfectly distinct records; whether this carries the reconstructed-state meaning is **[OPEN]** |
| Ideal compression | **[OPEN]** — no FTD analogue scored |
| Local distinguishability | **the $\mathbb{C}$-selecting axiom** (real-Hilbert quantum theory fails it); it meets OT-1.5's native $\mathbb{Z}[i]^2$ seed as a precise question, currently **[OPEN]** — the seed does not by itself establish operational local tomography |
| Pure conditioning | **[OPEN]** — no FTD analogue scored |
| Purification | **the quantum-selecting axiom and the likely crux import**; whether the dispositional layer $J$ can serve as purifier structure is a genuine research question, not a slogan, and identifying it as such would invoke the declined $M$ unless FC-1 is amended |

A future quantitative wing may price these six in FTD-0371's currency,
each with its falsifier. That is owner work and must record the FC-1
amendment price where the reconstructed structures are assigned a
physical role.

## 35. The Born problem split: static uniqueness, dynamic implementation

Postulate B and Busch's characterization give the **static** half: any
consistent, effect-noncontextual weighting on the imported quantum effect
space is trace-form — uniqueness, given consistency. They do not solve
the dynamic pushforward (11).

The temporal-interior campaigns addressed the **dynamic** half: whether
FTD's actual selector, coarse-grained over inaccessible microstate,
*implements* a consistent weighting. Their records, at their registered
scopes:

- [`PREREG_BORN_DENSITY_SATURATION_v2.md`](../preregistrations/quantum_foundations/PREREG_BORN_DENSITY_SATURATION_v2.md)
  — a pre-registered mechanism-level measurement in an **[IMPOSED]**
  ensemble on a quick-check platform. Across the registered grid the
  Born-fraction statistic rose monotonically from $0.049$ to $0.836$ as
  the fast-mode/slow-noise control parameter increased. Within the
  declared mechanism class this exhibits Born/occupation weighting as the
  fast-mode/slow-noise asymptote of threshold statistics. Whether the
  true asymptote is exactly $1$ or near the descriptive fit $0.86$
  remains open; the result is neither an exact Born law nor a
  substrate-wide theorem.
- [`PREREG_BORN_REGIME_MAP_ENGINE_v1.md`](../preregistrations/quantum_foundations/PREREG_BORN_REGIME_MAP_ENGINE_v1.md)
  — the engine-side execution, reporting **Outcome N**: no declared axis
  reached the registered structure threshold, so the toy regime law did
  not transfer to the native thermal field in the sampled domain. Its
  same-band signal/noise diagnosis was **post-hoc and non-registered**;
  it disfavours that route without proving that native flux noise can
  never succeed.
- [`PREREG_LATENCY_SLOW_GATE_v1.md`](../preregistrations/quantum_foundations/PREREG_LATENCY_SLOW_GATE_v1.md)
  — the latency sector scored **S+V** at mechanism level: the sector's
  fluctuations are slow relative to the flux band, and a channel at that
  timescale carries the weighting to $0.936$. The Stage-B coupling is a
  declared additive model class, and Stage A's profile has no RNG
  consumer, so its seed independence is vacuous.

In the chain's vocabulary, **FTD's Born problem is the quantum-equilibrium
problem for the genesis selector**: the framework does not need to prove
indeterminism, since P5 forbids it; it needs its deterministic seed — the
microstate — to be Born-distributed under the coarse-graining the
substrate itself supplies. That is §30's construction run in reverse, and
it is now accompanied by one measured necessary condition (an
actualization gate slower than the flux band) and one route disfavoured
at scope (thermal flux noise). Whether the engine's *native* manifestation
rule can couple to a slow non-flux channel with the purity the mechanism
demands is the registered next question. $s$-sector history and latency
remain candidates requiring separate preregistration.

---

## References

[1] G. Chiribella, G. M. D'Ariano, P. Perinotti, *Informational
derivation of quantum theory*, Phys. Rev. A **84**, 012311 (2011);
[arXiv:1011.6451](https://arxiv.org/abs/1011.6451),
[doi:10.1103/PhysRevA.84.012311](https://doi.org/10.1103/PhysRevA.84.012311).
[2] P. Busch, *Quantum states and generalized observables: a simple
proof of Gleason's theorem*, Phys. Rev. Lett. **91**, 120403 (2003);
[arXiv:quant-ph/9909073](https://arxiv.org/abs/quant-ph/9909073),
[doi:10.1103/PhysRevLett.91.120403](https://doi.org/10.1103/PhysRevLett.91.120403).
[3] E. B. Davies, J. T. Lewis, *An operational approach to quantum
probability*, Commun. Math. Phys. **17**, 239–260 (1970),
[doi:10.1007/BF01647093](https://doi.org/10.1007/BF01647093).
[4] J. F. Clauser, M. A. Horne, A. Shimony, R. A. Holt, *Proposed
experiment to test local hidden-variable theories*, Phys. Rev. Lett.
**23**, 880–884 (1969),
[doi:10.1103/PhysRevLett.23.880](https://doi.org/10.1103/PhysRevLett.23.880).
[5] B. S. Cirel'son, *Quantum generalizations of Bell's inequality*,
Lett. Math. Phys. **4**, 93–100 (1980),
[doi:10.1007/BF00417500](https://doi.org/10.1007/BF00417500).
[6] M. F. Pusey, J. Barrett, T. Rudolph, *On the reality of the quantum
state*, Nature Physics **8**, 475–478 (2012),
[arXiv:1111.3328](https://arxiv.org/abs/1111.3328),
[doi:10.1038/nphys2309](https://doi.org/10.1038/nphys2309).
[7] H. Everett III, *"Relative state" formulation of quantum mechanics*,
Rev. Mod. Phys. **29**, 454–462 (1957),
[doi:10.1103/RevModPhys.29.454](https://doi.org/10.1103/RevModPhys.29.454).
[8] D. Deutsch, *Quantum theory of probability and decisions*, Proc. R.
Soc. Lond. A **455**, 3129–3137 (1999),
[doi:10.1098/rspa.1999.0443](https://doi.org/10.1098/rspa.1999.0443).
[9] D. Wallace, *The Emergent Multiverse* (Oxford University Press,
2012).
[10] W. H. Zurek, *Probabilities from entanglement, Born's rule from
envariance*, Phys. Rev. A **71**, 052105 (2005),
[doi:10.1103/PhysRevA.71.052105](https://doi.org/10.1103/PhysRevA.71.052105).
[11] D. Frauchiger, R. Renner, *Quantum theory cannot consistently
describe the use of itself*, Nat. Commun. **9**, 3711 (2018),
[arXiv:1604.07422](https://arxiv.org/abs/1604.07422),
[doi:10.1038/s41467-018-05739-8](https://doi.org/10.1038/s41467-018-05739-8).
[12] C. Rovelli, *Relational quantum mechanics*, Int. J. Theor. Phys.
**35**, 1637–1678 (1996),
[arXiv:quant-ph/9609002](https://arxiv.org/abs/quant-ph/9609002),
[doi:10.1007/BF02302261](https://doi.org/10.1007/BF02302261).
[13] C. A. Fuchs, N. D. Mermin, R. Schack, *An introduction to QBism
with an application to the locality of quantum mechanics*, Am. J. Phys.
**82**, 749–754 (2014),
[arXiv:1311.5253](https://arxiv.org/abs/1311.5253),
[doi:10.1119/1.4874855](https://doi.org/10.1119/1.4874855).
[14] V. Bargmann, *On unitary ray representations of continuous
groups*, Ann. Math. **59**, 1–46 (1954),
[doi:10.2307/1969831](https://doi.org/10.2307/1969831).
[15] A. Kent, *One world versus many: the inadequacy of Everettian
accounts of evolution, probability, and scientific confirmation*, in
S. Saunders et al. (eds.), *Many Worlds?* (Oxford University Press,
2010), [arXiv:0905.0624](https://arxiv.org/abs/0905.0624).
[16] E. P. Wigner, *Gruppentheorie und ihre Anwendung auf die
Quantenmechanik der Atomspektren* (Vieweg, Braunschweig, 1931); English
translation, *Group Theory and its Application to the Quantum Mechanics
of Atomic Spectra* (Academic Press, New York, 1959).
[17] M. H. Stone, *On one-parameter unitary groups in Hilbert space*,
Ann. Math. **33**, 643–648 (1932),
[doi:10.2307/1968538](https://doi.org/10.2307/1968538).

---

**Booking note.** This unbooked draft preserves the owner's construction
with integrated scope flags and an FTD-boundary audit; it introduces no
new theorem and moves no canonical tag. A ledger row, if desired, is the
owner's to mint. Its natural companion is
[`SPEC_OBSERVERS_COMPLETION_MAP_v1.md`](./SPEC_OBSERVERS_COMPLETION_MAP_v1.md).

**Companion relativity draft (2026-08-07):**
[`FOUND_SPA_CHAIN_RELATIVITY_EXTENSION_v1.md`](./FOUND_SPA_CHAIN_RELATIVITY_EXTENSION_v1.md)
surveys several non-exhaustive ways an actualization architecture may be
related to relativistic and quantum-gravitational frameworks. It is a
separate synthesis with its own assumptions and open repairs; no
QFT-instrument theorem, causal-set identification, or exhaustive route
classification is imported into the present document merely by that
cross-reference.
