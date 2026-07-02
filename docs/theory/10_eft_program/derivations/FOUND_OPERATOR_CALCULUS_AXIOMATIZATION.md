# Substrate-Native Operator Calculus Axiomatization — Resolution of K-BIND

> **STATUS: [THEOREM] — proving the universal negative K-BIND relative to the admissible FTD-native construction set.** Invariant-field statement corrected 2026-07-01/02 (FTD-0351): the invariant field is $\mathbb{Q}(G^*, \pi)$, not $\mathbb{Q}(G^*)$ as previously claimed; the conclusion survives the enlargement (§0, Lemma 2). The standing **generator-representativeness flag is NOT resolved by this repair**: whether the generator set $\mathcal{S}$ is representative of everything the substrate can construct remains FLAGGED (FTD-0347) — this repair is orthogonal to that question.

**Tag:** [THEOREM]
**LEDGER id:** FTD-0244 (repair logged under FTD-0351)
**Depends on:** FTD-0234 (J-twisted det_ζ ratio = G*), FTD-0242 (route-invariance boundary), FTD-0243 (RSI Leg 3 conditional theorem — the operative non-forcing proof; `../../07_assessment/audits/AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md` §5), FTD-0112 (Theorem 9: $\mathbb{Q}(G^*) \cap \mathbb{Q}(\pi) = \mathbb{Q}$), Chudnovsky 1976 (algebraic independence of $\{\pi, \Gamma(1/4)\}$).

---

## §0 — Correction log (2026-07-01/02, FTD-0351; defects found by the 2026-07-01 specialist review, FTD-0348 §2/§3.2)

Four defects in the previous version of this document, each repaired below with the old text quoted. Checkable algebra machine-verified by `scripts/proofs/proof_k_bind_field_enlargement.py`.

1. **Lemma 1 was false as stated.** It claimed all invariants of operators in $\mathfrak{C}$ lie in $\mathbb{Q}(G^*)$, with the bullet *"The Watson self-energy operator scales the invariants by even powers of $G^*$, which lie in $\mathbb{Q}(G^*)$."* But generator 3 is $G_{\text{BCC}}(0)\cdot I$ with $G_{\text{BCC}}(0) = G^{*2}/(2\pi)$: its trace is $G^{*2}/\pi$, which lies in $\mathbb{Q}(G^*)$ only if $\pi \in \mathbb{Q}(G^*)$ — contradicting the corpus's own Theorem 9 (FTD-0112 / OT-2.3). **Repair:** the invariant field is enlarged to $\mathbb{Q}(G^*, \pi)$ throughout Lemma 1 and the theorem; the conclusion survives because $\delta = \sqrt{G^*(4G^*-1)}$ still lies outside the enlarged field (Lemma 2).
2. **The abstract's obstruction language misdescribed the logic.** The old abstract said the quadratic extension creates *"an insurmountable Galois obstruction that prevents any native construction from forcing the exponent of $G^*$ in the determinant to be exactly 1."* That is not what the extension does: the companion matrix $T_W$ realizes $P(x)$ **over the base field itself** — no Galois obstruction blocks assembling the polynomial. What the degree-2 extension obstructs is **root-distinguishing beables**: every invariant of a base-field operator is Galois-fixed and hence blind to the swap $x_+ \leftrightarrow x_-$. The actual proof that the exponent selection ($k = 1$ in $\operatorname{Det} = 16G^{*2+k}$) is not derivable from the calculus is the conditional theorem of FTD-0243 §5; this document contributes the invariant-field confinement that the FTD-0243 argument consumes.
3. **"Algebraically independent coordinates" was false as stated.** The old §4 said $16G^{*2}$ and $16G^{*3}$ *"represent algebraically independent coordinates in the space of 2x2 matrices."* As real numbers they are algebraically **dependent**: $(16G^{*3})^2 = (16G^{*2})^3/16$ (machine-verified to $10^{-36}$). What is meant — and now stated — is that $(\operatorname{Tr}, \operatorname{Det})$ are **functionally independent invariants on $M_2$**: the map $(\operatorname{Tr}, \operatorname{Det}) \colon M_2(F) \to F^2$ is surjective (companion matrices), so fixing the trace does not constrain the determinant.
4. **Lemma 2's irreducibility reason was wrong.** The old proof said $G^*(4G^*-1)$ is not a perfect square *"(as a rational function in $G^*$, it has odd degrees)."* The operative property is **squarefreeness** — $t(4t-1)$ is squarefree, exactly as FTD-0314's §3-C3 argument states — equivalently, odd valuation at the prime $(t)$; "odd degree" is neither the operative property nor correctly framed.

---

## Abstract

