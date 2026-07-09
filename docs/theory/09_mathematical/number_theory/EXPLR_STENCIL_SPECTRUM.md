# EXPLR — The stencil spectrum of N_dyn: the arithmetic of the spec's three lattice symbols

**Tag:** [EXPLR — B0 of the Clause-2/3 program] → **[NUMERICAL FACT — the 18-point default-stencil Green's function's annihilating ODE computed (order 4, degree 12), classified, and factored; FTD-0372]**. **P3(a) CLOSED** (order-4 operator) and **P3(b) CLOSED** (irreducible over ℚ̄(z), Sage/ore_algebra; not a symmetric cube; W₁₈ is a genuinely new order-4 period, not a classical elliptic Γ-quotient). **P3 residual, rigid-CY branch CLOSED NEGATIVE (FTD-0373):** W₁₈'s local system is **not self-dual** (exact local-exponent argument, §2.5), so it is not a rigid-Calabi–Yau / weight-4-modular / Sym^k-elliptic / K3 period; the remaining open piece is its Hadamard-diagonal closed form. Promotes nothing about α or δ; the golden gate is untouched.
**Program:** Clause-3 ("N as the object"), stage B0 — now complete. LEDGER: FTD-0372.
**Audience:** agents working the δ-IND residues (E1's precise statement), the Ram(N) flagship, or any future evaluation of the engine's own Green's function.

---

## §0 — The question

N_dyn's generators are limits of solves against the spec's lattice symbols. Which symbols, and what is the *arithmetic* of each symbol's Green's function? The engine's default is the 18-point (SC+FCC)/2 Laplacian with **zero BCC weight** (AUDIT_LINK8_CLOSURE §2); the BCC and SC symbols enter as sublattice projections (spec-level modes). The gap this note opened — **the engine's own default Green's function is arithmetically uncharted** (W₁₈(0) ≈ 1.2679) — is now **charted**: §2 computes its exact annihilating operator.

## §1 — The spectrum table (B0 complete, 2026-07-05)

| symbol | σ(k) | Green's value at 0 | arithmetic status | source |
|---|---|---|---|---|
| BCC (8 corners) | 1 − c_x c_y c_z | G\*²/(2π) = Γ(1/4)⁴/(4π³), **exact** | **CM, τ = i** (lemniscatic); hull-class ℚ̄·s⁴w⁻⁴ | Watson 1939; spine Thm 5 / OT-2.1 |
| SC (6 faces) | 3 − Σc_i (norm.) | ≈ 0.505462019 (Watson's I₃) | **Γ(1/24)-class** closed form (Glasser–Zucker); outside ℚ(G\*, π) — the source of E1; its LGF ODE is **order 3** (Joyce) | Watson 1939; Glasser–Zucker 1977/1980; Joyce 1973 |
| FCC (12 edges) | 3 − Σc_i c_j (norm.) | tabulated | **Γ(1/3)-class** (equianharmonic-adjacent); outside ℚ(G\*, π) — E1's second member; LGF ODE **order 6** (Koutschan 2013) | Glasser–Zucker 1980; Koutschan 2013 |
| **18-pt (SC+FCC)/2 — the engine default** | 1 − (1/6)Σc_i − (1/6)Σc_i c_j | ≈ 1.2679 | **Holonomic; minimal ODE order 4, degree 12; IRREDUCIBLE over ℚ̄(z) (FTD-0372, §2).** Sits strictly between the SC (order 3) and FCC (order 6) — *neither* classical lattice. Every genuine local exponent ∈ ½ℤ; no MUM point; **and (Sage/ore_algebra) the operator does not factor and is not a symmetric cube of an order-2** ⇒ **W₁₈ does NOT reduce to a classical elliptic Γ-quotient** — it is a *genuinely new order-4 period*, outside ℚ(G\*,π). This is the *opposite* of the sibling SC/FCC/BCC lattices, whose constants are all classical CM Γ-classes. | **this note (FTD-0372)**; Guttmann 2010; Lipshitz 1988; ore_algebra |

The program consequence, now settled: the default stencil is **not** arithmetically generic in the pejorative sense — order 4 with a ½ℤ-exponent lattice is *structured* — but the ½ℤ-exponent lattice that *looked* like a symmetric-power/elliptic-core signature does **not** in fact reduce to one. The operator is irreducible (§2), not a symmetric cube (§2), and its local system is **not self-dual** (§2.5), so it is neither an elliptic Γ-quotient nor any pure self-dual (rigid-CY / K3 / Sym^k) period. The default mixture is a genuinely new order-4 period whose home is the theory of diagonals of rational functions, not the classical Γ-world. [resolved — FTD-0372 + FTD-0373]

## §2 — B0(ii/iii), RESOLVED: the operator computed and classified

**Well-posedness.** F(z) = Σ_n m_n z^n with m_n = CT_k[σ₁₈(k)^n] is the constant term of the rational function 1/(1 − z·σ₁₈), hence **D-finite / holonomic** (Lipshitz 1988) — the annihilating ODE is *guaranteed* to exist; the only questions are its size and its arithmetic class.

**Method (the harder push over B0's first attempt).** Three exact stages, no PSLQ, no closed-form fishing — a reconstruction of a *structural* object (the ODE) from exact data, the standard LGF method:
1. **Exact moments** `explr_stencil18_moments.py` — 171 exact integer moments M_n = CT[(2A+B)ⁿ] (2A = Σ 2(x_i+1/x_i), B = Σ_{i<j}(x_i+1/x_i)(x_j+1/x_j)) via **meet-in-the-middle** (M_n = ⟨v_a, v_{n-a}⟩ with T symmetric ⇒ propagate only to depth n/2, ~16× faster). First values 1, 0, 36, 336, 6588, 110880, 2106720 (match the earlier B0 attempt).
2. **Modular-rank reconstruction** `explr_stencil18_reconstruct.py` — since gcd(24,p)=1, m_n mod p is a machine int, so the (order, degree) relation matrix is built and rank-reduced over 𝔽_p (no bigint blowup). Scan order 2..12; the minimal (order, then degree) with nullity ≥ 1 and large surplus, cross-checked on **two 61-bit primes**, is the operator; the exact rational coefficients are then extracted at that one size.
3. **Exact classification** `explr_stencil18_classify.py` — local exponents at every rational singular point and ∞ via the Euler-operator (θ = z d/dz) indicial method, exact in sympy.

**Pipeline validated on two textbook controls (reproducible, committed).** A parameterized companion to the 18-pt scripts — `explr_lgf_reconstruct.py` (reconstruction, moments+divisor parameterized) + `explr_lgf_classify_sage.py` (classification, the *same* self-validated ore_algebra factorizer as the 18-pt run) — recovers the *known* LGF operators of the neighbouring lattices exactly, before any claim about the 18-pt case:

- **2D square** → **irreducible order 2, degree 3** (nullity 1 on two 61-bit primes, surplus 104): the `(z→z²)`-pullback of the complete-elliptic-integral `₂F₁(½,½;1)` operator `z(z²−1)y″ + (3z²−1)y′ + zy`, true singular locus {0, ±1}, `{0,0}`+log exponents at each finite point (elliptic-K). `_square2d_operator.json`.
- **3D simple-cubic** → **irreducible order 3, degree 6** (surplus 54): Joyce's operator, `p₃(z) = z²(z²−1)(z²−9)`, true singular locus {0, ±1, ±3}, with its correct odd-moment vanishing (M = 1,0,6,0,90,0,1860 = OEIS A002896). `_sc3d_operator.json`.

Both `right_factor()` → None / single-factor (irreducible), on the same self-validated tool (reducible Fuchsian → order-1 factor; elliptic-K → None) that returns the 18-pt operator's irreducibility. That the identical reconstruct+classify pipeline gets these two *known* answers right — the right order, the right singular locus, the right irreducibility — is what underwrites its order-4/irreducible verdict for the 18-pt case. (B0's earlier 18-pt attempt missed the operator only because it capped order 4 at degree 10, and the true degree is 12 — just outside that box.)

**The operator (order 4, degree 12).** No order-2 or order-3 operator exists (scanned to degree ~155); the minimal operator is order 4, found with **nullity 1 on both primes and surplus 90** (155 exact-moment equations vs 65 unknowns — a massive overdetermination that serves as the certificate). Its leading coefficient factors completely over ℚ:

> **p₄(z) = 4 · z³ · (z−1)(z+2)(z+3)(z+6)(z+8) · (3z⁴ + 16z³ + 24z² − 24z + 16).**

The full integer operator is saved by the reconstruction script (`_stencil18_operator.json`, regenerable).

**Local exponents (exact).** The singular points and their indicial exponents:

| point | exponents | reading |
|---|---|---|
| z = 0 | {0, 0, 0, ½} | solution basis {√z, 1, log z, log²z}: a **rank-3 unipotent** block + a ½ branch — **not** a MUM {0,0,0,0} point |
| z = 1 (physical, radius = 1/max\|σ₁₈\| = 1) | {0, ½, 1, 2} | carries the 3D lattice's (1−z)^{½} branch |
| z = −2, −3 | {0, ½, 1, 2} | genuine singularities |
| z = −6, −8 and the 4 roots of 3z⁴+16z³+24z²−24z+16 | integer exponents | **apparent** (removable) — ore_algebra confirms |
| z = ∞ | {1, 3/2, 5/2, 3} | half-integer-shifted |

The **true singular locus is {0, 1, −2, −3, ∞}** (Sage/ore_algebra; z=−6, −8 and the complex roots of the leading coefficient are apparent). **Every genuine local exponent lies in ½ℤ.**

**The factorization (Sage + ore_algebra) settles P3(b) — and refutes the natural guess.** The ½ℤ-exponent lattice *looked* like the signature of a symmetric power / √-twist of a second-order elliptic operator (the simple-cubic order-3 LGF *is* the symmetric square of an elliptic order-2, with W_SC a Γ(1/24)-quotient) — which would have handed W₁₈ its own Γ-quotient closed form. The differential-operator factorization decides otherwise, three ways:

- **The operator is IRREDUCIBLE over ℚ̄(z).** `right_factor()` returns None and `factor()` returns the operator itself (Sage 9.5 + ore_algebra 0.5). The factorizer **self-validated** first, in the same finite-singularity regime: a Fuchsian *reducible* operator → an order-1 right factor, and the irreducible *elliptic-K* operator → None. Script: `factor_stencil18_sage.py`.
- **It is not a symmetric cube of an order-2 operator** — an exact-arithmetic argument independent of the CAS: if it were `Sym³(M)` with M's z=0 exponents {a,b}, the operator's exponents would be {3a, 2a+b, a+2b, 3b}, a multiset that can hold three equal values only when a=b (which forces all four equal); but z=0 has {0,0,0,½}, three-and-one. No {a,b} produces it.
- **No MUM point**, so it is not a Calabi–Yau operator in the strict AESZ sense either.

Therefore W₁₈ is the period of a **genuinely irreducible order-4 operator**: it does **not** reduce to a classical elliptic (order-2, Γ-quotient) period the way the SC/FCC/BCC lattice constants do. The substrate's own default Green's constant is arithmetically *new* — outside ℚ(G\*, π) not as a quadratic surd (like δ) nor as a sibling Γ-class (like W_S), but as a higher, order-4 period. (Whether that period has its *own* non-classical closed form — a rigid-Calabi–Yau weight-4 modular link, or a quadratic pullback rationalizing the ½ℤ lattice — is a further question; the P3(b) dichotomy is resolved on the "does not reduce to order-2" side.)

## §2.5 — The self-duality obstruction (P3 residual, rigid-CY branch): CLOSED NEGATIVE (FTD-0373)

The residual left by P3(b) — whether this genuinely-new order-4 period nonetheless has a *rigid-Calabi–Yau / weight-4 modular* closed form — is now settled **negative**, by an exact local-exponent argument independent of the CAS.

Every such identification (rigid-CY H³, K3/Sym² H², Sym³ of an elliptic curve, or a weight-4 newform L-value of the Mazur–van Straten–Yui / Gouvêa–Yui type) requires the operator's 4-dimensional monodromy local system to be a **self-dual** polarized variation of Hodge structure — to carry a nondegenerate monodromy-invariant bilinear form (symplectic → the CY/Sym³ case; orthogonal → the K3/Sym² case). A polarized VHS forces the local exponents at **each** singular point to be symmetric about *that point's own* center — the center may differ point to point (the mirror quintic is symmetric about 0 at its MUM point, 1 at the conifold, ½ at ∞). For a 4-element multiset, sorted, that is exactly `a + d = b + c`. (A *single global* weight is **not** required and is not the test — a genuine Calabi–Yau operator also fails a single-global-weight test, so nothing here rests on it.)

**W₁₈ fails the necessary condition at every genuine finite point.** At z = 1, −2, −3 the exponent multiset is {0, ½, 1, 2}, and `0 + 2 = 2 ≠ ½ + 1 = 3⁄2`: it is symmetric about **no** center. So the monodromy carries **no** nondegenerate invariant bilinear form — neither symplectic nor orthogonal — and W₁₈ is **not self-dual**. (At the weaker mod-ℤ monodromy-eigenvalue level the eigenvalues are all ±1 and trivially self-inverse; the obstruction is specifically to the *pure-weight polarized* self-dual structure that the modular identifications need, which is the one that matters here.)

Therefore W₁₈ is **not** a rigid-Calabi–Yau H³ period, **not** Sym²/Sym³ of an elliptic curve, **not** a K3 transcendental piece, and its L-function is **not** a weight-4 newform L-function. The rigid-CY / weight-4-modular branch of the residual is **CLOSED NEGATIVE (FTD-0373)**.

The verifier `explr_stencil18_selfduality_derived.py` derives the exponents from the operator itself (no hardcoding), validates the derivation against a hypergeometric operator with known exponents ({0,¼} at 0, {−1/12,0} at 1, {1/3,½} at ∞), and validates the per-point predicate against genuine self-dual controls — including the mirror-quintic operator whose three points carry *different* centers (0, 1, ½), confirming the test accepts point-varying self-duality and that only W₁₈'s asymmetric multiset fails.

**Positive redirect.** A non-self-dual order-4 period is not a defect but a signature: W₁₈'s natural home is the theory of **diagonals of rational functions** — the LGF is literally the diagonal of 1/(1 − z·σ₁₈(**k**)), and diagonals are generically non-self-dual mixed motives, not pure CY/modular periods. Its *own* closed form (a Hadamard-type factorization of the order-4 operator) is the remaining open question. LEDGER FTD-0373.

## §3 — Verdict: B0 complete, P3(a) **and** P3(b) resolved

**P3(a) — the operator.** The 18-pt default Green's function is holonomic with a **minimal order-4, degree-12** annihilating ODE (reconstructed from 171 exact moments; verified to overdetermination 90 on two primes; pipeline validated on SC). True singular locus {0, 1, −2, −3, ∞}.

**P3(b) — the arithmetic nature: RESOLVED (irreducible).** The differential-operator factorization (Sage 9.5 + ore_algebra 0.5, WSL2; self-validated on a Fuchsian reducible operator and the irreducible elliptic-K operator) shows the operator is **irreducible over ℚ̄(z)**, and the exact exponent argument shows it is **not a symmetric cube** of an order-2 operator; it also has **no MUM point**. So the CM/modular-vs-generic dichotomy resolves on the *generic* side: **W₁₈ does not reduce to a classical elliptic (Γ-quotient) period** — it is a genuinely irreducible order-4 period, a new arithmetic object outside ℚ(G\*, π). The earlier "odds-on symmetric-power hypothesis" is **refuted**. Scripts: `factor_stencil18_sage.py` (D-module), `explr_stencil18_{moments,reconstruct,classify}.py` (operator + exponents). LEDGER FTD-0372.

**Residual (not needed for the P3 verdict) — now half-closed.** The *own* closed form of this order-4 period splits into two branches. The **rigid-Calabi–Yau / weight-4-modular** branch is **CLOSED NEGATIVE** (§2.5, FTD-0373): W₁₈'s local system is not self-dual, so it is not a rigid-CY / K3 / Sym^k / weight-4-newform period. The **remaining open** branch is a quadratic pullback / Hadamard-diagonal factorization rationalizing the ½ℤ-exponent lattice — the correct home for a non-self-dual diagonal period. Neither branch changes the settled classical-vs-new dichotomy.

**Falsifiers.** (i) A literature closed form for the (SC+FCC)/2 mixture (would give W₁₈ directly). (ii) A CAS factorization showing the operator is **irreducible** order 4 — that would make it a genuine Calabi–Yau-class (non-Γ-quotient) period, the opposite verdict, equally publishable. Either outcome re-adjudicates the §1 row.

## §4 — Cross-references

`ANALYSIS_DELTA_IND_CLOSURE_v1.md` (FTD-0369 — why the SC/FCC rows force E1); `AUDIT_LINK8_CLOSURE.md` §2 (the stencil decomposition + W₁₈ numeric); `FOUND_NATIVE_CLOSURE_REALIZABILITY.md` (B1 — W₁₈ ∈ N_dyn realized; this note gives its operator); `REF_BIBLIOGRAPHY.md` §5 (Watson 1939; Glasser–Zucker; Joyce 1973; Lipshitz 1988; Koutschan 2013); `REF_EXPORTED_PROBLEMS_E1_E2.md` §P3 (the mathematician-facing statement — P3(a) now closed, P3(b) = the factorization). Scripts: `explr_stencil18_moments.py`, `explr_stencil18_reconstruct.py`, `explr_stencil18_classify.py`, `factor_stencil18_sage.py` (P3(b) factorization), `explr_stencil18_selfduality_derived.py` (P3 residual / FTD-0373 — the operator-derived self-duality obstruction) (and the superseded first attempt `explr_stencil18_ode_attempt.py` / `_run2.py`, kept for provenance). **Control-lattice validation** (reproducible; §2 "Pipeline validated"): `explr_square2d_moments.py` + `explr_sc3d_moments.py` (exact moments of the 2D-square and 3D-SC LGFs, each with a closed-form self-check), `explr_lgf_reconstruct.py` (general minimal-ODE reconstruction), `explr_lgf_classify_sage.py` (general Sage/ore_algebra classification, self-validating) — recovering the elliptic (2D square, order 2) and Joyce (3D SC, order 3) operators confirms the reconstruct+classify method on lattices whose answers are known from the literature. Committed operators: `_square2d_operator.json`, `_sc3d_operator.json` (regenerable; the large `_*_moments.txt` are gitignored like `_stencil18_moments.txt`).
