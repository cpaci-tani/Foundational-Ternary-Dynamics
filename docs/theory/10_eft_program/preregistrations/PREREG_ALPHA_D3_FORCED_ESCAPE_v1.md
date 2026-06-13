# PRE-REGISTRATION — The D=3 FORCED-escape: can a reducible 3-dimensional operator force the α-readout assembly? (the RSI Leg-3 crux)

**Status:** `[PRE-REGISTRATION]` — design lock; the computation runs only after the hash-lock.
**Date:** 2026-06-13
**LEDGER id (reserved):** FTD-0284
**Git tag (to be applied at lock):** `preregister-alpha-d3-forced-escape-v1`
**Target:** the single surviving exit of MC-T4.3 — the **FORCED-escape** flagged "explicitly
still live" in `AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md` (FTD-0242) and
`AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md` (FTD-0243): a *reducible / 3-dimensional /
infinite-descended* single operator co-realizing the trace's complex structure and the
determinant's three-plane product **without recollapsing** to unbroken O_h.
**Posture:** a genuine, rigorous swing at α. **Prior-favoured outcome: CLOSED-NEGATIVE**
(the C₄/C₃ wall is robust; every concrete rank-2 version has collapsed). The value is in
making whichever verdict lands a theorem-grade contribution to the boundary.

---

## §1 · Background (what is and is not already settled)

The fine-structure identification `x₊ = 1/α` (master quadratic `x² − 16G*²x + 16G*³ = 0`,
1.26 ppm) is `[STRONGLY MOTIVATED CONJECTURE]` (FTD-0013). MC-T4.3 reduces the question to
forcing the **operator assembly** `(Tr, Det) = (16G*², 16G*³)` from the postulates. Settled
facts (cited, not re-litigated):

- **Forward-forced** `[THEOREM]`: the trace `16G*²` (16 = |ℤ[i]^×|², G*² = 2π·G_BCC(0) Watson)
  and a clean odd source `G* = det_ζ(D_{3/4})/det_ζ(D_{1/4})` (FTD-0234).
- **Unforced** (W-CRIT-2, FTD-0235): for a 2×2, Tr and Det are independent; nothing forces
  `det = 16G*³` over `16G*⁴` (root 130.68) etc.
- **Closed `[THEOREM]`** for the **rank-2 finite** calculus 𝔠 over `V_complex ≅ ℤ[i]²`
  (K-BIND, FTD-0244): all such operators have invariants in `Q(G*)`; the assembly is
  unforced; α is dynamical, not structural.
- **The C₄/C₃ wall** (Legs 1–2, machine-checked): the trace's `i` needs a single C₄ axis
  (`mult_O(E)=0`); the determinant's `G*³` as a C₃(⟨111⟩) three-plane product needs C₃ ∈ Stab;
  `⟨C₄, C₃⟩ = O` (order 24) ⇒ unbroken O_h ⇒ no localized charge ⇒ no readout.
- **3b-scope `[THEOREM]`**: no C₃-equivariant *rank-2 restriction* of the three-plane source
  carries `(16G*², 16G*³)` (Schur on the C-plane ⇒ conjugate pair, not real-distinct).

**The single open item** (FTD-0242 §6, FTD-0243 surviving exit): Leg 3 — a reducible /
3-dimensional / infinite-descended *single* operator that is **not** a C₃-equivariant rank-2
restriction, co-realizing both invariants without collapse. This pre-registration attacks it.

## §2 · The new structural reading (the motivation, not a result)

The two exponents `(2, 3)` of `(G*², G*³)` admit a reading no closed route used:
**`(2, 3) = (D−1, D)` for D=3.** The determinant's three G\* factors are the **three
coordinate planes** (C₃-symmetric det_ζ three-plane product, one G\* per plane ⇒ G*³); the
trace's two are the transverse Watson G*²; `16 = 2⁴ = |ℤ[i]^×|²`. If the determinant exponent
is forced to **D** by dimensionality (three planes ⇒ three factors), the assembly stops being
a free W-CRIT-2 choice. The escape hypothesis: **C₄ acts on a 2-dimensional readout subspace
(carrying the trace's `i`) while C₃ acts on the ambient ℝ³ (carrying the determinant's three
planes) — different structures, so `⟨C₄, C₃⟩` need not act on the readout and the wall is
evaded.** Whether this is realizable or itself collapses is the question.