We resolve the central foundational obstruction **MC-T4.3 / K-BIND** (the universal negative over substrate-native operator constructions) as a formal **[THEOREM]** by axiomatizing the **substrate-native operator construction calculus** $\mathfrak{C}$.

We prove that for any operator $T$ constructed strictly within $\mathfrak{C}$, its eigenvalues cannot be forced to realize the master-quadratic roots $\{x_+, x_-\}$ without an external selection $W$ (which is consistent with $\mathfrak{C}$ but not derivable from it). This is established by showing that all invariants (traces and determinants) of operators in $\mathfrak{C}$ lie in the field $\mathbb{Q}(G^*, \pi)$, while the splitting field of the master quadratic is a quadratic extension $K = \mathbb{Q}(G^*, \pi)(\sqrt{G^*(4G^*-1)})$ of degree 2 over $\mathbb{Q}(G^*, \pi)$. Because every invariant of a $\mathfrak{C}$-operator is fixed by $\operatorname{Gal}(K/\mathbb{Q}(G^*, \pi))$, no characteristic-polynomial datum of a native operator can distinguish $x_+$ from $x_-$ (no root-distinguishing beable); and the selection of the determinant exponent ($\operatorname{Det} = \operatorname{Tr}\cdot G^*$, i.e. $k = 1$) is not derivable from $\mathfrak{C}$ — the non-forcing proof is FTD-0243 §5, which this invariant-field confinement feeds. *(Abstract corrected 2026-07-01/02, FTD-0351 — see §0 items 1–2 for the previous wording and why it was wrong.)*

---

## §1 — The Admissible Generator Set $\mathcal{S}$ and Operator Calculus $\mathfrak{C}$

Let $V_{\text{complex}} \cong \mathbb{Z}[i]^2$ be the rank-2 module representing the complexified readout structure on the cubic lattice. We define the **substrate-native operator construction calculus** $\mathfrak{C}$ as the associative algebra over $\mathbb{Q}$ generated by the set of admissible substrate-native operations $\mathcal{S}$:

$$\mathfrak{C} = \langle \mathcal{S} \rangle_{\mathbb{Q}}$$

where the generator set $\mathcal{S}$ is defined by:
1. **The identity operator** $I$.
2. **The complex structure operator** $J$, satisfying $J^2 = -I$ (representing multiplication by $i$, i.e. planar $90^\circ$ rotation).
3. **The Watson self-energy scaling operator** $G_{\text{BCC}}(0) \cdot I$, where $G_{\text{BCC}}(0) = \frac{G^{*2}}{2\pi}$ represents the self-energy of the BCC sublattice.
4. **The $J$-twisted zeta regularized determinants ratio** $D_{3/4}/D_{1/4}$, which evaluates to the Lemniscatic constant $G^*$ (the $2\pi$ factors cancel).
5. **Finite difference stencils** $\Delta_{\text{stencil}}$ and translations on the module $V_{\text{complex}}$ corresponding to the Moore neighborhood.

> **Standing flag (FTD-0347, unresolved and orthogonal to the FTD-0351 repair):** whether $\mathcal{S}$ is *representative* of everything the substrate can construct — i.e. whether the axiomatization is drawn broadly enough that the universal negative binds all native routes — remains an open specialist question. Nothing in the field-enlargement repair below touches it.

---

## §2 — Rationality of Invariants over $\mathbb{Q}(G^*, \pi)$

**Lemma 1 (corrected 2026-07-01/02, FTD-0351).** The trace and determinant of any operator $T \in \mathfrak{C}$ lie in the field $\mathbb{Q}(G^*, \pi)$.

*Proof.* The generators in $\mathcal{S}$ have matrix representations (under the canonical complex basis) whose entries lie in $\mathbb{Q}(G^*, \pi)$:
- $\operatorname{Tr}(I) = 2$, $\operatorname{Det}(I) = 1 \in \mathbb{Q}$.
- $\operatorname{Tr}(J) = 0$, $\operatorname{Det}(J) = 1 \in \mathbb{Q}$.
- The Watson self-energy operator $G_{\text{BCC}}(0)\cdot I = \frac{G^{*2}}{2\pi} I$ has entries in $\mathbb{Q}(G^*, \pi)$; its invariants are $\operatorname{Tr} = G^{*2}/\pi$ and $\operatorname{Det} = G^{*4}/(4\pi^2)$, which lie in $\mathbb{Q}(G^*, \pi)$ but **not** in $\mathbb{Q}(G^*)$ (membership would force $\pi \in \mathbb{Q}(G^*)$, contradicting Theorem 9 / FTD-0112). *(This bullet previously claimed the invariants lie in $\mathbb{Q}(G^*)$ via "even powers of $G^*$" — false; see §0 item 1.)*
- The determinant ratio operator yields $G^* \in \mathbb{Q}(G^*) \subset \mathbb{Q}(G^*, \pi)$.
- The finite-difference stencils and translations have rational coefficients, hence entries in $\mathbb{Q}$.

