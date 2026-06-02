# FTD Empirical Verification Suite

**Status:** current runbook
**Scope:** C++ engine, browser/WASM web engine, Python manifest/constant contracts
**Primary runner:** `scripts/runners/run_empirical_verification_suite.py`

This suite treats the FTD engine as an apparatus for testing the discrete
framework against pre-stated, falsifiable predictions. It can validate or
falsify substrate behavior inside the model. It does not prove that physical
nature is FTD, and it does not turn seeded constants or parametric insertions
into derivations.

The runner intentionally avoids numerical search scripts, near-miss scans, and
coincidence hunts.

---

## Quick Commands

Run the focused empirical suite:

```powershell
python scripts/runners/run_empirical_verification_suite.py --profile quick
```

Force a fresh native build first:

```powershell
python scripts/runners/run_empirical_verification_suite.py --profile quick --build --jobs 24
```

Run only the web/WASM empirical subset:

```powershell
python scripts/runners/run_empirical_verification_suite.py --profile web
```

Run the draft Scale-0 substrate falsifier protocol:

```powershell
python scripts/runners/run_empirical_verification_suite.py --profile substrate
```

Include the substrate protocol in the focused profile:

```powershell
python scripts/runners/run_empirical_verification_suite.py --profile quick --include-substrate-protocol
```

Run the full non-GPU CPU sweep:

```powershell
python scripts/runners/run_empirical_verification_suite.py --profile full --build --jobs 24
```

Run the web subset directly:

```powershell
cd engine/web/tests
npm run test:empirical
```

GPU campaigns remain WSL2-only:

```powershell
wsl.exe -d Ubuntu-22.04 -- bash -lc "cd /mnt/c/Users/cpaci/Desktop/ftd && cd engine/build_wsl && ctest --output-on-failure -j 1 -L gpu"
```

---

## Coverage Map

| Layer | What is tested | Command lane |
|---|---|---|
| Python contracts | Verify-manifest tier discipline, constants parity, dimensional-map shape | `python contract tests` |
| C++ discrete substrate | Quick native smoke gate: golden tick plus Gauss, conservation, determinism, constants, closed-negative, strict-validation, and master-quadratic checks | `engine empirical smoke` |
| C++ broader substrate sweep | Foundation labels, physical-law labels, golden tick, deterministic/emergent instrument labels | `engine focused empirical labels` |
| Web/WASM bridge | JS to C++ scenario parity, WASM scenario execution, Verify panel honesty contract | `web empirical specs` |
| Scale-0 protocol | Energy Hamiltonian, `c_lat`, locality, charge conservation, genesis scaling, cluster count, determinism, Gauss projection | `web Scale-0 substrate protocol` |
| Regression probes | Audit scenario invariants and force-field sampler non-emptiness/finite data | `audit-regression.spec.js`, `force-field-samplers.spec.js` |

The full profile expands the C++ lane to every non-GPU, non-benchmark CTest
and expands the web lane to the full Playwright suite. The Scale-0 substrate
protocol is kept as an explicit falsifier lane because its purpose is to fail
when the current apparatus deviates from the pre-stated predictions.

---

## Core Claims Covered

| Claim type | Representative tests | Honest status |
|---|---|---|
| Discrete locality | Scale-0 front tracking and no-flux-beyond-bound test | Internal model falsifier |
| Wave speed `c_lat = 1/sqrt(3)` | Scale-0 front speed test, C++ dispersion/lattice labels | Internal model falsifier |
| Constraint enforcement | Gauss projection tests in C++ and web protocol | Internal model falsifier |
| Determinism | C++ `determinism`, web two-run checkpoint trajectory | Internal model falsifier |
| Conservation in admissible configs | C++ physics label, web Maxwell Hamiltonian and charge conservation | Internal model falsifier |
| Emergent genesis behavior | Web amplitude scaling, cluster count/null-control tests, C++ engine-as-instrument labels | Internal model falsifier, not physical-world confirmation |
| Evidence scoreboard integrity | Verify manifest builder and Playwright panel checks | Documentation/UI honesty contract |

---

## What This Suite Does Not Claim

- It does not empirically prove FTD as a theory of nature.
- It does not run look-elsewhere searches or generate new near-miss claims.
- It does not promote seeded constants, standard-model formula insertions, or
  illustrative scenarios to derivations.
- It does not use MockBridge results to falsify WASM/C++ core claims.
- It does not make GPU campaign claims unless the WSL2 GPU lane is run.

Use result language like **confirmed inside the discrete substrate**, **deviation**,
or **falsified for this apparatus/configuration**. Avoid wording like
"proved physics" unless a separate physical measurement comparison has actually
been performed and its epistemic tag supports that claim.

---

## Maintenance Rules

1. Add new empirical web tests to `npm run test:empirical` only when they are
   falsifier-gated and not merely visual.
2. Add C++ tests to the broader empirical runner through CTest labels. Keep the
   quick smoke regex limited to stable, low-cost gates that should always run.
3. Keep pre-registered protocols immutable after lock; put results in audit
   files, not in the pre-registration.
4. If a test is only an implementation-faithfulness check, say so in its name
   or result doc.
5. Run `git diff --check` after edits and run at least the relevant lane before
   claiming the suite is green.
