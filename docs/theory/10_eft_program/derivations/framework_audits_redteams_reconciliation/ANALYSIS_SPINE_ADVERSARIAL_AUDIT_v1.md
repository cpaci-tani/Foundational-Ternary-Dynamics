# FTD-0785 — Algebraic Spine Adversarial Audit v1

**Status:** `[AUDIT — REMEDIATED]` + `[MEASURED CONTROL — ARITHMETIC CLEAN]`
**Verdict:** `SPINE_ARITHMETIC_CLEAN_CLASSIFICATION_CORRECTED_6_PLUS_3`
**Target:** `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` and
`docs/theory/07_assessment/audits/AUDIT_W_CARRIER_NARROWING.md` (FTD-0314)
**Method:** two independent auditors, each instructed to **refute by
default** and to compute rather than recall; every identity recomputed at
50–150 digits; Watson integral re-derived by independent quadrature;
citations checked against the literature; artifact paths checked on disk
**Production impact:** none — documentation corrections only

## 1. Why this audit existed

Five of five original constructions attempted earlier in this session failed
adversarial audit (§26's gravity mechanism, §23's uniqueness claim,
FTD-0778's first verdict, §31.4's numbers, the capture band). Only theorems
and negative results survived. That left the algebraic spine as the
program's principal surviving asset — and it was the one load-bearing
artifact that had never faced a refute-by-default pass.

## 2. Headline

**The arithmetic is clean.** Independently recomputed at 50–150 digits: the
`G*` reflection/lemniscatic/AGM identities; `x_+ = 137.03617145815548...`,
`x_- = 3.02396391633902...`; Vieta and discriminant forms; Watson
`W_3 = G*^2/(2 pi) = Gamma(1/4)^4/(4 pi^3) = 1.39320392968...` (re-derived
by numerical quadrature of the reduced BCC integral, agreement `2.6e-26`);
the four `L(chi_{-4})` special values; the harmonic-tower invariant
`1/y_+ + 1/y_- = 1` for `k = 3..7`; `alpha_tree = 1/x_+` to `1e-63`;
`f(D) = 2^D (D-1)!` equal to 16 only at `D = 3`; the FQCR `O(1/N^2)`
constant. `|Aut(E)| = 4` at `j = 1728` confirmed. The `x_+` deviation from
CODATA `1/alpha = 137.035999177(21)` is `+1.2572e-6`, matching the spine's
stated 1.26 ppm.

**Every failure was classificatory.** Corrected in place:

| # | Finding | Fix applied |
|---|---|---|
| 1 | Theorem 6's "Proof" is a regression fit against simulator output (`R^2 = 1.0000`, 0.07% median residual at `L = 384`); the statement is engine-referential and cannot be posed without the simulator; a 0.07% residual contradicts its own "exact identity, zero free parameters" framing | Demoted to `[NUMERICAL FACT — VALIDATED FIT]`, split into the (unexhibited) exact identity and the (established) measurement. **Canonical count changes from 7+2 to 6+3** — propagated to all eight count statements |
| 2 | An **untagged electron-mass identification** inside a document whose §0 contract is "[THEOREM]-only form, with no physics interpretation" and whose same section closes "Nothing about physics." Eight digits displayed; the **68 ppm** deviation from the measured value never stated; the "lattice units" hedge makes it deniable in the flattering direction | Excised from §1; restated in §11 tagged `[NUMERICAL FACT]` with the deviation and the missing look-elsewhere control stated |
| 3 | The load-bearing transcendence citation names **the wrong brother and a title that does not exist** ("D. V. Chudnovsky, *…exponential and hypergeometric functions*") | Corrected to G. V. Chudnovsky, "…exponential and **elliptic** functions," Dokl. Akad. Nauk Ukrain. SSR Ser. A (1976) no. 8, 698–701. The *substance* was fine: the cited theorem really is the independence of `pi` and `Gamma(1/4)`, and the spine's conditional claims do follow |
| 4 | §9 asserts Theorem 8 "**proves** `G*` is the unique named-constant generator" — Theorem 8's own scope block says the invariant holds for *any* family `{x^2 - bx + c : c = G* b}` and leaves multiplier rigidity `[OPEN]` | Rewritten to state the non-uniqueness explicitly |
| 5 | The maximality disclaimer is correct but its **reason is mathematically false**: `Q(Gamma(1/4))` cannot witness non-maximality because it does not contain `Q(G*)` at all (that would make `pi` algebraic over `Q(Gamma(1/4))`, contradicting the same Chudnovsky result) — the fields are *incomparable* | Replaced with the trivial correct witness `Q(G*, sqrt 2)`, and the note that this makes maximality a non-question rather than an interesting `[OPEN]` |
| 6 | Schneider 1941 gives transcendence of `B(1/2,1/4) = sqrt(pi) G*`, **not** of `G*`; only Chudnovsky does. Residual of a fix already registered as FTD-0318 | Attribution corrected in Theorem 8's dependencies |
| 7 | §14 reinstates the Chowla–Selberg misattribution that §1 corrects; §13's Theorem 7 row carries the pre-FTD-0350 status; §13's Theorem 3 row says "9 discriminants" where §3 says 63; two artifact paths wrong on disk | All corrected |