Since $\mathfrak{C}$ is generated by finite $\mathbb{Q}$-linear combinations and products of these generators, the matrix entries of any $T \in \mathfrak{C}$ remain in the subfield of $\mathbb{R}$ generated by the generators' entries, namely $\mathbb{Q}(G^*, \pi)$.

Consequently, the characteristic polynomial of any $T \in \mathfrak{C}$:
$$\chi_T(x) = x^2 - \operatorname{Tr}(T)x + \operatorname{Det}(T)$$
has coefficients $\operatorname{Tr}(T), \operatorname{Det}(T) \in \mathbb{Q}(G^*, \pi)$. $\square$

---

## §3 — The Galois Obstruction to Root-Distinguishing Beables

The master quadratic polynomial is:
$$P(x) = x^2 - 16 G^{*2} x + 16 G^{*3} = 0$$
with discriminant:
$$\Delta = (16 G^{*2})^2 - 4(16 G^{*3}) = 64 G^{*3}(4G^* - 1) > 0$$
and roots:
$$x_\pm = 8 G^{*2} \pm 4 G^* \sqrt{G^*(4G^* - 1)}$$
(machine-verified against the quadratic formula at 50 digits).

**Lemma 2 (corrected 2026-07-01/02, FTD-0351).** Write $\delta = \sqrt{G^*(4G^*-1)}$ and $F = \mathbb{Q}(G^*, \pi)$. Then the splitting field $K = F(\delta)$ of $P(x)$ over $F$ satisfies $[K : F] = 2$; in particular $\delta \notin \mathbb{Q}(G^*, \pi)$, and a fortiori $\delta \notin \mathbb{Q}(G^*)$.

