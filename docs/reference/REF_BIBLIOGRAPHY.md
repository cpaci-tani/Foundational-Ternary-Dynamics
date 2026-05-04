# REF — Bibliography of Classical Sources Cited by FTD

**Purpose:** Central bibliography of external (non-FTD) sources that load-bearing FTD claims depend on. Use this when writing papers, manuscript chapters, or LEDGER detail blocks to ensure proper attribution to classical mathematics.

**Format:** Citations are organized by topic, then chronologically within topic. Each entry includes: author(s), year, title, role-in-FTD (one line). FTD-internal documents are NOT listed here — they live in `META_INDEX.md` and `LEDGER.md`. This file is for *external* references only.

**Status:** Living document. Started 2026-05-03 (FTD-0132 synthesis). Add new entries as new classical citations are discovered to be load-bearing in FTD work.

---

## 1 · Γ-function and special values

**Euler, L. (ca. 1750s).** *Reflection formula:* `Γ(z)Γ(1−z) = π/sin(πz)`. Foundational complex analysis.
- **Used in FTD for:** establishing `Γ(1/4)·Γ(3/4) = π√2`, the identity that converts between `G* = Γ(1/4)/Γ(3/4)` and `G* = Γ(1/4)²/(π√2)`.
- **Citation contexts:** Theorem 1 (`SPEC_ALGEBRAIC_SPINE.md §1`), Theorem 9 (FTD-0112), FTD-0127 (parity twist), FTD-0132 (theta-nullwert synthesis).

**Gauss, C. F. (ca. 1797–1818).** *Lemniscatic period integral and AGM.* Posthumously collected in *Werke*, Bd. III. Modern exposition: Cox, *The Arithmetic-Geometric Mean of Gauss* (1984).
- **Used in FTD for:** the lemniscate constant `ϖ = Γ(1/4)²/(2√(2π))` as half-perimeter of the unit lemniscate; the identification of τ = i as the CM point of `y² = x³ − x`.
- **Citation contexts:** Theorem 1, the geometric content of `G*`, FTD-0117 (G* / ϖ disambiguation).

**Whittaker, E. T. & Watson, G. N. (1927).** *A Course of Modern Analysis.* 4th ed., Cambridge University Press. Chapter 21 (theta functions) and Chapter 22 (Jacobi elliptic functions).
- **Used in FTD for:** standard reference for theta nullwerten including `θ_3(0|i) = π^(1/4)/Γ(3/4)`.
- **Citation contexts:** FTD-0132 (theta-nullwert synthesis).

**Borwein, J. & Borwein, P. (1987).** *Pi and the AGM: A Study in Analytic Number Theory and Computational Complexity.* Wiley.
- **Used in FTD for:** modern textbook coverage of theta nullwerten at CM points; Borwein-Borwein-Joyce evaluation of the simple-cubic Watson integral as a Γ-product at 1/24, 5/24, 7/24, 11/24.
- **Citation contexts:** FTD-0132 (theta synthesis); Phase G small-r Green's function rank-3 module (engineering exploration 2026-05-03 evening).

**Chandrasekharan, K. (1985).** *Elliptic Functions.* Grundlehren der mathematischen Wissenschaften 281, Springer.
- **Used in FTD for:** modern reference for the theory of elliptic functions, periods, and CM evaluations.
- **Citation contexts:** Theorem 1, Theorem 9, FTD-0132.

---

## 2 · L-functions, functional equations, and Hecke characters

**Riemann, B. (1859).** *Über die Anzahl der Primzahlen unter einer gegebenen Größe.* Monatsberichte der Berliner Akademie.
- **Used in FTD for:** the completed Riemann zeta function `ξ(s) = π^(−s/2)·Γ(s/2)·ζ(s)` and its functional equation `ξ(s) = ξ(1−s)`. The chi function `χ(s)` formalism; explicit formula relating ζ-zeros to prime distribution.
- **Citation contexts:** FTD-0127 (parity twist), Result D, the s = 1/2 critical-line center as the unique fixed point of the functional equation.

**Hecke, E. (1918, 1920).** *Eine neue Art von Zetafunktionen und ihre Beziehungen zur Verteilung der Primzahlen.* Math. Zeitschr. 1, 357–376 and 6, 11–51.
- **Used in FTD for:** Hecke L-functions of number fields; the Dedekind zeta `ζ_K(s)` and its factorization for abelian extensions; the L-function `L(s, χ_{−4})` of the Gaussian field `Q(i)` and the factorization `ζ_{Q(i)}(s) = ζ(s)·L(s, χ_{−4})`.
- **Citation contexts:** FTD-0127, FTD-0132, the operational reading of Theorem 9.

**Lerch, M. (1894).** *Sur quelques formules relatives au nombre des classes.* Bull. Sci. Math. (2) 18, 285–284.
- **Used in FTD for:** the explicit formula `L'(0, χ) = Σ_a χ(a)·log Γ(a/q) − L(0, χ)·log q` for non-principal Dirichlet characters χ mod q. Specialized to χ_{−4}, q = 4 gives `L'(0, χ_{−4}) = log(G*/2)`.
- **Citation contexts:** FTD-0127 Result A (left-boundary identity).

**Tate, J. (1950).** *Fourier analysis in number fields and Hecke's zeta-functions.* Ph.D. Thesis, Princeton; published in Cassels & Fröhlich (eds.), *Algebraic Number Theory* (1967), Chapter XV.
- **Used in FTD for:** the modern adelic framework for L-functions; Archimedean local L-factors `Γ_R(s+a)` parametrized by parity `a ∈ {0, 1}`; the derivation of L-function functional equations from local-global compatibility.
- **Citation contexts:** FTD-0127 (parity twist between even and odd Dirichlet characters), the Γ-factor structure of ζ vs L(s, χ_{−4}).

