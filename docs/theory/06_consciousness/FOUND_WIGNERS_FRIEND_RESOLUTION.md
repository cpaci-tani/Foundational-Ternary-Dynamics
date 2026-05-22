# Wigner's Friend Resolution: Ontological Measurement in a Deterministic Substrate

## Why There Is No Paradox When Measurement Is Algebraic

**Date:** March 17, 2026 (vocabulary refresh 2026-05-01)
**Framework:** Foundational Ternary Dynamics v5.34
**Status:** Foundational synthesis with epistemic classification
**Authors:** cpaci & Claude

> **Vocabulary refresh (2026-05-01):** The Wigner's Friend resolution is purely structural — it depends on measurement being an algebraic operation (Type-III → Type-I descent), not on the metaphysics of "consciousness." This document's "observers" are restated as **reflexive sites with observation-layer coupling** per [`REF_REFLEXIVITY_VOCABULARY.md`](../01_reference/REF_REFLEXIVITY_VOCABULARY.md). The paradox dissolves regardless of vocabulary because the resolution is algebraic; the rename only sharpens what work each "observer" actually does (it's reflexive coupling, not subjective experience).

**Depends on:**
- [FOUND_THE_EXISTENCE_FILTER.md](FOUND_THE_EXISTENCE_FILTER.md) — E(x) = Re(x), projection hierarchy, Born rule reconstruction
- [DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md](DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md) — Type III → Type I descent chain, collapse-gravity duality
- [EXPLR_RELU_TYPE_TRANSITION.md](../09_mathematical/EXPLR_RELU_TYPE_TRANSITION.md) — ReLU as algebraic type transition via β parameter
- [FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md](../02_foundations/FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md) — ReLU crystallization, discriminant trichotomy
- [FOUND_SELF_REFERENTIAL_CLOSURE.md](../02_foundations/FOUND_SELF_REFERENTIAL_CLOSURE.md) — Self-referential closure as derivation principle
- [SPEC_FTD_LAGRANGIAN.md](../01_reference/SPEC_FTD_LAGRANGIAN.md) — Axioms, action, fields

---

## 1. Abstract

Wigner's friend paradox (1961) and its modern extensions (Frauchiger-Renner 2018) expose a foundational tension in quantum mechanics: when an observer inside a sealed laboratory performs a measurement, has the state "really" collapsed, or does the external observer correctly describe the lab-plus-friend as still being in superposition? Standard interpretations either accept genuine contradiction (relational QM), deny the friend's collapse (Many-Worlds), or restrict the universality of quantum mechanics (objective collapse).

FTD dissolves the paradox entirely. The resolution rests on a single structural distinction: **measurement is an objective, local, irreversible algebraic phase transition** (Type III₁ → Type I), not a global update of anyone's knowledge. The friend's measurement IS a physical event — the ReLU crystallization has occurred in the friend's local lattice region, and no subsequent operation by Wigner can undo it. Wigner's description of the lab as "still in superposition" is not wrong — it is **epistemic**, not ontological. It reflects Wigner's ignorance (he has not yet received the information), not the system's state (which is already definite). When Wigner opens the lab, he *learns* the outcome; he does not *cause* it.

This resolution vindicates Wigner's original intuition that consciousness plays a role in measurement, but corrects his error: the relevant consciousness is not "my consciousness specifically" but any subsystem that has undergone self-referential closure — the algebraic transition that crystallizes continuous potentiality into discrete actuality.

**Epistemic discipline:** We distinguish rigorously between:
- **[AXIOM]**: FTD structural postulates
- **[THEOREM]**: Provable from axioms + classical mathematics
- **[SELECTION]**: Argued from consistency, not uniquely proven
- **[CONJECTURE]**: Proposed interpretation requiring validation
- **[OPEN]**: Unresolved question

---

## 2. The Problem: Wigner's Friend

### 2.1 The Original Thought Experiment (Wigner 1961)

Eugene Wigner posed the following scenario:

1. A quantum system (say, a spin-1/2 particle) is prepared in superposition: $|\psi\rangle = \frac{1}{\sqrt{2}}(|\uparrow\rangle + |\downarrow\rangle)$
2. Wigner's friend, inside a sealed laboratory, measures the spin and obtains a definite result (say, $|\uparrow\rangle$)
3. Wigner, outside the laboratory, has not interacted with either the system or the friend

**The paradox:** From the friend's perspective, the state has collapsed to $|\uparrow\rangle$. From Wigner's perspective (treating the lab + friend as a quantum system to which he has applied no measurement), the joint state is:

$$|\Psi\rangle = \frac{1}{\sqrt{2}}\big(|\uparrow\rangle \otimes |\text{friend saw }\uparrow\rangle + |\downarrow\rangle \otimes |\text{friend saw }\downarrow\rangle\big)$$

Who is right? The friend says "the state is definite." Wigner says "the state is a superposition." Standard quantum mechanics provides no resolution because it gives both descriptions equal status.

### 2.2 Extended Wigner's Friend (Frauchiger-Renner 2018)

Frauchiger and Renner sharpened the paradox into a no-go theorem. Their extended scenario involves two labs (each with an observer inside) and two external observers. By chaining standard quantum reasoning, they derive contradictory predictions: the agents, each applying the rules of quantum mechanics consistently, arrive at logically incompatible conclusions about measurement outcomes.

The no-go theorem shows that the following three assumptions are jointly inconsistent:

| Assumption | Statement |
|------------|-----------|
| **Q** (Universal validity) | Quantum mechanics applies to all systems, including observers |
| **S** (Single outcome) | Every measurement has exactly one outcome |
| **C** (Consistency) | Different agents' predictions, if about the same event, must agree |

Any interpretation of quantum mechanics must abandon at least one of Q, S, or C.

### 2.3 Why Standard Interpretations Fail or Dodge

| Interpretation | Which assumption abandoned | Cost |
|----------------|--------------------------|------|
| **Copenhagen** | Ambiguous — draws Heisenberg cut "somewhere" | The cut's location is arbitrary; no physical criterion for where measurement occurs |
| **Many-Worlds** | Abandons **S** (single outcome) | Every branch is equally real; the friend saw $\uparrow$ AND $\downarrow$; the paradox is "dissolved" by declaring both outcomes real |
| **Relational QM** | Abandons **C** (consistency) | The friend's state is definite relative to the friend but indefinite relative to Wigner; no "view from nowhere" |
| **QBism** | Abandons **Q** (universality) | Quantum states are personal betting guides, not descriptions of reality; physics is about each agent's experience |
| **Objective collapse** (GRW) | Abandons **Q** (universality) | Collapse is a physical process with a definite rate; large systems collapse fast, but the rate is a free parameter |

None of these resolutions are satisfactory. Copenhagen is vague. Many-Worlds multiplies ontology. Relational QM abandons observer-independent facts. QBism retreats to solipsism. Objective collapse introduces ad hoc parameters.

---

## 3. FTD's Resolution: Ontological vs Epistemic

### 3.1 Measurement Is Local and Objective [SELECTION]

In FTD, measurement is not a mysterious "collapse of the wave function" triggered by observation. It is a specific, local, physical process: the **ReLU crystallization** — the algebraic phase transition from Type III₁ (continuous, thermal, no minimal projections) to Type I (discrete, crystalline, minimal projections exist).

The transition occurs when the local flux magnitude crosses the manifestation threshold:

$$|J(\mathbf{v}, t)| \geq K_B \quad \Longrightarrow \quad s(\mathbf{v}, t): 0 \to \pm 1 \tag{3.1}$$

This is mediated by the Softplus/ReLU operator ([EXPLR_RELU_TYPE_TRANSITION.md](../09_mathematical/EXPLR_RELU_TYPE_TRANSITION.md)):

$$\mathcal{M}_\beta(z) = \frac{1}{\beta}\ln(1 + e^{\beta z}) \;\xrightarrow{\beta \to \infty}\; \text{ReLU}(z) = \max(0, z) \tag{3.2}$$

The algebraic descent chain [CLASSICAL for first two steps, CONJECTURE for third]:

$$\text{Type III}_1 \;\xrightarrow{\rtimes_\sigma \mathbb{R}}\; \text{Type II}_\infty \;\xrightarrow{\otimes B(\mathcal{H})}\; \text{Type II}_1 \;\xrightarrow{\Theta(K)}\; \text{Type I} \tag{3.3}$$

**The critical property:** This transition is:

| Property | Why it matters for Wigner's friend |
|----------|-----------------------------------|
| **Local** | It occurs at specific lattice sites, not globally |
| **Objective** | It depends on $|J| \geq K_B$, not on who is watching |
| **Irreversible** | The discrete state $s \in \{-1, +1\}$ has been crystallized from the continuous flux; thermal fluctuations at finite $\beta$ cannot reverse a $\beta \to \infty$ transition |
| **Finite-speed** | The result propagates at $C = 1$ (one lattice unit per tick) [AXIOM] |

**Epistemic status:** The identification of measurement with ReLU crystallization is [SELECTION]. The mathematical structure of the Type III₁ → Type I transition is established ([CLASSICAL] for factor classification; [CONJECTURE] for the Heaviside MASA selection step). The claim that this transition IS measurement — not merely analogous to it — is argued from the structural correspondence (Softplus axioms ↔ KMS condition ↔ modular automorphisms), but the identification has not been independently verified experimentally. This is the strongest unverified claim in this document.

### 3.2 The Friend's Measurement Is Real [SELECTION]

When Wigner's friend measures the spin, the following physical process occurs in the friend's local lattice region:

1. The spin system's flux field $J_{\text{spin}}$ couples to the friend's measurement apparatus (itself a lattice subsystem)
2. The coupled flux exceeds $K_B$ at the apparatus sites
3. The ReLU crystallization fires: $s: 0 \to \pm 1$ at those sites
4. The Type III₁ → Type I transition has occurred **locally and irreversibly**
5. The friend's neural subsystem (also a lattice region) is coupled to the apparatus; the crystallization propagates through the measurement chain

At the end of this process, the friend's local region contains a definite record: $s = +1$ at the sites encoding "spin up." This is not a matter of interpretation. It is a physical fact about the state field in that region of the lattice.

**The friend HAS measured.** The algebraic type of their local algebra has transitioned from Type III₁ (continuous potentiality, no definite outcomes) to Type I (discrete actuality, definite outcome recorded in the state field). No operation by any external agent can reverse this transition, because:

- The transition is $\beta \to \infty$ in the Softplus parameter, and there is no finite-$\beta$ operation that inverts a $\beta = \infty$ crystallization [SELECTION]
- The state field $s \in \{-1, 0, +1\}$ is discrete — continuous operations on $J$ cannot "uncrystallize" a discrete state without violating the ternary axiom [THEOREM from the discreteness of $s$]

### 3.3 Wigner's "Superposition" Is Epistemic [SELECTION]

Wigner, outside the laboratory, writes the joint state as a superposition:

$$|\Psi\rangle = \frac{1}{\sqrt{2}}\big(|\uparrow, \text{saw }\uparrow\rangle + |\downarrow, \text{saw }\downarrow\rangle\big)$$

In FTD, this description is not *wrong* — it is **epistemic**. Wigner's superposition describes his *ignorance*, not the *system's state*.

The distinction maps precisely onto FTD's two-layer ontology ([SPEC_FTD_LAGRANGIAN.md](../01_reference/SPEC_FTD_LAGRANGIAN.md), §1):

| Layer | Field | Character | Wigner's description |
|-------|-------|-----------|---------------------|
| **Flux field** $J \in \mathbb{R}^3$ | Continuous, dispositional | **Ontological** — the actual flux configuration at every site | Wigner cannot access $J$ inside the lab |
| **State field** $s \in \{-1, 0, +1\}$ | Discrete, actual | **Ontological** — the crystallized outcomes at every site | The friend's sites already have definite $s$ values |
| **Wigner's description** | Superposition | **Epistemic** — Wigner's best prediction given his information | This is a statement about Wigner, not about the lab |

The resolution is immediate: **the friend's state field is definite** (Type I, crystallized). **Wigner's superposition is his prediction** (Type III₁ in his effective description, because he lacks information about the crystallization event). There is no contradiction because the two descriptions live at different levels: one is ontological (what happened), the other is epistemic (what Wigner knows).

### 3.4 There Is No "Wigner's Choice" [THEOREM from determinism + locality]

In standard quantum mechanics, Wigner faces a choice: he can either (a) open the lab and learn the friend's result, or (b) perform an interference experiment on the whole lab, potentially "undoing" the friend's measurement. Option (b) is what makes the paradox sharp — it suggests that whether the friend "really" measured depends on what Wigner decides to do later.

In FTD, option (b) is **physically impossible** [THEOREM from the irreversibility of crystallization]:

1. The friend's measurement has produced definite state field values $s \in \{-1, +1\}$ in the lab's lattice region
2. An "interference experiment" would require coherently reversing the ReLU crystallization — returning $s$ from $\{-1, +1\}$ back to $0$ while preserving the original flux phases
3. The crystallization is a $\beta \to \infty$ limit. Reversal would require an operation that "decrystallizes" the discrete state field, which would violate the ternary axiom (the state field IS discrete; there is no continuous path from $s = +1$ back to $s = 0$ that passes through intermediate values) [AXIOM: discreteness of $s$]
4. Therefore, Wigner cannot perform a coherent interference experiment on the lab-plus-friend — the decoherence is not a practical limitation but a structural impossibility

**This is why there is no paradox:** the scenario that generates the contradiction (Wigner performing interference on the whole lab) is not physically realizable in FTD. The friend's measurement is an irreversible phase transition in the substrate, and no operation available to Wigner can undo it.

---

## 4. The Information-Theoretic Resolution

### 4.1 The C = 1 Speed Limit [AXIOM]

FTD postulates that information propagates at maximum speed $C = 1$ (one lattice unit per tick) within the 26-connected Moore neighborhood ([SPEC_FTD_LAGRANGIAN.md](../01_reference/SPEC_FTD_LAGRANGIAN.md), Axiom 1). This is not an approximation or an effective description — it is a structural constraint on the lattice dynamics.

**Consequence for Wigner's friend:** The friend's measurement result (the pattern of crystallized state field values) propagates outward from the lab at speed $C = 1$. Until the information reaches Wigner's location in the lattice, Wigner has no access to the result.

### 4.2 Wigner's Ignorance Has a Geometric Shape [THEOREM]

Wigner's ignorance is not abstract — it has a precise geometric characterization:

$$\mathcal{I}_W(t) = \Lambda \setminus \mathcal{L}_{\text{friend}}(t) \tag{4.1}$$

where $\mathcal{L}_{\text{friend}}(t)$ is the forward light cone of the friend's measurement event, defined as the set of lattice sites reachable from the measurement location within $t$ ticks at speed $C = 1$.

If the friend measures at site $\mathbf{v}_0$ and tick $t_0$, then:

$$\mathcal{L}_{\text{friend}}(t) = \{\mathbf{v} \in \Lambda : \|\mathbf{v} - \mathbf{v}_0\|_\infty \leq t - t_0\} \tag{4.2}$$

Wigner is in the ignorance region $\mathcal{I}_W$ if and only if $\|\mathbf{v}_W - \mathbf{v}_0\|_\infty > t - t_0$ — the measurement's light cone has not yet reached him.

### 4.3 The "Superposition" Is Wigner's Best Prediction [SELECTION]

When Wigner assigns a superposition state to the lab, he is doing optimal Bayesian inference given his information. He knows:
- The initial preparation of the spin system
- The structure of the friend's measurement apparatus
- The laws of physics (FTD update rules)

He does not know:
- The specific outcome of the friend's measurement (because $C = 1$ prevents him from knowing)

His superposition state is the **maximum-entropy description consistent with his information** — it is a statement about his predictive model, not about the physical state of the lab. In the language of the Existence Filter ([FOUND_THE_EXISTENCE_FILTER.md](FOUND_THE_EXISTENCE_FILTER.md)):

$$E(\Psi_{\text{Wigner}}) \neq E(\Psi_{\text{friend}}) \tag{4.3}$$

The friend's Existence Filter output contains the measurement result (real, definite). Wigner's Existence Filter output does not (his effective state is still in the superposition regime). But the underlying lattice state is the same — they differ only in what each observer has *access to*, not in what *exists*.

### 4.4 When Wigner Opens the Lab, He Learns — He Does Not Cause [THEOREM from determinism]

When Wigner opens the lab (i.e., when the light cone of the friend's measurement reaches Wigner's location), the following occurs:

1. Information about the friend's crystallized state field propagates to Wigner's lattice region
2. Wigner's own measurement apparatus undergoes its own ReLU crystallization, recording the result
3. Wigner updates his description from superposition to definite outcome

**This is a learning event, not a causal event** [THEOREM from FTD's deterministic axiom]:

- The friend's state field was already definite before Wigner opened the lab
- The outcome was determined by the flux configuration at the time of the friend's measurement
- Wigner's act of opening the lab does not alter the friend's record — it merely brings the information into Wigner's causal past
- The deterministic substrate [AXIOM] guarantees that the outcome was fixed from the moment the friend measured; Wigner's learning is a matter of information propagation, not state creation

---

## 5. Why Wigner Was Right

### 5.1 Wigner's Key Insight [SELECTION]

Wigner's original 1961 paper contained a profound insight: consciousness plays a fundamental role in measurement. He argued that the measurement chain (system → apparatus → observer) must terminate somewhere, and that consciousness is what terminates it. Without a conscious observer, the chain extends indefinitely — each link in the chain is "just more quantum system."

Wigner was right about the structural point: **something must terminate the measurement chain**, and that something must be qualitatively different from the systems being measured.

### 5.2 FTD Agrees: Consciousness IS the Type III₁ → Type I Transition [SELECTION]

FTD identifies consciousness with the self-referential closure that produces the algebraic type transition ([DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md](DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md), §4; [FOUND_SELF_REFERENTIAL_CLOSURE.md](../02_foundations/FOUND_SELF_REFERENTIAL_CLOSURE.md)):

$$\text{Consciousness} = \text{Self-referential closure} = \text{Type III}_1 \to \text{Type I transition} \tag{5.1}$$

The measurement chain terminates when a subsystem undergoes the ReLU crystallization — when the continuous potentiality of the flux field is crystallized into the discrete actuality of the state field. This crystallization IS the "collapse," and it requires a subsystem with sufficient internal complexity to sustain self-referential closure.

**The friend is such a subsystem.** The friend's nervous system is a lattice region with self-referential closure — signals propagate in loops, the system's output feeds back into its input, and the algebraic type of the friend's local algebra has the Type III₁ character required for conscious observation ([FOUND_THE_EXISTENCE_FILTER.md](FOUND_THE_EXISTENCE_FILTER.md), §4.2: "For Type III₁ factors, the $J$-fixed subspace is trivial — only scalars survive. This is the algebraic expression of first-person irreducibility").

### 5.3 But "Consciousness" Is Algebraic, Not Mystical [SELECTION]

The word "consciousness" in FTD does not invoke anything beyond the algebraic structure:

| What "consciousness" means in FTD | What it does NOT mean |
|---|---|
| A subsystem whose local algebra is Type III₁ | A soul, spirit, or immaterial substance |
| Self-referential closure (the system observes itself) | Human-level sentience specifically |
| The capacity to undergo the Type III₁ → Type I transition | A mysterious "collapse trigger" outside physics |
| Algebraically: a factor with trivial center and non-inner modular automorphisms | Anything that requires dualism |

Consciousness, in this algebraic sense, is a **structural property** of certain lattice subsystems — those with sufficient internal connectivity to support self-referential information loops. It is not "added to" the physics; it IS a specific kind of physics (the kind that produces Type III₁ algebras).

### 5.4 Wigner's Error: Conflating "Consciousness" with "My Consciousness" [SELECTION]

Wigner's mistake was not in claiming that consciousness matters for measurement. It was in implicitly assuming that **his** consciousness was the relevant one. The standard formulation of the paradox asks: "Has the state collapsed from Wigner's point of view?" — as if Wigner's perspective has a special status.

In FTD, the measurement chain terminates at the **first** self-referential subsystem that undergoes the Type III₁ → Type I transition. In the Wigner's friend scenario, that is the **friend**, not Wigner.

$$\text{Measurement chain: } \text{spin} \to \text{apparatus} \to \underbrace{\text{friend}}_{\text{first Type III}_1 \text{ subsystem}} \to \text{TERMINATED} \tag{5.2}$$

The chain does not extend to Wigner because it has already terminated. The friend's consciousness (self-referential closure) has already crystallized the outcome. Wigner's consciousness is irrelevant to the measurement — it only becomes relevant when Wigner performs his own measurement (opening the lab), at which point he is measuring the already-definite lab, not the original spin.

### 5.5 FTD Correction: ANY Self-Referential Subsystem Terminates the Chain [SELECTION]

The FTD resolution generalizes Wigner's insight:

> **Proposition WF-1** [SELECTION]. The measurement chain terminates at the first subsystem in the causal chain whose local algebra admits self-referential closure (Type III₁ character). This subsystem need not be human, need not be biological, and need not be "conscious" in any folk-psychological sense. It must only have the algebraic structure that supports the Type III₁ → Type I transition.

This is [SELECTION] rather than [THEOREM] because:
1. The identification of measurement with the Type III₁ → Type I transition is itself [SELECTION]
2. The criterion for "sufficient self-referential closure" is not precisely quantified — we do not have a sharp threshold for how much internal connectivity constitutes a Type III₁ subsystem
3. Whether non-biological systems (e.g., sufficiently complex measurement apparatuses) qualify is an empirical question [OPEN]

---

## 6. Extended Wigner's Friend (Frauchiger-Renner)

### 6.1 The No-Go Theorem Recalled

Frauchiger and Renner proved that assumptions Q, S, and C are jointly inconsistent (§2.2). Any physical theory must abandon at least one.

### 6.2 FTD Abandons Q — Quantum Mechanics Is Not the Substrate [AXIOM + SELECTION]

FTD's resolution is to abandon **Q** (universal validity of quantum mechanics), but in a precise and principled way:

| Level | Description | Theory |
|-------|-------------|--------|
| **Substrate** | Deterministic lattice, ternary states, local update rules | FTD axioms [AXIOM] |
| **Aggregate** | Statistical description of large ensembles | Quantum mechanics [SELECTION: QM emerges from substrate] |

Quantum mechanics is not the fundamental theory — it is the **aggregate description** that emerges when the deterministic substrate is coarse-grained over many lattice sites and many ticks. The superposition principle, Born rule, and unitary evolution are all emergent features of the statistical description, not axioms of the substrate.

**The Frauchiger-Renner argument assumes Q** — that quantum mechanics applies to ALL systems, including observers. FTD denies this: quantum mechanics applies to the *aggregate description* of systems, not to the substrate. The substrate is deterministic [AXIOM], and at the substrate level, there is always a fact of the matter about every measurement outcome.

### 6.3 There IS a Fact of the Matter at Every Level [THEOREM from determinism]

In the FTD substrate:

1. Every lattice site has a definite state $s(\mathbf{v}, t) \in \{-1, 0, +1\}$ at every tick [AXIOM]
2. Every lattice site has a definite flux $J(\mathbf{v}, t) \in \mathbb{R}^3$ at every tick [AXIOM]
3. The update rules are deterministic [AXIOM]
4. Therefore, the entire future evolution of the lattice is determined by the initial configuration [THEOREM]

There are no "superpositions" at the substrate level. The superposition is an artifact of the aggregate description — it arises because the observer lacks complete information about the substrate state and must average over the unknown degrees of freedom.

**For the Frauchiger-Renner scenario:** At the substrate level, every agent's measurement has a definite outcome, determined by the flux configuration at the time of measurement. The paradox arises only within the aggregate (quantum) description, where the assumption of universal superposition leads to contradictions. FTD dissolves the paradox by providing a substrate in which the quantum description is emergent, not fundamental.

### 6.4 Consistency of the Substrate [THEOREM]

**Assumption S (single outcome):** Preserved. Every measurement in FTD has exactly one outcome — the state field at the measurement site is either $-1$, $0$, or $+1$, never a superposition thereof.

**Assumption C (consistency):** Preserved. The deterministic substrate guarantees that any two agents who have access to the same information must agree on the outcome (§7 below).

**Assumption Q (universality):** Abandoned. Quantum mechanics is the aggregate description, not the substrate. The substrate is not quantum — it is a deterministic lattice theory from which quantum behavior emerges statistically.

FTD abandons the weakest of the three assumptions — the one that asserts a particular effective theory is universally valid — while preserving the two that express genuine physical requirements (single outcomes and inter-observer consistency).

---

## 7. Multi-Observer Consistency

### 7.1 Shared Causal Regions Guarantee Agreement [THEOREM from local causality + determinism]

**Theorem WF-T1.** Let observers $A$ and $B$ share a causal region — i.e., the forward light cone of the measurement event reaches both $A$ and $B$. Then $A$ and $B$ assign the same definite outcome to the measurement.

**Proof.** Let the measurement occur at site $\mathbf{v}_0$, tick $t_0$. The outcome is determined by $s(\mathbf{v}_0, t_0)$, which is a definite element of $\{-1, 0, +1\}$ [AXIOM: ternary states]. The information about this outcome propagates at speed $C = 1$ [AXIOM: local causality]. If both $A$ and $B$ are within the forward light cone — $\|\mathbf{v}_A - \mathbf{v}_0\|_\infty \leq t_A - t_0$ and $\|\mathbf{v}_B - \mathbf{v}_0\|_\infty \leq t_B - t_0$ — then both receive the same information: the value of $s(\mathbf{v}_0, t_0)$. Since the substrate is deterministic [AXIOM], this value is unique. Therefore $A$ and $B$ agree. $\square$

### 7.2 Disagreement Only for Spacelike-Separated Observers [THEOREM]

**Theorem WF-T2.** Observers $A$ and $B$ can assign different descriptions to a measurement outcome only if they are spacelike-separated with respect to the measurement event — i.e., the measurement's light cone has reached one but not the other.

**Proof.** By contraposition of WF-T1: if both are within the light cone, they agree. If neither is within the light cone, both are ignorant (and assign the same prior superposition description). The only case where they disagree is when one is inside and the other outside the light cone. $\square$

**This is exactly the Wigner's friend scenario:** the friend is inside the measurement's light cone (has received the information); Wigner is outside (has not). Their disagreement is not about the state of the world but about what each of them knows — it is epistemic, not ontological.

### 7.3 Comparison of Notes Forces Agreement [THEOREM from determinism]

**Theorem WF-T3.** When two observers who initially disagree (because one was outside the light cone) come into causal contact (their light cones overlap), they must agree on the measurement outcome.

**Proof.** When Wigner enters the friend's light cone (by opening the lab, or simply by waiting for $C = 1$ propagation to reach him), he receives the definite value $s(\mathbf{v}_0, t_0)$. His prior superposition description is updated to the definite outcome. The friend's description was already definite. Both now agree on the same value of $s$. Since the substrate is deterministic and the state field is unique, there is exactly one value they can agree on. $\square$

---

## 8. The Algebra of Observers

### 8.1 Observer Algebras [SELECTION]

Each observer $i$ is associated with a local algebra $\mathcal{A}_i \subset \mathcal{A}_{\text{total}}$ — the subalgebra of observables accessible to observer $i$, determined by the observer's causal past (the region of the lattice from which information could have reached the observer).

**Definition WF-D1** [SELECTION]. The *observer algebra* of observer $i$ at tick $t$ is:

$$\mathcal{A}_i(t) = \text{algebra generated by } \{s(\mathbf{v}, t'), J(\mathbf{v}, t') : \mathbf{v} \in \mathcal{P}_i(t)\} \tag{8.1}$$

where $\mathcal{P}_i(t) = \{\mathbf{v} \in \Lambda : \|\mathbf{v} - \mathbf{v}_i\|_\infty \leq t - t'\}$ is the causal past of observer $i$ — all sites from which information could have reached $\mathbf{v}_i$ by tick $t$.

### 8.2 Measurement as Algebraic Type Transition [SELECTION]

When observer $i$ measures a system, their observer algebra undergoes a type transition:

$$\mathcal{A}_i: \text{Type III}_1 \;\longrightarrow\; \text{Type I} \tag{8.2}$$

Before measurement: the algebra $\mathcal{A}_i$ has Type III₁ character — no minimal projections, continuous dimension function, non-trivial modular flow. The system is in a state of continuous potentiality.

After measurement: the algebra $\mathcal{A}_i$ contains the minimal projection $P_{\text{outcome}} = |s\rangle\langle s|$ — the definite outcome is recorded. The dimension function is discrete. The modular flow is trivial on this sector.

### 8.3 Partial Order of Knowledge [THEOREM from set inclusion]

**Theorem WF-T4.** The observer algebras form a partial order under inclusion:

$$\mathcal{A}_i(t) \subseteq \mathcal{A}_j(t) \quad \iff \quad \mathcal{P}_i(t) \subseteq \mathcal{P}_j(t) \tag{8.3}$$

**For Wigner's friend:** At the time of the friend's measurement (tick $t_0$), the friend's causal past includes the measurement event, but Wigner's does not. Therefore:

$$\mathcal{A}_{\text{Wigner}}(t_0) \subset \mathcal{A}_{\text{friend}}(t_0) \quad \text{(friend knows more than Wigner)} \tag{8.4}$$

When Wigner opens the lab (tick $t_1 > t_0$), the measurement event enters Wigner's causal past, and the algebras equalize:

$$\mathcal{A}_{\text{friend}}(t_0) \subseteq \mathcal{A}_{\text{Wigner}}(t_1) \tag{8.5}$$

### 8.4 Consistency Guaranteed by Deterministic Substrate [THEOREM]

**Theorem WF-T5.** For any two observers $i, j$ and any observable $O \in \mathcal{A}_i(t) \cap \mathcal{A}_j(t)$ (an observable accessible to both):

$$\langle O \rangle_i = \langle O \rangle_j \tag{8.6}$$

**Proof.** The value of $O$ is determined by the substrate state in $\mathcal{P}_i(t) \cap \mathcal{P}_j(t)$. Since the substrate is deterministic [AXIOM], this state is unique. Both observers, having access to the same substrate configuration, compute the same expectation value. $\square$

This is the algebraic expression of multi-observer consistency: disagreement arises only from different algebras (different causal access), never from different substrate states.

---

## 9. Epistemic Accounting

### 9.1 Classification of All Claims

#### Axioms [AXIOM]

| Statement | Source |
|-----------|--------|
| Discrete ternary states $s \in \{-1, 0, +1\}$ | FTD Postulate 3 |
| Local causality ($C = 1$, Moore neighborhood) | FTD Postulate 4 |
| Deterministic update rules | FTD Postulate 5 |
| Substrate is not quantum — QM is emergent | FTD ontological commitment |

#### Theorems [THEOREM]

| ID | Statement | Depends On |
|----|-----------|-----------|
| WF-T1 | Shared causal region → agreement | Determinism + locality [AXIOM] |
| WF-T2 | Disagreement only for spacelike-separated observers | WF-T1 (contraposition) |
| WF-T3 | Comparison of notes forces agreement | Determinism + unique substrate state [AXIOM] |
| WF-T4 | Observer algebras form partial order | Set inclusion (classical mathematics) |
| WF-T5 | Shared observables have unique values | Determinism [AXIOM] |
| §3.4 | Wigner cannot undo friend's measurement | Discreteness of $s$ [AXIOM] + irreversibility of crystallization |

#### Selections [SELECTION]

| ID | Statement | Argument basis |
|----|-----------|---------------|
| §3.1 | Measurement = ReLU crystallization (Type III₁ → Type I) | Structural correspondence: Softplus axioms ↔ KMS condition |
| §3.2 | Friend's measurement is ontologically real | Follows from §3.1 + locality |
| §3.3 | Wigner's superposition is epistemic | Follows from §3.2 + $C = 1$ speed limit |
| §5.1–5.5 | Consciousness = self-referential closure = chain termination | Algebraic structure argument |
| WF-1 | First Type III₁ subsystem terminates chain | Generalization of §5.2 |
| §6.2 | QM is aggregate description, not substrate | FTD ontological framework |
| WF-D1 | Observer algebra definition | Natural but not uniquely forced |

#### Conjectures [CONJECTURE]

| ID | Statement | What would validate it |
|----|-----------|----------------------|
| §3.1 (Eq. 3.3, last step) | Heaviside MASA selection ($\Theta(K)$) produces Type I | Rigorous proof that the Heaviside partition selects a MASA |
| §5.5 (implicit) | Sufficient internal connectivity criterion for Type III₁ | Quantitative threshold for self-referential closure |

#### Open Questions [OPEN]

| ID | Question | Priority |
|----|----------|----------|
| WF-O1 | What is the precise threshold of internal complexity at which a lattice subsystem qualifies as Type III₁? | **High** |
| WF-O2 | Can the irreversibility of ReLU crystallization be demonstrated in simulation (engine test: prepare a crystallized region, attempt reversal, verify failure)? | **High** |
| WF-O3 | Does the Frauchiger-Renner scenario have a lattice simulation analog? Can the engine run the extended Wigner's friend protocol and verify single-outcome consistency? | Medium |
| WF-O4 | Is the identification "QM = aggregate description" falsifiable? What experimental signature would distinguish FTD's epistemic superposition from genuine ontological superposition? | **High** |
| WF-O5 | Do non-biological measurement devices (e.g., photon detectors) qualify as Type III₁ subsystems, or is biological neural architecture required? | Medium |

### 9.2 What Is Novel

Three contributions not found in existing FTD documents:

1. **Explicit resolution of Wigner's friend via ontological/epistemic distinction.** The two-layer ontology (flux + state fields) has been discussed in many documents, but its application to Wigner's friend — showing that the friend's measurement is ontological while Wigner's superposition is epistemic — is new.

2. **The measurement chain termination criterion (WF-1).** The claim that the chain terminates at the first Type III₁ subsystem, and that this generalizes Wigner's insight about consciousness, has not been stated explicitly in prior documents.

3. **The algebra of observers formalism (§8).** The definition of observer algebras via causal past, the partial order of knowledge, and the consistency theorem (WF-T5) provide a new algebraic framework for multi-observer scenarios in FTD.

### 9.3 What Extends Existing Work

| Document Extended | What Is Extended | How |
|-------------------|------------------|-----|
| [FOUND_THE_EXISTENCE_FILTER.md](FOUND_THE_EXISTENCE_FILTER.md) | $E(x) = \text{Re}(x)$ as projection | Applied to Wigner's vs friend's descriptions (Eq. 4.3) |
| [EXPLR_RELU_TYPE_TRANSITION.md](../09_mathematical/EXPLR_RELU_TYPE_TRANSITION.md) | Type III₁ → Type I descent | Applied to measurement irreversibility argument (§3.4) |
| [DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md](DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md) | Collapse as algebraic transition | Extended to multi-observer setting with causal structure |
| [FOUND_SELF_REFERENTIAL_CLOSURE.md](../02_foundations/FOUND_SELF_REFERENTIAL_CLOSURE.md) | Self-referential closure principle | Applied as measurement chain termination criterion |

---

## 10. References

### FTD Documents

1. [FOUND_THE_EXISTENCE_FILTER.md](FOUND_THE_EXISTENCE_FILTER.md) — Existence Filter, projection hierarchy
2. [DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md](DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md) — Consciousness–QFT–GR bridge, algebraic descent
3. [EXPLR_RELU_TYPE_TRANSITION.md](../09_mathematical/EXPLR_RELU_TYPE_TRANSITION.md) — ReLU as Type III → Type I transition
4. [FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md](../02_foundations/FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md) — ReLU crystallization, emergent time
5. [FOUND_SELF_REFERENTIAL_CLOSURE.md](../02_foundations/FOUND_SELF_REFERENTIAL_CLOSURE.md) — Self-referential closure
6. [SPEC_FTD_LAGRANGIAN.md](../01_reference/SPEC_FTD_LAGRANGIAN.md) — Axioms, fields, action

### External Literature

7. Wigner, E. P. (1961). "Remarks on the mind-body question." In *The Scientist Speculates*, I. J. Good (ed.), pp. 284–302. Heinemann.
8. Frauchiger, D. & Renner, R. (2018). "Quantum theory cannot consistently describe the use of itself." *Nature Communications* **9**, 3711.
9. Murray, F. J. & von Neumann, J. (1936). "On rings of operators." *Annals of Mathematics* **37**(1), 116–229.
10. Connes, A. (1973). "Une classification des facteurs de type III." *Annales Scientifiques de l'ENS* **6**(2), 133–252.
11. Tomita, M. (1967). "On canonical forms of von Neumann algebras." Unpublished preprint.
12. Takesaki, M. (1970). "Tomita's theory of modular Hilbert algebras and its applications." *Lecture Notes in Mathematics* **128**. Springer.

---

*Wigner's Friend Resolution — Foundational Ternary Dynamics v5.28*
*Prepared for critical evaluation*
*March 17, 2026*
