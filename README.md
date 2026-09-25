# Foundational Ternary Dynamics

**Foundational Ternary Dynamics (FTD)** is a philosophy-of-mathematics project
with an explicit mathematical core and deliberately bounded physics
connections. It asks one question, as honestly as it can be asked:

> Starting from finite-alphabet, local, deterministic records with a ternary
> manifestation readout, what can be *built*, and exactly where and why does
> the building stop?

FTD is **not** an attempt to replace the Standard Model or general relativity.
It is ordered **Ontology > Logic > Math > Physics**: the substrate fixes a small
set of commitments, mathematics is built forward from them, and physics enters
as a *constraint*, not the sole arbiter. The discipline that makes the project
worth reading is that it labels every claim by exactly how far it has actually
been carried: theorem, derivation, selection, conjecture, parametric insertion,
imposed input, open problem, or closed-negative route. It does not let a label
promote a claim beyond its evidence.

The repository combines a theory corpus, verification and proof scripts, a
C++/CUDA simulation engine, and a browser workspace. The
[repository map](REPOSITORY_MAP.md) shows module ownership; the
[dated resume](docs/WHERE_WE_LEFT_OFF.md) records the current working state.

## The selected model

The [v3 constitution](docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V3_STRICT_DISCRETE_COMMON_ACTION.md)
adopts five postulates. The dimension, carrier, and reference law are choices
of this model; the postulates do not establish that nature uses them.

| Element | Current v3 specification |
|---|---|
| Space | Oriented three-dimensional cubical cell complex with a global chart and undefined outer boundary. |
| Time | One ordinal tick `n = 0, 1, ...`; records have births and may expire. Physical seconds are not primitive. |
| State | Finite alphabets on cells. The site readout `m` returns `-1`, `0`, or `+1` and does not expose the complete site record. |
| Causality | One update depends on at most a radius-one Moore neighborhood. This is a ceiling, not a requirement to use every neighbor. |
| Law | One homogeneous deterministic update `X[n+1] = Phi(X[n])` with an explicit synchronous schedule and a non-injective expiry case. The reference `Phi` is selected, not forced by the preceding rows. |

The [selected carrier](docs/theory/01_reference/SPEC_V3_FINITE_CARRIER_INVENTORY_R1_v2.md)
makes the finite-state cost explicit:

| Cell | Alphabet | Number of states |
|---|---|---:|
| Site | Ternary value, three-state collision layer, and 384 binary channel slots | `9 x 2^384` |
| Bond | Two nine-state relation slots | `81` |
| Plaquette | Four nine-state relation slots | `6,561` |
| Cube | Singleton | `1` |

The 384 site bits are exclusion slots for distinct field-channel labels. A
ternary site value is therefore a many-to-one readout of a much larger
microscopic record, not the whole state.

## Results at their stated scope

The [selected law](docs/theory/01_reference/SPEC_V3_COMMON_ACTION_PHI_R2_R5_v2.md)
and its certificates support the following statements. All are conditional
on the chosen carrier, collision, and preparation.

| Result | Exact scope | Not established by it |
|---|---|---|
| Finite collision | The frozen three-layer table has **55,008 pair rows**. Its parent certificate checks **128,499** exact cases; the integrated verifier currently reports **31/31** checks passing. | A unique law or a measured interaction. |
| Expiry | Eight distinct frame presentations map to one bound reserve while named phase, polarity, token/work, and carrier records survive. | A general heat law or complete physical energy account. |
| Minimum dynamic witnesses | The same `Phi` has finite propagation, reciprocal manifestation/expiry, a localized recurrent clock or proto-body, and a source/current ledger. | Stable matter or charged Maxwell dynamics. |
| Transverse vacuum sector | The exact three-tick linearization has two divergence-free transverse real field pairs with speed **1/6** in model units. For wavevectors of norm at most `kappa`, its normalized tangent differs from the slow generator by at most `(9/2) kappa^2 exp(3 kappa)`. | The charged sector, action normalization, a physical light speed, or nonlinear stability. |
| Input/dataflow check | The [firewall verifier](scripts/proofs/proof_v3_target_firewall.py) currently passes **26/26** rule and preparation dataflow checks, including checks that active inputs omit named physical target values. | Uniqueness, physical sufficiency, or independence of the historical design choices. |

