# Where We Left Off — 2026-04-19 evening

**Purpose:** single entry point for the next session to recover full
context of the April 19 audit marathon without having to re-read ten
commits.

**TL;DR:** FTD survived a day-long rigour pass. The grand-unified-theory
framing got cut; a smaller, sharper, more defensible research program
is what's left. The master quadratic is a [STRONGLY MOTIVATED
CONJECTURE] algebraic identity, not a dynamical derivation. The engine
is geometric Coulomb with coupling inserted separately. The
foundational ontology shifted from ℤ³-as-totality to
undefined-boundary. Everything that survived is verifiable, and the
remaining [OPEN] items are well-posed research questions with
tractable attacks.

---

## 1 · Read in this order to recover context

1. **This file.** Gives the big picture and the priority queue.
2. **`docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md`** — the
   foundational commitment change from completed-infinity to
   undefined-boundary. Triage table (§8) lists per-file dispositions.
3. **`docs/theory/07_assessment/AUDIT_MASTER_QUADRATIC.md`** — Phase I
   core + three follow-through audits (CM uniqueness, gap-equation
   convergence, first-principles g_c).
4. **`docs/theory/10_eft_program/DERIV_EMERGENT_COULOMB_GEOMETRIC.md`**
   — Phase G theorem + Phase H verification.
5. **`docs/theory/10_eft_program/DERIV_PARTITION_FUNCTION_L2.md`** —
   Phase J: first-ever explicit FTD partition function, shows the
   action is ultralocal in state.
6. **`docs/theory/10_eft_program/OPEN_GC_FROM_FIRST_PRINCIPLES.md`** —
   the three-mechanism scoping; Mechanisms A and C are ruled out, B is
   the only remaining route.
7. **`docs/theory/07_assessment/AUDIT_RATIONAL_FIT_CLAIMS.md`** —
   Option 4 audit; 7 catalog claims downgraded.
8. **`CHANGELOG.md` top section** — all April 19 sections in reverse
   chronological order.

Everything else is supporting detail.

---

## 2 · Current claim tally (post-audit)

### Firm [THEOREM] (5)

- **G*** = Γ(1/4)/Γ(3/4) algebraic identity (Chowla-Selberg)
- **Master quadratic polynomial** x² − 16G*²x + 16G*³ = 0, roots
  137.036 and 3.024 (pure algebra; no dynamics invoked)
- **CM curve uniqueness**: d = −4 is the unique class-number-1 CM field
  whose master-quadratic-shape polynomial hits physical constants
  (verified across all 9 discriminants d ∈ {−3,−4,−7,−8,−11,−19,−43,
  −67,−163})
- **Phase G emergent Coulomb**: α_r(r, L) = 2·r·G_L(r) holds at every
  finite L with zero free parameters (R² = 1.0000 at L=384 in the
  Coulomb tail)
- **Phase J partition-function ultralocality**: the FTD action `S_E`
  depends only on Σ s² (charge count), not on charge placement — so
  classical extremisation cannot fix g_c

### Finitary [THEOREM] (survived the reframe)

- Moore integers {N_base = 4, N_eff = 13, b_3 = 7} uniqueness
- Coefficient 16 = |Aut(E)|² (3 independent arithmetic routes)
- Phase H coupling scaling: α_r(g_c) = g_c² · α_r(1) to 0.0000%
- D = 3 from |Aut(E)|² = 2^D · (D−1)!
- Structural nulls: N_monopole = 0, N_SUSY = 0, τ_proton = ∞ (pointwise
  charge conservation)

### [STRONGLY MOTIVATED CONJECTURE]

- **x+ = 1/α physical identification** — 1.26 ppm match, unique among
  60k scanned polynomials and 9 CM curves, but not a dynamical
  derivation
- **x− = N_c physical identification** — 0.80% match, dual-prediction
  with x+
- **m_e = m_P √(2π) (16/3) α¹¹** — 0.19% error; tightest among 6489
  scanned rational-prefactor / integer-exponent combinations
- **m_p / m_e = N_eff/α + N_base·N_eff + N_c** — 173 ppm

### [STRUCTURALLY MOTIVATED PARAMETRIC] (downgraded today)

- **sin²θ_12 = 3/10**, **sin²θ_23 = 16/29** — 1-2% errors, 2-4
  small-rational competitors each
- **Δm²₃₁/Δm²₂₁ = 100/3** — 1.63% error

### [PARAMETRIC] (downgraded from [THEOREM]/[DERIVED] today)

- **sin²θ_W = 3/13** — 3.5% error, 2/9 competitor fits better
- **sin²θ_13 = 1/52** — 12.6% error, essentially a mis-prediction
- **α_s(M_Z) = 7/59** — 0.6% error, 2/17 competitor fits better

### [OPEN] — the real research program

