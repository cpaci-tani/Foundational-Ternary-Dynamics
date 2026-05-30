# REF · QCR Trilogy — Branch / Curve / Compatibility Bridge

**Tag:** [REFERENCE]
**Date:** 2026-05-07
**Version:** 1.0
**LEDGER row reservation:** FTD-0144 [STRUCTURAL CORRESPONDENCE]
**Companion docs:** [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md), [`DERIV_GSTAR_QUARTER_CONJUGACY.md`](../03_derivations/DERIV_GSTAR_QUARTER_CONJUGACY.md) (FTD-0141), [`DERIV_GSTAR_FINITE_APPROX.md`](../03_derivations/DERIV_GSTAR_FINITE_APPROX.md) (FTD-0142), [`DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`](../03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md) Part VII, [`SPEC_ALGEBRAIC_SPINE.md`](../01_reference/SPEC_ALGEBRAIC_SPINE.md) §10.X (FTD-0111 Theorem 8).
**Verifier script:** [`scripts/proofs/proof_fqcr_branch_value.py`](../../../scripts/proofs/proof_fqcr_branch_value.py)

---

## §1 — The trilogy

Three short authorless mathematical notes were shared in 2026-05-06 / 2026-05-07. Together they constitute a self-contained framework on quarter-conjugacy recurrences and dyadic Fourier curves. **They are not part of FTD.** This document records (a) what each paper formalises, (b) where each cross-confirms an existing FTD construct, (c) what genuinely new structural content the compatibility paper introduces, and (d) how the cross-paper correspondence is integrated into FTD's documentation without inflating any tag.

| Paper | Contribution | Status in FTD |
|---|---|---|
| **Fourcier paper** | Dyadic Fourier curves $\gamma_N(t) = (\sum a_n \cos 2^n t,\,\sum b_n \sin 2^n t)$; their geometric spectrum (winding, signed area, radial minima, self-intersections) | Geometric reference. No FTD claim depends on this paper. |
| **QC-Recurrences paper** | Quarter-conjugacy recurrence $r_{m+1} - r_m + \kappa_N(t) r_{m-1} = 0$ with $\kappa_N(t) = R_N(t)/(16 G_N^*)$; symmetric form, projective ratio dynamics, reciprocal fixed branches $z_+ z_- = 1$; finite-$N$ approximant $G_N^*$ | Cross-confirms FQCR Models II–V (`SPEC_FQCR.md` §2 Propositions 2–5). $G_N^*$ is byte-identical to the FTD-0142 finite-N formula; reciprocal branches $z_+ z_- = 1$ are the projective form of the FTD master quadratic's transfer-matrix structure. |
| **Compatibility paper** | A categorical/structural pairing rule between branch spectra (algebraic) and curve spectra (geometric); weak/sector/strong compatibility levels; certification protocols; depth-4 five-harmonic worked example | New for FTD: curve-side geometric data, sector decomposition + branch coloring, compatibility persistence (open condition), and a concrete depth-4 numerical incarnation of FTD's $(1+i)$-tower. Honestly conjecture-flagged where appropriate. |

---

## §2 — Cross-paper structural correspondence

The trilogy organizes a single mathematical object — the depth-$N$ branch+curve spectrum — under three orthogonal lenses:

```
Branch spectrum  Q_N(t) = (G_N*, R_N(t), kappa_N(t), s_N(t), {z_+(t), z_-(t)}, I)
                    |                                              |
                    |                                              |
                  ALGEBRAIC                                    PROJECTIVE
                  side (QC paper)                              fixed branches
                                                               z_+ * z_- = 1
                                                                     |
                                                                     |
            CATEGORICAL (compatibility paper)                  TRANSFER MATRIX
            sector coloring c: S_N -> {z_+, z_-}                (FQCR Model V)
            balance, doublet-compat, persistence
                                                                     |
                                                                     |
                  GEOMETRIC                                    Master quadratic
                  side (Fourcier paper)                        x^2 - 16G*^2 x + 16G*^3 = 0
                  curve spectrum F_N(theta) =                  (FTD Theorem 2)
                    (N, wind_0, Area, I, M_N(theta), R_N, B_N)
```