R1-R6 are closed only at the scopes recorded in the constitution. The counts
above are finite certificate and software-verification counts, not numbers of
independently confirmed physical predictions.

## Physical status

The following remain open as general recoveries from the selected v3 law:

- stable matter and a particle spectrum;
- charged electromagnetism and a normalized physical coupling;
- prepared trials, detector events, and Born-rule frequencies;
- a common relativistic causal cone and material proper time;
- tensor gravity, lensing, and nonlinear gravitational dynamics.

A physical quantity must have a defined readout from finite `Phi` histories,
a domain of use, and an error or stability statement. A mathematical theorem
about the selected rule does not by itself identify its variables with
measurements. Values obtained by placing framework parameters into standard
physics formulas remain [parametric insertions](docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md),
not recoveries of those formulas.

The [C++ engine](engine/SPEC_ENGINE.md) is a simulation of a different,
legacy continuous-flux law with optional extensions. Its output measures that
engine at a specified configuration; it is not a run of the selected v3
`Phi`. The browser workspace also contains separate effective scale engines
and visualizations. Their current owners and the missing cross-scale links
are recorded in the [scale ownership audit](docs/reference/REF_SCALE_OWNERSHIP_AND_RECOVERY.md).

## Find the source of a claim

Claim status is controlled by the
[ledger](docs/theory/07_assessment/core_ledgers/LEDGER.md), followed by the
active v3 constitution, branch constitutions, and other prose. The
[dated resume](docs/WHERE_WE_LEFT_OFF.md) summarizes the current working
state; the [open-items tracker](docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md)
and [resolved-items tracker](docs/theory/07_assessment/core_ledgers/TRACKER_RESOLVED_ITEMS.md)
separate live work from provenance. Historical and retracted routes remain in
the [theory archive](docs/theory/archive/) so their status is visible.

| Area | Entry point |
|---|---|
| Theory and document navigation | [Curated index](docs/theory/META_INDEX.md) |
| v3 definitions and scope | [Constitution](docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V3_STRICT_DISCRETE_COMMON_ACTION.md), [carrier](docs/theory/01_reference/SPEC_V3_FINITE_CARRIER_INVENTORY_R1_v2.md), [law](docs/theory/01_reference/SPEC_V3_COMMON_ACTION_PHI_R2_R5_v2.md) |
| Repository and module ownership | [Repository map](REPOSITORY_MAP.md) |
| Engine and browser | [Engine specification](engine/SPEC_ENGINE.md), [web entry point](engine/web/index.html) |
| Contributions | [Contributing guide](CONTRIBUTING.md) |

## Run and check

These commands exercise documented code and finite checks. A passing test
does not change a claim's physical status.

```bash
python -m pip install -r requirements.txt -r requirements-dev.txt
python -m pytest scripts/tests/
python scripts/proofs/proof_v3_common_action_phi_v2.py
python scripts/proofs/proof_v3_target_firewall.py
python scripts/verification/verify_index_links.py --tracked
```

For the CPU-only C++ merge gate:

```bash
cmake -S engine -B engine/build -DFTD_ENABLE_CUDA=OFF
cmake --build engine/build --target ftd_merge_gate_build --config Release --parallel 24
ctest --test-dir engine/build -L merge_gate -j 24 -C Release --output-on-failure
```

The full Windows native build uses `engine\build_native.bat`. Its toolchain
and GPU requirements are recorded in [CLAUDE.md](CLAUDE.md).

Build the browser assets:

```bash
npm ci
npm run build
```

Run the source workspace with its local service:

```bash
python engine/web/serve.py 8080
```

Open `http://localhost:8080`. Browser test dependencies and commands are in
the [web test package](engine/web/tests/package.json).

## License and citation

The repository is distributed under [CC BY-NC-SA 4.0](LICENSE). Cite a
versioned document or certificate for a specific result; a repository-level
citation identifies the corpus, not the status of every historical claim.

```bibtex
@misc{ftd2026,
  title = {Foundational Ternary Dynamics},
  year = {2026},
  note = {Research corpus and simulation software},
  url = {https://github.com/cpaci-tani/Foundational-Ternary-Dynamics}
}
```
