# LEMMA — GNC pointwise rigidity: the flux Jacobian as a scaled local isometry

**LEDGER row:** FTD-0354 (row written by the controller; this document does not edit the LEDGER)
**Date:** 2026-07-02
**Provenance:** extension **E1** of [`ASSESSMENT_MATH_GRADES_AND_EXTENSIONS_2026-07-01.md`](../../07_assessment/ASSESSMENT_MATH_GRADES_AND_EXTENSIONS_2026-07-01.md) (FTD-0352), executed as proposed; grounded in FTD-0349 ([`DERIV_CLUSTER_COLLECTIVE_COORDINATE_v1.md`](DERIV_CLUSTER_COLLECTIVE_COORDINATE_v1.md)).
**Tag:** **[THEOREM]** for the rigidity lemma, the divergence identity, and the affine classification (§2–§4 — elementary linear algebra about the FTD-0349 ansatz class); **[THEOREM]** for the lattice non-rigidity exhibit (§5); **[CONJECTURE]** for the two-walls-one-shape unification (§6). **Zero promotions:** FTD-0110/FTD-0250 tags unchanged; the clock hypothesis stays **[AXIOM]** (FTD-0208); GNC stays un-forced; whether real engine clusters satisfy GNC stays **[OPEN]** — it is the pre-registered FTD-0349 §9 Q_ij measurement, which this document does not touch. FC-1, FC-2, FC-W untouched.
**Verification:** [`scripts/proofs/proof_gnc_rigidity.py`](../../../../scripts/proofs/proof_gnc_rigidity.py) — **27/27 PASS** (run of record 2026-07-02; test ids R1–R8 cited inline).
**Read first:** FTD-0349 §2–§3, §7–§9; [FTD-0208 LEDGER row](../../07_assessment/core_ledgers/LEDGER.md) (clock hypothesis [AXIOM]); [`SPEC_FTD_LAGRANGIAN.md`](../../01_reference/SPEC_FTD_LAGRANGIAN.md) §4.3.

---

## §0 · Verdict

> **The Gradient-Normalization Condition at a site is a rigidity condition: GNC-s at x ⟺ (∇J)ᵀ(∇J) = K_B²·I at x ⟺ ∇J(x) = K_B·Q with Q ∈ O(3).** A GNC-s flux field is precisely a field whose discrete Jacobian is everywhere a K_B-scaled isometry. Charge-freedom adds tr Q = 0; within the affine ansatz class the solutions then split into exactly two strata — the proper stratum, which is the θ = 2π/3 rotation family of FTD-0349 §7 (θ = 2π/3 is the **unique** rotation angle), and its previously unrecorded improper (parity-reversed) partner −R(n̂, 2π/3). The affine classification is exhaustive in the continuum-C² reading but **not** on the lattice: divergence-free non-affine GNC-s folds exist (§5). The clock hypothesis and GNC are the d = 1 and d = 3 instances of one pinned-operator-norm condition on dJ (§6) — stated as **[CONJECTURE]**, an identification of shape, not a derivation of either wall.

Everything in §2–§5 is mathematics about the FTD-0349 rigid-transport ansatz class. Whether the engine's actual dressed clusters satisfy GNC is not addressed here and cannot be: that is the pre-registered engine measurement of Q_ij (FTD-0349 §9), unchanged by this document.

Sub-verdicts, tagged:

| # | Claim | Tag | Verified |
|---|-------|-----|----------|
| 1 | GNC-s at x ⟺ (∇J)ᵀ(∇J) = K_B²I ⟺ ∇J/K_B ∈ O(3) (three-way, per site) | [THEOREM] | R1a–R1e |
| 2 | Lattice divergence of an affine field = tr(∇J) sitewise (fwd and bwd stencils); charge-free ⟹ tr Q = 0 | [THEOREM] | R2a–R2c |
| 3 | Affine GNC + div-free classification: exactly two strata, {R(n̂, 2π/3)} ⊔ {−R(n̂, 2π/3)}; θ = 2π/3 unique in SO(3) — recovers FTD-0349 §7 as the proper stratum | [THEOREM] | R3a–R3e, R4a–R4c |
| 4 | Born-Infeld resummation to −NK_B√(1−V²) is det-blind (both strata resum identically) | [THEOREM] | R4d |
| 5 | GNC-w (summed) is strictly weaker than GNC-s (pointwise) | [THEOREM — example] | R5a |
| 6 | Affine classification NOT exhaustive on the lattice: divergence-free non-affine GNC-s folds exist | [THEOREM — existence] | R6a–R6g |
| 7 | Continuum companion: C² fields with (∇J)ᵀ(∇J) = K_B²I on a connected domain are affine | [THEOREM — external-classical; key step verified] | R7a |
| 8 | Clock hypothesis and GNC-s = d = 1 and d = 3 instances of one pinned-operator-norm condition on dJ | **[CONJECTURE]** (unification); the d = 1 shape identity itself is trivial [THEOREM] | R8a |
| 9 | Whether engine clusters realize GNC | **[OPEN]** — FTD-0349 §9 Q_ij measurement, untouched | — |

---

## §1 · Setting

Let J: Λ → ℝ³ be the flux field and define the (discrete, forward-difference) **Jacobian** at site x as the 3×3 matrix

$$D(\mathbf{x})_{ai} \;=\; (\Delta^+_i J_a)(\mathbf{x}) \;=\; J_a(\mathbf{x}+\mathbf{e}_i) - J_a(\mathbf{x}), \qquad a,i \in \{1,2,3\}.$$

FTD-0349 Eq. (3) defines the pointwise condition

$$\text{GNC-s at } \mathbf{x}:\qquad \big|(\hat V\!\cdot\!\nabla)J(\mathbf{x})\big| = K_B \quad\text{for every unit } \hat V \in \mathbb{R}^3,$$

where $(\hat V\!\cdot\!\nabla)J$ has components $\sum_i \hat V_i\, \Delta^+_i J_a = (D\hat V)_a$, so that $|(\hat V\!\cdot\!\nabla)J|^2 = \hat V^{\top} D^{\top} D\, \hat V$. The summed form GNC-w of FTD-0349 is $\sum_{\mathbf{x}} (D^{\top}D)(\mathbf{x}) = N K_B^2\, \mathbb{1}$; GNC-s at every member site implies GNC-w, and not conversely (§4.4, R5a). Throughout, K_B > 0 (the FTD-0041 MeV-anchored calibration constant, K_B = 0.511 [IMPOSED]; only K_B ≠ 0 is used in the algebra). Convention note: $D^{\top}D = K_B^2 I \Leftrightarrow DD^{\top} = K_B^2 I$ for 3×3 matrices (both say $D/K_B$ orthogonal), so the row/column convention for the Jacobian is immaterial to every statement below. The forward-difference matrix $D(\mathbf{x})$ is the exact Jacobian for affine fields and the first-order estimator otherwise; the lemma is exact matrix algebra about $D(\mathbf{x})$, whatever field produced it.

---

## §2 · The rigidity lemma [THEOREM]

**Lemma (pointwise rigidity).** For a real 3×3 matrix $D = D(\mathbf{x})$ and $K_B > 0$, the following are equivalent:

1. **(GNC-s)** $|D\hat V| = K_B$ for every unit vector $\hat V \in \mathbb{R}^3$;
2. **(Gram)** $D^{\top} D = K_B^2\, I$;
3. **(scaled isometry)** $D = K_B\, Q$ with $Q \in O(3)$.

*Proof.* **(1) ⟹ (2).** By homogeneity, $V^{\top}(D^{\top}D)V = K_B^2 |V|^2$ for all $V \in \mathbb{R}^3$. The symmetric matrix $S = D^{\top}D - K_B^2 I$ then satisfies $V^{\top} S V = 0$ for all $V$; by polarization, $u^{\top} S v = \tfrac14\big[(u{+}v)^{\top}S(u{+}v) - (u{-}v)^{\top}S(u{-}v)\big] = 0$ for all $u, v$, hence $S = 0$. **(2) ⟹ (3).** $Q := D/K_B$ satisfies $Q^{\top}Q = I$, i.e. $Q \in O(3)$ (here $K_B \neq 0$ is used). **(3) ⟹ (1).** $|K_B Q \hat V| = K_B |Q\hat V| = K_B$. ∎

