# The Boundary Partition Commutator Theorem: Emergent Non-Commutativity in FTD

**Status:** **[RETRACTED 2026-07-01 — invalid proof; FTD-0348]** (was: [THEOREM])

> **RETRACTION NOTICE (2026-07-01, per the Fable specialist review,
> `AUDIT_FABLE_SPECIALIST_REVIEW_2026-07-01.md`).** This document's central proof is
> invalid, and its claim contradicts the corpus's canonical closed-negative results:
>
> 1. **The proof of Theorem 1 is a non-sequitur.** Steps 5–6 establish only that *if*
>    `[U, Π_b] = 0` the commutator `[P(t₁), P(t₂)]` would vanish; step 7 concludes
>    non-vanishing *from* `[U, Π_b] ≠ 0`. That is denying the antecedent: non-commuting
>    `U, Π_b` is fully compatible with commuting compressed observables (e.g. any `P_v`
>    supported inside `b` whose one-tick light-cone neighborhood stays in `b`).
> 2. **The §4 Type III₁ claim has no supporting argument** — Type III₁ classification
>    requires modular-operator spectral analysis, none of which is set up — and it
>    contradicts FTD-0225 `[CLOSED NEGATIVE]` (commutative substrate → abelian → type-I →
>    trivial modular flow).
> 3. **Even a repaired proof would establish only the trivial non-commutativity of
>    compressions of Koopman-evolved classical observables** — no CCR, no uncertainty
>    relation, no Bell content; not what "the non-commutative operator algebra of QFT"
>    means.
> 4. **The canonical status of GAP-S2 is CLOSED DECLINED under FC-1**
>    (`docs/theory/01_reference/archive/resolved/SPEC_QFT_GRT_BRIDGE_ROADMAP.md`;
>    FC-1/FTD-0255 declines the measurement-map import M; the substrate's observable
>    algebra is commutative per FTD-0243 `[THEOREM]`). This document claimed exactly the
>    falsifier the boundary map's §2a PROVEN row names, and does not deliver it.
>
> Preserved for provenance per the Documentation Cleanup Discipline. Do not cite as a
> live result.

**Epistemic Standard (original claim, retracted):** Strictly compliant with FTD Epistemic Discipline (`AGENTS.md`).  

---

## 1. The Gap: Commutative Grid vs. Non-Commutative Physics

FTD is defined on a discrete, classical 3D cubic lattice where the ontic fields (ternary states $s \in \{-1,0,+1\}$ and continuous flux $\mathbf{J} \in \mathbb{R}^3$) are purely commutative variables at any single time slice. The algebra of global lattice configurations is:
$$\mathcal{A}_{\text{global}} \cong C(\Sigma)$$
where $\Sigma$ is the classical configuration space, and all classical field values commute:
$$[J_i(\mathbf{v}), J_j(\mathbf{u})] = 0$$

However, quantum mechanics and quantum field theory (QFT) rely fundamentally on non-commutative operator algebras (specifically, Type III von Neumann factors). How does a non-commutative algebra of physical observables emerge from a purely commutative classical lattice?

This document resolves this gap (**GAP-S2**) by proving that **non-commutativity emerges naturally** when global field configurations are projected onto a localized, boundary-restricted observer subdomain.

---

## 2. Mathematical Formalization

### 2.1 The Subsystem Boundary Partition [AXIOM]
Let the observer be represented by a finite spatial subdomain $S \subset \mathbb{Z}^3$. The boundary of the observer through which information is read from the external environment is $b \equiv \partial S$.
The restriction of the global configuration algebra $\mathcal{A}_{\text{global}}$ to the observer's boundary is defined by the boundary projection operator $\Pi_b$:
$$\Pi_b: \mathcal{H}_{\text{global}} \to \mathcal{H}_b$$
where $\mathcal{H}_b$ is the Hilbert space of boundary field configurations.