In FTD terms, the trilogy supplies a concrete **numerical incarnation** of the $(1+i)$-tower (`SPEC_ALGEBRAIC_SPINE.md` Theorem 8 / FTD-0111) at depth 4, plus a categorical scaffolding that pairs the algebraic branches with geometric sectors of a specific dyadic Fourier curve. The trilogy itself does not interpret the sectors physically; FTD documents the correspondence without promoting any interpretation.

---

## §3 — What is theorem-grade across the trilogy

The following items are theorem-grade in the trilogy and either (a) confirm existing FTD constructs or (b) establish standalone facts that FTD documents as references.

### 3.1 — Confirmations of existing FTD constructs

| Trilogy item | FTD construct it confirms | LEDGER |
|---|---|---|
| Compatibility paper Definition 2.1: $G_N^* = (N+1)^{-1/2} \prod_{n=0}^{N} (n+\tfrac{3}{4})/(n+\tfrac{1}{4})$ | **Identical** to `DERIV_GSTAR_FINITE_APPROX.md` §2 Definition 1 | FTD-0142 |
| Compatibility paper Proposition 2.4: relational invariant $\mathcal{I}_m = u_m^2 + u_{m-1}^2 - s\,u_m\,u_{m-1}$ conserved by $u_{m+1} + u_{m-1} = s\,u_m$ | **Identical** to `SPEC_FQCR.md` §2 Proposition 3 (FQCR Model III Casimir) | (FQCR Model III, [THEOREM]) |
| Compatibility paper Proposition 2.6: projective fixed branches $z_+(t)\,z_-(t) = 1$ from $z^2 - s_N(t)\,z + 1 = 0$ | **Same algebraic form** as `SPEC_FQCR.md` §2 Proposition 4 (Möbius reduction) and as the projective version of the master quadratic transfer-matrix structure (`DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` Part VII) | (FQCR Model V, [THEOREM] notational) |
| Compatibility paper Definition 2.7 branch spectrum $\mathfrak{Q}_N(t)$ | Same data tuple FQCR Models II–V manipulate | (no FTD-side claim; reference) |

These four confirmations are *not* new FTD theorems. They cross-validate the FQCR formalism by an independent re-derivation in slightly different notation; the agreement is exact at the formula level.

### 3.2 — Standalone trilogy theorems

| Trilogy item | Statement | FTD use |
|---|---|---|
| Compatibility paper Proposition 3.5 | $\mathrm{Area}(\gamma_N) = \pi \sum_{n=0}^{N} 2^n\,a_n\,b_n$ | Used in §4 below; closed-form curve invariant. |
| Compatibility paper Proposition 5.2 | Weak compatibility is an *open* condition: persists under sufficiently small perturbations of recurrence parameters and curve coefficients, given nondegeneracy + threshold-margin assumptions | Methodological template for FTD-0143's perturbation-stability analysis. |
| Compatibility paper Proposition 7.4 | For the depth-4 five-harmonic model, $\mathrm{Area}(\gamma_4) = 177\pi/400$ | Verified by `proof_fqcr_branch_value.py` at exact-rational + 50-digit-numerical levels. |

These are theorem-grade in the trilogy. FTD documents them but does not adopt them as new spine theorems; the spine count is unchanged — nine numbered results (six theorem-grade + three honestly-tiered; see `SPEC_ALGEBRAIC_SPINE.md` §0).

---

## §4 — The depth-4 five-harmonic model as a concrete $(1+i)$-tower instance

The compatibility paper's §7 defines an explicit five-harmonic dyadic curve

$$ x_4(t) \;=\; \cos t + \tfrac{1}{2}\cos 2t + \tfrac{1}{2}\cos 4t + \tfrac{2}{5}\cos 8t + \tfrac{1}{16}\cos 16t, $$

$$ y_4(t) \;=\; \sin t - \tfrac{1}{2}\sin 2t + \tfrac{1}{2}\sin 4t - \tfrac{7}{20}\sin 8t + \tfrac{1}{16}\sin 16t, $$

with frequencies $1, 2, 4, 8, 16$ — i.e. the dyadic ladder $2^k$ for $k = 0, 1, 2, 3, 4$.

### 4.1 — Connection to FTD Theorem 8 (the $(1+i)$-tower)

Theorem 8 of the algebraic spine (`SPEC_ALGEBRAIC_SPINE.md`, FTD-0111) defines the harmonic-invariant tower

$$ M_k(x) \;=\; x^2 - 2^k\,G^{*\,k-2}\,x + 2^k\,G^{*\,k-1}, \qquad k \geq 3, $$