Verified R1a–R1c, including the polarization mechanism reconstructing $S$ from unit-sphere data alone, and the negation (R1d): a non-orthogonal $D$ always admits a unit direction violating (1) — the top or bottom eigenvector of its Gram — so the equivalence is not vacuous.

**Corollary (spectrum pinned).** GNC-s at x pins the **entire singular spectrum** of $D(\mathbf{x})$ at K_B — all three singular values equal K_B (R1e), not merely the operator norm. Geometrically: a GNC-s flux field is one whose site-to-site first-order behavior is a **local isometry up to the single scale K_B** — transport in any direction swings the flux by exactly K_B per hop, with no preferred axis. This is the precise sense in which FTD-0349 §3's phrase "swing the local flux by exactly K_B, isotropically" is a rigidity statement.

---

## §3 · Divergence: charge-freedom forces trace zero [THEOREM]

For an affine field $J(\mathbf{x}) = A\mathbf{x} + \mathbf{c}$ the forward differences are exact and site-independent, $D(\mathbf{x}) = A$ everywhere, and the lattice divergence — forward $\sum_i \Delta^+_i J_i$ or backward $\sum_i \Delta^-_i J_i$ (the Gauss-stencil convention matched to $J = -\Delta^+\varphi$) — equals $\operatorname{tr} A$ at every interior site (R2a, R2b; both conventions agree because affine fields have constant differences). Hence for a GNC-s affine field $A = K_B Q$:

$$\nabla\!\cdot\! J = K_B \operatorname{tr} Q \quad\text{sitewise}, \qquad\text{so charge-freedom } (\nabla\!\cdot\! J = 0) \iff \operatorname{tr} Q = 0. \tag{R2c}$$

---

## §4 · Classification of the affine solutions [THEOREM]

**Proposition.** The affine fields $J = K_B Q\,\mathbf{x} + \mathbf{c}$ satisfying GNC-s at every site **and** charge-freedom are classified by

$$\{\,Q \in O(3) : \operatorname{tr} Q = 0\,\} \;=\; \mathcal{C}_+ \,\sqcup\, \mathcal{C}_-,$$

$$\mathcal{C}_+ = \{\,R(\hat n, 2\pi/3) : \hat n \in S^2\,\} \quad (\det = +1,\ \text{eigenvalues } \{1, e^{\pm 2\pi i/3}\}),$$
$$\mathcal{C}_- = \{\,-R(\hat n, 2\pi/3) : \hat n \in S^2\,\} \quad (\det = -1,\ \text{eigenvalues } \{-1, e^{\pm i\pi/3}\}),$$

and the constant $\mathbf{c}$ is free (it drops out of every difference).

*Proof.* Any $Q \in O(3)$ has $\det Q = \pm 1$.
**Proper component.** $R \in SO(3)$ has eigenvalues $\{1, e^{i\theta}, e^{-i\theta}\}$ for a rotation angle $\theta \in [0, \pi]$, so $\operatorname{tr} R = 1 + 2\cos\theta$, which is strictly decreasing from 3 to −1 on $[0,\pi]$; hence $\operatorname{tr} R = 0 \iff \cos\theta = -\tfrac12 \iff \theta = 2\pi/3$, **uniquely** (R3a, R3b). The trace-zero proper matrices are therefore exactly one conjugacy class — rotations by 2π/3 about an arbitrary axis, a 2-sphere of solutions (R3c: every trace-zero proper orthogonal matrix reconstructs as $R(\hat n, 2\pi/3)$ from its fixed axis).
**Improper component.** In odd dimension $\det(-Q) = -\det Q$, so $Q \in O(3)\setminus SO(3) \iff -Q \in SO(3)$ (R4a). Then $\operatorname{tr} Q = 0 \iff \operatorname{tr}(-Q) = 0 \iff -Q \in \mathcal{C}_+$, i.e. $Q = -R(\hat n, 2\pi/3)$. Its eigenvalues are the negatives $\{-1, -e^{\pm 2\pi i/3}\} = \{-1, e^{\mp i\pi/3}\}$ — a rotoreflection through angle π/3 (R4b). The strata are disjoint by determinant. ∎

