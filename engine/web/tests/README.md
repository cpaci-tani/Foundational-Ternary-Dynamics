# FTD Web Dashboard — Smoke Suite

Playwright-based smoke suite for `engine/web/index.html`. Boots a real
Chromium against the dashboard served by `engine/web/serve.py` and verifies
scale switching, listener cleanup, scenario contracts, and known regressions.

## Running

From this directory (`engine/web/tests/`):

```bash
# one-time setup
npm install
npm run install:browsers

# run the full suite
npm test

# run only the empirical/WASM verification subset
npm run test:empirical

# run the Scale-0 substrate protocol only
npm run test:scale0-protocol

# watch UI mode (for debugging a specific test)
npm run test:ui

# run a single test by name
npx playwright test -g "Scale sweep"
```

The config uses Playwright's `webServer` block to start / stop
`python ../serve.py 8081 --cache --quiet` automatically, so you do not need to
run the dev server separately. Port **8081** is chosen so it does not collide
with the 8080 that many developers leave running manually.

## What the suite covers

- **Scale sweep**: each live engine mode (`lattice`, `particles`, `atoms`,
  `molecules`, `planetary`, `cosmic`, `meta`) loads without console errors or
  failed network requests.
- **Bridge initialization**: `window._ftdBridge` becomes non-null within
  15 s of page load.
- **Phase B.1 regression**: `window._cosmicInterval` is never set after
  the Scale 5 refactor (cosmic mode now drives physics from the rAF
  loop instead of a parallel `setInterval`).
- **Constants contract**: `constants.js` still exports `K_B`, `ALPHA`,
  `G_STAR` as named values and `K_B === 0.511`.
- **Empirical/WASM subset**: `npm run test:empirical` combines scenario
  parity, WASM scenario execution, Verify-panel honesty checks, audit
  invariants, and force-field sampler probes.
- **Scale-0 substrate protocol**: `npm run test:scale0-protocol` runs
  `scale0-substrate-protocol-v2.spec.js`, the draft falsifier protocol that
  checks the conservative toggle recipe, Maxwell Hamiltonian conservation,
  `c_lat`, strict locality, charge conservation, genesis scaling/cluster
  count/null control, determinism, and Gauss projection.
- **Lifecycle harness** (engine-flawless audit, 2026-06-01): `lifecycle-harness`
  drives a per-scale mount → unmount round trip and asserts the net listener /
  resource leak is zero across scales.
- **Claim reconciliation**: `reconcile-claims` re-asserts the four prior
  web-layer fixes still hold (guards against silent regression).
- **Toggle coverage**: `toggle-coverage` exercises the canonical Scale-0 field
  toggle binding map, confirming each toggle is reachable and round-trips
  through the UI.
- **Overlay scheduler**: `overlay-scheduler` checks the overlay-scheduler
  invariants (no double-schedule, clean teardown).
- **Scale-0 telemetry contracts**: targeted specs assert scenario energy
  telemetry, vacuum/non-vacuum audit wiring, and the conservation micropanel
  remain connected to live diagnostics.

## What the suite does NOT cover

- Visual regression. Three.js + GPU output is nondeterministic across
  drivers; screenshot diffing is a trap.
- Cross-browser. We use ES module importmaps and Three.js; Chromium
  only is sufficient for the dashboard.
- Physical-world proof. The empirical/WASM subset tests whether the discrete
  substrate exhibits its pre-stated internal behavior; C++ CTest and Python
  pytest remain the broader physics and manifest-contract suites.

## Complete empirical runner

From the repository root:

```bash
python scripts/runners/run_empirical_verification_suite.py --profile quick
```

See `docs/EMPIRICAL_VERIFICATION_SUITE.md` for the full runbook and the
claim-boundary language to use when reporting results.

## Adding tests

Put new `*.spec.js` files in this directory. The config's `testMatch`
picks them up automatically. Prefer short, focused tests over large
end-to-end flows; each test re-navigates the page anyway and the
boot cost is ~3 s.