*Proof.*
**(1) $\{G^*, \pi\}$ are algebraically independent over $\mathbb{Q}$.** By Chudnovsky's 1976 theorem, $\{\pi, \Gamma(1/4)\}$ are algebraically independent over $\mathbb{Q}$ (equivalently over $\overline{\mathbb{Q}}$). The reflection formula gives $\Gamma(1/4)\Gamma(3/4) = \pi/\sin(\pi/4) = \pi\sqrt{2}$, hence
$$G^* = \frac{\Gamma(1/4)}{\Gamma(3/4)} = \frac{\Gamma(1/4)^2}{\pi\sqrt{2}}.$$
A nontrivial polynomial relation $Q(G^*, \pi) = 0$ with $Q \in \mathbb{Q}[s, u] \setminus \{0\}$ would, after substituting $s = \Gamma(1/4)^2/(\pi\sqrt2)$ and clearing denominators, yield a nontrivial polynomial relation between $\Gamma(1/4)$ and $\pi$ with coefficients in $\mathbb{Q}(\sqrt2) \subset \overline{\mathbb{Q}}$, contradicting Chudnovsky. Hence the evaluation map $\mathbb{Q}[t, u] \to \mathbb{R}$, $t \mapsto G^*$, $u \mapsto \pi$ is injective, and
$$F = \mathbb{Q}(G^*, \pi) \;\cong\; \mathbb{Q}(t, u),$$
a rational function field in **two** independent variables. (This step uses Theorem 9's trivial-intersection result in its sharp form: $\pi$ is transcendental *over* $\mathbb{Q}(G^*)$, not merely $\pi \notin \mathbb{Q}(G^*)$.)

**(2) $\delta^2$ is a non-square in $F$.** Under the isomorphism of (1), $\delta^2 = G^*(4G^*-1)$ corresponds to $c(t) = t(4t-1) \in \mathbb{Q}[t] \subset \mathbb{Q}(u)[t]$. The element $c$ is **squarefree**: its irreducible factors $t$ and $4t-1$ are distinct and simple (machine-verified: `factor_list` $= [(t, 1), (4t-1, 1)]$, $\gcd(c, c') = 1$). In the rational function field $\mathbb{Q}(u)(t)$, any square has even valuation at every prime of $\mathbb{Q}(u)[t]$; but $v_{(t)}(c) = 1$ is odd. Hence $c$ is not a square in $F \cong \mathbb{Q}(u)(t)$, so $y^2 - c$ is irreducible over $F$ (machine-verified: `factor_list` over $\mathbb{Q}[y, t, u]$ returns the single factor $y^2 - 4t^2 + t$ with multiplicity 1) and
$$[K : F] = [F(\delta) : F] = 2.$$
*(The old proof's reason — "as a rational function in $G^*$, it has odd degrees" — is replaced by the squarefree/odd-valuation argument, matching FTD-0314 §3-C3; see §0 item 4.)*

**(3) Galois action.** $\operatorname{Gal}(K/F) \cong \mathbb{Z}/2$, generated by $\sigma \colon \delta \mapsto -\delta$; since $x_\pm = 8G^{*2} \pm 4G^*\delta$,
$$\sigma(x_+) = x_- \quad \text{and} \quad \sigma(x_-) = x_+. \qquad \square$$

---

## §4 — Resolution of K-BIND

**Theorem 1 (K-BIND Resolution).** No operator $T \in \mathfrak{C}$ uniquely forces the co-realization of the master-quadratic invariants $(\operatorname{Tr}, \operatorname{Det}) = (16G^{*2}, 16G^{*3})$.

*Proof.* For any operator $T$ to realize the master-quadratic roots $\{x_+, x_-\}$ as eigenvalues, its characteristic polynomial must be exactly $P(x)$, which requires:
$$\operatorname{Tr}(T) = 16 G^{*2} \quad \text{and} \quad \operatorname{Det}(T) = 16 G^{*3}$$

Both $16 G^{*2}$ and $16 G^{*3}$ lie in $\mathbb{Q}(G^*) \subset \mathbb{Q}(G^*, \pi)$, and $(\operatorname{Tr}, \operatorname{Det})$ are **functionally independent invariants on $M_2$**: the map $(\operatorname{Tr}, \operatorname{Det}) \colon M_2(F) \to F^2$ is surjective (every pair is realized by a companion matrix), so fixing the trace does not constrain the determinant. *(As real numbers the two targets are algebraically dependent — $(16G^{*3})^2 = (16G^{*2})^3/16$, machine-verified — so the previous "algebraically independent coordinates" wording was false as stated; see §0 item 3.)*

To bind the odd scalar $G^*$ into the determinant slot at exponent 1 relative to the trace (i.e., $\operatorname{Det}(T) = \operatorname{Tr}(T) \cdot G^*$) requires a composition rule $W$. In the calculus $\mathfrak{C}$, any alternative selection $T_k$ with $\operatorname{Tr}(T_k) = 16 G^{*2}$ and $\operatorname{Det}(T_k) = 16 G^{*2+k}$ (for $k \in \mathbb{Z}$) is equally admissible and has invariants in $\mathbb{Q}(G^*) \subset \mathbb{Q}(G^*, \pi)$. That no rule of $\mathfrak{C}$ derives the specific choice $k = 1$ is the content of the conditional theorem FTD-0243 §5 (`AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md`), which consumes Lemma 1's invariant-field confinement; the present document does not re-prove it.

Because $\operatorname{Tr}(T), \operatorname{Det}(T) \in F$ are fixed by every $\sigma \in \operatorname{Gal}(K/F)$ (Lemma 2), the invariants of any native operator are completely blind to the root swap $x_+ \leftrightarrow x_-$: no beable constructed from characteristic-polynomial data of $\mathfrak{C}$-operators distinguishes the roots. The specific choice of $k = 1$ that yields $x_+ \approx 137.036$ is mathematically independent of the generating set $\mathcal{S}$ and cannot be derived from it.

Therefore, the master-quadratic companion matrix:
$$T_W = \begin{pmatrix} 0 & -16G^{*3} \\ 1 & 16G^{*2} \end{pmatrix}$$
is a consistent selection ($T_W \in M_2(\mathbb{Q}(G^*))$ — note the companion matrix realizes $P(x)$ over the base field; the Galois extension obstructs root-*distinguishing*, not polynomial-*assembly*), but its assembly is logically independent of the calculus $\mathfrak{C}$. This closes the K-BIND universal negative. $\square$

---

## §5 — Epistemic Status and Impact

With the axiomatization of the calculus $\mathfrak{C}$, the K-BIND open item transitions:

$$\boxed{\text{K-BIND } [\text{OPEN}] \;\longrightarrow\; [\text{CLOSED THEOREM-NEGATIVE}]}$$

This completes the proof of Readout-Structure Independence (RSI) Leg 3c. It establishes that the electromagnetic coupling constant $\alpha$ is fundamentally a **dynamical** parameter (selected by the external assembly $W$) rather than a **structural** parameter of the substrate's discrete ontology.

**Scope notes (2026-07-01/02, FTD-0351):**
- The [THEOREM] tag is relative to the stated generator set $\mathcal{S}$; the generator-representativeness question (FTD-0347) remains FLAGGED and is not resolved — or touched — by the field-enlargement repair.
- Checkable algebra for the repair (squarefreeness of $t(4t-1)$, irreducibility of $y^2 - t(4t-1)$ over $\mathbb{Q}(t,u)$, the $(16G^{*3})^2 = (16G^{*2})^3/16$ dependence, and the 50-digit root identity) is verified by `scripts/proofs/proof_k_bind_field_enlargement.py`.
