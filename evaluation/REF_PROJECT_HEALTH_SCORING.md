# FTD Project Health Scoring Methodology

**Purpose:** Provide a stable, repeatable method for scoring overall project health across engineering, documentation, verification, research rigor, and publication readiness.  
**Audience:** Maintainers, contributors, and future audit passes.  
**Applies to:** Repo-wide health reviews, cleanup rounds, release-readiness reviews, and recurring progress snapshots.

---

## 1. Scoring Philosophy

This scorecard is designed to answer three questions consistently:

1. How healthy is the project right now?
2. What should be prioritized next?
3. Did the last cleanup or development cycle actually improve the project?

The scorecard is intentionally **multi-dimensional**. FTD is not only a codebase; it is also a theory corpus, verification stack, publication pipeline, and assessment archive. A single “grade” hides too much.

Use this methodology for every future scorecard so trend lines stay meaningful.

---

## 2. Score Codes

Each health area has a stable score code:

| Code | Area | Weight |
|------|------|--------|
| `H-ENG` | Engineering foundation and runtime architecture | 20 |
| `H-VER` | Verification, tests, and reproducibility discipline | 15 |
| `H-DOC` | Documentation integrity and synchronization | 15 |
| `H-ONB` | Onboarding and navigability | 10 |
| `H-RIG` | Research rigor and epistemic discipline | 20 |
| `H-PUB` | Publication quality and accessibility readiness | 10 |
| `H-GOV` | Governance, maintenance habits, and prioritization | 10 |

**Total weight:** 100

---

## 3. Raw Scoring Scale

Each area is scored on a **0.0 to 5.0** scale in **0.5-point increments**.

| Raw Score | Meaning |
|-----------|---------|
| `0.0` | Non-functional or absent |
| `0.5` | Severely broken; almost no usable structure |
| `1.0` | Critical risk; major gaps dominate |
| `1.5` | Very weak; partially usable only with heavy caution |
| `2.0` | Weak; meaningful structure exists but serious blockers remain |
| `2.5` | Fragile-but-real; clearly alive, but high-risk or inconsistent |
| `3.0` | Workable; contributors can operate productively with caution |
| `3.5` | Solid; good structure with visible debt |
| `4.0` | Strong; reliable and well maintained |
| `4.5` | Excellent; high trust, low drift, strong discipline |
| `5.0` | Exceptional; reference-quality maturity |

---

## 4. Weighted Score Calculation

For each area:

```text
weighted_points = (raw_score / 5.0) * weight
```

Overall project score:

```text
overall_score = sum(all weighted_points)
```

This yields a final score from `0` to `100`.

---

## 5. Overall Grade Bands

| Overall Score | Grade | Interpretation |
|---------------|-------|----------------|
| `90-100` | `A` | Excellent |
| `80-89` | `A-` | Strong |
| `70-79` | `B+` | Good |
| `60-69` | `B-` | Workable but risk-heavy |
| `50-59` | `C` | Fragile |
| `40-49` | `D` | High risk |
| `<40` | `F` | Critical / unstable |

---

## 6. Confidence Codes

Every score must include a confidence code so readers know how much fresh evidence backs it.

| Confidence | Meaning |
|------------|---------|
| `A` | Directly validated from current repo state and runtime checks where appropriate |
| `B` | Based on current repo inspection and current documents, but without fresh runtime execution |
| `C` | Based largely on indirect or older reports; useful but less reliable |

If a future scorecard runs builds/tests and validates outputs, the confidence should rise where warranted.

---

## 7. Priority Rules

Assign priority from both **score weakness** and **known blocker severity**.

### Priority thresholds by raw score

| Raw Score | Default Priority |
|-----------|------------------|
| `< 2.0` | `P0` if blocking, otherwise `P1` |
| `2.0 - 2.5` | `P1` |
| `3.0` | `P2` |
| `3.5+` | `P3` unless tied to a critical blocker |

### Override rules

Escalate to `P0` or `P1` if any of these are true:

- The weakness blocks release credibility or contributor trust.
- The weakness is already a `[CRITICAL]` row in `docs/theory/07_assessment/AUDIT_WEAKNESSES_MASTER.md` or has an active `[OPEN]` entry in `docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md`.
- The weakness causes public docs to mislead readers about current repo truth.
- The weakness creates repeated maintenance churn across subsystems.

---

## 8. Area-Specific Rubrics

### `H-ENG` Engineering Foundation

Evaluate:

- Runtime architecture clarity
- separation of concerns
- build surfaces and entry points
- implementation maturity
- hotspot risk concentration

Score anchors:

- `5.0`: well-factored, low-drift, strong architecture, healthy execution paths
- `3.0`: architecture is usable and real, but hotspots and drift remain
- `1.0`: core runtime unclear, brittle, or effectively unmaintainable

### `H-VER` Verification And Reproducibility

Evaluate:

- tests and runner coverage
- clarity of verification layers
- reproducibility of claims/checks
- CI-like discipline or repeatable commands
- mismatch between reported and actual verification surfaces

### `H-DOC` Documentation Integrity

Evaluate:

- currentness of metadata
- consistency between maps/specs/live tree
- broken/stale links
- authority boundaries
- catalog accuracy

### `H-ONB` Onboarding And Navigability

Evaluate:

- newcomer reading path
- discoverability of authoritative docs
- cross-linking quality
- ability to choose a contributor lane without local lore

### `H-RIG` Research Rigor And Epistemic Discipline

Evaluate:

- honesty about theorem vs. selection vs. conjecture
- audit quality
- dependency on external physics or fitted constraints
- consistency between flagship claims and audit language
- resolution of critical scientific objections

### `H-PUB` Publication And Accessibility Readiness

Evaluate:

- accessibility quality
- publication build clarity
- visual/document QA
- citations, figure references, and output consistency

### `H-GOV` Governance And Maintenance

Evaluate:

- issue tracking quality
- prioritization clarity
- maintenance rules
- drift detection and remediation habits
- whether new work updates catalogs and status docs consistently

---

## 9. Required Evidence For Each Scorecard

Every scorecard must include:

1. Raw score
2. Weighted points
3. Confidence code
4. Short justification
5. Primary evidence sources
6. Priority assignment

Optional but recommended:

- trend vs. previous scorecard
- open-item IDs from `docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md`
- explicit "raise this score by doing X" note

---

## 10. Running Scorecard Format

Use this exact table shape for consistency:

| Code | Area | Weight | Raw | Weighted | Confidence | Priority | Summary |
|------|------|--------|-----|----------|------------|----------|---------|

After the table, include:

- overall weighted score
- overall grade
- top 3 priorities
- evidence basis
- scoring date

---

## 11. Change Control

When issuing a new scorecard:

1. Reuse these same score codes and weights unless there is a compelling governance reason to change them.
2. If weights or bands ever change, document the change in the scorecard itself and preserve comparability notes.
3. Prefer updating the latest scorecard document over scattering health judgments across multiple files.
4. Link each new scorecard to:
   - [AUDIT_WEAKNESSES_MASTER.md](AUDIT_WEAKNESSES_MASTER.md)
   - [../docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md](../docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md)
   - [../docs/theory/07_assessment/core_ledgers/LEDGER.md](../docs/theory/07_assessment/core_ledgers/LEDGER.md)
   - [../docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md](../docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md)
   - [../docs/audits/AUDIT_DOCUMENT_CLEANUP_LEDGER.md](../docs/audits/AUDIT_DOCUMENT_CLEANUP_LEDGER.md)
   - [../docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md](../docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md)

---

## 12. Interpretation Rule

This methodology scores **project health**, not “truth of the theory.”

A project can score:

- high on engineering health and low on research rigor
- high on epistemic honesty and low on publication readiness
- high on architecture and low on contributor onboarding

That distinction is intentional and should be preserved in every future review.
