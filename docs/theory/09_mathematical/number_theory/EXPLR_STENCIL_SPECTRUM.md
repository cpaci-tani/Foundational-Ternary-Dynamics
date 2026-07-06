# EXPLR — The stencil spectrum of N_dyn: the arithmetic of the spec's three lattice symbols

**Tag:** [EXPLR — B0 of the Clause-2/3 program] → **[NUMERICAL FACT — the 18-point default-stencil Green's function's annihilating ODE computed (order 4, degree 12), classified, and factored; FTD-0372]**. **P3(a) CLOSED** (order-4 operator) and **P3(b) CLOSED** (irreducible over ℚ̄(z), Sage/ore_algebra; not a symmetric cube; W₁₈ is a genuinely new order-4 period, not a classical elliptic Γ-quotient). Promotes nothing about α or δ; the golden gate is untouched.
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

The program consequence, now sharpened: the default stencil is **not** arithmetically generic in the pejorative sense — order 4 with a ½ℤ-exponent lattice is *structured*, and points back toward the classical Γ-world (an elliptic/modular substructure), not away from it. The "nice" Γ-class content need not be confined to the Moore sublattices after all; the default mixture may carry its own (order-2) elliptic core. [coherent-interpretation, pending the P3(b) factorization]

## §2 — B0(ii/iii), RESOLVED: the operator computed and classified

**Well-posedness.** F(z) = Σ_n m_n z^n with m_n = CT_k[σ₁₈(k)^n] is the constant term of the rational function 1/(1 − z·σ₁₈), hence **D-finite / holonomic** (Lipshitz 1988) — the annihilating ODE is *guaranteed* to exist; the only questions are its size and its arithmetic class.

**Method (the harder push over B0's first attempt).** Three exact stages, no PSLQ, no closed-form fishing — a reconstruction of a *structural* object (the ODE) from exact data, the standard LGF method:
1. **Exact moments** `explr_stencil18_moments.py` — 171 exact integer moments M_n = CT[(2A+B)ⁿ] (2A = Σ 2(x_i+1/x_i), B = Σ_{i<j}(x_i+1/x_i)(x_j+1/x_j)) via **meet-in-the-middle** (M_n = ⟨v_a, v_{n-a}⟩ with T symmetric ⇒ propagate only to depth n/2, ~16× faster). First values 1, 0, 36, 336, 6588, 110880, 2106720 (match the earlier B0 attempt).
2. **Modular-rank reconstruction** `explr_stencil18_reconstruct.py` — since gcd(24,p)=1, m_n mod p is a machine int, so the (order, degree) relation matrix is built and rank-reduced over 𝔽_p (no bigint blowup). Scan order 2..12; the minimal (order, then degree) with nullity ≥ 1 and large surplus, cross-checked on **two 61-bit primes**, is the operator; the exact rational coefficients are then extracted at that one size.
3. **Exact classification** `explr_stencil18_classify.py` — local exponents at every rational singular point and ∞ via the Euler-operator (θ = z d/dz) indicial method, exact in sympy.

**Pipeline validated:** the identical machinery recovers the *known* simple-cubic LGF operator (order 3, degree 6 — Joyce) and its correct odd-moment vanishing, before the 18-pt run. (B0's earlier attempt missed the operator only because it capped order 4 at degree 10, and the true degree is 12 — just outside that box.)

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

## §3 — Verdict: B0 complete, P3(a) **and** P3(b) resolved

**P3(a) — the operator.** The 18-pt default Green's function is holonomic with a **minimal order-4, degree-12** annihilating ODE (reconstructed from 171 exact moments; verified to overdetermination 90 on two primes; pipeline validated on SC). True singular locus {0, 1, −2, −3, ∞}.

**P3(b) — the arithmetic nature: RESOLVED (irreducible).** The differential-operator factorization (Sage 9.5 + ore_algebra 0.5, WSL2; self-validated on a Fuchsian reducible operator and the irreducible elliptic-K operator) shows the operator is **irreducible over ℚ̄(z)**, and the exact exponent argument shows it is **not a symmetric cube** of an order-2 operator; it also has **no MUM point**. So the CM/modular-vs-generic dichotomy resolves on the *generic* side: **W₁₈ does not reduce to a classical elliptic (Γ-quotient) period** — it is a genuinely irreducible order-4 period, a new arithmetic object outside ℚ(G\*, π). The earlier "odds-on symmetric-power hypothesis" is **refuted**. Scripts: `factor_stencil18_sage.py` (D-module), `explr_stencil18_{moments,reconstruct,classify}.py` (operator + exponents). LEDGER FTD-0372.

**Residual (not needed for the P3 verdict).** The *own* closed form of this order-4 period — a possible rigid-Calabi–Yau weight-4 modular interpretation, or a quadratic pullback rationalizing the ½ℤ-exponent lattice — is a further, harder question; it does not change the classical-vs-new dichotomy, which is settled.

**Falsifiers.** (i) A literature closed form for the (SC+FCC)/2 mixture (would give W₁₈ directly). (ii) A CAS factorization showing the operator is **irreducible** order 4 — that would make it a genuine Calabi–Yau-class (non-Γ-quotient) period, the opposite verdict, equally publishable. Either outcome re-adjudicates the §1 row.

## §4 — Cross-references

`ANALYSIS_DELTA_IND_CLOSURE_v1.md` (FTD-0369 — why the SC/FCC rows force E1); `AUDIT_LINK8_CLOSURE.md` §2 (the stencil decomposition + W₁₈ numeric); `FOUND_NATIVE_CLOSURE_REALIZABILITY.md` (B1 — W₁₈ ∈ N_dyn realized; this note gives its operator); `REF_BIBLIOGRAPHY.md` §5 (Watson 1939; Glasser–Zucker; Joyce 1973; Lipshitz 1988; Koutschan 2013); `REF_EXPORTED_PROBLEMS_E1_E2.md` §P3 (the mathematician-facing statement — P3(a) now closed, P3(b) = the factorization). Scripts: `explr_stencil18_moments.py`, `explr_stencil18_reconstruct.py`, `explr_stencil18_classify.py` (and the superseded first attempt `explr_stencil18_ode_attempt.py` / `_run2.py`, kept for provenance).