## §3 · The precise question (frozen)

Let the FTD-native generator set 𝒢 be exactly:
1. **Three coordinate-plane sources** `g_{xy}, g_{yz}, g_{zx}`, each = `G*` (the J-twisted
   det_ζ ratio per plane, FTD-0234 [THEOREM]), permuted by C₃(⟨111⟩).
2. A **complex structure** `J` (`J² = −I`) on a single chosen C₄ axis's perpendicular plane
   (the V_complex = ℤ[i]² structure, FTD-0006/0122 [THEOREM]).
3. The **Watson even scalar** `G*²` (BCC self-energy, FTD-0002 [THEOREM]) and the integer
   `16 = |ℤ[i]^×|²` (FTD-0006 [THEOREM]).
4. Real linear combination, direct sum (reducibility), and restriction-to-subspace.

> **Q (the FORCED-escape):** Does there exist a real operator `M` on ℝ³ (or a finite
> reducible/graded extension), built **only** from 𝒢 by the operations in (4), and a
> **2-dimensional real readout subspace** `R ⊂` domain(M), such that
> **(i)** `M|R` has characteristic polynomial exactly `x² − 16G*²x + 16G*³`
> (Tr = 16G*², Det = 16G*³, real-distinct eigenvalues);
> **(ii)** `Det(M|R) = 16G*³` is **inherited** from the C₃-symmetric three-plane product
> `g_{xy}·g_{yz}·g_{zx} = G*³` times `16` — i.e. the determinant value is a forced functional
> of the three plane-sources, **not** an independently chosen matrix entry (no hand-placed
> companion-form determinant; banned move F-HP below);
> **(iii)** the construction does **not** require both C₄ and C₃ to stabilize `R` (no
> collapse to O); C₄ may stabilize `R`, C₃ may act on the ambient, but `⟨C₄, C₃⟩` must not be
> forced to act faithfully on `R`?

## §4 · Frozen artifacts

| Artifact | role | SHA256 |
|---|---|---|
| `scripts/exploration/alpha_d3_forced_escape.py` | the symbolic (sympy) co-realizability test | `b320e0eb1d45abc81bb2c0ddeab348adb2ea2ea6627a087f500a4830b752bda4` |

**Integrity note (deterministic symbolic math):** unlike a stochastic measurement, this is a
deterministic symbolic computation — the result is a mathematical fact with no
researcher-degrees-of-freedom to manipulate. The pre-registration's role is to freeze the
**question** (§3), the **three-outcome criteria** (§6), the **adversarial gate** (§7), and the
**banned moves** (§8) before stating the answer. The verdict (below, in the analysis doc)
follows necessarily from the frozen criteria applied to the algebra.

## §5 · Method (frozen)

The computation is **symbolic and exhaustive over the admissible operator family**, not a
numerical scan. It:
1. Builds the generator algebra 𝒢 over ℝ with `G*` a free transcendental symbol.
2. Parameterizes the most general real `M` on ℝ³ ⊕ (optional graded summands) whose entries
   are 𝒢-admissible (real polynomials in `G*` with the plane-sources entering only through
   C₃-covariant combinations, and `J` only on the chosen C₄ plane).
3. For every 2D real subspace class `R` (axis⊕plane-direction, plane, graded-diagonal),
   computes `Tr(M|R)` and `Det(M|R)` symbolically and tests (i)+(ii)+(iii).
4. Records the C₄/C₃ stabilizer of any `R` that satisfies (i)+(ii) and checks (iii).

## §6 · Frozen verdict logic

