# SPEC — LOCK-STD v1: the mandatory pre-registration lock standard

**Tag:** [SPEC / DISCIPLINE] — adopted Arc 1 of the Consumption Program (FTD-0383 AM-4; owner-ratified with the roadmap approval of 2026-07-12). **Introduces no claim; promotes nothing.**
**Scope:** every Consumption Program pre-registration (all fronts), every reopened line, and — prospectively — every FTD pre-registration. Pre-LOCK-STD locks are grandfathered with Arc-2 disposition rows (census dispositions register).
**Distilled from the defect record:** FTD-0143 (B/C outcome overlap; unlocked tie-break; vacuous passes; prose/normative value conflict) · δ-IND A0 findings M-2/M-3/M-4 (quantifier coverage; execution-time definitions; transcribing verifier) · the vertex v1→v1.1 arc (wrong-operator harness; pre-measurement gate-sign amendment `280e5d86`) · FTD-0208-v2 (same-minute prereg+result, no tag) · the FTD-0384 census (counterfeit and unanchored locks).

## The eleven requirements

Every lock MUST contain, before any computation or proof-attempt output is observed:

1. **Partition proof.** The outcome map is mutually exclusive and jointly exhaustive, demonstrated by a table showing no admissible dataset fires two outcomes. *(Kills the FTD-0143 B/C overlap class.)*
2. **Frozen tie-breaks.** Every ranking/threshold criterion carries an explicit tie-break; "top-k" without one is an invalid lock.
3. **Precedence rule.** A locked precedence order decides criterion conflicts at adjudication; criterion text outranks rationale prose; prose/normative value conflicts adjudicate to the normative list — declared in advance, not discovered (the FTD-0143 disclosure-iii pattern, made mandatory).
4. **Effective-protocol declaration + executable correctness gate.** The lock names the operator/convention *as implemented*, backed by an executable identity/correctness gate that must pass before any verdict is credited (the Damerell §3.3 d=−4 gate is the model; the vertex v1 harness executed a provably-wrong operator for lack of one). For engine campaigns: the post-toggle-audit **effective** toggle set, platform, and `engine/build_wsl` provenance are part of the lock; a run whose effective configuration differs from its lock is **INVALID — not negative, not positive.** Pre-measurement amendments are legal only if committed before any dynamics output is observed (`280e5d86` precedent: exit-1, no output seen).
5. **Vacuity firewall.** Every PASS criterion carries a pre-declared witness that it *can* fail on admissible data; a criterion that passes on the whole space is reported as vacuous and contributes zero evidential weight. Program-wide corollary (Chair-1 guard, binding): an outcome in which a candidate closure is shown to contain a dense/universal class of computable reals is **definition failure (IMPROPER)** — never evidence about δ, α, or MC-T4.3, in either direction; IMPROPER precedes REFUTED in every δ-adjacent map, and any δ-construction exhibit must pass a frozen non-universality test before REFUTED may be claimed.
6. **Quantifier-coverage audit.** Every citation supporting a for-all claim is audited against the full admissible class at lock time (the M-2 lesson: m=1 citations cannot carry an m≥2 class).
7. **No execution-time definitions.** Every set, branch, and name used in adjudication is defined in the lock; anything minted during execution is post-hoc and voids the affected clause (M-3).
8. **Verifier recomputes.** Adjudication scripts recompute claimed identities; they never bookkeep author-supplied values (M-4).
9. **Anchor-before-run.** The git tag is cut before execution; same-minute prereg/result mtimes disqualify (FTD-0208-v2); a result doc citing a tag that does not resolve in git is INVALID on its face (the FTD-0217/0218 class). A **content SHA256 of the prereg is recorded in the lock** so that a skipped tag-cut remains recoverable by archaeology (the FTD-0384 §2.2 recovery precedent) — but recovery yields `anchored-late`, never an original lock.
10. **Scheduled execution window + auto-booked debt.** The lock declares an execution window and executor. A lock past window without a run, or a run past window without a verdict, auto-books an open F10 row (tracker/census). The census (`tools/preregister_census.py`) is the standing arc gate: **RED blocks lock-cutting.**
11. **Reconciliation actions are themselves LEDGER-booked** with before/after states, by the banner-plus-preserved-kernel method (FTD-0042/0232) — never deletion — so honest demotion remains distinguishable from history-editing (Chair-6 addition).

## Meta-F10 (program level)

No arc rollover without a booked arc verdict: every arc's metrics are declared before the arc runs; an arc ending without its metrics evaluated is booked INCOMPLETE and blocks the next arc's lock-cutting. The program-level review at Arc-6 close is itself pre-registered (`PREREG_PROGRAM_REVIEW_ARC6_v1.md`) with the frozen outcome map CONTINUE / RE-SCOPE / ARCHIVE-AS-MAPPED-BOUNDARY.

*Zero promotions. The standard constrains process, not verdicts; a lock that satisfies all eleven points can still close negative — that is the point.*