---

## 3 · CM elliptic curves, periods, and Chowla–Selberg

**Chowla, S. & Selberg, A. (1949).** *On Epstein's zeta function (I).* Proc. Nat. Acad. Sci. USA 35, 371–374. Extended in (1967), J. Reine Angew. Math. 227, 86–110.
- **Used in FTD for:** the Chowla–Selberg formula expressing special values of CM L-functions as products of Γ-values at rational arguments. For `Q(i)` (discriminant `d = −4`): the "Γ-product" `∏_a Γ(a/|d|)^{χ_d(a)}` reduces to the canonical `G* = Γ(1/4)/Γ(3/4)` exactly. For higher class numbers `h ≥ 2`: the Γ-product analogue `G*_d` generalizes the construction.
- **Citation contexts:** Theorem 1 (the analytic origin of G*), Theorem 9 (the field-theoretic positioning), FTD-0123 (the h ≥ 2 numerical scan), FTD-0132 (the theta-nullwert synthesis).

**Damerell, R. M. (1970, 1971).** *L-functions of elliptic curves with complex multiplication.* Acta Arithmetica 17, 287–301; 19, 311–317.
- **Used in FTD for:** the modern formulation of CM L-function special values as Γ-products at rational arguments; the per-ideal-class refinement of Chowla–Selberg.
- **Citation contexts:** referenced in FTD-0123 as the proper analogue at h ≥ 2 (not yet computed in FTD scans).

**Joyce, G. S. (1973).** *On the simple cubic lattice Green function.* Phil. Trans. Roy. Soc. London A 273, 583–610.
- **Used in FTD for:** closed-form evaluations of the simple-cubic lattice Green's function via complete elliptic integrals at the singular modulus `k_3`; the rank-3 ℚ-module structure of small-r SC Green's function values.
- **Citation contexts:** Phase G small-r exploration (2026-05-03 evening).

**Glasser, M. L. & Zucker, I. J. (1980).** *Lattice sums.* In *Theoretical Chemistry: Advances and Perspectives* 5, 67–139.
- **Used in FTD for:** comprehensive collection of lattice Green's function and Watson-integral evaluations; the structure of lattice constants for SC, BCC, FCC lattices.
- **Citation contexts:** Theorem 5 (Watson identity W₃ = G*²/(2π) on BCC), Phase G small-r Green's function exploration.

---

## 4 · Transcendence theory

**Chudnovsky, D. V. (1976).** *Algebraic independence of values of exponential and hypergeometric functions.* Proc. ICM, Vancouver, 339–350. Consolidated in Waldschmidt, *Diophantine Approximation on Linear Algebraic Groups* (2000), §1.4.
- **Used in FTD for:** the algebraic independence of `π` and `Γ(1/4)` over `Q`, which makes Theorem 9's statement `Q(G*) ∩ Q(π) = Q` non-trivial.
- **Citation contexts:** Theorem 9 (FTD-0112) is conditional on this. Without Chudnovsky, the π-free statement is unproved.

**Waldschmidt, M. (2000).** *Diophantine Approximation on Linear Algebraic Groups.* Grundlehren 326, Springer.
- **Used in FTD for:** modern textbook reference for Chudnovsky's theorem and the broader transcendence-theoretic landscape.
- **Citation contexts:** Theorem 9 reference.

**Schneider, T. (1957).** *Einführung in die transzendenten Zahlen.* Springer.
- **Used in FTD for:** the Schneider–Chudnovsky transcendence machinery underlying claims about the transcendence of `G*` over `Q` (via algebraic independence of Γ-values and π).
- **Citation contexts:** Theorem 8 (FTD-0111) anomaly factor `A_k` transcendence at `k ≥ 4` cites Schneider.

---

## 5 · Lattice Green's functions and Watson integrals

**Watson, G. N. (1939).** *Three triple integrals.* Quart. J. Math. (Oxford) 10, 266–276.
- **Used in FTD for:** the original Watson integrals for the SC, BCC, and FCC lattices; Watson's evaluation of W_BCC = G*²/(2π) is FTD's Theorem 5.
- **Citation contexts:** Theorem 5 (`SPEC_ALGEBRAIC_SPINE.md §5`), the BCC sub-lattice eigenvalue structure underlying FTD-0029.

---

## 6 · Modern auxiliary references

**Cox, D. A. (1984).** *The Arithmetic-Geometric Mean of Gauss.* Enseign. Math. 30, 275–330.
- **Used in FTD for:** modern accessible exposition of Gauss's lemniscatic and AGM work.

**Bossavit, A. (1998).** *Computational Electromagnetism.* Academic Press.
- **Used in FTD for:** standard reference for lattice exterior calculus, vertex/edge/face decompositions, and the classical proofs that `d² = 0` on lattice differential complexes.
- **Citation contexts:** FTD-0114 (lattice Hodge duality / Bianchi identities).

---

## Maintenance protocol

When citing classical mathematics in FTD documents:

1. **Don't introduce a new citation in a derivation doc without adding it here.** This file is the canonical source for the bibliographic detail.
2. **Don't use this file for FTD-internal documents** (those live in `META_INDEX.md` and `LEDGER.md`). This is for outside-FTD sources only.
3. **When extending an existing entry's "Citation contexts" line**, list new FTD documents that depend on the source.
4. **When writing papers**, copy the relevant entries into the paper's bibliography; this file is the master copy that all derivative bibliographies reference.

The intent is that a number theorist or analytic number theorist reviewing any FTD paper can look at this file and immediately see "yes, FTD's algebraic spine sits squarely in the classical Hecke / Chowla–Selberg / Tate / transcendence-theory literature with proper attribution" rather than "FTD reinvents classical mathematics without citing it."