- **FORCED:** at least one `(M, R)` satisfies (i) ∧ (ii) ∧ (iii) — i.e. the determinant
  `16G*³` is inherited (not placed) and the readout does not force ⟨C₄,C₃⟩. ⇒ the operator
  assembly is forced by the D=3 structure; α becomes `[DERIVED, modulo this construction]`,
  MC-T4.3 reopens positive. (This outcome demands the adversarial-verification gate §7.)
- **CLOSED-NEGATIVE:** it is shown that **every** `(M, R)` satisfying (i) violates (ii) or
  (iii) — i.e. forcing `Det = 16G*³` from the three-plane product always either requires the
  determinant to be hand-placed or collapses the readout symmetry to O. ⇒ the FORCED-escape
  is closed; FTD-0244 extends to D=3; MC-T4.3 upgrades from `[FOUNDATIONAL OBSTRUCTION]`
  (conjecture-grade no-go) toward `[THEOREM no-go]` (proof-grade, modulo the 𝒢 axiomatization).
- **UNDERDETERMINED:** the family is too large to exhaust symbolically, or a candidate
  satisfies (i)+(ii) but (iii) is undecided. ⇒ the open question is sharpened with the
  specific residual obligation stated; no status change.

## §7 · Adversarial-verification gate (mandatory for any FORCED claim)

A FORCED verdict is **provisional** until an independent adversarial pass confirms the
construction does not smuggle in a banned move. The verifier must check, against the explicit
matrix: (a) the determinant is a forced functional of `g_{xy}·g_{yz}·g_{zx}`, not an
independent entry; (b) no C₄/C₃ co-stabilization of `R` is hidden in a basis choice; (c) the
real-distinct spectrum does not secretly require a non-real (scalar-i) operator on a
C₃-equivariant subspace (the 3b mechanism). A FORCED claim that fails any check reverts to
CLOSED-NEGATIVE or UNDERDETERMINED. (FTD-0278's hydrogen overclaim — a non-probative falsifier
that passed — is the cautionary precedent; this gate is its lesson applied.)

## §8 · Pre-declared exclusions (banned moves)

1. **F-HP (hand-placed determinant):** writing the companion form `[[0, −16G*³],[1, 16G*²]]`
   or any `M` whose determinant entry is set to `16G*³` by fiat rather than derived from the
   three plane-sources. (This is the witness for W-CRIT-2, not a derivation.)
2. **No re-running closed routes:** R1 transverse stiffness, R2 source-current normalization,
   R3 two-sector response eigenvalue, R4 arithmetic-only, the jtwist/bcc/cm force-routes, the
   3b C₃-equivariant rank-2 restriction, ARC-D1 cluster-fission, Mechanisms A/B/C for g_c.
3. **No numerical near-miss / coincidence scans** (epistemic-discipline rule).
4. **No 6th-postulate import:** the construction must use only 𝒢 (§3); adding an axiom that
   names the assembly is the trivial non-result and is excluded.
5. **No tag promotion on UNDERDETERMINED:** FTD-0013 stays `[SMC]`; MC-T4.3 stays
   `[FOUNDATIONAL OBSTRUCTION]` unless FORCED (passes §7) or CLOSED-NEGATIVE (proof complete).

## §9 · Honest ceiling

- FORCED would yield `[DERIVED, modulo the D=3 construction + the 𝒢 axiomatization]` — a
  conditional derivation (the strongest result the project has ever had on α), **not**
  unconditional; the 𝒢 generator set is itself a modelling choice to be stated.
- CLOSED-NEGATIVE would yield a `[THEOREM no-go]` relative to 𝒢 — "FTD cannot force α from
  P1–P5 via any reducible D=3 operator built from the native generators" — the sharpest
  possible statement of the boundary (Number-One-Goal clause 2).
- Neither promotes `x₊ = 1/α` to `[DERIVED]` unconditionally; FC-class commitments and the
  spine are untouched.

## §10 · Hash-lock declaration

This document and the §4 artifact are committed together and tagged
`preregister-alpha-d3-forced-escape-v1` BEFORE the §5 computation runs. Any post-lock edit to
§§3, 5–8 or the artifact invalidates the lock and requires a v2.