with the property $1/y_+ + 1/y_- = 1$ for $y := x/G^*$ at every level $k$. The level index $k$ is a discrete dyadic depth.

The compatibility paper's depth-4 curve uses **exactly** the dyadic frequency tower $\{2^k : k = 0, \ldots, 4\}$ that FTD's Theorem 8 indexes. The trilogy's curve $\gamma_4$ is therefore a concrete numerical incarnation of FTD's $(1+i)$-tower at depth 4 — the algebraic level $k$ in Theorem 8 corresponds to the dyadic Fourier frequency $2^k$ in the compatibility paper's curve construction.

### 4.2 — Numerical observations on $\gamma_4$ (compatibility paper §7)

| Observation | Value | Status in trilogy | FTD interpretation |
|---|---|---|---|
| Six radial minima | $53.5°,\;66.9°,\;172.8°,\;187.3°,\;293.2°,\;306.5°$ | Numerical (interval-arithmetic certification not yet supplied) | Three doublets near $60°,\;180°,\;300°$ — the $Z_3$ rotational pattern, with each doublet a small splitting around the $Z_3$ ideal location. |
| Winding number | $\mathrm{wind}_0(\gamma_4) = -2$ | Numerical (angle unwrapping) | Consistent with reciprocal projective branches: $z_+ z_- = 1$ implies the projective dynamics has winding $\pm 2$ in a half-cycle. |
| Signed area | $\mathrm{Area}(\gamma_4) = 177\pi/400$ | **Theorem** (Proposition 7.4, exact rational by Green's formula) | Closed-form curve invariant. Verified independently by `proof_fqcr_branch_value.py`. |

### 4.3 — The $6 = 2 \times 3$ sector structure

Compatibility paper Example 5.5 organizes $\gamma_4$'s six sectors as **three doublets**, each containing one $z_+$ and one $z_-$ assignment under a balanced doublet-compatible coloring:

$$ (z_+, z_-),\;(z_+, z_-),\;(z_+, z_-) \quad \text{up to cyclic relabel and global branch exchange.} $$

This gives a $6 = 2 \times 3$ organization: **two reciprocal branch labels across three radial doublets**.

**Interpretation guard rail.** The compatibility paper itself states (Discussion §10):

> "Any additional interpretation of what the sectors represent is external to the compatibility theory and should not be used as a proof assumption."

FTD adopts this guard rail. The numerical equalities $2 = |\{z_+, z_-\}|$ (reciprocal branch labels) and $3 = q/2$ (radial doublet count) are real; *any* assignment of physical meaning to either factor (e.g. "$N_c = 3$", "three SM generations", "$3 = $ Moore polyhedral count") is **not justified by the compatibility paper**, is **not adopted into FTD's algebraic spine**, and is **not load-bearing for any FTD claim**. The numerical $6 = 2 \times 3$ structure is documented as a structural correspondence to investigate, never as an identification.

---

## §5 — The compatibility framework's genuinely new content

Section §3 above lists what cross-confirms FTD or reproduces standalone trilogy results. This section lists what is **genuinely new structural content** the compatibility paper supplies that FTD did not have before 2026-05-07.

### 5.1 — Curve-side geometric spectrum

Pre-trilogy FTD had no formal notion of a *dyadic Fourier curve* spectrum. Compatibility paper Definition 3.4 introduces $\mathfrak{F}_N(\theta) = (N, \mathrm{wind}_0, \mathrm{Area}, I, M_N(\theta), \mathcal{R}_N, \mathcal{B}_N)$ with self-intersection count, roughness, and box-counting statistics. This is a **genuinely new structural object** for FTD's reference shelf, even though no FTD claim currently depends on it.

### 5.2 — Sector decomposition + branch coloring

Compatibility paper §4 introduces:
- Radial sector partition $\mathcal{S}_N$ from radial minima (Definition 4.1).
- Branch set $\mathcal{Z}_N(t) = \{z_+(t), z_-(t)\}$ from the projective fixed branches (Definition 4.2).
- Sector coloring $c: \mathcal{S}_N \to \mathcal{Z}_N(t)$ as the structural map (Definition 4.3).
- Balance and doublet-compatibility conditions (Definitions 4.4–4.5).

This is the **categorical cement** between branches (algebraic side) and sectors (geometric side). FTD did not have this framework pre-trilogy.

### 5.3 — Compatibility levels (weak / sector / strong)

Compatibility paper §5 stratifies the bridge into three levels:

- **Weak** (Definition 5.1): five concrete conditions on $\mathfrak{Q}_N(t)$ and $\mathfrak{F}_N(\theta)$. **Open** — persists under perturbation by Proposition 5.2 (theorem-grade).
- **Sector** (Definition 5.3): existence of a coloring; balanced if even-sector and the coloring is balanced.
- **Strong** (Definition 5.4): coloring is stable under perturbation AND generated by a spectral rule depending only on spectral data.

Each level is testable. The strong level corresponds to a "canonical bridge" that the paper conjectures (9.3) exists for the five-harmonic model.

### 5.4 — Certification protocols

Compatibility paper §8 specifies two interval-arithmetic certification protocols:

- **Certified weak compatibility** (Protocol 8.1): six steps verifying $G_N^* > 0$, $R_N(t) > 0$, $\gamma_N(t) \neq 0$ for all $t$, winding-number certification, root isolation of $\rho'_N(t)$, and threshold-boundary separation.
- **Certified sector compatibility** (Protocol 8.2): five steps adding sector partition construction, branch coloring rule specification, invariance proof, and stability under perturbation.

These protocols are the **methodological template** for FTD-0143 Test 2 (the (4,6;3,2) uniqueness scan). The trilogy's interval-arithmetic discipline matches FTD's pre-registration discipline; both are forms of methodological hygiene that prevent overclaim.

### 5.5 — Conjectures (compatibility paper §9)

| Conjecture | Statement | FTD bookkeeping |
|---|---|---|
| 9.1 Six-sector branch compatibility | The five-harmonic model admits a stable 6-sector decomposition into 3 doublets, balanced sector-compatible | Not adopted as FTD claim; documented as trilogy conjecture. |
| 9.2 Compatibility persistence | Weak + balanced sector compatibility persist on an open coefficient + recurrence-parameter neighborhood | Methodologically aligned with FTD's perturbation-stability analyses; not adopted. |
| 9.3 Nontrivial strong compatibility | A spectral rule exists that canonically colors the six sectors by the two reciprocal branches, up to cyclic relabel and branch exchange | Not adopted; would be load-bearing if the rule were specified and derivable. |
| 9.4 Functorial compatibility | A category exists with depth-$N$ objects and harmonic extensions $N \mapsto N+1$ as morphisms; strong compatibility is functorial | Not adopted; relevant if the trilogy is extended to a tower of depths. |

**No conjecture from the compatibility paper is promoted to FTD claim status.** Each is documented as an external structural conjecture that FTD's $(1+i)$-tower could in principle interact with.

---

## §6 — Mapping into FTD's documentation surface

The trilogy's integration into FTD is deliberately minimal:

| Doc | Edit | Reason |
|---|---|---|
| `REF_QCR_TRILOGY_BRIDGE.md` (this doc) | NEW | Single canonical reference for the trilogy's content and its correspondence to FTD constructs. |
| `SPEC_FQCR.md` §3.3, §5 | Small additions | Cross-link to this bridge doc; note that compatibility paper Definition 2.2 $\kappa_N(t) = R_N(t)/(16 G_N^*)$ is the same $R_N(t)$. |
| `DERIV_GSTAR_QUARTER_CONJUGACY.md` §5 | One-paragraph note | Curve-side bridge cross-ref. |
| `LEDGER.md` | New row FTD-0144 | [STRUCTURAL CORRESPONDENCE] tag. Trilogy correspondence documented; spine count unchanged — nine numbered results (six theorem-grade + three honestly-tiered, see `SPEC_ALGEBRAIC_SPINE.md` §0); OT-1.7 / OT-1.8 unchanged. |
| `META_INDEX.md` | New row | Discoverability. |
| `scripts/proofs/proof_fqcr_branch_value.py` | NEW | Verifies depth-4 Area = $177\pi/400$ via two independent paths (rational arithmetic + numerical Green's-formula integration). |

**No new spine theorem.** **No tier promotion in `TRACKER_ONTIC_TRUTH.md`.** **No FTD claim depends on a trilogy conjecture.**

---

## §7 — Reproducibility

[`proof_fqcr_branch_value.py`](../../../scripts/proofs/proof_fqcr_branch_value.py) verifies:

1. **Exact rational arithmetic.** $\sum_{n=0}^{4} 2^n a_n b_n = 1 - \tfrac{1}{2} + 1 - \tfrac{28}{25} + \tfrac{1}{16} = \tfrac{177}{400}$ (LCM 400 denominator), so $\mathrm{Area}(\gamma_4) = 177\pi/400$ exactly. Asserts equality to the closed form claimed in compatibility paper Proposition 7.4.
2. **Numerical Green's-formula cross-check.** Computes $\tfrac{1}{2}\int_0^{2\pi}(x_4\,y'_4 - y_4\,x'_4)\,dt$ at 50-digit precision (mpmath) and asserts agreement with the rational value to $< 10^{-40}$.
3. **Reciprocal branches sanity.** For sample $s \in \{1.5, 2, 4, 8, 16, 16 G^*\}$ (the last being the FTD master-quadratic relevant value scaled into the symmetric form), verifies that the roots of $z^2 - s z + 1 = 0$ satisfy $z_+ z_- = 1$ to machine precision.

The script is read-only, deterministic, and runs in under one second. It does NOT verify FTD-side claims; it verifies the trilogy's own published numerical content as a sanity check.

---

## §8 — What this bridge does NOT establish

To prevent overclaim, in line with the compatibility paper's §10 self-restraint:

- Does NOT identify the $6 = 2 \times 3$ sector structure with $N_c = 3$, three SM generations, or any other physical "3".
- Does NOT promote any of compatibility-paper Conjectures 9.1–9.4 to FTD claim status.
- Does NOT supply a derivation chain from the trilogy to the master quadratic's $\alpha^{-1} = x_+$ identification (FTD-0013 stays at [STRONGLY MOTIVATED CONJECTURE]).
- Does NOT extend FTD's algebraic spine. The spine count is unchanged — nine numbered results (six theorem-grade + three honestly-tiered; see `SPEC_ALGEBRAIC_SPINE.md` §0).
- Does NOT provide an a-priori interpretation of the FQCR base point $t = 1$. Test 3 (running behaviour) still gates on that interpretation.
- Does NOT execute Test 2 of the FQCR program (FTD-0143 (4,6;3,2) uniqueness scan). The compatibility paper's §8 protocols inform the methodology of Test 2 but do not constitute its execution.

---

## §9 — Cross-references

| Cross-reference | Purpose |
|---|---|
| `SPEC_FQCR.md` | FQCR Models I–V; this bridge doc complements it from the curve side. |
| `SPEC_ALGEBRAIC_SPINE.md` Theorem 8 (FTD-0111) | $(1+i)$-tower; depth-4 five-harmonic curve is its concrete numerical incarnation. |
| `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` Part VII | Transfer-matrix interpretation; reciprocal projective branches $z_+ z_- = 1$ live here. |
| `DERIV_GSTAR_QUARTER_CONJUGACY.md` (FTD-0141) | Operator-theoretic provenance of $G^*$; the trilogy's $G_N^*$ is its finite-N approximant. |
| `DERIV_GSTAR_FINITE_APPROX.md` (FTD-0142) | Finite-N attractor formula; identical to compatibility paper Definition 2.1. |
| `PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md` (FTD-0143) | Pre-registered uniqueness scan; methodologically informed by compatibility paper §8 certification protocols. |
| `LEDGER.md` FTD-0144 | Structural-correspondence row for the trilogy. |

---

## §10 — Refresh policy

If any of the following land:

- **Compatibility paper Conjecture 9.3 is proved or disproved**: update §5.5 of this doc; if proved, evaluate whether the canonical sector-coloring rule's input data (radial minima + winding + branch reciprocity) admits an FTD-axiomatic interpretation. *Even then, do not promote to FTD claim status without an independent FTD-side derivation.*
- **A depth-$k > 4$ analogue of the five-harmonic model is supplied with comparable numerical content**: extend §4 with a multi-depth table; cross-reference Theorem 8's $k$-indexed tower more tightly.
- **FTD-0143 Test 2 executes**: this bridge doc is unchanged; FTD-0143's analysis doc handles the result. Cross-link from §6 here.
- **A fourth paper extends the trilogy**: re-evaluate whether `09_mathematical/` is still the right home or whether a `10_eft_program/` move is warranted.

Until then, this v1.0 is the canonical FTD-side reference for the trilogy.
