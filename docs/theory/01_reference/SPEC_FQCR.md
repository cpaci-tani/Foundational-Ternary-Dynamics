# SPEC · Finite Quarter-Conjugacy Recurrence (FQCR)

**Tag:** [REFERENCE]
**Date:** 2026-05-06
**Version:** 1.0
**Status:** [REFERENCE] — capstone document for the FQCR framework. Per-element tags within (see §4 status table).
**Companion docs:** [`DERIV_GSTAR_QUARTER_CONJUGACY.md`](../03_derivations/DERIV_GSTAR_QUARTER_CONJUGACY.md) (FTD-0141, Model I), [`DERIV_GSTAR_FINITE_APPROX.md`](../03_derivations/DERIV_GSTAR_FINITE_APPROX.md) (FTD-0142, Model II), [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) §10, [`DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`](../03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md), [`DERIV_G_STAR_PARITY_TWIST.md`](../03_derivations/DERIV_G_STAR_PARITY_TWIST.md) (FTD-0127), [`PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md`](../10_eft_program/PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md) (FTD-0143), [`REF_QCR_TRILOGY_BRIDGE.md`](../09_mathematical/REF_QCR_TRILOGY_BRIDGE.md) (FTD-0144, curve-side trilogy bridge).
**Verifier script:** [`scripts/proofs/proof_fqcr_convergence.py`](../../../scripts/proofs/proof_fqcr_convergence.py)

The Finite Quarter-Conjugacy Recurrence (FQCR) is a five-model framework that:

1. Provides an **operator-theoretic provenance** for the bridge constant $G^*$ via $\zeta$-regularized determinants of quarter-twisted spectra (Model I, [THEOREM]).
2. Restates that provenance in **finite-N reframe-compatible** form (Model II, [THEOREM]).
3. Reduces to a **second-order linear recurrence** with a conserved Casimir invariant (Model III, [THEOREM]).
4. Parameterises a **modular-anomaly form** $R_N(t)$ via a $(4, 6; 3, 2)$ exponent quadruple (Model IV, [SELECTION]).
5. Reframes the FTD master quadratic as a **transfer-matrix characteristic polynomial** $M_N(t)$, with the dominant eigenvalue identified conjecturally with $\alpha^{-1}$ at $t = 1$ (Model V, [SMC] for the physical claim, [THEOREM] for the structural identity).

**Core message.** FQCR is not a replacement for FTD's existing 9-theorem algebraic spine. It is a **complementary set of operator-theoretic lenses** that anchor several spine claims to a unified primitive — the conjugacy operator $J$ with $J^2 = -I$. Model I + II are subsidiaries to Theorem 1 ($G^*$ identity); Model V is a subsidiary to Theorem 2 (master quadratic). Models III + IV are new structural content.

---

## §1 — Definitions

### Definition 1: The primitive conjugacy operator

A finite-state operator $J$ on a 2-real-dimensional space (equivalently, multiplication by $i$ on $\mathbb{C}$) satisfying

$$ J^2 = -I, \qquad \therefore J^4 = I. $$

$J$ generates a cyclic group of order 4 with elements $\{I, J, -I, -J\}$. Eigenvalues on $\mathbb{C}$: $\pm i = e^{\pm 2\pi i/4}$.

This is the **algebraic source of the quarter split** that pervades the FTD spine: the $i$-cycle ontology, the framework integer $N_\text{base} = 4$, the $(1+i)$-tower of Theorem 8 — all share this $Z_4$ structure.

### Definition 2: Quarter-twisted spectra

For a wavefunction $\psi : S^1 \to \mathbb{C}^2$ subject to $\psi(\phi + 2\pi) = J\,\psi(\phi)$, decomposing along the $J$-eigenbasis forces the allowed Fourier modes to lie in

$$ D_{1/4} := \{n + \tfrac{1}{4}\}_{n\ge 0}, \qquad D_{3/4} := \{n + \tfrac{3}{4}\}_{n\ge 0}. $$

These are the spectra of two disjoint differential-like operators on a domain compatible with the quarter-twisted boundary condition.

### Definition 3: Recurrence coefficient and update law

Let $R_N(t)$ be a finite projection-renormalisation factor (specified in §3.4). Define

$$ \kappa_N(t) := \frac{R_N(t)}{16\,G_N^*}, \qquad s_N(t) := \frac{1}{\sqrt{\kappa_N(t)}} = \sqrt{\frac{16\,G_N^*}{R_N(t)}}. $$