**§4.1 — FTD-0349 §7 recovered.** The proper stratum $\mathcal{C}_+$ is *exactly* the FTD-0349 texture family ($J = K_B R\,\mathbf{x}$, $R \in SO(3)$, $\operatorname{tr} R = 0 \iff \theta = 2\pi/3$), now with uniqueness rather than existence: θ = 2π/3 is the **only** rotation stratum, and the cyclic coordinate permutation $P_{\text{cyc}} = R\big((1,1,1)/\sqrt3,\, 2\pi/3\big)$ exactly (R3d). Sitewise Gram $= K_B^2 I$ and vanishing divergence (both stencils) verified for random-axis members (R3e). As in FTD-0349, no physical identification of the 2π/3 angle is made or implied; it enters solely as the zero of $1 + 2\cos\theta$. [Anti-target discipline applied.]

**§4.2 — the improper stratum is new.** FTD-0349 §7 quantified over $SO(3)$ only; the rigidity lemma shows GNC-s admits all of $O(3)$, and charge-freedom then admits the parity-reversed family $\mathcal{C}_-$ on equal footing. Concrete member: $Q = -P_{\text{cyc}}$ (orthogonal, trace 0, det −1), whose lattice texture is GNC-s and divergence-free sitewise (R4c).

**§4.3 — inertia is parity-blind at this order [THEOREM].** The Born-Infeld resummation of FTD-0349 §2 uses only $|Q\hat V| = 1$: per-member transport speed $v_m = |D V|/K_B = |V|$, so $\sum_m \big[-K_B\sqrt{1 - v_m^2}\big] = -N K_B \sqrt{1 - V^2}$ for **both** strata (R4d). A parity-reversed texture is indistinguishable from its proper partner in the collective inertia; this echoes the sign-blindness of the FTD-0349 dressing trace identity (ρ̃² counts ±1 alike) — the inertia-relevant structures are consistently blind to the discrete labels. [Observation; no physics claim about parity is made.]

**§4.4 — GNC-w does not imply GNC-s [THEOREM — example].** Two sites with Gram matrices $K_B^2\,\mathrm{diag}(2,1,0)$ and $K_B^2\,\mathrm{diag}(0,1,2)$ sum to $2K_B^2 I$ while neither site is isotropic (R5a). Consequence for the pre-registered measurement: $Q_{ij} \approx \delta_{ij}$ (the FTD-0349 §9 observable, a *summed* member-site form) gates GNC-w — exactly what the Newtonian-limit reduction needs at O(V²) — but is necessary-not-sufficient for GNC-s, which the full γ_FTD resummation needs (FTD-0349 §2 already draws this distinction; the lemma sharpens what the pointwise form geometrically *is*). This is a reading aid for the measurement's interpretation, not a change to its pre-registration.

---

## §5 · Beyond affine: rigidity holds in the continuum, fails on the lattice

**§5.1 — continuum companion [THEOREM, classical].** If $J \in C^2(\Omega, \mathbb{R}^3)$ on a connected open $\Omega \subseteq \mathbb{R}^3$ satisfies $(\nabla J)^{\top}(\nabla J) = K_B^2 I$ pointwise, then $J$ is affine, $J = K_B Q\,\mathbf{x} + \mathbf{c}$.

*Proof.* Set $A_{ij,k} = \partial_i \partial_j J_a\, \partial_k J_a$ (sum over $a$). Differentiating the constant Gram, $0 = \partial_i(\partial_j J_a \partial_k J_a) = A_{ij,k} + A_{ik,j}$; symmetry of second derivatives gives $A_{ij,k} = A_{ji,k}$. Then
$$A_{ij,k} = A_{ji,k} = -A_{jk,i} = -A_{kj,i} = A_{ki,j} = A_{ik,j} = -A_{ij,k} \implies A \equiv 0.$$
Since the Gram is $K_B^2 I$, the vectors $\{\partial_k J\}_{k}$ span $\mathbb{R}^3$, so $A_{ij,k} = 0$ for all $k$ forces $\partial_i\partial_j J_a = 0$: $J$ is affine, and §2 gives the form. ∎ (The symmetry dance is the standard rigidity-of-local-isometries argument; its algebraic core — that the only tensor symmetric in $(i,j)$ and antisymmetric in $(j,k)$ is zero — is verified as a 27-dimensional rank computation, R7a.) With the determinant constrained to one sign the $C^2$ hypothesis can be relaxed substantially (Lipschitz maps with gradient a.e. in $SO(3)$ are affine — Reshetnyak-type rigidity [external mathematics, cited not proven here]); with full $O(3)$ allowed, folds break it even in the continuum, e.g. $\mathbf{x} \mapsto (|x_1|, x_2, x_3)$.