| Item | What it is | Status |
|---|---|---|
| **a_phys** | Lattice-to-physical-length conversion (1 lattice unit = ? meters). Either derivable from {D=3, ternary, 26-Moore, determinism, discrete time} or empirical. | **RESOLVED 2026-04-19** — Mechanisms α/β/γ all closed as derivation candidates; calibration `a_phys ≡ ℓ_P` declared in `docs/SPEC_FTD.md` (LEDGER FTD-0030, FTD-0041). Every dimensional prediction is now explicitly conditional on this calibration. |
| **Mechanism B for g_c** | Lattice-to-continuum matching via quantum path integral + Wilson coefficients. Months of physics work. | The only remaining route after A and C were ruled out |
| **Chowla-Selberg extension** | Does CM uniqueness extend rigorously to class-number ≥ 2? Pattern strongly suggests yes; proof needed. | Small / tractable (~2 days) |
| **Master quadratic paper** | Minimum-claim publication draft targeting arXiv + Comm. Number Theory and Physics. | Pending — see §3 Option 1 |
| **a_phys derivation attempt** | Can lattice invariants force a specific value? | **CLOSED 2026-04-19** — Mechanism γ attempt run (`docs/theory/10_eft_program/DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md`); negative result; recommendation accepted to declare `a_phys ≡ ℓ_P`. |

---

## 3 · Priority queue for next session

Ordered by value × tractability:

### Option 1 — Write the paper (highest value)

Target: a 15-20 page mathematical-physics note titled something like
**"A unique CM-curve polynomial identity producing the fine-structure
constant and QCD color number as roots."**

Core claims, all now defensible:

- Polynomial identity x² − 16G*²x + 16G*³ = 0 with G* = Γ(1/4)/Γ(3/4),
  coefficient 16 = |Aut(E)|² for E: y² = x³ − x
- Roots: x+ = 137.0362, x− = 3.0240
- Uniqueness among class-number-1 CM curves (verified numerically)
- Dual match: x+ → 1/α (1.26 ppm), x− → N_c (0.80%)
- 60k-polynomial rigidity scan showing master quadratic is the tightest
- Physical identification flagged as [SELECTION], not [DERIVATION]
- Finitary lattice-physics scaffold (undefined-boundary ontology)
- The engine as verification artifact (Phase G, R² = 1.0000)

Submission targets: arXiv → *Communications in Number Theory and
Physics*, *Letters in Mathematical Physics*, or *Experimental Mathematics*.

**Estimate:** 2-3 days of focused writing. I can draft in LaTeX directly
targeting arXiv format.

### Option 2 — Formalize the a_phys question

One-page scoping doc at
`docs/theory/10_eft_program/OPEN_A_PHYS_DERIVATION.md` stating:

- The framework must supply a_phys (the 1-lattice-unit = ? physical
  length conversion) either from axioms or as empirical input
- Under the parameter-free commitment, a_phys should come from the
  axioms {D=3, ternary, 26-Moore, determinism, discrete time}
- If derivable, an attempt follows; if not, explicitly admit empirical
  status

**Estimate:** 1-2 hours.

### Option 3 — Chowla-Selberg extension to h ≥ 2

Fix the Chowla-Selberg formula normalization I botched in the first
attempt, then verify the CM uniqueness holds across class-number 2 and
3 fields (~40 additional discriminants). If it holds cleanly, the
uniqueness theorem is much stronger.

**Estimate:** 1-2 days. Scope-well-bounded.

### Option 4 — Editorial pass on theory docs (RESTATE queue)

Per `AUDIT_INFINITY_REFRAME.md` §7 priorities 1-4: restate
`FOUND_AXIOM_ZERO.md`, `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` §VI,
and the stylistic "in the limit" language across `/03_derivations/`.

**Estimate:** 2-3 days of mostly mechanical work.

### Option 5 — Attempt Mechanism B (lattice-to-continuum matching)

Promote the classical FTD action to a quantum partition function,
compute 1-loop Wilson coefficient, see if the bare lattice g_c matches
to continuum α at any physical scale. This is the only remaining route
for first-principles g_c.

**Estimate:** weeks to months. Significant physics project.

**Recommended order for next session:** Option 1 (paper draft) first,
because it crystallizes what's been proven today and forces the final
shape of the claims. Then Option 2 (a_phys scoping) as a natural
follow-up. Options 3-5 as time permits or as separate projects.

---

## 4 · What you can actually claim to a physicist tomorrow

In order from most to least defensible:

1. **"I identified a specific polynomial, derived from the arithmetic
   of the CM elliptic curve y² = x³ − x and the Watson-style period
   integral G*²/(2π), whose two roots match 1/α and N_c simultaneously
   to permille-level precision. Among all class-number-1 CM curves,
   this polynomial is uniquely structured to produce this dual match."**
   This is the paper.

2. **"The corresponding lattice simulator reproduces the lattice Poisson
   Green's function as its Coulomb interaction, exactly, with no fine-
   structure constant in the coupling-free limit."** Phase G finding.

3. **"The physical identification of the polynomial's roots with α and
   N_c is a structurally-motivated conjecture — not a derivation — and
   the only open route to upgrade it to a theorem is via lattice-to-
   continuum matching in a full quantum path integral."** Honest state
   of the art.

4. **"The framework commits to an undefined-boundary lattice ontology
   rather than a completed-infinity ℤ³ — this is philosophically
   cleaner and aligns with constructive mathematics."** Optional; only
   bring up if the reviewer asks.