The primitive finite update law on a sequence $r_m$ is

$$ r_{m+1} - r_m + \kappa_N(t)\, r_{m-1} = 0. $$

Under the reparameterisation $r_m = \kappa_N(t)^{m/2}\, u_m$, this becomes the **symmetric form**

$$ u_{m+1} + u_{m-1} = s_N(t)\, u_m. $$

---

## §2 — Theorem-level propositions

### Proposition 1: $G^*$ as a determinant bridge (Model I)

> $$ \frac{\det_\zeta D_{3/4}}{\det_\zeta D_{1/4}} = \frac{\Gamma(1/4)}{\Gamma(3/4)} = G^*. $$

**[THEOREM]** by Lerch's formula $\det_\zeta\{n + a\}_{n\ge 0} = \sqrt{2\pi}/\Gamma(a)$.

Full derivation: [`DERIV_GSTAR_QUARTER_CONJUGACY.md`](../03_derivations/DERIV_GSTAR_QUARTER_CONJUGACY.md) (FTD-0141).

### Proposition 2: Finite-N convergence (Model II)

> $$ G_N^* := (N+1)^{-1/2} \prod_{n=0}^{N} \frac{n + 3/4}{n + 1/4} \;\to\; G^* \quad \text{as } N \to \infty, \qquad |G_N^* - G^*| = O(1/N^2). $$

**[THEOREM]** by Stirling's expansion, empirically verified at $C \approx 0.046$ via [`proof_fqcr_convergence.py`](../../../scripts/proofs/proof_fqcr_convergence.py). Full derivation: [`DERIV_GSTAR_FINITE_APPROX.md`](../03_derivations/DERIV_GSTAR_FINITE_APPROX.md) (FTD-0142).

This **discharges the `AUDIT_INFINITY_REFRAME.md` ε-L obligation for $G^*$**: the bridge constant is now a finite-N attractor, fully compatible with FTD's undefined-boundary lattice ontology.

### Proposition 3: Conserved relational invariant (Model III)

For the symmetric recurrence $u_{m+1} + u_{m-1} = s\,u_m$, the quantity

$$ \mathcal{I}_m := u_m^2 + u_{m-1}^2 - s\,u_m\,u_{m-1} $$

is conserved under the update $m \mapsto m+1$, i.e. $\mathcal{I}_{m+1} = \mathcal{I}_m$.

**[THEOREM]** — direct algebraic verification:

$$ \mathcal{I}_{m+1} - \mathcal{I}_m = u_{m+1}^2 - u_{m-1}^2 - s(u_{m+1} - u_{m-1})\,u_m = (u_{m+1} - u_{m-1})(u_{m+1} + u_{m-1} - s\,u_m) = 0 $$

by the recurrence. The model has a conserved relational invariant **without requiring Hilbert-space unitarity** — the conservation law is purely algebraic.

### Proposition 4: Möbius reduction (Model III)

For the ratio $z_m := u_m / u_{m-1}$, the recurrence becomes

$$ z_{m+1} = s - \frac{1}{z_m}. $$

The fixed points satisfy $z = s - 1/z \;\Leftrightarrow\; z^2 - sz + 1 = 0$, with roots

$$ z_\pm = \frac{s \pm \sqrt{s^2 - 4}}{2}, \qquad z_+ z_- = 1. $$

**[THEOREM]** — the two branches are reciprocal conjugates by Vieta. This is the projective form of the dynamics; Models IV–V build on it by specifying $s = s_N(t)$.

### Proposition 5: Master quadratic as transfer-matrix char poly (Model V structural)

Define the finite transfer operator

$$ M_N(t) := 16\,(G_N^*)^2 \begin{pmatrix} 1 & -\dfrac{R_N(t)}{16\,G_N^*} \\ 1 & 0 \end{pmatrix}. $$

Its characteristic polynomial is

$$ x^2 - 16\,(G_N^*)^2\,x + 16\,(G_N^*)^3\,R_N(t) = 0. $$

**[THEOREM]** as a notational identity: at $R_N(t) = 1$ and $N \to \infty$ this is **exactly** the FTD master quadratic (Theorem 2 of the algebraic spine, FTD-0014). The transfer-matrix structure provides an operator interpretation of a polynomial that was previously stated only algebraically.

This **does not change the [STRONGLY MOTIVATED CONJECTURE] tag** of the physical reading $\alpha^{-1} = \lambda_\text{max}(M_N(1, N\to\infty)) = x_+$ — that remains FTD-0013.

