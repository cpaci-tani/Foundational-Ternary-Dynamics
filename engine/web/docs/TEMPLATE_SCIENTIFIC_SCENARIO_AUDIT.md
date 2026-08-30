# Template — Scientific Scenario Qualification Record

**Record status:** **[OPEN]**

**Scenario ID:** `replace-me`

**Contract version:** `1`

**Audit opened:** `YYYY-MM-DD`

**Audit disposition:** `not-started`

This record is evidence bookkeeping, not proof of a physical identification.
Complete the gates in order. Do not open the next scenario record until this
record has a reproducible disposition and every representation agrees.

## 1. Frozen identity

| Field | Recorded value |
|---|---|
| Display name | |
| Scale | |
| Model relation | |
| Experimental roles | |
| Record lifecycle | open |
| Registry source | |
| Initialization owner | |
| Supported backends | |
| Resolution domain | |
| Boundary domain | |
| Historical aliases | |

## 2. Scientific contract

### Mathematical model

- Finite state space:
- Tick/update schedule:
- Initial conditions:
- Boundary conditions:
- Enabled terms:
- Native units:
- Physical calibration, if any:

### Provenance

| Input or parameter | Value/domain | Provenance class | Authoritative source |
|---|---|---|---|
| | | | |

Allowed provenance classes are `axiom`, `theorem`, `selection`, `conjecture`,
`imposed`, `emergent`, `open`, `parametric`, `external-reference`, and
`closed-negative`. A standard-physics formula populated with FTD values is
`parametric`; it is not a substrate derivation.

### Claims

| Claim ID | Exact wording and scope | Epistemic tag | Supporting gates/evidence | Limitations | Disposition |
|---|---|---|---|---|---|
| | | | | | |

| Prohibited claim ID | Exact wording and scope | Reason/evidence |
|---|---|---|
| | | |
- Known limitations:
- Cross-scale inputs:
- Required independent observable:

## 3. Preregistered protocol

### Null and candidate

- Null hypothesis:
- Candidate claim:
- Primary observables:
- Independent observable:
- Acceptance thresholds:
- Falsification thresholds:
- Uncertainty treatment:
- Seeds/ensembles:

### Controls

- Null control:
- Mirror/conjugate control:
- Perturbation control:
- Resolution control:
- Boundary control:
- Backend-parity control:

Acceptance criteria must be frozen before candidate measurements are run.
Post-hoc tuning opens a new versioned trial; it does not validate this one.

## 4. Gate ledger

Use only `not-started`, `in-progress`, `pass`, `fail`, or `blocked`. A later
gate cannot compensate for an earlier failure.

| Gate | Status | Evidence | Blocking findings |
|---|---|---|---|
| 1. Static trace | not-started | | |
| 2. Mathematical well-posedness | not-started | | |
| 3. Numerical validity | not-started | | |
| 4. Scientific validity | not-started | | |
| 5. Scale appropriateness | not-started | | |
| 6. UI and interpretive truth | not-started | | |
| 7. Performance and operational safety | not-started | | |

## 5. Numerical and backend evidence

| Run ID | Commit | Backend | Lattice/domain | Boundary | Seed | Result bundle | Verdict |
|---|---|---|---|---|---|---|---|
| | | | | | | | |

Record determinism, finite-size behavior, convergence, floating-point
sensitivity, lifecycle/race findings, and backend tolerances here. Raw result
bundles must remain reproducible and must identify calibration inputs.

## 6. UI and performance evidence

| Surface | Expected wording/state | Evidence | Verdict |
|---|---|---|---|
| Scenario picker | | | |
| Context description | | | |
| Status/provenance badge | | | |
| Applicable overlays | | | |
| Prohibited overlays | | | |
| Export metadata | | | |
| Help/knowledge base | | | |

| Performance condition | Budget | Measurement | Verdict |
|---|---|---|---|
| Foreground frame time | 16.67 ms reference target | | |
| Frame pacing | no sustained long-frame cluster | | |
| Hidden/collapsed panels | no polling, layout, chart draw, or field extraction | | |
| Load/switch lifecycle | no stale callbacks, duplicate listeners, or leaked workers | | |
| Scientific cadence | unchanged by presentation decimation | | |

Reference hardware, browser, viewport, active panels, lattice size, sampling
duration, warm-up, and throttling state are required with every measurement.

## 7. Final disposition

Choose exactly one:

- `qualified-within-contract`
- `qualified-parametric`
- `pedagogical-only`
- `candidate-open`
- `blocked`
- `closed-negative`
- `reference-only`

**Disposition:** `not-set`

**Reason:**

**Evidence bundle:**

**Required synchronization:** manifest, source, tests, scenario picker,
description, badges, overlays, export metadata, documentation, and atlas.

**Next scenario may open:** `no`