**§5.2 — lattice folds exist; one is divergence-free [THEOREM — existence].** The affine classification is **not** exhaustive on the lattice:

- **Walk fold (GNC-s, charged).** $J = K_B\,(f(x_1), x_2, x_3)$ with $f$ any ±1-increment walk has sitewise Gram $K_B^2 I$ everywhere (R6a) — an enormous non-affine solution family — but its divergence is $K_B(\pm 1 + 2) \in \{K_B, 3K_B\}$, never zero (R6b): charge-freedom is a genuine constraint on folds, not a formality.
- **Trace-zero fold (GNC-s and divergence-free).** $J(\mathbf{x}) = K_B\, P_{\text{cyc}}\, (|x_1 - x_0|,\, x_2,\, x_3)^{\top}$ satisfies sitewise Gram $= K_B^2 I$ at **every** interior site, has vanishing divergence at every interior site under both difference conventions, and is non-affine: its Jacobian takes exactly the two orthogonal values $K_B P_{\text{cyc}}$ and $K_B P_{\text{cyc}}\sigma_1$ ($\sigma_1 = \mathrm{diag}(-1,1,1)$), one on each side of the fold plane (R6c–R6e). The two branch matrices are rank-one connected (the fold-compatibility condition, R6f), and both are trace-zero because $(P_{\text{cyc}})_{11} = 0$; in general a 2π/3 rotation folds trace-zero across the plane $x_1 = x_0$ iff its axis satisfies the magic-angle condition $n_1^2 = 1/3$, since $R(\hat n, 2\pi/3)_{11} = -\tfrac12 + \tfrac32 n_1^2$ (R6g). $P_{\text{cyc}}$'s axis $(1,1,1)/\sqrt3$ meets it for every coordinate plane.

Consequence, stated plainly: **the θ = 2π/3 affine texture is the unique *affine* charge-free GNC-s stratum (up to parity), not the unique charge-free GNC-s field.** The lattice admits laminated GNC-s textures — the discrete analog of the continuum $O(3)$-gradient folding phenomenon. This matters for reading the future Q_ij measurement: an engine cluster could realize GNC without looking anywhere near affine, so a Q ≈ δ result would confirm GNC-w without licensing any inference to the affine texture family. [Interpretation note for FTD-0349 §9; the measurement itself is untouched.]

---

## §6 · Two walls, one shape [CONJECTURE]

The two named per-voxel walls of the mass sector:

- **Wall T (temporal) — the clock hypothesis.** FTD-0208 **[CLOSED NEGATIVE derivation ⟹ AXIOM]**; used in [`SPEC_FTD_LAGRANGIAN.md`](../../01_reference/SPEC_FTD_LAGRANGIAN.md) §4.3 [THEOREM conditional on it]. Content: along a rest worldline, the internal flux swing per **tick** is pinned at K_B.
- **Wall S (spatial) — GNC-s.** FTD-0349; un-forced by the action or the Gauss constraint. Content: at a member site, the flux swing per unit lattice **hop** is pinned at K_B, isotropically.

By §2, Wall S says: the spatial first-difference operator $\nabla J$ is a K_B-scaled isometry of the 3-dimensional hop domain. Wall T is literally the $d = 1$ instance of the same lemma: for a scalar rate $a = $ (flux swing per tick), $|a| = K_B \iff a^2 = K_B^2 \iff a/K_B \in O(1) = \{\pm 1\}$ (R8a) — a K_B-scaled isometry of the 1-dimensional tick domain. So both walls have the **provably identical shape** [THEOREM, trivial given §2]:

> the flux first-difference map $dJ$, restricted to unit displacements of its domain (one tick, or one hop in any direction), acts as a **K_B-scaled isometry** — equivalently, its entire singular spectrum on that domain is pinned at K_B.

("Pinned operator norm" is the loose name; the precise condition is the pinned *spectrum* — for $d = 1$ the two coincide, for $d = 3$ GNC-s is strictly stronger than a norm bound.)

**The conjecture** [CONJECTURE]: Walls T and S are two instances of **one** substrate condition — a single pinned-operator-norm law on $dJ$ over the full (1+3) displacement domain — rather than two independently imported axioms that happen to share a form. What is *proven* here is only the shared shape; what would substantiate the unification is either (a) a single covariant statement from which both legs follow (none exists — and neither leg is separately forced: FTD-0208 closed the temporal derivation negative, FTD-0349 showed nothing forces the spatial leg), or (b) the engine measurement finding both legs realized with the same constant K_B. Note the conjecture deliberately does **not** assert a joint (1+3) isometry — GNC-s and the clock hypothesis pin the spatial block and the temporal column separately and say nothing about a mixed spacetime Gram; writing down the honest joint object is part of what a v2 of the conjecture would owe. [Precision note.] The $d=1$ isometry group $O(1) = \{\pm1\}$ being a bare two-valued branch choice is the shape FTD-0340 names; pointer only, nothing promoted. [Observation.]

This observation **feeds** the four-walls-are-one forcing question ([`FOUND_MODULUS_ARGUMENT_FRONTIER.md`](../../02_foundations/FOUND_MODULUS_ARGUMENT_FRONTIER.md) §7, T2: whether FC-1, FC-2, FC-W and the L²-not-L¹ budget are one import) by exhibiting two *mass-sector* walls that provably share a single shape. It does **not** resolve that question, does not touch FC-1/FC-2/FC-W, and does not move FTD-0208 or FTD-0349: both walls remain imported exactly as tagged. [CONJECTURE, scope closed.]

---

## §7 · Scope

Everything proven here is linear algebra and lattice combinatorics about the FTD-0349 ansatz class (rigid transport, forward-difference Jacobian, weak field). The lemma characterizes what GNC-s *is*; it says nothing about whether the substrate dynamics *produces* it — that is the pre-registered FTD-0349 §9 engine measurement of $Q_{ij}$ on real locked clusters (CUDA/WSL2 engine as canonical instrument, gates fixed before running), which remains **[OPEN]** and is the only path by which FTD-0250 could move to [DERIVED given engine-verified GNC]. The classification adds two reading aids for that measurement (§4.4: Q gates the summed form only; §5.2: GNC-realization need not look affine) and changes nothing in its pre-registration. No number in this document is compared to a physical constant; θ = 2π/3 and the magic angle $n_1^2 = 1/3$ enter as zeros of trace polynomials only.

---

## §8 · Verification artifacts

[`scripts/proofs/proof_gnc_rigidity.py`](../../../../scripts/proofs/proof_gnc_rigidity.py) — **27/27 PASS**. R1 three-way rigidity equivalence (800-draw scaled-orthogonal sweep, polarization reconstruction from unit-sphere data, non-orthogonal negation, pinned singular spectrum); R2 affine divergence = trace (forward and backward stencils); R3 proper stratum (trace formula, strict monotonicity + unique zero, reconstruct-and-match exhaustiveness, $P_{\text{cyc}}$ identity, sitewise texture checks); R4 improper stratum (−R decomposition, rotoreflection eigenvalues, $-P_{\text{cyc}}$ texture, det-blind BI resummation); R5 GNC-w ⇏ GNC-s two-site example; R6 lattice folds (walk fold Gram + nonzero divergence, trace-zero fold Gram/divergence/two-valued Jacobian/rank-one connection, magic-angle formula); R7 27-dimensional null-space check of the continuum-rigidity symmetry dance; R8 the $d = 1$ clock-hypothesis shape.

*No engine campaigns were run for this document; no LEDGER, META_INDEX, or tracker file is edited by it.*
