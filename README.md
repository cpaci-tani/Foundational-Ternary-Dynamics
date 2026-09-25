# Foundational Ternary Dynamics

Foundational Ternary Dynamics (FTD) is a research program in the philosophy of
mathematics, discrete dynamics, and mathematical physics. It asks what follows
from explicitly chosen finite-alphabet, locally specified dynamics, what is
needed to interpret those dynamics as physics, and where proposed connections
fail. The active framework is not an established replacement for quantum field
theory or general relativity. A consistent construction, an exact computation,
and an empirical theory are different achievements.

The repository contains the theory record, proof and verification scripts, a
C++ simulation engine, and a browser workspace. These components do not all
implement the same microscopic law. Read the [claim ledger](docs/theory/07_assessment/core_ledgers/LEDGER.md)
before treating any result, plot, or older document as a current claim.

## What rigor means here

FTD keeps five questions separate. Passing one stage does not silently answer
the next.

| Question | What would establish it | What would not |
|---|---|---|
| **Specification**: What is assumed? | A complete signature, finite state types, declared postulates, a selected transition law, and explicit independent inputs. | Calling a choice inevitable because it is simple or suggestive. |
| **Mathematical consequence**: What follows? | A statement with quantified hypotheses and a checkable proof, or a complete finite certificate for its stated domain. | A numerical match, an analogy, a test pass, or an unchecked extrapolation from finite cases. |
| **Representation**: What effective object is obtained? | A defined map from microscopic histories, a validity domain, and an error or stability bound. | Renaming a lattice variable as a field, probability, mass, or metric. |
| **Physical interpretation**: What is measured? | An operational readout, units and calibration, preparation and detector rules, and a non-circular comparison protocol. | Identifying a mathematical parameter with a measured constant by resemblance alone. |
| **Empirical adequacy**: Does it survive tests? | Discriminating predictions, uncertainty and controls, independent data, and reproducible failures as well as successes. | Reproducing values used to choose inputs or substituting model numbers into an adopted formula. |

In the proof-theoretic sense, a theorem is conditional on its axioms and any
external results it invokes. Specifying one model establishes neither that it
is unique nor that nature instantiates it. An exhaustive program can certify a
finite domain if its encoding and enumeration are complete; it is not thereby
a proof over an unbounded domain. A no-go result must name the carrier,
operator class, and hypotheses it excludes. These distinctions are the
project's working standard, not a rhetorical grading system.

FTD adopts a **context-before-content** methodological principle: types,
interpretation rules, and observational contexts must be declared before
their values can be assessed. This is an organizing commitment in the
[type-priority discussion](docs/theory/02_foundations/FOUND_TYPE_PRIORITY_PRINCIPLE.md),
not a theorem about all mathematics or the physical world. Analogies can
motivate definitions; they cannot discharge proof obligations. The project
orders inquiry from ontology to logic to mathematics to physics as a chosen
research method, not as a demonstrated chain of metaphysical necessity.

## The active formal model

The [v3 constitution](docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V3_STRICT_DISCRETE_COMMON_ACTION.md)
ratifies five postulates and a selected reference transition. Ratification is
an adoption of a framework, not evidence that its proposed ontology is real.

| Postulate | Selected commitment | Not supplied by that commitment |
|---|---|---|
| P1 | An oriented, locally extensible three-dimensional cubical cell complex, with a global chart and undefined outer boundary. | A physical metric or a derivation of three dimensions. |
| P2 | One global ordinal tick, with explicit record births and expiry when it occurs. | Seconds, material clocks, or relativistic proper time. |
| P3 | Finite cell alphabets and a many-to-one site readout `m: A_0 -> {-1, 0, +1}`. | Complete microscopic state from a ternary label, or ontic real-valued fields. |
| P4 | A radius-one Moore causal ceiling per update. | A measured propagation speed or Lorentz symmetry. |
| P5 | One state-complete, homogeneous deterministic update `X[n+1] = Phi(X[n])`, with an explicit schedule and a named non-injective expiry. | The unique law forced by P1-P4 or any particular physical sector. |

The finite carrier and reference `Phi` are **selected**. Their exact
specification and scoped formal gates are recorded in the constitution and
its linked certificates. A law with several local transaction cases is still
one law only when those cases belong to the same state-complete update;
independent effective engines do not become one microscopic theory by sharing
a dashboard.

A small example shows the intended precision. **Given** a selected payload
with one blank state and each of four phases paired with either of two
polarities, the required alphabet has `1 + 4 * 2 = 9` labels. Because
`3 < 9 <= 3^2`, two ternary slots are the minimal encoding of that payload.
The count is exact under the stated payload assumption. It does not prove
that physical matter has this payload or that the payload was forced by
earlier postulates.

## The recovery obligation