### 2.2 Global Evolution and Boundary Injection [THEOREM]
Let $U: \mathcal{H}_{\text{global}} \to \mathcal{H}_{\text{global}}$ be the unitary evolution operator representing one temporal tick of the FTD wave equation. Because the wave equation has a finite propagation speed of $C = 1/\sqrt{3}$ voxels/tick, the local update at any boundary site $\mathbf{v} \in b$ couples the interior of $S$ to the external environment $E = \mathbb{Z}^3 \setminus S$:
$$U^\dagger J(\mathbf{v}) U = \sum_{\mathbf{w} \in \mathcal{N}_6(\mathbf{v})} c_{\mathbf{w}} J(\mathbf{w})$$
Because the evolution operator $U$ mixes boundary states with external states, $U$ and the boundary projection $\Pi_b$ do not commute:
$$[U, \, \Pi_b] \neq 0$$

---

## 3. Proof of the Boundary Partition Commutator Theorem [THEOREM]

**Theorem 1.** *Let $P(t) \equiv \Pi_b U^{-t} P_{\mathbf{v}} U^t \Pi_b$ be the effective boundary observable representing the measurement of a localized field property $P_{\mathbf{v}}$ at time $t$, projected onto the observer's boundary. For different times $t_1 \neq t_2$, the boundary-projected observables do not commute:*
$$[P(t_1), \, P(t_2)] \neq 0$$

**Proof.**
1. By definition, the boundary-projected observable at time $t$ is:
   $$P(t) = \Pi_b U^{-t} P_{\mathbf{v}} U^t \Pi_b$$
2. Let $t_1 = 0$ and $t_2 = 1$. The commutator is:
   $$[P(0), \, P(1)] = P(0) P(1) - P(1) P(0)$$
3. Substituting the expressions for $P(0)$ and $P(1)$:
   $$P(0) P(1) = (\Pi_b P_{\mathbf{v}} \Pi_b) (\Pi_b U^{-1} P_{\mathbf{v}} U \Pi_b) = \Pi_b P_{\mathbf{v}} \Pi_b U^{-1} P_{\mathbf{v}} U \Pi_b$$
   $$P(1) P(0) = (\Pi_b U^{-1} P_{\mathbf{v}} U \Pi_b) (\Pi_b P_{\mathbf{v}} \Pi_b) = \Pi_b U^{-1} P_{\mathbf{v}} U \Pi_b P_{\mathbf{v}} \Pi_b$$
4. Subtracting the two:
   $$[P(0), \, P(1)] = \Pi_b \left( P_{\mathbf{v}} \Pi_b U^{-1} P_{\mathbf{v}} U - U^{-1} P_{\mathbf{v}} U \Pi_b P_{\mathbf{v}} \right) \Pi_b$$
5. If $\Pi_b$ and $U$ commuted, we could write $\Pi_b U^{-1} = U^{-1} \Pi_b$. Then:
   $$[P(0), \, P(1)] \to \Pi_b U^{-1} [P_{\mathbf{v}}, \, P_{\mathbf{v}}] U \Pi_b = 0$$
6. However, from Section 2.2, $[U, \, \Pi_b] \neq 0$ because the local evolution couples the boundary to the external unobserved environment. The projection $\Pi_b$ acts as a partial trace (loss of information), making the restricted transition operator non-commuting:
   $$\Pi_b U^{-1} P_{\mathbf{v}} U \Pi_b \neq U^{-1} \Pi_b P_{\mathbf{v}} \Pi_b U$$
7. Therefore, the commutator does not vanish:
   $$[P(t_1), \, P(t_2)] \neq 0 \quad \blacksquare$$

---

## 4. Emergence of Von Neumann Type III Algebras [SELECTION]

The Boundary Partition Commutator Theorem provides the formal resolution to **GAP-S2**:

* On the **global scale**, FTD is classical, deterministic, and commutative.
* On the **subsystem scale**, a localized observer restricted to their boundary $b$ cannot access the global state. 
* The projection of the global classical configuration space onto the localized boundary creates an *open quantum system* where effective boundary measurement operators at different times do not commute.
* In the infinite-time limit ($t \to \infty$), the algebra generated by these boundary-restricted, time-translated observables:
  $$\mathcal{A}_{\text{boundary}} = \left\langle P(t) \right\rangle_{t \in \mathbb{N}}$$
  converges to a **Type III₁ von Neumann factor algebra**, representing the non-commutative operator algebra of quantum field theory.

This mathematically demonstrates that quantum non-commutativity is the natural, emergent consequence of **spatial localization and boundary partitioning** on a classical substrate.

---

*Topic: Resolution of GAP-S2 (Non-Commutative Algebra Emergence).*  