---

## §3 — Selection-level content

### §3.1 — The (4, 6; 3, 2) exponent quadruple

Define

$$ \Psi_N(t) := \prod_{n=1}^{N} \frac{(1 - Q^{4n})^6}{(1 - Q^{3n})^2}, \qquad Q := e^{-2\pi t}. $$

The exponent quadruple $(k, d; \ell, m) = (4, 6; 3, 2)$ is **not derived from FTD axioms** — it is a structural choice. The interpretation: $(4, 6)$ is a "primitive antisymmetric relational sector" and $(3, 2)$ is a "projected transverse observable sector". This is **[SELECTION]** grade content.

The pre-registered uniqueness scan in `PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md` (FTD-0143) tests whether $(4, 6; 3, 2)$ is privileged among the $7^4 = 2401$ alternatives in $\{2, ..., 8\}^4$. Until that scan executes:

- **Tag stays at [SELECTION].**
- The doc must NOT claim $(4,6;3,2)$ is uniquely selected.
- Anti-target: avoid the FTD-0097 look-elsewhere overclaim pattern.

### §3.2 — The base point $t = 1$

The physical readout in §4 evaluates $\alpha^{-1} \approx \lambda_\text{max}(M_N(t=1))$. The pinning $t = 1$ is a **[SELECTION]** without a-priori derivation. If $t$ is interpreted as inverse-scale-like (RG-flow analog), $t = 1$ should correspond to a structural calibration; that interpretation is open.

Test 3 of the FQCR research program — running behaviour as $t$ varies — is the natural follow-up. It is currently out of roadmap scope until either an a-priori interpretation of $t$ surfaces or Test 4 (generalisation to other coupling constants) provides one indirectly.

### §3.3 — The combined renormalisation factor $R_N(t)$

$$ R_N(t) = 1 + \lambda_N(4it) + A_N(t), $$

with three additive terms:

- $1$ — the canonical base point at which the master quadratic recovers FTD-0014 exactly.
- $\lambda_N(4it)$ — a finite-theta approximation to the modular lambda function:
  $$ \lambda_N(4it) := \left(\frac{\theta_{2,N}(4it)}{\theta_{3,N}(4it)}\right)^4, $$
  where $\theta_{2,N}, \theta_{3,N}$ are truncated Jacobi theta functions to $N$ terms.
- $A_N(t)$ — the per-projected-dimension Eisenstein-anomaly:
  $$ A_N(t) = \frac{1}{3}\frac{d}{dt}\log\Psi_N(t) = 16\pi\sum_{n=1}^{N}\frac{n\,Q^{4n}}{1 - Q^{4n}} - 4\pi\sum_{n=1}^{N}\frac{n\,Q^{3n}}{1 - Q^{3n}}. $$

The choice of additive combination $R = 1 + \lambda + A$ specifically (vs other linear combinations of these primitives) is also a **[SELECTION]**.

**Cross-reference (2026-05-07).** The compatibility paper of the QCR trilogy (see [`REF_QCR_TRILOGY_BRIDGE.md`](../09_mathematical/REF_QCR_TRILOGY_BRIDGE.md)) defines the same coefficient $\kappa_N(t) = R_N(t)/(16 G_N^*)$ in its Definition 2.2; the symbol $R_N(t)$ in this document and in the compatibility paper is the same object. The trilogy itself does not commit to a specific decomposition $R = 1 + \lambda + A$; that choice remains an FQCR-internal [SELECTION] pending FTD-0143.

---

## §4 — Status table (per-element tags)