## 3. Two findings left OPEN — not remediated

**(a) FTD-0314's C2 depends on an open conjecture.** C2 argues that a second
Watson self-energy lands in a different CM field and is "**algebraically
independent of `Gamma(1/4)`**." Joint algebraic independence of
`Gamma(1/3)` and `Gamma(1/4)` is **Waldschmidt Conjecture 5.23 — open**.
Chudnovsky gives `{pi, Gamma(1/4)}` and `{pi, Gamma(1/3)}` *separately*;
Nesterenko 1996 adds only same-field exponentials. The repo's own
`09_mathematical/number_theory/ANALYSIS_E1_E2_TRANSCENDENCE_SOTA.md`
(FTD-0377, dated *after* FTD-0314) states this explicitly. So C2's
cross-CM-field exclusion is conditional on **Chudnovsky plus an open
conjecture**, and FTD-0314's headline `[THEOREM]` over-consolidates. The
LEDGER row is more careful than the audit document — it drops the
independence phrase — which is itself a drift worth noting.

Recommended retag for FTD-0314, not yet applied because it is that audit's
own claim to revise: `[CONDITIONAL THEOREM — Chudnovsky]` (narrowing core +
C1 + C3) + `[CONDITIONAL — open Waldschmidt Conj. 5.23]` (C2's cross-field
exclusion) + `[SELECTION]` (exhaustiveness of the carrier taxonomy) +
`[OPEN]` (the loophole). Note also that FTD-0314 §0/§5's "**proves** no
cheaper native object can be it" re-asserts in theorem voice precisely what
FTD-0242 registers as an unproven universal negative.

Two smaller items in the same document: its verification script prints
"transcendental over `Q(G*)`" for the surd, which is **false as written** —
the surd is *algebraic of degree 2* over `Q(G*)` (the document body gets
this right); and the "~85%" credence attached to the verdict has no
computation, reference class, or calibration anywhere, unlike FTD-0319 where
an uncomputed Bayes figure was explicitly killed. It should read
"[subjective credence, not computed]" or be dropped.

**(b) The FTD-0319 look-elsewhere audit did not complete.** The third
auditor — assigned the ~2.65-million-polynomial uniqueness scan behind the
"sole dual-matcher" claim — terminated early on a session limit. Its last
partial observation, **unverified and recorded as a lead only**: the
historical in-house criterion may have accepted `x_-` within 1% of *any*
small integer 1–10, which if true would make the "dual" leg nearly free and
badly weaken the improbability argument. Independently, auditor 2 noted that
the second leg's target (`x_- ↔ N_c`) is an identification the program
itself **retired** in v1.4 §5, so the live content of "dual-match" may
already be a single-root ppm match. **This is the highest-value unfinished
audit in the program** and should be rerun.

## 4. Honest characterization of what survives

The six theorem-grade results are real but should be described without
inflation: Theorem 1 is elementary Γ-reflection algebra; Theorem 2 is the
quadratic formula; Theorem 3's core is a three-case pigeonhole over
`|mu_K| ∈ {2,4,6}` versus `|disc| >= 3` (true — an independent scan over
squarefree `d <= 200` reproduces `d = 1` as the sole solution — but shallow,
and nothing explains why that invariant *pair* is the right comparison);
Theorem 5 (Watson) is the one with genuine classical depth; Theorem 8 is a
Vieta consequence of `c = G* b`; Theorem 9 is a short conditional exercise.
Theorem 4's "three independent arithmetic routes" to `|Aut(E)| = 4` are the
same fact transported through standard equivalences — though the spine is
honest that `16 = |Aut(E)|^2` is `[SELECTION — declared]`, not forced.

That is a real but modest core: **standard or elementary facts about
`Gamma(1/4)`, one classical Watson integral, and one conditional
transcendence corollary.** It is not diminished by saying so, and the spine
now says so.

## 5. Consequence for the session's other work

The same no-promotion check applied to FTD-0784, registered hours earlier,
caught it presenting a **transport** of FTD-0314 C3's non-squareness
argument as an independent theorem. FTD-0784 was demoted the same day; its
surviving new content is the `[EXACT]` moment expressibility of the
master-quadratic invariants and the change of domain from curve periods to
measured observables. That document also records a definitional defect it
found in C3: the period field `F = Qbar(pi, Gamma(1/4))` is stated
inconsistently, since `Omega` is asserted to lie in `F` while
`sqrt(pi) ∉ F`.

**Standing lesson, reconfirmed for the fifth time this session:** in this
program the arithmetic survives adversarial audit and the classification
does not. Every artifact should be audited for *tag* before it is audited
for *content*.