The primitive v3 tuple contains the cell complex, ordinal order, finite
alphabets, ternary readout, and `Phi`. Continuous fields, actions, probability
rules, particle species, couplings, clocks, and geometry are **targets**, not
additional primitives. For an effective quantity `O_eff`, the
[constitution's closure contract](docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V3_STRICT_DISCRETE_COMMON_ACTION.md)
requires a map of the form

```text
B_(R,T): finite histories of Phi in region R over T ticks -> O_eff
```

together with an error or stability statement at the resolution where the
quantity is used. A cross-scale claim must keep one microscopic state owner,
state its block/readout map and domain of validity, and show which structures
are recovered rather than adopted. A continuum expression fitted after the
fact, or an independent simulator producing a similar image, does not meet
that contract.

Probability needs a preparation and detector protocol, an ensemble or
pushforward defined without target outcomes, and bounded frequency claims.
Gravity needs a common source ledger that governs matter, clocks,
trajectories, and light consistently. Correspondence with established physics
must not be built into `Phi` as an answer key. These are proof obligations,
not claims of completed recovery.

## Current boundary

The selected v3 reference law has passed its stated R1-R6 scoped technical
gates, including a controlled recovery result at a restricted transverse-vacuum
scope. Those gates do not establish stable matter, charged electromagnetism,
Born-rule measurement, a common relativistic causal cone, or nonlinear
gravity. The [open-items tracker](docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md)
records live obligations; the [resolved-items tracker](docs/theory/07_assessment/core_ledgers/TRACKER_RESOLVED_ITEMS.md)
preserves closed work without counting it as open.

The [C++ engine](engine/SPEC_ENGINE.md) is a computational probe of a legacy
continuous-flux law with optional phenomenological features. It is **not** an
implementation of the selected v3 `Phi`; the law-equivalence gap is recorded
in the ledger. Its measurements are evidence about that engine, at named
versions, toggles, initial states, and backends. Browser scales include
separate effective simulators and presentation layers. Their operation is not
by itself evidence of v3 emergence. See the
[scale ownership and recovery audit](docs/reference/REF_SCALE_OWNERSHIP_AND_RECOVERY.md).

No result is promoted because a script passes, a graph resembles known
physics, or a standard formula accepts an FTD parameter. Such substitutions
are **parametric insertions** and remain labeled as such in the
[insertion catalog](docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md).

## Claim status and navigation

The [ledger](docs/theory/07_assessment/core_ledgers/LEDGER.md) is the authority
for claim status. On disagreement, use this order: ledger, active v3
constitution, branch constitutions, then other prose. Historical documents
and archives remain available as provenance, not as competing current
authorities. Tags such as `[AXIOM]`, `[THEOREM]`, `[SELECTION]`, `[CONJECTURE]`,
`[PARAMETRIC]`, `[IMPOSED]`, `[OPEN]`, and `[CLOSED NEGATIVE]` state the kind
and scope of a claim; a tag is not itself a proof.

| Need | Start with |
|---|---|
| Current working state | [Dated resume](docs/WHERE_WE_LEFT_OFF.md) and [ledger](docs/theory/07_assessment/core_ledgers/LEDGER.md) |
| Formal postulates and recovery rules | [Active v3 constitution](docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V3_STRICT_DISCRETE_COMMON_ACTION.md) |
| Theory documents and provenance | [Curated theory index](docs/theory/META_INDEX.md) and [structure guide](docs/theory/META_STRUCTURE.md) |
| Open work and closed routes | [Open-items tracker](docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md) and [resolved-items tracker](docs/theory/07_assessment/core_ledgers/TRACKER_RESOLVED_ITEMS.md) |
| Code ownership and build boundaries | [Repository map](REPOSITORY_MAP.md) and [engine specification](engine/SPEC_ENGINE.md) |
| Contribution and review rules | [Contributing guide](CONTRIBUTING.md) |

```text
docs/           constitutions, claim ledgers, theory, indexes, provenance
scripts/        proofs, verification, tests, experiments, research tools
engine/         C++ engine, CUDA/WASM targets, browser workspace and tests
evaluation/     assessments and project-health work
dissemination/  publication and educational material, not status authority
```

## Reproduce the stated scope

Run checks relevant to a change and record the revision, platform,
configuration, inputs, and failures. The commands below check software and
document contracts; none certify a physical theory.

### Python and documentation

```bash
python -m pip install -r requirements.txt -r requirements-dev.txt
python -m pytest scripts/tests/
python scripts/verification/verify_index_links.py --tracked
python scripts/theory/build_open_items_index.py --check
```

### C++ merge gate

```bash
cmake -S engine -B engine/build -DFTD_ENABLE_CUDA=OFF
cmake --build engine/build --target ftd_merge_gate_build --config Release --parallel 24
ctest --test-dir engine/build -L merge_gate -j 24 -C Release --output-on-failure
```

For the full Windows native build, use `engine\build_native.bat`, which pins
the supported MSVC toolset; see the [engine specification](engine/SPEC_ENGINE.md).
GPU measurement campaigns have additional WSL2 requirements in
[`CLAUDE.md`](CLAUDE.md).

### Browser workspace

```bash
npm ci
npm run build
python engine/web/serve.py 8080
```

Open `http://localhost:8080`. Browser tests have their own dependencies and
runner in the [web test package](engine/web/tests/package.json). Keep a
software test result distinct from a measured physical claim, and keep both
distinct from a mathematical proof.

## How to contribute rigorously

For a new result, state its exact hypotheses, quantifiers, dependencies, and
failure conditions. Distinguish an analytic proof from a computer-assisted
certificate and publish enough of the finite domain and checker to reproduce
the latter. For an effective or empirical claim, define the microscopic
owner, readout, scale, calibration, error bounds, controls, and independent
comparison before making a physical identification. Record imposed inputs
and imported formulas. Preserve failed routes and retractions as provenance;
update the ledger and navigation when status changes.

Do not search for numerical near-misses or coincidences, relabel a parameter
substitution as a derivation, or treat aesthetic unity as independent
evidence. External mathematical and experimental criticism is welcome;
internal agreement among documents or AI reviews is not independent
validation. See [CONTRIBUTING.md](CONTRIBUTING.md) for the practical review
checklist.

## License and citation

The repository is distributed under
[CC BY-NC-SA 4.0](LICENSE). Cite the repository as a research corpus or
software artifact, and cite individual versioned results at their stated
scope; a repository citation does not endorse every historical claim.

```bibtex
@misc{ftd2026,
  title = {Foundational Ternary Dynamics},
  year = {2026},
  note = {Research corpus and simulation software},
  url = {https://github.com/cpaci-tani/Foundational-Ternary-Dynamics}
}
```