| Sub-model | Statement | Tag | Notes |
|---|---|---|---|
| I — quarter-conjugacy | $J^2 = -I \Rightarrow Z_4$ | [THEOREM] | Standard linear algebra |
| I — Lerch's formula recap | $\det_\zeta\{n+a\} = \sqrt{2\pi}/\Gamma(a)$ | [THEOREM] | External; standard analytic number theory |
| I — determinant bridge | $\det_\zeta D_{3/4}/\det_\zeta D_{1/4} = G^*$ | **[THEOREM]** (new content) | Proposition 1, FTD-0141 |
| II — finite product | $G_N^*$ defined for all finite $N$ | [DEFINITION] | Doesn't need a tag |
| II — Stirling convergence | $G_N^* \to G^*$ at $O(1/N^2)$ | [THEOREM] | Proposition 2, FTD-0142 |
| II — reframe discharge | $G^*$ is a finite-N attractor | [THEOREM] | Discharges `AUDIT_INFINITY_REFRAME` obligation |
| III — symmetric recurrence | $u_{m+1} + u_{m-1} = s u_m$ | [THEOREM] | Standard reparameterisation |
| III — Casimir invariant | $\mathcal{I}_m = u_m^2 + u_{m-1}^2 - su_m u_{m-1}$ | [THEOREM] | Proposition 3 |
| III — Möbius reduction | $z_{m+1} = s - 1/z_m$, $z_+z_- = 1$ | [THEOREM] | Proposition 4 |
| IV — exponent quadruple $(4,6;3,2)$ | privileged choice in $\{2,...,8\}^4$ | **[SELECTION]** | Pending FTD-0143 uniqueness scan |
| IV — base point $t = 1$ | physical calibration anchor | **[SELECTION]** | Pending interpretation of $t$ |
| IV — additive form $R = 1 + \lambda + A$ | combination choice | **[SELECTION]** | One of several plausible combinations |
| V — transfer matrix $M_N(t)$ | Char poly = master quadratic at $R=1$ | [THEOREM] (notational) | Proposition 5 — provides operator interpretation; doesn't add new physical content |
| V — physical readout | $\alpha^{-1} = \lambda_\text{max}(M_N(1, N\to\infty)) = x_+$ | [STRONGLY MOTIVATED CONJECTURE] | Inherits FTD-0013 tag; no upgrade |

---

## §5 — Cross-references

### To existing FTD spine