What NOT to claim:

- "I derive α from first principles to 0.001 ppt" — retracted today
- "23 Standard-Model constants derived from 5 axioms" — 7 of those
  were demoted today
- "FTD reproduces QED in the L → ∞ limit" — never well-posed
- "The master quadratic is the thermodynamic limit of FTD" — narrative,
  not proof

---

## 5 · Commits made today (in order)

```
ce71bc9  audit: narrow Phase-F alpha plateau claim from 3.6x to 1.8-3.6x alpha_ref
f2e3437  Phase G + H: resolve Phase-F alpha plateau as geometric Coulomb, verify coupling scaling
9b54253  Phase I: audit the master quadratic, downgrade [THEOREM] -> [STRONGLY MOTIVATED CONJECTURE]
e431469  Phase I follow-through: three sub-audits (CM uniqueness, gap-equation L->inf, g_c scoping)
5b64256  Option 3: extend CM-curve scan to all class-number-1 fields
fdfbaf2  Option 2: Wilson-loop topology benchmark rules out Mechanism A for g_c
8e34276  Option 4: audit other [THEOREM]/[DERIVED] rational-integer claims
2f2ddfe  Phase J: explicit partition function on L=2 - first-principles derivation attempt
4c2d6e9  Foundational reframe: completed-infinity -> undefined-boundary ontology
(this)   Session wrap-up: update documentation + WHERE_WE_LEFT_OFF
```

Each commit self-describes its scope in the message. `git log
--oneline 2f70d32..HEAD` reproduces this list.

---

## 6 · Stale items worth checking before resuming

These weren't fully swept today; flag them for next session:

- **manuscript_v2/** — 83-chapter Quarto book. Has [DERIVED] tags on
  sin²θ_W, m_e, PMNS angles that need downgrade. Editorial pass needed.
- **dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex** — was updated
  with Phase G/H resolution but doesn't yet reflect the foundational
  reframe. If this paper is going out, §1 needs a "framework
  commitments" paragraph citing AUDIT_INFINITY_REFRAME.md.
- **Engine code** — unchanged today except for `coulomb_charge_coupling`
  toggle (Phase H). Still compiles, still runs, 267/267 Python tests
  pass. But `ontic.h` comment "[THEOREM]" on g_c = √α is now stale;
  should read "[SELECTION: defined by target-matching, not derived]".
- **SPEC_FTD.md** — top-level spec. Not reviewed for reframe language
  today. Likely has "in the continuum limit" / "thermodynamic limit"
  stylistically that should be restated.

---

## 7 · Sanity-check commands to run at resume

```bash
# 1. Full test suite (expected: no regressions)
cd engine/build && ctest --output-on-failure
python scripts/proofs/proof_master_verification.py  # 54/54

# 2. Reproduce today's key findings
python scripts/proofs/fit_geometric_coulomb.py           # Phase G R²=1.0000
python scripts/proofs/audit_master_quadratic_rigidity.py  # 60k scan
python scripts/proofs/scan_cm_curves.py                  # CM uniqueness
python scripts/proofs/partition_function_L2.py           # Phase J ultralocality
PYTHONIOENCODING=utf-8 python scripts/proofs/audit_gap_equation_convergence.py  # gap-eq refutation

# 3. Verify Phase H scaling on WSL2 GPU (if CUDA available)
wsl.exe -d Ubuntu-22.04 -- bash -c "cd /mnt/c/Users/cpaci/Desktop/ftd/engine/build_wsl && ./test_phase_h_coupling"

# 4. Git state
git log --oneline 2f70d32..HEAD  # 10 commits of April 19
git status  # should be clean
```

If any of these fails, the reframe or one of the audits has drifted
and needs attention before new work.

---

## 8 · One-paragraph resume prompt

**If you want to drop this into a fresh session:**

> I'm resuming work on the FTD project. Read `docs/WHERE_WE_LEFT_OFF.md`
> first — it has the full state of the April 19 audit cycle and the
> priority queue. We landed 10 commits that cut the project's
> overclaims and replaced them with a tighter, defensible core: the
> master quadratic as an algebraic identity with CM-curve uniqueness,
> the engine as a geometric-Coulomb simulator, and an undefined-boundary
> ontology replacing completed-infinity ℤ³. The master quadratic's α
> identification is [STRONGLY MOTIVATED CONJECTURE], not [DERIVATION].
> The highest-value next task is Option 1: draft the
> mathematical-physics paper that crystallizes what survived. Second
> priority is the a_phys scoping doc. Don't claim anything that isn't
> in §4 of WHERE_WE_LEFT_OFF.md without auditing it first.

That's enough to get going again in a fresh context.

---

## 9 · Personal note (for Chris)

You did the right thing today. A lot of theoretical physics projects
die because the authors protect their overclaims against scrutiny. You
spent a ten-hour session asking for your own work to be cut harder
than any reviewer would. The project is smaller tonight, but it's
defensible, which is a place most never reach. Get some sleep. The
master quadratic isn't going anywhere — it's a real mathematical
object that will still be there tomorrow, and now it has the right
epistemic tag on it.

— Claude, 2026-04-19 evening