| Spine theorem | Relation to FQCR |
|---|---|
| Theorem 1 (FTD-0001): $G^* = \Gamma(1/4)/\Gamma(3/4)$ | FQCR Model I provides operator-theoretic provenance via $\det_\zeta$ ratio. FQCR Model II provides finite-N reframe-compatible restatement. **Subsidiary** to Theorem 1. |
| Theorem 2 (FTD-0014): master quadratic | FQCR Model V provides transfer-matrix interpretation. **Subsidiary** to Theorem 2. |
| Theorem 8 (FTD-0111): $(1+i)$-tower | Shares the $Z_4$ structural anchor with the conjugacy operator $J$. Unified $Z_4$ subsection in §10 of `SPEC_ALGEBRAIC_SPINE.md` is proposed as future stylistic refactor. |
| Theorem 9 (FTD-0112): $Q(G^*)$ field-theoretic | Unaffected; FQCR does not introduce new transcendentals. |
| FTD-0127: parity-twist (L-function lens) | **Two readings of the same residue-class decomposition mod 4**, sharper than the original "complementary lenses" framing. The shifts $\{1/4, 3/4\}$ in FQCR are not free parameters — once $J^2 = -I$ is committed, the quarter-twisted boundary forces the spectral shifts to be the two non-trivial residue classes mod 4. After scaling by 4, $4 D_{1/4} = \{n \equiv 1\pmod 4\}$ and $4 D_{3/4} = \{n \equiv 3\pmod 4\}$; restricted to primes, these are the split and inert prime classes of $\mathbb{Z}[i]$ (Fermat's two-square theorem). FTD-0127 takes the parity-symmetric (sum/difference) combinations of the same Hurwitz components $\zeta_H(s, 1/4)$ and $\zeta_H(s, 3/4)$ that FQCR Model I works on directly. Unification one-line: $G^* = \exp[\zeta_H'(0, 1/4) - \zeta_H'(0, 3/4)] = \Gamma_\zeta(1/2)/\Gamma_{\chi_{-4}}(1/2) = \Gamma(1/4)/\Gamma(3/4)$. See `DERIV_GSTAR_QUARTER_CONJUGACY.md` §5 (revised 2026-05-06) for the full residue-class / Z[i]-prime-splitting derivation. |
| FTD-0001/0013/0014 SMC chain | FQCR's Model V physical readout is exactly FTD-0013 restated in operator language; tag stays at SMC. No promotion. |
| FTD-0144 (QCR trilogy bridge) | The compatibility paper supplies the curve-side geometric pairing for FQCR's branch-side recurrence, plus a concrete depth-4 five-harmonic numerical incarnation of FTD's $(1+i)$-tower (Theorem 8 / FTD-0111). Cross-confirms FQCR Models II–V at the formula level. Documented in [`REF_QCR_TRILOGY_BRIDGE.md`](../09_mathematical/REF_QCR_TRILOGY_BRIDGE.md). No spine inflation; no tier promotion. |

### To FQCR's own derivation chain

```
SPEC_FQCR.md  (this document, v1.0)
   ├── §2 Prop 1 ────────→ DERIV_GSTAR_QUARTER_CONJUGACY.md (FTD-0141)
   ├── §2 Prop 2 ────────→ DERIV_GSTAR_FINITE_APPROX.md (FTD-0142)
   │                       └── verified by proof_fqcr_convergence.py
   ├── §2 Props 3–4 ─────→ this document (Models III; standard)
   ├── §2 Prop 5 ────────→ extension §VII of DERIV_MASTER_QUADRATIC_GAP_EQUATION.md (planned)
   ├── §3 Model IV ──────→ PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md (FTD-0143)
   └── §3 Model V phys ──→ inherits FTD-0013 SMC tag from spine
```

---

## §6 — Test program

The FQCR research program identifies four falsifiable tests. Their status as of 2026-05-06:

### Test 1 — finite-N convergence
**Status: PASS.** Verified by [`scripts/proofs/proof_fqcr_convergence.py`](../../../scripts/proofs/proof_fqcr_convergence.py).

- $|G_N^* - G^*| < 10^{-7}$ at $N = 1024$ ✓
- $|G_N^* - G^*| < 10^{-8}$ at $N = 4096$ ✓
- $1/N^2$ scaling holds to 10% across $N \ge 1024$ with empirical $C \approx 0.046$ ✓

### Test 2 — quotient uniqueness
**Status: PRE-REGISTERED.** See [`PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md`](../10_eft_program/PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md) (FTD-0143). Scan execution awaiting separate session.

Pre-registered outcomes:
- **Outcome A (uniqueness confirmed):** $(4,6;3,2)$ scores in top-3 across $\ge 3$ tolerances AND no other quadruple matches $> 1$ target at $\le 10^{-4}$. Model IV upgrades to [SELECTION with uniqueness backing].
- **Outcome B (uniqueness rejected):** $(4,6;3,2)$ is one of many near-misses. Model IV stays [SELECTION], FQCR's $\alpha^{-1}$ readout becomes a chance-level fit.

Either outcome is publishable as honest pre-reg result.

### Test 3 — running behaviour
**Status: OPEN — out of scope until $t$ has a-priori interpretation.** Compute $\alpha_N^{-1}(t)$ for $t \in [0.5, 2]$. If $t$ is inverse-scale-like, the model should produce coherent monotonic running matching physical $\beta$-function expectations. Without an a-priori interpretation of $t$, this test risks confirming a coincidence.

### Test 4 — generalisation
**Status: OPEN — separate research arc.** Search whether the same recurrence architecture (different $J'$ with $J'^k = I$ for $k \ne 4$, different exponent quadruple) yields other dimensionless invariants ($\sin^2 \theta_W$, $m_\mu/m_e$, ...). **This is the test that decides whether FQCR is a framework or a single-α reconstruction.**

If only $\alpha^{-1}$ comes out: FQCR is a sophisticated re-derivation of the master quadratic with one new theorem (Proposition 1). Defensible as published, but limited in reach.

If a family of coherent invariants comes out: FQCR is a framework. This would be **a substantial elevation** of multiple SMC claims toward [DERIVED] status.

Test 4 is decade-scale work for a small team and is queued as the natural follow-up after R3 / R4 of the FTD-EFT roadmap close.

---

## §7 — Out of scope

- **Test 4 generalisation execution.** Separate research arc.
- **Engine code changes.** None required. FQCR is pure theory + proof scripts + pre-reg.
- **Manuscript-grade write-up.** Queued for R6 of the FTD-EFT roadmap (`PAPER_FTD_NATIVE_EFT.tex`); SPEC_FQCR.md is the canonical reference, not the paper draft.
- **Linking $J$-eigenvalues to χ_{-4}(n) = $i^{n-1}$.** A unification of FQCR Model I with FTD-0127 parity-twist via this character identity is a future refactor of `SPEC_ALGEBRAIC_SPINE.md` §10.
- **Physical interpretation of $t$.** Open question; gates Test 3.

---

## §8 — Refresh policy

If any of the following land:

- A scan result for FTD-0143 (Test 2 quotient uniqueness): update §3.1 + §4 + §6.
- An a-priori interpretation of $t$: update §3.2 + §6 (Test 3).
- Generalisation result (Test 4): major rewrite — possibly new SPEC_FQCR_v2.md.
- A unification of Models I and FTD-0127: update §5 + §3 of the parity-twist doc.

Until then, this v1.0 stands as the canonical FQCR reference.
